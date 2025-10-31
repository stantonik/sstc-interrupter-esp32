/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file synth.c
 * @brief
 *
 * @author Stanley Arnaud
 * @date 10/31/2025
 * @version 0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "synth.h"
#include "driver/gptimer.h"
#include "esp_attr.h"
#include "esp_check.h"
#include <math.h>

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------
#define TAG "synth"

#define SIN_TABLE_SIZE 256
#define SIN_TABLE_MAX ((1 << (SYNTH_RESOLUTION_BITS - 1)) - 1)
#define SIN_TABLE_MIN (-(1 << (SYNTH_RESOLUTION_BITS - 1)))
#define PHASE_BITS 32 // DDS accumulator precision

#define OUT_HALF (SYNTH_OUT_MAX / 2)

#define GPTIMER_CLK_SRC GPTIMER_CLK_SRC_DEFAULT
#define GPTIMER_FREQ_HZ (1000000)
#define GPTIMER_ALARM_CNT (GPTIMER_FREQ_HZ / SYNTH_SAMPLING_RATE_HZ)

// -----------------------------------------------------------------------------
// Private Typedefs
// -----------------------------------------------------------------------------
typedef struct
{
    uint8_t code : 7;
    uint8_t active : 1;
    uint32_t phase_acc;
    uint32_t phase_inc;
} note_data_t;

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------
static gptimer_handle_t gptimer = NULL;

static int16_t sin_table[SIN_TABLE_SIZE] = {0};
static note_data_t active_notes[SYNTH_MAX_CHORD_SIZE] = {0};
static uint8_t active_notes_cnt = 0;

static synth_on_sampling_cb_t on_sampling_cb = NULL;

// -----------------------------------------------------------------------------
// Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Static Function Definitions
// -----------------------------------------------------------------------------
static IRAM_ATTR bool gptimer_on_alarm(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    int32_t mixed = 0;

    for (int i = 0; i < SYNTH_MAX_CHORD_SIZE; ++i)
    {
        note_data_t *note = &active_notes[i];
        if (note->active == 0) continue;

        // advance phase
        note->phase_acc += note->phase_inc;
        // take the upper bits as table index
        uint16_t index = note->phase_acc >> (PHASE_BITS - 8); // 8 bits for 256-entry table

        mixed += sin_table[index];
    }

    uint16_t out = SYNTH_OUT_SILENCE;
    if (active_notes_cnt > 0)
    {
        mixed /= active_notes_cnt;

        if (mixed > SIN_TABLE_MAX)
            mixed = SIN_TABLE_MAX;
        else if (mixed < SIN_TABLE_MIN)
            mixed = SIN_TABLE_MIN;

        out = (uint16_t)(mixed + OUT_HALF);
    }

    if (on_sampling_cb) on_sampling_cb(out);

    return true;
}

// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------
esp_err_t synth_init(void)
{
    // Initialize GPTimer
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC, .direction = GPTIMER_COUNT_UP, .resolution_hz = GPTIMER_FREQ_HZ};
    ESP_RETURN_ON_ERROR(gptimer_new_timer(&timer_config, &gptimer), TAG, "Failed to create GPTimer");

    gptimer_event_callbacks_t cbs = {.on_alarm = gptimer_on_alarm};
    ESP_RETURN_ON_ERROR(gptimer_register_event_callbacks(gptimer, &cbs, NULL), TAG, "");
    ESP_RETURN_ON_ERROR(gptimer_enable(gptimer), TAG, "");

    gptimer_alarm_config_t alarm_config = {.alarm_count = GPTIMER_ALARM_CNT, .flags.auto_reload_on_alarm = true};
    ESP_RETURN_ON_ERROR(gptimer_set_alarm_action(gptimer, &alarm_config), TAG, "");

    // Fill sin table
    for (int i = 0; i < SIN_TABLE_SIZE; i++)
    {
        float s = sinf(2.0f * M_PI * i / SIN_TABLE_SIZE);
        // scale -1..+1 -> -32768..32767
        sin_table[i] = (int16_t)(s * SIN_TABLE_MAX);
    }

    ESP_LOGI(TAG, "Initialization succeeded");

    return ESP_OK;
}

esp_err_t synth_play_note(synth_note_t note)
{
    if (active_notes_cnt >= SYNTH_MAX_CHORD_SIZE)
    {
        ESP_LOGW(TAG, "Max active note count reached");
        return ESP_ERR_NO_MEM;
    }

    uint8_t code = (note.octave + 2) * 12 + note.note;

    for (int i = 0; i < SYNTH_MAX_CHORD_SIZE; ++i)
    {
        note_data_t *note = &active_notes[i];
        if (note->active == 0)
        {
            float freq_hz = 440.0f * powf(2.0f, (code - 69) / 12.0f);

            note->code = code;
            note->phase_inc = (uint32_t)((freq_hz / (float)SYNTH_SAMPLING_RATE_HZ) * (1ULL << PHASE_BITS) + 0.5f);
            note->active = 1;

            active_notes_cnt++;
            break;
        }
    }

    return ESP_OK;
}

esp_err_t synth_stop_note(synth_note_t note)
{
    if (active_notes_cnt <= 0) return ESP_ERR_INVALID_STATE;

    uint8_t code = (note.octave + 2) * 12 + note.note;
    for (int i = 0; i < SYNTH_MAX_CHORD_SIZE; ++i)
    {
        note_data_t *note = &active_notes[i];
        if (note->code == code && note->active == 1)
        {
            // stop
            note->active = 0;

            active_notes_cnt--;
            break;
        }
    }

    return ESP_OK;
}

esp_err_t synth_enable(void) { return gptimer_start(gptimer); }

esp_err_t synth_disable(void) { return gptimer_stop(gptimer); }

esp_err_t synth_set_on_sampling_cb(synth_on_sampling_cb_t cb)
{
    on_sampling_cb = cb;
    return cb ? ESP_OK : ESP_ERR_INVALID_ARG;
}
