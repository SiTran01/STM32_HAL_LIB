/**
 * @file LCD162_I2C.h
 * @brief Library for interfacing with 16x2 HD44780-compatible LCD via I2C.
 * 
 * Author: Si_Tran  
 * Date: 05/16/2025
 * 
 * This file provides function declarations for controlling 16x2 character LCD displays
 * over I2C using a PCF8574 I/O expander.
 * 
 * Designed for STM32 and other microcontrollers using HAL I2C interface.
 * Reduces GPIO usage by shifting parallel control to I2C.
 * 
 * Functions include initialization, string and character output, cursor control,
 * and backlight handling.
 */



#ifndef INC_LCD_I2C_H_
#define INC_LCD_I2C_H_

#include "stm32f1xx_hal.h"  
#include <string.h>
#include <stdio.h>

// === DEFINE ===
#define LCD_I2C_ADDR 0x27 << 1  
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define LCD_ENABLE 0x04
#define LCD_READWRITE 0x02
#define LCD_REGISTER_SELECT 0x01

// === API ===
void lcd_init(I2C_HandleTypeDef *hi2c);
void lcd_send_cmd(char cmd);
void lcd_send_data(char data);
void lcd_send_string(char *str);
void lcd_put_cur(uint8_t row, uint8_t col);
void lcd_clear(void);
void lcd_backlight_on(void);
void lcd_backlight_off(void);

#endif /* INC_LCD_I2C_H_ */
