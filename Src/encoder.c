//
// Created by Kok on 4/9/26.
//

#include "encoder.h"

#include "app_state.h"
#include "error.h"
#include "tasks_common.h"

#define ENCODER_A_FILTER                0xF             // 0x0 - 0xF
#define ENCODER_B_FILTER                0xF             // 0x0 - 0xF

#define ENCODER_MAX_VALUE               (0xFFFF * 4)     // Each step counts as 4 incs/decs
#define ENCODER_MIN_VALUE               0
#define ENCODER_TIM_PERIOD              0xFFFF          // Max 16-bit value

#define ENCODER_UPDATE_DAC_TIMEOUT      1000

void enc1_task(void *arg);
void enc2_task(void *arg);

void handle_new_enc_value(TIM_HandleTypeDef *htim, SHVAL_HandleTypeDef *hshval, uint16_t *PrevCounter, int32_t *EncValue);
void setup_reset_buttons();

HAL_StatusTypeDef ENCODER_Init() {
    HAL_StatusTypeDef hal_err = HAL_OK;

    // Setup reset GPIOs
    setup_reset_buttons();

    // Setup timers
    TIM_HandleTypeDef TIM_Handle = {
        .Instance = TIM2,
        .Init = {
            .AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE,
            .ClockDivision = TIM_CLOCKDIVISION_DIV1,
            .CounterMode = TIM_COUNTERMODE_UP,
            .Prescaler = 0x00,
            .Period = ENCODER_TIM_PERIOD - 1
        }
    };
    gAppState.htim2 = TIM_Handle;

    TIM_Encoder_InitTypeDef TIM_Config = {
        .EncoderMode = TIM_ENCODERMODE_TI12,
        .IC1Polarity = TIM_ICPOLARITY_RISING,
        .IC1Selection = TIM_ICSELECTION_DIRECTTI,
        .IC1Prescaler = TIM_ICPSC_DIV1,
        .IC1Filter = ENCODER_A_FILTER,
        .IC2Polarity = TIM_ICPOLARITY_RISING,
        .IC2Selection = TIM_ICSELECTION_DIRECTTI,
        .IC2Prescaler = TIM_ICPSC_DIV1,
        .IC2Filter = ENCODER_B_FILTER
    };

    if ((hal_err = HAL_TIM_Encoder_Init(&gAppState.htim2, &TIM_Config))) {
        return hal_err;
    }

    TIM_Handle.Instance = TIM22;
    gAppState.htim22 = TIM_Handle;

    if ((hal_err = HAL_TIM_Encoder_Init(&gAppState.htim22, &TIM_Config))) {
        return hal_err;
    }

    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    HAL_NVIC_EnableIRQ(TIM22_IRQn);

    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
    HAL_NVIC_SetPriority(TIM22_IRQn, 5, 0);

    if ((hal_err = HAL_TIM_Encoder_Start_IT(&gAppState.htim2, TIM_CHANNEL_ALL)) != HAL_OK) {
        return hal_err;
    };

    if ((hal_err = HAL_TIM_Encoder_Start_IT(&gAppState.htim22, TIM_CHANNEL_ALL)) != HAL_OK) {
        return hal_err;
    };

    return hal_err;
}

void ENCODER_StartTasks() {
    gAppState.Tasks.Enc1Task = (SCHEDULER_TaskTypeDef){
        .Active = 0,
        .Name = "ENC 1 Task",
        .Priority = ENC_1_TASK_PRIORITY,
        .StackDepth = ENC_1_TASK_STACK_DEPTH,
        .Function = enc1_task,
        .Args = NULL
    };

    gAppState.Tasks.Enc2Task = (SCHEDULER_TaskTypeDef){
        .Active = 0,
        .Name = "ENC 2 Task",
        .Priority = ENC_2_TASK_PRIORITY,
        .StackDepth = ENC_2_TASK_STACK_DEPTH,
        .Function = enc2_task,
        .Args = NULL
    };

    SCHEDULER_Create(&gAppState.Tasks.Enc1Task);
    SCHEDULER_Create(&gAppState.Tasks.Enc2Task);
}

void enc1_task(void *arg) {
    uint16_t prevCounter = __HAL_TIM_GET_COUNTER(&gAppState.htim2);
    int32_t encValue = 0;
    uint32_t notifyValue = 0;

    while (1) {
        if (xTaskNotifyWait(0x00, 0xFFFFFFFF, &notifyValue, portMAX_DELAY)) {
            if (notifyValue & ENCODER_NOTIFY_RESET) {
                __HAL_TIM_SET_COUNTER(&gAppState.htim2, 0);
                prevCounter = 0;
                encValue = 0;
            }

            handle_new_enc_value(&gAppState.htim2, &gAppState.SharedValues.DacAValue, &prevCounter, &encValue);
        }
    }
}

void enc2_task(void *arg) {
    uint16_t prevCounter = __HAL_TIM_GET_COUNTER(&gAppState.htim22);
    int32_t encValue = 0;
    uint32_t notifyValue = 0;

    while (1) {
        if (xTaskNotifyWait(0x00, 0xFFFFFFFF, &notifyValue, portMAX_DELAY)) {
            if (notifyValue & ENCODER_NOTIFY_RESET) {
                __HAL_TIM_SET_COUNTER(&gAppState.htim22, 0);
                prevCounter = 0;
                encValue = 0;
            }

            handle_new_enc_value(&gAppState.htim22, &gAppState.SharedValues.DacBValue, &prevCounter, &encValue);
        }
    }
}

void handle_new_enc_value(TIM_HandleTypeDef *htim, SHVAL_HandleTypeDef *hshval, uint16_t *PrevCounter, int32_t *EncValue) {
    uint16_t counter = __HAL_TIM_GET_COUNTER(htim);
    int16_t diff = (int16_t)(counter - *PrevCounter);

    if (*EncValue + diff < ENCODER_MIN_VALUE || *EncValue + diff > ENCODER_MAX_VALUE) {
        *PrevCounter = counter;
        return;
    }

    *EncValue += diff;
    *PrevCounter = counter;

    SHVAL_ErrorTypeDef shval_err = SHVAL_ERROR_OK;
    // encValue / 4 => one encoder step is four pulses
    if ((shval_err = SHVAL_SetValue(hshval, *EncValue / 4, ENCODER_UPDATE_DAC_TIMEOUT)) != SHVAL_ERROR_OK) {
        ERROR_Trigger();
    };
}

void setup_reset_buttons() {
    GPIO_InitTypeDef GPIO_Config = {
        .Mode = GPIO_MODE_IT_RISING,
        .Pin = ENCODER_1_RESET_PIN,
        .Pull = GPIO_PULLDOWN,
        .Speed = GPIO_SPEED_LOW
    };

    HAL_GPIO_Init(ENCODER_RESET_PORT, &GPIO_Config);

    GPIO_Config.Pin = ENCODER_2_RESET_PIN;
    HAL_GPIO_Init(ENCODER_RESET_PORT, &GPIO_Config);

    HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
    HAL_NVIC_SetPriority(EXTI4_15_IRQn, 5, 0);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        xTaskNotify(gAppState.Tasks.Enc1Task.OsTask, 0, eNoAction);
    } else if (htim->Instance == TIM22) {
        xTaskNotify(gAppState.Tasks.Enc2Task.OsTask, 0, eNoAction);
    }
}