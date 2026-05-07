/*
 *  Multi-AP Platform Crypto
 *  Elliptic Curve Cryptography (ECC)
 *
 *  Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 */

#ifndef _PLATFORM_CRYPTO_ECC_H_
#define _PLATFORM_CRYPTO_ECC_H_

#include "platform_crypto.h"

#define MAX_CURVE_NAME_LEN 16
#define MAX_ASN1_DER_PRIVKEY_LEN 256
#define MAX_ASN1_DER_PUBKEY_LEN 256
#define MAX_ANSI_X963_LEN 192
#define MAX_SUBJECT_PUBKEY_INFO_LEN 128
#define MAX_SHARED_SECRET_LEN 66

typedef struct ECCKeyPair {

	// Key Type
	enum KEY_TYPE { PUBLIC_KEY,
		            PRIVATE_KEY } type;

	// Curve Parameter (e.g. prime256v1)
	char curve[MAX_CURVE_NAME_LEN];

	// ASN.1 DER formatted private key
	INT8U  priv[MAX_ASN1_DER_PRIVKEY_LEN];
	INT32S priv_len;

	// coordinates of public key
	// It follows the ANSI X9.63 standard using a byte string of 04 || X || Y.
	INT8U  pub_ansix963[MAX_ANSI_X963_LEN];
	INT32S pub_ansix963_len;

	// RFC 5280 ASN.1 SEQUENCE SubjectPublicKeyInfo formatted public key
	// this follows the format for DPP bootstrapping
	// Compressed format
	INT8U  subpkinfo[MAX_SUBJECT_PUBKEY_INFO_LEN];
	INT32S subpkinfo_len;

} ECCKeyPair;

typedef struct DPPReconfigID {
	ECCKeyPair *csign;
	ECCKeyPair *a_nonce;    // A-NONCE
	ECCKeyPair *e_id;       // E-id
	ECCKeyPair *e_prime_id; // E'-id
	ECCKeyPair *ppkey;
} DPPReconfigID;

// Generate Elliptic Curve Public-Private Key Pair
//
// Given any curve name supported by OpenSSL in obj_dat.h (such as prime256v1),
// this function generates a public private key pair based on the curve.
//
// Return "0" if there was a problem, "1" otherwise
//
INT8U PLATFORM_GENERATE_EC_KEYPAIR(const char *curve_name, ECCKeyPair *keypair);

// Parse SubjectPublicKeyInfo Object to ECCKeyPair
//
// Given a DER encoded ASN.1 SubjectPublicKeyInfo (as in DPP bootstrapping),
// this function parse the content to generate one ECCKeyPair public key object.
//
// Return "0" if there was a problem, "1" otherwise
//
INT8U PLATFORM_PARSE_EC_SUBJECT_PUBLIC_KEY_INFO(const INT8U *subpkinfo, const INT32S subpkinfo_len, ECCKeyPair *keypair);

// Parse ANSI X.963 Uncompressed Public Key to ECCKeyPair
//
// Given a ANSI X.963 public key, this function generates one ECCKeyPair public key object.
//
// Return "0" if there was a problem, "1" otherwise
//
INT8U PLATFORM_PARSE_EC_PUBLIC_ANSIX963(const char *curve_name, INT8U *pub_ansi963, INT32S pub_ansi963_len, ECCKeyPair *keypair);

// Parse ASN.1 DER binary uncompressed format or  ANSI X.963 compressed public key to ECCKeyPair
//
// Given a binary public key, this function generates one ECCKeyPair public key object.
//
// Return "0" if there was a problem, "1" otherwise
//
INT8U PLATFORM_PARSE_EC_PUBLIC_KEY(INT8U *pub, INT32S pub_len, ECCKeyPair *keypair);

// Parse ASN.1 DER binary format private key to ECCKeyPair
//
// Given a binary private key, this function generates one ECCKeyPair public-private key object.
//
// Return "0" if there was a problem, "1" otherwise
//
INT8U PLATFORM_PARSE_EC_PRIVATE_KEY(INT8U *priv, INT32S priv_len, ECCKeyPair *keypair);

// Calculate the Elliptic Curve Diffie-Hellman Key Exchange
//
// Given a public key and a private key, this function calculates the ECDH and output the common secret.
// For example, Alice sends a public key (a.P) to Bob, and Bob sends a public key (b.P) to Alice.
// it becomes b.(a.P) = a.(b.P) = common secret
//
// Return "0" if there was a problem, "1" otherwise
//
INT8U PLATFORM_COMPUTE_ECDH_SHARED_SECRET(ECCKeyPair *priv, ECCKeyPair *pub, INT8U *secret, size_t *secret_len);

// Sign a message vector using Elliptic Curve Digital Signature Algorithm
//
// Given a private key 'priv' and a message vector (num_elem, addr, len), this function runs ECDSA and outputs
// the generated signature (r,s) to 'signature'. Please be aware that the size of the preallocated 'signature'
// buffer (i.e., signature_len) must be equal to or larger than the size of (primelen * 2). The 'primelen' refers
// to the maximum length of prime number r or s. For example, prime256v1 requires 32 bytes (256 bits), secp384r1
// requires 48 bytes, and secp521r1 requires 66 bytes.
//
// As ECDSA needs to hash the message before running elliptic curve operations, a 'hashlen' is also required.
// Basically, 32/48/64 refers to SHA256/384/512 algorithm.
//
// Return "0" if there was a problem, "1" otherwise
//
INT8U PLATFORM_ECDSA_SIGN(INT8U hashlen, ECCKeyPair *priv, INT8U num_elem, INT8U **addr, INT32U *len, INT8U *signature, INT16U signature_len);

// Verify a signature using Elliptic Curve Digital Signature Algorithm
//
// Given a SHA hash length 'hashlen', a public key 'pub', a message vector (num_elem, addr, len) and a 'signature'
// (i.e., the (r,s)), this function runs ECDSA to verify if the 'r' can generate back the intended 's'.
//
// For 'signature_len', it refers to the buffer size of the signature. We assume both r and s are prime numbers taking
// the same number of bytes, so the size of r and s should be (signature_len / 2).
//
// Return "0" if there was a problem, "1" otherwise
//
INT8U PLATFORM_ECDSA_VERIFY(INT8U hashlen, ECCKeyPair *pub, INT8U num_elem, INT8U **addr, INT32U *len, INT8U *signature, INT16U signature_len);

/**
 * PLATFORM_EC_KEY_GET_PUBLIC_POINT - Get public key point coordinates
 *
 * @keypair: EC key from crypto_ec_key_parse/set_pub() or crypto_ec_key_parse_priv()
 * @out: Buffer with coordinates of public key in uncompressed form or %NULL on failure
 * @out_len: Length of the returned buffer
 * Returns: 1 on success or 0 on failure
 */
INT8U PLATFORM_EC_KEY_GET_PUBLIC_POINT(ECCKeyPair *keypair, INT8U *out, INT32S *out_len);

/**
 * PLATFORM_EC_KEY_GET_GROUP - Get IANA group identifier for an EC key
 *
 * @keypair: EC key from crypto_ec_key_parse/set_pub() or crypto_ec_key_parse_priv()
 * Returns: IANA group identifier or 0xFF on failure
 */
INT16U PLATFORM_EC_KEY_GET_GROUP(ECCKeyPair *keypair);

/**
 * PLATFORM_EC_KEY_SET_PUBLIC_KEY - Initialize an EC public key from EC point coordinates
 *
 * @group: Identifying number for the ECC group
 * @x: X coordinate of the public key
 * @y: Y coordinate of the public key
 * @len: Length of @x and @y buffer
 * @keypair: Output public key set based on EC coordinates
 * Returns: 1 on success or 0 on failure
 *
 * This function initialize an EC key from public key coordinates, in big endian
 * byte order padded to the length of the prime defining the group.
 */
INT8U PLATFORM_EC_KEY_SET_PUBLIC_KEY(INT16U group, INT8U *buf_x, INT8U *buf_y, size_t len, ECCKeyPair *keypair);

/**
 * PLATFORM_EC_KEY_ADDITION - Add public keys of two EC keypairs
 *
 * @a: One of the EC keypair containing public key to be added
 * @b: The other EC keypair containing public key to be added
 * @curve_name: Name of the curve used
 * @res: EC keypair containing public key resulted from addition
 * Returns: 1 on success or 0 on failure
 */
INT8U PLATFORM_EC_KEY_ADD_PUBLIC_KEY(ECCKeyPair *a, ECCKeyPair *b, const char *curve_name, ECCKeyPair *res);

/**
 * PLATFORM_DECRYPT_E_ID - Decrypts E-id from its E'-id and A-NONCE
 * E-id = E'-id - s_C * A-NONCE
 *
 * @ppkey: Key used in decryption of E-id
 * @a_nonce: A-NONCE used to decrypt E-id
 * @e_prime_id: E'-id from which E-id is decrypted
 * Returns: 1 on success or 0 on failure
 */
INT8U PLATFORM_DECRYPT_E_ID(ECCKeyPair *ppkey, ECCKeyPair *a_nonce, ECCKeyPair *e_prime_id);

/**
 * PLATFORM_GENERATE_RECONFIG_ID - Generates a dppReconfigID structure with the relevant contents
 *
 * @csign: Key to be used as member of dppReconfigID structure and derive other values
 * @ppkey: Key to be used as member of dppReconfigID structure
 * Returns: dppReconfigID structure generated on success or NULL on failure
 */
DPPReconfigID *PLATFORM_GENERATE_RECONFIG_ID(ECCKeyPair *csign, ECCKeyPair *ppkey);

/**
 * PLATFORM_UPDATE_RECONFIG_ID - Updates the contents of reconfig id passed
 *
 * @id: reconfig id to be updated
 * Returns: 1 on success or 0 on failure
 */
INT8U PLATFORM_UPDATE_RECONFIG_ID(DPPReconfigID *id);

#endif
