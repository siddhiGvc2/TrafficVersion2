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

                    
                    int err = send(sock, payload, strlen(payload), 0);
                    ESP_LOGI(TAG, "*Successfully connected#"); 
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

                                // if(strncmp(rx_buffer, "*SS:", 4) == 0){
                                //     sscanf(rx_buffer, "*SS:%[^:]:%[^:]:%[^#]#",SSuserName,SSdateTime, buf);
                                //     strcpy(WIFI_SSID_1, buf);
                                //     utils_nvs_set_str(NVS_SSID_1_KEY, WIFI_SSID_1);
                                //     utils_nvs_set_str(NVS_SSID_1_KEY, WIFI_SSID_1);
                                //     sprintf(payload, "*SS-OK,%s,%s#",SSuserName,SSdateTime);
                                //     utils_nvs_set_str(NVS_SS_USERNAME, SSuserName);
                                //     utils_nvs_set_str(NVS_SS_DATETIME, SSdateTime);
                                //     send(sock, payload, strlen(payload), 0);
                                //     tx_event_pending = 1;
                                // }
                                // if (strncmp(rx_buffer, "*SS:", 4) == 0) {
                                //     // Temporary buffers to read the values
                                //     char tempUserName[64], tempDateTime[64], tempBuf[64];

                                //     // Parse the input buffer
                                //     if (sscanf(rx_buffer, "*SS:%[^:]:%[^:]:%[^#]#", tempUserName, tempDateTime, tempBuf) == 3) {
                                //         // Check if any of the parsed values are empty
                                //         if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0) {
                                //             // Send error message if any required parameters are missing or invalid
                                //             const char* errorMsg = "Error: Missing or invalid parameters";
                                //             send(sock, errorMsg, strlen(errorMsg), 0);
                                //         } else {
                                //             // Copy parsed values to the actual variables
                                //             strcpy(SSuserName, tempUserName);
                                //             strcpy(SSdateTime, tempDateTime);
                                //             strcpy(WIFI_SSID_1, tempBuf);

                                //             // Save the values to non-volatile storage
                                //             utils_nvs_set_str(NVS_SSID_1_KEY, WIFI_SSID_1);
                                //             utils_nvs_set_str(NVS_SS_USERNAME, SSuserName);
                                //             utils_nvs_set_str(NVS_SS_DATETIME, SSdateTime);

                                //             // Format the success message and send it
                                //             sprintf(payload, "*SS-OK,%s,%s#", SSuserName, SSdateTime);
                                //             send(sock, payload, strlen(payload), 0);
                                //             tx_event_pending = 1;
                                //         }
                                //     } else {
                                //         // Send error message if parsing failed
                                //         const char* errorMsg = "Error: Invalid format";
                                //         send(sock, errorMsg, strlen(errorMsg), 0);
                                //     }
                                // }

                                // done by siddhi
                                // totPolarity
                                 if(strncmp(rx_buffer, "*CA?#", 5) == 0){
                                        ESP_LOGI(TAG, "CA Values @ numValue %d polarity %d username %s dateTime %s",pulseWitdh,polarity,CAuserName,CAdateTime);
                                        
                                        sprintf(payload, "*CA-OK,%s,%s,%d,%d#",CAuserName,CAdateTime,pulseWitdh,SignalPolarity); //actual when in production
                                        send(sock, payload, strlen(payload), 0);
                                 }

                                else if(strncmp(rx_buffer, "*TESTON#",8) == 0)
                                {
                                    HardwareTestMode = 1;    
                                    pin = 0;    
                                    ESP_LOGI(TAG, "*Hardware Test Started#");
                                    uart_write_string_ln("*Hardware Test Started#");
                                    // clear TC also
                                    for (int i = 0 ; i < 7 ; i++)
                                    {
                                        CashTotals[i] = 0;
                                    } 
                            }        
                                else if(strncmp(rx_buffer, "*TESTOFF#",9) == 0)
                                {
                                    HardwareTestMode = 0;    
                                    pin = 0;    
                                    ESP_LOGI(TAG, "*Hardware Test Stopped#");
                                    uart_write_string_ln("*Hardware Test Stopped#");
                                    RestartDevice();

                            }  
                            // added *HBT# on 251224
                               else if(strncmp(rx_buffer, "*HBT#",5) == 0)
                                {
                                    sprintf(payload, "*HBT-OK#");
                                    send(sock, payload, strlen(payload), 0);
                                    ServerHBTTimeOut = 0;
                                    uart_write_string_ln("*SERVER HBT-OK#");
                            }  
                               else if(strncmp(rx_buffer, "*D:",3) == 0){
                                    char tempBuf[100];
                                        sscanf(rx_buffer, "*D:%[^:#]#",tempBuf);
                                        strcpy(UniqueTimeStamp,tempBuf);
                                        sprintf(payload, "*D-OK,%s#",UniqueTimeStamp);
                                        utils_nvs_set_str(NVS_UNIX_TS,UniqueTimeStamp);
                                       
                                        send(sock, payload, strlen(payload), 0);
                                 }
                                  else if(strncmp(rx_buffer, "*QR:",4) == 0){
                                        char tempBuf[100];
                                        sscanf(rx_buffer, "*QR:%[^:#]#",tempBuf);
                                        strcpy(QrString,tempBuf);
                                        sprintf(payload, "*QR-OK,%s#",QrString);
                                        utils_nvs_set_str(NVS_QR_STRING,QrString);
                                       
                                        send(sock, payload, strlen(payload), 0);
                                        uart_write_string_ln(payload);
                                 }
                                  else if(strncmp(rx_buffer, "*QR?#",5) == 0){
                                     
                                        sprintf(payload, "*QR-OK,%s#",QrString); 
                                        send(sock, payload, strlen(payload), 0);
                                        uart_write_string_ln(payload);
                                 }      
                                  else if(strncmp(rx_buffer, "*VS?#",5) == 0){
                                     
                                        sprintf(payload, "*VS,%s,%d#",TID,AckPulseReceived); 
                                        send(sock, payload, strlen(payload), 0);
                                 }      

                                  else if(strncmp(rx_buffer, "*D?#",4) == 0){
                                     
                                        sprintf(payload, "*D-OK,%s#",UniqueTimeStamp); 
                                        send(sock, payload, strlen(payload), 0);
                                 }      
                                else if(strncmp(rx_buffer, "*INH?#",6) == 0){
                                        if (INHInputValue !=0)
                                            INHInputValue = 1;
                                        ESP_LOGI(TAG, "INH Values @ numValue %d ",INHInputValue);
                                        sprintf(payload, "*INH-IN,%s,%s,%d,%d#",INHuserName,INHdateTime,INHInputValue,INHOutputValue); 
                                        send(sock, payload, strlen(payload), 0);
                                 }
                                else if(strncmp(rx_buffer, "*INH:", 5) == 0){
                                        sscanf(rx_buffer, "*INH:%[^:]:%[^:]:%d#",INHuserName,INHdateTime, &INHOutputValue);
                                        if (INHOutputValue != 0)
                                        {
                                            INHOutputValue = 1;
                                            gpio_set_level(CINHO, 0);
                                        }
                                        else
                                        {
                                              gpio_set_level(CINHO, 1);
                                        }
                                        ESP_LOGI (TAG, "Set INH Output as %d",INHOutputValue);
                                        sprintf(payload, "*INH-DONE,%s,%s,%d#",SSuserName,SSdateTime,INHOutputValue);
                                        utils_nvs_set_str(NVS_INH_USERNAME, INHuserName);
                                        utils_nvs_set_str(NVS_INH_DATETIME, INHdateTime);
                                        send(sock, payload, strlen(payload), 0);
                                        // sprintf(payload, "*INH-DONE,%d#",INHOutputValue); //actual when in production
                                        // send(sock, payload, strlen(payload), 0);
                                        utils_nvs_set_int(NVS_INH_KEY, INHOutputValue);
                                }   
                                 else if(strncmp(rx_buffer, "*PT:", 4) == 0){
                                        char tempUserName[64], tempDateTime[64], tempBuf[64] ;
                                         if (sscanf(rx_buffer, "*PT:%[^:]:%[^:]:%[^:#]#", tempUserName, tempDateTime, tempBuf) == 3) {
                                        // Check if any of the parsed values are empty
                                        if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0 ) {
                                            // Send error message if any required parameters are missing or invalid
                                            const char* errorMsg = "Error: Missing or invalid parameters";
                                            send(sock, errorMsg, strlen(errorMsg), 0);
                                        }
                                        else{
                                       
                                        strcpy(PTuserName, tempUserName);
                                         strcpy(PTdateTime, tempDateTime);
                                         strcpy(PassThruValue, tempBuf);
                                       
                                        if (strstr(PassThruValue, "Y") == NULL && strstr(PassThruValue, "N") == NULL) {
                                            strcpy(PassThruValue, "Y");
                                        }

                                        ESP_LOGI (TAG, "Pass Thru %s",PassThruValue);
                                        sprintf(payload, "*PT-OK,%s,%s,%s#",PTuserName,PTdateTime,PassThruValue);
                                        utils_nvs_set_str(NVS_PT_USERNAME, PTuserName);
                                        utils_nvs_set_str(NVS_PT_DATETIME, PTdateTime);
                                        send(sock, payload, strlen(payload), 0);
                                   
                                        utils_nvs_set_str(NVS_PASS_THRU, PassThruValue);
                                        }
                                    }
                                }
                                  else if(strncmp(rx_buffer, "*PT?#",5) == 0){
                                    
                                        ESP_LOGI(TAG, "Pass Thru %s ",PassThruValue);
                                        sprintf(payload, "*PT,%s,%s,%s#",PTuserName,PTdateTime,PassThruValue); 
                                        send(sock, payload, strlen(payload), 0);
                                 }     


                                else if(strncmp(rx_buffer, "*SP:", 4) == 0){
                                        sscanf(rx_buffer, "*SP:%[^:]:%[^:]:%d#",SPuserName,SPdateTime, &jumperPort);
                                         sprintf(payload, "*SP-OK,%s,%s,%d#",SPuserName,SPdateTime,jumperPort);
                                        utils_nvs_set_str(NVS_SP_USERNAME, SPuserName);
                                        utils_nvs_set_str(NVS_SP_DATETIME, SPdateTime);
                                        send(sock, payload, strlen(payload), 0);
                                        // sprintf(payload, "*SP-OK,%d#",jumperPort); //actual when in production
                                        // send(sock, payload, strlen(payload), 0);
                                        utils_nvs_set_int(NVS_SERVER_PORT_KEY_JUMPER, jumperPort);
 
                                }        
                                else if(strncmp(rx_buffer, "*CA:", 4) == 0){
                                    char tempUserName[64], tempDateTime[64], tempBuf[64],tempBuf2[64];
                                    if (sscanf(rx_buffer, "*CA:%[^:]:%[^:]:%[^:]:%[^:#]#", tempUserName, tempDateTime, tempBuf,tempBuf2) == 4) {
                                        // Check if any of the parsed values are empty
                                        if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0 || strlen(tempBuf2) == 0) {
                                            // Send error message if any required parameters are missing or invalid
                                            const char* errorMsg = "Error: Missing or invalid parameters";
                                            send(sock, errorMsg, strlen(errorMsg), 0);
                                        }
                                        else{
                                         strcpy(CAuserName, tempUserName);
                                         strcpy(CAdateTime, tempDateTime);
                                         numValue = atoi(tempBuf);
                                         polarity = atoi(tempBuf2);
                                      
                                        ESP_LOGI(TAG, "Generate @ numValue %d polarity %d",numValue,polarity);
                                        sprintf(payload, "*CA-OK,%s,%s,%d,%d#",CAuserName,CAdateTime,numValue,polarity);
                                        utils_nvs_set_str(NVS_CA_USERNAME, CAuserName);
                                        utils_nvs_set_str(NVS_CA_DATETIME, CAdateTime);
                                        ESP_LOGI(TAG,"CA Values Saved %s,%s",CAuserName,CAdateTime);
                                        send(sock, payload, strlen(payload), 0);
                                        
                                        // sprintf(payload, "*CA-OK,%d,%d#",numValue,polarity); //actual when in production
                                        // send(sock, payload, strlen(payload), 0);
                                        // valid values are between 25 and 100
                                        if (numValue<10)
                                            numValue = 25;
                                        if (numValue>100)
                                            numValue = 100;
                                        // possible values are 0 and 1        
                                        if (polarity>0)
                                            polarity = 1;   
                                        polarity = 0;     
                                        pulseWitdh=numValue;
                                        SignalPolarity=polarity;

                                        tx_event_pending = 1;
                                        Out4094(0x00);
                                        utils_nvs_set_int(NVS_CA_KEY, numValue*2+polarity);
                                        }
                                    }
                                    else {
                                        // Send error message if parsing failed
                                        const char* errorMsg = "Error: Invalid format";
                                        send(sock, errorMsg, strlen(errorMsg), 0);
                                    }

                                }
                                else if(strncmp(rx_buffer, "*SS:", 4) == 0){
                                  char tempUserName[64], tempDateTime[64], tempBuf[64];

                                    // Parse the input buffer
                                    if (sscanf(rx_buffer, "*SS:%[^:]:%[^:]:%[^#]#", tempUserName, tempDateTime, tempBuf) == 3) {
                                        // Check if any of the parsed values are empty
                                        if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0) {
                                            // Send error message if any required parameters are missing or invalid
                                            const char* errorMsg = "Error: Missing or invalid parameters";
                                            send(sock, errorMsg, strlen(errorMsg), 0);
                                        } else {
                                            // Copy parsed values to the actual variables
                                            strcpy(SSuserName, tempUserName);
                                            strcpy(SSdateTime, tempDateTime);
                                            strcpy(WIFI_SSID_2, tempBuf);

                                            // Save the values to non-volatile storage
                                            utils_nvs_set_str(NVS_SSID_2_KEY, WIFI_SSID_2);
                                            utils_nvs_set_str(NVS_SS_USERNAME, SSuserName);
                                            utils_nvs_set_str(NVS_SS_DATETIME, SSdateTime);

                                            // Format the success message and send it
                                            sprintf(payload, "*SS-OK,%s,%s#", SSuserName, SSdateTime);
                                            send(sock, payload, strlen(payload), 0);
                                            tx_event_pending = 1;
                                        }
                                    } else {
                                        // Send error message if parsing failed
                                        const char* errorMsg = "Error: Invalid format";
                                        send(sock, errorMsg, strlen(errorMsg), 0);
                                    }
                                }
                                // else if(strncmp(rx_buffer, "*SS2:", 5) == 0){
                                //    char tempUserName[64], tempDateTime[64], tempBuf[64];

                                //     // Parse the input buffer
                                //     if (sscanf(rx_buffer, "*SS2:%[^:]:%[^:]:%[^#]#", tempUserName, tempDateTime, tempBuf) == 3) {
                                //         // Check if any of the parsed values are empty
                                //         if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0) {
                                //             // Send error message if any required parameters are missing or invalid
                                //             const char* errorMsg = "Error: Missing or invalid parameters";
                                //             send(sock, errorMsg, strlen(errorMsg), 0);
                                //         } else {
                                //             // Copy parsed values to the actual variables
                                //             strcpy(SS2userName, tempUserName);
                                //             strcpy(SS2dateTime, tempDateTime);
                                //             strcpy(WIFI_SSID_3, tempBuf);

                                //             // Save the values to non-volatile storage
                                //             utils_nvs_set_str(NVS_SSID_3_KEY, WIFI_SSID_3);
                                //             utils_nvs_set_str(NVS_SS2_USERNAME, SS2userName);
                                //             utils_nvs_set_str(NVS_SS2_DATETIME, SS2dateTime);

                                //             // Format the success message and send it
                                //             sprintf(payload, "*SS2-OK,%s,%s#", SS2userName, SS2dateTime);
                                //             send(sock, payload, strlen(payload), 0);
                                //             tx_event_pending = 1;
                                //         }
                                //     } else {
                                //         // Send error message if parsing failed
                                //         const char* errorMsg = "Error: Invalid format";
                                //         send(sock, errorMsg, strlen(errorMsg), 0);
                                //     }
                                // }else if(strncmp(rx_buffer, "*PW:", 4) == 0){
                                    
                                //     char tempUserName[64], tempDateTime[64], tempBuf[64];

                                //     // Parse the input buffer
                                //     if (sscanf(rx_buffer, "*PW:%[^:]:%[^:]:%[^#]#", tempUserName, tempDateTime, tempBuf) == 3) {
                                //         // Check if any of the parsed values are empty
                                //         if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0) {
                                //             // Send error message if any required parameters are missing or invalid
                                //             const char* errorMsg = "Error: Missing or invalid parameters";
                                //             send(sock, errorMsg, strlen(errorMsg), 0);
                                //         } else {
                                //             // Copy parsed values to the actual variables
                                //             strcpy(PWuserName, tempUserName);
                                //             strcpy(PWdateTime, tempDateTime);
                                //             strcpy(WIFI_PASS_1, tempBuf);

                                //             // Save the values to non-volatile storage
                                //             utils_nvs_set_str(NVS_PASS_1_KEY, WIFI_PASS_1);
                                //             utils_nvs_set_str(NVS_PW_USERNAME, PWuserName);
                                //             utils_nvs_set_str(NVS_PW_DATETIME, PWdateTime);

                                //             // Format the success message and send it
                                //             sprintf(payload, "*PW-OK,%s,%s#", PWuserName, PWdateTime);
                                //             send(sock, payload, strlen(payload), 0);
                                //             tx_event_pending = 1;
                                //         }
                                //     } else {
                                //         // Send error message if parsing failed
                                //         const char* errorMsg = "Error: Invalid format";
                                //         send(sock, errorMsg, strlen(errorMsg), 0);
                                //     }
                                // }
                                else if(strncmp(rx_buffer, "*PW:", 4) == 0){
                                    char tempUserName[64], tempDateTime[64], tempBuf[64];

                                    // Parse the input buffer
                                    if (sscanf(rx_buffer, "*PW:%[^:]:%[^:]:%[^#]#", tempUserName, tempDateTime, tempBuf) == 3) {
                                        // Check if any of the parsed values are empty
                                        if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0) {
                                            // Send error message if any required parameters are missing or invalid
                                            const char* errorMsg = "Error: Missing or invalid parameters";
                                            send(sock, errorMsg, strlen(errorMsg), 0);
                                        } else {
                                            // Copy parsed values to the actual variables
                                            strcpy(PWuserName, tempUserName);
                                            strcpy(PWdateTime, tempDateTime);
                                            strcpy(WIFI_PASS_2, tempBuf);

                                            // Save the values to non-volatile storage
                                            utils_nvs_set_str(NVS_PASS_2_KEY, WIFI_PASS_2);
                                            utils_nvs_set_str(NVS_PW_USERNAME, PWuserName);
                                            utils_nvs_set_str(NVS_PW_DATETIME, PWdateTime);

                                            // Format the success message and send it
                                            sprintf(payload, "*PW-OK,%s,%s#", PWuserName, PWdateTime);
                                            send(sock, payload, strlen(payload), 0);
                                            tx_event_pending = 1;
                                        }
                                    } else {
                                        // Send error message if parsing failed
                                        const char* errorMsg = "Error: Invalid format";
                                        send(sock, errorMsg, strlen(errorMsg), 0);
                                    }
                                }
                                // else if(strncmp(rx_buffer, "*PW2:", 5) == 0){
                                //      char tempUserName[64], tempDateTime[64], tempBuf[64];

                                //     // Parse the input buffer
                                //     if (sscanf(rx_buffer, "*PW2:%[^:]:%[^:]:%[^#]#", tempUserName, tempDateTime, tempBuf) == 3) {
                                //         // Check if any of the parsed values are empty
                                //         if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0) {
                                //             // Send error message if any required parameters are missing or invalid
                                //             const char* errorMsg = "Error: Missing or invalid parameters";
                                //             send(sock, errorMsg, strlen(errorMsg), 0);
                                //         } else {
                                //             // Copy parsed values to the actual variables
                                //             strcpy(PW2userName, tempUserName);
                                //             strcpy(PW2dateTime, tempDateTime);
                                //             strcpy(WIFI_PASS_3, tempBuf);

                                //             // Save the values to non-volatile storage
                                //             utils_nvs_set_str(NVS_PASS_3_KEY, WIFI_PASS_3);
                                //             utils_nvs_set_str(NVS_PW2_USERNAME, PW2userName);
                                //             utils_nvs_set_str(NVS_PW2_DATETIME, PW2dateTime);

                                //             // Format the success message and send it
                                //             sprintf(payload, "*PW2-OK,%s,%s#", PW2userName, PW2dateTime);
                                //             send(sock, payload, strlen(payload), 0);
                                //             tx_event_pending = 1;
                                //         }
                                //     } else {
                                //         // Send error message if parsing failed
                                //         const char* errorMsg = "Error: Invalid format";
                                //         send(sock, errorMsg, strlen(errorMsg), 0);
                                //     }
                                   
                                // }
                                else if(strncmp(rx_buffer, "*URL:", 5) == 0){
                                      char tempUserName[64], tempDateTime[64], tempBuf[64];

                                    // Parse the input buffer
                                    if (sscanf(rx_buffer, "*URL:%[^:]:%[^:]:%[^#]#", tempUserName, tempDateTime, tempBuf) == 3) {
                                        // Check if any of the parsed values are empty
                                        if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0) {
                                            // Send error message if any required parameters are missing or invalid
                                            const char* errorMsg = "Error: Missing or invalid parameters";
                                            send(sock, errorMsg, strlen(errorMsg), 0);
                                        } else {
                                                strcpy(URLuserName, tempUserName);
                                                strcpy(URLdateTime, tempDateTime);
                                                strcpy(FOTA_URL, tempBuf);
                                                utils_nvs_set_str(NVS_OTA_URL_KEY, FOTA_URL);
                                                sprintf(payload, "*URL-OK,%s,%s#",URLuserName,URLdateTime);
                                                utils_nvs_set_str(NVS_URL_USERNAME, URLuserName);
                                                utils_nvs_set_str(NVS_URL_DATETIME, URLdateTime);
                                                send(sock, payload, strlen(payload), 0);
                                                // send(sock, "*URL-OK#", strlen("*URL-OK#"), 0);
                                                tx_event_pending = 1;
                                        }
                                    }
                                    else {
                                        // Send error message if parsing failed
                                        const char* errorMsg = "Error: Invalid format";
                                        send(sock, errorMsg, strlen(errorMsg), 0);
                                    }
                                }
                                else if (strncmp(rx_buffer, "*SSID?#", 7) == 0){
                                sprintf(payload, "*SSID,%s,%s,%d,%s,%s,%s#",SSuserName,SSdateTime,WiFiNumber,WIFI_SSID_1,WIFI_SSID_2,WIFI_SSID_3); 
                                send(sock, payload, strlen(payload), 0);
                                tx_event_pending = 1;
                                }
                                else if(strncmp(rx_buffer, "*URL?#", 6) == 0){
                                    ESP_LOGI(TAG,"URL RECEIVED,%s,%s,%s",URLuserName,URLdateTime,FOTA_URL);
                                 char msg[600];
                                sprintf(msg,"*URL,%s,%s,%s#",URLuserName,URLdateTime,FOTA_URL); 
                                send(sock, msg, strlen(msg), 0);
                                tx_event_pending = 1;
                                }else if(strncmp(rx_buffer, "*FOTA:", 6) == 0){
                                    fotaStatus=1;
                                    send(sock, "*FOTA-OK#", strlen("*FOTA-OK#"), 0);
                                    
                                     uart_write_string_ln("FOTA-OK");
                                    send(sock,FOTA_URL,strlen(FOTA_URL),0);
                                    tx_event_pending = 1;
                                    http_fota();
                                }else if(strncmp(rx_buffer, "*SIP:", 5) == 0){
                                    
                                    char tempUserName[64], tempDateTime[64], tempBuf[64] ,tempBuf2[64];

                                    if (sscanf(rx_buffer, "*SIP:%[^:]:%[^:]:%[^:]#", tempUserName, tempDateTime, tempBuf) == 3) { 
                                          if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0  ) {
                                            // Send error message if any required parameters are missing or invalid
                                            const char* errorMsg = "Error: Missing or invalid parameters";
                                            send(sock, errorMsg, strlen(errorMsg), 0);
                                        }
                                        else{
                                                // sscanf(rx_buffer, "*SIP:%[^:]:%[^:]:%[^:]:%d#",SIPuserName,SIPdateTime,server_ip_addr,
                                                //     &sp_port);

                                                strcpy(SIPuserName, tempUserName);
                                                strcpy(SIPdateTime, tempDateTime);
                                               
                                              
                                                char buf[100];
                                                if ((atoi(tempBuf) == 0) || (atoi(tempBuf) >MAXSIPNUMBER))  
                                                {  
                                                    sprintf(payload, "*SIP-Error#");
                                                    ESP_LOGI(TAG,"*SIP-ERROR#");
                                                }else 
                                                {
                                                    sprintf(payload, "*SIP-OK,%s,%s#",SIPuserName,SIPdateTime);                                                   
                                                    utils_nvs_set_int(NVS_SIP_NUMBER, atoi(tempBuf));
                                                    utils_nvs_set_str(NVS_SIP_USERNAME, SIPuserName);
                                                    utils_nvs_set_str(NVS_SIP_DATETIME, SIPdateTime);
                                                    ESP_LOGI(TAG,"*SIP-OK,%s,%s#",SIPuserName,SIPdateTime);
                                                }    
                                                send(sock, payload, strlen(payload), 0);
                                                uart_write_string_ln(payload);
                                                tx_event_pending = 1;

                                        }
                                  
                                    }
                                    else{
                                        const char* errorMsg = "Error: Invalid format";
                                        send(sock, errorMsg, strlen(errorMsg), 0);  
                                    }
                                }else if (strncmp(rx_buffer, "*ERASE:", 7) == 0){
                                      char tempUserName[64], tempDateTime[64], tempBuf[64];
                                       // if seria no of device != ErasedSerialNumber then do not erase
                                            // if all values are not avalible then do not erase

                                    if (sscanf(rx_buffer, "*ERASE:%[^:]:%[^:]:%[^:#]", tempUserName, tempDateTime, tempBuf) == 3) { 
                                        if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0 ) {
                                            // Send error message if any required parameters are missing or invalid
                                            const char* errorMsg = "*Error: Missing or invalid parameters#";
                                            send(sock, errorMsg, strlen(errorMsg), 0);
                                            uart_write_string_ln(errorMsg);
                                        }
                                        else if (strcmp(tempBuf, SerialNumber) != 0) {
                                            sprintf (payload,"*Erase:Serial Not Matched Command:%s Actual:%s#",tempBuf,SerialNumber);
                                            send(sock, payload, strlen(payload), 0);
                                            uart_write_string_ln(payload);
                                        }

                                        else{
                                        
                                           
                                            strcpy(ERASEuserName, tempUserName);
                                            strcpy(ERASEdateTime, tempDateTime);
                                            strcpy(ErasedSerialNumber,tempBuf);

                                            utils_nvs_set_str(NVS_ERASE_USERNAME, ERASEuserName);
                                            utils_nvs_set_str(NVS_ERASE_DATETIME, ERASEdateTime);
                                            utils_nvs_set_str(NVS_ERASED_SERIAL_NUMBER, ErasedSerialNumber);
                                            utils_nvs_erase_all();
                                            utils_nvs_set_str(NVS_SERIAL_NUMBER, ErasedSerialNumber);
                                            send(sock, "*ERASE-OK#", strlen("*ERASE-OK#"), 0);
                                            uart_write_string_ln("*ERASE-OK#");  }
                                    }
                                    else{
                                          const char* errorMsg = "*Erase: error#";
                                        send(sock, errorMsg, strlen(errorMsg), 0);
                                        uart_write_string_ln(errorMsg);  
                                    }
                                }
                                else if (strncmp(rx_buffer, "*ERASE?", 7) == 0){
                                char msg[600];
                                sprintf(msg,"*ERASE,%s,%s,%s#",ERASEuserName,ERASEdateTime,ErasedSerialNumber); 
                                send(sock, msg, strlen(msg), 0);
                                   
                                }
                                else if(strncmp(rx_buffer, "*RESTART#", 9) == 0){
                                    send(sock, "*RESTART-OK#", strlen("*RESTART-OK#"), 0);
                                    uart_write_string_ln("*Resetting device#");
                                    tx_event_pending = 1;
                                    RestartDevice();
                                }
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



                                else if(strncmp(rx_buffer, "*V:", 3) == 0){
                                    if (edges == 0) 
                                    {
                                        AckPulseReceived = 0;
                                        sscanf(rx_buffer, "*V:%[^:]:%d:%d#",TID,&pin,&pulses);
                                        // if (INHInputValue == INHIBITLevel)
                                        // {
                                        // //   ESP_LOGI(TAG, "*UNIT DISABLED#");
                                        // //   send(sock, "*VEND DISABLED#", strlen("*VEND DISABLED#"), 0);
                                            
                                        // }
                                        // else if (TID != LastTID)

                                
                                        if (memcmp(TID, LastTID, 100) != 0)
                                        {
                                            uart_write_string_ln("*VENDING#");
                                            edges = pulses*2;  // doubled edges
                                            // strcpy(WIFI_PASS_2, buf);
                                            // utils_nvs_set_str(NVS_PASS_2_KEY, WIFI_PASS_2);
                                            ESP_LOGI(TAG, "*V-OK,%s,%d,%d#",TID,pin,pulses);
                                            sprintf(payload, "*V-OK,%s,%d,%d#", TID,pin,pulses); //actual when in production
                                            send(sock, payload, strlen(payload), 0);
                                            vTaskDelay(1000/portTICK_PERIOD_MS);
                                            sprintf(payload, "*T-OK,%s,%d,%d#",TID,pin,pulses); //actual when in production
                                            ESP_LOGI(TAG, "*T-OK,%s,%d,%d#",TID,pin,pulses);
                                            send(sock, payload, strlen(payload), 0);
                                            tx_event_pending = 1;
                                            Totals[pin-1] += pulses;
                                            strcpy(LastTID,TID);
                                            utils_nvs_set_str(NVS_LAST_TID,LastTID);
                                        }
                                        else
                                        {
                                          ESP_LOGI(TAG, "Duplicate TID");
                                          send(sock, "*DUP TID#", strlen("*DUP TID#"), 0);
                                        }  

                                    }
                                }



                                else if(strncmp(rx_buffer, "*SL:", 4) == 0){
                                    if (edges == 0)
                                    {
                                        sscanf(rx_buffer, "*SL:%[^:]:%[^:]:%d:%d#",SLuserName,SLdateTime,&ledpin,&ledstatus);
                                        // strcpy(WIFI_PASS_2, buf);
                                        // utils_nvs_set_str(NVS_PASS_2_KEY, WIFI_PASS_2);
                                        ESP_LOGI(TAG, "Set LED @ Pin %d Status %d",ledpin,ledstatus);
                                        send(sock, "*SL-OK#", strlen("*SL-OK#"), 0);
                                        tx_event_pending = 1;
                                        if (ledpin == 1)
                                            gpio_set_level(L1, ledstatus);
                                        if (ledpin == 2)
                                            gpio_set_level(L2, ledstatus);
                                        if (ledpin == 3)
                                            gpio_set_level(L3, ledstatus);
                                        
                                    }
                                }
                                // when TC command is received send totals

                                else if(strncmp(rx_buffer, "*TV?#", 5) == 0){
                                        sprintf(payload, "*TV,%d,%d,%d,%d,%d,%d,%d#", Totals[0],Totals[1],Totals[2],Totals[3],Totals[4],Totals[5],Totals[6]); //actual when in production
                                        send(sock, payload, strlen(payload), 0);
                                        ESP_LOGI(TAG, "TV Sending");
                                        
                                }

                                else if(strncmp(rx_buffer, "*TC?#", 5) == 0){
                                        sprintf(payload, "*TC,%s,%d,%d,%d,%d,%d,%d,%d#",UniqueTimeStamp,CashTotals[0],CashTotals[1],CashTotals[2],CashTotals[3],CashTotals[4],CashTotals[5],CashTotals[6]); //actual when in production
                                        send(sock, payload, strlen(payload), 0);
                                        ESP_LOGI(TAG, "*TC,%s,%d,%d,%d,%d,%d,%d,%d#",UniqueTimeStamp, CashTotals[0],CashTotals[1],CashTotals[2],CashTotals[3],CashTotals[4],CashTotals[5],CashTotals[6] );
                                        
                                }
                                  else if(strncmp(rx_buffer, "*SIP?#", 6) == 0){
                                        sprintf(payload, "*SIP,%s,%s,%s,%d#",SIPuserName,SIPdateTime,server_ip_addr,
                                        sp_port ); //actual when in production
                                        send(sock, payload, strlen(payload), 0);
                                        ESP_LOGI(TAG, "*SIP,%s,%s,%s,%d#",SIPuserName,SIPdateTime,server_ip_addr,
                                        sp_port );
                                        
                                }
                                else if(strncmp(rx_buffer, "*CC:", 4) == 0){
                                    sscanf(rx_buffer, "*CC:%[^:]:%[^:]:%[^#]#",CCuserName,CCdateTime,UniqueTimeStamp); // changed on 20-12-24 as per EC10
                                        ESP_LOGI(TAG, "*CC-OK#");
                                        // sprintf(payload, "*CC-OK#"); //actual when in production
                                          sprintf(payload, "*CC-OK,%s,%s,%s#",CCuserName,CCdateTime,UniqueTimeStamp);  // changed on 20-12-24 as per EC10
                                    utils_nvs_set_str(NVS_CC_USERNAME, CCuserName);
                                    utils_nvs_set_str(NVS_CC_DATETIME, CCdateTime);  // added on 20-12-24 as per EC10
                                    utils_nvs_set_str(NVS_UNIX_TS, UniqueTimeStamp);
                                    send(sock, payload, strlen(payload), 0);
                                        // send(sock, payload, strlen(payload), 0);
                                        for (int i = 0 ; i < 7 ; i++)
                                        {
                                            Totals[i] = 0;
                                            CashTotals[i] = 0;
                                        } 
                                        utils_nvs_set_int(NVS_CASH1_KEY, CashTotals[0]);
                                        utils_nvs_set_int(NVS_CASH2_KEY, CashTotals[1]);
                                        utils_nvs_set_int(NVS_CASH3_KEY, CashTotals[2]);
                                        utils_nvs_set_int(NVS_CASH4_KEY, CashTotals[3]);
                                        utils_nvs_set_int(NVS_CASH5_KEY, CashTotals[4]);
                                        utils_nvs_set_int(NVS_CASH6_KEY, CashTotals[5]);
                                        utils_nvs_set_int(NVS_CASH7_KEY, CashTotals[6]);
                                 }
                                 // added on 20-12-24 as per EC10
                                   else if(strncmp(rx_buffer, "*CC?#", 5) == 0){
                                         sprintf(payload,"*CC,%s,%s,%s#",CCuserName,CCdateTime,UniqueTimeStamp);
                                        send(sock, payload, strlen(payload), 0);
                                        tx_event_pending = 1;
                                      
                                        
                                    }


                                else if(strncmp(rx_buffer, "*FW?#", 5) == 0){
                                        ESP_LOGI(TAG, "*%s#",FWVersion);
                                        sprintf(payload,"*FW:%s#",FWVersion);
                                        send(sock, FWVersion, strlen(FWVersion), 0);
                                        uart_write_string_ln(payload);
                                        tx_event_pending = 1;
                                        if (ledpin == 1)
                                            gpio_set_level(L1, ledstatus);
                                        if (ledpin == 2)
                                            gpio_set_level(L2, ledstatus);
                                        if (ledpin == 3)
                                            gpio_set_level(L3, ledstatus);
                                    }
                                else if(strncmp(rx_buffer, "*RST:", 5) == 0){
                                        sscanf(rx_buffer, "*RST:%[^:]:%[^#]#",RSTuserName,RSTdateTime);
                                        ESP_LOGI(TAG, "**************Restarting after 3 second*******");
                                        utils_nvs_set_str(NVS_RST_USERNAME, RSTuserName);
                                        utils_nvs_set_str(NVS_RST_DATETIME, RSTdateTime);
                                        send(sock, "*RST-OK#", strlen("*RST-OK#"), 0);
                                        ESP_LOGI(TAG, "*RST-OK#");
                                        uart_write_string_ln("*Resetting device#");
                                        RestartDevice();
                                }
                                 else if(strncmp(rx_buffer, "*SN:", 4) == 0){
      
                                        if (strstr(SerialNumber,"999999"))
                                        {
                                            sscanf(rx_buffer, "*SN:%[^:]:%[^:]:%[^#]#",SNuserName,SNdateTime,SerialNumber);
                                            utils_nvs_set_str(NVS_SERIAL_NUMBER, SerialNumber);
                                            utils_nvs_set_str(NVS_SN_USERNAME, SNuserName);
                                            utils_nvs_set_str(NVS_SN_DATETIME, SNdateTime);
                                            send(sock, "*SN-OK#", strlen("*SN-OK#"), 0);
                                        }
                                        else
                                        {
                                             send(sock, "*SN CAN NOT BE SET#", strlen("*SN CAN NOT BE SET#"), 0);

                                        }
                                            tx_event_pending = 1;
                                        
                                    }
                                    else if(strncmp(rx_buffer, "*SN?#", 5) == 0){
                                        sprintf(payload, "*SN,%s,%s,%s#",SNuserName,SNdateTime,SerialNumber);
                                        send(sock, payload, strlen(payload), 0);
                                        tx_event_pending = 1;
                                        
                                    }
                                else{
                                    if(extractSubstring(rx_buffer, buf) == true){
                                        uart_write_string("*");
                                        uart_write_string(buf);
                                        uart_write_string("#");
                                        tx_event_pending = 1;
                                    }
                                }
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
