/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _EASYMESH_DATAMODEL_H_
#define _EASYMESH_DATAMODEL_H_
#include <stdint.h>
#include "map_initialization.h"
//
enum {
	SOFT_RELOAD,
	FULL_RELOAD,
	VENDOR_DATA_RELOAD,
	BAND_MIB_SET,
	CONFIG_STATE_SET,
	MIB_RELOAD,
	TEST_RELOAD,
	AGENT_RESTART
};

#define EASYMESH_RADIO_2G                 (0)
#define EASYMESH_RADIO_5GL                (1)
#define EASYMESH_RADIO_5GH                (2)
#define EASYMESH_RADIO_5GF                (3)

#define AUTH_DISABLED          0x01
#define AUTH_WPA               0x02
#define AUTH_WEP               0x04
#define AUTH_WPA2              0x20
#define AUTH_WPA3              0x40
#define AUTH_DPP               0x80

#define ENCRYPT_AES            0x08

#define CUSTOMIZE_WSC_INFO		0x00000001  //Modify for AUTH/ENCRYPTION FLAGS in autconfig M1/M2 packet to fit CT SPEC
#define SUPPORT_OPEN_ENCRYPT	0x00000002  //multi-ap support open encrypt
#define SUPPORT_NOT_WPS_CYCLE	0x00000004  //multi-ap wps not cycle
#define SUPPORT_ETHERNET_ADAPTER	0x00000008  //multi-ap support ethernet adapter
#define SUPPORT_SHOW_EXTRA_TOPOLOGY	0x00000010  //multi-ap show extra topology
#define FRONTBSS_DEFAULT_ENABLE	0x00000020  // default enable fronthaul bss before easymesh autoconfig
#define RCPI_ROAMING_ENABLE	0x00000080  // enable agent initiated rcpi roaming for ct
#define SUPPORT_WIRED_LINER_TOPOLOGY	0x00000100  // multi-ap support wired liner topology

struct easymesh_interface_mib {
	char *  ssid;
	char *  network_key;
	uint8_t network_type;
	uint8_t is_enabled; //1: enabled, 0: disabled
	uint8_t bss_index;
	uint8_t encrypt_type;
	uint8_t need_configure;
	uint8_t interface_index;
	uint8_t authentication_type;
	char *  signed_connector;
};

struct easymesh_radio_mib {
	uint8_t                        radio_band;
	uint8_t                        radio_channel;
	uint8_t                        channel_bandwidth;
	uint8_t                        control_sideband; // 0: upper, 1: lower
	uint8_t                        need_change_channel;
	char *                         repeater_ssid;
	uint8_t                        interface_nr;
	struct easymesh_interface_mib *interface_mib;
};

struct easymesh_datamodel {
	//global
	uint16_t alme_port_number;
	uint8_t  max_resend_time;
	uint16_t max_sta_number;
	char *   device_name;
	char *   controller_interface;
	char *   controller_al_interface;
	uint8_t  rssi_weightage;
	uint8_t  path_weightage;
	uint8_t  cu_weightage;
	uint8_t  roam_score_difference;
	uint8_t  min_evaluation_interval;
	uint8_t  min_roam_interval;
	uint8_t  max_num_device_allowed;
	uint32_t throughput_threshold;
	char *   excluded_interfaces;
	uint16_t primary_vid;
	uint8_t  default_pcp;
	uint8_t  max_bss_per_radio;
	uint8_t  rssi_5g_threshold;
	uint8_t  rssi_24g_threshold;
	uint8_t  common_netlink;
	uint8_t  stp_monitor_enable;
	uint8_t  loop_detect_enable;
	uint8_t  map_supported_service;
	uint8_t  max_renew_resend_time;
	uint8_t  beacon_request_ch;
	uint8_t  steering_opportunity_enable;
	uint32_t max_log_size;
	uint8_t  bss_list_update;
	uint8_t  backhaul_bss_index;
	//mib
	uint8_t  configured_band;
	uint8_t  hw_reg_domain;
	char *   hw_country_str;
	uint32_t customize_features;
	uint8_t  ap_mib_channel;

	char *   radio_name_24g;
	uint8_t  radio_24g_mac[6];

	char *   radio_name_5gl;
	uint8_t  radio_5gl_mac[6];

	char *   radio_name_5gh;
	uint8_t  radio_5gh_mac[6];

	char *   vap_prefix;
	char *   bridge_name;

	uint8_t autoconfig_renew_on_band;

	uint8_t radio_number;

	uint8_t                    radio_data_nr; //radio number
	struct easymesh_radio_mib *radio_data;

	uint8_t                   config_data_nr; //controller autoconfig radio data number
	struct radio_config_data *per_radio_config;

	uint8_t  vpair_number; // controller vlan configuration number
	uint16_t *vlan_id;
	char     **vssid;
	char *   interface_manufacturer_name;
	char *   interface_model_name;
	char *   interface_device_name;
	uint8_t  mcs_20m_table[22];
	uint8_t  mcs_40m_table[18];
	uint8_t  eth_only;
	char *   signed_connector_1905;
	char *   netaccess_key;
	char *   csign_key;
	char *   pp_key;
};

struct easymesh_api_info {
	uint8_t is_controller_found;
	char*   backhaul_interface;
	uint8_t parent_id[6];
};

struct backhaul_link_detect_control{
	int backhaul_switch;
	int check_interval;
	int link_down_threshold;
	int discovery_down_threshold;
	int backhaul_sta_radio_index;
};

struct map_checker_datamodel{
	int backhaul_switch;
	int check_interval;
	int link_down_threshold;
	int discovery_down_threshold;
	int backhaul_sta_radio_index;
	int arp_loop;
	int auto_renew_ip;
};

struct customized_status_info {
	uint8_t current_status;
};

#endif
