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
#include <dlms/class_profile_generic.h>

static int get_object_access_rights(struct cosem_ctx_t *ctx, struct cosem_object_t *object, struct cosem_pdu_t *output)
{
	int ret;
	dlms_integer_t access_selectors[] = {1, 2};

	/*
	 * method_access_descriptor
	 */
	ret = dlms_put_array(0, output);
	if (ret < 0)
		return ret;

	ret = encode_attribute_access_item(profile_generic_logical_name, attribute_read_only, 0, 0, output);
	if (ret < 0)
		return ret;

	ret = encode_attribute_access_item(profile_generic_buffer, attribute_read_only,
			access_selectors, sizeof(access_selectors), output);
	if (ret < 0)
		return ret;

	/*
	 * attribute_access_descriptor
	 */
	ret = dlms_put_array(2, output);
	if (ret < 0)
		return ret;

	ret = dlms_put_structure(2, output);
	if (ret < 0)
		return ret;

	return 0;
}

static int get_normal(struct cosem_ctx_t *ctx, struct cosem_object_t *object,
                      struct get_request_normal_t *get_request_normal, struct get_response_t *response,
                      struct cosem_pdu_t *output)
{
	printf("class profile_generic: get attribute\n");

	switch (get_request_normal->cosem_attribute_descriptor.attribute_id)
	{
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
	printf("class profile_generic: action\n");

	switch (action_request_normal->cosem_method_descriptor.method_id)
	{
	default:
		response->action_response_normal.result = action_result_other_reason;
		break;
	}

	return 0;
}

const struct cosem_class_t class_profile_generic = {
	.class_id = 7,
	.version  = 0,

	.get_normal           = get_normal,
	.action_normal        = action_normal,
	.get_object_access_rights = get_object_access_rights,
};
