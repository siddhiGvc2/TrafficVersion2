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

#include "mqtt_client.h"

#include "externVars.h"
#include "calls.h"

static const char *TAG = "MQTT";

void InitMqtt (void);


int32_t MQTT_CONNEECTED = 1;

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */


 esp_mqtt_client_handle_t client = NULL;

 


void publish_message(const char *message, esp_mqtt_client_handle_t client) {
    // Publish the provided message to the MQTT topic
    esp_mqtt_client_publish(client, "GVC/KP/ALL", message, strlen(message), 0, 0);

    // Indicate that a transaction is pending
    tx_event_pending = 1;

    // Log the published message for debugging
    ESP_LOGI(TAG, "Published SIP message: %s", message);
}

void Publisher_Task(void *params)
{
  while (true)
  {
    if(MQTT_CONNEECTED)
    {
        publish_message("*HBT#", client);
        vTaskDelay(15000 / portTICK_PERIOD_MS);
    }
  }
}


static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, (int)event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    // Declare variables outside the switch statement
    char topic[356]; // Assuming a max topic length
    char data[256];  // Assuming a max data length
    char payload[500];
    char buf[500];

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        MQTT_CONNEECTED = 1;  // Ensure MQTT_CONNECTED is defined
        
      
        sprintf(topic, "GVC/KP/%s", SerialNumber);
        sprintf (payload,"Topic is %s",topic);
        uart_write_string_ln(payload);
        msg_id = esp_mqtt_client_subscribe(client, topic, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        MQTT_CONNEECTED = 0;
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        ESP_LOGI(TAG,"TOPIC=%.*s\r\n", event->topic_len, event->topic);
        ESP_LOGI(TAG,"DATA=%.*s\r\n", event->data_len, event->data);

        // Ensure topic and data are within bounds
        if (event->topic_len < sizeof(topic) && event->data_len < sizeof(data)) {
            strncpy(topic, event->topic, event->topic_len);
            topic[event->topic_len] = '\0';

            strncpy(data, event->data, event->data_len);
            data[event->data_len] = '\0';

            char expected_topic[150];
            sprintf(expected_topic, "GVC/KP/%s", SerialNumber);

            if (strcmp(topic, expected_topic) == 0) {
                if (strcmp(data, "*HBT#") == 0) {
                    ESP_LOGI(TAG, "Heartbeat message received.");
                } else if (strcmp(data, "SS1:") == 0) {
                    ESP_LOGI(TAG, "Command 1 received.");
                    // Execute action for COMMAND1
                } else if (strcmp(data, "COMMAND2") == 0) {
                    ESP_LOGI(TAG, "Command 2 received.");
                    // Execute action for COMMAND2
                } 
                else if(strncmp(data, "*SIP:", 5) == 0){
                                  
                    sscanf(data, "*SIP:%d#",&SipNumber);
                    strcpy(SIPuserName,"MQTT_LOCAL");
                    strcpy(SIPdateTime,"00/00/00");
                    char buf[100];
                    sprintf(payload, "*SIP-OK,%s,%s#",SIPuserName,SIPdateTime);
                    sprintf(buf, "%s",server_ip_addr);

                     if ((atoi(SipNumber) == 0) || (atoi(SipNumber) >MAXSIPNUMBER))  
                        {  
                            sprintf(payload, "*SIP-Error#");
                            ESP_LOGI(TAG,"*SIP-ERROR#");
                        }else 
                        {
                            sprintf(payload, "*SIP-OK,%s,%s#",SIPuserName,SIPdateTime);                                                   
                            utils_nvs_set_int(NVS_SIP_NUMBER, atoi(SipNumber));
                            utils_nvs_set_str(NVS_SIP_USERNAME, SIPuserName);
                            utils_nvs_set_str(NVS_SIP_DATETIME, SIPdateTime);
                            ESP_LOGI(TAG,"*SIP-OK,%s,%s#",SIPuserName,SIPdateTime);
                        } 
                
                    
                     publish_message(payload, client);
                    // send(sock, payload, strlen(payload), 0);
                    tx_event_pending = 1;
                }
                else if(strncmp(data, "*SIP?#", 6) == 0){
                    sprintf(payload, "*SIP,%s,%s,%s,%d#",SIPuserName,SIPdateTime,server_ip_addr,
                    sp_port ); //actual when in production
                     publish_message(payload, client);
                    ESP_LOGI(TAG, "*SIP,%s,%s,%s,%d#",SIPuserName,SIPdateTime,server_ip_addr,
                    sp_port );
                }
                 else if(strncmp(data, "*D:",3) == 0){
                    sscanf(data, "*D:%[^:#]#",UniqueTimeStamp);
                    utils_nvs_set_str(NVS_UNIX_TS,UniqueTimeStamp);
                    sprintf(payload, "*D-OK,%s#",UniqueTimeStamp); 
                    publish_message(payload, client);
                }  
                 else if(strncmp(data, "*D?#",4) == 0){
                    
                    sprintf(payload,"*D-OK,%s#",UniqueTimeStamp); 
                    publish_message(payload, client);
                }  
                else if(strncmp(data, "*CC#", 4) == 0){
                  
                    ESP_LOGI(TAG, "*CC-OK#");
                    strcpy(CCuserName,"MQTT_LOCAL");
                    strcpy(CCdateTime,"00/00/00");
                   
                    utils_nvs_set_str(NVS_CC_USERNAME, CCuserName);
                    utils_nvs_set_str(NVS_CC_DATETIME, CCdateTime);
                    sprintf(payload, "*CC-OK,%s,%s#",CCuserName,CCdateTime);
                     publish_message(payload, client);
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
                else if(strncmp(data, "*RST#", 5) == 0){
                        ESP_LOGI(TAG, "**************Restarting after 3 second*******");
                        strcpy(RSTuserName,"MQTT_LOCAL");
                        strcpy(RSTdateTime,"00/00/00");
                        sprintf(payload, "*RST-OK,%s,%s#",RSTuserName,RSTdateTime);
                         publish_message(payload, client);
                        ESP_LOGI(TAG, "*RST-OK#");
                        uart_write_string_ln("*Resetting device#");
                       RestartDevice();
                }
                 else if(strncmp(data, "*URL?#", 6) == 0){
                    ESP_LOGI(TAG,"URL RECEIVED,%s,%s,%s",URLuserName,URLdateTime,FOTA_URL);
                    char msg[600];
                    sprintf(msg,"*URL,%s,%s,%s#",URLuserName,URLdateTime,FOTA_URL); 
                     publish_message(msg, client);
                    tx_event_pending = 1;
                }
                else if(strncmp(data, "*CA?#", 5) == 0){
                    ESP_LOGI(TAG, "CA Values @ numValue %d polarity %d username %s dateTime %s",pulseWitdh,polarity,CAuserName,CAdateTime);
                    
                    sprintf(payload, "*CA-OK,%s,%s,%d,%d#",CAuserName,CAdateTime,pulseWitdh,SignalPolarity); //actual when in production
                    publish_message(payload, client);
                }
                else  if(strncmp(data, "*SS:", 4) == 0){
                    
                    sscanf(data, "*SS:%[^#]#", buf);
                    strcpy(SSuserName,"MQTT_LOCAL");
                    strcpy(SSdateTime,"00/00/00");
                    strcpy(WIFI_SSID_1, buf);
                    utils_nvs_set_str(NVS_SSID_1_KEY, WIFI_SSID_1);
                    utils_nvs_set_str(NVS_SSID_1_KEY, WIFI_SSID_1);
                    sprintf(payload, "*SS-OK,%s,%s#",SSuserName,SSdateTime);
                    utils_nvs_set_str(NVS_SS_USERNAME, SSuserName);
                    utils_nvs_set_str(NVS_SS_DATETIME, SSdateTime);
                    publish_message(payload, client);
                    tx_event_pending = 1;
                }
                else  if(strncmp(data, "*SN:", 4) == 0){
                    
                    sscanf(data, "*SS:%[^#]#",buf);
                    strcpy(SSuserName,"MQTT_LOCAL");
                    strcpy(SSdateTime,"00/00/00");
                  
                    sprintf(payload, "*SN-OK,%s,%s#",SNuserName,SNdateTime);
                    utils_nvs_set_str(NVS_SN_USERNAME, SNuserName);
                    utils_nvs_set_str(NVS_SN_DATETIME, SNdateTime);
                    publish_message(payload, client);
                    tx_event_pending = 1;
                }
                else if(strncmp(data, "*SN?#",5) == 0){
                   
                    ESP_LOGI(TAG, "SN?# %s ",SerialNumber);
                    sprintf(payload, "*SN,%s,%s,%s#",SNuserName,SNdateTime,SerialNumber); 
                    publish_message(payload, client);
                }
                else if(strncmp(data, "*INH?#",6) == 0){
                    if (INHInputValue !=0)
                        INHInputValue = 1;
                    ESP_LOGI(TAG, "INH Values @ numValue %d ",INHInputValue);
                    sprintf(payload, "*INH-IN,%s,%s,%d,%d#",INHuserName,INHdateTime,INHInputValue,INHOutputValue); 
                    publish_message(payload, client);
                }
                 else if(strncmp(data, "*INH:", 5) == 0){
                        sscanf(data, "*INH:%d#",&INHOutputValue);
                        strcpy(INHuserName,"MQTT_LOCAL");
                        strcpy(INHdateTime,"00/00/00");
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
                        sprintf(payload, "*INH-DONE,%s,%s,%d#",INHuserName,INHdateTime,INHOutputValue);
                        utils_nvs_set_str(NVS_INH_USERNAME, INHuserName);
                        utils_nvs_set_str(NVS_INH_DATETIME, INHdateTime);
                        publish_message(payload, client);
                        // sprintf(payload, "*INH-DONE,%d#",INHOutputValue); //actual when in production
                        // send(sock, payload, strlen(payload), 0);
                        utils_nvs_set_int(NVS_INH_KEY, INHOutputValue);
                }
                else if(strncmp(data, "*CA:", 4) == 0){
                    sscanf(data, "*CA:%d:%d#",&numValue,&polarity);
                    strcpy(CAuserName,"MQTT_LOCAL");
                    strcpy(CAdateTime,"00/00/00");
                    ESP_LOGI(TAG, "Generate @ numValue %d polarity %d",numValue,polarity);
                    sprintf(payload, "*CA-OK,%s,%s,%d,%d#",CAuserName,CAdateTime,numValue,polarity);
                    utils_nvs_set_str(NVS_CA_USERNAME, CAuserName);
                    utils_nvs_set_str(NVS_CA_DATETIME, CAdateTime);
                    ESP_LOGI(TAG,"CA Values Saved %s,%s",CAuserName,CAdateTime);
                    publish_message(payload, client);
                    if (numValue<10)
                        numValue = 25;
                    if (numValue>100)
                        numValue = 100;
                    // possible values are 0 and 1        
                    if (polarity>0)
                        polarity = 1;    
                    polarity = 0; // hard code polairty to 0
                    pulseWitdh=numValue;
                    SignalPolarity=polarity;
                    tx_event_pending = 1;
                    Out4094(0x00);
                    utils_nvs_set_int(NVS_CA_KEY, numValue*2+polarity);
                }
                else if(strncmp(data, "*SS1:", 5) == 0){
                    sscanf(data, "*SS1:%[^#]#", buf);
                    strcpy(WIFI_SSID_2, buf);
                    strcpy(SS1userName,"MQTT_LOCAL");
                    strcpy(SS1dateTime,"00/00/00");
                    utils_nvs_set_str(NVS_SSID_2_KEY, WIFI_SSID_2);
                    sprintf(payload, "*SS1-OK,%s,%s#",SS1userName,SS1dateTime);
                    utils_nvs_set_str(NVS_SS1_USERNAME, SS1userName);
                    utils_nvs_set_str(NVS_SS1_DATETIME, SS1dateTime);
                    publish_message(payload, client);
                    tx_event_pending = 1;
                }
                else if(strncmp(data, "*PW:", 4) == 0){
                    sscanf(data, "*PW:%[^#]#", buf);
                    strcpy(WIFI_PASS_1, buf);
                    strcpy(PWuserName,"MQTT_LOCAL");
                    strcpy(PWdateTime,"00/00/00");
                    utils_nvs_set_str(NVS_PASS_1_KEY, WIFI_PASS_1);
                    sprintf(payload, "*PW-OK,%s,%s#",PWuserName,PWdateTime);
                    utils_nvs_set_str(NVS_PW_USERNAME, PWuserName);
                    utils_nvs_set_str(NVS_PW_DATETIME, PWdateTime);
                    
                    // send(sock, "*PW-OK#", strlen("*PW-OK#"), 0);
                    tx_event_pending = 1;
                }
                else if(strncmp(data, "*PW1:", 5) == 0){
                    sscanf(data, "*PW1:%[^#]#", buf);
                    strcpy(WIFI_PASS_2, buf);
                    strcpy(PW1userName,"MQTT_LOCAL");
                    strcpy(PW1dateTime,"00/00/00");
                    utils_nvs_set_str(NVS_PASS_2_KEY, WIFI_PASS_2);
                    sprintf(payload, "*PW1-OK,%s,%s#",PW1userName,PW1dateTime);
                    utils_nvs_set_str(NVS_PW1_USERNAME, PW1userName);
                    utils_nvs_set_str(NVS_PW1_DATETIME, PW1dateTime);
                    publish_message(payload, client);
                    tx_event_pending = 1;
                }
                else if(strncmp(data, "*URL:", 5) == 0){
                    sscanf(data, "*URL:%[^#]#",buf);
                    strcpy(FOTA_URL, buf);
                    strcpy(URLuserName,"MQTT_LOCAL");
                    strcpy(URLdateTime,"00/00/00");
                    utils_nvs_set_str(NVS_OTA_URL_KEY, FOTA_URL);
                    sprintf(payload, "*URL-OK,%s,%s#",URLuserName,URLdateTime);
                    utils_nvs_set_str(NVS_URL_USERNAME, URLuserName);
                    utils_nvs_set_str(NVS_URL_DATETIME, URLdateTime);
                    publish_message(payload, client);
                    tx_event_pending = 1;
                }
                else if (strncmp(data, "*SSID?#", 7) == 0){
                    sprintf(payload, "*SSID,%s,%s,%d,%s,%s,%s#",SSuserName,SSdateTime,WiFiNumber,WIFI_SSID_1,WIFI_SSID_2,WIFI_SSID_3); 
                    publish_message(payload, client);
                    tx_event_pending = 1;
                }
                else if (strncmp(data, "*ERASE:", 7) == 0){

                    sscanf(payload, "*ERASE:%[^:]#",ErasedSerialNumber);
                     if (strcmp(ErasedSerialNumber, SerialNumber) != 0) {
                        const char* errorMsg = "Erase:Serial Not Matched";
                       publish_message("Erase:Serial Not Matched", client);
                    }
                    else{
                    strcpy(ERASEuserName,"MQTT_LOCAL");
                    strcpy(ERASEdateTime,"00/00/00");
                    utils_nvs_set_str(NVS_ERASE_USERNAME, ERASEuserName);
                    utils_nvs_set_str(NVS_ERASE_DATETIME, ERASEdateTime);
                    utils_nvs_set_str(NVS_ERASED_SERIAL_NUMBER, ErasedSerialNumber);
                    utils_nvs_erase_all();
                    utils_nvs_set_str(NVS_SERIAL_NUMBER, ErasedSerialNumber);
                    publish_message("*ERASE-OK#", client);
                    }
                }
                 else if (strncmp(data, "*ERASE?", 7) == 0){
            
                sprintf(payload,"*ERASE,%s,%s,%s#",ERASEuserName,ERASEdateTime,ErasedSerialNumber); 
                  publish_message(payload, client);
                    
                }
                else if(strncmp(data, "*RESTART#", 9) == 0){
                    publish_message("*RESTART-OK#", client);
                    uart_write_string_ln("*Resetting device#");
                    tx_event_pending = 1;
                   RestartDevice();
                }
                else if(strncmp(data, "*V:", 3) == 0){
                    if (edges == 0) 
                    {
                        sscanf(data, "*V:%[^:]:%d:%d#",TID,&pin,&pulses);
                        // if (INHInputValue == INHIBITLevel)
                        // {
                        //     ESP_LOGI(TAG, "*UNIT DISABLED#");
                        //     publish_message("*VEND DISABLED#", client);
                            
                            
                        // }
//                        else if (TID != LastTID)
                         if (memcmp(TID, LastTID, 100) != 0)
                        {
                            edges = pulses*2;  // doubled edges
                            // strcpy(WIFI_PASS_2, buf);
                            // utils_nvs_set_str(NVS_PASS_2_KEY, WIFI_PASS_2);
                            ESP_LOGI(TAG, "*V-OK,%s,%d,%d#",TID,pin,pulses);
                            sprintf(payload, "*V-OK,%s,%d,%d#", TID,pin,pulses); //actual when in production
                            publish_message(payload, client);
                            vTaskDelay(1000/portTICK_PERIOD_MS);
                            sprintf(payload, "*T-OK,%s,%d,%d#",TID,pin,pulses); //actual when in production
                            ESP_LOGI(TAG, "*T-OK,%s,%d,%d#",TID,pin,pulses);
                            publish_message(payload, client);
                            tx_event_pending = 1;
                            Totals[pin-1] += pulses;
                            strcpy(LastTID,TID);
                            utils_nvs_set_str(NVS_LAST_TID,LastTID);
                        }
                        else
                        {
                            ESP_LOGI(TAG, "Duplicate TID");
                            publish_message("*DUP TID#", client);
                          
                        }  

                    }
                }
                else if(strncmp(data, "*SL:", 4) == 0){
                    if (edges == 0)
                    {
                        sscanf(data, "*SL:%d:%d#",&ledpin,&ledstatus);
                        // strcpy(WIFI_PASS_2, buf);
                        // utils_nvs_set_str(NVS_PASS_2_KEY, WIFI_PASS_2);
                        ESP_LOGI(TAG, "Set LED @ Pin %d Status %d",ledpin,ledstatus);
                        publish_message("SL-OK", client);
                        tx_event_pending = 1;
                        if (ledpin == 1)
                            gpio_set_level(L1, ledstatus);
                        if (ledpin == 2)
                            gpio_set_level(L2, ledstatus);
                        if (ledpin == 3)
                            gpio_set_level(L3, ledstatus);
                        
                    }
                }
                else if(strncmp(data, "*TV?#", 5) == 0){
                        sprintf(payload, "*TV,%d,%d,%d,%d,%d,%d,%d#", Totals[0],Totals[1],Totals[2],Totals[3],Totals[4],Totals[5],Totals[6]); //actual when in production
                        publish_message(payload, client);
                        ESP_LOGI(TAG, "TV Sending");
                        
                }
                else if(strncmp(data, "*TC?#", 5) == 0){
                        sprintf(payload, "*TC,%s,%d,%d,%d,%d,%d,%d,%d#",UniqueTimeStamp, CashTotals[0],CashTotals[1],CashTotals[2],CashTotals[3],CashTotals[4],CashTotals[5],CashTotals[6]); //actual when in production
                        publish_message(payload, client);
                        ESP_LOGI(TAG, "*TC,%s,%d,%d,%d,%d,%d,%d,%d#",UniqueTimeStamp, CashTotals[0],CashTotals[1],CashTotals[2],CashTotals[3],CashTotals[4],CashTotals[5],CashTotals[6] );
                        
                }
                 else if(strncmp(data, "*FW?#", 5) == 0){
                    ESP_LOGI(TAG, "*%s#",FWVersion);
                     publish_message(FWVersion, client);
                    tx_event_pending = 1;
                    if (ledpin == 1)
                        gpio_set_level(L1, ledstatus);
                    if (ledpin == 2)
                        gpio_set_level(L2, ledstatus);
                    if (ledpin == 3)
                        gpio_set_level(L3, ledstatus);
                }
                else if(strncmp(data, "*FOTA:", 6) == 0){
                    strcpy(FOTAuserName,"MQTT_LOCAL");
                    strcpy(FOTAdateTime,"00/00/00");
                    publish_message("*FOTA-OK#", client);
                    publish_message(FOTA_URL, client);
                    utils_nvs_set_str(NVS_FOTA_USERNAME, FOTAuserName);
                    utils_nvs_set_str(NVS_FOTA_DATETIME, FOTAdateTime);
                    tx_event_pending = 1;
                    http_fota();
                }
                 else if(strncmp(data, "*SP:", 4) == 0){
                        sscanf(data, "*SP:%d#",&jumperPort);
                        strcpy(SPuserName,"MQTT_LOCAL");
                        strcpy(SPdateTime,"00/00/00");
                        sprintf(payload, "*SP-OK,%s,%s,%d#",SPuserName,SPdateTime,jumperPort);
                        utils_nvs_set_str(NVS_SP_USERNAME, SPuserName);
                        utils_nvs_set_str(NVS_SP_DATETIME, SPdateTime);
                        publish_message(payload, client);
                        utils_nvs_set_int(NVS_SERVER_PORT_KEY_JUMPER, jumperPort);

                } 
                else if(strncmp(data, "*SS2:", 5) == 0){
                    sscanf(data, "*SS2:%[^#]#",buf);
                    strcpy(WIFI_SSID_3, buf);
                    strcpy(SS2userName,"MQTT_LOCAL");
                    strcpy(SS2dateTime,"00/00/00");
                    utils_nvs_set_str(NVS_SSID_3_KEY, WIFI_SSID_3);
                    utils_nvs_set_str(NVS_SS2_USERNAME, SS2userName);
                    utils_nvs_set_str(NVS_SS2_DATETIME, SS2dateTime);
                    publish_message("*SS2-OK#", client);
                    tx_event_pending = 1;
                }
                else if(strncmp(data, "*PW2:", 5) == 0){
                    sscanf(data, "*PW2:%[^#]#", buf);
                    strcpy(WIFI_PASS_3, buf);
                    strcpy(PW2userName,"MQTT_LOCAL");
                    strcpy(PW2dateTime,"00/00/00");
                    utils_nvs_set_str(NVS_PASS_3_KEY, WIFI_PASS_3);
                    utils_nvs_set_str(NVS_PW2_USERNAME, PW2userName);
                    utils_nvs_set_str(NVS_PW2_DATETIME, PW2dateTime);
                    publish_message("*PW2-OK#", client);
                    tx_event_pending = 1;
                }  
                else {
                    ESP_LOGI(TAG, "Unknown message received.");
                }
            }
        } else {
            ESP_LOGE(TAG, "Received topic/data too large");
        }
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        // Add more error handling here if needed
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}


void mqtt_app_start(void)
{
    ESP_LOGI(TAG, "STARTING MQTT");
    esp_mqtt_client_config_t mqttConfig = {
         .broker.address.uri = "mqtt://zest-iot.in:1883",
        .session.protocol_ver = MQTT_PROTOCOL_V_3_1_1,
        .network.disable_auto_reconnect = true,
        .credentials.username = "123",
        .credentials.authentication.password = "456",
        .session.last_will.topic = "GVC/VM/00002",
        .session.last_will.msg = "i will leave",
        .session.last_will.msg_len = 12,
        .session.last_will.qos = 1,
        .session.last_will.retain = true,};
    
    client = esp_mqtt_client_init(&mqttConfig);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}





void InitMqtt (void)
{
     xTaskCreate(Publisher_Task, "Publisher_Task", 1024 * 5, NULL, 5, NULL);
    
     ESP_LOGI(TAG,"MQTT Initiated");
}