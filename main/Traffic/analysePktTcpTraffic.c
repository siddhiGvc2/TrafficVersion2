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


static const char *TAG = "TrafficTCP";

void AnalaysePacketTrafficTCP(const char* rx_buffer){
   ESP_LOGI(TAG,"TRAFFIC TCP INITIALLIZED");
   if(uartDebugInfo)
     uart_write_string_ln("TRAFFIC TCP INITIALLIZED");
  
   char payload[756]; 
   char expected_start2[10];
   sprintf(expected_start2, "*%s,", CMD_PREFIX2);

   char expected_start1[10];
   sprintf(expected_start1, "*%s,", CMD_PREFIX1);

        

        int queryR;
        char queryPrefix[20];
        if (strncmp(rx_buffer, expected_start1,strlen(expected_start1)) == 0)
        {
            
         int R;
         char firstInput[100];
         char secondInput[100];
         if (sscanf(rx_buffer, "*%*[^,],%d,%[^,],%99[^#]#", &R, firstInput, secondInput) == 3){
         ESP_LOGI(TAG,"%d",R);
         if(R==1)
         {
            sprintf(DEF1, "%d,%s,%s", R, firstInput, secondInput);
            sprintf(payload, "*%s1-OK#",CMD_PREFIX1);
            utils_nvs_set_str(NVS_DEF1, DEF1);
            send(sock, payload, strlen(payload), 0);
            ESP_LOGI(TAG,"%s",payload);
            if(uartDebugInfo)
               uart_write_string_ln(payload);
            
         }
         else if(R==2)
         {
            sprintf(DEF2, "%d,%s,%s", R, firstInput, secondInput);
            sprintf(payload,"*%s2-OK#",CMD_PREFIX1);
            utils_nvs_set_str(NVS_DEF2, DEF2);
            send(sock, payload, strlen(payload), 0);
             ESP_LOGI(TAG,"%s",payload);
            if(uartDebugInfo)
              uart_write_string_ln(payload);
         }
         else if(R==3)
         {
            sprintf(DEF3, "%d,%s,%s", R, firstInput, secondInput);
            sprintf(payload, "*%s3-OK#",CMD_PREFIX1);
            utils_nvs_set_str(NVS_DEF3, DEF3);
            send(sock, payload, strlen(payload), 0);
             ESP_LOGI(TAG,"%s",payload);
            if(uartDebugInfo)
               uart_write_string_ln(payload);
         }
         else if(R==4)
         {
            sprintf(DEF4, "%d,%s,%s", R, firstInput, secondInput);
            sprintf(payload,"*%s4-OK#",CMD_PREFIX1);
            utils_nvs_set_str(NVS_DEF4, DEF4);
            send(sock, payload, strlen(payload), 0);
             ESP_LOGI(TAG,"%s",payload);
            if(uartDebugInfo)
               uart_write_string_ln(payload);
         }
         CalculateAllTime();
         }
         else  if (sscanf(rx_buffer, "*%[^,],%d?#", queryPrefix, &queryR) == 2) {
            if (strcmp(queryPrefix, CMD_PREFIX1) == 0 && queryR >= 1 && queryR <= 4) {
        
            
                if(queryR==1)
                {
                sprintf(payload, "*%s%d-VAL,%s#", CMD_PREFIX1,queryR,DEF1); // e.g., *DEF1-VAL,1,G20,A5#
                }
                else if(queryR==2)
                {
                sprintf(payload, "*%s%d-VAL,%s#", CMD_PREFIX1,queryR,DEF2); // e.g., *DEF1-VAL,1,G20,A5#
                }
                else if(queryR==3)
                {
                sprintf(payload, "*%s%d-VAL,%s#", CMD_PREFIX1,queryR,DEF3); // e.g., *DEF1-VAL,1,G20,A5#
                }
                else if(queryR==4)
                {
                sprintf(payload, "*%s%d-VAL,%s#", CMD_PREFIX1,queryR,DEF4); // e.g., *DEF1-VAL,1,G20,A5#
                }

        
                send(sock, payload, strlen(payload), 0);
                 ESP_LOGI(TAG,"%s",payload);
                if(uartDebugInfo)
                  uart_write_string_ln(payload);
                return; // ✅ Done with query, skip the rest
            }
          }


        }
        else if (strncmp(rx_buffer, expected_start2,strlen(expected_start2)) == 0)
        {
         int R;
         char firstInput[100];
         char secondInput[100];
          if(sscanf(rx_buffer, "*%*[^,],%d,%[^,],%99[^#]#",&R,firstInput,secondInput)==3)
          {
         if(R==1)
         {
            sprintf(ATC1, "%d,%s,%s", R, firstInput, secondInput);
            sprintf(payload, "*%s1-OK#",CMD_PREFIX2);
            utils_nvs_set_str(NVS_ATC1, ATC1);
            send(sock, payload, strlen(payload), 0);
             ESP_LOGI(TAG,"%s",payload);
            if(uartDebugInfo)
              uart_write_string_ln(payload);
         }
         else if(R==2)
         {
            sprintf(ATC2, "%d,%s,%s", R, firstInput, secondInput);
            sprintf(payload, "*%s2-OK#",CMD_PREFIX2);
            utils_nvs_set_str(NVS_ATC2, ATC2);
            send(sock, payload, strlen(payload), 0);
             ESP_LOGI(TAG,"%s",payload);
            if(uartDebugInfo)
              uart_write_string_ln(payload);
         }
         else if(R==3)
         {
            sprintf(ATC3, "%d,%s,%s", R, firstInput, secondInput);
            sprintf(payload,"*%s3-OK#",CMD_PREFIX2);
            utils_nvs_set_str(NVS_ATC3, ATC3);
            send(sock, payload, strlen(payload), 0);
             ESP_LOGI(TAG,"%s",payload);
            if(uartDebugInfo)
              uart_write_string_ln(payload);
         }
         else if(R==4)
         {
            sprintf(ATC4, "%d,%s,%s", R, firstInput, secondInput);
            sprintf(payload, "*%s4-OK#",CMD_PREFIX2);
            utils_nvs_set_str(NVS_ATC4, ATC4);
            send(sock, payload, strlen(payload), 0);
             ESP_LOGI(TAG,"%s",payload);
            if(uartDebugInfo)
              uart_write_string_ln(payload);
         }
        }
        else   if (sscanf(rx_buffer, "*%[^,],%d?#", queryPrefix, &queryR) == 2) {
            if (strcmp(queryPrefix, CMD_PREFIX2) == 0 && queryR >= 1 && queryR <= 4) {
                
                if(queryR==1)
                {
                sprintf(payload, "*%s%d-VAL,%s#", CMD_PREFIX2,queryR,ATC1); // e.g., *DEF1-VAL,1,G20,A5#
                }
                else if(queryR==2)
                {
                sprintf(payload, "*%s%d-VAL,%s#", CMD_PREFIX2,queryR,ATC2); // e.g., *DEF1-VAL,1,G20,A5#
                }
                else if(queryR==3)
                {
                sprintf(payload, "*%s%d-VAL,%s#", CMD_PREFIX2,queryR,ATC3); // e.g., *DEF1-VAL,1,G20,A5#
                }
                else if(queryR==4)
                {
                sprintf(payload, "*%s%d-VAL,%s#", CMD_PREFIX2,queryR,ATC4); // e.g., *DEF1-VAL,1,G20,A5#
                }
        
                send(sock, payload, strlen(payload), 0);
                 ESP_LOGI(TAG,"%s",payload);
                if(uartDebugInfo)
                  uart_write_string_ln(payload);
                return; // ✅ Done with query, skip the rest
            }
        }

        }
      
      
      


        else if (strncmp(rx_buffer, "*SERVER#", 8) == 0) {
           strcpy(Mode,"SERVER");
           sprintf(payload, "*MODE:%s#",Mode);
           send(sock, payload, strlen(payload), 0);
            ESP_LOGI(TAG,"%s",payload);
            
               strcpy(stageMode, "SERVER");
           
              
            
           if(uartDebugInfo)
              uart_write_string_ln(payload);

        }
        else if (strncmp(rx_buffer, "*FIXED#", 7) == 0) {
           strcpy(Mode,"FIXED");
           sprintf(payload, "*MODE:%s#",Mode);
           send(sock, payload, strlen(payload), 0);
            ESP_LOGI(TAG,"%s",payload);
             strcpy(stageMode, "FIXED");
           if(uartDebugInfo)
              uart_write_string_ln(payload);

        }
        else if (strncmp(rx_buffer, "*CURRENT?#", 10) == 0) {
            sprintf(payload, "*MODE:%s#",Mode);
            send(sock, payload, strlen(payload), 0);
             ESP_LOGI(TAG,"%s",payload);
            if(uartDebugInfo)
               uart_write_string_ln(payload);
 
         }
        else if (strncmp(rx_buffer, "*SERVER?#", 9) == 0) {
            if(uartDebugInfo)
            {
               uart_write_string_ln(ATC1);
               uart_write_string_ln(ATC2);
               uart_write_string_ln(ATC3);
               uart_write_string_ln(ATC4);
            }

        }
        else if (strncmp(rx_buffer, "*FIXED?#", 8) == 0) {
            if(uartDebugInfo)
            {
               uart_write_string_ln(DEF1);
               uart_write_string_ln(DEF2);
               uart_write_string_ln(DEF3);
               uart_write_string_ln(DEF4);
            }
        }
        // else if (strncmp(rx_buffer, "*CURRENT?#", 10) == 0) {
        //     // uart_write_string_ln(DEF1);
        //     // uart_write_string_ln(DEF2);
        //     // uart_write_string_ln(DEF3);
        //     // uart_write_string_ln(DEF4);
        // }
       
        else if(strncmp(rx_buffer,"*TIME,",6)==0)
        {
            sscanf(rx_buffer, "*TIME,%d,%d,%d#",&Hours,&Mins,&Secs);
            sprintf(payload, "*TIME-OK#");
            send(sock, payload, strlen(payload), 0);
             ESP_LOGI(TAG,"%s",payload);
            
        }

  

}