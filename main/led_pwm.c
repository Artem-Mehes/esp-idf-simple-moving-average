#include "led_pwm.h"

#include <stdbool.h>

#include "driver/ledc.h"

#define LED_PWM_MODE LEDC_LOW_SPEED_MODE
#define LED_PWM_TIMER LEDC_TIMER_0
#define LED_PWM_CHANNEL LEDC_CHANNEL_0
#define LED_PWM_FREQUENCY_HZ 5000
#define LED_PWM_DUTY_RESOLUTION LEDC_TIMER_10_BIT
#define LED_PWM_MAX_DUTY 1023
#define LED_PWM_MAX_BRIGHTNESS_PERCENT 100

static bool s_initialized;
static uint32_t s_target_duty;

static esp_err_t led_pwm_fade_to(uint32_t target_duty,
                                 uint32_t duration_ms) {
  if (!s_initialized) {
    return ESP_ERR_INVALID_STATE;
  }

  // Do not restart the same fade on every pass through the main loop.
  if (target_duty == s_target_duty) {
    return ESP_OK;
  }

  esp_err_t err = ledc_set_fade_time_and_start(
      LED_PWM_MODE, LED_PWM_CHANNEL, target_duty, duration_ms,
      LEDC_FADE_NO_WAIT);

  if (err == ESP_OK) {
    s_target_duty = target_duty;
  }

  return err;
}

esp_err_t led_pwm_init(gpio_num_t gpio_num) {
  if (s_initialized) {
    return ESP_ERR_INVALID_STATE;
  }

  const ledc_timer_config_t timer_config = {
      .speed_mode = LED_PWM_MODE,
      .duty_resolution = LED_PWM_DUTY_RESOLUTION,
      .timer_num = LED_PWM_TIMER,
      .freq_hz = LED_PWM_FREQUENCY_HZ,
      .clk_cfg = LEDC_AUTO_CLK,
  };

  esp_err_t err = ledc_timer_config(&timer_config);
  if (err != ESP_OK) {
    return err;
  }

  const ledc_channel_config_t channel_config = {
      .gpio_num = gpio_num,
      .speed_mode = LED_PWM_MODE,
      .channel = LED_PWM_CHANNEL,
      .intr_type = LEDC_INTR_DISABLE,
      .timer_sel = LED_PWM_TIMER,
      .duty = 0,
      .hpoint = 0,
  };

  err = ledc_channel_config(&channel_config);
  if (err != ESP_OK) {
    return err;
  }

  err = ledc_fade_func_install(0);
  if (err != ESP_OK) {
    return err;
  }

  s_target_duty = 0;
  s_initialized = true;
  return ESP_OK;
}

esp_err_t led_pwm_set_brightness(uint32_t brightness_percent,
                                 uint32_t duration_ms) {
  if (brightness_percent > LED_PWM_MAX_BRIGHTNESS_PERCENT) {
    return ESP_ERR_INVALID_ARG;
  }

  const uint32_t target_duty =
      brightness_percent * LED_PWM_MAX_DUTY /
      LED_PWM_MAX_BRIGHTNESS_PERCENT;

  return led_pwm_fade_to(target_duty, duration_ms);
}
