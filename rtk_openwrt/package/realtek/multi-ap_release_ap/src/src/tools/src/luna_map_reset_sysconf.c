/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

//#include "mib.h"
//#include "sysconfig.h"
//#include "subr_wlan.h"

#include "map_tlvs.h"

enum MAP_RESET_MODE {
	MAP_RESET_NULL,
	MAP_RESET_CONTROLLER,
	MAP_RESET_AGENT
};

#define BACKHAUL_STATION 0x80

#define ENVP_MAP_RESET				"reason=multi_ap_reset"		/* multi-ap reset. */

/* Script name */
#define SCRIPT_MAP                  "/var/multi_ap_script"
#define MULTI_AP_TMP_RESET_FILE	    "/tmp/multi_ap_reset_file"

#if 0
int rtk_compare_and_set(unsigned char *ptr, unsigned value)
{
	if (*ptr != value) {
		*ptr = value;
		// printf("%d and %d are different!\n", *ptr, value);
		return 1;
	}
	// printf("%d and %d are same!\n", *ptr, value);
	return 0;
}

int rtk_set_byte_mib(int mib_name, unsigned char mib_val)
{
	unsigned char old_value;
	mib_get(mib_name, (void *)&old_value);
	if (old_value != mib_val) {
		mib_set(mib_name, (void *)&mib_val);
		// printf("%d and %d are different!\n", old_value, mib_val);
		return 1;
	}
	// printf("%d and %d are the same!\n", old_value, mib_val);
	return 0;
}

int rtk_set_string_mib(int mib_name, char* mib_val)
{
	char old_value[100];
	mib_get(mib_name, (void *)&old_value);
	if (strcmp(old_value, mib_val)) {
		mib_set(mib_name, (void *)mib_val);
		// printf("%d and %d are different!\n", old_value, mib_val);
		return 1;
	}
	// printf("%d and %d are the same!\n", old_value, mib_val);
	return 0;
}
#endif

void _rtk_syscall_map_reset(char *config)
{
	char *argv[] = {SCRIPT_MAP, config, (char *)0};
	char *envp[] = {ENVP_MAP_RESET, (char *)0};
	char script[] = SCRIPT_MAP;
	int  pid = fork();

	if(pid < 0) {
		printf("[%s] fork() failed\n", __FUNCTION__);
	} else if(pid) {
		wait(0);
		printf("[%s] execve() done\n", __FUNCTION__);
	} else {
		//system("sleep 5");
		if(execve(script, argv, envp) < 0) {
			printf("[%s] execve() failed\n", __FUNCTION__);
			exit(0);
		}
	}
}

int main(int argc, char *argv[])
{
	int           c;
	int           reset_mode = MAP_RESET_NULL;
	unsigned char mib_val    = 0;
	int           radio_idx = 0, vap_idx = 0;

	int multiap_profile = 0;

	int need_reload = 0;

	//MIB_CE_MBSSIB_T entry;
	FILE *fp=NULL;
	char ifname[16]={0};

	char ssid[] = "RTK_00_0";

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

	fp = fopen(MULTI_AP_TMP_RESET_FILE, "w");
	if(fp == NULL) {
		printf("[ERROR] Can not open file %s.\n", MULTI_AP_TMP_RESET_FILE);
		return 1;
	}

	if (1 == reset_mode) {
		//mib_val = 129;
		if (MULTI_AP_PROFILE_4 == multiap_profile) {
			printf("Set as r4 controller\n");
			mib_val = 135;
		} if (MULTI_AP_PROFILE_3 == multiap_profile) {
			printf("Set as r3 controller\n");
			mib_val = 133;
		} else if (MULTI_AP_PROFILE_2 == multiap_profile) {
			printf("Set as r2 controller\n");
			mib_val = 131;
		} else {
			printf("Set as r1 controller\n");
			mib_val = 129;
		}
		fprintf(fp, "MAP_CONTROLLER=%d\n", mib_val);
		//need_reload |= rtk_set_byte_mib(MIB_MAP_CONTROLLER, mib_val);
#if 0
		mib_val = 0;
		need_reload |= rtk_set_byte_mib(MIB_REPEATER_ENABLED1, mib_val);
		need_reload |= rtk_set_byte_mib(MIB_REPEATER_ENABLED1 + 1, mib_val);
#endif
	} else if (2 == reset_mode) {
		//mib_val = 130;
		if (MULTI_AP_PROFILE_4 == multiap_profile) {
			printf("Set as r4 agent\n");
			mib_val = 136;
		} else if (MULTI_AP_PROFILE_3 == multiap_profile) {
			printf("Set as r3 agent\n");
			mib_val = 134;
		} else if (MULTI_AP_PROFILE_2 == multiap_profile) {
			printf("Set as r2 agent\n");
			mib_val = 132;
		} else {
			printf("Set as r1 agent\n");
			mib_val = 130;
		}
		fprintf(fp, "MAP_CONTROLLER=%d\n", mib_val);
		//need_reload |= rtk_set_byte_mib(MIB_MAP_CONTROLLER, mib_val);

#if 0
		for (radio_idx = 0; radio_idx < 2; radio_idx++) {
			// Root
			vap_idx = 0;
			need_reload |= rtk_set_byte_mib(MIB_WSC_DISABLE + radio_idx, 0);
			mib_chain_get(MIB_MBSSIB_TBL + radio_idx, vap_idx, &entry);
			need_reload |= rtk_compare_and_set(&entry.wsc_disabled, 0);
			mib_chain_update(MIB_MBSSIB_TBL + radio_idx, &entry, vap_idx);
			// VXD
			vap_idx = 5;
			mib_chain_get(MIB_MBSSIB_TBL + radio_idx, vap_idx, &entry);
			need_reload |= rtk_compare_and_set(&entry.encrypt, WIFI_SEC_WPA2);
			need_reload |= rtk_compare_and_set(&entry.wsc_disabled, 0);
			need_reload |= rtk_compare_and_set(&entry.wlanMode, CLIENT_MODE);
			need_reload |= rtk_compare_and_set(&entry.multiap_bss_type, BACKHAUL_STATION);
#ifdef WLAN_11W
			need_reload |= rtk_compare_and_set(&entry.dotIEEE80211W, 0);
#endif
			mib_chain_update(MIB_MBSSIB_TBL + radio_idx, &entry, vap_idx);
		}
#endif
	}

	mib_val = 0;
	fprintf(fp, "MAP_CONFIGURED_BAND=%d\n", mib_val);

	for (radio_idx = 0; radio_idx < 2; radio_idx++) {
		ssid[3] = '_';
		for (vap_idx = 0; vap_idx < 6; vap_idx++) {
			ssid[7] = '0' + vap_idx;
			if(0 == vap_idx)
				snprintf(ifname, sizeof(ifname), "wlan%d", radio_idx);
			else if(5 == vap_idx)
				snprintf(ifname, sizeof(ifname), "wlan%d-vxd", radio_idx);
			else
				snprintf(ifname, sizeof(ifname), "wlan%d-vap%d", radio_idx, vap_idx-1);
			fprintf(fp, "#####################\nINTERFACE=%s\n#####################\n", ifname);
			//mib_chain_get(MIB_MBSSIB_TBL + radio_idx, vap_idx, &entry);
			// Root
			if (0 == vap_idx) {
				if(1 == reset_mode){
					fprintf(fp, "REPEATER_ENABLED=0\n");
				}
				else if (2 == reset_mode){
					fprintf(fp, "WSC_DISABLED=0\n");
				}
				// Global Domain
				mib_val = 14;
				fprintf(fp, "HW_REG_DOMAIN=%d\n", mib_val);
				//need_reload |= rtk_set_byte_mib(MIB_HW_REG_DOMAIN + radio_idx, mib_val);
				mib_val = 0;
				fprintf(fp, "WLAN_DISABLED=%d\n", mib_val);
				//need_reload |= rtk_set_byte_mib(MIB_WLAN_DISABLED + radio_idx, mib_val);
				mib_val = 0;
				fprintf(fp, "WLAN_HIDDEN_SSID=%d\n", mib_val);
				//need_reload |= rtk_set_byte_mib(MIB_WLAN_HIDDEN_SSID + radio_idx, mib_val);
				// Set Bandwidth to 20MHz so sniffer can capture packets
				fprintf(fp, "WLAN_CHANNEL_WIDTH=%d\n", mib_val);
				//need_reload |= rtk_set_byte_mib(MIB_WLAN_CHANNEL_WIDTH + radio_idx, mib_val);
				mib_val = 0;
				fprintf(fp, "WLAN_AGGREGATION=%d\n", mib_val);
				//need_reload |= rtk_set_byte_mib(MIB_WLAN_AGGREGATION + radio_idx, mib_val);
				if (0 == radio_idx) {
					ssid[4] = '5';
					ssid[5] = 'H';
				} else if (1 == radio_idx) {
					ssid[4] = '5';
					ssid[5] = 'L';
				} else {
					ssid[4] = '2';
					ssid[5] = '4';
				}
				//need_reload |= rtk_set_string_mib(MIB_WLAN_SSID + radio_idx, ssid);
				//need_reload |= rtk_set_string_mib(MIB_REPEATER_SSID1 + radio_idx, "MAP_RPT");
			}
			else if(5 == vap_idx){
				if (2 == reset_mode){
					fprintf(fp, "WLAN_ENCRYPT=%d\n", 4); //WIFI_SEC_WPA2
					fprintf(fp, "WSC_DISABLED=0\n");
					fprintf(fp, "WLAN_MODE=%d\n", 1); //CLIENT_MODE
					fprintf(fp, "WLAN_MAP_BSS_TYPE=%d\n", BACKHAUL_STATION);
					fprintf(fp, "WLAN_DOTIEEE80211W=0\n");
				}
			}

			if (vap_idx > 1 && vap_idx < 5) {
				// Disable vap1-3 by default
				//need_reload |= rtk_compare_and_set(&entry.wlanDisabled, 1);
				fprintf(fp, "WLAN_DISABLED=%d\n", 1);
			}

			if (5 != vap_idx) {
				//need_reload |= rtk_compare_and_set(&entry.hidessid, 0);
				fprintf(fp, "WLAN_HIDDEN_SSID=%d\n", 1);
				//need_reload |= rtk_compare_and_set(&entry.multiap_bss_type, 0);
				fprintf(fp, "WLAN_MAP_BSS_TYPE=%d\n", 0);
			} else {
				ssid[3] = '\0';
			}
#if defined(WLAN_11K) && defined(WLAN_11V)
			//need_reload |= rtk_compare_and_set(&entry.rm_activated, 1);
			fprintf(fp, "RM_ACTIVATED=%d\n", 1);
			//need_reload |= rtk_compare_and_set(&entry.BssTransEnable, 1);
			fprintf(fp, "BSS_TRANS_ENABLE=%d\n", 1);
#endif
			// printf("set ssid for radio %d vap %d...\n", radio_idx, vap_idx);

#if 0
			if (strcmp((char *)entry.ssid, ssid)) {
				// printf("%s and %s are different!\n", entry.ssid, ssid);
				strcpy((char *)entry.ssid, ssid);
				need_reload = 1;
			} else {
				// printf("%s and %s are same!\n", entry.ssid, ssid);
			}
#endif
			fprintf(fp, "WLAN_SSID=%s\n", ssid);

			//mib_chain_update(MIB_MBSSIB_TBL + radio_idx, &entry, vap_idx);
		}
	}

	//mib_val = 0;

	//need_reload |= rtk_set_byte_mib(MIB_MAP_CONFIGURED_BAND, mib_val);

	//mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

	fclose(fp);
	_rtk_syscall_map_reset(MULTI_AP_TMP_RESET_FILE);

	system("cp /etc/multiap.conf /var/");
	system("rm /var/log/message*");
	system("rm /var/log/multi_ap_r2.txt");
	system("rm /tmp/*.config");
	system("rm /tmp/map_*.txt");
	system("rm /tmp/map_vlan_setting");
	system("rm /tmp/map_vlan_setting_configured");

	if(-1 != access("/tmp/map_vlan_reset", F_OK)) {
		system("sh /tmp/map_vlan_reset");
		system("rm /tmp/map_vlan_reset");
	}

	system("rm /tmp/map_vlan_need_reset");
	system("echo 1 > /tmp/map_lock");

	printf("Need reload: %d\n", need_reload);
	return 0;
}