#include "hardware/pio.h"
#include "debug_uart.pio.h"
#include <ctype.h>
#include <stdarg.h> 

#include "debug_uart.h"

#include "ff_headers.h"
#include "ff_sddisk.h"
#include "ff_stdio.h"
#include "ff_utils.h"

#include "hw_config.h"

#include "globals.h"

PIO pio;
uint sm;
uint offset;

//Variable to track whether SD card was successfully intialized
int sd_detected = 0;

QueueHandle_t debug_queue = NULL;
char debug_buffer[512];

SemaphoreHandle_t debug_mutex;

TaskHandle_t sd_writeTaskHandle = NULL;

const uint SERIAL_BAUD = 115200;

static inline void stop() {
    fflush(stdout);
    __breakpoint();
}

void debug_uart_init(int pio_tx_pin){
    // This will find a free pio and state machine for our program and load it for us
    // We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
    // so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&debug_uart_program, &pio, &sm, &offset, pio_tx_pin, 1, true);
    hard_assert(success);

    debug_uart_program_init(pio, sm, offset, pio_tx_pin, SERIAL_BAUD);

    debug_queue = xQueueCreate(256, sizeof(char)*256);

    debug_mutex = xSemaphoreCreateMutex();
}

void debug_print(const char* fmt, ...){
    xSemaphoreTake(debug_mutex, portMAX_DELAY);

    {
        memset(debug_buffer, 0, sizeof(debug_buffer));

        char timestamp[50];
        memset(timestamp, 0, sizeof(timestamp));

        //If real-time clock is initialized, first write timestamp to start of buffer
        if (rtc_initialized){
            //Copy timestamp string into variable timestamp
            strcpy(timestamp, rtc_get_timestamp_string());
            sprintf(debug_buffer, timestamp);
        }

        //Write debug message to buffer (AFTER timestamp)
        va_list list;
        va_start(list, fmt);

        vsprintf(debug_buffer + strlen(timestamp), fmt, list);
            
        va_end(list);
            
        debug_uart_program_puts(pio, sm, debug_buffer);
        
        if (sd_detected != -1){
             //Send debug message to queue
            xQueueSendToBackFromISR(
                        debug_queue, 
                        (void*)&debug_buffer, 
                        NULL);
            
            //Notify SD Card Write Task to write to file, if it has started
            if (startup_done){
                xTaskNotify(sd_writeTaskHandle, 0, eNoAction);
            } 
        }
       
    }
    
    xSemaphoreGive(debug_mutex);
}


void vSdWriteTask(){
    FF_Disk_t* pxDisk = FF_SDDiskInit("sd0");
    if (pxDisk){
        sd_detected = 1;
        debug_print("Initialized disk.\r\n");
        configASSERT(pxDisk);
        FF_Error_t xError = FF_SDDiskMount(pxDisk);
        if (FF_isERR(xError) != pdFALSE) {
            stop();
        }
        debug_print("Mounted disk.\r\n");
        FF_FS_Add("/sd0", pxDisk);
        debug_print("Added file system.\r\n");
        debug_print("Initialization complete.\r\n");

        //Name the file with the following format: "/sd0/db_log_HH.MM_DDMMYY"
        char filename[60];
        memset(filename, 0, 60);
        strcat(filename, "/sd0/db_log_");
        strcat(filename, rtc_get_simple_timestamp_string());
        strcat(filename, ".txt");

        //Open file
        FF_FILE *pxFile;
        pxFile = ff_fopen(filename, "w");
        if (!pxFile) {
            //printf("ff_fopen failed: %s (%d)\n", strerror(stdioGET_ERRNO()),
            //stdioGET_ERRNO());
            stop();
        }

        //First, write all the debug messages that were saved before the SD Write Task started
        char buffer[256];
        //Read and empty the entire queue
        while(uxQueueMessagesWaiting(debug_queue) != 0){
            memset(buffer, 0, sizeof(buffer));
            
            xQueueReceive(
                debug_queue,
                &buffer,
                0
            );

            if (ff_fprintf(pxFile, buffer) < 0) {
                // printf("ff_fprintf failed: %s (%d)\n", strerror(stdioGET_ERRNO()),
                //        stdioGET_ERRNO());
                stop();
            }
        }
        if (-1 == ff_fclose(pxFile)) {
            //printf("ff_fclose failed: %s (%d)\n", strerror(stdioGET_ERRNO()),
            //        stdioGET_ERRNO());
            stop();
        }
        debug_print("SD startup done!\r\n");

        startup_done = 1;
        for(;;){
            //Wait for notification from debug_print()
            xTaskNotifyWait(0,0, NULL, portMAX_DELAY);
            //Open file
            pxFile = ff_fopen(filename, "a");
            if (!pxFile) {
                printf("ff_fopen failed: %s (%d)\n", strerror(stdioGET_ERRNO()),
                        stdioGET_ERRNO());
                stop();
            }

            //Read and empty the entire queue of debug messages
            while(uxQueueMessagesWaiting(debug_queue) != 0){
                //Clear buffer
                memset(buffer, 0, sizeof(buffer));
                
                //Read message from queue into buffer
                xQueueReceive(
                    debug_queue,
                    &buffer,
                    0
                );

                //Write buffer to file
                if (ff_fprintf(pxFile, buffer) < 0) {
                    // printf("ff_fprintf failed: %s (%d)\n", strerror(stdioGET_ERRNO()),
                    //        stdioGET_ERRNO());
                    stop();
                }
            }
            //Close file
            if (-1 == ff_fclose(pxFile)) {
                //printf("ff_fclose failed: %s (%d)\n", strerror(stdioGET_ERRNO()),
                //        stdioGET_ERRNO());
                stop();
            }

        }
    } else {
        debug_print("SD card reader failed to initialize.\r\n");
        sd_detected = -1;
    }
    
}