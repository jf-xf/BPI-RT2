/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "map_dataelement.h"
#include "easymesh_utils.h"

#include "map_logger.h"

static struct Network *global_network;

uint16_t max_sta_num_per_bss = 64;

struct Network *get_network()
{
	return global_network;
}

uint8_t rssi_to_rcpi(uint8_t rssi)
{
	//convert per 100 to per 220
	return (2 * (10 + rssi));
}

uint8_t rcpi_to_rssi(uint8_t rcpi)
{
	if (rcpi < 20) {
		return 0;
	}
	//convert per 220 to per 110
	return (rcpi / 2 - 10);
}

void reset_renew_ack(struct Network *network)
{
	uint8_t i;
	for (i = 0; i < network->number_of_devices; i++) {
		if (!memcmp(network->device_list[i]->id, network->controller_id, 6)) {
			network->device_list[i]->renew_ack = 1;
		} else {
			network->device_list[i]->renew_ack = 0;
		}
	}
}

void reset_visited(struct Network *network)
{
	uint8_t i;
	for (i = 0; i < network->number_of_devices; i++) {
		network->device_list[i]->is_visited = 0;
	}
}

uint8_t _is_neighbor_exist(struct Device *device, uint8_t *neighbor_mac, uint8_t *index)
{
	int i = 0;
	for (i = 0; i < device->number_of_direct_neighbors; i++) {
		if (0 == memcmp(device->direct_neighbor_list[i].al_mac, neighbor_mac, 6)) {
			if (index)
				*index = i;
			return 1;
		}
	}
	return 0;
}

uint8_t remove_neighbor(struct Device *device, uint8_t *neighbor_mac)
{
	int     i      = 0;
	uint8_t result = 0;
	for (i = 0; i < device->number_of_direct_neighbors; i++) {
		if (0 == memcmp(device->direct_neighbor_list[i].al_mac, neighbor_mac, 6)) {
			result = 1;
			break;
		}
	}
	if (result) {
		device->direct_neighbor_list[i] = device->direct_neighbor_list[device->number_of_direct_neighbors - 1];
		device->number_of_direct_neighbors--;
		device->direct_neighbor_list = (struct DeviceNeighbor *)realloc(device->direct_neighbor_list, device->number_of_direct_neighbors * sizeof(struct DeviceNeighbor));
		if (NULL != device->direct_neighbor_list) {
			result = 0;
		}
	}
	return result;
}

uint8_t insert_neighbor(struct Device *device, uint8_t *neighbor_mac)
{
	if (_is_neighbor_exist(device, neighbor_mac, NULL)) {
		return 1;
	}

	// log_info("Insert %02x:%02x:%02x:%02x:%02x:%02x to %02x:%02x:%02x:%02x:%02x:%02x\n", neighbor_mac[0], neighbor_mac[1], neighbor_mac[2], neighbor_mac[3], neighbor_mac[4], neighbor_mac[5],
	//        device->id[0], device->id[1], device->id[2], device->id[3], device->id[4], device->id[5]);

	device->number_of_direct_neighbors++;
	device->direct_neighbor_list = (struct DeviceNeighbor *)realloc(device->direct_neighbor_list, device->number_of_direct_neighbors * sizeof(struct DeviceNeighbor));
	if (NULL == device->direct_neighbor_list) {
		device->number_of_direct_neighbors--;
		return 1;
	}
	memcpy(device->direct_neighbor_list[device->number_of_direct_neighbors - 1].al_mac, neighbor_mac, 6);
	// device->direct_neighbor_list[device->number_of_direct_neighbors - 1].is_expired = 0;
	return 0;
}

void optimize_neighbor(struct Network *network)
{
	int i = 0, l = 0;
	for (l = 0; l < network->number_of_devices; l++) {
		struct Device *device = network->device_list[l];
		for (i = 0; i < device->number_of_direct_neighbors; i++) {
			struct Device *neighbor_device = locate_device(network, device->direct_neighbor_list[i].al_mac, NEED_CREATE_FALSE, NULL);

			if (NULL == neighbor_device) {
				continue;
			}

			insert_neighbor(neighbor_device, device->id);
		}
	}
	return;
}

void optimize_datamodel(struct Network *network)
{
	int i = 0, j = 0, k = 0, l = 0;
	for (l = 0; l < network->number_of_devices; l++) {
		struct Device *device = network->device_list[l];
		for (i = 0; i < device->number_of_radios; i++) {
			struct Radio *radio = device->radio_list[i];
			for (j = 0; j < radio->number_of_bss; j++) {
				struct StructBSS *bss = radio->bss_list[j];
				for (k = 0; k < bss->number_of_sta; k++) {
					struct STA *   sta             = bss->sta_list[k];
					struct Device *neighbor_device = locate_device(network, sta->mac_address, NEED_CREATE_FALSE, NULL);
					if (NULL == neighbor_device) {
						neighbor_device = locate_device_by_bss_mac(network, sta->mac_address);
					}

					if (NULL == neighbor_device) {
						continue;
					}

					sta->is_backhaul_sta = 1;

					if (!_is_neighbor_exist(device, neighbor_device->id, NULL)) {
						int m = 0;
						for (m = 0; m < network->number_of_devices; m++) {
							struct Device *device_duplicate = network->device_list[m];
							if (remove_neighbor(device_duplicate, neighbor_device->id)) {
								struct Device *device_reverse = locate_device(network, neighbor_device->id, NEED_CREATE_FALSE, NULL);
								if (device_reverse != NULL) {
									remove_neighbor(device_reverse, device_duplicate->id);
								}
							}
						}

						log_warn("STA %02x:%02x:%02x:%02x:%02x:%02x belongs to the Easy Mesh device with al %02x:%02x:%02x:%02x:%02x:%02x does not exist in neighbor list of device %02x:%02x:%02x:%02x:%02x:%02x, inserting..\n", sta->mac_address[0], sta->mac_address[1], sta->mac_address[2], sta->mac_address[3], sta->mac_address[4], sta->mac_address[5], neighbor_device->id[0], neighbor_device->id[1], neighbor_device->id[2], neighbor_device->id[3], neighbor_device->id[4], neighbor_device->id[5], device->id[0], device->id[1], device->id[2], device->id[3], device->id[4], device->id[5]);
						insert_neighbor(device, neighbor_device->id);
						insert_neighbor(neighbor_device, device->id);
					}
				}
			}
		}
	}
	return;
}

struct Device *get_controller_device(struct Network *network)
{
	uint8_t i;
	for (i = 0; i < network->number_of_devices; i++) {
		if (!memcmp(network->device_list[i]->id, network->controller_id, 6)) {
			return network->device_list[i];
		}
	}
	return NULL;
}

//A function to retrieve the RADIO struct from ApOperationalBSSTLV
struct RADIO *radioFromList(unsigned char *radio_mac, struct ApOperationalBSSTLV *tlv)
{
	unsigned char i;
	for (i = 0; i < tlv->radios_nr; i++) {
		if (memcmp(tlv->radios[i].radio_unique_id, radio_mac, 6) == 0)
			return &tlv->radios[i];
	}
	return NULL;
}

uint8_t update_network(struct Network *network, uint8_t *controller_mac)
{
	uint8_t al_mac_empty[6] = { 0 };
	network->id = "RTK_EasyMesh_Network";
	memcpy(network->controller_id, controller_mac, 6);
	network->number_of_devices  = 0;
	network->max_device_reached = 0;
	global_network = network;

	if (memcmp(al_mac_empty, controller_mac, 6)) {
		locate_device(network, controller_mac, NEED_CREATE_TRUE, NULL);
	}
	return 0;
}

struct Device *locate_device(struct Network *network, uint8_t *src_mac, uint8_t need_create_flag, uint16_t *index)
{
	unsigned char i;
	for (i = 0; i < network->number_of_devices; i++) {
		if (!memcmp(network->device_list[i]->id, src_mac, 6)) {
			if (index)
				*index = i;
			return network->device_list[i];
		}
	}

	//if not found and need_create flag is set, create a new device
	if (need_create_flag) {
		if (network->number_of_devices == MAX_DEVICE_NUM_PER_NETWORK) {
			log_error("Error : Failed to add new device. Max device number reached!\n");
			return NULL; //Error when a need_create flag is set but a null is returned
		}
		network->device_list[network->number_of_devices] = (struct Device *)malloc(sizeof(struct Device));
		memcpy(network->device_list[network->number_of_devices]->id, src_mac, 6);
		memset(network->device_list[network->number_of_devices]->parent_id, 0, 6);
		network->device_list[network->number_of_devices]->keep_alive = MAX_KEEP_ALIVE;
		network->device_list[network->number_of_devices]->number_of_radios        = 0;
		network->device_list[network->number_of_devices]->multi_ap_capabilities   = 0;
		network->device_list[network->number_of_devices]->policy_config           = 0;
		network->device_list[network->number_of_devices]->received_channel_report = 0;
		time(&network->device_list[network->number_of_devices]->updated_timestamp);
		network->device_list[network->number_of_devices]->number_of_direct_neighbors = 0;
		network->device_list[network->number_of_devices]->direct_neighbor_list       = NULL;
		network->device_list[network->number_of_devices]->device_name                = strdup("EZMSDeivce");
		network->device_list[network->number_of_devices]->ip_addr                    = strdup("UNKNOWN");
		network->device_list[network->number_of_devices]->path_score                 = 0;
		network->device_list[network->number_of_devices]->renew_ack                  = 0;
		network->device_list[network->number_of_devices]->is_visited                 = 0;
		network->device_list[network->number_of_devices]->logical_distance_to_controller		 = 0xFF;
		network->device_list[network->number_of_devices]->physical_distance_to_controller		 = 0xFF;
		network->device_list[network->number_of_devices]->need_vlan_configure		 = VLAN_CONFIGURE_NOT_READY;
		network->device_list[network->number_of_devices]->multi_ap_profile			 = 1;
		network->device_list[network->number_of_devices]->non_1905_client_nr         = 0;
		network->device_list[network->number_of_devices]->non_1905_clients           = NULL;
		network->device_list[network->number_of_devices]->renew_send_count           = 0;
		if (0 == memcmp(network->controller_id, src_mac, 6)) {
			network->device_list[network->number_of_devices]->renew_ack = 1;
		}
		network->device_list[network->number_of_devices]->model_name             = NULL;
		network->device_list[network->number_of_devices]->manufacturer           = NULL;
		network->device_list[network->number_of_devices]->local_disallowed_sta_nr    = 0;
		network->device_list[network->number_of_devices]->btm_disallowed_sta_nr      = 0;

		if (index)
			*index = network->number_of_devices;
		network->number_of_devices++;
		return network->device_list[network->number_of_devices - 1];
	}

	return NULL;
}

struct Device *locate_device_by_al_mac(struct Network *network, uint8_t *al_mac)
{
	if (network)
		return locate_device(network, al_mac, NEED_CREATE_FALSE, NULL);
	return locate_device(global_network, al_mac, NEED_CREATE_FALSE, NULL);
}

uint16_t find_band_by_backhaul_sta_mac(struct Network *network, uint8_t *sta_mac)
{
	int i, j, k, l;
	if (network) {
		for (l = 0; l < network->number_of_devices; l++) {
			for (i = 0; i < network->device_list[l]->number_of_radios; i++) {
				for (j = 0; j < network->device_list[l]->radio_list[i]->number_of_bss; j++) {
					for (k = 0; k < network->device_list[l]->radio_list[i]->bss_list[j]->number_of_sta; k++) {
						if (!memcmp(network->device_list[l]->radio_list[i]->bss_list[j]->sta_list[k]->mac_address, sta_mac, 6)) {
							return network->device_list[l]->radio_list[i]->band;
						}
					}
				}
			}
		}
	}
	return RADIO_BAND_UNKNOWN;
}

void update_device_info(struct Device *device, struct deviceInformationTypeTLV *info, uint8_t radio_number, struct Network *network)
{
	int i = 0;

	struct Radio *radio_5gh           = NULL;
	struct Radio *radio_5gl           = NULL;
	struct Radio *radio_2g            = NULL;
	uint8_t *   backhaul_sta_mac_5gh  = NULL;
	uint8_t *   backhaul_sta_mac_5gl  = NULL;
	uint8_t *   backhaul_sta_mac_2g   = NULL;
	uint16_t    backhaul_sta_band     = RADIO_BAND_UNKNOWN;

	for (i = 0; i < info->local_interfaces_nr; i++) {
		if (IS_IEEE_802_11_5_GHZ_MEDIA(info->local_interfaces[i].media_type)) {
			if (IEEE80211_SPECIFIC_INFO_ROLE_AP == info->local_interfaces[i].media_specific_data.ieee80211.role) {
				struct Radio *curr_radio = locate_radio(device, NULL, 0, info->local_interfaces[i].mac_address, NEED_CREATE_FALSE, NULL);
				if (curr_radio) {
					if (radio_number > 2 && info->local_interfaces[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_1 >= 100) {
						// We assume center frequency channel over 100(inclusive) to be 5gh, even if 2 full band exist.
						// for ax5400, manually setting wlan1 to below 100 and wlan0 over 100 are deemed as forbidden.
						radio_5gh = curr_radio;
					} else {
						radio_5gl = curr_radio;
					}
				}
			} else if (IEEE80211_SPECIFIC_INFO_ROLE_NON_AP_NON_PCP_STA == info->local_interfaces[i].media_specific_data.ieee80211.role) {
				if (radio_number > 2 && info->local_interfaces[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_1 >= 100) {
					backhaul_sta_mac_5gh = info->local_interfaces[i].mac_address;
				} else {
					backhaul_sta_mac_5gl = info->local_interfaces[i].mac_address;
				}
			}
		} else if (IS_IEEE_802_11_2_4_GHZ_MEDIA(info->local_interfaces[i].media_type)) {
			if (IEEE80211_SPECIFIC_INFO_ROLE_AP == info->local_interfaces[i].media_specific_data.ieee80211.role) {
				struct Radio *curr_radio = locate_radio(device, NULL, 0, info->local_interfaces[i].mac_address, NEED_CREATE_FALSE, NULL);
				if (curr_radio) {
					radio_2g = curr_radio;
				}
			} else if (IEEE80211_SPECIFIC_INFO_ROLE_NON_AP_NON_PCP_STA == info->local_interfaces[i].media_specific_data.ieee80211.role) {
				backhaul_sta_mac_2g = info->local_interfaces[i].mac_address;
			}
		} else if (IS_IEEE_802_11AX_MEDIA(info->local_interfaces[i].media_type)) {
			struct Radio *curr_radio = locate_radio(device, NULL, 0, info->local_interfaces[i].mac_address, NEED_CREATE_FALSE, NULL);
			if (curr_radio) {
				if (curr_radio->band == RADIO_BAND_5GH) {
					radio_5gh = curr_radio;
				} else if (curr_radio->band == RADIO_BAND_5GL) {
					radio_5gl = curr_radio;
				} else if (curr_radio->band == RADIO_BAND_2G) {
					radio_2g = curr_radio;
				}
			} else {
				backhaul_sta_band = find_band_by_backhaul_sta_mac(network, info->local_interfaces[i].mac_address);
				if (backhaul_sta_band == RADIO_BAND_5GH) {
					backhaul_sta_mac_5gh = info->local_interfaces[i].mac_address;
				} else if (backhaul_sta_band == RADIO_BAND_5GL) {
					backhaul_sta_mac_5gl = info->local_interfaces[i].mac_address;
				} else if (backhaul_sta_band == RADIO_BAND_2G) {
					backhaul_sta_mac_2g = info->local_interfaces[i].mac_address;
				}
			}
		}
	}

	if (radio_5gh && backhaul_sta_mac_5gh) {
		memcpy(radio_5gh->backhaul_sta.mac_address, backhaul_sta_mac_5gh, 6);
	}

	if (radio_5gl && backhaul_sta_mac_5gl) {
		memcpy(radio_5gl->backhaul_sta.mac_address, backhaul_sta_mac_5gl, 6);
	}

	if (radio_2g && backhaul_sta_mac_2g) {
		memcpy(radio_2g->backhaul_sta.mac_address, backhaul_sta_mac_2g, 6);
	}
}

uint8_t update_device_neighbor(struct Network network, struct Device *device, struct neighborDeviceListTLV *neighbor_device_list_tlv, uint32_t custom_features)
{
	// struct Device *device = locate_device(&network, device_id, NEED_CREATE_FALSE, NULL);
	if (NULL == device)
		return 1;

	uint8_t result = 0;
	uint8_t i;
	//overwrite neighbor list every time when receive topology response
	if (neighbor_device_list_tlv) {
		device->number_of_direct_neighbors = 0;
		if (device->direct_neighbor_list) {
			free(device->direct_neighbor_list);
			device->direct_neighbor_list = NULL;
		}
		for (i = 0; i < neighbor_device_list_tlv->neighbors_nr; i++) {
			if (custom_features & SUPPORT_WIRED_LINER_TOPOLOGY) {
                          	if(0 == neighbor_device_list_tlv->neighbors[i].bridge_flag) {
					insert_neighbor(device, neighbor_device_list_tlv->neighbors[i].mac_address);
					struct Device *neighbor_device = locate_device(&network, neighbor_device_list_tlv->neighbors[i].mac_address, NEED_CREATE_FALSE, NULL);
					if (neighbor_device != NULL) {
						if (insert_neighbor(neighbor_device, device->id)) {
							result = 1;
						}
					}
                                }
			} else {
				insert_neighbor(device, neighbor_device_list_tlv->neighbors[i].mac_address);
				struct Device *neighbor_device = locate_device(&network, neighbor_device_list_tlv->neighbors[i].mac_address, NEED_CREATE_FALSE, NULL);
				if (neighbor_device != NULL) {
					if (insert_neighbor(neighbor_device, device->id)) {
						result = 1;
					}
				}
			}
		}
	}
	return result;
}

uint8_t update_device_operational_bss(struct Device *device, struct ApOperationalBSSTLV *operational_bss)
{

	if (NULL == device)
		return 1;

	int x, y;
	//overwrite operational bss list every time when receive topology response
	for (x = 0; x < operational_bss->radios_nr; x++) {
		struct Radio *curr_radio = locate_radio(device, operational_bss->radios[x].radio_unique_id, 0, NULL, NEED_CREATE_TRUE, NULL);
		if (NULL == curr_radio) {
			continue;
		}
		for (y = 0; y < operational_bss->radios[x].BSSs_nr; y++) {
			struct StructBSS *curr_bss = locate_bss(curr_radio, operational_bss->radios[x].BSSs[y].mac_addr, NEED_CREATE_TRUE, NULL);
			if (NULL == curr_bss) {
				continue;
			}
			update_bss(curr_bss, operational_bss->radios[x].BSSs[y].ssid, operational_bss->radios[x].BSSs[y].ssid_len, NULL);
		}
	}

	return 1;
}

uint8_t update_device_associated_clients(struct Network network, struct Device *device, struct AssociatedClientsTLV *associated_clients)
{

	if (NULL == device)
		return 1;

	int x, y;
	for (x = 0; x < associated_clients->BSSs_nr; x++) {
		struct Radio *curr_radio = locate_radio(device, NULL, 0, associated_clients->BSSs[x].bssid, NEED_CREATE_FALSE, NULL);
		if (NULL == curr_radio) {
			continue;
		}
		struct StructBSS *curr_bss = locate_bss(curr_radio, associated_clients->BSSs[x].bssid, NEED_CREATE_FALSE, NULL);
		if (NULL == curr_bss) {
			continue;
		}
		for (y = 0; y < associated_clients->BSSs[x].clients_nr; y++) {
			uint8_t           passive_roam_count = 0;
			struct StructBSS *prev_bss           = NULL;
			struct STA *      old_sta            = locate_sta_by_mac(&network, NULL, associated_clients->BSSs[x].clients[y].mac_addr, NULL, NULL, &prev_bss);
			if (old_sta && prev_bss) {
				if (memcmp(prev_bss->bssid, curr_bss->bssid, 6) != 0) {
					passive_roam_count = old_sta->passive_roam_count;
					log_warn("\tSta %02x%02x%02x%02x%02x%02x roamed from %02x%02x%02x%02x%02x%02x to %02x%02x%02x%02x%02x%02x\n",
					         associated_clients->BSSs[x].clients[y].mac_addr[0],
					         associated_clients->BSSs[x].clients[y].mac_addr[1],
					         associated_clients->BSSs[x].clients[y].mac_addr[2],
					         associated_clients->BSSs[x].clients[y].mac_addr[3],
					         associated_clients->BSSs[x].clients[y].mac_addr[4],
					         associated_clients->BSSs[x].clients[y].mac_addr[5],
					         prev_bss->bssid[0], prev_bss->bssid[1], prev_bss->bssid[2], prev_bss->bssid[3], prev_bss->bssid[4], prev_bss->bssid[5],
					         curr_bss->bssid[0], curr_bss->bssid[1], curr_bss->bssid[2], curr_bss->bssid[3], curr_bss->bssid[4], curr_bss->bssid[5]);
					free_sta(prev_bss, associated_clients->BSSs[x].clients[y].mac_addr);
				}
			}
			struct STA *curr_sta = locate_sta(curr_bss, associated_clients->BSSs[x].clients[y].mac_addr, NEED_CREATE_TRUE, NULL);
			if (NULL != curr_sta) {
				update_sta(curr_sta, NULL, NULL, NULL);
				if (passive_roam_count != 0)
					curr_sta->passive_roam_count = passive_roam_count;
				curr_sta->still_present = 1;
			}
		}
		//free those sta which are not in topology response any more
		int tmp_number_of_sta = curr_bss->number_of_sta;
		for (y = tmp_number_of_sta - 1; y >= 0; y--) {
			if (0 == curr_bss->sta_list[y]->still_present) {
				free_sta(curr_bss, curr_bss->sta_list[y]->mac_address);
			} else {
				//reset to 0
				curr_bss->sta_list[y]->still_present = 0;
			}
		}
	}

	return 1;
}

uint8_t update_device(struct Device *device, struct APCapabilityTLV *ap_capability_tlv, struct vendorSpecificTLV *vendor_specific_tlv, struct MultiAPProfileTLV *multiap_profile_tlv)
{

	//update ap_capability
	if (ap_capability_tlv) {
		device->multi_ap_capabilities = ap_capability_tlv->ap_capability;
	}

	if (vendor_specific_tlv) {

		//Handle vendor specific data from Realtek devices only
		if (!memcmp(vendor_specific_tlv->vendor_oui, REALTEK_OUI, 3)) {
			if (device->ip_addr)
				free(device->ip_addr);
			device->ip_addr = strdup((char *)vendor_specific_tlv->m);
			if (device->device_name)
				free(device->device_name);
			device->device_name = strdup((char *)(vendor_specific_tlv->m + strlen((char *)vendor_specific_tlv->m) + 1));
		}
	}

	//update multi-ap_profile
	if (multiap_profile_tlv) {
		if (device->multi_ap_profile < multiap_profile_tlv->profile) {
			device->multi_ap_profile = multiap_profile_tlv->profile;
			device->policy_config = 0;
		}
	}

	return 0;
}

//This function is for retrieving radio in a given device
//There are 2 possible scenarios
//1. Locate by radio mac for all TLV expect TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE
//2. Locate by operating channel for TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE where radio mac is not available
struct Radio *locate_radio(struct Device *device, uint8_t *radio_mac, uint8_t cur_op_class, uint8_t *bssid, uint8_t need_create_flag, uint16_t *index)
{

	uint8_t dummy_mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char i, k;
	if (device == NULL) {
		log_error("Error : Failed to add new radio. Device list is null!\n");
		return NULL;
	}

	//locate by current operating channel
	if (0 != cur_op_class) {
		//traverse through radios
		for (i = 0; i < device->number_of_radios; i++) {
			if (device->radio_list[i]->current_operating_classes == NULL) {
				return NULL;
			}
			//number of current op class (channel) is hard coded to 1
			if (device->radio_list[i]->current_operating_classes[0].op_class == cur_op_class) {
				if (index)
					*index = i;
				return device->radio_list[i];
			}
		}
	} else if (radio_mac) { //locate by radio mac
		//traverse through radios
		for (i = 0; i < device->number_of_radios; i++) {
			if (!memcmp(device->radio_list[i]->id, radio_mac, 6)) {
				if (index)
					*index = i;
				return device->radio_list[i];
			}
		}
	} else if (bssid) {
		for (i = 0; i < device->number_of_radios; i++) {
			for (k = 0; k < device->radio_list[i]->number_of_bss; k++) {
				if (!memcmp(device->radio_list[i]->bss_list[k]->bssid, bssid, 6)) {
					if (index)
						*index = i;
					return device->radio_list[i];
				}
			}
		}
	}

	if (need_create_flag) {
		if (device->number_of_radios == MAX_RADIO_NUM_PER_DEVICE) {
			log_error("Error : Failed to add new radio. Max radio number reached!\n");
			return NULL; //Error when a need_create flag is set but a null is returned
		}

		if (NULL == radio_mac) {
			log_error("Error : Failed to add new radio without radio mac!\n");
			return NULL; //Error when a need_create flag is set but a null is returned
		}

		device->radio_list[device->number_of_radios] = (struct Radio *)malloc(sizeof(struct Radio));

		memcpy(device->radio_list[device->number_of_radios]->id, radio_mac, 6);
		device->radio_list[device->number_of_radios]->band                      = RADIO_BAND_UNKNOWN;
		device->radio_list[device->number_of_radios]->current_operating_classes = NULL;
		device->radio_list[device->number_of_radios]->number_of_curr_op_class   = 1; //Hard code current operating class to 1 according to our implementation
		device->radio_list[device->number_of_radios]->utilization               = 0;
		device->radio_list[device->number_of_radios]->number_of_bss             = 0;
		memset(device->radio_list[device->number_of_radios]->bss_list, 0, MAX_BSS_NUM_PER_RADIO * sizeof(struct StructBSS *));
		//vxd mac = wlan root mac
		memcpy(device->radio_list[device->number_of_radios]->backhaul_sta.mac_address, dummy_mac, 6);
		device->radio_list[device->number_of_radios]->unassociated_sta_list = NULL;
		memset(&device->radio_list[device->number_of_radios]->policies, 0, sizeof(struct SteeringPolicy));
		device->radio_list[device->number_of_radios]->number_of_unassoc_sta = 0;

		if (index)
			*index = device->number_of_radios;

		device->number_of_radios++;

		return device->radio_list[device->number_of_radios - 1];
	}

	return NULL;
}

uint8_t update_radio(struct Radio *radio, //the specific radio to update
                     struct OperatingChannelReportTLV *            operatingChannelReportTlv,
                     struct APMetricsTLV *                         apMetricsTlv,
                     struct UnassociatedSTALinkMetricsResponseTLV *unassociatedStaLinkMetricsResponseTlv,
                     uint8_t *                                     backhaul_sta_mac)
{
	unsigned char j, k;

	if (operatingChannelReportTlv && operatingChannelReportTlv->cur_op_class_nr) {
		radio->number_of_curr_op_class = operatingChannelReportTlv->cur_op_class_nr;
		if (NULL == radio->current_operating_classes) {
			radio->current_operating_classes = (struct CurrentOperatingClassProfile *)malloc(sizeof(struct CurrentOperatingClassProfile) * radio->number_of_curr_op_class);
		}
		radio->band = get_band_from_op_class(operatingChannelReportTlv->operating_channels[0].op_class);
		for (int i = 0; i < radio->number_of_curr_op_class; i++) {
			radio->current_operating_classes[i].op_class = operatingChannelReportTlv->operating_channels[i].op_class;
			radio->current_operating_classes[i].channel  = operatingChannelReportTlv->operating_channels[i].cur_channel;
			radio->current_operating_classes[i].tx_power = operatingChannelReportTlv->cur_transmit_power;
		}
	}

	if (apMetricsTlv) {
		if (memcmp(radio->id, apMetricsTlv->bssid, 6) == 0) {
			radio->utilization = apMetricsTlv->ch_util;
		}
	}

	//TODO: need to consider if overwrite the entire record vs extending it dynamically
	if (unassociatedStaLinkMetricsResponseTlv) {
		if (radio->unassociated_sta_list) {
			free(radio->unassociated_sta_list);
			radio->unassociated_sta_list = NULL;
		}

		// Filter associated stations
		int number_of_unassoc_sta  = 0;
		int unassoc_sta_index[128] = { 0 };
		for (j = 0; j < unassociatedStaLinkMetricsResponseTlv->sta_nr; j++) {
			int is_associated_sta = 0;
			for (k = 0; k < radio->number_of_bss; k++) {
				if (NULL != locate_sta(radio->bss_list[k], unassociatedStaLinkMetricsResponseTlv->unassoc_sta_link_metrics[j].sta_mac_address, NEED_CREATE_FALSE, NULL)) {
					is_associated_sta = 1;
					log_warn("Sta mac:%02x%02x%02x%02x%02x%02x is not unassocited sta!\n",
					         unassociatedStaLinkMetricsResponseTlv->unassoc_sta_link_metrics[j].sta_mac_address[0], unassociatedStaLinkMetricsResponseTlv->unassoc_sta_link_metrics[j].sta_mac_address[1], unassociatedStaLinkMetricsResponseTlv->unassoc_sta_link_metrics[j].sta_mac_address[2],
					         unassociatedStaLinkMetricsResponseTlv->unassoc_sta_link_metrics[j].sta_mac_address[3], unassociatedStaLinkMetricsResponseTlv->unassoc_sta_link_metrics[j].sta_mac_address[4], unassociatedStaLinkMetricsResponseTlv->unassoc_sta_link_metrics[j].sta_mac_address[5]);
					break;
				}
			}
			if (is_associated_sta) {
				continue;
			}
			unassoc_sta_index[number_of_unassoc_sta] = j;
			number_of_unassoc_sta += 1;
		}

		//unassociated sta list
		radio->number_of_unassoc_sta = number_of_unassoc_sta;
		if (radio->number_of_unassoc_sta)
			radio->unassociated_sta_list = malloc(sizeof(struct UnassociatedSTA) * radio->number_of_unassoc_sta);

		for (j = 0; j < radio->number_of_unassoc_sta; j++) {
			int sta_index = unassoc_sta_index[j];
			memcpy(radio->unassociated_sta_list[j].mac_address, unassociatedStaLinkMetricsResponseTlv->unassoc_sta_link_metrics[sta_index].sta_mac_address, 6);
			radio->unassociated_sta_list[j].signal_strength = unassociatedStaLinkMetricsResponseTlv->unassoc_sta_link_metrics[sta_index].uplink_rcpi;
		}
	}

	if (backhaul_sta_mac) {
		//Update backhaul_sta mac addr
		memcpy(radio->backhaul_sta.mac_address, backhaul_sta_mac, 6);
		log_debug("Update backhaul_sta mac:%02x%02x%02x%02x%02x%02x\n", radio->backhaul_sta.mac_address[0], radio->backhaul_sta.mac_address[1], radio->backhaul_sta.mac_address[2], radio->backhaul_sta.mac_address[3], radio->backhaul_sta.mac_address[4], radio->backhaul_sta.mac_address[5]);
	}

	return 0;
}

struct StructBSS *locate_bss_by_ssid(struct Radio *radio, char *ssid)
{
	unsigned char i;
	for (i = 0; i < radio->number_of_bss; i++) {
		if (strcmp(radio->bss_list[i]->ssid, ssid) == 0) {
			return radio->bss_list[i];
		}
	}
	return NULL;
}

struct StructBSS *locate_bss(struct Radio *radio, uint8_t *bssid, uint8_t need_create_flag, uint16_t *index)
{

	unsigned char i;
	for (i = 0; i < radio->number_of_bss; i++) {
		if (!memcmp(radio->bss_list[i]->bssid, bssid, 6)) {
			if (index)
				*index = i;
			return radio->bss_list[i];
		}
	}

	if (need_create_flag) {
		if (radio->number_of_bss == MAX_BSS_NUM_PER_RADIO) {
			log_error("Error : Failed to add new BSS. Max BSS number reached!\n");
			return NULL; //Error when a need_create flag is set but a null is returned
		}

		radio->bss_list[radio->number_of_bss] = (struct StructBSS *)malloc(sizeof(struct StructBSS));

		memcpy(radio->bss_list[radio->number_of_bss]->bssid, bssid, 6);
		radio->bss_list[radio->number_of_bss]->ssid        = strdup("");
		radio->bss_list[radio->number_of_bss]->sta_list = (struct STA **)malloc(max_sta_num_per_bss * sizeof(struct STA *));
		memset(radio->bss_list[radio->number_of_bss]->sta_list, 0, max_sta_num_per_bss * sizeof(struct STA *));
		radio->bss_list[radio->number_of_bss]->number_of_sta = 0;
		memset(radio->bss_list[radio->number_of_bss]->est_service_parameters_be, 0, 3);
		memset(radio->bss_list[radio->number_of_bss]->est_service_parameters_bk, 0, 3);
		memset(radio->bss_list[radio->number_of_bss]->est_service_parameters_vi, 0, 3);
		memset(radio->bss_list[radio->number_of_bss]->est_service_parameters_vo, 0, 3);
		memset(&radio->bss_list[radio->number_of_bss]->steering_opportunity, 0, sizeof(struct SteeringOpportunityBSS));

		if (index)
			*index = radio->number_of_bss;

		radio->number_of_bss++;

		return radio->bss_list[radio->number_of_bss - 1];
	}

	return NULL;
}

uint8_t update_bss(struct StructBSS *   bss,
                   char *               ssid,
                   uint8_t              ssid_len,
                   struct APMetricsTLV *apMetricsTlv)
{
	if (ssid) {
		if (NULL != bss->ssid)
			free(bss->ssid);
		//bss->ssid = strdup(ssid);
		bss->ssid = malloc(ssid_len + 1);
		memcpy(bss->ssid, ssid, ssid_len);
		bss->ssid[ssid_len] = '\0';
	}

	if (apMetricsTlv) {
		memcpy(bss->est_service_parameters_be, apMetricsTlv->esp_acbe, 3);
		memcpy(bss->est_service_parameters_bk, apMetricsTlv->esp_acbk, 3);
		memcpy(bss->est_service_parameters_vi, apMetricsTlv->esp_acvi, 3);
		memcpy(bss->est_service_parameters_vo, apMetricsTlv->esp_acvo, 3);
	}

	return 0;
}

struct STA *locate_sta(struct StructBSS *bss, uint8_t *sta_mac, uint8_t need_create_flag, uint16_t *index)
{

	unsigned char i;
	for (i = 0; i < bss->number_of_sta; i++) {
		if (!memcmp(bss->sta_list[i]->mac_address, sta_mac, 6)) {
			if (index)
				*index = i;
			return bss->sta_list[i];
		}
	}

	if (need_create_flag) {
		if (bss->number_of_sta == max_sta_num_per_bss) {
			log_error("Error : Failed to add new STA. Max STA number reached!\n");
			return NULL; //Error when a need_create flag is set but a null is returned
		}

		bss->sta_list[bss->number_of_sta] = (struct STA *)malloc(sizeof(struct STA));

		memcpy(bss->sta_list[bss->number_of_sta]->mac_address, sta_mac, 6);
		bss->sta_list[bss->number_of_sta]->last_data_downlink_rate    = 0;
		bss->sta_list[bss->number_of_sta]->last_data_uplink_rate      = 0;
		bss->sta_list[bss->number_of_sta]->est_mac_data_rate_downlink = 0;
		bss->sta_list[bss->number_of_sta]->est_mac_data_rate_uplink   = 0;
		bss->sta_list[bss->number_of_sta]->signal_strength            = 0;
		bss->sta_list[bss->number_of_sta]->bytes_sent                 = 0;
		bss->sta_list[bss->number_of_sta]->bytes_received             = 0;
		bss->sta_list[bss->number_of_sta]->packets_sent               = 0;
		bss->sta_list[bss->number_of_sta]->packets_received           = 0;
		bss->sta_list[bss->number_of_sta]->errors_sent                = 0;
		bss->sta_list[bss->number_of_sta]->errors_received            = 0;
		bss->sta_list[bss->number_of_sta]->retrans_count              = 0;
		bss->sta_list[bss->number_of_sta]->throughput                 = 0;
		bss->sta_list[bss->number_of_sta]->measurement_report         = NULL;
		bss->sta_list[bss->number_of_sta]->number_of_measure_reports  = 0;
		bss->sta_list[bss->number_of_sta]->still_present              = 0;
		bss->sta_list[bss->number_of_sta]->is_backhaul_sta            = 0;
		bss->sta_list[bss->number_of_sta]->curr_bss_score             = 0;
		memset(bss->sta_list[bss->number_of_sta]->best_bss_mac, 0, 6);
		bss->sta_list[bss->number_of_sta]->best_bss_count     = 0;
		bss->sta_list[bss->number_of_sta]->best_bss_score     = 0;
		bss->sta_list[bss->number_of_sta]->status             = 0;
		bss->sta_list[bss->number_of_sta]->passive_roam_count = 0;
		bss->sta_list[bss->number_of_sta]->misc_count         = 0;
		bss->sta_list[bss->number_of_sta]->capability_recv    = 0;
		bss->sta_list[bss->number_of_sta]->sent_11k           = 0;
		memset(bss->sta_list[bss->number_of_sta]->rm_cap, 0, 5);
		bss->sta_list[bss->number_of_sta]->support_bss_transition     = 0xFF;
		bss->sta_list[bss->number_of_sta]->mbo_cap = 0;

		bss->sta_list[bss->number_of_sta]->rcpi.bcn_rpt_nr = 0;

		if (index)
			*index = bss->number_of_sta;

		bss->number_of_sta++;

		return bss->sta_list[bss->number_of_sta - 1];
	}

	return NULL;
}

uint8_t update_sta(struct STA *                         sta,
                   struct AssocSTALinkMetrics *         assocStaLinkMetrics,
                   struct AssociatedSTATrafficStatsTLV *associatedStaTrafficStatsTlv,
                   struct BeaconMetricsResponseTLV *    beaconMetricsResponseTlv)
{
	if (assocStaLinkMetrics) {
		sta->est_mac_data_rate_downlink = assocStaLinkMetrics->dataRate_downlink;
		sta->est_mac_data_rate_uplink   = assocStaLinkMetrics->dataRate_uplink;
		sta->signal_strength            = assocStaLinkMetrics->uplink_rcpi;
	}

	if (associatedStaTrafficStatsTlv) {
		sta->throughput       = ((((associatedStaTrafficStatsTlv->bytes_sent - sta->bytes_sent) + (associatedStaTrafficStatsTlv->bytes_received - sta->bytes_received)) * 8) / (10 * 1000));
		sta->last_data_downlink_rate    = ((associatedStaTrafficStatsTlv->bytes_received - sta->bytes_received)* 8) / (10 * 1000*1000);
		sta->last_data_uplink_rate      = ((associatedStaTrafficStatsTlv->bytes_sent - sta->bytes_sent)* 8) / (10 * 1000*1000);
		sta->bytes_sent       = associatedStaTrafficStatsTlv->bytes_sent;
		sta->bytes_received   = associatedStaTrafficStatsTlv->bytes_received;
		sta->packets_sent     = associatedStaTrafficStatsTlv->packets_sent;
		sta->packets_received = associatedStaTrafficStatsTlv->packets_received;
		sta->errors_sent      = associatedStaTrafficStatsTlv->tx_packets_errors;
		sta->errors_received  = associatedStaTrafficStatsTlv->rx_packets_errors;
		sta->retrans_count    = associatedStaTrafficStatsTlv->retransmission_count;
	}

	if (beaconMetricsResponseTlv) {
		sta->number_of_measure_reports = beaconMetricsResponseTlv->measurement_report_nr;
		if (sta->measurement_report) {
			free(sta->measurement_report);
			sta->measurement_report = NULL;
		}
		if (sta->number_of_measure_reports) {
			sta->measurement_report = (struct BeaconMeasurementReportIE *)malloc(sta->number_of_measure_reports * sizeof(struct BeaconMeasurementReportIE));
			memcpy(sta->measurement_report, beaconMetricsResponseTlv->measurement_reports, sta->number_of_measure_reports * sizeof(struct BeaconMeasurementReportIE));
		}
	}

	return 0;
}

struct STA *find_sta(struct Network *network, uint8_t *sta_mac)
{
	unsigned char i, j, k, l;
	//Devices
	for (i = 0; i < network->number_of_devices; i++) {
		//Radios
		for (j = 0; j < network->device_list[i]->number_of_radios; j++) {
			//BSS
			for (k = 0; k < network->device_list[i]->radio_list[j]->number_of_bss; k++) {
				//Sta
				for (l = 0; l < network->device_list[i]->radio_list[j]->bss_list[k]->number_of_sta; l++) {
					if (memcmp(network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l]->mac_address, sta_mac, 6) == 0)
						return network->device_list[i]->radio_list[j]->bss_list[k]->sta_list[l];
				}
			}
		}
	}

	return NULL;
}

struct STA *locate_sta_by_mac(struct Network *network, struct Device *curr_device, uint8_t *sta_mac, struct Device **device, struct Radio **radio, struct StructBSS **bss)
{
	int i, j, k, l;
	if (network) {
		for (l = 0; l < network->number_of_devices; l++) {
			for (i = 0; i < network->device_list[l]->number_of_radios; i++) {
				for (j = 0; j < network->device_list[l]->radio_list[i]->number_of_bss; j++) {
					for (k = 0; k < network->device_list[l]->radio_list[i]->bss_list[j]->number_of_sta; k++) {
						if (!memcmp(network->device_list[l]->radio_list[i]->bss_list[j]->sta_list[k]->mac_address, sta_mac, 6)) {
							if (device)
								*device = network->device_list[l];
							if (radio)
								*radio = network->device_list[l]->radio_list[i];
							if (bss)
								*bss = network->device_list[l]->radio_list[i]->bss_list[j];
							return network->device_list[l]->radio_list[i]->bss_list[j]->sta_list[k];
						}
					}
				}
			}
		}
	} else if (curr_device) {
		for (i = 0; i < curr_device->number_of_radios; i++) {
			for (j = 0; j < curr_device->radio_list[i]->number_of_bss; j++) {
				for (k = 0; k < curr_device->radio_list[i]->bss_list[j]->number_of_sta; k++) {
					if (!memcmp(curr_device->radio_list[i]->bss_list[j]->sta_list[k]->mac_address, sta_mac, 6)) {
						if (radio)
							*radio = curr_device->radio_list[i];
						if (bss)
							*bss = curr_device->radio_list[i]->bss_list[j];
						return curr_device->radio_list[i]->bss_list[j]->sta_list[k];
					}
				}
			}
		}
	}
	return NULL;
}

uint8_t free_sta(struct StructBSS *bss, uint8_t *sta_mac)
{
	struct STA *sta = NULL;
	uint16_t    index;
	sta = locate_sta(bss, sta_mac, 0, &index);

	if (sta) {
		// subelement didn't get allocated and copied, it's just a dummy pointer, no need to free
		if (sta->measurement_report)
			free(sta->measurement_report);

		free(sta);
		bss->number_of_sta -= 1;

		if (index == bss->number_of_sta)
			return 0;
		else
			bss->sta_list[index] = bss->sta_list[bss->number_of_sta];

	} else {
		log_warn("Sta %02x%02x%02x%02x%02x%02x not found!\n", sta_mac[0], sta_mac[1], sta_mac[2], sta_mac[3], sta_mac[4], sta_mac[5]);
	}

	return 0;
}

uint8_t free_bss(struct Radio *radio, uint8_t *bss_mac)
{
	struct StructBSS *bss = NULL;
	uint16_t          sta_nr, index, sta_list_nr;

	bss = locate_bss(radio, bss_mac, 0, &index);

	if (bss) {
		sta_list_nr = bss->number_of_sta;
		for (sta_nr = 0; sta_nr < sta_list_nr; sta_nr++) {
			free_sta(bss, bss->sta_list[sta_nr]->mac_address);
		}

		if (bss->ssid)
			free(bss->ssid);

		if (bss->sta_list)
			free(bss->sta_list);

		free(bss);
		radio->number_of_bss -= 1;

		if (index == radio->number_of_bss)
			return 0;
		else
			radio->bss_list[index] = radio->bss_list[radio->number_of_bss];

	} else {
		log_warn("Bss %02x%02x%02x%02x%02x%02x not found!\n", bss_mac[0], bss_mac[1], bss_mac[2], bss_mac[3], bss_mac[4], bss_mac[5]);
	}

	return 0;
}

uint8_t free_radio(struct Device *device, uint8_t *radio_mac)
{
	struct Radio *radio = NULL;
	uint16_t      bss_nr, index, bss_list_nr;
	radio = locate_radio(device, radio_mac, 0, NULL, 0, &index);

	if (radio) {
		bss_list_nr = radio->number_of_bss;
		for (bss_nr = 0; bss_nr < bss_list_nr; bss_nr++) {
			free_bss(radio, radio->bss_list[bss_nr]->bssid);
		}

		if (radio->number_of_curr_op_class)
			free(radio->current_operating_classes);

		if (radio->unassociated_sta_list)
			free(radio->unassociated_sta_list);

		free(radio);
		device->number_of_radios -= 1;

		if (index == device->number_of_radios)
			return 0;
		else
			device->radio_list[index] = device->radio_list[device->number_of_radios];

	} else {
		log_warn("Radio %02x%02x%02x%02x%02x%02x not found!\n", radio_mac[0], radio_mac[1], radio_mac[2], radio_mac[3], radio_mac[4], radio_mac[5]);
	}

	return 0;
}

uint8_t free_device(struct Network *network, uint8_t *device_mac)
{
	struct Device *device = NULL;
	uint16_t       radio_nr, index, radio_list_nr;
	//Find the target device to free
	device = locate_device(network, device_mac, 0, &index);

	if (device) {
		radio_list_nr = device->number_of_radios;
		for (radio_nr = 0; radio_nr < radio_list_nr; radio_nr++) {
			free_radio(device, device->radio_list[radio_nr]->id);
		}
		if (device->non_1905_clients)
			free(device->non_1905_clients);
		if (device->direct_neighbor_list)
			free(device->direct_neighbor_list);
		if (device->device_name)
			free(device->device_name);
		if (device->manufacturer)
			free(device->manufacturer);
		if (device->model_name)
			free(device->model_name);
		if (device->ip_addr)
			free(device->ip_addr);
		free(device);
		network->number_of_devices -= 1;
		if (index == network->number_of_devices)
			return 0;
		else
			network->device_list[index] = network->device_list[network->number_of_devices];
	} else {
		log_warn("Device %02x%02x%02x%02x%02x%02x not found!\n", device_mac[0], device_mac[1], device_mac[2], device_mac[3], device_mac[4], device_mac[5]);
	}

	return 0;
}

uint8_t _remove_duplicated_neighbor(struct Device *curr_device)
{
	uint8_t i, ret = 0;
	for (i = 0; i < curr_device->number_of_direct_neighbors - 1; i++) {
		uint8_t start = i + 1;
		while (start < curr_device->number_of_direct_neighbors) {
			if (!memcmp(&curr_device->direct_neighbor_list[i], &curr_device->direct_neighbor_list[start], sizeof(struct DeviceNeighbor))) {
				//if duplicated found, remove it
				memcpy(&curr_device->direct_neighbor_list[start], &curr_device->direct_neighbor_list[start + 1], (curr_device->number_of_direct_neighbors - start - 1) * sizeof(struct DeviceNeighbor));
				curr_device->number_of_direct_neighbors--;
				ret = 1;
			} else {
				start++;
			}
		}
	}
	return ret;
}

void _obtain_child_connection_info(struct Device *parent, struct Device *child, int *rssi, uint8_t *connected_band)
{
	struct STA *backhaul_sta = NULL;
	*connected_band          = RADIO_BAND_ETH;
	int i                    = 0;
	for (i = 0; i < child->number_of_radios; i++) {
		backhaul_sta = locate_sta_by_mac(NULL, parent, child->radio_list[i]->backhaul_sta.mac_address, NULL, NULL, NULL);
		if (backhaul_sta) {
			if (child->radio_list[i]->current_operating_classes) {
				*connected_band = get_band_from_op_class(child->radio_list[i]->current_operating_classes->op_class); //get_hw_reg_domain()
			} else {
				*connected_band = RADIO_BAND_UNKNOWN;
			}
			backhaul_sta->is_backhaul_sta = 1;
			break;
		}
	}

	if (NULL == backhaul_sta) {
		*rssi = -1; // the agent is connected by ethernet
	} else {
		*rssi = rcpi_to_rssi(backhaul_sta->signal_strength);
	}
}

const char *_connected_band_to_string(uint8_t band, struct Device *curr_device)
{
	if (RADIO_BAND_2G == band) {
		return "2G";
	} else if (RADIO_BAND_5GL == band || RADIO_BAND_5GH == band) {
		if (curr_device->number_of_radios == 3) {
			if (RADIO_BAND_5GL == band) {
				return "5GL";
			} else {
				return "5GH";
			}
		} else {
			return "5G";
		}
	} else if (RADIO_BAND_UNKNOWN == band) {
		return "TBU";
	} else if (RADIO_BAND_ETH == band) {
		return "ETH";
	} else {
		return "ERR";
	}
}

void show_all_open_interface_topology_json(struct Device *curr_device, FILE *fp)
{
	uint8_t is_first = 1, tmp_band = 0xFF;
	uint8_t i, k=0;
	int j;

	fprintf(fp, "\"open interface\":[");

	for (i = 0; i < curr_device->number_of_radios; i++) {
		k = 0;
		tmp_band = 0xFF;
		if (curr_device->radio_list[i]->current_operating_classes) {
			tmp_band = get_band_from_op_class(curr_device->radio_list[i]->current_operating_classes->op_class);
		}

		fprintf(fp, "{");
		is_first = 1;

		fprintf(fp, "\"device_name\":\"%s\",", curr_device->device_name);

		fprintf(fp, "\"Radio\":\"");
		if (RADIO_BAND_2G == tmp_band) {
			fprintf(fp, "2G");
		} else if (curr_device->number_of_radios == 3) {
			if (RADIO_BAND_5GH == tmp_band) {
				fprintf(fp, "5GH");
			} else if (RADIO_BAND_5GL == tmp_band) {
				fprintf(fp, "5GL");
			} else {
				fprintf(fp, "UNKNOWN");
			}
		} else if (RADIO_BAND_5GH == tmp_band || RADIO_BAND_5GL == tmp_band) {
			fprintf(fp, "5G");
		} else {
			fprintf(fp, "UNKNOWN");
		}
		fprintf(fp, "\",");
		fprintf(fp, "\"interface\":[");

		for (j = (curr_device->radio_list[i]->number_of_bss - 1); j >= 0; j--) {
			if (is_first) {
				is_first = 0;
			} else {
				fprintf(fp, ",");
			}

			fprintf(fp, "{");

			fprintf(fp, "\"index\":\"");
			fprintf(fp, "%d", k);
			fprintf(fp, "\",");
			fprintf(fp, "\"Bss_mac\":\"%02x%02x%02x%02x%02x%02x\"", curr_device->radio_list[i]->bss_list[j]->bssid[0], curr_device->radio_list[i]->bss_list[j]->bssid[1], curr_device->radio_list[i]->bss_list[j]->bssid[2], curr_device->radio_list[i]->bss_list[j]->bssid[3], curr_device->radio_list[i]->bss_list[j]->bssid[4], curr_device->radio_list[i]->bss_list[j]->bssid[5]);
			fprintf(fp, "}");
			k++;
		}
		fprintf(fp, "]}");
		if(i != (curr_device->number_of_radios - 1))
			fprintf(fp, ",");
	}
	fprintf(fp, "],");
}

uint8_t tree_topology_json_export(struct Network *network, struct Device *curr_device, uint8_t *parent_mac, FILE *fp, uint8_t depth, uint8_t hw_reg_doamin, uint8_t physical_distance, uint32_t custom_features)
{
	uint8_t result   = 0;
	uint8_t is_first = 1;
	uint8_t i, j, k;

	if (1 == curr_device->is_visited) {
		// log_debug("Skip visited device %02x:%02x:%02x:%02x:%02x:%02x\n", curr_device->id[0], curr_device->id[1], curr_device->id[2], curr_device->id[3], curr_device->id[4], curr_device->id[5]);
		return 0;
	}

	if (depth > 6) {
		return 1;
	}

	if (1 != depth) {
		curr_device->logical_distance_to_controller = depth - 1;
		curr_device->physical_distance_to_controller = physical_distance;
	} else {
		curr_device->logical_distance_to_controller = 0;
		curr_device->physical_distance_to_controller = 0;
	}

	if(MULTI_AP_PROFILE_2 > curr_device->multi_ap_profile) {
		curr_device->need_vlan_configure = VLAN_CONFIGURE_NO_NEED;
	}

	memcpy(curr_device->parent_id, parent_mac, 6);

	curr_device->is_visited = 1;

	//temporary fix for duplicate neighbor device in web-ui
	// _remove_duplicated_neighbor(curr_device);

	fprintf(fp, "{");
	fprintf(fp, "\"device_name\":\"%s\",", curr_device->device_name);
	if (curr_device->manufacturer)
		fprintf(fp, "\"manufacturer\":\"%s\",", curr_device->manufacturer);
	if (curr_device->model_name)
		fprintf(fp, "\"model_name\":\"%s\",", curr_device->model_name);
	fprintf(fp, "\"ip_addr\":\"%s\",", curr_device->ip_addr);
	fprintf(fp, "\"mac_address\":\"%02x%02x%02x%02x%02x%02x\",", curr_device->id[0], curr_device->id[1], curr_device->id[2], curr_device->id[3], curr_device->id[4], curr_device->id[5]);

	if (depth != 1) {
		struct Device *parent         = locate_device(network, parent_mac, NEED_CREATE_FALSE, NULL);
		uint8_t        connected_band = RADIO_BAND_ETH; //eth
		int            rssi           = 0;
		_obtain_child_connection_info(parent, curr_device, &rssi, &connected_band);

		fprintf(fp, "\"child_rssi\":\"%d\",", rssi);
		fprintf(fp, "\"child_band\":\"%s\",", _connected_band_to_string(connected_band, curr_device));
	}

	fprintf(fp, "\"lan_port_info\":[");
	is_first = 1;
	for (i = 0; i < curr_device->non_1905_client_nr; i++) {
		if(NULL != locate_sta_by_mac(NULL, curr_device, curr_device->non_1905_clients[i].mac, NULL, NULL, NULL)){
			continue;
		}

		if(NULL != locate_device(network, curr_device->non_1905_clients[i].mac, NEED_CREATE_FALSE, NULL)){
			continue;
		}

		if(NULL != locate_sta_by_mac(network, NULL, curr_device->non_1905_clients[i].mac, NULL, NULL, NULL)){
			continue;
		}

		if (is_first) {
			is_first = 0;
		} else {
			fprintf(fp, "%s", ",");
		}
		fprintf(fp, "{");
				fprintf(fp, "\"Non_1905_neighbour_mac_address\":");
				fprintf(fp, "\"");
				fprintf(fp, "%02x", curr_device->non_1905_clients[i].mac[0]);
				fprintf(fp, "%02x", curr_device->non_1905_clients[i].mac[1]);
				fprintf(fp, "%02x", curr_device->non_1905_clients[i].mac[2]);
				fprintf(fp, "%02x", curr_device->non_1905_clients[i].mac[3]);
				fprintf(fp, "%02x", curr_device->non_1905_clients[i].mac[4]);
				fprintf(fp, "%02x", curr_device->non_1905_clients[i].mac[5]);
				fprintf(fp, "\"");
		fprintf(fp, "}");
	}
	fprintf(fp, "%s", "]");
	fprintf(fp, "%s", ",");

	fprintf(fp, "\"station_info\":[");

	is_first = 1;

	//TODO: Add support for ethernet. For ethernet stas that are not inside station lists
	for (i = 0; i < curr_device->number_of_radios; i++) {
		for (j = 0; j < curr_device->radio_list[i]->number_of_bss; j++) {
			uint8_t operating_band = 0xFF;
			if (curr_device->radio_list[i]->current_operating_classes) {
				operating_band = get_band_from_op_class(curr_device->radio_list[i]->current_operating_classes->op_class);
			}
			for (k = 0; k < curr_device->radio_list[i]->bss_list[j]->number_of_sta; k++) {
				if (curr_device->radio_list[i]->bss_list[j]->sta_list[k]->is_backhaul_sta) {
					continue;
				}
				if (is_first) {
					is_first = 0;
				} else {
					fprintf(fp, ",");
				}
				fprintf(fp, "{");
				fprintf(fp, "\"station_mac\":\"%02x%02x%02x%02x%02x%02x\",", curr_device->radio_list[i]->bss_list[j]->sta_list[k]->mac_address[0], curr_device->radio_list[i]->bss_list[j]->sta_list[k]->mac_address[1], curr_device->radio_list[i]->bss_list[j]->sta_list[k]->mac_address[2], curr_device->radio_list[i]->bss_list[j]->sta_list[k]->mac_address[3], curr_device->radio_list[i]->bss_list[j]->sta_list[k]->mac_address[4], curr_device->radio_list[i]->bss_list[j]->sta_list[k]->mac_address[5]);
				fprintf(fp, "\"station_rssi\":\"%d\",", rcpi_to_rssi(curr_device->radio_list[i]->bss_list[j]->sta_list[k]->signal_strength));
				fprintf(fp, "\"station_connected_band\":\"");
				if (RADIO_BAND_2G == operating_band) {
					fprintf(fp, "2G");
				} else if (curr_device->number_of_radios == 3) {
					if (RADIO_BAND_5GH == operating_band) {
						fprintf(fp, "5GH");
					} else if (RADIO_BAND_5GL == operating_band) {
						fprintf(fp, "5GL");
					} else {
						fprintf(fp, "UNKNOWN");
					}
				} else if (RADIO_BAND_5GH == operating_band || RADIO_BAND_5GL == operating_band) {
					fprintf(fp, "5G");
				} else {
					fprintf(fp, "UNKNOWN");
				}
				fprintf(fp, "\",");
				if(custom_features & SUPPORT_SHOW_EXTRA_TOPOLOGY)
				{
					fprintf(fp, "\"station_connected_bssid\":\"%02x%02x%02x%02x%02x%02x\",", curr_device->radio_list[i]->bss_list[j]->bssid[0], curr_device->radio_list[i]->bss_list[j]->bssid[1], curr_device->radio_list[i]->bss_list[j]->bssid[2], curr_device->radio_list[i]->bss_list[j]->bssid[3], curr_device->radio_list[i]->bss_list[j]->bssid[4], curr_device->radio_list[i]->bss_list[j]->bssid[5]);
					fprintf(fp, "\"station_connected_ssid\":\"%s\",", curr_device->radio_list[i]->bss_list[j]->ssid);
				}
				fprintf(fp, "\"station_downlink\":\"%u\",", curr_device->radio_list[i]->bss_list[j]->sta_list[k]->est_mac_data_rate_downlink);
				fprintf(fp, "\"station_uplink\":\"%u\",", curr_device->radio_list[i]->bss_list[j]->sta_list[k]->est_mac_data_rate_uplink);
				fprintf(fp, "\"last_data_downlink_rate\":\"%u\",", curr_device->radio_list[i]->bss_list[j]->sta_list[k]->last_data_downlink_rate);
				fprintf(fp, "\"last_data_uplink_rate\":\"%u\"", curr_device->radio_list[i]->bss_list[j]->sta_list[k]->last_data_uplink_rate);

				if (curr_device->radio_list[i]->bss_list[j]->sta_list[k]->support_bss_transition <= 1)
					fprintf(fp, ",\"bss_transition_support\":\"%u\"", curr_device->radio_list[i]->bss_list[j]->sta_list[k]->support_bss_transition);
				fprintf(fp, "}");
			}
		}
	}
	fprintf(fp, "],");

	uint8_t         child_nr   = 0;
	struct Device **child_list = NULL;

	for (i = 0; i < curr_device->number_of_direct_neighbors; i++) {
		if (memcmp(parent_mac, curr_device->direct_neighbor_list[i].al_mac, 6)) {
			struct Device *child = locate_device(network, curr_device->direct_neighbor_list[i].al_mac, NEED_CREATE_FALSE, NULL);
			if (NULL == child || child->is_visited) //device no longer in database, neighbor list will be updated next time when receive topology response
				continue;
			child->is_visited = 2;
			child_nr += 1;
			child_list               = (struct Device **)realloc(child_list, sizeof(struct Device *) * child_nr);
			child_list[child_nr - 1] = child;
			// log_info("Adding child %d %02x:%02x:%02x:%02x:%02x:%02x\n", child_nr - 1, child->id[0], child->id[1], child->id[2], child->id[3], child->id[4], child->id[5]);
		}
	}

	fprintf(fp, "\"neighbor_devices\":[");
	if (child_nr) {
		is_first = 1;
		for (i = 0; i < child_nr; i++) {
			uint8_t connected_band = RADIO_BAND_ETH; //eth
			int     rssi           = 0;

			_obtain_child_connection_info(curr_device, child_list[i], &rssi, &connected_band);
			if (is_first) {
				is_first = 0;
			} else {
				fprintf(fp, ",");
			}

			fprintf(fp, "{");
			fprintf(fp, "\"neighbor_mac\":\"%02x%02x%02x%02x%02x%02x\",", child_list[i]->id[0], child_list[i]->id[1], child_list[i]->id[2], child_list[i]->id[3], child_list[i]->id[4], child_list[i]->id[5]);
			fprintf(fp, "\"neighbor_name\":\"%s\",", child_list[i]->device_name);
			fprintf(fp, "\"neighbor_rssi\":\"%d\",", rssi);
			fprintf(fp, "\"neighbor_band\":\"%s\"", _connected_band_to_string(connected_band, curr_device));
			fprintf(fp, "}");
		}
	}
	fprintf(fp, "],");

	if(custom_features & SUPPORT_SHOW_EXTRA_TOPOLOGY) {
		show_all_open_interface_topology_json(curr_device, fp);
	}

	//child_devices
	fprintf(fp, "\"child_devices\":[");

	if (child_nr) {
		is_first = 1;
		for (i = 0; i < child_nr; i++) {
			if (is_first) {
				is_first = 0;
			} else {
				fprintf(fp, ",");
			}

			struct STA *curr_sta = NULL;
			for(j = 0; j < child_list[i]->number_of_radios; j++){
				curr_sta = locate_sta_by_mac(NULL, curr_device, child_list[i]->radio_list[j]->backhaul_sta.mac_address, NULL, NULL, NULL);
				if (curr_sta)
					break;
			}

			if(VLAN_CONFIGURE_NO_NEED == curr_device->need_vlan_configure) {
				child_list[i]->need_vlan_configure = VLAN_CONFIGURE_NO_NEED;
			} else if(VLAN_CONFIGURE_NO_NEED == child_list[i]->need_vlan_configure) {
				child_list[i]->need_vlan_configure = VLAN_CONFIGURE_NOT_READY;
			}

			if (NULL == curr_sta) {
				result = tree_topology_json_export(network, child_list[i], curr_device->id, fp, depth + 1, hw_reg_doamin, physical_distance, custom_features);
			} else {
				result = tree_topology_json_export(network, child_list[i], curr_device->id, fp, depth + 1, hw_reg_doamin, physical_distance + 1, custom_features);
			}

			if (result) {
				fprintf(fp, "]}");
				free(child_list);
				return result;
			}
		}
	}
	fprintf(fp, "]");

	if (child_list)
		free(child_list);

	fprintf(fp, "}");

	return result;
}

struct Device *locate_device_by_bss_mac(struct Network *network, uint8_t *mac_addr)
{
	int             i, j, k;
	struct Network *network_pt;
	network_pt = network;
	if (NULL == network_pt) {
		network_pt = global_network;
	}

	for (i = 0; i < network_pt->number_of_devices; i++) {
		struct Device *device = network_pt->device_list[i];
		if (0 == memcmp(device->id, mac_addr, 6))
			return device;
		for (j = 0; j < device->number_of_radios; j++) {
			struct Radio *radio = device->radio_list[j];
			for (k = 0; k < radio->number_of_bss; k++) {
				struct StructBSS *bss = radio->bss_list[k];
				if (0 == memcmp(bss->bssid, mac_addr, 6)) {
					return device;
				}
			}
			if (0 == memcmp(radio->backhaul_sta.mac_address, mac_addr, 6)) {
				return device;
			}
		}
	}

	return NULL;
}

unsigned char *find_bsta_mac_by_al_mac(struct Network *network, unsigned char *al_mac, int band)
{
	int i;

	if (NULL == al_mac) {
		return NULL;
	}

	if (NULL == network) {
		network = global_network;
	}

	struct Device *device = locate_device_by_al_mac(network, al_mac);
	if (device) {
		for (i = 0; i < device->number_of_radios; i++) {
			if (band | get_band_from_op_class(device->radio_list[i]->current_operating_classes[0].op_class)) {
				return device->radio_list[i]->backhaul_sta.mac_address;
			}
		}
	}
	log_warn("unable to located backhaul sta for device %02x%02x%02x%02x%02x%02x\n", al_mac[0], al_mac[1], al_mac[2], al_mac[3], al_mac[4], al_mac[5]);

	return NULL;
}

unsigned char *find_al_mac_by_bss_mac(struct Network *network, unsigned char *mac_addr)
{
	if (NULL == mac_addr)
		return NULL;

	if (NULL == network) {
		network = global_network;
	}

	struct Device *device = locate_device_by_bss_mac(network, mac_addr);
	if (device)
		return device->id;

	log_warn("unable to located al_mac for mac %02x%02x%02x%02x%02x%02x\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
	return NULL;
}

void update_need_vlan_configure(struct Network *network, struct Device *device)
{
	int x, y, z, m; //Iterator for data element use

	struct Device *controller_device = get_controller_device(network);

	for (x = 0; x < controller_device->number_of_radios; x++) {
		for (y = 0; y < device->number_of_radios; y++) {
			if ((NULL == controller_device->radio_list[x]->current_operating_classes)
				|| (NULL == device->radio_list[y]->current_operating_classes)) {
				log_error("Current operating class has not been updated!\n");
				return;
			}
			if (controller_device->radio_list[x]->current_operating_classes[0].channel == device->radio_list[y]->current_operating_classes[0].channel) {
				if (controller_device->radio_list[x]->number_of_bss == device->radio_list[y]->number_of_bss) {
					int                              deleted_sum = 0;
					uint8_t                          bss_number  = controller_device->radio_list[x]->number_of_bss;
					struct vlan_need_configure_check controller_bss_set;
					struct vlan_need_configure_check device_bss_set;

					memset(&controller_bss_set, 0, sizeof(controller_bss_set));
					memset(&device_bss_set, 0, sizeof(device_bss_set));

					for (z = 0; z < bss_number; z++) {
						controller_bss_set.ssid[z] = controller_device->radio_list[x]->bss_list[z]->ssid;
						device_bss_set.ssid[z]     = device->radio_list[y]->bss_list[z]->ssid;
					}

					for (z = 0; z < bss_number; z++) {
						for (m = 0; m < bss_number; m++) {
							if (!device_bss_set.deleted[m]) {
								if (!strcmp(controller_bss_set.ssid[z], device_bss_set.ssid[m])) {
									device_bss_set.deleted[m] = 1;
									deleted_sum += 1;
									break;
								}
							}
						}
					}

					if (controller_device->radio_list[x]->number_of_bss != deleted_sum) {
						log_error("SSID haven't been configured yet!\n");
						return;
					}
				} else {
					log_error("SSID haven't been sync yet!\n");
					return;
				}
			}
		}
	}
	if ((x != 0) && (x == controller_device->number_of_radios))
		device->need_vlan_configure = VLAN_CONFIGURE_PENDING;
}

uint8_t topology_db_update(struct Network *network, struct Device *curr_device, uint8_t *parent_mac, uint8_t depth, uint8_t physical_distance)
{
	uint8_t             result = 0;
	uint8_t             i, j;
	char                depth_space[32];

	if (1 == curr_device->is_visited) {
		return 0;
	}

	if (depth > 6) {
		log_warn("\x1B[31m");
		log_warn("topology_db_update recursion issue!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		log_warn("\x1B[0m");
		return 1;
	}

	if (0 != depth) {
		curr_device->logical_distance_to_controller  = depth;
		curr_device->physical_distance_to_controller = physical_distance;
	} else {
		curr_device->logical_distance_to_controller  = 0;
		curr_device->physical_distance_to_controller = 0;
	}
	memcpy(curr_device->parent_id, parent_mac, 6);

	curr_device->is_visited = 1;

	_remove_duplicated_neighbor(curr_device);

	memset(depth_space, 0, sizeof(depth_space));
	for (i = 0; i < depth; i++)
		sprintf(depth_space, "  ");

	log_debug("%sDevice %02x:%02x:%02x:%02x:%02x:%02x "
		"| Parent %02x:%02x:%02x:%02x:%02x:%02x "
		"| Logical distance to controller %d "
		"| Physical distance to controller %d\n",
		depth_space,
		curr_device->id[0], curr_device->id[1], curr_device->id[2],
		curr_device->id[3], curr_device->id[4], curr_device->id[5],
		curr_device->parent_id[0], curr_device->parent_id[1], curr_device->parent_id[2],
		curr_device->parent_id[3], curr_device->parent_id[4], curr_device->parent_id[5],
		curr_device->logical_distance_to_controller,
		curr_device->physical_distance_to_controller);

	if (curr_device->number_of_direct_neighbors) {
		uint8_t         child_nr   = 0;
		struct Device **child_list = NULL;
		for (i = 0; i < curr_device->number_of_direct_neighbors; i++) {
			// log_debug("Checking neighbor %d %02x:%02x:%02x:%02x:%02x:%02x\n", i, curr_device->direct_neighbor_list[i].al_mac[0], curr_device->direct_neighbor_list[i].al_mac[1], curr_device->direct_neighbor_list[i].al_mac[2], curr_device->direct_neighbor_list[i].al_mac[3], curr_device->direct_neighbor_list[i].al_mac[4], curr_device->direct_neighbor_list[i].al_mac[5]);
			//if it is not equals to parent device, step into
			if (memcmp(parent_mac, curr_device->direct_neighbor_list[i].al_mac, 6)) {
				struct Device *child = locate_device(network, curr_device->direct_neighbor_list[i].al_mac, NEED_CREATE_FALSE, NULL);
				if (NULL == child || child->is_visited) //device no longer in database, neighbor list will be updated next time when receive topology response
					continue;
				child->is_visited = 2;
				child_nr += 1;
				child_list               = (struct Device **)realloc(child_list, sizeof(struct Device *) * child_nr);
				child_list[child_nr - 1] = child;
				// log_debug("Adding child %d %02x:%02x:%02x:%02x:%02x:%02x\n", child_nr - 1, child->id[0], child->id[1], child->id[2], child->id[3], child->id[4], child->id[5]);
			}
		}

		if (child_nr) {
			for (i = 0; i < child_nr; i++) {
				log_debug("Visiting child %d %02x:%02x:%02x:%02x:%02x:%02x\n",
					i,
					child_list[i]->id[0], child_list[i]->id[1],
					child_list[i]->id[2], child_list[i]->id[3],
					child_list[i]->id[4], child_list[i]->id[5]);

				struct STA *curr_sta = NULL;
				for (j = 0; j < child_list[i]->number_of_radios; j++) {
					log_debug("locating station %d %02x:%02x:%02x:%02x:%02x:%02x\n",
						i,
						child_list[i]->radio_list[j]->backhaul_sta.mac_address[0],
						child_list[i]->radio_list[j]->backhaul_sta.mac_address[1],
						child_list[i]->radio_list[j]->backhaul_sta.mac_address[2],
						child_list[i]->radio_list[j]->backhaul_sta.mac_address[3],
						child_list[i]->radio_list[j]->backhaul_sta.mac_address[4],
						child_list[i]->radio_list[j]->backhaul_sta.mac_address[5]);
					curr_sta = locate_sta_by_mac(NULL, curr_device, child_list[i]->radio_list[j]->backhaul_sta.mac_address, NULL, NULL, NULL);
					if (curr_sta)
						break;
				}
				if (NULL == curr_sta) {
					result = topology_db_update(network, child_list[i], curr_device->id, depth + 1, physical_distance);
				} else {
					result = topology_db_update(network, child_list[i], curr_device->id, depth + 1, physical_distance + 1);
				}

				if (result) {
					free(child_list);
					return result;
				}
			}
			free(child_list);
		}
	}

	return result;
}

uint8_t database_garbage_collection(struct Network *network, uint8_t device_al_mac[6], uint16_t threshold)
{
	unsigned char i;

	for (i = 0; i < network->number_of_devices; i++) {
		time_t now;
		now = time(&now);
		if (difftime(now, network->device_list[i]->updated_timestamp) > threshold) {
			if (memcmp(network->device_list[i]->id, device_al_mac, 6))
				free_device(network, network->device_list[i]->id);
		}
	}

	return 0;
}

//This function is for retrieving radio in a given device
//There are 1 possible scenarios
//1. Locate neighbor connected radio by local connected bss id for TLV_TYPE_RECEIVER_LINK_METRIC
struct Radio *locate_backhaul_radio(struct Device *local_device, struct Device *neighbor_device, uint8_t *neighbor_bssid)
{
	unsigned char i, k;
	uint8_t radio_band = RADIO_BAND_UNKNOWN;
	//locate by bss_id
	if (neighbor_bssid) {
		for (i = 0; i < neighbor_device->number_of_radios; i++) {
			for (k = 0; k < neighbor_device->radio_list[i]->number_of_bss; k++) {
				log_detail("Compare %02x:%02x:%02x:%02x:%02x:%02x with %02x:%02x:%02x:%02x:%02x:%02x\n", neighbor_device->radio_list[i]->bss_list[k]->bssid[0], neighbor_device->radio_list[i]->bss_list[k]->bssid[1], neighbor_device->radio_list[i]->bss_list[k]->bssid[2], neighbor_device->radio_list[i]->bss_list[k]->bssid[3], neighbor_device->radio_list[i]->bss_list[k]->bssid[4], neighbor_device->radio_list[i]->bss_list[k]->bssid[5], neighbor_bssid[0], neighbor_bssid[1], neighbor_bssid[2], neighbor_bssid[3], neighbor_bssid[4], neighbor_bssid[5]);
				if (0 == memcmp(neighbor_device->radio_list[i]->bss_list[k]->bssid, neighbor_bssid, 6)) {
					//find backhaul radio index by neighbor connected interface mac
					log_detail("Found radio with mac %02x:%02x:%02x:%02x:%02x:%02x with band %d\n", neighbor_bssid[0], neighbor_bssid[1], neighbor_bssid[2], neighbor_bssid[3], neighbor_bssid[4], neighbor_bssid[5], neighbor_device->radio_list[i]->band);
					radio_band = neighbor_device->radio_list[i]->band;
					break;
				}
			}
		}

		if(RADIO_BAND_UNKNOWN == radio_band) {
			log_error("Cannot find radio with mac %02x:%02x:%02x:%02x:%02x:%02x\n", neighbor_bssid[0], neighbor_bssid[1], neighbor_bssid[2], neighbor_bssid[3], neighbor_bssid[4], neighbor_bssid[5]);
			return NULL;
		}
	}

	for (i = 0; i < local_device->number_of_radios; i++) {
		if(local_device->radio_list[i]->band == radio_band)
			return local_device->radio_list[i];
	}

	for (i = 0; i < local_device->number_of_radios; i++) {
		if((radio_band == RADIO_BAND_5GL || radio_band == RADIO_BAND_5GH) && (local_device->radio_list[i]->band == RADIO_BAND_5GL || local_device->radio_list[i]->band == RADIO_BAND_5GH))
		{
			return local_device->radio_list[i];
		}
	}

	return NULL;
}

uint8_t obtain_5g_bss_list(struct Network *network, char *interface_name, int is_agent)
{
	int      i = 0, j = 0, k = 0;
	uint16_t total_bss_number = 0;
	int      ret = 0, band = 0;
	uint16_t tlv_length;
	uint8_t  update_buffer[604];
	uint16_t update_buffer_size = sizeof(update_buffer);

	update_buffer[0] = MAP_SEND_5G_BSS_MACS;

	for (i = 0; i < network->number_of_devices; i++) {
		bool breakloops = false;
		for (j = 0; j < network->device_list[i]->number_of_radios; j++) {
			band = network->device_list[i]->radio_list[j]->band;
			//Agent not classify 5G/2G radio. Radio band always UNKNOW(255)
			if (band == RADIO_BAND_5GH || band == RADIO_BAND_5GL || is_agent) {
				for (k = 0; k < network->device_list[i]->radio_list[j]->number_of_bss; k++) {
					if ((total_bss_number + 1) * 6 + 4 > update_buffer_size) {
						log_warn("[Debug] 5G bss list update list overflow! not update !");
						breakloops = true;
						break;
					}
					memcpy(update_buffer + (4 + total_bss_number * 6), network->device_list[i]->radio_list[j]->bss_list[k]->bssid, 6);
					total_bss_number++;
				}
				if (breakloops) {
					break;
				}
			}
		}
		if (breakloops) {
			break;
		}
	}
	tlv_length = total_bss_number * 6 + 1; // number_of_bss + BSS MACs//
	memcpy(&update_buffer[1], &tlv_length, 2);
	update_buffer[3] = total_bss_number;

	ret = map_set_5g_bss_macs_ioctl(interface_name, update_buffer, update_buffer_size);

	if (ret) {
		log_error("[Debug] map_set_5g_bss_macs_ioctl return err !");
	}

	return ret;
}

uint8_t need_sync_back_channel(struct Radio *radio, struct OperatingChannelReportTLV *operatingChannelReportTlv)
{
	int i = 0;
	if (NULL == radio || NULL == radio->current_operating_classes
	    || NULL == operatingChannelReportTlv || 0 == operatingChannelReportTlv->cur_op_class_nr)
		return 0;

	if (operatingChannelReportTlv->cur_op_class_nr != radio->number_of_curr_op_class)
		return 1;

	for (i = 0; i < operatingChannelReportTlv->cur_op_class_nr; i++) {
		if (operatingChannelReportTlv->operating_channels[i].op_class != radio->current_operating_classes[i].op_class
		    || operatingChannelReportTlv->operating_channels[i].cur_channel != radio->current_operating_classes[i].channel
		    || operatingChannelReportTlv->cur_transmit_power != radio->current_operating_classes[i].tx_power)
			return 1;
	}
	return 0;
}