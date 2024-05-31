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

class WIFI {
public:
    WIFI(const char* ssid, const char* password);
    void start();

private:
    const char* ssid;
    const char* password;
    static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    void wifi_init_sta();
    void init_sntp();

    static EventGroupHandle_t s_wifi_event_group;
    static const char *TAG;
    static const int WIFI_CONNECTED_BIT;
};

#endif // WIFI_H
