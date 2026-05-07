/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <mqueue.h>
#include <errno.h>
#include <string.h>
#include <sys/inotify.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <limits.h>

#include "ini.h"

#include "map_initialization.h"
#include "map_utils.h"
#include "map_commands.h"
#include "map_tlvs.h"
#include "map_timer.h"
#include "map_logger.h"
#include "map_dataelement.h"
#include "map_clientsteer.h"

#include "easymesh_utils.h"
#include "config_file_handler.h"
#include "map_backhaul.h"

#include "eth_monitor.h"

#include "easymesh_backhaul_switch.h"

#define QUEUE_NAME "/map_queue"

#define MAX_BRIDGE_PRIORITY 61440

#define BRIDGE_PRIORITY_STEP 4096

#define MAX_MESSAGE_SIZE 4096

#define MAX_RESEND_TIME 3

#define MAX_SEARCH_TIME 6

#define ETH_SEARCH_TIME 2

#ifndef O_CLOEXEC
#define O_CLOEXEC	02000000	/* set close_on_exec */
#endif

uint8_t _controller_found = 0;

uint8_t _vxd_off = 0;

uint8_t _boot_time_func_off = 4;

#ifdef EASYMESH_ETH_MONITOR
void update_backhaul_link_file(uint8_t backhaul_link)
{
	FILE *fd = fopen(BACKHAUL_LINK_FLAG_PATH, "w+e");
	if (NULL == fd) {
		printf("Could not create tmp file map_backhaul_link\n");
		return;
	}
	fputc('0' + backhaul_link, fd);
	fclose(fd);
}
#endif

#define SSID_BSS_INDEX_MAPPING 1
#define SUB_ELEMENT_TYPE_SIZE 1
#define SUB_ELEMENT_LENGTH_SIZE 2
#define SSID_LENGTH_SIZE 1
#define BSS_INDEX_LENGTH_SIZE 1

void _enable_agent(struct easymesh_datamodel *database)
{
	if (!_controller_found) {
		_controller_found = 1;
		configure_aps_func_off(0, MAP_CONFIGURE_BAND_FULL, database);
		send_operating_channel_change_notification();
	}
}

void _disable_agent(struct easymesh_datamodel *database)
{
	if (_controller_found) {
		_controller_found = 0;
		if((database->customize_features&FRONTBSS_DEFAULT_ENABLE) == 0){
			configure_aps_func_off(0, 0, database);
		}
	}
}

void dump_dataelement(struct Network *network)
{
	unsigned char i, j, k, l, m;

	// log_info("-----------------DATA ELEMENT-------------------------\n\n");

	log_info("===============================================\n");
	log_info("Network ID: %s | %d Devices\n", network->id, network->number_of_devices);
	log_info("===============================================\n");

	//Devices
	for (i = 0; i < network->number_of_devices; i++) {
		log_info("Device %d - %02x%02x%02x%02x%02x%02x | %d Radios\n",
			i + 1,
			network->device_list[i]->id[0], network->device_list[i]->id[1],
			network->device_list[i]->id[2], network->device_list[i]->id[3],
			network->device_list[i]->id[4], network->device_list[i]->id[5],
			network->device_list[i]->number_of_radios);

		log_info("Direct nb nr: %d\n", network->device_list[i]->number_of_direct_neighbors);
		for (m = 0; m < network->device_list[i]->number_of_direct_neighbors; m++) {
			log_info("Nb %d : %02x%02x%02x%02x%02x%02x\n",
				m,
				network->device_list[i]->direct_neighbor_list[m].al_mac[0],
				network->device_list[i]->direct_neighbor_list[m].al_mac[1],
				network->device_list[i]->direct_neighbor_list[m].al_mac[2],
				network->device_list[i]->direct_neighbor_list[m].al_mac[3],
				network->device_list[i]->direct_neighbor_list[m].al_mac[4],
				network->device_list[i]->direct_neighbor_list[m].al_mac[5]);
		}

		//Radios
		for (j = 0; j < network->device_list[i]->number_of_radios; j++) {
			log_info("\t Radio %d - %02x%02x%02x%02x%02x%02x | %d BSS\n",
				j + 1,
				network->device_list[i]->radio_list[j]->id[0],
				network->device_list[i]->radio_list[j]->id[1],
				network->device_list[i]->radio_list[j]->id[2],
				network->device_list[i]->radio_list[j]->id[3],
				network->device_list[i]->radio_list[j]->id[4],
				network->device_list[i]->radio_list[j]->id[5],
				network->device_list[i]->radio_list[j]->number_of_bss);

			//BSS
			for (k = 0; k < network->device_list[i]->radio_list[j]->number_of_bss; k++) {
				log_info("\t\t Bss %d - %02x%02x%02x%02x%02x%02x | %s | %d Stas\n",
					k + 1,
					network->device_list[i]->radio_list[j]->bss_list[k]->bssid[0],
					network->device_list[i]->radio_list[j]->bss_list[k]->bssid[1],
					network->device_list[i]->radio_list[j]->bss_list[k]->bssid[2],
					network->device_list[i]->radio_list[j]->bss_list[k]->bssid[3],
					network->device_list[i]->radio_list[j]->bss_list[k]->bssid[4],
					network->device_list[i]->radio_list[j]->bss_list[k]->bssid[5],
					network->device_list[i]->radio_list[j]->bss_list[k]->ssid,
					network->device_list[i]->radio_list[j]->bss_list[k]->number_of_sta);

				//STA
				for (l = 0; l < network->device_list[i]->radio_list[j]->bss_list[k]->number_of_sta; l++) {
					log_info("\t\t\tSta %d - %02x%02x%02x%02x%02x%02x \n",
						l + 1,
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->mac_address[0],
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->mac_address[1],
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->mac_address[2],
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->mac_address[3],
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->mac_address[4],
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->mac_address[5]);
				}
			}
		}
		log_info("-----------------------------------------------\n");
	}
	// log_info("\n-----------------DATA ELEMENT-------------------------\n\n");
}

mqd_t create_mqueue(const char *name)
{
	mqd_t          mqdes;
	struct mq_attr attr;
	char           name_tmp[20];

	if (name[0] != '/') {
		snprintf(name_tmp, 20, "/%s", name);
		name = name_tmp;
	}

	attr.mq_flags   = 0;
	attr.mq_maxmsg  = 50;
	attr.mq_curmsgs = 0;
	attr.mq_msgsize = MAX_MESSAGE_SIZE;

	mq_unlink(name);

	if ((mqd_t)-1 == (mqdes = mq_open(name, O_RDWR | O_CREAT | O_CLOEXEC, 0666, &attr))) {
		log_error("mq_open('%s') returned with errno=%d (%s)\n", name, errno, strerror(errno));
		return 0;
	}

	return mqdes;
}

void _extract_bss_index_from_vendor_data(struct vendor_specific_data *vendor_data, struct radio_config_data *radio_config)
{
	int bss_data_index = 0;
	int payload_idx    = 0;
	int ssid_len;
	//sub element id type (1)
	payload_idx += SUB_ELEMENT_TYPE_SIZE;
	//sub element length (2)
	payload_idx += SUB_ELEMENT_LENGTH_SIZE;
	while (payload_idx < vendor_data->payload_len - 1) {
		//ssid length (1)
		ssid_len = vendor_data->payload[payload_idx];
		payload_idx += SSID_LENGTH_SIZE;
		//ssid (ssid_length)
		if (!strcmp((char *)(&(vendor_data->payload[payload_idx])), radio_config->bss_data[bss_data_index].ssid) && (int)strlen(radio_config->bss_data[bss_data_index].ssid) + 1 == ssid_len) {
			payload_idx += ssid_len;
			//bss_index (1)
			radio_config->bss_data[bss_data_index].bss_index = vendor_data->payload[payload_idx];
			payload_idx += BSS_INDEX_LENGTH_SIZE;
			bss_data_index++;
		}
	}
}

void _set_backhaul_config_bss_index(struct radio_config_data *radio_config, uint8_t backhaul_bss_index)
{
	int     i;
	uint8_t bss_index = 0;

	for (i = 0; i < radio_config->bss_data_nr; i++) {
		if (MULTI_AP_BACKHAUL_BSS_BIT & radio_config->bss_data[i].network_type) {
			radio_config->bss_data[i].bss_index = backhaul_bss_index;
		} else {
			if (bss_index == backhaul_bss_index) {
				bss_index++;
			}
			radio_config->bss_data[i].bss_index = bss_index;
			bss_index++;
		}
	}
}

int main(int argc, char *argv[])
{
	int   c;
	int   verbose_level = 1; // Only ERROR messages
	int   verbose_quiet = 1; // Only log to file
	char  buffer[MAX_MESSAGE_SIZE + 1];
	int   as_daemon            = 0;
	int   wfa_test             = 0;
	int   overwrite_pushbutton = 0;
	mqd_t mq;
	int   self_query       = 0;
#if defined(BACKHAUL_REAL_TIME_SWITCH) || defined(EASYMESH_RENEW_IP)
	int   backhaul_uplink_count = 0 ;
	char  controller_ip[16] = {0};
#endif
	int   backhaul_uplink_status = MAP_LINK_DOWN;
	char  backhaul_receiving_interface[20] = {0};

	char *image_version = EASY_MESH_VERSION;

	uint8_t controller_aliveness_max    = 30;
	uint8_t controller_aliveness        = 60;

	uint8_t identical_state = 0, need_full_reload = 0;
	uint8_t configure_state; // = get_configure_state();
	uint8_t have_vendor_data = 0;

	MAP_VLAN_CONFIG_T *vlan_config     = NULL;
	uint8_t            vlan_config_nr  = 0;
	uint16_t           primary_vid     = 0;
	uint8_t            multiap_profile = 1;

	uint8_t vlan_configured_flag = 0;
	uint8_t vlan_configure_delay = 0;

	uint8_t single_backhaul_selection_flag = 0;
	uint8_t single_backhaul_selection_delay = 0;

	uint8_t need_search_controller = 0;

	uint8_t status_updated = 0;

	pthread_t                      eth_thread;
	struct eth_monitor_thread_data eth_thread_data         = { 0 };
	uint8_t                        controller_search_count = MAX_SEARCH_TIME;
	uint8_t                        is_searching_controller = 0;
	uint8_t                        vxd_controller_flag     = 0;
	int                            eth_interface_nr        = 0;
	char **                        eth_interface_names     = NULL;
	char                           cmd[128]                = { 0 };

	uint8_t is_controller_interface_up = 0;
	uint8_t map_agent_reinit_timer = 0;
	char    reinit_cmd[64] = {0};

	uint64_t controller_version = 0;

	uint64_t old_release      = convertVersionStringToNumber("EasyMesh V1.3FBeta");
	uint64_t old_beta_release = convertVersionStringToNumber("EasyMesh V1.4Beta");

	uint8_t             timer_counter = 0;
	struct Network      map_network   = { 0 };
	struct device_info *info          = NULL;

	uint8_t rssi_timer_counter = 0;

	while ((c = getopt(argc, argv, "vdtp:x")) != -1) {
		switch (c) {

		case 'v': {
			// Each time this flag appears, the verbose_level is incremented.
			verbose_level++;
			break;
		}
		case 'd': {
			as_daemon = 1;
			break;
		}
		case 't': {
			log_info("Start in WFA Certification Mode\n");
			wfa_test = 1;
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
		case 'x': {
			// enable logging to console
			verbose_quiet = 0;
			break;
		}
		}
	}

	if (as_daemon) {
		if (-1 == daemon(0, 1)) {
			log_error("Run multi-ap as daemon error!\n");
			return 0;
		}
	}

	if (verbose_quiet) {
		disableLogDebug();
	} else {
		enableLogDebug();
	}
	log_set_level(verbose_level);

	//read config file
	struct easymesh_datamodel easymesh_database = { 0 };

	easymesh_database.config_data_nr   = 0;
	easymesh_database.configured_band  = 0;
	easymesh_database.hw_reg_domain    = 0;
	easymesh_database.hw_country_str   = NULL;
	easymesh_database.customize_features  = 0;
	easymesh_database.radio_data_nr    = 0;
	easymesh_database.device_name      = NULL;
	easymesh_database.per_radio_config = NULL;
	easymesh_database.radio_data       = NULL;
	easymesh_database.primary_vid      = 0;
	easymesh_database.default_pcp      = 0;
	easymesh_database.max_bss_per_radio  = 5;
	easymesh_database.rssi_5g_threshold  =50;
	easymesh_database.rssi_24g_threshold =20;
	easymesh_database.common_netlink     = 0;
	easymesh_database.controller_interface = NULL;
	easymesh_database.controller_al_interface = NULL;
	easymesh_database.excluded_interfaces = NULL;
	easymesh_database.steering_opportunity_enable = 0;
	easymesh_database.bridge_name = NULL;
	easymesh_database.eth_only = 0;

	easymesh_database.interface_manufacturer_name = NULL;
	easymesh_database.interface_model_name = NULL;
	easymesh_database.interface_device_name = NULL;
	easymesh_database.stp_monitor_enable = 1;
	easymesh_database.loop_detect_enable = 1;
	easymesh_database.autoconfig_renew_on_band = 2;
	memset(easymesh_database.mcs_20m_table, 0, sizeof(easymesh_database.mcs_20m_table));
	memset(easymesh_database.mcs_40m_table, 0, sizeof(easymesh_database.mcs_40m_table));
	easymesh_database.max_sta_number  = 64;
	easymesh_database.max_log_size = 100;
	easymesh_database.bss_list_update = 0;
	easymesh_database.backhaul_bss_index = 1;

	easymesh_database.csign_key             = NULL;
	easymesh_database.netaccess_key         = NULL;
	easymesh_database.pp_key                = NULL;
	easymesh_database.signed_connector_1905 = NULL;

	//easymesh status info
	struct customized_status_info status_info;

	status_info.current_status = STATUS_CONTROLLER_LOST;

	//easymesh api info
	struct easymesh_api_info easymesh_api = {0};
	uint8_t need_update_api_output    = 0;

	int8_t backhaul_sta_radio_index = -1;
	int8_t backhaul_sta_radio_byte = 0;

	if (ini_parse("/var/multiap.conf", read_config_file, &easymesh_database) < 0) {
		log_error("[RTK] Can't load configuration file!! \n");
		return INIT_ERROR_CONFIG_FILE;
	}
	if (ini_parse("/var/multiap_mib.conf", read_mib_config_file, &easymesh_database) < 0) {
		log_error("[RTK] Can't load mib configuration file!! \n");
		return INIT_ERROR_CONFIG_FILE;
	}

	initLogFile(MAP_LOG_FILE_PATH, easymesh_database.max_log_size);

	log_info("<<<< Initialized EasyMesh Log >>>\n");

	log_info("EasyMesh Turnkey %s\n", image_version);

	if (NULL == easymesh_database.radio_name_5gh) {
		log_error("5G radio undefined!\n");
		exit(1);
	}

	if (strcmp(easymesh_database.radio_name_5gl, easymesh_database.radio_name_5gh)) {
		easymesh_database.radio_number = 3;
	} else {
		easymesh_database.radio_number = 2;
	}

	log_info("Radio Number: %d\n", easymesh_database.radio_number);

	//do auto-config initialization
	int i, j;
	for (i = 0; i < easymesh_database.radio_data_nr; i++) {
		for (j = 0; j < easymesh_database.radio_data[i].interface_nr; j++) {
			easymesh_database.radio_data[i].interface_mib[j].need_configure = 0;
		}
		if (easymesh_database.radio_data[i].interface_mib[j - 1].is_enabled && MULTI_AP_BACKHAUL_STA_BIT == easymesh_database.radio_data[i].interface_mib[j - 1].network_type) {
			if (RADIO_BAND_2G == easymesh_database.radio_data[i].radio_band) {
				backhaul_sta_radio_byte |= MASK_BAND_2G;
			} else if (RADIO_BAND_5GL == easymesh_database.radio_data[i].radio_band) {
				backhaul_sta_radio_byte |= MASK_BAND_5GL;
			} else if (RADIO_BAND_5GH == easymesh_database.radio_data[i].radio_band) {
				backhaul_sta_radio_byte |= MASK_BAND_5GH;
			}
			if (backhaul_sta_radio_byte & MASK_BAND_5GH) {
				backhaul_sta_radio_index = easymesh_database.radio_name_5gh[4] - '0';
			} else if (backhaul_sta_radio_byte & MASK_BAND_5GL) {
				backhaul_sta_radio_index = easymesh_database.radio_name_5gl[4] - '0';
			} else if (backhaul_sta_radio_byte & MASK_BAND_2G) {
				backhaul_sta_radio_index = easymesh_database.radio_name_24g[4] - '0';
			}
		}
	}

	if (!easymesh_database.stp_monitor_enable && backhaul_sta_radio_byte >= (MASK_BAND_2G | MASK_BAND_5GL)) {//stp is disabled and have two vxd on
		single_backhaul_selection_flag =1;
	}

	if (0 == easymesh_database.primary_vid) {

		FILE * vlan_fp = fopen("/tmp/map_vlan_setting_configured", "re");
		if (vlan_fp) {
			char *  vlan_line = NULL;
			size_t  vlan_len  = 0;
			ssize_t vlan_read;

			while ((vlan_read = getline(&vlan_line, &vlan_len, vlan_fp)) != -1) {
				char ssid[64];
				int  vid = 0;
				sscanf(vlan_line, "%63s %d", ssid, &vid);

				if (0 == strcmp(ssid, "Primary")) {
					log_info("Primary VID: %d\n", vid);
					easymesh_database.primary_vid = vid;
					break;
				}
			}

			fclose(vlan_fp);
		}
	}

	if (!easymesh_database.bridge_name) {
		easymesh_database.bridge_name = strdup("br0");
		log_debug("Set default bridge name br0\n");
	}

	info = map_get_device_information(easymesh_database.bridge_name);
	log_info("Device own al_mac %02x:%02x:%02x:%02x:%02x:%02x\n",
		info->al_mac[0], info->al_mac[1], info->al_mac[2],
		info->al_mac[3], info->al_mac[4], info->al_mac[5]);

	if (-1 != backhaul_sta_radio_index)
		log_info("Backhaul STA is wlan%d-vxd. Backhaul STA radio byte is %d \n", backhaul_sta_radio_index, backhaul_sta_radio_byte);
	else
		log_info("No backhaul STA defined, eth backhaul only.\n");

	identical_state = easymesh_database.configured_band; //mismatch protection

	if(easymesh_database.controller_interface && easymesh_database.controller_al_interface) {
		// If both controller_interface & controller_al_interface are defined in /var/multiap.conf,
		// it means a controller exists on this device and link us within controller_interface
		if(is_interface_up(easymesh_database.controller_interface)) {
			is_controller_interface_up = 1;
#ifdef EASYMESH_ETH_MONITOR
			update_backhaul_link_file(BACKHAUL_ETH);
#endif
		}

		log_debug("Controller interface %s %s up!!\n",
			easymesh_database.controller_interface, is_controller_interface_up ? "is" : "isn't");
	}

	char *interfaces = (char *)malloc(512);
	memset(interfaces, 0, 512);

	if (easymesh_database.controller_interface && easymesh_database.controller_al_interface) {
		for(i = 0; i < easymesh_database.radio_data_nr; i++) {
			for(j = 0; j < easymesh_database.radio_data[i].interface_nr; j++) {
				char *radio_name = easymesh_database.radio_name_24g;
				char  ifname[20] = {0};
				int   polling = 0;

				if (0 == easymesh_database.radio_data[i].interface_mib[j].is_enabled)
					continue;

				if (easymesh_database.radio_data[i].radio_band == RADIO_BAND_2G)
					radio_name = easymesh_database.radio_name_24g;
				else if (easymesh_database.radio_data[i].radio_band == RADIO_BAND_5GL)
					radio_name = easymesh_database.radio_name_5gl;
				else if (easymesh_database.radio_data[i].radio_band == RADIO_BAND_5GH)
					radio_name = easymesh_database.radio_name_5gh;

				if (MULTI_AP_BACKHAUL_STA_BIT == easymesh_database.radio_data[i].interface_mib[j].network_type)
					add_single_radio_interface(&interfaces, radio_name, "vxd", -1, ifname);
				else
					add_single_radio_interface(&interfaces, radio_name, easymesh_database.vap_prefix, j, ifname);

				while (1) {
					if (is_interface_up(ifname))
						break;

					polling++;
					log_warn("Interface %s is not ready, polling(%d)...\n", ifname, polling);
					if (polling == 10) {
						log_warn("Interface %s polling timeout --> please check interface\n", ifname);
						break;
					}
					sleep(1);
				}
			}
		}
	}
	else {
		// Check interfaces which is_enable equals 1 are ready (wait hostapd)
		for(i = 0; i < easymesh_database.radio_data_nr; i++) {
			for(j = 0; j < easymesh_database.radio_data[i].interface_nr; j++) {
				char *radio_name = easymesh_database.radio_name_24g;
				char  ifname[20] = {0};
				int   polling = 0;

				if (0 == easymesh_database.radio_data[i].interface_mib[j].is_enabled)
					continue;

				if (easymesh_database.radio_data[i].radio_band == RADIO_BAND_2G)
					radio_name = easymesh_database.radio_name_24g;
				else if (easymesh_database.radio_data[i].radio_band == RADIO_BAND_5GL)
					radio_name = easymesh_database.radio_name_5gl;
				else if (easymesh_database.radio_data[i].radio_band == RADIO_BAND_5GH)
					radio_name = easymesh_database.radio_name_5gh;

				if (MULTI_AP_BACKHAUL_STA_BIT == easymesh_database.radio_data[i].interface_mib[j].network_type)
					get_radio_interface(radio_name, "vxd", -1, ifname);
				else
					get_radio_interface(radio_name, easymesh_database.vap_prefix, j, ifname);

				while (1) {
					if (is_interface_up(ifname))
						break;

					polling++;
					log_warn("Interface %s is not ready, polling(%d)...\n", ifname, polling);
					if (polling == 10) {
						log_warn("Interface %s polling timeout --> please check interface\n", ifname);
						break;
					}
					sleep(1);
				}
			}
		}

		add_radio_interfaces(&interfaces, easymesh_database.radio_name_24g, easymesh_database.vap_prefix, easymesh_database.max_bss_per_radio - 1, 1);
		add_radio_interfaces(&interfaces, easymesh_database.radio_name_5gl, easymesh_database.vap_prefix, easymesh_database.max_bss_per_radio - 1, 1);
		add_radio_interfaces(&interfaces, easymesh_database.radio_name_5gh, easymesh_database.vap_prefix, easymesh_database.max_bss_per_radio - 1, 1);
	}

	if (easymesh_database.controller_interface) {
		// insert controller_interface into interfaces
		strcat(interfaces, easymesh_database.controller_interface);
		interfaces[strcspn(interfaces, "\r\n")] = ',';
	}

	append_br_interfaces(easymesh_database.bridge_name, interfaces);

	char *intf_name = strdup(interfaces);
	char *intf_token = strtok(intf_name, ",");

	while (intf_token != NULL) {
		log_debug("Find Interface: %s\n", intf_token);
		intf_token = strtok(NULL, ",");
	}
	free(intf_name);

	if (!easymesh_database.excluded_interfaces)
		easymesh_database.excluded_interfaces = "";

	if (!easymesh_database.stp_monitor_enable) {
		char *interfaces_copy = strdup(interfaces);
		char *token           = strtok(interfaces_copy, ",");

		while (token != NULL) {
			token = strtok(NULL, ",");
			if (NULL == token) {
				break;
			}

			char buffer[64]         = { 0 };
			int  interface_name_len = strlen(token);

			memcpy(buffer, token, interface_name_len);
			buffer[interface_name_len] = ',';
			if (token && NULL == strstr(easymesh_database.excluded_interfaces, buffer)) {
				if (get_valid_interface_name(token, VALID_ETH_INTERFACES_IN_BRIDGE)
				    || (easymesh_database.controller_interface && strstr(token, easymesh_database.controller_interface))) {
					eth_interface_nr += 1;
					eth_interface_names                       = (char **)realloc(eth_interface_names, sizeof(char *) * eth_interface_nr);
					eth_interface_names[eth_interface_nr - 1] = strdup(token);
					char *pos                                 = NULL;
					if ((pos = strchr(eth_interface_names[eth_interface_nr - 1], '\n')) != NULL)
						*pos = '\0';
				}
			}
		}
		free(interfaces_copy);
	}

	pthread_t                      map_thread;
	struct map_service_thread_data map_thread_data;
	map_thread_data.queue_name                                 = QUEUE_NAME;
	map_thread_data.device_role                                = 0;
	map_thread_data.multiap_profile                            = multiap_profile;
	map_thread_data.default_setting.self_listening             = 1;
	map_thread_data.default_setting.neighbor_removal_threshold = 90;
	map_thread_data.configure_state                            = easymesh_database.configured_band; //configure_state;
	map_thread_data.default_setting.resend_time                = MAX_RESEND_TIME;
	map_thread_data.default_setting.verbose_level              = verbose_level;
	map_thread_data.default_setting.verbose_quiet              = verbose_quiet;
	map_thread_data.default_setting.log_file_size              = easymesh_database.max_log_size;
	map_thread_data.default_setting.overwrite_pushbutton       = overwrite_pushbutton;
	map_thread_data.default_setting.primary_vid                = easymesh_database.primary_vid;
	map_thread_data.default_setting.default_pcp                = easymesh_database.default_pcp;
	map_thread_data.default_setting.max_bss_per_radio          = easymesh_database.max_bss_per_radio;
	map_thread_data.default_setting.common_netlink             = easymesh_database.common_netlink;
	map_thread_data.default_setting.customize_features         = easymesh_database.customize_features;
	map_thread_data.default_setting.autoconfig_renew_on_band   = easymesh_database.autoconfig_renew_on_band;
	map_thread_data.default_setting.max_sta_num                = easymesh_database.max_sta_number;
	map_thread_data.config_nr                                  = 0;
	map_thread_data.config_data                                = NULL;
	if (easymesh_database.device_name) {
		map_thread_data.default_setting.device_name            = strdup(easymesh_database.device_name);
	} else {
		map_thread_data.default_setting.device_name            = "EasyMesh Agent";
	}
	map_thread_data.default_setting.radio_name_24g             = easymesh_database.radio_name_24g;
	map_thread_data.default_setting.radio_name_5gl             = easymesh_database.radio_name_5gl;
	map_thread_data.default_setting.radio_name_5gh             = easymesh_database.radio_name_5gh;
	map_thread_data.default_setting.vap_prefix                 = easymesh_database.vap_prefix;
	map_thread_data.default_setting.bridge_name                = easymesh_database.bridge_name;
	if (easymesh_database.excluded_interfaces) {
		map_thread_data.default_setting.excluded_interfaces    = easymesh_database.excluded_interfaces;
	} else {
		map_thread_data.default_setting.excluded_interfaces    = "";
	}
	map_thread_data.default_setting.controller_interface       = easymesh_database.controller_interface;
	map_thread_data.default_setting.controller_al_interface    = easymesh_database.controller_al_interface;
	map_thread_data.default_setting.eth_only                   = 0;
	map_thread_data.default_setting.hw_reg_domain              = easymesh_database.hw_reg_domain;
	map_thread_data.al_interfaces                              = interfaces;
	map_thread_data.wfa_test_mode                              = wfa_test;

	max_sta_num_per_bss                                        = easymesh_database.max_sta_number;

	map_thread_data.default_setting.interface_manufacturer_name = easymesh_database.interface_manufacturer_name;
	map_thread_data.default_setting.interface_model_name = easymesh_database.interface_model_name;
	map_thread_data.default_setting.interface_device_name = easymesh_database.interface_device_name;
	map_thread_data.default_setting.hw_country_str        = easymesh_database.hw_country_str;

	map_thread_data.default_setting.csign_key             = easymesh_database.csign_key;
	map_thread_data.default_setting.netaccess_key         = easymesh_database.netaccess_key;
	map_thread_data.default_setting.pp_key                = easymesh_database.pp_key;
	map_thread_data.default_setting.signed_connector_1905 = easymesh_database.signed_connector_1905;

	mq = create_mqueue(QUEUE_NAME);

	log_debug("Registering time out event (MAP_QUEUE_EVENT_TIMEOUT_PERIODIC)...\n");
	{
		struct mapEventTimeOut aux;

		aux.timeout_ms = 5000; // 5 seconds
		aux.token      = 1;

		if (0 == MapRegisterQueueEvent(mq, MAP_QUEUE_EVENT_TIMEOUT_PERIODIC, &aux)) {
			log_error("Could not register timer callback\n");
			return 0;
		}
	}

	if (!wfa_test) {
		//initialize datamodel
		uint8_t al_mac_empty[6] = { 0 };
		if (update_network(&map_network, al_mac_empty)) {
			log_debug("Failed to initialize database! \n");
		}

		if((easymesh_database.customize_features&FRONTBSS_DEFAULT_ENABLE) == 0){
			configure_aps_func_off(0, 0, &easymesh_database);
		}

		log_debug("Registering API command event (MAP_QUEUE_EVENT_API_COMMAND)...\n");
		if (0 == MapRegisterQueueEvent(mq, MAP_QUEUE_EVENT_API_COMMAND, NULL)) {
			log_error("Could not register API callback\n");
			return 0;
		}

		log_debug("Registering time out event (MAP_QUEUE_EVENT_ONE_SECOND_TIMER)...\n");
		{
			struct mapEventTimeOut aux;

			aux.timeout_ms = 1000; // 1 seconds
			aux.token      = 1;

			if (0 == MapRegisterQueueEvent(mq, MAP_QUEUE_EVENT_ONE_SECOND_TIMER, &aux)) {
				log_error("Could not register timer callback\n");
				return 0;
			}
		}

		if (backhaul_sta_radio_index != -1) {
			if (!easymesh_database.stp_monitor_enable) {
				if (-1 != access(BACKHAUL_LINK_FLAG_PATH, F_OK)) {
					FILE *fd_tmp              = fopen(BACKHAUL_LINK_FLAG_PATH, "re");
					int  connected_interface = fgetc(fd_tmp);
					fclose(fd_tmp);
					if ('1' == connected_interface) {
						if (easymesh_database.loop_detect_enable) {
							log_debug("Turn off vxd interface...\n");
							sprintf(cmd, "ifconfig wlan%d-vxd down", backhaul_sta_radio_index);
							system(cmd);
						}
					} else { //we treat EOF as wifi backhaul so it will be closed in later steps.
						if (easymesh_database.loop_detect_enable) {
							log_debug("Turn on vxd interface...\n");
							sprintf(cmd, "iwpriv wlan%d-vxd set_mib func_off=0; ifconfig wlan%d-vxd up", backhaul_sta_radio_index, backhaul_sta_radio_index);
							system(cmd);
						}
					}
				} else {
					// if eth is not connected or device is unconfigured, disable func off and bring up vxd interface
					configure_state = easymesh_database.configured_band;
					if (is_any_eth_connected(eth_interface_nr, eth_interface_names, NULL) == 0 || configure_state != MAP_CONFIGURE_BAND_FULL) {
						if (easymesh_database.loop_detect_enable) {
							log_debug("Turn on vxd interface...\n");
							sprintf(cmd, "iwpriv wlan%d-vxd set_mib func_off=0; ifconfig wlan%d-vxd down up", backhaul_sta_radio_index, backhaul_sta_radio_index);
							system(cmd);
						}
					}

					// fully configured
					if (configure_state == MAP_CONFIGURE_BAND_FULL && -1 == access(BACKHAUL_LINK_FLAG_PATH, F_OK)) {
						log_debug("Device is fully configured, start ethernet controller checking....\n");
						is_searching_controller                  = 1;
						eth_thread_data.interface_nr             = eth_interface_nr;
						eth_thread_data.interface_names          = eth_interface_names;
						eth_thread_data.backhaul_sta_radio_index = backhaul_sta_radio_index;
						pthread_attr_t attr;
						pthread_attr_init(&attr);
						pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
						if (pthread_create(&eth_thread, &attr, eth_monitor_thread, &eth_thread_data)) {
							fprintf(stderr, "Error creating eth monitor thread\n");
							return 1;
						}
					}
				}
			} else {
				if (easymesh_database.loop_detect_enable) {
					log_debug("Turn on vxd interface...");
					char backhaul_sta_name[16] = { 0 };
					sprintf(backhaul_sta_name, "wlan%d-vxd", backhaul_sta_radio_index);
					set_vxd_interface_state(backhaul_sta_name, primary_vid, STATE_UP);
				}
			}
		}

		//------R1 DFS------------
		// buffer_mib_channel(&easymesh_database);
		//------------------------
	}

	if (pthread_create(&map_thread, NULL, map_service_thread, &map_thread_data)) {
		fprintf(stderr, "Error creating thread\n");
		return 1;
	}

	/* initialize customized status */
	struct customized_status cus_status;
	init_ctomusized_status(&cus_status);

	//User process for continuously reading user message queue
	while (1) {
		uint8_t *pt;
		uint8_t  message_type;

		// log_trace("\n");
		// log_trace("Waiting for new queue message...\n");
		if (-1 == mq_receive(mq, buffer, MAX_MESSAGE_SIZE, NULL)) {
			log_error("Something went wrong while trying to retrieve a new message from the queue. Ignoring...\n");
			log_error("mq_receive() returned with errno=%d (%s)\n", errno, strerror(errno));
			exit(errno);
		}

		// The first byte of 'queue_message' tells us the type of message that
		// we have just received
		//
		if (!self_query && _controller_found) {
			send_topology_query_request(info->al_mac);
			self_query = 1;
		}

		message_type = buffer[0];

		switch (message_type) {
		case MAP_QUEUE_EVENT_TIMEOUT_PERIODIC: {
			log_trace("<-- MAP_QUEUE_EVENT_TIMEOUT_PERIODIC\n");
			if (easymesh_database.bss_list_update) {
				obtain_5g_bss_list(&map_network, easymesh_database.radio_name_5gl, true);
			}
			if (!wfa_test) {
				if ((_boot_time_func_off > 0) && (0 == _controller_found)) {
					if((easymesh_database.customize_features&FRONTBSS_DEFAULT_ENABLE) == 0){
						configure_aps_func_off(0, 0, &easymesh_database);
					}
					_boot_time_func_off--;
				}

				if (!easymesh_database.stp_monitor_enable) {
					if (is_searching_controller) {
						if (get_eth_event()) {
							controller_search_count = ETH_SEARCH_TIME;
							clear_eth_event();
						}
						if (!_controller_found) {
							if (controller_search_count) {
								controller_search_count--;
								log_debug("Controller is not found, searching......\n");
								send_autoconfig_search_request();
							} else {
								log_debug("Controller is still not found.\n");
								sprintf(cmd, "wlan%d-vxd", backhaul_sta_radio_index);
								if (!is_interface_up(cmd)) {
									if (easymesh_database.loop_detect_enable) {
										log_debug("Switching to vxd......\n");
										sprintf(cmd, "iwpriv wlan%d-vxd set_mib func_off=0; ifconfig wlan%d-vxd up", backhaul_sta_radio_index, backhaul_sta_radio_index);
										system(cmd);
									}
									if (!is_any_eth_connected(eth_interface_nr, eth_interface_names, NULL)) {
										resume_eth_monitor();
									}
								}
								controller_search_count = MAX_SEARCH_TIME;
							}
						}
					} else if (!controller_aliveness) {
						send_autoconfig_search_request();
					}
				}
				if (controller_aliveness) {
					controller_aliveness -= 5;
					if (!controller_aliveness) {
						log_debug("CONTROLLER_LOST\n");
						status_updated = update_status(&status_info.current_status, STATUS_CONTROLLER_LOST);
						if (easymesh_database.stp_monitor_enable) {
							sprintf(cmd, "brctl setbridgeprio %s %d", easymesh_database.bridge_name, MAX_BRIDGE_PRIORITY);
							system(cmd);
						}
						if (backhaul_uplink_status == MAP_LINK_UP) {
							backhaul_uplink_status = MAP_LINK_DOWN;
							update_backhaul_link_status(backhaul_uplink_status, NULL);
						}
						_disable_agent(&easymesh_database);
						_boot_time_func_off = 10;
						if (easymesh_database.loop_detect_enable) {
							log_debug("Turn on vxd interface...");
							up_backhaul_sta(easymesh_database.radio_name_5gh, easymesh_database.radio_name_5gl, easymesh_database.radio_name_24g, primary_vid, backhaul_sta_radio_byte);
						}
					} else if (need_search_controller) {
						send_autoconfig_search_request();
					}
				} else {
					send_autoconfig_search_request();
				}

				if (_controller_found) {
					if (timer_counter % 2 == 0) {
						send_topology_query_request(map_network.controller_id);
					}
					if (timer_counter % 6 == 0 || need_update_api_output) {

						struct Device *current_device = locate_device(&map_network, info->al_mac, NEED_CREATE_FALSE, NULL);
						if (current_device) {
							uint8_t        old_distance   = current_device->physical_distance_to_controller;
							struct Device *controller_device = locate_device(&map_network, map_network.controller_id, NEED_CREATE_FALSE, NULL);
							if (NULL != controller_device) {
								optimize_neighbor(&map_network);
								optimize_datamodel(&map_network);
								topology_db_update(&map_network, controller_device, controller_device->id, 0, 0);
								reset_visited(&map_network);
								dump_dataelement(&map_network);

								if (current_device->physical_distance_to_controller != old_distance) {
									if (easymesh_database.stp_monitor_enable) {
										int priority = (current_device->physical_distance_to_controller + 1) * BRIDGE_PRIORITY_STEP;
										if (priority > MAX_BRIDGE_PRIORITY)
											priority = MAX_BRIDGE_PRIORITY;
										sprintf(cmd, "brctl setbridgeprio %s %d", easymesh_database.bridge_name, priority);
										system(cmd);
									}
									cus_status.physical_dist = current_device->physical_distance_to_controller;
									cus_status.logical_dist  = current_device->logical_distance_to_controller;
									update_customized_status_to_file(&cus_status);
									// 	update_hop_to_controller(easymesh_database.radio_name_5gh, current_device->physical_distance_to_controller);
									// 	update_hop_to_controller(easymesh_database.radio_name_5gl, current_device->physical_distance_to_controller);
									// 	update_hop_to_controller(easymesh_database.radio_name_24g, current_device->physical_distance_to_controller);
								}

								if(0 != memcmp(easymesh_api.parent_id, current_device->parent_id, 6)) {
									need_update_api_output = 1;
								}
							}

							if(need_update_api_output) {
								memcpy(easymesh_api.parent_id, current_device->parent_id, 6);
								write_api_info_file(&easymesh_api);
								need_update_api_output = 0;
							}
						}

						database_garbage_collection(&map_network, info->al_mac, 90);

						if (timer_counter >= 12) {
							//ask for topology response periodically
							send_topology_query_request(info->al_mac);
							timer_counter = 0;
						}
					}
					timer_counter++;
				}

				if (rssi_timer_counter >= 2) {
					//ask for read rssi value periodically
					if (mixed_backhaul_sta_trigger(get_backhaul_interface(), &easymesh_database)){
						set_backhaul_interface(NULL);
						controller_aliveness = 3 * controller_aliveness_max;
						need_search_controller = 1;
					}
					rssi_timer_counter = 0;
				}
				rssi_timer_counter++;

				if (single_backhaul_selection_flag) {
					if (single_backhaul_selection_delay >=1) {
						if (easymesh_database.loop_detect_enable) {
							log_trace("Down 24G backhaul sta interface in case loop occur on NON-STP mode\n");
							if (backhaul_sta_radio_byte & MASK_BAND_2G) {
								sprintf(cmd, "ifconfig %s-vxd down", easymesh_database.radio_name_24g);
								system(cmd);
							}
						}
						single_backhaul_selection_delay = 0;
						single_backhaul_selection_flag  = 0;
					}
					single_backhaul_selection_delay +=1;
				}

				if(easymesh_database.controller_interface
					&& easymesh_database.controller_al_interface
					&& !is_controller_interface_up) {
					if(is_interface_up(easymesh_database.controller_interface)) {
						char *ret;
						log_debug("Dectected controller_interface %s up, restart the daemon...\n",
							easymesh_database.controller_interface);
						system("map_reinit -a &");
						map_exit();
						pthread_join(map_thread, (void **)&ret);
					}
				}
			}

			if (vlan_configured_flag) {
				if (vlan_configure_delay >= 2) {
					if (1 == vlan_configured_flag) {
						send_vlan_configured_notification();
					} else {
						send_vlan_clear_notification();
					}
					vlan_configured_flag = 0;
					vlan_configure_delay = 0;
				}
				vlan_configure_delay += 1;
			}

			FILE *fp = fopen(VERBOSE_LEVEL_FILE, "re");
			if (fp != NULL) {
				int verbose_logging = -1;
				(void) fscanf(fp, "%d %d", &verbose_level, &verbose_logging);
				fclose(fp);
				(void) remove(VERBOSE_LEVEL_FILE);
 				map_set_verbose_level(verbose_level);
 				log_set_level(verbose_level);
				if (verbose_logging != -1)
					verbose_quiet = !verbose_logging;
				if (verbose_quiet) {
					disableLogDebug();
					map_disable_log();
				} else {
					enableLogDebug();
					map_enable_log();
				}
			}
			break;
		}
		case MAP_QUEUE_EVENT_API_COMMAND: {  /* /tmp/map_command */
			char cmd_type = buffer[3];
			char cmd_cont = buffer[4];
			uint16_t cmd_len = ((uint16_t)buffer[1] << 8) | buffer[2];

			if (cmd_len != 2)
				break;

			if (cmd_type == 'z') {
				uint8_t timer = cmd_cont - '0';

				if (timer > 0 && timer < 10 && map_agent_reinit_timer == 0) {
					map_agent_reinit_timer = timer + 7;
					sprintf(reinit_cmd, "map_reinit -t &");
				}
			}
			// enable/disable console logging
			if (cmd_type == 'v') {
				verbose_quiet = '1' - cmd_cont;
				if (verbose_quiet) {
					disableLogDebug();
					map_disable_log();
				} else {
					enableLogDebug();
					map_enable_log();
				}
			}
			break;
		}
		case MAP_QUEUE_EVENT_ONE_SECOND_TIMER: {
			struct Device *dev = locate_device_by_al_mac(&map_network, info->al_mac);

			if (status_updated) {
				update_customized_information(status_info);
				status_updated = 0;
			}

			if (map_agent_reinit_timer) {
				map_agent_reinit_timer--;
				printf("Reinit in %d second...\n", map_agent_reinit_timer);

				if (map_agent_reinit_timer == 0) {
					char *ret;

					system(reinit_cmd);
					map_exit();
					pthread_join(map_thread, (void **)&ret);
					return 0;
				}
			} else {
				if (easymesh_database.steering_opportunity_enable)
					steering_opportunity_watchdog(dev);

				if(easymesh_database.customize_features & RCPI_ROAMING_ENABLE)
					steering_rcpi_watchdog(dev);
			}

			break;
		}
		case MAP_API_RESPONSE: {
			map_message *message = map_parse_message((uint8_t *)buffer);
			switch (*message->element) {
			case MAP_GENERAL_ACKNOWLEDGEMENT_ELEMENT: {
				// log_trace("<-- MAP_GENERAL_ACKNOWLEDGEMENT_MSG\n");
				// struct map_general_acknowledgement *element = (struct map_general_acknowledgement *)message->element;
				// log_trace("Status %02x\n", element->status);
				break;
			}
			case MAP_AUTOCONFIG_RESPONSE_ELEMENT: {
				log_trace("<-- MAP_AUTOCONFIG_RESPONSE_ELEMENT\n");
				map_response_element *element = (map_response_element *)message->element;
				log_trace("tlv_nr %02x\n", element->tlv_nr);
				controller_aliveness = controller_aliveness_max;
				break;
			}
			case MAP_TOPOLOGY_RESPONSE_ELEMENT: {
				map_response_element *element = (map_response_element *)message->element;
				//log_trace("<-- MAP_TOPOLOGY_RESPONSE_MSG\n");
				//log_trace("tlv_nr %02x\n", element->tlv_nr);

				if (!memcmp(map_network.controller_id, element->src_mac, 6)) {
					controller_aliveness = controller_aliveness_max;
				}

				//--------------------------------DATA ELEMENT -------------------------------------
				struct Device *curr_device = locate_device(&map_network, element->src_mac, NEED_CREATE_TRUE, NULL);
				if (NULL == curr_device) {
					log_trace("DE - MAP_TOPOLOGY_RESPONSE_ELEMENT : Error when locating device!\n");
					break;
				}
				time(&curr_device->updated_timestamp);
				//--------------------------------DATA ELEMENT -------------------------------------

				int i = 0;
				struct deviceInformationTypeTLV *info_tlv = NULL;
				for (i = 0; i < element->tlv_nr; i++) {
					pt = element->list_of_tlvs[i];
					switch (*pt) {
					//--------------------------------DATA ELEMENT -------------------------------------
					case TLV_TYPE_NEIGHBOR_DEVICE_LIST: {
						// log_trace("<-- TLV_TYPE_NEIGHBOR_DEVICE_LIST\n");
						struct neighborDeviceListTLV *neighbor_1905 = (struct neighborDeviceListTLV *)pt;
						update_device_neighbor(map_network, curr_device, neighbor_1905, easymesh_database.customize_features);
						break;
					}
					case TLV_TYPE_AP_OPERATIONAL_BSS: {
						// log_trace("<-- TLV_TYPE_AP_OPERATIONAL_BSS\n");
						struct ApOperationalBSSTLV *operational_bss = (struct ApOperationalBSSTLV *)pt;
						update_device_operational_bss(curr_device, operational_bss);
						break;
					}
					case TLV_TYPE_ASSOCIATED_CLIENTS: {
						// log_trace("<-- TLV_TYPE_ASSOCIATED_CLIENTS\n");
						struct AssociatedClientsTLV *associated_clients = (struct AssociatedClientsTLV *)pt;
						update_device_associated_clients(map_network, curr_device, associated_clients);
						break;
					}
					case TLV_TYPE_DEVICE_INFORMATION_TYPE: {
						info_tlv = (struct deviceInformationTypeTLV *)pt;
						break;
					}
					//--------------------------------DATA ELEMENT -------------------------------------
					case TLV_TYPE_VENDOR_SPECIFIC: {
						struct vendorSpecificTLV *vendor_tlv = (struct vendorSpecificTLV *)pt;
						if (0 == memcmp(REALTEK_OUI, vendor_tlv->vendor_oui, 3)) {
							//if it is from controller
							if (!memcmp(map_network.controller_id, element->src_mac, 6)) {
								char *ip_addr        = (char *)vendor_tlv->m;
								char *device_name    = (char *)(ip_addr + strlen((char *)ip_addr) + 1);
								char *device_version = "EasyMesh V1.0";
#if defined(BACKHAUL_REAL_TIME_SWITCH) || defined(EASYMESH_RENEW_IP)
								snprintf(controller_ip, sizeof(controller_ip), "%s", ip_addr);
#endif
								if (strlen((char *)ip_addr) + strlen(device_name) + 2 < vendor_tlv->m_nr) {
									device_version = (char *)(device_name + strlen(device_name) + 1);
								}
								controller_version = convertVersionStringToNumber(device_version);
							}
						}
						break;
					}
					// default: {
					// log_debug("Unexpected TLV (%02x) type inside CMDU\n", *pt);
					// break;
					// }
					}
				}

				if (info_tlv) {
					update_device_info(curr_device, info_tlv, easymesh_database.radio_number, &map_network);
				}

				optimize_datamodel(&map_network);

				if(easymesh_database.controller_interface
					&& easymesh_database.controller_al_interface
					&& is_controller_interface_up) {
					if (!memcmp(map_network.controller_id, element->src_mac, 6)) {
						controller_aliveness = controller_aliveness_max;
					}
				}

				break;
			}
			case MAP_BACKHAUL_STEERING_RESPONSE_ELEMENT: {
				map_response_element *element = (map_response_element *)message->element;
				log_trace("<-- MAP_BACKHAUL_STEERING_RESPONSE_MSG\n");
				log_trace("tlv_nr %02x\n", element->tlv_nr);
				controller_aliveness = controller_aliveness_max;
				break;
			}
			case MAP_LINK_METRIC_RESPONSE_ELEMENT: {
				map_response_element *element = (map_response_element *)message->element;
				log_trace("<-- MAP_LINK_METRIC_RESPONSE_MSG\n");
				log_trace("tlv_nr %02x\n", element->tlv_nr);
				if (!memcmp(map_network.controller_id, element->src_mac, 6)) {
					controller_aliveness = controller_aliveness_max;
				}
				break;
			}
			case MAP_AP_CAPABILITY_REPORT_ELEMENT: {
				map_response_element *element = (map_response_element *)message->element;
				log_trace("<-- MAP_AP_CAPABILITY_RESPONSE_MSG\n");
				log_trace("tlv_nr %02x\n", element->tlv_nr);
				if (!memcmp(map_network.controller_id, element->src_mac, 6)) {
					controller_aliveness = controller_aliveness_max;
				}
				break;
			}
			case MAP_CLIENT_CAPABILITY_REPORT_ELEMENT: {
				map_response_element *element = (map_response_element *)message->element;
				log_trace("<-- MAP_CLIENT_CAPABILITY_REPORT_ELEMENT\n");
				log_trace("tlv_nr %02x\n", element->tlv_nr);
				if (!memcmp(map_network.controller_id, element->src_mac, 6)) {
					controller_aliveness = controller_aliveness_max;
				}
				break;
			}
			case MAP_AP_METRIC_RESPONSE_ELEMENT: {
				map_response_element *element = (map_response_element *)message->element;
				log_trace("<-- MAP_AP_METRIC_RESPONSE_MSG\n");
				log_trace("tlv_nr %02x\n", element->tlv_nr);
				if (!memcmp(map_network.controller_id, element->src_mac, 6)) {
					controller_aliveness = controller_aliveness_max;
				}
				break;
			}
			case MAP_ASSOC_STA_METRIC_RESPONSE_ELEMENT: {
				int i = 0;
				struct Device *dev = NULL;
				struct Radio *radio = NULL;
				struct StructBSS *bss = NULL;
				struct STA *station = NULL;
				map_response_element *element = (map_response_element *)message->element;

				log_trace("<-- MAP_ASSOC_STA_METRIC_RESPONSE_MSG\n");
				log_trace("tlv_nr %02x\n", element->tlv_nr);

				/* only process myself device */
				if (memcmp(info->al_mac, element->src_mac, 6))
					break;

				if (NULL == element->list_of_tlvs) {
					log_warn("Malformed structure.");
					break;
				}

				for (i = 0; i < element->tlv_nr; i++) {
					pt = element->list_of_tlvs[i];
					switch (*pt) {
					case TLV_TYPE_ASSOCIATED_STA_LINK_METRICS: {
						struct AssociatedSTALinkMetricsTLV *t = (struct AssociatedSTALinkMetricsTLV *)pt;

						if (t->bssid_nr) {
							dev = locate_device_by_al_mac(&map_network, info->al_mac);
							if (NULL == dev)
								break;

							radio = locate_radio(dev, NULL, 0, t->assoc_sta_link_metrics[0].bssid, NEED_CREATE_FALSE, NULL);

							if (NULL == radio)
								break;

							bss = locate_bss(radio, t->assoc_sta_link_metrics[0].bssid, NEED_CREATE_FALSE, NULL);
							if (NULL == bss)
								break;

							station = locate_sta(bss, t->mac_address, NEED_CREATE_FALSE, NULL);
							if (NULL == station)
								break;

							update_sta(station, t->assoc_sta_link_metrics, NULL, NULL);
						}
						break;
					}
					default: {
						break;
					}
					}
				}
				break;
			}
			case MAP_UNASSOC_STA_METRIC_RESPONSE_ELEMENT: {
				map_response_element *element = (map_response_element *)message->element;
				log_trace("<-- MAP_UNASSOC_STA_METRIC_RESPONSE_MSG\n");
				log_trace("tlv_nr %02x\n", element->tlv_nr);
				break;
			}
			case MAP_BEACON_METRIC_RESPONSE_ELEMENT: {
				int i = 0;
				map_response_element *element = (map_response_element *)message->element;
				log_trace("<-- MAP_BEACON_METRIC_RESPONSE_MSG\n");
				log_trace("tlv_nr %02x\n", element->tlv_nr);

				if (NULL == element->list_of_tlvs) {
					log_trace("Malformed structure.");
					break;
				}

				for (i = 0; i < element->tlv_nr; i++) {
					pt = element->list_of_tlvs[i];
					switch (*pt) {
					case TLV_TYPE_BEACON_METRICS_RESPONSE: {
						struct BeaconMetricsResponseTLV *t = (struct BeaconMetricsResponseTLV *)pt;
						struct Device *dev = locate_device_by_al_mac(&map_network, info->al_mac);
						struct StructBSS *bss = NULL;
						/*struct STA *sta =*/
						locate_sta_by_mac(NULL, dev, t->mac_address, NULL, NULL, &bss);

						if (easymesh_database.steering_opportunity_enable)
							steering_opportunity_collect_beacon_report(&map_network, bss, pt);

						if(easymesh_database.customize_features & RCPI_ROAMING_ENABLE)
							steering_rcpi_collect_beacon_report(&map_network, bss, pt);
					}
					default: {
						break;
					}
					}
				}
				break;
			}
			case MAP_CHANNEL_PREFERENCE_REPORT_ELEMENT: {
				map_response_element *element = (map_response_element *)message->element;
				log_trace("<-- MAP_CHANNEL_PREFERENCE_REPORT_MSG\n");
				log_trace("tlv_nr %02x\n", element->tlv_nr);
				break;
			}
			case MAP_CHANNEL_SELECTION_RESPONSE_ELEMENT: {
				map_response_element *element = (map_response_element *)message->element;
				log_trace("<-- MAP_CHANNEL_SELECTION_RESPONSE_MSG\n");
				log_trace("tlv_nr %02x\n", element->tlv_nr);
				break;
			}
			case MAP_OPERATING_CHANNEL_REPORT_ELEMENT: {
				map_response_element *element = (map_response_element *)message->element;
				log_trace("<-- MAP_OPERATING_CHANNEL_REPORT_MSG\n");
				log_trace("tlv_nr %02x\n", element->tlv_nr);

				//---------CHANNEL SELECTION--------------
				int i = 0, j = 0;
				if (NULL == element->list_of_tlvs) {
					log_trace("Malformed structure.");
					break;
				}

				uint8_t dummy_addr[6] = { 0 };
				if (!memcmp(element->src_mac, dummy_addr, 6)) { //check if it is self-signal
					for (i = 0; i < element->tlv_nr; i++) {
						pt = element->list_of_tlvs[i];
						switch (*pt) {
						case TLV_TYPE_OPERATING_CHANNEL_REPORT: {
							struct OperatingChannelReportTLV *t = (struct OperatingChannelReportTLV *)pt;

							int bandwidth = BAND_WIDTH_20MHZ, sideband = SIDE_BAND_NONE;
							int new_bandwidth = BAND_WIDTH_20MHZ, new_sideband = SIDE_BAND_NONE;
							uint8_t band = RADIO_BAND_UNKNOWN, channel = 0;

							for (j = 0; j < t->cur_op_class_nr; j++) {
								getOpClassBandInfo(t->operating_channels[j].op_class, &bandwidth, &sideband);
								log_debug("Op class: %d, bandwidth: %d, sideband: %d\n", t->operating_channels[j].op_class, bandwidth, sideband);
								if (bandwidth > new_bandwidth) {
									new_bandwidth = bandwidth;
								}
								if (sideband != SIDE_BAND_NONE && new_sideband == SIDE_BAND_NONE) {
									new_sideband = sideband;
								}
								if (BAND_WIDTH_20MHZ == bandwidth) {
									band    = get_band_from_op_class(t->operating_channels[j].op_class);
									channel = t->operating_channels[j].cur_channel;
								}
							}

							if (channel) {
								set_radio_channel(band, channel, new_bandwidth, new_sideband, &easymesh_database);
							}

							if (0 == strcmp(easymesh_database.radio_name_5gh, easymesh_database.radio_name_5gl) && RADIO_BAND_5GH == band) {
								band = RADIO_BAND_5GL;
							}
							//
							if (controller_version != 0 && (controller_version < old_release || controller_version == old_beta_release)) {
								sprintf(cmd, "map_reinit -b %d_%d &", band, channel);
								log_debug("CHANNEL SELECTION : Channel AP Mib Set %d.\n", channel);
							} else {
								if (SIDE_BAND_NONE == new_sideband) {
									new_sideband = -1;
								} else if (SIDE_BAND_UPPER == new_sideband) {
									new_sideband = 0;
								} else if (SIDE_BAND_LOWER == new_sideband) {
									sideband = 1;
								} else {
									log_warn("Invalid sideband %d\n", new_sideband);
									break;
								}
								sprintf(cmd, "map_reinit -b %d_%d_%d_%d &", band, channel, new_bandwidth, new_sideband);
								log_debug("CHANNEL SELECTION : Channel AP Mib Set channel %d, bandwidth %d, sideband %d\n", channel, new_bandwidth, new_sideband);
							}
							system(cmd);
							//
							break;
						}
						default: {
							// log_warn("Unexpected TLV (%02x) type inside CMDU\n", *pt);
							break;
						}
						}
					}
					//------R1 DFS------------
					// buffer_mib_channel(&easymesh_database);
					//------------------------
				}

				//---------CHANNEL SELECTION--------------
				break;
			}
			case MAP_CLIENT_STEERING_REQUEST_OPPORTUNITY_ELEMENT: {
				if (easymesh_database.steering_opportunity_enable) {
					struct map_client_steering_request *req
					    = (struct map_client_steering_request *)message->element;

					struct steering_request *t     = &(req->steering_req);
					struct Device           *dev   = locate_device_by_al_mac(&map_network, info->al_mac);
					struct Radio            *radio = NULL;
					struct StructBSS        *bss   = NULL;

					radio = locate_radio(dev, NULL, 0, t->bssid, NEED_CREATE_FALSE, NULL);
					if (radio == NULL)
						break;

					bss = locate_bss(radio, t->bssid, NEED_CREATE_FALSE, NULL);
					steering_opportunity_init(dev, bss, (uint8_t *)t);
				}

				break;
			}
			case MAP_CLIENT_STEERING_BTM_REPORT_ELEMENT: {
				map_response_element *element = (map_response_element *)message->element;
				log_trace("<-- MAP_CLIENT_STEERING_BTM_REPORT_MSG\n");
				log_trace("tlv_nr %02x\n", element->tlv_nr);
				break;
			}
			case MAP_STEERING_COMPLETE_ELEMENT: {
				map_response_element *element = (map_response_element *)message->element;
				log_trace("<-- MAP_STEERING_COMPLETE_MSG\n");
				log_trace("tlv_nr %02x\n", element->tlv_nr);
				break;
			}
			case MAP_TOPOLOGY_NOTIFICATION_ELEMENT: {
				map_response_element *element = (map_response_element *)message->element;
				log_trace("<-- MAP_TOPOLOGY_NOTIFICATION_ELEMENT\n");
				log_trace("tlv_nr %02x\n", element->tlv_nr);
				break;
			}
			case MAP_PUSH_BUTTON_NOTIFICATION_ELEMENT: {
				log_trace("<-- MAP_PUSH_BUTTON_NOTIFICATION_ELEMENT\n");
				break;
			}
			case MAP_CONTROLLER_CHANGE_NOTIFICATION_ELEMENT: {
				log_trace("<-- MAP_CONTROLLER_CHANGE_NOTIFICATION_ELEMENT\n");
				if (!wfa_test) {
					struct map_controller_change_element *element = (struct map_controller_change_element *)message->element;
					if (CONTROLLER_LOST == element->event_type) {
						log_debug("CONTROLLER_LOST\n");
						status_updated = update_status(&status_info.current_status, STATUS_CONTROLLER_LOST);
						if (easymesh_database.stp_monitor_enable) {
							sprintf(cmd, "brctl setbridgeprio %s %d", easymesh_database.bridge_name, MAX_BRIDGE_PRIORITY);
							system(cmd);
						}
						if (backhaul_uplink_status == MAP_LINK_UP) {
							backhaul_uplink_status = MAP_LINK_DOWN;
							update_backhaul_link_status(backhaul_uplink_status, NULL);
						}
						_disable_agent(&easymesh_database);
						if (easymesh_database.loop_detect_enable) {
							log_debug("Turn on vxd interface...");
							up_backhaul_sta(easymesh_database.radio_name_5gh, easymesh_database.radio_name_5gl, easymesh_database.radio_name_24g, primary_vid, backhaul_sta_radio_byte);
						}
					} else if (CONTROLLER_FOUND == element->event_type) {
						log_debug("CONTROLLER_FOUND from %s\n", element->receiving_interface);
						if (backhaul_sta_radio_byte >= 3) {
							set_backhaul_interface(element->receiving_interface);//store vxd interface
						}
						need_search_controller = 0;

						if (!easymesh_database.stp_monitor_enable) {
							if (easymesh_database.loop_detect_enable) {
								log_debug("Down off vxd interface in case of loop\n");
								if (element->receiving_interface[4] == easymesh_database.radio_name_5gl[4]) {
									if (backhaul_sta_radio_byte & MASK_BAND_2G) {
										log_debug("Down 24G Vxd\n");
										sprintf(cmd, "ifconfig %s-vxd down", easymesh_database.radio_name_24g);
										system(cmd);
									}
								} else if (element->receiving_interface[4] == easymesh_database.radio_name_24g[4]) {
									if (backhaul_sta_radio_byte & MASK_BAND_5GL) {
										log_debug("Down 5GL Vxd\n");
										sprintf(cmd, "ifconfig %s-vxd down", easymesh_database.radio_name_5gl);
										system(cmd);
									} else if (backhaul_sta_radio_byte & MASK_BAND_5GH) {
										log_debug("Down 5GH Vxd\n");
										sprintf(cmd, "ifconfig %s-vxd down", easymesh_database.radio_name_5gh);
										system(cmd);
									}
								}
							}
						}

#if defined(BACKHAUL_REAL_TIME_SWITCH) || defined(EASYMESH_RENEW_IP)
						backhaul_uplink_count += 1;
						snprintf(cmd, sizeof(cmd), "echo %s %d %s> %s", element->receiving_interface, backhaul_uplink_count, controller_ip, BACKHAUL_LINK_INFO);
						system(cmd);
#endif
						if (backhaul_uplink_status == MAP_LINK_DOWN || strcmp(element->receiving_interface, backhaul_receiving_interface)) {
							backhaul_uplink_status = MAP_LINK_UP;
							update_backhaul_link_status(backhaul_uplink_status, element->receiving_interface);
							strlcpy(backhaul_receiving_interface, element->receiving_interface, sizeof(backhaul_receiving_interface));
						}
						memcpy(map_network.controller_id, element->controller_al_mac, 6);
						_enable_agent(&easymesh_database);

						if (get_valid_interface_name(element->receiving_interface, VALID_ETH_INTERFACES_IN_BRIDGE)
							|| (easymesh_database.controller_interface && strstr(element->receiving_interface, easymesh_database.controller_interface))) {
							if (easymesh_database.loop_detect_enable) {
								log_debug("Controller is found on %s, turn off vxd.\n", element->receiving_interface);
								if (backhaul_sta_radio_byte & MASK_BAND_5GH) {
									sprintf(cmd, "ifconfig %s-vxd down", easymesh_database.radio_name_5gh);
									system(cmd);
								}

								if (backhaul_sta_radio_byte & MASK_BAND_5GL) {
									sprintf(cmd, "ifconfig %s-vxd down", easymesh_database.radio_name_5gl);
									system(cmd);
								}

								if (backhaul_sta_radio_byte & MASK_BAND_2G) {
									sprintf(cmd, "ifconfig %s-vxd down", easymesh_database.radio_name_24g);
									system(cmd);
								}
							}
						}
						if (!easymesh_database.stp_monitor_enable) {
							if (is_searching_controller) {
								is_searching_controller = 0;
								if (get_valid_interface_name(element->receiving_interface, VALID_ETH_INTERFACES_IN_BRIDGE)
								    || (easymesh_database.controller_interface && strstr(element->receiving_interface, easymesh_database.controller_interface))) {
									if (easymesh_database.loop_detect_enable) {
										log_debug("Controller is found on %s, end search.\n", element->receiving_interface);
										sprintf(cmd, "iwpriv wlan%d-vxd vxd_disassoc; ifconfig wlan%d-vxd down", backhaul_sta_radio_index, backhaul_sta_radio_index);
										system(cmd);
									}
									end_eth_monitor(BACKHAUL_ETH);
									break;
								}

								if (vxd_controller_flag) {
									if (is_interface_up(element->receiving_interface)) {
										log_debug("Controller is found on vxd again, end search.\n");
										end_eth_monitor(BACKHAUL_WIFI);
										break;
									}
									log_debug("Controller is found on vxd again when it's off, ignore.\n");
								}

								vxd_controller_flag = 1;
								if (1 == is_any_eth_connected(eth_interface_nr, eth_interface_names, NULL)) {
									if (easymesh_database.loop_detect_enable) {
										log_debug("Controller is found on vxd, eth is connected, search controller on eth again.\n");
										sprintf(cmd, "iwpriv wlan%d-vxd vxd_disassoc; ifconfig wlan%d-vxd down", backhaul_sta_radio_index, backhaul_sta_radio_index);
										system(cmd);
									}
									_disable_agent(&easymesh_database);
									resume_eth_monitor();
									is_searching_controller = 1;
									controller_search_count = MAX_SEARCH_TIME;
								} else {
									if (!is_interface_up(element->receiving_interface)) {
										if (easymesh_database.loop_detect_enable) {
											log_debug("Controller is found on vxd but interface down, up vxd and end search.\n");
											snprintf(cmd, sizeof(cmd), "ifconfig wlan%d-vxd up", backhaul_sta_radio_index);
											system(cmd);
										}
									}
									log_debug("Controller is found on vxd, eth is not connected, end search.\n");
									end_eth_monitor(BACKHAUL_WIFI);
								}
							} else if (-1 == access(BACKHAUL_LINK_FLAG_PATH, F_OK)
							           || (get_valid_interface_name(element->receiving_interface, VALID_ETH_INTERFACES_IN_BRIDGE)
							               || (easymesh_database.controller_interface && strstr(element->receiving_interface, easymesh_database.controller_interface)))) {
								if (get_valid_interface_name(element->receiving_interface, VALID_ETH_INTERFACES_IN_BRIDGE)
								    || (easymesh_database.controller_interface && strstr(element->receiving_interface, easymesh_database.controller_interface))) {
#ifdef BACKHAUL_REAL_TIME_SWITCH
									char interface_name[20] = { 0 };
									snprintf(interface_name, sizeof(interface_name), "wlan%d-vxd", backhaul_sta_radio_index);
									if (is_interface_up(interface_name)) {
										if (easymesh_database.loop_detect_enable) {
											snprintf(cmd, sizeof(cmd), "iwpriv wlan%d-vxd vxd_disassoc; ifconfig wlan%d-vxd down", backhaul_sta_radio_index, backhaul_sta_radio_index);
											system(cmd);
										}
									}
#endif
									end_eth_monitor(BACKHAUL_ETH);
								} else {
									if (!is_interface_up(element->receiving_interface)) {
										if (easymesh_database.loop_detect_enable) {
											log_info("Controller is receive on vxd but interface down, up vxd and end search.\n");
											snprintf(cmd, sizeof(cmd), "ifconfig wlan%d-vxd up", backhaul_sta_radio_index);
											system(cmd);
										}
									}
									end_eth_monitor(BACKHAUL_WIFI);
								}
							}
						}
					}
					if(easymesh_api.is_controller_found != element->event_type || memcmp(easymesh_api.backhaul_interface, element->receiving_interface,5)) {
						easymesh_api.is_controller_found = element->event_type;
						if(CONTROLLER_LOST == element->event_type) {
							write_api_info_file(&easymesh_api);
							if(easymesh_api.backhaul_interface != NULL) {
								free(easymesh_api.backhaul_interface);
								easymesh_api.backhaul_interface = NULL; //Incase double free codedump
							}
						}
						if(CONTROLLER_FOUND == element->event_type) {
							need_update_api_output = 1;
							if(easymesh_api.backhaul_interface != NULL) {
								free(easymesh_api.backhaul_interface);
							}
							easymesh_api.backhaul_interface = strdup(element->receiving_interface);
						}
					}
				}
				break;
			}
			case MAP_CUSTOMIZED_STATUS_NOTIFICATION_ELEMENT: {
				struct map_customized_status_notification *element = (struct map_customized_status_notification *)message->element;
				uint8_t next_status =0;
				next_status                                        = element->customized_status;
				status_updated = update_status(&status_info.current_status, next_status);
				break;
			}
			case MAP_DPP_CONFIGURATION_ELEMENT: {
				struct map_dpp_configuration *element = (struct map_dpp_configuration *)message->element;
				log_debug("1905 signed connector: %s\n", element->signed_connector_1905);
				if(easymesh_database.signed_connector_1905 != NULL) {
					free(easymesh_database.signed_connector_1905);
				}
				easymesh_database.signed_connector_1905 = strdup(element->signed_connector_1905);
				log_debug("netaccess_key: %s\n", element->netaccess_key);
				if(easymesh_database.netaccess_key != NULL) {
					free(easymesh_database.netaccess_key);
				}
				easymesh_database.netaccess_key = strdup(element->netaccess_key);
				log_debug("csign_key: %s\n", element->csign_key);
				if(easymesh_database.csign_key != NULL) {
					free(easymesh_database.csign_key);
				}
				easymesh_database.csign_key = strdup(element->csign_key);
				if (element->pp_key) {
					log_debug("pp_key: %s\n", element->pp_key);
					if(easymesh_database.pp_key != NULL) {
						free(easymesh_database.pp_key);
					}
					easymesh_database.pp_key = strdup(element->pp_key);
				}
				break;
			}
			case MAP_AUTOCONFIG_RENEW_NOTIFICATION_ELEMENT: {
				log_trace("<-- MAP_AUTOCONFIG_RENEW_NOTIFICATION_ELEMENT\n");
				easymesh_database.configured_band = 0;
				//
				system("map_reinit -c0 &");
				//
				identical_state = 0;
				controller_aliveness = controller_aliveness_max;
				break;
			}
			case MAP_VLAN_APPLY_NOTIFICATION_ELEMENT: {
				log_trace("<-- MAP_VLAN_APPLY_NOTIFICATION_ELEMENT\n");
				int i = 0;
				if (vlan_config_nr) {
					FILE *fp = fopen("/tmp/map_vlan_setting", "we");
					fprintf(fp, "Primary %d\n", primary_vid);
					for (i = 0; i < vlan_config_nr; i++) {
						log_debug("VLAN %d %s %d\n", i, vlan_config[i].ssid, vlan_config[i].vid);
						fprintf(fp, "%s %d\n", vlan_config[i].ssid, vlan_config[i].vid);
					}
					fclose(fp);
					system("map_vlan_config");
					vlan_configured_flag = 1;
					vlan_configure_delay = 0;
				} else {
					system("sh /tmp/map_vlan_reset");
					system("rm /tmp/map_vlan_reset");
					system("rm /tmp/map_vlan_need_reset");
					vlan_configured_flag = 2;
					vlan_configure_delay = 0;
					log_debug("Reset VLAN setting\n");
				}
				break;
			}
			case MAP_VLAN_CONFIGURATION_ELEMENT: {
				log_trace("<-- MAP_VLAN_CONFIGURATION_ELEMENT\n");
				map_response_element *element = (map_response_element *)message->element;
				int                   i = 0, j = 0;
				int                   vlan_ssid_nr              = 0;
				int                   is_traffic_separation_tlv = 0;
				for (i = 0; i < element->tlv_nr; i++) {
					pt = element->list_of_tlvs[i];
					switch (*pt) {
					case TLV_TYPE_TRAFFIC_SEPARATION_POLICY: {
						is_traffic_separation_tlv            = 1;
						struct TrafficSeparationPolicyTLV *t = (struct TrafficSeparationPolicyTLV *)pt;
						vlan_ssid_nr += t->ssid_nr;
						if (t->ssid_nr) {
							for (j = 0; j < t->ssid_nr; j++) {
								append_vlan_config(&vlan_config, &vlan_config_nr, t->ssid_info[j].ssid_name, t->ssid_info[j].ssid_length, t->ssid_info[j].vlan_id);
							}
						}
						break;
					}
					case TLV_TYPE_DEFAULT_802_1Q_SETTINGS: {
						struct Default8021QSettingsTLV *t = (struct Default8021QSettingsTLV *)pt;
						log_debug("Default VID %d\n", t->primary_vlan_id);
						primary_vid = t->primary_vlan_id;
						break;
					}
					}
				}

				if (0 == vlan_ssid_nr && is_traffic_separation_tlv) {
					for (i = 0; i < vlan_config_nr; i++) {
						free(vlan_config[i].ssid);
					}
					free(vlan_config);
					vlan_config    = NULL;
					vlan_config_nr = 0;
				}
				break;
			}
			case MAP_CONFIGURE_NOTIFICATION_ELEMENT: {
				log_trace("<-- MAP_CONFIGURE_NOTIFICATION_ELEMENT\n");
				struct map_configure_notification *element = (struct map_configure_notification *)message->element;
				struct radio_config_data *         radio_config = NULL;
				int vendor_data_nr;
				int vendor_data_idx;
				int ssid_bss_index_mapping_tlv = 0;
				int backhaul_config_nr = 0;

				radio_config = malloc(element->config_nr * sizeof(struct radio_config_data));
				for (i = 0; i < element->config_nr; i++) {
					radio_config[i].radio_band  = element->config_data[i].radio_band;
					radio_config[i].bss_data_nr = element->config_data[i].bss_data_nr;
					radio_config[i].bss_data    = element->config_data[i].bss_data;
					for (j = 0; j < radio_config[i].bss_data_nr; j++) {
						radio_config[i].bss_data[j].bss_index = j;
					}
					//vendor auto
					vendor_data_nr = element->config_data[i].vendor_data_nr;
					if (element->config_data[i].vendor_data_nr) {
						for (j = 0; j < element->config_data[i].vendor_data_nr; j++) {
							if (!memcmp(element->config_data[i].vendor_datas[j].vendor_oui, REALTEK_OUI, 3) && element->config_data[i].vendor_datas[j].payload[0] == SSID_BSS_INDEX_MAPPING) {
								_extract_bss_index_from_vendor_data(&(element->config_data[i].vendor_datas[j]), &(radio_config[i]));
								vendor_data_nr--;
								ssid_bss_index_mapping_tlv = 1;
							}
						}
						if (vendor_data_nr == 0) {
							break;
						}
						easymesh_database.config_data_nr += 1;
						easymesh_database.per_radio_config                                                      = (struct radio_config_data *)realloc(easymesh_database.per_radio_config, easymesh_database.config_data_nr * sizeof(struct radio_config_data));
						easymesh_database.per_radio_config[easymesh_database.config_data_nr - 1].vendor_datas   = (struct vendor_specific_data *)malloc(vendor_data_nr * sizeof(struct vendor_specific_data));
						easymesh_database.per_radio_config[easymesh_database.config_data_nr - 1].radio_band     = radio_config[i].radio_band;
						easymesh_database.per_radio_config[easymesh_database.config_data_nr - 1].vendor_data_nr = vendor_data_nr;
						//easymesh_database.per_radio_config[easymesh_database.config_data_nr - 1].vendor_datas = element->vendor_datas;
						vendor_data_idx = 0;
						for (j = 0; j < element->config_data[i].vendor_data_nr; j++) {
							if (!memcmp(element->config_data[i].vendor_datas[j].vendor_oui, REALTEK_OUI, 3) && element->config_data[i].vendor_datas[j].payload[0] == SSID_BSS_INDEX_MAPPING) {
								continue;
							}
							easymesh_database.per_radio_config[easymesh_database.config_data_nr - 1].vendor_datas[vendor_data_idx].payload_len = element->config_data[i].vendor_datas[j].payload_len;
							easymesh_database.per_radio_config[easymesh_database.config_data_nr - 1].vendor_datas[vendor_data_idx].payload     = (unsigned char *)strdup((char *)element->config_data[i].vendor_datas[j].payload);
							memcpy(easymesh_database.per_radio_config[easymesh_database.config_data_nr - 1].vendor_datas[vendor_data_idx].vendor_oui, element->config_data[i].vendor_datas[j].vendor_oui, 3);
							have_vendor_data++;
							vendor_data_idx++;
						}
					}
					//vendor auto
					if (!ssid_bss_index_mapping_tlv && !wfa_test) {
						for (j = 0; j < radio_config[i].bss_data_nr; j++) {
							if (MULTI_AP_BACKHAUL_BSS_BIT & radio_config[i].bss_data[j].network_type) {
								backhaul_config_nr++;
							}
						}
						if (backhaul_config_nr != 1) {
							continue;
						}
						_set_backhaul_config_bss_index(&(radio_config[i]), easymesh_database.backhaul_bss_index);
					}
				}

				uint8_t result = configureDevice(wfa_test, element->config_nr, radio_config, &identical_state, &easymesh_database, &vlan_config, &vlan_config_nr);

				if (2 == result) {
					need_full_reload = 1;
				}
				//get_configure_state();
				configure_state = easymesh_database.configured_band;
				if (MAP_CONFIGURE_BAND_FULL == configure_state) {
					log_debug("Fully configured!!\n");
					status_updated = update_status(&status_info.current_status, STATUS_AUTOCONFIG_DONE);
					write_set_mib_config_file(&easymesh_database);
					if (status_updated) {
						update_customized_information(status_info);
						status_updated = 0;
					}
					if (-1 != access("/tmp/map_vlan_reset", F_OK)) {
						identical_state = 0;
						system("echo 1 > /tmp/map_vlan_need_reset");
					}
					if (vlan_config_nr) {
						FILE *fp = fopen("/tmp/map_vlan_setting", "we");
						fprintf(fp, "Primary %d\n", primary_vid);
						for (i = 0; i < vlan_config_nr; i++) {
							log_debug("VLAN %d %s %d\n", i, vlan_config[i].ssid, vlan_config[i].vid);
							fprintf(fp, "%s %d\n", vlan_config[i].ssid, vlan_config[i].vid);
						}
						fclose(fp);
						identical_state = 0;
					}
					if ((configure_state == identical_state) && (have_vendor_data == 0)) {
						log_debug("All config received is identical to the original, skip re-init!!\n");
						char command[64];
						sprintf(command, "map_reinit -c%d &", configure_state);
						system(command);
					} else if ((configure_state == identical_state) && (have_vendor_data)) {
						log_debug("Vendor data Reload!\n");
						system("map_reinit -v &");
					} else if (easymesh_database.controller_interface && easymesh_database.controller_al_interface) {
						if (need_full_reload) {
							log_debug("Full Reload!\n");
							sprintf(reinit_cmd, "map_reinit -f &");
						} else {
							log_debug("Soft Reload!\n");
							sprintf(reinit_cmd, "map_reinit -l &");
						}
						have_vendor_data = 0;
						map_agent_reinit_timer = 4;
					} else {
						char *ret;
						if (need_full_reload) {
							log_debug("Full Reload!\n");
							system("map_reinit -f &");
						} else {
							log_debug("Soft Reload!\n");
							system("map_reinit -l &");
						}
						have_vendor_data = 0;
						map_exit();
						pthread_join(map_thread, (void **)&ret);
						if(easymesh_api.backhaul_interface != NULL) {
							free(easymesh_api.backhaul_interface);
							easymesh_api.backhaul_interface = NULL;
						}
						return 0;
					}
					have_vendor_data = 0;
				}
				if (radio_config) {
					free(radio_config);
				}
				controller_aliveness = controller_aliveness_max;
				break;
			}
			case MAP_1905_VENDOR_SPECIFIC_CMDU_ELEMENT: {
				log_trace("<-- MAP_1905_VENDOR_SPECIFIC_CMDU_ELEMENT\n");
				map_response_element *element = (map_response_element *)message->element;
				log_trace("tlv_nr %02x\n", element->tlv_nr);
				int i = 0;
				for (i = 0; i < element->tlv_nr; i++) {
					pt = element->list_of_tlvs[i];
					switch (*pt) {
					case TLV_TYPE_VENDOR_SPECIFIC: {
						// log_trace("<-- TLV_TYPE_VENDOR_SPECIFIC\n");
						process_rtk_vendor_spec_tlv((struct vendorSpecificTLV *)pt, easymesh_database.radio_data_nr, easymesh_database.radio_data, easymesh_database.vap_prefix);
						break;
					}
					default: {
						log_warn("<-- Unknown TLV\n");
					}
					}
				}
				if (!memcmp(map_network.controller_id, element->src_mac, 6)) {
					controller_aliveness = controller_aliveness_max;
				}
				break;
			}
			case MAP_POLICY_CONFIG_NOTIFICATION_ELEMENT: {
				log_trace("<-- MAP_POLICY_CONFIG_NOTIFICATION_ELEMENT\n");
				map_response_element *element = (map_response_element *)message->element;
				log_trace("tlv_nr %02x\n", element->tlv_nr);
				int i = 0, j = 0;
				for (i = 0; i < element->tlv_nr; i++) {
					pt = element->list_of_tlvs[i];
					switch (*pt) {
					case TLV_TYPE_STEERING_POLICY: {
						log_trace("<-- TLV_TYPE_STEERING_POLICY\n");
						struct SteeringPolicyTLV *tlv = (struct SteeringPolicyTLV *)pt;

						struct Device *dev = locate_device_by_al_mac(&map_network, info->al_mac);

						if(easymesh_database.customize_features & RCPI_ROAMING_ENABLE)
							steering_rcpi_init(dev, (uint8_t *)tlv);

						break;
					}
					case TLV_TYPE_METRIC_REPORT_POLICY: {
						log_trace("<-- TLV_TYPE_METRIC_REPORT_POLICY\n");
						break;
					}
					case TLV_TYPE_DEFAULT_802_1Q_SETTINGS: {
						log_trace("<-- TLV_TYPE_DEFAULT_802_1Q_SETTINGS\n");
						struct Default8021QSettingsTLV *tlv = (struct Default8021QSettingsTLV *)pt;
						log_detail("PVID: %d\n", tlv->primary_vlan_id);
						log_detail("Default pcp: %d\n", tlv->default_pcp);
						break;
					}
					case TLV_TYPE_TRAFFIC_SEPARATION_POLICY: {
						log_trace("<-- TLV_TYPE_TRAFFIC_SEPARATION_POLICY\n");
						struct TrafficSeparationPolicyTLV *trafficSeparationPolicy = (struct TrafficSeparationPolicyTLV *)pt;
						log_detail("Ssid nr: %d\n", trafficSeparationPolicy->ssid_nr);
						for (j = 0; j < trafficSeparationPolicy->ssid_nr; j++) {
							log_detail("Ssid length: %d\n", trafficSeparationPolicy->ssid_info[j].ssid_length);
							log_detail("Ssid name: %s\n", trafficSeparationPolicy->ssid_info[j].ssid_name);
							log_detail("vlan id: %d\n", trafficSeparationPolicy->ssid_info[j].vlan_id);
						}
						break;
					}
					default: {
						log_warn("<-- Unknown TLV\n");
					}
					}
				}
				controller_aliveness = controller_aliveness_max;
				break;
			}
			default: {
			}
			}
			map_free_element(message->element);
			free(message);
			message = NULL;
			break;
		}
		default: {
			log_warn("Unexpected message received...\n");
			continue;
		}
		}
	}

	return 0;
}
