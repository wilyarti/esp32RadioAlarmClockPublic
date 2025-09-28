//
// Created by wilyarti on 16/9/25.
//

#ifndef ESP32RADIOALARMCLOCK_HARDWARE_DEFINITIONS_H
#pragma once
#define ESP32RADIOALARMCLOCK_HARDWARE_DEFINITIONS_H
// Your pin definitions:
#define TFT_CS    27    // CS IO27
#define TFT_DC    26    // RS IO26 (assuming "RS 1026" was a typo for IO26)
#define TFT_MOSI  23    // SDI IO23 (MOSI = Master Out Slave In)
#define TFT_CLK   18    // SCLK IO18
#define TFT_RST   5     // RST IO05
#define TFT_MISO  12    // SDO IO12 (MISO = Master In Slave Out)

#define SDA 21
#define SCL 22
#define SD_SCK 14
#define SD_MISO 2
#define SD_MOSI 15
#define SD_CS 13

#define BCK_PIN  25
#define WS_PIN   32
#define DATA_PIN 33
#define BUTTON_A  38  //          37 CENTRE
#define BUTTON_B 37  //          38 LEFT
#define BUTTON_C  39  //          39 RIGHT
#define TOUCH_CS -1  // Disable touch


#endif //ESP32RADIOALARMCLOCK_HARDWARE_DEFINITIONS_H