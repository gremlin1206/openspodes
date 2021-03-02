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

#include "frame.h"
#include "hdlc.h"
#include "fcs16.h"

static int hdlc_put_format_type(unsigned char *p, struct hdlc_format_t value, unsigned int length)
{
	p[0] = ((length >> 8) & 0x7) | (value.type << 4) | (value.S << 3);
	p[1] = (length & 0xFF);

	return 2;
}

static int hdlc_put_address_type(unsigned char *p, struct hdlc_address_t addr)
{
	switch (addr.len)
	{
	case 1:
		p[0] = (addr.upper << 1) | 1;
		return 1;

	case 2:
		p[0] = (addr.upper << 1);
		p[1] = (addr.lower << 1) | 1;
		return 2;

	case 4:
		p[0] = (addr.upper >> 6) & 0xFE;
		p[1] = (addr.upper << 1);
		p[2] = (addr.lower >> 6) & 0xFE;
		p[3] = (addr.lower << 1) | 1;
		return 4;

	default:
		return -1;
	}
}

static int hdlc_put_control_type(unsigned char *p, struct hdlc_control_t value)
{
	unsigned char b = 0;

	switch (value.code)
	{
	case HDLC_FRAME_I:
		b = (value.nr << 5) | (value.pf << 4) | (value.ns);
		break;

	case HDLC_FRAME_RR:
		b = (value.nr << 5) | (value.pf << 4) | 0x01;
		break;

	case HDLC_FRAME_RNR:
		b = (value.nr << 5) | (value.pf << 4) | 0x05;
		break;

	case HDLC_COMMAND_SNRM:
		b = (value.pf << 4) | 0x83;
		break;

	case HDLC_COMMAND_DISC:
		b = (value.pf << 4) | 0x43;
		break;

	case HDLC_RESPONSE_UA:
		b = (value.pf << 4) | 0x63;
		break;

	case HDLC_RESPONSE_DM:
		b = (value.pf << 4) | 0x0F;
		break;

	case HDLC_RESPONSE_FRMR:
		b = (value.pf << 4) | 0x87;
		break;

	case HDLC_FRAME_UI:
		b = (value.pf << 4) | 0x03;
		break;

	default:
		return -1;
	}

	*p = b;

	return 1;
}

static int hdlc_get_frame_length(unsigned char* p, unsigned int size)
{
	unsigned char high, low;
	uint16_t length;

	if (size < 2)
		return -1;

	high = p[0];
	low  = p[1];

	length = (high & 0x7);
	length <<= 8;
	length |= low;

	return (int)length;
}

static int hdlc_get_format_type(struct hdlc_format_t *value, unsigned char* p, unsigned int size)
{
  unsigned char high;

  if (size < 2)
    return -1;

  high = p[0];

  value->type = high >> 4;
  value->S = (high & (1 << 3)) ? 1 : 0;

  return 2;
}

static int hdlc_get_address_type(struct hdlc_address_t *value, unsigned char* p, unsigned int size)
{
	uint32_t b1, b2, b3, b4;

	value->upper = value->lower = 0;

	if (size < 1)
		return -1;

	b1 = p[0];
	if (b1 & 1)
	{
		value->upper = b1 >> 1;
		value->len = 1;
		return 1;
	}

	if (size < 2)
		return -1;

	b2 = p[1];
	if (b2 & 1)
	{
		value->upper = (b1 >> 1);
		value->lower = (b2 >> 1);
		value->len = 2;
		return 2;
	}

	if (size < 4)
		return -1;

	b3 = p[2];
	b4 = p[3];

	if ((b3 & 1) == 0 && (b4 & 1))
	{
		value->upper = (b1 << 6) | (b2 >> 1);
		value->lower = (b3 << 6) | (b4 >> 1);
		value->len = 4;
		return 4;
	}

	return -1;
}

static int hdlc_check_hcs_fcs(unsigned char* p, unsigned int size, int hcs_index)
{
	uint16_t fcs = FCS16_INIT_VALUE;
	unsigned int pos = 0;

	if (hcs_index > 0)
	{
		for (; pos < hcs_index; pos++)
		{
			fcs = fcs16(fcs, p[pos]);
		}

		if (fcs != FCS16_GOOD_VALUE)
		{
			printf("BAD HCS\n");
			return -2;
		}
	}

	for (; pos < size; pos++)
	{
		fcs = fcs16(fcs, p[pos]);
	}

	if (fcs != FCS16_GOOD_VALUE)
	{
		printf("BAD FCS\n");
		return -1;
	}

	return 0;
}

static int hdlc_get_control_type(struct hdlc_control_t *value, unsigned char* p, unsigned int size)
{
  unsigned char b;
  unsigned char nr, ns;

  if (size < 1)
    return -1;

  memset(value, 0, sizeof(*value));

  b = *p;

  value->pf = (b & (1 << 4)) ? 1 : 0;

  nr = b >> 5;
  ns = (b >> 1) & 0x7;

  if (!(b & 1))
  {
    // I command/response
    value->nr = nr;
    value->ns = ns;
    value->code = HDLC_FRAME_I;
  }
  else
  {
    switch (ns)
    {
      case 0:
        value->nr = nr;
        value->code = HDLC_FRAME_RR;
        break;

      case 2:
        value->nr = nr;
        value->code = HDLC_FRAME_RNR;
        break;

      case 1:
        switch (nr)
        {
          case 4:
            value->code = HDLC_COMMAND_SNRM;
            break;

          case 2:
            value->code = HDLC_COMMAND_DISC;
            break;

          case 3:
            value->code = HDLC_RESPONSE_UA;
            break;

          case 0:
            value->code = HDLC_FRAME_UI;
            break;
        }
        break;

      case 7:
        if (nr == 0)
          value->code = HDLC_RESPONSE_DM;
        break;

      case 3:
        if (nr == 4)
          value->code = HDLC_RESPONSE_FRMR;
        break;
    }
  }

  if (value->code == 0)
  {
    printf("unrecognized frame code\n");
    value->code = HDLC_FRAME_UNKNOWN;
  }

  return 1;
}

void hdlc_frame_print(struct hdlc_frame_t *frame)
{
	printf("frame: [%p]\n", frame);
	printf("\ttype: %u\n", frame->format.type);
	printf("\tS: %u\n", frame->format.S);
	//printf("\tlength: %u\n", frame->format.length);
	printf("\tdest: %u.%u\n", frame->dest_address.upper, frame->dest_address.lower);
	printf("\tsrc: %u.%u\n", frame->src_address.upper, frame->src_address.lower);
	printf("\tctrl.code: %u\n", frame->control.code);
	printf("\tN(R): %u\n", frame->control.nr);
	printf("\tN(S): %u\n", frame->control.ns);
	printf("\tP/F: %u\n", frame->control.pf);

	printf("\tinfo_len: %u\n", frame->info_len);
	if (frame->info_len > 0)
	{
		unsigned int i;
		for (i = 0; i < frame->info_len; i++)
		{
			printf("%02X ", frame->info[i]);
		}
		printf("\n");
	}
}


/*
 * Frame Format:
 * Flag | Frame format | Dest. address | Src. address | Control | HCS | Information | FCS | Flag
 *
 * Flag = 0x7E
 */
int hdlc_frame_parse(struct hdlc_frame_t *frame, struct hdlc_bs_t *bs)
{
	int ret;
	unsigned char *frm = bs->frame;
	unsigned char *p = frm;
	unsigned int length = bs->length;
	unsigned int s = length;
	//uint16_t hcs = FCS16_INIT_VALUE;
	int hcs_index = 0;

	//printf("hdlc_frame_parse length: %u\n", length);

	ret = hdlc_get_frame_length(p, s);
	if (ret < 0)
	{
		printf("fail to get frame length\n");
		return ret;
	}

	frame->length = (unsigned int)ret;

	if (length != frame->length)
	{
		printf("invalid length\n");
		printf("expected frame length: %u input length: %u\n", frame->length, length);
		unsigned int i;
		for (i = 0; i < length; i++)
		{
			printf("%02X ", frm[i]);
		}
		printf("\n");

		return -1;
	}

	ret = hdlc_get_format_type(&frame->format, p, s);
	if (ret < 0)
	{
		printf("bad format type\n");
		return ret;
	}

	if (frame->format.type != HDLC_TYPE_3)
	{
		printf("invalid format type value: %u\n", frame->format.type);
		return -1;
	}

	s -= ret;
	p += ret;

	ret = hdlc_get_address_type(&frame->dest_address, p, s);
	if (ret < 0)
	{
		printf("bad dest address\n");
		return ret;
	}

	s -= ret;
	p += ret;

	ret = hdlc_get_address_type(&frame->src_address, p, s);
	if (ret < 0)
	{
		printf("bad src address\n");
		return ret;
	}

	s -= ret;
	p += ret;

	ret = hdlc_get_control_type(&frame->control, p, s);
	if (ret < 0)
	{
		printf("bad control field\n");
		return ret;
	}

	s -= ret;
	p += ret;

	if (s > 0)
	{
		hcs_index = p - frm + 2;
		if (hcs_index >= length) {
			hcs_index = 0;
			frame->info_len = 0; // No info field
			frame->info     = 0;
		}
		else {
			frame->info_len = s - (2 /* HCS */ + 2 /* FCS */); // Truncate two FCS bytes and two HCS bytes
			frame->info     = p + 2; // Skip two bytes of HCS (Header check sequence)
		}
	}
	else
	{
		frame->info = 0;
		frame->info_len = 0;
	}

	ret = hdlc_check_hcs_fcs(frm, length, hcs_index);
	if (ret < 0)
	{
		printf("FCS HCS check failed\n");
		return ret;
	}

	return ret;
}

int hdlc_frame_encode_hdr(unsigned char *hdr, unsigned int hdr_len, struct hdlc_frame_t *frame)
{
	int ret;
	unsigned char *p, *pformat;
	unsigned int length;
	unsigned int frame_length;

	p = hdr;

	// Save format pointer as length will be known at the end of encoding process
	//
	pformat = p;
	p += 2;
	length = 2;
	hdr_len -= 2;

	if (hdr_len < 4)
		return -1;

	ret = hdlc_put_address_type(p, frame->dest_address);
	if (ret < 0)
	{
		printf("Fail to put dest address\n");
		return ret;
	}
	length += ret;
	p += ret;
	hdr_len -= ret;

	if (hdr_len < 4)
		return -1;

	ret = hdlc_put_address_type(p, frame->src_address);
	if (ret < 0)
	{
		printf("Fail to put src address\n");
		return ret;
	}
	length += ret;
	p += ret;
	hdr_len -= ret;

	if (hdr_len < 1)
		return -1;

	ret = hdlc_put_control_type(p, frame->control);
	if (ret < 0)
		return ret;
	length += ret;
	p += ret;

	frame_length = length + 2;
	if (frame->info_len > 0) {
		frame_length += frame->info_len + 2;
	}

	ret = hdlc_put_format_type(pformat, frame->format, frame_length);
	if (ret < 0)
		return ret;

	return (int)length;
}
