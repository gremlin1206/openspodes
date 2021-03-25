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

#include "cosem.h"
#include "class_association_ln.h"

static int encode_object_list(struct get_request_t *request, struct get_response_t *response)
{
	return 0;
}

static int reply_to_hls_authentication(struct action_request_t *request, struct action_response_t *response)
{
	unsigned char reply[16];
	int ret;

	if (request->action_request_normal.length != 16) {
		printf("reply_to_hls_authentication: wrong request length\n");
		response->result = action_result_temporary_failure;
		return 0;
	}

	ret = cosem_association_high_level_security_authentication_stage2(request->ctx, &request->ctx->association, request->action_request_normal.data, reply);
	if (ret < 0) {
		printf("reply_to_hls_authentication: authentication failed/n");
		response->result = action_result_temporary_failure;
		return 0;
	}

	response->buffer[0] = 0x01;
	response->buffer[1] = 0x00;
	response->buffer[2] = 0x09;
	response->buffer[3] = 0x10;

	memcpy(&response->buffer[4], reply, sizeof(reply));

	response->length = sizeof(reply) + 4;

	return 0;
}

static int get_attribute(struct get_request_t *request, struct get_response_t *response)
{
	printf("class association_ln: get attribute\n");

	switch (request->get_request_normal.cosem_attribute_descriptor.attribute_id)
	{
	case association_ln_object_list:
		return encode_object_list(request, response);
	}

	// Attribute not found
	return access_result_scope_of_access_violated;
}

static int action(struct action_request_t *request, struct action_response_t *response)
{
	printf("class association_ln: action\n");

	switch (request->action_request_normal.cosem_method_descriptor.method_id)
	{
	case association_ln_reply_to_hls_authentication:
		return reply_to_hls_authentication(request, response);
	}

	return action_result_scope_of_access_violated;
}

const struct cosem_class_t class_association_ln = {
	.class_id = 15,
	.get_normal = get_attribute,
	.action_normal = action,
};
