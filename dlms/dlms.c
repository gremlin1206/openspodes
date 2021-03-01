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

#include "dlms.h"

#include <stdio.h>
#include <string.h>

#define LLC_REMOTE_LSAP         0xE6
#define LLC_LOCAL_LSAP_REQUEST  0xE6
#define LLC_LOCAL_LSAP_RESPONSE 0xE6
#define LLC_HEADER_LENGTH       3

enum apdu_tag_t
{
	apdu_tag_aa_request                  = 0x60,
	apdu_tag_aa_response                 = 0x61,
	apdu_tag_get_request                 = 0xC0,
	apdu_tag_set_request                 = 0xC1,
	apdu_tag_event_notification_request  = 0xC2,
	apdu_tag_action_request              = 0xC3,
	apdu_tag_get_response                = 0xC4,
	apdu_tag_set_response                = 0xC5,
	apdu_tag_action_response             = 0xC7,
	apdu_tag_glo_get_request             = 0xC8,
	apdu_tag_glo_set_request             = 0xC9,
	apdu_tag_event_notification_response = 0xCA,
	apdu_tag_glo_action_request          = 0xCB,
	apdu_tag_glo_get_response            = 0xCC,
	apdu_tag_glo_set_response            = 0xCD,
	apdu_tag_glo_action_response         = 0xCF,
};

//static const unsigned char llc_pdu_request_hdr[] = {
//		0xE6, 0xE6, 0x00
//};

static int asn_decode_length(unsigned int *out, const unsigned char **buffer, unsigned int *len)
{
	unsigned int length = *len;
	unsigned int result;
	unsigned char bytes;
	const unsigned char *p = *buffer;
	unsigned char byte;

	if (length < 1)
		return -1;

	byte = p[0];

	if ((byte & 0x80) == 0) {
		result = byte;
	}
	else {
		length--;
		p++;

		bytes = byte & ~0x80;
		if (bytes > length || bytes > 4)
			return -1;

		length -= bytes;

		result = 0;
		for (;;) {
			result |= *p;
			bytes--;
			if (bytes == 0)
				break;
			result <<= 8;
			p++;
		}
	}

	*out = result;
	*buffer = p;
	*len = length;

	return 0;
}

static int dlms_process_aa_request(struct dlms_ctx_t *ctx, const unsigned char *p, unsigned int len)
{
	int ret;
	unsigned int data_length;

	printf("AARQ\n");

	ret = asn_decode_length(&data_length, &p, &len);
	if (ret < 0)
		return ret;

	return 0;
}

// C0
//   01  get-type 0
//   C0  invoke-id-and-priority
//   00 0F class-id
//   00 00 28 00 00 FF instance-id
//   01 attribute-id
//   00 access-selection (00 means not present)
static int dlms_process_get_request(struct dlms_ctx_t *ctx, unsigned char *p, unsigned int len)
{
	unsigned int get_type;
	unsigned int invoke_id_and_priority;
	unsigned int invoke_id;
	unsigned int priority;
	unsigned int service_class;
	unsigned int class_id;
	const unsigned char *instance_id;
	unsigned int attribute_id;

	if (len < 12)
		return -1;

	get_type = *p; p++;

	invoke_id_and_priority = *p; p++;
	invoke_id     = invoke_id_and_priority & 0x0F;
	service_class = (invoke_id_and_priority >> 6) & 0x01;
	priority      = (invoke_id_and_priority >> 7) & 0x01;

	class_id  = *p; p++;
	class_id <<= 8;
	class_id |= *p; p++;

	instance_id = p; p += 6;

	attribute_id = *p; p++;



	return 0;
}

static int dlms_process_apdu(struct dlms_ctx_t *ctx, unsigned char *p, unsigned int len)
{
	int ret;
	unsigned int i;
	unsigned char tag;

	printf("process pdu (%u bytes):\n", len);
	for (i = 0; i < len; i++)
		printf("%02X ", p[i]);
	printf("\n");

	tag = p[0];
	p += 1;
	len -= 1;

	switch (tag)
	{
	case apdu_tag_aa_request:
		return dlms_process_aa_request(ctx, p, len);

	case apdu_tag_get_request:
		return dlms_process_get_request(ctx, p, len);

	default:
		printf("request with unknown tag: %u\n", tag);
	}

	return -1;
}

static int dlms_process_llc(struct dlms_ctx_t *ctx, unsigned char *p, unsigned int len)
{
	if (len <= LLC_HEADER_LENGTH || p[0] != LLC_REMOTE_LSAP || p[1] != LLC_LOCAL_LSAP_REQUEST)
		return -1;

	return dlms_process_apdu(ctx, p + LLC_HEADER_LENGTH, len - LLC_HEADER_LENGTH);
}

int dlms_init(struct dlms_ctx_t *ctx)
{
	ctx->pdu.length = 0;
	return 0;
}

int dlms_input(struct dlms_ctx_t *ctx, unsigned char *p, unsigned int len, int more)
{
	unsigned int pdu_length = ctx->pdu.length;
	int ret;

	if (pdu_length + len > DLMS_MAX_PDU_SIZE)
	{
		return -1;
	}

	memcpy(&ctx->pdu.data[pdu_length], p, len);
	ctx->pdu.length = pdu_length + len;

	ret = 0;

	if (!more) {
		ret = dlms_process_llc(ctx, ctx->pdu.data, len);
		ctx->pdu.length = 0;
	}

	return ret;
}
