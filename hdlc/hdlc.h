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

#ifndef OPENSPODES_HDLC_H
#define OPENSPODES_HDLC_H

#define HDLC_BUFFER_SIZE 128
#define HDLC_TYPE_3      0x0A

#include <stdint.h>

#include <cosem/pdu.h>

#include "frame.h"
#include "bytestream.h"

struct hdlc_ctx_t;
struct dlms_ctx_t;

typedef int (*hdlc_sendfn_t)(struct hdlc_ctx_t *ctx, uint8_t *bytes, uint32_t length);

typedef enum {
	HDLC_STATE_NDM,
	HDLC_STATE_NRM,
} hdlc_state_t;

typedef enum {
	HDLC_ERROR         = -1,
	HDLC_OK            = 0,
	HDLC_ABORT_RECEIVE = 1,
} hdlc_result_t;

struct hdlc_ctx_t
{
	struct hdlc_address_t hdlc_address;

	struct hdlc_bs_t in_bs;
	struct hdlc_bs_t out_bs;

	hdlc_state_t state;

	hdlc_sendfn_t sendfn;

	uint8_t in_buf[HDLC_BUFFER_SIZE];
	uint8_t out_buf[HDLC_BUFFER_SIZE];
	uint8_t nr;
	uint8_t ns;

	uint64_t last_receive_timestamp;

	struct cosem_pdu_t *input_pdu;
	struct cosem_pdu_t *output_pdu;

	unsigned char *last_sent_data;
	unsigned int last_sent_length;

	struct dlms_ctx_t *dlms;
};

#ifdef __cplusplus
extern "C" {
#endif

int hdlc_init(struct hdlc_ctx_t *ctx, struct cosem_pdu_t *input_pdu, struct cosem_pdu_t *output_pdu);
int hdlc_receive(struct hdlc_ctx_t *ctx, uint8_t *bytes, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* OPENSPODES_HDLC_H */
