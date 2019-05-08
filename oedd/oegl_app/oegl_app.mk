CSRCS += oe_gui.c
CSRCS += oe_json.c
CSRCS += oe_mqueue.c
CSRCS += oe_keypad.c
CSRCS += oe_button.c
CSRCS += oe_image.c
CSRCS += bmp.c

DEPPATH += --dep-path oegl_app
VPATH += :oegl_app

CFLAGS += "-I$(LVGL_DIR)/oegl_app"
