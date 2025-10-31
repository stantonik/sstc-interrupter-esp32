/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file menu.c
 * @brief
 *
 * @author Stanley Arnaud
 * @date 10/24/2025
 * @version 0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "menu.h"
#include "./generated/ui.h"
#include "core/event_bus.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "hal/controls.h"
#include "hal/display.h"
#include "knobs.h"
#include "lvgl.h"

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------
#define TAG "menu"

// -----------------------------------------------------------------------------
// Private Typedef
// -----------------------------------------------------------------------------
typedef struct
{
    controls_event_t type;
    int8_t diff;
} menu_re_evt_t;

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------
static menu_mode_t mode = MENU_MODE_MANUAL;
static menu_state_mask_t state = 0;

static QueueHandle_t re_queue = NULL;

// -----------------------------------------------------------------------------
// Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Static Function Definitions
// -----------------------------------------------------------------------------
void indev_encoder_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    menu_re_evt_t e = {0};

    if (xQueueReceive(re_queue, &e, 0) == pdPASS)
    {
        lv_group_t *group = lv_group_get_default();
        if (group)
        {
            lv_obj_t *arc = lv_group_get_focused(group);
            bool editing = lv_group_get_editing(group);

            switch (e.type)
            {
            case CONTROLS_EVENT_RE_BTN_CLICKED:
                lv_group_set_editing(group, !editing);
                break;
            case CONTROLS_EVENT_RE_BTN_LONG_PRESSED:
                lv_event_send(arc, LV_EVENT_LONG_PRESSED, NULL);
                break;
            case CONTROLS_EVENT_RE_CHANGED:
                if (editing)
                {
                    lv_arc_set_value(arc, lv_arc_get_value(arc) + e.diff * knobs_get_current_step());
                    lv_event_send(arc, LV_EVENT_VALUE_CHANGED, NULL);
                }
                else
                {
                    if (e.diff > 0)
                        lv_group_focus_next(group);
                    else if (e.diff < 0)
                        lv_group_focus_prev(group);
                }
                break;
            default:
                break;
            }
        }
    }
}

static void encoder_event_cb(const event_t *event, void *user_data)
{
    menu_re_evt_t e = {.type = event->type, .diff = event->value};
    xQueueSend(re_queue, &e, 0);
}

// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------
esp_err_t menu_init(void)
{
    // Initialize LVGL
    display_handles_t display = display_get_handles();

    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "Failed to initialize LVGL");

    const lvgl_port_display_cfg_t disp_cfg = {.io_handle = display.io_handle,
        .panel_handle = display.panel_handle,
        .buffer_size = DISPLAY_WIDTH * DISPLAY_HEIGHT,
        .double_buffer = true,
        .hres = DISPLAY_WIDTH,
        .vres = DISPLAY_HEIGHT,
        .monochrome = true,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        }};

    lvgl_port_add_disp(&disp_cfg);

    // Subscribe to encoder (within controls) events
    event_bus_subscribe(EVENT_SRC_CONTROLS, encoder_event_cb, NULL);
    re_queue = xQueueCreate(4, sizeof(menu_re_evt_t));

    // Initialize UI
    lvgl_port_lock(0);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = indev_encoder_cb;
    lv_indev_drv_register(&indev_drv);

    ui_create_groups();

    lv_group_set_wrap(groups.range_group, false);
    lv_group_set_default(groups.range_group);

    ui_init();

    menu_set_mode(MENU_MODE_MANUAL, false);
    menu_set_state(MENU_STATE_IDLE);
    menu_set_header_text("");

    lvgl_port_unlock();

    ESP_LOGI(TAG, "Initializaion succeeded");

    return ESP_OK;
}

esp_err_t menu_set_mode(menu_mode_t m, bool msg_box)
{
    mode = m;

    lvgl_port_lock(0);

    lv_group_set_editing(lv_group_get_default(), false);

    switch (mode)
    {
    case MENU_MODE_MANUAL:
        // Hide Audio mode cues
        lv_label_set_text(objects.aux_type_label, "MANUAL");
        lv_obj_add_flag(objects.aux_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(objects.aux_type_label, LV_OBJ_FLAG_HIDDEN);

        // Display the corresponding knobs
        lv_obj_clear_flag(objects.pd_arc, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(objects.prf_arc, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(objects.pwr_arc, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(objects.gdb_arc, LV_OBJ_FLAG_HIDDEN);

        lv_group_focus_obj(objects.pd_arc);

        // Display message bow
        if (msg_box) menu_display_msg_box("Manual mode", 1000);
        break;
    case MENU_MODE_AUDIO_JACK:
        // Hide Audio mode cues
        lv_label_set_text(objects.aux_type_label, "LINE");
        lv_obj_clear_flag(objects.aux_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(objects.aux_type_label, LV_OBJ_FLAG_HIDDEN);

        // Display the corresponding knobs
        lv_obj_add_flag(objects.pd_arc, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(objects.prf_arc, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(objects.pwr_arc, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(objects.gdb_arc, LV_OBJ_FLAG_HIDDEN);

        lv_group_focus_obj(objects.gdb_arc);

        // Display message bow
        if (msg_box) menu_display_msg_box("Line IN mode", 1000);
        break;
    case MENU_MODE_MIDI:
        // Hide Audio mode cues
        lv_label_set_text(objects.aux_type_label, "MIDI");
        lv_obj_clear_flag(objects.aux_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(objects.aux_type_label, LV_OBJ_FLAG_HIDDEN);

        // Display the corresponding knobs
        lv_obj_add_flag(objects.pd_arc, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(objects.prf_arc, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(objects.pwr_arc, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(objects.gdb_arc, LV_OBJ_FLAG_HIDDEN);

        lv_group_focus_obj(objects.pwr_arc);

        // Display message bow
        if (msg_box) menu_display_msg_box("MIDI mode", 1000);
        break;
    default:
        break;
    }

    lvgl_port_unlock();

    return ESP_OK;
}

inline menu_mode_t menu_get_mode(void) { return mode; }

inline esp_err_t menu_set_state(menu_state_t s)
{
    state = s;
    lvgl_port_lock(0);

    if (state & MENU_STATE_IDLE)
    {
        lv_obj_add_flag(objects.armed_icon, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(objects.state_label, "IDLE");
    }
    else if (state & MENU_STATE_ARMED)
    {
        lv_obj_clear_flag(objects.armed_icon, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(objects.state_label, "FIRE");
    }

    lvgl_port_unlock();

    return ESP_OK;
}

inline void menu_set_header_text(const char *text)
{
    lvgl_port_lock(0);

    lv_label_set_text(objects.header_label, text);

    lvgl_port_unlock();
}

static void hide_msg_box_cb(lv_timer_t *timer)
{
    lv_obj_add_flag(objects.message_box, LV_OBJ_FLAG_HIDDEN);
    lv_group_set_default(groups.range_group);
    if (timer) lv_timer_del(timer);
}

inline void menu_hide_msg_box(void)
{
    lvgl_port_lock(0);
    hide_msg_box_cb(NULL);
    lvgl_port_unlock();
}

void menu_display_msg_box(const char *msg, uint16_t duration_ms)
{
    lvgl_port_lock(0);

    lv_obj_t *message = lv_obj_get_child(objects.message_box, 0);
    lv_label_set_text(message, msg);
    lv_obj_clear_flag(objects.message_box, LV_OBJ_FLAG_HIDDEN);
    lv_group_set_default(NULL);
    lvgl_port_unlock();

    if (duration_ms > 0)
    {
        lv_timer_t *timer = lv_timer_create(hide_msg_box_cb, duration_ms, NULL);
        lv_timer_set_repeat_count(timer, 1);
        lv_timer_create(hide_msg_box_cb, duration_ms, objects.message_box);
    }

    lvgl_port_unlock();
}
