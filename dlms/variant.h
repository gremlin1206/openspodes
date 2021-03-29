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

#ifndef DLMS_VARIANT_H_
#define DLMS_VARIANT_H_

#include <dlms/data.h>

typedef struct
{
	enum dlms_data_type_t type;

	union {
		dlms_integer_t        integer_value;
		dlms_unsigned_t       unsigned_value;
		dlms_enum_t           enum_value;
		dlms_long_unsigned_t  long_unsigned_value;

		struct {
			unsigned char *string;
			unsigned int length;
		} octet_string_value;
	};
} dlms_variant_t;

int dlms_put_variant(const dlms_variant_t *variant, struct cosem_pdu_t *output);

#endif /* DLMS_VARIANT_H_ */
