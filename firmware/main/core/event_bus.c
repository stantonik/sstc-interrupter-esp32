/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file event_bus.c
 * @brief
 *
 * @author Stanley Arnaud
 * @date 10/25/2025
 * @version 0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "event_bus.h"
#include "freertos/FreeRTOS.h"
#include <stdlib.h>

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------
#define TAG "event_bus"

#define EVENT_BUS_QUEUE_LEN 32
#define SUBSCRIBER_PER_SRC_MAX 4

// -----------------------------------------------------------------------------
// Private Typedef
// -----------------------------------------------------------------------------
typedef struct
{
    event_callback_t cb;
    void *user_data;
} subscriber_t;

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------
static QueueHandle_t event_queue = NULL;
static subscriber_t subscribers[EVENT_SRC_COUNT][SUBSCRIBER_PER_SRC_MAX] = {0};
static uint8_t sub_counts[EVENT_SRC_COUNT] = {0};

// -----------------------------------------------------------------------------
// Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Static Function Definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------
esp_err_t event_bus_init(void) 
{
    if (event_queue) return ESP_ERR_INVALID_STATE;

    event_queue = xQueueCreate(EVENT_BUS_QUEUE_LEN, sizeof(event_t)); 
    if (event_queue == NULL) return ESP_ERR_NO_MEM;

    return ESP_OK;
}

esp_err_t event_bus_subscribe(event_source_t source, event_callback_t cb, void *user_data)
{
    if (sub_counts[source] >= SUBSCRIBER_PER_SRC_MAX) return ESP_ERR_NO_MEM;

    subscribers[source][sub_counts[source]].cb = cb;
    subscribers[source][sub_counts[source]].user_data = user_data;
    sub_counts[source]++;

    return ESP_OK;
}

esp_err_t event_bus_publish(const event_t *event)
{
    // safe to call from ISR or task
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t ret;
    if (xPortInIsrContext())
    {
        ret = xQueueSendFromISR(event_queue, event, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    else
    {
        ret = xQueueSend(event_queue, event, 0);
    }
    return ret == pdPASS ? ESP_OK: ESP_ERR_NO_MEM;
}

esp_err_t event_bus_dispatch(event_t *event, TickType_t tick_to_wait)
{
    if (xQueueReceive(event_queue, event, tick_to_wait))
    {
        event_source_t source = event->source;
        for (int i = 0; i < sub_counts[source]; i++)
        {
            subscribers[source][i].cb(event, subscribers[source][i].user_data);
        }
        return ESP_OK;
    }

    return ESP_ERR_TIMEOUT;
}
