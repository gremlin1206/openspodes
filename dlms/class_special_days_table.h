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

#ifndef DLMS_CLASS_SPECIAL_DAYS_TABLE_H_
#define DLMS_CLASS_SPECIAL_DAYS_TABLE_H_

#include <dlms/class.h>
#include <dlms/object.h>

/*
 * Class ID 8
 */

enum class_special_days_table_attribute_t
{
	special_days_tablezero_attribute,

	special_days_table_logical_name,

	special_days_table_entries,
};

enum class_special_days_table_method_t
{
	special_days_table_zero_method,

	special_days_table_insert,
	special_days_table_delete,
};

struct cosem_special_days_table_object_t
{
	struct cosem_object_t base; // must be the first struct member
};

extern const struct cosem_class_t class_special_days_table;

#endif /* DLMS_CLASS_SPECIAL_DAYS_TABLE_H_ */
