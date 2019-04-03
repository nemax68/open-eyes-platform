#include <arpa/inet.h>
#include <sys/types.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))


extern void errno_exit(const char *s);

enum io_method {
        IO_METHOD_READ,
        IO_METHOD_MMAP,
        IO_METHOD_USERPTR,
};

struct buffer {
        void   *start;
        size_t  length;
};

struct capture {
	int                    sockfd;
	char                   dev_name[64];
	char                   ipaddr[16];
	struct sockaddr_in     serv_addr;
	int                    frame_number;
	int                    image_format;
	u_int32_t              image_type;
	int                    total_frame;
	int			           max_pkt_size;
	int           		   zoom;
	int                    img_height;
	int                    img_width;
	int                    ratio;
	int                    jpeg_quality;
	int                    outb;
	int                    view_debug;
	enum io_method         io;
	struct timeval         time_hold;
};

struct frame_report{
	long 			       time_between_frame;
	long 			       time_to_send_frame;
	int 			        total_size;
};

/*
struct face_detect{
	int 			frame_width;
	int 			frame_height;
	int 			face_num;
	int 			detect_size;
	int 			face_x;
	int 			face_y;
	int 			face_xsize;
	int 			face_ysize;
	long			time_used;
	int  			x_target;
	int 			y_target;
	int 			zoom_target;
	int                     pan;
	int                     tilt;
	int                     zoom;
	int 			zoom_inc;
	int 			pan_inc;
	int 			tilt_inc;
	int 			locked;
};
*/
