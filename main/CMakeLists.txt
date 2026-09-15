idf_component_register(
    SRCS "simple-moving-average.c"
    INCLUDE_DIRS "."
    REQUIRES esp_adc esp_driver_gpio
)
