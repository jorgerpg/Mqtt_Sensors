// mosquitto -v -c testconfig.txt / ativar mosquitto com o ip da maquina
#include "app.hpp"
#include "WifiManager.hpp"
#include "MQTTCLI.hpp"
#include "esp_log.h"
#include "esp_system.h"
#include "cJSON.h"

// Define the pins for the water sensor and DHT11 sensor
#define WATER_SENSOR_PIN ADC_CHANNEL_6
#define DHT11_PIN GPIO_NUM_26

// Create instances of DHT11 sensor, WiFi manager, and MQTT client
// DHT11 dht11(DHT11_PIN);
WIFI wifi("Vrumvrum", "jayjayojatinho"); // WiFi SSID and password
MQTTClient mqttClient("mqtt://192.168.220.237:1883"); // MQTT broker URI

// Timezone offset in seconds (3 hours for Brazil, GMT -3)
const long timezoneOffset = -3 * 3600;

// Instance for ADC one-shot driver
adc_oneshot_unit_handle_t adc1_handle;

// Logger tag for the AppManager
static const char *TAG = "AppManager";

/**
 * @brief Constructor for the AppManager class.
 */
AppManager::AppManager() : m_temperature(0), m_humidity(0), m_precipitacao(0), currentState(INITIALIZE) {}

AppManager::~AppManager() {}

/**
 * @brief Initialize the application components.
 */
void AppManager::initialize() {
    // Start timers for sensors and MQTT publish
    read_dht_sensor_timer.start(2000);
    read_water_sensor_timer.start(9000);
    mqtt_publish_timer.start(10000);

    // Configure ADC one-shot driver
    configure_adc();

    DHT11_init(GPIO_NUM_26);

    // Start WiFi and MQTT services
    wifi.start();
    mqttClient.start();
}

/**
 * @brief Main application logic.
 */
void AppManager::application() {
    switch (currentState) {
        case INITIALIZE:
            initialize();
            currentState = WATER_SENSOR_READ;
            break;

        case WATER_SENSOR_READ:
            if (read_water_sensor_timer.get()) {
                // Read the ADC value from the water sensor using one-shot driver
                int adc_reading;
                ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, WATER_SENSOR_PIN, &adc_reading));

                // Log the ADC reading
                ESP_LOGI(TAG, "ADC reading of water sensor: %d", adc_reading);

                // Determine precipitation based on ADC reading
                m_precipitacao = adc_reading > 0 ? 1 : 0;
                
                read_water_sensor_timer.restart();
            }
            currentState = DHT_SENSOR_READ;
            break;

        case DHT_SENSOR_READ:
            if (read_dht_sensor_timer.get()) {
                dht11_reading result = DHT11_read();
                if(result.temperature >= 0 && result.humidity >= 20)
                {
                    m_temperature = result.temperature;
                    m_humidity = result.humidity;
                }
                ESP_LOGI(TAG, "Temperature: %d°C, Humidity: %d%%", m_temperature, m_humidity);

                read_dht_sensor_timer.restart();
            }
            currentState = MQTT_PUBLISH_DATA;
            break;

        case MQTT_PUBLISH_DATA:
            if (mqtt_publish_timer.get()) {
                publish_mqtt_data();
                mqtt_publish_timer.restart();
            }
            currentState = WATER_SENSOR_READ;
            break;
    }
}

/**
 * @brief Publish sensor data to the MQTT broker.
 */
void AppManager::publish_mqtt_data() {
    // Get the MAC address of the ESP32
    char mac_addr[13];
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    snprintf(mac_addr, sizeof(mac_addr), "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // Get the current timestamp adjusted for the timezone
    time_t now;
    time(&now);
    now += timezoneOffset;
    struct tm *timeinfo = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    // Create the JSON object for the message
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "id", mac_addr);
    cJSON_AddStringToObject(root, "timestamp", timestamp);

    // Create the JSON payload
    cJSON *payload = cJSON_CreateObject();
    cJSON_AddNumberToObject(payload, "temperature", m_temperature);
    cJSON_AddNumberToObject(payload, "humidity", m_humidity);
    cJSON_AddNumberToObject(payload, "precipitation", m_precipitacao);

    // Add the payload to the root object
    cJSON_AddItemToObject(root, "payload", payload);

    // Convert the JSON object to a string and publish it
    char *json_str = cJSON_Print(root);
    mqttClient.publish("/sensor/data", json_str);

    // Clean up JSON object and string
    cJSON_Delete(root);
    free(json_str);
}

/**
 * @brief Configure the ADC one-shot driver.
 */
void AppManager::configure_adc() {
    // Configuration of the one-shot driver
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT, // Initialize clk_src
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc1_handle));

    // Configuration of the ADC channel
    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, WATER_SENSOR_PIN, &chan_cfg));
}