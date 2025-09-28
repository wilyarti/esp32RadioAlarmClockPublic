//
// Created by wilyarti on 14/9/25.
//

#include "graphing_functions.h"
#include <TFT_eSPI.h>
#include <SPI.h>

#include "../lib/TFT_eSPI/TFT_eSPI.h"

// Meter colour schemes
#define GREEN2RED   4
#define RED2GREEN   5
#define RAINBOW     6
#define BLUETORED 3
// #########################################################################
//  Draw the linear meter
// #########################################################################
void linearMeter(int val, int x, int y, int w, int h, int g, int n, byte s) {
    int colour = TFT_BLUE;
    for (int b = 1; b <= n; b++) {
        if (val > 0 && b <= val) {
            // Fill in coloured blocks
            switch (s) {
                case 0: colour = TFT_RED;
                    break; // Fixed colour
                case 1: colour = TFT_GREEN;
                    break; // Fixed colour
                case 2: colour = TFT_BLUE;
                    break; // Fixed colour
                case 3: colour = rainbowColor(map(b, 0, n, 127, 0));
                    break; // Blue to red
                case 4: colour = rainbowColor(map(b, 0, n, 63, 0));
                    break; // Green to red
                case 5: colour = rainbowColor(map(b, 0, n, 0, 63));
                    break; // Red to green
                case 6: colour = rainbowColor(map(b, 0, n, 0, 159));
                    break; // Rainbow (red to violet)
            }
            g_machine.tft->fillRect(x + b * (w + g), y, w, h, colour);
        } else // Fill in blank segments
        {
            g_machine.tft->fillRect(x + b * (w + g), y, w, h, TFT_DARKGREY);
        }
    }
}

// #########################################################################
//  Draw the wifi meter
// #########################################################################
void wifiMeter(int val, int x, int y, int w, int h, int g, int n, byte s) {
    int colour = TFT_BLUE;
    for (int b = 1; b <= n; b++) {
        int hieghtMultiplier = 2;
        int segmentHeight = h + (b * hieghtMultiplier); // This bar's height
        int segmentY = y + (h + (n * hieghtMultiplier)) - segmentHeight; // Adjust Y so bottom aligns

        if (val > 0 && b <= val) {
            // Fill in coloured blocks
            switch (s) {
                case 0: colour = TFT_RED;
                    break;
                case 1: colour = TFT_GREEN;
                    break;
                case 2: colour = TFT_BLUE;
                    break;
                case 3: colour = rainbowColor(map(b, 0, n, 127, 0));
                    break;
                case 4: colour = rainbowColor(map(b, 0, n, 63, 0));
                    break;
                case 5: colour = rainbowColor(map(b, 0, n, 0, 63));
                    break;
                case 6: colour = rainbowColor(map(b, 0, n, 0, 159));
                    break;
            }
            g_machine.tft->fillRect(x + b * (w + g), segmentY, w, segmentHeight, colour);
        } else // Fill in blank segments
        {
            g_machine.tft->fillRect(x + b * (w + g), segmentY, w, segmentHeight, TFT_DARKGREY);
        }
    }
}

// Display current memory information
void drawGraphicalComponents() {
    const Rect graphRect = g_machine.layout.info_graph_area;
    const Point graphCursor = g_machine.layout.info_graph_cursor;

    const int graphSpacingY = 15;
    // ------- Volume ------ //
    int volumePercentage = map(g_machine.audio->getVolume(), 21, 0, 0, 100);
    int usagePercent = 100 - volumePercentage;
    uint32_t freeBytes = ESP.getFreeHeap();

    // Map to 0-20 scale for the meter (20 segments)
    int totalWidth = g_machine.tft->width() - (2 * g_machine.layout.graph_origin.x); // Available width
    int reading = map(usagePercent, 0, 100, 0, 20);
    int segmentWidth = (totalWidth - (19 * 3)) / 20; // Calculate segment width (19 gaps between 20 segments)
    segmentWidth = constrain(segmentWidth, 3, 10); // Keep reasonable size

    // Draw Graph
    linearMeter(reading, graphRect.x, graphRect.y, segmentWidth, 12, 3, 20,GREEN2RED);
    // ------- Buffer ------ //
    // ------ Draw Buffer Filling Graph ----- //
    float bufferLatency = 0;
    if (g_machine.audioIsPlaying) {
        Audio::BufferInfo bufferInfo = g_machine.audio->getBufferInfo();
        bufferLatency = bufferInfo.latencySeconds;
        g_machine.bufferLengthInSeconds = bufferInfo.latencySeconds;
    }
    int latencyReading = map(
        constrain(bufferLatency, 0, g_machine.bufferTarget), 0, g_machine.bufferTarget * 1.5, 0, 20);
    // Draw Graph
    linearMeter(latencyReading, graphRect.x, graphRect.y + graphSpacingY, segmentWidth, 12, 3, 20,RED2GREEN);

    // ------- Power ------ //
    // ------ Draw Power Use Filling Graph ----- //
    float totalMw = 0;
    for (int i = 0; i < 12; i++) {
        totalMw += g_machine.mWpowerReadings[i];
    }
    float power = totalMw / 12;
    int powerReading = map(constrain(power, 0, g_machine.maxPowerLevelmW), 0, g_machine.maxPowerLevelmW, 0, 20);

    g_machine.avgPwrmW = power;
    // Draw Graph
    linearMeter(powerReading, graphRect.x, graphRect.y + (graphSpacingY * 2), segmentWidth, 12, 3, 20,BLUETORED);

    // ------- Volume ------ //
    int color = rainbowColor(map(g_machine.audio->getVolume(), 0, 21, 0, 63));
    g_machine.tft->fillRect(0, graphRect.y, graphRect.x+10, graphSpacingY*3, TFT_BLACK);
    g_machine.tft->setTextSize(1); // Use size 1 for more text
    g_machine.tft->setFreeFont(&DejaVu_Sans_Mono_12);
    g_machine.tft->setCursor(graphCursor.x, graphCursor.y);
    g_machine.tft->setTextColor(color, TFT_BLACK);
    g_machine.tft->printf("%d", g_machine.volume);

    // ------- Buffer ------ //
    color = rainbowColor(map(g_machine.bufferLengthInSeconds, 0, g_machine.bufferTarget, 0, 63));
    g_machine.tft->setTextColor(color, TFT_BLACK);
    g_machine.tft->setCursor(graphCursor.x, graphCursor.y + graphSpacingY);
    g_machine.tft->printf("%d", g_machine.bufferLengthInSeconds);

    // ------- Power ------ //
    color = rainbowColor(map(g_machine.avgPwrmW, 0, g_machine.maxPowerLevelmW, 127, 0));
    g_machine.tft->setTextColor(color, TFT_BLACK);
    g_machine.tft->setCursor(graphCursor.x, graphCursor.y + (graphSpacingY * 2));
    g_machine.tft->printf("%.0f", power);
    g_machine.tft->unloadFont();
}

void drawLowBattery(int x, int y, int width, int height) {
    float voltage = g_machine.current_sensor->getBusVoltage_mV() / 1000.0;

    // LiPo voltage to percentage (typical 3.7V nominal, 4.2V full, 3.2V empty)
    int percent;
    if (voltage >= 4.2) percent = 100;
    else if (voltage <= 3.2) percent = 0;
    else percent = map(voltage * 100, 320, 420, 0, 100); // 3.2V-4.2V range

    percent = constrain(percent, 0, 100);

    // Battery outline
    g_machine.tft->fillRect(x, y, width + 40, height, TFT_BLACK);
    g_machine.tft->drawRect(x, y, width, height, TFT_WHITE);

    // Battery tip
    int tipWidth = width / 6;
    int tipHeight = height / 3;
    g_machine.tft->fillRect(x + width, y + (height - tipHeight) / 2, tipWidth, tipHeight, TFT_WHITE);

    // Battery fill with color gradient
    int fillWidth = (width - 2) * percent / 100;
    if (fillWidth > 0) {
        // Use RED2GREEN gradient (case 5 from your wifiMeter function)
        uint16_t color = rainbowColor(map(percent, 0, 100, 0, 63));
        g_machine.tft->fillRect(x + 1, y + 1, fillWidth, height - 2, color);
    }

    // Voltage text
    g_machine.tft->setTextColor(TFT_WHITE);
    g_machine.tft->setTextSize(1);
    g_machine.tft->setCursor(x + width + tipWidth + 6, y + 10);
    g_machine.tft->printf("%d%", percent);
}

void displayRSSILevel() {
    int rssi = WiFi.RSSI();

    // Map RSSI to 0-5 scale (5 bars)
    // Typical RSSI range: -30 (excellent) to -90 (poor)
    int bars = map(rssi, -90, -30, 0, 5);
    bars = constrain(bars, 0, 5); // Ensure within 0-5 range

    //Serial.printf("RSSI: %d dBm, Bars: %d\n", rssi, bars);

    int segmentWidth = (80 - ((5 - 1) * 3)) / 5;
    segmentWidth = constrain(segmentWidth, 3, 10);

    // Draw the meter
    wifiMeter(bars, 240 - 60, 0, segmentWidth, 2, 3, 5, RED2GREEN);

    // g_machine.tft->setTextSize(1);
    // g_machine.tft->setCursor(240 - 70, 0);
    // g_machine.tft->printf("%d", rssi);
}


/***************************************************************************************
** Function name:           rainbowColor
** Description:             Return a 16 bit rainbow colour
***************************************************************************************/
uint16_t rainbowColor(uint8_t spectrum) {
    spectrum = spectrum % 192;

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;

    uint8_t sector = spectrum >> 5;
    uint8_t amplit = spectrum & 0x1F;

    switch (sector) {
        case 0:
            red = 0x1F;
            green = amplit;
            blue = 0;
            break;
        case 1:
            red = 0x1F - amplit;
            green = 0x1F;
            blue = 0;
            break;
        case 2:
            red = 0;
            green = 0x1F;
            blue = amplit;
            break;
        case 3:
            red = 0;
            green = 0x1F - amplit;
            blue = 0x1F;
            break;
        case 4:
            red = amplit;
            green = 0;
            blue = 0x1F;
            break;
        case 5:
            red = 0x1F;
            green = 0;
            blue = 0x1F - amplit;
            break;
    }

    return red << 11 | green << 6 | blue;
}

String formatPlayingTime(uint32_t milliseconds) {
    uint32_t totalSeconds = milliseconds / 1000;
    uint32_t hours = totalSeconds / 3600;
    uint32_t minutes = (totalSeconds % 3600) / 60;
    uint32_t seconds = totalSeconds % 60;

    char timeStr[10];
    sprintf(timeStr, "%02d:%02d:%02d", hours, minutes, seconds);
    return String(timeStr);
}
