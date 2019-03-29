#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>

#include "openeyes_gui.h"

#if GUI_USE_IPC
#include "ipc_common.h"


int ipc_send_event(char *event)
{
    mqd_t mq;
    char buffer[MAX_MSG_SIZE];
    int	len;

    len=(strlen(event)>MAX_MSG_SIZE)?MAX_MSG_SIZE:strlen(event);

    memset(buffer,0,MAX_MSG_SIZE);
	memcpy(buffer,event,len);

    /* open the mail queue */
    mq = mq_open(QUEUE_GUI_EVENT, O_WRONLY|O_NONBLOCK );
    if(mq==(mqd_t)-1)
		return -1;

	if(	mq_send(mq, buffer, len, 0) )
		return -1;

	printf("\nEVENT %s size %d",event,len);

	mq_close(mq);

	return 0;
}

int ipc_receive_event(char *cmd)
{
    mqd_t mq;
    struct mq_attr attr;
    char buffer[MAX_MSG_SIZE];
    int n=0;

    /* open the mail queue */
    mq = mq_open(QUEUE_GUI_CMD, O_RDONLY|O_NONBLOCK );
    if(mq==(mqd_t)-1)
		return -3;


    if (mq_getattr(mq, &attr) == -1) {
    	mq_close(mq);
        return -2;
    }

    if(attr.mq_curmsgs){
    	if(	(n=mq_receive(mq, buffer, MAX_MSG_SIZE, 0))<0 ){
    		mq_close(mq);
    		return n;
    	}
    	memcpy(cmd,buffer,n);
    }

	mq_close(mq);

	return n;
}



#endif
