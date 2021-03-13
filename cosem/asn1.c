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

#include "asn1.h"

int asn_get_length(unsigned int *out, const unsigned char **buffer, unsigned int *len)
{
	unsigned int length = *len;
	unsigned int result;
	unsigned char bytes;
	const unsigned char *p = *buffer;
	unsigned char byte;

	if (length < 1)
		return -1;

	byte = p[0]; length--; p++;

	if ((byte & 0x80) == 0) {
		result = byte;
	}
	else {
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

	printf("result %u, length: %u\n", result, length);

	if (result > length)
		return -1;

	*out = result;
	*buffer = p;
	*len = length;

	return 0;
}

int asn_get_uint16(unsigned short *out, const unsigned char **buffer, unsigned int *length)
{
	unsigned short result;
	const unsigned char *p = *buffer;

	if (*length < 2)
		return -1;

	result = p[0];
	result <<= 8;
	result |= p[1];

	*out = result;

	*length -= 2;
	*buffer += 2;

	return 0;
}

int asn_put_uint16(unsigned char *buffer, unsigned short value)
{
	buffer[0] = (unsigned char)(value >> 8);
	buffer[1] = (unsigned char)(value & 0xFF);

	return 2;
}

int asn_get_uint8(unsigned char *out, const unsigned char **buffer, unsigned int *length)
{
	const unsigned char *p = *buffer;

	if (*length < 1)
		return -1;

	*out = p[0];

	*length -= 1;
	*buffer += 1;

	return 0;
}

int asn_get_buffer(void *out, unsigned int bytes, const unsigned char **buffer, unsigned int *length)
{
	if (bytes > *length)
		return -1;

	memcpy(out, *buffer, bytes); *buffer += bytes; *length -= bytes;

	return 0;
}
