//
// Created by wilyarti on 12/9/25.
//

#ifndef ESP32RADIOALARMCLOCK_DISPLAY_H
#define ESP32RADIOALARMCLOCK_DISPLAY_H
#pragma once
#include <TFT_eSPI.h>
#include "main.h"
#include "bitmap.c"
#include "7seg20.h"
#include "ani.h"
#include "Orbitron_Medium_16.h"
#include "Orbitron_12.h"
#include "Orbitron_14.h"
#include "Orbitron_18.h"
#include "dejavusansmono_12.h"
#define grey 0x65DB


// Clock constants moved to UILayout in globals.h
void drawClock();
void drawSmoothClock();
void setupDisplay();
void updateDisplay();
void drawButton(int x, int y, int w, int h, const char* label, bool selected);
void displayLoop();
void updateNowPlayingText(const char *info);
void updateNowPlayingTitle(const char *info);
void drawStatusBar();
void updateButtons();
void drawAlarmClock();
void drawDisplay();
void displayAlarm();
void currentSensorData();
void pulseTelgrameLogo();
void pulsePingLogo();
#endif //ESP32RADIOALARMCLOCK_DISPLAY_H
