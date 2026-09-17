#pragma once

#include "driver/gpio.h"

#define LDR_GPIO GPIO_NUM_4
#define LED_PIN GPIO_NUM_47
#define SAMPLE_PERIOD_MS 500
#define U_FS_VOLTAGE 3.3f
#define ADC_MAX_VALUE 4095.0f

// Software fade: change the brightness by 2% every 10 ms.
#define LED_BRIGHTNESS_UPDATE_INTERVAL_MS 10
#define LED_BRIGHTNESS_STEP_PERCENT 2U

// This wiring assumes that the ADC voltage decreases when it gets darker.
// The LED turns on only below the dark threshold and turns off only above
// the light threshold. Between the thresholds it keeps its previous state.
#define LED_DARK_THRESHOLD_VOLTAGE 1.0f
#define LED_LIGHT_THRESHOLD_VOLTAGE 2.0f

// Ignore tiny brightness changes caused by ADC noise.
#define LED_BRIGHTNESS_HYSTERESIS_PERCENT 3U
