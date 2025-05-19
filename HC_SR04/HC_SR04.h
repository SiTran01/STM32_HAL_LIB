#ifndef __HC_SR04_H__
#define __HC_SR04_H__

#include "stm32f1xx_hal.h"  // Thay d?i theo chip b?n dùng

typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t channel; // Ví d?: TIM_CHANNEL_1
    GPIO_TypeDef *TRIG_Port;
    uint16_t TRIG_Pin;

    uint32_t ic_rising;
    uint32_t ic_falling;
    uint8_t is_first_captured;
    uint8_t done;
} HCSR04_t;

void HCSR04_Init(HCSR04_t *sensor, TIM_HandleTypeDef *htim, uint32_t channel,
                 GPIO_TypeDef *TRIG_Port, uint16_t TRIG_Pin);

void HCSR04_Trigger(HCSR04_t *sensor);
void HCSR04_TIM_IC_CaptureCallback(HCSR04_t *sensor);
float HCSR04_ReadDistance(HCSR04_t *sensor); // don v?: cm

#endif
