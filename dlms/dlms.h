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

#ifndef OPENSPODES_DLMS_H
#define OPENSPODES_DLMS_H

#define DLMS_MAX_PDU_SIZE 1024

struct dlms_pdu_t
{
	unsigned char data[DLMS_MAX_PDU_SIZE];
	unsigned int length;
};

struct dlms_ctx_t
{
	struct dlms_pdu_t pdu;
};

int dlms_init(struct dlms_ctx_t *ctx);
int dlms_input(struct dlms_ctx_t *ctx, unsigned char *p, unsigned int len, int more);

#endif /* OPENSPODES_DLMS_H */
