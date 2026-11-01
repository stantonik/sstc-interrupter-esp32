/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file controls.c
 * @brief
 *
 * @author Stanley Arnaud
 * @date 10/24/2025
 * @version 0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "controls.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "button_gpio.h"
#include "core/event_bus.h"
#include "encoder.h"
#include "esp_check.h"
#include "iot_button.h"

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------
#define TAG "controls"

#define PIN_TRIGGER CONFIG_INTERRUPTER_PIN_TRIGGER

#define PIN_RE_A CONFIG_INTERRUPTER_PIN_RE_A
#define PIN_RE_B CONFIG_INTERRUPTER_PIN_RE_B
#define PIN_RE_SW CONFIG_INTERRUPTER_PIN_RE_SW

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------
static rotary_encoder_t re = {0};
static QueueHandle_t re_queue = NULL;

static button_handle_t trigger_btn = NULL;

static volatile uint8_t state = 0;

// -----------------------------------------------------------------------------
// Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Static Function Definitions
// -----------------------------------------------------------------------------
static void button_down_cb(void *arg, void *usr_data)
{
    button_handle_t btn = (button_handle_t)arg;
    event_t event = {.source = EVENT_SRC_CONTROLS};

    if (btn == trigger_btn)
    {
        ESP_LOGI(TAG, "Trigger pressed");
        state |= CONTROLS_FLAG_TRIGGER_PRESSED;
        event.type = CONTROLS_EVENT_TRIGGER_PRESSED;
    }

    event_bus_publish(&event);
}

static void button_up_cb(void *arg, void *usr_data)
{
    button_handle_t btn = (button_handle_t)arg;
    event_t event = {.source = EVENT_SRC_CONTROLS};

    if (btn == trigger_btn)
    {
        ESP_LOGI(TAG, "Trigger released");
        state &= ~CONTROLS_FLAG_TRIGGER_PRESSED;
        event.type = CONTROLS_EVENT_TRIGGER_RELEASED;
    }

    event_bus_publish(&event);
}

static void re_task(void *pvParam)
{
    rotary_encoder_event_t re_event;
    event_t event = {.source = EVENT_SRC_CONTROLS};

    while (1)
    {
        xQueueReceive(re_queue, &re_event, portMAX_DELAY);

        switch (re_event.type)
        {
        case RE_ET_BTN_PRESSED:
            event.type = CONTROLS_EVENT_RE_BTN_PRESSED;
            state |= CONTROLS_FLAG_RE_PRESSED;
            break;
        case RE_ET_BTN_RELEASED:
            event.type = CONTROLS_EVENT_RE_BTN_RELEASED;
            state &= ~CONTROLS_FLAG_RE_PRESSED;
            break;
        case RE_ET_BTN_CLICKED:
            event.type = CONTROLS_EVENT_RE_BTN_CLICKED;
            break;
        case RE_ET_BTN_LONG_PRESSED:
            event.type = CONTROLS_EVENT_RE_BTN_LONG_PRESSED;
            break;
        case RE_ET_CHANGED:
            event.type = CONTROLS_EVENT_RE_CHANGED;
            event.value = re_event.diff;
            break;
        default:
            break;
        }

        event_bus_publish(&event);
    }
}

// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------
esp_err_t controls_init(void)
{
    // Rotary encoder
    re_queue = xQueueCreate(4, sizeof(rotary_encoder_event_t));
    ESP_RETURN_ON_ERROR(rotary_encoder_init(re_queue), TAG, "Failed to initialize rotary encoder driver");

    re.pin_a = PIN_RE_A;
    re.pin_b = PIN_RE_B;
    re.pin_btn = PIN_RE_SW;

    ESP_RETURN_ON_ERROR(rotary_encoder_add(&re), TAG, "Failed to create rotary encoder instance");

    ESP_RETURN_ON_FALSE(xTaskCreate(re_task, TAG, configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL) == pdPASS,
        ESP_ERR_NO_MEM, TAG, "Failed to create encoder task");

    // Trigger (switch and push button)
    button_config_t btn_cfg = {0};
    button_gpio_config_t gpio_cfg = {
        .gpio_num = PIN_TRIGGER,
        .active_level = 0,
    };
    ESP_RETURN_ON_ERROR(
        iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &trigger_btn), TAG, "Failed to create trigger button instance");

    iot_button_register_cb(trigger_btn, BUTTON_PRESS_UP, NULL, button_up_cb, (void *)trigger_btn);
    iot_button_register_cb(trigger_btn, BUTTON_PRESS_DOWN, NULL, button_down_cb, (void *)trigger_btn);

    ESP_LOGI(TAG, "Initializaion succeeded");

    return ESP_OK;
}

uint8_t controls_get_state(void) { return state; }
