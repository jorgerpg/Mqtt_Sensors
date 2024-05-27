#include <stdio.h>
#include "driver/adc.h"
#include "driver/gpio.h"
#include "DHT11.h"
#include "BASES.hpp"

enum {
    INITIALIZE,
    WATER_SENSOR_READ,
    DHT_SENSOR_READ,
};

class AppManager
{
public:
  AppManager();
  virtual ~AppManager();
  void initialize();
  void application();

  int m_temperature;
  int m_humidity;

  BASE read_dht_sensor_timer;
  BASE read_water_sensor_timer;
  
  uint8_t currentState;
};