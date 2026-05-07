#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "openssl/evp.h"
#include "openssl/ec.h"
#include "openssl/x509.h"
#include "openssl/asn1t.h"
#include "openssl/err.h"

#include "base64.h"

#define DPP_BOOTSTRAP_URI "/tmp/dpp_bootstrap_uri"
#define DPP_BOOTSTRAP_PRIV_KEY "/tmp/dpp_bootstrap_key"

#define MAX_CURVE_NAME_LEN 16
#define MAX_ASN1_DER_PRIVKEY_LEN 256
#define MAX_ASN1_DER_PUBKEY_LEN 256
#define MAX_ANSI_X963_LEN 192
#define MAX_SUBJECT_PUBKEY_INFO_LEN 128
#define MAX_SHARED_SECRET_LEN 66

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

uint8_t _convert_openssl_pkey_to_der(EVP_PKEY *key, uint8_t *priv, int *priv_len)
{
	EC_KEY *        eckey;
	const EC_GROUP *group;
	const EC_POINT *point;
	uint8_t *       der = NULL;
	int             der_len;
	uint8_t         ret = 0;

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

uint8_t _convert_openssl_pkey_to_subject_public_key_info(EVP_PKEY *key, uint8_t *subpkinfo, int *subpkinfo_len)
{
	uint8_t *                der = NULL;
	int                      der_len;
	EC_KEY *                 eckey;
	const EC_GROUP *         group;
	const EC_POINT *         point;
	BN_CTX *                 ctx;
	SUBJECT_PUBLIC_KEY_INFO *bootstrap = NULL;
	int                      nid, len;
	uint8_t                  ret = 0;

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

uint8_t dpp_gen_bootstrap_key(const char *curve_name, uint8_t *priv_der, uint16_t *priv_der_len, char **uri)
{
	EVP_PKEY_CTX *kctx      = NULL;
	EC_KEY *      ec_params = NULL;
	EC_KEY *      eckey     = NULL;
	EVP_PKEY *    params = NULL, *key = NULL;
	int           nid, ret            = 0;

	uint8_t pub_subpki[MAX_SUBJECT_PUBKEY_INFO_LEN];
	int     pub_subpki_len = 0;

	char * pubkey_b64     = NULL;
	size_t pubkey_b64_len = 0;

	size_t uri_len = 0;

	int result = 0;

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

	eckey  = EVP_PKEY_get0_EC_KEY(key);
	result = i2d_ECPrivateKey(eckey, &priv_der);
	if (result <= 0) {
		return 1;
	}
	*priv_der_len = (uint16_t)result;

	// Convert public key to SubjectPublicKeyInfo format
	ret = _convert_openssl_pkey_to_subject_public_key_info(key, pub_subpki, &pub_subpki_len);
	if (ret != 1) {
		printf("[CRYPTO] cannot convert pubkey to SubjectPublicKeyInfo\n");
		goto fail;
	}

	pubkey_b64 = (char *)base64_encode(pub_subpki, pub_subpki_len, &pubkey_b64_len);

	if (!pubkey_b64) {
		printf("DPP: base64 generation failed...\n");
		return 0;
	}

	uri_len += 4;                  // "DPP:"
	uri_len += 4;                  // "V:2;"
	uri_len += 4 + pubkey_b64_len; // K:...;;
	uri_len += 3;                  // ";;\0"
	*uri = malloc(uri_len);
	snprintf(*uri, uri_len, "DPP:V:2;K:%s;;", pubkey_b64);

	free(pubkey_b64);

fail:
	EC_KEY_free(ec_params);
	EVP_PKEY_free(params);
	EVP_PKEY_CTX_free(kctx);
	EVP_PKEY_free(key);
	return ret;
}

int main(void)
{
	uint8_t  priv_key_der[MAX_ASN1_DER_PRIVKEY_LEN];
	uint16_t priv_key_der_len;

	char *uri = NULL;

	FILE *fd = NULL;
	int   i;

	dpp_gen_bootstrap_key("prime256v1", priv_key_der, &priv_key_der_len, &uri);

	// write own bootstrap uri to file
	if ((fd = fopen(DPP_BOOTSTRAP_URI, "w+")) == NULL) {
		printf("DPP: uri file creation failed...\n");
		return 1;
	}
	fputs(uri, fd);
	fputs("\n", fd);
	fclose(fd);

	// write own bootstrap private key to file
	if ((fd = fopen(DPP_BOOTSTRAP_PRIV_KEY, "w+")) == NULL) {
		printf("DPP: priv key file creation failed...\n");
		return 1;
	}
	for (i = 0; i < priv_key_der_len; i++)
		fprintf(fd, "%02X", priv_key_der[i]);
	fputs("\n", fd);
	fclose(fd);

	return 0;
}