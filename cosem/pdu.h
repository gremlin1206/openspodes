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

#ifndef COSEM_PDU_H_
#define COSEM_PDU_H_

#include <hdlc/frame.h>

struct cosem_pdu_t
{
	int reversed;
	unsigned char *head;
	unsigned int length;
	unsigned int length_limit;

	unsigned int  max_length;
	unsigned char *data;
};

void cosem_pdu_init(struct cosem_pdu_t *pdu, unsigned int length_limit, int reversed,
		    void *buffer, unsigned int length);

void cosem_pdu_clear(struct cosem_pdu_t *pdu);
void cosem_pdu_reset(struct cosem_pdu_t *pdu);

unsigned char* cosem_pdu_put_cosem_data(struct cosem_pdu_t *pdu, unsigned int length);
unsigned char* cosem_pdu_put_data(struct cosem_pdu_t *pdu, unsigned int length);
unsigned char* cosem_pdu_get_data(struct cosem_pdu_t *pdu, unsigned int length);

int cosem_pdu_get_byte(struct cosem_pdu_t *pdu);

int cosem_pdu_get_sub_pdu(struct cosem_pdu_t *sub_pdu, struct cosem_pdu_t *pdu, unsigned int length);

#endif /* COSEM_PDU_H_ */
