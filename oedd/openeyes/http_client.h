/**
 * @file http_gui.h
 *
 */

#ifndef HTTP_GUI_H
#define HTTP_GUI_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

#define HTTP_EVENT_ADDRESS "localhost"
#define HTTP_EVENT_PORT 1880
#define HTTP_TYPE_GET	1
#define HTTP_TYPE_POST	2

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
int http_send_event(char type, char *message);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*HTTP_GUI_H*/
