/**
 * \file lvgl_timer.h
 * \brief 
 *
 * \author Stanley Arnaud
 * \date 2026-06-10
 */

#ifndef LVGL_TIMER_H
#define LVGL_TIMER_H

/* --------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------- */
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* --------------------------------------------------------------------------
 * Function Declarations
 * -------------------------------------------------------------------------- */
/**
 * \brief Initialize LVGL tick increment timer
 *
 * \param[in] period_ms Timer period (ms)
 *
 * \return
 *     - RET_OK
 *     - RET_ERR_INVALID_STATE
 *     - RET_ERR_FAIL
 */
int lvgl_timer_init(uint32_t period_ms);

/**
 * \brief Stop LVGL tick increment timer
 *
 * \return
 *     - RET_OK
 *     - RET_ERR_INVALID_STATE
 *     - RET_ERR_FAIL
 */
int lvgl_timer_stop(void);

/**
 * \brief Resume LVGL tick increment timer
 *
 * \return
 *     - RET_OK
 *     - RET_ERR_INVALID_STATE
 *     - RET_ERR_FAIL
 */
int lvgl_timer_resume(void);

/**
 * \brief Deinitialize LVGL tick increment timer
 *
 * \return
 *     - RET_OK
 *     - RET_ERR_INVALID_STATE
 *     - RET_ERR_FAIL
 */
int lvgl_timer_deinit(void);

/**
 * \brief External callback to implement in service layer to increment LVGL tick each timer loop
 */
extern void lvgl_timer_increase_tick_cb(void *arg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LVGL_TIMER_H */
