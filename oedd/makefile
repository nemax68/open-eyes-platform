#
# Makefile
#
CC = arm-linux-gnueabihf-gcc
CFLAGS = -Wall -Wshadow -Wmaybe-uninitialized
CFLAGS += -O3 -g3 -I./
#LDFLAGS += -lSDL2 -lm
BIN = bin/oedd
VPATH =
LIBS = -lrt -ljson-c
OBJDIR = obj
TARGET = 10.1.1.78
DEFINES=

LVGL_DIR = ${shell pwd}

MAINSRC = main.c

#LIBRARIES
include $(LVGL_DIR)/lvgl/lvgl.mk

#DRIVERS
include ./lv_drivers/display/display.mk
include ./lv_drivers/indev/indev.mk
include ./lv_drivers/itf/itf.mk

include ./oegl_app/oegl_app.mk

OBJEXT ?= .o

#AOBJS = $(patsubst %,$(OBJDIR)/%,$(ASRCS:.S=$(OBJEXT)))
#COBJS = $(patsubst %,$(OBJDIR)/%,$(CSRCS:.S=$(OBJEXT)))
AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

PCOBJS = $(patsubst %,$(OBJDIR)/%,$(COBJ))
PMOBJS = $(patsubst %,$(OBJDIR)/%,$(MAINOBJ))
PAOBJS = $(patsubst %,$(OBJDIR)/%,$(AOBJS))

## MAINOBJ -> OBJFILES

all: default

%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"

default: $(AOBJS) $(COBJS) $(MAINOBJ)
	$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(DEFINES) $(LDFLAGS) $(LIBS)

clean:
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ)

target:
	scp $(BIN) root@$(TARGET):/usr/bin
