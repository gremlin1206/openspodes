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

#include <cosem/pdu.h>

static unsigned char* cosem_pdu_put_(struct cosem_pdu_t *pdu, unsigned int length, unsigned int limit)
{
	unsigned char *p;
	int reversed = pdu->reversed;
	unsigned int pdu_length = pdu->length;
	unsigned int new_pdu_length = pdu_length + length;

	if (new_pdu_length > limit)
		return 0;

	p = pdu->head;

	if (reversed) {
		p -= length;
		pdu->head = p;
	}
	else {
		pdu->head = p + length;
	}

	pdu->length = new_pdu_length;

	return p;
}

void cosem_pdu_init(struct cosem_pdu_t *pdu, unsigned int length_limit, int reversed,
		    void *buffer, unsigned int length)
{
	pdu->data         = (unsigned char*)buffer;
	pdu->max_length   = length;
	pdu->length_limit = length_limit;
	pdu->reversed     = reversed;

	cosem_pdu_reset(pdu);
}

void cosem_pdu_reset(struct cosem_pdu_t *pdu)
{
	if (pdu->reversed)
		pdu->head = pdu->data + pdu->max_length;
	else
		pdu->head = pdu->data;
	pdu->length = 0;
}

unsigned char* cosem_pdu_put_cosem_data(struct cosem_pdu_t *pdu, unsigned int length)
{
	return cosem_pdu_put_(pdu, length, pdu->length_limit);
}

unsigned char* cosem_pdu_put_data(struct cosem_pdu_t *pdu, unsigned int length)
{
	return cosem_pdu_put_(pdu, length, pdu->max_length);
}

unsigned char* cosem_pdu_get_data(struct cosem_pdu_t *pdu, void *buffer, unsigned int length)
{
	if (length > pdu->length)
		return -1;

	pdu->head += length;
	pdu->length -= length;

	return pdu->head;
}

int cosem_pdu_get_byte(struct cosem_pdu_t *pdu)
{
	unsigned char byte;
	unsigned int length = pdu->length;

	if (length == 0)
		return -1;

	byte = pdu->head[0];
	pdu->head++;
	pdu->length = length - 1;

	return byte;
}

int cosem_pdu_get_sub_pdu(struct cosem_pdu_t *sub_pdu, struct cosem_pdu_t *pdu, unsigned int length)
{
	unsigned char *p;

	p = cosem_pdu_get_data(pdu, length);
	if (!p)
		return -1;

	cosem_pdu_init(sub_pdu, length, 0, p, length);
	sub_pdu->length = length;

	return (int)length;
}
