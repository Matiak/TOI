#ifndef DHT_ESP_PROJ
#define DHT_ESP_PROJ
#ifdef __cplusplus
extern "C" {
#endif
extern int hum;

void dht_task(void *pvParameter);
#ifdef __cplusplus
}
#endif
#endif