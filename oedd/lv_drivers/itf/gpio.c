/**
 * @file i2c.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <gpio.h>

/**********************
 *  STATIC VARIABLES
 **********************/

static char gpio_file_dir[64];
static char gpio_file_val[64];
static int gpio_initialized;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int gpio_init ( char *file_dir, char *dir, char *file_val, int init )
{
	int fd;

	strcpy(gpio_file_dir,file_dir);
	strcpy(gpio_file_val,file_val);
	gpio_initialized=0;

	// *************** set the pins to be an output and turn them on
	fd=open(gpio_file_dir,O_WRONLY);
	if(fd<0)
		return -1;

	write(fd,dir,strlen(dir));
	close(fd);

	gpio_st_write( 1 );

	return 0;
}

int gpio_st_write(int value)
{
	int fd;
	char s_0[] = "0";
	char s_1[] = "1";

	fd=open(gpio_file_val,O_WRONLY);
	if(fd<0)
		return -1;

	if(!value)
		write(fd,s_0,strlen(s_0));
	else
		write(fd,s_1,strlen(s_1));

	close(fd);

	return 0;
}

