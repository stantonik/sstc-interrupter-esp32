#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: range_value_style
lv_style_t *get_style_range_value_style_MAIN_DEFAULT();
void add_style_range_value_style(lv_obj_t *obj);
void remove_style_range_value_style(lv_obj_t *obj);

// Style: range_name_style
lv_style_t *get_style_range_name_style_MAIN_DEFAULT();
void add_style_range_name_style(lv_obj_t *obj);
void remove_style_range_name_style(lv_obj_t *obj);

// Style: range_arc_style
lv_style_t *get_style_range_arc_style_INDICATOR_DEFAULT();
lv_style_t *get_style_range_arc_style_MAIN_DEFAULT();
lv_style_t *get_style_range_arc_style_KNOB_DEFAULT();
lv_style_t *get_style_range_arc_style_KNOB_EDITED();
void add_style_range_arc_style(lv_obj_t *obj);
void remove_style_range_arc_style(lv_obj_t *obj);

// Style: header_label_style
lv_style_t *get_style_header_label_style_MAIN_DEFAULT();
void add_style_header_label_style(lv_obj_t *obj);
void remove_style_header_label_style(lv_obj_t *obj);

// Style: range_selector_style
lv_style_t *get_style_range_selector_style_MAIN_DEFAULT();
lv_style_t *get_style_range_selector_style_MAIN_FOCUSED();
void add_style_range_selector_style(lv_obj_t *obj);
void remove_style_range_selector_style(lv_obj_t *obj);

// Style: header_item_style
lv_style_t *get_style_header_item_style_MAIN_DEFAULT();
void add_style_header_item_style(lv_obj_t *obj);
void remove_style_header_item_style(lv_obj_t *obj);



#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/