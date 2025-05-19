#include "DS_RTC_core.h"

// ========== Utility ==========

static uint8_t ToBCD(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

static uint8_t FromBCD(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// ========== Read Time ==========

HAL_StatusTypeDef DS_RTC_ReadTime(DS_RTC_HandleTypeDef *rtc, DS_RTC_Time *time) {
    uint8_t buf[7];
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(rtc->hi2c, rtc->i2c_addr, 0x00, 1, buf, 7, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    time->seconds      = FromBCD(buf[0] & 0x7F);
    time->minutes      = FromBCD(buf[1] & 0x7F);
    time->hours        = FromBCD(buf[2] & 0x3F);
    time->day_of_week  = FromBCD(buf[3] & 0x07);
    time->day          = FromBCD(buf[4] & 0x3F);
    time->month        = FromBCD(buf[5] & 0x1F);
    time->year         = 2000 + FromBCD(buf[6]);

    return HAL_OK;
}

// ========== Write Time ==========

HAL_StatusTypeDef DS_RTC_WriteTime(DS_RTC_HandleTypeDef *rtc, const DS_RTC_Time *time) {
    uint8_t buf[7];

    buf[0] = ToBCD(time->seconds) & 0x7F; // Ensure CH = 0
    buf[1] = ToBCD(time->minutes);
    buf[2] = ToBCD(time->hours);
    buf[3] = ToBCD(time->day_of_week);
    buf[4] = ToBCD(time->day);
    buf[5] = ToBCD(time->month);
    buf[6] = ToBCD(time->year % 100);

    return HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x00, 1, buf, 7, HAL_MAX_DELAY);
}
