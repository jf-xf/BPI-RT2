/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>          // errno
#include <sys/ioctl.h>      // ioctl()
#include <sys/socket.h>     // socket()
#include <linux/wireless.h> // struct iwreq
#include <unistd.h>         // close()

#include "config_file_handler.h"
#include "apmib.h"
#include "ini.h"
#include "easymesh_datamodel.h"
#include "map_initialization.h"

#define DPP_CSIGN_KEY_PATH "/tmp/dpp_csign_key"
#define DPP_NET_ACCESS_KEY_PATH "/tmp/dpp_netaccess_key"
#define DPP_PRIVACY_PROTECTION_KEY_PATH "/tmp/dpp_pp_key"
#define DPP_CONNECTOR_PATH_1905 "/tmp/dpp_map_conn"

uint8_t _is_interface_up(char *interface)
{
	int          skfd = 0;
	struct ifreq ifr;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
		printf("Failed to create socket...\n");
		return 0;
	}

	strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);

	if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
		close(skfd);
		return 0;
	}
	close(skfd);
	return (uint8_t) !!(ifr.ifr_flags & IFF_UP);
}

uint8_t _set_radio_band_width(int radio_idx, int channel, int bandwidth, int sideband)
{
	printf("Setting Radio %d: channel: %d, bandwidth: %d, sideband: %d\n", radio_idx, channel, bandwidth, sideband);
	int w_idx  = wlan_idx;
	int vw_idx = vwlan_idx;

	wlan_idx  = radio_idx;
	vwlan_idx = 0;

	if (!apmib_init()) {
		printf("[ERROR] Initialize AP MIB failed in set_configure_state!\n");
		return 1;
	}

	apmib_set(MIB_WLAN_CHANNEL, (void *)&channel);

	if (bandwidth != -1)
		apmib_set(MIB_WLAN_CHANNEL_BONDING, (void *)&bandwidth);

	if (sideband != -1)
		apmib_set(MIB_WLAN_CONTROL_SIDEBAND, (void *)&sideband);

	apmib_update(CURRENT_SETTING);

	wlan_idx  = w_idx;
	vwlan_idx = vw_idx;

	return 0;
}

uint8_t _set_configure_state(uint8_t configure_state)
{
	int configured_state_int = (int)configure_state;
	if (!apmib_init()) {
		printf("[ERROR] Initialize AP MIB failed in set_configure_state!\n");
		return 1;
	}
	if (!apmib_set(MIB_MAP_CONFIGURED_BAND, (void *)&configured_state_int)) {
		printf("[ERROR] Cannot set configured band.\n");
		return 0;
	}
	apmib_update(CURRENT_SETTING);
	return 0;
}

uint8_t _set_ap_mib(uint8_t radio_idx, struct easymesh_interface_mib interface_mib, char *vap_prefix)
{
	int  val;
	char buffer[100];

	int encrypt_type   = 0;
	int wsc_auth_type  = 0;
	int enable_11w     = 0;
	int enable_sha_256 = 0;
	int wsc_enc_type = 0;
	int wpa_cipher_suite = 0;
	int wpa2_cipher_suite = 0;

	char interface_name[32] = {0};

	uint8_t connector_equal = 1;
	char connector_path[64] = { 0 };
	FILE *fd = NULL;

	if (radio_idx > 1) {
		// Only support wlan0 and wlan1
		printf("Unknown radio index %d\n", radio_idx);
		return 1;
	}

	if (!apmib_init()) {
		printf("[ERROR] Initialize AP MIB failed in apply_80211_configuration!\n");
		return 0;
	}

	wlan_idx  = radio_idx;
	vwlan_idx = interface_mib.interface_index;

	if (interface_mib.interface_index) {
		sprintf(interface_name, "wlan%d-%s%d", radio_idx, vap_prefix, interface_mib.interface_index - 1);
	} else {
		sprintf(interface_name, "wlan%d", radio_idx);
	}

	if (0 == interface_mib.is_enabled) {
		apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&val);
		if (val)
			return 0;
		else {
			val = 1;
			if (!apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&val)) {
				return 0;
			}
			apmib_update(CURRENT_SETTING);
			return 0;
		}
	}

	// interface enable
	apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&val);
	if (val) {
		val = 0;
		if (!apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&val)) {
			return 0;
		}
	}

	apmib_get(MIB_WLAN_SSID, (void *)buffer);

//Set the correct encrypt_type to determine if the mib needs to be set.
	encrypt_type = 0;
	if(interface_mib.authentication_type & AUTH_WPA) {
		encrypt_type |= ENCRYPT_WPA;
	}
	if(interface_mib.authentication_type & AUTH_WPA2) {
		encrypt_type |= ENCRYPT_WPA2;
	}
#ifdef WLAN_WPA3
	if(interface_mib.authentication_type & AUTH_WPA3) {
		encrypt_type |= ENCRYPT_WPA3;
	}
#endif

	if (interface_mib.authentication_type & AUTH_DPP) {
		char content[512] = { 0 };
		snprintf(connector_path, sizeof(connector_path), "/tmp/dpp_%s_conn", interface_name);
		fd = fopen(connector_path, "r");
		connector_equal = 0;

		if (fd) {
			fgets(content, sizeof(content), fd);
			if (0 == strncmp(content, interface_mib.signed_connector, strlen(interface_mib.signed_connector))) {
				connector_equal = 1;
			}
			fclose(fd);
		}
	}

	if (0 == strcmp(interface_mib.ssid, buffer)) {
		apmib_get(MIB_WLAN_WPA_PSK, (void *)buffer);
		apmib_get(MIB_WLAN_ENCRYPT, (void *)&val);
		if (0 == strcmp(interface_mib.network_key, buffer) && val == encrypt_type) {
			apmib_get(MIB_WLAN_MAP_BSS_TYPE, (void *)&val);
			if (interface_mib.network_type == (uint8_t)val && connector_equal) {
				printf("Setting is identical for interface %s, skip configuration\n", interface_name);
				return 0;
			}
		}
	}

#ifdef WLAN_WPA3
	if (interface_mib.authentication_type == (AUTH_WPA2 | AUTH_WPA3)) {
		encrypt_type   = ENCRYPT_WPA2_WPA3_MIXED;
		wsc_auth_type  = WSC_AUTH_WPA3SAE | WSC_AUTH_WPA2PSK;
		enable_sha_256 = 0;
		enable_11w     = MGMT_FRAME_PROTECTION_OPTIONAL;
	} else if (interface_mib.authentication_type == AUTH_WPA3) {
		encrypt_type   = ENCRYPT_WPA3;
		wsc_auth_type  = WSC_AUTH_WPA3SAE;
		enable_sha_256 = 1;
		enable_11w     = MGMT_FRAME_PROTECTION_REQUIRED;
	} else {
		encrypt_type   = ENCRYPT_WPA2;
		wsc_auth_type  = WSC_AUTH_WPA2PSK;
		enable_sha_256 = 0;
		enable_11w     = NO_MGMT_FRAME_PROTECTION;
	}
#else //WLAN_WPA3
	encrypt_type   = ENCRYPT_WPA2;
	wsc_auth_type  = WSC_AUTH_WPA2PSK;
	enable_sha_256 = 0;
	enable_11w     = NO_MGMT_FRAME_PROTECTION;
#endif //WLAN_WPA3

#ifdef RTK_MULTI_AP_WFA
	if ((interface_mib.network_type & MULTI_AP_FRONTHAUL_BSS_BIT) || (interface_mib.network_type & MULTI_AP_BACKHAUL_BSS_BIT)) {
		enable_11w = MGMT_FRAME_PROTECTION_OPTIONAL;
	}
#endif

	wsc_enc_type = WSC_ENCRYPT_AES;
	wpa_cipher_suite = 0;
	wpa2_cipher_suite = WPA_CIPHER_AES;

	if(!(interface_mib.network_type & MULTI_AP_BACKHAUL_BSS_BIT)) {
		if(interface_mib.authentication_type == (AUTH_WPA | AUTH_WPA2)) {
			encrypt_type            = ENCRYPT_WPA2_MIXED;
			wsc_auth_type           = WSC_AUTH_WPA2PSKMIXED;
			wsc_enc_type            = WPA_CIPHER_MIXED;
			wpa_cipher_suite        = WPA_CIPHER_MIXED;
			wpa2_cipher_suite       = WPA_CIPHER_MIXED;
		} else if (interface_mib.authentication_type == AUTH_WPA) {
			encrypt_type            = ENCRYPT_WPA;
			wsc_auth_type           = WSC_AUTH_WPAPSK;
			wsc_enc_type            = WSC_ENCRYPT_TKIP;
		} else if (interface_mib.authentication_type == AUTH_WEP){
			encrypt_type            = ENCRYPT_WEP;
			wsc_auth_type           = WSC_AUTH_OPEN;
			wsc_enc_type            = WSC_ENCRYPT_NONE;
		} else if (interface_mib.authentication_type == AUTH_DISABLED){
			encrypt_type            = ENCRYPT_DISABLED;
			wsc_auth_type           = WSC_AUTH_OPEN;
			wsc_enc_type            = WSC_ENCRYPT_NONE;
		}
	}

#ifdef MULTI_AP_OPEN_ENCRYPT_SUPPORT
	if (interface_mib.authentication_type == AUTH_DISABLED){
		encrypt_type       = ENCRYPT_DISABLED;
		wsc_auth_type      = WSC_AUTH_OPEN;
	}
#endif

	// Set WLAN MODE
	val = AP_MODE;
	if (!apmib_set(MIB_WLAN_MODE, (void *)&val)) {
		return 0;
	}
	// Set SSID

	if (!apmib_set(MIB_WLAN_SSID, (void *)interface_mib.ssid)) {
		return 0;
	}
	// set bss_type
	int bss_type_int = (int)interface_mib.network_type;
	if (!apmib_set(MIB_WLAN_MAP_BSS_TYPE, (void *)&bss_type_int)) {
		return 0;
	}
#ifndef RTK_MULTI_AP_WFA
	// HIDDEN SSID
	if (MULTI_AP_BACKHAUL_BSS_BIT == bss_type_int) {
		val = 1;
		if (!apmib_set(MIB_WLAN_HIDDEN_SSID, (void *)&val)) {
			return 0;
		}
	}
#endif
	//
	val = encrypt_type;
	if (!apmib_set(MIB_WLAN_ENCRYPT, (void *)&val)) {
		return 0;
	}
	//
	val = WPA_AUTH_PSK;
	if (!apmib_set(MIB_WLAN_WPA_AUTH, (void *)&val)) {
		return 0;
	}
	//
	val = wpa_cipher_suite;
	if (!apmib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&val)) {
		return 0;
	}
	//
	val = wpa2_cipher_suite;
	if (!apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&val)) {
		return 0;
	}
	//
	val = 0; // PSK_FORMAT_PASSPHRASE
	if (!apmib_set(MIB_WLAN_PSK_FORMAT, (void *)&val)) {
		return 0;
	}
	//
	if (!apmib_set(MIB_WLAN_WPA_PSK, (void *)interface_mib.network_key)) {
		return 0;
	}
	//
	val = wsc_auth_type;
	if (!apmib_set(MIB_WLAN_WSC_AUTH, (void *)&val)) {
		return 0;
	}
	//
	val = wsc_enc_type;
	if (!apmib_set(MIB_WLAN_WSC_ENC, (void *)&val)) {
		return 0;
	}
	//
	if (!apmib_set(MIB_WLAN_WSC_PSK, (void *)interface_mib.network_key)) {
		return 0;
	}
	//
	val = 1;
	if (!apmib_set(MIB_WLAN_WSC_CONFIGURED, (void *)&val)) {
		return 0;
	}
#ifdef CONFIG_IEEE80211W
	//
	val = enable_sha_256;
	if (!apmib_set(MIB_WLAN_SHA256_ENABLE, (void *)&val)) {
		return 0;
	}
	//
	val = enable_11w;
	if (!apmib_set(MIB_WLAN_IEEE80211W, (void *)&val)) {
		return 0;
	}
#endif
	apmib_update(CURRENT_SETTING);

	if (interface_mib.authentication_type & AUTH_DPP) {
		// printf("%s support dpp! connector: %s\n", interface_name, interface_mib.signed_connector);
		if (strncmp(interface_mib.signed_connector, "null", strlen(interface_mib.signed_connector))) {
			fd = fopen(connector_path, "w");
			if (fd) {
				fprintf(fd, "%s", interface_mib.signed_connector);
				fclose(fd);
			} else {
				printf("Failed to write signned connector for %s\n", interface_name);
			}
		} else {
			printf("%s is set to use DPP auth without signed connector!!\n", interface_name);
		}
	}

	return 1;
}

uint8_t _set_vxd_mib(uint8_t radio_idx, struct easymesh_interface_mib interface_mib)
{
	// set vxd mib
	int  val;
	char buffer[100];

	int encrypt_type   = 0;
	int wsc_auth_type  = 0;
	int enable_11w     = 0;
	int enable_sha_256 = 0;

	if (radio_idx > 1) {
		// Only support wlan0 and wlan1
		printf("Unknown radio index %d\n", radio_idx);
		return 1;
	}

	if (!apmib_init()) {
		printf("[ERROR] Initialize AP MIB failed in apply_80211_configuration!\n");
		return 0;
	}

	wlan_idx = radio_idx;

	apmib_get(MIB_WLAN_SSID, (void *)buffer);

#ifdef WLAN_WPA3
	if (interface_mib.authentication_type == (AUTH_WPA2 | AUTH_WPA3)) {
		encrypt_type   = ENCRYPT_WPA2_WPA3_MIXED;
		wsc_auth_type  = WSC_AUTH_WPA3SAE | WSC_AUTH_WPA2PSK;
		enable_sha_256 = 0;
		enable_11w     = MGMT_FRAME_PROTECTION_OPTIONAL;
	} else if (interface_mib.authentication_type == AUTH_WPA3) {
		encrypt_type   = ENCRYPT_WPA3;
		wsc_auth_type  = WSC_AUTH_WPA3SAE;
		enable_sha_256 = 1;
		enable_11w     = MGMT_FRAME_PROTECTION_REQUIRED;
	} else if(interface_mib.authentication_type == AUTH_DISABLED){
		encrypt_type		= ENCRYPT_DISABLED;
		wsc_auth_type		= WSC_AUTH_OPEN;
		enable_sha_256		= 0;
		enable_11w			= NO_MGMT_FRAME_PROTECTION;
	} else {
		encrypt_type   = ENCRYPT_WPA2;
		wsc_auth_type  = WSC_AUTH_WPA2PSK;
		enable_sha_256 = 0;
		enable_11w     = NO_MGMT_FRAME_PROTECTION;
	}
#else
	encrypt_type        = ENCRYPT_WPA2;
	wsc_auth_type       = WSC_AUTH_WPA2PSK;
	enable_sha_256      = 0;
	enable_11w          = NO_MGMT_FRAME_PROTECTION;
#endif

	if (0 == strcmp(interface_mib.ssid, buffer)) {

		if (0 == wlan_idx) {
			apmib_get(MIB_REPEATER_SSID1, (void *)buffer);
		} else if (1 == wlan_idx) {
			apmib_get(MIB_REPEATER_SSID2, (void *)buffer);
		}
		if (0 == strcmp(interface_mib.ssid, buffer)) {
			apmib_get(MIB_WLAN_WPA_PSK, (void *)buffer);
			apmib_get(MIB_WLAN_ENCRYPT, (void *)&val);
			if (0 == strcmp(interface_mib.network_key, buffer) && val == encrypt_type) {
				apmib_get(MIB_WLAN_MAP_BSS_TYPE, (void *)&val);
				if (interface_mib.network_type == (uint8_t)val) {
					printf("Setting is identical for interface wlan%d-vxd, skip configuration\n", wlan_idx);
					return 2;
				}
			}
		}
	}
	// Set WLAN MODE
	val = CLIENT_MODE;
	if (!apmib_set(MIB_WLAN_MODE, (void *)&val)) {
		return 0;
	}
	// Set SSID
	if (!apmib_set(MIB_WLAN_SSID, (void *)interface_mib.ssid)) {
		return 0;
	}
	if (0 == wlan_idx) {
		if (!apmib_set(MIB_REPEATER_SSID1, (void *)interface_mib.ssid)) {
			return 0;
		}
	} else if (1 == wlan_idx) {
		if (!apmib_set(MIB_REPEATER_SSID2, (void *)interface_mib.ssid)) {
			return 0;
		}
	}
	if (!apmib_set(MIB_WLAN_WSC_SSID, (void *)interface_mib.ssid)) {
		return 0;
	}

	// Set BSS Type
	int bss_type_int = MULTI_AP_BACKHAUL_STA_BIT;
	if (!apmib_set(MIB_WLAN_MAP_BSS_TYPE, (void *)&bss_type_int)) {
		return 0;
	}

	val = encrypt_type;

	if (!apmib_set(MIB_WLAN_ENCRYPT, (void *)&val)) {
		return 0;
	}

	val = WPA_AUTH_PSK;

	if (!apmib_set(MIB_WLAN_WPA_AUTH, (void *)&val)) {
		return 0;
	}

	val = WPA_CIPHER_AES;

	if (!apmib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&val)) {
		return 0;
	}

	if (!apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&val)) {
		return 0;
	}

	val = 0; // PSK_FORMAT_PASSPHRASE
	if (!apmib_set(MIB_WLAN_PSK_FORMAT, (void *)&val)) {
		return 0;
	}

	if (!apmib_set(MIB_WLAN_WPA_PSK, (void *)interface_mib.network_key)) {
		return 0;
	}

	val = wsc_auth_type;
	if (!apmib_set(MIB_WLAN_WSC_AUTH, (void *)&val)) {
		return 0;
	}

	val = WSC_ENCRYPT_AES;
	if (!apmib_set(MIB_WLAN_WSC_ENC, (void *)&val)) {
		return 0;
	}
	// buffer =
	// strdup(data_container.radio_data[wlan_idx].bss_data[vwlan_idx].network_key);
	if (!apmib_set(MIB_WLAN_WSC_PSK, (void *)interface_mib.network_key)) {
		return 0;
	}

	val = 1;
	if (!apmib_set(MIB_WLAN_WSC_CONFIGURED, (void *)&val)) {
		return 0;
	}
#ifdef CONFIG_IEEE80211W
	//
	val = enable_sha_256;
	if (!apmib_set(MIB_WLAN_SHA256_ENABLE, (void *)&val)) {
		return 0;
	}
	//
	val = enable_11w;
	if (!apmib_set(MIB_WLAN_IEEE80211W, (void *)&val)) {
		return 0;
	}
#endif
	apmib_update(CURRENT_SETTING);
	return 1;
}

uint8_t _reload_setting(char *interfaces_name, uint8_t is_enabled)
{
	char cmd[128];

	if (NULL == strstr(interfaces_name, "wlan")) {
		return 1;
	}

	if (0 == _is_interface_up(interfaces_name) && !is_enabled) {
		return 2;
	}

	if (!is_enabled) {
		sprintf(cmd, "brctl delif br0 %s; ifconfig %s down", interfaces_name, interfaces_name);
		system(cmd);
		return 0;
	}

	sprintf(cmd, "brctl delif br0 %s; flash set_mib %s", interfaces_name,
	        interfaces_name);
	printf("%s\n", cmd);
	system(cmd);

	if (strstr(interfaces_name, "va")) {
		char root_interface[] = "wlan0";
		root_interface[4]     = interfaces_name[4];
		sprintf(cmd, "ifconfig %s up && brctl addif br0 %s", root_interface, root_interface);
		printf("%s\n", cmd);
		system(cmd);
	}
	sprintf(cmd, "ifconfig %s up && brctl addif br0 %s", interfaces_name, interfaces_name);
	printf("%s\n", cmd);
	system(cmd);

	return 0;
}

uint8_t _easymesh_set_mib()
{
	int                       i, j;
	struct easymesh_datamodel easymesh_db = { 0 };
	easymesh_db.configured_band  = 0;
	easymesh_db.radio_data_nr    = 0;
	easymesh_db.radio_data       = NULL;
	easymesh_db.per_radio_config = NULL;

	FILE *fd = NULL;

	if (ini_parse("/var/multiap.conf", read_config_file, &easymesh_db) < 0) {
		printf("[RTK] Can't load multiap configuration file!! \n");
		return INIT_ERROR_CONFIG_FILE;
	}

	if (ini_parse("/var/multiap_setmib.conf", read_set_mib_config_file, &easymesh_db) < 0) {
		printf("[RTK] Can't load multiap_setmib configuration file!! \n");
		return INIT_ERROR_CONFIG_FILE;
	}
	if (!apmib_init()) {
		printf("[ERROR] Initialize AP MIB failed in apply_80211_configuration!\n");
		return 0;
	}

	// set configure_state
	int configured_state_int = (int)easymesh_db.configured_band;
	if (!apmib_set(MIB_MAP_CONFIGURED_BAND, (void *)&configured_state_int)) {
		printf("[ERROR] Cannot set configured band.\n");
		return 0;
	}

	(void)remove(DPP_CONNECTOR_PATH_1905);
	(void)remove(DPP_CSIGN_KEY_PATH);
	(void)remove(DPP_NET_ACCESS_KEY_PATH);
	(void)remove(DPP_PRIVACY_PROTECTION_KEY_PATH);

	if (easymesh_db.signed_connector_1905) {
		// printf("signed_connector_1905: %s\n", easymesh_db.signed_connector_1905);
		fd = fopen(DPP_CONNECTOR_PATH_1905, "w");
		if (fd) {
			fprintf(fd, "%s", easymesh_db.signed_connector_1905);
			fclose(fd);
		}
	}

	if (easymesh_db.csign_key) {
		// printf("csign: %s\n", easymesh_db.csign_key);
		fd = fopen(DPP_CSIGN_KEY_PATH, "w");
		if (fd) {
			fprintf(fd, "%s", easymesh_db.csign_key);
			fclose(fd);
		}
	}

	if (easymesh_db.netaccess_key) {
		// printf("netaccess_key: %s\n", easymesh_db.netaccess_key);
		fd = fopen(DPP_NET_ACCESS_KEY_PATH, "w");
		if (fd) {
			fprintf(fd, "%s", easymesh_db.netaccess_key);
			fclose(fd);
		}
	}

	if (easymesh_db.pp_key) {
		// printf("ppk: %s\n", easymesh_db.pp_key);
		fd = fopen(DPP_PRIVACY_PROTECTION_KEY_PATH, "w");
		if (fd) {
			fprintf(fd, "%s", easymesh_db.pp_key);
			fclose(fd);
		}
	}

	// set channel & bonding

	// set mib
	int  radio_band, interface_index;
	char interface_name[32];
	char *radio_name = NULL;

	strncpy(interface_name, "wlan0-vap0\0", sizeof(interface_name));
	for (i = 0; i < easymesh_db.radio_data_nr; i++) {
		radio_band = easymesh_db.radio_data[i].radio_band;
		if (MAP_CONFIG_2G == radio_band) {
			wlan_idx   = easymesh_db.radio_name_24g[4] - '0';
			radio_name = easymesh_db.radio_name_24g;

		} else if (MAP_CONFIG_5GL == radio_band || MAP_CONFIG_5GH == radio_band) {
			wlan_idx   = easymesh_db.radio_name_5gl[4] - '0';
			radio_name = easymesh_db.radio_name_5gl;
		} else {
			printf("unsupported band!\n");
			continue;
		}

		if (easymesh_db.radio_data[i].need_change_channel) {
			_set_radio_band_width(wlan_idx, easymesh_db.radio_data[i].radio_channel, easymesh_db.radio_data[i].channel_bandwidth, easymesh_db.radio_data[i].control_sideband);
		}

		for (j = 0; j < easymesh_db.radio_data[i].interface_nr; j++) {
			interface_index = easymesh_db.radio_data[i].interface_mib[j].interface_index;

			if (NUM_VWLAN_INTERFACE == interface_index) {
				vwlan_idx = 5;
#ifndef RTK_MULTI_AP_WFA
				if (1 == easymesh_db.radio_data[i].interface_mib[j].need_configure) {
					_set_vxd_mib(wlan_idx, easymesh_db.radio_data[i].interface_mib[j]);
					//
					sprintf(interface_name, "%s-vxd", radio_name);
					_reload_setting(interface_name, easymesh_db.radio_data[i].interface_mib[j].is_enabled);
				}
#endif
			} else {
				vwlan_idx = j;
				if (1 == easymesh_db.radio_data[i].interface_mib[j].need_configure) {
					_set_ap_mib(wlan_idx, easymesh_db.radio_data[i].interface_mib[j], easymesh_db.vap_prefix);
					//
					if (0 == j) {
						sprintf(interface_name, "%s", radio_name);
					} else {
						sprintf(interface_name, "%s-%s%d", radio_name, easymesh_db.vap_prefix, j - 1);
					}
					_reload_setting(interface_name, easymesh_db.radio_data[i].interface_mib[j].is_enabled);
				}
			}
		}
	}
	return 1;
}

void _map_restart_wlan()
{
	if(-1 != access("/tmp/map_vlan_reset", F_OK) && -1 != access("/tmp/map_vlan_need_reset", F_OK)) {
		system("sh /tmp/map_vlan_reset");
		system("rm /tmp/map_vlan_reset");
		system("rm /tmp/map_vlan_need_reset");
	}

	system("sysconf init gw all");
}

int main(int argc, char *argv[])
{
	int c;
	int reinit = 0;
	int band = -1, cur_channel = -1, bandwidth = -1, sideband = -1;
	int configured_band = 0;

	while ((c = getopt(argc, argv, "flb:c:t")) != -1) {
		switch (c) {
		case 'f': {
			reinit = FULL_RELOAD;
			break;
		}
		case 'l': {
			reinit = SOFT_RELOAD;
			break;
		}
		case 'v': {
			reinit = VENDOR_DATA_RELOAD;
			break;
		}
		case 'b': {
			char *token, *saveptr;
			int * band_str = NULL;
			int   str_num = 0, i = 0;
			for (token = (char *)optarg;; token = NULL) {
				token = strtok_r(token, "_", &saveptr);
				if (NULL == token)
					break;
				i++;
				band_str          = (int *)realloc(band_str, i * sizeof(int));
				band_str[str_num] = atoi(token);
				str_num += 1;
			}
			if (2 == str_num) {
				band        = band_str[0];
				cur_channel = band_str[1];
			} else if (4 == str_num) {
				band        = band_str[0];
				cur_channel = band_str[1];
				bandwidth   = band_str[2];
				sideband    = band_str[3];
			} else {
				printf("wrong cmd for map_reinit daemon!!!!\n");
				free(band_str);
				return 0;
			}
			reinit = BAND_MIB_SET;
			free(band_str);
			break;
		}
		case 'c': {
			if (optarg) {
				configured_band = atoi(optarg);
			}
			reinit = CONFIG_STATE_SET;
			break;
		}
		case 't': {
			reinit = TEST_RELOAD;
			break;
		}
		}
	}

	if (TEST_RELOAD == reinit) {
		_map_restart_wlan();
	} else if (FULL_RELOAD == reinit) {
		system("killall map_checker; echo 1 > /tmp/map_checker_end.txt");
		_easymesh_set_mib();
		printf("\nNeed full reload!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		_map_restart_wlan();
	} else if (SOFT_RELOAD == reinit) {
		system("killall map_checker; echo 1 > /tmp/map_checker_end.txt");
		_easymesh_set_mib();
		printf("\nSoft reload!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		_map_restart_wlan();
	} else if (BAND_MIB_SET == reinit) {
		struct easymesh_datamodel easymesh_db = { 0 };
		char *                    radio_name = NULL;
		if (ini_parse("/var/multiap.conf", read_config_file, &easymesh_db) < 0) {
			printf("[RTK] Can't load configuration file!! \n");
			return INIT_ERROR_CONFIG_FILE;
		}
		if (MAP_CONFIG_2G == band) {
			radio_name = easymesh_db.radio_name_24g;
		} else if (MAP_CONFIG_5GL == band) {
			radio_name = easymesh_db.radio_name_5gl;
			sideband   = 0; // Auto
		} else if (MAP_CONFIG_5GH == band) {
			radio_name = easymesh_db.radio_name_5gh;
			sideband   = 0; // Auto
		} else {
			printf("Unsupported band %02x!\n", band);
			return 1;
		}
		int radio_idx = radio_name[4] - '0';
		_set_radio_band_width(radio_idx, cur_channel, bandwidth, sideband);
	} else if (CONFIG_STATE_SET == reinit) {
		_set_configure_state(configured_band);
	}
	return 0;
}
