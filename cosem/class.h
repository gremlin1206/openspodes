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

#ifndef COSEM_CLASS_H_
#define COSEM_CLASS_H_

#include "types.h"

typedef int (*cosem_get_attribute_t)(struct cosem_class_t *cosem_class, void *data, struct cosem_pdu_t *pdu);
//typedef int cosem_set_attribute(int attribute);
//typedef int cosem_call_method(int method);

struct cosem_class_t
{
	int id;
	cosem_get_attribute_t get;
};

int cosem_class_get_attribute(struct cosem_class_t *cosem_class, void *data, struct cosem_pdu_t *pdu);

#endif /* COSEM_CLASS_H_ */
