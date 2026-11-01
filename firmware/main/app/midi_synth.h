/**
 * \file midi_synth.h
 * \brief 
 *
 * \author Stanley Arnaud
 * \date 2026-11-01
 */

#ifndef MIDI_SYNTH_H
#define MIDI_SYNTH_H

/* --------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------- */
#include <stdint.h>
#include "driver/synth.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* --------------------------------------------------------------------------
 * Macros and Constants
 * -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
 * Type Definitions
 * -------------------------------------------------------------------------- */
typedef struct
{
    uint8_t status;
    uint8_t data1;
    uint8_t data2;
} midi_message_t;

/* --------------------------------------------------------------------------
 * Function Declarations
 * -------------------------------------------------------------------------- */
void midi_interpret_and_play_message(synth_t *synth, const midi_message_t *message, unsigned max_width);

/* --------------------------------------------------------------------------
 * Inline Function Definitions
 * -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !MIDI_SYNTH_H */
