#include "main.h"

int sleepTime = 15;
unsigned long curTime;

// ====== Time Keeping =====
long timeZone = 10;
static WiFiUDP ntpUDP;

// ====== Telegram ======
static WiFiClientSecure secureClient;
unsigned long botLastCheck = 0;
const unsigned long botInterval = 2000; // ms

// ======= Hardware Polling and Alarm Check =====
unsigned long lastLoop = 0;
const unsigned long loopControlInterval = 16; // 30 fps
// SPIClass hspi(HSPI);

unsigned long alarmCheck = 0;
const unsigned long alarmCheckInterval = 45000; // 15 secs
unsigned long lastLoopControllerIntervalDecrement = 0;
unsigned long lastLoopControllerIntervalIncrement = 0;

// ====== Buttons ======
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {BUTTON_A, BUTTON_B, BUTTON_C};
Bounce *buttons = new Bounce[NUM_BUTTONS];
bool ramMonitorDuringBoot = false;
bool slowBoot = false;
int LOOP_TARGET = 150;
unsigned long lastPingCheck = 0;

unsigned long screenshotTimer = millis();

void setup() {
    Serial.begin(9600);
    Serial.println("/*** Init Serial ***/");
    if (slowBoot) delay(1000);

    /*** Init TFT ***/
    g_machine.tft = new TFT_eSPI();

    //
    Wire.begin(SDA,SCL);
    g_machine.current_sensor = new INA219(0x40, &Wire); // &Wire is optional
    g_machine.current_sensor->setMaxCurrentShunt(5, 0.1);


    setupDisplay();
    Serial.println("/*** Setup Display ***/");
    if (slowBoot) delay(1000);

    // Load WiFi networks
    loadWiFi();
    // WiFi setup
    if (connectWiFi() == false) {
        setupNetworkViaInput();
    }
    //setupNetworkViaInput(); // Delete this -- testing
    WiFi.setSleep(false);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    //WiFi.setRxBufferSize(1024); // ✅ FIXED
    secureClient.setInsecure();
    //secureClient.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org

    // Initialize state
    loadParamatersOnBoot();
    loadTelegram();
    Serial.println("/*** Loaded Paramaters ***/");
    if (slowBoot) delay(1000);

    setupLayout();
    Serial.println("/*** Setup Layout ***/");
    if (slowBoot) delay(1000);


    g_machine.bot = new UniversalTelegramBot(g_machine.telegramToken, secureClient);
    int numNewMessages = g_machine.bot->getUpdates(g_machine.bot->last_message_received + 1);
    while (numNewMessages) {
        Serial.println("Emptying message queue.");
        numNewMessages = g_machine.bot->getUpdates(g_machine.bot->last_message_received + 1);
    }
    if (slowBoot) delay(1000);

    g_machine.timeClient = new NTPClient(ntpUDP, "pool.ntp.org", g_machine.timeZone * 3600, 60000);
    g_machine.timeClient->begin();
    g_machine.timeClient->setTimeOffset(g_machine.timeZone * 3600L);
    g_machine.timeClient->update();
    Serial.println("/*** TimeZone Setup ***/");

    // Buttons
    for (int i = 0; i < NUM_BUTTONS; i++) {
        buttons[i].attach(BUTTON_PINS[i], INPUT_PULLUP);
        buttons[i].interval(25);
    }
    Serial.println("/*** Button Interups Set ***/");

    // ✅ INITIALIZE AUDIO FIRST!
    Serial.println("Before audio_setup");
    g_machine.audio = new Audio();
    if (!g_machine.audio) {
        Serial.println("❌ Failed to allocate Audio object!");
        while (1);
    }
    audio_setup();
    Serial.println("/*** Audio Setup ***/");

    // ✅ THEN restore radio
    restoreRadioOnBoot();

    // ✅ THEN create tasks with adequate stack
    xTaskCreatePinnedToCore(
        telegramTask, "TelegramTask", 8192, NULL, 1, NULL, 0
    );

    xTaskCreatePinnedToCore(
        timeUpdateTask, "TimeUpdateTask", 2048, NULL, 2, NULL, 0
    );

    Serial.println("/*** Secondary Tasks Launched ***/");

    // hspi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    //
    // // Initialize SD card
    // if (!SD.begin(SD_CS, hspi)) {
    //     Serial.println("SD Card Mount Failed");
    //     return;
    // }

    g_machine.loopCount = 0;
    g_machine.loopCheck = millis();
    g_machine.bufferFilled = false;
    g_machine.bufferLowerThreshold = 5; // seconds
    g_machine.bufferLengthsIndex = 0;
    g_machine.bufferIndexCheck = 0;
    for (int i = 0; i < 12; i++) {
        g_machine.bufferLengths[i] = 0;
        g_machine.mWpowerReadings[i] = 0;
    }
    g_machine.mWpowerReadingsIndex = 0;
    g_machine.avgPwrmW = 0;
    g_machine.volumeModifiedTime = millis();

}

float getBufferSlope() {
    float sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;

    for (int i = 0; i < 12; i++) {
        sumX += i; // x = time index (0 to 11)
        sumY += g_machine.bufferLengths[i]; // y = buffer values
        sumXY += i * g_machine.bufferLengths[i];
        sumX2 += i * i;
    }

    // Slope formula: (n*Σxy - ΣxΣy) / (n*Σx² - (Σx)²)
    return (12 * sumXY - sumX * sumY) / (12 * sumX2 - sumX * sumX);
}

void loop() {
    g_machine.loopCount++; // increment loop counter
    g_machine.audio->loop(); // run audio tasks
    // TODO - is this worth keeping in? I just use it for development.
    // if (millis() - screenshotTimer > 5000) {
    //     Serial.println("Screenshot Timer");
    //     int result = serialScreenServer("screenshot.bmp");
    //     screenshotTimer =millis();
    // }

    // Check alarm periodically
    if (millis() - alarmCheck > alarmCheckInterval) {
        checkAlarm();
        alarmCheck = millis();
    }

    // Update buttons, screen etc
    if (millis() - lastLoop > loopControlInterval) {
        buttonCheck();
        checkSerial();
        displayLoop();
        managePower();
        lastLoop = millis();
    }

    int loopMillis = 250;
    if (g_machine.bufferFilled) {
        loopMillis = 5000; // 5 second sample for the case of bufferfilled
    }
    // Buffer checker array
    if (millis() - g_machine.bufferIndexCheck > loopMillis) {
        g_machine.bufferLengths[g_machine.bufferLengthsIndex] = g_machine.bufferLengthInSeconds;
        g_machine.bufferLengthsIndex += 1;
        if (g_machine.bufferLengthsIndex >= 12) g_machine.bufferLengthsIndex = 0;
        g_machine.bufferIndexCheck = millis();
    }

    int loopMillismW = 80;
    // Power reading sampler
    if (millis() - g_machine.mWpowerReadingsMillis > loopMillis) {
        g_machine.mWpowerReadings[g_machine.mWpowerReadingsIndex] = g_machine.current_sensor->getPower_mW();
        g_machine.mWpowerReadingsIndex += 1;
        if (g_machine.mWpowerReadingsIndex == 12) {
            g_machine.mWpowerReadingsIndex = 0;
        }
        g_machine.mWpowerReadingsMillis = millis();
    }
    // Check every second
    if (millis() - g_machine.loopCheck > 1000) {
        float slope = getBufferSlope();
        /** Two conditional branches.
         * 1. bufferFilled target hasn't been achieved
         * 2. bufferFilled has been achieved and is being depleted or used
         * */
        // Buffer filled once
        if (g_machine.bufferFilled) {
            if (g_machine.bufferLengthInSeconds > 5) {
                vTaskDelay(g_machine.loopRetarder);
                if (slope > 0) {
                    //Serial.printf("Slope was negative: %f.2 \n", slope);
                    g_machine.loopRetarder -= 1;
                } else {
                    //Serial.printf("Slope was positive: %f.2 \n", slope);
                    g_machine.loopRetarder += 1;
                }
            } else {
                g_machine.bufferFilled = false; // reset buffer
            }
        }
        // Buffer not yet filled
        if (!g_machine.bufferFilled) {
            if (g_machine.bufferLengthInSeconds < g_machine.bufferTarget) {
                vTaskDelay(g_machine.loopRetarderShortBuffer);
                if (slope > 0.25) {
                    //Serial.printf("Slope was negative: %f.2 \n", slope);
                    g_machine.loopRetarderShortBuffer -= 1;
                } else {
                    //Serial.printf("Slope was positive: %f.2 \n", slope);
                    g_machine.loopRetarderShortBuffer += 5;
                }
            }
        }


        // Serial.printf("Loop: %uHz, Buffer: %.1ds, Slope: %.2f Play buffer retarder: %d Short buffer retarder: %d \n",
        //               g_machine.loopCount, g_machine.bufferLengthInSeconds, slope, g_machine.loopRetarder, g_machine.loopRetarderShortBuffer);
        g_machine.loopCount = 0;
        g_machine.loopCheck = millis();
    }
}

// Helper function to get current volume (you might need to implement this)
uint8_t getAudioVolume() {
    // Return current volume level from your audio system
    return g_machine.audio->getVolume();
}

void managePower() {
    if (millis() - g_machine.volumeModifiedTime < 2000) {
        return; // only mess with volume every 2 seconds
    }
    if (g_machine.avgPwrmW > g_machine.maxPowerLevelmW) {
        int currentVolume = g_machine.volume;
        currentVolume -= 1;
        setAudioVolume(currentVolume);
        Serial.println("Reduced volume");
        g_machine.volumeModifiedTime = millis();
    }
}

void timeUpdateTask(void *pvParameters) {
    while (true) {
        g_machine.timeClient->update();
        vTaskDelay(30000 / portTICK_PERIOD_MS); // Update time every 30 seconds
    }
}

void telegramTask(void *pvParameters) {
    while (true) {
        try {
            int numNewMessages = g_machine.bot->getUpdates(g_machine.bot->last_message_received + 1);
            while (numNewMessages) {
                Serial.println("got response");
                handleNewMessages(numNewMessages);
                numNewMessages = g_machine.bot->getUpdates(g_machine.bot->last_message_received + 1);
                g_machine.telegramMessageFlag = true;
            }
        } catch (const std::exception &e) {
            Serial.printf("Telegram error: %s\n", e.what());
        }

        if (g_machine.audioIsPlaying && g_machine.currentStream.length() > 0) {
            String url = g_machine.currentStream;
            int hostStart = url.indexOf("://");

            if (hostStart > 0) {
                hostStart += 3; // Move past "://"
                int hostEnd = url.indexOf(':', hostStart);
                if (hostEnd == -1) {
                    hostEnd = url.indexOf('/', hostStart);
                }
                if (hostEnd == -1) {
                    hostEnd = url.length();
                }
                g_machine.host = url.substring(hostStart, hostEnd);

                if (Ping.ping(g_machine.host.c_str(), 5)) {
                    g_machine.newPingFlag = true;
                    g_machine.latestPingTime = Ping.averageTime();
                } else {
                    g_machine.latestPingTime = -1; // Indicates a failed ping
                }
            }
            //Serial.printf("Pinged %s: %dms\n", g_machine.host.c_str(), g_machine.latestPingTime);
        } else {
            g_machine.latestPingTime = 0; // Not playing, so ping is 0/NA
        }
        vTaskDelay(3000 / portTICK_PERIOD_MS); // Check every 2 seconds
    }
}

void buttonCheck() {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        // Update the Bounce instance :
        buttons[i].update();
        if (buttons[i].fell() && i == 0) {
            g_machine.btnState1 = HIGH;
        } else if (buttons[i].fell() && i == 1) {
            g_machine.btnState2 = HIGH;
        } else if (buttons[i].fell() && i == 2) {
            g_machine.btnState3 = HIGH;
        }
    }
}

void checkCpuUsage() {
    Serial.printf("CPU Frequency: %d MHz\n", getCpuFrequencyMhz());
    Serial.printf("CPU Usage: %.1f%%\n", 100.0 - (100.0 * getCpuFrequencyMhz() / 240));
}

void checkFullMemoryStatus() {
    Serial.println("\n--- MEMORY STATUS ---");

    // Internal RAM
    Serial.printf("Internal RAM:\n");
    Serial.printf("  Free: %6d kB\n", ESP.getFreeHeap() / 1024);
    Serial.printf("  Min Free: %6d kB\n", ESP.getMinFreeHeap() / 1024);
    Serial.printf("  Max Alloc: %6d kB\n", ESP.getMaxAllocHeap() / 1024);
    Serial.printf("  Total Size: %6d kB\n", ESP.getHeapSize() / 1024);

    // PSRAM
    if (psramFound()) {
        Serial.printf("PSRAM:\n");
        Serial.printf("  Free: %6d kB\n", ESP.getFreePsram() / 1024);
        Serial.printf("  Min Free: %6d kB\n", ESP.getMinFreePsram() / 1024);
        Serial.printf("  Total Size: %6d kB\n", ESP.getPsramSize() / 1024);
    } else {
        Serial.println("PSRAM: Not detected");
    }

    Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
    Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.println("---------------------");
}
