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

#ifndef DLMS_CLASS_LIMITER_H_
#define DLMS_CLASS_LIMITER_H_

#include <dlms/class.h>
#include <dlms/object.h>

/*
 * Class ID 71
 */

enum class_limiter_attribute_t
{
	limiter_zero_attribute,

	limiter_logical_name,

	limiter_monitored_value,
	limiter_threshold_active,
	limiter_threshold_normal,
	limiter_threshold_emergency,
	limiter_min_over_threshold_duration,
	limiter_min_under_threshold_duration,
	limiter_emergency_profile,
	limiter_emergency_profile_group_id_list,
	limiter_emergency_profile_active,
	limiter_actions,
};

struct cosem_limiter_object_t
{
	struct cosem_object_t base; // must be the first struct member
};

extern const struct cosem_class_t class_limiter;

#endif /* DLMS_CLASS_LIMITER_H_ */
