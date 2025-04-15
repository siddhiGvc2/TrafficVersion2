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

char payload[300];
void NetwrokFail(void)
{
    strcpy(DISCON_DTIME,currentDateTime);
    utils_nvs_set_str(NVS_DISCON_DTIME, DISCON_DTIME);
}

void NetworkConnect(void)
{
    strcpy(RICON_DTIME,currentDateTime);
    utils_nvs_set_str(NVS_RICON_DTIME, RICON_DTIME);
    sprintf(payload,"*NETWORKOKAY,%s#",RICON_DTIME);
    uart_write_string_ln(payload);
    if(MQTTRequired)
    {
        mqtt_publish_msg(payload);
    }
}

