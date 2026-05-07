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
#include <unistd.h>
#include "platform.h"

#include "al.h"
#include "al_recv.h"
#include "al_datamodel.h"
#include "al_utils.h"
#include "al_send.h"
#include "al_wsc.h"
#include "al_dpp.h"
#include "al_dpp_eapol.h"
#include "al_dpp_keystore.h"
#include "al_dpp_counters.h"
#include "al_dpp_reconfig.h"

#include "1905_tlvs.h"
#include "1905_cmdus.h"
#include "1905_alme.h"
#include "1905_l2.h"
#include "lldp_tlvs.h"
#include "lldp_payload.h"

#include "utils.h"

#include "platform_interfaces.h"
#include "platform_os.h"

#include "multi_ap_cmdus.h"
#include "multi_ap_cmdus_r2.h"
#include "multi_ap_cmdus_r3.h"
#include "multi_ap_tlvs.h"
#include "multi_ap_tlvs_r2.h"
#include "multi_ap_tlvs_r3.h"
#include "multi_ap_tlvs_r4.h"

#include "api_response.h"
#include "map_utils.h"

#include "global_settings.h"

#define MAX_RECV_M2_CONFIG_NR 16
#define MAX_RECV_BSS_CONF_OBJ_NR 16
#define MAX_RECV_BSS_AP_RADIO_NR 4

#define SCHEDULE_BSS_CONFIG_RESULT_FILE "/tmp/map_bss_conf_result"

#define RTK_DEFAULT_DECRYPT_FAILURE_THRESHOLD 10

void _updateControllerStatus(INT8U *controller_mac, char *interface_name)
{
	// INT8U *controller_mac_old = DMcontrollerMacGet();
	// if (0 == PLATFORM_MEMCMP(controller_mac_old, controller_mac, 6)) {
	// 	return;
	// }
	DMcontrollerMacSet(controller_mac);
	INT8U **tlv_list;
	tlv_list    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *));
	tlv_list[0] = NULL;
	api_controller_change_notification(CONTROLLER_FOUND, interface_name, controller_mac);
	PLATFORM_FREE(tlv_list);
	return;
}

#ifdef RTK_MULTI_AP_R3
// This function writes the MAC address of the current controller to
// "SCHEDULE_BSS_CONFIG_RESULT_FILE", such that the daemon can send
// the BSS Configuration Result CMDU to the controller right after it
// is restarted because of running map_reinit during configuration.
static void _scheduleBSSConfigResult(INT8U *controller_mac)
{
	FILE *fp = fopen(SCHEDULE_BSS_CONFIG_RESULT_FILE, "w+e");
	if (NULL != fp) {
		char mac_addr_hex[16] = { 0 };
		bin2hexstr((char *)controller_mac, mac_addr_hex, 6);
		fputs(mac_addr_hex, fp);
		fclose(fp);
		PLATFORM_PRINTF_INFO("Scheduled BSS config result (mac: %s)\n", mac_addr_hex);
	}
}

// This function checks if "SCHEDULE_BSS_CONFIG_RESULT_FILE" exists.
// If it does exist, that means BSS Configuration Result has been scheduled
// before the daemon restarts, so we need to send it in this function.
static void _runScheduledBSSConfigResult(INT8U *controller_mac, char *interface)
{
	if (MULTI_AP_PROFILE_3 <= DMgetMultiAPProfile()) {
		FILE *fp = fopen(SCHEDULE_BSS_CONFIG_RESULT_FILE, "re");
		if (NULL != fp) {
			char mac_addr_hex[16] = { 0 };
			char mac_addr[6]      = { 0 };
			fgets(mac_addr_hex, sizeof(mac_addr_hex), fp);
			hexstr2bin(mac_addr_hex, mac_addr, 6);
			if (PLATFORM_MEMCMP(mac_addr, controller_mac, 6) == 0) {
				PLATFORM_PRINTF_INFO("Send BSS config result through %s\n", interface);
				sendBSSConfigResultPacket(interface, getNextMid(), controller_mac);
				fclose(fp);
				unlink(SCHEDULE_BSS_CONFIG_RESULT_FILE);
			} else {
				PLATFORM_PRINTF_WARNING("Mismatch mac addr for BSS config result!\n");
				fclose(fp);
			}
		}
	}
}

void build1905EncapDPPTLV(struct Encap1905DPPTLV *encap_1905_dpp_tlv, INT8U *dpp_frame, INT16U dpp_frame_len)
{
	PLATFORM_MEMSET(encap_1905_dpp_tlv, 0, sizeof(struct Encap1905DPPTLV));
	encap_1905_dpp_tlv->tlv_type = TLV_TYPE_1905_ENCAP_DPP;
	encap_1905_dpp_tlv->enrollee_mac_address_present |= BIT(7);
	PLATFORM_MEMCPY(encap_1905_dpp_tlv->dest_sta_mac_address, DMgetDPPEnrollee(), 6);

	if (WLAN_PA_VENDOR_SPECIFIC != dpp_frame[0]) {
		encap_1905_dpp_tlv->enrollee_mac_address_present |= BIT(5);
		encap_1905_dpp_tlv->frame_type = 0xFF;
	} else {
		encap_1905_dpp_tlv->frame_type = dpp_frame[6];
	}

	encap_1905_dpp_tlv->encapsulated_frame_len = dpp_frame_len;
	encap_1905_dpp_tlv->encapsulated_frame     = (INT8U *)PLATFORM_MALLOC(dpp_frame_len);

	PLATFORM_MEMCPY(encap_1905_dpp_tlv->encapsulated_frame, dpp_frame, dpp_frame_len);
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Public functions (exported only to files in this same folder)
////////////////////////////////////////////////////////////////////////////////
void triggerAutoconfig(char *receiving_interface_name)
{
	INT8U band_to_configure = 0xFF;
	INT8U configured_band_flag;

	char *ifs_name = "";

	while (1) {
		band_to_configure = 0xFF;

		INT8U mask_2g   = (INTERFACE_BAND_2G | (INTERFACE_BAND_2G << 4));
		INT8U mask_5g_l = (INTERFACE_BAND_5GL | (INTERFACE_BAND_5GL << 4));
		INT8U mask_5g_h = (INTERFACE_BAND_5GH | (INTERFACE_BAND_5GH << 4));

		configured_band_flag = DMgetConfiguredState();
		PLATFORM_PRINTF_DEBUG("configured_band_flag %02x\n", configured_band_flag);

		if (0x00 == (configured_band_flag & mask_2g)) {
			PLATFORM_PRINTF_INFO("[AP Search] 2.4GHz unconfigured.\n");
			band_to_configure = IEEE80211_FREQUENCY_BAND_2_4_GHZ;
			DMsetConfiguredState(configured_band_flag | mask_2g);
			ifs_name = map_radio_name_24g;
		} else if (0x00 == (configured_band_flag & mask_5g_l)) {
			PLATFORM_PRINTF_INFO("[AP Search] 5GHz Low unconfigured.\n");
			band_to_configure = IEEE80211_FREQUENCY_BAND_5_GHZ;
			DMsetConfiguredState(configured_band_flag | mask_5g_l);
			if (!DMisTribandDevice()) {
				DMsetConfiguredState(configured_band_flag | mask_5g_l | mask_5g_h);
			}
			ifs_name = map_radio_name_5gl;
		} else if (0x00 == (configured_band_flag & mask_5g_h)) {
			if (DMisTribandDevice()) {
				PLATFORM_PRINTF_INFO("[AP Search] 5GHz High unconfigured.\n");
				band_to_configure = IEEE80211_FREQUENCY_BAND_5_GHZ;
				DMsetConfiguredState(configured_band_flag | mask_5g_h);
				ifs_name = map_radio_name_5gh;
			} else {
				DMsetConfiguredState(configured_band_flag | INTERFACE_BAND_5GH);
			}
		}

		if (band_to_configure == 0xFF) {
			return;
		}

		INT8U *controller_mac = DMcontrollerMacGet();

		// Check our local interfaces, looking for an unconfigured AP
		// interface that matches the freq band.
		// If one is found, send the response.
		//

		INT8U *m1;
		INT16U m1_size;
		void * key;

		PLATFORM_PRINTF_DEBUG("Interface %s is an unconfigured AP and uses the same freq band. Sending WSC-M1...\n", ifs_name);

		// Obtain WSC-M1 and send the WSC TLV
		//
		INT8U result = wscBuildM1(ifs_name, &m1, &m1_size, &key);

		if (result) {
			if (0 == send1905APAutoconfigurationWSCPacket(receiving_interface_name, getNextMid(), controller_mac, &m1, &m1_size, 1, DMinterfaceNameToMac(ifs_name), ifs_name, NULL, 0, NULL)) {
				PLATFORM_PRINTF_WARNING("Could not send 'AP autoconfiguration WSC-M1' message\n");
			}
		} else {
			PLATFORM_PRINTF_ERROR("Error building M1 for interface %s, skip send wsc packet.\n", ifs_name);
		}
		api_send_customized_status_notification(STATUS_M1_SEND);
	}
}

void delayTriggerAutoconfig(INT32U seconds, INT8U al_queue_id)
{
	// Set status to configuring all bands
	const INT8U configured_band_flag = DMgetConfiguredState();
	const INT8U mask_2g              = (INTERFACE_BAND_2G | (INTERFACE_BAND_2G << 4));
	const INT8U mask_5g_l            = (INTERFACE_BAND_5GL | (INTERFACE_BAND_5GL << 4));
	const INT8U mask_5g_h            = (INTERFACE_BAND_5GH | (INTERFACE_BAND_5GH << 4));
	DMsetConfiguredState(configured_band_flag | mask_2g | mask_5g_l | mask_5g_h);

	DMsetEventFlag(FLAG_DELAYED_AUTOCONFIG);

	// Schedule AP-Autoconfig WSC
	PLATFORM_PRINTF_INFO("Going to run WSC after %d second(s)\n", seconds);
	struct eventTimeOut aux;
	aux.timeout_ms = seconds * 1000;
	aux.token      = TIMER_TOKEN_AUTOCONFIG;
	if (PLATFORM_REGISTER_QUEUE_EVENT(al_queue_id, PLATFORM_QUEUE_EVENT_TIMEOUT, &aux) == 0)
		PLATFORM_PRINTF_WARNING("Cannot register TIMER_TOKEN_AUTOCONFIG\n");
}

#ifdef RTK_MULTI_AP_R4
void delayTriggerReconfig(INT32U seconds, INT8U al_queue_id)
{
	// Schedule Reconfig Announcement
	PLATFORM_PRINTF_DETAIL("Chirping reconfig announcement in %d second(s)\n", seconds);
	struct eventTimeOut aux;
	aux.timeout_ms = seconds * 1000;
	aux.token      = TIMER_TOKEN_RECONFIG;
	if (0 == PLATFORM_REGISTER_QUEUE_EVENT(al_queue_id, PLATFORM_QUEUE_EVENT_TIMEOUT, &aux)) {
		PLATFORM_PRINTF_ERROR("Could not register TIMER_TOKEN_RECONFIG\n");
	}
}

INT8U triggerDppPeerDiscovery(char *receiving_interface_name, INT8U *src_addr)
{
	struct dpp_authentication *dpp_auth              = NULL;
	struct dpp_config_obj *    conf_obj              = NULL;
	INT8U                      ret                   = 0;
	INT8U *                    peer_disc_req_msg     = NULL;
	INT32U                     peer_disc_req_msg_len = 0;

	// No need to perform peer discovery if there is already gtk for the controller
	if (dpp_keystore_get_ptk(src_addr))
		return ret;

	DMsetEventFlag(FLAG_DELAYED_AUTOCONFIG); // Block WSC autoconfig
	dpp_auth = DMgetDPPAuthenticationObject();
	conf_obj = dpp_find_config_obj(dpp_auth, "map", "*", DPP_NETROLE_MAP_AGENT);
	ret      = dpp_build_peer_disc_req(&conf_obj->conn, &peer_disc_req_msg, &peer_disc_req_msg_len);
	if (ret == 1 && peer_disc_req_msg != NULL && peer_disc_req_msg_len != 0) {
		if (0 == send1905DirectEncapDPPPacket(receiving_interface_name, getNextMid(), src_addr, peer_disc_req_msg, peer_disc_req_msg_len)) {
			PLATFORM_PRINTF_INFO("Could not send 'Direct Encap DPP' message\n");
			ret = 0;
		}
	} else {
		ret = 0;
	}

	PLATFORM_FREE(peer_disc_req_msg);
	return ret;
}
#endif // RTK_MULTI_AP_R4

void _free_radio_config(struct radio_config_data *radio_config)
{
	int i = 0;
	for (i = 0; i < radio_config->bss_data_nr; i++) {
		PLATFORM_FREE(radio_config->bss_data[i].ssid);
		PLATFORM_FREE(radio_config->bss_data[i].network_key);
		PLATFORM_FREE(radio_config->bss_data[i].signed_connector);
	}

	if (radio_config->bss_data_nr) {
		radio_config->bss_data_nr = 0;
		PLATFORM_FREE(radio_config->bss_data);
	}
}

// Match configs in 'config' with 'src_addr' and 'band' for
// autoconfiguration (both WSC M2 and DPP)
void getConfigsForAutoconfig(INT8U band, INT8U *src_addr,
                             struct configData *config_out[],
                             INT8U *            config_out_nr,
                             INT8U              max_bss_nr)
{
	struct configData *config_data   = NULL;
	INT8U              config_number = 0;

	static struct configData teardown_config = {
		.ssid            = "Teardown",
		.network_key     = "Teardown",
		.auth_type       = WPS_AUTH_WPA2PSK,
		.encryption_type = WPS_ENCR_AES,
		.network_type    = MULTI_AP_TEARDOWN_BIT,
		.al_id           = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		.vid             = 0
	};

	const INT8U zeros[6] = { 0,0,0,0,0,0 };

	INT8U i;

	// Select config for specific freq band
	if ((INTERFACE_BAND_5GL & band)
	    && (0 != config.config_number_5gl || 0 == (INTERFACE_BAND_5GH & band))) {
		PLATFORM_PRINTF_DEBUG("Getting config data for 5GL...\n");
		config_data   = config.config_data_5gl;
		config_number = config.config_number_5gl;
	} else if (INTERFACE_BAND_5GH & band) {
		PLATFORM_PRINTF_DEBUG("Getting config data for 5GH...\n");
		config_data   = config.config_data_5gh;
		config_number = config.config_number_5gh;
	} else if (INTERFACE_BAND_2G == band) {
		PLATFORM_PRINTF_DEBUG("Getting config data for 24G...\n");
		config_data   = config.config_data_24g;
		config_number = config.config_number_24g;
	} else {
		PLATFORM_PRINTF_ERROR("%s: Unidentified band!!!! (%d)\n", __FUNCTION__, band);
		*config_out_nr = 0;
		return;
	}

	// Find matched profiles in config_data
	// Matched: (al_id == src_addr) OR (al_id == 00:00:00:00:00:00)
	for (i = 0; i < config_number; i++) {
		if (*config_out_nr < max_bss_nr) {
			if (PLATFORM_MEMCMP(config_data[i].al_id, src_addr, 6) == 0
			    || PLATFORM_MEMCMP(config_data[i].al_id, zeros, 6) == 0) {
				config_out[*config_out_nr] = &config_data[i];
				(*config_out_nr)++;
			}
		} else {
			PLATFORM_PRINTF_WARNING("Number of matched BSS exceeded max_bss_nr!\n");
			*config_out_nr = max_bss_nr;
			break;
		}
	}

	// Only include 1 profile if MULTI_AP_TEARDOWN_BIT is set
	for (i = 0; i < (*config_out_nr); i++) {
		if (config_out[i]->network_type & MULTI_AP_TEARDOWN_BIT) {
			config_out[0]  = &teardown_config;
			*config_out_nr = 1;
			break;
		}
	}

	// Sanity checks for config_out_nr
	if (*config_out_nr == 0) {
		PLATFORM_PRINTF_WARNING("No matched config data, teardown\n");
		config_out[0]  = &teardown_config;
		*config_out_nr = 1;
	}

	// Print matched profiles
	PLATFORM_PRINTF_DEBUG("There are %d config data matches this radio\n",
	                      *config_out_nr);
	for (i = 0; i < *config_out_nr; i++) {
		PLATFORM_PRINTF_INFO("SSID: %s\n", config_out[i]->ssid);
		PLATFORM_PRINTF_INFO("KEY: %s\n", config_out[i]->network_key);
		PLATFORM_PRINTF_INFO("Auth: %02x\n", config_out[i]->auth_type);
		PLATFORM_PRINTF_INFO("Encrypt: %02x\n", config_out[i]->encryption_type);
		PLATFORM_PRINTF_INFO("BSS Type: %02x\n", config_out[i]->network_type);
		if (config_out[i]->vid)
			PLATFORM_PRINTF_INFO("VID: %d\n", config_out[i]->vid);
	}
}

INT8U getBackhaulConfig(INT8U band, INT8U *src_addr,
                       struct configData **config_out)
{
	struct configData *config_data   = NULL;
	INT8U              config_number = 0;
	const INT8U        zeros[6]      = { 0,0,0,0,0,0 };
	INT8U              i;
	INT8U              result = 1;

	// Select config for specific freq band
	if ((INTERFACE_BAND_5GL & band)
	    && (0 != config.config_number_5gl || 0 == (INTERFACE_BAND_5GH & band))) {
		PLATFORM_PRINTF_DEBUG("Getting config data for 5GL...\n");
		config_data   = config.config_data_5gl;
		config_number = config.config_number_5gl;
	} else if (INTERFACE_BAND_5GH & band) {
		PLATFORM_PRINTF_DEBUG("Getting config data for 5GH...\n");
		config_data   = config.config_data_5gh;
		config_number = config.config_number_5gh;
	} else if (INTERFACE_BAND_2G == band) {
		PLATFORM_PRINTF_DEBUG("Getting config data for 24G...\n");
		config_data   = config.config_data_24g;
		config_number = config.config_number_24g;
	} else {
		PLATFORM_PRINTF_ERROR("%s: Unidentified band!!!! (%d)\n", __FUNCTION__, band);
		return result;
	}

	// Find matched profiles in config_data
	// Matched: (al_id == src_addr) OR (al_id == 00:00:00:00:00:00)
	for (i = 0; i < config_number; i++) {
		if ((PLATFORM_MEMCMP(config_data[i].al_id, src_addr, 6) == 0
		     || PLATFORM_MEMCMP(config_data[i].al_id, zeros, 6) == 0)
		    && (config_data[i].network_type & MULTI_AP_BACKHAUL_BSS_BIT)) {
			*config_out = &config_data[i];
			result = 0;
			PLATFORM_PRINTF_DEBUG("Matching backhaul config found for band %d\n", band);
			break;
		}
	}

	return result;
}

INT8U process1905Cmdu(struct CMDU *c, char *receiving_interface_name, INT8U *src_addr)
{
	INT8U ret = PROCESS_CMDU_OK;
	if (NULL == c) {
		return PROCESS_CMDU_KO;
	}

	switch (c->message_type) {
	case CMDU_TYPE_TOPOLOGY_DISCOVERY: {
		// When a "topology discovery" is received we must update our
		// internal database (that keeps track of which AL MACs and
		// interface MACs are seen on each interface) and send a "topology
		// query" message asking for more details.

		INT8U *p;
		INT8U  i;

		INT8U dummy_mac_address[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

		INT8U al_mac_address[6];
		INT8U mac_address[6];

		INT8U  first_discovery = 0;
		INT32U ellapsed;

		PLATFORM_MEMCPY(al_mac_address, dummy_mac_address, 6);
		PLATFORM_MEMCPY(mac_address, dummy_mac_address, 6);

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_TOPOLOGY_DISCOVERY (%s) MID: %04x\n", receiving_interface_name, c->message_id);

		// We need to update the data model structure, which keeps track
		// of local interfaces, neighbors, and neighbors' interfaces, and
		// what type of discovery messages ("topology discovery" and/or
		// "bridge discovery") have been received on each link.

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("Malformed structure.");
			break;
		}

		// First, extract the AL MAC and MAC addresses of the interface
		// which transmitted this "topology discovery" message
		//
		i = 0;
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_VENDOR_SPECIFIC: {
				break;
			}
			case TLV_TYPE_AL_MAC_ADDRESS_TYPE: {
				struct alMacAddressTypeTLV *t = (struct alMacAddressTypeTLV *)p;

				PLATFORM_MEMCPY(al_mac_address, t->al_mac_address, 6);

				break;
			}
			case TLV_TYPE_MAC_ADDRESS_TYPE: {
				struct macAddressTypeTLV *t = (struct macAddressTypeTLV *)p;

				PLATFORM_MEMCPY(mac_address, t->mac_address, 6);

				break;
			}
			case TLV_TYPE_MIC: {
				struct MICTLV *t = (struct MICTLV *)p;
				PLATFORM_PRINTF_DEBUG("ITC: %llu\n", t->integrity_transmission_counter);
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		// Make sure that both the AL MAC and MAC addresses were contained
		// in the CMDU
		//
		if (0 == PLATFORM_MEMCMP(al_mac_address, dummy_mac_address, 6) || 0 == PLATFORM_MEMCMP(mac_address, dummy_mac_address, 6)) {
			PLATFORM_PRINTF_WARNING("More TLVs were expected inside this CMDU\n");
			return PROCESS_CMDU_KO;
		}

		PLATFORM_PRINTF_DETAIL("AL MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n", al_mac_address[0], al_mac_address[1], al_mac_address[2], al_mac_address[3], al_mac_address[4], al_mac_address[5]);
		PLATFORM_PRINTF_DETAIL("MAC    address = %02x:%02x:%02x:%02x:%02x:%02x\n", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);

		struct networkDevice *device = DMnetworkDeviceGet(al_mac_address);

		if (device != NULL) {
			device->discovery_timestamp = PLATFORM_GET_TIMESTAMP();
		}

		// Next, update the data model
		//
		if (PLATFORM_MEMCMP(al_mac_address, DMalMacGet(), 6) != 0 && (1 == (first_discovery = DMupdateDiscoveryTimeStamps(receiving_interface_name, al_mac_address, mac_address, TIMESTAMP_TOPOLOGY_DISCOVERY, &ellapsed)))) {
#ifdef SPEED_UP_DISCOVERY
			// If the data model did not contain an entry for this neighbor,
			// "manually" (ie. "out of cycle") send a "Topology Discovery"
			// message on the receiving interface.
			// This will speed up the network discovery process, so that
			// the new node does not have to wait until our "60 seconds"
			// timer expires for him to "discover" us
			//
			if(!DMgetWFATestMode()) {
				PLATFORM_PRINTF_DETAIL("Is this a new node? Re-scheduling a Topology Discovery so that he 'discovers' us\n");

				if (0 == send1905TopologyDiscoveryPacket(receiving_interface_name, getNextMid())) {
					PLATFORM_PRINTF_WARNING("Could not send 1905 topology discovery message\n");
				}
			}
#endif
		}

		if (DMisController()) {
			// Finally, query the advertising neighbor for (much) more detailed
			// information (but only if we haven't recently queried it!)
			// This will make the other end send us a
			// CMDU_TYPE_TOPOLOGY_RESPONSE message, which we will later
			// process.
			//
			if (
			    0 == DMnetworkDeviceInfoNeedsUpdate(al_mac_address) || // Recently received a Topology Response or....
			    (2 == first_discovery && ellapsed < 5000)              // ...recently (<5 seconds) received a Topology Discovery

			) {
				// The first condition prevents us from re-asking (ie.
				// re-sending "Topology Queries") to one same node (we already
				// knew of) faster than once every minute.
				//
				// The second condition prevents us from flooding new nodes
				// (from which we haven't received a "Topology Response" yet)
				// with "Topology Queries" faster than once every 5 seconds)
				//
				break;
			}
		}

		if (0 == send1905TopologyQueryPacket(receiving_interface_name, getNextMid(), al_mac_address, 1)) {
			PLATFORM_PRINTF_WARNING("Could not send 'topology query' message\n");
		}

		break;
	}
	case CMDU_TYPE_TOPOLOGY_NOTIFICATION: {
		// When a "topology notification" is received we must send a new
		// "topology query" to the sender.
		// The "sender" AL MAC address is contained in the unique TLV
		// embedded in the just received "topology notification" CMDU.

		INT8U *p;
		INT8U  i;

		INT8U dummy_mac_address[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

		INT8U al_mac_address[6];

		PLATFORM_MEMCPY(al_mac_address, dummy_mac_address, 6);

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_TOPOLOGY_NOTIFICATION (%s) MID %04x\n", receiving_interface_name, c->message_id);

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("Malformed structure.");
			break;
		}

		api_send_tlvs_response(MAP_TOPOLOGY_NOTIFICATION_ELEMENT, src_addr, c->list_of_TLVs);

		// Extract the AL MAC addresses of the interface which transmitted
		// this "topology notification" message
		//
		i = 0;
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_AL_MAC_ADDRESS_TYPE: {
				struct alMacAddressTypeTLV *t = (struct alMacAddressTypeTLV *)p;

				PLATFORM_MEMCPY(al_mac_address, t->al_mac_address, 6);

				break;
			}
			case TLV_TYPE_CLIENT_ASSOCIATION_EVENT: {
				struct ClientAssociationEventTLV *t = (struct ClientAssociationEventTLV *)p;

				PLATFORM_PRINTF_INFO("[PLATFORM] client "
					"MAC address %.2x:%.2x:%.2x:%.2x:%.2x:%.2x "
					"%s "
					"BSSID %.2x:%.2x:%.2x:%.2x:%.2x:%.2x \n",
					t->mac_address[0], t->mac_address[1],
					t->mac_address[2], t->mac_address[3],
					t->mac_address[4], t->mac_address[5],
					CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_JOIN == t->event ? "Join" : "Leave",
					t->bssid[0], t->bssid[1], t->bssid[2], t->bssid[3], t->bssid[4], t->bssid[5]);

				if (CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_JOIN == t->event) {
					INT8U result = delete_agent(t->mac_address, t->bssid, 1, BACKHAUL_BSS | FRONTHAUL_BSS, NULL);
					if (result) {
						PLATFORM_PRINTF_DEBUG("Disassoc %.2x:%.2x:%.2x:%.2x:%.2x:%.2x with %d\n",
							t->mac_address[0], t->mac_address[1],
							t->mac_address[2], t->mac_address[3],
							t->mac_address[4], t->mac_address[5],
							result);
					}
				}

				break;
			}
			case TLV_TYPE_VENDOR_SPECIFIC: {
				// PLATFORM_PRINTF_INFO("<-- TLV_TYPE_VENDOR_SPECIFIC \n");
				break;
			}
			case TLV_TYPE_MIC: {
				struct MICTLV *t = (struct MICTLV *)p;
				PLATFORM_PRINTF_DEBUG("ITC: %llu\n", t->integrity_transmission_counter);
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		// Make sure that both the AL MAC and MAC addresses were contained
		// in the CMDU
		//
		if (0 == PLATFORM_MEMCMP(al_mac_address, dummy_mac_address, 6)) {
			PLATFORM_PRINTF_WARNING("More TLVs were expected inside this CMDU\n");
			return PROCESS_CMDU_KO;
		}

		if (0 == c->relay_indicator) {
			PLATFORM_PRINTF_INFO("Unicast CMDU_TYPE_TOPOLOGY_NOTIFICATION, need ACK!\n");
			if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, al_mac_address, 0, NULL)) {
				PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_TOPOLOGY_NOTIFICATION MID: (%x) is failed\n", c->message_id);
			}
			PLATFORM_PRINTF_INFO("[Multi-AP] Sending ACK for CMDU_TYPE_TOPOLOGY_NOTIFICATION MID: (%x)\n", c->message_id);
		}

		PLATFORM_PRINTF_DETAIL("AL MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n", al_mac_address[0], al_mac_address[1], al_mac_address[2], al_mac_address[3], al_mac_address[4], al_mac_address[5]);

#ifdef SPEED_UP_DISCOVERY
		// We will send a topology discovery back. Why is this useful?
		// Well... imagine a node that has just entered the secure network.
		// The first thing this node will do is sending a
		// "topology notification" which, when received by us, will trigger
		// a "topology query".
		// However, unless we send a "topology discovery" back, the new node
		// will not query us for a while (until we actually send our
		// periodic "topology discovery").
		//
		if (PLATFORM_MEMCMP(src_addr, DMalMacGet(), 6) != 0) {
			PLATFORM_PRINTF_DETAIL("Is this a new node? Re-scheduling a Topology Discovery so that he 'discovers' us\n");

			if (0 == send1905TopologyDiscoveryPacket(receiving_interface_name, getNextMid())) {
				PLATFORM_PRINTF_WARNING("Could not send 1905 topology discovery message\n");
			}
		}
#endif
		// Finally, query the informing node.
		// Note that we don't have to check (as we did in the "topology
		// discovery" case) if we recently updated the data model or not.
		// This is because a "topology notification" *always* implies
		// network changes and thus the device must always be (re)-queried.
		//
		if (0 == send1905TopologyQueryPacket(receiving_interface_name, getNextMid(), al_mac_address, 1)) {
			PLATFORM_PRINTF_WARNING("Could not send 'topology query' message\n");
		}

		break;
	}
	case CMDU_TYPE_TOPOLOGY_QUERY: {
		// When a "topology query" is received we must obtain a series of
		// information from the platform and then package and send it back
		// in a "topology response" message.

		INT8U *dst_mac;
		INT8U *p;
		INT8U i, multiap_profile;

		struct networkDevice *device = NULL;
		device = DMnetworkDeviceGet(src_addr);
		i = 0;
		multiap_profile = MULTI_AP_PROFILE_1;

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_TOPOLOGY_QUERY (%s) MID %04x\n", receiving_interface_name, c->message_id);

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
#ifdef RTK_MULTI_AP_R2
			case TLV_TYPE_MULTIAP_PROFILE: {
				struct MultiAPProfileTLV *profile_tlv = (struct MultiAPProfileTLV *) p;
				if (MULTI_AP_PROFILE_2 <= profile_tlv->profile) {
					multiap_profile = profile_tlv->profile;
				}
				break;
			}
#endif
			default: {
				PLATFORM_PRINTF_WARNING("Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		if (device == NULL) {
			struct deviceInformationTypeTLV *     info = NULL;
			info = PLATFORM_MALLOC(sizeof(struct deviceInformationTypeTLV));
			PLATFORM_MEMCPY(info->al_mac_address, src_addr, 6);
			info->local_interfaces_nr = 0;
			info->local_interfaces = NULL;

			DMupdateNetworkDeviceInfo(src_addr, receiving_interface_name,
				1, info,
				0, NULL, 0,
				0, NULL, 0,
				0, NULL, 0,
				0, NULL,
				0, NULL,
				0, NULL);
		}

		DMsetTargetDeviceMultiAPProfile(src_addr, multiap_profile);

		// We must send the response to the AL MAC of the node who sent the
		// query, however, this AL MAC is *not* contained in the query.
		// The only thing we can do at this point is try to search our AL
		// neighbors data base for a matching MAC.
		//

		p = DMmacToAlMac(src_addr);

		if (NULL == p) {
			// The standard says we should always send to the AL MAC
			// address, however, in these cases, instead of just dropping
			// the packet, sending the response to the 'src' address from
			// the TOPOLOGY QUERY seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the TOPOLOGY QUERY (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = p;
		}

		if (0 == send1905TopologyResponsePacket(receiving_interface_name, c->message_id, dst_mac)) {
			PLATFORM_PRINTF_WARNING("Could not send 'topology response' message\n");
		}

#ifdef RTK_MULTI_AP_R3
		// Introduction with dpp creds between agents (Introduction between agent and controller should be triggered in autoconfiguration)
		if (NULL == dpp_keystore_get_gtk()) {
			PLATFORM_PRINTF_DEBUG("Agent should onboard first, ignore this introduction request.\n");
		} else if (dpp_keystore_get_ptk(src_addr)) {
			PLATFORM_PRINTF_DEBUG("Connection already established with %s, ignore this introduction request.\n", mac2str(src_addr));
		} else {
			if (!DMisController() && 0 != PLATFORM_MEMCMP(DMcontrollerMacGet(), src_addr, 6)
			    && 0 != PLATFORM_STRCMP("lo", receiving_interface_name)
			    && (BACKHAUL_STA & PLATFORM_GET_BSS_TYPE(receiving_interface_name))) {
				struct dpp_authentication *dpp_auth = DMgetDPPAuthenticationObject();
				struct dpp_config_obj     *conf_obj = dpp_find_config_obj(dpp_auth, "map", "*", DPP_NETROLE_MAP_AGENT);
				struct dpp_eapol_sm       *sm       = NULL;
				DMgetDPPEAPOLStateMachine(&sm);
				if (NULL != conf_obj && sm->state != STARTED && DEVICE_FULLY_CONFIGURED == DMgetConfiguredState()) {
					if (0 == triggerDppPeerDiscovery(receiving_interface_name, src_addr))
						PLATFORM_PRINTF_DEBUG("Fail to trigger DPP peer discovery with agent: %s\n", mac2str(src_addr));
				}
			}
		}
#endif

		if (NULL != p) {
			PLATFORM_FREE(p);
		}

		break;
	}
	case CMDU_TYPE_TOPOLOGY_RESPONSE: {
		// When a "topology response" is received we must update our
		// internal database (that keeps track of which 1905 devices are
		// present in the network)

		INT8U *p;
		INT8U  i;

		struct deviceInformationTypeTLV *     info = NULL;
		struct deviceBridgingCapabilityTLV ** x    = NULL;
		struct non1905NeighborDeviceListTLV **y    = NULL;
		struct neighborDeviceListTLV **       z    = NULL;

		struct ApOperationalBSSTLV * operational_bss    = NULL;
		struct SupportedServiceTLV * supported_service  = NULL;
		struct AssociatedClientsTLV *associated_clients = NULL;

		INT8U ss_update = 0, as_update = 0;

		INT8U bridges_nr;
		INT8U non1905_neighbors_nr;
		INT8U x1905_neighbors_nr;
		INT8U multiap_profile;

		INT8U xi, yi, zi;

		char device_version[32] = {0};

		INT8U not_from_controller_interface;

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_TOPOLOGY_RESPONSE (%s) MID %04x\n", receiving_interface_name, c->message_id);

		if (waiting_msgs_ack(c->message_id) == 1 && PLATFORM_STRCMP("lo", receiving_interface_name)) {
			PLATFORM_PRINTF_WARNING("[Multi-AP] CMDU_TYPE_TOPOLOGY_RESPONSE packet [%04x] is timeout.\n", c->message_id);
			// break;
		}

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("Malformed structure.");
			break;
		}

		api_send_tlvs_response(MAP_TOPOLOGY_RESPONSE_ELEMENT, src_addr, c->list_of_TLVs);

		api_trigger_callback(c->message_type, c->list_of_TLVs);

		// First, extract the device info TLV and "count" how many bridging
		// capability TLVs, non-1905 neighbors TLVs and 1905 neighbors TLVs
		// there are
		//
		bridges_nr           = 0;
		non1905_neighbors_nr = 0;
		x1905_neighbors_nr   = 0;
		i                    = 0;

		multiap_profile = MULTI_AP_PROFILE_1;

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_DEVICE_INFORMATION_TYPE: {
				info = (struct deviceInformationTypeTLV *)p;
				break;
			}
			case TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES: {
				bridges_nr++;
				break;
			}
			case TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST: {
				non1905_neighbors_nr++;
				break;
			}
			case TLV_TYPE_NEIGHBOR_DEVICE_LIST: {
				x1905_neighbors_nr++;
				break;
			}
			case TLV_TYPE_VENDOR_SPECIFIC: {
				// According to the standard, zero or more Vendor
				// Specific TLVs may be present.
				//
				struct vendorSpecificTLV *vendor_tlv = (struct vendorSpecificTLV *)p;
				if (0 == PLATFORM_MEMCMP(REALTEK_OUI, vendor_tlv->vendorOUI, 3)) {
					//if it is from controller
					if (!PLATFORM_MEMCMP(DMcontrollerMacGet(), src_addr, 6)) {
						char *ip_addr = PLATFORM_STRDUP((char *)vendor_tlv->m);
						DMsetControllerIP(ip_addr);
						PLATFORM_FREE(ip_addr);
					}
					char *device_name = (char *)(vendor_tlv->m + PLATFORM_STRLEN((char *)vendor_tlv->m) + 1);
					if (PLATFORM_STRLEN((char *)vendor_tlv->m) + PLATFORM_STRLEN(device_name) + 2 < vendor_tlv->m_nr) {
						strncpy(device_version, (char *)(device_name + (int)PLATFORM_STRLEN(device_name) + 1), 31);
					} else{
						strcpy(device_version, "EasyMesh V1.0");
					}
				}

				break;
			}
			case TLV_TYPE_SUPPORTED_SERVICE: {
				supported_service = (struct SupportedServiceTLV *)p;
				int k             = 0;
				for (k = 0; k < supported_service->supported_service_nr; k++) {
					if (MULTI_AP_CONTROLLER_SERVICE == supported_service->supported_service[k]) {
						_updateControllerStatus(src_addr, receiving_interface_name);
#ifdef RTK_MULTI_AP_R3
						_runScheduledBSSConfigResult(src_addr, receiving_interface_name);
#endif
						break;
					}
				}
				ss_update = 1;
				break;
			}
			case TLV_TYPE_AP_OPERATIONAL_BSS: {
				operational_bss = (struct ApOperationalBSSTLV *)p;
				break;
			}
			case TLV_TYPE_ASSOCIATED_CLIENTS: {
				associated_clients = (struct AssociatedClientsTLV *)p;
				as_update          = 1;
				break;
			}
#ifdef RTK_MULTI_AP_R2
			case TLV_TYPE_MULTIAP_PROFILE: {
				struct MultiAPProfileTLV *profile_tlv = (struct MultiAPProfileTLV *) p;
				if (MULTI_AP_PROFILE_2 <= profile_tlv->profile) {
					multiap_profile = profile_tlv->profile;
				}
				break;
			}
#endif
			case TLV_TYPE_BSS_CONFIG_REPORT: {
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		// Next, now that we know how many TLVs of each type there are,
		// create an array of pointers big enough to contain them and fill
		// it.
		//
		if (info) {
			if (bridges_nr > 0) {
				x = (struct deviceBridgingCapabilityTLV **)PLATFORM_MALLOC(sizeof(struct deviceBridgingCapabilityTLV *) * bridges_nr);
			}
			if (non1905_neighbors_nr > 0) {
				y = (struct non1905NeighborDeviceListTLV **)PLATFORM_MALLOC(sizeof(struct non1905NeighborDeviceListTLV *) * non1905_neighbors_nr);
			}
			if (x1905_neighbors_nr > 0) {
				z = (struct neighborDeviceListTLV **)PLATFORM_MALLOC(sizeof(struct neighborDeviceListTLV *) * x1905_neighbors_nr);
			}

			xi = 0;
			yi = 0;
			zi = 0;
			i  = 0;
			while (NULL != (p = c->list_of_TLVs[i])) {
				switch (*p) {
				case TLV_TYPE_DEVICE_INFORMATION_TYPE: {
					break;
				}
				case TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES: {
					if(x)
						x[xi++] = (struct deviceBridgingCapabilityTLV *)p;
					break;
				}
				case TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST: {
					if(y)
						y[yi++] = (struct non1905NeighborDeviceListTLV *)p;
					break;
				}
				case TLV_TYPE_NEIGHBOR_DEVICE_LIST: {
					if(z)
						z[zi++] = (struct neighborDeviceListTLV *)p;
					break;
				}
				case TLV_TYPE_SUPPORTED_SERVICE:
				case TLV_TYPE_AP_OPERATIONAL_BSS:
				case TLV_TYPE_ASSOCIATED_CLIENTS: {
					break;
				}
				case TLV_TYPE_VENDOR_SPECIFIC: {
					// According to the standard, zero or more Vendor
					// Specific TLVs may be present.
					//
					free_1905_TLV_structure(p);
					break;
				}
				default: {
					// We are not interested in other TLVs. Free them
					//
					free_1905_TLV_structure(p);
					break;
				}
				}
				i++;
			}

			// The CMDU structure is not needed anymore, but we cannot just let
			// the caller call "free_1905_CMDU_structure()", because it would
			// also free the TLVs references that we need (ie. those saved in
			// the "x", "y", "z", "q" and "r" pointer arrays).
			// The "fix" is easy: just set "c->list_of_TLVs" to NULL so that
			// when the caller calls "free_1905_CMDU_structure()", this function
			// ignores (ie. does not free) the "list_of_TLVs" list.
			//
			// This will work because and memory won't be lost because:
			//
			//   1. Those TLVs contained in "c->list_of_TLVs" that we are not
			//      keeping track off have already been freed (see the "default"
			//      case in the previous "switch" structure).
			//
			//   2. The rest of them will be freed when the data model entry
			//      is replaced/deleted.
			//
			//   3. Setting "C->list_of_TLVs" to NULL will cause
			//      "free_1905_CMDU_structure()" to ignore this list.
			//
			PLATFORM_FREE(c->list_of_TLVs);
			c->list_of_TLVs = NULL;

			// Next, update the database. This will take care of duplicate
			// entries (and free TLVs if needed)
			//
			PLATFORM_PRINTF_DETAIL("Updating network devices database...\n");

			DMupdateNetworkDeviceInfo(info->al_mac_address, receiving_interface_name,
			                          1, info,
			                          1, x, bridges_nr,
			                          1, y, non1905_neighbors_nr,
			                          1, z, x1905_neighbors_nr,
			                          ss_update, supported_service,
			                          1, operational_bss,
			                          as_update, associated_clients);

			// Update the Multiap Pofile of this device
			DMsetTargetDeviceMultiAPProfile(info->al_mac_address, multiap_profile);
			if (device_version[0]) {
				PLATFORM_PRINTF_DEBUG("Device %02x:%02x:%02x:%02x:%02x:%02x version: %s\n", info->al_mac_address[0], info->al_mac_address[1], info->al_mac_address[2], info->al_mac_address[3], info->al_mac_address[4], info->al_mac_address[5], device_version);
				DMupdateNetworkDeviceVersion(info->al_mac_address, device_version);
			}

			// Show all network devices (ie. print them through the logging
			// system)
			//

			// DMdumpNetworkDevices(PLATFORM_PRINTF_DETAIL);

			//Process associated client report by other ALs

			// And finally, send other queries to the device so that we can
			// keep updating the database once the responses are received
			//
			// if (0 == send1905MetricsQueryPacket(receiving_interface_name, getNextMid(), info->al_mac_address)) {
			// 	PLATFORM_PRINTF_WARNING("Could not send 'metrics query' message\n");
			// }

			// There is one extra thing that needs to be done: send topology
			// query to neighbor's neighbors.
			//
			// This is not strictly necessary for 1905 to work. In fact, as I
			// think the protocol was designed, every node should only be aware
			// of its *direct* neighbors; and it is the HLE responsability to
			// query each node and build the network topology map.
			//
			// However, the 1905 datamodel standard document, interestingly
			// (and, I think, erroneously) includes information from all the
			// nodes (even those that are not direct neighbors).
			//
			// Here we are going to retrieve that information but, because this
			// requires much more memory in the AL node, we will only do this
			// if the user actually expressed his desire to do so when starting
			// the AL entity.
			//
			if (controller_interface == NULL
				|| (controller_interface && PLATFORM_STRCMP(controller_interface, receiving_interface_name) != 0))
				not_from_controller_interface = 1;
			else
				not_from_controller_interface = 0;

			if (PLATFORM_STRCMP("lo", receiving_interface_name) != 0
				&& 1 == DMmapWholeNetworkGet()
				&& not_from_controller_interface) {
				// For each neighbor interface
				//
				for (i = 0; i < zi; i++) {
					INT8U j;

					// For each neighbor's neighbor on that interface
					//
					for (j = 0; j < z[i]->neighbors_nr; j++) {
						INT8U ii, jj;

						// Discard the current node (obviously)
						//
						if (0 == PLATFORM_MEMCMP(DMalMacGet(), z[i]->neighbors[j].mac_address, 6)) {
							continue;
						}

						// Discard nodes I have just asked for
						//
						for (ii = 0; ii < i; ii++) {
							for (jj = 0; jj < z[ii]->neighbors_nr; jj++) {
								if (0 == PLATFORM_MEMCMP(z[ii]->neighbors[jj].mac_address, z[i]->neighbors[j].mac_address, 6)) {
									continue;
								}
							}
						}

						// Discard neighbors whose information was updated
						// recently (ie. no need to flood the network)
						//
						if (0 == DMnetworkDeviceInfoNeedsUpdate(z[i]->neighbors[j].mac_address)) {
							continue;
						}
						PLATFORM_PRINTF_DEBUG("Query new neighbor %02x:%02x:%02x:%02x:%02x:%02x\n", z[i]->neighbors[j].mac_address[0], z[i]->neighbors[j].mac_address[1], z[i]->neighbors[j].mac_address[2], z[i]->neighbors[j].mac_address[3], z[i]->neighbors[j].mac_address[4], z[i]->neighbors[j].mac_address[5]);
						if (0 == send1905TopologyQueryPacket(receiving_interface_name, getNextMid(), z[i]->neighbors[j].mac_address, 1)) {
							PLATFORM_PRINTF_WARNING("Could not send 'topology query' message\n");
						}
					}
				}
			}
		}

		if (PLATFORM_STRSTR(receiving_interface_name, "eth0.") != NULL) {
			struct eth_port_map *eth_recv_1905 = PLATFORM_GET_ETH_PORT_1905();
			for (i = 0; i < 4; i++) {
				if (0 == PLATFORM_STRCMP(eth_recv_1905[i].ifname, receiving_interface_name)) {
					// update timestamp
					eth_recv_1905[i].update_timestamp = PLATFORM_GET_TIMESTAMP();
					break;
				}
			}
		}

		if (MULTI_AP_PROFILE_3 > DMgetMultiAPProfile())
			return PROCESS_CMDU_OK_TRIGGER_AP_SEARCH;

		break;
	}
	case CMDU_TYPE_VENDOR_SPECIFIC: {
		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_VENDOR_SPECIFIC (%s) MID %04x\n", receiving_interface_name, c->message_id);
		api_send_tlvs_response(MAP_1905_VENDOR_SPECIFIC_CMDU_ELEMENT, src_addr, c->list_of_TLVs);
		break;
	}
	case CMDU_TYPE_LINK_METRIC_QUERY: {
		INT8U *p;
		INT8U  i;

		INT8U *dst_mac;

		struct linkMetricQueryTLV *t;

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_LINK_METRIC_QUERY (%s) MID %04x\n", receiving_interface_name, c->message_id);

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("Malformed structure.");
			break;
		}

		// First, search for the "struct linkMetricQueryTLV"
		//
		i = 0;
		t = NULL;
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_LINK_METRIC_QUERY: {
				t = (struct linkMetricQueryTLV *)p;
				break;
			}
			case TLV_TYPE_VENDOR_SPECIFIC: {
				// According to the standard, zero or more Vendor
				// Specific TLVs may be present.
				//
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		if (NULL == t) {
			PLATFORM_PRINTF_WARNING("More TLVs were expected inside this CMDU\n");
			return PROCESS_CMDU_KO;
		}

		if (LINK_METRIC_QUERY_TLV_ALL_NEIGHBORS == t->destination) {
			PLATFORM_PRINTF_DETAIL("Destination = all neighbors\n");
		} else if (LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR == t->destination) {
			PLATFORM_PRINTF_DETAIL("Destination = specific neighbor (%02x:%02x:%02x:%02x:%02x:%02x)\n", t->specific_neighbor[0], t->specific_neighbor[1], t->specific_neighbor[2], t->specific_neighbor[3], t->specific_neighbor[4], t->specific_neighbor[5]);
		} else {
			PLATFORM_PRINTF_WARNING("Unexpected 'destination' (%d)\n", t->destination);
			return PROCESS_CMDU_KO;
		}

		if (LINK_METRIC_QUERY_TLV_TX_LINK_METRICS_ONLY == t->link_metrics_type) {
			PLATFORM_PRINTF_DETAIL("Type        = Tx metrics only\n");
		} else if (LINK_METRIC_QUERY_TLV_RX_LINK_METRICS_ONLY == t->link_metrics_type) {
			PLATFORM_PRINTF_DETAIL("Type        = Rx metrics only\n");
		} else if (LINK_METRIC_QUERY_TLV_BOTH_TX_AND_RX_LINK_METRICS == t->link_metrics_type) {
			PLATFORM_PRINTF_DETAIL("Type        = Tx and Rx metrics\n");
		} else {
			PLATFORM_PRINTF_WARNING("Unexpected 'type' (%d)\n", t->link_metrics_type);
			return PROCESS_CMDU_KO;
		}

		// And finally, send a "metrics response" to the requesting neighbor

		// We must send the response to the AL MAC of the node who sent the
		// query, however, this AL MAC is *not* contained in the query.
		// The only thing we can do at this point is try to search our AL
		// neighbors data base for a matching MAC.
		//
		p = DMmacToAlMac(src_addr);

		if (NULL == p) {
			// The standard says we should always send to the AL MAC
			// address, however, in these cases, instead of just dropping
			// the packet, sending the response to the 'src' address from
			// the METRICS QUERY seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the METRICS QUERY (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = p;
		}

		if (0 == send1905MetricsResponsePacket(receiving_interface_name, c->message_id, dst_mac, t->destination, t->specific_neighbor, t->link_metrics_type)) {
			PLATFORM_PRINTF_WARNING("Could not send 'metrics response' message\n");
		}

		if (NULL != p) {
			PLATFORM_FREE(p);
		}

		break;
	}
	case CMDU_TYPE_LINK_METRIC_RESPONSE: {
		// When a "metrics response" is received we must update our
		// internal database (that keeps track of which 1905 devices are
		// present in the network)

		INT8U *p;
		INT8U  i;

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_LINK_METRIC_RESPONSE (%s) MID %04x\n", receiving_interface_name, c->message_id);

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("Malformed structure.");
			break;
		}

		//Send MAP_LINK_METRIC_RESPONSE_ELEMENT to user message queue
		api_send_tlvs_response(MAP_LINK_METRIC_RESPONSE_ELEMENT, src_addr, c->list_of_TLVs);

		// Call "DMupdateNetworkDeviceMetrics()" for each TLV
		//
		PLATFORM_PRINTF_DETAIL("Updating network devices database...\n");

		i = 0;
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_TRANSMITTER_LINK_METRIC:
			case TLV_TYPE_RECEIVER_LINK_METRIC: {
				DMupdateNetworkDeviceMetrics(p);
				break;
			}
			case TLV_TYPE_VENDOR_SPECIFIC: {
				// According to the standard, zero or more Vendor
				// Specific TLVs may be present.
				//
				free_1905_TLV_structure(p);
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("Unexpected TLV (%d) type inside CMDU\n", *p);

				free_1905_TLV_structure(p);
				break;
			}
			}
			i++;
		}

		// References to the TLVs cannot be freed by the caller (see the
		// comment in "case CMDU_TYPE_TOPOLOGY_RESPONSE:" to understand the
		// following two lines).
		//
		PLATFORM_FREE(c->list_of_TLVs);
		c->list_of_TLVs = NULL;

		// Show all network devices (ie. print them through the logging
		// system)
		//
		if(PLATFORM_NEED_DETAIL_PRINT()) {
			DMdumpNetworkDevices(PLATFORM_PRINTF_DETAIL);
		}

		break;
	}
	case CMDU_TYPE_AP_AUTOCONFIGURATION_SEARCH: {
		// When a "AP-autoconfig search" is received then, *only* if this
		// device is the Multi AP Controller, an "AP-autoconfig response"
		// message must be sent.
		// Otherwise, the message is ignored.

		INT8U *p;
		INT8U  i;

		INT8U dummy_mac_address[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

		INT8U searched_role_is_present;
		INT8U searched_role = 0xFF;

		INT8U freq_band_is_present;
		INT8U freq_band = 0xFF;

		INT8U include_chirp = 0;
#ifdef RTK_MULTI_AP_R3
		// for dpp
		INT8U *dpp_chirp = NULL;
		struct dpp_authentication *dpp_auth         = NULL;
#endif

		INT8U al_mac_address[6];

		INT8U multiap_profile;
		struct networkDevice *device = NULL;

		device = DMnetworkDeviceGet(src_addr);
		multiap_profile = MULTI_AP_PROFILE_1;
		searched_role_is_present = 0;
		freq_band_is_present     = 0;
		PLATFORM_MEMCPY(al_mac_address, dummy_mac_address, 6);

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_AP_AUTOCONFIGURATION_SEARCH (%s) MID %04x\n", receiving_interface_name, c->message_id);

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("Malformed structure.");
			break;
		}

		// First, parse the incomming packet to find out three things:
		// - The AL MAC of the node searching for AP-autoconfiguration
		//   parameters.
		// - The "searched role" contained in the "searched role TLV" (must
		//   be "REGISTRAR")
		// - The "freq band" contained in the "autoconfig freq band TLV"
		//   (must match the one of our local registrar interface)
		//
		i = 0;
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_AL_MAC_ADDRESS_TYPE: {
				struct alMacAddressTypeTLV *t = (struct alMacAddressTypeTLV *)p;

				PLATFORM_MEMCPY(al_mac_address, t->al_mac_address, 6);

				break;
			}
			case TLV_TYPE_SEARCHED_ROLE: {
				struct searchedRoleTLV *t = (struct searchedRoleTLV *)p;

				searched_role_is_present = 1;
				searched_role            = t->role;

				break;
			}

			case TLV_TYPE_SUPPORTED_FREQ_BAND: { // added for logo QCA onboard
				struct supportedFreqBandTLV *t = (struct supportedFreqBandTLV *)p;

				freq_band_is_present = 1;
				freq_band            = t->freq_band;
				break;
			}

			case TLV_TYPE_AUTOCONFIG_FREQ_BAND: {
				struct autoconfigFreqBandTLV *t = (struct autoconfigFreqBandTLV *)p;

				freq_band_is_present = 1;
				freq_band            = t->freq_band;

				break;
			}
			case TLV_TYPE_VENDOR_SPECIFIC: {
				break;
			}
			case TLV_TYPE_SUPPORTED_SERVICE: {
				break;
			}
			case TLV_TYPE_SEARCHED_SERVICE: {
				break;
			}
#ifdef RTK_MULTI_AP_R2
			case TLV_TYPE_MULTIAP_PROFILE: {
				struct MultiAPProfileTLV *profile_tlv = (struct MultiAPProfileTLV *) p;
				if (MULTI_AP_PROFILE_2 <= profile_tlv->profile) {
					multiap_profile = profile_tlv->profile;
				}
				break;
			}
#endif
#ifdef RTK_MULTI_AP_R3
			case TLV_TYPE_DPP_CHIRP_VALUE: {
				if (DMisController()) {
					struct DPPChirpValueTLV *chirp_tlv = (struct DPPChirpValueTLV *)p;
					PLATFORM_PRINTF_INFO("Received DPP Chirp Value TLV!\n");
					dpp_chirp = chirp_tlv->hash_value;
				}
				break;
			}
#endif
			default: {
				PLATFORM_PRINTF_WARNING("Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		if (device == NULL) {
			struct deviceInformationTypeTLV *     info = NULL;
			info = PLATFORM_MALLOC(sizeof(struct deviceInformationTypeTLV));
			PLATFORM_MEMCPY(info->al_mac_address, src_addr, 6);
			info->local_interfaces_nr = 0;
			info->local_interfaces = NULL;

			DMupdateNetworkDeviceInfo(src_addr, receiving_interface_name,
				1, info,
				0, NULL, 0,
				0, NULL, 0,
				0, NULL, 0,
				0, NULL,
				0, NULL,
				0, NULL);
		}

		DMsetTargetDeviceMultiAPProfile(src_addr, multiap_profile);

		// Make sure that all needed parameters were present in the message
		//
		if (
		    0 == PLATFORM_MEMCMP(al_mac_address, dummy_mac_address, 6) || 0 == searched_role_is_present || 0 == freq_band_is_present) {
			PLATFORM_PRINTF_WARNING("More TLVs were expected inside this CMDU\n");
			return PROCESS_CMDU_KO;
		}

		if (IEEE80211_ROLE_AP != searched_role) {
			PLATFORM_PRINTF_WARNING("Unexpected 'searched role'\n");
			return PROCESS_CMDU_KO;
		}

		// Only controller can send AUTOCONFIGURATION_RESPONSE.
		//
		if (!DMisController()) {
			break;
		}

#ifdef RTK_MULTI_AP_R3
		if (dpp_chirp) {
			dpp_auth = DMgetDPPAuthenticationObjectByChirp(dpp_chirp, SHA256_MAC_LEN);
			if (NULL != dpp_auth) {
				PLATFORM_PRINTF_INFO("Send DPP Chirping in Autoconfig Response\n");
				include_chirp = 1;
			}
		}

#endif

		if (0 == send1905APAutoconfigurationResponsePacket(receiving_interface_name, c->message_id, al_mac_address, freq_band, include_chirp)) {
			PLATFORM_PRINTF_WARNING("Could not send 'AP autoconfiguration response' message\n");
		}

#ifdef RTK_MULTI_AP_R3
		// Initiator sends DPP Authentication Request
		if (include_chirp) {
			INT8U *                    auth_req_msg     = NULL;
			INT32U                     auth_req_msg_len = 0;

			dpp_auth = DMgetDPPAuthenticationObjectByChirp(dpp_chirp, SHA256_MAC_LEN);
			if (dpp_auth->in_progress || dpp_auth->auth_success) {
				PLATFORM_PRINTF_DETAIL("DPP authentication in progress/done.");
				break;
			}
			if (dpp_auth_init(dpp_auth, &auth_req_msg, &auth_req_msg_len) == 1) {
				if (0 == send1905DirectEncapDPPPacket(receiving_interface_name, getNextMid(), al_mac_address, auth_req_msg, auth_req_msg_len))
					PLATFORM_PRINTF_WARNING("Could not send 'Direct Encap DPP' message\n");
			} else {
				PLATFORM_PRINTF_WARNING("dpp_auth_init returned false\n");
			}
			PLATFORM_FREE(auth_req_msg);
		}
#endif

		break;
	}
	case CMDU_TYPE_AP_AUTOCONFIGURATION_RESPONSE: {
		// When a "AP-autoconfig response" is received then we have to
		// search for the first interface which is an unconfigured AP with
		// the same freq band as the one contained in the message and send
		// a AP-autoconfig WSC-M1

		INT8U *p;
		INT8U  i;

		INT8U supported_role_is_present = 0;
		INT8U supported_role = 0xFF;

		INT8U supported_freq_band_is_present = 0;
		INT8U supported_freq_band;

		// for multi-ap
		INT8U supported_service_is_present = 0;
		INT8U supported_controller         = 0;

#ifdef RTK_MULTI_AP_R3
		INT8U delay_wsc = 0;
#endif

		INT8U multiap_profile;
		struct networkDevice *device = NULL;

		device = DMnetworkDeviceGet(src_addr);
		multiap_profile = MULTI_AP_PROFILE_1;

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_AP_AUTOCONFIGURATION_RESPONSE (%s) MID %04x\n", receiving_interface_name, c->message_id);

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("Malformed structure.");
			break;
		}

		api_send_tlvs_response(MAP_AUTOCONFIG_RESPONSE_ELEMENT, src_addr, c->list_of_TLVs);

		// First, parse the incomming packet to find out two things:
		//   parameters.
		// - The "supported role" contained in the "supported role TLV"
		//   (must be "REGISTRAR")
		// - The "supported freq band" contained in the "supported freq
		// band TLV" (must match the one of our local unconfigured
		// interface)
		//
		i = 0;
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_SUPPORTED_ROLE: {
				struct supportedRoleTLV *t = (struct supportedRoleTLV *)p;

				supported_role_is_present = 1;
				supported_role            = t->role;

				break;
			}
			case TLV_TYPE_SUPPORTED_FREQ_BAND: {
				struct supportedFreqBandTLV *t = (struct supportedFreqBandTLV *)p;

				supported_freq_band_is_present = 1;
				supported_freq_band            = t->freq_band;
				PLATFORM_PRINTF_DEBUG("[AUTOCONFIG] supported band %02x\n", supported_freq_band);
				break;
			}
			case TLV_TYPE_SUPPORTED_SERVICE: {
				// Multi-AP supported service TLV has been received.
				struct SupportedServiceTLV *t = (struct SupportedServiceTLV *)p;
				if (t->supported_service_nr) {
					supported_service_is_present = 1;
				}
				int service_i;
				for (service_i = 0; service_i < t->supported_service_nr; service_i++) {
					if (t->supported_service[service_i] == MULTI_AP_CONTROLLER_SERVICE) {
						supported_controller = 1;
						break;
					} else {
						continue;
					}
				}
				break;
			}
#ifdef RTK_MULTI_AP_R2
			case TLV_TYPE_MULTIAP_PROFILE: {
				struct MultiAPProfileTLV *profile_tlv = (struct MultiAPProfileTLV *) p;
				if (MULTI_AP_PROFILE_2 <= profile_tlv->profile) {
					multiap_profile = profile_tlv->profile;
				}
				break;
			}
#endif
			case TLV_TYPE_VENDOR_SPECIFIC: {
				break;
			}
#ifdef RTK_MULTI_AP_R3
			case TLV_TYPE_1905_LAYER_SECURITY_CAPABILITY: {
				// PLATFORM_PRINTF_INFO("<-- TLV_TYPE_1905_LAYER_SECURITY_CAPABILITY \n");
				break;
			}
			case TLV_TYPE_DPP_CHIRP_VALUE: {
				struct DPPChirpValueTLV *  chirp_tlv = (struct DPPChirpValueTLV *)p;
				struct dpp_authentication *dpp_auth  = DMgetDPPAuthenticationObject();
				if (PLATFORM_MEMCMP(chirp_tlv->hash_value, dpp_auth->own_bi->pubkey_hash_chirp, chirp_tlv->hash_len) == 0) {
					PLATFORM_PRINTF_INFO("Own DPP chirp received!\n");
					delay_wsc = 1;                  // MAP Spec v3 5.3.5 requires Responder to delay WSC to DPP_TIMER (10 sec)
				}
				break;
			}
#endif
			default: {
				PLATFORM_PRINTF_WARNING("Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		if (device == NULL) {
			struct deviceInformationTypeTLV *     info = NULL;
			info = PLATFORM_MALLOC(sizeof(struct deviceInformationTypeTLV));
			PLATFORM_MEMCPY(info->al_mac_address, src_addr, 6);
			info->local_interfaces_nr = 0;
			info->local_interfaces = NULL;

			DMupdateNetworkDeviceInfo(src_addr, receiving_interface_name,
				1, info,
				0, NULL, 0,
				0, NULL, 0,
				0, NULL, 0,
				0, NULL,
				0, NULL,
				0, NULL);
		}

		DMsetTargetDeviceMultiAPProfile(src_addr, multiap_profile);

		// Make sure that all needed parameters were present in the message
		//
		if (
		    0 == supported_role_is_present || 0 == supported_freq_band_is_present) {
			PLATFORM_PRINTF_WARNING("More TLVs were expected inside this CMDU\n");
			return PROCESS_CMDU_KO;
		}

		if (IEEE80211_ROLE_AP != supported_role) {
			PLATFORM_PRINTF_WARNING("Unexpected 'searched role'\n");
			return PROCESS_CMDU_KO;
		}

		// for multi-ap
		if (0 == supported_service_is_present) {
			PLATFORM_PRINTF_WARNING("[Multi-AP] More TLVs were expected inside this CMDU\n");
			return PROCESS_CMDU_KO;
		}

		if (!supported_controller) {
			PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected CMDU_TYPE_AP_AUTOCONFIGURATION_RESPONSE from Multi AP Agent\n");
			return PROCESS_CMDU_KO;
		}

		INT8U *dst_mac;
		INT8U *al_addr;

		// We must send the WSC TLV to the AL MAC of the node who
		// sent the response, however, this AL MAC is *not*
		// contained in the response. The only thing we can do at
		// this point is try to search our AL neighbors data base
		// for a matching MAC.
		//
		al_addr = DMmacToAlMac(src_addr);

		if (NULL == al_addr) {
			// The standard says we should always send to the AL
			// MAC address, however, in these cases, instead of
			// just dropping the packet, sending the response to
			// the 'src' address from the AUTOCONFIGURATION RESPONSE
			// seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the AUTOCONFIGURATION RESPONSE (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = al_addr;
		}

		// Trigger WSC only if the packet is from supported controller
		//
		if (1 == supported_controller) {
			_updateControllerStatus(dst_mac, receiving_interface_name);
			PLATFORM_FREE(al_addr);
			PLATFORM_PRINTF_INFO("[Multi-AP] Controller is found. MAC:(%02x:%02x:%02x:%02x:%02x:%02x). \n", src_addr[0], src_addr[1], src_addr[2], src_addr[3], src_addr[4], src_addr[5]);

			if (0x00 != (0xF0 & DMgetConfiguredState())) {
				PLATFORM_PRINTF_DEBUG("Device is in the middle of autoconfig, skip trigger autoconfig wsc..\n");
				return PROCESS_CMDU_OK;
			}
#ifdef RTK_MULTI_AP_R3
			if (delay_wsc) {
				struct dpp_authentication *dpp_auth = DMgetDPPAuthenticationObject();
				struct dpp_config_obj *    conf_obj = dpp_find_config_obj(dpp_auth, "map", "*", DPP_NETROLE_MAP_AGENT);
				if (NULL == conf_obj) {
					dpp_auth_clear_state(dpp_auth);
					return PROCESS_CMDU_OK_DELAYED_AUTOCONFIG;
				} else {
					if (MULTI_AP_PROFILE_3 <= multiap_profile)
						return PROCESS_CMDU_OK_TRIGGER_PEER_DISCOVERY; // Trigger 1905 DPP network introduction, instead of wsc autoconfig. Multi-ap spec_v3 5.3.4 P38
					return PROCESS_CMDU_OK;
				}
			}
#endif
			return PROCESS_CMDU_OK_TRIGGER_AUTOCONFIG;
		}

		PLATFORM_FREE(al_addr);

		break;
	}
	case CMDU_TYPE_AP_AUTOCONFIGURATION_WSC: {
		INT8U *p;
		INT8U  i;

		struct wscTLV *wsc_tlvs;
		INT8U          wsc_tlv_nr;

		INT8U wsc_type;
		INT8U wsc_band;
		INT8U max_bss_nr = 0;
#ifdef RTK_MULTI_AP_R2
		INT8U **vlan_tlvs   = NULL;
		INT8U   vlan_tlv_nr = 0;
#endif
		struct vendor_specific_data *vendor_datas = NULL;
		INT8U                        vendor_data_nr = 0;

		struct OperatingClass *op_classes  = NULL;
		INT8U                  op_class_nr = 0;

		INT8U radio_identifier[6];
		PLATFORM_MEMSET(radio_identifier, 0x00, 6);

		// When a "AP-autoconfig WSC" is received we first have to find out
		// if the contained message is M1 or M2.
		// If it is M1, send an M2 response.
		// If it is M2, apply the received configuration.

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_AP_AUTOCONFIGURATION_WSC (%s) MID %04x\n", receiving_interface_name, c->message_id);

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("Malformed structure.");
			break;
		}

		wsc_tlvs   = NULL;
		wsc_tlv_nr = 0;
		i          = 0;
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_WSC: {
				wsc_tlv_nr += 1;
				break;
			}
			}
			i++;
		}

		wsc_tlvs   = (struct wscTLV *)PLATFORM_MALLOC(sizeof(struct wscTLV) * wsc_tlv_nr);
		wsc_tlv_nr = 0;
		i          = 0;

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_WSC: {
				struct wscTLV *t     = (struct wscTLV *)p;
				wsc_tlvs[wsc_tlv_nr] = *t;
				wsc_tlv_nr += 1;
				break;
			}
			case TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES: {
				// PLATFORM_PRINTF_INFO("<-- TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES \n");
				struct APRadioBasicCapabilitiesTLV *radio_capabilities_tlv = (struct APRadioBasicCapabilitiesTLV *)p;
				PLATFORM_MEMCPY(radio_identifier, radio_capabilities_tlv->radio_unique_id, 6);
				max_bss_nr  = radio_capabilities_tlv->max_BSSs_nr;
				op_classes  = radio_capabilities_tlv->operating_classes;
				op_class_nr = radio_capabilities_tlv->operating_classes_nr;
				break;
			}
			case TLV_TYPE_AP_RADIO_IDENTIFIER: {
				// PLATFORM_PRINTF_INFO("<-- TLV_TYPE_AP_RADIO_IDENTIFIER \n");
				struct ApRadioIdentifierTLV *radio_identifier_tlv = (struct ApRadioIdentifierTLV *)p;
				PLATFORM_MEMCPY(radio_identifier, radio_identifier_tlv->radio_unique_id, 6);
				break;
			}
#ifdef RTK_MULTI_AP_R2
			case TLV_TYPE_DEFAULT_802_1Q_SETTINGS: {
				PLATFORM_PRINTF_INFO("<-- TLV_TYPE_DEFAULT_802_1Q_SETTINGS \n");
				struct Default8021QSettingsTLV *default_80211q_settings_tlv = (struct Default8021QSettingsTLV *)p;
				DMsetDefault8021QSettings(default_80211q_settings_tlv->primary_vlan_id, default_80211q_settings_tlv->default_pcp);
				vlan_tlv_nr += 1;
				vlan_tlvs = (INT8U **)PLATFORM_REALLOC(vlan_tlvs, sizeof(INT8U *) * (vlan_tlv_nr + 1));
				vlan_tlvs[vlan_tlv_nr -1] = p;
				vlan_tlvs[vlan_tlv_nr] = NULL;
				break;
			}
			case TLV_TYPE_TRAFFIC_SEPARATION_POLICY: {
				PLATFORM_PRINTF_INFO("<-- TLV_TYPE_TRAFFIC_SEPARATION_POLICY \n");
				struct TrafficSeparationPolicyTLV *traffic_separation_policy_tlv = (struct TrafficSeparationPolicyTLV *)p;
				DMsetTrafficSeparationPolicy(traffic_separation_policy_tlv);
				break;
			}
			case TLV_TYPE_AP_RADIO_ADVANCED_CAPABILITIES: {
				break;
			}
			case TLV_TYPE_PROFILE_2_CAPABILITY: {
				break;
			}
#endif
			case TLV_TYPE_VENDOR_SPECIFIC: {
				struct vendorSpecificTLV *vendor_tlv = (struct vendorSpecificTLV *)p;
				vendor_data_nr++;
				vendor_datas = (struct vendor_specific_data *)PLATFORM_REALLOC(vendor_datas, sizeof(struct vendor_specific_data) * vendor_data_nr);
				PLATFORM_MEMCPY(vendor_datas[vendor_data_nr - 1].vendor_oui, vendor_tlv->vendorOUI, 3);
				vendor_datas[vendor_data_nr - 1].payload_len = vendor_tlv->m_nr;
				vendor_datas[vendor_data_nr - 1].payload = NULL;
				if (vendor_tlv->m_nr) {
					vendor_datas[vendor_data_nr - 1].payload     = PLATFORM_MALLOC(sizeof(INT8U) * vendor_tlv->m_nr);
					PLATFORM_MEMCPY(vendor_datas[vendor_data_nr - 1].payload, vendor_tlv->m, vendor_tlv->m_nr);
				}
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		// Make sure there was a WSC TLV in the message
		//
		if (0 == wsc_tlv_nr) {
			PLATFORM_PRINTF_WARNING("More TLVs were expected inside this CMDU\n");
			PLATFORM_FREE(wsc_tlvs);
			return PROCESS_CMDU_KO;
		}

		wsc_type = wscGetType(wsc_tlvs[0].wsc_frame, wsc_tlvs[0].wsc_frame_size);

		wsc_band = wscGetBand(wsc_tlvs[0].wsc_frame, wsc_tlvs[0].wsc_frame_size);

		if (WSC_TYPE_M2 != wsc_type) {
			// No need to process vendor data
			if (vendor_data_nr) {
				for (i = 0; i < vendor_data_nr; i++) {
					PLATFORM_FREE(vendor_datas[i].payload);
				}
				PLATFORM_FREE(vendor_datas);
			}
		}

		if (WSC_TYPE_M2 == wsc_type) {
			// Process it and apply the configuration to the corresponding
			// interface.
			//
			INT8U  do_M2 = 0;
			struct radio_config_data radio_config = { 0 };
			radio_config.bss_data_nr = wsc_tlv_nr;
			radio_config.bss_data    = PLATFORM_ZALLOC(sizeof(struct bss_config_data) * radio_config.bss_data_nr);
			radio_config.radio_band  = 0xFF;
			radio_config.vendor_data_nr = vendor_data_nr;
			radio_config.vendor_datas = vendor_datas;

			struct interfaceInfo *x = NULL;

#ifdef RTK_MULTI_AP_R2
			if(MULTI_AP_PROFILE_2 <= DMgetMultiAPProfile() && NULL != vlan_tlvs) {
				api_send_tlvs_response(MAP_VLAN_CONFIGURATION_ELEMENT, src_addr, vlan_tlvs);
				PLATFORM_FREE(vlan_tlvs);
			}
#endif
			INT8U disable_mac[6] = { 0x00, 0xe0, 0x4c, 0x81, 0x86, 0x86 };
			if (0 == PLATFORM_MEMCMP(disable_mac, radio_identifier, 6)) {
				if (IEEE80211_FREQUENCY_BAND_2_4_GHZ == wsc_band) {
					x = PLATFORM_GET_1905_INTERFACE_INFO(map_radio_name_24g);
				} else if (IEEE80211_FREQUENCY_BAND_5_GHZ == wsc_band) {
					x = PLATFORM_GET_1905_INTERFACE_INFO(map_radio_name_5gl);
				}
			} else {
				// Coredump due to null data, TBD, wait for hostapd easymesh
				if (DMmacToInterfaceName(radio_identifier) != NULL) {
					x = PLATFORM_GET_1905_INTERFACE_INFO(DMmacToInterfaceName(radio_identifier));
				}
			}

			if (NULL == x) {
				PLATFORM_PRINTF_ERROR("[WSC] Unknown band %02x \n", wsc_band);
				PLATFORM_FREE(wsc_tlvs);
				return PROCESS_CMDU_KO;
			}

			if (IS_IEEE_802_11_2_4_GHZ(x->interface_type, x->interface_type_data.ieee80211.rf_band)
				&& IEEE80211_FREQUENCY_BAND_2_4_GHZ == wsc_band)
				do_M2 = 1;

			if (IS_IEEE_802_11_5_GHZ(x->interface_type, x->interface_type_data.ieee80211.rf_band)
				&& IEEE80211_FREQUENCY_BAND_5_GHZ == wsc_band)
				do_M2 = 1;

			if (do_M2) {
				char wsc_interface_band_char = x->name[4];
				if (map_radio_name_24g[4] == wsc_interface_band_char)
					radio_config.radio_band = MAP_CONFIG_2G;
				else if (map_radio_name_5gl[4] == wsc_interface_band_char)
					radio_config.radio_band = MAP_CONFIG_5GL;
				else if (map_radio_name_5gh[4] == wsc_interface_band_char)
					radio_config.radio_band = MAP_CONFIG_5GH;

				PLATFORM_FREE_1905_INTERFACE_INFO(x);

				INT8U result = 0;

				for (i = 0; i < wsc_tlv_nr; i++) {
					if (wsc_type != wscGetType(wsc_tlvs[i].wsc_frame, wsc_tlvs[i].wsc_frame_size)) {
						PLATFORM_PRINTF_ERROR("[WSC] Malform WSC TLV received");
						continue;
					}

					struct wsc_config config_data;
					result = wscProcessM2(NULL, NULL, 0, wsc_tlvs[i].wsc_frame, wsc_tlvs[i].wsc_frame_size, &config_data, radio_config.radio_band);
					if (1 != result) {
						PLATFORM_FREE(radio_config.bss_data);
						break;
					}

					radio_config.bss_data[i].ssid                = PLATFORM_STRDUP(config_data.ssid);
					radio_config.bss_data[i].network_key         = PLATFORM_STRDUP(config_data.network_key);
					radio_config.bss_data[i].authentication_type = config_data.auth_type;
					radio_config.bss_data[i].encryption_type     = config_data.encryption_type;
					radio_config.bss_data[i].network_type        = config_data.network_type;
#ifdef RTK_MULTI_AP_R2
					radio_config.bss_data[i].vid                 = DMgetVIDbySSID(config_data.ssid);
#else
					radio_config.bss_data[i].vid                 = 0;
#endif
					PLATFORM_FREE(config_data.ssid);
					PLATFORM_FREE(config_data.network_key);

					PLATFORM_PRINTF_INFO("Config Data %d %s %s %02x %02x %02x %02x\n", wsc_tlv_nr, radio_config.bss_data[i].ssid, radio_config.bss_data[i].network_key, radio_config.bss_data[i].authentication_type, radio_config.bss_data[i].encryption_type, radio_config.bss_data[i].network_type, radio_config.radio_band);
				}

				wscFreeLastM1(wsc_band, radio_config.radio_band);
				if (0 == result) {
					PLATFORM_PRINTF_WARNING("Error when processing M2, stop.\n");
					api_send_customized_status_notification(STATUS_M2_PROCESS_FAIL);
					PLATFORM_FREE(wsc_tlvs);
					return PROCESS_CMDU_KO;
				}

				if (2 == result) {
					char **ifs_names;
					INT8U  ifs_nr;
					// INT8U  role;
					ifs_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&ifs_nr);
					for (i = 0; i < ifs_nr; i++) {
						if (PLATFORM_STRSTR(ifs_names[i], map_vxd_prefix)) {
							continue;
						}
						if (wsc_interface_band_char == ifs_names[i][4]) {
							// role = DMnameToLocalInterfaceStruct(ifs_names[i])->role;
							// if (FRONTHAUL_BSS & role) {
							PLATFORM_PRINTF_DEBUG("[WSC] teardown %s\n", ifs_names[i]);
							PLATFORM_BSS_IOCTL(ifs_names[i], TEAR_DOWN);
							// }
						}
					}
					radio_config.bss_data_nr               = 1;
					struct bss_config_data *tear_down_data = PLATFORM_ZALLOC(sizeof(struct bss_config_data) * radio_config.bss_data_nr);
					tear_down_data[0].ssid                 = PLATFORM_STRDUP("TEAR_DOWN");
					tear_down_data[0].network_key          = PLATFORM_STRDUP("00000000");
					tear_down_data[0].authentication_type  = WPS_AUTH_WPA2PSK;
					tear_down_data[0].encryption_type      = WPS_ENCR_AES;
					tear_down_data[0].network_type         = TEAR_DOWN;
					tear_down_data[0].vid                  = 0;
					radio_config.bss_data                  = tear_down_data;
				}

				api_send_config_request(radio_config);


				_free_radio_config(&radio_config);

				INT8U configured_band = DMgetConfiguredState();
				if (IEEE80211_FREQUENCY_BAND_5_GHZ == wsc_band) {
					if (MAP_CONFIG_5GH == radio_config.radio_band) {
						configured_band = configured_band & (0xBF);
					} else {
						configured_band = configured_band & (0xDF);
						if (!DMisTribandDevice()) {
							configured_band = configured_band & (0xBF);
						}
					}

				} else if (IEEE80211_FREQUENCY_BAND_2_4_GHZ == wsc_band) {
					configured_band = configured_band & (0xEF);
				}

				DMsetConfiguredState(configured_band);

				if (vendor_data_nr) {
					for (i = 0; i < vendor_data_nr; i++) {
						PLATFORM_FREE(vendor_datas[i].payload);
					}
					PLATFORM_FREE(vendor_datas);
				}

				// NOTE: this function will automatically free M1.
			} else {
				PLATFORM_PRINTF_ERROR("WSC M2 does not match the interface to be configures!\n");
				PLATFORM_FREE_1905_INTERFACE_INFO(x);
			}
		} else if (WSC_TYPE_M1 == wsc_type) {
			// We hadn't previously sent an M1 (ie. we are the registrar),
			// thus the contents of the just received message must be M1.
			//
			// Process it and send an M2 response.
			//
			if (wsc_tlv_nr != 1) {
				PLATFORM_PRINTF_ERROR("[WSC] More than 1 M1 received! Abort message.\n");
				PLATFORM_FREE(wsc_tlvs);
				break;
			}

			api_send_tlvs_response(MAP_WSC_M1_ELEMENT, src_addr, c->list_of_TLVs);

			INT8U **m2      = NULL;
			INT16U *m2_size = NULL;

			INT8U wsc_band;

			INT8U *dst_mac;
			INT8U *p;

			INT8U m1_mac[6];

			INT8U i;

			INT8U                     m2_vendor_tlv_nr = 0;
			struct vendorSpecificTLV *m2_vendor_tlvs   = NULL;

			struct configData *m2_config[MAX_RECV_M2_CONFIG_NR];
			INT8U              m2_nr = 0;

			INT8U band = get_band_from_op_classes(op_classes, op_class_nr);

			struct TrafficSeparationPolicyTLV *traffic_separation_policy = NULL;

			wscGetMacM1(wsc_tlvs[0].wsc_frame, wsc_tlvs[0].wsc_frame_size, &m1_mac);
			wsc_band = wscGetBand(wsc_tlvs[0].wsc_frame, wsc_tlvs[0].wsc_frame_size);

			PLATFORM_PRINTF_DEBUG("[WSC] M1 MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
				m1_mac[0], m1_mac[1], m1_mac[2], m1_mac[3], m1_mac[4], m1_mac[5]);

			// Vendor Specific TLV
			//
			if (IEEE80211_FREQUENCY_BAND_5_GHZ == wsc_band) {
				if ((INTERFACE_BAND_5GL & band) && (0 != config.config_number_5gl || 0 == (INTERFACE_BAND_5GH & band))) {
					m2_vendor_tlvs = DMgetAutoconfigVendorTLV(&m2_vendor_tlv_nr, MAP_CONFIG_5GL);
				} else {
					m2_vendor_tlvs = DMgetAutoconfigVendorTLV(&m2_vendor_tlv_nr, MAP_CONFIG_5GH);
				}
			} else if (IEEE80211_FREQUENCY_BAND_2_4_GHZ == wsc_band) {
				m2_vendor_tlvs = DMgetAutoconfigVendorTLV(&m2_vendor_tlv_nr, MAP_CONFIG_2G);
			} else {
				PLATFORM_PRINTF_WARNING("Unknown band!\n");
				PLATFORM_FREE(wsc_tlvs);
				break;
			}

			// WSC M2 TLV
			//
			PLATFORM_MEMSET(m2_config, 0, sizeof(m2_config));
			getConfigsForAutoconfig(band, src_addr, m2_config, &m2_nr, MAX_RECV_M2_CONFIG_NR);
			if (m2_nr > max_bss_nr) {
				PLATFORM_PRINTF_WARNING("Number of BSS to configure exceeded max_bss of this radio!\n");
				m2_nr = max_bss_nr;
			}

			m2      = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * m2_nr);
			m2_size = (INT16U *)PLATFORM_MALLOC(sizeof(INT16U) * m2_nr);

			struct networkDevice *device = DMnetworkDeviceGet(m1_mac);
			INT8U **              interface_mac;
			interface_mac = PLATFORM_MALLOC(sizeof(char *) * m2_nr);
			int j         = 0;

			if (NULL == device) {
				for (i = 0; i < m2_nr; i++) {
					interface_mac[i] = m1_mac;
				}
			} else {

				for (i = 0; i < device->info->local_interfaces_nr; i++) {
					if (IEEE80211_FREQUENCY_BAND_2_4_GHZ == wsc_band
						&& IS_IEEE_802_11_2_4_GHZ_MEDIA(device->info->local_interfaces[i].media_type)) {
						if (IEEE80211_ROLE_AP != device->info->local_interfaces[i].media_specific_data.ieee80211.role) {
							continue;
						}
						PLATFORM_PRINTF_INFO("2.4G Interface BSSID: %02x:%02x:%02x:%02x:%02x:%02x\n",
							device->info->local_interfaces[i].mac_address[0],
							device->info->local_interfaces[i].mac_address[1],
							device->info->local_interfaces[i].mac_address[2],
							device->info->local_interfaces[i].mac_address[3],
							device->info->local_interfaces[i].mac_address[4],
							device->info->local_interfaces[i].mac_address[5]);
						interface_mac[j] = device->info->local_interfaces[i].mac_address;
						j++;
						if (j >= m2_nr) {
							PLATFORM_PRINTF_WARNING("More interfaces than config data!!\n");
							break;
						}
					} else if (IEEE80211_FREQUENCY_BAND_5_GHZ == wsc_band
						&& IS_IEEE_802_11_5_GHZ_MEDIA(device->info->local_interfaces[i].media_type)) {
						if (IEEE80211_ROLE_AP != device->info->local_interfaces[i].media_specific_data.ieee80211.role) {
							continue;
						}
						PLATFORM_PRINTF_INFO("5G Interface BSSID: %02x:%02x:%02x:%02x:%02x:%02x\n",
							device->info->local_interfaces[i].mac_address[0],
							device->info->local_interfaces[i].mac_address[1],
							device->info->local_interfaces[i].mac_address[2],
							device->info->local_interfaces[i].mac_address[3],
							device->info->local_interfaces[i].mac_address[4],
							device->info->local_interfaces[i].mac_address[5]);
						interface_mac[j] = device->info->local_interfaces[i].mac_address;
						j++;
						if (j >= m2_nr) {
							PLATFORM_PRINTF_WARNING("More interfaces than config data!!\n");
							break;
						}
					}
				}
			}

			for (i = 0; i < m2_nr; i++) {
				if (i >= j) {
					PLATFORM_PRINTF_WARNING("Less local interfaces than config data..\n");
					interface_mac[i] = m1_mac;
				}
				PLATFORM_PRINTF_INFO("Send M2 for Interface BSSID %02x:%0x:%02x:%0x:%02x:%02x\n",
					interface_mac[i][0], interface_mac[i][1],
					interface_mac[i][2], interface_mac[i][3],
					interface_mac[i][4], interface_mac[i][5]);
				m2_config[i]->bssid = interface_mac[i];
				wscBuildM2(wsc_tlvs[0].wsc_frame, wsc_tlvs[0].wsc_frame_size, &m2[i], &m2_size[i], m2_config[i]);
			}

			PLATFORM_FREE(interface_mac);

#ifdef RTK_MULTI_AP_R2
			// Traffic Separation TLV
			//
			obtainTrafficSeparationPolicyTLV(&traffic_separation_policy, m2_config, m2_nr);
			if (!traffic_separation_policy) {
				PLATFORM_PRINTF_DEBUG("No Traffic Separation Policy Found\n");
			}
#endif

			// We must send M2 to the AL MAC of the node who sent M1,
			// however, this AL MAC is *not* contained in M1.
			// The only thing we can do at this point is try to search our
			// AL neighbors data base for a matching MAC.
			//
			p = DMmacToAlMac(src_addr);

			if (NULL == p) {
				// The standard says we should always send to the AL MAC
				// address, however, in these cases, instead of just
				// dropping the packet, sending M2 to the 'src' address
				// from the M1 seems the right thing to do.
				//
				dst_mac = src_addr;
				PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from M1 (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
			} else {
				dst_mac = p;
			}

			if (0 == send1905APAutoconfigurationWSCPacket(receiving_interface_name, getNextMid(), dst_mac, m2, m2_size, m2_nr, radio_identifier, NULL, traffic_separation_policy, m2_vendor_tlv_nr, m2_vendor_tlvs)) {
				PLATFORM_PRINTF_WARNING("Could not send 'AP autoconfiguration WSC-M2' message\n");
			}

			if (NULL != p) {
				PLATFORM_FREE(p);
			}

			if (traffic_separation_policy) {
				free_1905_TLV_structure((INT8U *)traffic_separation_policy);
			}

			for (i = 0; i < m2_nr; i++) {
				wscFreeM2(m2[i], m2_size[i]);
			}
			PLATFORM_FREE(m2);
			PLATFORM_FREE(m2_size);
		} else {
			PLATFORM_PRINTF_WARNING("Unknown type of WSC message!\n");
		}
		PLATFORM_FREE(wsc_tlvs);
		break;
	}
	case CMDU_TYPE_AP_AUTOCONFIGURATION_RENEW: {
		INT8U *p;
		INT8U  i;

		INT8U al_mac_address[6];
		INT8U al_mac_address_is_present = 0;

		INT8U supported_role_is_present = 0;
		INT8U supported_role = 0xFF;

		INT8U supported_freq_band_is_present = 0;
		INT8U supported_freq_band;

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_AP_AUTOCONFIGURATION_RENEW (%s) MID %04x\n", receiving_interface_name, c->message_id);

		if (DEVICE_FULLY_CONFIGURED != DMgetConfiguredState()) {
			PLATFORM_PRINTF_DEBUG("Device is not fully configured, skip autoconfig renew..\n");
			break;
		}

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("Malformed structure.");
			break;
		}

		api_send_tlvs_response(MAP_AUTOCONFIG_RENEW_NOTIFICATION_ELEMENT, src_addr, c->list_of_TLVs);

		// First, parse the incomming packet to find out two things:
		//   parameters.
		// - The "supported role" contained in the "supported role TLV"
		//   (must be "REGISTRAR")
		// - The "supported freq band" contained in the "supported freq
		// band TLV" (must match the one of our local unconfigured
		// interface)
		//
		i = 0;
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_SUPPORTED_ROLE: {
				struct supportedRoleTLV *t = (struct supportedRoleTLV *)p;

				supported_role_is_present = 1;
				supported_role            = t->role;

				break;
			}
			case TLV_TYPE_SUPPORTED_FREQ_BAND: {
				struct supportedFreqBandTLV *t = (struct supportedFreqBandTLV *)p;

				supported_freq_band_is_present = 1;
				supported_freq_band            = t->freq_band;
				PLATFORM_PRINTF_DEBUG("[AUTOCONFIG] supported band %02x\n", supported_freq_band);
				break;
			}
			case TLV_TYPE_AL_MAC_ADDRESS_TYPE: {
				// Multi-AP supported service TLV has been received.
				struct alMacAddressTypeTLV *t = (struct alMacAddressTypeTLV *)p;
				al_mac_address_is_present     = 1;

				PLATFORM_MEMCPY(al_mac_address, t->al_mac_address, 6);
				break;
			}
			case TLV_TYPE_VENDOR_SPECIFIC: {
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		// Make sure that all needed parameters were present in the message
		//
		if (
		    0 == supported_role_is_present || 0 == supported_freq_band_is_present || 0 == al_mac_address_is_present) {
			PLATFORM_PRINTF_WARNING("More TLVs were expected inside this CMDU\n");
			return PROCESS_CMDU_KO;
		}

		if (IEEE80211_ROLE_AP != supported_role) {
			PLATFORM_PRINTF_WARNING("Unexpected 'searched role'\n");
			return PROCESS_CMDU_KO;
		}

		_updateControllerStatus(al_mac_address, receiving_interface_name);
		DMsetConfiguredState(0x00);
		return PROCESS_CMDU_OK_TRIGGER_AUTOCONFIG;

	}
	case CMDU_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION: {
		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION (%s) MID %04x\n", receiving_interface_name, c->message_id);
		break;
	}
	case CMDU_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION: {
		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION (%s) MID %04x\n", receiving_interface_name, c->message_id);
		break;
	}
	default: {
		ret = processMultiAPCmdu(c, receiving_interface_name, src_addr);
		break;
	}
	}
	return ret;
}

INT8U processMultiAPCmdu(struct CMDU *c, char *receiving_interface_name, INT8U *src_addr)
{
	INT8U ret = PROCESS_CMDU_OK;
	switch (c->message_type) {
	case CMDU_TYPE_MULTI_AP_POLICY_CONFIG_REQUEST: {
		INT8U *p;

#ifdef RTK_MULTI_AP_R2
		INT8U **vlan_tlvs   = NULL;
		INT8U   vlan_tlv_nr = 0;
		INT8U vlan_ssid_nr = 0;
#endif

		INT8U *dst_mac;
		INT8U *al_addr = NULL;

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_MULTI_AP_POLICY_CONFIG_REQUEST (%s) MID: %x\n", receiving_interface_name, c->message_id);

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Malformed structure.");
			break;
		}

		al_addr = DMmacToAlMac(src_addr);

		if (NULL == al_addr) {
			dst_mac = src_addr;
		} else {
			dst_mac = al_addr;
		}

		// send ACK back without error code TLV
		if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_MULTI_AP_POLICY_CONFIG_REQUEST MID: (%x) is failed\n", c->message_id);
		}

		api_send_tlvs_response(MAP_POLICY_CONFIG_NOTIFICATION_ELEMENT, src_addr, c->list_of_TLVs);

		int i = 0, k;
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_STEERING_POLICY: {
				struct SteeringPolicyTLV *t = (struct SteeringPolicyTLV *)p;
				DMupdateSteeringPolicy(p);
				for (k = 0; k < t->radios_nr; k++) {
					char *interface_name;
					interface_name = DMmacToInterfaceName(t->policies[k].radio_unique_id);
					if (interface_name != NULL)
						PLATFORM_STEERING_POLICY_IOCTL(interface_name, t->policies[k].policy, t->policies[k].channel_util_threshold, t->policies[k].rcpi_steering_threshold);
				}
				break;
			}
			case TLV_TYPE_METRIC_REPORT_POLICY: {
				struct MetricReportingPolicyTLV *t = (struct MetricReportingPolicyTLV *)p;
				DMsetIntervalTime(t->report_interval);
				for (k = 0; k < t->radios_nr; k++) {
					char *interface_name;

					char **ifs_names;
					INT8U  ifs_nr, j;

					ifs_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&ifs_nr);

					interface_name = DMmacToInterfaceName(t->policies[k].radio_unique_id);
					if (interface_name) {
						for (j = 0; j < ifs_nr; j++) {
							if (PLATFORM_STRSTR(ifs_names[j], interface_name) == NULL) {
								continue;
							}
							struct interfaceInfo *x;
							x = PLATFORM_GET_1905_INTERFACE_INFO(ifs_names[j]);

							if (x) {
								if (INTERFACE_POWER_STATE_OFF == x->power_state || PLATFORM_STRSTR(x->name, map_vxd_prefix)) {
									PLATFORM_FREE_1905_INTERFACE_INFO(x);
									continue;
								}
								struct LocalInterface *local_interface = DMnameToLocalInterfaceStruct(x->name);
								if (local_interface) {
									PLATFORM_METRIC_POLICY_IOCTL(local_interface->name,
									                             t->policies[k].sta_rcpi_threshold,
									                             t->policies[k].sta_rcpi_hysteresis_margin_override,
									                             t->policies[k].ap_channel_utilization_threshold);
									local_interface->metric_inclusion_policy = t->policies[k].policy;
								}

								PLATFORM_FREE_1905_INTERFACE_INFO(x);
							}
						}
					}
					PLATFORM_FREE_LIST_OF_1905_INTERFACES(ifs_names, ifs_nr);
				}

				break;
			}
#ifdef RTK_MULTI_AP_R2
			case TLV_TYPE_DEFAULT_802_1Q_SETTINGS: {
				struct Default8021QSettingsTLV *t = (struct Default8021QSettingsTLV *)p;
				PLATFORM_PRINTF_DEBUG("TLV_TYPE_DEFAULT_802_1Q_SETTINGS\n");
				PLATFORM_PRINTF_DEBUG("Primary VLAN ID : %d\n", t->primary_vlan_id);
				PLATFORM_PRINTF_DEBUG("Default pcp: %d\n", t->default_pcp);
				DMsetDefault8021QSettings(t->primary_vlan_id, t->default_pcp);
				vlan_tlv_nr += 1;
				vlan_tlvs = (INT8U **)PLATFORM_REALLOC(vlan_tlvs, sizeof(INT8U *) * (vlan_tlv_nr + 1));
				vlan_tlvs[vlan_tlv_nr -1] = p;
				vlan_tlvs[vlan_tlv_nr] = NULL;
				break;
			}
			case TLV_TYPE_TRAFFIC_SEPARATION_POLICY: {
				struct TrafficSeparationPolicyTLV *t = (struct TrafficSeparationPolicyTLV *)p;
				//TODO: Add handling for traffic separation policy
				PLATFORM_PRINTF_DEBUG("TLV_TRAFFIC_SEPARATION_POLICY\n");
				PLATFORM_PRINTF_DEBUG("Ssid nr: %d\n", t->ssid_nr);
				vlan_ssid_nr += t->ssid_nr;
				for (k = 0; k < t->ssid_nr; k++) {
					PLATFORM_PRINTF_DEBUG("Ssid length: %d\n", t->ssid_info[k].ssid_length);
					PLATFORM_PRINTF_DEBUG("Ssid name: %s\n", t->ssid_info[k].ssid_name);
					PLATFORM_PRINTF_DEBUG("Vlan id: %d\n", t->ssid_info[k].vlan_id);
				}
				DMsetTrafficSeparationPolicy(t);
				vlan_tlv_nr += 1;
				vlan_tlvs = (INT8U **)PLATFORM_REALLOC(vlan_tlvs, sizeof(INT8U *) * (vlan_tlv_nr + 1));
				vlan_tlvs[vlan_tlv_nr -1] = p;
				vlan_tlvs[vlan_tlv_nr] = NULL;
				break;
			}
			case TLV_TYPE_CHANNEL_SCAN_REPORTING_POLICY: {
				struct ChannelScanReportingPolicyTLV *t = (struct ChannelScanReportingPolicyTLV *)p;
				if (INDEPENDENT_SCAN_FLAG_MASK & t->independent_report_flag) {
					DMsetReportIndChannelScanFlag(REPORT_INDEPENDENT_SCAN_TRUE);
				} else {
					DMsetReportIndChannelScanFlag(REPORT_INDEPENDENT_SCAN_FALSE);
				}

				//For now assume we never do independent channel scan
				//TODO : Corresponding actions for performing independent channel scan

				//-----------Debug Code---------
				PLATFORM_PRINTF_DEBUG("Independent Report Flag : %02x\n", t->independent_report_flag);
				//------------------------------
				break;
			}
			case TLV_TYPE_UNSUCCESSFUL_ASSOCIATION_POILCY: {
				INT8U report_policy = 0;
				char buff[4] = {0};
				struct UnsuccessfulAssociationPolicyTLV *t = (struct UnsuccessfulAssociationPolicyTLV *)p;
				if(t->report_unsuccessful_associations & 0x80) {
					report_policy = 1;
				}

				DMsetReportUnsuccessfulAssociation(report_policy);
				PLATFORM_SNPRINTF(buff, sizeof(buff), "%d", report_policy);
				if (DMisTribandDevice()) {
					PLATFORM_SET_MIB(map_radio_name_5gh, "multiap_report_fail_assoc", buff);
				}
				PLATFORM_SET_MIB(map_radio_name_5gl, "multiap_report_fail_assoc", buff);
				PLATFORM_SET_MIB(map_radio_name_24g, "multiap_report_fail_assoc", buff);
				break;
			}
			case TLV_TYPE_BACKHAUL_BSS_CONFIGURATION: {
				// struct BackhaulBSSConfigurationTLV *t = (struct BackhaulBSSConfigurationTLV *)p;
				break;
			}
#endif
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		_updateControllerStatus(dst_mac, receiving_interface_name);
#ifdef RTK_MULTI_AP_R2
		if(MULTI_AP_PROFILE_2 <= DMgetMultiAPProfile() && NULL != vlan_tlvs) {
			api_send_tlvs_response(MAP_VLAN_CONFIGURATION_ELEMENT, src_addr, vlan_tlvs);
			DMsetEventFlag(FLAG_APPLY_VLAN);
		}
		if(NULL != vlan_tlvs) {
			PLATFORM_FREE(vlan_tlvs);
		}
#endif

		if(NULL != al_addr) {
			PLATFORM_FREE(al_addr);
		}
		break;
	}

	case CMDU_TYPE_HIGHER_LAYER_DATA: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_HIGHER_LAYER_DATA (%s) MID: %x\n", receiving_interface_name, c->message_id);

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Malformed structure.");
			break;
		}

		INT8U *p;
		int    i = 0;
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_HIGHER_LAYER_DATA: {
				// todo process the protocol and payload
				// struct HigherLayerDataTLV *t = (struct HigherLayerDataTLV *)p;
				break;
			}

			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);

				break;
			}
			}
			i++;
		}

		// if everything is OK, send ACK back

		INT8U *dst_mac;
		INT8U *al_addr = DMmacToAlMac(src_addr);

		if (NULL == al_addr) {
			dst_mac = src_addr;
		} else {
			dst_mac = al_addr;
		}
		// send ACK back without error code TLV
		if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_HIGHER_LAYER_DATA MID: (%x) is failed\n", c->message_id);
		}

		if (NULL != al_addr) {
			PLATFORM_FREE(al_addr);
		}
		break;
	}

	case CMDU_TYPE_1905_ACK: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_1905_ACK (%s) MID: %04x\n", receiving_interface_name, c->message_id);

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Malformed structure.");
			break;
		}

		INT8U *p;
		int    i = 0;
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_ERROR_CODE: {
				struct ErrorCodeTLV *t = (struct ErrorCodeTLV *)p;
				PLATFORM_PRINTF_WARNING("[Multi-AP] reason code is %02x\n", t->reason_code);
				// todo process the ErrorCodeTLV
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);

				break;
			}
			}
			i++;
		}

		if (PLATFORM_STRCMP("lo", receiving_interface_name)) {
			(void) waiting_msgs_ack(c->message_id);
		}

		break;
	}
	case CMDU_TYPE_AP_CAPABILITY_QUERY: {

		INT8U *p, *dst_mac;

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_AP_CAPABILITY_QUERY (%s) MID %04x\n", receiving_interface_name, c->message_id);

		p = DMmacToAlMac(src_addr);

		if (NULL == p) {
			// The standard says we should always send to the AL MAC
			// address, however, in these cases, instead of just dropping
			// the packet, sending the response to the 'src' address from
			// the METRICS QUERY seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the METRICS QUERY (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = p;
		}

		if (0 == sendAPCapabilityReportPacket(receiving_interface_name, c->message_id, dst_mac)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending message for CMDU_TYPE_AP_CAPABILITY_REPORT: (%x) is failed\n", c->message_id);
		}

		if (NULL != p) {
			PLATFORM_FREE(p);
		}

		break;
	}
	case CMDU_TYPE_AP_CAPABILITY_REPORT: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_AP_CAPABILITY_REPORT (%s) MID %04x\n", receiving_interface_name, c->message_id);

		INT8U *p;
		int    i = 0;

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("Malformed structure.");
			break;
		}

		api_send_tlvs_response(MAP_AP_CAPABILITY_REPORT_ELEMENT, src_addr, c->list_of_TLVs);

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_AP_CAPABILITY: {
				break;
			}
			case TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES: {
				break;
			}
			case TLV_TYPE_AP_HT_CAPABILITIES: {
				break;
			}
			case TLV_TYPE_AP_VHT_CAPABILITIES: {
				break;
			}
			case TLV_TYPE_AP_HE_CAPABILITIES: {
				break;
			}
#ifdef RTK_MULTI_AP_R2
			case TLV_TYPE_CHANNEL_SCAN_CAPABILITIES: {
				struct ChannelScanCapabilitiesTLV *t = (struct ChannelScanCapabilitiesTLV *)p;

				INT8U k, l, m;
				PLATFORM_PRINTF_DEBUG("CHANNEL_SCAN_CAPABILITES_TLV \n");
				PLATFORM_PRINTF_DEBUG("Radio nr : %d\n", t->radio_nr);
				for (k = 0; k < t->radio_nr; k++) {
					PLATFORM_PRINTF_DEBUG("Radio id : %02x:%02x:%02x:%02x:%02x:%02x\n", t->radios[k].radio_unique_identifier[0], t->radios[k].radio_unique_identifier[1], t->radios[k].radio_unique_identifier[2], t->radios[k].radio_unique_identifier[3], t->radios[k].radio_unique_identifier[4], t->radios[k].radio_unique_identifier[5]);
					PLATFORM_PRINTF_DEBUG("scan_capability_flags : %02x\n", t->radios[k].scan_capability_flags);
					PLATFORM_PRINTF_DEBUG("minimum_scan_interval : %d\n", t->radios[k].minimum_scan_interval);
					PLATFORM_PRINTF_DEBUG("op_class_nr : %d\n", t->radios[k].op_class_nr);
					for (l = 0; l < t->radios[k].op_class_nr; l++) {
						PLATFORM_PRINTF_DEBUG("op_class : %d\n", t->radios[k].op_classes[l].op_class);
						PLATFORM_PRINTF_DEBUG("channel_nr : %d\n", t->radios[k].op_classes[l].channel_nr);
						PLATFORM_PRINTF_DEBUG("channel_list : \n");
						for (m = 0; m < t->radios[k].op_classes[l].channel_nr; m++) {
							PLATFORM_PRINTF_DEBUG("%d \n", t->radios[k].op_classes[l].channel_list[m]);
						}
						PLATFORM_PRINTF_DEBUG("\n");
					}
				}

				//Update datamodel
				if (DMsetChannelScanCapabilities(t, src_addr)) {
					PLATFORM_PRINTF_WARNING("Channel Scan Capablities received from unknown device. Skip storing...\n");
				}
				break;
			}
			case TLV_TYPE_CAC_CAPABILITIES: {
				struct CACCapabilitiesTLV *t = (struct CACCapabilitiesTLV *)p;

				INT8U k, l, m, n;
				PLATFORM_PRINTF_DEBUG("CAC_CAPABILITES_TLV \n");
				PLATFORM_PRINTF_DEBUG("Country code : %c%c\n", t->country_code[0], t->country_code[1]);
				PLATFORM_PRINTF_DEBUG("Radio nr : %d\n", t->radio_nr);
				for (k = 0; k < t->radio_nr; k++) {
					PLATFORM_PRINTF_DEBUG("Radio id : %02x:%02x:%02x:%02x:%02x:%02x\n", t->radios[k].radio_unique_identifier[0], t->radios[k].radio_unique_identifier[1], t->radios[k].radio_unique_identifier[2], t->radios[k].radio_unique_identifier[3], t->radios[k].radio_unique_identifier[4], t->radios[k].radio_unique_identifier[5]);
					PLATFORM_PRINTF_DEBUG("type_nr : %d\n", t->radios[k].type_nr);
					for (l = 0; l < t->radios[k].type_nr; l++) {
						PLATFORM_PRINTF_DEBUG("flags : %d\n", t->radios[k].types[l].flags);
						PLATFORM_PRINTF_DEBUG("second_nr : %02x:%02x:%02x\n", t->radios[k].types[l].second_nr[0], t->radios[k].types[l].second_nr[1], t->radios[k].types[l].second_nr[2]);
						PLATFORM_PRINTF_DEBUG("op_class_nr : %d\n", t->radios[k].types[l].op_class_nr);
						for (m = 0; m < t->radios[k].types[l].op_class_nr; m++) {
							PLATFORM_PRINTF_DEBUG("op_class: %d\n", t->radios[k].types[l].op_classes[m].op_class);
							PLATFORM_PRINTF_DEBUG("channel_nr: %d\n", t->radios[k].types[l].op_classes[m].channel_nr);
							PLATFORM_PRINTF_DEBUG("channels : ");
							for (n = 0; n < t->radios[k].types[l].op_classes[m].channel_nr; n++) {
								PLATFORM_PRINTF_DEBUG(" %d", t->radios[k].types[l].op_classes[m].channels[n]);
							}
							PLATFORM_PRINTF_DEBUG("\n");
						}
					}
				}

				break;
			}
#endif
#ifdef RTK_MULTI_AP_R3
			case TLV_TYPE_AP_WIFI_6_CAPABILITIES: {
				break;
			}
			case TLV_TYPE_DEVICE_INVENTORY: {
				break;
			}
#endif
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);

				break;
			}
			}
			i++;
		}

		break;
	}
	case CMDU_TYPE_CLIENT_CAPABILITY_QUERY: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_CLIENT_CAPABILITY_QUERY (%s) MID %04x\n", receiving_interface_name, c->message_id);

		INT8U *p, *dst_mac;
		INT8U  clientMac[6] = { 0 }, bssid[6] = { 0 };
		int    i = 0;

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_CLIENT_INFO: {
				struct ClientInfoTLV *t = (struct ClientInfoTLV *)p;
				PLATFORM_PRINTF_DEBUG("CLIENT_INFO bssid - %02x%02x%02x%02x%02x%02x\n",
				                      t->bssid[0], t->bssid[1], t->bssid[2], t->bssid[3], t->bssid[4], t->bssid[5]);
				PLATFORM_PRINTF_DEBUG("CLIENT_INFO mac - %02x%02x%02x%02x%02x%02x\n",
				                      t->mac_address[0], t->mac_address[1], t->mac_address[2], t->mac_address[3], t->mac_address[4], t->mac_address[5]);

				PLATFORM_MEMCPY(bssid, t->bssid, 6);
				PLATFORM_MEMCPY(clientMac, t->mac_address, 6);
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);

				break;
			}
			}
			i++;
		}

		p = DMmacToAlMac(src_addr);

		if (NULL == p) {
			// The standard says we should always send to the AL MAC
			// address, however, in these cases, instead of just dropping
			// the packet, sending the response to the 'src' address from
			// the METRICS QUERY seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the METRICS QUERY (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = p;
		}

		if (0 == sendClientCapabilityReportPacket(receiving_interface_name, c->message_id, dst_mac, bssid, clientMac)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending message for CMDU_TYPE_AP_CAPABILITY_REPORT: (%x) is failed\n", c->message_id);
		}

		if (NULL != p) {
			PLATFORM_FREE(p);
		}

		break;
	}
	case CMDU_TYPE_CLIENT_CAPABILITY_REPORT: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_CLIENT_CAPABILITY_REPORT (%s) MID %04x\n", receiving_interface_name, c->message_id);
		INT8U *p;
		int    i = 0;

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("Malformed structure.");
			break;
		}

		api_send_tlvs_response(MAP_CLIENT_CAPABILITY_REPORT_ELEMENT, src_addr, c->list_of_TLVs);

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_CLIENT_INFO: {
				struct ClientInfoTLV *t = (struct ClientInfoTLV *)p;
				PLATFORM_PRINTF_DEBUG("CLIENT_INFO bssid - %02x%02x%02x%02x%02x%02x\n",
				                t->bssid[0], t->bssid[1], t->bssid[2], t->bssid[3], t->bssid[4], t->bssid[5]);
				PLATFORM_PRINTF_DEBUG("CLIENT_INFO mac - %02x%02x%02x%02x%02x%02x\n",
				                t->mac_address[0], t->mac_address[1], t->mac_address[2], t->mac_address[3], t->mac_address[4], t->mac_address[5]);
				break;
			}
			case TLV_TYPE_CLIENT_CAPABLITY_REPORT: {
				struct ClientCapabilityReportTLV *t = (struct ClientCapabilityReportTLV *)p;
				PLATFORM_PRINTF_DEBUG("CLIENT_CAPABILITY_REPORT result code: %d\n", t->result_code);
				break;
			}
			case TLV_TYPE_ERROR_CODE: {
				struct ErrorCodeTLV *t = (struct ErrorCodeTLV *)p;
				PLATFORM_PRINTF_WARNING("[Multi-AP] reason code is %02x\n", t->reason_code);
				// todo process the ErrorCodeTLV
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);

				break;
			}
			}
			i++;
		}

		break;
	}
	case CMDU_TYPE_AP_METRICS_QUERY: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_AP_METRICS_QUERY (%s) MID %04x\n", receiving_interface_name, c->message_id);

		INT8U *p, *dst_mac;
		int    i = 0;
		INT8U  k, bssid_nr = 0, radio_nr = 0;
		INT8U(*bssid_list)[6] = NULL;
		INT8U (*radio_mac_list)[6] = NULL;

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_AP_METRIC_QUERY: {
				struct APMetricQueryTLV *t = (struct APMetricQueryTLV *)p;
				PLATFORM_PRINTF_DEBUG("%s - TLV TYPE AP METRIC QUERY\n", __FUNCTION__);
				PLATFORM_PRINTF_DEBUG("%s - Bssid nr: %d\n", __FUNCTION__, t->bssid_nr);
				for (k = 0; k < t->bssid_nr; k++) {
					bssid_nr++;
					bssid_list = PLATFORM_REALLOC(bssid_list, sizeof(*(bssid_list)) * bssid_nr);
					PLATFORM_MEMCPY(bssid_list[bssid_nr - 1], t->bssid[k], 6);
					PLATFORM_PRINTF_DEBUG("%s - Receive AP Metric Request for %02x%02x%02x%02x%02x%02x\n", __FUNCTION__,
					                t->bssid[k][0], t->bssid[k][1], t->bssid[k][2], t->bssid[k][3], t->bssid[k][4], t->bssid[k][5]);
				}
				break;
			}
			case TLV_TYPE_AP_RADIO_IDENTIFIER: {
				PLATFORM_PRINTF_DEBUG("%s - TLV TYPE AP RADIO IDENTIFIER\n", __FUNCTION__);
				struct ApRadioIdentifierTLV *t = (struct ApRadioIdentifierTLV *)p;
				radio_nr++;
				radio_mac_list = PLATFORM_REALLOC(radio_mac_list, sizeof(*(radio_mac_list)) * radio_nr);
				PLATFORM_MEMCPY(radio_mac_list[radio_nr - 1], t->radio_unique_id, 6);
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		p = DMmacToAlMac(src_addr);

		if (NULL == p) {
			// The standard says we should always send to the AL MAC
			// address, however, in these cases, instead of just dropping
			// the packet, sending the response to the 'src' address from
			// the METRICS QUERY seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the METRICS QUERY (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = p;
		}

		sendAPMetricResponsePacket(receiving_interface_name, c->message_id, dst_mac, bssid_nr, bssid_list, radio_nr, radio_mac_list);

		if (bssid_nr) {
			PLATFORM_FREE(bssid_list);
		}

		if (radio_nr) {
			PLATFORM_FREE(radio_mac_list);
		}

		if (NULL != p) {
			PLATFORM_FREE(p);
		}

		break;
	}
	case CMDU_TYPE_AP_METRICS_RESPONSE: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_AP_METRICS_RESPONSE (%s) MID %04x\n", receiving_interface_name, c->message_id);

		INT8U *p;
		int    i = 0;

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("Malformed structure.");
			break;
		}

		api_send_tlvs_response(MAP_AP_METRIC_RESPONSE_ELEMENT, src_addr, c->list_of_TLVs);

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_AP_METRICS: {
				struct APMetricsTLV *t = (struct APMetricsTLV *)p;

				PLATFORM_PRINTF_DETAIL("AP Metrics TLV bssid - %02x%02x%02x%02x%02x%02x\n",
				                       t->bssid[0], t->bssid[1], t->bssid[2], t->bssid[3], t->bssid[4], t->bssid[5]);
				PLATFORM_PRINTF_DETAIL("AP Metrics TLV ch util - %d\n", t->ch_util);
				PLATFORM_PRINTF_DETAIL("AP Metrics TLV sta nr - %d\n", t->sta_nr);
				DMupdateNetworkDeviceAPMetric(src_addr, p);
				break;
			}
			case TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS: {
				// PLATFORM_PRINTF_DEBUG("Received Associated Sta Traffic Stats TLV\n");
				// struct AssociatedSTATrafficStatsTLV *t = (struct AssociatedSTATrafficStatsTLV *)p;
				// To allow the controller to use the same byte counter units (as agent sending the packet) to interpret
				DMsetByteCounterUnits(DMgetTargetDeviceByteCounterUnits(src_addr));
				break;
			}
			case TLV_TYPE_ASSOCIATED_STA_LINK_METRICS: {
				// PLATFORM_PRINTF_DEBUG("Received Associated Sta Link Metric TLV\n");
				// struct AssociatedSTALinkMetricsTLV *t = (struct AssociatedSTALinkMetricsTLV *)p;
				break;
			}
			case TLV_TYPE_AP_EXTENDED_METRICS: {
				// To allow the controller to use the same byte counter units (as agent sending the packet) to interpret
				DMsetByteCounterUnits(DMgetTargetDeviceByteCounterUnits(src_addr));
				break;
			}
			case TLV_TYPE_RADIO_METRICS: {
				break;
			}
			case TLV_TYPE_ASSOCIATED_STA_EXTENDED_LINK_METRICS: {
				break;
			}
#ifdef RTK_MULTI_AP_R3
			case TLV_TYPE_ASSOCIATED_WIFI_6_STA_STATUS_REPORT: {
				break;
			}
#endif
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}
		break;
	}
	case CMDU_TYPE_ASSOCIATED_STA_LINK_METRICS_QUERY: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_ASSOCIATED_STA_LINK_METRICS_QUERY (%s) MID %04x\n", receiving_interface_name, c->message_id);

		INT8U *p, *dst_mac;
		int    i = 0;

		INT8U(*client_macs)[6] = NULL;
		INT8U client_mac_nr    = 0;

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_STA_MAC_ADDRESS_TYPE: {
				struct STAMacAddressTypeTLV *t = (struct STAMacAddressTypeTLV *)p;
				//send STA Link Metric Response
				client_mac_nr += 1;
				client_macs = PLATFORM_REALLOC(client_macs, sizeof(*(client_macs)) * client_mac_nr);
				PLATFORM_MEMCPY(client_macs[client_mac_nr - 1], t->mac_address, 6);
				PLATFORM_PRINTF_DEBUG("Client MAC: %02x%02x%02x%02x%02x%02x\n",
				                      client_macs[i][0], client_macs[i][1], client_macs[i][2],
				                      client_macs[i][3], client_macs[i][4], client_macs[i][5]);
				break;
			}
			case TLV_TYPE_ERROR_CODE: {
				struct ErrorCodeTLV *t = (struct ErrorCodeTLV *)p;
				PLATFORM_PRINTF_WARNING("[Multi-AP] reason code is %02x\n", t->reason_code);
				// todo process the ErrorCodeTLV
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		p = DMmacToAlMac(src_addr);

		if (NULL == p) {
			// The standard says we should always send to the AL MAC
			// address, however, in these cases, instead of just dropping
			// the packet, sending the response to the 'src' address from
			// the METRICS QUERY seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the METRICS QUERY (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = p;
		}

		for (i = 0; i < client_mac_nr; i++) {
			sendAssocSTALinkMetricResponsePacket(receiving_interface_name, c->message_id, dst_mac, client_macs[i]);
		}

		if(client_mac_nr) {
			PLATFORM_FREE(client_macs);
		}

		if (NULL != p) {
			PLATFORM_FREE(p);
		}

		break;
	}
	case CMDU_TYPE_ASSOCIATED_STA_LINK_METRICS_RESPONSE: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_ASSOCIATED_STA_LINK_METRICS_RESPONSE (%s) MID %04x\n", receiving_interface_name, c->message_id);

		INT8U *p;
		int    i = 0, j;

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("Malformed structure.");
			break;
		}

		api_send_tlvs_response(MAP_ASSOC_STA_METRIC_RESPONSE_ELEMENT, src_addr, c->list_of_TLVs);

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_ASSOCIATED_STA_LINK_METRICS: {
				//PLATFORM_PRINTF_DEBUG("STA Link Metric TLV\n");
				struct AssociatedSTALinkMetricsTLV *t = (struct AssociatedSTALinkMetricsTLV *)p;
				//process STA Link Metric Response
				PLATFORM_PRINTF_DEBUG("Client MAC: %02x%02x%02x%02x%02x%02x\n",
				                      t->mac_address[0], t->mac_address[1], t->mac_address[2],
				                      t->mac_address[3], t->mac_address[4], t->mac_address[5]);
				PLATFORM_PRINTF_DEBUG("BSSID nr: %d\n", t->bssid_nr);

				if (t->bssid_nr > 0) {
					for (j = 0; j < t->bssid_nr; j++) {
						PLATFORM_PRINTF_DEBUG("BSSID: %02x%02x%02x%02x%02x%02x\n",
						                      t->assoc_sta_link_metrics[j].bssid[0], t->assoc_sta_link_metrics[j].bssid[1], t->assoc_sta_link_metrics[j].bssid[2],
						                      t->assoc_sta_link_metrics[j].bssid[3], t->assoc_sta_link_metrics[j].bssid[4], t->assoc_sta_link_metrics[j].bssid[5]);
						PLATFORM_PRINTF_DEBUG("Time delta: %d\n", t->assoc_sta_link_metrics[j].time_delta);
						PLATFORM_PRINTF_DEBUG("Downlink data rate: %d\n", t->assoc_sta_link_metrics[j].dataRate_downlink);
						PLATFORM_PRINTF_DEBUG("Uplink data rate: %d\n", t->assoc_sta_link_metrics[j].dataRate_uplink);
						PLATFORM_PRINTF_DEBUG("Uplink rcpi: %d\n", t->assoc_sta_link_metrics[j].uplink_rcpi);
					}
				}

				break;
			}
			case TLV_TYPE_ERROR_CODE: {
				struct ErrorCodeTLV *t = (struct ErrorCodeTLV *)p;
				PLATFORM_PRINTF_WARNING("[Multi-AP] reason code is %02x\n", t->reason_code);
				// todo process the ErrorCodeTLV
				break;
			}
			case TLV_TYPE_ASSOCIATED_STA_EXTENDED_LINK_METRICS: {
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}
		break;
	}
	case CMDU_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY (%s) MID %04x\n", receiving_interface_name, c->message_id);

		INT8U *p, *dst_mac;
		int    i       = 0, k, j;
		INT8U *al_addr = DMmacToAlMac(src_addr);

		if (NULL == al_addr) {
			// The standard says we should always send to the AL MAC
			// address, however, in these cases, instead of just dropping
			// the packet, sending the response to the 'src' address from
			// the METRICS QUERY seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the METRICS QUERY (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = al_addr;
		}

		_updateControllerStatus(dst_mac, receiving_interface_name);

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY: {
				struct UnassociatedSTALinkMetricsQueryTLV *t = (struct UnassociatedSTALinkMetricsQueryTLV *)p;

				PLATFORM_PRINTF_DEBUG("Received Unassociated sta link metrics query TLV\n");

				PLATFORM_PRINTF_DEBUG("Op class: %d\n", t->op_class);

				for (k = 0; k < t->channel_list_nr; k++) {
					PLATFORM_PRINTF_DEBUG("Channel number: %d\n", t->channel_list_entry[k].channel_nr);
					PLATFORM_PRINTF_DEBUG("Sta nr: %d\n", t->channel_list_entry[k].sta_nr);
					for (j = 0; j < t->channel_list_entry[k].sta_nr; j++) {
						PLATFORM_PRINTF_DEBUG("Sta %02x%02x%02x%02x%02x%02x\n",
						                      t->channel_list_entry[k].sta_mac_address[j][0],
						                      t->channel_list_entry[k].sta_mac_address[j][1],
						                      t->channel_list_entry[k].sta_mac_address[j][2],
						                      t->channel_list_entry[k].sta_mac_address[j][3],
						                      t->channel_list_entry[k].sta_mac_address[j][4],
						                      t->channel_list_entry[k].sta_mac_address[j][5]);
					}
					if (t->channel_list_entry[k].channel_nr < 12)
						PLATFORM_UNASSOC_STA_IOCTL(map_radio_name_24g, t->op_class, t->channel_list_entry[k].channel_nr, t->channel_list_entry[k].sta_nr, t->channel_list_entry[k].sta_mac_address);
					else if (t->channel_list_entry[k].channel_nr < 100)
						PLATFORM_UNASSOC_STA_IOCTL(map_radio_name_5gl, t->op_class, t->channel_list_entry[k].channel_nr, t->channel_list_entry[k].sta_nr, t->channel_list_entry[k].sta_mac_address);
					else
						PLATFORM_UNASSOC_STA_IOCTL(map_radio_name_5gh, t->op_class, t->channel_list_entry[k].channel_nr, t->channel_list_entry[k].sta_nr, t->channel_list_entry[k].sta_mac_address);
				}

				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		// send ACK back without error code TLV
		if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY MID: (%x) is failed\n", c->message_id);
		}

		if (NULL != al_addr) {
			PLATFORM_FREE(al_addr);
		}

		break;
	}
	case CMDU_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE (%s) MID %04x\n", receiving_interface_name, c->message_id);

		INT8U *p;
		int    i = 0, k;

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("Malformed structure.");
			break;
		}

		api_send_tlvs_response(MAP_UNASSOC_STA_METRIC_RESPONSE_ELEMENT, src_addr, c->list_of_TLVs);

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE: {
				struct UnassociatedSTALinkMetricsResponseTLV *t = (struct UnassociatedSTALinkMetricsResponseTLV *)p;
				//Process Unassociated STA Link Metric Response
				PLATFORM_PRINTF_DEBUG("Received Unassociated sta link metrics report TLV\n");
				PLATFORM_PRINTF_DEBUG("Op class: %d\n", t->op_class);
				PLATFORM_PRINTF_DEBUG("Sta nr: %d\n", t->sta_nr);
				for (k = 0; k < t->sta_nr; k++) {
					PLATFORM_PRINTF_DEBUG("Sta mac: %02x%02x%02x%02x%02x%02x\n",
					                      t->unassoc_sta_link_metrics[k].sta_mac_address[0], t->unassoc_sta_link_metrics[k].sta_mac_address[1], t->unassoc_sta_link_metrics[k].sta_mac_address[2],
					                      t->unassoc_sta_link_metrics[k].sta_mac_address[3], t->unassoc_sta_link_metrics[k].sta_mac_address[4], t->unassoc_sta_link_metrics[k].sta_mac_address[5]);
					PLATFORM_PRINTF_DEBUG("Channel number: %d\n", t->unassoc_sta_link_metrics[k].channel_number);
					PLATFORM_PRINTF_DEBUG("Time delta: %d\n", t->unassoc_sta_link_metrics[k].time_delta);
					PLATFORM_PRINTF_DEBUG("Uplink rcpi: %d\n", t->unassoc_sta_link_metrics[k].uplink_rcpi);
				}
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}
		break;
	}
	case CMDU_TYPE_CHANNEL_PREFERENCE_QUERY: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_MULTI_AP_CHANNEL_PREFERENCE_QUERY (%s) MID: %04x\n", receiving_interface_name, c->message_id);

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Malformed structure.");
			break;
		}

		INT8U *dst_mac;
		INT8U *al_addr = DMmacToAlMac(src_addr);

		if (NULL == al_addr) {
			dst_mac = src_addr;
		} else {
			dst_mac = al_addr;
		}
		// send Multi-AP Channel Preference Report Packet
		if (0 == sendMultiAPChannelPreferenceReportPacket(receiving_interface_name, c->message_id, dst_mac, NULL, false)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending REPORT for CMDU_TYPE_MULTI_AP_CHANNEL_PREFERENCE_QUERY MID: (%04x) is failed\n", c->message_id);
		}

		if (NULL != al_addr) {
			PLATFORM_FREE(al_addr);
		}
		break;
	}
	case CMDU_TYPE_CHANNEL_PREFERENCE_REPORT: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_MULTI_AP_CHANNEL_PREFERENCE_REPORT (%s) MID: %04x\n", receiving_interface_name, c->message_id);

		if (DMchannelQueryStatusGet() == 1 && waiting_msgs_ack(c->message_id) == 1 && PLATFORM_STRCMP("lo", receiving_interface_name)) {
			PLATFORM_PRINTF_WARNING("[Multi-AP] This Channel Preference Report [%04x] is timeout.\n", c->message_id);
			break;
		}

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Malformed structure.");
			break;
		}

		api_send_tlvs_response(MAP_CHANNEL_PREFERENCE_REPORT_ELEMENT, src_addr, c->list_of_TLVs);

		INT8U *p;
		int    i       = 0;
		INT8U *al_addr = DMmacToAlMac(src_addr);
		if (NULL == al_addr) {
			al_addr = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * 6);
			PLATFORM_MEMCPY(al_addr, src_addr, 6);
		}
		struct networkDevice *tmp = DMnetworkDeviceGet(al_addr);
		if (DMgetWFATestMode()) {
			if (tmp != NULL && tmp->last_ch_prefs_nr > 0) {
				for (i = 0; i < tmp->last_ch_prefs_nr; i++) {
					free_1905_TLV_structure((INT8U *)tmp->last_ch_prefs[i]);
				}
				PLATFORM_FREE(tmp->last_ch_prefs);
				tmp->last_ch_prefs = NULL;
				tmp->last_ch_prefs_nr = 0;
			}
		}
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_CHANNEL_PREFERENCE: {
				// struct ChannelPreferenceTLV *t = (struct ChannelPreferenceTLV *)p;
				// PLATFORM_PRINTF_DEBUG("[MULTI_AP] receive Channel Preference TLV with radio unique id: %02x:%02x:%02x:%02x:%02x:%02x\n",
				//                       t->radio_unique_id[0],
				//                       t->radio_unique_id[1],
				//                       t->radio_unique_id[2],
				//                       t->radio_unique_id[3],
				//                       t->radio_unique_id[4],
				//                       t->radio_unique_id[5]);
				if (DMgetWFATestMode()) {
					if (tmp == NULL) break;
					if (tmp->last_ch_prefs_nr == 0) {
						tmp->last_ch_prefs = (struct ChannelPreferenceTLV **)PLATFORM_MALLOC(sizeof(struct ChannelPreferenceTLV *));
					}
					tmp->last_ch_prefs_nr++;
					tmp->last_ch_prefs = (struct ChannelPreferenceTLV **)PLATFORM_REALLOC(tmp->last_ch_prefs, tmp->last_ch_prefs_nr * sizeof(struct ChannelPreferenceTLV *));
					tmp->last_ch_prefs[tmp->last_ch_prefs_nr - 1] = (struct ChannelPreferenceTLV *)copy_1905_TLV_structure(p);
				}
				break;
			}
			case TLV_TYPE_RADIO_OPERATION_RESTRICTION: {
				// todo process the protocol and payload
				break;
			}
			case TLV_TYPE_CAC_COMPLETION_REPORT: {
				PLATFORM_PRINTF_INFO("[MULTI_AP] receive TLV_TYPE_CAC_COMPLETION_REPORT \n");
				struct CACCompletionReportTLV *t = (struct CACCompletionReportTLV *)p;
				PLATFORM_PRINTF_DETAIL("radio_nr: %d\n", t->radio_nr);
				INT8U k, l;
				for (k = 0; k < t->radio_nr; k++) {
					PLATFORM_PRINTF_DETAIL("radio_unique_identifier: %02x:%02x:%02x:%02x:%02x:%02x\n",
					                      t->radios[k].radio_unique_identifier[0],
					                      t->radios[k].radio_unique_identifier[1],
					                      t->radios[k].radio_unique_identifier[2],
					                      t->radios[k].radio_unique_identifier[3],
					                      t->radios[k].radio_unique_identifier[4],
					                      t->radios[k].radio_unique_identifier[5]);
					PLATFORM_PRINTF_DETAIL("op_class: %d\n", t->radios[k].op_class);
					PLATFORM_PRINTF_DETAIL("channel: %d\n", t->radios[k].channel);
					PLATFORM_PRINTF_DETAIL("flags: %d\n", t->radios[k].flags);
					PLATFORM_PRINTF_DETAIL("pairs_nr: %d\n", t->radios[k].pairs_nr);
					for (l = 0; l < t->radios[k].pairs_nr; l++) {
						PLATFORM_PRINTF_DETAIL("pairs_op_class: %d\n", t->radios[k].pairs[l].pairs_op_class);
						PLATFORM_PRINTF_DETAIL("pairs_channel: %d\n", t->radios[k].pairs[l].pairs_channel);
					}
				}
				break;
			}
			case TLV_TYPE_CAC_STATUS_REPORT: {
				PLATFORM_PRINTF_INFO("[MULTI_AP] receive TLV_TYPE_CAC_STATUS_REPORT \n");
				struct CACStatusReportTLV *t = (struct CACStatusReportTLV *)p;
				PLATFORM_PRINTF_DETAIL("available_channel_nr: %d\n", t->available_channel_nr);
				INT8U k;
				for (k = 0; k < t->available_channel_nr; k++) {
					PLATFORM_PRINTF_DETAIL("ac_op_class: %d\n", t->available_channels[k].ac_op_class);
					PLATFORM_PRINTF_DETAIL("ac_channel: %d\n", t->available_channels[k].ac_channel);
					PLATFORM_PRINTF_DETAIL("identify_time: %d\n", t->available_channels[k].identify_time);
				}
				PLATFORM_PRINTF_DETAIL("nonoccup_pair_nr: %d\n", t->nonoccup_pair_nr);
				for (k = 0; k < t->nonoccup_pair_nr; k++) {
					PLATFORM_PRINTF_DETAIL("nonoccup_op_class: %d\n", t->nonoccup_pairs[k].nonoccup_op_class);
					PLATFORM_PRINTF_DETAIL("nonoccup_channel: %d\n", t->nonoccup_pairs[k].nonoccup_channel);
					PLATFORM_PRINTF_DETAIL("nonoccup_remaining_time: %d\n", t->nonoccup_pairs[k].nonoccup_remaining_time);
				}
				PLATFORM_PRINTF_DETAIL("active_pair_nr: %d\n", t->active_pair_nr);
				for (k = 0; k < t->active_pair_nr; k++) {
					PLATFORM_PRINTF_DETAIL("active_op_class: %d\n", t->active_pairs[k].active_op_class);
					PLATFORM_PRINTF_DETAIL("active_channel: %d\n", t->active_pairs[k].active_channel);
					int active_remaining_time = (t->active_pairs[k].active_remaining_time[0] << 16) + (t->active_pairs[k].active_remaining_time[1] << 8) + t->active_pairs[k].active_remaining_time[2];
					PLATFORM_PRINTF_DETAIL("active_remaining_time: %d\n", active_remaining_time);
				}
				break;
			}

			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}
		// This channel preference report is not unsolicited
		if (DMchannelQueryStatusGet() == 1) {
			PLATFORM_PRINTF_INFO("[Multi-AP] Solicited channel preference report\n");
			DMchannelQueryStatusSet(0);
		} else if (DMchannelQueryStatusGet() == 0) {
			// This channel preference report is unsolicited
			PLATFORM_PRINTF_INFO("[Multi-AP] Unsolicited channel preference report\n");
			INT8U *dst_mac;
			if (NULL == al_addr) {
				dst_mac = src_addr;
			} else {
				dst_mac = al_addr;
			}
			// send ACK back without error code TLV
			if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
				PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_CHANNEL_PREFERENCE_REPORT MID: (%x) is failed\n", c->message_id);
			}
		}

		if (NULL != al_addr) {
			PLATFORM_FREE(al_addr);
		}
		break;
	}
	case CMDU_TYPE_CHANNEL_SELECTION_REQUEST: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_MULTI_AP_CHANNEL_SELECTION_REQUEST (%s) MID: %04x\n", receiving_interface_name, c->message_id);

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Malformed structure.");
			break;
		}

		INT8U *p;
		int    i        = 0;
		INT8U  response = 0;
		INT8U *dst_mac;
		INT8U *al_addr = DMmacToAlMac(src_addr);
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_CHANNEL_PREFERENCE: {
				struct ChannelPreferenceTLV *t = (struct ChannelPreferenceTLV *)p;
				PLATFORM_PRINTF_DEBUG("[MULTI_AP] receive Channel Preference TLV with radio unique id: %02x:%02x:%02x:%02x:%02x:%02x\n",
				                      t->radio_unique_id[0],
				                      t->radio_unique_id[1],
				                      t->radio_unique_id[2],
				                      t->radio_unique_id[3],
				                      t->radio_unique_id[4],
				                      t->radio_unique_id[5]);
				response |= DMupdateControllerChannelPreference(t);
				break;
			}
			case TLV_TYPE_TRANSMIT_POWER_LIMIT: {
				struct TransmitPowerLimitTLV *t = (struct TransmitPowerLimitTLV *)p;
				PLATFORM_PRINTF_INFO("[MULTI_AP] receive Transmit Power Limit TLV (%d dBm) with radio unique id: %02x:%02x:%02x:%02x:%02x:%02x\n",
				                     t->transmit_power_limit,
				                     t->radio_unique_id[0],
				                     t->radio_unique_id[1],
				                     t->radio_unique_id[2],
				                     t->radio_unique_id[3],
				                     t->radio_unique_id[4],
				                     t->radio_unique_id[5]);
				response |= DMchangeTransmitPower(p);
				break;
			}
#ifdef RTK_MULTI_AP_R4
			case TLV_TYPE_SPATIAL_REUSE_REQUEST: {
				if (DMgetMultiAPProfile() >= MULTI_AP_PROFILE_4) {
					struct SpatialReuseRequestTLV *t = (struct SpatialReuseRequestTLV *)p;
					PLATFORM_PRINTF_INFO("[MULTI_AP] receive Spatial Reuse Request TLV with radio unique id: %02x:%02x:%02x:%02x:%02x:%02x\n",
										t->ruid[0],
										t->ruid[1],
										t->ruid[2],
										t->ruid[3],
										t->ruid[4],
										t->ruid[5]);
					char *interface_name = NULL;
					interface_name       = DMmacToInterfaceName(t->ruid);
					PLATFORM_MAP_APPLY_SPATIAL_REUSE_CONFIG(t, interface_name);
				}
				break;
			}
#endif //RTK_MULTI_AP_R4
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		if (NULL == al_addr) {
			dst_mac = src_addr;
		} else {
			dst_mac = al_addr;
		}
		// send Multi-AP Channel Selection Response Packet
		if (!DMgetWFATestMode() || (DMgetMultiAPProfile() != MULTI_AP_PROFILE_2)) {
			//Ignore the send multiap channel selection resp R2 logo due to IOT issue with MTK
			if (0 == sendMultiAPChannelSelectionResponsePacket(receiving_interface_name, c->message_id, dst_mac, response)) {
				PLATFORM_PRINTF_ERROR("[Multi-AP] Sending Response for CMDU_TYPE_MULTI_AP_CHANNEL_SELECTION_REQUEST MID: (%04x) is failed\n", c->message_id);
			}
		}

		// send Multi-AP Operating Channel Report Packet if the agent accepts channel selection request
		if (0 == response) {
			// switch to the best channel
			DMswitchToBestChannel();
			// set the tx_power
			// ...
			if (0 == sendMultiAPOperatingChannelReportPacket(receiving_interface_name, getNextMid(), dst_mac)) {
				PLATFORM_PRINTF_ERROR("[Multi-AP] Sending Response for CMDU_TYPE_MULTI_AP_CHANNEL_SELECTION_REQUEST MID: (%04x) is failed\n", c->message_id);
			}
		}

		if (NULL != al_addr) {
			PLATFORM_FREE(al_addr);
		}
		break;
	}
	case CMDU_TYPE_CHANNEL_SELECTION_RESPONSE: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_MULTI_AP_CHANNEL_SELECTION_RESPONSE (%s) MID: %04x\n", receiving_interface_name, c->message_id);

		if (waiting_msgs_ack(c->message_id) == 1 && PLATFORM_STRCMP("lo", receiving_interface_name)) {
			PLATFORM_PRINTF_WARNING("[Multi-AP] This Channel Selection Response packet [%04x] is timeout.\n", c->message_id);
			break;
		}

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Malformed structure.");
			break;
		}

		api_send_tlvs_response(MAP_CHANNEL_SELECTION_RESPONSE_ELEMENT, src_addr, c->list_of_TLVs);

		INT8U *p;
		int    i = 0;
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_CHANNEL_SELECTION_RESPONSE: {
				// todo process the protocol and payload
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		break;
	}
	case CMDU_TYPE_BEACON_METRICS_QUERY: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_BEACON_METRICS_QUERY (%s) MID %04x\n", receiving_interface_name, c->message_id);

		INT8U *p, *dst_mac;
		INT8U  reason_code = 0;
		INT8U  empty_mac[6]  = { 0 };
		INT8U  client_mac[6] = { 0 };
		INT8U  radio_mac[6]  = { 0 };
		char * interface_name;
		int    i = 0, k;

		struct BeaconMeasurementRequest beacon_req;
		PLATFORM_MEMSET(&beacon_req, 0, sizeof(struct BeaconMeasurementRequest));
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_BEACON_METRICS_QUERY: {
				struct BeaconMetricsQueryTLV *t = (struct BeaconMetricsQueryTLV *)p;
				//call driver to do 11k beacon query
				PLATFORM_MEMCPY(client_mac, t->mac_address, MACADDRLEN);
				beacon_req.op_class = t->op_class;
				beacon_req.channel  = t->channel_number;
				PLATFORM_MEMCPY(beacon_req.bssid, t->bssid, MACADDRLEN);
				beacon_req.report_detail = t->report_detail;

				if (t->ssid_len > 0) {
					PLATFORM_MEMCPY(beacon_req.ssid, t->ssid, t->ssid_len);
				}

				beacon_req.measure_duration = 13;
				beacon_req.mode             = 1;
				beacon_req.random_interval  = 20;

				if (t->ap_channel_report_nr > 0) {
					for (k = 0; k < t->ap_channel_report_nr; k++) {
						beacon_req.ap_channel_report[k].len      = t->ap_channel_report[k].len;
						beacon_req.ap_channel_report[k].op_class = t->ap_channel_report[k].op_class;
						if (beacon_req.ap_channel_report[k].len > 1)
							PLATFORM_MEMCPY(beacon_req.ap_channel_report[k].channel, t->ap_channel_report[k].channel_list, beacon_req.ap_channel_report[k].len);
					}
				}

				beacon_req.request_ie_len = t->elementID_nr;
				if (beacon_req.request_ie_len > 0)
					PLATFORM_MEMCPY(beacon_req.request_ie, t->element_list, t->elementID_nr);

				//find interface STA is associated to
				DMclientMacToAssociatedRadioMac(client_mac, (INT8U *)&radio_mac);
				if (0 == PLATFORM_MEMCMP(radio_mac, empty_mac, MACADDRLEN))
				{
					reason_code = REASON_CODE_STA_NOT_ASSOC;
					PLATFORM_PRINTF_WARNING("Cannot find radio mac associated by STATION %02x%02x%02x%02x%02x%02x\n",
					                        client_mac[0], client_mac[1], client_mac[2], client_mac[3], client_mac[4], client_mac[5]);
				}

				interface_name = DMmacToInterfaceName(radio_mac);

				if (interface_name)
					PLATFORM_ISSUE_BEACON_MEASUREMENT(interface_name, client_mac, &beacon_req);
				else
					PLATFORM_PRINTF_WARNING("Interface_name is NULL, not calling ioctl\n");
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		p = DMmacToAlMac(src_addr);

		if (NULL == p) {
			// The standard says we should always send to the AL MAC
			// address, however, in these cases, instead of just dropping
			// the packet, sending the response to the 'src' address from
			// the METRICS QUERY seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the METRICS QUERY (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = p;
		}

		if (0 != PLATFORM_MEMCMP(DMalMacGet(), dst_mac, 6)) /* different: update */
			_updateControllerStatus(dst_mac, receiving_interface_name);

		// send ACK back, insert error code TLV if necessary
		if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, reason_code, NULL)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_BEACON_METRICS_QUERY MID: (%x) is failed\n", c->message_id);
		}

		if (NULL != p) {
			PLATFORM_FREE(p);
		}

		break;
	}
	case CMDU_TYPE_BEACON_METRICS_RESPONSE: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_BEACON_METRICS_RESPONSE (%s) MID %04x\n", receiving_interface_name, c->message_id);

		INT8U *p;
		int    i = 0, k;

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("Malformed structure.");
			break;
		}

		api_send_tlvs_response(MAP_BEACON_METRIC_RESPONSE_ELEMENT, src_addr, c->list_of_TLVs);

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_BEACON_METRICS_RESPONSE: {
				struct BeaconMetricsResponseTLV *t = (struct BeaconMetricsResponseTLV *)p;
				//Process beacon metric response
				PLATFORM_PRINTF_DEBUG("Received Beacon metrics response\n");
				PLATFORM_PRINTF_DEBUG("MAC: %02x%02x%02x%02x%02x%02x\n", t->mac_address[0], t->mac_address[1], t->mac_address[2],
				                t->mac_address[3], t->mac_address[4], t->mac_address[5]);
				PLATFORM_PRINTF_DEBUG("Measurement Report nr: %d\n", t->measurement_report_nr);
				for (k = 0; k < t->measurement_report_nr; k++) {
					PLATFORM_PRINTF_DEBUG("Element ID: %d\n", t->measurement_reports[k].elementId);
					PLATFORM_PRINTF_DEBUG("Len: %d\n", t->measurement_reports[k].len);
					PLATFORM_PRINTF_DEBUG("Measurement Token: %d\n", t->measurement_reports[k].measurementToken);
					PLATFORM_PRINTF_DEBUG("Measurement Report Mode: %d\n", t->measurement_reports[k].measurementReportMode);
					PLATFORM_PRINTF_DEBUG("Measurement Type: %d\n", t->measurement_reports[k].measurementType);
				}
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}
		break;
	}
	case CMDU_TYPE_COMBINED_INFRASTRUCTURE_METRICS: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_COMBINED_INFRASTRUCTURE_METRICS (%s) MID %04x\n", receiving_interface_name, c->message_id);

		INT8U *p, *dst_mac;
		int    i = 0;

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_AP_METRICS: {
				PLATFORM_PRINTF_DEBUG("Received TLV_TYPE_AP_METRICS\n");
				// struct APMetricsTLV *t = (struct APMetricsTLV *)p;
				break;
			}
			case TLV_TYPE_TRANSMITTER_LINK_METRIC: {
				PLATFORM_PRINTF_DEBUG("Received TLV_TYPE_TRANSMITTER_LINK_METRIC\n");
				// struct transmitterLinkMetricTLV *t = (struct transmitterLinkMetricTLV *)p;
				break;
			}
			case TLV_TYPE_RECEIVER_LINK_METRIC: {
				PLATFORM_PRINTF_DEBUG("Received TLV_TYPE_RECEIVER_LINK_METRIC\n");
				// struct receiverLinkMetricTLV *t = (struct receiverLinkMetricTLV *)p;
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		p = DMmacToAlMac(src_addr);

		if (NULL == p) {
			// The standard says we should always send to the AL MAC
			// address, however, in these cases, instead of just dropping
			// the packet, sending the response to the 'src' address from
			// the METRICS QUERY seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the METRICS QUERY (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = p;
		}

		// send ACK back without error code TLV
		if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_COMBINED_INFRASTRUCTURE_METRICS MID: (%x) is failed\n", c->message_id);
		}

		if (NULL != p) {
			PLATFORM_FREE(p);
		}
		break;
	}
	case CMDU_TYPE_OPERATING_CHANNEL_REPORT: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_MULTI_AP_OPERATING_CHANNEL_REPORT (%s) MID: %04x\n", receiving_interface_name, c->message_id);

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Malformed structure.");
			break;
		}

		api_send_tlvs_response(MAP_OPERATING_CHANNEL_REPORT_ELEMENT, src_addr, c->list_of_TLVs);

		INT8U *p;
		int    i = 0;
		INT8U *dst_mac;
		INT8U *al_addr = DMmacToAlMac(src_addr);
		if (NULL == al_addr) {
			dst_mac = src_addr;
		} else {
			dst_mac = al_addr;
		}
		// send ACK back without error code TLV
		if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_OPERATING_CHANNEL_REPORT MID: (%x) is failed\n", c->message_id);
		}
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_OPERATING_CHANNEL_REPORT: {
				struct OperatingChannelReportTLV *t = (struct OperatingChannelReportTLV *)p;
				PLATFORM_PRINTF_DEBUG("[MULTI_AP] receive Operating Channel Report TLV with radio unique id: %02x:%02x:%02x:%02x:%02x:%02x\n",
				                      t->radio_unique_id[0],
				                      t->radio_unique_id[1],
				                      t->radio_unique_id[2],
				                      t->radio_unique_id[3],
				                      t->radio_unique_id[4],
				                      t->radio_unique_id[5]);
				if (t->cur_op_class_nr)
					PLATFORM_PRINTF_DEBUG("Primary Op class %d, Channel: %d\n", t->operating_channels[0].op_class, t->operating_channels[0].cur_channel);
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		if (NULL != al_addr) {
			PLATFORM_FREE(al_addr);
		}
		break;
	}
	case CMDU_TYPE_CLIENT_STEERING_REQUEST: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_MULTI_AP_CLIENT_STEERING_REQUEST (%s) MID: %04x\n", receiving_interface_name, c->message_id);

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Malformed structure.");
			break;
		}

		INT8U *p;
		int    i = 0;
		INT8U *dst_mac;
		INT8U *al_addr      = DMmacToAlMac(src_addr);
		INT8U  request_mode = 0; // 1 - mandate ; 2 - opportunity
		INT8U  mode         = 0; // bit 7 -- request mode, bit 6 -- disassoc imminent bit, bit 5 -- abridged bit
		INT16U disassoc_timer;
		char   ifname[16];
		INT8U  sta_mac[6];
		INT8U  target_bssid[6];
		INT8U  target_opclass;
		INT8U  target_channel;
		INT16U new_mid;
		new_mid = getNextMid();

		if (NULL == al_addr) {
			dst_mac = src_addr;
		} else {
			dst_mac = al_addr;
		}

		if (0 != PLATFORM_MEMCMP(DMalMacGet(), dst_mac, 6)) /* different: update */
			_updateControllerStatus(dst_mac, receiving_interface_name);

		// send ACK back without error code TLV
		if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_CLIENT_STEERING_REQUEST MID: (%x) is failed\n", c->message_id);
		}
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_STEERING_REQUEST: {
				struct SteeringRequestTLV *t = (struct SteeringRequestTLV *)p;

				PLATFORM_PRINTF_DETAIL("[MULTI_AP] receive Steering Request TLV with bssid: "
					"%02x:%02x:%02x:%02x:%02x:%02x\n",
					t->bssid[0], t->bssid[1], t->bssid[2],
					t->bssid[3], t->bssid[4], t->bssid[5]);

				if (DMmacToInterfaceName(t->bssid) != NULL) {
					strlcpy(ifname, DMmacToInterfaceName(t->bssid), sizeof(ifname));
					mode = t->mode;
					if ((t->mode >> 7) & 0x01) {
						request_mode   = 1; // mandate
						disassoc_timer = t->disassociation_timer;
						PLATFORM_PRINTF_DETAIL("[MULTI_AP] The number of steering clients: %d\n", t->sta_nr);
						PLATFORM_PRINTF_DETAIL("[MULTI_AP] The number of target BSSs: %d\n", t->target_bss_nr);
						int k;
						if (t->sta_nr == 0) {
							// Todo
							PLATFORM_PRINTF_INFO("[MULTI_AP] Steer all associated STAs to the target BSSID\n");
						} else {
							if (t->target_bss_nr == 1) { // The same target BSSID for all specified STAs
								PLATFORM_MEMCPY(target_bssid, t->target_bss[0].bssid, 6);
								target_opclass = t->target_bss[0].opclass;
								target_channel = t->target_bss[0].channel;
								for (k = 0; k < t->sta_nr; k++) {
									PLATFORM_MEMCPY(sta_mac, t->sta_mac_address[k], 6);
									if (1 == DMcheckBTMSteeringDisallow(t->sta_mac_address[k])) {
										// the sta is in the BTM disallowed list
										PLATFORM_SEND_DISASSOC(ifname, sta_mac);
									} else {
										PLATFORM_BTM_REQ(ifname, sta_mac, mode, target_bssid, target_opclass, target_channel, disassoc_timer, 0);
									}
								}
							} else if (t->sta_nr == t->target_bss_nr) {
								for (k = 0; k < t->sta_nr; k++) {
									PLATFORM_MEMCPY(sta_mac, t->sta_mac_address[k], 6);
									if (1 == DMcheckBTMSteeringDisallow(t->sta_mac_address[k])) {
										// the sta is in the BTM disallowed list
										PLATFORM_SEND_DISASSOC(ifname, sta_mac);
									} else {
										PLATFORM_MEMCPY(target_bssid, t->target_bss[k].bssid, 6);
										target_opclass = t->target_bss[k].opclass;
										target_channel = t->target_bss[k].channel;
										PLATFORM_BTM_REQ(ifname, sta_mac, mode, target_bssid, target_opclass, target_channel, disassoc_timer, 0);
									}
								}
							} else {
								PLATFORM_PRINTF_ERROR("[MULTI_AP] The number of steering STAs doesn't match with the number of target BSSIDs\n");
							}
						}
					} else {
						request_mode = 2; // opportunity
						sendMultiAPSteeringCompletedPacket(receiving_interface_name, new_mid, dst_mac);
						api_send_single_tlv_response(MAP_CLIENT_STEERING_REQUEST_OPPORTUNITY_ELEMENT, src_addr, p);
					}
				} else {
					PLATFORM_PRINTF_ERROR("[MULTI_AP] receive Steering Request TLV with bssid: "
						"%02x:%02x:%02x:%02x:%02x:%02x, can't found corresponding local interface!\n",
						t->bssid[0], t->bssid[1], t->bssid[2],
						t->bssid[3], t->bssid[4], t->bssid[5]);
				}
				break;
			}
			//Profile 2 CLIENT STEERING REQUEST
			case TLV_TYPE_PROFILE_2_STEERING_REQUEST: {
				PLATFORM_PRINTF_DEBUG("Received Profile2SteeringRequest TLV\n");
				INT8U  reason_code;
				struct Profile2SteeringRequestTLV *t = (struct Profile2SteeringRequestTLV *)p;
				PLATFORM_PRINTF_DEBUG("BSSID: %02x%02x%02x%02x%02x%02x\n",
					t->bssid[0], t->bssid[1], t->bssid[2], t->bssid[3], t->bssid[4], t->bssid[5]);

				if (DMmacToInterfaceName(t->bssid) != NULL) {
					strlcpy(ifname, DMmacToInterfaceName(t->bssid), sizeof(ifname));
					mode = t->steer_parameters;
					if ((t->steer_parameters >> 7) & 0x01) {
						request_mode   = 1; // mandate
						disassoc_timer = t->btm_disassociation_timer;
						PLATFORM_PRINTF_DETAIL("[MULTI_AP] The number of steering clients: %d\n", t->sta_list_count);
						PLATFORM_PRINTF_DETAIL("[MULTI_AP] The number of target BSSs: %d\n", t->target_bssid_list_count);
						int k;
						if (t->sta_list_count == 0) {
							// Todo
							PLATFORM_PRINTF_INFO("[MULTI_AP] Steer all associated STAs to the target BSSID\n");
							;
						} else {
							if (t->target_bssid_list_count == 1) { // The same target BSSID for all specified STAs
								PLATFORM_MEMCPY(target_bssid, t->target_bssid_list[0].target_bssid, 6);
								target_opclass = t->target_bssid_list[0].target_bss_operating_class;
								target_channel = t->target_bssid_list[0].target_bss_channel_number;
								reason_code    = t->target_bssid_list[0].reason_code;
								for (k = 0; k < t->sta_list_count; k++) {
									PLATFORM_MEMCPY(sta_mac, t->sta_list[k], 6);
									if (1 == DMcheckBTMSteeringDisallow(t->sta_list[k])) {
										// the sta is in the BTM disallowed list
										PLATFORM_SEND_DISASSOC(ifname, sta_mac);
									} else {
										PLATFORM_BTM_REQ(ifname, sta_mac, mode, target_bssid, target_opclass, target_channel, disassoc_timer, reason_code);
									}
								}
							} else if (t->sta_list_count == t->target_bssid_list_count) {
								for (k = 0; k < t->sta_list_count; k++) {
									PLATFORM_MEMCPY(sta_mac, t->sta_list[k], 6);
									if (1 == DMcheckBTMSteeringDisallow(t->sta_list[k])) {
										// the sta is in the BTM disallowed list
										PLATFORM_SEND_DISASSOC(ifname, sta_mac);
									} else {
										PLATFORM_MEMCPY(target_bssid, t->target_bssid_list[k].target_bssid, 6);
										target_opclass = t->target_bssid_list[k].target_bss_operating_class;
										target_channel = t->target_bssid_list[k].target_bss_channel_number;
										reason_code    = t->target_bssid_list[0].reason_code;
										PLATFORM_BTM_REQ(ifname, sta_mac, mode, target_bssid, target_opclass, target_channel, disassoc_timer, reason_code);
									}
								}
							} else {
								PLATFORM_PRINTF_ERROR("[MULTI_AP] The number of steering STAs doesn't match with the number of target BSSIDs\n");
							}
						}
					} else {
						request_mode = 2; // opportunity
						sendMultiAPSteeringCompletedPacket(receiving_interface_name, new_mid, dst_mac);
					}
				} else {
					PLATFORM_PRINTF_ERROR("BSSID: %02x%02x%02x%02x%02x%02x, can't found corresponding local interface!\n",
						t->bssid[0], t->bssid[1], t->bssid[2], t->bssid[3], t->bssid[4], t->bssid[5]);
				}
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}
		if (request_mode == 1) { // mandate
			PLATFORM_PRINTF_DETAIL("[MULTI_AP] Steering Mandate\n");
		} else if (request_mode == 2) { // opportunity
			PLATFORM_PRINTF_DETAIL("[MULTI_AP] Steering Opportunity\n");
		} else {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Neither Steering Mandate nor Steering Opportunity\n");
		}

		if (NULL != al_addr) {
			PLATFORM_FREE(al_addr);
		}
		break;
	}
	case CMDU_TYPE_CLIENT_STEERING_BTM_REPORT: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_MULTI_AP_CLIENT_STEERING_BTM_REPORT (%s) MID: %04x\n", receiving_interface_name, c->message_id);

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Malformed structure.");
			break;
		}

		api_send_tlvs_response(MAP_CLIENT_STEERING_BTM_REPORT_ELEMENT, src_addr, c->list_of_TLVs);

		INT8U *p;
		int    i = 0;
		INT8U *dst_mac;
		INT8U *al_addr = DMmacToAlMac(src_addr);

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_STEERING_BTM_REPORT: {
				struct SteeringBTMReportTLV *t = (struct SteeringBTMReportTLV *)p;

				PLATFORM_PRINTF_DETAIL("[MULTI_AP] receive Steering BTM Report TLV with bssid unique id: %02x:%02x:%02x:%02x:%02x:%02x\n",
				                       t->bssid[0],
				                       t->bssid[1],
				                       t->bssid[2],
				                       t->bssid[3],
				                       t->bssid[4],
				                       t->bssid[5]);
				PLATFORM_PRINTF_DETAIL("[MULTI_AP] Status code: %d\n", t->status_code);
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		if (NULL == al_addr) {
			dst_mac = src_addr;
		} else {
			dst_mac = al_addr;
		}
		// send ACK back without error code TLV
		if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_CLIENT_STEERING_BTM_REPORT MID: (%x) is failed\n", c->message_id);
		}

		if (NULL != al_addr) {
			PLATFORM_FREE(al_addr);
		}
		break;
	}
	case CMDU_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_MULTI_AP_CLIENT_ASSOCIATION_CONTROL_REQUEST (%s) MID: %04x\n", receiving_interface_name, c->message_id);

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Malformed structure.");
			break;
		}

		INT8U *p;
		int    i = 0;
		INT8U *dst_mac;
		INT8U *al_addr = DMmacToAlMac(src_addr);
		char   ifname[16];
		INT8U  sta_mac[6];

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST: {
				struct ClientAssociationControlRequestTLV *t = (struct ClientAssociationControlRequestTLV *)p;
				PLATFORM_PRINTF_INFO("[MULTI_AP] receive Client Association Control Request TLV with bssid unique id: "
					"%02x:%02x:%02x:%02x:%02x:%02x\n",
					t->bssid[0], t->bssid[1], t->bssid[2],
					t->bssid[3], t->bssid[4], t->bssid[5]);

				if (DMmacToInterfaceName(t->bssid) != NULL) {
					strlcpy(ifname, DMmacToInterfaceName(t->bssid), sizeof(ifname));
					if (t->sta_nr != 0) {
						int k;
						for (k = 0; k < t->sta_nr; k++) {
							PLATFORM_MEMCPY(sta_mac, t->sta_mac_address[k], 6);
							// association_control == 0x00 : block STA
							// association_control == 0x01 : unblock STA
							PLATFORM_ASSOC_CONTROL(ifname, sta_mac, t->validity_period, t->association_control);

							/* controller's block STA request: delete the station */
							if ((t->association_control == 0)
								&& (0 == PLATFORM_MEMCMP(DMcontrollerMacGet(), src_addr, 6)))
								PLATFORM_DELETE_STA(ifname, sta_mac);
						}
					}
				} else {
					PLATFORM_PRINTF_ERROR("[MULTI_AP] receive Client Association Control Request TLV with bssid unique id: "
						"%02x:%02x:%02x:%02x:%02x:%02x, can't found corresponding local interface!\n",
						t->bssid[0], t->bssid[1], t->bssid[2],
						t->bssid[3], t->bssid[4], t->bssid[5]);
				}
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		if (NULL == al_addr) {
			dst_mac = src_addr;
		} else {
			dst_mac = al_addr;
		}
		// send ACK back without error code TLV
		if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST MID: (%x) is failed\n", c->message_id);
		}

		if (NULL != al_addr) {
			PLATFORM_FREE(al_addr);
		}
		break;
	}
	case CMDU_TYPE_STEERING_COMPLETED: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_MULTI_AP_STEERING_COMPLETED (%s) MID: %04x\n", receiving_interface_name, c->message_id);

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Malformed structure.");
			break;
		}

		api_send_tlvs_response(MAP_STEERING_COMPLETE_ELEMENT, src_addr, c->list_of_TLVs);

		INT8U *dst_mac;
		INT8U *al_addr = DMmacToAlMac(src_addr);

		if (NULL == al_addr) {
			dst_mac = src_addr;
		} else {
			dst_mac = al_addr;
		}
		// send ACK back without error code TLV
		if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_STEERING_COMPLETED MID: (%x) is failed\n", c->message_id);
		}

		if (NULL != al_addr) {
			PLATFORM_FREE(al_addr);
		}
		break;
	}
	case CMDU_TYPE_BACKHAUL_STEERING_REQUEST: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_BACKHAUL_STEERING_REQUEST (%s) MID %04x\n", receiving_interface_name, c->message_id);

		INT8U *p, *dst_mac, *al_addr;
		int    i                   = 0;
		INT8U  target_bss_mac[6]   = { 0 };
		INT8U  backhaul_bss_mac[6] = { 0 };
		int    ret                 = 0;

		al_addr = DMmacToAlMac(src_addr);

		if (NULL == al_addr) {
			// The standard says we should always send to the AL MAC
			// address, however, in these cases, instead of just dropping
			// the packet, sending the response to the 'src' address from
			// the METRICS QUERY seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the METRICS QUERY (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = al_addr;
		}

		// send ACK back without error code TLV
		if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_BACKHAUL_STEERING_REQUEST MID: (%x) is failed\n", c->message_id);
		}

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_BACKHAUL_STEERING_REQUEST: {
				struct BackhaulSteeringRequestTLV *t = (struct BackhaulSteeringRequestTLV *)p;
				//Process backhaul steering request
				PLATFORM_PRINTF_DEBUG("Received Backhaul steering request\n");
				PLATFORM_PRINTF_DEBUG("Backhaul BSS - %02x%02x%02x%02x%02x%02x\n",
				                t->backhaul_mac[0], t->backhaul_mac[1], t->backhaul_mac[2],
				                t->backhaul_mac[3], t->backhaul_mac[4], t->backhaul_mac[5]);
				PLATFORM_PRINTF_DEBUG("Backhaul Target - %02x%02x%02x%02x%02x%02x\n",
				                t->target_bssid[0], t->target_bssid[1], t->target_bssid[2],
				                t->target_bssid[3], t->target_bssid[4], t->target_bssid[5]);
				PLATFORM_PRINTF_DEBUG("Op Class - %d\n", t->op_class);
				PLATFORM_PRINTF_DEBUG("Channel number - %d\n", t->channel_number);

				PLATFORM_MEMCPY(backhaul_bss_mac, t->backhaul_mac, MACADDRLEN);
				PLATFORM_MEMCPY(target_bss_mac, t->target_bssid, MACADDRLEN);

				ret = PLATFORM_BACKHAUL_STEER_IOCTL(receiving_interface_name, backhaul_bss_mac, target_bss_mac, t->op_class, t->channel_number);

				if (ret == 2) {
					DMsetBackhaulSteeringTarget(target_bss_mac);
					DMsetEventFlag(FLAG_PREFER_BSSID_SET);
				}

				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		_updateControllerStatus(dst_mac, receiving_interface_name);

		if (NULL != al_addr) {
			PLATFORM_FREE(al_addr);
		}
		break;
	}
	case CMDU_TYPE_BACKHAUL_STEERING_RESPONSE: {

		api_send_tlvs_response(MAP_BACKHAUL_STEERING_RESPONSE_ELEMENT, src_addr, c->list_of_TLVs);

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_BACKHAUL_STEERING_RESPONSE (%s) MID %04x\n", receiving_interface_name, c->message_id);

		INT8U *p;
		int    i = 0;
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_BACKHAUL_STEERING_RESPONSE: {
				struct BackhaulSteeringResponseTLV *t = (struct BackhaulSteeringResponseTLV *)p;
				//Process backhaul steering response
				PLATFORM_PRINTF_DEBUG("Received Backhaul steering response\n");
				PLATFORM_PRINTF_DEBUG("Backhaul Steering response - %d\n", t->result_code);
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}
		break;
	}
	default:{
#ifdef RTK_MULTI_AP_R2
		if(MULTI_AP_PROFILE_2 <= DMgetMultiAPProfile()) {
			ret = processMultiAPR2Cmdu(c, receiving_interface_name, src_addr);
		} else
#endif
		{
			PLATFORM_PRINTF_WARNING("Unknown CMDU type %x received with mid %x on R1 device, drop packet.\n", c->message_type, c->message_id);
		}
		break;
	}
	}
	return ret;
}

#ifdef RTK_MULTI_AP_R2
INT8U processMultiAPR2Cmdu(struct CMDU *c, char *receiving_interface_name, INT8U *src_addr)
{
	INT8U ret = PROCESS_CMDU_OK;
	////////////////////////////////////////////////////////////////////////////////
	/// Profile 2 CMDUs
	////////////////////////////////////////////////////////////////////////////////
	switch (c->message_type) {
	case CMDU_TYPE_CAC_TERMINATION: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_CAC_TERMINATION (%s) MID %04x\n", receiving_interface_name, c->message_id);

		INT8U *p, *dst_mac;
		int    i = 0;

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_CAC_TERMINATION: {
				struct CACTerminationTLV *t = (struct CACTerminationTLV *)p;

				INT8U k;
				PLATFORM_PRINTF_DEBUG("Received CAC Termination TLV\n");

				PLATFORM_PRINTF_DEBUG("Radio nr : %d\n", t->radio_nr);

				for (k = 0; k < t->radio_nr; k++) {
					PLATFORM_PRINTF_DEBUG("Radio id : %02x:%02x:%02x:%02x:%02x:%02x\n", t->radios[k].radio_unique_identifier[0], t->radios[k].radio_unique_identifier[1], t->radios[k].radio_unique_identifier[2], t->radios[k].radio_unique_identifier[3], t->radios[k].radio_unique_identifier[4], t->radios[k].radio_unique_identifier[5]);
					PLATFORM_PRINTF_DEBUG("Op class : %d\n", t->radios[k].op_class);
					PLATFORM_PRINTF_DEBUG("Channel : %d\n", t->radios[k].channel);
				}

				//forge stream and send down to wifi driver

				INT8U  channel_nr_5gl = 0;
				INT8U  channel_nr_5gh = 0;
				INT8U *channels_5gl   = NULL;
				INT8U *channels_5gh   = NULL;

				for (k = 0; k < t->radio_nr; k++) {
					char *radio_name = DMmacToInterfaceName(t->radios[0].radio_unique_identifier);
					if (0 == PLATFORM_STRCMP(radio_name, map_radio_name_5gh)) {
						channel_nr_5gh++;
						channels_5gh = (INT8U *)PLATFORM_REALLOC(channels_5gh, sizeof(INT8U) * channel_nr_5gh * 2);
						PLATFORM_MEMCPY(channels_5gh + 2 * (channel_nr_5gh - 1), &t->radios[k].channel, 1);
						PLATFORM_MEMCPY(channels_5gh + 2 * (channel_nr_5gh - 1) + 1, &t->radios[k].op_class, 1);
					} else if (0 == PLATFORM_STRCMP(radio_name, map_radio_name_5gl)) {
						channel_nr_5gl++;
						channels_5gl = (INT8U *)PLATFORM_REALLOC(channels_5gl, sizeof(INT8U) * channel_nr_5gl * 2);
						PLATFORM_MEMCPY(channels_5gl + 2 * (channel_nr_5gl - 1), &t->radios[k].channel, 1);
						PLATFORM_MEMCPY(channels_5gl + 2 * (channel_nr_5gl - 1) + 1, &t->radios[k].op_class, 1);
					} else {
						continue;
					}
				}

				if (channels_5gh) {
					PLATFORM_CAC_IOCTL(map_radio_name_5gh, channel_nr_5gh, channels_5gh, 1);
					PLATFORM_FREE(channels_5gh);
				}

				if (channel_nr_5gl) {
					PLATFORM_CAC_IOCTL(map_radio_name_5gl, channel_nr_5gl, channels_5gl, 1);
					PLATFORM_FREE(channels_5gl);
				}

				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		p        = DMmacToAlMac(src_addr);

		if (NULL == p) {
			// The standard says we should always send to the AL MAC
			// address, however, in these cases, instead of just dropping
			// the packet, sending the response to the 'src' address from
			// the METRICS QUERY seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the CAC Request (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = p;
		}

		// send ACK back without error code TLV
		if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_CAC_TERMINATION MID: (%x) is failed\n", c->message_id);
		}

		if (NULL != p) {
			PLATFORM_FREE(p);
		}

		break;
	}

	case CMDU_TYPE_CAC_REQUEST: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_CAC_REQUEST (%s) MID %04x\n", receiving_interface_name, c->message_id);

		INT8U *p, *dst_mac;
		int    i                              = 0;
		int    k                              = 0;
		struct CACRequestTLV *cac_request_tlv = NULL;

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_CAC_REQUEST: {
				cac_request_tlv = (struct CACRequestTLV *)p;

				PLATFORM_PRINTF_DEBUG("Received CAC Request TLV\n");

				PLATFORM_PRINTF_DEBUG("Radio nr : %d\n", cac_request_tlv->radio_nr);

				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		p = DMmacToAlMac(src_addr);

		if (NULL == p) {
			// The standard says we should always send to the AL MAC
			// address, however, in these cases, instead of just dropping
			// the packet, sending the response to the 'src' address from
			// the METRICS QUERY seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the CAC Request (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = p;
		}

		if (cac_request_tlv) {
			// send ACK back without error code TLV
			if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
				PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_CAC_REQUEST MID: (%x) is failed\n", c->message_id);
			}

			//forge stream and send down to wifi driver
			INT8U  channel_nr_5gl = 0;
			INT8U  channel_nr_5gh = 0;
			INT8U *channels_5gl   = NULL;
			INT8U *channels_5gh   = NULL;

			for (k = 0; k < cac_request_tlv->radio_nr; k++) {
				char *radio_name = DMmacToInterfaceName(cac_request_tlv->radios[0].radio_unique_identifier);
				if (0 == PLATFORM_STRCMP(radio_name, map_radio_name_5gh)) {
					channel_nr_5gh++;
					channels_5gh = (INT8U *)PLATFORM_REALLOC(channels_5gh, sizeof(INT8U) * channel_nr_5gh * 3);
					PLATFORM_MEMCPY(channels_5gh + 3 * (channel_nr_5gh - 1), &cac_request_tlv->radios[k].channel, 1);
					PLATFORM_MEMCPY(channels_5gh + 3 * (channel_nr_5gh - 1) + 1, &cac_request_tlv->radios[k].flags, 1);
					PLATFORM_MEMCPY(channels_5gh + 3 * (channel_nr_5gh - 1) + 2, &cac_request_tlv->radios[k].op_class, 1);
				} else if (0 == PLATFORM_STRCMP(radio_name, map_radio_name_5gl)) {
					channel_nr_5gl++;
					channels_5gl = (INT8U *)PLATFORM_REALLOC(channels_5gl, sizeof(INT8U) * channel_nr_5gl * 3);
					PLATFORM_MEMCPY(channels_5gl + 3 * (channel_nr_5gl - 1), &cac_request_tlv->radios[k].channel, 1);
					PLATFORM_MEMCPY(channels_5gl + 3 * (channel_nr_5gl - 1) + 1, &cac_request_tlv->radios[k].flags, 1);
					PLATFORM_MEMCPY(channels_5gl + 3 * (channel_nr_5gl - 1) + 2, &cac_request_tlv->radios[k].op_class, 1);
				} else {
					continue;
				}
				DMupdateCACRecord(cac_request_tlv->radios[k].radio_unique_identifier, cac_request_tlv->radios[k].op_class, cac_request_tlv->radios[k].channel, CAC_ON_GOING);
			}

			DMdumpCACStatus();

			if (channels_5gh) {
				PLATFORM_CAC_IOCTL(map_radio_name_5gh, channel_nr_5gh, channels_5gh, 0);
				PLATFORM_FREE(channels_5gh);
			}

			if (channel_nr_5gl) {
				PLATFORM_CAC_IOCTL(map_radio_name_5gl, channel_nr_5gl, channels_5gl, 0);
				PLATFORM_FREE(channels_5gl);
			}
		}

		if (NULL != p) {
			PLATFORM_FREE(p);
		}

		break;
	}

	case CMDU_TYPE_CHANNEL_SCAN_REQUEST: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_CHANNEL_SCAN_REQUEST (%s) MID %04x\n", receiving_interface_name, c->message_id);

		INT8U *p, *dst_mac;
		int    i = 0;

		INT8U scan_mode = 0;

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_CHANNEL_SCAN_REQUEST: {
				struct ChannelScanRequestTLV *t = (struct ChannelScanRequestTLV *)p;

				INT8U k, l, m;
				PLATFORM_PRINTF_DEBUG("Received Channel Scan Request TLV\n");

				PLATFORM_PRINTF_DEBUG("Flags : %d\n", t->flags);
				PLATFORM_PRINTF_DEBUG("Target radio nr : %d\n", t->target_radio_nr);

				for (k = 0; k < t->target_radio_nr; k++) {
					PLATFORM_PRINTF_DEBUG("Radio id : %02x:%02x:%02x:%02x:%02x:%02x\n", t->target_radios[k].radio_unique_identifier[0], t->target_radios[k].radio_unique_identifier[1], t->target_radios[k].radio_unique_identifier[2], t->target_radios[k].radio_unique_identifier[3], t->target_radios[k].radio_unique_identifier[4], t->target_radios[k].radio_unique_identifier[5]);
					PLATFORM_PRINTF_DEBUG("Target op class nr : %d\n", t->target_radios[k].target_op_class_nr);
					for (l = 0; l < t->target_radios[k].target_op_class_nr; l++) {
						PLATFORM_PRINTF_DEBUG("Op class : %d\n", t->target_radios[k].target_op_classes[l].op_class);
						PLATFORM_PRINTF_DEBUG("Target channel nr : %d\n", t->target_radios[k].target_op_classes[l].target_channel_nr);
						for (m = 0; m < t->target_radios[k].target_op_classes[l].target_channel_nr; m++) {
							PLATFORM_PRINTF_DEBUG("Channel : %d\n", t->target_radios[k].target_op_classes[l].target_channel_list[m]);
						}
					}
				}

				//buffer this channel scan request in database for the use of re-constructing the channel scan report
				if (DMbufferChannelScanRequest(t)) {
					PLATFORM_PRINTF_ERROR("Buffer Channel Scan Request Failed...\n");
					continue;
				}

				if (PERFORM_FRESH_SCAN_FALSE == (PERFORM_FRESH_SCAN_MASK & t->flags)) {
					scan_mode = 2;
				} else {
					scan_mode = 1;
				}

				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		p = DMmacToAlMac(src_addr);

		if (NULL == p) {
			// The standard says we should always send to the AL MAC
			// address, however, in these cases, instead of just dropping
			// the packet, sending the response to the 'src' address from
			// the METRICS QUERY seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the Channel Scan Request (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = p;
		}

		// send ACK back without error code TLV
		if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_CHANNEL_SCAN_REPORT MID: (%x) is failed\n", c->message_id);
		}

		if(1 == scan_mode) {
			PLATFORM_PRINTF_DEBUG("Perform fresh scan bit is set. Performing a fresh scan...\n");
			DMsetEventFlag(FLAG_CHANNEL_SCAN);
		} else if (2 == scan_mode) {
			//no need fresh scan. send report with results in database
			PLATFORM_PRINTF_DEBUG("Perform fresh scan bit not set. Replying with stored channel scan results...\n");
			if (0 == sendChannelScanReportPacket(receiving_interface_name, getNextMid(), dst_mac, 0)) {
				PLATFORM_PRINTF_ERROR("[Multi-AP] Sending TLV_TYPE_CHANNEL_SCAN_REQUEST MID: (%x) failed\n", c->message_id);
			}
		} else {
			PLATFORM_PRINTF_WARNING("Invalid channel scan request, ignored.\n");
		}

		if (NULL != p) {
			PLATFORM_FREE(p);
		}

		break;
	}

	case CMDU_TYPE_CHANNEL_SCAN_REPORT: {

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_CHANNEL_SCAN_REPORT (%s) MID %04x\n", receiving_interface_name, c->message_id);

		if (NULL == c->list_of_TLVs) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Malformed structure.");
			break;
		}
		api_send_tlvs_response(MAP_CHANNEL_SCAN_REPORT_ELEMENT, src_addr, c->list_of_TLVs);

		INT8U *p, *dst_mac;

		p        = DMmacToAlMac(src_addr);

		if (NULL == p) {
			// The standard says we should always send to the AL MAC
			// address, however, in these cases, instead of just dropping
			// the packet, sending the response to the 'src' address from
			// the METRICS QUERY seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the Channel Scan Report (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = p;
		}

		//if asked for stored channel scan results.
		if (DMgetRequestStoredResultsFlag() == 1) {
			if (waiting_msgs_ack(c->message_id) == 1 && PLATFORM_STRCMP("lo", receiving_interface_name)) {
				PLATFORM_PRINTF_WARNING("[Multi-AP] This Channel Scan Request [%04x] is timeout.\n", c->message_id);
			}
			DMsetRequestStoredResultsFlag(0);
		} else {
			// send ACK back without error code TLV
			if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
				PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_CHANNEL_SCAN_REPORT MID: (%x) is failed\n", c->message_id);
			}
		}

		if (NULL != p) {
			PLATFORM_FREE(p);
		}

		break;
	}
	case CMDU_TYPE_ASSOCIATION_STATUS_NOTIFICATION: {

		INT8U *p, *dst_mac;
		int    i = 0;

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_ASSOCIATION_STATUS_NOTIFICATION (%s) MID %04x\n", receiving_interface_name, c->message_id);

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION: {
				int                                      k;
				struct AssociationStatusNotificationTLV *t = (struct AssociationStatusNotificationTLV *)p;
				PLATFORM_PRINTF_DEBUG("Received AssociationStatusNotification TLV\n");
				PLATFORM_PRINTF_DEBUG("BSSID nr: %d\n", t->bssid_nr);
				for (k = 0; k < t->bssid_nr; k++) {
					PLATFORM_PRINTF_DEBUG("BSSID: %02x%02x%02x%02x%02x%02x\n", t->bss_info[k].bssid[0], t->bss_info[k].bssid[1], t->bss_info[k].bssid[2], t->bss_info[k].bssid[3], t->bss_info[k].bssid[4], t->bss_info[k].bssid[5]);
					PLATFORM_PRINTF_DEBUG("Association allowance status: %d\n", t->bss_info[k].association_allowance_status);
				}

				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		p        = DMmacToAlMac(src_addr);

		if (NULL == p) {
			// The standard says we should always send to the AL MAC
			// address, however, in these cases, instead of just dropping
			// the packet, sending the response to the 'src' address from
			// the METRICS QUERY seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the association status Message (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = p;
		}

		// send ACK back without error code TLV
		if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_ASSOCIATION_STATUS_NOTIFICATION MID: (%x) is failed\n", c->message_id);
		}

		if (NULL != p) {
			PLATFORM_FREE(p);
		}

		break;
	}
	case CMDU_TYPE_TUNNELED: {

		INT8U *p, *dst_mac;
		int    i = 0;

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_TUNNELED (%s) MID %04x\n", receiving_interface_name, c->message_id);

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_SOURCE_INFO: {
				struct SourceInfoTLV *t = (struct SourceInfoTLV *)p;
				PLATFORM_PRINTF_DEBUG("Received SourceInfo TLV\n");
				PLATFORM_PRINTF_DEBUG("Mac address: %02x%02x%02x%02x%02x%02x\n", t->mac_address[0], t->mac_address[1], t->mac_address[2], t->mac_address[3], t->mac_address[4], t->mac_address[5]);
				break;
			}
			case TLV_TYPE_TUNNELED_MESSAGE_TYPE: {
				struct TunneledMessageTypeTLV *t = (struct TunneledMessageTypeTLV *)p;
				PLATFORM_PRINTF_DEBUG("Received TunneledMessageType TLV\n");
				PLATFORM_PRINTF_DEBUG("Tunnedled protocol payload type: %d\n", t->tunneled_protocol_type_payload);
				break;
			}
			case TLV_TYPE_TUNNELED: {
				struct TunneledTLV *t = (struct TunneledTLV *)p;
				PLATFORM_PRINTF_DEBUG("Received Tunneled TLV\n");
				PLATFORM_PRINTF_DEBUG("Payload length: %d\n", t->tlv_length);
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		p        = DMmacToAlMac(src_addr);

		if (NULL == p) {
			// The standard says we should always send to the AL MAC
			// address, however, in these cases, instead of just dropping
			// the packet, sending the response to the 'src' address from
			// the METRICS QUERY seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the Tunneled Message (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = p;
		}

		// send ACK back without error code TLV
		if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_TUNNELED MID: (%x) is failed\n", c->message_id);
		}

		if (NULL != p) {
			PLATFORM_FREE(p);
		}

		break;
	}
	case CMDU_TYPE_FAILED_CONNECTION: {
		INT8U *p, *dst_mac;

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_FAILED_CONNECTION (%s) MID %04x\n", receiving_interface_name, c->message_id);

		p        = DMmacToAlMac(src_addr);

		if (NULL == p) {
			// The standard says we should always send to the AL MAC
			// address, however, in these cases, instead of just dropping
			// the packet, sending the response to the 'src' address from
			// the METRICS QUERY seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the Failed Connection Message (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = p;
		}

		// send ACK back without error code TLV
		if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_FAILED_CONNECTION MID: (%x) is failed\n", c->message_id);
		}

		if (NULL != p) {
			PLATFORM_FREE(p);
		}

		break;
	}
	case CMDU_TYPE_CLIENT_DISASSOCIATION_STATS: {
		INT8U *p, *dst_mac;

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_CLIENT_DISASSOCIATION_STATS (%s) MID %04x\n", receiving_interface_name, c->message_id);

		p        = DMmacToAlMac(src_addr);

		if (NULL == p) {
			// The standard says we should always send to the AL MAC
			// address, however, in these cases, instead of just dropping
			// the packet, sending the response to the 'src' address from
			// the METRICS QUERY seems the right thing to do.
			//
			dst_mac = src_addr;
			PLATFORM_PRINTF_WARNING("Unknown destination AL MAC. Using the 'src' MAC from the Client Disassociation Message (%02x:%02x:%02x:%02x:%02x:%02x)\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		} else {
			dst_mac = p;
		}

		// send ACK back without error code TLV
		if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_CLIENT_DISASSOCIATION_STATS MID: (%x) is failed\n", c->message_id);
		}

#ifdef RTK_MULTI_AP_R4
		// EasyMesh Spec v4 5.3.10.2 & Figure 14
		// If controller detects connectivity loss with enrollee,
		// Controller should instruct ALL agents to advertise CCE
		if (DMisController()) {
			triggerCCEAdvertisement(START_CCE_ADVERTISEMENT);
		}
#endif

		if (NULL != p) {
			PLATFORM_FREE(p);
		}
		break;
	}
	default: {
#ifdef RTK_MULTI_AP_R3
		if (MULTI_AP_PROFILE_3 <= DMgetMultiAPProfile()) {
			ret = processMultiAPR3Cmdu(c, receiving_interface_name, src_addr);
		} else
#endif
		{
			PLATFORM_PRINTF_WARNING("Unknown CMDU type %x received with mid %x on R2 device, drop packet.\n", c->message_type, c->message_id);
			break;
		}
	}
	}
	return ret;
}
#endif

#ifdef RTK_MULTI_AP_R3
static INT8U processDPPRxAction(char *interface_name, INT8U *mac, INT8U *dpp_frame, INT16U dpp_frame_len)
{
	struct dpp_authentication *dpp_auth = NULL;

	INT16U msg_len    = 0;
	INT32U rx_msg_len = 0;
	INT8U *msg = NULL, *rx_msg = NULL;
	INT8U  ret = 0;

	dpp_auth = DMgetDPPAuthenticationObject();

	// Append first byte (0x04 Action Category Field)
	msg_len = dpp_frame_len + 1;
	msg     = PLATFORM_MALLOC(msg_len);
	msg[0]  = WLAN_ACTION_PUBLIC;
	PLATFORM_MEMCPY(msg + 1, dpp_frame, dpp_frame_len);

	// Process DPP Action Frame
	if (msg[1] == WLAN_PA_VENDOR_SPECIFIC) {
		enum dpp_public_action_frame_type type = msg[7];
		switch (type) {
		case DPP_PA_AUTHENTICATION_REQ:
			if ((ret = dpp_rx_auth_req(dpp_auth, msg, msg_len)) == 1) {
				ret = dpp_auth_build_resp_ok(dpp_auth, &rx_msg, &rx_msg_len);
			}
			break;
		case DPP_PA_AUTHENTICATION_RESP:
			if ((ret = dpp_rx_auth_resp(dpp_auth, msg, msg_len)) == 1) {
				ret = dpp_auth_build_conf(dpp_auth, DPP_STATUS_OK, &rx_msg, &rx_msg_len);
			}
			break;
		case DPP_PA_AUTHENTICATION_CONF:
			if ((ret = dpp_rx_auth_conf(dpp_auth, msg, msg_len)) == 1) {
				// Build DPP Configuration Request Object
				struct dpp_config_req_obj req_obj;
				PLATFORM_MEMSET(&req_obj, 0, sizeof(struct dpp_config_req_obj));
				strlcpy(req_obj.name, DMgetDeviceName(), sizeof(req_obj.name));
				strlcpy(req_obj.wifi_tech, "map", sizeof(req_obj.wifi_tech));
				req_obj.netRole               = DPP_NETROLE_MAP_AGENT;
				req_obj.b_sta_list_count      = 0;
				// Send DPP Configuration Request
				ret = dpp_build_conf_req(dpp_auth, &req_obj, &rx_msg, &rx_msg_len);
			}
			break;
		case DPP_PA_CONFIGURATION_RESULT:
			if ((ret = dpp_conf_result_rx(dpp_auth, msg, msg_len)) == 1) {
				PLATFORM_PRINTF_INFO("DPP Configuration Completed!\n");
			}
			break;
		case DPP_PA_PEER_DISCOVERY_REQ: {
			// During onboarding only controller should receive DPP Peer Discovery Request.
			struct dpp_connector rx_conn;

			struct dpp_eapol_sm *sm  = NULL;

			PLATFORM_MEMSET(&rx_conn, 0, sizeof(rx_conn));
			DMgetDPPEAPOLStateMachine(&sm);
			// TODO: Always respond to discovery request, but dealy the sending EAPOL M1
			if (sm->state == STARTED)
				break;

			if (NULL == dpp_keystore_get_gtk()) {
				PLATFORM_PRINTF_DEBUG("Missing GTK, ignore this introduction request.\n");
				break;
			}

			if ((ret = dpp_rx_peer_disc_req(dpp_auth->conf, &rx_conn, msg, msg_len)) == 1) {
				struct dpp_config_obj  *conf_obj;
				struct dpp_introduction intro;
				conf_obj = dpp_find_config_obj(dpp_auth, "map", "*", DMisController() ? DPP_NETROLE_MAP_CONTROLLER : DPP_NETROLE_MAP_AGENT);

				// Controller should send DPP Peer Discovery Response if PMK is OK:
				if (conf_obj
				    && (ret = dpp_peer_intro(&conf_obj->conn, &rx_conn, &intro))
				    && (ret = dpp_build_peer_disc_resp(&conf_obj->conn, DPP_STATUS_OK, &rx_msg, &rx_msg_len))) {
					send1905DirectEncapDPPPacket(interface_name, getNextMid(), mac, rx_msg, rx_msg_len);
					PLATFORM_SAFE_FREE((void **)&rx_msg);
				}

				// Controller should also be the EAPOL authenticator and send EAPOL 1/4:
				if (ret) {
					DMsetEventFlag(FLAG_ON_EAPOL);
					struct dpp_gtk      *gtk = dpp_keystore_get_gtk();
					dpp_eapol_sm_init(sm, &intro, gtk, AUTHENTICATOR, DMalMacGet(), mac);
					if (dpp_eapol_sm_start(sm, &rx_msg, &rx_msg_len) == EAPOL_SM_OK_TX) {
						send1905EncapEAPOLMessage(interface_name, getNextMid(), mac, rx_msg, rx_msg_len);
						PLATFORM_SAFE_FREE((void **)&rx_msg);
						PLATFORM_PRINTF_INFO("DPP Peer Discovery completed and started EAPOL 1/4\n");
					}
				}
			}
			dpp_free_connector(&rx_conn);
			break;
		}
		case DPP_PA_PEER_DISCOVERY_RESP: {
			// Agent should receive DPP Peer Discovery Response
			struct dpp_connector rx_conn;

			char *out_group_id = NULL;

			struct dpp_eapol_sm *sm  = NULL;

			PLATFORM_MEMSET(&rx_conn, 0, sizeof(rx_conn));
			DMgetDPPEAPOLStateMachine(&sm);
			// TODO: Always respond to discovery request, but dealy the sending EAPOL M1
			if (sm->state == STARTED)
				break;

			if ((ret = dpp_rx_peer_disc_resp(dpp_auth->conf, &rx_conn, msg, msg_len, &out_group_id)) == 1) {
				if (out_group_id == NULL) {
					PLATFORM_PRINTF_WARNING("No groupID found in Config Resp\n");
					break;
				}
				struct dpp_config_obj *conf_obj = dpp_find_config_obj(dpp_auth, "map", out_group_id, DPP_NETROLE_MAP_AGENT);
				struct dpp_introduction intro;

				// Agent parse received DPP connector:
				ret = dpp_peer_intro(&conf_obj->conn, &rx_conn, &intro);
				PLATFORM_PRINTF_INFO("DPP Peer Discovery completed\n");

				// Agent should start EAPOL as supplicant if PMK is OK:
				if (ret) {
					DMsetEventFlag(FLAG_WAIT_FOR_M1);
					dpp_eapol_sm_init(sm, &intro, NULL, SUPPLICANT, DMalMacGet(), mac);
					dpp_eapol_sm_start(sm, NULL, 0);
					PLATFORM_PRINTF_INFO("EAPOL supplicant state machine started (ret: %d) \n", ret);
				}
			}
			if (out_group_id)
				PLATFORM_FREE(out_group_id);
			break;
		}
		default:
			PLATFORM_PRINTF_WARNING("Unsupported DPP Frame Type %d\n", type);
			break;
		}

	} else if (msg[1] == WLAN_PA_GAS_INITIAL_REQ) {

		// Process GAS / DPP Configuration Request
		//
		// Parse DPP Configuration Request Object
		INT8U dialog_token = msg[2];
		struct dpp_config_req_obj req_obj;
		PLATFORM_MEMSET(&req_obj, 0, sizeof(struct dpp_config_req_obj));
		if ((ret = dpp_conf_req_rx(dpp_auth, &req_obj, msg, msg_len)) != 1) {
			PLATFORM_PRINTF_WARNING("Invalid config req obj\n");
			goto fail;
		}
		if (PLATFORM_STRCMP(req_obj.wifi_tech, "map") != 0) {
			PLATFORM_PRINTF_WARNING("wifi tech is not map\n");
			goto fail;
		}
		if (req_obj.netRole != DPP_NETROLE_MAP_AGENT) {
			PLATFORM_PRINTF_WARNING("netRole is not mapAgent\n");
			goto fail;
		}

		// Build DPP Configuration Response Objects
		struct dpp_config_obj conf_objs[1];
		PLATFORM_MEMSET(conf_objs, 0, sizeof(conf_objs));
		PLATFORM_STRCPY(conf_objs[0].wifi_tech, "map");
		conf_objs[0].dfCounterThreshold = RTK_DEFAULT_DECRYPT_FAILURE_THRESHOLD;
		PLATFORM_STRCPY(conf_objs[0].akm, "dpp");
		char *dummy_ssid = "mywifi";
		PLATFORM_STRCPY(conf_objs[0].ssid, dummy_ssid);
		conf_objs[0].ssid_len = strlen(dummy_ssid);
		if ((ret = dpp_build_signed_connector(dpp_auth->conf,
		                                      &dpp_auth->peer_protocol_key,
		                                      "mapNW", DPP_NETROLE_MAP_AGENT, &(conf_objs[0].conn)))
		    != 1) {
			PLATFORM_PRINTF_WARNING("Cannot generate DPP Signed Connector\n");
			goto fail;
		}

		// Send DPP Configuration
		ret = dpp_build_conf_resp(dpp_auth, conf_objs, 1, &rx_msg, &rx_msg_len, dialog_token);
		dpp_free_conf_obj(&conf_objs[0]);

	} else if (msg[1] == WLAN_PA_GAS_INITIAL_RESP) {

		// Process GAS / DPP Configuration Response
		//
		char *out_group_id = NULL;

		if ((ret = dpp_conf_resp_rx(dpp_auth,
		                            dpp_auth->conf_obj,
		                            &dpp_auth->conf_obj_count, DPP_MAX_CONF_OBJ,
		                            msg, msg_len, &out_group_id))
		    == 1) {
			// Issue Configuration Result
			if ((ret = dpp_build_conf_result(dpp_auth, DPP_STATUS_OK, &rx_msg, &rx_msg_len)) == 1) {
				send1905DirectEncapDPPPacket(interface_name, getNextMid(), mac, rx_msg, rx_msg_len);
				PLATFORM_SAFE_FREE((void **)&rx_msg);
			}

			// Agent should send DPP Peer Discovery Request
			if (out_group_id == NULL) {
				PLATFORM_PRINTF_WARNING("No groupID found in Config Resp\n");
				goto fail;
			}
			struct dpp_config_obj *conf_obj = dpp_find_config_obj(dpp_auth, "map", out_group_id, DPP_NETROLE_MAP_AGENT);
			if (conf_obj) {
				char *netaccess_key = NULL;
				char *csign_key = NULL;
				char *pp_key = NULL;
				char *signed_connector_1905 = NULL;

				signed_connector_1905 = PLATFORM_STRDUP((char *)conf_obj->conn.signedConnector);

				csign_key = PLATFORM_ZALLOC(dpp_auth->conf->csign.subpkinfo_len * 2 + 1);
				bin2hexstr((char *)dpp_auth->conf->csign.subpkinfo, csign_key, dpp_auth->conf->csign.subpkinfo_len);

				pp_key = PLATFORM_ZALLOC(dpp_auth->conf->ppkey.subpkinfo_len * 2 + 1);
				bin2hexstr((char *)dpp_auth->conf->ppkey.subpkinfo, pp_key, dpp_auth->conf->ppkey.subpkinfo_len);

				netaccess_key = PLATFORM_ZALLOC(dpp_auth->own_protocol_key.priv_len * 2 + 1);
				bin2hexstr((char *)dpp_auth->own_protocol_key.priv, netaccess_key, dpp_auth->own_protocol_key.priv_len);

				PLATFORM_PRINTF_DEBUG("1905 signed_connector %s\n", signed_connector_1905);
				PLATFORM_PRINTF_DEBUG("netaccess key %s\n", netaccess_key);
				PLATFORM_PRINTF_DEBUG("csign key %s\n", csign_key);
				PLATFORM_PRINTF_DEBUG("pp key %s\n", pp_key);

				api_send_dpp_configuration(signed_connector_1905, netaccess_key, csign_key, pp_key);

				PLATFORM_FREE(signed_connector_1905);
				PLATFORM_FREE(csign_key);
				PLATFORM_FREE(pp_key);
				PLATFORM_FREE(netaccess_key);

				ret = dpp_build_peer_disc_req(&conf_obj->conn, &rx_msg, &rx_msg_len);
			}
		}
		if (out_group_id)
			PLATFORM_FREE(out_group_id);
	}

	if (ret == 1 && rx_msg != NULL && rx_msg_len != 0) {
		if (0 == send1905DirectEncapDPPPacket(interface_name, getNextMid(), mac, rx_msg, rx_msg_len)) {
			PLATFORM_PRINTF_INFO("Could not send 'Direct Encap DPP' message\n");
		}
	}

fail:
	PLATFORM_FREE(msg);
	PLATFORM_FREE(rx_msg);
	return ret;
}

// dpp_frame starts with vendor specific usage.
static INT8U processDPPProxiedRxAction(char *interface_name, INT8U *mac, INT8U *dpp_frame, INT16U dpp_frame_len)
{
	struct dpp_authentication *dpp_auth = NULL;

	INT16U msg_len    = 0;
	INT32U rx_msg_len = 0;
	INT8U *msg = NULL, *rx_msg = NULL;
	INT8U  ret = 0;

	dpp_auth = DMgetDPPAuthenticationObject();

	// Append first byte (0x04 Action Category Field)
	msg_len = dpp_frame_len + 1;
	msg     = PLATFORM_MALLOC(msg_len);
	msg[0]  = WLAN_ACTION_PUBLIC;
	PLATFORM_MEMCPY(msg + 1, dpp_frame, dpp_frame_len);

	// Process DPP Action Frame
	if (msg[1] == WLAN_PA_VENDOR_SPECIFIC) {
		enum dpp_public_action_frame_type type = msg[7];
		switch (type) {
		case DPP_PA_AUTHENTICATION_RESP:
			PLATFORM_PRINTF_INFO("Received authentication response\n");
			if ((ret = dpp_rx_auth_resp(dpp_auth, msg, msg_len)) == 1) {
				ret = dpp_auth_build_conf(dpp_auth, DPP_STATUS_OK, &rx_msg, &rx_msg_len);
				PLATFORM_PRINTF_INFO("Sending authentication confirm...\n");
			}
			break;
		case DPP_PA_CONFIGURATION_RESULT:
			PLATFORM_PRINTF_INFO("Received configuration result\n");
			if ((ret = dpp_conf_result_rx(dpp_auth, msg, msg_len)) == 1) {
				PLATFORM_PRINTF_INFO("DPP Configuration Completed!\n");
			}
			break;
		case DPP_PA_RECONFIG_ANNOUNCEMENT:
			PLATFORM_PRINTF_INFO("Received reconfiguration announcement\n");
			if ((ret = dpp_rx_reconfig_announcement(dpp_auth, msg, msg_len)) == 1) {
				ret = dpp_build_reconfig_auth_req(dpp_auth, &rx_msg, &rx_msg_len);
				PLATFORM_PRINTF_INFO("Sending reconfig authentication request...\n");
			}
			break;
		case DPP_PA_RECONFIG_AUTH_RESP:
			PLATFORM_PRINTF_INFO("Received reconfiguration authentication response\n");
			struct dpp_connector rx_conn;
			PLATFORM_MEMSET(&rx_conn, 0, sizeof(rx_conn));
			if ((ret = dpp_rx_reconfig_auth_resp(dpp_auth, &rx_conn, msg, msg_len)) == 1) {
				ret = dpp_build_reconfig_auth_conf(dpp_auth, &rx_msg, &rx_msg_len);
				PLATFORM_PRINTF_INFO("Sending reconfig authentication confirm...\n");
			}
			dpp_free_connector(&rx_conn);
			break;
		default:
			PLATFORM_PRINTF_WARNING("Unsupported DPP Frame Type %d\n", type);
			break;
		}

	} else if (msg[1] == WLAN_PA_GAS_INITIAL_REQ) {
		PLATFORM_PRINTF_INFO("Received dpp configuration request\n");

		// Entering configuration & authentication has successfully finished
		// Controller should instruct ALL agents to STOP advertising CCE
		if(DMisController()){
			triggerCCEAdvertisement(STOP_CCE_ADVERTISEMENT);
		}

		// Process GAS / DPP Configuration Request
		//
		// Parse DPP Configuration Request Object
		INT8U dialog_token = msg[2];
		struct dpp_config_req_obj req_obj;
		PLATFORM_MEMSET(&req_obj, 0, sizeof(struct dpp_config_req_obj));
		if ((ret = dpp_conf_req_rx(dpp_auth, &req_obj, msg, msg_len)) != 1) {
			PLATFORM_PRINTF_WARNING("Invalid config req obj\n");
			goto fail;
		}
		if (PLATFORM_STRCMP(req_obj.wifi_tech, "map") != 0) {
			PLATFORM_PRINTF_WARNING("wifi tech is not map\n");
			// goto fail;
		}
		if (req_obj.netRole != DPP_NETROLE_MAP_AGENT) {
			PLATFORM_PRINTF_WARNING("netRole is not mapAgent\n");
			// goto fail;
		}

		// Build DPP Configuration Response Objects
		int                   conf_obj_num = 2; // 1905 layer & backhaul STA
		struct dpp_config_obj conf_objs[conf_obj_num];
		PLATFORM_MEMSET(conf_objs, 0, sizeof(conf_objs));

		// Configuration Response Object for 1905 layer
		PLATFORM_STRCPY(conf_objs[0].wifi_tech, "map");
		conf_objs[0].dfCounterThreshold = RTK_DEFAULT_DECRYPT_FAILURE_THRESHOLD;
		PLATFORM_STRCPY(conf_objs[0].akm, "dpp");
		char *dummy_ssid = "mywifi";
		PLATFORM_STRCPY(conf_objs[0].ssid, dummy_ssid);
		conf_objs[0].ssid_len = strlen(dummy_ssid);
		if ((ret = dpp_build_signed_connector(dpp_auth->conf,
		                                      &dpp_auth->peer_protocol_key,
		                                      "mapNW", DPP_NETROLE_MAP_AGENT, &(conf_objs[0].conn)))
		    != 1) {
			PLATFORM_PRINTF_WARNING("Cannot generate DPP Signed Connector\n");
			goto fail;
		}

		// Configuration Response Object for backhaul STA
		if (2 == conf_obj_num) {
			struct configData *config_bh;
			INT8U  band;
			INT8U *op_class = dpp_get_op_class_from_conf_req(&req_obj);
			if (NULL == op_class) {
				band = INTERFACE_BAND_5GH;
			} else {
				band     = get_band_from_single_op_class(op_class[0]);
			}

			INT8U  result   = getBackhaulConfig(band, mac, &config_bh);

			while (result && band) {
				band   = band >> 1;
				result = getBackhaulConfig(band, mac, &config_bh);
			}

			if (band == INTERFACE_BAND_UNKNOWN) {
				PLATFORM_PRINTF_WARNING("Cannot generate DPP Signed Connector, no matching backhaul config found\n");
				goto fail;
			}

			convert_map_config_to_dpp_config_obj(dpp_auth, config_bh, &conf_objs[1]);
			if ((ret = dpp_build_signed_connector(dpp_auth->conf,
			                                      &dpp_auth->peer_protocol_key,
			                                      "mapNW", DPP_NETROLE_MAP_BACKHAUL_STA, &(conf_objs[1].conn)))
			    != 1) {
				PLATFORM_PRINTF_WARNING("Cannot generate DPP Signed Connector\n");
				goto fail;
			}
		}

		// Send DPP Configuration
		ret = dpp_build_conf_resp(dpp_auth, conf_objs, conf_obj_num, &rx_msg, &rx_msg_len, dialog_token);
		dpp_free_conf_obj(&conf_objs[0]);
		if (2 == conf_obj_num) {
			dpp_free_conf_obj(&conf_objs[1]);
		}
		PLATFORM_PRINTF_INFO("Sending configuration response...\n");
	}

	if (ret == 1 && rx_msg != NULL && rx_msg_len != 0) {
		struct Encap1905DPPTLV encap_1905_dpp_tlv;
		// remove PA header when encap msg into dpp encap content.
		encap_dpp_frame(rx_msg + 1, rx_msg_len - 1, DMgetDPPEnrollee(), &encap_1905_dpp_tlv);
		if (0 == send1905ProxiedEncapDPPPacket(interface_name, getNextMid(), mac, &encap_1905_dpp_tlv, NULL)) {
			PLATFORM_PRINTF_WARNING("Could not send 'Direct Encap DPP' message\n");
		}
	}

fail:
	PLATFORM_FREE(msg);
	PLATFORM_FREE(rx_msg);
	return ret;
}

static INT8U convert_dpp_config_obj_to_bss_config_data(struct dpp_config_obj * from,
                                                       struct bss_config_data *to)
{
	// Initialize destination to be clean
	PLATFORM_MEMSET(to, 0, sizeof(struct bss_config_data));

	// Configuration Object
	if (PLATFORM_STRCMP(from->wifi_tech, "inframap") == 0) {
		if (from->ssid_len) {
			to->network_type = MULTI_AP_FRONTHAUL_BSS_BIT;
		} else {
			to->network_type        = MULTI_AP_TEARDOWN_BIT;
			to->ssid                = PLATFORM_STRDUP("TEAR_DOWN");
			to->network_key         = PLATFORM_STRDUP("00000000");
			to->authentication_type = WPS_AUTH_WPA2PSK;
			to->encryption_type     = WPS_ENCR_AES;
			to->vid                 = 0;
			return 0;
		}
	} else if (PLATFORM_STRCMP(from->wifi_tech, "map") == 0) {
		to->network_type = MULTI_AP_BACKHAUL_BSS_BIT;
	} else {
		PLATFORM_PRINTF_ERROR("unknown configuartion type!\n");
		goto fail;
	}

	// Discovery Object
	if (from->ssid_len) {
		to->ssid = PLATFORM_STRDUP(from->ssid);
	} else {
		PLATFORM_PRINTF_ERROR("SSID is empty!\n");
		goto fail;
	}

	// Credential Object
	if (PLATFORM_STRCMP(from->akm, "psk") == 0) {
		int network_key_len = PLATFORM_STRLEN(from->psk_hex) / 2;
		to->network_key     = PLATFORM_MALLOC(network_key_len + 1);
		PLATFORM_MEMSET(to->network_key, 0, network_key_len + 1);
		hexstr2bin(from->psk_hex, to->network_key, network_key_len);
		to->authentication_type = WPS_AUTH_WPA2PSK;
	} else if (PLATFORM_STRCMP(from->akm, "sae") == 0) {
		to->network_key         = PLATFORM_STRDUP(from->pass);
		to->authentication_type = WPS_AUTH_WPA3;
	} else if (PLATFORM_STRSTR(from->akm, "dpp") && PLATFORM_STRSTR(from->akm, "sae") && PLATFORM_STRSTR(from->akm, "psk")) {
		to->network_key         = PLATFORM_STRDUP(from->pass);
		to->authentication_type = WPS_AUTH_WPA2PSK | WPS_AUTH_WPA3 | WPS_AUTH_DPP;
		to->signed_connector    = PLATFORM_STRDUP((char *)from->conn.signedConnector);
	} else if (PLATFORM_STRSTR(from->akm, "sae") && PLATFORM_STRSTR(from->akm, "psk")) {
		to->network_key         = PLATFORM_STRDUP(from->pass);
		to->authentication_type = WPS_AUTH_WPA2PSK | WPS_AUTH_WPA3;
	} else {
		PLATFORM_PRINTF_ERROR("unknown credential type %s !\n", from->akm);
		goto fail;
	}
	to->encryption_type = WPS_ENCR_AES;

	// VLAN ID
	to->vid = DMgetVIDbySSID(to->ssid);
	return 0;

fail:
	PLATFORM_SAFE_FREE((void **)&to->ssid);
	PLATFORM_SAFE_FREE((void **)&to->network_key);
	return 1;
}

INT8U processMultiAPR3Cmdu(struct CMDU *c, char *receiving_interface_name, INT8U *src_addr)
{
	INT8U ret = PROCESS_CMDU_OK;

	// Retreive destination mac address
	INT8U *al_addr = DMmacToAlMac(src_addr);
	INT8U *dst_mac = (al_addr) ? al_addr : src_addr;
	INT8U *src_mac = dst_mac;

	////////////////////////////////////////////////////////////////////////////////
	/// Profile 3 CMDUs
	////////////////////////////////////////////////////////////////////////////////
	switch (c->message_type) {
	case CMDU_TYPE_DIRECT_ENCAP_DPP: {
		INT8U *p;
		int    i = 0;
		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_DIRECT_ENCAP_DPP (%s) MID %04x\n",
		                     receiving_interface_name, c->message_id);
		PLATFORM_PRINTF_DEBUG("source mac: %02X:%02X:%02X:%02X:%02X:%02X\n",
		                     src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]);

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_DPP_MESSAGE: {
				struct DPPMessageTLV *t = (struct DPPMessageTLV *)p;
				PLATFORM_PRINTF_DEBUG("Received DPP Message TLV (length: %d)\n", t->dpp_frame_len);
				if (processDPPRxAction(receiving_interface_name, src_mac, t->dpp_frame, t->dpp_frame_len) != 1)
					PLATFORM_PRINTF_WARNING("Error occurred during DPP Rx\n");
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}
		break;
	}
	case CMDU_TYPE_PROXIED_ENCAP_DPP: {
		INT8U *p;
		int    i = 0;
		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_PROXIED_ENCAP_DPP (%s) MID %04x\n",
		                     receiving_interface_name, c->message_id);
		PLATFORM_PRINTF_DEBUG("source mac: %02X:%02X:%02X:%02X:%02X:%02X\n",
		                     src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]);

		struct Encap1905DPPTLV *encap_1905_dpp_tlv = NULL;
		struct DPPChirpValueTLV *dpp_chirp_value_tlv = NULL;

		INT8U  forward_to_broker = 1;

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_1905_ENCAP_DPP: {
				encap_1905_dpp_tlv = (struct Encap1905DPPTLV *)p;
				PLATFORM_PRINTF_DEBUG("Received 1905 Encap DPP TLV (length: %d)\n", encap_1905_dpp_tlv->encapsulated_frame_len);
				break;
			}
			case TLV_TYPE_DPP_CHIRP_VALUE: {
				dpp_chirp_value_tlv = (struct DPPChirpValueTLV *)p;
				PLATFORM_PRINTF_DEBUG("Received DPP CHIRP VALUE TLV (length: %d)\n", dpp_chirp_value_tlv->hash_len);
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		if (NULL == encap_1905_dpp_tlv) {
			PLATFORM_PRINTF_ERROR("Proxy Encap DPP contains no 1905 encap tlv!\n");
			break;
		}

		if (DMisController()) {
			PLATFORM_PRINTF_INFO("Received proxied encap from proxy agent now processing \n");
			// No need forward to broker if controller processed this message (e.g auth req, gas req, discovery req config result)
			forward_to_broker = !processDPPProxiedRxAction(receiving_interface_name, src_mac, encap_1905_dpp_tlv->encapsulated_frame, encap_1905_dpp_tlv->encapsulated_frame_len);
		}

		if (forward_to_broker) {
			PLATFORM_PRINTF_INFO("Received proxied encap from controller now send to dpp broker \n");
			INT8U *chirp_stream = NULL, *encap_stream = NULL;
			INT16U chirp_len = 0, encap_len = 0;
			struct dpp_proxy *proxy = NULL;

			if (dpp_chirp_value_tlv) {
				chirp_stream = forge_1905_TLV_from_structure((INT8U *)dpp_chirp_value_tlv, &chirp_len, 0);
				proxy = DMgetChirpProxy(dpp_chirp_value_tlv->hash_value, dpp_chirp_value_tlv->hash_len);
			}

			encap_stream = forge_1905_TLV_from_structure((INT8U *)encap_1905_dpp_tlv, &encap_len, 0);

			INT8U *message = (INT8U *)PLATFORM_MALLOC(chirp_len + encap_len);
			INT16U length = chirp_len + encap_len;

			if (chirp_stream)
				PLATFORM_MEMCPY(message, chirp_stream, chirp_len);

			PLATFORM_MEMCPY(message + chirp_len, encap_stream, encap_len);

			PLATFORM_PRINTF_INFO("Sending message to dpp_broker, message length: [%d] \n", length);
			send_to_dpp_broker(message, length);

			if (proxy) {
				proxy->proxy_auth_req_msg = (INT8U *)PLATFORM_MALLOC(length);
				PLATFORM_MEMCPY(proxy->proxy_auth_req_msg, message, length);
				proxy->proxy_auth_req_msg_len = length;
			}

			PLATFORM_FREE(message);

			if (chirp_stream)
				PLATFORM_FREE(chirp_stream);
			PLATFORM_FREE(encap_stream);
		}

		break;
	}
	case CMDU_TYPE_DPP_CCE_INDICATION: {
		INT8U *p;
		int i = 0;

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_DPP_CCE_INDICATION (%s) MID %04x\n",
		                     receiving_interface_name, c->message_id);
		PLATFORM_PRINTF_DEBUG("source mac: %02X:%02X:%02X:%02X:%02X:%02X\n",
		                     src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]);

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_DPP_CCE_INDICATION: {
				struct DPPCCEIndicationTLV *t = (struct DPPCCEIndicationTLV *)p;
				PLATFORM_PRINTF_DEBUG("[Multi-AP] DPP CCE Indication message advertise_cce field is [%d]. \n", t->advertise_cce);
				if (1 == t->advertise_cce || 0 == t->advertise_cce) {
					PLATFORM_PRINTF_DEBUG("[Multi-AP] Now requst hostapd to issue command with CCE IE status [%d] \n", t->advertise_cce);
					PLATFORM_MAP_ISSUE_CCE_IE_INDICATION(map_radio_name_24g, t->advertise_cce);
					PLATFORM_MAP_ISSUE_CCE_IE_INDICATION(map_radio_name_5gl, t->advertise_cce);
					if (DMisTribandDevice()) {
						PLATFORM_MAP_ISSUE_CCE_IE_INDICATION(map_radio_name_5gh, t->advertise_cce);
					}
				} else {
					PLATFORM_PRINTF_DEBUG("[Multi-AP] Unexpected value in DPP CCE Indication message advertise_cce field. \n");
				}
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}
		break;
	}

	case CMDU_TYPE_CHIRP_NOTIFICATION: { // recv it as controller during wireless bootstrapping/authentication, otherwise ignore
		INT8U *p;
		int i = 0;

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_CHIRP_NOTIFICATION (%s) MID %04x\n",
		                     receiving_interface_name, c->message_id);
		PLATFORM_PRINTF_DEBUG("source mac: %02X:%02X:%02X:%02X:%02X:%02X\n",
		                     src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]);

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_DPP_CHIRP_VALUE: {
				// here we need to compare the hash bi value to see if it is the same as a bi we received through ucc or any other methods.
				if (DMisController()) {
					struct DPPChirpValueTLV *chirp_tlv = (struct DPPChirpValueTLV *)p;
					PLATFORM_PRINTF_INFO("Received DPP Chirp Value TLV in Chirp Notification Message! \n");
					struct dpp_authentication *dpp_auth = DMgetDPPAuthenticationObject();

					if (PLATFORM_MEMCMP(chirp_tlv->hash_value, dpp_auth->peer_bi.pubkey_hash_chirp, chirp_tlv->hash_len) == 0) {
						PLATFORM_PRINTF_INFO("Hash values match, controller send Proxied Encap DPP message now \n");
						INT8U *auth_req_msg     = NULL;
						INT32U auth_req_msg_len = 0;

						if (dpp_auth_init(dpp_auth, &auth_req_msg, &auth_req_msg_len) == 1) {
							struct Encap1905DPPTLV *encap_1905_dpp_tlv = (struct Encap1905DPPTLV *)PLATFORM_MALLOC(sizeof(struct Encap1905DPPTLV));
							struct DPPChirpValueTLV *chirp_value_tlv = (struct DPPChirpValueTLV *)PLATFORM_MALLOC(sizeof(struct DPPChirpValueTLV));

							PLATFORM_MEMSET(encap_1905_dpp_tlv, 0, sizeof(struct Encap1905DPPTLV));
							// set encap_1905_dpp_tlv according to p35 of R3 spec.
							encap_1905_dpp_tlv->tlv_type = TLV_TYPE_1905_ENCAP_DPP;
							encap_1905_dpp_tlv->enrollee_mac_address_present |= BIT(7);
							PLATFORM_MEMCPY(encap_1905_dpp_tlv->dest_sta_mac_address, chirp_tlv->sta_mac_address, 6);
							encap_1905_dpp_tlv->encapsulated_frame_len = auth_req_msg_len - 1;
							encap_1905_dpp_tlv->encapsulated_frame = auth_req_msg + 1;

							// set chirp_value_tlv according to p35 of R3 spec. memcpy will do.
							PLATFORM_MEMCPY(chirp_value_tlv, chirp_tlv, sizeof(struct DPPChirpValueTLV));

							if (0 == send1905ProxiedEncapDPPPacket(receiving_interface_name, getNextMid(), dst_mac, encap_1905_dpp_tlv, chirp_value_tlv)) {
								PLATFORM_PRINTF_WARNING("Could not send 1905 Proxied Encap DPP message\n");
							}
							PLATFORM_FREE(encap_1905_dpp_tlv);
							PLATFORM_FREE(chirp_value_tlv);
						} else {
							PLATFORM_PRINTF_ERROR("ERROR: FAILED to generate dpp authentication request \n");
							break;
						}

					}
				} else {
					PLATFORM_PRINTF_ERROR("ERROR: Only controller should receive Chirp Notification Message \n");
				}
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}
		break;

	}
	case CMDU_TYPE_1905_ENCAP_EAPOL: {
		INT8U *p;
		int    i = 0;
		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_1905_ENCAP_EAPOL (%s) MID %04x\n",
		                     receiving_interface_name, c->message_id);
		PLATFORM_PRINTF_DEBUG("source mac: %02X:%02X:%02X:%02X:%02X:%02X\n",
		                     src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]);

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_1905_ENCAP_EAPOL: {
				struct Encap1905EAPOLTLV *t          = (struct Encap1905EAPOLTLV *)p;
				struct dpp_eapol_sm *     sm         = NULL;
				INT8U                     sm_ret     = 0;
				INT8U *                   rx_msg     = NULL;
				INT32U                    rx_msg_len = 0;

				PLATFORM_PRINTF_DEBUG("Received 1905 Encap EAPOL Message TLV (length: %d)\n", t->eapol_frame_payload_len);

				// Run EAPOL state machine
				DMgetDPPEAPOLStateMachine(&sm);
				// When it receives M1 and state is STARTED, reset M1 flag, start EAPOL flag
				// Unless it is receiving M1 for the first time, EAPOL flag should already be set
				if (sm->state == STARTED && !(DMisEventFlagSet(FLAG_ON_EAPOL))){
					DMresetEventFlag(FLAG_WAIT_FOR_M1);
					DMsetM1Delay(0);
					DMsetEventFlag(FLAG_ON_EAPOL);
				}
				sm_ret = dpp_eapol_sm_rx(sm, t->eapol_frame_payload, t->eapol_frame_payload_len, &rx_msg, &rx_msg_len);
				if (sm_ret == EAPOL_SM_OK_TX)
					send1905EncapEAPOLMessage(receiving_interface_name, getNextMid(), dst_mac, rx_msg, rx_msg_len);
				PLATFORM_FREE(rx_msg);

				// Check if 4-way handshake has just completed
				if (sm_ret != EAPOL_SM_FAIL && sm->state == FINISHED) {
					PLATFORM_PRINTF_INFO("[EAPOL] DPP EAPOL-4 Finished!\n");

					// Install Per-device PTK
					dpp_keystore_set_ptk(src_mac, &sm->ptk);
					PLATFORM_PRINTF_INFO("[EAPOL] Installed 1905 PTK successfully\n");

					// Install GTK if device is supplicant
					if (sm->role == SUPPLICANT) {
						dpp_keystore_set_gtk(&sm->gtk);
						PLATFORM_PRINTF_INFO("[EAPOL] Supplicant installed GTK successfully\n");
					}

					// Reset DPP related counters
					dpp_counters_reset(src_mac);
					struct networkDevice *network_device = DMnetworkDeviceGet(src_mac);
					if (network_device) {
						network_device->integrity_transmission_counter = 0;
					}

					struct dpp_authentication *dpp_auth = DMgetDPPAuthenticationObject();
					struct dpp_config_obj *conf_obj = dpp_find_config_obj(dpp_auth, "map", "*", DPP_NETROLE_MAP_AGENT);
					if (DMisController()) {
						PLATFORM_PRINTF_INFO("DFC: Controller, Set to default.\n");
						dpp_counters_set_dfc(src_mac, RTK_DEFAULT_DECRYPT_FAILURE_THRESHOLD);
					} else {
						if (conf_obj) {
							dpp_counters_set_dfc(src_mac, conf_obj->dfCounterThreshold);
						} else {
							PLATFORM_PRINTF_ERROR("DFC: No configuration object! Set to default.\n");
							dpp_counters_set_dfc(src_mac, RTK_DEFAULT_DECRYPT_FAILURE_THRESHOLD);
						}
					}

					// Agent should send BSS Config Request
					if (!DMisController() && DEVICE_FULLY_CONFIGURED != DMgetConfiguredState()) {
						PLATFORM_PRINTF_INFO("[EAPOL] Sending BSS Config Request...\n");
						sendBSSConfigRequestPacket(receiving_interface_name, getNextMid(), dst_mac);
					}
				}
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}
		break;
	}
	case CMDU_TYPE_1905_DECRYPTION_FAILURE: {
		INT8U *p;
		int    i = 0;

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_1905_DECRYPTION_FAILURE (%s) MID %04x\n",
		                     receiving_interface_name, c->message_id);
		PLATFORM_PRINTF_DEBUG("source mac: %02X:%02X:%02X:%02X:%02X:%02X\n",
		                     src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]);

		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_AL_MAC_ADDRESS_TYPE: {
				PLATFORM_PRINTF_DEBUG("Received 1905 AL MAC Address TLV\n");
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}
		break;
	}
	case CMDU_TYPE_BSS_CONFIG_REQUEST: {
		// AP Radio Basic Cap TLVs
		struct APRadioBasicCapabilitiesTLV *ap_radio_tlv[MAX_RECV_BSS_AP_RADIO_NR];
		INT8U                               ap_radio_tlv_nr = 0;

		// Configuration Object
		struct dpp_config_req_obj dpp_conf_req_obj;
		INT8U *                   dpp_conf_req_obj_attr     = NULL;
		INT16U                    dpp_conf_req_obj_attr_len = 0;

		INT8U *p;
		int    i = 0;

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_BSS_CONFIG_REQUEST (%s) MID %04x\n",
		                     receiving_interface_name, c->message_id);
		PLATFORM_PRINTF_DEBUG("source mac: %02X:%02X:%02X:%02X:%02X:%02X\n",
		                     src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]);

		// Extract TLVs
		//
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_MULTIAP_PROFILE: {
				// PLATFORM_PRINTF_INFO("<-- TLV_TYPE_MULTIAP_PROFILE \n");
				break;
			}
			case TLV_TYPE_SUPPORTED_SERVICE: {
				// PLATFORM_PRINTF_INFO("<-- TLV_TYPE_SUPPORTED_SERVICE \n");
				break;
			}
			case TLV_TYPE_AKM_SUITE_CAPABILITIES: {
				// PLATFORM_PRINTF_INFO("<-- TLV_TYPE_AKM_SUITE_CAPABILITIES \n");
				break;
			}
			case TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES: {
				// PLATFORM_PRINTF_INFO("<-- TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES \n");
				break;
			}
			case TLV_TYPE_PROFILE_2_CAPABILITY: {
				// PLATFORM_PRINTF_INFO("<-- TLV_TYPE_PROFILE_2_CAPABILITY \n");
				break;
			}
			case TLV_TYPE_AP_RADIO_ADVANCED_CAPABILITIES: {
				// PLATFORM_PRINTF_INFO("<-- TLV_TYPE_AP_RADIO_ADVANCED_CAPABILITIES \n");
				break;
			}
			// Save AP Radio Basic Cap TLVs
			case TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES: {
				// PLATFORM_PRINTF_INFO("<-- TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES \n");
				struct APRadioBasicCapabilitiesTLV *tlv = (struct APRadioBasicCapabilitiesTLV *)p;
				if (ap_radio_tlv_nr < MAX_RECV_BSS_AP_RADIO_NR) {
					ap_radio_tlv[ap_radio_tlv_nr] = tlv;
					ap_radio_tlv_nr++;
				} else {
					PLATFORM_PRINTF_WARNING("Only support %d AP Radio Basic Cap TLVs\n", MAX_RECV_BSS_AP_RADIO_NR);
				}
				break;
			}
			// Get Configuration Objects
			case TLV_TYPE_BSS_CONFIG_REQUEST: {
				PLATFORM_PRINTF_INFO("<-- TLV_TYPE_BSS_CONFIG_REQUEST \n");
				struct BSSConfigRequestTLV *tlv = (struct BSSConfigRequestTLV *)p;
				dpp_conf_req_obj_attr           = tlv->dpp_config_request_object;
				dpp_conf_req_obj_attr_len       = tlv->dpp_config_request_object_len;
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		// Handle Config Request
		//
		// Basic sanity check
		if (!ap_radio_tlv_nr || !dpp_conf_req_obj_attr_len) {
			PLATFORM_PRINTF_WARNING("No APRadioBasicCapabilitiesTLV or BSSConfigRequestTLV found!\n");
			break;
		}
		// Check Config Request Object
		if (dpp_map_bss_conf_req_rx(&dpp_conf_req_obj,
		                            dpp_conf_req_obj_attr, dpp_conf_req_obj_attr_len)
		    != 1) {
			PLATFORM_PRINTF_WARNING("Invalid configuration request object\n");
			break;
		}
		if (dpp_conf_req_obj.netRole != DPP_NETROLE_MAP_AGENT) {
			PLATFORM_PRINTF_WARNING("Non mapAgent netrole is currently not supported\n");
			break;
		}
		// Forge BSS Config Response
		if (DMisController()) {
			sendBSSConfigResponsePacket(receiving_interface_name, getNextMid(), src_mac, dst_mac,
			                            ap_radio_tlv, ap_radio_tlv_nr);
		}
		break;
	}
	case CMDU_TYPE_BSS_CONFIG_RESPONSE: {
		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_BSS_CONFIG_RESPONSE (%s) MID %04x\n",
		                     receiving_interface_name, c->message_id);
		PLATFORM_PRINTF_DEBUG("source mac: %02X:%02X:%02X:%02X:%02X:%02X\n",
		                     src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]);
		struct dpp_config_obj *conf_objs = (struct dpp_config_obj *)PLATFORM_MALLOC(sizeof(struct dpp_config_obj) * MAX_RECV_BSS_CONF_OBJ_NR);
		if (!conf_objs) {
			PLATFORM_PRINTF_ERROR("Fail to allocate buffer for conf_objs, process bss config response failed \n");
			break;
		}
		INT8U                 conf_objs_nr = 0;

		INT8U **vlan_tlvs   = NULL;
		INT8U   vlan_tlv_nr = 0;

		INT8U *p;
		int    i = 0;

		PLATFORM_MEMSET(conf_objs, 0, sizeof(struct dpp_config_obj) * MAX_RECV_BSS_CONF_OBJ_NR);

		// Extract TLVs
		//
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_BSS_CONFIG_RESPONSE: {
				PLATFORM_PRINTF_INFO("<-- TLV_TYPE_BSS_CONFIG_RESPONSE \n");
				struct BSSConfigResponseTLV *tlv = (struct BSSConfigResponseTLV *)p;
				if (dpp_map_bss_conf_resp_rx(conf_objs, &conf_objs_nr, MAX_RECV_BSS_CONF_OBJ_NR,
				                             tlv->dpp_config_object, tlv->dpp_config_object_len)
				    != 1) {
					PLATFORM_PRINTF_ERROR("Cannot Parse Config Resp Objects\n");
				}
				break;
			}
			case TLV_TYPE_DEFAULT_802_1Q_SETTINGS: {
				PLATFORM_PRINTF_INFO("<-- TLV_TYPE_DEFAULT_802_1Q_SETTINGS \n");
				struct Default8021QSettingsTLV *default_80211q_settings_tlv = (struct Default8021QSettingsTLV *)p;
				DMsetDefault8021QSettings(default_80211q_settings_tlv->primary_vlan_id,
				                          default_80211q_settings_tlv->default_pcp);
				vlan_tlv_nr += 1;
				vlan_tlvs                  = (INT8U **)PLATFORM_REALLOC(vlan_tlvs, sizeof(INT8U *) * (vlan_tlv_nr + 1));
				vlan_tlvs[vlan_tlv_nr - 1] = p;
				vlan_tlvs[vlan_tlv_nr]     = NULL;
				break;
			}
			case TLV_TYPE_TRAFFIC_SEPARATION_POLICY: {
				PLATFORM_PRINTF_INFO("<-- TLV_TYPE_TRAFFIC_SEPARATION_POLICY \n");
				struct TrafficSeparationPolicyTLV *traffic_separation_policy_tlv = (struct TrafficSeparationPolicyTLV *)p;
				DMsetTrafficSeparationPolicy(traffic_separation_policy_tlv);
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		// Default 802.1Q Settings TLV
		// Apply received vLANs
		if (vlan_tlv_nr && vlan_tlvs != NULL) {
			api_send_tlvs_response(MAP_VLAN_CONFIGURATION_ELEMENT, src_mac, vlan_tlvs);
			PLATFORM_FREE(vlan_tlvs);
		}

		// BSS Config Resp TLV
		// Sanity checks
		if (!conf_objs_nr) {
			PLATFORM_PRINTF_ERROR("No BSS config objects found!\n");
			PLATFORM_FREE(conf_objs);
			break;
		}

		PLATFORM_PRINTF_INFO("%d BSS config objects found, continue...\n", conf_objs_nr);

		// BSS Config Resp TLV
		// Initialize radio config objects
		struct radio_config_data  radio_config_24g, radio_config_5gl, radio_config_5gh;
		PLATFORM_MEMSET(&radio_config_24g, 0, sizeof(radio_config_24g));
		radio_config_24g.radio_band = MAP_CONFIG_2G;
		PLATFORM_MEMSET(&radio_config_5gl, 0, sizeof(radio_config_5gl));
		radio_config_5gl.radio_band = MAP_CONFIG_5GL;
		PLATFORM_MEMSET(&radio_config_5gh, 0, sizeof(radio_config_5gh));
		radio_config_5gh.radio_band = MAP_CONFIG_5GH;

		// Convert received configuration objects
		for (i = 0; i < conf_objs_nr; i++) {
			struct radio_config_data *radio_config   = NULL;
			char *                    interface_name = NULL;
			INT8U                     ruid[6]        = { 0 };

			// Get interface info from RUID
			hexstr2bin(conf_objs[i].ruid, (char *)ruid, RUID_ASCII_HEX_LEN);
			interface_name = DMmacToInterfaceName(ruid);

			if (PLATFORM_STRCMP(map_radio_name_24g, interface_name) == 0)
				radio_config = &radio_config_24g;
			else if (PLATFORM_STRCMP(map_radio_name_5gl, interface_name) == 0)
				radio_config = &radio_config_5gl;
			else if (PLATFORM_STRCMP(map_radio_name_5gh, interface_name) == 0)
				radio_config = &radio_config_5gh;

			if (!radio_config) {
				PLATFORM_PRINTF_WARNING("Cannot find back radio with RUID %02X:%02X:%02X:%02X:%02X:%02X\n",
				                        ruid[0], ruid[1], ruid[2], ruid[3], ruid[4], ruid[5]);
				dpp_free_conf_obj(&conf_objs[i]);
				continue;
			}

			// Append bss config data
			struct bss_config_data bss_config = {0};
			if (!convert_dpp_config_obj_to_bss_config_data(&conf_objs[i], &bss_config)) {
				radio_config->bss_data_nr++;
				radio_config->bss_data = PLATFORM_REALLOC(radio_config->bss_data,
				                                          sizeof(struct bss_config_data) * radio_config->bss_data_nr);
				PLATFORM_MEMCPY(&radio_config->bss_data[radio_config->bss_data_nr - 1],
				                &bss_config, sizeof(bss_config));
			} else {
				PLATFORM_PRINTF_ERROR("Error converting DPP config to BSS config\n");
			}

			dpp_free_conf_obj(&conf_objs[i]);
		}

		// Schedule BSS config result to be sent later
		_scheduleBSSConfigResult(dst_mac);

		// Apply parsed radio configs to the upper layer
		if (radio_config_24g.bss_data_nr) {
			PLATFORM_PRINTF_INFO("Sending 2.4g radio config...\n");
			api_send_config_request(radio_config_24g);
			_free_radio_config(&radio_config_24g);
			DMsetConfiguredState(DMgetConfiguredState() & (0xEF));
		}

		if (radio_config_5gl.bss_data_nr) {
			PLATFORM_PRINTF_INFO("Sending 5gl radio config...\n");
			api_send_config_request(radio_config_5gl);
			_free_radio_config(&radio_config_5gl);
			DMsetConfiguredState(DMgetConfiguredState() & (0xDF));
			if (!DMisTribandDevice()) {
				DMsetConfiguredState(DMgetConfiguredState() & (0xBF));
			}
		}

		if (radio_config_5gh.bss_data_nr) {
			PLATFORM_PRINTF_INFO("Sending 5gh radio config...\n");
			api_send_config_request(radio_config_5gh);
			_free_radio_config(&radio_config_5gh);
			DMsetConfiguredState(DMgetConfiguredState() & (0xBF));
		}

		PLATFORM_FREE(conf_objs);
		break;
	}
	case CMDU_TYPE_BSS_CONFIG_RESULT: {
		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_BSS_CONFIG_RESULT (%s) MID %04x\n",
		                     receiving_interface_name, c->message_id);
		PLATFORM_PRINTF_INFO("source mac: %02X:%02X:%02X:%02X:%02X:%02X\n",
		                     src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]);
		// TODO: send Agent List CMDU to enrollee MAP AGT and all other existing MAP AGT (EasyMesh Spec 5.3.8)
		break;
	}
	case CMDU_TYPE_SERVICE_PRIORITIZATION_REQUEST: {
		INT8U *                        p;
		INT8U                          i = 0, p2_error_code_nr = 0, error_code = 0;
		struct Profile2ErrorCodeTLV *  p2_error_code = NULL;

		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_SERVICE_PRIORITIZATION_REQUEST (%s) MID %04x \n",
		                     receiving_interface_name, c->message_id);
		PLATFORM_PRINTF_DEBUG("source mac: %02X:%02X:%02X:%02X:%02X:%02X \n",
		                     src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]);

		// respond within 1 second with 1905 ACK
		if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
			PLATFORM_PRINTF_ERROR("[Multi-AP] Sending ACK for CMDU_TYPE_SERVICE_PRIORITIZATION_REQUEST MID: (%x) is failed\n", c->message_id);
		}
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_SERVICE_PRIORITIZATION_RULE: {
				PLATFORM_PRINTF_DEBUG("<-- TLV_TYPE_SERVICE_TLV_TYPE_SERVICE_PRIORITIZATION_RULE \n");

				struct ServicePrioritizationRuleTLV *t = (struct ServicePrioritizationRuleTLV *)p;
				error_code                             = DMsetServicePrioritizationRule(t);
				// Check whether rule received exceed capability or rule to be removed not found
				if (error_code != 0) {
					if (0 == p2_error_code_nr) {
						p2_error_code = (struct Profile2ErrorCodeTLV *)PLATFORM_MALLOC(sizeof(struct Profile2ErrorCodeTLV));
					} else {
						p2_error_code = (struct Profile2ErrorCodeTLV *)PLATFORM_REALLOC(p2_error_code, sizeof(struct Profile2ErrorCodeTLV) * (p2_error_code_nr + 1));
					}
					p2_error_code[p2_error_code_nr].reason_code = error_code;
					p2_error_code_nr++;
				} else { // Successfully add or remove rule, continue to process in driver
					struct ServicePrioritizationRuleTLV *rule = findDominantServicePrioritizationRule();
					if (rule) {
						PLATFORM_SET_SP_RULE(rule);
					} else {
						PLATFORM_CLEAR_SP_RULE();
					}
				}
				break;
			}
			case TLV_TYPE_DSCP_MAPPING_TABLE: {
				PLATFORM_PRINTF_INFO("<-- TLV_TYPE_DSCP_MAPPING_TABLE \n");
				struct DSCPMappingTableTLV *t = (struct DSCPMappingTableTLV *)p;
				PLATFORM_SET_DSCP_PCP_MAPPING_TABLE(t);
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}

		// Send Profile-2 Error Response if any error exists
		if (p2_error_code_nr > 0) {
			if (0 == sendErrorResponsePacket(receiving_interface_name, c->message_id, dst_mac, p2_error_code_nr, p2_error_code)) {
				PLATFORM_PRINTF_ERROR("[Multi-AP] Sending Error Response for CMDU_TYPE_ERROR_RESPONSE MID: (%x) is failed\n", c->message_id);
			}
		}

		if (p2_error_code) {
			PLATFORM_FREE(p2_error_code);
		}

		break;
	}
	case CMDU_TYPE_AGENT_LIST: {
		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_AGENT_LIST (%s) MID %04x \n", receiving_interface_name, c->message_id);
		INT8U *p;
		INT8U  i, j;
		i = 0;
		while (NULL != (p = c->list_of_TLVs[i])) {
			switch (*p) {
			case TLV_TYPE_AGENT_LIST: {
				// PLATFORM_PRINTF_INFO("<-- TLV_TYPE_AGENT_LIST \n");
				struct AgentListTLV *t = (struct AgentListTLV *)p;

				INT8U agents_nr, map_profile, security;
				agents_nr = t->agents_nr;

				for (j = 0; j < agents_nr; j++) {
					map_profile = t->agents[j].map_profile;
					if (map_profile == 0 || map_profile > 3) {
						PLATFORM_PRINTF_WARNING("Multi-AP Profile value of agent %d is not valid!\n", j);
					} else {
						PLATFORM_PRINTF_INFO("Agent %d is MAP Profile %d.\n", j, map_profile);
					}

					security = t->agents[j].security;
					if (security == 1) {
						PLATFORM_PRINTF_INFO("Agent %d 1905 Security is enabled.\n", j);
					} else if (security == 0) {
						PLATFORM_PRINTF_INFO("Agent %d 1905 Security is not enabled.\n", j);
					} else {
						PLATFORM_PRINTF_WARNING("1905 Security value of agent %d is not valid!\n", j);
					}
					DMsetSecurity(t->agents[j].al_mac_address, security);
				}
				break;
			}
			default: {
				PLATFORM_PRINTF_WARNING("[Multi-AP] Unexpected TLV (%d) type inside CMDU\n", *p);
				break;
			}
			}
			i++;
		}
		break;
	}
	case CMDU_TYPE_RECONFIG_TRIGGER: {
		PLATFORM_PRINTF_INFO("<-- CMDU_TYPE_RECONFIG_TRIGGER (%s) MID %04x \n", receiving_interface_name, c->message_id);

		if (!DMisController()) {
			// Respond within 1 second with 1905 ACK
			if (0 == send1905ACKPacket(receiving_interface_name, c->message_id, dst_mac, 0, NULL)) {
				PLATFORM_PRINTF_INFO("[Multi-AP] Sending ACK for CMDU_TYPE_RECONFIG_TRIGGER MID: (%x) is failed\n", c->message_id);
			}
			// Send a BSS Config Request message to MAP Controller
			// to receive fresh fhBSS and/or bhBSS config info
			PLATFORM_PRINTF_INFO("[EAPOL] Sending BSS Config Request...\n");
			sendBSSConfigRequestPacket(receiving_interface_name, getNextMid(), DMcontrollerMacGet());
		}

		break;
	}
	default: {
		PLATFORM_PRINTF_WARNING("Unknown CMDU type %x received with mid %x on R3 device, drop packet.\n", c->message_type, c->message_id);
		break;
	}
	}

	PLATFORM_FREE(al_addr);
	return ret;
}
#endif

INT8U processLlpdPayload(struct PAYLOAD *payload, char *receiving_interface_name)
{
	INT8U *p;
	INT8U  i;

	INT8U dummy_mac_address[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	INT8U al_mac_address[6];
	INT8U mac_address[6];

	PLATFORM_MEMCPY(al_mac_address, dummy_mac_address, 6);
	PLATFORM_MEMCPY(mac_address, dummy_mac_address, 6);

	if (NULL == payload) {
		return 0;
	}

	PLATFORM_PRINTF_INFO("<-- LLDP BRIDGE DISCOVERY (%s)\n", receiving_interface_name);

	// We need to update the data model structure, which keeps track
	// of local interfaces, neighbors, and neighbors' interfaces, and
	// what type of discovery messages ("topology discovery" and/or
	// "bridge discovery") have been received on each link.

	// First, extract the AL MAC and MAC addresses of the interface
	// which transmitted this bridge discovery message
	//
	i = 0;
	while (NULL != (p = payload->list_of_TLVs[i])) {
		switch (*p) {
		case TLV_TYPE_CHASSIS_ID: {
			struct chassisIdTLV *t = (struct chassisIdTLV *)p;

			if (CHASSIS_ID_TLV_SUBTYPE_MAC_ADDRESS == t->chassis_id_subtype) {
				PLATFORM_MEMCPY(al_mac_address, t->chassis_id, 6);
			}

			break;
		}
		case TLV_TYPE_MAC_ADDRESS_TYPE: {
			struct portIdTLV *t = (struct portIdTLV *)p;

			if (PORT_ID_TLV_SUBTYPE_MAC_ADDRESS == t->port_id_subtype) {
				PLATFORM_MEMCPY(mac_address, t->port_id, 6);
			}

			break;
		}
		case TLV_TYPE_TIME_TO_LIVE: {
			break;
		}
		default: {
			PLATFORM_PRINTF_DETAIL("Ignoring TLV type %d\n", *p);
			break;
		}
		}
		i++;
	}

	// Make sure that both the AL MAC and MAC addresses were contained
	// in the CMDU
	//
	if (0 == PLATFORM_MEMCMP(al_mac_address, dummy_mac_address, 6) || 0 == PLATFORM_MEMCMP(mac_address, dummy_mac_address, 6)) {
		PLATFORM_PRINTF_WARNING("More TLVs were expected inside this LLDP message\n");
		return 0;
	}

	// Avoid insert neighbor from LLDP if it was not found from topology discovery
	if (!DMisExistingNeighbor(receiving_interface_name, al_mac_address))
		return 1;

	PLATFORM_PRINTF_DETAIL("AL MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n", al_mac_address[0], al_mac_address[1], al_mac_address[2], al_mac_address[3], al_mac_address[4], al_mac_address[5]);
	PLATFORM_PRINTF_DETAIL("MAC    address = %02x:%02x:%02x:%02x:%02x:%02x\n", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);

	// Finally, update the data model
	//
	if (0 == DMupdateDiscoveryTimeStamps(receiving_interface_name, al_mac_address, mac_address, TIMESTAMP_BRIDGE_DISCOVERY, NULL)) {
		PLATFORM_PRINTF_WARNING("Problems updating data model with topology response TLVs\n");
		return 0;
	}

	return 1;
}

INT8U process1905Alme(INT8U *alme_tlv, INT8U alme_client_id)
{
	if (NULL == alme_tlv) {
		return 0;
	}

	// The first byte of the 'alme_tlv' structure always contains its type
	//
	switch (*alme_tlv) {
	case ALME_TYPE_GET_INTF_LIST_REQUEST: {
		// Obtain the list of local interfaces, retrieve detailed info for
		// each of them, build a response, and send it back.
		//
		PLATFORM_PRINTF_INFO("<-- ALME_TYPE_GET_INTF_LIST_REQUEST\n");

		send1905InterfaceListResponseALME(alme_client_id);

		break;
	}
	case ALME_TYPE_SET_INTF_PWR_STATE_REQUEST: {
		PLATFORM_PRINTF_INFO("<-- ALME_TYPE_SET_INTF_PWR_STATE_REQUEST\n");
		break;
	}
	case ALME_TYPE_GET_INTF_PWR_STATE_REQUEST: {
		PLATFORM_PRINTF_INFO("<-- ALME_TYPE_GET_INTF_PWR_STATE_REQUEST\n");
		break;
	}
	case ALME_TYPE_SET_FWD_RULE_REQUEST: {
		PLATFORM_PRINTF_INFO("<-- ALME_TYPE_SET_FWD_RULE_REQUEST\n");
		break;
	}
	case ALME_TYPE_GET_FWD_RULES_REQUEST: {
		PLATFORM_PRINTF_INFO("<-- ALME_TYPE_GET_FWD_RULES_REQUEST\n");
		break;
	}
	case ALME_TYPE_MODIFY_FWD_RULE_REQUEST: {
		PLATFORM_PRINTF_INFO("<-- ALME_TYPE_MODIFY_FWD_RULE_REQUEST\n");
		break;
	}
	case ALME_TYPE_REMOVE_FWD_RULE_REQUEST: {
		PLATFORM_PRINTF_INFO("<-- ALME_TYPE_MODIFY_FWD_RULE_CONFIRM\n");
		break;
	}
	case ALME_TYPE_GET_METRIC_REQUEST: {
		// Obtain the requested metrics, build a response, and send it back.
		//
		struct getMetricRequestALME *p;

		INT8U dummy_mac_address[6] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

		PLATFORM_PRINTF_INFO("<-- ALME_TYPE_GET_METRIC_REQUEST\n");

		p = (struct getMetricRequestALME *)alme_tlv;

		if (0 == PLATFORM_MEMCMP(p->interface_address, dummy_mac_address, 6)) {
			// Request metrics against all neighbors
			//
			send1905MetricsResponseALME(alme_client_id, NULL);
		} else {
			// Request metrics against one specific neighbor
			//
			send1905MetricsResponseALME(alme_client_id, p->interface_address);
		}

		break;
	}
	case ALME_TYPE_CUSTOM_COMMAND_REQUEST: {
		struct customCommandRequestALME *p;

		PLATFORM_PRINTF_INFO("<-- ALME_TYPE_CUSTOM_COMMAND_REQUEST\n");

		p = (struct customCommandRequestALME *)alme_tlv;

		send1905CustomCommandResponseALME(alme_client_id, p->command);

		break;
	}
	case ALME_TYPE_MULTIAP_COMMAND_REQUEST: {
		struct multiApCommandRequestALME *p;

		PLATFORM_PRINTF_INFO("<-- ALME_TYPE_MULTIAP_COMMAND_REQUEST\n");

		p = (struct multiApCommandRequestALME *)alme_tlv;

		sendMultiApCommandResponseALME(alme_client_id, p);
		break;
	}
	case ALME_TYPE_GET_INTF_LIST_RESPONSE:
	case ALME_TYPE_SET_INTF_PWR_STATE_CONFIRM:
	case ALME_TYPE_GET_INTF_PWR_STATE_RESPONSE:
	case ALME_TYPE_SET_FWD_RULE_CONFIRM:
	case ALME_TYPE_GET_FWD_RULES_RESPONSE:
	case ALME_TYPE_MODIFY_FWD_RULE_CONFIRM:
	case ALME_TYPE_REMOVE_FWD_RULE_CONFIRM:
	case ALME_TYPE_GET_METRIC_RESPONSE:
	case ALME_TYPE_CUSTOM_COMMAND_RESPONSE:
	case ALME_TYPE_MULTIAP_COMMAND_RESPONSE: {
		// These messages should never be receiving by an AL entity. It is
		// the AL entity the one who generates them and then sends them to
		// the HLE.
		//
		PLATFORM_PRINTF_WARNING("ALME RESPONSE/CONFIRM message received (type = %d). Ignoring...\n", *alme_tlv);
		break;
	}

	default: {
		PLATFORM_PRINTF_WARNING("Unknown ALME message received (type = %d). Ignoring...\n", *alme_tlv);
		break;
	}
	}

	return 1;
}

INT8U processAPIRequest(INT8U *element)
{
	INT8U ret = PROCESS_CMDU_OK;
	int i = 0;
	switch (*element) {
	case MAP_TOPOLOGY_QUERY_REQUEST_ELEMENT: {
		struct map_topology_query_request *req = (struct map_topology_query_request *)element;
		struct networkDevice *             tmp = DMnetworkDeviceGet(req->target_al_mac);
		if (NULL != tmp) {
			if (0 == send1905TopologyQueryPacket(tmp->receiving_interface, getNextMid(), req->target_al_mac, 1)) {
				ret = PROCESS_CMDU_KO;
			}
		} else {
			char **backhaul_interfaces;
			INT8U  backhaul_interfaces_nr;
			backhaul_interfaces = (char **)get_backhaul_interfaces(&backhaul_interfaces_nr);
			for (i = 0; i < backhaul_interfaces_nr; i++) {
				if (0 == send1905TopologyQueryPacket(backhaul_interfaces[i], getNextMid(), req->target_al_mac, 1)) {
					PLATFORM_PRINTF_WARNING("Could not send 'topology query' message\n");
				}
			}
			PLATFORM_FREE(backhaul_interfaces);
		}
		api_send_general_acknowledgement(*element, req->target_al_mac, ret);
		break;
	}
	case MAP_AUTOCONFIG_SEARCH_REQUEST_ELEMENT: {
		PLATFORM_PRINTF_INFO("<---- MAP_AUTOCONFIG_SEARCH_REQUEST_ELEMENT (API)\n");
		char **backhaul_interfaces;
		INT8U  backhaul_interfaces_nr;
		backhaul_interfaces = (char **)get_backhaul_interfaces(&backhaul_interfaces_nr);
		INT16U mid          = getNextMid();
		for (i = 0; i < backhaul_interfaces_nr; i++) {
			send1905APAutoconfigurationSearchPacket(backhaul_interfaces[i], mid, IEEE80211_FREQUENCY_BAND_5_GHZ);
		}
		PLATFORM_FREE(backhaul_interfaces);
		break;
	}
	case MAP_AUTOCONFIG_RENEW_REQUEST_ELEMENT: {
		PLATFORM_PRINTF_INFO("<---- MAP_AUTOCONFIG_RENEW_REQUEST_ELEMENT (API)\n");

		if (!DMisController()) {
			PLATFORM_PRINTF_ERROR("AutoconfigRenew request received on agent, ignore the request\n");
			break;
		}
		struct map_autoconfig_renew_request *req = (struct map_autoconfig_renew_request *)element;
		struct networkDevice *               tmp = DMnetworkDeviceGet(req->target_al_mac);
		PLATFORM_PRINTF_INFO("MAP_AUTOCONFIG_RENEW_REQUEST_ELEMENT %02x:%02x:%02x:%02x:%02x:%02x\n", req->target_al_mac[0], req->target_al_mac[1], req->target_al_mac[2], req->target_al_mac[3], req->target_al_mac[4], req->target_al_mac[5]);
		if (NULL != tmp) {
			if (DMgetAutoconfigRenewBand() == 2) {
				PLATFORM_PRINTF_INFO("Send Autoconfiguration Renew packet to both 5G and 24g (%d)\n", __LINE__);
				send1905APAutoconfigurationRenewPacket(tmp->receiving_interface, getNextMid(), req->target_al_mac, IEEE80211_FREQUENCY_BAND_5_GHZ);
				send1905APAutoconfigurationRenewPacket(tmp->receiving_interface, getNextMid(), req->target_al_mac, IEEE80211_FREQUENCY_BAND_2_4_GHZ);
			} else if (DMgetAutoconfigRenewBand() == 1) {
				PLATFORM_PRINTF_INFO("Send Autoconfiguration Renew packet to 5G (%d)\n", __LINE__);
				send1905APAutoconfigurationRenewPacket(tmp->receiving_interface, getNextMid(), req->target_al_mac, IEEE80211_FREQUENCY_BAND_5_GHZ);
			}
		} else {
			char **backhaul_interfaces;
			INT8U  backhaul_interfaces_nr;
			backhaul_interfaces = (char **)get_backhaul_interfaces(&backhaul_interfaces_nr);
			for (i = 0; i < backhaul_interfaces_nr; i++) {
				if (DMgetAutoconfigRenewBand() == 2) {
					PLATFORM_PRINTF_INFO("Send Autoconfiguration Renew packet to both 5G and 24g (%d)\n", __LINE__);
					send1905APAutoconfigurationRenewPacket(backhaul_interfaces[i], getNextMid(), req->target_al_mac, IEEE80211_FREQUENCY_BAND_5_GHZ);
					send1905APAutoconfigurationRenewPacket(backhaul_interfaces[i], getNextMid(), req->target_al_mac, IEEE80211_FREQUENCY_BAND_2_4_GHZ);
				} else if (DMgetAutoconfigRenewBand() == 1) {
					PLATFORM_PRINTF_INFO("Send Autoconfiguration Renew packet to 5G (%d)\n", __LINE__);
					send1905APAutoconfigurationRenewPacket(backhaul_interfaces[i], getNextMid(), req->target_al_mac, IEEE80211_FREQUENCY_BAND_5_GHZ);
				}
			}
			PLATFORM_FREE(backhaul_interfaces);
		}
		break;
	}

	case MAP_CHANNEL_PREFERENCE_QUERY_ELEMENT: {
		struct map_channel_preference_query *req = (struct map_channel_preference_query *)element;
		struct networkDevice *               tmp = DMnetworkDeviceGet(req->target_al_mac);
		if (NULL != tmp) {
			sendMultiAPChannelPreferenceQueryPacket(tmp->receiving_interface, getNextMid(), req->target_al_mac);
		} else {
			char **backhaul_interfaces;
			INT8U  backhaul_interfaces_nr;
			backhaul_interfaces = (char **)get_backhaul_interfaces(&backhaul_interfaces_nr);
			for (i = 0; i < backhaul_interfaces_nr; i++) {
				sendMultiAPChannelPreferenceQueryPacket(backhaul_interfaces[i], getNextMid(), req->target_al_mac);
			}
			PLATFORM_FREE(backhaul_interfaces);
		}
		break;
	}

	case MAP_CHANNEL_SELECTION_REQUEST_ELEMENT: {
		INT8U                                 tlv_index, k;
		struct map_channel_selection_request *req = (struct map_channel_selection_request *)element;

		/* create Channel Preference TLVs by the information from user */
		struct ChannelPreferenceTLV **channel_preferences = (struct ChannelPreferenceTLV **)PLATFORM_MALLOC(sizeof(struct ChannelPreferenceTLV *) * req->ch_preference_nr);
		for (tlv_index = 0; tlv_index < req->ch_preference_nr; tlv_index++) {
			channel_preferences[tlv_index]           = (struct ChannelPreferenceTLV *)PLATFORM_MALLOC(sizeof(struct ChannelPreferenceTLV));
			channel_preferences[tlv_index]->tlv_type = TLV_TYPE_CHANNEL_PREFERENCE;
			PLATFORM_MEMCPY(channel_preferences[tlv_index]->radio_unique_id, req->ch_preference[tlv_index].radio_mac, MACADDRLEN);
			channel_preferences[tlv_index]->op_class_nr         = req->ch_preference[tlv_index].op_class_nr;
			channel_preferences[tlv_index]->channel_preferences = (struct ChannelPreferencePerOpClass *)PLATFORM_MALLOC(sizeof(struct ChannelPreferencePerOpClass) * channel_preferences[tlv_index]->op_class_nr);
			for (k = 0; k < channel_preferences[tlv_index]->op_class_nr; k++) {
				channel_preferences[tlv_index]->channel_preferences[k].op_class     = req->ch_preference[tlv_index].ch_pre_op_class[k].op_class;
				channel_preferences[tlv_index]->channel_preferences[k].channel_nr   = req->ch_preference[tlv_index].ch_pre_op_class[k].channel_nr;
				channel_preferences[tlv_index]->channel_preferences[k].channel_list = NULL;
				if (channel_preferences[tlv_index]->channel_preferences[k].channel_nr) {
					channel_preferences[tlv_index]->channel_preferences[k].channel_list = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * channel_preferences[tlv_index]->channel_preferences[k].channel_nr);
					PLATFORM_MEMCPY(channel_preferences[tlv_index]->channel_preferences[k].channel_list, req->ch_preference[tlv_index].ch_pre_op_class[k].channel_list, channel_preferences[tlv_index]->channel_preferences[k].channel_nr);
			}
				channel_preferences[tlv_index]->channel_preferences[k].preference_reason_code = req->ch_preference[tlv_index].ch_pre_op_class[k].preference_reason_code;
			}
		}

		/* create Transmit Power Limit TLVs */
		struct TransmitPowerLimitTLV **transmit_power_limits = (struct TransmitPowerLimitTLV **)PLATFORM_MALLOC(sizeof(struct TransmitPowerLimitTLV *) * req->tx_pwr_limit_nr);
		for (tlv_index = 0; tlv_index < req->tx_pwr_limit_nr; tlv_index++) {
			transmit_power_limits[tlv_index]           = (struct TransmitPowerLimitTLV *)PLATFORM_MALLOC(sizeof(struct TransmitPowerLimitTLV));
			transmit_power_limits[tlv_index]->tlv_type = TLV_TYPE_TRANSMIT_POWER_LIMIT;
			PLATFORM_MEMCPY(transmit_power_limits[tlv_index]->radio_unique_id, req->tx_pwr_limit[tlv_index].radio_mac, MACADDRLEN);
			transmit_power_limits[tlv_index]->transmit_power_limit = req->tx_pwr_limit[tlv_index].tx_pwr_limit;
		}

		struct networkDevice *tmp = DMnetworkDeviceGet(req->target_al_mac);
		if (NULL != tmp) {
			if (0 == sendMultiAPChannelSelectionRequestPacket(tmp->receiving_interface, getNextMid(), req->target_al_mac, channel_preferences,req->ch_preference_nr, transmit_power_limits, req->tx_pwr_limit_nr)) {
				ret = PROCESS_CMDU_KO;
			}
		} else {
			char **backhaul_interfaces;
			INT8U  backhaul_interfaces_nr;
			backhaul_interfaces = (char **)get_backhaul_interfaces(&backhaul_interfaces_nr);
			for (i = 0; i < backhaul_interfaces_nr; i++) {
				sendMultiAPChannelSelectionRequestPacket(backhaul_interfaces[i], getNextMid(), req->target_al_mac, channel_preferences,req->ch_preference_nr, transmit_power_limits, req->tx_pwr_limit_nr);
			}
			PLATFORM_FREE(backhaul_interfaces);
		}
		api_send_general_acknowledgement(*element, req->target_al_mac, ret);
		/* release TLV space */
		for (i = 0; i < req->ch_preference_nr; i++) {
			free_1905_TLV_structure((INT8U *)channel_preferences[i]);
		}
		PLATFORM_FREE(channel_preferences);

		for (i = 0; i < req->tx_pwr_limit_nr; i++) {
			free_1905_TLV_structure((INT8U *)transmit_power_limits[i]);
		}
		PLATFORM_FREE(transmit_power_limits);
		break;
	}
#ifdef RTK_MULTI_AP_R2
	case MAP_CAC_REQUEST_ELEMENT: {
		PLATFORM_PRINTF_INFO("<---- MAP_CAC_REQUEST_ELEMENT (API)\n");
		struct map_cac_request *req = (struct map_cac_request *)element;
		struct networkDevice *  tmp = DMnetworkDeviceGet(req->target_al_mac);
		struct CACRequestTLV    cac_request_tlv = { 0 };

		cac_request_tlv.tlv_type = TLV_TYPE_CAC_REQUEST;
		cac_request_tlv.radio_nr = req->cac_req.radio_nr;
		cac_request_tlv.radios   = (struct CACRequestRadio*)PLATFORM_MALLOC(cac_request_tlv.radio_nr * sizeof(struct CACRequestRadio));
		for (i = 0; i < cac_request_tlv.radio_nr; i++) {
			PLATFORM_MEMCPY(cac_request_tlv.radios[i].radio_unique_identifier, req->cac_req.radios[i].radio_id, 6);
			cac_request_tlv.radios[i].op_class	= req->cac_req.radios[i].op_class;
			cac_request_tlv.radios[i].channel	= req->cac_req.radios[i].channel;
			cac_request_tlv.radios[i].flags		= req->cac_req.radios[i].flags;
		}
		if (NULL != tmp) {
			sendCACRequestPacket(tmp->receiving_interface, getNextMid(), req->target_al_mac, cac_request_tlv);
		} else {
			char **backhaul_interfaces;
			INT8U  backhaul_interfaces_nr;
			backhaul_interfaces = (char **)get_backhaul_interfaces(&backhaul_interfaces_nr);
			for (i = 0; i < backhaul_interfaces_nr; i++) {
				sendCACRequestPacket(backhaul_interfaces[i], getNextMid(), req->target_al_mac, cac_request_tlv);
			}

			PLATFORM_FREE(backhaul_interfaces);
		}

		PLATFORM_FREE(cac_request_tlv.radios);

		api_send_general_acknowledgement(*element, req->target_al_mac, ret);
		break;
	}
	case MAP_CHANNEL_SCAN_REQUEST_ELEMENT: {
		PLATFORM_PRINTF_INFO("<---- MAP_CHANNEL_SCAN_REQUEST_ELEMENT (API)\n");
		struct map_channel_scan_request   *req = (struct map_channel_scan_request *)element;
		struct networkDevice *             tmp = DMnetworkDeviceGet(req->target_al_mac);
		struct ChannelScanRequestTLV       channel_scan_req_tlv = { 0 };
		int    j;

		channel_scan_req_tlv.tlv_type = TLV_TYPE_CHANNEL_SCAN_REQUEST;
		channel_scan_req_tlv.flags    = req->channel_scan_req.flags;
		channel_scan_req_tlv.target_radio_nr = req->channel_scan_req.target_radio_nr;
		if (channel_scan_req_tlv.target_radio_nr) {
			channel_scan_req_tlv.target_radios   = (struct ChannelScanTargetRadio*)PLATFORM_MALLOC(channel_scan_req_tlv.target_radio_nr * sizeof(struct ChannelScanTargetRadio));
			for(i = 0; i < channel_scan_req_tlv.target_radio_nr; i++) {
				PLATFORM_MEMCPY(channel_scan_req_tlv.target_radios[i].radio_unique_identifier, req->channel_scan_req.target_radios[i].radio_id, 6);
				channel_scan_req_tlv.target_radios[i].target_op_class_nr = req->channel_scan_req.target_radios[i].target_op_class_nr;
				channel_scan_req_tlv.target_radios[i].target_op_classes  = (struct ChannelScanTargetOpClass*)PLATFORM_MALLOC(channel_scan_req_tlv.target_radios[i].target_op_class_nr * sizeof(struct ChannelScanTargetOpClass));
				for(j = 0; j < channel_scan_req_tlv.target_radios[i].target_op_class_nr; j++) {
					channel_scan_req_tlv.target_radios[i].target_op_classes[j].op_class = req->channel_scan_req.target_radios[i].target_op_classes[j].op_class;
					channel_scan_req_tlv.target_radios[i].target_op_classes[j].target_channel_nr = req->channel_scan_req.target_radios[i].target_op_classes[j].target_channel_nr;

					if(channel_scan_req_tlv.target_radios[i].target_op_classes[j].target_channel_nr) {
						channel_scan_req_tlv.target_radios[i].target_op_classes[j].target_channel_list = (INT8U *)PLATFORM_MALLOC(channel_scan_req_tlv.target_radios[i].target_op_classes[j].target_channel_nr * sizeof(INT8U));
						PLATFORM_MEMCPY(channel_scan_req_tlv.target_radios[i].target_op_classes[j].target_channel_list, req->channel_scan_req.target_radios[i].target_op_classes[j].target_channel_list, channel_scan_req_tlv.target_radios[i].target_op_classes[j].target_channel_nr);
					}
				}
			}
		}

		if (NULL != tmp) {
			sendChannelScanRequestPacket(tmp->receiving_interface, getNextMid(), &channel_scan_req_tlv, req->target_al_mac);
		} else {
			char **backhaul_interfaces;
			INT8U  backhaul_interfaces_nr;
			backhaul_interfaces = (char **)get_backhaul_interfaces(&backhaul_interfaces_nr);
			for (i = 0; i < backhaul_interfaces_nr; i++) {
				sendChannelScanRequestPacket(backhaul_interfaces[i], getNextMid(), &channel_scan_req_tlv, req->target_al_mac);
			}
			PLATFORM_FREE(backhaul_interfaces);
		}

		for (i = 0; i < channel_scan_req_tlv.target_radio_nr; i++) {
			PLATFORM_FREE(channel_scan_req_tlv.target_radios[i].target_op_classes);
		}
		PLATFORM_FREE(channel_scan_req_tlv.target_radios);

		api_send_general_acknowledgement(*element, req->target_al_mac, ret);
		break;
	}
#endif
	case MAP_CLIENT_STEERING_REQUEST_ELEMENT: {
		struct map_client_steering_request *req = (struct map_client_steering_request *)element;
		struct networkDevice *              tmp = DMnetworkDeviceGet(req->target_al_mac);
		if (NULL != tmp) {
			sendMultiAPClientSteeringRequestPacket(tmp->receiving_interface, getNextMid(), req->target_al_mac, &req->steering_req);
		} else {
			char **backhaul_interfaces;
			INT8U  backhaul_interfaces_nr;
			backhaul_interfaces = (char **)get_backhaul_interfaces(&backhaul_interfaces_nr);
			for (i = 0; i < backhaul_interfaces_nr; i++) {
				sendMultiAPClientSteeringRequestPacket(backhaul_interfaces[i], getNextMid(), req->target_al_mac, &req->steering_req);
			}

			PLATFORM_FREE(backhaul_interfaces);
		}
		api_send_general_acknowledgement(*element, req->target_al_mac, ret);
		break;
	}
	case MAP_CLIENT_ASSOCIATION_CONTROL_REQUEST_ELEMENT: {
		struct map_client_association_control_request *req = (struct map_client_association_control_request *)element;

		struct networkDevice *tmp = DMnetworkDeviceGet(req->target_al_mac);
		if (NULL != tmp) {
			sendMultiAPClientAssociationControlRequestPacket(tmp->receiving_interface, getNextMid(), req->target_al_mac, req->client_assoc_ctrl_req, req->client_assoc_ctrl_req_nr);
		} else {
			char **backhaul_interfaces;
			INT8U  backhaul_interfaces_nr;
			backhaul_interfaces = (char **)get_backhaul_interfaces(&backhaul_interfaces_nr);
			for (i = 0; i < backhaul_interfaces_nr; i++) {
				sendMultiAPClientAssociationControlRequestPacket(backhaul_interfaces[i], getNextMid(), req->target_al_mac, req->client_assoc_ctrl_req, req->client_assoc_ctrl_req_nr);
			}
			PLATFORM_FREE(backhaul_interfaces);
		}
		api_send_general_acknowledgement(*element, req->target_al_mac, ret);
		break;
	}
	case MAP_BACKHAUL_STEERING_REQUEST_ELEMENT: {
		struct map_backhaul_steering_request *req = (struct map_backhaul_steering_request *)element;

		// /////Test Code/////
		// PLATFORM_PRINTF_ERROR("Element ID %02x\n", req->element_id);
		// PLATFORM_PRINTF_ERROR("Target AL Mac %02x %02x %02x %02x %02x %02x\n",
		// 	req->target_al_mac[0], req->target_al_mac[1], req->target_al_mac[2], req->target_al_mac[3], req->target_al_mac[4], req->target_al_mac[5]);
		// PLATFORM_PRINTF_ERROR("Backhaul STA Mac %02x %02x %02x %02x %02x %02x\n",
		// 	req->backhaul_sta_mac[0], req->backhaul_sta_mac[1], req->backhaul_sta_mac[2], req->backhaul_sta_mac[3], req->backhaul_sta_mac[4], req->backhaul_sta_mac[5]);
		// PLATFORM_PRINTF_ERROR("Target BSSID %02x %02x %02x %02x %02x %02x\n",
		// 	req->target_bssid[0], req->target_bssid[1], req->target_bssid[2], req->target_bssid[3], req->target_bssid[4], req->target_bssid[5]);
		// PLATFORM_PRINTF_ERROR("Op Class %02x\n", req->op_class);
		// PLATFORM_PRINTF_ERROR("Channel %02x\n", req->channel);
		// //////////////////

		struct networkDevice *tmp = DMnetworkDeviceGet(req->target_al_mac);
		if (NULL != tmp) {
			//if got received message from the target agent before
			sendBackhaulSteeringRequestPacket(tmp->receiving_interface,
			                                  getNextMid(),
			                                  req->target_al_mac,
			                                  req->backhaul_sta_mac,
			                                  req->target_bssid,
			                                  req->op_class,
			                                  req->channel);
			PLATFORM_PRINTF_INFO("BACKHAUL STEERING REQUEST Message Sent in ONE interface\n");
		} else {
			//else broadcast in all interfaces
			char **backhaul_interfaces;
			INT8U  backhaul_interfaces_nr;
			backhaul_interfaces = (char **)get_backhaul_interfaces(&backhaul_interfaces_nr);
			for (i = 0; i < backhaul_interfaces_nr; i++) {
				sendBackhaulSteeringRequestPacket(backhaul_interfaces[i],
				                                  getNextMid(),
				                                  req->target_al_mac,
				                                  req->backhaul_sta_mac,
				                                  req->target_bssid,
				                                  req->op_class,
				                                  req->channel);
			}
			PLATFORM_PRINTF_WARNING("BACKHAUL STEERING REQUEST Message Sent through ALL Backhaul interfaces\n");
			PLATFORM_FREE(backhaul_interfaces);
		}
		api_send_general_acknowledgement(*element, req->target_al_mac, ret);
		break;
	}

	case MAP_POLICY_CONFIG_REQUEST_ELEMENT: {
		struct map_policy_config_request *    req = (struct map_policy_config_request *)element;
		struct SteeringPolicyTLV              steerPolicyTLV                 = { 0 };
		struct MetricReportingPolicyTLV       metricReportPolicyTLV          = { 0 };
		struct Default8021QSettingsTLV        default8021QSettingsTLV        = { 0 };
		struct TrafficSeparationPolicyTLV     trafficSeparationPolicyTLV     = { 0 };
		INT8U                                 policy_inclusion, i;
		INT16U                                mid;
		mid = getNextMid();

		policy_inclusion = 0;

		if (req->steer_policy_inclusion) {
			policy_inclusion |= 1;
			steerPolicyTLV.tlv_type                = TLV_TYPE_STEERING_POLICY;
			steerPolicyTLV.local_disallowed_sta_nr = req->steer_policy_data.ls_sta_nr;
			if (steerPolicyTLV.local_disallowed_sta_nr > 0) {
				steerPolicyTLV.local_disallowed_sta_mac = PLATFORM_MALLOC(6 * steerPolicyTLV.local_disallowed_sta_nr);
				for (i = 0; i < steerPolicyTLV.local_disallowed_sta_nr; i++) {
					PLATFORM_MEMCPY(steerPolicyTLV.local_disallowed_sta_mac[i], req->steer_policy_data.ls_stas[i], 6);
				}
			}
			steerPolicyTLV.btm_disallowed_sta_nr = req->steer_policy_data.btm_sta_nr;
			if (steerPolicyTLV.btm_disallowed_sta_nr > 0) {
				steerPolicyTLV.btm_disallowed_sta_mac = PLATFORM_MALLOC(6 * steerPolicyTLV.btm_disallowed_sta_nr);
				for (i = 0; i < steerPolicyTLV.btm_disallowed_sta_nr; i++) {
					PLATFORM_MEMCPY(steerPolicyTLV.btm_disallowed_sta_mac[i], req->steer_policy_data.btm_stas[i], 6);
				}
			}
			steerPolicyTLV.radios_nr = req->steer_policy_data.radio_nr;
			if (steerPolicyTLV.radios_nr > 0) {
				steerPolicyTLV.policies = PLATFORM_MALLOC(sizeof(struct SteeringPolicy) * steerPolicyTLV.radios_nr);
				for (i = 0; i < steerPolicyTLV.radios_nr; i++) {
					PLATFORM_MEMCPY(steerPolicyTLV.policies[i].radio_unique_id, req->steer_policy_data.radio_steer_policy_data[i].radio_mac, 6);
					steerPolicyTLV.policies[i].policy                  = req->steer_policy_data.radio_steer_policy_data[i].policy;
					steerPolicyTLV.policies[i].channel_util_threshold  = req->steer_policy_data.radio_steer_policy_data[i].cu_threshold;
					steerPolicyTLV.policies[i].rcpi_steering_threshold = req->steer_policy_data.radio_steer_policy_data[i].rcpi_threshold;
				}
			}
		}

		if (req->metric_policy_inclusion) {
			policy_inclusion |= 2;
			metricReportPolicyTLV.tlv_type        = TLV_TYPE_METRIC_REPORT_POLICY;
			metricReportPolicyTLV.report_interval = req->metric_policy_data.report_interval;
			metricReportPolicyTLV.radios_nr       = req->metric_policy_data.radio_nr;
			if (metricReportPolicyTLV.radios_nr)
				metricReportPolicyTLV.policies        = PLATFORM_MALLOC(sizeof(struct MetricReportingPolicy) * metricReportPolicyTLV.radios_nr);
			for (i = 0; i < metricReportPolicyTLV.radios_nr; i++) {
				PLATFORM_MEMCPY(metricReportPolicyTLV.policies[i].radio_unique_id, req->metric_policy_data.radio_metric_policy_data[i].radio_mac, 6);
				metricReportPolicyTLV.policies[i].sta_rcpi_threshold                  = req->metric_policy_data.radio_metric_policy_data[i].rcpi_threshold;
				metricReportPolicyTLV.policies[i].sta_rcpi_hysteresis_margin_override = req->metric_policy_data.radio_metric_policy_data[i].rcpi_hysteresis;
				metricReportPolicyTLV.policies[i].ap_channel_utilization_threshold     = req->metric_policy_data.radio_metric_policy_data[i].cu_threshold;
				metricReportPolicyTLV.policies[i].policy                              = req->metric_policy_data.radio_metric_policy_data[i].inclusion_policy;
			}
		}

		if (req->default8021q_inclusion) {
			policy_inclusion |= 8;
			default8021QSettingsTLV.tlv_type        = TLV_TYPE_DEFAULT_802_1Q_SETTINGS;
			default8021QSettingsTLV.primary_vlan_id = req->default8021q_data.primary_vlan_id;
			default8021QSettingsTLV.default_pcp     = req->default8021q_data.default_pcp;
		}

		if (req->traffic_separation_inclusion) {
			policy_inclusion |= 16;
			trafficSeparationPolicyTLV.tlv_type  = TLV_TYPE_TRAFFIC_SEPARATION_POLICY;
			trafficSeparationPolicyTLV.ssid_nr   = req->traffic_separation_data.ssid_nr;

			if (trafficSeparationPolicyTLV.ssid_nr)
				trafficSeparationPolicyTLV.ssid_info = PLATFORM_MALLOC(sizeof(struct TrafficSeparationSsidInfo) * trafficSeparationPolicyTLV.ssid_nr);

			for (i = 0; i < trafficSeparationPolicyTLV.ssid_nr; i++) {
				trafficSeparationPolicyTLV.ssid_info[i].ssid_length = req->traffic_separation_data.ssid_info[i].ssid_length;
				trafficSeparationPolicyTLV.ssid_info[i].ssid_name   = PLATFORM_MALLOC(trafficSeparationPolicyTLV.ssid_info[i].ssid_length);
				PLATFORM_MEMCPY(trafficSeparationPolicyTLV.ssid_info[i].ssid_name, req->traffic_separation_data.ssid_info[i].ssid, trafficSeparationPolicyTLV.ssid_info[i].ssid_length);
				trafficSeparationPolicyTLV.ssid_info[i].vlan_id = req->traffic_separation_data.ssid_info[i].vlan_id;
			}
		}

		struct networkDevice *tmp = DMnetworkDeviceGet(req->target_al_mac);
		if (NULL != tmp) {
			if (0 == sendMultiAPPolicyConfigRequestPacket(tmp->receiving_interface, mid, req->target_al_mac, policy_inclusion, &steerPolicyTLV, &metricReportPolicyTLV, NULL, &default8021QSettingsTLV, &trafficSeparationPolicyTLV)) {
				PLATFORM_PRINTF_WARNING("[%s] Could not send policy config request message\n", tmp->receiving_interface);
				ret = PROCESS_CMDU_KO;
			}
		}

		if (req->steer_policy_inclusion) {
			if (steerPolicyTLV.local_disallowed_sta_nr > 0)
				PLATFORM_FREE(steerPolicyTLV.local_disallowed_sta_mac);
			if (steerPolicyTLV.btm_disallowed_sta_nr > 0)
				PLATFORM_FREE(steerPolicyTLV.btm_disallowed_sta_mac);
			if (steerPolicyTLV.radios_nr > 0)
				PLATFORM_FREE(steerPolicyTLV.policies);
		}

		if (req->metric_policy_inclusion) {
			if (req->metric_policy_data.radio_nr > 0)
				PLATFORM_FREE(metricReportPolicyTLV.policies);
		}

		if (req->traffic_separation_inclusion) {
			if (req->traffic_separation_data.ssid_nr) {
				for (i = 0; i < req->traffic_separation_data.ssid_nr; i++) {
					if (req->traffic_separation_data.ssid_info[i].ssid_length) {
						PLATFORM_FREE(trafficSeparationPolicyTLV.ssid_info[i].ssid_name);
					}
				}
				PLATFORM_FREE(trafficSeparationPolicyTLV.ssid_info);
			}
		}

		api_send_general_acknowledgement(*element, req->target_al_mac, ret);

		break;
	}
	case MAP_AP_CAPABILITY_QUERY_ELEMENT: {
		struct map_ap_capability_query *req = (struct map_ap_capability_query *)element;
		INT16U                          mid;
		mid = getNextMid();

		struct networkDevice *tmp = DMnetworkDeviceGet(req->target_al_mac);
		if (NULL != tmp) {
			if (0 == sendAPCapabilityQueryPacket(tmp->receiving_interface, mid, req->target_al_mac)) {
				PLATFORM_PRINTF_WARNING("[%s] Could not send ap capability query message\n", tmp->receiving_interface);
				ret = PROCESS_CMDU_KO;
			}
		}
		api_send_general_acknowledgement(*element, req->target_al_mac, ret);
		break;
	}
	case MAP_CLIENT_CAPABILITY_QUERY_ELEMENT: {
		struct map_client_capability_query *req = (struct map_client_capability_query *)element;
		INT16U                              mid;
		mid = getNextMid();

		struct networkDevice *tmp = DMnetworkDeviceGet(req->target_al_mac);
		if (NULL != tmp) {
			if (0 == sendClientCapabilityQueryPacket(tmp->receiving_interface, mid, req->target_al_mac, req->bssid, req->sta_mac)) {
				PLATFORM_PRINTF_WARNING("[%s] Could not send client capability query message\n", tmp->receiving_interface);
				ret = PROCESS_CMDU_KO;
			}
		}
		api_send_general_acknowledgement(*element, req->target_al_mac, ret);
		break;
	}
	case MAP_AP_METRIC_QUERY_ELEMENT: {
		struct map_ap_metric_query *req = (struct map_ap_metric_query *)element;
		INT16U                      mid;
		mid = getNextMid();

		struct networkDevice *tmp = DMnetworkDeviceGet(req->target_al_mac);
		if (NULL != tmp) {
			if (0 == sendAPMetricQueryPacket(tmp->receiving_interface, mid, req->target_al_mac, req->bssid_nr, req->bssid)) {
				PLATFORM_PRINTF_WARNING("[%s] Could not send ap metric query message\n", tmp->receiving_interface);
				ret = PROCESS_CMDU_KO;
			}
		}
		api_send_general_acknowledgement(*element, req->target_al_mac, ret);
		break;
	}

	case MAP_LINK_METRIC_QUERY_ELEMENT: {
		struct map_link_metric_query *req = (struct map_link_metric_query *)element;

		struct networkDevice *tmp = DMnetworkDeviceGet(req->target_al_mac);
		if (NULL != tmp) {
			//if got received message from the target agent before
			send1905MetricsQueryPacket(tmp->receiving_interface, getNextMid(), req->target_al_mac, req->destination, req->type, req->sta_mac);
			PLATFORM_PRINTF_INFO("LINK METRIC QUERY Message Sent in ONE interface\n");
		} else {
			//else broadcast in all interfaces
			char **backhaul_interfaces;
			INT8U  backhaul_interfaces_nr;
			backhaul_interfaces = (char **)get_backhaul_interfaces(&backhaul_interfaces_nr);
			for (i = 0; i < backhaul_interfaces_nr; i++) {
				send1905MetricsQueryPacket(backhaul_interfaces[i], getNextMid(), req->target_al_mac, req->destination, req->type, req->sta_mac);
			}
			PLATFORM_PRINTF_WARNING("LINK METRIC QUERY Message Sent through ALL Backhaul interfaces\n");
			PLATFORM_FREE(backhaul_interfaces);
		}

		api_send_general_acknowledgement(*element, req->target_al_mac, ret);
		break;
	}

	case MAP_ASSOC_STA_METRIC_QUERY_ELEMENT: {
		struct map_assoc_sta_metric_query *req = (struct map_assoc_sta_metric_query *)element;
		INT16U                             mid;
		mid = getNextMid();

		struct networkDevice *tmp = DMnetworkDeviceGet(req->target_al_mac);
		if (NULL != tmp) {
			if (0 == sendAssocSTALinkMetricQueryPacket(tmp->receiving_interface, mid, req->target_al_mac, req->sta_mac)) {
				PLATFORM_PRINTF_WARNING("[%s] Could not send assoc sta metric query message\n", tmp->receiving_interface);
				ret = PROCESS_CMDU_KO;
			}
		}
		api_send_general_acknowledgement(*element, req->target_al_mac, ret);
		break;
	}
	case MAP_UNASSOC_STA_METRIC_QUERY_ELEMENT: {
		struct map_unassoc_sta_metric_query *req = (struct map_unassoc_sta_metric_query *)element;
		INT16U                               mid;
		mid = getNextMid();

		struct networkDevice *tmp = DMnetworkDeviceGet(req->target_al_mac);
		if (NULL != tmp) {
			if (0 == sendUnassocSTALinkMetricQueryPacket(tmp->receiving_interface, mid, req->target_al_mac, req->op_class, req->channel_data_nr, (struct ChannelListInfo *)req->channel_data)) {
				PLATFORM_PRINTF_WARNING("[%s] Could not send unassoc sta metric query message\n", tmp->receiving_interface);
				ret = PROCESS_CMDU_KO;
			}
		}
		api_send_general_acknowledgement(*element, req->target_al_mac, ret);
		break;
	}
	case MAP_BEACON_METRIC_QUERY_ELEMENT: {
		struct map_beacon_metric_query *req = (struct map_beacon_metric_query *)element;
		struct BeaconMetricsQueryTLV    beacon_metric_tlv;
		INT16U                          mid;
		INT8U                           i;
		mid = getNextMid();

		PLATFORM_MEMSET(&beacon_metric_tlv, 0, sizeof(struct BeaconMetricsQueryTLV));

		PLATFORM_MEMCPY(beacon_metric_tlv.mac_address, req->beacon_metric_query.sta_mac, 6);
		beacon_metric_tlv.op_class       = req->beacon_metric_query.op_class;
		beacon_metric_tlv.channel_number = req->beacon_metric_query.channel_nr;
		PLATFORM_MEMCPY(beacon_metric_tlv.bssid, req->beacon_metric_query.bssid, 6);
		beacon_metric_tlv.report_detail = req->beacon_metric_query.report_detail;
		beacon_metric_tlv.ssid_len      = req->beacon_metric_query.ssid_len;
		if (beacon_metric_tlv.ssid_len > 0) {
			beacon_metric_tlv.ssid = PLATFORM_MALLOC(beacon_metric_tlv.ssid_len);
			PLATFORM_MEMCPY(beacon_metric_tlv.ssid, req->beacon_metric_query.ssid, beacon_metric_tlv.ssid_len);
		}
		beacon_metric_tlv.ap_channel_report_nr = req->beacon_metric_query.channel_report_nr;
		if (beacon_metric_tlv.ap_channel_report_nr > 0) {
			beacon_metric_tlv.ap_channel_report = PLATFORM_MALLOC(sizeof(struct ApChannelReport) * beacon_metric_tlv.ap_channel_report_nr);
			for (i = 0; i < beacon_metric_tlv.ap_channel_report_nr; i++) {
				beacon_metric_tlv.ap_channel_report[i].len      = req->beacon_metric_query.channel_report[i].len;
				beacon_metric_tlv.ap_channel_report[i].op_class = req->beacon_metric_query.channel_report[i].op_class;
				if ((beacon_metric_tlv.ap_channel_report[i].len - 1) > 0) {
					beacon_metric_tlv.ap_channel_report[i].channel_list = PLATFORM_MALLOC(beacon_metric_tlv.ap_channel_report[i].len - 1);
					PLATFORM_MEMCPY(beacon_metric_tlv.ap_channel_report[i].channel_list, req->beacon_metric_query.channel_report[i].channel_list, beacon_metric_tlv.ap_channel_report[i].len - 1);
				}
			}
		}
		beacon_metric_tlv.elementID_nr = req->beacon_metric_query.element_nr;
		if (beacon_metric_tlv.elementID_nr > 0) {
			beacon_metric_tlv.element_list = PLATFORM_MALLOC(beacon_metric_tlv.elementID_nr);
			PLATFORM_MEMCPY(beacon_metric_tlv.element_list, req->beacon_metric_query.element_list, 6);
		}

		struct networkDevice *tmp = DMnetworkDeviceGet(req->target_al_mac);
		if (NULL != tmp) {
			if (0 == sendBeaconMetricsQueryPacket(tmp->receiving_interface, mid, req->target_al_mac, &beacon_metric_tlv)) {
				PLATFORM_PRINTF_WARNING("[%s] Could not send beacon metric query message\n", tmp->receiving_interface);
				ret = PROCESS_CMDU_KO;
			}
		}

		if (beacon_metric_tlv.ssid_len > 0)
			PLATFORM_FREE(beacon_metric_tlv.ssid);
		if (beacon_metric_tlv.ap_channel_report_nr > 0) {
			for (i = 0; i < beacon_metric_tlv.ap_channel_report_nr; i++) {
				if ((beacon_metric_tlv.ap_channel_report[i].len - 1) > 0) {
					PLATFORM_FREE(beacon_metric_tlv.ap_channel_report[i].channel_list);
				}
			}
		}
		if (beacon_metric_tlv.elementID_nr > 0)
			PLATFORM_FREE(beacon_metric_tlv.element_list);
		if (beacon_metric_tlv.ap_channel_report_nr > 0)
			PLATFORM_FREE(beacon_metric_tlv.ap_channel_report);

		api_send_general_acknowledgement(*element, req->target_al_mac, ret);
		break;
	}
	case MAP_CONFIGURE_NOTIFICATION_ELEMENT: {
		PLATFORM_PRINTF_INFO("<--- MAP_CONFIGURE_NOTIFICATION_ELEMENT (API)\n");
		if (!DMisController()) {
			PLATFORM_PRINTF_ERROR("MAP_CONFIGURE_NOTIFICATION received on agent, ignore the request\n");
			break;
		}
		struct map_configure_notification *req = (struct map_configure_notification *)element;
		freeConfigData();

		struct radio_config_data *radio_config;
		radio_config = PLATFORM_MALLOC(req->config_nr * sizeof(struct radio_config_data));
		for (i = 0; i < req->config_nr; i++) {
			radio_config[i].radio_band     = req->config_data[i].radio_band;
			radio_config[i].bss_data_nr    = req->config_data[i].bss_data_nr;
			radio_config[i].bss_data       = req->config_data[i].bss_data;
			radio_config[i].vendor_data_nr = req->config_data[i].vendor_data_nr;
			radio_config[i].vendor_datas   = req->config_data[i].vendor_datas;
		}
		updateConfigData(req->config_nr, radio_config);
		PLATFORM_FREE(radio_config);
		break;
	}
	case MAP_OPERATING_CHANNEL_CHANGE_NOTIFICATION_ELEMENT: {
		INT8U *controller_mac                  = DMcontrollerMacGet();
		struct networkDevice *             tmp = DMnetworkDeviceGet(controller_mac);
		if (NULL != tmp) {
			sendMultiAPOperatingChannelReportPacket(tmp->receiving_interface, getNextMid(), controller_mac);
		}
		break;
	}
	case MAP_VENDOR_SPECIFIC_ELEMENT: {
		PLATFORM_PRINTF_INFO("<--- MAP_VENDOR_SPECIFIC_ELEMENT (API)\n");
		struct map_vendor_specific_message *req = (struct map_vendor_specific_message *)element;
		PLATFORM_PRINTF_TRACE("Target AL %02x %02x %02x %02x %02x %02x\n", req->target_al_mac[0], req->target_al_mac[1], req->target_al_mac[2],
		                     req->target_al_mac[3], req->target_al_mac[4], req->target_al_mac[5]);
		PLATFORM_PRINTF_TRACE("OUI: %02x %02x %02x\n", req->vendor_oui[0], req->vendor_oui[1], req->vendor_oui[2]);
		PLATFORM_PRINTF_TRACE("Relay %x\n", req->relay_indicator);
		PLATFORM_PRINTF_TRACE("Data length %04x\n", req->vendor_data_len);
		PLATFORM_PRINTF_TRACE("Data value ");
		int i = 0;
		for (i = 0; i < req->vendor_data_len; i++) {
			PLATFORM_PRINTF_TRACE("%02x", req->vendor_data[i]);
		}
		PLATFORM_PRINTF_TRACE("\n");

		struct vendorSpecificTLV vendor_tlv;
		vendor_tlv.tlv_type = TLV_TYPE_VENDOR_SPECIFIC;
		PLATFORM_MEMCPY(vendor_tlv.vendorOUI, req->vendor_oui, 3);
		vendor_tlv.m_nr = req->vendor_data_len;
		vendor_tlv.m    = PLATFORM_MALLOC(sizeof(INT8U) * req->vendor_data_len);
		PLATFORM_MEMCPY(vendor_tlv.m, req->vendor_data, req->vendor_data_len);
		INT8U multicast_addr[6] = MCAST_1905;

		if (0 == PLATFORM_MEMCMP(multicast_addr, req->target_al_mac, 6)) {
			char **backhaul_interfaces;
			INT8U  backhaul_interfaces_nr;
			backhaul_interfaces = (char **)get_backhaul_interfaces(&backhaul_interfaces_nr);
			for (i = 0; i < backhaul_interfaces_nr; i++) {
				send1905VendorSpecificMessage(backhaul_interfaces[i], getNextMid(), req->target_al_mac, req->relay_indicator, &vendor_tlv);
			}
			PLATFORM_FREE(backhaul_interfaces);
		} else {
			struct networkDevice *tmp = DMnetworkDeviceGet(req->target_al_mac);
			if (NULL != tmp) {
				send1905VendorSpecificMessage(tmp->receiving_interface, getNextMid(), req->target_al_mac, req->relay_indicator, &vendor_tlv);
			} else {
				PLATFORM_PRINTF_WARNING("Unknown address %02x:%02x:%02x:%02x:%02x:%02x, vendor specific message will not be sent.\n", req->target_al_mac[0], req->target_al_mac[1], req->target_al_mac[2],
				                        req->target_al_mac[3], req->target_al_mac[4], req->target_al_mac[5]);
				ret = INTERFACE_POWER_RESULT_KO;
			}
		}
		if (vendor_tlv.m) {
			PLATFORM_FREE(vendor_tlv.m);
		}
		api_send_general_acknowledgement(*element, req->target_al_mac, ret);
		break;
	}
#ifdef RTK_MULTI_AP_R2
	case MAP_VLAN_APPLY_NOTIFICATION_ELEMENT: {
		PLATFORM_PRINTF_INFO("<--- MAP_VLAN_APPLY_NOTIFICATION_ELEMENT\n");
		ret = PROCESS_CMDU_OK_MONITOR_VLAN;
		break;
	}
	case MAP_VLAN_CLEAR_NOTIFICATION_ELEMENT: {
		PLATFORM_PRINTF_INFO("<--- MAP_VLAN_CLEAR_NOTIFICATION_ELEMENT\n");
		ret = PROCESS_CMDU_OK_CLEAR_VLAN;
		break;
	}
#endif
	default: {
	} break;
	}

	return ret;
}
