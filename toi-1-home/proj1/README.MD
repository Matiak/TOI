
common ERRORS and it's solving:
undefined portTICK_RATE_MS >> renamed as portTICK_PERIOD_MS

undefined gpio_pad_select_gpio >> add #include "rom/gpio.h" into "driver/gpio.h" (file /home/xbabac02/esp/esp-idf/components/driver/gpio/include/driver/gpio.h)
