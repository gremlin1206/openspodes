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

#ifndef OPENSPODES_HDLC_FRAME_H
#define OPENSPODES_HDLC_FRAME_H

#include <stdint.h>
#include "bytestream.h"

typedef enum {
  HDLC_FRAME_I       = 1,
  HDLC_FRAME_RR      = 2,
  HDLC_FRAME_RNR     = 3,
  HDLC_COMMAND_SNRM  = 4,
  HDLC_COMMAND_DISC  = 5,
  HDLC_RESPONSE_UA   = 6,
  HDLC_RESPONSE_DM   = 7,
  HDLC_RESPONSE_FRMR = 8,
  HDLC_FRAME_UI      = 9,
  HDLC_FRAME_UNKNOWN = 10,
}
hdlc_control_code_t;

struct hdlc_format_t
{
  uint8_t  S;
  uint8_t  type;
};

struct hdlc_address_t
{
	uint16_t upper;
	uint16_t lower;
	uint16_t len;
};

struct hdlc_control_t
{
  hdlc_control_code_t code;
  uint8_t nr;
  uint8_t ns;
  uint8_t pf;
};

struct hdlc_frame_t
{
  struct hdlc_format_t format;
  struct hdlc_address_t dest_address;
  struct hdlc_address_t src_address;
  struct hdlc_control_t control;

  unsigned int length;

  unsigned char *info;
  unsigned int info_len;
};

#ifdef __cplusplus
extern "C" {
#endif

int hdlc_frame_parse(struct hdlc_frame_t *frame, struct hdlc_bs_t *bs);
int hdlc_frame_encode_hdr(unsigned char *hdr, unsigned int length, struct hdlc_frame_t *frame);
unsigned int hdlc_frame_max_info_length(struct hdlc_frame_t *frame, unsigned int hdlc_frame_size);

void hdlc_frame_print(struct hdlc_frame_t *frame);

#ifdef __cplusplus
}
#endif

#endif /* OPENSPODES_HDLC_FRAME_H */
