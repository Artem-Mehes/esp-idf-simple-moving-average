#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LDR_GPIO GPIO_NUM_4
#define LED_PIN GPIO_NUM_47

static const char* TAG = "LDR";

void app_main(void) {
  adc_unit_t unit;
  adc_channel_t channel;
  adc_oneshot_unit_handle_t adc_handle;

  ESP_ERROR_CHECK(adc_oneshot_io_to_channel(LDR_GPIO, &unit, &channel));

  const adc_oneshot_unit_init_cfg_t unit_config = {
      .unit_id = unit,
  };

  const adc_oneshot_chan_cfg_t channel_config = {
      .bitwidth = ADC_BITWIDTH_DEFAULT,
      .atten = ADC_ATTEN_DB_12,
  };

  ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_config, &adc_handle));

  ESP_ERROR_CHECK(
      adc_oneshot_config_channel(adc_handle, channel, &channel_config));

  gpio_reset_pin(LED_PIN);
  gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(LED_PIN, 0);

  while (true) {
    int raw_value;

    esp_err_t err = adc_oneshot_read(adc_handle, channel, &raw_value);

    if (err == ESP_OK) {
      ESP_LOGI(TAG, "Raw value: %d", raw_value);
    } else {
      ESP_LOGE(TAG, "ADC read failed: %s", esp_err_to_name(err));
    }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}