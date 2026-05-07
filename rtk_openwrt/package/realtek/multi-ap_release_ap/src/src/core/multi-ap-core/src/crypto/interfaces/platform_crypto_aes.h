/*
 *  Multi-AP Platform Crypto
 *  Advanced Encryption Standard (AES)
 *
 *  Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 */

#ifndef _PLATFORM_CRYPTO_AES_H_
#define _PLATFORM_CRYPTO_AES_H_

#include "platform_crypto.h"

#define AES_BLOCK_SIZE 16

// Encrypt the provided 'data' (which is a pointer to buffer of size
// n*AES_BLOCK_SIZE) using the AES 128 CBC algorithm with the provided
// "initialization vector" ('iv', which is also a pointer to a buffer of
// AES_BLOCK_SIZE bytes).
//
// The result is written to THE SAME 'data' buffer and has the same length as
// the input (plain) data.
//
// Note that you might have to "pad" the data buffer (so that its length is a
// multiple of AES_BLOCK_SIZE) in most cases.
//
// Return "0" if there was a problem, "1" otherwise
INT8U PLATFORM_AES_CBC_ENCRYPT(INT8U *key, INT8U *iv, INT8U *data, INT32U data_len);

// Encrypt the provided 'data' (which is a pointer to a buffer of size
// n*AES_BLOCK_SIZE) using the AES 128/192/256 CTR algorithm with the provided
// "initialization vector" ('iv', which is also a pointer to a buffer of
// AES_BLOCK_SIZE bytes) and 'key_len' (16 = 128 / 24 = 192 / 32 = 256).
//
// The result is written to THE SAME 'data' buffer and has the same length as
// the input (plain) data.
//
// Return "0" if there was a problem, "1" otherwise
INT8U PLATFORM_AES_CTR_ENCRYPT(const INT8U *key, const INT32U key_len, const INT8U *iv, INT8U *data, INT32U data_len);

// Encrypt the provided 'pw' lengthed 'pwlen', and authenticate (not encrypt)
// the associated data elements 'addr' using the AES 256/384/512 SIV algorithm using the
// 'key' and 'key_len' (32 = 256 / 48 = 384 / 64 = 512). This implementation follows
// the RFC 5297 standard, and the output is deterministic because no IV is needed.
//
// AES-SIV divides the 'key' into two parts. It runs the AES_S2V (implemented with AES-CBC)
// for the top half of the 'key', and AES-CTR for the bottom half.
//
// The result is written to a buffer 'out' and has the length sized
// AES_BLOCK_SIZE + pwlen, where the first part is message authentication code,
// and the second part is ciphertext.
//
// Return "0" if there was a problem, "1" otherwise
INT8U PLATFORM_AES_SIV_ENCRYPT(const INT8U *key, INT32U key_len,
                               const INT8U *pw, INT32U pwlen,
                               INT32U num_elem, const INT8U *addr[], const INT32U *len,
                               INT8U *out);

// Works exactly like "PLATFORM_AES_*_ENCRYPT", but now the 'data' buffer
// originally contains encrypted data and after the call it contains
// unencrypted data.
INT8U PLATFORM_AES_CBC_DECRYPT(INT8U *key, INT8U *iv, INT8U *data, INT32U data_len);
INT8U PLATFORM_AES_CTR_DECRYPT(const INT8U *key, const INT32U key_len, const INT8U *iv, INT8U *data, INT32U data_len);

// Work similarly like "PLATFORM_AES_SIV_ENCRYPT":
// it reads the ciphertext at 'iv_crypt' and 'iv_c_len',
// authenticates the associated data elements at 'addr' and 'len'.
// Finally, it checks the associated data and decrypt the ciphertext.
//
// Successful execution will output cleartext at 'out'.
// Return "0" if there was a problem, "1" otherwise
INT8U PLATFORM_AES_SIV_DECRYPT(const INT8U *key, INT32U key_len,
                               const INT8U *iv_crypt, INT32U iv_c_len,
                               INT32U num_elem, const INT8U *addr[], const INT32U *len,
                               INT8U *out);

// This function implements RFC 4493 to support AES-CMAC algorithm.
// AES-CMAC is a messaage authentication code.
// It is equivalent to One-Key CBC MAC1 (OMAC1) proposed by Iwata and Kurosawa.
//
// Given a 'key' with 'keylen' 16 (128 bits), 24 (384 bits) or 32 (256 bits),
// it authenticates the associated data elements at 'addr' and output a MAC.
//
// Return "0" if there was a problem, "1" otherwise
INT8U PLATFORM_AES_CMAC(const INT8U *key, INT32U keylen, INT8U num_elem, const INT8U **addr, INT32U *len, INT8U *mac);

// Encrypt the provided 'plain', which is a pointer to buffer sized
// n_block * 8 (each block is 64 bits) using the NIST AES WRAP algorithm (RFC 3394).
//
// The result is written to the preallocated 'cipher_out' buffer. The buffer
// size must be one block (64 bits, or 8 bytes) larger than the input (plain) data.
//
// Return "0" if there was a problem, "1" otherwise
INT8U PLATFORM_AES_WRAP(const INT8U *key, const INT32U key_len,
                        INT8U *plain, INT32U n_block, INT8U *cipher_out);

// Works exactly like "PLATFORM_AES_WRAP", but now the 'cipher' buffer
// contains encrypted data. After calling, the decrypted plain text will
// be stored in the preallocated 'plain_out' buffer.
//
// The 'plain_out' buffer should be one block smaller than the 'cipher' buffer.
//
// Return "0" if there was a problem, "1" for success.
INT8U PLATFORM_AES_UNWRAP(const INT8U *key, const INT32U key_len,
                          INT8U *cipher, INT32U n_block, INT8U *plain_out);

#endif