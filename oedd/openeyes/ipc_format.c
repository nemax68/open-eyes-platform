/**
 * @file http_client.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <fcntl.h>
#include "ipc_client.h"

#define MAX_ARG_NUM			16

int extract_button_cmd(char **argv, int argc, struct oe_cmd *s)
{
	char *token;

	s->type=OE_BUTTON_TYPE;
	s->oeu.oeb.id=atoi(argv[1]);
	s->oeu.oeb.size_w=atoi(argv[2]);
	s->oeu.oeb.size_h=atoi(argv[3]);
	s->oeu.oeb.pos_x=atoi(argv[4]);
	s->oeu.oeb.pos_y=atoi(argv[5]);
	s->oeu.oeb.font_size=atoi(argv[6]);
	s->oeu.oeb.font_color=strtol(argv[7],NULL,16);
	s->oeu.oeb.border_size=atoi(argv[8]);
	s->oeu.oeb.border_radius=atoi(argv[9]);
	s->oeu.oeb.border_color=strtol(argv[10],NULL,16);
	s->oeu.oeb.main_color=strtol(argv[11],NULL,16);
	s->oeu.oeb.grad_color=strtol(argv[12],NULL,16);
	strcpy(s->oeu.oeb.text,argv[13]);

	// substitution of \n token with SPACE+CR sequence
    token = (char *) strstr(s->oeu.oeb.text, "\\n");
    if(token){
    	*token++ = 0x20;
    	*token = 0x0a;
    }

	return 0;
}

int extract_screen_cmd(char **argv, int argc, struct oe_cmd *s)
{
	printf("\ndetect screen command");

	s->type=OE_SCREEN_TYPE;
	s->oeu.oes.id=atoi(argv[1]);
	s->oeu.oes.font_size=atoi(argv[2]);
	s->oeu.oes.font_color=strtol(argv[3],NULL,16);
	s->oeu.oes.main_color=strtol(argv[4],NULL,16);
	s->oeu.oes.grad_color=strtol(argv[5],NULL,16);

	return 0;
}

int extract_delbutton_cmd(char **argv, int argc, struct oe_cmd *s)
{
	printf("\ndetect delbutton command");

	s->type=OE_DELBUTTON_TYPE;
	s->oeu.oeb.id=atoi(argv[1]);

	return 0;
}

int extract_ipc_cmd(char *cmd, struct oe_cmd *s)
{
	char *token;
	int	argcnt=0;
	int	ret=-2;
	char *argarray[MAX_ARG_NUM];

    token = (char *) strtok(cmd, ",");
    while(token != NULL)
    {
        argarray[argcnt++] = token;

        if (argcnt >= MAX_ARG_NUM)
			return -1;
        else
            token = (char *) strtok(NULL, ",");
    }

    if(strcmp("BUTTON",argarray[0])==0)
    	ret=extract_button_cmd(argarray,argcnt,s);

    if(strcmp("BUTTOFF",argarray[0])==0)
     	ret=extract_delbutton_cmd(argarray,argcnt,s);

    if(strcmp("SCREEN",argarray[0])==0)
        ret=extract_screen_cmd(argarray,argcnt,s);

    return ret;
}
