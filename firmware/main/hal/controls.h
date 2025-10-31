/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file controls.h
 * @brief
 *
 *
 *
 * @author Stanley Arnaud
 * @date 10/24/2025
 * @version 0
 */

#ifndef CONTROLS_H
#define CONTROLS_H

// clang-format off
#ifdef __cplusplus
extern "C" 
{
#endif

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "esp_err.h"
#include "freertos/idf_additions.h"

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Type Definitions
// -----------------------------------------------------------------------------
#define CONTROLS_FLAG_RE_PRESSED        (1 << 0)
#define CONTROLS_FLAG_TRIGGER_PRESSED   (1 << 1)

typedef enum
{
    CONTROLS_EVENT_RE_BTN_PRESSED,
    CONTROLS_EVENT_RE_BTN_CLICKED,
    CONTROLS_EVENT_RE_BTN_LONG_PRESSED,
    CONTROLS_EVENT_RE_BTN_RELEASED,
    CONTROLS_EVENT_RE_CHANGED,

    CONTROLS_EVENT_TRIGGER_PRESSED,
    CONTROLS_EVENT_TRIGGER_RELEASED,
} controls_event_t;

// -----------------------------------------------------------------------------
// Inline Function Definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Function Declarations
// -----------------------------------------------------------------------------
esp_err_t controls_init(void);
uint8_t controls_get_state(void);

#ifdef __cplusplus
}
#endif
// clang-format on

#endif /* !CONTROLS_H */
