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
        uart_write_string_ln(Message);
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
            uart_write_string_ln(Message);
        }
    }
}


void AnalyzeInputPkt(const char *rx_buffer,const char *InputVia)
{
    const char payload[800];
    // sprintf(payload,"Received command %s from %s",rx_buffer,InputVia);
    // uart_write_string_ln(payload);
    // sprintf(payload,"RESPONSE-OK");
    // SendResponse(payload,InputVia);
// All Query Command
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
    
    else if(strncmp(rx_buffer, "*STATUS?#",9) == 0){
        if(fotaStatus==1)
        {
          sprintf(payload, "*FOTA#");
          SendResponse(payload,InputVia);  
        }
        else if(serverStatus==0)
        {
         sprintf(payload, "*NOSERVER#");
         SendResponse(payload,InputVia);
        }
        else if(serverStatus==1){
          sprintf(payload, "*QR:%s#",QrString); 
          SendResponse(payload,InputVia);
        }
    }
    else if(strncmp(rx_buffer, "*FW?#", 5) == 0){
        ESP_LOGI(InputVia, "*%s#",FWVersion);
        SendResponse(FWVersion,InputVia);
        tx_event_pending = 1;
        if (ledpin == 1)
            gpio_set_level(L1, ledstatus);
        if (ledpin == 2)
            gpio_set_level(L2, ledstatus);
        if (ledpin == 3)
            gpio_set_level(L3, ledstatus);
    }
    else if(strncmp(rx_buffer, "*URL?#", 6) == 0){
         sprintf(payload,"*URL,%s,%s,%s#",URLuserName,URLdateTime,FOTA_URL);
         SendResponse(payload,InputVia);
         tx_event_pending = 1;
     
     }
     else if(strncmp(rx_buffer, "*SSID?#", 7) == 0){
        sprintf(payload, "*SSID,%s,%s,%d,%s,%s,%s#",SSuserName,SSdateTime,WiFiNumber,WIFI_SSID_1,WIFI_SSID_2,WIFI_SSID_3); 
        SendResponse(payload,InputVia);
        tx_event_pending = 1;
    }
    else if(strncmp(rx_buffer, "*TC?#", 5) == 0){
      
        sprintf(payload, "*TC,%s,%d,%d,%d,%d,%d,%d,%d#", 
             UniqueTimeStamp,CashTotals[0],CashTotals[1],CashTotals[2],CashTotals[3],CashTotals[4],CashTotals[5],CashTotals[6]);
        SendResponse(payload,InputVia);
        tx_event_pending = 1;
    }
    else if(strncmp(rx_buffer, "*TV?#", 5) == 0){
       
        sprintf(payload, "*TV,%d,%d,%d,%d,%d,%d,%d#", 
            Totals[0], Totals[1], Totals[2], Totals[3], Totals[4], Totals[5], Totals[6]);
        SendResponse(payload,InputVia);
        tx_event_pending = 1;
    
    }
    else if(strncmp(rx_buffer, "*SIP?#", 6) == 0){
        sprintf(payload,"*SIP,%s,%s,%s,%d#",SIPuserName,SIPdateTime,server_ip_addr,
                                         sp_port );
        SendResponse(payload,InputVia);
        tx_event_pending = 1;
    }
    else if (strncmp(rx_buffer, "*ERASE?", 7) == 0){
        sprintf(payload,"*ERASE,%s,%s,%s#",ERASEuserName,ERASEdateTime,ErasedSerialNumber); 
        SendResponse(payload,InputVia);
    }
    else if(strncmp(rx_buffer, "*LS?#", 5) == 0){
        sprintf(payload,"LED State is %d",led_state);
        SendResponse(payload,InputVia);
    }
    // added on 20-12-24 as per EC10
    else if(strncmp(rx_buffer, "*CC?#", 5) == 0){
        sprintf(payload,"*CC,%s,%s,%s#",CCuserName,CCdateTime,UniqueTimeStamp);
        SendResponse(payload,InputVia);
        tx_event_pending = 1;
    }
    else if(strncmp(rx_buffer, "*VS?#",5) == 0){
                                     
        sprintf(payload, "*VS,%s,%d#",TID,AckPulseReceived); 
        SendResponse(payload,InputVia);
    } 
    else if(strncmp(rx_buffer, "*INH?#",6) == 0){
        if (INHInputValue !=0)
            INHInputValue = 1;
        ESP_LOGI(InputVia, "INH Values @ numValue %d ",INHInputValue);
        sprintf(payload, "*INH-IN,%s,%s,%d,%d#",INHuserName,INHdateTime,INHInputValue,INHOutputValue); 
        SendResponse(payload,InputVia);
    }     
    

//All Order Commands
    else if(strncmp(rx_buffer, "*TESTON#",8) == 0)
    {
        HardwareTestMode = 1;    
        pin = 0;    
        ESP_LOGI(InputVia, "*Hardware Test Started#");
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
        ESP_LOGI(InputVia, "*Hardware Test Stopped#");
        uart_write_string_ln("*Hardware Test Stopped#");
        RestartDevice();

     } 
     else if(strncmp(rx_buffer, "*HBT#",5) == 0)
     {
            sprintf(payload, "*HBT-OK#");
            send(sock, payload, strlen(payload), 0);
            ServerHBTTimeOut = 0;
            uart_write_string_ln("*SERVER HBT-OK#");
     }
     
     else if(strncmp(rx_buffer, "*RESTART#", 9) == 0){
        send(sock, "*RESTART-OK#", strlen("*RESTART-OK#"), 0);
        uart_write_string_ln("*Resetting device#");
        tx_event_pending = 1;
        RestartDevice();
    }
   






}