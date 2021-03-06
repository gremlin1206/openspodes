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

#include <dlms/dlms.h>

int dlms_init(struct dlms_ctx_t *ctx)
{
	int ret;

	ret = cosem_init(&ctx->cosem);
	if (ret < 0)
		return ret;

	return 0;
}

void dlms_close_association(struct dlms_ctx_t *ctx)
{
	cosem_close_association(&ctx->cosem);
}

int dlms_input(struct dlms_ctx_t *ctx, enum spodes_access_level_t access_level,
	       struct cosem_pdu_t *input_pdu, struct cosem_pdu_t *output_pdu)
{
	return cosem_input(&ctx->cosem, access_level, input_pdu, output_pdu);
}
