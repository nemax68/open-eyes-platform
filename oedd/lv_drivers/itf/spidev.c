/*
    spidevlib.c - A user-space program to comunicate using spidev.
                Gustavo Zamboni
*/
#include <stdint.h>
#include <unistd.h>
/**
 * @file spidev.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <include/spi/spidev.h>
#include <sys/ioctl.h>

/**********************
 *  STATIC VARIABLES
 **********************/

static char spi_file[64];
static int spi_initialized;
static int spi_id;

struct spi_ioc_transfer xfer[2];


//////////
// Init SPIdev
//////////
int spi_open(char *dev)
{
	int fd;
    uint8_t    mode, lsb, bits;
    uint32_t speed=2500000;

	strcpy(spi_file,dev);
	spi_initialized=0;

	if ((fd = open(spi_file,O_RDWR)) < 0)
	{
		//perror("Failed to open the bus\n");
		/* ERROR HANDLING; you can check errno to see what went wrong */
		return -1;
	}

	if (ioctl(fd, SPI_IOC_RD_MODE, &mode) < 0)
	{
		//perror("SPI rd_mode");
		return -1;
	}
	if (ioctl(fd, SPI_IOC_RD_LSB_FIRST, &lsb) < 0)
	{
		//perror("SPI rd_lsb_fist");
		return -1;
	}
	if (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0)
	{
		//perror("SPI bits_per_word");
		return -1;
	}
	if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0)
	{
		//perror("SPI max_speed_hz");
		return -1;
	}

	spi_id=fd;

    //printf("%s: spi mode %d, %d bits %sper word, %d Hz max file desc=%d\n",spi_file, mode, bits, lsb ? "(lsb first) " : "", speed,spi_id);

    xfer[0].len = 3; /* Length of  command to write*/
    xfer[0].cs_change = 0; /* Keep CS activated */
    xfer[0].delay_usecs = 0, //delay in us
    xfer[0].speed_hz = 25000000, //speed
    xfer[0].bits_per_word = 8, // bites per word 8

    //xfer[1].rx_buf = (unsigned long) buf2;
    xfer[1].len = 4; /* Length of Data to read */
    xfer[1].cs_change = 0; /* Keep CS activated */
    xfer[0].delay_usecs = 0;
    xfer[0].speed_hz = 25000000;
    xfer[0].bits_per_word = 8;

    spi_initialized=1;

    return 0;
}

void spi_close(void)
{
	close(spi_id);
	spi_initialized=0;
}
//////////
// Read n bytes from the 2 bytes add1 add2 address
//////////

char spi_read(int nbytes, char cmd, char *rxbuf)
{
    unsigned char   buf[64];
    int status;

    if(!spi_initialized)
    	return -1;

    if(nbytes>64)
     {
 		//perror("SPI_NBYTES_ERROR");
 		return -1;
 	}

    memset(buf, 0, sizeof buf);
    buf[0] = cmd;
    xfer[0].tx_buf = (unsigned long)buf;
    xfer[0].len = 1; /* Length of  command to write*/
    xfer[1].rx_buf = (unsigned long) rxbuf;
    xfer[1].len = nbytes; /* Length of Data to read */
    status = ioctl(spi_id, SPI_IOC_MESSAGE(2), xfer);
    if (status < 0)
	{
    	//perror("SPI_IOC_MESSAGE");
    	return -1;
	}
    return 0;
}

//////////
// Write n bytes int the 2 bytes address add1 add2
//////////
int spi_write(uint16_t nbytes, uint8_t *pt)
{
    unsigned char   buf[64];
    int status;

    if(!spi_initialized)
    	return -1;

    if(nbytes>64)
    {
		//perror("SPI_NBYTES_ERROR");
		return -1;
	}

    memset(buf, 0, 64);
    memcpy(buf,pt,nbytes);

    xfer[0].tx_buf = (unsigned long)buf;
    xfer[0].len = nbytes; /* Length of  command to write*/
    status = ioctl(spi_id, SPI_IOC_MESSAGE(1), xfer);
    if (status < 0)
	{
		//printf("WRITE SPI_IOCTL_MESSAGE file desc=%d",spi_id);
		return -1;
	}
    return 0;
}
