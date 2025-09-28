//
// Created by wilyarti on 16/9/25.
//

#include "storage_functions.h"

Preferences preferences;

// Redundant
void initialisePreferences() {
    // Open Preferences with a namespace. 'my-app' is the namespace,
    // and 'false' means it's opened in read-write mode.
    preferences.begin("Preferences", false);


    // Get the current counter value. If the key 'counter' doesn't exist,
    // it will return the default value (0 in this case).
    unsigned int init = preferences.getUInt("init", 0);

    if (init) {
        Serial.println("Skipping. Already initialized.");
    }

    // Store the updated counter value back to NVS
    preferences.putUInt("init", 1);

    // Close the preferences
    preferences.end();
}

void loadParamatersOnBoot() {
    g_machine.defaultRadioStations[0] = "http://radiorecord.hostingradio.ru/hbass96.aacp";
    g_machine.defaultRadioStations[1] = "http://serpent0.duckdns.org:8088/mbcsfm.pls";
    g_machine.defaultRadioStations[2] = "https://ice.creacast.com/sudradio_aac";
    g_machine.defaultRadioStations[3] = "http://www.abc.net.au/res/streaming/audio/aac/radio_national.pls";
    g_machine.defaultRadioStations[4] = "http://icecast.vgtrk.cdnvideo.ru/vestifm_mp3_192kbps";
    g_machine.defaultRadioStations[5] = "http://173.226.180.143/alexjonesshow-mp3";
    g_machine.defaultRadioStations[6] = "https://icecast-rian.cdnvideo.ru/voicerus";
    g_machine.defaultRadioStations[7] = "http://radiorecord.hostingradio.ru/hbass96.aacp";


    preferences.begin("Preferences", false);
    g_machine.volume = preferences.getUInt("volume", 4);
    g_machine.currentStation = preferences.getInt("currentStation", 0);
    g_machine.currentStream = preferences.getString("currentStream", g_machine.defaultRadioStations[0]);

    // Fetch radio stations. Program them if they aren't there.
    for (int i = 0; i < 8; i++) {
        String key = "station_" + String(i);
        g_machine.radioStations[i] = preferences.getString(key.c_str(), "");

        // Set default if empty
        if (g_machine.radioStations[i] == "") {
            Serial.println("No stations programmed. Using default.");
            g_machine.radioStations[i] = g_machine.defaultRadioStations[i];
            preferences.putString(key.c_str(), g_machine.defaultRadioStations[i]);
        }
    }
    g_machine.loopRetarderShortBuffer = preferences.getInt("loopRtrdrShort", 500);
    g_machine.loopRetarder = preferences.getInt("loopRetarder", 0);
    g_machine.bufferTarget = preferences.getInt("bufferTarget", 20);

    g_machine.maxPowerLevelmW = preferences.getInt("maxPowerLevelmW", 2500);
    g_machine.alarm_hours = preferences.getUInt("alarm_hours", 3);
    g_machine.alarm_minutes = preferences.getUInt("alarm_minutes", 30);
    g_machine.alarm_enabled = preferences.getBool("alarm_minutes", true);
    g_machine.alarm_volume = preferences.getInt("alarm_volume", 4);
    g_machine.timeZone = preferences.getInt("timeZone", 10);
    g_machine.alarm_enabled = preferences.getBool("alarm_enabled", false);
    //g_machine.recieveNowPlayingUpdates = preferences.getBool("playingUpdates",false);
    g_machine.recieveNowPlayingUpdates = false; //TODO fix this
    preferences.end();
}

void loadTelegram() {
    preferences.begin("Telegram", true); // read only
    g_machine.telegramToken = preferences.getString("token", "");
    g_machine.telegramChatID = preferences.getString("chatID", "");
    preferences.end();
}

void loadWiFi() {
    preferences.begin("WiFi", true); // read only
    // Fetch radio stations. Program them if they aren't there.
    for (int i = 0; i < 5; i++) {
        String ssidKey = "ssid_" + String(i);
        String passKey = "password_" + String(i);
        String ssid = preferences.getString(ssidKey.c_str(), "");
        String password = preferences.getString(passKey.c_str(), "");
        // Copy to fixed char arrays
        strncpy(g_machine.WifiNetworks[i].ssid, ssid.c_str(), MAX_SSID_LEN);
        strncpy(g_machine.WifiNetworks[i].password, password.c_str(), MAX_PASS_LEN);

        // Ensure null termination
        g_machine.WifiNetworks[i].ssid[MAX_SSID_LEN-1] = '\0';
        g_machine.WifiNetworks[i].password[MAX_PASS_LEN-1] = '\0';
    }
    preferences.end();
}

void debugPreferences() {
    delay(5000);
    preferences.begin("Preferences", true); // Read-only mode

    Serial.println("\n=== PREFERENCES DEBUG ===");

    // Print all the values you're loading
    Serial.printf("Volume: %u\n", preferences.getUInt("volume", 9999));
    Serial.printf("Current Station: %d\n", preferences.getInt("currentStation", 9999));
    Serial.printf("Current Stream: %s\n", preferences.getString("currentStream", "DEFAULT").c_str());

    Serial.println("\nRadio Stations:");
    for (int i = 0; i < 8; i++) {
        String key = "station_" + String(i);
        String value = preferences.getString(key.c_str(), "NOT_SET");
        Serial.printf("  %s: %s\n", key.c_str(), value.c_str());
    }

    Serial.printf("Alarm Hours: %u\n", preferences.getUInt("alarm_hours", 9999));
    Serial.printf("Alarm Minutes: %u\n", preferences.getUInt("alarm_minutes", 9999));

    // Fixed: There was a typo in your code - using "alarm_minutes" instead of "alarm_enabled"
    Serial.printf("Alarm Enabled: %s\n", preferences.getBool("alarm_enabled", false) ? "true" : "false");

    Serial.printf("Timezone: %u\n", preferences.getInt("timezone", 9999));

    preferences.end();
    Serial.println("=== END DEBUG ===\n");
}
