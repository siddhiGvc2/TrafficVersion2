#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"   
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_timer.h"
#include "esp_ota_ops.h"
#include "driver/uart.h"
#include "esp_netif.h"
#include "rom/ets_sys.h"
#include "esp_smartconfig.h"
#include <sys/socket.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "externVars.h"
#include "calls.h"



void incrementDateTimeByOneSecond(const char *dateTimeStr, char *outputStr) {
    struct tm t = {0};

    sscanf(dateTimeStr, "%2d%2d%2d%2d%2d%2d",
           &t.tm_mday,
           &t.tm_mon,
           &t.tm_year,
           &t.tm_hour,
           &t.tm_min,
           &t.tm_sec);

    t.tm_mon -= 1;         // struct tm months are 0-11
    t.tm_year += 100;      // tm_year is years since 1900 (assuming 20xx)

    time_t timeEpoch = mktime(&t);
    timeEpoch += 1; // Increment by 1 second

    struct tm *newTime = localtime(&timeEpoch);

    // Format back to ddmmyyhhmmss
    sprintf(outputStr, "%02d%02d%02d%02d%02d%02d",
            newTime->tm_mday,
            newTime->tm_mon + 1,
            newTime->tm_year % 100,
            newTime->tm_hour,
            newTime->tm_min,
            newTime->tm_sec);
}

void date_time_task(void) {
   

    while (1) {
        incrementDateTimeByOneSecond(currentDateTime, updatedDateTime);
        strcpy(currentDateTime, updatedDateTime);

        // printf("Current DateTime: %s\n", currentDateTime);

        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 second
    }
}
