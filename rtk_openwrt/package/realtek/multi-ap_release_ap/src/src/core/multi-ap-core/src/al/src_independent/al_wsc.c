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

#include <string.h>
#include "platform.h"
#include "platform_os.h"

#include "al.h"
#include "al_wsc.h"
#include "al_datamodel.h"
#include "packet_tools.h"

#include "platform_crypto.h"
#include "platform_crypto_aes.h"
#include "platform_interfaces.h"

#include "global_settings.h"

////////////////////////////////////////////////////////////////////////////////
// Private functions and data
////////////////////////////////////////////////////////////////////////////////
#define ATTR_VERSION (0x104a)
#define ATTR_MSG_TYPE (0x1022)
#define WPS_M1 (0x04)
#define WPS_M2 (0x05)
#define ATTR_UUID_E (0x1047)
#define ATTR_UUID_R (0x1048)
#define ATTR_MAC_ADDR (0x1020)
#define ATTR_ENROLLEE_NONCE (0x101a)
#define ATTR_REGISTRAR_NONCE (0x1039)
#define ATTR_PUBLIC_KEY (0x1032)
#define ATTR_AUTH_TYPE_FLAGS (0x1004)
#define ATTR_ENCR_TYPE_FLAGS (0x1010)
#define ATTR_CONN_TYPE_FLAGS (0x100d)
#define WPS_CONN_ESS (0x01)
#define WPS_CONN_IBSS (0x02)
#define ATTR_CONFIG_METHODS (0x1008)
#define WPS_CONFIG_VIRT_PUSHBUTTON (0x0280)
#define WPS_CONFIG_PHY_PUSHBUTTON (0x0480)
#define ATTR_WPS_STATE (0x1044)
#define WPS_STATE_NOT_CONFIGURED (1)
#define WPS_STATE_CONFIGURED (2)
#define ATTR_MANUFACTURER (0x1021)
#define ATTR_MODEL_NAME (0x1023)
#define ATTR_MODEL_NUMBER (0x1024)
#define ATTR_SERIAL_NUMBER (0x1042)
#define ATTR_PRIMARY_DEV_TYPE (0x1054)
#define WPS_DEV_COMPUTER (1)
#define WPS_DEV_COMPUTER_PC (1)
#define WPS_DEV_COMPUTER_SERVER (2)
#define WPS_DEV_COMPUTER_MEDIA_CENTER (3)
#define WPS_DEV_COMPUTER_ULTRA_MOBILE (4)
#define WPS_DEV_COMPUTER_NOTEBOOK (5)
#define WPS_DEV_COMPUTER_DESKTOP (6)
#define WPS_DEV_COMPUTER_MID (7)
#define WPS_DEV_COMPUTER_NETBOOK (8)
#define WPS_DEV_COMPUTER_TABLET (9)
#define WPS_DEV_INPUT (2)
#define WPS_DEV_INPUT_KEYBOARD (1)
#define WPS_DEV_INPUT_MOUSE (2)
#define WPS_DEV_INPUT_JOYSTICK (3)
#define WPS_DEV_INPUT_TRACKBALL (4)
#define WPS_DEV_INPUT_GAMING (5)
#define WPS_DEV_INPUT_REMOTE (6)
#define WPS_DEV_INPUT_TOUCHSCREEN (7)
#define WPS_DEV_INPUT_BIOMETRIC_READER (8)
#define WPS_DEV_INPUT_BARCODE_READER (9)
#define WPS_DEV_PRINTER (3)
#define WPS_DEV_PRINTER_PRINTER (1)
#define WPS_DEV_PRINTER_SCANNER (2)
#define WPS_DEV_PRINTER_FAX (3)
#define WPS_DEV_PRINTER_COPIER (4)
#define WPS_DEV_PRINTER_ALL_IN_ONE (5)
#define WPS_DEV_CAMERA (4)
#define WPS_DEV_CAMERA_DIGITAL_STILL_CAMERA (1)
#define WPS_DEV_CAMERA_VIDEO (2)
#define WPS_DEV_CAMERA_WEB (3)
#define WPS_DEV_CAMERA_SECURITY (4)
#define WPS_DEV_STORAGE (5)
#define WPS_DEV_STORAGE_NAS (1)
#define WPS_DEV_NETWORK_INFRA (6)
#define WPS_DEV_NETWORK_INFRA_AP (1)
#define WPS_DEV_NETWORK_INFRA_ROUTER (2)
#define WPS_DEV_NETWORK_INFRA_SWITCH (3)
#define WPS_DEV_NETWORK_INFRA_GATEWAY (4)
#define WPS_DEV_NETWORK_INFRA_BRIDGE (5)
#define WPS_DEV_DISPLAY (7)
#define WPS_DEV_DISPLAY_TV (1)
#define WPS_DEV_DISPLAY_PICTURE_FRAME (2)
#define WPS_DEV_DISPLAY_PROJECTOR (3)
#define WPS_DEV_DISPLAY_MONITOR (4)
#define WPS_DEV_MULTIMEDIA (8)
#define WPS_DEV_MULTIMEDIA_DAR (1)
#define WPS_DEV_MULTIMEDIA_PVR (2)
#define WPS_DEV_MULTIMEDIA_MCX (3)
#define WPS_DEV_MULTIMEDIA_SET_TOP_BOX (4)
#define WPS_DEV_MULTIMEDIA_MEDIA_SERVER (5)
#define WPS_DEV_MULTIMEDIA_PORTABLE_VIDEO_PLAYER (6)
#define WPS_DEV_GAMING (9)
#define WPS_DEV_GAMING_XBOX (1)
#define WPS_DEV_GAMING_XBOX360 (2)
#define WPS_DEV_GAMING_PLAYSTATION (3)
#define WPS_DEV_GAMING_GAME_CONSOLE (4)
#define WPS_DEV_GAMING_PORTABLE_DEVICE (5)
#define WPS_DEV_PHONE (10)
#define WPS_DEV_PHONE_WINDOWS_MOBILE (1)
#define WPS_DEV_PHONE_SINGLE_MODE (2)
#define WPS_DEV_PHONE_DUAL_MODE (3)
#define WPS_DEV_PHONE_SP_SINGLE_MODE (4)
#define WPS_DEV_PHONE_SP_DUAL_MODE (5)
#define WPS_DEV_AUDIO (11)
#define WPS_DEV_AUDIO_TUNER_RECV (1)
#define WPS_DEV_AUDIO_SPEAKERS (2)
#define WPS_DEV_AUDIO_PMP (3)
#define WPS_DEV_AUDIO_HEADSET (4)
#define WPS_DEV_AUDIO_HEADPHONES (5)
#define WPS_DEV_AUDIO_MICROPHONE (6)
#define WPS_DEV_AUDIO_HOME_THEATRE (7)
#define ATTR_DEV_NAME (0x1011)
#define ATTR_RF_BANDS (0x103c)
#define WPS_RF_24GHZ (0x01)
#define WPS_RF_50GHZ (0x02)
#define WPS_RF_60GHZ (0x04)
#define ATTR_ASSOC_STATE (0x1002)
#define WPS_ASSOC_NOT_ASSOC (0)
#define WPS_ASSOC_CONN_SUCCESS (1)
#define ATTR_DEV_PASSWORD_ID (0x1012)
#define DEV_PW_PUSHBUTTON (0x0004)
#define ATTR_CONFIG_ERROR (0x1009)
#define WPS_CFG_NO_ERROR (0)
#define ATTR_OS_VERSION (0x102d)
#define ATTR_VENDOR_EXTENSION (0x1049)
#define WPS_VENDOR_ID_WFA_1 (0x00)
#define WPS_VENDOR_ID_WFA_2 (0x37)
#define WPS_VENDOR_ID_WFA_3 (0x2A)
#define WFA_ELEM_VERSION2 (0x00)
#define WPS_VERSION (0x20)
#define WPS_VENDOR_MAP_SUBELEMENT (0x06)
#define ATTR_SSID (0x1045)
#define ATTR_AUTH_TYPE (0x1003)
#define ATTR_ENCR_TYPE (0x100f)
#define ATTR_NETWORK_KEY (0x1027)
#define ATTR_KEY_WRAP_AUTH (0x101e)
#define ATTR_ENCR_SETTINGS (0x1018)
#define ATTR_AUTHENTICATOR (0x1005)

#define MULTI_AP_SUBELEMENT_ID (0x06)
#define MULTI_AP_SUBELEMENT_LEN (0x01)

// Keys sizes
//
#define WPS_AUTHKEY_LEN 32
#define WPS_KEYWRAPKEY_LEN 16
#define WPS_EMSK_LEN 32

#define MAP_CONFIG_2G (0)
#define MAP_CONFIG_5GL (1)
#define MAP_CONFIG_5GH (2)

struct wscKey {
	INT8U *key;
	INT32U key_len;
	INT8U  mac[6];
};

// Global variable to save the latest M1 message created
//
INT8U *        last_m1      = NULL;
INT16U         last_m1_size = 0;
struct wscKey *last_key     = NULL;

INT8U *        last_m1_5g      = NULL;
INT16U         last_m1_size_5g = 0;
struct wscKey *last_key_5g     = NULL;

INT8U *        last_m1_5gh      = NULL;
INT16U         last_m1_size_5gh = 0;
struct wscKey *last_key_5gh     = NULL;

// This is the key derivation function used in the WPS standard to obtain a
// final hash that is later used for encryption.
//
// The output is stored in the memory buffer pointed by 'res', which must be
// "SHA256_MAC_LEN" bytes long (ie. 're_len' must always be "SHA256_MAC_LEN",
// even if it is an input argument)
//
void _wps_key_derivation_function(INT8U *key, INT8U *label_prefix, INT32U label_prefix_len, char *label, INT8U *res, INT32U res_len)
{
	INT8U i_buf[4];
	INT8U key_bits[4];

	INT8U *addr[4];
	INT32U len[4];

	INT32U i, iter;

	INT8U  hash[SHA256_MAC_LEN];
	INT8U *opos;

	INT32U left;

	INT8U *p;
	INT32U aux;

	aux = res_len * 8;
	p   = key_bits;

	_I4B(&aux, &p);

	addr[0] = i_buf;
	addr[1] = label_prefix;
	addr[2] = (INT8U *)label;
	addr[3] = key_bits;
	len[0]  = sizeof(i_buf);
	len[1]  = label_prefix_len;
	len[2]  = PLATFORM_STRLEN(label);
	len[3]  = sizeof(key_bits);

	iter = (res_len + SHA256_MAC_LEN - 1) / SHA256_MAC_LEN;
	opos = res;
	left = res_len;

	for (i = 1; i <= iter; i++) {
		p = i_buf;
		_I4B(&i, &p);

		PLATFORM_HMAC_SHA256(key, SHA256_MAC_LEN, 4, addr, len, hash);

		if (i < iter) {
			PLATFORM_MEMCPY(opos, hash, SHA256_MAC_LEN);
			opos += SHA256_MAC_LEN;
			left -= SHA256_MAC_LEN;
		} else {
			PLATFORM_MEMCPY(opos, hash, left);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////
void wscFreeLastM1(INT8U wsc_band, INT8U radio_band)
{
	if (IEEE80211_FREQUENCY_BAND_2_4_GHZ == wsc_band) {
		PLATFORM_FREE(last_m1);
		last_m1 = NULL;
		if (last_key != NULL) {
			PLATFORM_FREE(last_key->key);
			last_key->key = NULL;
			PLATFORM_FREE(last_key);
			last_key = NULL;
		}
		PLATFORM_PRINTF_DEBUG("WSC 2.4G Freed!!!!!!\n");
	} else if (IEEE80211_FREQUENCY_BAND_5_GHZ == wsc_band) {
		if (MAP_CONFIG_5GH == radio_band) {
			PLATFORM_FREE(last_m1_5gh);
			last_m1_5gh = NULL;
			if (last_key_5gh != NULL) {
				PLATFORM_FREE(last_key_5gh->key);
				last_key_5gh->key = NULL;
				PLATFORM_FREE(last_key_5gh);
				last_key_5gh = NULL;
			}
			PLATFORM_PRINTF_DEBUG("WSC 5GH Freed!!!!!!\n");
		} else {
			PLATFORM_FREE(last_m1_5g);
			last_m1_5g = NULL;
			if (last_key_5g != NULL) {
				PLATFORM_FREE(last_key_5g->key);
				last_key_5g->key = NULL;
				PLATFORM_FREE(last_key_5g);
				last_key_5g = NULL;
			}
			PLATFORM_PRINTF_DEBUG("WSC 5GL Freed!!!!!!\n");
		}
	}
}
//
//////////////////////////////////////// Enrollee functions ////////////////////
//
INT8U wscBuildM1(char *interface_name, INT8U **m1, INT16U *m1_size, void **key)
{
	INT8U *buffer;

	struct interfaceInfo *x;

	struct wscKey *private_key;

	struct deviceInfo info;
	PLATFORM_MEMSET(&info, 0, sizeof(info));

	INT8U *p;

	INT8U  aux8;
	INT16U aux16;
	INT32U aux32;

	if (NULL == interface_name || NULL == m1 || NULL == m1_size || NULL == key) {
		PLATFORM_PRINTF_WARNING("Invalid arguments to wscBuildM1()\n");
		return 0;
	}

	if (NULL == (x = PLATFORM_GET_1905_INTERFACE_INFO(interface_name))) {
		PLATFORM_PRINTF_WARNING("Could not retrieve info of interface %s\n", interface_name);
		return 0;
	}

	INT8U rf_bands;

	rf_bands = 0;

	if (IS_IEEE_802_11_2_4_GHZ(x->interface_type, x->interface_type_data.ieee80211.rf_band)) {
		rf_bands = WPS_RF_24GHZ;
	} else if (IS_IEEE_802_11_5_GHZ(x->interface_type, x->interface_type_data.ieee80211.rf_band)) {
		rf_bands = WPS_RF_50GHZ;
	} else if (IS_IEEE_802_11_60_GHZ(x->interface_type)) {
		PLATFORM_PRINTF_WARNING("Unsupported band!!\n");
		PLATFORM_FREE_1905_INTERFACE_INFO(x);
		return 0;
	}

	// PLATFORM_FREE(last_m1);
	// last_m1 = NULL;
	// if (last_key != NULL) {
	// 	PLATFORM_FREE(last_key->key);
	// 	last_key->key = NULL;
	// 	PLATFORM_FREE(last_key);
	// 	last_key = NULL;
	// }

	buffer = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * 1000);
	p      = buffer;

	INT8U al_mac_address[6];
	PLATFORM_MEMCPY(al_mac_address, DMalMacGet(), 6);

	PLATFORM_GET_DEVICE_INFO(&info);

	// VERSION
	{
		aux16 = ATTR_VERSION;
		_I2B(&aux16, &p);
		aux16 = 1;
		_I2B(&aux16, &p);
		aux8 = 0x10;
		_I1B(&aux8, &p);
	}

	// MESSAGE TYPE
	{
		aux16 = ATTR_MSG_TYPE;
		_I2B(&aux16, &p);
		aux16 = 1;
		_I2B(&aux16, &p);
		aux8 = WPS_M1;
		_I1B(&aux8, &p);
	}

	// UUID
	{
		aux16 = ATTR_UUID_E;
		_I2B(&aux16, &p);
		aux16 = 16;
		_I2B(&aux16, &p);
		_InB(info.uuid, &p, 16);
	}

	// MAC ADDRESS
	{
		aux16 = ATTR_MAC_ADDR;
		_I2B(&aux16, &p);
		aux16 = 6;
		_I2B(&aux16, &p);
		_InB(al_mac_address, &p, 6);
	}

	// ENROLLEE NONCE
	{
		INT8U enrollee_nonce[16];

		PLATFORM_GET_RANDOM_BYTES(enrollee_nonce, 16);

		aux16 = ATTR_ENROLLEE_NONCE;
		_I2B(&aux16, &p);
		aux16 = 16;
		_I2B(&aux16, &p);
		_InB(enrollee_nonce, &p, 16);
	}

	// PUBLIC KEY
	{
		INT8U *priv, *pub;
		INT16U priv_len, pub_len;

		PLATFORM_GENERATE_DH_KEY_PAIR(&priv, &priv_len, &pub, &pub_len);
		// TODO: ZERO PAD the pub key (doesn't seem to be really needed though)

		aux16 = ATTR_PUBLIC_KEY;
		_I2B(&aux16, &p);
		aux16 = pub_len;
		_I2B(&aux16, &p);
		_InB(pub, &p, pub_len);
		// The private key is one of the output arguments
		//
		private_key          = (struct wscKey *)PLATFORM_MALLOC(sizeof(struct wscKey));
		private_key->key     = (INT8U *)PLATFORM_MALLOC(priv_len);
		private_key->key_len = priv_len;
		PLATFORM_MEMCPY(private_key->key, priv, priv_len);
		PLATFORM_MEMCPY(private_key->mac, al_mac_address, 6);

		PLATFORM_FREE(priv);
		PLATFORM_FREE(pub);
	}

	// AUTHENTICATION TYPES
	{
		INT16U auth_types;

		auth_types = 0;
		auth_types |= WPS_AUTH_OPEN;
		if(customize_features & CUSTOMIZE_WSC_INFO){
			auth_types |= WPS_AUTH_WPAPSK;
		}
		auth_types |= WPS_AUTH_WPA2PSK;

#ifdef WPA3_SUPPORT
		auth_types |= WPS_AUTH_WPA3;
#endif

		aux16 = ATTR_AUTH_TYPE_FLAGS;
		_I2B(&aux16, &p);
		aux16 = 2;
		_I2B(&aux16, &p);
		_I2B(&auth_types, &p);
	}

	// ENCRYPTION TYPES
	{
		INT16U encryption_types;

		encryption_types = 0;

		if(customize_features & CUSTOMIZE_WSC_INFO){
			encryption_types |= WPS_ENCR_NONE;
			encryption_types |= WPS_ENCR_WEP;
			encryption_types |= WPS_ENCR_TKIP;
			encryption_types |= WPS_ENCR_AES;
		} else {
			if (IEEE80211_ENCRYPTION_MODE_NONE & x->interface_type_data.ieee80211.encryption_mode) {
				encryption_types |= WPS_ENCR_NONE;
			}
			if (IEEE80211_ENCRYPTION_MODE_TKIP & x->interface_type_data.ieee80211.encryption_mode) {
				encryption_types |= WPS_ENCR_TKIP;
			}
			if (IEEE80211_ENCRYPTION_MODE_AES & x->interface_type_data.ieee80211.encryption_mode) {
				encryption_types |= WPS_ENCR_AES;
			}
		}

		aux16 = ATTR_ENCR_TYPE_FLAGS;
		_I2B(&aux16, &p);
		aux16 = 2;
		_I2B(&aux16, &p);
		_I2B(&encryption_types, &p);
	}

	// CONNECTION TYPES
	{
		// Two possible types: ESS or IBSS. In the 1905 context, enrollees will
		// always want to acts as "ESS" to create an "extended" network where
		// all APs share the same credentials as the registrar.

		aux16 = ATTR_CONN_TYPE_FLAGS;
		_I2B(&aux16, &p);
		aux16 = 1;
		_I2B(&aux16, &p);
		aux8 = WPS_CONN_ESS;
		_I1B(&aux8, &p);
	}

	// CONFIGURATION METHODS
	{
		// In the 1905 context, the configuration methods the AP is willing to
		// offer will always be these two

		aux16 = ATTR_CONFIG_METHODS;
		_I2B(&aux16, &p);
		aux16 = 2;
		_I2B(&aux16, &p);
		aux16 = WPS_CONFIG_PHY_PUSHBUTTON | WPS_CONFIG_VIRT_PUSHBUTTON;
		_I2B(&aux16, &p);
	}

	// WPS STATE
	{
		aux16 = ATTR_WPS_STATE;
		_I2B(&aux16, &p);
		aux16 = 1;
		_I2B(&aux16, &p);
		aux8 = WPS_STATE_NOT_CONFIGURED;
		_I1B(&aux8, &p);
	}

	// MANUFACTURER
	{
		aux16 = ATTR_MANUFACTURER;
		_I2B(&aux16, &p);
		aux16 = PLATFORM_STRLEN(info.manufacturer_name);
		_I2B(&aux16, &p);
		_InB(info.manufacturer_name, &p, PLATFORM_STRLEN(info.manufacturer_name));
	}

	// MODEL NAME
	{

		aux16 = ATTR_MODEL_NAME;
		_I2B(&aux16, &p);
		aux16 = PLATFORM_STRLEN(info.model_name);
		_I2B(&aux16, &p);
		_InB(info.model_name, &p, PLATFORM_STRLEN(info.model_name));
	}

	// MODEL NUMBER
	{
		aux16 = ATTR_MODEL_NUMBER;
		_I2B(&aux16, &p);
		aux16 = PLATFORM_STRLEN(info.model_number);
		_I2B(&aux16, &p);
		_InB(info.model_number, &p, PLATFORM_STRLEN(info.model_number));
	}

	// SERIAL NUMBER
	{
		aux16 = ATTR_SERIAL_NUMBER;
		_I2B(&aux16, &p);
		aux16 = PLATFORM_STRLEN(info.serial_number);
		_I2B(&aux16, &p);
		_InB(info.serial_number, &p, PLATFORM_STRLEN(info.serial_number));
	}

	// PRIMARY DEVICE TYPE
	{
		// In the 1905 context, they node sending a M1 message will always be
		// (at least) a "network router"

		INT8U oui[4] = { 0x00, 0x50, 0xf2, 0x00 }; // Fixed value from the
		                                           // WSC spec

		aux16 = ATTR_PRIMARY_DEV_TYPE;
		_I2B(&aux16, &p);
		aux16 = 8;
		_I2B(&aux16, &p);
		aux16 = WPS_DEV_NETWORK_INFRA;
		_I2B(&aux16, &p);
		_InB(oui, &p, 4);
		aux16 = WPS_DEV_NETWORK_INFRA_ROUTER;
		_I2B(&aux16, &p);
	}

	// DEVICE NAME
	{
		aux16 = ATTR_DEV_NAME;
		_I2B(&aux16, &p);
		aux16 = PLATFORM_STRLEN(info.device_name);
		_I2B(&aux16, &p);
		_InB(info.device_name, &p, PLATFORM_STRLEN(info.device_name));
	}

	// RF BANDS
	{
		aux16 = ATTR_RF_BANDS;
		_I2B(&aux16, &p);
		aux16 = 1;
		_I2B(&aux16, &p);
		_I1B(&rf_bands, &p);
	}

	// ASSOCIATION STATE
	{
		aux16 = ATTR_ASSOC_STATE;
		_I2B(&aux16, &p);
		aux16 = 2;
		_I2B(&aux16, &p);
		aux16 = WPS_ASSOC_NOT_ASSOC;
		_I2B(&aux16, &p);
	}

	// DEVICE PASSWORD ID
	{
		aux16 = ATTR_DEV_PASSWORD_ID;
		_I2B(&aux16, &p);
		aux16 = 2;
		_I2B(&aux16, &p);
		aux16 = DEV_PW_PUSHBUTTON;
		_I2B(&aux16, &p);
	}

	// CONFIG ERROR
	{
		aux16 = ATTR_CONFIG_ERROR;
		_I2B(&aux16, &p);
		aux16 = 2;
		_I2B(&aux16, &p);
		aux16 = WPS_CFG_NO_ERROR;
		_I2B(&aux16, &p);
	}

	// OS VERSION
	{
		// TODO: Fill with actual properties from the interface

		INT32U os_version = 0x00000001;

		aux16 = ATTR_OS_VERSION;
		_I2B(&aux16, &p);
		aux16 = 4;
		_I2B(&aux16, &p);
		aux32 = 0x80000000 | os_version;
		_I4B(&aux32, &p);
	}

	// VENDOR EXTENSIONS
	{
		aux16 = ATTR_VENDOR_EXTENSION;
		_I2B(&aux16, &p);
		aux16 = 9;
		_I2B(&aux16, &p);
		aux8 = WPS_VENDOR_ID_WFA_1;
		_I1B(&aux8, &p);
		aux8 = WPS_VENDOR_ID_WFA_2;
		_I1B(&aux8, &p);
		aux8 = WPS_VENDOR_ID_WFA_3;
		_I1B(&aux8, &p);
		aux8 = WFA_ELEM_VERSION2;
		_I1B(&aux8, &p);
		aux8 = 1;
		_I1B(&aux8, &p);
		aux8 = WPS_VERSION;
		_I1B(&aux8, &p);
		// Multi AP
		aux8 = WPS_VENDOR_MAP_SUBELEMENT;
		_I1B(&aux8, &p);
		aux8 = 1;
		_I1B(&aux8, &p);
		INT8U multi_ap_bss = PLATFORM_GET_BSS_TYPE(interface_name);
		if (strstr(interface_name, map_vxd_prefix)) {
			multi_ap_bss |= 0x80;
		}
		PLATFORM_PRINTF_DEBUG("[WSC M1] %s %02x\n", interface_name, multi_ap_bss);
		aux8 = multi_ap_bss;
		_I1B(&aux8, &p);
	}

	PLATFORM_FREE_1905_INTERFACE_INFO(x);
	if (WPS_RF_24GHZ == rf_bands) {
		*m1 = last_m1 = buffer;
		*m1_size = last_m1_size = p - buffer;
		*key = last_key = private_key;
	} else if (WPS_RF_50GHZ == rf_bands) {
		if (0 == PLATFORM_STRCMP(interface_name, map_radio_name_5gl)) {
			*m1 = last_m1_5g = buffer;
			*m1_size = last_m1_size_5g = p - buffer;
			*key = last_key_5g = private_key;
		} else if (0 == PLATFORM_STRCMP(interface_name, map_radio_name_5gh)) {
			*m1 = last_m1_5gh = buffer;
			*m1_size = last_m1_size_5gh = p - buffer;
			*key = last_key_5gh = private_key;
		} else {
			*m1 = NULL;
			*m1_size = 0;
			*key = NULL;
			PLATFORM_FREE(buffer);
			PLATFORM_FREE(private_key);
			PLATFORM_PRINTF_ERROR("Unknown 5G interface %s!!\n", interface_name);
			return 0;
		}
	} else {
		*m1      = NULL;
		*m1_size = 0;
		*key     = NULL;
		PLATFORM_FREE(buffer);
		PLATFORM_FREE(private_key);
		PLATFORM_PRINTF_ERROR("Unsupported band %02x for interface %s!!\n", rf_bands, interface_name);
		return 0;
	}

	return 1;
}

INT8U wscProcessM2(void *key, INT8U *m1, INT16U m1_size, INT8U *m2, INT16U m2_size, struct wsc_config *config_data, INT8U radio_band)
{
	INT8U *        p;
	struct wscKey *k = NULL;

	// "Useful data" we want to extract from M2
	//
	char   ssid[64];
	INT8U  ssid_present;
	INT8U  bssid[6];
	INT8U  bssid_present;
	INT16U auth_type;
	INT8U  auth_type_present;
	INT16U encryption_type;
	INT8U  encryption_type_present;
	char   network_key[64];
	INT8U  network_key_present;

	INT8U bss_type = 0;

	// Keys we need to compute to authenticate and decrypt M2
	//
	INT8U authkey[WPS_AUTHKEY_LEN];
	INT8U keywrapkey[WPS_KEYWRAPKEY_LEN];
	INT8U emsk[WPS_EMSK_LEN];

	// "Intermediary data" we also need to extract from M2 to obtain the keys
	// that will let us decrypt the "useful data" from M2
	//
	INT8U *m2_nonce = NULL;
	INT8U  m2_nonce_present;
	INT8U *m2_pubkey = NULL;
	INT8U  m2_pubkey_present;
	INT16U m2_pubkey_len         = 0;
	INT8U *m2_encrypted_settings = 0;
	INT8U  m2_encrypted_settings_present;
	INT16U m2_encrypted_settings_len = 0;
	INT8U *m2_authenticator          = NULL;
	INT8U  m2_authenticator_present;

	// "Intermediary data" we also need to extract from M1 to obtain the keys
	// that will let us decrypt the "useful data" from M2
	//
	INT8U *                      m1_nonce = NULL;
	INT8U                        m1_nonce_present;
	/*INT8U  *m1_pubkey;*/ INT8U m1_pubkey_present;
	INT16U                       m1_pubkey_len = 0;

	// "Intermediary data" contained in the "key" argument also needed to obtain
	// the keys that will let us decrypt the "useful data" from M2
	//
	INT8U *m1_privkey;
	INT16U m1_privkey_len;
	INT8U *m1_mac;
	INT8U  m2_band;

	m2_band = wscGetBand(m2, m2_size);

	if (NULL == m1) {
		// Use the last M1 built message
		if (IEEE80211_FREQUENCY_BAND_2_4_GHZ == m2_band) {
			m1      = last_m1;
			m1_size = last_m1_size;
			k       = last_key;
		} else if (IEEE80211_FREQUENCY_BAND_5_GHZ == m2_band) {
			m1      = last_m1_5g;
			m1_size = last_m1_size_5g;
			k       = last_key_5g;
			if (MAP_CONFIG_5GH == radio_band) {
				m1      = last_m1_5gh;
				m1_size = last_m1_size_5gh;
				k       = last_key_5gh;
			}
		}
	} else {
		k = (struct wscKey *)key;
	}

	if (NULL == m1 || 0 == m1_size) {
		return 0;
	}

	m1_privkey     = k->key;
	m1_privkey_len = k->key_len;
	m1_mac         = k->mac;

	// Extract "intermediary data" from M2
	//
	m2_nonce_present              = 0;
	m2_pubkey_present             = 0;
	m2_encrypted_settings_present = 0;
	m2_authenticator_present      = 0;
	p                             = m2;
	while (p - m2 < m2_size) {
		INT16U attr_type;
		INT16U attr_len;

		_E2B(&p, &attr_type);
		_E2B(&p, &attr_len);

		if (ATTR_REGISTRAR_NONCE == attr_type) {
			if (16 != attr_len) {
				PLATFORM_PRINTF_WARNING("Incorrect length (%d) for ATTR_REGISTRAR_NONCE\n", attr_len);
				return 0;
			}
			m2_nonce = p;

			p += attr_len;
			m2_nonce_present = 1;
		} else if (ATTR_PUBLIC_KEY == attr_type) {
			m2_pubkey_len = attr_len;
			m2_pubkey     = p;

			p += attr_len;
			m2_pubkey_present = 1;
		} else if (ATTR_ENCR_SETTINGS == attr_type) {
			m2_encrypted_settings_len = attr_len;
			m2_encrypted_settings     = p;

			p += attr_len;
			m2_encrypted_settings_present = 1;
		} else if (ATTR_AUTHENTICATOR == attr_type) {
			if (8 != attr_len) {
				PLATFORM_PRINTF_WARNING("Incorrect length (%d) for ATTR_AUTHENTICATOR\n", attr_len);
				return 0;
			}
			m2_authenticator = p;

			p += attr_len;
			m2_authenticator_present = 1;
		} else {
			p += attr_len;
		}
	}
	if (
	    0 == m2_nonce_present || 0 == m2_pubkey_present || 0 == m2_encrypted_settings_present || 0 == m2_authenticator_present) {
		PLATFORM_PRINTF_WARNING("Missing attributes in the received M2 message\n");
		return 0;
	}

	// Extract "intermediary data" from M1
	//
	m1_nonce_present  = 0;
	m1_pubkey_present = 0;
	p                 = m1;
	while (p - m1 < m1_size) {
		INT16U attr_type;
		INT16U attr_len;

		_E2B(&p, &attr_type);
		_E2B(&p, &attr_len);

		if (ATTR_ENROLLEE_NONCE == attr_type) {
			if (16 != attr_len) {
				PLATFORM_PRINTF_WARNING("Incorrect length (%d) for ATTR_REGISTRAR_NONCE\n", attr_len);
				return 0;
			}
			m1_nonce = p;

			p += attr_len;
			m1_nonce_present = 1;
		} else if (ATTR_PUBLIC_KEY == attr_type) {
			m1_pubkey_len = attr_len;
			/*m1_pubkey     = p;*/

			p += m1_pubkey_len;
			m1_pubkey_present = 1;
		} else {
			p += attr_len;
		}
	}
	if (
	    0 == m1_nonce_present || 0 == m1_pubkey_present) {
		PLATFORM_PRINTF_WARNING("Missing attributes in the received M1 message\n");
		return 0;
	}

	// With all the information we have just extracted from M1 and M2, obtain
	// the authentication/encryption keys.
	{
		INT8U *shared_secret;
		INT16U shared_secret_len;

		INT8U *addr[3];
		INT32U len[3];

		INT8U dhkey[SHA256_MAC_LEN];
		INT8U kdk[SHA256_MAC_LEN];

		INT8U keys[WPS_AUTHKEY_LEN + WPS_KEYWRAPKEY_LEN + WPS_EMSK_LEN];

		// With the enrolle public key (which we received in M1) and our private
		// key (which we generated above), obtain the Diffie Hellman shared
		// secret (when receiving M2, the enrollee will be able to obtain this
		// same shared secret using its private key and ou public key -contained
		// in M2-)
		//
		PLATFORM_COMPUTE_DH_SHARED_SECRET(&shared_secret, &shared_secret_len, m2_pubkey, m2_pubkey_len, m1_privkey, m1_privkey_len);
		// TODO: ZERO PAD the shared_secret (doesn't seem to be really needed
		// though)

		// Next, obtain the SHA-256 digest of this shared secret. We will call
		// this "dhkey"
		//
		addr[0] = shared_secret;
		len[0]  = shared_secret_len;

		PLATFORM_SHA256(1, addr, len, dhkey);

		// Next, concatenate three things (the enrolle nonce contained in M1,
		// the enrolle MAC address, and the nonce we just generated before, and
		// calculate its HMAC (hash message authentication code) using "dhkey"
		// as the secret key.
		//
		addr[0] = m1_nonce;
		addr[1] = m1_mac;
		addr[2] = m2_nonce;
		len[0]  = 16;
		len[1]  = 6;
		len[2]  = 16;

		PLATFORM_HMAC_SHA256(dhkey, SHA256_MAC_LEN, 3, addr, len, kdk);

		// Finally, take "kdk" and using a function provided in the "Wi-Fi
		// simple configuration" standard, obtain THREE KEYS that we will use
		// later ("authkey", "keywrapkey" and "emsk")
		//
		_wps_key_derivation_function(kdk, NULL, 0, "Wi-Fi Easy and Secure Key Derivation", keys, sizeof(keys));

		PLATFORM_MEMCPY(authkey, keys, WPS_AUTHKEY_LEN);
		PLATFORM_MEMCPY(keywrapkey, keys + WPS_AUTHKEY_LEN, WPS_KEYWRAPKEY_LEN);
		PLATFORM_MEMCPY(emsk, keys + WPS_AUTHKEY_LEN + WPS_KEYWRAPKEY_LEN, WPS_EMSK_LEN);

		PLATFORM_PRINTF_DETAIL("WPS keys: \n");
		PLATFORM_PRINTF_DETAIL("  Registrar pubkey  (%3d bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", m2_pubkey_len, m2_pubkey[0], m2_pubkey[1], m2_pubkey[2], m2_pubkey[m2_pubkey_len - 3], m2_pubkey[m2_pubkey_len - 2], m2_pubkey[m2_pubkey_len - 1]);
		PLATFORM_PRINTF_DETAIL("  Enrollee privkey  (%3d bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", m1_privkey_len, m1_privkey[0], m1_privkey[1], m1_privkey[2], m1_privkey[m1_privkey_len - 3], m1_privkey[m1_privkey_len - 2], m1_privkey[m1_privkey_len - 1]);
		PLATFORM_PRINTF_DETAIL("  Shared secret     (%3d bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", shared_secret_len, shared_secret[0], shared_secret[1], shared_secret[2], shared_secret[shared_secret_len - 3], shared_secret[shared_secret_len - 2], shared_secret[shared_secret_len - 1]);
		PLATFORM_PRINTF_DETAIL("  DH key            ( 32 bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", dhkey[0], dhkey[1], dhkey[2], dhkey[29], dhkey[30], dhkey[31]);
		PLATFORM_PRINTF_DETAIL("  Enrollee nonce    ( 16 bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", m1_nonce[0], m1_nonce[1], m1_nonce[2], m1_nonce[13], m1_nonce[14], m1_nonce[15]);
		PLATFORM_PRINTF_DETAIL("  Registrar nonce   ( 16 bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", m2_nonce[0], m2_nonce[1], m2_nonce[2], m2_nonce[13], m2_nonce[14], m2_nonce[15]);
		PLATFORM_PRINTF_DETAIL("  KDK               ( 32 bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", kdk[0], kdk[1], kdk[2], kdk[29], kdk[30], kdk[31]);
		PLATFORM_PRINTF_DETAIL("  authkey           (%3d bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", WPS_AUTHKEY_LEN, authkey[0], authkey[1], authkey[2], authkey[WPS_AUTHKEY_LEN - 3], authkey[WPS_AUTHKEY_LEN - 2], authkey[WPS_AUTHKEY_LEN - 1]);
		PLATFORM_PRINTF_DETAIL("  keywrapkey        (%3d bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", WPS_KEYWRAPKEY_LEN, keywrapkey[0], keywrapkey[1], keywrapkey[2], keywrapkey[WPS_KEYWRAPKEY_LEN - 3], keywrapkey[WPS_KEYWRAPKEY_LEN - 2], keywrapkey[WPS_KEYWRAPKEY_LEN - 1]);
		PLATFORM_PRINTF_DETAIL("  emsk              (%3d bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", WPS_EMSK_LEN, emsk[0], emsk[1], emsk[2], emsk[WPS_EMSK_LEN - 3], emsk[WPS_EMSK_LEN - 2], emsk[WPS_EMSK_LEN - 1]);

		//free shared_secret after printf detail
	    PLATFORM_FREE(shared_secret);
	}

	// With the just computed key, check the message authentication
	//
	{
		// Concatenate M1 and M2 (excluding the last 12 bytes, where the
		// authenticator attribute is contained) and calculate the HMAC, then
		// check it against the actual authenticator attribute value.
		//
		INT8U hash[SHA256_MAC_LEN];

		INT8U *addr[2];
		INT32U len[2];

		addr[0] = m1;
		addr[1] = m2;
		len[0]  = m1_size;
		len[1]  = m2_size - 12;

		PLATFORM_HMAC_SHA256(authkey, WPS_AUTHKEY_LEN, 2, addr, len, hash);

		if (PLATFORM_MEMCMP(m2_authenticator, hash, 8) != 0) {
			PLATFORM_PRINTF_WARNING("Message M2 authentication failed\n");
			PLATFORM_PRINTF_WARNING("m2_authenticator %02x %02x %02x %02x %02x %02x %02x %02x\n", m2_authenticator[0], m2_authenticator[1],
			                        m2_authenticator[2], m2_authenticator[3], m2_authenticator[4], m2_authenticator[5],
			                        m2_authenticator[6], m2_authenticator[7]);
			PLATFORM_PRINTF_WARNING("hash %02x %02x %02x %02x %02x %02x %02x %02x\n", hash[0], hash[1],
			                        hash[2], hash[3], hash[4], hash[5], hash[6], hash[7]);
			return 0;
		}
	}

	// With the just computed keys, decrypt the message and check the keywrap
	{
		INT8U *plain;
		INT32U plain_len;

		INT8U m2_keywrap_present;

		plain     = m2_encrypted_settings + AES_BLOCK_SIZE;
		plain_len = m2_encrypted_settings_len - AES_BLOCK_SIZE;

		PLATFORM_PRINTF_DETAIL("AP settings before decryption (%d bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", plain_len, plain[0], plain[1], plain[2], plain[plain_len - 3], plain[plain_len - 2], plain[plain_len - 1]);
		PLATFORM_PRINTF_DETAIL("IV (%d bytes)                           : 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", AES_BLOCK_SIZE, m2_encrypted_settings[0], m2_encrypted_settings[1], m2_encrypted_settings[2], m2_encrypted_settings[AES_BLOCK_SIZE - 3], m2_encrypted_settings[AES_BLOCK_SIZE - 2], m2_encrypted_settings[AES_BLOCK_SIZE - 1]);
		PLATFORM_AES_CBC_DECRYPT(keywrapkey, m2_encrypted_settings, plain, plain_len);
		PLATFORM_PRINTF_DETAIL("AP settings after  decryption (%d bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", plain_len, plain[0], plain[1], plain[2], plain[plain_len - 3], plain[plain_len - 2], plain[plain_len - 1]);

		// Remove padding
		//
		plain_len -= plain[plain_len - 1];

		// Parse contents of decrypted settings
		//
		ssid_present            = 0;
		bssid_present           = 0;
		auth_type_present       = 0;
		encryption_type_present = 0;
		network_key_present     = 0;
		m2_keywrap_present      = 0;
		p                       = plain;
		while ((INT32U)(p - plain) < plain_len) {
			INT16U attr_type;
			INT16U attr_len;

			_E2B(&p, &attr_type);
			_E2B(&p, &attr_len);

			if (ATTR_VENDOR_EXTENSION == attr_type) {
				INT8U vendor_id[3];
				_EnB(&p, vendor_id, 3);
				attr_len -= 3;
				if (WPS_VENDOR_ID_WFA_1 != vendor_id[0] || WPS_VENDOR_ID_WFA_2 != vendor_id[1] || WPS_VENDOR_ID_WFA_3 != vendor_id[2]) {
					p += attr_len;
					continue;
				}
				while (attr_len > 0) {
					PLATFORM_PRINTF_DEBUG("[M2] SUB ID %02x\n", *p);
					if (MULTI_AP_SUBELEMENT_ID == *(p++)) {
						PLATFORM_PRINTF_DEBUG("[M2] MULTI_AP_SUBELEMENT_ID found, length %02x\n", *p);
						break;
					} else {
						PLATFORM_PRINTF_DEBUG("[M2] NON MULTIAP ID, length %02x\n", *p);
						attr_len -= (2 + *p);
						p += (*p) + 1;
					}
				}

				if (MULTI_AP_SUBELEMENT_LEN != *(p++)) {
					PLATFORM_PRINTF_WARNING("[WSC] Malformed Multi AP IE!\n");
					p += (attr_len - 2);
					continue;
				}
				_E1B(&p, &bss_type);
				PLATFORM_PRINTF_DEBUG("[M2] length left %04x\n", attr_len);
				p += (attr_len - 3);
				continue;

			} else if (ATTR_SSID == attr_type) {
				_EnB(&p, ssid, attr_len);
				ssid[attr_len] = 0x00;
				ssid_present   = 1;
			} else if (ATTR_AUTH_TYPE == attr_type) {
				_E2B(&p, &auth_type);
				auth_type_present = 1;
			} else if (ATTR_ENCR_TYPE == attr_type) {
				_E2B(&p, &encryption_type);
				encryption_type_present = 1;
			} else if (ATTR_NETWORK_KEY == attr_type) {
				_EnB(&p, network_key, attr_len);
				network_key[attr_len] = 0x00;
				network_key_present   = 1;
			} else if (ATTR_MAC_ADDR == attr_type) {
				_EnB(&p, bssid, attr_len);
				bssid_present = 1;
			} else if (ATTR_KEY_WRAP_AUTH == attr_type) {
				// This attribute is always the last one contained in the plain
				// text, thus 4 bytes *before* where "p" is pointing right now
				// is the end of the plain text blob whose HMAC we are going to
				// compute to check the keywrap.
				//
				INT8U *end_of_hmac;
				INT8U  hash[SHA256_MAC_LEN];

				INT8U *addr[1];
				INT32U len[1];

				end_of_hmac = p - 4;

				addr[0] = plain;
				len[0]  = end_of_hmac - plain;

				PLATFORM_HMAC_SHA256(authkey, WPS_AUTHKEY_LEN, 1, addr, len, hash);

				if (PLATFORM_MEMCMP(p, hash, 8) != 0) {
					PLATFORM_PRINTF_WARNING("Message M2 keywrap failed\n");
					return 0;
				}

				p += attr_len;
				m2_keywrap_present = 1;
			} else {
				p += attr_len;
			}
		}
		if (
		    0 == ssid_present || 0 == bssid_present || 0 == auth_type_present || 0 == encryption_type_present || 0 == network_key_present || 0 == m2_keywrap_present) {
			if (!(bss_type & MULTI_AP_TEARDOWN_BIT)) {
				PLATFORM_PRINTF_WARNING("Missing attributes in the configuration settings received in the M2 message ssid_present: %d, bssid_present: %d, auth_type_present: %d, encryption_type_present: %d, network_key_present: %d, m2_keywrap_present: %d\n", ssid_present, bssid_present, auth_type_present, encryption_type_present, network_key_present, m2_keywrap_present);
				return 0;
			}
		}
	}
	if (bss_type & MULTI_AP_TEARDOWN_BIT) {
		PLATFORM_PRINTF_INFO("[M2] Radio tear down. %02x\n", bss_type);
		return 2;
	} else {
		if (bss_type & MULTI_AP_BACKHAUL_BSS_BIT) {
			PLATFORM_PRINTF_INFO("[M2] BSS support backhaul connection.\n");
		}
		if (bss_type & MULTI_AP_FRONTHAUL_BSS_BIT) {
			PLATFORM_PRINTF_INFO("[M2] BSS support fronthaul connection.\n");
		}

		config_data->ssid            = PLATFORM_STRDUP(ssid);
		config_data->network_key     = PLATFORM_STRDUP(network_key);
		config_data->auth_type       = auth_type;
		config_data->encryption_type = encryption_type;
		config_data->network_type    = bss_type;
	}

	// Apply the security settings so that this AP clones the registrar
	// configuration
	//

	return 1;
}

//
//////////////////////////////////////// Registrar functions ///////////////////
//
INT8U wscBuildM2(INT8U *m1, INT16U m1_size, INT8U **m2, INT16U *m2_size, struct configData *config_data)
{
	INT8U *buffer;

	struct interfaceInfo *i = NULL;
	struct deviceInfo info;
	PLATFORM_MEMSET(&info, 0, sizeof(info));

	INT8U *p;

	INT8U  aux8;
	INT16U aux16;
	INT32U aux32;

	INT8U *m1_mac_address = NULL;
	INT8U  m1_mac_address_present;
	INT8U *m1_nonce = NULL;
	INT8U  m1_nonce_present;
	INT8U *m1_pubkey = NULL;
	INT8U  m1_pubkey_present;
	INT16U m1_pubkey_len = 0;

	INT8U m1_band = 0;

	INT8U *local_privkey;
	INT16U local_privkey_len;

	INT8U authkey[WPS_AUTHKEY_LEN];
	INT8U keywrapkey[WPS_KEYWRAPKEY_LEN];
	INT8U emsk[WPS_EMSK_LEN];

	INT8U registrar_nonce[16];

	char *ifs_name = "";

	INT16U encryption_types;
	INT16U auth_types;

	// If this node is processing an M1 message, it must mean one of our
	// interfaces is the network registrar.
	//
	if (!DMisController()) {
		PLATFORM_PRINTF_WARNING("Not a multi ap controller. Ignoring M1 message.\n");
		return 0;
	}

	// We first need to extract the following parameters contained in "M1":
	//
	//   - Mac address
	//   - Nounce
	//   - Public key
	//
	m1_mac_address_present = 0;
	m1_nonce_present       = 0;
	m1_pubkey_present      = 0;
	p                      = m1;
	while (p - m1 < m1_size) {
		INT16U attr_type;
		INT16U attr_len;

		_E2B(&p, &attr_type);
		_E2B(&p, &attr_len);

		if (ATTR_MAC_ADDR == attr_type) {
			if (6 != attr_len) {
				PLATFORM_PRINTF_WARNING("Incorrect length (%d) for ATTR_MAC_ADDR\n", attr_len);
				return 0;
			}
			m1_mac_address = p;

			p += attr_len;
			m1_mac_address_present = 1;
		} else if (ATTR_ENROLLEE_NONCE == attr_type) {
			if (16 != attr_len) {
				PLATFORM_PRINTF_WARNING("Incorrect length (%d) for ATTR_ENROLLEE_NONCE\n", attr_len);
				return 0;
			}
			m1_nonce = p;

			p += attr_len;
			m1_nonce_present = 1;
		} else if (ATTR_PUBLIC_KEY == attr_type) {
			m1_pubkey_len = attr_len;
			m1_pubkey     = p;

			p += attr_len;
			m1_pubkey_present = 1;
		} else if (ATTR_RF_BANDS == attr_type) {
			if (1 != attr_len) {
				PLATFORM_PRINTF_WARNING("Incorrect length (%d) for ATTR_RF_BANDS\n", attr_len);
				return 0;
			}
			_E1B(&p, &m1_band);

			p += attr_len;
		} else {
			p += attr_len;
		}
	}
	if (
	    0 == m1_mac_address_present || 0 == m1_nonce_present || 0 == m1_pubkey_present) {
		PLATFORM_PRINTF_WARNING("Imcomplete M1 message received\n");
		return 0;
	}

	PLATFORM_GET_DEVICE_INFO(&info);

	if (customize_features & CUSTOMIZE_WSC_INFO) {
		//when eth on-board,registrar_interface_name is eth ifs,thus some info cannot get correctly throut PLATFORM_GET_1905_INTERFACE_INFO
		//exp:ATTR_AUTH_TYPE_FLAGS,ATTR_ENCR_TYPE_FLAGS
		if(m1_band == WPS_RF_24GHZ){
			ifs_name = map_radio_name_24g;
		}else if(m1_band == WPS_RF_50GHZ){
			ifs_name = map_radio_name_5gl;
		}else{
			PLATFORM_PRINTF_WARNING("invalid rf_band %d\n", m1_band);
		}

		if (NULL == (i = PLATFORM_GET_1905_INTERFACE_INFO(ifs_name))) {
			PLATFORM_PRINTF_WARNING("Could not retrieve info of interface %s\n", ifs_name);
			return 0;
		}
	}

	// Now we can build "M2"

	buffer = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * 1000);
	p      = buffer;

	// VERSION
	{
		aux16 = ATTR_VERSION;
		_I2B(&aux16, &p);
		aux16 = 1;
		_I2B(&aux16, &p);
		aux8 = 0x10;
		_I1B(&aux8, &p);
	}

	// MESSAGE TYPE
	{
		aux16 = ATTR_MSG_TYPE;
		_I2B(&aux16, &p);
		aux16 = 1;
		_I2B(&aux16, &p);
		aux8 = WPS_M2;
		_I1B(&aux8, &p);
	}

	// ENROLLEE NONCE
	{
		aux16 = ATTR_ENROLLEE_NONCE;
		_I2B(&aux16, &p);
		aux16 = 16;
		_I2B(&aux16, &p);
		_InB(m1_nonce, &p, 16);
	}

	// REGISTRAR NONCE
	{
		PLATFORM_GET_RANDOM_BYTES(registrar_nonce, 16);

		aux16 = ATTR_REGISTRAR_NONCE;
		_I2B(&aux16, &p);
		aux16 = 16;
		_I2B(&aux16, &p);
		_InB(registrar_nonce, &p, 16);
	}

	// UUID
	{
		aux16 = ATTR_UUID_R;
		_I2B(&aux16, &p);
		aux16 = 16;
		_I2B(&aux16, &p);
		_InB(info.uuid, &p, 16);
	}

	// PUBLIC KEY
	{
		INT8U *priv = NULL, *pub = NULL;
		INT16U priv_len, pub_len;

		PLATFORM_GENERATE_DH_KEY_PAIR(&priv, &priv_len, &pub, &pub_len);
		// TODO: ZERO PAD the pub key (doesn't seem to be really needed though)

		aux16 = ATTR_PUBLIC_KEY;
		_I2B(&aux16, &p);
		aux16 = pub_len;
		_I2B(&aux16, &p);
		_InB(pub, &p, pub_len);

		PLATFORM_FREE(pub);

		// We will use it later... save it.
		//
		local_privkey     = priv;
		local_privkey_len = priv_len;
	}

	// Key derivation (no bytes are written to the output buffer in the next
	// block of code, we just obtain three cryptographic keys that are needed
	// later
	{
		INT8U *shared_secret;
		INT16U shared_secret_len;

		INT8U *addr[3];
		INT32U len[3];

		INT8U dhkey[SHA256_MAC_LEN];
		INT8U kdk[SHA256_MAC_LEN];

		INT8U keys[WPS_AUTHKEY_LEN + WPS_KEYWRAPKEY_LEN + WPS_EMSK_LEN];

		// With the enrolle public key (which we received in M1) and our private
		// key (which we generated above), obtain the Diffie Hellman shared
		// secret (when receiving M2, the enrollee will be able to obtain this
		// same shared secret using its private key and our public key
		// -contained in M2-)
		//
		PLATFORM_COMPUTE_DH_SHARED_SECRET(&shared_secret, &shared_secret_len, m1_pubkey, m1_pubkey_len, local_privkey, local_privkey_len);
		// TODO: ZERO PAD the shared_secret (doesn't seem to be really needed
		// though)

		// Next, obtain the SHA-256 digest of this shared secret. We will call
		// this "dhkey"
		//
		addr[0] = shared_secret;
		len[0]  = shared_secret_len;

		PLATFORM_SHA256(1, addr, len, dhkey);

		// Next, concatenate three things (the enrollee nonce contained in M1,
		// the enrolle MAC address -also contained in M1-, and the nonce we just
		// generated before and calculate its HMAC (hash message authentication
		// code) using "dhkey" as the secret key.
		//
		addr[0] = m1_nonce;
		addr[1] = m1_mac_address;
		addr[2] = registrar_nonce;
		len[0]  = 16;
		len[1]  = 6;
		len[2]  = 16;

		PLATFORM_HMAC_SHA256(dhkey, SHA256_MAC_LEN, 3, addr, len, kdk);

		// Finally, take "kdk" and using a function provided in the "Wi-Fi
		// simple configuration" standard, obtain THREE KEYS that we will use
		// later ("authkey", "keywrapkey" and "emsk")
		//
		_wps_key_derivation_function(kdk, NULL, 0, "Wi-Fi Easy and Secure Key Derivation", keys, sizeof(keys));

		PLATFORM_MEMCPY(authkey, keys, WPS_AUTHKEY_LEN);
		PLATFORM_MEMCPY(keywrapkey, keys + WPS_AUTHKEY_LEN, WPS_KEYWRAPKEY_LEN);
		PLATFORM_MEMCPY(emsk, keys + WPS_AUTHKEY_LEN + WPS_KEYWRAPKEY_LEN, WPS_EMSK_LEN);

		PLATFORM_PRINTF_DETAIL("WPS keys: \n");
		PLATFORM_PRINTF_DETAIL("  Enrollee pubkey   (%3d bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", m1_pubkey_len, m1_pubkey[0], m1_pubkey[1], m1_pubkey[2], m1_pubkey[m1_pubkey_len - 3], m1_pubkey[m1_pubkey_len - 2], m1_pubkey[m1_pubkey_len - 1]);
		PLATFORM_PRINTF_DETAIL("  Registrar privkey (%3d bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", local_privkey_len, local_privkey[0], local_privkey[1], local_privkey[2], local_privkey[local_privkey_len - 3], local_privkey[local_privkey_len - 2], local_privkey[local_privkey_len - 1]);
		// PLATFORM_PRINTF_DETAIL("  Shared secret     (%3d bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", shared_secret_len, shared_secret[0], shared_secret[1], shared_secret[2], shared_secret[shared_secret_len - 3], shared_secret[shared_secret_len - 2], shared_secret[shared_secret_len - 1]);
		PLATFORM_PRINTF_DETAIL("  DH key            ( 32 bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", dhkey[0], dhkey[1], dhkey[2], dhkey[29], dhkey[30], dhkey[31]);
		PLATFORM_PRINTF_DETAIL("  Enrollee nonce    ( 16 bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", m1_nonce[0], m1_nonce[1], m1_nonce[2], m1_nonce[13], m1_nonce[14], m1_nonce[15]);
		PLATFORM_PRINTF_DETAIL("  Registrar nonce   ( 16 bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", registrar_nonce[0], registrar_nonce[1], registrar_nonce[2], registrar_nonce[13], registrar_nonce[14], registrar_nonce[15]);
		PLATFORM_PRINTF_DETAIL("  KDK               ( 32 bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", kdk[0], kdk[1], kdk[2], kdk[29], kdk[30], kdk[31]);
		PLATFORM_PRINTF_DETAIL("  authkey           (%3d bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", WPS_AUTHKEY_LEN, authkey[0], authkey[1], authkey[2], authkey[WPS_AUTHKEY_LEN - 3], authkey[WPS_AUTHKEY_LEN - 2], authkey[WPS_AUTHKEY_LEN - 1]);
		PLATFORM_PRINTF_DETAIL("  keywrapkey        (%3d bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", WPS_KEYWRAPKEY_LEN, keywrapkey[0], keywrapkey[1], keywrapkey[2], keywrapkey[WPS_KEYWRAPKEY_LEN - 3], keywrapkey[WPS_KEYWRAPKEY_LEN - 2], keywrapkey[WPS_KEYWRAPKEY_LEN - 1]);
		PLATFORM_PRINTF_DETAIL("  emsk              (%3d bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", WPS_EMSK_LEN, emsk[0], emsk[1], emsk[2], emsk[WPS_EMSK_LEN - 3], emsk[WPS_EMSK_LEN - 2], emsk[WPS_EMSK_LEN - 1]);
		PLATFORM_FREE(shared_secret);
	}

	PLATFORM_FREE(local_privkey);

	// AUTHENTICATION TYPES
	{
		auth_types = 0;

		if(customize_features & CUSTOMIZE_WSC_INFO){
			if (IEEE80211_AUTH_MODE_WPAPSK & i->interface_type_data.ieee80211.authentication_mode) {
				auth_types |= WPS_AUTH_WPAPSK;
			}
			if (IEEE80211_AUTH_MODE_WPA2PSK & i->interface_type_data.ieee80211.authentication_mode) {
				auth_types |= WPS_AUTH_WPA2PSK;
			}
#ifdef WPA3_SUPPORT
			if (IEEE80211_AUTH_MODE_WPA3PSK & i->interface_type_data.ieee80211.authentication_mode) {
				auth_types |= WPS_AUTH_WPA3;
			}
#endif
		} else {

			auth_types |= WPS_AUTH_OPEN;
			auth_types |= WPS_AUTH_WPA2PSK;

#ifdef WPA3_SUPPORT
			auth_types |= WPS_AUTH_WPA3;
#endif
		}

		aux16 = ATTR_AUTH_TYPE_FLAGS;
		_I2B(&aux16, &p);
		aux16 = 2;
		_I2B(&aux16, &p);
		_I2B(&auth_types, &p);
	}

	// ENCRYPTION TYPES
	{
		encryption_types = 0;

		if(customize_features & CUSTOMIZE_WSC_INFO){
			if (IEEE80211_ENCRYPTION_MODE_NONE & i->interface_type_data.ieee80211.encryption_mode) {
				encryption_types |= WPS_ENCR_NONE;
			}
			if (IEEE80211_ENCRYPTION_MODE_TKIP & i->interface_type_data.ieee80211.encryption_mode) {
				encryption_types |= WPS_ENCR_TKIP;
			}
			if (IEEE80211_ENCRYPTION_MODE_AES & i->interface_type_data.ieee80211.encryption_mode) {
				encryption_types |= WPS_ENCR_AES;
			}
		} else {
			encryption_types = config_data->encryption_type;
		}
		aux16 = ATTR_ENCR_TYPE_FLAGS;
		_I2B(&aux16, &p);
		aux16 = 2;
		_I2B(&aux16, &p);
		_I2B(&encryption_types, &p);
	}

	// CONNECTION TYPES
	{
		// Two possible types: ESS or IBSS. In the 1905 context, registrars
		// will always be "ESS", meaning they are willing to have their
		// credentials cloned by other APs in order to end up with a network
		// which is "roaming-friendly" ("ESS": "extended service set")

		aux16 = ATTR_CONN_TYPE_FLAGS;
		_I2B(&aux16, &p);
		aux16 = 1;
		_I2B(&aux16, &p);
		aux8 = WPS_CONN_ESS;
		_I1B(&aux8, &p);
	}

	// CONFIGURATION METHODS
	{
		// In the 1905 context, the configuration methods the AP is willing to
		// offer will always be these two

		aux16 = ATTR_CONFIG_METHODS;
		_I2B(&aux16, &p);
		aux16 = 2;
		_I2B(&aux16, &p);
		aux16 = WPS_CONFIG_PHY_PUSHBUTTON | WPS_CONFIG_VIRT_PUSHBUTTON;
		_I2B(&aux16, &p);
	}

	// MANUFACTURER
	{
		aux16 = ATTR_MANUFACTURER;
		_I2B(&aux16, &p);
		aux16 = PLATFORM_STRLEN(info.manufacturer_name);
		_I2B(&aux16, &p);
		_InB(info.manufacturer_name, &p, PLATFORM_STRLEN(info.manufacturer_name));
	}

	// MODEL NAME
	{
		aux16 = ATTR_MODEL_NAME;
		_I2B(&aux16, &p);
		aux16 = PLATFORM_STRLEN(info.model_name);
		_I2B(&aux16, &p);
		_InB(info.model_name, &p, PLATFORM_STRLEN(info.model_name));
	}

	// MODEL NUMBER
	{
		aux16 = ATTR_MODEL_NUMBER;
		_I2B(&aux16, &p);
		aux16 = PLATFORM_STRLEN(info.model_number);
		_I2B(&aux16, &p);
		_InB(info.model_number, &p, PLATFORM_STRLEN(info.model_number));
	}

	// SERIAL NUMBER
	{
		aux16 = ATTR_SERIAL_NUMBER;
		_I2B(&aux16, &p);
		aux16 = PLATFORM_STRLEN(info.serial_number);
		_I2B(&aux16, &p);
		_InB(info.serial_number, &p, PLATFORM_STRLEN(info.serial_number));
	}

	// PRIMARY DEVICE TYPE
	{
		// In the 1905 context, they node sending a M2 message will always be
		// (at least) a "network router"

		INT8U oui[4] = { 0x00, 0x50, 0xf2, 0x00 }; // Fixed value from the
		                                           // WSC spec

		aux16 = ATTR_PRIMARY_DEV_TYPE;
		_I2B(&aux16, &p);
		aux16 = 8;
		_I2B(&aux16, &p);
		aux16 = WPS_DEV_NETWORK_INFRA;
		_I2B(&aux16, &p);
		_InB(oui, &p, 4);
		aux16 = WPS_DEV_NETWORK_INFRA_ROUTER;
		_I2B(&aux16, &p);
	}

	// DEVICE NAME
	{
		aux16 = ATTR_DEV_NAME;
		_I2B(&aux16, &p);
		aux16 = PLATFORM_STRLEN(info.device_name);
		_I2B(&aux16, &p);
		_InB(info.device_name, &p, PLATFORM_STRLEN(info.device_name));
	}

	// RF BANDS
	{
		INT8U rf_bands;

		rf_bands = m1_band;

		aux16 = ATTR_RF_BANDS;
		_I2B(&aux16, &p);
		aux16 = 1;
		_I2B(&aux16, &p);
		_I1B(&rf_bands, &p);
	}

	// ASSOCIATION STATE
	{
		aux16 = ATTR_ASSOC_STATE;
		_I2B(&aux16, &p);
		aux16 = 2;
		_I2B(&aux16, &p);
		aux16 = WPS_ASSOC_CONN_SUCCESS;
		_I2B(&aux16, &p);
	}

	// CONFIG ERROR
	{
		aux16 = ATTR_CONFIG_ERROR;
		_I2B(&aux16, &p);
		aux16 = 2;
		_I2B(&aux16, &p);
		aux16 = WPS_CFG_NO_ERROR;
		_I2B(&aux16, &p);
	}

	// DEVICE PASSWORD ID
	{
		aux16 = ATTR_DEV_PASSWORD_ID;
		_I2B(&aux16, &p);
		aux16 = 2;
		_I2B(&aux16, &p);
		aux16 = DEV_PW_PUSHBUTTON;
		_I2B(&aux16, &p);
	}

	// OS VERSION
	{
		// TODO: Fill with actual properties from the interface

		INT32U os_version = 0x00000001;

		aux16 = ATTR_OS_VERSION;
		_I2B(&aux16, &p);
		aux16 = 4;
		_I2B(&aux16, &p);
		aux32 = 0x80000000 | os_version;
		_I4B(&aux32, &p);
	}

	// // Multi AP VENDOR EXTENSIONS
	// {
	// 	aux16 = ATTR_VENDOR_EXTENSION;
	// 	_I2B(&aux16, &p);
	// 	aux16 = 6;
	// 	_I2B(&aux16, &p);
	// 	aux8 = WPS_VENDOR_ID_WFA_1;
	// 	_I1B(&aux8, &p);
	// 	aux8 = WPS_VENDOR_ID_WFA_2;
	// 	_I1B(&aux8, &p);
	// 	aux8 = WPS_VENDOR_ID_WFA_3;
	// 	_I1B(&aux8, &p);
	// 	// Multi AP Subelement
	// 	aux8 = MULTI_AP_SUBELEMENT_ID;
	// 	_I1B(&aux8, &p);
	// 	aux8 = MULTI_AP_SUBELEMENT_LEN;
	// 	_I1B(&aux8, &p);

	// 	aux8 = config_data->network_type;
	// 	_I1B(&aux8, &p);
	// }

	// ENCRYPTED SETTINGS
	{
		// This is what we are going to do next:
		//
		//   1. Fill a tmp buffer ("aux") ith credential attributes (SSID,
		//      network key, etc...)
		//
		//   2. Add an HMAC to this tmp buffer (so that the enrollee, when
		//      receiving this buffer in M2, can be sure no one has tampered
		//      with these attributes)
		//
		//   3. Encryp the message + HMAC with AES (so that no one else
		//      snooping can have a look at these attributes)

		INT8U  plain[200];
		INT8U  hash[SHA256_MAC_LEN];
		INT8U *iv_start;
		INT8U *data_start;
		INT8U *r;
		INT8U  pad_elements_nr;

		INT8U *addr[1];
		INT32U len[1];

		char *ssid;
		char *network_key;

		ssid        = config_data->ssid;
		network_key = config_data->network_key;

		r = plain;

		// VENDOR EXTENSIONS
		{
			aux16 = ATTR_VENDOR_EXTENSION;
			_I2B(&aux16, &r);
			aux16 = 9;
			_I2B(&aux16, &r);
			aux8 = WPS_VENDOR_ID_WFA_1;
			_I1B(&aux8, &r);
			aux8 = WPS_VENDOR_ID_WFA_2;
			_I1B(&aux8, &r);
			aux8 = WPS_VENDOR_ID_WFA_3;
			_I1B(&aux8, &r);
			aux8 = WFA_ELEM_VERSION2;
			_I1B(&aux8, &r);
			aux8 = 1;
			_I1B(&aux8, &r);
			aux8 = WPS_VERSION;
			_I1B(&aux8, &r);

			aux8 = MULTI_AP_SUBELEMENT_ID;
			_I1B(&aux8, &r);
			aux8 = MULTI_AP_SUBELEMENT_LEN;
			_I1B(&aux8, &r);

			aux8 = config_data->network_type;
			_I1B(&aux8, &r);
		}

		// SSID
		aux16 = ATTR_SSID;
		_I2B(&aux16, &r);
		aux16 = PLATFORM_STRLEN(ssid);
		_I2B(&aux16, &r);
		_InB(ssid, &r, PLATFORM_STRLEN(ssid));

		// AUTH TYPE
		aux16 = ATTR_AUTH_TYPE;
		_I2B(&aux16, &r);
		aux16 = 2;
		_I2B(&aux16, &r);
		aux16 = config_data->auth_type;
		_I2B(&aux16, &r);

		// ENCRYPTION TYPE
		aux16 = ATTR_ENCR_TYPE;
		_I2B(&aux16, &r);
		aux16 = 2;
		_I2B(&aux16, &r);
		aux16 = config_data->encryption_type;
		_I2B(&aux16, &r);

		// NETWORK KEY
		aux16 = ATTR_NETWORK_KEY;
		_I2B(&aux16, &r);
		aux16 = PLATFORM_STRLEN(network_key);
		_I2B(&aux16, &r);
		_InB(network_key, &r, PLATFORM_STRLEN(network_key));

		// MAC ADDR
		aux16 = ATTR_MAC_ADDR;
		_I2B(&aux16, &r);
		aux16 = 6;
		_I2B(&aux16, &r);
		_InB(config_data->bssid, &r, 6);

		PLATFORM_PRINTF_DEBUG("AP configuration settings that we are going to send:\n");
		PLATFORM_PRINTF_DEBUG("  - SSID            : %s\n", ssid);
		PLATFORM_PRINTF_DEBUG("  - BSSID           : %02x:%02x:%02x:%02x:%02x:%02x\n", config_data->bssid[0], config_data->bssid[1], config_data->bssid[2],
		                      config_data->bssid[3], config_data->bssid[4], config_data->bssid[5]);
		PLATFORM_PRINTF_DEBUG("  - AUTH_TYPE       : 0x%04x\n", config_data->auth_type);
		PLATFORM_PRINTF_DEBUG("  - ENCRYPTION_TYPE : 0x%04x\n", config_data->encryption_type);
		PLATFORM_PRINTF_DEBUG("  - NETWORK_KEY     : %s\n", network_key);

		// Obtain the HMAC of the whole plain buffer using "authkey" as the
		// secret key.
		//
		addr[0] = plain;
		len[0]  = r - plain;
		PLATFORM_HMAC_SHA256(authkey, WPS_AUTHKEY_LEN, 1, addr, len, hash);

		// ...and add it to the same plain buffer (well, only the first 8 bytes
		// of the hash)
		//
		aux16 = ATTR_KEY_WRAP_AUTH;
		_I2B(&aux16, &r);
		aux16 = 8;
		_I2B(&aux16, &r);
		_InB(hash, &r, 8);

		// Finally, encrypt everything with AES and add the resulting blob to
		// the M2 buffer, as an "ATTR_ENCR_SETTINGS" attribute
		//
		//// Pad the length of the message to encrypt to a multiple of
		//// AES_BLOCK_SIZE. The new padded bytes must have their value equal to
		//// the amount of bytes padded (PKCS#5 v2.0 pad)
		////
		pad_elements_nr = AES_BLOCK_SIZE - ((r - plain) % AES_BLOCK_SIZE);
		for (aux8 = 0; aux8 < pad_elements_nr; aux8++) {
			_I1B(&pad_elements_nr, &r);
		}
		//// Add the attribute header ("type" and "lenght") to the M2 buffer,
		//// followed by the IV and the data to encrypt.
		////
		aux16 = ATTR_ENCR_SETTINGS;
		_I2B(&aux16, &p);
		aux16 = AES_BLOCK_SIZE + (r - plain);
		_I2B(&aux16, &p);
		iv_start = p;
		PLATFORM_GET_RANDOM_BYTES(p, AES_BLOCK_SIZE);
		p += AES_BLOCK_SIZE;
		data_start = p;
		_InB(plain, &p, r - plain);
		//// Encrypt the data IN-PLACE. Note that the "ATTR_ENCR_SETTINGS"
		//// attribute containes both the IV and the encrypted data.
		////
		PLATFORM_PRINTF_DEBUG("AP settings before encryption (%d bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", r - plain, data_start[0], data_start[1], data_start[2], data_start[(r - plain) - 3], data_start[(r - plain) - 2], data_start[(r - plain) - 1]);
		PLATFORM_PRINTF_DEBUG("IV (%d bytes)                           : 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", AES_BLOCK_SIZE, iv_start[0], iv_start[1], iv_start[2], iv_start[AES_BLOCK_SIZE - 3], iv_start[AES_BLOCK_SIZE - 2], iv_start[AES_BLOCK_SIZE - 1]);
		PLATFORM_AES_CBC_ENCRYPT(keywrapkey, iv_start, data_start, r - plain);
		PLATFORM_PRINTF_DEBUG("AP settings after  encryption (%d bytes): 0x%02x, 0x%02x, 0x%02x, ..., 0x%02x, 0x%02x, 0x%02x\n", r - plain, data_start[0], data_start[1], data_start[2], data_start[(r - plain) - 3], data_start[(r - plain) - 2], data_start[(r - plain) - 1]);
	}

	// AUTHENTICATOR
	{
		// This one is easy: concatenate M1 and M2 (everything in the M2 buffer
		// up to this point) and calculate the HMAC, then append it to M2 as
		// a new (and final!) attribute
		//
		INT8U hash[SHA256_MAC_LEN];

		INT8U *addr[2];
		INT32U len[2];

		addr[0] = m1;
		addr[1] = buffer;
		len[0]  = m1_size;
		len[1]  = p - buffer;

		PLATFORM_HMAC_SHA256(authkey, WPS_AUTHKEY_LEN, 2, addr, len, hash);

		aux16 = ATTR_AUTHENTICATOR;
		_I2B(&aux16, &p);
		aux16 = 8;
		_I2B(&aux16, &p);
		_InB(hash, &p, 8);
	}

	if (i)
		PLATFORM_FREE_1905_INTERFACE_INFO(i);

	*m2      = buffer;
	*m2_size = p - buffer;

	return 1;
}

INT8U wscFreeM2(INT8U *m, INT16U m_size)
{
	if (0 == m_size || NULL == m) {
		return 1;
	}

	PLATFORM_FREE(m);
	return 1;
}

//
//////////////////////////////////////// Common functions //////////////////////
//
INT8U wscGetType(INT8U *m, INT16U m_size)
{
	INT8U *p;

	p = m;
	while (p - m < m_size) {
		INT16U attr_type;
		INT16U attr_len;
		INT8U  msg_type;

		_E2B(&p, &attr_type);
		_E2B(&p, &attr_len);

		if (ATTR_MSG_TYPE == attr_type) {
			if (1 != attr_len) {
				PLATFORM_PRINTF_WARNING("Incorrect length (%d) for ATTR_MSG_TYPE\n", attr_len);
				return WSC_TYPE_UNKNOWN;
			}
			_E1B(&p, &msg_type);

			if (WPS_M1 == msg_type) {
				return WSC_TYPE_M1;
			} else if (WPS_M2 == msg_type) {
				return WSC_TYPE_M2;
			} else {
				return WSC_TYPE_UNKNOWN;
			}
		} else {
			p += attr_len;
		}
	}

	return WSC_TYPE_UNKNOWN;
}

INT8U wscGetBand(INT8U *m, INT16U m_size)
{
	INT8U *p;

	p = m;
	while (p - m < m_size) {
		INT16U attr_type;
		INT16U attr_len;
		INT8U  rf_band;

		_E2B(&p, &attr_type);
		_E2B(&p, &attr_len);

		if (ATTR_RF_BANDS == attr_type) {
			if (1 != attr_len) {
				PLATFORM_PRINTF_WARNING("Incorrect length (%d) for ATTR_RF_BANDS\n", attr_len);
				return 0xFF;
			}
			_E1B(&p, &rf_band);

			if (rf_band & WPS_RF_60GHZ) {
				return IEEE80211_FREQUENCY_BAND_60_GHZ;
			}

			if (rf_band & WPS_RF_50GHZ) {
				return IEEE80211_FREQUENCY_BAND_5_GHZ;
			}

			if (rf_band & WPS_RF_24GHZ) {
				return IEEE80211_FREQUENCY_BAND_2_4_GHZ;
			}

			return 0xFF;

		} else {
			p += attr_len;
		}
	}

	return 0xFF;
}

INT8U wscGetMacM1(INT8U *m, INT16U m_size, INT8U (*m1_mac_address)[6])
{
	INT8U *p;

	p = m;

	while (p - m < m_size) {
		INT16U attr_type;
		INT16U attr_len;

		_E2B(&p, &attr_type);
		_E2B(&p, &attr_len);

		if (ATTR_MAC_ADDR == attr_type) {
			if (6 != attr_len) {
				PLATFORM_PRINTF_WARNING("Incorrect length (%d) for ATTR_MAC_ADDR\n", attr_len);
				return 0;
			}
			_EnB(&p, m1_mac_address, 6);

			return 1;

		} else {
			p += attr_len;
		}
	}

	return 0;
}
