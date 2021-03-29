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

#ifndef DLMS_CLASS_REGISTER_H_
#define DLMS_CLASS_REGISTER_H_

#include <dlms/class.h>
#include <dlms/object.h>
#include <dlms/variant.h>

enum dlms_physical_unit_t
{
	dlms_unit_year = 1,
	dlms_unit_month,
	dlms_unit_week,
	dlms_unit_dya,
	dlms_unit_hour,
	dlms_unit_min,
	dlms_unit_second,
	dlms_unit_degree,
	dlms_unit_degree_celcius,
	dlms_unit_metre,
	dlms_unit_metre_per_second,

	// TODO: complete the list
};

/*
 * Class ID 3
 */

enum class_register_attribute_t
{
	register_zero_attribute,

	register_logical_name,
	register_value,
	register_scaler_unit,
};

enum class_register_method_t
{
	register_zero_method,

	register_reset,
};

struct cosem_register_object_t
{
	struct cosem_object_t base; // must be the first struct member
	dlms_variant_t value;
	dlms_integer_t scaler;
	dlms_enum_t    unit;
};

extern const struct cosem_class_t class_register;

#endif /* DLMS_CLASS_REGISTER_H_ */
