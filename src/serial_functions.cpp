#include "serial_functions.h"

extern WiFiNetwork networks[MAX_NETWORKS];

void checkSerial() {
    static char serialBuffer[256];
    static int bufferPos = 0;

    while (Serial.available() > 0) {
        char inChar = Serial.read();

        if (inChar == '\n' || inChar == '\r') { // End of line
            serialBuffer[bufferPos] = '\0'; // Null-terminate the string
            if (bufferPos > 0) {
                String line(serialBuffer);
                line.trim();
                if (line == "screenshot") {
                    int result = serialScreenServer("screenshot.bmp");
                } else {
                    processCommand(line, 0);
                }
            }
            bufferPos = 0; // Reset for next command
            break; // Process one command per call
        } else if (bufferPos < sizeof(serialBuffer) - 1) {
            serialBuffer[bufferPos++] = inChar;
        }
    }
}
//====================================================================================
//                Serial server function that sends the data to the client
//====================================================================================
bool serialScreenServer(String filename)
{
    // Precautionary receive buffer garbage flush for 50ms
    uint32_t clearTime = millis() + 50;
    while ( millis() < clearTime && Serial.read() >= 0) delay(0); // Equivalent to yield() for ESP8266;

    // Send screen size etc. using a simple header with delimiters for client checks
    //sendParameters(filename);
    Serial.println("SCREENSHOT:");
    uint32_t timeoutTime = millis() + 100;  // Calculate timeout time

    // Wait for 'S' response with 100ms timeout
    while (millis() < timeoutTime) {
        if (Serial.available() > 0 && Serial.read() == 'S') {
            break; // Received 'S', continue
        }
        delay(0); // Equivalent to yield() for ESP8266;
    }

    // Check if we timed out
    if (millis() >= timeoutTime) {
        return false; // Timeout - no 'S' received within 100ms
    }
    uint8_t color[3 * NPIXELS]; // RGB and 565 format color buffer for N pixels
    uint32_t lastCmdTime = millis(); // Initialize for timeout tracking

    // Send all the pixels on the whole screen
    for ( uint32_t y = 0; y < g_machine.tft->height(); y++)
    {
        // Increment x by NPIXELS as we send NPIXELS for every byte received
        for ( uint32_t x = 0; x < g_machine.tft->width(); x += NPIXELS)
        {
            delay(0); // Equivalent to yield() for ESP8266;

            // Check for abort command without blocking
            if ( Serial.available() > 0 ) {
                if ( Serial.read() == 'X' ) {
                    // X command byte means abort, so clear the buffer and return
                    clearTime = millis() + 50;
                    while ( millis() < clearTime && Serial.read() >= 0) delay(0); // Equivalent to yield() for ESP8266;
                    return false;
                }
            }

#if defined BITS_PER_PIXEL && BITS_PER_PIXEL >= 24 && NPIXELS > 1
            // Fetch N RGB pixels from x,y and put in buffer
            g_machine.tft->readRectRGB(x, y, NPIXELS, 1, color);
            // Send buffer to client
            Serial.write(color, 3 * NPIXELS); // Write all pixels in the buffer
#else
            // Fetch N 565 format pixels from x,y and put in buffer
            if (NPIXELS > 1) g_machine.tft->readRect(x, y, NPIXELS, 1, (uint16_t *)color);
            else
            {
                uint16_t c = g_machine.tft->readPixel(x, y);
                color[0] = c>>8;
                color[1] = c & 0xFF;  // Swap bytes
            }
            // Send buffer to client
            Serial.write(color, 2 * NPIXELS); // Write all pixels in the buffer
#endif
        }
    }

    Serial.flush(); // Make sure all pixel bytes have been despatched
    Serial.println("ENDDATA");

    return true;
}
//====================================================================================
//    Send screen size etc.using a simple header with delimiters for client checks
//====================================================================================
void sendParameters(String filename)
{
    Serial.write('W'); // Width
    Serial.write(g_machine.tft->width()  >> 8);
    Serial.write(g_machine.tft->width()  & 0xFF);

    Serial.write('H'); // Height
    Serial.write(g_machine.tft->height() >> 8);
    Serial.write(g_machine.tft->height() & 0xFF);

    Serial.write('Y'); // Bits per pixel (16 or 24)
    if (NPIXELS > 1) Serial.write(BITS_PER_PIXEL);
    else Serial.write(16); // readPixel() only provides 16-bit values

    Serial.write('?'); // Filename next
    Serial.print(filename);

    Serial.write('.'); // End of filename marker

    Serial.write(FILE_EXT); // Filename extension identifier

    Serial.write(*FILE_TYPE); // First character defines file type j,b,p,t
}
void setupNetworkViaInput() {
    g_machine.tft->fillScreen(TFT_BLUE);
    g_machine.tft->setCursor(0, 0);
    g_machine.tft->println("ERROR ALL WIFI NETWORKS FAILED");
    g_machine.tft->println("Connect serial baud 9600 \nto configure device.");

    // Initialize networks array first
    //memset(networks, 0, sizeof(networks));
    int waitingForAMate = millis();
    while (true) {

        Serial.println("\nNo network programmed.");

        // Wait for SSID
        while (!Serial.available()) {
            if (millis() - waitingForAMate > 1000*10) {
                ESP.restart();
            }
            Serial.println("Enter WiFi SSID: ");
            delay(100);
        }
        String ssid = Serial.readStringUntil('\n');
        ssid.trim();

        Serial.println("Enter password: ");

        // Wait for password
        while (!Serial.available()) delay(100);
        String password = Serial.readStringUntil('\n');
        password.trim();

        if (ssid.length() > 0) {
            preferences.begin("WiFi", false);
            preferences.putString("ssid_0", ssid);
            preferences.putString("password_0", password);
            preferences.end();
            Serial.print("Network programmed ssid: ");
        } else {
            Serial.print("Invalid input ssid: ");
        }
        Serial.print(ssid);
        Serial.print(" pass: ");
        Serial.println(password);
        break;
    }
    Serial.println("\nConfigure Telegram? ");
    while (!Serial.available()) delay(100);
    String response = Serial.readStringUntil('\n');
    response.trim();
    if (response == "Yes" || response == "yes" || response == "Y"|| response == "y") {
        Serial.println("Configuring telegram:");
        while (true) {
            Serial.println("Enter Telegram Chat ID: ");

            // Wait for SSID
            while (!Serial.available()) delay(100);
            String chatID = Serial.readStringUntil('\n');
            chatID.trim();

            Serial.println("Enter token: ");

            // Wait for password
            while (!Serial.available()) delay(100);
            String token = Serial.readStringUntil('\n');
            token.trim();

            if (chatID.length() > 0 && token.length() > 0) {
                preferences.begin("Telegram", false);
                preferences.putString("chatID", chatID);
                preferences.putString("token", token);
                preferences.end();
                Serial.println("Configured chatID: ");
            }else {
                Serial.print("Invalid input chatID: ");
            }
            Serial.print(chatID);
            Serial.print(" token: ");
            Serial.println(token);
            break;
        }
    }
    Serial.println("Configured. Restarting...");
    delay(2000);
    ESP.restart();

}
