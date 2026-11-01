/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file event_bus.h
 * @brief
 *
 *
 *
 * @author Stanley Arnaud
 * @date 10/25/2025
 * @version 0
 */

#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#ifdef __cplusplus
extern "C" {
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
// clang-format off
typedef enum
{
    EVENT_SRC_CONTROLS,
    EVENT_SRC_USB_MIDI,
    EVENT_SRC_AUDIO_JACK,
    EVENT_SRC_COUNT
} event_source_t;

typedef struct
{
    event_source_t source;
    uint8_t type;
    uint32_t value;
    void *data;
} event_t;

typedef void (*event_callback_t)(const event_t *event, void *user_data);

// -----------------------------------------------------------------------------
// Inline Function Definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Function Declarations
// -----------------------------------------------------------------------------
esp_err_t event_bus_init(void);
esp_err_t event_bus_publish(const event_t *event);
esp_err_t event_bus_subscribe(event_source_t source, event_callback_t cb, void *user_data);
esp_err_t event_bus_dispatch(event_t *event, TickType_t tick_to_wait);

// clang-format on
#ifdef __cplusplus
}
#endif

#endif /* !EVENT_BUS_H */
