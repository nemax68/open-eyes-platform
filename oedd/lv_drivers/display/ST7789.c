/**
 * @file ST7789.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "ST7789.h"
#if USE_ST7789

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>

#include "lvgl/lv_core/lv_vdb.h"
#include "lv_drivers/itf/i2c.h"
#include "lv_drivers/itf/spi.h"
#include "lv_drivers/itf/gpio.h"

#include LV_DRV_DISP_INCLUDE
#include LV_DRV_DELAY_INCLUDE

/*********************
 *      DEFINES
 *********************/

//#define ST_GPIO_FILE_VALUE "/sys/class/gpio/gpio2_pc5/value"


//#define ST7789_BAUD      2000000    /*< 2,5 MHz (400 ns)*/



#define CMD_DISPLAY_OFF         0xAE
#define CMD_DISPLAY_ON          0xAF

#define CMD_SET_DISP_START_LINE 0x40
#define CMD_SET_PAGE            0xB0

#define CMD_SET_COLUMN_UPPER    0x10
#define CMD_SET_COLUMN_LOWER    0x00

#define CMD_SET_ADC_NORMAL      0xA0
#define CMD_SET_ADC_REVERSE     0xA1

#define CMD_SET_DISP_NORMAL     0xA6
#define CMD_SET_DISP_REVERSE    0xA7

#define CMD_SET_ALLPTS_NORMAL   0xA4
#define CMD_SET_ALLPTS_ON       0xA5
#define CMD_SET_BIAS_9          0xA2
#define CMD_SET_BIAS_7          0xA3

#define CMD_RMW                 0xE0
#define CMD_RMW_CLEAR           0xEE
#define CMD_INTERNAL_RESET      0xE2
#define CMD_SET_COM_NORMAL      0xC0
#define CMD_SET_COM_REVERSE     0xC8
#define CMD_SET_POWER_CONTROL   0x28
#define CMD_SET_RESISTOR_RATIO  0x20
#define CMD_SET_VOLUME_FIRST    0x81
#define CMD_SET_VOLUME_SECOND   0x00
#define CMD_SET_STATIC_OFF      0xAC
#define CMD_SET_STATIC_ON       0xAD
#define CMD_SET_STATIC_REG      0x00
#define CMD_SET_BOOSTER_FIRST   0xF8
#define CMD_SET_BOOSTER_234     0x00
#define CMD_SET_BOOSTER_5       0x01
#define CMD_SET_BOOSTER_6       0x03
#define CMD_NOP                 0xE3
#define CMD_TEST                0xF0

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
//static void st7789_sync(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
static int st7789_command(uint8_t cmd);
static int st7789_data(uint8_t data);

/**********************
 *  STATIC VARIABLES
 **********************/

static uint16_t lcd_fb[ST7789_FB_SIZE];
//static uint8_t pagemap[] = { 7, 6, 5, 4, 3, 2, 1, 0 };

static struct st7789_function st7789_cfg_script[] = {
	{ ST7789_START, ST7789_START},
	{ ST7789_CMD, ST7789_SWRESET},
	{ ST7789_DELAY, 150},
	{ ST7789_CMD, ST7789_SLPOUT},
	{ ST7789_DELAY, 500},
	{ ST7789_CMD, ST7789_INVOFF},
	{ ST7789_CMD, ST7789_MADCTL},
	{ ST7789_DATA, 0xA0},
	{ ST7789_CMD, ST7789_COLMOD},
	{ ST7789_DATA, 0x05},
	{ ST7789_CMD, ST7789_PORCTRL},
	{ ST7789_DATA, 0x0C},
	{ ST7789_DATA, 0x0C},
	{ ST7789_DATA, 0x00},
	{ ST7789_DATA, 0x33},
	{ ST7789_DATA, 0x33},
	{ ST7789_CMD, ST7789_GCTRL},
	{ ST7789_DATA, 0x35},
	{ ST7789_CMD, ST7789_VDVVRHEN},
	{ ST7789_DATA, 0x01},
	{ ST7789_DATA, 0xFF},
	{ ST7789_CMD, ST7789_VRHS},
	{ ST7789_DATA, 0x17},
	{ ST7789_CMD, ST7789_VDVSET},
	{ ST7789_DATA, 0x20},
	{ ST7789_CMD, ST7789_VCOMS},
	{ ST7789_DATA, 0x17},
	{ ST7789_CMD, ST7789_VCMOFSET},
	{ ST7789_DATA, 0x20},
	{ ST7789_CMD, ST7789_PWCTRL1},
	{ ST7789_DATA, 0xA4},
	{ ST7789_DATA, 0xA1},
	{ ST7789_CMD, ST7789_PVGAMCTRL},
	{ ST7789_DATA, 0xD0},
	{ ST7789_DATA, 0x00},
	{ ST7789_DATA, 0x14},
	{ ST7789_DATA, 0x15},
	{ ST7789_DATA, 0x13},
	{ ST7789_DATA, 0x2C},
	{ ST7789_DATA, 0x42},
	{ ST7789_DATA, 0x43},
	{ ST7789_DATA, 0x4E},
	{ ST7789_DATA, 0x09},
	{ ST7789_DATA, 0x16},
	{ ST7789_DATA, 0x14},
	{ ST7789_DATA, 0x18},
	{ ST7789_DATA, 0x21},
	{ ST7789_CMD, ST7789_NVGAMCTRL},
	{ ST7789_DATA, 0xD0},
	{ ST7789_DATA, 0x00},
	{ ST7789_DATA, 0x14},
	{ ST7789_DATA, 0x15},
	{ ST7789_DATA, 0x13},
	{ ST7789_DATA, 0x0B},
	{ ST7789_DATA, 0x43},
	{ ST7789_DATA, 0x55},
	{ ST7789_DATA, 0x53},
	{ ST7789_DATA, 0x0C},
	{ ST7789_DATA, 0x17},
	{ ST7789_DATA, 0x14},
	{ ST7789_DATA, 0x23},
	{ ST7789_DATA, 0x20},
	{ ST7789_CMD, ST7789_CASET},
	{ ST7789_DATA, 0x00},
	{ ST7789_DATA, 0x00},
	{ ST7789_DATA, 0x01},
	{ ST7789_DATA, 0x3F},
	{ ST7789_CMD, ST7789_RASET},
	{ ST7789_DATA, 0x00},
	{ ST7789_DATA, 0x00},
	{ ST7789_DATA, 0x00},
	{ ST7789_DATA, 0xEF},
	{ ST7789_CMD, ST7789_DISPON},
	{ ST7789_DELAY, 100},
	{ ST7789_END, ST7789_END},
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/


/**
 * Write a command to the ST7789
 * @param cmd the command
 */
static int st7789_command(uint8_t cmd)
{
	int ret = 0;

	/* Set command mode */
	if( (ret=gpio_st_write(0)) )
		return ret;

	ret = spi_write(1,&cmd);

	return ret;
}

/**
 * Write data to the ST7789
 * @param data the data
 */
static int st7789_data(uint8_t data)
{
	int ret = 0;

	/* Set command mode */
	if( (ret=gpio_st_write(1)) )
		return ret;

	ret = spi_write(1,&data);

	return ret;
}

static int st7789_databuf(uint32_t len, uint8_t *buf)
{
	uint32_t byte_left=len;
	uint8_t *pt=buf;
	int ret;

	if( (ret=gpio_st_write(1)) )
		return ret;

	while(byte_left)
	{
		if(byte_left>64)
		{
			spi_write(64, pt);
			byte_left=byte_left-64;
			pt=pt+64;
		}
		else
		{
			spi_write(byte_left, pt);
			byte_left=0;
		}
	}

	return 0;
}


void ST7789_ms_sleep( uint16_t ms)
{
	//uint16_t i,j;
	//for(i=0;i<ms;i++)
		usleep(1);
}
// hard reset of the tft controller
// ----------------------------------------------------------
void ST7789_hard_reset( void )
{
	ST7789_ms_sleep(10);
 	i2c_Port(0x15,0xC0);
 	ST7789_ms_sleep(10);
    i2c_Port(0x15,0xC1);
    ST7789_ms_sleep(10);

    i2c_Port(0x14,80);
}

// Configuration of the tft controller
// ----------------------------------------------------------
static void ST7789_run_cfg_script(void)
{
	int i = 0;
	int end_script = 0;

	do {
		switch (st7789_cfg_script[i].cmd)
		{
			case ST7789_START:
				break;
			case ST7789_CMD:
				st7789_command( st7789_cfg_script[i].data & 0xFF );
				break;
			case ST7789_DATA:
				st7789_data( st7789_cfg_script[i].data & 0xFF );
				break;
			case ST7789_DELAY:
				ST7789_ms_sleep(st7789_cfg_script[i].data);
				break;
			case ST7789_END:
				end_script = 1;
		}
		i++;
	} while (!end_script);
}

// 0xe007 = GREEN 0x1f00=blue 0x00F8=red
#define SCREEN_COLOR	0x1f00

void ST7789_blue_scree(void)
{
	int i,j;
	uint16_t	*dst;

	dst = lcd_fb;
	for(i=0;i<ST7789_VER_RES;i++){
		for(j=0;j<ST7789_HOR_RES;j++){
			*dst++=SCREEN_COLOR;
		}
	}

	st7789_command( ST7735_RAMWR );

	st7789_databuf( (ST7789_FB_SIZE*2), (uint8_t *)lcd_fb );

}

/**
 * Initialize the ST7789
 */
int st7789_init(void)
{
	ST7789_hard_reset();

	ST7789_run_cfg_script();

	ST7789_blue_scree();

	return 0;
}

static void st7789_set_addr_win(int xs, int xe, int ys, int ye)
{
	st7789_command(ST7789_CASET);
	st7789_data(((xs>>8)&0xFF));
	st7789_data((xs&0xFF));
	st7789_data(((xe>>8)&0xFF));
	st7789_data((xe&0xFF));
	st7789_command(ST7789_RASET);
	st7789_data(((ys>>8)&0xFF));
	st7789_data((ys&0xFF));
	st7789_data(((ye>>8)&0xFF));
	st7789_data((ye&0xFF));
}


void st7789_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
{
     /*Return if the area is out the screen*/
    if(x2 < 0) return;
    if(y2 < 0) return;
    if(x1 > ST7789_HOR_RES - 1) return;
    if(y1 > ST7789_VER_RES - 1) return;

    /*Truncate the area to the screen*/
    int32_t act_x1 = x1 < 0 ? 0 : x1;
    int32_t act_y1 = y1 < 0 ? 0 : y1;
    int32_t act_x2 = x2 > ST7789_HOR_RES - 1 ? ST7789_HOR_RES - 1 : x2;
    int32_t act_y2 = y2 > ST7789_VER_RES - 1 ? ST7789_VER_RES - 1 : y2;

    st7789_set_addr_win(act_x1,act_x2,act_y1,act_y2);
    /*Set the first row in */

    uint16_t color16;
    uint32_t size = (act_x2 - act_x1 + 1) * (act_y2 - act_y1 + 1);
    uint32_t i;

  	for(i = 0; i < size; i++,color_p++) {
		color16=color_p->full;
		lcd_fb[i]=(color16>>8)&0x00FF;
		lcd_fb[i]=lcd_fb[i] | ((color16<<8)&0xFF00);
		//lcd_fb[i]=color16;
	}

    st7789_command( ST7735_RAMWR );

    st7789_databuf( (size*2), (uint8_t *)lcd_fb );

    lv_flush_ready();
}

#ifdef notdef

void st7789_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color)
{
     /*Return if the area is out the screen*/
    if(x2 < 0) return;
    if(y2 < 0) return;
    if(x1 > ST7789_HOR_RES - 1) return;
    if(y1 > ST7789_VER_RES - 1) return;

    /*Truncate the area to the screen*/
    int32_t act_x1 = x1 < 0 ? 0 : x1;
    int32_t act_y1 = y1 < 0 ? 0 : y1;
    int32_t act_x2 = x2 > ST7789_HOR_RES - 1 ? ST7789_HOR_RES - 1 : x2;
    int32_t act_y2 = y2 > ST7789_VER_RES - 1 ? ST7789_VER_RES - 1 : y2;
    
    int32_t x, y;
    uint8_t white = lv_color_to1(color);

    /*Refresh frame buffer*/
    for(y= act_y1; y <= act_y2; y++) {
        for(x = act_x1; x <= act_x2; x++) {
            if (white != 0) {
                lcd_fb[x+ (y/8)*ST7789_HOR_RES] |= (1 << (7-(y%8)));
            } else {
                lcd_fb[x+ (y/8)*ST7789_HOR_RES] &= ~( 1 << (7-(y%8)));
            }
        }
    }
    
    st7789_sync(act_x1, act_y1, act_x2, act_y2);
}

void st7789_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t * color_p)
{
     /*Return if the area is out the screen*/
    if(x2 < 0) return;
    if(y2 < 0) return;
    if(x1 > ST7789_HOR_RES - 1) return;
    if(y1 > ST7789_VER_RES - 1) return;

    /*Truncate the area to the screen*/
    int32_t act_x1 = x1 < 0 ? 0 : x1;
    int32_t act_y1 = y1 < 0 ? 0 : y1;
    int32_t act_x2 = x2 > ST7789_HOR_RES - 1 ? ST7789_HOR_RES - 1 : x2;
    int32_t act_y2 = y2 > ST7789_VER_RES - 1 ? ST7789_VER_RES - 1 : y2;
    
    int32_t x, y;


    //Set the rectangular area
     ssd1963_cmd(0x002A);
     ssd1963_data(act_x1 >> 8);
     ssd1963_data(0x00FF & act_x1);
     ssd1963_data(act_x2 >> 8);
     ssd1963_data(0x00FF & act_x2);

     ssd1963_cmd(0x002B);
     ssd1963_data(act_y1 >> 8);
     ssd1963_data(0x00FF & act_y1);
     ssd1963_data(act_y2 >> 8);
     ssd1963_data(0x00FF & act_y2);

    /*Set the first row in */
    
    /*Refresh frame buffer*/
    for(y= act_y1; y <= act_y2; y++) {
        for(x = act_x1; x <= act_x2; x++) {
            if (lv_color_to1(*color_p) != 0) {
                lcd_fb[x+ (y/8)*ST7789_HOR_RES] &= ~( 1 << (7-(y%8)));
            } else {
                lcd_fb[x+ (y/8)*ST7789_HOR_RES] |= (1 << (7-(y%8)));
            }
            color_p ++;
        }
        
        color_p += x2 - act_x2; /*Next row*/
    }
    
    st7789_sync(act_x1, act_y1, act_x2, act_y2);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
/**
 * Flush a specific part of the buffer to the display
 * @param x1 left coordinate of the area to flush
 * @param y1 top coordinate of the area to flush
 * @param x2 right coordinate of the area to flush
 * @param y2 bottom coordinate of the area to flush
 */
static void st7789_sync(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
	printf("F.");
	st7789_command( ST7735_RAMWR );

	st7789_databuf( ST7789_FB_SIZE*2, (uint8_t *)lcd_fb );
}
#endif
#endif
