/**
 * @file dht22.h
 * @brief Library for interfacing with DHT22 temperature and humidity sensor.
 * 
 * Author: Si_Tran  
 * Date: 05/14/2025
 * 
 * This file provides function declarations for initializing the DHT22 sensor,
 * reading temperature and humidity, and handling sensor data.
 * 
 * Designed for use with ESP32 and other microcontrollers.
 * Supports reading via single-wire GPIO protocol with microsecond precision.
*/



#ifndef __DHT22_H__
#define __DHT22_H__


#include "stm32f1xx_hal.h"

typedef enum {
    DHT22_OK,
    DHT22_ERROR_TIMEOUT,
    DHT22_ERROR_CHECKSUM,
    DHT22_ERROR_INTERVAL
} DHT22_StatusTypedef;

typedef struct {
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
    uint32_t lastReadTick;
} DHT22_HandleTypedef;

typedef struct {
    float Temperature;
    float Humidity;
} DHT22_DataTypedef;

DHT22_HandleTypedef DHT22_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, TIM_HandleTypeDef* htim);
DHT22_StatusTypedef DHT22_Read(DHT22_HandleTypedef* dht, DHT22_DataTypedef* data);

#endif
