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
#include <time.h>  // Include time.h for time functions, if available
#include "externVars.h"
#include "calls.h"

char payload[300];

void incrementDateTimeByOneSecond(const char *dateTimeStr, char *outputStr) {
    struct tm t = {0};

    sscanf(dateTimeStr, "%4d-%2d-%2d %2d:%2d:%2d",
           &t.tm_year,
           &t.tm_mon,
           &t.tm_mday,
           &t.tm_hour,
           &t.tm_min,
           &t.tm_sec);

    t.tm_year -= 1900;     // struct tm expects years since 1900
    t.tm_mon -= 1;         // struct tm months are 0–11

    time_t timeEpoch = mktime(&t);
    timeEpoch += 1;  // Add one second

    struct tm newTime;
    localtime_r(&timeEpoch, &newTime);

    // Format: YYYY-MM-DD HH:MM:SS
    snprintf(outputStr,sizeof(currentDateTime), "%04d-%02d-%02d %02d:%02d:%02d",
             newTime.tm_year + 1900,
             newTime.tm_mon + 1,
             newTime.tm_mday,
             newTime.tm_hour,
             newTime.tm_min,
             newTime.tm_sec);
}


void date_time_task(void) {
    time_t now = time(NULL);
    now += 19800;  // Add IST offset (+5 hours 30 minutes)

    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);

    snprintf(currentDateTime, sizeof(currentDateTime), "%04d-%02d-%02d %02d:%02d:%02d",
             timeinfo.tm_year + 1900,
             timeinfo.tm_mon + 1,
             timeinfo.tm_mday,
             timeinfo.tm_hour,
             timeinfo.tm_min,
             timeinfo.tm_sec);

    while (1) {
        incrementDateTimeByOneSecond(currentDateTime, updatedDateTime);
        strcpy(currentDateTime, updatedDateTime);

        // Debug print
        // printf("IST Time: %s\n", currentDateTime);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
