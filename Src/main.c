//
// Created by i.stefanov on 2.4.2026 г..
//

#include "app_state.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_gcc.h"
#include "dac.h"
#include "encoder.h"
#include "error.h"
#include "i2c.h"
#include "power.h"
#include "stm32l0xx_hal.h"

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName ) {
    ERROR_TriggerFatal();
}

int main(void) {
    uint8_t err;
    if ((err = HAL_Init()) != HAL_OK) {
        while (1);
    }

    // Initialize error handling
    ERROR_Init();

    // Setup power and clocks
    if ((err = POWER_Init()) != HAL_OK) {
        ERROR_TriggerFatal();
    }

    // Setup I2C Bus
    if ((err = I2C_Init()) != 0) {
        ERROR_TriggerFatal();
    }

    // Initialize DAC
    if ((err = DAC_Init()) != 0) {
        ERROR_TriggerFatal();
    }

    // Initialize encoder
    if ((err = ENCODER_Init()) != HAL_OK) {
        ERROR_TriggerFatal();
    }

    // Start DAC tasks
    DAC_StartTasks();

    // Start encoder tasks
    ENCODER_StartTasks();

    // Start scheduler
    vTaskStartScheduler();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == DAC_CHCTRL_1_PIN || GPIO_Pin == DAC_CHCTRL_2_PIN) {
        DAC_HandleChanCtrlEvent();
    } else if (GPIO_Pin == ENCODER_1_RESET_PIN) {
        xTaskNotify(gAppState.Tasks.Enc1Task.OsTask, ENCODER_NOTIFY_RESET, eNoAction);
    } else if (GPIO_Pin == ENCODER_2_RESET_PIN) {
        xTaskNotify(gAppState.Tasks.Enc2Task.OsTask, ENCODER_NOTIFY_RESET, eNoAction);
    }
}