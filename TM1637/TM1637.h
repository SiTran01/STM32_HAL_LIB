/**
 * @file TM1637.h
 * @brief Library for controlling a TM1637-based 4-digit 7-segment display.
 * 
 * Author: Si_Tran  
 * Date: 05/14/2025
 * 
 * This library provides functions to initialize and control a TM1637 4-digit display,
 * including brightness adjustment, number display, individual digit control, and colon toggling.
 * 
 * Designed for STM32 using HAL drivers.
 */



#ifndef TM1637_H
#define TM1637_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    GPIO_TypeDef* CLK_Port;
    uint16_t CLK_Pin;
    GPIO_TypeDef* DIO_Port;
    uint16_t DIO_Pin;
    bool colonOn;
} TM1637_Handle;

void TM1637_Init(TM1637_Handle* tm, GPIO_TypeDef* clk_port, uint16_t clk_pin,
                 GPIO_TypeDef* dio_port, uint16_t dio_pin);
void TM1637_SetBrightness(TM1637_Handle* tm, uint8_t level);
void TM1637_DisplayDecimal(TM1637_Handle* tm, int16_t num);
void TM1637_Clear(TM1637_Handle* tm);
void TM1637_DisplayDigit(TM1637_Handle* tm, uint8_t digit, uint8_t position);
void TM1637_Point(TM1637_Handle* tm, bool state);

#endif
