/**
 * @file lcd162.h
 * @brief Library for interfacing with 16x2 HD44780-compatible LCD displays.
 * 
 * Author: Si_Tran  
 * Date: 05/14/2025
 * 
 * This file provides function declarations for initializing and controlling
 * 16x2 character LCD modules using either 4-bit or 8-bit parallel interface.
 * 
 * Designed for use with STM32 or other microcontrollers.
 * Supports GPIO-based communication with configurable data and control pins.
 */

 

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include "stm32f1xx_hal.h"  // Ho?c ch?nh l?i t�y theo d�ng STM32 b?n dang d�ng
#include <stdint.h>

// ==== �?nh nghia ch�n k?t n?i (c?n d?nh nghia ? main ho?c file c?u h�nh) ====
#define GPIO_PORT     GPIOB
#define RS_Pin        GPIO_PIN_0
#define E_Pin         GPIO_PIN_1
#define DATA1_Pin     GPIO_PIN_10
#define DATA2_Pin     GPIO_PIN_3
#define DATA3_Pin     GPIO_PIN_4
#define DATA4_Pin     GPIO_PIN_5
#define DATA5_Pin     GPIO_PIN_6
#define DATA6_Pin     GPIO_PIN_7
#define DATA7_Pin     GPIO_PIN_8
#define DATA8_Pin     GPIO_PIN_9

// ==== L?a ch?n mode ====
#define LCD8Bit  // B? comment n?u b?n d�ng 8-bit mode

// ==== C�c l?nh LCD ====
#define LCD_CLEARDISPLAY   0x01
#define LCD_RETURNHOME     0x02
#define LCD_ENTRYMODESET   0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT    0x10
#define LCD_FUNCTIONSET    0x20
#define LCD_SETCGRAMADDR   0x40
#define LCD_SETDDRAMADDR   0x80

// Entry mode flags
#define LCD_ENTRYRIGHT          0x00
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// Display control flags
#define LCD_DISPLAYON  0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON   0x02
#define LCD_CURSOROFF  0x00
#define LCD_BLINKON    0x01
#define LCD_BLINKOFF   0x00

// Function set flags
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE    0x08
#define LCD_1LINE    0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS  0x00

// ==== Ki?u d? li?u ====
typedef uint8_t lcdDispSetting_t;

// ==== Prototype c�c h�m ====
void initLCD(void);
void clearLCD(void);
void clearDisp(void);
void putLCD(char c);
void writeLCD(char *str);
void setCursor(char x, char y);
void cursorOn(void);
void blinkOn(void);
void setDisplay(lcdDispSetting_t dispSetting);

#endif /* INC_LCD_H_ */
