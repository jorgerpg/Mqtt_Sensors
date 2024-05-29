#include "app.hpp"
#include "WifiManager.hpp"
#include "MQTTCLI.hpp"
#include "esp_log.h"
#include "esp_system.h"
#include "cJSON.h"
#include <string>

#define WATER_SENSOR_PIN ADC1_CHANNEL_6
#define DHT11_PIN GPIO_NUM_26

DHT11 dht11(DHT11_PIN);
WIFI wifi; // Instância da classe WIFI
MQTTClient mqttClient; // Instância da classe MQTTClient

// Fuso horário (em segundos) - 3 horas para o Brasil (GMT -3)
const long timezoneOffset = -3 * 3600;

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
  read_water_sensor_timer.start(2000);
  mqtt_publish_timer.start(10000);

  // Configuration of ADC1 (ADC unit 1)
  adc1_config_width(ADC_WIDTH_BIT_12); // 12-bit width (0-4095)
  adc1_config_channel_atten(WATER_SENSOR_PIN, ADC_ATTEN_DB_12);

  // Inicializa WiFi
  wifi.start();

  // Inicializa MQTT
  mqttClient.start();
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
    currentState = MQTT_PUBLISH_DATA;
    break;

  case MQTT_PUBLISH_DATA:
    if (mqtt_publish_timer.get())
    {
      publish_mqtt_data();
      mqtt_publish_timer.restart();
    }
    currentState = WATER_SENSOR_READ;
    break;
  }
}

void AppManager::publish_mqtt_data() {
  // Obtenção do endereço MAC do ESP32
  char mac_addr[13];
  uint8_t mac[6];
  esp_wifi_get_mac(WIFI_IF_STA, mac);
  snprintf(mac_addr, 13, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  // Obtenção do timestamp atual ajustado para o fuso horário
  time_t now;
  time(&now);
  now += timezoneOffset; // Aplica o offset de fuso horário
  struct tm *timeinfo = localtime(&now);
  char timestamp[20];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
  
  // Criação do objeto JSON
  cJSON *root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "id", mac_addr);
  cJSON_AddStringToObject(root, "timestamp", timestamp);

  // Criação do payload JSON
  cJSON *payload = cJSON_CreateObject();
  cJSON_AddNumberToObject(payload, "temperatura", m_temperature);
  cJSON_AddNumberToObject(payload, "umidade", m_humidity);
  cJSON_AddNumberToObject(payload, "precipitacao", adc1_get_raw(WATER_SENSOR_PIN));

  // Adiciona o payload ao objeto root
  cJSON_AddItemToObject(root, "payload", payload);

  // Converte o objeto JSON para string
  char *json_str = cJSON_Print(root);
  mqttClient.publish("/sensor/data", json_str);

  cJSON_Delete(root);
  free(json_str);
}
