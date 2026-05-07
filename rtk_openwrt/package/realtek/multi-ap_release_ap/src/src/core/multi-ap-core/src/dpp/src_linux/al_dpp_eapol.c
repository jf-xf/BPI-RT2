/*
 *  Multi-AP Device Provisioning Protocol (DPP)
 *  1905 PTK Generation through Extensible Authentication Protocol over LAN (EAPOL)
 *
 *  Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 */

#include "platform.h"
#include "packet_tools.h"

#include "al_dpp.h"
#include "al_dpp_eapol.h"
#include "al_dpp_eapol_prf.h"

#include "platform_crypto.h"
#include "platform_crypto_aes.h"

/*
 * dpp_eapol_lengths - parameters following Easy Connect Spec 2.0
 *                     Table 3 and Chapter 8.4.2.
 *
 * TK Length is not discussed elsewhere, but Multi-AP Spec 3.0 mentioned
 * that the 256-bit curve should generate TK sized EAPOL_TK_MAX_LEN.
 * Hence, we assume other curves also use EAPOL_TK_MAX_LEN.
 *
 * For GTK Length, we assume the key size is the same as the PMK size.
 * This aligns Multi-AP Spec 3.0 that when prime256v1 is used, the GTK
 * should be lengthed 256-bit.
 */
static const struct dpp_eapol_params dpp_eapol_lengths[] = {
	{ 32, 16, 16, EAPOL_TK_MAX_LEN, 16, 32 }, // 256 bits
	{ 48, 24, 32, EAPOL_TK_MAX_LEN, 24, 48 }, // 384 bits
	{ 64, 32, 32, EAPOL_TK_MAX_LEN, 32, 64 }, // 512 bits
	{ 0, 0, 0, 0, 0, 0 }
};

// Given the PMK length, get other DPP EAPOL related parameters
static const struct dpp_eapol_params *dpp_eapol_get_params(INT32U pmk_len)
{
	int i;

	for (i = 0; dpp_eapol_lengths[i].pmk_len != 0; i++) {
		if (pmk_len == dpp_eapol_lengths[i].pmk_len)
			return &dpp_eapol_lengths[i];
	}

	PLATFORM_PRINTF_WARNING("DPP EAPOL: cannot get param from pmk_len\n");
	return NULL;
}

// Generate a MIC (Message Integrity Code) from EAPOL frame in hdr
static INT8U dpp_eapol_key_mic(INT8U *kck, INT32U kck_len, INT8U *hdr, INT32U hdr_len, INT8U *key_mic)
{
	INT8U  hash[SHA512_MAC_LEN];
	INT8U *addr[1];
	INT32U len[1];

	INT8U ret = 0;

	addr[0] = hdr;
	len[0]  = hdr_len;

	if (kck_len == 16) {
		ret = PLATFORM_HMAC_SHA256(kck, kck_len, 1, addr, len, hash);
	} else if (kck_len == 24) {
		ret = PLATFORM_HMAC_SHA384(kck, kck_len, 1, addr, len, hash);
	} else if (kck_len == 32) {
		ret = PLATFORM_HMAC_SHA512(kck, kck_len, 1, addr, len, hash);
	} else {
		PLATFORM_PRINTF_INFO("DPP EAPOL: Unsupported KCK Length %d\n", kck_len);
		return 0;
	}

	if (ret != 1) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: MIC generation failed\n");
		return 0;
	}

	PLATFORM_MEMCPY(key_mic, hash, kck_len);
	return 1;
}

// Verify the MIC (Message Integrity Code) of the EAPOL frame in buf
static INT8U dpp_eapol_verify_eapol_key_mic(struct dpp_eapol_sm * sm,
                                            struct dpp_eapol_key *key, INT8U *buf, INT8U len)
{
	INT8U mic[EAPOL_KEY_MIC_MAX_LEN] = { 0 };

	INT32U mic_len = sm->params->mic_len;

	PLATFORM_MEMCPY(mic, key + 1, mic_len);
	PLATFORM_MEMSET(key + 1, 0, mic_len);

	if (dpp_eapol_key_mic(sm->ptk.kck, sm->ptk.kck_len, buf, len, (INT8U *)(key + 1)) != 1) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: MIC calculation failed\n");
		return 0;
	}

	if (PLATFORM_MEMCMP(mic, key + 1, mic_len) != 0) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: Invalid EAPOL Key MIC\n");
		return 0;
	}

	return 1;
}

// Use PRF to convert PMK to PTK (Pairwise Transient Key)
static INT8U dpp_pmk_to_ptk(INT8U *pmk, const char *label,
                            INT8U *addr1, INT8U *addr2,
                            INT8U *nonce1, INT8U *nonce2,
                            INT8U *z, INT32U z_len,
                            const struct dpp_eapol_params *params, struct dpp_ptk *ptk)
{
	INT8U  data[2 * EAPOL_ETH_ALEN + 2 * EAPOL_NONCE_LEN + EAPOL_DPP_MAX_Z_LEN];
	INT32U data_len = 2 * EAPOL_ETH_ALEN + 2 * EAPOL_NONCE_LEN;

	INT8U  tmp[EAPOL_KCK_MAX_LEN + EAPOL_KEK_MAX_LEN + EAPOL_TK_MAX_LEN];
	INT32U ptk_len;

	// sanity check
	if (!params || !ptk || z_len > EAPOL_DPP_MAX_Z_LEN) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: Invalid input for PTK derivation\n");
		return 0;
	}

	// Min(AA, SA) || Max(AA, SA)
	if (memcmp(addr1, addr2, EAPOL_ETH_ALEN) < 0) {
		PLATFORM_MEMCPY(data, addr1, EAPOL_ETH_ALEN);
		PLATFORM_MEMCPY(data + EAPOL_ETH_ALEN, addr2, EAPOL_ETH_ALEN);
	} else {
		PLATFORM_MEMCPY(data, addr2, EAPOL_ETH_ALEN);
		PLATFORM_MEMCPY(data + EAPOL_ETH_ALEN, addr1, EAPOL_ETH_ALEN);
	}

	PLATFORM_HEXDUMP("A1", data, EAPOL_ETH_ALEN);
	PLATFORM_HEXDUMP("A2", data + EAPOL_ETH_ALEN, EAPOL_ETH_ALEN);

	// Min(ANonce, SNonce) || Max(ANonce, SNonce)
	if (memcmp(nonce1, nonce2, EAPOL_NONCE_LEN) < 0) {
		PLATFORM_MEMCPY(data + 2 * EAPOL_ETH_ALEN, nonce1, EAPOL_NONCE_LEN);
		PLATFORM_MEMCPY(data + 2 * EAPOL_ETH_ALEN + EAPOL_NONCE_LEN, nonce2, EAPOL_NONCE_LEN);
	} else {
		PLATFORM_MEMCPY(data + 2 * EAPOL_ETH_ALEN, nonce2, EAPOL_NONCE_LEN);
		PLATFORM_MEMCPY(data + 2 * EAPOL_ETH_ALEN + EAPOL_NONCE_LEN, nonce1, EAPOL_NONCE_LEN);
	}

	PLATFORM_HEXDUMP("Nonce1", data + 2 * EAPOL_ETH_ALEN, EAPOL_NONCE_LEN);
	PLATFORM_HEXDUMP("Nonce2", data + 2 * EAPOL_ETH_ALEN + EAPOL_NONCE_LEN, EAPOL_NONCE_LEN);

	// (Optional) Z.x (for Perfect Forward Secrecy)
	if (z && z_len) {
		PLATFORM_MEMCPY(data + 2 * EAPOL_ETH_ALEN + 2 * EAPOL_NONCE_LEN, z, z_len);
		data_len += z_len;
	}

	// Run PRF to get PTK
	ptk->kck_len = params->kck_len;
	ptk->kek_len = params->kek_len;
	ptk->tk_len  = params->tk_len;
	ptk_len      = ptk->kck_len + ptk->kek_len + ptk->tk_len;
	if (dpp_ieee_prf_bits(params->pmk_len,
	                      pmk, params->pmk_len, label, data, data_len, tmp, ptk_len * 8)
	    != 1) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: PTK derivation failed\n");
		PLATFORM_MEMSET(ptk, 0, sizeof(struct dpp_ptk));
		return 0;
	}

	PLATFORM_MEMCPY(ptk->kck, tmp, ptk->kck_len);
	PLATFORM_MEMCPY(ptk->kek, tmp + ptk->kck_len, ptk->kek_len);
	PLATFORM_MEMCPY(ptk->tk, tmp + ptk->kck_len + ptk->kek_len, ptk->tk_len);

	PLATFORM_HEXDUMP("PMK", pmk, params->pmk_len);
	PLATFORM_HEXDUMP("PTK", tmp, ptk_len);
	PLATFORM_HEXDUMP("KCK", ptk->kck, ptk->kck_len);
	PLATFORM_HEXDUMP("KEK", ptk->kek, ptk->kek_len);
	PLATFORM_HEXDUMP("TK", ptk->tk, ptk->tk_len);

	PLATFORM_MEMSET(tmp, 0, ptk_len);
	return 1;
}

// Increment arbitrary length byte array (e.g. replay counter) by one
static void dpp_eapol_inc_byte_array(INT8U *counter, size_t len)
{
	int pos = len - 1;
	while (pos >= 0) {
		counter[pos]++;
		if (counter[pos] != 0)
			break;
		pos--;
	}
}

// Build 802.1X EAPOL Frame
static INT8U dpp_eapol_build(INT16U key_info,
                             INT8U *nonce,
                             INT8U *replay_counter,
                             INT8U *kde, INT16U kde_len,
                             const struct dpp_eapol_params *params, struct dpp_ptk *ptk,
                             INT8U **eapol_out, INT32U *eapol_out_len)
{
	struct ieee802_1x_hdr *hdr;
	struct dpp_eapol_key * key;
	size_t                 len, keyhdrlen;

	INT8U *key_mic, *key_data, *kde_block = NULL;
	int    key_data_len, pad_len;

	INT16U aux16;
	INT8U *p;

	// Determine KDE size
	key_data_len = kde_len;
	if ((key_info & EAPOL_KEY_INFO_SECURE) && kde_len && kde) {
		pad_len = key_data_len % 8;
		key_data_len += pad_len ? (8 - pad_len) : 0; // pad to fit one block
		key_data_len += 8;                           // extra block for AES-Wrap
		kde_block = PLATFORM_MALLOC(key_data_len);   // blocked memory for encryption
		PLATFORM_MEMSET(kde_block, 0, key_data_len);
		PLATFORM_MEMCPY(kde_block, kde, kde_len);
	}

	// Determine Pointers
	keyhdrlen = sizeof(struct dpp_eapol_key) + params->mic_len + EAPOL_KEY_DATA_LEN_OCTET_LEN;
	len       = sizeof(struct ieee802_1x_hdr) + keyhdrlen;
	len += key_data_len;

	// Allocate Memory
	hdr      = PLATFORM_MALLOC(len);
	key      = (struct dpp_eapol_key *)(hdr + 1);
	key_mic  = (INT8U *)(key + 1);
	key_data = ((INT8U *)(hdr + 1)) + keyhdrlen;
	PLATFORM_MEMSET(hdr, 0, len);

	// Header
	hdr->version = 1;
	hdr->type    = IEEE802_1X_TYPE_EAPOL_KEY;

	// Length
	p     = (INT8U *)&hdr->length;
	aux16 = len - sizeof(struct ieee802_1x_hdr);
	_I2B(&aux16, &p);

	// Force version to be WPA_KEY_INFO_TYPE_AKM_DEFINED
	key_info &= ~EAPOL_KEY_INFO_TYPE_MASK;
	key_info |= EAPOL_KEY_INFO_TYPE_AKM_DEFINED;

	// Key Descriptor Type
	key->type = EAPOL_KEY_TYPE_RSN;

	// Key Information Octet
	p     = key->key_info;
	aux16 = key_info;
	_I2B(&aux16, &p);

	// Key Length
	p     = key->key_length;
	aux16 = 32;
	_I2B(&aux16, &p);

	// Key Replay Counter
	PLATFORM_MEMCPY(key->replay_counter, replay_counter, EAPOL_REPLAY_COUNTER_LEN);

	// Key Nonce
	if (nonce)
		PLATFORM_MEMCPY(key->key_nonce, nonce, EAPOL_NONCE_LEN);

	// Key Data Encapsulation Length (KDE Length)
	p     = key_mic + params->mic_len;
	aux16 = key_data_len;
	_I2B(&aux16, &p);

	// Key Data Encapsulation (KDE)
	if (kde) {
		if (key_info & EAPOL_KEY_INFO_SECURE) {
			if (PLATFORM_AES_WRAP(ptk->kek, ptk->kek_len,
			                      kde_block, (key_data_len - 8) / 8,
			                      key_data)
			    != 1) {
				PLATFORM_PRINTF_WARNING("DPP EAPOL: KDE wrap failed\n");
				goto fail;
			}
			PLATFORM_PRINTF_INFO("DPP EAPOL: ciphertext %d bytes\n", key_data_len);
			PLATFORM_SAFE_FREE((void **)&kde_block);
		} else {
			PLATFORM_MEMCPY(key_data, kde, key_data_len);
		}
	}

	PLATFORM_HEXDUMP("KCK", ptk->kck, ptk->kck_len);

	// Message Integrity Code (MIC)
	if ((key_info & EAPOL_KEY_INFO_MIC)) {
		if (dpp_eapol_key_mic(ptk->kck, ptk->kck_len, (INT8U *)hdr, len, key_mic) != 1) {
			PLATFORM_PRINTF_WARNING("DPP EAPOL: MIC generation failed\n");
			goto fail;
		}
	}

	PLATFORM_HEXDUMP("MIC", key_mic, 16);

	PLATFORM_HEXDUMP("EAPOL", (INT8U *)hdr, len);

	*eapol_out     = (INT8U *)hdr;
	*eapol_out_len = len;
	return 1;

fail:
	PLATFORM_FREE(hdr);
	PLATFORM_FREE(kde_block);
	return 0;
}

// Decrypt the KDE (Key Data Encapsulation)
static INT8U dpp_eapol_decrypt_key_data(struct dpp_eapol_sm * sm,
                                        struct dpp_eapol_key *key,
                                        INT8U *key_data, INT16U *key_data_len)
{
	INT16U aux16;
	INT8U *buf, *p, ret = 0;

	if (*key_data_len < 8 || *key_data_len % 8) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: Unsupported AES-UNWRAP length\n");
		return 0;
	}

	PLATFORM_HEXDUMP("KEK", sm->ptk.kek, sm->ptk.kek_len);
	PLATFORM_HEXDUMP("CIPHERTEXT", key_data, *key_data_len);

	// decrypted content is 1 block (8 bytes) less
	*key_data_len -= 8;
	buf = PLATFORM_MALLOC(*key_data_len);

	if (PLATFORM_AES_UNWRAP(sm->ptk.kek, sm->ptk.kek_len,
	                        key_data, *key_data_len / 8, buf)
	    != 1) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: AES-UNWRAP failed\n");
		goto fail;
	}

	// Replace KDE data with decrypted data
	PLATFORM_MEMCPY(key_data, buf, *key_data_len);

	// Replace KDE length with decrypted length
	p     = ((INT8U *)(key + 1)) + sm->params->mic_len;
	aux16 = *key_data_len;
	_I2B(&aux16, &p);

	PLATFORM_HEXDUMP("CLEARTEXT", key_data, *key_data_len);

	PLATFORM_PRINTF_INFO("DPP EAPOL: cleartext %d bytes\n", *key_data_len);
	ret = 1;

fail:
	PLATFORM_FREE(buf);
	return ret;
}

// Locate specific information element (IE) from the given KDE buffer
static INT8U *dpp_eapol_get_kde_ie(INT8U *buf, INT32U len, INT8U req_id, INT32U oui, INT8U type, INT8U *ret_len)
{
	INT8U *pos = buf, *end = buf + len;
	INT8U  ie_id, ie_len, *ie_ptr;

	while (end - pos > 1) {
		ie_id  = pos[0];
		ie_len = pos[1];
		ie_ptr = pos + 2;

		// ignore padding
		if (ie_id == 0xdd && ((pos == end - 1) || ie_len == 0))
			break;

		// further check if type match and large enough to put OUI + data type
		if (req_id == ie_id && ie_len > 4) {
			if ((ie_ptr[0] == ((oui >> 16) & 0xFF))
			    && (ie_ptr[1] == ((oui >> 8) & 0xFF))
			    && (ie_ptr[2] == (oui & 0xFF))
			    && (ie_ptr[3] == type)) {
				*ret_len = ie_len;
				return ie_ptr;
			}
		}
		pos = ie_ptr + ie_len;
	}
	return NULL;
}

// Generate DPP KDE to a dynamically allocated buffer
static INT32U dpp_eapol_gen_dpp_kde_ie(INT8U **buf_out)
{
	INT32U kde_len;
	INT8U *kde, *p, aux8;

	/* Build DPP KDE: Determine KDE Length */
	kde_len = 2;  // Type, Length
	kde_len += 3; // Wi-Fi Alliance OUI
	kde_len += 1; // Data Type
	kde_len += 1; // Protocol Version
	kde_len += 1; // Flags (PFS requirements)

	/* Allocate Memory */
	kde = PLATFORM_MALLOC(kde_len);
	p   = kde;

	/* DPP KDE: Type */
	aux8 = WLAN_EID_VENDOR_SPECIFIC;
	_I1B(&aux8, &p);

	/* DPP KDE: Length */
	aux8 = 6;
	_I1B(&aux8, &p);

	/* DPP KDE: OUI */
	aux8 = (OUI_WFA >> 16) & 0xFF;
	_I1B(&aux8, &p);
	aux8 = (OUI_WFA >> 8) & 0xFF;
	_I1B(&aux8, &p);
	aux8 = OUI_WFA & 0xFF;
	_I1B(&aux8, &p);

	/* DPP KDE: Data Type */
	aux8 = EAPOL_DPP_KDE_DATA_TYPE;
	_I1B(&aux8, &p);

	/* DPP KDE: Protocol Version */
	aux8 = DPP_VERSION;
	_I1B(&aux8, &p);

	/* DPP KDE: Flags */
	aux8 = 0; // PFS not supported
	_I1B(&aux8, &p);

	*buf_out = kde;
	return kde_len;
}

// Check the validity of the content in DPP KDE IE
static INT8U dpp_eapol_parse_dpp_kde_ie(INT8U *dpp_kde, INT8U kde_len)
{
	if (kde_len != 6) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: DPP KDE invalid length!\n");
		return 0;
	}

	/* Check Protocol Version */
	if (dpp_kde[4] > DPP_VERSION) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: DPP Version higher than expected\n");
		return 0;
	}

	/* Check DPP KDE Flags */
	if (dpp_kde[5] != 0x00) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: No PFS support currently\n");
		return 1;
	}

	return 1;
}

// Generate 1905 GTK KDE to a dynamically allocated buffer
static INT32U dpp_eapol_gen_1905_gtk_kde_ie(INT8U **buf_out, INT8U *gtk, INT8U gtk_len)
{
	INT32U kde_len;
	INT8U *kde, *p, aux8;

	/* Build GTK KDE: Determine KDE Length */
	kde_len = 2;        // Type, Length
	kde_len += 3;       // IEEE OUI
	kde_len += 1;       // Data Type
	kde_len += 1;       // Key ID, Reserved
	kde_len += gtk_len; // GTK Length

	/* Allocate Memory */
	kde = PLATFORM_MALLOC(kde_len);
	p   = kde;

	/* GTK KDE: Type */
	aux8 = WLAN_EID_VENDOR_SPECIFIC;
	_I1B(&aux8, &p);

	/* GTK KDE: Length */
	aux8 = 5 + gtk_len;
	_I1B(&aux8, &p);

	/* GTK KDE: OUI */
	aux8 = (OUI_WFA >> 16) & 0xFF;
	_I1B(&aux8, &p);
	aux8 = (OUI_WFA >> 8) & 0xFF;
	_I1B(&aux8, &p);
	aux8 = OUI_WFA & 0xFF;
	_I1B(&aux8, &p);

	/* GTK KDE: Data Type */
	aux8 = EAPOL_1905_GTK_KDE_DATA_TYPE;
	_I1B(&aux8, &p);

	/* GTK KDE: Key ID and Reserved */
	aux8 = 0x01;
	_I1B(&aux8, &p);

	/* GTK KDE: 1905 GTK */
	_InB(gtk, &p, gtk_len);

	*buf_out = kde;
	return kde_len;
}

// Check the validity of the content in GTK KDE IE
static INT8U dpp_eapol_parse_1905_gtk_kde_ie(INT8U *gtk_kde, INT8U kde_len,
                                             INT8U **gtk_out, INT8U *gtk_len_out)
{
	INT8U gtk_len;

	/* Check Length */
	if (kde_len < 5) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: KDE Length too short\n");
		return 0;
	}
	gtk_len = kde_len - 5;

	/* Return the GTK pointer */
	*gtk_out     = &gtk_kde[5];
	*gtk_len_out = gtk_len;
	return 1;
}

// Generate Robust Security Network Element (RSNE) to a dynamically allocated buffer
static INT32U dpp_eapol_gen_rsn_ie(INT8U **buf_out, INT8U *pairwise_cipher,
                                   INT8U *akm, INT8U *group_cipher,
                                   INT8U *pmkid, INT8U *group_mgmt_cipher)
{
	INT32U rsne_len;
	INT16U aux16;
	INT8U *kde, *p, aux8;

	/* Build RSNE: Determine KDE Length */
	rsne_len = EAPOL_RSN_IE_HDR_LEN;
	rsne_len += EAPOL_RSN_SELECTOR_LEN;                           // Group Cipher
	rsne_len += 2 + EAPOL_RSN_SELECTOR_LEN;                       // Pairwise Cipher
	rsne_len += 2 + EAPOL_RSN_SELECTOR_LEN;                       // AKM
	rsne_len += 2;                                                // RSN Capabilities
	rsne_len += 2 + PMKID_LEN;                                    // PMKID
	rsne_len += (group_mgmt_cipher) ? EAPOL_RSN_SELECTOR_LEN : 0; // Group Management Cipher

	/* Allocate Memory */
	kde = PLATFORM_MALLOC(rsne_len);
	p   = kde;

	/* RSN Header: Type */
	aux8 = WLAN_EID_RSN;
	_I1B(&aux8, &p);

	/* RSN Header: Length */
	aux8 = rsne_len - 2;
	_I1B(&aux8, &p);

	/* RSN Header: Version */
	aux16 = 1;
	_I2B_LE(&aux16, &p);

	/* RSN: Group Cipher */
	_InB(group_cipher, &p, EAPOL_RSN_SELECTOR_LEN);

	/* RSN: Pairwise Cipher */
	aux16 = 1;
	_I2B_LE(&aux16, &p);
	_InB(pairwise_cipher, &p, EAPOL_RSN_SELECTOR_LEN);

	/* RSN: AKM */
	aux16 = 1;
	_I2B_LE(&aux16, &p);
	_InB(akm, &p, EAPOL_RSN_SELECTOR_LEN);

	/* RSN: RSN Capabilities */
	aux16 = 0; // all not supported
	_I2B_LE(&aux16, &p);

	/* RSN: PMKID */
	aux16 = 1;
	_I2B_LE(&aux16, &p);
	_InB(pmkid, &p, PMKID_LEN);

	/* RSN: (Optional) Group Management Cipher */
	if (group_mgmt_cipher) {
		_InB(group_mgmt_cipher, &p, EAPOL_RSN_SELECTOR_LEN);
	}

	*buf_out = kde;
	return rsne_len;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// State Machine Logic
///////////////////////////////////////////////////////////////////////////////////////////////////
static INT8U dpp_eapol_authenticator_send_1_of_4(struct dpp_eapol_sm *sm, INT8U **tx_buf, INT32U *tx_len)
{
	/* Generate Anonce */
	if (PLATFORM_GET_RANDOM_BYTES(sm->ANonce, EAPOL_NONCE_LEN) != 1) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: Anonce generation failed\n");
		return EAPOL_SM_FAIL;
	}

	/* Build EAPOL-1 Frame */
	INT16U key_info = EAPOL_KEY_INFO_ACK | EAPOL_KEY_INFO_KEY_TYPE;
	dpp_eapol_inc_byte_array(sm->replay_counter, EAPOL_REPLAY_COUNTER_LEN);
	if (dpp_eapol_build(key_info, sm->ANonce, sm->replay_counter, NULL, 0,
	                    sm->params, &sm->ptk, tx_buf, tx_len)
	    != 1) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: EAPOL-1 forging failed\n");
		return EAPOL_SM_FAIL;
	}

	PLATFORM_PRINTF_INFO("DPP EAPOL: Authenticator - Forged 1/4\n");
	return EAPOL_SM_OK_TX;
}

static INT8U dpp_eapol_supplicant_process_1_of_4(struct dpp_eapol_sm *sm, INT16U key_info,
                                                 struct dpp_eapol_key *key)
{
	const INT8U  zeros[EAPOL_NONCE_LEN] = { 0 };
	const INT16U expected_key_info      = EAPOL_KEY_INFO_ACK | EAPOL_KEY_INFO_KEY_TYPE;

	PLATFORM_PRINTF_INFO("DPP EAPOL: Supplicant - Process 1/4\n");

	if (key_info != expected_key_info) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: key info is not 1/4");
		return EAPOL_SM_FAIL;
	}

	/* Process ANonce */
	if (PLATFORM_MEMCMP(key->key_nonce, zeros, EAPOL_NONCE_LEN) == 0) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: nonce is empty\n");
		return EAPOL_SM_FAIL;
	}
	PLATFORM_MEMCPY(sm->ANonce, key->key_nonce, EAPOL_NONCE_LEN);

	/* Generate SNonce */
	if (PLATFORM_GET_RANDOM_BYTES(sm->SNonce, EAPOL_NONCE_LEN) != 1) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: Snonce generation failed\n");
		return EAPOL_SM_FAIL;
	}

	/* Derive PTK */
	if (dpp_pmk_to_ptk(sm->intro.pmk, "Pairwise key expansion",
	                   sm->own_addr, sm->peer_addr,
	                   sm->ANonce, sm->SNonce, NULL, 0, sm->params, &sm->ptk)
	    != 1) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: cannot derive PTK\n");
		return EAPOL_SM_FAIL;
	}

	/* Save Replay Counter */
	PLATFORM_MEMCPY(sm->replay_counter, key->replay_counter, EAPOL_REPLAY_COUNTER_LEN);

	return EAPOL_SM_OK;
}

static INT8U dpp_eapol_supplicant_send_2_of_4(struct dpp_eapol_sm *sm, INT8U **tx_buf, INT32U *tx_len)
{
	INT8U *kde = NULL, *dpp_kde = NULL, *rsn_kde = NULL;
	INT8U  kde_len, dpp_kde_len, rsn_kde_len;
	INT16U key_info;
	INT8U  ret = EAPOL_SM_OK_TX;

	/* Hard-coded Values for RSNE */
	INT8U pairwise_cipher[] = { 0x00, 0x0f, 0xac, 0x04 }; // IEEE AES CCM
	INT8U akm[]             = { 0x50, 0x6f, 0x9a, 0x02 }; // WFA PSK
	INT8U group_cipher[]    = { 0x00, 0x0f, 0xac, 0x06 }; // IEEE BIP 128

	/* Build KDEs */
	dpp_kde_len = dpp_eapol_gen_dpp_kde_ie(&dpp_kde);
	rsn_kde_len = dpp_eapol_gen_rsn_ie(&rsn_kde, pairwise_cipher,
	                                   akm, group_cipher,
	                                   sm->intro.pmkid, NULL);
	kde_len     = dpp_kde_len + rsn_kde_len;
	kde         = PLATFORM_MALLOC(kde_len);
	PLATFORM_MEMCPY(kde, dpp_kde, dpp_kde_len);
	PLATFORM_MEMCPY(kde + dpp_kde_len, rsn_kde, rsn_kde_len);

	/* Build EAPOL 2/4 */
	key_info = EAPOL_KEY_INFO_MIC | EAPOL_KEY_INFO_KEY_TYPE;
	if (dpp_eapol_build(key_info, sm->SNonce, sm->replay_counter,
	                    kde, kde_len, sm->params, &sm->ptk, tx_buf, tx_len)
	    != 1) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: EAPOL-2 forging failed\n");
		ret = EAPOL_SM_FAIL;
		goto fail;
	}

	PLATFORM_PRINTF_INFO("DPP EAPOL: Supplicant - Forged 2/4\n");
fail:
	PLATFORM_FREE(kde);
	PLATFORM_FREE(dpp_kde);
	PLATFORM_FREE(rsn_kde);
	return ret;
}

static INT8U dpp_eapol_authenticator_process_2_of_4(struct dpp_eapol_sm * sm,
                                                    struct dpp_eapol_key *key, INT8U *kde, INT32U kde_len)
{
	INT8U *     dpp_kde, dpp_kde_len = 0;
	const INT8U zeros[EAPOL_NONCE_LEN] = { 0 };

	PLATFORM_PRINTF_INFO("DPP EAPOL: Authenticator - Process 2/4\n");

	/* Get DPP KDE */
	dpp_kde = dpp_eapol_get_kde_ie(kde, kde_len,
	                               WLAN_EID_VENDOR_SPECIFIC,
	                               OUI_WFA, EAPOL_DPP_KDE_DATA_TYPE, &dpp_kde_len);
	if (!dpp_kde) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: DPP KDE missing!\n");
	} else {
		/* Check DPP KDE */
		if (dpp_eapol_parse_dpp_kde_ie(dpp_kde, dpp_kde_len) != 1) {
			PLATFORM_PRINTF_WARNING("DPP EAPOL: invalid DPP KDE!\n");
			return EAPOL_SM_FAIL;
		}
	}

	/* Check SNonce */
	if (PLATFORM_MEMCMP(key->key_nonce, zeros, EAPOL_NONCE_LEN) == 0) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: nonce is empty\n");
		return EAPOL_SM_FAIL;
	}
	PLATFORM_MEMCPY(sm->SNonce, key->key_nonce, EAPOL_NONCE_LEN);

	/* Derive PTK */
	if (dpp_pmk_to_ptk(sm->intro.pmk, "Pairwise key expansion",
	                   sm->own_addr, sm->peer_addr,
	                   sm->ANonce, sm->SNonce, NULL, 0, sm->params, &sm->ptk)
	    != 1) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: cannot derive PTK\n");
		return EAPOL_SM_FAIL;
	}

	return EAPOL_SM_OK;
}

static INT8U dpp_eapol_authenticator_send_3_of_4(struct dpp_eapol_sm *sm, INT8U **tx_buf, INT32U *tx_len)
{
	INT8U *kde = NULL, *dpp_kde = NULL, *gtk_kde = NULL;
	INT8U  kde_len, dpp_kde_len, gtk_kde_len;
	INT8U  ret = EAPOL_SM_OK_TX;

	/* Build KDEs */
	dpp_kde_len = dpp_eapol_gen_dpp_kde_ie(&dpp_kde);
	gtk_kde_len = dpp_eapol_gen_1905_gtk_kde_ie(&gtk_kde, sm->gtk.gtk, sm->gtk.gtk_len);
	kde_len     = dpp_kde_len + gtk_kde_len;
	kde         = PLATFORM_MALLOC(kde_len);
	PLATFORM_MEMCPY(kde, dpp_kde, dpp_kde_len);
	PLATFORM_MEMCPY(kde + dpp_kde_len, gtk_kde, gtk_kde_len);

	/* Build EAPOL-3 Frame */
	INT16U key_info = EAPOL_KEY_INFO_ENCR_KEY_DATA | EAPOL_KEY_INFO_SECURE
	                  | EAPOL_KEY_INFO_MIC | EAPOL_KEY_INFO_ACK
	                  | EAPOL_KEY_INFO_INSTALL | EAPOL_KEY_INFO_KEY_TYPE;
	dpp_eapol_inc_byte_array(sm->replay_counter, EAPOL_REPLAY_COUNTER_LEN);
	if (dpp_eapol_build(key_info, sm->ANonce, sm->replay_counter, kde, kde_len,
	                    sm->params, &sm->ptk, tx_buf, tx_len)
	    != 1) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: EAPOL-3 forging failed\n");
		ret = EAPOL_SM_FAIL;
		goto fail;
	}

	PLATFORM_PRINTF_INFO("DPP EAPOL: Authenticator - Forged 3/4\n");
fail:
	PLATFORM_FREE(kde);
	PLATFORM_FREE(dpp_kde);
	PLATFORM_FREE(gtk_kde);
	return ret;
}

static INT8U dpp_eapol_supplicant_process_3_of_4(struct dpp_eapol_sm * sm,
                                                 struct dpp_eapol_key *key, INT8U *kde, INT32U kde_len)
{
	INT8U *dpp_kde = NULL, *gtk_kde = NULL, *gtk;
	INT8U  dpp_kde_len, gtk_kde_len, gtk_len;

	PLATFORM_PRINTF_INFO("DPP EAPOL: Supplicant - Process 3/4\n");

	/* Get and check DPP KDE */
	dpp_kde = dpp_eapol_get_kde_ie(kde, kde_len,
	                               WLAN_EID_VENDOR_SPECIFIC,
	                               OUI_WFA, EAPOL_DPP_KDE_DATA_TYPE, &dpp_kde_len);
	if (!dpp_kde) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: DPP KDE missing!\n");
	} else {
		/* Check DPP KDE */
		if (dpp_eapol_parse_dpp_kde_ie(dpp_kde, dpp_kde_len) != 1) {
			PLATFORM_PRINTF_WARNING("DPP EAPOL: invalid DPP KDE!\n");
			return EAPOL_SM_FAIL;
		}
	}

	/* Get and check GTK KDE */
	gtk_kde = dpp_eapol_get_kde_ie(kde, kde_len,
	                               WLAN_EID_VENDOR_SPECIFIC,
	                               OUI_WFA, EAPOL_1905_GTK_KDE_DATA_TYPE, &gtk_kde_len);
	if (!gtk_kde) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: GTK KDE missing!\n");
		return EAPOL_SM_FAIL;
	}

	if (dpp_eapol_parse_1905_gtk_kde_ie(gtk_kde, gtk_kde_len, &gtk, &gtk_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: invalid 1905 GTK KDE!\n");
		return EAPOL_SM_FAIL;
	}

	if (gtk_len != sm->params->gtk_len) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: invalid GTK length!\n");
		return EAPOL_SM_FAIL;
	}

	/* Save Parameters */
	PLATFORM_MEMCPY(sm->gtk.gtk, gtk, gtk_len);
	sm->gtk.gtk_len = gtk_len;

	PLATFORM_MEMCPY(sm->replay_counter, key->replay_counter, EAPOL_REPLAY_COUNTER_LEN);

	return EAPOL_SM_OK;
}

static INT8U dpp_eapol_supplicant_send_4_of_4(struct dpp_eapol_sm *sm, INT8U **tx_buf, INT32U *tx_len)
{
	/* Build EAPOL-4 Frame */
	INT16U key_info = EAPOL_KEY_INFO_SECURE | EAPOL_KEY_INFO_MIC
	                  | EAPOL_KEY_INFO_KEY_TYPE;
	if (dpp_eapol_build(key_info, NULL, sm->replay_counter, NULL, 0,
	                    sm->params, &sm->ptk, tx_buf, tx_len)
	    != 1) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: EAPOL-3 forging failed\n");
		return EAPOL_SM_FAIL;
	}

	sm->state = FINISHED;
	PLATFORM_PRINTF_INFO("DPP EAPOL: Supplicant - Forged 4/4\n");
	return EAPOL_SM_OK_TX;
}

static INT8U dpp_eapol_authenticator_process_4_of_4(struct dpp_eapol_sm *sm, INT16U key_info)
{
	const INT16U expected_key_info = EAPOL_KEY_INFO_SECURE | EAPOL_KEY_INFO_MIC
	                                 | EAPOL_KEY_INFO_KEY_TYPE;

	PLATFORM_PRINTF_INFO("DPP EAPOL: Authenticator - Process 4/4\n");

	if (key_info != expected_key_info) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: key info is not 4/4");
		return EAPOL_SM_FAIL;
	}

	sm->state = FINISHED;
	return EAPOL_SM_OK;
}

// GTK Initializer
INT8U dpp_eapol_gtk_init(struct dpp_gtk *gtk, INT32U pmk_len)
{
	const struct dpp_eapol_params *params;

	if ((params = dpp_eapol_get_params(pmk_len)) == NULL) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: cannot get gtk length from pmk length!\n");
		return EAPOL_SM_FAIL;
	}

	PLATFORM_MEMSET(gtk, 0, sizeof(*gtk));
	gtk->gtk_len = params->gtk_len;
	if (PLATFORM_GET_RANDOM_BYTES(gtk->gtk, gtk->gtk_len) != 1) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: GTK generation failed\n");
		return EAPOL_SM_FAIL;
	}

	PLATFORM_PRINTF_INFO("DPP EAPOL: GTK Generated (%d bytes)\n", gtk->gtk_len);
	return EAPOL_SM_OK;
}

// State Machine Initializer
INT8U dpp_eapol_sm_init(struct dpp_eapol_sm *    sm,
                        struct dpp_introduction *intro,
                        struct dpp_gtk *         gtk,
                        enum dpp_eapol_role      role,
                        INT8U *own_addr, INT8U *peer_addr)
{
	// Reset state machine
	PLATFORM_MEMSET(sm, 0, sizeof(struct dpp_eapol_sm));
	sm->state = DISABLED;

	// Copy information
	PLATFORM_MEMCPY(&sm->intro, intro, sizeof(struct dpp_introduction));
	sm->params = dpp_eapol_get_params(intro->pmk_len);
	PLATFORM_MEMCPY(sm->own_addr, own_addr, EAPOL_ETH_ALEN);
	PLATFORM_MEMCPY(sm->peer_addr, peer_addr, EAPOL_ETH_ALEN);
	sm->role = role;

	// Copy GTK
	if (gtk) {
		if (gtk->gtk_len != sm->params->gtk_len) {
			PLATFORM_PRINTF_WARNING("DPP EAPOL: GTK Length Mismatch!\n");
			return EAPOL_SM_FAIL;
		}
		PLATFORM_MEMCPY(&sm->gtk, gtk, sizeof(struct dpp_gtk));
	}

	return EAPOL_SM_OK;
}

// State Machine Starter
INT8U dpp_eapol_sm_start(struct dpp_eapol_sm *sm, INT8U **tx_buf, INT32U *tx_len)
{
	// mark state as started
	sm->state = STARTED;

	// Issue EAPOL-1/4 if it is an authenticator
	if (sm->role == AUTHENTICATOR)
		return dpp_eapol_authenticator_send_1_of_4(sm, tx_buf, tx_len);

	return EAPOL_SM_OK;
}

// State Machine Ticker
INT8U dpp_eapol_sm_rx(struct dpp_eapol_sm *sm,
                      INT8U *rx_buf, INT32U rx_len, INT8U **tx_buf, INT32U *tx_len)
{
	const struct ieee802_1x_hdr *hdr;
	struct dpp_eapol_key *       key;
	INT8U *                      tmp, *mic, *key_data;
	INT32U                       mic_len, keyhdrlen;

	INT16U key_info;
	INT16U plen, key_data_len, data_len;
	INT8U *p, ret = EAPOL_SM_FAIL;
	int    counter_ret;

	if (!sm->params) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: parameter not configured\n");
		return EAPOL_SM_FAIL;
	}
	mic_len   = sm->params->mic_len;
	keyhdrlen = sizeof(*key) + mic_len + EAPOL_KEY_DATA_LEN_OCTET_LEN;

	PLATFORM_PRINTF_INFO("DPP EAPOL: Rx\n");

	// State machine checks
	if (sm->state != STARTED) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: state is not started\n");
		return EAPOL_SM_FAIL;
	}

	// Sanity checks
	if (rx_len < sizeof(*hdr) + keyhdrlen) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: EAPOL frame too short\n");
		return EAPOL_SM_FAIL;
	}

	// Get lengths
	hdr = (const struct ieee802_1x_hdr *)rx_buf;
	p   = (INT8U *)&hdr->length;
	_E2B(&p, &plen);                // payload length
	data_len = plen + sizeof(*hdr); // data length (payload + header)

	// Check Version
	if (hdr->version > 1) {
		PLATFORM_PRINTF_INFO("DPP EAPOL: version higher than 1\n");
	}

	// Check Type
	if (hdr->type != IEEE802_1X_TYPE_EAPOL_KEY) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: not a key frame!\n");
		return EAPOL_SM_FAIL;
	}

	// Check Length
	if (data_len < rx_len) {
		PLATFORM_PRINTF_INFO("DPP EAPOL: ignoring %d bytes after the IEEE 802.1X data\n",
		                     rx_len - data_len);
	}

	// Copy the whole data
	// because we will modify it for calculating MIC and decrypting KDE
	tmp = PLATFORM_MALLOC(data_len);
	PLATFORM_MEMCPY(tmp, rx_buf, data_len);
	key      = (struct dpp_eapol_key *)(tmp + sizeof(struct ieee802_1x_hdr));
	mic      = (INT8U *)(key + 1);
	key_data = mic + mic_len + EAPOL_KEY_DATA_LEN_OCTET_LEN;

	// Check Key Type
	if (key->type != EAPOL_KEY_TYPE_RSN) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: key type is not EAPOL_KEY_TYPE_RSN\n");
		goto fail;
	}

	// Get Key Data Length
	p = mic + mic_len;
	_E2B(&p, &key_data_len);
	if (key_data_len > plen - keyhdrlen) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: key data overflow\n");
		goto fail;
	}

	// Get Key Info
	p = key->key_info;
	_E2B(&p, &key_info);

	// Check Key Descriptor Version
	if ((key_info & EAPOL_KEY_INFO_TYPE_MASK) != EAPOL_KEY_INFO_TYPE_AKM_DEFINED) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: key ver not AKM\n");
		goto fail;
	}

	// Check Counter
	counter_ret = memcmp(key->replay_counter, sm->replay_counter, EAPOL_REPLAY_COUNTER_LEN);
	if (sm->role == AUTHENTICATOR && counter_ret != 0) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: Rx replay counter not equal to Tx counter\n");
		goto fail;
	} else if (sm->role == SUPPLICANT && counter_ret <= 0) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: Replay counter did not increase\n");
		goto fail;
	}

	// Check MIC
	if ((key_info & EAPOL_KEY_INFO_MIC) && (key_info & EAPOL_KEY_INFO_SECURE)
	    && (dpp_eapol_verify_eapol_key_mic(sm, key, tmp, data_len) != 1)) {
		PLATFORM_PRINTF_WARNING("DPP EAPOL: MIC mismatch\n");
		goto fail;
	}

	// Decrypt KDE after checking MIC
	if (key_info & EAPOL_KEY_INFO_ENCR_KEY_DATA) {
		if (dpp_eapol_decrypt_key_data(sm, key, key_data, &key_data_len) != 1) {
			PLATFORM_PRINTF_WARNING("DPP EAPOL: decryption failed\n");
			goto fail;
		}
	}

	//// STATE MACHINE LOGIC STARTS HERE ////
	//
	if (key_info & EAPOL_KEY_INFO_KEY_TYPE) {

		if (key_info & EAPOL_KEY_INFO_KEY_INDEX_MASK) {
			PLATFORM_PRINTF_WARNING("DPP EAPOL: ignore non zero key index\n");
			goto fail;
		}

		switch (sm->role) {
		case AUTHENTICATOR: {
			if ((key_info & EAPOL_KEY_INFO_MIC)
			    && (key_info & EAPOL_KEY_INFO_SECURE)) {
				/* 4/4 4-Way Handshake */
				ret = dpp_eapol_authenticator_process_4_of_4(sm, key_info);
			} else if ((key_info & EAPOL_KEY_INFO_MIC)
			           && !(key_info & ~(EAPOL_KEY_INFO_MIC | EAPOL_KEY_INFO_KEY_TYPE))) {
				/* 2/4 4-Way Handshake */
				if (!dpp_eapol_authenticator_process_2_of_4(sm, key, key_data, key_data_len)) {
					PLATFORM_PRINTF_WARNING("DPP EAPOL: cannot parse 2/4\n");
					goto fail;
				}
				ret = dpp_eapol_authenticator_send_3_of_4(sm, tx_buf, tx_len);
			}
		} break;

		case SUPPLICANT: {
			if ((key_info & EAPOL_KEY_INFO_MIC)
			    && (key_info & EAPOL_KEY_INFO_ENCR_KEY_DATA)) {
				/* 3/4 4-Way Handshake */
				if (!dpp_eapol_supplicant_process_3_of_4(sm, key, key_data, key_data_len)) {
					PLATFORM_PRINTF_WARNING("DPP EAPOL: cannot parse 3/4\n");
					goto fail;
				}
				ret = dpp_eapol_supplicant_send_4_of_4(sm, tx_buf, tx_len);
			} else if ((key_info & EAPOL_KEY_INFO_ACK)
			           && !(key_info & ~(EAPOL_KEY_INFO_ACK | EAPOL_KEY_INFO_KEY_TYPE))) {
				/* 1/4 4-Way Handshake */
				if (dpp_eapol_supplicant_process_1_of_4(sm, key_info, key) != 1) {
					PLATFORM_PRINTF_WARNING("DPP EAPOL: cannot parse 1/4\n");
					goto fail;
				}
				ret = dpp_eapol_supplicant_send_2_of_4(sm, tx_buf, tx_len);
			}
		} break;
		}
	}

fail:
	PLATFORM_FREE(tmp);
	return ret;
}