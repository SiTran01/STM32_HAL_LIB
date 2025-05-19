/******************************************************************************
 * File: LCD162.c
 * Description: Driver for 16x2 character LCD using either 4-bit or 8-bit mode.
 *
 * This driver allows communication with HD44780-compatible LCDs using GPIO pins.
 * It supports both 4-bit and 8-bit data modes via conditional compilation.
 *
 * Functions:
 * - initLCD: Initializes the LCD in 4-bit or 8-bit mode.
 * - clearLCD: Clears the display content.
 * - setCursor: Sets the cursor position (column, row).
 * - writeLCD: Writes a string to the display.
 * - putLCD: Writes a single character to the display.
 * - cursorOn: Turns the cursor on.
 * - blinkOn: Enables cursor blinking.
 * - clearDisp: Turns off the display without clearing memory.
 * - setDisplay: Sets display, cursor, and blink settings.
 *
 * Usage:
 * - Ensure GPIO pins are correctly defined in LCD162.h.
 * - Call initLCD() once before any display operations.
 * - Use writeLCD(), setCursor(), etc., to control the LCD content.
 *
 * Notes:
 * - Timing is critical; HAL_Delay is used to meet hardware delay requirements.
 * - Define LCD8Bit to enable 8-bit communication, otherwise 4-bit mode is used.
 *
 ******************************************************************************/



#include "LCD162.h"

// Macro to simplify setting GPIO pin state based on condition
#define SET_IF(expr)  ((expr) ? GPIO_PIN_SET : GPIO_PIN_RESET)

// Global variable to store current LCD display settings
static char display_settings;

// ==== Generate a falling edge on the 'E' (Enable) pin ====
static void fallingEdge(void)
{
    HAL_GPIO_WritePin(GPIO_PORT, E_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIO_PORT, E_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIO_PORT, E_Pin, GPIO_PIN_RESET);
    HAL_Delay(1); // Short delay to ensure command/data is latched
}

#ifndef LCD8Bit
// ==== Send 4 bits of data to LCD (used in 4-bit mode) ====
static void send4Bits(char data)
{
    HAL_GPIO_WritePin(GPIO_PORT, DATA5_Pin, SET_IF(data & 0x01));
    HAL_GPIO_WritePin(GPIO_PORT, DATA6_Pin, SET_IF(data & 0x02));
    HAL_GPIO_WritePin(GPIO_PORT, DATA7_Pin, SET_IF(data & 0x04));
    HAL_GPIO_WritePin(GPIO_PORT, DATA8_Pin, SET_IF(data & 0x08));

    fallingEdge();
}
#endif

#ifdef LCD8Bit
// ==== Send full 8 bits of data to LCD (used in 8-bit mode) ====
static void send8Bits(char val)
{
    HAL_GPIO_WritePin(GPIO_PORT, DATA1_Pin, SET_IF(val & 0x01));
    HAL_GPIO_WritePin(GPIO_PORT, DATA2_Pin, SET_IF(val & 0x02));
    HAL_GPIO_WritePin(GPIO_PORT, DATA3_Pin, SET_IF(val & 0x04));
    HAL_GPIO_WritePin(GPIO_PORT, DATA4_Pin, SET_IF(val & 0x08));
    HAL_GPIO_WritePin(GPIO_PORT, DATA5_Pin, SET_IF(val & 0x10));
    HAL_GPIO_WritePin(GPIO_PORT, DATA6_Pin, SET_IF(val & 0x20));
    HAL_GPIO_WritePin(GPIO_PORT, DATA7_Pin, SET_IF(val & 0x40));
    HAL_GPIO_WritePin(GPIO_PORT, DATA8_Pin, SET_IF(val & 0x80));

    fallingEdge();
}
#endif

// ==== Send a command to LCD ====
static void sendCommand(char cmd)
{
#ifdef LCD8Bit
    HAL_GPIO_WritePin(GPIO_PORT, RS_Pin, GPIO_PIN_RESET); // Command mode
    send8Bits(cmd);
#else
    HAL_GPIO_WritePin(GPIO_PORT, RS_Pin, GPIO_PIN_RESET); // Command mode
    send4Bits(cmd >> 4);  // Send high nibble
    send4Bits(cmd);       // Send low nibble
#endif
}

// ==== Send data (a character) to LCD ====
static void sendData(char data)
{
#ifdef LCD8Bit
    HAL_GPIO_WritePin(GPIO_PORT, RS_Pin, GPIO_PIN_SET); // Data mode
    send8Bits(data);
#else
    HAL_GPIO_WritePin(GPIO_PORT, RS_Pin, GPIO_PIN_SET); // Data mode
    send4Bits(data >> 4);  // Send high nibble
    send4Bits(data);       // Send low nibble
#endif
}

// ==== Clear the LCD screen ====
void clearLCD(void)
{
    sendCommand(LCD_CLEARDISPLAY);
    HAL_Delay(5); // Wait for clear command to complete
}

// ==== Print a single character on LCD ====
void putLCD(char c)
{
    sendData(c);
}

// ==== Print a null-terminated string to LCD ====
void writeLCD(char *str)
{
    while (*str)
    {
        sendData(*str++);
    }
}

// ==== Initialize the LCD ====
void initLCD(void)
{
    HAL_GPIO_WritePin(GPIO_PORT, E_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIO_PORT, RS_Pin, GPIO_PIN_RESET);

    HAL_Delay(50); // Wait for LCD to power up

#ifdef LCD8Bit
    // Set LCD to 8-bit mode, 2 lines, 5x8 font
    display_settings = LCD_8BITMODE | LCD_2LINE | LCD_5x8DOTS;
    sendCommand(LCD_FUNCTIONSET | display_settings);
    HAL_Delay(5);
    sendCommand(LCD_FUNCTIONSET | display_settings);
    HAL_Delay(5);
    sendCommand(LCD_FUNCTIONSET | display_settings);
    HAL_Delay(5);
#else
    // Initialization sequence for 4-bit mode
    display_settings = LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS;
    send4Bits(0x03);
    HAL_Delay(5);
    send4Bits(0x03);
    HAL_Delay(5);
    send4Bits(0x03);
    HAL_Delay(2);
    send4Bits(0x02); // Set to 4-bit mode
    HAL_Delay(2);
#endif

    sendCommand(LCD_FUNCTIONSET | display_settings);

    // Turn on display, disable cursor and blinking
    display_settings = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    sendCommand(LCD_DISPLAYCONTROL | display_settings);
    HAL_Delay(2);

    clearLCD();

    // Set entry mode: cursor moves right, no display shift
    display_settings = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    sendCommand(LCD_ENTRYMODESET | display_settings);
    HAL_Delay(2);
}

// ==== Set cursor to position (x, y) ====
void setCursor(char x, char y)
{
    uint8_t base = (y == 1) ? 0x40 : 0x00;
    sendCommand(0x80 | (base + x));
}

// ==== Enable the cursor ====
void cursorOn(void)
{
    sendCommand(0x08 | 0x04 | 0x02); // Display on, cursor on
}

// ==== Enable blinking cursor ====
void blinkOn(void)
{
    sendCommand(0x08 | 0x04 | 0x01); // Display on, blink on
}

// ==== Turn off display (but content is retained) ====
void clearDisp(void)
{
    sendCommand(0x08 | 0x04 | 0x00); // Display on, cursor off, blink off
}

// ==== Set display configuration via custom enum ====
void setDisplay(lcdDispSetting_t dispSetting)
{
    sendCommand(0x08 | (dispSetting & 0x07));
}
