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

#ifndef OPENSPODES_COSEM_H
#define OPENSPODES_COSEM_H

#define LLC_REMOTE_LSAP         0xE6
#define LLC_LOCAL_LSAP_REQUEST  0xE6
#define LLC_LOCAL_LSAP_RESPONSE 0xE7
#define LLC_HEADER_LENGTH       3

struct dlms_pdu_t;

enum apdu_tag_t
{
	apdu_tag_confirmed_service_error     = 0x0E,

	apdu_tag_aa_request                  = 0x60,
	apdu_tag_aa_response                 = 0x61,
	apdu_tag_get_request                 = 0xC0,
	apdu_tag_set_request                 = 0xC1,
	apdu_tag_event_notification_request  = 0xC2,
	apdu_tag_action_request              = 0xC3,
	apdu_tag_get_response                = 0xC4,
	apdu_tag_set_response                = 0xC5,
	apdu_tag_action_response             = 0xC7,
	apdu_tag_glo_get_request             = 0xC8,
	apdu_tag_glo_set_request             = 0xC9,
	apdu_tag_event_notification_response = 0xCA,
	apdu_tag_glo_action_request          = 0xCB,
	apdu_tag_glo_get_response            = 0xCC,
	apdu_tag_glo_set_response            = 0xCD,
	apdu_tag_glo_action_response         = 0xCF,

	apdu_tag_exception_response          = 216,
};

enum cosem_error_service_t
{
	cosem_error_initiate = 1,
	cosem_error_read = 5,
};

enum cosem_error_t
{
	/*
	 * cosem_error_application [0]
	 */
	cosem_error_application_other = 0,
	cosem_error_application_time_elapsed,
	cosem_error_application_unreachable,
	cosem_error_application_reference_invalid,
	cosem_error_application_context_unsupported,
	cosem_error_application_provider_communication_error,
	cosem_error_application_deciphering_error,


	/*
	 * cosem_error_hardware_resource [1]
	 */
	cosem_error_hardware_resource_other = (1 << 8),
	cosem_error_hardware_resource_memory_unavailable,
	cosem_error_hardware_resource_processor_unavailable,
	cosem_error_hardware_resource_mass_storage_unavailable,
	cosem_error_hardware_resource_other_resource_unavailable,

	/*
	 * cosem_error_access [5]
	 */
	cosem_error_access_other = (5 << 8),
	cosem_error_access_scope_of_access_violated,
	cosem_error_access_object_access_violated,
	cosem_error_access_hardware_fault,
	cosem_error_access_object_unavailable,
};

struct confirmed_service_error_t
{
	enum cosem_error_service_t service;

};

struct invoke_id_and_priority_t
{
	union {
		struct {
			unsigned char invoke_id : 4;
			unsigned char reserved  : 2;
			unsigned char service_class : 1; /* 0 = Unconfirmed, 1 = Confirmed */
			unsigned char priority  : 1;     /* 0 = Normal, 1 = High */
		};

		unsigned char byte;
	};
};

struct cosem_attribute_descripor_t
{
	unsigned short class_id;
	unsigned char  instance_id[6];
	signed char    attribute_id;
};

struct cosem_method_descripor_t
{
	unsigned short class_id;
	unsigned char  instance_id[6];
	signed char    method_id;
};

struct get_request_normal_t
{
	struct cosem_attribute_descripor_t cosem_attribute_descriptor;
};

enum get_request_type_t
{
	get_request_normal_type = 1,
	get_request_next_type = 2,
	get_request_with_list_type = 3,
};

struct get_request_t
{
	enum get_request_type_t type;
	struct invoke_id_and_priority_t invoke_id_and_priority;
	union {
		struct get_request_normal_t get_request_normal;
	};
};

struct cosem_ctx_t
{
	int associated;
};

int cosem_process_get_request(struct cosem_ctx_t *ctx, struct dlms_pdu_t *pdu, const struct get_request_t *request);

#endif /* OPENSPODES_COSEM_H */
