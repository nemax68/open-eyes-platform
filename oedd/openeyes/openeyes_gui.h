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
#include "../../../lv_ex_conf.h"
#include "../../../lvgl/lvgl.h"
#include "ipc_common.h"
/*********************
 *      DEFINES
 *********************/

#define GUI_USE_IPC			1
#define GUI_USE_HTTP		0

#define RGB_2_16bit(x) LV_COLOR_MAKE( (x>>16)&0xFF, (x>>8)&0xFF, (x)&0xFF)

#define BUTTON_OPERATION_RUNNING	0x80000000

#define MAX_BUTTON			10
#define OE_BUTTON_TYPE		1
#define OE_SCREEN_TYPE		2
#define OE_DELBUTTON_TYPE	3

/**********************
 *      TYPEDEFS
 **********************/

struct oe_button
{
	uint16_t size_w;
	uint16_t size_h;
	uint16_t pos_x;
	uint16_t pos_y;
	uint8_t id;
	uint8_t font_size;
	uint8_t border_size;
	uint8_t border_radius;
	uint32_t font_color;
	uint32_t main_color;
	uint32_t grad_color;
	uint32_t border_color;
	char text[64];
};

struct oe_screen
{
	uint8_t id;
	uint8_t font_size;
	uint32_t font_color;
	uint32_t main_color;
	uint32_t grad_color;
};

struct oe_cmd
{
	uint8_t	type;
	union oec_ud{
		struct oe_button oeb;
		struct oe_screen oes;
	} oeu;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void openeyes_gui(void);
void openeyes_gui_button(int, struct oe_button *);
void openeyes_gui_screen(struct oe_screen *);


/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* OPENEYES_H */
