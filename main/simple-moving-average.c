#include "adc_reader.h"
#include "board_config.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_pwm.h"
#include "moving_average.h"

static const char* TAG = "LDR";

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

void app_main(void) {
  moving_average_t ldr_average;

  ESP_ERROR_CHECK(adc_reader_init(LDR_GPIO));
  ESP_ERROR_CHECK(led_pwm_init(LED_PIN));
  moving_average_init(&ldr_average);

  while (true) {
    int raw_value;

    esp_err_t err = adc_reader_read(&raw_value);

    if (err != ESP_OK) {
      ESP_LOGE(TAG, "ADC read failed: %s", esp_err_to_name(err));
      vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
      continue;
    }

    const int filtered_value = moving_average_add(&ldr_average, raw_value);
    const float u_adc = (float)filtered_value * (U_FS_VOLTAGE / ADC_MAX_VALUE);
    const uint32_t brightness = voltage_to_brightness_percent(u_adc);

    esp_err_t led_err = led_pwm_set_brightness(brightness, LED_FADE_TIME_MS);
    if (led_err != ESP_OK) {
      ESP_LOGE(TAG, "LED fade failed: %s", esp_err_to_name(led_err));
    }

    ESP_LOGI(TAG, "Raw: %d, SMA: %d", raw_value, filtered_value);
    ESP_LOGI(TAG, "ADC voltage: %.2f V", u_adc);
    ESP_LOGI(TAG, "LED brightness: %lu%%", (unsigned long)brightness);

    vTaskDelay(pdMS_TO_TICKS(1));
  }
}
