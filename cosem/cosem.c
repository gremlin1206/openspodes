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

#include <dlms/dlms.h>

#include "cosem.h"

#if 0
static void cosem_append_llc_response(struct dlms_pdu_t *pdu)
{
	unsigned char *p = &pdu->data[pdu->length];

	pdu->length += 3;
	p[0] = LLC_REMOTE_LSAP;
	p[1] = LLC_LOCAL_LSAP_RESPONSE;
	p[3] = 0;
}

static void cosem_append_error(struct dlms_pdu_t *pdu, enum cosem_error_service_t service, enum cosem_error_t error)
{
	unsigned char *p = &pdu->data[pdu->length];

	pdu->length += 4;
	p[0] = apdu_tag_confirmed_service_error;
	p[1] = (unsigned char)service;
	p[2] = (unsigned char)(error >> 8);
	p[3] = (unsigned char)(error & 0xFF);
}

static void cosem_append_exception(struct dlms_pdu_t *pdu)
{
	unsigned char *p = &pdu->data[pdu->length];

	pdu->length += 4;
	p[0] = apdu_tag_exception_response;
}
#endif

int cosem_process_aa_request(struct cosem_ctx_t *ctx, struct aarq_t *aarq, struct aare_t *aare)
{
/*
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
 */

	aare->application_context_name = aarq->application_context_name;

	aare->association_result = association_result_accepted;

	aare->acse_service_user = 0;
	aare->acse_service_provider = -1;

	aare->initiate_response.negotiated_dlms_version_number = 6;

	aare->initiate_response.negotiated_conformance.selective_access = 1;
	aare->initiate_response.negotiated_conformance.set = 1;
	aare->initiate_response.negotiated_conformance.get = 1;
	aare->initiate_response.negotiated_conformance.block_transfer_with_get_or_read = 1;

	aare->initiate_response.server_max_receive_pdu_size = 0x400;

	aare->initiate_response.vaa_name = 7;

	return 0;
}

int cosem_process_get_request(struct cosem_ctx_t *ctx, struct dlms_pdu_t *pdu, const struct get_request_t *request)
{
	pdu->length = 0;

	//cosem_append_llc_response(pdu);
	//cosem_append_error(pdu, cosem_error_read, cosem_error_access_object_access_violated);

	return -1;
}
