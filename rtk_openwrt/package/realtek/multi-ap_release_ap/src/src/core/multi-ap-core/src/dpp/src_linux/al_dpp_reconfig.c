/*
 *  Multi-AP Device Provisioning Protocol (DPP)
 *  DPP Reconfiguration
 *
 *  Copyright (C) 2022 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 */

#include "platform.h"
#include "packet_tools.h"

#include "platform_crypto.h"
#include "platform_crypto_aes.h"

#include "al_dpp.h"

#include "cJSON.h"

static ECCKeyPair *dpp_set_keypair(const struct dpp_curve_params **curve,
                                   unsigned char *priv, int priv_len)
{
	INT16U      group;
	ECCKeyPair *key = NULL;

	if (0 == PLATFORM_PARSE_EC_PRIVATE_KEY(priv, priv_len, key)) {
		PLATFORM_PRINTF_WARNING("DPP: Failed to parse private key\n");
		return NULL;
	}

	group = PLATFORM_EC_KEY_GET_GROUP(key);
	if (group == 0xFF) {
		return NULL;
	}

	*curve = dpp_get_curve_ike_group(group);
	if (!*curve) {
		PLATFORM_PRINTF_WARNING("DPP: Unsupported curve (group=%u) in pre-assigned key\n", group);
		return NULL;
	}

	return key;
}
static INT8U dpp_reconfig_derive_ke_responder(struct dpp_authentication *auth,
                                              unsigned char *net_access_key, int net_access_key_len,
                                              cJSON *peer_net_access_key)
{
	ECCKeyPair *own_key = NULL, *peer_key = NULL, *sum = NULL;
	INT8U       Mx[DPP_MAX_SHARED_SECRET_LEN];
	size_t      Mx_len;
	INT8U       prk[DPP_MAX_HASH_LEN];
	INT8U       nonces[2 * DPP_MAX_NONCE_LEN];
	INT8U       res = 0;

	const struct dpp_curve_params *curve;
	cJSON                         *cjson_crv = NULL;

	own_key = dpp_set_keypair(&auth->curve, net_access_key,
	                          net_access_key_len);
	if (!own_key) {
		PLATFORM_PRINTF_WARNING("DPP: Failed to parse own netAccessKey\n");
		goto fail;
	}

	if (dpp_parse_jwk(peer_net_access_key, peer_key, NULL, NULL) != 1)
		goto fail;

	// No need for fail checking here because done inside dpp_parse_jwk()
	cjson_crv = cJSON_GetObjectItem(peer_net_access_key, "crv");
	curve     = dpp_get_curve_name(cjson_crv->valuestring);

	if (auth->curve != curve) {
		PLATFORM_PRINTF_WARNING("DPP: Mismatching netAccessKey curves (own=%s != peer=%s)\n", auth->curve->name, curve->name);
		goto fail;
	}

	if (PLATFORM_GENERATE_EC_KEYPAIR(curve->name, &auth->own_protocol_key) != 1)
		goto fail;

	if (PLATFORM_GET_RANDOM_BYTES(auth->e_nonce, auth->curve->nonce_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot generate E-nonce\n");
		goto fail;
	}

	/* ECDH: M = { cR + pR } * CI */
	if (PLATFORM_EC_KEY_ADD_PUBLIC_KEY(own_key, &auth->own_protocol_key, curve->name, sum) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Error during public key addition\n");
		goto fail;
	}
	if (PLATFORM_COMPUTE_ECDH_SHARED_SECRET(sum, peer_key, Mx, &Mx_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot calculate ecdh\n");
		return 0;
	}

	/* ke = HKDF(C-nonce | E-nonce, "dpp reconfig key", M.x) */

	/* HKDF-Extract(C-nonce | E-nonce, M.x) */
	PLATFORM_MEMCPY(nonces, auth->c_nonce, curve->nonce_len);
	PLATFORM_MEMCPY(&nonces[curve->nonce_len], auth->e_nonce, curve->nonce_len);
	if (dpp_hmac(curve->hash_len, nonces, 2 * curve->nonce_len, Mx, curve->prime_len, prk) != 1)
		goto fail;

	/* HKDF-Expand(PRK, "dpp reconfig key", L) */
	if (dpp_hkdf_expand(curve->hash_len, prk, curve->hash_len, "dpp reconfig key", auth->ke, curve->hash_len) != 1)
		goto fail;

	res = 1;
	PLATFORM_FREE(&auth->reconfig_old_protocol_key);
	auth->reconfig_old_protocol_key = *own_key;
	own_key                         = NULL;

fail:
	PLATFORM_FREE(own_key);
	PLATFORM_FREE(peer_key);
	return res;
}

static INT8U dpp_reconfig_derive_ke_initiator(struct dpp_authentication *auth,
                                              INT8U *r_proto, INT16U r_proto_len,
                                              ECCKeyPair *net_access_key)
{
	ECCKeyPair *peer_key = NULL;
	ECCKeyPair *PR       = &(auth->peer_protocol_key);
	ECCKeyPair  sum      = { 0 };

	INT8U  Mx[DPP_MAX_SHARED_SECRET_LEN];
	size_t Mx_len;

	INT8U prk[DPP_MAX_HASH_LEN];
	INT8U nonces[2 * DPP_MAX_NONCE_LEN];

	const struct dpp_curve_params *curve;

	int    hash_len, nonce_len;
	INT16U r_proto_xy_len;
	INT8U  res = 0;

	if (PLATFORM_STRCMP(auth->curve->name, net_access_key->curve) != 0) {
		PLATFORM_PRINTF_WARNING("DPP: Mismatching netAccessKey curves (own=%s != peer=%s)\n", auth->curve->name, net_access_key->curve);
		goto fail;
	}

	curve     = dpp_get_curve_name(net_access_key->curve);
	hash_len  = curve->hash_len;
	nonce_len = curve->nonce_len;

	r_proto_xy_len = r_proto_len / 2;

	if (PLATFORM_EC_KEY_SET_PUBLIC_KEY(curve->ike_group, r_proto, r_proto + r_proto_xy_len, r_proto_xy_len, PR) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Could not set Responder Protocol Key\n");
		return 0;
	}

	/* ECDH: M = cI * { CR + PR } */
	if (PLATFORM_EC_KEY_ADD_PUBLIC_KEY(net_access_key, &auth->peer_protocol_key, curve->name, &sum) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Error during public key addition\n");
		goto fail;
	}
	if (PLATFORM_COMPUTE_ECDH_SHARED_SECRET(&auth->conf->csign, &sum, Mx, &Mx_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot calculate ecdh\n");
		return 0;
	}

	/* ke = HKDF(C-nonce | E-nonce, "dpp reconfig key", M.x) */

	/* HKDF-Extract(C-nonce | E-nonce, M.x) */
	PLATFORM_MEMCPY(nonces, auth->c_nonce, nonce_len);
	PLATFORM_MEMCPY(&nonces[nonce_len], auth->e_nonce, nonce_len);
	if (dpp_hmac(hash_len, nonces, 2 * nonce_len, Mx, curve->prime_len, prk) != 1)
		goto fail;

	/* HKDF-Expand(PRK, "dpp reconfig key", L) */
	if (dpp_hkdf_expand(hash_len, prk, hash_len, "dpp reconfig key", auth->ke, hash_len) != 1)
		goto fail;

	res = 1;

fail:
	PLATFORM_FREE(peer_key);
	return res;
}

INT8U dpp_build_conn_status(enum dpp_status_error result,
                            const char *ssid, INT32U ssid_len,
                            const char *channel_list,
                            char      **json_out)
{
	cJSON *obj = NULL;
	INT8U  ret = 0;

	obj = cJSON_CreateObject();

	cJSON_AddNumberToObject(obj, "result", result);
	if (ssid_len)
		cJSON_AddStringToObject(obj, "ssid", ssid);
	if (channel_list)
		cJSON_AddStringToObject(obj, "channelList", channel_list);

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

INT8U dpp_reconfig_init(struct dpp_authentication *auth, INT8U *a_nonce_attr, INT16U a_nonce_len, INT8U *e_id_attr, INT16U e_id_len)
{
	ECCKeyPair                     a_nonce = { 0 }, e_prime_id = { 0 };
	INT16U                         a_nonce_xy_len, e_id_xy_len;
	const struct dpp_curve_params *curve;

	INT8U ret = 0;

	if (auth == NULL)
		goto fail;
	curve = dpp_get_curve_name(auth->conf->csign.curve);

	if (!a_nonce_attr || a_nonce_len == 0) {
		PLATFORM_PRINTF_WARNING("DPP: Missing required A-NONCE attribute\n");
		goto fail;
	}

	a_nonce_xy_len = a_nonce_len / 2;
	if (PLATFORM_EC_KEY_SET_PUBLIC_KEY(curve->ike_group, a_nonce_attr, a_nonce_attr + a_nonce_xy_len, a_nonce_xy_len, &a_nonce) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Invalid A-NONCE\n");
		goto fail;
	}

	if (!e_id_attr || e_id_len == 0) {
		PLATFORM_PRINTF_WARNING("DPP: Missing required E'-id attribute\n");
		goto fail;
	}

	e_id_xy_len = e_id_len / 2;
	if (PLATFORM_EC_KEY_SET_PUBLIC_KEY(curve->ike_group, e_id_attr, e_id_attr + e_id_xy_len, e_id_xy_len, &e_prime_id) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Invalid E'-id\n");
		goto fail;
	}

	if (PLATFORM_DECRYPT_E_ID(&auth->conf->ppkey, &a_nonce, &e_prime_id) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Could not decrypt E'-id\n");
		goto fail;
	}

	auth->reconfig          = 1;
	auth->initiator         = 1;
	auth->waiting_auth_resp = 1;
	auth->allowed_roles     = DPP_CAPAB_CONFIGURATOR;
	auth->configurator      = 1;

	if (PLATFORM_GET_RANDOM_BYTES(auth->c_nonce, auth->curve->nonce_len) == 0) {
		PLATFORM_PRINTF_WARNING("DPP: Failed to generate C-nonce\n");
		goto fail;
	}

	ret = 1;

fail:
	return ret;
}

// DPP Reconfiguration Announcement Builder
INT8U dpp_build_reconfig_announcement(unsigned char *csign_key, int csign_key_len,
                                      unsigned char *net_access_key, int net_access_key_len,
                                      DPPReconfigID *id,
                                      INT8U **reconfig_announcement_msg, INT16U *reconfig_announcement_msg_len)
{

	INT8U *addr[1], *msg, *p;
	INT8U *a_nonce = NULL, *e_prime_id = NULL;
	INT8U  ret = 0;
	INT8U  hash[SHA256_MAC_LEN], uncomp[MAX_ANSI_X963_LEN];
	INT32S uncomp_len;
	INT32U attr_len, out_len, len[1];

	ECCKeyPair                    *csign = NULL, *own_key = NULL;
	const struct dpp_curve_params *own_curve;

	PLATFORM_PRINTF_INFO("DPP: Build Reconfiguration Announcement\n");

	own_key = dpp_set_keypair(&own_curve, net_access_key, net_access_key_len);
	if (!own_key) {
		PLATFORM_PRINTF_WARNING("DPP: Failed to parse own netAccessKey\n");
		goto fail;
	}

	if (PLATFORM_PARSE_EC_PUBLIC_KEY(csign_key, csign_key_len, csign) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Failed to parse local C-sign-key information\n");
		goto fail;
	}

	if (PLATFORM_EC_KEY_GET_PUBLIC_POINT(csign, uncomp, &uncomp_len) != 1)
		goto fail;
	PLATFORM_FREE(csign);
	addr[0] = uncomp;
	len[0]  = uncomp_len;
	PLATFORM_HEXDUMP("DPP: Uncompressed C-sign key", addr[0], len[0]);

	if (PLATFORM_SHA256(1, addr, len, hash) != 1) {
		PLATFORM_FREE(uncomp);
		goto fail;
	}
	PLATFORM_FREE(uncomp);
	PLATFORM_HEXDUMP("DPP: kid = SHA256(uncompressed C-sign key)", hash, SHA256_MAC_LEN);

	if (PLATFORM_UPDATE_RECONFIG_ID(id) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Failed to generate E'-id\n");
		goto fail;
	}

	/* Build DPP Reconfiguration Announcement frame attributes, P131 of Easy Connect Spec v2. */
	attr_len = 4 + SHA256_MAC_LEN;                    // SHA256(csign)
	attr_len += 4 + 2;                                // Finite Cyclic Group
	attr_len += 4 + id->a_nonce->pub_ansix963_len;    // A-NONCE
	attr_len += 2 + id->e_prime_id->pub_ansix963_len; // E'-id

	/* Allocate message buffer */
	msg = dpp_alloc_msg(DPP_PA_RECONFIG_ANNOUNCEMENT, attr_len, &out_len);
	if (!msg) {
		PLATFORM_PRINTF_WARNING("DPP: alloc_msg failed!\n");
		goto fail;
	}
	p = msg + (out_len - attr_len);

	INT16U aux16;

	/* Configurator C-sign Key Hash */
	aux16 = (INT16U)DPP_ATTR_C_SIGN_KEY_HASH;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)SHA256_MAC_LEN;
	_I2B_LE(&aux16, &p);
	_InB(&hash, &p, SHA256_MAC_LEN);

	/* Finite Cyclic Group */
	aux16 = (INT16U)DPP_ATTR_FINITE_CYCLIC_GROUP;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)2;
	_I2B_LE(&aux16, &p);
	aux16 = own_curve->ike_group;
	_I2B_LE(&aux16, &p);

	/* A-NONCE */
	aux16 = (INT16U)DPP_ATTR_A_NONCE;
	_I2B_LE(&aux16, &p);
	aux16 = id->a_nonce->pub_ansix963_len;
	_I2B_LE(&aux16, &p);
	_InB(&id->a_nonce->pub_ansix963, &p, id->a_nonce->pub_ansix963_len);

	/* E'-id */
	aux16 = (INT16U)DPP_ATTR_E_PRIME_ID;
	_I2B_LE(&aux16, &p);
	aux16 = id->e_prime_id->pub_ansix963_len;
	_I2B_LE(&aux16, &p);
	_InB(&id->e_prime_id->pub_ansix963, &p, id->e_prime_id->pub_ansix963_len);

	*reconfig_announcement_msg     = msg;
	*reconfig_announcement_msg_len = out_len;

	PLATFORM_PRINTF_INFO("DPP: Reconfiguration Announcement Generated\n");
	ret = 1;

fail:
	PLATFORM_FREE(a_nonce);
	PLATFORM_FREE(e_prime_id);
	PLATFORM_FREE(own_key);
	return ret;
}

// DPP Reconfiguration Announcement Parser
INT8U dpp_rx_reconfig_announcement(struct dpp_authentication *auth, INT8U *buf, INT32U len)
{
	INT16U csign_hash_len, ike_group_len, a_nonce_len, e_id_len;
	INT8U *csign_hash, *ike_group, *a_nonce, *e_id;

	INT8U *attr_start = buf + 8;
	INT32U attr_len   = len - 8;

	INT8U ret = 0;

	/* Get C-Sign Key Hash */
	csign_hash = dpp_get_attr(attr_start, attr_len, DPP_ATTR_C_SIGN_KEY_HASH, &csign_hash_len);
	if (!csign_hash || csign_hash_len != SHA256_MAC_LEN) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required C-sign Key Hash attribute\n");
		goto fail;
	}

	if (PLATFORM_MEMCMP(auth->conf->csign_kid_hash, csign_hash, csign_hash_len) != 0){
		PLATFORM_PRINTF_WARNING("DPP: Configurator info does not match\n");
		goto fail;
	}

	/* Get Finite Cyclic Group */
	ike_group = dpp_get_attr(attr_start, attr_len, DPP_ATTR_FINITE_CYCLIC_GROUP, &ike_group_len);
	if (!ike_group || ike_group_len != 2) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required Finite Cyclic Group attribute\n");
		goto fail;
	}
	if ((auth->curve = dpp_get_curve_ike_group(ike_group[0])) == NULL) {
		PLATFORM_PRINTF_WARNING("DPP: Unable to find curve based on group\n");
		goto fail;
	}

	/* Get A-Nonce */
	a_nonce = dpp_get_attr(attr_start, attr_len, DPP_ATTR_A_NONCE, &a_nonce_len);
	if (!a_nonce) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required A-Nonce attribute\n");
		goto fail;
	}

	/* Get E'-id */
	e_id = dpp_get_attr(attr_start, attr_len, DPP_ATTR_E_PRIME_ID, &e_id_len);
	if (!e_id) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required E'-id attribute\n");
		goto fail;
	}

	if (dpp_reconfig_init(auth, a_nonce, a_nonce_len, e_id, e_id_len) == 0) {
		PLATFORM_PRINTF_WARNING("DPP: Unable to initialize reconfiguration\n");
		goto fail;
	}

	ret = 1;

fail:
	return ret;
}

// DPP Reconfiguration Authentication Request Builder
INT8U dpp_build_reconfig_auth_req(struct dpp_authentication *auth,
                                  INT8U **reconfig_auth_req_msg, INT32U *reconfig_auth_req_msg_len)
{
	INT8U *msg, *p;
	INT8U  ret = 0;
	INT16U conn_len;
	INT32U attr_len, out_len;

	PLATFORM_PRINTF_INFO("DPP: Build Reconfiguration Authentication Request\n");

	/* Sign connector */
	struct dpp_connector conn;
	conn.netaccesskey_kid = NULL;
	conn.signedConnector  = NULL;
	if (dpp_build_signed_connector(auth->conf, &auth->conf->csign, "*", DPP_NETROLE_CONFIGURATOR, &conn) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: signing failed\n");
		goto fail;
	}

	conn_len = PLATFORM_STRLEN((char *)conn.signedConnector);

	/* Build DPP Reconfiguration Authentication Request frame attributes, P131 of Easy Connect Spec v2. */
	attr_len = 4 + 1;                       // Transaction ID
	attr_len += 4 + 1;                      // Protocol Version
	attr_len += 4 + conn_len;               // Connector
	attr_len += 4 + auth->curve->nonce_len; // Configurator Nonce

	/* Allocate message buffer */
	msg = dpp_alloc_msg(DPP_PA_RECONFIG_AUTH_REQ, attr_len, &out_len);
	if (!msg) {
		PLATFORM_PRINTF_WARNING("DPP: alloc_msg failed!\n");
		goto fail;
	}
	p = msg + (out_len - attr_len);

	INT8U  aux8;
	INT16U aux16;

	/* Transaction ID */
	aux16 = (INT16U)DPP_ATTR_TRANSACTION_ID;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)1;
	_I2B_LE(&aux16, &p);
	aux8 = TRANSACTION_ID;
	_I1B(&aux8, &p);

	/* Protocol Version */
	aux16 = (INT16U)DPP_ATTR_PROTOCOL_VERSION;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)1;
	_I2B_LE(&aux16, &p);
	aux8 = DPP_VERSION;
	_I1B(&aux8, &p);

	/* Connector */
	aux16 = (INT16U)DPP_ATTR_CONNECTOR;
	_I2B_LE(&aux16, &p);
	aux16 = conn_len;
	_I2B_LE(&aux16, &p);
	_InB(conn.signedConnector, &p, conn_len);

	/* C-Nonce */
	aux16 = (INT16U)DPP_ATTR_CONFIGURATOR_NONCE;
	_I2B_LE(&aux16, &p);
	aux16 = auth->curve->nonce_len;
	_I2B_LE(&aux16, &p);
	_InB(auth->c_nonce, &p, auth->curve->nonce_len);

	*reconfig_auth_req_msg     = msg;
	*reconfig_auth_req_msg_len = out_len;

	PLATFORM_PRINTF_INFO("DPP: Reconfiguration Authentication Request Generated\n");
	ret = 1;

fail:
	return ret;
}

// DPP Reconfiguration Authentication Request Parser
struct dpp_authentication *dpp_rx_reconfig_auth_req(struct dpp_configurator *conf,
                                                    struct dpp_connector    *own_conn,
                                                    INT8U *net_access_key, INT32U net_access_key_len,
                                                    INT8U *buf, INT32U len)
{
	INT8U *trans_id, *version, *i_connector, *c_nonce;
	INT16U trans_id_len = 0, version_len = 0, i_connector_len = 0, c_nonce_len = 0;

	INT8U *attr_start = buf + 8;
	INT32U attr_len   = len - 8;

	struct dpp_authentication *auth   = NULL;
	struct dpp_connector      *i_conn = NULL;

	cJSON *cJSON_body = NULL, *token = NULL;
	char  *conn_status = NULL;
	INT8U  i, j, matched = 0;

	/* Get Transaction ID */
	trans_id = dpp_get_attr(attr_start, attr_len, DPP_ATTR_TRANSACTION_ID, &trans_id_len);
	if (!trans_id || trans_id_len != 1 || trans_id[0] != TRANSACTION_ID) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required Transaction ID attribute\n");
		return NULL;
	}

	/* Get Protocol Version */
	version = dpp_get_attr(attr_start, attr_len, DPP_ATTR_PROTOCOL_VERSION, &version_len);
	if (!version || version_len < 1 || version[0] < DPP_VERSION) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required Protocol Version attribute\n");
		return NULL;
	}

	/* Get I-Connector */
	i_connector = dpp_get_attr(attr_start, attr_len, DPP_ATTR_CONNECTOR, &i_connector_len);
	if (!i_connector) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required I-Connector attribute\n");
		return NULL;
	}

	/* Process signed connector */
	if (dpp_parse_signed_connector((char *)i_connector, i_connector_len, conf, i_conn, NULL) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Invalid Connector\n");
		return NULL;
	}

	/* Get C-Nonce */
	c_nonce = dpp_get_attr(attr_start, attr_len, DPP_ATTR_CONFIGURATOR_NONCE, &c_nonce_len);
	if (!c_nonce) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required C-Nonce attribute\n");
		return NULL;
	}

	for (i = 0; i < own_conn->group_count; i++) {
		if (PLATFORM_STRLEN(own_conn->groups[i].groupId) == 0)
			continue;
		for (j = 0; j < i_conn->group_count; j++) {
			if (PLATFORM_STRCMP(own_conn->groups[i].groupId, i_conn->groups[j].groupId) == 0) {
				matched = 1;
				break;
			}
		}
	}
	if (!matched) {
		PLATFORM_PRINTF_WARNING("DPP:I-Connector does not include compatible group netrole with own connector\n");
		return NULL;
	}

	cJSON_body = cJSON_Parse((char *)i_connector);

	// TODO: Check expiry

	token = cJSON_GetObjectItem(cJSON_body, "netAccessKey");
	if (!token || token->type != cJSON_Object) {
		PLATFORM_PRINTF_WARNING("DPP: No netAccessKey attribute!\n");
		goto fail;
	}

	auth = PLATFORM_MALLOC(sizeof(*auth));

	auth->reconfig      = 1;
	auth->allowed_roles = DPP_CAPAB_ENROLLEE;

	PLATFORM_MEMCPY(auth->c_nonce, c_nonce, c_nonce_len);

	/* Derive responder ke */
	if (dpp_reconfig_derive_ke_responder(auth, net_access_key, net_access_key_len, token) != 1)
		goto fail;

	if (c_nonce_len != auth->curve->nonce_len) {
		PLATFORM_PRINTF_WARNING("DPP: Unexpected C-Nonce length %u (curve nonce len %u)\n", c_nonce_len, auth->curve->nonce_len);
		goto fail;
	}

	/* Build Connection Status object */
	/* TODO: Get appropriate result value */
	/* TODO: ssid64 and channelList */
	if (dpp_build_conn_status(DPP_STATUS_NO_AP, NULL, 0, NULL, &conn_status) != 1)
		goto fail;

	PLATFORM_PRINTF_INFO("DPP: Reconfig Auth Req Rx finished...\n");

out:
	PLATFORM_FREE(own_conn);
	return auth;
fail:
	PLATFORM_FREE(auth);
	auth = NULL;
	goto out;
}

// DPP Reconfiguration Authentication Response Builder
INT8U dpp_build_reconfig_auth_resp(struct dpp_authentication *auth,
                                   char *own_connector, INT8U *conn_status, INT32U conn_status_len,
                                   INT8U **reconfig_auth_resp_msg, INT16U *reconfig_auth_resp_msg_len)
{
	INT8U  ret = 0, pr[MAX_ANSI_X963_LEN];
	INT8U *msg, *p, *p2, *clear = NULL;
	INT32S pr_len;
	INT32U clear_len, conn_len, siv_len, attr_len, out_len;

	const INT8U *addr[2];
	INT32U       len[2];

	PLATFORM_PRINTF_INFO("DPP: Build Reconfiguration Authentication Response\n");

	clear_len = 4 + auth->curve->nonce_len; // C-nonce
	clear_len += 4 + conn_status_len;       // Connection Status

	INT8U wrapped_data[clear_len + AES_BLOCK_SIZE];

	if (PLATFORM_EC_KEY_GET_PUBLIC_POINT(&auth->own_protocol_key, pr, &pr_len) != 1)
		goto fail;

	conn_len = PLATFORM_STRLEN(own_connector);

	/* Build DPP Reconfiguration Authentication Response frame attributes, P132 of Easy Connect Spec v2. */
	attr_len = 4 + 1;                       // Transaction ID
	attr_len += 4 + 1;                      // Protocol Version
	attr_len += 4 + conn_len;               // Connector
	attr_len += 4 + auth->curve->nonce_len; // Enrollee Nonce
	attr_len += 4 + pr_len;                 // PR
	attr_len += 4 + clear_len;              // {...}ke

	/* Allocate message buffer */
	msg = dpp_alloc_msg(DPP_PA_RECONFIG_AUTH_RESP, attr_len, &out_len);
	if (!msg) {
		PLATFORM_PRINTF_WARNING("DPP: alloc_msg failed!\n");
		goto fail;
	}
	p = msg + (out_len - attr_len);

	INT8U  aux8;
	INT16U aux16;

	/* Transaction ID */
	aux16 = (INT16U)DPP_ATTR_TRANSACTION_ID;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)1;
	_I2B_LE(&aux16, &p);
	aux8 = TRANSACTION_ID;
	_I1B(&aux8, &p);

	/* Protocol Version */
	aux16 = (INT16U)DPP_ATTR_PROTOCOL_VERSION;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)1;
	_I2B_LE(&aux16, &p);
	aux8 = DPP_VERSION;
	_I1B(&aux8, &p);

	/* R-Connector */
	aux16 = (INT16U)DPP_ATTR_CONNECTOR;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)conn_len;
	_I2B_LE(&aux16, &p);
	_InB(own_connector, &p, conn_len);

	/* E-Nonce */
	aux16 = (INT16U)DPP_ATTR_ENROLLEE_NONCE;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)auth->curve->nonce_len;
	_I2B_LE(&aux16, &p);
	_InB(auth->e_nonce, &p, auth->curve->nonce_len);

	/* Responder Protocol Key (Pr) */
	aux16 = (INT16U)DPP_ATTR_R_PROTOCOL_KEY;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)pr_len;
	_I2B_LE(&aux16, &p);
	_InB(pr, &p, pr_len);

	/* Wrapped data ({C-Nonce, Connection Status}ke) */
	clear = (INT8U *)PLATFORM_MALLOC(clear_len);
	p2    = clear;

	/* (Wrapped) C-nonce */
	aux16 = (INT16U)DPP_ATTR_CONFIGURATOR_NONCE;
	_I2B_LE(&aux16, &p2);
	aux16 = (INT16U)auth->curve->nonce_len;
	_I2B_LE(&aux16, &p2);
	_InB(auth->c_nonce, &p2, auth->curve->nonce_len);

	/* (Wrapped) Connection Status */
	aux16 = (INT16U)DPP_ATTR_CONN_STATUS;
	_I2B_LE(&aux16, &p2);
	aux16 = (INT16U)conn_status_len;
	_I2B_LE(&aux16, &p2);
	_InB(conn_status, &p2, conn_status_len);

	/* Associated data: OUI, OUI type, Crypto Suite, DPP frame type */
	addr[0] = msg + 2;
	len[0]  = 3 + 1 + 1 + 1;

	/* Attributes before Wrapped Data */
	addr[1] = msg + 2 + 3 + 1 + 1 + 1;
	len[1]  = p - (msg + 2 + 3 + 1 + 1 + 1);

	siv_len = p2 - clear;
	if (PLATFORM_AES_SIV_ENCRYPT(auth->ke, auth->curve->hash_len, clear, siv_len, 2, addr, len, wrapped_data) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot encrypt with ke\n");
		PLATFORM_FREE(msg);
		goto fail;
	}

	/* Wrapped data */
	siv_len += AES_BLOCK_SIZE;
	aux16 = (INT16U)DPP_ATTR_WRAPPED_DATA;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)siv_len;
	_I2B_LE(&aux16, &p);
	_InB(wrapped_data, &p, siv_len);

	*reconfig_auth_resp_msg     = msg;
	*reconfig_auth_resp_msg_len = out_len;

	PLATFORM_PRINTF_INFO("DPP: Reconfiguration Authentication Response Generated\n");
	ret = 1;

fail:
	PLATFORM_FREE(clear);
	return ret;
}

// DPP Reconfiguration Authentication Request Parser
INT8U dpp_rx_reconfig_auth_resp(struct dpp_authentication *auth, struct dpp_connector *conn_out,
                                INT8U *buf, INT32U len)
{
	INT8U *trans_id, *version, *r_connector, *e_nonce, *r_proto,
	    *wrapped_data, *c_nonce, *conn_status;
	INT16U trans_id_len = 0, version_len = 0, r_connector_len = 0, e_nonce_len = 0, r_proto_len = 0,
	       wrapped_data_len = 0, c_nonce_len = 0, conn_status_len = 0;

	int    unwrapped_len = 0;
	INT8U *unwrapped     = 0;

	INT8U *attr_start = buf + 8;
	INT32U attr_len   = len - 8;

	cJSON *conn_status_json = NULL;

	INT8U ret = 0;

	if (!auth->configurator)
		return ret;

	if (!auth->reconfig) {
		PLATFORM_PRINTF_WARNING("DPP: No DPP Reconfiguration in progress - drop\n");
		return ret;
	}

	/* Get wrapped data {...}ke */
	wrapped_data = dpp_get_attr(attr_start, attr_len, DPP_ATTR_WRAPPED_DATA, &wrapped_data_len);
	if (!wrapped_data || wrapped_data_len < AES_BLOCK_SIZE) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required Wrapped Data attribute\n");
		return ret;
	}

	// Ignore things related to wrapped data
	attr_len = wrapped_data - 4 - attr_start;

	/* Get Transaction ID */
	trans_id = dpp_get_attr(attr_start, attr_len, DPP_ATTR_TRANSACTION_ID, &trans_id_len);
	if (!trans_id || trans_id_len != 1 || trans_id[0] != TRANSACTION_ID) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required Transaction ID attribute\n");
		return ret;
	}

	/* Get Protocol Version */
	version = dpp_get_attr(attr_start, attr_len, DPP_ATTR_PROTOCOL_VERSION, &version_len);
	if (!version || version_len < 1 || version[0] < DPP_VERSION) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required Protocol Version attribute\n");
		return ret;
	}

	/* Get R-Connector */
	r_connector = dpp_get_attr(attr_start, attr_len, DPP_ATTR_CONNECTOR, &r_connector_len);
	if (!r_connector) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required R-Connector attribute\n");
		return ret;
	}

	/* Get E-Nonce */
	e_nonce = dpp_get_attr(attr_start, attr_len, DPP_ATTR_ENROLLEE_NONCE, &e_nonce_len);
	if (!e_nonce) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required E-Nonce attribute\n");
		return ret;
	}
	PLATFORM_MEMCPY(auth->e_nonce, e_nonce, e_nonce_len);

	/* Get Responder Protocol Key */
	r_proto = dpp_get_attr(attr_start, attr_len, DPP_ATTR_R_PROTOCOL_KEY, &r_proto_len);
	if (!r_proto) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required Responder Protocol Key attribute\n");
		return ret;
	}

	/* Process signed connector */
	if (dpp_parse_signed_connector((char *)r_connector, r_connector_len, auth->conf, conn_out, NULL) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Invalid Connector\n");
		dpp_free_connector(conn_out);
		return ret;
	}

	/* Derive initiator ke */
	if (dpp_reconfig_derive_ke_initiator(auth, r_proto, r_proto_len, &conn_out->netaccesskey) != 1)
		return ret;

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

	/* Check decrypted content format */
	if (dpp_check_attrs(unwrapped, unwrapped_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Invalid attribute in unwrapped data\n");
		goto fail;
	}

	/* Get (wrapped) C-nonce */
	c_nonce = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_CONFIGURATOR_NONCE, &c_nonce_len);
	if (!c_nonce || c_nonce_len != auth->curve->nonce_len) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid C-nonce\n");
		goto fail;
	}
	PLATFORM_MEMCPY(auth->e_nonce, e_nonce, e_nonce_len);

	/* Get (wrapped) Connection Status */
	conn_status = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_CONN_STATUS, &conn_status_len);
	if (!conn_status) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid Connection Status\n");
		goto fail;
	}

	conn_status_json = cJSON_Parse((char *)conn_status);
	if (!conn_status_json) {
		PLATFORM_PRINTF_WARNING("DPP: Could not parse connStatus\n");
		goto fail;
	}

	/* TODO: use connStatus information */

	auth->auth_success = 1;
	auth->in_progress  = 0;
	PLATFORM_PRINTF_INFO("DPP: Reconfig Auth Resp Rx finished...\n");
	ret = 1;

fail:
	PLATFORM_FREE(unwrapped);
	return ret;
}

// DPP Reconfiguration Authentication Confirm Builder
INT8U dpp_build_reconfig_auth_conf(struct dpp_authentication *auth,
                                 INT8U **reconfig_auth_conf_msg, INT32U *reconfig_auth_conf_msg_len)
{
	INT8U  ret = 0, flags;
	INT8U *msg, *p, *p2;
	INT32U wrapped_len, siv_len, attr_len, out_len;

	const INT8U *addr[2];
	INT32U       len[2];

	PLATFORM_PRINTF_INFO("DPP: Build Reconfiguration Authentication Confirm\n");

	wrapped_len = 4 + 1;                             // Wrapped data: Transaction ID
	wrapped_len += 4 + 1;                            // Wrapped data: Protocol Version
	wrapped_len += 2 * (4 + auth->curve->nonce_len); // Wrapped data: C-nonce, E-nonce
	wrapped_len += 4 + 1;                            // Wrapped data: Reconfiguration Flags
	wrapped_len += AES_BLOCK_SIZE;                   // Wrapped data: AES Block

	INT8U clear[wrapped_len - AES_BLOCK_SIZE];
	INT8U wrapped_data[wrapped_len];

	/* Build DPP Reconfiguration Authentication Confirm frame attributes, P132 of Easy Connect Spec v2. */
	attr_len = 4 + 1;            // DPP Status
	attr_len += 4 + wrapped_len; // {...}ke

	/* Allocate message buffer */
	msg = dpp_alloc_msg(DPP_PA_RECONFIG_AUTH_CONF, attr_len, &out_len);
	if (!msg) {
		PLATFORM_PRINTF_WARNING("DPP: alloc_msg failed!\n");
		goto fail;
	}
	p = msg + (out_len - attr_len);

	INT8U  aux8;
	INT16U aux16;

	/* DPP Status */
	aux16 = (INT16U)DPP_ATTR_STATUS;
	_I2B_LE(&aux16, &p);
	aux16 = 1;
	_I2B_LE(&aux16, &p);
	aux8 = (INT8U)DPP_STATUS_OK;
	_I1B(&aux8, &p);

	/* Wrapped data ({...}ke) */
	p2 = clear;

	/* (Wrapped) Transaction ID */
	aux16 = (INT16U)DPP_ATTR_TRANSACTION_ID;
	_I2B_LE(&aux16, &p2);
	aux16 = (INT16U)1;
	_I2B_LE(&aux16, &p2);
	aux8 = TRANSACTION_ID;
	_I1B(&aux8, &p2);

	/* (Wrapped) Protocol Version */
	aux16 = (INT16U)DPP_ATTR_PROTOCOL_VERSION;
	_I2B_LE(&aux16, &p2);
	aux16 = (INT16U)1;
	_I2B_LE(&aux16, &p2);
	aux8 = DPP_VERSION;
	_I1B(&aux8, &p2);

	/* (Wrapped) C-Nonce */
	aux16 = (INT16U)DPP_ATTR_CONFIGURATOR_NONCE;
	_I2B_LE(&aux16, &p2);
	aux16 = (INT16U)auth->curve->nonce_len;
	_I2B_LE(&aux16, &p2);
	_InB(auth->c_nonce, &p2, auth->curve->nonce_len);

	/* (Wrapped) E-Nonce */
	aux16 = (INT16U)DPP_ATTR_ENROLLEE_NONCE;
	_I2B_LE(&aux16, &p2);
	aux16 = (INT16U)auth->curve->nonce_len;
	_I2B_LE(&aux16, &p2);
	_InB(auth->e_nonce, &p2, auth->curve->nonce_len);

	/* (Wrapped) Reconfiguration Flags */
	flags = DPP_CONFIG_REPLACEKEY;
	aux16 = (INT16U)DPP_ATTR_RECONFIG_FLAGS;
	_I2B_LE(&aux16, &p2);
	aux16 = (INT16U)1;
	_I2B_LE(&aux16, &p2);
	aux8 = flags;
	_I1B(&aux8, &p2);

	/* Associated data: OUI, OUI type, Crypto Suite, DPP frame type */
	addr[0] = msg + 2;
	len[0]  = 3 + 1 + 1 + 1;

	/* Attributes before Wrapped Data */
	addr[1] = msg + 2 + 3 + 1 + 1 + 1;
	len[1]  = p - (msg + 2 + 3 + 1 + 1 + 1);

	siv_len = p2 - clear;
	if (PLATFORM_AES_SIV_ENCRYPT(auth->ke, auth->curve->hash_len, clear, siv_len, 2, addr, len, wrapped_data) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: cannot encrypt with ke\n");
		PLATFORM_FREE(msg);
		goto fail;
	}

	/* Wrapped data */
	siv_len += AES_BLOCK_SIZE;
	aux16 = (INT16U)DPP_ATTR_WRAPPED_DATA;
	_I2B_LE(&aux16, &p);
	aux16 = (INT16U)siv_len;
	_I2B_LE(&aux16, &p);
	_InB(wrapped_data, &p, siv_len);

	*reconfig_auth_conf_msg     = msg;
	*reconfig_auth_conf_msg_len = out_len;

	PLATFORM_PRINTF_INFO("DPP: Reconfiguration Authentication Confirm Generated\n");
	ret = 1;

fail:
	return ret;
}

// DPP Reconfiguration Authentication Confirm Parser
INT8U dpp_rx_reconfig_auth_conf(struct dpp_authentication *auth,
                                INT8U *buf, INT32U len)
{
	INT8U *status, *wrapped_data, *trans_id, *version,
	    *c_nonce, *e_nonce, *reconfig_flags;
	INT16U status_len = 0, wrapped_data_len = 0, trans_id_len = 0, version_len = 0,
	       c_nonce_len = 0, e_nonce_len = 0, reconfig_flags_len = 0;

	int    unwrapped_len = 0;
	INT8U *unwrapped     = 0;

	INT8U *attr_start = buf + 8;
	INT32U attr_len   = len - 8;

	INT8U ret = 0;

	if (auth->configurator)
		return ret;

	if (!auth->reconfig) {
		PLATFORM_PRINTF_WARNING("DPP: No DPP Reconfiguration in progress - drop\n");
		return ret;
	}

	/* Get wrapped data {...}ke */
	wrapped_data = dpp_get_attr(attr_start, attr_len, DPP_ATTR_WRAPPED_DATA, &wrapped_data_len);
	if (!wrapped_data || wrapped_data_len < AES_BLOCK_SIZE) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required Wrapped Data attribute\n");
		return ret;
	}

	// Ignore things related to wrapped data
	attr_len = wrapped_data - 4 - attr_start;

	/* Get Status */
	status = dpp_get_attr(attr_start, attr_len, DPP_ATTR_STATUS, &status_len);
	if (!status || status_len < 1) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required Status attribute\n");
		return ret;
	}
	if (status[0] != DPP_STATUS_OK) {
		PLATFORM_PRINTF_WARNING("DPP: Reconfiguration did not complete successfully\n");
		return ret;
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

	/* Check decrypted content format */
	if (dpp_check_attrs(unwrapped, unwrapped_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP: Invalid attribute in unwrapped data\n");
		goto fail;
	}

	/* Get (wrapped) Transaction ID */
	trans_id = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_TRANSACTION_ID, &trans_id_len);
	if (!trans_id || trans_id_len != 1 || trans_id[0] != TRANSACTION_ID) {
		PLATFORM_PRINTF_WARNING("DPP: Peer did not include valid Transaction ID\n");
		goto fail;
	}

	/* Get (wrapped) Protocol Version */
	version = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_PROTOCOL_VERSION, &version_len);
	if (!version || version_len < 1 || version[0] < DPP_VERSION) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required Protocol Version attribute\n");
		goto fail;
	}

	/* Get (wrapped) C-Nonce */
	c_nonce = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_CONFIGURATOR_NONCE, &c_nonce_len);
	if (!c_nonce) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required C-Nonce attribute\n");
		goto fail;
	}

	/* Get (wrapped) E-Nonce */
	e_nonce = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_ENROLLEE_NONCE, &e_nonce_len);
	if (!e_nonce) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required E-Nonce attribute\n");
		goto fail;
	}

	/* Get (wrapped) Reconfiguration Flags */
	reconfig_flags = dpp_get_attr(unwrapped, unwrapped_len, DPP_ATTR_ENROLLEE_NONCE, &reconfig_flags_len);
	if (!reconfig_flags || reconfig_flags_len < 1) {
		PLATFORM_PRINTF_WARNING("DPP: Missing or invalid required Reconfiguration Flags attribute\n");
		goto fail;
	}
	auth->reconfig_connector_key = reconfig_flags[0] & BIT(0);

	auth->auth_success = 1;
	auth->in_progress  = 0;
	PLATFORM_PRINTF_INFO("DPP: Reconfig Auth Confirm Rx finished...\n");
	ret                = 1;

fail:
	PLATFORM_FREE(unwrapped);
	return ret;
}
