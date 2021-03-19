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

#include <crypto/aes.h>

#include "association.h"
#include "pdu.h"
#include "cosem.h"

#include "class_association_ln.h"

static void cosem_reject_association_user(struct aare_t *aare, enum acse_service_user_t service_user)
{
	aare->association_result = association_result_rejected_permanent;
	aare->acse_service_user = service_user;
	aare->acse_service_provider = acse_service_provider_no_error;
}

static void cosem_reject_association_provider(struct aare_t *aare, enum acse_service_provider_t service_provider)
{
	aare->association_result = association_result_rejected_permanent;
	aare->acse_service_user = acse_service_user_no_error;
	aare->acse_service_provider = service_provider;
}

static int cosem_lowest_level_security_authentication(struct cosem_ctx_t *ctx, struct cosem_association_t *a, struct aarq_t *aarq, struct aare_t *aare)
{
	printf("cosem_lowest_level_security_authentication\n");
	a->authenticated = 1;
	return 0;
}

static int cosem_validate_low_level_security_authentication(struct cosem_ctx_t *ctx, struct cosem_association_t *a, struct aarq_t *aarq, struct aare_t *aare)
{
	int i;
	int sum;

	if (!aarq->has_calling_authentication) {
		return -1;
	}

	sum = 0;
	for (i = 0; i < 16; i++) {
		sum += ctx->lls_auth_key.bytes[i] ^ aarq->calling_authentication.bytes[i];
	}

	if (sum != 0) {
		printf("cosem_validate_low_level_security_authentication: wrong password provided\n");
		return -1;
	}

	a->authenticated = 1;

	return 0;
}

static int cosem_high_level_security_authentication_stage1(struct cosem_ctx_t *ctx, struct cosem_association_t *a, struct aarq_t *aarq, struct aare_t *aare)
{
	printf("cosem_high_level_security_authentication_stage1\n");

	if (!aarq->has_calling_authentication || aarq->calling_authentication.length != 16) {
		return -1;
	}

	a->ctos_challenge = aarq->calling_authentication;

	// TODO: generate stoc_challenge using RNG
	//
	memset(a->stoc_challenge.bytes, '1', 16);
	a->stoc_challenge.length = 16;

	aare->has_acse_requirements = 1;
	aare->acse_requirements.value = 0x0780;

	aare->has_mechanism_name = 1;
	aare->mechanism_name = aarq->mechanism_name;

	printf("mechanism requested: %02X\n", aarq->mechanism_name);

	aare->has_responding_authentication_value = 1;
	aare->responding_authentication_value = a->stoc_challenge;

	aare->acse_service_user = acse_service_user_authentication_required;

	return 0;
}

int cosem_association_high_level_security_authentication_stage2(struct cosem_ctx_t *ctx, struct cosem_association_t *a, const unsigned char *fStoC_input, unsigned char *fCtoS_output)
{
	unsigned int i;
	void *aes;
	unsigned char fStoC_input_check[16];
	int cmp;

	printf("cosem_association_high_level_security_authentication_stage2\n");

	printf("CtoS: ");
	for (i = 0; i < 16; i++) {
		printf("%02X ", a->ctos_challenge.bytes[i]);
	}
	printf("\n");

	aes = aes_encrypt_init(ctx->hls_auth_key.bytes, ctx->hls_auth_key.length);

	aes_encrypt(aes, a->stoc_challenge.bytes, fStoC_input_check);

	cmp = 0;
	for (i = 0; i < 16; i++)
		cmp += fStoC_input_check[i] ^ fStoC_input[i];

	if (cmp == 0)
		aes_encrypt(aes, a->ctos_challenge.bytes, fCtoS_output);

	aes_encrypt_deinit(aes);

	if (cmp != 0) {
		printf("authentication failed: wrong fStoC\n");
		return -1;
	}

	printf("fCtoS: ");
	for (i = 0; i < 16; i++) {
		printf("%02X ", fCtoS_output[i]);
	}
	printf("\n");

	a->authenticated = 1;

	return 0;
}

static int cosem_authenticate_association(struct cosem_ctx_t *ctx, struct cosem_association_t *a, struct aarq_t *aarq, struct aare_t *aare)
{
	int ret;

	switch (aarq->mechanism_name)
	{
	case cosem_lowest_level_security:
		ret = cosem_lowest_level_security_authentication(ctx, a, aarq, aare);
		break;

	case cosem_low_level_security:
		ret = cosem_validate_low_level_security_authentication(ctx, a, aarq, aare);
		break;

	case cosem_high_level_security:
		ret = cosem_high_level_security_authentication_stage1(ctx, a, aarq, aare);
		break;

	//case cosem_high_level_security_gmac:

	default:
		printf("invalid mechanism_name %i\n", aarq->mechanism_name);
		cosem_reject_association_user(aare, acse_service_user_authentication_mechanism_name_not_recognised);
		return -1;
	}

	if (ret < 0) {
		cosem_reject_association_user(aare, acse_service_user_authentication_failure);
		return ret;
	}

	return 0;
}

static int spodes_check_aarq(struct cosem_ctx_t *ctx, struct cosem_association_t *a, struct aarq_t *aarq, struct aare_t *aare)
{
	int spodes_access_level;

	if (aarq->has_protocol_version && aarq->protocol_version != 0x0284) {
		printf("wrong protocol version: %04X\n", aarq->protocol_version);
		cosem_reject_association_provider(aare, no_common_acse_version);
		return -1;
	}

	spodes_access_level = spodes_client_address_to_access_level(&aarq->client_address);
	if (spodes_access_level < 0) {
		cosem_reject_association_user(aare, acse_service_user_no_reason_given);
		return spodes_access_level;
	}

	if (spodes_access_level >= spodes_access_level_reader) {
		if (!aarq->has_acse_requirements || aarq->acse_requirements != 0x0780) {
			cosem_reject_association_user(aare, acse_service_user_no_reason_given);
			return -1;
		}

		if (!aarq->has_mechanism_name) {
			cosem_reject_association_user(aare, acse_service_user_authentication_mechanism_name_required);
			return -1;
		}

		if (spodes_access_level == spodes_access_level_reader && aarq->mechanism_name != cosem_low_level_security) {
			cosem_reject_association_user(aare, acse_service_user_authentication_mechanism_name_not_recognised);
			return -1;
		}

		if (spodes_access_level == spodes_access_level_configurator && aarq->mechanism_name != cosem_high_level_security) {
			cosem_reject_association_user(aare, acse_service_user_authentication_mechanism_name_not_recognised);
			return -1;
		}

		if (!aarq->has_calling_authentication) {
			cosem_reject_association_user(aare, acse_service_user_authentication_failure);
			return -1;
		}
	}

	a->spodes_access_level = spodes_access_level;

	return 0;
}

int cosem_association_open(struct cosem_ctx_t *ctx, struct cosem_association_t *a, struct aarq_t *aarq, struct aare_t *aare)
{
	int ret;
	struct initiate_response_t *initiate_response = &aare->user_information.initiate_response;

	memset(aare, 0, sizeof(*aare));


	aare->association_result       = association_result_accepted;
	aare->acse_service_user        = acse_service_user_null;
	aare->acse_service_provider    = acse_service_provider_no_error;
	aare->application_context_name = aarq->application_context_name;

	ret = spodes_check_aarq(ctx, a, aarq, aare);
	if (ret < 0)
		return 0;

	switch (aarq->application_context_name)
	{
	case application_context_long_names_clear_text:
		initiate_response->vaa_name = 7;
		break;

	default:
		printf("invalid application_context_name %i\n", aarq->application_context_name);
		cosem_reject_association_user(aare, acse_service_user_application_context_name_not_supported);
		return 0;
	}

	initiate_response->vaa_name = 7;

	ret = cosem_authenticate_association(ctx, a, aarq, aare);
	if (ret < 0) {
		printf("authentication failed\n");
		return 0;
	}

	initiate_response->negotiated_dlms_version_number = 6;

	initiate_response->negotiated_conformance.selective_access = 1;
	initiate_response->negotiated_conformance.set = 1;
	initiate_response->negotiated_conformance.get = 1;
	initiate_response->negotiated_conformance.block_transfer_with_get_or_read = 1;

	initiate_response->server_max_receive_pdu_size = DLMS_MAX_PDU_SIZE;

	a->associated = 1;
	a->mechanism_name = aarq->mechanism_name;
	a->calling_authentication = aarq->calling_authentication;
	a->negotiated_conformance = initiate_response->negotiated_conformance;

	printf("cosem_association_open: associated\n");

	return 0;
}

void cosem_association_close(struct cosem_ctx_t *ctx, struct cosem_association_t *a)
{
	a->associated = 0;
	a->authenticated = 0;
}

static struct cosem_association_ln_object_t cosem_object_current_association = {
		.base = {. cosem_class = (struct cosem_class_t*)&class_association_ln },
};

struct cosem_object_t *cosem_association_get_object(struct cosem_longname_t name)
{
	if (name.D != 0 || name.F != 255)
		return 0;

	switch (name.E)
	{
	case 0: // current association
		return (struct cosem_object_t*)&cosem_object_current_association;

	case 1: // guest client access
		break;

	case 2: // billing reader access
		break;

	case 3: // configurator/administrator access
		break;
	}

	return 0;
}
