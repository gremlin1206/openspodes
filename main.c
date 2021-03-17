#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>

#include "dlms.h"

#include <crypto/aes.h>

int fd;

#if 0
unsigned char IC[] = { 0x00, 0x00, 0x00, 0x01 };
unsigned char AK[] = { 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF };
unsigned char SC[] = { 0x10 };

unsigned char EK[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
unsigned char IV[] = { 0x4D, 0x4D, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01 };

unsigned char CtoS[] = { 0x4B, 0x35, 0x36, 0x69, 0x56, 0x61, 0x67, 0x59 };
unsigned char StoC[] = { 0x50, 0x36, 0x77, 0x52, 0x4A, 0x32, 0x31, 0x46 };

//unsigned char K[] =  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
//unsigned char IV[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
//unsigned char P[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static int auth_test(void)
{
	unsigned char *key = (unsigned char*)"SettingRiM489.2X";
	unsigned char challenge[] = {0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31};
	unsigned char hash[16];



	//unsigned int offset = 0;
	//unsigned char msg1[128];
	//unsigned char hash[16];

	//memcpy(msg1 + offset, SC, sizeof(SC)); offset += sizeof(SC);
	//memcpy(msg1 + offset, AK, sizeof(AK)); offset += sizeof(AK);
	//memcpy(msg1 + offset, StoC, sizeof(StoC)); offset += sizeof(StoC);

	//aes_gmac(EK, sizeof(EK), IV, sizeof(IV), msg1, offset, hash);

	int i;
	/*printf("M: ");
	for (i = 0; i < offset; i++) {
		printf("%02X ", msg1[i]);
	}
	printf("\n");*/

	printf("H: ");
	for (i = 0; i < 16; i++) {
		printf("%02X ", hash[i]);
	}
	printf("\n");

	return 0;
}
#endif

static int sendfn(struct hdlc_ctx_t *ctx, uint8_t *bytes, uint32_t length)
{
	uint32_t i;
	printf("send data %u:\n", length);

	for (i = 0; i < length; i++)
		printf("%02X ", bytes[i]);
	printf("\n");

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

#if 0
	unsigned char buf[] = {/*0x7E,*/ 0xA0, 0x08, 0x21, 0x02, 0x21, 0x10, 0x57, 0x21, 0xE6, 0xE7, 0x00, 0x0E, 0x05, 0x05, 0x02, 0x57, 0x21/*, 0x7E*/};
	struct hdlc_frame_t f;
	struct hdlc_bs_t bs;

	bs.frame = buf;
	bs.length = sizeof(buf);

	ret = hdlc_frame_parse(&f, &bs);
	printf("ret: %i\n", ret);

	return 0;
#endif

	//auth_test();
	//return 0;

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

	dlms_init(&dlms);

	hdlc_init(&hdlc);
	hdlc.dlms = &dlms;
	hdlc.sendfn = sendfn;
	hdlc.hdlc_address.len = 2;
	hdlc.hdlc_address.upper = 1;
	hdlc.hdlc_address.lower = 16;

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
