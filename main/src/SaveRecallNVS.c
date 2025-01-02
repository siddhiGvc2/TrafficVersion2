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


static const char *TAG = "NVS";

void utils_nvs_init(void);
esp_err_t utils_nvs_get_str(const char *  , char * , size_t );
void utils_nvs_erase_all(void);
esp_err_t utils_nvs_get_int(const char * , int16_t *);
void utils_nvs_erase_key(const char *);
void utils_nvs_set_int(const char *  , int16_t);
void load_settings_nvs(void);
void utils_nvs_set_str(const char * , const char * );




void utils_nvs_init(){
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &utils_nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "NVS Open Done\n");
    }
}


esp_err_t utils_nvs_get_str(const char * key , char * val, size_t max_length){
    esp_err_t ret = ESP_FAIL;
    if(utils_nvs_handle){
        ret = nvs_get_str(utils_nvs_handle, key, val, &max_length);
    }
    return ret;
}

esp_err_t utils_nvs_get_int(const char * key , int16_t * val){
    esp_err_t ret = ESP_FAIL;
    if(utils_nvs_handle){
        ret = nvs_get_i16(utils_nvs_handle, key, val);
    }
    return ret;
}

void utils_nvs_erase_all(){
    if(utils_nvs_handle){
        nvs_erase_all(utils_nvs_handle);
    }
}

void utils_nvs_erase_key(const char * key){
    if(utils_nvs_handle){
        nvs_erase_key(utils_nvs_handle, key);
    }
}

void utils_nvs_set_int(const char * key , int16_t val){
    if(utils_nvs_handle){
        nvs_set_i16(utils_nvs_handle, key, val);
    }
}

void utils_nvs_set_str(const char * key , const char * val){
    if(utils_nvs_handle){
        nvs_set_str(utils_nvs_handle, key, val);
        nvs_commit(utils_nvs_handle);
    }
}


void load_settings_nvs(){
    
    char payload[150];
    ESP_LOGI(TAG, "*NVS Reading Started#");

      if(utils_nvs_get_str(NVS_UNIX_TS,UniqueTimeStamp,100) == ESP_OK){
     utils_nvs_get_str(NVS_UNIX_TS,UniqueTimeStamp,100);
    }
    else{
        strcpy(UniqueTimeStamp,DEFAULT_TIMESTAMP);
       utils_nvs_set_str(NVS_UNIX_TS,UniqueTimeStamp); 
    }

      if(utils_nvs_get_str(NVS_QR_STRING,QrString,100) == ESP_OK){
     utils_nvs_get_str(NVS_QR_STRING,QrString,100);
    }
   
 
   
    if(utils_nvs_get_str(NVS_LAST_TID, LastTID,100) == ESP_OK){
       utils_nvs_get_str(NVS_LAST_TID, LastTID,100);
    }



    ESP_LOGI(TAG,"*Reading SIP Number#");
    if(utils_nvs_get_int(NVS_SIP_NUMBER,&SipNumber) == ESP_OK){
       utils_nvs_get_int(NVS_SIP_NUMBER,&SipNumber);
    }
    else {
        SipNumber = 1;
    }
    ESP_LOGI(TAG,"*SIP Number %d#",SipNumber);
    if ((SipNumber == 0) || (SipNumber > MAXSIPNUMBER))
        SipNumber = 1;

    if (SipNumber == 1)
    {
        strcpy(server_ip_addr, TCP_URL1);
        server_port = TCP_PORT1;
    }
    if (SipNumber == 2)
    {
        strcpy(server_ip_addr, TCP_URL2);
        server_port = TCP_PORT2;
    }
    if (SipNumber == 3)
    {
        strcpy(server_ip_addr, TCP_URL3);
        server_port = TCP_PORT3;
    }
    ESP_LOGI(TAG, "*Server IP Address : %s#", server_ip_addr);
    sprintf(payload,"*Server IP Address : %s#", server_ip_addr);
    uart_write_string_ln(payload);
   

    if(utils_nvs_get_str(NVS_ERASE_DATETIME,ERASEdateTime,100) == ESP_OK){
     utils_nvs_get_str(NVS_ERASE_DATETIME,ERASEdateTime,100);
    }
     if(utils_nvs_get_str(NVS_ERASE_USERNAME,ERASEuserName,100) == ESP_OK){
      utils_nvs_get_str(NVS_ERASE_USERNAME,ERASEuserName,100);
    }
     if(utils_nvs_get_str(NVS_ERASED_SERIAL_NUMBER,ErasedSerialNumber,100) == ESP_OK){
      utils_nvs_get_str(NVS_ERASED_SERIAL_NUMBER,ErasedSerialNumber,100);
    }
    
    if(utils_nvs_get_str(NVS_SERIAL_NUMBER, SerialNumber,100) == ESP_OK){
       utils_nvs_get_str(NVS_SERIAL_NUMBER, SerialNumber,100);
       if (strstr(SerialNumber,"NA-1507-"))
       {
            ;
       }
       else
       {
            sprintf(payload,"NA-1507-%s",SerialNumber);
            strcpy (SerialNumber,payload);
            uart_write_string("SerialNumber is - ");
            uart_write_string_ln(SerialNumber);
       }
    }
    else
    {
        sprintf(SerialNumber,"999999");
    }
    if(utils_nvs_get_str(NVS_SIP_USERNAME,SIPuserName,100) == ESP_OK){
      utils_nvs_get_str(NVS_SIP_USERNAME,SIPuserName,100);
    }
     if(utils_nvs_get_str(NVS_SIP_DATETIME,SIPdateTime,100) == ESP_OK){
     utils_nvs_get_str(NVS_SIP_DATETIME,SIPdateTime,100);
    }
    if(utils_nvs_get_str(NVS_CA_USERNAME,CAuserName,100) == ESP_OK){
      utils_nvs_get_str(NVS_CA_USERNAME,CAuserName,100);
    }
     if(utils_nvs_get_str(NVS_CA_DATETIME,CAdateTime,100) == ESP_OK){
     utils_nvs_get_str(NVS_CA_DATETIME,CAdateTime,100);
    }

    if(utils_nvs_get_str(NVS_CC_USERNAME,CCuserName,100) == ESP_OK){
      utils_nvs_get_str(NVS_CC_USERNAME,CCuserName,100);
    }
     if(utils_nvs_get_str(NVS_CC_DATETIME,CCdateTime,100) == ESP_OK){
     utils_nvs_get_str(NVS_CC_DATETIME,CCdateTime,100);
    }
    
    if(utils_nvs_get_str(NVS_URL_USERNAME,URLuserName,100) == ESP_OK){
      utils_nvs_get_str(NVS_URL_USERNAME,URLuserName,100);
    }
     if(utils_nvs_get_str(NVS_URL_DATETIME,URLdateTime,100) == ESP_OK){
     utils_nvs_get_str(NVS_URL_DATETIME,URLdateTime,100);
    }

    if(utils_nvs_get_str(NVS_FOTA_USERNAME,FOTAuserName,100) == ESP_OK){
      utils_nvs_get_str(NVS_FOTA_USERNAME,FOTAuserName,100);
    }
     if(utils_nvs_get_str(NVS_FOTA_DATETIME,FOTAdateTime,100) == ESP_OK){
     utils_nvs_get_str(NVS_FOTA_DATETIME,FOTAdateTime,100);
    }

    if(utils_nvs_get_str(NVS_RST_USERNAME,RSTuserName,100) == ESP_OK){
      utils_nvs_get_str(NVS_RST_USERNAME,RSTuserName,100);
    }
     if(utils_nvs_get_str(NVS_RST_DATETIME,RSTdateTime,100) == ESP_OK){
     utils_nvs_get_str(NVS_RST_DATETIME,RSTdateTime,100);
    }

    if(utils_nvs_get_str(NVS_SN_USERNAME,SNuserName,100) == ESP_OK){
      utils_nvs_get_str(NVS_SN_USERNAME,SNuserName,100);
    }
     if(utils_nvs_get_str(NVS_SN_DATETIME,SNdateTime,100) == ESP_OK){
     utils_nvs_get_str(NVS_SN_DATETIME,SNdateTime,100);
    }

    if(utils_nvs_get_str(NVS_SS_USERNAME,SSuserName,100) == ESP_OK){
      utils_nvs_get_str(NVS_SS_USERNAME,SSuserName,100);
    }
     if(utils_nvs_get_str(NVS_SS_DATETIME,SSdateTime,100) == ESP_OK){
     utils_nvs_get_str(NVS_SS_DATETIME,SSdateTime,100);
    }

    if(utils_nvs_get_str(NVS_SL_USERNAME,SLuserName,100) == ESP_OK){
      utils_nvs_get_str(NVS_SL_USERNAME,SLuserName,100);
    }
     if(utils_nvs_get_str(NVS_SL_DATETIME,SLdateTime,100) == ESP_OK){
     utils_nvs_get_str(NVS_SL_DATETIME,SLdateTime,100);
    }

    if(utils_nvs_get_str(NVS_PW_USERNAME,PWuserName,100) == ESP_OK){
      utils_nvs_get_str(NVS_PW_USERNAME,PWuserName,100);
    }
     if(utils_nvs_get_str(NVS_PW_DATETIME,PWdateTime,100) == ESP_OK){
     utils_nvs_get_str(NVS_PW_DATETIME,PWdateTime,100);
    }

    if(utils_nvs_get_str(NVS_SS1_USERNAME,SS1userName,100) == ESP_OK){
      utils_nvs_get_str(NVS_SS1_USERNAME,SS1userName,100);
    }
     if(utils_nvs_get_str(NVS_SS1_DATETIME,SS1dateTime,100) == ESP_OK){
     utils_nvs_get_str(NVS_SS1_DATETIME,SS1dateTime,100);
    }

    if(utils_nvs_get_str(NVS_PW1_USERNAME,PW1userName,100) == ESP_OK){
      utils_nvs_get_str(NVS_PW1_USERNAME,PW1userName,100);
    }
     if(utils_nvs_get_str(NVS_PW1_DATETIME,PW1dateTime,100) == ESP_OK){
     utils_nvs_get_str(NVS_PW1_DATETIME,PW1dateTime,100);
    
    }
    if(utils_nvs_get_str(NVS_SS2_USERNAME,SS2userName,100) == ESP_OK){
      utils_nvs_get_str(NVS_SS2_USERNAME,SS2userName,100);
    }
     if(utils_nvs_get_str(NVS_SS2_DATETIME,SS2dateTime,100) == ESP_OK){
     utils_nvs_get_str(NVS_SS2_DATETIME,SS2dateTime,100);
    }

    if(utils_nvs_get_str(NVS_PW2_USERNAME,PW2userName,100) == ESP_OK){
      utils_nvs_get_str(NVS_PW2_USERNAME,PW2userName,100);
    }
     if(utils_nvs_get_str(NVS_PW2_DATETIME,PW2dateTime,100) == ESP_OK){
     utils_nvs_get_str(NVS_PW2_DATETIME,PW2dateTime,100);
    }
    if(utils_nvs_get_str(NVS_INH_USERNAME,INHuserName,100) == ESP_OK){
      utils_nvs_get_str(NVS_INH_USERNAME,INHuserName,100);
    }
     if(utils_nvs_get_str(NVS_INH_DATETIME,INHdateTime,100) == ESP_OK){
     utils_nvs_get_str(NVS_INH_DATETIME,INHdateTime,100);
    }

    if(utils_nvs_get_str(NVS_SP_USERNAME,SPuserName,100) == ESP_OK){
      utils_nvs_get_str(NVS_SP_USERNAME,SPuserName,100);
    }
     if(utils_nvs_get_str(NVS_SP_DATETIME,SPdateTime,100) == ESP_OK){
     utils_nvs_get_str(NVS_SP_DATETIME,SPdateTime,100);
    }

     if(utils_nvs_get_str(NVS_PT_USERNAME,PTuserName,100) == ESP_OK){
      utils_nvs_get_str(NVS_PT_USERNAME,PTuserName,100);
    }

     if(utils_nvs_get_str(NVS_PT_DATETIME,PTdateTime,100) == ESP_OK){
     utils_nvs_get_str(NVS_PT_DATETIME,PTdateTime,100);
    }

   



    if(utils_nvs_get_str(NVS_SSID_1_KEY, WIFI_SSID_1, 64) == ESP_OK){
        utils_nvs_get_str(NVS_PASS_1_KEY, WIFI_PASS_1, 64);
        ESP_LOGI(TAG, "*WIFI 1 Credentials %s|%s#", WIFI_SSID_1, WIFI_PASS_1);
    }else{
        ESP_LOGI(TAG, "*Could not get Wifi 1 Credentials From NVS#");
        strcpy(WIFI_SSID_1, DEFAULT_SSID1);
        strcpy(WIFI_PASS_1, DEFAULT_PASS1);
        ESP_LOGI(TAG, "*Default WIFI 1 Credentials %s|%s#", WIFI_SSID_1, WIFI_PASS_1);
        utils_nvs_set_str(NVS_SSID_1_KEY, WIFI_SSID_1);
        utils_nvs_set_str(NVS_PASS_1_KEY, WIFI_PASS_1);
    }

    if(utils_nvs_get_str(NVS_SSID_2_KEY, WIFI_SSID_2, 64) == ESP_OK){
        utils_nvs_get_str(NVS_PASS_2_KEY, WIFI_PASS_2, 64);
        ESP_LOGI(TAG, "*WIFI 2 Credentials %s|%s#", WIFI_SSID_2, WIFI_PASS_2);
    }else{
        ESP_LOGI(TAG, "*Could not get Wifi 2 Credentials From NVS#");
        strcpy(WIFI_SSID_2, DEFAULT_SSID2);
        strcpy(WIFI_PASS_2, DEFAULT_PASS2);
        ESP_LOGI(TAG, "*Default WIFI 2 Credentials %s|%s#", WIFI_SSID_2, WIFI_PASS_2);
        utils_nvs_set_str(NVS_SSID_2_KEY, WIFI_SSID_2);
        utils_nvs_set_str(NVS_PASS_2_KEY, WIFI_PASS_2);
    }

    if(utils_nvs_get_str(NVS_SSID_3_KEY, WIFI_SSID_3, 64) == ESP_OK){
        utils_nvs_get_str(NVS_PASS_3_KEY, WIFI_PASS_3, 64);
        ESP_LOGI(TAG, "*WIFI 3 Credentials %s|%s#", WIFI_SSID_3, WIFI_PASS_3);
    }else{
        ESP_LOGI(TAG, "*Could not get Wifi 3 Credentials From NVS#");
        strcpy(WIFI_SSID_3, DEFAULT_SSID3);
        strcpy(WIFI_PASS_3, DEFAULT_PASS3);
        ESP_LOGI(TAG, "*Default WIFI 3 Credentials %s|%s#", WIFI_SSID_3, WIFI_PASS_3);
        utils_nvs_set_str(NVS_SSID_3_KEY, WIFI_SSID_3);
        utils_nvs_set_str(NVS_PASS_3_KEY, WIFI_PASS_3);
    }




    // if(utils_nvs_get_int(NVS_SERVER_PORT_KEY, &server_port) == ESP_OK){
    //     ESP_LOGI(TAG, "*Server Port From NVS %d#", server_port);
    // }else{
    //     server_port = DEFAULT_SERVER_PORT;
    //     ESP_LOGI(TAG, "*Default Server Port %d#", server_port);
    //     utils_nvs_set_int(NVS_SERVER_PORT_KEY, server_port);
    // }


    if(utils_nvs_get_int(NVS_INH_KEY, &INHOutputValue) == ESP_OK){
        if (INHOutputValue != 0)
            INHOutputValue = 1;
        ESP_LOGI(TAG, "*INH Value is %d#", INHOutputValue);
    }else{
        // strcpy(server_ip_addr, DEFAULT_SERVER_IP_ADDR);
        ESP_LOGI(TAG, "*Default INH Output Value is 0#" );
        utils_nvs_set_int(NVS_INH_KEY, 0);
        INHOutputValue = 0;
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
   


    // if(utils_nvs_get_str(NVS_SERVER_IP_KEY, server_ip_addr, 100) == ESP_OK){
    //     ESP_LOGI(TAG, "*Server IP From NVS %s#", server_ip_addr);
    // }else{
    //     strcpy(server_ip_addr, DEFAULT_SERVER_IP_ADDR);
    //     ESP_LOGI(TAG, "*Default Server IP : %s#", server_ip_addr);
    //     utils_nvs_set_str(NVS_SERVER_IP_KEY, server_ip_addr);
    // }
   
    if(utils_nvs_get_str(NVS_OTA_URL_KEY, FOTA_URL, 256) == ESP_OK){
        ESP_LOGI(TAG, "*FOTA URL From NVS %s#", FOTA_URL);
    }else{
        strcpy(FOTA_URL, DEFAULT_FOTA_URL);
        ESP_LOGI(TAG, "*Default FOTA URL : %s#", FOTA_URL);
        utils_nvs_set_str(NVS_OTA_URL_KEY, FOTA_URL);
    }

     if(utils_nvs_get_str(NVS_PASS_THRU, PassThruValue, 256) == ESP_OK){
        ESP_LOGI(TAG, "*Pass Thru %s#", PassThruValue);
    }else{
        strcpy(PassThruValue, PASS_THRU);
        ESP_LOGI(TAG, "*Pass Thru : %s#", PassThruValue);
        utils_nvs_set_str(NVS_PASS_THRU, PassThruValue);
    }


// if jumper set server address and server port as jumperPort
//    if (gpio_get_level(JUMPER) == 0)
#ifdef GVCOptionRequired
      if (gpio_get_level(JUMPER) == 0)    // disable jumper sensing
    {        
        strcpy(server_ip_addr, DEFAULT_SERVER_IP_ADDR_TRY);
        strcpy(FOTA_URL, DEFAULT_FOTA_URL_TRY);
//        DEFAULT_FOTA_URL_TRY
        ESP_LOGI(TAG, "***************************");
        ESP_LOGI(TAG, "*JUMPER SENSED AT POWER ON#");
        ESP_LOGI(TAG, "*Server IP ADDRESS is %s#",server_ip_addr);
        if(utils_nvs_get_int(NVS_SERVER_PORT_KEY_JUMPER, &jumperPort) == ESP_OK){
               ESP_LOGI(TAG, "*JUMPER Port is %d#",jumperPort);
               server_port = jumperPort; 
        }
        
    }
#endif
    if(utils_nvs_get_int(NVS_CASH1_KEY, &cashValue) == ESP_OK){
        CashTotals[0] = cashValue;
    }
    if(utils_nvs_get_int(NVS_CASH2_KEY, &cashValue) == ESP_OK){
        CashTotals[1] = cashValue;
    }
    if(utils_nvs_get_int(NVS_CASH3_KEY, &cashValue) == ESP_OK){
        CashTotals[2] = cashValue;
    }
    if(utils_nvs_get_int(NVS_CASH4_KEY, &cashValue) == ESP_OK){
        CashTotals[3] = cashValue;
    }
    if(utils_nvs_get_int(NVS_CASH5_KEY, &cashValue) == ESP_OK){
        CashTotals[4] = cashValue;
    }
    if(utils_nvs_get_int(NVS_CASH6_KEY, &cashValue) == ESP_OK){
        CashTotals[5] = cashValue;
    }
    if(utils_nvs_get_int(NVS_CASH7_KEY, &cashValue) == ESP_OK){
        CashTotals[6] = cashValue;
    }

    if(utils_nvs_get_int(NVS_CA_KEY, &caValue) == ESP_OK){
        SignalPolarity = caValue%2;
        pulseWitdh = caValue/2;
        ESP_LOGI(TAG, "CA Values %d - %d", pulseWitdh,SignalPolarity);
    }
    else
    {
        SignalPolarity = 0;
        pulseWitdh = 100;
        ESP_LOGI(TAG, "Default CA Values %d - %d", pulseWitdh,SignalPolarity);
    }



}
