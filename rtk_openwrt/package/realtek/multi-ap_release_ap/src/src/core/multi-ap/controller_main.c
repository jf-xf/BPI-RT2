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
#include <pthread.h>
#include <sys/inotify.h>
#include <poll.h>
#include <signal.h>

#include "ini.h"

//#include "map_initialization.h"
#include "map_utils.h"
#include "map_commands.h"
#include "map_tlvs.h"

#include "easymesh_utils.h"
#include "config_file_handler.h"

#include "map_timer.h"
#include "map_dataelement.h"
#include "map_pathsel.h"
#include "map_clientsteer.h"
#include "map_logger.h"
#include "./customized/map_vendor_specific_data.h"
#define PATHSEL 1
#define CLIENT_STEERING 1

#define MANUAL_CLIENT_STEER 1
#define QUEUE_NAME "/map_queue"
#define CONFIG_FILE_PATH "/tmp/multiap_config.ini"

#define MAX_MESSAGE_SIZE 12288

#define MAX_RESEND_TIME 3

#define AUTOCONFIG_RENEW_WAIT_TIMEOUT 10

#ifndef O_CLOEXEC
#define O_CLOEXEC	02000000	/* set close_on_exec */
#endif

uint8_t _check_renew_ack(struct Network network, uint8_t renew_threshold)
{
	uint8_t i, ret = 0;
	for (i = 0; i < network.number_of_devices; i++) {
		if ((0 == network.device_list[i]->renew_ack) && (!renew_threshold || (network.device_list[i]->renew_send_count < renew_threshold))) {
			network.device_list[i]->renew_send_count++;
			log_debug("Need renew %02x:%02x:%02x:%02x:%02x:%02x\n", network.device_list[i]->id[0], network.device_list[i]->id[1], network.device_list[i]->id[2], network.device_list[i]->id[3], network.device_list[i]->id[4], network.device_list[i]->id[5]);
			send_autoconfig_renew_request(network.device_list[i]->id);
			ret = 1;
		}
	}
	return ret;
}

void dump_dataelement(struct Network *network)
{
	unsigned char i, j, k, l, m;

	// log_info("\n-----------------DATA ELEMENT-------------------------\n\n");
	log_info("=============================================\n");
	log_info("Network ID: %s | %d Devices\n", network->id, network->number_of_devices);
	log_info("=============================================\n");

	//Devices
	for (i = 0; i < network->number_of_devices; i++) {
		log_info("Device %d - %02x%02x%02x%02x%02x%02x | %d Radios | Policy config: %d | Ch report: %d\n",
		         i + 1,
		         network->device_list[i]->id[0], network->device_list[i]->id[1], network->device_list[i]->id[2],
		         network->device_list[i]->id[3], network->device_list[i]->id[4], network->device_list[i]->id[5],
		         network->device_list[i]->number_of_radios,
		         network->device_list[i]->policy_config,
		         network->device_list[i]->received_channel_report);
		log_info("                        | Device name: %s | Renew Ack: %d | Renew Send: %d | Need VLAN: %d\n",
		         network->device_list[i]->device_name,
		         network->device_list[i]->renew_ack,
		         network->device_list[i]->renew_send_count,
		         network->device_list[i]->need_vlan_configure);
		log_info("                        | Physical distance: %d | Logical distance: %d | Profile: %d\n",
		         network->device_list[i]->physical_distance_to_controller,
		         network->device_list[i]->logical_distance_to_controller,
		         network->device_list[i]->multi_ap_profile);
		log_info("                        | IP Address : %s | Path score: %d | Direct nb nr: %d\n",
		         network->device_list[i]->ip_addr,
		         network->device_list[i]->path_score,
		         network->device_list[i]->number_of_direct_neighbors);

		log_info("\n");
		for (m = 0; m < network->device_list[i]->number_of_direct_neighbors; m++) {
			log_info("Nb %d : %02x%02x%02x%02x%02x%02x\n", m,
				network->device_list[i]->direct_neighbor_list[m].al_mac[0],
				network->device_list[i]->direct_neighbor_list[m].al_mac[1],
				network->device_list[i]->direct_neighbor_list[m].al_mac[2],
				network->device_list[i]->direct_neighbor_list[m].al_mac[3],
				network->device_list[i]->direct_neighbor_list[m].al_mac[4],
				network->device_list[i]->direct_neighbor_list[m].al_mac[5]);
		}
		log_info("\n");

		//Radios
		for (j = 0; j < network->device_list[i]->number_of_radios; j++) {

			if(network->device_list[i]->received_channel_report) {
				if(network->device_list[i]->radio_list[j]->current_operating_classes == NULL) {
					log_error("current_operating_classes has not updated yet!!\n");
					continue;
				}
			}

			log_info("Radio %d - %02x%02x%02x%02x%02x%02x | %d BSS | Op class: %d | Channel: %d | Utilization: %d\n",
				j + 1,
				network->device_list[i]->radio_list[j]->id[0], network->device_list[i]->radio_list[j]->id[1],
				network->device_list[i]->radio_list[j]->id[2], network->device_list[i]->radio_list[j]->id[3],
				network->device_list[i]->radio_list[j]->id[4], network->device_list[i]->radio_list[j]->id[5],
				network->device_list[i]->radio_list[j]->number_of_bss,
				(network->device_list[i]->received_channel_report) ? (network->device_list[i]->radio_list[j]->current_operating_classes->op_class) : (0),
				(network->device_list[i]->received_channel_report) ? (network->device_list[i]->radio_list[j]->current_operating_classes->channel) : (0),
				network->device_list[i]->radio_list[j]->utilization);
			//BSS
			for (k = 0; k < network->device_list[i]->radio_list[j]->number_of_bss; k++) {
				log_info("\tBss %d - %02x%02x%02x%02x%02x%02x | %s | %d Stas\n",
					k + 1,
					network->device_list[i]->radio_list[j]->bss_list[k]->bssid[0], network->device_list[i]->radio_list[j]->bss_list[k]->bssid[1],
					network->device_list[i]->radio_list[j]->bss_list[k]->bssid[2], network->device_list[i]->radio_list[j]->bss_list[k]->bssid[3],
					network->device_list[i]->radio_list[j]->bss_list[k]->bssid[4], network->device_list[i]->radio_list[j]->bss_list[k]->bssid[5],
					network->device_list[i]->radio_list[j]->bss_list[k]->ssid,
					network->device_list[i]->radio_list[j]->bss_list[k]->number_of_sta);
				//Sta
				for (l = 0; l < network->device_list[i]->radio_list[j]->bss_list[k]->number_of_sta; l++) {
					log_info("\t\tSta %d - %02x%02x%02x%02x%02x%02x | Rssi - %d | Bss score - %d | Best BSS - %02x%02x%02x%02x%02x%02x (%d:%d) | Status - %d | Throughput - %d kbps | BSS Trans - %d \n",
						l + 1,
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->mac_address[0],
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->mac_address[1],
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->mac_address[2],
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->mac_address[3],
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->mac_address[4],
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->mac_address[5],
						rcpi_to_rssi(network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->signal_strength),
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->curr_bss_score,
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->best_bss_mac[0],
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->best_bss_mac[1],
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->best_bss_mac[2],
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->best_bss_mac[3],
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->best_bss_mac[4],
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->best_bss_mac[5],
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->best_bss_score,
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->best_bss_count,
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->status,
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->throughput,
						network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->support_bss_transition);
				}
			}
		}
		log_info("---------------------------------------------\n");
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
	attr.mq_maxmsg  = 20;
	attr.mq_curmsgs = 0;
	attr.mq_msgsize = MAX_MESSAGE_SIZE;

	mq_unlink(name);

	if ((mqd_t)-1 == (mqdes = mq_open(name, O_RDWR | O_CREAT | O_CLOEXEC, 0666, &attr))) {
		log_error("mq_open('%s') returned with errno=%d (%s)\n", name, errno, strerror(errno));
		return 0;
	}

	return mqdes;
}

uint8_t *check_mbo_capability(uint8_t max_len, uint8_t *pbuf)
{
	unsigned char oui[]={0x50, 0x6f, 0x9a};
	uint8_t *p, j, len;
	j    = 4;
	p    = pbuf + 4;
	//_VENDOR_SPECIFIC_IE_ 221
	// find WFA OUI and MBO type 0x16
	if (j >= max_len)
		return NULL;

	while (1) {
		if (*p == 221 && !memcmp(p+2, oui, 3) && *(p+2+3) == 0x16) {
			return p;
		} else {
			len = *(p + 1);
			p += (len + 2);
			j += (len + 2);
		}
		if (j >= max_len)
			break;
	}
	return NULL;
}

void send_config(struct Network *network, uint16_t device_index, struct easymesh_datamodel *easymesh_database)
{
	uint8_t                          i;
	struct steering_config           steer_policy_data           = { 0 };
	struct metric_config             metric_policy_data          = { 0 };
	struct default8021q_config       default8021q_data           = { 0 };
	struct traffic_separation_config traffic_separation_data     = { 0 };

	uint8_t traffic_separation_tlv_exist = 0;

	steer_policy_data.ls_sta_nr               = 0;
	steer_policy_data.btm_sta_nr              = 0;
	steer_policy_data.radio_nr                = network->device_list[device_index]->number_of_radios;
	steer_policy_data.radio_steer_policy_data = (struct radio_steer_config *)malloc(sizeof(struct radio_steer_config) * steer_policy_data.radio_nr);
	memset(steer_policy_data.radio_steer_policy_data, 0, sizeof(struct radio_steer_config) * steer_policy_data.radio_nr);

	metric_policy_data.report_interval          = 10;
	metric_policy_data.radio_nr                 = network->device_list[device_index]->number_of_radios;
	metric_policy_data.radio_metric_policy_data = malloc(sizeof(struct radio_metric_config) * metric_policy_data.radio_nr);
	memset(metric_policy_data.radio_metric_policy_data, 0, sizeof(struct radio_metric_config) * metric_policy_data.radio_nr);

	for (i = 0; i < network->device_list[device_index]->number_of_radios; i++) {
		memcpy(steer_policy_data.radio_steer_policy_data[i].radio_mac, network->device_list[device_index]->radio_list[i]->id, 6);
		steer_policy_data.radio_steer_policy_data[i].policy = 1;

		memcpy(metric_policy_data.radio_metric_policy_data[i].radio_mac, network->device_list[device_index]->radio_list[i]->id, 6);
		metric_policy_data.radio_metric_policy_data[i].rcpi_threshold  = 0;
		metric_policy_data.radio_metric_policy_data[i].rcpi_hysteresis = 0;
		metric_policy_data.radio_metric_policy_data[i].cu_threshold    = 0;
		metric_policy_data.radio_metric_policy_data[i].inclusion_policy |= 0x40; //For associated sta link metric
		metric_policy_data.radio_metric_policy_data[i].inclusion_policy |= 0x80; //For associated sta traffic stats
	}

	if (VLAN_CONFIGURE_PENDING == network->device_list[device_index]->need_vlan_configure) {
		traffic_separation_tlv_exist = 1;

		default8021q_data.primary_vlan_id = easymesh_database->primary_vid;
		default8021q_data.default_pcp = 0;

		traffic_separation_data.ssid_nr = easymesh_database->vpair_number;
		traffic_separation_data.ssid_info = malloc(sizeof(struct traffic_separation_ssidInfo) * traffic_separation_data.ssid_nr);
		for (i = 0; i < traffic_separation_data.ssid_nr; i++) {
			traffic_separation_data.ssid_info[i].ssid_length = strlen(easymesh_database->vssid[i]);
			traffic_separation_data.ssid_info[i].ssid = malloc(traffic_separation_data.ssid_info[i].ssid_length);
			strncpy(traffic_separation_data.ssid_info[i].ssid, easymesh_database->vssid[i], traffic_separation_data.ssid_info[i].ssid_length);
			traffic_separation_data.ssid_info[i].vlan_id = easymesh_database->vlan_id[i];
		}
	}

	log_info("Send config to %02x%02x%02x%02x%02x%02x\n",
	         network->device_list[device_index]->id[0],
	         network->device_list[device_index]->id[1],
	         network->device_list[device_index]->id[2],
	         network->device_list[device_index]->id[3],
	         network->device_list[device_index]->id[4],
	         network->device_list[device_index]->id[5]);
	send_policy_config_request(network->device_list[device_index]->id, 1, steer_policy_data, 1, metric_policy_data, traffic_separation_tlv_exist, default8021q_data, traffic_separation_tlv_exist, traffic_separation_data);

	if (steer_policy_data.radio_steer_policy_data)
		free(steer_policy_data.radio_steer_policy_data);

	if (metric_policy_data.radio_metric_policy_data)
		free(metric_policy_data.radio_metric_policy_data);

	for (i = 0; i < traffic_separation_data.ssid_nr; i++) {
		free(traffic_separation_data.ssid_info[i].ssid);
	}

	if (traffic_separation_data.ssid_info)
		free(traffic_separation_data.ssid_info);
	// free(ethernet_configuration_data.ethernet_interface_data);
}

void _send_max_device_reached_notification(struct Network *network)
{
	unsigned char *buff = (unsigned char *)malloc(1);
	*buff               = network->max_device_reached;
	unsigned char rtk_oui[3];
	memcpy(rtk_oui, REALTEK_OUI, 3);

	//send this nofitication to all devices in the network
	unsigned char i;
	for (i = 0; i < network->number_of_devices; i++) {
		send_vendor_specific_message(network->device_list[i]->id, 0, rtk_oui, 1, buff);
	}

	free(buff);
}

unsigned char _check_max_device_reached(struct Network *network)
{
	//Enter blocking state if current state = 0 and number of devices reaches limit
	if (network->number_of_devices >= max_num_device_allowed && !network->max_device_reached) {
		network->max_device_reached = 1;
		_send_max_device_reached_notification(network);
		return 1;
	}

	//Exit blocking state if current state = 0 and number of devices fall below limit
	if (network->number_of_devices < max_num_device_allowed && network->max_device_reached) {
		network->max_device_reached = 0;
		_send_max_device_reached_notification(network);
		return 2;
	}

	return 0;
}

void free_dead_devices(struct Network *network)
{
	int i = 0;
	for (i = 0; i < network->number_of_devices; i++) {
		time_t now;
		now = time(&now);

		if(memcmp(network->controller_id, network->device_list[i]->id, 6)) {
			network->device_list[i]->keep_alive -= 1;
		}

		if(0 == network->device_list[i]->keep_alive && memcmp(network->controller_id, network->device_list[i]->id, 6)) {
			log_info("Device %s did not update for over 30 seconds, consider timeout and expired...\n", network->device_list[i]->device_name);
#ifdef PATHSEL
			remove_path_node(network->device_list[i]->id);
#endif
			free_device(network, network->device_list[i]->id);
		}
	}
}

void per_interval_action(struct Network *network, struct easymesh_datamodel *easymesh_database)
{
	int i;
	int config_nr = 0;
	int config_index[network->number_of_devices];

	//re-export topology info to file every 10 sec
	FILE *         fp                = fopen("/tmp/topology_json_tmp", "we");
	if (fp) {
		struct Device *controller_device = locate_device(network, network->controller_id, NEED_CREATE_FALSE, NULL);
		if (NULL != controller_device) {
			tree_topology_json_export(network, controller_device, network->controller_id, fp, 1, easymesh_database->hw_reg_domain, 0, easymesh_database->customize_features);
		}
		fclose(fp);
		reset_visited(network);
		(void) rename("/tmp/topology_json_tmp", "/tmp/topology_json");
	}

	// Shall separate free device and other device manipulation to avoid freed pointer gets dereferenced in speculative execution or optimization
	free_dead_devices(network);

	//Devices
	for (i = 0; i < network->number_of_devices; i++) {
		time_t now;
		now = time(&now);

		if(0 == memcmp(network->device_list[i]->id, network->controller_id, 6) && network->device_list[i]->policy_config != 1) {
			config_index[config_nr++] = (network->device_list[i]->physical_distance_to_controller << 8) | i;
			network->device_list[i]->policy_config = 1;
			continue;
		}

		if (network->device_list[i]->need_vlan_configure) {
			if (network->device_list[i]->policy_config != 1) {
				config_index[config_nr++] = (network->device_list[i]->physical_distance_to_controller << 8) | i;
				network->device_list[i]->policy_config = 1;
				continue;
			} else if (difftime(now, network->device_list[i]->updated_timestamp) > 20) {
				if(memcmp(network->device_list[i]->id, network->controller_id, 6)) {
					log_info("Device did not update, resending config...\n");
					config_index[config_nr++] = (network->device_list[i]->physical_distance_to_controller << 8) | i;
					continue;
				}
			}
		}

		//---------CHANNEL SELECTION--------------
		//Send once when device join and once again after 1 min.
		if (0 == network->device_list[i]->received_channel_report || 6 == network->device_list[i]->received_channel_report) {
			send_channel_preference_query(network->device_list[i]->id);
			continue;
		}
		if (network->device_list[i]->received_channel_report >= 1 && network->device_list[i]->received_channel_report <= 5) {
			network->device_list[i]->received_channel_report++;
		}
		int j = 0, empty_op_class_info = 0;
		for ( j =0; j< network->device_list[i]->number_of_radios; j++) {
			if(network->device_list[i]->radio_list[j]->current_operating_classes == NULL) {
				log_error("current_operating_classes not updated yet!\n");
				empty_op_class_info = 1;
				break;
			}
			if( 0 == network->device_list[i]->radio_list[j]->current_operating_classes->op_class) {
				empty_op_class_info = 1;
				break;
			}
		}
		if (empty_op_class_info) {
			send_channel_preference_query(network->device_list[i]->id);
			continue;
		}
		//---------CHANNEL SELECTION--------------

		pre_steer_process(network, i);
	}
//---------POLICY CONFIG REQUEST--------------
	qsort(config_index, config_nr, sizeof(int), cmpfunc);
	for (i = config_nr - 1; i >= 0; i--) {
		send_config(network, config_index[i] & 0x00FF, easymesh_database);
		log_trace("Send config to Device: %04x\n", config_index[i]);
	}
//---------POLICY CONFIG REQUEST--------------

#ifdef MANUAL_CLIENT_STEER
	manual_roam_check(network);
#endif

	//process roaming for each sta
	if(roaming_enable) {
		client_steer_process(network, easymesh_database->beacon_request_ch);
	}
	//self query every 20 sec to get neighbor info updated
	static unsigned char self_query_counter = 0;
	self_query_counter++;
	if (2 == self_query_counter) {
		//Check max device reached
		unsigned char block_result = _check_max_device_reached(network);
		if (1 == block_result) {
			log_info("Number of Devices Reached Max Allowed Device Num!! START blocking new AP from joining the network...\n");
		} else if (2 == block_result) {
			log_info("Number of Devices Fell Below Max Allowed Device Num!! STOP blocking new AP from joining the network...\n");
		}
		//ask for topology response periodically
		for (i = 0; i < network->number_of_devices; i++) {
			send_topology_query_request(network->device_list[i]->id);
		}
		//send_topology_query_request(network->controller_id);
		self_query_counter = 0;
	}
	//send device limit notification every 40 sec
	static unsigned char max_device_limit_notification_counter = 0;
	max_device_limit_notification_counter++;
	if (max_device_limit_notification_counter >= 4) {
		_send_max_device_reached_notification(network);
		max_device_limit_notification_counter = 0;
	}
}

void _free_channel_preference(struct channel_preference *pref,uint8_t pref_number)
{
	int i, j;
	for (i = 0; i < pref_number; i++) {
		for (j = 0; j < pref[i].op_class_nr; j++) {
			free(pref[i].ch_pre_op_class[j].channel_list);
		}
		free(pref[i].ch_pre_op_class);
	}

	if (pref) {
		free(pref);
	}
}

//---------CHANNEL SELECTION--------------
int _update_controller_ch_pref(struct channel_preference *controller_ch_pref, struct ChannelPreferenceTLV *ch_pref_tlv, char *radio_name, struct easymesh_datamodel *easymesh_database)
{
	uint8_t i;
	int     bandwidth = 0, sideband = 0;

	memcpy(controller_ch_pref->radio_mac, ch_pref_tlv->radio_unique_id, 6);
	if (!(easymesh_database->eth_only) && obtainRadioBandInfo(radio_name, &bandwidth, &sideband)) {
		log_error("Error: Cannot get band info for radio %s\n", radio_name);
		controller_ch_pref->op_class_nr = 0;
		controller_ch_pref->ch_pre_op_class = NULL;
		return -1;
	}

	controller_ch_pref->op_class_nr     = ch_pref_tlv->op_class_nr;
	controller_ch_pref->ch_pre_op_class = (struct channel_preference_op_class *)zalloc(sizeof(struct channel_preference_op_class) * ch_pref_tlv->op_class_nr);

	for (i = 0; i < ch_pref_tlv->op_class_nr; i++) {
		uint8_t band          = get_band_from_op_class(ch_pref_tlv->channel_preferences[i].op_class);
		uint8_t skip_op_class = 0;
		uint8_t j, found = 0;

		if (easymesh_database->eth_only) {
			if (band == RADIO_BAND_5GL || band == RADIO_BAND_5GH || band == RADIO_BAND_5GF) {
				for (j = 0; j < easymesh_database->radio_data_nr; j++) {
					if (RADIO_BAND_5GL == easymesh_database->radio_data[j].radio_band
						|| RADIO_BAND_5GH == easymesh_database->radio_data[j].radio_band
						|| RADIO_BAND_5GF == easymesh_database->radio_data[j].radio_band) {
						bandwidth = easymesh_database->radio_data[j].channel_bandwidth;
						sideband = SIDE_BAND_NONE;

						found = 1;
						break;
					}
				}
				if (!found) {
					log_error("Error: Cannot get band info for radio %s\n", easymesh_database->radio_name_5gl);
					continue;
				}
			} else if (band == RADIO_BAND_2G) {
				for (j = 0; j < easymesh_database->radio_data_nr; j++) {
					if (band == easymesh_database->radio_data[j].radio_band) {
						bandwidth = easymesh_database->radio_data[j].channel_bandwidth;
						if (easymesh_database->radio_data[j].control_sideband == 0)
							sideband = SIDE_BAND_UPPER;
						else if (easymesh_database->radio_data[j].control_sideband == 1)
							sideband = SIDE_BAND_LOWER;

						found = 1;
						break;
					}
				}
				if (!found) {
					log_error("Error: Cannot get band info for radio %s\n", easymesh_database->radio_name_24g);
					continue;
				}
			} else {
				skip_op_class = 1;
				continue;
			}
		} else {
			if (band >= RADIO_BAND_ETH) {
				skip_op_class = 1;
			} else {
				skip_op_class = isUnoperableOpClass(ch_pref_tlv->channel_preferences[i].op_class, bandwidth, sideband);
			}
		}

		controller_ch_pref->ch_pre_op_class[i].op_class               = ch_pref_tlv->channel_preferences[i].op_class;

		if(skip_op_class) {
			log_debug("Skip unoperable op class %d\n", ch_pref_tlv->channel_preferences[i].op_class);
			controller_ch_pref->ch_pre_op_class[i].preference_reason_code = 0;
			controller_ch_pref->ch_pre_op_class[i].channel_nr             = 0;
			controller_ch_pref->ch_pre_op_class[i].channel_list           = NULL;
		} else {
			log_debug("Updating op class [%d], channel_nr: [%d]\n", ch_pref_tlv->channel_preferences[i].op_class, ch_pref_tlv->channel_preferences[i].channel_nr);
			controller_ch_pref->ch_pre_op_class[i].preference_reason_code = ch_pref_tlv->channel_preferences[i].preference_reason_code;
			controller_ch_pref->ch_pre_op_class[i].channel_nr             = ch_pref_tlv->channel_preferences[i].channel_nr;
			controller_ch_pref->ch_pre_op_class[i].channel_list           = NULL;
			if (controller_ch_pref->ch_pre_op_class[i].channel_nr) {
				controller_ch_pref->ch_pre_op_class[i].channel_list           = (uint8_t *)malloc(sizeof(uint8_t) * ch_pref_tlv->channel_preferences[i].channel_nr);
				memcpy(controller_ch_pref->ch_pre_op_class[i].channel_list, ch_pref_tlv->channel_preferences[i].channel_list, ch_pref_tlv->channel_preferences[i].channel_nr);
			}
		}
	}

	return 0;
}

int write_cac_report_file(struct cac_completion_report *cac_report)
{
	int i, j;

	FILE *fp = fopen("/tmp/map_cac_report", "we");
	if (fp == NULL) {
		printf("Error opening cac report file!\n");
		return 0;
	}

	fprintf(fp, "[CAC Completion Report]\n");
	fprintf(fp, "radio_nr: %d\n", cac_report->radio_nr);
	for(i = 0; i < cac_report->radio_nr; i++) {
		fprintf(fp, "cac_radio_id: %02x:%02x:%02x:%02x:%02x:%02x\n",
					cac_report->radios[i].radio_id[0],
					cac_report->radios[i].radio_id[1],
					cac_report->radios[i].radio_id[2],
					cac_report->radios[i].radio_id[3],
					cac_report->radios[i].radio_id[4],
					cac_report->radios[i].radio_id[5]);
		fprintf(fp, "op_class: %d\n", cac_report->radios[i].op_class);
		fprintf(fp, "channel: %d\n", cac_report->radios[i].channel);
		fprintf(fp, "flags: %d\n", cac_report->radios[i].flags);
		fprintf(fp, "pairs_nr: %d\n", cac_report->radios[i].pairs_nr);
		for(j = 0; j < cac_report->radios[i].pairs_nr; j++) {
			fprintf(fp, "pairs_op_class: %d\n", cac_report->radios[i].pairs[j].op_class);
			fprintf(fp, "pairs_channel: %d\n", cac_report->radios[i].pairs[j].channel);
		}
	}

	fclose(fp);

	return 0;
}

int write_channel_scan_report_file(struct channel_scan_report *data)
{
	int i, band, channel_nr_5G = 0, channel_nr_2G = 0;
	FILE *fp_tmp = NULL;

	FILE *fp_5G = fopen("/tmp/map_channel_scan_report_5G", "we");
	if (fp_5G == NULL) {
		log_error("Error opening 5G channel scan report file!\n");
		return 0;
	}
	FILE *fp_2G = fopen("/tmp/map_channel_scan_report_2G", "we");
	if (fp_2G == NULL) {
		log_error("Error opening 2G channel scan report file!\n");
		fclose(fp_5G);
		return 0;
	}

	for(i = 0; i < data->channel_nr; i++) {
		band = get_band_from_op_class(data->channels[i].op_class);
		if(band == RADIO_BAND_5GH || band == RADIO_BAND_5GL || band == RADIO_BAND_5GF) {
			channel_nr_5G++;
			fp_tmp = fp_5G;
		} else if (band == RADIO_BAND_2G) {
			channel_nr_2G++;
			fp_tmp = fp_2G;
		}
		if(fp_tmp) {
			fprintf(fp_tmp, "op_class %d\n", data->channels[i].op_class);
			fprintf(fp_tmp, "channel %d\n", data->channels[i].channel);
			fprintf(fp_tmp, "scan_status %d\n", data->channels[i].scan_status);
			fprintf(fp_tmp, "utilization %d\n", data->channels[i].utilization);
			fprintf(fp_tmp, "noise %d\n", data->channels[i].noise);
			fprintf(fp_tmp, "neighbor_nr %d\n", data->channels[i].neighbor_nr);
		}
	}

	if (channel_nr_5G == 0) {
		if (remove("/tmp/map_channel_scan_report_5G")) {
			log_error("remove /tmp/map_channel_scan_report_5G file error!\n");
		}
	} else {
		FILE *fp_5G_nr = fopen("/tmp/map_channel_scan_nr_5G", "we");
		if (fp_5G_nr) {
			fprintf(fp_5G_nr, "channel_nr %d\n", channel_nr_5G);
			fclose(fp_5G_nr);
		}
	}
	if (channel_nr_2G == 0) {
		if (remove("/tmp/map_channel_scan_report_2G")) {
			log_error("remove /tmp/map_channel_scan_report_2G file error!\n");
		}
	} else {
		FILE *fp_2G_nr = fopen("/tmp/map_channel_scan_nr_2G", "we");
		if (fp_2G_nr) {
			fprintf(fp_2G_nr, "channel_nr %d\n", channel_nr_2G);
			fclose(fp_2G_nr);
		}
	}
	fclose(fp_5G);
	fclose(fp_2G);

	return 1;
}

int construct_channel_scan_request(struct Device *target_device, int scan_band, int reg_domain, struct channel_scan_request *scan_req)
{
	int i, k;

	uint8_t  band, op_class_nr_5GF = 0, op_class_nr_5GL = 0, op_class_nr_5GH = 0, op_class_nr_2G = 0;
	uint8_t *op_class_5GF = NULL, *op_class_5GL = NULL, *op_class_5GH = NULL, *op_class_2G = NULL;

	// check if requested scan_band is found on the target device.
	int found_2g = 0;
	int found_5g = 0;
	int skip     = 1;
	for (i = 0; i < target_device->number_of_radios; i++) {
		if (target_device->radio_list[i]->current_operating_classes) {
			band = get_band_from_op_class(target_device->radio_list[i]->current_operating_classes[0].op_class);
			if (band != RADIO_BAND_UNKNOWN) {
				if (0 == scan_band) {
					if (band == RADIO_BAND_2G) {
						found_2g = 1;
					}
					if (band == RADIO_BAND_5GL || band == RADIO_BAND_5GH || band == RADIO_BAND_5GF) {
						found_5g = 1;
					}
					if (found_2g && found_5g) {
						skip = 0;
						break;
					}
				} else if (1 == scan_band) {
					if (band == RADIO_BAND_2G) {
						skip = 0;
						break;
					}
				} else if (2 == scan_band) {
					if (band == RADIO_BAND_5GL || band == RADIO_BAND_5GH || band == RADIO_BAND_5GF) {
						skip = 0;
						break;
					}
				}
			}
		}
	}

	if (skip) {
		return 0;
	}

	scan_req->flags = 0x80; //set Perform Fresh Scan to one

	OP_CLASS *op_classes = get_op_class_with_reg_domain(reg_domain);

	if (NULL == op_classes) {
		log_error("No operating class is available with certain reg_domain\n");
		return 0;
	}

	for (i = 0; i < op_classes->op_class_len; i++) {
		if (op_classes[i].op_class > 80 && op_classes[i].op_class <= 84) {
			op_class_2G                 = (uint8_t *)realloc(op_class_2G, sizeof(uint8_t) * (op_class_nr_2G + 1));
			op_class_2G[op_class_nr_2G] = op_classes[i].op_class;
			op_class_nr_2G++;
		} else if (op_classes[i].op_class > 110 && op_classes[i].op_class <= 120) {
			op_class_5GL                  = (uint8_t *)realloc(op_class_5GL, sizeof(uint8_t) * (op_class_nr_5GL + 1));
			op_class_5GF                  = (uint8_t *)realloc(op_class_5GF, sizeof(uint8_t) * (op_class_nr_5GF + 1));
			op_class_5GL[op_class_nr_5GL] = op_classes[i].op_class;
			op_class_5GF[op_class_nr_5GF] = op_classes[i].op_class;
			op_class_nr_5GL++;
			op_class_nr_5GF++;
		} else if (op_classes[i].op_class > 120 && op_classes[i].op_class < 130) {
			op_class_5GH                  = (uint8_t *)realloc(op_class_5GH, sizeof(uint8_t) * (op_class_nr_5GH + 1));
			op_class_5GF                  = (uint8_t *)realloc(op_class_5GF, sizeof(uint8_t) * (op_class_nr_5GF + 1));
			op_class_5GH[op_class_nr_5GH] = op_classes[i].op_class;
			op_class_5GF[op_class_nr_5GF] = op_classes[i].op_class;
			op_class_nr_5GH++;
			op_class_nr_5GF++;
		}
	}

	if (scan_band == 0) {
		scan_req->target_radio_nr = (uint8_t)target_device->number_of_radios;
		scan_req->target_radios   = (struct channel_scan_target_radio *)malloc(scan_req->target_radio_nr * sizeof(struct channel_scan_target_radio));
		memset(scan_req->target_radios, 0, scan_req->target_radio_nr * sizeof(struct channel_scan_target_radio));
		for (i = 0; i < scan_req->target_radio_nr; i++) {
			if (target_device->radio_list[i] == NULL || target_device->radio_list[i]->current_operating_classes == NULL)
				continue;
			band = get_band_from_op_class(target_device->radio_list[i]->current_operating_classes[0].op_class);
			if (band == RADIO_BAND_2G) {
				memcpy(scan_req->target_radios[i].radio_id, target_device->radio_list[i]->id, 6);
				scan_req->target_radios[i].target_op_class_nr = op_class_nr_2G;
				scan_req->target_radios[i].target_op_classes  = (struct channel_scan_target_op_class *)malloc(scan_req->target_radios[i].target_op_class_nr * sizeof(struct channel_scan_target_op_class));
				for (k = 0; k < scan_req->target_radios[i].target_op_class_nr; k++) {
					scan_req->target_radios[i].target_op_classes[k].op_class          = op_class_2G[k];
					scan_req->target_radios[i].target_op_classes[k].target_channel_nr = 0;
				}
			} else if (band == RADIO_BAND_5GF || scan_req->target_radio_nr == 2) {
				memcpy(scan_req->target_radios[i].radio_id, target_device->radio_list[i]->id, 6);
				scan_req->target_radios[i].target_op_class_nr = op_class_nr_5GF;
				scan_req->target_radios[i].target_op_classes  = (struct channel_scan_target_op_class *)malloc(scan_req->target_radios[i].target_op_class_nr * sizeof(struct channel_scan_target_op_class));
				for (k = 0; k < scan_req->target_radios[i].target_op_class_nr; k++) {
					scan_req->target_radios[i].target_op_classes[k].op_class          = op_class_5GF[k];
					scan_req->target_radios[i].target_op_classes[k].target_channel_nr = 0;
				}
			} else if (band == RADIO_BAND_5GL) {
				memcpy(scan_req->target_radios[i].radio_id, target_device->radio_list[i]->id, 6);
				scan_req->target_radios[i].target_op_class_nr = op_class_nr_5GL;
				scan_req->target_radios[i].target_op_classes  = (struct channel_scan_target_op_class *)malloc(scan_req->target_radios[i].target_op_class_nr * sizeof(struct channel_scan_target_op_class));
				for (k = 0; k < scan_req->target_radios[i].target_op_class_nr; k++) {
					scan_req->target_radios[i].target_op_classes[k].op_class          = op_class_5GL[k];
					scan_req->target_radios[i].target_op_classes[k].target_channel_nr = 0;
				}
			} else if (band == RADIO_BAND_5GH) {
				memcpy(scan_req->target_radios[i].radio_id, target_device->radio_list[i]->id, 6);
				scan_req->target_radios[i].target_op_class_nr = op_class_nr_5GH;
				scan_req->target_radios[i].target_op_classes  = (struct channel_scan_target_op_class *)malloc(scan_req->target_radios[i].target_op_class_nr * sizeof(struct channel_scan_target_op_class));
				for (k = 0; k < scan_req->target_radios[i].target_op_class_nr; k++) {
					scan_req->target_radios[i].target_op_classes[k].op_class          = op_class_5GH[k];
					scan_req->target_radios[i].target_op_classes[k].target_channel_nr = 0;
				}
			}
		}
	} else {
		scan_req->target_radio_nr = 1;
		scan_req->target_radios   = (struct channel_scan_target_radio *)malloc(sizeof(struct channel_scan_target_radio));
		memset(scan_req->target_radios, 0, scan_req->target_radio_nr * sizeof(struct channel_scan_target_radio));
		for (i = 0; i < target_device->number_of_radios; i++) {
			if (target_device->radio_list[i] == NULL || target_device->radio_list[i]->current_operating_classes == NULL)
				continue;
			band = get_band_from_op_class(target_device->radio_list[i]->current_operating_classes[0].op_class);
			if (band == RADIO_BAND_2G && scan_band == 1) {
				memcpy(scan_req->target_radios[0].radio_id, target_device->radio_list[i]->id, 6);
				scan_req->target_radios[0].target_op_class_nr = op_class_nr_2G;
				scan_req->target_radios[0].target_op_classes  = (struct channel_scan_target_op_class *)malloc(scan_req->target_radios[0].target_op_class_nr * sizeof(struct channel_scan_target_op_class));
				for (k = 0; k < scan_req->target_radios[0].target_op_class_nr; k++) {
					scan_req->target_radios[0].target_op_classes[k].op_class          = op_class_2G[k];
					scan_req->target_radios[0].target_op_classes[k].target_channel_nr = 0;
				}
			} else if ((band == RADIO_BAND_5GH || band == RADIO_BAND_5GL || band == RADIO_BAND_5GF) && scan_band == 2) {
				memcpy(scan_req->target_radios[0].radio_id, target_device->radio_list[i]->id, 6);
				scan_req->target_radios[0].target_op_class_nr = op_class_nr_5GF;
				scan_req->target_radios[0].target_op_classes  = (struct channel_scan_target_op_class *)malloc(scan_req->target_radios[0].target_op_class_nr * sizeof(struct channel_scan_target_op_class));
				for (k = 0; k < scan_req->target_radios[0].target_op_class_nr; k++) {
					scan_req->target_radios[0].target_op_classes[k].op_class          = op_class_5GF[k];
					scan_req->target_radios[0].target_op_classes[k].target_channel_nr = 0;
				}
			}
		}
	}
	free(op_classes);
	if (op_class_2G)
		free(op_class_2G);
	if (op_class_5GF)
		free(op_class_5GF);
	if (op_class_5GL)
		free(op_class_5GL);
	if (op_class_5GH)
		free(op_class_5GH);

	return 1;
}

int check_channel_scan_command(struct Network *network, int hw_reg_domain)
{
	int                         scan_band, i, j;
	struct channel_scan_request channel_scan_req = { 0 };

	FILE *fp = fopen("/tmp/channel_scan_band", "re");
	if (fp == NULL) {
		return 0;
	}
	if (fscanf(fp, "%d", &scan_band) != 1) {
		log_error("read channel scan band error!\n");
		fclose(fp);
		return 0;
	}
	log_debug("read from /tmp/channel_scan_band file: %d\n", scan_band);

	for (i = 0; i < network->number_of_devices; i++) {
		memset(&channel_scan_req, 0, sizeof(struct channel_scan_request));
		if (0 == construct_channel_scan_request(network->device_list[i], scan_band, hw_reg_domain, &channel_scan_req)) {
			log_debug("skip send_channel_scan_request to device %02x:%02x:%02x:%02x:%02x:%02x !\n",
			          network->device_list[i]->id[0],
			          network->device_list[i]->id[1],
			          network->device_list[i]->id[2],
			          network->device_list[i]->id[3],
			          network->device_list[i]->id[4],
			          network->device_list[i]->id[5]);
			continue;
		}
		send_channel_scan_request(network->device_list[i]->id, channel_scan_req);
		if (channel_scan_req.target_radio_nr > 0) {
			for (j = 0; j < channel_scan_req.target_radio_nr; j++) {
				if (channel_scan_req.target_radios[j].target_op_classes) {
					free(channel_scan_req.target_radios[j].target_op_classes);
				}
			}
		}
		if (channel_scan_req.target_radios)
			free(channel_scan_req.target_radios);
		log_debug("send_channel_scan_request to device %02x:%02x:%02x:%02x:%02x:%02x !\n",
		          network->device_list[i]->id[0],
		          network->device_list[i]->id[1],
		          network->device_list[i]->id[2],
		          network->device_list[i]->id[3],
		          network->device_list[i]->id[4],
		          network->device_list[i]->id[5]);
	}

	if (-1 == remove("/tmp/channel_scan_band")) {
		log_error("remove /tmp/channel_scan_band file error!\n");
	}
	fclose(fp);
	return 1;
}

int main(int argc, char *argv[])
{
	int c;
	int verbose_level = 1; // Only ERROR messages
	int verbose_quiet = 1; // Only log to file
	int self_query    = 0;

	uint8_t autoconfig_renew_count = 0;
	// uint8_t cac_timer_counter = 0;//test
	uint8_t channel_scan_report_recv = 0;
	uint8_t channel_scan_timer = 0;
	uint8_t channel_scan_flag = 0;
	struct  channel_scan_report aggre_report = { 0 };
	//---------CHANNEL SELECTION--------------
	uint8_t                    controller_ch_pref_updated = 0;
	uint8_t                    controller_ch_pref_number  = 0;
	struct channel_preference *controller_ch_pref         = NULL;
	//---------CHANNEL SELECTION--------------

	char *buffer               = (char *)malloc(MAX_MESSAGE_SIZE + 1);
	int   as_daemon            = 0;
	int   wfa_test             = 0;
	int   overwrite_pushbutton = 0;
	mqd_t mq;

	char *image_version = EASY_MESH_VERSION;

	uint8_t timer_counter = 0;
	uint8_t multiap_profile = 1;
	struct easymesh_datamodel easymesh_database;

	easymesh_database.config_data_nr   = 0;
	easymesh_database.configured_band  = 0;
	easymesh_database.hw_reg_domain    = 0;
	easymesh_database.hw_country_str   = NULL;
	easymesh_database.customize_features = 0;
	easymesh_database.radio_data_nr    = 0;
	easymesh_database.device_name      = NULL;
	easymesh_database.per_radio_config = NULL;
	easymesh_database.radio_data       = NULL;
	easymesh_database.primary_vid      = 0;
	easymesh_database.default_pcp      = 0;
	easymesh_database.vpair_number     = 0;
	easymesh_database.vlan_id         = NULL;
	easymesh_database.vssid            = NULL;
	easymesh_database.bridge_name     = NULL;
	easymesh_database.max_bss_per_radio  = 5;
	easymesh_database.max_renew_resend_time  = 0;
	easymesh_database.common_netlink     = 0;
	easymesh_database.controller_interface = NULL;
	easymesh_database.controller_al_interface = NULL;
	easymesh_database.excluded_interfaces = NULL;
	easymesh_database.eth_only = 0;

	easymesh_database.interface_manufacturer_name = NULL;
	easymesh_database.interface_model_name = NULL;
	easymesh_database.interface_device_name = NULL;
	easymesh_database.autoconfig_renew_on_band = 2;
	memset(easymesh_database.mcs_20m_table, 0, sizeof(easymesh_database.mcs_20m_table));
	memset(easymesh_database.mcs_40m_table, 0, sizeof(easymesh_database.mcs_40m_table));
	easymesh_database.max_sta_number  = 64;
	easymesh_database.max_log_size = 100;
	easymesh_database.bss_list_update = 0;
	easymesh_database.backhaul_bss_index = 1;

	if (ini_parse("/var/multiap.conf", read_config_file, &easymesh_database) < 0) {
		log_error("[RTK] Can't load configuration file!! \n");
		free(buffer);
		return INIT_ERROR_CONFIG_FILE;
	}

	if (ini_parse("/var/multiap_mib.conf", read_mib_config_file, &easymesh_database) < 0) {
		log_error("[RTK] Can't load mib configuration file!! \n");
		free(buffer);
		return INIT_ERROR_CONFIG_FILE;
	}

	fill_bss_index_vendor_config(&easymesh_database);

	initLogFile(MAP_LOG_FILE_PATH, easymesh_database.max_log_size);

	log_info("<<<< Initialized EasyMesh Log >>>\n");

	log_info("EasyMesh Turnkey %s\n", image_version);

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
			free(buffer);
			return 0;
		}
	}

	if (verbose_quiet) {
		disableLogDebug();
	} else {
		enableLogDebug();
	}
	log_set_level(verbose_level);

	if (!easymesh_database.bridge_name) {
		easymesh_database.bridge_name = strdup("br0");
		log_debug("Set default bridge name br0\n");
	}

	if (strcmp(easymesh_database.radio_name_5gl, easymesh_database.radio_name_5gh)) {
		easymesh_database.radio_number = 3;
	} else {
		easymesh_database.radio_number = 2;
	}

	log_info("Radio Number: %d\n", easymesh_database.radio_number);

	char *interfaces = (char*)malloc(512);
	memset(interfaces, 0, 512);

	int i, j;
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
				if (easymesh_database.eth_only || is_interface_up(ifname))
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

	append_br_interfaces(easymesh_database.bridge_name, interfaces);

	if (!map_get_mac(easymesh_database.radio_name_24g, easymesh_database.radio_24g_mac)) {
		log_error("failed to get %s mac addr for easymesh_database \n", easymesh_database.radio_name_24g);
	}

	if (!map_get_mac(easymesh_database.radio_name_5gl, easymesh_database.radio_5gl_mac)) {
		log_error("failed to get %s mac addr for easymesh_database \n", easymesh_database.radio_name_5gl);
	}

	if (easymesh_database.radio_number == 3) {
		if (!map_get_mac(easymesh_database.radio_name_5gh, easymesh_database.radio_5gh_mac)) {
			log_error("failed to get %s mac addr for easymesh_database \n", easymesh_database.radio_name_5gh);
		}
	}

	char *intf_name = strdup(interfaces);
	char *intf_token = strtok(intf_name, ",");

	while (intf_token != NULL) {
		log_debug("Find Interface: %s\n", intf_token);
		intf_token = strtok(NULL, ",");
	}
	free(intf_name);

	update_mcs_table_from_conf(&easymesh_database.mcs_20m_table[0], sizeof(easymesh_database.mcs_20m_table));
	update_mcs_table_from_conf(&easymesh_database.mcs_40m_table[0], sizeof(easymesh_database.mcs_40m_table));

	mq = create_mqueue(QUEUE_NAME);

	pthread_t                      map_thread;
	struct map_service_thread_data map_thread_data;
	map_thread_data.queue_name                                 = QUEUE_NAME;
	map_thread_data.device_role                                = easymesh_database.map_supported_service;
	map_thread_data.multiap_profile                            = multiap_profile;
	map_thread_data.default_setting.self_listening             = 1;
	map_thread_data.default_setting.neighbor_removal_threshold = 90;
	map_thread_data.configure_state                            = easymesh_database.configured_band; //configure_state;
	map_thread_data.default_setting.resend_time                = easymesh_database.max_resend_time;
	map_thread_data.default_setting.verbose_level              = verbose_level;
	map_thread_data.default_setting.verbose_quiet              = verbose_quiet;
	map_thread_data.default_setting.log_file_size              = easymesh_database.max_log_size;
	map_thread_data.default_setting.max_bss_per_radio          = easymesh_database.max_bss_per_radio;
	map_thread_data.default_setting.overwrite_pushbutton       = overwrite_pushbutton;
	map_thread_data.default_setting.primary_vid                = easymesh_database.primary_vid;
	map_thread_data.default_setting.default_pcp                = easymesh_database.default_pcp;
	map_thread_data.default_setting.common_netlink             = easymesh_database.common_netlink;
	map_thread_data.default_setting.customize_features         = easymesh_database.customize_features;
	map_thread_data.default_setting.autoconfig_renew_on_band   = easymesh_database.autoconfig_renew_on_band;
	map_thread_data.default_setting.max_sta_num                = easymesh_database.max_sta_number;
	map_thread_data.config_nr                                  = easymesh_database.config_data_nr; //config_nr: how many interfaces configured
	map_thread_data.config_data                                = easymesh_database.per_radio_config;
	if (easymesh_database.device_name) {
		map_thread_data.default_setting.device_name            = strdup(easymesh_database.device_name);
	} else {
		map_thread_data.default_setting.device_name            = "EasyMesh Controller";
	}
	map_thread_data.default_setting.radio_name_24g             = easymesh_database.radio_name_24g;
	map_thread_data.default_setting.radio_name_5gl             = easymesh_database.radio_name_5gl;
	map_thread_data.default_setting.radio_name_5gh             = easymesh_database.radio_name_5gh;
	map_thread_data.default_setting.vap_prefix                 = easymesh_database.vap_prefix;
	map_thread_data.default_setting.bridge_name                 = easymesh_database.bridge_name;
	if (easymesh_database.excluded_interfaces) {
		map_thread_data.default_setting.excluded_interfaces    = easymesh_database.excluded_interfaces;
	} else {
		map_thread_data.default_setting.excluded_interfaces    = "";
	}
	map_thread_data.default_setting.controller_interface       = NULL;
	map_thread_data.default_setting.controller_al_interface    = NULL;
	map_thread_data.default_setting.eth_only                   = easymesh_database.eth_only;
	map_thread_data.default_setting.hw_reg_domain              = easymesh_database.hw_reg_domain;
	map_thread_data.default_setting.radio_data_nr              = easymesh_database.radio_data_nr;
	map_thread_data.default_setting.radio_data                 = easymesh_database.radio_data;
	map_thread_data.al_interfaces                              = interfaces;
	map_thread_data.wfa_test_mode                              = wfa_test;

	map_thread_data.default_setting.interface_manufacturer_name = easymesh_database.interface_manufacturer_name;
	map_thread_data.default_setting.interface_model_name = easymesh_database.interface_model_name;
	map_thread_data.default_setting.interface_device_name = easymesh_database.interface_device_name;
	map_thread_data.default_setting.hw_country_str        = easymesh_database.hw_country_str;

	rssi_weightage          = easymesh_database.rssi_weightage;
	path_weightage          = easymesh_database.path_weightage;
	cu_weightage            = easymesh_database.cu_weightage;
	roam_score_difference   = easymesh_database.roam_score_difference;
	min_evaluation_interval = easymesh_database.min_evaluation_interval;
	min_roam_interval       = easymesh_database.min_roam_interval;
	throughput_threshold    = easymesh_database.throughput_threshold;

	max_sta_num_per_bss     = easymesh_database.max_sta_number;

	if ((path_weightage + rssi_weightage + cu_weightage) != 10) {
		log_error("\n\nWrong configuration for client steering weightage.\nTotal value must be 10.\n\n\n");
		exit(1);
	}

	log_info("RSSI weightage: %d\n", rssi_weightage);
	log_info("Path weightage: %d\n", path_weightage);
	log_info("CU weightage: %d\n", cu_weightage);
	log_info("Roam score difference: %d\n", roam_score_difference);
	log_info("Min evaluation interval: %d\n", min_evaluation_interval);
	log_info("Min roam interval: %d\n", min_roam_interval);
	log_info("Thoughput threshold: %d\n", throughput_threshold);

	max_num_device_allowed = easymesh_database.max_num_device_allowed;
	if (max_num_device_allowed < 2 || max_num_device_allowed > 16) {
		log_error("Invalid Max Number of Device Allowed! Range : 2 - 16. Value set by user : %d\n", max_num_device_allowed);
		free(buffer);
		return 1;
	}
	// if (0x03 != configure_state) {
	// 	// Either 2.4G or 5G or both is unconfigured
	// 	configure_state = configureDevice(easymesh_database.config_data_nr, easymesh_database.per_interface_config_data);
	// }

	if(!wfa_test) {
		configure_aps_func_off(1, easymesh_database.configured_band, &easymesh_database);
	}

	if (pthread_create(&map_thread, NULL, map_service_thread, &map_thread_data)) {
		fprintf(stderr, "Error creating thread\n");
		free(buffer);
		return 1;
	}

	if(wfa_test) {
		while (1) {
			if (-1 == mq_receive(mq, buffer, MAX_MESSAGE_SIZE, NULL)) {
				log_error("mq_receive() returned with errno=%d (%s)\n", errno, strerror(errno));
				exit(errno);
			}
		}
	}

	//User process for continuously reading user message queue

	log_debug("Registering time out event (periodic)...\n");
	{
		struct mapEventTimeOut aux;

		aux.timeout_ms = 1000; // 1 seconds
		aux.token      = 1;

		if (0 == MapRegisterQueueEvent(mq, MAP_QUEUE_EVENT_TIMEOUT_PERIODIC, &aux)) {
			log_error("Could not register timer callback\n");
			free(buffer);
			return 0;
		}
	}

	if (0 == MapRegisterQueueEvent(mq, MAP_QUEUE_EVENT_API_COMMAND, NULL)) {
		log_error("Could not register API callback\n");
		free(buffer);
		return 0;
	}

	struct device_info *info;
	info = map_get_device_information(easymesh_database.bridge_name);
	log_info("Device own al_mac %02x:%02x:%02x:%02x:%02x:%02x\n", info->al_mac[0], info->al_mac[1], info->al_mac[2], info->al_mac[3], info->al_mac[4], info->al_mac[5]);

	//initialize datamodel
	struct Network map_network;
	if (update_network(&map_network, info->al_mac)) {
		log_warn("Failed to initialize database! \n");
	}

	//------R1 DFS------------
	buffer_mib_channel(&easymesh_database);
	//------------------------
#ifdef PATHSEL
	path_db_init(info->al_mac);
#endif

	while (1) {
		uint8_t *pt;
		uint8_t  message_type;

		map_message *message;

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
		if (!self_query) {
			send_topology_query_request(info->al_mac);
			self_query = 1;
		}

		message_type = buffer[0];
		if (MAP_QUEUE_EVENT_TIMEOUT_PERIODIC == message_type) {
			// log_trace("MAP_QUEUE_EVENT_TIMEOUT_PERIODIC...\n");
			if (timer_counter >= 10) {
				optimize_neighbor(&map_network);
				optimize_datamodel(&map_network);
				dump_dataelement(&map_network);
				per_interval_action(&map_network, &easymesh_database);
				if (easymesh_database.bss_list_update) {
					obtain_5g_bss_list(&map_network, easymesh_database.radio_name_5gl, false);
				}
				timer_counter = 0;
			}
			do_controller(&map_network);
			timer_counter++;

			if (autoconfig_renew_count > 0 && autoconfig_renew_count < 255) {
				autoconfig_renew_count--;
			} else if (0 == autoconfig_renew_count) {
				int renew_ack = _check_renew_ack(map_network, easymesh_database.max_renew_resend_time);
				if (0 == renew_ack) {
					autoconfig_renew_count = 255;
				} else {
					autoconfig_renew_count = AUTOCONFIG_RENEW_WAIT_TIMEOUT;
				}
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

			if (channel_scan_flag) {
				channel_scan_timer++;
				if (channel_scan_timer >= 30) {
					log_debug("Channel Scan Timeout! write_channel_scan_report_file\n");
					write_channel_scan_report_file(&aggre_report);
					channel_scan_flag = 0;
					if (aggre_report.channels) {
						free(aggre_report.channels);
						aggre_report.channels = NULL;
					}
				}
			}

			if(check_channel_scan_command(&map_network, easymesh_database.hw_reg_domain)) {
				channel_scan_report_recv = 0;
				aggre_report.channel_nr = 0;
				channel_scan_flag = 1;
				channel_scan_timer = 0;
			}

			// if (cac_timer_counter >= 60) {
			// 	//---for test-------------------------------
			// 	uint8_t tmp_mac[6] = {0x00, 0xE0, 0x4C, 0x11, 0x33, 0x11};
			// 	struct cac_request cac_req;
			// 	cac_req.radio_nr        = 2;
			// 	cac_req.radios          = (struct cac_request_radio*)malloc(2 * sizeof(struct cac_request_radio));
			// 	{
			// 		memcpy(cac_req.radios[0].radio_id, tmp_mac, 6);
			// 		cac_req.radios[0].op_class	= 121;
			// 		cac_req.radios[0].channel	= 100;
			// 		cac_req.radios[0].flags		= 0b00001000;
			// 		memcpy(cac_req.radios[1].radio_id, tmp_mac, 6);
			// 		cac_req.radios[1].op_class	= 121;
			// 		cac_req.radios[1].channel	= 104;
			// 		cac_req.radios[1].flags		= 0b00001000;
			// 	}
			// 	//-----------------------------------------------
			// 	send_cac_request(tmp_mac, cac_req);
			//	free(cac_req.radios);
			// 	printf("send CAC request to %02x%02x%02x%02x%02x%02x!\n", tmp_mac[0], tmp_mac[1], tmp_mac[2], tmp_mac[3], tmp_mac[4], tmp_mac[5]);
			// 	cac_timer_counter = 0;
			// }
			// cac_timer_counter++;
			continue;
		}
		if (MAP_QUEUE_EVENT_API_COMMAND == message_type) {
			uint8_t command_type = buffer[3], command_content = buffer[4];
			uint16_t message_length = ((uint16_t)buffer[1] << 8) | buffer[2];
			if (message_length != 2) {
				break;
			}
			//printf("API command message: %c %c\n",command_type,command_content);
			if (command_type == 'c') {
				switch (command_content) {
				case '0': {
					roaming_enable = 0;
					reset_steer_state(&map_network);
					break;
				}
				case '1': {
					roaming_enable = 1;
					break;
				}
				default:
					break;
				}
			} else if (command_type == 'b') {
				switch (command_content) {
				case '0': {
					pathsel_enable = 0;
					break;
				}
				case '1': {
					pathsel_enable = 1;
					break;
				}
				default:
					break;
				}
			} else if (command_type == 'v') { // enable/disable console logging
				verbose_quiet = '1' - command_content;
				if (verbose_quiet) {
					disableLogDebug();
					map_disable_log();
				} else {
					enableLogDebug();
					map_enable_log();
				}
			}
		}
		if (MAP_API_RESPONSE != message_type) {
			log_warn("Unexpected message received...\n");
			continue;
		}

		// message_len  = (buffer[1] << 8) | buffer[2];

		message = map_parse_message((uint8_t *)buffer);

		//log_detail("Response type: %02x\n", message->message_type);
		//log_detail("Response length: %d\n", (int)message->message_len);
		//log_detail("Response length: %04x\n", message->message_len);
		//log_detail("Element ID: %02x\n", *message->element);
		switch (*message->element) {
		case MAP_GENERAL_ACKNOWLEDGEMENT_ELEMENT: {
			// log_trace("<-- MAP_GENERAL_ACKNOWLEDGEMENT_MSG\n");
			// struct map_general_acknowledgement *element = (struct map_general_acknowledgement *)message->element;
			// log_trace("Status %02x\n", element->status);
			break;
		}
		case MAP_TOPOLOGY_RESPONSE_ELEMENT: {
			map_response_element *element = (map_response_element *)message->element;
			//log_trace("<-- MAP_TOPOLOGY_RESPONSE_MSG\n");
			//log_trace("tlv_nr %02x\n", element->tlv_nr);

			//--------------------------------DATA ELEMENT -------------------------------------
			struct Device *curr_device = locate_device(&map_network, element->src_mac, NEED_CREATE_TRUE, NULL);
			if (NULL == curr_device) {
				log_warn("DE - MAP_TOPOLOGY_RESPONSE_ELEMENT : Error when locating device!\n");
				break;
				//---------CHANNEL SELECTION--------------
			} else if (0 == curr_device->number_of_radios) {
				//Ask controller for its own preference, assume this will always work
				if (!memcmp(map_network.controller_id, element->src_mac, 6)) {
					send_channel_preference_query(element->src_mac);
				}
				if (!easymesh_database.eth_only
						|| memcmp(map_network.controller_id, element->src_mac, 6)) {
					if (!easymesh_database.max_renew_resend_time) {
						uint8_t mcast_address[] = MCAST_1905;
						send_autoconfig_renew_request(mcast_address);
					} else if (curr_device->renew_send_count < easymesh_database.max_renew_resend_time) {
						curr_device->renew_send_count++;
						send_autoconfig_renew_request(element->src_mac);
					}
					autoconfig_renew_count = AUTOCONFIG_RENEW_WAIT_TIMEOUT;
				}
			}
			curr_device->keep_alive  = MAX_KEEP_ALIVE;
			//---------CHANNEL SELECTION--------------

			//--------------------------------DATA ELEMENT -------------------------------------

			//struct deviceBridgingCapabilityTLV ** bridging_cap_list      = NULL;
			//struct non1905NeighborDeviceListTLV **neighbor_non_1905_list = NULL;
			//struct neighborDeviceListTLV **       neighbor_1905_list     = NULL;

			uint8_t bridges_nr           = 0;
			uint8_t non1905_neighbors_nr = 0;

			int     i         = 0;
			uint8_t al_mac[6] = { 0 };
#ifdef PATHSEL
			uint8_t        x1905_neighbors_nr = 0;
			unsigned char  neighbor_num       = 0;
			unsigned char *neighbors          = NULL;
#endif

			struct deviceInformationTypeTLV *info_tlv = NULL;

			if (NULL == element->list_of_tlvs) {
				log_warn("Malformed structure.");
				break;
			}

			curr_device->non_1905_client_nr = 0;

			if (curr_device->non_1905_clients) {
				free(curr_device->non_1905_clients);
				curr_device->non_1905_clients = NULL;
			}

			//bridges_nr           = 0;
			//non1905_neighbors_nr = 0;
			//x1905_neighbors_nr   = 0;

			for (i = 0; i < element->tlv_nr; i++) {
				pt = element->list_of_tlvs[i];
				switch (*pt) {
				case TLV_TYPE_DEVICE_INFORMATION_TYPE: {
					//log_trace("<-- TLV_TYPE_DEVICE_INFORMATION_TYPE\n");
					info_tlv = (struct deviceInformationTypeTLV *)pt;
					memcpy(al_mac, info_tlv->al_mac_address, 6);
					break;
				}
				case TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES: {
					//log_trace("<-- TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES\n");
					//struct deviceBridgingCapabilityTLV *bridging_cap = (struct deviceBridgingCapabilityTLV *)pt;
					bridges_nr++;
					break;
				}
				case TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST: {
					//log_trace("<-- TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST\n");
					struct non1905NeighborDeviceListTLV *neighbor_non_1905 = (struct non1905NeighborDeviceListTLV *)pt;
					int j = 0, k = 0, skip = 0;
					for(j =0; j < neighbor_non_1905->non_1905_neighbors_nr; j++) {
						skip = 0;
						for(k =0; k < curr_device->non_1905_client_nr; k++) {
							if(0 == memcmp(curr_device->non_1905_clients[k].mac, neighbor_non_1905->non_1905_neighbors[j].mac_address, 6)) {
								skip = 1;
								break;
							}
						}
						if(skip)
							continue;
						curr_device->non_1905_client_nr += 1;
						curr_device->non_1905_clients = realloc(curr_device->non_1905_clients, curr_device->non_1905_client_nr * sizeof(struct Non1905Client));
						memcpy(curr_device->non_1905_clients[curr_device->non_1905_client_nr - 1].mac, neighbor_non_1905->non_1905_neighbors[j].mac_address, 6);
					}
					non1905_neighbors_nr++;
					break;
				}
				case TLV_TYPE_NEIGHBOR_DEVICE_LIST: {
					//log_trace("<-- TLV_TYPE_NEIGHBOR_DEVICE_LIST\n");
					struct neighborDeviceListTLV *neighbor_1905 = (struct neighborDeviceListTLV *)pt;
					//--------------------------------DATA ELEMENT -------------------------------------
					if (curr_device) {
						update_device_neighbor(map_network, curr_device, neighbor_1905, easymesh_database.customize_features);
						curr_device->path_score = get_path_score(curr_device->id);
					}
#ifdef PATHSEL
					neighbor_num = neighbor_1905->neighbors_nr;
					if (neighbor_num <= 0)
						break;
					int num;
					if (x1905_neighbors_nr == 0) {
						neighbors = calloc((neighbor_num)*6, sizeof(unsigned char));
					} else {
						neighbors = realloc(neighbors, (x1905_neighbors_nr + neighbor_num) * 6 * sizeof(unsigned char));
					}
					for (num = 0; num < neighbor_num; num++) {
						//log_debug(" %02x:%02x:%02x:%02x:%02x:%02x 's neighbor %d is %02x:%02x:%02x:%02x:%02x:%02x \n", neighbor_1905->local_mac_address[0], neighbor_1905->local_mac_address[1], neighbor_1905->local_mac_address[2] , neighbor_1905->local_mac_address[3], neighbor_1905->local_mac_address[4], neighbor_1905->local_mac_address[5], num, neighbor_1905->neighbors[num].mac_address[0] , neighbor_1905->neighbors[num].mac_address[1], neighbor_1905->neighbors[k].mac_address[2], neighbor_1905->neighbors[num].mac_address[3] , neighbor_1905->neighbors[num].mac_address[4] , neighbor_1905->neighbors[num].mac_address[5]);
						memcpy(neighbors + x1905_neighbors_nr * 6 + num * 6, neighbor_1905->neighbors[num].mac_address, 6);
					}
					// log_debug("finished");
					x1905_neighbors_nr += neighbor_num;
#endif
					break;
				}
				case TLV_TYPE_SUPPORTED_SERVICE: {
					//log_trace("<-- TLV_TYPE_SUPPORTED_SERVICE\n");
					//struct SupportedServiceTLV *supported_service = (struct SupportedServiceTLV *)pt;
					break;
				}
				case TLV_TYPE_AP_OPERATIONAL_BSS: {
					//log_trace("<-- TLV_TYPE_AP_OPERATIONAL_BSS\n");
					struct ApOperationalBSSTLV *operational_bss = (struct ApOperationalBSSTLV *)pt;
					update_device_operational_bss(curr_device, operational_bss);

					//--------------------------------VLAN CONFIGURATION -------------------------------
					if (curr_device && (VLAN_CONFIGURE_NOT_READY == curr_device->need_vlan_configure)) {
						if(memcmp(map_network.controller_id, curr_device->id, 6))
							update_need_vlan_configure(&map_network, curr_device);
					}
					//--------------------------------VLAN CONFIGURATION -------------------------------
					break;
				}
				case TLV_TYPE_ASSOCIATED_CLIENTS: {
					// log_trace("<-- TLV_TYPE_ASSOCIATED_CLIENTS\n");
					struct AssociatedClientsTLV *associated_clients = (struct AssociatedClientsTLV *)pt;
					update_device_associated_clients(map_network, curr_device, associated_clients);
					break;
				}
				case TLV_TYPE_VENDOR_SPECIFIC: {
					log_trace("<-- TLV_TYPE_VENDOR_SPECIFIC\n");
					struct vendorSpecificTLV *t = (struct vendorSpecificTLV *)pt;
					//--------------------------------DATA ELEMENT -------------------------------------
					if (curr_device) {
						update_device(curr_device, NULL, t, NULL);
					}
#if 1//def RTK_ETHERNET_ADAPTER
					if(easymesh_database.customize_features & SUPPORT_ETHERNET_ADAPTER)
					{
						if (curr_device) {
							if (!memcmp(t->vendor_oui, REALTEK_OUI, 3)) {
								map_process_topo_response_vendor_data(t->m, t->m_nr);
							}
						}
					}
#endif
					//--------------------------------DATA ELEMENT -------------------------------------
					break;
				}
				case TLV_TYPE_MULTIAP_PROFILE: {
					log_trace("<-- TLV_TYPE_MULTIAP_PROFILE\n");
					struct MultiAPProfileTLV *t = (struct MultiAPProfileTLV *)pt;
					//--------------------------------DATA ELEMENT -------------------------------------
					if (curr_device) {
						update_device(curr_device, NULL, NULL, t);
					}
					//--------------------------------DATA ELEMENT -------------------------------------
					break;
				}
				default: {
					// log_warn("Unexpected TLV (%02x) type inside CMDU\n", *pt);
					break;
				}
				}
			}
			optimize_datamodel(&map_network);

			if (info_tlv) {
				update_device_info(curr_device, info_tlv, easymesh_database.radio_number, &map_network);
			}
#ifdef PATHSEL
			if (x1905_neighbors_nr > 0) {
				unsigned char temp[6] = { 0 };
				if (memcmp(al_mac, temp, 6)) {
					update_node_neighbours(al_mac, neighbors, x1905_neighbors_nr, true);
				}
			}
			if (neighbors) {
				free(neighbors);
			}
#endif
			break;
		}
		case MAP_BACKHAUL_STEERING_RESPONSE_ELEMENT: {
			map_response_element *element = (map_response_element *)message->element;
			log_trace("<-- MAP_BACKHAUL_STEERING_RESPONSE_MSG\n");
			log_trace("tlv_nr %02x\n", element->tlv_nr);

			int i = 0;

			if (NULL == element->list_of_tlvs) {
				log_warn("Malformed structure.");
				break;
			}

			///////////////////////////////////////////////////////////////////
			//For now tlv_nr can only be 1 as Error Code is not yet implemented.
			//There might be a tlv_nr greater than 1 in the future where one or
			//more error code tlvs are included from the 2nd tlv onwards
			///////////////////////////////////////////////////////////////////
			for (i = 0; i < element->tlv_nr; i++) {
				pt = element->list_of_tlvs[i];
				switch (*pt) {
				case TLV_TYPE_BACKHAUL_STEERING_RESPONSE: {
					log_trace("<-- TLV_TYPE_BACKHAUL_STEERING_RESPONSE\n");
					struct BackhaulSteeringResponseTLV *backhaul_steering_response_tlv = (struct BackhaulSteeringResponseTLV *)pt;
					//handle backhaul steering response here
					log_detail("	tlv_type%d, backhaul_mac%02x%02x%02x%02x%02x%02x, target_bssid%02x%02x%02x%02x%02x%02x, result_code%d",
					         backhaul_steering_response_tlv->tlv_type, backhaul_steering_response_tlv->backhaul_mac[0], backhaul_steering_response_tlv->backhaul_mac[1], backhaul_steering_response_tlv->backhaul_mac[2],
					         backhaul_steering_response_tlv->backhaul_mac[3], backhaul_steering_response_tlv->backhaul_mac[4], backhaul_steering_response_tlv->backhaul_mac[5], backhaul_steering_response_tlv->target_bssid[0],
					         backhaul_steering_response_tlv->target_bssid[1], backhaul_steering_response_tlv->target_bssid[2], backhaul_steering_response_tlv->target_bssid[3], backhaul_steering_response_tlv->target_bssid[4], backhaul_steering_response_tlv->target_bssid[5], backhaul_steering_response_tlv->result_code);

					break;
				}
				case TLV_TYPE_ERROR_CODE: {
					log_trace("<-- TLV_TYPE_ERROR_CODE\n");
					// struct ErrorCodeTLV *error_code_tlv = (struct ErrorCodeTLV *)pt;
					//handle error code here

					break;
				}
				default: {
					// log_warn("Unexpected TLV (%02x) type inside CMDU\n", *pt);
					break;
				}
				}
			}
			break;
		}
		case MAP_LINK_METRIC_RESPONSE_ELEMENT: {
			map_response_element *element = (map_response_element *)message->element;
			//log_trace("<-- MAP_LINK_METRIC_RESPONSE_MSG\n");
			//log_trace("tlv_nr %02x\n", element->tlv_nr);

			int i = 0, j = 0;

			//--------------------------------DATA ELEMENT -------------------------------------
			struct Device *curr_device = locate_device(&map_network, element->src_mac, NEED_CREATE_FALSE, NULL);
			if (NULL == curr_device) {
				log_error("DE - MAP_LINK_METRIC_RESPONSE_ELEMENT : Device not found! Skip data element update...\n");
				break;
			}
			curr_device->keep_alive  = MAX_KEEP_ALIVE;
			//--------------------------------DATA ELEMENT -------------------------------------

			if (NULL == element->list_of_tlvs) {
				log_warn("Malformed structure.");
				break;
			}

			///////////////////////////////////////////////////////////////////////////
			//There might be      0 or more Tx Link TLVs      AND     0 or more Rx Link
			//TLVs corresponding to how the request is sent
			///////////////////////////////////////////////////////////////////////////
			for (i = 0; i < element->tlv_nr; i++) {
				pt = element->list_of_tlvs[i];
				switch (*pt) {
				case TLV_TYPE_TRANSMITTER_LINK_METRIC: {
					log_trace("<-- TLV_TYPE_TRANSMITTER_LINK_METRIC\n");
					struct transmitterLinkMetricTLV *transmitter_link_metric_tlv = (struct transmitterLinkMetricTLV *)pt;

					//--------------------------------DATA ELEMENT -------------------------------------
					struct Device *neighbor_device = locate_device(&map_network, transmitter_link_metric_tlv->neighbor_al_address, NEED_CREATE_FALSE, NULL);
					log_trace("<-- TLV_TYPE_TRANSMITTER_LINK_METRIC %d\n", transmitter_link_metric_tlv->transmitter_link_metrics_nr);
					if (neighbor_device && curr_device) {
						for (j = 0; j < transmitter_link_metric_tlv->transmitter_link_metrics_nr; j++) {
							if (locate_radio(curr_device, NULL, 0, transmitter_link_metric_tlv->transmitter_link_metrics[j].local_interface_address, NEED_CREATE_FALSE, NULL)) {
								continue;
							}
							struct Radio *curr_radio = locate_backhaul_radio(curr_device, neighbor_device, transmitter_link_metric_tlv->transmitter_link_metrics[j].neighbor_interface_address);
							if (curr_radio) {
								update_radio(curr_radio, NULL, NULL, NULL, transmitter_link_metric_tlv->transmitter_link_metrics[j].local_interface_address);
							}
						}
					}
					//--------------------------------DATA ELEMENT -------------------------------------

					break;
				}
				case TLV_TYPE_RECEIVER_LINK_METRIC: {
					struct receiverLinkMetricTLV *receiver_link_metric_tlv = (struct receiverLinkMetricTLV *)pt;
					log_trace("<-- TLV_TYPE_RECEIVER_LINK_METRIC %d\n", receiver_link_metric_tlv->receiver_link_metrics_nr);
					int num;
					uint8_t rssi;
					for (num = 0; num < receiver_link_metric_tlv->receiver_link_metrics_nr; num++) {
						rssi = receiver_link_metric_tlv->receiver_link_metrics[num].rssi;
						if (receiver_link_metric_tlv->receiver_link_metrics[num].intf_type == MEDIA_TYPE_IEEE_802_3U_FAST_ETHERNET || MEDIA_TYPE_IEEE_802_3AB_GIGABIT_ETHERNET == receiver_link_metric_tlv->receiver_link_metrics[num].intf_type) {
							rssi = 100;
							update_metric(receiver_link_metric_tlv->receiver_link_metrics[num].neighbor_interface_address, receiver_link_metric_tlv->receiver_link_metrics[num].local_interface_address, rssi);
						} else {
							update_metric(receiver_link_metric_tlv->receiver_link_metrics[num].local_interface_address, receiver_link_metric_tlv->receiver_link_metrics[num].neighbor_interface_address, rssi);
						}
					}

					//--------------------------------DATA ELEMENT -------------------------------------
					struct Device *neighbor_device = locate_device(&map_network, receiver_link_metric_tlv->neighbor_al_address, NEED_CREATE_FALSE, NULL);
					log_trace("<-- TLV_TYPE_RECEIVER_LINK_METRIC %d\n", receiver_link_metric_tlv->receiver_link_metrics_nr);
					if (neighbor_device && curr_device) {
						for (j = 0; j < receiver_link_metric_tlv->receiver_link_metrics_nr; j++) {
							if (locate_radio(curr_device, NULL, 0, receiver_link_metric_tlv->receiver_link_metrics[j].local_interface_address, NEED_CREATE_FALSE, NULL)) {
								continue;
							}
							struct Radio *curr_radio = locate_backhaul_radio(curr_device, neighbor_device, receiver_link_metric_tlv->receiver_link_metrics[j].neighbor_interface_address);
							if (curr_radio) {
								update_radio(curr_radio, NULL, NULL, NULL, receiver_link_metric_tlv->receiver_link_metrics[j].local_interface_address);
							}
						}
					}
					//--------------------------------DATA ELEMENT -------------------------------------

					break;
				}
				default: {
					// log_warn("Unexpected TLV (%02x) type inside CMDU\n", *pt);
					break;
				}
				}
			}
			break;
		}
		case MAP_AP_CAPABILITY_REPORT_ELEMENT: {
			map_response_element *element = (map_response_element *)message->element;
			//log_trace("<-- MAP_AP_CAPABILITY_RESPONSE_MSG\n");
			//log_trace("tlv_nr %02x\n", element->tlv_nr);

			int i = 0;

			//--------------------------------DATA ELEMENT -------------------------------------
			struct Device *curr_device = locate_device(&map_network, element->src_mac, NEED_CREATE_FALSE, NULL);
			if (NULL == curr_device) {
				log_warn("DE - MAP_AP_CAPABILITY_REPORT_ELEMENT : Device not found! Skip data element update...\n");
				break;
			}
			curr_device->keep_alive  = MAX_KEEP_ALIVE;
			//--------------------------------DATA ELEMENT -------------------------------------

			if (NULL == element->list_of_tlvs) {
				log_warn("Malformed structure.");
				break;
			}

			for (i = 0; i < element->tlv_nr; i++) {
				pt = element->list_of_tlvs[i];
				switch (*pt) {
				case TLV_TYPE_AP_CAPABILITY: {
					struct APCapabilityTLV *t = (struct APCapabilityTLV *)pt;
					//log_trace("<-- TLV_TYPE_AP_CAPABILITY\n");

					//--------------------------------DATA ELEMENT -------------------------------------
					if (curr_device) {
						update_device(curr_device, t, NULL, NULL);
					}

					//--------------------------------DATA ELEMENT -------------------------------------

					break;
				}
				case TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES: {
					//log_trace("<-- TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES\n");
					struct APRadioBasicCapabilitiesTLV *t = (struct APRadioBasicCapabilitiesTLV *)pt;

					//--------------------------------DATA ELEMENT -------------------------------------
					if (curr_device) {
						struct Radio *curr_radio = locate_radio(curr_device, t->radio_unique_id, 0, NULL, NEED_CREATE_FALSE, NULL);
						if (NULL == curr_radio) {
							log_warn("DE - TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES : Radio not found! Skip data element update...\n");
						} else {
							update_radio(curr_radio, NULL, NULL, NULL, NULL);
						}
					}

					//--------------------------------DATA ELEMENT -------------------------------------
					break;
				}
				case TLV_TYPE_AP_HT_CAPABILITIES: {
					//log_trace("<-- TLV_TYPE_AP_HT_CAPABILITIES\n");
					struct APHTCapabilitiesTLV *t = (struct APHTCapabilitiesTLV *)pt;

					//--------------------------------DATA ELEMENT -------------------------------------
					if (curr_device) {
						struct Radio *curr_radio = locate_radio(curr_device, t->radio_unique_id, 0, NULL, NEED_CREATE_FALSE, NULL);
						if (NULL == curr_radio) {
							log_warn("DE - TLV_TYPE_AP_HT_CAPABILITIES : Radio not found! Skip data element update...\n");
						} else {
							update_radio(curr_radio, NULL, NULL, NULL, NULL);
						}
					}
					//--------------------------------DATA ELEMENT -------------------------------------

					break;
				}
				case TLV_TYPE_AP_VHT_CAPABILITIES: {
					//log_trace("<-- TLV_TYPE_AP_VHT_CAPABILITIES\n");
					struct APVHTCapabilitiesTLV *t = (struct APVHTCapabilitiesTLV *)pt;

					//--------------------------------DATA ELEMENT -------------------------------------
					if (curr_device) {
						struct Radio *curr_radio = locate_radio(curr_device, t->radio_unique_id, 0, NULL, NEED_CREATE_FALSE, NULL);
						if (NULL == curr_radio) {
							log_warn("DE - TLV_TYPE_AP_VHT_CAPABILITIES : Radio not found! Skip data element update...\n");
						} else {
							update_radio(curr_radio, NULL, NULL, NULL, NULL);
						}
					}
					//--------------------------------DATA ELEMENT -------------------------------------

					break;
				}
				// TODO: case TLV_TYPE_AP_HE_CAPABILITIES 
				default: {
					// log_warn("Unexpected TLV (%02x) type inside CMDU\n", *pt);
					break;
				}
				}
			}

			break;
		}
		case MAP_CLIENT_CAPABILITY_REPORT_ELEMENT: {
			map_response_element *element = (map_response_element *)message->element;
			//log_trace("<-- MAP_CLIENT_CAPABILITY_REPORT_ELEMENT\n");
			//log_trace("tlv_nr %02x\n", element->tlv_nr);

			int i = 0;
			if (NULL == element->list_of_tlvs) {
				log_warn("Malformed structure.");
				break;
			}

			struct Device *curr_device = locate_device(&map_network, element->src_mac, NEED_CREATE_FALSE, NULL);
			if (curr_device == NULL) {
				log_warn("DE - MAP_CLIENT_CAPABILITY_REPORT_ELEMENT : Device not found! Skip data element update...\n");
				break;
			}
			curr_device->keep_alive  = MAX_KEEP_ALIVE;
			struct STA *curr_sta = NULL;
			for (i = 0; i < element->tlv_nr; i++) {
				pt = element->list_of_tlvs[i];
				switch (*pt) {
				case TLV_TYPE_CLIENT_INFO: {
					log_trace("<-- TLV_TYPE_CLIENT_INFO\n");
					struct ClientInfoTLV *t          = (struct ClientInfoTLV *)pt;
					struct Radio *        curr_radio = locate_radio(curr_device, NULL, 0, t->bssid, NEED_CREATE_FALSE, NULL);
					if (curr_radio) {
						struct StructBSS *curr_bss = locate_bss(curr_radio, t->bssid, NEED_CREATE_FALSE, NULL);
						if (curr_bss) {
							curr_sta = locate_sta(curr_bss, t->mac_address, NEED_CREATE_FALSE, NULL);
						} else {
							log_warn("DE - MAP_CLIENT_CAPABILITY_REPORT_ELEMENT : Bss not found! Skip data element update...\n");
						}
					} else {
						log_warn("DE - MAP_CLIENT_CAPABILITY_REPORT_ELEMENT : Radio not found! Skip data element update...\n");
					}
					break;
				}
				case TLV_TYPE_CLIENT_CAPABLITY_REPORT: {
					log_trace("<-- TLV_TYPE_CLIENT_CAPABLITY_REPORT\n");
					struct ClientCapabilityReportTLV *t = (struct ClientCapabilityReportTLV *)pt;
					if (curr_sta) {
						if (t->result_code == 0) {
							uint8_t  len    = 0;
							uint8_t *rm_cap = get_ie_pointer(t->frame_body_length, t->frame_body, RM_ENABLE_CAP_IE, &len);
							if (rm_cap) {
								memcpy(curr_sta->rm_cap, rm_cap + 2, 5);
								// log_detail("RM cap: %02x%02x%02x%02x%02x\n", curr_sta->rm_cap[0], curr_sta->rm_cap[1],
								// 	curr_sta->rm_cap[2], curr_sta->rm_cap[3], curr_sta->rm_cap[4]);
							}
							uint8_t *mbo_cap = check_mbo_capability(t->frame_body_length, t->frame_body);
							if(mbo_cap) {
								curr_sta->mbo_cap = 1;
								printf("MBO capable STA found from TLV_TYPE_CLIENT_CAPABLITY_REPORT!");
							}

							curr_sta->support_bss_transition = check_bss_transition_support(t->frame_body, t->frame_body_length);
							// log_debug("Station %02x:%02x:%02x:%02x:%02x:%02x with bss trans support %d\n", curr_sta->mac_address[0], curr_sta->mac_address[1], curr_sta->mac_address[2], curr_sta->mac_address[3], curr_sta->mac_address[4], curr_sta->mac_address[5], curr_sta->support_bss_transition);
						}
						curr_sta->capability_recv = 1;
					}
					break;
				}
				default: {
					// log_warn("Unexpected TLV (%02x) type inside CMDU\n", *pt);
					break;
				}
				}
			}

			break;
		}
		case MAP_AP_METRIC_RESPONSE_ELEMENT: {
			map_response_element *element = (map_response_element *)message->element;
			// log_trace("<-- MAP_AP_METRIC_RESPONSE_MSG\n");
			// log_trace("tlv_nr %02x\n", element->tlv_nr);

			//--------------------------------DATA ELEMENT -------------------------------------
			struct Device *curr_device = locate_device(&map_network, element->src_mac, NEED_CREATE_FALSE, NULL);
			if (NULL == curr_device) {
				log_warn("DE - MAP_AP_METRIC_RESPONSE_ELEMENT : Device not found! Skip data element update...\n");
			} else {
				time(&curr_device->updated_timestamp);
				curr_device->keep_alive  = MAX_KEEP_ALIVE;
			}
			//--------------------------------DATA ELEMENT -------------------------------------

			int i = 0;

			if (NULL == element->list_of_tlvs) {
				log_warn("Malformed structure.");
				break;
			}

			for (i = 0; i < element->tlv_nr; i++) {
				pt = element->list_of_tlvs[i];
				switch (*pt) {
				case TLV_TYPE_AP_METRICS: {
					// log_trace("<-- TLV_TYPE_AP_METRICS\n");
					struct APMetricsTLV *t = (struct APMetricsTLV *)pt;

					//--------------------------------DATA ELEMENT -------------------------------------
					if (curr_device) {
						struct Radio *curr_radio = locate_radio(curr_device, NULL, 0, t->bssid, NEED_CREATE_FALSE, NULL);
						if (NULL == curr_radio) {
							// log_warn("DE - TLV_TYPE_AP_METRICS : Radio not found! Skip data element update...\n");
						} else {
							update_radio(curr_radio, NULL, NULL, NULL, NULL);
							struct StructBSS *curr_bss = locate_bss(curr_radio, t->bssid, NEED_CREATE_FALSE, NULL);
							if (NULL == curr_bss) {
								log_warn("DE - TLV_TYPE_AP_METRICS : BSS not found! Skip data element update...\n");
							} else {
								update_bss(curr_bss, NULL, 0, t);
							}
						}
					}
					//--------------------------------DATA ELEMENT -------------------------------------

					break;
				}
				case TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS: {
					// log_trace("<-- TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS\n");
					struct AssociatedSTATrafficStatsTLV *t = (struct AssociatedSTATrafficStatsTLV *)pt;

					//--------------------------------DATA ELEMENT -------------------------------------
					if (curr_device) {
						struct STA *curr_sta = locate_sta_by_mac(NULL, curr_device, t->sta_mac_address, NULL, NULL, NULL);
						if (NULL == curr_sta) {
							log_warn("DE - TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS : STA not found! Skip data element update...\n");
						} else {
							update_sta(curr_sta, NULL, t, NULL);
						}
					}
					//--------------------------------DATA ELEMENT -------------------------------------

					break;
				}
				case TLV_TYPE_ASSOCIATED_STA_LINK_METRICS: {
					// log_trace("<-- TLV_TYPE_ASSOCIATED_STA_LINK_METRICS\n");
					struct AssociatedSTALinkMetricsTLV *t = (struct AssociatedSTALinkMetricsTLV *)pt;

					//--------------------------------DATA ELEMENT -------------------------------------
					if (curr_device && t->bssid_nr) {
						struct Radio *curr_radio = locate_radio(curr_device, NULL, 0, t->assoc_sta_link_metrics[0].bssid, NEED_CREATE_FALSE, NULL);
						if (NULL == curr_radio) {
							log_warn("DE - TLV_TYPE_ASSOCIATED_STA_LINK_METRICS : Radio not found! Skip data element update...\n");
						} else {
							struct StructBSS *curr_bss = locate_bss(curr_radio, t->assoc_sta_link_metrics[0].bssid, NEED_CREATE_FALSE, NULL);
							if (NULL == curr_bss) {
								log_warn("DE - TLV_TYPE_ASSOCIATED_STA_LINK_METRICS : BSS not found! Skip data element update...\n");
							} else {
								struct STA *curr_sta = locate_sta(curr_bss, t->mac_address, NEED_CREATE_FALSE, NULL);
								if (NULL == curr_sta) {
									log_warn("DE - TLV_TYPE_ASSOCIATED_STA_LINK_METRICS : STA not found! Skip data element update...\n");
								} else {
									//t->bssid-nr always = 1
									update_sta(curr_sta, t->assoc_sta_link_metrics, NULL, NULL);
								}
							}
						}
					}
					//--------------------------------DATA ELEMENT -------------------------------------

					break;
				}
				default: {
					// log_warn("Unexpected TLV (%02x) type inside CMDU\n", *pt);
					break;
				}
				}
			}
			break;
		}
		case MAP_ASSOC_STA_METRIC_RESPONSE_ELEMENT: {
			map_response_element *element = (map_response_element *)message->element;
			//log_trace("<-- MAP_ASSOC_STA_METRIC_RESPONSE_MSG\n");
			//log_trace("tlv_nr %02x\n", element->tlv_nr);

			//--------------------------------DATA ELEMENT -------------------------------------
			struct Device *curr_device = locate_device(&map_network, element->src_mac, NEED_CREATE_FALSE, NULL);
			if (NULL == curr_device) {
				log_warn("DE - MAP_ASSOC_STA_METRIC_RESPONSE_ELEMENT : Device not found! Skip data element update...\n");
			} else {
				curr_device->keep_alive  = MAX_KEEP_ALIVE;
			}

			//--------------------------------DATA ELEMENT -------------------------------------

			int i = 0;

			if (NULL == element->list_of_tlvs) {
				log_warn("Malformed structure.");
				break;
			}

			for (i = 0; i < element->tlv_nr; i++) {
				pt = element->list_of_tlvs[i];
				switch (*pt) {
				case TLV_TYPE_ASSOCIATED_STA_LINK_METRICS: {
					//log_trace("<-- TLV_TYPE_ASSOCIATED_STA_LINK_METRICSs\n");
					struct AssociatedSTALinkMetricsTLV *t = (struct AssociatedSTALinkMetricsTLV *)pt;

					//--------------------------------DATA ELEMENT -------------------------------------
					if (curr_device && t->bssid_nr) {
						struct Radio *curr_radio = locate_radio(curr_device, NULL, 0, t->assoc_sta_link_metrics[0].bssid, NEED_CREATE_FALSE, NULL);
						if (NULL == curr_radio) {
							log_warn("DE - TLV_TYPE_ASSOCIATED_STA_LINK_METRICS : Radio not found! Skip data element update...\n");
						} else {
							struct StructBSS *curr_bss = locate_bss(curr_radio, t->assoc_sta_link_metrics[0].bssid, NEED_CREATE_FALSE, NULL);
							if (NULL == curr_bss) {
								log_warn("DE - TLV_TYPE_ASSOCIATED_STA_LINK_METRICS : BSS not found! Skip data element update...\n");
							} else {
								struct STA *curr_sta = locate_sta(curr_bss, t->mac_address, NEED_CREATE_FALSE, NULL);
								if (NULL == curr_sta) {
									log_warn("DE - TLV_TYPE_ASSOCIATED_STA_LINK_METRICS : STA not found! Skip data element update...\n");
								} else {
									//t->bssid-nr always = 1
									update_sta(curr_sta, t->assoc_sta_link_metrics, NULL, NULL);
								}
							}
						}
					}
					//--------------------------------DATA ELEMENT -------------------------------------
					break;
				}
				default: {
					// log_warn("Unexpected TLV (%02x) type inside CMDU\n", *pt);
					break;
				}
				}
			}

			break;
		}
		case MAP_UNASSOC_STA_METRIC_RESPONSE_ELEMENT: {
			map_response_element *element = (map_response_element *)message->element;
			// log_trace("<-- MAP_UNASSOC_STA_METRIC_RESPONSE_MSG\n");
			// log_trace("tlv_nr %02x\n", element->tlv_nr);

			//--------------------------------DATA ELEMENT -------------------------------------
			struct Device *curr_device = locate_device(&map_network, element->src_mac, NEED_CREATE_FALSE, NULL);
			if (NULL == curr_device) {
				log_warn("DE - MAP_UNASSOC_STA_METRIC_RESPONSE_ELEMENT : Device not found! Skip data element update...\n");
			} else {
				curr_device->keep_alive  = MAX_KEEP_ALIVE;
			}

			//--------------------------------DATA ELEMENT -------------------------------------

			int i = 0;
			//int j; // k;

			if (NULL == element->list_of_tlvs) {
				log_warn("Malformed structure.");
				break;
			}

			for (i = 0; i < element->tlv_nr; i++) {
				pt = element->list_of_tlvs[i];
				switch (*pt) {
				case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE: {
					// log_trace("<-- TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE\n");
					struct UnassociatedSTALinkMetricsResponseTLV *t = (struct UnassociatedSTALinkMetricsResponseTLV *)pt;

					//--------------------------------DATA ELEMENT -------------------------------------
					if (curr_device) {
						int           path_score = get_path_score(curr_device->id);
						struct Radio *curr_radio = locate_radio(curr_device, NULL, t->op_class, NULL, NEED_CREATE_FALSE, NULL);
						if (NULL == curr_radio) {
							log_warn("DE - TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE : Radio not found! Skip data element update...\n");
						} else {
							update_radio(curr_radio, NULL, NULL, t, NULL);
							if(roaming_enable) {
								update_roam_info(&map_network, curr_radio, path_score);
							}
						}
					}

#ifdef PATHSEL
					if (t->sta_nr) {
						// log_info("Number of Stations: %d\n", t->sta_nr);
						int f;
						for (f = 0; f < t->sta_nr; f++) {
							//  log_info("Sta mac: %02x%02x%02x%02x%02x%02x Rssi: %d\n",
							//         t->unassoc_sta_link_metrics[f].sta_mac_address[0],
							//         t->unassoc_sta_link_metrics[f].sta_mac_address[1],
							//        t->unassoc_sta_link_metrics[f].sta_mac_address[2],
							//         t->unassoc_sta_link_metrics[f].sta_mac_address[3],
							//         t->unassoc_sta_link_metrics[f].sta_mac_address[4],
							//         t->unassoc_sta_link_metrics[f].sta_mac_address[5],
							//         t->unassoc_sta_link_metrics[f].uplink_rcpi);
							struct Device *device  = locate_device_by_bss_mac(&map_network, t->unassoc_sta_link_metrics[f].sta_mac_address);
							if (device && (find_node_index_by_id(device->id) != -1)) {
								// log_info("!!!! %02x%02x%02x%02x%02x%02x", element->src_mac[0], element->src_mac[1], element->src_mac[2], element->src_mac[3], element->src_mac[4], element->src_mac[5]);
								update_node_neighbours(element->src_mac, device->id, 1, false);
								update_metric(element->src_mac, device->id, rcpi_to_rssi(t->unassoc_sta_link_metrics[f].uplink_rcpi));
								/*else
									log_info("Oops, cannot find for %02x%02x%02x%02x%02x%02x, this should never happen",
									         t->unassoc_sta_link_metrics[f].sta_mac_address[0],
									         t->unassoc_sta_link_metrics[f].sta_mac_address[1],
									         t->unassoc_sta_link_metrics[f].sta_mac_address[2],
									         t->unassoc_sta_link_metrics[f].sta_mac_address[3],
									         t->unassoc_sta_link_metrics[f].sta_mac_address[4],
									         t->unassoc_sta_link_metrics[f].sta_mac_address[5]);*/
							}
						}
					}
#endif
					//--------------------------------DATA ELEMENT -------------------------------------
					break;
				}
				default: {
					// log_warn("Unexpected TLV (%02x) type inside CMDU\n", *pt);
					break;
				}
				}
			}

			break;
		}
		case MAP_BEACON_METRIC_RESPONSE_ELEMENT: {
			map_response_element *element = (map_response_element *)message->element;
			// log_trace("<-- MAP_BEACON_METRIC_RESPONSE_MSG\n");
			// log_trace("tlv_nr %02x\n", element->tlv_nr);

			int i = 0;

			//--------------------------------DATA ELEMENT -------------------------------------
			struct Device *curr_device = locate_device(&map_network, element->src_mac, NEED_CREATE_FALSE, NULL);
			if (NULL == curr_device) {
				log_warn("DE - MAP_BEACON_METRIC_RESPONSE_ELEMENT : Device not found! Skip data element update...\n");
			}

			//--------------------------------DATA ELEMENT -------------------------------------

			if (NULL == element->list_of_tlvs) {
				log_warn("Malformed structure.");
				break;
			}

			for (i = 0; i < element->tlv_nr; i++) {
				pt = element->list_of_tlvs[i];
				switch (*pt) {
				case TLV_TYPE_BEACON_METRICS_RESPONSE: {
					// log_trace("<-- TLV_TYPE_BEACON_METRICS_RESPONSE\n");
					struct BeaconMetricsResponseTLV *t = (struct BeaconMetricsResponseTLV *)pt;

					//--------------------------------DATA ELEMENT -------------------------------------
					if (curr_device) {
						struct StructBSS *curr_sta_bss;
						struct STA *      curr_sta = locate_sta_by_mac(NULL, curr_device, t->mac_address, NULL, NULL, &curr_sta_bss);
						if (NULL == curr_sta) {
							log_warn("DE - MAP_BEACON_METRIC_RESPONSE_ELEMENT : STA not found! Skip data element update...\n");
						} else {
#if CLIENT_STEER_ENABLED
							if (roaming_enable && curr_sta_bss)
								update_11k_roam_info(&map_network, curr_device, t, curr_sta_bss, curr_sta);
#endif
						}
					}
					//--------------------------------DATA ELEMENT -------------------------------------

					break;
				}
				default: {
					// log_warn("Unexpected TLV (%02x) type inside CMDU\n", *pt);
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

			int i = 0, k = 0;
			uint8_t is_from_controller = !memcmp(map_network.controller_id, element->src_mac, 6);
			struct Device *curr_device = locate_device(&map_network, element->src_mac, NEED_CREATE_FALSE, NULL);

			if (NULL == curr_device) {
				log_warn("DE - MAP_CHANNEL_PREFERENCE_REPORT_ELEMENT : Device not found! Skip data element update...\n");
			}

			if (is_from_controller) {
				controller_ch_pref_updated = 0;
				_free_channel_preference(controller_ch_pref, controller_ch_pref_number);
				controller_ch_pref        = NULL;
				controller_ch_pref_number = 0;
			}

			if (NULL == element->list_of_tlvs) {
				log_warn("Malformed structure.");
				break;
			}
			for (i = 0; i < element->tlv_nr; i++) {
				pt = element->list_of_tlvs[i];
				switch (*pt) {
				case TLV_TYPE_CHANNEL_PREFERENCE: {
					log_trace("<-- TLV_TYPE_CHANNEL_PREFERENCE\n");
					//---------CHANNEL SELECTION--------------
					struct ChannelPreferenceTLV *t = (struct ChannelPreferenceTLV *)pt;

					//if it is from controller
					if (is_from_controller) {
						char *radio_name = NULL;
						if (!memcmp(t->radio_unique_id, easymesh_database.radio_24g_mac, 6)) {
							radio_name = easymesh_database.radio_name_24g;
						} else if (!memcmp(t->radio_unique_id, easymesh_database.radio_5gl_mac, 6)) {
							radio_name = easymesh_database.radio_name_5gl;
						} else if (easymesh_database.radio_number == 3 && (!memcmp(t->radio_unique_id, easymesh_database.radio_5gh_mac, 6))){
							radio_name = easymesh_database.radio_name_5gh;
						} else {
							log_error("Fail to map channel preference tlv with radio \n");
							continue;
						}
						controller_ch_pref_number++;
						controller_ch_pref = (struct channel_preference *)realloc(controller_ch_pref, sizeof(struct channel_preference) * controller_ch_pref_number);
						_update_controller_ch_pref(&controller_ch_pref[controller_ch_pref_number - 1], t, radio_name, &easymesh_database);
						controller_ch_pref_updated = 1;
					} else if (controller_ch_pref_updated) {
						int band = RADIO_BAND_UNKNOWN;
						for (k = 0; k < t->op_class_nr; k++) {
							band = get_band_from_op_class(t->channel_preferences[k].op_class);
							if (band != RADIO_BAND_UNKNOWN)
								break;
						}
						if (band == RADIO_BAND_UNKNOWN) {
							struct Device *curr_device = locate_device(&map_network, element->src_mac, NEED_CREATE_FALSE, NULL);
							if (NULL == curr_device) {
								log_warn("DE - MAP_CHANNEL_PREFERENCE_REPORT_ELEMENT : Device not found! Skip data element update...\n");
							} else {
								struct Radio *curr_radio = locate_radio(curr_device, t->radio_unique_id, 0, NULL, NEED_CREATE_FALSE, NULL);
								if (NULL == curr_radio) {
									log_warn("DE - TLV_TYPE_CHANNEL_PREFERENCE : Radio not found! Skip data element update...\n");
								} else {
									band = curr_radio->band;
								}
							}
						}
						//if it is from a agent
						for (k = 0; k < controller_ch_pref_number; k++) {
							if (band == get_band_from_op_class(controller_ch_pref[k].ch_pre_op_class[0].op_class)) {
								memcpy(controller_ch_pref[k].radio_mac, t->radio_unique_id, 6);
								break;
							}
						}
					}

					//---------CHANNEL SELECTION--------------
					break;
				}
				case TLV_TYPE_RADIO_OPERATION_RESTRICTION: {
					log_trace("<-- TLV_TYPE_RADIO_OPERATION_RESTRICTION\n");
					break;
				}
				case TLV_TYPE_CAC_COMPLETION_REPORT: {
					log_trace("<-- TLV_TYPE_CAC_COMPLETION_REPORT\n");
					struct CACCompletionReportTLV *t = (struct CACCompletionReportTLV *)pt;
					struct cac_completion_report *cac_report;
					int l;
					cac_report = (struct cac_completion_report*)malloc(sizeof(struct cac_completion_report));
					log_trace("radio_nr: %d\n", t->radio_nr);
					cac_report->radio_nr = t->radio_nr;
					cac_report->radios = (struct cac_report_radio*)malloc(t->radio_nr * sizeof(struct cac_report_radio));
					for (k = 0; k < t->radio_nr; k++) {
						log_trace("radio_unique_identifier: %02x:%02x:%02x:%02x:%02x:%02x\n",
											t->radios[k].radio_unique_identifier[0],
											t->radios[k].radio_unique_identifier[1],
											t->radios[k].radio_unique_identifier[2],
											t->radios[k].radio_unique_identifier[3],
											t->radios[k].radio_unique_identifier[4],
											t->radios[k].radio_unique_identifier[5]);
						memcpy(cac_report->radios[k].radio_id, t->radios[k].radio_unique_identifier, 6);
						log_trace("op_class: %d\n", t->radios[k].op_class);
						cac_report->radios[k].op_class = t->radios[k].op_class;
						log_trace("channel: %d\n", t->radios[k].channel);
						cac_report->radios[k].channel = t->radios[k].channel;
						log_trace("flags: %d\n", t->radios[k].flags);
						cac_report->radios[k].flags = t->radios[k].flags;
						log_trace("pairs_nr: %d\n", t->radios[k].pairs_nr);
						cac_report->radios[k].pairs_nr = t->radios[k].pairs_nr;
						cac_report->radios[k].pairs = (struct cac_report_pair*)malloc(t->radios[k].pairs_nr * sizeof(struct cac_report_pair));
						for (l = 0; l < t->radios[k].pairs_nr; l++) {
							log_trace("pairs_op_class: %d\n", t->radios[k].pairs[l].pairs_op_class);
							cac_report->radios[k].pairs[l].op_class = t->radios[k].pairs[l].pairs_op_class;
							log_trace("pairs_channel: %d\n", t->radios[k].pairs[l].pairs_channel);
							cac_report->radios[k].pairs[l].channel = t->radios[k].pairs[l].pairs_channel;
						}
					}
					write_cac_report_file(cac_report);

					for (k = 0; k < cac_report->radio_nr; k++) {
						if (cac_report->radios[k].pairs) {
							free(cac_report->radios[k].pairs);
						}
					}
					free(cac_report->radios);
					free(cac_report);

					break;
				}
				default: {
					// log_warn("Unexpected TLV (%02x) type inside CMDU\n", *pt);
					break;
				}
				}
			}

			if (is_from_controller && controller_ch_pref_updated) {
				log_info("New controller channel preference received, will resend channel selection to all agents...\n");
				for (i = 0; i < map_network.number_of_devices; i++) {
					if (0 != memcmp(map_network.controller_id, map_network.device_list[i]->id, 6))
						map_network.device_list[i]->received_channel_report = 0;
				}
			}

			// log_info("-------------CHANNEL SELECTION-----------\n");
			// int j = 0;
			// for (i = 0; i < controller_ch_pref_number; i++) {
			// 	log_info("============ Controller Preference %d ====================\n", i);
			// 	log_info("Radio Mac : %02x %02x %02x %02x %02x %02x\n", controller_ch_pref[i].radio_mac[0], controller_ch_pref[i].radio_mac[1], controller_ch_pref[i].radio_mac[2], controller_ch_pref[i].radio_mac[3], controller_ch_pref[i].radio_mac[4], controller_ch_pref[i].radio_mac[5]);
			// 	log_info("Number of Op_class : %d\n", controller_ch_pref[i].op_class_nr);
			// 	for (j = 0; j < controller_ch_pref[i].op_class_nr; j++) {
			// 		log_info("Op_class : %d\n", controller_ch_pref[i].ch_pre_op_class[j].op_class);
			// 		log_info("Number of Channels : %d\n", controller_ch_pref[i].ch_pre_op_class[j].channel_nr);
			// 		for (k = 0; k < controller_ch_pref[i].ch_pre_op_class[j].channel_nr; k++) {
			// 			log_info("Channel%d : %d\n", k, controller_ch_pref[i].ch_pre_op_class[j].channel_list[k]);
			// 		}
			// 		log_info("Preference: %d\n", k, controller_ch_pref[i].ch_pre_op_class[j].preference_reason_code);
			// 	}
			// }
			// log_info("-------------CHANNEL SELECTION-----------\n");

			//---------CHANNEL SELECTION--------------
			//send a channel selection request to sync its channel with controller's preference
			if (controller_ch_pref_updated)
				send_channel_selection_request(element->src_mac, controller_ch_pref_number, controller_ch_pref, 0, NULL);
			//---------CHANNEL SELECTION--------------

			break;
		}
		case MAP_CHANNEL_SELECTION_RESPONSE_ELEMENT: {
			map_response_element *element = (map_response_element *)message->element;
			// log_trace("<-- MAP_CHANNEL_SELECTION_RESPONSE_MSG\n");
			// log_trace("tlv_nr %02x\n", element->tlv_nr);

			int i = 0;

			if (NULL == element->list_of_tlvs) {
				log_warn("Malformed structure.");
				break;
			}
			for (i = 0; i < element->tlv_nr; i++) {
				pt = element->list_of_tlvs[i];
				switch (*pt) {
				case TLV_TYPE_CHANNEL_SELECTION_RESPONSE: {
					// log_trace("<-- TLV_TYPE_CHANNEL_SELECTION_RESPONSE\n");
					//---------CHANNEL SELECTION--------------
					// struct ChannelSelectionResponseTLV *t = (struct ChannelSelectionResponseTLV *)pt;
					// log_info("-------------CHANNEL SELECTION-----------\n");
					// log_info("Src mac: %02x %02x %02x %02x %02x %02x\n", element->src_mac[0], element->src_mac[1], element->src_mac[2], element->src_mac[3], element->src_mac[4], element->src_mac[5]);
					// log_info("Channel Selection Response: %02x\n", t->response);
					// log_info("-------------CHANNEL SELECTION-----------\n");
					//---------CHANNEL SELECTION--------------
					break;
				}
				default: {
					// log_warn("Unexpected TLV (%02x) type inside CMDU\n", *pt);
					break;
				}
				}
			}
			break;
		}
		case MAP_OPERATING_CHANNEL_REPORT_ELEMENT: {
			map_response_element *element = (map_response_element *)message->element;
			// log_trace("<-- MAP_OPERATING_CHANNEL_REPORT_MSG\n");
			// log_trace("tlv_nr %02x\n", element->tlv_nr);

			uint8_t is_from_controller = !memcmp(map_network.controller_id, element->src_mac, 6);
			//--------------------------------DATA ELEMENT -------------------------------------
			struct Device *curr_device = locate_device(&map_network, element->src_mac, NEED_CREATE_FALSE, NULL);
			if (NULL == curr_device) {
				log_warn("DE - MAP_OPERATING_CHANNEL_REPORT_ELEMENT : Device not found! Skip data element update...\n");
			} else {
				curr_device->received_channel_report += 1;
			}

			//--------------------------------DATA ELEMENT -------------------------------------

			int i;

			if (NULL == element->list_of_tlvs) {
				log_warn("Malformed structure.");
				break;
			}

			for (i = 0; i < element->tlv_nr; i++) {
				pt = element->list_of_tlvs[i];
				switch (*pt) {
				case TLV_TYPE_OPERATING_CHANNEL_REPORT: {
					//log_trace("<-- TLV_TYPE_OPERATING_CHANNEL_REPORT\n");
					struct OperatingChannelReportTLV *t = (struct OperatingChannelReportTLV *)pt;
					//--------------------------------DATA ELEMENT -------------------------------------
					if (curr_device) {
						struct Radio *curr_radio = locate_radio(curr_device, t->radio_unique_id, 0, NULL, NEED_CREATE_FALSE, NULL);
						if (NULL == curr_radio) {
							log_warn("DE - TLV_TYPE_OPERATING_CHANNEL_REPORT : Radio not found! Skip data element update...\n");
						} else {
							if (!is_from_controller && need_sync_back_channel(curr_radio, t))
								curr_device->received_channel_report = 0;
							update_radio(curr_radio, t, NULL, NULL, NULL);
						}
					}
					//--------------------------------DATA ELEMENT -------------------------------------
					break;
				}
				default: {
					// log_warn("Unexpected TLV (%02x) type inside CMDU\n", *pt);
					break;
				}
				}
			}

			break;
		}
		case MAP_CHANNEL_SCAN_REPORT_ELEMENT: {
			map_response_element *element = (map_response_element *)message->element;
			log_trace("<-- MAP_CHANNEL_SCAN_REPORT_ELEMENT\n");
			log_trace("tlv_nr %d\n", element->tlv_nr);

			if (channel_scan_flag == 0) {
				log_warn("channel_scan_flag is 0.");
				break;
			}

			if (NULL == element->list_of_tlvs) {
				log_warn("Malformed structure.");
				break;
			}

			int i = 0, j = 0;
			struct channel_scan_report *report = (struct channel_scan_report*)malloc(sizeof(struct channel_scan_report));
			report->channel_nr = element->tlv_nr - 1;
			report->channels = (struct channel_list*)malloc(sizeof(struct channel_list) * report->channel_nr);

			for (i = 0; i < element->tlv_nr; i++) {
				pt = element->list_of_tlvs[i];
				switch (*pt) {
				case TLV_TYPE_CHANNEL_SCAN_RESULT: {
					log_trace("<-- TLV_TYPE_CHANNEL_SCAN_RESULT\n");
					struct ChannelScanResultTLV *t = (struct ChannelScanResultTLV *)pt;
					//--------------------------------Store channel scan results-------------------------------------
					report->channels[j].op_class = t->op_class;
					report->channels[j].channel = t->channel;
					report->channels[j].scan_status = t->scan_status;
					report->channels[j].utilization = t->channel_utilization;
					report->channels[j].noise = t->noise;
					report->channels[j].neighbor_nr = t->neighbor_nr;
					j++;
					break;
				}
				default: {
					// log_warn("Unexpected TLV (%02x) type inside CMDU\n", *pt);
					break;
				}
				}
			}

			if(report->channel_nr > 0) {
				channel_scan_report_recv++;
				if(channel_scan_report_recv == 1) {
					aggre_report.channel_nr = report->channel_nr;
					if (aggre_report.channel_nr) {
						aggre_report.channels = (struct channel_list *)malloc(sizeof(struct channel_list) * aggre_report.channel_nr);
						memcpy(aggre_report.channels, report->channels, sizeof(struct channel_list) * aggre_report.channel_nr);
					}
				} else if (channel_scan_report_recv > 1 && channel_scan_report_recv <= map_network.number_of_devices) {
					for(i = 0; i < report->channel_nr; i++) {
						uint8_t channel_found = 0;
						for(j = 0; j < aggre_report.channel_nr; j++) {
							if(report->channels[i].op_class == aggre_report.channels[j].op_class && report->channels[i].channel == aggre_report.channels[j].channel) {
								aggre_report.channels[j].utilization = (aggre_report.channels[j].utilization * (channel_scan_report_recv - 1) + report->channels[i].utilization)/channel_scan_report_recv;
								aggre_report.channels[j].noise = (aggre_report.channels[j].noise * (channel_scan_report_recv - 1) + report->channels[i].noise)/channel_scan_report_recv;
								aggre_report.channels[j].neighbor_nr = (aggre_report.channels[j].neighbor_nr * (channel_scan_report_recv - 1) + report->channels[i].neighbor_nr)/channel_scan_report_recv;
								channel_found = 1;
								break;
							}
						}
						if(channel_found == 0) {
							aggre_report.channel_nr += 1;
							aggre_report.channels = (struct channel_list*)realloc(aggre_report.channels, sizeof(struct channel_list) * aggre_report.channel_nr);
							memcpy(&aggre_report.channels[aggre_report.channel_nr - 1], &report->channels[i], sizeof(struct channel_list));
						}
					}
				}
			}

			if(channel_scan_report_recv == map_network.number_of_devices) {
				log_debug("All channel scan reports received! write_channel_scan_report_file\n");
				write_channel_scan_report_file(&aggre_report);
				channel_scan_flag = 0;
				if (aggre_report.channels) {
					free(aggre_report.channels);
					aggre_report.channels = NULL;
				}
			}

			if (report->channels) {
				free(report->channels);
			}
			free(report);

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
			map_response_element *element      = (map_response_element *)message->element;
			uint8_t               empty_bss[6] = { 0 };
			log_trace("<-- MAP_TOPOLOGY_NOTIFICATION_ELEMENT\n");
			log_trace("tlv_nr %02x\n", element->tlv_nr);
			//--------------------------------DATA ELEMENT -------------------------------------
			struct Device *curr_device = locate_device(&map_network, element->src_mac, NEED_CREATE_FALSE, NULL);
			if (NULL == curr_device) {
				log_warn("DE - MAP_TOPOLOGY_NOTIFICATION_ELEMENT : Device not found! Skip data element update...\n");
			}
			//--------------------------------DATA ELEMENT -------------------------------------
			int     i         = 0;
			uint8_t al_mac[6] = { 0 };

			if (NULL == element->list_of_tlvs) {
				log_warn("Malformed structure.");
				break;
			}

			for (i = 0; i < element->tlv_nr; i++) {
				pt = element->list_of_tlvs[i];
				switch (*pt) {
				case TLV_TYPE_AL_MAC_ADDRESS_TYPE: {
					log_trace("<-- TLV_TYPE_AL_MAC_ADDRESS_TYPE\n");
					struct alMacAddressTypeTLV *t = (struct alMacAddressTypeTLV *)pt;
					memcpy(al_mac, t->al_mac_address, 6);
					//send_topology_query_request(al_mac);
					break;
				}
				case TLV_TYPE_CLIENT_ASSOCIATION_EVENT: {
					log_trace("<-- TLV_TYPE_CLIENT_ASSOCIATION_EVENT\n");
					//					struct sta_info_data              sta_info;
					uint8_t                           previous_bss[6]    = { 0 };
					uint8_t                           passive_roam_count = 0;
					struct ClientAssociationEventTLV *t                  = (struct ClientAssociationEventTLV *)pt;

					//--------------------------------DATA ELEMENT -------------------------------------
					if (t->event == CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_JOIN) {
						struct StructBSS *curr_bss;
						struct STA *      curr_sta = locate_sta_by_mac(&map_network, NULL, t->mac_address, NULL, NULL, &curr_bss);
						if (curr_sta) {
							passive_roam_count = curr_sta->passive_roam_count;
							memcpy(previous_bss, curr_bss->bssid, 6);
							free_sta(curr_bss, curr_sta->mac_address);
						}
					}
					if (curr_device) {
						//Locate radio and bss given bssid
						struct Radio *curr_radio = locate_radio(curr_device, NULL, 0, t->bssid, NEED_CREATE_FALSE, NULL);
						if (NULL == curr_radio) {
							log_warn("DE - TLV_TYPE_CLIENT_ASSOCIATION_EVENT : Radio not found! Skip data element update...\n");
						} else {
							struct StructBSS *curr_bss = locate_bss(curr_radio, t->bssid, NEED_CREATE_FALSE, NULL);
							if (NULL == curr_bss) {
								log_warn("DE - TLV_TYPE_CLIENT_ASSOCIATION_EVENT : BSS not found! Skip data element update...\n");
							} else {
								//if it is a JOIN event, add sta into database; else remove it from database
								if (t->event == CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_JOIN) {
									struct STA *curr_sta = locate_sta(curr_bss, t->mac_address, NEED_CREATE_TRUE, NULL);
									if (NULL == curr_sta) {
										log_warn("DE - TLV_TYPE_CLIENT_ASSOCIATION_EVENT : Error when locating STA...\n");
									} else {
										if ((memcmp(empty_bss, previous_bss, 6) != 0) && (memcmp(previous_bss, curr_bss->bssid, 6) != 0)) {
											if (passive_roam_count != 0)
												curr_sta->passive_roam_count = passive_roam_count;
											log_info("\tSta %02x%02x%02x%02x%02x%02x roamed from %02x%02x%02x%02x%02x%02x to %02x%02x%02x%02x%02x%02x\n",
											         curr_sta->mac_address[0], curr_sta->mac_address[1], curr_sta->mac_address[2],
											         curr_sta->mac_address[3], curr_sta->mac_address[4], curr_sta->mac_address[5],
											         previous_bss[0], previous_bss[1], previous_bss[2],
											         previous_bss[3], previous_bss[4], previous_bss[5],
											         curr_bss->bssid[0], curr_bss->bssid[1], curr_bss->bssid[2],
											         curr_bss->bssid[3], curr_bss->bssid[4], curr_bss->bssid[5]);
										}
									}
								} else if (t->event == CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_LEAVE) {
									free_sta(curr_bss, t->mac_address);
								}
							}
						}
					}
					//--------------------------------DATA ELEMENT -------------------------------------

					break;
				}
				default: {
					// log_warn("Unexpected TLV (%02x) type inside CMDU\n", *pt);
					break;
				}
				}
			}
			optimize_datamodel(&map_network);
			break;
		}
		case MAP_PUSH_BUTTON_NOTIFICATION_ELEMENT: {
			log_trace("<-- MAP_PUSH_BUTTON_NOTIFICATION_ELEMENT\n");

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
					if (!easymesh_database.eth_only)
						process_rtk_vendor_spec_tlv((struct vendorSpecificTLV *)pt, easymesh_database.radio_data_nr, easymesh_database.radio_data, easymesh_database.vap_prefix);
					break;
				}
				default: {
					log_trace("<-- Unknown TLV\n");
				}
				}
			}
			break;
		}
		case MAP_WSC_M1_ELEMENT: {
			map_response_element *element = (map_response_element *)message->element;
			log_trace("<-- MAP_WSC_M1_ELEMENT from %02x:%02x:%02x:%02x:%02x:%02x\n", element->src_mac[0], element->src_mac[1], element->src_mac[2], element->src_mac[3], element->src_mac[4], element->src_mac[5]);
			struct Device *curr_device = locate_device(&map_network, element->src_mac, NEED_CREATE_FALSE, NULL);
			if (NULL == curr_device) {
				log_warn("DE - MAP_WSC_M1_ELEMENT : Error when locating device!\n");
				break;
			}
			int            i            = 0;
			for (i = 0; i < element->tlv_nr; i++) {
				pt = element->list_of_tlvs[i];
				switch (*pt) {
				case TLV_TYPE_WSC: {
					struct wscTLV *wsc_tlv = (struct wscTLV *)pt;
					uint8_t *frame     = NULL;
					uint16_t frame_len = 0;
					if (NULL == curr_device->manufacturer) {
						frame = get_wsc_attribute(wsc_tlv->wsc_frame, wsc_tlv->wsc_frame_size, ATTR_MANUFACTURER);
						if (frame) {
							frame_len = (*frame << 8) | *(frame + 1);
							frame += 2;
							curr_device->manufacturer = malloc(sizeof(uint8_t) * frame_len + 1);
							memset(curr_device->manufacturer, 0, frame_len + 1);
							memcpy(curr_device->manufacturer, frame, frame_len);
						}
					}
					if (NULL == curr_device->model_name) {
						frame = get_wsc_attribute(wsc_tlv->wsc_frame, wsc_tlv->wsc_frame_size, ATTR_MODEL_NAME);
						if (frame) {
							frame_len = (*frame << 8) | *(frame + 1);
							frame += 2;
							curr_device->model_name = malloc(sizeof(uint8_t) * frame_len + 1);
							memset(curr_device->model_name, 0, frame_len + 1);
							memcpy(curr_device->model_name, frame, frame_len);
						}
					}
					break;
				}
				}
			}
			curr_device->renew_ack = 1;
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
					log_trace("<-- Unknown TLV\n");
				}
				}
			}
			break;
		}
		case MAP_OPERATING_CHANNEL_CHANGE_NOTIFICATION_ELEMENT: {
			log_trace("<-- MAP_OPERATING_CHANNEL_CHANGE_NOTIFICATION_ELEMENT\n");
			controller_ch_pref_updated = 0;
			send_channel_preference_query(map_network.controller_id);
			break;
		}
		default: {
		}
		}
		map_free_element(message->element);
		free(message);
		message = NULL;
	}
	free(buffer);
	return 0;
}
