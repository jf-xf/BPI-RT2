/*
 * Copyright (C) 2018 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <errno.h>          // errno
#include <sys/ioctl.h>      // ioctl()
#include <sys/socket.h>     // socket()
#include <linux/wireless.h> // struct iwreq
#include <stdio.h>          // printf(), popen()
#include <stdlib.h>         // malloc(), ssize_t
#include <string.h>         // strdup()
#include <unistd.h>         // close()
#include <dirent.h>
//#include <arpa/inet.h>     // will cause build conflict when also #include <linux/if_bridge.h>
#include <linux/if_bridge.h> // struct __fdb_entry

#include "platform_interfaces.h"
#include "platform_interfaces_rtk_priv.h"
#include "platform_ioctl.h"
#include "platform_os_priv.h"
#include "multi_ap_tlvs.h"
#include "al_wsc.h"

#include "global_settings.h"

#if defined(CONFIG_RTK_PON_XDSL) || !defined(__UCLIBC__)
#include "utils.h"
#endif

#include "../../../../multi-ap/customized/map_vendor_specific_data.h"

#define UNUSED(x) (void)(x)

// #include "utility.h"
////////////////////////////////////////////////////////////////////////////////
// Private data and functions
////////////////////////////////////////////////////////////////////////////////

// 0 - 130
static const INT8U us_to_global_lookup[] = {
	0,
	115, 118, 124, 121, 125, 103, 103, 102, 102, 101,
	101, 81, 94, 95, 96, 0, 0, 0, 0, 0,
	0, 116, 119, 122, 126, 126, 117, 120, 123, 127,
	127, 83, 84, 180, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 128, 129, 130
};

// 0 - 130
static const INT8U eu_to_global_lookup[] = {
	0,
	115, 118, 121, 81, 116, 119, 122, 117, 120, 123,
	83, 84, 0, 0, 0, 0, 125, 180, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 128, 129, 130
};

// 0 - 130
static const INT8U jp_to_global_lookup[] = {
	0,
	115, 112, 112, 112, 112, 112, 109, 109, 109, 109,
	109, 113, 113, 113, 113, 110, 110, 110, 110, 110,
	114, 114, 114, 114, 111, 111, 111, 111, 111, 81,
	82, 118, 118, 121, 121, 116, 119, 119, 122, 122,
	117, 120, 120, 123, 123, 104, 104, 104, 104, 104,
	104, 105, 105, 105, 105, 105, 83, 84, 121, 180,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 128, 129, 130
};

// 0 - 130
static const INT8U cn_to_global_lookup[] = {
	0,
	115, 118, 125, 116, 119, 126, 81, 83, 84, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 128, 129, 130
};

// 81 - 130
static const INT8U global_to_us_lookup[] = {
	12, 0, 32, 33, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 13, 14, 15, 0, 0, 0, 0,
	11, 8, 6, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 22, 27, 2, 23, 28,
	4, 24, 29, 3, 5, 25, 30, 128, 129, 130
};

// 81 - 130
static const INT8U global_to_eu_lookup[] = {
	4, 0, 11, 12, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 5, 8, 2, 6, 9,
	3, 7, 10, 0, 17, 0, 0, 128, 129, 130
};

// 81 - 130
static const INT8U global_to_jp_lookup[] = {
	30, 31, 56, 57, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 46, 51, 0, 0, 0, 7, 16,
	25, 2, 12, 21, 1, 36, 41, 32, 37, 42,
	34, 39, 44, 0, 0, 0, 0, 128, 129, 130
};

// 81 - 130
static const INT8U global_to_cn_lookup[] = {
	7, 0, 8, 9, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 4, 0, 2, 5, 0,
	0, 0, 0, 0, 3, 6, 0, 128, 129, 130
};

#define SSID_LEN 32

#define WIFI_STATION_STATE 0x00000008
#define WIFI_AP_STATE      0x00000010


enum NETWORK_TYPE {
	WIRELESS_11B  = 1,
	WIRELESS_11G  = 2,
	WIRELESS_11A  = 4,
	WIRELESS_11N  = 8,
	WIRELESS_11AC = 64,
	WIRELESS_11AX = 128
};

#if !defined(_DRIVER_COMMON_STRUCTURE_) && !defined(CONFIG_RTK_PON_XDSL)
#define STA_INFO_FLAG_ASOC			0x04 //same to mib.h

typedef struct _bss_info {
	unsigned char state;
	unsigned char channel;
	unsigned char txRate;
	unsigned char bssid[6];
	unsigned char rssi, sq; // RSSI  and signal strength
	unsigned char ssid[SSID_LEN + 1];
} bss_info;

/* WLAN sta info structure */
typedef struct sta_info_2_web {
    unsigned short aid;
    unsigned char  addr[6];
    unsigned long  tx_packets;
    unsigned long  rx_packets;
    unsigned long  expired_time; // 10 mini-sec
    unsigned short flag;
    unsigned char  txOperaRates;
    unsigned char  rssi;
    unsigned long  link_time; // 1 sec unit
    unsigned long  tx_fail;
    unsigned long  tx_bytes;
    unsigned long  rx_bytes;
    unsigned char  network;
    unsigned char  ht_info;	// bit0: 0=20M mode, 1=40M mode; bit1: 0=longGI, 1=shortGI; bit2: 80M mode; bit3: 160M mode
    unsigned char  RxOperaRate;
    unsigned char  resv[3];
    unsigned short acTxOperaRate;
#ifdef RTK_MULTI_AP
    unsigned char  multiap_profile;
#endif
} WLAN_STA_INFO_T, *WLAN_STA_INFO_Tp;
#else
typedef sta_info_2_web WLAN_STA_INFO_T, *WLAN_STA_INFO_Tp;
#endif

int _find_wscd_status_file(char *interface_name, char *wscd_file)
{
	DIR *          dir;
	struct dirent *ent;
	if ((dir = opendir("/tmp")) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			if (PLATFORM_STRSTR(ent->d_name, "wscd") && PLATFORM_STRSTR(ent->d_name, interface_name)) {
				sprintf(wscd_file, "%s/%s", "/tmp", ent->d_name);
				closedir(dir);
				return 0;
			}
		}
		closedir(dir);
	}
	return 1;
}

int _getWlBssInfo(char *interface, bss_info *pInfo)
{
	int          skfd = 0;
	struct iwreq wrq;

	skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (skfd == -1)
		return 0;

	strncpy(wrq.ifr_name, interface, IFNAMSIZ - 1);

	/* Get wireless name */
	if (ioctl(skfd, SIOCGIWNAME, &wrq) < 0) {
		/* If no wireless name : no wireless extensions */
		close(skfd);
		return 0;
	}

	wrq.u.data.pointer = (caddr_t)pInfo;
	wrq.u.data.length  = sizeof(bss_info);

	if (ioctl(skfd, SIOCGIWRTLGETBSSINFO, &wrq) < 0) {
		close(skfd);
		return 0;
	}
	close(skfd);

	return 0;
}

int _getWlStaInfo(char *interface, WLAN_STA_INFO_Tp pInfo)
{
	int          skfd = 0;
	struct iwreq wrq;

	skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (skfd == -1)
		return 0;

	strncpy(wrq.ifr_name, interface, IFNAMSIZ - 1);

	/* Get wireless name */
	if (ioctl(skfd, SIOCGIWNAME, &wrq) < 0) {
		/* If no wireless name : no wireless extensions */
		close(skfd);
		return 0;
	}

	wrq.u.data.pointer = (caddr_t)pInfo;
	wrq.u.data.length  = sizeof(WLAN_STA_INFO_T) * (map_max_sta_num + 1);
	memset(pInfo, 0, sizeof(WLAN_STA_INFO_T) * (map_max_sta_num + 1));

	if (ioctl(skfd, SIOCGIWRTLSTAINFO, &wrq) < 0) {
		close(skfd);
		return 0;
	}
	close(skfd);

	return 0;
}

static void _get_wifi_connected_devices(char *interface_name, struct interfaceInfo *m)
{
	// This is a copy of wirelessClientList() in boa/src/fmwlan.c:6752 to get all associated stations info

	char *           buff;
	WLAN_STA_INFO_Tp pInfo;
	INT8U            i;

	m->neighbor_mac_addresses_nr = 0;
	buff                         = calloc(1, sizeof(WLAN_STA_INFO_T) * (map_max_sta_num + 1));
	if (buff == 0) {
		PLATFORM_PRINTF_ERROR("[RTL] Allocate buffer failed!\n");
		return;
	}

	if (_getWlStaInfo(interface_name, (WLAN_STA_INFO_Tp)buff) < 0) {
		PLATFORM_PRINTF_ERROR("[RTL] Read wlan sta info failed!\n");
	}
	for (i = 1; i <= map_max_sta_num; i++) {
		pInfo = (WLAN_STA_INFO_Tp)&buff[i * sizeof(WLAN_STA_INFO_T)];
#if defined(_DRIVER_COMMON_STRUCTURE_) || defined(CONFIG_RTK_PON_XDSL)
		if (pInfo->aid && (pInfo->flags & STA_INFO_FLAG_ASOC)) {
#else
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC)) {
#endif
			// calcuate the number of associated devices
			m->neighbor_mac_addresses_nr += 1;
		}
	}
	if (m->neighbor_mac_addresses_nr <= 0) {
		PLATFORM_PRINTF_FULL("[RTL] [%s] NO station is associated on this interface.\n",
		                     interface_name);
		PLATFORM_FREE(buff);
		return;
	}

	m->neighbor_list = (struct neighborInfo *)realloc(m->neighbor_list, sizeof(struct neighborInfo) * m->neighbor_mac_addresses_nr + 1);

	int counter = 0;
	for (i = 1; i <= map_max_sta_num; i++) {
		pInfo = (WLAN_STA_INFO_Tp)&buff[i * sizeof(WLAN_STA_INFO_T)];
#if defined(_DRIVER_COMMON_STRUCTURE_) || defined(CONFIG_RTK_PON_XDSL)
		if (pInfo->aid && (pInfo->flags & STA_INFO_FLAG_ASOC)) {
#else
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC)) {
#endif
			m->neighbor_list[counter].link_time = pInfo->link_time;
			memcpy(m->neighbor_list[counter].mac_address, pInfo->addr, 6);
			m->neighbor_list[counter].multiap_profile = pInfo->multiap_profile;
			PLATFORM_PRINTF_FULL("[RTL] [%s] Associated STA MAC \t%02x:%02x:%02x:%02x:%02x:%02x\tLink Time (s):\t %lu\tProfile:\t %d\n",
			                     interface_name,
			                     m->neighbor_list[counter].mac_address[0], m->neighbor_list[counter].mac_address[1], m->neighbor_list[counter].mac_address[2],
			                     m->neighbor_list[counter].mac_address[3], m->neighbor_list[counter].mac_address[4], m->neighbor_list[counter].mac_address[5],
			                     m->neighbor_list[counter].link_time, m->neighbor_list[counter].multiap_profile);
			counter++;
		}
	}
	PLATFORM_FREE(buff);
}

extern char *if_indextoname (unsigned int __ifindex, char *__ifname);
int _collect_intf_fdb(char *intf_name, struct interfaceInfo *m)
{
	int br_socket_fd = -1, ret = 0, count = 0;

	if ((br_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return 0;
	}
	else
	{
		struct __fdb_entry fdb[64];
		unsigned char fdb_entry_list[64][6] = {{0}};
		unsigned long args[4];
		struct ifreq ifr;
		int ifindices[256];
		int list_num = 0, i, num, offset = 0, fdb_num;
		unsigned long list_args[4] = { BRCTL_GET_PORT_LIST, (unsigned long)ifindices, 256, 0 };

		memset(&ifr, 0, sizeof(ifr));
		memset(ifindices, 0, sizeof(ifindices));
		strncpy(ifr.ifr_name, map_bridge_name, IFNAMSIZ);
		ifr.ifr_data = (char *)&list_args;

		list_num = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);
		if (list_num < 0) {
			close(br_socket_fd);
			return 0;
		}

		memset(&ifr, 0, sizeof(ifr));
		fdb_num = sizeof(fdb)/sizeof(struct __fdb_entry);
		args[0] = BRCTL_GET_FDB_ENTRIES;
		args[1] = (unsigned long)fdb;
		args[2] = fdb_num;
		memcpy(ifr.ifr_name, map_bridge_name, IFNAMSIZ);
		ifr.ifr_data = (char *)&args;

		ret = offset = 0;
		do {
			args[3] = offset;
			num = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);
			for (i = 0; i < num; i++) {
				char port_intf_name[20] = {0};
				struct __fdb_entry *f = fdb + i;

				if_indextoname(ifindices[f->port_no], port_intf_name);
				if (strcmp(intf_name, port_intf_name) == 0 && count < 64) {
					memcpy(fdb_entry_list[count], f->mac_addr, 6);
					count++;
				}
			}

			if (num > 0 && num >= fdb_num)
				offset += num;
		} while ((num > 0 && num >= fdb_num) && ret == 0);

		close(br_socket_fd);

		if (count) {
			m->neighbor_mac_addresses_nr = count;
			m->neighbor_list = realloc(m->neighbor_list, sizeof(struct neighborInfo) * count);
			for (i = 0; i < count; i++) {
				memcpy(m->neighbor_list[i].mac_address, fdb_entry_list[i], 6);
				m->neighbor_list[i].multiap_profile = 0;
				PLATFORM_PRINTF_DEBUG("[%s] connected MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
					intf_name,
					m->neighbor_list[i].mac_address[0],
					m->neighbor_list[i].mac_address[1],
					m->neighbor_list[i].mac_address[2],
					m->neighbor_list[i].mac_address[3],
					m->neighbor_list[i].mac_address[4],
					m->neighbor_list[i].mac_address[5]);
			}
		}
	}

	return count;
}

static void _get_eth_connected_devices(char *intf_name, struct interfaceInfo *m)
{
	struct eth_port_map *eth_recv_1905 = PLATFORM_GET_ETH_PORT_1905();
	int i;

	for (i = 0; i < 4; i++) {
		if (0 == PLATFORM_STRCMP(eth_recv_1905[i].ifname, intf_name)) {
			if (PLATFORM_GET_TIMESTAMP() - eth_recv_1905[i].update_timestamp > 90 * 1000 ||
				eth_recv_1905[i].update_timestamp == 0) {
				// no 1905 packet recv for 90 sec and longer, we assume no agent connected via this ethernet port
				_collect_intf_fdb(intf_name, m);
			}
			break;
		}
	}
}

static INT32S _read_sta_info_proc(char *interface_name, INT8U *neighbor_interface_address, char *parameter_name)
{
	INT32S ret;
	FILE  *pipe;
	char  *line;
	char  *line_break = NULL;
	size_t len;

	char command[200];

	ret = 0;

	if ((NULL == interface_name) || (NULL == neighbor_interface_address) || (NULL == parameter_name)) {
		return 0;
	}

	int  count         = 0;
	int  start_line    = 0;
	char file_path[64] = { 0 };

	snprintf(file_path, sizeof(file_path), "/proc/%s/sta_info", interface_name);

	if (-1 == access(file_path, F_OK)) {
		PLATFORM_PRINTF_ERROR("File %s does not exist!\n", file_path);
		return 0;
	}

	sprintf(command, "cat /proc/%s/sta_info | grep -n %02x%02x%02x%02x%02x%02x",
	        interface_name,
	        neighbor_interface_address[0], neighbor_interface_address[1], neighbor_interface_address[2],
	        neighbor_interface_address[3], neighbor_interface_address[4], neighbor_interface_address[5]);

	pipe = popen(command, "re");

	if (!pipe) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] popen() returned with errno=%d (%s)\n", errno, strerror(errno));
		return 0;
	}

	line = NULL;
	if (-1 != getline(&line, &len, pipe)) {

		// Remove the last "\n"
		//
		line[strlen(line) - 1] = 0x00;

		line_break = PLATFORM_STRSTR(line, ":");
		if (line_break) {
			line_break[0] = '\0';
			sscanf(line, "%d", &ret);
			PLATFORM_PRINTF_DETAIL("[PLATFORM] Neighbor %02x:%02x:%02x:%02x:%02x:%02x is under (%s) around %d\n",
			                       neighbor_interface_address[0], neighbor_interface_address[1], neighbor_interface_address[2],
			                       neighbor_interface_address[3], neighbor_interface_address[4], neighbor_interface_address[5],
			                       interface_name, ret);
		} else {
			PLATFORM_PRINTF_WARNING("[PLATFORM] Parameter not found\n");
			free(line);
			pclose(pipe);
			return 0;
		}
	} else {
		PLATFORM_PRINTF_DETAIL("[PLATFORM] Station is not found under %s\n", interface_name);
		free(line);
		pclose(pipe);
		return 0;
	}

	free(line);
	pclose(pipe);

	pipe       = fopen(file_path, "re");
	start_line = ret;

	if (!pipe) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] popen() returned with errno=%d (%s)\n", errno, strerror(errno));
		return 0;
	}

	line = PLATFORM_MALLOC(sizeof(char) * 256);

	while (fgets(line, 256, pipe) != NULL) {
		if (count >= start_line) {
			line[strlen(line) - 1] = 0x00;
			char *tmp              = PLATFORM_STRSTR(line, parameter_name);
			if (tmp) {
				tmp += strlen(parameter_name);
				if (0 == PLATFORM_STRCMP("current_tx_rate:", parameter_name)) {
					char cap_string[10];
					char nss_string[20];
					if (-1 == sscanf(tmp, "%s %s %d", cap_string, nss_string, &ret)) {
						PLATFORM_PRINTF_WARNING("Parsing current_tx_rate failed, line is <%s>\n", tmp);
						ret = 0;
					}
					// PLATFORM_PRINTF_DEBUG("current_tx_rate result %s %s %d\n", cap_string, nss_string, ret);
				} else {
					sscanf(tmp, "%d", &ret);
				}
				PLATFORM_PRINTF_DEBUG("Neighbor %02x:%02x:%02x:%02x:%02x:%02x %s %d.\n", neighbor_interface_address[0], neighbor_interface_address[1], neighbor_interface_address[2],
				                      neighbor_interface_address[3], neighbor_interface_address[4], neighbor_interface_address[5],
				                      parameter_name, ret);
				break;
			}
		}
		count++;
	}

	fclose(pipe);
	free(line);

	return ret;
}

////////////////////////////////////////////////////////////////////////////////
// Internal API: to be used by other platform-specific files (functions
// declaration is found in "./platform_interfaces_rtk_linux_priv.h")
////////////////////////////////////////////////////////////////////////////////

INT8U rtk_linux_get_interface_info(char *interface_name, struct interfaceInfo *m, INT8U need_wps)
{
	FILE *fd;
	int   btn_val;
	char  ssidbuf[64];

	char wscd_status_file[64];

	INT8U  mib_val_int8u    = 0;
	INT32U mib_val_int32u   = 0;
	int    interface_status = 0;

	memset(ssidbuf, 0, 64);

	ssidbuf[63] = '\0';

	interface_status = PLATFORM_IS_INTERFACE_UP(interface_name);
	if (interface_status) {
		m->power_state = INTERFACE_POWER_STATE_ON;
	}

	if (strstr(interface_name, "wlan") != NULL) {

		INT8U bss_type = rtk_linux_get_bss_type(interface_name);
		PLATFORM_GET_MIB(interface_name, "func_off", &mib_val_int32u, 4);

		if (MULTI_AP_TEARDOWN_BIT == bss_type && 1 != mib_val_int32u && INTERFACE_POWER_STATE_ON == m->power_state) {
			char buf[] = "1";
			PLATFORM_PRINTF_WARNING("[RTL] Interface %s is still on, tearing down...\n", interface_name);
			rtk_linux_set_mib(interface_name, "func_off", buf);
			mib_val_int32u = 1;
		}

		if (1 == mib_val_int32u) {
			PLATFORM_PRINTF_DETAIL("[RTL] Interface %s has func_off, set as power off\n", interface_name);
			m->power_state = INTERFACE_POWER_STATE_OFF;
		}

		int wlan_idx = interface_name[4] - '0';
		if (wlan_idx > 2 || wlan_idx < 0) {
			PLATFORM_PRINTF_ERROR("[RTL] UNSUPPORTED INTERFACE %s \n", interface_name);
			return 0;
		}

		// Retrieve Network band information
		//
		PLATFORM_GET_MIB(interface_name, "band", &mib_val_int8u, 1);

		if (mib_val_int8u & WIRELESS_11AX) {
			m->interface_type = INTERFACE_TYPE_IEEE_802_11AX;
		} else if (mib_val_int8u & WIRELESS_11AC) {
			m->interface_type = INTERFACE_TYPE_IEEE_802_11AC_5_GHZ;
		} else if (mib_val_int8u & WIRELESS_11A) {
			m->interface_type = INTERFACE_TYPE_IEEE_802_11A_5_GHZ;
		} else if (mib_val_int8u & WIRELESS_11N) {
			if (map_radio_name_24g[4] - '0' == wlan_idx) {
				m->interface_type = INTERFACE_TYPE_IEEE_802_11N_2_4_GHZ;
			} else {
				m->interface_type = INTERFACE_TYPE_IEEE_802_11N_5_GHZ;
			}
		} else if (mib_val_int8u & WIRELESS_11G) {
			m->interface_type = INTERFACE_TYPE_IEEE_802_11G_2_4_GHZ;
		} else if (mib_val_int8u & WIRELESS_11B) {
			m->interface_type = INTERFACE_TYPE_IEEE_802_11B_2_4_GHZ;
		}

		if (!interface_status || INTERFACE_TYPE_UNKNOWN == m->interface_type) {
			if (map_radio_name_24g[4] - '0' == wlan_idx)
				m->interface_type = INTERFACE_TYPE_IEEE_802_11N_2_4_GHZ;
			else if (map_radio_name_5gh[4] - '0' == wlan_idx)
				m->interface_type = INTERFACE_TYPE_IEEE_802_11AC_5_GHZ;
			else if (map_radio_name_5gl[4] - '0' == wlan_idx)
				m->interface_type = INTERFACE_TYPE_IEEE_802_11N_5_GHZ;
		}

		if (map_radio_name_24g[4] - '0' == wlan_idx)
			m->interface_type_data.ieee80211.rf_band = IEEE80211_RF_BAND_2_4_GHZ;
		else
			m->interface_type_data.ieee80211.rf_band = IEEE80211_RF_BAND_5_GHZ;

		PLATFORM_PRINTF_FULL("[RTL] Interface %s type: %04x\n", interface_name, m->interface_type);

		PLATFORM_GET_MIB(interface_name, "opmode", &mib_val_int32u, 4);

		if (WIFI_AP_STATE & mib_val_int32u) {
			m->interface_type_data.ieee80211.role = IEEE80211_ROLE_AP;
		} else if (WIFI_STATION_STATE & mib_val_int32u) {
			m->interface_type_data.ieee80211.role = IEEE80211_ROLE_NON_AP_NON_PCP_STA;
		} else {
			PLATFORM_PRINTF_ERROR("[RTL] Unsupport mode %08x for interface %s\n", mib_val_int32u, interface_name);
		}

		// PLATFORM_PRINTF_INFO("[RTL] Interface %s mode: %02x\n", interface_name, m->interface_type_data.ieee80211.role);

		PLATFORM_GET_MIB(interface_name, "ssid", &ssidbuf, 0);

		strcpy(m->interface_type_data.ieee80211.ssid, ssidbuf);

		m->is_secured                                        = 0;
		m->interface_type_data.ieee80211.authentication_mode = IEEE80211_AUTH_MODE_OPEN;

		PLATFORM_GET_MIB(interface_name, "authtype", &mib_val_int32u, 4);

		PLATFORM_GET_MIB(interface_name, "encmode", &mib_val_int8u, 1);

		// PLATFORM_PRINTF_INFO("%s auth type %08x encmode %02x\n", interface_name, mib_val_int32u, mib_val_int8u);
		if (2 == mib_val_int8u || (4 == mib_val_int8u && PLATFORM_STRSTR(interface_name, map_vxd_prefix))) {
			m->is_secured                                        = 1;
			m->interface_type_data.ieee80211.authentication_mode = IEEE80211_AUTH_MODE_WPA2PSK | IEEE80211_AUTH_MODE_WPAPSK;
			m->interface_type_data.ieee80211.encryption_mode     = IEEE80211_ENCRYPTION_MODE_TKIP | IEEE80211_ENCRYPTION_MODE_AES;
		} else {
			PLATFORM_PRINTF_DETAIL("%s is insecure: auth type %08x encmode %02x\n", interface_name, mib_val_int32u, mib_val_int8u);
		}

		// Retrieve Push Button information
		//
		m->push_button_on_going = 2;
		m->wsc_result           = 0;

		if (NULL == strstr(m->name, map_vap_prefix) && need_wps) {
			sprintf(wscd_status_file, "%s-%s", "/tmp/wscd_status", m->name);
			if (access(wscd_status_file, F_OK) == -1) {
				if (1 == _find_wscd_status_file(m->name, wscd_status_file)) {
					sprintf(wscd_status_file, "%s", "/tmp/wscd_status");
				}
			}

			if (access(wscd_status_file, F_OK) != -1) {
				fd = fopen(wscd_status_file, "re");

				if (NULL == fd) {
					PLATFORM_PRINTF_WARNING("[PLATFORM] Cannot open /tmp/wscd_status\n");
					m->push_button_on_going = 2;
					m->wsc_result           = 0;
				} else {

					fscanf(fd, "%d", &btn_val);
					if (btn_val == -1 || btn_val == 2 || btn_val == 3 || btn_val == 1)
						m->push_button_on_going = 0;
					else
						m->push_button_on_going = 1;

					m->wsc_result = 0;

					if (btn_val == 3)
						m->wsc_result = 1;

					fclose(fd);
				}
			}
		}

		// if (m->wsc_result == 1) {
		// 	if (access("/tmp/wscd_enrollee", F_OK) != -1) {
		// 		fd = fopen("/tmp/wscd_enrollee", "r");
		// 		INT8U enrollee[6];

		// 		fscanf(fd, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx%*c", &enrollee[0], &enrollee[1], &enrollee[2],
		// 		       &enrollee[3], &enrollee[4], &enrollee[5]);
		// 		PLATFORM_MEMCPY(m->push_button_new_mac_address, enrollee, 6);
		// 		fclose(fd);
		// 	}
		// }

		bss_info pInfo;
		memset(&pInfo, 0x00, sizeof(bss_info));
		if (!eth_only)
			_getWlBssInfo(interface_name, &pInfo);

		PLATFORM_MEMCPY(m->interface_type_data.ieee80211.bssid, pInfo.bssid, 6);
		PLATFORM_PRINTF_FULL("[RTL] [%s] state: %d BSSID:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x \n",
		                     interface_name,
		                     pInfo.state,
		                     m->interface_type_data.ieee80211.bssid[0],
		                     m->interface_type_data.ieee80211.bssid[1],
		                     m->interface_type_data.ieee80211.bssid[2],
		                     m->interface_type_data.ieee80211.bssid[3],
		                     m->interface_type_data.ieee80211.bssid[4],
		                     m->interface_type_data.ieee80211.bssid[5]);
		//Retrieve list of connected devices
		//
		if(INTERFACE_POWER_STATE_OFF == m->power_state) {
			m->neighbor_mac_addresses_nr = 0;
		} else {
			_get_wifi_connected_devices(interface_name, m);
		}
	} else {
		m->interface_type       = INTERFACE_TYPE_IEEE_802_3AB_GIGABIT_ETHERNET;
		m->is_secured           = 1;
		m->push_button_on_going = 2;

		if (strstr(interface_name, "eth0.") != NULL)
			_get_eth_connected_devices(interface_name, m);
	}

	return 1;
}

INT8U rtk_linux_update_ethernet_clients(INT8U *neighbor_mac_addresses_nr, struct ethernetNeighborInfo **neighbor_list, char *connected_interface)
{
	if(customize_features & SUPPORT_ETHERNET_ADAPTER)
	{
		RTK_L2_TAB_INFO *pL2List = (RTK_L2_TAB_INFO *)PLATFORM_MALLOC(sizeof(RTK_L2_TAB_INFO) * 128);
		if (!pL2List) {
			PLATFORM_PRINTF_ERROR("Fail to allocate buffer for pL2List, update ethernet clients failed \n");
			return 0;
		}
		PLATFORM_MEMSET(pL2List, 0, sizeof(RTK_L2_TAB_INFO) * 128);
		int real_num = 0, flag = 1;
		int i = 0;

		rtk_ethernet_get_l2_list(pL2List, 128, &real_num, flag);

		for (i = 0; i < real_num; i++) {
			if (-1 == pL2List[i].port_num  || !strcmp(connected_interface, pL2List[i].ifName)) {
				//fprintf(stderr, "\n[%s %d]This station:%02x:%02x:%02x:%02x:%02x:%02x is from other 1905 device, skip.\n", __FUNCTION__, __LINE__,
				//pL2List[i].mac[0], pL2List[i].mac[1], pL2List[i].mac[2], pL2List[i].mac[3], pL2List[i].mac[4], pL2List[i].mac[5]);
				continue;
			}

			*neighbor_mac_addresses_nr += 1;
			*neighbor_list = (struct ethernetNeighborInfo *)PLATFORM_REALLOC(*neighbor_list, sizeof(struct ethernetNeighborInfo) * (*neighbor_mac_addresses_nr));
			PLATFORM_MEMSET(&(*neighbor_list)[*neighbor_mac_addresses_nr - 1], 0, sizeof(struct ethernetNeighborInfo));
			PLATFORM_MEMCPY((*neighbor_list)[*neighbor_mac_addresses_nr - 1].mac_address, pL2List[i].mac, 6);
			PLATFORM_MEMCPY((*neighbor_list)[*neighbor_mac_addresses_nr - 1].ifName, pL2List[i].ifName, strlen(pL2List[i].ifName));
			(*neighbor_list)[*neighbor_mac_addresses_nr - 1].port = pL2List[i].port_num;
		}
		PLATFORM_FREE(pL2List);
	}
	else
	{
		*neighbor_list = NULL;
		*neighbor_mac_addresses_nr = 0;
	}
	return 0;
}

#define SIOC11VBSSTRANSREQ 0x8BF5
INT8U rtk_linux_send_btm_request(char *ifname, INT8U *macaddr, INT8U mode, INT8U *target_bssid, INT8U target_opclass, INT8U target_channel, INT16U disassoc_timer, INT8U reason_code)
{
	struct target_transition_list bssTrans_req;
	int                           sock;
	struct iwreq                  wrq;
	int                           err;
	unsigned char                 tempbuf[16384];

	/*** Initialize socket ***/
	sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (sock < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("%s(%s): Can't create socket for ioctl.(%d)", __FUNCTION__, ifname, err);
		return 0;
	}
	memset(&bssTrans_req, 0, sizeof(struct target_transition_list));

	memcpy(bssTrans_req.addr, macaddr, MACADDRLEN);
	bssTrans_req.mode = mode;
	if ((mode >> 7) & 0x01) {
		bssTrans_req.disassoc_timer = disassoc_timer;
	}
	if (target_bssid == NULL) {
		PLATFORM_PRINTF_ERROR("PLATFORM_BTM_REQ NULL BSSID!!\n");
		close(sock);
		return 0;
	}
	memcpy(bssTrans_req.target_bssid, target_bssid, MACADDRLEN);
	PLATFORM_PRINTF_DETAIL("[MULTI_AP] PLATFORM_BTM_REQ target bssid: %02x:%02x:%02x:%02x:%02x:%02x\n",
	                       target_bssid[0],
	                       target_bssid[1],
	                       target_bssid[2],
	                       target_bssid[3],
	                       target_bssid[4],
	                       target_bssid[5]);
	bssTrans_req.opclass = target_opclass;
	bssTrans_req.channel = target_channel;
	bssTrans_req.reason_code = reason_code;
	memcpy(tempbuf, &bssTrans_req, sizeof(struct target_transition_list));

	/*** Initialize struct iwreq ***/
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, ifname, IFNAMSIZ - 1);

	/*** give parameter and buffer ***/
	wrq.u.data.pointer = (caddr_t)tempbuf;
	wrq.u.data.length  = sizeof(struct target_transition_list);

	/*** ioctl ***/
	if (ioctl(sock, SIOC11VBSSTRANSREQ, &wrq) < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("%s(%s): ioctl Error.(%d)", __FUNCTION__, ifname, err);
		close(sock);
		return 0;
	}

	close(sock);
	return 1;
}

INT8U rtk_linux_bss_ioctl(char *interface_name, INT8U type)
{
	int          s;
	struct iwreq iwr;
	char         tmp[1];

	if (NULL == strstr(interface_name, "wlan")) {
		PLATFORM_PRINTF_WARNING("[RTK] bss_ioctl does not accept non-wlan device.\n");
		return 0;
	}

	tmp[0] = type;

	PLATFORM_PRINTF_DEBUG("[PLATFORM] Preparing to set BSS type:\n");
	PLATFORM_PRINTF_DEBUG("[PLATFORM]   - Interface name = %s\n", interface_name);
	PLATFORM_PRINTF_DEBUG("[PLATFORM]   - BSS Type = %02x\n", type);

	s = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (-1 == s) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] socket('%s') returned with errno=%d (%s) while opening a DGRAM socket\n", interface_name, errno, strerror(errno));
		return 0;
	}

	PLATFORM_PRINTF_DEBUG("[PLATFORM] Set interface type\n");
	strncpy(iwr.ifr_name, interface_name, IFNAMSIZ - 1);
	iwr.u.data.pointer = tmp;
	iwr.u.data.length  = 1;

	if (ioctl(s, RTL8192CD_IOCTL_UPDATE_BSS, &iwr) < 0) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] ioctl('%s',RTL8192CD_IOCTL_UPDATE_BSS) returned with errno=%d (%s) while setting BSS\n", interface_name, errno, strerror(errno));
		close(s);
		return 0;
	}

	PLATFORM_PRINTF_DEBUG("[PLATFORM] BSS set!\n");

	close(s);
	return 1;
}

#define RTL8192CD_IOCTL_SET_MIB 0x89f1
INT8U rtk_linux_set_mib(char *interface_name, char *item_str, char *value_str)
{
	// #ifdef _RTK_LINUX_
	// 	return rtk_linuk_set_mib(interface_name, item_str,value_str);
	// #else
	// 	return 0;
	// #endif
	int          s;
	struct iwreq wrq;
	static char  mib_str_buf[512];
	PLATFORM_SNPRINTF(mib_str_buf, 512, "%s=%s", item_str, value_str);
	PLATFORM_PRINTF_DEBUG("Set mib for %s: %s\n", interface_name, mib_str_buf);

	/*** Initialize socket ***/
	s = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (-1 == s) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] socket('%s') returned with errno=%d (%s) while opening a DGRAM socket\n", interface_name, errno, strerror(errno));
		return 0;
	}

	/*** Initialize struct iwreq ***/
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, interface_name, IFNAMSIZ - 1);

	/*** fill mib string ***/
	wrq.u.data.pointer = (caddr_t)mib_str_buf;
	wrq.u.data.length  = strlen(mib_str_buf) + 1;

	/*** set mib ***/
	if (ioctl(s, RTL8192CD_IOCTL_SET_MIB, &wrq) < 0) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] set_mib: %s(%s). Error to ioctl.(%d)\n", mib_str_buf, interface_name, errno);
		close(s);
		return 0;
	}

	close(s);
	return 1;
}

#define RTL8192CD_IOCTL_GET_MIB 0x89f2
INT8U rtk_linux_get_mib(char *interfacename, char *mibname, void *result, int size)
{
	int          skfd;
	struct iwreq wrq;
	char         tmp[64];

	skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (skfd < 0) {
		PLATFORM_PRINTF_ERROR("ioctl [RTL8192CD_IOCTL_GET_MIB] socket error\n");
		return 0;
	}

	memset(&wrq, 0, sizeof(wrq));
	strlcpy(wrq.ifr_name, interfacename, sizeof(wrq.ifr_name));
	strlcpy(tmp, mibname, sizeof(tmp));

	wrq.u.data.pointer = tmp;
	wrq.u.data.length  = strlen(tmp);

	/* Do the request */
	if (ioctl(skfd, RTL8192CD_IOCTL_GET_MIB, &wrq) < 0) {
		PLATFORM_PRINTF_ERROR("ioctl [RTL8192CD_IOCTL_GET_MIB] for mib %s on %s failed!\n", mibname, interfacename);
		close(skfd);
		return 0;
	}

	if (size)
		memcpy(result, tmp, size);
	else
		strcpy(result, (char *)tmp);

	close(skfd);
	return 0;
}

INT8U rtk_linux_get_mib_from_radio_data(char *interfacename, char *mibname, void *result, int size)
{
	char         tmp[64] = {0};
	INT32U       tmp_32u = 0;
	INT8U        tmp_8u = 0;
	int          tmp_int = 0;
	int          i, j;
	char        root_name[32] = {0};
	char        interface_name[64]= {0};
	INT8U       bss_index;
	int         radio_idx = -1, if_idx = -1;

	// get bss index from interface name
	for (i = 0; i < radio_data_nr; i++) { // radio band
		if (if_idx != -1)
			break;
		if (radio_data[i].radio_band == RADIO_BAND_5GH)
			sprintf(root_name, "%s", map_radio_name_5gh);
		else if (radio_data[i].radio_band == RADIO_BAND_5GL)
			sprintf(root_name, "%s", map_radio_name_5gl);
		else if (radio_data[i].radio_band == RADIO_BAND_2G)
			sprintf(root_name, "%s", map_radio_name_24g);

		for (j = 0; j < radio_data[i].interface_nr; j++) { // bss
			bss_index = radio_data[i].interface_mib[j].bss_index;
			if(0 == bss_index) {
				sprintf(interface_name, "%s", root_name);
			} else {
				sprintf(interface_name, "%s-%s%d", root_name, map_vap_prefix, bss_index - 1);
			}
			if (0 == PLATFORM_STRCMP(interfacename, interface_name)) {
				radio_idx = i;
				if_idx = j;
				break;
			}
		}
	}

	if (if_idx == -1)
		return 0;

	i = radio_idx;
	j = if_idx;
	
	if (0 == PLATFORM_STRCMP(mibname, "use40M")) {
		tmp_int = radio_data[i].channel_bandwidth;
		memcpy(tmp, &tmp_int, size);
	} else if (0 == PLATFORM_STRCMP(mibname, "2ndchoffset")) {
		if (radio_data[i].radio_band == RADIO_BAND_2G && radio_data[i].channel_bandwidth == BAND_WIDTH_40MHZ) {
			if (radio_data[i].control_sideband == 0) // upper
				tmp_int = 1; // below
			else if (radio_data[i].control_sideband == 1) // lower
				tmp_int = 2; // above
			else
				tmp_int = 0;
		} else
			tmp_int = 0;
		memcpy(tmp, &tmp_int, size);
	} else if (0 == PLATFORM_STRCMP(mibname, "regdomain")) {
		tmp_32u = hw_reg_domain;
		memcpy(tmp, &tmp_32u, size);
	} else if (0 == PLATFORM_STRCMP(mibname, "opmode")) {
		if (PLATFORM_STRSTR(interfacename, map_vxd_prefix))
			tmp_32u = WIFI_STATION_STATE;
		else
			tmp_32u = WIFI_AP_STATE;
		memcpy(tmp, &tmp_32u, size);
	} else if (0 == PLATFORM_STRCMP(mibname, "func_off")) {
		tmp_32u = !radio_data[i].interface_mib[j].is_enabled;
		memcpy(tmp, &tmp_32u, size);
	} else if (0 == PLATFORM_STRCMP(mibname, "authtype")) {
		tmp_32u = radio_data[i].interface_mib[j].authentication_type;
		memcpy(tmp, &tmp_32u, size);
	} else if (0 == PLATFORM_STRCMP(mibname, "multiap_bss_type")) {
		tmp_8u = radio_data[i].interface_mib[j].network_type;
		memcpy(tmp, &tmp_8u, size);
	} else if (0 == PLATFORM_STRCMP(mibname, "channel")) {
		if (radio_data[i].radio_channel) {
			tmp_8u = radio_data[i].radio_channel;
		} else {
			// Assign a non-zero default value
			if (radio_data[i].radio_band == RADIO_BAND_5GH)
				tmp_8u = 149;
			else if (radio_data[i].radio_band == RADIO_BAND_5GL)
				tmp_8u = 40;
			else if (radio_data[i].radio_band == RADIO_BAND_2G)
				tmp_8u = 6;
		}
		memcpy(tmp, &tmp_8u, size);
	} else if (0 == PLATFORM_STRCMP(mibname, "max_tx_power")) {
		tmp_8u = 20;
		memcpy(tmp, &tmp_8u, size);
	} else if (0 == PLATFORM_STRCMP(mibname, "band")) {
		if (radio_data[i].radio_band == RADIO_BAND_2G)
			tmp_8u = (WIRELESS_11B | WIRELESS_11G | WIRELESS_11N | WIRELESS_11AX);
		else
			tmp_8u = (WIRELESS_11A | WIRELESS_11N | WIRELESS_11AC | WIRELESS_11AX);
		memcpy(tmp, &tmp_8u, size);
	} else if (0 == PLATFORM_STRCMP(mibname, "encmode")) {
		tmp_8u = radio_data[i].interface_mib[j].encrypt_type;
		memcpy(tmp, &tmp_8u, size);
	} else if (0 == PLATFORM_STRCMP(mibname, "ssid")) {
		strlcpy(tmp, radio_data[i].interface_mib[j].ssid, sizeof(tmp)); // String
	}

	if (size)
		memcpy(result, tmp, size);
	else
		strcpy(result, (char *)tmp);

	return 0;
}

INT8U rtk_linux_send_disassoc(char *interfacename, INT8U *sta_mac)
{
	int           skfd;
	int           err;
	struct iwreq  wrq;
	unsigned char tmp[30];

	skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (skfd < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("ioctl [RTL8192CD_IOCTL_SEND_DISASSOC] socket error\n");
		return err;
	}

	memset(&wrq, 0, sizeof(wrq));
	strlcpy(wrq.ifr_name, interfacename, sizeof(wrq.ifr_name));
	if (sta_mac != NULL) {
		memcpy(tmp, sta_mac, 6);
	} else
		PLATFORM_PRINTF_ERROR("ioctl [RTL8192CD_IOCTL_SEND_DISASSOC] sta_mac is NULL\n");

	wrq.u.data.pointer = (caddr_t)tmp;
	wrq.u.data.length  = 6; //MACADDRLEN

	/* Do the request */
	if (ioctl(skfd, RTL8192CD_IOCTL_SEND_DISASSOC, &wrq) < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("ioctl [RTL8192CD_IOCTL_SEND_DISASSOC] on %s failed!\n", interfacename);
		close(skfd);
		return err;
	}
	err = 0;

	close(skfd);
	return err;
}

#define RTL8192CD_IOCTL_DEL_STA 0x89F7
INT8U rtk_linux_del_sta(char *interfacename, INT8U *sta_mac)
{
	int          skfd;
	int          err;
	struct iwreq wrq;
	char         hex_str[3];

	unsigned char tmp[30] = {0};

	unsigned char *ptr = NULL;

	int i = 0;

	skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (skfd < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("ioctl [RTL8192CD_IOCTL_DEL_STA] socket error\n");
		return err;
	}

	memset(&wrq, 0, sizeof(wrq));
	strlcpy(wrq.ifr_name, interfacename, sizeof(wrq.ifr_name));
	if (sta_mac != NULL) {
		ptr = &tmp[0];
		for (i = 0; i < 6; i++) {
			sprintf(hex_str, "%02x", sta_mac[i]);
			PLATFORM_MEMCPY(ptr, hex_str, 2);
			ptr += 2;
		}
	} else
		PLATFORM_PRINTF_ERROR("ioctl [RTL8192CD_IOCTL_DEL_STA] sta_mac is NULL\n");

	tmp[12] = 'n';
	tmp[13] = 'o';
	tmp[14] = 0;

	wrq.u.data.pointer = (caddr_t)tmp;
	wrq.u.data.length  = 15; // MACADDRLEN Sta + no + 0

	/* Do the request */
	if (ioctl(skfd, RTL8192CD_IOCTL_DEL_STA, &wrq) < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("ioctl [RTL8192CD_IOCTL_DEL_STA] on %s failed!\n", interfacename);
		close(skfd);
		return err;
	}
	err = 0;

	close(skfd);
	return err;
}

#define RTL8192CD_IOCTL_ASSOC_CONTROL 0x8B38
INT8U rtk_linux_assoc_control(char *interfacename, INT8U *sta_mac, INT16U period, INT8U control)
{
	struct assoc_control_para assoc_ctrl_para;
	int                       skfd;
	int                       err;
	struct iwreq              wrq;
	char                      tmp[30];

	skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (skfd < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("ioctl [RTL8192CD_IOCTL_ASSOC_CONTROL] socket error\n");
		return err;
	}
	memset(&assoc_ctrl_para, 0, sizeof(struct assoc_control_para));
	memcpy(assoc_ctrl_para.addr, sta_mac, 6);
	assoc_ctrl_para.period  = period;
	assoc_ctrl_para.control = control;
	memcpy(tmp, &assoc_ctrl_para, sizeof(struct assoc_control_para));

	memset(&wrq, 0, sizeof(wrq));
	strlcpy(wrq.ifr_name, interfacename, sizeof(wrq.ifr_name));

	wrq.u.data.pointer = (caddr_t)tmp;
	wrq.u.data.length  = sizeof(struct assoc_control_para);

	/* Do the request */
	if (ioctl(skfd, RTL8192CD_IOCTL_ASSOC_CONTROL, &wrq) < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("ioctl [RTL8192CD_IOCTL_ASSOC_CONTROL] on %s failed!\n", interfacename);
		close(skfd);
		return err;
	}
	err = 0;

	close(skfd);
	return err;
}

#define RTL8192CD_IOCTL_GET_AP_CAPABILITY 0x8B51

INT8U rtk_linux_get_ap_capability_info(char *interface_name, INT8U tlv_type, unsigned char *result)
{
	int          s;
	struct iwreq wrq;
	//unsigned char         tmp[2048] = {0};
	//unsigned char *buf;
	unsigned int len = 0;
	char type[8] = {0};

	switch (tlv_type) {
		case 0:
			break;
		case 1:
			strcpy(type, "(HT)");
			break;
		case 2:
			strcpy(type, "(VHT)");
			break;
		case 3:
			strcpy(type, "(HE)");
			break;
	}

	PLATFORM_PRINTF_DEBUG("[PLATFORM] Preparing to get AP Capability%s\n", type);
	PLATFORM_PRINTF_DEBUG("[PLATFORM]   - Interface name = %s\n", interface_name);

	s = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (-1 == s) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] socket('%s') returned with errno=%d (%s) while opening a DGRAM socket\n", interface_name, errno, strerror(errno));
		return 0;
	}

	*result = tlv_type;

	PLATFORM_MEMSET(&wrq, 0, sizeof(wrq));
	//PLATFORM_PRINTF_DEBUG("[PLATFORM] Set interface type\n");
	strncpy(wrq.ifr_name, interface_name, IFNAMSIZ - 1);
	wrq.u.data.pointer = (caddr_t)result;
	wrq.u.data.length  = 1;

	if (ioctl(s, RTL8192CD_IOCTL_GET_AP_CAPABILITY, &wrq) < 0) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] ioctl('%s',RTL8192CD_IOCTL_GET_AP_CAPABILITY)%s returned with errno=%d (%s)\n", interface_name, type, errno, strerror(errno));
		close(s);
		return 0;
	}

	len = wrq.u.data.length;
	//buf = *(unsigned char *)wrq.u.data.pointer;

	//PLATFORM_PRINTF_DETAIL("[PLATFORM] AP Capability retrieved (Length: %d)!\n", wrq.u.data.length);
	/*
	PLATFORM_PRINTF_DEBUG("IOCTL contents:\n");
	for(i = 0; i < len; i++)
		PLATFORM_PRINTF_DEBUG("%02x", tmp[i]);
	PLATFORM_PRINTF_DEBUG("\n");
*/
	//PLATFORM_MEMCPY(result, buf , len);

	close(s);

	return len;
}

INT8U rtk_get_associated_clients(char *interface_name, INT8U *results)
{
	// This is a copy of wirelessClientList() in boa/src/fmwlan.c:6752 to get all associated stations info
	INT8U            client_nr = 0;
	char *           buff;
	WLAN_STA_INFO_Tp pInfo;
	INT8U            i;

	PLATFORM_PRINTF_DEBUG("[PLATFORM] Preparing RTK get associated clients\n");

	buff = calloc(1, sizeof(WLAN_STA_INFO_T) * (map_max_sta_num + 1));
	if (buff == 0) {
		PLATFORM_PRINTF_ERROR("[RTL] Allocate buffer failed!\n");
		return 0;
	}

	if (_getWlStaInfo(interface_name, (WLAN_STA_INFO_Tp)buff) < 0) {
		PLATFORM_PRINTF_ERROR("[RTL] Read wlan sta info failed!\n");
	}
	for (i = 1; i <= map_max_sta_num; i++) {
		pInfo = (WLAN_STA_INFO_Tp)&buff[i * sizeof(WLAN_STA_INFO_T)];
#if defined(_DRIVER_COMMON_STRUCTURE_) || defined(CONFIG_RTK_PON_XDSL)
		if (pInfo->aid && (pInfo->flags & STA_INFO_FLAG_ASOC)) {
#else
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC)) {
#endif
			// calcuate the number of associated devices
			memcpy(&results[(i - 1) * 6], pInfo->addr, 6);
			client_nr += 1;
		}
	}
	if (client_nr <= 0) {
		PLATFORM_PRINTF_FULL("[RTL] [%s] NO station is associated on this interface.\n",
		                     interface_name);
	}

	free(buff);
	PLATFORM_PRINTF_DEBUG("[PLATFORM] Rtk get associated clients - returning client nr: %d\n", client_nr);
	return client_nr;
}

#define RTL8192CD_IOCTL_GET_CLIENT_CAPABILITY 0x8B52

INT8U rtk_linux_get_client_capability_info(char *interface_name, INT8U *macaddr, INT8U *results)
{
	int           s;
	struct iwreq  wrq;
	unsigned char len = 0;

	PLATFORM_PRINTF_DEBUG("[PLATFORM] Preparing to get Client Capability\n");
	PLATFORM_PRINTF_DEBUG("[PLATFORM]   - Interface name = %s\n", interface_name);

	s = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (-1 == s) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] socket('%s') returned with errno=%d (%s) while opening a DGRAM socket\n", interface_name, errno, strerror(errno));
		return 0;
	}

	PLATFORM_MEMCPY(results, macaddr, MACADDRLEN);

	PLATFORM_MEMSET(&wrq, 0, sizeof(wrq));
	//PLATFORM_PRINTF_DEBUG("[PLATFORM] Set interface type\n");
	strncpy(wrq.ifr_name, interface_name, IFNAMSIZ - 1);
	wrq.u.data.pointer = (caddr_t)results;
	wrq.u.data.length  = 6;

	if (ioctl(s, RTL8192CD_IOCTL_GET_CLIENT_CAPABILITY, &wrq) < 0) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] ioctl('%s',RTL8192CD_IOCTL_GET_CLIENT_CAPABILITY) returned with errno=%d (%s)\n", interface_name, errno, strerror(errno));
		close(s);
		return 0;
	}

	PLATFORM_PRINTF_DEBUG("[PLATFORM] Client Capability retrieved!\n");

	len = wrq.u.data.length;

	close(s);

	return len;
}

#define SIOC11KBEACONREQ 0x8BD2

int rtk_linux_issue_beacon_measurement(char *ifname, unsigned char *macaddr,
                                       struct BeaconMeasurementRequest *beacon_req)
{
	int           sock;
	struct iwreq  wrq;
	int           err;
	int           len = 0;
	unsigned char tempbuf[2048];

	/*** Initialize socket ***/
	sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (sock < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("[%s %d]: Can't create socket for ioctl. %s(%d)", __FUNCTION__, __LINE__, ifname, err);
		return err;
	}
	memcpy(tempbuf, macaddr, MACADDRLEN);
	len += MACADDRLEN;
	memcpy(tempbuf + len, beacon_req, sizeof(struct BeaconMeasurementRequest));
	len += sizeof(struct BeaconMeasurementRequest);

	/*** Initialize struct iwreq ***/
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, ifname, IFNAMSIZ - 1);

	/*** give parameter and buffer ***/
	wrq.u.data.pointer = (caddr_t)tempbuf;
	wrq.u.data.length  = len;

	/*** ioctl ***/
	if (ioctl(sock, SIOC11KBEACONREQ, &wrq) < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("[%s %d]: %s ioctl Error.(%d)", __FUNCTION__, __LINE__, ifname, err);
		close(sock);
		return err;
	}
	err = 0;

	close(sock);
	return err;
}

INT32U rtk_get_prefer_bssid_interval(void)
{
	return 0;
}

void rtk_reset_prefer_bssid(void)
{
	return;
}

#define SIOCMAP_BACKHAULSTEER 0x8B53

int rtk_linux_do_backhaul_steering(char *ifname, unsigned char *backhaul_bss, unsigned char *target_bssid, unsigned char op_class, unsigned char channel)
{
	int           sock;
	struct iwreq  wrq;
	int           err;
	int           len = 0;
	unsigned char tempbuf[2048];

	/*** Initialize socket ***/
	sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (sock < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("[%s %d]: Can't create socket for ioctl. %s(%d)", __FUNCTION__, __LINE__, ifname, err);
		return err;
	}
	memcpy(tempbuf, backhaul_bss, MACADDRLEN);
	memcpy(tempbuf + 6, target_bssid, MACADDRLEN);
	len += (MACADDRLEN * 2);

	tempbuf[12] = op_class;
	tempbuf[13] = channel;
	len += 2;

	/*** Initialize struct iwreq ***/
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, ifname, IFNAMSIZ - 1);

	/*** give parameter and buffer ***/
	wrq.u.data.pointer = (caddr_t)tempbuf;
	wrq.u.data.length  = len;

	/*** ioctl ***/
	if (ioctl(sock, SIOCMAP_BACKHAULSTEER, &wrq) < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("%s backhaul steering ioctl returned with errno (%d) %s", ifname, errno, strerror(errno));
		close(sock);
		return err;
	}

	close(sock);
	return 0;
}

int rtk_linux_do_channel_scan(char *interface_name, unsigned char channel_nr, unsigned char *channels)
{
	int           sock;
	struct iwreq  wrq;
	int           err;
	int           len = 0;
	unsigned char tempbuf[2048];

	/*** Initialize socket ***/
	sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (sock < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("[%s %d]: Can't create socket for ioctl. %s(%d)", __FUNCTION__, __LINE__, interface_name, err);
		return err;
	}
	memcpy(tempbuf, &channel_nr, 1);
	len += 1;
	memcpy(tempbuf + 1, channels, channel_nr);
	len += channel_nr;

	/*** Initialize struct iwreq ***/
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, interface_name, IFNAMSIZ - 1);

	/*** give parameter and buffer ***/
	wrq.u.data.pointer = (caddr_t)tempbuf;
	wrq.u.data.length  = len;

	/*** ioctl ***/
	if (ioctl(sock, SIOCMAP_CHANNELSCAN, &wrq) < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("[%s %d]: %s ioctl Error.(%d)\n", __FUNCTION__, __LINE__, interface_name, err);
		close(sock);
		return err;
	}
	err = 0;

	close(sock);
	return err;
}

#define SIOCMAP_GETAPMETRIC 0x8B54
#define SIOCMAP_GETASSOCSTAMETRIC 0x8B55
#define SIOCMAP_GETUNASSOCSTAMETRIC 0x8B56

int rtk_linux_get_ap_metric(char *ifname, INT8U tlv_type, INT8U *results)
{
	int          sock;
	struct iwreq wrq;
	int          err;
	int          len = 0;
	//unsigned char tempbuf[2048];

	/*** Initialize socket ***/
	sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (sock < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("[%s %d]: Can't create socket for ioctl. %s(%d)", __FUNCTION__, __LINE__, ifname, err);
		return len;
	}

	*results = tlv_type;

	/*** Initialize struct iwreq ***/
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, ifname, IFNAMSIZ - 1);

	/*** give parameter and buffer ***/
	wrq.u.data.pointer = (caddr_t)results;
	wrq.u.data.length  = 1;

	/*** ioctl ***/
	if (ioctl(sock, SIOCMAP_GETAPMETRIC, &wrq) < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("[%s %d]: %s ioctl Error.(%d)", __FUNCTION__, __LINE__, ifname, err);
		len = 0;
		close(sock);
		return 0;
	} else {
		err = 0;
		len = 1;
	}

	close(sock);
	return len;
}

int rtk_linux_get_assocSta_metric(char *ifname, INT8U tlv_type, INT8U *macaddr, INT8U *results)
{
	int          sock;
	struct iwreq wrq;
	int          err;
	int          len = 0;
	//unsigned char tempbuf[2048];

	/*** Initialize socket ***/
	sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (sock < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("[%s %d]: Can't create socket for ioctl. %s(%d)", __FUNCTION__, __LINE__, ifname, err);
		return 0;
	}

	results[0] = tlv_type;
	memcpy(&results[1], macaddr, MACADDRLEN);

	/*** Initialize struct iwreq ***/
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, ifname, IFNAMSIZ - 1);

	/*** give parameter and buffer ***/
	wrq.u.data.pointer = (caddr_t)results;
	wrq.u.data.length  = 7;

	/*** ioctl ***/
	if (ioctl(sock, SIOCMAP_GETASSOCSTAMETRIC, &wrq) < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("[%s %d]: %s ioctl Error.(%d)", __FUNCTION__, __LINE__, ifname, err);
		close(sock);
		return 0;
	} else {
		err = 0;
		len = 1;
	}

	close(sock);
	return len;
}

int rtk_linux_record_unAssocSta_metric(char *ifname, INT8U op_class, INT8U channel_nr, INT8U sta_nr, INT8U (*macaddr)[6])
{
	int           sock;
	struct iwreq  wrq;
	int           err;
	int           len = 0;
	unsigned char tempbuf[2048];

	/*** Initialize socket ***/
	sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (sock < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("[%s %d]: Can't create socket for ioctl. %s(%d)", __FUNCTION__, __LINE__, ifname, err);
		return err;
	}

	tempbuf[0] = op_class;
	tempbuf[1] = channel_nr;
	tempbuf[2] = sta_nr;
	len += 3;

	memcpy(&tempbuf[3], macaddr, (sta_nr * MACADDRLEN));
	len += (sta_nr * MACADDRLEN);

	/*** Initialize struct iwreq ***/
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, ifname, IFNAMSIZ - 1);

	/*** give parameter and buffer ***/
	wrq.u.data.pointer = (caddr_t)tempbuf;
	wrq.u.data.length  = len;

	/*** ioctl ***/
	if (ioctl(sock, SIOCMAP_GETUNASSOCSTAMETRIC, &wrq) < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("[%s %d]: %s ioctl Error.(%d)", __FUNCTION__, __LINE__, ifname, err);
		close(sock);
		return err;
	}
	err = 0;

	close(sock);
	return err;
}

int rtk_linux_metric_policy_ioctl(char *interface_name, INT8U rcpi_threshold, INT8U rcpi_hysteris_margin, INT8U channel_utilization_threshold)
{
	int          s;
	struct iwreq wrq;
	char         tmp[5] = { 0 };

	tmp[0] = rcpi_threshold;
	tmp[1] = rcpi_hysteris_margin;
	tmp[2] = channel_utilization_threshold;

	PLATFORM_PRINTF_DETAIL("[PLATFORM] Preparing to update metric policy:\n");
	PLATFORM_PRINTF_DETAIL("[PLATFORM]   - Interface name = %s\n", interface_name);
	PLATFORM_PRINTF_DETAIL("[PLATFORM]   - RCPI Threshold = %d\n", rcpi_threshold);
	PLATFORM_PRINTF_DETAIL("[PLATFORM]   - RCPI Hysteris  = %d\n", rcpi_hysteris_margin);
	PLATFORM_PRINTF_DETAIL("[PLATFORM]   - Channel Utilization = %d\n", channel_utilization_threshold);

	s = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (-1 == s) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] socket('%s') returned with errno=%d (%s) while opening a DGRAM socket\n", interface_name, errno, strerror(errno));
		return 0;
	}
	memset(&wrq, 0, sizeof(wrq));

	PLATFORM_PRINTF_DETAIL("[PLATFORM] Set interface type\n");
	strncpy(wrq.ifr_name, interface_name, IFNAMSIZ - 1);
	wrq.u.data.pointer = tmp;
	wrq.u.data.length  = 3;

	if (ioctl(s, SIOCMAP_UPDATEPOLICY, &wrq) < 0) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] ioctl('%s',SIOCMAP_UPDATEPOLICY) returned with errno=%d (%s) while updating\n", interface_name, errno, strerror(errno));
		close(s);
		return 0;
	}

	PLATFORM_PRINTF_DETAIL("[PLATFORM] Policy updated!\n");

	close(s);
	return 1;
}

#define RTL8192CD_IOCTL_AGENT_STEERING 0x8B57

int rtk_linux_steering_policy_ioctl(char *interface_name, INT8U policy, INT8U channel_utilization_threshold, INT8U rcpi_threshold)
{
	int          s;
	struct iwreq wrq;
	char         tmp[5] = { 0 };

	tmp[0] = policy;
	tmp[1] = channel_utilization_threshold;
	tmp[2] = rcpi_threshold;

	PLATFORM_PRINTF_DETAIL("[PLATFORM] Preparing to update steering policy:\n");
	PLATFORM_PRINTF_DETAIL("[PLATFORM]   - Interface name = %s\n", interface_name);
	PLATFORM_PRINTF_DETAIL("[PLATFORM]   - Policy = %02x\n", policy);
	PLATFORM_PRINTF_DETAIL("[PLATFORM]   - Channel Utilization = %d\n", channel_utilization_threshold);
	PLATFORM_PRINTF_DETAIL("[PLATFORM]   - RCPI Threshold = %d\n", rcpi_threshold);

	s = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (-1 == s) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] socket('%s') returned with errno=%d (%s) while opening a DGRAM socket\n", interface_name, errno, strerror(errno));
		return 0;
	}
	memset(&wrq, 0, sizeof(wrq));

	PLATFORM_PRINTF_DETAIL("[PLATFORM] Set interface type\n");
	strncpy(wrq.ifr_name, interface_name, IFNAMSIZ - 1);
	wrq.u.data.pointer = tmp;
	wrq.u.data.length  = 3;

	if (ioctl(s, RTL8192CD_IOCTL_AGENT_STEERING, &wrq) < 0) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] ioctl('%s',RTL8192CD_IOCTL_AGENT_STEERING) returned with errno=%d (%s) while updating\n", interface_name, errno, strerror(errno));
		close(s);
		return 0;
	}

	PLATFORM_PRINTF_DETAIL("[PLATFORM] Steering Policy updated!\n");

	close(s);
	return 1;
}

#define SIOCMAP_SET_TXMAXPOWER 0x8B95

int rtk_linux_set_tx_max_power_ioctl(char *interface_name, INT8U tx_max_power)
{
	int          s;
	struct iwreq wrq;
	char         tmp[1] = { 0 };

	tmp[0] = tx_max_power;

	PLATFORM_PRINTF_DEBUG("[PLATFORM] Preparing to set max tx power:\n");
	PLATFORM_PRINTF_DEBUG("[PLATFORM]   - Interface name = %s\n", interface_name);
	PLATFORM_PRINTF_DEBUG("[PLATFORM]   - Max tx power = %02x\n", tx_max_power);

	s = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (-1 == s) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] socket('%s') returned with errno=%d (%s) while opening a DGRAM socket\n", interface_name, errno, strerror(errno));
		return 0;
	}
	memset(&wrq, 0, sizeof(wrq));

	PLATFORM_PRINTF_DEBUG("[PLATFORM] Set interface type\n");
	strncpy(wrq.ifr_name, interface_name, IFNAMSIZ - 1);
	wrq.u.data.pointer = tmp;
	wrq.u.data.length  = 1;

	if (ioctl(s, SIOCMAP_SET_TXMAXPOWER, &wrq) < 0) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] ioctl('%s',SIOCMAP_SET_TXMAXPOWER) returned with errno=%d (%s) while updating\n", interface_name, errno, strerror(errno));
		close(s);
		return 0;
	}

	PLATFORM_PRINTF_DEBUG("[PLATFORM] Max tx power updated!\n");

	close(s);
	return 1;
}

int rtk_linux_do_cac(char *interface_name, unsigned char channel_nr, unsigned char *channels, unsigned char type)
{
	int           sock;
	struct iwreq  wrq;
	int           len = 0;
	unsigned char tempbuf[2048];

	/*** Initialize socket ***/
	sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (sock < 0) {
		PLATFORM_PRINTF_ERROR("[%s %d]: Can't create socket for ioctl. %s(%d) %s\n", __FUNCTION__, __LINE__, interface_name, errno, strerror(errno));
		return 1;
	}

	memcpy(tempbuf, &type, 1);
	len += 1;
	memcpy(tempbuf + 1, &channel_nr, 1);
	len += 1;

	if (0 == type) {
		memcpy(tempbuf + 2, channels, channel_nr * 3);
		len += channel_nr * 3;
	} else {
		memcpy(tempbuf + 2, channels, channel_nr * 2);
		len += channel_nr * 2;
	}

	/*** Initialize struct iwreq ***/
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, interface_name, IFNAMSIZ - 1);

	/*** give parameter and buffer ***/
	wrq.u.data.pointer = (caddr_t)tempbuf;
	wrq.u.data.length  = len;

	/*** ioctl ***/
	if (ioctl(sock, SIOCMAP_CAC, &wrq) < 0) {
		PLATFORM_PRINTF_ERROR("[%s %d]: %s ioctl Error.(%d) %s", __FUNCTION__, __LINE__, interface_name, errno, strerror(errno));
		close(sock);
		return 1;
	}
	close(sock);
	return 0;
}

INT8U rtk_linux_get_bss_type(char *interface_name)
{
	INT8U bss_type = 0;

	if (PLATFORM_GET_VALID_INTERFACE_NAME(interface_name, VALID_ETH_INTERFACES_IN_BRIDGE)
		|| (controller_interface && PLATFORM_STRSTR(interface_name, controller_interface))) {
		return (MULTI_AP_FRONTHAUL_BSS | MULTI_AP_BACKHAUL_BSS);
	}

	PLATFORM_GET_MIB(interface_name, "multiap_bss_type", &bss_type, 1);

	PLATFORM_PRINTF_FULL("[RTL] %s bss_type %02x\n", interface_name, bss_type);

	return bss_type;
}

OP_CLASS *rtk_linux_get_op_class(int *op_classes_len)
{
	INT32U domain = DOMAIN_GLOBAL;

	int       reg_op_classes_len = 0;
	OP_CLASS *reg_op_classes     = NULL;
	int       i                  = 0;
	INT8U     index_buf[32]      = { 0 };

	// reg_op_classes = (OP_CLASS *)PLATFORM_MALLOC(sizeof(OP_CLASS)* 32);

	if (PLATFORM_IS_INTERFACE_UP(map_radio_name_5gh)) {
		PLATFORM_GET_MIB(map_radio_name_5gh, "regdomain", &domain, 4);
	} else if (PLATFORM_IS_INTERFACE_UP(map_radio_name_5gl)) {
		PLATFORM_GET_MIB(map_radio_name_5gl, "regdomain", &domain, 4);
	} else {
		PLATFORM_GET_MIB(map_radio_name_24g, "regdomain", &domain, 4);
	}

	for (i = 0; i < (int)(sizeof(GLOBAL_OP_CLASS) / sizeof(OP_CLASS)); i++) {
		if (DOMAIN_GLOBAL == domain || DOMAIN_WORLD_WIDE == domain) {
			switch (GLOBAL_OP_CLASS[i].op_class) {
			case 82:
				continue;
			}
		} else if (DOMAIN_ETSI == domain) {
			switch (GLOBAL_OP_CLASS[i].op_class) {
			case 82:
			case 124:
			case 126:
			case 127:
				continue;
			}
		} else if (DOMAIN_MKK == domain) {
			switch (GLOBAL_OP_CLASS[i].op_class) {
			case 124:
			case 125:
			case 126:
			case 127:
				continue;
			}
		} else if (DOMAIN_CN == domain) {
			switch (GLOBAL_OP_CLASS[i].op_class) {
			case 82:
			case 117:
			case 120:
			case 121:
			case 122:
			case 123:
			case 124:
			case 127:
				continue;
			}
		}

		index_buf[reg_op_classes_len] = i;

		reg_op_classes_len += 1;
	}

	reg_op_classes = (OP_CLASS *)PLATFORM_MALLOC(sizeof(OP_CLASS) * reg_op_classes_len);

	for (i = 0; i < reg_op_classes_len; i++) {
		int index = index_buf[i];
		reg_op_classes[i].op_class      = GLOBAL_OP_CLASS[index].op_class;
		reg_op_classes[i].band          = GLOBAL_OP_CLASS[index].band;
		reg_op_classes[i].sub_band      = GLOBAL_OP_CLASS[index].sub_band;
		reg_op_classes[i].bandwidth     = GLOBAL_OP_CLASS[index].bandwidth;
		reg_op_classes[i].side_band     = GLOBAL_OP_CLASS[index].side_band;
		reg_op_classes[i].channel_array = GLOBAL_OP_CLASS[index].channel_array;
	}

	*op_classes_len = reg_op_classes_len;
	return (OP_CLASS *)reg_op_classes;
}

INT8U rtk_linux_convert_to_global_op_class(INT8U op_class)
{
	INT32U domain = DOMAIN_GLOBAL;

	if (op_class > 130) {
		PLATFORM_PRINTF_ERROR("Unsupported op class %d\n", op_class);
		return 0;
	}

	if (op_class >= 60) {
		return op_class;
	}

	if (PLATFORM_IS_INTERFACE_UP(map_radio_name_5gh)) {
		PLATFORM_GET_MIB(map_radio_name_5gh, "regdomain", &domain, 4);
	} else if (PLATFORM_IS_INTERFACE_UP(map_radio_name_5gl)) {
		PLATFORM_GET_MIB(map_radio_name_5gl, "regdomain", &domain, 4);
	} else {
		PLATFORM_GET_MIB(map_radio_name_24g, "regdomain", &domain, 4);
	}

	switch (domain) {
	case DOMAIN_FCC:
		return us_to_global_lookup[op_class];
	case DOMAIN_ETSI:
		return eu_to_global_lookup[op_class];
	case DOMAIN_MKK:
		return jp_to_global_lookup[op_class];
	case DOMAIN_CN:
		return cn_to_global_lookup[op_class];
	case DOMAIN_GLOBAL:
	case DOMAIN_KR:
	case DOMAIN_WORLD_WIDE:
		return op_class;
	default:
		// PLATFORM_PRINTF_DEBUG("Unknown op class %d for domain %d\n", op_class, domain);
		return op_class;
	}
}

INT8U rtk_linux_convert_to_local_op_class(INT8U op_class)
{
	INT32U domain = DOMAIN_GLOBAL;

	if (PLATFORM_IS_INTERFACE_UP(map_radio_name_5gh)) {
		PLATFORM_GET_MIB(map_radio_name_5gh, "regdomain", &domain, 4);
	} else if (PLATFORM_IS_INTERFACE_UP(map_radio_name_5gl)) {
		PLATFORM_GET_MIB(map_radio_name_5gl, "regdomain", &domain, 4);
	} else {
		PLATFORM_GET_MIB(map_radio_name_24g, "regdomain", &domain, 4);
	}

	switch (domain) {
	case DOMAIN_FCC:
		return global_to_us_lookup[op_class - 81];
	case DOMAIN_ETSI:
		return global_to_eu_lookup[op_class - 81];
	case DOMAIN_MKK:
		return global_to_jp_lookup[op_class - 81];
	case DOMAIN_CN:
		return global_to_cn_lookup[op_class - 81];
	case DOMAIN_GLOBAL:
	case DOMAIN_KR:
	case DOMAIN_WORLD_WIDE:
		return op_class;
	default:
		// PLATFORM_PRINTF_DEBUG("Unknown op class %d for domain %d\n", op_class, domain);
		return op_class;
	}
}

INT8U ch_5g_dfs[16][25] = {
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // No defined
		{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140,144,149,153,157,161,165}, // DOMAIN_FCC 1
		{36,40,44,48,52,56,60,64,100,104,108,112,116,132,136,140,144,149,153,157,161,165,0,0,0}, // DOMAIN_IC 2
		{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140,0,0,0,0,0,0}, // DOMAIN_ETSI 3
		{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140,0,0,0,0,0,0}, /// DOMAIN_SPAIN 4
		{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140,0,0,0,0,0,0}, // DOMAIN_FRANCE 5
		{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140,0,0,0,0,0,0}, // DOMAIN_MKK 6
		{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140,0,0,0,0,0,0}, // DOMAIN_ISRAEL 7
		{34,38,42,46,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // DOMAIN_MKK1 8
		{36,40,44,48,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // DOMAIN_MKK2 9
		{36,40,44,48,52,56,60,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // DOMAIN_MKK3 10
		{56,60,64,100,104,108,112,116,136,140,149,153,157,161,165,0,0,0,0,0,0,0,0,0,0}, // DOMAIN_NCC 11
		{36,40,44,48,52,56,60,64,132,136,140,149,153,157,161,165,0,0,0,0,0,0,0,0,0}, // DOMAIN_RUSSIAN 12
		{36,40,44,48,52,56,60,64,149,153,157,161,165,0,0,0,0,0,0,0,0,0,0,0,0}, // DOMAIN_CN 13
		{36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161,165,0,0,0,0,0}, // DOMAIN_GLOBAL 14
		{36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161,165,0,0,0,0,0}}; // DOMAIN_WORLD_WIDE 15
#define SIOCMAP_GET_AVAILABLE_CHANNELS 0x8B94
INT8U rtk_linux_get_available_channels(char *ifname, INT8U *buffer, INT16U buffer_size)
{
	int          sock;
	struct iwreq wrq;
	int          err;
	int          len = 0;
	int          i, j, k;
	char         root_name[32]      = { 0 };
	char         interface_name[64] = { 0 };
	INT8U        bss_index;
	int          radio_idx = -1, if_idx = -1;
	INT32U       domain = DOMAIN_GLOBAL;

	if (eth_only && PLATFORM_STRSTR(ifname, "wlan") != NULL) {
		// get bss index from interface name
		for (i = 0; i < radio_data_nr; i++) { // radio band
			if (if_idx != -1)
				break;
			if (radio_data[i].radio_band == RADIO_BAND_5GH)
				sprintf(root_name, "%s", map_radio_name_5gh);
			else if (radio_data[i].radio_band == RADIO_BAND_5GL)
				sprintf(root_name, "%s", map_radio_name_5gl);
			else if (radio_data[i].radio_band == RADIO_BAND_2G)
				sprintf(root_name, "%s", map_radio_name_24g);

			for (j = 0; j < radio_data[i].interface_nr; j++) { // bss
				bss_index = radio_data[i].interface_mib[j].bss_index;
				if(0 == bss_index) {
					sprintf(interface_name, "%s", root_name);
				} else {
					sprintf(interface_name, "%s-%s%d", root_name, map_vap_prefix, bss_index - 1);
				}
				if (0 == PLATFORM_STRCMP(ifname, interface_name)) {
					radio_idx = i;
					if_idx = j;
					break;
				}
			}
		}

		if (if_idx == -1)
			return 0;

		i = radio_idx;
		j = if_idx;

		// get regdomain
		if (PLATFORM_IS_INTERFACE_UP(map_radio_name_5gh)) {
			PLATFORM_GET_MIB(map_radio_name_5gh, "regdomain", &domain, 4);
		} else if (PLATFORM_IS_INTERFACE_UP(map_radio_name_5gl)) {
			PLATFORM_GET_MIB(map_radio_name_5gl, "regdomain", &domain, 4);
		} else {
			PLATFORM_GET_MIB(map_radio_name_24g, "regdomain", &domain, 4);
		}
		if (domain == 0 || domain > DOMAIN_WORLD_WIDE)
			domain = DOMAIN_GLOBAL;

		if (radio_data[i].radio_band == RADIO_BAND_5GH || radio_data[i].radio_band == RADIO_BAND_5GL) {
			// 5G available channel
			for (k = 0; k < 25; k++) {
				if (ch_5g_dfs[domain][k] == 0)
					break;
				buffer[k] = ch_5g_dfs[domain][k];
				len++;
			}

			PLATFORM_PRINTF_DETAIL("Available channel number for %s is %d\n", ifname, len);
			int i = 0;
			for (i = 0; i < len; i++) {
				PLATFORM_PRINTF_DETAIL("Available channel %d: %d\n", i , buffer[i]);
			}
		} else if (radio_data[i].radio_band == RADIO_BAND_2G) {
			// 2G available channel
			buffer[0] = 1;
			buffer[1] = 2;
			buffer[2] = 3;
			buffer[3] = 4;
			buffer[4] = 5;
			buffer[5] = 6;
			buffer[6] = 7;
			buffer[7] = 8;
			buffer[8] = 9;
			buffer[9] = 10;
			buffer[10] = 11;
			buffer[11] = 12;
			buffer[12] = 13;
			len = 13;

			PLATFORM_PRINTF_DETAIL("Available channel number for %s is %d\n", ifname, len);
			int i = 0;
			for (i = 0; i < len; i++) {
				PLATFORM_PRINTF_DETAIL("Available channel %d: %d\n", i , buffer[i]);
			}
		}
			
		return (INT8U)len;

	} else {
		/*** Initialize socket ***/
		sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
		if (sock < 0) {
			err = errno;
			PLATFORM_PRINTF_ERROR("[%s %d]: Can't create socket for ioctl. %s(%d)", __FUNCTION__, __LINE__, ifname, err);
			return len;
		}

		/*** Initialize struct iwreq ***/
		memset(&wrq, 0, sizeof(wrq));
		strncpy(wrq.ifr_name, ifname, IFNAMSIZ - 1);

		/*** give parameter and buffer ***/
		wrq.u.data.pointer = (caddr_t)buffer;
		wrq.u.data.length  = buffer_size;

		/*** ioctl ***/
		if (ioctl(sock, SIOCMAP_GET_AVAILABLE_CHANNELS, &wrq) < 0) {
			err = errno;
			PLATFORM_PRINTF_ERROR("[%s %d]: %s ioctl Error.(%d)", __FUNCTION__, __LINE__, ifname, err);
			len = 0;
			close(sock);
			return 0;
		} else {
			err = 0;
			len = buffer[0];

			memmove(buffer, buffer + 1, len);

			PLATFORM_PRINTF_DETAIL("Available channel number for %s is %d\n", ifname, len);
			int i = 0;
			for (i = 0; i < len; i++) {
				PLATFORM_PRINTF_DETAIL("Available channel %d: %d\n", i, buffer[i]);
			}
		}

		close(sock);
		return (INT8U)len;
	}
}

int rtk_get_is_ax_support(char *interface)
{
	UNUSED(interface);
	return 0;
}

INT8U rtk_linux_trigger_wps(char *interface)
{
	char          command[50];
	sprintf(command, "wscd -sig_pbc %s", interface);

	system(command);
	PLATFORM_PRINTF_DEBUG("[WSC] WSC started with command (%s)\n", command);

	return 0;
}

INT8U rtk_linux_need_hapd()
{
	return 0;
}

int rtk_linux_get_link_metric_ioctl(char *ifname, INT8U *mac, struct linkMetrics *link_metrics)
{
	link_metrics->tx_packet_ok         = (INT32U)_read_sta_info_proc(ifname, mac, "tx_pkts:");
	link_metrics->tx_packet_errors     = (INT32U)_read_sta_info_proc(ifname, mac, "tx_fail:");
	link_metrics->tx_phy_rate          = (INT32U)_read_sta_info_proc(ifname, mac, "current_tx_rate:");
	link_metrics->rx_packet_ok         = (INT32U)_read_sta_info_proc(ifname, mac, "rx_pkts:");
	link_metrics->tx_max_xput          = link_metrics->tx_phy_rate;
	link_metrics->tx_link_availability = 100;
	link_metrics->rx_packet_errors     = 0;

	return 0;
}

INT8U *rtk_linux_get_clients_rssi(char *interface_name, INT8U *clients_address, INT8U clients_nr, INT8U allow_sta_mode)
{
	UNUSED(interface_name);
	UNUSED(clients_address);
	UNUSED(clients_nr);
	UNUSED(allow_sta_mode);
	return 0;
}

int rtk_linux_map_general(char *interface_name, INT8U operation_type, INT8U *buffer, INT16U buffer_len, INT16U content_len)
{
	UNUSED(interface_name);
	UNUSED(operation_type);
	UNUSED(buffer);
	UNUSED(buffer_len);
	UNUSED(content_len);
	return -1;
}

#ifdef RTK_MULTI_AP_R3
void rtk_linux_issue_cce_ie_indication(char *interface_name, INT8U include_cce_ie)
{
	UNUSED(interface_name);
	UNUSED(include_cce_ie);
}
#endif // RTK_MULTI_AP_R3

#ifdef RTK_MULTI_AP_R4
void rtk_linux_get_spatial_reuse_config_response(struct SpatialReuseConfigResponseTLV **response, INT8U *response_nr)
{
	UNUSED(response);
	UNUSED(response_nr);
}

INT8U rtk_linux_get_spatial_reuse_report_tlv(INT8U *buf, char *interface_name)
{
	UNUSED(buf);
	UNUSED(interface_name);
	return 0;
}

void rtk_linux_start_reconfig(char *interface_name)
{
	UNUSED(interface_name);
}

void rtk_linux_apply_spatial_reuse_config(struct SpatialReuseRequestTLV *request, char *interface_name)
{
	UNUSED(request);
	UNUSED(interface_name);
}
#endif // RTK_MULTI_AP_R4