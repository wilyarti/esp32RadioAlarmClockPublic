//
// Created by wilyarti on 18/8/25.
//
#pragma once
#include <FS.h>           //this needs to be first, or it all crashes and burns...
#include <Arduino.h>
#include <WiFi.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
//#define LED_BUILTIN 22
#include <time.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SPIFFS.h>
#include <Wire.h>
#include <Arduino.h>
#include <Wire.h>
#include <Bounce2.h>
#include <ArduinoJson.h>  //https://github.com/bblanchon/ArduinoJson
#include <TimeLib.h>      // Software RTC library
#include <SPI.h>
#include "audio_functions.h"
#include "wifi_functions.h"
#include "serial_functions.h"
#include "handleNewMessages.h"
#include "display.h"
#include "alarm_functions.h"
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <SPI.h>
#include "audio_functions.h"
#include "graphing_functions.h"
#include "hardware_definitions.h"
#include <SPI.h>
#include "globals.h"
#include "display.h"
#include "current_sensor_functions.h"
#include "INA219.h"
#include <ESP32Ping.h>
#include <SD.h>
#include <SPI.h>

// Buttons
#define NUM_BUTTONS 3
#define CUSTOM_DARK 0x3186 // Background color

#ifndef BAREMETALCLOCK_MAIN_H
#define BAREMETALCLOCK_MAIN_H

// Time

class main{

};

extern HardwareSerial Serial;

String formatTime(unsigned long epoch);
void telegramTask(void * pvParameters);
void buttonCheck();
void restoreRadioOnBoot();
void timeUpdateTask(void * pvParameters);
void telegramTask(void * pvParameters);
void checkFullMemoryStatus();
void checkCpuUsage();
void managePower();
float getBufferSlope();
#endif //BAREMETALCLOCK_MAIN_H
