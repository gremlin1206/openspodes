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

#include <dlms/class.h>
#include <dlms/object.h>

#ifndef DLMS_CLASS_PROFILE_GENERIC_H_
#define DLMS_CLASS_PROFILE_GENERIC_H_

/*
 * Class ID 7
 */

enum class_profile_generic_attribute_t
{
	profile_generic_zero_attribute,

	profile_generic_logical_name,
	profile_generic_buffer,
	profile_generic_capture_objects,
	profile_generic_capture_period,
	profile_generic_sort_method,
	profile_generic_sort_object,
	profile_generic_entries_in_use,
	profile_generic_profile_entries,
};

enum class_profile_generic_method_t
{
	profile_generic_zero_method,

	profile_generic_reset,
	profile_generic_capture,
	profile_generic_get_buffer_by_range,
	profile_generic_get_buffer_by_index,
};

struct cosem_profile_generic_object_t
{
	struct cosem_object_t base; // must be the first struct member
};

extern const struct cosem_class_t class_profile_generic;

#endif /* DLMS_CLASS_PROFILE_GENERIC_H_ */
