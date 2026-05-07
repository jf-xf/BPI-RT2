/*
 * Copyright (C) 2018 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <stdio.h>  // printf
#include <unistd.h> // getopt
#include <stdlib.h> // exit
#include <string.h> // strtok
#include <ctype.h>

#include "platform.h"
#include "platform_interfaces.h"
#include "platform_interfaces_priv.h"  // addInterface
#include "platform_alme_server_priv.h" // almeServerPortSet()
#include "al.h"                        // start1905AL
#include "al_wsc.h"                    // start1905AL
#include "al_utils.h"                  // start1905AL
#include "al_datamodel.h"              // DMsetConfiguredState

#include "global_settings.h"

#include "map_initialization.h"

#include "api_response.h"

char *map_radio_name_24g = NULL;

char *map_radio_name_5gl = NULL;

char *map_radio_name_5gh = NULL;

char *map_vap_prefix = NULL;

char *map_vxd_prefix = "vxd";

char *map_bridge_name = NULL;

INT8U map_supported_service = 0;

INT32U customize_features;

char *controller_interface = NULL;

char *controller_al_interface = NULL;

char *interface_manufacturer_name = NULL;
char *interface_model_name = NULL;
char *interface_device_name = NULL;

struct CONFIG config = {0};

char *hw_country_str = NULL;

INT16U map_max_sta_num = 64;

INT8U eth_only = 0;

INT8U hw_reg_domain = 0;

INT8U  radio_data_nr = 0; //radio number

struct easymesh_radio_mib *radio_data = NULL;

////////////////////////////////////////////////////////////////////////////////
// Static (auxiliary) private functions, structures and macros
////////////////////////////////////////////////////////////////////////////////

unsigned char _interface_count_in_list(char *interface_name, char *interface_list)
{
	unsigned char count = 0;
	const char *  tmp   = interface_list;
	while (NULL != (tmp = strstr(tmp, interface_name))) {
		count++;
		tmp++;
	}
	return count;
}

// This function receives a comma separated list of interface names (example:
// "eth0,eth1,wlan0") and, for each of them, calls "addInterface()" (example:
// addInterface("eth0") + addInterface("eth1") + addInterface("wlan0"))
//
static void _parseInterfacesList(const char *str, INT16U primary_vid)
{
	char *aux;
	char *interface_name;
	char *save_ptr;

	char * base_interface_name = NULL;
	INT16U vid                 = 0;

	if (NULL == str) {
		return;
	}

	aux = strdup(str);

	interface_name = strtok_r(aux, ",", &save_ptr);
	if (NULL != interface_name) {
		base_interface_name = trimInterfaceNameVID(interface_name, &vid, NULL);
		if(primary_vid) {
			if (0 == vid || vid == primary_vid) {
				addInterface(base_interface_name);
			}
		} else {
			if (0 == vid) {
				addInterface(base_interface_name);
			}
		}
		PLATFORM_FREE(base_interface_name);

		while (NULL != (interface_name = strtok_r(NULL, ",", &save_ptr))) {
			base_interface_name = trimInterfaceNameVID(interface_name, &vid, NULL);
			if(primary_vid) {
				if (0 == vid || vid == primary_vid) {
					addInterface(base_interface_name);
				}
			} else {
				if (0 == vid) {
					addInterface(base_interface_name);
				}
			}
			PLATFORM_FREE(base_interface_name);
		}
	}

	free(aux);
	return;
}

////////////////////////////////////////////////////////////////////////////////
// External public functions
////////////////////////////////////////////////////////////////////////////////
struct device_info *map_get_device_information(char *bridge_name)
{
	struct device_info *info = PLATFORM_MALLOC(sizeof(struct device_info));
	PLATFORM_GET_MAC(bridge_name, info->al_mac);
	return info;
}

uint8_t map_init(const char *mq_name, uint8_t device_role,
                 uint8_t configure_state, uint8_t multiap_profile,
                 struct default_map_config default_setting,
                 uint8_t config_data_nr, struct radio_config_data *config_data,
                 char *al_interfaces, uint8_t wfa_test_mode)
{
	INT8U al_mac_address[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	char cmd[128] = {0x00};

	// Initialize platform-specific code
	//
	if (0 == PLATFORM_INIT()) {
		PLATFORM_PRINTF_ERROR("Failed to initialize platform\n");
		return INIT_ERROR_OS;
	}

	int verbosity_counter = (int)default_setting.verbose_level;

#ifdef _RTK_LINUX_
	PLATFORM_GET_MAC(default_setting.bridge_name, al_mac_address);
#endif

	PLATFORM_PRINTF_DEBUG_SET_VERBOSITY_LEVEL(verbosity_counter - 1);
	PLATFORM_LOG_FILE_INIT(MAP_CORE_LOG_FILE_PATH, default_setting.log_file_size, (int)default_setting.verbose_quiet); // unit of log_file_size is KB.
#if  defined(MAP_VER_ID) && defined(MAP_VER_DATE)
	PLATFORM_PRINTF_INFO("Start %s (%s) %s\n", MAP_VERSION, MAP_VER_ID, MAP_VER_DATE);
	snprintf(cmd, sizeof(cmd), "echo \"%s (%s) %s\" > /tmp/map_version", MAP_VERSION, MAP_VER_ID, MAP_VER_DATE);
#else
	PLATFORM_PRINTF_INFO("Start %s\n", MAP_VERSION);
	snprintf(cmd, sizeof(cmd), "echo \"%s\" > /tmp/map_version", MAP_VERSION);
#endif
	system(cmd);
	PLATFORM_PRINTF_DEBUG("AL MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", al_mac_address[0], al_mac_address[1], al_mac_address[2], al_mac_address[3], al_mac_address[4], al_mac_address[5]);

	config.is_controller  = 0;
	config.self_listening = 0;

	config.wfa_test_mode = 0;

	if (default_setting.self_listening) {
		config.self_listening = 1;
	}

	map_radio_name_24g = default_setting.radio_name_24g;

	map_radio_name_5gl = default_setting.radio_name_5gl;

	map_radio_name_5gh = default_setting.radio_name_5gh;

	eth_only = default_setting.eth_only;

	hw_reg_domain = default_setting.hw_reg_domain;

	radio_data_nr = default_setting.radio_data_nr; //radio number

	radio_data = default_setting.radio_data;

	// Notice: Do _parseInterfacesList() after assign:
	// map_radio_name_24g, map_radio_name_5gl, map_radio_name_5gh, eth_only
	_parseInterfacesList(al_interfaces, default_setting.primary_vid);

	customize_features = default_setting.customize_features;

	map_vap_prefix = default_setting.vap_prefix;

	map_bridge_name = default_setting.bridge_name;

	map_max_sta_num = default_setting.max_sta_num;

	interface_manufacturer_name = default_setting.interface_manufacturer_name;
	interface_model_name = default_setting.interface_model_name;
	interface_device_name = default_setting.interface_device_name;

	hw_country_str = default_setting.hw_country_str;

	DMinit();

	map_supported_service = device_role;

	if (device_role) {
		config.is_controller = 1;
		updateConfigData(config_data_nr, config_data);
		config.self_listening = 1;
	}

	config.configure_state = configure_state;

	config.primary_vid = default_setting.primary_vid;

	config.default_pcp = default_setting.default_pcp;

	config.wfa_test_mode = wfa_test_mode;

	config.overwrite_pbc = 0;

	config.alme_port_number = 8888;

	if (default_setting.overwrite_pushbutton) {
		config.overwrite_pbc = 1;
	}

	config.max_bss_number = default_setting.max_bss_per_radio;

	config.max_resend_time = default_setting.resend_time;

	config.device_name = strdup(default_setting.device_name);

	config.neighbor_removal_threshold = default_setting.neighbor_removal_threshold;

	config.multiap_profile = multiap_profile;

	config.common_netlink = default_setting.common_netlink;

	controller_interface = default_setting.controller_interface;

	controller_al_interface = default_setting.controller_al_interface;

	DMsetDPPCreds(default_setting.csign_key, default_setting.netaccess_key, default_setting.pp_key, default_setting.signed_connector_1905);

	DMsetExcludedInterfaces(default_setting.excluded_interfaces);

	DMsetAutoconfigRenewBand(default_setting.autoconfig_renew_on_band);

	PLATFORM_PRINTF_INFO("[CONFIG] This is a Multi-AP %s with Profile %d\n", config.is_controller ? "controller" : "agent", multiap_profile);

	almeServerPortSet(config.alme_port_number);

	api_set_response_queue(mq_name);

	start1905AL(al_mac_address);

	return 0;
}

void map_set_verbose_level(int verbose_level)
{
	PLATFORM_PRINTF_DEBUG_SET_VERBOSITY_LEVEL(verbose_level - 1);
}

void map_enable_log()
{
	PLATFORM_ENABLE_LOG();
}

void map_disable_log()
{
	PLATFORM_DISABLE_LOG();
}

void map_exit()
{
	terminate1905AL();
}
