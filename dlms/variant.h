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
