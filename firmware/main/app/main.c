/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file main.c
 * @brief
 *
 * @author Stanley Arnaud
 * @date 10/25/2025
 * @version 0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "app/clients/usb_midi.h"
#include "app/gui/knobs.h"
#include "clients/usb_midi.h"
#include "core/event_bus.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "gui/menu.h"
#include "hal/audio_jack.h"
#include "hal/controls.h"
#include "hal/display.h"
#include "hal/pwm.h"
#include "hal/synth.h"
#include "hal/usb.h"

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------
#define TAG "main"

#define RETURN_ON_ERROR(x)                                                                                             \
    {                                                                                                                  \
        esp_err_t ret = (x);                                                                                           \
        if (ret != ESP_OK) return;                                                                                     \
    }

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Static Function Definitions
// -----------------------------------------------------------------------------
static void midi_on_receive_cb(midi_message_t msg)
{
    if (menu_get_mode() != MENU_MODE_MIDI) return;

    static synth_note_t synth_note = {0};
    synth_note.note = msg.note % 12;
    synth_note.octave = -2 + msg.note / 12;

    if (msg.state == 1)
    {
        synth_play_note(synth_note);

        midi_msg_parsed_t parsed_msg = usb_midi_parse_msg(&msg);

        char header[16];
        snprintf(header, 16, "%s%d", parsed_msg.note, parsed_msg.octave);
        menu_set_header_text(header);
    }
    else
    {
        synth_stop_note(synth_note);
        menu_set_header_text("");
    }
}

static void knobs_on_change_cb(knobs_mask_t ranges, const int16_t *values[])
{
    static float prf = 0;
    static uint16_t pd = 0;

    if (ranges & (1 << KNOB_PRF))
    {
        prf = *values[KNOB_PRF];
    }
    if (ranges & (1 << KNOB_PD))
    {
        pd = *values[KNOB_PD];
    }

    ESP_LOGI(TAG, "prf=%d, pd=%d", (int)prf, (int)pd);
    pwm_manual_update(prf, pd);
}

static IRAM_ATTR void audio_jack_out_cb(uint16_t value)
{
    uint8_t dt = 0;
    dt = value * PWM_MOD_DUTY_MAX / AUDIO_JACK_OUT_MAX;
    pwm_modulation_update(dt);
}

static IRAM_ATTR void synth_on_sampling_cb(uint16_t value)
{
    uint8_t dt = 0;
    dt = value * PWM_MOD_DUTY_MAX / SYNTH_OUT_MAX;
    pwm_modulation_update(dt);
}

// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------
void app_main(void)
{
    ESP_LOGI(TAG, "Firmware starting...");

    // Initialize Serial bridge at first
    RETURN_ON_ERROR(usb_init_serial());

    // Create event queue
    RETURN_ON_ERROR(event_bus_init());

    // Initialize hardware
    RETURN_ON_ERROR(controls_init());
    RETURN_ON_ERROR(audio_jack_init());
    RETURN_ON_ERROR(display_init());
    RETURN_ON_ERROR(pwm_init());
    audio_jack_set_output_cb(audio_jack_out_cb);

    uint8_t ctrl_state = controls_get_state();

    // Initialize GUI
    RETURN_ON_ERROR(menu_init());
    knobs_set_on_change_cb(knobs_on_change_cb);
    RETURN_ON_ERROR(knobs_init());

    // Check if trigger was pressed on boot
    if (ctrl_state & CONTROLS_FLAG_TRIGGER_PRESSED)
    {
        menu_display_msg_box("Release trigger\non boot!", 0);
        ESP_LOGI(TAG, "Triggered on boot! Please release it.");
        while (ctrl_state & CONTROLS_FLAG_TRIGGER_PRESSED)
        {
            ctrl_state = controls_get_state();
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        menu_hide_msg_box();
    }

    // Configure USB Host Library if encoder switch was not pressed on boot
    if (ctrl_state & CONTROLS_FLAG_RE_PRESSED)
    {
        menu_display_msg_box("Debug mode\n(MIDI disabled)", 1500);
        ESP_LOGI(TAG, "Booted in debug mode (MIDI disabled)");
    }
    else
    {
        ESP_LOGI(TAG, "Booted in normal operation");
        RETURN_ON_ERROR(usb_free_serial());
        if (usb_init_host() != ESP_OK)
        {
            usb_init_serial();
            ESP_LOGW(TAG, "Could not install USB host");
        }

        RETURN_ON_ERROR(usb_midi_init());
        usb_midi_set_on_receive_cb(midi_on_receive_cb);

        RETURN_ON_ERROR(synth_init());
        synth_set_on_sampling_cb(synth_on_sampling_cb);
    }

    event_t e;
    while (1)
    {
        event_bus_dispatch(&e, portMAX_DELAY);

        if (e.source == EVENT_SRC_CONTROLS)
        {
            switch (e.type)
            {
            case CONTROLS_EVENT_TRIGGER_PRESSED:
                ESP_LOGI(TAG, "Armed");
                menu_set_state(MENU_STATE_ARMED);
                pwm_enable();
                break;
            case CONTROLS_EVENT_TRIGGER_RELEASED:
                ESP_LOGI(TAG, "Disarmed");
                menu_set_state(MENU_STATE_IDLE);
                pwm_disable();
                break;
            default:
                break;
            }
        }
        else if (e.source == EVENT_SRC_AUDIO_JACK)
        {
            switch (e.type)
            {
            case AUDIO_JACK_EVENT_PLUGGED:
                if (menu_get_mode() != MENU_MODE_MANUAL) break;
                ESP_LOGI(TAG, "Line-IN mode");
                menu_set_mode(MENU_MODE_AUDIO_JACK, true);
                audio_jack_start_listen();
                pwm_set_mode(PWM_MODE_MODULATION);
                break;
            case AUDIO_JACK_EVENT_UNPLUGGED:
                if (menu_get_mode() != MENU_MODE_AUDIO_JACK) break;
                ESP_LOGI(TAG, "Manual mode");
                menu_set_mode(MENU_MODE_MANUAL, true);
                audio_jack_stop_listen();
                pwm_set_mode(PWM_MODE_MANUAL);
                break;
            default:
                break;
            }
        }
        else if (e.source == EVENT_SRC_USB_MIDI)
        {
            switch (e.type)
            {
            case USB_MIDI_EVENT_CONNECTED:
                if (menu_get_mode() != MENU_MODE_MANUAL) break;
                ESP_LOGI(TAG, "MIDI mode");
                menu_set_mode(MENU_MODE_MIDI, true);
                pwm_set_mode(PWM_MODE_MODULATION);
                synth_enable();
                break;
            case USB_MIDI_EVENT_DISCONNECTED:
                if (menu_get_mode() != MENU_MODE_MIDI) break;
                ESP_LOGI(TAG, "Manual mode");
                menu_set_mode(MENU_MODE_MANUAL, true);
                synth_disable();
                pwm_set_mode(PWM_MODE_MANUAL);
                break;
            default:
                break;
            }
        }
    }

    ESP_LOGI(TAG, "Firmware exited");
}
