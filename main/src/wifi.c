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



static const char *TAG = "WIFI";
static int s_retry_num = 0;
static int FirstWiFiConnection = 0;


bool connect_to_wifi(char *, char *);
void event_handler(void* , esp_event_base_t , int32_t , void* );
static EventGroupHandle_t s_wifi_event_group;
void smartconfig_example_task(void * );
void wifi_init_sta(void);
void http_fota(void);
esp_err_t _http_event_handler(esp_http_client_event_t *);
/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const int CONNECTED_BIT = BIT2;
static const int ESPTOUCH_DONE_BIT = BIT3;


esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        break;
    }
    return ESP_OK;
}


void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    char buffer[100];
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
         if (gpio_get_level(JUMPER2) == 0)
         {
             xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 6, NULL);
            set_led_state(SEARCH_FOR_ESPTOUCH);
            ESP_LOGI(TAG,"*Start Looking for ESP TOUCH#");
         }
         else
             esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
            ESP_LOGI(TAG, "*WiFi Connected %d#",WiFiNumber);
            sprintf(buffer,"*WiFi Connected %d#",WiFiNumber);
            uart_write_string_ln(buffer);
            s_retry_num = 0;
            FirstWiFiConnection = 1;
            connected_to_wifi_and_internet = true;
 
    }    
    
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        connected_to_wifi_and_internet = false;
        set_led_state(SEARCH_FOR_WIFI);
        ESP_LOGI(TAG,"*Connect WiFi after disconnection#");
        vTaskDelay(ESP_RETRY_GAP);
        if (s_retry_num < ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "*retry to connect to the AP  %d#",s_retry_num);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGI(TAG, "*WiFi failed bit set %d#",WiFiNumber);
            if ((FirstWiFiConnection == 1) || (WiFiNumber == 3))
            {
                ESP_LOGI(TAG, "*restarting after 2 seconds#");
                uart_write_string_ln("*Resetting device#");
                vTaskDelay(2000/portTICK_PERIOD_MS);
                esp_restart();
            }
    }
        ESP_LOGI(TAG,"*connect to the AP fail#");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "*got ip:*" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
// disable mqtt      
    if (MQTTRequired)
            mqtt_app_start();  // connect to MQTT when IP received
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        ESP_LOGI(TAG, "*Scan done#");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        ESP_LOGI(TAG, "*Found channel#");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(TAG, "*Got SSID and password#");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        char ssid[33] = { 0 };
        char password[65] = { 0 };
        uint8_t rvd_data[33] = { 0 };

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true) {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "*SSID3:%s#", ssid);
        ESP_LOGI(TAG, "*PASSWORD3:%s#", password);
        // memorise in NV RAM
        utils_nvs_set_str(NVS_SSID_3_KEY, ssid);
        utils_nvs_set_str(NVS_PASS_3_KEY, password);

        if (evt->type == SC_TYPE_ESPTOUCH_V2) {
            ESP_ERROR_CHECK( esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)) );
            ESP_LOGI(TAG, "*RVD_DATA:");
            for (int i=0; i<33; i++) {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n#");
        }

        ESP_ERROR_CHECK( esp_wifi_disconnect() );
        ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
        esp_wifi_connect();
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}


bool connect_to_wifi(char * ssid, char * psk){
    bool wifi_connected = false;
    esp_wifi_stop();
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    strcpy((char *)wifi_config.sta.ssid, ssid);
    strcpy((char *)wifi_config.sta.password, psk);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "*connected to ap ssid:%s password:%s \r\n#", ssid, psk);
        wifi_connected = true;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "*failed to connect to ssid:%s, password:%s \r\n#", ssid, psk);
    } else {
        ESP_LOGE(TAG, "*unexpected event#");
    }

    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
    ESP_LOGI(TAG,"*returning from connecting 2 wifi#");
    return wifi_connected;
}
 
void smartconfig_example_task(void * parm)
{
    EventBits_t uxBits;
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_smartconfig_start(&cfg) );
    while (1) {
        uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if(uxBits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if(uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}

void wifi_init_sta(void)
{

    load_settings_nvs();
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    /*
    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );*/

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id) );
    
    set_led_state(SEARCH_FOR_WIFI);
    ESP_LOGI(TAG,"*Connect WiFi at Power On#");
    bool connected_to_wifi = false;
    //ESP_LOGI(TAG, "Trying to connect to SSID1 %s | %s",DEFAULT_SSID1,DEFAULT_PASS1);
    ESP_LOGI(TAG, "*Trying to connect to SSID1#");
    WiFiNumber = 1;
    if(!connect_to_wifi(WIFI_SSID_1, WIFI_PASS_1)){
        //ESP_LOGI(TAG, "Trying to connect to SSID2 %S | %S",DEFAULT_SSID2, DEFAULT_PASS1);
        ESP_LOGI(TAG, "*Trying to connect to SSID2# ");
        WiFiNumber = 2;
        s_retry_num = 0;
        if(!connect_to_wifi(WIFI_SSID_2, WIFI_PASS_2)){

            ESP_LOGI(TAG, "*Trying to connect to SSID3# ");
            WiFiNumber = 3;
            s_retry_num = 0;

             if(!connect_to_wifi(WIFI_SSID_3, WIFI_PASS_3)){
                ESP_LOGI(TAG, "Could not connect to SSID3. Restarting....");
                ESP_LOGI(TAG, "*restarting after 2 seconds#");
                uart_write_string_ln("*Resetting device - Could not connect to SSID3#");
                vTaskDelay(2000/portTICK_PERIOD_MS);
                esp_restart();
             }
             else{
            ESP_LOGI(TAG, "*Connected To WiFi3#");
            connected_to_wifi = true;
        }
        }
          else{
            ESP_LOGI(TAG, "*Connected To WiFi2#");
            connected_to_wifi = true;
        }
    }else{
        ESP_LOGI(TAG, "*Connected To WiFi1#");
        connected_to_wifi = true;
    }
  
    if(connected_to_wifi){
        connected_to_wifi_and_internet = true;
        // esp_http_client_config_t config = {
        //     .url = "http://www.google.com",  
        // };
        // esp_http_client_handle_t client = esp_http_client_init(&config);
        // esp_err_t err = esp_http_client_perform(client);
        // if (err == ESP_OK) {
        //     ESP_LOGI(TAG, "Internet connection test successful\n");
        //     set_led_state(WIFI_AND_INTERNET_NO_SERVER);
        // } else {
        //     ESP_LOGI(TAG,"Internet connection test failed: %s\n", esp_err_to_name(err));
        //     set_led_state(WIFI_FOUND_NO_INTERNET);
        // }
        // esp_http_client_cleanup(client);
    }

}




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
                send(sock, "*FOTA-ERROR#", strlen("*FOTA-ERROR#"), 0);
            }
            //ESP_LOGI(TAG, "read_len = %d", read_len);
            total_read_len += read_len;
            err = esp_ota_write(ota_handle, (const void *)data, read_len);
            if (err != ESP_OK) {
                printf("Failed to write OTA data: %s\n", esp_err_to_name(err));
                send(sock, "*FOTA-ERROR#", strlen("*FOTA-ERROR#"), 0);
                uart_write_string_ln("*FOTA-ERROR#");
                esp_http_client_cleanup(client);
            }else{
                sprintf(buffer,"*OTA Percent : %d#", ((total_read_len*100)/content_length));    
                ESP_LOGI(TAG, "*OTA Percent : %d#", ((total_read_len*100)/content_length) );
                uart_write_string_ln(buffer);
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
        esp_http_client_cleanup(client);
        set_led_state(prev_state);
        return;
    }

    ESP_LOGI(TAG, "*esp_ota_set_boot_partition#");
    uart_write_string_ln("*esp_ota_set_boot_partition#");
    
    esp_http_client_cleanup(client);
    printf("*OTA update successful! Restarting...\n#");
    send(sock, "*FOTA-OVER#", strlen("*FOTA-OVER#"), 0);
    uart_write_string_ln("OTA update successful! Restarting...");
    uart_write_string_ln("*Resetting device-FOTA over#");
    vTaskDelay(2000/portTICK_PERIOD_MS);
    esp_restart();
    set_led_state(prev_state);
}
