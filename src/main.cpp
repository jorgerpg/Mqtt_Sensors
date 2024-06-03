#include "main.hpp"
#include "app.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void AppManagerTask(void *pvParameters) {
    AppManager appManager;
    while (true) {
        appManager.application();
        vTaskDelay(pdMS_TO_TICKS(100)); // Delay de 1 segundo
    }
}

void app_main()
{
    xTaskCreate(AppManagerTask, "AppManagerTask", 4096, NULL, 1, NULL);
}