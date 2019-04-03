/**
 * Copyright 2018 OPEN-EYES S.r.l.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#include <jpeglib.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <termios.h>

#include <sys/select.h>
#include <stropts.h>

#include "capture.h"
#include "udpimg.h"

static void usage(FILE *fp, int argc, char **argv,struct capture *cap)
{
    fprintf(fp,
        "Usage: %s [options]\n\n"
        "Version 1.0\n"
        "Options:\n"
        "-d | --device name   Video device name [%s]\n"
		"-i | --ip            Remote IP address [%s]\n"
		"-z | --zoom          Zoom [%d]\n"
		"-s | --size          packet size [%d]\n"
		"-t | --ratio         FPS [%d]\n"
        "-h | --help          Print this message\n"
        "-m | --mmap          Use memory mapped buffers [default]\n"
        "-r | --red           Red color gain\n"
        "-u | --userp         Use application allocated buffers\n"
        "-o | --output        Outputs stream to stdout\n"
        "-f | --format        Format QVGA,VGA,SVGA,LXGA,720p,SXGA,UXGA,1080p\n"
        "-v | --video0        JPEG/YUYV/YVYU.....\n"
        "-c | --count         Number of frames to grab [%i]\n"
		"-b | --bugs          Bugs\n"
        "",
        argv[0], cap->dev_name, cap->ipaddr, cap->zoom, cap->max_pkt_size, cap->ratio, cap->total_frame
    );
}

static const char short_options[] = "d:i:z:s:t:hmruof:v:c:xb";

static const struct option long_options[] = {
    { "device", required_argument, NULL, 'd' },
    { "ip",     required_argument, NULL, 'i' },
    { "zoom",   required_argument, NULL, 'z' },
    { "size",   required_argument, NULL, 's' },
	{ "ratio",  required_argument, NULL, 't' },
    { "help",   no_argument,       NULL, 'h' },
    { "mmap",   no_argument,       NULL, 'm' },
    { "read",   no_argument,       NULL, 'r' },
    { "userp",  no_argument,       NULL, 'u' },
    { "output", no_argument,       NULL, 'o' },
    { "format", required_argument, NULL, 'f' },
    { "video",  required_argument, NULL, 'v' },
    { "count",  required_argument, NULL, 'c' },
	{ "bugs",   no_argument,       NULL, 'b' },
    { 0, 0, 0, 0 }
};

int ProcessArgs(int argc, char** argv,struct capture *cap)
{
	int idx;
	int c;

	strcpy(cap->dev_name,"/dev/video0");
	cap->zoom = 100;
	cap->ratio=2;
	cap->max_pkt_size=MAX_PACKET_SIZE;
	//cap->img_jpeg=0;
	cap->image_format=FMT_320x240;
	cap->image_type=V4L2_PIX_FMT_JPEG;
	cap->img_height=240;
	cap->img_width=320;
	cap->io = IO_METHOD_MMAP;
	cap->total_frame=50000;
	cap->view_debug=0;

	for (;;) {

        c = getopt_long(argc, argv, short_options, long_options, &idx);

        if (-1 == c)
            break;

        switch (c) {
        case 0: /* getopt_long() flag */
            break;
        case 'd':
            strcpy(cap->dev_name,optarg);
            break;
	 	case 'i':
            strcpy(cap->ipaddr,optarg);
            break;
        case 'z':
            cap->zoom = strtol(optarg, NULL, 0);
		    break;
		case 't':
			cap->ratio = strtol(optarg, NULL, 0);
            break;
        case 'b':
            cap->view_debug = 1;
		    break;
        case 's':
            cap->max_pkt_size = strtol(optarg, NULL, 0);
            break;
        case 'h':
            usage(stdout, argc, argv, cap);
            exit(EXIT_SUCCESS);
        case 'm':
            cap->io = IO_METHOD_MMAP;
		    break;
        case 'r':
            cap->io = IO_METHOD_READ;
            break;
        case 'u':
            cap->io = IO_METHOD_USERPTR;
            break;
        case 'o':
            cap->outb++;
            break;
        case 'v':
            if(strstr(optarg,"jpeg") != NULL) {
            	//cap->img_jpeg=1;
            	cap->image_type=V4L2_PIX_FMT_JPEG;
            }
            else if(strstr(optarg,"yuyv") != NULL) {
            	//cap->img_jpeg=1;
            	cap->image_type=V4L2_PIX_FMT_YUYV;
            }
            else if(strstr(optarg,"yvyu") != NULL) {
            	//cap->img_jpeg=1;
            	cap->image_type=V4L2_PIX_FMT_YVYU;
            }
            else if(strstr(optarg,"vyuy") != NULL) {
            	//cap->img_jpeg=1;
            	cap->image_type=V4L2_PIX_FMT_VYUY;
            }
            else if(strstr(optarg,"uyvy") != NULL) {
            	//cap->img_jpeg=1;
            	cap->image_type=V4L2_PIX_FMT_UYVY;
            }
            else if(strstr(optarg,"rgb") != NULL) {
            	//cap->img_jpeg=0;
            	cap->image_type=V4L2_PIX_FMT_RGB24;
            }
            else if(strstr(optarg,"bgr") != NULL) {
            	//cap->img_jpeg=0;
            	cap->image_type=V4L2_PIX_FMT_BGR24;
            }
            else if(strstr(optarg,"grey") != NULL) {
            	//cap->img_jpeg=0;
            	cap->image_type=V4L2_PIX_FMT_GREY;
            }
            else
                return -1;
        case 'f':
            //force_format = strtol(optarg, NULL, 0);
            if(strstr(optarg,"QVGA") != NULL) {
            	cap->img_height=240;
            	cap->img_width=320;
            	cap->image_format=FMT_320x240;
            }
            else if(strstr(optarg,"SVGA") != NULL) {
            	cap->img_height=600;
            	cap->img_width=800;
            	cap->image_format=FMT_800x600;
            }
            else if(strstr(optarg,"VGA") != NULL) {
            	cap->img_height=480;
            	cap->img_width=640;
            	cap->image_format=FMT_640x480;
            }
            else if(strstr(optarg,"LXGA") != NULL) {
            	printf("\n1024x768\n");
            	cap->img_height=768;
            	cap->img_width=1024;
            	cap->image_format=FMT_XGA;
            }
            else if(strstr(optarg,"720p") != NULL) {
            	printf("\n1280x720\n");
            	cap->img_height=720;
            	cap->img_width=1280;
            	cap->image_format=FMT_720p;
            }
            else if(strstr(optarg,"SXGA") != NULL) {
            	printf("\n1280x960\n");
            	cap->img_height=960;
            	cap->img_width=1280;
            	cap->image_format=FMT_SXGA;
            }
            else if(strstr(optarg,"UXGA") != NULL) {
            	printf("\n1600x1200\n");
            	cap->img_height=1200;
            	cap->img_width=1600;
            	cap->image_format=FMT_UXGA;
            }
            else if(strstr(optarg,"1080p") != NULL) {
            	printf("\n1920x1080\n");
            	cap->img_height=1080;
            	cap->img_width=1920;
            	cap->image_format=FMT_1080p;
            }
            break;
        case 'c':
            errno = 0;
            cap->total_frame = strtol(optarg, NULL, 0);
            if (errno)
                    errno_exit(optarg);
            break;

        default:
            usage(stderr, argc, argv, cap);
            exit(EXIT_FAILURE);
        }
    }

	if(strlen(cap->ipaddr)==0)
	{
		printf("\nRemote IP address required!\n");
        usage(stderr, argc, argv, cap);
		return -1;
	}

	return 0;
}
