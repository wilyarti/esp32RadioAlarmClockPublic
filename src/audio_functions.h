#ifndef AUDIO_FUNCTIONS_H
#define AUDIO_FUNCTIONS_H
#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include "Audio.h"
#include "main.h"
#include "storage_functions.h"
#include "hardware_definitions.h"
#include "commands.h"

// Function declarations
bool psramAvailable();
void audio_setup();
void audio_start(const char *url);
void audio_stop();
void audio_loop();
void stopRadio();
bool isAudioPlaying();
void connectToStream(String url);
void nextStation();
void audio_showstation();
void stopBluetooth();
void startBluetooth();
void setupBluetooth();
void startRadio();
void setAudioVolume(int volume);

#endif //AUDIO_FUNCTIONS_H
