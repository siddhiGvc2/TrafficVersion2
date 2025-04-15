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



static const char *TAG = "TCP";

void tcpip_client_task(void);
void sendHBT (void);
void tcp_ip_client_send_str(const char *);



void sendError(int sock, const char* errorMsg) {
    send(sock, errorMsg, strlen(errorMsg), 0);
}

void sendSSIDData(int sock, const char* SSuserName, const char* SSdateTime, int WiFiNumber, const char* WIFI_SSID_1, const char* WIFI_SSID_2, const char* WIFI_SSID_3) {
    char payload[256];  // Ensure this buffer is large enough to hold the formatted string

    // Check if any of the first four parameters are missing
    if (!SSuserName || !SSdateTime || WiFiNumber < 0 || !WIFI_SSID_1) {
        sendError(sock, "Error: Missing or invalid parameters");
        return;
    }

    sprintf(payload, "*SSID,%s,%s,%d,%s,%s,%s#", SSuserName, SSdateTime, WiFiNumber, WIFI_SSID_1, WIFI_SSID_2 ? WIFI_SSID_2 : "", WIFI_SSID_3 ? WIFI_SSID_3 : "");
    send(sock, payload, strlen(payload), 0);
}
void tcpip_client_task(){
    char payload[700];
    char rx_buffer[128];
    int addr_family = 0;
    int ip_protocol = 0;
    uint32_t lastPrint = 0;
    for(;;){
        if(connected_to_wifi_and_internet){ //continously check for wifi
            //if wifi connected try to connect to tcp server
             resolve_hostname(server_ip_addr);
            struct sockaddr_in dest_addr;
            dest_addr.sin_addr.s_addr = inet_addr(ipstr);
            dest_addr.sin_family = AF_INET;
            dest_addr.sin_port = htons(server_port);
            addr_family = AF_INET;
            ip_protocol = IPPROTO_IP;
            ServerRetryCount++;
            if (ServerRetryCount >= 10)
                RestartDevice();

            ESP_LOGI(TAG, "*Trying to connect to TCP Server#");
            set_led_state(WIFI_AND_INTERNET_NO_SERVER);
            sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
            if (sock < 0) {
                ESP_LOGE(TAG, "*Unable to create socket: errno %d#", errno);
                shutdown(sock, 0);
                close(sock);
            }else{
                ESP_LOGI(TAG, "*Socket created, connecting to %s:%s:%d#", server_ip_addr,ipstr, server_port);
                int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
                if (err != 0) {
                    ESP_LOGE(TAG, "*Socket unable to connect: errno %d#", errno);
                    ESP_LOGE(TAG, "*Shutting down socket and restarting...#");
                    serverStatus=0;
                    sprintf(payload, "*NOSERVER#");
                    shutdown(sock, 0);
                    close(sock);
                    sock = -1;
                }else{
               
                    ServerRetryCount = 0;
                    set_led_state(EVERYTHING_OK_LED); 
                    if (gpio_get_level(JUMPER) == 0)
                        sprintf(payload, "*MAC,%s,%s#", MAC_ADDRESS_ESP,SerialNumber);  // for GVC use ,
                    else
                        sprintf(payload, "*MAC:%s:%s#", MAC_ADDRESS_ESP,SerialNumber);  // for KP use :
                    uart_write_string_ln(payload);
                    if(MQTTRequired)
                    {
                    uart_write_string_ln("Publishing msg On Powr On");
                    mqtt_publish_msg(payload);
                    }
                    
                    int err = send(sock, payload, strlen(payload), 0);
                   
                    ESP_LOGI(TAG, "*Successfully connected#"); 
                    strcpy(RICON_DTIME,currentDateTime);
                    utils_nvs_set_str(NVS_RICON_DTIME, RICON_DTIME);
                    serverStatus=1;
                     sprintf(payload, "*QR:%s#",QrString); 
                    uart_write_string_ln(payload);

                    if (gpio_get_level(JUMPER) == 0)
                        ESP_LOGI(TAG, "*MAC,%s,%s#", MAC_ADDRESS_ESP,SerialNumber) ;
                    else
                        ESP_LOGI(TAG, "*MAC,%s,%s#", MAC_ADDRESS_ESP,SerialNumber) ;

                    sprintf(payload, "*WiFi,%d#", WiFiNumber); //actual when in production
                    err = send(sock, payload, strlen(payload), 0);

                    ESP_LOGI(TAG, "*%s#",FWVersion);
                    err = send(sock, FWVersion, strlen(FWVersion), 0);

                    sprintf(payload, "*QR-OK,%s#",QrString); 
          
                    uart_write_string_ln(payload);
                    uart_write_string_ln('*BOOTING#');

                    sprintf(payload,"*FW:%s#",FWVersion);
                    uart_write_string_ln(payload);


                    if (err < 0) {
                        ESP_LOGE(TAG, "*Error occurred during sending: errno %d#", errno);
                        shutdown(sock, 0);
                        close(sock);
                        sock = -1;
                    }else{
                        while(1){
                            /*
                            if(pending_tcp_packet){
                                pending_tcp_packet = false;
                                ESP_LOGI(TAG, "Sending to TCP Socket : %s", tcp_packet);
                                
                            }
                            */

                            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
                            // Error occurred during receiving
                            if (len < 0) {
                                ESP_LOGE(TAG, "*recv failed: errno %d#", errno);
                                ESP_LOGE(TAG, "*Shutting down socket and restarting...#");
                              
                                shutdown(sock, 0);
                                close(sock);
                                sock  = -1;
                                break;
                            }
                            else if(len == 0){
                                //No Data
                                if(millis() - lastPrint > 5000){
                                    lastPrint = millis();
                                    ESP_LOGI(TAG, "*Waiting For Data On TCP Port#");
                                }
                            }
                            // Data received
                            else {
                                blinkLEDNumber = 1;
                                LED4TCPPacket = 1;
                                ticks_100 = 0;
                                rx_event_pending = 1;
                                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                                ESP_LOGI(TAG, "Received %d bytes from %s Pulses %d:", len, server_ip_addr,pulses);
                                ESP_LOGI(TAG, "%s", rx_buffer);
                                char buf[len+1];

                      
                             
//                                 start genertaing pulses
//                                  INPUT  -   *V:{TID},{pin},{Pulses}#
//                                  *V-OK{TID}:{pin}:{Pulses}#
//                                   generate pulses on pin
//                                  *T-OK{TID}:{pin}:{Pulses}#
//                                  avoid duplicate TID
//                                  add up all pulses for specific pins
//                                  if to begin with
//                                  1 is 0, 2 is 0, 3 is 0.....
//                                  if I get commands say 3 pulses for pin1, 5 pulses for pin 2 etc.. add
//                                  1 is 3, 2 is 5 and so on
//                                  next time , add again to previous counts



                             
                             
                            
                             
                                    strcpy(InputVia,"TCP");
                                    AnalyzeInputPkt(rx_buffer,InputVia);
                                  
                                
//                                Write On UART
                                uart_write_string(rx_buffer);
                                // gpio_set_level(LedTCP, 1);
                                // vTaskDelay(200/portTICK_PERIOD_MS);
                                // gpio_set_level(LedTCP, 0);
                            }
                            vTaskDelay(1000 / portTICK_PERIOD_MS);
                        }
                    }
                }
            }
        }
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}

void sendHBT (void)
{
    char payload[400];
    for (;;) {
        ESP_LOGI(TAG, "*HBT,%s,%s#", MAC_ADDRESS_ESP,SerialNumber);
        sprintf(payload, "*HBT,%s,%s#", MAC_ADDRESS_ESP,SerialNumber); //actual when in production
        int err = send(sock, payload, strlen(payload), 0);
        // gpio_set_level(LedHBT, 1);
        // vTaskDelay(200/portTICK_PERIOD_MS);
        // gpio_set_level(LedHBT, 0);
        vTaskDelay(HBTDelay/portTICK_PERIOD_MS);
    }
}

void tcp_ip_client_send_str(const char * str){
    pending_tcp_packet = true;
    strcpy(tcp_packet, str);
    if(sock != -1){
        ESP_LOGI(TAG, "Sending packet to TCP socket : %s", str);
        uart_write_string(tcp_packet);
        int err = send(sock, tcp_packet, strlen(tcp_packet), 0);
        if (err < 0) {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            sock = -1;
            shutdown(sock, 0);
            close(sock);
        }
    }
}





