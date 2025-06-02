
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


static const char *TAG = "TrafficNVS";

void CallTrafficNVS(void){
    ESP_LOGI(TAG,"TRAFFIC NVS INITIALLIZED");
    if(uartDebugInfo)
        uart_write_string_ln("TRAFFIC NVS INITIALLIZED");



    if(utils_nvs_get_str(NVS_DEF1,DEF1,100) == ESP_OK){
        utils_nvs_get_str(NVS_DEF1,DEF1,100);
       }
       else{
           strcpy(DEF1,DEFAULT_DEF1);
          utils_nvs_set_str(NVS_DEF1,DEF1); 
       }
       if(utils_nvs_get_str(NVS_DEF2,DEF2,100) == ESP_OK){
        utils_nvs_get_str(NVS_DEF2,DEF2,100);
       }
       else{
           strcpy(DEF2,DEFAULT_DEF2);
          utils_nvs_set_str(NVS_DEF2,DEF2); 
       }

       if(utils_nvs_get_str(NVS_DEF3,DEF3,100) == ESP_OK){
        utils_nvs_get_str(NVS_DEF3,DEF3,100);
       }
       else{
           strcpy(DEF3,DEFAULT_DEF3);
          utils_nvs_set_str(NVS_DEF3,DEF3); 
       }

       if (utils_nvs_get_str(NVS_DEF4, DEF4, 100) == ESP_OK) {
            utils_nvs_get_str(NVS_DEF4, DEF4, 100);
        } else {
            strcpy(DEF4, DEFAULT_DEF4);
            utils_nvs_set_str(NVS_DEF4, DEF4);
        }
        ////////
        
    CalculateAllTime();    

    if(utils_nvs_get_str(NVS_ATC1,ATC1,100) == ESP_OK){
        utils_nvs_get_str(NVS_ATC1,ATC1,100);
       }
       else{
           strcpy(ATC1,DEFAULT_ATC1);
          utils_nvs_set_str(NVS_ATC1,ATC1); 
       }
       if(utils_nvs_get_str(NVS_ATC2,ATC2,100) == ESP_OK){
        utils_nvs_get_str(NVS_ATC2,ATC2,100);
       }
       else{
           strcpy(ATC2,DEFAULT_ATC2);
          utils_nvs_set_str(NVS_ATC2,ATC2); 
       }

       if(utils_nvs_get_str(NVS_ATC3,ATC3,100) == ESP_OK){
        utils_nvs_get_str(NVS_ATC3,ATC3,100);
       }
       else{
           strcpy(ATC3,DEFAULT_ATC3);
          utils_nvs_set_str(NVS_ATC3,ATC3); 
       }

       if (utils_nvs_get_str(NVS_ATC4, ATC4, 100) == ESP_OK) {
            utils_nvs_get_str(NVS_ATC4, ATC4, 100);
        } else {
            strcpy(ATC4, DEFAULT_ATC4);
            utils_nvs_set_str(NVS_ATC4, ATC4);
        }
        ////////

    if(utils_nvs_get_str(NVS_STAGE1,stage1,100) == ESP_OK){
        utils_nvs_get_str(NVS_STAGE1,stage1,100);
       }
       else{
           strcpy(stage1,DEFAULT_STAGE1);
          utils_nvs_set_str(NVS_STAGE1,stage1); 
       }
       if(utils_nvs_get_str(NVS_STAGE2,stage2,100) == ESP_OK){
        utils_nvs_get_str(NVS_STAGE2,stage2,100);
       }
       else{
           strcpy(stage2,DEFAULT_STAGE2);
          utils_nvs_set_str(NVS_STAGE2,stage2); 
       }

       if(utils_nvs_get_str(NVS_STAGE3,stage3,100) == ESP_OK){
        utils_nvs_get_str(NVS_STAGE3,stage3,100);
       }
       else{
           strcpy(stage3,DEFAULT_STAGE3);
          utils_nvs_set_str(NVS_STAGE3,stage3); 
       }

       if (utils_nvs_get_str(NVS_STAGE4, stage4, 100) == ESP_OK) {
            utils_nvs_get_str(NVS_STAGE4, stage4, 100);
        } else {
            strcpy(stage4, DEFAULT_STAGE4);
            utils_nvs_set_str(NVS_STAGE4, stage4);
        }

        if (utils_nvs_get_str(NVS_STAGE5, stage5, 100) == ESP_OK) {
            utils_nvs_get_str(NVS_STAGE5, stage5, 100);
        } else {
            strcpy(stage5, DEFAULT_STAGE5);
            utils_nvs_set_str(NVS_STAGE5, stage5);
        }
        
        if (utils_nvs_get_str(NVS_STAGE6, stage6, 100) == ESP_OK) {
            utils_nvs_get_str(NVS_STAGE6, stage6, 100);
        } else {
            strcpy(stage6, DEFAULT_STAGE6);
            utils_nvs_set_str(NVS_STAGE6, stage6);
        }
}