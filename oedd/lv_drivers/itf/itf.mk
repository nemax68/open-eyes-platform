CSRCS += i2c.c
CSRCS += spidev.c
CSRCS += gpio.c

DEPPATH += --dep-path lv_drivers/itf
VPATH += :lv_drivers/itf

CFLAGS += "-I$(LVGL_DIR)/lv_drivers/itf"
