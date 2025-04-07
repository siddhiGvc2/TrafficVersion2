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

   
   
     if(strncmp(pkt, "*RST#", 5) == 0){
    
        sprintf(buffer, "*RST-OK#");
        uart_write_string_ln(buffer);
        uart_write_string_ln("*Resetting device#");
        RestartDevice();
    
      
    }
   
   
   else if(strncmp(pkt, "*FOTA#", 6) == 0){
        uart_write_string_ln("*FOTA-OK#");
        tx_event_pending = 1;
        http_fota();
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
