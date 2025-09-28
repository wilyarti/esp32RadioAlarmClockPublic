cmake_minimum_required(VERSION 3.16)
project(esp32RadioAlarmClock)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set path to Arduino core
set(ARDUINO_CORE_DIR "$ENV{HOME}/.platformio/packages/framework-arduinoespressif32")

# -------------------------
# Include directories - FIXED WITH EXPLICIT PATHS
# -------------------------
# -------------------------
# Include directories - COMPLETE SET
# -------------------------
include_directories(
        # Arduino core
        ${ARDUINO_CORE_DIR}/cores/esp32
        ${ARDUINO_CORE_DIR}/variants/esp32

        # Arduino libraries
        ${ARDUINO_CORE_DIR}/libraries/ArduinoOTA/src
        ${ARDUINO_CORE_DIR}/libraries/EEPROM/src
        ${ARDUINO_CORE_DIR}/libraries/FS/src
        ${ARDUINO_CORE_DIR}/libraries/HTTPClient/src
        ${ARDUINO_CORE_DIR}/libraries/SPI/src
        ${ARDUINO_CORE_DIR}/libraries/SPIFFS/src
        ${ARDUINO_CORE_DIR}/libraries/Update/src
        ${ARDUINO_CORE_DIR}/libraries/WiFi/src
        ${ARDUINO_CORE_DIR}/libraries/WiFiClientSecure/src
        ${ARDUINO_CORE_DIR}/libraries/Wire/src

        # ESP32-specific includes
        ${ARDUINO_CORE_DIR}/tools/sdk/esp32/include
        ${ARDUINO_CORE_DIR}/tools/sdk/esp32/include/esp32
        ${ARDUINO_CORE_DIR}/tools/sdk/esp32/include/freertos
        ${ARDUINO_CORE_DIR}/tools/sdk/esp32/include/heap
        ${ARDUINO_CORE_DIR}/tools/sdk/esp32/include/log
        ${ARDUINO_CORE_DIR}/tools/sdk/esp32/include/soc
        ${ARDUINO_CORE_DIR}/tools/sdk/esp32/include/spiffs
        ${ARDUINO_CORE_DIR}/tools/sdk/esp32/include/tcpip_adapter
        ${ARDUINO_CORE_DIR}/tools/sdk/esp32/include/ulp
        ${ARDUINO_CORE_DIR}/tools/sdk/esp32/include/vfs
        ${ARDUINO_CORE_DIR}/tools/sdk/esp32/include/wpa_supplicant
        ${ARDUINO_CORE_DIR}/tools/sdk/esp32/include/xtensa-debug-module
        ${ARDUINO_CORE_DIR}/tools/sdk/esp32/include/xtensa-fpu
        ${ARDUINO_CORE_DIR}/tools/sdk/esp32/include/xtensa

        # Project directories
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/lib/arduino-audio-tools
        ${CMAKE_SOURCE_DIR}/lib/ESP32-A2DP
)

# -------------------------
# Source files
# -------------------------
file(GLOB_RECURSE ARDUINO_CORE_SOURCES
        "${ARDUINO_CORE_DIR}/cores/esp32/*.c"
        "${ARDUINO_CORE_DIR}/cores/esp32/*.cpp"
)

file(GLOB_RECURSE ARDUINO_LIB_SOURCES
        "${ARDUINO_CORE_DIR}/libraries/*/src/*.c"
        "${ARDUINO_CORE_DIR}/libraries/*/src/*.cpp"
)

# Get PlatformIO libdeps path
set(PLATFORMIO_LIBDEPS "${CMAKE_SOURCE_DIR}/.pio/libdeps/esp32dev")

# Manually add specific library paths that CLion can't find
if(EXISTS ${PLATFORMIO_LIBDEPS})
    include_directories(lib/Volos/src)
    # Bounce2
    include_directories(${PLATFORMIO_LIBDEPS}/Bounce2/src)

    # ArduinoJson
    include_directories(${PLATFORMIO_LIBDEPS}/ArduinoJson/src)

    # WiFiManager
    include_directories(${PLATFORMIO_LIBDEPS}/WiFiManager)

    # Adafruit GFX
    include_directories(${PLATFORMIO_LIBDEPS}/Adafruit GFX Library)

    # Adafruit ILI9341
    include_directories(${PLATFORMIO_LIBDEPS}/Adafruit ILI9341)

    # Time
    include_directories(${PLATFORMIO_LIBDEPS}/Time)

    # NTPClient
    include_directories(${PLATFORMIO_LIBDEPS}/NTPClient/src)

    # lvgl
    include_directories(${PLATFORMIO_LIBDEPS}/ESP32-audioI2S)

    # lv drivers
    include_directories(${PLATFORMIO_LIBDEPS}/lv_drivers/display)
    include_directories(${PLATFORMIO_LIBDEPS}/lv_drivers/indev)

    # Include all other libraries generically
    file(GLOB LIB_DIRS "${PLATFORMIO_LIBDEPS}/*")
    foreach(LIB_DIR ${LIB_DIRS})
        if(IS_DIRECTORY ${LIB_DIR})
            include_directories(${LIB_DIR})
            if(EXISTS "${LIB_DIR}/src")
                include_directories("${LIB_DIR}/src")
            endif()
        endif()
    endforeach()
endif()

# -------------------------
# Source files
# -------------------------
file(GLOB_RECURSE SOURCES
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/*.c"
        "${CMAKE_SOURCE_DIR}/lib/*.cpp"
        "${CMAKE_SOURCE_DIR}/lib/*.c"
)

# -------------------------
# Dummy executable for CLion indexing
# -------------------------
add_executable(${PROJECT_NAME} ${SOURCES}
        src/handleNewMessages.cpp
        src/handleNewMessages.h
)
# -------------------------
# Define Arduino/ESP macros for code analysis
# -------------------------
target_compile_definitions(${PROJECT_NAME} PRIVATE
        ARDUINO=10805
        ARDUINO_ARCH_ESP32
        ESP32=1
        CORE_DEBUG_LEVEL=0
        USE_ARDUINO_FFAT=1
)


# -------------------------
# PlatformIO custom targets
# -------------------------
add_custom_target(pio_build
        COMMAND pio run
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Building project with PlatformIO"
)

add_custom_target(pio_upload
        COMMAND pio run -t upload
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Uploading project to device"
)

add_custom_target(pio_clean
        COMMAND pio run -t clean
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Cleaning PlatformIO build"
)

add_custom_target(pio_monitor
        COMMAND pio device monitor
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Starting serial monitor"
)
set(SERIAL_PORT "/dev/ttyUSB0")  # Change this to your ESP32's port
# NEW: Combined upload and monitor target
add_custom_target(pio_upload_and_monitor
        COMMAND pio run -t upload && picocom ${SERIAL_PORT}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Uploading project and starting serial monitor"
)