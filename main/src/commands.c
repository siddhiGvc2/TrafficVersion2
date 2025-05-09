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
        sendSocketData(sock, Message, strlen(Message), 0);
        uart_write_string_ln(Message);
        if(MQTT_CONNEECTED && connected_to_wifi && MQTTRequired)
        {
            char message[200];
            strcpy(message,Message);
            if (message[0] == '*' && message[strlen(message)-1] == '#') {
                // Extract the command between * and #
                char command[100];
                char modified_message[256];
                strncpy(command, message + 1, strlen(message) - 2);
                command[strlen(message) - 2] = '\0';
                
                // Create new message with serial number
                sprintf(modified_message, "*TCP-OUT,%s#", command);
                mqtt_publish_msg(modified_message);
            }
            
        }
    }
    else if(strcmp(OutputVia, "UART") == 0)
    {
        uart_write_string_ln(Message);
    }
    else if(strcmp(OutputVia, "MQTT") == 0)
    {
        if(MQTT_CONNEECTED && connected_to_wifi && MQTTRequired)
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
// blink LEDs
// moved next 3 lines from analysePacketTcp.c to commands.c
// on 050525 
// so that led blinks in all cases
    blinkLEDNumber = 1;
    LED4TCPPacket = 1;
    ticks_100 = 0;

if(strcmp(InputVia,"TCP")==0)
    {
        if(MQTTRequired)
        {

            char message[200];
            strcpy(message,rx_buffer);
            if (message[0] == '*' && message[strlen(message)-1] == '#') {
                // Extract the command between * and #
                char command[100];
                char modified_message[256];
                strncpy(command, message + 1, strlen(message) - 2);
                command[strlen(message) - 2] = '\0';
                
                // Create new message with serial number
                sprintf(modified_message, "*TCP-IN,%s#", command);
                mqtt_publish_msg(modified_message);
            }
        }
    }

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
//        else if(serverStatus==0)
        else if(IsSocketConnected==0)
        {
         sprintf(payload, "*NOSERVER#");
         SendResponse(payload,InputVia);
        }
        else if(IsSocketConnected==0){
//        else if(serverStatus==1){
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
     else if(strncmp(rx_buffer, "*HBT-S#",7) == 0)
     {
            // removed on 050525
            //sprintf(payload, "*HBT-OK#");
            //sendSocketData(sock, payload, strlen(payload), 0);
            ServerHBTTimeOut = 0;
            uart_write_string_ln("*SERVER HBT-OK#");
            hbt_received();
     }
     
     else if(strncmp(rx_buffer, "*RESTART#", 9) == 0){
        sendSocketData(sock, "*RESTART-OK#", strlen("*RESTART-OK#"), 0);
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
        if(strcmp(InputVia, "TCP") == 0 || strcmp(InputVia, "MQTT") == 0)
        {
            sscanf(rx_buffer, "*INH:%[^:]:%[^:]:%d#",INHuserName,INHdateTime, &INHOutputValue);
          
        }
        else if(strcmp(InputVia, "UART") == 0)
        {
            sscanf(rx_buffer, "*INH:%d#",&INHOutputValue);
            strcpy(INHuserName,"LOCAL");
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
        if(strcmp(InputVia, "TCP") == 0 || strcmp(InputVia, "MQTT") == 0)
        {
            char tempUserName[64], tempDateTime[64], tempBuf[64] ;
            if (sscanf(rx_buffer, "*PT:%[^:]:%[^:]:%[^:#]#", tempUserName, tempDateTime, tempBuf) == 3) {
            // Check if any of the parsed values are empty
            if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0 ) {
                // send error message if any required parameters are missing or invalid
                const char* errorMsg = "*Error: Missing or invalid parameters#";
                SendResponse(errorMsg,InputVia);
            }
            else{
        
            strcpy(PTuserName, tempUserName);
            strcpy(PTdateTime, tempDateTime);
            strcpy(PassThruValue, tempBuf);
             }
            }
            else {
                // Send error message if parsing failed
                const char* errorMsg = "*Error: Invalid format#";
                SendResponse(errorMsg,InputVia);
            }
        }
        else if(strcmp(InputVia,"UART") == 0)
        {
            sscanf(rx_buffer, "*PT:%[^#]#",PassThruValue);
            strcpy(PTuserName, "LOCAL");
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
        if(strcmp(InputVia, "TCP") == 0 || strcmp(InputVia, "MQTT") == 0)
        {
            sscanf(rx_buffer, "*SP:%[^:]:%[^:]:%d#",SPuserName,SPdateTime, &jumperPort);
        }
        else if(strcmp(InputVia, "UART") == 0)
        {
            sscanf(rx_buffer, "*SP:%d#",&jumperPort);
            strcpy(SPuserName,"LOCAL");
            strcpy(SPdateTime,"00/00/00");
        }
    
        sprintf(payload, "*SP-OK,%s,%s,%d#",SPuserName,SPdateTime,jumperPort);
        utils_nvs_set_str(NVS_SP_USERNAME, SPuserName);
        utils_nvs_set_str(NVS_SP_DATETIME, SPdateTime);
        SendResponse(payload,InputVia);
        utils_nvs_set_int(NVS_SERVER_PORT_KEY_JUMPER, jumperPort);


     }
     else if(strncmp(rx_buffer, "*CA:", 4) == 0){
        if(strcmp(InputVia, "TCP") == 0 || strcmp(InputVia, "MQTT") == 0)
        {
            char tempUserName[64], tempDateTime[64], tempBuf[64],tempBuf2[64];
            if (sscanf(rx_buffer, "*CA:%[^:]:%[^:]:%[^:]:%[^:#]#", tempUserName, tempDateTime, tempBuf,tempBuf2) == 4) {
                // Check if any of the parsed values are empty
                if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0 || strlen(tempBuf2) == 0) {
                    // Send error message if any required parameters are missing or invalid
                    const char* errorMsg = "*Error: Missing or invalid parameters#";
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
                const char* errorMsg = "*Error: Invalid format#";
                SendResponse(errorMsg,InputVia);
            }
        }
        else if(strcmp(InputVia, "UART") == 0)
        {
            sscanf(rx_buffer, "*CA:%d:%d#",&numValue,&polarity);
            strcpy(CAuserName,"LOCAL");
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
    else if(strncmp(rx_buffer, "*SS:", 4) == 0){
        if(strcmp(InputVia, "TCP") == 0 || strcmp(InputVia, "MQTT") == 0)
        {
            char tempUserName[64], tempDateTime[64], tempBuf[64];

            // Parse the input buffer
            if (sscanf(rx_buffer, "*SS:%[^:]:%[^:]:%[^#]#", tempUserName, tempDateTime, tempBuf) == 3) {
                // Check if any of the parsed values are empty
                if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0) {
                    // Send error message if any required parameters are missing or invalid
                    const char* errorMsg = "*Error: Missing or invalid parameters#";
                    SendResponse(errorMsg,InputVia);
                } else {
                    // Copy parsed values to the actual variables
                    strcpy(SSuserName, tempUserName);
                    strcpy(SSdateTime, tempDateTime);
                    strcpy(WIFI_SSID_1, tempBuf);
                }
            }
            else {
                // Send error message if parsing failed
                const char* errorMsg = "*Error: Invalid format#";
                SendResponse(errorMsg,InputVia);
            }
        }
        else if(strcmp(InputVia, "UART") == 0)
        {
            sscanf(rx_buffer, "*SS:%[^#]#",WIFI_SSID_1);
            strcpy(SSuserName,"LOCAL");
            strcpy(SSdateTime,"00/00/00");
        }
    
           // Save the values to non-volatile storage
           utils_nvs_set_str(NVS_SSID_1_KEY, WIFI_SSID_1);
           utils_nvs_set_str(NVS_SS_USERNAME, SSuserName);
           utils_nvs_set_str(NVS_SS_DATETIME, SSdateTime);

           // Format the success message and send it
           sprintf(payload, "*SS-OK,%s,%s#", SSuserName, SSdateTime);
           SendResponse(payload,InputVia);
           tx_event_pending = 1;

    }
    else if(strncmp(rx_buffer, "*SS1:", 5) == 0){
        if(strcmp(InputVia, "TCP") == 0 || strcmp(InputVia, "MQTT") == 0)
        {
            char tempUserName[64], tempDateTime[64], tempBuf[64];

            // Parse the input buffer
            if (sscanf(rx_buffer, "*SS1:%[^:]:%[^:]:%[^#]#", tempUserName, tempDateTime, tempBuf) == 3) {
                // Check if any of the parsed values are empty
                if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0) {
                    // Send error message if any required parameters are missing or invalid
                    const char* errorMsg = "*Error: Missing or invalid parameters#";
                    SendResponse(errorMsg,InputVia);
                } else {
                    // Copy parsed values to the actual variables
                    strcpy(SS1userName, tempUserName);
                    strcpy(SS1dateTime, tempDateTime);
                    strcpy(WIFI_SSID_2, tempBuf);
                }
            }
            else {
                // Send error message if parsing failed
                const char* errorMsg = "*Error: Invalid format#";
                SendResponse(errorMsg,InputVia);
            }
        }
        else if(strcmp(InputVia, "UART") == 0)
        {
            sscanf(rx_buffer, "*SS1:%[^#]#",WIFI_SSID_2);
            strcpy(SS1userName,"LOCAL");
            strcpy(SS1dateTime,"00/00/00");
        }
       

           // Save the values to non-volatile storage
           utils_nvs_set_str(NVS_SSID_2_KEY, WIFI_SSID_2);
           utils_nvs_set_str(NVS_SS1_USERNAME, SS1userName);
           utils_nvs_set_str(NVS_SS1_DATETIME, SS1dateTime);

           // Format the success message and send it
           sprintf(payload, "*SS1-OK,%s,%s#", SS1userName, SS1dateTime);
           SendResponse(payload,InputVia);
           tx_event_pending = 1;

    }
    else if(strncmp(rx_buffer, "*SS2:", 5) == 0){
        if(strcmp(InputVia, "TCP") == 0 || strcmp(InputVia, "MQTT") == 0)
        {
            char tempUserName[64], tempDateTime[64], tempBuf[64];

            // Parse the input buffer
            if (sscanf(rx_buffer, "*SS2:%[^:]:%[^:]:%[^#]#", tempUserName, tempDateTime, tempBuf) == 3) {
                // Check if any of the parsed values are empty
                if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0) {
                    // Send error message if any required parameters are missing or invalid
                    const char* errorMsg = "*Error: Missing or invalid parameters#";
                    SendResponse(errorMsg,InputVia);
                } else {
                    // Copy parsed values to the actual variables
                    strcpy(SS2userName, tempUserName);
                    strcpy(SS2dateTime, tempDateTime);
                    strcpy(WIFI_SSID_3, tempBuf);
                }
            }
            else {
                // Send error message if parsing failed
                const char* errorMsg = "*Error: Invalid format#";
                SendResponse(errorMsg,InputVia);
            }
        }
        else if(strcmp(InputVia, "UART") == 0)
        {
            sscanf(rx_buffer, "*SS2:%[^#]#",WIFI_SSID_3);
            strcpy(SS2userName,"LOCAL");
            strcpy(SS2dateTime,"00/00/00");
        }
       

           // Save the values to non-volatile storage
           utils_nvs_set_str(NVS_SSID_2_KEY, WIFI_SSID_2);
           utils_nvs_set_str(NVS_SS1_USERNAME, SS1userName);
           utils_nvs_set_str(NVS_SS1_DATETIME, SS1dateTime);

           // Format the success message and send it
           sprintf(payload, "*SS2-OK,%s,%s#", SS2userName, SS2dateTime);
           SendResponse(payload,InputVia);
           tx_event_pending = 1;

    }
    else if(strncmp(rx_buffer, "*PW:", 4) == 0){
        if(strcmp(InputVia, "TCP") == 0 || strcmp(InputVia, "MQTT") == 0)
        {
            char tempUserName[64], tempDateTime[64], tempBuf[64];

            // Parse the input buffer
            if (sscanf(rx_buffer, "*PW:%[^:]:%[^:]:%[^#]#", tempUserName, tempDateTime, tempBuf) == 3) {
                // Check if any of the parsed values are empty
                if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0) {
                    // Send error message if any required parameters are missing or invalid
                    const char* errorMsg = "Error: Missing or invalid parameters";
                    SendResponse(errorMsg,InputVia);
                } else {
                    // Copy parsed values to the actual variables
                    strcpy(PWuserName, tempUserName);
                    strcpy(PWdateTime, tempDateTime);
                    strcpy(WIFI_PASS_1, tempBuf);
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
            sscanf(rx_buffer, "*PW:%[^#]#",WIFI_PASS_1);
            strcpy(PWuserName,"LOCAL");
            strcpy(PWdateTime,"00/00/00");
        }
      

           // Save the values to non-volatile storage
           utils_nvs_set_str(NVS_PASS_1_KEY, WIFI_PASS_1);
           utils_nvs_set_str(NVS_PW_USERNAME, PWuserName);
           utils_nvs_set_str(NVS_PW_DATETIME, PWdateTime);

           // Format the success message and send it
           sprintf(payload, "*PW-OK,%s,%s#", PWuserName, PWdateTime);
           SendResponse(payload,InputVia);
           tx_event_pending = 1;

    }
    else if(strncmp(rx_buffer, "*PW1:", 5) == 0){
        if(strcmp(InputVia, "TCP") == 0 || strcmp(InputVia, "MQTT") == 0)
        {
            char tempUserName[64], tempDateTime[64], tempBuf[64];

            // Parse the input buffer
            if (sscanf(rx_buffer, "*PW1:%[^:]:%[^:]:%[^#]#", tempUserName, tempDateTime, tempBuf) == 3) {
                // Check if any of the parsed values are empty
                if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0) {
                    // Send error message if any required parameters are missing or invalid
                    const char* errorMsg = "*Error: Missing or invalid parameters#";
                    SendResponse(errorMsg,InputVia);
                } else {
                    // Copy parsed values to the actual variables
                    strcpy(PW1userName, tempUserName);
                    strcpy(PW1dateTime, tempDateTime);
                    strcpy(WIFI_PASS_2, tempBuf);
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
            sscanf(rx_buffer, "*PW1:%[^#]#",WIFI_PASS_2);
            strcpy(PW1userName,"LOCAL");
            strcpy(PW1dateTime,"00/00/00");
        }
        
           // Save the values to non-volatile storage
           utils_nvs_set_str(NVS_PASS_2_KEY, WIFI_PASS_2);
           utils_nvs_set_str(NVS_PW1_USERNAME, PW1userName);
           utils_nvs_set_str(NVS_PW1_DATETIME, PW1dateTime);

           // Format the success message and send it
           sprintf(payload, "*PW1-OK,%s,%s#", PW1userName, PW1dateTime);
           SendResponse(payload,InputVia);
           tx_event_pending = 1;

    }
    else if(strncmp(rx_buffer, "*PW2:", 5) == 0){
        if(strcmp(InputVia, "TCP") == 0 || strcmp(InputVia, "MQTT") == 0)
        {
            char tempUserName[64], tempDateTime[64], tempBuf[64];

            // Parse the input buffer
            if (sscanf(rx_buffer, "*PW2:%[^:]:%[^:]:%[^#]#", tempUserName, tempDateTime, tempBuf) == 3) {
                // Check if any of the parsed values are empty
                if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0) {
                    // Send error message if any required parameters are missing or invalid
                    const char* errorMsg = "Error: Missing or invalid parameters";
                    SendResponse(errorMsg,InputVia);
                } else {
                    // Copy parsed values to the actual variables
                    strcpy(PW2userName, tempUserName);
                    strcpy(PW2dateTime, tempDateTime);
                    strcpy(WIFI_PASS_3, tempBuf);
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
            sscanf(rx_buffer, "*PW2:%[^#]#",WIFI_PASS_3);
            strcpy(PW2userName,"LOCAL");
            strcpy(PW2dateTime,"00/00/00");
        }
    

           // Save the values to non-volatile storage
           utils_nvs_set_str(NVS_PASS_3_KEY, WIFI_PASS_3);
           utils_nvs_set_str(NVS_PW2_USERNAME, PW2userName);
           utils_nvs_set_str(NVS_PW2_DATETIME, PW2dateTime);

           // Format the success message and send it
           sprintf(payload, "*PW2-OK,%s,%s#", PW2userName, PW2dateTime);
           SendResponse(payload,InputVia);
           tx_event_pending = 1;

    }
    else if(strncmp(rx_buffer, "*URL:", 5) == 0){
        if(strcmp(InputVia, "TCP") == 0 || strcmp(InputVia, "MQTT") == 0)
        {
            char tempUserName[64], tempDateTime[64], tempBuf[64];

            // Parse the input buffer
            if (sscanf(rx_buffer, "*URL:%[^:]:%[^:]:%[^#]#", tempUserName, tempDateTime, tempBuf) == 3) {
                // Check if any of the parsed values are empty
                if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0) {
                    // Send error message if any required parameters are missing or invalid
                    const char* errorMsg = "*Error: Missing or invalid parameters#";
                    SendResponse(errorMsg,InputVia);
                } else {
                        strcpy(URLuserName, tempUserName);
                        strcpy(URLdateTime, tempDateTime);
                        strcpy(FOTA_URL, tempBuf);
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
            sscanf(rx_buffer, "*URL:%[^#]#", FOTA_URL);
            strcpy(URLuserName,"LOCAL");
            strcpy(URLdateTime,"00/00/00");
        }
        
        utils_nvs_set_str(NVS_OTA_URL_KEY, FOTA_URL);
        sprintf(payload, "*URL-OK,%s,%s#",URLuserName,URLdateTime);
        utils_nvs_set_str(NVS_URL_USERNAME, URLuserName);
        utils_nvs_set_str(NVS_URL_DATETIME, URLdateTime);
        SendResponse(payload,InputVia);
        tx_event_pending = 1;

    }
    else if(strncmp(rx_buffer, "*SIP:", 5) == 0){

        if(strcmp(InputVia, "TCP") == 0 || strcmp(InputVia, "MQTT") == 0)
        {
            char tempUserName[64], tempDateTime[64], tempBuf[64] ,tempBuf2[64];

            if (sscanf(rx_buffer, "*SIP:%[^:]:%[^:]:%[^:]#", tempUserName, tempDateTime, tempBuf) == 3) { 
                  if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0  ) {
                    // Send error message if any required parameters are missing or invalid
                    const char* errorMsg = "*Error: Missing or invalid parameters#";
                    SendResponse(errorMsg,InputVia);
                }
                else{
                        strcpy(SIPuserName, tempUserName);
                        strcpy(SIPdateTime, tempDateTime);
                        SipNumber=atoi(tempBuf);
                }
           
            }
            else {
                // Send error message if parsing failed
                const char* errorMsg = "*Error: Invalid format#";
                SendResponse(errorMsg,InputVia);
            }
        }
        else if(strcmp(InputVia, "UART") == 0)
        {
            sscanf(rx_buffer, "*SIP:%d#",&SipNumber);
            strcpy(SIPuserName,"LOCAL");
            strcpy(SIPdateTime,"00/00/00");
        }
      

        if ((SipNumber == 0) || (SipNumber >MAXSIPNUMBER))  
        {  
            sprintf(payload, "*SIP-Error#");
            ESP_LOGI(InputVia,"*SIP-ERROR#");
        }else 
        {
            sprintf(payload, "*SIP-OK,%s,%s#",SIPuserName,SIPdateTime);                                                   
            utils_nvs_set_int(NVS_SIP_NUMBER,  SipNumber);
            utils_nvs_set_str(NVS_SIP_USERNAME, SIPuserName);
            utils_nvs_set_str(NVS_SIP_DATETIME, SIPdateTime);
            ESP_LOGI(InputVia,"*SIP-OK,%s,%s#",SIPuserName,SIPdateTime);
        }    
        SendResponse(payload,InputVia);
        uart_write_string_ln(payload);
        tx_event_pending = 1;

    }
    else if (strncmp(rx_buffer, "*ERASE:", 7) == 0){
        if(strcmp(InputVia, "TCP") == 0 || strcmp(InputVia, "MQTT") == 0)
        {
            char tempUserName[64], tempDateTime[64], tempBuf[64];
                                       // if seria no of device != ErasedSerialNumber then do not erase
                                            // if all values are not avalible then do not erase

                                    if (sscanf(rx_buffer, "*ERASE:%[^:]:%[^:]:%[^:#]", tempUserName, tempDateTime, tempBuf) == 3) { 
                                        if (strlen(tempUserName) == 0 || strlen(tempDateTime) == 0 || strlen(tempBuf) == 0 ) {
                                            // Send error message if any required parameters are missing or invalid
                                            const char* errorMsg = "*Error: Missing or invalid parameters#";
                                            SendResponse(errorMsg,InputVia);
                                            uart_write_string_ln(errorMsg);
                                        }
                                        else if (strcmp(tempBuf, SerialNumber) != 0) {
                                            sprintf (payload,"*Erase:Serial Not Matched Command:%s Actual:%s#",tempBuf,SerialNumber);
                                            SendResponse(payload,InputVia);
                                            uart_write_string_ln(payload);
                                        }

                                        else{
                                        
                                           
                                            strcpy(ERASEuserName, tempUserName);
                                            strcpy(ERASEdateTime, tempDateTime);
                                            strcpy(ErasedSerialNumber,tempBuf);

                                        }
           
            }
            else {
                // Send error message if parsing failed
                const char* errorMsg = "*Error: Invalid format#";
                SendResponse(errorMsg,InputVia);
            }
        }
        else if(strcmp(InputVia, "UART") == 0)
        {
            sscanf(rx_buffer, "*ERASE:%[^:#]",ErasedSerialNumber);
            if (strcmp(ErasedSerialNumber, SerialNumber) != 0) {
               // send (payload,"*Erase:Serial Not Matched Command:%s Actual:%s#",tempBuf,SerialNumber);
               const char* errorMsg = "*Erase:Serial Not Matched#";
               // sendSocketData(sock, errorMsg, strlen(errorMsg), 0);
               SendResponse(errorMsg,InputVia);
           }
           else{
          
           strcpy(ERASEuserName,"LOCAL");
           strcpy(ERASEdateTime,"00/00/00");
         
           }
        }
       

        utils_nvs_set_str(NVS_ERASE_USERNAME, ERASEuserName);
        utils_nvs_set_str(NVS_ERASE_DATETIME, ERASEdateTime);
        utils_nvs_set_str(NVS_ERASED_SERIAL_NUMBER, ErasedSerialNumber);
        utils_nvs_erase_all();
        utils_nvs_set_str(NVS_SERIAL_NUMBER, ErasedSerialNumber);
        SendResponse("*ERASE-OK#",InputVia);
    }
    else if(strncmp(rx_buffer, "*SL:", 4) == 0){
        if(strcmp(InputVia, "TCP") == 0 || strcmp(InputVia, "MQTT") == 0)
        {
            if (edges == 0)
            {
                sscanf(rx_buffer, "*SL:%[^:]:%[^:]:%d:%d#",SLuserName,SLdateTime,&ledpin,&ledstatus);
              
                
            }  
        }
        else if(strcmp(InputVia, "UART") == 0)
        {
            sscanf(rx_buffer, "*SL:%d:%d#", &ledpin,&ledstatus);
            strcpy(SLuserName,"LOCAL");
            strcpy(SLdateTime,"00/00/00");
        }
     
         
                ESP_LOGI(InputVia, "Set LED @ Pin %d Status %d",ledpin,ledstatus);
                
                SendResponse( "*SL-OK#",InputVia);
                tx_event_pending = 1;
                if (ledpin == 1)
                    gpio_set_level(L1, ledstatus);
                if (ledpin == 2)
                    gpio_set_level(L2, ledstatus);
                if (ledpin == 3)
                    gpio_set_level(L3, ledstatus);

    }
    else if(strncmp(rx_buffer, "*CC", 3) == 0){
        if(strcmp(InputVia, "TCP") == 0 || strcmp(InputVia, "MQTT") == 0)
        {
            sscanf(rx_buffer, "*CC:%[^:]:%[^:]:%[^#]#",CCuserName,CCdateTime,UniqueTimeStamp); // changed on 20-12-24 as per EC10
          
            sprintf(payload, "*CC-OK,%s,%s,%s#",CCuserName,CCdateTime,UniqueTimeStamp);  // changed on 20-12-24 as per EC10
            SendResponse(payload,InputVia);
        }
        else if(strcmp(InputVia, "UART") == 0){
            strcpy(CCuserName,"LOCAL");
            strcpy(CCdateTime,"00/00/00");
            sprintf(payload, "*CC-OK,%s,%s#",CCuserName,CCdateTime);
            SendResponse(payload,InputVia);
        }
     
            utils_nvs_set_str(NVS_CC_USERNAME, CCuserName);
            utils_nvs_set_str(NVS_CC_DATETIME, CCdateTime);  // added on 20-12-24 as per EC10
            utils_nvs_set_str(NVS_UNIX_TS, UniqueTimeStamp);
            // sendSocketData(sock, payload, strlen(payload), 0);
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
    else if(strncmp(rx_buffer, "*RST", 4) == 0){
        if(strcmp(InputVia, "TCP") == 0 || strcmp(InputVia, "MQTT") == 0)
        {
            sscanf(rx_buffer, "*RST:%[^:]:%[^#]#",RSTuserName,RSTdateTime);
        }
        else if(strcmp(InputVia,"UART")==0)
        {
            strcpy(RSTuserName,"LOCAL");
            strcpy(RSTdateTime,"00/00/00");
        }
       
       
        ESP_LOGI(InputVia, "**************Restarting after 3 second*******");
        utils_nvs_set_str(NVS_RST_USERNAME, RSTuserName);
        utils_nvs_set_str(NVS_RST_DATETIME, RSTdateTime);
        sprintf(payload, "*RST-OK,%s,%s#",RSTuserName,RSTdateTime);
        SendResponse(payload,InputVia);
        ESP_LOGI(InputVia, "*RST-OK#");
        uart_write_string_ln("*Resetting device#");
        RestartDevice();

    }
    else if(strncmp(rx_buffer, "*SN:", 4) == 0){
        if(strcmp(InputVia, "TCP") == 0 || strcmp(InputVia, "MQTT") == 0)
        {
            if (strstr(SerialNumber,"999999"))
            {
                sscanf(rx_buffer, "*SN:%[^:]:%[^:]:%[^#]#",SNuserName,SNdateTime,SerialNumber);
                utils_nvs_set_str(NVS_SERIAL_NUMBER, SerialNumber);
                utils_nvs_set_str(NVS_SN_USERNAME, SNuserName);
                utils_nvs_set_str(NVS_SN_DATETIME, SNdateTime);
                sendSocketData(sock, "*SN-OK#", strlen("*SN-OK#"), 0);
            }
            else
            {
               sendSocketData(sock, "*SN CAN NOT BE SET#", strlen("*SN CAN NOT BE SET#"), 0);

            }
                tx_event_pending = 1; 
        }
        else if(strcmp(InputVia,"UART")==0)
        {
            sscanf(rx_buffer, "*SN:%[^#]#",SerialNumber);
            strcpy(SNuserName, "LOCAL");
            strcpy(SNdateTime, "00/00/00");
            
            utils_nvs_set_str(NVS_SERIAL_NUMBER, SerialNumber);
            utils_nvs_set_str(NVS_SN_USERNAME,SNuserName);
            utils_nvs_set_str(NVS_SN_DATETIME,SNdateTime);
            
        
            SendResponse("*SN-OK#",InputVia);
            tx_event_pending = 1;
        }
      
  
    }
    else if(strncmp(rx_buffer, "*FOTA", 5) == 0){
        fotaStatus=1;
        SendResponse("*FOTA-OK#",InputVia); 
        SendResponse(FOTA_URL,InputVia); 
        tx_event_pending = 1;
        http_fota();
        SendResponse("*FOTA-OK#",InputVia); 
      
    }
    else if(strncmp(rx_buffer, "*DATA:", 6) == 0){
        sscanf(rx_buffer, "*DATA:%s#",currentDateTime);
        SendResponse("*DATA-OK#",InputVia); 
      
        
    }
    else if(strncmp(rx_buffer,"*LedState?#",11)==0)
    {
        sprintf(payload, "*LedState,%d,%d#",prev_state,led_state);
        SendResponse(payload,InputVia); 
    }
    else if(strncmp(rx_buffer,"*CommState?#",12)==0)
    {
        sprintf(payload, "*CommState,%d,%d#",IsSocketConnected,MQTT_CONNEECTED);
        SendResponse(payload,InputVia); 
    }
    // added on 090525
    else if(strncmp(rx_buffer,"*MQTT:",6)==0)
    {
        sscanf(rx_buffer, "*MQTT:%[^:]:%[^#]#",mqtt_user,mqtt_pass);
        utils_nvs_set_str(NVS_MQTT_USER,mqtt_user);
        utils_nvs_set_str(NVS_MQTT_PASS,mqtt_pass);
        sprintf(payload, "*MQTT-OK,%s,%s#",mqtt_user,mqtt_pass);
        SendResponse(payload,InputVia); 
    }
    else if(strncmp(rx_buffer,"*MQTT?#",7)==0)
    {
        sprintf(payload, "*MQTT-OK,%s,%s#",mqtt_user,mqtt_pass);
        SendResponse(payload,InputVia); 
    }
    else{
        int l = strlen(rx_buffer);
        char buf[l+1];
        if(strcmp(InputVia, "TCP") == 0)
        {
        if(extractSubstring(rx_buffer, buf) == true){
            uart_write_string("*");
            uart_write_string(buf);
            uart_write_string("#");
            tx_event_pending = 1;
        }
       }
       else if(strcmp(InputVia, "UART") == 0){
        if(extractSubstring(rx_buffer, buf) == true){
            int l2 = strlen(buf);
            char b[l2+3];
            sprintf(b, "*%s#", buf);
            tcp_ip_client_send_str(b);
            tx_event_pending = 1;
        }
       }
       else{
        if(extractSubstring(rx_buffer, buf) == true){
            uart_write_string("*");
            uart_write_string(buf);
            uart_write_string("#");
            tx_event_pending = 1;
        }
       }

    }


}