
/**
 * @file main
 *
 */
/*********************
 *      INCLUDES
 *********************/
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>   // for nanosleep
#include "lv_drv_conf.h"
#ifdef PC_SIMULATOR
#include <SDL2/SDL.h>
#include "lv_drivers/display/monitor.h"
#include "lv_drivers/indev/mouse.h"
#include "lv_drivers/indev/mousewheel.h"
#include "lv_drivers/indev/keyboard.h"
#include "lv_examples/lv_apps/demo/demo.h"
#include "lv_examples/lv_apps/benchmark/benchmark.h"
#include "lv_examples/lv_tests/lv_test.h"
#endif
#include "lvgl/lvgl.h"
#include "lv_drivers/display/ST7789.h"
#include "lv_drivers/indev/evdev.h"
#include "lv_drivers/itf/i2c.h"
#include "lv_drivers/itf/spi.h"
#include "lv_drivers/itf/gpio.h"
#include <oegl_app/oe_gui.h>
#include <oegl_app/oe_mqueue.h>
#include <oedd.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
#ifdef PC_SIMULATOR
static void hal_init(void);
static int tick_thread(void * data);
static void memory_monitor(void * param);
#endif
/**********************
 *  STATIC VARIABLES
 **********************/
char *trasport_json;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void sleep_ms(int milliseconds) // cross-platform sleep function
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

void InitGuiLog(void)
{
	setlogmask (LOG_UPTO (LOG_NOTICE));

	openlog (OEGUILOG_NAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	syslog (LOG_NOTICE, "Program started by User %d", getuid ());

	closelog ();
}

void GuiLog(const char *fmt, ...)
{
	va_list arg;

	openlog (OEGUILOG_NAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	va_start (arg, fmt);
	syslog (LOG_NOTICE, fmt, arg);
	va_end (arg);

	closelog ();
}

int main(void)
{

    InitGuiLog();

    /*Initialize LittlevGL*/
    lv_init();

    #ifdef PC_SIMULATOR
    /*Initialize the HAL (display, input devices, tick) for LittlevGL*/
    hal_init();
    #endif

    /*Initialize posix queues */
    if( posix_command_init()!=0 ) {
    	GuiLog("Failed to init posix queue");
       	return -1;
    }
#ifndef PC_SIMULATOR
    if(i2c_init("/dev/i2c-2",0x36)){
    	GuiLog("Failed to init i2c");
       	return -1;
    }

   	if(spi_open("/dev/spidev0.0")){
   		GuiLog("Failed to init spi");
       	return -1;
   	}

   	if(gpio_init("/sys/class/gpio/gpio2_pc5/direction","out","/sys/class/gpio/gpio2_pc5/value",1)){
   		GuiLog("Failed to init gpio");
   		return -1;
   	}

    /*St7789 device init*/
   	if(st7789_init()){
       	GuiLog("Failed to init ST7789");
   		return -1;
   	}

   	evdev_init();

   	init_gui_evdev();

    /*Add a display the LittlevGL sing the frame buffer driver*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.disp_flush = st7789_flush;      /*It flushes the internal graphical buffer to the frame buffer*/
    lv_disp_drv_register(&disp_drv);

#endif

    /*Create a Demo*/
   	init_gui();

    while(1) {
        /* Periodically call the lv_task handler.
         * It could be done in a timer interrupt or an OS task too.*/
        lv_task_handler();
        //sleep_ms(1000);
        usleep(5 * 1000);       /*Just to let the system breath*/
        handle_posix_command();
    }

    return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#ifdef PC_SIMULATOR

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the Littlev graphics library
 */
static void hal_init(void)
{
    /* Add a display
     * Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
    monitor_init();
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
    disp_drv.disp_flush = monitor_flush;    /*Used when `LV_VDB_SIZE != 0` in lv_conf.h (buffered drawing)*/
    disp_drv.disp_fill = monitor_fill;      /*Used when `LV_VDB_SIZE == 0` in lv_conf.h (unbuffered drawing)*/
    disp_drv.disp_map = monitor_map;        /*Used when `LV_VDB_SIZE == 0` in lv_conf.h (unbuffered drawing)*/
    lv_disp_drv_register(&disp_drv);

    /* Add the mouse as input device
     * Use the 'mouse' driver which reads the PC's mouse*/
    mouse_init();
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read = mouse_read;         /*This function will be called periodically (by the library) to get the mouse position and state*/
    lv_indev_t * mouse_indev = lv_indev_drv_register(&indev_drv);

    /*Set a cursor for the mouse*/
    LV_IMG_DECLARE(mouse_cursor_icon);                          /*Declare the image file.*/
    lv_obj_t * cursor_obj =  lv_img_create(lv_scr_act(), NULL); /*Create an image object for the cursor */
    lv_img_set_src(cursor_obj, &mouse_cursor_icon);             /*Set the image source*/
    lv_indev_set_cursor(mouse_indev, cursor_obj);               /*Connect the image  object to the driver*/

    /* Tick init.
     * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about how much time were elapsed
     * Create an SDL thread to do this*/
    SDL_CreateThread(tick_thread, "tick", NULL);

    /* Optional:
     * Create a memory monitor task which prints the memory usage in periodically.*/
    lv_task_create(memory_monitor, 3000, LV_TASK_PRIO_MID, NULL);
}

/**
 * A task to measure the elapsed time for LittlevGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void * data)
{
    (void)data;

    while(1) {
        SDL_Delay(5);   /*Sleep for 5 millisecond*/
        lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
    }

    return 0;
}

/**
 * Print the memory usage periodically
 * @param param
 */
static void memory_monitor(void * param)
{
    (void) param; /*Unused*/

    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    printf("used: %6d (%3d %%), frag: %3d %%, biggest free: %6d\n", (int)mon.total_size - mon.free_size,
           mon.used_pct,
           mon.frag_pct,
           (int)mon.free_biggest_size);
}

#endif
