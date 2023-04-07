#include <stdio.h>
#include "rom/gpio.h"
#include "driver/gpio.h"
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "dht11.h"

#include "dht.h"

#define TAG "dht"

int hum;

void dht_task(void *pvParameter)
{
    DHT11_init(GPIO_NUM_18);
    while (1)
    {
        if (!DHT11_read().status)
            hum = DHT11_read().humidity;
        // printf("Temperature is %d \n", DHT11_read().temperature);
        // printf("Humidity is %d\n", DHT11_read().humidity);
        // printf("Status code is %d\n", DHT11_read().status);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
