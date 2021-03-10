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

struct application_context_name_t
{
	unsigned int value;
};

struct acse_requirements_t
{
	unsigned short value;
};

struct mechanism_name_t
{
	unsigned int value;
};

struct calling_authentication_t
{
	unsigned char key[16];
	unsigned char length;
};

struct conformance_t
{
	unsigned char reserved;

	unsigned char reserved_seven : 1;
	unsigned char reserved_six : 1;
	unsigned char unconfirmed_write : 1;
	unsigned char write : 1;
	unsigned char read : 1;
	unsigned char general_block_transfer : 1;
	unsigned char general_protection : 1;
	unsigned char reserved_zero : 1;

	unsigned char information_report : 1;
	unsigned char multiple_references : 1;
	unsigned char block_transfer_with_action : 1;
	unsigned char block_transfer_with_set_or_write : 1;
	unsigned char block_transfer_with_get_or_read : 1;
	unsigned char attribute0_supported_with_get : 1;
	unsigned char priority_mgmt_supported : 1;
	unsigned char attribute0_supported_with_set : 1;

	unsigned char action : 1;
	unsigned char event_notification : 1;
	unsigned char selective_access : 1;
	unsigned char set : 1;
	unsigned char get : 1;
	unsigned char parameterized_access : 1;
	unsigned char access : 1;
	unsigned char data_notification : 1;
};

struct initiate_request_t
{
	unsigned char proposed_dlms_version_number;
	struct conformance_t proposed_conformance;
	unsigned short server_max_receive_pdu_size;
};

struct initiate_response_t
{
	unsigned char negotiated_dlms_version_number;
	struct conformance_t negotiated_conformance;
	unsigned short server_max_receive_pdu_size;
	unsigned short vaa_name;
};

enum association_result_t
{
	association_result_accepted = 0,
	association_result_rejected_permanent = 1,
	association_result_rejected_transient = 2,
};

enum acse_service_user_t
{
	acse_service_user_null,
	acse_service_user_no_reason_given,
	acse_service_user_application_context_name_not_supported,
	acse_service_user_calling_AP_title_not_recognized,
	acse_service_user_calling_AP_invocation_identifier_not_recognized,
	acse_service_user_calling_AE_qualifier_not_recognized,
	acse_service_user_calling_AE_invocation_identifier_not_recognized,
	acse_service_user_called_AP_title_not_recognized,
	acse_service_user_called_AP_invocation_identifier_not_recognized,
	acse_service_user_called_AE_qualifier_not_recognized,
	acse_service_user_called_AE_invocation_identifier_not_recognized,
	acse_service_user_authentication_mechanism_name_not_recognised,
	acse_service_user_authentication_mechanism_name_required,
	acse_service_user_authentication_failure,
	acse_service_user_authentication_required,
};

enum acse_service_provider_t
{
	acse_service_provider_null,
	acse_service_provider_no_reason_given,
	no_common_acse_version,
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

struct aarq_t
{
	struct application_context_name_t application_context_name;
	struct acse_requirements_t acse_requirements;
	struct mechanism_name_t mechanism_name;
	struct calling_authentication_t calling_authentication;
	struct initiate_request_t initiate_request;
};

struct aare_t
{
	struct application_context_name_t application_context_name; // A1
	enum association_result_t         association_result;       // A2
	enum acse_service_user_t          acse_service_user;        // A3
	enum acse_service_provider_t      acse_service_provider;    // A3
	struct initiate_response_t        initiate_response;        // BE
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

int cosem_process_aa_request(struct cosem_ctx_t *ctx, struct aarq_t *aarq, struct aare_t *aare);
int cosem_process_get_request(struct cosem_ctx_t *ctx, struct dlms_pdu_t *pdu, const struct get_request_t *request);

#endif /* OPENSPODES_COSEM_H */
