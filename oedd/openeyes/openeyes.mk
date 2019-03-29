CSRCS += openeyes_gui.c
CSRCS += ipc_client.c
CSRCS += ipc_format.c
CSRCS += http_client.c
CSRCS += bmp.c

DEPPATH += --dep-path openeyes
VPATH += :openeyes

CFLAGS += "-I$(LVGL_DIR)/openeyes"
