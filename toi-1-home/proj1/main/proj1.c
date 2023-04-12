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
// #include "protocol_examples_common.h"
#include "esp_sntp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <string.h>

#include "lwip/err.h"
#include "lwip/sys.h"

#include "time.h"
#include "esp_sntp.h"
#include "dht11.h"
#include "esp_spiffs.h"

#include "temperatur.h"
#include "sntp_time.h"
#include "wifi.h"
#include "dht.h"

#define GPIO_LED_RED 2
#define GPIO_LED1 13
#define GPIO_LED2 12
#define GPIO_LED3 14


#define TAG "main"

void report_task(void *pvParameter)
{
    time_t now;
    char buffer[32];
    FILE *f;
    char line[256];
    esp_vfs_spiffs_conf_t config = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true,
    };
    esp_vfs_spiffs_register(&config);

    while (1)
    {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        f = fopen("/spiffs/data.csv", "w");
        if (f == NULL)
        {
            ESP_LOGE(TAG, "Failed to open file for writing");
            continue;
        }
        time(&now);
        strftime(buffer, sizeof(buffer), "%FT%TZ", gmtime(&now));
        printf("%.32s,%.2f,%d,%.2f\n", buffer, temperatur, hum, temperatur);
        fprintf(f, "%.32s,%.2f,%d,%.2f,\n", buffer, temperatur, hum, temperatur);
        fclose(f);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        f = fopen("/spiffs/data.csv", "r");
        if (f == NULL)
        {
            ESP_LOGE(TAG, "Failed to open file for reading");
            continue;
        }
        while (fgets(line, sizeof(line), f) != NULL)
        {
            printf(line);
        }
        fclose(f);
    }
}

void app_main()
{

    time_t now;
    ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();
    // sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    sntp_setservername(0, "pool.ntp.org");
    initialize_sntp();
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%FT%TZ", gmtime(&now));
    ESP_LOGI(TAG, "The current date/time is: %s", buffer);
    xTaskCreate(&dht_task, "dht_task", 2048, NULL, 5, NULL);
    xTaskCreate(&onewire_task, "onewire_task", 2048, NULL, 5, NULL);
    FILE *f;
    char line[256];
    esp_vfs_spiffs_conf_t config = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true,
    };
    esp_vfs_spiffs_register(&config);

    /* Zapis do suboru */
    // Nedokazal som odstranit errory, ktore vznikli pri pouziti v ramci tasku, preto je kod zapisu vo funkcii main

    //f = fopen("/spiffs/data.csv", "w");
    //fclose(f);

    while (1)
    {
        vTaskDelay(300000 / portTICK_PERIOD_MS);
        f = fopen("/spiffs/data.csv", "a");
        if (f == NULL)
        {
            ESP_LOGE(TAG, "Failed to open file for writing");
            continue;
        }
        time(&now);
        strftime(buffer, sizeof(buffer), "%FT%TZ", gmtime(&now));
        printf("%.32s,%.2f,%d,%.2f\n", buffer, temperatur, hum, temperatur);
        fprintf(f, "%.32s,%.2f,%d,%.2f,\n", buffer, temperatur, hum, temperatur);
        fclose(f);
        /*vTaskDelay(5000 / portTICK_PERIOD_MS);
        f = fopen("/spiffs/data.csv", "r");
        if (f == NULL)
        {
            ESP_LOGE(TAG, "Failed to open file for reading");
            continue;
        }
        while (fgets(line, sizeof(line), f) != NULL)
        {
            printf(line);
        }
        fclose(f);*/
    }
}