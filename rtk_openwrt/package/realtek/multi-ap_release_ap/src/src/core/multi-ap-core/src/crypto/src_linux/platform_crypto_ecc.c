/*
 *  Multi-AP Platform Crypto
 *  Elliptic Curve Cryptography (ECC)
 *
 *  Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 */

#include <string.h>
#include <stdio.h>

#include "openssl/evp.h"
#include "openssl/ec.h"
#include "openssl/x509.h"
#include "openssl/asn1t.h"
#include "openssl/err.h"

#include "platform_crypto_ecc.h"

typedef struct {
	/* AlgorithmIdentifier ecPublicKey with optional parameters present
	 * as an OID identifying the curve */
	X509_ALGOR *alg;
	/* Compressed format public key per ANSI X9.63 */
	ASN1_BIT_STRING *pub_key;
} SUBJECT_PUBLIC_KEY_INFO;

// OpenSSL macros (from hostap DPP)
ASN1_SEQUENCE(SUBJECT_PUBLIC_KEY_INFO) = {
	ASN1_SIMPLE(SUBJECT_PUBLIC_KEY_INFO, alg, X509_ALGOR),
	ASN1_SIMPLE(SUBJECT_PUBLIC_KEY_INFO, pub_key, ASN1_BIT_STRING)
} ASN1_SEQUENCE_END(SUBJECT_PUBLIC_KEY_INFO);

IMPLEMENT_ASN1_FUNCTIONS(SUBJECT_PUBLIC_KEY_INFO);

enum DH_GROUP {
	GROUP_RANDOM_ECP_256 = 19,
	GROUP_RANDOM_ECP_384 = 20,
	GROUP_RANDOM_ECP_521 = 21,
	GROUP_RANDOM_ECP_192 = 25,
	GROUP_RANDOM_ECP_224 = 26,
	GROUP_BRAINPOOL_224  = 27,
	GROUP_BRAINPOOL_256  = 28,
	GROUP_BRAINPOOL_384  = 29,
	GROUP_BRAINPOOL_512  = 30,
	GROUP_UNKNOWN        = 0xFF
};

int _ssl_big_num_rand(BIGNUM *r, const BIGNUM *m)
{
	return BN_rand_range(r, m) == 1 ? 0 : -1;
}

int _ssl_ec_point_add(const EC_GROUP *group, const EC_POINT *a, const EC_POINT *b, EC_POINT *c, BN_CTX *ctx)
{
	return EC_POINT_add(group, c, a, b, ctx) ? 0 : -1;
}

int _ssl_ec_point_mul(const EC_GROUP *group, const EC_POINT *p, const BIGNUM *b, EC_POINT *res, BN_CTX *ctx)
{
	return EC_POINT_mul(group, res, NULL, p, b, ctx) ? 0 : -1;
}

int _ssl_ec_point_invert(const EC_GROUP *group, EC_POINT *p, BN_CTX *ctx)
{
	return EC_POINT_invert(group, p, ctx) ? 0 : 0xFF;
}

INT32S _ec_group_to_nid(INT16U group)
{
	/* Map from IANA registry for IKE D-H groups to OpenSSL NID */
	switch (group) {
	case GROUP_RANDOM_ECP_256:
		return NID_X9_62_prime256v1;
	case GROUP_RANDOM_ECP_384:
		return NID_secp384r1;
	case GROUP_RANDOM_ECP_521:
		return NID_secp521r1;
	case GROUP_RANDOM_ECP_192:
		return NID_X9_62_prime192v1;
	case GROUP_RANDOM_ECP_224:
		return NID_secp224r1;
#ifdef NID_brainpoolP224r1
	case GROUP_BRAINPOOL_224:
		return NID_brainpoolP224r1;
#endif /* NID_brainpoolP224r1 */
#ifdef NID_brainpoolP256r1
	case GROUP_BRAINPOOL_256:
		return NID_brainpoolP256r1;
#endif /* NID_brainpoolP256r1 */
#ifdef NID_brainpoolP384r1
	case GROUP_BRAINPOOL_384:
		return NID_brainpoolP384r1;
#endif /* NID_brainpoolP384r1 */
#ifdef NID_brainpoolP512r1
	case GROUP_BRAINPOOL_512:
		return NID_brainpoolP512r1;
#endif /* NID_brainpoolP512r1 */
	default:
		return GROUP_UNKNOWN;
	}
}

INT16U _nid_to_ec_group(INT32S nid)
{
	switch (nid) {
	case NID_X9_62_prime256v1:
		return GROUP_RANDOM_ECP_256;
	case NID_secp384r1:
		return GROUP_RANDOM_ECP_384;
	case NID_secp521r1:
		return GROUP_RANDOM_ECP_521;
#ifdef NID_brainpoolP256r1
	case NID_brainpoolP256r1:
		return GROUP_BRAINPOOL_256;
#endif /* NID_brainpoolP256r1 */
#ifdef NID_brainpoolP384r1
	case NID_brainpoolP384r1:
		return GROUP_BRAINPOOL_384;
#endif /* NID_brainpoolP384r1 */
#ifdef NID_brainpoolP512r1
	case NID_brainpoolP512r1:
		return GROUP_BRAINPOOL_512;
#endif /* NID_brainpoolP512r1 */
	default:
		printf("[CRYPTO] Unsupported curve (nid=%d) in EC key", nid);
		return GROUP_UNKNOWN;
	}
}

INT8U _convert_openssl_pkey_to_der(EVP_PKEY *key, INT8U *priv, INT32S *priv_len)
{
	EC_KEY         *eckey;
	const EC_GROUP *group;
	const EC_POINT *point;
	INT8U	      *der = NULL;
	INT32S          der_len;
	INT8U           ret = 0;

	if (!key || !priv || !priv_len) {
		return 0;
	}

	eckey = EVP_PKEY_get0_EC_KEY(key);
	if (!eckey) {
		return 0;
	}

	group = EC_KEY_get0_group(eckey);
	point = EC_KEY_get0_public_key(eckey);
	if (!group || !point)
		return 0;

	// Convert private key to ASN.1 DER binary format
	der_len = i2d_ECPrivateKey(eckey, &der);
	if (der_len <= 0)
		goto fail;

	// fail if the key is too long
	if (der_len > MAX_ASN1_DER_PRIVKEY_LEN) {
		printf("[CRYPTO] conversion ok but too long!\n");
		goto fail;
	}

	memcpy(priv, der, der_len);
	*priv_len = der_len;
	ret       = 1;

fail:
	OPENSSL_free(der);
	return ret;
}

INT8U _convert_openssl_pkey_to_public_der(EVP_PKEY *key, INT8U *pub, INT32S *pub_len)
{
	EC_KEY *eckey;
	INT8U  *der = NULL;
	INT32S  der_len;
	INT8U   ret = 0;

	if (!key || !pub || !pub_len) {
		return 0;
	}

	eckey = EVP_PKEY_get1_EC_KEY((EVP_PKEY *)key);
	if (!eckey)
		return 0;

	der_len = i2d_EC_PUBKEY(eckey, &der);
	if (der_len <= 0) {
		goto fail;
	}

	EC_KEY_free(eckey);

	memcpy(pub, der, der_len);
	*pub_len = der_len;
	ret      = 1;

fail:
	OPENSSL_free(der);
	return ret;
}

INT8U _ec_key_parse_pub(const INT8U *pub, const INT32S pub_len, EVP_PKEY **key)
{
	EVP_PKEY *pkey  = NULL;

	if (!key)
		return 0;

	if (!pub || !pub_len)
		return 0;

	pkey = d2i_PUBKEY(NULL, &pub, pub_len);
	if (!pkey) {
		goto fail;
	}

	/* Ensure this is an EC key */
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	if (!EVP_PKEY_is_a(pkey, "EC"))
		goto fail;
#else /* OpenSSL version >= 3.0 */
	if (!EVP_PKEY_get0_EC_KEY(pkey))
		goto fail;
#endif /* OpenSSL version >= 3.0 */
	*key = pkey;

	return 1;

fail:
	EVP_PKEY_free(pkey);
	return 0;
}

INT8U _convert_der_to_openssl_pkey(const INT8U *priv, const INT32S priv_len, EVP_PKEY **key)
{
	EVP_PKEY *pkey  = NULL;
	EC_KEY   *eckey = NULL;
	INT8U     ret   = 1;

	if (!key)
		return 0;

	pkey = EVP_PKEY_new();

	if (!priv || !priv_len)
		return 0;

	eckey = d2i_ECPrivateKey(NULL, &priv, priv_len);
	if (!eckey) {
		printf("[CRYPTO] d2i_ECPrivateKey failed: %s\n",
		       ERR_error_string(ERR_get_error(), NULL));
		EVP_PKEY_free(pkey);
		return 0;
	}

	if (EVP_PKEY_assign_EC_KEY(pkey, eckey) != 1) {
		printf("[CRYPTO] EVP_PKEY_assign_EC_KEY failed\n");
		EVP_PKEY_free(pkey);
		EC_KEY_free(eckey);
		return 0;
	}

	*key = pkey;
	return ret;
}

INT8U _convert_openssl_pkey_to_subject_public_key_info(EVP_PKEY *key, INT8U *subpkinfo, INT32S *subpkinfo_len)
{
	INT8U                   *der = NULL;
	INT32S                   der_len;
	EC_KEY                  *eckey;
	const EC_GROUP          *group;
	const EC_POINT          *point;
	BN_CTX                  *ctx;
	SUBJECT_PUBLIC_KEY_INFO *bootstrap = NULL;
	INT32S                   nid, len;
	INT8U                    ret = 0;

	if (!key)
		return 0;

	ctx   = BN_CTX_new();
	eckey = EVP_PKEY_get0_EC_KEY(key);
	if (!ctx || !eckey)
		goto fail;

	group = EC_KEY_get0_group(eckey);
	point = EC_KEY_get0_public_key(eckey);
	if (!group || !point)
		goto fail;

	nid = EC_GROUP_get_curve_name(group);
	if (nid == NID_undef)
		goto fail;

	bootstrap = SUBJECT_PUBLIC_KEY_INFO_new();
	if (!bootstrap || X509_ALGOR_set0(bootstrap->alg, OBJ_nid2obj(EVP_PKEY_EC), V_ASN1_OBJECT, (void *)OBJ_nid2obj(nid)) != 1)
		goto fail;

	len = EC_POINT_point2oct(group, point, POINT_CONVERSION_COMPRESSED,
	                         NULL, 0, ctx); // get length for malloc
	if (len == 0)
		goto fail;

	der = OPENSSL_malloc(len);
	if (!der)
		goto fail;

	len = EC_POINT_point2oct(group, point, POINT_CONVERSION_COMPRESSED,
	                         der, len, ctx); // actual conversion

	OPENSSL_free(bootstrap->pub_key->data);
	bootstrap->pub_key->data   = der;
	der                        = NULL;
	bootstrap->pub_key->length = len;

	/* No unused bits */
	bootstrap->pub_key->flags &= ~(ASN1_STRING_FLAG_BITS_LEFT | 0x07);
	bootstrap->pub_key->flags |= ASN1_STRING_FLAG_BITS_LEFT;

	der_len = i2d_SUBJECT_PUBLIC_KEY_INFO(bootstrap, &der);
	if (der_len <= 0) {
		printf("[CRYPTO] Failed to build DER encoded public key");
		goto fail;
	}

	if (der_len > MAX_SUBJECT_PUBKEY_INFO_LEN) {
		printf("[CRYPTO] conversion ok but too long!\n");
		goto fail;
	}

	// copy to user buffer
	memcpy(subpkinfo, der, der_len);
	*subpkinfo_len = der_len;
	ret            = 1;

fail:
	SUBJECT_PUBLIC_KEY_INFO_free(bootstrap);
	OPENSSL_free(der);
	BN_CTX_free(ctx);
	return ret;
}

INT8U _convert_subject_public_key_info_to_openssl_pkey(
    const INT8U *data, const INT32S data_len, EVP_PKEY **key, char **curve)
{
	EVP_PKEY          *_key = NULL;
	const INT8U       *p;
	INT32S             res;
	X509_PUBKEY       *pub = NULL;
	ASN1_OBJECT       *ppkalg;
	const INT8U       *pk;
	INT32S             ppklen;
	X509_ALGOR        *pa;
	const ASN1_OBJECT *pa_oid;
	const void        *pval;
	INT32S             ptype;
	const ASN1_OBJECT *poid;
	char               buf[100];
	char              *_curve = NULL;

	/* DER encoded ASN.1 SubjectPublicKeyInfo
	 *
	 * SubjectPublicKeyInfo  ::=  SEQUENCE  {
	 *      algorithm            AlgorithmIdentifier,
	 *      subjectPublicKey     BIT STRING  }
	 *
	 * AlgorithmIdentifier  ::=  SEQUENCE  {
	 *      algorithm               OBJECT IDENTIFIER,
	 *      parameters              ANY DEFINED BY algorithm OPTIONAL  }
	 *
	 * subjectPublicKey = compressed format public key per ANSI X9.63
	 * algorithm = ecPublicKey (1.2.840.10045.2.1)
	 * parameters = shall be present and shall be OBJECT IDENTIFIER; e.g.,
	 *       prime256v1 (1.2.840.10045.3.1.7)
	 */

	p    = data;
	_key = d2i_PUBKEY(NULL, &p, data_len);

	if (!_key) {
		printf("[CRYPTO] Could not parse public-key of SubjectPublicKeyInfo\n");
		return 0;
	}

	if (EVP_PKEY_type(EVP_PKEY_id(_key)) != EVP_PKEY_EC) {
		printf("[CRYPTO] SubjectPublicKeyInfo does not describe an EC key\n");
		EVP_PKEY_free(_key);
		return 0;
	}

	res = X509_PUBKEY_set(&pub, _key);
	if (res != 1) {
		printf("[CRYPTO] Could not set pubkey\n");
		goto fail;
	}

	res = X509_PUBKEY_get0_param(&ppkalg, &pk, &ppklen, &pa, pub);
	if (res != 1) {
		printf("[CRYPTO] Could not extract SubjectPublicKeyInfo parameters\n");
		goto fail;
	}

	res = OBJ_obj2txt(buf, sizeof(buf), ppkalg, 0);
	if (res < 0 || (size_t)res >= sizeof(buf)) {
		printf("[CRYPTO] Could not extract SubjectPublicKeyInfo algorithm\n");
		goto fail;
	}

	if (strcmp(buf, "id-ecPublicKey") != 0) {
		printf("[CRYPTO] Unsupported SubjectPublicKeyInfo algorithm\n");
		goto fail;
	}

	X509_ALGOR_get0(&pa_oid, &ptype, (void *)&pval, pa);
	if (ptype != V_ASN1_OBJECT) {
		printf("[CRYPTO] SubjectPublicKeyInfo parameters did not contain an OID\n");
		goto fail;
	}

	poid = pval;
	res  = OBJ_obj2txt(buf, sizeof(buf), poid, 0);
	if (res < 0 || (size_t)res >= sizeof(buf)) {
		printf("[CRYPTO] Could not extract SubjectPublicKeyInfo parameters OID\n");
		goto fail;
	}

	if (curve) {
		_curve = malloc(strlen(buf) + 1);
		if (!_curve) {
			printf("[CRYPTO] malloc for curve parameter failed\n");
			goto fail;
		}
		strcpy(_curve, buf);
		*curve = _curve;
	}

	X509_PUBKEY_free(pub);
	*key   = _key;

	return 1;

fail:
	X509_PUBKEY_free(pub);
	EVP_PKEY_free(_key);
	free(_curve);
	return 0;
}

INT8U _convert_openssl_pkey_to_coordinates(EVP_PKEY *key, INT8U *buf_ansi963, INT32S *buf_ansi963_len)
{
	EC_KEY *eckey;
	INT32S  len, res;
	INT8U  *buf = NULL;
	INT8U   ret = 0;

	if (!buf_ansi963 || !buf_ansi963_len)
		return 0;

	eckey = EVP_PKEY_get1_EC_KEY(key);
	if (!eckey)
		return 0;

	// Set ANSI X9.63 standard
	EC_KEY_set_conv_form(eckey, POINT_CONVERSION_UNCOMPRESSED);

	// get length
	len = i2o_ECPublicKey(eckey, NULL);
	if (len <= 0) {
		printf("[CRYPTO] get EVP_PKEY length failed\n");
		goto fail;
	}

	// run actual conversion
	res = i2o_ECPublicKey(eckey, &buf);
	if (res != len) {
		printf("[CRYPTO] coordinates conversion failed\n");
		goto fail;
	}

	if (res > MAX_ANSI_X963_LEN) {
		printf("[CRYPTO] conversion ok but too long!\n");
		goto fail;
	}

	memcpy(buf_ansi963, buf, len);
	*buf_ansi963_len = len;
	ret              = 1;

fail:
	EC_KEY_free(eckey);
	free(buf);
	return ret;
}

INT8U _convert_coordinates_to_openssl_pkey(const char *curve_name, INT8U *buf_ansi963, INT32S buf_ansi963_len, EVP_PKEY **key)
{
	EC_GROUP *group;
	BN_CTX   *ctx;
	EC_KEY   *eckey = NULL;
	EC_POINT *point = NULL;
	BIGNUM   *x = NULL, *y = NULL;
	EVP_PKEY *pkey = NULL;
	INT8U     ret  = 0;
	INT32S    nid, len = 0;

	if (!buf_ansi963 || !buf_ansi963_len)
		return 0;

	// Get OpenSSL parameters from specified curve
	nid = OBJ_txt2nid(curve_name);
	if (nid == NID_undef)
		return 0;

	group = EC_GROUP_new_by_curve_name(nid);
	if (!group)
		return 0;

	ctx = BN_CTX_new();
	if (!ctx) {
		printf("[CRYPTO] cannot initialize ctx\n");
		goto fail;
	}

	// ANSI X9.63 standard ( 0x04 || X || Y )
	len   = (buf_ansi963_len - 1) / 2;
	x     = BN_bin2bn(buf_ansi963 + 1, len, NULL);
	y     = BN_bin2bn(buf_ansi963 + 1 + len, len, NULL);
	point = EC_POINT_new(group);
	if (!point || !x || !y) {
		printf("[CRYPTO] cannot initialize BN or EC_POINT\n");
		goto fail;
	}

	if (!EC_POINT_set_affine_coordinates_GFp(group, point, x, y, ctx)) {
		printf("[CRYPTO] EC_POINT_set_affine_coordinates_GFp failed: %s\n",
		       ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	if (!EC_POINT_is_on_curve(group, point, ctx) || EC_POINT_is_at_infinity(group, point)) {
		printf("[CRYPTO] Invalid point\n");
		goto fail;
	}

	eckey = EC_KEY_new();
	if (!eckey || EC_KEY_set_group(eckey, group) != 1 || EC_KEY_set_public_key(eckey, point) != 1) {
		printf("[CRYPTO] Failed to set EC_KEY: %s\n",
		       ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	EC_KEY_set_asn1_flag(eckey, OPENSSL_EC_NAMED_CURVE);

	pkey = EVP_PKEY_new();
	if (!pkey) {
		printf("[CRYPTO] cannot create EVP_PKEY\n");
		goto fail;
	}

	if (EVP_PKEY_set1_EC_KEY(pkey, eckey) != 1) {
		printf("[CRYPTO] cannot set EC_KEY to PKEY\n");
		EVP_PKEY_free(pkey);
		goto fail;
	}

	*key = pkey;
	ret  = 1;

fail:
	BN_free(x);
	BN_free(y);
	EC_POINT_free(point);
	EC_GROUP_free(group);
	EC_KEY_free(eckey);
	BN_CTX_free(ctx);
	return ret;
}

EC_POINT *_convert_public_der_to_openssl_public_key(ECCKeyPair *keypair)
{
	const EC_KEY *  eckey      = NULL;
	EVP_PKEY *      pkey       = NULL;
	const EC_POINT *point      = NULL;
	const EC_GROUP *group      = NULL;
	EC_POINT *public_key = NULL;

	// Convert public key to EVP_PKEY
	if (_convert_subject_public_key_info_to_openssl_pkey(keypair->subpkinfo, keypair->subpkinfo_len, &pkey, NULL) != 1) {
		printf("[CRYPTO] cannot convert pub to EVP_PKEY\n");
		return NULL;
	}

	eckey = EVP_PKEY_get0_EC_KEY(pkey);
	if (!eckey)
		goto fail;

	group = EC_KEY_get0_group(eckey);
	if (!group)
		goto fail;

	point = EC_KEY_get0_public_key(eckey);
	if (!point)
		goto fail;

	public_key = EC_POINT_dup(point, group);

fail:
	EVP_PKEY_free(pkey);
	return public_key;
}

BIGNUM *_convert_der_to_openssl_private_key(ECCKeyPair *keypair)
{
	const EC_KEY *eckey       = NULL;
	EVP_PKEY *    pkey        = NULL;
	const BIGNUM *bn          = NULL;
	BIGNUM *private_key = NULL;

	// Convert private key to EVP_PKEY
	if (_convert_der_to_openssl_pkey(keypair->priv, keypair->priv_len, &pkey) != 1) {
		printf("[CRYPTO] cannot convert priv to EVP_PKEY\n");
		return NULL;
	}

	eckey = EVP_PKEY_get0_EC_KEY(pkey);
	if (!eckey)
		goto fail;

	bn = EC_KEY_get0_private_key(eckey);
	if (!bn)
		goto fail;

	private_key = BN_dup(bn);

fail:
	EVP_PKEY_free(pkey);
	return private_key;
}

INT8U PLATFORM_GENERATE_EC_KEYPAIR(const char *curve_name, ECCKeyPair *keypair)
{
	EVP_PKEY_CTX *kctx      = NULL;
	EC_KEY       *ec_params = NULL;
	EVP_PKEY     *params = NULL, *key = NULL;
	INT32S        nid;

	INT8U  priv[MAX_ASN1_DER_PRIVKEY_LEN];
	INT8U  pub_subpki[MAX_SUBJECT_PUBKEY_INFO_LEN];
	INT8U  pub_ansix963[MAX_ANSI_X963_LEN];
	INT32S priv_len = 0, pub_subpki_len = 0, pub_ansix963_len = 0;

	INT8U ret = 0;

	// Get OpenSSL parameters from specified curve
	nid = OBJ_txt2nid(curve_name);
	if (nid == NID_undef)
		return 0;

	ec_params = EC_KEY_new_by_curve_name(nid);
	if (!ec_params)
		goto fail;

	// Load and configure parameters
	EC_KEY_set_asn1_flag(ec_params, OPENSSL_EC_NAMED_CURVE);
	params = EVP_PKEY_new();
	if (!params || EVP_PKEY_set1_EC_KEY(params, ec_params) != 1)
		goto fail;

	// Use loaded parameters to generate asymmetric key
	kctx = EVP_PKEY_CTX_new(params, NULL);
	if (!kctx || EVP_PKEY_keygen_init(kctx) != 1 || EVP_PKEY_keygen(kctx, &key) != 1)
		goto fail;

	// Convert private key to ASN.1 DER binary format
	ret = _convert_openssl_pkey_to_der(key, priv, &priv_len);
	if (ret != 1) {
		printf("[CRYPTO] cannot convert privkey to ASN.1 DER\n");
		goto fail;
	}

	// Convert public key to SubjectPublicKeyInfo format
	ret = _convert_openssl_pkey_to_subject_public_key_info(key, pub_subpki, &pub_subpki_len);
	if (ret != 1) {
		printf("[CRYPTO] cannot convert pubkey to SubjectPublicKeyInfo\n");
		goto fail;
	}

	// Convert public key to ANSI X9.63 format
	ret = _convert_openssl_pkey_to_coordinates(key, pub_ansix963, &pub_ansix963_len);
	if (ret != 1) {
		printf("[CRYPTO] cannot convert pubkey to coordinates\n");
		goto fail;
	}

	// Copy everything to keypair
	keypair->type = PRIVATE_KEY;
	strncpy(keypair->curve, curve_name, MAX_CURVE_NAME_LEN - 1);
	memcpy(keypair->priv, priv, priv_len);
	keypair->priv_len = priv_len;
	memcpy(keypair->subpkinfo, pub_subpki, pub_subpki_len);
	keypair->subpkinfo_len = pub_subpki_len;
	memcpy(keypair->pub_ansix963, pub_ansix963, pub_ansix963_len);
	keypair->pub_ansix963_len = pub_ansix963_len;

fail:
	EC_KEY_free(ec_params);
	EVP_PKEY_free(params);
	EVP_PKEY_CTX_free(kctx);
	EVP_PKEY_free(key);
	return ret;
}

INT8U PLATFORM_PARSE_EC_SUBJECT_PUBLIC_KEY_INFO(const INT8U *subpkinfo, const INT32S subpkinfo_len, ECCKeyPair *keypair)
{
	EVP_PKEY *pubkey = NULL;
	char     *curve  = NULL;
	INT8U     ret    = 0;
	INT8U     pub_ansix963[MAX_ANSI_X963_LEN];
	INT32S    pub_ansix963_len = 0;

	if (subpkinfo_len > MAX_SUBJECT_PUBKEY_INFO_LEN) {
		printf("[CRYPTO] the SubjectPublicKeyInfo is too long\n");
		return 0;
	}

	// Convert SubjectPublicKeyInfo to EVP_PKEY pubkey
	if (_convert_subject_public_key_info_to_openssl_pkey(subpkinfo, subpkinfo_len, &pubkey, &curve) != 1) {
		printf("[CRYPTO] cannot convert SubjectPublicKeyInfo to EVP_PKEY\n");
		goto fail;
	}

	// Convert EVP_PKEY pubkey to ANSI X9.63 format
	if (_convert_openssl_pkey_to_coordinates(pubkey, pub_ansix963, &pub_ansix963_len) != 1) {
		printf("[CRYPTO] cannot convert pubkey to coordinates\n");
		goto fail;
	}

	// Copy everything to keypair
	memcpy(keypair->subpkinfo, subpkinfo, subpkinfo_len);
	keypair->subpkinfo_len = subpkinfo_len;
	memcpy(keypair->pub_ansix963, pub_ansix963, pub_ansix963_len);
	keypair->pub_ansix963_len = pub_ansix963_len;
	strncpy(keypair->curve, curve, MAX_CURVE_NAME_LEN - 1);
	keypair->type = PUBLIC_KEY;
	ret           = 1;

fail:
	EVP_PKEY_free(pubkey);
	free(curve);
	return ret;
}

INT8U PLATFORM_PARSE_EC_PUBLIC_ANSIX963(const char *curve_name, INT8U *pub_ansi963, INT32S pub_ansi963_len, ECCKeyPair *keypair)
{
	EVP_PKEY *pubkey = NULL;
	INT8U     subpkinfo[MAX_SUBJECT_PUBKEY_INFO_LEN];
	INT32S    subpkinfo_len = 0;
	INT8U     ret           = 0;

	if (pub_ansi963_len > MAX_ANSI_X963_LEN) {
		printf("[CRYPTO] the input coordinates are too long\n");
		return 0;
	}

	// Convert ANSI X9.63 format pubkey to EVP_PKEY pubkey
	if (_convert_coordinates_to_openssl_pkey(curve_name, pub_ansi963, pub_ansi963_len, &pubkey) != 1) {
		printf("[CRYPTO] cannot parse the coordinates into EVP_PKEY\n");
		goto fail;
	}

	// Convert EVP_PKEY pubkey to EVP_PKEY SubjectPublicKeyInfo
	if (_convert_openssl_pkey_to_subject_public_key_info(pubkey, subpkinfo, &subpkinfo_len) != 1) {
		printf("[CRYPTO] cannot convert pubkey to SubjectPublicKeyInfo\n");
		goto fail;
	}

	// Copy everything to keypair
	memcpy(keypair->subpkinfo, subpkinfo, subpkinfo_len);
	keypair->subpkinfo_len = subpkinfo_len;
	memcpy(keypair->pub_ansix963, pub_ansi963, pub_ansi963_len);
	keypair->pub_ansix963_len = pub_ansi963_len;
	strncpy(keypair->curve, curve_name, MAX_CURVE_NAME_LEN - 1);
	keypair->type = PUBLIC_KEY;
	ret           = 1;

fail:
	EVP_PKEY_free(pubkey);
	return ret;
}

INT8U PLATFORM_PARSE_EC_PUBLIC_KEY(INT8U *pub, INT32S pub_len, ECCKeyPair *keypair)
{
	EVP_PKEY       *pubkey = NULL;
	const EC_KEY   *eckey  = NULL;
	const EC_GROUP *group  = NULL;
	INT32S          nid    = 0;
	const char     *curve  = NULL;
	INT8U           subpkinfo[MAX_SUBJECT_PUBKEY_INFO_LEN];
	INT32S          subpkinfo_len = 0;
	INT8U           pub_ansix963[MAX_ANSI_X963_LEN];
	INT32S          pub_ansix963_len = 0;
	INT8U           ret              = 0;

	if (pub_len > MAX_ASN1_DER_PUBKEY_LEN) {
		printf("[CRYPTO] the input coordinates are too long\n");
		return 0;
	}

	// Convert pubkey to EVP_PKEY pubkey
	if (_ec_key_parse_pub(pub, pub_len, &pubkey) != 1) {
		printf("[CRYPTO] cannot parse the coordinates into EVP_PKEY\n");
		goto fail;
	}

	// Convert EVP_PKEY pubkey to ANSI X9.63 format
	if (_convert_openssl_pkey_to_coordinates(pubkey, pub_ansix963, &pub_ansix963_len) != 1) {
		printf("[CRYPTO] cannot convert pubkey to coordinates\n");
		goto fail;
	}

	// Convert EVP_PKEY pubkey to EVP_PKEY SubjectPublicKeyInfo
	if (_convert_openssl_pkey_to_subject_public_key_info(pubkey, subpkinfo, &subpkinfo_len) != 1) {
		printf("[CRYPTO] cannot convert pubkey to SubjectPublicKeyInfo\n");
		goto fail;
	}

	// get and verify curve information
	eckey = EVP_PKEY_get0_EC_KEY(pubkey);
	if (!eckey) {
		printf("[CRYPTO] EVP_PKEY_get0_EC_KEY cannot get key\n");
		goto fail;
	}

	group = EC_KEY_get0_group(eckey);
	if (!group) {
		printf("[CRYPTO] EC_KEY_get0_group cannot get group\n");
		goto fail;
	}

	nid = EC_GROUP_get_curve_name(group);
	if (nid == NID_undef) {
		printf("[CRYPTO] NID is undefined\n");
		goto fail;
	}

	curve = OBJ_nid2ln(nid);
	if (!curve) {
		printf("[CRYPTO] cannot get long name from NID\n");
		goto fail;
	}

	// Copy everything to keypair
	memcpy(keypair->subpkinfo, subpkinfo, subpkinfo_len);
	keypair->subpkinfo_len = subpkinfo_len;
	memcpy(keypair->pub_ansix963, pub_ansix963, pub_ansix963_len);
	keypair->pub_ansix963_len = pub_ansix963_len;
	strncpy(keypair->curve, curve, MAX_CURVE_NAME_LEN - 1);
	keypair->type = PUBLIC_KEY;
	ret           = 1;

fail:
	EVP_PKEY_free(pubkey);
	return ret;
}

INT8U PLATFORM_PARSE_EC_PRIVATE_KEY(INT8U *priv, INT32S priv_len, ECCKeyPair *keypair)
{
	EVP_PKEY       *key   = NULL;
	const EC_KEY   *eckey = NULL;
	const EC_GROUP *group = NULL;
	INT8U           ret   = 0;
	INT32S          nid   = 0;
	const char     *curve = NULL;

	INT8U           pub_subpki[MAX_SUBJECT_PUBKEY_INFO_LEN];
	INT8U           pub_ansix963[MAX_ANSI_X963_LEN];
	INT32S          pub_subpki_len = 0, pub_ansix963_len = 0;

	if (!priv || !priv_len || !keypair || priv_len > MAX_ASN1_DER_PRIVKEY_LEN)
		return 0;

	// Convert private key to EVP_PKEY
	if (_convert_der_to_openssl_pkey(priv, priv_len, &key) != 1) {
		printf("[CRYPTO] cannot convert priv to EVP_PKEY\n");
		return 0;
	}

	// get and verify curve information
	eckey = EVP_PKEY_get0_EC_KEY(key);
	if (!eckey) {
		printf("[CRYPTO] EVP_PKEY_get0_EC_KEY cannot get key\n");
		goto fail;
	}

	group = EC_KEY_get0_group(eckey);
	if (!group) {
		printf("[CRYPTO] EC_KEY_get0_group cannot get group\n");
		goto fail;
	}

	nid = EC_GROUP_get_curve_name(group);
	if (nid == NID_undef) {
		printf("[CRYPTO] NID is undefined\n");
		goto fail;
	}

	curve = OBJ_nid2ln(nid);
	if (!curve) {
		printf("[CRYPTO] cannot get long name from NID\n");
		goto fail;
	}

	// Convert pubkey to SubjectPublicKeyInfo format
	ret = _convert_openssl_pkey_to_subject_public_key_info(key, pub_subpki, &pub_subpki_len);
	if (ret != 1) {
		printf("[CRYPTO] cannot convert pubkey to SubjectPublicKeyInfo\n");
		goto fail;
	}

	// Convert pubkey to ANSI X9.63 format
	ret = _convert_openssl_pkey_to_coordinates(key, pub_ansix963, &pub_ansix963_len);
	if (ret != 1) {
		printf("[CRYPTO] cannot convert pubkey to coordinates\n");
		goto fail;
	}

	// Copy everything to keypair
	strncpy(keypair->curve, curve, MAX_CURVE_NAME_LEN - 1);
	memcpy(keypair->priv, priv, priv_len);
	keypair->priv_len = priv_len;
	memcpy(keypair->subpkinfo, pub_subpki, pub_subpki_len);
	keypair->subpkinfo_len = pub_subpki_len;
	memcpy(keypair->pub_ansix963, pub_ansix963, pub_ansix963_len);
	keypair->pub_ansix963_len = pub_ansix963_len;
	keypair->type             = PRIVATE_KEY;
	ret                       = 1;

fail:
	EVP_PKEY_free(key);
	return ret;
}

INT8U PLATFORM_COMPUTE_ECDH_SHARED_SECRET(ECCKeyPair *priv, ECCKeyPair *pub, INT8U *secret, size_t *secret_len)
{
	EVP_PKEY_CTX *ctx = NULL;
	EVP_PKEY     *own = NULL, *peer = NULL;
	char	     *curve = NULL;
	INT8U         ret   = 0;

	ERR_clear_error();
	*secret_len = 0;

	// Convert private key to EVP_PKEY
	if (_convert_der_to_openssl_pkey(priv->priv, priv->priv_len, &own) != 1) {
		printf("[CRYPTO] cannot convert priv to EVP_PKEY\n");
		return 0;
	}

	// Convert public key to EVP_PKEY
	if (_convert_subject_public_key_info_to_openssl_pkey(pub->subpkinfo, pub->subpkinfo_len, &peer, &curve) != 1) {
		printf("[CRYPTO] cannot convert pub to EVP_PKEY\n");
		goto fail;
	}

	ctx = EVP_PKEY_CTX_new(own, NULL);
	if (!ctx) {
		printf("[CRYPTO] EVP_PKEY_CTX_new failed: %s",
		       ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	if (EVP_PKEY_derive_init(ctx) != 1) {
		printf("[CRYPTO] EVP_PKEY_derive_init failed: %s",
		       ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	if (EVP_PKEY_derive_set_peer(ctx, peer) != 1) {
		printf("[CRYPTO] EVP_PKEY_derive_set_peet failed: %s",
		       ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	// get length
	if (EVP_PKEY_derive(ctx, NULL, secret_len) != 1) {
		printf("[CRYPTO] EVP_PKEY_derive(NULL) failed: %s",
		       ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	if (*secret_len > MAX_SHARED_SECRET_LEN) {
		INT8U buf[200];

		/* It looks like OpenSSL can return unexpectedly large buffer
		 * need for shared secret from EVP_PKEY_derive(NULL) in some
		 * cases. For example, group 19 has shown cases where secret_len
		 * is set to 72 even though the actual length ends up being
		 * updated to 32 when EVP_PKEY_derive() is called with a buffer
		 * for the value. Work around this by trying to fetch the value
		 * and continue if it is within supported range even when the
		 * initial buffer need is claimed to be larger. */
		printf("[CRYPTO] Unexpected secret_len=%d from EVP_PKEY_derive()\n", (INT32S)*secret_len);
		if (*secret_len > 200)
			goto fail;

		// actual derivation
		if (EVP_PKEY_derive(ctx, buf, secret_len) != 1) {
			printf("[CRYPTO] EVP_PKEY_derive failed: %s",
			       ERR_error_string(ERR_get_error(), NULL));
			goto fail;
		}
		if (*secret_len > MAX_SHARED_SECRET_LEN) {
			printf("[CRYPTO] Unexpected secret_len=%d from EVP_PKEY_derive()\n", (INT32S)*secret_len);
			goto fail;
		}
		printf("[CRYPTO] Unexpected secret_len changed to %d\n", (INT32S)*secret_len);
		memcpy(secret, buf, *secret_len);
		goto done;
	}

	// actual derivation
	if (EVP_PKEY_derive(ctx, secret, secret_len) != 1) {
		printf("[CRYPTO] EVP_PKEY_derive failed: %s", ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

done:
	ret = 1;

fail:
	EVP_PKEY_CTX_free(ctx);
	EVP_PKEY_free(own);
	EVP_PKEY_free(peer);
	free(curve);
	return ret;
}

INT8U PLATFORM_ECDSA_SIGN(INT8U hashlen, ECCKeyPair *priv, INT8U num_elem, INT8U **addr, INT32U *len, INT8U *signature, INT16U signature_len)
{
	const int     prime_len = signature_len / 2;
	const EVP_MD *type      = NULL;
	EVP_MD_CTX   *md_ctx    = NULL;
	EVP_PKEY     *priv_pkey = NULL;
	INT8U        *der       = NULL;
	const INT8U  *p         = NULL;
	size_t        der_len   = 0;
	ECDSA_SIG    *sig       = NULL;
	const BIGNUM *r, *s;
	INT8U         ret = 0;
	size_t        i;

	switch (hashlen) {
	case SHA256_MAC_LEN:
		type = EVP_sha256();
		break;
	case SHA384_MAC_LEN:
		type = EVP_sha384();
		break;
	case SHA512_MAC_LEN:
		type = EVP_sha512();
		break;
	default:
		return 0;
	}

	// Convert private key to EVP_PKEY
	if (_convert_der_to_openssl_pkey(priv->priv, priv->priv_len, &priv_pkey) != 1) {
		printf("[CRYPTO] cannot convert priv to EVP_PKEY\n");
		goto fail;
	}

	// Initialize envelope functions
	md_ctx = EVP_MD_CTX_create();
	if (!md_ctx) {
		printf("[CRYPTO] md_ctx returned empty!\n");
		goto fail;
	}

	ERR_clear_error();
	if (EVP_DigestSignInit(md_ctx, NULL, type, NULL, priv_pkey) != 1) {
		printf("[CRYPTO] EVP_DigestSignInit failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	// Insert data
	for (i = 0; i < num_elem; i++) {
		if (EVP_DigestSignUpdate(md_ctx, addr[i], len[i]) != 1) {
			printf("[CRYPTO] EVP_DigestSignUpdate failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
			goto fail;
		}
	}

	// Get ECDSA signature length
	if (EVP_DigestSignFinal(md_ctx, NULL, &der_len) != 1) {
		printf("[CRYPTO] EVP_DigestSignFinal failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	if (!(der = malloc(der_len))) {
		printf("[CRYPTO] malloc for signature failed\n");
		goto fail;
	}

	// Sign ECDSA
	if (EVP_DigestSignFinal(md_ctx, der, &der_len) != 1) {
		printf("[CRYPTO] EVP_DigestSignFinal failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	// Convert DER formatted signature to r,s coordinates
	p = der;
	if (!(sig = d2i_ECDSA_SIG(NULL, &p, der_len))) {
		printf("[CRYPTO] d2i_ECDSA_SIG failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	ECDSA_SIG_get0(sig, &r, &s);

	// Save (r,s) to rs buffer
	if (!signature
	    || BN_bn2binpad(r, signature, prime_len) != prime_len
	    || BN_bn2binpad(s, signature + prime_len, prime_len) != prime_len) {
		printf("[CRYPTO] cannot convert big number to binary\n");
		goto fail;
	}

	ret = 1;

fail:
	EVP_PKEY_free(priv_pkey);
	EVP_MD_CTX_destroy(md_ctx);
	ECDSA_SIG_free(sig);
	free(der);
	return ret;
}

INT8U PLATFORM_ECDSA_VERIFY(INT8U hashlen, ECCKeyPair *pub, INT8U num_elem, INT8U **addr, INT32U *len, INT8U *signature, INT16U signature_len)
{
	const EVP_MD  *type = NULL;
	ECDSA_SIG     *sig  = NULL;
	BIGNUM        *r    = NULL;
	BIGNUM        *s    = NULL;
	unsigned char *der  = NULL;
	int            der_len;
	EVP_MD_CTX    *md_ctx = NULL;
	EVP_PKEY      *pubkey = NULL;
	char          *curve  = NULL;
	INT8U          ret    = 0;
	size_t         i;

	switch (hashlen) {
	case SHA256_MAC_LEN:
		type = EVP_sha256();
		break;
	case SHA384_MAC_LEN:
		type = EVP_sha384();
		break;
	case SHA512_MAC_LEN:
		type = EVP_sha512();
		break;
	default:
		return 0;
	}

	// Convert (r,s) to ECDSA_SIG
	r   = BN_bin2bn(signature, signature_len / 2, NULL);
	s   = BN_bin2bn(signature + signature_len / 2, signature_len / 2, NULL);
	sig = ECDSA_SIG_new();
	if (!r || !s || !sig || ECDSA_SIG_set0(sig, r, s) != 1) {
		printf("[CRYPTO] r, s, or sig init error!\n");
		return 0;
	}
	r = NULL;
	s = NULL;

	// Convert ECDSA_SIG to DER encoded string
	der_len = i2d_ECDSA_SIG(sig, &der);
	if (der_len <= 0) {
		printf("[CRYPTO] Could not DER encode signature!\n");
		goto fail;
	}

	// Convert public key to EVP_PKEY pubkey
	if (_convert_subject_public_key_info_to_openssl_pkey(pub->subpkinfo, pub->subpkinfo_len, &pubkey, &curve) != 1) {
		printf("[CRYPTO] cannot convert SubjectPublicKeyInfo to EVP_PKEY\n");
		goto fail;
	}

	// Initialize envelope functions
	md_ctx = EVP_MD_CTX_create();
	if (!md_ctx) {
		printf("[CRYPTO] md_ctx returned empty!\n");
		goto fail;
	}

	ERR_clear_error();
	if (EVP_DigestVerifyInit(md_ctx, NULL, type, NULL, pubkey) != 1) {
		printf("[CRYPTO] EVP_DigestVerifyInit failed: %s", ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	// Insert data
	for (i = 0; i < num_elem; i++) {
		if (EVP_DigestVerifyUpdate(md_ctx, addr[i], len[i]) != 1) {
			printf("[CRYPTO] EVP_DigestVerifyUpdate failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
			goto fail;
		}
	}

	// Verify ECDSA
	if (EVP_DigestVerifyFinal(md_ctx, der, der_len) != 1) {
		printf("[CRYPTO]  EVP_DigestVerifyFinal failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	ret = 1;

fail:
	BN_free(r);
	BN_free(s);
	ECDSA_SIG_free(sig);
	OPENSSL_free(der);
	EVP_MD_CTX_destroy(md_ctx);
	EVP_PKEY_free(pubkey);
	free(curve);
	return ret;
}

INT16U PLATFORM_EC_KEY_GET_GROUP(ECCKeyPair *keypair)
{
	const EC_KEY   *eckey     = NULL;
	const EC_GROUP *group     = NULL;
	EVP_PKEY       *priv_pkey = NULL;
	INT32S          nid       = 0;

	// Convert private key to EVP_PKEY
	if (_convert_der_to_openssl_pkey(keypair->priv, keypair->priv_len, &priv_pkey) != 1) {
		printf("[CRYPTO] cannot convert priv to EVP_PKEY\n");
		goto fail;
	}

	eckey = EVP_PKEY_get0_EC_KEY(priv_pkey);
	if (!eckey)
		goto fail;

	group = EC_KEY_get0_group(eckey);
	if (!group)
		goto fail;

	nid = EC_GROUP_get_curve_name(group);
	if (nid == NID_undef) {
		printf("[CRYPTO] NID is undefined\n");
		goto fail;
	}

fail:
	EVP_PKEY_free(priv_pkey);
	return _nid_to_ec_group(nid);
}

INT8U PLATFORM_EC_KEY_GET_PUBLIC_POINT(ECCKeyPair *keypair, INT8U *pubkey_point, INT32S *pubkey_point_len)
{
	EVP_PKEY *pkey = NULL;

	INT8U ret = 0;

	// Convert compressed public key to EVP_PKEY
	if (_ec_key_parse_pub(keypair->subpkinfo, keypair->subpkinfo_len, &pkey) != 1) {
		printf("[CRYPTO] cannot convert pub to EVP_PKEY\n");
		goto fail;
	}

	// Convert EVP_PKEY to coordinates
	if (_convert_openssl_pkey_to_coordinates(pkey, pubkey_point, pubkey_point_len) != 1) {
		printf("[CRYPTO] cannot convert EVP_PKEY to coordinate\n");
		goto fail;
	}

	ret = 1;

fail:
	EVP_PKEY_free(pkey);
	return ret;
}

INT8U PLATFORM_EC_KEY_SET_PUBLIC_KEY(INT16U group, INT8U *buf_x, INT8U *buf_y, size_t len, ECCKeyPair *keypair)
{
	EC_KEY     *eckey    = NULL;
	EVP_PKEY   *pkey     = NULL;
	EC_GROUP   *ec_group = NULL;
	BN_CTX     *ctx      = NULL;
	EC_POINT   *point    = NULL;
	BIGNUM     *x = NULL, *y = NULL;
	INT32S      nid = 0;
	const char *curve = NULL;
	INT8U       ret   = 0;

	INT8U       pub_ansix963[MAX_ANSI_X963_LEN] = {0};
	INT8U       pub_subpki[MAX_SUBJECT_PUBKEY_INFO_LEN] = {0};
	INT32S      pub_ansix963_len = 0, pub_subpki_len = 0;

	if (!buf_x || !buf_y)
		return 0;

	nid = _ec_group_to_nid(group);
	if (nid < 0) {
		printf("[CRYPTO] Unsupported group %u\n", group);
		return 0;
	}

	curve = OBJ_nid2ln(nid);
	if (!curve) {
		printf("[CRYPTO] cannot get long name from NID\n");
		goto fail;
	}

	ctx = BN_CTX_new();
	if (!ctx)
		goto fail;

	ec_group = EC_GROUP_new_by_curve_name(nid);
	if (!ec_group)
		goto fail;

	x     = BN_bin2bn(buf_x, len, NULL);
	y     = BN_bin2bn(buf_y, len, NULL);
	point = EC_POINT_new(ec_group);
	if (!x || !y || !point)
		goto fail;

	if (!EC_POINT_set_affine_coordinates_GFp(ec_group, point, x, y, ctx)) {
		printf("[CRYPTO] EC_POINT_set_affine_coordinates_GFp failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}

	if (!EC_POINT_is_on_curve(ec_group, point, ctx) || EC_POINT_is_at_infinity(ec_group, point)) {
		printf("[CRYPTO] Invalid point\n");
		goto fail;
	}

	eckey = EC_KEY_new();
	if (!eckey || EC_KEY_set_group(eckey, ec_group) != 1 || EC_KEY_set_public_key(eckey, point) != 1) {
		printf("[CRYPTO] Failed to set EC_KEY: %s\n", ERR_error_string(ERR_get_error(), NULL));
		EC_KEY_free(eckey);
		goto fail;
	}
	EC_KEY_set_asn1_flag(eckey, OPENSSL_EC_NAMED_CURVE);

	pkey = EVP_PKEY_new();
	if (!pkey || EVP_PKEY_assign_EC_KEY(pkey, eckey) != 1) {
		printf("[CRYPTO] Could not create EVP_PKEY\n");
		EC_KEY_free(eckey);
		goto fail;
	}

	// Convert pubkey to ANSI X9.63 format
	ret = _convert_openssl_pkey_to_coordinates(pkey, pub_ansix963, &pub_ansix963_len);
	if (ret != 1) {
		printf("[CRYPTO] cannot convert pubkey to coordinates\n");
		goto fail;
	}

	// Convert pubkey to SubjectPublicKeyInfo format
	ret = _convert_openssl_pkey_to_subject_public_key_info(pkey, pub_subpki, &pub_subpki_len);
	if (ret != 1) {
		printf("[CRYPTO] cannot convert pubkey to SubjectPublicKeyInfo\n");
		goto fail;
	}

	// Copy everything to keypair
	keypair->type = PUBLIC_KEY;
	memcpy(keypair->subpkinfo, pub_subpki, pub_subpki_len);
	keypair->subpkinfo_len = pub_subpki_len;
	memcpy(keypair->pub_ansix963, pub_ansix963, pub_ansix963_len);
	keypair->pub_ansix963_len = pub_ansix963_len;
	strncpy(keypair->curve, curve, MAX_CURVE_NAME_LEN - 1);

	ret = 1;

fail:
	EVP_PKEY_free(pkey);
	pkey = NULL;
	EC_GROUP_free(ec_group);
	BN_free(x);
	BN_free(y);
	EC_POINT_free(point);
	BN_CTX_free(ctx);
	return ret;
}

INT8U PLATFORM_EC_KEY_SET_PUBLIC_POINT(INT16U group, const EC_POINT *pub_point, ECCKeyPair *keypair)
{
	EC_KEY     *eckey = NULL;
	EVP_PKEY   *pkey     = NULL;
	EC_GROUP   *ec_group = NULL;
	INT32S      nid      = 0;
	const char *curve    = NULL;

	INT8U  pub_subpki[MAX_SUBJECT_PUBKEY_INFO_LEN] = { 0 };
	INT8U  pub_ansix963[MAX_ANSI_X963_LEN]         = { 0 };
	INT32S pub_subpki_len = 0, pub_ansix963_len = 0;

	INT8U ret = 0;

	nid = _ec_group_to_nid(group);
	if (nid < 0) {
		printf("[CRYPTO] Unsupported group %u\n", group);
		return 0;
	}

	curve = OBJ_nid2ln(nid);
	if (!curve) {
		printf("[CRYPTO] cannot get long name from NID\n");
		goto fail;
	}

	ec_group = EC_GROUP_new_by_curve_name(nid);
	if (!ec_group)
		goto fail;

	eckey = EC_KEY_new();
	if (!eckey || EC_KEY_set_group(eckey, ec_group) != 1 || EC_KEY_set_public_key(eckey, (const EC_POINT *)pub_point) != 1) {
		printf("[CRYPTO] Failed to set EC_KEY: %s\n", ERR_error_string(ERR_get_error(), NULL));
		EC_KEY_free(eckey);
		goto fail;
	}

	pkey = EVP_PKEY_new();
	if (!pkey || EVP_PKEY_assign_EC_KEY(pkey, eckey) != 1) {
		printf("[CRYPTO] cannot create EVP_PKEY\n");
		EC_KEY_free(eckey);
		goto fail;
	}

	// Convert EVP_PKEY pubkey to EVP_PKEY SubjectPublicKeyInfo
	if (_convert_openssl_pkey_to_subject_public_key_info(pkey, pub_subpki, &pub_subpki_len) != 1) {
		printf("[CRYPTO] cannot convert pubkey to SubjectPublicKeyInfo\n");
		goto fail;
	}

	// Convert EVP_PKEY pubkey to ANSI X9.63 format
	if (_convert_openssl_pkey_to_coordinates(pkey, pub_ansix963, &pub_ansix963_len) != 1) {
		printf("[CRYPTO] cannot convert pubkey to coordinates\n");
		goto fail;
	}

	keypair->type = PUBLIC_KEY;
	strncpy(keypair->curve, curve, MAX_CURVE_NAME_LEN - 1);
	memcpy(keypair->subpkinfo, pub_subpki, pub_subpki_len);
	keypair->subpkinfo_len = pub_subpki_len;
	memcpy(keypair->pub_ansix963, pub_ansix963, pub_ansix963_len);
	keypair->pub_ansix963_len = pub_ansix963_len;

	ret = 1;

fail:
	EVP_PKEY_free(pkey);
	pkey = NULL;
	EC_GROUP_free(ec_group);
	return ret;
}

INT8U PLATFORM_EC_KEY_ADD_PUBLIC_KEY(ECCKeyPair *a, ECCKeyPair *b, const char *curve_name, ECCKeyPair *res)
{
	EC_POINT *ecp_sum = NULL, *ecp_a = NULL, *ecp_b = NULL;
	EC_GROUP *group = NULL;
	BN_CTX   *ctx   = NULL;
	EC_KEY   *eckey = NULL;
	EVP_PKEY *pkey  = NULL;

	INT8U  subpkinfo[MAX_SUBJECT_PUBKEY_INFO_LEN];
	INT8U  pub_ansix963[MAX_ANSI_X963_LEN];
	INT32S subpkinfo_len = 0, pub_ansix963_len = 0;

	INT32S nid = 0;
	INT8U  ret = 0;

	if (!a || !b)
		return 0;

	// Get EC group from curve
	nid   = OBJ_txt2nid(curve_name);
	group = EC_GROUP_new_by_curve_name(nid);
	if (!group)
		goto fail;

	ctx = BN_CTX_new();
	if (!ctx)
		goto fail;

	/* ecp_sum = ecp_a + ecp_b */
	ecp_sum = EC_POINT_new(group);
	ecp_a   = _convert_public_der_to_openssl_public_key(a);
	ecp_b   = _convert_public_der_to_openssl_public_key(b);
	if (!ecp_sum || !ecp_a || !ecp_b
	    || _ssl_ec_point_add(group, ecp_a, ecp_b, ecp_sum, ctx)) {
		printf("[CRYPTO] Failure in EC_POINT calculation\n");
		goto fail;
	}

	// Check validity of ecp_sum
	if (!EC_POINT_is_on_curve(group, ecp_sum, ctx) || EC_POINT_is_at_infinity(group, ecp_sum)) {
		printf("[CRYPTO] Invalid point\n");
		goto fail;
	}

	// Convert ecp_sum into EC_KEY format
	eckey = EC_KEY_new();
	if (!eckey || EC_KEY_set_group(eckey, group) != 1 || EC_KEY_set_public_key(eckey, ecp_sum) != 1) {
		printf("[CRYPTO] Failed to set EC_KEY: %s\n",
		       ERR_error_string(ERR_get_error(), NULL));
		goto fail;
	}
	EC_KEY_set_asn1_flag(eckey, OPENSSL_EC_NAMED_CURVE);

	// Convert EC_KEY ecp_sum into EVP_PKEY format
	pkey = EVP_PKEY_new();
	if (!pkey) {
		printf("[CRYPTO] cannot create EVP_PKEY\n");
		goto fail;
	}
	if (EVP_PKEY_set1_EC_KEY(pkey, eckey) != 1) {
		printf("[CRYPTO] cannot set EC_KEY to PKEY\n");
		EVP_PKEY_free(pkey);
		goto fail;
	}

	// Convert EVP_PKEY ecp_sum to ANSI X9.63 format
	if (_convert_openssl_pkey_to_coordinates(pkey, pub_ansix963, &pub_ansix963_len) != 1) {
		printf("[CRYPTO] cannot convert pkey to coordinates\n");
		goto fail;
	}

	// Convert EVP_PKEY ecp_sum to EVP_PKEY SubjectPublicKeyInfo
	if (_convert_openssl_pkey_to_subject_public_key_info(pkey, subpkinfo, &subpkinfo_len) != 1) {
		printf("[CRYPTO] cannot convert pkey to SubjectPublicKeyInfo\n");
		goto fail;
	}

	// Copy everything to result keypair
	memcpy(res->subpkinfo, subpkinfo, subpkinfo_len);
	res->subpkinfo_len = subpkinfo_len;
	memcpy(res->pub_ansix963, pub_ansix963, pub_ansix963_len);
	res->pub_ansix963_len = pub_ansix963_len;
	strncpy(res->curve, curve_name, MAX_CURVE_NAME_LEN - 1);
	res->type = PUBLIC_KEY;

	ret = 1;

fail:
	EC_KEY_free(eckey);
	EVP_PKEY_free(pkey);
	EC_GROUP_free(group);
	BN_CTX_free(ctx);
	EC_POINT_free(ecp_sum);
	EC_POINT_free(ecp_a);
	EC_POINT_free(ecp_b);
	return ret;
}

INT8U PLATFORM_DECRYPT_E_ID(ECCKeyPair *ppkey, ECCKeyPair *a_nonce, ECCKeyPair *e_prime_id)
{
	BIGNUM   *pp   = NULL;
	EC_POINT *e_id = NULL, *a_nonce_point = NULL, *e_prime_id_point = NULL;
	EC_GROUP *ec_group = NULL;
	BN_CTX   *ctx      = NULL;

	INT32S    nid   = 0;
	INT8U     ret   = 0;

	if (!ppkey)
		return 0;

	nid = OBJ_txt2nid(ppkey->curve);

	/* E-id = E'-id - s_C * A-NONCE */
	ec_group = EC_GROUP_new_by_curve_name(nid);
	if (!ec_group)
		goto fail;

	ctx = BN_CTX_new();
	if (!ctx)
		goto fail;

	pp               = _convert_der_to_openssl_private_key(ppkey);
	a_nonce_point    = _convert_public_der_to_openssl_public_key(a_nonce);
	e_prime_id_point = _convert_public_der_to_openssl_public_key(e_prime_id);
	e_id             = EC_POINT_new(ec_group);

	if (!pp || !a_nonce_point || !e_prime_id_point || !e_id
	    || _ssl_ec_point_mul(ec_group, a_nonce_point, pp, e_id, ctx)
	    || _ssl_ec_point_invert(ec_group, e_id, ctx)
	    || _ssl_ec_point_add(ec_group, e_id, e_prime_id_point, e_id, ctx)) {
		goto fail;
	}

	ret = 1;

fail:
	BN_clear_free(pp);
	EC_POINT_clear_free(a_nonce_point);
	EC_POINT_clear_free(e_prime_id_point);
	EC_POINT_clear_free(e_id);
	EC_GROUP_free(ec_group);
	BN_CTX_free(ctx);
	return ret;
}

DPPReconfigID *PLATFORM_GENERATE_RECONFIG_ID(ECCKeyPair *csign, ECCKeyPair *ppkey)
{
	DPPReconfigID  *id        = NULL;
	EC_POINT       *e_id      = NULL;
	const EC_POINT *generator = NULL;
	BIGNUM         *bn        = NULL;
	const BIGNUM   *q         = NULL;
	INT16U          group     = 0;

	INT32S nid = 0;

	EC_GROUP *ec_group = NULL;
	BN_CTX   *ctx      = NULL;

	group = PLATFORM_EC_KEY_GET_GROUP(csign);
	if (group == GROUP_UNKNOWN)
		goto fail;

	nid      = _ec_group_to_nid(group);
	ec_group = EC_GROUP_new_by_curve_name(nid);
	if (!ec_group)
		goto fail;

	ctx = BN_CTX_new();
	if (!ctx)
		goto fail;

	e_id = EC_POINT_new(ec_group);
	if (!e_id)
		goto fail;

	bn        = BN_new();
	q         = EC_GROUP_get0_order(ec_group);
	generator = EC_GROUP_get0_generator(ec_group);

	if (!e_id || !bn || !q || !generator
	    || _ssl_big_num_rand(bn, q)
	    || _ssl_ec_point_mul(ec_group, generator, bn, e_id, ctx))
		goto fail;

	id = (DPPReconfigID *)malloc(sizeof(*id));
	if (id == NULL)
		goto fail;
	memset(id, 0, sizeof(*id));

	id->csign = (ECCKeyPair *)malloc(sizeof(ECCKeyPair));
	memcpy(id->csign, csign, sizeof(ECCKeyPair));

	id->ppkey = (ECCKeyPair *)malloc(sizeof(ECCKeyPair));
	memcpy(id->ppkey, ppkey, sizeof(ECCKeyPair));

	id->e_id = (ECCKeyPair *)malloc(sizeof(ECCKeyPair));
	// Convert e_id EC_POINT to keypair type
	PLATFORM_EC_KEY_SET_PUBLIC_POINT(group, e_id, id->e_id);

fail:
	BN_clear_free(bn);
	EC_POINT_free(e_id);
	EC_GROUP_free(ec_group);
	BN_CTX_free(ctx);
	return id;
}

INT8U PLATFORM_UPDATE_RECONFIG_ID(DPPReconfigID *id)
{
	const BIGNUM *  q          = NULL;
	const EC_POINT *generator  = NULL;
	BIGNUM *        bn         = NULL;
	EC_POINT *      e_prime_id = NULL, *a_nonce = NULL, *e_id = NULL, *pp = NULL;

	INT16U group = 0;
	INT8U  ret   = 0;
	INT32S nid   = 0;

	EC_GROUP *ec_group = NULL;
	BN_CTX *  ctx      = NULL;

	if (!id)
		goto fail;

	// Get group from csign
	group = PLATFORM_EC_KEY_GET_GROUP(id->csign);
	if (group == 0xFF)
		goto fail;

	nid = _ec_group_to_nid(group);
	ec_group = EC_GROUP_new_by_curve_name(nid);
	if (!ec_group)
		goto fail;

	ctx = BN_CTX_new();
	if (!ctx)
		goto fail;

	pp         = _convert_public_der_to_openssl_public_key(id->ppkey);
	e_prime_id = EC_POINT_new(ec_group);
	a_nonce    = EC_POINT_new(ec_group);
	bn         = BN_new();
	q          = EC_GROUP_get0_order(ec_group);
	generator  = EC_GROUP_get0_generator(ec_group);
	e_id       = _convert_public_der_to_openssl_public_key(id->e_id);

	/* Generate random 0 <= a-nonce < q
	 * A-NONCE = a-nonce * G
	 * E'-id = E-id + a-nonce * P_pk */
	if (!pp || !e_prime_id || !a_nonce || !bn || !q || !generator || !e_id
	    || _ssl_big_num_rand(bn, q) != 0 // bn = a-nonce
	    || _ssl_ec_point_mul(ec_group, generator, bn, a_nonce, ctx) != 0
	    || _ssl_ec_point_mul(ec_group, pp, bn, e_prime_id, ctx) != 0
	    || _ssl_ec_point_add(ec_group, e_id, e_prime_id, e_prime_id, ctx) != 0)
		goto fail;

	// Convert a_nonce EC_POINT to keypair type
	id->a_nonce = (ECCKeyPair *)malloc(sizeof(ECCKeyPair));
	PLATFORM_EC_KEY_SET_PUBLIC_POINT(group, a_nonce, id->a_nonce);

	// Convert e_prime_id EC_POINT to keypair type
	id->e_prime_id = (ECCKeyPair *)malloc(sizeof(ECCKeyPair));
	PLATFORM_EC_KEY_SET_PUBLIC_POINT(group, e_prime_id, id->e_prime_id);

	ret = 1;

fail:
	BN_clear_free(bn);
	EC_POINT_free(e_prime_id);
	EC_POINT_free(a_nonce);
	EC_POINT_free(e_id);
	EC_GROUP_free(ec_group);
	BN_CTX_free(ctx);
	return ret;
}

void PLATFORM_FREE_RECONFIG_ID(DPPReconfigID *id)
{
	if (id) {
		free(id->csign);
		free(id->a_nonce);
		free(id->e_id);
		free(id->e_prime_id);
		free(id->ppkey);
		free(id);
	}
}