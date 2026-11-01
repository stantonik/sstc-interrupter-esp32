/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file usb_midi.h
 * @brief 
 *
 * 
 *
 * @author Stanley Arnaud 
 * @date 10/24/2025
 * @version 0
 */

#ifndef USB_MIDI_H
#define USB_MIDI_H

// clang-format off
#include <stdbool.h>
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

// -----------------------------------------------------------------------------
// Type Definitions
// -----------------------------------------------------------------------------
typedef enum
{
    USB_MIDI_EVENT_CONNECTED,
    USB_MIDI_EVENT_DISCONNECTED
} usb_midi_event_t;

typedef struct
{
    uint8_t state: 1;
    uint8_t note: 7;
    uint8_t velocity;
} midi_message_t;

typedef struct
{
    char note[3];
    float velocity;
    int8_t octave;
    float freq_hz;
} midi_msg_parsed_t;

typedef void (* midi_on_receive_cb_t)(midi_message_t);

// -----------------------------------------------------------------------------
// Inline Function Definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Function Declarations
// -----------------------------------------------------------------------------
esp_err_t usb_midi_init(void);
esp_err_t usb_midi_set_on_receive_cb(midi_on_receive_cb_t cb);
midi_msg_parsed_t usb_midi_parse_msg(midi_message_t *msg);

#ifdef __cplusplus
}
#endif
// clang-format on

#endif /* !USB_MIDI_H */

