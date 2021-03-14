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

#ifndef COSEM_OBJECTS_H_
#define COSEM_OBJECTS_H_

#include "types.h"

struct cosem_object_t;
struct cosem_ctx_t;
struct get_request_t;
struct get_response_t;

typedef int (*cosem_get_attribute_t)(struct get_request_t *request, struct get_response_t *response);
//typedef int cosem_set_attribute(int attribute);
//typedef int cosem_call_method(int method);

struct cosem_class_t
{
	int id;
	cosem_get_attribute_t get;
};

struct cosem_object_t
{
	const struct cosem_class_t *cosem_class;
};

struct get_request_t
{
	enum get_request_type_t type;
	struct invoke_id_and_priority_t invoke_id_and_priority;
	union {
		struct get_request_normal_t get_request_normal;
	};

	struct cosem_ctx_t *ctx;
	struct cosem_object_t *object;
};

struct get_response_t
{
	enum data_access_result_t data_access_result;
	unsigned char *buffer;
	unsigned int length;
};

struct cosem_object_t *cosem_find_object_by_name(struct cosem_longname_t name);

int cosem_object_get_attribute(struct get_request_t *request, struct get_response_t *response);

#endif /* COSEM_OBJECTS_H_ */
