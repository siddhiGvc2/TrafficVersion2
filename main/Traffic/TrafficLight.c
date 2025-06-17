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
#include "lvgl.h"
#include "lv_conf.h"
#include "externVars.h"
#include "calls.h"


static const char *TAG = "TrafficLight";



lv_obj_t *time_label;  // Global time label referenc
lv_obj_t *color_label[4];  // For 4 CDTColor labels
lv_obj_t *stripe_colors[4];  // For 4 CDTColor labels
lv_color_t colors[5];

void init_colors() {
    colors[0] = lv_color_white();
    colors[1] = lv_color_make(0, 0, 255);
    colors[2] =  lv_color_make(0, 152, 255);
    colors[3] =  lv_color_make(0, 128, 0);
    colors[4] =  lv_color_black();
    
}

void DecodeTL(const char* input){
    char CurText;
    int CurValue,i,j,k;
    char payload[256]; 
    ESP_LOGI(TAG,"HOURS:%d,MINS:%d,SECS:%d",Hours,Mins,Secs);
        // sprintf(payload,"HOURS:%d,MINS:%d,SECS:%d",Hours,Mins,Secs); 
      
      ESP_LOGI(TAG,"DECODING Road: %d,Input:%s",road,input);
      sprintf(payload,"DECODING Road: %d,Input:%s",road,input);
      sprintf(payload,"*R,%d,%s#",road,input);
      uart_write_string_ln(payload);
   
     if (sscanf(input, "%c%d", &CurText, &CurValue) == 2) 
     {
        sprintf(CDTColor[road-1], "%c", CurText);  // Store character as string
        CDTime[road-1] = CurValue;
        CDTimeInput[road-1]=CurValue;
        sprintf(payload,"*R%d,C%c,T%d#",road,CurText,CurValue);
        uart_write_string_ln(payload);
    
   
        if (CurText == 'G')
        {

        }
    }   
}

void DecodeStage(const char* input){
    ESP_LOGI(TAG,"waiting to start decoding.%s",input);
  
    sscanf(input, "%d,%[^,],%99[^:]",&road,CurInput,NextCurInput);
    DecodeTL(CurInput);
    DecodeTL(NextCurInput);
  

}


void CalculateRedTimeings (void)
{
            int i,j,k;
            char payload[100];
            i = road-1;
         
            k = PhaseTime[i*2] + PhaseTime[i*2 + 1];
            i = (i+1) % MAX_ITEMS ;
            CDTime[i] = k;
            sprintf(CDTColor[i], "R"); 

            k = k+ PhaseTime[i*2] + PhaseTime[i*2 + 1];
            i = (i+1) % MAX_ITEMS ;
            CDTime[i] = k;
            sprintf(CDTColor[i], "R"); 

            k = k+ PhaseTime[i*2] + PhaseTime[i*2 + 1];
            i = (i+1) % MAX_ITEMS ;
            CDTime[i] = k;
            sprintf(CDTColor[i], "R"); 


            sprintf(payload,"*******New Time - Time/Color %s%d  %s%d  %s%d  %s%d*****", CDTColor[0],CDTime[0],CDTColor[1],CDTime[1],CDTColor[2],CDTime[2],CDTColor[3],CDTime[3]);   
            uart_write_string_ln(payload);    

}
void CalculateAllTime (void)
{
    char CurText;
    int CurValue;
    char payload[100];
    DecodeStage(DEF1);
    sscanf(CurInput, "%c%d", &CurText, &CurValue);
    if (CurText == 'G') 
     PhaseTime[0] = CurValue;
    if (CurText == 'A') 
    {
         PhaseTime[1] = CurValue;
    }
    sscanf(NextCurInput, "%c%d", &CurText, &CurValue);
    if (CurText == 'G') 
     PhaseTime[0] = CurValue;
    if (CurText == 'A') 
    {
     PhaseTime[1] = CurValue;
    }    


     DecodeStage(DEF2);
    sscanf(CurInput, "%c%d", &CurText, &CurValue);
    if (CurText == 'G') 
     PhaseTime[2] = CurValue;
    if (CurText == 'A') 
    {
         PhaseTime[3] = CurValue;
    }
    sscanf(NextCurInput, "%c%d", &CurText, &CurValue);
    if (CurText == 'G') 
     PhaseTime[2] = CurValue;
    if (CurText == 'A') 
    {
         PhaseTime[3] = CurValue;
    }    


     DecodeStage(DEF3);
    sscanf(CurInput, "%c%d", &CurText, &CurValue);
    if (CurText == 'G') 
     PhaseTime[4] = CurValue;
    if (CurText == 'A') 
     PhaseTime[5] = CurValue;
    sscanf(NextCurInput, "%c%d", &CurText, &CurValue);
    if (CurText == 'G') 
     PhaseTime[4] = CurValue;
    if (CurText == 'A') 
    {
         PhaseTime[5] = CurValue;
    }    


     DecodeStage(DEF4);
    sscanf(CurInput, "%c%d", &CurText, &CurValue);
    if (CurText == 'G') 
     PhaseTime[6] = CurValue;
    if (CurText == 'A') 
    {
         PhaseTime[7] = CurValue;
    }
    sscanf(NextCurInput, "%c%d", &CurText, &CurValue);
    if (CurText == 'G') 
     PhaseTime[6] = CurValue;
    if (CurText == 'A') 
    {
     PhaseTime[7] = CurValue;
    }    


    sprintf(payload,"Phase %d %d %d %d %d %d %d %d ", PhaseTime[0],PhaseTime[1],PhaseTime[2],PhaseTime[3],PhaseTime[4],PhaseTime[5],PhaseTime[6],PhaseTime[7]);
    uart_write_string_ln(payload);
    CDTime[0] = PhaseTime[0];
    sprintf(CDTColor[0], "G"); 
    road = 1;
    stage = 1;
    CalculateRedTimeings();
}


void decrement_CDTColor() {
    int i;
    char payload[100];
    for (i = 0; i < 4; i++) 
    {
        int value;
       
        if(CDTime[i]==0 && (strstr(CDTColor[i], "G") != NULL))
        {
                  sprintf(CDTColor[i], "A"); 
                   CDTime[i] = PhaseTime[i*2+1];
         }
         else if(CDTime[i] == 0 && (strstr(CDTColor[i], "A") != NULL)) {
             sprintf(CDTColor[i], "R"); 
             stage++;
             if (stage > MAX_ITEMS)
                stage = 1;
             road = stage;
                 switch(stage) {
                     case 1:
                         if(strcmp(stageMode, "FIXED") == 0) {
                             CDTime[0] = PhaseTime[0];
                             sprintf(CDTColor[0], "G"); 
                         } else {
                             DecodeStage(ATC1);
                         }
                         break;
                     case 2:
                         if(strcmp(stageMode, "FIXED") == 0) {
                             CDTime[1] = PhaseTime[2];
                             sprintf(CDTColor[1], "G"); 
                         } else {
                             DecodeStage(ATC2);
                         }
                         break;
                     case 3:
                         if(strcmp(stageMode, "FIXED") == 0) {
                             CDTime[2] = PhaseTime[4];
                            sprintf(CDTColor[2], "G"); 
    
                         } else {
                             DecodeStage(ATC3);
                         }
                         break;
                     case 4:
                         if(strcmp(stageMode, "FIXED") == 0) {
                              CDTime[3] = PhaseTime[6];
                              sprintf(CDTColor[3], "G"); 

                        } else {
                             DecodeStage(ATC4);
                         }
                        
                         break;
                 }
 
                 CalculateRedTimeings();

            }

// if server mode and RED color display ATC
        if ( (strstr(CDTColor[i], "R") != NULL) && (strstr(stageMode,"SERVER")!=NULL)){
            strcpy(Command,"ATC");
            CDTime[i] = 0;
            lv_label_set_text_fmt(color_label[i], "%s", Command);
        }
      
// if fixed mode or server mode and green/amber color display time       
        else {
            lv_label_set_text_fmt(color_label[i], "%d", CDTime[i]);
           // ESP_LOGI(TAG,"Color Label %02d changed to %02d",i,CDTime[i]);
        }
        if (CDTColor[i][0] != '\0')  // ✅ Correct null character check
            {
                if (strstr(CDTColor[i], "R") != NULL)
                {
                    lv_obj_set_style_bg_color(stripe_colors[i], colors[1], LV_PART_MAIN);
                }
                else if (strstr(CDTColor[i], "A") != NULL)
                {
                    lv_obj_set_style_bg_color(stripe_colors[i], colors[2], LV_PART_MAIN);
                }
                else if (strstr(CDTColor[i], "G") != NULL)
                {
                    lv_obj_set_style_bg_color(stripe_colors[i], colors[3], LV_PART_MAIN);
                }
              
            }
            CDTime[i]--;
            
        }      
            sprintf(payload,"*%s,%s%d,%s%d,%s%d,%s%d#", stageMode,CDTColor[0],CDTime[0],CDTColor[1],CDTime[1],CDTColor[2],CDTime[2],CDTColor[3],CDTime[3]);   

        uart_write_string_ln(payload);    
}




// ⏰ Timer callback function
void update_time_label(void) {
    while (1) {
        // your task loop
         
    Secs++;
    decrement_CDTColor(); 
    if (Secs >= 60) {
        Secs = 0;
        Mins++;
        if (Mins >= 60) {
            Mins = 0;
            Hours++;
            if (Hours >= 24) {
                Hours = 0;
            }
        }
    }
   
        if (time_label != NULL && connected_to_wifi && TimeReceived) {
      
        lv_label_set_text_fmt(time_label, "%02d:%02d:%02d",Hours, Mins, Secs);
        
        }
        else if(time_label != NULL && !connected_to_wifi){
        strcpy(stageMode, "FIXED");
        lv_label_set_text_fmt(time_label,"%s","");    
        }
    


        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
      
}


void displayLights(void) {
    init_colors();

    lv_obj_t * parent = lv_scr_act();
    lv_obj_set_style_bg_color(parent, lv_color_white(), LV_PART_MAIN); // Set white background
    static lv_style_t style_label;
   
    const char *color_names[] = {"WHITE", "RED", "ORANGE", "GREEN", "GREEN"};
    
    int y_offset = 0;
    int stripe_height = 50; // Adjust stripe height as needed

    for (int i = 0; i < 5; i++) {
        // Create a colored stripe
        lv_obj_t * stripe = lv_obj_create(parent);
        lv_obj_set_size(stripe, lv_pct(100), stripe_height);
        lv_obj_align(stripe, LV_ALIGN_TOP_LEFT, 0, y_offset);
     
        if(i==0)
        {
            time_label = lv_label_create(stripe);
            if(connected_to_wifi)
            {
            lv_label_set_text_fmt(time_label,"%d:%d:%d",Hours,Mins,Secs);
            }
            else{
               lv_label_set_text_fmt(time_label,"%s","");  
            }
            lv_obj_set_style_text_color(time_label, lv_color_black(), LV_PART_MAIN);
            lv_obj_set_style_bg_color(stripe,colors[0], LV_PART_MAIN);
            lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 0); // Center text within the stripe
            lv_style_init(&style_label);
            lv_style_set_text_font(&style_label, &lv_font_montserrat_22);
            lv_obj_add_style(time_label, &style_label, LV_PART_MAIN);

        }

        if(i>0)
        {
            stripe_colors[i-1]=stripe;
        if(strstr(CDTColor[i-1],"R")!=NULL)
        {
            lv_obj_set_style_bg_color(stripe_colors[i-1], colors[1], LV_PART_MAIN);
        }
        else if(strstr(CDTColor[i-1],"A")!=NULL)
        {
            lv_obj_set_style_bg_color(stripe_colors[i-1], colors[2], LV_PART_MAIN);
        }
        else if(strstr(CDTColor[i-1],"G")!=NULL)
        {
            lv_obj_set_style_bg_color(stripe_colors[i-1], colors[3], LV_PART_MAIN);
        }
        else{
            lv_obj_set_style_bg_color(stripe_colors[i-1], lv_color_black(), LV_PART_MAIN);
        }
        color_label[i - 1] = lv_label_create(stripe_colors[i-1]);  // Create label and assign to array
        lv_label_set_text_fmt(color_label[i - 1], "%d", CDTime[i - 1]);
        lv_obj_set_style_text_color(color_label[i - 1], lv_color_white(), LV_PART_MAIN);
        lv_obj_align(color_label[i - 1], LV_ALIGN_CENTER, 0, 0); // Center text within the stripe
        lv_style_init(&style_label);
        lv_style_set_text_font(&style_label, &lv_font_montserrat_22);
        lv_obj_add_style(color_label[i - 1], &style_label, LV_PART_MAIN);
        
        }
      
     
        // Create label for the color name
       
      
        // lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN); // Default to white text

    

      
        y_offset += stripe_height; // Move to next stripe position
    }
    // if (TimerSet == 0)
    // {
    //     lv_timer_create(update_time_label, 1000, NULL);  // 1000ms = 1s
    //     TimerSet = 1;        
    // }
    
    // lv_timer_create(update_time_label, 1000, NULL);
}

