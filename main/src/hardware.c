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
static const char *TAG = "HW";
static QueueHandle_t uart0_queue;

void Out4094Byte (unsigned char);
void gpio_read_n_act(void);
void ICH_init();
void Out4094 (unsigned char);
void BlinkLED (void);
void GeneratePulsesInBackGround (void);
void TestCoin (void);
void Test4094 (void);
void s2p_init();
void console_uart_init(void);
void read_mac_address();
void led_set_level(gpio_num_t , int);
void status_leds_init();
void leds_update_task();
void set_led_state(Led_State_t);
bool extractSubstring(const char* , char* );
uint32_t millis(void);
void resolve_hostname(const char *);

void RestartDevice (void)
{
     ESP_LOGI(TAG, "**************Restarting after 3 second******#");
     send(sock, "*RST-OK#", strlen("*RST-OK#"), 0);
     ESP_LOGI(TAG, "*RST-OK#");
     uart_write_string_ln("*Resetting device#");
     vTaskDelay(3000/portTICK_PERIOD_MS);
     esp_restart();
}

void leds_update_task(){
    for(;;){

        //Handle Led 1 States

        ticks_100 = ticks_100+1;
        if (ticks_100 >= 20)
        {
            ticks_100 = 0;
         }    
        if (led_state == SEARCH_FOR_ESPTOUCH)
        {
            current_interval = 0;
            if(led2_gpio_state == 1){
                led2_gpio_state = 0;
                led_set_level(LEDG, led2_gpio_state);
            }
            
        }
        if(led_state == STANDBY_LED){
            current_interval = 0;
            if(led1_gpio_state == 1){
                led1_gpio_state = 0;
                led_set_level(LEDR, led1_gpio_state);
            }
        }else if(led_state == SEARCH_FOR_WIFI){
            numberOfPulses = 20;
        }else if(led_state == WIFI_FOUND_NO_INTERNET){
            numberOfPulses = 6;
        }else if(led_state == WIFI_AND_INTERNET_NO_SERVER){
            numberOfPulses = 4;
        }else if(led_state == EVERYTHING_OK_LED){
            numberOfPulses = 2;
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
           else if (led_state == OTA_IN_PROGRESS){
            led2_gpio_state = !led2_gpio_state;
            led_set_level(LEDG, led2_gpio_state);
        }

        if (numberOfPulses>ticks_100)
        {
            // if (led_state == SEARCH_FOR_ESPTOUCH)
            // {
            //     led2_gpio_state = !led2_gpio_state;
            //     led_set_level(LEDG, led2_gpio_state);
            //     led_set_level(LEDR, 0);

            // }
            // else
            // {
            if (led_state == EVERYTHING_OK_LED)
                led1_gpio_state = 1;
            else
            {
                led1_gpio_state = !led1_gpio_state;
            }
            led_set_level(LEDR, led1_gpio_state);
            led2_gpio_state = 1;
            led_set_level(LEDG, led2_gpio_state);
            // }
        }
        else
        {
            if (led_state == EVERYTHING_OK_LED)
                led1_gpio_state = 1;
            else
                led1_gpio_state = 0;

            led_set_level(LEDR, led1_gpio_state);
            led2_gpio_state = 1;
            led_set_level(LEDG, led2_gpio_state);
        }
        // // if(((last_update_led1 == 0) || (millis() - last_update_led1 > current_interval)) && (current_interval != 0)){
        // //     //This will be only called when there is some error
        // //     last_update_led1 = millis();
        // //     led1_gpio_state = !led1_gpio_state;
        // //     led_set_level(LEDR, led1_gpio_state);
        // // }
        vTaskDelay(200/portTICK_PERIOD_MS);
    }
}

void set_led_state(Led_State_t st){
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



static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    for(;;) {
        //Waiting for UART event.
        if(xQueueReceive(uart0_queue, (void * )&event, (TickType_t)portMAX_DELAY)) {
            switch(event.type) {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:{
                    char arr[event.size + 1];
                    uart_read_bytes(EX_UART_NUM, arr, event.size, portMAX_DELAY);
                    arr[event.size] = '\0';
                    process_uart_packet(arr);
                    break;
                }
                    
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                    ESP_LOGI(TAG, "uart rx break");
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error");
                    break;
                //UART_PATTERN_DET
                case UART_PATTERN_DET:
                    ESP_LOGI(TAG, "[UART PATTERN DETECTED] ");
                    break;
                //Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
    vTaskDelete(NULL);
}


void console_uart_init(void){
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart0_queue, 0);
    uart_param_config(EX_UART_NUM, &uart_config);
    uart_set_pin(MKM_IC_UART, MKM_IC_UART_TX, MKM_IC_UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
//    uart_set_pin(EX_UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    xTaskCreate(uart_event_task, "uart_event_task", 4096, NULL, 6, NULL);
}



void Out4094Byte (unsigned char value)
{
    uint8_t i,j;
    uint8_t OutputMap[9] = {99,6,0,4,5,2,3,1,99};
    uint8_t ReverseBitMap[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};


    j =0;
    for (i = 1 ; i< 8 ; i++)
    {
        if (value & (0x01<<i))
            j = 0x01 << (OutputMap[i]);
    }

    for (i = 0 ; i < 8 ; i++)
    {
        if (j && (ReverseBitMap[i]))  
            gpio_set_level(DAT, 1);
        else    
            gpio_set_level(DAT, 0);
        ets_delay_us(10);
        gpio_set_level(CLK, 1);
        ets_delay_us(10);
        gpio_set_level(CLK, 0);
    }
    ets_delay_us(10);
    gpio_set_level(STRB, 1);
    ets_delay_us(10);
    gpio_set_level(STRB, 0);
}

void gpio_read_n_act(void)
{
    int testCounter = 0;
    int BlinkMode = 0;
    char payload[100];
    for (;;)
    {
        if (gpio_get_level(ErasePin) == 0)
        {
            if (LastErasePinStatus == 1)    
            {
                ErasePinDebounce = 2000;        
                LastErasePinStatus = 0;
                ESP_LOGI(TAG,"*Eraseing Sense Started#");
            }
            else
            {
                if (ErasePinDebounce)
                { 
                    ErasePinDebounce = ErasePinDebounce-1;
                    if (ErasePinDebounce == 0)
                    {
                        ErasePinStatus = 0;
                        ESP_LOGI(TAG,"*Eraseing All Parameters#");
                        send(sock, "*ERASE-LOCAL#", strlen("*ERASE-LOCAL#"), 0);
                        utils_nvs_erase_all();
                        ESP_LOGI(TAG, "**************Restarting after 3 second******#");
                        send(sock, "*RST-OK#", strlen("*RST-OK#"), 0);
                        ESP_LOGI(TAG, "*RST-OK#");
                        uart_write_string_ln("*Resetting device#");
                        vTaskDelay(3000/portTICK_PERIOD_MS);
                        esp_restart();
                    }
                }

            }
        }
        else
        {
            if (LastErasePinStatus == 0)
            {
                ErasePinDebounce = 200; 
                LastErasePinStatus = 0;
            }
            else
            {
                if (ErasePinDebounce)
                { 
                    ErasePinDebounce = ErasePinDebounce-1;
                    if (ErasePinDebounce == 0)
                        ErasePinStatus = 1;
                }

            }
        }
        if (gpio_get_level(CINHI) == 0)
        {
            INHInputValue = 0;        
        }
        else
        {
            INHInputValue = 1;        
            
        }
        if (PreviousINHValue != INHInputValue)
        {
            PreviousINHValue = INHInputValue;
            // if (gpio_get_level(JUMPER) == 0)
            // {
                sprintf(payload, "*INH,%d#",INHInputValue); 
                send(sock, payload, strlen(payload), 0);
            // }
        }
        InputPin = 0;
        if (gpio_get_level(ICH1) == 0)
        {
            InputPin = 1;
        }
        else if (gpio_get_level(ICH2) == 0)
        {
            InputPin = 2;
        }
        else if (gpio_get_level(ICH3) == 0)
        {
            InputPin = 3;
        }
        else if (gpio_get_level(ICH4) == 0)
        {
            InputPin = 4;
        }
        else if (gpio_get_level(ICH5) == 0)
        {
            InputPin = 5;
        }
        else if (gpio_get_level(ICH6) == 0)
        {
            InputPin = 6;
        }
        else if (gpio_get_level(ICH7) == 0)
        {
            InputPin = 7;
        }
        else 
        {
            InputPin = 0;
        }    
        if (InputPin == 0)
        {
            if (PulseStoppedDelay>0)
            {
                PulseStoppedDelay--;
                if (PulseStoppedDelay == 0)
                {
                    
                    CashTotals[LastInputPin-1] += TotalPulses;
                    if (LastInputPin == 1)
                        utils_nvs_set_int(NVS_CASH1_KEY, CashTotals[0]);
                    if (LastInputPin == 2)
                        utils_nvs_set_int(NVS_CASH2_KEY, CashTotals[1]);
                    if (LastInputPin == 3)
                        utils_nvs_set_int(NVS_CASH3_KEY, CashTotals[2]);
                    if (LastInputPin == 4)
                        utils_nvs_set_int(NVS_CASH4_KEY, CashTotals[3]);
                    if (LastInputPin == 5)
                        utils_nvs_set_int(NVS_CASH5_KEY, CashTotals[4]);
                    if (LastInputPin == 6)
                        utils_nvs_set_int(NVS_CASH6_KEY, CashTotals[5]);
                    if (LastInputPin == 7)
                        utils_nvs_set_int(NVS_CASH7_KEY, CashTotals[6]);

                    // ESP_LOGI("COIN","Input Pin %d Pulses %d",LastInputPin,TotalPulses);
                   if (gpio_get_level(JUMPER) == 0)
                   {
                        sprintf(payload, "*RP,%d,%d#",LastInputPin,TotalPulses); 
                        send(sock, payload, strlen(payload), 0);
                   }
                   // create same pules on same output pin 17-06-24
                   // provided this is not hardware test mode
                   if (HardwareTestMode == 0)
                   {
                    edges = TotalPulses * 2;
                    pin = LastInputPin;
                   }
                   sprintf(payload, "*RP,%d,%d#",LastInputPin,TotalPulses); 
                   uart_write_string(payload);
                   ESP_LOGI(TAG,"*RP,%d,%d#",LastInputPin,TotalPulses);

                   TotalPulses = 0;
                }
            }
        }
        if (LastValue != InputPin)
        {
            LastValue = InputPin;
            DebounceCount = 5;
        }
        else
        {
            if (DebounceCount)
            {
                DebounceCount--;
                if (DebounceCount == 0)
                {
                    if (InputPin == 0)
                    {
                    }
                    if (InputPin != 0)
                    {
                    TotalPulses++;                                      
                    PulseStoppedDelay = 100;
                    LastInputPin = InputPin;
                    }
                }
            }

        }
        vTaskDelay(5/portTICK_PERIOD_MS);
    }
}


// static void gpio_read_n_act_old(void* arg)
// {
//     char buff[50];
//     uint16_t Pin = 0;
//     uint16_t Counter = 0;
//     for (;;) {

//         x=0;
//         Pin = 0;
//         if (gpio_get_level(ICH1) == 0)
//         {
//             x=x+1;
//             Pin = 1;
//         }
//         if (gpio_get_level(ICH2) == 0)
//         {
//             x=x+2;
//             Pin = 2;
//         }
//         if (gpio_get_level(ICH3) == 0)
//         {
//             x=x+4;
//             Pin = 3;
//         }
//         if (gpio_get_level(ICH4) == 0)
//         {
//             x=x+8;
//             Pin = 4;
//         }
//         if (gpio_get_level(ICH5) == 0)
//         {
//             x=x+16;
//             Pin = 5;
//         }
//         if (gpio_get_level(ICH6) == 0)
//         {
//             x=x+32;
//             Pin = 6;
//         }
//         if (gpio_get_level(ICH7) == 0)
//         {
//             x=x+64;
//             Pin = 7;
//         }
//         if (x!= PreviousSwitchStatus)
//         {
//              PreviousSwitchStatus =x;
//              DebounceCount = 1;   
//              ESP_LOGI("COIN","Counter %d Input Value:%d Pin %d",Counter,SwitchStatus,Pin);
//              if (Pin == 0)
//                 Counter = 0;
//              else  
//                 Counter++;      
//         }   
//         else if (DebounceCount>0)
//         {
//             DebounceCount--;
//             if (DebounceCount == 0)
//             {
//                 SwitchStatus = PreviousSwitchStatus;
//                 if (SwitchStatus != 0)
//                 {
//                     // if firts pulse, start timeout
//                     if (PulseTimeOut == 0)
//                         PulseCount = 1;
//                     else    
//                         PulseCount++;   
//                     PulseTimeOut = 200; // 100  * delay
//                 }         
//                     Out4094Byte(SwitchStatus);    
//  //                   blinkLEDNumber = 2;
//                     //tcp_ip_client_send_str(buff);
                
//             }
//         }
//         if (PulseTimeOut>0)
//         {
//             PulseTimeOut--;
//             if (PulseTimeOut == 0) 
//             {
//                  PulseCount = 0;    
//             //    ESP_LOGI("COIN","*RP:%d:%d#",Pin,PulseCount);
//                 sprintf (buff, "*RP:%d:%d#",Pin,PulseCount);
//                 blinkLEDNumber = 2;
//             //     tcp_ip_client_send_str(buff);
//              }
//         }
//         vTaskDelay(5/portTICK_PERIOD_MS);
//     }
// }


void ICH_init()
{
    ESP_LOGI(TAG,"********Starting ICH INIT*************");
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
//    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = 1ULL << ErasePin | 1ULL << JUMPER | 1ULL << JUMPER2 |1ULL << CINHI | 1ULL << INH | 1ULL << ICH1 | 1ULL << ICH2 | 1ULL << ICH3 | 1ULL << ICH4 | 1ULL << ICH5 | 1ULL << ICH6| 1ULL << ICH7;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);


    // //create a queue to handle gpio event from isr
    // gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    // //start gpio task
// **************** skip reading input
    if (Production)
        xTaskCreate(gpio_read_n_act, "gpio_read_n_act", 2048, NULL, 10, NULL);

    //install gpio isr service
    // gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // //hook isr handler for specific gpio pin
    // gpio_isr_handler_add(ICH1, gpio_isr_handler, (void*) ICH1);
    // gpio_isr_handler_add(ICH2, gpio_isr_handler, (void*) ICH2);
    // gpio_isr_handler_add(ICH3, gpio_isr_handler, (void*) ICH3);
    // gpio_isr_handler_add(ICH4, gpio_isr_handler, (void*) ICH4);
    // gpio_isr_handler_add(ICH5, gpio_isr_handler, (void*) ICH5);
    // gpio_isr_handler_add(ICH6, gpio_isr_handler, (void*) ICH6);
    // gpio_isr_handler_add(ICH7, gpio_isr_handler, (void*) ICH7);
 }


void Out4094 (unsigned char value)
{
    uint8_t i,j;
    uint8_t OutputMap[9] = {99,6,0,4,5,2,3,1,99};
    j = OutputMap[value];
//    ESP_LOGI("OUT4094","pin %d",j);
    for (i = 0 ; i < 8 ; i++)
    {
        if (SignalPolarity == 0)
        {
            if (j == 7-i)
               gpio_set_level(DAT, 1);
            else    
                gpio_set_level(DAT, 0);
        }
        else
        {
            if (j == 7-i)
               gpio_set_level(DAT, 0);
            else    
                gpio_set_level(DAT, 1);
        }

        ets_delay_us(10);
        gpio_set_level(CLK, 1);
        ets_delay_us(10);
        gpio_set_level(CLK, 0);
    }
    ets_delay_us(10);
    gpio_set_level(STRB, 1);
    ets_delay_us(10);
    gpio_set_level(STRB, 0);
    // if (value<7)
    //     ESP_LOGI("OUT4094","Start Pulse %d is %lu",edges,xTaskGetTickCount());
    // else
    //     ESP_LOGI("OUT4094","End Pulses %d is %lu",edges,xTaskGetTickCount());

}

// generate pulses in background
// as soon as pulses value is non zero - generate 1 pulse and decrement pulses by 1 

            // if (edges%2 == 0)
            // {
            //     Out4094(pin);
            //     ESP_LOGI("OUT4094","Start Pulse %d is %lu",edges,xTaskGetTickCount());
            // }
            // else
            // {    
            //     Out4094(8);
            //     ESP_LOGI("OUT4094","End Pulse %d is %lu",edges,xTaskGetTickCount());
            // }

// blink LED as per number - set led on, wait, led off, clear led number
void BlinkLED (void)
{
    for (;;)
    {
        if (blinkLEDNumber>0)
        {
            if (blinkLEDNumber==1)
            {
                gpio_set_level(L1, 1);
                vTaskDelay(500/portTICK_PERIOD_MS);
                gpio_set_level(L1, 0);
                blinkLEDNumber = 0;
            }
            if (blinkLEDNumber==2)
            {
                gpio_set_level(L2, 1);
                vTaskDelay(500/portTICK_PERIOD_MS);
                gpio_set_level(L2, 0);
                blinkLEDNumber = 0;
            }
            if (blinkLEDNumber==3)
            {
                gpio_set_level(L1, 1);
                vTaskDelay(500/portTICK_PERIOD_MS);
                gpio_set_level(L3, 0);
                blinkLEDNumber = 0;
            }
        }
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}


void GeneratePulsesInBackGround (void)
{
    char buffer[100];
    int pulses = 0;
    for (;;)
    {
        if (edges)
        {
            if (edges%2 == 0)
            {
                Out4094(pin);
            }
            else
            {    
                Out4094(8);
            }
            edges--;
            pulses++;
            if (edges == 0)
            {
                 ESP_LOGI("GenPulse","Pulse Width is %d ",pulseWitdh);
                 sprintf(buffer,"*Generate Pulses %d on Pin %d#",pin,pulses/2);
                 uart_write_string_ln(buffer);
                 pulses=0;
            }
        }
        vTaskDelay(pulseWitdh/portTICK_PERIOD_MS);
    }
}

void TestCoin (void)
{
    char buffer[100];
    int j;
    for (;;) 
    {
        if (HardwareTestMode)
        {
            gpio_set_level(L1, 0);
            gpio_set_level(L2, 0);
            gpio_set_level(L3, 0);
            pin++;
            if (pin>7)
            {
                j = 0;
                pin = 1;
                HardwareTestCount++;
                for (int i = 0 ; i < 7 ; i++)
                {
                    if ( CashTotals[i] != HardwareTestCount)
                    {
                        sprintf(buffer, "Error - Pin %d & Count Number %d ",i+1,CashTotals[i]); //actual when in production
                        ESP_LOGI("TestCoin","Error %d - Pin %d, Count",i+1 , CashTotals[i]);
                        uart_write_string_ln(buffer);
                        j++;
                    }
                    if (j == 0)
                    {    
                        ESP_LOGI("TestCoin","Test Cycle Okay");
                        uart_write_string_ln("Test Cycle Okay");        
                    }   
                }
            }   
            if ((pin == 1) || (pin == 4))
                gpio_set_level(L1, 1);
            if ((pin == 2) || (pin == 5))
                gpio_set_level(L2, 1);
            if ((pin == 3) || (pin == 6))
                gpio_set_level(L3, 1);

            edges = 2;    
            ESP_LOGI("TestCoin","Test Coin Pin Number %d ",pin);
            sprintf(buffer, "Test Coin Pin Number %d ",pin); //actual when in production
            uart_write_string_ln(buffer);
            
        }
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}


void TestRGB (void)
{
    gpio_set_level(L1, 0);
    gpio_set_level(L2, 0);
    gpio_set_level(L3, 0);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    gpio_set_level(L1, 1);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    
    gpio_set_level(L1, 0);
    gpio_set_level(L2, 1);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    gpio_set_level(L2, 0);
    gpio_set_level(L3, 1);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    gpio_set_level(L3, 0);
    
}

void Test4094 (void)
{
    for (;;) 
    {
        Test4094Count++;
        if (Test4094Count == 8)
            Test4094Count = 0;

        gpio_set_level(L1, 0);
        gpio_set_level(L2, 0);
        gpio_set_level(L3, 0);
        if ((Test4094Count == 0) || (Test4094Count == 3))
            gpio_set_level(L1, 1);
        if ((Test4094Count == 1) || (Test4094Count == 4))
            gpio_set_level(L2, 1);
        if ((Test4094Count == 2) || (Test4094Count == 5))
            gpio_set_level(L3, 1);

        Out4094(Test4094Count);  
        ESP_LOGI(TAG, "Pulse 4094 %d",Test4094Count);  
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}

void s2p_init(){
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = 1ULL << STRB | 1ULL << CLK | 1ULL << DAT | 1ULL << CINHO | 1ULL << L1 | 1ULL << L2 | 1ULL << L3 ;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
    gpio_set_level(STRB, 0);
    gpio_set_level(CLK, 0);
    gpio_set_level(DAT, 0);
    gpio_set_level(L1, 0);
    gpio_set_level(L2, 0);
    gpio_set_level(L3, 0);
    if (INHOutputValue != 0)
    {
        INHOutputValue = 1;
        gpio_set_level(CINHO, 0);
    }
    else
    {
        gpio_set_level(CINHO, 1);
    }

    
    Test4094Count = 0;
    ESP_LOGI(TAG, "4094 IOs,RGB initialised");  
    xTaskCreate(GeneratePulsesInBackGround, "GeneratePulsesInBackGround", 2048, NULL, 9, NULL);

}

void read_mac_address(){
    uint8_t macAddress[6];
    esp_err_t err = esp_read_mac(macAddress, ESP_MAC_WIFI_STA);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "*MAC Address: %02x:%02x:%02x:%02x:%02x:%02x#\n",
            macAddress[0], macAddress[1], macAddress[2],
            macAddress[3], macAddress[4], macAddress[5]);
        sprintf(MAC_ADDRESS_ESP, "%02X:%02X:%02X:%02X:%02X:%02X",
            macAddress[0], macAddress[1], macAddress[2],
            macAddress[3], macAddress[4], macAddress[5]);
    } else {
        ESP_LOGE(TAG, "Failed to read MAC address. Error code: %d\n", err);
    }
}

void led_set_level(gpio_num_t gpio_num, int state){
    #ifdef LED_ACTIVE_HIGH
        gpio_set_level(gpio_num, state);
    #else
        gpio_set_level(gpio_num, !state);
    #endif
}

bool extractSubstring(const char* str, char* result) {
    const char* start = strchr(str, '*');
    const char* end = strchr(str, '#');

    if (start != NULL && end != NULL && end > start + 1) {
        strncpy(result, start + 1, end - start - 1);
        result[end - start - 1] = '\0';
        return true;
    }

    return false;
}

uint32_t millis(void) {
    return (uint32_t)(esp_timer_get_time() / 1000ULL);
}

void resolve_hostname(const char *hostname) {
    struct addrinfo hints, *res;
    int status;
    // char ipstr[100];
    char payload[200];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // AF_INET for IPv4, AF_INET6 for IPv6
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
        // Corrected here: use gai_strerror instead of strerror
        fprintf(stderr, "getaddrinfo: %s\n", strerror(status));
        return;
    }

    printf("*IP addresses for %s:\n\n#", hostname);
    ESP_LOGI(TAG,"*IP addresses for %s:\n\n#", hostname);
    struct addrinfo *p;
    for (p = res; p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;

        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        sprintf(payload,"*IP VER - %s: IP STR - %s#", ipver, ipstr);
        uart_write_string(payload);
//        ESP_LOGI(TAG,payload);

    }

    freeaddrinfo(res); // free the linked list
}

