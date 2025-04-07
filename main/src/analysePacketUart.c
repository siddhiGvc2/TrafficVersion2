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
    char buf[200];
    char buffer[900]; 
    char payload[240]; 
    // sprintf(buffer,"*MAC:%s:%s#", MAC_ADDRESS_ESP,SerialNumber);
    // uart_write_string_ln(buffer);
    // sprintf(payload, "*HBT,%s,%s#", MAC_ADDRESS_ESP,SerialNumber);
    // uart_write_string_ln(payload);

   
    
    if(strncmp(pkt, "*CC#", 4) == 0){
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
      else if(strncmp(pkt, "*SN:", 4) == 0){
      
       sscanf(pkt, "*SN:%[^#]#",buf);
        strcpy(SerialNumber, buf);
        strcpy(SNuserName, "LOCAL");
        strcpy(SNdateTime, "00/00/00");
        utils_nvs_set_str(NVS_SERIAL_NUMBER, SerialNumber);
        utils_nvs_set_str(NVS_SN_USERNAME,SNuserName);
        utils_nvs_set_str(NVS_SN_DATETIME,SNdateTime);
        
      
        uart_write_string_ln("*SN-OK#");
        tx_event_pending = 1;
        
    }
     else if(strncmp(pkt, "*PT:", 4) == 0){
      
       sscanf(pkt, "*PT:%[^#]#",PassThruValue);
      
        strcpy(PTuserName, "LOCAL");
        strcpy(PTdateTime, "00/00/00");
        utils_nvs_set_str(NVS_PASS_THRU, PassThruValue);
        utils_nvs_set_str(NVS_PT_USERNAME,PTuserName);
        utils_nvs_set_str(NVS_PT_DATETIME,PTdateTime);
        
      
        uart_write_string_ln("*PT-OK#");
        tx_event_pending = 1;
        
    }
   
    
    else if(strncmp(pkt, "*CA:", 4) == 0){
        sscanf(pkt, "*CA:%d:%d#",&numValue,&polarity);
        strcpy(CAuserName,"LOCAL");
        strcpy(CAdateTime,"00/00/00");
        
        sprintf(buffer,"*CA-OK,%d,%d#",numValue,polarity);
        utils_nvs_set_str(NVS_CA_USERNAME, CAuserName);
        utils_nvs_set_str(NVS_CA_DATETIME, CAdateTime);
       uart_write_string_ln(buffer);
        tx_event_pending = 1;
        
    }
   
     else if(strncmp(pkt, "*RST#", 5) == 0){
    
        sprintf(buffer, "*RST-OK#");
        uart_write_string_ln(buffer);
        uart_write_string_ln("*Resetting device#");
        RestartDevice();
    
      
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
    }else if(strncmp(pkt, "*SS2:", 5) == 0){
        sscanf(pkt, "*SS2:%[^#]#", buf);
        strcpy(WIFI_SSID_3, buf);
        utils_nvs_set_str(NVS_SSID_3_KEY, WIFI_SSID_3);
        uart_write_string_ln("*SS2-OK#");
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
    }

   
 
   
   
    else if(strncmp(pkt, "*SIP:", 5) == 0){
        sscanf(pkt, "*SIP:%d#",&SipNumber);
        strcpy(SIPuserName,"LOCAL");
        strcpy(SIPdateTime,"00/00/00");

          if ( (SipNumber == 0) || (SipNumber > MAXSIPNUMBER) )  
            {  
                //sprintf(payload, "*SIP-Error#");
                ESP_LOGI(TAG,"*SIP-ERROR#");
                uart_write_string_ln("*SIP-ERROR#");

            }else 
            {
                sprintf(payload, "*SIP-OK,%s,%s#",SIPuserName,SIPdateTime);                                                   
                uart_write_string_ln(payload);
                utils_nvs_set_int(NVS_SIP_NUMBER, SipNumber);
                utils_nvs_set_str(NVS_SIP_USERNAME, SIPuserName);
                utils_nvs_set_str(NVS_SIP_DATETIME, SIPdateTime);
                ESP_LOGI(TAG,"*SIP-OK,%s,%s#",SIPuserName,SIPdateTime);
            } 

        // sprintf(buf, "%s", server_ip_addr);
        
        // utils_nvs_set_str(NVS_SERVER_IP_KEY, buf);
        // utils_nvs_set_int(NVS_SERVER_PORT_KEY, sp_port);

        // utils_nvs_set_str(NVS_SIP_USERNAME, SIPuserName);
        // utils_nvs_set_str(NVS_SIP_DATETIME, SIPdateTime);
        // uart_write_string_ln("*SIP-OK#");
        tx_event_pending = 1;
    }
    else if (strncmp(pkt, "*ERASE:", 7) == 0){
         sscanf(pkt, "*ERASE:%[^:#]",ErasedSerialNumber);
         if (strcmp(ErasedSerialNumber, SerialNumber) != 0) {
            // send (payload,"*Erase:Serial Not Matched Command:%s Actual:%s#",tempBuf,SerialNumber);
            const char* errorMsg = "Erase:Serial Not Matched";
            // send(sock, errorMsg, strlen(errorMsg), 0);
            uart_write_string_ln(errorMsg);
        }
        else{
       
        strcpy(ERASEuserName,"LOCAL");
        strcpy(ERASEdateTime,"00/00/00");
        utils_nvs_set_str(NVS_ERASE_USERNAME, ERASEuserName);
        utils_nvs_set_str(NVS_ERASE_DATETIME, ERASEdateTime);
        utils_nvs_set_str(NVS_ERASED_SERIAL_NUMBER, ErasedSerialNumber);
        utils_nvs_erase_all();
        utils_nvs_set_str(NVS_SERIAL_NUMBER, ErasedSerialNumber);
        uart_write_string_ln("*ERASE:OK#");
        }

    }
 
    
    
    else{
     

        uart_write_string_ln(pkt);
        // int l = strlen(pkt);
        // char buff[l+1];
        // /*   *---#  */

        // if(extractSubstring(pkt, buff) == true){
        //     int l2 = strlen(buff);
        //     char b[l2+3];
        //     sprintf(b, "*%s#", buff);
        //     tcp_ip_client_send_str(b);
        //     tx_event_pending = 1;
        // }
        strcpy(InputVia,"UART");
        AnalyzeInputPkt(pkt,InputVia);
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
