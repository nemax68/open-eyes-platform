/*
 * oe_button.c
 *
 *  Created on: 06/mag/2019
 *      Author: maxn
 */


/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "oe_gui.h"
#include "oe_json.h"

/*********************
 *      DEFINES
 *********************/

#define MAX_BUTTON_DESCRIPTOR		8
#define BUTTON_OPERATION_RUNNING	0x80000000

/**********************
 *      TYPEDEFS
 **********************/

struct btn_desc {
	char name[64];
	lv_task_t * refr_task;
	lv_obj_t *button;
	lv_obj_t *button_label;
	lv_style_t button_style;
};

struct buttons_desc {
	struct btn_desc *id[MAX_BUTTON_DESCRIPTOR];
};

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

static struct buttons_desc bd;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int init_button(void)
{
	memset(&bd,0,sizeof(bd));
	return 0;
}

/**
 * Called when a button is released
 * @param btn pointer to the released button
 * @return LV_RES_OK because the object is not deleted in this function
 */
static  void btn_close(void * btn)
{
	struct btn_desc 	*p;
	lv_color_t			color;
	lv_style_t			*bstyle;
	lv_obj_t			*lvbtn = (lv_obj_t *)btn;
	uint32_t			id=lvbtn->free_num;
	int					idx=id&0xFF;

	if(	! (id&BUTTON_OPERATION_RUNNING) )
		return;

	if(idx<MAX_BUTTON_DESCRIPTOR){
		p=bd.id[idx];
		/*point to button specific style */
		bstyle = &p->button_style;

		/*Swap the button color*/
		color = bstyle->body.main_color;
		bstyle->body.main_color = bstyle->body.border.color;
		bstyle->body.border.color = color;

		/*Update button color */
		lv_obj_set_style((lv_obj_t *)btn, bstyle);

		/*delete task */
		lvbtn->free_num = id & ~BUTTON_OPERATION_RUNNING;
		if(p->refr_task)
			lv_task_del(p->refr_task);
	}
}

/**
 * Called when a button is released
 * @param btn pointer to the released button
 * @return LV_RES_OK because the object is not deleted in this function
 */
static  lv_res_t btn_action(lv_obj_t * btn)
{
	lv_color_t				color;
	lv_style_t				*bstyle;
	uint32_t				id=btn->free_num;
	int						idx=id&0xFF;
	struct btn_desc 		*p;
	struct json_encoder		je;

	if(	id&BUTTON_OPERATION_RUNNING )
		return LV_RES_OK;

	if(idx<MAX_BUTTON_DESCRIPTOR){
		p=bd.id[id];
		/*point to button specific style */
		bstyle = &p->button_style;

		/*Swap the button color*/
		color = bstyle->body.main_color;
		bstyle->body.main_color = bstyle->body.border.color;
		bstyle->body.border.color = color;

		lv_obj_set_style(btn, bstyle);

		/*Call progress here */
		btn->free_num = id | BUTTON_OPERATION_RUNNING;

		printf("press button %s",p->name);
		memset(&je,0,sizeof(struct json_encoder));
		strcpy(je.type,"button");
		strcpy(je.event,"press");
		strcpy(je.name,p->name);
		json_encoder(&je);

		p->refr_task=lv_task_create(btn_close, 1000, LV_TASK_PRIO_MID, btn);
	}
    return LV_RES_OK;
}


/**
 * Create button objects
 *
 * JSON format:
 * {
 *  "type"  :  "addbutton",
 *  "position"  :  { "x" : "", "y" : "" },
 *
 */

int button_add(struct json_decoder *jsond)
{
	int i;
	int rgb;
	struct btn_desc *p;

	for (i=0;i<MAX_BUTTON_DESCRIPTOR;i++) {
		if(bd.id[i]==NULL)
			break;
	}

	if(i==MAX_BUTTON_DESCRIPTOR)
		return(ENOMEM);

	bd.id[i]=malloc(sizeof(struct btn_desc));
	if(bd.id[i]==NULL)
		return(ENOMEM);

	p=bd.id[i];

	strcpy(p->name,jsond->name);

	printf("Create button (%s) (%s)",p->name,jsond->name);

	p->button = lv_btn_create(lv_scr_act(), NULL);          		/*Create a button on the currently loaded screen*/
	p->button_label = lv_label_create(p->button, NULL);				/*Create a label on the current button*/
	lv_style_copy(&p->button_style, &lv_style_pretty_color);		/*Create style */
	p->button->free_num=i;													/*link obj_id */
	/* CUSTOM BUTTON STYLE */

	rgb=color_conv(jsond->font.color);
	p->button_style.text.color = RGB_2_16bit(rgb);

	switch(jsond->font.size){
		case 10: p->button_style.text.font = &lv_font_dejavu_10; break;
		case 20: p->button_style.text.font = &lv_font_dejavu_20; break;
		case 30: p->button_style.text.font = &lv_font_dejavu_30; break;
		default: p->button_style.text.font = &lv_font_dejavu_40;
	}

	rgb=color_conv(jsond->color.main);
	p->button_style.body.main_color = RGB_2_16bit(rgb);
	rgb=color_conv(jsond->color.grad);
	p->button_style.body.grad_color = RGB_2_16bit(rgb);
	p->button_style.body.radius = jsond->border.radius;
	rgb=color_conv(jsond->border.color);
	p->button_style.body.border.color = RGB_2_16bit(rgb);
	p->button_style.body.border.width = jsond->border.size;

	lv_btn_set_action(p->button, LV_BTN_ACTION_CLICK, btn_action); /*Set function to be called when the button is released*/

	lv_obj_set_size(p->button,jsond->size.x,jsond->size.y);
	lv_obj_set_pos(p->button,jsond->pos.x, jsond->pos.y);
	lv_obj_set_style(p->button, &p->button_style);

	lv_label_set_text(p->button_label, jsond->text);

	return 0;
}

/**
 * Delete button
 *
 * JSON format:
 * {
 *  "type"  :  "delbutton",
 *  "name" :  "button_name"
 * }
 *
 * Desc: remove all instance with matching name
 *
 */

int button_del(struct json_decoder *jsond)
{
	int i;

	for (i=0;i<MAX_BUTTON_DESCRIPTOR;i++) {
		if (bd.id[i]) {
			if (strcmp(bd.id[i]->name,jsond->name)==0) {
				lv_obj_del(bd.id[i]->button);
				free(bd.id[i]);
				bd.id[i]=NULL;
			}
		}
	}

    return 0;
}
