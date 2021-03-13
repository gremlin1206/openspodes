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

#include <string.h>

#include "pdu.h"

void cosem_pdu_init(struct cosem_pdu_t *pdu, unsigned int header)
{
	memset(pdu, 0, sizeof(*pdu));
	pdu->header = header;
}

void cosem_pdu_reset(struct cosem_pdu_t *pdu)
{
	pdu->length = 0;
}

int cosem_pdu_append_buffer(struct cosem_pdu_t *pdu, const void *buffer, unsigned int length)
{
	unsigned int pdu_length = pdu->length;
	unsigned int new_pdu_length = pdu_length + length;

	if (new_pdu_length > sizeof(pdu->data))
		return -1;

	memcpy(pdu->data + pdu_length, buffer, length);
	pdu->length = new_pdu_length;

	return (int)new_pdu_length;
}

unsigned char* cosem_pdu_header(struct cosem_pdu_t *pdu)
{
	return pdu->data;
}

unsigned char* cosem_pdu_payload(struct cosem_pdu_t *pdu)
{
	return pdu->data + pdu->header;
}

unsigned int cosem_pdu_payload_length(struct cosem_pdu_t *pdu)
{
	unsigned int length = pdu->length;
	unsigned int header = pdu->header;

	return (length > header) ? length - header : 0;
}

void cosem_pdu_set_payload_length(struct cosem_pdu_t *pdu, unsigned int length)
{
	pdu->length = length + pdu->header;
}
