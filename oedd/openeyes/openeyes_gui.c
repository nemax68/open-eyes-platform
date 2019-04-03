/**
 * @file openeyes.c
 *
 */

/*
 * ------------------------------------------------
 * Learn how to create GUI elements on the screen
 * ------------------------------------------------
 *
 * The basic building blocks (components or widgets) in LittlevGL are the graphical objects.
 * For example:
 *  - Buttons
 *  - Labels
 *  - Charts
 *  - Sliders etc
 *
 * In this part you can learn the basics of the objects like creating, positioning, sizing etc.
 * You will also meet some different object types and their attributes.
 *
 * Regardless to the object's type the 'lv_obj_t' variable type is used
 * and you can refer to an object with an lv_obj_t pointer (lv_obj_t *)
 *
 * PARENT-CHILD STRUCTURE
 * -------------------------
 * A parent can be considered as the container of its children.
 * Every object has exactly one parent object (except screens).
 * A parent can have unlimited number of children.
 * There is no limitation for the type of the parent.
 *
 * The children are visible only on their parent. The parts outside will be cropped (not displayed)
 *
 * If the parent is moved the children will be moved with it.
 *
 * The earlier created object (and its children) will drawn earlier.
 * Using this layers can be built.
 *
 * INHERITANCE
 * -------------
 * Similarly to object oriented languages some kind of inheritance is used
 * among the object types. Every object is derived from the 'Basic object'. (lv_obj)
 * The types are backward compatible therefore to set the basic parameters (size, position etc.)
 * you can use 'lv_obj_set/get_...()' function.

 * LEARN MORE
 * -------------
 * - General overview: http://www.gl.littlev.hu/objects
 * - Detailed description of types: http://www.gl.littlev.hu/object-types
 */

/*********************
 *      INCLUDES
 *********************/
#include "openeyes_gui.h"
#include "ipc_client.h"
#include "evdev.h"
#include <stdio.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static  lv_res_t btn_rel_action(lv_obj_t * btn);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_style_t scr_style;

static lv_obj_t *scr_wp;
//static lv_style_t btn_style;

//static int	action_in_progress;
static lv_task_t * refr_task;
static int	enabled_button;

static lv_obj_t *button[MAX_BUTTON];
static lv_obj_t *button_label[MAX_BUTTON];
static lv_style_t button_style[MAX_BUTTON];

/**********************
 *      MACROS
 **********************/

uint16_t img_pattern_pixel_map[LV_HOR_RES*LV_VER_RES];

const lv_img_t img_screen = {
  .header.w = 320,			/*Image width in pixel count*/
  .header.h = 240,			/*Image height in pixel count*/
  .header.alpha_byte = 0,		/*No alpha byte*/
  .header.chroma_keyed = 0,	/*No chroma keying*/
  .header.format = LV_IMG_FORMAT_INTERNAL_RAW,	/*It's a variable compiled into the code*/
  .pixel_map = (uint8_t *)img_pattern_pixel_map	/*Pointer the array of image pixels.*/
};

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

extern int32_t Read_bmp2memory ( char const *file_name, uint16_t *picture_pointer  );
extern void GuiLog(const char *fmt, ...);
/**
 * Create some objects
 */
void openeyes_gui(void)
{
	int i;
  int err=0;

	for(i=0;i<MAX_BUTTON;i++){
		button[i]=NULL;
		button_label[i]=NULL;
	}
	enabled_button=0;

	/* EVDEV Touchpanel Initialization */
	lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);     			/*Basic initialization*/
	indev_drv.type = LV_INDEV_TYPE_POINTER;		/*See below.*/
	indev_drv.read = evdev_read;              	/*See below.*/
	lv_indev_drv_register(&indev_drv);     		/*Register the driver in LittlevGL*/

	refr_task = NULL;

  err=Read_bmp2memory ( "/usr/share/oedd/img/logo.bmp", &img_pattern_pixel_map[LV_HOR_RES*LV_VER_RES] );

	if(err!=0){
		GuiLog("Failed to load bitmap error=%d",err);
	}


	scr_wp = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(scr_wp, &img_screen);
    //lv_obj_set_width(wp, LV_HOR_RES * 4);
    //lv_obj_set_protect(wp, LV_PROTECT_NONE);

}

void openeyes_gui_screen(struct oe_screen *oes)
{
	/* CUSTOM SCREEN STYLE */
	lv_style_copy(&scr_style, &lv_style_pretty_color);
	scr_style.text.color = LV_COLOR_WHITE;
	scr_style.text.font = &lv_font_dejavu_30;   /*Unicode and symbol fonts already assigned by the library*/
	scr_style.body.main_color = RGB_2_16bit(oes->main_color);//LV_COLOR_MAKE(0x1E, 0x1E, 0x1E);
	scr_style.body.grad_color = RGB_2_16bit(oes->grad_color);//LV_COLOR_MAKE(0x1E, 0x1E, 0x1E);

	printf("\nGUI SCREEN main color %x",oes->main_color);

	if(scr_wp)
	{
		lv_obj_del(scr_wp);
		scr_wp=NULL;
	}
	/********************
     * CREATE A SCREEN
     *******************/
    /* Create a new screen and load it
     * Screen can be created from any type object type
     * Now a Page is used which is an objects with scrollable content*/
    lv_obj_t *scr = lv_scr_act();
    lv_page_set_style(scr,LV_PAGE_STYLE_BG,&scr_style);
    lv_scr_load(scr);
}

void openeyes_gui_button(int enable, struct oe_button *oeb)
{
	lv_obj_t *call_btn;
	lv_obj_t *name;
	lv_style_t *bstyle;

	if(oeb->id<MAX_BUTTON){

		if(button[oeb->id]==NULL){
			if(!enable)
				return;

			button[oeb->id] = lv_btn_create(lv_scr_act(), NULL);          		/*Create a button on the currently loaded screen*/
			button_label[oeb->id] = lv_label_create(button[oeb->id], NULL);		/*Create a label on the current button*/
			lv_style_copy(&button_style[oeb->id], &lv_style_pretty_color);		/*Create style */

			button[oeb->id]->free_num=oeb->id;									/* link obj_id */
		}

		call_btn = button[oeb->id];
		name = button_label[oeb->id];
		bstyle = &button_style[oeb->id];

		if(!enable)
		{
			// remove button
			printf("\nRemove button");
			lv_obj_del(call_btn);

			button[oeb->id]=NULL;

			return;
		}

		/* CUSTOM BUTTON STYLE */

		bstyle->text.color = RGB_2_16bit(oeb->font_color);

		switch(oeb->font_size){
			case 10: bstyle->text.font = &lv_font_dejavu_10; break;
			case 20: bstyle->text.font = &lv_font_dejavu_20; break;
			case 30: bstyle->text.font = &lv_font_dejavu_30; break;
			default: bstyle->text.font = &lv_font_dejavu_40;
		}

		bstyle->body.main_color = RGB_2_16bit(oeb->main_color);
		bstyle->body.grad_color = RGB_2_16bit(oeb->grad_color);
		bstyle->body.radius = oeb->border_radius;
		bstyle->body.border.color = RGB_2_16bit(oeb->border_color);
		bstyle->body.border.width = oeb->border_size;

		lv_btn_set_action(call_btn, LV_BTN_ACTION_CLICK, btn_rel_action); /*Set function to be called when the button is released*/

		lv_obj_set_size(call_btn,oeb->size_w,oeb->size_h);
		lv_obj_set_pos(call_btn,oeb->pos_x,oeb->pos_y);
		lv_obj_set_style(call_btn, bstyle);

		lv_label_set_text(name, oeb->text);

printf("\ncreate button");
		//lv_label_set_text(name, SYMBOL_KEYBOARD);
	}

	//printf("\nGUI BUTTON style %p - id=%d",bstyle,button[oeb->id]->free_num);

}


/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Called when a button is released
 * @param btn pointer to the released button
 * @return LV_RES_OK because the object is not deleted in this function
 */
static  void call_close(void * btn)
{
	lv_color_t	color;
	lv_style_t	*bstyle;
	lv_obj_t	*lvbtn = (lv_obj_t *)btn;
	uint32_t	id=lvbtn->free_num;
	int			idx=id&0xFF;

	//printf("\n CALL CLOSE id=%lx",idx);

	if(	! (id&BUTTON_OPERATION_RUNNING) )
		return;

	if(idx<MAX_BUTTON){
		// point to button specific style
		bstyle = &button_style[idx];

		/*Swap the button color*/
		color = bstyle->body.main_color;
		bstyle->body.main_color = bstyle->body.border.color;
		bstyle->body.border.color = color;

		lv_obj_set_style((lv_obj_t *)btn, bstyle);

		// Call close here
		//printf("Call close\n");
		//action_in_progress = 0;
		lvbtn->free_num = id & ~BUTTON_OPERATION_RUNNING;

		if(refr_task)
			lv_task_del(refr_task);

	}
}

/**
 * Called when a button is released
 * @param btn pointer to the released button
 * @return LV_RES_OK because the object is not deleted in this function
 */
static  lv_res_t btn_rel_action(lv_obj_t * btn)
{
	lv_color_t	color;
	lv_style_t	*bstyle;
	uint32_t	id=btn->free_num;
	int			idx=id&0xFF;
	char		event[16];

	printf("\button");

	if(	id&BUTTON_OPERATION_RUNNING )
		return LV_RES_OK;

	if(idx<MAX_BUTTON){
		// point to button specific style
		bstyle = &button_style[idx];

		/*Swap the button color*/
		color = bstyle->body.main_color;
		bstyle->body.main_color = bstyle->body.border.color;
		bstyle->body.border.color = color;

		lv_obj_set_style(btn, bstyle);

		// Call progress here
		btn->free_num = id | BUTTON_OPERATION_RUNNING;

#if GUI_USE_IPC
		sprintf(event,"BUTTON,%d",idx);
		ipc_send_event(event);
#endif

#if GUI_USE_HTTP
		http_send_event(HTTP_TYPE_POST,"button=on");
#endif

		//printf("\nCall progress br=%d style %p id=%d\n",bstyle->body.radius,bstyle,btn->free_num);

		refr_task=lv_task_create(call_close, 1000, LV_TASK_PRIO_MID, btn);
	}
    return LV_RES_OK;
}
