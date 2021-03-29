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

#ifndef COSEM_SPODES_H_
#define COSEM_SPODES_H_

struct hdlc_address_t;

enum spodes_access_level_t
{
	spodes_access_level_public       = 0,
	spodes_access_level_reader       = 1,
	spodes_access_level_configurator = 2,
};

enum spodes_association_t
{
	spodes_association_current,
	spodes_association_guest,
	spodes_association_reader,
	spodes_association_configurator,
};

struct spodes_device_logical_name_t
{
	unsigned char data[16];
	unsigned char length;
};

int spodes_client_address_to_access_level(struct hdlc_address_t *address);

#endif /* COSEM_SPODES_H_ */
