#ifndef DS_RTC_CORE_H
#define DS_RTC_CORE_H

#include "main.h"  // hoặc nơi chứa HAL và kiểu chuẩn
#include <stdbool.h>

typedef enum {
    DS_RTC_MODEL_DS1307,
    DS_RTC_MODEL_DS1337,
		DS_RTC_MODEL_DS1338,
		DS_RTC_MODEL_DS1340,
    DS_RTC_MODEL_DS3231,
		DS_RTC_MODEL_DS3232
    // thêm các model khác nếu cần
} DS_RTC_Model;

typedef enum {
    DS_RTC_SQW_OFF = 0xFF,       // Không xuất xung
    DS_RTC_SQW_1HZ = 0,          // RS2=0, RS1=0
    DS_RTC_SQW_1024HZ = 1,       // RS2=0, RS1=1
    DS_RTC_SQW_4096HZ = 2,       // RS2=1, RS1=0
    DS_RTC_SQW_8192HZ = 3,       // RS2=1, RS1=1
    DS_RTC_SQW_32768HZ = 4,      // Chỉ DS1307 (bit OUT = 1)
    DS_RTC_SQW_FIXED_1HZ = 5     // Chỉ dùng cho chip cố định tần số
} DS_RTC_SquareWaveFreq;




typedef struct {
    I2C_HandleTypeDef *hi2c;
    DS_RTC_Model model;
    uint8_t i2c_addr;

    bool has_alarm;
    bool has_temp;
    bool has_ram;
    bool has_eeprom;
} DS_RTC_HandleTypeDef;

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day_of_week;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} DS_RTC_Time;

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    bool match_seconds;
    bool match_minutes;
    bool match_hours;
    bool match_day;
} DS_RTC_Alarm1;

typedef struct {
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    bool match_minutes;
    bool match_hours;
    bool match_day;
} DS_RTC_Alarm2;

// Hàm tiện ích chuyển đổi BCD
uint8_t DS_RTC_ToBCD(uint8_t val);
uint8_t DS_RTC_FromBCD(uint8_t bcd);

// Khởi tạo RTC
void DS_RTC_Init(DS_RTC_HandleTypeDef *rtc, I2C_HandleTypeDef *hi2c, DS_RTC_Model model);

// Thời gian
HAL_StatusTypeDef DS_RTC_ReadTime(DS_RTC_HandleTypeDef *rtc, DS_RTC_Time *time);
HAL_StatusTypeDef DS_RTC_WriteTime(DS_RTC_HandleTypeDef *rtc, const DS_RTC_Time *time);

// Báo thức
HAL_StatusTypeDef DS_RTC_SetAlarm1(DS_RTC_HandleTypeDef *rtc, const DS_RTC_Alarm1 *alarm);
HAL_StatusTypeDef DS_RTC_SetAlarm2(DS_RTC_HandleTypeDef *rtc, const DS_RTC_Alarm2 *alarm);
HAL_StatusTypeDef DS_RTC_ClearAlarmFlags(DS_RTC_HandleTypeDef *rtc);
uint8_t DS_RTC_GetAlarmFlags(DS_RTC_HandleTypeDef *rtc);

// Nhiệt độ
HAL_StatusTypeDef DS_RTC_GetTemperature(DS_RTC_HandleTypeDef *rtc, float *temperature);

// RAM
HAL_StatusTypeDef DS_RTC_ReadRAM(DS_RTC_HandleTypeDef *rtc, uint8_t offset, uint8_t *data, uint8_t length);
HAL_StatusTypeDef DS_RTC_WriteRAM(DS_RTC_HandleTypeDef *rtc, uint8_t offset, const uint8_t *data, uint8_t length);

// EEPROM
HAL_StatusTypeDef DS_RTC_ReadEEPROM(DS_RTC_HandleTypeDef *rtc, uint8_t address, uint8_t *data, uint8_t length);
HAL_StatusTypeDef DS_RTC_WriteEEPROM(DS_RTC_HandleTypeDef *rtc, uint8_t address, const uint8_t *data, uint8_t length);


// xung vuong
HAL_StatusTypeDef DS_RTC_SetSquareWave(DS_RTC_HandleTypeDef *rtc, DS_RTC_SquareWaveFreq freq, bool enable);
HAL_StatusTypeDef DS_RTC_Enable32KOutput(DS_RTC_HandleTypeDef *rtc, bool enable);



#endif // DS_RTC_CORE_H
