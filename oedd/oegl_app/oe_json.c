/*
 * json.c
 *
 *  Created on: 04/mag/2019
 *      Author: maxn
 */

#include<stdio.h>
#include<string.h>

#include<json-c/json.h>

#include"oe_json.h"

enum typefunc {
	IMAGE_FUNCTION,
};

int image_add(struct json_decoder *);
int image_del(struct json_decoder *);
int button_add(struct json_decoder *);
int button_del(struct json_decoder *);
int keypad_add(struct json_decoder *);
int keypad_del(struct json_decoder *);
int clear_screen(struct json_decoder *);


static const struct json_function jf[] = {
		{"addimage", 	image_add},
		{"delimage", 	image_del},
		{"clrscreen", 	clear_screen},
		{"addbutton", 	button_add},
		{"delbutton", 	button_del},
		{"addkeypad", 	keypad_add},
		{"delkeypad", 	keypad_del},
		{"",			NULL}
};


int json_parser(char *buffer)
{
	struct json_object *parsed_json;
	struct json_object *type;
	struct json_object *name;
	struct json_object *path;
	struct json_object *text;
	struct json_object *position;
	struct json_object *size;
	struct json_object *font;
	struct json_object *border;
	struct json_object *param;
	struct json_object *color;

	char				*str;
	struct json_decoder jd;
	const struct json_function *pjf=jf;

	memset(&jd,0,sizeof(jd));

	printf("JSON: %s\n",buffer);

	parsed_json = json_tokener_parse(buffer);
	if (parsed_json) {

		// extract name
		if( json_object_object_get_ex(parsed_json, "name", &name) ) {
			str=(char *)json_object_get_string(name);
			strcpy(jd.name,str);
			json_object_put(name);
		}

		// extract position
		if( json_object_object_get_ex(parsed_json, "position", &position) ) {
			if( json_object_object_get_ex(position, "x", &param) ) {
				jd.pos.x=json_object_get_int(param);
				json_object_put(param);
			}
			if( json_object_object_get_ex(position, "y", &param) ) {
				jd.pos.y=json_object_get_int(param);
				json_object_put(param);
			}
			json_object_put(position);
		}

		// extract size
		if( json_object_object_get_ex(parsed_json, "size", &size) ) {
			if( json_object_object_get_ex(size, "x", &param) ) {
				jd.size.x=json_object_get_int(param);
				json_object_put(param);
			}
			if( json_object_object_get_ex(size, "y", &param) ) {
				jd.size.y=json_object_get_int(param);
				json_object_put(param);
			}
			json_object_put(size);
		}

		// extract color
		if( json_object_object_get_ex(parsed_json, "color", &color) ) {
			if( json_object_object_get_ex(color, "main", &param) ) {
				str=(char *)json_object_get_string(param);
				strcpy(jd.color.main,str);
				json_object_put(param);
			}
			if( json_object_object_get_ex(color, "gradient", &param) ) {
				str=(char *)json_object_get_string(param);
				strcpy(jd.color.grad,str);
				json_object_put(param);
			}
			json_object_put(color);
		}

		// extract font
		if( json_object_object_get_ex(parsed_json, "font", &font) ) {
			if( json_object_object_get_ex(font, "color", &param) ) {
				str=(char *)json_object_get_string(param);
				strcpy(jd.font.color,str);
				json_object_put(param);
			}
			if( json_object_object_get_ex(font, "size", &param) ) {
				jd.font.size=json_object_get_int(param);
				json_object_put(param);
			}
			json_object_put(font);
		}

		// extract border
		if( json_object_object_get_ex(parsed_json, "border", &border) ) {
			if( json_object_object_get_ex(border, "color", &param) ) {
				str=(char *)json_object_get_string(param);
				strcpy(jd.border.color,str);
				json_object_put(param);
			}
			if( json_object_object_get_ex(border, "size", &param) ) {
				jd.border.size=json_object_get_int(param);
				json_object_put(param);
			}
			if( json_object_object_get_ex(border, "round", &param) ) {
				jd.border.radius=json_object_get_int(param);
				json_object_put(param);
			}
			json_object_put(border);
		}

		// extract path
		if( json_object_object_get_ex(parsed_json, "path", &path) ) {
			str=(char *)json_object_get_string(path);
			strcpy(jd.path,str);
			json_object_put(path);
		}

		// extract text
		if( json_object_object_get_ex(parsed_json, "text", &text) ) {
			str=(char *)json_object_get_string(text);
			strcpy(jd.text,str);
			json_object_put(text);
		}

		if (json_object_object_get_ex(parsed_json, "type", &type)) {
			str=(char *)json_object_get_string(type);

			while (strlen(pjf->name)) {
				if(strcmp(str,pjf->name)==0) {
					(pjf->jfunc)(&jd);
				}
				pjf++;

			}
		}


		json_object_put(type);
		json_object_put(parsed_json);
	}

	return 0;
}

int json_encoder(struct json_encoder *json_enc)
{
	json_object * jobj = json_object_new_object();
	json_object *jevent = json_object_new_string(json_enc->event);
	json_object *jtype = json_object_new_string(json_enc->type);
	json_object *jname = json_object_new_string(json_enc->name);

	json_object_object_add(jobj,"name", jname);
	json_object_object_add(jobj,"event", jevent);
	json_object_object_add(jobj,"type", jtype);

	printf ("JSON TX %sn",json_object_to_json_string(jobj));
	return 0;
}
