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

#include <dlms/class_association_ln.h>

static int get_object_list(
		struct cosem_ctx_t *ctx,
		struct cosem_object_t *object,
		struct get_request_normal_t *get_request_normal,
		struct get_response_t *response,
		struct cosem_pdu_t *output
		)
{
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

	.get_normal    = get_normal,
	.action_normal = action_normal,
};
