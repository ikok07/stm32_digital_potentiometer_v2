//
// Created by Kok on 4/7/26.
//

#ifndef STM32_DIGITAL_POTENTIOMETER_V2_DAC_H
#define STM32_DIGITAL_POTENTIOMETER_V2_DAC_H

#include "stm32l0xx.h"
#include "dac_x050x.h"

#define DAC_I2C_ADDRESS                     0x90
#define DAC_I2C_CONN_TRIALS                 3
#define DAC_I2C_CONN_TIMEOUT                1000

#define DAC_CHCTRL_PORT                     GPIOB
#define DAC_CHCTRL_1_PIN                    GPIO_PIN_0
#define DAC_CHCTRL_2_PIN                    GPIO_PIN_1

#define DAC_SHVAL_TIMEOUT                   1000
#define DAC_VALUE_SHVAL_QUEUE_SIZE          1               // One subscriber - the task which operates the channel

uint8_t DAC_Init();

void DAC_StartTasks();

void DAC_HandleChanCtrlEvent();


#endif //STM32_DIGITAL_POTENTIOMETER_V2_DAC_H