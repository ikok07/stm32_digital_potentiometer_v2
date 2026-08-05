//
// Created by Kok on 7/17/25.
//

#include "app_state.h"

#include "stm32l0xx_hal.h"

// TODO: Encoder interrupts MUST be with higher NVIC priority than the FreeRTOS's max priority

void EXTI0_1_IRQHandler() {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

void EXTI4_15_IRQHandler() {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
}

void TIM2_IRQHandler() {
    HAL_TIM_IRQHandler(&gAppState.htim2);
}

void TIM21_IRQHandler() {
    HAL_IncTick();
    HAL_TIM_IRQHandler(&gAppState.htim21);
}

void TIM22_IRQHandler() {
    HAL_TIM_IRQHandler(&gAppState.htim22);
}