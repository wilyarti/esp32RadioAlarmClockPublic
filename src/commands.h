//
// Created by wilyarti on 14/9/25.
//
#pragma once
#include "handleNewMessages.h"
#include <Arduino.h>
#ifndef ESP32RADIOALARMCLOCK_COMMANDS_H
#define ESP32RADIOALARMCLOCK_COMMANDS_H

void processCommand(String command, int type);
void sendMessage(String message, int type);
void triggerAlarm();
void checkAlarm();
void checkAlarmStatus(int type);
void disableAlarm(int type);
void setAlarm(String command, int type);
void sendHelpMessage(int type);
void handleAddStationCommand(String command, int type);
void listStations(int type);
void setVolume(String command, int type);
void setStation(String command, int type);
void setTelegramChatID(String command, int type);
void setTimezone(String command, int type);
void getPowerStatus(int type);
void getPowerSettings(int type);
void setPowerSettings(String command, int type);
void sendCommandList(int type);
void sendNowPlaying(int type);
void toggleNowPlayingCommand(int type);

#endif //ESP32RADIOALARMCLOCK_COMMANDS_H
