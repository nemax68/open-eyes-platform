#include "lvgl/lvgl.h"
#include "lv_drivers/display/ST7789.h"
#include "lv_drivers/indev/evdev.h"
#include "lv_drivers/itf/i2c.h"
#include "lv_drivers/itf/spi.h"
#include "lv_drivers/itf/gpio.h"
#include "lv_examples/lv_tutorial/1_hello_world/lv_tutorial_hello_world.h"
#include "lv_examples/lv_tutorial/2_objects/lv_tutorial_objects.h"
#include "lv_examples/lv_tests/lv_test_obj/lv_test_obj.h"
#include "lv_examples/lv_apps/tpcal/tpcal.h"
#include "openeyes/openeyes_gui.h"
#include "openeyes/ipc_common.h"
#include "openeyes/ipc_client.h"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <oedd.h>
#include <stdarg.h>
#include <time.h>   // for nanosleep

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
	char posix_cmd[MAX_MSG_SIZE];
	int posix_ret;
	struct oe_button oeb;
	struct oe_cmd oec;
    /*LittlevGL init*/

	oeb.border_color=0xff0000;
	oeb.border_radius=5;
	oeb.border_size=10;
	oeb.font_color=0xff0000;
	oeb.font_size=30;
	oeb.grad_color=0xffffff;
	oeb.main_color=0x0000ff;
	oeb.pos_x=20;
	oeb.pos_y=20;
	oeb.size_h=100;
	oeb.size_w=200;
	strcpy(oeb.text,"Caislaghi A.\nNegretti M.");


	InitGuiLog();

    lv_init();

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

/*#if GUI_USE_IPC
    while(ipc_send_event(EVENT_GUI_START_APP))
    {
     	usleep(100000);
    }
#endif
*/
    /*Add a display the LittlevGL sing the frame buffer driver*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.disp_flush = st7789_flush;      /*It flushes the internal graphical buffer to the frame buffer*/
    lv_disp_drv_register(&disp_drv);

    /*Create a Demo*/
    openeyes_gui();

    /*Handle LitlevGL tasks (tickless mode)*/
    while(1) {
    	//printf("\n%d",cnt++);
        lv_tick_inc(5);
        lv_task_handler();
        sleep_ms(5);

        posix_ret=ipc_receive_event(posix_cmd);
        if(posix_ret>0){
        	printf("\n%s",posix_cmd);
        	if( extract_ipc_cmd(posix_cmd,&oec)==0 )
        	{
        		switch(oec.type){
        			case OE_BUTTON_TYPE:
        		   		openeyes_gui_button(1,&oec.oeu.oeb);
        		   		break;
        			case OE_DELBUTTON_TYPE:
        			    openeyes_gui_button(0,&oec.oeu.oeb);
        			    break;
        			case OE_SCREEN_TYPE:
        				openeyes_gui_screen(&oec.oeu.oes);
        		   		break;
        		}
        	}
        }
    }

    return 0;
}
