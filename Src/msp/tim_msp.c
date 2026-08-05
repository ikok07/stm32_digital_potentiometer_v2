//
// Created by Kok on 4/9/26.
//

#include "app_state.h"
#include "stm32l0xx_hal.h"

#define ENC1_PORT                   GPIOA
#define ENC1_A_PIN                  0
#define ENC1_B_PIN                  1

#define ENC2_PORT                   GPIOA
#define ENC2_A_PIN                  6
#define ENC2_B_PIN                  7


HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority) {
    HAL_StatusTypeDef hal_err = HAL_OK;

    gAppState.htim21 = (TIM_HandleTypeDef){
        .Instance = TIM21,
        .Init = {
            .ClockDivision = TIM_CLOCKDIVISION_DIV1,
            .AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE,
            .CounterMode = TIM_COUNTERMODE_UP,
            .Prescaler = (SystemCoreClock / 1000000) - 1,
            .Period =  (1000000U / configTICK_RATE_HZ) - 1
        }
    };

    if ((hal_err = HAL_TIM_Base_Init(&gAppState.htim21)) != HAL_OK) return hal_err;
    if ((hal_err = HAL_TIM_Base_Start_IT(&gAppState.htim21)) != HAL_OK) return hal_err;
    return hal_err;
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM21) {
        __HAL_RCC_TIM21_CLK_ENABLE();
        HAL_NVIC_SetPriority(TIM21_IRQn, 4, 0);
        HAL_NVIC_EnableIRQ(TIM21_IRQn);
    }

}

void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef *htim) {
    GPIO_InitTypeDef GPIO_Config = {
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_LOW
    };

    __HAL_RCC_GPIOA_CLK_ENABLE();

    if (htim->Instance == TIM2) {
        __HAL_RCC_TIM2_CLK_ENABLE();

        GPIO_Config.Pin = ENC1_A_PIN;
        HAL_GPIO_Init(ENC1_PORT, &GPIO_Config);

        GPIO_Config.Pin = ENC1_B_PIN;
        HAL_GPIO_Init(ENC1_PORT, &GPIO_Config);
    } else if (htim->Instance == TIM22) {
        __HAL_RCC_TIM22_CLK_ENABLE();

        GPIO_Config.Pin = ENC2_A_PIN;
        HAL_GPIO_Init(ENC2_PORT, &GPIO_Config);

        GPIO_Config.Pin = ENC2_B_PIN;
        HAL_GPIO_Init(ENC2_PORT, &GPIO_Config);
    }
}