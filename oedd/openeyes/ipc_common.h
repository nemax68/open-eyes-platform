
#ifndef COMMON_H_
#define COMMON_H_

#define QUEUE_GUI_EVENT			"/gui_event"
#define QUEUE_GUI_CMD			"/gui_cmd"
#define MAX_MSG_SIZE 			256

/* EVENT FORMAT:
 * EVT(x)
 */
#define EVENT_GUI_START_APP		0x0001
#define EVENT_GUI_CALL_BUTTON	0x0002
#define EVENT_GUI_ALIVE			0x0003


/* COMMAND FORMAT:
 * CMD(x)
 */
#define CMD_CHECK_ALIVE			0x8001

#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x); \
            exit(-1); \
        } \
    } while (0) \


#endif /* #ifndef COMMON_H_ */
