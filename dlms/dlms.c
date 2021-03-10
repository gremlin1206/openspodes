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

#include <cosem/cosem.h>

#include "dlms.h"

static int asn_get_length(unsigned int *out, const unsigned char **buffer, unsigned int *len)
{
	unsigned int length = *len;
	unsigned int result;
	unsigned char bytes;
	const unsigned char *p = *buffer;
	unsigned char byte;

	if (length < 1)
		return -1;

	byte = p[0]; length--; p++;

	if ((byte & 0x80) == 0) {
		result = byte;
	}
	else {
		bytes = byte & ~0x80;
		if (bytes > length || bytes > 4)
			return -1;

		length -= bytes;

		result = 0;
		for (;;) {
			result |= *p;
			bytes--;
			if (bytes == 0)
				break;
			result <<= 8;
			p++;
		}
	}

	printf("result %u, length: %u\n", result, length);

	if (result > length)
		return -1;

	*out = result;
	*buffer = p;
	*len = length;

	return 0;
}

static int asn_get_uint16(unsigned short *out, const unsigned char **buffer, unsigned int *length)
{
	unsigned short result;
	const unsigned char *p = *buffer;

	if (*length < 2)
		return -1;

	result = p[0];
	result <<= 8;
	result |= p[1];

	*out = result;

	*length -= 2;
	*buffer += 2;

	return 0;
}

static int asn_put_uint16(unsigned char *buffer, unsigned short value)
{
	buffer[0] = (unsigned char)(value >> 8);
	buffer[1] = (unsigned char)(value & 0xFF);

	return 2;
}

static int asn_get_uint8(unsigned char *out, const unsigned char **buffer, unsigned int *length)
{
	const unsigned char *p = *buffer;

	if (*length < 1)
		return -1;

	*out = p[0];

	*length -= 1;
	*buffer += 1;

	return 0;
}

static int asn_get_buffer(void *out, unsigned int bytes, const unsigned char **buffer, unsigned int *length)
{
	if (bytes > *length)
		return -1;

	memcpy(out, *buffer, bytes); *buffer += bytes; *length -= bytes;

	return 0;
}

static const unsigned char application_context_name_preamble[] = {
	0x09, 0x06, 0x07, 0x60, 0x85, 0x74
};

static const unsigned char mechanism_name_preamble[] = {
	0x07, 0x60, 0x85, 0x74
};

static int dlms_decode_application_context_name(struct application_context_name_t *out, const unsigned char **buffer, unsigned int *length)
{
	const unsigned char *p = *buffer;
	unsigned int len = *length;

	printf("dlms_decode_application_context_name\n");

	if (len < 10)
		return -1;

	if (memcmp(p, application_context_name_preamble, sizeof(application_context_name_preamble)) != 0)
		return -1;

	memcpy(out, p + sizeof(application_context_name_preamble), 4);

	*buffer += 10;
	*length -= 10;

	return 0;
}

static int dlms_encode_application_context_name(unsigned char *buffer, const struct application_context_name_t *name)
{
	unsigned char *p = buffer;

	printf("dlms_encode_application_context_name\n");

	p[0] = 0xA1; p++;

	memcpy(p, application_context_name_preamble, sizeof(application_context_name_preamble));
	p += sizeof(application_context_name_preamble);

	memcpy(p, name, sizeof(*name));

	return 1 + sizeof(application_context_name_preamble) + sizeof(*name);
}

static int dlms_encode_association_result(unsigned char *buffer, enum association_result_t result)
{
	buffer[0] = 0xA2; buffer[1] = 0x03; // association result tag and length
	buffer[2] = 0x02; buffer[3] = 0x01; // INTEGER tag and length
	buffer[4] = (unsigned char)result;

	return 5;
}

static int dlsm_encode_result_source_diagnostic(unsigned char *buffer,
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

static int dlms_encode_conformance(unsigned char *buffer, struct conformance_t conformance)
{
	buffer[0] = 0x5F; buffer[1] = 0x1F; buffer[2] = 0x04; // conformance tag (2 bytes) and length
	memcpy(buffer + 3, &conformance, sizeof(conformance));

	return 3 + sizeof(conformance);
}

static int dlms_encode_initiate_response(unsigned char *buffer, const struct initiate_response_t *response)
{
	buffer[0] = 0xBE; buffer[1] = 0x10;
	buffer[2] = 0x04; buffer[3] = 0x0E;

	buffer[4] = 0x08;
	buffer[5] = 0x00;

	buffer[6] = response->negotiated_dlms_version_number;

	dlms_encode_conformance(buffer + 7, response->negotiated_conformance);

	asn_put_uint16(buffer + 14, response->server_max_receive_pdu_size);
	asn_put_uint16(buffer + 16, response->vaa_name);

	return 18;
}

static int dlms_decode_acse_requirements(struct acse_requirements_t *out, const unsigned char **buffer, unsigned int *length)
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

static int dlms_decode_mechanism_name(struct mechanism_name_t *out, const unsigned char **buffer, unsigned int *length)
{
	const unsigned char *p = *buffer;

	printf("dlms_decode_mechanism_name\n");

	if (*length < 8)
		return -1;

	if (memcmp(p, mechanism_name_preamble, sizeof(mechanism_name_preamble)) != 0)
		return -1;

	memcpy(out, p + sizeof(mechanism_name_preamble), 4);

	*buffer += 8;
	*length -= 8;

	return 0;
}

static int dlms_decode_calling_authentication(struct calling_authentication_t *out, const unsigned char **buffer, unsigned int *length)
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

static int dlms_decode_initiate_request(struct initiate_request_t *out, const unsigned char **buffer, unsigned int *length)
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

static int dlms_decode_aa_request(struct aarq_t *request, const unsigned char *buffer, unsigned int length)
{
	int ret;
	unsigned char tag;

	printf("AARQ\n");

	ret = asn_get_length(&length, &buffer, &length);
	if (ret < 0) {
		printf("dlms_decode_aa_request fail to get length\n");
		return ret;
	}

	printf("\tlen: %u\n", length);

	while (length > 0) {
		ret = asn_get_uint8(&tag, &buffer, &length);
		if (ret < 0)
			return ret;

		switch (tag) {
		case 0xA1: // EXPLICIT
			ret = dlms_decode_application_context_name(&request->application_context_name, &buffer, &length);
			break;

		case 0x8A: // IMPLICIT
			ret = dlms_decode_acse_requirements(&request->acse_requirements, &buffer, &length);
			break;

		case 0x8B: // IMPLICIT
			ret = dlms_decode_mechanism_name(&request->mechanism_name, &buffer, &length);
			break;

		case 0xAC: // EXPLICIT
			ret = dlms_decode_calling_authentication(&request->calling_authentication, &buffer, &length);
			break;

		case 0xBE: // EXPLICIT
			ret = dlms_decode_initiate_request(&request->initiate_request, &buffer, &length);
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

static int dlms_encode_aa_response(unsigned char *buffer, const struct aare_t *aare)
{
	int ret;
	unsigned char *p = buffer;
	unsigned int length;

	p[0] = 0x61;
/*	p[1] = length will be filled at the end */

	p += 2;

	ret = dlms_encode_application_context_name(p, &aare->application_context_name);
	if (ret < 0)
		return ret;

	p += ret;

	ret = dlms_encode_association_result(p, aare->association_result);
	if (ret < 0)
		return ret;

	p += ret;

	ret = dlsm_encode_result_source_diagnostic(p, aare->acse_service_user, aare->acse_service_provider);
	if (ret < 0)
		return ret;

	p += ret;

	ret = dlms_encode_initiate_response(p, &aare->initiate_response);
	if (ret < 0)
		return ret;

	p += ret;

	length = (p - buffer) - 2;

	buffer[1] = length;

	return (int)length + 2;
}

// C0
//   01  get-type 0
//   C0  invoke-id-and-priority
//   00 0F class-id
//   00 00 28 00 00 FF instance-id
//   01 attribute-id
//   00 access-selection (00 means not present)
static int dlms_parse_get_request(struct get_request_t *request, unsigned char *p, unsigned int len)
{
	unsigned short class_id;

	printf("parce get request len: %u\n", len);

	if (len < 12)
		return -1;

	request->type = *p; p++;
	request->invoke_id_and_priority.byte = *p; p++;

	printf("  type: %i\n", request->type);

	switch (request->type) {
	case get_request_normal_type:
		class_id  = *p; p++;
		class_id <<= 8;
		class_id |= *p; p++;
		request->get_request_normal.cosem_attribute_descriptor.class_id = class_id;
		memcpy(request->get_request_normal.cosem_attribute_descriptor.instance_id, p, 6); p += 6;
		request->get_request_normal.cosem_attribute_descriptor.attribute_id = *p; p++;
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

static int dlms_process_aa_request(struct dlms_ctx_t *ctx, const unsigned char *p, unsigned int len)
{
	int ret;
	struct aarq_t request;
	struct aare_t response;

	ret = dlms_decode_aa_request(&request, p, len);
	if (ret < 0)
		return ret;

	ret = cosem_process_aa_request(ctx->cosem, &request, &response);
	if (ret < 0)
		return ret;

	ctx->pdu.data[0] = LLC_REMOTE_LSAP;
	ctx->pdu.data[1] = LLC_LOCAL_LSAP_RESPONSE;
	ctx->pdu.data[2] = 0;

	ret = dlms_encode_aa_response(ctx->pdu.data + 3, &response);
	if (ret < 0)
		return ret;

	ctx->pdu.length = (unsigned int)ret + 3;

	return 0;
}

static int dlms_process_get_request(struct dlms_ctx_t *ctx, unsigned char *p, unsigned int len)
{
	int ret;
	struct get_request_t request;

	ret = dlms_parse_get_request(&request, p, len);
	if (ret < 0)
		return ret;

	return cosem_process_get_request(ctx->cosem, &ctx->pdu, &request);
}

static int dlms_process_apdu(struct dlms_ctx_t *ctx, unsigned char *p, unsigned int len)
{
	//int ret;
	unsigned int i;
	unsigned char tag;

	printf("process pdu (%u bytes):\n", len);
	for (i = 0; i < len; i++)
		printf("%02X ", p[i]);
	printf("\n");

	tag = p[0];
	p += 1;
	len -= 1;

	switch (tag)
	{
	case apdu_tag_aa_request:
		return dlms_process_aa_request(ctx, p, len);

	case apdu_tag_get_request:
		return dlms_process_get_request(ctx, p, len);

	default:
		printf("request with unknown tag: %u\n", tag);
	}

	return -1;
}

static int dlms_process_llc(struct dlms_ctx_t *ctx, unsigned char *p, unsigned int len)
{
	if (len <= LLC_HEADER_LENGTH || p[0] != LLC_REMOTE_LSAP || p[1] != LLC_LOCAL_LSAP_REQUEST)
		return -1;

	return dlms_process_apdu(ctx, p + LLC_HEADER_LENGTH, len - LLC_HEADER_LENGTH);
}

int dlms_init(struct dlms_ctx_t *ctx)
{

#if 0
	struct aarq_t aa;
	static const unsigned char buffer[] = {
			/*0x60,*/ 0x34,
			0xA1, 0x09, 0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x01,
			0x8A, 0x02, 0x07, 0x80,
			0x8B, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x02, 0x01,
			0xAC, 0x08, 0x80, 0x06, 0x52, 0x65, 0x61, 0x64, 0x65, 0x72,
			0xBE, 0x10,
			  0x04, 0x0E,
			    0x01,
			      0x00,
			      0x00,
			      0x00,
			      0x06,
			      0x5F, 0x1F,
			        0x04, 0x00, 0x00, 0x10, 0x1C,
			      0xFF, 0xFF
	};

	int ret = dlms_decode_aa_request(&aa, buffer, sizeof(buffer));
	printf("decode ret: %i\n", ret);
#endif

#if 0
	struct aare_t aare;
	int ret;

	memset(&aare, 0, sizeof(aare));

	aare.application_context_name.value = 0x01010805;
	aare.association_result = association_result_accepted;
	aare.acse_service_user = 0;
	aare.acse_service_provider = -1;
	aare.initiate_response.negotiated_dlms_version_number = 6;
	aare.initiate_response.negotiated_conformance.selective_access = 1;
	aare.initiate_response.negotiated_conformance.set = 1;
	aare.initiate_response.negotiated_conformance.get = 1;
	aare.initiate_response.negotiated_conformance.block_transfer_with_get_or_read = 1;
	aare.initiate_response.server_max_receive_pdu_size = 0x400;
	aare.initiate_response.vaa_name = 7;

	unsigned char buffer[256];

	ret = dlms_encode_aa_response(buffer, &aare);
	if (ret < 0)
		printf("Encode error\n");

	int i;
	for (i = 0; i < ret; i++)
		printf("%02X ", buffer[i]);
	printf("\n");
#endif

	ctx->pdu.length = 0;
	return 0;
}

int dlms_input(struct dlms_ctx_t *ctx, unsigned char *p, unsigned int len, int more)
{
	unsigned int pdu_length = ctx->pdu.length;
	int ret;

	if (pdu_length + len > DLMS_MAX_PDU_SIZE) {
		dlms_drop_pdu(ctx);
		return -1;
	}

	memcpy(&ctx->pdu.data[pdu_length], p, len);
	ctx->pdu.length = pdu_length + len;

	if (!more) {
		ret = dlms_process_llc(ctx, ctx->pdu.data, len);
		if (ret < 0)
			return ret;
	}

	return more;
}

void dlms_drop_pdu(struct dlms_ctx_t *ctx)
{
	ctx->pdu.length = 0;
}
