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

#include <cosem/association.h>
#include <cosem/asn1.h>

#include <dlms/class.h>
#include <dlms/objects.h>
#include <dlms/object.h>

static int cosem_encode_logical_name(struct cosem_longname_t longname, struct cosem_pdu_t *output)
{
	unsigned char *p;

	p = cosem_pdu_put_cosem_data(output, 8);
	if (!p)
		return -1;

	p[0] = 0x09;
	p[1] = 0x06;

	p[2] = longname.A;
	p[3] = 0;
	p[4] = longname.C;
	p[5] = longname.D;
	p[6] = longname.E;
	p[7] = longname.F;

	return 8;
}

static int cosem_object_get_request_normal(struct cosem_ctx_t *ctx,
		                           struct get_request_normal_t *get_request_normal, struct get_response_t *response,
					   struct cosem_pdu_t *output)
{
	int ret;
	const struct cosem_class_t *cosem_class;
	struct cosem_object_t *object;

	response->type = get_response_normal_type;
	response->get_response_normal.result = access_result_success;
	response->get_response_normal.invoke_id_and_priority = get_request_normal->invoke_id_and_priority;

	object = cosem_find_object_by_name(ctx, get_request_normal->cosem_attribute_descriptor.instance_id);
	if (!object) {
		printf("cosem_process_get_request: object not found\n");
		response->get_response_normal.result = access_result_object_undefined;
		return 0;
	}

	cosem_class = object->cosem_class;

	if (get_request_normal->cosem_attribute_descriptor.class_id != cosem_class->class_id) {
		printf("cosem_object_get_request: object class inconsistent\n");
		response->get_response_normal.result = access_result_object_class_inconsistent;
		return 0;
	}

	if (get_request_normal->cosem_attribute_descriptor.attribute_id == 1) {
		ret = cosem_encode_logical_name(get_request_normal->cosem_attribute_descriptor.instance_id, output);
		if (ret < 0)
			return ret;

		return 0;
	}

	if (cosem_class->get_normal == 0) {
		printf("cosem_object_get_request: method get not implemented\n");
		response->get_response_normal.result = access_result_other_reason;
		return 0;
	}

	return cosem_class->get_normal(ctx, object, get_request_normal, response, output);
}

static int cosem_object_get_request_next(struct cosem_ctx_t *ctx,
		                           struct get_request_next_t *request, struct get_response_t *response,
					   struct cosem_pdu_t *output)
{
	struct cosem_object_t *object;
	const struct cosem_class_t *cosem_class;

	printf("cosem_object_get_request_next\n");

	response->type = get_response_with_datablock;
	response->get_response_with_datablock.result = access_result_success;
	response->get_response_with_datablock.invoke_id_and_priority = request->invoke_id_and_priority;

	object = cosem_find_object_by_name(ctx, ctx->block_transfer.cosem_attribute_descriptor.instance_id);
	if (!object) {
		printf("cosem_process_get_request: object not found\n");
		response->get_response_normal.result = access_result_object_unavailable;
		return 0;
	}

	cosem_class = object->cosem_class;

	if (cosem_class->get_next == 0) {
		response->get_response_with_datablock.result = access_result_other_reason;
		return 0;
	}

	return cosem_class->get_next(ctx, object, request, response, output);
}

int cosem_object_get_request(struct cosem_ctx_t *ctx,
		               struct get_request_t *request, struct get_response_t *response,
			       struct cosem_pdu_t *output)
{
	int ret;

	if (!ctx->association.authenticated) {
		printf("not authenticated\n");
		return -1;
	}

	switch (request->type)
	{
	case get_request_normal_type:
		ret = cosem_object_get_request_normal(ctx, &request->get_request_normal, response, output);
		break;

	case get_request_next_type:
		ret = cosem_object_get_request_next(ctx, &request->get_request_next, response, output);
		break;

	case get_request_with_list_type:
		ret = -1;
		break;

	default:
		ret = -1;
		break;
	}

	return ret;
}

static int cosem_object_set_request_normal(struct cosem_ctx_t *ctx,
		                           struct set_request_normal_t *set_request_normal, struct set_response_t *response,
					   struct cosem_pdu_t *pdu, struct cosem_pdu_t *output)
{
	const struct cosem_class_t *cosem_class;
	struct cosem_object_t *object;

	response->type = set_response_normal;

	object = cosem_find_object_by_name(ctx, set_request_normal->cosem_attribute_descriptor.instance_id);
	if (!object) {
		printf("cosem_process_get_request: object not found\n");
		response->set_response_normal.result = access_result_object_undefined;
		return 0;
	}

	cosem_class = object->cosem_class;

	if (set_request_normal->cosem_attribute_descriptor.class_id != cosem_class->class_id) {
		printf("cosem_object_get_request: object class inconsistent\n");
		response->set_response_normal.result = access_result_object_class_inconsistent;
		return 0;
	}

	if (cosem_class->set_normal == 0) {
		printf("cosem_object_get_request: method get not implemented\n");
		response->set_response_normal.result = access_result_other_reason;
		return 0;
	}

	return cosem_class->set_normal(ctx, object, set_request_normal, response, pdu, output);
}

int cosem_object_set_request(struct cosem_ctx_t *ctx,
                             struct set_request_t *request, struct set_response_t *response,
		             struct cosem_pdu_t *pdu, struct cosem_pdu_t *output)
{
	int ret;

	switch (request->type)
	{
	case set_request_normal:
		ret = cosem_object_set_request_normal(ctx, &request->set_request_normal, response, pdu, output);
		break;

	default:
		ret = -1;
		break;
	}

	return ret;
}

static int cosem_object_action_request_normal(struct cosem_ctx_t *ctx,
		                              struct action_request_normal_t *action_request_normal, struct action_response_t *response,
					      struct cosem_pdu_t *pdu, struct cosem_pdu_t *output)
{
	int ret;
	const struct cosem_class_t *cosem_class;
	struct cosem_object_t *object;
	unsigned char has_data;

	response->type = action_response_normal_type;

	object = cosem_find_object_by_name(ctx, action_request_normal->cosem_method_descriptor.instance_id);
	if (!object) {
		printf("cosem_process_get_request: object not found\n");
		response->action_response_normal.result = action_result_object_undefined;
		return 0;
	}

	cosem_class = object->cosem_class;

	if (action_request_normal->cosem_method_descriptor.class_id != cosem_class->class_id) {
		printf("cosem_object_get_request: object class inconsistent\n");
		response->action_response_normal.result = action_result_object_class_inconsistent;
		return 0;
	}

	if (cosem_class->action_normal == 0) {
		printf("cosem_object_get_request: method get not implemented\n");
		response->action_response_normal.result = action_result_other_reason;
		return 0;
	}

	ret = asn_get_uint8(&has_data, pdu);
	if (ret < 0)
		return ret;

	return cosem_class->action_normal(ctx, object, action_request_normal, response, has_data ? pdu : 0, output);
}

int cosem_object_action(struct cosem_ctx_t *ctx,
	                struct action_request_t *request, struct action_response_t *response,
			struct cosem_pdu_t *pdu, struct cosem_pdu_t *output)
{
	int ret;

	switch (request->type)
	{
	case action_request_normal_type:
		ret = cosem_object_action_request_normal(ctx, &request->action_request_normal, response, pdu, output);
		break;

	default:
		ret = -1;
		break;
	}

	return ret;
}
