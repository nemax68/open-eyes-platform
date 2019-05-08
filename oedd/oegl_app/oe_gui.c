/**
 * @file openeyes_gui.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <stdlib.h>
#include "lv_drivers/indev/evdev.h"
#include "oe_gui.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/


/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create some objects
 */
void init_gui_evdev(void)
{
	/* EVDEV Touchpanel Initialization */
	lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);     			/*Basic initialization*/
	indev_drv.type = LV_INDEV_TYPE_POINTER;		/*See below.*/
	indev_drv.read = evdev_read;              	/*See below.*/
	lv_indev_drv_register(&indev_drv);     		/*Register the driver in LittlevGL*/
}

void init_gui(void)
{
	struct json_decoder jd;

	init_image();
	init_button();
	init_keypad();

	/*Display logo full screen */
	memset(&jd,0,sizeof(jd));
	strcpy(jd.path,"/usr/share/oedd/img/logo.bmp");
	image_add(&jd);
}

uint32_t color_conv(char *c)
{
	int r,g,b;
	char col[3];
	char *pt=c;

	col[0]=pt[1];
	col[1]=pt[2];
	col[2]=0;
	r=strtol(col,NULL,16);

	col[0]=pt[3];
	col[1]=pt[4];
	g=strtol(col,NULL,16);

	col[0]=pt[5];
	col[1]=pt[6];
	b=strtol(col,NULL,16);

	return( (r<<16) | (g<<8) | b );
}
