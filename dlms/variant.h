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

enum dlms_data_type_t
{
	dlms_null                 = 0,
	dlms_boolean              = 3,
	dlms_bit_string           = 4,
	dlms_double_long          = 5,
	dlms_double_long_unsigned = 6,
	dlms_octet_string         = 9,
	dlms_visible_string       = 10,
	dlms_utf8_string          = 12,
	dlms_bcd                  = 13,
	dlms_integer              = 15,
	dlms_long                 = 16,
	dlms_unsigned             = 17,
	dlms_long_unsigned        = 18,
	dlms_long64               = 20,
	dlms_long64_unsigned      = 21,
	dlms_enum                 = 22,
	dlms_float32              = 23,
	dlms_float64              = 24,
	dlms_date_time            = 25,
	dlms_date                 = 26,
	dlms_time                 = 27,

	dlms_array                = 1,
	dlms_structure            = 2,
	dlms_compact_array        = 19,
};

typedef unsigned short dlms_long_unsigned_t;
typedef unsigned char dlms_unsigned_t;

struct dlsm_variant_t
{
	enum dlms_data_type_t type;

	union {
		unsigned char      unsigned8;
		signed char        integer8;
		unsigned short     unsigned16;
		short              integer16;
		unsigned int       unsigned32;
		int                integer32;
		unsigned long long unsigned64;
		long long          integer64;

		struct {
			void *data;
			unsigned int length;
		} buffer;
	};
};

int dlms_variant_encode(unsigned char *buffer, const struct dlsm_variant_t *variant);

#endif /* DLMS_VARIANT_H_ */
