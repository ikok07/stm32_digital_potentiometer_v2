//
// Created by Kok on 5/12/26.
//

#include "error.h"

#include "stm32l0xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

#define ERROR_LED_GPIOEN                    __HAL_RCC_GPIOB_CLK_ENABLE()
#define ERROR_LED_PORT                      GPIOB
#define ERROR_LED_PIN                       GPIO_PIN_7

void ERROR_Init() {
    GPIO_InitTypeDef GPIO_Config = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pin = ERROR_LED_PIN,
        .Pull = GPIO_PULLDOWN,
        .Speed = GPIO_SPEED_LOW
    };
    HAL_GPIO_Init(ERROR_LED_PORT, &GPIO_Config);
}

void ERROR_Trigger() {
    HAL_GPIO_WritePin(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_PIN_SET);
}

void ERROR_TriggerFatal() {
    HAL_GPIO_WritePin(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_PIN_SET);
    taskDISABLE_INTERRUPTS();
    while (1);
}
