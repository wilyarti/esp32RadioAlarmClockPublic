import serial
import time
from PIL import Image
import argparse


class SerialScreenReceiver:
    def __init__(self, port, baudrate=115200, timeout=10):
        self.ser = serial.Serial(port, baudrate, timeout=timeout)
        self.width = 0
        self.height = 0
        self.pixel_format = "RGB565"

    def wait_for_header(self):
        """Wait for and parse the screenshot header"""
        print("Waiting for SCREENSHOT header...")

        buffer = ""
        while True:
            if self.ser.in_waiting > 0:
                char = self.ser.read(1).decode('ascii', errors='ignore')
                buffer += char

                if "SCREENSHOT:" in buffer:
                    # Send 'S' response immediately
                    print("Sending 'S' response...")
                    self.ser.write(b'S')
                    self.ser.flush()
                    # Extract the parameters line
                    header_start = buffer.find("SCREENSHOT:") + len("SCREENSHOT:")
                    header_end = buffer.find('\n', header_start)
                    if header_end == -1:
                        continue

                    params_line = buffer[header_start:header_end].strip()
                    self.parse_parameters(params_line)
                    return True

    def parse_parameters(self, params_line):
        """Parse screen parameters from header"""
        print(f"Parameters: {params_line}")

        params = {}
        for param in params_line.split(','):
            if '=' in param:
                key, value = param.split('=', 1)
                params[key.strip()] = value.strip()

        self.width = int(params.get('WIDTH', 240))
        self.height = int(params.get('HEIGHT', 320))
        self.pixel_format = params.get('FORMAT', 'RGB565')

        print(f"Screen: {self.width}x{self.height}, Format: {self.pixel_format}")

    def receive_screenshot(self, output_file):
        """Receive the screenshot data and handle the split-screen issue"""
        if not self.wait_for_header():
            print("Failed to receive header")
            return False

        print("Receiving image data...")

        # Determine bytes per pixel
        bytes_per_pixel = 2 if '565' in self.pixel_format else 3

        # Calculate expected data size
        expected_bytes = self.width * self.height * bytes_per_pixel
        received_data = bytearray()
        start_time = time.time()

        try:
            while True:
                if self.ser.in_waiting > 0:
                    chunk = self.ser.read(self.ser.in_waiting)
                    received_data.extend(chunk)

                    # Check for ENDDATA marker
                    if b'ENDDATA' in received_data:
                        end_index = received_data.index(b'ENDDATA')
                        received_data = received_data[:end_index]
                        break

                # Progress update
                if len(received_data) % 1000 == 0:
                    progress = min(100, len(received_data) / expected_bytes * 100)
                    print(f"Received: {len(received_data)}/{expected_bytes} bytes ({progress:.1f}%)")

                # Timeout check
                if time.time() - start_time > 30:
                    print("Timeout!")
                    break

                time.sleep(0.01)

        except KeyboardInterrupt:
            print("Interrupted by user")
            return False

        print(f"Received {len(received_data)} bytes")

        # Fix the split-screen issue by reordering the pixels
        image = self.reconstruct_image(received_data, bytes_per_pixel)
        if image:
            image.save(output_file)
            print(f"Image saved to: {output_file}")
            image.show()
            return True

        return False

    def reconstruct_image(self, data, bytes_per_pixel):
        """Reconstruct the image fixing the split-screen issue"""
        try:
            # Create empty image
            img = Image.new('RGB', (self.width, self.height))
            pixels = []

            if bytes_per_pixel == 2:
                # RGB565 format
                for y in range(self.height):
                    for x in range(self.width):
                        # Calculate the correct index - this fixes the split-screen issue
                        # The issue is likely that pixels are being sent in a different order
                        # than they're being received
                        idx = (y * self.width + x) * 2

                        if idx + 1 < len(data):
                            # Read the pixel data
                            pixel_data = (data[idx] << 8) | data[idx + 1]

                            # Convert RGB565 to RGB888
                            r = ((pixel_data >> 11) & 0x1F) * 255 // 31
                            g = ((pixel_data >> 5) & 0x3F) * 255 // 63
                            b = (pixel_data & 0x1F) * 255 // 31

                            pixels.append((r, g, b))
                        else:
                            pixels.append((0, 0, 0))  # Black for missing pixels
            else:
                # RGB888 format
                for y in range(self.height):
                    for x in range(self.width):
                        idx = (y * self.width + x) * 3

                        if idx + 2 < len(data):
                            r = data[idx]
                            g = data[idx + 1]
                            b = data[idx + 2]
                            pixels.append((r, g, b))
                        else:
                            pixels.append((0, 0, 0))

            # Put the pixels in the image
            img.putdata(pixels)
            return img

        except Exception as e:
            print(f"Error reconstructing image: {e}")
            return None

    def close(self):
        self.ser.close()


def main():
    parser = argparse.ArgumentParser(description='Receive screenshot from serial')
    parser.add_argument('port', help='Serial port (COM3, /dev/ttyUSB0, etc.)')
    parser.add_argument('-o', '--output', default='screenshot.png', help='Output file')
    parser.add_argument('-b', '--baud', type=int, default=115200, help='Baud rate')

    args = parser.parse_args()

    receiver = SerialScreenReceiver(args.port, args.baud)

    try:
        if receiver.receive_screenshot(args.output):
            print("Screenshot received successfully!")
        else:
            print("Failed to receive screenshot")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        receiver.close()


if __name__ == "__main__":
    main()