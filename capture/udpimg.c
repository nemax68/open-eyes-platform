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
#include <time.h>   // for nanosleep
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

extern struct frame_report	rep;


void sleep_ms(int milliseconds) // cross-platform sleep function
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

int udp_open(struct capture *cap)
{
	struct sockaddr_in	*serv_addr=&cap->serv_addr;
	int 			sockfd;

	//setup UDP client:
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	{
		printf("Open socket error\n");;
		return -1;
	}
	cap->sockfd=sockfd;


        bzero(serv_addr, sizeof(struct sockaddr_in));
	serv_addr->sin_family = AF_INET;
	serv_addr->sin_port = htons(PORT);
	if (inet_aton(cap->ipaddr, &serv_addr->sin_addr)==0)
	{
		printf("inet_aton error");
		return -2;
	}

	return 0;
}


int send_image(struct capture *cap, const void *p, int size)
{
	unsigned char 		*pt = (unsigned char *) p;
	struct	iovec 		iov[2];
	struct 	msghdr 		message;
	struct vid_udp_head 	head;
	struct timeval 		time_s,time_e;
	long 			time_elapsed;
	int			len;
	int			tlen=size;
	int 			ret;
	int 			npkt;
	int			i;

	cap->frame_number++;

	gettimeofday(&time_s,NULL);

	time_elapsed=(time_s.tv_sec-(cap->time_hold.tv_sec))*1000000+time_s.tv_usec-(cap->time_hold.tv_usec);
	cap->time_hold=time_s;
	if(cap->view_debug)
		printf("\n write on socket [%ldus]",time_elapsed);
	rep.time_between_frame=time_elapsed;
	rep.total_size=size;

	npkt=tlen/(cap->max_pkt_size);

	if(tlen%(cap->max_pkt_size))
		npkt++;

	for(i=0;i<npkt;i++){
		len = (tlen>(cap->max_pkt_size))?(cap->max_pkt_size):tlen;
		tlen= tlen -len;

		head.pkt_size=len;
		head.code=0;
		head.id=0;
		head.frame=cap->frame_number;
		head.pkt=i+1;
		head.npkt=npkt;
		head.image_size=size;

		switch(cap->image_type)
		{
			case V4L2_PIX_FMT_YUYV:
			case V4L2_PIX_FMT_YVYU:
			case V4L2_PIX_FMT_VYUY:
			case V4L2_PIX_FMT_UYVY:
				head.format_type=TYP_YUV422;
				break;
			case V4L2_PIX_FMT_RGB24:
			case V4L2_PIX_FMT_BGR24:
				head.format_type=TYP_RGB;
				break;
			case V4L2_PIX_FMT_GREY:
				head.format_type=TYP_GREY;
				break;
			case V4L2_PIX_FMT_JPEG:
				head.format_type=TYP_JPEG;
				break;

		}
		head.format_type=head.format_type | ((cap->image_format<<4)&0xF0);

		iov[0].iov_base=&head;
		iov[0].iov_len=sizeof(head);
		iov[1].iov_base=pt;
		iov[1].iov_len=len;

		pt=pt+len;

		message.msg_name=(struct sockaddr*)&cap->serv_addr;
		message.msg_namelen=sizeof(struct sockaddr);
		message.msg_iov=iov;
		message.msg_iovlen=2;
		message.msg_control=0;
		message.msg_controllen=0;

		ret=sendmsg(cap->sockfd,&message,0);

		sleep_ms(30);
		if(cap->view_debug){
			if(ret>=0) printf("[%d]",len); else { printf("#"); break; }
		}
	}

	gettimeofday(&time_e,NULL);
	time_elapsed=(time_e.tv_sec-time_s.tv_sec)*1000000+time_e.tv_usec-time_s.tv_usec;

	rep.time_to_send_frame=time_elapsed;

	if(cap->view_debug){
		if(ret>=0){
			printf(" %d bytes in %ldus",size,time_elapsed);
		}else{
			perror("write failed bytes");
		}
	}

	fflush(stdout);
	return ret;
}
