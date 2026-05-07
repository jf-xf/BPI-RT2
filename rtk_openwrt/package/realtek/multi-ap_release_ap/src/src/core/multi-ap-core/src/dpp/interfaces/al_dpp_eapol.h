/*
 *  Multi-AP Device Provisioning Protocol (DPP)
 *  1905 PTK Generation through Extensible Authentication Protocol over LAN (EAPOL)
 *
 *  Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 */

#ifndef _DPP_EAPOL_H_
#define _DPP_EAPOL_H_

#include "al_dpp.h"

/* Macro for struct without padding */
#ifdef __GNUC__
#	define STRUCT_PACKED __attribute__((packed))
#else
#	define STRUCT_PACKED
#endif

/* KDE Related */
#define WLAN_EID_VENDOR_SPECIFIC 0xDD
#define WLAN_EID_RSN 0x30
#define EAPOL_DPP_KDE_DATA_TYPE 0x21
#define EAPOL_DPP_MAX_Z_LEN 66 /* z length for secp521r1 curve */
#define EAPOL_1905_GTK_KDE_DATA_TYPE 0x00
#define EAPOL_RSN_IE_HDR_LEN 4
#define EAPOL_RSN_SELECTOR_LEN 4
#define EAPOL_RSN_VERSION 1

/* IEEE 802.11i */
#define EAPOL_REPLAY_COUNTER_LEN 8
#define EAPOL_NONCE_LEN 32
#define EAPOL_KEY_RSC_LEN 8
#define EAPOL_KEY_DATA_LEN_OCTET_LEN 2

/* IEEE 802.11, 8.5.2 EAPOL-Key frames */
#define EAPOL_KEY_INFO_TYPE_MASK ((INT16U)(BIT(0) | BIT(1) | BIT(2)))
#define EAPOL_KEY_INFO_TYPE_AKM_DEFINED 0
#define EAPOL_KEY_INFO_TYPE_HMAC_MD5_RC4 BIT(0)
#define EAPOL_KEY_INFO_TYPE_HMAC_SHA1_AES BIT(1)
#define EAPOL_KEY_INFO_TYPE_AES_128_CMAC 3
#define EAPOL_KEY_INFO_KEY_INDEX_MASK (BIT(4) | BIT(5))
#define EAPOL_KEY_INFO_KEY_TYPE BIT(3) /* 1 = Pairwise, 0 = Group key */
#define EAPOL_KEY_INFO_INSTALL BIT(6)  /* pairwise */
#define EAPOL_KEY_INFO_ACK BIT(7)
#define EAPOL_KEY_INFO_MIC BIT(8)
#define EAPOL_KEY_INFO_SECURE BIT(9)
#define EAPOL_KEY_INFO_ERROR BIT(10)
#define EAPOL_KEY_INFO_REQUEST BIT(11)
#define EAPOL_KEY_INFO_ENCR_KEY_DATA BIT(12) /* IEEE 802.11i/RSN only */
#define EAPOL_KEY_INFO_SMK_MESSAGE BIT(13)

/* EAPOL Key Length */
#define EAPOL_KEY_MIC_MAX_LEN 32
#define EAPOL_KCK_MAX_LEN 32
#define EAPOL_KEK_MAX_LEN 64
#define EAPOL_TK_MAX_LEN 32
#define EAPOL_ETH_ALEN 6

/* EAPOL Packet Type */
enum { IEEE802_1X_TYPE_EAP_PACKET                   = 0,
	   IEEE802_1X_TYPE_EAPOL_START                  = 1,
	   IEEE802_1X_TYPE_EAPOL_LOGOFF                 = 2,
	   IEEE802_1X_TYPE_EAPOL_KEY                    = 3,
	   IEEE802_1X_TYPE_EAPOL_ENCAPSULATED_ASF_ALERT = 4,
	   IEEE802_1X_TYPE_EAPOL_MKA                    = 5,
};

/* Key Descriptor Type */
enum { EAPOL_KEY_TYPE_RC4 = 1,
	   EAPOL_KEY_TYPE_RSN = 2 };

/* EAPOL State Machine Return Values */
#define EAPOL_SM_FAIL 0
#define EAPOL_SM_OK 1
#define EAPOL_SM_OK_TX 2

/* IEEE 802.1x Authentication Header */
struct ieee802_1x_hdr {
	INT8U  version;
	INT8U  type;
	INT16U length;
	/* followed by length octets of data */
} STRUCT_PACKED;

/* EAPOL Key Frame */
struct dpp_eapol_key {
	INT8U type;
	/* Note: key_info, key_length, and key_data_length are unaligned */
	INT8U key_info[2];   /* big endian */
	INT8U key_length[2]; /* big endian */
	INT8U replay_counter[EAPOL_REPLAY_COUNTER_LEN];
	INT8U key_nonce[EAPOL_NONCE_LEN];
	INT8U key_iv[16];
	INT8U key_rsc[EAPOL_KEY_RSC_LEN];
	INT8U key_id[8]; /* Reserved in IEEE 802.11i/RSN */
	                 /* variable length Key MIC field */
	                 /* big endian 2-octet Key Data Length field */
	                 /* followed by Key Data Length bytes of Key Data */
} STRUCT_PACKED;

struct dpp_ptk {
	INT8U  kck[EAPOL_KCK_MAX_LEN]; /* EAPOL-Key Key Confirmation Key (KCK) */
	INT8U  kek[EAPOL_KEK_MAX_LEN]; /* EAPOL-Key Key Encryption Key (KEK) */
	INT8U  tk[EAPOL_TK_MAX_LEN];   /* Temporal Key (TK) */
	INT32U kck_len;
	INT32U kek_len;
	INT32U tk_len;
};

struct dpp_gtk {
	INT8U gtk[SHA512_MAC_LEN];
	INT8U gtk_len;
};

struct dpp_eapol_params {
	const INT32U pmk_len;
	const INT32U kck_len;
	const INT32U kek_len;
	const INT32U tk_len;
	const INT32U mic_len;
	const INT32U gtk_len;
};

struct dpp_eapol_sm {

	struct dpp_introduction intro;
	struct dpp_ptk          ptk;
	struct dpp_gtk          gtk;

	const struct dpp_eapol_params *params;

	INT8U own_addr[EAPOL_ETH_ALEN];
	INT8U peer_addr[EAPOL_ETH_ALEN];

	INT8U ANonce[EAPOL_NONCE_LEN];
	INT8U SNonce[EAPOL_NONCE_LEN];

	INT8U replay_counter[EAPOL_REPLAY_COUNTER_LEN];

	enum dpp_eapol_role {
		AUTHENTICATOR,
		SUPPLICANT
	} role;

	enum {
		DISABLED = 0,
		STARTED,
		FINISHED
	} state;
};

/**
 * dpp_eapol_gtk_init - Initialize Group Temporal Key
 * Given the length of PMK (Pairwise Master Key), this function finds the
 * appropriate size of GTK, and randomly generates one to the 'gtk' buffer.
 *
 * @gtk: a preallocated buffer of GTK
 * @pmk_len: PMK length in bytes
 *
 * Returns: EAPOL_SM_OK on success, EAPOL_SM_FAIL on failure
 */
INT8U dpp_eapol_gtk_init(struct dpp_gtk *gtk, INT32U pmk_len);

/**
 * dpp_eapol_sm_init - Initialize DPP EAPOL State Machine
 * This function resets all the parameters in 'sm', and replaces them with the
 * parameters stored in 'intro' and 'gtk'. After reset, the state machine will
 * be in the 'DISABLED' state.
 *
 * To start forging and parsing of EAPOL frames, you should run the
 * 'dpp_eapol_sm_start()' to start the first step of the state machine.
 *
 * @sm: the state machine to be initialized
 * @intro: the PMK and PMKID generated from DPP Network Introduction (Peer Discovery)
 * @gtk: the group temporal key. Only authenticator needs to supply this
 * @role: role in the EAPOL - either authenticator or supplicant
 * @own_addr: MAC address of own interface (6 bytes) for generating PTK
 * @peer_addr: MAC address of peer's interface (6 bytes) for generating PTK
 *
 * Returns: EAPOL_SM_OK on success, EAPOL_SM_FAIL on failure
 */
INT8U dpp_eapol_sm_init(struct dpp_eapol_sm *    sm,
                        struct dpp_introduction *intro,
                        struct dpp_gtk *         gtk,
                        enum dpp_eapol_role      role,
                        INT8U *own_addr, INT8U *peer_addr);

/**
 * dpp_eapol_sm_start - Start DPP EAPOL State Machine
 * This function change the state of the state machine to be 'STARTED', such
 * that it can parse and forge packets.
 *
 * For EAPOL authenticator, this function also generates EAPOL 1/4 message to
 * the 'tx_buf' and returns EAPOL_SM_OK_TX to indicate that the TX buffer is
 * filled with a new EAPOL frame.
 *
 * @sm: the state machine
 * @tx_buf: it outputs the address of a dynamically generated message buffer
 * @tx_len: it outputs the length of the message buffer
 *
 * Returns: EAPOL_SM_OK on success without TX message,
 * EAPOL_SM_OK_TX on success with new TX message generated,
 * EAPOL_SM_FAIL on failure.
 */
INT8U dpp_eapol_sm_start(struct dpp_eapol_sm *sm, INT8U **tx_buf, INT32U *tx_len);

/**
 * dpp_eapol_sm_rx - Parser of incoming EAPOL Frames
 * This function acts as the ticker of the state machine. By feeding it with
 * the received EAPOL messages, it parses the content and forges new EAPOL responds.
 *
 * @sm: the state machine
 * @rx_buf: input of the received EAPOL frame
 * @rx_len: length of the inputted EAPOL frame
 * @tx_buf: it outputs the address of a dynamically generated message buffer
 * @tx_len: it outputs the length of the message buffer
 *
 * Returns: EAPOL_SM_OK on success without TX message,
 * EAPOL_SM_OK_TX on success with new TX message generated,
 * EAPOL_SM_FAIL on failure.
 */
INT8U dpp_eapol_sm_rx(struct dpp_eapol_sm *sm,
                      INT8U *rx_buf, INT32U rx_len, INT8U **tx_buf, INT32U *tx_len);

#endif