/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "apmib.h"
#include "map_tlvs.h"

enum MAP_RESET_MODE {
	MAP_RESET_NULL,
	MAP_RESET_CONTROLLER,
	MAP_RESET_AGENT
};

int rtk_set_int_mib(int mib_name, int mib_val)
{
	int old_value;
	apmib_get(mib_name, (void *)&old_value);
	if (old_value != mib_val) {
		apmib_set(mib_name, (void *)&mib_val);
		//printf("%d and %d are different!\n", old_value, mib_val);
		return 1;
	}
	// printf("%d and %d are the same!\n", old_value, mib_val);
	return 0;
}

int rtk_set_string_mib(int mib_name, char *str_val)
{
	char old_value[64];
	apmib_get(mib_name, (void *)old_value);
	if (strcmp(old_value, str_val)) {
		apmib_set(mib_name, (void *)str_val);
		// printf("%s and %s are different!\n", old_value, str_val);
		return 1;
	}
	// printf("%s and %s are the same!\n", old_value, str_val);
	return 0;
}

int main(int argc, char *argv[])
{
	int c;
	int reset_mode = MAP_RESET_NULL;
	int i = 0, j = 0;
	int mib_val      = 0;
	int old_wlan_idx = 0, old_vwlan_idx = 0;

	int multiap_profile = 0;

	int need_reload = 0;

	char ssid[] = "RTK_0_0";

	if (-1 == daemon(0, 1)) {
		printf("Run map_reset as daemon error!\n");
		return 1;
	}

	while ((c = getopt(argc, argv, "acp:")) != -1) {
		switch (c) {
		case 'c': {
			reset_mode += MAP_RESET_CONTROLLER;
			break;
		}
		case 'a': {
			reset_mode += MAP_RESET_AGENT;
			break;
		}
		case 'p': {
			if (optarg) {
				switch (atoi(optarg)) {
				case 1:
					multiap_profile = MULTI_AP_PROFILE_1;
					break;
				case 2:
					multiap_profile = MULTI_AP_PROFILE_2;
					break;
				case 3:
					multiap_profile = MULTI_AP_PROFILE_3;
					break;
				case 4:
					multiap_profile = MULTI_AP_PROFILE_4;
					break;
				}
			}
			break;
		}
		}
	}

	if (reset_mode > MAP_RESET_AGENT) {
		printf("Cannot reset controller and agent at same time!\n");
		return 1;
	}

	if (MAP_RESET_NULL == reset_mode) {
		printf("Must reset as cotroller or agent!\n");
		return 1;
	}

	old_wlan_idx = wlan_idx;

	old_vwlan_idx = vwlan_idx;

	apmib_init();

	if (1 == reset_mode) {
		//mib_val = 129;
		if (MULTI_AP_PROFILE_4 == multiap_profile) {
			printf("Set as r4 controller\n");
			mib_val = 135;
		} else if (MULTI_AP_PROFILE_3 == multiap_profile) {
			printf("Set as r3 controller\n");
			mib_val = 133;
		} else if (MULTI_AP_PROFILE_2 == multiap_profile) {
			printf("Set as r2 controller\n");
			mib_val = 131;
		} else {
			mib_val = 129;
		}
		need_reload |= rtk_set_int_mib(MIB_MAP_CONTROLLER, mib_val);
		mib_val = 0;
		need_reload |= rtk_set_int_mib(MIB_REPEATER_ENABLED1, mib_val);
		need_reload |= rtk_set_int_mib(MIB_REPEATER_ENABLED2, mib_val);
	} else if (2 == reset_mode) {
		//mib_val = 130;
		if (MULTI_AP_PROFILE_4 == multiap_profile) {
			printf("Set as r2 agent\n");
			mib_val = 136;
		} else if (MULTI_AP_PROFILE_3 == multiap_profile) {
			printf("Set as r2 agent\n");
			mib_val = 134;
		} else if (MULTI_AP_PROFILE_2 == multiap_profile) {
			printf("Set as r2 agent\n");
			mib_val = 132;
		} else {
			mib_val = 130;
		}
		need_reload |= rtk_set_int_mib(MIB_MAP_CONTROLLER, mib_val);
		for (i = 0; i < 2; i++) {
			// Root
			wlan_idx  = i;
			vwlan_idx = 0;
			need_reload |= rtk_set_int_mib(MIB_WLAN_WSC_DISABLE, 0);
			// VXD
			vwlan_idx = 5;
			need_reload |= rtk_set_int_mib(MIB_WLAN_WSC_DISABLE, 0);
			need_reload |= rtk_set_int_mib(MIB_WLAN_AUTH_TYPE, 0);
			need_reload |= rtk_set_int_mib(MIB_WLAN_ENCRYPT, 0);
			need_reload |= rtk_set_int_mib(MIB_WLAN_WPA_CIPHER_SUITE, 1);
			need_reload |= rtk_set_int_mib(MIB_WLAN_IEEE80211W, 0);
			need_reload |= rtk_set_int_mib(MIB_WLAN_SHA256_ENABLE, 0);
			need_reload |= rtk_set_int_mib(MIB_WLAN_WSC_CONFIGURED, 0);
			need_reload |= rtk_set_int_mib(MIB_WLAN_WSC_AUTH, 0);
			need_reload |= rtk_set_int_mib(MIB_WLAN_WSC_ENC, 0);
			need_reload |= rtk_set_int_mib(MIB_WLAN_MODE, CLIENT_MODE);
		}
	}

	for (i = 0; i < 2; i++) {
		wlan_idx = i;
		ssid[3]  = '_';
		for (j = 0; j < 6; j++) {
			vwlan_idx = j;
			ssid[6]   = '0' + j;
			// Root
			if (0 == j) {
				// Global Domain
				mib_val = 14;
				need_reload |= rtk_set_int_mib(MIB_HW_REG_DOMAIN, mib_val);
				mib_val = 0;
				need_reload |= rtk_set_int_mib(MIB_WLAN_WLAN_DISABLED, mib_val);
				mib_val = 0;
				// Set Bandwidth to 20MHz so sniffer can capture packets
				//need_reload |= rtk_set_int_mib(MIB_WLAN_CHANNEL_BONDING, mib_val);
				if (0 == i) {
					ssid[4] = '5';
				} else {
					ssid[4] = '2';
				}
				mib_val = 0;
				need_reload |= rtk_set_int_mib(MIB_WLAN_FUNC_OFF, mib_val);
				mib_val = 1;
				#ifdef WLAN_HS2_CONFIG
				need_reload |= rtk_set_int_mib(MIB_WLAN_HS2_ENABLE, mib_val);
				#endif
			}

			if (j > 1 && j < 5) {
				// Disable vap1-3 by default
				mib_val = 1;
				need_reload |= rtk_set_int_mib(MIB_WLAN_WLAN_DISABLED, mib_val);
			}

			if (5 != j) {
				mib_val = 0;
				need_reload |= rtk_set_int_mib(MIB_WLAN_HIDDEN_SSID, mib_val);
				need_reload |= rtk_set_int_mib(MIB_WLAN_MAP_BSS_TYPE, mib_val);
			} else {
				ssid[3] = '\0';
			}
			mib_val = 1;
			need_reload |= rtk_set_int_mib(MIB_WLAN_DOT11V_ENABLE, mib_val);
			need_reload |= rtk_set_int_mib(MIB_WLAN_DOT11K_ENABLE, mib_val);

			need_reload |= rtk_set_string_mib(MIB_WLAN_SSID, ssid);
		}
	}

	mib_val = 0;

	need_reload |= rtk_set_int_mib(MIB_MAP_CONFIGURED_BAND, mib_val);

	wlan_idx  = old_wlan_idx;
	vwlan_idx = old_vwlan_idx;

	system("cp /etc/multiap.conf /var/");
	system("rm /var/log/message*");
	system("rm /var/log/easymesh_log.txt");
	system("rm /tmp/*.config");
	system("rm /tmp/map_*.txt");
	system("rm /tmp/map_vlan_setting");
	system("rm /tmp/map_vlan_setting_configured");
	system("rm /tmp/dpp_*");
	
	if(-1 != access("/tmp/map_vlan_reset", F_OK)) {
		system("sh /tmp/map_vlan_reset");
		system("rm /tmp/map_vlan_reset");
	}
	
	system("rm /tmp/map_vlan_need_reset");
	system("echo 1 > /tmp/map_lock");

	if (need_reload) {
		apmib_update(CURRENT_SETTING);
	}

	printf("Need reload: %d\n", need_reload);
	return 0;
}
