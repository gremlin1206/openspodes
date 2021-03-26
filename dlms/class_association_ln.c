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
#include <dlms/class_association_ln.h>

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

static int encode_attribute_access_item(const struct attribute_access_item_t *attribute_access_item, struct cosem_pdu_t *output)
{
	int ret;

	/*
	 * access_selectors [null]
	 */
	ret = dlms_put_null(output);
	if (ret < 0)
		return ret;

	/*
	 * access_mode
	 */
	ret = dlms_put_enum(attribute_access_item->access_mode, output);
	if (ret < 0)
		return ret;

	/*
	 * attribute_id
	 */
	ret = dlms_put_integer(attribute_access_item->attribute_id, output);
	if (ret < 0)
		return ret;

	ret = dlms_put_structure(3, output);
	if (ret < 0)
		return ret;

	return 0;
}

#if 0
static int encode_method_access_item(const struct method_access_item_t *method_access_item, struct cosem_pdu_t *output)
{
	int ret;

	ret = dlms_put_enum(method_access_item->access_mode, output);
	if (ret < 0)
		return ret;

	ret = dlms_put_integer((unsigned char)method_access_item->method_id, output);
	if (ret < 0)
		return ret;

	ret = dlms_put_structure(2, output);
	if (ret < 0)
		return ret;

	return 0;
}
#endif

static int encode_access_rights(struct cosem_pdu_t *output)
{
	int ret;
	struct attribute_access_item_t attribute_access_item = {
			.attribute_id = 1,
			.access_mode = attribute_read_only,
	};

	/*
	 * method_access_descriptor
	 */
	ret = dlms_put_array(0, output);
	if (ret < 0)
		return ret;

	ret = encode_attribute_access_item(&attribute_access_item, output);
	if (ret < 0)
		return ret;

	/*
	 * attribute_access_descriptor
	 */
	ret = dlms_put_array(1, output);
	if (ret < 0)
		return ret;

	ret = dlms_put_structure(2, output);
	if (ret < 0)
		return ret;

	return 0;
}

static int encode_object_list_element(struct cosem_object_t *object, struct cosem_pdu_t *output)
{
	int ret;
	const struct cosem_class_t *cosem_class = object->cosem_class;

	ret = encode_access_rights(output);
	if (ret < 0)
		return ret;

	ret = dlms_put_logical_name(object->logical_name, output);
	if (ret < 0)
		return ret;

	ret = dlms_put_unsigned(cosem_class->version, output);
	if (ret < 0)
		return ret;

	ret = dlms_put_long_unsigned(cosem_class->class_id, output);
	if (ret < 0)
		return ret;

	ret = dlms_put_structure(4, output);
	if (ret < 0)
		return ret;

	return 0;
}

static int get_level_public_object_list(struct cosem_ctx_t *ctx, struct get_response_t *response, struct cosem_pdu_t *output)
{
	int ret;
	struct cosem_object_t *object;
	struct cosem_longname_t name = {
			.A = 0, .C = 40, .D = 0, .E = 0, .F = 255
	};

	printf("****************** get_level_public_object_list\n");

	object = cosem_association_get_object(ctx, name);

	ret = encode_object_list_element(object, output);
	if (ret < 0)
		return ret;

	ret = dlms_put_array(1, output);
	if (ret < 0)
		return ret;

	response->get_response_normal.result = access_result_success;

	return 0;
}

static int get_object_list(
		struct cosem_ctx_t *ctx,
		struct cosem_object_t *object,
		struct get_request_normal_t *get_request_normal,
		struct get_response_t *response,
		struct cosem_pdu_t *output
		)
{
	printf("****************** get_object_list\n");

	switch (ctx->association.spodes_access_level)
	{
	case spodes_access_level_public:
		return get_level_public_object_list(ctx, response, output);

	case spodes_access_level_reader:
		return get_level_public_object_list(ctx, response, output);

	case spodes_access_level_configurator:
		return get_level_public_object_list(ctx, response, output);
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
	struct cosem_association_t *association;

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

	association = (struct cosem_association_t*)object->data;

	ret = cosem_association_high_level_security_authentication_stage2(ctx, association, fStoC_input, fCtoS_output);
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

static int get_normal(struct cosem_ctx_t *ctx, struct cosem_object_t *object,
                      struct get_request_normal_t *get_request_normal, struct get_response_t *response,
                      struct cosem_pdu_t *output)
{
	printf("class association_ln: get attribute\n");

	switch (get_request_normal->cosem_attribute_descriptor.attribute_id)
	{
	case association_ln_object_list:
		return get_object_list(ctx, object, get_request_normal, response, output);

	default:
		response->get_response_normal.result = access_result_other_reason;
		break;
	}

	// Attribute not found
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
	}

	return action_result_scope_of_access_violated;
}

const struct cosem_class_t class_association_ln = {
	.class_id = 15,
	.version  = 1,

	.get_normal    = get_normal,
	.action_normal = action_normal,
};
