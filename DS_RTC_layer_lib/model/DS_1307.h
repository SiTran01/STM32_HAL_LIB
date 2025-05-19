#ifndef __DS1307_H__
#define __DS1307_H__

#include "../core/DS_RTC_core.h"

#ifdef __cplusplus
extern "C" {
#endif

void DS1307_Init(DS_RTC_HandleTypeDef *rtc, I2C_HandleTypeDef *hi2c);

#ifdef __cplusplus
}
#endif

#endif // __DS1307_H__
