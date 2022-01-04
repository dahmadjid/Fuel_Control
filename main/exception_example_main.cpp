#include <iostream>
#include <string>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/dac.h"

#include "GUI_utils.hpp" 
#include "RFID_utils.hpp"
#include "IPC_utils.hpp"
#include "PWM_utils.hpp"
#include "WIFI_utils.hpp"
#include "string_utils.hpp"

#define MAIN_TAG "MAIN"
GuiScreen_t curr_scr = SCHLUMBERGER_SCREEN;

extern "C" void app_main() 
{   

    IPCInit();
    
    dac_output_enable(DAC_CHANNEL_1);
    dac_output_voltage(DAC_CHANNEL_1,64);

    xTaskCreatePinnedToCore(RFIDInit, "RFID", 4096*4, NULL, 8, NULL, 1);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    
    xTaskCreatePinnedToCore(guiTask, "gui", 4096*4, NULL, 3, NULL, 1);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    wifiInit();
    xTaskCreatePinnedToCore(PWMInit, "PWM", 2048*1, NULL, 1, NULL, 0);
    Consumption_t Consumption_Qmsg;
    Capture_t PWM_Qmsg;
    GuiMessage_t GUI_Qmsg;
    Uid RFID_Qmsg;
    float fuel_quota = 0;
    float liters = 0;
    int old_total_pulses = 0;
    GUI_Qmsg.screen = SCHLUMBERGER_SCREEN;
    sendGuiMessage(GUI_Qmsg);
    
    while(1)
    {
        
        if (xQueueReceive(rfid_queue, &RFID_Qmsg, 0))
        {   
            char buf[64];
            sendTag(RFID_Qmsg, buf);
            std::string response_from_server = buf;
            std::vector<std::string> splitted = split(response_from_server, ",");
            
            fuel_quota = stof(splitted[0]);
            std::cout << fuel_quota << std::endl;
            liters = 0;
            strncpy(GUI_Qmsg.label1, splitted[1].c_str(), 15);
            strncpy(GUI_Qmsg.label2, splitted[2].c_str(), 15);
            strncpy(GUI_Qmsg.label3, splitted[3].c_str(), 15);
            
            if (curr_scr != SCAN_SCREEN)
            {
                curr_scr = SCAN_SCREEN;
                GUI_Qmsg.screen = curr_scr;
                sendGuiMessage(GUI_Qmsg);
            }

            vTaskDelay(2500/portTICK_PERIOD_MS);
            ESP_LOGI(MAIN_TAG, "opening valve");
            curr_scr = COUNTING_SCREEN;
            GUI_Qmsg.screen = curr_scr;
            sendGuiMessage(GUI_Qmsg);

        }
        
        if (xQueueReceive(cap_queue, &PWM_Qmsg, 0))
        {   
            if (liters < fuel_quota)
            {
                if (curr_scr != COUNTING_SCREEN)
                {
                    curr_scr = COUNTING_SCREEN;
                    GUI_Qmsg.screen = COUNTING_SCREEN;
                    sendGuiMessage(GUI_Qmsg);
                    vTaskDelay(10/portTICK_PERIOD_MS);
                }
                liters = (float)(PWM_Qmsg.pulses - old_total_pulses)/ 5880;
                Consumption_Qmsg.liters = liters;
                xQueueSend(counting_screen_queue, (void*)&Consumption_Qmsg, 0);
            }
            else
            {   
                old_total_pulses = PWM_Qmsg.pulses;
                if (curr_scr != SCHLUMBERGER_SCREEN)
                {
                    ESP_LOGI(MAIN_TAG, "closing valve");
                    curr_scr = SCHLUMBERGER_SCREEN;
                    GUI_Qmsg.screen = curr_scr;
                    sendGuiMessage(GUI_Qmsg);
                }
                
            }
        }

    vTaskDelay(10/portTICK_PERIOD_MS);
    }
}
