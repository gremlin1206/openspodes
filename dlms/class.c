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

#include "class.h"

int encode_attribute_access_item(dlms_integer_t attribute_id, dlms_enum_t access_mode,
		                 dlms_integer_t *access_selectors, unsigned int access_selectors_count,
				 struct cosem_pdu_t *output)
{
	unsigned int i;
	int ret;

	/*
	 * access_selectors
	 */
	if (access_selectors_count == 0) {
		ret = dlms_put_null(output);
		if (ret < 0)
			return ret;
	}
	else {
		for (i = 0; i < access_selectors_count; i++) {
			ret = dlms_put_integer(access_selectors[i], output);
			if (ret < 0)
				return ret;
		}

		ret = dlms_put_array(access_selectors_count, output);
		if (ret < 0)
			return ret;
	}

	/*
	 * access_mode
	 */
	ret = dlms_put_enum(access_mode, output);
	if (ret < 0)
		return ret;

	/*
	 * attribute_id
	 */
	ret = dlms_put_integer(attribute_id, output);
	if (ret < 0)
		return ret;

	ret = dlms_put_structure(3, output);
	if (ret < 0)
		return ret;

	return 0;
}

int encode_method_access_item(dlms_integer_t method_id, dlms_enum_t access_mode, struct cosem_pdu_t *output)
{
	int ret;

	ret = dlms_put_enum(access_mode, output);
	if (ret < 0)
		return ret;

	ret = dlms_put_integer(method_id, output);
	if (ret < 0)
		return ret;

	ret = dlms_put_structure(2, output);
	if (ret < 0)
		return ret;

	return 0;
}
