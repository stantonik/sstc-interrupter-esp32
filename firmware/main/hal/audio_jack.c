/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file audio_jack.c
 * @brief
 *
 * @author Stanley Arnaud
 * @date 10/24/2025
 * @version 0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "audio_jack.h"
#include "button_gpio.h"
#include "core/event_bus.h"
#include "esp_adc/adc_continuous.h"
#include "esp_attr.h"
#include "esp_check.h"
#include "iot_button.h"

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------
#define TAG "audio_jack"

#define PIN_JACK_SW CONFIG_INTERRUPTER_PIN_JACK_SW

#define ADC_UNIT ADC_UNIT_1
#define ADC_CHANNEL ADC_CHANNEL_0
#define ADC_FREQ_HZ AUDIO_JACK_SAMPLING_RATE_HZ
#define CONV_MODE ADC_CONV_SINGLE_UNIT_1
#define OUTPUT_TYPE ADC_DIGI_OUTPUT_FORMAT_TYPE2
#define ADC_BITWIDTH AUDIO_JACK_RESOLUTION_BITS

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------
static adc_continuous_handle_t adc_handle = NULL;
static button_handle_t jack_sw = NULL;
static audio_jack_output_cb_t out_cb = NULL;

// -----------------------------------------------------------------------------
// Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Static Function Definitions
// -----------------------------------------------------------------------------
static bool IRAM_ATTR adc_conv_done_cb(adc_continuous_handle_t handle,
                                       const adc_continuous_evt_data_t *edata,
                                       void *user_data) {
    adc_digi_output_data_t *p =
        (adc_digi_output_data_t *)(edata->conv_frame_buffer);

    if (out_cb)
        out_cb(p->type2.data);

    return true;
}

static void plug_cb(void *arg, void *usr_data) {
    ESP_LOGI(TAG, "Plugged");
    event_t event_data = {.source = EVENT_SRC_AUDIO_JACK,
                          .type = AUDIO_JACK_EVENT_PLUGGED};
    event_bus_publish(&event_data);
}

static void unplug_cb(void *arg, void *usr_data) {
    ESP_LOGI(TAG, "Unplugged");
    event_t event_data = {.source = EVENT_SRC_AUDIO_JACK,
                          .type = AUDIO_JACK_EVENT_UNPLUGGED};
    event_bus_publish(&event_data);
}

// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------
esp_err_t audio_jack_init(void) {
    // Setup switch
    button_config_t btn_cfg = {0};

    button_gpio_config_t gpio_cfg = {
        .gpio_num = PIN_JACK_SW,
        .active_level = 0,
    };
    ESP_RETURN_ON_ERROR(
        iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &jack_sw), TAG,
        "Failed to create switch instance");

    iot_button_register_cb(jack_sw, BUTTON_PRESS_UP, NULL, unplug_cb, NULL);
    iot_button_register_cb(jack_sw, BUTTON_PRESS_DOWN, NULL, plug_cb, NULL);

    // Setup ADC continuous mode
    adc_continuous_handle_cfg_t handle_cfg = {
        .max_store_buf_size = 64,
        .conv_frame_size = sizeof(adc_digi_output_data_t),
    };
    ESP_RETURN_ON_ERROR(adc_continuous_new_handle(&handle_cfg, &adc_handle),
                        TAG, "Failed to create ADC handle");

    adc_digi_pattern_config_t adc_pattern = {
        .atten = ADC_ATTEN_DB_12,
        .channel = ADC_CHANNEL,
        .unit = ADC_UNIT,
        .bit_width = ADC_BITWIDTH,
    };

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = ADC_FREQ_HZ,
        .conv_mode = CONV_MODE,
        .format = OUTPUT_TYPE,
        .pattern_num = 1,
        .adc_pattern = &adc_pattern,
    };
    ESP_RETURN_ON_ERROR(adc_continuous_config(adc_handle, &dig_cfg), TAG,
                        "Failed to configure ADC");

    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = adc_conv_done_cb,
    };
    ESP_RETURN_ON_ERROR(
        adc_continuous_register_event_callbacks(adc_handle, &cbs, NULL), TAG,
        "Failed to register ADC's callback");

    ESP_LOGI(TAG, "Initializaion succeeded");

    return ESP_OK;
}

inline esp_err_t audio_jack_start_listen() {
    return adc_continuous_start(adc_handle);
}

inline esp_err_t audio_jack_stop_listen() {
    return adc_continuous_stop(adc_handle);
}

inline esp_err_t audio_jack_set_output_cb(audio_jack_output_cb_t cb) {
    out_cb = cb;
    return cb ? ESP_OK: ESP_ERR_INVALID_ARG;
}
