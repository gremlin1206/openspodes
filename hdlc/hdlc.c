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
#include <string.h>
#include <time.h>

#include <dlms.h>

#include "hdlc.h"

//#define DLMS_HDLC_DEBUG 1

#ifdef DLMS_HDLC_DEBUG
#  define PRINTF printf
#else
#  define PRINTF(...)
#endif

#define LLC_REMOTE_LSAP            0xE6
#define LLC_LOCAL_LSAP_REQUEST     0xE6
#define LLC_LOCAL_LSAP_RESPONSE    0xE7
#define LLC_HEADER_LENGTH          3

#define HDLC_BYTE_TIMEOUT_MS       500
#define HDLC_INACTIVITY_TIMEOUT_MS 6000

static int hdlc_send_dm_response(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	int ret;
	struct hdlc_frame_t response;
	PRINTF("Send DM response\n");

	if (!frame->control.pf)
		return 0;

	memset(&response, 0, sizeof(response));

	response.format.S = 0;
	response.format.type = HDLC_TYPE_3;

	response.control.code = HDLC_RESPONSE_DM;
	response.control.pf = 1;

	response.dest_address = frame->src_address;
	response.src_address  = ctx->hdlc_address;

	ret = hdlc_bs_put_frame(&ctx->out_bs, &response);
	if (ret < 0)
		return ret;

	ctx->sendfn(ctx, ctx->out_bs.frame, ctx->out_bs.length);

	return ret;
}

static int hdlc_send_ua_response(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	int ret;
	struct hdlc_frame_t response;
	PRINTF("Send UA response\n");

	if (!frame->control.pf)
		return 0;

	memset(&response, 0, sizeof(response));

	response.format.S = 0;
	response.format.type = HDLC_TYPE_3;

	response.control.code = HDLC_RESPONSE_UA;
	response.control.pf = 1;

	response.dest_address = frame->src_address;
	response.src_address  = ctx->hdlc_address;

	ret = hdlc_bs_put_frame(&ctx->out_bs, &response);
	if (ret < 0)
		return ret;

	ctx->sendfn(ctx, ctx->out_bs.frame, ctx->out_bs.length);

	return ret;
}

static int hdlc_send_rr_response(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	int ret;
	struct hdlc_frame_t response;
	PRINTF("Send RR response\n");

	if (!frame->control.pf)
		return 0;

	memset(&response, 0, sizeof(response));

	response.format.S = 0;
	response.format.type = HDLC_TYPE_3;

	response.control.code = HDLC_FRAME_RR;
	response.control.pf = 1;
	response.control.nr = ctx->nr;

	response.dest_address = frame->src_address;
	response.src_address  = ctx->hdlc_address;

	ret = hdlc_bs_put_frame(&ctx->out_bs, &response);
	if (ret < 0)
		return ret;

	ctx->sendfn(ctx, ctx->out_bs.frame, ctx->out_bs.length);

	return ret;
}

static int hdlc_send_i_response(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	int ret;
	struct hdlc_frame_t response;
	unsigned int max_info_len;
	unsigned char *data;
	unsigned int length;

	PRINTF("Send I response\n");

	if (!frame->control.pf)
		return 0;

	memset(&response, 0, sizeof(response));

	response.format.S = 0;
	response.format.type = HDLC_TYPE_3;

	response.control.code = HDLC_FRAME_I;
	response.control.pf = 1;
	response.control.ns = ctx->ns;
	response.control.nr = ctx->nr;

	response.dest_address = frame->src_address;
	response.src_address  = ctx->hdlc_address;

	if (ctx->pdu_output_offset >= ctx->pdu.length)
		return -1;

	length = ctx->pdu.length - ctx->pdu_output_offset;
	data = (unsigned char*)ctx->pdu.data + ctx->pdu_output_offset;

	max_info_len = hdlc_frame_max_info_length(frame, ctx->out_bs.max_length);
	if (length > max_info_len) {
		PRINTF("set fragmented bit\n");
		length = max_info_len;
		response.format.S = 1;
	}

	response.info = data;
	response.info_len = length;

	ret = hdlc_bs_put_frame(&ctx->out_bs, &response);
	if (ret < 0)
		return ret;

	ctx->sendfn(ctx, ctx->out_bs.frame, ctx->out_bs.length);

	ctx->last_sent_length = length;

	return 0;
}

static int hdlc_send_frmr_response(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	int ret;
	struct hdlc_frame_t response;
	PRINTF("Send FRMR response\n");

	if (!frame->control.pf)
		return 0;

	memset(&response, 0, sizeof(response));

	response.format.S = 0;
	response.format.type = HDLC_TYPE_3;

	response.control.code = HDLC_RESPONSE_FRMR;
	response.control.pf = 1;

	response.dest_address = frame->src_address;
	response.src_address  = ctx->hdlc_address;

	ret = hdlc_bs_put_frame(&ctx->out_bs, &response);
	if (ret < 0)
		return ret;

	ctx->sendfn(ctx, ctx->out_bs.frame, ctx->out_bs.length);

	return ret;
}

static void hdlc_disconnect(struct hdlc_ctx_t *ctx)
{
	PRINTF("HDLC disconnect\n");

	hdlc_bs_reset(&ctx->in_bs);
	hdlc_bs_reset(&ctx->out_bs);
	cosem_pdu_reset(&ctx->pdu);

	ctx->nr = ctx->ns = 0;

	ctx->state = HDLC_STATE_NDM;

	dlms_close_association(ctx->dlms);
}

static int hdlc_cmd_disc(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	hdlc_state_t state = ctx->state;

	PRINTF("DISC command received\n");

	hdlc_disconnect(ctx);

	if (state == HDLC_STATE_NDM)
		hdlc_send_dm_response(ctx, frame);
	else
		hdlc_send_ua_response(ctx, frame);

	return HDLC_ABORT_RECEIVE;
}

static int hdlc_cmd_snrm(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	PRINTF("SNRM command received\n");

	ctx->state = HDLC_STATE_NRM;

	hdlc_send_ua_response(ctx, frame);

	return HDLC_OK;
}

static int hdlc_cmd_rr(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	unsigned int pdu_length;
	PRINTF("RR command received\n");

	if (ctx->state == HDLC_STATE_NDM) {
		hdlc_send_dm_response(ctx, frame);
		return HDLC_OK;
	}

	pdu_length = ctx->pdu.length;

	if (ctx->ns != frame->control.nr) {
		hdlc_send_frmr_response(ctx, frame);
		return HDLC_OK;
	}

	ctx->pdu_output_offset += ctx->last_sent_length;

	if (pdu_length > 0 && ctx->pdu_output_offset < pdu_length) {
		hdlc_send_i_response(ctx, frame);
		ctx->ns = (ctx->ns + 1) & 0x7;
	}
	else
		hdlc_send_rr_response(ctx, frame);

	return HDLC_OK;
}

static int llc_input(struct hdlc_ctx_t *ctx, struct cosem_pdu_t *pdu)
{
	int ret;
	unsigned char *llc_header;

	if (pdu->length < LLC_HEADER_LENGTH) {
		PRINTF("no LLC header\n");
		return -1;
	}

#ifdef DLMS_HDLC_DEBUG
	unsigned int i;
	for (i = 0; i < pdu->length; i++) {
		printf("%02X ", pdu->data[i]);
	}
	printf("\n");
#endif

	llc_header = cosem_pdu_header(pdu);

	if (llc_header[0] != LLC_REMOTE_LSAP || llc_header[1] != LLC_LOCAL_LSAP_REQUEST) {
		PRINTF("invalid LLC header\n");
		return -1;
	}

	ret = dlms_input(ctx->dlms, pdu);
	if (ret < 0)
		return ret;

	/*
	 * Set response LLC header
	 */
	llc_header[1] = LLC_LOCAL_LSAP_RESPONSE;
	llc_header[2] = 0;

	return ret;
}

static int hdlc_cmd_i(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	int ret;
	int fragmented;

	PRINTF("I command received\n");

	if (ctx->state == HDLC_STATE_NDM) {
		hdlc_send_dm_response(ctx, frame);
		return HDLC_OK;
	}

	if (ctx->nr != frame->control.ns) {
		PRINTF("nr-ns mismatch\n");
		hdlc_send_rr_response(ctx, frame);
		return HDLC_OK;
	}

	if (ctx->ns != frame->control.nr) {
		PRINTF("ns-nr mismatch\n");
		hdlc_send_frmr_response(ctx, frame);
		return HDLC_OK;
	}

	ctx->nr = (frame->control.ns + 1) & 0x7;
	fragmented = frame->format.S;

	if (ctx->last_sent_length) {
		/*
		 * Drop the response if we received a new data
		 */
		cosem_pdu_reset(&ctx->pdu);
		ctx->pdu_output_offset = 0;
		ctx->last_sent_length = 0;
	}

	ret = cosem_pdu_append_buffer(&ctx->pdu, frame->info, frame->info_len);
	if (ret < 0) {
		// TODO: add proper handling of PDU oversize
		cosem_pdu_reset(&ctx->pdu);
		return HDLC_OK;
	}

	if (fragmented) {
		hdlc_send_rr_response(ctx, frame);
		return HDLC_OK;
	}

	ctx->pdu_output_offset = 0;
	ctx->last_sent_length = 0;

	ctx->pdu.server_address = frame->dest_address;
	ctx->pdu.client_address = frame->src_address;

	ret = llc_input(ctx, &ctx->pdu);
	if (ret < 0) {
		PRINTF("dlms handling error\n");
		cosem_pdu_reset(&ctx->pdu);
		hdlc_send_rr_response(ctx, frame);
		return HDLC_OK;
	}

	hdlc_send_i_response(ctx, frame);
	ctx->ns = (ctx->ns + 1) & 0x7;

	return HDLC_OK;
}

static int hdlc_process_frame(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	hdlc_control_code_t ctrl = frame->control.code;

#ifdef DLMS_HDLC_DEBUG
	PRINTF("Frame received:\n");
	hdlc_frame_print(frame);
#endif

	if (memcmp(&frame->dest_address, &ctx->hdlc_address, sizeof(struct hdlc_address_t)) != 0) {
		PRINTF("Ignore wrong address\n");
		return HDLC_OK;
	}

	switch (ctrl)
	{
	case HDLC_FRAME_I:
		return hdlc_cmd_i(ctx, frame);

	case HDLC_COMMAND_DISC:
		return hdlc_cmd_disc(ctx, frame);

	case HDLC_COMMAND_SNRM:
		return hdlc_cmd_snrm(ctx, frame);

	case HDLC_FRAME_RR:
		return hdlc_cmd_rr(ctx, frame);

	default:
		hdlc_send_frmr_response(ctx, frame);
		return HDLC_OK;
	}
}

static int hdlc_handle_input_frame(struct hdlc_ctx_t *ctx, struct hdlc_bs_t *bs)
{
	int ret;
	struct hdlc_frame_t frame;

	ret = hdlc_frame_parse(&frame, bs);
	if (ret < 0) {
		PRINTF("Fail to parse frame\n");
		return HDLC_OK;
	}

	return hdlc_process_frame(ctx, &frame);
}

static uint64_t hdlc_get_timestamp_ms(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

int hdlc_receive(struct hdlc_ctx_t *ctx, uint8_t *bytes, uint32_t length)
{
	int ret;
	uint32_t i;
	struct hdlc_bs_t *bs = &ctx->in_bs;
	uint64_t timeout;

	PRINTF("hdlc receive len: %u\n", length);
	for (i = 0; i < length; i++)
	{
		PRINTF("%02X ", bytes[i]);
	}
	PRINTF("\n");

	if (ctx->last_receive_timestamp == 0)
	{
		ctx->last_receive_timestamp = hdlc_get_timestamp_ms();
	}

	timeout = hdlc_get_timestamp_ms() - ctx->last_receive_timestamp;
	PRINTF("timeout: %u\n", (unsigned int)timeout);
	if (timeout > HDLC_INACTIVITY_TIMEOUT_MS)
	{
		hdlc_bs_reset(bs);
		hdlc_disconnect(ctx);
	}
	else if (timeout > HDLC_BYTE_TIMEOUT_MS)
	{
		hdlc_bs_reset(bs);
	}

	ctx->last_receive_timestamp = hdlc_get_timestamp_ms();

	while (length > 0)
	{
		ret = hdlc_bs_receive(bs, bytes, length);
		if (ret < 0)
			return ret;

		if (ret >= length)
			length = 0;
		else
			length -= ret;

		if (bs->length > 0)
		{
			// Handle received frame
			//
			ret = hdlc_handle_input_frame(ctx, bs);

			// And forget it
			//
			bs->length = 0;

			if (ret == HDLC_ABORT_RECEIVE)
				return ret;
		}
	}

	return 0;
}

int hdlc_init(struct hdlc_ctx_t *ctx)
{
	memset(ctx, 0, sizeof(*ctx));

	ctx->state = HDLC_STATE_NDM;

	hdlc_bs_init(&ctx->in_bs,  ctx->in_buf,  HDLC_BUFFER_SIZE);
	hdlc_bs_init(&ctx->out_bs, ctx->out_buf, HDLC_BUFFER_SIZE);

	ctx->nr = ctx->ns = 0;

	cosem_pdu_init(&ctx->pdu, LLC_HEADER_LENGTH);

	return 0;
}
