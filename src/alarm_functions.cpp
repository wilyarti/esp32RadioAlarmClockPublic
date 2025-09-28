//
// Created by wilyarti on 13/9/25.
//

#include "alarm_functions.h"

// Global variables are now in g_machine.
// Their initial values should be set in setup().

void incrementAlarmHours() {
    g_machine.alarm_hours += 1;
    if (g_machine.alarm_hours >= 24) {
        g_machine.alarm_hours = 0;
    }
}
void incrementAlarmMinutes() {
    g_machine.alarm_minutes += 1;
    if (g_machine.alarm_minutes >= 60 ){
        g_machine.alarm_minutes = 0;
        incrementAlarmHours();
    }
}
