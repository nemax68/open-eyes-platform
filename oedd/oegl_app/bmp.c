/*##############################################################*/
/* 																*/
/* File		: bmp.c												*/
/*																*/
/* Project	: TFT for Raspberry Pi Revision 2					*/
/* 																*/
/* Date		: 2014-08-13   	    last update: 2014-08-13			*/
/* 																*/
/* Author	: Hagen Ploog   									*/
/*		  	  Kai Gillmann										*/
/*		  	  Timo Pfander										*/
/* 																*/
/* IDE	 	: Geany 1.22										*/
/* Compiler : gcc (Debian 4.6.3-14+rpi1) 4.6.3					*/
/*																*/
/* Copyright (C) 2013 admatec GmbH								*/
/*																*/
/*																*/
/* Description  :												*/
/* 																*/
/*	The current software can display BMP files on the C-Berry.	*/
/*	The BMP file must have a dimension of 320 x 240 pixel and	*/
/*	a color depth of 24Bit.										*/
/*	The picture(s) will be stored into a memory. The function	*/
/*	returns back a pointer with the address of the memory.		*/
/*																*/
/*																*/
/* License:														*/
/*																*/
/*	This program is free software; you can redistribute it 		*/
/*	and/or modify it under the terms of the GNU General			*/
/*	Public License as published by the Free Software 			*/
/*	Foundation; either version 3 of the License, or 			*/
/*	(at your option) any later version. 						*/
/*    															*/
/*	This program is distributed in the hope that it will 		*/
/*	be useful, but WITHOUT ANY WARRANTY; without even the 		*/
/*	implied warranty of MERCHANTABILITY or 						*/
/*	FITNESS FOR A PARTICULAR PURPOSE. See the GNU General 		*/
/*	Public License for more details. 							*/
/*																*/
/*	You should have received a copy of the GNU General 			*/
/*	Public License along with this program; if not, 			*/
/*	see <http://www.gnu.org/licenses/>.							*/
/*																*/
/*																*/
/* Revision History:											*/
/*																*/
/*	Version 1.0 - Initial release								*/
/*																*/
/*																*/
/*																*/
/*##############################################################*/

//
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "oe_gui.h"
#include "bmp.h"

// store BMP files in memory
// ----------------------------------------------------------
int Read_bmp2memory ( char const *file_name, struct bmp_file *desc  )
{
	FILE *my_file;

	uint8_t bmp_header_buffer[54];
	uint8_t bmp_line_buffer[ LV_HOR_RES * 3 ];

	uint16_t bfType;
	uint32_t bfOffBits;

	uint32_t biSize;

	int32_t biWidth;
	int32_t biHeight;
	uint16_t biBitCount;

	uint32_t y,x;

	size_t img_size;
	uint8_t red, green, blue;

	lv_color_t	*ppt;

	// check for file
	if ( ( my_file = fopen( file_name, "rb" ) ) == NULL )
		return( ENOENT );

	// read header
	fread( &bmp_header_buffer, 1, 54, my_file );

	// check for "BM"
	bfType = bmp_header_buffer[1];
	bfType = (bfType << 8) | bmp_header_buffer[0];
	desc->type=bfType;

	if ( bfType != 0x4D42) {
		fclose( my_file );
		return( EINVAL );
	}

	biSize = bmp_header_buffer[17];
	biSize = (biSize << 8) | bmp_header_buffer[16];
	biSize = (biSize << 8) | bmp_header_buffer[15];
	biSize = (biSize << 8) | bmp_header_buffer[14];
	desc->size=biSize;

	biWidth = bmp_header_buffer[21];
	biWidth = (biWidth << 8) | bmp_header_buffer[20];
	biWidth = (biWidth << 8) | bmp_header_buffer[19];
	biWidth = (biWidth << 8) | bmp_header_buffer[18];
	desc->width=biWidth;

	biHeight = bmp_header_buffer[25];
	biHeight = (biHeight << 8) | bmp_header_buffer[24];
	biHeight = (biHeight << 8) | bmp_header_buffer[23];
	biHeight = (biHeight << 8) | bmp_header_buffer[22];
	desc->height=biHeight;

	biBitCount = bmp_header_buffer[29];
	biBitCount = (biBitCount << 8) | bmp_header_buffer[28];
	desc->bitcolor=biBitCount;

	if ( (biWidth > LV_HOR_RES) || (biHeight > LV_VER_RES) || (biBitCount != 24) ) {
		fclose( my_file );
		return( EINVAL );
	}

	bfOffBits = bmp_header_buffer[13];
	bfOffBits = (bfOffBits << 8) | bmp_header_buffer[12];
	bfOffBits = (bfOffBits << 8) | bmp_header_buffer[11];
	bfOffBits = (bfOffBits << 8) | bmp_header_buffer[10];

	img_size=biWidth*biHeight*(biBitCount/8);
	desc->img=malloc(img_size);
	if (!desc->img) {
		fclose( my_file );
		return( ENOMEM );
	}

	// point to the end of buffer
	img_size=biWidth*biHeight;
	ppt=(lv_color_t *)desc->img+img_size;

	fseek( my_file, bfOffBits, SEEK_SET );

	for (y=biHeight; y>0; y--)
	{
		fread( &bmp_line_buffer[0], biWidth*3, 1, my_file );
		for (x=biWidth; x>0; x--)
		{
			blue =  bmp_line_buffer[(x-1)*3 +0];
			green = bmp_line_buffer[(x-1)*3 +1];
			red =   bmp_line_buffer[(x-1)*3 +2];

			*ppt-- = LV_COLOR_MAKE(red,green,blue);
		}
	}

	fclose( my_file );

	return ( 0 );
}
