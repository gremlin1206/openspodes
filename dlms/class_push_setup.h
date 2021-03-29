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

#ifndef DLMS_CLASS_PUSH_SETUP_H_
#define DLMS_CLASS_PUSH_SETUP_H_

#include <dlms/class.h>
#include <dlms/object.h>

/*
 * Class ID 40
 */

enum class_push_setup_attribute_t
{
	push_setup_zero_attribute,

	push_setup_logical_name,

	push_setup_push_object_list,
	push_setup_send_destination_and_method,
	push_setup_communication_window,
	push_setup_randomisation_start_interval,
	push_setup_number_of_retries,
	push_setup_repetition_delay,
};

enum class_push_setup_method_t
{
	push_setup_zero_method,

	push_setup_push,
};

struct cosem_push_setup_object_t
{
	struct cosem_object_t base; // must be the first struct member
};

extern const struct cosem_class_t class_push_setup;

#endif /* DLMS_CLASS_PUSH_SETUP_H_ */
