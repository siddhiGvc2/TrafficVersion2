
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
#include "esp_heap_caps.h"
#include "calls.h"
#include "vars.h"

// void resolve_hostname(const char *);
// uint32_t millis(void);

// extern bool extractSubstring(const char* , char* );


static const char *TAG = "main";

void heap_monitor_task(void *pvParameters) {
    while (1) {
        free_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
        free_internal_heap = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        // printf("Free heap: %d bytes\n", free_heap);
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 second delay
    }
}

void app_main(void)
{
    //Initialize NVS
    //esp_log_level_set("*", ESP_LOG_NONE);
    // set totals to 0

    
    char payload[100];
    MQTTRequired = 1;
    TCPRequired = 1;
    FirstTryMQTT = 1;
    FirstPowerOn = 1;
    for (int i = 0 ; i < 7 ; i++)
    {
        Totals[i] = 0;
        CashTotals[i] = 0;
    }   
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "================================");
    ESP_LOGI(TAG, "*FW:%s#",FWVersion);
    ESP_LOGI(TAG, "================================");
    utils_nvs_init();
    status_leds_init();
    console_uart_init();
    uart_write_string(FWVersion);
    read_mac_address();
    xTaskCreate(tcpip_client_task, "tcpip_client_task", 1024 * 10, NULL, 12, NULL);
    load_settings_nvs();
    ESP_LOGI(TAG, "*Starting ICH#");
    ICH_init();
    ESP_LOGI(TAG, "*Starting S2P#");
    s2p_init();
    Out4094(0x00);; // set all outputs inactive

    // for (int i = 0 ; i < 3 ; i++)
    // {
    //     led_set_level(LEDR, 1);
    //     led_set_level(LEDG, 0);
    //     vTaskDelay(500/portTICK_PERIOD_MS);   
    //     led_set_level(LEDR, 0);
    //     led_set_level(LEDG, 1);
    //     vTaskDelay(500/portTICK_PERIOD_MS);   
    //     led_set_level(LEDR, 0);
    //     led_set_level(LEDG, 0);
    //     vTaskDelay(500/portTICK_PERIOD_MS);   
    // }
    ESP_LOGI(TAG, "*Starting WiFi#");
    wifi_init_sta();
    ESP_LOGI(TAG, "*Testing RGB #");
    TestRGB();

    last_hbt_time_us = esp_timer_get_time(); // Init with current time

    
    xTaskCreate(sendHBT, "sendHBT", 4096, NULL, 6, NULL);
    xTaskCreate(BlinkLED, "BlinkLED", 2048, NULL, 6, NULL);
    // removed on 10-05-25
    // xTaskCreate(RetryMqtt, "RetryMqtt", 2048, NULL, 6, NULL);
  
   // Create monitor task
   // xTaskCreate(hbt_monitor_task, "hbt_monitor_task", 2048, NULL, 5, NULL);
    xTaskCreate(heap_monitor_task, "heap_monitor", 2048, NULL, 5, NULL);
    xTaskCreate(date_time_task, "date_time_task", 2048, NULL, 5, NULL);
    xTaskCreate(SendTCcommand,"SendTCcommand", 8192, NULL, 6, NULL);
    xTaskCreate(TestCoin, "TestCoin", 2048, NULL, 6, NULL);
   
    for (;;) 
    {
        vTaskDelay(1000/portTICK_PERIOD_MS);  // 1 sec delay
        // logic added on 251224
        //  display No HBT For X minutes once every minute
        // and restart if no HBT for Y minutes

        // removed on 301224 as advised by Neeraj Ji
        if (IsSocketConnected) 
        {
            if (MQTTRequired)
            {
                    if (MQTT_CONNEECTED)
                    ServerHBTTimeOut++;
            }    
            else
                ServerHBTTimeOut++;
        }    
    // skip Error in first minute
        if ( ((ServerHBTTimeOut % 60) == 10) && (ServerHBTTimeOut > 65) )
        {
            sprintf(payload,"*No HBT For %d Minutes",ServerHBTTimeOut/60);
            uart_write_string_ln(payload);
        }
        if (ServerHBTTimeOut > HBTTIMEBEFORERESTART)
        {
            RestartDevice();
        }
     //   1 min = 60 sec, 30 min  = 1800 seconds = 18000 X 100 msec ticks  
        // 35 mins = 2100 seconds = 21000 X 100 msecs ticks
    }
}
