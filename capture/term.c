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
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#include <linux/videodev2.h>

#include "capture.h"
#include "cam_ctrl.h"

extern struct capture		cap;

extern struct frame_report	rep;

struct termios orig_termios;

static char initialized = 0;

void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

int nkbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return(select(1, &fds, NULL, NULL, &tv));
}

int kbhit(void)
{
    static const int STDIN = 0;

    if (! initialized) {
        // Use termios to turn off line buffering
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = 1;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}


int getch(void)
{
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}

void errno_exit(const char *s)
{
        fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
        exit(EXIT_FAILURE);
}

#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (x), (y))

static int current_selection;
static int action_max;
static int action_min;
static int action_val_old;
static int action_val;
static int action_step;
static int action_query_fail;
static int action_get_fail;
static int action_set_fail;
static int action_exit;
static int esc;

void terminal_refresh(int, int);
void terminal_exit(int, int);
void terminal_query(int, int);
void terminal_set(int, int);
void term_get_fps(int, int);
void term_set_fps(int fd, int id);

struct menu
{
	char 		title[64];
	char 		sel;
	int 		ctrl;
	void		(*func_get)(int,int);
	void		(*func_set)(int,int);
};

static const struct menu menu_voice[] = {
	"vertical flip", 	'v',	V4L2_CID_VFLIP,				terminal_query,terminal_set,
	"horizontal flip",	'h',	V4L2_CID_HFLIP,				terminal_query,terminal_set,
	"brightness",		'b',	V4L2_CID_BRIGHTNESS,			terminal_query,terminal_set,
	"contrast",		'c',	V4L2_CID_CONTRAST,			terminal_query,terminal_set,
	"saturation",		's',	V4L2_CID_SATURATION,			terminal_query,terminal_set,
	"hue",			'u',	V4L2_CID_HUE,				terminal_query,terminal_set,
	"white balance",	'w',	V4L2_CID_DO_WHITE_BALANCE,		terminal_query,terminal_set,
	"AUTO white balance",	'W',	V4L2_CID_AUTO_WHITE_BALANCE,		terminal_query,terminal_set,
	"red gain",		'r',	V4L2_CID_RED_BALANCE,			terminal_query,terminal_set,
	"blue gain",		'R',	V4L2_CID_BLUE_BALANCE,			terminal_query,terminal_set,
	"gain",			'g',	V4L2_CID_GAIN,				terminal_query,terminal_set,
	"AUTO gain",		'G',	V4L2_CID_AUTOGAIN,			terminal_query,terminal_set,
	"exposure",		'e',	V4L2_CID_EXPOSURE,			terminal_query,terminal_set,
	"AUTO exposure",	'E',	V4L2_CID_EXPOSURE_AUTO,			terminal_query,terminal_set,
	"color",		'o',	V4L2_CID_COLORFX,			terminal_query,terminal_set,
	"zoom",			'z',	V4L2_CID_ZOOM_ABSOLUTE,			terminal_query,terminal_set,
	"pan",			'p',	V4L2_CID_PAN_ABSOLUTE,			terminal_query,terminal_set,
	"tilt",			't',	V4L2_CID_TILT_ABSOLUTE,			terminal_query,terminal_set,
	"jpeg",			'j',	V4L2_CID_JPEG_COMPRESSION_QUALITY,	terminal_query,terminal_set,
	"FPS",			'f',	0,					term_get_fps,term_set_fps,
	"quit",			'q',	0,					terminal_exit,NULL,
	"refresh",		'y',	0,					terminal_refresh,NULL,
	"",			0,	0,					NULL,NULL
};

void term_get_fps(int fd, int id)
{
	cap.ratio=get_frame_rate(fd);
	action_val=cap.ratio;
	action_min=0;
	action_max=30;
	action_step=1;
	action_get_fail=0;
	action_query_fail=0;

}

void term_set_fps(int fd, int id)
{
	if( set_frame_rate(fd, action_val)< 0 )
		action_set_fail=1;
	else{
		action_set_fail=0;
		cap.ratio=action_val;
	}
}

void terminal_set(int fd, int id)
{
	if(menu_voice[id].ctrl){
		if (set_io_control (fd, menu_voice[id].ctrl, action_val)<0)
			action_set_fail=1;
		else
			action_set_fail=0;
	}
	else
	{

	}
}

void terminal_query(int fd, int ctrl)
{
	struct v4l2_queryctrl 	queryctrl;

	if(ctrl){
		queryctrl.id=ctrl;
		if (0 == ioctl (fd, VIDIOC_QUERYCTRL, &queryctrl)) {
			action_min=queryctrl.minimum;
			action_max=queryctrl.maximum;
			action_step=queryctrl.step;
			action_get_fail=0;
			action_query_fail=0;
			if (get_io_control (fd, queryctrl.id, &action_val)<0){
				action_val = queryctrl.default_value;
				action_get_fail=1;
			}
		}
		else
			action_query_fail=1;
	}
}

void terminal_exit(int fd,int init)
{
	action_exit=1;
}

void terminal_refresh(int fd, int init)
{
	int 	i=0;
	int 	row,col;

	if(init){
		current_selection=0;
		action_val=0;
		action_val_old=0;
		esc=0;
		action_exit=0;
	}

	action_set_fail=0;
	action_get_fail=0;
	action_query_fail=0;

	clear();

	gotoxy(10,10);
	printf("*************** MENU ******************\n");
	while(menu_voice[i].sel)
	{
		row=i/2;
		col=(i%2)*25;
		gotoxy(11+row,col);
		printf("%c=%s\n",menu_voice[i].sel,menu_voice[i].title);
		i++;
	}
	gotoxy(0,0);
}


int terminal_menu(int fd)
{
	char			c;
	int 			i=0;

	while(kbhit()) {
		c=getch(); /* consume the character */
		/* MAIN MENU CONTROLS */
		//if(c=='q') return 1;
		//if(c=='r') terminal_refresh(0);
		//if(c=='f') {action_val=cap.ratio;action_val_old=action_val;}

		if(c>='0' && c<='9') action_val=c-'0';
		if(c==0x1b && esc==0)	esc=1;
		if(c==0x5b && esc==1)	esc=2;
		if(c==0x41 && esc==2){
			esc=0;
			action_val=action_val+action_step;
		}
		if(c==0x42 && esc==2){
			esc=0;
			action_val=action_val-action_step;
		}

		if(action_val>action_max) action_val=action_max;
		if(action_val<action_min) action_val=action_min;

    		if(action_val!=action_val_old)
		{
			if(menu_voice[current_selection].func_set)
				menu_voice[current_selection].func_set(fd, current_selection);
/*			if(menu_voice[current_selection].ctrl){
				if (set_io_control (fd, menu_voice[current_selection].ctrl, action_val)<0)
					action_set_fail=1;
				else
					action_set_fail=0;
			}
			else
			{

			}*/
			action_val_old=action_val;
		}

		/* TABLE MENU CONTROLS */
		while(menu_voice[i].sel){
			if(menu_voice[i].sel==c){
				current_selection=i;
				if(menu_voice[i].func_get){
					menu_voice[i].func_get(fd, menu_voice[i].ctrl);
				}
			}
			i++;
		}

		/* UPDATES MENU CONTROLS */
		gotoxy(9,0);
		printf("Current selection: %s [%d<%d<%d] c=%x",
		       menu_voice[current_selection].title,action_min,action_val,action_max,c);
		if(action_get_fail) printf(" GET FAIL ");
		if(action_set_fail) printf(" SET FAIL ");
		if(action_query_fail) printf(" QUERY FAIL ");

	}
	gotoxy(0,0);

	printf("FPS=%d	TIME=%ldms  FRAME SIZE=%d TIME2SEND=%ldms   ",cap.ratio,rep.time_between_frame/1000,rep.total_size,rep.time_to_send_frame/1000);
	return action_exit;
}
