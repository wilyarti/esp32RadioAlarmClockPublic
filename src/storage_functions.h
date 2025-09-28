//
// Created by wilyarti on 16/9/25.
//

#ifndef ESP32RADIOALARMCLOCK_STORAGE_FUNCTIONS_H
#define ESP32RADIOALARMCLOCK_STORAGE_FUNCTIONS_H
#pragma once
#include <Preferences.h>
#include "main.h"
#include "hardware_definitions.h"

extern Preferences preferences;
void loadWiFi();
void loadTelegram();
void debugPreferences();
void loadParamatersOnBoot();
void initialisePreferences();
#endif //ESP32RADIOALARMCLOCK_STORAGE_FUNCTIONS_H