/*
 *  Multi-AP Platform Crypto
 *  Advanced Encryption Standard (AES)
 *
 *  Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 *
 */

#include <string.h>
#include <stdio.h>

#include "openssl/evp.h"
#include "openssl/cmac.h"
#include "openssl/aes.h"

#include "platform_crypto_aes.h"

////////////////////////////////////////////////////////////////////////////////
// Private data and functions
////////////////////////////////////////////////////////////////////////////////

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
static INT8U _aes_encrypt(const EVP_CIPHER *cipher, const INT8U *key, const INT8U *iv, INT8U *data, INT32U data_len)
{
	EVP_CIPHER_CTX *ctx;

	int   clen, len;
	INT8U buf[AES_BLOCK_SIZE];

	ctx = EVP_CIPHER_CTX_new();
	if (NULL == ctx) {
		return 0;
	}
	if (EVP_EncryptInit_ex(ctx, cipher, NULL, key, iv) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return 0;
	}
	EVP_CIPHER_CTX_set_padding(ctx, 0);

	clen = data_len;
	if (EVP_EncryptUpdate(ctx, data, &clen, data, data_len) != 1 || clen != (int)data_len) {
		EVP_CIPHER_CTX_free(ctx);
		return 0;
	}

	len = sizeof(buf);
	if (EVP_EncryptFinal_ex(ctx, buf, &len) != 1 || len != 0) {
		EVP_CIPHER_CTX_free(ctx);
		return 0;
	}
	EVP_CIPHER_CTX_free(ctx);

	return 1;
}

static INT8U _aes_decrypt(const EVP_CIPHER *cipher, const INT8U *key, const INT8U *iv, INT8U *data, INT32U data_len)
{
	EVP_CIPHER_CTX *ctx;

	int   plen, len;
	INT8U buf[AES_BLOCK_SIZE];

	ctx = EVP_CIPHER_CTX_new();
	if (NULL == ctx) {
		return 0;
	}
	if (EVP_DecryptInit_ex(ctx, cipher, NULL, key, iv) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return 0;
	}
	EVP_CIPHER_CTX_set_padding(ctx, 0);

	plen = data_len;
	if (EVP_DecryptUpdate(ctx, data, &plen, data, data_len) != 1 || plen != (int)data_len) {
		EVP_CIPHER_CTX_free(ctx);
		return 0;
	}

	len = sizeof(buf);
	if (EVP_DecryptFinal_ex(ctx, buf, &len) != 1 || len != 0) {
		EVP_CIPHER_CTX_free(ctx);
		return 0;
	}
	EVP_CIPHER_CTX_free(ctx);

	return 1;
}
#else
static INT8U _aes_encrypt(const EVP_CIPHER *cipher, const INT8U *key, const INT8U *iv, INT8U *data, INT32U data_len)
{

	EVP_CIPHER_CTX ctx;

	int   clen, len;
	INT8U buf[AES_BLOCK_SIZE];

	EVP_CIPHER_CTX_init(&ctx);
	if (EVP_EncryptInit_ex(&ctx, cipher, NULL, key, iv) != 1) {
		return 0;
	}
	EVP_CIPHER_CTX_set_padding(&ctx, 0);

	clen = data_len;
	if (EVP_EncryptUpdate(&ctx, data, &clen, data, data_len) != 1 || clen != (int)data_len) {
		return 0;
	}

	len = sizeof(buf);
	if (EVP_EncryptFinal_ex(&ctx, buf, &len) != 1 || len != 0) {
		return 0;
	}
	EVP_CIPHER_CTX_cleanup(&ctx);

	return 1;
}

static INT8U _aes_decrypt(const EVP_CIPHER *cipher, const INT8U *key, const INT8U *iv, INT8U *data, INT32U data_len)
{
	EVP_CIPHER_CTX ctx;

	int   plen, len;
	INT8U buf[AES_BLOCK_SIZE];

	EVP_CIPHER_CTX_init(&ctx);
	if (EVP_DecryptInit_ex(&ctx, cipher, NULL, key, iv) != 1) {
		return 0;
	}
	EVP_CIPHER_CTX_set_padding(&ctx, 0);

	plen = data_len;
	if (EVP_DecryptUpdate(&ctx, data, &plen, data, data_len) != 1 || plen != (int)data_len) {
		return 0;
	}

	len = sizeof(buf);
	if (EVP_DecryptFinal_ex(&ctx, buf, &len) != 1 || len != 0) {
		return 0;
	}
	EVP_CIPHER_CTX_cleanup(&ctx);

	return 1;
}
#endif

static void _dbl(INT8U *pad)
{
	int i, carry;

	carry = pad[0] & 0x80;
	for (i = 0; i < AES_BLOCK_SIZE - 1; i++)
		pad[i] = (pad[i] << 1) | (pad[i + 1] >> 7);
	pad[AES_BLOCK_SIZE - 1] <<= 1;
	if (carry)
		pad[AES_BLOCK_SIZE - 1] ^= 0x87;
}

static void _xor(INT8U *a, const INT8U *b)
{
	int i;

	for (i = 0; i < AES_BLOCK_SIZE; i++)
		*a++ ^= *b++;
}

static void _xorend(INT8U *a, int alen, const INT8U *b, int blen)
{
	int i;

	if (alen < blen)
		return;

	for (i = 0; i < blen; i++)
		a[alen - blen + i] ^= b[i];
}

static void _pad_block(INT8U *pad, const INT8U *addr, int len)
{
	memset(pad, 0, AES_BLOCK_SIZE);
	memcpy(pad, addr, len);

	if (len < AES_BLOCK_SIZE)
		pad[len] = 0x80;
}

static INT8U _aes_s2v(const INT8U *key, INT32U key_len,
                      INT32U num_elem, const INT8U *addr[], INT32U *len, INT8U *mac)
{
	static const INT8U zero[AES_BLOCK_SIZE] = { 0 };
	INT8U              tmp[AES_BLOCK_SIZE], tmp2[AES_BLOCK_SIZE];
	INT8U *            buf = NULL;
	INT32U             i;
	const INT8U *      data[1];
	INT32U             data_len[1];
	INT8U              ret = 0;

	if (!num_elem) {
		memcpy(tmp, zero, sizeof(zero));
		tmp[AES_BLOCK_SIZE - 1] = 1;
		data[0]                 = tmp;
		data_len[0]             = sizeof(tmp);
		return PLATFORM_AES_CMAC(key, key_len, 1, data, data_len, mac);
	}

	data[0]     = zero;
	data_len[0] = sizeof(zero);
	ret         = PLATFORM_AES_CMAC(key, key_len, 1, data, data_len, tmp);
	if (!ret)
		return ret;

	for (i = 0; i < num_elem - 1; i++) {
		ret = PLATFORM_AES_CMAC(key, key_len, 1, &addr[i], &len[i],
		                        tmp2);
		if (!ret)
			return ret;

		_dbl(tmp);
		_xor(tmp, tmp2);
	}
	if (len[i] >= AES_BLOCK_SIZE) {
		buf = malloc(len[i]);
		if (!buf)
			return 0;

		memcpy(buf, addr[i], len[i]);
		_xorend(buf, len[i], tmp, AES_BLOCK_SIZE);
		data[0] = buf;
		ret     = PLATFORM_AES_CMAC(key, key_len, 1, data, &len[i], mac);
		memset(buf, 0, len[i]);
		free(buf);
		return ret;
	}

	_dbl(tmp);
	_pad_block(tmp2, addr[i], len[i]);
	_xor(tmp, tmp2);

	data[0]     = tmp;
	data_len[0] = sizeof(tmp);
	return PLATFORM_AES_CMAC(key, key_len, 1, data, data_len, mac);
}

////////////////////////////////////////////////////////////////////////////////////
// Platform API: Interface related functions to be used by platform-independent
// files (functions declarations are found in "../interfaces/platform_crypto_aes.h)
////////////////////////////////////////////////////////////////////////////////////

INT8U PLATFORM_AES_SIV_ENCRYPT(const INT8U *key, INT32U key_len,
                               const INT8U *pw, INT32U pwlen,
                               INT32U num_elem, const INT8U *addr[], const INT32U *len,
                               INT8U *out)
{
	const INT8U *_addr[6];
	INT32U       _len[6];
	const INT8U *k1, *k2;
	INT8U        v[AES_BLOCK_SIZE];
	INT32U       i;
	INT8U *      iv, *crypt_pw;

	if (num_elem > ARRAY_SIZE(_addr) - 1)
		return 0;

	key_len /= 2;
	k1 = key;
	k2 = key + key_len;

	for (i = 0; i < num_elem; i++) {
		_addr[i] = addr[i];
		_len[i]  = len[i];
	}
	_addr[num_elem] = pw;
	_len[num_elem]  = pwlen;

	if (!_aes_s2v(k1, key_len, num_elem + 1, _addr, _len, v))
		return 0;

	iv       = out;
	crypt_pw = out + AES_BLOCK_SIZE;

	memcpy(iv, v, AES_BLOCK_SIZE);
	memcpy(crypt_pw, pw, pwlen);

	/* zero out 63rd and 31st bits of ctr (from right) */
	v[8] &= 0x7f;
	v[12] &= 0x7f;
	return PLATFORM_AES_CTR_ENCRYPT(k2, key_len, v, crypt_pw, pwlen);
}

INT8U PLATFORM_AES_SIV_DECRYPT(const INT8U *key, INT32U key_len,
                               const INT8U *iv_crypt, INT32U iv_c_len,
                               INT32U num_elem, const INT8U *addr[], const INT32U *len,
                               INT8U *out)
{
	const INT8U *_addr[6];
	INT32U       _len[6];
	const INT8U *k1, *k2;
	INT32U       crypt_len;
	INT32U       i;
	INT8U        ret;
	INT8U        iv[AES_BLOCK_SIZE];
	INT8U        check[AES_BLOCK_SIZE];

	if (iv_c_len < AES_BLOCK_SIZE || num_elem > ARRAY_SIZE(_addr) - 1)
		return 0;

	crypt_len = iv_c_len - AES_BLOCK_SIZE;
	key_len /= 2;
	k1 = key;
	k2 = key + key_len;

	for (i = 0; i < num_elem; i++) {
		_addr[i] = addr[i];
		_len[i]  = len[i];
	}
	_addr[num_elem] = out;
	_len[num_elem]  = crypt_len;

	memcpy(iv, iv_crypt, AES_BLOCK_SIZE);
	memcpy(out, iv_crypt + AES_BLOCK_SIZE, crypt_len);

	iv[8] &= 0x7f;
	iv[12] &= 0x7f;

	ret = PLATFORM_AES_CTR_ENCRYPT(k2, key_len, iv, out, crypt_len);
	if (!ret)
		return ret;

	ret = _aes_s2v(k1, key_len, num_elem + 1, _addr, _len, check);
	if (!ret)
		return ret;
	if (memcmp(check, iv_crypt, AES_BLOCK_SIZE) == 0)
		return 1;

	return 0;
}

INT8U PLATFORM_AES_CMAC(const INT8U *key, INT32U keylen, INT8U num_elem, const INT8U **addr, INT32U *len, INT8U *mac)
{
	CMAC_CTX *ctx;
	INT8U     ret = 0;
	size_t    outlen, i;

	ctx = CMAC_CTX_new();
	if (ctx == NULL)
		return 0;

	if (keylen == 32) {
		if (!CMAC_Init(ctx, key, 32, EVP_aes_256_cbc(), NULL))
			goto fail;
	} else if (keylen == 24) {
		if (!CMAC_Init(ctx, key, 24, EVP_aes_192_cbc(), NULL))
			goto fail;
	} else if (keylen == 16) {
		if (!CMAC_Init(ctx, key, 16, EVP_aes_128_cbc(), NULL))
			goto fail;
	} else {
		printf("[CRYPTO] CMAC does not support %d keylen\n", keylen);
		goto fail;
	}
	for (i = 0; i < num_elem; i++) {
		if (!CMAC_Update(ctx, addr[i], len[i]))
			goto fail;
	}
	if (!CMAC_Final(ctx, mac, &outlen) || outlen != 16)
		goto fail;

	ret = 1;
fail:
	CMAC_CTX_free(ctx);
	return ret;
}

INT8U PLATFORM_AES_CBC_ENCRYPT(INT8U *key, INT8U *iv, INT8U *data, INT32U data_len)
{
	return _aes_encrypt(EVP_aes_128_cbc(), key, iv, data, data_len);
}

INT8U PLATFORM_AES_CBC_DECRYPT(INT8U *key, INT8U *iv, INT8U *data, INT32U data_len)
{
	return _aes_decrypt(EVP_aes_128_cbc(), key, iv, data, data_len);
}

INT8U PLATFORM_AES_CTR_ENCRYPT(const INT8U *key, const INT32U key_len, const INT8U *iv, INT8U *data, INT32U data_len)
{
	switch (key_len) {
	case 16:
		return _aes_encrypt(EVP_aes_128_ctr(), key, iv, data, data_len);
	case 24:
		return _aes_encrypt(EVP_aes_192_ctr(), key, iv, data, data_len);
	case 32:
		return _aes_encrypt(EVP_aes_256_ctr(), key, iv, data, data_len);
	default:
		break;
	}
	printf("[CRYPTO] Invalid key length\n");
	return 0;
}

INT8U PLATFORM_AES_CTR_DECRYPT(const INT8U *key, const INT32U key_len, const INT8U *iv, INT8U *data, INT32U data_len)
{
	switch (key_len) {
	case 16:
		return _aes_decrypt(EVP_aes_128_ctr(), key, iv, data, data_len);
	case 24:
		return _aes_decrypt(EVP_aes_192_ctr(), key, iv, data, data_len);
	case 32:
		return _aes_decrypt(EVP_aes_256_ctr(), key, iv, data, data_len);
	default:
		break;
	}
	printf("[CRYPTO] invalid key length\n");
	return 0;
}

INT8U PLATFORM_AES_WRAP(const INT8U *key, const INT32U key_len, INT8U *plain, INT32U n_blocks, INT8U *cipher_out)
{
	AES_KEY actx;
	INT8U   ret = 0;

	memset(&actx, 0, sizeof(AES_KEY));

	if (AES_set_encrypt_key(key, key_len << 3, &actx)) // shift 3 converts bytes to bits
		return 0;

	ret = AES_wrap_key(&actx, NULL, cipher_out, plain, n_blocks * 8);
	OPENSSL_cleanse(&actx, sizeof(actx));

	return ret > 0 ? 1 : 0;
}

INT8U PLATFORM_AES_UNWRAP(const INT8U *key, const INT32U key_len, INT8U *cipher, INT32U n_block, INT8U *plain_out)
{
	AES_KEY actx;
	int     ret = 0;

	memset(&actx, 0, sizeof(AES_KEY));

	if (AES_set_decrypt_key(key, key_len << 3, &actx)) // shift 3 converts bytes to bits
		return 0;

	ret = AES_unwrap_key(&actx, NULL, plain_out, cipher, (n_block + 1) * 8);
	OPENSSL_cleanse(&actx, sizeof(actx));

	return ret > 0 ? 1 : 0;
}