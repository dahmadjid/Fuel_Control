#ifndef RFID_UTILS_H
#define RFID_UTILS_H


#include "esp_log.h"
#include "rc522_2.h"

#include <math.h>
#include "inttypes.h"

#include "GUI_utils.hpp"
#include "IPC_utils.hpp"

#define RFID_TAG "RFID"
Uid card_uid;

void onTag(uint8_t* sn) { // serial number is always 5 bytes long
    ESP_LOGI(RFID_TAG, "Tag: %#x %#x %#x %#x %#x",
        sn[0], sn[1], sn[2], sn[3], sn[4]
    );
    // int tag = ((int)rc522_sn_to_u64(sn)) >> 24;
    // GuiMessage_t GUI_Qmsg;
    // GUI_Qmsg.screen = COUNTING_SCREEN;
    // ESP_LOGI(RFID_TAG, "%d", tag);
    // sendGuiMessage(GUI_Qmsg);
}

void RFIDInit(void* unused) {
    spiInit();
	PCD_Init();
	uint8_t r ;
    
	while (1) 
    {
        uint64_t tag = 0;
		r = PICC_IsNewCardPresent();
		if (r) 
        {
            memset(&card_uid,0,sizeof(card_uid));
            
			if (PICC_Select(&card_uid,0) == STATUS_OK) 
            {
                ESP_LOGI(RFID_TAG, "Tag: %#x %#x %#x %#x %#x %#x %#x",card_uid.uidByte[0], card_uid.uidByte[1], card_uid.uidByte[2], card_uid.uidByte[3], card_uid.uidByte[4],card_uid.uidByte[5],card_uid.uidByte[6]);
                for (int i = 0; i < card_uid.size; i++)
                {
                   
                    tag = tag + (card_uid.uidByte[i] * pow(2,(8 * (card_uid.size - i - 1))));
                    
                }
                
                ESP_LOGI(RFID_TAG, "Tag: %llx",tag);
               
                xQueueSend(rfid_queue, (void*)&card_uid, (TickType_t)0);
                
				PICC_HaltA();
			}
			
		}
		vTaskDelay(100 / portTICK_PERIOD_MS);

	}
    
    
    // rc522_start_args_t start_args; 
    // start_args.miso_io  = 19;
    // start_args.mosi_io  = 23;
    // start_args.sck_io   = 18;
    // start_args.sda_io   = 4;
    // start_args.callback = &onTag;
    // start_args.task_priority = 10;
    // rc522_start(start_args);
    ESP_LOGI(RFID_TAG,"RFID Initialized");
    vTaskDelete(NULL);
}




#endif