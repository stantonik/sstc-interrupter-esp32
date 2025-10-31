/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file pwm.c
 * @brief
 *
 * @author Stanley Arnaud
 * @date 10/25/2025
 * @version 0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "pwm.h"
#include "driver/ledc.h"
#include "driver/rmt.h"
#include "esp_attr.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "hal/ledc_hal.h"
#include "rom/gpio.h"
#include "soc/gpio_sig_map.h"
#include <string.h>

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------
#define TAG "pwm"

#define PIN_OUTPUT CONFIG_INTERRUPTER_PIN_OUTPUT

#define RMT_CHANNEL RMT_CHANNEL_0
#define RMT_CLK_DIV 80 // 80 MHz / 80 = 1 MHz → 1 tick = 1 us
#define RMT_TICK_US (80 / RMT_CLK_DIV)

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES PWM_MOD_DUTY_RES_BITS
#define LEDC_FREQUENCY PWM_MOD_CARRIER_FREQ_HZ

// -----------------------------------------------------------------------------
// Private Typedef
// -----------------------------------------------------------------------------
typedef struct
{
    uint32_t period_tick;
    uint32_t pulse_width_tick;
} pwm_data_t;

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------
static QueueHandle_t low_pwm_task_queue = NULL;
static TaskHandle_t low_pwm_task_handle = NULL;

static pwm_mode_t mode = 0;
static int sig_out_idx = SIG_GPIO_OUT_IDX;
static bool enabled = false;

// -----------------------------------------------------------------------------
// Static Function Definitions
// -----------------------------------------------------------------------------
static void low_pwm_task(void *pvParams)
{
    pwm_data_t data;

    while (1)
    {
        // Try to receive new PWM data (non-blocking)
        xQueueReceive(low_pwm_task_queue, &data, 0);

        while (data.pulse_width_tick == 0 || data.period_tick == 0)
        {
            xQueueReceive(low_pwm_task_queue, &data, portMAX_DELAY);
        }

        rmt_item32_t item = {
            .level0 = 1, .level1 = 0, .duration0 = data.pulse_width_tick, .duration1 = data.pulse_width_tick};

        rmt_write_items(RMT_CHANNEL, &item, 1, false);

        vTaskDelay(pdMS_TO_TICKS((data.period_tick * RMT_TICK_US) / 1000));
    }
}

// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------
esp_err_t pwm_init(void)
{
    low_pwm_task_queue = xQueueCreate(4, sizeof(pwm_data_t));

    ledc_channel_config_t ledc_channel = {.speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = PIN_OUTPUT,
        .duty = 0,
        .hpoint = 0};
    ESP_RETURN_ON_ERROR(ledc_channel_config(&ledc_channel), TAG, "Failed to configure LEDC channel");

    rmt_config_t config = {.rmt_mode = RMT_MODE_TX,
        .channel = RMT_CHANNEL,
        .gpio_num = PIN_OUTPUT,
        .clk_div = RMT_CLK_DIV,
        .mem_block_num = 1,
        .tx_config.carrier_en = false,
        .tx_config.idle_output_en = true,
        .tx_config.idle_level = 0};
    ESP_RETURN_ON_ERROR(rmt_config(&config), TAG, "Failed to configure RMT channel");
    ESP_RETURN_ON_ERROR(rmt_driver_install(config.channel, 0, 0), TAG, "Failed to install RMT driver");

    ESP_RETURN_ON_FALSE(xTaskCreate(low_pwm_task, "low_pwm_task", 1024, NULL, 2, &low_pwm_task_handle) == pdPASS,
        ESP_ERR_NO_MEM, TAG, "Failed to start low pwm task");

    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {.speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_RETURN_ON_ERROR(ledc_timer_config(&ledc_timer), TAG, "Failed to configure LEDC timer");

    // Disable output
    gpio_config_t gpio_out_cfg = {
        .pin_bit_mask = 1ULL << PIN_OUTPUT,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
    };
    gpio_config(&gpio_out_cfg);
    gpio_matrix_out(PIN_OUTPUT, SIG_GPIO_OUT_IDX, 0, 0);
    WRITE_PERI_REG(GPIO_OUT_W1TC_REG, (1ULL << PIN_OUTPUT));

    pwm_set_mode(PWM_MODE_MANUAL);

    ESP_LOGI(TAG, "Initializaion succeeded");

    return ESP_OK;
}

esp_err_t pwm_enable(void)
{
    if (enabled) return ESP_ERR_INVALID_STATE;

    gpio_matrix_out(PIN_OUTPUT, sig_out_idx, 0, 0);
    enabled = true;
    ESP_LOGI(TAG, "Enabled");

    return ESP_OK;
}

esp_err_t pwm_disable(void)
{
    if (!enabled) return ESP_ERR_INVALID_STATE;

    gpio_matrix_out(PIN_OUTPUT, SIG_GPIO_OUT_IDX, 0, 0);
    WRITE_PERI_REG(GPIO_OUT_W1TC_REG, (1ULL << PIN_OUTPUT));
    enabled = false;
    ESP_LOGI(TAG, "Disabled");

    return ESP_OK;
}

esp_err_t pwm_manual_update(float freq_hz, uint16_t pulse_width_us)
{
    if (mode != PWM_MODE_MANUAL) return ESP_ERR_INVALID_STATE;
    if (freq_hz < 0.1 && freq_hz > 0) return ESP_ERR_INVALID_ARG;

    pwm_data_t data = {0};
    if (freq_hz == 0 || pulse_width_us == 0)
    {
        // Careful here, race condition possible
        xQueueSend(low_pwm_task_queue, &data, 0);
        rmt_tx_stop(RMT_CHANNEL);
        return ESP_OK;
    }

    data.period_tick = (uint32_t)(1.e6 / (freq_hz * RMT_TICK_US));
    data.pulse_width_tick = pulse_width_us / RMT_TICK_US;

    if (data.pulse_width_tick > data.period_tick) data.pulse_width_tick = data.period_tick;

    bool low_pwm = data.period_tick > 32767; // rmt_item32_t.duration0 overflow

    if (low_pwm)
    {
        rmt_set_tx_loop_mode(RMT_CHANNEL, false);
        xQueueSend(low_pwm_task_queue, &data, 0);
    }
    else
    {
        rmt_item32_t item = {.level0 = 1,
            .level1 = 0,
            .duration0 = data.pulse_width_tick,
            .duration1 = data.period_tick - data.pulse_width_tick};

        memset(&data, 0, sizeof(data));
        xQueueSend(low_pwm_task_queue, &data, 0);

        rmt_set_tx_loop_mode(RMT_CHANNEL, true);
        rmt_write_items(RMT_CHANNEL, &item, 1, false);
    }

    ESP_LOGI(TAG, "Manual update (%s,prf=%.1f,pd=%d)", low_pwm ? "LF":"HF", freq_hz, (int)pulse_width_us);

    return ESP_OK;
}

esp_err_t inline IRAM_ATTR pwm_modulation_update(uint8_t duty)
{
    if (mode != PWM_MODE_MODULATION) return ESP_ERR_INVALID_STATE;

    // 1. Set the integer part of the duty (same as ledc_hal_set_duty_int_part)
    LEDC.channel_group[LEDC_MODE].channel[LEDC_CHANNEL].duty.duty = duty << 4;

    // 2. Reset fade parameters (equivalent to ledc_hal_set_fade_param)
    LEDC.channel_group[LEDC_MODE].channel[LEDC_CHANNEL].conf1.duty_inc = 1;
    LEDC.channel_group[LEDC_MODE].channel[LEDC_CHANNEL].conf1.duty_num = 1;
    LEDC.channel_group[LEDC_MODE].channel[LEDC_CHANNEL].conf1.duty_cycle = 1;
    LEDC.channel_group[LEDC_MODE].channel[LEDC_CHANNEL].conf1.duty_scale = 0;

    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

    return ESP_OK;
}

esp_err_t pwm_set_mode(pwm_mode_t m)
{
    if (m == PWM_MODE_MODULATION)
    {
        if (mode == PWM_MODE_MODULATION) return ESP_ERR_INVALID_STATE;

        // Stop MANUAL
        vTaskSuspend(low_pwm_task_handle);
        rmt_tx_stop(RMT_CHANNEL);

        // Start MODULATION
        sig_out_idx = LEDC_LS_SIG_OUT0_IDX;

        ESP_LOGI(TAG, "Set mode: MODULATION");
    }
    else if (m == PWM_MODE_MANUAL)
    {
        if (mode == PWM_MODE_MANUAL) return ESP_ERR_INVALID_STATE;

        // Stop MODULATION
        ledc_stop(LEDC_MODE, LEDC_CHANNEL, 0);

        // Start MANUAL
        sig_out_idx = RMT_SIG_OUT0_IDX;
        vTaskResume(low_pwm_task_handle);
        rmt_tx_start(RMT_CHANNEL, true);

        ESP_LOGI(TAG, "Set mode: MANUAL");
    }

    if (enabled) gpio_matrix_out(PIN_OUTPUT, sig_out_idx, 0, 0);
    mode = m;

    return ESP_OK;
}

pwm_mode_t pwm_get_mode(void) { return mode; }
