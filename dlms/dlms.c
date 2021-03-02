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

#include <stdio.h>
#include <string.h>

#include <cosem/cosem.h>

#include "dlms.h"

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
static int dlms_parse_get_request(struct get_request_t *request, unsigned char *p, unsigned int len)
{
	unsigned short class_id;

	printf("parce get request len: %u\n", len);

	if (len < 12)
		return -1;

	request->type = *p; p++;
	request->invoke_id_and_priority.byte = *p; p++;

	printf("  type: %i\n", request->type);

	switch (request->type) {
	case get_request_normal_type:
		class_id  = *p; p++;
		class_id <<= 8;
		class_id |= *p; p++;
		request->get_request_normal.cosem_attribute_descriptor.class_id = class_id;
		memcpy(request->get_request_normal.cosem_attribute_descriptor.instance_id, p, 6); p += 6;
		request->get_request_normal.cosem_attribute_descriptor.attribute_id = *p; p++;
		break;

	case get_request_next_type:
		break;

	case get_request_with_list_type:
		break;

	default:
		return -1;
	}

	printf("return ok\n");

	return 0;
}

static int dlms_process_get_request(struct dlms_ctx_t *ctx, unsigned char *p, unsigned int len)
{
	int ret;
	struct get_request_t request;

	ret = dlms_parse_get_request(&request, p, len);
	if (ret < 0)
		return ret;

	return cosem_process_get_request(ctx->cosem, &ctx->pdu, &request);
}

static int dlms_process_apdu(struct dlms_ctx_t *ctx, unsigned char *p, unsigned int len)
{
	//int ret;
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

	if (pdu_length + len > DLMS_MAX_PDU_SIZE) {
		dlms_drop_pdu(ctx);
		return -1;
	}

	memcpy(&ctx->pdu.data[pdu_length], p, len);
	ctx->pdu.length = pdu_length + len;

	if (!more) {
		ret = dlms_process_llc(ctx, ctx->pdu.data, len);
		if (ret < 0)
			return ret;
	}

	return more;
}

void dlms_drop_pdu(struct dlms_ctx_t *ctx)
{
	ctx->pdu.length = 0;
}
