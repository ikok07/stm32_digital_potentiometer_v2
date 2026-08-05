//
// Created by Kok on 4/8/26.
//

#include "stm32l0xx_hal.h"

#define I2C_PORT                            GPIOA
#define I2C_SCL_PIN                         GPIO_PIN_9
#define I2C_SDA_PIN                         GPIO_PIN_10

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_Config = {
        .Mode = GPIO_MODE_AF_OD,
        .Alternate = GPIO_AF1_I2C1,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FAST,
    };

    // Setup SCL
    GPIO_Config.Pin = I2C_SCL_PIN;
    HAL_GPIO_Init(I2C_PORT, &GPIO_Config);

    // Setup SDA
    GPIO_Config.Pin = I2C_SDA_PIN;
    HAL_GPIO_Init(I2C_PORT, &GPIO_Config);
}