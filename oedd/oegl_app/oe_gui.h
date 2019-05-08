/**
 * @file openeyes.h
 *
 */

#ifndef OPENEYES_H
#define OPENEYES_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lv_ex_conf.h"
#include "../lvgl/lvgl.h"
#include "oe_json.h"

/*********************
 *      DEFINES
 *********************/

#define GUI_USE_IPC			1
#define GUI_USE_HTTP		0

#define RGB_2_16bit(x) LV_COLOR_MAKE( (x>>16)&0xFF, (x>>8)&0xFF, (x)&0xFF)


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void init_gui_evdev(void);
void init_gui(void);
uint32_t color_conv(char *);

int init_image(void);
int image_add(struct json_decoder *);
int image_del(struct json_decoder *);
int clear_screen(struct json_decoder *);

int init_keypad(void);
int keypad_add(struct json_decoder *);
int keypad_del(struct json_decoder *);

int init_button(void);
int button_add(struct json_decoder *);
int button_del(struct json_decoder *);

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* OPENEYES_H */
