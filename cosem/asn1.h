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

#ifndef COSEM_ASN1_H_
#define COSEM_ASN1_H_

int asn_get_length(unsigned int *out, const unsigned char **buffer, unsigned int *len);
int asn_get_uint16(unsigned short *out, const unsigned char **buffer, unsigned int *length);
int asn_put_uint16(unsigned char *buffer, unsigned short value);
int asn_get_uint8(unsigned char *out, const unsigned char **buffer, unsigned int *length);
int asn_get_buffer(void *out, unsigned int bytes, const unsigned char **buffer, unsigned int *length);

#endif /* COSEM_ASN1_H_ */
