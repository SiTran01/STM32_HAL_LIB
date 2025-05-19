#include "HC_SR04.h"

void HCSR04_Init(HCSR04_t *sensor, TIM_HandleTypeDef *htim, uint32_t channel,
                 GPIO_TypeDef *TRIG_Port, uint16_t TRIG_Pin)
{
    sensor->htim = htim;
    sensor->channel = channel;
    sensor->TRIG_Port = TRIG_Port;
    sensor->TRIG_Pin = TRIG_Pin;
    sensor->is_first_captured = 0;
    sensor->done = 0;

    HAL_TIM_IC_Start_IT(sensor->htim, sensor->channel);
}

void HCSR04_Trigger(HCSR04_t *sensor)
{
    HAL_GPIO_WritePin(sensor->TRIG_Port, sensor->TRIG_Pin, GPIO_PIN_SET);
    HAL_Delay(1); // Delay 1ms d? d?m b?o pin ?n d?nh
    HAL_GPIO_WritePin(sensor->TRIG_Port, sensor->TRIG_Pin, GPIO_PIN_RESET);
}

void HCSR04_TIM_IC_CaptureCallback(HCSR04_t *sensor)
{
    if (sensor->is_first_captured == 0)
    {
        sensor->ic_rising = HAL_TIM_ReadCapturedValue(sensor->htim, sensor->channel);
        __HAL_TIM_SET_CAPTUREPOLARITY(sensor->htim, sensor->channel, TIM_INPUTCHANNELPOLARITY_FALLING);
        sensor->is_first_captured = 1;
    }
    else
    {
        sensor->ic_falling = HAL_TIM_ReadCapturedValue(sensor->htim, sensor->channel);
        __HAL_TIM_SET_CAPTUREPOLARITY(sensor->htim, sensor->channel, TIM_INPUTCHANNELPOLARITY_RISING);
        sensor->is_first_captured = 0;
        sensor->done = 1;
    }
}

float HCSR04_ReadDistance(HCSR04_t *sensor)
{
    if (!sensor->done) return -1;

    uint32_t diff;
    if (sensor->ic_falling >= sensor->ic_rising)
        diff = sensor->ic_falling - sensor->ic_rising;
    else
        diff = (0xFFFF - sensor->ic_rising + sensor->ic_falling);

    float time_us = (diff * 1.0f) / (HAL_RCC_GetPCLK1Freq() / (sensor->htim->Init.Prescaler + 1)) * 1e6;
    float distance_cm = time_us / 58.0f;

    sensor->done = 0;
    return distance_cm;
}
