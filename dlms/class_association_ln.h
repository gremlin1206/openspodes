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

#ifndef COSEM_CLASS_ASSOCIATION_LN_H_
#define COSEM_CLASS_ASSOCIATION_LN_H_

#include <dlms/class.h>
#include <dlms/object.h>

/*
 * Class ID 15
 */

enum class_association_ln_attribute_t
{
	association_ln_zero_attribute,

	association_ln_logical_name,
	association_ln_object_list,
	association_ln_associated_partners_id,
	association_ln_application_context_name,
	association_ln_dlms_context_info,
	association_ln_authentication_mechanism_name,
	association_ln_secret,
	association_ln_association_status,
	association_ln_secret_security_setup_reference,
	association_ln_user_list,
	association_ln_current_user,
};

enum class_association_ln_method_t
{
	association_ln_zero_method,

	association_ln_reply_to_hls_authentication,
	association_ln_change_hls_secret,
	association_ln_add_object,
	association_ln_remove_object,
	association_ln_add_user,
	association_ln_remove_user,
};

struct cosem_association_ln_object_t
{
	struct cosem_object_t base; // must be the first struct member
};

extern const struct cosem_class_t class_association_ln;

#endif /* COSEM_CLASS_ASSOCIATION_LN_H_ */
