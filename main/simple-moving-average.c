#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "adc_reader.h"
#include "board_config.h"

static const char* TAG = "LDR";

void setup_led(void) {
  gpio_reset_pin(LED_PIN);
  gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(LED_PIN, 0);
}

void app_main(void) {
  ESP_ERROR_CHECK(adc_reader_init(LDR_GPIO));

  setup_led();

  while (true) {
    int raw_value;

    esp_err_t err = adc_reader_read(&raw_value);

    const float u_adc = (float)raw_value * (3.3f / 4095.0f);
    const int led_state = (u_adc < 1.0f) ? 1 : 0;
    gpio_set_level(LED_PIN, led_state);

    if (err == ESP_OK) {
      ESP_LOGI(TAG, "ADC voltage: %.2f V", u_adc);
      ESP_LOGI(TAG, "LED state: %s", led_state ? "ON" : "OFF");
      ESP_LOGI(TAG, "Raw value: %d", raw_value);
    } else {
      ESP_LOGE(TAG, "ADC read failed: %s", esp_err_to_name(err));
    }

    vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
  }
}
