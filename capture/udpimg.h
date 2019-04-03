#include <stdint.h>

extern int udp_open(struct capture *);
extern int send_image(struct capture *, const void *, int );

#define PORT 			9930

#define MAX_PACKET_SIZE		32768

#define FMT_320x240	0x1
#define FMT_640x480	0x2
#define FMT_800x600	0x3
#define FMT_XGA		0x4
#define FMT_720p	0x5
#define FMT_SXGA	0x6
#define FMT_UXGA	0x7
#define FMT_1080p	0x8
#define FMT_XXX		0xF

#define TYP_YUV422	0x1
#define TYP_JPEG	0x2
#define TYP_RGB		0x3
#define TYP_GREY	0x4

struct vid_udp_head{
	uint8_t		code;
	uint8_t		id;
	uint16_t	frame;
	uint16_t	pkt;
	uint16_t	npkt;
	uint16_t	pkt_size;
	uint8_t		format_type;
	uint8_t		reserved;
	uint32_t 	image_size;
};


