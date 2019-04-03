CC=arm-linux-gnueabihf-g++

TARGET = 10.1.1.78
BIN = bin/capture
CFLAGS =-Iinc
CFLAGS +=-g
CFLAGS += -Wall
#CFLAGS += -fpermissive
#CFLAGS += -pthread

SRCS = capture.c
SRCS += jpeg.c
SRCS += cam_ctrl.c
SRCS += term.c
SRCS += args.c
SRCS += udpimg.c

capture: $(SRCS)
	$(CC) $(SRCS) -o $(BIN) $(CFLAGS) `pkg-config --cflags --libs opencv`

clean:
	rm -f $(BIN)

#copy executable into target
target:
	scp $(BIN) root@$(TARGET):/usr/bin
