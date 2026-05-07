/*
 *  Multi-AP Device Provisioning Protocol (DPP)
 *
 *  Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 */

#ifndef _AL_DPP_H_
#define _AL_DPP_H_

#include "platform_crypto.h"
#include "platform_crypto_ecc.h"

#include "list.h"
#include "cJSON.h"

#define DPP_BOOTSTRAP_OWN_URI_FILENAME "/tmp/dpp_bootstrap_uri"
#define DPP_BOOTSTRAP_OWN_PRIV_FILENAME "/tmp/dpp_bootstrap_key"

#define DPP_BOOTSTRAP_CSIGN_FILENAME "/tmp/dpp_csign"
#define DPP_BOOTSTRAP_PPK_FILENAME "/tmp/dpp_ppk"

#define SSID_MAX_LEN 32
#define RUID_ASCII_HEX_LEN 12
#define BSSID_ASCII_HEX_LEN 12

#define DPP_MAX_NONCE_LEN 32
#define DPP_MAX_HASH_LEN 64
#define DPP_MAX_SHARED_SECRET_LEN 66

#define DPP_MAX_CONF_REQ_OBJ_BSTALIST 2
#define DPP_MAX_CONFIGURATOR 2
#define DPP_MAX_CONF_OBJ 2
#define DPP_MAX_CONF_OBJ_GROUPS 2

#define WLAN_ACTION_PUBLIC 4
#define WLAN_PA_VENDOR_SPECIFIC 9
#define WLAN_PA_GAS_INITIAL_REQ 10
#define WLAN_PA_GAS_INITIAL_RESP 11
#define WLAN_PA_GAS_COMEBACK_REQ 12
#define WLAN_PA_GAS_COMEBACK_RESP 13

/* Status codes (IEEE Std 802.11-2016, 9.4.1.9, Table 9-46) */
#define WLAN_STATUS_SUCCESS 0

/* IEEE 802.11i */
#define PMKID_LEN 16
#define PMK_LEN 32
#define PMK_LEN_MAX 64

#define OUI_WFA 0x506f9a
#define DPP_OUI_TYPE 0x1A

#define DPP_VERSION 2

enum dpp_public_action_frame_type {
	DPP_PA_AUTHENTICATION_REQ       = 0,
	DPP_PA_AUTHENTICATION_RESP      = 1,
	DPP_PA_AUTHENTICATION_CONF      = 2,
	DPP_PA_PEER_DISCOVERY_REQ       = 5,
	DPP_PA_PEER_DISCOVERY_RESP      = 6,
	DPP_PA_PKEX_EXCHANGE_REQ        = 7,
	DPP_PA_PKEX_EXCHANGE_RESP       = 8,
	DPP_PA_PKEX_COMMIT_REVEAL_REQ   = 9,
	DPP_PA_PKEX_COMMIT_REVEAL_RESP  = 10,
	DPP_PA_CONFIGURATION_RESULT     = 11,
	DPP_PA_CONNECTION_STATUS_RESULT = 12,
	DPP_PA_PRESENCE_ANNOUNCEMENT    = 13,
	DPP_PA_RECONFIG_ANNOUNCEMENT    = 14,
	DPP_PA_RECONFIG_AUTH_REQ        = 15,
	DPP_PA_RECONFIG_AUTH_RESP       = 16,
	DPP_PA_RECONFIG_AUTH_CONF       = 17,
};

enum dpp_attribute_id {
	DPP_ATTR_STATUS               = 0x1000,
	DPP_ATTR_I_BOOTSTRAP_KEY_HASH = 0x1001,
	DPP_ATTR_R_BOOTSTRAP_KEY_HASH = 0x1002,
	DPP_ATTR_I_PROTOCOL_KEY       = 0x1003,
	DPP_ATTR_WRAPPED_DATA         = 0x1004,
	DPP_ATTR_I_NONCE              = 0x1005,
	DPP_ATTR_I_CAPABILITIES       = 0x1006,
	DPP_ATTR_R_NONCE              = 0x1007,
	DPP_ATTR_R_CAPABILITIES       = 0x1008,
	DPP_ATTR_R_PROTOCOL_KEY       = 0x1009,
	DPP_ATTR_I_AUTH_TAG           = 0x100A,
	DPP_ATTR_R_AUTH_TAG           = 0x100B,
	DPP_ATTR_CONFIG_OBJ           = 0x100C,
	DPP_ATTR_CONNECTOR            = 0x100D,
	DPP_ATTR_CONFIG_ATTR_OBJ      = 0x100E,
	DPP_ATTR_BOOTSTRAP_KEY        = 0x100F,
	DPP_ATTR_OWN_NET_NK_HASH      = 0x1011,
	DPP_ATTR_FINITE_CYCLIC_GROUP  = 0x1012,
	DPP_ATTR_ENCRYPTED_KEY        = 0x1013,
	DPP_ATTR_ENROLLEE_NONCE       = 0x1014,
	DPP_ATTR_CODE_IDENTIFIER      = 0x1015,
	DPP_ATTR_TRANSACTION_ID       = 0x1016,
	DPP_ATTR_BOOTSTRAP_INFO       = 0x1017,
	DPP_ATTR_CHANNEL              = 0x1018,
	DPP_ATTR_PROTOCOL_VERSION     = 0x1019,
	DPP_ATTR_ENVELOPED_DATA       = 0x101A,
	DPP_ATTR_SEND_CONN_STATUS     = 0x101B,
	DPP_ATTR_CONN_STATUS          = 0x101C,
	DPP_ATTR_RECONFIG_FLAGS       = 0x101D,
	DPP_ATTR_C_SIGN_KEY_HASH      = 0x101E,
	DPP_ATTR_CSR_ATTR_REQ         = 0x101F,
	DPP_ATTR_A_NONCE              = 0x1020,
	DPP_ATTR_E_PRIME_ID           = 0x1021,
	DPP_ATTR_CONFIGURATOR_NONCE   = 0x1022,
};

enum dpp_status_error {
	DPP_STATUS_OK                = 0,
	DPP_STATUS_NOT_COMPATIBLE    = 1,
	DPP_STATUS_AUTH_FAILURE      = 2,
	DPP_STATUS_UNWRAP_FAILURE    = 3,
	DPP_STATUS_BAD_GROUP         = 4,
	DPP_STATUS_CONFIGURE_FAILURE = 5,
	DPP_STATUS_RESPONSE_PENDING  = 6,
	DPP_STATUS_INVALID_CONNECTOR = 7,
	DPP_STATUS_NO_MATCH          = 8,
	DPP_STATUS_CONFIG_REJECTED   = 9,
	DPP_STATUS_NO_AP             = 10,
	DPP_STATUS_CONFIGURE_PENDING = 11,
	DPP_STATUS_CSR_NEEDED        = 12,
	DPP_STATUS_CSR_BAD           = 13,
};

struct dpp_curve_params {
	const char *name;
	INT32U      hash_len;
	INT32U      aes_siv_key_len;
	INT32U      nonce_len;
	INT32U      prime_len;
	const char *jwk_crv;
	INT16U      ike_group;
	const char *jws_alg;
};

struct dpp_bootstrap_info {
	struct map_list list;
	struct dpp_authentication *auth;
	ECCKeyPair     keypair;
	unsigned char  pubkey_hash[SHA256_MAC_LEN];
	unsigned char  pubkey_hash_chirp[SHA256_MAC_LEN];
	unsigned char *pubkey_b64;
	char *         uri;
};

enum dpp_netrole {
	DPP_NETROLE_STA,
	DPP_NETROLE_AP,
	DPP_NETROLE_CONFIGURATOR,
	DPP_NETROLE_MAP_BACKHAUL_STA,
	DPP_NETROLE_MAP_BACKHAUL_BSS,
	DPP_NETROLE_MAP_CONTROLLER,
	DPP_NETROLE_MAP_AGENT,
	DPP_NETROLE_UNKNOWN
};

/* DPP Reconfig Flags object - connectorKey values */
enum dpp_connector_key {
	DPP_CONFIG_REUSEKEY   = 0,
	DPP_CONFIG_REPLACEKEY = 1,
};

struct dpp_netrole_params {
	enum dpp_netrole        netrole;
	const char *            netrole_str;
	const enum dpp_netrole *compatible_list;
};

struct dpp_config_req_obj {
	char             name[32];
	char             wifi_tech[32];
	enum dpp_netrole netRole;
	struct bSTAList {
		enum dpp_netrole netRole;
		char             akm[32];
		char             channelList[128];
	} b_sta_list[DPP_MAX_CONF_REQ_OBJ_BSTALIST];
	INT8U b_sta_list_count;
};

struct dpp_connector {
	struct dpp_groups {
		char             groupId[32];
		enum dpp_netrole netRole;
	} groups[DPP_MAX_CONF_OBJ_GROUPS];
	INT8U      group_count;
	ECCKeyPair netaccesskey;
	INT8U      netaccesskey_kid_hash[SHA256_MAC_LEN];
	char *     netaccesskey_kid;
	INT8U *    signedConnector;
};

struct dpp_config_obj {
	char   wifi_tech[32];
	INT16U dfCounterThreshold;

	// Discovery
	char   ssid[SSID_MAX_LEN + 1]; // +1 for null terminated string
	INT8U  ssid_len;
	char * ssid64;
	INT16U ssid_charset;

	char ruid[RUID_ASCII_HEX_LEN + 1]; // +1 for null terminated string
	char ruid_enable;

	char bssid[BSSID_ASCII_HEX_LEN + 1]; // +1 for null terminated string
	char bssid_enable;

	// Cred
	char                 akm[32];
	char                 psk_hex[64];
	char                 pass[64];
	struct dpp_connector conn;
	ECCKeyPair           csign_pub;
	ECCKeyPair           pp_pub;
};

struct dpp_configurator {
	const struct dpp_curve_params *curve;

	// C-Sign Key
	ECCKeyPair csign;
	INT8U      csign_kid_hash[SHA256_MAC_LEN];
	char      *csign_kid;

	// Privacy Protection Key
	ECCKeyPair ppkey;
	INT8U      ppkey_kid_hash[SHA256_MAC_LEN];
	char      *ppkey_kid;
};

struct dpp_introduction {
	INT8U  pmkid[PMKID_LEN];
	INT8U  pmk[PMK_LEN_MAX];
	INT32U pmk_len;
};

struct dpp_authentication {
	struct map_list list;

	INT8U allowed_roles;
	INT8U initiator;
	INT8U configurator;

	INT8U in_progress;
	INT8U auth_success;
	INT8U waiting_auth_resp;

	const struct dpp_curve_params *curve;

	struct dpp_bootstrap_info *own_bi;
	struct dpp_bootstrap_info peer_bi;

	ECCKeyPair own_protocol_key;
	ECCKeyPair peer_protocol_key;

	INT8U  Mx[MAX_SHARED_SECRET_LEN];
	size_t Mx_len;

	INT8U  Nx[MAX_SHARED_SECRET_LEN];
	size_t Nx_len;

	INT8U i_capab;
	INT8U r_capab;

	INT8U i_nonce[DPP_MAX_NONCE_LEN];
	INT8U r_nonce[DPP_MAX_NONCE_LEN];
	INT8U e_nonce[DPP_MAX_NONCE_LEN];
	INT8U c_nonce[DPP_MAX_NONCE_LEN];

	INT8U k1[DPP_MAX_HASH_LEN];
	INT8U k2[DPP_MAX_HASH_LEN];
	INT8U ke[DPP_MAX_HASH_LEN];
	INT8U bk[DPP_MAX_HASH_LEN];

	struct dpp_configurator *conf;
	struct dpp_config_obj conf_obj[DPP_MAX_CONF_OBJ];
	INT8U                 conf_obj_count;

	INT8U                  reconfig;
	ECCKeyPair             reconfig_old_protocol_key;
	enum dpp_connector_key reconfig_connector_key;
};

// hardcoded Transaction ID because only single transaction is supported
static const INT8U TRANSACTION_ID = 1;

#ifndef BIT
#	define BIT(x) (1U << (x))
#endif

#define DPP_CAPAB_ENROLLEE BIT(0)
#define DPP_CAPAB_CONFIGURATOR BIT(1)
#define DPP_CAPAB_ROLE_MASK (BIT(0) | BIT(1))

const struct dpp_curve_params *dpp_get_curve_ike_group(INT16U group);

const struct dpp_curve_params *dpp_get_curve_name(const char *name);

/**
 * dpp_keygen - Generate DPP bootstrap key
 * If a private key is given, this function parses the key into 'bi'.
 * Otherwise, it generates a new key using the given curve.
 *
 * @bi: a predefined bootstrap info buffer
 * @curve: curve name (e.g., "prime256v1")
 * @priv: private key buffer (NULL for keygen, parsing otherwise)
 * @priv_len: private key length (0 for keygen, parsing otherwise)
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_keygen(struct dpp_bootstrap_info *bi, const char *curve, unsigned char *priv, int priv_len);

/**
 * dpp_keygen_configurator - Generate DPP configurator csign key
 * If a private key is given, this function parses the key into 'conf'.
 * Otherwise, it generates a new key using the given curve.
 *
 * @conf: a predefined configurator buffer
 * @curve: curve name (e.g., "prime256v1")
 * @csign_priv: private csign key buffer (NULL for keygen, parsing otherwise)
 * @csign_priv_len: private csign key length (0 for keygen, parsing otherwise)
 * @ppk_priv: private privacy protection key buffer (NULL for keygen, parsing otherwise)
 * @ppk_priv_len: private privacy protection key length (0 for keygen, parsing otherwise)
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_keygen_configurator(struct dpp_configurator *conf, const char *curve, INT8U *csign, INT16U csign_len, INT8U *ppk, INT16U ppk_len);

/**
 * dpp_parse_uri - Parse DPP URI
 * Given a DPP URI string, it parses the content and stores them into 'bi'.
 *
 * @uri: DPP URI string
 * @bi: a predefined bootstrap info buffer
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_parse_uri(const char *uri, struct dpp_bootstrap_info *bi);

int dpp_hmac(size_t hash_len, INT8U *key, size_t key_len, INT8U *data, INT32U data_len, INT8U *mac);

int dpp_hkdf_expand(size_t hash_len, INT8U *secret, size_t secret_len, const char *label, INT8U *out, size_t outlen);

/**
 * dpp_auth_clear_state - Clear states in the DPP Authentication struct
 * Reset the current state variables updated by dpp_auth_* and dpp_rx_auth_* functions.
 *
 * @auth: DPP authentication struct stored with authentication information
 * Returns: N/A
 */
void dpp_auth_clear_state(struct dpp_authentication *auth);

/**
 * dpp_auth_init - Start DPP Authentication Request
 * Using the information stored in 'auth' to generate DPP auth request.
 *
 * @auth: DPP authentication struct stored with authentication information
 * @auth_req_msg: upon successful execution, it stores the address of dynamically generated message buffer
 * @auth_req_msg_len: upon successful execution, it stores the length of the message buffer
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_auth_init(struct dpp_authentication *auth, INT8U **auth_req_msg, INT32U *auth_req_msg_len);

/**
 * dpp_alloc_msg - Allocate DPP message buffer based on the specified details
 *
 * @type: DPP Action Frame type to be set
 * @len: Calculated length of attributes contained in message before overhead appended
 * @out_len: Calculated length of message buffer needed for the specified message with the overhead
 * Returns: 1 on success, 0 on failure
 */
INT8U *dpp_alloc_msg(enum dpp_public_action_frame_type type, INT32U len, INT32U *out_len);

/**
 * dpp_get_attr - Get a specified attribute from DPP Action Frame buffer based on the specified attribute ID
 *
 * @buf: DPP Action Frame buffer
 * @len: Total length of the buffer of attributes
 * @req_id: Attribute ID that is requested
 * @ret_len: Returned length of the attribute found
 * Returns: start address of searched attribute on success, NULL on failure
 */
INT8U *dpp_get_attr(INT8U *buf, INT32U len, INT16U req_id, INT16U *ret_len);

/**
 * dpp_check_attrs - Sanity check of DPP Action Frame buffer
 *
 * @buf: DPP Action Frame buffer
 * @len: Total length of the buffer of attributes
 * Returns: 1 on success, 0 on failure
 */
int dpp_check_attrs(INT8U *buf, INT32U len);

/**
 * dpp_rx_auth_req - Receive DPP Authentication Request
 * Validate the incoming DPP auth response stored in 'buf', by comparing the information stored in 'auth'.
 *
 * @auth: DPP authentication struct stored with authentication information
 * @buf: DPP authentication request message going to parse
 * @len: length of the authentication request message
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_rx_auth_req(struct dpp_authentication *auth, INT8U *buf, INT32U len);

/**
 * dpp_auth_build_resp_ok - Build DPP Authentication Response with OK status
 * Using the information stored in 'auth' to generate DPP auth response.
 *
 * @auth: DPP authentication struct stored with authentication information
 * @auth_req_msg: upon successful execution, it stores the address of dynamically generated message buffer
 * @auth_req_msg_len: upon successful execution, it stores the length of the message buffer
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_auth_build_resp_ok(struct dpp_authentication *auth, INT8U **auth_resp_msg, INT32U *auth_resp_msg_len);

/**
 * dpp_rx_auth_resp - Receive DPP Authentication Response
 * Validate the incoming DPP auth response stored in 'buf', by comparing the information stored in 'auth'.
 *
 * @auth: DPP authentication struct stored with authentication information
 * @buf: DPP authentication response message going to parse
 * @len: length of the authentication request message
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_rx_auth_resp(struct dpp_authentication *auth, INT8U *buf, INT32U len);

/**
 * dpp_auth_build_conf - Build DPP Authentication Confirm
 * Using the information stored in 'auth' and 'status' to generate DPP auth confirm.
 *
 * @auth: DPP authentication struct stored with authentication information
 * @status: DPP status you wish to report in the auth confim message
 * @auth_req_msg: upon successful execution, it stores the address of dynamically generated message buffer
 * @auth_req_msg_len: upon successful execution, it stores the length of the message buffer
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_auth_build_conf(struct dpp_authentication *auth, enum dpp_status_error status, INT8U **auth_conf_msg, INT32U *auth_conf_msg_len);

/**
 * dpp_rx_auth_conf - Receive DPP Authentication Confirm
 * Validate the incoming DPP auth conf stored in 'buf', by comparing the information stored in 'auth'.
 *
 * @auth: DPP authentication struct stored with authentication information
 * @buf: DPP authentication response message going to parse
 * @len: length of the authentication request message
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_rx_auth_conf(struct dpp_authentication *auth, INT8U *buf, INT32U len);

/**
 * dpp_build_conf_req - Build DPP Configuration Request
 * Using the information stored in 'auth' to generate DPP config request (GAS initial request).
 *
 * @auth: DPP authentication struct stored with authentication information
 * @conf_req_msg: upon successful execution, it stores the address of dynamically generated message buffer
 * @conf_req_msg_len: upon successful execution, it stores the length of the message buffer
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_build_conf_req(struct dpp_authentication *auth, struct dpp_config_req_obj *obj,
                         INT8U **conf_req_msg, INT32U *conf_req_msg_len);
/**
 * dpp_conf_req_rx - Receive DPP Configuration Request
 * Validate the incoming DPP config req stored in 'buf', by comparing the information stored in 'auth'.
 *
 * @auth: DPP authentication struct stored with authentication information
 * @buf: DPP authentication response message going to parse
 * @len: length of the authentication request message
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_conf_req_rx(struct dpp_authentication *auth, struct dpp_config_req_obj *obj_out,
                      INT8U *buf, INT32U len);

/**
 * @brief Get all operation classes contained in dpp_config_req_obj.
 * @param req_obj dpp_config_req_obj from configuration request message.
 * @return INT8U* Currently only return a pointer to the operation class array.
 */
INT8U *dpp_get_op_class_from_conf_req(struct dpp_config_req_obj *req_obj);

INT8U dpp_parse_jwk(cJSON *jwk, ECCKeyPair *pub_key_out, INT8U *kid_hash_out, char **kid_out);

/**
 * dpp_build_signed_connector - Build Signed DPP Connector Object
 * Sign the netAccessKey + groupId + netRole information with the csign key stored in 'conf'.
 * The information and signed connector string will be saved to 'conn'.
 *
 * This function assumes only one groupId + netRole presents for simplicity.
 *
 * @conf: The private C-Sign Key (Configurator Signing Key) for generating signed Connector.
 * @net_access_key: the public key in the connector.
 * if NULL is given, it generates a new key and store it to conn's netaccesskey.
 * @group_id: the "groupId" field in connector's header.
 * @net_role: the "netRole" field in connector's header.
 * @conn: output DPP connector. This buffer should be preallocated.
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_build_signed_connector(struct dpp_configurator *conf,
                                 ECCKeyPair *netAccessKey, char *groupId, enum dpp_netrole netRole,
                                 struct dpp_connector *conn);

/**
 * dpp_parse_signed_connector - Parse Base64URL Signed DPP Connector String
 * Given a 'signed_conn' and the public csign key 'conf', verify the signature and generate the 'conn' object.
 *
 * @signed_conn: the base64url encoded string formatted with (header.jwk.signature).
 * @signed_conn_len: length of signed_conn string.
 * @conf: the public C-Sign Key (Configurator Signing Key) from the configurator.
 * @conn: output DPP connector. This buffer should be preallocated.
 * @out_group_id: output groupId. This value is to be used in finding conf_obj based on groupId.
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_parse_signed_connector(char *signed_conn, INT16U signed_conn_len, struct dpp_configurator *conf, struct dpp_connector *conn, char **out_group_id);

/**
 * dpp_free_connector - Free dynamically allocated variables in 'conn'
 * Run this function to free all content when you no longer need the dpp_connector.
 *
 * @conn: DPP connector. This buffer should be preallocated.
 */
void dpp_free_connector(struct dpp_connector *conn);

/**
 * dpp_free_conf_obj - Free dynamically allocated variables in 'obj'
 * Run this function to free all content when you no longer need the dpp_config_obj.
 *
 * @conn: DPP configuration object. This buffer should be preallocated.
 */
void dpp_free_conf_obj(struct dpp_config_obj *obj);

/**
 * dpp_build_conf_resp - Build DPP Configuration Response
 * Using 'auth', 'obj' and 'obj_count' to generate DPP configuration response message.
 *
 * @auth: DPP authentication struct stored with authentication information
 * @obj: DPP configuration objects buffer stored with credentials
 * @obj_count: number of objects in the 'obj' array
 * @conf_resp_msg: upon successful execution, it stores the address of dynamically generated message buffer
 * @conf_resp_msg_len: upon successful execution, it stores the length of the message buffer
 * @dialog_token: must match the gas initial request
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_build_conf_resp(struct dpp_authentication *auth,
                          struct dpp_config_obj obj[], INT8U obj_count,
                          INT8U **conf_resp_msg, INT32U *conf_resp_msg_len,
                          INT8U dialog_token);

/**
 * dpp_conf_resp_rx - Receive DPP Configuration Response
 * Validate the incoming DPP config req stored in 'buf', by comparing the information stored in 'auth'.
 * It also saves the parsed configuration objects to the preallocated 'obj_out' and 'obj_count_out'.
 *
 * @auth: DPP authentication struct stored with authentication information
 * @obj_out: a preallocated struct array storing output results
 * @obj_count_out: upon successful execution, it stores the number of parsed objects in 'obj_out'
 * @max_obj_count: the maximum size of 'obj_out' array
 * @buf: DPP configuration response message going to parse
 * @len: length of the configuration response message
 * @out_group_id: groupId retrieved
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_conf_resp_rx(struct dpp_authentication *auth,
                       struct dpp_config_obj obj_out[], INT8U *obj_count_out, INT8U max_obj_count,
                       INT8U *buf, INT32U len, char **out_group_id);

/**
 * dpp_build_conf_result - Build DPP Configuration Result
 * Using 'auth', 'status' and 'obj_count' to generate DPP configuration response message.
 *
 * @auth: DPP authentication struct stored with authentication information
 * @status: DPP status (should be DPP_STATUS_OK or DPP_STATUS_CONFIG_REJECTED)
 * @conf_result_msg: upon successful execution, it stores the address of dynamically generated message buffer
 * @conf_result_msg_len: upon successful execution, it stores the length of the message buffer
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_build_conf_result(struct dpp_authentication *auth,
                            enum dpp_status_error      status,
                            INT8U **conf_result_msg, INT32U *conf_result_msg_len);

/**
 * dpp_conf_result_rx - Build DPP Configuration Result
 * Validate the incoming DPP config result stored in 'buf', by comparing the information stored in 'auth'.
 * It only returns success if DPP_STATUS_OK is received.
 *
 * @auth: DPP authentication struct stored with authentication information
 * @buf: DPP configuration result message going to parse
 * @len: length of the configuration result message
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_conf_result_rx(struct dpp_authentication *auth, INT8U *buf, INT32U len);

/**
 * dpp_configurator_sign - Configurator Self-Signing
 * Generate a new configuration object for ourselves, and use the local configurator to sign this new object.
 * To sign the object, it is assumed that the 'auth' must be the configurator, such that the 'auth.conf' has
 * the configurator private key.
 *
 * @auth: DPP authentication struct stored with authentication and configuration information
 * @wifi_tech: the "wi-fi_tech" attribute in newly generated config object
 * @group_id: the "groupId" attribute in newly generated config object
 * @net_role: the "netRole" attribute in newly generated config object
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_configurator_sign(struct dpp_authentication *auth,
                            char *wifi_tech, char *groupId, enum dpp_netrole netRole);

/**
 * dpp_find_config_obj - Find configurator object by groupId and netRole
 * After self signed or received DPP configuration response, 'auth' should have stored some configuration
 * objects in its 'conf_obj' buffer. This function searches and returns the object containing the given
 * 'groupId' and 'netRole'.
 *
 * @auth: DPP authentication struct stored with configuration information
 * @group_id: the "groupId" attribute to be searched
 * @net_role: the "netRole" attribute to be searched
 * Returns: address of the dpp_config_obj on success, NULL on failure
 */
struct dpp_config_obj *dpp_find_config_obj(struct dpp_authentication *auth,
                                           char *wifi_tech, char *groupId, enum dpp_netrole netRole);

/**
 * dpp_build_peer_disc_req - Build DPP Peer Discovery Request
 * Using the information stored in 'conn' to generate DPP peer disc req.
 *
 * @conn: DPP connector struct stored with configuration information
 * @peer_disc_msg: upon successful execution, it stores the address of dynamically generated message buffer
 * @peer_disc_msg_len: upon successful execution, it stores the length of the message buffer
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_build_peer_disc_req(struct dpp_connector *conn,
                              INT8U **peer_disc_msg, INT32U *peer_disc_msg_len);

/**
 * dpp_rx_peer_disc_req - Receive DPP Peer Discovery Request
 * Validate the incoming DPP peer disc req stored in 'buf', by verifying with the csign key in 'conf'.
 *
 * @conf: DPP configurator struct stored with public csign key
 * @conn_out: preallocated buffer to store the parsed DPP connector
 * @buf: DPP peer discovery request message going to parse
 * @len: length of the peer discovery request message
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_rx_peer_disc_req(struct dpp_configurator *conf,
                           struct dpp_connector *   conn_out,
                           INT8U *buf, INT32U len);

/**
 * dpp_peer_intro - Run DPP Peer Introduction
 * Using 'own' connector and 'peer' connector, derive the PMK (Pairwise Master Key) and PMKID.
 *
 * @own: Own connector with private netAccessKey
 * @peer: Peer connector with public netAccessKey
 * @intro: preallocated buffer to store the derived PMK and PMKID.
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_peer_intro(struct dpp_connector *   own,
                     struct dpp_connector *   peer,
                     struct dpp_introduction *intro);

/**
 * dpp_build_peer_disc_resp - Build DPP Peer Discovery Response
 * Using the information stored in 'conn' and 'status' to generate DPP peer disc resp.
 *
 * @conn: DPP connector struct stored with configuration information
 * @status: the DPP status. For success case, DPP_STATUS_OK should be used.
 * @peer_resp_msg: upon successful execution, it stores the address of dynamically generated message buffer
 * @peer_resp_msg_len: upon successful execution, it stores the length of the message buffer
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_build_peer_disc_resp(struct dpp_connector *conn,
                               enum dpp_status_error status,
                               INT8U **peer_resp_msg, INT32U *peer_resp_msg_len);

/**
 * dpp_rx_peer_disc_resp - Receive DPP Peer Discovery Response
 * Validate the incoming DPP peer disc req stored in 'buf', by verifying with the csign key in 'conf'.
 *
 * @conf: DPP configurator struct stored with public csign key
 * @conn_out: preallocated buffer to store the parsed DPP connector
 * @buf: DPP peer discovery response message going to parse
 * @len: length of the peer discovery response message
 * @out_group_id: output groupId. This value is to be used in finding conf_obj based on groupId.
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_rx_peer_disc_resp(struct dpp_configurator *conf,
                            struct dpp_connector    *conn_out,
                            INT8U *buf, INT32U len,
                            char **out_group_id);

/**
 * dpp_build_map_bss_conf_req - Build Multi-AP BSS Configuration Request
 * Similar to dpp_build_conf_req(), this function uses the information stored in 'obj'
 * to generate the "DPP configuration request object attributes" defined in Multi-AP
 * Specifications 3.0.
 *
 * @obj: DPP config request object struct with name, wifi tech, and netrole etc.
 * @conf_req_msg: upon successful execution, it stores the address of dynamically generated message buffer
 * @conf_req_msg_len: upon successful execution, it stores the length of the message buffer
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_build_map_bss_conf_req(struct dpp_config_req_obj *obj,
                                 INT8U **bss_conf_req_msg, INT16U *bss_conf_req_msg_len);

/**
 * dpp_map_bss_conf_req_rx - Receive Multi-AP BSS Configuration Request
 * Similar to dpp_conf_req_rx(), this function validates the incoming DPP config req obj
 * attributes stored in 'buf', and fill the content to 'obj_out' accordingly.
 *
 * @obj_out: upon successful execution, it is updated with the info parsed from 'buf'.
 * @buf: BSS configuration request object attributes message going to parse
 * @len: length of the configuration request message
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_map_bss_conf_req_rx(struct dpp_config_req_obj *obj_out, INT8U *buf, INT16U len);

/**
 * dpp_build_map_bss_conf_resp - Build Multi-AP BSS DPP Configuration Response
 * Similar to dpp_build_conf_resp, this function uses the information stored in 'obj'
 * and 'obj_count' to generate the "DPP configuration object attributes" defined in
 * Multi-AP Specification R3.
 * 
 * @auth: DPP authentication struct stored with authentication information
 * @obj: DPP configuration objects buffer stored with credentials
 * @obj_count: number of objects in the 'obj' array
 * @bss_conf_resp_msg: upon successful execution, it stores the address of dynamically
 *  generated message buffer
 * @bss_conf_resp_msg_len: upon successful execution, it stores the length of the
 *  message buffer
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_build_map_bss_conf_resp(struct dpp_authentication *auth,
                                  struct dpp_config_obj obj[], INT8U obj_count,
                                  INT8U **bss_conf_resp_msg, INT16U *bss_conf_resp_msg_len);

/**
 * dpp_map_bss_conf_resp_rx - Receive Multi-AP BSS DPP Configuration Response
 * Validate the incoming DPP config req stored in 'buf', by comparing the information stored in 'auth'.
 * It also saves the parsed configuration objects to the preallocated 'obj_out' and 'obj_count_out'.
 *
 * @obj_out: a preallocated struct array storing output results
 * @obj_count_out: upon successful execution, it stores the number of parsed objects in 'obj_out'
 * @max_obj_count: the maximum size of 'obj_out' array
 * @buf: BSS DPP configuration response message going to parse
 * @len: length of the configuration response message
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_map_bss_conf_resp_rx(struct dpp_config_obj obj_out[], INT8U *obj_count_out,
                               INT8U max_obj_count, INT8U *buf, INT16U len);

/**
 * dpp_load_creds_info - Retrieve DPP Credentials and store into dpp_auth
 * Load DPP keys and 1905 signed connector from data_model and save them to dpp_auth.
 * The saved DPP credentials are used in peer discovery and network introduction after reinit.
 *
 * @dpp_auth: DPP authentication struct stored with authentication information
 * @csign_key: Public C-sign key from controller
 * @pp_key: Private Protection key from controller
 * @netaccess_key: Private netaccess key of agent itself
 * @signed_connector: Signed connector for 1905 layer
 * Returns: 1 on success
 */
INT8U dpp_load_creds_info(struct dpp_authentication *dpp_auth, char *csign_key, char *pp_key, char *netaccess_key, char *signed_connector);

#endif
