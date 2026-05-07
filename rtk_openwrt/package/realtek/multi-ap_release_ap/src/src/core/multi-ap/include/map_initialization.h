/*
 * Copyright (C) 2018 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _MAP_INITIALIZATION_H_
#define _MAP_INITIALIZATION_H_

#include <stdint.h>

#define INIT_ERROR_OUT_OF_MEMORY      (1)
#define INIT_ERROR_INVALID_ARGUMENTS  (2)
#define INIT_ERROR_NO_INTERFACES      (3)
#define INIT_ERROR_INTERFACE_ERROR    (4)
#define INIT_ERROR_OS                 (5)
#define INIT_ERROR_PROTOCOL_EXTENSION (6)
#define INIT_ERROR_CONFIG_FILE        (7)

#define MULTI_AP_TEARDOWN_BIT      (0x10)
#define MULTI_AP_FRONTHAUL_BSS_BIT (0x20)
#define MULTI_AP_BACKHAUL_BSS_BIT  (0x40)
#define MULTI_AP_BACKHAUL_STA_BIT  (0x80)

#define MAP_CONFIG_2G                 (0)
#define MAP_CONFIG_5GL                (1)
#define MAP_CONFIG_5GH                (2)

#define MAP_LOG_FILE_PATH "/var/log/easymesh_log.txt"
#define MAP_CORE_LOG_FILE_PATH "/var/log/multiap_core_log.txt"

struct bss_config_data {
	char *   ssid;
	char *   network_key;
	uint8_t  authentication_type;
	uint8_t  encryption_type;
	uint8_t  network_type;
	uint8_t  bss_index;
	uint8_t  al_id[6];
	uint16_t vid;
	char *   signed_connector;
};

struct vendor_specific_data {
	uint8_t  vendor_oui[3];
	uint16_t payload_len;
	uint8_t *payload;
};

struct radio_config_data {
	uint8_t                      radio_band;
	uint8_t                      bss_data_nr;
	struct bss_config_data *     bss_data;
	uint8_t                      vendor_data_nr;
	struct vendor_specific_data *vendor_datas;
};

struct default_map_config {
	char *   device_name;
	uint8_t  resend_time;
	uint16_t max_sta_num;
	uint8_t  verbose_level;
	uint8_t  verbose_quiet;
	uint8_t  log_file_size;
	uint8_t  max_bss_per_radio;
	uint8_t  overwrite_pushbutton;
	uint8_t  self_listening;
	uint16_t neighbor_removal_threshold;
	char *   excluded_interfaces;
	char *   radio_name_5gh;
	char *   radio_name_5gl;
	char *   radio_name_24g;
	char *   vap_prefix;
	char *   bridge_name;
	uint16_t primary_vid;
	uint8_t  default_pcp;
	uint8_t  common_netlink;
	uint32_t customize_features;
	char *   controller_interface;
	char *   controller_al_interface;
	char *   interface_manufacturer_name;
	char *   interface_model_name;
	char *   interface_device_name;
	uint8_t  autoconfig_renew_on_band;
	char *   hw_country_str;
	uint8_t  eth_only;
	uint8_t  hw_reg_domain;
	uint8_t  radio_data_nr; //radio number
	struct easymesh_radio_mib *radio_data;
	char *   csign_key;
	char *   netaccess_key;
	char *   pp_key;
	char *   signed_connector_1905;
};

struct device_info {
	uint8_t al_mac[6];
};

uint8_t map_init(const char *mq_name, uint8_t device_role,
                 uint8_t configure_state, uint8_t multiap_profile,
                 struct default_map_config default_setting,
                 uint8_t config_data_nr, struct radio_config_data *config_data,
                 char *al_interfaces, uint8_t wfa_test_mode);

uint8_t map_set_controller(int as_controller);

struct device_info* map_get_device_information(char *bridge_name);

void map_set_verbose_level(int verbose_level);

void map_enable_log();

void map_disable_log();

void map_exit();

#endif

