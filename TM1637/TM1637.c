/**
 * @file TM1637.c
 * @brief Driver for TM1637 4-digit 7-segment LED display.
 * 
 * Author: Si_Tran
 * Date: 14/05/2025
 * 
 * This file provides functions to initialize and control a TM1637-based display.
 * It supports decimal number display, individual digit updates, brightness control,
 * colon (dot) toggling, and display clearing.
 * 
 * Notes:
 * * TM1637 requires specific timing between CLK and DIO signals; delays are implemented via NOPs.
 * * Ensure proper GPIO configuration before calling any TM1637 functions.
 * * The display supports digits 0–9 and some characters (limited by segment map).
 
 ======================================================================================================
 
 */


#include "TM1637.h"

static const uint8_t digitToSegment[] = {
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07,
    0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71,
    0x3E, 0x38, 0x76, 0x40, 0x00
};

static void TM1637_Delay(void) {
    for (volatile int i = 0; i < 80; i++) {
        __NOP();
    }
}

static void TM1637_Start(TM1637_Handle* tm) {
    HAL_GPIO_WritePin(tm->CLK_Port, tm->CLK_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(tm->DIO_Port, tm->DIO_Pin, GPIO_PIN_SET);
    TM1637_Delay();
    HAL_GPIO_WritePin(tm->DIO_Port, tm->DIO_Pin, GPIO_PIN_RESET);
    TM1637_Delay();
    HAL_GPIO_WritePin(tm->CLK_Port, tm->CLK_Pin, GPIO_PIN_RESET);
}

static void TM1637_Stop(TM1637_Handle* tm) {
    HAL_GPIO_WritePin(tm->CLK_Port, tm->CLK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(tm->DIO_Port, tm->DIO_Pin, GPIO_PIN_RESET);
    TM1637_Delay();
    HAL_GPIO_WritePin(tm->CLK_Port, tm->CLK_Pin, GPIO_PIN_SET);
    TM1637_Delay();
    HAL_GPIO_WritePin(tm->DIO_Port, tm->DIO_Pin, GPIO_PIN_SET);
}

static void TM1637_WriteByte(TM1637_Handle* tm, uint8_t b) {
    for (int i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(tm->CLK_Port, tm->CLK_Pin, GPIO_PIN_RESET);
        TM1637_Delay();
        HAL_GPIO_WritePin(tm->DIO_Port, tm->DIO_Pin, (b & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        b >>= 1;
        TM1637_Delay();
        HAL_GPIO_WritePin(tm->CLK_Port, tm->CLK_Pin, GPIO_PIN_SET);
        TM1637_Delay();
    }

    HAL_GPIO_WritePin(tm->CLK_Port, tm->CLK_Pin, GPIO_PIN_RESET);
    TM1637_Delay();
    HAL_GPIO_WritePin(tm->DIO_Port, tm->DIO_Pin, GPIO_PIN_SET);
    TM1637_Delay();
    HAL_GPIO_WritePin(tm->CLK_Port, tm->CLK_Pin, GPIO_PIN_SET);
    TM1637_Delay();
    HAL_GPIO_WritePin(tm->CLK_Port, tm->CLK_Pin, GPIO_PIN_RESET);
}

void TM1637_Init(TM1637_Handle* tm, GPIO_TypeDef* clk_port, uint16_t clk_pin,
                 GPIO_TypeDef* dio_port, uint16_t dio_pin) {
    tm->CLK_Port = clk_port;
    tm->CLK_Pin = clk_pin;
    tm->DIO_Port = dio_port;
    tm->DIO_Pin = dio_pin;
    tm->colonOn = false;
    TM1637_Clear(tm);
}

void TM1637_SetBrightness(TM1637_Handle* tm, uint8_t level) {
    if (level > 7) level = 7;
    TM1637_Start(tm);
    TM1637_WriteByte(tm, 0x88 | level);
    TM1637_Stop(tm);
}

void TM1637_DisplayDecimal(TM1637_Handle* tm, int16_t num) {
    uint8_t digits[4];
    bool negative = false;

    if (num < 0) {
        negative = true;
        num = -num;
    }

    if (num > (negative ? 999 : 9999)) {
        num = negative ? 999 : 9999;
    }

    for (int i = 3; i >= 0; i--) {
        digits[i] = digitToSegment[num % 10];
        num /= 10;
    }

    if (negative) {
        digits[0] = 0x40;
    }

    if (tm->colonOn) digits[1] |= 0x80;

    TM1637_Start(tm);
    TM1637_WriteByte(tm, 0x40);
    TM1637_Stop(tm);

    TM1637_Start(tm);
    TM1637_WriteByte(tm, 0xC0);
    for (int i = 0; i < 4; i++) {
        TM1637_WriteByte(tm, digits[i]);
    }
    TM1637_Stop(tm);
}

void TM1637_Clear(TM1637_Handle* tm) {
    TM1637_Start(tm);
    TM1637_WriteByte(tm, 0x40);
    TM1637_Stop(tm);

    TM1637_Start(tm);
    TM1637_WriteByte(tm, 0xC0);
    for (int i = 0; i < 4; i++) {
        TM1637_WriteByte(tm, 0x00);
    }
    TM1637_Stop(tm);
}

void TM1637_DisplayDigit(TM1637_Handle* tm, uint8_t digit, uint8_t position) {
    if (position > 3) return;

    uint8_t seg = (digit < 21) ? digitToSegment[digit] : 0x00;
    if (tm->colonOn && position == 1) seg |= 0x80;

    TM1637_Start(tm);
    TM1637_WriteByte(tm, 0x44);
    TM1637_Stop(tm);

    TM1637_Start(tm);
    TM1637_WriteByte(tm, 0xC0 | position);
    TM1637_WriteByte(tm, seg);
    TM1637_Stop(tm);
}

void TM1637_Point(TM1637_Handle* tm, bool state) {
    tm->colonOn = state;
}


/**
 * End of TM1637.c
 * 
 * ============================
 * Usage Guide and API Summary
 * ============================
 * 
 * 1. TM1637_Init(...)
 *    - Initializes the TM1637_Handle structure and clears the display.
 *    - Must be called before using other display functions.
 * 
 * 2. TM1637_SetBrightness(level)
 *    - Sets the display brightness (range: 0–7).
 * 
 * 3. TM1637_DisplayDecimal(num)
 *    - Displays a signed integer on the 4-digit display.
 *    - Automatically limits values to -999 to 9999.
 * 
 * 4. TM1637_DisplayDigit(digit, position)
 *    - Displays a single digit at the specified position (0–3).
 *    - Useful for custom digit-level updates.
 * 
 * 5. TM1637_Point(state)
 *    - Turns the colon (:) on or off between digits 2 and 3.
 * 
 * 6. TM1637_Clear()
 *    - Clears the display (all digits off).
 * 
 * =====================================
 * Notes:
 * - Always call TM1637_Init() before using other functions.
 * - You can create advanced effects using TM1637_WriteByte().
 */
