#include "DS_RTC_core.h"

// ========== Utility ==========

static uint8_t ToBCD(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

//static uint8_t FromBCD(uint8_t bcd) {
//    return ((bcd >> 4) * 10) + (bcd & 0x0F);
//}

// ========== Alarm 1 ==========

HAL_StatusTypeDef DS_RTC_SetAlarm1(DS_RTC_HandleTypeDef *rtc, const DS_RTC_Alarm1 *alarm) {
    if (!rtc->has_alarm) return HAL_ERROR;

    uint8_t buf[4];
    buf[0] = ToBCD(alarm->seconds) | (alarm->match_seconds ? 0x00 : 0x80); // A1M1
    buf[1] = ToBCD(alarm->minutes) | (alarm->match_minutes ? 0x00 : 0x80); // A1M2
    buf[2] = ToBCD(alarm->hours)   | (alarm->match_hours   ? 0x00 : 0x80); // A1M3
    buf[3] = ToBCD(alarm->day & 0x3F) | (alarm->match_day ? 0x00 : 0x80);  // A1M4

    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x07, 1, buf, 4, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    // Enable A1IE and INTCN
    uint8_t ctrl;
    HAL_I2C_Mem_Read(rtc->hi2c, rtc->i2c_addr, 0x0E, 1, &ctrl, 1, HAL_MAX_DELAY);
    ctrl |= (1 << 0) | (1 << 2); // A1IE = 1, INTCN = 1
    return HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x0E, 1, &ctrl, 1, HAL_MAX_DELAY);
}	

// ========== Alarm 2 ==========

HAL_StatusTypeDef DS_RTC_SetAlarm2(DS_RTC_HandleTypeDef *rtc, const DS_RTC_Alarm2 *alarm) {
    if (!rtc->has_alarm) return HAL_ERROR;

    uint8_t buf[3];
    buf[0] = ToBCD(alarm->minutes) | (alarm->match_minutes ? 0x00 : 0x80);
    buf[1] = ToBCD(alarm->hours)   | (alarm->match_hours   ? 0x00 : 0x80);
    buf[2] = ToBCD(alarm->day & 0x3F) | (alarm->match_day ? 0x00 : 0x80);

    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x0B, 1, buf, 3, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    // Enable A2IE and INTCN
    uint8_t ctrl;
    HAL_I2C_Mem_Read(rtc->hi2c, rtc->i2c_addr, 0x0E, 1, &ctrl, 1, HAL_MAX_DELAY);
    ctrl |= (1 << 1) | (1 << 2); // A2IE = 1, INTCN = 1
    return HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x0E, 1, &ctrl, 1, HAL_MAX_DELAY);
}

// ========== Clear Alarm Flags ==========

HAL_StatusTypeDef DS_RTC_ClearAlarmFlags(DS_RTC_HandleTypeDef *rtc) {
    if (!rtc->has_alarm) return HAL_ERROR;

    uint8_t status_reg;
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(rtc->hi2c, rtc->i2c_addr, 0x0F, 1, &status_reg, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    status_reg &= ~(0x03); // Clear A1F and A2F
    return HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x0F, 1, &status_reg, 1, HAL_MAX_DELAY);
}


uint8_t DS_RTC_GetAlarmFlags(DS_RTC_HandleTypeDef *rtc) {
    if (!rtc->has_alarm) return 0;

    uint8_t status = 0;
    HAL_I2C_Mem_Read(rtc->hi2c, rtc->i2c_addr, 0x0F, 1, &status, 1, HAL_MAX_DELAY);
    return status & 0x03; // Tr? v? bit A1F và A2F
}

