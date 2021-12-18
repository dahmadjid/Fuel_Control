

#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/dac.h"



#include "GUI_utils.h" 
#include "RFID_utils.h"

using std::cout;
using std::endl;
using std::runtime_error;


extern "C" void app_main() 
{
    
   // xTaskCreatePinnedToCore(guiTask, "gui", 4096*2, NULL, 0, NULL, 1);
    //vTaskDelay(1000/portTICK_PERIOD_MS);
    RFIDInit();
}
