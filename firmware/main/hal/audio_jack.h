/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file audio_jack.h
 * @brief
 *
 *
 *
 * @author Stanley Arnaud
 * @date 10/24/2025
 * @version 0
 */

#ifndef AUDIO_JACK_H
#define AUDIO_JACK_H

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
#define AUDIO_JACK_RESOLUTION_BITS 12
#define AUDIO_JACK_OUT_MAX     ((1U << AUDIO_JACK_RESOLUTION_BITS) - 1)
#define AUDIO_JACK_SAMPLING_RATE_HZ (16000)

// -----------------------------------------------------------------------------
// Type Definitions
// -----------------------------------------------------------------------------
typedef enum
{
    AUDIO_JACK_EVENT_PLUGGED,
    AUDIO_JACK_EVENT_UNPLUGGED,
} audio_jack_event_t;

typedef void (* audio_jack_output_cb_t)(uint16_t value);

// -----------------------------------------------------------------------------
// Inline Function Definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Function Declarations
// -----------------------------------------------------------------------------
esp_err_t audio_jack_init(void);

esp_err_t audio_jack_start_listen(void);
esp_err_t audio_jack_stop_listen(void);
esp_err_t audio_jack_set_output_cb(audio_jack_output_cb_t cb);

#ifdef __cplusplus
}
#endif
// clang-format on

#endif /* !AUDIO_JACK_H */
