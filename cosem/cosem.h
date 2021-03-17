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

#ifndef OPENSPODES_COSEM_H
#define OPENSPODES_COSEM_H

#include "types.h"
#include "pdu.h"
#include "association.h"

struct cosem_ctx_t
{
	struct cosem_key_t hls_auth_key;
	struct cosem_key_t lls_auth_key;
	struct cosem_association_t association;
};

int cosem_input(struct cosem_ctx_t *ctx, struct cosem_pdu_t *pdu);
void cosem_close_association(struct cosem_ctx_t *ctx);

int cosem_init(struct cosem_ctx_t *ctx);

#endif /* OPENSPODES_COSEM_H */
