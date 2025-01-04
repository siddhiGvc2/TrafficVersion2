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



static const char *TAG = "FOTA";

void http_fota(void){
    
   // http_perform_as_stream_reader();
   // return;
   char buffer[100];
    esp_err_t err;
    esp_ota_handle_t ota_handle = 0;
    const esp_partition_t *update_partition = NULL;

    Led_State_t prev_state = led_state;

    update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition == NULL) {
        printf("Failed to get OTA partition.\n#");
        uart_write_string_ln("*Failed to get OTA partition#");
        //esp_http_client_cleanup(client);
        set_led_state(prev_state);
        return;
    }

    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
    if (err != ESP_OK) {
        printf("Failed to begin OTA update: %s\n", esp_err_to_name(err));
        uart_write_string_ln("*Failed to get OTA udpate#");
        //esp_http_client_cleanup(client);
        set_led_state(prev_state);
        return;
    }

    esp_http_client_config_t config = {
        .url = FOTA_URL,
        .event_handler = _http_event_handler
    };

    

    esp_http_client_handle_t client = esp_http_client_init(&config);
    /*
    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        printf("Failed to download firmware image: %s\n", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        set_led_state(prev_state);
        return;
    }
    */
    
    if ((err = esp_http_client_open(client, 0)) != ESP_OK) {
        ESP_LOGE(TAG, "*Failed to open HTTP connection: %s#", esp_err_to_name(err));
        send(sock, "*FOTA-ERROR#", strlen("*FOTA-ERROR#"), 0);
        uart_write_string_ln("*FOTA-ERROR#");
        fotaStatus=0;
        uart_write_string_ln("*Failed to open http#");
        esp_http_client_cleanup(client);
        set_led_state(prev_state);
        return;
    }

    ESP_LOGI(TAG, "*esp_http_client_open#");
    uart_write_string_ln("*esp_http_client_open#");

    /*
    int read_bytes = 0;
    while (1) {
        read_bytes = esp_http_client_read(client, data, sizeof(data));
        if (read_bytes <= 0) {
            break;
        }

        err = esp_ota_write(ota_handle, (const void *)data, read_bytes);
        if (err != ESP_OK) {
            printf("Failed to write OTA data: %s\n", esp_err_to_name(err));
            esp_http_client_cleanup(client);
        }else{
            ESP_LOGI(TAG, "Written : %d", read_bytes);
        }
    }
    */
    char data[MAX_HTTP_RECV_BUFFER+1];
    int content_length =  esp_http_client_fetch_headers(client);
    int total_read_len = 0, read_len;
    set_led_state(OTA_IN_PROGRESS);
    if(content_length > 0){
        while (total_read_len < content_length ) {
            read_len = esp_http_client_read(client, data, MAX_HTTP_RECV_BUFFER);
            if (read_len <= 0) {
                ESP_LOGI(TAG, "*Error read data#");
                fotaStatus=0;
                send(sock, "*FOTA-ERROR#", strlen("*FOTA-ERROR#"), 0);
                uart_write_string_ln("*FOTA-ERROR#");
            }
            //ESP_LOGI(TAG, "read_len = %d", read_len);
            total_read_len += read_len;
            err = esp_ota_write(ota_handle, (const void *)data, read_len);
            if (err != ESP_OK) {
                printf("Failed to write OTA data: %s\n", esp_err_to_name(err));
                send(sock, "*FOTA-ERROR#", strlen("*FOTA-ERROR#"), 0);
                fotaStatus=0;
                uart_write_string_ln("*FOTA-ERROR#");
                esp_http_client_cleanup(client);
            }else{
                sprintf(buffer,"*OTA Percent : %d#", ((total_read_len*100)/content_length));    
                // ESP_LOGI(TAG, "*OTA Percent : %d#", ((total_read_len*100)/content_length) );
                // uart_write_string_ln(buffer);
            }
        }
    }
    

    

    if(err != ESP_OK){
        set_led_state(prev_state);
        return;
    }

    ESP_LOGI(TAG, "*ota data written#");
    uart_write_string("*ota data written#");
    err = esp_ota_end(ota_handle);
    if (err != ESP_OK) {
        printf("*OTA update failed: %s\n#", esp_err_to_name(err));
        send(sock, "*FOTA-ERROR#", strlen("*FOTA-ERROR#"), 0);
        uart_write_string_ln("*FOTA-ERROR#");
        fotaStatus=0;
        uart_write_string_ln("*ota data written#");
        esp_http_client_cleanup(client);
        set_led_state(prev_state);
        return;
    }

    ESP_LOGI(TAG, "*esp_ota_end#");
    uart_write_string_ln("*ota data written#");

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        printf("Failed to set boot partition: %s\n", esp_err_to_name(err));
        send(sock, "*FOTA-ERROR#", strlen("*FOTA-ERROR#"), 0);
        fotaStatus=0;
        uart_write_string_ln("*FOTA-ERROR#");
        esp_http_client_cleanup(client);
        set_led_state(prev_state);
        return;
    }

    ESP_LOGI(TAG, "*esp_ota_set_boot_partition#");
    uart_write_string_ln("*esp_ota_set_boot_partition#");
    
    esp_http_client_cleanup(client);
    printf("*OTA update successful! Restarting...\n#");
    send(sock, "*FOTA-OVER#", strlen("*FOTA-OVER#"), 0);
    fotaStatus=0;
    uart_write_string_ln("*FOTA-OVER#");
    uart_write_string_ln("OTA update successful! Restarting...");
    uart_write_string_ln("*Resetting device-FOTA over#");
    RestartDevice();
    set_led_state(prev_state);
}
