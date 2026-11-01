/**
 * \file lvgl_timer.c
 * \brief 
 *
 * \author Stanley Arnaud
 * \date 2026-06-10
 */

/* --------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------- */
#include "lvgl_timer.h"
#include "system/error.h"

#include "esp_timer.h"

/* --------------------------------------------------------------------------
 * Type Definitions
 * -------------------------------------------------------------------------- */
typedef struct
{
    esp_timer_handle_t timer;
    uint32_t period_ms;
} lvgl_timer_ctx_t;


/* --------------------------------------------------------------------------
 * Static Variables
 * -------------------------------------------------------------------------- */
static lvgl_timer_ctx_t ctx = {0};

/* --------------------------------------------------------------------------
 * Function Definitions
 * -------------------------------------------------------------------------- */
int
lvgl_timer_init(uint32_t period_ms)
{
    esp_err_t res;
    esp_timer_create_args_t lvgl_tick_timer_args = {0};
    esp_timer_handle_t lvgl_tick_timer = NULL;

    lvgl_tick_timer_args.callback = lvgl_timer_increase_tick_cb;
    lvgl_tick_timer_args.name = "lvgl_tick";

    res = esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    if (res != ESP_OK)
    {
        return RET_ERR_FAIL;
    }
    res = esp_timer_start_periodic(lvgl_tick_timer, period_ms * 1000);
    if (res != ESP_OK)
    {
        esp_timer_delete(lvgl_tick_timer);
        return RET_ERR_FAIL;
    }

    ctx.timer = lvgl_tick_timer;
    ctx.period_ms = period_ms;

    return RET_OK;
}

int
lvgl_timer_stop(void)
{
    esp_err_t ret;

    ret = esp_timer_stop(ctx.timer);
    if (ret != ESP_OK)
    {
        return RET_ERR_FAIL;
    }

    return RET_OK;
}

int
lvgl_timer_resume(void)
{
    esp_err_t ret;

    ret = esp_timer_start_periodic(ctx.timer, ctx.period_ms * 1000);
    if (ret != ESP_OK)
    {
        return RET_ERR_FAIL;
    }

    return RET_OK;
}

int
lvgl_timer_deinit(void)
{
    esp_timer_stop(ctx.timer);
    esp_timer_delete(ctx.timer);
    ctx.timer = NULL;

    return RET_OK;
}

