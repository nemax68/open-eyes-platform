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

/**********************
 *  STATIC VARIABLES
 **********************/

static char i2c_file[64];
static int i2c_addr;
static int i2c_initialized;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int i2c_init(char *dev, int addr)
{
	int fd;

	strcpy(i2c_file,dev);
	i2c_addr=addr;
	i2c_initialized=0;

	if ((fd = open(dev,O_RDWR)) < 0)
		return -1;

	if (ioctl(fd,I2C_SLAVE,addr) < 0)
		return -1;

	close(fd);

	i2c_initialized=1;

	return 0;
}

//////////
// Write n bytes
//////////
int i2c_write(int add,char value,int file)
{
    unsigned char buf[2];
    buf[0] = add;
    buf[1] = value;

    if (write(file, buf, 2) != 2)
		return -1;

    return 0;
}

int i2c_Port(int add, char value)
{
	int fd;

	if ((fd = open(i2c_file,O_RDWR)) < 0)
		return -1;

	if (ioctl(fd,I2C_SLAVE,i2c_addr) < 0)
		return -1;

	i2c_write(add,value,fd); //this will write value 0xC0 to the address 0x15 i.e. RESET

	close(fd);

	return 0;
}

