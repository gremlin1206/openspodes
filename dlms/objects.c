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

#include <dlms/profile_generic.h>
#include <dlms/objects.h>

#define OBJECT_ID(a, b, c, d, e, f) (((a) << 31) | ((d) << 24) | ((c) << 16) | (((e) | 0x80) << 8) | (f))

static unsigned int longname2id(struct cosem_longname_t name)
{
	return (name.A << 31) | (name.D << 24) | (name.C << 16) | ((name.E | 0x80) << 8) | name.F;
}

struct cosem_object_t *cosem_find_object_by_index(struct cosem_ctx_t *ctx, unsigned int id)
{
	switch (id)
	{
	case OBJECT_ID(0,0,40,0,0,255):
	case __COUNTER__:
		return cosem_association_get_object(ctx, spodes_association_current);

	case OBJECT_ID(0,0,40,0,1,255):
	case __COUNTER__:
		return cosem_association_get_object(ctx, spodes_association_guest);

	case OBJECT_ID(0,0,40,0,2,255):
	case __COUNTER__:
		return cosem_association_get_object(ctx, spodes_association_reader);

	case OBJECT_ID(0,0,40,0,3,255):
	case __COUNTER__:
		return cosem_association_get_object(ctx, spodes_association_configurator);

	case OBJECT_ID(1,0,99,1,0,255):
	case __COUNTER__:
		return profile_generic_get_object(ctx);
	}

	return 0;
}

unsigned int cosem_objects_count(void)
{
	return __COUNTER__;
}

struct cosem_object_t *cosem_find_object_by_name(struct cosem_ctx_t *ctx, struct cosem_longname_t name)
{
	return cosem_find_object_by_index(ctx, longname2id(name));
}
