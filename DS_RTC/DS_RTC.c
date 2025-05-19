#include "DS_RTC.h"

// ========== Utility ==========

uint8_t DS_RTC_ToBCD(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

uint8_t DS_RTC_FromBCD(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// ========== Init ==========

void DS_RTC_Init(DS_RTC_HandleTypeDef *rtc, I2C_HandleTypeDef *hi2c, DS_RTC_Model chip) {
    rtc->hi2c = hi2c;
    rtc->chip = chip;
    rtc->i2c_addr = 0x68 << 1; // HAL expects 8-bit address

    // Reset flags
    rtc->has_alarm = false;
    rtc->has_temp  = false;
    rtc->has_sram  = false; // Optional if supported
    rtc->has_eeprom = false;

    switch (chip) {
        case DS_RTC_DS1307:
            rtc->has_sram = true;
            break;

        case DS_RTC_DS1338:
        case DS_RTC_DS1340:
            break;

        case DS_RTC_DS1337:
        case DS_RTC_DS1339:
        case DS_RTC_DS1341:
        case DS_RTC_DS1342:
            rtc->has_alarm = true;
            break;

        case DS_RTC_DS1388:
            rtc->has_alarm = true;
            rtc->has_eeprom = true;
            break;

        case DS_RTC_DS3231:
        case DS_RTC_DS3232:
            rtc->has_alarm = true;
            rtc->has_temp = true;
            if (chip == DS_RTC_DS3232) {
                rtc->has_sram = true;
            }

            // Enable battery-backed oscillator and alarm control
            {
                uint8_t ctrl = 0x1C; // EOSC=0, BBSQW=1, CONV=1, RS2=1, RS1=0
                HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x0E, 1, &ctrl, 1, HAL_MAX_DELAY);
            }
            break;

        default:
            break;
    }
}

// ========== Read Time ==========

HAL_StatusTypeDef DS_RTC_ReadTime(DS_RTC_HandleTypeDef *rtc, DS_RTC_Time *time) {
    uint8_t buf[7];
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(rtc->hi2c, rtc->i2c_addr, 0x00, 1, buf, 7, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    time->seconds      = DS_RTC_FromBCD(buf[0] & 0x7F);
    time->minutes      = DS_RTC_FromBCD(buf[1] & 0x7F);
    time->hours        = DS_RTC_FromBCD(buf[2] & 0x3F);
    time->day_of_week  = DS_RTC_FromBCD(buf[3] & 0x07);
    time->day          = DS_RTC_FromBCD(buf[4] & 0x3F);
    time->month        = DS_RTC_FromBCD(buf[5] & 0x1F);
    time->year         = 2000 + DS_RTC_FromBCD(buf[6]);

    return HAL_OK;
}

// ========== Write Time ==========

HAL_StatusTypeDef DS_RTC_WriteTime(DS_RTC_HandleTypeDef *rtc, const DS_RTC_Time *time) {
    uint8_t buf[7];

    buf[0] = DS_RTC_ToBCD(time->seconds) & 0x7F; // Ensure CH bit is 0
    buf[1] = DS_RTC_ToBCD(time->minutes);
    buf[2] = DS_RTC_ToBCD(time->hours);
    buf[3] = DS_RTC_ToBCD(time->day_of_week);
    buf[4] = DS_RTC_ToBCD(time->day);
    buf[5] = DS_RTC_ToBCD(time->month);
    buf[6] = DS_RTC_ToBCD(time->year % 100);

    return HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x00, 1, buf, 7, HAL_MAX_DELAY);
}

// ========== Set Alarm ==========

HAL_StatusTypeDef DS_RTC_SetAlarm(DS_RTC_HandleTypeDef *rtc, const DS_RTC_Alarm *alarm) {
    if (!rtc->has_alarm) return HAL_ERROR;

    uint8_t buf[5];

    // Bây gi? so kh?p giây n?u match_seconds == true
    buf[0] = DS_RTC_ToBCD(alarm->seconds) | (alarm->match_seconds ? 0x00 : 0x80); // A1M1
    buf[1] = DS_RTC_ToBCD(alarm->minutes) | (alarm->match_minutes ? 0x00 : 0x80); // A1M2
    buf[2] = DS_RTC_ToBCD(alarm->hours)   | (alarm->match_hours   ? 0x00 : 0x80); // A1M3
    buf[3] = (DS_RTC_ToBCD(alarm->day) & 0x3F) | (alarm->match_day ? 0x00 : 0x80); // A1M4

    buf[4] = 0x00; // DY/DT = 0, ch? dùng ngày, không dùng th?

    // Ghi 5 byte vào t? d?a ch? 0x07
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x07, 1, buf, 4, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    // B?t A1IE (Alarm 1 Interrupt Enable) và INTCN
    uint8_t ctrl = 0x05;
    status = HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x0E, 1, &ctrl, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    // Xóa c? báo th?c A1F n?u dang set
    uint8_t status_reg;
    HAL_I2C_Mem_Read(rtc->hi2c, rtc->i2c_addr, 0x0F, 1, &status_reg, 1, HAL_MAX_DELAY);
    status_reg &= ~(1 << 0); // Clear bit A1F
    return HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x0F, 1, &status_reg, 1, HAL_MAX_DELAY);
}


HAL_StatusTypeDef DS_RTC_SetAlarm2(DS_RTC_HandleTypeDef *rtc, const DS_RTC_Alarm2 *alarm) {
    if (!rtc->has_alarm) return HAL_ERROR;

    uint8_t buf[3];
    buf[0] = DS_RTC_ToBCD(alarm->minutes) | (alarm->match_minutes ? 0x00 : 0x80);
    buf[1] = DS_RTC_ToBCD(alarm->hours)   | (alarm->match_hours   ? 0x00 : 0x80);
    buf[2] = (DS_RTC_ToBCD(alarm->day) & 0x3F) | (alarm->match_day ? 0x00 : 0x80);

    // Ghi vào thanh ghi Alarm 2 (0x0B - 0x0D)
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x0B, 1, buf, 3, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    // B?t Alarm 2 và INTCN
    uint8_t ctrl;
    HAL_I2C_Mem_Read(rtc->hi2c, rtc->i2c_addr, 0x0E, 1, &ctrl, 1, HAL_MAX_DELAY);
    ctrl |= (1 << 2) | (1 << 1); // INTCN = 1, A2IE = 1
    HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x0E, 1, &ctrl, 1, HAL_MAX_DELAY);

    // Clear c? A2F
    uint8_t status_reg;
    HAL_I2C_Mem_Read(rtc->hi2c, rtc->i2c_addr, 0x0F, 1, &status_reg, 1, HAL_MAX_DELAY);
    status_reg &= ~(1 << 1); // clear A2F
    return HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x0F, 1, &status_reg, 1, HAL_MAX_DELAY);
}





// ========== Get Temperature ==========

HAL_StatusTypeDef DS_RTC_GetTemperature(DS_RTC_HandleTypeDef *rtc, float *temperature) {
    if (!rtc->has_temp) return HAL_ERROR;

    uint8_t buf[2];
    HAL_StatusTypeDef status;

    // Thanh ghi nhi?t d?: 0x11 (MSB), 0x12 (LSB)
    status = HAL_I2C_Mem_Read(rtc->hi2c, rtc->i2c_addr, 0x11, 1, buf, 2, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    int8_t temp_msb = (int8_t)buf[0];       // Có d?u
    uint8_t temp_lsb = buf[1] >> 6;         // L?y 2 bit cao nh?t

    *temperature = temp_msb + (temp_lsb * 0.25f);

    return HAL_OK;
}


// ========== Clear Alarm Flag ==========

HAL_StatusTypeDef DS_RTC_ClearAlarmFlag(DS_RTC_HandleTypeDef *rtc) {
    if (!rtc->has_alarm) return HAL_ERROR;

    uint8_t status_reg;
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(rtc->hi2c, rtc->i2c_addr, 0x0F, 1, &status_reg, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    status_reg &= ~(0x01); // Clear A1F
    return HAL_I2C_Mem_Write(rtc->hi2c, rtc->i2c_addr, 0x0F, 1, &status_reg, 1, HAL_MAX_DELAY);
}
