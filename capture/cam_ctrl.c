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
#include <fcntl.h>              /* low-level i/o */
#include <errno.h>

#include <sys/ioctl.h>
#include <linux/videodev2.h>

int xioctl(int fh, int request, void *arg);

int zoom_absolute(int fd, int zoom);
int zoom_relative(int fd, int zoom);
int pan_relative(int fd, int pan);
int pan_absolute(int fd, int pan);
int tilt_relative(int fd, int tilt);
int tilt_absolute(int fd, int tilt);

int xioctl(int fh, int request, void *arg)
{
	int r;

	do {
		r = ioctl(fh, request, arg);
	} while (-1 == r && EINTR == errno);

	return r;
}

int get_io_control(int fd, int control, int *value)
{
	struct v4l2_control ctrl;
	int ret;

	ctrl.id=control;

	ret=xioctl(fd, VIDIOC_G_CTRL, &ctrl);

	if( ret>=0 )
		*value=ctrl.value;

	return ret;
}

int set_io_control(int fd, int control, int value)
{
	struct v4l2_control ctrl;
	int ret;

	ctrl.id=control;
	ctrl.value=value;

	ret=xioctl(fd, VIDIOC_S_CTRL, &ctrl);

	return ret;
}

int set_jpeg_quality(int fd, int quality)
{
	struct v4l2_control ctrl;

	ctrl.id=V4L2_CID_JPEG_COMPRESSION_QUALITY;
	ctrl.value=quality;

	return(xioctl(fd, VIDIOC_S_CTRL, &ctrl));
}

int set_frame_rate(int fd, int rate)
{
	struct v4l2_streamparm parm;

	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = rate;
	parm.parm.capture.extendedmode=0;

	int ret = xioctl(fd, VIDIOC_S_PARM, &parm);

	if (ret < 0)
		return ret;

	return 0;
}

int get_frame_rate(int fd)
{
	int ret;

	struct v4l2_streamparm parm;

	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	parm.parm.capture.extendedmode=0;

	ret = xioctl(fd, VIDIOC_G_PARM, &parm);
	if( ret<0 )
		return 0;

	return parm.parm.capture.timeperframe.denominator;
}

int zoom_absolute(int fd, int zoom)
{
	struct v4l2_control ctrl;

	//printf("\n###V4L2_CID_ZOOM_ABSOLUTE###");

	ctrl.id=V4L2_CID_ZOOM_ABSOLUTE;
	ctrl.value=zoom;

	return(xioctl(fd, VIDIOC_S_CTRL, &ctrl));
}

/*
 * zoom positive = in
 * zoom negative = out
 *
 */

int zoom_relative(int fd, int zoom)
{
	struct v4l2_control ctrl;

	//printf("Frame %d zoom in\n",frame_number);

	ctrl.id=V4L2_CID_ZOOM_RELATIVE;
	ctrl.value=zoom;

	return(xioctl(fd, VIDIOC_S_CTRL, &ctrl));
}

/*
 * pan positive = left
 * pan negative = right
 *
 */

int pan_relative(int fd, int pan)
{
	struct v4l2_control ctrl;

	//printf("Frame %d pan left\n",frame_number);

	ctrl.id=V4L2_CID_PAN_RELATIVE;
	ctrl.value=pan;

	return(xioctl(fd, VIDIOC_S_CTRL, &ctrl));
}

int pan_absolute(int fd, int pan)
{
	struct v4l2_control ctrl;

	//printf("Frame %d pan right\n",frame_number);

	ctrl.id=V4L2_CID_PAN_ABSOLUTE;
	ctrl.value=pan;

	return(xioctl(fd, VIDIOC_S_CTRL, &ctrl));
}

int tilt_relative(int fd, int tilt)
{
	struct v4l2_control ctrl;

	//printf("Frame %d tilt up\n",frame_number);

	ctrl.id=V4L2_CID_TILT_RELATIVE;
	ctrl.value=tilt;

	return(xioctl(fd, VIDIOC_S_CTRL, &ctrl));
}

int tilt_absolute(int fd, int tilt)
{
	struct v4l2_control ctrl;

	//printf("Frame %d tilt down\n",frame_number);

	ctrl.id=V4L2_CID_TILT_RELATIVE;
	ctrl.value=tilt;

	return(xioctl(fd, VIDIOC_S_CTRL, &ctrl));
}
