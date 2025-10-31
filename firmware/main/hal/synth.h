/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file synth.h
 * @brief
 *
 *
 *
 * @author Stanley Arnaud
 * @date 10/31/2025
 * @version 0
 */

#ifndef SYNTH_H
#define SYNTH_H

// clang-format off
#ifdef __cplusplus
extern "C" 
{
#endif

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "esp_err.h"

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------
#define SYNTH_MAX_CHORD_SIZE (8)
#define SYNTH_SAMPLING_RATE_HZ (16000)
#define SYNTH_RESOLUTION_BITS 16
#define SYNTH_OUT_MAX ((1U << SYNTH_RESOLUTION_BITS) - 1)
#define SYNTH_OUT_SILENCE (SYNTH_OUT_MAX / 2)

// -----------------------------------------------------------------------------
// Type Definitions
// -----------------------------------------------------------------------------
typedef enum
{
    SYNTH_NOTE_C = 0,
    SYNTH_NOTE_C_SHARP,
    SYNTH_NOTE_D,
    SYNTH_NOTE_D_SHARP,
    SYNTH_NOTE_E,
    SYNTH_NOTE_F,
    SYNTH_NOTE_F_SHARP,
    SYNTH_NOTE_G,
    SYNTH_NOTE_G_SHARP,
    SYNTH_NOTE_A,
    SYNTH_NOTE_A_SHARP,
    SYNTH_NOTE_B
} synth_note_name_t;

typedef struct
{
    int8_t octave;
    synth_note_name_t note;
} synth_note_t;

// -----------------------------------------------------------------------------
// Inline Function Definitions
// -----------------------------------------------------------------------------
typedef void (* synth_on_sampling_cb_t)(uint16_t value);

// -----------------------------------------------------------------------------
// Function Declarations
// -----------------------------------------------------------------------------
esp_err_t synth_init(void);
esp_err_t synth_enable(void);
esp_err_t synth_disable(void);
esp_err_t synth_play_note(synth_note_t note);
esp_err_t synth_stop_note(synth_note_t note);

esp_err_t synth_set_on_sampling_cb(synth_on_sampling_cb_t cb);

#ifdef __cplusplus
}
#endif
// clang-format on

#endif /* !SYNTH_H */
