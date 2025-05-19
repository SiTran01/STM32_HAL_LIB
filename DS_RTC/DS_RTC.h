#ifndef __DS_RTC_H
#define __DS_RTC_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// H? tr? các lo?i chip
typedef enum {
    DS_RTC_DS1307,
    DS_RTC_DS1337,
    DS_RTC_DS1338,
    DS_RTC_DS1339,  // NEW
    DS_RTC_DS1340,
    DS_RTC_DS1341,  // NEW
    DS_RTC_DS1342,  // NEW
    DS_RTC_DS1388,  // NEW
    DS_RTC_DS3231,
    DS_RTC_DS3232
} DS_RTC_Model;

// C?u trúc th?i gian
typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day_of_week;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} DS_RTC_Time;

// C?u trúc báo th?c don gi?n
typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    bool match_seconds;
    bool match_minutes;
    bool match_hours;
    bool match_day;
} DS_RTC_Alarm;

typedef struct {
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;             // ngày ho?c th?
    bool match_minutes;
    bool match_hours;
    bool match_day;
} DS_RTC_Alarm2;


typedef struct {
    I2C_HandleTypeDef *hi2c;
    DS_RTC_Model chip;
    uint8_t i2c_addr;
    bool has_alarm;
    bool has_temp;
		bool has_sram;
		bool has_eeprom;
} DS_RTC_HandleTypeDef;

// API
void DS_RTC_Init(DS_RTC_HandleTypeDef *rtc, I2C_HandleTypeDef *hi2c, DS_RTC_Model chip);
HAL_StatusTypeDef DS_RTC_ReadTime(DS_RTC_HandleTypeDef *rtc, DS_RTC_Time *time);
HAL_StatusTypeDef DS_RTC_WriteTime(DS_RTC_HandleTypeDef *rtc, const DS_RTC_Time *time);
HAL_StatusTypeDef DS_RTC_SetAlarm(DS_RTC_HandleTypeDef *rtc, const DS_RTC_Alarm *alarm);
HAL_StatusTypeDef DS_RTC_SetAlarm2(DS_RTC_HandleTypeDef *rtc, const DS_RTC_Alarm2 *alarm);
HAL_StatusTypeDef DS_RTC_ClearAlarmFlag(DS_RTC_HandleTypeDef *rtc);
HAL_StatusTypeDef DS_RTC_GetTemperature(DS_RTC_HandleTypeDef *rtc, float *temperature);

// Internal
uint8_t DS_RTC_ToBCD(uint8_t val);
uint8_t DS_RTC_FromBCD(uint8_t bcd);

#endif
