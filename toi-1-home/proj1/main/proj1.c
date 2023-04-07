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
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "time.h"
#include "esp_sntp.h"
#include "dht11.h"

#include "temperatur.h"
#include "sntp_time.h"
int hum;

#define EXAMPLE_ESP_WIFI_SSID "TP-Link_Archer"
#define EXAMPLE_ESP_WIFI_PASS "9904JezoPecuchaKlucho"
#define EXAMPLE_ESP_MAXIMUM_RETRY 10

#define GPIO_LED_RED 2
#define GPIO_LED1 13
#define GPIO_LED2 12
#define GPIO_LED3 14

static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Setting a password implies station will connect to all
             * security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be
             * used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting
             * below line */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false},
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

void hello_task(void *pvParameter)
{
    printf("Hello world!\n");
    for (int i = 1; 1; i++)
    {
        printf("Running %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void blink_task(void *pvParameter)
{
    gpio_pad_select_gpio(GPIO_LED_RED);
    gpio_pad_select_gpio(GPIO_LED1);
    gpio_pad_select_gpio(GPIO_LED2);
    gpio_pad_select_gpio(GPIO_LED3);

    /* Set the GPIO as a push/pull output */
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_LED_RED, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_LED1, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_LED2, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_LED3, GPIO_MODE_OUTPUT));

    int cnt = 0;

    while (1)
    {
        /* Blink off (output low) */
        ESP_ERROR_CHECK(gpio_set_level(GPIO_LED_RED, cnt & 0x01));
        ESP_ERROR_CHECK(gpio_set_level(GPIO_LED1, cnt & 0x01));
        ESP_ERROR_CHECK(gpio_set_level(GPIO_LED2, cnt & 0x02));
        ESP_ERROR_CHECK(gpio_set_level(GPIO_LED3, cnt & 0x04));
        vTaskDelay(500 / portTICK_PERIOD_MS);
        cnt++;
    }
}

void dht_task(void *pvParameter)
{
    DHT11_init(GPIO_NUM_18);
    while (1)
    {
        if (!DHT11_read().status)
            hum = DHT11_read().humidity;
        //printf("Temperature is %d \n", DHT11_read().temperature);
        //printf("Humidity is %d\n", DHT11_read().humidity);
        //printf("Status code is %d\n", DHT11_read().status);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void report_task(void *pvParameter)
{
    time_t now;
    char buffer[32];
    while (1)
    {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        time(&now);
        strftime(buffer, sizeof(buffer), "%FT%TZ", gmtime(&now));
        printf("%.32s,%.2f,%d,%2f\n",buffer,temperatur,hum,temperatur );
    }
    
}


void app_main()
{
    time_t now;
    struct tm timeinfo;
    ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();
    //sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    esp_sntp_setservername(0, "pool.ntp.org");
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
    xTaskCreate(&report_task, "report_task", 2048, NULL, 5, NULL);
}