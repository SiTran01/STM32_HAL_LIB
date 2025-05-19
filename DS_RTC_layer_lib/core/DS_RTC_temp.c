#include "DS_RTC_core.h"

// ========== Get Temperature ==========

HAL_StatusTypeDef DS_RTC_GetTemperature(DS_RTC_HandleTypeDef *rtc, float *temperature) {
    if (!rtc->has_temp) return HAL_ERROR;

    uint8_t buf[2];
    HAL_StatusTypeDef status;

    // Đọc thanh ghi nhiệt độ: 0x11 (MSB), 0x12 (LSB)
    status = HAL_I2C_Mem_Read(rtc->hi2c, rtc->i2c_addr, 0x11, 1, buf, 2, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    int8_t temp_msb = (int8_t)buf[0];       // Phần nguyên có dấu
    uint8_t temp_lsb = buf[1] >> 6;         // 2 bit cao nhất (quy ước 0.25 độ)

    *temperature = temp_msb + (temp_lsb * 0.25f);

    return HAL_OK;
}
