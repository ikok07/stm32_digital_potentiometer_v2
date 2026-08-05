//
// Created by Kok on 4/9/26.
//

#ifndef STM32_DIGITAL_POTENTIOMETER_V2_ENCODER_H
#define STM32_DIGITAL_POTENTIOMETER_V2_ENCODER_H

#include "stm32l0xx_hal.h"

#define ENCODER_RESET_PORT              GPIOB
#define ENCODER_1_RESET_PIN             4
#define ENCODER_2_RESET_PIN             5

#define ENCODER_NOTIFY_RESET            (1 << 0)

HAL_StatusTypeDef ENCODER_Init();
void ENCODER_StartTasks();

#endif //STM32_DIGITAL_POTENTIOMETER_V2_ENCODER_H