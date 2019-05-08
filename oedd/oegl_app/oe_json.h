/*
 * json.h
 *
 *  Created on: 06/mag/2019
 *      Author: maxn
 */

#ifndef OPENEYES_OE_JSON_H_
#define OPENEYES_OE_JSON_H_

struct sdim {
	int x;
	int y;
};

struct scolor {
	char main[16];
	char grad[16];
};

struct sfont {
	char color[16];
	int size;
};

struct sborder {
	char color[16];
	int size;
	int radius;
};

struct json_decoder {
	char name[64];
	char path[64];
	char text[64];
	struct sdim pos;
	struct sdim size;
	struct scolor color;
	struct sfont font;
	struct sborder border;
};

struct json_encoder {
	char name[64];
	char type[64];
	char event[64];
};

struct json_function {
	char name[64];
	int	(*jfunc)(struct json_decoder *jsond);
};

int json_parser(char *);
int json_encoder(struct json_encoder*);


#endif /* OPENEYES_OE_JSON_H_ */
