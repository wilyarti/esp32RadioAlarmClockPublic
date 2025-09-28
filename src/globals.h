#ifndef ESP32RADIOALARMCLOCK_GLOBALS_H
#define ESP32RADIOALARMCLOCK_GLOBALS_H

// Necessary headers for the types used in the state structs.
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Audio.h" // from ESP32-audioI2S
#include <NTPClient.h>
#include <UniversalTelegramBot.h>
#include <WiFiClient.h>

#include "INA219.h"
#define MAX_SSID_LEN 32
#define MAX_PASS_LEN 64
#define MAX_NETWORKS 5


/*
 * =============================================================================
 * Casey Muratori-style "Fat Structs" for State Management
 * =============================================================================
 */

// A simple struct for 2D points.
struct Point {
    int x, y;
};

// A simple struct for rectangles (position and size).
struct Rect {
    int x, y, w, h;
};

// WiFi Network Struct
struct WiFiNetwork {
    char ssid[MAX_SSID_LEN];
    char password[MAX_PASS_LEN];
};
// This struct holds all the layout information (coordinates and dimensions)
// for the UI elements. Centralizing this makes the UI easier to manage and
// modify.
struct UILayout {
    // --- Main Screen Layout ---
    Rect now_playing_title_area;
    Rect now_playing_text_area;

    // --- Buttons on "Now Playing" screen ---
    Rect btn_next;
    Rect btn_play_pause;
    Rect btn_volume;

    // --- Buttons on "Alarm Settings" screen ---
    Rect btn_alarm_hour;
    Rect btn_alarm_min;
    Rect btn_alarm_toggle;

    // --- Clock Display ---
    int clock_y;
    int clock_height;
    Point clock_time_pos;
    Point clock_seconds_pos;
    Rect clock_seconds_clear;
    Rect alarm_clock_clear_area;
    Point alarm_clock_cursor;


    // --- Info Area (RSSI, Volume text) ---
    Rect info_rssi_area;
    Point info_rssi_cursor;
    Rect info_graph_area;

    Point info_graph_cursor;

    // --- Graphs/Meters ---
    Point graph_origin;
    int graph_ram_y_offset;
    int graph_volume_y_offset;
    Rect graph_ram_label_clear_area;
    Rect alarm_label_clear_area;

    Point graph_ram_label_cursor;
    Point graph_ram_free_cursor;
    Point graph_playtime_cursor;
    Point graph_rssi_cursor;
    Point graph_volume_cursor;
};


/*
 * CASEY MURATORI-STYLE FAT STRUCT
 * All state in one flat, monolithic struct for maximum locality
 * and simplicity of passing around entire program state.
 */
typedef struct {
    // --- WiFi State ----
    WiFiNetwork WifiNetworks[5];
    volatile WiFiClient wifiClient;
    volatile unsigned int currentWiFiNetwork;
    // --- Display State ---
    TFT_eSPI* tft;
    int screenDisplayed;

    // --- Button States ---
    bool btnState1;
    bool btnState2;
    bool btnState3;
    bool antiFlickering;

    // --- Audio Playback ---
    Audio* audio;
    unsigned int volume;
    bool audioIsPlaying;
    String currentStream;
    String nowPlayingText;
    volatile int bufferLengthInSeconds;
    volatile bool recieveNowPlayingUpdates;
    volatile int currentStation;
    volatile bool stopPlayBackFlag;
    volatile bool startPlayBackFlag;
    volatile bool advanceAudioVolumeFlag;
    volatile bool advanceRadioStationFlag;
    volatile int bitratekBs;
    String host;


    // --- Radio Stations ---
    String radioStations[8];
    String defaultRadioStations[8];
    char customStreamURL[256];
    bool playCustomStreamFlag;

    // --- Alarm Clock ---
    unsigned int alarm_hours;
    unsigned int alarm_minutes;
    int alarm_volume;
    bool alarm_enabled;

    // --- System & Network ---
    NTPClient* timeClient;
    UniversalTelegramBot* bot;
    String telegramChatID;
    String telegramToken;
    volatile bool telegramMessageFlag;
    int timeZone;
    volatile int latestPingTime;
    volatile bool newPingFlag;


    // --- SD Card ---
    bool sdcard;
    // --- UI Layout ---
    UILayout layout;

    // --- Sensors ---
    INA219* current_sensor;

    // --- Power management ---
    int loopCount;
    unsigned int loopCheck;
    unsigned int bufferIndexCheck;
    volatile int loopRetarder;
    volatile int loopRetarderShortBuffer;
    volatile bool bufferFilled;
    volatile  int bufferLengths[12];
    volatile int bufferLengthsIndex;
    volatile unsigned int bufferTarget;
    volatile unsigned int bufferLowerThreshold;
    volatile unsigned int maxPowerLevelmW;
    unsigned int mWpowerReadingsMillis;
    volatile  float mWpowerReadings[12];
    volatile int mWpowerReadingsIndex;
    volatile unsigned int avgPwrmW;

    unsigned int volumeModifiedTime;



} MachineState;

// Global instance (typical in this style)
extern MachineState g_machine;

// Initialization functions
void setupLayout();



#endif //ESP32RADIOALARMCLOCK_GLOBALS_H