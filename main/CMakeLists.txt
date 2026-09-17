idf_component_register(
    SRCS "simple-moving-average.c" "adc_reader.c" "moving_average.c" "led_pwm.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_adc esp_driver_gpio esp_driver_ledc esp_timer
)
