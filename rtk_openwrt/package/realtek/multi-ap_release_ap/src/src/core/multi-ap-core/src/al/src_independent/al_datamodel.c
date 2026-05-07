/*
 *  Broadband Forum IEEE 1905.1/1a stack
 *
 *  Copyright (c) 2017, Broadband Forum
 *  Copyright (c) 2018, Realtek Semiconductor Corp.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  Subject to the terms and conditions of this license, each copyright
 *  holder and contributor hereby grants to those receiving rights under
 *  this license a perpetual, worldwide, non-exclusive, no-charge,
 *  royalty-free, irrevocable (except for failure to satisfy the
 *  conditions of this license) patent license to make, have made, use,
 *  offer to sell, sell, import, and otherwise transfer this software,
 *  where such license applies only to those patent claims, already
 *  acquired or hereafter acquired, licensable by such copyright holder or
 *  contributor that are necessarily infringed by:
 *
 *  (a) their Contribution(s) (the licensed copyrights of copyright holders
 *      and non-copyrightable additions of contributors, in source or binary
 *      form) alone; or
 *
 *  (b) combination of their Contribution(s) with the work of authorship to
 *      which such Contribution(s) was added by such copyright holder or
 *      contributor, if, at the time the Contribution is added, such addition
 *      causes such combination to be necessarily infringed. The patent
 *      license shall not apply to any other combinations which include the
 *      Contribution.
 *
 *  Except as expressly stated above, no rights or licenses from any
 *  copyright holder or contributor is granted under this license, whether
 *  expressly, by implication, estoppel or otherwise.
 *
 *  DISCLAIMER
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 *  DAMAGE.
 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "platform.h"
#include "utils.h"

#include "al.h"
#include "al_datamodel.h"
#include "al_utils.h"
#include "al_dpp.h"

#include "1905_cmdus.h"
#include "1905_tlvs.h"

#include "platform_interfaces.h"

#include "global_settings.h"

////////////////////////////////////////////////////////////////////////////////
// Private stuff
////////////////////////////////////////////////////////////////////////////////
struct _dataModel {
	INT8U map_whole_network_flag;

	INT8U al_mac_address[6];
	INT8U local_interfaces_nr;

	INT8U is_triband;

	INT8U radio_number;

	INT8U multiap_profile;

	INT8U wfa_test_mode;

	INT8U autoconfig_renew_band;

	INT8U is_controller;
	INT8U listening_lo;
	INT8U configure_state;
	INT8U controller_mac_address[6];

	char **listening_interfaces;
	INT8U  listening_interface_nr;

	char                      controller_ip[16];

	// Should be 48-bit, will do the overflow handling in getter
	INT64U integrity_transmission_counter;

	struct LocalInterface *local_interfaces;

	INT8U network_devices_nr;

	struct networkDevice *network_devices;
	// This list will always contain at least ONE entry,
	// containing the info of the *local* device.

	INT8U send_channel_preference_query;

	INT8U                        controller_channel_reports_nr;
	struct ChannelPreferenceTLV *controller_channel_reports;

	struct SteeringPolicyTLV *steering_policy;

	//--------Channel Scan----------
	INT8U                         channel_scan_result_nr;
	struct ChannelScanResultTLV * channel_scan_results;
	INT8U                         report_independent_channel_scan;
	INT8U *                       result_buffer;
	int                           result_buffer_size;
	struct ChannelScanRequestTLV *buffered_ch_scan_req;
	INT8U                         channel_scan_completed_2g;
	INT8U                         channel_scan_completed_5gh;
	INT8U                         channel_scan_completed_5gl;
	INT8U                         request_stored_results;
	INT8U                         on_boot_scan_flag;
	INT8U *                       on_boot_scan_result_buffer_2g;
	INT8U *                       on_boot_scan_result_buffer_5g;
	//------------------------------

	struct RadioCACStatus cac_radios[3];

	INT32U last_timestamp;
	INT8U  interval_time;
	char * device_name;
	char * ip_addr;
	INT8U  stp_state;

	INT16U neighbor_remove_threshold;

	INT16U primary_vlan_id;
	INT8U  default_pcp;

	struct TrafficSeparationPolicyTLV *traffic_separation_policy;

	INT8U event_flag;

	INT8U                     autoconfig_vendor_tlv_2g_nr;
	struct vendorSpecificTLV *autoconfig_vendor_tlv_2gs;

	INT8U                     autoconfig_vendor_tlv_5gl_nr;
	struct vendorSpecificTLV *autoconfig_vendor_tlv_5gls;

	INT8U                     autoconfig_vendor_tlv_5gh_nr;
	struct vendorSpecificTLV *autoconfig_vendor_tlv_5ghs;

	char * excluded_interfaces;

	INT8U is_common_netlink;

	INT8U backhaul_steering_target_mac[6];

	INT8U byte_counter_units; // 0 : bytes
	                          // 1 : kibibytes (KiB) = 1,024 bytes
	                          // 2 : mebibytes (MiB) = 1,048,576 bytes
	                          // 3 : reserved

	INT8U report_unsuccessful_association;

	INT8U max_bss_per_radio;

	struct map_list dpp_auth_list;
	struct map_list dpp_chirp_list;
	struct dpp_bootstrap_info own_bootstrap_info;
	struct dpp_configurator   configurator;
	struct dpp_eapol_sm       dpp_eapol_state_machine;

	INT8U max_total_nr_service_prioritization_rules;
	INT8U service_prioritization_rule_nr;

	struct ServicePrioritizationRuleTLV *service_prioritization_rule;

	int radio_names_nr;
	char **radio_names;

	INT8U dpp_enrollee_mac[6];

	INT8U security;

	INT8U m1_delay;

	char *csign_key;
	char *netaccess_key;
	char *pp_key;
	char *signed_connector_1905;

} data_model;

// Given a 'mac_address', return a pointer to the "struct _localInterface" that
// represents the local interface with that address.
// Returns NONE if such a local interface could not be found.
//
struct LocalInterface *_macAddressToLocalInterfaceStruct(INT8U *mac_address)
{
	INT8U i;
	INT16U vid                 = 0;
	INT8U  port                = 0;

	if (NULL != mac_address) {
		for (i = 0; i < data_model.local_interfaces_nr; i++) {
			char *iface_name = data_model.local_interfaces[i].name;

#ifdef _RTK_LINUX_
			if (PLATFORM_STRSTR(iface_name, map_vxd_prefix)
				|| PLATFORM_GET_VALID_INTERFACE_NAME(iface_name, VALID_ETH_INTERFACES_IN_BRIDGE)
				|| (controller_interface && PLATFORM_STRSTR(iface_name, controller_interface))) {
				continue;
			}
#endif
			vid             = 0;
			char *base_name = trimInterfaceNameVID(iface_name, &vid, &port);
			PLATFORM_FREE(base_name);
			if (vid) {
				continue;
			}
			if (0 == PLATFORM_MEMCMP(data_model.local_interfaces[i].mac_address, mac_address, 6)) {
				return &data_model.local_interfaces[i];
			}
		}
	}

	// Not found!
	//
	return NULL;
}

// Given a 'name', return a pointer to the "struct LocalInterface" that
// represents the local interface with that interface name.
// Returns NONE if such a local interface could not be found.
//
struct LocalInterface *DMnameToLocalInterfaceStruct(char *name)
{
	INT8U i;
	INT16U vid                 = 0;
	INT8U  port                = 0;

	if (NULL != name) {
		char * base_name = trimInterfaceNameVID(name, &vid, &port);
		for (i = 0; i < data_model.local_interfaces_nr; i++) {
			if (0 == PLATFORM_STRCMP(data_model.local_interfaces[i].name, base_name)) {
				PLATFORM_FREE(base_name);
				return &data_model.local_interfaces[i];
			}
		}
		PLATFORM_FREE(base_name);
	}

	// Not found!
	//
	return NULL;
}

// Given an 'al_mac_address', return a pointer to the "struct _neighbor" that
// represents a 1905 neighbor with that 'al_mac_address' visible from the
// provided 'local_interface_name'.
// Returns NONE if such a neighbor could not be found.
//
struct _neighbor *_alMacAddressToNeighborStruct(char *local_interface_name, INT8U *al_mac_address)
{
	INT8U i;

	struct LocalInterface *x;

	if (NULL == (x = DMnameToLocalInterfaceStruct(local_interface_name))) {
		// Non existent interface
		//
		return NULL;
	}

	if (NULL != al_mac_address) {
		for (i = 0; i < x->neighbors_nr; i++) {
			if (0 == PLATFORM_MEMCMP(x->neighbors[i].al_mac_address, al_mac_address, 6)) {
				return &x->neighbors[i];
			}
		}
	}

	// Not found!
	//
	return NULL;
}

// Given a 'mac_address', return a pointer to the "struct _remoteInterface" that
// represents an interface in the provided 1905 neighbor (identified by its
// 'neighbor_al_mac_address' and visible in 'local_interface) that contains that
// address.
// Returns NONE if such a remote interface could not be found.
//
struct _remoteInterface *_macAddressToRemoteInterfaceStruct(char *local_interface_name, INT8U *neighbor_al_mac_address, INT8U *mac_address)
{
	INT8U i;

	struct _neighbor *x;

	if (NULL == (x = _alMacAddressToNeighborStruct(local_interface_name, neighbor_al_mac_address))) {
		// Non existent neighbor
		//
		return NULL;
	}

	if (NULL != mac_address) {
		for (i = 0; i < x->remote_interfaces_nr; i++) {
			if (0 == PLATFORM_MEMCMP(x->remote_interfaces[i].mac_address, mac_address, 6)) {
				return &x->remote_interfaces[i];
			}
		}
	}

	// Not found!
	//
	return NULL;
}

// When a new 1905 neighbor is discovered on a local interface, this function
// must be called to update the database.
// Returns '0' if there was a problem (out of memory, etc...), '2' if the
// neighbor had already been inserted, '1' if the new neighbor was succesfully
// inserted.
//
INT8U _insertNeighbor(char *local_interface_name, INT8U *al_mac_address)
{
	struct LocalInterface *x;

	// First, make sure the interface exists
	//
	if (NULL == (x = DMnameToLocalInterfaceStruct(local_interface_name))) {
		return 0;
	}

	// Next, make sure this neighbor does not already exist
	//
	if (NULL != _alMacAddressToNeighborStruct(local_interface_name, al_mac_address)) {
		// The neighbor exists! We don't need to do anything special.
		//
		return 2;
	}

	if (0 == x->neighbors_nr) {
		x->neighbors = (struct _neighbor *)PLATFORM_MALLOC(sizeof(struct _neighbor));

	} else {
		x->neighbors = (struct _neighbor *)PLATFORM_REALLOC(x->neighbors, sizeof(struct _neighbor) * (x->neighbors_nr + 1));
	}

	PLATFORM_MEMCPY(x->neighbors[x->neighbors_nr].al_mac_address, al_mac_address, 6);
	x->neighbors[x->neighbors_nr].remote_interfaces_nr = 0;
	x->neighbors[x->neighbors_nr].remote_interfaces    = NULL;

	x->neighbors_nr++;

	return 1;
}

// When a new interface of a 1905 neighbor is discovered, this function must be
// called to update the database.
// Returns '0' if there was a problem (out of memory, etc...), '2' if the
// neighbor interface had already been inserted, '1' if the new neighbor
// interface was successfully inserted.
//
INT8U _insertNeighborInterface(char *local_interface_name, INT8U *neighbor_al_mac_address, INT8U *mac_address)
{
	struct _neighbor *x;

	// First, make sure the interface and neighbor exist
	//
	if (NULL == (x = _alMacAddressToNeighborStruct(local_interface_name, neighbor_al_mac_address))) {
		return 0;
	}

	// Next, make sure this neighbor interface does not already exist
	//
	if (NULL != _macAddressToRemoteInterfaceStruct(local_interface_name, neighbor_al_mac_address, mac_address)) {
		// The neighbor exists! We don't need to do anything special.
		//
		return 2;
	}

	if (0 == x->remote_interfaces_nr) {
		x->remote_interfaces = (struct _remoteInterface *)PLATFORM_MALLOC(sizeof(struct _remoteInterface));
	} else {
		x->remote_interfaces = (struct _remoteInterface *)PLATFORM_REALLOC(x->remote_interfaces, sizeof(struct _remoteInterface) * (x->remote_interfaces_nr + 1));
	}

	PLATFORM_MEMCPY(x->remote_interfaces[x->remote_interfaces_nr].mac_address, mac_address, 6);
	x->remote_interfaces[x->remote_interfaces_nr].last_topology_discovery_ts = 0;
	x->remote_interfaces[x->remote_interfaces_nr].last_bridge_discovery_ts   = 0;

	x->remote_interfaces_nr++;

	return 1;
}

void _setChannelPreference(struct ChannelPreference *c, INT8U channel_nr, INT8U channel, INT8U preference)
{
	int i;
	for (i = 0; i < channel_nr; i++) {
		if (c[i].channel == channel) {
			c[i].preference = preference;
			PLATFORM_PRINTF_DETAIL("[MULTI_AP] channel %d preference is set to: %d\n", channel, preference);
			return;
		}
	}
	PLATFORM_PRINTF_DEBUG("[MULTI_AP] Cannot set preference for non-operable channel %d, skip\n", channel);
}

INT8U _getChannelPreference(struct ChannelPreference *c, INT8U channel_nr, INT8U channel)
{
	int i;
	for (i = 0; i < channel_nr; i++) {
		if (c[i].channel == channel) {
			return c[i].preference;
		}
	}
	PLATFORM_PRINTF_ERROR("[MULTI_AP] Cannot retrieve preference for non-operable channel %d\n", channel);
	return 0;
}

INT8U _findBestChannel(struct ChannelPreference *c, INT8U channel_nr)
{
	int   i;
	INT8U max          = 0;
	INT8U best_channel = 0;
	for (i = 0; i < channel_nr; i++) {
		if (c[i].preference == 15) {
			return c[i].channel;
		} else if (c[i].preference > max) {
			max          = c[i].preference;
			best_channel = c[i].channel;
		}
	}
	return best_channel;
}

////////////////////////////////////////////////////////////////////////////////
// API functions (only available to the 1905 core itself, ie. files inside the
// 'lib1905' folder)
////////////////////////////////////////////////////////////////////////////////

void DMinit()
{
	// To avoid undefined initial value of newly added entries.
	PLATFORM_MEMSET(&data_model, 0, sizeof(data_model));

	data_model.map_whole_network_flag = 1;

	data_model.is_triband = 1;

	data_model.radio_number = 3;

	if (0 == PLATFORM_STRCMP(map_radio_name_5gh, map_radio_name_5gl)) {
		data_model.is_triband   = 0;
		data_model.radio_number = 2;
	}

	data_model.wfa_test_mode = 0;

	data_model.autoconfig_renew_band = 1;

	data_model.is_controller = 0;

	data_model.listening_lo = 0;

	data_model.configure_state = 0;

	data_model.multiap_profile = 1;

	data_model.controller_mac_address[0] = 0x00;
	data_model.controller_mac_address[1] = 0x00;
	data_model.controller_mac_address[2] = 0x00;
	data_model.controller_mac_address[3] = 0x00;
	data_model.controller_mac_address[4] = 0x00;
	data_model.controller_mac_address[5] = 0x00;

	data_model.al_mac_address[0] = 0x00;
	data_model.al_mac_address[1] = 0x00;
	data_model.al_mac_address[2] = 0x00;
	data_model.al_mac_address[3] = 0x00;
	data_model.al_mac_address[4] = 0x00;
	data_model.al_mac_address[5] = 0x00;

	data_model.local_interfaces_nr = 0;
	data_model.local_interfaces    = NULL;

	data_model.listening_interface_nr = 0;
	data_model.listening_interfaces   = NULL;

	// Regarding the "network_devices" list, we will init it with one element,
	// representing the local node
	//
	data_model.network_devices_nr = 1;
	data_model.network_devices    = (struct networkDevice *)PLATFORM_MALLOC(sizeof(struct networkDevice));

	data_model.network_devices[0].update_timestamp    = PLATFORM_GET_TIMESTAMP();
	data_model.network_devices[0].topology_timestamp    = PLATFORM_GET_TIMESTAMP();
	data_model.network_devices[0].discovery_timestamp = PLATFORM_GET_TIMESTAMP();
	data_model.network_devices[0].info                = NULL;
	PLATFORM_MEMSET(data_model.network_devices[0].receiving_interface, '\0', sizeof(data_model.network_devices[0].receiving_interface));
	data_model.network_devices[0].receiving_interface[0]    = 'l';
	data_model.network_devices[0].receiving_interface[1]    = 'o';
	data_model.network_devices[0].bridges_nr                = 0;
	data_model.network_devices[0].bridges                   = NULL;
	data_model.network_devices[0].non1905_neighbors_nr      = 0;
	data_model.network_devices[0].non1905_neighbors         = NULL;
	data_model.network_devices[0].x1905_neighbors_nr        = 0;
	data_model.network_devices[0].x1905_neighbors           = NULL;
	data_model.network_devices[0].metrics_with_neighbors_nr = 0;
	data_model.network_devices[0].metrics_with_neighbors    = NULL;
	data_model.network_devices[0].extensions                = NULL;
	data_model.network_devices[0].extensions_nr             = 0;

	data_model.network_devices[0].ap_metrics_nr = 0;
	data_model.network_devices[0].ap_metrics    = NULL;

	data_model.network_devices[0].supported_services = NULL;
	data_model.network_devices[0].operational_bss    = NULL;
	data_model.network_devices[0].associated_clients = NULL;

	data_model.network_devices[0].channel_scan_capabilities = NULL;
	data_model.network_devices[0].multiap_profile           = 0;
	data_model.network_devices[0].byte_counter_units        = 0;
	PLATFORM_MEMSET(&data_model.network_devices[0].multiap_version, 0, sizeof(MultiapVersion));

	data_model.network_devices[0].integrity_transmission_counter = 0;

	data_model.network_devices[0].security = SECURITY_NOT_ENABLED;

	data_model.send_channel_preference_query = 0;

	data_model.controller_channel_reports_nr = 0;
	data_model.controller_channel_reports    = NULL;

	data_model.steering_policy = NULL;

	data_model.channel_scan_result_nr          = 0;
	data_model.channel_scan_results            = NULL;
	data_model.report_independent_channel_scan = 0;
	data_model.buffered_ch_scan_req            = NULL;
	data_model.request_stored_results          = 0;
	data_model.on_boot_scan_flag               = 0;
	data_model.channel_scan_completed_2g       = 0;
	data_model.channel_scan_completed_5gh      = 0;
	data_model.channel_scan_completed_5gl      = 0;
	data_model.result_buffer                   = NULL;
	data_model.on_boot_scan_result_buffer_2g   = NULL;
	data_model.on_boot_scan_result_buffer_5g   = NULL;

	data_model.interval_time  = 0;
	data_model.last_timestamp = PLATFORM_GET_TIMESTAMP();

	data_model.stp_state = 0;

	data_model.neighbor_remove_threshold = GC_MAX_AGE;

	PLATFORM_MEMSET(data_model.controller_ip, '\0', 16);

	data_model.autoconfig_vendor_tlv_2g_nr = 0;
	data_model.autoconfig_vendor_tlv_2gs   = NULL;
	data_model.autoconfig_vendor_tlv_5gl_nr = 0;
	data_model.autoconfig_vendor_tlv_5gls   = NULL;
	data_model.autoconfig_vendor_tlv_5gh_nr = 0;
	data_model.autoconfig_vendor_tlv_5ghs   = NULL;

	data_model.network_devices[0].last_ch_prefs             = NULL;
	data_model.network_devices[0].last_ch_prefs_nr           = 0;

#ifdef RTK_MULTI_AP_R2
	int i                             = 0;
	int             op_classes_len;
	OP_CLASS *op_classes = PLATFORM_GET_OP_CLASS(&op_classes_len);

	if (NULL == op_classes) {
		PLATFORM_PRINTF_ERROR("[RTK] No operating class is available\n");
		return;
	}

	for (i = 0; i < data_model.radio_number; i++) {
		char *radio_interface = map_radio_name_5gh;
		if(1 == i) {
			radio_interface = map_radio_name_5gl;
		}
		PLATFORM_GET_MAC(radio_interface, (data_model.cac_radios[i].radio_identifier));
		data_model.cac_radios[i].cac_record_nr = 0;
		data_model.cac_radios[i].cac_records   = NULL;
		int             j, k;
		for (j = 0; j < op_classes_len; j++) {
			int append_cac_record = 0;

			if(_5G != op_classes[j].band) {
				continue;
			}

			if (DMisTribandDevice()) {
				append_cac_record |= (_5GH == op_classes[j].sub_band && (0 == PLATFORM_STRCMP(radio_interface, map_radio_name_5gh)));
				append_cac_record |= (_5GL == op_classes[j].sub_band && (0 == PLATFORM_STRCMP(radio_interface, map_radio_name_5gl)));
			}
		    else {
				append_cac_record |= (0 == PLATFORM_STRCMP(radio_interface, map_radio_name_5gl));
			}

			if (append_cac_record) {
				data_model.cac_radios[i].cac_record_nr += op_classes[j].channel_array[0];
				data_model.cac_radios[i].cac_records = (struct CACStatusRecord *)PLATFORM_REALLOC(data_model.cac_radios[i].cac_records, sizeof(struct CACStatusRecord) * (data_model.cac_radios[i].cac_record_nr));
				for (k = 0; k < op_classes[j].channel_array[0]; k++) {
					int channel_idx = data_model.cac_radios[i].cac_record_nr - op_classes[j].channel_array[0] + k;
					PLATFORM_PRINTF_DETAIL("%d Filling CAC for channel %d op class %d\n", channel_idx, data_model.cac_radios[i].cac_records[channel_idx].channel, data_model.cac_radios[i].cac_records[channel_idx].op_class);
					data_model.cac_radios[i].cac_records[channel_idx].op_class       = op_classes[j].op_class;
					data_model.cac_radios[i].cac_records[channel_idx].channel        = op_classes[j].channel_array[k + 1];
					data_model.cac_radios[i].cac_records[channel_idx].cac_status     = CAC_COMPLETED;
					data_model.cac_radios[i].cac_records[channel_idx].last_timestamp = 0;
				}
			}
		}

		PLATFORM_PRINTF_DEBUG("Radio %s mac %02x:%02x:%02x:%02x:%02x:%02x\n", radio_interface, data_model.cac_radios[i].radio_identifier[0], data_model.cac_radios[i].radio_identifier[1], data_model.cac_radios[i].radio_identifier[2], data_model.cac_radios[i].radio_identifier[3], data_model.cac_radios[i].radio_identifier[4], data_model.cac_radios[i].radio_identifier[5]);
	}

	PLATFORM_FREE(op_classes);
#endif

	data_model.integrity_transmission_counter = 0;

	data_model.security = SECURITY_NOT_ENABLED;

	data_model.m1_delay = 0;

	data_model.primary_vlan_id = 0;

	data_model.default_pcp = 0;

	data_model.traffic_separation_policy = NULL;

	data_model.event_flag = 0;

	data_model.excluded_interfaces = NULL;

	data_model.backhaul_steering_target_mac[0] = 0x00;
	data_model.backhaul_steering_target_mac[1] = 0x00;
	data_model.backhaul_steering_target_mac[2] = 0x00;
	data_model.backhaul_steering_target_mac[3] = 0x00;
	data_model.backhaul_steering_target_mac[4] = 0x00;
	data_model.backhaul_steering_target_mac[5] = 0x00;

	// default unit is bytes
	data_model.byte_counter_units = 0;

	data_model.report_unsuccessful_association = 0;

	data_model.max_bss_per_radio = 5;

	// Hard coded just like obtain R2 capability tlv did.
	DMsetMaxServicePrioritizationRules(1);

	data_model.service_prioritization_rule_nr = 0;

	// int radio_names_nr;
	// char **radio_names;

	if (DMisTribandDevice()) {
		data_model.radio_names_nr = 3;
	} else {
		data_model.radio_names_nr = 2;
	}

	data_model.radio_names = (char **)PLATFORM_MALLOC(data_model.radio_names_nr * sizeof(char *));
	data_model.radio_names[0] = map_radio_name_24g;
	data_model.radio_names[1] = map_radio_name_5gl;
	if (DMisTribandDevice()) {
		data_model.radio_names[2] = map_radio_name_5gh;
	}

	return;
}

void DMalMacSet(INT8U *al_mac_address)
{
	PLATFORM_MEMCPY(data_model.al_mac_address, al_mac_address, 6);
	return;
}

INT8U *DMalMacGet()
{
	return data_model.al_mac_address;
}

INT8U DMisTribandDevice()
{
	return data_model.is_triband;
}

INT8U DMisController()
{
	return data_model.is_controller;
}

void DMcontrollerInit(INT8U *controller_mac_address)
{
	data_model.is_controller = 1;
	data_model.listening_lo  = 1;
	PLATFORM_MEMCPY(data_model.controller_mac_address, controller_mac_address, 6);
}

void DMdppInit()
{
	INT8U reuse_key = 0;
	struct dpp_bootstrap_info *own_bi = &data_model.own_bootstrap_info;

	FILE *fd = fopen(DPP_BOOTSTRAP_OWN_PRIV_FILENAME, "r+e");

	list_init(&data_model.dpp_auth_list);
	list_init(&data_model.dpp_chirp_list);

	if (NULL != fd) {
		char content[DPP_BOOTSTRAP_INFO_MAX_LEN];
		char *                     privkey = NULL;
		size_t                     privkey_len;

		memset(content, 0, sizeof(content));
		fgets(content, sizeof(content), fd);
		content[strcspn(content, "\r\n")] = 0;
		fclose(fd);
		privkey_len = strlen(content) / 2;
		privkey     = PLATFORM_MALLOC(privkey_len);
		if (privkey_len > 0
		&& hexstr2bin(content, privkey, privkey_len) >= 0
		&& dpp_keygen(own_bi, NULL, (INT8U *)privkey, privkey_len) == 1) {
			PLATFORM_PRINTF_INFO("[PLATFORM] Reuse generated bootstrapping key...\n");
			reuse_key = 1;
		}
		PLATFORM_FREE(privkey);
	}

	if (!reuse_key) {
		// Generate DPP Bootstrapping Key
		dpp_keygen(own_bi, "prime256v1", NULL, 0);
	}

	// Add an new auth to store information
	// For agent, there shall only be one auth, this auth will be reused and stores all the information
	// For controller, it shall have one auth per agent, this is just a dummy auth and is used to store own signed configuration objects
	DMaddDPPAuth(own_bi->pubkey_hash_chirp, SHA256_MAC_LEN);

	if (DMisController()) {
		// Init DPP Configurator (Controller is always Configurator in MAP)
		INT8U *csign_hex = NULL, *ppk_hex = NULL;
		INT16U csign_hex_len = 0, ppk_hex_len = 0;

		csign_hex = DMgetDPPCsignFromFile(&csign_hex_len);
		ppk_hex   = DMgetDPPPpkFromFile(&ppk_hex_len);

		dpp_keygen_configurator(&data_model.configurator, "prime256v1", csign_hex, csign_hex_len, ppk_hex, ppk_hex_len);
		PLATFORM_FREE(csign_hex);
		PLATFORM_FREE(ppk_hex);
		struct dpp_authentication *dpp_auth = DMgetDPPAuthenticationObject();
		dpp_configurator_sign(dpp_auth, "map", "mapNW", DPP_NETROLE_CONFIGURATOR);
		dpp_configurator_sign(dpp_auth, "map", "mapNW", DPP_NETROLE_MAP_CONTROLLER);
	} else {
		struct dpp_authentication *dpp_auth = DMgetDPPAuthenticationObject();
		dpp_load_creds_info(dpp_auth, data_model.csign_key, data_model.pp_key, data_model.netaccess_key, data_model.signed_connector_1905);
	}

	PLATFORM_MEMSET(&data_model.dpp_eapol_state_machine, 0, sizeof(data_model.dpp_eapol_state_machine));
}

void DMcontrollerMacSet(INT8U *controller_mac_address)
{
	PLATFORM_MEMCPY(data_model.controller_mac_address, controller_mac_address, 6);
	INT8U null_addr[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	if (0 == PLATFORM_MEMCMP(null_addr, controller_mac_address, 6)) {
		return;
	}

	struct networkDevice *controller = DMnetworkDeviceGet(controller_mac_address);
	if (controller != NULL) {
		controller->update_timestamp = PLATFORM_GET_TIMESTAMP();
	}
}

INT8U *DMcontrollerMacGet()
{
	return data_model.controller_mac_address;
}

void DMmapWholeNetworkSet(INT8U map_whole_network_flag)
{
	data_model.map_whole_network_flag = map_whole_network_flag;

	return;
}

INT8U DMmapWholeNetworkGet()
{
	return data_model.map_whole_network_flag;
}

void DMchannelQueryStatusSet(INT8U status)
{
	data_model.send_channel_preference_query = status;

	return;
}

INT8U DMchannelQueryStatusGet()
{
	return data_model.send_channel_preference_query;
}

INT8U DMinsertInterface(char *name, INT8U *mac_address, INT8U role)
{
	struct LocalInterface *x;

	// First, make sure this interface does not already exist
	//
	if (NULL != (x = DMnameToLocalInterfaceStruct(name))) {
		// The interface exists!
		//
		// Even if it already exists, if the provided 'mac_address' and the
		// already existing entry match, do not return an error.
		//
		if (0 == PLATFORM_MEMCMP(x->mac_address, mac_address, 6)) {
			// Ok
			//
			return 1;
		} else {
			// Interface exists and its MAC address is different from the
			// provided one. Maybe the caller should first "remove" this
			// interface and then try to add the new one.
			//
			return 0;
		}
	}

	if (0 == data_model.local_interfaces_nr) {
		data_model.local_interfaces = (struct LocalInterface *)PLATFORM_MALLOC(sizeof(struct LocalInterface));

	} else {
		data_model.local_interfaces = (struct LocalInterface *)PLATFORM_REALLOC(data_model.local_interfaces, sizeof(struct LocalInterface) * (data_model.local_interfaces_nr + 1));
	}

	data_model.local_interfaces[data_model.local_interfaces_nr].name = PLATFORM_STRDUP(name);
	PLATFORM_MEMCPY(data_model.local_interfaces[data_model.local_interfaces_nr].mac_address, mac_address, 6);
	data_model.local_interfaces[data_model.local_interfaces_nr].role                    = role;
	data_model.local_interfaces[data_model.local_interfaces_nr].neighbors               = NULL;
	data_model.local_interfaces[data_model.local_interfaces_nr].neighbors_nr            = 0;
	data_model.local_interfaces[data_model.local_interfaces_nr].metric_inclusion_policy = 0;

	data_model.local_interfaces_nr++;

	return 1;
}

char *DMmacToInterfaceName(INT8U *mac_address)
{
	struct LocalInterface *x;

	x = _macAddressToLocalInterfaceStruct(mac_address);

	if (NULL != x) {
		return x->name;
	} else {
		// Not found!
		//
		return NULL;
	}
}

INT8U *DMinterfaceNameToMac(char *interface_name)
{
	INT8U i;

	if (NULL != interface_name) {
		if (PLATFORM_STRCMP(interface_name, "lo") == 0) {
			return data_model.al_mac_address;
		}
		char *base_interface_name = trimInterfaceNameVID(interface_name, NULL, NULL);
		for (i = 0; i < data_model.local_interfaces_nr; i++) {
			if (0 == PLATFORM_STRCMP(data_model.local_interfaces[i].name, base_interface_name)) {
				PLATFORM_FREE(base_interface_name);
				return data_model.local_interfaces[i].mac_address;
			}
		}

		for (i = 0; i < data_model.local_interfaces_nr; i++) {
			if (0 == PLATFORM_MEMCMP(data_model.local_interfaces[i].name, base_interface_name, PLATFORM_STRLEN(base_interface_name))) {
				PLATFORM_FREE(base_interface_name);
				return data_model.local_interfaces[i].mac_address;
			}
		}

		PLATFORM_FREE(base_interface_name);
	}

	// Not found!
	//
	return NULL;
}

INT8U (*DMgetListOfInterfaceNeighbors(char *local_interface_name, INT8U *al_mac_addresses_nr))[6]
{
	INT8U i;
	INT8U(*ret)[6];

	struct LocalInterface *x;

	if (NULL == (x = DMnameToLocalInterfaceStruct(local_interface_name))) {
		// Non existent interface
		//
		*al_mac_addresses_nr = 0;
		return NULL;
	}

	if (0 == x->neighbors_nr) {
		*al_mac_addresses_nr = 0;
		return NULL;
	}

	ret = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(INT8U[6]) * x->neighbors_nr);

	for (i = 0; i < x->neighbors_nr; i++) {
		ret[i][0] = x->neighbors[i].al_mac_address[0];
		ret[i][1] = x->neighbors[i].al_mac_address[1];
		ret[i][2] = x->neighbors[i].al_mac_address[2];
		ret[i][3] = x->neighbors[i].al_mac_address[3];
		ret[i][4] = x->neighbors[i].al_mac_address[4];
		ret[i][5] = x->neighbors[i].al_mac_address[5];
	}

	*al_mac_addresses_nr = x->neighbors_nr;
	return ret;
}

INT8U (*DMgetListOfNeighbors(INT8U *al_mac_addresses_nr))[6]
{
	INT8U i, j, k;

	INT8U total;
	INT8U(*ret)[6];

	if (NULL == al_mac_addresses_nr) {
		return NULL;
	}

	total = 0;
	ret   = NULL;

	for (i = 0; i < data_model.local_interfaces_nr; i++) {
		for (j = 0; j < data_model.local_interfaces[i].neighbors_nr; j++) {
			// Check for duplicates
			//
			INT8U already_present;

			already_present = 0;
			for (k = 0; k < total; k++) {
				if (0 == PLATFORM_MEMCMP(&ret[k], data_model.local_interfaces[i].neighbors[j].al_mac_address, 6)) {
					already_present = 1;
					break;
				}
			}

			if (1 == already_present) {
				continue;
			}

			// If we get here, this is a new neighbor and we need to add it to
			// the list
			//
			if (NULL == ret) {
				ret = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(INT8U[6]));
			} else {
				ret = (INT8U(*)[6])PLATFORM_REALLOC(ret, sizeof(INT8U[6]) * (total + 1));
			}
			PLATFORM_MEMCPY(&ret[total], data_model.local_interfaces[i].neighbors[j].al_mac_address, 6);

			total++;
		}
	}

	*al_mac_addresses_nr = total;

	return ret;
}

INT8U (*DMgetListOfLinksWithNeighbor(INT8U *neighbor_al_mac_address, char ***interfaces, INT8U *links_nr))
[6]
{
	INT8U i, j, k;
	INT8U total;

	INT8U(*ret)[6];
	char **intfs;

	total = 0;
	ret   = NULL;
	intfs = NULL;

	for (i = 0; i < data_model.local_interfaces_nr; i++) {
		for (j = 0; j < data_model.local_interfaces[i].neighbors_nr; j++) {
			// Filter neighbor (we are just interested in
			// 'neighbor_al_mac_address')
			//
			if (0 != PLATFORM_MEMCMP(neighbor_al_mac_address, data_model.local_interfaces[i].neighbors[j].al_mac_address, 6)) {
				continue;
			}

			for (k = 0; k < data_model.local_interfaces[i].neighbors[j].remote_interfaces_nr; k++) {
				// This is a new link between the local AL and the remote AL.
				// Add it.
				//
				if (NULL == ret) {
					ret   = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(INT8U[6]));
					intfs = (char **)PLATFORM_MALLOC(sizeof(char *));
				} else {
					ret   = (INT8U(*)[6])PLATFORM_REALLOC(ret, sizeof(INT8U[6]) * (total + 1));
					intfs = (char **)PLATFORM_REALLOC(intfs, sizeof(char *) * (total + 1));
				}
				PLATFORM_MEMCPY(&ret[total], data_model.local_interfaces[i].neighbors[j].remote_interfaces[k].mac_address, 6);
				intfs[total] = data_model.local_interfaces[i].name;

				total++;
			}
		}
	}

	*links_nr   = total;
	*interfaces = intfs;

	return ret;
}

void DMfreeListOfLinksWithNeighbor(INT8U (*p)[6], char **interfaces, INT8U links_nr)
{
	if (0 == links_nr) {
		return;
	}

	if (NULL != interfaces) {
		PLATFORM_FREE(interfaces);
	}

	if (NULL != p) {
		PLATFORM_FREE(p);
	}

	return;
}

INT8U DMupdateDiscoveryTimeStamps(char *receiving_interface_name, INT8U *al_mac_address, INT8U *mac_address, INT8U timestamp_type, INT32U *ellapsed)
{

	struct _remoteInterface *x;

	INT32U aux1, aux2;
	INT8U  insert_result;
	INT8U  ret;

	ret = 2;

	if (!(receiving_interface_name && al_mac_address && mac_address)) {
		PLATFORM_PRINTF_ERROR("Invalid 'receiving_interface_name or al_mac_address or mac_address !'\n");
		return 0;
	}

	if (0 == (insert_result = _insertNeighbor(receiving_interface_name, al_mac_address)) || 0 == _insertNeighborInterface(receiving_interface_name, al_mac_address, mac_address)) {
		PLATFORM_PRINTF_ERROR("Could not create new entries in the database\n");
		return 0;
	}

	if (1 == insert_result) {
		// This is the first time we know of this neighbor (ie. the neighbor
		// has been inserted in the data model for the first time)
		//
		ret = 1;
	}

	x = _macAddressToRemoteInterfaceStruct(receiving_interface_name, al_mac_address, mac_address);

	PLATFORM_PRINTF_DETAIL("New discovery timestamp udpate:\n");
	PLATFORM_PRINTF_DETAIL("  - local_interface      : %s\n", receiving_interface_name);
	PLATFORM_PRINTF_DETAIL("  - 1905 neighbor AL MAC : %02x:%02x:%02x:%02x:%02x:%02x:\n", al_mac_address[0], al_mac_address[1], al_mac_address[2], al_mac_address[3], al_mac_address[4], al_mac_address[5]);
	PLATFORM_PRINTF_DETAIL("  - remote interface MAC : %02x:%02x:%02x:%02x:%02x:%02x:\n", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);

	aux1 = x->last_topology_discovery_ts;
	aux2 = x->last_bridge_discovery_ts;

	switch (timestamp_type) {
	case TIMESTAMP_TOPOLOGY_DISCOVERY: {
		INT32U aux;

		aux = PLATFORM_GET_TIMESTAMP();

		if (NULL != ellapsed) {
			if (2 == ret) {
				*ellapsed = aux - x->last_topology_discovery_ts;
			} else {
				*ellapsed = 0;
			}
		}

		x->last_topology_discovery_ts = aux;
		break;
	}
	case TIMESTAMP_BRIDGE_DISCOVERY: {
		INT32U aux;

		aux = PLATFORM_GET_TIMESTAMP();

		if (NULL != ellapsed) {
			if (2 == ret) {
				*ellapsed = aux - x->last_bridge_discovery_ts;
			} else {
				*ellapsed = 0;
			}
		}
		x->last_bridge_discovery_ts = aux;
		break;
	}
	default: {
		PLATFORM_PRINTF_ERROR("Unknown 'timestamp_type' (%d)\n", timestamp_type);

		return 0;
	}
	}

	PLATFORM_PRINTF_DETAIL("  - topology disc TS     : %d --> %d\n", aux1, x->last_topology_discovery_ts);
	PLATFORM_PRINTF_DETAIL("  - bridge   disc TS     : %d --> %d\n", aux2, x->last_bridge_discovery_ts);

	return ret;
}

INT8U DMisLinkBridged(char *local_interface_name, INT8U *neighbor_al_mac_address, INT8U *neighbor_mac_address)
{
	struct _remoteInterface *x;

	INT32U aux;

	if (NULL == (x = _macAddressToRemoteInterfaceStruct(local_interface_name, neighbor_al_mac_address, neighbor_mac_address))) {
		// Non existent neighbor
		//
		return 2;
	}

	if (x->last_topology_discovery_ts > x->last_bridge_discovery_ts) {
		aux = x->last_topology_discovery_ts - x->last_bridge_discovery_ts;
	} else {
		aux = x->last_bridge_discovery_ts - x->last_topology_discovery_ts;
	}

	if (aux < DISCOVERY_THRESHOLD_MS) {
		// Links is *not* bridged
		//
		return 0;
	} else {
		// Link is bridged
		//
		return 1;
	}
}

INT8U DMisNeighborBridged(char *local_interface_name, INT8U *neighbor_al_mac_address)
{
	struct _neighbor *x;

	INT8U i;

	if (NULL == (x = _alMacAddressToNeighborStruct(local_interface_name, neighbor_al_mac_address))) {
		// Non existent neighbor
		//
		return 2;
	}

	for (i = 0; i < x->remote_interfaces_nr; i++) {
		if (1 == DMisLinkBridged(local_interface_name, neighbor_al_mac_address, x->remote_interfaces[i].mac_address)) {
			// If at least one link is bridged, then this neighbor is
			// considered to be bridged.
			//
			return 1;
		}
	}

	// All links are not bridged, thus the neighbor is considered to be not
	// bridged also.
	//
	return 0;
}

INT8U DMisInterfaceBridged(char *local_interface_name)
{
	struct LocalInterface *x;

	INT8U i;

	x = DMnameToLocalInterfaceStruct(local_interface_name);
	if (NULL == x) {
		PLATFORM_PRINTF_ERROR("Invalid local interface name\n");
		return 2;
	}

	for (i = 0; i < x->neighbors_nr; i++) {
		if (1 == DMisNeighborBridged(local_interface_name, x->neighbors[i].al_mac_address)) {
			// If at least one neighbor is bridged, then this interface is
			// considered to be bridged.
			//
			return 1;
		}
	}

	// All neighbors are not bridged, thus the interface is considered to be not
	// bridged also.
	//
	return 0;
}

INT8U DMisExistingNeighbor(char *local_interface_name, INT8U *neighbor_al_mac_address)
{
	if (_alMacAddressToNeighborStruct(local_interface_name, neighbor_al_mac_address)) {
		return 1;
	}

	return 0;
}

INT8U *DMmacToAlMac(INT8U *mac_address)
{
	INT8U i, j, k;

	INT8U *al_mac;
	al_mac = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * 6);

	if (0 == PLATFORM_MEMCMP(data_model.al_mac_address, mac_address, 6)) {
		PLATFORM_MEMCPY(al_mac, data_model.al_mac_address, 6);
		return al_mac;
	}
	for (i = 0; i < data_model.local_interfaces_nr; i++) {
		if (0 == PLATFORM_MEMCMP(data_model.local_interfaces[i].mac_address, mac_address, 6)) {
			PLATFORM_MEMCPY(al_mac, data_model.al_mac_address, 6);
			return al_mac;
		}

		for (j = 0; j < data_model.local_interfaces[i].neighbors_nr; j++) {
			if (0 == PLATFORM_MEMCMP(data_model.local_interfaces[i].neighbors[j].al_mac_address, mac_address, 6)) {
				PLATFORM_MEMCPY(al_mac, data_model.local_interfaces[i].neighbors[j].al_mac_address, 6);
				return al_mac;
			}

			for (k = 0; k < data_model.local_interfaces[i].neighbors[j].remote_interfaces_nr; k++) {
				if (0 == PLATFORM_MEMCMP(data_model.local_interfaces[i].neighbors[j].remote_interfaces[k].mac_address, mac_address, 6)) {
					PLATFORM_MEMCPY(al_mac, data_model.local_interfaces[i].neighbors[j].al_mac_address, 6);
					return al_mac;
				}
			}
		}
	}

	for (i = 0; i < data_model.network_devices_nr; i++) {
		if (data_model.network_devices[i].info) {
			if (0 == PLATFORM_MEMCMP(data_model.network_devices[i].info->al_mac_address, mac_address, 6)) {
				PLATFORM_MEMCPY(al_mac, data_model.network_devices[i].info->al_mac_address, 6);
				return al_mac;
			}

			if (data_model.network_devices[i].operational_bss) {
				for (j = 0; j < data_model.network_devices[i].operational_bss->radios_nr; j++) {
					if (0 == PLATFORM_MEMCMP(data_model.network_devices[i].operational_bss->radios[j].radio_unique_id, mac_address, 6)) {
						PLATFORM_MEMCPY(al_mac, data_model.network_devices[i].info->al_mac_address, 6);
						return al_mac;
					}

					for (k = 0; k < data_model.network_devices[i].operational_bss->radios[j].BSSs_nr; k++) {
						if (0 == PLATFORM_MEMCMP(data_model.network_devices[i].operational_bss->radios[j].BSSs[k].mac_addr, mac_address, 6)) {
							PLATFORM_MEMCPY(al_mac, data_model.network_devices[i].info->al_mac_address, 6);
							return al_mac;
						}
					}
				}
			}
		}
	}

	PLATFORM_FREE(al_mac);
	return NULL;
}

INT8U DMaddNetworkDevice(INT8U *al_mac_address, char *receiving_interface)
{
	INT8U i;

	if (NULL == al_mac_address) {
		return 0;
	}
	// First, search for an existing entry with the same AL MAC address
	// Remember that the first entry holds a reference to the *local* node.
	//
	if (0 == PLATFORM_MEMCMP(DMalMacGet(), al_mac_address, 6)) {
		return 0;
	}

	for (i = 1; i < data_model.network_devices_nr; i++) {
		if (NULL != data_model.network_devices[i].info) {
			if (0 == PLATFORM_MEMCMP(data_model.network_devices[i].info->al_mac_address, al_mac_address, 6)) {
				return 0;
			}
		}
	}

	// A matching entry was *not* found. Create a new one, but only if this
	// new information contains the "info" TLV (otherwise don't do anything
	// and wait for the "info" TLV to be received in the future)
	//
	if (0 == data_model.network_devices_nr) {
		data_model.network_devices = (struct networkDevice *)PLATFORM_MALLOC(sizeof(struct networkDevice));
	} else {
		data_model.network_devices = (struct networkDevice *)PLATFORM_REALLOC(data_model.network_devices, sizeof(struct networkDevice) * (data_model.network_devices_nr + 1));
	}

	PLATFORM_MEMSET(&data_model.network_devices[data_model.network_devices_nr], 0, sizeof(struct networkDevice));

	data_model.network_devices[data_model.network_devices_nr].update_timestamp = PLATFORM_GET_TIMESTAMP();
	data_model.network_devices[data_model.network_devices_nr].topology_timestamp = 0;
	data_model.network_devices[data_model.network_devices_nr].discovery_timestamp = 0;
	strlcpy(data_model.network_devices[data_model.network_devices_nr].receiving_interface, receiving_interface, sizeof(data_model.network_devices[data_model.network_devices_nr].receiving_interface));

	data_model.network_devices[data_model.network_devices_nr].info = (struct deviceInformationTypeTLV *)PLATFORM_MALLOC(sizeof(struct deviceInformationTypeTLV));
	PLATFORM_MEMCPY(data_model.network_devices[data_model.network_devices_nr].info->al_mac_address, al_mac_address, 6);
	data_model.network_devices[data_model.network_devices_nr].info->local_interfaces    = NULL;
	data_model.network_devices[data_model.network_devices_nr].info->local_interfaces_nr = 0;
	data_model.network_devices[data_model.network_devices_nr].info->tlv_type            = TLV_TYPE_DEVICE_INFORMATION_TYPE;

	data_model.network_devices[data_model.network_devices_nr].bridges_nr           = 0;
	data_model.network_devices[data_model.network_devices_nr].bridges              = NULL;
	data_model.network_devices[data_model.network_devices_nr].non1905_neighbors_nr = 0;
	data_model.network_devices[data_model.network_devices_nr].non1905_neighbors    = NULL;
	data_model.network_devices[data_model.network_devices_nr].x1905_neighbors_nr   = 0;
	data_model.network_devices[data_model.network_devices_nr].x1905_neighbors      = NULL;

	data_model.network_devices[data_model.network_devices_nr].supported_services = NULL;
	data_model.network_devices[data_model.network_devices_nr].operational_bss    = NULL;
	data_model.network_devices[data_model.network_devices_nr].associated_clients = NULL;

	data_model.network_devices[data_model.network_devices_nr].metrics_with_neighbors_nr = 0;
	data_model.network_devices[data_model.network_devices_nr].metrics_with_neighbors    = NULL;

	data_model.network_devices[data_model.network_devices_nr].extensions    = NULL;
	data_model.network_devices[data_model.network_devices_nr].extensions_nr = 0;

	data_model.network_devices[data_model.network_devices_nr].last_ch_prefs = NULL;
	data_model.network_devices[data_model.network_devices_nr].last_ch_prefs_nr = 0;

	data_model.network_devices[data_model.network_devices_nr].ap_metrics_nr = 0;

	data_model.network_devices[data_model.network_devices_nr].multiap_profile           = 0;
	PLATFORM_MEMSET(&data_model.network_devices[data_model.network_devices_nr].multiap_version, 0, sizeof(MultiapVersion));

	data_model.network_devices[data_model.network_devices_nr].integrity_transmission_counter = 0;

	data_model.network_devices[data_model.network_devices_nr].security = SECURITY_NOT_ENABLED;

	data_model.network_devices_nr++;

	return 1;
}

INT8U DMupdateNetworkDeviceInfo(INT8U *al_mac_address, char *receiving_interface,
                                INT8U in_update, struct deviceInformationTypeTLV *info,
                                INT8U br_update, struct deviceBridgingCapabilityTLV **bridges, INT8U bridges_nr,
                                INT8U no_update, struct non1905NeighborDeviceListTLV **non1905_neighbors, INT8U non1905_neighbors_nr,
                                INT8U x1_update, struct neighborDeviceListTLV **x1905_neighbors, INT8U x1905_neighbors_nr,
                                INT8U ss_update, struct SupportedServiceTLV *supported_services,
                                INT8U op_update, struct ApOperationalBSSTLV *operational_bss,
                                INT8U as_update, struct AssociatedClientsTLV *associated_clients)
{
	INT8U i, j;

	if (
	    (NULL == al_mac_address) || (br_update == 1 && (bridges_nr > 0 && NULL == bridges)) || (no_update == 1 && (non1905_neighbors_nr > 0 && NULL == non1905_neighbors)) || (x1_update == 1 && (x1905_neighbors_nr > 0 && NULL == x1905_neighbors))) {
		return 0;
	}
	// First, search for an existing entry with the same AL MAC address
	// Remember that the first entry holds a reference to the *local* node.
	//
	if (0 == PLATFORM_MEMCMP(DMalMacGet(), al_mac_address, 6)) {
		i = 0;
	} else {
		for (i = 1; i < data_model.network_devices_nr; i++) {
			if (NULL != data_model.network_devices[i].info) {
				if (0 == PLATFORM_MEMCMP(data_model.network_devices[i].info->al_mac_address, al_mac_address, 6)) {
					break;
				}
			}
		}
	}

	char *base_receiving_interface = trimInterfaceNameVID(receiving_interface, NULL, NULL);

	if (i == data_model.network_devices_nr) {
		// A matching entry was *not* found. Create a new one, but only if this
		// new information contains the "info" TLV (otherwise don't do anything
		// and wait for the "info" TLV to be received in the future)
		//
		if (1 == in_update && NULL != info) {
			if (0 == data_model.network_devices_nr) {
				data_model.network_devices = (struct networkDevice *)PLATFORM_MALLOC(sizeof(struct networkDevice));
			} else {
				data_model.network_devices = (struct networkDevice *)PLATFORM_REALLOC(data_model.network_devices, sizeof(struct networkDevice) * (data_model.network_devices_nr + 1));
			}

			PLATFORM_MEMSET(&data_model.network_devices[data_model.network_devices_nr], 0, sizeof(struct networkDevice));

			data_model.network_devices[data_model.network_devices_nr].topology_timestamp = PLATFORM_GET_TIMESTAMP();
			data_model.network_devices[data_model.network_devices_nr].update_timestamp   = PLATFORM_GET_TIMESTAMP();
			strlcpy(data_model.network_devices[data_model.network_devices_nr].receiving_interface, base_receiving_interface, sizeof(data_model.network_devices[data_model.network_devices_nr].receiving_interface));
			data_model.network_devices[data_model.network_devices_nr].info                 = info;
			data_model.network_devices[data_model.network_devices_nr].bridges_nr           = 1 == br_update ? bridges_nr : 0;
			data_model.network_devices[data_model.network_devices_nr].bridges              = 1 == br_update ? bridges : NULL;
			data_model.network_devices[data_model.network_devices_nr].non1905_neighbors_nr = 1 == no_update ? non1905_neighbors_nr : 0;
			data_model.network_devices[data_model.network_devices_nr].non1905_neighbors    = 1 == no_update ? non1905_neighbors : NULL;
			data_model.network_devices[data_model.network_devices_nr].x1905_neighbors_nr   = 1 == x1_update ? x1905_neighbors_nr : 0;
			data_model.network_devices[data_model.network_devices_nr].x1905_neighbors      = 1 == x1_update ? x1905_neighbors : NULL;

			data_model.network_devices[data_model.network_devices_nr].supported_services = 1 == ss_update ? supported_services : NULL;
			data_model.network_devices[data_model.network_devices_nr].operational_bss    = 1 == op_update ? operational_bss : NULL;
			data_model.network_devices[data_model.network_devices_nr].associated_clients = 1 == as_update ? associated_clients : NULL;

			data_model.network_devices[data_model.network_devices_nr].metrics_with_neighbors_nr = 0;
			data_model.network_devices[data_model.network_devices_nr].metrics_with_neighbors    = NULL;

			data_model.network_devices[data_model.network_devices_nr].extensions    = NULL;
			data_model.network_devices[data_model.network_devices_nr].extensions_nr = 0;

			data_model.network_devices[data_model.network_devices_nr].last_ch_prefs = NULL;
			data_model.network_devices[data_model.network_devices_nr].last_ch_prefs_nr = 0;

			data_model.network_devices[data_model.network_devices_nr].ap_metrics_nr = 0;

			data_model.network_devices[data_model.network_devices_nr].discovery_timestamp = 0;

			data_model.network_devices[data_model.network_devices_nr].channel_scan_capabilities = NULL;

			data_model.network_devices[data_model.network_devices_nr].multiap_profile = 0;
			PLATFORM_MEMSET(&data_model.network_devices[data_model.network_devices_nr].multiap_version, 0, sizeof(MultiapVersion));

			data_model.network_devices[data_model.network_devices_nr].integrity_transmission_counter = 0;

			data_model.network_devices[data_model.network_devices_nr].security = SECURITY_NOT_ENABLED;

			data_model.network_devices_nr++;
		}
	} else {
		// A matching entry was found. Update it. But first, free the old TLV
		// structures (but only if a new value was provided!... otherwise retain
		// the old item)
		//
		data_model.network_devices[i].topology_timestamp = PLATFORM_GET_TIMESTAMP();
		data_model.network_devices[i].update_timestamp   = PLATFORM_GET_TIMESTAMP();
		strlcpy(data_model.network_devices[i].receiving_interface, base_receiving_interface, sizeof(data_model.network_devices[i].receiving_interface));

		if (NULL != info) {
			if (NULL != data_model.network_devices[i].info) {
				free_1905_TLV_structure((INT8U *)data_model.network_devices[i].info);
			}
			data_model.network_devices[i].info = info;
		}

		if (1 == br_update) {
			for (j = 0; j < data_model.network_devices[i].bridges_nr; j++) {
				free_1905_TLV_structure((INT8U *)data_model.network_devices[i].bridges[j]);
			}
			if (data_model.network_devices[i].bridges_nr > 0 && NULL != data_model.network_devices[i].bridges) {
				PLATFORM_FREE(data_model.network_devices[i].bridges);
			}
			data_model.network_devices[i].bridges_nr = bridges_nr;
			data_model.network_devices[i].bridges    = bridges;
		}

		if (1 == no_update) {
			for (j = 0; j < data_model.network_devices[i].non1905_neighbors_nr; j++) {
				free_1905_TLV_structure((INT8U *)data_model.network_devices[i].non1905_neighbors[j]);
			}
			if (data_model.network_devices[i].non1905_neighbors_nr > 0 && NULL != data_model.network_devices[i].non1905_neighbors) {
				PLATFORM_FREE(data_model.network_devices[i].non1905_neighbors);
			}
			data_model.network_devices[i].non1905_neighbors_nr = non1905_neighbors_nr;
			data_model.network_devices[i].non1905_neighbors    = non1905_neighbors;
		}

		if (1 == x1_update) {
			for (j = 0; j < data_model.network_devices[i].x1905_neighbors_nr; j++) {
				free_1905_TLV_structure((INT8U *)data_model.network_devices[i].x1905_neighbors[j]);
			}
			if (data_model.network_devices[i].x1905_neighbors_nr > 0 && NULL != data_model.network_devices[i].x1905_neighbors) {
				PLATFORM_FREE(data_model.network_devices[i].x1905_neighbors);
			}
			data_model.network_devices[i].x1905_neighbors_nr = x1905_neighbors_nr;
			data_model.network_devices[i].x1905_neighbors    = x1905_neighbors;
		}

		if (1 == ss_update) {
			if (NULL != data_model.network_devices[i].supported_services) {
				free_1905_TLV_structure((INT8U *)data_model.network_devices[i].supported_services);
			}

			data_model.network_devices[i].supported_services = supported_services;
		}

		if (1 == op_update) {
			if (NULL != data_model.network_devices[i].operational_bss) {
				free_1905_TLV_structure((INT8U *)data_model.network_devices[i].operational_bss);
			}

			data_model.network_devices[i].operational_bss = operational_bss;
		}

		if (1 == as_update) {
			if (NULL != data_model.network_devices[i].associated_clients) {
				free_1905_TLV_structure((INT8U *)data_model.network_devices[i].associated_clients);
			}

			data_model.network_devices[i].associated_clients = associated_clients;
		}
	}

	PLATFORM_FREE(base_receiving_interface);

	return 1;
}

INT8U DMnetworkDeviceInfoNeedsUpdate(INT8U *al_mac_address)
{
	INT8U i;

	// First, search for an existing entry with the same AL MAC address
	//
	for (i = 0; i < data_model.network_devices_nr; i++) {
		if (NULL != data_model.network_devices[i].info) {
			if (0 == PLATFORM_MEMCMP(data_model.network_devices[i].info->al_mac_address, al_mac_address, 6)) {
				break;
			}
		}
	}

	if (i == data_model.network_devices_nr) {
		// A matching entry was *not* found. Thus a refresh of the information
		// is needed.
		//
		return 1;
	} else {
		// A matching entry was found. Check its timestamp.
		//
		if (PLATFORM_GET_TIMESTAMP() - data_model.network_devices[i].topology_timestamp > MAX_AGE * 1000) {
			return 1;
		} else {
			return 0;
		}
	}
}

INT8U DMupdateNetworkDeviceMetrics(INT8U *metrics)
{
	INT8U *FROM_al_mac_address; // Metrics are reported FROM this AL entity...
	INT8U *TO_al_mac_address;   // ... TO this other one.

	INT8U i, j;

	if (NULL == metrics) {
		PLATFORM_PRINTF_ERROR("Invalid 'metrics' argument\n");
		return 0;
	}

	// Obtain the AL MAC of the devices involved in the metrics report (ie.
	// the "from" and the "to" AL MAC addresses).
	// This information is contained inside the 'metrics' structure itself.
	//
	if (TLV_TYPE_TRANSMITTER_LINK_METRIC == *metrics) {
		struct transmitterLinkMetricTLV *p;

		p = (struct transmitterLinkMetricTLV *)metrics;

		FROM_al_mac_address = p->local_al_address;
		TO_al_mac_address   = p->neighbor_al_address;

	} else if (TLV_TYPE_RECEIVER_LINK_METRIC == *metrics) {
		struct receiverLinkMetricTLV *p;

		p = (struct receiverLinkMetricTLV *)metrics;

		FROM_al_mac_address = p->local_al_address;
		TO_al_mac_address   = p->neighbor_al_address;
	} else {
		PLATFORM_PRINTF_DETAIL("Invalid 'metrics' argument. Type = %d\n", *metrics);
		return 0;
	}

	// Next, search for an existing entry with the same AL MAC address
	//
	for (i = 0; i < data_model.network_devices_nr; i++) {
		if (NULL == data_model.network_devices[i].info) {
			// We haven't received general info about this device yet.
			// This can happen, for example, when only metrics have been
			// received so far.
			//
			continue;
		}
		if (0 == PLATFORM_MEMCMP(data_model.network_devices[i].info->al_mac_address, FROM_al_mac_address, 6)) {
			break;
		}
	}

	if (i == data_model.network_devices_nr) {
		// A matching entry was *not* found.
		//
		// At this point, even if we could just create a new device entry with
		// everything set to zero (except for the metrics that we have just
		// received), it is probably wiser to simply discard the data.
		//
		// In other words, we should not accept metrics data until the "general
		// info" for this node has been processed.
		//
		PLATFORM_PRINTF_DETAIL("Metrics received from an unknown 1905 node (%02x:%02x:%02x:%02x:%02x:%02x). Ignoring data...\n", FROM_al_mac_address[0], FROM_al_mac_address[1], FROM_al_mac_address[2], FROM_al_mac_address[3], FROM_al_mac_address[4], FROM_al_mac_address[5]);
		return 0;
	}

	// Now that we have found the corresponding neighbor entry (or created a
	// new one) search for a sub-entry that matches the AL MAC of the node the
	// metrics are being reported against.
	//
	for (j = 0; j < data_model.network_devices[i].metrics_with_neighbors_nr; j++) {
		if (0 == PLATFORM_MEMCMP(data_model.network_devices[i].metrics_with_neighbors[j].neighbor_al_mac_address, TO_al_mac_address, 6)) {
			break;
		}
	}

	if (j == data_model.network_devices[i].metrics_with_neighbors_nr) {
		// A matching entry was *not* found. Create a new one
		//
		if (0 == data_model.network_devices[i].metrics_with_neighbors_nr) {
			data_model.network_devices[i].metrics_with_neighbors = (struct _metricsWithNeighbor *)PLATFORM_MALLOC(sizeof(struct _metricsWithNeighbor));
		} else {
			data_model.network_devices[i].metrics_with_neighbors = (struct _metricsWithNeighbor *)PLATFORM_REALLOC(data_model.network_devices[i].metrics_with_neighbors, sizeof(struct _metricsWithNeighbor) * (data_model.network_devices[i].metrics_with_neighbors_nr + 1));
		}

		PLATFORM_MEMCPY(data_model.network_devices[i].metrics_with_neighbors[data_model.network_devices[i].metrics_with_neighbors_nr].neighbor_al_mac_address, TO_al_mac_address, 6);

		if (TLV_TYPE_TRANSMITTER_LINK_METRIC == *metrics) {
			data_model.network_devices[i].metrics_with_neighbors[data_model.network_devices[i].metrics_with_neighbors_nr].tx_metrics_timestamp = PLATFORM_GET_TIMESTAMP();
			data_model.network_devices[i].metrics_with_neighbors[data_model.network_devices[i].metrics_with_neighbors_nr].tx_metrics           = (struct transmitterLinkMetricTLV *)metrics;

			data_model.network_devices[i].metrics_with_neighbors[data_model.network_devices[i].metrics_with_neighbors_nr].rx_metrics_timestamp = 0;
			data_model.network_devices[i].metrics_with_neighbors[data_model.network_devices[i].metrics_with_neighbors_nr].rx_metrics           = NULL;
		} else {
			data_model.network_devices[i].metrics_with_neighbors[data_model.network_devices[i].metrics_with_neighbors_nr].tx_metrics_timestamp = 0;
			data_model.network_devices[i].metrics_with_neighbors[data_model.network_devices[i].metrics_with_neighbors_nr].tx_metrics           = NULL;

			data_model.network_devices[i].metrics_with_neighbors[data_model.network_devices[i].metrics_with_neighbors_nr].rx_metrics_timestamp = PLATFORM_GET_TIMESTAMP();
			data_model.network_devices[i].metrics_with_neighbors[data_model.network_devices[i].metrics_with_neighbors_nr].rx_metrics           = (struct receiverLinkMetricTLV *)metrics;
		}

		data_model.network_devices[i].metrics_with_neighbors_nr++;
	} else {
		// A matching entry was found. Update it. But first, free the old TLV
		// structures.
		//
		if (TLV_TYPE_TRANSMITTER_LINK_METRIC == *metrics) {
			free_1905_TLV_structure((INT8U *)data_model.network_devices[i].metrics_with_neighbors[j].tx_metrics);

			data_model.network_devices[i].metrics_with_neighbors[j].tx_metrics_timestamp = PLATFORM_GET_TIMESTAMP();
			data_model.network_devices[i].metrics_with_neighbors[j].tx_metrics           = (struct transmitterLinkMetricTLV *)metrics;
		} else {
			free_1905_TLV_structure((INT8U *)data_model.network_devices[i].metrics_with_neighbors[j].rx_metrics);

			data_model.network_devices[i].metrics_with_neighbors[j].rx_metrics_timestamp = PLATFORM_GET_TIMESTAMP();
			data_model.network_devices[i].metrics_with_neighbors[j].rx_metrics           = (struct receiverLinkMetricTLV *)metrics;
		}
	}

	return 1;
}

INT8U DMupdateNetworkDeviceAPMetric(INT8U *mac, INT8U *metrics)
{

	struct networkDevice *network_device;
	struct APMetricsTLV * p = (struct APMetricsTLV *)metrics;

	const INT8U zeros[6] = { 0,0,0,0,0,0 };

	if (PLATFORM_MEMCMP(p->bssid, zeros, 6) == 0)  // in case bssid is zeros, do not do anything
		return 0;

	network_device = DMnetworkDeviceGet(mac);

	if (network_device == NULL) {
		PLATFORM_PRINTF_WARNING("%s - Network device for mac %02x%02x%02x%02x%02x%02x not found!\n",
		                        __FUNCTION__, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		return 0;
	}

	if (network_device->ap_metrics_nr < 1) {
		network_device->ap_metrics = PLATFORM_MALLOC(sizeof(struct APMetricsTLV));
		network_device->ap_metrics_nr++;
	}

	network_device->ap_metrics->tlv_type = p->tlv_type;
	PLATFORM_MEMCPY(network_device->ap_metrics->bssid, p->bssid, MACADDRLEN);
	network_device->ap_metrics->ch_util = p->ch_util;
	network_device->ap_metrics->sta_nr  = p->sta_nr;
	network_device->ap_metrics->esp_ie  = p->esp_ie;
	PLATFORM_MEMCPY(network_device->ap_metrics->esp_acbe, p->esp_acbe, 3);
	PLATFORM_MEMCPY(network_device->ap_metrics->esp_acbk, p->esp_acbk, 3);
	PLATFORM_MEMCPY(network_device->ap_metrics->esp_acvo, p->esp_acvo, 3);
	PLATFORM_MEMCPY(network_device->ap_metrics->esp_acvi, p->esp_acvi, 3);

	return 1;
}

INT8U DMupdateNetworkDeviceVersion(INT8U *al_mac, char *device_version)
{
	struct networkDevice *network_device;

	if (0 == PLATFORM_MEMCMP(DMalMacGet(), al_mac, 6)) {
		network_device = DMlocalNetworkDeviceGet();
	} else {
		network_device = DMnetworkDeviceGet(al_mac);
	}

	if (network_device == NULL) {
		PLATFORM_PRINTF_WARNING("%s - Network device with al mac %02x%02x%02x%02x%02x%02x not found!\n",
		                        __FUNCTION__, al_mac[0], al_mac[1], al_mac[2], al_mac[3], al_mac[4], al_mac[5]);
		return 1;
	}

	INT8U generation = 0, major = 0, update = 0, stage = 0;
	char  minor      = 0;
	char *ptr        = NULL;
	char  buffer[64] = { 0 };

	strlcpy(buffer, device_version, sizeof(buffer));

	if ((ptr = strstr(buffer, "Alpha"))) {
		*ptr  = 0;
		stage = 0;
	} else if ((ptr = strstr(buffer, "Beta"))) {
		*ptr  = 0;
		stage = 1;
	} else {
		stage = 0xFF;
	}

	sscanf(buffer, "EasyMesh V%hhu.%hhu%c.%hhu", &generation, &major, &minor, &update);

	network_device->multiap_version.generation = generation;
	network_device->multiap_version.major      = major;
	network_device->multiap_version.minor      = minor;
	network_device->multiap_version.update     = update;
	network_device->multiap_version.stage      = stage;

	return 0;
}

uint64_t DMgetNetworkDeviceVersion(INT8U *al_mac)
{
	struct networkDevice *network_device;

	if (0 == PLATFORM_MEMCMP(DMalMacGet(), al_mac, 6)) {
		network_device = DMlocalNetworkDeviceGet();
	} else {
		network_device = DMnetworkDeviceGet(al_mac);
	}

	if (network_device == NULL) {
		PLATFORM_PRINTF_WARNING("%s - Network device with al mac %02x%02x%02x%02x%02x%02x not found!\n",
		                        __FUNCTION__, al_mac[0], al_mac[1], al_mac[2], al_mac[3], al_mac[4], al_mac[5]);
		return 1;
	}

	return ( (uint64_t)network_device->multiap_version.generation << 56) | ((uint64_t)network_device->multiap_version.major << 48)
	       | ((uint64_t)network_device->multiap_version.minor << 40) | ((uint64_t)network_device->multiap_version.update << 32)
	       | ((uint64_t)network_device->multiap_version.stage << 24);
}

void DMdumpNetworkDevices(void (*write_function)(const char *fmt, ...))
{
// Buffer size to store a prefix string that will be used to show each
// element of a structure on screen
//
#define MAX_PREFIX 100

	INT8U i, j;

	write_function("\n");

	write_function("  device_nr: %d\n", data_model.network_devices_nr);

	for (i = 0; i < data_model.network_devices_nr; i++) {
		char new_prefix[MAX_PREFIX];

		PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "  device[%d]->", i);
		new_prefix[MAX_PREFIX - 1] = 0x0;
		write_function("%supdate timestamp: %d\n", new_prefix, data_model.network_devices[i].update_timestamp);

		PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "  device[%d]->general_info->", i);
		new_prefix[MAX_PREFIX - 1] = 0x0;
		visit_1905_TLV_structure((INT8U *)data_model.network_devices[i].info, print_callback, write_function, new_prefix);

		PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "  device[%d]->bridging_capabilities_nr: %d", i, data_model.network_devices[i].bridges_nr);
		new_prefix[MAX_PREFIX - 1] = 0x0;
		write_function("%s\n", new_prefix);
		for (j = 0; j < data_model.network_devices[i].bridges_nr; j++) {
			PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "  device[%d]->bridging_capabilities[%d]->", i, j);
			new_prefix[MAX_PREFIX - 1] = 0x0;
			visit_1905_TLV_structure((INT8U *)data_model.network_devices[i].bridges[j], print_callback, write_function, new_prefix);
		}

		PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "  device[%d]->non_1905_neighbors_nr: %d", i, data_model.network_devices[i].non1905_neighbors_nr);
		new_prefix[MAX_PREFIX - 1] = 0x0;
		write_function("%s\n", new_prefix);
		for (j = 0; j < data_model.network_devices[i].non1905_neighbors_nr; j++) {
			PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "  device[%d]->non_1905_neighbors[%d]->", i, j);
			new_prefix[MAX_PREFIX - 1] = 0x0;
			visit_1905_TLV_structure((INT8U *)data_model.network_devices[i].non1905_neighbors[j], print_callback, write_function, new_prefix);
		}

		PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "  device[%d]->x1905_neighbors_nr: %d", i, data_model.network_devices[i].x1905_neighbors_nr);
		new_prefix[MAX_PREFIX - 1] = 0x0;
		write_function("%s\n", new_prefix);
		for (j = 0; j < data_model.network_devices[i].x1905_neighbors_nr; j++) {
			PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "  device[%d]->x1905_neighbors[%d]->", i, j);
			new_prefix[MAX_PREFIX - 1] = 0x0;
			visit_1905_TLV_structure((INT8U *)data_model.network_devices[i].x1905_neighbors[j], print_callback, write_function, new_prefix);
		}

		PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "  device[%d]->metrics_nr: %d", i, data_model.network_devices[i].metrics_with_neighbors_nr);
		new_prefix[MAX_PREFIX - 1] = 0x0;
		write_function("%s\n", new_prefix);
		for (j = 0; j < data_model.network_devices[i].metrics_with_neighbors_nr; j++) {
			PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "  device[%d]->metrics[%d]->tx->", i, j);
			new_prefix[MAX_PREFIX - 1] = 0x0;
			if (NULL != data_model.network_devices[i].metrics_with_neighbors[j].tx_metrics) {
				write_function("%slast_updated: %d\n", new_prefix, data_model.network_devices[i].metrics_with_neighbors[j].tx_metrics_timestamp);
				visit_1905_TLV_structure((INT8U *)data_model.network_devices[i].metrics_with_neighbors[j].tx_metrics, print_callback, write_function, new_prefix);
			}
			PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "  device[%d]->metrics[%d]->rx->", i, j);
			new_prefix[MAX_PREFIX - 1] = 0x0;
			if (NULL != data_model.network_devices[i].metrics_with_neighbors[j].rx_metrics) {
				write_function("%slast updated: %d\n", new_prefix, data_model.network_devices[i].metrics_with_neighbors[j].rx_metrics_timestamp);
				visit_1905_TLV_structure((INT8U *)data_model.network_devices[i].metrics_with_neighbors[j].rx_metrics, print_callback, write_function, new_prefix);
			}
		}

		PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "  device[%d]->supported_service->", i);
		new_prefix[MAX_PREFIX - 1] = 0x0;
		visit_1905_TLV_structure((INT8U *)data_model.network_devices[i].operational_bss, print_callback, write_function, new_prefix);

		PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "  device[%d]->operational_bss->", i);
		new_prefix[MAX_PREFIX - 1] = 0x0;
		visit_1905_TLV_structure((INT8U *)data_model.network_devices[i].operational_bss, print_callback, write_function, new_prefix);

		PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "  device[%d]->associated_clients->", i);
		new_prefix[MAX_PREFIX - 1] = 0x0;
		visit_1905_TLV_structure((INT8U *)data_model.network_devices[i].associated_clients, print_callback, write_function, new_prefix);

		// Non-standard report section.
		// Allow registered third-party developers to extend the neighbor info
		// (ex. BBF adds non-1905 link metrics)
		//
		PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "  device[%d]->", i);
		new_prefix[MAX_PREFIX - 1] = 0x0;
	}

	return;
}

INT8U DMrunGarbageCollector(INT8U *is_controller_lost)
{
	INT8U i, j, k;
	INT8U removed_entries;
	INT8U original_devices_nr;

	removed_entries = 0;

	*is_controller_lost = 0;

	// Visit all existing devices, searching for those with a timestamp older
	// than GC_MAX_AGE
	//
	// Note that we skip element "0", which is always the local device. We don't
	// care when it was last updated as it is always updated "on demand", just
	// before someone requests its data (right now the only place where this
	// happens is when using an ALME custom command)
	//
	original_devices_nr = data_model.network_devices_nr;
	for (i = 1; i < data_model.network_devices_nr; i++) {
		INT8U *p = NULL;
		if ((PLATFORM_GET_TIMESTAMP() - data_model.network_devices[i].discovery_timestamp > (data_model.neighbor_remove_threshold * 1000))) {
			if(data_model.network_devices[i].info) {
				DMremoveALNeighborFromInterface(data_model.network_devices[i].info->al_mac_address, "all");
				// if ((PLATFORM_GET_TIMESTAMP() - data_model.network_devices[i].discovery_timestamp > (data_model.neighbor_remove_threshold * 1000 * 2))) {
				// 	delete_agent_with_al_mac(data_model.network_devices[i].info->al_mac_address, 0);
				// }
			}
		}
		if (
		    (PLATFORM_GET_TIMESTAMP() - data_model.network_devices[i].update_timestamp > (GC_MAX_AGE * 1000)) || (NULL != data_model.network_devices[i].info && NULL == (p = DMmacToAlMac(data_model.network_devices[i].info->al_mac_address)))) {
			// Entry too old or with a MAC address no longer registered in the
			// "topology discovery" database. Remove it.
			//
			INT8U                 al_mac_address[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
			struct networkDevice *x;

			removed_entries++;

			x = &data_model.network_devices[i];

			// First, free all child structures
			//
			if (NULL != x->info) {
				// Save the MAC of the node that is going to be removed for
				// later use
				//
				PLATFORM_MEMCPY(al_mac_address, x->info->al_mac_address, 6);

				PLATFORM_PRINTF_DEBUG("Removing old device entry (%02x:%02x:%02x:%02x:%02x:%02x)\n", x->info->al_mac_address[0], x->info->al_mac_address[1], x->info->al_mac_address[2], x->info->al_mac_address[3], x->info->al_mac_address[4], x->info->al_mac_address[5]);
				free_1905_TLV_structure((INT8U *)x->info);
				x->info = NULL;
			} else {
				PLATFORM_PRINTF_WARNING("Removing old device entry (Unknown AL MAC)\n");
			}

			for (j = 0; j < x->bridges_nr; j++) {
				free_1905_TLV_structure((INT8U *)x->bridges[j]);
			}
			if (0 != x->bridges_nr && NULL != x->bridges) {
				PLATFORM_FREE(x->bridges);
				x->bridges_nr = 0;
				x->bridges    = NULL;
			}

			for (j = 0; j < x->non1905_neighbors_nr; j++) {
				free_1905_TLV_structure((INT8U *)x->non1905_neighbors[j]);
			}
			if (0 != x->non1905_neighbors_nr && NULL != x->non1905_neighbors) {
				PLATFORM_FREE(x->non1905_neighbors);
				x->non1905_neighbors_nr = 0;
				x->non1905_neighbors    = NULL;
			}

			for (j = 0; j < x->x1905_neighbors_nr; j++) {
				free_1905_TLV_structure((INT8U *)x->x1905_neighbors[j]);
			}
			if (0 != x->x1905_neighbors_nr && NULL != x->x1905_neighbors) {
				PLATFORM_FREE(x->x1905_neighbors);
				x->x1905_neighbors_nr = 0;
				x->x1905_neighbors    = NULL;
			}

			for (j = 0; j < x->metrics_with_neighbors_nr; j++) {
				free_1905_TLV_structure((INT8U *)x->metrics_with_neighbors[j].tx_metrics);
				free_1905_TLV_structure((INT8U *)x->metrics_with_neighbors[j].rx_metrics);
			}
			if (0 != x->metrics_with_neighbors_nr && NULL != x->metrics_with_neighbors) {
				PLATFORM_FREE(x->metrics_with_neighbors);
				x->metrics_with_neighbors = NULL;
			}

			if (x->ap_metrics) {
				free_1905_TLV_structure((INT8U *)x->ap_metrics);
				x->ap_metrics_nr = 0;
				x->ap_metrics = NULL;
			}

			// Next, remove the _networkDevice entry
			//
			if (i == (data_model.network_devices_nr - 1)) {
				// Last element. It will automatically be removed below (keep
				// reading)
			} else {
				// Place the last element here (we don't care about preserving
				// order)
				//
				data_model.network_devices[i] = data_model.network_devices[data_model.network_devices_nr - 1];
				i--;
			}
			data_model.network_devices_nr--;

			// Next, Remove all references to this node from other node's
			// metrics information entries
			//
			for (j = 0; j < data_model.network_devices_nr; j++) {
				INT8U original_neighbors_nr;

				original_neighbors_nr = data_model.network_devices[j].metrics_with_neighbors_nr;

				for (k = 0; k < data_model.network_devices[j].metrics_with_neighbors_nr; k++) {
					if (0 == PLATFORM_MEMCMP(al_mac_address, data_model.network_devices[j].metrics_with_neighbors[k].neighbor_al_mac_address, 6)) {
						free_1905_TLV_structure((INT8U *)data_model.network_devices[j].metrics_with_neighbors[k].tx_metrics);
						free_1905_TLV_structure((INT8U *)data_model.network_devices[j].metrics_with_neighbors[k].rx_metrics);

						// Place last element here (we don't care about
						// preserving order)
						//
						if (k == (data_model.network_devices[j].metrics_with_neighbors_nr - 1)) {
							// Last element. It will automatically be removed
							// below (keep reading)
						} else {
							data_model.network_devices[j].metrics_with_neighbors[k] = data_model.network_devices[j].metrics_with_neighbors[data_model.network_devices[j].metrics_with_neighbors_nr - 1];
							k--;
						}
						data_model.network_devices[j].metrics_with_neighbors_nr--;
					}
				}

				if (original_neighbors_nr != data_model.network_devices[j].metrics_with_neighbors_nr) {
					if (0 == data_model.network_devices[j].metrics_with_neighbors_nr) {
						PLATFORM_FREE(data_model.network_devices[j].metrics_with_neighbors);
					} else {
						data_model.network_devices[j].metrics_with_neighbors = (struct _metricsWithNeighbor *)PLATFORM_REALLOC(data_model.network_devices[j].metrics_with_neighbors, sizeof(struct _metricsWithNeighbor) * (data_model.network_devices[j].metrics_with_neighbors_nr));
					}
				}
			}

			if (NULL != x->supported_services) {
				free_1905_TLV_structure((INT8U *)x->supported_services);
				x->supported_services = NULL;
			}

			if (NULL != x->operational_bss) {
				free_1905_TLV_structure((INT8U *)x->operational_bss);
				x->operational_bss = NULL;
			}

			if (NULL != x->associated_clients) {
				free_1905_TLV_structure((INT8U *)x->associated_clients);
				x->associated_clients = NULL;
			}

			// And also from the local interfaces database
			//
			DMremoveALNeighborFromInterface(al_mac_address, "all");

			if (0 == PLATFORM_MEMCMP(DMcontrollerMacGet(), al_mac_address, 6)) {
				*is_controller_lost = 1;
				INT8U null_addr[6]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
				DMcontrollerMacSet(null_addr);
			}
		}

		if (NULL != p) {
			PLATFORM_FREE(p);
		}
	}

	// If at least one element was removed, we need to realloc
	//
	if (original_devices_nr != data_model.network_devices_nr) {
		if (0 == data_model.network_devices_nr) {
			PLATFORM_FREE(data_model.network_devices);
		} else {
			data_model.network_devices = (struct networkDevice *)PLATFORM_REALLOC(data_model.network_devices, sizeof(struct networkDevice) * (data_model.network_devices_nr));
		}
	}

	return removed_entries;
}

void DMremoveALNeighborFromInterface(INT8U *al_mac_address, char *interface_name)
{
	INT8U i, j;

	for (i = 0; i < data_model.local_interfaces_nr; i++) {
		INT8U original_neighbors_nr;

		if (
		    (0 != PLATFORM_STRCMP(data_model.local_interfaces[i].name, interface_name)) && (0 != PLATFORM_STRCMP(interface_name, "all"))) {
			// Ignore this interface
			//
			continue;
		}

		original_neighbors_nr = data_model.local_interfaces[i].neighbors_nr;

		for (j = 0; j < data_model.local_interfaces[i].neighbors_nr; j++) {
			if (0 == PLATFORM_MEMCMP(al_mac_address, data_model.local_interfaces[i].neighbors[j].al_mac_address, 6)) {
				if (data_model.local_interfaces[i].neighbors[j].remote_interfaces_nr > 0 && NULL != data_model.local_interfaces[i].neighbors[j].remote_interfaces) {
					PLATFORM_FREE(data_model.local_interfaces[i].neighbors[j].remote_interfaces);

					data_model.local_interfaces[i].neighbors[j].remote_interfaces    = NULL;
					data_model.local_interfaces[i].neighbors[j].remote_interfaces_nr = 0;
				}

				// Place last element here (we don't care about preserving
				// order)
				//
				if (j == (data_model.local_interfaces[i].neighbors_nr - 1)) {
					// Last element. It will automatically be removed
					// below (keep reading)
				} else {
					data_model.local_interfaces[i].neighbors[j] = data_model.local_interfaces[i].neighbors[data_model.local_interfaces[i].neighbors_nr - 1];
					j--;
				}
				data_model.local_interfaces[i].neighbors_nr--;
			}
		}

		if (original_neighbors_nr != data_model.local_interfaces[i].neighbors_nr) {
			if (0 == data_model.local_interfaces[i].neighbors_nr) {
				PLATFORM_SAFE_FREE((void **)&(data_model.local_interfaces[i].neighbors));
			} else {
				data_model.local_interfaces[i].neighbors = (struct _neighbor *)PLATFORM_REALLOC(data_model.local_interfaces[i].neighbors, sizeof(struct _neighbor) * (data_model.local_interfaces[i].neighbors_nr));
			}
		}
	}
}

struct vendorSpecificTLV ***DMextensionsGet(INT8U *al_mac_address, INT8U **nr)
{
	INT8U                       i;
	struct vendorSpecificTLV ***extensions;

	// Find device
	if ((NULL == al_mac_address) || (NULL == nr)) {
		PLATFORM_PRINTF_ERROR("Invalid 'DMextensionsGet' argument\n");
		return NULL;
	}

	// Search for an existing entry with the same AL MAC address
	//
	for (i = 0; i < data_model.network_devices_nr; i++) {
		if (NULL == data_model.network_devices[i].info) {
			// We haven't received general info about this device yet.
			//
			continue;
		}
		if (0 == PLATFORM_MEMCMP(data_model.network_devices[i].info->al_mac_address, al_mac_address, 6)) {
			break;
		}
	}

	if (i == data_model.network_devices_nr) {
		// A matching entry was *not* found.
		//
		PLATFORM_PRINTF_DETAIL("Extension received from an unknown 1905 node (%02x:%02x:%02x:%02x:%02x:%02x). Ignoring data...\n", al_mac_address[0], al_mac_address[1], al_mac_address[2], al_mac_address[3], al_mac_address[4], al_mac_address[5]);
		extensions = NULL;
	} else {
		// Point to the datamodel extensions section
		//
		extensions = &data_model.network_devices[i].extensions;
		*nr        = &data_model.network_devices[i].extensions_nr;
	}

	return extensions;
}

struct networkDevice *DMnetworkDeviceGet(INT8U *al_mac_address)
{
	int i = 1;
	if (data_model.is_controller || data_model.listening_lo) {
		if (0 == PLATFORM_MEMCMP(al_mac_address, data_model.al_mac_address, 6)) {
			return &data_model.network_devices[0];
		}
	}
	for (i = 1; i < data_model.network_devices_nr; i++) {
		if (NULL != data_model.network_devices[i].info) {
			if (0 == PLATFORM_MEMCMP(data_model.network_devices[i].info->al_mac_address, al_mac_address, 6)) {
				return &data_model.network_devices[i];
			}
		}
	}
	// Return NULL for device without 1905 Topology Response received
	return NULL;
}

INT8U *DMallNetworkDevicesGet(struct networkDevice **network_devices, INT8U *network_devices_nr)
{
	*network_devices    = data_model.network_devices;
	*network_devices_nr = data_model.network_devices_nr;
	// Return NULL for device without 1905 Topology Response received
	return 0;
}

INT8U DMupdateControllerChannelPreference(struct ChannelPreferenceTLV *preference)
{
	INT8U *radio_unique_ID;
	INT8U  i;
	if (NULL == preference) {
		PLATFORM_PRINTF_ERROR("Invalid 'preference' argument\n");
		return 2;
	}

	radio_unique_ID = preference->radio_unique_id;

	if (validateChannelPreferenceReport(*preference)) {
		return 2;
	}

	for (i = 0; i < data_model.controller_channel_reports_nr; i++) {
		if (0 == PLATFORM_MEMCMP(data_model.controller_channel_reports[i].radio_unique_id, radio_unique_ID, 6)) {
			break;
		}
	}

	if (i == data_model.controller_channel_reports_nr) {
		// A matching entry was *not* found. Create a new one
		//
		if (0 == data_model.controller_channel_reports_nr) {
			data_model.controller_channel_reports = (struct ChannelPreferenceTLV *)PLATFORM_MALLOC(sizeof(struct ChannelPreferenceTLV));
		} else {
			data_model.controller_channel_reports = (struct ChannelPreferenceTLV *)PLATFORM_REALLOC(data_model.controller_channel_reports, sizeof(struct ChannelPreferenceTLV) * (data_model.controller_channel_reports_nr + 1));
		}

		data_model.controller_channel_reports[i].tlv_type = TLV_TYPE_CHANNEL_PREFERENCE;
		PLATFORM_MEMCPY(data_model.controller_channel_reports[i].radio_unique_id, radio_unique_ID, 6);
		PLATFORM_PRINTF_DEBUG("[MULTI_AP] Agent: Store a new Channel Preference TLV with radio unique id: %02x:%02x:%02x:%02x:%02x:%02x\n",
		                      data_model.controller_channel_reports[i].radio_unique_id[0],
		                      data_model.controller_channel_reports[i].radio_unique_id[1],
		                      data_model.controller_channel_reports[i].radio_unique_id[2],
		                      data_model.controller_channel_reports[i].radio_unique_id[3],
		                      data_model.controller_channel_reports[i].radio_unique_id[4],
		                      data_model.controller_channel_reports[i].radio_unique_id[5]);
		data_model.controller_channel_reports[i].op_class_nr = preference->op_class_nr;
		PLATFORM_PRINTF_DETAIL("[MULTI_AP] Agent: The number of Operating Class: %d\n", data_model.controller_channel_reports[i].op_class_nr);
		data_model.controller_channel_reports[i].channel_preferences = (struct ChannelPreferencePerOpClass *)PLATFORM_ZALLOC(sizeof(struct ChannelPreferencePerOpClass) * preference->op_class_nr);

		int k;
		for (k = 0; k < preference->op_class_nr; k++) {
			data_model.controller_channel_reports[i].channel_preferences[k].op_class = preference->channel_preferences[k].op_class;
			PLATFORM_PRINTF_DETAIL("[MULTI_AP] Agent: Operating Class[%d]: %d\n", k, data_model.controller_channel_reports[i].channel_preferences[k].op_class);
			data_model.controller_channel_reports[i].channel_preferences[k].channel_nr = preference->channel_preferences[k].channel_nr;
			PLATFORM_PRINTF_DETAIL("[MULTI_AP] Agent: The number of channels: %d\n", data_model.controller_channel_reports[i].channel_preferences[k].channel_nr);
			if (0 != data_model.controller_channel_reports[i].channel_preferences[k].channel_nr) {
				data_model.controller_channel_reports[i].channel_preferences[k].channel_list = (INT8U *)PLATFORM_ZALLOC(sizeof(INT8U) * preference->channel_preferences[k].channel_nr);
				PLATFORM_MEMCPY(data_model.controller_channel_reports[i].channel_preferences[k].channel_list, preference->channel_preferences[k].channel_list, preference->channel_preferences[k].channel_nr);
				int j;
				for (j = 0; j < data_model.controller_channel_reports[i].channel_preferences[k].channel_nr; j++) {
					PLATFORM_PRINTF_DETAIL("[MULTI_AP] Agent: Channels[%d]: %d\n", j, data_model.controller_channel_reports[i].channel_preferences[k].channel_list[j]);
				}
			}
			data_model.controller_channel_reports[i].channel_preferences[k].preference_reason_code = preference->channel_preferences[k].preference_reason_code;
			PLATFORM_PRINTF_DETAIL("[MULTI_AP] Agent: Preference: %d \t Reason Code: %d\n", data_model.controller_channel_reports[i].channel_preferences[k].preference_reason_code >> 4, data_model.controller_channel_reports[i].channel_preferences[k].preference_reason_code & 0x0F);
		}

		data_model.controller_channel_reports_nr++;
	} else {
		// A matching entry was found. Update it. But first, free the old TLV
		// structures.
		// TLV type and radio unique id are unnecessary to update.
		//
		int k;
		for (k = 0; k < data_model.controller_channel_reports[i].op_class_nr; k++) {
			PLATFORM_FREE(data_model.controller_channel_reports[i].channel_preferences[k].channel_list);
		}
		PLATFORM_FREE(data_model.controller_channel_reports[i].channel_preferences);

		PLATFORM_PRINTF_DEBUG("[MULTI_AP] Agent: Update a old Channel Preference TLV with radio unique id: %02x:%02x:%02x:%02x:%02x:%02x\n",
		                      data_model.controller_channel_reports[i].radio_unique_id[0],
		                      data_model.controller_channel_reports[i].radio_unique_id[1],
		                      data_model.controller_channel_reports[i].radio_unique_id[2],
		                      data_model.controller_channel_reports[i].radio_unique_id[3],
		                      data_model.controller_channel_reports[i].radio_unique_id[4],
		                      data_model.controller_channel_reports[i].radio_unique_id[5]);
		data_model.controller_channel_reports[i].op_class_nr         = preference->op_class_nr;

		data_model.controller_channel_reports[i].channel_preferences = NULL;
		if (preference->op_class_nr) {
			data_model.controller_channel_reports[i].channel_preferences = (struct ChannelPreferencePerOpClass *)PLATFORM_MALLOC(sizeof(struct ChannelPreferencePerOpClass) * preference->op_class_nr);
			for (k = 0; k < preference->op_class_nr; k++) {
				data_model.controller_channel_reports[i].channel_preferences[k].op_class     = preference->channel_preferences[k].op_class;
				data_model.controller_channel_reports[i].channel_preferences[k].channel_nr   = preference->channel_preferences[k].channel_nr;
				data_model.controller_channel_reports[i].channel_preferences[k].channel_list = NULL;
				if (preference->channel_preferences[k].channel_nr > 0) {
					data_model.controller_channel_reports[i].channel_preferences[k].channel_list = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * preference->channel_preferences[k].channel_nr);
					PLATFORM_MEMCPY(data_model.controller_channel_reports[i].channel_preferences[k].channel_list, preference->channel_preferences[k].channel_list, preference->channel_preferences[k].channel_nr);
				}
				data_model.controller_channel_reports[i].channel_preferences[k].preference_reason_code = preference->channel_preferences[k].preference_reason_code;
			}
		}
	}

	return 0;
}

void DMgetControllerChannelPreference(struct ChannelPreferenceTLV **preferences, INT8U *preference_nr)
{
	*preferences = data_model.controller_channel_reports;
	*preference_nr = data_model.controller_channel_reports_nr;
}

void DMswitchToBestChannel()
{
	char **   interfaces_names;
	INT8U     interfaces_nr;
	int       i;
	char *    radio_name     = NULL;
	int       op_classes_len = 0;
	OP_CLASS *op_classes     = PLATFORM_GET_OP_CLASS(&op_classes_len);

	if (NULL == op_classes) {
		PLATFORM_PRINTF_ERROR("[RTK] No operating class is available\n");
		return;
	}

	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_nr);

	for (i = 0; i < interfaces_nr; i++) {
		if (PLATFORM_STRSTR(interfaces_names[i], map_vap_prefix) || PLATFORM_STRSTR(interfaces_names[i], map_vxd_prefix)) {
			PLATFORM_PRINTF_DETAIL("[MULTI_AP] Skip non-root interface %s\n", interfaces_names[i]);
			continue;
		}
		struct interfaceInfo *x;
		x = PLATFORM_GET_1905_INTERFACE_INFO(interfaces_names[i]);
		if (NULL == x) {
			PLATFORM_PRINTF_ERROR("[MULTI_AP] Could not retrieve interface info for %s\n", interfaces_names[i]);
			continue;
		}

		// ignore the interface if it is not an AP
		if (!(IEEE80211_ROLE_AP == x->interface_type_data.ieee80211.role
			&& IS_IEEE_802_11_INTERFACE(x->interface_type))) {
			PLATFORM_PRINTF_WARNING("[MULTI_AP] The interface %s is not operating a BSS.\n", interfaces_names[i]);
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
			continue;
		}

		int   band = 0, sub_band = 0;
		INT8U channel_nr    = 0;
		INT8U channels[256] = { 0 };
		INT8U best_channel;
		int   best_bandwidth = BAND_WIDTH_20MHZ, best_sideband = SIDE_BAND_NONE;
		INT8U cur_channel;
		int   current_bandwidth = BAND_WIDTH_20MHZ, current_sideband = SIDE_BAND_NONE;
		INT8U filter = FILTER_NONE;

		int k;
		for (k = 0; k < data_model.controller_channel_reports_nr; k++) {
			if (0 == PLATFORM_MEMCMP(x->mac_address, data_model.controller_channel_reports[k].radio_unique_id, 6)) {
				break;
			}
		}

		PLATFORM_FREE_1905_INTERFACE_INFO(x);

		if (k == data_model.controller_channel_reports_nr) { /*no channel reports relate to this interface*/
			PLATFORM_PRINTF_WARNING("[MULTI_AP] No stored channel report for interface %s.\n", interfaces_names[i]);
			continue;
		}

		if (PLATFORM_STRSTR(interfaces_names[i], map_radio_name_5gl) && PLATFORM_STRSTR(interfaces_names[i], map_radio_name_5gh)) {
			band = _5G;
			PLATFORM_PRINTF_DEBUG("[MULTI_AP] 5G band: %s\n", interfaces_names[i]);
			radio_name = map_radio_name_5gl;
		} else if (PLATFORM_STRSTR(interfaces_names[i], map_radio_name_5gh)) {
			band     = _5G;
			sub_band = _5GH;
			PLATFORM_PRINTF_DEBUG("[MULTI_AP] 5GH band: %s\n", interfaces_names[i]);
			filter = FILTER_5GH;
			radio_name = map_radio_name_5gh;
		} else if (PLATFORM_STRSTR(interfaces_names[i], map_radio_name_5gl)) {
			band     = _5G;
			sub_band = _5GL;
			PLATFORM_PRINTF_DEBUG("[MULTI_AP] 5GL band: %s\n", interfaces_names[i]);
			filter = FILTER_5GL;
			radio_name = map_radio_name_5gl;
		} else if (PLATFORM_STRSTR(interfaces_names[i], map_radio_name_24g)) {
			band = _2G;
			PLATFORM_PRINTF_DEBUG("[MULTI_AP] 2G band: %s\n", interfaces_names[i]);
			radio_name = map_radio_name_24g;
		} else {
			PLATFORM_PRINTF_ERROR("[RTK] [%s] unknown interface: %s\n", __FUNCTION__, interfaces_names[i]);
			continue;
		}

		if(obtain_channel_info(radio_name, &cur_channel, &current_bandwidth, &current_sideband, NULL)){
			PLATFORM_PRINTF_ERROR("Can not get channel info for interface %s\n", radio_name);
			continue;
		}

		PLATFORM_PRINTF_DEBUG("[MULTI_AP] Current operating channel: %d\n", cur_channel);

		channel_nr = PLATFORM_GET_AVAILABLE_CHANNELS(radio_name, channels, (INT16U)sizeof(channels), filter);

		int                       j, p, q;
		struct ChannelPreference *ch_preference = NULL;
		// set the origin preference value to all channels
		ch_preference = (struct ChannelPreference *)PLATFORM_MALLOC(sizeof(struct ChannelPreference) * channel_nr);
		for (j = 0; j < channel_nr; j++) {
			ch_preference[j].channel    = channels[j];
			ch_preference[j].preference = 15;
			//PLATFORM_PRINTF_INFO("[MULTI_AP] channel: %d, preference: %d\n", ch_preference[j].channel, ch_preference[j].preference);
		}
		for (j = 0; j < op_classes_len; j++) {
			if (band == op_classes[j].band && (sub_band == op_classes[j].sub_band || 0 == sub_band || _5GF == op_classes[j].sub_band)) {
				for (p = 0; p < data_model.controller_channel_reports[k].op_class_nr; p++) {
					if (op_classes[j].op_class == data_model.controller_channel_reports[k].channel_preferences[p].op_class) {
						if(op_classes[j].bandwidth == BAND_WIDTH_20MHZ) {
							// the whole op class with the same preference
							if (data_model.controller_channel_reports[k].channel_preferences[p].channel_nr == 0) {
								for (q = 0; q < op_classes[j].channel_array[0]; q++) {
									_setChannelPreference(ch_preference, channel_nr, op_classes[j].channel_array[q + 1], data_model.controller_channel_reports[k].channel_preferences[p].preference_reason_code >> 4);
								}
							} else { // only set the preference for the channels in the TLV
								for (q = 0; q < data_model.controller_channel_reports[k].channel_preferences[p].channel_nr; q++) {
									_setChannelPreference(ch_preference, channel_nr, data_model.controller_channel_reports[k].channel_preferences[p].channel_list[q], data_model.controller_channel_reports[k].channel_preferences[p].preference_reason_code >> 4);
								}
							}
						} else {
							if ((data_model.controller_channel_reports[k].channel_preferences[p].preference_reason_code >> 4) || data_model.controller_channel_reports[k].channel_preferences[p].channel_nr) {
								if(op_classes[j].bandwidth > best_bandwidth) {
									best_bandwidth = op_classes[j].bandwidth;
									if(op_classes[j].side_band != SIDE_BAND_NONE && best_sideband == SIDE_BAND_NONE) {
										best_sideband = op_classes[j].side_band;
									}
								}
							}
						}
					}
				}
			}
		}

		if (DMgetWFATestMode()) {
			if (_2G == band)
				best_bandwidth = BAND_WIDTH_20MHZ;
			else if (_5G == band && sub_band == _5GH)
				best_bandwidth = BAND_WIDTH_80MHZ;
			else if (_5G == band) // if band is 5G and sub_band is not 5GH, do not check the sub_band 
				best_bandwidth = BAND_WIDTH_40MHZ;
		}

		for (j = 0; j < channel_nr; j++) {
			PLATFORM_PRINTF_DETAIL("channel: %d \t preference: %d\n", ch_preference[j].channel, ch_preference[j].preference);
		}

		// find out the channel with the highest preference
		best_channel = _findBestChannel(ch_preference, channel_nr);

		if (_getChannelPreference(ch_preference, channel_nr, best_channel) == _getChannelPreference(ch_preference, channel_nr, cur_channel) && current_bandwidth == best_bandwidth && current_sideband == best_sideband) {
			PLATFORM_PRINTF_DEBUG("[MULTI_AP] Current channel %d is already the best channel\n", cur_channel);
		} else {
			PLATFORM_PRINTF_DEBUG("[MULTI_AP] Switch to the best channel: %d\n", best_channel);
			// switch to the best channel
			char buff[4];
			// char cmd[128];
			PLATFORM_SNPRINTF(buff, sizeof(buff), "%d", best_channel);
			PLATFORM_SET_MIB(radio_name, "channel", buff);

			uint64_t device_version = DMgetNetworkDeviceVersion(DMcontrollerMacGet());
			PLATFORM_PRINTF_DETAIL("Controller with EasyMesh Version %llu\n", device_version);

			if(device_version != 0 && (device_version < OLD_RELEASE_VERSION || device_version == OLD_BETA_RELEASE_VERSION)) {
				PLATFORM_PRINTF_TRACE("Old Realtek Controller %d.%d%s, skip set bandwidth and sideband\n", (int)((device_version >> 56) & 0xFFL), (int)((device_version >> 48) & 0xFFL), (int)((device_version >> 24) & 0xFFL)?"Beta":"");
			} else {
				PLATFORM_SNPRINTF(buff, sizeof(buff), "%d", best_bandwidth);
				PLATFORM_SET_MIB(radio_name, "use40M", buff);
				if (SIDE_BAND_LOWER == best_sideband) {
					// SecondaryChannelAbove
					PLATFORM_SNPRINTF(buff, sizeof(buff), "%d", 2);
				} else if (SIDE_BAND_UPPER == best_sideband) {
					// SecondaryChannelBelow
					PLATFORM_SNPRINTF(buff, sizeof(buff), "%d", 1);
				} else {
					PLATFORM_SNPRINTF(buff, sizeof(buff), "%d", 0);
				}
				PLATFORM_SET_MIB(radio_name, "2ndchoffset", buff);
				PLATFORM_SNPRINTF(buff, sizeof(buff), "%d", 1);
			}

			PLATFORM_SET_MIB(radio_name, "multiap_change_channel", buff);
		}

		// free channel preferences
		PLATFORM_FREE(ch_preference);
	}

	PLATFORM_FREE(op_classes);
}

INT8U DMchangeTransmitPower(INT8U *report)
{
	INT8U *radio_unique_ID;
	INT8U  i;
	if (NULL == report) {
		PLATFORM_PRINTF_ERROR("Invalid 'report' argument\n");
		return 2;
	}

	struct TransmitPowerLimitTLV *p;
	p               = (struct TransmitPowerLimitTLV *)report;
	radio_unique_ID = p->radio_unique_id;
	char **interfaces_names;
	INT8U  interfaces_nr;

	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_nr);
	for (i = 0; i < interfaces_nr; i++) {
		struct interfaceInfo *x;

		if (strstr(interfaces_names[i], map_vxd_prefix) || strstr(interfaces_names[i], map_vap_prefix)) {
			PLATFORM_PRINTF_DETAIL("[MULTI_AP] Skip non root interface %s\n", interfaces_names[i]);
			continue;
		}

		x = PLATFORM_GET_1905_INTERFACE_INFO(interfaces_names[i]);

		if (NULL == x) {
			PLATFORM_PRINTF_ERROR("[MULTI_AP] Could not retrieve interface info for %s\n", interfaces_names[i]);
			continue;
		}

		if (PLATFORM_MEMCMP(radio_unique_ID, x->mac_address, 6)) {
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
			continue;
		}

		// ignore the interface if it is not an AP
		if (!(IEEE80211_ROLE_AP == x->interface_type_data.ieee80211.role
			&& IS_IEEE_802_11_INTERFACE(x->interface_type))) {
			PLATFORM_PRINTF_DETAIL("[MULTI_AP] The interface %s is not operating a BSS.\n", interfaces_names[i]);
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
			continue;
		}
		char buff[3];
		PLATFORM_SNPRINTF(buff, sizeof(buff), "%d", p->transmit_power_limit);
		// set memory mib
		PLATFORM_SET_MIB(interfaces_names[i], "tpc_tx_power", buff);
		PLATFORM_SET_TX_MAX_POWER_IOCTL(interfaces_names[i], p->transmit_power_limit);
		PLATFORM_FREE_1905_INTERFACE_INFO(x);
		break;
	}
	return 0;
}

INT8U DMupdateSteeringPolicy(INT8U *report)
{
	INT8U i;
	if (NULL == report) {
		PLATFORM_PRINTF_ERROR("Invalid 'report' argument\n");
		return 1;
	}

	struct SteeringPolicyTLV *p;
	p = (struct SteeringPolicyTLV *)report;

	// Delete previous steering policy
	//
	if (NULL != data_model.steering_policy) {
		free_1905_TLV_structure((INT8U *)data_model.steering_policy);
	}

	data_model.steering_policy = (struct SteeringPolicyTLV *)PLATFORM_MALLOC(sizeof(struct SteeringPolicyTLV));
	PLATFORM_MEMSET(data_model.steering_policy, 0, sizeof(struct SteeringPolicyTLV));

	data_model.steering_policy->tlv_type = TLV_TYPE_STEERING_POLICY;

	data_model.steering_policy->local_disallowed_sta_nr  = p->local_disallowed_sta_nr;
	if (p->local_disallowed_sta_nr) {
		data_model.steering_policy->local_disallowed_sta_mac = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(INT8U[6]) * p->local_disallowed_sta_nr);
		for (i = 0; i < p->local_disallowed_sta_nr; i++) {
			PLATFORM_MEMCPY(data_model.steering_policy->local_disallowed_sta_mac[i], p->local_disallowed_sta_mac[i], 6);
		}
	}

	data_model.steering_policy->btm_disallowed_sta_nr  = p->btm_disallowed_sta_nr;
	if (p->btm_disallowed_sta_nr) {
		data_model.steering_policy->btm_disallowed_sta_mac = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(INT8U[6]) * p->btm_disallowed_sta_nr);
		for (i = 0; i < p->btm_disallowed_sta_nr; i++) {
			PLATFORM_MEMCPY(data_model.steering_policy->btm_disallowed_sta_mac[i], p->btm_disallowed_sta_mac[i], 6);
		}
	}

	data_model.steering_policy->radios_nr = p->radios_nr;
	if (p->radios_nr) {
		data_model.steering_policy->policies  = (struct SteeringPolicy *)PLATFORM_MALLOC(sizeof(struct SteeringPolicy) * p->radios_nr);
		for (i = 0; i < p->radios_nr; i++) {
			PLATFORM_MEMCPY(data_model.steering_policy->policies[i].radio_unique_id, p->policies[i].radio_unique_id, 6);
			data_model.steering_policy->policies[i].policy                  = p->policies[i].policy;
			data_model.steering_policy->policies[i].channel_util_threshold  = p->policies[i].channel_util_threshold;
			data_model.steering_policy->policies[i].rcpi_steering_threshold = p->policies[i].rcpi_steering_threshold;
		}
	}

	return 0;
}

INT8U DMcheckBTMSteeringDisallow(INT8U *sta_mac)
{
	INT8U i;
	if (NULL == data_model.steering_policy) {
		return 0;
	}
	for (i = 0; i < data_model.steering_policy->btm_disallowed_sta_nr; i++) {
		if (0 == PLATFORM_MEMCMP(data_model.steering_policy->btm_disallowed_sta_mac[i], sta_mac, 6)) {
			return 1;
		}
	}
	return 0;
}

struct networkDevice *DMlocalNetworkDeviceGet()
{
	if (data_model.network_devices_nr < 1)
		return NULL;

	return &data_model.network_devices[0];
}

void DMclientMacToAssociatedRadioMac(INT8U *client_mac, INT8U *radio_mac)
{

	char **interfaces_names;
	INT8U  interfaces_nr;
	int    i, j;
	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_nr);

	for (i = 0; i < interfaces_nr; i++) {
		struct interfaceInfo *x;
		x = PLATFORM_GET_1905_INTERFACE_INFO(interfaces_names[i]);
		if (NULL == x) {
			PLATFORM_PRINTF_ERROR("[MULTI_AP] Could not retrieve interface info for %s\n", interfaces_names[i]);
			continue;
		}

		if (PLATFORM_STRSTR(x->name, map_vxd_prefix)
			|| PLATFORM_GET_VALID_INTERFACE_NAME(x->name, VALID_ETH_INTERFACES_IN_BRIDGE)
			|| (controller_interface && PLATFORM_STRSTR(x->name, controller_interface))) {
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
			continue;
		}

		for (j = 0; j < x->neighbor_mac_addresses_nr; j++) {
			if (PLATFORM_MEMCMP(client_mac, x->neighbor_list[j].mac_address, MACADDRLEN) == 0) {
				PLATFORM_MEMCPY(radio_mac, x->mac_address, MACADDRLEN);
				PLATFORM_FREE_1905_INTERFACE_INFO(x);
				return;
			}
		}
		PLATFORM_FREE_1905_INTERFACE_INFO(x);
	}
}

void DMsetIntervalTime(INT8U interval)
{
	data_model.interval_time = interval;
}

INT8U DMintervalActionRequired()
{

	if (data_model.interval_time > 0) {
		INT32U now_timestamp;
		now_timestamp = PLATFORM_GET_TIMESTAMP();
		if (((now_timestamp - data_model.last_timestamp) / 1000) >= data_model.interval_time) {
			data_model.last_timestamp = PLATFORM_GET_TIMESTAMP();
			return 1;
		}
	}

	return 0;
}

INT8U DMgetConfiguredState()
{
	return data_model.configure_state;
}

void DMsetConfiguredState(INT8U state)
{
	data_model.configure_state = state;

	return;
}

void DMsetDeviceName(char *device_name)
{

	data_model.device_name = PLATFORM_STRDUP(device_name);
	return;
}

char *DMgetDeviceName()
{
	return data_model.device_name;
}

void DMsetSTPState(INT8U stp_state)
{
	data_model.stp_state = stp_state;
	return;
}

INT8U DMgetSTPState()
{
	return data_model.stp_state;
}

void DMsetControllerIP(char *ip)
{
	strlcpy(data_model.controller_ip, ip, sizeof(data_model.controller_ip));
	return;
}

char *DMgetControllerIP()
{
	if (DMisController())
		return PLATFORM_STRDUP(PLATFORM_GET_IP_ADDRESS());
	PLATFORM_PRINTF_DEBUG("Controller IP: %s\n", data_model.controller_ip);
	return PLATFORM_STRDUP(data_model.controller_ip);
}

void DMsetListeningLoopback(INT8U state)
{
	if (state)
		data_model.listening_lo = 1;
	else
		data_model.listening_lo = 0;
	return;
}

INT8U DMisListeningLoopback()
{
	return data_model.listening_lo;
}

INT8U DMgetWFATestMode()
{
	return data_model.wfa_test_mode;
}

void DMsetWFATestMode(INT8U wfa_test_mode)
{
	data_model.wfa_test_mode = wfa_test_mode;

	return;
}

void DMsetAutoconfigRenewBand(INT8U renew_band)
{
	data_model.autoconfig_renew_band = renew_band;

	return;
}

INT8U DMgetAutoconfigRenewBand()
{
	return data_model.autoconfig_renew_band;
}

INT8U DMupdateAutoconfigVendorTLV(INT8U tlv_nr, struct vendorSpecificTLV *vendor_tlvs, INT8U radio_band)
{
	int i = 0;
	switch (radio_band) {
	case MAP_CONFIG_2G: {
		for (i = 0; i < data_model.autoconfig_vendor_tlv_2g_nr; i++) {
			PLATFORM_FREE(data_model.autoconfig_vendor_tlv_2gs[i].m);
		}
		PLATFORM_SAFE_FREE((void **)&data_model.autoconfig_vendor_tlv_2gs);
		data_model.autoconfig_vendor_tlv_2g_nr = tlv_nr;
		data_model.autoconfig_vendor_tlv_2gs   = vendor_tlvs;
		break;
	}
	case MAP_CONFIG_5GL: {
		for (i = 0; i < data_model.autoconfig_vendor_tlv_5gl_nr; i++) {
			PLATFORM_FREE(data_model.autoconfig_vendor_tlv_5gls[i].m);
		}
		PLATFORM_SAFE_FREE((void **)&data_model.autoconfig_vendor_tlv_5gls);
		data_model.autoconfig_vendor_tlv_5gl_nr = tlv_nr;
		data_model.autoconfig_vendor_tlv_5gls   = vendor_tlvs;
		break;
	}
	case MAP_CONFIG_5GH: {
		for (i = 0; i < data_model.autoconfig_vendor_tlv_5gh_nr; i++) {
			PLATFORM_FREE(data_model.autoconfig_vendor_tlv_5ghs[i].m);
		}
		PLATFORM_SAFE_FREE((void **)&data_model.autoconfig_vendor_tlv_5ghs);
		data_model.autoconfig_vendor_tlv_5gh_nr = tlv_nr;
		data_model.autoconfig_vendor_tlv_5ghs   = vendor_tlvs;
		break;
	}
	default: {
		return 1;
	}
	}
	return 0;
}

struct vendorSpecificTLV *DMgetAutoconfigVendorTLV(INT8U *tlv_nr, INT8U radio_band)
{
	switch (radio_band) {
	case MAP_CONFIG_2G: {
		*tlv_nr = data_model.autoconfig_vendor_tlv_2g_nr;
		return data_model.autoconfig_vendor_tlv_2gs;
	}
	case MAP_CONFIG_5GL: {
		*tlv_nr = data_model.autoconfig_vendor_tlv_5gl_nr;
		return data_model.autoconfig_vendor_tlv_5gls;
	}
	case MAP_CONFIG_5GH: {
		*tlv_nr = data_model.autoconfig_vendor_tlv_5gh_nr;
		return data_model.autoconfig_vendor_tlv_5ghs;
	}
	}
	*tlv_nr = 0;
	return NULL;
}

INT64U DMgetCurrentITC()
{
	return data_model.integrity_transmission_counter;
}

INT64U DMgetNextITC()
{
	data_model.integrity_transmission_counter++;
	if (data_model.integrity_transmission_counter > 0xFFFFFFFFFFFF) {
		data_model.integrity_transmission_counter = 0;
	}

	return DMgetCurrentITC();
}

void DMresetITC()
{
	data_model.integrity_transmission_counter = 0;
}

#ifdef RTK_MULTI_AP_R2
INT8U DMsetChannelScanResult(INT8U channel_scan_result_nr, struct ChannelScanResultTLV *channel_scan_results)
{
	INT8U i, j;
	//free the previously stored one and replace with the new one
	if (data_model.channel_scan_results) {
		for (i = 0; i < data_model.channel_scan_result_nr; i++) {
			if (data_model.channel_scan_results[i].neighbors) {
				for (j = 0; j < data_model.channel_scan_results[i].neighbor_nr; j++) {
					if (data_model.channel_scan_results[i].neighbors[j].channel_bandwidth)
						PLATFORM_SAFE_FREE((void **)&(data_model.channel_scan_results[i].neighbors[j].channel_bandwidth));
					if (data_model.channel_scan_results[i].neighbors[j].ssid)
						PLATFORM_SAFE_FREE((void **)&(data_model.channel_scan_results[i].neighbors[j].ssid));
				}
				PLATFORM_SAFE_FREE((void **)&(data_model.channel_scan_results[i].neighbors));
			}
		}
		PLATFORM_SAFE_FREE((void **)&(data_model.channel_scan_results));
	}

	data_model.channel_scan_result_nr = channel_scan_result_nr;
	data_model.channel_scan_results = channel_scan_results;

	return 0;
}

INT8U DMgetChannelScanResult(INT8U *channel_scan_result_nr, struct ChannelScanResultTLV **channel_scan_results)
{

	if (NULL == data_model.channel_scan_results) {
		return 1;
	}

	*channel_scan_result_nr = data_model.channel_scan_result_nr;
	*channel_scan_results   = data_model.channel_scan_results;

	return 0;
}

void DMsetReportIndChannelScanFlag(INT8U flag)
{
	data_model.report_independent_channel_scan = flag;
}

INT8U DMgetReportIndChannelScanFlag()
{
	return data_model.report_independent_channel_scan;
}

INT8U DMsetChannelScanCapabilities(struct ChannelScanCapabilitiesTLV *t, INT8U *al_mac)
{
	struct networkDevice *network_device;

	network_device = DMnetworkDeviceGet(al_mac);

	if (network_device == NULL) {
		PLATFORM_PRINTF_WARNING("%s - Network device for mac %02x%02x%02x%02x%02x%02x not found!\n",
		                        __FUNCTION__, al_mac[0], al_mac[1], al_mac[2], al_mac[3], al_mac[4], al_mac[5]);
		return 1;
	}

	INT8U i, j;
	//free the old one
	if (network_device->channel_scan_capabilities) {
		for (i = 0; i < network_device->channel_scan_capabilities->radio_nr; i++) {
			if (network_device->channel_scan_capabilities->radios[i].op_classes) {
				for (j = 0; j < network_device->channel_scan_capabilities->radios[i].op_class_nr; j++) {
					if (network_device->channel_scan_capabilities->radios[i].op_classes[j].channel_list) {
						PLATFORM_FREE(network_device->channel_scan_capabilities->radios[i].op_classes[j].channel_list);
					}
				}
				PLATFORM_FREE(network_device->channel_scan_capabilities->radios[i].op_classes);
			}
		}
		PLATFORM_FREE(network_device->channel_scan_capabilities);
	}

	//replace with the new one
	network_device->channel_scan_capabilities           = (struct ChannelScanCapabilitiesTLV *)PLATFORM_MALLOC(sizeof(struct ChannelScanCapabilitiesTLV));
	network_device->channel_scan_capabilities->tlv_type = t->tlv_type;
	network_device->channel_scan_capabilities->radio_nr = t->radio_nr;
	network_device->channel_scan_capabilities->radios   = (struct ChannelScanCapabilitiesRadio *)PLATFORM_MALLOC(t->radio_nr * sizeof(struct ChannelScanCapabilitiesRadio));
	for (i = 0; i < t->radio_nr; i++) {
		PLATFORM_MEMCPY(network_device->channel_scan_capabilities->radios[i].radio_unique_identifier, t->radios[i].radio_unique_identifier, 6);
		network_device->channel_scan_capabilities->radios[i].scan_capability_flags = t->radios[i].scan_capability_flags;
		network_device->channel_scan_capabilities->radios[i].minimum_scan_interval = t->radios[i].minimum_scan_interval;
		network_device->channel_scan_capabilities->radios[i].op_class_nr           = t->radios[i].op_class_nr;
		network_device->channel_scan_capabilities->radios[i].op_classes            = (struct ChannelScanCapabilitiesOpClass *)PLATFORM_MALLOC(t->radios[i].op_class_nr * sizeof(struct ChannelScanCapabilitiesOpClass));

		for (j = 0; j < t->radios[i].op_class_nr; j++) {
			network_device->channel_scan_capabilities->radios[i].op_classes[j].op_class     = t->radios[i].op_classes[j].op_class;
			network_device->channel_scan_capabilities->radios[i].op_classes[j].channel_nr   = t->radios[i].op_classes[j].channel_nr;
			network_device->channel_scan_capabilities->radios[i].op_classes[j].channel_list = (INT8U *)PLATFORM_MALLOC(t->radios[i].op_classes[j].channel_nr * sizeof(INT8U));
			PLATFORM_MEMCPY(network_device->channel_scan_capabilities->radios[i].op_classes[j].channel_list, t->radios[i].op_classes[j].channel_list, t->radios[i].op_classes[j].channel_nr);
		}
	}

	return 0;
}

struct ChannelScanCapabilitiesTLV *DMgetChannelScanCapabilities(INT8U *al_mac)
{
	struct networkDevice *network_device;

	network_device = DMnetworkDeviceGet(al_mac);

	if (network_device == NULL) {
		PLATFORM_PRINTF_WARNING("%s - Network device for mac %02x%02x%02x%02x%02x%02x not found!\n",
		                        __FUNCTION__, al_mac[0], al_mac[1], al_mac[2], al_mac[3], al_mac[4], al_mac[5]);
		return NULL;
	}

	if (network_device->channel_scan_capabilities) {
		return network_device->channel_scan_capabilities;
	}

	return NULL;
}

INT8U DMbufferChannelScanRequest(struct ChannelScanRequestTLV *t)
{
	INT8U i, j;

	int             op_classes_len;
	OP_CLASS *op_classes = PLATFORM_GET_OP_CLASS(&op_classes_len);

	if (NULL == op_classes) {
		PLATFORM_PRINTF_ERROR("[RTK] No operating class is available\n");
		return 1;
	}

	//free the previous one if exist
	if (data_model.buffered_ch_scan_req) {
		if (data_model.buffered_ch_scan_req->target_radios) {
			for (i = 0; i < data_model.buffered_ch_scan_req->target_radio_nr; i++) {
				if (data_model.buffered_ch_scan_req->target_radios[i].target_op_classes) {
					for (j = 0; j < data_model.buffered_ch_scan_req->target_radios[i].target_op_class_nr; j++) {
						if (data_model.buffered_ch_scan_req->target_radios[i].target_op_classes[j].target_channel_nr) {
							PLATFORM_FREE(data_model.buffered_ch_scan_req->target_radios[i].target_op_classes[j].target_channel_list);
						}
					}
					PLATFORM_FREE(data_model.buffered_ch_scan_req->target_radios[i].target_op_classes);
				}
			}
			PLATFORM_FREE(data_model.buffered_ch_scan_req->target_radios);
		}
		PLATFORM_FREE(data_model.buffered_ch_scan_req);
		// data_model.buffered_ch_scan_req = NULL;
	}

	//replace with the new one
	data_model.buffered_ch_scan_req                  = (struct ChannelScanRequestTLV *)PLATFORM_MALLOC(sizeof(struct ChannelScanRequestTLV));
	data_model.buffered_ch_scan_req->tlv_type        = t->tlv_type;
	data_model.buffered_ch_scan_req->flags           = t->flags;
	data_model.buffered_ch_scan_req->target_radio_nr = t->target_radio_nr;
	data_model.buffered_ch_scan_req->target_radios   = (struct ChannelScanTargetRadio *)PLATFORM_MALLOC(data_model.buffered_ch_scan_req->target_radio_nr * sizeof(struct ChannelScanTargetRadio));
	for (i = 0; i < data_model.buffered_ch_scan_req->target_radio_nr; i++) {
		PLATFORM_MEMCPY(data_model.buffered_ch_scan_req->target_radios[i].radio_unique_identifier, t->target_radios[i].radio_unique_identifier, 6);
		data_model.buffered_ch_scan_req->target_radios[i].target_op_class_nr = t->target_radios[i].target_op_class_nr;
		data_model.buffered_ch_scan_req->target_radios[i].target_op_classes  = (struct ChannelScanTargetOpClass *)PLATFORM_MALLOC(data_model.buffered_ch_scan_req->target_radios[i].target_op_class_nr * sizeof(struct ChannelScanTargetOpClass));
		for (j = 0; j < data_model.buffered_ch_scan_req->target_radios[i].target_op_class_nr; j++) {
			data_model.buffered_ch_scan_req->target_radios[i].target_op_classes[j].op_class = t->target_radios[i].target_op_classes[j].op_class;
			if (0 == t->target_radios[i].target_op_classes[j].target_channel_nr) {
				INT8U           n;

				for (n = 0; n < op_classes_len; n++) {
					if (t->target_radios[i].target_op_classes[j].op_class == op_classes[n].op_class) {
						PLATFORM_PRINTF_DETAIL("Channel nr in Channel Scan Request for op class %d is 0. Storing all channels in this op class for this scan request...\n", t->target_radios[i].target_op_classes[j].op_class);
						break;
					}
				}
				if (n == op_classes_len) {
					PLATFORM_PRINTF_WARNING("Channel nr in Channel Scan Request for op class %d is 0, but this op class is invalid. Aborting Channel Scan...\n", t->target_radios[i].target_op_classes[j].op_class);
					PLATFORM_FREE(op_classes);
					return 1;
				}

				data_model.buffered_ch_scan_req->target_radios[i].target_op_classes[j].target_channel_nr   = op_classes[n].channel_array[0];
				data_model.buffered_ch_scan_req->target_radios[i].target_op_classes[j].target_channel_list = (INT8U *)PLATFORM_MALLOC(data_model.buffered_ch_scan_req->target_radios[i].target_op_classes[j].target_channel_nr * sizeof(INT8U));
				PLATFORM_MEMCPY(data_model.buffered_ch_scan_req->target_radios[i].target_op_classes[j].target_channel_list, &op_classes[n].channel_array[1], data_model.buffered_ch_scan_req->target_radios[i].target_op_classes[j].target_channel_nr);
			} else {
				data_model.buffered_ch_scan_req->target_radios[i].target_op_classes[j].target_channel_nr   = t->target_radios[i].target_op_classes[j].target_channel_nr;
				data_model.buffered_ch_scan_req->target_radios[i].target_op_classes[j].target_channel_list = (INT8U *)PLATFORM_MALLOC(data_model.buffered_ch_scan_req->target_radios[i].target_op_classes[j].target_channel_nr * sizeof(INT8U));
				PLATFORM_MEMCPY(data_model.buffered_ch_scan_req->target_radios[i].target_op_classes[j].target_channel_list, t->target_radios[i].target_op_classes[j].target_channel_list, data_model.buffered_ch_scan_req->target_radios[i].target_op_classes[j].target_channel_nr);
			}
		}
	}

	PLATFORM_FREE(op_classes);

	return 0;
}

struct ChannelScanRequestTLV *DMgetBufferedChannelScanRequest()
{
	return data_model.buffered_ch_scan_req;
}

INT8U DMsetChannelScanCompleted(INT8U band)
{
	if (band == _2G) {
		data_model.channel_scan_completed_2g = 1;
	} else if (band == _5GL) {
		data_model.channel_scan_completed_5gl = 1;
	} else if (band == _5GH) {
		data_model.channel_scan_completed_5gh = 1;
	}

	return 0;
}

INT8U DMclearChannelScanCompleted(INT8U band)
{
	if (band == _2G) {
		data_model.channel_scan_completed_2g = 0;
	} else if (band == _5GL) {
		data_model.channel_scan_completed_5gl = 0;
	} else if (band == _5GH) {
		data_model.channel_scan_completed_5gh = 0;
	}

	return 0;
}

INT8U DMgetChannelScanCompleted(INT8U band)
{
	if (band == _2G) {
		if (!PLATFORM_IS_INTERFACE_UP_AND_ON(map_radio_name_24g)) {
			return 1;
		}
		return data_model.channel_scan_completed_2g;
	} else if (band == _5GL) {
		if (!PLATFORM_IS_INTERFACE_UP_AND_ON(map_radio_name_5gl)) {
			return 1;
		}
		return data_model.channel_scan_completed_5gl;
	} else if (band == _5GH) {
		if (!PLATFORM_IS_INTERFACE_UP_AND_ON(map_radio_name_5gh)) {
			return 1;
		}
		return data_model.channel_scan_completed_5gh;
	}

	return 3; //invalid
}

INT8U *DMgetChannelScanResultBuffer()
{
	return data_model.result_buffer;
}

void DMappendChannelScanResultBuffer(int buffer_size, INT8U *content)
{
	if (NULL == data_model.result_buffer) {
		data_model.result_buffer      = (unsigned char *)PLATFORM_MALLOC(buffer_size * sizeof(unsigned char));
		data_model.result_buffer_size = buffer_size;
		PLATFORM_MEMCPY(data_model.result_buffer, content, data_model.result_buffer_size);
	} else {
		data_model.result_buffer = (unsigned char *)PLATFORM_REALLOC(data_model.result_buffer, (buffer_size + data_model.result_buffer_size) * sizeof(unsigned char));
		PLATFORM_MEMCPY(data_model.result_buffer + data_model.result_buffer_size, content, buffer_size);
		data_model.result_buffer_size += buffer_size;
	}
}

void DMclearChannelScanResultBuffer()
{
	PLATFORM_FREE(data_model.result_buffer);
	data_model.result_buffer      = NULL;
	data_model.result_buffer_size = 0;
}

INT8U DMgetRequestStoredResultsFlag()
{
	return data_model.request_stored_results;
}

INT8U DMsetRequestStoredResultsFlag(INT8U flag)
{

	data_model.request_stored_results = flag;

	return 0;
}

INT8U DMsetOnBootScanFlag(INT8U flag)
{
	data_model.on_boot_scan_flag = flag;
	return 0;
}

INT8U DMgetOnBootScanFlag()
{
	return data_model.on_boot_scan_flag;
}

INT8U DMupdateCACRecord(INT8U *radio_identifier, INT8U op_class, INT8U channel, INT8U status)
{
	int i = 0, radio_idx = -1;
	for (i = 0; i < data_model.radio_number; i++) {
		if (0 == PLATFORM_MEMCMP(data_model.cac_radios[i].radio_identifier, radio_identifier, 6)) {
			radio_idx = i;
			break;
		}
	}
	if (-1 == radio_idx) {
		PLATFORM_PRINTF_ERROR("Error: cannot locate radio %02x:%02x:%02x:%02x:%02x:%02x\n", radio_identifier[0], radio_identifier[1], radio_identifier[2], radio_identifier[3], radio_identifier[4], radio_identifier[5]);
		return 1;
	}
	for (i = 0; i < data_model.cac_radios[radio_idx].cac_record_nr; i++) {
		if (op_class == data_model.cac_radios[radio_idx].cac_records[i].op_class && channel == data_model.cac_radios[radio_idx].cac_records[i].channel) {
			data_model.cac_radios[radio_idx].cac_records[i].cac_status     = status;
			data_model.cac_radios[radio_idx].cac_records[i].last_timestamp = PLATFORM_GET_TIMESTAMP();
			return 0;
		}
	}
	data_model.cac_radios[radio_idx].cac_record_nr++;
	data_model.cac_radios[radio_idx].cac_records                   = PLATFORM_REALLOC(data_model.cac_radios[radio_idx].cac_records, sizeof(struct CACStatusRecord) * data_model.cac_radios[radio_idx].cac_record_nr);
	data_model.cac_radios[radio_idx].cac_records[i].channel        = channel;
	data_model.cac_radios[radio_idx].cac_records[i].op_class       = op_class;
	data_model.cac_radios[radio_idx].cac_records[i].cac_status     = status;
	data_model.cac_radios[radio_idx].cac_records[i].last_timestamp = PLATFORM_GET_TIMESTAMP();
	return 0;
}
void DMgetRadioCACStatus(INT8U *radio_identifier, struct RadioCACStatus *radio_cac)
{
	int i = 0;
	for (i = 0; i < data_model.radio_number; i++) {
		if (0 == PLATFORM_MEMCMP(data_model.cac_radios[i].radio_identifier, radio_identifier, 6)) {
			*radio_cac = data_model.cac_radios[i];
			return;
		}
	}
}
void DMgetAllRadioCACStatus(struct RadioCACStatus **cac_status)
{
	*cac_status = data_model.cac_radios;
}

void DMdumpCACStatus()
{
	int i = 0, j = 0;
	for (i = 0; i < data_model.radio_number; i++) {
		PLATFORM_PRINTF_DEBUG("CAC Status for Radio %02x:%02x:%02x:%02x:%02x:%02x\n", data_model.cac_radios[i].radio_identifier[0], data_model.cac_radios[i].radio_identifier[1], data_model.cac_radios[i].radio_identifier[2], data_model.cac_radios[i].radio_identifier[3], data_model.cac_radios[i].radio_identifier[4], data_model.cac_radios[i].radio_identifier[5]);
		for (j = 0; j < data_model.cac_radios[i].cac_record_nr; j++) {
			struct CACStatusRecord *record = &data_model.cac_radios[i].cac_records[j];
			PLATFORM_PRINTF_DEBUG("Op Class: %d, Channel: %d, Status: %d, second elapsed %d\n", record->op_class, record->channel, record->cac_status, PLATFORM_GET_TIMESTAMP() - record->last_timestamp);
		}
	}
}

void DMsetDefault8021QSettings(INT16U primary_vlan_id, INT8U default_pcp)
{
	PLATFORM_PRINTF_DEBUG("Set primary VID to %d, default PCP to %d\n", primary_vlan_id, default_pcp);
	char buf[4] = { 0 };
	char **backhaul_interfaces;
	INT8U  backhaul_interfaces_nr;
	int i = 0;

	data_model.primary_vlan_id = primary_vlan_id;
	data_model.default_pcp     = default_pcp;

	PLATFORM_SNPRINTF(buf, sizeof(buf), "%d", primary_vlan_id);

	backhaul_interfaces = (char **)get_backhaul_interfaces(&backhaul_interfaces_nr);
	for (i = 0; i < backhaul_interfaces_nr; i++) {
		if(PLATFORM_STRSTR(backhaul_interfaces[i], "wlan") && NULL == PLATFORM_STRSTR(backhaul_interfaces[i], map_vxd_prefix)) {
			PLATFORM_SET_MIB(backhaul_interfaces[i], "multiap_vlan_id", buf);
		}
	}
	PLATFORM_FREE(backhaul_interfaces);

	return;
}

void DMgetDefault8021QSettings(INT16U *primary_vlan_id, INT8U *default_pcp)
{
	*primary_vlan_id = data_model.primary_vlan_id;
	*default_pcp     = data_model.default_pcp;

	return;
}

void DMsetTrafficSeparationPolicy(struct TrafficSeparationPolicyTLV *tlv)
{
	INT8U i;

	if (data_model.traffic_separation_policy) {
		free_1905_TLV_structure((INT8U *)data_model.traffic_separation_policy);
	}

	data_model.traffic_separation_policy            = (struct TrafficSeparationPolicyTLV *)PLATFORM_MALLOC(sizeof(struct TrafficSeparationPolicyTLV));
	data_model.traffic_separation_policy->tlv_type  = TLV_TYPE_TRAFFIC_SEPARATION_POLICY;
	data_model.traffic_separation_policy->ssid_info = PLATFORM_MALLOC(sizeof(struct TrafficSeparationSsidInfo) * tlv->ssid_nr);
	data_model.traffic_separation_policy->ssid_nr   = tlv->ssid_nr;

	for (i = 0; i < tlv->ssid_nr; i++) {
		data_model.traffic_separation_policy->ssid_info[i].ssid_length = tlv->ssid_info[i].ssid_length;
		data_model.traffic_separation_policy->ssid_info[i].ssid_name   = (char *)PLATFORM_MALLOC(sizeof(char) * tlv->ssid_info[i].ssid_length);
		PLATFORM_MEMCPY(data_model.traffic_separation_policy->ssid_info[i].ssid_name, tlv->ssid_info[i].ssid_name, tlv->ssid_info[i].ssid_length);
		data_model.traffic_separation_policy->ssid_info[i].vlan_id = tlv->ssid_info[i].vlan_id;
	}

	return;
}

INT16U DMgetVIDbySSID(char* ssid)
{
	INT8U i;
	if(NULL == data_model.traffic_separation_policy) {
		PLATFORM_PRINTF_DEBUG("No VLAN Configuration, use untag.\n");
		return 0;
	}

	for (i = 0; i < data_model.traffic_separation_policy->ssid_nr; i++) {
		if( 0 == PLATFORM_MEMCMP(data_model.traffic_separation_policy->ssid_info[i].ssid_name, ssid, data_model.traffic_separation_policy->ssid_info[i].ssid_length)){
			return data_model.traffic_separation_policy->ssid_info[i].vlan_id;
		}
	}

	PLATFORM_PRINTF_DEBUG("No matching VLAN configuration, use untag.\n");
	return 0;
}
#endif

void DMsetMultiAPProfile(INT8U profile)
{
	char buf[4] = { 0 };
	PLATFORM_SNPRINTF(buf, sizeof(buf), "%d", profile);
	PLATFORM_SET_MIB(map_radio_name_5gh, "multiap_profile", buf);
	PLATFORM_SET_MIB(map_radio_name_5gl, "multiap_profile", buf);
	PLATFORM_SET_MIB(map_radio_name_24g, "multiap_profile", buf);
	data_model.multiap_profile = profile;
	data_model.network_devices[0].multiap_profile  = profile;
}

INT8U DMgetMultiAPProfile()
{
	return data_model.multiap_profile;
}

void DMsetTargetDeviceMultiAPProfile(INT8U *al_mac, INT8U multiap_profile)
{
	struct networkDevice *device = NULL;
	device = DMnetworkDeviceGet(al_mac);
	if (device) {
		if (device->multiap_profile < multiap_profile)
			device->multiap_profile = multiap_profile;
	}
}

INT8U DMgetTargetDeviceMultiAPProfile(INT8U *al_mac)
{
	struct networkDevice *device = NULL;
	device = DMnetworkDeviceGet(al_mac);
	if(device) {
		return device->multiap_profile;
	}

	return 0;
}

struct LocalInterface *DMssidToLocalInterface(char *ssid)
{
	INT8U i;
	if (NULL != ssid) {
		for (i = 0; i < data_model.local_interfaces_nr; i++) {
			char *iface_name = data_model.local_interfaces[i].name;

			if (PLATFORM_GET_VALID_INTERFACE_NAME(iface_name, VALID_ETH_INTERFACES_IN_BRIDGE)
				|| (controller_interface && PLATFORM_STRSTR(iface_name, controller_interface))) {
				continue;
			}
			char interface_ssid[32] = "\0";
			PLATFORM_GET_MIB(iface_name, "ssid", &interface_ssid, 32);
			if (0 == PLATFORM_STRCMP(interface_ssid, ssid)) {
				return &data_model.local_interfaces[i];
			}
		}
	}
	return NULL;
}

void DMsetNeighborRemovalThreshold(INT16U threshold)
{
	data_model.neighbor_remove_threshold = threshold;
}

void DMsetEventFlag(INT32U flag)
{
	data_model.event_flag |= flag;
}

void DMresetEventFlag(INT32U flag)
{
	data_model.event_flag &= ~flag;
}

INT8U DMisEventFlagSet(INT32U flag)
{
	return (flag == (data_model.event_flag & flag));
}

INT8U DMaddNewListeningInterface(char *interface_name)
{
	int i = 0;
	for ( i = 0; i < data_model.listening_interface_nr; i++) {
		if(0 == PLATFORM_STRCMP(data_model.listening_interfaces[i], interface_name)){
			return 1;
		}
	}
	data_model.listening_interface_nr += 1;
	data_model.listening_interfaces = PLATFORM_REALLOC(data_model.listening_interfaces, sizeof(char *) * data_model.listening_interface_nr);
	data_model.listening_interfaces[data_model.listening_interface_nr - 1] = PLATFORM_STRDUP(interface_name);
	return 0;
}

void DMsetMultiAPCommonNetlink(INT8U common_nl)
{
	data_model.is_common_netlink = common_nl;
}

INT8U DMgetMultiAPCommonNetlink()
{
	return data_model.is_common_netlink;
}

void DMsetExcludedInterfaces(char *excluded_interfaces)
{
	data_model.excluded_interfaces = strdup(excluded_interfaces);

	return;
}

INT8U DMisInterfaceExcluded(char *interface_name)
{
	if (!data_model.excluded_interfaces)
		return 0;

	int   buffer_size = PLATFORM_STRLEN(interface_name) + 2;
	char *buffer      = (char *)PLATFORM_MALLOC(buffer_size);

	if (!buffer) {
		PLATFORM_PRINTF_ERROR("Failed to allocate buffer, check interface exclusion failed! \n");
		return 0;
	}

	PLATFORM_MEMSET(buffer, 0, buffer_size);
	int interface_name_len = PLATFORM_STRLEN(interface_name);

	PLATFORM_MEMCPY(buffer, interface_name, interface_name_len);
	buffer[interface_name_len] = ',';
	// buffer is initialized with all 0, so no need to add null term at the next byte.

	if (PLATFORM_STRSTR(data_model.excluded_interfaces, buffer)) {
		PLATFORM_FREE(buffer);
		return 1;
	}

	PLATFORM_FREE(buffer);
	return 0;
}

void DMsetBackhaulSteeringTarget(INT8U *target_mac_address)
{
	PLATFORM_MEMCPY(data_model.backhaul_steering_target_mac, target_mac_address, 6);
	return;
}

void *DMgetBackhaulSteeringTarget()
{
	return data_model.backhaul_steering_target_mac;
}

INT8U DMgetRCPISteeringThreshold(INT8U *radio_mac, INT8U *rcpi_steering_threshold)
{
	INT8U i;
	if (NULL == data_model.steering_policy || NULL == radio_mac)
		return 0;

	for (i = 0; i < data_model.steering_policy->radios_nr; i++) {
		if (0 == PLATFORM_MEMCMP(data_model.steering_policy->policies[i].radio_unique_id, radio_mac, 6)) {
			*rcpi_steering_threshold = data_model.steering_policy->policies[i].rcpi_steering_threshold;
			PLATFORM_PRINTF_DETAIL("[Multiap]Find mac %02X:%02X:%02X:%02X%02X:%02X rcpi_threshold:%u\n",
					radio_mac[0], radio_mac[1], radio_mac[2], radio_mac[3], radio_mac[4], radio_mac[5],
					data_model.steering_policy->policies[i].rcpi_steering_threshold);
			return 1;
		}
	}
	return 0;
}

INT8U *DMgetRootInterfaceMAC(char *interface_name) {
	char *root_interface = NULL;

	if (0 == PLATFORM_STRCMP(interface_name, map_radio_name_5gh)) {
		// this is the radio 5GH
		root_interface = map_radio_name_5gh;
	} else if (PLATFORM_STRSTR(interface_name, map_radio_name_5gh) && PLATFORM_STRSTR(interface_name, map_vap_prefix)) {
		// these are the interfaces that belong to radio 5GH
		root_interface = map_radio_name_5gh;
	} else if (0 == PLATFORM_STRCMP(interface_name, map_radio_name_5gl)) {
		// this is the radio 5GL (Triband only)
		root_interface = map_radio_name_5gl;
	} else if (PLATFORM_STRSTR(interface_name, map_radio_name_5gl) && PLATFORM_STRSTR(interface_name, map_vap_prefix)) {
		// these are the interfaces that belong to radio 5GL
		root_interface = map_radio_name_5gl;
	} else if (0 == PLATFORM_STRCMP(interface_name, map_radio_name_24g)) {
		// this is the radio 2G
		root_interface = map_radio_name_24g;
	} else if (PLATFORM_STRSTR(interface_name, map_radio_name_24g) && PLATFORM_STRSTR(interface_name, map_vap_prefix)) {
		// these are the interfaces that belong to radio 2G
		root_interface = map_radio_name_24g;
	} else {
		return NULL;
	}
	if(root_interface != NULL) {
		PLATFORM_PRINTF_DETAIL("[Multiap]ori interface:%s root interface: %s\n", interface_name, root_interface);
		return DMinterfaceNameToMac(root_interface);
	}
	else
		return NULL;
}

void DMsetByteCounterUnits(INT8U unit)
{
	data_model.byte_counter_units                    = unit;
	data_model.network_devices[0].byte_counter_units = unit;
}

INT8U DMgetByteCounterUnits()
{
	return data_model.byte_counter_units;
}

// For controllers when it receives and needs to know what the agent byte counter units is
INT8U DMgetTargetDeviceByteCounterUnits(INT8U *al_mac)
{
	struct networkDevice *device = NULL;
	device                       = DMnetworkDeviceGet(al_mac);
	if (device) {
		return device->byte_counter_units;
	}
	return 0;
}

void DMsetReportUnsuccessfulAssociation(INT8U report_unsuccessful_association)
{
	data_model.report_unsuccessful_association = report_unsuccessful_association;

	return;
}

INT8U DMgetReportUnsuccessfulAssociation()
{
	return data_model.report_unsuccessful_association;
}

INT8U DMgetMaxBssPerRadio()
{
	return data_model.max_bss_per_radio;
}

void DMsetMaxBssPerRadio(INT8U max_bss_per_radio)
{
	data_model.max_bss_per_radio = max_bss_per_radio;

	return;
}

struct dpp_bootstrap_info *DMgetOwnDPPBootstrapInfo()
{
	return &data_model.own_bootstrap_info;
}

struct dpp_authentication *DMaddDPPAuth(INT8U *chirp, INT8U chirp_len)
{
	struct dpp_authentication *dpp_auth = NULL;

	if (chirp)
		dpp_auth = DMgetDPPAuthenticationObjectByChirp(chirp, chirp_len);

	if (!dpp_auth) {
		dpp_auth = PLATFORM_MALLOC(sizeof(*dpp_auth));
		PLATFORM_MEMSET(dpp_auth, 0, sizeof(*dpp_auth));
		list_add(&data_model.dpp_auth_list, &dpp_auth->list);
		dpp_auth->own_bi = &data_model.own_bootstrap_info;
		dpp_auth->conf = &data_model.configurator;
	}

	return dpp_auth;
}

void DMinsertDPPAuth(struct dpp_authentication *dpp_auth)
{
	if (!dpp_auth)
		return;

	list_add(&data_model.dpp_auth_list, &dpp_auth->list);
	dpp_auth->own_bi = &data_model.own_bootstrap_info;
	dpp_auth->conf = &data_model.configurator;
}

struct dpp_authentication *DMgetDPPAuthenticationObject()
{
	struct map_list *          item = &data_model.dpp_auth_list;
	struct dpp_authentication *auth;

	if (NULL == item->next)
		return NULL;

	auth = (struct dpp_authentication *)item->next;
	return auth;
}

struct dpp_authentication *DMgetDPPAuthenticationObjectByChirp(INT8U *hash_value, INT8U hash_len)
{
	struct map_list *          item = &data_model.dpp_auth_list;
	struct dpp_authentication *auth;
	while (item->next) {
		item = item->next;
		auth = (struct dpp_authentication *)item;
		if (0 == PLATFORM_MEMCMP(auth->peer_bi.pubkey_hash_chirp, hash_value, hash_len)) {
			return auth;
		}
	}
	return NULL;
}

struct dpp_proxy *DMgetChirpProxy(INT8U *chirp, INT8U chirp_len)
{
	struct map_list *          item = &data_model.dpp_chirp_list;
	struct dpp_proxy *proxy;

	if (!chirp)
		return NULL;

	while (item->next) {
		item = item->next;
		proxy = (struct dpp_proxy *)item;
		if (0 == PLATFORM_MEMCMP(proxy->chirp, chirp, chirp_len)) {
			return proxy;
		}
	}
	return NULL;
}

struct dpp_proxy *DMaddDPPChirpProxy(INT8U *chirp, INT8U chirp_len)
{
	struct dpp_proxy *proxy = NULL;

	if (!chirp)
		return NULL;

	proxy = DMgetChirpProxy(chirp, chirp_len);

	if (!proxy) {
		proxy = PLATFORM_MALLOC(sizeof(*proxy));
		PLATFORM_MEMSET(proxy, 0, sizeof(*proxy));
		list_add(&data_model.dpp_chirp_list, &proxy->list);
		PLATFORM_MEMCPY(proxy->chirp, chirp ,chirp_len);
	}

	return proxy;
}

void DMgetDPPEAPOLStateMachine(struct dpp_eapol_sm **sm)
{
	*sm = &data_model.dpp_eapol_state_machine;
	return;
}

void DMsetMaxServicePrioritizationRules(INT8U max_nr_rules)
{
	data_model.max_total_nr_service_prioritization_rules = max_nr_rules;
}

INT8U DMgetMaxServicePrioritizationRules()
{
	return data_model.max_total_nr_service_prioritization_rules;
}

void DMsetServicePrioritizationRulesNr(INT8U sp_rule_nr)
{
	data_model.service_prioritization_rule_nr = sp_rule_nr;
}

INT8U DMgetServicePrioritizationRulesNr()
{
	return data_model.service_prioritization_rule_nr;
}

INT8U DMsetServicePrioritizationRule(struct ServicePrioritizationRuleTLV *tlv)
{
	INT8U                                i;
	struct ServicePrioritizationRuleTLV *tmp = NULL;

	INT8U sp_rule_nr;
	sp_rule_nr = DMgetServicePrioritizationRulesNr();

	// If Add-Remove Rule bit is set to 1 (i.e. add rule)
	if (tlv->add_remove_rule & ADD_REMOVE_RULE_BIT_MASK) {
		// If Always Match bit is set to 1 (i.e. have to add the rule)
		if (tlv->always_match & ALWAYS_MATCH_BIT_MASK) {
			// Check if existing Rule ID exists
			for (i = 0; i < sp_rule_nr; i++) {
				// If Rule ID exists, overwrite
				if (tlv->service_prioritization_rule_id == data_model.service_prioritization_rule[i].service_prioritization_rule_id) {
					data_model.service_prioritization_rule[i].tlv_type        = tlv->tlv_type;
					data_model.service_prioritization_rule[i].add_remove_rule = tlv->add_remove_rule;
					data_model.service_prioritization_rule[i].rule_precedence = tlv->rule_precedence;
					data_model.service_prioritization_rule[i].rule_output     = tlv->rule_output;
					data_model.service_prioritization_rule[i].always_match    = tlv->always_match;
					return 0; // Successful
				}
			}
			// If Rule ID doesn't exist, check if exceed max number of SP rules in capability
			// If exceed max number of SP rules in capability, error
			if ((sp_rule_nr + 1) > DMgetMaxServicePrioritizationRules())
				return 0x02;
			// If doesn't exceed max number of SP rules in capability, add as new rule
			if (0 == sp_rule_nr) {
				data_model.service_prioritization_rule = (struct ServicePrioritizationRuleTLV *)PLATFORM_MALLOC(sizeof(struct ServicePrioritizationRuleTLV));
				DMsetServicePrioritizationRulesNr(sp_rule_nr + 1);
			} else {
				data_model.service_prioritization_rule = (struct ServicePrioritizationRuleTLV *)PLATFORM_REALLOC(data_model.service_prioritization_rule, sizeof(struct ServicePrioritizationRuleTLV) * (sp_rule_nr + 1));
				DMsetServicePrioritizationRulesNr(sp_rule_nr + 1);
			}
			data_model.service_prioritization_rule[sp_rule_nr].tlv_type                       = tlv->tlv_type;
			data_model.service_prioritization_rule[sp_rule_nr].service_prioritization_rule_id = tlv->service_prioritization_rule_id;
			data_model.service_prioritization_rule[sp_rule_nr].add_remove_rule                = tlv->add_remove_rule;
			data_model.service_prioritization_rule[sp_rule_nr].rule_precedence                = tlv->rule_precedence;
			data_model.service_prioritization_rule[sp_rule_nr].rule_output                    = tlv->rule_output;
			data_model.service_prioritization_rule[sp_rule_nr].always_match                   = tlv->always_match;
			return 0; // Successful
		}
		// If Always Match bit is set to 0 (i.e. can choose not to add the rule)
		else {
			return 0; // Successful
		}
	}

	// If Add-Remove Rule bit is set to 0 (i.e. remove rule)
	if (!(tlv->add_remove_rule & ADD_REMOVE_RULE_BIT_MASK)) {
		// Check if existing Rule ID exists
		for (i = 0; i < sp_rule_nr; i++) {
			// If Rule ID exists, remove
			if (tlv->service_prioritization_rule_id == data_model.service_prioritization_rule[i].service_prioritization_rule_id) {
				tmp = &data_model.service_prioritization_rule[i];
				free_1905_TLV_structure((INT8U *)tmp);
				if (i == (sp_rule_nr - 1)) {
					// Last rule, don't need to move anything
				} else {
					// Move the last rule to fill the gap
					data_model.service_prioritization_rule[i] = data_model.service_prioritization_rule[sp_rule_nr - 1];
				}
				DMsetServicePrioritizationRulesNr(sp_rule_nr - 1);
				return 0; // Successful
			}
		}
		// If Rule ID doesn't exist, error
		return 0x01;
	}

	// Should not be able to reach here, put placeholder error code
	return 0x01;
}

/*
	dominant rule is the one taking effect. If a rule always match is 0, it is not saved in our
	implementaion. All rules stored have always match set to 1 and add_remove set to 1. So we only
	need to iterate through and find the one with highest precedence.

	Always update driver if dominant is not NULL, even if redundant.
	Send delete rules to the driver when dominant is NULL, meaning no rule is taking effect right now.

*/
struct ServicePrioritizationRuleTLV * findDominantServicePrioritizationRule()
{
	INT8U                                sp_rule_number = DMgetServicePrioritizationRulesNr();
	struct ServicePrioritizationRuleTLV *dominant       = NULL;

	int i;
	for (i = 0; i < sp_rule_number; i++) { //initialize to the first one with always_match
		if (NULL == dominant && ((data_model.service_prioritization_rule)[i].always_match & ALWAYS_MATCH_BIT_MASK)) {
			dominant = &(data_model.service_prioritization_rule[i]);
			continue;
		}
		if (NULL != dominant) {
			if ((data_model.service_prioritization_rule)[i].always_match & ALWAYS_MATCH_BIT_MASK) {
				if (dominant->rule_precedence < (data_model.service_prioritization_rule)[i].rule_precedence) {
					dominant = &(data_model.service_prioritization_rule[i]);
				}
			}
		}
	}

	return dominant;
}

char **DMgetRadioNames(int *radio_number)
{
	*radio_number = data_model.radio_names_nr;
	return data_model.radio_names;
}

void DMsetDPPEnrollee(INT8U *enrollee_mac_address)
{
	PLATFORM_MEMCPY(data_model.dpp_enrollee_mac, enrollee_mac_address, 6);
	return;
}

void *DMgetDPPEnrollee()
{
	return data_model.dpp_enrollee_mac;
}

void DMsetSecurity(INT8U *al_mac, INT8U agent_security)
{
	struct networkDevice *network_device;
	network_device = DMnetworkDeviceGet(al_mac);

	if (network_device == NULL) {
		PLATFORM_PRINTF_WARNING("%s - Network device for mac %02x%02x%02x%02x%02x%02x not found!\n",
		                        __FUNCTION__, al_mac[0], al_mac[1], al_mac[2], al_mac[3], al_mac[4], al_mac[5]);
		PLATFORM_PRINTF_WARNING("Unable to set 1905 Security ability!\n");
		return;
	}

	network_device->security = agent_security;
}

INT8U DMgetSecurity(INT8U *al_mac)
{
	struct networkDevice *network_device;
	network_device = DMnetworkDeviceGet(al_mac);

	if (network_device == NULL) {
		PLATFORM_PRINTF_WARNING("%s - Network device for mac %02x%02x%02x%02x%02x%02x not found!\n",
		                        __FUNCTION__, al_mac[0], al_mac[1], al_mac[2], al_mac[3], al_mac[4], al_mac[5]);
		PLATFORM_PRINTF_WARNING("Unable to get 1905 Security ability!\n");
		return 0xFF; // error
	}

	return network_device->security;
}

void DMsetM1Delay(INT8U delay)
{
	data_model.m1_delay = delay;
}

INT8U DMgetM1Delay()
{
	return data_model.m1_delay;
}

void DMsetDPPCreds(char *csign_key, char *netaccess_key, char *pp_key, char *signed_connector_1905)
{
	data_model.csign_key             = csign_key;
	data_model.netaccess_key         = netaccess_key;
	data_model.pp_key                = pp_key;
	data_model.signed_connector_1905 = signed_connector_1905;
}

INT8U *DMgetDPPCsignFromFile(INT16U *csign_len)
{
	char  *csign_priv     = NULL;
	int    csign_priv_len = 0;
	INT8U *csign          = NULL;

	FILE *pFile;

	// Get csign from file
	if ((pFile = fopen(DPP_BOOTSTRAP_CSIGN_FILENAME, "re")) != NULL) {
		csign_priv = malloc(sizeof(char) * MAX_ASN1_DER_PRIVKEY_LEN);
		if (NULL != fgets(csign_priv, MAX_ASN1_DER_PRIVKEY_LEN, pFile)) {
			csign_priv_len             = strlen(csign_priv) - 1;
			csign_priv[csign_priv_len] = '\0';
		}
		fclose(pFile);
	}
	if (csign_priv && csign_priv_len) {
		*csign_len = csign_priv_len / 2;
		csign      = (INT8U *)PLATFORM_MALLOC(*csign_len);
		hexstr2bin(csign_priv, (char *)csign, *csign_len);
	}
	PLATFORM_FREE(csign_priv);

	return csign;
}

INT8U *DMgetDPPPpkFromFile(INT16U *ppk_len)
{
	char  *ppk_priv     = NULL;
	int    ppk_priv_len = 0;
	INT8U *ppk          = NULL;

	FILE *pFile;

	// Get ppk from file
	if ((pFile = fopen(DPP_BOOTSTRAP_PPK_FILENAME, "re")) != NULL) {
		ppk_priv = malloc(sizeof(char) * MAX_ASN1_DER_PRIVKEY_LEN);
		if (NULL != fgets(ppk_priv, MAX_ASN1_DER_PRIVKEY_LEN, pFile)) {
			ppk_priv_len           = strlen(ppk_priv) - 1;
			ppk_priv[ppk_priv_len] = '\0';
		}
		fclose(pFile);
	}
	if (ppk_priv && ppk_priv_len) {
		*ppk_len = ppk_priv_len / 2;
		ppk      = (INT8U *)PLATFORM_MALLOC(*ppk_len);
		hexstr2bin(ppk_priv, (char *)ppk, *ppk_len);
	}
	PLATFORM_FREE(ppk_priv);

	return ppk;
}
