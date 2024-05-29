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