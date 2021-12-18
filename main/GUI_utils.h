#ifndef GUI_UTILS_H
#define GUI_UTILS_H


extern "C" { 
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/dac.h"


#include "sdkconfig.h"
#include "ili9341.h"


#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "img.h"

#include "lvgl_helpers.h"

#define GUI_TAG "TFT"
#define LV_TICK_PERIOD_MS 1

lv_style_t style_common;
lv_obj_t * scene_bg;
lv_obj_t * scene_bg2;
lv_obj_t * scr;
lv_obj_t * img_schlum;

static void lv_tick_task(void *arg);
void guiTask(void *pvParameter);

void scanScreen(void);
void startScreen(void);
void initGUI(void*);
void countingScreen(void);



/* Creates a semaphore to handle concurrent call to lvgl stuff
 * If you wish to call *any* lvgl function from other threads/tasks
 * you should lock on the very same semaphore! */
SemaphoreHandle_t xGuiSemaphore;

void guiTask(void *pvParameter) {
    ESP_LOGI(GUI_TAG,"GUI Initalization");
    schlumberger.header.always_zero = 0;
    schlumberger.header.w = 320;
    schlumberger.header.h = 240;
    schlumberger.data_size = 320*240*2;
    schlumberger.header.cf = LV_IMG_CF_TRUE_COLOR;
    schlumberger.data = schlum_data;
    

    (void) pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();
    dac_output_enable(DAC_CHANNEL_1);
    dac_output_voltage(DAC_CHANNEL_1,64);

    lv_init();

    /* Initialize SPI or I2C bus used by the drivers */
    lvgl_driver_init();  // this is the display init TODO
    
    lv_color_t* buf1 = (lv_color_t*)heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);
    
    /* Use double buffered when not working with monochrome displays */
#ifndef CONFIG_LV_TFT_DISPLAY_MONOCHROME
    lv_color_t* buf2 = (lv_color_t*)heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);
#else
    static lv_color_t *buf2 = NULL;
#endif

    static lv_disp_buf_t disp_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;
    /* Initialize the working buffer depending on the selected display.
     * NOTE: buf2 == NULL when using monochrome displays. */
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);
    
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;

    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    /* Create the demo application */

    xTaskCreatePinnedToCore(initGUI,"initGUI",4096*2, NULL, 0, NULL, 1);
    /* A task should NEVER return */
    ESP_LOGI(GUI_TAG,"GUI Initialized");
    startScreen();
    
    vTaskDelay(3000/portTICK_PERIOD_MS);
    scanScreen();
    vTaskDelay(3000/portTICK_PERIOD_MS);
    countingScreen();
 

    while(1){};
    vTaskDelete(NULL);
}

lv_color_t hexToColor(int hex)
{
    return lv_color_make((hex >> 8) & 0xff, (hex >> 0) & 0xff, (hex >> 16) & 0xff);
}


void create_label(lv_obj_t *label,lv_style_t *style, const char text[],int color, int x_off, int y_off)
{   
    lv_style_reset(style);
    lv_style_set_text_font(style, LV_STATE_DEFAULT, &lv_font_montserrat_42);
    lv_obj_add_style(label, LV_OBJ_PART_MAIN, style);
    lv_label_set_text(label, text);
    lv_obj_set_style_local_text_color(label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, hexToColor(color));
    lv_obj_align(label, scene_bg2, LV_ALIGN_CENTER, x_off, y_off);
    
}

void initGUI(void* unused)
{
    lv_style_init(&style_common);
    LV_FONT_DECLARE(lv_font_montserrat_12_compr_az);
    LV_FONT_DECLARE(lv_font_montserrat_16_compr_az);
    LV_FONT_DECLARE(lv_font_montserrat_38);
    LV_FONT_DECLARE(lv_font_montserrat_42);
    LV_FONT_DECLARE(lv_font_montserrat_48);
    
    scr = lv_disp_get_scr_act(NULL);
    scene_bg = lv_obj_create(scr, NULL);
    lv_obj_add_style(scene_bg, LV_OBJ_PART_MAIN,&style_common);
    lv_obj_set_size(scene_bg, lv_obj_get_width(scr)+5, lv_obj_get_height(scr)+5);
    lv_obj_align(scene_bg, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_color(scene_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, hexToColor(0x1703FC));
    lv_obj_set_style_local_border_color(scene_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, hexToColor(0x1703fc));
    while (1) 
    {
    /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
    vTaskDelay(pdMS_TO_TICKS(10));

    /* Try to take the semaphore, call lvgl related function on success */
    if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
        lv_task_handler();
        xSemaphoreGive(xGuiSemaphore);
        }
    }
}

void startScreen(void)
{    
    ESP_LOGI(GUI_TAG,"Schlumberger start screen");
    LV_IMG_DECLARE(schlumberger);
    vTaskDelay(10/portTICK_PERIOD_MS);
    if(xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
    {
    lv_obj_clean(scene_bg);
    img_schlum = lv_img_create(scene_bg, NULL);
    //lv_obj_add_style(img_schlum,LV_OBJ_PART_MAIN, &style_common);
    lv_img_set_src(img_schlum, &schlumberger);
    xSemaphoreGive(xGuiSemaphore);
    }
}

void scanScreen(void)
{   
    
    ESP_LOGI(GUI_TAG,"Scan Screen");
    vTaskDelay(10/portTICK_PERIOD_MS);
    if(xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
    {
        lv_obj_clean(scene_bg);
        scene_bg2 = lv_obj_create(scene_bg, NULL);
        lv_obj_add_style(scene_bg2, LV_OBJ_PART_MAIN, &style_common);
        lv_obj_set_size(scene_bg2, lv_obj_get_width(scr)+5, lv_obj_get_height(scr)+5);
        lv_obj_align(scene_bg2, NULL, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_local_bg_color(scene_bg2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, hexToColor(0xFFFFFF));
        lv_obj_set_style_local_border_color(scene_bg2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, hexToColor(0xFFFFFF));
        
        
        lv_obj_t * label1 =  lv_label_create(scene_bg2, NULL);
        lv_obj_t * label2 =  lv_label_create(scene_bg2, NULL);
        lv_obj_t * label3 =  lv_label_create(scene_bg2, NULL);
        create_label(label1, &style_common, "02775 109 44", 0x003060, 0, -38);
        create_label(label2, &style_common, "Renault", 0x003060, 0, 0);
        create_label(label3, &style_common, "Clio 3", 0x003060, 0, 38);
        xSemaphoreGive(xGuiSemaphore);   
    } 
}

void countingScreen(void)
{   

    ESP_LOGI(GUI_TAG,"Counting Screen");
    vTaskDelay(10/portTICK_PERIOD_MS);
    if(xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
    {
        lv_obj_clean(scene_bg);
        scene_bg2 = lv_obj_create(scene_bg, NULL);
        lv_obj_add_style(scene_bg2, LV_OBJ_PART_MAIN, &style_common);
        lv_obj_set_size(scene_bg2, lv_obj_get_width(scr)+5, lv_obj_get_height(scr)+5);
        lv_obj_align(scene_bg2, NULL, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_local_bg_color(scene_bg2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, hexToColor(0xFFFFFF));
        lv_obj_set_style_local_border_color(scene_bg2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, hexToColor(0xFFFFFF));
        lv_obj_t * label1 =  lv_label_create(scene_bg2, NULL);
        lv_style_reset(&style_common);
        lv_style_set_text_font(&style_common, LV_STATE_DEFAULT, &lv_font_montserrat_48);
        
        lv_obj_add_style(label1, LV_OBJ_PART_MAIN, &style_common);
        lv_label_set_text(label1, "0L");
        lv_obj_set_style_local_text_color(label1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, hexToColor(0x003060));
        lv_obj_align(label1, scene_bg2, LV_ALIGN_CENTER, 0, 0);
        xSemaphoreGive(xGuiSemaphore);
        vTaskDelay(10/portTICK_PERIOD_MS);
        for(float i = 0; i < 100; i+=0.1)
        {   
            if(xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
            {
                char string[7] = {'0','0','0','0','0','L','\0'};
                sprintf(string,"%.2fL",i);
                ESP_LOGI("TFT","%s",string);
                lv_label_set_text(label1, string);
                lv_obj_align(label1, scene_bg2, LV_ALIGN_CENTER, 0, 0);
                xSemaphoreGive(xGuiSemaphore);
                vTaskDelay(10/portTICK_PERIOD_MS);
            }
        }
        
    }
}
static void lv_tick_task(void *arg) 
{
    (void) arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}



}
#endif