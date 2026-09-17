#pragma once

#include <stdint.h>

#include "driver/gpio.h"
#include "esp_err.h"

esp_err_t led_pwm_init(gpio_num_t gpio_num);
esp_err_t led_pwm_set_brightness(uint32_t brightness_percent);
