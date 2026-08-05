//
// Created by Kok on 4/7/26.
//

#include "dac.h"

#include "FreeRTOS.h"
#include "task.h"
#include "app_state.h"
#include "error.h"
#include "stm32l0xx_hal.h"
#include "tasks_common.h"

/* --- Tasks --- */
void dac_a_task(void *arg);
void dac_b_task(void *arg);
void dac_chan_ctrl_task(void *arg);

/* --- Init Methods --- */
void chan_ctrl_init();

/* --- Callback Methods --- */
uint8_t i2c_send(uint8_t DeviceAddress, uint8_t RegisterAddress, uint16_t Data);
uint8_t i2c_read(uint8_t DeviceAddress, uint8_t RegisterAddress, uint16_t *RegContents);
void dac_err_log(DACx050x_ErrorTypeDef DacError, uint8_t SpecificErrorCode);

uint8_t DAC_Init() {
    HAL_StatusTypeDef hal_err = HAL_OK;
    DACx050x_ErrorTypeDef dac_err = DAC_X050X_ERROR_OK;

    // Setup check if device is ready
    if ((hal_err = HAL_I2C_IsDeviceReady(&gAppState.hi2c, DAC_I2C_ADDRESS, DAC_I2C_CONN_TRIALS, DAC_I2C_CONN_TIMEOUT)) != HAL_OK) {
        return 1;
    }

    // Setup channel control pins
    chan_ctrl_init();

    // Read channel control pins
    GPIO_PinState ch1_en = HAL_GPIO_ReadPin(DAC_CHCTRL_PORT, DAC_CHCTRL_1_PIN);
    GPIO_PinState ch2_en = HAL_GPIO_ReadPin(DAC_CHCTRL_PORT, DAC_CHCTRL_2_PIN);

    // Setup for max voltage - VREF
    gAppState.hdac = (DACx050x_HandleTypeDef){
        .Config = {
            .DeviceI2CAddress = DAC_I2C_ADDRESS,
            .DacAEnabled = ch1_en,
            .DacBEnabled = ch2_en,
            .BufferAGain = DAC_X050X_GAIN_2,
            .BufferBGain = DAC_X050X_GAIN_2,
            .RefDivider = DAC_X050X_REF_DIV_2,
            .DacABroadcastEnabled = 0,
            .DacBBroadcastEnabled = 0,
            .DacASyncEnabled = 0,
            .DacBSyncEnabled = 0,
            .InternalReferenceEnabled = 0,
        },
        .Callbacks = {
            .I2CSend = i2c_send,
            .I2CRead = i2c_read,
            .LogError = dac_err_log
        }
    };

    // Setup DAC
    if ((dac_err = DACx050x_Init(&gAppState.hdac))) {
        return 1;
    }

    // Setup shared dac value variables
    SHVAL_ConfigTypeDef SHVAL_Config = {
        .InitialValue = 0,
        .SubscribersQueueSize = DAC_VALUE_SHVAL_QUEUE_SIZE
    };
    gAppState.SharedValues.DacAValue = SHVAL_Init(&SHVAL_Config);
    gAppState.SharedValues.DacBValue = SHVAL_Init(&SHVAL_Config);

    return 0;
}

void DAC_StartTasks() {
    // Each DAC channel is operated by a separate task
    gAppState.Tasks.DacATask = (SCHEDULER_TaskTypeDef){
        .Active = 0,
        .Name = "DAC A Task",
        .Priority = DAC_CH_A_TASK_PRIORITY,
        .StackDepth = DAC_CH_A_TASK_STACK_DEPTH,
        .Args = NULL,
        .Function = dac_a_task
    };

    gAppState.Tasks.DacBTask = (SCHEDULER_TaskTypeDef){
        .Active = 0,
        .Name = "DAC B Task",
        .Priority = DAC_CH_B_TASK_PRIORITY,
        .StackDepth = DAC_CH_B_TASK_STACK_DEPTH,
        .Args = NULL,
        .Function = dac_b_task
    };

    // Channels control task
    gAppState.Tasks.DacChanCtrlTask = (SCHEDULER_TaskTypeDef){
        .Active = 0,
        .Name = "DAC CHCTRL Task",
        .Priority = DAC_CHAN_CTRL_TASK_PRIORITY,
        .StackDepth = DAC_CHAN_CTRL_TASK_STACK_DEPTH,
        .Args = NULL,
        .Function = dac_chan_ctrl_task
    };

    SCHEDULER_Create(&gAppState.Tasks.DacATask);
    SCHEDULER_Create(&gAppState.Tasks.DacBTask);
    SCHEDULER_Create(&gAppState.Tasks.DacChanCtrlTask);

    // Sync currently active channels
    DAC_HandleChanCtrlEvent();
}

void DAC_HandleChanCtrlEvent() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xTaskNotifyFromISR(gAppState.Tasks.DacChanCtrlTask.OsTask, 0, eNoAction, &xHigherPriorityTaskWoken);

    /* Request a context switch if a higher priority task was woken */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void dac_a_task(void *ptr) {
    uint32_t value;
    while (1) {
        if (xQueueReceive(gAppState.SharedValues.DacAValue.SubscribersQueue, &value, portMAX_DELAY)) {
            // Error is handled in the logging callback
            DACx50x_WriteData(&gAppState.hdac, DAC_X050X_OUTPUTA, value);
        }
    }
}

void dac_b_task(void *ptr) {
    uint32_t value;
    while (1) {
        if (xQueueReceive(gAppState.SharedValues.DacBValue.SubscribersQueue, &value, portMAX_DELAY)) {
            // Error is handled in the logging callback
            DACx50x_WriteData(&gAppState.hdac, DAC_X050X_OUTPUTB, value);
        }
    }
}

void dac_chan_ctrl_task(void *arg) {
    GPIO_PinState ch1_en, ch2_en;

    while (1) {
        if (xTaskNotifyWait(0x00, 0xFF, NULL, portMAX_DELAY)) {
            ch1_en = HAL_GPIO_ReadPin(DAC_CHCTRL_PORT, DAC_CHCTRL_1_PIN);
            ch2_en = HAL_GPIO_ReadPin(DAC_CHCTRL_PORT, DAC_CHCTRL_2_PIN);

            // Errors are handled in the logging callback
            if (DACx050x_OutputControl(&gAppState.hdac, DAC_X050X_OUTPUTA, ch1_en) != DAC_X050X_ERROR_OK) {
                continue;
            };

            if (DACx050x_OutputControl(&gAppState.hdac, DAC_X050X_OUTPUTB, ch2_en) != DAC_X050X_ERROR_OK) {
                continue;
            };
        }
    }
}

void chan_ctrl_init() {
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_Config = {
        .Mode = GPIO_MODE_IT_RISING_FALLING,
        .Pin = DAC_CHCTRL_1_PIN,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_LOW
    };
    HAL_GPIO_Init(DAC_CHCTRL_PORT, &GPIO_Config);

    GPIO_Config.Pin = DAC_CHCTRL_2_PIN;
    HAL_GPIO_Init(DAC_CHCTRL_PORT, &GPIO_Config);

    HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
}

uint8_t i2c_send(uint8_t DeviceAddress, uint8_t RegisterAddress, uint16_t Data) {
    uint8_t buffer[3];
    buffer[0] = RegisterAddress;
    buffer[1] = (Data >> 8) & 0xFF;
    buffer[2] = Data & 0xFF;

    return HAL_I2C_Master_Transmit(&gAppState.hi2c, DeviceAddress, buffer, sizeof(buffer) / sizeof(buffer[0]), 1000);
}

uint8_t i2c_read(uint8_t DeviceAddress, uint8_t RegisterAddress, uint16_t *RegContents) {
    HAL_StatusTypeDef hal_err;
    if ((hal_err = HAL_I2C_Master_Transmit(&gAppState.hi2c, DeviceAddress, &RegisterAddress, 1, 1000)) != HAL_OK) {
        return hal_err;
    }

    uint8_t buffer[2];
    if ((hal_err = HAL_I2C_Master_Receive(&gAppState.hi2c, DeviceAddress, buffer, 2, 1000)) != HAL_OK) {
        return hal_err;
    };

    *RegContents = ((uint16_t)buffer[0] << 8) | buffer[1];

    return hal_err;
}

void dac_err_log(DACx050x_ErrorTypeDef DacError, uint8_t SpecificErrorCode) {
    // Any error would be fatal to the application
    ERROR_TriggerFatal();
}
