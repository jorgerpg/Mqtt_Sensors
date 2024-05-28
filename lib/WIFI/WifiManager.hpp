#ifndef WIFI_H
#define WIFI_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "ping/ping_sock.h"

class WIFI {
public:
    WIFI();
    void init();
    void start();

private:
    static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    void wifi_init_sta();
    static void ping_test(void* pvParameters);

    static EventGroupHandle_t s_wifi_event_group;
    static const char *TAG;
    static const int WIFI_CONNECTED_BIT;
};

#endif // WIFI_H