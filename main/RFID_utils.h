#ifndef RFID_UTILS_H
#define RFID_UTILS_H

#include "esp_log.h"
#include "rc522.h"

#define RFID_TAG "RFID"

void onTag(uint8_t* sn) { // serial number is always 5 bytes long
    ESP_LOGI(RFID_TAG, "Tag: %#x %#x %#x %#x %#x",
        sn[0], sn[1], sn[2], sn[3], sn[4]
    );
}

void RFIDInit(void) {
    rc522_start_args_t start_args; 
    start_args.miso_io  = 19;
    start_args.mosi_io  = 23;
    start_args.sck_io   = 18;
    start_args.sda_io   = 4;
    start_args.callback = &onTag;

        // Uncomment next line for attaching RC522 to SPI2 bus. Default is VSPI_HOST (SPI3)
    start_args.spi_host_id = VSPI_HOST;
   

    rc522_start(start_args);
}




#endif