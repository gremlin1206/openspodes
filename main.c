#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>

#include <hdlc/hdlc.h>
#include <dlms/dlms.h>

int fd;

static int sendfn(struct hdlc_ctx_t *ctx, uint8_t *bytes, uint32_t length)
{
	printf("send data %u\n", length);
	return write(fd, bytes, length);
}

#define DEBUG 0

#if DEBUG

#if 1
unsigned char buffer1[] = {
		0x7E, 0xA0, 0x2C, 0x02, 0x21, 0x21, 0x10, 0x17,
		0x2A, 0xE6, 0xE6, 0x00, 0x60, 0x1D, 0xA1, 0x09,
		0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01,
		0x01, 0xBE, 0x10, 0x04, 0x0E, 0x01, 0x00, 0x00,
		0x00, 0x06, 0x5F, 0x1F, 0x04, 0x00, 0x00, 0x7E,
		0x1F, 0x10, 0x00, 0xB0, 0xAD, 0x7E
};
#endif

#if 1
unsigned char buffer2[] = {
		0x7E, 0xA0, 0x1F, 0x02, 0x21, 0x21, 0x93, 0x98, 0x55, 0x81, 0x80, 0x12,
		0x05, 0x01, 0x80, 0x06, 0x01, 0x80, 0x07, 0x04, 0x00, 0x00, 0x00, 0x01,
		0x08, 0x04, 0x00, 0x00, 0x00, 0x01, 0x9A, 0xB2, 0x7E
};
#endif

//int ret = sizeof(buffer);
#endif

//7E
// A0 2C 02 21 21 14 33 6C E6 E6 00 60 1D A1 09 06
// 07 60 85 74 05 08 01 01 BE 10 04 0E 01 00 00 00
// 06 5F 1F 04 00 00
// 7E

// 7E
// A0 2C 02 21 21 10 17 2A E6 E6 00 60 1D A1 09 06
// 07 60 85 74 05 08 01 01 BE 10 04 0E 01 00 00 00
// 06 5F 1F 04 00 00 7E 1F 10 00 B0 AD
// 7E

int main(void)
{
	struct hdlc_ctx_t hdlc;
	struct dlms_ctx_t dlms;
	int ret;

#if !DEBUG
	struct sockaddr_un addr;

	if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
	{
		perror("socket error");
		exit(-1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, "/tmp/vboxcom1", sizeof(addr.sun_path)-1);

	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		perror("connect error");
		exit(-1);
	}
#endif

	hdlc_init(&hdlc);
	hdlc.sendfn = sendfn;
	hdlc.hdlc_address.len = 2;
	hdlc.hdlc_address.upper = 1;
	hdlc.hdlc_address.lower = 16;

	dlms_init(&dlms);
	hdlc.dlms = &dlms;

#if !DEBUG
	while (1)
	{
		unsigned char buffer[128];

		ret = read(fd, buffer, sizeof(buffer));
		if (ret <= 0)
			continue;
#endif
#if DEBUG
		ret = hdlc_receive(&hdlc, buffer1, sizeof(buffer1));
		printf("hdlc receive ret: %i\n", ret);

		ret = hdlc_receive(&hdlc, buffer2, sizeof(buffer2));
		printf("hdlc receive ret: %i\n", ret);
#else
		ret = hdlc_receive(&hdlc, buffer, ret);
#endif
#if !DEBUG
	}
#endif

	return 0;
}
