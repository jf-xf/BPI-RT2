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
#include <features.h>

#include "platform_interfaces.h"
#include "platform_interfaces_rtk_priv.h"
#include "platform_ioctl.h"
#include "platform_os_priv.h"
#include "multi_ap_tlvs.h"
#include "al_wsc.h"
#include "packet_tools.h"
#include "al_utils.h"

#if defined(CONFIG_RTK_PON_XDSL) || !defined(__UCLIBC__)
#include "utils.h"
#endif

#include "global_settings.h"
#include "../../../../multi-ap/customized/map_vendor_specific_data.h"

#define SIOCDEVPRIVATEAXEXT 0x89f2 //SIOCDEVPRIVATE+2

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

enum MAP_GENERAL_OPERATION {
	MAP_RESERVED,
	MAP_GET_GENERAL,
	MAP_SET_GENERAL,
	MAP_SEND_5G_BSS_MACS,
	MAP_GET_LINK_METRICS,
	MAP_GET_STA_INFO,
	MAP_GET_CLIENTS_RSSI
};

#if !defined(_DRIVER_COMMON_STRUCTURE_) && !defined(CONFIG_RTK_PON_XDSL)
#define STA_INFO_FLAG_ASOC			0x04 //same to mib.h

typedef struct _bss_info {
    unsigned char state;
    unsigned char channel;
    unsigned char txRate;
    unsigned char bssid[6];
    unsigned char rssi, sq;	// RSSI  and signal strength
    unsigned char ssid[SSID_LEN+1];
} bss_info;
#endif

/* WLAN sta info structure */
typedef struct old_sta_info {
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
    unsigned char  multiap_profile;
} OLD_WLAN_STA_INFO_T, *OLD_WLAN_STA_INFO_Tp;

struct map_sta_info {
	INT8U  mac_addr[6];
	INT64U link_time;
	INT8U  multiap_profile;
};

struct _spatialReuseData {
	INT8U                                 spatial_reuse_response_nr;
	struct SpatialReuseConfigResponseTLV *spatial_reuse_response;
} spatial_reuse_data;

int _get_wifi_connected_devices_private(char *interface_name, struct interfaceInfo *m);

int rtk_get_is_ax_support(char *interface)
{
	if (!interface) {
		printf("Invalid interface name in rtk_get_is_ax_support, please check the error! \n");
		return 0;
	}

	char filename[64]  = { 0 };
	int  is_ax_support = 0;

	snprintf(filename, sizeof(filename), "/proc/net/rtl8852ae/%s", interface);
	if (-1 != access(filename, F_OK))
		is_ax_support = 1;

	snprintf(filename, sizeof(filename), "/proc/net/rtk_wifi6/%s", interface);
	if (-1 != access(filename, F_OK))
		is_ax_support = 2;

	return is_ax_support;
}

extern char *trimInterfaceNameVID(char *name, INT16U *vid, INT8U *port);
int _getWlBssInfo(char *interface, bss_info *pInfo)
{
	int          skfd = 0;
	struct iwreq wrq;
	struct ifreq rq;
	int cmd_id, is_ax_support = 0;
	char *base_name           = NULL;
	INT16U vid                = 0;

	base_name = trimInterfaceNameVID(interface, &vid, NULL);
	if (NULL != base_name) {
		PLATFORM_FREE(base_name);
		if(0 < vid)
			return 0;
	}

	/*** Initialize socket ***/
	skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (-1 == skfd) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] socket('%s') returned with errno=%d (%s) while opening a DGRAM socket\n", interface, errno, strerror(errno));
		return 0;
	}

	is_ax_support = rtk_get_is_ax_support(interface);

	/*** Initialize struct iwreq ***/
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, interface, IFNAMSIZ - 1);
	if (is_ax_support) {
		wrq.u.data.flags = SIOCGIWRTLGETBSSINFO;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = SIOCGIWRTLGETBSSINFO;
	}

	/*** Fill data ***/
	wrq.u.data.pointer = (caddr_t)pInfo;
	wrq.u.data.length  = sizeof(bss_info);

	memcpy(&rq, &wrq, sizeof(struct iwreq));
	if (ioctl(skfd, cmd_id, &rq) < 0) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] _getWlBssInfo: %s Error to ioctl.(%d)\n", interface, errno);
		close(skfd);
		return 0;
	}
	close(skfd);

	return 0;
}

int _getWlStaInfo(char *interface, INT8U *buffer, INT16U buffer_size)
{
	int          skfd = 0;
	struct iwreq wrq;
	struct ifreq rq;
	int cmd_id, is_ax_support;
	char *base_name    = NULL;
	INT16U vid         = 0;

	base_name = trimInterfaceNameVID(interface, &vid, NULL);
	if (NULL != base_name) {
		PLATFORM_FREE(base_name);
		if(0 < vid)
			return 0;
	}

	/*** Initialize socket ***/
	skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if ( -1 == skfd ) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] socket('%s') returned with errno=%d (%s) while opening a DGRAM socket\n", interface, errno, strerror(errno));
		return 0;
	}

	is_ax_support = rtk_get_is_ax_support(interface);

	memset(buffer, 0, buffer_size);

	/*** Initialize struct iwreq ***/
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, interface, IFNAMSIZ - 1);
	wrq.u.data.pointer = (caddr_t)buffer;
	wrq.u.data.length  = buffer_size;
	if (is_ax_support) {
		wrq.u.data.flags = SIOCGIWRTLSTAINFO;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = SIOCGIWRTLSTAINFO;
	}

	memcpy(&rq, &wrq, sizeof(struct iwreq));
	if (ioctl(skfd, cmd_id, &rq) < 0) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] _getWlStaInfo: %s Error to ioctl.(%d)\n", interface, errno);
		close(skfd);
		return 0;
	}
	close(skfd);

	return 0;
}

#ifndef CONFIG_RTK_PON_XDSL
static void _get_wifi_connected_devices(char *interface_name, struct interfaceInfo *m)
{
	// This is a copy of wirelessClientList() in boa/src/fmwlan.c:6752 to get all associated stations info

	INT8U *          buffer;
	OLD_WLAN_STA_INFO_Tp pInfo;
	INT8U            i;
	INT16U buffer_size = sizeof(OLD_WLAN_STA_INFO_T) * (map_max_sta_num + 1);

	m->neighbor_mac_addresses_nr = 0;
	buffer                         = calloc(1, buffer_size);
	if (buffer == 0) {
		PLATFORM_PRINTF_ERROR("[RTL] Allocate buffer failed!\n");
		return;
	}

	if (_getWlStaInfo(interface_name, buffer, buffer_size) < 0) {
		PLATFORM_PRINTF_ERROR("[RTL] Read wlan sta info failed!\n");
	}
	for (i = 1; i <= map_max_sta_num; i++) {
		pInfo = (OLD_WLAN_STA_INFO_Tp)&buffer[i * sizeof(OLD_WLAN_STA_INFO_T)];
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC)) {
			// calcuate the number of associated devices
			m->neighbor_mac_addresses_nr += 1;
		}
	}

	if (m->neighbor_mac_addresses_nr <= 0) {
		PLATFORM_PRINTF_FULL("[RTL] [%s] NO station is associated on this interface.\n",
		                     interface_name);
		PLATFORM_FREE(buffer);
		return;
	}

	m->neighbor_list = (struct neighborInfo *)realloc(m->neighbor_list, sizeof(struct neighborInfo) * m->neighbor_mac_addresses_nr + 1);

	int counter = 0;
	for (i = 1; i <= map_max_sta_num; i++) {
		pInfo = (OLD_WLAN_STA_INFO_Tp)&buffer[i * sizeof(OLD_WLAN_STA_INFO_T)];
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC)) {
			m->neighbor_list[counter].link_time = pInfo->link_time;
			memcpy(m->neighbor_list[counter].mac_address, pInfo->addr, 6);
			m->neighbor_list[counter].multiap_profile = pInfo->multiap_profile;
			PLATFORM_PRINTF_FULL("[RTL] [%s] Associated STA MAC \t%02x:%02x:%02x:%02x:%02x:%02x\tLink Time (s):\t %llu\tProfile:\t %u\n",
			                     interface_name,
			                     m->neighbor_list[counter].mac_address[0], m->neighbor_list[counter].mac_address[1], m->neighbor_list[counter].mac_address[2],
			                     m->neighbor_list[counter].mac_address[3], m->neighbor_list[counter].mac_address[4], m->neighbor_list[counter].mac_address[5],
			                     m->neighbor_list[counter].link_time, m->neighbor_list[counter].multiap_profile);
			counter++;
		}
	}
	PLATFORM_FREE(buffer);
}
#endif

#if defined(_DRIVER_COMMON_STRUCTURE_) || defined(CONFIG_RTK_PON_XDSL)
static void _get_wifi_connected_devices_new(char *interface_name, struct interfaceInfo *m)
{
	// This is a copy of wirelessClientList() in boa/src/fmwlan.c:6752 to get all associated stations info

	INT8U *          buffer;
	sta_info_2_web  *pInfo;
	INT8U            i;
	INT16U buffer_size = sizeof(sta_info_2_web) * (map_max_sta_num + 1);

	m->neighbor_mac_addresses_nr = 0;
	buffer                         = calloc(1, buffer_size);
	if (buffer == 0) {
		PLATFORM_PRINTF_ERROR("[RTL] Allocate buffer failed!\n");
		return;
	}

	if (_getWlStaInfo(interface_name, buffer, buffer_size) < 0) {
		PLATFORM_PRINTF_ERROR("[RTL] Read wlan sta info failed!\n");
	}
	for (i = 1; i <= map_max_sta_num; i++) {
		pInfo = (sta_info_2_web *)&buffer[i * sizeof(sta_info_2_web)];
		if (pInfo->aid && (pInfo->flags & STA_INFO_FLAG_ASOC)) {
			// calcuate the number of associated devices
			m->neighbor_mac_addresses_nr += 1;
		}
	}

	if (m->neighbor_mac_addresses_nr <= 0) {
		PLATFORM_PRINTF_FULL("[RTL] [%s] NO station is associated on this interface.\n",
		                     interface_name);
		PLATFORM_FREE(buffer);
		return;
	}

	m->neighbor_list = (struct neighborInfo *)realloc(m->neighbor_list, sizeof(struct neighborInfo) * m->neighbor_mac_addresses_nr + 1);

	int counter = 0;
	for (i = 1; i <= map_max_sta_num; i++) {
		pInfo = (sta_info_2_web *)&buffer[i * sizeof(sta_info_2_web)];
		if (pInfo->aid && (pInfo->flags & STA_INFO_FLAG_ASOC)) {
			m->neighbor_list[counter].link_time = pInfo->link_time;
			memcpy(m->neighbor_list[counter].mac_address, pInfo->addr, 6);
			m->neighbor_list[counter].multiap_profile = pInfo->multiap_profile;
			PLATFORM_PRINTF_FULL("[RTL] [%s] Associated STA MAC \t%02x:%02x:%02x:%02x:%02x:%02x\tLink Time (s):\t %llu\tProfile:\t %u\n",
			                     interface_name,
			                     m->neighbor_list[counter].mac_address[0], m->neighbor_list[counter].mac_address[1], m->neighbor_list[counter].mac_address[2],
			                     m->neighbor_list[counter].mac_address[3], m->neighbor_list[counter].mac_address[4], m->neighbor_list[counter].mac_address[5],
			                     m->neighbor_list[counter].link_time, m->neighbor_list[counter].multiap_profile);
			counter++;
		}
	}
	PLATFORM_FREE(buffer);
}
#endif

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
		strlcpy(ifr.ifr_name, map_bridge_name, IFNAMSIZ);
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
		strlcpy(ifr.ifr_name, map_bridge_name, IFNAMSIZ);
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
					INT8U interface_al_mac[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
					PLATFORM_GET_MAC(port_intf_name, interface_al_mac);
					if (memcmp(interface_al_mac, f->mac_addr, 6) == 0) {
						continue;
					}
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

////////////////////////////////////////////////////////////////////////////////
// Internal API: to be used by other platform-specific files (functions
// declaration is found in "./platform_interfaces_rtk_linux_priv.h")
////////////////////////////////////////////////////////////////////////////////
char *_get_root_interface_name(char *interface_name)
{
	char *root_interface = NULL;

	if (PLATFORM_STRSTR(interface_name, map_radio_name_5gh)) {
		root_interface = map_radio_name_5gh;
	} else if (PLATFORM_STRSTR(interface_name, map_radio_name_5gl)) {
		root_interface = map_radio_name_5gl;
	} else if (PLATFORM_STRSTR(interface_name, map_radio_name_24g)) {
		root_interface = map_radio_name_24g;
	} else {
		PLATFORM_PRINTF_DEBUG("Failed to get the root interface name for %s \n", interface_name);
	}
	return root_interface;
}

INT8U rtk_linux_get_interface_info(char *interface_name, struct interfaceInfo *m, INT8U need_wps)
{
	FILE *fd;

	INT8U  mib_val_int8u    = 0;
	INT32U mib_val_int32u   = 0;
	int    interface_status = 0;

	char buf[1024] = { 0 }, newline[1024] = { 0 };

	interface_status = PLATFORM_IS_INTERFACE_UP(interface_name);
	if (interface_status) {
		m->power_state = INTERFACE_POWER_STATE_ON;
	}

	if (strstr(interface_name, "wlan") != NULL) {

		// Retrieve BSS type information
		//
		INT8U bss_type = rtk_linux_get_bss_type(interface_name);
		m->interface_type_data.ieee80211.network_type = bss_type;

		// Check and correct func_off status
		//
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

		// Retrieve Network band information
		//
		int wlan_idx = interface_name[4] - '0';
		if (0 != PLATFORM_GET_MIB(_get_root_interface_name(interface_name), "band", &mib_val_int8u, 1)) {
			PLATFORM_PRINTF_WARNING("[RTK] [%s] cannot get band for interface %s\n", __FUNCTION__, interface_name);
		}
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

		// Retrieve Operation Mode
		//
		if (PLATFORM_STRSTR(interface_name, map_vxd_prefix)) {
			m->interface_type_data.ieee80211.role = IEEE80211_ROLE_NON_AP_NON_PCP_STA;
		} else {
			m->interface_type_data.ieee80211.role = IEEE80211_ROLE_AP;
		}
		PLATFORM_PRINTF_FULL("[RTL] Interface %s mode: %02x\n", interface_name, m->interface_type_data.ieee80211.role);

		// Retrieve Authentication Type and Encryption Mode
		//
		m->is_secured                                        = 0;
		m->interface_type_data.ieee80211.authentication_mode = IEEE80211_AUTH_MODE_OPEN;
		PLATFORM_GET_MIB(interface_name, "authtype", &mib_val_int32u, 4);
		if (0 != PLATFORM_GET_MIB(interface_name, "encmode", &mib_val_int8u, 1)) {
			PLATFORM_PRINTF_WARNING("[RTK] [%s] cannot get encmode for interface %s\n", __FUNCTION__, interface_name);
		}
		// PLATFORM_PRINTF_INFO("%s auth type %08x encmode %02x\n", interface_name, mib_val_int32u, mib_val_int8u);
		// mib "encmode" in driver: 0x0:none | 0x2:wpa | 0x4:wpa2 | 0x10:wpa3
		if (0x16 & mib_val_int8u) {
			if(customize_features & CUSTOMIZE_WSC_INFO){
				m->is_secured                                        = 1;

				m->interface_type_data.ieee80211.authentication_mode = 0;
				if(mib_val_int8u & 2){
					m->interface_type_data.ieee80211.authentication_mode |= IEEE80211_AUTH_MODE_WPAPSK;
				}
				if(mib_val_int8u & 4){
					m->interface_type_data.ieee80211.authentication_mode |= IEEE80211_AUTH_MODE_WPA2PSK;
				}
				if(mib_val_int8u & 16){
					m->interface_type_data.ieee80211.authentication_mode |= IEEE80211_AUTH_MODE_WPA3PSK;
				}

				//TODO: set encryption_mode as reality as interface configured
				m->interface_type_data.ieee80211.encryption_mode     = IEEE80211_ENCRYPTION_MODE_AES;
			}else{

				m->is_secured                                        = 1;
				m->interface_type_data.ieee80211.authentication_mode = IEEE80211_AUTH_MODE_WPA2PSK | IEEE80211_AUTH_MODE_WPAPSK;
				m->interface_type_data.ieee80211.encryption_mode     = IEEE80211_ENCRYPTION_MODE_TKIP | IEEE80211_ENCRYPTION_MODE_AES;
			}
		} else {
			if((customize_features & SUPPORT_OPEN_ENCRYPT) && 0 == mib_val_int8u){
				m->is_secured                                    =1;
				m->interface_type_data.ieee80211.authentication_mode = IEEE80211_AUTH_MODE_OPEN;
				m->interface_type_data.ieee80211.encryption_mode     = IEEE80211_ENCRYPTION_MODE_TKIP | IEEE80211_ENCRYPTION_MODE_AES;
			}
			else
				PLATFORM_PRINTF_DETAIL("%s is insecure: auth type %08x encmode %02x\n", interface_name, mib_val_int32u, mib_val_int8u);
		}

		int is_ax_support;
		is_ax_support = rtk_get_is_ax_support(interface_name);
		if (is_ax_support) {
			if (0 != PLATFORM_GET_MIB(interface_name, "use40M", &mib_val_int8u, 1)) {
				PLATFORM_PRINTF_ERROR("Can not get bandwidth for interface %s\n", interface_name);
			} else {
				m->interface_type_data.ieee80211.ap_channel_band = 10 * (2 << mib_val_int8u);
				// TO DO: this will not be valid for 80 + 80, need to change.
			}
		} else {
			if (0 != PLATFORM_GET_MIB(interface_name, "use40M", &mib_val_int32u, 4)) {
				PLATFORM_PRINTF_ERROR("Can not get bandwidth for interface %s\n", interface_name);
			} else {
				m->interface_type_data.ieee80211.ap_channel_band = 10 * (2 << mib_val_int8u);
				// TO DO: this will not be valid for 80 + 80, need to change.
			}
		}

		m->interface_type_data.ieee80211.ap_channel_center_frequency_index_1 = get_center_frequency(interface_name);
		m->interface_type_data.ieee80211.ap_channel_center_frequency_index_2 = 0;

		// Retrieve Push Button information
		//
		m->push_button_on_going = 2;
		m->wsc_result           = 0;

		if (NULL == strstr(m->name, map_vap_prefix) && need_wps) {
			if (!(strstr(interface_name, map_vxd_prefix))) {
				snprintf(buf, sizeof(buf), "%s wps_get_status -i %s", "/bin/hostapd_cli", interface_name);
				PLATFORM_PRINTF_DEBUG("Start %s\n", buf);
				fd = popen(buf, "re");
			} else
				fd = fopen("/var/wpas_wps_stas", "re");

			if (fd) {
				if (fgets(newline, 1024, fd) != NULL) {
					sscanf(newline, "%*[^:]:%1023s", buf);
					//printf("sw_commit %d\n", version);
					printf("%s\n", buf);
				}

				if (!strcmp(buf, "Active"))
					m->push_button_on_going = 1;
				else
					m->push_button_on_going = 0;

				m->wsc_result = 0;

				if (!strcmp(buf, "Disabled")) {
					if (fgets(newline, 1024, fd) != NULL) {
						sscanf(newline, "%*[^:]:%1023s", buf);
						//printf("sw_commit %d\n", version);
						printf("%s\n", buf);
					}

					if (!strcmp(buf, "Success"))
						m->wsc_result = 1;
				}

				if (!(strstr(interface_name, map_vxd_prefix)))
					pclose(fd);
				else
					fclose(fd);
			}
		}

		// Retrieve BSSID and SSID
		//
		bss_info pInfo;
		memset(&pInfo, 0x00, sizeof(bss_info));
		if (!eth_only)
			_getWlBssInfo(interface_name, &pInfo);
		if (pInfo.state == 0 ||
			(pInfo.state != 4 && m->interface_type_data.ieee80211.role == IEEE80211_ROLE_NON_AP_NON_PCP_STA))
			PLATFORM_MEMSET(pInfo.bssid, 0x00, 6);
		PLATFORM_STRCPY(m->interface_type_data.ieee80211.ssid, (char *)pInfo.ssid);
		PLATFORM_MEMCPY(m->interface_type_data.ieee80211.bssid, pInfo.bssid, 6);
		PLATFORM_PRINTF_FULL("[RTL] [%s] state: %d SSID:%s BSSID:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x \n",
		                     interface_name,
		                     pInfo.state,
		                     pInfo.ssid,
		                     m->interface_type_data.ieee80211.bssid[0],
		                     m->interface_type_data.ieee80211.bssid[1],
		                     m->interface_type_data.ieee80211.bssid[2],
		                     m->interface_type_data.ieee80211.bssid[3],
		                     m->interface_type_data.ieee80211.bssid[4],
		                     m->interface_type_data.ieee80211.bssid[5]);

		// Retrieve list of connected devices
		//
		if (INTERFACE_POWER_STATE_OFF == m->power_state) {
			m->neighbor_mac_addresses_nr = 0;
		} else {
			char *base_name = trimInterfaceNameVID(interface_name, NULL, NULL);
#if defined(CONFIG_RTK_PON_XDSL) // SD9 Platform, use new struct only for both wifi5 and wifi6
			_get_wifi_connected_devices_new(base_name, m);
#else
			//use SIOCMAP_GENERAL_IOCTL get sta_info first, if fail, use common ioctl SIOCGIWRTLSTAINFO
			if(_get_wifi_connected_devices_private(base_name, m) <= 0) {
				if (rtk_get_is_ax_support(base_name)) {
#if defined(_DRIVER_COMMON_STRUCTURE_)
					_get_wifi_connected_devices_new(base_name, m);
#else
					_get_wifi_connected_devices(base_name, m);
#endif
				} else {
					_get_wifi_connected_devices(base_name, m);
				}
			}
#endif
			PLATFORM_FREE(base_name);
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
#define PROC_NB_REPORT_FMT "/proc/net/rtl8852ae/%s/nb_report"
#define NEW_PROC_NB_REPORT_FMT "/proc/net/rtk_wifi6/%s/nb_report"
#define BSS_TM_REQ_PARAMS_FMT "valid_int=50 pref=%d disassoc_imminent=%d disassoc_timer=%d abridged=%d mbo=%d:5:0"
#define NB_RPT "neighbor=%02x:%02x:%02x:%02x:%02x:%02x,%u,%u,%u,%u,%02X%02X%02X"
INT8U rtk_linux_send_btm_request(char *ifname, INT8U *macaddr, INT8U mode, INT8U *target_bssid, INT8U target_opclass, INT8U target_channel, INT16U disassoc_timer, INT8U reason_code)
{

	int  is_ax_support = 0;
	char echo_cmd[128] = { 0 }, hapd_cmd[256] = { 0 };
	int  disassoc_imminent = 0, pref = 0, abridged = 0, disassoc_time_bi = 0;
	INT8U subelement_id = 0, subelement_len = 0, subelement_content = 0;

	disassoc_imminent = 1; //(mode >> 6) & 0x01;
	abridged          = 1; //(mode >> 5) & 0x01;
	disassoc_time_bi  = (disassoc_timer / 100); //TU transfers to Beacon Interval
	UNUSED(mode);

	if (macaddr == NULL || ifname == NULL) {
		printf("information is not enough\n");
		return -1;
	}
	else if (target_bssid == NULL) { //don't need to set up NB list
		pref              = 0;
		snprintf(hapd_cmd, sizeof(hapd_cmd),
		         "hostapd_cli -i %s bss_tm_req %02x:%02x:%02x:%02x:%02x:%02x " BSS_TM_REQ_PARAMS_FMT " ",
		         ifname,
		         macaddr[0], macaddr[1],
		         macaddr[2], macaddr[3],
		         macaddr[4], macaddr[5],
		         pref, disassoc_imminent,
		         disassoc_time_bi, abridged, reason_code);
		system(hapd_cmd);
		PLATFORM_PRINTF_INFO("[PLATFORM] cmd: %s\n", hapd_cmd);
	}
	else {
		is_ax_support   = rtk_get_is_ax_support(ifname);
		pref            = 1; //(mode >> 7) & 0x01;
		if (is_ax_support) {
			char neighbor_report_path[64] = {0};
			if (2 == is_ax_support) {
				snprintf(neighbor_report_path, sizeof(neighbor_report_path), NEW_PROC_NB_REPORT_FMT, ifname);
			} else {
				snprintf(neighbor_report_path, sizeof(neighbor_report_path), PROC_NB_REPORT_FMT, ifname);
			}
			snprintf(echo_cmd, sizeof(echo_cmd),
			         "echo add %02x:%02x:%02x:%02x:%02x:%02x 3 %d %d 9 255 > %s",
			         target_bssid[0], target_bssid[1],
			         target_bssid[2], target_bssid[3],
			         target_bssid[4], target_bssid[5],
			         target_opclass, target_channel, neighbor_report_path);
			system(echo_cmd);
			PLATFORM_PRINTF_INFO("[PLATFORM] cmd: %s\n", echo_cmd);

			snprintf(echo_cmd, sizeof(echo_cmd),
			         "echo tag %02x:%02x:%02x:%02x:%02x:%02x 1 > %s",
			         target_bssid[0], target_bssid[1],
			         target_bssid[2], target_bssid[3],
			         target_bssid[4], target_bssid[5],
			         neighbor_report_path);
			system(echo_cmd);
			PLATFORM_PRINTF_INFO("[PLATFORM] cmd: %s\n", echo_cmd);

			snprintf(hapd_cmd, sizeof(hapd_cmd),
			         "hostapd_cli -i %s bss_tm_req %02x:%02x:%02x:%02x:%02x:%02x " BSS_TM_REQ_PARAMS_FMT "",
			         ifname,
			         macaddr[0], macaddr[1],
			         macaddr[2], macaddr[3],
			         macaddr[4], macaddr[5],
			         pref, disassoc_imminent, disassoc_time_bi, abridged, reason_code);
			system(hapd_cmd);
			PLATFORM_PRINTF_INFO("[PLATFORM] cmd: %s\n", hapd_cmd);

		} else {
			INT8U phy_type = 0, bss_info = 0;
			phy_type = 7; // DOT11_PHY_TYPE_HT
			bss_info = 3; // reachable

			/* Candidate Preference subelement field format */
			subelement_id      = 0x03;
			subelement_len     = 0x01;
			subelement_content = 0xff; // 0x01~0xff:the least to the most preferred candidate

			snprintf(hapd_cmd, sizeof(hapd_cmd),
			         "hostapd_cli -i %s bss_tm_req %02x:%02x:%02x:%02x:%02x:%02x " BSS_TM_REQ_PARAMS_FMT " "NB_RPT" ",
			         ifname,
			         macaddr[0], macaddr[1],
			         macaddr[2], macaddr[3],
			         macaddr[4], macaddr[5],
			         pref, disassoc_imminent,
			         disassoc_time_bi, abridged, reason_code,
			         target_bssid[0], target_bssid[1],
			         target_bssid[2], target_bssid[3],
			         target_bssid[4], target_bssid[5],
			         bss_info, target_opclass,
			         target_channel, phy_type,
			         subelement_id, subelement_len, subelement_content);
			system(hapd_cmd);
			PLATFORM_PRINTF_INFO("[PLATFORM] cmd: %s\n", hapd_cmd);
		}
	}
	return 0;
}

INT8U rtk_linux_bss_ioctl(char *interface_name, INT8U type)
{

	int          s;
	struct iwreq iwr;
	struct ifreq rq;
	char         tmp[1];
	int          cmd_id, is_ax_support = 0;

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

	is_ax_support = rtk_get_is_ax_support(interface_name);

	PLATFORM_PRINTF_DEBUG("[PLATFORM] Set interface type\n");
	strncpy(iwr.ifr_name, interface_name, IFNAMSIZ - 1);
	iwr.u.data.pointer = tmp;
	iwr.u.data.length  = 1;
	if (is_ax_support) {
		iwr.u.data.flags = RTL8192CD_IOCTL_UPDATE_BSS;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = RTL8192CD_IOCTL_UPDATE_BSS;
	}

	memcpy(&rq, &iwr, sizeof(struct iwreq));
	if (ioctl(s, cmd_id, &rq) < 0) {
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
	int          cmd_id, is_ax_support = 0;
	struct iwreq wrq;
	struct ifreq rq;
	char *       mib_str_buf = PLATFORM_ZALLOC(512);
	if (!mib_str_buf) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] Cannot allocate buffer for %s\n", __FUNCTION__);
		return 0;
	}
	PLATFORM_SNPRINTF(mib_str_buf, 512, "%s=%s", item_str, value_str);
	PLATFORM_PRINTF_INFO("Set mib for %s: %s\n", interface_name, mib_str_buf);

	/*** Initialize socket ***/
	s = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (-1 == s) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] socket('%s') returned with errno=%d (%s) while opening a DGRAM socket\n", interface_name, errno, strerror(errno));
		PLATFORM_FREE(mib_str_buf);
		return 0;
	}
	is_ax_support = rtk_get_is_ax_support(interface_name);
	/*** Initialize struct iwreq ***/
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, interface_name, IFNAMSIZ - 1);

	if (is_ax_support) {
		wrq.u.data.flags = RTL8192CD_IOCTL_SET_MIB;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = RTL8192CD_IOCTL_SET_MIB;
	}

	/*** fill mib string ***/
	wrq.u.data.pointer = (caddr_t)mib_str_buf;
	wrq.u.data.length  = strlen(mib_str_buf) + 1;

	memcpy(&rq, &wrq, sizeof(struct iwreq));
	/*** set mib ***/

	if (ioctl(s, cmd_id, &rq) < 0) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] set_mib: %s(%s). Error to ioctl.(%d)\n", mib_str_buf, interface_name, errno);
		close(s);
		PLATFORM_FREE(mib_str_buf);
		return 0;
	}

	close(s);
	PLATFORM_FREE(mib_str_buf);
	return 1;
}

#define RTL8192CD_IOCTL_GET_MIB 0x89f2
INT8U rtk_linux_get_mib(char *interfacename, char *mibname, void *result, int size)
{
	int          skfd;
	int          cmd_id, is_ax_support = 0;
	struct iwreq wrq;
	struct ifreq rq;
	char *tmp = (char *)malloc(64);

	if (!tmp) {
		PLATFORM_PRINTF_ERROR("[%s][%d] Fail to allocate buffer with malloc \n", __FUNCTION__, __LINE__);
		return 0;
	}

	memset(tmp, 0, 64);

	skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (skfd < 0) {
		PLATFORM_PRINTF_ERROR("ioctl [RTL8192CD_IOCTL_GET_MIB] socket error\n");
		if (tmp) {
			free(tmp);
			tmp = NULL;
		}
		return 1;
	}
	is_ax_support = rtk_get_is_ax_support(interfacename);
	memset(&wrq, 0, sizeof(wrq));
	strlcpy(wrq.ifr_name, interfacename, sizeof(wrq.ifr_name));
	strlcpy(tmp, mibname, 64);

	wrq.u.data.pointer = tmp;

	if (is_ax_support) {
		wrq.u.data.flags  = RTL8192CD_IOCTL_GET_MIB;
		wrq.u.data.length = strlen(tmp) + 1;
		cmd_id            = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id            = RTL8192CD_IOCTL_GET_MIB;
		wrq.u.data.length = strlen(tmp);
	}

	memcpy(&rq, &wrq, sizeof(struct iwreq));
	/* Do the request */
	if (ioctl(skfd, cmd_id, &rq) < 0) {
		PLATFORM_PRINTF_ERROR("ioctl [RTL8192CD_IOCTL_GET_MIB] for mib %s on %s failed!\n", mibname, interfacename);
		close(skfd);
		if (tmp) {
			free(tmp);
			tmp = NULL;
		}
		return 1;
	}

	if (size)
		memcpy(result, tmp, size);
	else {
		tmp[SSID_LEN] = '\0';
		strcpy(result, (char *)tmp);
	}

	if (tmp) {
		free(tmp);
		tmp = NULL;
	}
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

	if ( size > (int)sizeof(tmp) ) {
		PLATFORM_PRINTF_WARNING("[%s] insufficient buffer size!\n",__FUNCTION__);
		return 1;
	}

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
		return 1;

	i = radio_idx;
	j = if_idx;

	if (0 == PLATFORM_STRCMP(mibname, "use40M")) {
		tmp_int = radio_data[i].channel_bandwidth;
		memcpy(tmp, &tmp_int, sizeof(tmp_int));
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
		memcpy(tmp, &tmp_int, sizeof(tmp_int));
	} else if (0 == PLATFORM_STRCMP(mibname, "regdomain")) {
		tmp_32u = hw_reg_domain;
		memcpy(tmp, &tmp_32u, sizeof(tmp_32u));
	} else if (0 == PLATFORM_STRCMP(mibname, "opmode")) {
		if (PLATFORM_STRSTR(interfacename, map_vxd_prefix))
			tmp_32u = WIFI_STATION_STATE;
		else
			tmp_32u = WIFI_AP_STATE;
		memcpy(tmp, &tmp_32u, sizeof(tmp_32u));
	} else if (0 == PLATFORM_STRCMP(mibname, "func_off")) {
		tmp_32u = !radio_data[i].interface_mib[j].is_enabled;
		memcpy(tmp, &tmp_32u, sizeof(tmp_32u));
	} else if (0 == PLATFORM_STRCMP(mibname, "authtype")) {
		tmp_32u = radio_data[i].interface_mib[j].authentication_type;
		memcpy(tmp, &tmp_32u, sizeof(tmp_32u));
	} else if (0 == PLATFORM_STRCMP(mibname, "multiap_bss_type")) {
		tmp_8u = radio_data[i].interface_mib[j].network_type;
		memcpy(tmp, &tmp_8u, sizeof(tmp_8u));
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
		memcpy(tmp, &tmp_8u, sizeof(tmp_8u));
	} else if (0 == PLATFORM_STRCMP(mibname, "max_tx_power")) {
		tmp_8u = 20;
		memcpy(tmp, &tmp_8u, sizeof(tmp_8u));
	} else if (0 == PLATFORM_STRCMP(mibname, "band")) {
		if (radio_data[i].radio_band == RADIO_BAND_2G)
			tmp_8u = (WIRELESS_11B | WIRELESS_11G | WIRELESS_11N | WIRELESS_11AX);
		else
			tmp_8u = (WIRELESS_11A | WIRELESS_11N | WIRELESS_11AC | WIRELESS_11AX);
		memcpy(tmp, &tmp_8u, sizeof(tmp_8u));
	} else if (0 == PLATFORM_STRCMP(mibname, "encmode")) {
		tmp_8u = radio_data[i].interface_mib[j].encrypt_type;
		memcpy(tmp, &tmp_8u, sizeof(tmp_8u));
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
	struct ifreq  rq;
	unsigned char tmp[30];
	int           cmd_id, is_ax_support = 0;

	skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (skfd < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("ioctl [RTL8192CD_IOCTL_SEND_DISASSOC] socket error\n");
		return err;
	}

	is_ax_support = rtk_get_is_ax_support(interfacename);

	memset(&wrq, 0, sizeof(wrq));
	strlcpy(wrq.ifr_name, interfacename, sizeof(wrq.ifr_name));
	if (sta_mac != NULL) {
		memcpy(tmp, sta_mac, 6);
	} else
		PLATFORM_PRINTF_ERROR("ioctl [RTL8192CD_IOCTL_SEND_DISASSOC] sta_mac is NULL\n");

	wrq.u.data.pointer = (caddr_t)tmp;
	wrq.u.data.length  = 6; //MACADDRLEN
	if (is_ax_support) {
		wrq.u.data.flags = RTL8192CD_IOCTL_SEND_DISASSOC;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = RTL8192CD_IOCTL_SEND_DISASSOC;
	}

	memcpy(&rq, &wrq, sizeof(struct iwreq));
	/* Do the request */
	if (ioctl(skfd, cmd_id, &rq) < 0) {
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
	int is_ax_support = 0;
	int err           = 0;

	is_ax_support = rtk_get_is_ax_support(interfacename);

	if (is_ax_support) {
		unsigned char mac_addr_str[18];
		char          tmp_cmd[128] = { 0 };

		if (sta_mac == NULL || interfacename == NULL) {
			return -1;
		}

		memcpy(mac_addr_str, sta_mac, 6);
		//snprintf(mac_addr_str, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
		//sta_mac[0], sta_mac[1], sta_mac[2], sta_mac[3], sta_mac[4], sta_mac[5]);
		snprintf(tmp_cmd, sizeof(tmp_cmd),
			"hostapd_cli -i %s deauthenticate %02x:%02x:%02x:%02x:%02x:%02x",
			interfacename,
			sta_mac[0], sta_mac[1],
			sta_mac[2], sta_mac[3],
			sta_mac[4], sta_mac[5]);

		system(tmp_cmd);

		PLATFORM_PRINTF_DEBUG("[PLATFORM] cmd: %s\n", tmp_cmd);
		err = 0;
	} else {
		int          skfd;
		int          err;
		struct iwreq wrq;
		struct ifreq rq;
		char         hex_str[3];

		unsigned char tmp[30] = { 0 };

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
		memcpy(&rq, &wrq, sizeof(struct iwreq));
		/* Do the request */
		if (ioctl(skfd, RTL8192CD_IOCTL_DEL_STA, &rq) < 0) {
			err = errno;
			PLATFORM_PRINTF_ERROR("ioctl [RTL8192CD_IOCTL_DEL_STA] on %s failed!\n", interfacename);
			close(skfd);
			return err;
		}
		err = 0;

		close(skfd);
	}

	return err;
}

#define RTL8192CD_IOCTL_ASSOC_CONTROL 0x8B38
INT8U rtk_linux_assoc_control(char *interfacename, INT8U *sta_mac, INT16U period, INT8U control)
{
	struct assoc_control_para assoc_ctrl_para;
	int                       skfd;
	int                       err;
	struct iwreq              wrq;
	struct ifreq              rq;
	char                      tmp[30];
	int                       cmd_id, is_ax_support = 0;

	skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (skfd < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("ioctl [RTL8192CD_IOCTL_ASSOC_CONTROL] socket error\n");
		return err;
	}

	is_ax_support = rtk_get_is_ax_support(interfacename);

	memset(&assoc_ctrl_para, 0, sizeof(struct assoc_control_para));
	memcpy(assoc_ctrl_para.addr, sta_mac, 6);
	assoc_ctrl_para.period  = period;
	assoc_ctrl_para.control = control;
	memcpy(tmp, &assoc_ctrl_para, sizeof(struct assoc_control_para));

	memset(&wrq, 0, sizeof(wrq));
	strlcpy(wrq.ifr_name, interfacename, sizeof(wrq.ifr_name));

	wrq.u.data.pointer = (caddr_t)tmp;

	if (is_ax_support) {
		wrq.u.data.flags = RTL8192CD_IOCTL_ASSOC_CONTROL;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = RTL8192CD_IOCTL_ASSOC_CONTROL;
	}

	wrq.u.data.length = sizeof(struct assoc_control_para);
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	/* Do the request */
	if (ioctl(skfd, cmd_id, &rq) < 0) {
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

INT8U rtk_linux_get_ap_capability_info(char *interface_name, INT8U tlv_type, INT8U *result)
{
	struct iwreq wrq;
	struct ifreq rq;
	int          skfd;
	int          err;
	//unsigned char         tmp[2048] = {0};
	//unsigned char *buf;
	unsigned int len = 0;
	char type[8] = {0};
	int          cmd_id, is_ax_support = 0;

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
		case 4:
			strcpy(type, "(WIFI6)");
			break;
	}

	PLATFORM_PRINTF_DEBUG("[PLATFORM] Preparing to get AP Capability%s\n", type);
	PLATFORM_PRINTF_DEBUG("[PLATFORM]   - Interface name = %s\n", interface_name);

	skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (skfd < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("ioctl [RTL8192CD_IOCTL_ASSOC_CONTROL] socket error\n");
		return err;
	}
	is_ax_support = rtk_get_is_ax_support(interface_name);

	*result = tlv_type;

	PLATFORM_MEMSET(&wrq, 0, sizeof(wrq));
	//PLATFORM_PRINTF_DEBUG("[PLATFORM] Set interface type\n");
	strncpy(wrq.ifr_name, interface_name, IFNAMSIZ - 1);
	wrq.u.data.pointer = (caddr_t)result;

	if (is_ax_support) {
		wrq.u.data.flags = RTL8192CD_IOCTL_GET_AP_CAPABILITY;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = RTL8192CD_IOCTL_GET_AP_CAPABILITY;
	}

	wrq.u.data.length = 1;
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	if (ioctl(skfd, cmd_id, &rq) < 0) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] ioctl('%s',RTL8192CD_IOCTL_GET_AP_CAPABILITY)%s returned with errno=%d (%s)\n", interface_name, type, errno, strerror(errno));
		close(skfd);
		return 0;
	}
	memcpy(&wrq, &rq, sizeof(struct iwreq));
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

	close(skfd);
	return len;
}

#define RTL8192CD_IOCTL_GET_CLIENT_CAPABILITY 0x8B52

INT8U rtk_linux_get_client_capability_info(char *interface_name, INT8U *macaddr, INT8U *results)
{
	int           skfd;
	int           err;
	struct iwreq  wrq;
	struct ifreq  rq;
	unsigned char len = 0;
	int           cmd_id, is_ax_support = 0;

	PLATFORM_PRINTF_DEBUG("[PLATFORM] Preparing to get Client Capability\n");
	PLATFORM_PRINTF_DEBUG("[PLATFORM]   - Interface name = %s\n", interface_name);

	skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (skfd < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("ioctl [RTL8192CD_IOCTL_ASSOC_CONTROL] socket error\n");
		return err;
	}
	is_ax_support = rtk_get_is_ax_support(interface_name);

	PLATFORM_MEMCPY(results, macaddr, MACADDRLEN);
	PLATFORM_MEMSET(&wrq, 0, sizeof(wrq));
	//PLATFORM_PRINTF_DEBUG("[PLATFORM] Set interface type\n");
	strncpy(wrq.ifr_name, interface_name, IFNAMSIZ - 1);
	wrq.u.data.pointer = (caddr_t)results;

	if (is_ax_support) {
		wrq.u.data.flags = RTL8192CD_IOCTL_GET_CLIENT_CAPABILITY;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = RTL8192CD_IOCTL_GET_CLIENT_CAPABILITY;
	}

	wrq.u.data.length = 6;
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	if (ioctl(skfd, cmd_id, &rq) < 0) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] ioctl('%s',RTL8192CD_IOCTL_GET_CLIENT_CAPABILITY) returned with errno=%d (%s)\n", interface_name, errno, strerror(errno));
		close(skfd);
		return 0;
	}

	PLATFORM_PRINTF_DEBUG("[PLATFORM] Client Capability retrieved!\n");
	memcpy(&wrq, &rq, sizeof(struct iwreq));
	len = wrq.u.data.length;

	close(skfd);

	return len;
}

#define SIOC11KBEACONREQ 0x8BD2

int rtk_linux_issue_beacon_measurement(char *ifname, unsigned char *macaddr,
                                       struct BeaconMeasurementRequest *beacon_req)
{
	char hapd_cmd[1024]     = { 0 };
	char bcn_req[256]       = { 0 };
	char chl_rport_cmd[32]  = { 0 };
	char rpt_dtail_cmd[256] = { 0 };
	unsigned char sub_element     = 0;
	unsigned char sub_element_len = 0;
	unsigned char is_detail_rpt = 0, is_ch_rpt = 0, len = 0;
	int  i             = 0;
	int  j             = 0;
	int  p             = 0;
	int  ssid_len      = 0;

	if (macaddr == NULL || ifname == NULL || beacon_req == NULL) {
		PLATFORM_PRINTF_DEBUG("[PLATFORM] Information is not enough\n");
		return -1;
	}

	sprintf(bcn_req,
	        "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
	        beacon_req->op_class, beacon_req->channel,
	        (INT8U)(beacon_req->random_interval >> 8),
	        (INT8U)(beacon_req->random_interval & 0xff),
	        (INT8U)(beacon_req->measure_duration >> 8),
	        (INT8U)(beacon_req->measure_duration & 0xff),
	        beacon_req->mode,
	        beacon_req->bssid[0], beacon_req->bssid[1], beacon_req->bssid[2],
	        beacon_req->bssid[3], beacon_req->bssid[4], beacon_req->bssid[5]);

	p += 26;

	/* construct SSID subelement */
	if (beacon_req->ssid[0] != 0) {
		sprintf(&(bcn_req[p]), "%02x", 0x00);
		p += 2;

		ssid_len = strlen(beacon_req->ssid);
		sprintf(&(bcn_req[p]), "%02x", ssid_len);
		p += 2;

		for (i = 0; i < ssid_len; i++) {
			sprintf(&(bcn_req[p]), "%02x", beacon_req->ssid[i]);
			p += 2;
		}
	}

	/* construct report detail subelement */
	sub_element     = 0x02;
	sub_element_len = 0x01;
	if (beacon_req->report_detail != 0) {
		sprintf(rpt_dtail_cmd, "%02x%02x%02x",
			sub_element, sub_element_len, beacon_req->report_detail);
		is_detail_rpt = 1;
	}

	/* construct channel report subelement */
	sub_element     = 0x33;
	sub_element_len = 0x00;
	if (beacon_req->channel == 255) {
		for (i = 0; i < MAX_AP_CHANNEL_REPORT; i++) {
			if (beacon_req->ap_channel_report[i].len) {
				sprintf(chl_rport_cmd, "%02x%02x%02x",
					sub_element, beacon_req->ap_channel_report[i].len, beacon_req->ap_channel_report[i].op_class);
				sub_element_len += 6;
				for (j = 0; j < beacon_req->ap_channel_report[i].len-1; j++) {
					sprintf(&(chl_rport_cmd[sub_element_len]), "%02x", beacon_req->ap_channel_report[i].channel[j]);
					sub_element_len += 2;
				}
			}
		}
		is_ch_rpt = 1;
	}

	sprintf(hapd_cmd,
	        "hostapd_cli -i %s req_beacon %02x:%02x:%02x:%02x:%02x:%02x %s",
	        ifname, macaddr[0], macaddr[1], macaddr[2],
	        macaddr[3], macaddr[4], macaddr[5], bcn_req);

	len = PLATFORM_STRLEN(hapd_cmd);
	if(is_detail_rpt) {
		PLATFORM_SNPRINTF(hapd_cmd + len, 1024 - len, "%s", rpt_dtail_cmd);
		len = PLATFORM_STRLEN(hapd_cmd);
	}
	if(is_ch_rpt) {
		PLATFORM_SNPRINTF(hapd_cmd + len, 1024 - len, "%s", chl_rport_cmd);
	}
	system(hapd_cmd);
	PLATFORM_PRINTF_DEBUG("[PLATFORM] cmd: %s\n", hapd_cmd);
	return 0;
}

INT32U rtk_get_prefer_bssid_interval(void)
{
	char line[50] = {0};
	FILE *fp;
	INT32U prefer_bssid_interval = 120;

	fp = fopen("/var/prefer_bssid_interval", "re");
	if (fp) {
		if (fgets(line, sizeof(line), fp)) {
			sscanf(line, "%d", &prefer_bssid_interval);
			if (prefer_bssid_interval == 0)
				prefer_bssid_interval = 120;
			printf("%s %d prefer_bssid_interval: %d\n", __FUNCTION__,__LINE__,prefer_bssid_interval);
		}

		fclose(fp);

		unlink("/var/prefer_bssid_interval");
	}

	return prefer_bssid_interval;
}

void rtk_reset_prefer_bssid(void)
{
	char wpacli_cmd[256] = {0}, line[50] = {0}, tmp_ifname[20] = {0};
	FILE *fp;

	fp = fopen("/var/agent_prefer_bssid_set", "re");
	if (fp) {
		if (fgets(line, sizeof(line), fp)) {
			sscanf(line, "%19s", tmp_ifname);
			memset(wpacli_cmd, 0, sizeof(wpacli_cmd));
			snprintf(wpacli_cmd, sizeof(wpacli_cmd), "wpa_cli -i %s bssid 0 00:00:00:00:00:00",
				tmp_ifname);
			system(wpacli_cmd);
			printf("%s %d wpacli_cmd: %s\n", __FUNCTION__,__LINE__,wpacli_cmd);
		}

		fclose(fp);
	}

	unlink("/var/agent_prefer_bssid_set");
}

int rtk_linux_do_backhaul_steering(char *ifname, unsigned char *backhaul_bss, unsigned char *target_bssid, unsigned char op_class, unsigned char channel)
{
	int tmp_count = 0, is_find = 0, is_bssid_find = 0;
	char wpacli_cmd[256] = {0}, newline[500] = {0};
	char tmp_bssid[20] = {0}, target_str_bssid[20] = {0}, tmp_frequency[20] = {0}, tmp_signal[20] = {0}, tmp_flags[50] = {0},  tmp_ssid[100] = {0};
	FILE *fp=NULL;
	struct interfaceInfo *m;

	if (op_class || channel) {
		PLATFORM_PRINTF_DEBUG("[%s %d] op_class:%d, channel:%d\n",__FUNCTION__,__LINE__,op_class,channel);
	}


	PLATFORM_PRINTF_DEBUG("[%s %d]ax is support\n",__FUNCTION__,__LINE__);

	if(backhaul_bss == NULL || ifname == NULL || target_bssid == NULL)
	{
		printf("information is not enough\n");
		return -1;
	}

	char *base_name = trimInterfaceNameVID(ifname, NULL, NULL);

	m = PLATFORM_GET_1905_INTERFACE_INFO(ifname);

	snprintf(target_str_bssid, sizeof(target_str_bssid), "%02x:%02x:%02x:%02x:%02x:%02x",target_bssid[0], target_bssid[1], target_bssid[2],
	target_bssid[3], target_bssid[4], target_bssid[5]);

	while(tmp_count < 8)
	{
		is_bssid_find = 0;
		PLATFORM_MEMSET(wpacli_cmd, 0, sizeof(wpacli_cmd));
		snprintf(wpacli_cmd, sizeof(wpacli_cmd), "wpa_cli -i %s scan_results", base_name);
		fp = popen(wpacli_cmd, "r");
		if(fp)
		{
			fgets(newline, 500, fp);
			while(fgets(newline, 500, fp) != NULL){
				PLATFORM_MEMSET(tmp_bssid, 0, sizeof(tmp_bssid));
				PLATFORM_MEMSET(tmp_frequency, 0, sizeof(tmp_frequency));
				PLATFORM_MEMSET(tmp_signal, 0, sizeof(tmp_signal));
				PLATFORM_MEMSET(tmp_flags, 0, sizeof(tmp_flags));
				PLATFORM_MEMSET(tmp_ssid, 0, sizeof(tmp_ssid));
				sscanf(newline, "%19s %19s %19s %49s %99s", tmp_bssid, tmp_frequency, tmp_signal, tmp_flags, tmp_ssid);
				PLATFORM_PRINTF_DEBUG("[%s %d]%s %s %s %s %s\n", __FUNCTION__,__LINE__,tmp_bssid, tmp_frequency, tmp_signal, tmp_flags, tmp_ssid);

				if((strlen(tmp_bssid) == strlen(target_str_bssid)) && (strcmp(tmp_bssid, target_str_bssid) == 0))
				{
					is_bssid_find = 1;
					if(strlen(tmp_ssid) != 0)
					{
						printf("%s %d find bssid and ssid\n", __FUNCTION__,__LINE__);
						is_find = 1;

						if(m)
						{
							PLATFORM_PRINTF_DEBUG("[%s %d]ssid %s %s len %d %d\n", __FUNCTION__,__LINE__,m->interface_type_data.ieee80211.ssid, tmp_ssid, strlen(tmp_ssid),strlen(m->interface_type_data.ieee80211.ssid));
							if(strlen(tmp_ssid) != strlen(m->interface_type_data.ieee80211.ssid) || strcmp(tmp_ssid, m->interface_type_data.ieee80211.ssid) != 0)
							{
								pclose(fp);
								PLATFORM_FREE_1905_INTERFACE_INFO(m);
								PLATFORM_FREE(base_name);
								printf("%s %d ssid not the same\n", __FUNCTION__,__LINE__);
								return 0;
							}
						}

						break;
					}
				}
			}

			pclose(fp);

			if(is_find)
				break;
			else
				tmp_count++;
		}
		else
			tmp_count++;

		PLATFORM_MEMSET(wpacli_cmd, 0, sizeof(wpacli_cmd));
		snprintf(wpacli_cmd, sizeof(wpacli_cmd), "wpa_cli -i %s scan TYPE=ONLY", base_name);
		system(wpacli_cmd);
		PLATFORM_PRINTF_DEBUG("[%s %d]wpacli_cmd: %s\n", __FUNCTION__,__LINE__,wpacli_cmd);

		sleep(1);
	}

	if(is_bssid_find == 0)
	{
		PLATFORM_FREE_1905_INTERFACE_INFO(m);
		PLATFORM_FREE(base_name);
		printf("%s %d not find bssid\n", __FUNCTION__,__LINE__);
		return 0;
	}

	tmp_count = 0;

	while(tmp_count < 3)
	{
		PLATFORM_MEMSET(wpacli_cmd, 0, sizeof(wpacli_cmd));
		snprintf(wpacli_cmd, sizeof(wpacli_cmd), "wpa_cli -i %s roam %02x:%02x:%02x:%02x:%02x:%02x",
			base_name,
			target_bssid[0], target_bssid[1],
			target_bssid[2], target_bssid[3],
			target_bssid[4], target_bssid[5]);
		system(wpacli_cmd);
		PLATFORM_PRINTF_DEBUG("[%s %d]wpacli_cmd: %s\n", __FUNCTION__,__LINE__,wpacli_cmd);
		tmp_count++;
	}

	PLATFORM_MEMSET(wpacli_cmd, 0, sizeof(wpacli_cmd));
	snprintf(wpacli_cmd, sizeof(wpacli_cmd), "wpa_cli -i %s bssid 0 %02x:%02x:%02x:%02x:%02x:%02x",
		base_name,
		target_bssid[0], target_bssid[1],
		target_bssid[2], target_bssid[3],
		target_bssid[4], target_bssid[5]);
	system(wpacli_cmd);
	PLATFORM_PRINTF_DEBUG("[%s %d]wpacli_cmd: %s\n", __FUNCTION__,__LINE__,wpacli_cmd);

	PLATFORM_MEMSET(wpacli_cmd, 0, sizeof(wpacli_cmd));
	snprintf(wpacli_cmd, sizeof(wpacli_cmd), "echo %s > /var/agent_prefer_bssid_set", base_name);
	system(wpacli_cmd);

	if (NULL != m) {
		PLATFORM_FREE_1905_INTERFACE_INFO(m);
	}

	PLATFORM_FREE(base_name);
	return 2;
}

#define SIOCMAP_CHANNELSCAN 0x8B58
#define SIOCOPCLASS_CHANNELSCAN_REQ 0x8B5A

int rtk_linux_do_channel_scan(char *interface_name, unsigned char channel_nr, unsigned char *channels)
{
	int           sock;
	struct iwreq  wrq;
	struct ifreq  rq;
	int           err;
	int           len = 0;
	unsigned char tempbuf[2048];
	int           cmd_id, is_ax_support = 0;

	/*** Initialize socket ***/
	sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (sock < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("[%s %d]: Can't create socket for ioctl. %s(%d)", __FUNCTION__, __LINE__, interface_name, err);
		return err;
	}
	is_ax_support = rtk_get_is_ax_support(interface_name);

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

	if (is_ax_support) {
		wrq.u.data.flags = SIOCOPCLASS_CHANNELSCAN_REQ;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = SIOCMAP_CHANNELSCAN;
	}
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	/*** ioctl ***/
	if (ioctl(sock, cmd_id, &rq) < 0) {
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
	struct ifreq rq;
	int          err;
	int          len = 0;
	int          cmd_id, is_ax_support = 0;

	/*** Initialize socket ***/
	sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (sock < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("[%s %d]: Can't create socket for ioctl. %s(%d)", __FUNCTION__, __LINE__, ifname, err);
		return len;
	}
	is_ax_support = rtk_get_is_ax_support(ifname);

	*results = tlv_type;

	/*** Initialize struct iwreq ***/
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, ifname, IFNAMSIZ - 1);

	/*** give parameter and buffer ***/
	wrq.u.data.pointer = (caddr_t)results;

	if (is_ax_support) {
		wrq.u.data.flags = SIOCMAP_GETAPMETRIC;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = SIOCMAP_GETAPMETRIC;
	}

	wrq.u.data.length = 1;
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	/*** ioctl ***/
	if (ioctl(sock, cmd_id, &rq) < 0) {
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
	struct ifreq rq;
	int          err;
	int          len = 0;
	//unsigned char tempbuf[2048];
	int cmd_id, is_ax_support = 0;

	/*** Initialize socket ***/
	sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (sock < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("[%s %d]: Can't create socket for ioctl. %s(%d)", __FUNCTION__, __LINE__, ifname, err);
		return 0;
	}

	is_ax_support = rtk_get_is_ax_support(ifname);

	results[0] = tlv_type;
	memcpy(&results[1], macaddr, MACADDRLEN);

	/*** Initialize struct iwreq ***/
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, ifname, IFNAMSIZ - 1);

	/*** give parameter and buffer ***/
	wrq.u.data.pointer = (caddr_t)results;

	if (is_ax_support) {
		wrq.u.data.flags = SIOCMAP_GETASSOCSTAMETRIC;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = SIOCMAP_GETASSOCSTAMETRIC;
	}

	wrq.u.data.length = 7;
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	/*** ioctl ***/
	if (ioctl(sock, cmd_id, &rq) < 0) {
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
	struct ifreq  rq;
	int           err;
	int           len = 0;
	unsigned char tempbuf[2048];
	int           cmd_id, is_ax_support = 0;

	/*** Initialize socket ***/
	sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (sock < 0) {
		err = errno;
		PLATFORM_PRINTF_ERROR("[%s %d]: Can't create socket for ioctl. %s(%d)", __FUNCTION__, __LINE__, ifname, err);
		return err;
	}

	is_ax_support = rtk_get_is_ax_support(ifname);

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
	if (is_ax_support) {
		wrq.u.data.flags = SIOCMAP_GETUNASSOCSTAMETRIC;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = SIOCMAP_GETUNASSOCSTAMETRIC;
	}

	wrq.u.data.length = len;
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	/*** ioctl ***/
	if (ioctl(sock, cmd_id, &rq) < 0) {
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
	struct ifreq rq;
	char         tmp[5] = { 0 };
	int          cmd_id, is_ax_support = 0;

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

	is_ax_support = rtk_get_is_ax_support(interface_name);

	memset(&wrq, 0, sizeof(wrq));

	PLATFORM_PRINTF_DETAIL("[PLATFORM] Set interface type\n");
	strncpy(wrq.ifr_name, interface_name, IFNAMSIZ - 1);
	wrq.u.data.pointer = tmp;

	if (is_ax_support) {
		wrq.u.data.flags = SIOCMAP_UPDATEPOLICY;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = SIOCMAP_UPDATEPOLICY;
	}

	wrq.u.data.length = 3;
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	if (ioctl(s, cmd_id, &rq) < 0) {
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
	struct ifreq rq;
	char         tmp[5] = { 0 };
	int          cmd_id, is_ax_support = 0;

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

	is_ax_support = rtk_get_is_ax_support(interface_name);

	memset(&wrq, 0, sizeof(wrq));

	PLATFORM_PRINTF_DETAIL("[PLATFORM] Set interface type\n");
	strncpy(wrq.ifr_name, interface_name, IFNAMSIZ - 1);
	wrq.u.data.pointer = tmp;

	if (is_ax_support) {
		wrq.u.data.flags = RTL8192CD_IOCTL_AGENT_STEERING;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = RTL8192CD_IOCTL_AGENT_STEERING;
	}

	wrq.u.data.length = 3;
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	if (ioctl(s, cmd_id, &rq) < 0) {
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
	struct ifreq rq;
	char         tmp[1] = { 0 };
	int          cmd_id, is_ax_support = 0;

	tmp[0] = tx_max_power;

	PLATFORM_PRINTF_DEBUG("[PLATFORM] Preparing to set max tx power:\n");
	PLATFORM_PRINTF_DEBUG("[PLATFORM]   - Interface name = %s\n", interface_name);
	PLATFORM_PRINTF_DEBUG("[PLATFORM]   - Max tx power = %02x\n", tx_max_power);

	s = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (-1 == s) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] socket('%s') returned with errno=%d (%s) while opening a DGRAM socket\n", interface_name, errno, strerror(errno));
	return 0;
}

	is_ax_support = rtk_get_is_ax_support(interface_name);

	memset(&wrq, 0, sizeof(wrq));

	if (is_ax_support) {
		wrq.u.data.flags = SIOCMAP_SET_TXMAXPOWER;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = SIOCMAP_SET_TXMAXPOWER;
	}

	PLATFORM_PRINTF_DEBUG("[PLATFORM] Set interface type\n");
	strncpy(wrq.ifr_name, interface_name, IFNAMSIZ - 1);
	wrq.u.data.pointer = tmp;
	wrq.u.data.length  = 1;
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	if (ioctl(s, cmd_id, &rq) < 0) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] ioctl('%s',SIOCMAP_SET_TXMAXPOWER) returned with errno=%d (%s) while updating\n", interface_name, errno, strerror(errno));
		close(s);
		return 0;
	}

	PLATFORM_PRINTF_DEBUG("[PLATFORM] Max tx power updated!\n");

	close(s);
	return 1;
}

#define SIOCMAP_CAC 0x8B72

int rtk_linux_do_cac(char *interface_name, unsigned char channel_nr, unsigned char *channels, unsigned char type)
{
	int           sock;
	struct iwreq  wrq;
	struct ifreq  rq;
	int           cmd_id, is_ax_support = 0;
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
	is_ax_support = rtk_get_is_ax_support(interface_name);
	if (is_ax_support) {
		wrq.u.data.flags = SIOCMAP_CAC;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = SIOCMAP_CAC;
	}
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	if (ioctl(sock, cmd_id, &rq) < 0) {
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

	if (!interface_name) {
		PLATFORM_PRINTF_ERROR("[%s][%d] Null interface_name is passed in ! \n", __FUNCTION__, __LINE__);
		return 0;
	}

	if (PLATFORM_GET_VALID_INTERFACE_NAME(interface_name, VALID_ETH_INTERFACES_IN_BRIDGE)
		|| (controller_interface && PLATFORM_STRSTR(interface_name, controller_interface))) {
		return (MULTI_AP_FRONTHAUL_BSS | MULTI_AP_BACKHAUL_BSS);
	}

	if (interface_name != NULL && PLATFORM_IS_INTERFACE_VALID(interface_name)) {
		if (0 != PLATFORM_GET_MIB(interface_name, "multiap_bss_type", &bss_type, 1)) {
			PLATFORM_PRINTF_ERROR("[%s][%d] Failed to get mib value for bss_type on %s ! \n", __FUNCTION__, __LINE__, interface_name);
			return 0;
		}
	}

	PLATFORM_PRINTF_FULL("[RTL] %s bss_type %02x\n", interface_name, bss_type);

	return bss_type;
}

OP_CLASS *rtk_linux_get_op_class(int *op_classes_len)
{
	INT32U    domain             = DOMAIN_GLOBAL;
	int       reg_op_classes_len = 0;
	OP_CLASS *reg_op_classes     = NULL;
	int       i                  = 0;
	INT8U     index_buf[32]      = { 0 };

	if (hw_country_str != NULL) {
		for (i = 0; i < (int)(sizeof(COUNTRY_IE_ARRAY) / sizeof(COUNTRY_IE)); i++) {
			if (0 == PLATFORM_STRCMP(COUNTRY_IE_ARRAY[i].country_str, hw_country_str)) {
				domain = COUNTRY_IE_ARRAY[i].region_domain;
				PLATFORM_PRINTF_FULL("hw_country_str = %s Domain = %u\n", hw_country_str, domain);
				break;
			}
		}
	} else {
		if (PLATFORM_IS_INTERFACE_UP(map_radio_name_5gh)) {
			PLATFORM_GET_MIB(map_radio_name_5gh, "regdomain", &domain, 4);
		} else if (PLATFORM_IS_INTERFACE_UP(map_radio_name_5gl)) {
			PLATFORM_GET_MIB(map_radio_name_5gl, "regdomain", &domain, 4);
		} else {
			PLATFORM_GET_MIB(map_radio_name_24g, "regdomain", &domain, 4);
		}
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
		int index                       = index_buf[i];
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
	struct ifreq rq;
	int          err;
	int          len = 0;
	int          cmd_id, is_ax_support = 0;
	int          i, j, k;
	char         root_name[32] = {0};
	char         interface_name[64]= {0};
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

		is_ax_support = rtk_get_is_ax_support(ifname);

		/*** Initialize struct iwreq ***/
		memset(&wrq, 0, sizeof(wrq));
		strncpy(wrq.ifr_name, ifname, IFNAMSIZ - 1);

		/*** give parameter and buffer ***/
		wrq.u.data.pointer = (caddr_t)buffer;
		wrq.u.data.length  = buffer_size;
		if (is_ax_support) {
			wrq.u.data.flags = SIOCMAP_GET_AVAILABLE_CHANNELS;
			cmd_id           = SIOCDEVPRIVATEAXEXT;
		} else {
			cmd_id = SIOCMAP_GET_AVAILABLE_CHANNELS;
		}
		memcpy(&rq, &wrq, sizeof(struct iwreq));
		/*** ioctl ***/
		if (ioctl(sock, cmd_id, &rq) < 0) {
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

INT8U rtk_linux_trigger_wps(char *interface)
{
	char command[50];
	if (!(strstr(interface, map_vxd_prefix))) {
		sprintf(command, "/bin/hostapd_cli wps_pbc -i %s", interface);
	} else {
		sprintf(command, "/bin/wpa_cli wps_pbc -i %s multi_ap=1", interface);
	}

	system(command);
	PLATFORM_PRINTF_DEBUG("[WSC] WSC started with command (%s)\n", command);

	return 0;
}

INT8U rtk_linux_need_hapd()
{
	return 1;
}

void rtk_linux_issue_cce_ie_indication(char *interface_name, INT8U include_cce_ie)
{
	char cmd[128];
	PLATFORM_MEMSET(cmd, 0, sizeof(cmd));
	sprintf(cmd, "hostapd_cli -i %s set dpp_configurator_connectivity %d", interface_name, include_cce_ie);
	PLATFORM_PRINTF_DEBUG("%s \n", cmd);
	system(cmd);
}

void rtk_linux_start_reconfig(char *interface_name)
{
	char cmd[128];
	PLATFORM_MEMSET(cmd, 0, sizeof(cmd));
	sprintf(cmd, "wpa_cli -i %s dpp_reconfig 0 multi_ap=1", interface_name);
	PLATFORM_PRINTF_DEBUG("%s \n", cmd);
	system(cmd);
}

int rtk_linux_get_link_metric_ioctl(char *ifname, INT8U *mac, struct linkMetrics *link_metrics)
{
	INT8U        *pt          = NULL;
	INT16U       tlv_len      = 0;
	int          result       = 0;
	INT8U        buffer[80]   = { 0 };

	if (mac == NULL)
		return 1;

	tlv_len = 6;
	PLATFORM_MEMSET(buffer, 0, sizeof(buffer));
	PLATFORM_MEMCPY(&buffer[3], &tlv_len, 2);
	PLATFORM_MEMCPY(&buffer[5], mac, 6);

	result  = rtk_linux_map_general(ifname, MAP_GET_LINK_METRICS, buffer, sizeof(buffer), tlv_len + 2);
	if (result <= 0) {
		return result;
	}

	PLATFORM_MEMCPY(&tlv_len, &buffer, 2);
	if (tlv_len) {
		pt = &(buffer[18]);
		_EnB(&pt, &link_metrics->tx_packet_ok, 4);
		_EnB(&pt, &link_metrics->tx_packet_errors, 4);
		_EnB(&pt, &link_metrics->tx_max_xput, 2);
		_EnB(&pt, &link_metrics->tx_phy_rate, 2);
		_EnB(&pt, &link_metrics->tx_link_availability, 2);
		_EnB(&pt, &link_metrics->rx_packet_ok, 4);
		_EnB(&pt, &link_metrics->rx_packet_errors, 4);
		link_metrics->rx_rssi = *pt;
		return 1;
	} else {
		return 0;
	}

}

// For detailed documentation, please refer to bookstack, under general ioctl.
#ifdef _RTK_LINUX_
INT8U * rtk_linux_get_clients_rssi(char *interface_name, INT8U *clients_address, INT8U clients_nr, INT8U allow_sta_mode)
{
	int ret;
	INT16U content_size = 3 + 2 + MACADDRLEN * clients_nr;
	INT8U buffer[512] = { 0 };
	INT8U *result;
	INT8U obtained_sta_nr;

	INT8U *p = buffer + 3;
	PLATFORM_MEMCPY(p, &allow_sta_mode, 1);
	p++;
	PLATFORM_MEMCPY(p, &clients_nr, 1);
	p++;
	PLATFORM_MEMCPY(p, clients_address, clients_nr * MACADDRLEN);

	ret = rtk_linux_map_general(interface_name, MAP_GET_CLIENTS_RSSI, buffer, sizeof(buffer), content_size);

	if (ret < 0) {
		PLATFORM_PRINTF_WARNING("Failed to obtain rssi for clients \n");
		return NULL;
	}

	obtained_sta_nr = buffer[0];
	if (obtained_sta_nr != clients_nr) {
		PLATFORM_PRINTF_DEBUG("Failed to query rssi of some stations, please check errors. Expected:[%d], Obtained:[%d] \n", clients_nr, obtained_sta_nr);
		return NULL;
	}

	result = (INT8U *)PLATFORM_MALLOC(obtained_sta_nr);
	PLATFORM_MEMCPY(result, buffer + 1, obtained_sta_nr);
	return result;
}
#endif

/* Starting from map r3, all additional ioctl operations use this function.
 * operation type: 0x00 reserved, 0x01 get, 0x02 set, others undefined.
 * For get:
 *		Before:
 * 			pointer = [operation_type, reserved_len, reserved_len, TLV_TYPE, TLV_LEN, TLV_LEN, 0...];
 * 			length = receiving buffet length, large enough;
 * 		After:
 * 			pointer = [TLV_TYPE, TLV_LEN, TLV_LEN, payload...]
 * 			length = returned buffer length, equals to length of payload + 3.
 *
 * For set:
 * 		Before:
 * 			pointer = [operation_type, reserved_len, reserved_len, TLV_TYPE, TLV_LEN, TLV_LEN, payload...]
 * 			length = created buffer length, large enough;
 * 		After:
 * 			pointer = unchanged
 * 			length = processed buffer length.
 *
 */
#define SIOCMAP_GENERAL_IOCTL		0x8B83
int rtk_linux_map_general(char *interface_name, INT8U operation_type, INT8U *buffer, INT16U buffer_len, INT16U content_len)
{
	struct iwreq wrq;
	struct ifreq rq;
	int          skfd;
	int          err;
	INT8U        len = 0;
	int          cmd_id, is_ax_support = 0;

	char  *base_name = NULL;
	INT16U vid       = 0;

	if (buffer_len < 3 || buffer_len < content_len) {
		PLATFORM_PRINTF_ERROR("Not enough space for map general ioctl:content_len[%02x]!\n", content_len);
		return -1;
	}

	if (!interface_name) {
		PLATFORM_PRINTF_ERROR("Interface_name : null\n");
		return -1;
	}
	base_name = trimInterfaceNameVID(interface_name, &vid, NULL);
	if (NULL != base_name) {
		PLATFORM_FREE(base_name);
		if (0 < vid)
			return -1;
	}

	INT16U *total_content_length = (INT16U *)(buffer + 1);

	*buffer           = operation_type;
	*total_content_length = content_len;  // Driver uses memcpy or INT16U pointer to get length.

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
		err = -errno;
		PLATFORM_PRINTF_ERROR("ioctl [SIOCMAP_GET_GENERAL] socket error\n");
		return err;
	}

	PLATFORM_MEMSET(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, interface_name, IFNAMSIZ - 1);

	wrq.u.data.pointer = (caddr_t)buffer;

	is_ax_support = rtk_get_is_ax_support(interface_name);
	if (is_ax_support) {
		wrq.u.data.flags = SIOCMAP_GENERAL_IOCTL; // this will be extracted as subcmd in function __rtw_ioctl_private() in g6 driver.
		cmd_id           = SIOCDEVPRIVATEAXEXT;   // this actually goes to ioctl private cmd case.
	} else {
		cmd_id = SIOCMAP_GENERAL_IOCTL; // this will go to wifi5 driver, 8192fe
	}

	wrq.u.data.length = buffer_len;
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	if (ioctl(skfd, cmd_id, &rq) < 0) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] ioctl('%s',rtk_linux_map_general) returned with errno=%d (%s)\n", interface_name, errno, strerror(errno));
		close(skfd);
		return -errno;
	}
	memcpy(&wrq, &rq, sizeof(struct iwreq));
	len = wrq.u.data.length;

	close(skfd);
	return len;
}

int _rtk_get_sta_info(char *ifname, struct map_sta_info **sta_info, INT16U *sta_nr)
{
	int    result = 0;
	INT8U *buffer = NULL;
	size_t buffer_size = 2000;  // up to 133 connected stations
	int    i      = 0;
	INT8U *pt     = NULL;

	*sta_nr = 0;
	buffer = PLATFORM_ZALLOC(buffer_size);
	if (!buffer) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] Cannot allocate buffer for %s\n", __FUNCTION__);
		return 0;
	}

	result  = rtk_linux_map_general(ifname, MAP_GET_STA_INFO, buffer, buffer_size, 0);
	if (result <= 0) {
		PLATFORM_FREE(buffer);
		return result;
	}

	*sta_nr = buffer[0];
	if (*sta_nr <= 0) {
		PLATFORM_FREE(buffer);
		return 1;
	}
	pt        = &(buffer[1]);
	*sta_info = PLATFORM_MALLOC(sizeof(struct map_sta_info) * (*sta_nr));
	PLATFORM_MEMSET(*sta_info, 0, sizeof(struct map_sta_info) * (*sta_nr));

	for (i = 0; i < *sta_nr; i++) {
		_EnB(&pt, (*sta_info)[i].mac_addr, 6);
		_EnB(&pt, &(*sta_info)[i].link_time, 4);
		_E1B(&pt, &(*sta_info)[i].multiap_profile);
	}

	PLATFORM_FREE(buffer);
	return 1;
}

int _get_wifi_connected_devices_private(char *interface_name, struct interfaceInfo *m)
{
	struct map_sta_info *sta_info = NULL;
	INT16U               sta_nr   = 0;
	INT8U                i;

	if (_rtk_get_sta_info(interface_name, &sta_info, &sta_nr) <= 0) {
		PLATFORM_PRINTF_ERROR("[RTL] Read wlan sta info failed!\n");
		return -1;
	}

	m->neighbor_mac_addresses_nr = sta_nr;

	if (m->neighbor_mac_addresses_nr <= 0) {
		PLATFORM_PRINTF_FULL("[RTL] [%s] NO station is associated on this interface.\n",
		                     interface_name);
		PLATFORM_FREE(sta_info);
		return 1;
	}

	m->neighbor_list = (struct neighborInfo *)realloc(m->neighbor_list, sizeof(struct neighborInfo) * m->neighbor_mac_addresses_nr + 1);

	for (i = 0; i < sta_nr; i++) {
		memcpy(m->neighbor_list[i].mac_address, sta_info[i].mac_addr, 6);
		m->neighbor_list[i].link_time = sta_info[i].link_time;
		m->neighbor_list[i].multiap_profile = sta_info[i].multiap_profile;
		PLATFORM_PRINTF_FULL("[RTL] [%s] Associated STA MAC \t%02x:%02x:%02x:%02x:%02x:%02x\tLink Time (s):\t %llu\tProfile:\t %u\n",
		                     interface_name,
		                     m->neighbor_list[i].mac_address[0], m->neighbor_list[i].mac_address[1], m->neighbor_list[i].mac_address[2],
		                     m->neighbor_list[i].mac_address[3], m->neighbor_list[i].mac_address[4], m->neighbor_list[i].mac_address[5],
		                     m->neighbor_list[i].link_time, m->neighbor_list[i].multiap_profile);
	}
	PLATFORM_FREE(sta_info);
	return 1;
}

#ifdef RTK_MULTI_AP_R4
void rtk_linux_apply_spatial_reuse_config(struct SpatialReuseRequestTLV *request, char *interface_name)
{
	INT8U *radio_unique_ID;
	INT8U  i;
	INT8U  buffer[256] = { 0 };
	char   cmd[128];
	int    result = 0;

	if (NULL == request) {
		PLATFORM_PRINTF_ERROR("Invalid 'preference' argument\n");
		return;
	}
	radio_unique_ID = request->ruid;
	i               = spatial_reuse_data.spatial_reuse_response_nr;
	// Any rules that prevent the request from being set in the driver

	if (0 == spatial_reuse_data.spatial_reuse_response_nr) {
		spatial_reuse_data.spatial_reuse_response = (struct SpatialReuseConfigResponseTLV *)PLATFORM_MALLOC(sizeof(struct SpatialReuseConfigResponseTLV));
	} else {
		spatial_reuse_data.spatial_reuse_response = (struct SpatialReuseConfigResponseTLV *)PLATFORM_REALLOC(spatial_reuse_data.spatial_reuse_response, sizeof(struct SpatialReuseConfigResponseTLV) * (spatial_reuse_data.spatial_reuse_response_nr + 1));
	}

	spatial_reuse_data.spatial_reuse_response[i].tlv_type = TLV_TYPE_SPATIAL_REUSE_CONFIG_RESPONSE;
	PLATFORM_MEMCPY(spatial_reuse_data.spatial_reuse_response[i].ruid, radio_unique_ID, 6);
	PLATFORM_PRINTF_DEBUG("[MULTI_AP] Agent: Store a new spatial reuse response TLV with radio unique id: %02x:%02x:%02x:%02x:%02x:%02x\n",
	                      spatial_reuse_data.spatial_reuse_response[i].ruid[0],
	                      spatial_reuse_data.spatial_reuse_response[i].ruid[1],
	                      spatial_reuse_data.spatial_reuse_response[i].ruid[2],
	                      spatial_reuse_data.spatial_reuse_response[i].ruid[3],
	                      spatial_reuse_data.spatial_reuse_response[i].ruid[4],
	                      spatial_reuse_data.spatial_reuse_response[i].ruid[5]);

	PLATFORM_MEMSET(buffer, 0, sizeof(buffer));
	PLATFORM_MEMCPY(buffer + 5, request, sizeof(struct SpatialReuseRequestTLV));
	INT16U length = sizeof(struct SpatialReuseRequestTLV);
	buffer[3]     = TLV_TYPE_SPATIAL_REUSE_REQUEST;
	INT8U *length_ptr = buffer + 4;
	_E2B(&length_ptr, &length);
	result = PLATFORM_MAP_SET_GENERAL_IOCTL(interface_name, buffer, (INT16U)sizeof(buffer));
	if (result <= 0) {
		PLATFORM_PRINTF_INFO("ERROR: PLATFORM_MAP_SET_GENERAL_IOCTL on radio %s return value: [%d] \n", interface_name, result);
		spatial_reuse_data.spatial_reuse_response[i].response_code = 1;
	} else {
		spatial_reuse_data.spatial_reuse_response[i].response_code = 0;

		// Send a command to hostapd to trigger the change in the beacon
		sprintf(cmd, "hostapd_cli -i %s set bss_color %d", interface_name, request->bss_color);
		PLATFORM_PRINTF_INFO("Set hostapd interface %s bss colour to %d \n", interface_name, request->bss_color);
		PLATFORM_PRINTF_DEBUG("%s \n", cmd);
		system(cmd);
	}
	spatial_reuse_data.spatial_reuse_response_nr++;
}

void rtk_linux_get_spatial_reuse_config_response(struct SpatialReuseConfigResponseTLV **response, INT8U *response_nr)
{
	*response    = spatial_reuse_data.spatial_reuse_response;
	*response_nr = spatial_reuse_data.spatial_reuse_response_nr;
	// Reset the variable as its not useful after
	spatial_reuse_data.spatial_reuse_response_nr = 0;
}

INT8U rtk_linux_get_spatial_reuse_report_tlv(INT8U *buf, char *interface_name)
{
	INT16U tlv_length = 37;
	// 3 for PLATFORM_MAP_GET_GENERAL_IOCTL(operation_type + total_length)
	// 1 for TLV_TYPE
	// 2 for TLV_LENGTH
	// TLV_PAYLOAD will be fix value 37, according to the spec
	// so tempbuf size will be 43
	INT8U  tempbuf[43];
	int  processed_bytes = 0;
	INT8U *p;

	p    = tempbuf;
	*p++ = 0;                             //operation_type
	*p++ = 0;                             //total_length
	*p++ = 0;                             //total_length
	*p++ = TLV_TYPE_SPATIAL_REUSE_REPORT; //TLV_TYPE
	PLATFORM_MEMCPY(p, &tlv_length, 2);
	p += 2;
	PLATFORM_MEMSET(p, 0, tlv_length);

	processed_bytes = PLATFORM_MAP_GET_GENERAL_IOCTL(interface_name, tempbuf, sizeof(tempbuf));
	if (processed_bytes <= 0) {
		PLATFORM_PRINTF_DEBUG("Obtain Spatial Reuse Report TLV failed in MAP_GET_GENERAL_IOCTL.");
		return 0;
	} else {
		PLATFORM_MEMCPY(buf, tempbuf, processed_bytes);
		return 1;
	}
}

#endif //RTK_MULTI_AP_R4