#pragma once

#include "driver/gpio.h"
#include "esp_err.h"

esp_err_t adc_reader_init(gpio_num_t gpio_num);
esp_err_t adc_reader_read(int *raw_value);
