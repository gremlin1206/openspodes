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

static const unsigned char llc_pdu_request_hdr[] = {
		0xE6, 0xE6, 0x00
};

static int dlms_process_pdu(struct dlms_ctx_t *ctx)
{
	unsigned char *p = ctx->pdu.data;
	unsigned int len = ctx->pdu.length;



	return 0;
}

int dlms_init(struct dlms_ctx_t *ctx)
{
	ctx->pdu.length = 0;
	return 0;
}

int dlms_input(struct dlms_ctx_t *ctx, unsigned char *p, unsigned int len, int more)
{
	unsigned int pdu_length = ctx->pdu.length;

	if (pdu_length + len > DLMS_MAX_PDU_SIZE)
	{
		return -1;
	}

	memcpy(&ctx->pdu.data[pdu_length], p, len);
	ctx->pdu.length = pdu_length + len;

	if (!more)
	{
		dlms_process_pdu(ctx);
	}

	return 0;
}
