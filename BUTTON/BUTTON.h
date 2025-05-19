/**
 * @file button.h
 * @brief Button handling library.
 * 
 * Author: Si_Tran
 * Date: 06/05/2025
 * 
 * This file provides functions for initializing, managing, and updating button states.
 * Supports debounce, toggle, and hold modes. 
 */



#ifndef __BUTTON_H
#define __BUTTON_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

#define BUTTON_MAX 10

typedef enum {
    BUTTON_Mode_Toggle = 0,
    BUTTON_Mode_Hold
} ButtonMode_t;

typedef enum {
    BUTTON_PressType_OnPressed = 0,
    BUTTON_PressType_Normal,
    BUTTON_PressType_Long,
    BUTTON_PressType_VeryLong,
    BUTTON_PressType_Double,
    BUTTON_PressType_Repeat,
    BUTTON_PressType_RepeatOnce
} ButtonPressType_t;

typedef enum {
    BUTTON_STATE_START = 0,
    BUTTON_STATE_DEBOUNCE,
    BUTTON_STATE_PRESSED
} ButtonState_t;

typedef struct Button_s {
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
    uint8_t ActiveState;

    ButtonState_t State;
    uint8_t LastStatus;

    ButtonMode_t Mode;

    uint32_t StartTime;
    uint32_t LastPressDuration;
    uint32_t FirstClickReleaseTime;
    uint8_t FirstClickDone;
    uint16_t DebounceTime;

    // Toggle mode
    uint16_t NormalTime;
    uint16_t LongTime;
    uint16_t VeryLongTime;
    uint16_t DoubleClickTime;

    // Hold mode
    uint16_t RepeatDelay;
    uint16_t RepeatInterval;
    uint32_t LastRepeatTime;
    uint8_t RepeatStarted;

    void (*Handler)(struct Button_s*, ButtonPressType_t);
} Button_t;

Button_t* BUTTON_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t ActiveState,
                      ButtonMode_t mode, void (*Handler)(Button_t*, ButtonPressType_t));
void BUTTON_Deinit(Button_t* btn);
void BUTTON_Update(void);


void Set_DebounceTime(Button_t* btn, uint8_t debounceTime);
void SetTime_Hold_mode(Button_t* btn, uint16_t delay, uint16_t interval);
void SetTime_Toggle_mode(Button_t* btn, uint16_t time4Double, uint16_t normal, uint16_t longer, uint16_t very_long);

__weak void BUTTON_Callback(Button_t* btn, ButtonPressType_t type);

#endif // __BUTTON_H
