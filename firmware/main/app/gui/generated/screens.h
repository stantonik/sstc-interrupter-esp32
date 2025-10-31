#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _groups_t {
    lv_group_t *range_group;
} groups_t;

extern groups_t groups;

void ui_create_groups();

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *aux_icon;
    lv_obj_t *aux_type_label;
    lv_obj_t *header_label;
    lv_obj_t *armed_icon;
    lv_obj_t *state_label;
    lv_obj_t *pd_arc;
    lv_obj_t *prf_arc;
    lv_obj_t *gdb_arc;
    lv_obj_t *pwr_arc;
    lv_obj_t *message_box;
    lv_obj_t *obj0;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
};

void create_screen_main();
void tick_screen_main();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/