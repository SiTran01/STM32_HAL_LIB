#include "../core/DS_RTC_core.h"
#include "DS_3231.h"  // <-- dùng header chính chủ

void DS3231_Init(DS_RTC_HandleTypeDef *rtc, I2C_HandleTypeDef *hi2c) {
    if (rtc == NULL || hi2c == NULL) return;

    rtc->hi2c = hi2c;
    rtc->model = DS_RTC_MODEL_DS3231;
    rtc->i2c_addr = 0x68 << 1;

    rtc->has_alarm = true;     // Có cả Alarm1 + Alarm2
    rtc->has_temp  = true;     // Có cảm biến nhiệt độ
    rtc->has_ram   = true;     // Có 236 byte SRAM
    rtc->has_eeprom = false;   // Không có EEPROM

    // Optional: Bật Oscillator (đảm bảo nó đang hoạt động)
    uint8_t ctrl;
    HAL_I2C_Mem_Read(rtc->hi2c, rtc->i2c_addr, 0x0E, 1, &ctrl, 1, HAL_MAX_DELAY);
    ctrl &= ~(1 << 7);  // Clear EOSC = 0 (enable oscillator)
    HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x0E, 1, &ctrl, 1, HAL_MAX_DELAY);
}
