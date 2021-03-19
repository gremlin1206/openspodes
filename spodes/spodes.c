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

#include <stdio.h>

#include <hdlc/frame.h>

#include "spodes.h"

int spodes_client_address_to_access_level(struct hdlc_address_t *address)
{
	if (address->len != 1) {
		printf("spodes_client_address_to_access_level: invalid address length %u\n", address->len);
		return -1;
	}

	switch (address->upper) {
	case 16:
		return spodes_access_level_public;

	case 32:
		return spodes_access_level_reader;

	case 48:
		return spodes_access_level_configurator;

	default:
		printf("spodes_client_address_to_access_level: unknown client address %u\n", address->upper);
		return -1;
	}
}
