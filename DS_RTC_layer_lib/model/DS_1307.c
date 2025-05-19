#include "../core/DS_RTC_core.h"
#include "DS_1307.h"  // <-- dùng header chính chủ

void DS1307_Init(DS_RTC_HandleTypeDef *rtc, I2C_HandleTypeDef *hi2c) {
    if (rtc == NULL || hi2c == NULL) return;

    rtc->hi2c = hi2c;
    rtc->model = DS_RTC_MODEL_DS1307;
    rtc->i2c_addr = 0x68 << 1;
    rtc->has_alarm = false;
    rtc->has_temp = false;
    rtc->has_ram = true;      // DS1307 có RAM 56 byte
    rtc->has_eeprom = false;  // Không có EEPROM
}
