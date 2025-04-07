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



void SendResponse(const char *Message,const char *OutputVia)
{
    if(strcmp(OutputVia, "TCP") == 0)
    {
        send(sock, Message, strlen(Message), 0);
    }
    else if(strcmp(OutputVia, "UART") == 0)
    {
        uart_write_string_ln(Message);
    }
    else if(strcmp(OutputVia, "MQTT") == 0)
    {
        if(MQTTRequired)
        {
            mqtt_publish_msg(Message);
        }
    }
}


void AnalyzeInputPkt(const char *rx_buffer,const char *InputVia)
{
    const char payload[400];
    // sprintf(payload,"Received command %s from %s",rx_buffer,InputVia);
    // uart_write_string_ln(payload);
    // sprintf(payload,"RESPONSE-OK");
    // SendResponse(payload,InputVia);

    if(strncmp(rx_buffer, "*CA?#", 5) == 0){
        sprintf(payload,"*CA-OK,%s,%s,%d,%d#",CAuserName,CAdateTime,pulseWitdh,SignalPolarity);
        SendResponse(payload,InputVia);
        tx_event_pending = 1;
    }
    else if(strncmp(rx_buffer, "*PT?#", 5) == 0){
        sprintf(payload, "*PT,%s,%s,%s#",PTuserName,PTdateTime,PassThruValue); //actual when in production
        SendResponse(payload,InputVia);
        tx_event_pending = 1;
     }
     else if(strncmp(rx_buffer, "*SN?#", 5) == 0){
        sprintf(payload, "*SN,%s,%s,%s#",SNuserName,SNdateTime,SerialNumber); //actual when in production
        SendResponse(payload,InputVia);
        tx_event_pending = 1;
    }
    else if(strncmp(rx_buffer, "*D?#",4) == 0){
        sprintf(payload, "*D-OK,%s#",UniqueTimeStamp); 
        SendResponse(payload,InputVia);
    }
    else if(strncmp(rx_buffer, "*QR?#",5) == 0){
        sprintf(payload, "*QR-OK,%s#",QrString); 
        SendResponse(payload,InputVia);
    }       



}