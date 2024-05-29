#include "MQTTCLI.hpp"
#include "esp_log.h"

// Define the logging tag for this module
static const char *TAG = "MQTTClient";

// Constructor for the MQTTClient class, initializes the client with the given URI
MQTTClient::MQTTClient(const char* uri) : uri(uri) {
}

// Start the MQTT client
void MQTTClient::start() {
    mqtt_app_start();
}

// Publish a message to the specified topic
void MQTTClient::publish(const char* topic, const char* data) {
    esp_mqtt_client_publish(client, topic, data, 0, 1, 0);
}

// Subscribe to the specified topic
void MQTTClient::subscribe(const char* topic) {
    esp_mqtt_client_subscribe(client, topic, 0);
}

// Event handler for MQTT events
void MQTTClient::mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t) event_data;
    MQTTClient* mqttClient = (MQTTClient*) handler_args;

    // Handle different types of MQTT events
    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            mqttClient->subscribe("/test/topic"); // Example topic
            mqttClient->publish("/test/topic", "Hello MQTT");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

// Initialize and start the MQTT client
void MQTTClient::mqtt_app_start() {
    esp_mqtt_client_config_t mqtt_cfg = {}; // Initialize the MQTT client configuration structure
    mqtt_cfg.broker.address.uri = uri; // Set the broker URI

    client = esp_mqtt_client_init(&mqtt_cfg); // Initialize the MQTT client
    esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler, this); // Register the event handler
    esp_mqtt_client_start(client); // Start the MQTT client
}