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



static const char *TAG = "UART";


void uart_write_number(uint8_t );
void uart_write_string(const char * );
void uart_write_string_ln(const char * );


void process_uart_packet(const char *);

void process_uart_packet(const char *pkt){
    rx_event_pending = 1;
    char buf[100];
      if(strncmp(pkt, "*CA?#", 5) == 0){
         char buffer[300]; 
       sprintf(buffer,"*CA-OK,%s,%s,%d,%d#",CAuserName,CAdateTime,pulseWitdh,SignalPolarity);

       uart_write_string_ln(buffer);
        tx_event_pending = 1;
    }else if(strncmp(pkt, "*CC#", 4) == 0){
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
         char buffer[150]; 
        sprintf(buffer, "*CC-OK#");
      
        uart_write_string_ln(buffer);
        tx_event_pending = 1;
    }
     else if(strncmp(pkt, "*SL:", 4) == 0){
        if(edges==0)
        {
        sscanf(pkt, "*SL:%d:%d#", &ledpin,&ledstatus);
        if (ledpin == 1)
            gpio_set_level(L1, ledstatus);
        if (ledpin == 2)
            gpio_set_level(L2, ledstatus);
        if (ledpin == 3)
            gpio_set_level(L3, ledstatus);
      
        uart_write_string_ln("*SL-OK#");
        tx_event_pending = 1;
        }
    }
    else if(strncmp(pkt, "*CA:", 4) == 0){
        sscanf(pkt, "*CA:%d:%d#",&numValue,&polarity);
        strcpy(CAuserName,"LOCAL");
        strcpy(CAdateTime,"00/00/00");
         char buffer[100]; 
        sprintf(buffer,"*CA-OK,%d,%d#",numValue,polarity);
        utils_nvs_set_str(NVS_CA_USERNAME, CAuserName);
        utils_nvs_set_str(NVS_CA_DATETIME, CAdateTime);
       uart_write_string_ln(buffer);
        tx_event_pending = 1;
        
    }
     else if(strncmp(pkt, "*FOTA:", 6) == 0){
    
         char buffer[100]; 
        sprintf(buffer,"*FOTA-OK#");
     

       uart_write_string_ln(buffer);
       uart_write_string_ln(FOTA_URL);
       http_fota();
        tx_event_pending = 1;
    }
     else if(strncmp(pkt, "*RST:", 5) == 0){
    
         char buffer[100]; 
        sprintf(buffer, "*RST-OK#");
        uart_write_string_ln(buffer);
        vTaskDelay(3000/portTICK_PERIOD_MS);
        esp_restart();
    
      
    }
    else if(strncmp(pkt, "*V:", 3) == 0){
        if(edges==0)
        {
           sscanf(pkt, "*V:%d:%d:%d#",&TID,&pin,&pulses);
            // if (INHInputValue == INHIBITLevel)
            // {
            //     ESP_LOGI(TAG, "*UNIT DISABLED#");
            //     char buffer[100]; 
            //     sprintf(buffer, "*VEND DISABLED#");
            //     uart_write_string_ln(buffer);
                
            // }
            //  else if (TID != LastTID)
            if (TID != LastTID)
            {
                edges = pulses*2;  // doubled edges
                // strcpy(WIFI_PASS_2, buf);
                // utils_nvs_set_str(NVS_PASS_2_KEY, WIFI_PASS_2);
                ESP_LOGI(TAG, "*V-OK,%d,%d,%d#",TID,pin,pulses);
                 char buffer[100]; 
                sprintf(buffer, "*V-OK,%d,%d,%d#", TID,pin,pulses); //actual when in production
                uart_write_string_ln(buffer);
                vTaskDelay(1000/portTICK_PERIOD_MS);
                sprintf(buffer, "*T-OK,%d,%d,%d#",TID,pin,pulses); //actual when in production
                ESP_LOGI(TAG, "*T-OK,%d,%d,%d#",TID,pin,pulses);
                uart_write_string_ln(buffer);
                tx_event_pending = 1;
                Totals[pin-1] += pulses;
                LastTID = TID;
            }
            else
            {
                ESP_LOGI(TAG, "Duplicate TID");
           
                 char buffer[100]; 
                sprintf(buffer,  "*DUP TID#");
                uart_write_string_ln(buffer);
            }  
      
      
        }
    
      
    }
      else if(strncmp(pkt, "*FW?#", 5) == 0){
         ESP_LOGI(TAG, "*%s#",FWVersion);
       
         uart_write_string_ln(FWVersion);
      
        tx_event_pending = 1;
        if (ledpin == 1)
            gpio_set_level(L1, ledstatus);
        if (ledpin == 2)
            gpio_set_level(L2, ledstatus);
        if (ledpin == 3)
            gpio_set_level(L3, ledstatus);
    }
    else if(strncmp(pkt, "*SS:", 4) == 0){
        sscanf(pkt, "*SS:%[^#]#",buf);
        //uart_write_string_ln(buf);
        strcpy(WIFI_SSID_1, buf);
        utils_nvs_set_str(NVS_SSID_1_KEY, WIFI_SSID_1);
        uart_write_string_ln("*SS-OK#");
        tx_event_pending = 1;
    }else if(strncmp(pkt, "*SS1:", 5) == 0){
        sscanf(pkt, "*SS1:%[^#]#", buf);
        strcpy(WIFI_SSID_2, buf);
        utils_nvs_set_str(NVS_SSID_2_KEY, WIFI_SSID_2);
        uart_write_string_ln("*SS1-OK#");
        tx_event_pending = 1;
    }else if(strncmp(pkt, "*PW:", 4) == 0){
        sscanf(pkt, "*PW:%[^#]#", buf);
        strcpy(WIFI_PASS_1, buf);
        utils_nvs_set_str(NVS_PASS_1_KEY, WIFI_PASS_1);
        uart_write_string_ln("*PW-OK#");
        tx_event_pending = 1;
    }else if(strncmp(pkt, "*PW1:", 5) == 0){
        sscanf(pkt, "*PW1:%[^#]#", buf);
        strcpy(WIFI_PASS_2, buf);
        utils_nvs_set_str(NVS_PASS_2_KEY, WIFI_PASS_2);
        uart_write_string_ln("*PW1-OK#");
        tx_event_pending = 1;
    }else if(strncmp(pkt, "*PW2:", 5) == 0){
        sscanf(pkt, "*PW2:%[^#]#", buf);
        strcpy(WIFI_PASS_3, buf);
        utils_nvs_set_str(NVS_PASS_3_KEY, WIFI_PASS_3);
        uart_write_string_ln("*PW2-OK#");
        tx_event_pending = 1;
    }else if(strncmp(pkt, "*URL:", 5) == 0){
        sscanf(pkt, "*URL:%[^#]#", buf);
        strcpy(FOTA_URL, buf);
        strcpy(URLuserName,"LOCAL");
        strcpy(URLdateTime,"00/00/00");
        utils_nvs_set_str(NVS_OTA_URL_KEY, FOTA_URL);
        utils_nvs_set_str(NVS_URL_USERNAME, URLuserName);
        utils_nvs_set_str(NVS_URL_DATETIME, URLdateTime);

        uart_write_string_ln("*URL-OK#");
        tx_event_pending = 1;
    }else if(strncmp(pkt, "*FOTA#", 6) == 0){
        uart_write_string_ln("*FOTA-OK#");
        tx_event_pending = 1;
        http_fota();
    }else if(strncmp(pkt, "*URL?#", 6) == 0){
         char buffer[650]; 
       sprintf(buffer,"*%s,%s,%s#",URLuserName,URLdateTime,FOTA_URL);
        uart_write_string_ln(buffer);
        tx_event_pending = 1;
    
    }
//    WIFI_SSID_1,WIFI_SSID_2,WIFI_SSID_3
    else if(strncmp(pkt, "*SSID?#", 7) == 0){
        uart_write_string("SSID Current/1/2/3 are - ");
        uart_write_number(WiFiNumber);
        uart_write_string(" , ");
        uart_write_string(WIFI_SSID_1);
        uart_write_string(" , ");
        uart_write_string(WIFI_SSID_2);
        uart_write_string(" , ");
        uart_write_string_ln(WIFI_SSID_3);

        tx_event_pending = 1;
    
    }
    
    else if(strncmp(pkt, "*TC?#", 5) == 0){
        char buffer[100]; 
        sprintf(buffer, "*TC,%d,%d,%d,%d,%d,%d,%d#", 
             CashTotals[0],CashTotals[1],CashTotals[2],CashTotals[3],CashTotals[4],CashTotals[5],CashTotals[6]);

   
        uart_write_string_ln(buffer);
        tx_event_pending = 1;
    }
    else if(strncmp(pkt, "*TV?#", 5) == 0){
        char buffer[100]; 
        sprintf(buffer, "*TV,%d,%d,%d,%d,%d,%d,%d#", 
            Totals[0], Totals[1], Totals[2], Totals[3], Totals[4], Totals[5], Totals[6]);

   
        uart_write_string_ln(buffer);
        tx_event_pending = 1;
    
    }
    else if(strncmp(pkt, "*SIP:", 5) == 0){
      
        sscanf(pkt, "*SIP:%[^:]:%d#",server_ip_addr,
                                        &sp_port);
        strcpy(SIPuserName,"LOCAL");
        strcpy(SIPdateTime,"00/00/00");
        char buf[100];
        sprintf(buf, "%s", server_ip_addr);
        
        utils_nvs_set_str(NVS_SERVER_IP_KEY, buf);
        utils_nvs_set_int(NVS_SERVER_PORT_KEY, sp_port);

        utils_nvs_set_str(NVS_SIP_USERNAME, SIPuserName);
        utils_nvs_set_str(NVS_SIP_DATETIME, SIPdateTime);
        uart_write_string_ln("*SIP-OK#");
        tx_event_pending = 1;
    } else if(strncmp(pkt, "*SIP?#", 6) == 0){
        char buffer[350]; 
       sprintf(buffer,"*SIP,%s,%s,%s,%d#",SIPuserName,SIPdateTime,server_ip_addr,
                                        sp_port );

   
        uart_write_string_ln(buffer);
        tx_event_pending = 1;
    
    }
    else if (strncmp(pkt, "*ERASE#", 7) == 0){
        utils_nvs_erase_all();
        uart_write_string("*ERASE:OK#");
    }else if(strncmp(pkt, "*RESTART#", 9) == 0){
        uart_write_string("*RESTART:OK#");
        tx_event_pending = 1;
        vTaskDelay(2000/portTICK_PERIOD_MS);
        esp_restart();
    }else{
        uart_write_string_ln(pkt);
        int l = strlen(pkt);
        char buff[l+1];
        /*   *---#  */

        if(extractSubstring(pkt, buff) == true){
            int l2 = strlen(buff);
            char b[l2+3];
            sprintf(b, "*%s#", buff);
            tcp_ip_client_send_str(b);
            tx_event_pending = 1;
        }
    }
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
