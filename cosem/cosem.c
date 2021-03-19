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
static void cosem_append_exception(struct cosem_pdu_t *pdu)
{
	unsigned char *p = &pdu->data[pdu->length];

	pdu->length += 4;
	p[0] = apdu_tag_exception_response;
}
#endif

static int cosem_decode_protocol_version(struct aarq_t *request, const unsigned char **buffer, unsigned int *length)
{
	int ret;
	unsigned int len;

	ret = asn_get_length(&len, buffer, length);
	if (ret < 0)
		return ret;

	if (len != 2)
		return -1;

	ret = asn_get_uint16(&request->protocol_version, buffer, length);
	if (ret < 0)
		return ret;

	request->has_protocol_version = 1;

	return 0;
}

static const unsigned char application_context_name_preamble[] = {
	0x09, 0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01
};

static const unsigned char mechanism_name_preamble[] = {
	0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x02
};

static int cosem_decode_application_context_name(struct aarq_t *request, const unsigned char **buffer, unsigned int *length)
{
	const unsigned char *p = *buffer;
	unsigned int len = *length;

	printf("dlms_decode_application_context_name\n");

	if (len < 10)
		return -1;

	if (memcmp(p, application_context_name_preamble, sizeof(application_context_name_preamble)) != 0)
		return -1;

	request->application_context_name = p[sizeof(application_context_name_preamble)];
	request->has_application_context_name = 1;

	*buffer += sizeof(application_context_name_preamble) + 1;
	*length -= sizeof(application_context_name_preamble) + 1;

	return 0;
}

static int cosem_encode_protocol_version(unsigned char *buffer, unsigned short protocol_version)
{
	buffer[0] = 0x80; buffer[1] = 0x02; buffer += 2;
	asn_put_uint16(buffer, protocol_version);

	return 4;
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
	else if (acse_service_provider >= 0) {
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

	buffer[4] = 0x08; // InitiateResponse
	buffer[5] = 0x00;

	buffer[6] = response->negotiated_dlms_version_number;

	cosem_encode_conformance(buffer + 7, response->negotiated_conformance);

	asn_put_uint16(buffer + 14, response->server_max_receive_pdu_size);
	asn_put_uint16(buffer + 16, response->vaa_name);

	return 18;
}

static int cosem_encode_confirmed_service_error(unsigned char *buffer, const struct confirmed_service_error_t *confirmed_service_error)
{
	enum cosem_error_t error;

	buffer[0] = 0xBE; buffer[1] = 0x06;
	buffer[2] = 0x04; buffer[3] = 0x04;

	buffer[4] = 14; // ConfirmedServiceError tag

	buffer[5] = (unsigned char)confirmed_service_error->service;

	error = confirmed_service_error->error;
	buffer[6] = (unsigned char)(error >> 8);
	buffer[7] = (unsigned char)(error & 0xFF);

	return 8;
}

static int cosem_decode_acse_requirements(struct aarq_t *request, const unsigned char **buffer, unsigned int *length)
{
	int ret;
	unsigned int len;

	printf("dlms_decode_acse_requirements\n");

	ret = asn_get_length(&len, buffer, length);
	if (ret < 0)
		return ret;

	ret = asn_get_uint16(&request->acse_requirements, buffer, length);
	if (ret < 0)
		return ret;

	request->has_acse_requirements = 1;

	return 0;
}

static int cosem_encode_acse_requirements(unsigned char *buffer, struct acse_requirements_t acse)
{
	buffer[0] = 0x88; buffer[1] = 0x02;

	buffer[2] = (unsigned char)(acse.value >> 8);
	buffer[3] = (unsigned char)(acse.value & 0xFF);

	return 4;
}

static int cosem_decode_mechanism_name(struct aarq_t *request, const unsigned char **buffer, unsigned int *length)
{
	const unsigned char *p = *buffer;

	printf("dlms_decode_mechanism_name\n");

	if (*length < (sizeof(mechanism_name_preamble) + 1))
		return -1;

	if (memcmp(p, mechanism_name_preamble, sizeof(mechanism_name_preamble)) != 0)
		return -1;

	request->mechanism_name = p[sizeof(mechanism_name_preamble)];
	request->has_mechanism_name = 1;

	*buffer += sizeof(mechanism_name_preamble) + 1;
	*length -= sizeof(mechanism_name_preamble) + 1;

	return 0;
}

static int cosem_encode_mechanism_name(unsigned char *buffer, enum mechanism_name_t mn)
{
	unsigned char *p = buffer;
	p[0] = 0x89; p++;

	memcpy(p, mechanism_name_preamble, sizeof(mechanism_name_preamble));
	p += sizeof(mechanism_name_preamble);

	p[0] = (unsigned char)mn; p++;

	return p - buffer;
}

static int cosem_decode_calling_authentication(struct aarq_t *request, const unsigned char **buffer, unsigned int *length)
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

	if (block_length > sizeof(request->calling_authentication.bytes))
		return -1;

	memcpy(request->calling_authentication.bytes, block_buffer, block_length);
	request->calling_authentication.length = (unsigned char)block_length;
	request->has_calling_authentication = 1;

	return 0;
}

static int cosem_encode_responding_authentication_value(unsigned char *buffer, const struct authentication_value_t *out)
{
	unsigned char *p = buffer;
	unsigned int length = out->length;

	p[0] = 0xAA; p++;
	p[0] = (unsigned char)(length + 2); p++;

	p[0] = 0x80; p++;
	p[0] = (unsigned char)(length); p++;

	memcpy(p, out->bytes, length); p += length;

	return p - buffer;
}

static int cosem_decode_initiate_request(struct aarq_t *request, const unsigned char **buffer, unsigned int *length)
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

	ret = asn_get_uint8(&request->initiate_request.proposed_dlms_version_number, &block_buffer, &block_length);
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

	if (len != sizeof(request->initiate_request.proposed_conformance))
		return -1;

	ret = asn_get_buffer(&request->initiate_request.proposed_conformance, sizeof(request->initiate_request.proposed_conformance), &block_buffer, &block_length);
	if (ret < 0)
		return ret;

	ret = asn_get_uint16(&request->initiate_request.server_max_receive_pdu_size, &block_buffer, &block_length);
	if (ret < 0)
		return ret;

	request->has_initiate_request = 1;

	return 0;
}

static int cosem_decode_unknown_pdu_tag(const unsigned char **buffer, unsigned int *length)
{
	int ret;
	unsigned int block_length;

	ret = asn_get_length(&block_length, buffer, length);
	if (ret < 0)
		return ret;

	if (*length < block_length)
		return -1;

	*buffer += block_length;
	*length -= block_length;

	return 0;
}

static int cosem_decode_aarq(struct aarq_t *request, const unsigned char *buffer, unsigned int length)
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
		case 0x80: // IMPLICIT
			ret = cosem_decode_protocol_version(request, &buffer, &length);
			break;

		case 0xA1: // EXPLICIT
			ret = cosem_decode_application_context_name(request, &buffer, &length);
			break;

		case 0x8A: // IMPLICIT
			ret = cosem_decode_acse_requirements(request, &buffer, &length);
			break;

		case 0x8B: // IMPLICIT
			ret = cosem_decode_mechanism_name(request, &buffer, &length);
			break;

		case 0xAC: // EXPLICIT
			ret = cosem_decode_calling_authentication(request, &buffer, &length);
			break;

		case 0xBE: // EXPLICIT
			ret = cosem_decode_initiate_request(request, &buffer, &length);
			break;

		default:
			printf("unknown tag: 0x%02X\n", tag);
			ret = cosem_decode_unknown_pdu_tag(&buffer, &length);
			break;
		}

		if (ret < 0) {
			printf("fail to decode tag: 0x%02X\n", tag);
			return ret;
		}
	}

	return 0;
}

static int cosem_encode_aare(unsigned char *buffer, const struct aare_t *aare)
{
	int ret;
	unsigned char *p = buffer;
	unsigned int length;

	p[0] = 0x61;
/*	p[1] = length will be filled at the end */

	p += 2;

	if (aare->has_protocol_version) {
		ret = cosem_encode_protocol_version(p, aare->protocol_version);
		if (ret < 0)
			return ret;
		p += ret;
	}

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

	if (aare->association_result == association_result_accepted) {
		if (aare->has_acse_requirements) {
			ret = cosem_encode_acse_requirements(p, aare->acse_requirements);
			if (ret < 0)
				return ret;
			p += ret;
		}

		if (aare->has_mechanism_name) {
			ret = cosem_encode_mechanism_name(p, aare->mechanism_name);
			if (ret < 0)
				return ret;
			p += ret;
		}

		if (aare->has_responding_authentication_value) {
			ret = cosem_encode_responding_authentication_value(p, &aare->responding_authentication_value);
			if (ret < 0)
				return ret;
			p += ret;
		}

		if (aare->has_confirmed_service_error)
			ret = cosem_encode_confirmed_service_error(p, &aare->user_information.confirmed_service_error);
		else
			ret = cosem_encode_initiate_response(p, &aare->user_information.initiate_response);
		if (ret < 0)
			return ret;
		p += ret;
	}

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

/*

C3 tag
01 normal action request
40 invoke id and priority
00 0F class
00 00 28 00 00 FF object
01 method id
01 has data
  09 10 octet string 16 bytes
    59 46 2C 2A B1 68 D2 D7 76 9F 2E 4C 87 0F 5F 58

 */

static int cosem_decode_action_request(struct action_request_t *request, const unsigned char *buffer, unsigned int length)
{
	int ret;
	unsigned short class_id;
	struct cosem_longname_t instance_id;
	unsigned char tag;
	unsigned char optional;
	unsigned int len;

	ret = asn_get_uint8(&tag, &buffer, &length);
	if (ret < 0)
		return ret;

	if (tag != apdu_tag_action_request)
		return -1;

	request->type = *buffer; buffer++;
	request->invoke_id_and_priority.byte = *buffer; buffer++;

	printf("  type: %i\n", request->type);

	switch (request->type) {
	case action_request_normal_type:
		class_id  = *buffer; buffer++;
		class_id <<= 8;
		class_id |= *buffer; buffer++;
		request->action_request_normal.cosem_method_descriptor.class_id = class_id;

		if (buffer[0] > 1)
			return -1;
		instance_id.A = buffer[0];
		instance_id.C = buffer[2];
		instance_id.D = buffer[3];
		instance_id.E = buffer[4];
		instance_id.F = buffer[5];
		request->action_request_normal.cosem_method_descriptor.instance_id = instance_id; buffer += 6;

		request->action_request_normal.cosem_method_descriptor.method_id = *buffer; buffer++;

		optional = *buffer; buffer++;
		if (optional) {
			ret = asn_get_uint8(&tag, &buffer, &length);
			if (ret < 0)
				return ret;

			switch (tag) {
			case 0x09:
				ret = asn_get_length(&len, &buffer, &length);
				if (ret < 0)
					return ret;

				request->action_request_normal.data = buffer;
				request->action_request_normal.length = len;

				break;

			default:
				printf("unknown data type tag: %u\n", tag);
				return -1;
			}
		}

		break;

	default:
		printf("unknown request type\n");
		return -1;
	}

	printf("return ok\n");

	return 0;
}

static int cosem_encode_action_response(unsigned char *buffer, struct action_response_t *response)
{
	unsigned char payload[] = { apdu_tag_action_response, 0x01, response->invoke_id_and_priority.byte, response->result };

	memcpy(buffer, payload, sizeof(payload));

	return sizeof(payload);
}

static int cosem_encode_get_access_result(unsigned char *buffer, enum data_access_result_t data_access_result)
{
	unsigned char payload[] = { apdu_tag_get_response, 0x01, 0xC1, 0x01, 0x00 };

	payload[sizeof(payload) - 1] = data_access_result;

	memcpy(buffer, payload, sizeof(payload));

	return sizeof(payload);
}

static int cosem_encode_action_result(unsigned char *buffer, enum action_result_t action_result)
{
	unsigned char payload[] = { apdu_tag_action_response, 0x01, 0xC1, 0x01, 0x00 };

	payload[sizeof(payload) - 1] = action_result;

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

	if (!request->ctx->association.authenticated) {
		printf("cosem_process_get_request: not authenticated\n");
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

static int cosem_process_action_request(struct action_request_t *request, struct action_response_t *response)
{
	struct cosem_object_t* object;

	printf("cosem_process_action_request");

	if (!request->ctx->association.associated) {
		printf("cosem_process_action_request: not associated\n");
		return -1;
	}

	object = cosem_find_object_by_name(request->action_request_normal.cosem_method_descriptor.instance_id);
	if (!object) {
		printf("cosem_process_get_request: object not found\n");
		response->result = action_result_object_undefined;
		return 0;
	}

	request->object = object;

	return cosem_object_action(request, response);
}

static int dlms_process_aa_request(struct cosem_ctx_t *ctx, struct cosem_pdu_t *pdu)
{
	int ret;
	struct aarq_t request;
	struct aare_t response;
	unsigned int length = cosem_pdu_payload_length(pdu);
	unsigned char* buffer = cosem_pdu_payload(pdu);

	ret = cosem_decode_aarq(&request, buffer, length);
	if (ret < 0)
		return ret;

	request.client_address = pdu->client_address;

	ret = cosem_process_aa_request(ctx, &request, &response);
	if (ret < 0)
		return ret;

	ret = cosem_encode_aare(buffer, &response);
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
		printf("dlms_process_get_request: parse failed\n");
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

static int dlms_process_action_request(struct cosem_ctx_t *ctx, unsigned char *buffer, unsigned int length)
{
	int ret;
	struct action_request_t request;
	struct action_response_t response;

	ret = cosem_decode_action_request(&request, buffer, length);
	if (ret < 0) {
		printf("dlms_process_action_request: parse failed\n");
		return ret;
	}

	request.ctx = ctx;

	memset(&response, 0, sizeof(response));
	response.buffer = buffer + 4;

	ret = cosem_process_action_request(&request, &response);
	if (ret < 0)
		return ret;

	if (response.result == action_result_success) {
		ret = cosem_encode_action_response(buffer, &response);
		if (ret < 0)
			return ret;

		return (int)(ret + response.length);
	}
	else {
		return cosem_encode_action_result(buffer, response.result);
	}

	return 0;
}

static int cosem_process_apdu(struct cosem_ctx_t *ctx, struct cosem_pdu_t *pdu)
{
	int ret;
	unsigned int i;
	unsigned char tag;
	unsigned int len = cosem_pdu_payload_length(pdu);
	unsigned char* buffer = cosem_pdu_payload(pdu);

	printf("process pdu (%u bytes):\n", len);
	for (i = 0; i < len; i++)
		printf("%02X ", buffer[i]);
	printf("\n");

	tag = buffer[0];

	switch (tag)
	{
	case apdu_tag_aa_request:
		ret = dlms_process_aa_request(ctx, pdu);
		break;

	case apdu_tag_get_request:
		ret = dlms_process_get_request(ctx, buffer, len);
		break;

	case apdu_tag_action_request:
		ret = dlms_process_action_request(ctx, buffer, len);
		break;

	default:
		printf("request with unknown tag: %u\n", tag);
		ret = -1;
		break;
	}

	if (ret < 0)
		return ret;

	cosem_pdu_set_payload_length(pdu, (unsigned int)ret);

	return 0;
}

int cosem_input(struct cosem_ctx_t *ctx, struct cosem_pdu_t *pdu)
{
	printf("cosem_input\n");
	return cosem_process_apdu(ctx, pdu);
}

void cosem_close_association(struct cosem_ctx_t *ctx)
{
	cosem_association_close(ctx, &ctx->association);
}

int cosem_init(struct cosem_ctx_t *ctx)
{
	memset(ctx, 0, sizeof(*ctx));

	memcpy(ctx->hls_auth_key.bytes, "SettingRiM489.2X", 16);
	ctx->hls_auth_key.length = 16;

	memcpy(ctx->lls_auth_key.bytes, "Reader", 6);
	ctx->lls_auth_key.length = 16;

	return 0;
}
