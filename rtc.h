#ifndef RTC_H
#define RTC_H
#include "ds3231.h"

extern ds3231_t ds3231;

//Get current timestamp from RTC in string format
//Format: HH:MM:SS DD/MM/20YY
char* rtc_get_timestamp_string();

//Get a simplified current timestamp from RTC in sting format (used for debug log filename)
//Format: HH.MM_DDMMYY
char* rtc_get_simple_timestamp_string();

//Initialize the real-time clock and set default time
void rtc_init();

#endif