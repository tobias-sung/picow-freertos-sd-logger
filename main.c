
#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

//SD Card Reader Depndencies
#include "ff_headers.h"
#include "ff_sddisk.h"
#include "ff_stdio.h"
#include "ff_utils.h"

#include "hw_config.h"

//UART PIO Header file
#include "debug_uart.h"

#include <ctype.h>

#include "globals.h"

TaskHandle_t sd_taskHandle = NULL;
TaskHandle_t debug_taskHandle = NULL;

int startup_done = 0;
int rtc_initialized = 0;

//Simple task that prints out debug messages every 250 milliseconds
void vDbPrintTask() {
    int counter = 0;
    for (;;) {
        debug_print("%d: This a debug message.\r\n", counter);
        counter++;
        vTaskDelay(250/portTICK_PERIOD_MS);
        debug_print("%d: This is another debug message.\r\n", counter);
        counter++;
        vTaskDelay(250/portTICK_PERIOD_MS);
    }
}

void main() {
    stdio_init_all();

    //Initialize PIO UART output
    debug_uart_init(3);

    debug_print("Hello world!\r\n");

    debug_print("Initializing RTC...\r\n");

    //Initialize the Real-Time Clock
    rtc_init();

    rtc_initialized = 1;

    debug_print("Initialized RTC!\r\n");

    //Print out some startup debug messages
    debug_print("Doing some startup stuff...\r\n");
    sleep_ms(1000);
    debug_print("Doing some more startup stuff...\r\n");
    sleep_ms(1000);

    xTaskCreate(vDbPrintTask, "Debug Message Print Task", 512, NULL, 1, &debug_taskHandle);
    xTaskCreate(vSdWriteTask, "SD Write Task", 1024, NULL, 1, &sd_writeTaskHandle);
    vTaskStartScheduler();
}
