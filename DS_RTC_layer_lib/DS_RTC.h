#ifndef DS_RTC_H
#define DS_RTC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "core/DS_RTC_core.h"

// Khởi tạo theo model
void DS_RTC_Init(DS_RTC_HandleTypeDef *rtc, I2C_HandleTypeDef *hi2c, DS_RTC_Model model);

// Tính năng thời gian
HAL_StatusTypeDef DS_RTC_ReadTime(DS_RTC_HandleTypeDef *rtc, DS_RTC_Time *time);
HAL_StatusTypeDef DS_RTC_WriteTime(DS_RTC_HandleTypeDef *rtc, const DS_RTC_Time *time);

// Báo thức (chỉ nếu có)
HAL_StatusTypeDef DS_RTC_SetAlarm(DS_RTC_HandleTypeDef *rtc, const DS_RTC_Alarm1 *alarm);
HAL_StatusTypeDef DS_RTC_SetAlarm2(DS_RTC_HandleTypeDef *rtc, const DS_RTC_Alarm2 *alarm);
HAL_StatusTypeDef DS_RTC_ClearAlarmFlag(DS_RTC_HandleTypeDef *rtc);

// Nhiệt độ (nếu hỗ trợ)
HAL_StatusTypeDef DS_RTC_GetTemperature(DS_RTC_HandleTypeDef *rtc, float *temperature);

// RAM hoặc EEPROM (nếu hỗ trợ)
HAL_StatusTypeDef DS_RTC_ReadMemory(DS_RTC_HandleTypeDef *rtc, uint8_t mem_addr, uint8_t *data, uint16_t len);
HAL_StatusTypeDef DS_RTC_WriteMemory(DS_RTC_HandleTypeDef *rtc, uint8_t mem_addr, const uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif // DS_RTC_H
