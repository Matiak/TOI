#ifndef WIFI_ESP_PROJ
#define WIFI_ESP_PROJ
#ifdef __cplusplus
extern "C" {
#endif
void wifi_init_sta(void);
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data);
#ifdef __cplusplus
}
#endif
#endif