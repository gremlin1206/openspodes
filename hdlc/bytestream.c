/*
MIT License

Copyright (c) 2021 gremlin1206

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <errno.h>
#include <stdio.h>

#include "bytestream.h"
#include "fcs16.h"
#include "frame.h"

#define HDLC_FLAG_SEQUENCE  0x7E

static int hdlc_bs_insert(struct hdlc_bs_t *bs, uint8_t b)
{
	uint32_t length = bs->length;
	uint32_t max_length = bs->max_length;
	uint8_t *p = bs->frame + length;

	if (length >= max_length)
		return -1;

	length++;
	*p = b;

	bs->length = length;

	return 0;
}

static int hdlc_bs_insert_fcs(struct hdlc_bs_t *bs, uint16_t fcs)
{
	int ret;

	// Insert FCS low byte
	//
	ret = hdlc_bs_insert(bs, (uint8_t)(fcs & 0xFF));
	if (ret < 0)
		return ret;

	// Insert FCS high byte
	//
	ret = hdlc_bs_insert(bs, (uint8_t)(fcs >> 8));
	if (ret < 0)
		return ret;

	return 0;
}

void hdlc_bs_init(struct hdlc_bs_t *bs, void *buffer, uint32_t max_length)
{
	bs->frame = buffer;
	bs->max_length = max_length;

	hdlc_bs_reset(bs);
}

void hdlc_bs_reset(struct hdlc_bs_t *bs)
{
	bs->length = 0;
	bs->frame_index = 0;

	bs->started = bs->ended = 0;
	bs->expected_length = 0;
}

int hdlc_bs_put(struct hdlc_bs_t *bs, struct hdlc_frame_t *frame)
{
	int ret;
	int hdr_len;
	int i;
	unsigned char hdr[32];
	uint16_t hcs, fcs;

	hdlc_bs_reset(bs);

	ret = hdlc_bs_insert(bs, HDLC_FLAG_SEQUENCE);
	if (ret < 0)
	{
		printf("fail to insert frame start\n");
		return ret;
	}

	ret = hdlc_frame_encode_hdr(hdr, sizeof(hdr), frame);
	if (ret < 0)
	{
		printf("fail to encode frame header\n");
		return ret;
	}

	fcs = FCS16_INIT_VALUE;

	hdr_len = ret;
	printf("header length: %i\n", hdr_len);
	for (i = 0; i < hdr_len; i++)
	{
		unsigned char b = hdr[i];
		fcs = fcs16(fcs, b);

		ret = hdlc_bs_insert(bs, b);
		if (ret < 0)
			return ret;
	}

	fcs ^= 0xFFFF;

	ret = hdlc_bs_insert_fcs(bs, fcs);
	if (ret < 0)
		return ret;

	return hdlc_bs_insert(bs, HDLC_FLAG_SEQUENCE);
}

int hdlc_bs_receive(struct hdlc_bs_t *bs, uint8_t *bytes, uint32_t length)
{
	int bytes_received;
	uint8_t value;
	uint32_t i;
	uint8_t *dest;
	unsigned int dest_len;

	dest = bs->frame;

	for (i = 0; i < length; i++)
	{
		value = bytes[i];

		if (!bs->started)
		{
			if (value == HDLC_FLAG_SEQUENCE)
			{
				if ((i < (length - 1)) && (bytes[i + 1] == HDLC_FLAG_SEQUENCE))
				{
					continue;
				}

				printf("START FRAME @ %u\n", i);
				bs->started = 1;
			}
		}
		else
		{
			if (value == HDLC_FLAG_SEQUENCE)
			{
				if (bs->frame_index <= 2)
				{
					printf("END FRAME @ %u\n", i);
					bs->ended = 1;
					break;
				}

				if (bs->expected_length == 0)
				{
					bs->expected_length = ((dest[0] & 0x7) << 8) |  dest[1];
				}

				printf("expected_length: %u bs->frame_index: %u\n", bs->expected_length, bs->frame_index);
				if (bs->frame_index >= bs->expected_length)
				{
					bs->ended = 1;
					break;
				}
			}

			// Check buffer capacity
			//
			if (bs->frame_index >= bs->max_length)
			{
				hdlc_bs_reset(bs);
				break;
			}

			dest[bs->frame_index++] = value;
		}
	}

	if (!bs->started || !bs->ended)
	{
		dest_len = 0;
		bytes_received = (int)length;
	}
	else
	{
		bytes_received = i;
		dest_len = bs->frame_index;

		printf("dest_len: %u bytes_received: %i\n", dest_len, bytes_received);
		for (i = 0; i < dest_len; i++)
		{
			printf("%02X ", ((uint8_t*)dest)[i]);
		}
		printf("\n");

		printf("reset bytestream\n");
		hdlc_bs_reset(bs);
	}

	bs->length = dest_len;
	printf("bs->length: %u\n", bs->length);

	return bytes_received;
}
