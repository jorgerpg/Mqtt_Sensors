/*
 * MIT License
 * 
 * Copyright (c) 2018 Michele Biondi
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/
#include <stdio.h>
#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"
#include "driver/gpio.h"
#include "DHT11.h"
#include "BASES.hpp"

enum {
    INITIALIZE,
    WATER_SENSOR_READ,
    DHT_SENSOR_READ,
    MQTT_PUBLISH_DATA
};

class AppManager
{
public:
  AppManager();
  virtual ~AppManager();
  void initialize();
  void application();
  void publish_mqtt_data();
  void configure_adc();

  int m_temperature;
  int m_humidity;
  uint8_t m_precipitacao;

  BASE read_dht_sensor_timer;
  BASE read_water_sensor_timer;
  BASE mqtt_publish_timer;
  
  uint8_t currentState;
};