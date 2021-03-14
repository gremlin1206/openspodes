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
#include <string.h>

#include "asn1.h"
#include "cosem.h"
#include "objects.h"

#if 0
static void cosem_append_error(struct cosem_pdu_t *pdu, enum cosem_error_service_t service, enum cosem_error_t error)
{
	unsigned char *p = &pdu->data[pdu->length];

	pdu->length += 4;
	p[0] = apdu_tag_confirmed_service_error;
	p[1] = (unsigned char)service;
	p[2] = (unsigned char)(error >> 8);
	p[3] = (unsigned char)(error & 0xFF);
}

static void cosem_append_exception(struct cosem_pdu_t *pdu)
{
	unsigned char *p = &pdu->data[pdu->length];

	pdu->length += 4;
	p[0] = apdu_tag_exception_response;
}
#endif

static const unsigned char application_context_name_preamble[] = {
	0x09, 0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01
};

static const unsigned char mechanism_name_preamble[] = {
	0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x02
};

static int cosem_decode_application_context_name(enum application_context_name_t *out, const unsigned char **buffer, unsigned int *length)
{
	const unsigned char *p = *buffer;
	unsigned int len = *length;

	printf("dlms_decode_application_context_name\n");

	if (len < 10)
		return -1;

	if (memcmp(p, application_context_name_preamble, sizeof(application_context_name_preamble)) != 0)
		return -1;

	*out = p[sizeof(application_context_name_preamble)];

	*buffer += sizeof(application_context_name_preamble) + 1;
	*length -= sizeof(application_context_name_preamble) + 1;

	return 0;
}

static int cosem_encode_application_context_name(unsigned char *buffer, enum application_context_name_t name)
{
	unsigned char *p = buffer;

	printf("dlms_encode_application_context_name\n");

	p[0] = 0xA1; p++;

	memcpy(p, application_context_name_preamble, sizeof(application_context_name_preamble));
	p[sizeof(application_context_name_preamble)] = name;

	return 1 + sizeof(application_context_name_preamble) + 1;
}

static int cosem_encode_association_result(unsigned char *buffer, enum association_result_t result)
{
	buffer[0] = 0xA2; buffer[1] = 0x03; // association result tag and length
	buffer[2] = 0x02; buffer[3] = 0x01; // INTEGER tag and length
	buffer[4] = (unsigned char)result;

	return 5;
}

static int cosem_encode_result_source_diagnostic(unsigned char *buffer,
		enum acse_service_user_t acse_service_user,
		enum acse_service_provider_t acse_service_provider)
{
	buffer[0] = 0xA3; buffer[1] = 0x05; // result-source-diagnostic tag and length
	if (acse_service_user >= 0) {
		buffer[2] = 0xA1; buffer[3] = 0x03; // acse-service-user tag and length
		buffer[4] = 0x02; buffer[5] = 0x01; // INTEGER tag and length
		buffer[6] = (unsigned char)acse_service_user;
	}
	else {
		buffer[2] = 0xA2; buffer[3] = 0x03; // acse-service-provider tag and length
		buffer[4] = 0x02; buffer[5] = 0x01; // INTEGER tag and length
		buffer[6] = (unsigned char)acse_service_provider;
	}

	return 7;
}

static int cosem_encode_conformance(unsigned char *buffer, struct conformance_t conformance)
{
	buffer[0] = 0x5F; buffer[1] = 0x1F; buffer[2] = 0x04; // conformance tag (2 bytes) and length
	memcpy(buffer + 3, &conformance, sizeof(conformance));

	return 3 + sizeof(conformance);
}

static int cosem_encode_initiate_response(unsigned char *buffer, const struct initiate_response_t *response)
{
	buffer[0] = 0xBE; buffer[1] = 0x10;
	buffer[2] = 0x04; buffer[3] = 0x0E;

	buffer[4] = 0x08;
	buffer[5] = 0x00;

	buffer[6] = response->negotiated_dlms_version_number;

	cosem_encode_conformance(buffer + 7, response->negotiated_conformance);

	asn_put_uint16(buffer + 14, response->server_max_receive_pdu_size);
	asn_put_uint16(buffer + 16, response->vaa_name);

	return 18;
}

static int cosem_decode_acse_requirements(struct acse_requirements_t *out, const unsigned char **buffer, unsigned int *length)
{
	const unsigned char *p = *buffer;
	unsigned int len = *length;

	printf("dlms_decode_acse_requirements\n");

	if (len < 3)
		return -1;

	if (p[0] != 2)
		return -1;

	memcpy(out, p + 1, 2);

	*buffer += 3;
	*length -= 3;

	return 0;
}

static int cosem_decode_mechanism_name(enum mechanism_name_t *out, const unsigned char **buffer, unsigned int *length)
{
	const unsigned char *p = *buffer;

	printf("dlms_decode_mechanism_name\n");

	if (*length < (sizeof(mechanism_name_preamble) + 1))
		return -1;

	if (memcmp(p, mechanism_name_preamble, sizeof(mechanism_name_preamble)) != 0)
		return -1;

	*out = p[sizeof(mechanism_name_preamble)];

	*buffer += sizeof(mechanism_name_preamble) + 1;
	*length -= sizeof(mechanism_name_preamble) + 1;

	return 0;
}

static int cosem_decode_calling_authentication(struct authentication_value_t *out, const unsigned char **buffer, unsigned int *length)
{
	int ret;
	unsigned int block_length;
	const unsigned char* block_buffer;
	unsigned char tag;

	printf("dlms_decode_calling_authentication\n");

	ret = asn_get_length(&block_length, buffer, length);
	if (ret < 0)
		return ret;

	block_buffer = *buffer;
	*buffer += block_length;
	*length -= block_length;

	ret = asn_get_uint8(&tag, &block_buffer, &block_length);
	if (ret < 0)
		return ret;

	switch (tag)
	{
	case 0x80:
		break;

	case 0x81:
		break;

	default:
		printf("unknown tag 0x%02X in calling authentication\n", tag);
		return -1;
	}

	ret = asn_get_length(&block_length, &block_buffer, &block_length);
	if (ret < 0)
		return ret;

	if (block_length > sizeof(out->key))
		return -1;

	memcpy(out->key, block_buffer, block_length);
	out->length = (unsigned char)block_length;

	return 0;
}

static int cosem_decode_initiate_request(struct initiate_request_t *out, const unsigned char **buffer, unsigned int *length)
{
	int ret;
	unsigned int block_length;
	const unsigned char *block_buffer;
	unsigned char tag;
	unsigned char option;
	unsigned int len;

	printf("dlms_decode_initiate_request\n");

	ret = asn_get_length(&block_length, buffer, length);
	if (ret < 0)
		return ret;

	block_buffer = *buffer;
	*buffer += block_length;
	*length -= block_length;

	ret = asn_get_uint8(&tag, &block_buffer, &block_length);
	if (ret < 0)
		return ret;

	if (tag != 0x04)
		return -1;

	ret = asn_get_length(&block_length, &block_buffer, &block_length);
	if (ret < 0)
		return ret;

	ret = asn_get_uint8(&tag, &block_buffer, &block_length);
	if (ret < 0)
		return ret;

	if (tag != 0x01)
		return -1;

	ret = asn_get_uint8(&option, &block_buffer, &block_length);
	if (ret < 0)
		return ret;

	if (option != 0)
		return -1;

	ret = asn_get_uint8(&option, &block_buffer, &block_length);
	if (ret < 0)
		return ret;

	if (option != 0)
		return -1;

	ret = asn_get_uint8(&option, &block_buffer, &block_length);
	if (ret < 0)
		return ret;

	if (option != 0)
		return -1;

	ret = asn_get_uint8(&out->proposed_dlms_version_number, &block_buffer, &block_length);
	if (ret < 0)
		return -1;

	ret = asn_get_uint8(&tag, &block_buffer, &block_length);
	if (ret < 0)
		return ret;

	if (tag != 0x5F)
		return -1;

	ret = asn_get_uint8(&tag, &block_buffer, &block_length);
	if (ret < 0)
		return ret;

	if (tag != 0x1F)
		return -1;

	ret = asn_get_length(&len, &block_buffer, &block_length);
	if (ret < 0)
		return ret;

	if (len != sizeof(out->proposed_conformance))
		return -1;

	ret = asn_get_buffer(&out->proposed_conformance, sizeof(out->proposed_conformance), &block_buffer, &block_length);
	if (ret < 0)
		return ret;

	ret = asn_get_uint16(&out->server_max_receive_pdu_size, &block_buffer, &block_length);
	if (ret < 0)
		return ret;

	return 0;
}

static int cosem_decode_aa_request(struct aarq_t *request, const unsigned char *buffer, unsigned int length)
{
	int ret;
	unsigned char tag;

	printf("AARQ\n");

	ret = asn_get_uint8(&tag, &buffer, &length);
	if (ret < 0)
		return ret;

	if (tag != apdu_tag_aa_request)
		return ret;

	ret = asn_get_length(&length, &buffer, &length);
	if (ret < 0)
		return ret;

	memset(request, 0, sizeof(*request));

	printf("\tlen: %u\n", length);

	while (length > 0) {
		ret = asn_get_uint8(&tag, &buffer, &length);
		if (ret < 0)
			return ret;

		switch (tag) {
		case 0xA1: // EXPLICIT
			ret = cosem_decode_application_context_name(&request->application_context_name, &buffer, &length);
			break;

		case 0x8A: // IMPLICIT
			ret = cosem_decode_acse_requirements(&request->acse_requirements, &buffer, &length);
			break;

		case 0x8B: // IMPLICIT
			ret = cosem_decode_mechanism_name(&request->mechanism_name, &buffer, &length);
			break;

		case 0xAC: // EXPLICIT
			ret = cosem_decode_calling_authentication(&request->calling_authentication, &buffer, &length);
			break;

		case 0xBE: // EXPLICIT
			ret = cosem_decode_initiate_request(&request->initiate_request, &buffer, &length);
			break;

		default:
			printf("unknown tag: 0x%02X\n", tag);
			ret = -1;
			break;
		}

		if (ret < 0) {
			printf("fail to decode tag: 0x%02X\n", tag);
			return ret;
		}
	}

	return 0;
}

static int cosem_encode_aa_response(unsigned char *buffer, const struct aare_t *aare)
{
	int ret;
	unsigned char *p = buffer;
	unsigned int length;

	p[0] = 0x61;
/*	p[1] = length will be filled at the end */

	p += 2;

	ret = cosem_encode_application_context_name(p, aare->application_context_name);
	if (ret < 0)
		return ret;

	p += ret;

	ret = cosem_encode_association_result(p, aare->association_result);
	if (ret < 0)
		return ret;

	p += ret;

	ret = cosem_encode_result_source_diagnostic(p, aare->acse_service_user, aare->acse_service_provider);
	if (ret < 0)
		return ret;

	p += ret;

	ret = cosem_encode_initiate_response(p, &aare->initiate_response);
	if (ret < 0)
		return ret;

	p += ret;

	length = (p - buffer) - 2;

	buffer[1] = length;

	return (int)length + 2 /* 1 tag byte and 1 length byte */;
}

// C0
//   01  get-type 0
//   C0  invoke-id-and-priority
//   00 0F class-id
//   00 00 28 00 00 FF instance-id
//   01 attribute-id
//   00 access-selection (00 means not present)
static int cosem_decode_get_request(struct get_request_t *request, const unsigned char *buffer, unsigned int length)
{
	int ret;
	unsigned short class_id;
	struct cosem_longname_t instance_id;
	unsigned char tag;

	printf("parce get request len: %u\n", length);

	if (length < 13)
		return -1;

	ret = asn_get_uint8(&tag, &buffer, &length);
	if (ret < 0)
		return ret;

	if (tag != apdu_tag_get_request)
		return -1;

	request->type = *buffer; buffer++;
	request->invoke_id_and_priority.byte = *buffer; buffer++;

	printf("  type: %i\n", request->type);

	switch (request->type) {
	case get_request_normal_type:
		class_id  = *buffer; buffer++;
		class_id <<= 8;
		class_id |= *buffer; buffer++;
		request->get_request_normal.cosem_attribute_descriptor.class_id = class_id;

		if (buffer[0] > 1)
			return -1;
		instance_id.A = buffer[0];
		instance_id.C = buffer[2];
		instance_id.D = buffer[3];
		instance_id.E = buffer[4];
		instance_id.F = buffer[5];
		request->get_request_normal.cosem_attribute_descriptor.instance_id = instance_id; buffer += 6;

		request->get_request_normal.cosem_attribute_descriptor.attribute_id = *buffer; buffer++;
		break;

	case get_request_next_type:
		break;

	case get_request_with_list_type:
		break;

	default:
		return -1;
	}

	printf("return ok\n");

	return 0;
}

static int cosem_encode_get_response(unsigned char *buffer, struct get_response_t *response)
{
	unsigned char payload[] = { 0xC4, 0x01, 0xC1, 0x00 };

	memcpy(buffer, payload, sizeof(payload));

	return sizeof(payload);
}

static int cosem_encode_get_access_result(unsigned char *buffer, enum data_access_result_t data_access_result)
{
	unsigned char payload[] = { 0xC4, 0x01, 0xC1, 0x01, 0x00 };

	payload[sizeof(payload) - 1] = data_access_result;

	memcpy(buffer, payload, sizeof(payload));

	return sizeof(payload);
}

static int cosem_process_aa_request(struct cosem_ctx_t *ctx, struct aarq_t *aarq, struct aare_t *aare)
{
	return cosem_association_open(ctx, &ctx->association, aarq, aare);
}

static int cosem_process_get_request(struct get_request_t *request, struct get_response_t *response)
{
	struct cosem_object_t* object;

	printf("cosem_process_get_request\n");

	if (!request->ctx->association.associated) {
		printf("cosem_process_get_request: not associated\n");
		return -1;
	}

	object = cosem_find_object_by_name(request->get_request_normal.cosem_attribute_descriptor.instance_id);
	if (!object) {
		printf("cosem_process_get_request: object not found\n");
		response->data_access_result = access_result_object_undefined;
		return 0;
	}

	request->object = object;

	return cosem_object_get_attribute(request, response);
}

static int dlms_process_aa_request(struct cosem_ctx_t *ctx, unsigned char *buffer, unsigned int length)
{
	int ret;
	struct aarq_t request;
	struct aare_t response;

	ret = cosem_decode_aa_request(&request, buffer, length);
	if (ret < 0)
		return ret;

	ret = cosem_process_aa_request(ctx, &request, &response);
	if (ret < 0)
		return ret;

	ret = cosem_encode_aa_response(buffer, &response);
	if (ret < 0)
		return ret;

	return ret;
}

static int dlms_process_get_request(struct cosem_ctx_t *ctx, unsigned char *buffer, unsigned int length)
{
	int ret;
	struct get_request_t request;
	struct get_response_t response;
	
	printf("dlms_process_get_request\n");

	ret = cosem_decode_get_request(&request, buffer, length);
	if (ret < 0) {
		printf("dlms_process_get_request: parce failed\n");
		return ret;
	}

	request.ctx = ctx;

	memset(&response, 0, sizeof(response));
	response.buffer = buffer + 4;

	ret = cosem_process_get_request(&request, &response);
	if (ret < 0)
		return ret;

	if (response.data_access_result == access_result_success) {
		ret = cosem_encode_get_response(buffer, &response);
		if (ret < 0)
			return ret;

		return (int)(ret + response.length);
	}
	else {
		return cosem_encode_get_access_result(buffer, response.data_access_result);
	}
}

static int cosem_process_apdu(struct cosem_ctx_t *ctx, unsigned char *buffer, unsigned int len)
{
	unsigned int i;
	unsigned char tag;

	printf("process pdu (%u bytes):\n", len);
	for (i = 0; i < len; i++)
		printf("%02X ", buffer[i]);
	printf("\n");

	tag = buffer[0];

	switch (tag)
	{
	case apdu_tag_aa_request:
		return dlms_process_aa_request(ctx, buffer, len);

	case apdu_tag_get_request:
		return dlms_process_get_request(ctx, buffer, len);

	default:
		printf("request with unknown tag: %u\n", tag);
	}

	return -1;
}

int cosem_input(struct cosem_ctx_t *ctx, struct cosem_pdu_t *pdu)
{
	int ret;
	unsigned int length = cosem_pdu_payload_length(pdu);
	unsigned char* payload = cosem_pdu_payload(pdu);

	printf("cosem_input\n");

	if (length == 0) {
		printf("empty PDU\n");
		return -1;
	}

	ret = cosem_process_apdu(ctx, payload, length);
	if (ret < 0)
		return ret;

	cosem_pdu_set_payload_length(pdu, (unsigned int)ret);

	return 0;
}

void cosem_close_association(struct cosem_ctx_t *ctx)
{
	memset(&ctx->association, 0, sizeof(ctx->association));
}

int cosem_init(struct cosem_ctx_t *ctx)
{
	memset(ctx, 0, sizeof(*ctx));
	return 0;
}
