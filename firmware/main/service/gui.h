/**
 * \file gui.h
 * \brief 
 *
 * \author Stanley Arnaud
 * \date 2026-06-04
 */

#ifndef GUI_H
#define GUI_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include <stdint.h>

#include "driver/button.h"
#include "driver/display.h"
#include "driver/rotary_encoder.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

// -----------------------------------------------------------------------------
// Macros and constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Type Definitions
// -----------------------------------------------------------------------------

/**
 * \brief Graphic event type
 */
typedef enum
{
    GUI_CMD_NONE = 0,

    GUI_CMD_SET_MODE_MIDI,
    GUI_CMD_SET_MODE_MANUAL,

    GUI_CMD_UPDATE_HOME_LABEL,
} gui_cmd_code_t;

typedef struct
{
    gui_cmd_code_t code;

    union
    {
        int32_t val;
        void *ptr;
    } payload;
} gui_cmd_t;

typedef struct
{
    display_t *display;
    re_t *re;
    btn_t *trigger;
} gui_ctx_t;

// -----------------------------------------------------------------------------
// Function Declarations
// -----------------------------------------------------------------------------
int gui_init(const gui_ctx_t *ctx);
int gui_send_command(gui_cmd_t *cmd);
int gui_sleep(void);
int gui_wake(void);

void gui_get_home_label_buf(char **out_buf, size_t *out_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !GUI_H */
