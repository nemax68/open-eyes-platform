/**
 * @file screen.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "oe_mqueue.h"
#include "bmp.h"
#include "oe_gui.h"
#include "oe_json.h"
#include "lv_drivers/indev/evdev.h"

/*********************
 *      DEFINES
 *********************/

#define MAX_IMAGE_DESCRIPTOR	8

/**********************
 *      TYPEDEFS
 **********************/

struct image_desc {
	char name[64];
	lv_img_dsc_t img_screen;
	lv_obj_t *scr_wp;
	struct bmp_file desc;
};

struct screen_desc {
	lv_style_t scr_style;
	struct image_desc *id[MAX_IMAGE_DESCRIPTOR];
};

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

static struct screen_desc sd;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

extern void GuiLog(const char *fmt, ...);


int init_image(void)
{
	memset(&sd,0,sizeof(sd));
	return 0;
}



/**
 * Create some objects
 *
 * JSON format:
 * {
 *  "type"  :  "image",
 *  "path"  :  "/../..",
 *  "position"  :  { "x" : "", "y" : "" },
 *
 */

int image_add(struct json_decoder *jsond)
{
	struct image_desc	*p;
	int err=0;
	int i;

	for (i=0;i<MAX_IMAGE_DESCRIPTOR;i++) {
		if(sd.id[i]==NULL)
			break;
	}

	if(i==MAX_IMAGE_DESCRIPTOR)
		return(ENOMEM);

	p=malloc(sizeof(struct image_desc));
	if(!p)
		return(ENOMEM);

	sd.id[i]=p;

	strcpy(p->name,jsond->name);

	err=Read_bmp2memory ( jsond->path, &p->desc );

	if(err!=0){
		GuiLog("Failed to load bitmap error=%d",err);
		return err;
	}

	p->img_screen.header.always_zero=0;
	p->img_screen.data = p->desc.img;
	p->img_screen.data_size= p->desc.height * p->desc.width * LV_COLOR_SIZE / 8;
	p->img_screen.header.cf=LV_IMG_CF_TRUE_COLOR;
	p->img_screen.header.w=p->desc.width;
	p->img_screen.header.h=p->desc.height;

	p->scr_wp = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(p->scr_wp, &p->img_screen);
    lv_obj_set_pos(p->scr_wp, jsond->pos.x, jsond->pos.y);      /*Set the positions*/

    return 0;
}

/**
 * delete image
 *
 * JSON format:
 * {
 *  "type"  :  "delimage",
 *  "name" :  "image_name"
 * }
 *
 * Desc: remove all instance with matching name
 *
 */

int image_del(struct json_decoder *jsond)
{
	int i;

	for (i=0;i<MAX_IMAGE_DESCRIPTOR;i++) {
		if (sd.id[i]!=NULL) {
			if (strcmp(sd.id[i]->name,jsond->name)==0) {
				lv_obj_del(sd.id[i]->scr_wp);
				free(sd.id[i]);
				sd.id[i]=NULL;
				return 0;
			}
		}
	}

    return 0;
}

/**
 * Clear screen
 *
 * JSON format:
 * {
 *  "type"  :  "image",
 *  "color" :  { "main" : "#000000", "gradient" : "#000000" }
 *
 */

int clear_screen(struct json_decoder *jsond)
{
	int i;
	int rgb;

	for (i=0;i<MAX_IMAGE_DESCRIPTOR;i++) {
		if (sd.id[i]!=NULL) {
			lv_obj_del(sd.id[i]->scr_wp);
			free(sd.id[i]);
			sd.id[i]=NULL;
		}
	}

	/* CUSTOM SCREEN STYLE */
	lv_style_copy(&sd.scr_style, &lv_style_pretty_color);

	sd.scr_style.text.color = LV_COLOR_WHITE;
	sd.scr_style.text.font = &lv_font_dejavu_30;   /*Unicode and symbol fonts already assigned by the library*/
	rgb=color_conv(jsond->color.main);
	sd.scr_style.body.main_color = RGB_2_16bit(rgb);
	rgb=color_conv(jsond->color.grad);
	sd.scr_style.body.grad_color = RGB_2_16bit(rgb);

	/********************
     * CREATE A SCREEN
     *******************/
    /* Create a new screen and load it
     * Screen can be created from any type object type
     * Now a Page is used which is an objects with scrollable content*/
    lv_obj_t *scr = lv_scr_act();
    lv_page_set_style(scr,LV_PAGE_STYLE_BG,&sd.scr_style);
    lv_scr_load(scr);

    return 0;
}
