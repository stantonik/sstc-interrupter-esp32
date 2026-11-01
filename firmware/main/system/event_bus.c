/**
 * \file event.c
 * \brief 
 *
 * \author Stanley Arnaud
 * \date 2026-06-12
 */

/* --------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------- */
#include "event_bus.h"
#include "system/error.h"
// #include "system/logging.h"

#include "freertos/idf_additions.h"

/* --------------------------------------------------------------------------
 * Macro Definitions
 * -------------------------------------------------------------------------- */
#define TAG           "event bus"

/* --------------------------------------------------------------------------
 * Static Variables
 * -------------------------------------------------------------------------- */
static struct
{
    QueueHandle_t queue;
    event_t queue_buf[EVENT_BUS_QUEUE_SIZE];
    StaticQueue_t queue_ctrl;
} state = {0};

/* --------------------------------------------------------------------------
 * Function Definitions
 * -------------------------------------------------------------------------- */
int
event_bus_init(void)
{
    state.queue = xQueueCreateStatic(
        EVENT_BUS_QUEUE_SIZE, sizeof(event_t), (uint8_t *)state.queue_buf, &state.queue_ctrl
    );
    if (state.queue == NULL)
    {
        // LOG_ERROR(TAG, "Failed to create event queue");
        return RET_ERR_FAIL;
    }

    // LOG_INFO(TAG, "Initialized successfully");

    return RET_OK;
}

int
event_bus_publish(const event_t *event)
{
    event_t event_cpy = *event;
    event_cpy.timestamp = xTaskGetTickCount();

    if (xQueueSend(state.queue, &event_cpy, 10) != pdTRUE)
    {
        return RET_ERR_TIMEOUT;
    }

    return RET_OK;
}

bool
event_bus_check(event_t *event, int timeout_ms)
{
    return xQueueReceive(state.queue, event, timeout_ms) == pdTRUE;
}
