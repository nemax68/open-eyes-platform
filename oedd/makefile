#
# Makefile
#
CC = arm-linux-gnueabihf-gcc
CFLAGS = -Wall -Wshadow -Wmaybe-uninitialized
CFLAGS += -O3 -g3 -I./
#LDFLAGS += -lSDL2 -lm
BIN = oe_gui
VPATH = 
LIBS = -lrt
OBJDIR = obj

LVGL_DIR = ${shell pwd}

MAINSRC = main.c

#LIBRARIES
include ./lvgl/lv_core/lv_core.mk
include ./lvgl/lv_hal/lv_hal.mk
include ./lvgl/lv_objx/lv_objx.mk
include ./lvgl/lv_misc/lv_fonts/lv_fonts.mk
include ./lvgl/lv_misc/lv_misc.mk
include ./lvgl/lv_themes/lv_themes.mk
include ./lvgl/lv_draw/lv_draw.mk

#DRIVERS
include ./lv_drivers/display/display.mk
include ./lv_drivers/indev/indev.mk
include ./lv_drivers/itf/itf.mk


#EXAMPLES
include ./lv_examples/lv_apps/demo/demo.mk
include ./lv_examples/lv_apps/tpcal/tpcal.mk
include ./lv_examples/lv_tutorial/1_hello_world/lv_tutorial_hello_world.mk
include ./lv_examples/lv_tutorial/2_objects/lv_tutorial_objects.mk
include ./lv_examples/lv_tests/lv_test_obj/lv_test_obj.mk

include ./openeyes/openeyes.mk

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
	$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS) $(LIBS)

clean: 
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ)
