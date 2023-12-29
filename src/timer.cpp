#include "timer.h"

#include "Globals.h"
#include "PeripheryManager.h"
#include "DisplayManager.h"

time_t now;
struct tm *datetime;

time_t timer_time()
{
    return now;
}

struct tm *timer_localtime()
{
    return datetime;
}

void timer_tick()
{
    static bool ringing = false;
    static time_t last_ring = 0;

    now = time(NULL);
    datetime = localtime(&now);

    /* power on screen in advance of alarm */
    if (datetime->tm_hour == (ALARM_HOUR - 1) && datetime->tm_min == ALARM_MINUTE)
        DisplayManager.setPower(true);

    if (ringing) {
        if (datetime->tm_hour != ALARM_HOUR || datetime->tm_min != ALARM_MINUTE ||
                PeripheryManager.buttonS->wasPressed()) {
            /* end of alarm */
            ringing = false;
            PeripheryManager.stopSound();
        } else if (!PeripheryManager.isPlaying()) {
            /* start alarm sound */
            PeripheryManager.playRTTTLString(MELODIES[ALARM_SOUND]);
        }
    } else {
        if (datetime->tm_hour == ALARM_HOUR && datetime->tm_min == ALARM_MINUTE &&
                now > (last_ring + 60) && ALARM_SOUND > 0) {
            /* alarm */
            ringing = true;
            last_ring = now;
            DisplayManager.setPower(true);
        }
    }
}
