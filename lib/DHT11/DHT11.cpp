/**
 * DHT11.cpp
 * Library for reading temperature and humidity from the DHT11 sensor.
 *
 * Author: Dhruba Saha
 * Adapted for ESP-IDF by [Your Name]
 * Version: 2.1.0
 * License: MIT
 */

#include "DHT11.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

#define TAG "DHT11"

DHT11::DHT11(gpio_num_t pin) : _pin(pin)
{
    gpio_set_direction(_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(_pin, 1);
}

void DHT11::setDelay(uint32_t delay)
{
    _delayMS = delay;
}

int DHT11::readRawData(uint8_t data[5])
{
    vTaskDelay(pdMS_TO_TICKS(_delayMS));
    startSignal();
    uint64_t timeout_start = esp_timer_get_time();

    while (gpio_get_level(_pin) == 1)
    {
        if ((esp_timer_get_time() - timeout_start) > TIMEOUT_DURATION * 1000)
        {
            return DHT11::ERROR_TIMEOUT;
        }
    }

    if (gpio_get_level(_pin) == 0)
    {
        esp_rom_delay_us(80);
        if (gpio_get_level(_pin) == 1)
        {
            esp_rom_delay_us(80);
            for (int i = 0; i < 5; i++)
            {
                data[i] = readByte();
                if (data[i] == DHT11::ERROR_TIMEOUT)
                {
                    return DHT11::ERROR_TIMEOUT;
                }
            }

            if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))
            {
                return 0; // Success
            }
            else
            {
                return DHT11::ERROR_CHECKSUM;
            }
        }
    }
    return DHT11::ERROR_TIMEOUT;
}

uint8_t DHT11::readByte()
{
    uint8_t value = 0;

    for (int i = 0; i < 8; i++)
    {
        while (gpio_get_level(_pin) == 0)
            ;
        esp_rom_delay_us(30);
        if (gpio_get_level(_pin) == 1)
        {
            value |= (1 << (7 - i));
        }
        while (gpio_get_level(_pin) == 1)
            ;
    }
    return value;
}

void DHT11::startSignal()
{
    gpio_set_direction(_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(18));
    gpio_set_level(_pin, 1);
    esp_rom_delay_us(40);
    gpio_set_direction(_pin, GPIO_MODE_INPUT);
}

int DHT11::readTemperature()
{
    uint8_t data[5];
    int error = readRawData(data);
    if (error != 0)
    {
        return error;
    }
    return data[2];
}

int DHT11::readHumidity()
{
    uint8_t data[5];
    int error = readRawData(data);
    if (error != 0)
    {
        return error;
    }
    return data[0];
}

int DHT11::readTemperatureHumidity(int &temperature, int &humidity)
{
    uint8_t data[5];
    int error = readRawData(data);
    if (error != 0)
    {
        return error;
    }
    humidity = data[0];
    temperature = data[2];
    return 0; // Indicate success
}

const char* DHT11::getErrorString(int errorCode)
{
    switch (errorCode)
    {
    case DHT11::ERROR_TIMEOUT:
        return "Error 253: Reading from DHT11 timed out.";
    case DHT11::ERROR_CHECKSUM:
        return "Error 254: Checksum mismatch while reading from DHT11.";
    default:
        return "Unknown error.";
    }
}
