
OBJS += hdlc/fcs16.o
OBJS += hdlc/bytestream.o
OBJS += hdlc/frame.o
OBJS += hdlc/hdlc.o

OBJS += cosem/asn1.o
OBJS += cosem/association.o
OBJS += cosem/cosem.o
OBJS += cosem/pdu.o

OBJS += dlms/class_association_ln.o
OBJS += dlms/class.o
OBJS += dlms/data.o
OBJS += dlms/objects.o
OBJS += dlms/dlms.o

OBJS += crypto/aes-gcm.o
OBJS += crypto/aes-internal-enc.o
OBJS += crypto/aes-internal.o

OBJS += spodes/spodes.o

OBJS += main.o

APP := spodes_sim

LD := gcc
CC := gcc

INCLUDES += -I$(CURDIR)

CFLAGS := $(INCLUDES) -Wall -Winline -g -O0
LDFLAGS :=

app: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(APP)

%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

clean:
	rm -f $(OBJS) $(APP)

cloc:
	cloc cosem dlms spodes dlms.c dlms.h hdlc
