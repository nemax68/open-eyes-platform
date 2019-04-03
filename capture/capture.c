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
#include "args.h"
#include "cam_ctrl.h"

extern int xioctl(int fh, int request, void *arg);


extern void errno_exit(const char *s);

extern int kbhit(void);
extern int getch(void);
extern int terminal_menu(int);
extern void terminal_refresh(int,int);

#ifndef V4L2_PIX_FMT_VYUY
#define V4L2_PIX_FMT_VYUY     v4l2_fourcc('V', 'Y', 'U', 'Y') /* H264 with start codes */
#endif


struct capture		            cap;
struct frame_report	            rep;

int                             fd = -1;
struct buffer                   *buffers;
unsigned int                    n_buffers;


static int read_frame(void)
{
    struct v4l2_buffer buf;
#ifdef TEST_V4L2_CID_COLORFX
    struct v4l2_control ctrl;
#endif
    unsigned int i;
	unsigned char *pt;
	unsigned int len;


    switch (cap.io) {
	case IO_METHOD_READ:
        if (-1 == read(fd, buffers[0].start, buffers[0].length)) {
            switch (errno) {
            case EAGAIN:
                    return 0;

            case EIO:
                    /* Could ignore EIO, see spec. */

                    /* fall through */

            default:
                    errno_exit("read");
            }
        }

        printf("Rx image len=%d\n",buffers[0].length);
        break;

    case IO_METHOD_MMAP:
    	CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
                switch (errno) {
                case EAGAIN:
                        return 0;

                case EIO:
                        /* Could ignore EIO, see spec. */

                        /* fall through */

                default:
                        errno_exit("VIDIOC_DQBUF");
                }
        }

        assert(buf.index < n_buffers);
		if(cap.view_debug)
            printf("\nxioct extract frame %d of %d bytes used %d pt=%p idx=%d",cap.frame_number,buf.length,buf.bytesused,buffers[buf.index].start,buf.index);

		pt=(unsigned char *)buffers[buf.index].start;
		len=buf.bytesused;

		send_image(&cap,pt, len);

        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
            errno_exit("VIDIOC_QBUF");
        break;

    case IO_METHOD_USERPTR:
        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;

        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
            switch (errno) {
            case EAGAIN:
                return 0;
            case EIO:
                    /* Could ignore EIO, see spec. */
                    /* fall through */
            default:
                errno_exit("VIDIOC_DQBUF");
            }
        }

        for (i = 0; i < n_buffers; ++i)
                if (buf.m.userptr == (unsigned long)buffers[i].start
                    && buf.length == buffers[i].length)
                        break;

        assert(i < n_buffers);

        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                errno_exit("VIDIOC_QBUF");
        break;
    }

    return 1;
}

static void mainloop(void)
{
        unsigned int 	count;

        count = cap.total_frame;

	if(!cap.view_debug)
		terminal_refresh(fd,1);

	while (count-- > 0) {
		fd_set fds;
		struct timeval tv;
		int r;

		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		/* Timeout. */
		tv.tv_sec = 10;
		tv.tv_usec = 0;

		r = select(fd + 1, &fds, NULL, NULL, &tv);

		if (-1 == r) {
			if (EINTR == errno)
				continue;
			errno_exit("select");
		}

		if (0 == r) {
			fprintf(stderr, "select timeout\n");
			exit(EXIT_FAILURE);
		}

		read_frame();

		if(!cap.view_debug){
			if(terminal_menu(fd))
				return;
		}else{
			if(cap.frame_number>30)
				return;
		}
        }
}

static void stop_capturing(void)
{
        enum v4l2_buf_type type;

        switch (cap.io) {
        case IO_METHOD_READ:
                /* Nothing to do. */
                break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
                        errno_exit("VIDIOC_STREAMOFF");
                break;
        }
}

static void start_capturing(void)
{
        unsigned int i;
        enum v4l2_buf_type type;

        switch (cap.io) {
		case IO_METHOD_READ:
	       		printf("IO_METHOD_READ:Start capture\n");
		        /* Nothing to do. */
		        break;

		case IO_METHOD_MMAP:
	      		printf("IO_METHOD_MMAP:Start capture nbuf=%d\n",n_buffers);
		        for (i = 0; i < n_buffers; ++i) {
		                struct v4l2_buffer buf;

		                CLEAR(buf);
		                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		                buf.memory = V4L2_MEMORY_MMAP;
		                buf.index = i;

		                if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
		                        errno_exit("VIDIOC_QBUF");
		        }
		        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		        if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
		                errno_exit("VIDIOC_STREAMON");
		        break;

		case IO_METHOD_USERPTR:
		        for (i = 0; i < n_buffers; ++i) {
		                struct v4l2_buffer buf;

		                CLEAR(buf);
		                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		                buf.memory = V4L2_MEMORY_USERPTR;
		                buf.index = i;
		                buf.m.userptr = (unsigned long)buffers[i].start;
		                buf.length = buffers[i].length;

		                if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
		                        errno_exit("VIDIOC_QBUF");
		        }
		        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		        if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
		                errno_exit("VIDIOC_STREAMON");
		        break;
	}
}

static void uninit_device(void)
{
        unsigned int i;

        switch (cap.io) {
        case IO_METHOD_READ:
                free(buffers[0].start);
                break;

        case IO_METHOD_MMAP:
                for (i = 0; i < n_buffers; ++i)
                        if (-1 == munmap(buffers[i].start, buffers[i].length))
                                errno_exit("munmap");
                break;

        case IO_METHOD_USERPTR:
                for (i = 0; i < n_buffers; ++i)
                        free(buffers[i].start);
                break;
        }

        free(buffers);
}

static void init_read(unsigned int buffer_size)
{
        buffers = (buffer *)calloc(1, sizeof(*buffers));

        if (!buffers) {
                fprintf(stderr, "Out of memory\n");
                exit(EXIT_FAILURE);
        }

        buffers[0].length = buffer_size;
        buffers[0].start = malloc(buffer_size);

        if (!buffers[0].start) {
                fprintf(stderr, "Out of memory\n");
                exit(EXIT_FAILURE);
        }
}

static void init_mmap(void)
{
        struct v4l2_requestbuffers req;

        printf("\n\n ##### init_mmap ####\n");

        CLEAR(req);

        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        fprintf(stderr, "%s does not support "
                                 "memory mapping\n", cap.dev_name);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_REQBUFS");
                }
        }

        if (req.count < 2) {
                fprintf(stderr, "Insufficient buffer memory on %s\n",
                         cap.dev_name);
                exit(EXIT_FAILURE);
        }

        buffers = (buffer *)calloc(req.count, sizeof(*buffers));

        printf("\ninit_mmap:buffer count %d pt=%p\n",req.count,buffers);

        if (!buffers) {
                fprintf(stderr, "Out of memory\n");
                exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
                struct v4l2_buffer buf;

                CLEAR(buf);

                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = n_buffers;

                if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
                        errno_exit("VIDIOC_QUERYBUF");

                printf("\ninit_mmap:buf[%d] len=%d offset=%d\n",n_buffers,buf.length,buf.m.offset);

                buffers[n_buffers].length = buf.length;
                buffers[n_buffers].start =
                        mmap(NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              fd, buf.m.offset);

                if (MAP_FAILED == buffers[n_buffers].start)
                        errno_exit("mmap");
        }
}

static void init_userp(unsigned int buffer_size)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count  = 4;
        req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_USERPTR;

        if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        fprintf(stderr, "%s does not support "
                                 "user pointer i/o\n", cap.dev_name);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_REQBUFS");
                }
        }

        buffers = (buffer *)calloc(4, sizeof(*buffers));

        if (!buffers) {
                fprintf(stderr, "Out of memory\n");
                exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < 4; ++n_buffers) {
                buffers[n_buffers].length = buffer_size;
                buffers[n_buffers].start = malloc(buffer_size);

                if (!buffers[n_buffers].start) {
                        fprintf(stderr, "Out of memory\n");
                        exit(EXIT_FAILURE);
                }
        }
}

static void init_device(void)
{
        struct v4l2_capability capab;
        struct v4l2_cropcap cropcap;
        struct v4l2_crop crop;
        struct v4l2_format fmt;
        unsigned int min;

	printf("\n\n ##### init_device ####\n");

	set_frame_rate(fd,cap.ratio);

        if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &capab)) {
                if (EINVAL == errno) {
                        fprintf(stderr, "%s is no V4L2 device\n",
                                 cap.dev_name);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_QUERYCAP");
                }
        }

        if (!(capab.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
                fprintf(stderr, "%s is no video capture device\n",
                         cap.dev_name);
                exit(EXIT_FAILURE);
        }

        switch (cap.io) {
        case IO_METHOD_READ:
                if (!(capab.capabilities & V4L2_CAP_READWRITE)) {
                        fprintf(stderr, "%s does not support read i/o\n",
                                 cap.dev_name);
                        exit(EXIT_FAILURE);
                }
                break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                if (!(capab.capabilities & V4L2_CAP_STREAMING)) {
                        fprintf(stderr, "%s does not support streaming i/o\n",
                                 cap.dev_name);
                        exit(EXIT_FAILURE);
                }
                break;
        }


        /* Select video input, video standard and tune here. */


        CLEAR(cropcap);

        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
                crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                crop.c = cropcap.defrect; /* reset to default */

                if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
                        switch (errno) {
                        case EINVAL:
                                /* Cropping not supported. */
                                break;
                        default:
                                /* Errors ignored. */
                                break;
                        }
                }
        } else {
                /* Errors ignored. */
        }

        CLEAR(fmt);

	fmt.type 		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = cap.img_width; //replace
	fmt.fmt.pix.height      = cap.img_height; //replace
	fmt.fmt.pix.pixelformat = cap.image_type; //replace
	fmt.fmt.pix.field       = V4L2_FIELD_ANY;

	if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
		errno_exit("VIDIOC_S_FMT");

#ifdef notdef
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (force_format==1) {
                fmt.fmt.pix.width       = img_width; //replace
                fmt.fmt.pix.height      = img_height; //replace
                fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG; //replace
                fmt.fmt.pix.field       = V4L2_FIELD_ANY;

                if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
                        errno_exit("VIDIOC_S_FMT");

                /* Note VIDIOC_S_FMT may change width and height. */
	}
        else if (force_format==2) {
                fmt.fmt.pix.width       = img_width; //replace
                fmt.fmt.pix.height      = img_height; //replace
                fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG; //replace
                fmt.fmt.pix.field       = V4L2_FIELD_ANY;

                if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
                        errno_exit("VIDIOC_S_FMT");

                /* Note VIDIOC_S_FMT may change width and height. */



	} else {
                /* Preserve original settings as set by v4l2-ctl for example */
                if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
                        errno_exit("VIDIOC_G_FMT");
        }
#endif
        /* Buggy driver paranoia. */
        min = fmt.fmt.pix.width * 2;
        if (fmt.fmt.pix.bytesperline < min)
                fmt.fmt.pix.bytesperline = min;
        min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
        if (fmt.fmt.pix.sizeimage < min)
                fmt.fmt.pix.sizeimage = min;

        switch (cap.io) {
        case IO_METHOD_READ:
                init_read(fmt.fmt.pix.sizeimage);
                break;

        case IO_METHOD_MMAP:
                init_mmap();
                break;

        case IO_METHOD_USERPTR:
                init_userp(fmt.fmt.pix.sizeimage);
                break;
        }
}

static void close_device(void)
{
        if (-1 == close(fd))
                errno_exit("close");

        fd = -1;
}

static void open_device(void)
{
        struct stat st;

        if (-1 == stat(cap.dev_name, &st)) {
                fprintf(stderr, "Cannot identify '%s': %d, %s\n",
                         cap.dev_name, errno, strerror(errno));
                exit(EXIT_FAILURE);
        }

        if (!S_ISCHR(st.st_mode)) {
                fprintf(stderr, "%s is no device\n", cap.dev_name);
                exit(EXIT_FAILURE);
        }

        fd = open(cap.dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

        if (-1 == fd) {
                fprintf(stderr, "Cannot open '%s': %d, %s\n",
                         cap.dev_name, errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
}




#ifdef notdef
#define SA struct sockaddr
void tcp_open(void)
{
	//struct sockaddr_in servaddr;

	if ((sockfd = socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("\nOpen socket error");;
		exit(1);
	}

	bzero(&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(PORT);

	if(inet_pton(AF_INET,cap.ipaddr,&serv_addr.sin_addr)!=1)
	{
		perror("inet_pton error");
		exit(1);
	}

	if(connect(sockfd,(SA*)&serv_addr,sizeof(serv_addr)))
	{
		perror("\nconnect error");
		exit(1);
	}

}
#endif

int main(int argc, char **argv)
{
	memset(&cap,0,sizeof(cap));

	if( ProcessArgs(argc,argv,&cap) )
		return 0;

	if(udp_open(&cap))
		return 0;

 	printf("\ndevice name: [%s] ip=%s io=%x\n",cap.dev_name,cap.ipaddr,cap.io);

	open_device();
    init_device();
	zoom_absolute(fd,cap.zoom);
    start_capturing();
    mainloop();
    stop_capturing();
    uninit_device();
    close_device();
    fprintf(stderr, "\n");
    return 0;
}
