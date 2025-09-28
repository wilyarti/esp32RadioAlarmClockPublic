#include "wifi_functions.h"

#include "../lib/TFT_eSPI/TFT_eSPI.h"


bool connectWiFi() {
    for (int i = 0; i < MAX_NETWORKS; i++) {
        if (strlen(g_machine.WifiNetworks[i].ssid) == 0) continue; // skip empty slots
        Serial.print("Trying Wi-Fi: ");
        Serial.print(g_machine.WifiNetworks[i].ssid);
        Serial.print("    ");
        Serial.println(g_machine.WifiNetworks[i].password);
        WiFi.begin(g_machine.WifiNetworks[i].ssid, g_machine.WifiNetworks[i].password);

        g_machine.tft->setCursor(20, 200);
        g_machine.tft->printf("Connecting to %s", g_machine.WifiNetworks[i].ssid);
        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
            delay(200);
            g_machine.tft->drawBitmap(70, 50, wifi_1, 100, 100, TFT_WHITE);
            g_machine.tft->print(".");
            delay(200);
            g_machine.tft->drawBitmap(70, 50, wifi_2, 100, 100, TFT_WHITE);
            g_machine.tft->print(".");
            delay(200);
            g_machine.tft->drawBitmap(70, 50, wifi_3, 100, 100, TFT_WHITE);
            g_machine.tft->print(".");
            delay(200);
            g_machine.tft->fillRect(70, 50, 100, 100, TFT_BLACK);
            Serial.print(".");
            g_machine.tft->print(".");
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nConnected!");
            g_machine.tft->fillScreen(TFT_BLACK);
            return true;
        }
    }
    Serial.println("Failed to connect to any network");
    return false;
}
