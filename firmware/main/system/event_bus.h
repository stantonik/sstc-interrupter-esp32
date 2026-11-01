/**
 * \file event_bus.h
 * \brief 
 *
 * \author Stanley Arnaud
 * \date 2026-06-12
 */

#ifndef EVENT_BUS_H
#define EVENT_BUS_H

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
#define EVENT_BUS_QUEUE_SIZE 40

/* --------------------------------------------------------------------------
 * Type Definitions
 * -------------------------------------------------------------------------- */
typedef enum
{
    EVENT_NONE = 0,

    /* Input events */
    EVENT_INPUT_TRIGGER_PRESSED,
    EVENT_INPUT_TRIGGER_RELEASED,

    /* USB events */
    EVENT_USB_HOST_ATTACHED,
    EVENT_USB_HOST_DETACHED,
    EVENT_USB_SUSPENDED,
    EVENT_USB_RESUMED,

    /* GUI events */
    EVENT_GUI_PRF_VALUE_CHANGED,
    EVENT_GUI_PD_VALUE_CHANGED,
} event_code_t;

typedef struct
{
    event_code_t code;
    uint32_t timestamp;

    union
    {
        uint64_t val;
        void *ptr;
    } payload;
} event_t;

/* --------------------------------------------------------------------------
 * Function Declarations
 * -------------------------------------------------------------------------- */
int event_bus_init(void);
int event_bus_publish(const event_t *event);
bool event_bus_check(event_t *event, int timeout_ms);

/* --------------------------------------------------------------------------
 * Inline Function Definitions
 * -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !EVENT_BUS_H */
