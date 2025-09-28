//
// Created by wilyarti on 14/9/25.
//

#ifndef ESP32RADIOALARMCLOCK_GRAPHING_FUNCTIONS_H
#define ESP32RADIOALARMCLOCK_GRAPHING_FUNCTIONS_H
#pragma once
#include <TFT_eSPI.h>
#include "display.h"
#include "../lib/ESP32-audioI2S/src/Audio.h"
void drawGraphicalComponents();
void displayVolumeLevel();
void displayRSSILevel();
String formatPlayingTime(uint32_t milliseconds) ;
uint16_t rainbowColor(uint8_t spectrum);
void linearMeter(int val, int x, int y, int w, int h, int g, int n, byte s);
void wifiMeter(int val, int x, int y, int w, int h, int g, int n, byte s);
void drawLowBattery(int x, int y, int width, int height);
#endif //ESP32RADIOALARMCLOCK_GRAPHING_FUNCTIONS_H