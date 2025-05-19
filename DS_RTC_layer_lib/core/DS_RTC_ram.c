#include "DS_RTC_core.h"

HAL_StatusTypeDef DS_RTC_ReadRAM(DS_RTC_HandleTypeDef *rtc, uint8_t offset, uint8_t *data, uint8_t length) {
    if (!rtc->has_ram) return HAL_ERROR;

    uint8_t ram_addr;
    switch (rtc->model) {
        case DS_RTC_MODEL_DS1307:
            ram_addr = 0x08 + offset;
            if (ram_addr + length > 0x3F) return HAL_ERROR; // SRAM DS1307: 56 bytes (0x08 - 0x3F)
            break;

        case DS_RTC_MODEL_DS3232:
            ram_addr = 0x14 + offset;
            if (ram_addr + length > 0xFF) return HAL_ERROR; // DS3232: up to 236 bytes (0x14 - 0xFF)
            break;

        default:
            return HAL_ERROR;
    }

    return HAL_I2C_Mem_Read(rtc->hi2c, rtc->i2c_addr, ram_addr, 1, data, length, HAL_MAX_DELAY);
}

HAL_StatusTypeDef DS_RTC_WriteRAM(DS_RTC_HandleTypeDef *rtc, uint8_t offset, const uint8_t *data, uint8_t length) {
    if (!rtc->has_ram) return HAL_ERROR;

    uint8_t ram_addr;
    switch (rtc->model) {
        case DS_RTC_MODEL_DS1307:
            ram_addr = 0x08 + offset;
            if (ram_addr + length > 0x3F) return HAL_ERROR;
            break;

        case DS_RTC_MODEL_DS3232:
            ram_addr = 0x14 + offset;
            if (ram_addr + length > 0xFF) return HAL_ERROR;
            break;

        default:
            return HAL_ERROR;
    }

    return HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, ram_addr, 1, (uint8_t *)data, length, HAL_MAX_DELAY);
}
