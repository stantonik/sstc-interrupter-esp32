/**
 * \file rotary_encoder.h
 * \brief 
 *
 * \author Stanley Arnaud
 * \date 2026-10-30
 */

#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

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

/* --------------------------------------------------------------------------
 * Type Definitions
 * -------------------------------------------------------------------------- */
typedef struct
{
    int a_gpio;
    int b_gpio;
    int sw_gpio;

    void *ctx;
} re_config_t;

typedef struct
{
    re_config_t cfg;

    void *unit;
    void *chan_a;
    void *chan_b;

    int last_count;
    int position;
    int remainder;
} re_t;

/* --------------------------------------------------------------------------
 * Function Declarations
 * -------------------------------------------------------------------------- */
int re_init(const re_config_t *config, re_t *re);
int re_deinit(re_t *re);
int re_enable(re_t *re);
int re_disable(re_t *re);
int32_t re_get_position(re_t *re);
bool re_is_switch_pressed(re_t *re);
int re_reset(re_t *re);

/* --------------------------------------------------------------------------
 * Inline Function Definitions
 * -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !ROTARY_ENCODER_H */
