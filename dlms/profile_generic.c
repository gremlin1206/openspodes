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

#include <dlms/class_profile_generic.h>
#include <dlms/profile_generic.h>

static struct cosem_profile_generic_object_t cosem_object_current_association = {
		.base = {
				.cosem_class  = (struct cosem_class_t*)&class_profile_generic,
				.logical_name = { .A = 1, /*.B = 0*/ .C = 99, .D = 1, .E = 0, .F = 255 },
		},
};

struct cosem_object_t* profile_generic_get_object(struct cosem_ctx_t *ctx)
{
	if (ctx->association.spodes_access_level == spodes_access_level_public)
		return 0;

	return &cosem_object_current_association.base;
}
