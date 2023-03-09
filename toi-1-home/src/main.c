#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_system.h"

#include "esp_log.h"

int app_main() {
    while(1) {
        ESP_LOGI("main", "Hello from main\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}