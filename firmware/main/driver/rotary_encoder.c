/**
 * \file rotary_encoder.c
 * \brief 
 *
 * \author Stanley Arnaud
 * \date 2026-10-30
 */

/* --------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------- */
#include "rotary_encoder.h"

#include "system/error.h"

#include "driver/gpio.h"
#include "driver/pulse_cnt.h"

/* --------------------------------------------------------------------------
 * Macros and Constants
 * -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
 * Type Definitions
 * -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
 * Static Variables
 * -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
 * Static Function Declarations
 * -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
 * Function Definitions
 * -------------------------------------------------------------------------- */
int
re_init(const re_config_t *config, re_t *re)
{
    int ret;
    pcnt_unit_config_t unit_cfg = {0};
    pcnt_unit_handle_t pcnt_unit = NULL;
    pcnt_glitch_filter_config_t filter_cfg = {0};
    pcnt_chan_config_t chan_a_cfg = {0};
    pcnt_channel_handle_t chan_a = NULL;
    pcnt_chan_config_t chan_b_cfg = {0};
    pcnt_channel_handle_t chan_b = NULL;

    unit_cfg.high_limit = INT16_MAX;
    unit_cfg.low_limit = INT16_MIN;
    if (pcnt_new_unit(&unit_cfg, &pcnt_unit) != ESP_OK)
    {
        return RET_ERR_FAIL;
    }

    filter_cfg.max_glitch_ns = 1000;
    if (pcnt_unit_set_glitch_filter(pcnt_unit, &filter_cfg) != ESP_OK)
    {
        ret = RET_ERR_FAIL;
        goto cleanup;
    }

    if (gpio_pullup_en(config->a_gpio) != ESP_OK)
    {
        ret = RET_ERR_FAIL;
        goto cleanup;
    }
    if (gpio_pullup_en(config->b_gpio) != ESP_OK)
    {
        ret = RET_ERR_FAIL;
        goto cleanup;
    }

    chan_a_cfg.edge_gpio_num = config->a_gpio;
    chan_a_cfg.level_gpio_num = config->b_gpio;
    if (pcnt_new_channel(pcnt_unit, &chan_a_cfg, &chan_a) != ESP_OK)
    {
        ret = RET_ERR_FAIL;
        goto cleanup;
    }

    chan_b_cfg.edge_gpio_num = config->b_gpio;
    chan_b_cfg.level_gpio_num = config->a_gpio;
    if (pcnt_new_channel(pcnt_unit, &chan_b_cfg, &chan_b) != ESP_OK)
    {
        ret = RET_ERR_FAIL;
        goto cleanup;
    }

    if (pcnt_channel_set_edge_action(
            chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE
        ))
    {
        ret = RET_ERR_FAIL;
        goto cleanup;
    }
    if (pcnt_channel_set_level_action(
            chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE
        ))
    {
        ret = RET_ERR_FAIL;
        goto cleanup;
    }
    if (pcnt_channel_set_edge_action(
            chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE
        ))
    {
        ret = RET_ERR_FAIL;
        goto cleanup;
    }
    if (pcnt_channel_set_level_action(
            chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE
        ))
    {
        ret = RET_ERR_FAIL;
        goto cleanup;
    }

    pcnt_unit_clear_count(pcnt_unit);

    /* Encoder has a switch */
    if (config->sw_gpio != -1)
    {
        if (gpio_set_direction(config->sw_gpio, GPIO_MODE_INPUT) != ESP_OK)
        {
            ret = RET_ERR_FAIL;
            goto cleanup;
        }
        if (gpio_pullup_en(config->sw_gpio) != ESP_OK)
        {
            ret = RET_ERR_FAIL;
            goto cleanup;
        }
    }

    re->unit = pcnt_unit;
    re->chan_a = chan_a;
    re->chan_b = chan_b;
    re->cfg = *config;

    return RET_OK;

cleanup:
    pcnt_unit_disable(pcnt_unit);
    pcnt_del_channel(chan_a);
    pcnt_del_channel(chan_b);
    pcnt_del_unit(pcnt_unit);
    return ret;
}

int
re_reset(re_t *re)
{
    if (pcnt_unit_clear_count(re->unit) != ESP_OK)
    {
        return RET_ERR_FAIL;
    }

    return RET_OK;
}

int
re_enable(re_t *re)
{
    if (pcnt_unit_enable(re->unit) != ESP_OK)
    {
        return RET_ERR_FAIL;
    }
    if (pcnt_unit_start(re->unit) != ESP_OK)
    {
        return RET_ERR_FAIL;
    }

    return RET_OK;
}

int
re_disable(re_t *re)
{
    if (pcnt_unit_stop(re->unit) != ESP_OK)
    {
        return RET_ERR_FAIL;
    }
    if (pcnt_unit_disable(re->unit) != ESP_OK)
    {
        return RET_ERR_FAIL;
    }

    return RET_OK;
}

int
re_get_raw_count(re_t *re, int *out_count)
{
    if (pcnt_unit_get_count(re->unit, out_count) != ESP_OK)
    {
        return RET_ERR_FAIL;
    }

    return RET_OK;
}

int32_t
re_get_position(re_t *re)
{
    int raw;

    if (pcnt_unit_get_count(re->unit, &raw) != ESP_OK)
    {
        return re->position;
    }

    int delta = raw - re->last_count;
    re->last_count = raw;

    /*
     * If direction changes, discard an incomplete step from the
     * previous direction.
     */
    if ((delta > 0 && re->remainder < 0) || (delta < 0 && re->remainder > 0))
    {
        re->remainder = 0;
    }

    re->remainder += delta;

    while (re->remainder >= 4)
    {
        re->position++;
        re->remainder -= 4;
    }

    while (re->remainder <= -4)
    {
        re->position--;
        re->remainder += 4;
    }

    return re->position;
}

bool
re_is_switch_pressed(re_t *re)
{ return !gpio_get_level(re->cfg.sw_gpio); }

/* --------------------------------------------------------------------------
 * Static Function Definitions
 * -------------------------------------------------------------------------- */
