/**
 * \file gui.c
 * \brief 
 *
 * \author Stanley Arnaud
 * \date 2026-06-05
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "gui.h"
#include "driver/button.h"
#include "driver/display.h"
#include "driver/lvgl_timer.h"
#include "driver/rotary_encoder.h"
#include "gui/generated/screens.h"
#include "gui/generated/ui.h"
#include "service/gui/generated/actions.h"
#include "system/config.h"
#include "system/error.h"
#include "system/event_bus.h"
#include "system/logging.h"

#include "freertos/FreeRTOS.h"
#include "lvgl.h"

#include <stdio.h>
#include <string.h>

#define LV_ANIM_DEF_TIME       200

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------
#define MIN(a, b)              (((a) < (b)) ? (a) : (b))
#define MAX(a, b)              (((a) > (b)) ? (a) : (b))

/* LVGL settings */
#define LVGL_TICK_PERIOD_MS    5
#define LVGL_TASK_MIN_DELAY_MS (1000 / CONFIG_FREERTOS_HZ)
#define LVGL_TASK_MAX_DELAY_MS 500
#define LVGL_PALETTE_SIZE      8

#define HOR_RES                128
#define VER_RES                64
#define PIXEL_BUF_SIZE         ((VER_RES + 7) / 8 * HOR_RES)
#define DRAW_BUF_SIZE          ((HOR_RES + 7) / 8 * VER_RES + LVGL_PALETTE_SIZE)

#define CMD_QUEUE_SIZE         10

#define TAG                    "gui"

/* --------------------------------------------------------------------------
 * Type Definitions
 * -------------------------------------------------------------------------- */
typedef struct
{
    unsigned min;
    unsigned max;
    char label_buf[5];
} knob_t;

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------
static struct
{
    display_t *display;
    lv_display_t *lv_disp;
    uint8_t pixel_buffer[PIXEL_BUF_SIZE];
    uint8_t draw_buffer[DRAW_BUF_SIZE];

    btn_t *trigger;
    bool last_trigger_state;
    re_t *re;
    int re_last_pos;
    lv_indev_t *indev;

    QueueHandle_t lvgl_queue;
    gui_cmd_t lvgl_queue_buf[CMD_QUEUE_SIZE];
    StaticQueue_t lvgl_queue_ctrl;

    StackType_t lvgl_task_buf[LVGL_TASK_SIZE];
    StaticTask_t lvgl_task_ctrl;
    TaskHandle_t lvgl_task;

    knob_t prf_knob;
    knob_t pd_knob;

    char home_label_buf[64];
} state = {.prf_knob.min = 1, .prf_knob.max = 1000, .pd_knob.min = 50, .pd_knob.max = 150};

// -----------------------------------------------------------------------------
// Static Function Declarations
// -----------------------------------------------------------------------------
static void lvgl_task_fn(void *arg);
static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
static void notify_lvgl_flush_ready(void *arg);

// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------
int
gui_init(const gui_ctx_t *ctx)
{
    int ret;

    if (ctx->display == NULL)
    {
        LOG_ERROR(TAG, "display is not initialized yet");
        return RET_ERR_FAIL;
    }

    state.display = ctx->display;
    state.trigger = ctx->trigger;
    state.re = ctx->re;

    if (re_enable(state.re) != RET_OK)
    {
        LOG_ERROR(TAG, "failed to enable re");
        return RET_ERR_FAIL;
    }

    /* Initialize LVGL */
    lv_init();
    LOG_INFO(TAG, "LVGL initialized");

    /* Create LVGL display */
    state.lv_disp = lv_display_create(HOR_RES, VER_RES);

    lv_display_set_color_format(state.lv_disp, LV_COLOR_FORMAT_I1);
    lv_display_set_buffers(
        state.lv_disp, state.draw_buffer, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_FULL
    );
    lv_display_set_flush_cb(state.lv_disp, lvgl_flush_cb);

    /* Set display bus color transfer done callback */
    display_set_color_trans_done_cb(state.display, notify_lvgl_flush_ready, NULL);

    /* Initialize LVGL timer */
    ret = lvgl_timer_init(LVGL_TICK_PERIOD_MS);
    if (ret != RET_OK)
    {
        LOG_ERROR(TAG, "failed to create LVGL timer");
        return RET_ERR_FAIL;
    }

    /* Create one unique task for LVGL/GUI events (thread-safe) */
    state.lvgl_queue = xQueueCreateStatic(
        CMD_QUEUE_SIZE, sizeof(gui_cmd_t), (uint8_t *)state.lvgl_queue_buf, &state.lvgl_queue_ctrl
    );
    if (state.lvgl_queue == NULL)
    {
        LOG_ERROR(TAG, "failed to create event queue");
        return RET_ERR_FAIL;
    }

    state.lvgl_task = xTaskCreateStatic(
        lvgl_task_fn, "lvgl_task", LVGL_TASK_SIZE, NULL, LVGL_TASK_PRIORITY, state.lvgl_task_buf,
        &state.lvgl_task_ctrl
    );
    if (state.lvgl_task == NULL)
    {
        LOG_ERROR(TAG, "failed to create lvgl task");
        return RET_ERR_FAIL;
    }

    LOG_INFO(TAG, "initialized");

    return RET_OK;
}

int
gui_sleep(void)
{
    if (display_turn_off(state.display) != RET_OK)
    {
        LOG_INFO(TAG, "failed to turn-off display");
        return RET_ERR_FAIL;
    }

    LOG_INFO(TAG, "display powered-off");

    return RET_OK;
}

int
gui_wake(void)
{
    if (display_turn_on(state.display) != RET_OK)
    {
        LOG_INFO(TAG, "failed to turn-on display");
        return RET_ERR_FAIL;
    }

    LOG_INFO(TAG, "display powered-on");

    return RET_OK;
}

int
gui_send_command(gui_cmd_t *cmd)
{
    if (xQueueSend(state.lvgl_queue, cmd, 0) != pdTRUE)
    {
        return RET_ERR_TIMEOUT;
    }

    return RET_OK;
}

void
action_knob_value_changed(lv_event_t *e)
{
    lv_obj_t *arc = lv_event_get_target_obj(e);
    lv_obj_t *value_label = lv_obj_get_child(arc, 1);

    knob_t *knob;
    unsigned value;

    event_t app_event = {0};

    if (lv_arc_get_value(arc) < 1)
    {
        lv_arc_set_value(arc, 1);
    }

    if (arc == objects.home_knob_prf)
    {
        knob = &state.prf_knob;
        app_event.code = EVENT_GUI_PRF_VALUE_CHANGED;
    }
    else if (arc == objects.home_knob_pd)
    {
        knob = &state.pd_knob;
        app_event.code = EVENT_GUI_PD_VALUE_CHANGED;
    }
    else
    {
        return;
    }

    value = (lv_arc_get_value(arc) - 1) * (knob->max - knob->min) / (lv_arc_get_max_value(arc) - 1)
            + knob->min;

    snprintf(knob->label_buf, sizeof(knob->label_buf), "%u", value % 10000);
    lv_label_set_text_static(value_label, knob->label_buf);

    app_event.payload.val = value;
    event_bus_publish(&app_event);
}

void
gui_get_home_label_buf(char **out_buf, size_t *out_size)
{
    *out_buf = state.home_label_buf;
    *out_size = sizeof(state.home_label_buf);
}

// -----------------------------------------------------------------------------
// Static Function Definitions
// -----------------------------------------------------------------------------
static inline void
process_cmd(gui_cmd_t *cmd)
{
    switch (cmd->code)
    {
        case GUI_CMD_NONE:
            break;
        case GUI_CMD_SET_MODE_MIDI:
            lv_obj_set_checked(objects.mode_midi_label, true);
            lv_obj_set_checked(objects.mode_manual_label, false);

            if (lv_obj_has_state(objects.home_knob_prf, LV_STATE_FOCUSED))
            {
                lv_group_focus_next(lv_obj_get_group(objects.home_knob_prf));
            }
            lv_obj_set_hidden(lv_obj_get_parent(objects.home_knob_prf), true);
            break;
        case GUI_CMD_SET_MODE_MANUAL:
            lv_obj_set_checked(objects.mode_midi_label, false);
            lv_obj_set_checked(objects.mode_manual_label, true);

            lv_obj_set_hidden(lv_obj_get_parent(objects.home_knob_prf), false);
            break;
        case GUI_CMD_UPDATE_HOME_LABEL:
            lv_label_set_text_static(objects.home_label, state.home_label_buf);
            break;
    }
}

static void
lvgl_indev_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    int diff, pos;
    bool trigger_state;
    event_t app_event;

    /* Rotary encoder */
    pos = re_get_position(state.re);
    diff = pos - state.re_last_pos;
    state.re_last_pos = pos;

    data->enc_diff = diff;
    data->state = re_is_switch_pressed(state.re) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;

    /* Trigger (app should own ownership but easier to debounce here since this
     * callback is quite slow ~30Hz) */
    trigger_state = btn_is_pressed(state.trigger);
    if (!state.last_trigger_state && trigger_state)
    {
        app_event.code = EVENT_INPUT_TRIGGER_PRESSED;
        event_bus_publish(&app_event);
    }
    else if (state.last_trigger_state && !trigger_state)
    {
        app_event.code = EVENT_INPUT_TRIGGER_RELEASED;
        event_bus_publish(&app_event);
    }

    state.last_trigger_state = trigger_state;
}

static void
lvgl_task_fn(void *arg)
{
    gui_cmd_t cmd = {0};
    int break_ms = 0;

    /* Initialize EEZ-Studio graphics */
    ui_init();

    state.indev = lv_indev_create();
    lv_indev_set_type(state.indev, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(state.indev, lvgl_indev_read_cb);

    lv_indev_set_group(state.indev, groups.home_knobs);

    while (1)
    {
        while (xQueueReceive(state.lvgl_queue, &cmd, 0) == pdTRUE)
        {
            process_cmd(&cmd);
        }

        /* Tick EEZ-Studio graphics */
        ui_tick();
        break_ms = lv_timer_handler();
        break_ms = MAX(break_ms, LVGL_TASK_MIN_DELAY_MS);
        break_ms = MIN(break_ms, LVGL_TASK_MAX_DELAY_MS);
        vTaskDelay(pdMS_TO_TICKS(break_ms));
    }
}

static void
notify_lvgl_flush_ready(void *arg)
{ lv_display_flush_ready(state.lv_disp); }

/* This function is found here:
 * https://github.com/espressif/esp-idf/blob/master/examples/peripherals/lcd/i2c_oled/main/i2c_oled_example_main.c */
static void
lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    // This is necessary because LVGL reserves 2 x 4 bytes in the buffer, as these are assumed to be used as a palette. Skip the palette here
    // More information about the monochrome, please refer to https://docs.lvgl.io/9.2/porting/display.html#monochrome-displays
    px_map += LVGL_PALETTE_SIZE;

    uint16_t hor_res = lv_display_get_physical_horizontal_resolution(disp);
    int x1 = area->x1;
    int x2 = area->x2;
    int y1 = area->y1;
    int y2 = area->y2;

    for (int y = y1; y <= y2; y++)
    {
        for (int x = x1; x <= x2; x++)
        {
            /* The order of bits is MSB first
                        MSB           LSB
               bits      7 6 5 4 3 2 1 0
               pixels    0 1 2 3 4 5 6 7
                        Left         Right
            */
            bool chroma_color = (px_map[(hor_res >> 3) * y + (x >> 3)] & 1 << (7 - x % 8));

            /* Write to the buffer as required for the display.
            * It writes only 1-bit for monochrome displays mapped vertically.*/
            uint8_t *buf = state.pixel_buffer + hor_res * (y >> 3) + (x);
            if (chroma_color)
            {
                (*buf) |= (1 << (y % 8));
            }
            else
            {
                (*buf) &= ~(1 << (y % 8));
            }
        }
    }

    // pass the draw buffer to the driver
    if (display_draw_bitmap(state.display, x1, y1, x2 + 1, y2 + 1, state.pixel_buffer) != RET_OK)
    {
        /*
         * No asynchronous completion callback will arrive,
         * so we must complete the LVGL flush ourselves.
         */
        lv_display_flush_ready(disp);
    }
}

// External LVGL timer callback implementation
void
lvgl_timer_increase_tick_cb(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}
