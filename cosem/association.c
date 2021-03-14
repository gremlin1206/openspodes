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

#include "association.h"
#include "pdu.h"
#include "class_association_ln.h"

static void cosem_reject_association(struct aarq_t *aarq, struct aare_t *aare, enum acse_service_user_t acse_service_user)
{
	aare->application_context_name = aarq->application_context_name;
	aare->association_result = association_result_rejected_permanent;
	aare->acse_service_user = acse_service_user;
}

static int cosem_validate_lls_password(struct cosem_ctx_t *ctx, struct authentication_value_t *auth)
{
	return 0;
}

int cosem_association_open(struct cosem_ctx_t *ctx, struct cosem_association_t *a, struct aarq_t *aarq, struct aare_t *aare)
{
	int ret;

	memset(aare, 0, sizeof(*aare));

	switch (aarq->application_context_name)
	{
	case application_context_long_names_clear_text:
		aare->initiate_response.vaa_name = 7;
		break;

	default:
		printf("invalid application_context_name %i\n", aarq->application_context_name);
		cosem_reject_association(aarq, aare, acse_service_user_application_context_name_not_supported);
		return 0;
	}

	switch (aarq->mechanism_name)
	{
	case cosem_lowest_level_security:
		break;

	case cosem_low_level_security:
		ret = cosem_validate_lls_password(ctx, &aarq->calling_authentication);
		if (ret < 0) {
			cosem_reject_association(aarq, aare, acse_service_user_authentication_failure);
			return 0;
		}
		break;

	case cosem_high_level_security:
		break;

	case cosem_high_level_security_gmac:
		break;

	default:
		printf("invalid mechanism_name %i\n", aarq->mechanism_name);
		cosem_reject_association(aarq, aare, acse_service_user_authentication_mechanism_name_not_recognised);
		return 0;
	}

	aare->association_result = association_result_accepted;
	aare->acse_service_user = acse_service_user_null;
	aare->acse_service_provider = -1;

	aare->application_context_name = aarq->application_context_name;

	aare->initiate_response.negotiated_dlms_version_number = 6;

	aare->initiate_response.negotiated_conformance.selective_access = 1;
	aare->initiate_response.negotiated_conformance.set = 1;
	aare->initiate_response.negotiated_conformance.get = 1;
	aare->initiate_response.negotiated_conformance.block_transfer_with_get_or_read = 1;

	aare->initiate_response.server_max_receive_pdu_size = DLMS_MAX_PDU_SIZE;

	a->associated = 1;
	a->mechanism_name = aarq->mechanism_name;
	a->calling_authentication = aarq->calling_authentication;
	a->negotiated_conformance = aare->initiate_response.negotiated_conformance;

	printf("cosem_association_open: associated\n");

	return 0;
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
