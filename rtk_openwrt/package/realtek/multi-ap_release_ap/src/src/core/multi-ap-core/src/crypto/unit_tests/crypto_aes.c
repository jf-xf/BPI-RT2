#include "platform.h"
#include "utils.h"

#include "platform_crypto.h"
#include "platform_crypto_aes.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Sample Materials for Encrytion / Decryption
///////////////////////////////////////////////////////////////////////////////////////////////////
// Sample cleartext
INT8U cleartext[] = {
	0x28, 0x64, 0x4a, 0x12, 0x40, 0xda, 0x50, 0x5f, 0x62, 0x06, 0xc1, 0x0c, 0xb7, 0x7c, 0x52, 0xe7,
	0x54, 0x5c, 0x38, 0x99, 0xcc, 0x8c, 0x57, 0xef, 0x1a, 0x27, 0xaf, 0xfe, 0x26, 0x7e, 0xda, 0x8b,
	0x41, 0x11, 0xb2, 0x05, 0x82, 0xad, 0x59, 0xa5, 0x29, 0xad, 0x3a, 0x7d, 0x79, 0x31, 0x56, 0x3e,
	0xea, 0x33, 0xf0, 0xcb, 0x1d, 0xf1, 0x96, 0x6a, 0xf5, 0x51, 0x6c, 0x7d, 0x0e, 0x27, 0x60, 0x37
};

// AES-CMAC-128 generated Message Authentication Code based on cleartext
INT8U cleartext_cmac16[] = {
	0x1B, 0x1D, 0x9E, 0x4C, 0xE0, 0x1E, 0x0F, 0x67, 0x7B, 0xF6, 0xA7, 0xD7, 0xAA, 0x0B, 0x32, 0xAA
};

// AES-CMAC-192 generated Message Authentication Code based on cleartext
INT8U cleartext_cmac24[] = {
	0x68, 0x6C, 0x08, 0xB3, 0x79, 0xA4, 0x9D, 0x90, 0x43, 0xE9, 0xA4, 0xF7, 0xB4, 0xA4, 0x67, 0xF4
};

// AES-CMAC-256 generated Message Authentication Code based on cleartext
INT8U cleartext_cmac32[] = {
	0xE2, 0x03, 0x9A, 0xD1, 0xB3, 0x42, 0xA5, 0x17, 0xBB, 0x47, 0x9F, 0x4C, 0x35, 0xB3, 0x56, 0xCA
};

// Actual cleartext sent in CMDU
// Multi-AP Profile TLV = b3 00 01 03
INT8U map_cleartext[] = {
	0xb3, 0x00, 0x01, 0x03
};

// Actual ciphertext sent in CMDU
INT8U map_ciphertext[] = {
	0x3a, 0xfc, 0xcf, 0xed, 0xed, 0x48, 0xfc, 0x69, 0x6c, 0x1b,
	0x5e, 0xe8, 0x82, 0x6c, 0xac, 0x8e, 0xfb, 0x8b, 0xe6, 0x0c
};

// Sample associated data for AEAD ciphers (e.g. AES-SIV)
const INT8U  assoc_data1[]    = { 0xdb, 0x92, 0xd5, 0x55, 0x70, 0x6a, 0x65, 0x35 };
const INT8U  assoc_data2[]    = { 0xdb, 0x92, 0xd5, 0x55, 0x70, 0x6a, 0x65, 0x35 };
const INT8U  assoc_data3[]    = { 0xdb, 0x92, 0xd5, 0x55, 0x70, 0x6a, 0x65, 0x35 };
const INT32U assoc_data_len[] = { 0x08, 0x08, 0x08 };
const INT8U *assoc_data_arr[] = { assoc_data1, assoc_data2, assoc_data3 };

// Actual associated data based on CMDU
// // AD1
// msg ver             00
// msg type            0002
// msg id              5e96
// relay indicator     00
// // AD2
// etc                 0000002
// // AD3
// src mac addr        00e04c267001
// // AD4
// dst mac addr        00e04c467001
const INT8U  map_assoc_data1[]    = { 0x00, 0x00, 0x02, 0x5e, 0x96, 0x00 };
const INT8U  map_assoc_data2[]    = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 };
const INT8U  map_assoc_data3[]    = { 0x00, 0xe0, 0x4c, 0x26, 0x70, 0x01 };
const INT8U  map_assoc_data4[]    = { 0x00, 0xe0, 0x4c, 0x46, 0x70, 0x01 };
const INT32U map_assoc_data_len[] = { 0x06, 0x06, 0x06 , 0x06 };
const INT8U *map_assoc_data_arr[] = { map_assoc_data1, map_assoc_data2, map_assoc_data3, map_assoc_data4 };

// Randomly generated testing key of different length
INT8U key_16bytes[] = {
	0xb7, 0x43, 0x86, 0x43, 0x11, 0x46, 0xe9, 0x10, 0xa7, 0x0d, 0x15, 0x90, 0x81, 0xc2, 0x99, 0xc2
};

INT8U key_24bytes[] = {
	0x79, 0x90, 0xc6, 0xcb, 0x44, 0xca, 0x92, 0xa5, 0x1c, 0x28, 0xd9, 0x37, 0xe9, 0x40, 0x5c, 0x48,
	0x50, 0xbb, 0x83, 0xdd, 0x90, 0xab, 0x8b, 0x8a
};

INT8U key_32bytes[] = {
	0xf1, 0x0e, 0xad, 0x04, 0xbb, 0x04, 0x9e, 0xfe, 0x79, 0x69, 0xf0, 0x52, 0x44, 0xf1, 0xa8, 0x2d,
	0xb2, 0x09, 0x07, 0x09, 0xd3, 0x61, 0x99, 0x33, 0x19, 0x7e, 0x7d, 0xef, 0x92, 0xb6, 0xd3, 0xd5
};

INT8U key_48bytes[] = {
	0x66, 0x02, 0x06, 0xa2, 0x89, 0xe0, 0xd3, 0x9e, 0x50, 0x6b, 0x5a, 0x8b, 0x49, 0xac, 0x7d, 0xb9,
	0xf4, 0x34, 0x3e, 0x2b, 0xd6, 0xc2, 0xfe, 0xb1, 0xe9, 0x68, 0x57, 0xa6, 0xce, 0xe9, 0xbe, 0x94,
	0x17, 0x21, 0xee, 0x66, 0xce, 0x80, 0x8c, 0xdc, 0x79, 0x0e, 0x3d, 0xc4, 0xb8, 0xae, 0x72, 0xd3
};

INT8U key_64bytes[] = {
	0xaf, 0xc5, 0x77, 0x3f, 0x4c, 0xc6, 0x28, 0xce, 0x9c, 0x45, 0x91, 0xed, 0x8c, 0x90, 0xb8, 0x81, 0x1d,
	0x6f, 0xf3, 0xa8, 0xf5, 0x92, 0x5b, 0x6d, 0x90, 0x49, 0xf5, 0x37, 0x05, 0xe4, 0x45, 0xd3, 0xb8, 0xd2,
	0x9a, 0x56, 0xba, 0xde, 0xb0, 0xc2, 0x6a, 0xa2, 0xd8, 0x4b, 0x84, 0x5d, 0x9b, 0x54, 0x82, 0x49, 0x21,
	0xd2, 0x1a, 0x46, 0x5a, 0xb0, 0x56, 0xe7, 0x31, 0x40, 0x08, 0x34, 0x7f, 0xeb
};

// Actual testing key during runtime from /tmp/dpp_keystore
// tk = f829df385ed0c9520a3d112610eb645434ba8217cb76d47def31087acd378562
INT8U map_key[] = {
	0xf8, 0x29, 0xdf, 0x38, 0x5e, 0xd0, 0xc9, 0x52, 0x0a, 0x3d, 0x11, 0x26, 0x10, 0xeb, 0x64, 0x54,
	0x34, 0xba, 0x82, 0x17, 0xcb, 0x76, 0xd4, 0x7d, 0xef, 0x31, 0x08, 0x7a, 0xcd, 0x37, 0x85, 0x62
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Test Case Functions
///////////////////////////////////////////////////////////////////////////////////////////////////

INT8U TEST_AESCMACGenerate(INT8U *key, INT32U keysize, INT8U *expected_mac)
{
	const INT8U *addr[1];
	INT32U       len[1];
	INT8U        mac[AES_BLOCK_SIZE] = { 0 };
	char         err[256];

	addr[0] = cleartext;
	len[0]  = sizeof(cleartext);

	if (PLATFORM_AES_CMAC(key, keysize, ARRAY_SIZE(addr), addr, len, mac) != 1) {
		PLATFORM_SNPRINTF(err, sizeof(err), "PLATFORM_AES_CMAC returned false!");
		goto fail;
	}

	if (PLATFORM_MEMCMP(mac, expected_mac, AES_BLOCK_SIZE) != 0) {
		PLATFORM_SNPRINTF(err, sizeof(err), "PLATFORM_AES_CMAC returned false!");
		goto fail;
	}

	PLATFORM_PRINTF("%-30s - %-70d: OK\n", __FUNCTION__, keysize);
	return 0;

fail:
	PLATFORM_PRINTF("%-30s - %-70d: KO !!!\n", __FUNCTION__, keysize);
	PLATFORM_PRINTF("  %s\n", err);
	return 1;
}

INT8U TEST_AESSIVEncryptDecrypt(INT8U *key, INT32U keysize)
{
	INT8U ciphertext[AES_BLOCK_SIZE + sizeof(cleartext)] = { 0 };
	INT8U deciphertext[sizeof(cleartext)]                = { 0 };
	char  err[256];

	if (PLATFORM_AES_SIV_ENCRYPT(key, keysize,
	                             cleartext, sizeof(cleartext),
	                             ARRAY_SIZE(assoc_data_arr), assoc_data_arr, assoc_data_len,
	                             ciphertext)
	    != 1) {
		PLATFORM_SNPRINTF(err, sizeof(err), "PLATFORM_AES_SIV_ENCRYPT returned false!");
		goto fail;
	}

	if (PLATFORM_AES_SIV_DECRYPT(key, keysize,
	                             ciphertext, sizeof(ciphertext),
	                             ARRAY_SIZE(assoc_data_arr), assoc_data_arr, assoc_data_len,
	                             deciphertext)
	    != 1) {
		PLATFORM_SNPRINTF(err, sizeof(err), "PLATFORM_AES_SIV_DECRYPT returned false!");
		goto fail;
	}

	if (PLATFORM_MEMCMP(cleartext, deciphertext, sizeof(cleartext)) != 0) {
		PLATFORM_SNPRINTF(err, sizeof(err), "decrypted text is not the same as cleartext!");
		goto fail;
	}

	PLATFORM_PRINTF("%-30s - %-70d: OK\n", __FUNCTION__, keysize);
	return 0;

fail:
	PLATFORM_PRINTF("%-30s - %-70d: KO !!!\n", __FUNCTION__, keysize);
	PLATFORM_PRINTF("  %s\n", err);
	return 1;
}

INT8U TEST_AESSIVEncrypt(INT8U *key, INT32U keysize)
{
	INT8U ciphertext[AES_BLOCK_SIZE + sizeof(map_cleartext)] = { 0 };
	char  err[256];

	if (PLATFORM_AES_SIV_ENCRYPT(key, keysize,
	                             map_cleartext, sizeof(map_cleartext),
	                             ARRAY_SIZE(map_assoc_data_arr), map_assoc_data_arr, map_assoc_data_len,
	                             ciphertext)
	    != 1) {
		PLATFORM_SNPRINTF(err, sizeof(err), "PLATFORM_AES_SIV_ENCRYPT returned false!");
		goto fail;
	}

	if (PLATFORM_MEMCMP(map_ciphertext, ciphertext, sizeof(map_ciphertext)) != 0) {
		PLATFORM_SNPRINTF(err, sizeof(err), "expected ciphertext is not the same as result ciphertext!");
		goto fail;
	}

	PLATFORM_PRINTF("%-30s - %-70d: OK\n", __FUNCTION__, keysize);
	return 0;

fail:
	PLATFORM_PRINTF("%-30s - %-70d: KO !!!\n", __FUNCTION__, keysize);
	PLATFORM_PRINTF("  %s\n", err);
	return 1;
}

INT8U TEST_AESSIVDecrypt(INT8U *key, INT32U keysize)
{
	INT8U deciphertext[sizeof(map_cleartext)] = { 0 };
	char  err[256];

	if (PLATFORM_AES_SIV_DECRYPT(key, keysize,
	                             map_ciphertext, sizeof(map_ciphertext),
	                             ARRAY_SIZE(map_assoc_data_arr), map_assoc_data_arr, map_assoc_data_len,
	                             deciphertext)
	    != 1) {
		PLATFORM_SNPRINTF(err, sizeof(err), "PLATFORM_AES_SIV_DECRYPT returned false!");
		goto fail;
	}

	if (PLATFORM_MEMCMP(map_cleartext, deciphertext, sizeof(map_cleartext)) != 0) {
		PLATFORM_SNPRINTF(err, sizeof(err), "expected cleartext is not the same as result deciphertext!");
		goto fail;
	}

	PLATFORM_PRINTF("%-30s - %-70d: OK\n", __FUNCTION__, keysize);
	return 0;

fail:
	PLATFORM_PRINTF("%-30s - %-70d: KO !!!\n", __FUNCTION__, keysize);
	PLATFORM_PRINTF("  %s\n", err);
	return 1;
}

INT8U TEST_AESCTREncryptDecrypt(INT8U *key, INT32U keysize)
{
	INT8U       data[sizeof(cleartext)] = { 0 };
	const INT8U iv[32]                  = { 0 };
	char        err[256];

	PLATFORM_MEMCPY(data, cleartext, sizeof(data));

	if (PLATFORM_AES_CTR_ENCRYPT(key, keysize, iv, data, sizeof(data)) != 1) {
		PLATFORM_SNPRINTF(err, sizeof(err), "PLATFORM_AES_CTR_ENCRYPT returned false!");
		goto fail;
	}

	if (PLATFORM_AES_CTR_DECRYPT(key, keysize, iv, data, sizeof(data)) != 1) {
		PLATFORM_SNPRINTF(err, sizeof(err), "PLATFORM_AES_CTR_DECRYPT returned false!");
		goto fail;
	}

	if (PLATFORM_MEMCMP(data, cleartext, sizeof(data)) != 0) {
		PLATFORM_SNPRINTF(err, sizeof(err), "decrypted text is not the same as cleartext!");
		goto fail;
	}

	PLATFORM_PRINTF("%-30s - %-70d: OK\n", __FUNCTION__, keysize);
	return 0;

fail:
	PLATFORM_PRINTF("%-30s - %-70d: KO !!!\n", __FUNCTION__, keysize);
	PLATFORM_PRINTF("  %s\n", err);
	return 1;
}

INT8U TEST_AESCBCEncryptDecrypt(INT8U *key)
{
	INT8U data[sizeof(cleartext)] = { 0 };
	INT8U iv[32]                  = { 0 };
	char  err[256];

	PLATFORM_MEMCPY(data, cleartext, sizeof(data));

	if (PLATFORM_AES_CBC_ENCRYPT(key, iv, data, sizeof(data)) != 1) {
		PLATFORM_SNPRINTF(err, sizeof(err), "PLATFORM_AES_CBC_ENCRYPT returned false!");
		goto fail;
	}

	if (PLATFORM_AES_CBC_DECRYPT(key, iv, data, sizeof(data)) != 1) {
		PLATFORM_SNPRINTF(err, sizeof(err), "PLATFORM_AES_CBC_DECRYPT returned false!");
		goto fail;
	}

	if (PLATFORM_MEMCMP(data, cleartext, sizeof(data)) != 0) {
		PLATFORM_SNPRINTF(err, sizeof(err), "decrypted text is not the same as cleartext!");
		goto fail;
	}

	PLATFORM_PRINTF("%-100s   : OK\n", __FUNCTION__);
	return 0;

fail:
	PLATFORM_PRINTF("%-100s   : KO !!!\n", __FUNCTION__);
	PLATFORM_PRINTF("  %s\n", err);
	return 1;
}

int main(void)
{
	INT8U result = 0;

	result += TEST_AESCMACGenerate(key_16bytes, sizeof(key_16bytes), cleartext_cmac16);
	result += TEST_AESCMACGenerate(key_24bytes, sizeof(key_24bytes), cleartext_cmac24);
	result += TEST_AESCMACGenerate(key_32bytes, sizeof(key_32bytes), cleartext_cmac32);

	result += TEST_AESSIVEncryptDecrypt(key_32bytes, sizeof(key_32bytes));
	result += TEST_AESSIVEncryptDecrypt(key_48bytes, sizeof(key_48bytes));
	result += TEST_AESSIVEncryptDecrypt(key_64bytes, sizeof(key_64bytes));

	result += TEST_AESSIVEncrypt(map_key, sizeof(map_key));
	result += TEST_AESSIVDecrypt(map_key, sizeof(map_key));

	result += TEST_AESCBCEncryptDecrypt(key_16bytes);

	return result;
}