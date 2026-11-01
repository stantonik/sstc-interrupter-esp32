/**
 * \file app.c
 * \brief 
 *
 * \author Stanley Arnaud
 * \date 2026-11-01
 */

/* --------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------- */
#include <stdatomic.h>
#define COMPILE_TIME_LOG_LEVEL LOG_LEVEL_INFO
#include "system/logging.h"

#include "app.h"
#include "app/midi_synth.h"
#include "driver/synth.h"
#include "service/gui.h"
#include "service/usb.h"
#include "system/config.h"
#include "system/error.h"
#include "system/event_bus.h"

#include "freertos/FreeRTOS.h"

#include <stdio.h>

/* --------------------------------------------------------------------------
 * Macros and Constants
 * -------------------------------------------------------------------------- */
#define MIDI_QUEUE_SIZE 20

#define TAG             "app"

/* --------------------------------------------------------------------------
 * Type Definitions
 * -------------------------------------------------------------------------- */
typedef enum
{
    APP_MODE_MANUAL,
    APP_MODE_MIDI,
} app_mode_t;

/* --------------------------------------------------------------------------
 * Static Variables
 * -------------------------------------------------------------------------- */
static struct
{
    synth_t *synth;
    float freq;
    atomic_uint width;

    atomic_int mode;

    StackType_t task_buf[APP_TASK_SIZE];
    StaticTask_t task_ctrl;
    TaskHandle_t task;

    QueueHandle_t midi_queue;
    SemaphoreHandle_t synth_mutex;
    StaticSemaphore_t synth_mutex_buf;
    gui_cmd_t midi_queue_buf[MIDI_QUEUE_SIZE];
    StaticQueue_t midi_queue_ctrl;
    StackType_t midi_task_buf[MIDI_TASK_SIZE];
    StaticTask_t midi_task_ctrl;
    TaskHandle_t midi_task;
} g_state = {0};

/* --------------------------------------------------------------------------
 * Static Function Declarations
 * -------------------------------------------------------------------------- */
static void task(void *arg);
static void midi_task(void *arg);

/* --------------------------------------------------------------------------
 * Function Definitions
 * -------------------------------------------------------------------------- */
int
app_init(const app_ctx_t *ctx)
{
    g_state.synth = ctx->synth;

    g_state.synth_mutex = xSemaphoreCreateMutexStatic(&g_state.synth_mutex_buf);

    g_state.midi_queue = xQueueCreateStatic(
        MIDI_QUEUE_SIZE, sizeof(midi_message_t), (uint8_t *)g_state.midi_queue_buf,
        &g_state.midi_queue_ctrl
    );
    if (g_state.midi_queue == NULL)
    {
        LOG_ERROR(TAG, "failed to create midi event queue");
        return RET_ERR_FAIL;
    }

    g_state.task = xTaskCreateStatic(
        task, "app_task", APP_TASK_SIZE, NULL, APP_TASK_PRIORITY, g_state.task_buf,
        &g_state.task_ctrl
    );
    if (g_state.task == NULL)
    {
        LOG_ERROR(TAG, "failed to create app task");
        return RET_ERR_FAIL;
    }

    g_state.midi_task = xTaskCreateStatic(
        midi_task, "midi_task", MIDI_TASK_SIZE, NULL, MIDI_TASK_PRIORITY, g_state.midi_task_buf,
        &g_state.midi_task_ctrl
    );
    if (g_state.midi_task == NULL)
    {
        LOG_ERROR(TAG, "failed to create midi task");
        return RET_ERR_FAIL;
    }

    return RET_OK;
}

/* --------------------------------------------------------------------------
 * Static Function Definitions
 * -------------------------------------------------------------------------- */
static void
midi_event_cb(uint8_t *packet, size_t size)
{
    if (size < 4)
    {
        return;
    }

    midi_message_t msg;

    if (atomic_load(&g_state.mode) == APP_MODE_MIDI)
    {
        msg.status = packet[1];
        msg.data1 = packet[2];
        msg.data2 = packet[3];
        xQueueSend(g_state.midi_queue, &msg, 0);
    }
}

static void
midi_task(void *arg)
{
    midi_message_t msg;

    while (xQueueReceive(g_state.midi_queue, &msg, portMAX_DELAY) == pdTRUE)
    {

        xSemaphoreTake(g_state.synth_mutex, portMAX_DELAY);
        if (atomic_load(&g_state.mode) == APP_MODE_MIDI)
        {
            midi_interpret_and_play_message(g_state.synth, &msg, atomic_load(&g_state.width));
        }
        xSemaphoreGive(g_state.synth_mutex);
    }
}

static void
task(void *arg)
{
    event_t event;
    gui_cmd_t gui_cmd = {0};
    TickType_t start_time = xTaskGetTickCount();
    bool boot_triggered = false;

    char *label_buf;
    size_t label_buf_sz;

    atomic_store(&g_state.mode, APP_MODE_MANUAL);

    usb_set_midi_event_cb(midi_event_cb);

    gui_get_home_label_buf(&label_buf, &label_buf_sz);
    gui_wake();

    while (1)
    {
        // Process events
        while (event_bus_check(&event, portMAX_DELAY) == pdPASS)
        {
            switch (event.code)
            {
                case EVENT_NONE:
                    break;

                case EVENT_USB_HOST_ATTACHED:
                case EVENT_USB_RESUMED:
                    for (int i = 0; i < SYNTH_MAX_CHANNEL; ++i)
                    {
                        synth_set_channel(g_state.synth, i, 0, 0);
                    }
                    atomic_store(&g_state.mode, APP_MODE_MIDI);
                    gui_cmd.code = GUI_CMD_SET_MODE_MIDI;
                    break;
                case EVENT_USB_HOST_DETACHED:
                case EVENT_USB_SUSPENDED:
                    /* 1. Prevent new MIDI messages from being accepted. */
                    atomic_store(&g_state.mode, APP_MODE_MANUAL);

                    /* 2. Discard MIDI messages that haven't started processing. */
                    xQueueReset(g_state.midi_queue);

                    /* 3. Wait for any MIDI operation currently using the synth. */
                    xSemaphoreTake(g_state.synth_mutex, portMAX_DELAY);

                    /* 4. Now establish the final synth state. */
                    synth_set_channel(g_state.synth, 0, g_state.freq, atomic_load(&g_state.width));

                    for (int i = 1; i < SYNTH_MAX_CHANNEL; ++i)
                    {
                        synth_set_channel(g_state.synth, i, 0, 0);
                    }

                    xSemaphoreGive(g_state.synth_mutex);

                    /* Clear last note */
                    label_buf[0] = '\0';
                    gui_cmd.code = GUI_CMD_UPDATE_HOME_LABEL;
                    gui_send_command(&gui_cmd);

                    gui_cmd.code = GUI_CMD_SET_MODE_MANUAL;
                    break;
                case EVENT_INPUT_TRIGGER_PRESSED:
                    if (xTaskGetTickCount() - start_time <= pdMS_TO_TICKS(500))
                    {
                        /* If trigger is pressed 500ms after the boot, we discard and print a warning */
                        LOG_WARN(TAG, "trigger pressed during boot, ignoring");
                        strlcpy(label_buf, "Release trigger!", label_buf_sz);
                        gui_cmd.code = GUI_CMD_UPDATE_HOME_LABEL;
                        boot_triggered = true;
                        break;
                    }

                    LOG_WARN(TAG, "output armed");

                    xSemaphoreTake(g_state.synth_mutex, portMAX_DELAY);
                    synth_enable(g_state.synth);
                    xSemaphoreGive(g_state.synth_mutex);
                    break;
                case EVENT_INPUT_TRIGGER_RELEASED:
                    if (!boot_triggered)
                    {
                        LOG_WARN(TAG, "output disarmed");
                    }

                    xSemaphoreTake(g_state.synth_mutex, portMAX_DELAY);
                    synth_disable(g_state.synth);
                    xSemaphoreGive(g_state.synth_mutex);

                    boot_triggered = false;
                    label_buf[0] = '\0';
                    gui_cmd.code = GUI_CMD_UPDATE_HOME_LABEL;
                    break;
                case EVENT_GUI_PRF_VALUE_CHANGED:
                case EVENT_GUI_PD_VALUE_CHANGED:
                    if (event.code == EVENT_GUI_PRF_VALUE_CHANGED)
                    {
                        g_state.freq = event.payload.val;
                    }
                    if (event.code == EVENT_GUI_PD_VALUE_CHANGED)
                    {
                        atomic_store(&g_state.width, event.payload.val);
                    }
                    if (atomic_load(&g_state.mode) == APP_MODE_MANUAL)
                    {
                        synth_set_channel(
                            g_state.synth, 0, g_state.freq, atomic_load(&g_state.width)
                        );
                    }
                    break;
            }

            if (gui_cmd.code != GUI_CMD_NONE)
            {
                gui_send_command(&gui_cmd);
                gui_cmd.code = GUI_CMD_NONE;
            }
        }
    }
}
