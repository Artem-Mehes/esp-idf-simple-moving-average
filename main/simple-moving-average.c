#include <stdbool.h>
#include <stdint.h>

#include "adc_reader.h"
#include "board_config.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_pwm.h"
#include "moving_average.h"

static const char* TAG = "LDR";

typedef struct {
  bool is_enabled;
  uint32_t current_brightness;
  uint32_t target_brightness;
  int64_t last_sample_time_us;
  int64_t last_brightness_update_time_us;
} led_control_state_t;

#define MILLISECONDS_TO_MICROSECONDS 1000LL

static uint32_t voltage_to_brightness_percent(float voltage) {
  if (voltage <= 0.0f) {
    return 100;
  }

  if (voltage >= U_FS_VOLTAGE) {
    return 0;
  }

  const float darkness = (U_FS_VOLTAGE - voltage) / U_FS_VOLTAGE;
  return (uint32_t)(darkness * 100.0f);
}

static void update_led_enabled_state(led_control_state_t* state,
                                     float voltage) {
  if (!state->is_enabled && voltage <= LED_DARK_THRESHOLD_VOLTAGE) {
    // It has become dark enough to turn the LED on.
    state->is_enabled = true;
    ESP_LOGI(TAG, "Dark threshold crossed: LED enabled");
  } else if (state->is_enabled && voltage >= LED_LIGHT_THRESHOLD_VOLTAGE) {
    // It has become light enough to turn the LED off.
    state->is_enabled = false;
    ESP_LOGI(TAG, "Light threshold crossed: LED disabled");
  }

  // Between the two thresholds is the hysteresis zone. The previous state
  // is deliberately kept, so ADC noise cannot rapidly toggle the LED.
}

static bool brightness_change_is_significant(uint32_t current_target,
                                             uint32_t new_brightness) {
  // Always allow a complete switch-off, even if the last value was only 1-2%.
  if (new_brightness == 0) {
    return current_target != 0;
  }

  const uint32_t difference = current_target > new_brightness
                                  ? current_target - new_brightness
                                  : new_brightness - current_target;

  return difference >= LED_BRIGHTNESS_HYSTERESIS_PERCENT;
}

static void update_target_brightness(led_control_state_t* state,
                                     uint32_t new_target) {
  if (brightness_change_is_significant(state->target_brightness, new_target)) {
    state->target_brightness = new_target;
  }
}

static esp_err_t update_current_brightness(led_control_state_t* state) {
  if (state->current_brightness < state->target_brightness) {
    const uint32_t remaining =
        state->target_brightness - state->current_brightness;
    const uint32_t step = remaining < LED_BRIGHTNESS_STEP_PERCENT
                              ? remaining
                              : LED_BRIGHTNESS_STEP_PERCENT;
    state->current_brightness += step;
  } else if (state->current_brightness > state->target_brightness) {
    const uint32_t remaining =
        state->current_brightness - state->target_brightness;
    const uint32_t step = remaining < LED_BRIGHTNESS_STEP_PERCENT
                              ? remaining
                              : LED_BRIGHTNESS_STEP_PERCENT;
    state->current_brightness -= step;
  } else {
    return ESP_OK;
  }

  return led_pwm_set_brightness(state->current_brightness);
}

void app_main(void) {
  moving_average_t ldr_average;
  led_control_state_t led_state = {
      .is_enabled = false,
      .current_brightness = 0,
      .target_brightness = 0,
      .last_sample_time_us = -SAMPLE_PERIOD_MS * MILLISECONDS_TO_MICROSECONDS,
      .last_brightness_update_time_us = 0,
  };

  ESP_ERROR_CHECK(adc_reader_init(LDR_GPIO));
  ESP_ERROR_CHECK(led_pwm_init(LED_PIN));
  moving_average_init(&ldr_average);

  while (true) {
    const int64_t now_us = esp_timer_get_time();

    if (now_us - led_state.last_sample_time_us >=
        SAMPLE_PERIOD_MS * MILLISECONDS_TO_MICROSECONDS) {
      led_state.last_sample_time_us = now_us;

      int raw_value;
      esp_err_t err = adc_reader_read(&raw_value);

      if (err != ESP_OK) {
        ESP_LOGE(TAG, "ADC read failed: %s", esp_err_to_name(err));
      } else {
        const int filtered_value = moving_average_add(&ldr_average, raw_value);
        const float u_adc =
            (float)filtered_value * (U_FS_VOLTAGE / ADC_MAX_VALUE);

        update_led_enabled_state(&led_state, u_adc);

        const uint32_t mapped_brightness = voltage_to_brightness_percent(u_adc);
        const uint32_t new_target =
            led_state.is_enabled ? mapped_brightness : 0;
        update_target_brightness(&led_state, new_target);

        ESP_LOGI(TAG, "Raw: %d, SMA: %d, voltage: %.2f V", raw_value,
                 filtered_value, u_adc);
        ESP_LOGI(TAG, "LED: %s, current: %lu%%, target: %lu%%",
                 led_state.is_enabled ? "enabled" : "disabled",
                 (unsigned long)led_state.current_brightness,
                 (unsigned long)led_state.target_brightness);
      }
    }

    if (now_us - led_state.last_brightness_update_time_us >=
        LED_BRIGHTNESS_UPDATE_INTERVAL_MS * MILLISECONDS_TO_MICROSECONDS) {
      led_state.last_brightness_update_time_us = now_us;

      esp_err_t led_err = update_current_brightness(&led_state);
      if (led_err != ESP_OK) {
        ESP_LOGE(TAG, "LED brightness update failed: %s",
                 esp_err_to_name(led_err));
      }
    }

    vTaskDelay(1);
  }
}
