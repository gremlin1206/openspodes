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

#include <cosem/cosem.h>

struct cosem_class_t;

struct cosem_object_t
{
	const struct cosem_class_t *cosem_class;
	void *data;
};

typedef int (*cosem_get_normal_t)(struct cosem_ctx_t *ctx, struct cosem_object_t *object,
                                  struct get_request_normal_t *get_request_normal, struct get_response_t *response,
		                  struct cosem_pdu_t *output);

/*
typedef int (*cosem_set_attribute_t)(struct cosem_ctx_t *ctx, struct cosem_object_t *object,
		                     struct set_request_t *request, struct cosem_pdu_t *pdu,
				     struct cosem_pdu_t *output);
*/

typedef int (*cosem_action_normal_t)(struct cosem_ctx_t *ctx, struct cosem_object_t *object,
                                     struct action_request_normal_t *request, struct action_response_t *response,
				     struct cosem_pdu_t *pdu, struct cosem_pdu_t *output);

typedef int (*coset_get_object_metadata_t)(struct cosem_object_t *obj, struct cosem_pdu_t *output);

struct cosem_class_t
{
	int class_id;

	cosem_get_normal_t      get_normal;
//	cosem_set_attribute_t   set_normal;
	cosem_action_normal_t   action_normal;

	coset_get_object_metadata_t get_metadata;
};

int cosem_object_get_request(struct cosem_ctx_t *ctx,
	                       struct get_request_t *request, struct get_response_t *response,
		               struct cosem_pdu_t *output);

int cosem_object_set_attribute(struct cosem_ctx_t *ctx,
	                       struct set_request_t *request, struct cosem_pdu_t *pdu,
		               struct cosem_pdu_t *output);

int cosem_object_action(struct cosem_ctx_t *ctx,
                        struct action_request_t *request, struct action_response_t *response,
			struct cosem_pdu_t *pdu, struct cosem_pdu_t *output);

#endif /* COSEM_OBJECTS_H_ */
