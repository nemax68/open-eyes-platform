/**
 * @file evdev.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "evdev.h"
#if USE_EVDEV != 0

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

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
int evdev_fd;
int evdev_root_x;
int evdev_root_y;
int evdev_button;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the evdev interface
 */
void evdev_init(void)
{
    evdev_fd = open(EVDEV_NAME, O_RDWR|O_NOCTTY|O_NDELAY);
    if (evdev_fd == -1) {
        perror("unable open evdev interface:");
        return;
    }

    fcntl(evdev_fd, F_SETFL, O_ASYNC|O_NONBLOCK);

    evdev_root_x = 0;
    evdev_root_y = 0;
    evdev_button = LV_INDEV_STATE_REL;
}

#define TP_X_MIN	25
#define TP_Y_MIN	25
#define TP_X_MAX	3400
#define TP_Y_MAX	3400

#define TP_X_SCALE  ((TP_X_MAX)/LV_HOR_RES)
#define TP_Y_SCALE  ((TP_Y_MAX)/LV_VER_RES)

/**
 * Get the current position and state of the evdev
 * @param data store the evdev data here
 * @return false: because the points are not buffered, so no more data to be read
 */
bool evdev_read(lv_indev_data_t * data)
{
    struct input_event in;

    while(read(evdev_fd, &in, sizeof(struct input_event)) > 0) {
        if (in.type == EV_REL) {
        	/* MOUSE NOT SUPPORTED
            if (in.code == REL_X)
                evdev_root_x += in.value;
            else if (in.code == REL_Y)
                evdev_root_y += in.value;
            */
        } else if (in.type == EV_ABS) {
            if (in.code == ABS_X)
                evdev_root_y = in.value;	// x/y swap
            else if (in.code == ABS_Y)
                evdev_root_x = in.value;
        } else if (in.type == EV_KEY) {
            if (in.code == BTN_MOUSE || in.code == BTN_TOUCH) {
                if (in.value == 0)
                    evdev_button = LV_INDEV_STATE_REL;
                else if (in.value == 1)
                    evdev_button = LV_INDEV_STATE_PR;
            }
        }
    }

    evdev_root_x = evdev_root_x/TP_X_SCALE;
    evdev_root_y = evdev_root_y/TP_Y_SCALE;

    evdev_root_x = evdev_root_x - TP_X_MIN;
    evdev_root_y = evdev_root_y - TP_Y_MIN;

	if (evdev_root_x < 0)
		evdev_root_x = 0;
	if (evdev_root_y < 0)
		evdev_root_y = 0;

	if (evdev_root_x >= LV_HOR_RES)
		evdev_root_x = LV_HOR_RES - 1;
	if (evdev_root_y >= LV_VER_RES)
		evdev_root_y = LV_VER_RES - 1;

    evdev_root_x = 320-evdev_root_x;

#ifdef notdef
    if( evdev_root_x!=0 && evdev_root_y!=0 )
    	printf("EVDEV %d %d %d\n",evdev_root_x,evdev_root_y,evdev_button );
#endif
     /*Store the collected data*/
    data->point.x = evdev_root_x;
    data->point.y = evdev_root_y;
    data->state = evdev_button;

    return false;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
