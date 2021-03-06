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
#include <dlms/class_activity_calendar.h>

static int get_attributes(struct cosem_ctx_t *ctx, struct cosem_object_t *object, struct cosem_pdu_t *output)
{
	int ret;

	ret = encode_attribute_access_item(activity_calendar_name_active, attribute_write_only, 0, 0, output);
	if (ret < 0)
		return ret;

	return 1;
}

static int get_normal(struct cosem_ctx_t *ctx, struct cosem_object_t *object,
                      struct get_request_normal_t *get_request_normal, struct get_response_t *response,
                      struct cosem_pdu_t *output)
{
	printf("class activity_calendar: get attribute\n");

	switch (get_request_normal->cosem_attribute_descriptor.attribute_id)
	{
	case activity_calendar_name_active:
		return dlms_put_octet_string((unsigned char*)"test", 4, output);

	default:
		// Attribute not found
		response->get_response_normal.result = access_result_other_reason;
		break;
	}


	return 0;
}

static int action_normal(struct cosem_ctx_t *ctx, struct cosem_object_t *object,
                         struct action_request_normal_t *action_request_normal, struct action_response_t *response,
		         struct cosem_pdu_t *pdu, struct cosem_pdu_t *output)
{
	//int ret;

	printf("class activity_calendar: action\n");

	switch (action_request_normal->cosem_method_descriptor.method_id)
	{
	default:
		response->action_response_normal.result = action_result_other_reason;
		break;
	}

	return 0;
}

const struct cosem_class_t class_activity_calendar = {
	.class_id = 20,
	.version  = 0,

	.get_normal           = get_normal,
	.action_normal        = action_normal,
	.get_attributes       = get_attributes,
};
