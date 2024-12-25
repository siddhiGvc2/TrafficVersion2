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



void led_set_level(gpio_num_t , int);
void status_leds_init();
void leds_update_task();
void set_led_state(Led_State_t);

static const char *TAG = "LED";


void led_set_level(gpio_num_t gpio_num, int state){
    #ifdef LED_ACTIVE_HIGH
        gpio_set_level(gpio_num, state);
    #else
        gpio_set_level(gpio_num, !state);
    #endif
}


/*
    STANDBY_LED,
    SEARCH_FOR_WIFI, NOT USED
    WAIT4ESPTOUCH, red blink once Green ON
    SEARCH_FOR_ESPTOUCH,red blink twice Green ON
    SEARCH_FOR_WIFI1, green blink once 
    SEARCH_FOR_WIFI2, green blink twice 
    SEARCH_FOR_WIFI3, green blink thrice 
    WIFI_FOUND_NO_INTERNET,
    WIFI_AND_INTERNET_NO_SERVER, GREEN ON, RED blinking continously
    EVERYTHING_OK_LED, RED and GREEN ON
    OTA_IN_PROGRESS RED and GREEN ALTERNATE ON
*/

void leds_update_task(){
    int LedInUse = 0;
    int LedMode = 0;
    for(;;){
        // program comes here after every 100 msec
        // ticks_100 move from 0 to 19 in 2 second perioud
        ticks_100 = ticks_100+1;    
        if (ticks_100 >= 20)
        {
            ticks_100 = 0;
            LedMode = led_state;
            led1_gpio_state = 0;
            led2_gpio_state = 0;
         }

        if(led_state == STANDBY_LED){
            current_interval = 0;
            if(led1_gpio_state == 1){
                led1_gpio_state = 0;
                led_set_level(LEDR, led1_gpio_state);
            }
        }
        else if (led_state == WAITING_FOR_RESTART){
            numberOfPulses = 20;
            LedInUse= 3;  
        }
        else if(led_state == SEARCH_FOR_WIFI1){
            numberOfPulses = 2;
            LedInUse= 1;            
        }else if(led_state == SEARCH_FOR_WIFI2){
            numberOfPulses = 4;
            LedInUse= 1;            
        }else if(led_state == SEARCH_FOR_WIFI3){
            numberOfPulses = 6;
            LedInUse= 1;            
        }else if(led_state == WIFI_FOUND_NO_INTERNET){
            numberOfPulses = 2;
            LedInUse= 2;            
        }else if(led_state == WIFI_AND_INTERNET_NO_SERVER){
            numberOfPulses = 4;
            LedInUse= 2;            
        }else if(led_state == EVERYTHING_OK_LED){
            numberOfPulses = 0;
            LedInUse= 3;            

        }else if(led_state == SEARCH_FOR_ESPTOUCH){
            numberOfPulses = 4;
            LedInUse= 3;            
        }else if(led_state == WAIT4ESPTOUCH){
            numberOfPulses = 2;
            LedInUse= 3;            

        }else if(led_state == OTA_IN_PROGRESS){
            numberOfPulses = 20;
            LedInUse= 3;            
        }
        else if(led_state == INCOMING_PULSE_DETECTED){
            numberOfPulses = 6;
            LedInUse= 3;            
        }

    
        //     // if(rx_event_pending){
        //     //     rx_event_pending = 0; 
        //     //     led_set_level(LEDG, 0);
        //     //     vTaskDelay(50/portTICK_PERIOD_MS);
        //     //     led_set_level(LEDG, 1);
        //     //     vTaskDelay(50/portTICK_PERIOD_MS);
        //     // }

        //     // if(tx_event_pending){
        //     //     tx_event_pending = 0; 
        //     //     led_set_level(LEDG, 0);
        //     //     vTaskDelay(50/portTICK_PERIOD_MS);
        //     //     led_set_level(LEDG, 1);
        //     //     vTaskDelay(50/portTICK_PERIOD_MS);
        //     //     led_set_level(LEDG, 0);
        //     //     vTaskDelay(50/portTICK_PERIOD_MS);
        //     //     led_set_level(LEDG, 1);
        //     //     vTaskDelay(50/portTICK_PERIOD_MS);
        //     // }

        // }
        // when number of pulses = 0, led are on
        // this is every thing okay 
        // if any data received on TCPIP
        // switch off leds for 500 msec, then swithc on for 500 msecs and then switch off for 500 msec

        if (numberOfPulses == 0)
        {
            if (LED4TCPPacket)
            {
                if (ticks_100 < 4)
                {
                    led_set_level(LEDR, 0);
                    led_set_level(LEDG, 0);
                }                
                else if (ticks_100 <6)
                {
                    led_set_level(LEDR, 1);
                    led_set_level(LEDG, 1);
                }
                else if (ticks_100 <10)
                {
                    led_set_level(LEDR, 0);
                    led_set_level(LEDG, 0);
                }
                else if (ticks_100 <12)
                {
                    led_set_level(LEDR, 1);
                    led_set_level(LEDG, 1);
                }
                else if (ticks_100 <16)
                {
                    led_set_level(LEDR, 0);
                    led_set_level(LEDG, 0);
                }
                else
                {
                    led_set_level(LEDR, 1);
                    led_set_level(LEDG, 1);
                    LED4TCPPacket = 0;
                }
            }
            else
            {
                led_set_level(LEDR, 1);
                led_set_level(LEDG, 1);
            }
        }
        else if (numberOfPulses == 20)
        {
                led2_gpio_state ^= 1;
                led1_gpio_state = !led2_gpio_state;
                led_set_level(LEDG, led2_gpio_state);
                led_set_level(LEDR, led1_gpio_state);
        }

        else if (numberOfPulses>ticks_100)
        {
            if (LedInUse == 1)
            {   
                led1_gpio_state ^= 0x01;
                led_set_level(LEDR, led1_gpio_state);
                led_set_level(LEDG, 1);
            }
            if (LedInUse == 2)
            {   
                led2_gpio_state ^= 0x01;
                led_set_level(LEDG, led2_gpio_state);
                led_set_level(LEDR, 0);
            }
            if (LedInUse == 3)
            {   
                led2_gpio_state ^= 0x01;
                led1_gpio_state = led2_gpio_state;
                led_set_level(LEDG, led2_gpio_state);
                led_set_level(LEDR, led1_gpio_state);
            }
        }
        else
        { 
            if ((led_state == SEARCH_FOR_WIFI) || (led_state == SEARCH_FOR_WIFI1) || (led_state == SEARCH_FOR_WIFI2)  || (led_state == SEARCH_FOR_WIFI3))
                led_set_level(LEDG, 1);
            else         
                led_set_level(LEDG, 0);
            led_set_level(LEDR, 0);
        }
        vTaskDelay(200/portTICK_PERIOD_MS);
    }
}

// waiting for restart does not allow any other change in led state
void set_led_state(Led_State_t st){
    if (led_state == WAITING_FOR_RESTART) 
        return;    
    if (led_state == SEARCH_FOR_ESPTOUCH)
    {   
        last_update_led1 = 0;
        led_state = st;
        return;
    }
    if (led_state == WAIT4ESPTOUCH)
    {
        last_update_led1 = 0;
        led_state = st;
        return;
    }
    last_update_led1 = 0;
    led_state = st;
}

void status_leds_init(){
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = 1ULL << LEDR | 1ULL << LEDG ;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
    led_set_level(LEDR, 0);
    led_set_level(LEDG, 0);
    xTaskCreate(leds_update_task, "leds_update_task", 2048, NULL, 6, NULL);
}
