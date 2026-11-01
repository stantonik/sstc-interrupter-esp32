/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file knobs.c
 * @brief
 *
 * @author Stanley Arnaud
 * @date 10/26/2025
 * @version 0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "knobs.h"
#include "app/gui/generated/screens.h"
#include "app/gui/generated/vars.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "lvgl.h"

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------
#define TAG "knobs"

#if CONFIG_INTERRUPTER_TON_MIN >= CONFIG_INTERRUPTER_TON_MAX
#error "CONFIG_INTERRUPTER_TON_MIN must be less than CONFIG_INTERRUPTER_TON_MAX"
#endif

#if CONFIG_INTERRUPTER_PD_DEFAULT < CONFIG_INTERRUPTER_TON_MIN ||                                                      \
    CONFIG_INTERRUPTER_PD_DEFAULT > CONFIG_INTERRUPTER_TON_MAX
#error "CONFIG_INTERRUPTER_PD_DEFAULT must be between TON_MIN and TON_MAX"
#endif

#if CONFIG_INTERRUPTER_PRF_MIN > CONFIG_INTERRUPTER_PRF_MAX
#error "CONFIG_INTERRUPTER_PRF_MIN must be <= CONFIG_INTERRUPTER_PRF_MAX"
#endif

#if CONFIG_INTERRUPTER_PRF_DEFAULT < CONFIG_INTERRUPTER_PRF_MIN ||                                                     \
    CONFIG_INTERRUPTER_PRF_DEFAULT > CONFIG_INTERRUPTER_PRF_MAX
#error "CONFIG_INTERRUPTER_PRF_DEFAULT must be between PRF_MIN and PRF_MAX"
#endif

#define PD_MIN 0
#define PD_DEFAULT CONFIG_INTERRUPTER_PD_DEFAULT
#define PD_MAX CONFIG_INTERRUPTER_TON_MAX
#define PD_STEP CONFIG_INTERRUPTER_PD_STEP

#define PRF_MIN CONFIG_INTERRUPTER_PRF_MIN
#define PRF_DEFAULT CONFIG_INTERRUPTER_PRF_DEFAULT
#define PRF_MAX CONFIG_INTERRUPTER_PRF_MAX
#define POWER_STEP CONFIG_INTERRUPTER_POWER_STEP

#define POWER_DEFAULT 50
#define POWER_MIN 0
#define POWER_MAX 100
#define POWER_STEP CONFIG_INTERRUPTER_POWER_STEP

#define GAIN_MIN -50
#define GAIN_DEFAULT 0
#define GAIN_MAX 50
#define GAIN_STEP CONFIG_INTERRUPTER_GAIN_STEP

#define INIT_KNOB(knob, type_val, arc_obj, min_val, max_val, value_val, step0_val, step1_val)                          \
    do                                                                                                                 \
    {                                                                                                                  \
        (knob).arc = (arc_obj);                                                                                        \
        (knob).type = (type_val);                                                                                      \
        (knob).min_physical = (min_val);                                                                               \
        (knob).max_physical = (max_val);                                                                               \
        (knob).min_effective = (min_val);                                                                              \
        (knob).max_effective = (max_val);                                                                              \
        (knob).value = (value_val);                                                                                    \
        (knob).user_value = (value_val);                                                                               \
        (knob).steps[0] = (step0_val);                                                                                 \
        (knob).steps[1] = (step1_val);                                                                                 \
        all[(type_val)] = &(knob);                                                                                     \
    } while (0)

// -----------------------------------------------------------------------------
// Private Typedef
// -----------------------------------------------------------------------------
typedef struct
{
    lv_obj_t *arc;
    knob_name_t type;
    int16_t user_value;
    struct
    {
        volatile int16_t value;
        int16_t min_physical;
        int16_t max_physical;
    };
    int16_t min_effective;
    int16_t max_effective;
    uint16_t steps[2];
} knob_data_t;

knob_data_t prf, pd, pwr, gdb;
knob_data_t *all[KNOB_COUNT];
knob_t *knobs_pub[KNOB_COUNT] = {NULL};

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------
static knobs_on_change_cb_t changed_cb = NULL;
static volatile uint16_t current_step = 1;
static uint8_t step_ind = 0;

// -----------------------------------------------------------------------------
// Static Function Declarations
// -----------------------------------------------------------------------------
static void arc_value_change_cb(lv_event_t *e);
static void arc_change_step_cb(lv_event_t *e);
static void arc_refresh_visual(lv_event_t *e);

// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------
esp_err_t knobs_init(void)
{
    /* Pulse Delay Range */
    INIT_KNOB(pd, KNOB_PD, objects.pd_arc, 0, PD_MAX, PD_DEFAULT, 1, 10);
    /* Pulse Repetition Frequency Range */
    INIT_KNOB(prf, KNOB_PRF, objects.prf_arc, PRF_MIN, PRF_MAX, PRF_DEFAULT, 1, 100);
    /* Power Range */
    INIT_KNOB(pwr, KNOB_PWR, objects.pwr_arc, POWER_MIN, POWER_MAX, POWER_DEFAULT, 1, 10);
    /* Gain Range */
    INIT_KNOB(gdb, KNOB_GDB, objects.gdb_arc, GAIN_MIN, GAIN_MAX, GAIN_DEFAULT, 5, 10);

    // Common assignations
    for (int i = 0; i < KNOB_COUNT; ++i)
    {
        knob_data_t *knob = all[i];

        knobs_pub[i] = (knob_t *)((uint8_t *)knob + offsetof(knob_data_t, value));

        lv_arc_set_range(knob->arc, knob->min_physical, knob->max_physical);
        lv_arc_set_value(knob->arc, knob->value);

        // Add event callbacks
        // Value changed
        lv_obj_add_event_cb(knob->arc, arc_value_change_cb, LV_EVENT_VALUE_CHANGED, knob);
        lv_obj_add_event_cb(knob->arc, arc_change_step_cb, LV_EVENT_LONG_PRESSED, knob);
        // Change step
        lv_obj_add_event_cb(knob->arc, arc_change_step_cb, LV_EVENT_FOCUSED, knob);
        // Refresh visual
        lv_obj_add_event_cb(knob->arc, arc_refresh_visual, LV_EVENT_REFRESH, knob);
        lv_obj_add_event_cb(knob->arc, arc_refresh_visual, LV_EVENT_FOCUSED, knob);
        lv_obj_add_event_cb(knob->arc, arc_refresh_visual, LV_EVENT_DEFOCUSED, knob);

        // Update visual
        lv_event_send(knob->arc, LV_EVENT_VALUE_CHANGED, NULL);
    }

    ESP_LOGI(TAG, "Initializaion succeeded");

    return ESP_OK;
}

esp_err_t knobs_set_on_change_cb(knobs_on_change_cb_t cb)
{
    changed_cb = cb;
    return cb ? ESP_OK : ESP_ERR_INVALID_ARG;
}

inline uint16_t knobs_get_current_step(void) { return current_step; }

inline IRAM_ATTR esp_err_t knobs_get_values(const knob_t *knobs[KNOB_COUNT])
{
    memcpy(knobs, knobs_pub, KNOB_COUNT * sizeof(knobs_pub[0]));
    return ESP_OK;
}

static inline void sync_changes(knob_data_t *knob)
{
    lv_arc_set_value(knob->arc, knob->value);
    lv_event_send(knob->arc, LV_EVENT_REFRESH, NULL);
}

static inline void update_value(knob_data_t *knob, int16_t value, bool user)
{
    if (pd.max_effective > pd.max_physical) pd.max_effective = pd.max_physical;
    if (pd.min_effective < pd.min_physical) pd.min_effective = pd.min_physical;

    if (value > knob->max_effective)
        value = knob->max_effective;
    else if (value < knob->min_effective)
        value = knob->min_effective;

    if (user) knob->user_value = value;
    knob->value = value;
}

static inline void constrain_pd_to_prf(void)
{
    float period = 1e6 / prf.value;
    int32_t pd_max_allowed = period - CONFIG_INTERRUPTER_TOFF_MIN;
    if (pd_max_allowed < pd.min_effective) pd_max_allowed = pd.min_effective;

    pd.max_effective = (pd_max_allowed < PD_MAX) ? pd_max_allowed : PD_MAX;

    // Try to restore user's intended value if possible
    float target = pd.user_value;
    if (target > pd.max_effective) target = pd.max_effective;
    if (target < pd.min_effective) target = pd.min_effective;
    update_value(&pd, target, false);
}

// -----------------------------------------------------------------------------
// LVGL knobs callback
// -----------------------------------------------------------------------------
static void arc_value_change_cb(lv_event_t *e)
{
    knob_data_t *knob = lv_event_get_user_data(e);
    update_value(knob, lv_arc_get_value(knob->arc), true);
    knobs_mask_t mask = (1 << knob->type);

    if (knob == &pd)
    {
        constrain_pd_to_prf();
    }
    else if (knob == &prf)
    {
        constrain_pd_to_prf();
        sync_changes(&pd);
        mask |= (1 << KNOB_PD);
    }
    else if (knob == &pwr)
    {
    }
    else if (knob == &gdb)
    {
    }

    sync_changes(knob);
    ESP_LOGI(TAG, "Values updated (knob mask=%d)", (int)mask);
    if (changed_cb) changed_cb(mask, (const knob_t **)knobs_pub);
}

static void arc_change_step_cb(lv_event_t *e)
{
    knob_data_t *knob = lv_event_get_user_data(e);
    if (!(lv_obj_get_state(knob->arc) & LV_STATE_EDITED)) return;

    if (e->code == LV_EVENT_LONG_PRESSED)
    {
        step_ind++;
        if (step_ind > 1) step_ind = 0;

        current_step = knob->steps[step_ind];
    }
    else if (e->code == LV_EVENT_FOCUSED)
    {
        step_ind = 0;
        current_step = knob->steps[step_ind];
    }
}

static void arc_refresh_visual(lv_event_t *e)
{
    lv_obj_t *arc = lv_event_get_current_target(e);
    lv_obj_t *val_label = lv_obj_get_child(arc, arc_child_ind_VALUE);
    lv_obj_t *sel_img = lv_obj_get_child(arc, arc_child_ind_SEL);

    if (e->code == LV_EVENT_REFRESH)
    {
        int val = lv_arc_get_value(arc);
        char val_str[8];

        if (val < 1000)
        {
            // Small numbers: display normally
            snprintf(val_str, sizeof(val_str), "%d", val);
        }
        else if (val < 10000)
        {
            // 1,000–9,999: use 1 decimal
            float v = val / 1000.0f;
            // Round to 1 decimal and fit in 5 chars
            snprintf(val_str, sizeof(val_str), "%.1fk", v);
        }
        else if (val < 100000)
        {
            // 10,000–99,999: round to nearest integer in k
            int v = (val + 50) / 100; // rounding to nearest 0.1k
            snprintf(val_str, sizeof(val_str), "%d.%dk", v / 10, v % 10);
        }
        else
        {
            // ≥100,000: just show 5 digits with k
            int v = val / 1000;
            snprintf(val_str, sizeof(val_str), "%dk", v);
        }

        lv_label_set_text(val_label, val_str);
    }
    else if (e->code == LV_EVENT_FOCUSED)
    {
        lv_obj_add_state(sel_img, LV_STATE_FOCUSED);
    }
    else if (e->code == LV_EVENT_DEFOCUSED)
    {
        lv_obj_clear_state(sel_img, LV_STATE_FOCUSED);
    }
}
