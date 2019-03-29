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

#include "openeyes_gui.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/


/**********************
 * GLOBAL PROTOTYPES
 **********************/

int ipc_send_event(char *);
int ipc_receive_event(char *);
int extract_ipc_cmd(char *cmd, struct oe_cmd *s);


/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* IPC_CLIENT_H */
