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

#include <dlms/data.h>
#include <cosem/asn1.h>

static int dlms_put_uint8(unsigned char value, enum dlms_data_type_t type, struct cosem_pdu_t *output)
{
	int ret;

	ret = asn_put_uint8(value, output);
	if (ret < 0)
		return ret;

	return asn_put_uint8(type, output);
}

int dlms_put_integer(dlms_integer_t value, struct cosem_pdu_t *output)
{
	return dlms_put_uint8((unsigned char)value, dlms_integer, output);
}

int dlms_put_long_unsigned(dlms_long_unsigned_t value, struct cosem_pdu_t *output)
{
	int ret;

	ret = asn_put_uint16(value, output);
	if (ret < 0)
		return ret;

	return asn_put_uint8(dlms_long_unsigned, output);
}

int dlms_put_unsigned(dlms_unsigned_t value, struct cosem_pdu_t *output)
{
	return dlms_put_uint8((unsigned char)value, dlms_unsigned, output);
}

int dlms_put_enum(int value, struct cosem_pdu_t *output)
{
	return dlms_put_uint8((unsigned char)value, dlms_enum, output);
}

int dlms_put_structure(unsigned int members, struct cosem_pdu_t *output)
{
	return dlms_put_uint8((unsigned char)members, dlms_structure, output);
}

int dlms_put_array(unsigned int items, struct cosem_pdu_t *output)
{
	return dlms_put_uint8((unsigned char)items, dlms_array, output);
}

int dlms_put_octet_string(unsigned char *string, unsigned int length, struct cosem_pdu_t *output)
{
	return asn_put_octet_string(string, length, output);
}

int dlms_put_logical_name(struct cosem_longname_t logical_name, struct cosem_pdu_t *output)
{
	unsigned char buffer[6];

	buffer[0] = logical_name.A;
	buffer[1] = 0;
	buffer[2] = logical_name.C;
	buffer[3] = logical_name.D;
	buffer[4] = logical_name.E;
	buffer[5] = logical_name.F;

	return asn_put_octet_string(buffer, sizeof(buffer), output);
}
