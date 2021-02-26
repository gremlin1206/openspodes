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

#ifndef OPENSPODES_HDLC_BYTESTREAM_H
#define OPENSPODES_HDLC_BYTESTREAM_H

#include <stdint.h>

struct hdlc_frame_t;

struct hdlc_bs_t
{
	void *frame;
	int frame_index;
	int length;
	uint32_t max_length;

	int started;
	int ended;
	int expected_length;
};

#ifdef __cplusplus
extern "C" {
#endif

void hdlc_bs_init(struct hdlc_bs_t *bs, void *buffer, uint32_t max_length);
void hdlc_bs_reset(struct hdlc_bs_t *bs);

int hdlc_bs_put(struct hdlc_bs_t *bs, struct hdlc_frame_t *frame);
int hdlc_bs_receive(struct hdlc_bs_t *bs, uint8_t *bytes, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* OPENSPODES_HDLC_BYTESTREAM_H */
