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

#include <string.h>
#include <stdio.h>

#include <cosem/cosem.h>
#include <cosem/asn1.h>
#include <cosem/types.h>

#include <dlms/data.h>
#include <dlms/objects.h>
#include <dlms/class_association_ln.h>

static int encode_object_access_rights(struct cosem_ctx_t *ctx, struct cosem_object_t *object,
		const struct cosem_class_t *cosem_class, struct cosem_pdu_t *output)
{
	int ret;
	int count;

	/* Encode Methods */
	{
		if (cosem_class->get_methods) {
			ret = cosem_class->get_methods(ctx, object, output);
			if (ret < 0) {
				printf("encode_object_access_rights error 1\n");
				return ret;
			}
			count = ret;
		}
		else
			count = 0;

		/*
		 * method_access_descriptor
		 */
		ret = dlms_put_array(count, output);
		if (ret < 0) {
			printf("encode_object_access_rights error 2\n");
			return ret;
		}
	}

	/* Encode Attributes */
	{
		/*
		 * All objects have 'logical-name' attribute with 'attribute-id = 1'
		 */
		ret = encode_attribute_access_item(1, attribute_read_only, 0, 0, output);
		if (ret < 0) {
			printf("encode_object_access_rights error 3\n");
			return ret;
		}

		if (cosem_class->get_attributes) {
			ret = cosem_class->get_attributes(ctx, object, output);
			if (ret < 0) {
				printf("pdu length: %u %u\n", output->length, output->length_limit);
				printf("encode_object_access_rights error 4\n");
				return ret;
			}
			count = ret;
		}
		else
			count = 0;

		/*
		 * method_access_descriptor
		 */
		ret = dlms_put_array(count + 1, output);
		if (ret < 0) {
			printf("encode_object_access_rights error 5\n");
			return ret;
		}
	}

	ret = dlms_put_structure(2, output);
	if (ret < 0) {
		printf("encode_object_access_rights error 6\n");
		return ret;
	}

	return 0;
}

static int encode_object_list_item(struct cosem_ctx_t *ctx, struct cosem_object_t *object, struct cosem_pdu_t *output)
{
	int ret;
	const struct cosem_class_t *cosem_class = object->cosem_class;

	ret = encode_object_access_rights(ctx, object, cosem_class, output);
	if (ret < 0) {
		printf("encode_object_list_item error 1\n");
		return ret;
	}

	ret = dlms_put_logical_name(object->logical_name, output);
	if (ret < 0) {
		printf("encode_object_list_item error 2\n");
		return ret;
	}

	ret = dlms_put_unsigned(cosem_class->version, output);
	if (ret < 0) {
		printf("encode_object_list_item error 3\n");
		return ret;
	}

	ret = dlms_put_long_unsigned(cosem_class->class_id, output);
	if (ret < 0) {
		printf("encode_object_list_item error 4\n");
		return ret;
	}

	ret = dlms_put_structure(4, output);
	if (ret < 0) {
		printf("encode_object_list_item error 5\n");
		return ret;
	}

	return 0;
}

static int get_object_list(
		struct cosem_ctx_t *ctx,
		struct get_request_normal_t *get_request_normal,
		struct get_response_t *response,
		struct cosem_pdu_t *output
		)
{
	int ret;
	unsigned int i;
	unsigned int count = cosem_objects_count();
	struct cosem_object_t *object;
	unsigned int objects_encoded;
	int use_block_transfer = 0;

	objects_encoded = 0;
	for (i = 0; i < count; i++) {
		printf("object #%u pdu: %u\n", i, output->length);
		object = cosem_find_object_by_id(ctx, i);
		if (object && object->spodes_access_level <= ctx->association.spodes_access_level) {
			ret = encode_object_list_item(ctx, object, output);
			if (ret < 0)
				return ret;
			objects_encoded++;
		}
	}

	printf("get_object_list: total objects count %u, encoded %u\n", count, objects_encoded);

	if (use_block_transfer) {
		ctx->block_transfer.cosem_attribute_descriptor = get_request_normal->cosem_attribute_descriptor;

		ret = dlms_put_array(count, output);
				if (ret < 0)
					return ret;

		response->type = get_response_with_datablock;
		response->get_response_with_datablock.result = access_result_success;
		response->get_response_with_datablock.last_block = 1;
		response->get_response_with_datablock.block_number = 17;
	}
	else {
		ret = dlms_put_array(objects_encoded, output);
		if (ret < 0)
			return ret;

		response->get_response_normal.result = access_result_success;
	}

	return 0;
}

static int action_reply_to_hls_authentication(struct cosem_ctx_t *ctx, struct cosem_object_t *object,
               				      struct action_request_normal_t *action_request_normal, struct action_response_t *response,
					      struct cosem_pdu_t *pdu, struct cosem_pdu_t *output)
{
	int ret;
	unsigned char fCtoS_output[16];
	unsigned int fStoC_input_length;
	unsigned char *fStoC_input;
	struct cosem_association_ln_object_t *association_object = (struct cosem_association_ln_object_t*)object;

	if (!pdu) {
		response->action_response_normal.result = action_result_other_reason;
		return 0;
	}

	ret = asn_get_octet_string(&fStoC_input, &fStoC_input_length, pdu);
	if (ret < 0) {
		response->action_response_normal.result = action_result_other_reason;
		return 0;
	}

	if (fStoC_input_length != 16) {
		printf("reply_to_hls_authentication: wrong request length\n");
		response->action_response_normal.result = action_result_other_reason;
		return 0;
	}

	ret = cosem_association_high_level_security_authentication_stage2(ctx, association_object->association, fStoC_input, fCtoS_output);
	if (ret < 0) {
		printf("reply_to_hls_authentication: authentication failed/n");
		response->action_response_normal.result = action_result_other_reason;
		return 0;
	}

	ret = asn_put_octet_string(fCtoS_output, sizeof(fCtoS_output), output);
	if (ret < 0)
		return ret;

	/*
	 * Set get-data-result type to [0] Data
	 */
	ret = asn_put_uint8(0x00, output);
	if (ret < 0)
		return ret;

	/*
	 * Insert data presence indication byte. Must be equal to 0x01
	 */
	ret = asn_put_uint8(0x01, output);
	if (ret < 0)
		return ret;

	response->action_response_normal.result = action_result_success;

	return 0;
}

static int get_attributes(struct cosem_ctx_t *ctx, struct cosem_object_t *object, struct cosem_pdu_t *output)
{
	int ret;

	ret = encode_attribute_access_item(association_ln_object_list, attribute_read_only, 0, 0, output);
	if (ret < 0)
		return ret;

	return 1;
}

static int get_normal(struct cosem_ctx_t *ctx, struct cosem_object_t *object,
                      struct get_request_normal_t *get_request_normal, struct get_response_t *response,
                      struct cosem_pdu_t *output)
{
	printf("class association_ln: get-normal\n");

	switch (get_request_normal->cosem_attribute_descriptor.attribute_id)
	{
	case association_ln_object_list:
		return get_object_list(ctx, get_request_normal, response, output);

	default:
		response->get_response_normal.result = access_result_other_reason;
		break;
	}

	// Attribute not found
	return 0;
}

static int get_next(struct cosem_ctx_t *ctx, struct cosem_object_t *object,
                    struct get_request_next_t *request, struct get_response_t *response,
                    struct cosem_pdu_t *output)
{
	printf("class association_ln: get-next\n");

	response->get_response_with_datablock.last_block = 1;
	response->get_response_with_datablock.block_number = 1;

	dlms_put_null(output);

	return 0;
}

static int action_normal(struct cosem_ctx_t *ctx, struct cosem_object_t *object,
                         struct action_request_normal_t *action_request_normal, struct action_response_t *response,
		         struct cosem_pdu_t *pdu, struct cosem_pdu_t *output)
{
	printf("class association_ln: action\n");

	switch (action_request_normal->cosem_method_descriptor.method_id)
	{
	case association_ln_reply_to_hls_authentication:
		return action_reply_to_hls_authentication(ctx, object, action_request_normal, response, pdu, output);

	default:
		response->action_response_normal.result = action_result_other_reason;
		break;
	}

	return 0;
}

const struct cosem_class_t class_association_ln = {
	.class_id = 15,
	.version  = 1,

	.get_normal     = get_normal,
	.get_next       = get_next,
	.action_normal  = action_normal,
	.get_attributes = get_attributes,
};
