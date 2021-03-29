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

#ifndef COSEM_TYPES_H_
#define COSEM_TYPES_H_

#include <spodes/spodes.h>
#include <hdlc/frame.h>

enum apdu_tag_t
{
	apdu_tag_aarq                        = 0x60,
	apdu_tag_aare                        = 0x61,
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
	cosem_error_read     = 5,
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

	/*
	 * initiate [6]
	 */
	cosem_error_initiate_other = (6 << 8),
	cosem_error_initiate_dlms_version_too_low,
	cosem_error_initiate_incompatible_conformance,
	cosem_error_initiate_pdu_size_too_short,
	cosem_error_initiate_refused_by_the_vde_handler,
};

enum cosem_state_error_t
{
	cosem_state_error_service_not_allowed = 1,
	cosem_state_error_service_unknown     = 2,
};

enum cosem_service_error_t
{
	cosem_service_error_operation_not_possible = 1,
	cosem_service_error_service_not_supported  = 2,
	cosem_service_error_other_reason           = 3,
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

struct cosem_longname_t
{
	union {
		struct {
			unsigned char A : 1;
			unsigned char D : 7;
			unsigned char C;
			unsigned char E;
			unsigned char F;
		};

		unsigned int value;
	};
};

struct cosem_key_t
{
	unsigned char bytes[16];
	unsigned int length;
};

struct cosem_attribute_descriptor_t
{
	unsigned short class_id;
	struct cosem_longname_t instance_id;
	signed char    attribute_id;
};

struct cosem_method_descriptor_t
{
	unsigned short class_id;
	struct cosem_longname_t instance_id;
	signed char    method_id;
};

struct cosem_method_descripor_t
{
	unsigned short class_id;
	unsigned char  instance_id[4];
	signed char    method_id;
};

enum application_context_name_t
{
	application_context_long_names_clear_text = 0x01,
};

struct acse_requirements_t
{
	unsigned short value;
};

enum mechanism_name_t
{
	cosem_lowest_level_security,
	cosem_low_level_security,
	cosem_high_level_security,
	cosem_high_level_security_md5,
	cosem_high_level_security_sha1,
	cosem_high_level_security_gmac,
	cosem_high_level_security_sha256,
	cosem_high_level_security_ecdsa,
};

struct authentication_value_t
{
	unsigned char bytes[16];
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

struct confirmed_service_error_t
{
	enum cosem_error_service_t        service;
	enum cosem_error_t                error;
};

enum association_result_t
{
	association_result_accepted = 0,
	association_result_rejected_permanent = 1,
	association_result_rejected_transient = 2,
};

enum acse_service_user_t
{
	acse_service_user_no_error = -1,
	acse_service_user_null = 0,
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
	acse_service_provider_no_error = -1,
	acse_service_provider_null = 0,
	acse_service_provider_no_reason_given,
	no_common_acse_version,
};

enum data_access_result_t
{
	access_result_success                     = 0,
	access_result_hardware_fault              = 1,
	access_result_temporary_failure           = 2,
	access_result_read_write_denied           = 3,
	access_result_object_undefined            = 4,
	access_result_object_class_inconsistent   = 9,
	access_result_object_unavailable          = 11,
	access_result_type_unmatched              = 12,
	access_result_scope_of_access_violated    = 13,
	access_result_data_block_unavailable      = 14,
	access_result_long_get_aborted            = 15,
	access_result_no_long_get_in_progress     = 16,
	access_result_long_set_aborted            = 17,
	access_result_no_long_set_in_progres      = 18,
	access_result_data_block_number_invalid   = 19,
	access_result_other_reason                = 250,
};

enum action_result_t
{
	action_result_success                     = 0,
	action_result_hardware_fault              = 1,
	action_result_temporary_failure           = 2,
	action_result_read_write_denied           = 3,
	action_result_object_undefined            = 4,
	action_result_object_class_inconsistent   = 9,
	action_result_object_unavailable          = 11,
	action_result_type_unmatched              = 12,
	action_result_scope_of_access_violated    = 13,
	action_result_data_block_unavailable      = 14,
	action_result_long_action_aborted         = 15,
	action_result_no_long_action_in_progress  = 16,
	action_result_other_reason                = 250
};

enum get_request_type_t
{
	get_request_normal_type = 1,
	get_request_next_type = 2,
	get_request_with_list_type = 3,
};

enum get_response_type_t
{
	get_response_normal_type = 1,
	get_response_with_datablock = 2,
	get_response_with_list = 3,
};

enum set_request_type_t
{
	set_request_normal                        = 1,
	set_request_with_first_datablock          = 2,
	set_request_with_datablock                = 3,
	set_request_with_list                     = 4,
	set_request_with_list_and_first_datablock = 5,
};

enum set_response_type_t
{
	set_response_normal                       = 1,
	set_response_datablock                    = 2,
	set_response_last_datablock               = 3,
	set_response_last_datablock_with_list     = 4,
	set_response_with_list                    = 5,
};

enum action_request_type_t
{
	action_request_normal_type                = 1,
	action_request_next_pblock                = 2,
	action_request_with_list                  = 3,
	action_request_with_first_pblock          = 4,
	action_request_with_list_and_first_pblock = 5,
	action_request_with_pblock                = 6,
};

enum action_response_type_t
{
	action_response_normal_type               = 1,
	action_response_with_pblock               = 2,
	action_response_with_list                 = 3,
	action_response_next_pblock               = 4,
};

struct get_request_normal_t
{
	struct invoke_id_and_priority_t     invoke_id_and_priority;
	struct cosem_attribute_descriptor_t cosem_attribute_descriptor;
};

struct get_request_next_t
{
	struct invoke_id_and_priority_t     invoke_id_and_priority;
	unsigned int                        block_number;
};

struct get_request_t
{
	enum get_request_type_t type;
	union {
		struct get_request_normal_t get_request_normal;
		struct get_request_next_t   get_request_next;
	};
};

struct get_response_normal_t
{
	struct invoke_id_and_priority_t invoke_id_and_priority;
	enum data_access_result_t       result;
};

struct get_response_with_datablock_t
{
	struct invoke_id_and_priority_t invoke_id_and_priority;
	unsigned char                   last_block;
	unsigned int                    block_number;
	enum data_access_result_t       result;
};

struct get_response_t
{
	enum get_response_type_t type;
	union {
		struct get_response_normal_t get_response_normal;
		struct get_response_with_datablock_t get_response_with_datablock;
	};
};

struct set_request_normal_t
{
	struct invoke_id_and_priority_t invoke_id_and_priority;
	struct cosem_attribute_descriptor_t cosem_attribute_descriptor;
};

struct set_response_normal_t
{
	struct invoke_id_and_priority_t invoke_id_and_priority;
	enum data_access_result_t       result;
};

struct set_request_t
{
	enum set_request_type_t type;
	union {
		struct set_request_normal_t set_request_normal;
	};
};

struct set_response_t
{
	enum set_response_type_t type;
	union {
		struct set_response_normal_t set_response_normal;
	};
};

struct action_request_normal_t
{
	struct invoke_id_and_priority_t  invoke_id_and_priority;
	struct cosem_method_descriptor_t cosem_method_descriptor;
};

struct action_response_normal_t
{
	struct invoke_id_and_priority_t  invoke_id_and_priority;
	enum action_result_t             result;
};

struct action_request_t
{
	enum action_request_type_t type;
	union {
		struct action_request_normal_t action_request_normal;
	};
};

struct action_response_t
{
	enum action_response_type_t type;
	union {
		struct action_response_normal_t action_response_normal;
	};
};

struct aarq_t
{
	unsigned int has_protocol_version : 1;
	unsigned int has_application_context_name : 1;
	unsigned int has_mechanism_name : 1;
	unsigned int has_calling_authentication : 1;
	unsigned int has_acse_requirements : 1;
	unsigned int has_initiate_request : 1;

	enum application_context_name_t   application_context_name;
	enum mechanism_name_t             mechanism_name;
	struct authentication_value_t     calling_authentication;
	struct initiate_request_t         initiate_request;
	unsigned short                    protocol_version;
	unsigned short                    acse_requirements;

	enum spodes_access_level_t        spodes_access_level;
};

union association_information_t
{
	struct initiate_response_t        initiate_response;
	struct confirmed_service_error_t  confirmed_service_error;
};

struct aare_t
{
	unsigned int has_protocol_version;
	unsigned int has_acse_requirements : 1;
	unsigned int has_mechanism_name : 1;
	unsigned int has_responding_authentication_value : 1;
	unsigned int has_confirmed_service_error : 1;

	unsigned short                    protocol_version;                // 80
	enum application_context_name_t   application_context_name;        // A1
	enum association_result_t         association_result;              // A2
	enum acse_service_user_t          acse_service_user;               // A3
	enum acse_service_provider_t      acse_service_provider;           // A3
	unsigned short                    acse_requirements;               // 88
	enum mechanism_name_t             mechanism_name;                  // 89
	struct authentication_value_t     responding_authentication_value; // AA
	union association_information_t   user_information;                // BE
};

#endif /* COSEM_TYPES_H_ */
