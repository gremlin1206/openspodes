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

#include "objects.h"
#include "association.h"

static struct cosem_object_t *cosem_find_abstract_object(struct cosem_longname_t name)
{
	switch (name.C)
	{
	case 40: // Instances of IC “Association SN/LN”
		return cosem_association_get_object(name);

	default:
		return 0;
	}
}

struct cosem_object_t *cosem_find_object_by_name(struct cosem_longname_t name)
{
	if (name.A == 0)
		return cosem_find_abstract_object(name);
	else
		return 0; //return cosem_find_energy_object(name);
}

int cosem_object_get_attribute(struct get_request_t *request, struct get_response_t *response)
{
	const struct cosem_class_t *cosem_class = request->object->cosem_class;

	printf("cosem_object_get_attribute\n");

	if (request->get_request_normal.cosem_attribute_descriptor.class_id != cosem_class->id) {
		printf("cosem_object_get_attribute: object class inconsistent\n");
		response->data_access_result = access_result_object_class_inconsistent;
		return 0;
	}

	if (cosem_class->get == 0) {
		printf("cosem_object_get_attribute: method get not implemented\n");
		response->data_access_result = access_result_scope_of_access_violated;
		return 0;
	}

	return cosem_class->get(request, response);
}

int cosem_object_action(struct action_request_t *request, struct action_response_t *response)
{
	const struct cosem_class_t *cosem_class = request->object->cosem_class;

	printf("cosem_object_action\n");

	if (request->action_request_normal.cosem_method_descriptor.class_id != cosem_class->id) {
		printf("cosem_object_get_attribute: object class inconsistent\n");
		response->result = action_result_object_class_inconsistent;
		return 0;
	}

	if (cosem_class->get == 0) {
		printf("cosem_object_get_attribute: method get not implemented\n");
		response->result = action_result_scope_of_access_violated;
		return 0;
	}

	return cosem_class->action(request, response);
}
