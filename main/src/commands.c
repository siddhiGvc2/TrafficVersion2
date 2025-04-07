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
    else if(strncmp(rx_buffer, "*D:",3) == 0){
             char tempBuf[100];
            sscanf(rx_buffer, "*D:%[^:#]#",tempBuf);
            strcpy(UniqueTimeStamp,tempBuf);
            sprintf(payload, "*D-OK,%s#",UniqueTimeStamp);
            utils_nvs_set_str(NVS_UNIX_TS,UniqueTimeStamp);
            SendResponse(payload,InputVia);
     }
     else if(strncmp(rx_buffer, "*QR:",4) == 0){
        char tempBuf[100];
        sscanf(rx_buffer, "*QR:%[^:#]#",tempBuf);
        strcpy(QrString,tempBuf);
        sprintf(payload, "*QR-OK,%s#",QrString);
        utils_nvs_set_str(NVS_QR_STRING,QrString);
       
        SendResponse(payload,InputVia);
        
     }
     else if(strncmp(rx_buffer, "*V:", 3) == 0){
        if (edges == 0) 
        {
            AckPulseReceived = 0;
            sscanf(rx_buffer, "*V:%[^:]:%d:%d#",TID,&pin,&pulses);
    
            if (memcmp(TID, LastTID, 100) != 0)
            {
                uart_write_string_ln("*VENDING#");
                edges = pulses*2;  // doubled edges
                // strcpy(WIFI_PASS_2, buf);
                // utils_nvs_set_str(NVS_PASS_2_KEY, WIFI_PASS_2);
                ESP_LOGI(InputVia, "*V-OK,%s,%d,%d#",TID,pin,pulses);
                sprintf(payload, "*V-OK,%s,%d,%d#", TID,pin,pulses); //actual when in production
                SendResponse(payload,InputVia);
                vTaskDelay(1000/portTICK_PERIOD_MS);
                sprintf(payload, "*T-OK,%s,%d,%d#",TID,pin,pulses); //actual when in production
                ESP_LOGI(InputVia, "*T-OK,%s,%d,%d#",TID,pin,pulses);
                SendResponse(payload,InputVia);
                tx_event_pending = 1;
                Totals[pin-1] += pulses;
                strcpy(LastTID,TID);
                utils_nvs_set_str(NVS_LAST_TID,LastTID);
            }
            else
            {
              ESP_LOGI(InputVia, "Duplicate TID");
              SendResponse("*DUP TID#",InputVia);
              
            }  

        }
    }
// All settings commands
    else if(strncmp(rx_buffer, "*INH:", 5) == 0){
        if(strcmp(InputVia, "TCP") == 0)
        {
            sscanf(rx_buffer, "*INH:%[^:]:%[^:]:%d#",INHuserName,INHdateTime, &INHOutputValue);
          
        }
        else if(strcmp(InputVia, "UART") == 0)
        {
            sscanf(rx_buffer, "*INH:%d#",&INHOutputValue);
            strcpy(INHuserName,"LOCAL");
            strcpy(INHdateTime,"00/00/00");
        }
        else if(strcmp(InputVia, "MQTT") == 0)
        {
            sscanf(rx_buffer, "*INH:%d#",&INHOutputValue);
            strcpy(INHuserName,"MQTT_LOCAL");
            strcpy(INHdateTime,"00/00/00");
        }

        if (INHOutputValue != 0)
        {
            INHOutputValue = 1;
            gpio_set_level(CINHO, 0);
        }
        else
        {
              gpio_set_level(CINHO, 1);
        }
        ESP_LOGI (InputVia, "Set INH Output as %d",INHOutputValue);
        sprintf(payload, "*INH-DONE,%s,%s,%d#",SSuserName,SSdateTime,INHOutputValue);
        utils_nvs_set_str(NVS_INH_USERNAME, INHuserName);
        utils_nvs_set_str(NVS_INH_DATETIME, INHdateTime);
        SendResponse(payload,InputVia);
        utils_nvs_set_int(NVS_INH_KEY, INHOutputValue);
     }  
     else if(strncmp(rx_buffer, "*PT:", 4) == 0){
        if(strcmp(InputVia, "TCP") == 0)
        {
            char tempUserName[64], tempDateTime[64], tempBuf[64] ;
            if (sscanf(rx_buffer, "*PT:%[^:]:%[^:]:%[^:#]#", tempUserName, tempDateTime, tempBuf) == 3) {
            // Check if any of the parsed values are empty
            if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0 ) {
                // Send error message if any required parameters are missing or invalid
                const char* errorMsg = "Error: Missing or invalid parameters";
                SendResponse(errorMsg,InputVia);
            }
            else{
        
            strcpy(PTuserName, tempUserName);
            strcpy(PTdateTime, tempDateTime);
            strcpy(PassThruValue, tempBuf);
             }
            }
        }
        else if(strcmp(InputVia,"UART") == 0)
        {
            sscanf(rx_buffer, "*PT:%[^#]#",PassThruValue);
            strcpy(PTuserName, "LOCAL");
            strcpy(PTdateTime, "00/00/00");
        }
        else if(strcmp(InputVia,"MQTT") == 0)
        {
            sscanf(rx_buffer, "*PT:%[^#]#",PassThruValue);
            strcpy(PTuserName, "MQTT_LOCAL");
            strcpy(PTdateTime, "00/00/00");
        }

        if (strstr(PassThruValue, "Y") == NULL && strstr(PassThruValue, "N") == NULL) {
            strcpy(PassThruValue, "Y");
        }

        ESP_LOGI (InputVia, "Pass Thru %s",PassThruValue);
        sprintf(payload, "*PT-OK,%s,%s,%s#",PTuserName,PTdateTime,PassThruValue);
        utils_nvs_set_str(NVS_PT_USERNAME, PTuserName);
        utils_nvs_set_str(NVS_PT_DATETIME, PTdateTime);
        SendResponse(payload,InputVia);
        utils_nvs_set_str(NVS_PASS_THRU, PassThruValue);

     }
     else if(strncmp(rx_buffer, "*SP:", 4) == 0){
        if(strcmp(InputVia, "TCP") == 0)
        {
            sscanf(rx_buffer, "*SP:%[^:]:%[^:]:%d#",SPuserName,SPdateTime, &jumperPort);
        }
        else if(strcmp(InputVia, "UART") == 0)
        {
            sscanf(rx_buffer, "*SP:%d#",&jumperPort);
            strcpy(SPuserName,"LOCAL");
            strcpy(SPdateTime,"00/00/00");
        }
        else if(strcmp(InputVia, "MQTT") == 0)
        {
            sscanf(rx_buffer, "*SP:%d#",&jumperPort);
            strcpy(SPuserName,"MQTT_LOCAL");
            strcpy(SPdateTime,"00/00/00");
        }

        sprintf(payload, "*SP-OK,%s,%s,%d#",SPuserName,SPdateTime,jumperPort);
        utils_nvs_set_str(NVS_SP_USERNAME, SPuserName);
        utils_nvs_set_str(NVS_SP_DATETIME, SPdateTime);
        SendResponse(payload,InputVia);
        utils_nvs_set_int(NVS_SERVER_PORT_KEY_JUMPER, jumperPort);


     }
     else if(strncmp(rx_buffer, "*CA:", 4) == 0){
        if(strcmp(InputVia, "TCP") == 0)
        {
            char tempUserName[64], tempDateTime[64], tempBuf[64],tempBuf2[64];
            if (sscanf(rx_buffer, "*CA:%[^:]:%[^:]:%[^:]:%[^:#]#", tempUserName, tempDateTime, tempBuf,tempBuf2) == 4) {
                // Check if any of the parsed values are empty
                if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0 || strlen(tempBuf2) == 0) {
                    // Send error message if any required parameters are missing or invalid
                    const char* errorMsg = "Error: Missing or invalid parameters";
                    SendResponse(errorMsg,InputVia);
                }
                else{
                    strcpy(CAuserName, tempUserName);
                    strcpy(CAdateTime, tempDateTime);
                    numValue = atoi(tempBuf);
                    polarity = atoi(tempBuf2);
                }
            }
            else {
                // Send error message if parsing failed
                const char* errorMsg = "Error: Invalid format";
                SendResponse(errorMsg,InputVia);
            }
        }
        else if(strcmp(InputVia, "UART") == 0)
        {
            sscanf(rx_buffer, "*CA:%d:%d#",&numValue,&polarity);
            strcpy(CAuserName,"LOCAL");
            strcpy(CAdateTime,"00/00/00"); 
        }
        else if(strcmp(InputVia, "MQTT") == 0)
        {
            sscanf(rx_buffer, "*CA:%d:%d#",&numValue,&polarity);
            strcpy(CAuserName,"MQTT_LOCAL");
            strcpy(CAdateTime,"00/00/00");
        }

      
     
       ESP_LOGI(InputVia, "Generate @ numValue %d polarity %d",numValue,polarity);
       sprintf(payload, "*CA-OK,%s,%s,%d,%d#",CAuserName,CAdateTime,numValue,polarity);
       utils_nvs_set_str(NVS_CA_USERNAME, CAuserName);
       utils_nvs_set_str(NVS_CA_DATETIME, CAdateTime);
       ESP_LOGI(InputVia,"CA Values Saved %s,%s",CAuserName,CAdateTime);
       SendResponse(payload,InputVia);
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