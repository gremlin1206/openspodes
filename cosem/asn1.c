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

#include <cosem/asn1.h>
#include <cosem/pdu.h>

int asn_get_length(unsigned int *out, struct cosem_pdu_t *pdu)
{
	int ret;
	unsigned int result = 0;
	int bytes;
	int byte;

	byte = cosem_pdu_get_byte(pdu);
	if (byte < 0)
		return byte;

	if ((byte & 0x80) == 0) {
		result = (unsigned int)byte;
		bytes = 1;
	}
	else {
		bytes = byte & ~0x80;

		switch (bytes) // bytes number of length value
		{
		case 1:
			ret = asn_get_uint8((unsigned char*)&result, pdu);
			break;

		case 2:
			ret = asn_get_uint16((unsigned short*)&result, pdu);
			break;

		case 4:
			ret = asn_get_uint32(&result, pdu);
			break;

		default:
			ret = -1;
			break;
		}

		if (ret < 0)
			return ret;

		bytes += 1;
	}

	if (result > pdu->length)
		return -1;

	return bytes;
}

int asn_put_length(unsigned int length, struct cosem_pdu_t *pdu)
{
	int ret;

	if (length < 128) {
		return asn_put_uint8((unsigned char)length, pdu);
	}
	else if (length < 65536) {
		ret = asn_put_uint16((unsigned short)length, pdu);
		if (ret < 0)
			return ret;

		ret = asn_put_uint8(0x82, pdu);
		if (ret < 0)
			return ret;

		return 3;
	}
	else {
		ret = asn_put_uint32((unsigned short)length, pdu);
		if (ret < 0)
			return ret;

		ret = asn_put_uint8(0x84, pdu);
		if (ret < 0)
			return ret;

		return 5;
	}
}

int asn_get_uint32(unsigned int *out, struct cosem_pdu_t *pdu)
{
	unsigned int result;
	unsigned char *p;

	p = cosem_pdu_get_data(pdu, 4);
	if (!p)
		return -1;

	result = p[0];
	result <<= 8;
	result |= p[1];
	result <<= 8;
	result |= p[2];
	result <<= 8;
	result |= p[3];

	*out = result;

	return 4;
}

int asn_get_uint16(unsigned short *out, struct cosem_pdu_t *pdu)
{
	unsigned short result;
	unsigned char *p;

	p = cosem_pdu_get_data(pdu, 2);
	if (!p)
		return -1;

	result = p[0];
	result <<= 8;
	result |= p[1];

	*out = result;

	return 2;
}

int asn_get_uint8(unsigned char *out, struct cosem_pdu_t *pdu)
{
	int byte;

	byte = cosem_pdu_get_byte(pdu);
	if (byte < 0)
		return byte;

	*out = (unsigned char)byte;

	return 1;
}

int asn_put_uint8(unsigned char value, struct cosem_pdu_t *pdu)
{
	unsigned char *p;

	p = cosem_pdu_put_cosem_data(pdu, 1);
	if (!p)
		return -1;

	p[0] = value;

	return 1;
}

int asn_put_uint16(unsigned short value, struct cosem_pdu_t *pdu)
{
	unsigned char *p;

	p = cosem_pdu_put_cosem_data(pdu, 2);
	if (!p)
		return -1;

	p[0] = (unsigned char)(value >> 8);
	p[1] = (unsigned char)(value & 0xFF);

	return 2;
}

int asn_put_uint32(unsigned int value, struct cosem_pdu_t *pdu)
{
	unsigned char *p;

	p = cosem_pdu_put_cosem_data(pdu, 4);
	if (!p)
		return -1;

	p[3] = (unsigned char)(value & 0xFF); value >>= 8;
	p[2] = (unsigned char)(value & 0xFF); value >>= 8;
	p[1] = (unsigned char)(value & 0xFF); value >>= 8;
	p[0] = (unsigned char)(value & 0xFF);

	return 4;
}

int asn_get_tagged_data(struct asn1_tagged_data_t *out, struct cosem_pdu_t *pdu)
{
	int ret;
	unsigned int length;
	unsigned int bytes;

	ret = asn_get_uint8(&out->tag, pdu);
	if (ret < 0)
		return ret;

	bytes = 1;

	ret = asn_get_length(&length, pdu);
	if (ret < 0)
		return ret;

	bytes += (unsigned int)ret;
	bytes += length;

	ret = cosem_pdu_get_sub_pdu(&out->pdu, pdu, length);
	if (ret < 0)
		return ret;

	return (int)bytes;
}

int asn_put_tagged_data(unsigned char tag, unsigned int length, struct cosem_pdu_t *pdu)
{
	int ret;
	int bytes = (int)length;

	ret = asn_put_length(length, pdu);
	if (ret < 0)
		return ret;

	bytes += ret;

	ret = asn_put_uint8(tag, pdu);
	if (ret < 0)
		return ret;

	bytes += ret;

	return bytes;
}
