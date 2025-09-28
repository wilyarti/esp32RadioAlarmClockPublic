#include <Arduino.h>
#include "main.h"
#include "../lib/ESP32-audioI2S/src/Audio.h"

// Created by wilyarti on 14/9/25.

void sendResponse(String message, int type) {
    if (type == 0) {
        // Send via Serial
        Serial.println(message);
    } else if (type == 1) {
        // Send via Telegram bot
        g_machine.bot->sendMessage(g_machine.telegramChatID, message, "HTML"); // Added "HTML"
    }
}

void processCommand(String line, int type) {
    if (line.startsWith("/")) {
        line = line.substring(1);
    }
    if (line.startsWith("http")) {
        connectToStream(line);
        g_machine.currentStation = -1;
        g_machine.currentStream = line;
    } else if (line.startsWith("addStation")) {
        handleAddStationCommand(line, type);
        g_machine.currentStation = -1;
        g_machine.currentStream = line;
    } else if (line.startsWith("listStations")) {
        listStations(type);
    } else if (line == "start" || line == "help") {
        sendHelpMessage(type);
    } else if (line == "time") {
        char timeStr[20]; // Buffer for the formatted time
        sprintf(timeStr, "Time: %02d:%02d:%02d",
                g_machine.timeClient->getHours(),
                g_machine.timeClient->getMinutes(),
                g_machine.timeClient->getSeconds());
        sendResponse(String(timeStr), type);
    } else if (line == "updateTime") {
        g_machine.timeClient->update();
        sendResponse("Time updated.", type);
    } else if (line == "startRadio") {
        sendResponse("Starting radio...", type);
        startRadio();
    } else if (line == "stopRadio") {
        sendResponse("Stopping radio...", type);
        stopRadio();
    } else if (line.startsWith("readEEPROM")) {
        //readEEPROM(chat_id);
    } else if (line == "reset") {
        // TODO figure out how to make this not loop....
        if (type == 0) ESP.restart();
    } else if (line.startsWith("alarm ")) {
        // Parse alarm command: "alarm 0330 12"
        setAlarm(line, type);
    } else if (line == "alarmOff") {
        disableAlarm(type);
    } else if (line == "alarmStatus") {
        checkAlarmStatus(type);
    } else if (line == "nowPlaying") {
        sendNowPlaying(type);
    } else if (line.startsWith("volume_")) {
        setVolume(line, type);
    } else if (line.startsWith("station")) {
        setStation(line, type);
    } else if (line.startsWith("setChatID")) {
        setTelegramChatID(line, type);
    } else if (line.startsWith("setTimezone")) {
        setTimezone(line, type);
    } else if (line == "powerStatus") {
        getPowerStatus(type);
    } else if (line == "getPowerSettings") {
        getPowerSettings(type);
    } else if (line.startsWith("setPowerSettings")) {
        setPowerSettings(line, type);
    } else if (line == "commandList") {
        sendCommandList(type);
    } else if (line == "toggleNowPlayingCommand") {
        toggleNowPlayingCommand(type);
    } else if (line == "screenshot") {
        //handleScreenshotCommand();
        //sendResponse("Taking screenshot...", 1);
    } else if (line == "song") {
        sendResponse(g_machine.nowPlayingText, type);
    } else if (line == "getLatency") {

        Audio::BufferInfo bufferStatus = g_machine.audio->getBufferInfo();
        // Access individual values
        Serial.printf("Latency: %.2f seconds\n", bufferStatus.latencySeconds);
        Serial.printf("Buffer: %u/%u bytes (%.1f%%)\n",
                      bufferStatus.bufferBytes, bufferStatus.bufferSize, bufferStatus.bufferPercent);
        Serial.printf("Bitrate: %u bps\n", bufferStatus.bitrate);
        Serial.printf("Time remaining: %u seconds\n", bufferStatus.timeRemaining);

        // Calculate how many bytes correspond to current latency
        if (bufferStatus.bitrate > 0) {
            uint32_t bytesForCurrentLatency = (bufferStatus.latencySeconds * bufferStatus.bitrate) / 8;
            Serial.printf("Bytes for %.1fs latency: %u\n",
                          bufferStatus.latencySeconds, bytesForCurrentLatency);
        }
        sendResponse(String(g_machine.audio->getStreamLatency()), type);
    } else {
        sendResponse("Unknown command, bro.", type);
    }
}
void toggleNowPlayingCommand(int type) {
    if (g_machine.recieveNowPlayingUpdates) {
        sendResponse("Updates turned off.", type);
        g_machine.recieveNowPlayingUpdates = false;
    } else {
        sendResponse("Updates turned on.", type);
        g_machine.recieveNowPlayingUpdates = true;
    }
    preferences.begin("Preferences", false);
    preferences.putBool("playingUpdates", g_machine.recieveNowPlayingUpdates);
    preferences.end();
}


void sendNowPlaying(int type) {
    sendResponse(g_machine.nowPlayingText, type);
}

void setVolume(String command, int type) {
    String value;
    if (command.startsWith("/")) {
        value = command.substring(8); // "/volume_" is 8 chars
    } else {
        value = command.substring(7); // "volume_" is 7 chars
    }
    int volume = value.toInt();
    if (volume >= 0 && volume <= 100) {
        g_machine.volume = volume;
        setAudioVolume(volume);
        char response[30];
        sprintf(response, "Volume set to %d", volume);
        sendResponse(response, type);
    } else {
        sendResponse("Volume must be between 0 and 100.", type);
    }
}

void setStation(String command, int type) {
    String value = command.substring(8);
    int stationIndex = value.toInt();
    if (stationIndex >= 0 && stationIndex < 8) {
        if (g_machine.radioStations[stationIndex] != "") {
            g_machine.currentStation = stationIndex;
            g_machine.currentStream = g_machine.radioStations[stationIndex];
            connectToStream(g_machine.currentStream);
            char response[50];
            sprintf(response, "Playing station %d", stationIndex);
            sendResponse(response, type);
        } else {
            sendResponse("Station not set.", type);
        }
    } else {
        sendResponse("Invalid station index.", type);
    }
}

void setTelegramChatID(String command, int type) {
    String chatID = command.substring(10);
    g_machine.telegramChatID = chatID;
    preferences.begin("Preferences", false);
    preferences.putString("telegramChatID", chatID);
    preferences.end();
    sendResponse("Telegram Chat ID updated.", type);
}

void setTimezone(String command, int type) {
    String value = command.substring(12);

    // Input validation
    if (value.length() == 0) {
        sendResponse("Error: Timezone value missing", type);
        return;
    }

    // Check if it's a valid integer
    for (int i = 0; i < value.length(); i++) {
        char c = value[i];
        // Allow digits and optional minus sign at the beginning
        if (!isdigit(c) && !(i == 0 && c == '-')) {
            sendResponse("Error: Invalid timezone format. Use integer like -5, 0, 3", type);
            return;
        }
    }

    int timezone = value.toInt();

    // Validate timezone range (typically -12 to +14)
    if (timezone < -12 || timezone > 14) {
        sendResponse("Error: Timezone must be between -12 and +14", type);
        return;
    }

    // Store the valid timezone
    g_machine.timeZone = timezone;
    g_machine.timeClient->setTimeOffset(timezone * 3600);

    // Save to preferences
    preferences.begin("Preferences", false);
    preferences.putInt("timeZone", timezone);
    preferences.end();

    char response[30];
    sprintf(response, "Timezone set to UTC%d", timezone);
    sendResponse(response, type);

    // Optional: Force immediate update with new timezone
    g_machine.timeClient->update();
}

void getPowerStatus(int type) {
    float bus_voltage = g_machine.current_sensor->getBusVoltage_mV();
    float current_mA = g_machine.current_sensor->getCurrent_mA();
    float power_mW = g_machine.current_sensor->getPower_mW();

    char response[100];
    sprintf(response, "Bus Voltage: %.2f V\nCurrent: %.2f mA\nPower: %.2f mW", bus_voltage, current_mA, power_mW);
    sendResponse(response, type);
}

void getPowerSettings(int type) {
    char response[100];
    sprintf(response, "loopRetarder: %d\nloopRtrdrShort: %d, buffer: %d(s)\n \nmaxPowerLevelmW: %d, ", g_machine.loopRetarder,
    g_machine.loopRetarderShortBuffer, g_machine.bufferTarget, g_machine.maxPowerLevelmW);
    sendResponse(response, type);
}

void setPowerSettings(String command, int type) {
    String params = command.substring(17);
    int spaceIndex = params.indexOf(' ');
    if (spaceIndex != -1) {
        String target = params.substring(0, spaceIndex);
        String valueStr = params.substring(spaceIndex + 1);
        int value = valueStr.toInt();
        preferences.begin("Preferences", false);

        if (target == "playing") {
            if (value <0) value = 0;
            if (value >1000) value = 1000;
            g_machine.loopRetarder = value;
            preferences.putInt("loopRetarder", g_machine.loopRetarder);
            sendResponse("loopRetarder updated.", type);
        } else if (target == "stopped") {
            if (value <0) value = 0;
            if (value >1000) value = 1000;
            g_machine.loopRetarderShortBuffer = value;
            preferences.putInt("loopRtrdrShort", g_machine.loopRetarderShortBuffer);
            sendResponse("loopRetarderShortBuffer updated.", type);
        } else if (target == "buffer") {
            if (value > 30) value = 30;
            if (value < 5) value = 5;
            g_machine.bufferTarget = value;
            preferences.putInt("bufferTarget", g_machine.bufferTarget);
        } else if (target == "maxPowerLevelmW") {
            if (value > 4000) value = 4000;
            if (value < 500) value = 500;
            g_machine.maxPowerLevelmW = value;
            preferences.putInt("maxPowerLevelmW", g_machine.maxPowerLevelmW);
        }            else {
            sendResponse("Invalid target. Use 'playing' or 'stopped'.", type);
        }
        preferences.end();

    } else {
        sendResponse("Invalid format. Use: setPowerSettings <target> <value>", type);
    }
}

void sendCommandList(int type) {
    String commandList = "/commandList\n/time\n/updateTime\n/startRadio\n/stopRadio\n/listStations\n/addStation\n/http\n/alarm\n/alarmOff\n/alarmstatus\n/nowPlaying\n/volume_\n/station\n/setChatID\n/setTimezone\n/powerStatus\n/getPowerSettings\n/setPowerSettings\n/help\n/reset";
    sendResponse(commandList, type);
}


void setAlarm(String command, int type) {
    // Remove "alarm " prefix
    String params = command.substring(6);

    // Split into time and volume
    int spaceIndex = params.indexOf(' ');
    if (spaceIndex == -1) {
        sendResponse("Invalid format. Use: alarm HHMM VOLUME (e.g., alarm 0330 12)", type);
        return;
    }

    String timeStr = params.substring(0, spaceIndex);
    String volumeStr = params.substring(spaceIndex + 1);

    // Validate time format (4 digits)
    if (timeStr.length() != 4) {
        sendResponse("Time must be 4 digits (HHMM)", type);
        return;
    }

    // Extract hours and minutes
    int hour = timeStr.substring(0, 2).toInt();
    int minute = timeStr.substring(2, 4).toInt();
    int volume = volumeStr.toInt();

    // Validate time values
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
        sendResponse("Invalid time format. Use 24-hour format (0000-2359)", type);
        return;
    }

    // Validate volume
    if (volume < 0 || volume > 100) {
        sendResponse("Volume must be between 0-100", type);
        return;
    }

    // Set alarm
    g_machine.alarm_hours = hour;
    g_machine.alarm_minutes = minute;
    g_machine.alarm_volume = volume;
    g_machine.alarm_enabled = true;

    preferences.begin("Preferences", false);
    preferences.putUInt("alarm_hours", g_machine.alarm_hours);
    preferences.putUInt("alarm_minutes", g_machine.alarm_minutes);
    preferences.putBool("alarm_enabled", g_machine.alarm_enabled);
    preferences.putInt("alarm_volume", g_machine.alarm_volume);

    preferences.end();
    char response[60];
    sprintf(response, "Alarm set for %02d:%02d at volume %d", g_machine.alarm_hours, g_machine.alarm_minutes,
            g_machine.alarm_volume);
    sendResponse(String(response), type);
}

void disableAlarm(int type) {
    g_machine.alarm_enabled = false;
    g_machine.alarm_hours = 0;
    g_machine.alarm_minutes = 0;
    g_machine.alarm_volume = 0;
    preferences.begin("Preferences", false);
    preferences.putUInt("alarm_hours", g_machine.alarm_hours);
    preferences.putUInt("alarm_minutes", g_machine.alarm_minutes);
    preferences.putBool("alarm_enabled", g_machine.alarm_enabled);
    preferences.putInt("alarm_volume", g_machine.alarm_volume);
    preferences.end();
    sendResponse("Alarm disabled", type);
}

void checkAlarmStatus(int type) {
    if (g_machine.alarm_enabled) {
        char response[50];
        sprintf(response, "Alarm set for %02d:%02d at volume %d", g_machine.alarm_hours, g_machine.alarm_minutes,
                g_machine.alarm_volume);
        sendResponse(String(response), type);
    } else {
        char response[50];
        sprintf(response, "Last alarm was %02d:%02d at volume %d", g_machine.alarm_hours, g_machine.alarm_minutes,
                g_machine.alarm_volume);
        sendResponse(String(response), type);
        sendResponse("No alarm set", type);
    }
}

// Add this function to your main loop to check for alarm triggers
void checkAlarm() {
    if (g_machine.alarm_enabled == true) {
        int currentHour = g_machine.timeClient->getHours();
        int currentMinute = g_machine.timeClient->getMinutes();
        //int currentSecond = g_machine.timeClient->getSeconds();

        // Check if it's alarm time (only check once per minute at second 0)
        if (currentHour == g_machine.alarm_hours && currentMinute == g_machine.alarm_minutes) {
            triggerAlarm();
        }
    }
}

void triggerAlarm() {
    // Set volume
    setAudioVolume(g_machine.alarm_volume);

    /*
    g_machine.alarm_enabled = false;
    preferences.begin("Preferences", false);
    preferences.putBool("alarm_enabled", g_machine.alarm_enabled);
    preferences.end();
    */
    //sendResponse("Alarm running!", 0);
    //sendResponse("Alarm running!", 1);
}

void sendHelpMessage(int type) {
    String helpMessage = "";

    if (type == 1) {
        // HTML formatting for Telegram/Web
        helpMessage = "🔊 <b>Radio & Alarm System</b> 🔔\n\n";
        helpMessage += "📋 <b>Available Commands:</b>\n";
        helpMessage += "🕐 <b>Time Commands:</b>\n";
        helpMessage += "▫️ <code>/time</code> - Display current time\n";
        helpMessage += "▫️ <code>/updateTime</code> - Sync with time server\n";
        helpMessage += "▫️ <code>/setTimezone &lt;offset&gt;</code> - Set timezone offset from UTC\n\n";
        helpMessage += "📻 <b>Radio Commands:</b>\n";
        helpMessage += "▫️ <code>/startRadio</code> - Start radio playback\n";
        helpMessage += "▫️ <code>/stopRadio</code> - Stop radio playback\n";
        helpMessage += "▫️ <code>/station_&lt;index&gt;</code> - Play station from list\n";
        helpMessage += "▫️ <code>/listStations</code> - List saved stations\n";
        helpMessage += "▫️ <code>/addStation &lt;index&gt; &lt;url&gt;</code> - Add station to list\n";
        helpMessage += "▫️ <code>http://...</code> - Play specific stream URL\n\n";
        helpMessage += "🔊 <b>Audio Commands:</b>\n";
        helpMessage += "▫️ <code>/volume_NN</code> - Set playback volume (e.g. /volume_50)\n\n";
        helpMessage += "⏰ <b>Alarm Commands:</b>\n";
        helpMessage += "▫️ <code>/alarm HHMM VOL</code> - Set alarm\n";
        helpMessage += "   Example: <code>alarm 0630 10</code> (6:30 AM, vol 10)\n";
        helpMessage += "▫️ <code>/alarmOff</code> - Disable alarm\n";
        helpMessage += "▫️ <code>/alarmstatus</code> - Check alarm status\n\n";
        helpMessage += "⚙️ <b>System Commands:</b>\n";
        helpMessage += "▫️ <code>/setChatID &lt;ID&gt;</code> - Set your Telegram chat ID\n";
        helpMessage += "▫️ <code>/powerStatus</code> - Read power sensor\n";
        helpMessage += "▫️ <code>/getPowerSettings</code> - View power settings\n";
        helpMessage += "▫️ <code>/setPowerSettings &lt;target&gt; &lt;value&gt;</code> - Change power settings (playing/stopped/buffer(s)/maxPowerLevelmW)\n";
        helpMessage += "▫️ <code>/commandList</code> - Get a clickable list of commands\n";
        helpMessage += "▫️ <code>/reset</code> - Restart device\n";
        helpMessage += "▫️ <code>/help</code> - Show this message\n\n";
        helpMessage += "🎯 <b>Quick Examples:</b>\n";
        helpMessage += "• Morning news: <code>alarm 0700 8</code> then <code>/startRadio</code>\n";
        helpMessage += "• Custom station: <code>http://stream.url</code>\n";
        helpMessage += "• Check time: <code>/time</code>\n";
    } else {
        // Serial-compatible formatting (plain text)
        helpMessage += "=== Radio & Alarm System ===\n\n";
        helpMessage += "AVAILABLE COMMANDS:\n\n";
        helpMessage += "TIME COMMANDS:\n";
        helpMessage += "  /time - Display current time\n";
        helpMessage += "  /updateTime - Sync with time server\n";
        helpMessage += "  /setTimezone <offset> - Set timezone offset from UTC\n\n";
        helpMessage += "RADIO COMMANDS:\n";
        helpMessage += "  /startRadio - Start radio playback\n";
        helpMessage += "  /stopRadio - Stop radio playback\n";
        helpMessage += "  /station <index> - Play station from list\n";
        helpMessage += "  /listStations - List saved stations\n";
        helpMessage += "  /addStation <index> <url> - Add station to list\n";
        helpMessage += "  http://... - Play specific stream URL\n\n";
        helpMessage += "AUDIO COMMANDS:\n";
        helpMessage += "  /volume_NN - Set playback volume (e.g. /volume_21)\n\n";
        helpMessage += "ALARM COMMANDS:\n";
        helpMessage += "  alarm HHMM VOL - Set alarm\n";
        helpMessage += "    Example: alarm 0630 10 (6:30 AM, vol 10)\n";
        helpMessage += "  /alarmOff - Disable alarm\n";
        helpMessage += "  /alarmStatus - Check alarm status\n\n";
        helpMessage += "SYSTEM COMMANDS:\n";
        helpMessage += "  /setChatID <ID> - Set your Telegram chat ID\n";
        helpMessage += "  /powerStatus - Read power sensor\n";
        helpMessage += "  /getPowerSettings - View power settings\n";
        helpMessage += "  /setPowerSettings <target> <value> - Change power settings\n";
        helpMessage += "  /commandList - Get a clickable list of commands\n";
        helpMessage += "  /reset - Restart device\n";
        helpMessage += "  /help - Show this message\n\n";
        helpMessage += "QUICK EXAMPLES:\n";
        helpMessage += "  * Morning news: alarm 0700 8 then /startRadio\n";
        helpMessage += "  * Custom station: http://stream.url\n";
        helpMessage += "  * Check time: /time\n";
    }

    sendResponse(helpMessage, type);
}

void listStations(int type) {
    preferences.begin("Preferences", true);

    // Fetch radio stations. Program them if they're not there.
    String helpMessage = "List of stations: \n";
    for (int i = 0; i < 8; i++) {
        String key = "station_" + String(i);
        helpMessage += "/" + key + ": " + preferences.getString(key.c_str(), "") + "\n";
    }
    helpMessage += "currentStation: " + String(g_machine.currentStation);
    helpMessage += " currentStream: " + g_machine.currentStream;

    preferences.end();
    if (type == 99) {
        Serial.println(helpMessage);
    } else {
        sendResponse(helpMessage, type);
    }
}

void handleAddStationCommand(const String command, int type) {
    String cmd = command;
    cmd.trim();
    cmd.toLowerCase();
    // Parse the command: "addStation 1 http://example.com/stream"
    int firstSpace = cmd.indexOf(' ');
    if (firstSpace == -1) {
        sendResponse("Error: Invalid format. Use: addStation <index> <url>", type);
        return;
    }

    int secondSpace = cmd.indexOf(' ', firstSpace + 1);
    if (secondSpace == -1) {
        sendResponse("Error: Invalid format. Use: addStation <index> <url>", type);
        return;
    }

    // Extract station index
    String indexStr = cmd.substring(firstSpace + 1, secondSpace);
    int stationIndex = indexStr.toInt();

    // Extract URL (rest of the string)
    String url = command.substring(secondSpace + 1);
    url.trim();

    // Validate station index
    if (stationIndex < 0 || stationIndex >= 8) {
        sendResponse("Out of bounds.", type);
        return;
    }

    // Validate URL (basic check)
    if (url.length() == 0) {
        sendResponse("Error: URL cannot be empty", type);
        return;
    }

    Serial.printf("Adding station %d: %s\n", stationIndex, url.c_str());

    // Update in-memory storage
    g_machine.radioStations[stationIndex] = url;

    // Update persistent storage
    preferences.begin("Preferences", false);

    String key = "station_" + String(stationIndex);
    if (preferences.putString(key.c_str(), url)) {
        sendResponse("Success.", type);
    } else {
        sendResponse("Failed.", type);
    }

    preferences.end();

    // Verify the save worked
    preferences.begin("Preferences", true);
    String savedUrl = preferences.getString(key.c_str(), "");
    preferences.end();

    if (savedUrl == url) {
        Serial.printf("Verification successful: Station %d = %s\n", stationIndex, savedUrl.c_str());
    } else {
        Serial.printf("Verification failed! Expected: %s, Got: %s\n", url.c_str(), savedUrl.c_str());
    }
}
