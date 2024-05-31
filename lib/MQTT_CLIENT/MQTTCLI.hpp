#ifndef MQTT_CLIENT_HPP
#define MQTT_CLIENT_HPP

#include "mqtt_client.h"

class MQTTClient {
public:
    MQTTClient(const char* uri);
    void start();
    void publish(const char* topic, const char* data);
    void subscribe(const char* topic);

private:
    const char* uri;
    esp_mqtt_client_handle_t client;

    static void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data);
};

#endif // MQTT_CLIENT_HPP
