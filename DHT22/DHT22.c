/**
 * @file DHT22.c
 * @brief Driver for DHT22 temperature and humidity sensor.
 * 
 * This file provides functions to interface with the DHT22 sensor using a single GPIO pin.
 * The DHT22 sensor communicates using a proprietary 1-wire protocol that requires precise timing.
 * 
 * Functions:
 * - DHT22_Init: Initializes the sensor with the specified GPIO pin.
 * - DHT22_Read: Reads temperature and humidity values from the sensor.
 * - DHT22_GetTemperature: Returns the last read temperature.
 * - DHT22_GetHumidity: Returns the last read humidity.
 * 
 * Usage:
 * - Call DHT22_Init once before reading data.
 * - Use DHT22_Read periodically (e.g. every 2 seconds).
 * - After a successful read, use DHT22_GetTemperature and DHT22_GetHumidity to get values.
 * 
* Notes:
 * - DHT22 requires a delay of at least 2 seconds between reads.
 * - Reading may fail due to timing issues or sensor errors; always check return status.
 *** - If hardware timer is not properly initialized, the sensor may return invalid data or cause runtime errors.


	========================================================================================================
	
*/


#include "DHT22.h"

// Pointer to timer handler used for microsecond delay
static TIM_HandleTypeDef* _dhtTim;

// Delay function in microseconds using hardware timer
static void delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(_dhtTim, 0);
    while (__HAL_TIM_GET_COUNTER(_dhtTim) < us);
}

// Configure the GPIO pin as output (push-pull)
static void Set_Pin_Output(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

// Configure the GPIO pin as input (no pull-up or pull-down)
static void Set_Pin_Input(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

// Initialize the DHT22 sensor and associated GPIO and Timer
DHT22_HandleTypedef DHT22_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, TIM_HandleTypeDef* htim) {
    _dhtTim = htim;
    HAL_TIM_Base_Start(htim); // Start the hardware timer

    DHT22_HandleTypedef dht;
    dht.GPIOx = GPIOx;
    dht.GPIO_Pin = GPIO_Pin;
    dht.lastReadTick = HAL_GetTick() - 2000;  // Allow immediate reading
    return dht;
}

// Read a single bit from DHT22 data line
static uint8_t DHT22_ReadBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
    uint32_t timeout = 0;

    // Wait for pin to go HIGH (start of bit)
    while (!HAL_GPIO_ReadPin(GPIOx, GPIO_Pin)) {
        if (++timeout > 10000) return 0;
    }

    // Delay 40us then read the level (1 or 0)
    delay_us(40);
    uint8_t bit = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);

    // Wait until pin goes LOW (end of bit)
    timeout = 0;
    while (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin)) {
        if (++timeout > 10000) break;
    }

    return bit;
}

// Read a byte (8 bits) from the DHT22
static uint8_t DHT22_ReadByte(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        byte <<= 1;
        byte |= DHT22_ReadBit(GPIOx, GPIO_Pin);
    }
    return byte;
}

// Read temperature and humidity from DHT22 sensor
DHT22_StatusTypedef DHT22_Read(DHT22_HandleTypedef* dht, DHT22_DataTypedef* data) {
    uint8_t bits[5] = {0};

    // Enforce minimum interval between reads (2 seconds)
    if (HAL_GetTick() - dht->lastReadTick < 2000) return DHT22_ERROR_INTERVAL;
    dht->lastReadTick = HAL_GetTick();

    // Send start signal
    Set_Pin_Output(dht->GPIOx, dht->GPIO_Pin);
    HAL_GPIO_WritePin(dht->GPIOx, dht->GPIO_Pin, GPIO_PIN_RESET);
    delay_us(1000);  // Hold low for at least 1ms
    HAL_GPIO_WritePin(dht->GPIOx, dht->GPIO_Pin, GPIO_PIN_SET);
    delay_us(30);    // Pull high briefly before switching to input

    // Wait for sensor response
    Set_Pin_Input(dht->GPIOx, dht->GPIO_Pin);
    uint32_t timeout = 0;

    // Wait for sensor to pull pin low
    if (HAL_GPIO_ReadPin(dht->GPIOx, dht->GPIO_Pin)) return DHT22_ERROR_TIMEOUT;
    while (!HAL_GPIO_ReadPin(dht->GPIOx, dht->GPIO_Pin)) {
        if (++timeout > 10000) return DHT22_ERROR_TIMEOUT;
    }

    // Wait for sensor to pull pin high
    timeout = 0;
    while (HAL_GPIO_ReadPin(dht->GPIOx, dht->GPIO_Pin)) {
        if (++timeout > 10000) return DHT22_ERROR_TIMEOUT;
    }

    // Read 5 bytes (40 bits) from sensor
    for (int i = 0; i < 5; i++) {
        bits[i] = DHT22_ReadByte(dht->GPIOx, dht->GPIO_Pin);
    }

    // Verify checksum
    uint8_t sum = bits[0] + bits[1] + bits[2] + bits[3];
    if (sum != bits[4]) return DHT22_ERROR_CHECKSUM;

    // Convert and store humidity and temperature
    data->Humidity = ((bits[0] << 8) | bits[1]) / 10.0f;
    data->Temperature = (((bits[2] & 0x7F) << 8) | bits[3]) / 10.0f;
    if (bits[2] & 0x80) data->Temperature *= -1;

    return DHT22_OK;
}

/*
	====================================================================================

The DHT22 sensor sends 40 bits of data in total:
- 16 bits for humidity
- 16 bits for temperature
- 8 bits for checksum (sum of the previous 4 bytes)

Communication protocol:
1. MCU pulls the data line LOW for at least 1 ms to start the transmission.
2. MCU releases the line and waits for the DHT22 to respond.
3. DHT22 pulls the line LOW and HIGH to signal its response.
4. It then sends 40 bits by varying the duration of HIGH pulses:
   - ~26-28µs HIGH = bit 0
   - ~70µs HIGH = bit 1

Important timing considerations:
- Reading should be done using precise delays (microsecond level).
- Disable interrupts or use critical sections during read for timing accuracy.

Example usage:

    DHT22_Init(GPIO_NUM_4);
    
    if (DHT22_Read() == DHT22_OK) {
        float temp = DHT22_GetTemperature();
        float humi = DHT22_GetHumidity();
        // Use the temperature and humidity values
    }

*/


