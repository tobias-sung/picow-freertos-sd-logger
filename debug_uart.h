#ifndef DEBUG_UART_H
#define DEBUG_UART_H

#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t sd_writeTaskHandle;

//Initialize UART PIO (outputs to pio_tx_pin)
void debug_uart_init(int pio_tx_pin);

//Print debug message, outputs to GPIO pin chosen in debug_uart_init
void debug_print(const char* fmt, ...);

//Task that initializes SD Card Reader, and writes messages to file on receiving notification from debug_print()
void vSdWriteTask();

#endif