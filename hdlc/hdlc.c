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

#include <dlms/dlms.h>

#include "hdlc.h"

#define HDLC_BYTE_TIMEOUT_MS 500
#define HDLC_INACTIVITY_TIMEOUT_MS 5000

static void hdlc_send_dm_response(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	struct hdlc_frame_t response;
	printf("Send DM response\n");

	memset(&response, 0, sizeof(response));

	response.format.S = 0;
	response.format.type = HDLC_TYPE_3;

	response.control.code = HDLC_RESPONSE_DM;
	response.control.pf = 1;

	response.dest_address = frame->src_address;
	response.src_address  = ctx->hdlc_address;

	hdlc_bs_put(&ctx->out_bs, &response);
	ctx->sendfn(ctx, ctx->out_bs.frame, ctx->out_bs.length);
}

static void hdlc_send_ua_response(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	struct hdlc_frame_t response;
	printf("Send UA response\n");

	memset(&response, 0, sizeof(response));

	response.format.S = 0;
	response.format.type = HDLC_TYPE_3;

	response.control.code = HDLC_RESPONSE_UA;
	response.control.pf = 1;

	response.dest_address = frame->src_address;
	response.src_address  = ctx->hdlc_address;

	hdlc_bs_put(&ctx->out_bs, &response);
	ctx->sendfn(ctx, ctx->out_bs.frame, ctx->out_bs.length);
}

static void hdlc_send_rr_response(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	struct hdlc_frame_t response;
	printf("Send RR response\n");

	memset(&response, 0, sizeof(response));

	response.format.S = 0;
	response.format.type = HDLC_TYPE_3;

	response.control.code = HDLC_FRAME_RR;
	response.control.pf = 1;
	response.control.nr = ctx->nr;

	response.dest_address = frame->src_address;
	response.src_address  = ctx->hdlc_address;

	hdlc_bs_put(&ctx->out_bs, &response);
	ctx->sendfn(ctx, ctx->out_bs.frame, ctx->out_bs.length);
}

static void hdlc_send_i_response(struct hdlc_ctx_t *ctx, const unsigned char *data, unsigned int length, struct hdlc_frame_t *frame)
{
	struct hdlc_frame_t response;
	printf("Send I response\n");

	memset(&response, 0, sizeof(response));

	response.format.S = 0;
	response.format.type = HDLC_TYPE_3;

	response.control.code = HDLC_FRAME_I;
	response.control.pf = 1;

	response.dest_address = frame->src_address;
	response.src_address  = ctx->hdlc_address;

	response.info = (unsigned char*)data;
	response.info_len = length;

	hdlc_bs_put(&ctx->out_bs, &response);
	ctx->sendfn(ctx, ctx->out_bs.frame, ctx->out_bs.length);
}

static void hdlc_send_frmr_response(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	struct hdlc_frame_t response;
	printf("Send FRMR response\n");

	memset(&response, 0, sizeof(response));

	response.format.S = 0;
	response.format.type = HDLC_TYPE_3;

	response.control.code = HDLC_RESPONSE_FRMR;
	response.control.pf = 1;

	response.dest_address = frame->src_address;
	response.src_address  = ctx->hdlc_address;

	hdlc_bs_put(&ctx->out_bs, &response);
	ctx->sendfn(ctx, ctx->out_bs.frame, ctx->out_bs.length);
}

static void hdlc_disconnect(struct hdlc_ctx_t *ctx)
{
	dlms_drop_pdu(ctx->dlms);

	hdlc_bs_reset(&ctx->in_bs);
	hdlc_bs_reset(&ctx->out_bs);

	ctx->nr = ctx->ns = 0;

	ctx->state = HDLC_STATE_NDM;
}

static int hdlc_cmd_disc(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	hdlc_state_t state = ctx->state;

	printf("DISC command received\n");

	hdlc_disconnect(ctx);

	if (state == HDLC_STATE_NDM)
		hdlc_send_dm_response(ctx, frame);
	else
		hdlc_send_ua_response(ctx, frame);

	return HDLC_ABORT_RECEIVE;
}

static int hdlc_cmd_snrm(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	printf("SNRM command received\n");

	ctx->state = HDLC_STATE_NRM;

	hdlc_send_ua_response(ctx, frame);

	return HDLC_OK;
}

static int hdlc_cmd_rr(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	printf("RR command received\n");

	if (ctx->state == HDLC_STATE_NDM)
	{
		hdlc_send_dm_response(ctx, frame);
		return HDLC_OK;
	}

	if (ctx->ns != frame->control.nr)
	{
		hdlc_send_frmr_response(ctx, frame);
		return HDLC_OK;
	}

	hdlc_send_rr_response(ctx, frame);

	return HDLC_OK;
}

static int hdlc_cmd_i(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	int ret;

	printf("I command received\n");

	if (ctx->state == HDLC_STATE_NDM) {
		hdlc_send_dm_response(ctx, frame);
		return HDLC_OK;
	}

	if (ctx->nr != frame->control.ns) {
		hdlc_send_rr_response(ctx, frame);
		return HDLC_OK;
	}

	ret = dlms_input(ctx->dlms, frame->info, frame->info_len, frame->format.S);
	if (ret < 0)
		return HDLC_ABORT_RECEIVE;

	ctx->nr = (frame->control.ns + 1) & 0x7;

	if (ret == 0) {
		hdlc_send_i_response(ctx, ctx->dlms->pdu.data, ctx->dlms->pdu.length, frame);
		dlms_drop_pdu(ctx->dlms);
	}
	else
		hdlc_send_rr_response(ctx, frame);

	return HDLC_OK;
}

static int hdlc_process_frame(struct hdlc_ctx_t *ctx, struct hdlc_frame_t *frame)
{
	hdlc_control_code_t ctrl = frame->control.code;

#if 1
	printf("Frame received:\n");
	hdlc_frame_print(frame);
#endif

	if (memcmp(&frame->dest_address, &ctx->hdlc_address, sizeof(struct hdlc_address_t)) != 0)
	{
		printf("Ignore wrong address\n");
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
	if (ret < 0)
	{
		printf("Fail to pare frame\n");
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

int hdlc_init(struct hdlc_ctx_t *ctx)
{
	memset(ctx, 0, sizeof(*ctx));

	ctx->state = HDLC_STATE_NDM;

	hdlc_bs_init(&ctx->in_bs,  ctx->in_buf,  HDLC_BUFFER_SIZE);
	hdlc_bs_init(&ctx->out_bs, ctx->out_buf, HDLC_BUFFER_SIZE);

	ctx->nr = ctx->ns = 0;

	return 0;
}

int hdlc_receive(struct hdlc_ctx_t *ctx, uint8_t *bytes, uint32_t length)
{
	int ret;
	uint32_t i;
	struct hdlc_bs_t *bs = &ctx->in_bs;
	uint64_t timeout;

	printf("hdlc receive len: %u\n", length);
	for (i = 0; i < length; i++)
	{
		printf("%02X ", bytes[i]);
	}
	printf("\n");

	if (ctx->last_receive_timestamp == 0)
	{
		ctx->last_receive_timestamp = hdlc_get_timestamp_ms();
	}

	timeout = hdlc_get_timestamp_ms() - ctx->last_receive_timestamp;
	printf("timeout: %u\n", (unsigned int)timeout);
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
