#ifndef __DS3231_H__
#define __DS3231_H__

#include "../core/DS_RTC_core.h"

#ifdef __cplusplus
extern "C" {
#endif

void DS3231_Init(DS_RTC_HandleTypeDef *rtc, I2C_HandleTypeDef *hi2c);

#ifdef __cplusplus
}
#endif

#endif // __DS3231_H__
