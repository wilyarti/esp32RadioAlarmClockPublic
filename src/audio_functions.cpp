#include "audio_functions.h"


// Check if PSRAM is available
bool psramAvailable() {
#if defined(ESP32) && defined(CONFIG_SPIRAM_SUPPORT)
    return true;
#else
    return false;
#endif
}

char *currentAudioUrl = nullptr;

void audio_setup() {
    g_machine.audio->setPinout(BCK_PIN, WS_PIN, DATA_PIN);
    g_machine.audio->setVolume(g_machine.volume);
    // Set buffer size based on PSRAM availability
    if (psramAvailable()) {
        // Larger buffers when PSRAM is available
        g_machine.audio->setBufsize(-1, 1024 * 512);
        Serial.println("PSRAM detected - using larger audio buffers");
    } else {
        // Default buffer sizes
        g_machine.audio->setBufsize(4 * 1024, -1);
        Serial.println("No PSRAM - using default audio buffers");
    }
    g_machine.audioIsPlaying = false;
    g_machine.bufferLengthInSeconds = 0;
}

void startRadio() {
    String url;
    if (g_machine.currentStation == -1) {
        url = g_machine.currentStream;
    } else {
        url = g_machine.radioStations[g_machine.currentStation];
    }
    Serial.print("Playing: ");
    Serial.println(url);
    connectToStream(url);
}

void restoreRadioOnBoot() {
    startRadio();
}

// This function is blind and doesn't know if its playing a URL in the array or a foreign one.
// So you need to set that in the calling function.
void connectToStream(String url) {
    if (g_machine.audioIsPlaying) {
        Serial.println("Stopping current playback...");
        g_machine.audio->stopSong(); // <- CRITICAL: Stop the current stream first!
        delay(250);
    }
    Serial.println("Connecting to new stream...");
    g_machine.audio->connecttohost(url.c_str()); // Connect to the new URL
    Serial.println("Stream changed successfully!");
    g_machine.audioIsPlaying = true;
    drawStatusBar();
}

// Optional: Audio status information
void audio_info(const char *info) {
    Serial.print("audio_info: ");
    Serial.println(info);

    static unsigned long lastIncrease = 0;
    if (strstr(info, "slow stream") || strstr(info, "dropout")) {
        const int text_position_y = 280;
        const int text_area_h = 20;

        // Clear area
        g_machine.tft->fillRect(0, text_position_y, 320, text_area_h, ILI9341_BLACK);
        g_machine.tft->setCursor(0, text_position_y);
        g_machine.tft->setTextSize(1);
        g_machine.tft->setTextColor(ILI9341_RED);
        g_machine.tft->println(info);
        g_machine.tft->setTextColor(ILI9341_WHITE);
    }
}

// Set struct and preferences
void setAudioVolume(int volume) {
    if (volume < 0) return;
    if (volume > 21) return;
    g_machine.volume = volume;
    g_machine.audio->setVolume(g_machine.volume);
    preferences.begin("Preferences", false);
    preferences.putInt("volume", g_machine.volume);
    preferences.end();
}

void audio_showstreamtitle(const char *info) {
    g_machine.nowPlayingText = info; // Keep original info without prefix
    if (g_machine.recieveNowPlayingUpdates) {
        // Send message with prefix for Telegram
        String telegramMessage = "Now playing: " + String(info);
        g_machine.bot->sendMessage(g_machine.telegramChatID, telegramMessage.c_str(), "HTML");
        Serial.println(telegramMessage.c_str());
    }
    updateNowPlayingText(info);
}

void audio_showstation(const char *info) {
    updateNowPlayingTitle(info);
}

bool isAudioPlaying() {
    return g_machine.audio->isRunning();
}

void stopRadio() {
    g_machine.audio->stopSong();
    g_machine.loopCount = 16;
    g_machine.audioIsPlaying = false;
}

void nextStation() {
    g_machine.currentStation = g_machine.currentStation + 1;
    if (g_machine.currentStation >= 8) {
        // radioStations array has 8 elements
        g_machine.currentStation = 0; // loop around
    }
    String url = g_machine.radioStations[g_machine.currentStation];
    Serial.print("Playing: ");
    Serial.println(url);
    connectToStream(url);
    setAudioVolume(8);
    preferences.begin("Preferences", false);
    preferences.putString("currentUrl", url);
    preferences.putInt("currentStation", g_machine.currentStation);
    preferences.end();
}

void audio_id3data(const char *info) {
    //id3 metadata
    Serial.print("id3data     ");
    Serial.println(info);
}

void audio_eof_mp3(const char *info) {
    //end of file
    Serial.print("eof_mp3     ");
    g_machine.nowPlayingText  = "Stopped. EOF.";
    Serial.println(info);
}


void audio_bitrate(const char *info) {
    Serial.print("/*********************** ---- ************************/\nbitrate: ");

    // Check for null pointer or empty string
    if (info == nullptr || strlen(info) == 0) {
        Serial.println("Invalid bitrate info");
        g_machine.bitratekBs = 0;
        return;
    }

    Serial.print("Raw info: '");
    Serial.print(info);
    Serial.println("'");

    // Convert with proper error checking
    char *endptr;
    long bitrate = strtol(info, &endptr, 10);

    // Check if conversion was successful
    if (endptr == info || *endptr != '\0') {
        Serial.println("Failed to convert bitrate");
        g_machine.bitratekBs = 0;
        return;
    }

    // Check for reasonable bitrate range (e.g., 32kbps to 320kbps)
    if (bitrate < 32000 || bitrate > 320000) {
        Serial.print("Bitrate out of range: ");
        Serial.println(bitrate);
        // You might still want to use it, or set to 0
    }

    g_machine.bitratekBs = bitrate / 1000;  // Use 1000 for kbps, not 1024
    Serial.print("Converted: ");
    Serial.print(g_machine.bitratekBs);
    Serial.println(" kbps");
}

void audio_commercial(const char *info) {
    //duration in sec
    Serial.print("commercial  ");
    Serial.println(info);
}

void audio_icyurl(const char *info) {
    //homepage
    Serial.print("icyurl      ");
    Serial.println(info);
}

void audio_lasthost(const char *info) {
    //stream URL played
    Serial.print("lasthost    ");
    Serial.println(info);
}

void audio_eof_speech(const char *info) {
    Serial.print("eof_speech  ");
    Serial.println(info);
}
