#include "app.hpp"

#define WATER_SENSOR_PIN ADC1_CHANNEL_6
#define DHT11_PIN GPIO_NUM_26

DHT11 dht11(DHT11_PIN);

/**
 * @brief Constructor for the AppManager class.
 */
AppManager::AppManager()
{
  m_temperature = 0;
  m_humidity = 0;
  currentState = INITIALIZE;
}

AppManager::~AppManager() {}

void AppManager::initialize()
{
  read_dht_sensor_timer.start(1000);
  read_water_sensor_timer.start(5000);

  // Configuration of ADC1 (ADC unit 1)
  adc1_config_width(ADC_WIDTH_BIT_12); // 12-bit width (0-4095)
  adc1_config_channel_atten(WATER_SENSOR_PIN, ADC_ATTEN_DB_12);
}

void AppManager::application()
{
  switch (currentState)
  {
  case INITIALIZE:
    initialize();
    currentState = WATER_SENSOR_READ;
    break;

  case WATER_SENSOR_READ:
    if (read_water_sensor_timer.get())
    {
      // Read the ADC value from channel 6 (GPIO34)
      uint16_t adc_reading = adc1_get_raw(WATER_SENSOR_PIN);
      // Print the ADC reading
      printf("ADC reading: %d\n", adc_reading);
      read_water_sensor_timer.restart();
    }
    currentState = DHT_SENSOR_READ;
    break;

  case DHT_SENSOR_READ:
    if (read_dht_sensor_timer.get())
    {
      int result = dht11.readTemperatureHumidity(m_temperature, m_humidity);
      if (result == 0)
      {
        printf("Temperature: %d°C, Humidity: %d%%\n", m_temperature, m_humidity);
      }
      else
      {
        printf("Failed to read from DHT11 sensor: %s\n", dht11.getErrorString(result));
      }
      read_dht_sensor_timer.restart();
    }
    currentState = WATER_SENSOR_READ;
    break;
  }
}