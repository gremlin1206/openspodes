
OBJS += main.o

OBJS += hdlc/fcs16.o
OBJS += hdlc/bytestream.o
OBJS += hdlc/frame.o
OBJS += hdlc/hdlc.o

OBJS += dlms/dlms.o

APP := spodes

LD := gcc
CC := gcc

INCLUDES += -I$(CURDIR)

CFLAGS := $(INCLUDES) -Wall

app: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(APP)

%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

clean:
	rm -f $(OBJS) $(APP)
