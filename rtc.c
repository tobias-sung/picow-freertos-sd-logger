#include "rtc.h"

#include "time.h"
#include <stdio.h>
#include <string.h>

#include "debug_uart.h"

#define RTC_SDA 14
#define RTC_SCL 15

#define RTC_I2C i2c1

#define TIMEZONE_OFFSET 28800 //HK Timezone

ds3231_t ds3231;

//Default time for Real-Time clock on startup
ds3231_data_t ds3231_data = {
        .seconds = 25,
        .minutes = 23,
        .hours = 23,
        .day = 4,
        .date = 10,
        .month = 8,
        .year = 23,
        .century = 1,
        .am_pm = false
};

//Character array for storing timestamp string
char timestamp_str[128];

char* rtc_get_timestamp_string(){
    memset(timestamp_str, 0, 128);
    //Read the current time
    if(ds3231_read_current_time(&ds3231, &ds3231_data)) {
        sprintf(timestamp_str, "Time unknown    ");
    } else {
        sprintf(timestamp_str, "%02u:%02u:%02u %02u/%02u/20%02u    ", 
            ds3231_data.hours, ds3231_data.minutes, ds3231_data.seconds, 
            ds3231_data.date, ds3231_data.month, ds3231_data.year);
    }
    return timestamp_str;
}

char* rtc_get_simple_timestamp_string(){
    memset(timestamp_str,0,128);
    if(ds3231_read_current_time(&ds3231, &ds3231_data)) {
        sprintf(timestamp_str, "Time unknown    ");
    } else {
        sprintf(timestamp_str, "%02u.%02u_%02u%02u%02u", 
            ds3231_data.hours, ds3231_data.minutes, 
            ds3231_data.date, ds3231_data.month, ds3231_data.year);
    }
    return timestamp_str;
}

void rtc_init(){
    
    debug_print("Intializing Ds3231 RTC\r\n");
    ds3231_init(&ds3231, RTC_I2C, DS3231_DEVICE_ADRESS, AT24C32_EEPROM_ADRESS_0);

    /* Initiliaze I2C line. */
    gpio_init(RTC_SDA);
    gpio_init(RTC_SCL);
    gpio_set_function(RTC_SDA, GPIO_FUNC_I2C);
    gpio_set_function(RTC_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(RTC_SDA);
    gpio_pull_up(RTC_SCL);
    i2c_init(ds3231.i2c, 400 * 1000);

    /* Update the DS3231 time registers with the desired time*/
    ds3231_configure_time(&ds3231, &ds3231_data);
    
    debug_print("Initialized Ds3231 RTC!\r\n");

}


