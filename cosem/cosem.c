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

#include <dlms/objects.h>
#include <cosem/asn1.h>
#include <cosem/cosem.h>

static int cosem_encode_exception(enum cosem_state_error_t state_error, enum cosem_service_error_t service_error, struct cosem_pdu_t *output)
{
	unsigned char *p;

	cosem_pdu_clear(output);

	p = cosem_pdu_put_cosem_data(output, 3);
	if (!p)
		return -1;

	p[0] = apdu_tag_exception_response;
	p[1] = (unsigned char)state_error;
	p[2] = (unsigned char)service_error;

	return 3;
}

static int cosem_decode_protocol_version(struct aarq_t *request, struct cosem_pdu_t *pdu)
{
	int ret;

	if (pdu->length != 2)
		return -1;

	ret = asn_get_uint16(&request->protocol_version, pdu);
	if (ret < 0)
		return ret;

	request->has_protocol_version = 1;

	return 0;
}

static const unsigned char application_context_name_preamble[] = {
	0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01
};

static int cosem_decode_application_context_name(struct aarq_t *request, struct cosem_pdu_t *pdu)
{
	unsigned char *p;

	printf("dlms_decode_application_context_name\n");

	if (pdu->length != sizeof(application_context_name_preamble) + 1)
		return -1;

	p = cosem_pdu_get_data(pdu, sizeof(application_context_name_preamble) + 1);
	if (!p)
		return -1;

	if (memcmp(p, application_context_name_preamble, sizeof(application_context_name_preamble)) != 0)
		return -1;

	request->application_context_name = p[sizeof(application_context_name_preamble)];
	request->has_application_context_name = 1;

	return 0;
}

static int cosem_encode_protocol_version(unsigned short protocol_version, struct cosem_pdu_t *output)
{
	int ret;

	ret = asn_put_uint16(protocol_version, output);
	if (ret < 0)
		return ret;

	return asn_put_tagged_data(0x80, ret, output);
}

static int cosem_encode_application_context_name(enum application_context_name_t name, struct cosem_pdu_t *output)
{
	unsigned char *p;

	p = cosem_pdu_put_cosem_data(output, sizeof(application_context_name_preamble) + 1);
	if (!p)
		return -1;

	printf("dlms_encode_application_context_name\n");

	memcpy(p, application_context_name_preamble, sizeof(application_context_name_preamble));
	p[sizeof(application_context_name_preamble)] = name;

	return asn_put_tagged_data(0xA1, sizeof(application_context_name_preamble) + 1, output);
}

static int cosem_encode_association_result(enum association_result_t result, struct cosem_pdu_t *output)
{
	unsigned char *p;

	p = cosem_pdu_put_cosem_data(output, 3);
	if (!p)
		return -1;

	p[0] = 0x02; p[1] = 0x01; // INTEGER tag and length
	p[2] = (unsigned char)result;

	return asn_put_tagged_data(0xA2, 3, output);
}

static int cosem_encode_result_source_diagnostic(enum acse_service_user_t acse_service_user,
						 enum acse_service_provider_t acse_service_provider,
						 struct cosem_pdu_t *output)
{
	unsigned char *p;

	p = cosem_pdu_put_cosem_data(output, 5);
	if (!p)
		return -1;

	if (acse_service_user >= 0) {
		p[0] = 0xA1; p[1] = 0x03; // acse-service-user tag and length
		p[2] = 0x02; p[3] = 0x01; // INTEGER tag and length
		p[4] = (unsigned char)acse_service_user;
	}
	else if (acse_service_provider >= 0) {
		p[0] = 0xA2; p[1] = 0x03; // acse-service-provider tag and length
		p[2] = 0x02; p[3] = 0x01; // INTEGER tag and length
		p[4] = (unsigned char)acse_service_provider;
	}

	return asn_put_tagged_data(0xA3, 5, output);
}

static int cosem_encode_conformance(struct conformance_t conformance, struct cosem_pdu_t *output)
{
	unsigned char *p;

	p = cosem_pdu_put_cosem_data(output, sizeof(conformance) + 3);
	if (!p)
		return -1;

	p[0] = 0x5F; p[1] = 0x1F; p[2] = sizeof(conformance); // conformance tag (2 bytes) and length
	memcpy(p + 3, &conformance, sizeof(conformance));

	return 3 + sizeof(conformance);
}

static int cosem_encode_initiate_response(const struct initiate_response_t *response, struct cosem_pdu_t *output)
{
	int ret;
	int bytes;

	printf("cosem_encode_initiate_response\n  vaa_name: %u\n  server_max_receive_pdu_size: %u\n",
			response->vaa_name,
			response->server_max_receive_pdu_size);


	ret = asn_put_uint16(response->vaa_name, output);
	if (ret < 0)
		return ret;
	bytes = ret;

	ret = asn_put_uint16(response->server_max_receive_pdu_size, output);
	if (ret < 0)
		return ret;
	bytes += ret;

	ret = cosem_encode_conformance(response->negotiated_conformance, output);
	if (ret < 0)
		return ret;
	bytes += ret;

	ret = asn_put_uint8(response->negotiated_dlms_version_number, output);
	if (ret < 0)
		return ret;
	bytes += ret;

	ret = asn_put_uint16(0x0800, output);
	if (ret < 0)
		return ret;
	bytes += ret;

	return bytes;
}

static int cosem_encode_confirmed_service_error(const struct confirmed_service_error_t *confirmed_service_error, struct cosem_pdu_t *output)
{
	unsigned char *p;
	enum cosem_error_t error;

	p = cosem_pdu_put_cosem_data(output, 4);
	if (!p)
		return -1;

	p[0] = 14; // ConfirmedServiceError tag

	p[1] = (unsigned char)confirmed_service_error->service;

	error = confirmed_service_error->error;
	p[2] = (unsigned char)(error >> 8);
	p[3] = (unsigned char)(error & 0xFF);

	return 4;
}

static int cosem_encode_aare_user_information(const struct aare_t *aare, struct cosem_pdu_t *output)
{
	int ret;

	if (aare->has_confirmed_service_error) {
		ret = cosem_encode_confirmed_service_error(&aare->user_information.confirmed_service_error, output);
	}
	else {
		ret = cosem_encode_initiate_response(&aare->user_information.initiate_response, output);
	}

	if (ret < 0)
		return ret;

	ret = asn_put_tagged_data(0x04, ret, output);
	if (ret < 0)
		return ret;

	return asn_put_tagged_data(0xBE, ret, output);
}

static int cosem_decode_acse_requirements(struct aarq_t *request, struct cosem_pdu_t *pdu)
{
	int ret;

	printf("dlms_decode_acse_requirements\n");

	if (pdu->length != 2)
		return -1;

	ret = asn_get_uint16(&request->acse_requirements, pdu);
	if (ret < 0)
		return ret;

	request->has_acse_requirements = 1;

	return 0;
}

static int cosem_encode_acse_requirements(unsigned short acse, struct cosem_pdu_t *output)
{
	int ret;

	ret = asn_put_uint16(acse, output);
	if (ret < 0)
		return ret;

	return asn_put_tagged_data(0x88, ret, output);
}

static const unsigned char mechanism_name_preamble[] = {
	0x60, 0x85, 0x74, 0x05, 0x08, 0x02
};

static int cosem_decode_mechanism_name(struct aarq_t *request, struct cosem_pdu_t *pdu)
{
	unsigned char *p;

	printf("dlms_decode_mechanism_name\n");

	if (pdu->length != sizeof(mechanism_name_preamble) + 1)
		return -1;

	p = cosem_pdu_get_data(pdu, sizeof(mechanism_name_preamble) + 1);
	if (!p)
		return -1;

	if (memcmp(p, mechanism_name_preamble, sizeof(mechanism_name_preamble)) != 0)
		return -1;

	request->mechanism_name = p[sizeof(mechanism_name_preamble)];
	request->has_mechanism_name = 1;

	return 0;
}

static int cosem_encode_mechanism_name(enum mechanism_name_t mn, struct cosem_pdu_t *output)
{
	unsigned char *p;

	p = cosem_pdu_put_cosem_data(output, sizeof(mechanism_name_preamble) + 1);
	if (!p)
		return -1;

	memcpy(p, mechanism_name_preamble, sizeof(mechanism_name_preamble));
	p[sizeof(mechanism_name_preamble)] = (unsigned char)mn;

	return asn_put_tagged_data(0x89, sizeof(mechanism_name_preamble) + 1, output);
}

static int cosem_decode_calling_authentication(struct aarq_t *request, struct cosem_pdu_t *pdu)
{
	int ret;
	unsigned int length;
	struct asn1_tagged_data_t tagged_data;
	unsigned char *p;

	printf("dlms_decode_calling_authentication\n");

	ret = asn_get_tagged_data(&tagged_data, pdu);
	if (ret < 0)
		return ret;

	switch (tagged_data.tag)
	{
	case 0x80:
		break;

	case 0x81:
		break;

	default:
		printf("unknown tag 0x%02X in calling authentication\n", tagged_data.tag);
		return -1;
	}

	length = tagged_data.pdu.length;

	if (length > sizeof(request->calling_authentication.bytes))
		return -1;

	p = cosem_pdu_get_data(&tagged_data.pdu, length);
	if (!p)
		return -1;

	memcpy(request->calling_authentication.bytes, p, length);
	request->calling_authentication.length = (unsigned char)length;
	request->has_calling_authentication = 1;

	return 0;
}

static int cosem_encode_responding_authentication_value(const struct authentication_value_t *out, struct cosem_pdu_t *output)
{
	unsigned char *p;
	unsigned int length = out->length;

	p = cosem_pdu_put_cosem_data(output, length + 2);
	if (!p)
		return -1;

	*p = 0x80; p++;
	*p = (unsigned char)(length); p++;

	memcpy(p, out->bytes, length);

	return asn_put_tagged_data(0xAA, length + 2, output);
}

static int cosem_decode_dedicated_key_dummy(struct cosem_pdu_t *pdu)
{
	int ret;
	unsigned int length;
	unsigned char *p;

	ret = asn_get_length(&length, pdu);
	if (ret < 0)
		return ret;

	p = cosem_pdu_get_data(pdu, length);
	if (!p)
		return -1;

	return 0;
}

static int cosem_decode_initiate_request(struct initiate_request_t *initiate_request, struct cosem_pdu_t *pdu)
{
	int ret;
	unsigned char tag;
	unsigned char option;
	unsigned int length;
	unsigned char *p;


	printf("dlms_decode_initiate_request\n");

	ret = asn_get_uint8(&option, pdu);
	if (ret < 0)
		return ret;

	if (option) {
		ret = cosem_decode_dedicated_key_dummy(pdu);
		if (ret < 0)
			return ret;
	}

	ret = asn_get_uint8(&option, pdu);
	if (ret < 0)
		return ret;

	if (option != 0)
		return -1;

	ret = asn_get_uint8(&option, pdu);
	if (ret < 0)
		return ret;

	if (option != 0)
		return -1;

	ret = asn_get_uint8(&initiate_request->proposed_dlms_version_number, pdu);
	if (ret < 0)
		return -1;

	ret = asn_get_uint8(&tag, pdu);
	if (ret < 0)
		return ret;

	if (tag != 0x5F)
		return -1;

	ret = asn_get_uint8(&tag, pdu);
	if (ret < 0)
		return ret;

	if (tag != 0x1F)
		return -1;

	ret = asn_get_length(&length, pdu);
	if (ret < 0)
		return ret;

	if (length != sizeof(initiate_request->proposed_conformance))
		return -1;

	p = cosem_pdu_get_data(pdu, length);
	if (!p)
		return -1;

	memcpy(&initiate_request->proposed_conformance, p, sizeof(initiate_request->proposed_conformance));

	ret = asn_get_uint16(&initiate_request->server_max_receive_pdu_size, pdu);
	if (ret < 0)
		return ret;

	return 0;
}

static int cosem_decode_aarq_user_information(struct aarq_t *request, struct cosem_pdu_t *pdu)
{
	int ret;
	unsigned char choice;
	struct asn1_tagged_data_t tagged_data;

	printf("cosem_decode_aarq_user_information\n");

	ret = asn_get_tagged_data(&tagged_data, pdu);
	if (ret < 0)
		return ret;

	if (tagged_data.tag != 0x04)
		return -1;

	ret = asn_get_uint8(&choice, &tagged_data.pdu);
	if (ret < 0)
		return ret;

	if (choice != 0x01)
		return -1;

	ret = cosem_decode_initiate_request(&request->initiate_request, &tagged_data.pdu);
	if (ret < 0)
		return ret;

	request->has_initiate_request = 1;

	return 0;
}

static int cosem_decode_aarq(struct aarq_t *request, struct cosem_pdu_t *pdu)
{
	int ret;
	struct asn1_tagged_data_t aarq;
	struct asn1_tagged_data_t field;

	printf("AARQ\n");

	memset(request, 0, sizeof(*request));

	ret = asn_get_tagged_data(&aarq, pdu);
	if (ret < 0) {
		printf("cosem_decode_aarq: error 1\n");
		return ret;
	}

	if (aarq.tag != apdu_tag_aarq) {
		printf("cosem_decode_aarq: error 2\n");
		return -1;
	}

	while (aarq.pdu.length > 0) {
		ret = asn_get_tagged_data(&field, &aarq.pdu);
		if (ret < 0) {
			printf("cosem_decode_aarq: error 3\n");
			return ret;
		}

		switch (field.tag)
		{
		case 0x80: // IMPLICIT
			ret = cosem_decode_protocol_version(request, &field.pdu);
			break;

		case 0xA1: // EXPLICIT
			ret = cosem_decode_application_context_name(request, &field.pdu);
			break;

		case 0x8A: // IMPLICIT
			ret = cosem_decode_acse_requirements(request, &field.pdu);
			break;

		case 0x8B: // IMPLICIT
			ret = cosem_decode_mechanism_name(request, &field.pdu);
			break;

		case 0xAC: // EXPLICIT
			ret = cosem_decode_calling_authentication(request, &field.pdu);
			break;

		case 0xBE: // EXPLICIT
			ret = cosem_decode_aarq_user_information(request, &field.pdu);
			break;

		default:
			printf("unknown tag: 0x%02X\n", field.tag);
			ret = 0; //cosem_decode_unknown_pdu_tag(&field.pdu);
			break;
		}

		if (ret < 0) {
			printf("fail to decode tag: 0x%02X\n", field.tag);
			return ret;
		}
	}

	printf("AARQ OK\n");

	return 0;
}

static int cosem_encode_aare(const struct aare_t *aare, struct cosem_pdu_t *output)
{
	int ret;
	int bytes;

	ret = cosem_encode_aare_user_information(aare, output);
	if (ret < 0)
		return ret;
	bytes = ret;

	if (aare->association_result == association_result_accepted) {
		if (aare->has_responding_authentication_value) {
			ret = cosem_encode_responding_authentication_value(&aare->responding_authentication_value, output);
			if (ret < 0)
				return ret;
			bytes += ret;
		}

		if (aare->has_mechanism_name) {
			ret = cosem_encode_mechanism_name(aare->mechanism_name, output);
			if (ret < 0)
				return ret;
			bytes += ret;
		}

		if (aare->has_acse_requirements) {
			ret = cosem_encode_acse_requirements(aare->acse_requirements, output);
			if (ret < 0)
				return ret;
			bytes += ret;
		}
	}

	ret = cosem_encode_result_source_diagnostic(aare->acse_service_user, aare->acse_service_provider, output);
	if (ret < 0)
		return ret;
	bytes += ret;

	ret = cosem_encode_association_result(aare->association_result, output);
	if (ret < 0)
		return ret;
	bytes += ret;

	ret = cosem_encode_application_context_name(aare->application_context_name, output);
	if (ret < 0)
		return ret;
	bytes += ret;

	if (aare->has_protocol_version) {
		ret = cosem_encode_protocol_version(aare->protocol_version, output);
		if (ret < 0)
			return ret;
		bytes += ret;
	}

	printf("cosem_encode_aare bytes = %u\n", bytes);

	return asn_put_tagged_data(0x61, bytes, output);
}

static int cosem_encode_aare_exception(enum acse_service_user_t service_user, struct cosem_pdu_t *output)
{
	struct aare_t aare;

	printf("cosem_encode_aare_exception\n");

	memset(&aare, 0, sizeof(aare));

	aare.application_context_name = application_context_long_names_clear_text;
	aare.association_result = association_result_rejected_permanent;

	aare.has_confirmed_service_error = 1;
	aare.acse_service_user = service_user;
	aare.acse_service_provider = acse_service_provider_no_error;

	return cosem_encode_aare(&aare, output);
}

static int cosem_decode_attribute_descriptor(struct cosem_attribute_descriptor_t *attribute_descriptor, struct cosem_pdu_t *pdu)
{
	int ret;
	struct cosem_longname_t instance_id;
	const unsigned char *p;

	ret = asn_get_uint16(&attribute_descriptor->class_id, pdu);
	if (ret < 0)
		return ret;

	p = cosem_pdu_get_data(pdu, 6);
	if (!p)
		return -1;

	if (p[0] > 1)
		return -1;

	instance_id.A = p[0];
	instance_id.C = p[2];
	instance_id.D = p[3];
	instance_id.E = p[4];
	instance_id.F = p[5];
	attribute_descriptor->instance_id = instance_id;

	ret = asn_get_uint8((unsigned char*)&attribute_descriptor->attribute_id, pdu);
	if (ret < 0)
		return ret;

	return 0;
}

//   C0  invoke-id-and-priority
//   00 0F class-id
//   00 00 28 00 00 FF instance-id
//   01 attribute-id
//   00 access-selection (00 means not present)
static int cosem_decode_get_request_normal(struct get_request_normal_t *get_request_normal, struct cosem_pdu_t *pdu)
{
	int ret;

	ret = asn_get_uint8(&get_request_normal->invoke_id_and_priority.byte, pdu);
	if (ret < 0)
		return ret;

	ret = cosem_decode_attribute_descriptor(&get_request_normal->cosem_attribute_descriptor, pdu);
	if (ret < 0)
		return ret;

	return 0;
}

// C0
//   01  get-type 0
static int cosem_decode_get_request(struct get_request_t *request, struct cosem_pdu_t *pdu)
{
	int ret;
	unsigned char tag;
	unsigned char type;

	printf("parce get request len: %u\n", pdu->length);

	ret = asn_get_uint8(&tag, pdu);
	if (ret < 0)
		return ret;

	if (tag != apdu_tag_get_request)
		return -1;

	ret = asn_get_uint8(&type, pdu);
	if (ret < 0)
		return ret;

	request->type = type;

	printf("  type: %u\n", type);

	switch (type)
	{
	case get_request_normal_type:
		ret = cosem_decode_get_request_normal(&request->get_request_normal, pdu);
		break;

	case get_request_next_type:
		ret = -1;
		break;

	case get_request_with_list_type:
		ret = -1;
		break;

	default:
		ret = -1;
		break;
	}

	if (ret < 0)
		return ret;

	printf("return ok\n");

	return 0;
}

static int cosem_docode_set_request_normal(struct set_request_normal_t *set_request_normal, struct cosem_pdu_t *pdu)
{
	int ret;
	unsigned char has_access_selection;

	ret = asn_get_uint8(&set_request_normal->invoke_id_and_priority.byte, pdu);
	if (ret < 0)
		return ret;

	ret = cosem_decode_attribute_descriptor(&set_request_normal->cosem_attribute_descriptor, pdu);
	if (ret < 0)
		return ret;

	ret = asn_get_uint8(&has_access_selection, pdu);
	if (ret < 0)
		return ret;

	if (has_access_selection != 0)
		return -1;

	return 0;
}

static int cosem_decode_set_request(struct set_request_t *request, struct cosem_pdu_t *pdu)
{
	int ret;
	unsigned char tag;
	unsigned char type;

	printf("parse set request length: %u\n", pdu->length);

	ret = asn_get_uint8(&tag, pdu);
	if (ret < 0)
		return ret;

	if (tag != apdu_tag_set_request)
		return -1;

	ret = asn_get_uint8(&type, pdu);
	if (ret < 0)
		return ret;

	printf("  type: %u\n", type);

	request->type = type;

	switch (type)
	{
	case set_request_normal:
		ret = cosem_docode_set_request_normal(&request->set_request_normal, pdu);
		break;

	case set_request_with_first_datablock:
		ret = -1;
		break;

	case set_request_with_datablock:
		ret = -1;
		break;

	case set_request_with_list:
		ret = -1;
		break;

	case set_request_with_list_and_first_datablock:
		ret = -1;
		break;

	default:
		ret = -1;
		break;
	}

	if (ret < 0)
		return ret;

	printf("return ok\n");

	return 0;
}

static int cosem_encode_data_result(enum data_access_result_t data_access_result, struct cosem_pdu_t *output)
{
	int ret;

	if (data_access_result == access_result_success) {
		ret = asn_put_uint8(0x00, output);
	}
	else {
		cosem_pdu_clear(output);

		ret = asn_put_uint8(data_access_result, output);
		if (ret < 0)
			return ret;

		ret = asn_put_uint8(0x01, output);
	}

	return ret;
}

static int cosem_encode_get_response_normal(struct get_response_normal_t *get_response_normal, struct cosem_pdu_t *output)
{
	int ret;

	ret = cosem_encode_data_result(get_response_normal->result, output);
	if (ret < 0)
		return ret;

	ret = asn_put_uint8(get_response_normal->invoke_id_and_priority.byte, output);
	if (ret < 0)
		return ret;

	return ret;
}

static int cosem_encode_get_response(struct get_response_t *response, struct cosem_pdu_t *output)
{
	int ret;

	switch (response->type)
	{
	case get_response_normal_type:
		ret = cosem_encode_get_response_normal(&response->get_response_normal, output);
		break;

	case get_response_with_datablock:
		ret = -1;
		break;

	case get_response_with_list:
		ret = -1;
		break;

	default:
		return -1;
	}

	ret = asn_put_uint8(response->type, output);
	if (ret < 0)
		return ret;

	ret = asn_put_uint8(apdu_tag_get_response, output);
	if (ret < 0)
		return ret;

	return 0;
}

static int cosem_encode_set_response_normal(struct set_response_normal_t *set_response_normal, struct cosem_pdu_t *output)
{
	int ret;

	ret = cosem_encode_data_result(set_response_normal->result, output);
	if (ret < 0)
		return ret;

	ret = asn_put_uint8(set_response_normal->invoke_id_and_priority.byte, output);
	if (ret < 0)
		return ret;

	return ret;
}

static int cosem_encode_set_response(struct set_response_t *response, struct cosem_pdu_t *output)
{
	int ret;

	switch (response->type)
	{
	case set_response_normal:
		ret = cosem_encode_set_response_normal(&response->set_response_normal, output);
		break;

	default:
		return -1;
	}

	ret = asn_put_uint8(response->type, output);
	if (ret < 0)
		return ret;

	ret = asn_put_uint8(apdu_tag_set_response, output);
	if (ret < 0)
		return ret;

	return 0;
}

static int cosem_decode_method_descriptor(struct cosem_method_descriptor_t *method_descriptor, struct cosem_pdu_t *pdu)
{
	int ret;
	struct cosem_longname_t instance_id;
	const unsigned char *p;

	ret = asn_get_uint16(&method_descriptor->class_id, pdu);
	if (ret < 0)
		return ret;

	p = cosem_pdu_get_data(pdu, 6);

	if (p[0] > 1)
		return -1;

	instance_id.A = p[0];
	instance_id.C = p[2];
	instance_id.D = p[3];
	instance_id.E = p[4];
	instance_id.F = p[5];
	method_descriptor->instance_id = instance_id;

	ret = asn_get_uint8((unsigned char*)&method_descriptor->method_id, pdu);
	if (ret < 0)
		return ret;

	return 0;
}

static int cosem_decode_action_request_normal(struct action_request_normal_t *action_request_normal, struct cosem_pdu_t *pdu)
{
	int ret;

	ret = asn_get_uint8(&action_request_normal->invoke_id_and_priority.byte, pdu);
	if (ret < 0)
		return ret;

	ret = cosem_decode_method_descriptor(&action_request_normal->cosem_method_descriptor, pdu);
	if (ret < 0)
		return ret;

	return 0;
}

/*
 *
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

static int cosem_decode_action_request(struct action_request_t *request, struct cosem_pdu_t *pdu)
{
	int ret;
	unsigned char tag;
	unsigned char type;

	printf("cosem_decode_action_request\n");

	ret = asn_get_uint8(&tag, pdu);
	if (ret < 0)
		return ret;

	if (tag != apdu_tag_action_request)
		return -1;

	ret = asn_get_uint8(&type, pdu);
	if (ret < 0)
		return ret;

	request->type = type;

	switch (type)
	{
	case action_request_normal_type:
		ret = cosem_decode_action_request_normal(&request->action_request_normal, pdu);
		break;

	default:
		ret = -1;
		break;
	}

	printf("cosem_decode_action_request: ok\n");

	return 0;
}

static int cosem_encode_action_response_normal(struct action_response_normal_t *action_response_normal, struct cosem_pdu_t *output)
{
	int ret;

	ret = asn_put_uint8(action_response_normal->result, output);
	if (ret < 0)
		return ret;

	ret = asn_put_uint8(action_response_normal->invoke_id_and_priority.byte, output);
	if (ret < 0)
		return ret;

	return ret;
}

static int cosem_encode_action_response(struct action_response_t *response, struct cosem_pdu_t *output)
{
	int ret;

	switch (response->type)
	{
	case get_response_normal_type:
		ret = cosem_encode_action_response_normal(&response->action_response_normal, output);
		break;

	case get_response_with_datablock:
		ret = -1;
		break;

	case get_response_with_list:
		ret = -1;
		break;

	default:
		return -1;
	}

	ret = asn_put_uint8(response->type, output);
	if (ret < 0)
		return ret;

	ret = asn_put_uint8(apdu_tag_action_response, output);
	if (ret < 0)
		return ret;

	return 0;
}

static int cosem_process_aarq(struct cosem_ctx_t *ctx, enum spodes_access_level_t access_level,
		                   struct cosem_pdu_t *pdu, struct cosem_pdu_t *output)
{
	int ret;
	struct aarq_t aarq;
	struct aare_t aare;

	ret = cosem_decode_aarq(&aarq, pdu);
	if (ret < 0)
		return cosem_encode_aare_exception(acse_service_user_no_reason_given, output);

	aarq.spodes_access_level = access_level;

	ret = cosem_association_open(ctx, &ctx->association, &aarq, &aare);
	if (ret < 0)
		return cosem_encode_aare_exception(acse_service_user_no_reason_given, output);

	printf("cosem_process_aarq: vaa_name: %u\n", aare.user_information.initiate_response.vaa_name);

	return cosem_encode_aare(&aare, output);
}

static int cosem_process_get_request(struct cosem_ctx_t *ctx, enum spodes_access_level_t access_level,
		                     struct cosem_pdu_t *pdu, struct cosem_pdu_t *output)
{
	int ret;
	struct get_request_t request;
	struct get_response_t response;
	
	printf("cosem_process_get_request\n");

	ret = cosem_decode_get_request(&request, pdu);
	if (ret < 0) {
		printf("cosem_process_get_request: fail to decode request\n");
		goto exception;
	}

	if (!ctx->association.associated) {
		printf("cosem_process_get_request: not associated\n");
		goto exception;
	}

	ret = cosem_object_get_request(ctx, &request, &response, output);
	if (ret < 0) {
		goto exception;
	}

	ret = cosem_encode_get_response(&response, output);
	if (ret < 0) {
		goto exception;
	}

	return 0;

exception:
	return cosem_encode_exception(cosem_state_error_service_not_allowed, cosem_service_error_operation_not_possible, output);
}

static int cosem_process_set_request(struct cosem_ctx_t *ctx, enum spodes_access_level_t access_level,
		                     struct cosem_pdu_t *pdu, struct cosem_pdu_t *output)
{
	int ret;
	struct set_request_t request;
	struct set_response_t response;

	printf("cosem_process_set_request\n");

	ret = cosem_decode_set_request(&request, pdu);
	if (ret < 0) {
		goto exception;
	}

	if (!ctx->association.associated) {
		printf("cosem_process_set_request: not associated\n");
		goto exception;
	}

	ret = cosem_object_set_request(ctx, &request, &response, pdu, output);
	if (ret < 0) {
		goto exception;
	}

	ret = cosem_encode_set_response(&response, output);
	if (ret < 0) {
		goto exception;
	}

	return ret;

exception:
	return cosem_encode_exception(cosem_state_error_service_not_allowed, cosem_service_error_operation_not_possible, output);
}

static int cosem_process_action_request(struct cosem_ctx_t *ctx, enum spodes_access_level_t access_level,
		                        struct cosem_pdu_t *pdu, struct cosem_pdu_t *output)
{
	int ret;
	struct action_request_t request;
	struct action_response_t response;

	printf("cosem_process_action_request");

	ret = cosem_decode_action_request(&request, pdu);
	if (ret < 0) {
		printf("dlms_process_action_request: parse failed\n");
		goto exception;
	}

	if (!ctx->association.associated) {
		printf("cosem_process_action_request: not associated\n");
		goto exception;
	}

	ret = cosem_object_action(ctx, &request, &response, pdu, output);
	if (ret < 0) {
		goto exception;
	}

	ret = cosem_encode_action_response(&response, output);
	if (ret < 0)
		goto exception;

	return 0;

exception:
	return cosem_encode_exception(cosem_state_error_service_not_allowed, cosem_service_error_operation_not_possible, output);
}

int cosem_input(struct cosem_ctx_t *ctx, enum spodes_access_level_t access_level,
		struct cosem_pdu_t *input_pdu, struct cosem_pdu_t *output_pdu)
{
	int ret;
	unsigned char tag;

#if 1
	unsigned int i;
	printf("process pdu (%u bytes):\n", input_pdu->length);
	for (i = 0; i < input_pdu->length; i++)
		printf("%02X ", input_pdu->head[i]);
	printf("\n");
#endif

	/*
	 * Peek tag from PDU head
	 */
	tag = input_pdu->head[0];

	switch (tag)
	{
	case apdu_tag_aarq:
		ret = cosem_process_aarq(ctx, access_level, input_pdu, output_pdu);
		break;

	case apdu_tag_get_request:
		ret = cosem_process_get_request(ctx, access_level, input_pdu, output_pdu);
		break;

	case apdu_tag_set_request:
		ret = cosem_process_set_request(ctx, access_level, input_pdu, output_pdu);
		break;

	case apdu_tag_action_request:
		ret = cosem_process_action_request(ctx, access_level, input_pdu, output_pdu);
		break;

	default:
		printf("request with unknown tag: %u\n", tag);
		ret = -1;
		break;
	}

	return ret;
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
