/**
 * \file synth.h
 * \brief 
 *
 * \author Stanley Arnaud
 * \date 2026-10-06
 */

#ifndef SYNTH_H
#define SYNTH_H

/* --------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------- */
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* --------------------------------------------------------------------------
 * Macros and Constants
 * -------------------------------------------------------------------------- */
#define SYNTH_MAX_CHANNEL 8

/* --------------------------------------------------------------------------
 * Type Definitions
 * -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
 * Function Declarations
 * -------------------------------------------------------------------------- */
typedef struct
{
    int output_gpio;
} synth_config_t;

typedef struct synth_internal_s synth_t;

int synth_create(synth_config_t *config, synth_t **synth_out);
int synth_set_channel(synth_t *synth, int chan_id, float freq_hz, int width_us);
int synth_enable(synth_t *synth);
int synth_disable(synth_t *synth);
// int synth_del(synth_t *synth);

/* --------------------------------------------------------------------------
 * Inline Function Definitions
 * -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !SYNTH_H */
