#include "adc_reader.h"

#include "esp_adc/adc_oneshot.h"

static adc_oneshot_unit_handle_t s_adc_handle;
static adc_channel_t s_adc_channel;

esp_err_t adc_reader_init(gpio_num_t gpio_num) {
  if (s_adc_handle != NULL) {
    return ESP_ERR_INVALID_STATE;
  }

  adc_unit_t unit;
  esp_err_t err = adc_oneshot_io_to_channel(gpio_num, &unit, &s_adc_channel);
  if (err != ESP_OK) {
    return err;
  }

  const adc_oneshot_unit_init_cfg_t unit_config = {
      .unit_id = unit,
  };

  err = adc_oneshot_new_unit(&unit_config, &s_adc_handle);
  if (err != ESP_OK) {
    return err;
  }

  const adc_oneshot_chan_cfg_t channel_config = {
      .bitwidth = ADC_BITWIDTH_DEFAULT,
      .atten = ADC_ATTEN_DB_12,
  };

  err = adc_oneshot_config_channel(s_adc_handle, s_adc_channel,
                                   &channel_config);
  if (err != ESP_OK) {
    adc_oneshot_del_unit(s_adc_handle);
    s_adc_handle = NULL;
  }

  return err;
}

esp_err_t adc_reader_read(int *raw_value) {
  if (raw_value == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  if (s_adc_handle == NULL) {
    return ESP_ERR_INVALID_STATE;
  }

  return adc_oneshot_read(s_adc_handle, s_adc_channel, raw_value);
}
