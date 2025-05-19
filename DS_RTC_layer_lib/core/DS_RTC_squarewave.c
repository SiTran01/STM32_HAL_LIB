#include "DS_RTC_core.h"

// ===================== DS1307Z ===================== //
static HAL_StatusTypeDef DS1307_SetSquareWave(DS_RTC_HandleTypeDef *rtc, DS_RTC_SquareWaveFreq freq, bool enable) {
    uint8_t ctrl;

    if (HAL_I2C_Mem_Read(rtc->hi2c, rtc->i2c_addr, 0x07, 1, &ctrl, 1, HAL_MAX_DELAY) != HAL_OK)
        return HAL_ERROR;

    ctrl &= ~(0x90); // Clear SQWE (bit 4), RS1/RS0 (bits 7-6)

    if (enable) {
        ctrl |= (1 << 4); // SQWE = 1
        switch (freq) {
            case DS_RTC_SQW_1HZ:     ctrl |= (0 << 6); break;
            case DS_RTC_SQW_4096HZ:  ctrl |= (1 << 6); break;
            case DS_RTC_SQW_8192HZ:  ctrl |= (2 << 6); break;
            case DS_RTC_SQW_32768HZ: ctrl |= (3 << 6); break;
            default: return HAL_ERROR;
        }
    }

    return HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x07, 1, &ctrl, 1, HAL_MAX_DELAY);
}

// ===================== DS3231M ===================== //
// Ch? h? tr? 1Hz c? d?nh, không c?u hình RS bits

static HAL_StatusTypeDef DS3231M_SetSquareWave(DS_RTC_HandleTypeDef *rtc, DS_RTC_SquareWaveFreq freq, bool enable) {
    uint8_t ctrl;

    if (freq != DS_RTC_SQW_1HZ && freq != DS_RTC_SQW_FIXED_1HZ)
        return HAL_ERROR; // Không h? tr? các t?n s? khác

    if (HAL_I2C_Mem_Read(rtc->hi2c, rtc->i2c_addr, 0x0E, 1, &ctrl, 1, HAL_MAX_DELAY) != HAL_OK)
        return HAL_ERROR;

    if (enable) {
        ctrl &= ~(1 << 2); // INTCN = 0 d? b?t SQW
    } else {
        ctrl |= (1 << 2);  // INTCN = 1 d? t?t SQW
    }

    return HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x0E, 1, &ctrl, 1, HAL_MAX_DELAY);
}

// ===================== API CHUNG ===================== //

HAL_StatusTypeDef DS_RTC_SetSquareWave(DS_RTC_HandleTypeDef *rtc, DS_RTC_SquareWaveFreq freq, bool enable) {
    switch (rtc->model) {
        case DS_RTC_MODEL_DS1307:
            return DS1307_SetSquareWave(rtc, freq, enable);

        case DS_RTC_MODEL_DS3231:
            return DS3231M_SetSquareWave(rtc, freq, enable);

        default:
            return HAL_ERROR;
    }
}

// ===================== B?T/T?T 32KHz (DS3231M only) ===================== //

HAL_StatusTypeDef DS_RTC_Enable32KOutput(DS_RTC_HandleTypeDef *rtc, bool enable) {
    if (rtc->model != DS_RTC_MODEL_DS3231)
        return HAL_ERROR;

    uint8_t status;
    if (HAL_I2C_Mem_Read(rtc->hi2c, rtc->i2c_addr, 0x0F, 1, &status, 1, HAL_MAX_DELAY) != HAL_OK)
        return HAL_ERROR;

    if (enable)
        status |= (1 << 3);  // EN32kHz = 1
    else
        status &= ~(1 << 3); // EN32kHz = 0

    return HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x0F, 1, &status, 1, HAL_MAX_DELAY);
}
