#ifndef PWM_UTILS_H
#define PWM_UTILS_H

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_attr.h"
#include "soc/rtc.h"
#include "esp_log.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "IPC_utils.h"




#define CAP0_INT_EN BIT(27)  //Capture 0 interrupt bit
#define GPIO_CAP0_IN GPIO_NUM_32  
#define PWM_TAG "PWM"



static mcpwm_dev_t *MCPWM[2] = {&MCPWM0, &MCPWM1};
uint32_t current_cap_value = 0;
uint32_t previous_cap_value = 0;
float prev_freq = 0;
float sum = 0;



/**
 * @brief this is ISR handler function, here we check for interrupt that triggers rising edge on CAP0 signal and according take action
 */
static void IRAM_ATTR isr_handler(void *arg)
{   
    uint32_t mcpwm_intr_status;
    Capture_t Qmsg;
    mcpwm_intr_status = MCPWM[MCPWM_UNIT_0]->int_st.val; //Read interrupt status
    if (mcpwm_intr_status & CAP0_INT_EN) { //Check for interrupt on rising edge on CAP0 signal
        // current_cap_value[0] = mcpwm_capture_signal_get_value(MCPWM_UNIT_0, MCPWM_SELECT_CAP0); //get capture signal counter value
        // Qmsg.capture_signal = (current_cap_value[0] - previous_cap_value[0]) / (rtc_clk_apb_freq_get() / 1000000);
        // previous_cap_value[0] = current_cap_value[0];
        // Qmsg.sel_cap_signal = MCPWM_SELECT_CAP0;
        current_cap_value += 1;
        Qmsg.pulses = current_cap_value;
        xQueueSendFromISR(cap_queue, &Qmsg, NULL);
    }
    MCPWM[MCPWM_UNIT_0]->int_clr.val = mcpwm_intr_status;
}


/**
 * @brief Configure whole MCPWM module for bldc motor control
 */
static void PWMInit(void* unused)
{
    ESP_LOGI(PWM_TAG,"initializing mcpwm bldc control gpio...\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_CAP_0, GPIO_CAP0_IN);
    //gpio_set_intr_type(GPIO_CAP0_IN,GPIO_INTR_POSEDGE);     
    gpio_pulldown_en(GPIO_CAP0_IN);    //Enable pull down on CAP0   signal
    mcpwm_capture_enable(MCPWM_UNIT_0, MCPWM_SELECT_CAP0, MCPWM_POS_EDGE, 0);  //capture signal on rising edge, pulse num = 0 i.e. 800,000,000 counts is equal to one second
    MCPWM[MCPWM_UNIT_0]->int_ena.val = CAP0_INT_EN;  //Enable interrupt on  CAP0, CAP1 and CAP2 signal
    
    mcpwm_isr_register(MCPWM_UNIT_0, isr_handler, NULL, ESP_INTR_FLAG_IRAM, NULL);  //Set ISR Handler
    vTaskDelete(NULL);
}


#endif