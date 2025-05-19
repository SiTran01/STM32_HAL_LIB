/******************************************************************************
 * @file LCD162_I2C.c
 * @brief Driver for 16x2 HD44780-compatible LCD using I2C interface (PCF8574).
 * 
 * This driver allows communication with character LCDs via I2C using a PCF8574 I/O expander.
 * It simplifies wiring and reduces required GPIOs by using I2C protocol.
 * 
 * Functions:
 * - lcd_init: Initializes the LCD through the I2C interface.
 * - lcd_send_cmd: Sends a command to the LCD.
 * - lcd_send_data: Sends a data byte (character) to the LCD.
 * - lcd_send_string: Displays a null-terminated string on the LCD.
 * - lcd_put_cur: Sets the cursor position (column, row).
 * - lcd_clear: Clears the display.
 * - lcd_backlight_on: Turns on the LCD backlight.
 * - lcd_backlight_off: Turns off the LCD backlight.
 * 
 * Usage:
 * - Call lcd_init() once, passing a valid I2C handle.
 * - Use lcd_send_string(), lcd_put_cur(), etc., to interact with the display.
 * 
 * Notes:
 * - Ensure the correct I2C address is defined (LCD_I2C_ADDR).
 * - Uses HAL I2C functions (HAL_I2C_Master_Transmit).
 * - Includes basic delays using HAL_Delay to meet timing requirements.
 * 
 ******************************************************************************/



#include "LCD162_I2C.h"

// Pointer to the I2C handle provided by the user
static I2C_HandleTypeDef *i2cHandle;

// Backlight state, default is ON
static uint8_t backlight = LCD_BACKLIGHT;

/**
 * @brief Sends a byte to the LCD with control flags via I2C.
 * 
 * @param data 8-bit data to send
 * @param mode LCD command mode (0x00) or data mode (LCD_REGISTER_SELECT)
 */
static void lcd_send_internal(uint8_t data, uint8_t mode)
{
	uint8_t high = data & 0xF0;
	uint8_t low  = (data << 4) & 0xF0;
	uint8_t data_arr[4];

	data_arr[0] = high | mode | backlight | LCD_ENABLE;
	data_arr[1] = high | mode | backlight;
	data_arr[2] = low  | mode | backlight | LCD_ENABLE;
	data_arr[3] = low  | mode | backlight;

	HAL_I2C_Master_Transmit(i2cHandle, LCD_I2C_ADDR, data_arr, 4, HAL_MAX_DELAY);
	HAL_Delay(1);
}

/**
 * @brief Sends a command byte to the LCD.
 * 
 * @param cmd Command to send
 */
void lcd_send_cmd(char cmd)
{
	lcd_send_internal(cmd, 0x00);
}

/**
 * @brief Sends a single character to be displayed on the LCD.
 * 
 * @param data Character to send
 */
void lcd_send_data(char data)
{
	lcd_send_internal(data, LCD_REGISTER_SELECT);
}

/**
 * @brief Initializes the LCD in 4-bit I2C mode.
 * 
 * @param hi2c Pointer to HAL I2C handle
 */
void lcd_init(I2C_HandleTypeDef *hi2c)
{
	i2cHandle = hi2c;

	HAL_Delay(50); // Wait for LCD to power up

	lcd_send_cmd(0x33); // Initialization sequence
	lcd_send_cmd(0x32);
	lcd_send_cmd(0x28); // Function set: 4-bit, 2 line, 5x8 dots
	lcd_send_cmd(0x0C); // Display ON, cursor OFF
	lcd_send_cmd(0x06); // Entry mode set: increment, no shift
	lcd_send_cmd(0x01); // Clear display
	HAL_Delay(5);
}

/**
 * @brief Sends a null-terminated string to the LCD.
 * 
 * @param str Pointer to string
 */
void lcd_send_string(char *str)
{
	while (*str) {
		lcd_send_data(*str++);
	}
}

/**
 * @brief Sets the cursor to a specific position on the LCD.
 * 
 * @param row Row number (0 or 1)
 * @param col Column number (0 to 15)
 */
void lcd_put_cur(uint8_t row, uint8_t col)
{
	uint8_t pos = (row == 0) ? 0x80 + col : 0xC0 + col;
	lcd_send_cmd(pos);
}

/**
 * @brief Clears the LCD display and resets the cursor position.
 */
void lcd_clear(void)
{
	lcd_send_cmd(0x01);
	HAL_Delay(2);
}

/**
 * @brief Turns the LCD backlight on.
 */
void lcd_backlight_on(void)
{
	backlight = LCD_BACKLIGHT;
}

/**
 * @brief Turns the LCD backlight off.
 */
void lcd_backlight_off(void)
{
	backlight = LCD_NOBACKLIGHT;
}

/* End of file */
