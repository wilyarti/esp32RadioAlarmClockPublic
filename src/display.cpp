//
// Created by wilyarti on 12/9/25.
//
#include "display.h"
#include "graphing_functions.h"
#include "../lib/TFT_eSPI/TFT_eSPI.h"

volatile int SCREENLOCK = false;
// TFT_eSPI and screenDisplayed are now in g_machine

// Custom colors
#define CUSTOM_DARK 0x3186 // Background color
unsigned long lastScreenUpdateMillis = 0;
unsigned long lastTextBarUpdate = 0;
bool labelUpdated = false;
int selectedButton = 0;

char timeBuf[10]; // HH:MM:SS + null terminator = 9 chars
char alarmBuf[8];
// Variables for smooth updates
String time2 = ""; // HH:MM
String date2 = ""; // MM-DD
String year2 = ""; // YYYY
String se = ""; // Seconds
bool inv = false; // Invert display
int press2 = 0, press3 = 0; // Button states
int c = 0; // Brightness level
int frame = 0; // Animation frame
uint8_t bright[6] = {10, 60, 120, 180, 220, 255}; // Brightness levels

void drawDisplay() {
    while (SCREENLOCK) delay(2);
    SCREENLOCK = true;
    drawGraphicalComponents(); // can we make it 30 FPS???
    drawLowBattery(0, 0, 30, 15); // x, y, width, height, percentage
    if (g_machine.telegramMessageFlag) {
        pulseTelgrameLogo();
    }
    if (g_machine.newPingFlag) {
        pulsePingLogo();
    }
    SCREENLOCK = false;
}

void pulseTelgrameLogo() {
    // for (int colorStep = 0; colorStep < 192; colorStep += 24) {
    //     uint16_t color = rainbowColor(colorStep);
    //     g_machine.tft->drawBitmap(160, 0, telegramBitmap, 20, 20, color);
    //     delay(125);
    // }
    g_machine.tft->drawBitmap(160, 0, telegramBitmap, 20, 20, TFT_GREEN);
    //g_machine.tft->fillRect(160, 0, 20, 20, TFT_BLACK);
}

// Pulse the ping logo. The remaining color reflects the quality of the connection.
void pulsePingLogo() {
    // for (int colorStep = 0; colorStep < 192; colorStep += 24) {
    //     uint16_t color = rainbowColor(colorStep);
    //     g_machine.tft->drawBitmap(140, 0, pingBitmap, 20, 20, color);
    //     delay(125);
    // }
    int color = rainbowColor(map(g_machine.latestPingTime, 0, 500, 63, 0)); // GREED2RED
    g_machine.tft->drawBitmap(160 - 25, 0, pingBitmap, 20, 20, color);
    g_machine.newPingFlag = false;
}

// Thanks to Volos Projects: https://github.com/VolosR/InternetClock180x60
void drawProfessionalClock() {
    // Get current time components
    int hours = g_machine.timeClient->getHours();
    int minutes = g_machine.timeClient->getMinutes();
    int seconds = g_machine.timeClient->getSeconds();
    int day = g_machine.timeClient->getDay(); // Day of week (0-6)

    // Create formatted strings manually
    char timeBuf[10];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hours, minutes);

    char secBuf[3];
    snprintf(secBuf, sizeof(secBuf), "%02d", seconds);

    while (SCREENLOCK) {
        delay(5);
        Serial.println("Screen locked.");
    }
    SCREENLOCK = true;
    g_machine.tft->setTextColor(TFT_WHITE, TFT_BLACK);

    // Draw seconds - only when changed
    if (se != String(secBuf)) {
        g_machine.tft->setTextSize(2); // Set default size

        g_machine.tft->setFreeFont(&Orbitron_Medium_16);
        se = String(secBuf);
        g_machine.tft->fillRect(g_machine.layout.clock_seconds_clear.x, g_machine.layout.clock_seconds_clear.y,
                                g_machine.layout.clock_seconds_clear.w, g_machine.layout.clock_seconds_clear.h,
                                TFT_BLACK); // Clear area
        uint16_t secColor = rainbowColor(map(seconds, 0, 60, 0, 159));
        g_machine.tft->setTextColor(secColor, TFT_BLACK);
        g_machine.tft->drawString(se, g_machine.layout.clock_seconds_pos.x, g_machine.layout.clock_seconds_pos.y);
        g_machine.tft->unloadFont(); // Unload any custom font
    } else if (String(timeBuf) != time2) {
        g_machine.tft->setTextSize(2); // Set default size
        g_machine.tft->setTextColor(TFT_WHITE, TFT_BLACK);
        //char hours[2] = strcpy(hours, timeBuf);

        time2 = String(timeBuf);
        g_machine.tft->setFreeFont(&DSEG7_Classic_Bold_30);
        g_machine.tft->setCursor(g_machine.layout.clock_time_pos.x, g_machine.layout.clock_time_pos.y);
        uint16_t hourColour = rainbowColor(map(hours, 0, 12, 0, 159));
        g_machine.tft->setTextColor(hourColour, TFT_BLACK);
        g_machine.tft->drawString(String(timeBuf[0]) + String(timeBuf[1]), g_machine.layout.clock_time_pos.x,
                                  g_machine.layout.clock_time_pos.y);
        g_machine.tft->setTextColor(TFT_WHITE, TFT_BLACK);
        g_machine.tft->drawString(":", g_machine.layout.clock_time_pos.x + 100, g_machine.layout.clock_time_pos.y);
        uint16_t minuteColour = rainbowColor(map(minutes, 0, 60, 0, 159));
        g_machine.tft->setTextColor(minuteColour, TFT_BLACK);
        g_machine.tft->drawString(String(timeBuf[3]) + String(timeBuf[4]), g_machine.layout.clock_time_pos.x + 120,
                                  g_machine.layout.clock_time_pos.y);
    }
    g_machine.tft->unloadFont(); // Unload any custom font
    g_machine.tft->setTextFont(1); // Set to default font
    g_machine.tft->setTextSize(2); // Set default size
    g_machine.tft->setTextColor(TFT_WHITE, TFT_BLACK);
    SCREENLOCK = false;
}

// Lock done elsewhere
void displayAlarm() {
    while (SCREENLOCK) {
        delay(5);
        Serial.println("Screen locked.");
    }
    SCREENLOCK = true;
    char newTimeBuf[8]; // HH:MM:SS + null terminator = 9 chars
    // Format the time into buffer
    snprintf(newTimeBuf, sizeof(newTimeBuf), "%02d:%02d",
             g_machine.alarm_hours, g_machine.alarm_minutes);
    int hours = g_machine.timeClient->getHours();
    const Rect r = g_machine.layout.alarm_clock_clear_area;
    g_machine.tft->fillRect(r.x, r.y, r.w, r.h, TFT_BLACK);
    g_machine.tft->setTextSize(2); // Set default size

    g_machine.tft->setFreeFont(&Orbitron_Medium_16);
    uint16_t secColor = rainbowColor(map(hours, 0, 60, 0, 159));
    g_machine.tft->setTextColor(secColor, TFT_BLACK);
    if (g_machine.alarm_enabled) {
        g_machine.tft->drawString(newTimeBuf, r.x, r.y);
        g_machine.tft->setTextSize(1); // Set default size
        g_machine.tft->drawString(String(g_machine.alarm_volume), r.x + 120, r.y);
        g_machine.tft->drawBitmap(110, 0, alarmBitmap, 20, 20, TFT_GREEN);
    } else {
        g_machine.tft->drawString("-", r.x, r.y);
        g_machine.tft->drawBitmap(110, 0, alarmBitmap, 20, 20, TFT_RED);
    }
    g_machine.tft->drawRect(0, r.y, r.w, r.h, TFT_WHITE);
    g_machine.tft->unloadFont(); // Unload any custom font
    SCREENLOCK = false;
}

// Lock done elsewhere
void currentSensorData() {
    g_machine.tft->setTextColor(TFT_ORANGE, TFT_BLACK);
    g_machine.tft->setTextSize(1);
    g_machine.tft->setCursor(g_machine.layout.graph_ram_label_cursor.x + 140,
                             g_machine.layout.graph_ram_label_cursor.y - 16); // Above ram
    g_machine.tft->printf("%dmW %dv", g_machine.current_sensor->getPower_mW(),
                          g_machine.current_sensor->getBusVoltage());
    g_machine.tft->setTextSize(1);
    g_machine.tft->printf(" volume: %d", g_machine.alarm_volume);
}


void setupDisplay() {
    g_machine.tft->init();
    g_machine.tft->setRotation(1);
    g_machine.tft->writecommand(0x11);
    g_machine.tft->fillScreen(TFT_BLUE);
    g_machine.tft->setRotation(0); // 0 = 0°  1 = 90°  2 = 180°  3 = 270°
    g_machine.tft->fillScreen(TFT_BLACK);
    // g_machine.tft->setTextColor(TFT_WHITE);
    g_machine.tft->setTextColor(TFT_WHITE, TFT_BLACK);
    g_machine.tft->setTextWrap(true);
    g_machine.tft->setFreeFont(&DejaVu_Sans_Mono_12);
    g_machine.tft->setCursor(0, 0);
    g_machine.tft->setTextSize(1);
    lastScreenUpdateMillis = millis() - 200;
    //g_machine.tft->loadFont(FreeSansBold24pt7b);
}

void updateButtons() {
    // TODO add indicator of buttonpress
    if (g_machine.btnState1 == true) {
        //drawButton(g_machine.layout.btn_play_pause.x, g_machine.layout.btn_play_pause.y, g_machine.layout.btn_play_pause.w, g_machine.layout.btn_play_pause.h, ">|", true);
        if (g_machine.audioIsPlaying) {
            stopRadio();
        } else {
            startRadio();
        }
        drawDisplay();
        g_machine.btnState1 = false;
    }
    if (g_machine.btnState2 == true) {
        //drawButton(g_machine.layout.btn_volume.x, g_machine.layout.btn_volume.y, g_machine.layout.btn_volume.w, g_machine.layout.btn_volume.h, "+", true);
        int newVolume = 0;
        if (g_machine.volume < 20) {
            newVolume = g_machine.volume + 1;
        } else {
            newVolume = 0;
        }
        setAudioVolume(newVolume);
        drawDisplay();
        g_machine.btnState2 = false;
    }
    if (g_machine.btnState3 == true) {
        //drawButton(g_machine.layout.btn_next.x, g_machine.layout.btn_next.y, g_machine.layout.btn_next.w, g_machine.layout.btn_next.h, ">>", true);
        nextStation();
        g_machine.btnState3 = false;
    }
}

void displayLoop() {
    unsigned long currentMillis = millis();
    // 3 times a second
    if (currentMillis - lastScreenUpdateMillis >= 300) {
        if (g_machine.screenDisplayed == 0) {
            //drawClock();
            //drawSmoothClock();
            drawProfessionalClock();
        }
    }
    // Every 1s
    if (currentMillis - lastScreenUpdateMillis >= 1000) {
        drawDisplay();
        lastScreenUpdateMillis = currentMillis;
    }
    // Every 5s
    if (currentMillis - lastTextBarUpdate >= 5000) {
        displayAlarm(); // local function
        drawStatusBar(); // local function
        while (SCREENLOCK) {
            delay(5);
            Serial.println("Screen locked.");
        }
        SCREENLOCK = true;
        displayRSSILevel(); // Also displays RSSI in graphing_functions.cpp
        SCREENLOCK = false;
        lastTextBarUpdate = currentMillis;
    }
    updateButtons();
}

void drawStatusBar() {
    // This function draws a status bar at the bottom of the screen.
    // It's designed to fit in the space where buttons might have been.
    while (SCREENLOCK) delay(2);
    SCREENLOCK = true;

    int bar_y = 255; // Y-position of the status bar, near the bottom
    int bar_height = 320 - bar_y;
    int screen_width = g_machine.tft->width();

    // --- Style Setup ---
    uint16_t bg_color = TFT_BLACK;
    uint16_t text_color = TFT_GREEN;
    uint16_t outline_color = TFT_GREEN;

    // --- Draw Background and Outline ---
    g_machine.tft->fillRect(0, bar_y, screen_width, bar_height, bg_color);
    g_machine.tft->drawRect(0, bar_y, screen_width, bar_height, outline_color);

    // --- Text Setup ---
    g_machine.tft->setTextSize(1); // Use size 1 for more text
    g_machine.tft->setTextColor(text_color, bg_color);
    g_machine.tft->setFreeFont(&DejaVu_Sans_Mono_12);

    // --- Prepare Text ---
    // Date
    time_t epochTime = g_machine.timeClient->getEpochTime();
    struct tm *ptm = localtime(&epochTime);
    char dateStr[20];
    strftime(dateStr, sizeof(dateStr), "%a %d %b %y", ptm);

    // host name display
    char hostDisplay[21]; // 15 chars + null terminator + buffer
    if (g_machine.host.length() > 18) {
        snprintf(hostDisplay, sizeof(hostDisplay), "%.18s...", g_machine.host.c_str());
    } else {
        snprintf(hostDisplay, sizeof(hostDisplay), "%s", g_machine.host.c_str());
    }
    // System Info
    String ip = WiFi.localIP().toString();
    char sysInfoStr[150];
    snprintf(sysInfoStr, sizeof(sysInfoStr), "%s - Station #%d\nIP:%s\n%s - %dms\nBitrate: %dkBs", dateStr,
             g_machine.currentStation, ip.c_str(), hostDisplay, g_machine.latestPingTime, g_machine.bitratekBs);

    // --- Draw Text ---
    // Draw Date on the left
    g_machine.tft->setCursor(1, bar_y + 12);
    g_machine.tft->print(sysInfoStr);

    // --- Cleanup ---
    g_machine.tft->unloadFont();
    SCREENLOCK = false;
}

// BUTTON DRAWING (reusable)
void drawButton(int x, int y, int w, int h, const char *label, bool selected) {
    uint16_t bg_color = selected ? TFT_BLUE : TFT_DARKGREY;
    uint16_t text_color = TFT_WHITE;

    g_machine.tft->fillRoundRect(x, y, w, h, 5, bg_color);
    g_machine.tft->drawRoundRect(x, y, w, h, 5, TFT_BLACK);

    g_machine.tft->setCursor(x + (w - strlen(label) * 12) / 2, y + h / 2 - 8);
    g_machine.tft->setTextSize(2);
    g_machine.tft->print(label);
}

void updateNowPlayingText(const char *info) {
    if (g_machine.screenDisplayed != 0) return;
    while (SCREENLOCK) delay(2);
    SCREENLOCK = true;
    const Rect r = g_machine.layout.now_playing_text_area;

    int lineLength = 24;
    int arrayLengthFirstLine = lineLength + 1;
    char firstLine[arrayLengthFirstLine] = {0}; // 14 chars + null terminator
    strncpy(firstLine, info, lineLength);

    // Clear area
    g_machine.tft->fillRect(r.x, r.y, r.w, r.h, TFT_BLACK);
    g_machine.tft->setFreeFont(&Orbitron_Medium_14);
    g_machine.tft->setTextSize(1);
    g_machine.tft->drawString(firstLine, r.x, r.y);
    if (strlen(info) > lineLength) {
        char secondLine[arrayLengthFirstLine] = {0};
        strncpy(secondLine, info + lineLength, lineLength);
        g_machine.tft->drawString(secondLine, r.x, r.y + 16); // 16 pixels down
    }

    g_machine.tft->unloadFont();

    SCREENLOCK = false;
}

void updateNowPlayingTitle(const char *info) {
    if (g_machine.screenDisplayed != 0) return;
    g_machine.nowPlayingText = info;
    while (SCREENLOCK) delay(2);
    SCREENLOCK = true;
    const Rect r = g_machine.layout.now_playing_title_area;
    const int max_chars = 18; // Adjust based on your text size and screen width

    // Clear area
    g_machine.tft->fillRect(r.x, r.y, r.w, r.h, TFT_BLACK);
    g_machine.tft->setTextSize(1);
    g_machine.tft->setFreeFont(&Orbitron_Medium_18);
    // Truncate long text and add ellipsis
    if (strlen(info) > max_chars) {
        char truncated[20];
        strncpy(truncated, info, max_chars - 3);
        truncated[max_chars - 3] = '\0';
        strcat(truncated, "...");
        g_machine.tft->drawString(info, r.x, r.y);
    } else {
        g_machine.tft->drawString(info, r.x, r.y);
    }
    g_machine.tft->unloadFont();
    SCREENLOCK = false;
}
