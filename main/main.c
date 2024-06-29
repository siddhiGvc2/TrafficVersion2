
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
#include "calls.h"
//#include "externVars.h"
#include "vars.h"







void set_led_state(Led_State_t st);
void uart_write_string(const char * str);
void uart_write_string_ln(const char * str);
void uart_write_number(uint8_t);
void tcp_ip_client_send_str(const char *);
void resolve_hostname(const char *);
uint32_t millis(void);
void http_fota( void ) ; 
void Out4094 (unsigned char);

extern bool extractSubstring(const char* , char* );


/* FreeRTOS event group to signal when we are connected*/


static const char *TAG = "main";



void Out4094 (unsigned char);

void WiFiConnection (void);




#define LED_ACTIVE_HIGH

nvs_handle_t utils_nvs_handle;














bool extractSubstring(const char* str, char* result) {
    const char* start = strchr(str, '*');
    const char* end = strchr(str, '#');

    if (start != NULL && end != NULL && end > start + 1) {
        strncpy(result, start + 1, end - start - 1);
        result[end - start - 1] = '\0';
        return true;
    }

    return false;
}

void resolve_hostname(const char *hostname) {
    struct addrinfo hints, *res;
    int status;
    // char ipstr[100];
    char payload[200];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // AF_INET for IPv4, AF_INET6 for IPv6
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
        // Corrected here: use gai_strerror instead of strerror
        fprintf(stderr, "getaddrinfo: %s\n", strerror(status));
        return;
    }

    printf("*IP addresses for %s:\n\n#", hostname);
    ESP_LOGI(TAG,"*IP addresses for %s:\n\n#", hostname);
    struct addrinfo *p;
    for (p = res; p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;

        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        sprintf(payload,"*IP VER - %s: IP STR - %s#", ipver, ipstr);
        uart_write_string(payload);
//        ESP_LOGI(TAG,payload);

    }

    freeaddrinfo(res); // free the linked list
}


// void WiFiConnection (void)
// {
//         set_led_state(SEARCH_FOR_WIFI);
//         bool connected_to_wifi = false;
//         int WiFiRetryCount = 0;
//         while (connected_to_wifi == 0)
//         {
//             if (WiFiRetryCount >= 3) 
//             {
//                 ESP_LOGI(TAG, "*Restarting as WiFi Not Sensed#");
//                 vTaskDelay(500);
//                 esp_restart();                
//             }   
//             if (WiFiNumber != 1)
//             {
//                 ESP_LOGI(TAG, "*Trying to connect to SSID1 %s %s#",WIFI_SSID_1, WIFI_PASS_1);
//                 if(!connect_to_wifi(WIFI_SSID_1, WIFI_PASS_1)){
//                     s_retry_num = 0;
//                     WiFiNumber = 1;
//                     ESP_LOGI(TAG, "*Failed to Connect SSID1#");
//                     WiFiRetryCount++;
//                 }else{
//                     ESP_LOGI(TAG, "*Connected To WiFi1#");
//                     connected_to_wifi = true;
//                     WiFiNumber = 1;
//                 }
//             }

//             else
//             {
//                 ESP_LOGI(TAG, "*Trying to connect to SSID2 %s %s#",WIFI_SSID_2, WIFI_PASS_2);
//                 if(!connect_to_wifi(WIFI_SSID_2, WIFI_PASS_2)){
//                     s_retry_num = 0;
//                     WiFiNumber = 2;
//                     ESP_LOGI(TAG, "*Failed to Connect SSID2#");
//                     WiFiRetryCount++;
//                 }else{
//                     ESP_LOGI(TAG, "*Connected To WiFi2#");
//                     connected_to_wifi = true;
//                     WiFiNumber = 2;
//                 }
//             }
//         }

//     if(connected_to_wifi){
//              connected_to_wifi_and_internet = true;
//         // esp_http_client_config_t config = {
//         //     .url = "http://www.google.com",  
//         // };
//         // esp_http_client_handle_t client = esp_http_client_init(&config);
//         // esp_err_t err = esp_http_client_perform(client);
//         // if (err == ESP_OK) {
//         //     ESP_LOGI(TAG, "*Internet connection test successful#");
//         //     connected_to_wifi_and_internet = true;
//         // } else {
//         //     ESP_LOGI(TAG,"*Internet connection test failed: %s#", esp_err_to_name(err));
//         //     set_led_state(WIFI_FOUND_NO_INTERNET);
//         // }
//         // esp_http_client_cleanup(client);
//     }


// }


uint32_t millis(void) {
    return (uint32_t)(esp_timer_get_time() / 1000ULL);
}

uint32_t last_update_led1 = 0;
uint8_t led1_gpio_state = 0;
uint8_t led2_gpio_state = 0;

void set_led_state(Led_State_t st){
    last_update_led1 = 0;
    led_state = st;
}

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

#define MAX_HTTP_RECV_BUFFER 1024

void http_fota( void ){
    
   // http_perform_as_stream_reader();
   // return;
    esp_err_t err;
    esp_ota_handle_t ota_handle = 0;
    const esp_partition_t *update_partition = NULL;

    Led_State_t prev_state = led_state;

    update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition == NULL) {
        printf("Failed to get OTA partition.\n#");
        //esp_http_client_cleanup(client);
        set_led_state(prev_state);
        return;
    }

    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
    if (err != ESP_OK) {
        printf("Failed to begin OTA update: %s\n", esp_err_to_name(err));
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
        esp_http_client_cleanup(client);
        set_led_state(prev_state);
        return;
    }

    ESP_LOGI(TAG, "*esp_http_client_open#");

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
                esp_http_client_cleanup(client);
            }else{
                ESP_LOGI(TAG, "*OTA Percent : %d#", ((total_read_len*100)/content_length) );
            }
        }
    }
    

    

    if(err != ESP_OK){
        set_led_state(prev_state);
        return;
    }

    ESP_LOGI(TAG, "*ota data written#");

    err = esp_ota_end(ota_handle);
    if (err != ESP_OK) {
        printf("*OTA update failed: %s\n#", esp_err_to_name(err));
        send(sock, "*FOTA-ERROR#", strlen("*FOTA-ERROR#"), 0);
        esp_http_client_cleanup(client);
        set_led_state(prev_state);
        return;
    }

    ESP_LOGI(TAG, "*esp_ota_end#");

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        printf("Failed to set boot partition: %s\n", esp_err_to_name(err));
        send(sock, "*FOTA-ERROR#", strlen("*FOTA-ERROR#"), 0);
        esp_http_client_cleanup(client);
        set_led_state(prev_state);
        return;
    }

    ESP_LOGI(TAG, "*esp_ota_set_boot_partition#");
    
    esp_http_client_cleanup(client);
    printf("*OTA update successful! Restarting...\n#");
    send(sock, "*FOTA-OVER#", strlen("*FOTA-OVER#"), 0);
    
    vTaskDelay(2000/portTICK_PERIOD_MS);
    esp_restart();
    set_led_state(prev_state);
}

void leds_update_task(){
    for(;;){

        //Handle Led 1 States

        ticks_100 = ticks_100+1;
        if (ticks_100 >= 20)
        {
            ticks_100 = 0;
         }    
        if (led_state == SEARCH_FOR_ESPTOUCH)
        {
            current_interval = 0;
            if(led2_gpio_state == 1){
                led2_gpio_state = 0;
                led_set_level(LEDG, led2_gpio_state);
            }
            
        }
        if(led_state == STANDBY_LED){
            current_interval = 0;
            if(led1_gpio_state == 1){
                led1_gpio_state = 0;
                led_set_level(LEDR, led1_gpio_state);
            }
        }else if(led_state == SEARCH_FOR_WIFI){
            numberOfPulses = 20;
        }else if(led_state == WIFI_FOUND_NO_INTERNET){
            numberOfPulses = 6;
        }else if(led_state == WIFI_AND_INTERNET_NO_SERVER){
            numberOfPulses = 4;
        }else if(led_state == EVERYTHING_OK_LED){
            numberOfPulses = 2;
        }    
        //     // if(rx_event_pending){
        //     //     rx_event_pending = 0; 
        //     //     led_set_level(LEDG, 0);
        //     //     vTaskDelay(50/portTICK_PERIOD_MS);
        //     //     led_set_level(LEDG, 1);
        //     //     vTaskDelay(50/portTICK_PERIOD_MS);
        //     // }

        //     // if(tx_event_pending){
        //     //     tx_event_pending = 0; 
        //     //     led_set_level(LEDG, 0);
        //     //     vTaskDelay(50/portTICK_PERIOD_MS);
        //     //     led_set_level(LEDG, 1);
        //     //     vTaskDelay(50/portTICK_PERIOD_MS);
        //     //     led_set_level(LEDG, 0);
        //     //     vTaskDelay(50/portTICK_PERIOD_MS);
        //     //     led_set_level(LEDG, 1);
        //     //     vTaskDelay(50/portTICK_PERIOD_MS);
        //     // }

        // }
           else if (led_state == OTA_IN_PROGRESS){
            led2_gpio_state = !led2_gpio_state;
            led_set_level(LEDG, led2_gpio_state);
        }

        if (numberOfPulses>ticks_100)
        {
            // if (led_state == SEARCH_FOR_ESPTOUCH)
            // {
            //     led2_gpio_state = !led2_gpio_state;
            //     led_set_level(LEDG, led2_gpio_state);
            //     led_set_level(LEDR, 0);

            // }
            // else
            // {
                led1_gpio_state = !led1_gpio_state;
                led_set_level(LEDR, led1_gpio_state);
                led_set_level(LEDG, 0);
            // }
        }
        else
        {
            led1_gpio_state = 0;
            led_set_level(LEDR, led1_gpio_state);
        }
        // // if(((last_update_led1 == 0) || (millis() - last_update_led1 > current_interval)) && (current_interval != 0)){
        // //     //This will be only called when there is some error
        // //     last_update_led1 = millis();
        // //     led1_gpio_state = !led1_gpio_state;
        // //     led_set_level(LEDR, led1_gpio_state);
        // // }
        vTaskDelay(200/portTICK_PERIOD_MS);
    }
}

void status_leds_init(){
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = 1ULL << LEDR | 1ULL << LEDG ;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
    led_set_level(LEDR, 0);
    led_set_level(LEDG, 0);
    xTaskCreate(leds_update_task, "leds_update_task", 2048, NULL, 6, NULL);
}

void uart_write_number(uint8_t number){
    char str[5];
    if (number<10)
    {
         str[0] = number+'0';
         str[1] = 0x00;
    }   
    uart_write_bytes(EX_UART_NUM, str, 1);
}

void uart_write_string(const char * str){
    uart_write_bytes(EX_UART_NUM, str, strlen(str));
}

void uart_write_string_ln(const char * str){
    uart_write_bytes(EX_UART_NUM, str, strlen(str));
    uart_write_string("\r\n");
}











void app_main(void)
{
    //Initialize NVS
    //esp_log_level_set("*", ESP_LOG_NONE);
    // set totals to 0
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
    xTaskCreate(tcpip_client_task, "tcpip_client_task", 8192, NULL, 7, NULL);
   
    ESP_LOGI(TAG, "*Starting ICH#");
    ICH_init();
    ESP_LOGI(TAG, "*Starting WiFi#");
    wifi_init_sta();
    ESP_LOGI(TAG, "*Starting S2P#");
    s2p_init();
    ESP_LOGI(TAG, "*Clearing 4094 Output#");
    Out4094(0x00);; // set all outputs inactive
    xTaskCreate(sendHBT, "sendHBT", 2048, NULL, 6, NULL);
    xTaskCreate(BlinkLED, "BlinkLED", 2048, NULL, 6, NULL);
    if (!Production)
        xTaskCreate(TestCoin, "TestCoin", 2048, NULL, 6, NULL);
    for (;;) 
    {
        vTaskDelay(100);   
    }
}
