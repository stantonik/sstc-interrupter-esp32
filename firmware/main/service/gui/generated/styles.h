#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: home_knob_arc
lv_style_t *get_style_home_knob_arc_INDICATOR_DEFAULT();
lv_style_t *get_style_home_knob_arc_MAIN_DEFAULT();
lv_style_t *get_style_home_knob_arc_KNOB_DEFAULT();
lv_style_t *get_style_home_knob_arc_KNOB_FOCUSED();
lv_style_t *get_style_home_knob_arc_KNOB_EDITED();
void add_style_home_knob_arc(lv_obj_t *obj);
void remove_style_home_knob_arc(lv_obj_t *obj);

// Style: mode_label
lv_style_t *get_style_mode_label_MAIN_DEFAULT();
lv_style_t *get_style_mode_label_MAIN_CHECKED();
void add_style_mode_label(lv_obj_t *obj);
void remove_style_mode_label(lv_obj_t *obj);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/