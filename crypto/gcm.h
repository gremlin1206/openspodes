/*
 * GCM functions
 * Copyright (c) 2003-2006, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef GCM_H
#define GCM_H

int aes_gcm_ae(const u8 *key, size_t key_len, const u8 *iv, size_t iv_len,
	       const u8 *plain, size_t plain_len,
	       const u8 *aad, size_t aad_len, u8 *crypt, u8 *tag);

int aes_gcm_ad(const u8 *key, size_t key_len, const u8 *iv, size_t iv_len,
	       const u8 *crypt, size_t crypt_len,
	       const u8 *aad, size_t aad_len, const u8 *tag, u8 *plain);

int aes_gmac(const u8 *key, size_t key_len, const u8 *iv, size_t iv_len,
	     const u8 *aad, size_t aad_len, u8 *tag);


#endif /* GCM_H */
