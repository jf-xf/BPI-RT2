/*
 *  Multi-AP Device Provisioning Protocol (DPP)
 *
 *  Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 */

#include <stdio.h>

#include "platform.h"
#include "al_dpp.h"

#include "packet_tools.h"
#include "platform_crypto_aes.h"

#include "base64.h"
#include "cJSON.h"

#include "utils.h"

////////////////////////////////////////////////////////////////////////////////
// Private functions and data
////////////////////////////////////////////////////////////////////////////////
// Data Declaration
static const struct dpp_curve_params dpp_curves[] = {
	/* The mandatory to support and the default NIST P-256 curve needs to
     * be the first entry on this list. */
	{ "prime256v1", 32, 32, 16, 32, "P-256", 19, "ES256" },
	{ "secp384r1", 48, 48, 24, 48, "P-384", 20, "ES384" },
	{ "secp521r1", 64, 64, 32, 66, "P-521", 21, "ES512" },
	{ "brainpoolP256r1", 32, 32, 16, 32, "BP-256", 28, "BS256" },
	{ "brainpoolP384r1", 48, 48, 24, 48, "BP-384", 29, "BS384" },
	{ "brainpoolP512r1", 64, 64, 32, 64, "BP-512", 30, "BS512" },
	{ NULL, 0, 0, 0, 0, NULL, 0, NULL }
};

// The netRole Compatibility List
// Ref: Table 22 of Easy Conenct 2.0 Spec , Table 8, Table 11 of Multi-AP R3 Spec
const enum dpp_netrole sta_compatible_list[]            = { DPP_NETROLE_AP, 0 };
const enum dpp_netrole ap_compatible_list[]             = { DPP_NETROLE_STA, 0 };
const enum dpp_netrole configurator_compatible_list[]   = { DPP_NETROLE_MAP_BACKHAUL_STA, DPP_NETROLE_MAP_AGENT, 0 };
const enum dpp_netrole mapBackhaulSta_compatible_list[] = { DPP_NETROLE_MAP_BACKHAUL_BSS, 0 };
const enum dpp_netrole mapBackhaulBss_compatible_list[] = { DPP_NETROLE_MAP_BACKHAUL_STA, 0 };
const enum dpp_netrole mapController_compatible_list[]  = { DPP_NETROLE_MAP_AGENT, 0 };
const enum dpp_netrole mapAgent_compatible_list[]       = { DPP_NETROLE_MAP_CONTROLLER, DPP_NETROLE_MAP_AGENT, 0 };

static const struct dpp_netrole_params dpp_netroles[] = {
	{ DPP_NETROLE_STA, "sta", sta_compatible_list },
	{ DPP_NETROLE_AP, "ap", sta_compatible_list },
	{ DPP_NETROLE_CONFIGURATOR, "configurator", configurator_compatible_list },
	{ DPP_NETROLE_MAP_BACKHAUL_STA, "mapBackhaulSta", mapBackhaulSta_compatible_list },
	{ DPP_NETROLE_MAP_BACKHAUL_BSS, "mapBackhaulBss", mapBackhaulBss_compatible_list },
	{ DPP_NETROLE_MAP_CONTROLLER, "mapController", mapController_compatible_list },
	{ DPP_NETROLE_MAP_AGENT, "mapAgent", mapAgent_compatible_list },
	{ DPP_NETROLE_UNKNOWN, NULL, NULL }
};

const struct dpp_curve_params *dpp_get_curve_ike_group(INT16U group)
{
	int i;

	for (i = 0; dpp_curves[i].name; i++) {
		if (dpp_curves[i].ike_group == group)
			return &dpp_curves[i];
	}
	return NULL;
}

const struct dpp_curve_params *dpp_get_curve_name(const char *name)
{
	int i = 0;
	if (!name)
		return &dpp_curves[0];

	for (i = 0; dpp_curves[i].name; i++) {
		if (PLATFORM_STRCMP(name, dpp_curves[i].name) == 0
		    || (dpp_curves[i].jwk_crv && PLATFORM_STRCMP(name, dpp_curves[i].jwk_crv) == 0))
			return &dpp_curves[i];
	}
	return NULL;
}

static const char *dpp_netrole_str(enum dpp_netrole netrole)
{
	int i;

	for (i = 0; dpp_netroles[i].netrole != DPP_NETROLE_UNKNOWN; i++) {
		if (netrole == dpp_netroles[i].netrole)
			return dpp_netroles[i].netrole_str;
	}

	return "???";
}

static enum dpp_netrole dpp_str_netrole(char *netrole_str)
{
	int i;

	for (i = 0; dpp_netroles[i].netrole != DPP_NETROLE_UNKNOWN; i++) {
		if (PLATFORM_STRCMP(dpp_netroles[i].netrole_str, netrole_str) == 0)
			return dpp_netroles[i].netrole;
	}

	return DPP_NETROLE_UNKNOWN;
}

static INT8U dpp_netrole_is_compatible(enum dpp_netrole own, enum dpp_netrole peer)
{
	int i, own_i = -1;

	for (i = 0; dpp_netroles[i].netrole != DPP_NETROLE_UNKNOWN; i++) {
		if (own == dpp_netroles[i].netrole) {
			own_i = i;
			break;
		}
	}

	if (own_i != -1) {
		const enum dpp_netrole *own_compatible_list = dpp_netroles[own_i].compatible_list;
		for (i = 0; own_compatible_list[i] != 0; i++) {
			if (peer == own_compatible_list[i])
				return 1;
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// DPP functions
////////////////////////////////////////////////////////////////////////////////

static INT8U dpp_bi_pubkey_hash(struct dpp_bootstrap_info *bi)
{
	unsigned char *addr[2];
	unsigned int   len[2];

	addr[0] = bi->keypair.subpkinfo;
	len[0]  = bi->keypair.subpkinfo_len;
	if (PLATFORM_SHA256(1, addr, len, bi->pubkey_hash) != 1)
		return 0;

	addr[0] = (unsigned char *)"chirp";
	len[0]  = 5;
	addr[1] = bi->keypair.subpkinfo;
	len[1]  = bi->keypair.subpkinfo_len;
	if (PLATFORM_SHA256(2, addr, len, bi->pubkey_hash_chirp) != 1)
		return 0;

	return 1;
}

static INT8U dpp_gen_uri(struct dpp_bootstrap_info *bi)
{
	size_t len = 0;

	if (!bi || PLATFORM_STRLEN((const char *)bi->pubkey_b64) == 0)
		return 0;

	len += 4;                                                 // "DPP:"
	len += 4;                                                 // "V:2;"
	len += 4 + PLATFORM_STRLEN((const char *)bi->pubkey_b64); // K:...;;
	len += 3;                                                 // ";;\0"
	PLATFORM_SAFE_FREE((void **)&(bi->uri));
	bi->uri = PLATFORM_MALLOC(len);
	PLATFORM_SNPRINTF(bi->uri, len, "DPP:V:2;K:%s;;", bi->pubkey_b64);
	return 1;
}

static INT8U dpp_write_bootstrap_info(struct dpp_bootstrap_info *bi)
{
	FILE *pFile = NULL;
	int   i;

	// write own bootstrap uri to file
	if ((pFile = fopen(DPP_BOOTSTRAP_OWN_URI_FILENAME, "w+")) == NULL) {
		PLATFORM_PRINTF_WARNING("DPP: uri file creation failed...\n");
		return 0;
	}
	fputs(bi->uri, pFile);
	fputs("\n", pFile);
	fclose(pFile);

	// write own bootstrap private key to file
	if ((pFile = fopen(DPP_BOOTSTRAP_OWN_PRIV_FILENAME, "w+")) == NULL) {
		PLATFORM_PRINTF_WARNING("DPP: priv key file creation failed...\n");
		return 0;
	}
	for (i = 0; i < bi->keypair.priv_len; i++)
		fprintf(pFile, "%02X", bi->keypair.priv[i]);
	fputs("\n", pFile);
	fclose(pFile);

	return 1;
}

static INT8U dpp_write_configurator_keys(struct dpp_configurator *conf)
{
	FILE *pFile = NULL;
	int   i;

	// write own csign key to file
	if ((pFile = fopen(DPP_BOOTSTRAP_CSIGN_FILENAME, "w+")) == NULL) {
		PLATFORM_PRINTF_WARNING("DPP: csign key file creation failed...\n");
		return 0;
	}
	for (i = 0; i < conf->csign.priv_len; i++)
		fprintf(pFile, "%02X", conf->csign.priv[i]);
	fputs("\n", pFile);
	fclose(pFile);

	// write own privacy protection key to file
	if ((pFile = fopen(DPP_BOOTSTRAP_PPK_FILENAME, "w+")) == NULL) {
		PLATFORM_PRINTF_WARNING("DPP: ppk file creation failed...\n");
		return 0;
	}
	for (i = 0; i < conf->ppkey.priv_len; i++)
		fprintf(pFile, "%02X", conf->ppkey.priv[i]);
	fputs("\n", pFile);
	fclose(pFile);

	return 1;
}

INT8U dpp_keygen(struct dpp_bootstrap_info *bi, const char *curve, unsigned char *priv, int priv_len)
{
	size_t pubkey_b64_len = 0;
	int    ret            = 0;

	// generate new key if priv is missing or priv_len is zero
	if (priv && priv_len) {
		PLATFORM_PRINTF_DEBUG("DPP: Parsing bootstrap key\n");
		ret = PLATFORM_PARSE_EC_PRIVATE_KEY(priv, priv_len, &(bi->keypair));
	} else {
		PLATFORM_PRINTF_DEBUG("DPP: Generating new bootstrap key\n");
		ret = PLATFORM_GENERATE_EC_KEYPAIR(dpp_get_curve_name(curve)->name, &(bi->keypair));
	}

	if (ret != 1) {
		PLATFORM_PRINTF_WARNING("DPP: key generation or parsing failed...\n");
		return 0;
	}

	// generate hash
	ret = dpp_bi_pubkey_hash(bi);
	if (ret != 1) {
		PLATFORM_PRINTF_WARNING("DPP: hash generation failed...\n");
		return 0;
	}

	// generate base64 of public key
	PLATFORM_SAFE_FREE((void **)&(bi->pubkey_b64));
	bi->pubkey_b64 = base64_encode(bi->keypair.subpkinfo, bi->keypair.subpkinfo_len, &pubkey_b64_len);
	if (!bi->pubkey_b64) {
		PLATFORM_PRINTF_WARNING("DPP: base64 generation failed...\n");
		return 0;
	}

	// generate DPP URI based on base64 public key
	if (dpp_gen_uri(bi) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: uri generation failed...\n");
		return 0;
	}

	// generate key files
	if (dpp_write_bootstrap_info(bi) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: file write failed...\n");
		return 0;
	}

	return 1;
}

INT8U dpp_keygen_configurator(struct dpp_configurator *conf, const char *curve, INT8U *csign, INT16U csign_len, INT8U *ppk, INT16U ppk_len)
{
	INT8U *addr[1];
	INT32U len[1];
	int    ret = 0;

	// generate new csign key if priv is missing or priv_len is zero
	if (csign && csign_len) {
		PLATFORM_PRINTF_DEBUG("DPP: Parsing c-sign key\n");
		ret = PLATFORM_PARSE_EC_PRIVATE_KEY(csign, csign_len, &(conf->csign));
	} else {
		PLATFORM_PRINTF_INFO("DPP: Generating new c-sign key\n");
		ret = PLATFORM_GENERATE_EC_KEYPAIR(dpp_get_curve_name(curve)->name, &(conf->csign));
	}

	if (ret != 1) {
		PLATFORM_PRINTF_WARNING("DPP: csign key generation or parsing failed...\n");
		return 0;
	}

	// set the elliptic curve info
	conf->curve = dpp_get_curve_name(conf->csign.curve);

	// generate kid = SHA256(ANSI X9.63 uncompressed C-sign-key)
	addr[0] = conf->csign.pub_ansix963;
	len[0]  = conf->csign.pub_ansix963_len;
	if (PLATFORM_SHA256(1, addr, len, conf->csign_kid_hash) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: csign kid generation failed!\n");
		return 0;
	}

	// generate base64(kid)
	PLATFORM_FREE(conf->csign_kid);
	conf->csign_kid = (char *)base64_url_encode(conf->csign_kid_hash, sizeof(conf->csign_kid_hash), NULL);
	if (!conf->csign_kid) {
		PLATFORM_PRINTF_WARNING("DPP: csign base64(kid) generation failed!\n");
		return 0;
	}

	if (ppk && ppk_len) {
		PLATFORM_PRINTF_DEBUG("DPP: Parsing private protection key\n");
		ret = PLATFORM_PARSE_EC_PRIVATE_KEY(ppk, ppk_len, &(conf->ppkey));
	} else {
		PLATFORM_PRINTF_INFO("DPP: Generating new private protection key\n");
		ret = PLATFORM_GENERATE_EC_KEYPAIR(dpp_get_curve_name(curve)->name, &(conf->ppkey));
	}

	if (ret != 1) {
		PLATFORM_PRINTF_WARNING("DPP: configurator pp key generation or parsing failed...\n");
		return 0;
	}

	// generate kid = SHA256(ANSI X9.63 uncompressed private protection key)
	addr[0] = conf->ppkey.pub_ansix963;
	len[0]  = conf->ppkey.pub_ansix963_len;
	if (PLATFORM_SHA256(1, addr, len, conf->ppkey_kid_hash) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: ppkey kid generation failed!\n");
		return 0;
	}

	// generate base64(kid)
	PLATFORM_FREE(conf->ppkey_kid);
	conf->ppkey_kid = (char *)base64_url_encode(conf->ppkey_kid_hash, sizeof(conf->ppkey_kid_hash), NULL);
	if (!conf->ppkey_kid) {
		PLATFORM_PRINTF_WARNING("DPP: ppkey base64(kid) generation failed!\n");
		return 0;
	}

	// write configurator keys
	if (dpp_write_configurator_keys(conf) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: file write failed...\n");
		return 0;
	}

	return 1;
}

void dpp_free_configurator(struct dpp_configurator *conf)
{
	PLATFORM_SAFE_FREE((void **)&conf->csign_kid);
}

// Decode info to MAP compatible keypair
static int dpp_parse_uri_pk(struct dpp_bootstrap_info *bi, const char *info)
{
	unsigned char *data;
	size_t         data_len;
	const char *   end;

	end = strchr(info, ';');
	if (!end)
		return 0;

	data = base64_decode((const unsigned char *)info, end - info, &data_len);
	if (!data) {
		PLATFORM_PRINTF_WARNING("DPP: Invalid base64 encoding on URI public-key\n");
		PLATFORM_FREE(data);
		return 0;
	}

	if (PLATFORM_PARSE_EC_SUBJECT_PUBLIC_KEY_INFO(data, data_len, &(bi->keypair)) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: the public key cannot be parsed\n");
		PLATFORM_FREE(data);
		return 0;
	}

	if (dpp_bi_pubkey_hash(bi) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: hash generation failed...\n");
		PLATFORM_FREE(data);
		return 0;
	}

	PLATFORM_FREE(data);
	return 1;
}

// Decode URI to MAP compatible dpp_bootstrap_info
INT8U dpp_parse_uri(const char *uri, struct dpp_bootstrap_info *bi)
{
	const char *pos = uri;
	const char *end;
	const char *chan_list = NULL, *mac = NULL, *info = NULL, *pk = NULL;
	const char *version = NULL;

	PLATFORM_PRINTF_INFO("DPP: Parsing DPP URI: %s\n", uri);

	if (strncmp(pos, "DPP:", 4) != 0) {
		PLATFORM_PRINTF_WARNING("DPP: Not a DPP URI\n");
		return 0;
	}
	pos += 4;

	for (;;) {
		end = strchr(pos, ';');
		if (!end)
			break;

		if (end == pos) {
			/* Handle terminating ";;" and ignore unexpected ";"
             * for parsing robustness. */
			pos++;
			continue;
		}

		if (pos[0] == 'C' && pos[1] == ':' && !chan_list)
			chan_list = pos + 2;
		else if (pos[0] == 'M' && pos[1] == ':' && !mac)
			mac = pos + 2;
		else if (pos[0] == 'I' && pos[1] == ':' && !info)
			info = pos + 2;
		else if (pos[0] == 'K' && pos[1] == ':' && !pk)
			pk = pos + 2;
		else if (pos[0] == 'V' && pos[1] == ':' && !version)
			version = pos + 2;
		else
			PLATFORM_PRINTF_WARNING("DPP: Ignore unrecognized URI parameter: %C\n", pos[0]);
		pos = end + 1;
	}

	if (!pk) {
		PLATFORM_PRINTF_WARNING("DPP: URI missing public-key\n");
		return 0;
	}

	if (dpp_parse_uri_pk(bi, pk) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: uri pk parsing failed\n");
		return 0;
	}

	return 1;
}

static int dpp_hmac_vector(size_t hash_len, INT8U *key, size_t key_len,
                           size_t num_elem, INT8U *addr[], INT32U *len, INT8U *mac)
{
	if (hash_len == 32)
		return PLATFORM_HMAC_SHA256(key, key_len, num_elem, addr, len, mac);
	if (hash_len == 48)
		return PLATFORM_HMAC_SHA384(key, key_len, num_elem, addr, len, mac);
	if (hash_len == 64)
		return PLATFORM_HMAC_SHA512(key, key_len, num_elem, addr, len, mac);
	return 0;
}

int dpp_hmac(size_t hash_len, INT8U *key, size_t key_len,
                    INT8U *data, INT32U data_len, INT8U *mac)
{
	INT8U *addr[1];
	INT32U len[1];

	addr[0] = data;
	len[0]  = data_len;

	return dpp_hmac_vector(hash_len, key, key_len, 1, addr, len, mac);
}

int dpp_hkdf_expand(size_t hash_len, INT8U *secret, size_t secret_len,
                           const char *label, INT8U *out, size_t outlen)
{
	if (hash_len == 32)
		return PLATFORM_HMAC_SHA256_HKDF_EXPAND(secret, secret_len, NULL,
		                                        (INT8U *)label, strlen(label),
		                                        out, outlen);
	if (hash_len == 48)
		return PLATFORM_HMAC_SHA384_HKDF_EXPAND(secret, secret_len, NULL,
		                                        (INT8U *)label, strlen(label),
		                                        out, outlen);
	if (hash_len == 64)
		return PLATFORM_HMAC_SHA512_HKDF_EXPAND(secret, secret_len, NULL,
		                                        (INT8U *)label, strlen(label),
		                                        out, outlen);
	return 0;
}

static int dpp_hash_vector(const struct dpp_curve_params *curve,
                           INT8U num_elem, INT8U **addr, INT32U *len, INT8U *digest)
{
	if (curve->hash_len == 32)
		return PLATFORM_SHA256(num_elem, addr, len, digest);
	if (curve->hash_len == 48)
		return PLATFORM_SHA384(num_elem, addr, len, digest);
	if (curve->hash_len == 64)
		return PLATFORM_SHA512(num_elem, addr, len, digest);
	return 0;
}

static INT8U dpp_derive_k1(unsigned char *Mx, INT32U Mx_len, unsigned char *k1, unsigned int hash_len)
{
	INT8U salt[DPP_MAX_HASH_LEN];
	INT8U prk[DPP_MAX_HASH_LEN];
	char *info = "first intermediate key";
	int   res;

	/* k1 = HKDF(<>, "first intermediate key", M.x) */

	/* HKDF-Extract(<>, M.x) */
	memset(salt, 0, DPP_MAX_HASH_LEN);
	if (dpp_hmac(hash_len, salt, hash_len, Mx, Mx_len, prk) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: HKDF-Extract Failed..\n");
		return 0;
	}

	/* HKDF-Expand(PRK, info, L) */
	res = dpp_hkdf_expand(hash_len, prk, hash_len, info, k1, hash_len);
	memset(prk, 0, DPP_MAX_HASH_LEN);
	if (res != 1) {
		PLATFORM_PRINTF_WARNING("DPP: HKDF-Expand Failed..\n");
		return 0;
	}

	return 1;
}

static INT8U dpp_derive_k2(unsigned char *Nx, INT32U Nx_len, unsigned char *k2, unsigned int hash_len)
{
	INT8U salt[DPP_MAX_HASH_LEN];
	INT8U prk[DPP_MAX_HASH_LEN];
	char *info = "second intermediate key";
	int   res;

	/* k2 = HKDF(<>, "second intermediate key", N.x) */

	/* HKDF-Extract(<>, N.x) */
	memset(salt, 0, DPP_MAX_HASH_LEN);
	if (dpp_hmac(hash_len, salt, hash_len, Nx, Nx_len, prk) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: HKDF-Extract Failed..\n");
		return 0;
	}

	/* HKDF-Expand(PRK, info, L) */
	res = dpp_hkdf_expand(hash_len, prk, hash_len, info, k2, hash_len);
	memset(prk, 0, DPP_MAX_HASH_LEN);
	if (res != 1) {
		PLATFORM_PRINTF_WARNING("DPP: HKDF-Expand Failed..\n");
		return 0;
	}

	return 1;
}

static int dpp_derive_bk_ke(struct dpp_authentication *auth)
{
	int hash_len  = auth->curve->hash_len;
	int nonce_len = auth->curve->nonce_len;

	// For bk
	INT8U  nonces[2 * DPP_MAX_NONCE_LEN];
	INT8U *addr[2];
	INT32U len[2];

	// For ke
	const char *info_ke = "DPP Key";
	int         res;

	/* bk = HKDF-Extract(I-nonce | R-nonce, M.x | N.x [| L.x]) */
	PLATFORM_MEMCPY(nonces, auth->i_nonce, nonce_len);
	PLATFORM_MEMCPY(nonces + nonce_len, auth->r_nonce, nonce_len);
	addr[0] = auth->Mx;
	len[0]  = auth->Mx_len;
	addr[1] = auth->Nx;
	len[1]  = auth->Nx_len;

	if (dpp_hmac_vector(hash_len, nonces, 2 * nonce_len, 2, addr, len, auth->bk) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot generate bk\n");
		return 0;
	}

	/* ke = HKDF-Expand(bk, "DPP Key", length) */
	res = dpp_hkdf_expand(hash_len, auth->bk, hash_len, info_ke, auth->ke, hash_len);
	if (res != 1) {
		PLATFORM_PRINTF_WARNING("DPP: HKDF-Expand Failed..\n");
		return 0;
	}

	return 1;
}

static int dpp_derive_pmk(INT8U *Nx, INT32U Nx_len, INT8U *pmk, unsigned int hash_len)
{
	INT8U salt[DPP_MAX_HASH_LEN], prk[DPP_MAX_HASH_LEN];

	// for PMK
	const char *info = "DPP PMK";
	int         res;

	/* PMK = HKDF(<>, "DPP PMK", N.x) */

	/* PRK = HKDF-Extract(<>, N.x) */
	PLATFORM_MEMSET(salt, 0, hash_len);
	if (dpp_hmac(hash_len, salt, hash_len, Nx, Nx_len, prk) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot generate prk\n");
		return 0;
	}

	/* PMK = HKDF-Expand(PRK, info, L) */
	res = dpp_hkdf_expand(hash_len, prk, hash_len, info, pmk, hash_len);
	PLATFORM_MEMSET(prk, 0, hash_len);
	if (res != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot generate pmk\n");
		return 0;
	}

	return 1;
}

static int dpp_derive_pmkid(ECCKeyPair *own_key, ECCKeyPair *peer_key, INT8U *pmkid)
{
	INT8U *nkx, *pkx;
	INT32U nkx_len, pkx_len;

	INT8U  hash[SHA256_MAC_LEN];
	INT8U *addr[2];
	INT32U len[2];

	nkx     = own_key->pub_ansix963 + 1;            // skip the first 0x04
	nkx_len = (own_key->pub_ansix963_len - 1) / 2;  // only the x coordinate
	pkx     = peer_key->pub_ansix963 + 1;           // skip the first 0x04
	pkx_len = (peer_key->pub_ansix963_len - 1) / 2; // only the x coordinate
	if (nkx_len != pkx_len) {
		PLATFORM_PRINTF_WARNING("DPP: length mismatch\n");
		return 0;
	}

	/* PMKID = Truncate-128(H(min(NK.x, PK.x) | max(NK.x, PK.x))) */
	addr[0] = nkx;
	len[0]  = nkx_len;
	addr[1] = pkx;
	len[1]  = pkx_len;
	if (PLATFORM_MEMCMP(nkx, pkx, nkx_len) > 0) {
		addr[0] = pkx;
		addr[1] = nkx;
	}

	if (PLATFORM_SHA256(2, addr, len, hash) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: hash error in generating pmkid\n");
		return 0;
	}

	PLATFORM_MEMCPY(pmkid, hash, PMKID_LEN);
	return 1;
}

static INT8U dpp_gen_r_auth(struct dpp_authentication *auth, INT8U *r_auth)
{
	size_t      nonce_len;
	ECCKeyPair *PI, *PR, *BR;
	INT8U       zero = 0, res = 0;
	INT8U *     addr[6];
	INT32U      len[6];

	/* R-auth = H(I-nonce | R-nonce | PI.x | PR.x | [BI.x |] BR.x | 0) */
	nonce_len = auth->curve->nonce_len;

	if (auth->initiator) {
		// own is initiator
		PI = &(auth->own_protocol_key);
		PR = &(auth->peer_protocol_key);
		BR = &(auth->peer_bi.keypair);
	} else {
		// own is responder
		PI = &(auth->peer_protocol_key);
		PR = &(auth->own_protocol_key);
		BR = &(auth->own_bi->keypair);
	}

	addr[0] = auth->i_nonce;
	len[0]  = nonce_len;

	addr[1] = auth->r_nonce;
	len[1]  = nonce_len;

	addr[2] = PI->pub_ansix963 + 1;           // skip the first 0x04
	len[2]  = (PI->pub_ansix963_len - 1) / 2; // only the x coordinate

	addr[3] = PR->pub_ansix963 + 1;           // skip the first 0x04
	len[3]  = (PR->pub_ansix963_len - 1) / 2; // only the x coordinate

	addr[4] = BR->pub_ansix963 + 1;           // skip the first 0x04
	len[4]  = (BR->pub_ansix963_len - 1) / 2; // only the x coordinate

	addr[5] = &zero;
	len[5]  = 1;

	res = dpp_hash_vector(auth->curve, 6, addr, len, r_auth);
	if (res != 1) {
		PLATFORM_PRINTF_WARNING("DPP: R-Auth Generation Failed..\n");
		return 0;
	}

	return 1;
}

static INT8U dpp_gen_i_auth(struct dpp_authentication *auth, INT8U *i_auth)
{
	size_t      nonce_len;
	ECCKeyPair *PI, *PR, *BR;
	INT8U       one = 1, res = 0;
	INT8U *     addr[6];
	INT32U      len[6];

	/* I-auth = H(R-nonce | I-nonce | PR.x | PI.x | BR.x | [BI.x |] 1) */
	nonce_len = auth->curve->nonce_len;

	if (auth->initiator) {
		// own is initiator
		PI = &(auth->own_protocol_key);
		PR = &(auth->peer_protocol_key);
		BR = &(auth->peer_bi.keypair);
	} else {
		// own is responder
		PI = &(auth->peer_protocol_key);
		PR = &(auth->own_protocol_key);
		BR = &(auth->own_bi->keypair);
	}

	addr[0] = auth->r_nonce;
	len[0]  = nonce_len;

	addr[1] = auth->i_nonce;
	len[1]  = nonce_len;

	addr[2] = PR->pub_ansix963 + 1;           // skip the first 0x04
	len[2]  = (PR->pub_ansix963_len - 1) / 2; // only the x coordinate

	addr[3] = PI->pub_ansix963 + 1;           // skip the first 0x04
	len[3]  = (PI->pub_ansix963_len - 1) / 2; // only the x coordinate

	addr[4] = BR->pub_ansix963 + 1;           // skip the first 0x04
	len[4]  = (BR->pub_ansix963_len - 1) / 2; // only the x coordinate

	addr[5] = &one;
	len[5]  = 1;

	res = dpp_hash_vector(auth->curve, 6, addr, len, i_auth);
	if (res != 1) {
		PLATFORM_PRINTF_WARNING("DPP: R-Auth Generation Failed..\n");
		return 0;
	}

	return 1;
}

static void dpp_auth_success(struct dpp_authentication *auth)
{
	PLATFORM_PRINTF_INFO("DPP: Authentication success\n");
	PLATFORM_PRINTF_DEBUG("DPP: Clearing temporary keys..\n");
	PLATFORM_MEMSET(auth->Mx, 0, sizeof(auth->Mx));
	auth->Mx_len = 0;
	PLATFORM_MEMSET(auth->Nx, 0, sizeof(auth->Nx));
	auth->Nx_len = 0;
	PLATFORM_MEMSET(auth->k1, 0, sizeof(auth->k1));
	PLATFORM_MEMSET(auth->k2, 0, sizeof(auth->k2));
	auth->auth_success = 1;
	auth->in_progress  = 0;
}

INT8U *dpp_alloc_msg(enum dpp_public_action_frame_type type, INT32U len, INT32U *out_len)
{
	INT8U  aux8;
	INT8U *msg;
	INT8U *p;

	*out_len = 8 + len;
	msg      = PLATFORM_MALLOC(*out_len);
	PLATFORM_MEMSET(msg, 0, *out_len);
	p = msg;

	/* Category: Public Action Frame */
	aux8 = WLAN_ACTION_PUBLIC;
	_I1B(&aux8, &p);

	/* Action Field: Vendor Specific */
	aux8 = WLAN_PA_VENDOR_SPECIFIC;
	_I1B(&aux8, &p);

	/* OUI: Wi-Fi Alliance */
	aux8 = (OUI_WFA >> 16) & 0xFF;
	_I1B(&aux8, &p);
	aux8 = (OUI_WFA >> 8) & 0xFF;
	_I1B(&aux8, &p);
	aux8 = (OUI_WFA)&0xFF;
	_I1B(&aux8, &p);

	/* OUI Type: DPP */
	aux8 = DPP_OUI_TYPE;
	_I1B(&aux8, &p);

	/* Crypto Suite */
	aux8 = 0x01;
	_I1B(&aux8, &p);

	/* DPP Frame Type */
	aux8 = (INT8U)type;
	_I1B(&aux8, &p);

	return msg;
}

// Reset states in DPP Authentication struct
void dpp_auth_clear_state(struct dpp_authentication *auth)
{
	auth->initiator    = 0;
	auth->in_progress  = 0;
	auth->auth_success = 0;
}

// DPP Authentication Request Builder
INT8U dpp_auth_init(struct dpp_authentication *auth, INT8U **auth_req_msg, INT32U *auth_req_msg_len)
{
	ECCKeyPair * pI = &(auth->own_protocol_key);
	ECCKeyPair * BR = &(auth->peer_bi.keypair);
	int          nonce_len;
	int          hash_len;
	INT8U        wrapped_data[4 + DPP_MAX_NONCE_LEN + 4 + 1 + AES_BLOCK_SIZE];
	const INT8U *addr[2];
	INT32U       len[2];
	INT32U       siv_len, attr_len, attr_out_len, wrapped_data_len;

	PLATFORM_PRINTF_INFO("DPP: Build Authentication Request\n");

	/* Clear all states */
	dpp_auth_clear_state(auth);

	/* Set elliptic curve info */
	if ((auth->curve = dpp_get_curve_name(BR->curve)) == NULL) {
		PLATFORM_PRINTF_WARNING("DPP: BR is not ready\n");
		return 0;
	}
	nonce_len = auth->curve->nonce_len;
	hash_len  = auth->curve->hash_len;

	/* I-nonce */
	if (PLATFORM_GET_RANDOM_BYTES(auth->i_nonce, nonce_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot generate I-nonce\n");
		return 0;
	}

	/* I-capabilities */
	auth->i_capab = DPP_CAPAB_CONFIGURATOR; // In MAP, Initiator is always Configurator

	/* Initiator's own protocol key */
	if (PLATFORM_GENERATE_EC_KEYPAIR(BR->curve, pI) != 1) {
		printf("DPP: protocol key generation failed..\n");
		return 0;
	}

	/* ECDH: M = pI * BR */
	if (PLATFORM_COMPUTE_ECDH_SHARED_SECRET(pI, BR, auth->Mx, &(auth->Mx_len)) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot calculate ecdh\n");
		return 0;
	}

	if (dpp_derive_k1(auth->Mx, auth->Mx_len, auth->k1, hash_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: k1 derivation error!\n");
		return 0;
	}

	/* Build DPP Authentication Request frame attributes, P126 of Easy Connect Spec v2. */
	attr_len = 4 + SHA256_MAC_LEN;            // SHA256(BR)
	attr_len += 4 + SHA256_MAC_LEN;           // SHA256(BI)
	attr_len += 4 + pI->pub_ansix963_len - 1; // X || Y (omit 0x04)
	attr_len += 4 + 1;                        // DPP Version
	wrapped_data_len = 4 + nonce_len;         // Wrapped data: I-Nonce
	wrapped_data_len += 4 + 1;                // Wrapped data: I-Capabilities
	wrapped_data_len += AES_BLOCK_SIZE;       // Wrapped data: AES Block
	attr_len += 4 + wrapped_data_len;             // {I-nonce | I-cap}k1

	/* Allocate message buffer */
	INT8U *msg = dpp_alloc_msg(DPP_PA_AUTHENTICATION_REQ, attr_len, &attr_out_len);
	if (!msg) {
		PLATFORM_PRINTF_WARNING("DPP: alloc_msg failed!\n");
		return 0;
	}
	INT8U *p = msg + (attr_out_len - attr_len);
	INT16U aux16;
	INT8U  aux8;

	/* Responder Bootstrapping Key Hash */
	aux16 = (INT16U)DPP_ATTR_R_BOOTSTRAP_KEY_HASH;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)SHA256_MAC_LEN;
	_I2B_LE(&aux16, &p);
	_InB(&(auth->peer_bi.pubkey_hash), &p, SHA256_MAC_LEN);

	/* Initiator Bootstrapping Key Hash */
	aux16 = (INT16U)DPP_ATTR_I_BOOTSTRAP_KEY_HASH;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)SHA256_MAC_LEN;
	_I2B_LE(&aux16, &p);
	_InB(&(auth->own_bi->pubkey_hash), &p, SHA256_MAC_LEN);

	/* Initiator Protocol Key */
	aux16 = (INT16U)DPP_ATTR_I_PROTOCOL_KEY;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)pI->pub_ansix963_len - 1;
	_I2B_LE(&aux16, &p);
	_InB(pI->pub_ansix963 + 1, &p, pI->pub_ansix963_len - 1); // Remove 0x04 prefix

	/* Protocol Version */
	aux16 = (INT16U)DPP_ATTR_PROTOCOL_VERSION;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)1;
	_I2B_LE(&aux16, &p);
	aux8 = DPP_VERSION;
	_I1B(&aux8, &p);

	/* Wrapped data ({I-nonce, I-capabilities}k1) */
	INT8U  clear[4 + DPP_MAX_NONCE_LEN + 4 + 1] = { 0 };
	INT8U *p2                                   = clear;

	/* Wrapped data: I-nonce */
	aux16 = (INT16U)DPP_ATTR_I_NONCE;
	_I2B_LE(&aux16, &p2);
	aux16 = (INT16U)nonce_len;
	_I2B_LE(&aux16, &p2);
	_InB(auth->i_nonce, &p2, nonce_len);

	/* Wrapped data: I-capabilities */
	aux16 = (INT16U)DPP_ATTR_I_CAPABILITIES;
	_I2B_LE(&aux16, &p2);
	aux16 = (INT16U)1;
	_I2B_LE(&aux16, &p2);
	aux8 = auth->i_capab;
	_I1B(&aux8, &p2);

	/* Associated data: OUI, OUI type, Crypto Suite, DPP frame type */
	addr[0] = msg + 2;
	len[0]  = 3 + 1 + 1 + 1;

	/* Attributes before Wrapped Data */
	addr[1] = msg + 2 + 3 + 1 + 1 + 1;
	len[1]  = p - (msg + 2 + 3 + 1 + 1 + 1);

	siv_len = p2 - clear;
	if (PLATFORM_AES_SIV_ENCRYPT(auth->k1, hash_len, clear, siv_len, 2, addr, len, wrapped_data) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot encrypt with k1\n");
		PLATFORM_FREE(msg);
		return 0;
	}

	/* Wrapped data */
	siv_len += AES_BLOCK_SIZE;
	aux16 = (INT16U)DPP_ATTR_WRAPPED_DATA;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)siv_len;
	_I2B_LE(&aux16, &p);
	_InB(wrapped_data, &p, siv_len);

	/* Update states */
	auth->in_progress = 1;
	auth->initiator   = 1;

	*auth_req_msg     = msg;
	*auth_req_msg_len = attr_out_len;
	PLATFORM_PRINTF_INFO("DPP: Authentication Request Generated\n");
	return 1;
}

// Search Attribute ID within a DPP Action Frame buffer
INT8U *dpp_get_attr(INT8U *buf, INT32U len, INT16U req_id, INT16U *ret_len)
{
	INT16U id = 0, alen = 0;
	INT8U *pos = buf, *end = buf + len;

	while (end - pos >= 4) {
		_E2B_LE(&pos, &id);
		_E2B_LE(&pos, &alen);
		if (alen > end - pos)
			return NULL;
		if (id == req_id) {
			*ret_len = alen;
			return pos;
		}
		pos += alen;
	}

	return NULL;
}

static INT8U *dpp_get_attr_next(INT8U *prev, INT8U *buf, INT32U len, INT16U req_id, INT16U *ret_len)
{
	INT16U prevlen = 0;
	INT8U *pos, *prevptr, *end = buf + len;

	if (!prev) {
		pos = buf;
	} else {
		prevptr = prev - 2;
		_E2B_LE(&prevptr, &prevlen);
		pos = prev + prevlen;
	}

	return dpp_get_attr(pos, end - pos, req_id, ret_len);
}

// Sanity Check for a DPP Action Frame buffer
int dpp_check_attrs(INT8U *buf, INT32U len)
{
	INT8U *pos, *end;
	INT8U  wrapped_data = 0;

	pos = buf;
	end = buf + len;
	while (end - pos >= 4) {
		INT16U id = 0, alen = 0;

		_E2B_LE(&pos, &id);
		_E2B_LE(&pos, &alen);
		PLATFORM_PRINTF_DEBUG("DPP: Attribute ID %04x len %u\n", id, alen);

		if (alen > end - pos) {
			PLATFORM_PRINTF_WARNING("DPP: Truncated message - not enough room for the attribute - dropped\n");
			return 0;
		}

		if (wrapped_data) {
			PLATFORM_PRINTF_WARNING("DPP: An unexpected attribute included after the Wrapped Data attribute\n");
			return 0;
		}

		if (id == DPP_ATTR_WRAPPED_DATA)
			wrapped_data = 1;

		pos += alen;
	}

	if (end != pos) {
		PLATFORM_PRINTF_WARNING("DPP: Unexpected octets (%d) after the last attribute\n", (int)(end - pos));
		return 0;
	}

	return 1;
}

// DPP Authentication Request Parser
INT8U dpp_rx_auth_req(struct dpp_authentication *auth, INT8U *buf, INT32U len)
{
	ECCKeyPair *PI = &(auth->peer_protocol_key);
	ECCKeyPair *bR = &(auth->own_bi->keypair);
	int         hash_len;
	int         nonce_len;
	INT16U      r_bootstrap_len = 0, i_bootstrap_len = 0, i_proto_len = 0,
	       version_len = 0, wrapped_data_len = 0, i_nonce_len = 0, i_capab_len = 0;
	const INT8U *r_bootstrap, *i_bootstrap, *i_proto, *version, *wrapped_data, *i_nonce, *i_capab;

	INT8U *attr_start = buf + 8;
	INT32U attr_len   = len - 8;

	int    unwrapped_len = 0;
	INT8U *unwrapped     = 0;

	/* Get elliptic curve info */
	if ((auth->curve = dpp_get_curve_name(bR->curve)) == NULL) {
		PLATFORM_PRINTF_WARNING("DPP: bR is not ready\n");
		return 0;
	}
	nonce_len = auth->curve->nonce_len;
	hash_len  = auth->curve->hash_len;

	// Get SHA256(BR)
	r_bootstrap = dpp_get_attr(attr_start, attr_len, DPP_ATTR_R_BOOTSTRAP_KEY_HASH, &r_bootstrap_len);
	if (!r_bootstrap || r_bootstrap_len != SHA256_MAC_LEN) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required SHA256(BR) attribute %d\n", r_bootstrap_len);
		return 0;
	}

	// Get SHA256(BI)
	i_bootstrap = dpp_get_attr(attr_start, attr_len, DPP_ATTR_I_BOOTSTRAP_KEY_HASH, &i_bootstrap_len);
	if (!i_bootstrap || i_bootstrap_len != SHA256_MAC_LEN) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required SHA256(BI) attribute\n");
		return 0;
	}

	// Compare received SHA256(BR) and local SHA256(BR)
	if (PLATFORM_MEMCMP(auth->own_bi->pubkey_hash, r_bootstrap, r_bootstrap_len) != 0) {
		PLATFORM_PRINTF_WARNING("DPP: BR does not match hashed own bootstrapping key\n");
		return 0;
	}

	// TODO: check if Already in DPP authentication exchange

	// Check Version
	version = dpp_get_attr(attr_start, attr_len, DPP_ATTR_PROTOCOL_VERSION, &version_len);
	if (!version || version_len < 1 || version[0] != DPP_VERSION) {
		PLATFORM_PRINTF_WARNING("DPP: DPP version is not %d\n", DPP_VERSION);
		return 0;
	}

	// Get PI (Initiator's Public Protocol Key)
	i_proto = dpp_get_attr(attr_start, attr_len, DPP_ATTR_I_PROTOCOL_KEY, &i_proto_len);
	if (!i_proto) {
		PLATFORM_PRINTF_WARNING("DPP: Missing required Initiator Protocol Key attribute\n");
		return 0;
	}

	// Convert Protocol Key to ECCKeyPair
	INT8U i_proto_ansix[MAX_ANSI_X963_LEN];
	i_proto_ansix[0] = 0x04; // add back 0x04
	PLATFORM_MEMCPY(i_proto_ansix + 1, i_proto, i_proto_len);
	if (PLATFORM_PARSE_EC_PUBLIC_ANSIX963(auth->curve->name, i_proto_ansix, i_proto_len + 1, PI) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: the coordinates are incorrect\n");
		return 0;
	}

	/* ECDH: M = bR * PI */
	if (PLATFORM_COMPUTE_ECDH_SHARED_SECRET(bR, PI, auth->Mx, &(auth->Mx_len)) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot calculate ecdh\n");
		return 0;
	}

	if (dpp_derive_k1(auth->Mx, auth->Mx_len, auth->k1, hash_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: k1 derivation error!\n");
		return 0;
	}

	wrapped_data = dpp_get_attr(attr_start, attr_len, DPP_ATTR_WRAPPED_DATA, &wrapped_data_len);
	if (!wrapped_data) {
		PLATFORM_PRINTF_WARNING("DPP: cannot get wrapped data!\n");
		return 0;
	}

	/* Associated data protected by k1 */
	const INT8U *addr[2];
	INT32U       alen[2];

	/* Associated data: OUI, OUI type, Crypto Suite, DPP frame type */
	addr[0] = buf + 2;
	alen[0] = 3 + 1 + 1 + 1;

	/* Attributes before Wrapped Data */
	addr[1]       = buf + 2 + 3 + 1 + 1 + 1;
	alen[1]       = (wrapped_data - 4) - (buf + 2 + 3 + 1 + 1 + 1);
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped     = PLATFORM_MALLOC(unwrapped_len);

	/* Decrypt wrapped data */
	if (PLATFORM_AES_SIV_DECRYPT(auth->k1, hash_len, wrapped_data, wrapped_data_len, 2, addr, alen, unwrapped) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: AES-SIV decryption failed\n");
		PLATFORM_FREE(unwrapped);
		return 0;
	}

	/* I-nonce */
	i_nonce = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_I_NONCE, &i_nonce_len);
	if (!i_nonce || i_nonce_len != nonce_len) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid I-nonce\n");
		PLATFORM_FREE(unwrapped);
		return 0;
	}
	PLATFORM_MEMCPY(auth->i_nonce, i_nonce, i_nonce_len);

	/* I-capabilities */
	i_capab = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_I_CAPABILITIES, &i_capab_len);
	if (!i_capab || i_capab_len < 1) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid I-capabilities\n");
		PLATFORM_FREE(unwrapped);
		return 0;
	}

	/* Handle I-capabilities */
	auth->i_capab = i_capab[0];
	switch (auth->i_capab & DPP_CAPAB_ROLE_MASK) {
	case DPP_CAPAB_CONFIGURATOR:
		PLATFORM_PRINTF_DEBUG("DPP: Initiator is Configurator, Good for MAP!\n");
		break;
	case DPP_CAPAB_ENROLLEE:
		PLATFORM_PRINTF_WARNING("DPP: Initiator is Enrollee! Not normal...\n");
		break;
	case DPP_CAPAB_CONFIGURATOR | DPP_CAPAB_ENROLLEE:
		PLATFORM_PRINTF_WARNING("DPP: Initiator is both Configurator and Enrollee!\n");
		break;
	default:
		PLATFORM_PRINTF_WARNING("DPP: I-capabilities: what is %02X??\n", auth->i_capab);
		break;
	}

	/* Update states */
	auth->in_progress = 1;

	PLATFORM_FREE(unwrapped);
	PLATFORM_PRINTF_INFO("DPP: Authentication Request Rx finished...\n");
	return 1;
}

static INT8U dpp_auth_build_resp(struct dpp_authentication *auth,
                                 enum dpp_status_error      status,
                                 INT8U *PR, INT32U PR_len,
                                 size_t nonce_len,
                                 INT8U *r_pubkey_hash,
                                 INT8U *r_nonce, INT8U *i_nonce,
                                 INT8U *wrapped_r_auth, INT32U wrapped_r_auth_len,
                                 INT8U * siv_key,
                                 INT8U **auth_resp_msg, INT32U *auth_resp_len)
{
	const int DPP_AUTH_RESP_K2_CLEAR_LEN = 2 * (4 + DPP_MAX_NONCE_LEN) +                // R-nonce, I-nonce
	                                       4 + 1 +                                      // R-capabilities
	                                       4 + (4 + DPP_MAX_HASH_LEN + AES_BLOCK_SIZE); // {R-auth}ke
	INT8U clear[DPP_AUTH_RESP_K2_CLEAR_LEN];
	INT8U wrapped_data[DPP_AUTH_RESP_K2_CLEAR_LEN + AES_BLOCK_SIZE];

	const INT8U *addr[2];
	INT32U       len[2];
	INT32U       siv_len, attr_len, attr_out_len, wrapped_data_len;

	// TODO: set state to waiting authentication confirm

	wrapped_data_len = 2 * (4 + nonce_len);                               // R-nonce, I-nonce
	wrapped_data_len += 4 + 1;                                            // R-capabilities
	wrapped_data_len += 4 + wrapped_r_auth_len + AES_BLOCK_SIZE;          // {R-auth}ke

	/* Build DPP Authentication Response frame attributes */
	attr_len = 4 + 1;                     // DPP Status
	attr_len += 4 + SHA256_MAC_LEN;       // SHA256(BR)
	attr_len += 4 + (PR ? PR_len : 0);    // PR
	attr_len += 4 + 1;                    // Protocol Version
	attr_len += 4 + wrapped_data_len;     // {...}k2

	/* Allocate message buffer */
	INT8U *msg = dpp_alloc_msg(DPP_PA_AUTHENTICATION_RESP, attr_len, &attr_out_len);
	if (!msg) {
		PLATFORM_PRINTF_WARNING("DPP: alloc_msg failed!\n");
		return 0;
	}
	INT8U *p = msg + (attr_out_len - attr_len);
	INT16U aux16;
	INT8U  aux8;

	/* DPP Status */
	aux16 = (INT16U)DPP_ATTR_STATUS;
	_I2B_LE(&aux16, &p);
	aux16 = 1;
	_I2B_LE(&aux16, &p);
	aux8 = (INT8U)status;
	_I1B(&aux8, &p);

	/* Responder Bootstrapping Key Hash SHA256(BR) */
	aux16 = (INT16U)DPP_ATTR_R_BOOTSTRAP_KEY_HASH;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)SHA256_MAC_LEN;
	_I2B_LE(&aux16, &p);
	_InB(r_pubkey_hash, &p, SHA256_MAC_LEN);

	/* Responder Protocol Key */
	if (PR && PR_len) {
		aux16 = (INT16U)DPP_ATTR_R_PROTOCOL_KEY;
		_I2B_LE(&aux16, &p);
		aux16 = (INT16U)PR_len;
		_I2B_LE(&aux16, &p);
		_InB(PR, &p, PR_len);
	}

	/* Protocol Version */
	aux16 = (INT16U)DPP_ATTR_PROTOCOL_VERSION;
	_I2B_LE(&aux16, &p);
	aux16 = 1;
	_I2B_LE(&aux16, &p);
	aux8 = DPP_VERSION;
	_I1B(&aux8, &p);

	/* Wrapped data ({R-nonce, I-nonce, R-capabilities, {R-auth}ke}k2) */
	INT8U *p2 = clear;
	if (r_nonce) {
		/* R-nonce */
		aux16 = (INT16U)DPP_ATTR_R_NONCE;
		_I2B_LE(&aux16, &p2);
		aux16 = (INT16U)nonce_len;
		_I2B_LE(&aux16, &p2);
		_InB(r_nonce, &p2, nonce_len);
	}

	if (i_nonce) {
		/* I-nonce */
		aux16 = (INT16U)DPP_ATTR_I_NONCE;
		_I2B_LE(&aux16, &p2);
		aux16 = (INT16U)nonce_len;
		_I2B_LE(&aux16, &p2);
		_InB(i_nonce, &p2, nonce_len);
	}

	/* R-capabilities */
	aux16 = (INT16U)DPP_ATTR_R_CAPABILITIES;
	_I2B_LE(&aux16, &p2);
	aux16 = (INT16U)1;
	_I2B_LE(&aux16, &p2);
	aux8 = auth->r_capab;
	_I1B(&aux8, &p2);

	if (wrapped_r_auth && wrapped_r_auth_len) {
		/* {R-auth}ke */
		aux16 = (INT16U)DPP_ATTR_WRAPPED_DATA;
		_I2B_LE(&aux16, &p2);
		aux16 = (INT16U)wrapped_r_auth_len;
		_I2B_LE(&aux16, &p2);
		_InB(wrapped_r_auth, &p2, wrapped_r_auth_len);
	}

	/* Associated data: OUI, OUI type, Crypto Suite, DPP frame type */
	addr[0] = msg + 2;
	len[0]  = 3 + 1 + 1 + 1;

	/* Attributes before Wrapped Data */
	addr[1] = msg + 2 + 3 + 1 + 1 + 1;
	len[1]  = p - (msg + 2 + 3 + 1 + 1 + 1);

	siv_len = p2 - clear;
	if (PLATFORM_AES_SIV_ENCRYPT(siv_key, auth->curve->hash_len, clear, siv_len, 2, addr, len, wrapped_data) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot encrypt with siv_key\n");
		PLATFORM_FREE(msg);
		return 0;
	}

	/* Wrapped data */
	siv_len += AES_BLOCK_SIZE;
	aux16 = (INT16U)DPP_ATTR_WRAPPED_DATA;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)siv_len;
	_I2B_LE(&aux16, &p);
	_InB(wrapped_data, &p, siv_len);

	*auth_resp_msg = msg;
	*auth_resp_len = attr_out_len;
	return 1;
}

// DPP Authentication Response Builder (with DPP status OK)
INT8U dpp_auth_build_resp_ok(struct dpp_authentication *auth, INT8U **auth_resp_msg, INT32U *auth_resp_msg_len)
{
	ECCKeyPair *PI        = &(auth->peer_protocol_key);
	ECCKeyPair *pR        = &(auth->own_protocol_key);
	int         hash_len  = auth->curve->hash_len;
	int         nonce_len = auth->curve->nonce_len;

	INT8U  r_auth[4 + DPP_MAX_HASH_LEN];
	INT8U  wrapped_r_auth[4 + DPP_MAX_HASH_LEN + AES_BLOCK_SIZE];
	INT32U wrapped_r_auth_len;

	INT8U *p;
	INT16U aux16;

	PLATFORM_PRINTF_INFO("DPP: Build Authentication Response\n");

	/* R-nonce */
	if (PLATFORM_GET_RANDOM_BYTES(auth->r_nonce, nonce_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot generate R-nonce\n");
		return 0;
	}

	/* R-capabilities */
	auth->r_capab = DPP_CAPAB_ENROLLEE; // In MAP, Responder is always Enrollee

	/* Responder's own protocol key */
	if (PLATFORM_GENERATE_EC_KEYPAIR(PI->curve, pR) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: protocol key generation failed..\n");
		return 0;
	}

	/* ECDH: N = pR * PI */
	if (PLATFORM_COMPUTE_ECDH_SHARED_SECRET(pR, PI, auth->Nx, &(auth->Nx_len)) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot calculate ecdh\n");
		return 0;
	}

	if (dpp_derive_k2(auth->Nx, auth->Nx_len, auth->k2, hash_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: k2 derivation error!\n");
		return 0;
	}

	if (dpp_derive_bk_ke(auth) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: bk and ke derivation error!\n");
		return 0;
	}

	/* R-auth = H(I-nonce | R-nonce | PI.x | PR.x | [BI.x |] BR.x | 0) */
	if (dpp_gen_r_auth(auth, r_auth + 4) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: R-auth generation error!\n");
		return 0;
	}

	/* {R-auth}ke */
	p     = r_auth;
	aux16 = (INT16U)DPP_ATTR_R_AUTH_TAG;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)hash_len;
	_I2B_LE(&aux16, &p);
	if (PLATFORM_AES_SIV_ENCRYPT(auth->ke, hash_len, r_auth, 4 + hash_len, 0, NULL, NULL, wrapped_r_auth) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot encrypt with k1\n");
		return 0;
	}
	wrapped_r_auth_len = 4 + hash_len + AES_BLOCK_SIZE;

	INT8U *pr_coordinates     = pR->pub_ansix963 + 1;
	INT8U  pr_coordinates_len = pR->pub_ansix963_len - 1;
	INT8U *r_pubkey_hash      = auth->own_bi->pubkey_hash;
	INT8U *i_nonce            = auth->i_nonce;
	INT8U *r_nonce            = auth->r_nonce;

	if (dpp_auth_build_resp(auth, DPP_STATUS_OK,
	                        pr_coordinates, pr_coordinates_len,
	                        nonce_len,
	                        r_pubkey_hash,
	                        r_nonce, i_nonce,
	                        wrapped_r_auth, wrapped_r_auth_len,
	                        auth->k2,
	                        auth_resp_msg, auth_resp_msg_len)
	    != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Authentication Response Generation Error!\n");
		return 0;
	}

	PLATFORM_PRINTF_INFO("DPP: Authentication Response Generated\n");
	return 1;
}

// DPP Authentication Response Parser
INT8U dpp_rx_auth_resp(struct dpp_authentication *auth, INT8U *buf, INT32U len)
{
	ECCKeyPair *PR = &(auth->peer_protocol_key);
	ECCKeyPair *pI = &(auth->own_protocol_key);

	INT16U r_bootstrap_len = 0, wrapped_data_len = 0, status_len = 0,
	       r_proto_len = 0, r_nonce_len = 0, i_nonce_len = 0,
	       r_capab_len = 0, version_len = 0, wrapped2_len = 0, r_auth_len = 0;
	const INT8U *r_bootstrap, *wrapped_data, *status, *r_proto,
	    *r_nonce, *i_nonce, *r_capab, *wrapped2, *r_auth, *version;

	int    unwrapped_len = 0, unwrapped2_len = 0;
	INT8U *unwrapped = 0, *unwrapped2 = 0;
	INT8U  r_auth2[DPP_MAX_HASH_LEN];

	INT8U *attr_start = buf + 8;
	INT32U attr_len   = len - 8;

	if (!auth->in_progress) {
		PLATFORM_PRINTF_WARNING("DPP: No DPP Authentication in progress - drop\n");
		return 0;
	}

	// Get wrapped data {...}k2
	wrapped_data = dpp_get_attr(attr_start, attr_len, DPP_ATTR_WRAPPED_DATA, &wrapped_data_len);
	if (!wrapped_data || wrapped_data_len < AES_BLOCK_SIZE) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required Wrapped Data attribute\n");
		return 0;
	}

	// only search things not related to wrapped data
	attr_len = wrapped_data - 4 - attr_start;

	// Get SHA256(BR)
	r_bootstrap = dpp_get_attr(attr_start, attr_len, DPP_ATTR_R_BOOTSTRAP_KEY_HASH, &r_bootstrap_len);
	if (!r_bootstrap || r_bootstrap_len != SHA256_MAC_LEN) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required SHA256(BR) attribute\n");
		return 0;
	}

	// Compare received SHA256(BR) and local SHA256(BR)
	if (PLATFORM_MEMCMP(auth->peer_bi.pubkey_hash, r_bootstrap, SHA256_MAC_LEN) != 0) {
		PLATFORM_PRINTF_WARNING("DPP: BR does not match hashed peer bootstrapping key\n");
		return 0;
	}

	// Get DPP Version
	version = dpp_get_attr(attr_start, attr_len, DPP_ATTR_PROTOCOL_VERSION, &version_len);
	if (!version || version_len < 1 || version[0] != DPP_VERSION) {
		PLATFORM_PRINTF_WARNING("DPP: DPP version is not %d\n", DPP_VERSION);
		return 0;
	}

	// Get DPP Status
	status = dpp_get_attr(attr_start, attr_len, DPP_ATTR_STATUS, &status_len);
	if (!status || status_len < 1) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required DPP Status attribute\n");
		return 0;
	}

	if (status[0] != DPP_STATUS_OK) {
		// TODO: do different actions
		PLATFORM_PRINTF_WARNING("DPP: status is not OK\n");
		return 0;
	}

	// Get PR (Responder's Public Protocol Key)
	r_proto = dpp_get_attr(attr_start, attr_len, DPP_ATTR_R_PROTOCOL_KEY, &r_proto_len);
	if (!r_proto) {
		PLATFORM_PRINTF_WARNING("DPP: Missing required Responder Protocol Key attribute\n");
		return 0;
	}

	// Convert Protocol Key to ECCKeyPair
	INT8U r_proto_ansix[MAX_ANSI_X963_LEN];
	r_proto_ansix[0] = 0x04; // add back 0x04
	memcpy(r_proto_ansix + 1, r_proto, r_proto_len);
	if (PLATFORM_PARSE_EC_PUBLIC_ANSIX963(auth->curve->name, r_proto_ansix, r_proto_len + 1, PR) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: the coordinates are incorrect\n");
		return 0;
	}

	/* ECDH: N = pI * PR */
	if (PLATFORM_COMPUTE_ECDH_SHARED_SECRET(pI, PR, auth->Nx, &(auth->Nx_len)) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot calculate ecdh\n");
		return 0;
	}

	if (dpp_derive_k2(auth->Nx, auth->Nx_len, auth->k2, auth->curve->hash_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: k1 derivation error!\n");
		return 0;
	}

	/* Associated data protected by k2 */
	const INT8U *addr[2];
	INT32U       alen[2];

	/* Associated data: OUI, OUI type, Crypto Suite, DPP frame type */
	addr[0] = buf + 2;
	alen[0] = 3 + 1 + 1 + 1;

	/* Attributes before Wrapped Data */
	addr[1]       = attr_start;
	alen[1]       = attr_len;
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped     = PLATFORM_MALLOC(unwrapped_len);

	/* Decrypt wrapped data */
	if (PLATFORM_AES_SIV_DECRYPT(auth->k2, auth->curve->hash_len,
	                             wrapped_data, wrapped_data_len, 2, addr, alen, unwrapped)
	    != 1) {
		PLATFORM_PRINTF_WARNING("DPP: AES-SIV decryption failed\n");
		goto fail;
	}

	// Check decrypted content format
	if (dpp_check_attrs(unwrapped, unwrapped_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Invalid attribute in unwrapped data\n");
		goto fail;
	}

	/* R-nonce */
	r_nonce = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_R_NONCE, &r_nonce_len);
	if (!r_nonce || r_nonce_len != auth->curve->nonce_len) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid R-nonce\n");
		goto fail;
	}
	PLATFORM_MEMCPY(auth->r_nonce, r_nonce, r_nonce_len);

	/* I-nonce */
	i_nonce = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_I_NONCE, &i_nonce_len);
	if (!i_nonce || i_nonce_len != auth->curve->nonce_len) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid I-nonce\n");
		goto fail;
	}

	if (PLATFORM_MEMCMP(auth->i_nonce, i_nonce, i_nonce_len) != 0) {
		PLATFORM_PRINTF_WARNING("DPP: I-nonce mismatch\n");
		goto fail;
	}

	/* R-capabilities */
	r_capab = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_R_CAPABILITIES, &r_capab_len);
	if (!r_capab || r_capab_len < 1) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid R-capabilities\n");
		goto fail;
	}

	/* Handle R-capabilities */
	auth->r_capab = r_capab[0];
	switch (auth->r_capab & DPP_CAPAB_ROLE_MASK) {
	case DPP_CAPAB_CONFIGURATOR:
		PLATFORM_PRINTF_WARNING("DPP: Responder is Configurator! Not normal...\n");
		break;
	case DPP_CAPAB_ENROLLEE:
		PLATFORM_PRINTF_DEBUG("DPP: Responder is Enrollee! Good for MAP!\n");
		break;
	case DPP_CAPAB_CONFIGURATOR | DPP_CAPAB_ENROLLEE:
		PLATFORM_PRINTF_WARNING("DPP: Responder is both Configurator and Enrollee!\n");
		break;
	default:
		PLATFORM_PRINTF_WARNING("DPP: R-capabilities: what is %02X??\n", auth->i_capab);
		break;
	}

	/* {R-auth}ke */
	wrapped2 = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_WRAPPED_DATA, &wrapped2_len);
	if (!wrapped2 || wrapped2_len < AES_BLOCK_SIZE) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid Secondary Wrapped Data");
		goto fail;
	}

	if (dpp_derive_bk_ke(auth) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: bk/ke derivation error!\n");
		goto fail;
	}

	/* Decrypt Secondary Wrapped Data */
	unwrapped2_len = wrapped2_len - AES_BLOCK_SIZE;
	unwrapped2     = PLATFORM_MALLOC(unwrapped2_len);
	if (PLATFORM_AES_SIV_DECRYPT(auth->ke, auth->curve->hash_len, wrapped2, wrapped2_len, 0, NULL, NULL, unwrapped2) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: AES-SIV decryption failed\n");
		goto fail;
	}

	// Check decrypted content format
	if (dpp_check_attrs(unwrapped2, unwrapped2_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Invalid attribute in secondary unwrapped data\n");
		goto fail;
	}

	/* R-auth */
	r_auth = dpp_get_attr(unwrapped2, unwrapped2_len, DPP_ATTR_R_AUTH_TAG, &r_auth_len);
	if (!r_auth || r_auth_len != auth->curve->hash_len) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid Responder Authenticating Tag\n");
		goto fail;
	}

	if (dpp_gen_r_auth(auth, r_auth2) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Cannot Generate R-auth for Matching\n");
		goto fail;
	}

	if (PLATFORM_MEMCMP(r_auth, r_auth2, r_auth_len) != 0) {
		PLATFORM_PRINTF_WARNING("DPP: Mismatching Responder Authenticating Tag\n");
		goto fail;
	}

	PLATFORM_FREE(unwrapped);
	PLATFORM_FREE(unwrapped2);
	PLATFORM_PRINTF_INFO("DPP: Authentication Response Rx finished...\n");
	return 1;

fail:
	PLATFORM_FREE(unwrapped);
	PLATFORM_FREE(unwrapped2);
	return 0;
}

// DPP Authentication Confirm Builder
INT8U dpp_auth_build_conf(struct dpp_authentication *auth, enum dpp_status_error status, INT8U **auth_conf_msg, INT32U *auth_conf_msg_len)
{
	const INT8U *addr[2];
	INT32U       len[2];
	INT32U       attr_len, attr_out_len;

	INT8U        i_auth[4 + DPP_MAX_HASH_LEN];
	const INT32U i_auth_len = 4 + auth->curve->hash_len;
	INT8U        r_nonce[4 + DPP_MAX_NONCE_LEN];
	const INT32U r_nonce_len = 4 + auth->curve->nonce_len;

	INT8U *r_pubkey_hash = auth->peer_bi.pubkey_hash;

	PLATFORM_PRINTF_INFO("DPP: Build Authentication Confirm\n");

	/* Build DPP Authentication Response frame attributes */
	attr_len = 4 + 1;                                                                      // DPP Status
	attr_len += 4 + SHA256_MAC_LEN;                                                        // SHA256(BR)
	attr_len += 4 + (status == DPP_STATUS_OK ? i_auth_len : r_nonce_len) + AES_BLOCK_SIZE; // I-auth for OK, R-nonce otherwise

	/* Allocate message buffer */
	INT8U *msg = dpp_alloc_msg(DPP_PA_AUTHENTICATION_CONF, attr_len, &attr_out_len);
	if (!msg) {
		PLATFORM_PRINTF_WARNING("DPP: alloc_msg failed!\n");
		return 0;
	}
	INT8U *p = msg + (attr_out_len - attr_len);
	INT16U aux16;
	INT8U  aux8;

	/* DPP Status */
	aux16 = (INT16U)DPP_ATTR_STATUS;
	_I2B_LE(&aux16, &p);
	aux16 = 1;
	_I2B_LE(&aux16, &p);
	aux8 = (INT8U)status;
	_I1B(&aux8, &p);

	/* Responder Bootstrapping Key Hash SHA256(BR) */
	aux16 = (INT16U)DPP_ATTR_R_BOOTSTRAP_KEY_HASH;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)SHA256_MAC_LEN;
	_I2B_LE(&aux16, &p);
	_InB(r_pubkey_hash, &p, SHA256_MAC_LEN);

	/* Associated data: OUI, OUI type, Crypto Suite, DPP frame type */
	addr[0] = msg + 2;
	len[0]  = 3 + 1 + 1 + 1;

	/* Attributes before Wrapped Data */
	addr[1] = msg + 2 + 3 + 1 + 1 + 1;
	len[1]  = p - (msg + 2 + 3 + 1 + 1 + 1);

	/* Wrapped I-auth or R-nonce */
	aux16 = (INT16U)DPP_ATTR_WRAPPED_DATA;
	_I2B_LE(&aux16, &p);

	if (status == DPP_STATUS_OK) {

		/* Length of wrapped I-auth */
		aux16 = (INT16U)i_auth_len + AES_BLOCK_SIZE;
		_I2B_LE(&aux16, &p);

		/* I-auth wrapped with ke */
		/* I-auth = H(R-nonce | I-nonce | PR.x | PI.x | BR.x | [BI.x |] 1) */
		if (dpp_gen_i_auth(auth, i_auth + 4) != 1) {
			PLATFORM_PRINTF_WARNING("DPP: I-auth generation error!\n");
			free(msg);
			return 0;
		}

		/* {I-auth}ke */
		INT8U *p2 = i_auth;
		aux16     = (INT16U)DPP_ATTR_I_AUTH_TAG;
		_I2B_LE(&aux16, &p2);
		aux16 = (INT16U)auth->curve->hash_len;
		_I2B_LE(&aux16, &p2);
		if (PLATFORM_AES_SIV_ENCRYPT(auth->ke, auth->curve->hash_len, i_auth, i_auth_len, 2, addr, len, p) != 1) {
			PLATFORM_PRINTF_WARNING("DPP: cannot encrypt with ke\n");
			free(msg);
			return 0;
		}
		p += (i_auth_len + AES_BLOCK_SIZE);

	} else {

		/* Length of wrapped R-nonce */
		aux16 = (INT16U)r_nonce_len + AES_BLOCK_SIZE;
		_I2B_LE(&aux16, &p);

		/* R-nonce wrapped with k2 */
		INT8U *p2 = r_nonce;
		aux16     = (INT16U)DPP_ATTR_R_NONCE;
		_I2B_LE(&aux16, &p2);
		aux16 = (INT16U)auth->curve->nonce_len;
		_I2B_LE(&aux16, &p2);
		_InB(auth->r_nonce, &p2, auth->curve->nonce_len);
		if (PLATFORM_AES_SIV_ENCRYPT(auth->k2, auth->curve->hash_len, r_nonce, r_nonce_len, 2, addr, len, p) != 1) {
			PLATFORM_PRINTF_WARNING("DPP: cannot encrypt with ke\n");
			free(msg);
			return 0;
		}
		p += (r_nonce_len + AES_BLOCK_SIZE);
	}

	/* Update states */
	if (status == DPP_STATUS_OK)
		dpp_auth_success(auth);

	*auth_conf_msg     = msg;
	*auth_conf_msg_len = attr_out_len;
	PLATFORM_PRINTF_INFO("DPP: Authentication Confirm Generated\n");
	return 1;
}

// DPP Authentication Confirm Parser
INT8U dpp_rx_auth_conf(struct dpp_authentication *auth, INT8U *buf, INT32U len)
{
	INT8U *attr_start = buf + 8;
	INT32U attr_len   = len - 8;

	const INT8U *wrapped_data, *r_bootstrap, *status, *i_auth;
	INT16U       wrapped_data_len = 0, r_bootstrap_len = 0, status_len = 0, i_auth_len = 0;
	int          unwrapped_len = 0;
	INT8U *      unwrapped     = 0;

	INT8U i_auth2[DPP_MAX_HASH_LEN];

	if (!auth->in_progress) {
		PLATFORM_PRINTF_WARNING("DPP: No DPP Authentication in progress - drop\n");
		return 0;
	}

	if (auth->initiator) {
		PLATFORM_PRINTF_WARNING("DPP: Unexpected Authentication Confirm\n");
		return 0;
	}

	// Get {I-auth}ke
	wrapped_data = dpp_get_attr(attr_start, attr_len, DPP_ATTR_WRAPPED_DATA, &wrapped_data_len);
	if (!wrapped_data || wrapped_data_len < AES_BLOCK_SIZE) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required Wrapped Data attribute\n");
		return 0;
	}

	// only search things not related to wrapped data
	attr_len = wrapped_data - 4 - attr_start;

	// Get SHA256(BR)
	r_bootstrap = dpp_get_attr(attr_start, attr_len, DPP_ATTR_R_BOOTSTRAP_KEY_HASH, &r_bootstrap_len);
	if (!r_bootstrap || r_bootstrap_len != SHA256_MAC_LEN) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required SHA256(BR) attribute\n");
		return 0;
	}

	if (PLATFORM_MEMCMP(r_bootstrap, auth->own_bi->pubkey_hash, SHA256_MAC_LEN) != 0) {
		PLATFORM_PRINTF_WARNING("DPP: Responder Bootstrapping Key Hash mismatch\n");
		return 0;
	}

	// Get DPP Status
	status = dpp_get_attr(attr_start, attr_len, DPP_ATTR_STATUS, &status_len);
	if (!status || status_len < 1) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required DPP Status attribute\n");
		return 0;
	}

	if (status[0] != DPP_STATUS_OK) {
		PLATFORM_PRINTF_WARNING("DPP: status is not OK\n");
		return 0;
	}

	/* Associated data protected by ke */
	const INT8U *addr[2];
	INT32U       alen[2];

	/* Associated data: OUI, OUI type, Crypto Suite, DPP frame type */
	addr[0] = buf + 2;
	alen[0] = 3 + 1 + 1 + 1;

	/* Attributes before Wrapped Data */
	addr[1]       = attr_start;
	alen[1]       = attr_len;
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped     = PLATFORM_MALLOC(unwrapped_len);

	/* Decrypt wrapped data */
	if (PLATFORM_AES_SIV_DECRYPT(auth->ke, auth->curve->hash_len,
	                             wrapped_data, wrapped_data_len, 2, addr, alen, unwrapped)
	    != 1) {
		PLATFORM_PRINTF_WARNING("DPP: AES-SIV decryption failed\n");
		goto fail;
	}

	// Check decrypted content format
	if (dpp_check_attrs(unwrapped, unwrapped_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Invalid attribute in unwrapped data\n");
		goto fail;
	}

	// Get I-auth
	i_auth = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_I_AUTH_TAG, &i_auth_len);
	if (!i_auth || i_auth_len != auth->curve->hash_len) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid Initiator Authenticating Tag\n");
		goto fail;
	}

	/* I-auth' = H(R-nonce | I-nonce | PR.x | PI.x | BR.x | [BI.x |] 1) */
	if (dpp_gen_i_auth(auth, i_auth2) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: I-auth generation error!\n");
		goto fail;
	}

	if (PLATFORM_MEMCMP(i_auth, i_auth2, i_auth_len) != 0) {
		PLATFORM_PRINTF_WARNING("DPP: Mismatching Initiator Authenticating Tag\n");
		goto fail;
	}

	/* Update states */
	dpp_auth_success(auth);

	PLATFORM_FREE(unwrapped);
	PLATFORM_PRINTF_INFO("DPP: Authentication Confirm Rx finished...\n");
	return 1;

fail:
	PLATFORM_FREE(unwrapped);
	return 0;
}

static INT8U *dpp_gas_build_req(INT8U action, INT8U dialog_token, INT16U len, INT16U *out_len)
{
	INT8U  aux8;
	INT16U aux16;
	INT8U *msg;
	INT8U *p;

	*out_len = 15 + len;
	msg      = PLATFORM_MALLOC(*out_len);
	PLATFORM_MEMSET(msg, 0, *out_len);
	p = msg;

	/* Category: Public Action Frame */
	aux8 = WLAN_ACTION_PUBLIC;
	_I1B(&aux8, &p);

	/* Action Field */
	aux8 = action;
	_I1B(&aux8, &p);

	/* Dialog Token */
	aux8 = dialog_token;
	_I1B(&aux8, &p);

	/* Advertisement Protocol Element */
	aux8 = 0x6C;
	_I1B(&aux8, &p);
	aux8 = 0x08;
	_I1B(&aux8, &p);
	aux8 = 0x00;
	_I1B(&aux8, &p);

	/*Advertisement Protocol ID */
	aux8 = 0xDD;
	_I1B(&aux8, &p);
	aux8 = 0x05;
	_I1B(&aux8, &p);

	/* OUI: Wi-Fi Alliance */
	aux8 = (OUI_WFA >> 16) & 0xFF;
	_I1B(&aux8, &p);
	aux8 = (OUI_WFA >> 8) & 0xFF;
	_I1B(&aux8, &p);
	aux8 = (OUI_WFA)&0xFF;
	_I1B(&aux8, &p);

	/* OUI Type: DPP */
	aux8 = DPP_OUI_TYPE;
	_I1B(&aux8, &p);

	/* Type: DPP Configuration Protocol */
	aux8 = 0x01;
	_I1B(&aux8, &p);

	/* Query Request Length */
	aux16 = len;
	_I2B_LE(&aux16, &p);

	return msg;
}

static INT8U *dpp_gas_build_resp(INT8U  action,
                                 INT8U  dialog_token,
                                 INT16U status_code,
                                 INT8U frag_id, INT8U more, INT16U comeback_delay,
                                 INT16U len, INT16U *out_len)
{
	INT8U  aux8;
	INT16U aux16;
	INT8U *msg;
	INT8U *p;

	*out_len = (action == WLAN_PA_GAS_COMEBACK_RESP) ? 20 + len : 19 + len;
	msg      = PLATFORM_MALLOC(*out_len);
	PLATFORM_MEMSET(msg, 0, *out_len);
	p = msg;

	/* Category: Public Action Frame */
	aux8 = WLAN_ACTION_PUBLIC;
	_I1B(&aux8, &p);

	/* Action Field */
	aux8 = action;
	_I1B(&aux8, &p);

	/* Dialog Token */
	aux8 = dialog_token;
	_I1B(&aux8, &p);

	/* Status Code */
	aux16 = status_code;
	_I2B_LE(&aux16, &p);

	/* Comeback Response */
	if (action == WLAN_PA_GAS_COMEBACK_RESP) {
		aux8 = frag_id | (more ? 0x80 : 0);
		_I1B(&aux8, &p);
	}

	/* Comeback Delay */
	aux16 = comeback_delay;
	_I2B_LE(&aux16, &p);

	/* Advertisement Protocol Element */
	aux8 = 0x6C;
	_I1B(&aux8, &p);
	aux8 = 0x08;
	_I1B(&aux8, &p);
	aux8 = 0x7f;
	_I1B(&aux8, &p);

	/*Advertisement Protocol ID */
	aux8 = 0xDD;
	_I1B(&aux8, &p);
	aux8 = 0x05;
	_I1B(&aux8, &p);

	/* OUI: Wi-Fi Alliance */
	aux8 = (OUI_WFA >> 16) & 0xFF;
	_I1B(&aux8, &p);
	aux8 = (OUI_WFA >> 8) & 0xFF;
	_I1B(&aux8, &p);
	aux8 = (OUI_WFA)&0xFF;
	_I1B(&aux8, &p);

	/* OUI Type: DPP */
	aux8 = DPP_OUI_TYPE;
	_I1B(&aux8, &p);

	/* Type: DPP Configuration Protocol */
	aux8 = 0x01;
	_I1B(&aux8, &p);

	/* Response Length */
	aux16 = len;
	_I2B_LE(&aux16, &p);

	return msg;
}

static INT8U dpp_build_conf_req_obj(struct dpp_config_req_obj *obj, char **json_out)
{
	cJSON *configReq = NULL, *bSTAList = NULL, *bSTA = NULL;
	INT8U  ret = 0;
	int    i;

	configReq = cJSON_CreateObject();
	cJSON_AddStringToObject(configReq, "name", obj->name);
	cJSON_AddStringToObject(configReq, "wi-fi_tech", obj->wifi_tech);
	cJSON_AddStringToObject(configReq, "netRole", dpp_netrole_str(obj->netRole));

	if (obj->b_sta_list_count) {
		bSTAList = cJSON_CreateArray();
		for (i = 0; i < obj->b_sta_list_count; i++) {
			bSTA = cJSON_CreateObject();
			cJSON_AddStringToObject(bSTA, "netRole", dpp_netrole_str(obj->b_sta_list[i].netRole));
			cJSON_AddStringToObject(bSTA, "akm", obj->b_sta_list[i].akm);
			cJSON_AddStringToObject(bSTA, "channelList", obj->b_sta_list[i].channelList);
			cJSON_AddItemToArray(bSTAList, bSTA);
		}
		cJSON_AddItemToObject(configReq, "bSTAList", bSTAList);
	}

	*json_out = cJSON_PrintUnformatted(configReq);
	if (!*json_out) {
		PLATFORM_PRINTF_WARNING("DPP: cannot generate conf req obj json!\n");
		goto fail;
	}

	ret = 1;
fail:
	cJSON_Delete(configReq);
	return ret;
}

static INT8U dpp_parse_conf_req_obj(char *json, struct dpp_config_req_obj *obj_out)
{
	cJSON *req_obj = NULL, *token = NULL, *t = NULL;
	INT8U  ret = 0;

	req_obj = cJSON_Parse((char *)json);
	if (!req_obj) {
		PLATFORM_PRINTF_WARNING("DPP: Could not parse Config Attributes\n");
		goto fail;
	}

	// JSON: name
	token = cJSON_GetObjectItem(req_obj, "name");
	if (token == NULL || token->type != cJSON_String) {
		PLATFORM_PRINTF_WARNING("DPP: No Config Attributes - name\n");
	} else {
		strlcpy(obj_out->name, token->valuestring, sizeof(obj_out->name));
		PLATFORM_PRINTF_DEBUG("DPP: Enrollee name = %s\n", obj_out->name);
	}

	// JSON: wifi-tech
	token = cJSON_GetObjectItem(req_obj, "wi-fi_tech");
	if (token == NULL || token->type != cJSON_String) {
		PLATFORM_PRINTF_WARNING("DPP: No Config Attributes - wi-fi_tech\n");
		goto fail;
	}
	strlcpy(obj_out->wifi_tech, token->valuestring, sizeof(obj_out->wifi_tech));
	PLATFORM_PRINTF_DEBUG("DPP: wifi-tech = %s\n", obj_out->wifi_tech);

	// JSON: netRole
	token = cJSON_GetObjectItem(req_obj, "netRole");
	if (token == NULL || token->type != cJSON_String) {
		PLATFORM_PRINTF_WARNING("DPP: No Config Attributes - netRole\n");
		goto fail;
	}
	obj_out->netRole = dpp_str_netrole(token->valuestring);
	PLATFORM_PRINTF_DEBUG("DPP: netRole = %s\n", dpp_netrole_str(obj_out->netRole));

	// JSON: bSTAList
	token = cJSON_GetObjectItem(req_obj, "bSTAList");
	if (token != NULL && token->type == cJSON_Array) {
		obj_out->b_sta_list_count = 0;
		cJSON_ArrayForEach(t, token)
		{
			cJSON *netRole     = cJSON_GetObjectItem(t, "netRole");
			cJSON *akm         = cJSON_GetObjectItem(t, "akm");
			cJSON *channelList = cJSON_GetObjectItem(t, "channelList");
			if (obj_out->b_sta_list_count < DPP_MAX_CONF_REQ_OBJ_BSTALIST
			    && netRole && netRole->type == cJSON_String
			    && akm && akm->type == cJSON_String
			    && channelList && channelList->type == cJSON_String) {
				PLATFORM_PRINTF_DEBUG("DPP: netRole: %s\n", netRole->valuestring);
				PLATFORM_PRINTF_DEBUG("DPP: akm: %s\n", akm->valuestring);
				PLATFORM_PRINTF_DEBUG("DPP: channelList: %s\n", channelList->valuestring);
				obj_out->b_sta_list[obj_out->b_sta_list_count].netRole = dpp_str_netrole(netRole->valuestring);
				strlcpy(obj_out->b_sta_list[obj_out->b_sta_list_count].akm, akm->valuestring, sizeof(obj_out->b_sta_list[0].akm));
				strlcpy(obj_out->b_sta_list[obj_out->b_sta_list_count].channelList, channelList->valuestring, sizeof(obj_out->b_sta_list[0].channelList));
				obj_out->b_sta_list_count++;
			}
		}
	}

	ret = 1;
fail:
	cJSON_Delete(req_obj);
	return ret;
}

// DPP Configuration Request Builder
INT8U dpp_build_conf_req(struct dpp_authentication *auth,
                         struct dpp_config_req_obj *obj,
                         INT8U **conf_req_msg, INT32U *conf_req_msg_len)
{
	char * configReqBuf = NULL;
	INT32U nonce_len, json_len, clear_len;
	INT16U attr_len, attr_out_len;
	INT8U *clear = NULL, *msg = NULL;

	INT8U *p;
	INT16U aux16;
	INT8U  ret = 0;

	PLATFORM_PRINTF_INFO("DPP: Build Configuration Request\n");

	// Sanity Check
	if (!auth->curve) {
		PLATFORM_PRINTF_WARNING("DPP: auth object is problematic\n");
		return 0;
	}

	/* configAttrib: the configuration request object */
	if (dpp_build_conf_req_obj(obj, &configReqBuf) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: conf req object is problematic\n");
		goto fail;
	}
	nonce_len = auth->curve->nonce_len;
	json_len  = PLATFORM_STRLEN(configReqBuf);

	/* E-nonce */
	if (PLATFORM_GET_RANDOM_BYTES(auth->e_nonce, nonce_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot generate R-nonce\n");
		goto fail;
	}

	/* Build { E-nonce, configAttrib }ke */
	clear_len = 4 + nonce_len;
	clear_len += 4 + json_len;
	attr_len = 4 + clear_len + AES_BLOCK_SIZE;

	/* Allocate message buffer */
	clear = PLATFORM_MALLOC(clear_len);
	msg   = dpp_gas_build_req(WLAN_PA_GAS_INITIAL_REQ, 0, attr_len, &attr_out_len);
	if (!msg) {
		PLATFORM_PRINTF_WARNING("DPP: build_req failed!\n");
		goto fail;
	}

	/* E-nonce */
	p     = clear;
	aux16 = DPP_ATTR_ENROLLEE_NONCE;
	_I2B_LE(&aux16, &p);
	aux16 = nonce_len;
	_I2B_LE(&aux16, &p);
	_InB(auth->e_nonce, &p, nonce_len);

	/* configAttrib */
	aux16 = DPP_ATTR_CONFIG_ATTR_OBJ;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)json_len;
	_I2B_LE(&aux16, &p);
	_InB(configReqBuf, &p, json_len);

	/* Wrapped E-nonce and configAttrib */
	p     = msg + (attr_out_len - attr_len);
	aux16 = DPP_ATTR_WRAPPED_DATA;
	_I2B_LE(&aux16, &p);
	aux16 = clear_len + AES_BLOCK_SIZE;
	_I2B_LE(&aux16, &p);
	if (PLATFORM_AES_SIV_ENCRYPT(auth->ke, auth->curve->hash_len, clear, clear_len, 0, NULL, NULL, p) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot encrypt with ke\n");
		goto fail;
	}
	p += (clear_len + AES_BLOCK_SIZE);

	*conf_req_msg     = msg;
	*conf_req_msg_len = attr_out_len;
	PLATFORM_PRINTF_INFO("DPP: Configuration Request Generated\n");
	ret               = 1;

fail:
	if (!ret)
		PLATFORM_FREE(msg);
	PLATFORM_FREE(clear);
	PLATFORM_FREE(configReqBuf);
	return ret;
}

// DPP Configuration Request Parser
INT8U dpp_conf_req_rx(struct dpp_authentication *auth,
                      struct dpp_config_req_obj *obj_out, INT8U *buf, INT32U len)
{
	INT8U *attr_start = buf + 15;
	INT32U attr_len   = len - 15;

	const INT8U *wrapped_data, *e_nonce, *config_attr;
	INT16U       wrapped_data_len = 0, e_nonce_len = 0, config_attr_len = 0;
	int          unwrapped_len = 0;
	INT8U *      unwrapped     = 0;

	if (!auth->auth_success) {
		PLATFORM_PRINTF_WARNING("DPP: auth_success is false\n");
		return 0;
	}

	if (dpp_check_attrs(attr_start, attr_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Invalid attribute in config request\n");
		return 0;
	}

	// Get { E-nonce, configAttrib }ke
	wrapped_data = dpp_get_attr(attr_start, attr_len, DPP_ATTR_WRAPPED_DATA, &wrapped_data_len);
	if (!wrapped_data || wrapped_data_len < AES_BLOCK_SIZE) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required Wrapped Data attribute\n");
		return 0;
	}

	/* Decrypt wrapped data */
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped     = PLATFORM_MALLOC(unwrapped_len);
	if (PLATFORM_AES_SIV_DECRYPT(auth->ke, auth->curve->hash_len,
	                             wrapped_data, wrapped_data_len, 0, NULL, NULL, unwrapped)
	    != 1) {
		PLATFORM_PRINTF_WARNING("DPP: AES-SIV decryption failed\n");
		goto fail;
	}

	// Check decrypted content format
	if (dpp_check_attrs(unwrapped, unwrapped_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Invalid attribute in unwrapped data\n");
		goto fail;
	}

	// Get E-nonce
	e_nonce = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_ENROLLEE_NONCE, &e_nonce_len);
	if (!e_nonce || e_nonce_len != auth->curve->nonce_len) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid Enrollee Nonce attribute\n");
		goto fail;
	}
	PLATFORM_MEMCPY(auth->e_nonce, e_nonce, e_nonce_len);

	// Get configAttrib
	config_attr = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_CONFIG_ATTR_OBJ, &config_attr_len);
	if (!config_attr) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid Config Attributes attribute\n");
		goto fail;
	}

	if (dpp_parse_conf_req_obj((char *)config_attr, obj_out) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Cannot parse configAttrib\n");
		goto fail;
	}

	/* Update states */
	// TODO: update config struct

	PLATFORM_FREE(unwrapped);
	PLATFORM_PRINTF_INFO("DPP: Configuration Request Rx finished...\n");
	return 1;

fail:
	PLATFORM_FREE(unwrapped);
	return 0;
}

INT8U *dpp_get_op_class_from_conf_req(struct dpp_config_req_obj *req_obj)
{
	static INT8U op_class[64] = { 0 };
	char        *channel_list = NULL;
	char        *pos          = NULL;
	int          opcls;
	INT8U        i, count = 0;

	if (!req_obj->b_sta_list_count || (req_obj->b_sta_list_count > DPP_MAX_CONF_REQ_OBJ_BSTALIST)) {
		PLATFORM_PRINTF_WARNING("DPP: Config Req contains invalid bSTA list attribute\n");
		return NULL;
	}
	pos          = PLATFORM_MALLOC(128);
	channel_list = pos;
	for (i = 0; i < req_obj->b_sta_list_count; i++) {
		if (0 == req_obj->b_sta_list[i].channelList[0]) {
			PLATFORM_PRINTF_WARNING("DPP: Config Req missing channel list\n");
			PLATFORM_FREE(pos);
			return NULL;
		}
		PLATFORM_STRCPY(channel_list, req_obj->b_sta_list[i].channelList);
		sscanf(channel_list, "%d/%*d", &opcls);
		op_class[count++] = opcls;
		while (strstr(channel_list, ",")) {
			channel_list = strstr(channel_list, ",") + 1;
			sscanf(channel_list, "%d/%*d", &opcls);
			op_class[count++] = opcls;
		}
	}
	PLATFORM_FREE(pos);
	return op_class;
}

//
//  Configuration Response
//

static char *dpp_build_jwk_prot_hdr(char *kid, const char *jws_alg)
{
	cJSON *jwk_prot_hdr = NULL;
	char * jwkph_buff   = NULL;

	// Construct JWK (JSON Web Key) object
	jwk_prot_hdr = cJSON_CreateObject();
	cJSON_AddStringToObject(jwk_prot_hdr, "typ", "dppCon");
	cJSON_AddStringToObject(jwk_prot_hdr, "kid", kid);
	cJSON_AddStringToObject(jwk_prot_hdr, "alg", jws_alg);
	jwkph_buff = cJSON_PrintUnformatted(jwk_prot_hdr);

	cJSON_Delete(jwk_prot_hdr);
	return jwkph_buff;
}

static INT8U dpp_parse_jwk_prot_hdr(const char *jwk_prot_hdr, char **kid, char **jws_alg)
{
	cJSON *cJSON_jwkheader = NULL, *token = NULL;
	char * _kid = NULL, *_jws_alg = NULL;

	// Parse JWK (JSON Web Key) object
	cJSON_jwkheader = cJSON_Parse((char *)jwk_prot_hdr);
	if (!cJSON_jwkheader) {
		PLATFORM_PRINTF_WARNING("DPP: Could not parse JWK Protocol Header\n");
		goto fail;
	}

	// typ = dppConn
	token = cJSON_GetObjectItem(cJSON_jwkheader, "typ");
	if (token == NULL || token->type != cJSON_String) {
		PLATFORM_PRINTF_WARNING("DPP: No Attributes - typ\n");
		goto fail;
	}

	if (PLATFORM_STRCMP(token->valuestring, "dppCon") != 0) {
		PLATFORM_PRINTF_WARNING("DPP: Unsupported typ %s\n", token->valuestring);
		goto fail;
	}

	// kid
	token = cJSON_GetObjectItem(cJSON_jwkheader, "kid");
	if (token == NULL || token->type != cJSON_String) {
		PLATFORM_PRINTF_WARNING("DPP: No Attributes - kid\n");
		goto fail;
	}
	_kid = PLATFORM_MALLOC(PLATFORM_STRLEN(token->valuestring) + 1);
	PLATFORM_STRCPY(_kid, token->valuestring);

	// alg (as known as jwk_alg)
	token = cJSON_GetObjectItem(cJSON_jwkheader, "alg");
	if (token == NULL || token->type != cJSON_String) {
		PLATFORM_PRINTF_WARNING("DPP: No Attributes - alg\n");
		goto fail;
	}
	_jws_alg = PLATFORM_MALLOC(PLATFORM_STRLEN(token->valuestring) + 1);
	PLATFORM_STRCPY(_jws_alg, token->valuestring);

	// return correctly
	*kid     = _kid;
	*jws_alg = _jws_alg;
	cJSON_Delete(cJSON_jwkheader);
	return 1;

fail:
	PLATFORM_FREE(_kid);
	PLATFORM_FREE(_jws_alg);
	cJSON_Delete(cJSON_jwkheader);
	return 0;
}

static cJSON *dpp_build_jwk(ECCKeyPair *key, char *kid, const struct dpp_curve_params *curve)
{
	cJSON *jwk         = NULL;
	size_t base64x_len = 0, base64y_len = 0;
	INT8U *base64x = NULL, *base64y = NULL;

	INT8U *pub     = key->pub_ansix963 + 1;
	INT8U  pub_len = key->pub_ansix963_len - 1;

	// Sanity check
	if (!key->pub_ansix963_len) {
		PLATFORM_PRINTF_WARNING("DPP: key coordinates empty!\n");
		return NULL;
	}

	// Encode coordinates to base64
	base64x = base64_url_encode(pub, pub_len / 2, &base64x_len);
	base64y = base64_url_encode(pub + pub_len / 2, pub_len / 2, &base64y_len);
	if (!base64x_len || !base64y_len) {
		PLATFORM_PRINTF_WARNING("DPP: base64 encode coordinates failed\n");
		goto fail;
	}

	// Construct JWK (JSON Web Key) object
	jwk = cJSON_CreateObject();
	cJSON_AddStringToObject(jwk, "kty", "EC");
	cJSON_AddStringToObject(jwk, "crv", curve->jwk_crv);
	cJSON_AddStringToObject(jwk, "x", (char *)base64x);
	cJSON_AddStringToObject(jwk, "y", (char *)base64y);
	if (kid)
		cJSON_AddStringToObject(jwk, "kid", kid);

	// Return cJSON JWK
	PLATFORM_FREE(base64x);
	PLATFORM_FREE(base64y);
	return jwk;

fail:
	PLATFORM_FREE(base64x);
	PLATFORM_FREE(base64y);
	cJSON_Delete(jwk);
	return NULL;
}

INT8U dpp_parse_jwk(cJSON *jwk, ECCKeyPair *pub_key_out, INT8U *kid_hash_out, char **kid_out)
{
	const struct dpp_curve_params *params = NULL;

	cJSON *cjson_kty = NULL, *cjson_crv = NULL,
	      *cjson_x = NULL, *cjson_y = NULL, *cjson_kid = NULL;
	size_t x_len = 0, y_len = 0, ansix963_len = 0;
	INT8U *x = NULL, *y = NULL, *ansix963 = NULL;
	INT8U *addr[1];
	INT32U len[1];
	INT8U  ret = 0;

	// Parse JSON
	cjson_kty = cJSON_GetObjectItem(jwk, "kty");
	if (cjson_kty == NULL || cjson_kty->type != cJSON_String) {
		PLATFORM_PRINTF_WARNING("DPP: No Attributes - kty\n");
		goto fail;
	}
	PLATFORM_PRINTF_DEBUG("DPP: kty = %s\n", cjson_kty->valuestring);

	cjson_crv = cJSON_GetObjectItem(jwk, "crv");
	if (cjson_crv == NULL || cjson_crv->type != cJSON_String) {
		PLATFORM_PRINTF_WARNING("DPP: No Attributes - crv\n");
		goto fail;
	}
	PLATFORM_PRINTF_DEBUG("DPP: crv = %s\n", cjson_crv->valuestring);

	cjson_x = cJSON_GetObjectItem(jwk, "x");
	if (cjson_x == NULL || cjson_x->type != cJSON_String) {
		PLATFORM_PRINTF_WARNING("DPP: No Attributes - x\n");
		goto fail;
	}
	PLATFORM_PRINTF_DEBUG("DPP: x = %s\n", cjson_x->valuestring);

	cjson_y = cJSON_GetObjectItem(jwk, "y");
	if (cjson_y == NULL || cjson_y->type != cJSON_String) {
		PLATFORM_PRINTF_WARNING("DPP: No Attributes - y\n");
		goto fail;
	}
	PLATFORM_PRINTF_DEBUG("DPP: y = %s\n", cjson_y->valuestring);

	cjson_kid = cJSON_GetObjectItem(jwk, "kid");
	if (cjson_kid && cjson_kid->type == cJSON_String) {
		PLATFORM_PRINTF_DEBUG("DPP: kid = %s\n", cjson_kid->valuestring);
	}

	// Only support "EC" (elliptic curve)
	if (PLATFORM_STRCMP(cjson_kty->valuestring, "EC") != 0) {
		PLATFORM_PRINTF_WARNING("DPP: does not support non ec key\n");
		goto fail;
	}

	// Find back curve parameters using the jwk_crv
	params = dpp_get_curve_name(cjson_crv->valuestring);
	if (!params) {
		PLATFORM_PRINTF_WARNING("DPP: does not support curve %s\n", cjson_crv->valuestring);
		goto fail;
	}

	// Decode coordinates from base64
	x = base64_url_decode((INT8U *)cjson_x->valuestring, PLATFORM_STRLEN(cjson_x->valuestring), &x_len);
	y = base64_url_decode((INT8U *)cjson_y->valuestring, PLATFORM_STRLEN(cjson_y->valuestring), &y_len);
	if (!x || !x_len || !y || !y_len) {
		PLATFORM_PRINTF_WARNING("DPP: base64 decode coordinates failed\n");
		goto fail;
	}

	// Combine coordinates into ANSI X9.63 format
	ansix963_len = 1 + x_len + y_len; // 0x04 + coordinates
	ansix963     = PLATFORM_MALLOC(ansix963_len);
	ansix963[0]  = 0x04;
	PLATFORM_MEMCPY(&ansix963[1], x, x_len);
	PLATFORM_MEMCPY(&ansix963[1 + x_len], y, y_len);

	// Parse key
	if (PLATFORM_PARSE_EC_PUBLIC_ANSIX963(params->name, ansix963, ansix963_len, pub_key_out) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot parse the key\n");
		goto fail;
	}

	// generate kid = SHA256(ANSI X9.63 uncompressed key)
	addr[0] = ansix963;
	len[0]  = ansix963_len;
	if (PLATFORM_SHA256(1, addr, len, kid_hash_out) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: kid generation failed!\n");
		goto fail;
	}

	// generate base64(kid)
	PLATFORM_FREE(*kid_out);
	*kid_out = (char *)base64_url_encode(kid_hash_out, SHA256_MAC_LEN, NULL);
	if (!*kid_out) {
		PLATFORM_PRINTF_WARNING("DPP: base64(kid) generation failed!\n");
		goto fail;
	}

	// verify generated kid with parsed kid
	if (cjson_kid && cjson_kid->type == cJSON_String) {
		if (PLATFORM_STRCMP(cjson_kid->valuestring, *kid_out) != 0) {
			PLATFORM_PRINTF_WARNING("DPP: kid mismatch!\n");
			PLATFORM_SAFE_FREE((void **)kid_out);
			goto fail;
		}
	}

	ret = 1;
fail:
	PLATFORM_FREE(x);
	PLATFORM_FREE(y);
	PLATFORM_FREE(ansix963);
	return ret;
}

static INT8U dpp_sign_connector(struct dpp_configurator *conf, struct dpp_connector *conn)
{
	const struct dpp_curve_params *netaccesskeycurve = NULL;

	cJSON *body       = NULL;
	char * header_buf = NULL, *body_buf = NULL, *b64conn = NULL;
	INT8U *sign_buf       = NULL;
	INT8U *b64_header_buf = NULL, *b64_body_buf = NULL, *b64_sign_buf = NULL;
	size_t b64_header_len = 0, b64_body_len = 0, b64_sign_len = 0;
	INT8U *addr[3];
	INT32U len[3];
	INT16U sign_size = 0;
	INT8U  ret       = 0, i;

	// Header:
	header_buf = dpp_build_jwk_prot_hdr(conf->csign_kid, conf->curve->jws_alg);
	if (!header_buf) {
		PLATFORM_PRINTF_WARNING("DPP: JWK header failed!\n");
		goto fail;
	}

	b64_header_buf = base64_url_encode((INT8U *)header_buf, strlen(header_buf), &b64_header_len);
	if (!b64_header_buf || b64_header_len <= 0) {
		PLATFORM_PRINTF_WARNING("DPP: base64_url(jwk_prot_hdr) failed!\n");
		goto fail;
	}

	// Body:
	body = cJSON_CreateObject();
	{
		// groups:
		cJSON *groups = cJSON_CreateArray();
		for (i = 0; i < conn->group_count; i++) {
			cJSON *group = cJSON_CreateObject();
			cJSON_AddStringToObject(group, "groupId", conn->groups[i].groupId);
			cJSON_AddStringToObject(group, "netRole", dpp_netrole_str(conn->groups[i].netRole));
			cJSON_AddItemToArray(groups, group);
		}
		cJSON_AddItemToObject(body, "groups", groups);

		// netAccessKey:
		netaccesskeycurve   = dpp_get_curve_name(conn->netaccesskey.curve);
		cJSON *netAccessKey = dpp_build_jwk(&conn->netaccesskey, NULL, netaccesskeycurve);
		cJSON_AddItemToObject(body, "netAccessKey", netAccessKey);
	}
	body_buf = cJSON_PrintUnformatted(body);

	b64_body_buf = base64_url_encode((INT8U *)body_buf, strlen(body_buf), &b64_body_len);
	if (!b64_body_buf || b64_body_len <= 0) {
		PLATFORM_PRINTF_WARNING("DPP: base64_url(jwk_body) failed!\n");
		goto fail;
	}

	// Signature ECDSA (r,s))
	addr[0]   = b64_header_buf;
	len[0]    = b64_header_len;
	addr[1]   = (INT8U *)".";
	len[1]    = 1;
	addr[2]   = b64_body_buf;
	len[2]    = b64_body_len;
	sign_size = conf->curve->prime_len * 2;
	sign_buf  = PLATFORM_MALLOC(sign_size);
	if (PLATFORM_ECDSA_SIGN(conf->curve->hash_len, &conf->csign, 3, addr, len, sign_buf, sign_size) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: ECDSA sign failed\n");
		goto fail;
	}

	b64_sign_buf = base64_url_encode(sign_buf, sign_size, &b64_sign_len);
	if (!b64_sign_buf || b64_body_len <= 0) {
		PLATFORM_PRINTF_WARNING("DPP: base64_url(sign) failed!\n");
		goto fail;
	}

	// DPP connector (base64(jwk_header)||.||base64(connector)||.||base64(signature))
	b64conn = PLATFORM_MALLOC(b64_header_len + 1 + b64_body_len + 1 + b64_sign_len + 1);
	PLATFORM_STRCPY(b64conn, (char *)b64_header_buf);
	PLATFORM_STRCAT(b64conn, ".");
	PLATFORM_STRCAT(b64conn, (char *)b64_body_buf);
	PLATFORM_STRCAT(b64conn, ".");
	PLATFORM_STRCAT(b64conn, (char *)b64_sign_buf);
	conn->signedConnector = (INT8U *)b64conn;
	ret                   = 1;

fail:
	PLATFORM_FREE(header_buf);
	PLATFORM_FREE(b64_header_buf);
	cJSON_Delete(body);
	PLATFORM_FREE(body_buf);
	PLATFORM_FREE(b64_body_buf);
	PLATFORM_FREE(sign_buf);
	PLATFORM_FREE(b64_sign_buf);
	return ret;
}

INT8U dpp_build_signed_connector(struct dpp_configurator *conf,
                                 ECCKeyPair *             netAccessKey,
                                 char *groupId, enum dpp_netrole netRole, struct dpp_connector *conn)
{
	INT8U *addr[1];
	INT32U len[1];

	// assume only one group
	strlcpy(conn->groups[0].groupId, groupId, sizeof(conn->groups[0].groupId)); // e.g., "mapNW"
	conn->groups[0].netRole = netRole;
	conn->group_count       = 1;

	// Use or generate netAccessKey
	if (netAccessKey) {
		PLATFORM_PRINTF_DEBUG("DPP: Using predefined netAccessKey\n");
		PLATFORM_MEMCPY(&conn->netaccesskey, netAccessKey, sizeof(conn->netaccesskey));
	} else {
		PLATFORM_PRINTF_DEBUG("DPP: Generating new netAccessKey\n");
		if (PLATFORM_GENERATE_EC_KEYPAIR(conf->curve->name, &conn->netaccesskey) != 1) {
			PLATFORM_PRINTF_WARNING("DPP: kid generation failed!\n");
			return 0;
		}
	}

	// generate kid = SHA256(ANSI X9.63 uncompressed netAccessKey)
	addr[0] = conn->netaccesskey.pub_ansix963;
	len[0]  = conn->netaccesskey.pub_ansix963_len;
	if (PLATFORM_SHA256(1, addr, len, conn->netaccesskey_kid_hash) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: kid generation failed!\n");
		return 0;
	}

	// generate base64(kid)
	PLATFORM_FREE(conn->netaccesskey_kid);
	conn->netaccesskey_kid = (char *)base64_url_encode(conn->netaccesskey_kid_hash, sizeof(conn->netaccesskey_kid_hash), NULL);
	if (!conn->netaccesskey_kid) {
		PLATFORM_PRINTF_WARNING("DPP: base64(kid) generation failed!\n");
		goto fail;
	}

	// sign the connector
	PLATFORM_FREE(conn->signedConnector);
	if (dpp_sign_connector(conf, conn) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot sign connector!\n");
		goto fail;
	}

	return 1;

fail:
	PLATFORM_SAFE_FREE((void **)&conn->netaccesskey_kid);
	PLATFORM_SAFE_FREE((void **)&conn->signedConnector);
	return 0;
}

INT8U dpp_parse_signed_connector(char *signed_conn, INT16U signed_conn_len, struct dpp_configurator *conf, struct dpp_connector *conn, char **out_group_id)
{
	char * b64_jwkheader, *b64_body, *b64_sign, *b64_kid = NULL, *alg = NULL;
	INT8U *jwkheader = NULL, *body = NULL, *sign = NULL;
	size_t b64_jwkheader_len, jwkheader_len, b64_body_len, jwk_len, b64_sign_len, sign_len;
	cJSON *cJSON_body = NULL, *token = NULL, *t;
	INT8U *addr[3];
	INT32U len[3];
	INT8U  ret = 0;

	// JSON Parsing
	//
	b64_jwkheader     = signed_conn;
	b64_jwkheader_len = strchr(b64_jwkheader, '.') - b64_jwkheader;
	jwkheader         = base64_url_decode((INT8U *)b64_jwkheader, b64_jwkheader_len, &jwkheader_len);
	PLATFORM_PRINTF_DEBUG("DPP: JWK Header = %s\n", jwkheader);

	b64_body     = b64_jwkheader + b64_jwkheader_len + 1;
	b64_body_len = strchr(b64_body, '.') - b64_body;
	body         = base64_url_decode((INT8U *)b64_body, b64_body_len, &jwk_len);
	PLATFORM_PRINTF_DEBUG("DPP: Body = %s\n", body);

	b64_sign     = b64_body + b64_body_len + 1;
	b64_sign_len = signed_conn_len - b64_jwkheader_len - b64_body_len - 2;
	sign         = base64_url_decode((INT8U *)b64_sign, b64_sign_len, &sign_len);

	// Verify signature
	//
	addr[0] = (INT8U *)b64_jwkheader;
	len[0]  = b64_jwkheader_len;
	addr[1] = (INT8U *)".";
	len[1]  = 1;
	addr[2] = (INT8U *)b64_body;
	len[2]  = b64_body_len;
	if (PLATFORM_ECDSA_VERIFY(conf->curve->hash_len, &conf->csign, 3, addr, len, sign, sign_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Signature is INVALID!\n");
		goto fail;
	}

	// Header
	if (dpp_parse_jwk_prot_hdr((char *)jwkheader, &b64_kid, &alg) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: JWK Protocol Header is INVALID!\n");
		goto fail;
	}

	// Body
	//
	cJSON_body = cJSON_Parse((char *)body);
	if (!cJSON_body) {
		PLATFORM_PRINTF_WARNING("DPP: DPP connector body is INVALID!\n");
		goto fail;
	}

	// Body - group
	token = cJSON_GetObjectItem(cJSON_body, "groups");
	if (!token || token->type != cJSON_Array) {
		PLATFORM_PRINTF_WARNING("DPP: No Config Attributes - groups\n");
		goto fail;
	}

	conn->group_count = 0;
	cJSON_ArrayForEach(t, token)
	{
		cJSON *groupId = cJSON_GetObjectItem(t, "groupId");
		cJSON *netRole = cJSON_GetObjectItem(t, "netRole");
		if (conn->group_count < DPP_MAX_CONF_OBJ_GROUPS
		    && groupId && groupId->type == cJSON_String
		    && netRole && netRole->type == cJSON_String) {
			PLATFORM_PRINTF_DEBUG("DPP: groupId: %s\n", groupId->valuestring);
			if (out_group_id) {
				*out_group_id = (char *)PLATFORM_MALLOC(strlen(groupId->valuestring) + 1);
				PLATFORM_MEMSET(*out_group_id, '\0', strlen(groupId->valuestring) + 1);
				PLATFORM_MEMCPY(*out_group_id, groupId->valuestring, strlen(groupId->valuestring));
			}
			PLATFORM_PRINTF_DEBUG("DPP: netRole: %s\n", netRole->valuestring);
			strlcpy(conn->groups[conn->group_count].groupId, groupId->valuestring, sizeof(conn->groups[0].groupId));
			conn->groups[conn->group_count].netRole = dpp_str_netrole(netRole->valuestring);
			conn->group_count++;
		}
	}

	// Body - netAccessKey
	token = cJSON_GetObjectItem(cJSON_body, "netAccessKey");
	if (!token || token->type != cJSON_Object) {
		PLATFORM_PRINTF_WARNING("DPP: No netAccessKey attribute!\n");
		goto fail;
	}

	if (dpp_parse_jwk(token,
	                  &conn->netaccesskey,
	                  conn->netaccesskey_kid_hash,
	                  &conn->netaccesskey_kid)
	    != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot parse netAccessKey!\n");
		goto fail;
	}

	if (PLATFORM_STRCMP(b64_kid, conf->csign_kid) != 0) {
		PLATFORM_PRINTF_WARNING("DPP: kid mismatch!\n");
		goto fail;
	}

	// copy signed connector string
	PLATFORM_FREE(conn->signedConnector);
	conn->signedConnector = PLATFORM_MALLOC(PLATFORM_STRLEN(signed_conn) + 1);
	PLATFORM_STRCPY((char *)conn->signedConnector, signed_conn);
	ret = 1;

fail:
	cJSON_Delete(cJSON_body);
	PLATFORM_FREE(jwkheader);
	PLATFORM_FREE(body);
	PLATFORM_FREE(sign);
	PLATFORM_FREE(b64_kid);
	PLATFORM_FREE(alg);
	return ret;
}

void dpp_free_connector(struct dpp_connector *conn)
{
	if (conn->netaccesskey_kid)
		PLATFORM_SAFE_FREE((void **)&conn->netaccesskey_kid);
	if (conn->signedConnector)
		PLATFORM_SAFE_FREE((void **)&conn->signedConnector);
}

static INT8U dpp_build_conf_obj(struct dpp_configurator *conf, struct dpp_config_obj *conf_obj, int is_bss_conf_obj, char **json_out)
{
	cJSON *obj = NULL, *discovery = NULL, *cred = NULL, *csign = NULL, *ppkey = NULL;
	INT8U  ret = 0;

	obj = cJSON_CreateObject();

	// Wi-Fi Technology Object
	cJSON_AddStringToObject(obj, "wi-fi_tech", conf_obj->wifi_tech);

	// Decryption Failure Counter Threshold (MAP only)
	if (strstr(conf_obj->wifi_tech, "map") && !is_bss_conf_obj)
		cJSON_AddNumberToObject(obj, "dfCounterThreshold", conf_obj->dfCounterThreshold);

	// Discovery Object
	discovery = cJSON_CreateObject();
	if (conf_obj->ssid64)
		cJSON_AddStringToObject(discovery, "ssid64", conf_obj->ssid64);
	if (conf_obj->ssid_charset)
		cJSON_AddNumberToObject(discovery, "ssid_charset", conf_obj->ssid_charset);
	if (conf_obj->ruid_enable)
		cJSON_AddStringToObject(discovery, "RUID", conf_obj->ruid);
	if (conf_obj->ssid_len)
		cJSON_AddStringToObject(discovery, "ssid", conf_obj->ssid);
	if (conf_obj->bssid_enable) {
		cJSON_AddStringToObject(discovery, "BSSID", conf_obj->bssid);
	} else {
		cJSON_AddStringToObject(discovery, "BSSID", "000000000000");
	}
	cJSON_AddItemToObject(obj, "discovery", discovery);

	// Credential Object (only exists if akm exists)
	// In the case of BSS TEARDOWN, akm does not exist
	if (strstr(conf_obj->akm, "psk") || strstr(conf_obj->akm, "sae") || strstr(conf_obj->akm, "dpp")) {
		cred = cJSON_CreateObject();
		cJSON_AddStringToObject(cred, "akm", conf_obj->akm);
		if (strstr(conf_obj->akm, "psk"))
			cJSON_AddStringToObject(cred, "psk_hex", conf_obj->psk_hex);
		if (strstr(conf_obj->akm, "sae"))
			cJSON_AddStringToObject(cred, "pass", conf_obj->pass);
		// (80211) signed conn, csign, ppkey will be sent regardless of which akm is used
		// signed connector
		if (!conf_obj->conn.signedConnector) {
			PLATFORM_PRINTF_WARNING("DPP: signedConnector is null\n");
			cJSON_Delete(cred);
			goto fail;
		}
		cJSON_AddStringToObject(cred, "signedConnector", (char *)conf_obj->conn.signedConnector);
		// c-sign key
		csign = dpp_build_jwk(&conf->csign, conf->csign_kid, conf->curve);
		if (!csign) {
			PLATFORM_PRINTF_WARNING("DPP: cannot generate signed connector\n");
			cJSON_Delete(cred);
			goto fail;
		}
		cJSON_AddItemToObject(cred, "csign", csign);
		// ppKey
		if (!is_bss_conf_obj) {
			ppkey = dpp_build_jwk(&conf->ppkey, NULL, conf->curve);
			if (!ppkey) {
				PLATFORM_PRINTF_WARNING("DPP: cannot generate ppkey jwk\n");
				cJSON_Delete(cred);
				goto fail;
			}
			cJSON_AddItemToObject(cred, "ppKey", ppkey);
		}
		cJSON_AddItemToObject(obj, "cred", cred);
	}

	// Generate JSON string
	*json_out = cJSON_PrintUnformatted(obj);
	if (!*json_out) {
		PLATFORM_PRINTF_WARNING("DPP: cannot generate json!\n");
		goto fail;
	}

	ret = 1;
fail:
	cJSON_Delete(obj);
	return ret;
}

static INT8U dpp_parse_conf_obj(char *json, int is_bss_conf_obj, struct dpp_configurator *config_out, struct dpp_config_obj *obj_out, char **out_group_id)
{
	cJSON *conf_obj = NULL, *discovery = NULL, *cred = NULL, *token = NULL;
	INT8U  ret = 0;

	conf_obj = cJSON_Parse((char *)json);
	if (!conf_obj) {
		PLATFORM_PRINTF_WARNING("DPP: Could not parse Config Object\n");
		goto fail;
	}

	// Wi-Fi Technology Object
	token = cJSON_GetObjectItem(conf_obj, "wi-fi_tech");
	if (!token || token->type != cJSON_String) {
		PLATFORM_PRINTF_WARNING("DPP: No Attributes - wi-fi_tech\n");
		goto fail;
	}
	strlcpy(obj_out->wifi_tech, token->valuestring, sizeof(obj_out->wifi_tech));

	// Decryption Failure Counter Threshold (MAP only)
	if (strstr(obj_out->wifi_tech, "map") && !is_bss_conf_obj) {
		token = cJSON_GetObjectItem(conf_obj, "dfCounterThreshold");
		if (!token || token->type != cJSON_Number) {
			PLATFORM_PRINTF_WARNING("DPP: No Attributes - dfCounterThreshold\n");
			goto fail;
		}
		obj_out->dfCounterThreshold = (INT16U)token->valueint;
	}

	// Discovery Object
	//
	discovery = cJSON_GetObjectItem(conf_obj, "discovery");
	if (!discovery || discovery->type != cJSON_Object) {
		PLATFORM_PRINTF_WARNING("DPP: No Attributes - discovery\n");
	} else {
		token = cJSON_GetObjectItem(discovery, "ssid");
		if (token && token->type == cJSON_String) {
			strlcpy(obj_out->ssid, token->valuestring, sizeof(obj_out->ssid));
			obj_out->ssid_len = PLATFORM_STRLEN(obj_out->ssid);
		}
		token = cJSON_GetObjectItem(discovery, "ssid64");
		if (token && token->type == cJSON_String) {
			PLATFORM_FREE(obj_out->ssid64);
			obj_out->ssid64 = PLATFORM_MALLOC(PLATFORM_STRLEN(token->valuestring) + 1);
			PLATFORM_STRCPY(obj_out->ssid64, token->valuestring);
		}
		token = cJSON_GetObjectItem(discovery, "ssid_charset");
		if (token && token->type == cJSON_Number)
			obj_out->ssid_charset = token->valueint;
		token = cJSON_GetObjectItem(discovery, "RUID");
		if (token && token->type == cJSON_String) {
			strlcpy(obj_out->ruid, token->valuestring, sizeof(obj_out->ruid));
			obj_out->ruid_enable = 1;
		}
		token = cJSON_GetObjectItem(discovery, "BSSID");
		if (token && token->type == cJSON_String) {
			strlcpy(obj_out->bssid, token->valuestring, sizeof(obj_out->bssid));
			obj_out->bssid_enable = 1;
		}
	}

	// Credential Object
	//
	cred = cJSON_GetObjectItem(conf_obj, "cred");
	if (!cred || cred->type != cJSON_Object) {
		PLATFORM_PRINTF_WARNING("DPP: No Attributes - cred\n");
	} else {
		token = cJSON_GetObjectItem(cred, "akm");
		if (!token || token->type != cJSON_String) {
			PLATFORM_PRINTF_WARNING("DPP: No Attributes - akm\n");
			goto fail;
		}
		strlcpy(obj_out->akm, token->valuestring, sizeof(obj_out->akm));
		if (strstr(obj_out->akm, "psk")) {
			if ((token = cJSON_GetObjectItem(cred, "psk_hex")) != NULL)
				strlcpy(obj_out->psk_hex, token->valuestring, sizeof(obj_out->psk_hex));
		}
		if (strstr(obj_out->akm, "sae")) {
			if ((token = cJSON_GetObjectItem(cred, "pass")) != NULL)
				strlcpy(obj_out->pass, token->valuestring, sizeof(obj_out->pass));
		}
		if (strstr(obj_out->akm, "dpp")) {
			// assign csign to configurator
			token = cJSON_GetObjectItem(cred, "csign");
			PLATFORM_PRINTF_DEBUG("DPP: csign\n");
			if (!token
				|| dpp_parse_jwk(token,
								&config_out->csign,
								config_out->csign_kid_hash,
								&config_out->csign_kid)
					!= 1) {
				PLATFORM_PRINTF_WARNING("DPP: csign key parsing failed\n");
				goto fail;
			}
			// assign ppkey to configurator
			token = cJSON_GetObjectItem(cred, "ppKey");
			if (token) {
				PLATFORM_PRINTF_DEBUG("DPP: ppkey\n");
				if (dpp_parse_jwk(token,
				                  &config_out->ppkey,
				                  config_out->ppkey_kid_hash,
				                  &config_out->ppkey_kid)
				    != 1) {
					PLATFORM_PRINTF_WARNING("DPP: ppkey parsing failed\n");
				}
			}
			// assign curve to configurator
			config_out->curve = dpp_get_curve_name(config_out->csign.curve);

			// parse connector
			token = cJSON_GetObjectItem(cred, "signedConnector");
			if (!token
				|| dpp_parse_signed_connector(token->valuestring,
											strlen(token->valuestring),
											config_out,
											&obj_out->conn, out_group_id)
					!= 1) {
				PLATFORM_PRINTF_WARNING("DPP: signedConnector parsing failed\n");
				goto fail;
			}
		}
	}

	ret = 1;
fail:
	cJSON_Delete(conf_obj);
	return ret;
}

static void dpp_replace_netaccesskey(struct dpp_authentication *auth, struct dpp_config_obj *obj)
{
	if (strstr(obj->akm, "dpp")) {
		if (PLATFORM_MEMCMP(obj->conn.netaccesskey.pub_ansix963,
		                    auth->own_protocol_key.pub_ansix963,
		                    obj->conn.netaccesskey.pub_ansix963_len)
		    == 0) {
			PLATFORM_PRINTF_DEBUG("DPP: Found own protocol key in dpp_config_obj\n");
			PLATFORM_MEMCPY(&obj->conn.netaccesskey, &auth->own_protocol_key, sizeof(ECCKeyPair));
		}
	}
}

void dpp_free_conf_obj(struct dpp_config_obj *obj)
{
	if (obj->ssid64)
		PLATFORM_SAFE_FREE((void **)&obj->ssid64);
	dpp_free_connector(&obj->conn);
}

// DPP Configuration Response Builder
INT8U dpp_build_conf_resp(struct dpp_authentication *auth,
                          struct dpp_config_obj obj[], INT8U obj_count,
                          INT8U **conf_resp_msg, INT32U *conf_resp_msg_len,
                          INT8U dialog_token)
{
	enum dpp_status_error status;

	INT32U nonce_len, clear_len;
	INT16U attr_len, attr_out_len;
	INT8U *clear = NULL, *msg = NULL;
	char * obj_json[obj_count];
	INT8U  ret = 0;
	int    i;

	INT8U *p;
	INT16U aux16;
	INT8U  aux8;

	const INT8U *addr[1];
	INT32U       len[1];

	PLATFORM_PRINTF_INFO("DPP: Build Configuration Response\n");

	nonce_len = auth->curve->nonce_len;
	status    = DPP_STATUS_OK;

	/* Build configuration objects */
	PLATFORM_MEMSET(obj_json, 0, sizeof(obj_json));
	for (i = 0; i < obj_count; i++) {
		if (dpp_build_conf_obj(auth->conf, &obj[i], 0, &obj_json[i]) != 1) {
			PLATFORM_PRINTF_WARNING("DPP: cannot build config obj json!\n");
			goto fail;
		}
	}

	/* Build { E-nonce, configObject }ke */
	clear_len = 4 + nonce_len;
	for (i = 0; i < obj_count; i++)
		clear_len += 4 + PLATFORM_STRLEN(obj_json[i]);

	attr_len = 4 + 1;                           // DPP Status
	attr_len += 4 + clear_len + AES_BLOCK_SIZE; // Wrapped Data

	/* Allocate message buffer */
	clear = PLATFORM_MALLOC(clear_len);
	msg   = dpp_gas_build_resp(WLAN_PA_GAS_INITIAL_RESP, dialog_token, WLAN_STATUS_SUCCESS, 0, 0, 0, attr_len, &attr_out_len);
	if (!msg) {
		PLATFORM_PRINTF_WARNING("DPP: build_req failed!\n");
		goto fail;
	}

	/* Clear e-nonce */
	p     = clear;
	aux16 = DPP_ATTR_ENROLLEE_NONCE;
	_I2B_LE(&aux16, &p);
	aux16 = nonce_len;
	_I2B_LE(&aux16, &p);
	_InB(auth->e_nonce, &p, nonce_len);

	/* Clear config objects */
	for (i = 0; i < obj_count; i++) {
		aux16 = DPP_ATTR_CONFIG_OBJ;
		_I2B_LE(&aux16, &p);
		aux16 = (INT16U)PLATFORM_STRLEN(obj_json[i]);
		_I2B_LE(&aux16, &p);
		_InB(obj_json[i], &p, aux16);
	}

	/* DPP Status */
	p     = msg + (attr_out_len - attr_len);
	aux16 = (INT16U)DPP_ATTR_STATUS;
	_I2B_LE(&aux16, &p);
	aux16 = 1;
	_I2B_LE(&aux16, &p);
	aux8 = (INT8U)status;
	_I1B(&aux8, &p);

	addr[0] = msg + (attr_out_len - attr_len); // Start of DPP status attribute
	len[0]  = 5;                               // Length of DPP status attribute

	/* Wrapped E-nonce and config objects */
	aux16 = DPP_ATTR_WRAPPED_DATA;
	_I2B_LE(&aux16, &p);
	aux16 = clear_len + AES_BLOCK_SIZE;
	_I2B_LE(&aux16, &p);

	if (PLATFORM_AES_SIV_ENCRYPT(auth->ke, auth->curve->hash_len, clear, clear_len, 1, addr, len, p) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot encrypt with ke\n");
		goto fail;
	}
	p += (clear_len + AES_BLOCK_SIZE);

	*conf_resp_msg     = msg;
	*conf_resp_msg_len = attr_out_len;
	PLATFORM_PRINTF_INFO("DPP: Configuration Response Generated\n");
	ret                = 1;

fail:
	if (!ret)
		PLATFORM_FREE(msg);
	for (i = 0; i < obj_count; i++)
		PLATFORM_FREE(obj_json[i]);
	PLATFORM_FREE(clear);
	return ret;
}

// DPP Configuration Response Parser
INT8U dpp_conf_resp_rx(struct dpp_authentication *auth,
                       struct dpp_config_obj obj_out[], INT8U *obj_count_out, INT8U max_obj_count,
                       INT8U *buf, INT32U len, char **out_group_id)
{
	INT8U *attr_start = buf + 19;
	INT32U attr_len   = len - 19;

	INT8U *wrapped_data, *e_nonce, *status, *unwrapped = NULL, *conf_obj = NULL;
	INT16U wrapped_data_len = 0, e_nonce_len = 0, status_len = 0, conf_obj_len = 0;
	int    unwrapped_len = 0;
	INT8U  obj_count = 0, ret = 0, i;

	const INT8U *addr[1];
	INT32U       addr_len[1];

	if (!auth->auth_success) {
		PLATFORM_PRINTF_WARNING("DPP: auth_success is false\n");
		return 0;
	}

	if (dpp_check_attrs(attr_start, attr_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Invalid attribute in config resposne\n");
		return 0;
	}

	// Get DPP Status
	status = dpp_get_attr(attr_start, attr_len, DPP_ATTR_STATUS, &status_len);
	if (!status || status_len < 1) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required DPP Status attribute\n");
		return 0;
	}

	if (status[0] != DPP_STATUS_OK) {
		PLATFORM_PRINTF_WARNING("DPP: status is not OK\n");
		return 0;
	}

	// Get { E-nonce, configAttrib }ke
	wrapped_data = dpp_get_attr(attr_start, attr_len, DPP_ATTR_WRAPPED_DATA, &wrapped_data_len);
	if (!wrapped_data || wrapped_data_len < AES_BLOCK_SIZE) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required Wrapped Data attribute\n");
		return 0;
	}

	addr[0]     = status - 4;
	addr_len[0] = 4 + status_len;

	/* Decrypt wrapped data */
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped     = PLATFORM_MALLOC(unwrapped_len);
	if (PLATFORM_AES_SIV_DECRYPT(auth->ke, auth->curve->hash_len,
	                             wrapped_data, wrapped_data_len, 1, addr, addr_len, unwrapped)
	    != 1) {
		PLATFORM_PRINTF_WARNING("DPP: AES-SIV decryption failed\n");
		goto fail;
	}

	// Check decrypted content format
	if (dpp_check_attrs(unwrapped, unwrapped_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Invalid attribute in unwrapped data\n");
		goto fail;
	}

	// Get E-nonce
	e_nonce = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_ENROLLEE_NONCE, &e_nonce_len);
	if (!e_nonce || e_nonce_len != auth->curve->nonce_len) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid Enrollee Nonce attribute\n");
		goto fail;
	}
	if (PLATFORM_MEMCMP(auth->e_nonce, e_nonce, e_nonce_len) != 0) {
		PLATFORM_PRINTF_WARNING("DPP: Enrollee Nonce mismatch\n");
		goto fail;
	}

	// Get config objects
	do {
		conf_obj = dpp_get_attr_next(conf_obj, unwrapped, unwrapped_len, DPP_ATTR_CONFIG_OBJ, &conf_obj_len);
		if (conf_obj) {
			if (dpp_parse_conf_obj((char *)conf_obj, 0, auth->conf, &obj_out[obj_count], out_group_id) != 1) {
				PLATFORM_PRINTF_WARNING("DPP: cannot parse conf obj\n");
				for (i = 0; i < obj_count; i++)
					dpp_free_conf_obj(&obj_out[i]);
				goto fail;
			}
			// Replace the received public key with own private protocol key if matched
			dpp_replace_netaccesskey(auth, &obj_out[obj_count]);
			obj_count++;
			PLATFORM_PRINTF_INFO("DPP: config object #%d parsed!\n", obj_count);
		}
	} while (conf_obj && obj_count < max_obj_count);

	// at least one config obj is required
	if (!obj_count) {
		PLATFORM_PRINTF_WARNING("DPP: Missing required config object attribute\n");
		goto fail;
	}

	*obj_count_out = obj_count;
	ret            = 1;
	PLATFORM_PRINTF_INFO("DPP: Configuration Response Rx finished...\n");

fail:
	PLATFORM_FREE(unwrapped);
	return ret;
}

// DPP Configuration Result Builder
INT8U dpp_build_conf_result(struct dpp_authentication *auth, enum dpp_status_error status, INT8U **conf_result_msg, INT32U *conf_result_msg_len)
{
	INT32U       nonce_len, clear_len, attr_len, attr_out_len;
	INT8U *      clear = NULL, *msg = NULL, *p;
	const INT8U *addr[2];
	INT32U       len[2];
	INT16U       aux16;
	INT8U        aux8, ret = 0;

	PLATFORM_PRINTF_INFO("DPP: Build Configuration Result\n");

	nonce_len = auth->curve->nonce_len;

	/* Build { DPP Status, E-nonce }ke */
	clear_len = 4 + 1; // DPP Status
	clear_len += 4 + nonce_len;
	attr_len = 4 + clear_len + AES_BLOCK_SIZE;

	/* Allocate message buffer */
	clear = PLATFORM_MALLOC(clear_len);
	msg   = dpp_alloc_msg(DPP_PA_CONFIGURATION_RESULT, attr_len, &attr_out_len);
	if (!msg) {
		PLATFORM_PRINTF_WARNING("DPP: build_req failed!\n");
		goto fail;
	}

	/* DPP Status */
	p     = clear;
	aux16 = (INT16U)DPP_ATTR_STATUS;
	_I2B_LE(&aux16, &p);
	aux16 = 1;
	_I2B_LE(&aux16, &p);
	aux8 = (INT8U)status;
	_I1B(&aux8, &p);

	/* E-nonce */
	aux16 = DPP_ATTR_ENROLLEE_NONCE;
	_I2B_LE(&aux16, &p);
	aux16 = nonce_len;
	_I2B_LE(&aux16, &p);
	_InB(auth->e_nonce, &p, nonce_len);

	/* Associated data: OUI, OUI type, Crypto Suite, DPP frame type */
	addr[0] = msg + 2;
	len[0]  = 3 + 1 + 1 + 1;

	/* Attributes before Wrapped Data (None) */
	addr[1] = msg + 2 + 3 + 1 + 1 + 1;
	len[1]  = 0;

	/* Wrapped DPP Status and E-nonce */
	p     = msg + (attr_out_len - attr_len);
	aux16 = (INT16U)DPP_ATTR_WRAPPED_DATA;
	_I2B_LE(&aux16, &p);
	aux16 = clear_len + AES_BLOCK_SIZE;
	_I2B_LE(&aux16, &p);
	if (PLATFORM_AES_SIV_ENCRYPT(auth->ke, auth->curve->hash_len, clear, clear_len, 2, addr, len, p) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot encrypt with ke\n");
		goto fail;
	}
	p += (clear_len + AES_BLOCK_SIZE);

	*conf_result_msg     = msg;
	*conf_result_msg_len = attr_out_len;
	PLATFORM_PRINTF_INFO("DPP: Configuration Result Generated\n");
	ret                  = 1;

fail:
	PLATFORM_FREE(clear);
	if (!ret)
		PLATFORM_FREE(msg);
	return ret;
}

// DPP Configuration Result Parser
INT8U dpp_conf_result_rx(struct dpp_authentication *auth, INT8U *buf, INT32U len)
{
	INT8U *attr_start = buf + 8;
	INT32U attr_len   = len - 8;

	const INT8U *wrapped_data, *e_nonce, *status;
	INT16U       wrapped_data_len = 0, e_nonce_len = 0, status_len = 0;
	int          unwrapped_len = 0;
	INT8U *      unwrapped     = 0;
	INT8U        ret           = 0;

	const INT8U *addr[2];
	INT32U       alen[2];

	wrapped_data = dpp_get_attr(attr_start, attr_len, DPP_ATTR_WRAPPED_DATA, &wrapped_data_len);
	if (!wrapped_data || wrapped_data_len < AES_BLOCK_SIZE) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required Wrapped Data attribute\n");
		return 0;
	}

	/* Associated data: OUI, OUI type, Crypto Suite, DPP frame type */
	addr[0] = buf + 2;
	alen[0] = 3 + 1 + 1 + 1;

	/* Attributes before Wrapped Data */
	addr[1]       = attr_start;
	alen[1]       = 0;
	unwrapped_len = wrapped_data_len - AES_BLOCK_SIZE;
	unwrapped     = PLATFORM_MALLOC(unwrapped_len);

	/* Decrypt wrapped data */
	if (PLATFORM_AES_SIV_DECRYPT(auth->ke, auth->curve->hash_len,
	                             wrapped_data, wrapped_data_len, 2, addr, alen, unwrapped)
	    != 1) {
		PLATFORM_PRINTF_WARNING("DPP: AES-SIV decryption failed\n");
		goto fail;
	}

	// Check decrypted content format
	if (dpp_check_attrs(unwrapped, unwrapped_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Invalid attribute in unwrapped data\n");
		goto fail;
	}

	// Get DPP Status
	status = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_STATUS, &status_len);
	if (!status || status_len < 1) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required DPP Status attribute\n");
		goto fail;
	}
	if (status[0] != DPP_STATUS_OK) {
		PLATFORM_PRINTF_WARNING("DPP: status is not OK\n");
		goto fail;
	}

	// Get E-Nonce
	e_nonce = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_ENROLLEE_NONCE, &e_nonce_len);
	if (!e_nonce || e_nonce_len != auth->curve->nonce_len) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid Enrollee Nonce attribute\n");
		goto fail;
	}
	if (PLATFORM_MEMCMP(auth->e_nonce, e_nonce, e_nonce_len) != 0) {
		PLATFORM_PRINTF_WARNING("DPP: Enrollee Nonce mismatch\n");
		goto fail;
	}

	ret = 1;
	PLATFORM_PRINTF_INFO("DPP: Configuration Result Rx finished...\n");

fail:
	PLATFORM_FREE(unwrapped);
	return ret;
}

// Self generate and sign configuration object
INT8U dpp_configurator_sign(struct dpp_authentication *auth,
                            char *wifi_tech, char *groupId, enum dpp_netrole netRole)
{
	struct dpp_config_obj *obj = NULL;

	PLATFORM_PRINTF_DEBUG("DPP: configurator self-sign\n");

	if (auth->conf_obj_count >= DPP_MAX_CONF_OBJ) {
		PLATFORM_PRINTF_WARNING("DPP: not enough space to sign\n");
		return 0;
	}

	/* Assign content and sign the object */
	obj = &(auth->conf_obj[auth->conf_obj_count]);
	PLATFORM_MEMSET(obj, 0, sizeof(struct dpp_config_obj));
	if (PLATFORM_STRLEN(wifi_tech) >= sizeof(obj->wifi_tech)) {
		PLATFORM_PRINTF_WARNING("DPP: not enough space for wifi_tech \n");
		return 0;
	}
	PLATFORM_STRCPY(obj->wifi_tech, wifi_tech);
	PLATFORM_STRCPY(obj->akm, "dpp");
	if (dpp_build_signed_connector(auth->conf, NULL,
	                               groupId, netRole, &obj->conn)
	    != 1) {
		PLATFORM_PRINTF_WARNING("DPP: connector self-signing failed\n");
		dpp_free_conf_obj(obj);
		return 0;
	}
	/* Update status */
	auth->conf_obj_count++;
	return 1;
}

struct dpp_config_obj *dpp_find_config_obj(struct dpp_authentication *auth,
                                           char *wifi_tech, char *groupId, enum dpp_netrole netRole)
{
	int i, j;

	for (i = 0; i < auth->conf_obj_count; i++) {
		if (strstr(auth->conf_obj[i].wifi_tech, wifi_tech)) {
			struct dpp_connector *conn = &(auth->conf_obj[i].conn);
			for (j = 0; j < conn->group_count; j++) {
				if (strcmp(groupId, "*") == 0) {
					if (conn->groups[j].netRole == netRole){
						return &auth->conf_obj[i];
					}
				}
				if (strstr(conn->groups[j].groupId, groupId)
				    && conn->groups[j].netRole == netRole) {
					return &auth->conf_obj[i];
				}
			}
		}
	}

	return NULL;
}

// DPP Network Introduction (Peer Discovery) Request Builder
INT8U dpp_build_peer_disc_req(struct dpp_connector *conn, INT8U **peer_disc_msg, INT32U *peer_disc_msg_len)
{
	INT32U len, out_len, conn_len;
	INT8U *msg = NULL, *p;
	INT16U aux16;
	INT8U  aux8;

	PLATFORM_PRINTF_INFO("DPP: Build Peer Disc Request\n");
	if (!conn || !conn->signedConnector) {
		PLATFORM_PRINTF_WARNING("DPP: invalid connector\n");
		return 0;
	}

	conn_len = PLATFORM_STRLEN((char *)conn->signedConnector);

	/* Build (Transaction ID, Connector, Version) */
	len = 4 + 1;         // Transaction ID
	len += 4 + conn_len; // DPP Connector
	len += 4 + 1;        // DPP Version

	/* Allocate message buffer */
	msg = dpp_alloc_msg(DPP_PA_PEER_DISCOVERY_REQ, len, &out_len);
	if (!msg) {
		PLATFORM_PRINTF_WARNING("DPP: build_req failed!\n");
		goto fail;
	}

	/* Transaction ID */
	p     = msg + (out_len - len);
	aux16 = (INT16U)DPP_ATTR_TRANSACTION_ID;
	_I2B_LE(&aux16, &p);
	aux16 = 1;
	_I2B_LE(&aux16, &p);
	aux8 = TRANSACTION_ID;
	_I1B(&aux8, &p);

	/* DPP Connector */
	aux16 = (INT16U)DPP_ATTR_CONNECTOR;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)conn_len;
	_I2B_LE(&aux16, &p);
	_InB(conn->signedConnector, &p, aux16);

	/* Protocol Version */
	aux16 = (INT16U)DPP_ATTR_PROTOCOL_VERSION;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)1;
	_I2B_LE(&aux16, &p);
	aux8 = DPP_VERSION;
	_I1B(&aux8, &p);

	*peer_disc_msg     = msg;
	*peer_disc_msg_len = out_len;
	PLATFORM_PRINTF_INFO("DPP: Peer Disc Request Generated\n");
	return 1;

fail:
	PLATFORM_FREE(msg);
	return 0;
}

// DPP Network Introduction (Peer Discovery) Request Parser
INT8U dpp_rx_peer_disc_req(struct dpp_configurator *conf, struct dpp_connector *conn_out, INT8U *buf, INT32U len)
{
	INT8U *conn_str, *trans_id;
	INT16U conn_str_len, trans_id_len;
	INT8U *attr_start = buf + 8;
	INT32U attr_len   = len - 8;

	// Transaction ID
	trans_id = dpp_get_attr(attr_start, attr_len, DPP_ATTR_TRANSACTION_ID, &trans_id_len);
	if (!trans_id || trans_id_len != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Peer did not include Transaction ID\n");
		return 0;
	}

	// DPP Connector
	conn_str = dpp_get_attr(attr_start, attr_len, DPP_ATTR_CONNECTOR, &conn_str_len);
	if (!conn_str) {
		PLATFORM_PRINTF_WARNING("DPP: Peer did not include its Connector\n");
		return 0;
	}
	if (dpp_parse_signed_connector((char *)conn_str, conn_str_len, conf, conn_out, NULL) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Invalid Connector\n");
		dpp_free_connector(conn_out);
		return 0;
	}

	PLATFORM_PRINTF_INFO("DPP: Peer Disc Request Rx finished...\n");
	return 1;
}

INT8U dpp_peer_intro(struct dpp_connector *own, struct dpp_connector *peer, struct dpp_introduction *intro)
{
	INT8U matched = 0;
	int   i, j;

	INT8U  Nx[MAX_SHARED_SECRET_LEN];
	size_t Nx_len;

	const struct dpp_curve_params *curve, *own_curve;

	/* Check groups */
	for (i = 0; i < own->group_count; i++) {
		for (j = 0; i < peer->group_count; j++) {
			if (strcmp(own->groups->groupId, peer->groups->groupId) == 0) {
				matched = 1;
				break;
			}
		}
		if (matched)
			break;
	}
	if (!matched) {
		PLATFORM_PRINTF_WARNING("DPP: does not have the same group!\n");
		return 0;
	}

	/* Check netRoles */
	if (0 == dpp_netrole_is_compatible(own->groups[i].netRole, peer->groups[j].netRole)) {
		PLATFORM_PRINTF_WARNING("DPP: netRole is not compatible\n");
		return 0;
	}

	/* Check Curve */
	curve     = dpp_get_curve_name(peer->netaccesskey.curve);
	own_curve = dpp_get_curve_name(own->netaccesskey.curve);
	if (curve != own_curve) {
		PLATFORM_PRINTF_WARNING("DPP: Mismatching netAccessKey curves (%s != %s)\n",
		                        own_curve->name, curve->name);
		return 0;
	}

	/* ECDH: N = nk * PK */
	if (PLATFORM_COMPUTE_ECDH_SHARED_SECRET(&own->netaccesskey, &peer->netaccesskey, Nx, &Nx_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot calculate ecdh\n");
		return 0;
	}

	/* PMK = HKDF(<>, "DPP PMK", N.x) */
	if (dpp_derive_pmk(Nx, Nx_len, intro->pmk, curve->hash_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Failed to derive PMK\n");
		return 0;
	}
	intro->pmk_len = curve->hash_len;

	/* PMKID = Truncate-128(H(min(NK.x, PK.x) | max(NK.x, PK.x))) */
	if (dpp_derive_pmkid(&own->netaccesskey, &peer->netaccesskey, intro->pmkid) < 0) {
		PLATFORM_PRINTF_WARNING("DPP: Failed to derive PMKID\n");
		return 0;
	}

	return 1;
}

// DPP Network Introduction (Peer Discovery) Response Builder
INT8U dpp_build_peer_disc_resp(struct dpp_connector *conn,
                               enum dpp_status_error status,
                               INT8U **peer_resp_msg, INT32U *peer_resp_msg_len)
{
	INT32U len, out_len, conn_len;
	INT8U *msg = NULL, *p;
	INT16U aux16;
	INT8U  aux8;

	PLATFORM_PRINTF_INFO("DPP: Build Peer Disc Response\n");
	if (!conn || !conn->signedConnector) {
		PLATFORM_PRINTF_WARNING("DPP: invalid connector\n");
		return 0;
	}

	conn_len = PLATFORM_STRLEN((char *)conn->signedConnector);

	/* Build (Transaction ID, Connector, Version) */
	len = 4 + 1;         // DPP Status
	len += 4 + 1;        // Transaction ID
	len += 4 + conn_len; // DPP Connector
	len += 4 + 1;        // DPP Version

	/* Allocate message buffer */
	msg = dpp_alloc_msg(DPP_PA_PEER_DISCOVERY_RESP, len, &out_len);
	if (!msg) {
		PLATFORM_PRINTF_WARNING("DPP: build_req failed!\n");
		goto fail;
	}

	/* Transaction ID */
	p     = msg + (out_len - len);
	aux16 = (INT16U)DPP_ATTR_TRANSACTION_ID;
	_I2B_LE(&aux16, &p);
	aux16 = 1;
	_I2B_LE(&aux16, &p);
	aux8 = TRANSACTION_ID;
	_I1B(&aux8, &p);

	/* DPP Status */
	aux16 = (INT16U)DPP_ATTR_STATUS;
	_I2B_LE(&aux16, &p);
	aux16 = 1;
	_I2B_LE(&aux16, &p);
	aux8 = (INT8U)status;
	_I1B(&aux8, &p);

	/* DPP Connector */
	aux16 = (INT16U)DPP_ATTR_CONNECTOR;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)conn_len;
	_I2B_LE(&aux16, &p);
	_InB(conn->signedConnector, &p, aux16);

	/* Protocol Version */
	aux16 = (INT16U)DPP_ATTR_PROTOCOL_VERSION;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)1;
	_I2B_LE(&aux16, &p);
	aux8 = DPP_VERSION;
	_I1B(&aux8, &p);

	*peer_resp_msg     = msg;
	*peer_resp_msg_len = out_len;
	PLATFORM_PRINTF_INFO("DPP: Peer Disc Response Generated\n");
	return 1;

fail:
	PLATFORM_FREE(msg);
	return 0;
}

// DPP Network Introduction (Peer Discovery) Response Parser
INT8U dpp_rx_peer_disc_resp(struct dpp_configurator *conf, struct dpp_connector *conn_out, INT8U *buf, INT32U len, char **out_group_id)
{
	INT8U *conn_str, *trans_id, *status;
	INT16U conn_str_len, trans_id_len, status_len;
	INT8U *attr_start = buf + 8;
	INT32U attr_len   = len - 8;

	// Transaction ID
	trans_id = dpp_get_attr(attr_start, attr_len, DPP_ATTR_TRANSACTION_ID, &trans_id_len);
	if (!trans_id || trans_id_len != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Peer did not include Transaction ID\n");
		return 0;
	}
	if (trans_id[0] != TRANSACTION_ID) {
		PLATFORM_PRINTF_WARNING("DPP: Ignore frame with unexpected Transaction ID %d\n", trans_id[0]);
		return 0;
	}

	// DPP Status
	status = dpp_get_attr(attr_start, attr_len, DPP_ATTR_STATUS, &status_len);
	if (!status || status_len != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Peer did not include Status\n");
		return 0;
	}
	if (status[0] != DPP_STATUS_OK) {
		PLATFORM_PRINTF_WARNING("DPP: Peer rejected network introduction: Status %d\n", status[0]);
		return 0;
	}

	// DPP Connector
	conn_str = dpp_get_attr(attr_start, attr_len, DPP_ATTR_CONNECTOR, &conn_str_len);
	if (!conn_str) {
		PLATFORM_PRINTF_WARNING("DPP: Peer did not include its Connector\n");
		return 0;
	}
	if (dpp_parse_signed_connector((char *)conn_str, conn_str_len, conf, conn_out, out_group_id) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Invalid Connector\n");
		dpp_free_connector(conn_out);
		return 0;
	}

	PLATFORM_PRINTF_INFO("DPP: Peer Disc Response Rx finished...\n");
	return 1;
}

// [Multi-AP] BSS Configuration Request Builder
INT8U dpp_build_map_bss_conf_req(struct dpp_config_req_obj *obj,
                                 INT8U **bss_conf_req_msg, INT16U *bss_conf_req_msg_len)
{
	char * configReqBuf = NULL;
	INT16U json_len, msg_len = 0;
	INT8U *msg = NULL;

	INT8U *p;
	INT16U aux16;
	INT8U  ret = 0;

	PLATFORM_PRINTF_INFO("DPP: Build Multi-AP BSS Configuration Request\n");

	/* configAttrib: the configuration request object */
	if (dpp_build_conf_req_obj(obj, &configReqBuf) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: conf req object is problematic\n");
		goto fail;
	}
	json_len = (INT16U)PLATFORM_STRLEN(configReqBuf);

	/* Build DPP Configuration Request Attributes */
	msg_len += 4 + json_len;
	p = msg = PLATFORM_MALLOC(msg_len);

	/* configAttrib */
	aux16 = DPP_ATTR_CONFIG_ATTR_OBJ;
	_I2B_LE(&aux16, &p);
	aux16 = json_len;
	_I2B_LE(&aux16, &p);
	_InB(configReqBuf, &p, json_len);

	*bss_conf_req_msg     = msg;
	*bss_conf_req_msg_len = msg_len;
	PLATFORM_PRINTF_INFO("DPP: Multi-AP BSS Configuration Request Generated\n");
	ret                   = 1;

fail:
	if (!ret)
		PLATFORM_FREE(msg);
	PLATFORM_FREE(configReqBuf);
	return ret;
}

// [Multi-AP] BSS Configuration Request Parser
INT8U dpp_map_bss_conf_req_rx(struct dpp_config_req_obj *obj_out, INT8U *buf, INT16U len)
{
	const INT8U *config_attr;
	INT16U       config_attr_len = 0;

	if (*buf == 0x7b) { // 0x7b is ASCII for '{'
		if (dpp_parse_conf_req_obj((char *)buf, obj_out) != 1) {
			PLATFORM_PRINTF_WARNING("DPP: Cannot parse configAttrib\n");
			return 0;
		}
	} else {
		if (dpp_check_attrs(buf, len) != 1) {
			PLATFORM_PRINTF_WARNING("DPP: Invalid attribute in config request\n");
			return 0;
		}

		// Get the first configAttrib
		config_attr = dpp_get_attr(buf, len, DPP_ATTR_CONFIG_ATTR_OBJ, &config_attr_len);
		if (!config_attr) {
			PLATFORM_PRINTF_WARNING("DPP: Missing or invalid Config Attributes attribute\n");
			return 0;
		}

		if (dpp_parse_conf_req_obj((char *)config_attr, obj_out) != 1) {
			PLATFORM_PRINTF_WARNING("DPP: Cannot parse configAttrib\n");
			return 0;
		}
	}

	PLATFORM_PRINTF_INFO("DPP: Multi-AP BSS Configuration Request Rx finished...\n");
	return 1;
}

// [Multi-AP] BSS Configuration Response Builder
INT8U dpp_build_map_bss_conf_resp(struct dpp_authentication *auth,
                                  struct dpp_config_obj obj[], INT8U obj_count,
                                  INT8U **bss_conf_resp_msg, INT16U *bss_conf_resp_msg_len)
{
	INT16U msg_len = 0;
	INT8U *msg     = NULL;
	char * obj_json[obj_count];
	INT8U  ret = 0;
	int    i;

	INT8U *p;
	INT16U aux16;

	PLATFORM_PRINTF_INFO("DPP: Build Multi-AP BSS Configuration Response\n");

	/* Build configuration objects */
	PLATFORM_MEMSET(obj_json, 0, sizeof(obj_json));
	for (i = 0; i < obj_count; i++) {
		if (dpp_build_conf_obj(auth->conf, &obj[i], 1, &obj_json[i]) != 1) {
			PLATFORM_PRINTF_WARNING("DPP: cannot build config obj json!\n");
			goto fail;
		}
		msg_len += 4 + PLATFORM_STRLEN(obj_json[i]);
	}

	/* Allocate message buffer */
	p = msg = PLATFORM_MALLOC(msg_len);

	/* config objects */
	for (i = 0; i < obj_count; i++) {
		aux16 = DPP_ATTR_CONFIG_OBJ;
		_I2B_LE(&aux16, &p);
		aux16 = (INT16U)PLATFORM_STRLEN(obj_json[i]);
		_I2B_LE(&aux16, &p);
		_InB(obj_json[i], &p, aux16);
	}

	*bss_conf_resp_msg     = msg;
	*bss_conf_resp_msg_len = msg_len;
	PLATFORM_PRINTF_INFO("DPP: Multi-AP BSS Configuration Response Generated\n");
	ret                    = 1;

fail:
	if (!ret)
		PLATFORM_FREE(msg);
	for (i = 0; i < obj_count; i++)
		PLATFORM_FREE(obj_json[i]);
	return ret;
}

// [Multi-AP] BSS Configuration Response Parser
INT8U dpp_map_bss_conf_resp_rx(struct dpp_config_obj obj_out[], INT8U *obj_count_out,
                               INT8U max_obj_count, INT8U *buf, INT16U len)
{
	INT8U *conf_obj     = NULL;
	INT16U conf_obj_len = 0;
	INT8U  obj_count    = 0, i;

	// Use dummy config because we ignore received connectors for now
	struct dpp_configurator dummy_conf;
	PLATFORM_MEMSET(&dummy_conf, 0, sizeof(dummy_conf));

	if (dpp_check_attrs(buf, len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Invalid attribute in bss config resposne\n");
		return 0;
	}

	// Get config objects
	do {
		conf_obj = dpp_get_attr_next(conf_obj, buf, len, DPP_ATTR_CONFIG_OBJ, &conf_obj_len);
		if (conf_obj) {
			if (dpp_parse_conf_obj((char *)conf_obj, 1, &dummy_conf, &obj_out[obj_count], NULL) != 1) {
				PLATFORM_PRINTF_WARNING("DPP: cannot parse bss dpp conf obj\n");
				for (i = 0; i < obj_count; i++)
					dpp_free_conf_obj(&obj_out[i]);
				dpp_free_configurator(&dummy_conf);
				return 0;
			}
			obj_count++;
			PLATFORM_PRINTF_INFO("DPP: bss dpp config object #%d parsed!\n", obj_count);
		}
	} while (conf_obj && obj_count < max_obj_count);

	// at least one config obj is required
	if (!obj_count) {
		PLATFORM_PRINTF_WARNING("DPP: Missing required bss dpp config object attr\n");
		return 0;
	}

	*obj_count_out = obj_count;
	dpp_free_configurator(&dummy_conf);
	PLATFORM_PRINTF_INFO("DPP: Multi-AP BSS Configuration Response Rx finished...\n");
	return 1;
}

INT8U dpp_load_creds_info(struct dpp_authentication *dpp_auth, char *csign_key_str, char *pp_key_str, char *netaccess_key_str, char *signed_connector)
{
	INT8U                    ret, *csign_key_hex, *pp_key_hex, *netaccess_key_hex, *addr;
	INT16U                   csign_len, pp_len, netaccess_len;
	INT32U                   len;
	ECCKeyPair               csign_pub = { 0 }, pp_pub = { 0 };
	struct dpp_configurator *dummy_conf;
	struct dpp_config_obj   *dummy_conf_obj;

	ret = 0;

	// sanity check
	if (NULL == csign_key_str) {
		PLATFORM_PRINTF_ERROR("Invalid DPP credentials C-sign key.");
		return ret;
	} else {
		PLATFORM_PRINTF_DEBUG("DPP C-sign key: %s\n", csign_key_str);
	}
	if (NULL == pp_key_str) {
		PLATFORM_PRINTF_ERROR("Invalid DPP credentials Private Protection key.");
		return ret;
	} else {
		PLATFORM_PRINTF_INFO("DPP Private Protection key: %s\n", pp_key_str);
	}
	if (NULL == netaccess_key_str) {
		PLATFORM_PRINTF_ERROR("Invalid DPP credentials Netaccess key.");
		return ret;
	} else {
		PLATFORM_PRINTF_DEBUG("DPP Netaccess key: %s\n", netaccess_key_str);
	}
	if (NULL == signed_connector) {
		PLATFORM_PRINTF_ERROR("Invalid DPP Signed connector.");
		return ret;
	} else {
		PLATFORM_PRINTF_DEBUG("DPP Signed connector: %s\n", signed_connector);
	}
	if (NULL != dpp_find_config_obj(dpp_auth, "map", "*", DPP_NETROLE_MAP_AGENT)) {
		PLATFORM_PRINTF_ERROR("DPP credentials already exist.");
		return ret;
	}

	// parse public C-sign key and pp key and private netaccess key in datamodel
	csign_len         = strlen(csign_key_str) / 2;
	pp_len            = strlen(pp_key_str) / 2;
	netaccess_len     = strlen(netaccess_key_str) / 2;
	csign_key_hex     = (INT8U *)PLATFORM_MALLOC(csign_len);
	pp_key_hex        = (INT8U *)PLATFORM_MALLOC(pp_len);
	netaccess_key_hex = (INT8U *)PLATFORM_MALLOC(netaccess_len);
	hexstr2bin(csign_key_str, (char *)csign_key_hex, csign_len);
	hexstr2bin(pp_key_str, (char *)pp_key_hex, pp_len);
	hexstr2bin(netaccess_key_str, (char *)netaccess_key_hex, netaccess_len);
	if (0 == PLATFORM_PARSE_EC_PUBLIC_KEY(csign_key_hex, csign_len, &csign_pub)) {
		PLATFORM_PRINTF_ERROR("Fail to parse DPP C-sign key, drop loading.");
		goto exit_fail_1;
	}
	if (0 == PLATFORM_PARSE_EC_PUBLIC_KEY(pp_key_hex, pp_len, &pp_pub)) {
		PLATFORM_PRINTF_ERROR("Fail to parse DPP Private Protection key, drop loading.");
		goto exit_fail_1;
	}
	if (0 == PLATFORM_PARSE_EC_PRIVATE_KEY(netaccess_key_hex, netaccess_len, &(dpp_auth->own_protocol_key))) {
		PLATFORM_PRINTF_ERROR("Fail to parse Netaccess key, drop loading.");
		goto exit_fail_1;
	}

	// make dummy configurator
	dummy_conf = (struct dpp_configurator *)PLATFORM_MALLOC(sizeof(struct dpp_configurator));
	PLATFORM_MEMSET(dummy_conf, 0, sizeof(struct dpp_configurator));
	// C-Sign Key
	dummy_conf->csign = csign_pub;
	addr              = dummy_conf->csign.pub_ansix963;
	len               = dummy_conf->csign.pub_ansix963_len;
	PLATFORM_SHA256(1, &addr, &len, dummy_conf->csign_kid_hash);
	dummy_conf->csign_kid = (char *)base64_url_encode(dummy_conf->csign_kid_hash, sizeof(dummy_conf->csign_kid_hash), NULL);
	// Private Protection Key
	dummy_conf->ppkey = pp_pub;
	addr              = dummy_conf->ppkey.pub_ansix963;
	len               = dummy_conf->ppkey.pub_ansix963_len;
	PLATFORM_SHA256(1, &addr, &len, dummy_conf->ppkey_kid_hash);
	dummy_conf->ppkey_kid = (char *)base64_url_encode(dummy_conf->ppkey_kid_hash, sizeof(dummy_conf->ppkey_kid_hash), NULL);
	// Curve
	dummy_conf->curve = dpp_get_curve_name(dummy_conf->csign.curve);

	// make dummy config_obj;
	dummy_conf_obj = (struct dpp_config_obj *)PLATFORM_MALLOC(sizeof(struct dpp_config_obj));
	PLATFORM_MEMSET(dummy_conf_obj, 0, sizeof(struct dpp_config_obj));
	PLATFORM_STRCPY(dummy_conf_obj->wifi_tech, "map");
	PLATFORM_STRCPY(dummy_conf_obj->akm, "dpp");
	// fill in public C-sign key (csign_key) and Private Protection key (ppkey) and signed connector (conn);
	dummy_conf_obj->csign_pub = csign_pub;
	dummy_conf_obj->pp_pub    = pp_pub;
	if (0 == dpp_parse_signed_connector(signed_connector, strlen(signed_connector), dummy_conf, &(dummy_conf_obj->conn), NULL)) {
		PLATFORM_PRINTF_ERROR("Fail to parse DPP Signed connector, drop loading.");
		goto exit_fail_2;
	}
	// replace conn->netaccesskey with the private one
	dpp_replace_netaccesskey(dpp_auth, dummy_conf_obj);

	// save dpp credentials into dpp_auth via dummy config_obj
	dpp_auth->conf        = dummy_conf;
	dpp_auth->conf_obj[0] = *dummy_conf_obj;
	dpp_auth->conf_obj_count++;
	ret = 1;
	goto exit_pass;

exit_fail_2:
	PLATFORM_FREE(dummy_conf);
exit_pass:
	PLATFORM_FREE(dummy_conf_obj);
exit_fail_1:
	PLATFORM_FREE(netaccess_key_hex);
	PLATFORM_FREE(csign_key_hex);
	PLATFORM_FREE(pp_key_hex);
	return ret;
}