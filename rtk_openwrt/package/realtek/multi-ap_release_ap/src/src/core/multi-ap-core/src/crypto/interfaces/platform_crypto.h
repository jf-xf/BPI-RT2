/*
 *  Broadband Forum IEEE 1905.1/1a stack
 *
 *  Copyright (c) 2017, Broadband Forum
 *  Copyright (c) 2018, Realtek Semiconductor Corp.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  Subject to the terms and conditions of this license, each copyright
 *  holder and contributor hereby grants to those receiving rights under
 *  this license a perpetual, worldwide, non-exclusive, no-charge,
 *  royalty-free, irrevocable (except for failure to satisfy the
 *  conditions of this license) patent license to make, have made, use,
 *  offer to sell, sell, import, and otherwise transfer this software,
 *  where such license applies only to those patent claims, already
 *  acquired or hereafter acquired, licensable by such copyright holder or
 *  contributor that are necessarily infringed by:
 *
 *  (a) their Contribution(s) (the licensed copyrights of copyright holders
 *      and non-copyrightable additions of contributors, in source or binary
 *      form) alone; or
 *
 *  (b) combination of their Contribution(s) with the work of authorship to
 *      which such Contribution(s) was added by such copyright holder or
 *      contributor, if, at the time the Contribution is added, such addition
 *      causes such combination to be necessarily infringed. The patent
 *      license shall not apply to any other combinations which include the
 *      Contribution.
 *
 *  Except as expressly stated above, no rights or licenses from any
 *  copyright holder or contributor is granted under this license, whether
 *  expressly, by implication, estoppel or otherwise.
 *
 *  DISCLAIMER
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 *  DAMAGE.
 */

#ifndef _PLATFORM_CRYPTO_H_
#define _PLATFORM_CRYPTO_H_

#define SHA256_MAC_LEN 32
#define SHA384_MAC_LEN 48
#define SHA512_MAC_LEN 64

// Fill the buffer of length 'len' pointed by 'p' with random bytes.
//
// Return "0" if there was a problem, "1" otherwise
//
INT8U PLATFORM_GET_RANDOM_BYTES(INT8U *p, INT16U len);

// Return a Diffie Hellman pair of private and public keys (and its lengths) in
// the output arguments "priv", "priv_len", "pub" and "pub_len".
//
// Both "priv" and "pub" must be deallocated by the caller when they are no
// longer needed (using "PLATFORM_FREE()")
//
// The keys are obtained using the DH group specified in RFC3523 "section 2"
// (ie. the "1536-bit MODP Group" where "g = 2" and "p = 2^1536 - 2^1472 - 1 +
// 2^64 * { [2^1406 pi] + 741804 }")
//
// Return "0" if there was a problem, "1" otherwise
//
INT8U PLATFORM_GENERATE_DH_KEY_PAIR(INT8U **priv, INT16U *priv_len, INT8U **pub, INT16U *pub_len);

// Return the Diffie Hell shared secret (in output argument "shared_secret"
// which is "shared_secret_len" bytes long) associated to a remote public key
// ("remote_pub", which is "remote_pub_len" bytes long") and a local private
// key ("local_priv", which is "local_priv_len" bytes long).
//
// "shared_secret" must be deallocated by the caller once it is no longer needed
// (using "PLATFORM_FREE()")
//
// Return "0" if there was a problem, "1" otherwise
//
INT8U PLATFORM_COMPUTE_DH_SHARED_SECRET(INT8U **shared_secret, INT16U *shared_secret_len,
                                        INT8U *remote_pub, INT16U remote_pub_len, INT8U *local_priv, INT8U local_priv_len);

// Return the SHA256/384/512 digest of the provided input.
//
// The provided input is the result of concatenating 'num_elem' elements
// (addr[0], addr[1], ..., addr[num_elem-1] of size len[0], len[1], ...,
// len[num_elem-1])
//
// The digest is returned in the 'digest' output argument which must point to
// a preallocated buffer.
//
// For SHA256, the buffer should be sized at least "SHA256_MAC_LEN" bytes.
// For SHA384, the buffer should be sized at least "SHA384_MAC_LEN" bytes.
// For SHA512, the buffer should be sized at least "SHA512_MAC_LEN" bytes.
//
INT8U PLATFORM_SHA256(INT8U num_elem, INT8U **addr, INT32U *len, INT8U *digest);
INT8U PLATFORM_SHA384(INT8U num_elem, INT8U **addr, INT32U *len, INT8U *digest);
INT8U PLATFORM_SHA512(INT8U num_elem, INT8U **addr, INT32U *len, INT8U *digest);

// Return the HMAC_SHA256/384/512 digest of the provided input using the provided 'key'
// (which is 'keylen' bytes long).
//
// The provided input is the result of concatenating 'num_elem' elements
// (addr[0], addr[1], ..., addr[num_elem-1] of size len[0], len[1], ...,
// len[num_elem-1])
//
// The digest is returned in the 'hmac' output argument which must point to
// a preallocated buffer of "SHA256/384/512_MAC_LEN" bytes.
//
// Return "0" if there was a problem, "1" otherwise
//
INT8U PLATFORM_HMAC_SHA256(INT8U *key, INT32U keylen, INT8U num_elem, INT8U **addr, INT32U *len, INT8U *hmac);
INT8U PLATFORM_HMAC_SHA384(INT8U *key, INT32U keylen, INT8U num_elem, INT8U **addr, INT32U *len, INT8U *hmac);
INT8U PLATFORM_HMAC_SHA512(INT8U *key, INT32U keylen, INT8U num_elem, INT8U **addr, INT32U *len, INT8U *hmac);

// HMAC-SHA256/384/512 based KDF (RFC 5869)
//
// This function is used to derive new, cryptographically separate keys from a
// given key in DPP. This KDF is defined in RFC 5295, Chapter 3.1.2. When used
// with label = NULL and seed = info, this matches HKDF-Expand() defined in
// RFC 5869, Chapter 2.3.
//
// Return "0" if there was a problem, "1" otherwise
//
INT8U PLATFORM_HMAC_SHA256_HKDF_EXPAND(INT8U *secret, INT32U secretlen, INT8U *label, INT8U *seed, INT32U seedlen, INT8U *out, INT32U outlen);
INT8U PLATFORM_HMAC_SHA384_HKDF_EXPAND(INT8U *secret, INT32U secretlen, INT8U *label, INT8U *seed, INT32U seedlen, INT8U *out, INT32U outlen);
INT8U PLATFORM_HMAC_SHA512_HKDF_EXPAND(INT8U *secret, INT32U secretlen, INT8U *label, INT8U *seed, INT32U seedlen, INT8U *out, INT32U outlen);
#endif
