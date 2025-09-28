#include "globals.h"

// Global instance of the machine state.
MachineState g_machine;

void setupLayout() {
    // Now Playing buttons
    g_machine.layout.btn_next = {180, 280, 40, 40};
    g_machine.layout.btn_play_pause = {0, 280, 40, 40};
    g_machine.layout.btn_volume = {80, 280, 40, 40};

    // Alarm setting buttons
    g_machine.layout.btn_alarm_hour = {180, 280, 40, 40};
    g_machine.layout.btn_alarm_min = {0, 280, 80, 40};
    g_machine.layout.btn_alarm_toggle = {80, 280, 40, 40};

    // Clock
    g_machine.layout.clock_y = 120;
    g_machine.layout.clock_height = 60;
    g_machine.layout.clock_time_pos = {0, g_machine.layout.clock_y - 30};
    g_machine.layout.clock_seconds_pos = {180, g_machine.layout.clock_y + 30};
    g_machine.layout.clock_seconds_clear = {180, g_machine.layout.clock_y + 30, 60, 40};
    g_machine.layout.alarm_clock_clear_area = {0, g_machine.layout.clock_y, 320, g_machine.layout.clock_height};
    g_machine.layout.alarm_clock_cursor = {20, g_machine.layout.clock_y};

    // Now Playing text
    g_machine.layout.now_playing_title_area = {0, 20, 320, 20};
    g_machine.layout.now_playing_text_area = {0, 40, 320, 40};

    // Info text (RSSI, Volume)
    g_machine.layout.info_rssi_area = {160, 220, 160, 20};
    g_machine.layout.info_rssi_cursor = {160, 220};

    g_machine.layout.info_graph_area = {30, 210, 210, 20};
    g_machine.layout.info_graph_cursor = {0, 220};

    // Graphs
    g_machine.layout.graph_origin = {0, 220};
    g_machine.layout.graph_ram_y_offset = 0;
    g_machine.layout.graph_volume_y_offset = 12;
    g_machine.layout.graph_ram_label_clear_area = {0, 70, 140, 40};
    g_machine.layout.alarm_clock_clear_area = {0, 160, 140, 40};

    g_machine.layout.graph_ram_label_cursor = {0, 200};
    g_machine.layout.graph_ram_free_cursor = {160, 30 + g_machine.layout.graph_origin.y};
    g_machine.layout.graph_playtime_cursor = {120, 40 + g_machine.layout.graph_origin.y};
    g_machine.layout.graph_rssi_cursor = {0, 30 + g_machine.layout.graph_origin.y};
    g_machine.layout.graph_volume_cursor = {0, 40 + g_machine.layout.graph_origin.y};

    // SD Card
    g_machine.sdcard = false;

}
