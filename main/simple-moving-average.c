#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "adc_reader.h"
#include "board_config.h"
#include "moving_average.h"

static const char* TAG = "LDR";

void setup_led(void) {
  gpio_reset_pin(LED_PIN);
  gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(LED_PIN, 0);
}

void app_main(void) {
  moving_average_t ldr_average;

  ESP_ERROR_CHECK(adc_reader_init(LDR_GPIO));
  moving_average_init(&ldr_average);

  setup_led();

  while (true) {
    int raw_value;

    esp_err_t err = adc_reader_read(&raw_value);

    if (err != ESP_OK) {
      ESP_LOGE(TAG, "ADC read failed: %s", esp_err_to_name(err));
      vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
      continue;
    }

    const int filtered_value = moving_average_add(&ldr_average, raw_value);
    const float u_adc =
        (float)filtered_value * (U_FS_VOLTAGE / ADC_MAX_VALUE);
    const int led_state = (u_adc < 1.0f) ? 1 : 0;

    gpio_set_level(LED_PIN, led_state);

    ESP_LOGI(TAG, "Raw: %d, SMA: %d", raw_value, filtered_value);
    ESP_LOGI(TAG, "ADC voltage: %.2f V", u_adc);
    ESP_LOGI(TAG, "LED state: %s", led_state ? "ON" : "OFF");

    vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
  }
}
