
OBJS += main.o

OBJS += hdlc/fcs16.o
OBJS += hdlc/bytestream.o
OBJS += hdlc/frame.o
OBJS += hdlc/hdlc.o

OBJS += dlms.o

OBJS += cosem/cosem.o
OBJS += cosem/asn1.o
OBJS += cosem/pdu.o
OBJS += cosem/class.o
OBJS += cosem/objects.o
OBJS += cosem/association.o
OBJS += cosem/class_association_ln.o

OBJS += crypto/aes-gcm.o
OBJS += crypto/aes-internal-enc.o
OBJS += crypto/aes-internal.o

OBJS += spodes/spodes.o

APP := spodes_sim

LD := gcc
CC := gcc

INCLUDES += -I$(CURDIR)

CFLAGS := $(INCLUDES) -Wall
LDFLAGS :=

app: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(APP)

%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

clean:
	rm -f $(OBJS) $(APP)
