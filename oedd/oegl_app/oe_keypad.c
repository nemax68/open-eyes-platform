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

#define MAX_KEYPAD_DESCRIPTOR		1
#define KEYPAD_OPERATION_RUNNING	0x40000000

#define KEYPAD_CODE_LEN				32
/**********************
 *      TYPEDEFS
 **********************/

struct kpd_desc {
	char name[64];
	char code[KEYPAD_CODE_LEN];
	char key_text[KEYPAD_CODE_LEN];
	lv_task_t * refr_task;
	lv_obj_t *keypad;
	lv_style_t keypad_style;
};

struct keypad_desc {
	struct kpd_desc *id[MAX_KEYPAD_DESCRIPTOR];
};

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

static struct keypad_desc kd;

/*Create a button descriptor string array*/
static const char * keyp_map[] = {"1", "2", "3", "4", "\n",
                           "5", "6", "7", "8", "\n",
                           "9", "0", "\202ENTER", ""};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int init_keypad(void)
{
	memset(&kd,0,sizeof(kd));
	return 0;
}

/**
 * Called when a button is released
 * @param btn pointer to the released button
 * @return LV_RES_OK because the object is not deleted in this function
 */
static  lv_res_t keypad_action(lv_obj_t * btn, const char *txt)
{
	struct kpd_desc 		*p;
	struct json_encoder		je;
	uint32_t				id=btn->free_num;
	int						idx=id&0xFF;

	if(idx<MAX_KEYPAD_DESCRIPTOR){
		p=kd.id[id];

		// sound
		if(strcmp(txt,"ENTER")==0){
			printf("[%s]\n",p->code);

			memset(&je,0,sizeof(struct json_encoder));

			if(strcmp(p->code,p->key_text)==0)
				strcpy(je.event,"correct");
			else
				strcpy(je.event,"errored");

			strcpy(je.type,"keypad");
			strcpy(je.name,p->name);
			json_encoder(&je);

			memset(p->code,0,KEYPAD_CODE_LEN);
		}
		else
		{
			strcat(p->code,txt);
			if(strlen(p->code)>(KEYPAD_CODE_LEN-2))
				memset(p->code,0,KEYPAD_CODE_LEN);
		}
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

int keypad_add(struct json_decoder *jsond)
{
	int i;
	int rgb;
	struct kpd_desc *p;

	for (i=0;i<MAX_KEYPAD_DESCRIPTOR;i++) {
		if(kd.id[i]==NULL)
			break;
	}

	if(i==MAX_KEYPAD_DESCRIPTOR)
		return(ENOMEM);

	kd.id[i]=malloc(sizeof(struct kpd_desc));
	if(kd.id[i]==NULL)
		return(ENOMEM);

	p=kd.id[i];

	memset(p,0,sizeof(struct kpd_desc));
	strcpy(p->name,jsond->name);
	strcpy(p->key_text,jsond->text);


	/*Create a default button matrix*/
	p->keypad = lv_btnm_create(lv_scr_act(), NULL);
	lv_btnm_set_map(p->keypad, keyp_map);


	lv_style_copy(&p->keypad_style, &lv_style_pretty_color);		/*Create style */
	p->keypad->free_num=i;											/*link obj_id */
	/* CUSTOM BUTTON STYLE */


	rgb=color_conv(jsond->font.color);
	p->keypad_style.text.color = RGB_2_16bit(rgb);

	switch(jsond->font.size){
		case 10: p->keypad_style.text.font = &lv_font_dejavu_10; break;
		case 20: p->keypad_style.text.font = &lv_font_dejavu_20; break;
		case 30: p->keypad_style.text.font = &lv_font_dejavu_30; break;
		default: p->keypad_style.text.font = &lv_font_dejavu_40;
	}


	rgb=color_conv(jsond->color.main);
	p->keypad_style.body.main_color = RGB_2_16bit(rgb);
	rgb=color_conv(jsond->color.grad);
	p->keypad_style.body.grad_color = RGB_2_16bit(rgb);
	p->keypad_style.body.radius = jsond->border.radius;
	rgb=color_conv(jsond->border.color);
	p->keypad_style.body.border.color = RGB_2_16bit(rgb);
	p->keypad_style.body.border.width = jsond->border.size;

	//lv_btn_set_action(p->button, LV_BTN_ACTION_CLICK, keypad_action); /*Set function to be called when the button is released*/
	lv_btnm_set_action(p->keypad, keypad_action);

	lv_obj_set_size(p->keypad,jsond->size.x,jsond->size.y);
	lv_obj_set_pos(p->keypad,jsond->pos.x, jsond->pos.y);
	//lv_obj_set_style(p->keypad, &p->button_style);
	lv_btnm_set_style(p->keypad, LV_BTNM_STYLE_BTN_REL, &p->keypad_style);
	//lv_label_set_text(p->button_label, jsond->text);

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

int keypad_del(struct json_decoder *jsond)
{
	int i;

	for (i=0;i<MAX_KEYPAD_DESCRIPTOR;i++) {
		if (kd.id[i]) {
			if (strcmp(kd.id[i]->name,jsond->name)==0) {
				lv_obj_del(kd.id[i]->keypad);
				free(kd.id[i]);
				kd.id[i]=NULL;
			}
		}
	}

    return 0;
}
