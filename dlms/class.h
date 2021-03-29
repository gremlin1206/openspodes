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

#ifndef COSEM_CLASS_H_
#define COSEM_CLASS_H_

#include <cosem/cosem.h>
#include <dlms/data.h>

struct cosem_object_t;

typedef int (*cosem_get_normal_t)(struct cosem_ctx_t *ctx, struct cosem_object_t *object,
                                  struct get_request_normal_t *get_request_normal, struct get_response_t *response,
		                  struct cosem_pdu_t *output);

typedef int (*cosem_get_next_t)(struct cosem_ctx_t *ctx, struct cosem_object_t *object,
                                struct get_request_next_t *request, struct get_response_t *response,
		                struct cosem_pdu_t *output);

typedef int (*cosem_set_normal_t)(struct cosem_ctx_t *ctx, struct cosem_object_t *object,
		                  struct set_request_normal_t *set_request_normal, struct set_response_t *response,
				  struct cosem_pdu_t *pdu, struct cosem_pdu_t *output);

typedef int (*cosem_action_normal_t)(struct cosem_ctx_t *ctx, struct cosem_object_t *object,
                                     struct action_request_normal_t *request, struct action_response_t *response,
				     struct cosem_pdu_t *pdu, struct cosem_pdu_t *output);

typedef int (*cosem_get_object_access_rights_t)(struct cosem_ctx_t *ctx, struct cosem_object_t *object, struct cosem_pdu_t *output);

typedef int (*cosem_get_attributes_t)(struct cosem_ctx_t *ctx, struct cosem_object_t *object, struct cosem_pdu_t *output);
typedef int (*cosem_get_methods_t)(struct cosem_ctx_t *ctx, struct cosem_object_t *object, struct cosem_pdu_t *output);

struct cosem_class_t
{
	unsigned short class_id;
	unsigned char  version;

	cosem_get_normal_t      get_normal;
	cosem_get_next_t        get_next;
	cosem_set_normal_t      set_normal;
	cosem_action_normal_t   action_normal;

	cosem_get_attributes_t  get_attributes;
	cosem_get_methods_t     get_methods;
};

enum method_access_mode_t
{
	method_no_access,
	method_access,
	method_authenticated_access,
};

enum attribute_access_mode_t
{
	attribute_no_access,
	attribute_read_only,
	attribute_write_only,
	attribute_read_and_write,
	attribute_authenticated_read_only,
	attribute_authenticated_write_only,
	attribute_authenticated_read_and_write,
};

struct attribute_access_item_t
{
	dlms_integer_t attribute_id;
	dlms_enum_t access_mode;
};

struct method_access_item_t
{
	dlms_integer_t method_id;
	dlms_enum_t access_mode;
};

int encode_attribute_access_item(dlms_integer_t attribute_id, dlms_enum_t access_mode,
		                 dlms_integer_t *access_selectors, unsigned int access_selectors_count,
				 struct cosem_pdu_t *output);
int encode_method_access_item(dlms_integer_t method_id, dlms_enum_t access_mode, struct cosem_pdu_t *output);

#endif /* COSEM_CLASS_H_ */
