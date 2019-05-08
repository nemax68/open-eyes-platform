/**
 * @file openeyes.h
 *
 */

#ifndef IPC_CLIENT_H
#define IPC_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

//#include "openeyes_gui.h"
#include <mqueue.h>

/*********************
 *      DEFINES
 *********************/

#define QUEUE_GUI_EVENT			"/gui_event"
#define QUEUE_GUI_CMD			"/gui_cmd"

#define POSIX_CMD_BUFFER_SIZE       256
#define POSIX_EVT_BUFFER_SIZE       1024

/**********************
 *      TYPEDEFS
 **********************/

struct posix_queue {
 	size_t size;
  	char *buf;
 	mqd_t mq;
};

struct posix_st {
    struct posix_queue	cmd;
 	struct posix_queue	evt;
};


/**********************
 * GLOBAL PROTOTYPES
 **********************/

void handle_posix_command(void);
int posix_command_init(void);
int send_posix_event(char *);

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* IPC_CLIENT_H */
