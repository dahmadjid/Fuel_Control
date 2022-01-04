#ifndef _IPC_UTILS_H_
#define _IPC_UTILS_H_

#include <iostream>
#include <string>

#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "rc522_2.h"
#define IPC_TAG "IPC"

QueueHandle_t gui_queue = NULL;
QueueHandle_t cap_queue = NULL;
QueueHandle_t counting_screen_queue = NULL;
QueueHandle_t rfid_queue = NULL;
enum GuiScreen_t 
{
    SCHLUMBERGER_SCREEN = 0,
    SCAN_SCREEN,
    COUNTING_SCREEN
};

struct GuiMessage_t
{
    GuiScreen_t screen;
    char label1[16] = {0};
    char label2[16] = {0};
    char label3[16] = {0};
};
struct Capture_t
{
    int pulses;
};
struct Consumption_t
{
    float liters;
};

void sendGuiMessage(GuiMessage_t Qmsg)
{
    xQueueSend(gui_queue, (void*)&Qmsg, (TickType_t)0);
}

void IPCInit(void)
{
    gui_queue = xQueueCreate(10, sizeof(GuiMessage_t));
    if(gui_queue == NULL)
    {
        ESP_LOGE(IPC_TAG, "failed to create gui_queue");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    cap_queue = xQueueCreate(1, sizeof(Capture_t));
    if(cap_queue == NULL)
    {
        ESP_LOGE(IPC_TAG, "failed to create cap_queue");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    counting_screen_queue = xQueueCreate(10, sizeof(Consumption_t));
    if(counting_screen_queue == NULL)
    {
        ESP_LOGE(IPC_TAG, "failed to create counting_screen_queue");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    rfid_queue = xQueueCreate(1, sizeof(Uid));
    if(rfid_queue == NULL)
    {
        ESP_LOGE(IPC_TAG, "failed to create rfid_queue");
        ESP_ERROR_CHECK(ESP_FAIL);
    }
}


#endif