#include "DS_RTC_core.h"

HAL_StatusTypeDef DS_RTC_ReadEEPROM(DS_RTC_HandleTypeDef *rtc, uint8_t address, uint8_t *data, uint8_t length) {
    if (!rtc->has_eeprom) return HAL_ERROR;

    // DS1388 có 512 bytes EEPROM, bắt đầu từ 0x00
    return HAL_I2C_Mem_Read(rtc->hi2c, rtc->i2c_addr, address, 1, data, length, HAL_MAX_DELAY);
}

HAL_StatusTypeDef DS_RTC_WriteEEPROM(DS_RTC_HandleTypeDef *rtc, uint8_t address, const uint8_t *data, uint8_t length) {
    if (!rtc->has_eeprom) return HAL_ERROR;

    // EEPROM cần delay sau khi ghi (tầm ~5ms/byte)
    for (uint8_t i = 0; i < length; i++) {
        HAL_StatusTypeDef status = HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, address + i, 1, (uint8_t *)&data[i], 1, HAL_MAX_DELAY);
        if (status != HAL_OK) return status;
        HAL_Delay(5); // delay để EEPROM ghi hoàn tất
    }

    return HAL_OK;
}
