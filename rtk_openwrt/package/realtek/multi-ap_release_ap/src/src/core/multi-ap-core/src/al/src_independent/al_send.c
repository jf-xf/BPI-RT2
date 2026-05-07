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

#include "platform.h"
#include "utils.h"

#include <stdarg.h> // va_list
                    // NOTE: This is part of the C standard, thus *all* platforms should have it
                    // available... and that's why this include can exist in this "platform
                    // independent" file
#include <stdio.h>

#include <unistd.h>

#include "al_send.h"
#include "al_recv.h"
#include "al_datamodel.h"
#include "al_utils.h"
#include "al_dpp.h"
#include "al_dpp_keystore.h"
#include "al_dpp_counters.h"
#include "al_wsc.h"
#include "al_buffer.h"

#include "al.h"

#include "1905_tlvs.h"
#include "1905_cmdus.h"
#include "1905_alme.h"
#include "1905_l2.h"
#include "lldp_tlvs.h"
#include "lldp_payload.h"

#include "platform_os.h"
#include "platform_interfaces.h"
#include "platform_alme_server.h"
#include "platform_crypto_aes.h"

#include "multi_ap_cmdus.h"
#include "multi_ap_cmdus_r2.h"
#include "multi_ap_cmdus_r3.h"

#include "api_response.h"

#include "global_settings.h"

#include "map_vendor_specific_data.h"

#include "packet_tools.h"

#define CAC_REQUIRED_TIME 300
#define CAC_OCCUPIED_TIME 3000

#define MAX_SEND_BSS_CONF_OBJ_NR 16
#define MAX_SEND_BSS_CONFIG_NR 8

#define START_CCE_INDICATION 1
#define STOP_CCE_INDICATION 0

////////////////////////////////////////////////////////////////////////////////
// Private functions and data
////////////////////////////////////////////////////////////////////////////////

INT8U _vendorSpecificTLVInsertInCMDU(struct CMDU *memory_structure, struct vendorSpecificTLV *vendor_specific)
{
	INT8U tlv_stop;

	if ((NULL == memory_structure) || (NULL == memory_structure->list_of_TLVs) || (NULL == vendor_specific) || (*(INT8U *)vendor_specific != TLV_TYPE_VENDOR_SPECIFIC)) {
		// Invalid params
		//
		return 0;
	}

	// Point at the end of the TLV list (NULL pointer)
	//
	tlv_stop = 0;
	while (memory_structure->list_of_TLVs[tlv_stop++]);
	tlv_stop--;

	// Insert TLV
	//
	memory_structure->list_of_TLVs             = (INT8U **)PLATFORM_REALLOC(memory_structure->list_of_TLVs, sizeof(INT8U *) * (tlv_stop + 2));
	memory_structure->list_of_TLVs[tlv_stop++] = (INT8U *)vendor_specific;
	memory_structure->list_of_TLVs[tlv_stop]   = NULL;

	return 1;
}

INT8U _sendToAllAgents(INT16U mid, struct CMDU *message)
{
	struct networkDevice *network_devices;
	INT8U                 network_devices_nr;
	INT8U                 ret = 1;
	INT8U                 i   = 0;

	DMallNetworkDevicesGet(&network_devices, &network_devices_nr);

	message->relay_indicator = 0;

	if (network_devices_nr > 1) {
		for (i = 1; i < network_devices_nr; i++) {
			PLATFORM_PRINTF_DEBUG("Send message to: %02x:%02x:%02x:%02x:%02x:%02x vis %s\n", network_devices[i].info->al_mac_address[0],
			                      network_devices[i].info->al_mac_address[1], network_devices[i].info->al_mac_address[2],
			                      network_devices[i].info->al_mac_address[3], network_devices[i].info->al_mac_address[4],
			                      network_devices[i].info->al_mac_address[5], network_devices[i].receiving_interface);
			if (0 == send1905RawPacket(network_devices[i].receiving_interface, mid, network_devices[i].info->al_mac_address, message, true, 0)) {
				PLATFORM_PRINTF_ERROR("Could not send the 1905 packet\n");
				ret = 0;
			}
		}
	}

	return ret;
}

//******************************************************************************
//******* Functions to fill (and free) TLVs with local data ********************
//******************************************************************************
//
// Note that not *all* types of TLVs have a corresponding function in this
// section. Only those that either:
//
//   a) Are called from more than one place.
//   b) In order to be filled, the local device/node needs to be queried.
//
// Acording to these rules, some of the TLVs that do *not* have a corresponding
// function in this section are, for example, all "power change" related TLVs,
// LLDP TLVS, etc... These will be manually "filled" and "freed" in the specific
// "send*()" function that makes use of them.

// Given a pointer to a preallocated "deviceInformationTypeTLV" structure, fill
// it with all the pertaining information retrieved from the local device.
//
void _obtainLocalDeviceInfoTLV(struct deviceInformationTypeTLV *device_info)
{
	INT8U al_mac_address[6];

	char **interfaces_names;
	INT8U  interfaces_names_nr;
	int    i;

	PLATFORM_MEMCPY(al_mac_address, DMalMacGet(), 6);

	device_info->tlv_type            = TLV_TYPE_DEVICE_INFORMATION_TYPE;
	device_info->al_mac_address[0]   = al_mac_address[0];
	device_info->al_mac_address[1]   = al_mac_address[1];
	device_info->al_mac_address[2]   = al_mac_address[2];
	device_info->al_mac_address[3]   = al_mac_address[3];
	device_info->al_mac_address[4]   = al_mac_address[4];
	device_info->al_mac_address[5]   = al_mac_address[5];
	device_info->local_interfaces_nr = 0;
	device_info->local_interfaces    = NULL;

	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_names_nr);
	// Add all interfaces that are *not* in "POWER OFF" mode
	//
	for (i = interfaces_names_nr - 1; i >= 0; i--) {
		struct interfaceInfo *x;

		if (0 == PLATFORM_IS_INTERFACE_UP(interfaces_names[i])) {
			// Ignore interfaces that are in "POWER OFF" mode (they will
			// be included in the "power off" TLV, later, on this same
			// CMDU)
			//
			continue;
		}

		if (NULL == (x = PLATFORM_GET_1905_INTERFACE_INFO(interfaces_names[i]))) {
			// Error retrieving information for this interface.
			// Ignore it.
			//
			continue;
		}

		if (0 == device_info->local_interfaces_nr) {
			device_info->local_interfaces = (struct _localInterfaceEntries *)PLATFORM_MALLOC(sizeof(struct _localInterfaceEntries));
		} else {
			device_info->local_interfaces = (struct _localInterfaceEntries *)PLATFORM_REALLOC(device_info->local_interfaces, sizeof(struct _localInterfaceEntries) * (device_info->local_interfaces_nr + 1));
		}

		device_info->local_interfaces[device_info->local_interfaces_nr].mac_address[0] = x->mac_address[0];
		device_info->local_interfaces[device_info->local_interfaces_nr].mac_address[1] = x->mac_address[1];
		device_info->local_interfaces[device_info->local_interfaces_nr].mac_address[2] = x->mac_address[2];
		device_info->local_interfaces[device_info->local_interfaces_nr].mac_address[3] = x->mac_address[3];
		device_info->local_interfaces[device_info->local_interfaces_nr].mac_address[4] = x->mac_address[4];
		device_info->local_interfaces[device_info->local_interfaces_nr].mac_address[5] = x->mac_address[5];
		device_info->local_interfaces[device_info->local_interfaces_nr].media_type     = x->interface_type;
		switch (x->interface_type) {
		case INTERFACE_TYPE_IEEE_802_11B_2_4_GHZ:
		case INTERFACE_TYPE_IEEE_802_11G_2_4_GHZ:
		case INTERFACE_TYPE_IEEE_802_11A_5_GHZ:
		case INTERFACE_TYPE_IEEE_802_11N_2_4_GHZ:
		case INTERFACE_TYPE_IEEE_802_11N_5_GHZ:
		case INTERFACE_TYPE_IEEE_802_11AC_5_GHZ:
		case INTERFACE_TYPE_IEEE_802_11AD_60_GHZ:
		case INTERFACE_TYPE_IEEE_802_11AF:
		//case INTERFACE_TYPE_IEEE_802_11AX:
		{
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data_size                                          = 10;
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data.ieee80211.network_membership[0]               = x->interface_type_data.ieee80211.bssid[0];
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data.ieee80211.network_membership[1]               = x->interface_type_data.ieee80211.bssid[1];
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data.ieee80211.network_membership[2]               = x->interface_type_data.ieee80211.bssid[2];
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data.ieee80211.network_membership[3]               = x->interface_type_data.ieee80211.bssid[3];
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data.ieee80211.network_membership[4]               = x->interface_type_data.ieee80211.bssid[4];
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data.ieee80211.network_membership[5]               = x->interface_type_data.ieee80211.bssid[5];
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data.ieee80211.role                                = x->interface_type_data.ieee80211.role;
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data.ieee80211.ap_channel_band                     = x->interface_type_data.ieee80211.ap_channel_band;
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data.ieee80211.ap_channel_center_frequency_index_1 = x->interface_type_data.ieee80211.ap_channel_center_frequency_index_1;
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data.ieee80211.ap_channel_center_frequency_index_2 = x->interface_type_data.ieee80211.ap_channel_center_frequency_index_2;
			break;
		}
		case INTERFACE_TYPE_IEEE_1901_FFT: {
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data_size                           = 7;
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data.ieee1901.network_identifier[0] = x->interface_type_data.ieee1901.network_identifier[0];
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data.ieee1901.network_identifier[1] = x->interface_type_data.ieee1901.network_identifier[1];
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data.ieee1901.network_identifier[2] = x->interface_type_data.ieee1901.network_identifier[2];
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data.ieee1901.network_identifier[3] = x->interface_type_data.ieee1901.network_identifier[3];
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data.ieee1901.network_identifier[4] = x->interface_type_data.ieee1901.network_identifier[4];
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data.ieee1901.network_identifier[5] = x->interface_type_data.ieee1901.network_identifier[5];
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data.ieee1901.network_identifier[6] = x->interface_type_data.ieee1901.network_identifier[6];
			break;
		}
		default: {
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data_size  = 0;
			device_info->local_interfaces[device_info->local_interfaces_nr].media_specific_data.dummy = 0;
			break;
		}
		}
		device_info->local_interfaces_nr++;

		PLATFORM_FREE_1905_INTERFACE_INFO(x);
	}

	PLATFORM_FREE_LIST_OF_1905_INTERFACES(interfaces_names, interfaces_names_nr);
}

// Free the contents of the provided "deviceInformationTypeTLV" structure (ie.
// only what was allocated by "_obtainLocalDeviceInfoTLV()", and not the
// "deviceInformationTypeTLV" structure itself, which is the caller's
// responsability)
//
void _freeLocalDeviceInfoTLV(struct deviceInformationTypeTLV *device_info)
{
	PLATFORM_FREE(device_info->local_interfaces);
}

// Given a pointer to a preallocated "deviceBridgingCapabilityTLV" structure,
// fill it with all the pertaining information retrieved from the local device.
//
void _obtainLocalBridgingCapabilitiesTLV(struct deviceBridgingCapabilityTLV *bridge_info)
{
	struct bridge *br;
	INT8U          br_nr;
	INT8U          i, j;

	bridge_info->tlv_type = TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES;

	if ((NULL == (br = PLATFORM_GET_LIST_OF_BRIDGES(&br_nr))) || 0 == br_nr) {
		// No bridge info
		//
		bridge_info->bridging_tuples_nr = 0;
		bridge_info->bridging_tuples    = NULL;
	} else {
		bridge_info->bridging_tuples_nr = br_nr;
		bridge_info->bridging_tuples    = (struct _bridgingTupleEntries *)PLATFORM_MALLOC(sizeof(struct _bridgingTupleEntries) * br_nr);

		for (i = 0; i < br_nr; i++) {
			bridge_info->bridging_tuples[i].bridging_tuple_macs_nr = br[i].bridged_interfaces_nr;

			if (0 == br[i].bridged_interfaces_nr) {
				bridge_info->bridging_tuples[i].bridging_tuple_macs = NULL;
			} else {
				bridge_info->bridging_tuples[i].bridging_tuple_macs = (struct _bridgingTupleMacEntries *)PLATFORM_MALLOC(sizeof(struct _bridgingTupleMacEntries) * br[i].bridged_interfaces_nr);

				for (j = 0; j < bridge_info->bridging_tuples[i].bridging_tuple_macs_nr; j++) {
					INT8U mac_address[6];

					PLATFORM_MEMCPY(mac_address, DMinterfaceNameToMac(br[i].bridged_interfaces[j]), 6);
					bridge_info->bridging_tuples[i].bridging_tuple_macs[j].mac_address[0] = mac_address[0];
					bridge_info->bridging_tuples[i].bridging_tuple_macs[j].mac_address[1] = mac_address[1];
					bridge_info->bridging_tuples[i].bridging_tuple_macs[j].mac_address[2] = mac_address[2];
					bridge_info->bridging_tuples[i].bridging_tuple_macs[j].mac_address[3] = mac_address[3];
					bridge_info->bridging_tuples[i].bridging_tuple_macs[j].mac_address[4] = mac_address[4];
					bridge_info->bridging_tuples[i].bridging_tuple_macs[j].mac_address[5] = mac_address[5];
				}
			}
		}
		PLATFORM_FREE_LIST_OF_BRIDGES(br, br_nr);
	}
}

// Free the contents of the provided "deviceBridgingCapabilityTLV" structure
// (ie.  only what was allocated by "_obtainLocalBridgingCapabilitiesTLV()",
// and not the "deviceBridgingCapabilityTLV" structure itself, which is the
// caller's responsability)
//
void _freeLocalBridgingCapabilitiesTLV(struct deviceBridgingCapabilityTLV *bridge_info)
{
	INT8U i;

	if (bridge_info->bridging_tuples_nr > 0) {
		for (i = 0; i < bridge_info->bridging_tuples_nr; i++) {
			if (bridge_info->bridging_tuples[i].bridging_tuple_macs_nr > 0) {
				PLATFORM_FREE(bridge_info->bridging_tuples[i].bridging_tuple_macs);
			}
		}
		PLATFORM_FREE(bridge_info->bridging_tuples);
	}
}

// Modify the provided pointers so that they now point to a list of pointers to
// "non1905NeighborDeviceListTLV" and "neighborDeviceListTLV" structures filled
// with all the pertaining information retrieved from the local device.
//
// Example: this is how you would use this function:
//
//   struct non1905NeighborDeviceListTLV **a;   INT8U a_nr;
//   struct neighborDeviceListTLV        **b;   INT8U b_nr;
//
//   _obtainLocalNeighborsTLV(&a, &a_nr, &b, &b_nr);
//
//   // a[0] -------> ptr to the first "non1905NeighborDeviceListTLV" structure
//   // a[1] -------> ptr to the second "non1905NeighborDeviceListTLV" structure
//   // ...
//   // a[a_nr-1] --> ptr to the last "non1905NeighborDeviceListTLV" structure
//
//   // b[0] -------> ptr to the first "neighborDeviceListTLV" structure
//   // b[1] -------> ptr to the second "neighborDeviceListTLV" structure
//   // ...
//   // b[b_nr-1] --> ptr to the last "neighborDeviceListTLV" structure
//
void _obtainLocalNeighborsTLV(struct non1905NeighborDeviceListTLV ***non_1905_neighbors, INT8U *non_1905_neighbors_nr, struct neighborDeviceListTLV ***neighbors, INT8U *neighbors_nr)
{
	char **interfaces_names;
	INT8U  interfaces_names_nr;
	INT8U  i, j, k;

	*non_1905_neighbors = NULL;
	*neighbors          = NULL;

	*non_1905_neighbors_nr = 0;
	*neighbors_nr          = 0;

	INT8U(*al_mac_addresses)[6]                       = NULL;
	INT8U al_mac_addresses_nr = 0;

	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_names_nr);

	for (i = 0; i < interfaces_names_nr; i++) {
		struct interfaceInfo *x;

		struct non1905NeighborDeviceListTLV *no;
		struct neighborDeviceListTLV *       yes;

		if (NULL == (x = PLATFORM_GET_1905_INTERFACE_INFO(interfaces_names[i]))) {
			PLATFORM_PRINTF_WARNING("Could not retrieve neighbors of interface %s\n", interfaces_names[i]);
			continue;
		}

		al_mac_addresses = DMgetListOfInterfaceNeighbors(interfaces_names[i], &al_mac_addresses_nr);

		no  = (struct non1905NeighborDeviceListTLV *)PLATFORM_MALLOC(sizeof(struct non1905NeighborDeviceListTLV));
		yes = (struct neighborDeviceListTLV *)PLATFORM_MALLOC(sizeof(struct neighborDeviceListTLV));

		no->tlv_type              = TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST;
		no->local_mac_address[0]  = x->mac_address[0];
		no->local_mac_address[1]  = x->mac_address[1];
		no->local_mac_address[2]  = x->mac_address[2];
		no->local_mac_address[3]  = x->mac_address[3];
		no->local_mac_address[4]  = x->mac_address[4];
		no->local_mac_address[5]  = x->mac_address[5];
		no->non_1905_neighbors_nr = 0;
		no->non_1905_neighbors    = NULL;

		yes->tlv_type             = TLV_TYPE_NEIGHBOR_DEVICE_LIST;
		yes->local_mac_address[0] = x->mac_address[0];
		yes->local_mac_address[1] = x->mac_address[1];
		yes->local_mac_address[2] = x->mac_address[2];
		yes->local_mac_address[3] = x->mac_address[3];
		yes->local_mac_address[4] = x->mac_address[4];
		yes->local_mac_address[5] = x->mac_address[5];
		yes->neighbors_nr         = 0;
		yes->neighbors            = NULL;

		// Decide if each neighbor is a 1905 or a non-1905 neighbor
		//
		if (x->neighbor_mac_addresses_nr != INTERFACE_NEIGHBORS_UNKNOWN) {
			INT8U *al_mac_address_has_been_reported = NULL;

			// Keep track of all the AL MACs that the interface reports he is
			// seeing.
			//
			if (0 != al_mac_addresses_nr) {
				// Originally, none of the neighbors in the data model has been
				// reported...
				//
				al_mac_address_has_been_reported = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * al_mac_addresses_nr);
				PLATFORM_MEMSET(al_mac_address_has_been_reported, 0x0, al_mac_addresses_nr);
			}

			for (j = 0; j < x->neighbor_mac_addresses_nr; j++) {
				INT8U *al_mac;
				INT8U  k;

				al_mac = DMmacToAlMac(x->neighbor_list[j].mac_address);

				if (NULL == al_mac && x->neighbor_list[j].multiap_profile) {
					send1905TopologyQueryPacket(interfaces_names[i], getNextMid(), x->neighbor_list[j].mac_address, 0);
					continue;
				}

				if (NULL == al_mac) {
					// Non-1905 neighbor

					INT8U already_added;

					// Make sure it has not already been added
					//
					already_added = 0;
					for (k = 0; k < no->non_1905_neighbors_nr; k++) {
						if (0 == PLATFORM_MEMCMP(x->neighbor_list[j].mac_address, no->non_1905_neighbors[k].mac_address, 6)) {
							already_added = 1;
							break;
						}
					}

					if (0 == already_added) {
						// This is a new neighbor
						//
						if (0 == no->non_1905_neighbors_nr) {
							no->non_1905_neighbors = (struct _non1905neighborEntries *)PLATFORM_MALLOC(sizeof(struct _non1905neighborEntries));
						} else {
							no->non_1905_neighbors = (struct _non1905neighborEntries *)PLATFORM_REALLOC(no->non_1905_neighbors, sizeof(struct _non1905neighborEntries) * (no->non_1905_neighbors_nr + 1));
						}

						no->non_1905_neighbors[no->non_1905_neighbors_nr].mac_address[0] = x->neighbor_list[j].mac_address[0];
						no->non_1905_neighbors[no->non_1905_neighbors_nr].mac_address[1] = x->neighbor_list[j].mac_address[1];
						no->non_1905_neighbors[no->non_1905_neighbors_nr].mac_address[2] = x->neighbor_list[j].mac_address[2];
						no->non_1905_neighbors[no->non_1905_neighbors_nr].mac_address[3] = x->neighbor_list[j].mac_address[3];
						no->non_1905_neighbors[no->non_1905_neighbors_nr].mac_address[4] = x->neighbor_list[j].mac_address[4];
						no->non_1905_neighbors[no->non_1905_neighbors_nr].mac_address[5] = x->neighbor_list[j].mac_address[5];

						no->non_1905_neighbors_nr++;
					}
				} else {
					// 1905 neighbor

					INT8U already_added;

					// Mark this AL MAC as reported
					//
					for (k = 0; k < al_mac_addresses_nr; k++) {
						if (0 == PLATFORM_MEMCMP(al_mac, al_mac_addresses[k], 6)) {
							al_mac_address_has_been_reported[k] = 1;
							break;
						}
					}

					// Make sure it has not already been added
					//
					already_added = 0;
					for (k = 0; k < yes->neighbors_nr; k++) {
						if (0 == PLATFORM_MEMCMP(al_mac, yes->neighbors[k].mac_address, 6)) {
							already_added = 1;
							break;
						}
					}

					if (0 == already_added) {
						// This is a new neighbor
						//
						if (0 == yes->neighbors_nr) {
							yes->neighbors = (struct _neighborEntries *)PLATFORM_MALLOC(sizeof(struct _neighborEntries));
						} else {
							yes->neighbors = (struct _neighborEntries *)PLATFORM_REALLOC(yes->neighbors, sizeof(struct _neighborEntries) * (yes->neighbors_nr + 1));
						}

						yes->neighbors[yes->neighbors_nr].mac_address[0] = al_mac[0];
						yes->neighbors[yes->neighbors_nr].mac_address[1] = al_mac[1];
						yes->neighbors[yes->neighbors_nr].mac_address[2] = al_mac[2];
						yes->neighbors[yes->neighbors_nr].mac_address[3] = al_mac[3];
						yes->neighbors[yes->neighbors_nr].mac_address[4] = al_mac[4];
						yes->neighbors[yes->neighbors_nr].mac_address[5] = al_mac[5];
						yes->neighbors[yes->neighbors_nr].bridge_flag    = DMisNeighborBridged(interfaces_names[i], al_mac);

						yes->neighbors_nr++;
					}

					PLATFORM_FREE(al_mac);
					if(yes->neighbors_nr > 200 && (DMgetMultiAPProfile() <= MULTI_AP_PROFILE_2)) {
						PLATFORM_PRINTF_WARNING("1905 neighbor>200 causes forge tlv fail\n");
						break;
					}
				}
			}
			PLATFORM_FREE_1905_INTERFACE_INFO(x);

			// Update the datamodel so that those neighbours whose MAC addresses
			// have not been reported are removed. (For 3.4.11E, reported means under sta_info)
			// This will speed up the "removal" of nodes.
			//
			INT8U is_controller_lost = 0;
			for (j = 0; j < al_mac_addresses_nr; j++) {
				if (0 == al_mac_address_has_been_reported[j]) {
					DMremoveALNeighborFromInterface(al_mac_addresses[j], interfaces_names[i]);
					DMrunGarbageCollector(&is_controller_lost);
					if (is_controller_lost) {
						INT8U **tlv_list;
						tlv_list    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *));
						tlv_list[0] = NULL;
						api_controller_change_notification(CONTROLLER_LOST, NULL, NULL);
						PLATFORM_FREE(tlv_list);
					}
				}
			}
			if (al_mac_addresses_nr > 0 && NULL != al_mac_address_has_been_reported) {
				PLATFORM_FREE(al_mac_address_has_been_reported);
			}
		} else {
			// The interface reports that it has no way of knowing which MAC
			// neighbors are connected to it.
			// In these cases, *at least* the already known 1905 neighbors
			// (which were discovered by us -not the platform- thanks to the
			// topology discovery process) should be returned.

			for (j = 0; j < al_mac_addresses_nr; j++) {
				if (controller_interface && controller_al_interface) {
					INT8U controller_al_mac[MACADDRLEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

					PLATFORM_GET_MAC(controller_al_interface, controller_al_mac);

					if (0 == PLATFORM_STRCMP(interfaces_names[i], controller_interface)) {
						// Skip any other neighbor besides controller_al_mac come from controller interface
						if (0 != PLATFORM_MEMCMP(controller_al_mac, al_mac_addresses[j], 6)) {
							PLATFORM_PRINTF_WARNING("Skip retrieve neighbor %02x:%02x:%02x:%02x:%02x:%02x of interface %s\n",
								al_mac_addresses[j][0], al_mac_addresses[j][1], al_mac_addresses[j][2],
								al_mac_addresses[j][3], al_mac_addresses[j][4], al_mac_addresses[j][5],
								interfaces_names[i]);
							continue;
						}
					}
					else {
						// Skip neighbor which al mac is the same as controller_al_mac come from other interfaces
						if (0 == PLATFORM_MEMCMP(controller_al_mac, al_mac_addresses[j], 6)) {
							PLATFORM_PRINTF_WARNING("Skip retrieve neighbor %02x:%02x:%02x:%02x:%02x:%02x "
								"(the same with controller_al_mac) of interface %s\n",
								al_mac_addresses[j][0], al_mac_addresses[j][1], al_mac_addresses[j][2],
								al_mac_addresses[j][3], al_mac_addresses[j][4], al_mac_addresses[j][5],
								interfaces_names[i]);
							continue;
						}
					}
				}

				INT8U already_added;

				// Make sure it has not already been added
				//
				already_added = 0;
				for (k = 0; k < yes->neighbors_nr; k++) {
					if (0 == PLATFORM_MEMCMP(al_mac_addresses[j], yes->neighbors[k].mac_address, 6)) {
						already_added = 1;
						break;
					}
				}

				if (0 == already_added) {
					// This is a new neighbor
					//
					if (0 == yes->neighbors_nr) {
						yes->neighbors = (struct _neighborEntries *)PLATFORM_MALLOC(sizeof(struct _neighborEntries));
					} else {
						yes->neighbors = (struct _neighborEntries *)PLATFORM_REALLOC(yes->neighbors, sizeof(struct _neighborEntries) * (yes->neighbors_nr + 1));
					}

					yes->neighbors[yes->neighbors_nr].mac_address[0] = al_mac_addresses[j][0];
					yes->neighbors[yes->neighbors_nr].mac_address[1] = al_mac_addresses[j][1];
					yes->neighbors[yes->neighbors_nr].mac_address[2] = al_mac_addresses[j][2];
					yes->neighbors[yes->neighbors_nr].mac_address[3] = al_mac_addresses[j][3];
					yes->neighbors[yes->neighbors_nr].mac_address[4] = al_mac_addresses[j][4];
					yes->neighbors[yes->neighbors_nr].mac_address[5] = al_mac_addresses[j][5];
					yes->neighbors[yes->neighbors_nr].bridge_flag    = DMisNeighborBridged(interfaces_names[i], al_mac_addresses[j]);

					yes->neighbors_nr++;
				}
				if(yes->neighbors_nr > 200 && (DMgetMultiAPProfile() <= MULTI_AP_PROFILE_2)) {
					PLATFORM_PRINTF_WARNING("1905 neighbor>200 causes forge tlv fail\n");
					break;
				}
			}

			if (IS_IEEE_802_3_INTERFACE(x->interface_type)) {
				INT8U                        neighbor_mac_addresses_nr = 0;
				struct ethernetNeighborInfo *neighbor_list             = NULL;
				INT8U                        k;
				int                          ignore_ports[32] = { -1 };
				int                          port_num         = 0;
				struct LocalInterface *p;
				char connected_1905_interface[32] = {0};

				p = DMnameToLocalInterfaceStruct(interfaces_names[i]);

				if(p && p->neighbors_nr > 0) { //search the connected eth port
					strlcpy(connected_1905_interface, interfaces_names[i], sizeof(connected_1905_interface));
					PLATFORM_PRINTF_DEBUG("\n[%s %d]the eth: %s is connected to 1905 dev.\n", __FUNCTION__, __LINE__, interfaces_names[i]);
				}

				PLATFORM_GET_ETHERNET_CLIENTS(&neighbor_mac_addresses_nr, &neighbor_list, connected_1905_interface);

				for (j = 0; j < neighbor_mac_addresses_nr; j++) {
					INT8U *al_mac = DMmacToAlMac(neighbor_list[j].mac_address);
					if (al_mac) {
						ignore_ports[port_num] = neighbor_list[j].port;
						port_num += 1;
						PLATFORM_PRINTF_DEBUG("\nDetected multi-ap device %02x:%02x:%02x:%02x:%02x:%02x under port %d, ignore all devices from this port\n", al_mac[0], al_mac[1], al_mac[2], al_mac[3], al_mac[4], al_mac[5], neighbor_list[j].port);
						PLATFORM_FREE(al_mac);
					}
				}

				for (j = 0; j < neighbor_mac_addresses_nr; j++) {
					// Non-1905 neighbor
					INT8U already_added;

					// Make sure it has not already been added
					//
					already_added = 0;

					for (k = 0; k < port_num; k++) {
						if (neighbor_list[j].port == ignore_ports[k]) {
							already_added = 1;
							break;
						}
					}

					if (!already_added) {
						for (k = 0; k < no->non_1905_neighbors_nr; k++) {
							if (0 == PLATFORM_MEMCMP(neighbor_list[j].mac_address, no->non_1905_neighbors[k].mac_address, 6)) {
								already_added = 1;
								break;
							}
						}
					}

					if (0 == already_added) {
						if(PLATFORM_STRSTR(interfaces_names[i],neighbor_list[j].ifName)){
							// This is a new neighbor
							//
							if (0 == no->non_1905_neighbors_nr) {
								no->non_1905_neighbors = (struct _non1905neighborEntries *)PLATFORM_MALLOC(sizeof(struct _non1905neighborEntries));
							} else {
								no->non_1905_neighbors = (struct _non1905neighborEntries *)PLATFORM_REALLOC(no->non_1905_neighbors, sizeof(struct _non1905neighborEntries) * (no->non_1905_neighbors_nr + 1));
							}

							PLATFORM_PRINTF_DEBUG("Adding new ethernet neighbour with MAC: %02x%02x%02x%02x%02x%02x\n",
								neighbor_list[j].mac_address[0],neighbor_list[j].mac_address[1],neighbor_list[j].mac_address[2],
								neighbor_list[j].mac_address[3],neighbor_list[j].mac_address[4],neighbor_list[j].mac_address[5]);

							no->non_1905_neighbors[no->non_1905_neighbors_nr].mac_address[0] = neighbor_list[j].mac_address[0];
							no->non_1905_neighbors[no->non_1905_neighbors_nr].mac_address[1] = neighbor_list[j].mac_address[1];
							no->non_1905_neighbors[no->non_1905_neighbors_nr].mac_address[2] = neighbor_list[j].mac_address[2];
							no->non_1905_neighbors[no->non_1905_neighbors_nr].mac_address[3] = neighbor_list[j].mac_address[3];
							no->non_1905_neighbors[no->non_1905_neighbors_nr].mac_address[4] = neighbor_list[j].mac_address[4];
							no->non_1905_neighbors[no->non_1905_neighbors_nr].mac_address[5] = neighbor_list[j].mac_address[5];

							no->non_1905_neighbors_nr++;
						}
					}
				}
				PLATFORM_FREE(neighbor_list);
			}
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
		}

		PLATFORM_FREE(al_mac_addresses);

		// At this point we have, for this particular interface, all the
		// non 1905 neighbors in "no" and all 1905 neighbors in "yes".
		// We just need to add "no" and "yes" to the "non_1905_neighbors"
		// and "neighbors" lists and proceed to the next interface.
		//
		if (no->non_1905_neighbors_nr > 0) {
			// Add this to the list of non-1905 neighbor TLVs
			//
			if (0 == *non_1905_neighbors_nr) {
				*non_1905_neighbors = (struct non1905NeighborDeviceListTLV **)PLATFORM_MALLOC(sizeof(struct non1905NeighborDeviceListTLV *));
			} else {
				*non_1905_neighbors = (struct non1905NeighborDeviceListTLV **)PLATFORM_REALLOC(*non_1905_neighbors, sizeof(struct non1905NeighborDeviceListTLV *) * (*non_1905_neighbors_nr + 1));
			}

			(*non_1905_neighbors)[*non_1905_neighbors_nr] = no;
			(*non_1905_neighbors_nr)++;
		} else {
			PLATFORM_FREE(no);
		}

		if (yes->neighbors_nr > 0) {
			//To pass logo test
			if (DMgetWFATestMode()) {
				if ((yes->neighbors_nr > 10) && (yes->neighbors_nr < 200)) {
					yes->neighbors = (struct _neighborEntries *)PLATFORM_REALLOC(yes->neighbors, sizeof(struct _neighborEntries) * 200);
					for (j = yes->neighbors_nr; j < 200; j++) {
						yes->neighbors[j].mac_address[0] = 0x84;
						yes->neighbors[j].mac_address[1] = 0xd3;
						yes->neighbors[j].mac_address[2] = 0x2a;
						yes->neighbors[j].mac_address[3] = 0x04;
						yes->neighbors[j].mac_address[4] = 0x03;
						yes->neighbors[j].mac_address[5] = j;
						yes->neighbors[j].bridge_flag    = 1;
					}
					yes->neighbors_nr = 200;
				}
			}
			// Add this to the list of non-1905 neighbor TLVs
			//
			if (0 == *neighbors_nr) {
				*neighbors = (struct neighborDeviceListTLV **)PLATFORM_MALLOC(sizeof(struct neighborDeviceListTLV *));
			} else {
				*neighbors = (struct neighborDeviceListTLV **)PLATFORM_REALLOC(*neighbors, sizeof(struct neighborDeviceListTLV *) * (*neighbors_nr + 1));
			}

			(*neighbors)[*neighbors_nr] = yes;
			(*neighbors_nr)++;
		} else {
			PLATFORM_FREE(yes);
		}
	}

	PLATFORM_FREE_LIST_OF_1905_INTERFACES(interfaces_names, interfaces_names_nr);
}

// Free the contents of the pointers filled by a previous call to
// "_obtainLocalNeighborsTLV()".
// This function is called with the same arguments as
// "_obtainLocalNeighborsTLV()"
//
void _freeLocalNeighborsTLV(struct non1905NeighborDeviceListTLV ***non_1905_neighbors, INT8U *non_1905_neighbors_nr, struct neighborDeviceListTLV ***neighbors, INT8U *neighbors_nr)
{
	INT8U i;

	if (*non_1905_neighbors_nr > 0) {
		for (i = 0; i < *non_1905_neighbors_nr; i++) {
			if ((*non_1905_neighbors)[i]->non_1905_neighbors_nr > 0) {
				PLATFORM_FREE((*non_1905_neighbors)[i]->non_1905_neighbors);
			}
			PLATFORM_FREE((*non_1905_neighbors)[i]);
		}
		PLATFORM_FREE(*non_1905_neighbors);
	}
	if (*neighbors_nr > 0) {
		for (i = 0; i < *neighbors_nr; i++) {
			if ((*neighbors)[i]->neighbors_nr > 0) {
				PLATFORM_FREE((*neighbors)[i]->neighbors);
			}
			PLATFORM_FREE((*neighbors)[i]);
		}
		PLATFORM_FREE(*neighbors);
	}
}

// Given a pointer to a preallocated "alMacAddressTypeTLV" structure, fill it
// with all the pertaining information retrieved from the local device.
//
void _obtainLocalAlMacAddressTLV(struct alMacAddressTypeTLV *al_mac_addr)
{
	INT8U al_mac_address[6];

	PLATFORM_MEMCPY(al_mac_address, DMalMacGet(), 6);

	al_mac_addr->tlv_type          = TLV_TYPE_AL_MAC_ADDRESS_TYPE;
	al_mac_addr->al_mac_address[0] = al_mac_address[0];
	al_mac_addr->al_mac_address[1] = al_mac_address[1];
	al_mac_addr->al_mac_address[2] = al_mac_address[2];
	al_mac_addr->al_mac_address[3] = al_mac_address[3];
	al_mac_addr->al_mac_address[4] = al_mac_address[4];
	al_mac_addr->al_mac_address[5] = al_mac_address[5];
}

void _obtainAlMacAddressTLV(struct alMacAddressTypeTLV *al_mac_addr_tlv, INT8U *dest_mac_addr)
{
	al_mac_addr_tlv->tlv_type = TLV_TYPE_AL_MAC_ADDRESS_TYPE;
	_EnB(&dest_mac_addr, al_mac_addr_tlv->al_mac_address, 6);
}

// Free the contents of the provided "alMacAddressTypeTLV" structure (ie.  only
// what was allocated by "_freeLocalAlMacAddressTLV()", and not the
// "alMacAddressTypeTLV" structure itself, which is the caller's responsability)
//
void _freeLocalAlMacAddressTLV(__attribute__((unused)) struct alMacAddressTypeTLV *al_mac_address)
{
	// Nothing needs to be freed
}

// Return a list of Tx metrics TLV and/or a list of Rx metrics TLV involving
// the local node and the neighbor whose AL MAC address matches
// 'specific_neighbor'.
//
// 'destination' can be either 'LINK_METRIC_QUERY_TLV_ALL_NEIGHBORS' (in which
// case argument 'specific_neighbor' is ignored) or
// 'LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR' (in which case 'specific_neighbor'
// is the AL MAC of the 1905 node at the other end of the link whose metrics
// are being reported.
//
// 'metrics_type' can be 'LINK_METRIC_QUERY_TLV_TX_LINK_METRICS_ONLY',
// 'LINK_METRIC_QUERY_TLV_RX_LINK_METRICS_ONLY' or
// 'LINK_METRIC_QUERY_TLV_BOTH_TX_AND_RX_LINK_METRICS'.
//
// 'tx' is an output argument which will be set to NULL if 'metrics_type' is
// set to 'LINK_METRIC_QUERY_TLV_RX_LINK_METRICS_ONLY' or point to a list of
// 1 or more (depending on the value of 'destination') pointers to Tx metrics
// TLVs.
//
// 'rx' is an output argument which will be set to NULL if 'metrics_type' is
// set to 'LINK_METRIC_QUERY_TLV_TX_LINK_METRICS_ONLY' or point to a list of
// 1 or more (depending on the value of 'destination') pointers to Rx metrics
// TLVs.
//
// 'nr' is an output argument set to the number of elements if rx and/or tx.
//
// If there is a problem (example: a specific neighbor was not found), this
// function returns '0', otherwise it returns '1'.
//
void _obtainLocalMetricsTLVs(INT8U destination, INT8U *specific_neighbor, INT8U metrics_type,
                             struct transmitterLinkMetricTLV ***tx,
                             struct receiverLinkMetricTLV ***   rx,
                             INT8U *                            nr)
{
	INT8U(*al_mac_addresses)[6]                       = NULL;
	INT8U al_mac_addresses_nr = 0;

	struct transmitterLinkMetricTLV **tx_tlvs;
	struct receiverLinkMetricTLV **   rx_tlvs;

	INT8U total_tlvs;
	INT8U i, j;

	al_mac_addresses = DMgetListOfNeighbors(&al_mac_addresses_nr);

	// We will need either 1 or 'al_mac_addresses_nr' Rx and/or Tx TLVs,
	// depending on the value of the 'destination' argument (ie. one Rx and/or
	// Tx TLV for each neighbor whose metrics we are going to report)
	//
	rx_tlvs = NULL;
	tx_tlvs = NULL;
	if (al_mac_addresses_nr > 0) {
		if (LINK_METRIC_QUERY_TLV_ALL_NEIGHBORS == destination) {
			if (
			    LINK_METRIC_QUERY_TLV_TX_LINK_METRICS_ONLY == metrics_type || LINK_METRIC_QUERY_TLV_BOTH_TX_AND_RX_LINK_METRICS == metrics_type) {
				tx_tlvs = (struct transmitterLinkMetricTLV **)PLATFORM_MALLOC(sizeof(struct transmitterLinkMetricTLV *) * al_mac_addresses_nr);
			}
			if (
			    LINK_METRIC_QUERY_TLV_RX_LINK_METRICS_ONLY == metrics_type || LINK_METRIC_QUERY_TLV_BOTH_TX_AND_RX_LINK_METRICS == metrics_type) {
				rx_tlvs = (struct receiverLinkMetricTLV **)PLATFORM_MALLOC(sizeof(struct receiverLinkMetricTLV *) * al_mac_addresses_nr);
			}
		} else {
			if (
			    LINK_METRIC_QUERY_TLV_TX_LINK_METRICS_ONLY == metrics_type || LINK_METRIC_QUERY_TLV_BOTH_TX_AND_RX_LINK_METRICS == metrics_type) {
				tx_tlvs = (struct transmitterLinkMetricTLV **)PLATFORM_MALLOC(sizeof(struct transmitterLinkMetricTLV *) * 1);
			}
			if (
			    LINK_METRIC_QUERY_TLV_RX_LINK_METRICS_ONLY == metrics_type || LINK_METRIC_QUERY_TLV_BOTH_TX_AND_RX_LINK_METRICS == metrics_type) {
				rx_tlvs = (struct receiverLinkMetricTLV **)PLATFORM_MALLOC(sizeof(struct receiverLinkMetricTLV *) * 1);
			}
		}
	}

	// Next, for each neighbor, fill the corresponding TLV structure (Rx, Tx or
	// both) that contains the information regarding all possible links that
	// "join" our local node with that neighbor.
	//
	total_tlvs = 0;
	for (i = 0; i < al_mac_addresses_nr; i++) {
		INT8U(*remote_macs)
		[6];
		char **local_interfaces;
		INT8U  links_nr;

		// Check if we are really interested in obtaining metrics information
		// regarding this particular neighbor
		//
		if (
		    LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR == destination && 0 != PLATFORM_MEMCMP(al_mac_addresses[i], specific_neighbor, 6)) {
			// Not interested
			//
			continue;
		}

		// Obtain the list of "links" that connect our AL node with this
		// specific neighbor.
		//
		remote_macs = DMgetListOfLinksWithNeighbor(al_mac_addresses[i], &local_interfaces, &links_nr);

		if (links_nr > 0) {

			// If there are 1 or more links between the local node and the
			// neighbor, first fill the TLV "header"
			//
			if (NULL != tx_tlvs) {
				tx_tlvs[total_tlvs] = (struct transmitterLinkMetricTLV *)PLATFORM_MALLOC(sizeof(struct transmitterLinkMetricTLV));

				tx_tlvs[total_tlvs]->tlv_type = TLV_TYPE_TRANSMITTER_LINK_METRIC;
				PLATFORM_MEMCPY(tx_tlvs[total_tlvs]->local_al_address, DMalMacGet(), 6);
				PLATFORM_MEMCPY(tx_tlvs[total_tlvs]->neighbor_al_address, al_mac_addresses[i], 6);
				tx_tlvs[total_tlvs]->transmitter_link_metrics_nr = links_nr;
				tx_tlvs[total_tlvs]->transmitter_link_metrics    = PLATFORM_MALLOC(sizeof(struct _transmitterLinkMetricEntries) * links_nr);
			}
			if (NULL != rx_tlvs) {
				rx_tlvs[total_tlvs] = (struct receiverLinkMetricTLV *)PLATFORM_MALLOC(sizeof(struct receiverLinkMetricTLV));

				rx_tlvs[total_tlvs]->tlv_type = TLV_TYPE_RECEIVER_LINK_METRIC;
				PLATFORM_MEMCPY(rx_tlvs[total_tlvs]->local_al_address, DMalMacGet(), 6);
				PLATFORM_MEMCPY(rx_tlvs[total_tlvs]->neighbor_al_address, al_mac_addresses[i], 6);
				rx_tlvs[total_tlvs]->receiver_link_metrics_nr = links_nr;
				rx_tlvs[total_tlvs]->receiver_link_metrics    = PLATFORM_MALLOC(sizeof(struct _receiverLinkMetricEntries) * links_nr);
			}

			// ...and then, for each link, fill the specific link information:
			//
			for (j = 0; j < links_nr; j++) {
				struct interfaceInfo *f;
				struct linkMetrics *  l;

				f = PLATFORM_GET_1905_INTERFACE_INFO(local_interfaces[j]);
				l = PLATFORM_GET_LINK_METRICS(local_interfaces[j], remote_macs[j], DMgetSTPState());

				if (NULL != tx_tlvs) {
					PLATFORM_MEMCPY(tx_tlvs[total_tlvs]->transmitter_link_metrics[j].local_interface_address, DMinterfaceNameToMac(local_interfaces[j]), 6);
					PLATFORM_MEMCPY(tx_tlvs[total_tlvs]->transmitter_link_metrics[j].neighbor_interface_address, remote_macs[j], 6);

					if (NULL == f) {
						tx_tlvs[total_tlvs]->transmitter_link_metrics[j].intf_type = MEDIA_TYPE_UNKNOWN;
					} else {
						tx_tlvs[total_tlvs]->transmitter_link_metrics[j].intf_type = f->interface_type;
					}
					tx_tlvs[total_tlvs]->transmitter_link_metrics[j].bridge_flag = DMisLinkBridged(local_interfaces[j], al_mac_addresses[i], remote_macs[j]);

					if (NULL == l) {
						tx_tlvs[total_tlvs]->transmitter_link_metrics[j].packet_errors           = 0;
						tx_tlvs[total_tlvs]->transmitter_link_metrics[j].transmitted_packets     = 0;
						tx_tlvs[total_tlvs]->transmitter_link_metrics[j].mac_throughput_capacity = 0;
						tx_tlvs[total_tlvs]->transmitter_link_metrics[j].link_availability       = 0;
						tx_tlvs[total_tlvs]->transmitter_link_metrics[j].phy_rate                = 0;
					} else {
						tx_tlvs[total_tlvs]->transmitter_link_metrics[j].packet_errors           = l->tx_packet_errors;
						tx_tlvs[total_tlvs]->transmitter_link_metrics[j].transmitted_packets     = l->tx_packet_ok;
						tx_tlvs[total_tlvs]->transmitter_link_metrics[j].mac_throughput_capacity = l->tx_max_xput;
						tx_tlvs[total_tlvs]->transmitter_link_metrics[j].link_availability       = l->tx_link_availability;
						tx_tlvs[total_tlvs]->transmitter_link_metrics[j].phy_rate                = l->tx_phy_rate;
					}
				}

				if (NULL != rx_tlvs) {
					PLATFORM_MEMCPY(rx_tlvs[total_tlvs]->receiver_link_metrics[j].local_interface_address, DMinterfaceNameToMac(local_interfaces[j]), 6);
					PLATFORM_MEMCPY(rx_tlvs[total_tlvs]->receiver_link_metrics[j].neighbor_interface_address, remote_macs[j], 6);

					if (NULL == f) {
						rx_tlvs[total_tlvs]->receiver_link_metrics[j].intf_type = MEDIA_TYPE_UNKNOWN;
					} else {
						rx_tlvs[total_tlvs]->receiver_link_metrics[j].intf_type = f->interface_type;
					}

					if (NULL == l) {
						rx_tlvs[total_tlvs]->receiver_link_metrics[j].packet_errors    = 0;
						rx_tlvs[total_tlvs]->receiver_link_metrics[j].packets_received = 0;
						rx_tlvs[total_tlvs]->receiver_link_metrics[j].rssi             = 0;
					} else {
						rx_tlvs[total_tlvs]->receiver_link_metrics[j].packet_errors    = l->rx_packet_errors;
						rx_tlvs[total_tlvs]->receiver_link_metrics[j].packets_received = l->rx_packet_ok;
						rx_tlvs[total_tlvs]->receiver_link_metrics[j].rssi             = l->rx_rssi; //mphx
					}
				}

				if (NULL != f) {
					PLATFORM_FREE_1905_INTERFACE_INFO(f);
				}
				if (NULL != l) {
					PLATFORM_FREE_LINK_METRICS(l);
				}
			}

			total_tlvs++;
		}

		DMfreeListOfLinksWithNeighbor(remote_macs, local_interfaces, links_nr);
	}

	PLATFORM_FREE(al_mac_addresses);

	if (
	    LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR == destination && total_tlvs == 0) {
		// Specific neighbor not found
		//
		*nr = 0;
		*tx = NULL;
		*rx = NULL;
	} else {
		*nr = total_tlvs;
		*tx = tx_tlvs;
		*rx = rx_tlvs;
	}
}

// Free the contents of the pointers filled by a previous call to
// "_obtainLocalMetricsTLVs()".
// This function is called with the same three last arguments as
// "_obtainLocalMetricsTLVs()"
//
void _freeLocalMetricsTLVs(struct transmitterLinkMetricTLV ***tx_tlvs, struct receiverLinkMetricTLV ***rx_tlvs, INT8U *total_tlvs)
{
	INT8U i;

	if (NULL != tx_tlvs && NULL != *tx_tlvs) {
		for (i = 0; i < *total_tlvs; i++) {
			if ((*tx_tlvs)[i]->transmitter_link_metrics_nr > 0 && NULL != (*tx_tlvs)[i]->transmitter_link_metrics) {
				PLATFORM_FREE((*tx_tlvs)[i]->transmitter_link_metrics);
			}
			PLATFORM_FREE((*tx_tlvs)[i]);
		}
		PLATFORM_FREE(*tx_tlvs);
	}
	if (NULL != rx_tlvs && NULL != *rx_tlvs) {
		for (i = 0; i < *total_tlvs; i++) {
			if ((*rx_tlvs)[i]->receiver_link_metrics_nr > 0 && NULL != (*rx_tlvs)[i]->receiver_link_metrics) {
				PLATFORM_FREE((*rx_tlvs)[i]->receiver_link_metrics);
			}
			PLATFORM_FREE((*rx_tlvs)[i]);
		}
		PLATFORM_FREE(*rx_tlvs);
	}
}

// This function is needed to present Tx and Rx TLVs in the way they are
// expected when contained inside an ALME-GET-METRIC.response reply.
//
// Let's explain this in more detail.
//
// Tx and Rx TLVs are "designed" to contain (each one of them) all possible
// links between two AL entities.
//
// In other words, if an AL has 3 neighbors, then 3 Rx (and 3 Tx) TLVs is all
// that is needed to contain all the information we might ever want.
//
// Example:
//
//   In this scenario, "AL 1" just has one neighbor ("AL 2") and thus only one
//   TLV is needed (even if there are *two* possible links)
//
//          eth0 ---------- eth3
//                    |
//     AL 1            -----eth2  AL 2
//
//          eth1 ---------- eth0
//
//   In particular, this unique TLV would be structured like this:
//
//     TLV:
//       - metrics AL1/eth0 <--> AL2/eth3
//       - metrics AL1/eth0 <--> AL2/eth2
//       - metrics AL1/eth1 <--> AL2/eth0
//
// However, when replying to an "ALME-GET-METRIC.response" message, for some
// reason, each Tx/Rx TLV in the list can only contain information for *one*
// local interface.
//
// In other words, in the example above, *even* if just one TLV has the
// capacity of providing information for both interfaces, we would have to
// create *two* TLVs instead, structured like this:
//
//   TLV1:
//     - metrics AL1/eth0 <--> AL2/eth3
//     - metrics AL1/eth0 <--> AL2/eth2
//
//   TLV2:
//     - metrics AL1/eth1 <--> AL2/eth0
//
// This is *obviously* an error in the standard (it causes more memory usage
// and repeated member structures that are not necessary)... but we have to live
// with it.
//
// Anyway... function "_obtainLocalMetricsTLVs()" returns a list of pointers to
// Tx/Rx TLVs where each element contains all the links between two neighbors.
// This function ("_reStructureMetricsTLVs()") takes that output and then
// returns a bigger list where each of the TLVs now only contains information
// regarding one local interface.
//
// The "restructured" returned pointers must be freed with a call to
// "_freeLocalMetricsTLVs()" when they are no longer needed
//
// Note: this function should always return "1". If it ever returns "0" it means
// there is a design error and the function implementation should be reviewed
//
INT8U _reStructureMetricsTLVs(struct transmitterLinkMetricTLV ***tx,
                              struct receiverLinkMetricTLV ***   rx,
                              INT8U *                            nr)
{
	struct transmitterLinkMetricTLV **tx_tlvs;
	struct receiverLinkMetricTLV **   rx_tlvs;

	struct transmitterLinkMetricTLV **new_tx_tlvs;
	struct receiverLinkMetricTLV **   new_rx_tlvs;

	INT8U total_tlvs;
	INT8U new_total_tlvs_rx;
	INT8U new_total_tlvs_tx;

	char **interfaces_names;
	INT8U  interfaces_names_nr;

	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_names_nr);

	INT8U i, j, k;

	tx_tlvs    = *tx;
	rx_tlvs    = *rx;
	total_tlvs = *nr;

	new_tx_tlvs       = NULL;
	new_rx_tlvs       = NULL;
	new_total_tlvs_rx = 0;
	new_total_tlvs_tx = 0;

	// For each neighbor
	//
	for (i = 0; i < total_tlvs; i++) {
		// Each "old" TLV (representing a neighbor) will "expand" into as many
		// "new" TLVs as local interfaces can be used to reach that neighbor.
		//
		if (NULL != tx_tlvs) {
			// For each local interface
			//
			for (j = 0; j < interfaces_names_nr; j++) {
				// ...find all TLV metrics associated to this local interface
				//
				for (k = 0; k < tx_tlvs[i]->transmitter_link_metrics_nr; k++) {
					if (0 == PLATFORM_MEMCMP(DMinterfaceNameToMac(interfaces_names[j]), tx_tlvs[i]->transmitter_link_metrics[k].local_interface_address, 6)) {
						// ...and add them
						//
						if (NULL == new_tx_tlvs) {
							// ...as NEW TLVs, if this is the first time
							//
							new_tx_tlvs    = (struct transmitterLinkMetricTLV **)PLATFORM_MALLOC(sizeof(struct transmitterLinkMetricTLV *));
							new_tx_tlvs[0] = (struct transmitterLinkMetricTLV *)PLATFORM_MALLOC(sizeof(struct transmitterLinkMetricTLV));

							new_tx_tlvs[0]->tlv_type = tx_tlvs[i]->tlv_type;
							PLATFORM_MEMCPY(new_tx_tlvs[0]->local_al_address, tx_tlvs[i]->local_al_address, 6);
							PLATFORM_MEMCPY(new_tx_tlvs[0]->neighbor_al_address, tx_tlvs[i]->neighbor_al_address, 6);
							new_tx_tlvs[0]->transmitter_link_metrics_nr = 1;
							new_tx_tlvs[0]->transmitter_link_metrics    = (struct _transmitterLinkMetricEntries *)PLATFORM_MALLOC(sizeof(struct _transmitterLinkMetricEntries));
							PLATFORM_MEMCPY(new_tx_tlvs[0]->transmitter_link_metrics, &(tx_tlvs[i]->transmitter_link_metrics[k]), sizeof(struct _transmitterLinkMetricEntries));

							new_total_tlvs_tx = 1;
						} else {
							// ...or as either NEW TLVs or part of a previously
							// created TLV which is also associated to this same
							// local interface.
							//
							if (
							    0 == PLATFORM_MEMCMP(new_tx_tlvs[new_total_tlvs_tx - 1]->transmitter_link_metrics[0].local_interface_address, tx_tlvs[i]->transmitter_link_metrics[k].local_interface_address, 6) && 0 == PLATFORM_MEMCMP(new_tx_tlvs[new_total_tlvs_tx - 1]->neighbor_al_address, tx_tlvs[i]->neighbor_al_address, 6)) {
								// Part of a previously created one. Append the
								// metrics info.
								//
								new_tx_tlvs[new_total_tlvs_tx - 1]->transmitter_link_metrics = (struct _transmitterLinkMetricEntries *)PLATFORM_REALLOC(new_tx_tlvs[new_total_tlvs_tx - 1]->transmitter_link_metrics, sizeof(struct _transmitterLinkMetricEntries) * (new_tx_tlvs[new_total_tlvs_tx - 1]->transmitter_link_metrics_nr + 1));
								PLATFORM_MEMCPY(&new_tx_tlvs[new_total_tlvs_tx - 1]->transmitter_link_metrics[new_tx_tlvs[new_total_tlvs_tx - 1]->transmitter_link_metrics_nr], &(tx_tlvs[i]->transmitter_link_metrics[k]), sizeof(struct _transmitterLinkMetricEntries));
								new_tx_tlvs[new_total_tlvs_tx - 1]->transmitter_link_metrics_nr++;
							} else {
								// New interface. Create new TLV.
								//
								new_tx_tlvs                    = (struct transmitterLinkMetricTLV **)PLATFORM_REALLOC(new_tx_tlvs, sizeof(struct transmitterLinkMetricTLV *) * (new_total_tlvs_tx + 1));
								new_tx_tlvs[new_total_tlvs_tx] = (struct transmitterLinkMetricTLV *)PLATFORM_MALLOC(sizeof(struct transmitterLinkMetricTLV));

								new_tx_tlvs[new_total_tlvs_tx]->tlv_type = tx_tlvs[i]->tlv_type;
								PLATFORM_MEMCPY(new_tx_tlvs[new_total_tlvs_tx]->local_al_address, tx_tlvs[i]->local_al_address, 6);
								PLATFORM_MEMCPY(new_tx_tlvs[new_total_tlvs_tx]->neighbor_al_address, tx_tlvs[i]->neighbor_al_address, 6);
								new_tx_tlvs[new_total_tlvs_tx]->transmitter_link_metrics_nr = 1;
								new_tx_tlvs[new_total_tlvs_tx]->transmitter_link_metrics    = (struct _transmitterLinkMetricEntries *)PLATFORM_MALLOC(sizeof(struct _transmitterLinkMetricEntries));
								PLATFORM_MEMCPY(new_tx_tlvs[new_total_tlvs_tx]->transmitter_link_metrics, &tx_tlvs[i]->transmitter_link_metrics[k], sizeof(struct _transmitterLinkMetricEntries));

								new_total_tlvs_tx++;
							}
						}
					}
				}
			}
		}

		// Repeat THE SAME for the "rx_tlvs" (this is "semi" copy & paste code,
		// because there are differences in the way structures and members are
		// called)
		//
		if (NULL != rx_tlvs) {
			// For each local interface
			//
			for (j = 0; j < interfaces_names_nr; j++) {
				// ...find all TLV metrics associated to this local interface
				//
				for (k = 0; k < rx_tlvs[i]->receiver_link_metrics_nr; k++) {
					if (0 == PLATFORM_MEMCMP(DMinterfaceNameToMac(interfaces_names[j]), rx_tlvs[i]->receiver_link_metrics[k].local_interface_address, 6)) {
						// ...and add them
						//
						if (NULL == new_rx_tlvs) {
							// ...as NEW TLVs, if this is the first time
							//
							new_rx_tlvs    = (struct receiverLinkMetricTLV **)PLATFORM_MALLOC(sizeof(struct receiverLinkMetricTLV *));
							new_rx_tlvs[0] = (struct receiverLinkMetricTLV *)PLATFORM_MALLOC(sizeof(struct receiverLinkMetricTLV));

							new_rx_tlvs[0]->tlv_type = rx_tlvs[i]->tlv_type;
							PLATFORM_MEMCPY(new_rx_tlvs[0]->local_al_address, rx_tlvs[i]->local_al_address, 6);
							PLATFORM_MEMCPY(new_rx_tlvs[0]->neighbor_al_address, rx_tlvs[i]->neighbor_al_address, 6);
							new_rx_tlvs[0]->receiver_link_metrics_nr = 1;
							new_rx_tlvs[0]->receiver_link_metrics    = (struct _receiverLinkMetricEntries *)PLATFORM_MALLOC(sizeof(struct _receiverLinkMetricEntries));
							PLATFORM_MEMCPY(new_rx_tlvs[0]->receiver_link_metrics, &rx_tlvs[i]->receiver_link_metrics[k], sizeof(struct _receiverLinkMetricEntries));

							new_total_tlvs_rx = 1;
						} else {
							// ...or as either NEW TLVs or part of a previously
							// created TLV which is also associated to this same
							// local interface.
							//
							if (
							    0 == PLATFORM_MEMCMP(new_rx_tlvs[new_total_tlvs_rx - 1]->receiver_link_metrics[0].local_interface_address, rx_tlvs[i]->receiver_link_metrics[k].local_interface_address, 6) && 0 == PLATFORM_MEMCMP(new_rx_tlvs[new_total_tlvs_rx - 1]->neighbor_al_address, rx_tlvs[i]->neighbor_al_address, 6)) {
								// Part of a previously created one. Append the
								// metrics info.
								//
								new_rx_tlvs[new_total_tlvs_rx - 1]->receiver_link_metrics = (struct _receiverLinkMetricEntries *)PLATFORM_REALLOC(new_rx_tlvs[new_total_tlvs_rx - 1]->receiver_link_metrics, sizeof(struct _receiverLinkMetricEntries) * (new_rx_tlvs[new_total_tlvs_rx - 1]->receiver_link_metrics_nr + 1));
								PLATFORM_MEMCPY(&new_rx_tlvs[new_total_tlvs_rx - 1]->receiver_link_metrics[new_rx_tlvs[new_total_tlvs_rx - 1]->receiver_link_metrics_nr], (&rx_tlvs[i]->receiver_link_metrics[k]), sizeof(struct _receiverLinkMetricEntries));
								new_rx_tlvs[new_total_tlvs_rx - 1]->receiver_link_metrics_nr++;
							} else {
								// New interface. Create new TLV.
								//
								new_rx_tlvs                    = (struct receiverLinkMetricTLV **)PLATFORM_REALLOC(new_rx_tlvs, sizeof(struct receiverLinkMetricTLV *) * (new_total_tlvs_rx + 1));
								new_rx_tlvs[new_total_tlvs_rx] = (struct receiverLinkMetricTLV *)PLATFORM_MALLOC(sizeof(struct receiverLinkMetricTLV));

								new_rx_tlvs[new_total_tlvs_rx]->tlv_type = rx_tlvs[i]->tlv_type;
								PLATFORM_MEMCPY(new_rx_tlvs[new_total_tlvs_rx]->local_al_address, rx_tlvs[i]->local_al_address, 6);
								PLATFORM_MEMCPY(new_rx_tlvs[new_total_tlvs_rx]->neighbor_al_address, rx_tlvs[i]->neighbor_al_address, 6);
								new_rx_tlvs[new_total_tlvs_rx]->receiver_link_metrics_nr = 1;
								new_rx_tlvs[new_total_tlvs_rx]->receiver_link_metrics    = (struct _receiverLinkMetricEntries *)PLATFORM_MALLOC(sizeof(struct _receiverLinkMetricEntries));
								PLATFORM_MEMCPY(new_rx_tlvs[new_total_tlvs_rx]->receiver_link_metrics, &rx_tlvs[i]->receiver_link_metrics[k], sizeof(struct _receiverLinkMetricEntries));

								new_total_tlvs_rx++;
							}
						}
					}
				}
			}
		}
	}

	PLATFORM_FREE_LIST_OF_1905_INTERFACES(interfaces_names, interfaces_names_nr);

	// Free the old structures
	//
	_freeLocalMetricsTLVs(&tx_tlvs, &rx_tlvs, &total_tlvs);

	if (new_total_tlvs_rx != new_total_tlvs_tx) {
		// Something went terribly wrong. This should NEVER happen
		//
		PLATFORM_PRINTF_ERROR("_reStructureMetricsTLVs contains a design error. Review it!\n");
		PLATFORM_FREE(new_rx_tlvs);
		PLATFORM_FREE(new_tx_tlvs);
		return 0;
	}

	// And return the new ones
	//
	*tx = new_tx_tlvs;
	*rx = new_rx_tlvs;
	*nr = new_total_tlvs_tx;

	return 1;
}

void _obtainSearchedServiceTLV(struct SearchedServiceTLV *t)
{
	t->tlv_type            = TLV_TYPE_SEARCHED_SERVICE;
	t->searched_service_nr = 1;
	t->searched_service    = (INT8U *)PLATFORM_MALLOC(t->searched_service_nr);
	t->searched_service[0] = MULTI_AP_CONTROLLER_SERVICE;
}

void _freeSearchedServiceTLV(struct SearchedServiceTLV *t)
{
	PLATFORM_FREE(t->searched_service);
}

void _obtainSupportedServiceTLV(struct SupportedServiceTLV *t)
{
	t->tlv_type = TLV_TYPE_SUPPORTED_SERVICE;

	if (DMisController()) {
		if (map_supported_service == 1) {
			t->supported_service_nr = 1;
			t->supported_service    = (INT8U *)PLATFORM_MALLOC(t->supported_service_nr);
			t->supported_service[0] = MULTI_AP_CONTROLLER_SERVICE;
		} else if (map_supported_service == 2) {
			t->supported_service_nr = 2;
			t->supported_service    = (INT8U *)PLATFORM_MALLOC(t->supported_service_nr);
			t->supported_service[0] = MULTI_AP_CONTROLLER_SERVICE;
			t->supported_service[1] = MULTI_AP_AGENT_SERVICE;
		}
	} else {
		t->supported_service_nr = 1;
		t->supported_service    = (INT8U *)PLATFORM_MALLOC(t->supported_service_nr);
		t->supported_service[0] = MULTI_AP_AGENT_SERVICE;
	}
}
void _freeSupportedServiceTLV(struct SupportedServiceTLV *t)
{
	PLATFORM_FREE(t->supported_service);
}

void _obtainApOperationalBSSTLV(struct ApOperationalBSSTLV *t)
{
	// create AP Operational BSS TLV
	//	t            = (struct ApOperationalBSSTLV *)PLATFORM_MALLOC(sizeof(struct ApOperationalBSSTLV));
	t->tlv_type  = TLV_TYPE_AP_OPERATIONAL_BSS;
	t->radios_nr = 0;
	t->radios    = NULL;

	INT8U radio_present_2g = 0, radio_present_5gl = 0, radio_present_5gh = 0;

	char **interfaces_names;
	INT8U  interfaces_nr, i;
	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_nr);

	struct RADIO wlan_2g_radio;
	struct RADIO wlan_5gl_radio;
	struct RADIO wlan_5gh_radio;
	wlan_5gh_radio.BSSs_nr = 0;
	wlan_5gl_radio.BSSs_nr = 0;
	wlan_2g_radio.BSSs_nr  = 0;
	wlan_5gh_radio.BSSs    = NULL;
	wlan_5gl_radio.BSSs    = NULL;
	wlan_2g_radio.BSSs     = NULL;
	struct RADIO *radio    = NULL;

	for (i = 0; i < interfaces_nr; i++) {
		struct interfaceInfo *x;

		x = PLATFORM_GET_1905_INTERFACE_INFO(interfaces_names[i]);
		if (NULL == x) {
			PLATFORM_PRINTF_ERROR("[MULTI_AP] Could not retrieve interface info for %s\n", interfaces_names[i]);
			continue;
		}
		// ignore the interface if it is not an AP
		if (!(IEEE80211_ROLE_AP == x->interface_type_data.ieee80211.role
			&& IS_IEEE_802_11_INTERFACE(x->interface_type))) {
			PLATFORM_PRINTF_DETAIL("[MULTI_AP] The interface %s is not operating a BSS.\n", interfaces_names[i]);
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
			continue;
		}
		// we can fill the TLV now
		// fill AP Operational BSS

		if (PLATFORM_STRSTR(interfaces_names[i], map_vxd_prefix)) {
			PLATFORM_PRINTF_DETAIL("[MULTI_AP] The interface %s is not operating a BSS.\n", interfaces_names[i]);
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
			continue;
		}

#if defined(_RTK_LINUX_)

		if (0 == PLATFORM_STRCMP(interfaces_names[i], map_radio_name_5gh)) {
			// this is the radio 5GH
			t->radios_nr += 1;
			radio = &wlan_5gh_radio;
			PLATFORM_MEMCPY(radio->radio_unique_id, x->mac_address, 6);
			radio_present_5gh = 1;
		} else if (PLATFORM_STRSTR(interfaces_names[i], map_radio_name_5gh) && PLATFORM_STRSTR(interfaces_names[i], map_vap_prefix)) {
			// these are the interfaces that belong to radio 5GH
			radio             = &wlan_5gh_radio;
		} else if (0 == PLATFORM_STRCMP(interfaces_names[i], map_radio_name_5gl)) {
			// this is the radio 5GL (Triband only)
			t->radios_nr += 1;
			radio = &wlan_5gl_radio;
			PLATFORM_MEMCPY(radio->radio_unique_id, x->mac_address, 6);
			radio_present_5gl = 1;
		} else if (PLATFORM_STRSTR(interfaces_names[i], map_radio_name_5gl) && PLATFORM_STRSTR(interfaces_names[i], map_vap_prefix)) {
			// these are the interfaces that belong to radio 5GL
			radio             = &wlan_5gl_radio;
		} else if (0 == PLATFORM_STRCMP(interfaces_names[i], map_radio_name_24g)) {
			// this is the radio 2G
			t->radios_nr += 1;
			radio = &wlan_2g_radio;
			PLATFORM_MEMCPY(radio->radio_unique_id, x->mac_address, 6);
			radio_present_2g = 1;
		} else if (PLATFORM_STRSTR(interfaces_names[i], map_radio_name_24g) && PLATFORM_STRSTR(interfaces_names[i], map_vap_prefix)) {
			// these are the interfaces that belong to radio 2G
			radio            = &wlan_2g_radio;
		} else {
			PLATFORM_PRINTF_WARNING("[RTK] Skip Unknown interface: %s", interfaces_names[i]);
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
			continue;
		}
		// Skip filling BSS if POWER_OFF
		if (INTERFACE_POWER_STATE_OFF == x->power_state) {
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
			continue;
		}
		radio->BSSs_nr += 1; // every interface in realtek sdk have one BSS only
		radio->BSSs = (struct BSS *)PLATFORM_REALLOC(radio->BSSs, radio->BSSs_nr * sizeof(struct BSS));
		PLATFORM_MEMCPY(radio->BSSs[radio->BSSs_nr - 1].mac_addr, x->mac_address, 6);
		radio->BSSs[radio->BSSs_nr - 1].ssid_len = PLATFORM_STRLEN(x->interface_type_data.ieee80211.ssid);
		radio->BSSs[radio->BSSs_nr - 1].ssid     = PLATFORM_MALLOC(radio->BSSs[radio->BSSs_nr - 1].ssid_len);
		PLATFORM_MEMCPY(radio->BSSs[radio->BSSs_nr - 1].ssid, x->interface_type_data.ieee80211.ssid, radio->BSSs[radio->BSSs_nr - 1].ssid_len);

#endif
		PLATFORM_FREE_1905_INTERFACE_INFO(x);
	}

	if (0 == t->radios_nr)
		return;

#if defined(_RTK_LINUX_)
	t->radios = (struct RADIO *)PLATFORM_REALLOC(t->radios, sizeof(struct RADIO) * t->radios_nr);
	i         = 0;
	if (radio_present_5gh) {
		t->radios[i].BSSs    = wlan_5gh_radio.BSSs;
		t->radios[i].BSSs_nr = wlan_5gh_radio.BSSs_nr;
		PLATFORM_MEMCPY(t->radios[i].radio_unique_id, wlan_5gh_radio.radio_unique_id, 6);
		i++;
	}
	if (radio_present_5gl) {
		t->radios[i].BSSs    = wlan_5gl_radio.BSSs;
		t->radios[i].BSSs_nr = wlan_5gl_radio.BSSs_nr;
		PLATFORM_MEMCPY(t->radios[i].radio_unique_id, wlan_5gl_radio.radio_unique_id, 6);
		i++;
	}
	if (radio_present_2g) {
		t->radios[i].BSSs    = wlan_2g_radio.BSSs;
		t->radios[i].BSSs_nr = wlan_2g_radio.BSSs_nr;
		PLATFORM_MEMCPY(t->radios[i].radio_unique_id, wlan_2g_radio.radio_unique_id, 6);
		i++;
	}
#endif
}
void _freeApOperationalBSSTLV(struct ApOperationalBSSTLV *t)
{
	struct ApOperationalBSSTLV *m = t;
	INT8U                       k1, m1;

	if (m->radios_nr > 0 && NULL != m->radios) {
		for (k1 = 0; k1 < m->radios_nr; k1++) {
			if (m->radios[k1].BSSs_nr > 0 && NULL != m->radios[k1].BSSs) {
				for (m1 = 0; m1 < m->radios[k1].BSSs_nr; m1++) {
					PLATFORM_FREE(m->radios[k1].BSSs[m1].ssid);
				}
				PLATFORM_FREE(m->radios[k1].BSSs);
			}
		}
		PLATFORM_FREE(m->radios);
	}
}
void _obtainAssociatedClientsTLV(struct AssociatedClientsTLV *t)
{
	t->tlv_type = TLV_TYPE_ASSOCIATED_CLIENTS;
	t->BSSs_nr  = 0;
	t->BSSs     = NULL;

	char **interfaces_names;
	INT8U  interfaces_nr, i;
	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_nr);

	for (i = 0; i < interfaces_nr; i++) {
		struct interfaceInfo *x;

		x = PLATFORM_GET_1905_INTERFACE_INFO(interfaces_names[i]);
		if (NULL == x) {
			PLATFORM_PRINTF_ERROR("Could not retrieve interface info for %s\n", interfaces_names[i]);
			continue;
		}

		//ignore vxd interface or power off interface
		if (PLATFORM_STRSTR(x->name, map_vxd_prefix) || x->power_state == INTERFACE_POWER_STATE_OFF) {
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
			continue;
		}

		// ignore the interface if it is not an AP
		if (!(IEEE80211_ROLE_AP == x->interface_type_data.ieee80211.role
			&& IS_IEEE_802_11_INTERFACE(x->interface_type))) {
			PLATFORM_PRINTF_DETAIL("The interface %s is not operating a BSS.\n", interfaces_names[i]);
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
			continue;
		}

		if (controller_interface && controller_al_interface) {
			// filter backhaul interfaces
			struct LocalInterface *local_interface = DMnameToLocalInterfaceStruct(interfaces_names[i]);
			if (local_interface) {
				if (local_interface->role & MULTI_AP_BACKHAUL_BSS) {
					PLATFORM_FREE_1905_INTERFACE_INFO(x);
					continue;
				}
			}
		}

		// we can fill the TLV now
		// fill Associated Clients TLV
		t->BSSs_nr += 1;
		t->BSSs = (struct BSSWithClients *)PLATFORM_REALLOC(t->BSSs, sizeof(struct BSSWithClients) * t->BSSs_nr);
		PLATFORM_MEMCPY(t->BSSs[t->BSSs_nr - 1].bssid, x->mac_address, 6);
		t->BSSs[t->BSSs_nr - 1].clients_nr = x->neighbor_mac_addresses_nr;
		// will not assign memory block if client number is zero
		if (t->BSSs[t->BSSs_nr - 1].clients_nr > 0) {
			t->BSSs[t->BSSs_nr - 1].clients = (struct Client *)PLATFORM_MALLOC(t->BSSs[t->BSSs_nr - 1].clients_nr * sizeof(struct Client));
			INT8U j;
			for (j = 0; j < t->BSSs[t->BSSs_nr - 1].clients_nr; j++) {
				PLATFORM_MEMCPY(t->BSSs[t->BSSs_nr - 1].clients[j].mac_addr, x->neighbor_list[j].mac_address, 6);
				if (x->neighbor_list[j].link_time < 0xFFFF) {
					t->BSSs[t->BSSs_nr - 1].clients[j].left_time = (INT16U)x->neighbor_list[j].link_time;
				} else {
					t->BSSs[t->BSSs_nr - 1].clients[j].left_time = 0xFFFF;
				}
			}
		}

		PLATFORM_FREE_1905_INTERFACE_INFO(x);
	}
}
void _freeAssociatedClientsTLV(struct AssociatedClientsTLV *t)
{
	struct AssociatedClientsTLV *m = t;
	INT8U                        k1;

	if (m->BSSs_nr > 0 && NULL != m->BSSs) {
		for (k1 = 0; k1 < m->BSSs_nr; k1++) {
			if (m->BSSs[k1].clients_nr > 0 && NULL != m->BSSs[k1].clients) {
				PLATFORM_FREE(m->BSSs[k1].clients);
			}
		}
		PLATFORM_FREE(m->BSSs);
	}
}

void _freeVendorSpecificTLV(struct vendorSpecificTLV *t)
{
	if (t->m) {
		PLATFORM_FREE(t->m);
	}
}

void _obtainAPCapabilityTLV(struct APCapabilityTLV *t)
{
	INT8U tempbuf[128];
	INT8U i              = 0;
	char *root_interface = map_radio_name_5gh;

	t->tlv_type      = TLV_TYPE_AP_CAPABILITY;
	t->ap_capability = 0;

	i = PLATFORM_AP_CAPABILITY_IOCTL(root_interface, 0, tempbuf);
	if (i > 0)
		t->ap_capability = tempbuf[2];
}

INT8U _isChannelInArray(INT8U channel, const INT8U channel_array[], const INT8U channel_array_size)
{
	int i;
	for (i = 0; i < channel_array_size; i++) {
		if (channel_array[i] == channel || channel_array[i] == get_channel_center_frequency_index(channel) || channel == get_channel_center_frequency_index(channel_array[i])
			|| channel_array[i] == get_160m_channel_center_frequency_index(channel) || channel == get_160m_channel_center_frequency_index(channel_array[i]))
			return 1;
	}
	return 0;
}

void _obtainAPRadioBasicCapabilitiesTLV(struct APRadioBasicCapabilitiesTLV *t, char *interface_name)
{
	struct interfaceInfo *x;

	int   band = 0, sub_band = 0;
	INT8U max_tx_power = 0;

	INT8U channel_nr    = 0;
	INT8U channels[128] = { 0 };
	INT8U filter        = FILTER_NONE;

	INT8U non_operable_channels_nr  = 0;
	INT8U non_operable_channels[32] = { 0 };
	INT8U disable_mac[6] = { 0x00, 0xe0, 0x4c, 0x81, 0x86, 0x86 };

	int       i, j, op_classes_len = 0;
	OP_CLASS *op_classes = PLATFORM_GET_OP_CLASS(&op_classes_len);

	t->tlv_type             = TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES;
	t->operating_classes    = NULL;
	t->operating_classes_nr = 0;

	//initialize radio_unique_id with disable_mac to avoid getting random value when interface does not exist(usdk g6).
	PLATFORM_MEMCPY(t->radio_unique_id, disable_mac, 6);

	if (NULL == op_classes) {
		PLATFORM_PRINTF_ERROR("[RTK] No operating class is available\n");
		return;
	}

	x = PLATFORM_GET_1905_INTERFACE_INFO(interface_name);
	if (NULL == x) {
		PLATFORM_PRINTF_ERROR("[MULTI_AP] Could not retrieve interface info for %s\n", interface_name);
		PLATFORM_FREE(op_classes);
		return;
	}

	// ignore the interface if it is not an AP
	if (!(IEEE80211_ROLE_AP == x->interface_type_data.ieee80211.role
		&& IS_IEEE_802_11_INTERFACE(x->interface_type))) {
		PLATFORM_PRINTF_DEBUG("[MULTI_AP] The interface %s is not operating a BSS.\n", interface_name);
		PLATFORM_FREE_1905_INTERFACE_INFO(x);
		PLATFORM_FREE(op_classes);
		return;
	}
	// start to fill the TLV
	PLATFORM_MEMCPY(t->radio_unique_id, x->mac_address, 6);
	PLATFORM_FREE_1905_INTERFACE_INFO(x);

	if (PLATFORM_STRSTR(interface_name, map_radio_name_5gl) && PLATFORM_STRSTR(interface_name, map_radio_name_5gh)) {
		// radio 5G
		band = _5G;
		if (0 != PLATFORM_GET_MIB(map_radio_name_5gl, "max_tx_power", &max_tx_power, 1)) {
			PLATFORM_PRINTF_ERROR("[RTK] [%s] can not get max_tx_power for 5G interface %s\n", __FUNCTION__, interface_name);
		}
		PLATFORM_PRINTF_DEBUG("[RTK] [%s] max_tx_power: %d\n", interface_name, max_tx_power);
	} else if (PLATFORM_STRSTR(interface_name, map_radio_name_5gh)) {
		band     = _5G;
		sub_band = _5GH;
		filter   = FILTER_5GH;
		if (0 != PLATFORM_GET_MIB(map_radio_name_5gh, "max_tx_power", &max_tx_power, 1)) {
			PLATFORM_PRINTF_ERROR("[RTK] [%s] can not get max_tx_power for 5GH interface %s\n", __FUNCTION__, interface_name);
		}
		PLATFORM_PRINTF_DEBUG("[RTK] [%s] max_tx_power: %d\n", interface_name, max_tx_power);
	} else if (PLATFORM_STRSTR(interface_name, map_radio_name_5gl)) {
		band     = _5G;
		sub_band = _5GL;
		filter   = FILTER_5GL;
		if (0 != PLATFORM_GET_MIB(map_radio_name_5gl, "max_tx_power", &max_tx_power, 1)) {
			PLATFORM_PRINTF_ERROR("[RTK] [%s] can not get max_tx_power for 5GL interface %s\n", __FUNCTION__, interface_name);
		}
		PLATFORM_PRINTF_DEBUG("[RTK] [%s] max_tx_power: %d\n", interface_name, max_tx_power);
	} else if (PLATFORM_STRSTR(interface_name, map_radio_name_24g)) {
		// radio 2.4G
		band = _2G;
		if (0 != PLATFORM_GET_MIB(map_radio_name_24g, "max_tx_power", &max_tx_power, 1)) {
			PLATFORM_PRINTF_ERROR("[RTK] [%s] can not get max_tx_power for 2G interface %s\n", __FUNCTION__, interface_name);
		}
		PLATFORM_PRINTF_DEBUG("[RTK] [%s] max_tx_power: %d\n", interface_name, max_tx_power);
	} else {
		PLATFORM_PRINTF_ERROR("[RTK] unknown interface: %s\n", interface_name);
		PLATFORM_FREE(op_classes);
		return;
	}

	t->max_BSSs_nr = config.max_bss_number;

	channel_nr = PLATFORM_GET_AVAILABLE_CHANNELS(interface_name, channels, (INT16U)sizeof(channels), filter);

	for (i = 0; i < op_classes_len; i++) {

		non_operable_channels_nr = 0;

		if (band == op_classes[i].band && (sub_band == op_classes[i].sub_band || 0 == sub_band || _5GF == op_classes[i].sub_band)) {
			t->operating_classes_nr += 1;
			t->operating_classes                                                       = (struct OperatingClass *)PLATFORM_REALLOC(t->operating_classes, sizeof(struct OperatingClass) * t->operating_classes_nr);
			t->operating_classes[t->operating_classes_nr - 1].operating_class          = op_classes[i].op_class;
			t->operating_classes[t->operating_classes_nr - 1].max_transmit_power       = max_tx_power;
			t->operating_classes[t->operating_classes_nr - 1].non_operable_channels_nr = 0;
			t->operating_classes[t->operating_classes_nr - 1].non_operable_channels    = NULL;

			for (j = 1; j < op_classes[i].channel_array[0]; j++) {
				if (0 == _isChannelInArray(op_classes[i].channel_array[j], channels, channel_nr)) {
					non_operable_channels[non_operable_channels_nr] = op_classes[i].channel_array[j];
					non_operable_channels_nr += 1;
				}
			}
			if (non_operable_channels_nr) {
				t->operating_classes[t->operating_classes_nr - 1].non_operable_channels_nr = non_operable_channels_nr;
				t->operating_classes[t->operating_classes_nr - 1].non_operable_channels    = PLATFORM_MALLOC(sizeof(INT8U) * non_operable_channels_nr);
				PLATFORM_MEMCPY(t->operating_classes[t->operating_classes_nr - 1].non_operable_channels, non_operable_channels, non_operable_channels_nr);
			}
		}
	}

	PLATFORM_FREE(op_classes);
}

void _freeAPRadioBasicCapabilitiesTLV(struct APRadioBasicCapabilitiesTLV *t)
{
	int i = 0;
	if (t->operating_classes_nr > 0 && t->operating_classes) {
		for (i = 0; i < t->operating_classes_nr; i++) {
			PLATFORM_FREE(t->operating_classes[i].non_operable_channels);
		}
		PLATFORM_FREE(t->operating_classes);
	}
}

int _obtainAPHTCapabilityTLV(struct APHTCapabilitiesTLV *t, char *interface_name)
{
	INT8U tempbuf[128];
	INT8U i, len;

	t->tlv_type = TLV_TYPE_AP_HT_CAPABILITIES;
	PLATFORM_MEMSET(t->radio_unique_id, 0, 6);
	t->ht_capability = 0;

	//i = rtk_linux_get_ap_capability_info(interface_name, 1, &tempbuf);
	i   = PLATFORM_AP_CAPABILITY_IOCTL(interface_name, 1, tempbuf);
	len = tempbuf[1];
	if (i > 0 && len > 0) {
		PLATFORM_MEMCPY(t->radio_unique_id, &tempbuf[2], MACADDRLEN);
		t->ht_capability = tempbuf[8];
		PLATFORM_PRINTF_DEBUG("%s MAC: %02x%02x%02x%02x%02x%02x\n", __FUNCTION__,
		                      t->radio_unique_id[0], t->radio_unique_id[1], t->radio_unique_id[2],
		                      t->radio_unique_id[3], t->radio_unique_id[4], t->radio_unique_id[5]);
		PLATFORM_PRINTF_DEBUG("%s HT capability: %d\n", __FUNCTION__, t->ht_capability);
		return 1;
	}

	return 0;
}

int _obtainAPVHTCapabilityTLV(struct APVHTCapabilitiesTLV *t, char *interface_name)
{
	INT8U tempbuf[128];
	INT8U i, len;

	t->tlv_type = TLV_TYPE_AP_VHT_CAPABILITIES;
	PLATFORM_MEMSET(t->radio_unique_id, 0, 6);
	t->vht_tx_msc       = 0;
	t->vht_rx_msc       = 0;
	t->vht_capability_1 = 0;
	t->vht_capability_2 = 0;

	//i = rtk_linux_get_ap_capability_info(interface_name, 2, &tempbuf);
	i = PLATFORM_AP_CAPABILITY_IOCTL(interface_name, 2, tempbuf);

	len = tempbuf[1];
	if (i > 0 && len > 0) {
		PLATFORM_MEMCPY(t->radio_unique_id, &tempbuf[2], MACADDRLEN);
		PLATFORM_MEMCPY(&t->vht_tx_msc, &tempbuf[8], 2);
		PLATFORM_MEMCPY(&t->vht_rx_msc, &tempbuf[10], 2);
		t->vht_capability_1 = tempbuf[12];
		t->vht_capability_2 = tempbuf[13];
		PLATFORM_PRINTF_DEBUG("%s MAC: %02x%02x%02x%02x%02x%02x\n", __FUNCTION__,
		                      t->radio_unique_id[0], t->radio_unique_id[1], t->radio_unique_id[2],
		                      t->radio_unique_id[3], t->radio_unique_id[4], t->radio_unique_id[5]);
		PLATFORM_PRINTF_DEBUG("%s vht tx msc: %d\n", __FUNCTION__, t->vht_tx_msc);
		PLATFORM_PRINTF_DEBUG("%s vht rx msc: %d\n", __FUNCTION__, t->vht_rx_msc);
		PLATFORM_PRINTF_DEBUG("%s vht cap 1: %d\n", __FUNCTION__, t->vht_capability_1);
		PLATFORM_PRINTF_DEBUG("%s vht cap 2: %d\n", __FUNCTION__, t->vht_capability_2);
		return 1;
	}
	return 0;
}

int _obtainAPHECapabilityTLV(struct APHECapabilitiesTLV *t, char *interface_name)
{
	INT8U tempbuf[128];
	INT8U i, len;

	t->tlv_type = TLV_TYPE_AP_HE_CAPABILITIES;
	PLATFORM_MEMSET(t->radio_unique_id, 0, 6);
	t->he_msc_len      = 0;
	t->he_msc          = NULL;
	t->he_capability_1 = 0;
	t->he_capability_2 = 0;

	if (PLATFORM_INTERFACE_HAS_AX_SUPPORT(interface_name) == 0) {
		PLATFORM_PRINTF_DEBUG("[PLATFORM] NOT obtain AP HE CAPABILITY in WIFI5\n");
		return 0;

	}
	//i = rtk_linux_get_ap_capability_info(interface_name, 2, &tempbuf);
	i = PLATFORM_AP_CAPABILITY_IOCTL(interface_name, 3, tempbuf);

	len = tempbuf[1];
	if (i > 0 && len > 0) {
		PLATFORM_MEMCPY(t->radio_unique_id, &tempbuf[2], MACADDRLEN);
		t->he_msc_len = tempbuf[8];

		t->he_msc = PLATFORM_MALLOC(t->he_msc_len);
		PLATFORM_MEMSET(t->he_msc, 0, t->he_msc_len);
		PLATFORM_MEMCPY(t->he_msc, &tempbuf[9], 4);

		t->he_capability_1 = tempbuf[13];
		t->he_capability_2 = tempbuf[14];

		PLATFORM_PRINTF_DEBUG("%s MAC: %02x%02x%02x%02x%02x%02x\n", __FUNCTION__,
		                      t->radio_unique_id[0], t->radio_unique_id[1], t->radio_unique_id[2],
		                      t->radio_unique_id[3], t->radio_unique_id[4], t->radio_unique_id[5]);
		PLATFORM_PRINTF_DEBUG("%s he msc len: %d\n", __FUNCTION__, t->he_msc_len);
		PLATFORM_PRINTF_DEBUG("%s he msc: %02x%02x%02x%02x\n", __FUNCTION__, t->he_msc[0], t->he_msc[1], t->he_msc[2], t->he_msc[3]);
		PLATFORM_PRINTF_DEBUG("%s he cap 1: %d\n", __FUNCTION__, t->he_capability_1);
		PLATFORM_PRINTF_DEBUG("%s he cap 2: %d\n", __FUNCTION__, t->he_capability_2);

		return 1;
	}
	return 0;
}

INT8U _obtainChannelPreferenceTLV(struct ChannelPreferenceTLV ***t)
{
	struct ChannelPreferenceTLV **tlvs = NULL;
	char **                       interfaces_names;
	INT8U                         interfaces_nr, i, j, l, feasible_interfaces_nr = 0;
	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_nr);
	int       op_classes_len;
	OP_CLASS *op_classes = PLATFORM_GET_OP_CLASS(&op_classes_len);

	if (op_classes) {

		for (i = 0; i < interfaces_nr; i++) {
			struct interfaceInfo *x;
			INT8U                 curr_channel = 0, curr_band = 0, curr_sub_band = 0;

			//Skip non-root interface
			if (PLATFORM_STRCMP(interfaces_names[i], map_radio_name_5gh) && PLATFORM_STRCMP(interfaces_names[i], map_radio_name_5gl) && PLATFORM_STRCMP(interfaces_names[i], map_radio_name_24g)) {
				continue;
			}

			x = PLATFORM_GET_1905_INTERFACE_INFO(interfaces_names[i]);
			if (NULL == x) {
				PLATFORM_PRINTF_ERROR("[MULTI_AP] Could not retrieve interface info for %s\n", interfaces_names[i]);
				continue;
			}

			// ignore the interface if it is not an AP
			if (!(IEEE80211_ROLE_AP == x->interface_type_data.ieee80211.role
				&& IS_IEEE_802_11_INTERFACE(x->interface_type))) {
				PLATFORM_PRINTF_DETAIL("[MULTI_AP] The interface %s is not operating a BSS.\n", interfaces_names[i]);
				PLATFORM_FREE_1905_INTERFACE_INFO(x);
				continue;
			} else {
				PLATFORM_PRINTF_DETAIL("[MULTI_AP] The interface %s is operating a BSS.\n", interfaces_names[i]);
			}

			if (0 != PLATFORM_GET_MIB(interfaces_names[i], "channel", &curr_channel, 1)) {
				PLATFORM_PRINTF_ERROR("[RTK] [%s] can not get channel for interface %s\n", __FUNCTION__, interfaces_names[i]);
			}

			//skip invalid channel
			if (0 == curr_channel) {
				PLATFORM_FREE_1905_INTERFACE_INFO(x);
				continue;
			}
			//assume channel < 14 to be 2g, otherwise 5g
			if (curr_channel <= 14) {
				curr_band = _2G;
			} else {
				curr_band = _5G;
				if (DMisTribandDevice()) {
					curr_sub_band = (curr_channel >= 100) ? _5GH : _5GL;
				}
			}

			if (0 == feasible_interfaces_nr) {
				tlvs = (struct ChannelPreferenceTLV **)PLATFORM_MALLOC(sizeof(struct ChannelPreferenceTLV *));
			} else {
				tlvs = (struct ChannelPreferenceTLV **)PLATFORM_REALLOC(tlvs, sizeof(struct ChannelPreferenceTLV *) * (feasible_interfaces_nr + 1));
			}

			// start to fill the TLV
			tlvs[feasible_interfaces_nr]           = (struct ChannelPreferenceTLV *)PLATFORM_MALLOC(sizeof(struct ChannelPreferenceTLV));
			tlvs[feasible_interfaces_nr]->tlv_type = TLV_TYPE_CHANNEL_PREFERENCE;
			PLATFORM_MEMCPY(tlvs[feasible_interfaces_nr]->radio_unique_id, x->mac_address, 6);
			PLATFORM_PRINTF_DEBUG("[MULTI_AP] radio unique id: %02x:%02x:%02x:%02x:%02x:%02x\n",
			                      tlvs[feasible_interfaces_nr]->radio_unique_id[0],
			                      tlvs[feasible_interfaces_nr]->radio_unique_id[1],
			                      tlvs[feasible_interfaces_nr]->radio_unique_id[2],
			                      tlvs[feasible_interfaces_nr]->radio_unique_id[3],
			                      tlvs[feasible_interfaces_nr]->radio_unique_id[4],
			                      tlvs[feasible_interfaces_nr]->radio_unique_id[5]);
			PLATFORM_FREE_1905_INTERFACE_INFO(x);

			tlvs[feasible_interfaces_nr]->op_class_nr         = 0;
			tlvs[feasible_interfaces_nr]->channel_preferences = NULL;
			// tlvs[feasible_interfaces_nr]->channel_preferences = (struct ChannelPreferencePerOpClass *)PLATFORM_MALLOC(sizeof(struct ChannelPreferencePerOpClass) * tlvs[feasible_interfaces_nr]->op_class_nr);
			for (j = 0; j < op_classes_len; j++) {
				if (curr_band == op_classes[j].band && ((DMisTribandDevice() && (curr_sub_band == op_classes[j].sub_band || _5GF == op_classes[j].sub_band)) || !DMisTribandDevice())) {
					PLATFORM_PRINTF_DETAIL("Channel band %d, class: %d\n", op_classes[j].band, op_classes[j].op_class);
					tlvs[feasible_interfaces_nr]->channel_preferences                                                         = (struct ChannelPreferencePerOpClass *)PLATFORM_REALLOC(tlvs[feasible_interfaces_nr]->channel_preferences, sizeof(struct ChannelPreferencePerOpClass) * (tlvs[feasible_interfaces_nr]->op_class_nr + 1));
					tlvs[feasible_interfaces_nr]->channel_preferences[tlvs[feasible_interfaces_nr]->op_class_nr].op_class     = op_classes[j].op_class;
					tlvs[feasible_interfaces_nr]->channel_preferences[tlvs[feasible_interfaces_nr]->op_class_nr].channel_nr   = 0;
					tlvs[feasible_interfaces_nr]->channel_preferences[tlvs[feasible_interfaces_nr]->op_class_nr].channel_list = NULL;
					//look for the channel
					if (_isChannelInArray(curr_channel, &(op_classes[j].channel_array[1]), op_classes[j].channel_array[0])) {
						tlvs[feasible_interfaces_nr]->channel_preferences[tlvs[feasible_interfaces_nr]->op_class_nr].channel_nr   = op_classes[j].channel_array[0] - 1;
						if (op_classes[j].channel_array[0] - 1 > 0) {
							tlvs[feasible_interfaces_nr]->channel_preferences[tlvs[feasible_interfaces_nr]->op_class_nr].channel_list = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * (op_classes[j].channel_array[0] - 1));
							INT8U count                                                                                               = 0;
							for (l = 1; l < op_classes[j].channel_array[0] + 1; l++) {
								if (op_classes[j].channel_array[l] == curr_channel || op_classes[j].channel_array[l] == get_channel_center_frequency_index(curr_channel) ||
										op_classes[j].channel_array[l] == get_160m_channel_center_frequency_index(curr_channel))
									continue;
								tlvs[feasible_interfaces_nr]->channel_preferences[tlvs[feasible_interfaces_nr]->op_class_nr].channel_list[count] = op_classes[j].channel_array[l];
								count++;
							}
						}
					}
					tlvs[feasible_interfaces_nr]->channel_preferences[tlvs[feasible_interfaces_nr]->op_class_nr].preference_reason_code = 0x00;
					tlvs[feasible_interfaces_nr]->op_class_nr++;
				}
			}
			/* End */
			feasible_interfaces_nr++;
			PLATFORM_PRINTF_DEBUG("[MULTI_AP] Create Channel Preference TLV for %s\n", interfaces_names[i]);
		}

		PLATFORM_FREE(op_classes);
	}

	*t = tlvs;
	return feasible_interfaces_nr;
}

void freeChannelPreferenceTLV(struct ChannelPreferenceTLV **t, INT8U TLV_nr)
{
	int i;
	for (i = 0; i < TLV_nr; i++) {
		PLATFORM_PRINTF_DETAIL("[MULTI_AP] Free Channel Preference TLV for radio unique id: %02x:%02x:%02x:%02x:%02x:%02x\n",
		                       t[i]->radio_unique_id[0], t[i]->radio_unique_id[1],
		                       t[i]->radio_unique_id[2], t[i]->radio_unique_id[3],
		                       t[i]->radio_unique_id[4], t[i]->radio_unique_id[5]);
		int j;
		for (j = 0; j < t[i]->op_class_nr; j++) {
			PLATFORM_FREE(t[i]->channel_preferences[j].channel_list);
		}
		PLATFORM_FREE(t[i]->channel_preferences);
		PLATFORM_FREE(t[i]);
	}
	PLATFORM_PRINTF_DETAIL("[MULTI_AP] Free %d Channel Preference TLVs\n", TLV_nr);
	PLATFORM_FREE(t);
	PLATFORM_PRINTF_DETAIL("[MULTI_AP] Finish free channel preference tlv task\n");
}

void _obtainRadioOperationRestrictionTLV(struct RadioOperationRestrictionTLV *t)
{
	t->tlv_type = TLV_TYPE_RADIO_OPERATION_RESTRICTION;
	//t->radio_unique_id = NULL;
	t->op_class_nr                 = 0;
	t->radio_operation_restriction = NULL;
	// todo fill the TLV
}

void _freeRadioOperationRestrictionTLV(struct RadioOperationRestrictionTLV *t)
{
	int i;

	for (i = 0; i < t->op_class_nr; i++) {
		PLATFORM_FREE(t->radio_operation_restriction[i].channel_operation_restriction);
	}

	PLATFORM_FREE(t->radio_operation_restriction);
}

INT8U _obtainChannelSelectionResponseTLV(struct ChannelSelectionResponseTLV ***t, INT8U response)
{
	struct ChannelPreferenceTLV *        controller_preferences    = NULL;
	INT8U                                controller_preferences_nr = 0;
	int                                  i;
	struct ChannelSelectionResponseTLV **tlvs = NULL;

	DMgetControllerChannelPreference(&controller_preferences, &controller_preferences_nr);

	if (controller_preferences_nr == 0) {
		tlvs              = (struct ChannelSelectionResponseTLV **)PLATFORM_MALLOC(sizeof(struct ChannelSelectionResponseTLV *));
		tlvs[0]           = (struct ChannelSelectionResponseTLV *)PLATFORM_MALLOC(sizeof(struct ChannelSelectionResponseTLV));
		tlvs[0]->tlv_type = TLV_TYPE_CHANNEL_SELECTION_RESPONSE;
		PLATFORM_MEMSET(tlvs[0]->radio_unique_id, 0, 6);
		tlvs[0]->response = response;
		*t                = tlvs;
		return 1;
	}

	tlvs = (struct ChannelSelectionResponseTLV **)PLATFORM_MALLOC(sizeof(struct ChannelSelectionResponseTLV *) * controller_preferences_nr);
	for (i = 0; i < controller_preferences_nr; i++) {
		tlvs[i]           = (struct ChannelSelectionResponseTLV *)PLATFORM_MALLOC(sizeof(struct ChannelSelectionResponseTLV));
		tlvs[i]->tlv_type = TLV_TYPE_CHANNEL_SELECTION_RESPONSE;
		PLATFORM_MEMCPY(tlvs[i]->radio_unique_id, controller_preferences[i].radio_unique_id, 6);
		tlvs[i]->response = response;
		if (tlvs[i]->response == 0) {
			PLATFORM_PRINTF_DEBUG("[MULTI_AP] Agent: Accept channel selection request with radio unique id: %02x:%02x:%02x:%02x:%02x:%02x\n",
			                      tlvs[i]->radio_unique_id[0],
			                      tlvs[i]->radio_unique_id[1],
			                      tlvs[i]->radio_unique_id[2],
			                      tlvs[i]->radio_unique_id[3],
			                      tlvs[i]->radio_unique_id[4],
			                      tlvs[i]->radio_unique_id[5]);
		} else {
			PLATFORM_PRINTF_DEBUG("[MULTI_AP] Agent: Reject channel selection request with radio unique id: %02x:%02x:%02x:%02x:%02x:%02x\n",
			                      tlvs[i]->radio_unique_id[0],
			                      tlvs[i]->radio_unique_id[1],
			                      tlvs[i]->radio_unique_id[2],
			                      tlvs[i]->radio_unique_id[3],
			                      tlvs[i]->radio_unique_id[4],
			                      tlvs[i]->radio_unique_id[5]);
		}
	}

	*t = tlvs;
	return controller_preferences_nr;
}

void _freeChannelSelectionResponseTLV(struct ChannelSelectionResponseTLV **t, INT8U TLV_nr)
{
	int i;
	for (i = 0; i < TLV_nr; i++) {
		PLATFORM_PRINTF_DETAIL("[MULTI_AP] Free Channel Selection Response TLV for radio unique id: %02x:%02x:%02x:%02x:%02x:%02x\n",
		                       t[i]->radio_unique_id[0], t[i]->radio_unique_id[1],
		                       t[i]->radio_unique_id[2], t[i]->radio_unique_id[3],
		                       t[i]->radio_unique_id[4], t[i]->radio_unique_id[5]);
		PLATFORM_FREE(t[i]);
	}
	PLATFORM_PRINTF_DETAIL("[MULTI_AP] Free %d Channel Selection Response TLVs\n", TLV_nr);
	PLATFORM_FREE(t);
	PLATFORM_PRINTF_DETAIL("[MULTI_AP] Finish free Channel Selection Response tlv task\n");
}

#ifdef RTK_MULTI_AP_R4
void _freeSpatialReuseReportTLV(struct SpatialReuseReportTLV **t, INT8U TLV_nr)
{
	int i;
	for (i = 0; i < TLV_nr; i++) {
		PLATFORM_PRINTF_DETAIL("[MULTI_AP] Free Spatial Reuse Report TLV for radio unique id: %02x:%02x:%02x:%02x:%02x:%02x\n",
		                       t[i]->ruid[0], t[i]->ruid[1],
		                       t[i]->ruid[2], t[i]->ruid[3],
		                       t[i]->ruid[4], t[i]->ruid[5]);
		PLATFORM_FREE(t[i]);
	}
	PLATFORM_PRINTF_DETAIL("[MULTI_AP] Free %d Spatial Reuse Report TLVs\n", TLV_nr);
	PLATFORM_FREE(t);
	PLATFORM_PRINTF_DETAIL("[MULTI_AP] Finish free Spatial Reuse Report TLV task\n");
}
#endif //RTK_MULTI_AP_R4

bool search_op_class(OP_CLASS op_class, const INT8U channel)
{
	int i;
	for (i = 0; i < op_class.channel_array[0]; i++) {
		if (channel == op_class.channel_array[i + 1]) {
			return true;
		}
	}
	return false;
}

INT8U _obtainOperatingChannelReportTLV(struct OperatingChannelReportTLV ***t)
{
	struct OperatingChannelReportTLV **tlvs = NULL;
	char **                            interfaces_names;
	INT8U                              interfaces_nr, i, feasible_interfaces_nr = 0;

	int       j, op_classes_len = 0;
	OP_CLASS *op_classes = PLATFORM_GET_OP_CLASS(&op_classes_len);

	if (NULL == op_classes) {
		PLATFORM_PRINTF_ERROR("[RTK] No operating class is available\n");
		return 0;
	}

	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_nr);

	for (i = 0; i < interfaces_nr; i++) {
		struct interfaceInfo *x;

		//only give root interface info
		if (PLATFORM_STRCMP(interfaces_names[i], map_radio_name_5gh) && PLATFORM_STRCMP(interfaces_names[i], map_radio_name_5gl) && PLATFORM_STRCMP(interfaces_names[i], map_radio_name_24g)) {
			continue;
		}

		x = PLATFORM_GET_1905_INTERFACE_INFO(interfaces_names[i]);
		if (NULL == x) {
			PLATFORM_PRINTF_ERROR("[MULTI_AP] Could not retrieve interface info for %s\n", interfaces_names[i]);
			continue;
		}

		// ignore the interface if it is not an AP
		if (!(IEEE80211_ROLE_AP == x->interface_type_data.ieee80211.role
			&& IS_IEEE_802_11_INTERFACE(x->interface_type))) {
			PLATFORM_PRINTF_DETAIL("[MULTI_AP] The interface %s is not operating a BSS.\n", interfaces_names[i]);
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
			continue;
		} else {
			PLATFORM_PRINTF_DETAIL("[MULTI_AP] The interface %s is operating a BSS.\n", interfaces_names[i]);
		}

		int   band = 0, sub_band = 0;
		INT8U channel;
		int   bandwidth, sideband = 0;
		INT8U transmit_power;

		if (PLATFORM_STRSTR(interfaces_names[i], map_radio_name_5gl) && PLATFORM_STRSTR(interfaces_names[i], map_radio_name_5gh)) {
			// radio 5G
			band = _5G;
			if (obtain_channel_info(map_radio_name_5gl, &channel, &bandwidth, &sideband, &transmit_power)) {
				PLATFORM_PRINTF_ERROR("Can not get channel info for interface %s\n", map_radio_name_5gl);
				PLATFORM_FREE_1905_INTERFACE_INFO(x);
				continue;
			}

		} else if (PLATFORM_STRSTR(interfaces_names[i], map_radio_name_5gh)) {
			band     = _5G;
			sub_band = _5GH;
			if (obtain_channel_info(map_radio_name_5gh, &channel, &bandwidth, &sideband, &transmit_power)) {
				PLATFORM_PRINTF_ERROR("Can not get channel info for interface %s\n", map_radio_name_5gh);
				PLATFORM_FREE_1905_INTERFACE_INFO(x);
				continue;
			}
		} else if (PLATFORM_STRSTR(interfaces_names[i], map_radio_name_5gl)) {
			band     = _5G;
			sub_band = _5GL;
			if (obtain_channel_info(map_radio_name_5gl, &channel, &bandwidth, &sideband, &transmit_power)) {
				PLATFORM_PRINTF_ERROR("Can not get channel info for interface %s\n", map_radio_name_5gl);
				PLATFORM_FREE_1905_INTERFACE_INFO(x);
				continue;
			}
		} else if (PLATFORM_STRSTR(interfaces_names[i], map_radio_name_24g)) {
			// radio 2.4G
			band = _2G;
			if (obtain_channel_info(map_radio_name_24g, &channel, &bandwidth, &sideband, &transmit_power)) {
				PLATFORM_PRINTF_ERROR("Can not get channel info for interface %s\n", map_radio_name_5gl);
				PLATFORM_FREE_1905_INTERFACE_INFO(x);
				continue;
			}
		} else {
			PLATFORM_PRINTF_ERROR("[RTK] [unknown interface: %s\n", interfaces_names[i]);
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
			continue;
		}

		if (0 == feasible_interfaces_nr) {
			tlvs = (struct OperatingChannelReportTLV **)PLATFORM_MALLOC(sizeof(struct OperatingChannelReportTLV *));
		} else {
			tlvs = (struct OperatingChannelReportTLV **)PLATFORM_REALLOC(tlvs, sizeof(struct OperatingChannelReportTLV *) * (feasible_interfaces_nr + 1));
		}

		// start to fill the TLV
		tlvs[feasible_interfaces_nr]           = (struct OperatingChannelReportTLV *)PLATFORM_MALLOC(sizeof(struct OperatingChannelReportTLV));
		tlvs[feasible_interfaces_nr]->tlv_type = TLV_TYPE_OPERATING_CHANNEL_REPORT;
		PLATFORM_MEMCPY(tlvs[feasible_interfaces_nr]->radio_unique_id, x->mac_address, 6);
		PLATFORM_PRINTF_DEBUG("[MULTI_AP] radio unique id: %02x:%02x:%02x:%02x:%02x:%02x\n",
		                      tlvs[feasible_interfaces_nr]->radio_unique_id[0],
		                      tlvs[feasible_interfaces_nr]->radio_unique_id[1],
		                      tlvs[feasible_interfaces_nr]->radio_unique_id[2],
		                      tlvs[feasible_interfaces_nr]->radio_unique_id[3],
		                      tlvs[feasible_interfaces_nr]->radio_unique_id[4],
		                      tlvs[feasible_interfaces_nr]->radio_unique_id[5]);

		PLATFORM_FREE_1905_INTERFACE_INFO(x);

		tlvs[feasible_interfaces_nr]->cur_op_class_nr    = 0;
		tlvs[feasible_interfaces_nr]->operating_channels = NULL;
		tlvs[feasible_interfaces_nr]->cur_transmit_power = 0;

		for (j = 0; j < op_classes_len; j++) {
			if (op_classes[j].bandwidth > bandwidth || (op_classes[j].side_band != SIDE_BAND_NONE && op_classes[j].side_band != sideband)) {
				continue;
			}

			if (band == op_classes[j].band && 128 == op_classes[j].op_class) {
				INT8U channel_center_frequency_index = get_channel_center_frequency_index(channel);
				if (channel_center_frequency_index) {
					tlvs[feasible_interfaces_nr]->cur_op_class_nr += 1;
					tlvs[feasible_interfaces_nr]->operating_channels                                                                = (struct OperatingChannelReportPerOpClass *)PLATFORM_REALLOC(tlvs[feasible_interfaces_nr]->operating_channels, sizeof(struct OperatingChannelReportPerOpClass) * tlvs[feasible_interfaces_nr]->cur_op_class_nr);
					tlvs[feasible_interfaces_nr]->operating_channels[tlvs[feasible_interfaces_nr]->cur_op_class_nr - 1].op_class    = op_classes[j].op_class;
					tlvs[feasible_interfaces_nr]->operating_channels[tlvs[feasible_interfaces_nr]->cur_op_class_nr - 1].cur_channel = channel_center_frequency_index;
				}
			} else if(band == op_classes[j].band && 129 == op_classes[j].op_class){
				INT8U channel_center_frequency_index = get_160m_channel_center_frequency_index(channel);
				if (channel_center_frequency_index) {
					tlvs[feasible_interfaces_nr]->cur_op_class_nr += 1;
					tlvs[feasible_interfaces_nr]->operating_channels                                                                = (struct OperatingChannelReportPerOpClass *)PLATFORM_REALLOC(tlvs[feasible_interfaces_nr]->operating_channels, sizeof(struct OperatingChannelReportPerOpClass) * tlvs[feasible_interfaces_nr]->cur_op_class_nr);
					tlvs[feasible_interfaces_nr]->operating_channels[tlvs[feasible_interfaces_nr]->cur_op_class_nr - 1].op_class    = op_classes[j].op_class;
					tlvs[feasible_interfaces_nr]->operating_channels[tlvs[feasible_interfaces_nr]->cur_op_class_nr - 1].cur_channel = channel_center_frequency_index;
				}
			} else if(band == op_classes[j].band && 129 == op_classes[j].op_class){
				INT8U channel_center_frequency_index = get_160m_channel_center_frequency_index(channel);
				if (channel_center_frequency_index) {
					tlvs[feasible_interfaces_nr]->cur_op_class_nr += 1;
					tlvs[feasible_interfaces_nr]->operating_channels                                                                = (struct OperatingChannelReportPerOpClass *)PLATFORM_REALLOC(tlvs[feasible_interfaces_nr]->operating_channels, sizeof(struct OperatingChannelReportPerOpClass) * tlvs[feasible_interfaces_nr]->cur_op_class_nr);
					tlvs[feasible_interfaces_nr]->operating_channels[tlvs[feasible_interfaces_nr]->cur_op_class_nr - 1].op_class    = op_classes[j].op_class;
					tlvs[feasible_interfaces_nr]->operating_channels[tlvs[feasible_interfaces_nr]->cur_op_class_nr - 1].cur_channel = channel_center_frequency_index;
				}
			} else if (band == op_classes[j].band && (sub_band == op_classes[j].sub_band || 0 == sub_band) && search_op_class(op_classes[j], channel)) {
				tlvs[feasible_interfaces_nr]->cur_op_class_nr += 1;
				tlvs[feasible_interfaces_nr]->operating_channels                                                             = (struct OperatingChannelReportPerOpClass *)PLATFORM_REALLOC(tlvs[feasible_interfaces_nr]->operating_channels, sizeof(struct OperatingChannelReportPerOpClass) * tlvs[feasible_interfaces_nr]->cur_op_class_nr);
				tlvs[feasible_interfaces_nr]->operating_channels[tlvs[feasible_interfaces_nr]->cur_op_class_nr - 1].op_class = op_classes[j].op_class;
				PLATFORM_PRINTF_DEBUG("[RTK] op class: %d\n", op_classes[j].op_class);
				tlvs[feasible_interfaces_nr]->operating_channels[tlvs[feasible_interfaces_nr]->cur_op_class_nr - 1].cur_channel = channel;
			}
		}

		tlvs[feasible_interfaces_nr]->cur_transmit_power = transmit_power;

		feasible_interfaces_nr++;
		PLATFORM_PRINTF_DEBUG("[MULTI_AP] Create Operating Channel Report TLV for %s\n", interfaces_names[i]);
	}

	*t = tlvs;
	PLATFORM_FREE(op_classes);
	return feasible_interfaces_nr;
}
void _freeOperatingChannelReportTLV(struct OperatingChannelReportTLV **t, INT8U TLV_nr)
{
	int i;
	for (i = 0; i < TLV_nr; i++) {
		PLATFORM_PRINTF_DETAIL("[MULTI_AP] Free Operating Channel Report TLV for radio unique id: %02x:%02x:%02x:%02x:%02x:%02x\n",
		                       t[i]->radio_unique_id[0], t[i]->radio_unique_id[1],
		                       t[i]->radio_unique_id[2], t[i]->radio_unique_id[3],
		                       t[i]->radio_unique_id[4], t[i]->radio_unique_id[5]);
		PLATFORM_FREE(t[i]->operating_channels);
		PLATFORM_FREE(t[i]);
	}
	PLATFORM_PRINTF_DETAIL("[MULTI_AP] Free %d Operating Channel Report TLVs\n", TLV_nr);
	PLATFORM_FREE(t);
	PLATFORM_PRINTF_DETAIL("[MULTI_AP] Finish free channel preference tlv task\n");
}

void _obtainSteeringRequestTLV(struct SteeringRequestTLV *t, struct steering_request *steering_req)
{
	INT8U index;
	t->tlv_type = TLV_TYPE_STEERING_REQUEST;
	PLATFORM_MEMCPY(t->bssid, steering_req->bssid, 6);
	t->mode                 = steering_req->mode; // request mode = 1, disassoc imminent bit = 1, abridged bit = 1
	t->window               = steering_req->window;
	t->disassociation_timer = steering_req->disassoc_timer;
	t->sta_nr               = steering_req->sta_nr;
	t->sta_mac_address      = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(INT8U[6]) * t->sta_nr);
	for (index = 0; index < t->sta_nr; index++) {
		PLATFORM_MEMCPY(t->sta_mac_address[index], steering_req->sta_mac_address[index], 6);
	}
	t->target_bss_nr = steering_req->target_bss_nr;
	t->target_bss    = (struct TargetBSS *)PLATFORM_MALLOC(sizeof(struct TargetBSS) * t->target_bss_nr);
	for (index = 0; index < t->target_bss_nr; index++) {
		PLATFORM_MEMCPY(t->target_bss[index].bssid, steering_req->target_bss[index].bssid, 6);
		t->target_bss[index].opclass = steering_req->target_bss[index].op_class;
		t->target_bss[index].channel = steering_req->target_bss[index].channel;
	}
}
void _freeSteeringRequestTLV(struct SteeringRequestTLV *t)
{
	PLATFORM_FREE(t->sta_mac_address);
	PLATFORM_FREE(t->target_bss);
}

void _obtainR2SteeringRequestTLV(struct Profile2SteeringRequestTLV *t, struct steering_request *steering_req)
{
	INT8U index;
	t->tlv_type = TLV_TYPE_PROFILE_2_STEERING_REQUEST;
	PLATFORM_MEMCPY(t->bssid, steering_req->bssid, 6);
	t->steer_parameters         = steering_req->mode; // request mode = 1, disassoc imminent bit = 1, abridged bit = 1
	t->steer_opportunity_window = steering_req->window;
	t->btm_disassociation_timer = steering_req->disassoc_timer;
	t->sta_list_count           = steering_req->sta_nr;
	t->sta_list                 = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(INT8U[6]) * t->sta_list_count);
	for (index = 0; index < t->sta_list_count; index++) {
		PLATFORM_MEMCPY(t->sta_list[index], steering_req->sta_mac_address[index], 6);
	}
	t->target_bssid_list_count = steering_req->target_bss_nr;
	t->target_bssid_list       = (struct Profile2SteeringRequestTargetBssidInfo *)PLATFORM_MALLOC(sizeof(struct Profile2SteeringRequestTargetBssidInfo) * t->target_bssid_list_count);
	for (index = 0; index < t->target_bssid_list_count; index++) {
		PLATFORM_MEMCPY(t->target_bssid_list[index].target_bssid, steering_req->target_bss[index].bssid, 6);
		t->target_bssid_list[index].target_bss_operating_class = steering_req->target_bss[index].op_class;
		t->target_bssid_list[index].target_bss_channel_number  = steering_req->target_bss[index].channel;
		t->target_bssid_list[index].reason_code                = steering_req->target_bss[index].reason_code;
	}
}
void _freeR2SteeringRequestTLV(struct Profile2SteeringRequestTLV *t)
{
	PLATFORM_FREE(t->sta_list);
	PLATFORM_FREE(t->target_bssid_list);
}

void _obtainSteeringBTMReportTLV(struct SteeringBTMReportTLV *t, struct bss_transition_response_para *response)
{
	t->tlv_type = TLV_TYPE_STEERING_BTM_REPORT;
	PLATFORM_MEMCPY(t->bssid, response->bssid, 6);
	PLATFORM_MEMCPY(t->sta_mac_address, response->addr, 6);
	t->status_code = response->status_code;
	PLATFORM_MEMCPY(t->target_bssid, response->target_bssid, 6);
	PLATFORM_PRINTF_INFO("[MULTI_AP] obtain Steering BTM Report TLV target_bssid: %02x:%02x:%02x:%02x:%02x:%02x status code = %d\n",
	                     t->target_bssid[0], t->target_bssid[1],
	                     t->target_bssid[2], t->target_bssid[3],
	                     t->target_bssid[4], t->target_bssid[5],
	                     t->status_code);
}

// void _freeSteeringBTMReportTLV(struct SteeringBTMReportTLV *t)
// {
// 	// todo free the TLV
// }

void _obtainClientAssociationControlRequestTLV(struct ClientAssociationControlRequestTLV **t, struct client_association_control_request *client_assoc_ctrl_req, INT8U TLV_nr)
{
	INT8U                                      index, k;
	struct ClientAssociationControlRequestTLV *tlvs = NULL;
	tlvs                                            = (struct ClientAssociationControlRequestTLV *)PLATFORM_MALLOC(sizeof(struct ClientAssociationControlRequestTLV) * TLV_nr);
	for (index = 0; index < TLV_nr; index++) {
		tlvs[index].tlv_type = TLV_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST;
		PLATFORM_MEMCPY(tlvs[index].bssid, client_assoc_ctrl_req[index].bssid, 6);
		tlvs[index].association_control = client_assoc_ctrl_req[index].assoc_ctrl;
		tlvs[index].validity_period     = client_assoc_ctrl_req[index].validity_period;
		tlvs[index].sta_nr              = client_assoc_ctrl_req[index].sta_nr;
		tlvs[index].sta_mac_address     = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(INT8U[6]) * tlvs[index].sta_nr);
		for (k = 0; k < tlvs[index].sta_nr; k++) {
			PLATFORM_MEMCPY(tlvs[index].sta_mac_address[k], client_assoc_ctrl_req[index].sta_mac_address[k], 6);
		}
	}

	*t = tlvs;
}
void _freeClientAssociationControlRequestTLV(struct ClientAssociationControlRequestTLV *t, INT8U TLV_nr)
{
	INT8U index;
	for (index = 0; index < TLV_nr; index++) {
		PLATFORM_FREE(t[index].sta_mac_address);
	}
	PLATFORM_FREE(t);
}

int _obtainClientCapabilityTLV(struct ClientCapabilityReportTLV *t, char *interface_name, INT8U *clientMac)
{
	INT8U tempbuf[580]; //header size(2~4) + max assoc req(0~512+4)
	INT8U ret = 0, i;

	t->tlv_type          = TLV_TYPE_CLIENT_CAPABLITY_REPORT;
	t->result_code       = 1;
	t->frame_body_length = 0;
	t->frame_body        = NULL;

	if (interface_name == NULL) {
		PLATFORM_PRINTF_WARNING("%s (%d) - Interface name is NULL\n", __FUNCTION__, __LINE__);
		return ret;
	}

	//i = rtk_linux_get_client_capability_info(interface_name, clientMac, &tempbuf);
	i = PLATFORM_CLIENT_CAPABILITY_IOCTL(interface_name, clientMac, tempbuf);

	if (i > 1) {
		t->result_code = tempbuf[0];
		if(t->result_code == 1) { //no assoc req buf
			ret = 0;
		} else { //with assoc req buf
			if (tempbuf[1] > 0) { //buf size <= 256
				t->frame_body_length = tempbuf[1];
				t->frame_body        = PLATFORM_MALLOC(t->frame_body_length);
				if (t->frame_body)
					PLATFORM_MEMCPY(t->frame_body, &tempbuf[2], t->frame_body_length);
			} else { //buf size > 256 && <= 512
				PLATFORM_MEMCPY(&t->frame_body_length, &tempbuf[2], sizeof(INT16U));
				if (t->frame_body_length > 512) {
					PLATFORM_PRINTF_ERROR("[%s][%d] frame body length wrong:[%d] \n", __FUNCTION__, __LINE__, t->frame_body_length);
					t->frame_body = NULL;
					t->frame_body_length = 0;
					return ret;
				}
				t->frame_body = PLATFORM_MALLOC(t->frame_body_length);
				if (t->frame_body)
					PLATFORM_MEMCPY(t->frame_body, &tempbuf[4], t->frame_body_length);
			}
			ret = 1;
		}
	}
	// PLATFORM_PRINTF_DEBUG("Result code: %d\n", t->result_code);
	// PLATFORM_PRINTF_DEBUG("Assoc Req Len: %u\n", t->frame_body_length);
	// for(i = 0; i < t->frame_body_length; i++){
	// 	PLATFORM_PRINTF_DEBUG("%02x", t->frame_body[i]);
	// }
	// PLATFORM_PRINTF_DEBUG("\n");
	return ret;
}

void _freeClientCapabilityTLV(struct ClientCapabilityReportTLV *t)
{
	if (t->frame_body_length && t->frame_body)
		PLATFORM_FREE(t->frame_body);
}

void _obtainAPMetricsTLV(struct APMetricsTLV *t, INT8U *bssid, INT8U dummy_tlv)
{
	INT8U  tempbuf[2048] = { 0 };
	INT16U len;
	int    i;
	char * interface_name;

	INT8U empty_mac[6] = { 0 };

	t->tlv_type = TLV_TYPE_AP_METRICS;
	PLATFORM_MEMCPY(t->bssid, bssid, MACADDRLEN);
	t->ch_util = 0;
	t->sta_nr  = 0;
	t->esp_ie  = 0;

	if (dummy_tlv) {
		PLATFORM_MEMCPY(t->bssid, empty_mac, 6);
		t->ch_util = 0;
		PLATFORM_MEMCPY(&t->sta_nr, &tempbuf[0], 2);
		t->esp_ie = 0;
		PLATFORM_MEMCPY(t->esp_acbe, &tempbuf[0], 3);
		PLATFORM_MEMCPY(t->esp_acbk, &tempbuf[0], 3);
		PLATFORM_MEMCPY(t->esp_acvi, &tempbuf[0], 3);
		PLATFORM_MEMCPY(t->esp_acvo, &tempbuf[0], 3);
		return;
	}

	interface_name = DMmacToInterfaceName((INT8U *)bssid);

	if (interface_name == NULL) {
		PLATFORM_PRINTF_WARNING("%s (%d) - Interface name is NULL\n", __FUNCTION__, __LINE__);
		return;
	}

	//i = rtk_linux_get_ap_metric(interface_name, 0, &tempbuf);
	i = PLATFORM_AP_METRIC_IOCTL(interface_name, 0, tempbuf);

	PLATFORM_MEMCPY(&len, &tempbuf[1], sizeof(INT16U));
	if (i > 0) {
		if (memcmp(empty_mac, &tempbuf[3], 6)) {
			PLATFORM_MEMCPY(t->bssid, &tempbuf[3], 6);
		}
		t->ch_util = tempbuf[9];
		PLATFORM_MEMCPY(&t->sta_nr, &tempbuf[10], sizeof(INT16U));
		t->esp_ie = tempbuf[12];
		PLATFORM_MEMCPY(t->esp_acbe, &tempbuf[13], 3);
		PLATFORM_MEMCPY(t->esp_acbk, &tempbuf[16], 3);
		PLATFORM_MEMCPY(t->esp_acvi, &tempbuf[19], 3);
		PLATFORM_MEMCPY(t->esp_acvo, &tempbuf[22], 3);
	}
#if 0
	PLATFORM_PRINTF_DEBUG("%s - ", __FUNCTION__);
	for(i = 0; i < (len+2); i++){
		PLATFORM_PRINTF_DEBUG("%02x", tempbuf[i]);
	}
	PLATFORM_PRINTF_DEBUG("\n");
#endif
}

void _obtainAPExtendedMetricsTLV(struct APExtendedMetricsTLV *t, INT8U *bssid)
{
	INT8U  tempbuf[2048] = { 0 };
	INT16U len           = 0;
	int    i, byte_counter_units;
	char * interface_name;

	t->tlv_type = TLV_TYPE_AP_EXTENDED_METRICS;
	PLATFORM_MEMCPY(t->radio_unique_identifier, bssid, MACADDRLEN);
	t->unicast_byte_sent       = 0;
	t->unicast_byte_received   = 0;
	t->multicast_byte_sent     = 0;
	t->multicast_byte_received = 0;
	t->broadcast_byte_sent     = 0;
	t->broadcast_byte_received = 0;

	interface_name = DMmacToInterfaceName((INT8U *)bssid);

	if (interface_name == NULL) {
		PLATFORM_PRINTF_WARNING("%s (%d) - Interface name is NULL\n", __FUNCTION__, __LINE__);
		return;
	}

	byte_counter_units = DMgetByteCounterUnits();

	//i = rtk_linux_get_ap_metric(interface_name, 0, &tempbuf);
	i = PLATFORM_AP_METRIC_IOCTL(interface_name, 3, tempbuf);

	PLATFORM_MEMCPY(&len, &tempbuf[1], sizeof(INT16U));
	if (i > 0) {
		PLATFORM_MEMCPY(t->radio_unique_identifier, &tempbuf[3], 6);
		// initial: get values in byte
		PLATFORM_MEMCPY(&t->unicast_byte_sent, &tempbuf[9], 4);
		PLATFORM_MEMCPY(&t->unicast_byte_received, &tempbuf[13], 4);
		PLATFORM_MEMCPY(&t->multicast_byte_sent, &tempbuf[17], 4);
		PLATFORM_MEMCPY(&t->multicast_byte_received, &tempbuf[21], 4);
		PLATFORM_MEMCPY(&t->broadcast_byte_sent, &tempbuf[25], 4);
		PLATFORM_MEMCPY(&t->broadcast_byte_received, &tempbuf[29], 4);
		// shift according to byte counter units (bcu)
		// byte to byte		= >> 0 (bcu = 0)
		// byte to kibibyte = >> 3 (bcu = 1)
		// byte to mebibyte = >> 6 (bcu = 2)
		t->unicast_byte_sent       = t->unicast_byte_sent >> (10 * byte_counter_units);
		t->unicast_byte_received   = t->unicast_byte_received >> (10 * byte_counter_units);
		t->multicast_byte_sent     = t->multicast_byte_sent >> (10 * byte_counter_units);
		t->multicast_byte_received = t->multicast_byte_received >> (10 * byte_counter_units);
		t->broadcast_byte_sent     = t->broadcast_byte_sent >> (10 * byte_counter_units);
		t->broadcast_byte_received = t->broadcast_byte_received >> (10 * byte_counter_units);
	}
}

void _obtainRadioMetricsTLV(struct RadioMetricsTLV *t, INT8U *radio_mac)
{
	t->tlv_type = TLV_TYPE_RADIO_METRICS;
	memcpy(t->radio_unique_identifier, radio_mac, 6);
	t->noise        = 1;
	t->transmit     = 1;
	t->receiveself  = 1;
	t->receiveother = 1;
}

int _obtainAssociatedStaLinkMetricsTLV(struct AssociatedSTALinkMetricsTLV *t, char *interface_name, INT8U *macaddr)
{

	INT8U tempbuf[2048];
	int   i = 0;

	t->tlv_type = TLV_TYPE_ASSOCIATED_STA_LINK_METRICS;
	PLATFORM_MEMCPY(t->mac_address, macaddr, 6);
	t->bssid_nr               = 0;
	t->assoc_sta_link_metrics = NULL;

	if (interface_name == NULL)
		return i;

	i = PLATFORM_ASSOC_STA_IOCTL(interface_name, 0, macaddr, tempbuf);

	if (i > 0) {
		t->bssid_nr = tempbuf[9];

		if (t->bssid_nr > 0) {
			t->assoc_sta_link_metrics = PLATFORM_MALLOC(sizeof(struct AssocSTALinkMetrics));

			PLATFORM_MEMCPY(t->assoc_sta_link_metrics->bssid, &tempbuf[10], 6);
			PLATFORM_MEMCPY(&t->assoc_sta_link_metrics->time_delta, &tempbuf[16], 4);
			PLATFORM_MEMCPY(&t->assoc_sta_link_metrics->dataRate_downlink, &tempbuf[20], 4);
			PLATFORM_MEMCPY(&t->assoc_sta_link_metrics->dataRate_uplink, &tempbuf[24], 4);
			t->assoc_sta_link_metrics->uplink_rcpi = tempbuf[28];
		}
		i = 1;
	} else {
		PLATFORM_PRINTF_WARNING("%s - Cannot retrieve STA %02x%02x%02x%02x%02x%02x info\n", __FUNCTION__,
		                        macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
		i = 0;
	}
#if 0
	len = tempbuf[1];
	PLATFORM_PRINTF_DEBUG("Len: %d\n", len);
	for(i = 0; i < len; i++){
		PLATFORM_PRINTF_DEBUG("%02x", tempbuf[i+2]);
	}
	PLATFORM_PRINTF_DEBUG("\n");
#endif
	return i;
}

void _obtainAssociatedStaTrafficStatsTLV(struct AssociatedSTATrafficStatsTLV *t, char *interface_name, INT8U *macaddr)
{

	INT8U tempbuf[2048];
	int   i, byte_counter_units;

	t->tlv_type = TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS;
	PLATFORM_MEMCPY(t->sta_mac_address, macaddr, 6);
	t->bytes_sent           = 0;
	t->bytes_received       = 0;
	t->packets_sent         = 0;
	t->packets_received     = 0;
	t->tx_packets_errors    = 0;
	t->rx_packets_errors    = 0;
	t->retransmission_count = 0;

	byte_counter_units = DMgetByteCounterUnits();

	i = PLATFORM_ASSOC_STA_IOCTL(interface_name, 1, macaddr, tempbuf);

	if (i > 0) {
		// initial: get values in byte
		PLATFORM_MEMCPY(&t->bytes_sent, &tempbuf[9], 4);
		PLATFORM_MEMCPY(&t->bytes_received, &tempbuf[13], 4);
		// shift according to byte counter units (bcu)
		// byte to byte		= >> 0 (bcu = 0)
		// byte to kibibyte = >> 3 (bcu = 1)
		// byte to mebibyte = >> 6 (bcu = 2)
		t->bytes_sent     = t->bytes_sent >> (10 * byte_counter_units);
		t->bytes_received = t->bytes_received >> (10 * byte_counter_units);
		PLATFORM_MEMCPY(&t->packets_sent, &tempbuf[17], 4);
		PLATFORM_MEMCPY(&t->packets_received, &tempbuf[21], 4);
		PLATFORM_MEMCPY(&t->tx_packets_errors, &tempbuf[25], 4);
		PLATFORM_MEMCPY(&t->rx_packets_errors, &tempbuf[29], 4);
		PLATFORM_MEMCPY(&t->retransmission_count, &tempbuf[33], 4);
	} else
		PLATFORM_PRINTF_WARNING("%s - Cannot retrieve STA %02x%02x%02x%02x%02x%02x info\n", __FUNCTION__,
		                        macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
#if 0
	len = tempbuf[1];
	for(i = 0; i < len; i++){
		PLATFORM_PRINTF_DEBUG("%02x", tempbuf[i+2]);
	}
	PLATFORM_PRINTF_DEBUG("\n");
#endif
}

int _obtainAssociatedStaExtendedLinkMetricsTLV(struct AssociatedSTAExtendedLinkMericsTLV *t, char *interface_name, INT8U *macaddr)
{

	INT8U tempbuf[2048];
	int   i = 0;

	t->tlv_type = TLV_TYPE_ASSOCIATED_STA_EXTENDED_LINK_METRICS;
	PLATFORM_MEMCPY(t->sta_mac_address, macaddr, 6);
	t->bssid_nr     = 0;
	t->reported_bss = NULL;

	if (interface_name == NULL)
		return i;

	i = PLATFORM_ASSOC_STA_IOCTL(interface_name, 2, macaddr, tempbuf);

	if (i > 0) {
		t->bssid_nr = tempbuf[9];

		if (t->bssid_nr > 0) {
			t->reported_bss = PLATFORM_MALLOC(sizeof(struct ReportedBSSInfo));
			PLATFORM_MEMCPY(t->reported_bss->bssid, &tempbuf[10], 6);
			PLATFORM_MEMCPY(&t->reported_bss->last_data_downlink_rate, &tempbuf[16], 4);
			PLATFORM_MEMCPY(&t->reported_bss->last_data_uplink_rate, &tempbuf[20], 4);
			PLATFORM_MEMCPY(&t->reported_bss->utilization_receive, &tempbuf[24], 4);
			PLATFORM_MEMCPY(&t->reported_bss->utilization_transmit, &tempbuf[28], 4);
		}
		i = 1;
	} else {
		PLATFORM_PRINTF_WARNING("%s - Cannot retrieve STA %02x%02x%02x%02x%02x%02x info\n", __FUNCTION__,
		                        macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
		i = 0;
	}
#if 0
	len = tempbuf[1];
	PLATFORM_PRINTF_DEBUG("Len: %d\n", len);
	for(i = 0; i < len; i++){
		PLATFORM_PRINTF_DEBUG("%02x", tempbuf[i+2]);
	}
	PLATFORM_PRINTF_DEBUG("\n");
#endif
	return i;
}

void _obtainUnassociatedStaLinkMetricsQueryTLV(struct UnassociatedSTALinkMetricsQueryTLV *t, INT8U channel_list_nr, struct ChannelListInfo *channel_list_entries)
{

	INT8U i, k;
	t->channel_list_entry = PLATFORM_MALLOC(sizeof(struct ChannelListInfo) * channel_list_nr);
	for (i = 0; i < channel_list_nr; i++) {
		t->channel_list_entry[i].channel_nr = channel_list_entries[i].channel_nr;
		t->channel_list_entry[i].sta_nr     = channel_list_entries[i].sta_nr;
		if (t->channel_list_entry[i].sta_nr > 0) {
			t->channel_list_entry[i].sta_mac_address = PLATFORM_MALLOC(6 * t->channel_list_entry[i].sta_nr);
			for (k = 0; k < t->channel_list_entry[i].sta_nr; k++) {
				PLATFORM_MEMCPY(t->channel_list_entry[i].sta_mac_address[k], channel_list_entries[i].sta_mac_address[k], MACADDRLEN);
			}
		}
	}
}

void _obtainUnassociatedStaLinkMetricsResponseTLV(struct UnassociatedSTALinkMetricsResponseTLV *t, INT8U op_class, INT8U sta_nr, INT8U *buf)
{
	INT8U *p;

	t->tlv_type                 = TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE;
	t->op_class                 = op_class;
	t->sta_nr                   = sta_nr;
	t->unassoc_sta_link_metrics = NULL;

	PLATFORM_PRINTF_DETAIL("==========================================\n");
	PLATFORM_PRINTF_DETAIL("==============STATION NUMBER:    %d\n", sta_nr);
	PLATFORM_PRINTF_DETAIL("==========================================\n");

	if (sta_nr > 0) {
		int i;
		p                           = buf;
		t->unassoc_sta_link_metrics = PLATFORM_MALLOC(sizeof(struct UnassocSTALinkMetrics) * sta_nr);

		for (i = 0; i < sta_nr; i++) {
			PLATFORM_MEMCPY(t->unassoc_sta_link_metrics[i].sta_mac_address, p, 6);
			p += 6;

			t->unassoc_sta_link_metrics[i].channel_number = *p;
			p++;

			PLATFORM_MEMCPY(&t->unassoc_sta_link_metrics[i].time_delta, p, 4);
			p += 4;

			t->unassoc_sta_link_metrics[i].uplink_rcpi = *p;
			p++;
		}
	}
}

void _obtainBeaconMetricsQueryTLV(struct BeaconMetricsQueryTLV *t, struct BeaconMetricsQueryTLV *l)
{

	INT8U i;

	t->tlv_type = TLV_TYPE_BEACON_METRICS_QUERY;
	PLATFORM_MEMCPY(t->mac_address, l->mac_address, MACADDRLEN);
	t->op_class       = l->op_class;
	t->channel_number = l->channel_number;
	PLATFORM_MEMCPY(t->bssid, l->bssid, MACADDRLEN);
	t->report_detail = l->report_detail;
	t->ssid_len      = l->ssid_len;
	if (t->ssid_len > 0) {
		t->ssid = PLATFORM_MALLOC(t->ssid_len);
		PLATFORM_MEMCPY(t->ssid, l->ssid, t->ssid_len);
	}

	t->ap_channel_report_nr = l->ap_channel_report_nr;

	if (t->ap_channel_report_nr > 0) {
		t->ap_channel_report = PLATFORM_MALLOC(sizeof(struct ApChannelReport) * t->ap_channel_report_nr);
		for (i = 0; i < t->ap_channel_report_nr; i++) {
			t->ap_channel_report[i].len      = l->ap_channel_report[i].len;
			t->ap_channel_report[i].op_class = l->ap_channel_report[i].op_class;
			if (t->ap_channel_report[i].len > 1) {
				t->ap_channel_report[i].channel_list = PLATFORM_MALLOC(sizeof(INT8U) * (t->ap_channel_report[i].len - 1));
				PLATFORM_MEMCPY(t->ap_channel_report[i].channel_list, l->ap_channel_report[i].channel_list, (t->ap_channel_report[i].len - 1));
			}
		}
	}

	t->elementID_nr = l->elementID_nr;
	if (t->elementID_nr > 0) {
		t->element_list = PLATFORM_MALLOC(sizeof(INT8U) * t->elementID_nr);
		PLATFORM_MEMCPY(t->element_list, l->element_list, t->elementID_nr);
	}
}

void freeBeaconMetricsQueryTLV(struct BeaconMetricsQueryTLV *t)
{

	INT8U i;

	if (t->ssid_len > 0)
		PLATFORM_FREE(t->ssid);
	if (t->ap_channel_report_nr > 0) {
		for (i = 0; i < t->ap_channel_report_nr; i++) {
			if (t->ap_channel_report[i].len > 1) {
				PLATFORM_FREE(t->ap_channel_report[i].channel_list);
			}
		}
		PLATFORM_FREE(t->ap_channel_report);
	}
	if (t->elementID_nr > 0)
		PLATFORM_FREE(t->element_list);
}

struct BeaconMetricsResponseTLV *_obtainBeaconMetricsResponseTLV(INT8U *buf, INT16U buf_len)
{
#define PENDING_LEN (buf_len - (p - buf))

	struct BeaconMetricsResponseTLV *   t;
	struct BeaconMeasurementReportInfo *report;
	INT8U                               i;
	INT8U *                             p;

	t = PLATFORM_MALLOC(sizeof(struct BeaconMetricsResponseTLV));

	t->tlv_type       = TLV_TYPE_BEACON_METRICS_RESPONSE;
	t->reserved_field = 0;
	PLATFORM_MEMCPY(t->mac_address, &buf[0], MACADDRLEN);
	t->measurement_report_nr = 0;
	t->measurement_reports   = NULL;

#if 0
	len = buf[1];
	PLATFORM_PRINTF_DEBUG("Len: %d\n", len);
	for(i = 0; i < len; i++){
		if(i > 0 && i % 16 == 0)
			PLATFORM_PRINTF_DEBUG("\n");
		PLATFORM_PRINTF_DEBUG("%02x", buf[i+2]);
	}
	PLATFORM_PRINTF_DEBUG("\n");
#endif

	t->measurement_report_nr = buf[7];

	if (t->measurement_report_nr <= 0)
		return t;

	t->measurement_reports = PLATFORM_MALLOC(sizeof(struct BeaconMeasurementReportIE) * t->measurement_report_nr);
	p                      = &buf[8];

	for (i = 0; i < t->measurement_report_nr; i++) {
		if (*p != MEASUREMENT_REPORT_ELEMENT_ID) {
			PLATFORM_FREE(t->measurement_reports);
			t->measurement_report_nr = 0;
			t->measurement_reports   = NULL;
			PLATFORM_PRINTF_WARNING("Invalid element id, data might be corrupted, skip.\n");
			break;
		}

		if (PENDING_LEN < (2 + BEACON_REPORT_FIXED_IE)) {
			PLATFORM_FREE(t->measurement_reports);
			t->measurement_report_nr = 0;
			t->measurement_reports   = NULL;
			PLATFORM_PRINTF_WARNING("Invalid report len, data might be corrupted, skip.\n");
			break;
		}

		t->measurement_reports[i].elementId = *p;
		p++;

		t->measurement_reports[i].len = *p;
		p++;

		t->measurement_reports[i].measurementToken = *p;
		p++;

		t->measurement_reports[i].measurementReportMode = *p;
		p++;

		t->measurement_reports[i].measurementType = *p;
		p++;

		PLATFORM_MEMSET(&t->measurement_reports[i].info, 0, sizeof(struct BeaconMeasurementReportInfo));
		t->measurement_reports[i].subelement_id   = 0;
		t->measurement_reports[i].subelements_len = 0;
		t->measurement_reports[i].subelements     = NULL;

		if (t->measurement_reports[i].measurementReportMode != 0) {
			PLATFORM_FREE(t->measurement_reports);
			t->measurement_report_nr = 0;
			t->measurement_reports   = NULL;
			PLATFORM_PRINTF_WARNING("Invalid report mode, data might be corrupted, skip.\n");
			break;
		}

		report = (struct BeaconMeasurementReportInfo *)p;

		//PLATFORM_MEMCPY(&t->measurement_reports[i].info, report, sizeof(struct BeaconMeasurementReportInfo));

		t->measurement_reports[i].info.op_class         = report->op_class;
		t->measurement_reports[i].info.channel          = report->channel;
		t->measurement_reports[i].info.measure_time_hi  = report->measure_time_hi;
		t->measurement_reports[i].info.measure_time_lo  = report->measure_time_lo;
		t->measurement_reports[i].info.measure_duration = report->measure_duration;
		t->measurement_reports[i].info.frame_info       = report->frame_info;
		t->measurement_reports[i].info.RCPI             = report->RCPI;
		t->measurement_reports[i].info.RSNI             = report->RSNI;
		memcpy(t->measurement_reports[i].info.bssid, report->bssid, 6);
		t->measurement_reports[i].info.antenna_id = report->antenna_id;
		t->measurement_reports[i].info.parent_tsf = report->parent_tsf;

		p += 26; //BEACON_REPORT_FIXED_IE;

		//p++; //offset for subelement len element
		if (t->measurement_reports[i].len > BEACON_REPORT_FIXED_IE) {

			t->measurement_reports[i].subelement_id = *p;
			p++;

			t->measurement_reports[i].subelements_len = *p;
			p++;

			if (t->measurement_reports[i].subelements_len > 0) {
				if (PENDING_LEN < t->measurement_reports[i].subelements_len) {
					PLATFORM_FREE(t->measurement_reports);
					t->measurement_report_nr = 0;
					t->measurement_reports   = NULL;
					PLATFORM_PRINTF_WARNING("Invalid subelements len, data might be corrupted, skip.\n");
					break;
				}

				t->measurement_reports[i].subelements = PLATFORM_MALLOC(t->measurement_reports[i].subelements_len);
				PLATFORM_MEMCPY(t->measurement_reports[i].subelements, p, t->measurement_reports[i].subelements_len);
				p += t->measurement_reports[i].subelements_len; //offset for value of subelement len
			}

			if (t->measurement_reports[i].len - 31 > t->measurement_reports[i].subelements_len)
				t->measurement_reports[i].subelements_remain_len = t->measurement_reports[i].len - 31 - t->measurement_reports[i].subelements_len;
			else
				t->measurement_reports[i].subelements_remain_len = 0;
			if (t->measurement_reports[i].subelements_remain_len > 0) {
				if (PENDING_LEN < t->measurement_reports[i].subelements_remain_len) {
					PLATFORM_FREE(t->measurement_reports);
					t->measurement_report_nr = 0;
					t->measurement_reports   = NULL;
					PLATFORM_PRINTF_WARNING("Invalid subelements remain len, data might be corrupted, skip.\n");
					break;
				}

				PLATFORM_MEMCPY(t->measurement_reports[i].subelements_remain, p, t->measurement_reports[i].subelements_remain_len);
				p += t->measurement_reports[i].subelements_remain_len;
			}
		}
		//p += sizeof(struct dot11k_beacon_measurement_report);
	}
	return t;
#undef PENDING_LEN
}

void _freeBeaconMetricsResponseTLV(struct BeaconMetricsResponseTLV *t)
{
	INT8U i;
	if (t->measurement_report_nr > 0 && t->measurement_reports) {
		for (i = 0; i < t->measurement_report_nr; i++) {
			if (t->measurement_reports[i].subelements_len > 0 && t->measurement_reports[i].subelements) {
				PLATFORM_FREE(t->measurement_reports[i].subelements);
			}
		}
		PLATFORM_FREE(t->measurement_reports);
	}
	PLATFORM_FREE(t);
}

#ifdef RTK_MULTI_AP_R2
//******************************************************************************
//* Profile 2 TLV Filling Up Functions
//******************************************************************************

void _obtainProfileTLV(struct MultiAPProfileTLV *profile_tlv)
{
	profile_tlv->tlv_type = TLV_TYPE_MULTIAP_PROFILE;
	profile_tlv->profile  = DMgetMultiAPProfile();
}

void _obtainProfile2APCapabilityTLV(struct Profile2APCapabilityTLV *tlv)
{
	tlv->tlv_type                                  = TLV_TYPE_PROFILE_2_CAPABILITY;
	tlv->max_total_nr_service_prioritization_rules = 1;
	tlv->reserved                                  = 0;
	tlv->units                                     = 0x40;
	tlv->max_total_nr_VIDs                         = 0;
	// Based on WFA Spec v3 9.1
	if (MULTI_AP_PROFILE_1 == DMgetTargetDeviceMultiAPProfile(DMcontrollerMacGet())) {
		// if connected to R1 controller, byte counter units = 0b00
		tlv->units &= 0b00 << 6;
		DMsetByteCounterUnits(0);
	}
	tlv->max_total_nr_VIDs = 2; // max total number of VIDs >= 2
#ifdef RTK_MULTI_AP_R3
	if (MULTI_AP_PROFILE_3 <= DMgetMultiAPProfile()) {
		tlv->units |= 1 << 5; // service prioritization bit = 1
	}
#endif
}

void _obtainMetricCollectionIntervalTLV(struct MetricCollectionIntervalTLV *tlv)
{
	tlv->tlv_type            = TLV_TYPE_METRIC_COLLECTION_INTERVAL;
	tlv->collection_interval = 1;
}

// void _freeProfile2APCapabilityTLV(struct Profile2APCapabilityTLV *tlv)
// {
// 	PLATFORM_FREE(tlv->interface_identifier);
// }

void _obtainRadioAdvancedCapabilitiesTLV(struct APRadioAdvancedCapabilitiesTLV *tlv, char *radio_name)
{
	tlv->tlv_type    = TLV_TYPE_AP_RADIO_ADVANCED_CAPABILITIES;
	INT8U *radio_uid = DMinterfaceNameToMac(radio_name);
	PLATFORM_MEMCPY(tlv->radio_id, radio_uid, 6);
	tlv->backhaul_bss_traffic_separation_mix_no_support = 0xC0;
}

void _obtainChannelScanCapabilitiesTLV(struct ChannelScanCapabilitiesTLV *t)
{
	//obtain op_classes
	int       op_classes_len = 0;
	OP_CLASS *op_classes     = PLATFORM_GET_OP_CLASS(&op_classes_len);

	t->tlv_type = TLV_TYPE_CHANNEL_SCAN_CAPABILITIES;
	t->radio_nr = 0;
	t->radios   = NULL;

	if (NULL == op_classes) {
		PLATFORM_PRINTF_ERROR("[RTK] No operating class is available\n");
		return;
	}

	//obtain info
	char **interfaces_names;
	INT8U  interfaces_nr, i, j;
	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_nr);

	for (i = 0; i < interfaces_nr; i++) {
		struct interfaceInfo *x;

		if (PLATFORM_STRSTR(interfaces_names[i], map_vxd_prefix)
			|| PLATFORM_STRSTR(interfaces_names[i], map_vap_prefix)
			|| PLATFORM_GET_VALID_INTERFACE_NAME(interfaces_names[i], VALID_ETH_INTERFACES_IN_BRIDGE)
			|| (controller_interface && PLATFORM_STRSTR(interfaces_names[i], controller_interface))) {
			continue; //skip non main root interfaces
		}

		x = PLATFORM_GET_1905_INTERFACE_INFO(interfaces_names[i]);

		if (NULL == x) {
			PLATFORM_PRINTF_ERROR("[MULTI_AP] Could not retrieve interface info for %s\n", interfaces_names[i]);
			continue;
		}

		// ignore the interface if it is not an AP
		if (!(IEEE80211_ROLE_AP == x->interface_type_data.ieee80211.role
			&& IS_IEEE_802_11_INTERFACE(x->interface_type))) {
			PLATFORM_PRINTF_DEBUG("[MULTI_AP] The interface %s is not operating a BSS.\n", interfaces_names[i]);
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
			continue;
		}

		//Start to fill in TLV
		t->radios = (struct ChannelScanCapabilitiesRadio *)PLATFORM_REALLOC(t->radios, sizeof(struct ChannelScanCapabilitiesRadio) * (t->radio_nr + 1));
		PLATFORM_MEMCPY(t->radios[t->radio_nr].radio_unique_identifier, x->mac_address, 6);
		t->radios[t->radio_nr].scan_capability_flags = 0;
		t->radios[t->radio_nr].scan_capability_flags |= ON_BOOT_ONLY_FALSE << 7;
		t->radios[t->radio_nr].scan_capability_flags |= UNAVAILABLE_FOR_MORE_THAN_2_SECONDS << 5;
		t->radios[t->radio_nr].minimum_scan_interval = 10; //hard code 10 for now

		// If ON_BOOT_ONLY (scan_capability_flags bit 7) == 1, SCAN_IMPACT (scan_capabilitiy_flags bits 6-5) = 0x00
		if(!(t->radios[t->radio_nr].scan_capability_flags & ON_BOOT_ONLY_TRUE)){
			t->radios[t->radio_nr].scan_capability_flags &= NO_IMPACT << 5;
		}

		PLATFORM_FREE_1905_INTERFACE_INFO(x);

		INT8U band = 0, sub_band = 0;
		if (PLATFORM_STRSTR(interfaces_names[i], map_radio_name_5gh) && PLATFORM_STRSTR(interfaces_names[i], map_radio_name_5gl)) {
			band = _5G;
		} else if (PLATFORM_STRSTR(interfaces_names[i], map_radio_name_5gh)) {
			band     = _5G;
			sub_band = _5GH;
		} else if (PLATFORM_STRSTR(interfaces_names[i], map_radio_name_5gl)) {
			band     = _5G;
			sub_band = _5GL;
		} else if (PLATFORM_STRSTR(interfaces_names[i], map_radio_name_24g)) {
			band = _2G;
		} else {
			PLATFORM_PRINTF_ERROR("[RTK] [%s] unknown interface: %s\n", __FUNCTION__, interfaces_names[i]);
			continue;
		}

		t->radios[t->radio_nr].op_class_nr = 0;
		t->radios[t->radio_nr].op_classes  = NULL;
		for (j = 0; j < op_classes_len; j++) {
			if (band == op_classes[j].band && (sub_band == op_classes[j].sub_band || 0 == sub_band)) {
				t->radios[t->radio_nr].op_classes                                                  = (struct ChannelScanCapabilitiesOpClass *)PLATFORM_REALLOC(t->radios[t->radio_nr].op_classes, sizeof(struct ChannelScanCapabilitiesOpClass) * (t->radios[t->radio_nr].op_class_nr + 1));
				t->radios[t->radio_nr].op_classes[t->radios[t->radio_nr].op_class_nr].op_class     = op_classes[j].op_class;
				t->radios[t->radio_nr].op_classes[t->radios[t->radio_nr].op_class_nr].channel_nr   = op_classes[j].channel_array[0];
				t->radios[t->radio_nr].op_classes[t->radios[t->radio_nr].op_class_nr].channel_list = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * op_classes[j].channel_array[0]);
				PLATFORM_MEMCPY(t->radios[t->radio_nr].op_classes[t->radios[t->radio_nr].op_class_nr].channel_list, &op_classes[j].channel_array[0] + 1, op_classes[j].channel_array[0]);
				//add 1 to op_class count
				t->radios[t->radio_nr].op_class_nr++;
			}
		}

		//add 1 to radio count
		t->radio_nr++;
	}

	PLATFORM_FREE(op_classes);
}

void _freeChannelScanCapabilitiesTLV(struct ChannelScanCapabilitiesTLV *t)
{

	INT8U k, l;

	for (k = 0; k < t->radio_nr; k++) {
		if (t->radios[k].op_class_nr) {
			for (l = 0; l < t->radios[k].op_class_nr; l++) {
				if (t->radios[k].op_classes[l].channel_nr) {
					PLATFORM_FREE(t->radios[k].op_classes[l].channel_list);
				}
			}
			PLATFORM_FREE(t->radios[k].op_classes);
		}
	}

	if (t->radio_nr) {
		PLATFORM_FREE(t->radios);
	}
}

void _obtainCACCapabilitiesTLV(struct CACCapabilitiesTLV *t)
{
	//obtain op_classes
	int       op_classes_len = 0;
	OP_CLASS *op_classes     = PLATFORM_GET_OP_CLASS(&op_classes_len);

	t->tlv_type = TLV_TYPE_CAC_CAPABILITIES;
	t->radio_nr = 0;
	t->radios   = NULL;

	if (NULL == op_classes) {
		PLATFORM_PRINTF_ERROR("[RTK] No operating class is available\n");
		return;
	}

	char *reg_domain = PLATFORM_GET_REGDOMAIN();
	PLATFORM_MEMCPY(t->country_code, reg_domain, 2);
	unsigned char cac_interval = 5;

	//obtain info
	char **interfaces_names;
	INT8U  interfaces_nr, i, j;
	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_nr);

	for (i = 0; i < interfaces_nr; i++) {
		struct interfaceInfo *x;
		INT32U cac_interval_total = 0;

		if (PLATFORM_STRSTR(interfaces_names[i], map_vxd_prefix) || PLATFORM_STRSTR(interfaces_names[i], map_vap_prefix) || PLATFORM_STRSTR(interfaces_names[i], "eth")) {
			continue; //skip non main root interfaces
		}

		x = PLATFORM_GET_1905_INTERFACE_INFO(interfaces_names[i]);

		if (NULL == x) {
			PLATFORM_PRINTF_ERROR("[MULTI_AP] Could not retrieve interface info for %s\n", interfaces_names[i]);
			continue;
		}

		// ignore the interface if it is not 5GHz AP
		if (!(IEEE80211_ROLE_AP == x->interface_type_data.ieee80211.role
		      && (INTERFACE_TYPE_IEEE_802_11A_5_GHZ == x->interface_type || INTERFACE_TYPE_IEEE_802_11N_5_GHZ == x->interface_type || INTERFACE_TYPE_IEEE_802_11AC_5_GHZ == x->interface_type || INTERFACE_TYPE_IEEE_802_11AD_60_GHZ == x->interface_type))) {
			PLATFORM_PRINTF_DEBUG("[MULTI_AP] The interface %s cannot operate on DFS channels.\n", interfaces_names[i]);
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
			continue;
		}

		//Start to fill in TLV
		t->radios         = (struct CACCapabilitiesRadio *)PLATFORM_REALLOC(t->radios, sizeof(struct CACCapabilitiesRadio) * (t->radio_nr + 1));
		PLATFORM_MEMCPY(t->radios[t->radio_nr].radio_unique_identifier, x->mac_address, 6);

		PLATFORM_FREE_1905_INTERFACE_INFO(x);

		INT8U band = 0, sub_band = 0;
		if (PLATFORM_STRSTR(interfaces_names[i], map_radio_name_5gh) && PLATFORM_STRSTR(interfaces_names[i], map_radio_name_5gl)) {
			band = _5G;
		} else if (PLATFORM_STRSTR(interfaces_names[i], map_radio_name_5gh)) {
			band     = _5G;
			sub_band = _5GH;
		} else if (PLATFORM_STRSTR(interfaces_names[i], map_radio_name_5gl)) {
			band     = _5G;
			sub_band = _5GL;
		} else {
			PLATFORM_PRINTF_ERROR("[RTK] [%s] unknown interface: %s\n", __FUNCTION__, interfaces_names[i]);
			continue;
		}

		t->radios[t->radio_nr].type_nr        = 1;
		t->radios[t->radio_nr].types          = (struct CACCapabilitiesType *)PLATFORM_MALLOC(1 * sizeof(struct CACCapabilitiesType));
		t->radios[t->radio_nr].types[0].flags = 2;
		// PLATFORM_MEMCPY(t->radios[t->radio_nr].types[0].second_nr, tmp_mac2, 3);
		t->radios[t->radio_nr].types[0].op_class_nr               = 0;
		t->radios[t->radio_nr].types[0].op_classes                = NULL;
		for (j = 0; j < op_classes_len; j++) {
			if (band == op_classes[j].band && (sub_band == op_classes[j].sub_band || 0 == sub_band)) {
				if((118 == op_classes[j].op_class) || (121 == op_classes[j].op_class) || (122 == op_classes[j].op_class) || (123 == op_classes[j].op_class) || (119 == op_classes[j].op_class) || (120 == op_classes[j].op_class)) {
					t->radios[t->radio_nr].types[0].op_classes                                                  = (struct CACCapabilitiesOpClass *)PLATFORM_REALLOC(t->radios[t->radio_nr].types[0].op_classes, sizeof(struct CACCapabilitiesOpClass) * (t->radios[t->radio_nr].types[0].op_class_nr + 1));
					t->radios[t->radio_nr].types[0].op_classes[t->radios[t->radio_nr].types[0].op_class_nr].op_class     = op_classes[j].op_class;
					t->radios[t->radio_nr].types[0].op_classes[t->radios[t->radio_nr].types[0].op_class_nr].channel_nr   = op_classes[j].channel_array[0];
					t->radios[t->radio_nr].types[0].op_classes[t->radios[t->radio_nr].types[0].op_class_nr].channels = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * op_classes[j].channel_array[0]);
					PLATFORM_MEMCPY(t->radios[t->radio_nr].types[0].op_classes[t->radios[t->radio_nr].types[0].op_class_nr].channels, &op_classes[j].channel_array[0] + 1, op_classes[j].channel_array[0]);
					//add 1 to op_class count
					t->radios[t->radio_nr].types[0].op_class_nr++;
					cac_interval_total += cac_interval * op_classes[j].channel_array[0];
				}
			}
		}
		t->radios[t->radio_nr].types[0].second_nr[0] = cac_interval_total >> 16;
		t->radios[t->radio_nr].types[0].second_nr[1] = cac_interval_total >> 8;
		t->radios[t->radio_nr].types[0].second_nr[2] = cac_interval_total;
		//add 1 to radio count
		t->radio_nr++;
	}
	PLATFORM_FREE(reg_domain);
	PLATFORM_FREE(op_classes);
}

void _freeCACCapabilitiesTLV(struct CACCapabilitiesTLV *t)
{

	INT8U k, l, m;

	for (k = 0; k < t->radio_nr; k++) {
		if (t->radios[k].type_nr) {
			for (l = 0; l < t->radios[k].type_nr; l++) {
				if (t->radios[k].types[l].op_class_nr) {
					for (m = 0; m < t->radios[k].types[l].op_class_nr; m++) {
						if (t->radios[k].types[l].op_classes[m].channel_nr) {
							PLATFORM_FREE(t->radios[k].types[l].op_classes[m].channels);
						}
					}
					PLATFORM_FREE(t->radios[k].types[l].op_classes);
				}
			}
			PLATFORM_FREE(t->radios[k].types);
		}
	}

	if (t->radio_nr) {
		PLATFORM_FREE(t->radios);
	}
}

void _obtainCACStatusTLV(struct CACStatusReportTLV *tlv)
{
	int                    i = 0, j = 0;
	struct RadioCACStatus *cac_status;
	uint32_t tmp_remaining_time;
	DMgetAllRadioCACStatus(&cac_status);
	(*tlv).tlv_type             = TLV_TYPE_CAC_STATUS_REPORT;
	(*tlv).active_pair_nr       = 0;
	(*tlv).active_pairs         = NULL;
	(*tlv).available_channel_nr = 0;
	(*tlv).available_channels   = NULL;
	(*tlv).nonoccup_pair_nr     = 0;
	(*tlv).nonoccup_pairs       = NULL;
	uint8_t radio_number        = DMisTribandDevice() + 2;
	for (i = 0; i < radio_number; i++) {
		for (j = 0; j < cac_status[i].cac_record_nr; j++) {
			struct CACStatusRecord *record = &cac_status[i].cac_records[j];
			switch (record->cac_status) {
			case CAC_ON_GOING: {
				(*tlv).active_pair_nr++;
				(*tlv).active_pairs                                                  = PLATFORM_REALLOC((*tlv).active_pairs, sizeof(struct CACCompletionReportActiveClassChannelPairs) * (*tlv).active_pair_nr);
				(*tlv).active_pairs[(*tlv).active_pair_nr - 1].active_channel        = record->channel;
				(*tlv).active_pairs[(*tlv).active_pair_nr - 1].active_op_class       = record->op_class;
				tmp_remaining_time = CAC_REQUIRED_TIME - ((PLATFORM_GET_TIMESTAMP() - record->last_timestamp) / 1000);
				(*tlv).active_pairs[(*tlv).active_pair_nr - 1].active_remaining_time[0] = (uint8_t) (tmp_remaining_time >> 16);
				(*tlv).active_pairs[(*tlv).active_pair_nr - 1].active_remaining_time[1] = (uint8_t) (tmp_remaining_time >> 8);
				(*tlv).active_pairs[(*tlv).active_pair_nr - 1].active_remaining_time[2] = (uint8_t) (tmp_remaining_time);
				break;
			}
			case CAC_COMPLETED: {
				(*tlv).available_channel_nr++;
				(*tlv).available_channels                                                = PLATFORM_REALLOC((*tlv).available_channels, sizeof(struct CACCompletionReportAvailabelChannels) * (*tlv).available_channel_nr);
				(*tlv).available_channels[(*tlv).available_channel_nr - 1].ac_channel    = record->channel;
				(*tlv).available_channels[(*tlv).available_channel_nr - 1].ac_op_class   = record->op_class;
				(*tlv).available_channels[(*tlv).available_channel_nr - 1].identify_time = (PLATFORM_GET_TIMESTAMP() - record->last_timestamp) / 1000 / 60;
				break;
			}
			case CAC_RADAR_DETECTED: {
				(*tlv).nonoccup_pair_nr++;
				(*tlv).nonoccup_pairs                                                      = PLATFORM_REALLOC((*tlv).nonoccup_pairs, sizeof(struct CACCompletionReportNonOccupancyClassChannelPairs) * (*tlv).nonoccup_pair_nr);
				(*tlv).nonoccup_pairs[(*tlv).nonoccup_pair_nr - 1].nonoccup_channel        = record->channel;
				(*tlv).nonoccup_pairs[(*tlv).nonoccup_pair_nr - 1].nonoccup_op_class       = record->op_class;
				(*tlv).nonoccup_pairs[(*tlv).nonoccup_pair_nr - 1].nonoccup_remaining_time = CAC_OCCUPIED_TIME - ((PLATFORM_GET_TIMESTAMP() - record->last_timestamp) / 1000);
				break;
			}
			default:
				break;
			}
		}
	}
}

void obtainTrafficSeparationPolicyTLV(struct TrafficSeparationPolicyTLV **tlv,
                                      struct configData *config[], INT8U config_nr)
{
	INT8U i;

	for (i = 0; i < config_nr; i++) {
		if (config[i]->vid) {
			// Allocate if it is not created before
			if (*tlv == NULL) {
				*tlv = (struct TrafficSeparationPolicyTLV *)PLATFORM_MALLOC(sizeof(struct TrafficSeparationPolicyTLV));
				PLATFORM_MEMSET(*tlv, 0, sizeof(struct TrafficSeparationPolicyTLV));
				(*tlv)->tlv_type = TLV_TYPE_TRAFFIC_SEPARATION_POLICY;
			}
			// Realloc for reserving existing content
			(*tlv)->ssid_nr++;
			(*tlv)->ssid_info = (struct TrafficSeparationSsidInfo *)
			    PLATFORM_REALLOC((*tlv)->ssid_info, sizeof(struct TrafficSeparationSsidInfo) * (*tlv)->ssid_nr);
			// Append the VID-SSID matching
			(*tlv)->ssid_info[(*tlv)->ssid_nr - 1].vlan_id     = config[i]->vid;
			(*tlv)->ssid_info[(*tlv)->ssid_nr - 1].ssid_length = PLATFORM_STRLEN(config[i]->ssid);
			(*tlv)->ssid_info[(*tlv)->ssid_nr - 1].ssid_name   = PLATFORM_STRDUP(config[i]->ssid);
		}
	}
}
#endif

#ifdef RTK_MULTI_AP_R3
//******************************************************************************
//* Profile 3 TLV Filling Up Functions
//******************************************************************************
void buildDPPChirpValueTLV(struct DPPChirpValueTLV *t, INT8U *enrollee_mac, INT8U *chirp_content, INT16U chirp_len)
{
	t->tlv_type      = TLV_TYPE_DPP_CHIRP_VALUE;
	t->hash_validity = DPP_CHIRP_HASH_VALIDITY;

	if (enrollee_mac) {
		t->hash_validity |= DPP_CHIRP_ENROLLEE_ADDR_PRESENT;
		PLATFORM_MEMCPY(t->sta_mac_address, enrollee_mac, 6);
	} else {
		PLATFORM_MEMSET(t->sta_mac_address, 0, 6);
	}

	t->hash_len   = chirp_len;
	t->hash_value = (INT8U *)PLATFORM_MALLOC(t->hash_len);
	PLATFORM_MEMCPY(t->hash_value, chirp_content, t->hash_len);
}

void _obtainDPPChirpValueTLV(struct DPPChirpValueTLV *t, INT8U own_chirp, INT8U *enrollee_mac)
{
	struct dpp_authentication *dpp_auth = NULL;
	struct dpp_bootstrap_info *bi       = NULL;

	dpp_auth = DMgetDPPAuthenticationObject();

	if (own_chirp)
		bi = dpp_auth->own_bi;
	else
		bi = &(dpp_auth->peer_bi);

	// Generate TLV
	t->tlv_type      = TLV_TYPE_DPP_CHIRP_VALUE;
	t->hash_validity = DPP_CHIRP_HASH_VALIDITY;
	if (enrollee_mac) {
		t->hash_validity |= DPP_CHIRP_ENROLLEE_ADDR_PRESENT;
		PLATFORM_MEMCPY(t->sta_mac_address, enrollee_mac, 6);
	}
	t->hash_len   = sizeof(bi->pubkey_hash_chirp);
	t->hash_value = PLATFORM_MALLOC(t->hash_len);
	PLATFORM_MEMCPY(t->hash_value, bi->pubkey_hash_chirp, t->hash_len);
}

void _obtainDPPMessageTLV(struct DPPMessageTLV *t, INT8U *dpp_frame, INT16U dpp_frame_len)
{
	// Generate TLV
	t->tlv_type      = TLV_TYPE_DPP_MESSAGE;
	t->dpp_frame_len = dpp_frame_len;
	t->dpp_frame     = PLATFORM_MALLOC(t->dpp_frame_len);
	PLATFORM_MEMCPY(t->dpp_frame, dpp_frame, t->dpp_frame_len);
}

void _obtain1905EncapEAPOLTLV(struct Encap1905EAPOLTLV *t, INT8U *eapol_frame, INT16U eapol_frame_len)
{
	// Generate TLV
	t->tlv_type                = TLV_TYPE_1905_ENCAP_EAPOL;
	t->eapol_frame_payload_len = eapol_frame_len;
	t->eapol_frame_payload     = PLATFORM_MALLOC(t->eapol_frame_payload_len);
	PLATFORM_MEMCPY(t->eapol_frame_payload, eapol_frame, t->eapol_frame_payload_len);
}

void _obtainSecurityCapabilityTLV(struct SecurityCapabilityTLV *security_capability_tlv)
{
	security_capability_tlv->tlv_type                               = TLV_TYPE_1905_LAYER_SECURITY_CAPABILITY;
	security_capability_tlv->message_encryption_algorithm_supported = ENCRYTPTION_AES_SIV;
	security_capability_tlv->message_integrity_algorithm_supported  = ALGORITHM_HMAC_SHA254;
	security_capability_tlv->onboarding_protocol_supported          = PROTOCOL_1905_DEVICE_PROVISIONING;
}

int _obtainAPWiFi6CapabilitiesTLV(struct APWiFi6CapabilitiesTLV *ap_wifi6_capabilities_tlv, char *interface_name)
{
	INT8U  tempbuf[128];
	INT8U  i, j, mcs_nss_nr;
	INT8U *p;

	ap_wifi6_capabilities_tlv->tlv_type = TLV_TYPE_AP_WIFI_6_CAPABILITIES;
	INT8U *radio_uid                    = DMinterfaceNameToMac(interface_name);
	PLATFORM_MEMCPY(ap_wifi6_capabilities_tlv->radio_unique_identifier, radio_uid, 6);
	ap_wifi6_capabilities_tlv->role_nr = 0;

	i = PLATFORM_AP_CAPABILITY_IOCTL(interface_name, 4, tempbuf);

	if (i > 0) {
		// The tempbuf is returned from calling PLATFORM_AP_CAPABILITY_IOCTL
		// Based on how the structure of the tempbuf, the values are assigned accordingly
		// tempbuf[9] contains number of role
		p = &tempbuf[9];
		_E1B(&p, &ap_wifi6_capabilities_tlv->role_nr);
		PLATFORM_PRINTF_DEBUG("%s role_nr : %d\n", __FUNCTION__, ap_wifi6_capabilities_tlv->role_nr);
		// The next octets depends on the number of agent role as well as the number of mcs nss
		// tempbuf[10] 						= from Agent Role to MCS NSS Length
		// tempbuf[11-14 / 11-18 / 11-22] 	= MCS NSS (for 4/8/12)
		// tempbuf[15 / 19 / 23] 			= SU Beamformer to DL OFDMA
		// tempbuf[16 / 20 / 24] 			= Max DL MU-MIMO TX and Max UL MU-MIMO RX
		// tempbuf[17 / 21 / 25] 			= Max DL OFDMA TX
		// tempbuf[18 / 22 / 26] 			= Max UL OFDMA RX
		// tempbuf[19 / 23 / 27] 			= RTS to TWT Responder
		// repeated again in buff 20 / 24 / 28 onwards if role > 1
		ap_wifi6_capabilities_tlv->agent_roles = PLATFORM_MALLOC(sizeof(struct AgentRole) * ap_wifi6_capabilities_tlv->role_nr);
		for (j = 0; j < ap_wifi6_capabilities_tlv->role_nr; j++) {
			_E1B(&p, &ap_wifi6_capabilities_tlv->agent_roles[j].flags);
			mcs_nss_nr                                        = (ap_wifi6_capabilities_tlv->agent_roles[j].flags & MCS_NSS_NUMBER_MASK); // mcs nss len = bits 3-0 of flags
			ap_wifi6_capabilities_tlv->agent_roles[j].mcs_nss = (INT8U *)PLATFORM_MALLOC(mcs_nss_nr);
			_EnB(&p, ap_wifi6_capabilities_tlv->agent_roles[j].mcs_nss, mcs_nss_nr);
			_E1B(&p, &ap_wifi6_capabilities_tlv->agent_roles[j].wifi6_capability_1);
			_E1B(&p, &ap_wifi6_capabilities_tlv->agent_roles[j].wifi6_capability_2);
			_E1B(&p, &ap_wifi6_capabilities_tlv->agent_roles[j].wifi6_capability_3);
			_E1B(&p, &ap_wifi6_capabilities_tlv->agent_roles[j].wifi6_capability_4);
			_E1B(&p, &ap_wifi6_capabilities_tlv->agent_roles[j].wifi6_capability_5);
			PLATFORM_PRINTF_DEBUG("%s agent_role[%d].flags : %x\n", __FUNCTION__, j, ap_wifi6_capabilities_tlv->agent_roles[j].flags);
			PLATFORM_PRINTF_DEBUG("%s agent_role[%d].mcs_nss : %02x%02x%02x%02x\n", __FUNCTION__, j, ap_wifi6_capabilities_tlv->agent_roles[j].mcs_nss[0],
			                      ap_wifi6_capabilities_tlv->agent_roles[j].mcs_nss[1],
			                      ap_wifi6_capabilities_tlv->agent_roles[j].mcs_nss[2],
			                      ap_wifi6_capabilities_tlv->agent_roles[j].mcs_nss[3]);
			PLATFORM_PRINTF_DEBUG("%s agent_role[%d].wifi6_capability_1 : %x\n", __FUNCTION__, j, ap_wifi6_capabilities_tlv->agent_roles[j].wifi6_capability_1);
			PLATFORM_PRINTF_DEBUG("%s agent_role[%d].wifi6_capability_2 : %x\n", __FUNCTION__, j, ap_wifi6_capabilities_tlv->agent_roles[j].wifi6_capability_2);
			PLATFORM_PRINTF_DEBUG("%s agent_role[%d].wifi6_capability_3 : %x\n", __FUNCTION__, j, ap_wifi6_capabilities_tlv->agent_roles[j].wifi6_capability_3);
			PLATFORM_PRINTF_DEBUG("%s agent_role[%d].wifi6_capability_4 : %x\n", __FUNCTION__, j, ap_wifi6_capabilities_tlv->agent_roles[j].wifi6_capability_4);
			PLATFORM_PRINTF_DEBUG("%s agent_role[%d].wifi6_capability_5 : %x\n", __FUNCTION__, j, ap_wifi6_capabilities_tlv->agent_roles[j].wifi6_capability_5);
		}

		return 1;
	}
	return 0;
}

int _obtainAssociatedWiFi6StaStatusReportTLV(struct AssociatedWiFi6STAStatusReportTLV *assoc_wifi6_sta_status_report_tlv, char *interface_name, INT8U *macaddr){
	INT8U tempbuf[2048];
	INT8U i, j;
	INT8U *p;

	assoc_wifi6_sta_status_report_tlv->tlv_type = TLV_TYPE_ASSOCIATED_WIFI_6_STA_STATUS_REPORT;
	PLATFORM_MEMCPY(assoc_wifi6_sta_status_report_tlv->mac_address, macaddr, 6);
	assoc_wifi6_sta_status_report_tlv->queue_nr = 0;

	if(interface_name == NULL){
		return 0;
	}

	i = PLATFORM_ASSOC_STA_IOCTL(interface_name, 3, macaddr, (INT8U *)&tempbuf);

	if(i > 0){
		// The tempbuf is returned from calling PLATFORM_ASSOC_STA_IOCTL
		// Based on how the structure of the tempbuf, the values are assigned accordingly
		// tempbuf[9] contains number of queue
		p = &tempbuf[9];
		_E1B(&p, &assoc_wifi6_sta_status_report_tlv->queue_nr);
		assoc_wifi6_sta_status_report_tlv->queues = PLATFORM_MALLOC(sizeof(struct Queue) * assoc_wifi6_sta_status_report_tlv->queue_nr);
		for(j = 0 ; j<assoc_wifi6_sta_status_report_tlv->queue_nr ; j++){
			_E1B(&p, &assoc_wifi6_sta_status_report_tlv->queues[i].tid);
			_E1B(&p, &assoc_wifi6_sta_status_report_tlv->queues[i].queue_size);
		}
		i = 1; // successful
	} else{
		PLATFORM_PRINTF_WARNING("%s - Cannot retrieve STA %02x%02x%02x%02x%02x%02x info\n", __FUNCTION__,
								macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
		i = 0; // failed
	}

	return i;
}

void _obtainDeviceInventoryTLV(struct DeviceInventoryTLV *device_inventory_tlv)
{
	struct deviceInfo device_info;
	PLATFORM_MEMSET(&device_info, 0, sizeof(device_info));
	char **interfaces_names;
	INT8U  interfaces_nr, i;
	INT8U  interface_mac[6] = {0};

	device_inventory_tlv->tlv_type         = TLV_TYPE_DEVICE_INVENTORY;
	device_inventory_tlv->lsn              = 0;
	device_inventory_tlv->serial_number    = NULL;
	device_inventory_tlv->lsv              = 0;
	device_inventory_tlv->software_version = NULL;
	device_inventory_tlv->lee              = 0;
	device_inventory_tlv->execution_env    = NULL;
	device_inventory_tlv->radio_nr         = 0;
	device_inventory_tlv->radios           = NULL;

	// Obtain device info
	PLATFORM_GET_DEVICE_INFO(&device_info);

	device_inventory_tlv->lsn           = strlen(device_info.serial_number);
	device_inventory_tlv->serial_number = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * device_inventory_tlv->lsn);
	PLATFORM_MEMCPY(device_inventory_tlv->serial_number, device_info.serial_number, device_inventory_tlv->lsn);

	device_inventory_tlv->lsv              = strlen(device_info.software_version);
	device_inventory_tlv->software_version = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * device_inventory_tlv->lsv);
	PLATFORM_MEMCPY(device_inventory_tlv->software_version, device_info.software_version, device_inventory_tlv->lsv);

	device_inventory_tlv->lee           = strlen(device_info.execution_env);
	device_inventory_tlv->execution_env = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * device_inventory_tlv->lee);
	PLATFORM_MEMCPY(device_inventory_tlv->execution_env, device_info.execution_env, device_inventory_tlv->lee);

	// Obtain radio info
	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_nr);

	for (i = 0; i < interfaces_nr; i++) {

		if (PLATFORM_STRSTR(interfaces_names[i], map_vxd_prefix)
		    || PLATFORM_STRSTR(interfaces_names[i], map_vap_prefix)
		    || PLATFORM_GET_VALID_INTERFACE_NAME(interfaces_names[i], VALID_ETH_INTERFACES_IN_BRIDGE)) {
			continue; // Skip non main root interfaces
		}

		if (0 == PLATFORM_GET_MAC(interfaces_names[i], interface_mac)) {
			PLATFORM_PRINTF_ERROR("[MULTI_AP] Could not get mac for %s\n", interfaces_names[i]);
			continue;
		}

		// Start to fill in TLV
		device_inventory_tlv->radios = (struct DeviceInventoryRadio *)PLATFORM_REALLOC(device_inventory_tlv->radios, sizeof(struct DeviceInventoryRadio) * (device_inventory_tlv->radio_nr + 1));
		PLATFORM_MEMCPY(device_inventory_tlv->radios[device_inventory_tlv->radio_nr].radio_unique_identifier, interface_mac, 6);
		device_inventory_tlv->radios[device_inventory_tlv->radio_nr].lcv            = strlen(device_info.manufacturer_name);
		device_inventory_tlv->radios[device_inventory_tlv->radio_nr].chipset_vendor = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * device_inventory_tlv->radios[device_inventory_tlv->radio_nr].lcv);
		PLATFORM_MEMCPY(device_inventory_tlv->radios[device_inventory_tlv->radio_nr].chipset_vendor, device_info.manufacturer_name, strlen(device_info.manufacturer_name));
		device_inventory_tlv->radio_nr += 1;
	}
}

void _obtainAKMSuiteCapabilitiesTLV(struct AKMSuiteCapabilitiesTLV *akm_suite_tlv)
{
	int i;

	// Hard-coded values for AKM
	// TODO: use wiphy akm_suites from driver
	const struct AKMSuiteSelector selectors[] = {
		{ { 0x00, 0x0F, 0xAC }, 0x02 }, // WLAN_AKM_SUITE_PSK
		{ { 0x00, 0x0F, 0xAC }, 0x06 }, // WLAN_AKM_SUITE_PSK_SHA256
		{ { 0x00, 0x0F, 0xAC }, 0x08 }, // WLAN_AKM_SUITE_SAE
	};
	const int selectors_nr = sizeof(selectors) / sizeof(struct AKMSuiteSelector);

	// Assign selector values
	akm_suite_tlv->tlv_type                            = TLV_TYPE_AKM_SUITE_CAPABILITIES;
	akm_suite_tlv->backhaul_bss_akm_suite_selectors_nr = selectors_nr;
	akm_suite_tlv->backhaul_bss_akm_suite_selectors    = PLATFORM_MALLOC(sizeof(selectors));
	for (i = 0; i < selectors_nr; i++)
		akm_suite_tlv->backhaul_bss_akm_suite_selectors[i] = selectors[i];
	akm_suite_tlv->fronthaul_bss_akm_suite_selectors_nr = selectors_nr;
	akm_suite_tlv->fronthaul_bss_akm_suite_selectors    = PLATFORM_MALLOC(sizeof(selectors));
	for (i = 0; i < selectors_nr; i++)
		akm_suite_tlv->fronthaul_bss_akm_suite_selectors[i] = selectors[i];
}

void _obtainEncryptedPayloadTLV(struct EncryptedPayloadTLV *t, INT8U *dst_mac_address, INT8U* ciphertext, INT16U ciphertext_len)
{
	if (!t) {
		PLATFORM_PRINTF_ERROR("Invalid EncryptedPayloadTLV, please check error! \n");
		return;
	}

	t->tlv_type = TLV_TYPE_ENCRYPTED_PAYLOAD;

	INT64U transmission_counter = dpp_counters_get_etc(dst_mac_address);
	t->encryption_transmission_counter = transmission_counter;

	PLATFORM_MEMCPY(t->source_1905_mac_address, DMalMacGet(), 6);
	PLATFORM_MEMCPY(t->dest_1905_mac_address, dst_mac_address, 6);

	t->aes_siv_len = ciphertext_len;
	t->aes_siv     = PLATFORM_MALLOC(t->aes_siv_len);
	PLATFORM_MEMCPY(t->aes_siv, ciphertext, t->aes_siv_len);
}

void _obtainBSSConfigRequestTLV(struct BSSConfigRequestTLV *bss_config_req_tlv)
{
	struct dpp_config_req_obj req_obj;
	INT8U *                   req_obj_attr     = NULL;
	INT16U                    req_obj_attr_len = 0;

	// Obtain Config Request Object JSON
	PLATFORM_MEMSET(&req_obj, 0, sizeof(struct dpp_config_req_obj));
	strlcpy(req_obj.name, DMgetDeviceName(), sizeof(req_obj.name));
	strlcpy(req_obj.wifi_tech, "map", sizeof(req_obj.wifi_tech));
	req_obj.netRole = DPP_NETROLE_MAP_AGENT;
	if (dpp_build_map_bss_conf_req(&req_obj, &req_obj_attr, &req_obj_attr_len) != 1) {
		PLATFORM_PRINTF_WARNING("[%s] Build Config Request Object Failed!\n", __FUNCTION__);
		return;
	}

	// Assign values to TLV
	bss_config_req_tlv->tlv_type                      = TLV_TYPE_BSS_CONFIG_REQUEST;
	bss_config_req_tlv->dpp_config_request_object_len = req_obj_attr_len;
	bss_config_req_tlv->dpp_config_request_object     = req_obj_attr;
}

void _obtainBSSConfigResponseTLV(struct BSSConfigResponseTLV *bss_config_resp_tlv,
                                 struct dpp_config_obj conf_objs[], INT8U conf_objs_nr)
{
	struct dpp_authentication *auth;

	INT8U *config_obj_attr;
	INT16U config_obj_attr_len = 0;

	// Obtain Config Response Object JSON
	PLATFORM_MEMSET(bss_config_resp_tlv, 0, sizeof(struct BSSConfigResponseTLV));
	auth = DMgetDPPAuthenticationObject();
	if (dpp_build_map_bss_conf_resp(auth, conf_objs, conf_objs_nr,
	                                &config_obj_attr, &config_obj_attr_len)
	    != 1) {
		PLATFORM_PRINTF_WARNING("[%s] Build Config Resp Object Failed!\n", __FUNCTION__);
		return;
	}

	// Assign values to TLV
	bss_config_resp_tlv->tlv_type              = TLV_TYPE_BSS_CONFIG_RESPONSE;
	bss_config_resp_tlv->dpp_config_object_len = config_obj_attr_len;
	bss_config_resp_tlv->dpp_config_object     = config_obj_attr;
}

void _obtainBSSConfigReportTLV(struct BSSConfigReportTLV *tlv)
{
	char **interfaces_names;
	INT8U  interfaces_nr, i, j;

	char *bss_reports_intf[] = { map_radio_name_24g,
		                         map_radio_name_5gl,
		                         map_radio_name_5gh };

	struct BSSConfigBSSReport *bss_reports[3]    = { 0 };
	INT8U                      bss_reports_nr[3] = { 0 };

	// Fetch info of all interfaces
	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_nr);
	for (i = 0; i < interfaces_nr; i++) {
		struct interfaceInfo *info;
		if ((info = PLATFORM_GET_1905_INTERFACE_INFO(interfaces_names[i])) == NULL) {
			PLATFORM_PRINTF_ERROR("Could not retrieve interface info for %s\n", interfaces_names[i]);
			continue;
		}

		// Ignore vxd / power off / non 802.11 / non AP interfaces
		if (info->power_state == INTERFACE_POWER_STATE_OFF
		    || PLATFORM_STRSTR(info->name, map_vxd_prefix)
		    || !IS_IEEE_802_11_INTERFACE(info->interface_type)
		    || !(IEEE80211_ROLE_AP == info->interface_type_data.ieee80211.role)) {
			PLATFORM_PRINTF_DETAIL("Ignore interface %s in BSS config report\n", interfaces_names[i]);
			PLATFORM_FREE_1905_INTERFACE_INFO(info);
			continue;
		}

		// Append the BSS report to corresponding bss report list
		for (j = 0; j < sizeof(bss_reports_nr); j++) {
			if (PLATFORM_STRSTR(info->name, bss_reports_intf[j])) {
				bss_reports_nr[j]++;
				bss_reports[j] = (struct BSSConfigBSSReport *)
				    PLATFORM_REALLOC(bss_reports[j], sizeof(struct BSSConfigBSSReport) * bss_reports_nr[j]);

				// Copy interface info to bss report
				struct BSSConfigBSSReport *bss_report = &bss_reports[j][bss_reports_nr[j] - 1];

				// BSSID
				PLATFORM_MEMCPY(bss_report->bssid, info->interface_type_data.ieee80211.bssid, 6);

				// Status Flags
				bss_report->status_flag = 0;
				if (info->interface_type_data.ieee80211.network_type != MULTI_AP_FRONTHAUL_BSS)
					bss_report->status_flag |= BSS_CONFIG_FRONTHAUL_BSS;
				if (info->interface_type_data.ieee80211.network_type != MULTI_AP_BACKHAUL_BSS)
					bss_report->status_flag |= BSS_CONFIG_BACKHAUL_BSS;

				// Reserved
				bss_report->reserved = 0;

				// SSID
				bss_report->ssid        = PLATFORM_STRDUP(info->interface_type_data.ieee80211.ssid);
				bss_report->ssid_length = PLATFORM_STRLEN(info->interface_type_data.ieee80211.ssid);

				PLATFORM_PRINTF_DETAIL("Created BSS Report for interface: %s\n", info->name);
				PLATFORM_PRINTF_DETAIL(" SSID: %s\n", info->interface_type_data.ieee80211.ssid);
				PLATFORM_PRINTF_DETAIL(" Network Type: %02X\n", info->interface_type_data.ieee80211.network_type);
				break;
			}
		}

		PLATFORM_FREE_1905_INTERFACE_INFO(info);
	}

	// Construct BSS Config Report TLV
	PLATFORM_MEMSET(tlv, 0, sizeof(struct BSSConfigReportTLV));
	tlv->tlv_type = TLV_TYPE_BSS_CONFIG_REPORT;
	for (i = 0; i < sizeof(bss_reports_nr); i++) {
		if (bss_reports_nr[i]) {
			tlv->radio_report_nr++;
			tlv->radio_reports = (struct BSSConfigRadioReport *)
			    PLATFORM_REALLOC(tlv->radio_reports, sizeof(struct BSSConfigRadioReport) * tlv->radio_report_nr);

			// Copy radio ruid and bss reports of the radio
			struct BSSConfigRadioReport *radio_report = &tlv->radio_reports[tlv->radio_report_nr - 1];
			PLATFORM_MEMCPY(radio_report->radio_unique_identifier, DMinterfaceNameToMac(bss_reports_intf[i]), 6);
			radio_report->bss_report_nr = bss_reports_nr[i];
			radio_report->bss_reports   = bss_reports[i];
		}
	}
}

void _obtainBackhaulSTARadioCapabilitiesTLV(struct BackhaulSTARadioCapabilitiesTLV **tlv, INT8U* tlv_nr) {
	char **interfaces_names;
	INT8U  interfaces_nr, i, bh_sta_found = 0;
	INT8U backhaul_sta_mac_address[6];
	struct BackhaulSTARadioCapabilitiesTLV *tlvs = NULL;

	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_nr);

	// Check for backhaul STA, do not return any TLV if theres no backhaul sta
	for (i = 0; i < interfaces_nr; i++) {
		struct LocalInterface *local_interface;

		local_interface = DMnameToLocalInterfaceStruct(interfaces_names[i]);
		if (local_interface && PLATFORM_STRSTR(interfaces_names[i], map_vxd_prefix) && (local_interface->role & MULTI_AP_BACKHAUL_STA)) {
			PLATFORM_MEMCPY(backhaul_sta_mac_address,local_interface->mac_address,6);
			bh_sta_found = 1;
		}
	}
	if (!bh_sta_found) {
		PLATFORM_FREE_LIST_OF_1905_INTERFACES(interfaces_names, interfaces_nr);
		return;
	}
	// Make as many tlvs as there are root interfaces
	for (i = 0; i < interfaces_nr; i++) {
		struct LocalInterface *local_interface;

		if (PLATFORM_STRSTR(interfaces_names[i], map_vxd_prefix) || PLATFORM_STRSTR(interfaces_names[i], map_vap_prefix) || PLATFORM_STRSTR(interfaces_names[i], "eth")) {
			continue; //skip non main root interfaces
		}
		local_interface = DMnameToLocalInterfaceStruct(interfaces_names[i]);
		if (local_interface != NULL) {
			// For each root interface, allocate a larger array
			if (0 == *tlv_nr) {
				tlvs = (struct BackhaulSTARadioCapabilitiesTLV *)PLATFORM_MALLOC(sizeof(struct BackhaulSTARadioCapabilitiesTLV));
			} else {
				tlvs = (struct BackhaulSTARadioCapabilitiesTLV *)PLATFORM_REALLOC(tlvs, sizeof(struct BackhaulSTARadioCapabilitiesTLV) * (*tlv_nr + 1));
			}

			// Construct the tlv for this root interface
			tlvs[*tlv_nr].tlv_type = TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES;
			PLATFORM_MEMCPY(tlvs[*tlv_nr].radio_unique_identifier,local_interface->mac_address,6);
			tlvs[*tlv_nr].included_mac = 1 << 7;
			PLATFORM_MEMCPY(tlvs[*tlv_nr].sta_mac_address,backhaul_sta_mac_address,6);
			(*tlv_nr)++;
		}
	}
	PLATFORM_FREE_LIST_OF_1905_INTERFACES(interfaces_names, interfaces_nr);
	*tlv = tlvs;
}

INT8U _insertMICTLV(struct CMDU *cmdu, INT8U *src_1905_al_mac_addr, struct MICTLV *mic_tlv)
{
	INT16U tlv_count = 0;

	struct dpp_gtk *gtk = dpp_keystore_get_gtk();

	if (!gtk) {
		PLATFORM_PRINTF_WARNING("No GTK is found. Skip inserting MIC\n");
		return 0;
	}

	// Construct TLV
	//
	mic_tlv->tlv_type       = TLV_TYPE_MIC;
	mic_tlv->gtk_key_id_ver = 0x40 | MIC_VERSION_1;
	PLATFORM_MEMCPY(mic_tlv->src_1905_al_mac_addr, src_1905_al_mac_addr, 6);
	mic_tlv->integrity_transmission_counter = DMgetNextITC();

	// Calculate MIC
	//
	if (calculate_mic(gtk->gtk, cmdu, 0, mic_tlv)) {
		PLATFORM_PRINTF_WARNING("GTK calculation failed. Skip inserting MIC\n");
		return 0;
	}

	// Insert TLV at the end
	//
	while (NULL != cmdu->list_of_TLVs[tlv_count]) {
		tlv_count++;
	}
	cmdu->list_of_TLVs              = (INT8U **)PLATFORM_REALLOC(cmdu->list_of_TLVs, sizeof(INT8U *) * (tlv_count + 2));
	cmdu->list_of_TLVs[tlv_count++] = (INT8U *)mic_tlv;
	cmdu->list_of_TLVs[tlv_count]   = NULL;

	PLATFORM_PRINTF_DEBUG("Inserted MIC TLV. Total TLVs count: %d\n", tlv_count);
	return tlv_count - 1;
}
#endif

#ifdef RTK_MULTI_AP_R4
INT8U _obtainSpatialReuseConfigResponseTLV(struct SpatialReuseConfigResponseTLV **t)
{
	struct SpatialReuseConfigResponseTLV *spatial_reuse_response    = NULL;
	INT8U                                 spatial_reuse_response_nr = 0;

	int                                   i;
	struct SpatialReuseConfigResponseTLV *tlvs = NULL;

	PLATFORM_MAP_GET_SPATIAL_REUSE_CONFIG_RESPONSE(&spatial_reuse_response, &spatial_reuse_response_nr);

	tlvs = (struct SpatialReuseConfigResponseTLV *)PLATFORM_MALLOC(sizeof(struct SpatialReuseConfigResponseTLV) * spatial_reuse_response_nr);

	for (i = 0; i < spatial_reuse_response_nr; i++) {
		tlvs[i].tlv_type = TLV_TYPE_SPATIAL_REUSE_CONFIG_RESPONSE;
		PLATFORM_MEMCPY(tlvs[i].ruid, spatial_reuse_response[i].ruid, 6);
		tlvs[i].response_code = spatial_reuse_response[i].response_code;
	}

	// Free the data from datamodel

	*t = tlvs;
	return spatial_reuse_response_nr;
}

INT8U _collectSpatialReuseReportTLVs(struct SpatialReuseReportTLV ***t)
{
	INT8U                          spatial_reuse_report_nr = 0;
	int                            i, j;
	struct SpatialReuseReportTLV **tlvs = NULL;
	//only give root interface info
	int    radio_names_nr = 0;
	char **radio_names    = DMgetRadioNames(&radio_names_nr);
	int    radio_valid[3] = {0,0,0}; // 1 is valid, 0 is invalid

	// Get all interfaces
	for (i = 0; i < radio_names_nr; i++) {
		struct interfaceInfo *x;
		x = PLATFORM_GET_1905_INTERFACE_INFO(radio_names[i]);
		if (NULL == x) {
			PLATFORM_PRINTF_ERROR("[MULTI_AP] Could not retrieve interface info for %s\n", radio_names[i]);
			continue;
		}

		if (PLATFORM_INTERFACE_HAS_AX_SUPPORT(radio_names[i])) {
			PLATFORM_PRINTF_DETAIL("[MULTI_AP] The interface %s support the wifi6.\n", radio_names[i]);
		} else {
			PLATFORM_PRINTF_ERROR("[MULTI_AP] interface %s not support the wifi6\n", radio_names[i]);
			continue;
		}

		// ignore the interface if it is not an AP
		if (!(IEEE80211_ROLE_AP == x->interface_type_data.ieee80211.role
		      && IS_IEEE_802_11_INTERFACE(x->interface_type))) {
			PLATFORM_PRINTF_DETAIL("[MULTI_AP] The interface %s is not operating a BSS.\n", radio_names[i]);
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
			continue;
		} else {
			PLATFORM_PRINTF_DETAIL("[MULTI_AP] The interface %s is operating a BSS.\n", radio_names[i]);
		}
		PLATFORM_FREE_1905_INTERFACE_INFO(x);
		spatial_reuse_report_nr++;
		radio_valid[i] = 1;
	}

	PLATFORM_PRINTF_DETAIL("[MULTI_AP] Prepare to get [%d] number of spatial reuse repot tlv.\n", spatial_reuse_report_nr);
	// Obtain the tlv and add to list
	tlvs          = (struct SpatialReuseReportTLV **)PLATFORM_MALLOC(sizeof(struct SpatialReuseReportTLV *) * (spatial_reuse_report_nr));
	j             = 0;
	INT8U *buffer = NULL;
	INT8U *parsed = NULL;
	buffer        = (INT8U *)PLATFORM_MALLOC(sizeof(struct SpatialReuseReportTLV) + 2); // struct not contains tlv_length, need add 2 more bytes
	for (i = 0; i < radio_names_nr; i++) {
		if (!radio_valid[i]) {
			PLATFORM_PRINTF_DETAIL("[MULTI_AP] The interface %s is not operating a BSS.\n", radio_names[i]);
			continue;
		}
		PLATFORM_MEMSET(buffer, 0, sizeof(struct SpatialReuseReportTLV) + 2);
		PLATFORM_MAP_GET_SPATIAL_REUSE_REPORT(buffer, radio_names[i]);
		parsed = parse_1905_TLV_from_packet(buffer);
		if (parsed != NULL) {
			if (*parsed == TLV_TYPE_SPATIAL_REUSE_REPORT) {
				tlvs[j] = (struct SpatialReuseReportTLV *)parsed;
				j++;
			} else {
				PLATFORM_FREE(parsed);
				PLATFORM_PRINTF_DETAIL("Spatial Reuse Report TLV failed in parsing. \n");
				//if can not obtain the TLV, not include this interface
				spatial_reuse_report_nr--;
			}
		}
	}
	PLATFORM_PRINTF_DETAIL("[MULTI_AP] total [%d] number of spatial reuse repot tlv parsed successflly.\n", spatial_reuse_report_nr);
	tlvs = (struct SpatialReuseReportTLV **)PLATFORM_REALLOC(tlvs, sizeof(struct SpatialReuseReportTLV *) * (spatial_reuse_report_nr));
	*t   = tlvs;
	PLATFORM_FREE(buffer);
	return spatial_reuse_report_nr;
}
#endif //RTK_MULTI_AP_R4

//******************************************************************************
//******* "Buffer writer" stuff (read below) ***********************************
//******************************************************************************
//
// The following "buffer writer" related variables and functions are used to
// "trick" the "DMdumpNetworkDevices()" function to print to a memory buffer
// instead of to a file descriptor (ex: STDOUT)
//
//   TODO: Review this mechanism so that such a huge malloc is not needed.
//         Because the information contained in this buffer is meant to be sent
//         through a TCP socket, maybe we could allocate small chunks and keep
//         sending them through the socket... however this would require several
//         changes in the way things operate now...
//         Think about it (and, who knows... maybe we decide to leave it as it
//         is now after all).
//
#define MEMORY_BUFFER_SIZE (63 * 1024)
char *memory_buffer
    = NULL;
INT16U memory_buffer_i = 0;

void _memoryBufferWriterInit()
{
	memory_buffer   = (char *)PLATFORM_MALLOC(MEMORY_BUFFER_SIZE);
	memory_buffer_i = 0;
}
void _memoryBufferWriter(const char *fmt, ...)
{
	va_list arglist;

	if (memory_buffer_i >= MEMORY_BUFFER_SIZE - 1) {
		// Too big...
		//
		PLATFORM_PRINTF_WARNING("Memory buffer overflow.\n");
		return;
	}

	va_start(arglist, fmt);
	PLATFORM_VSNPRINTF(memory_buffer + memory_buffer_i, MEMORY_BUFFER_SIZE - memory_buffer_i, fmt, arglist);
	va_end(arglist);

	memory_buffer[MEMORY_BUFFER_SIZE - 1] = 0x0;
	memory_buffer_i                       = PLATFORM_STRLEN(memory_buffer);
	memory_buffer[memory_buffer_i]        = 0x0;

	return;
}
void _memoryBufferWriterEnd()
{
	if (NULL != memory_buffer) {
		PLATFORM_FREE(memory_buffer);

		memory_buffer_i = 0;
	}
	return;
}

//******************************************************************************
//******* Local device data dump ***********************************************
//******************************************************************************
//
// This function is used to update the entry of the database associated to the
// local node.
//
// Let me explain this in more detail: the database contains information of
// all nodes (the local and remote ones):
//
//   - For remote nodes, every time a response CMDU is received, the TLVs
//     contained in that CMDU are added to the entry of the database associated
//     to that remote node (or updated, if they already existed)
//
//   - For the local node, however, we must "manually" force an "update" so that
//     the database entry associated to the local node contains updated
//     information. *This* is exactly what this function does.
//
// When should we call this function? Well... we are only interested in updating
// this local entry when someone is going to look at it which, as of today,
// only happens when a special ("custom") ALME is received
// ("CUSTOM_COMMAND_DUMP_NETWORK_DEVICES") and, as a result, we must send the
// local information as part of the response.
//
void _updateLocalDeviceData()
{
	struct deviceInformationTypeTLV *     info;
	struct deviceBridgingCapabilityTLV ** bridges;
	struct non1905NeighborDeviceListTLV **non1905_neighbors;
	INT8U                                 non1905_neighbors_nr;
	struct neighborDeviceListTLV **       x1905_neighbors;
	INT8U                                 x1905_neighbors_nr;

	struct ApOperationalBSSTLV * operational_bss;
	struct SupportedServiceTLV * supported_service;
	struct AssociatedClientsTLV *associated_clients;

	struct transmitterLinkMetricTLV **tx_tlvs;
	struct receiverLinkMetricTLV **   rx_tlvs;
	INT8U                             total_metrics_tlvs;
	INT8U                             i;

	// We need to allocate these structures in the heap (instead of simply
	// declaring variables in the stack) because they are going to be "saved"
	// in the database when calling "DMupdate*()"
	//
	info       = (struct deviceInformationTypeTLV *)PLATFORM_MALLOC(sizeof(struct deviceInformationTypeTLV));
	bridges    = (struct deviceBridgingCapabilityTLV **)PLATFORM_MALLOC(sizeof(struct deviceBridgingCapabilityTLV *));
	bridges[0] = (struct deviceBridgingCapabilityTLV *)PLATFORM_MALLOC(sizeof(struct deviceBridgingCapabilityTLV));

	operational_bss    = (struct ApOperationalBSSTLV *)PLATFORM_MALLOC(sizeof(struct ApOperationalBSSTLV));
	supported_service  = (struct SupportedServiceTLV *)PLATFORM_MALLOC(sizeof(struct SupportedServiceTLV));
	associated_clients = (struct AssociatedClientsTLV *)PLATFORM_MALLOC(sizeof(struct AssociatedClientsTLV));

	_obtainLocalDeviceInfoTLV(info);
	_obtainLocalBridgingCapabilitiesTLV(bridges[0]);
	_obtainLocalNeighborsTLV(&non1905_neighbors, &non1905_neighbors_nr, &x1905_neighbors, &x1905_neighbors_nr);

	_obtainLocalMetricsTLVs(LINK_METRIC_QUERY_TLV_ALL_NEIGHBORS,
	                        NULL,
	                        LINK_METRIC_QUERY_TLV_BOTH_TX_AND_RX_LINK_METRICS,
	                        &tx_tlvs,
	                        &rx_tlvs,
	                        &total_metrics_tlvs);

	_obtainApOperationalBSSTLV(operational_bss);
	_obtainSupportedServiceTLV(supported_service);
	_obtainAssociatedClientsTLV(associated_clients);

	// The following function will take care of "freeing" the allocated memory
	// if needed
	//
	DMupdateNetworkDeviceInfo(info->al_mac_address, "",
	                          1, info,
	                          1, bridges, 1,
	                          1, non1905_neighbors, non1905_neighbors_nr,
	                          1, x1905_neighbors, x1905_neighbors_nr,
	                          1, supported_service,
	                          1, operational_bss,
	                          1, associated_clients);

	// The next function, however, takes care only of the pointers to metrics
	// information, and not of the memory used to hold the list of pointers
	// themselves...
	//
	for (i = 0; i < total_metrics_tlvs; i++) {
		DMupdateNetworkDeviceMetrics((INT8U *)tx_tlvs[i]);
		DMupdateNetworkDeviceMetrics((INT8U *)rx_tlvs[i]);
	}

	// ... and that's why we need to do this (the "0" is so that pointers
	// themselves are not freed, as they are now responsibility of the
	// database)
	//
	total_metrics_tlvs = 0;
	_freeLocalMetricsTLVs(&tx_tlvs, &rx_tlvs, &total_metrics_tlvs);
}

INT8U _multicastToUnicast(char *interface_name, INT16U mid, struct CMDU *discovery_message, bool need_ack)
{
	struct networkDevice *network_devices;
	INT8U                 i = 0;
	INT8U                 network_devices_nr;
	DMallNetworkDevicesGet(&network_devices, &network_devices_nr);

	discovery_message->relay_indicator = 0;

	if (network_devices_nr > 1) {
		for (i = 1; i < network_devices_nr; i++) {
			if (0 != PLATFORM_STRCMP(interface_name, network_devices[i].receiving_interface)) {
				continue;
			}
			PLATFORM_PRINTF_DEBUG("MulticastToUnicast to: %02x:%02x:%02x:%02x:%02x:%02x\n", network_devices[i].info->al_mac_address[0],
			                      network_devices[i].info->al_mac_address[1], network_devices[i].info->al_mac_address[2],
			                      network_devices[i].info->al_mac_address[3], network_devices[i].info->al_mac_address[4],
			                      network_devices[i].info->al_mac_address[5]);
			if (0 == send1905RawPacket(interface_name, mid, network_devices[i].info->al_mac_address, discovery_message, need_ack, 0)) {
				PLATFORM_PRINTF_ERROR("Could not send the 1905 packet\n");
				return 0;
			}
		}
	}

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Public functions (exported only to files in this same folder)
////////////////////////////////////////////////////////////////////////////////

INT8U send1905RawPacket(char *interface_name, INT16U mid, INT8U *dst_mac_address, struct CMDU *cmdu, bool need_ack, INT8U is_forwarding)
{
	return send1905RawPacketWithSrc(interface_name, mid, dst_mac_address, DMalMacGet(), cmdu, need_ack, is_forwarding);
}

INT8U send1905RawPacketWithSrc(char *interface_name, INT16U mid, INT8U *dst_mac_address, INT8U *src_mac_address, struct CMDU *cmdu, bool need_ack, INT8U is_forwarding)
{
	INT8U **streams;
	INT16U *streams_lens = NULL;

	INT8U total_streams, x;

#ifdef RTK_MULTI_AP_R2
	INT16U primary_vid = 0;
	INT8U  default_pcp = 0;
#endif

#ifdef RTK_MULTI_AP_R3
	INT8U  mcast_address[] = MCAST_1905;
	struct MICTLV mic_tlv;
	INT8U  mic_tlv_index = 0;
#endif
	char sending_interface_name[64];

	INT8U revision = DMgetMultiAPProfile();
	if (MULTI_AP_PROFILE_1 == DMgetTargetDeviceMultiAPProfile(dst_mac_address))
		revision = MULTI_AP_PROFILE_1;
#ifdef RTK_MULTI_AP_R2
	else if (MULTI_AP_PROFILE_2 == DMgetTargetDeviceMultiAPProfile(dst_mac_address))
		revision = MULTI_AP_PROFILE_2;
#endif
#ifdef RTK_MULTI_AP_R3
	else if (MULTI_AP_PROFILE_3 == DMgetTargetDeviceMultiAPProfile(dst_mac_address))
		revision = MULTI_AP_PROFILE_3;
#endif
#ifdef RTK_MULTI_AP_R4
	else if (MULTI_AP_PROFILE_4 == DMgetTargetDeviceMultiAPProfile(dst_mac_address))
		revision = MULTI_AP_PROFILE_4;
#endif

#ifdef RTK_MULTI_AP_R3
	INT8U is_r3_format = 0;
	if (MULTI_AP_PROFILE_3 <= DMgetMultiAPProfile() && MULTI_AP_PROFILE_3 <= DMgetTargetDeviceMultiAPProfile(dst_mac_address)) {
		is_r3_format = 1;
	}
#endif

	struct vendorSpecificTLV api_vendor_specific_tlv = {0};
	void (*callback_func)(INT8U *)                   = NULL;
	api_get_append_tlv_callback(cmdu->message_type, &callback_func);
	if (callback_func) {
		callback_func((INT8U *)&api_vendor_specific_tlv);
		if (api_vendor_specific_tlv.m_nr) {
			_vendorSpecificTLVInsertInCMDU(cmdu, &api_vendor_specific_tlv);
		}
	}

#ifdef RTK_MULTI_AP_R3
	if (0 == PLATFORM_MEMCMP(dst_mac_address, mcast_address, 6) && !is_forwarding)
		mic_tlv_index = _insertMICTLV(cmdu, src_mac_address, &mic_tlv);
#endif

	// Buffer CMDU for Wi-Fi Test Suite dev_start_buffer
	if (DMgetWFATestMode())
		buffer_cmdu_message(cmdu);

	if (PLATFORM_NEED_DETAIL_PRINT()) {
		PLATFORM_PRINTF_DETAIL("Contents of CMDU to send:\n");
		visit_1905_CMDU_structure(cmdu, print_callback, PLATFORM_PRINTF_DETAIL, "");
	}

#ifdef RTK_MULTI_AP_R3
	if (is_r3_format) {
		encrypt1905Packet(dst_mac_address, mcast_address, cmdu, is_forwarding);
	}
#endif

	streams = forge_1905_CMDU_from_structure(cmdu, &streams_lens, revision, is_forwarding);

#ifdef RTK_MULTI_AP_R3
	if (mic_tlv_index) {
		// Reset MIC TLV pointer so it does not point to the stack
		cmdu->list_of_TLVs[mic_tlv_index] = NULL;
	}
#endif

	if (NULL == streams) {
		// Could not forge the packet. Error?
		//
		PLATFORM_PRINTF_WARNING("forge_1905_CMDU_from_structure() failed!\n");
		return 0;
	}

	// Free previously allocated CMDU extensions (no longer needed)
	//
	//comment out the following line as it literally does nothing
	// free1905CmduExtensions(cmdu);
	total_streams = 0;
	while (streams[total_streams]) {
		total_streams++;
	}

	if (0 == total_streams) {
		// Could not forge the packet. Error?
		//
		PLATFORM_PRINTF_WARNING("forge_1905_CMDU_from_structure() returned 0 streams!\n");

		free_1905_CMDU_packets(streams);
		PLATFORM_FREE(streams_lens);
		return 0;
	}

	if (DEVICE_FULLY_CONFIGURED == DMgetConfiguredState() || DMisController()) {
#ifdef RTK_MULTI_AP_R2
		DMgetDefault8021QSettings(&primary_vid, &default_pcp);
		if (primary_vid && PLATFORM_STRSTR(interface_name, "wlan")) {
			char *base_name = trimInterfaceNameVID(interface_name, NULL, NULL);
			sprintf(sending_interface_name, "%s.%d", base_name, primary_vid);
			PLATFORM_FREE(base_name);
			if (PLATFORM_IS_INTERFACE_UP_AND_ON(sending_interface_name)) {
				PLATFORM_PRINTF_DEBUG("Sending 1905 message on VLAN interface %s, MID %04x\n", sending_interface_name, mid);
			} else {
				sprintf(sending_interface_name, "%s", interface_name);
			}
		} else
#endif
		{
			sprintf(sending_interface_name, "%s", interface_name);
		}
	} else {
		sprintf(sending_interface_name, "%s", interface_name);
	}

	x = 0;
	while (streams[x]) {
		PLATFORM_PRINTF_DETAIL("Sending 1905 message on interface %s, MID %04x, fragment %d/%d\n", sending_interface_name, mid, x + 1, total_streams);
		if (0 == PLATFORM_SEND_RAW_PACKET(sending_interface_name, dst_mac_address, src_mac_address, ETHERTYPE_1905, streams[x], streams_lens[x])) {
			PLATFORM_PRINTF_ERROR("Packet could not be sent!\n");
		}

		x++;
	}

	// for the messages that need ack we have to save all the streams first
	if (need_ack && PLATFORM_STRCMP("lo", sending_interface_name)) {
		if (1 == waiting_msgs_add(sending_interface_name, dst_mac_address, streams, streams_lens)) {
			free_1905_CMDU_packets(streams);
			PLATFORM_FREE(streams_lens);
		}
	} else {
		free_1905_CMDU_packets(streams);
		PLATFORM_FREE(streams_lens);
	}
	return 1;
}

INT8U send1905RawALME(INT8U alme_client_id, INT8U *alme)
{
	INT8U *packet_out;
	INT16U packet_out_len;

	if (PLATFORM_NEED_DETAIL_PRINT()) {
		PLATFORM_PRINTF_DETAIL("Contents of ALME reply to send:\n");
		visit_1905_ALME_structure((INT8U *)alme, print_callback, PLATFORM_PRINTF_DETAIL, "");
	}

	// Use the getIntfListResponseALME structure to forge the packet
	// bit stream
	//
	packet_out = forge_1905_ALME_from_structure((INT8U *)alme, &packet_out_len);
	if (NULL == packet_out) {
		PLATFORM_PRINTF_WARNING("forge_1905_ALME_from_structure() failed.\n");

		PLATFORM_SEND_ALME_REPLY(alme_client_id, NULL, 0);
		return 0;
	}

	// Send the ALME reply back
	//
	PLATFORM_SEND_ALME_REPLY(alme_client_id, packet_out, packet_out_len);

	free_1905_ALME_packet(packet_out);

	return 1;
}

#ifdef RTK_MULTI_AP_R3
void encrypt1905Packet(INT8U *dst_mac_address, INT8U *mcast_address, struct CMDU *cmdu, INT8U is_forwarding)
{
	// Encryption is not done for the following CMDUs:
	// - 1905 AP-Autoconfiguration Search
	// - 1905 AP-Autoconfiguration Response
	// - Direct Encap DPP
	// - 1905 Encap EAPOL
	if (!(cmdu->message_type == CMDU_TYPE_AP_AUTOCONFIGURATION_SEARCH || cmdu->message_type == CMDU_TYPE_AP_AUTOCONFIGURATION_RESPONSE || cmdu->message_type == CMDU_TYPE_DIRECT_ENCAP_DPP || cmdu->message_type == CMDU_TYPE_1905_ENCAP_EAPOL)) {
		struct dpp_ptk *ptk = dpp_keystore_get_ptk(dst_mac_address);
		// Encryption is done only if:
		// 1. CMDU is unicast
		// 2. Device possesses the 1905 TK corresponding to the receiver of the message
		if ((0 != PLATFORM_MEMCMP(dst_mac_address, mcast_address, 6)) && ptk && (ptk->tk_len != 0)) {
			// Do AES-SIV Encryption for all TLVs except End Of Message TLV
			PLATFORM_PRINTF_DEBUG("[ENCRYPT] Starting 1905 Packet Encryption...\n");

			// The cleartext input of AES-SIV encryption is the concatenation of all TLVs except EOM TLV
			INT8U *concatenated_tlvs;
			INT16U concatenated_tlvs_size = 0;
			concatenated_tlvs             = concat_all_TLVs_from_structure(cmdu, &concatenated_tlvs_size, is_forwarding);

			// There are 4 associated data used:
			// AD1 = The first six octets of the 1905 CMDU.
			// AD2 = The Encryption Transmission Counter value at the sender.
			// AD3 = Source 1905 AL MAC Address.
			// AD4 = Destination 1905 AL MAC Address.
			INT8U assoc_data1[6], assoc_data2[6], assoc_data3[6], assoc_data4[6];

			INT8U *p1 = assoc_data1;
			INT8U  aux8;
			_I1B(&cmdu->message_version, &p1);
			aux8 = 0; // Reserved Field 0x00
			_I1B(&aux8, &p1);
			_I2B(&cmdu->message_type, &p1);
			_I2B(&cmdu->message_id, &p1);

			INT64U transmission_counter = dpp_counters_get_etc(dst_mac_address);
			INT8U *p2                   = assoc_data2;
			_INT64_I6B(&transmission_counter, &p2);

			PLATFORM_MEMCPY(assoc_data3, DMalMacGet(), 6);
			PLATFORM_MEMCPY(assoc_data4, dst_mac_address, 6);

			// Uncomment below for debugging purposes
			// PLATFORM_PRINTF_INFO("message type 		= %02x%02x\n", assoc_data1[1], assoc_data1[2]);
			// PLATFORM_PRINTF_INFO("message id 		= %02x%02x\n", assoc_data1[3], assoc_data1[4]);
			// PLATFORM_PRINTF_INFO("etc 				= %02x\n", assoc_data2[5]);
			// PLATFORM_PRINTF_INFO("src		 		= %02x:%02x:%02x:%02x:%02x:%02x\n", assoc_data3[0], assoc_data3[1], assoc_data3[2], assoc_data3[3], assoc_data3[4], assoc_data3[5]);
			// PLATFORM_PRINTF_INFO("dst		 		= %02x:%02x:%02x:%02x:%02x:%02x\n", assoc_data4[0], assoc_data4[1], assoc_data4[2], assoc_data4[3], assoc_data4[4], assoc_data4[5]);

			const INT32U assoc_data_len[] = { 0x06, 0x06, 0x06, 0x06 };
			const INT8U *assoc_data_arr[] = { assoc_data1, assoc_data2, assoc_data3, assoc_data4 };

			INT8U ciphertext[AES_BLOCK_SIZE + concatenated_tlvs_size];
			PLATFORM_MEMSET(ciphertext, 0, AES_BLOCK_SIZE + concatenated_tlvs_size);

			if (0 == PLATFORM_AES_SIV_ENCRYPT(ptk->tk, ptk->tk_len, concatenated_tlvs, concatenated_tlvs_size, ARRAY_SIZE(assoc_data_arr), assoc_data_arr, assoc_data_len, ciphertext)) {
				PLATFORM_PRINTF_WARNING("[ENCRYPT] 1905 Packet Encryption error\n");
			} else {
				// Uncomment below for debugging purposes
				// int j = 0, k = 0;
				// PLATFORM_PRINTF_INFO("tk = ");
				// while (*(ptk->tk + j)) {
				// 	printf("%02x", *(ptk->tk + j));
				// 	j++;
				// }
				// printf("\n");
				// PLATFORM_PRINTF_INFO("ciphertext = ");
				// for (k = 0; k < ciphertext_size; k++) {
				// 	printf("%02x", *(ciphertext + k));
				// }
				// printf("\n");
				// PLATFORM_PRINTF_INFO("sizeof(ciphertext) = %d\n", sizeof(ciphertext));

				struct EncryptedPayloadTLV *encrypted_payload_tlv = NULL;
				encrypted_payload_tlv                             = (struct EncryptedPayloadTLV *)PLATFORM_MALLOC(sizeof(struct EncryptedPayloadTLV));
				_obtainEncryptedPayloadTLV(encrypted_payload_tlv, dst_mac_address, ciphertext, sizeof(ciphertext));

				PLATFORM_FREE(cmdu->list_of_TLVs);
				cmdu->list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);
				cmdu->list_of_TLVs[0] = (INT8U *)encrypted_payload_tlv;
				cmdu->list_of_TLVs[1] = NULL;

				transmission_counter++;
				if (transmission_counter > 0xFFFFFFFFFFFF) {
					transmission_counter = 0;
				}
				dpp_counters_set_etc(dst_mac_address, transmission_counter);

				PLATFORM_PRINTF_DEBUG("[ENCRYPT] 1905 Packet Encryption successful\n");
			}
		}
	}
}

void encrypt1905Alme(struct multiApCommandRequestALME *p, INT16U mid)
{
	// Encryption is not done for the following CMDUs:
	// - 1905 AP-Autoconfiguration Search
	// - 1905 AP-Autoconfiguration Response
	// - Direct Encap DPP
	// - 1905 Encap EAPOL
	if (!(p->cmdu_type == CMDU_TYPE_AP_AUTOCONFIGURATION_SEARCH || p->cmdu_type == CMDU_TYPE_AP_AUTOCONFIGURATION_RESPONSE || p->cmdu_type == CMDU_TYPE_DIRECT_ENCAP_DPP || p->cmdu_type == CMDU_TYPE_1905_ENCAP_EAPOL)) {
		INT8U           mcast_address[] = MCAST_1905;
		struct dpp_ptk *ptk             = dpp_keystore_get_ptk(p->al_mac);
		// Encryption is done only if:
		// 1. CMDU is unicast
		// 2. Device possesses the 1905 TK corresponding to the receiver of the message
		if ((0 != PLATFORM_MEMCMP(p->al_mac, mcast_address, 6)) && ptk && (ptk->tk_len != 0)) {
			// Do AES-SIV Encryption for all TLVs except End Of Message TLV
			PLATFORM_PRINTF_DEBUG("[ENCRYPT] Starting 1905 ALME Encryption...\n");

			// There are 4 associated data used:
			// AD1 = The first six octets of the 1905 CMDU.
			// AD2 = The Encryption Transmission Counter value at the sender.
			// AD3 = Source 1905 AL MAC Address.
			// AD4 = Destination 1905 AL MAC Address.
			INT8U assoc_data1[6], assoc_data2[6], assoc_data3[6], assoc_data4[6];

			struct CMDU cmdu;
			cmdu.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
			cmdu.message_type    = p->cmdu_type;
			cmdu.message_id      = mid;
			cmdu.relay_indicator = 0;
			INT8U *p1            = assoc_data1;
			INT8U  aux8;
			_I1B(&cmdu.message_version, &p1);
			aux8 = 0; // Reserved Field 0x00
			_I1B(&aux8, &p1);
			_I2B(&cmdu.message_type, &p1);
			_I2B(&cmdu.message_id, &p1);

			INT64U transmission_counter = dpp_counters_get_etc(p->al_mac);
			INT8U *p2                   = assoc_data2;
			_INT64_I6B(&transmission_counter, &p2);

			PLATFORM_MEMCPY(assoc_data3, DMalMacGet(), 6);
			PLATFORM_MEMCPY(assoc_data4, p->al_mac, 6);

			const INT32U assoc_data_len[] = { 0x06, 0x06, 0x06, 0x06 };
			const INT8U *assoc_data_arr[] = { assoc_data1, assoc_data2, assoc_data3, assoc_data4 };

			INT8U ciphertext[AES_BLOCK_SIZE + p->tlv_length];
			PLATFORM_MEMSET(ciphertext, 0, AES_BLOCK_SIZE + p->tlv_length);

			if (0 == PLATFORM_AES_SIV_ENCRYPT(ptk->tk, ptk->tk_len, p->tlv_buf, p->tlv_length, ARRAY_SIZE(assoc_data_arr), assoc_data_arr, assoc_data_len, ciphertext)) {
				PLATFORM_PRINTF_WARNING("[ENCRYPT] 1905 ALME Encryption error\n");
			} else {
				struct EncryptedPayloadTLV *encrypted_payload_tlv = NULL;
				encrypted_payload_tlv                             = (struct EncryptedPayloadTLV *)PLATFORM_ZALLOC(sizeof(struct EncryptedPayloadTLV));
				_obtainEncryptedPayloadTLV(encrypted_payload_tlv, p->al_mac, ciphertext, sizeof(ciphertext));

				INT16U tlvs_size;

				PLATFORM_FREE(p->tlv_buf);
				p->tlv_buf    = forge_1905_TLV_from_structure((INT8U *)encrypted_payload_tlv, &tlvs_size, 0);
				p->tlv_length = tlvs_size;

				free_1905_TLV_structure((INT8U *)encrypted_payload_tlv);

				transmission_counter++;
				if (transmission_counter > 0xFFFFFFFFFFFF) {
					transmission_counter = 0;
				}
				dpp_counters_set_etc(p->al_mac, transmission_counter);

				PLATFORM_PRINTF_DEBUG("[ENCRYPT] 1905 ALME Encryption successful\n");
			}
		}
	}
}
#endif // RTK_MULTI_AP_R3

INT8U send1905TopologyDiscoveryPacket(char *interface_name, INT16U mid)
{
	// The "topology discovery" message is a CMDU with two TLVs:
	//   - One AL MAC address type TLV
	//   - One MAC address type TLV

	INT8U interface_mac_address[6];

	INT8U mcast_address[] = MCAST_1905;

	struct CMDU                discovery_message;
	struct alMacAddressTypeTLV al_mac_addr_tlv;
	struct macAddressTypeTLV   mac_addr_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_TOPOLOGY_DISCOVERY (%s) MID: (%04x)\n", interface_name, mid);

	PLATFORM_MEMCPY(interface_mac_address, DMinterfaceNameToMac(interface_name), 6);

	// Fill the AL MAC address type TLV
	//
	_obtainLocalAlMacAddressTLV(&al_mac_addr_tlv);

	// Fill the MAC address type TLV
	//
	mac_addr_tlv.tlv_type       = TLV_TYPE_MAC_ADDRESS_TYPE;
	mac_addr_tlv.mac_address[0] = interface_mac_address[0];
	mac_addr_tlv.mac_address[1] = interface_mac_address[1];
	mac_addr_tlv.mac_address[2] = interface_mac_address[2];
	mac_addr_tlv.mac_address[3] = interface_mac_address[3];
	mac_addr_tlv.mac_address[4] = interface_mac_address[4];
	mac_addr_tlv.mac_address[5] = interface_mac_address[5];

	// Build the CMDU
	//
	discovery_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	discovery_message.message_type    = CMDU_TYPE_TOPOLOGY_DISCOVERY;
	discovery_message.message_id      = mid;
	discovery_message.relay_indicator = 0;
	discovery_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 3);
	discovery_message.list_of_TLVs[0] = (INT8U *)&al_mac_addr_tlv;
	discovery_message.list_of_TLVs[1] = (INT8U *)&mac_addr_tlv;
	discovery_message.list_of_TLVs[2] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, mcast_address, &discovery_message, false, 0)) {
		PLATFORM_PRINTF_ERROR("Could not send the 1905 packet\n");
		PLATFORM_FREE(discovery_message.list_of_TLVs);
		return 0;
	}

	// Free memory
	//
	_freeLocalAlMacAddressTLV(&al_mac_addr_tlv);

	PLATFORM_FREE(discovery_message.list_of_TLVs);
	return 1;
}

INT8U send1905TopologyQueryPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U need_ack)
{
	// The "topology query" message is a CMDU with no TLVs

	INT8U ret;

	struct CMDU query_message;
#ifdef RTK_MULTI_AP_R2
	struct MultiAPProfileTLV profile_tlv;
#endif
	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_TOPOLOGY_QUERY (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", destination_al_mac_address[0], destination_al_mac_address[1], destination_al_mac_address[2], destination_al_mac_address[3], destination_al_mac_address[4], destination_al_mac_address[5]);

	// Build the CMDU
	//
	query_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	query_message.message_type    = CMDU_TYPE_TOPOLOGY_QUERY;
	query_message.message_id      = mid;
	query_message.relay_indicator = 0;

#ifdef RTK_MULTI_AP_R2
	if (checkTargetDeviceProfile2TLVCapability(destination_al_mac_address)) {
		query_message.list_of_TLVs = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);
		_obtainProfileTLV(&profile_tlv);
		query_message.list_of_TLVs[0] = (INT8U *)&profile_tlv;
		query_message.list_of_TLVs[1] = NULL;
	} else
#endif
	{
		query_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 1);
		query_message.list_of_TLVs[0] = NULL;
	}

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &query_message, need_ack, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	PLATFORM_FREE(query_message.list_of_TLVs);

	return ret;
}

INT8U send1905TopologyResponsePacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address)
{
	// The "topology response" message is a CMDU with the following TLVs:
	//   - One device information type TLV
	//   - Zero or one device bridging capability TLVs
	//   - Zero or more non-1905 neighbor device list TLVs
	//   - Zero or more 1905 neighbor device list TLVs
	//   - Zero or more power off interface TLVs
	//   - Zero or more L2 neighbor device TLVs
	//
	//   NOTE: The "non-1905 neighbor" and the "L2 neighbor" device TLVs are
	//   kind of overlaping... but this is what the standard says.
	//
	//   NOTE: Regarding the "device bridging capability", "power off interface"
	//   and "L2 neighbor device" TLVs, the standard says "zero or more" but
	//   it should be "zero or one", as one single TLV of these types can carry
	//   many entries.
	//   That's why in this implementation we are just sending zero or one (no
	//   more!) TLVs of these type. However, in reception (see
	//   "process1905Cmdu()") we will be ready to receive more.

	INT8U ret;

	struct CMDU                           response_message;
	struct deviceInformationTypeTLV       device_info;
	struct deviceBridgingCapabilityTLV    bridge_info;
	struct non1905NeighborDeviceListTLV **non_1905_neighbors;
	struct neighborDeviceListTLV **       neighbors;

	// for Multi-AP
	struct SupportedServiceTLV  supported_service_tlv;
	struct ApOperationalBSSTLV  ap_operational_bss_tlv;
	struct AssociatedClientsTLV associated_clients_tlv;
#ifdef RTK_MULTI_AP_R2
	struct MultiAPProfileTLV profile_tlv;
#endif
#ifdef RTK_MULTI_AP_R3
	struct BSSConfigReportTLV *bss_conf_rep_tlv;
	bss_conf_rep_tlv = (struct BSSConfigReportTLV *)PLATFORM_MALLOC(sizeof(struct BSSConfigReportTLV));
#endif

	INT8U non_1905_neighbors_nr;
	INT8U neighbors_nr;

	INT8U total_tlvs = 0;
	INT8U i, j;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_TOPOLOGY_RESPONSE (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", destination_al_mac_address[0], destination_al_mac_address[1], destination_al_mac_address[2], destination_al_mac_address[3], destination_al_mac_address[4], destination_al_mac_address[5]);

	// Fill all the needed TLVs
	//
	_obtainLocalDeviceInfoTLV(&device_info);
	_obtainLocalBridgingCapabilitiesTLV(&bridge_info);
	_obtainLocalNeighborsTLV(&non_1905_neighbors, &non_1905_neighbors_nr, &neighbors, &neighbors_nr);

	_obtainSupportedServiceTLV(&supported_service_tlv);
	_obtainApOperationalBSSTLV(&ap_operational_bss_tlv);
	_obtainAssociatedClientsTLV(&associated_clients_tlv);

	// Build the CMDU
	//
	total_tlvs = 1; // Device information type TLV

#ifndef SEND_EMPTY_TLVS
	if (bridge_info.bridging_tuples_nr != 0)
#endif
	{
		total_tlvs++; // Device bridging capability TLV
	}

	total_tlvs += neighbors_nr;          // Non-1905 neighbor device list TLVs
	total_tlvs += non_1905_neighbors_nr; // 1905 Neighbor device list TLVs

	// for Multi-AP
	total_tlvs += 3;
#ifdef RTK_MULTI_AP_R2
	if (checkTargetDeviceProfile2TLVCapability(destination_al_mac_address)) {
		// For Multi-AP Profile 2
		_obtainProfileTLV(&profile_tlv);
		total_tlvs += 1;
	}
#endif

#ifdef RTK_MULTI_AP_R3
	_obtainBSSConfigReportTLV(bss_conf_rep_tlv);
	total_tlvs += 1;
#endif

	response_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	response_message.message_type    = CMDU_TYPE_TOPOLOGY_RESPONSE;
	response_message.message_id      = mid;
	response_message.relay_indicator = 0;
	response_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * (total_tlvs + 1));

	if (DMgetWFATestMode()) {
		i = 0;

		for (j = 0; j < neighbors_nr; j++) {
			response_message.list_of_TLVs[i++] = (INT8U *)neighbors[j];
		}

		for (j = 0; j < non_1905_neighbors_nr; j++) {
			response_message.list_of_TLVs[i++] = (INT8U *)non_1905_neighbors[j];
		}

		response_message.list_of_TLVs[i++] = (INT8U *)&device_info;

#ifndef SEND_EMPTY_TLVS
		if (bridge_info.bridging_tuples_nr != 0)
#endif
		{
			response_message.list_of_TLVs[i++] = (INT8U *)&bridge_info;
		}
	} else {
		response_message.list_of_TLVs[0] = (INT8U *)&device_info;

		i = 1;
#ifndef SEND_EMPTY_TLVS
		if (bridge_info.bridging_tuples_nr != 0)
#endif
		{
			response_message.list_of_TLVs[i++] = (INT8U *)&bridge_info;
		}

		for (j = 0; j < non_1905_neighbors_nr; j++) {
			response_message.list_of_TLVs[i++] = (INT8U *)non_1905_neighbors[j];
		}

		for (j = 0; j < neighbors_nr; j++) {
			response_message.list_of_TLVs[i++] = (INT8U *)neighbors[j];
		}
	}

	response_message.list_of_TLVs[i++] = (INT8U *)&supported_service_tlv;
	response_message.list_of_TLVs[i++] = (INT8U *)&ap_operational_bss_tlv;
	response_message.list_of_TLVs[i++] = (INT8U *)&associated_clients_tlv;
#ifdef RTK_MULTI_AP_R2
	if (checkTargetDeviceProfile2TLVCapability(destination_al_mac_address))
		response_message.list_of_TLVs[i++] = (INT8U *)&profile_tlv;
#endif
#ifdef RTK_MULTI_AP_R3
	response_message.list_of_TLVs[i++] = (INT8U *)bss_conf_rep_tlv;
#endif
	response_message.list_of_TLVs[i] = NULL;

	//-------------Vendor specific tlv-------------------------
	struct vendorSpecificTLV vendor_specific_tlv;
	vendor_specific_tlv.tlv_type = TLV_TYPE_VENDOR_SPECIFIC;
	PLATFORM_MEMCPY(vendor_specific_tlv.vendorOUI, REALTEK_OUI, 3);
	char *ip_addr = NULL;
	char *buffer = NULL;

	ip_addr = PLATFORM_GET_IP_ADDRESS();

#if 1//def RTK_ETHERNET_ADAPTER
	if(customize_features & SUPPORT_ETHERNET_ADAPTER)
	{
		char *device_name = NULL;
		char ip_tmp[16] = {0};
		strlcpy(ip_tmp, ip_addr, sizeof(ip_tmp));
		device_name = DMgetDeviceName();
		if (!DMisController()) {
			buffer = map_fill_topo_response_vendor_data(ip_addr, device_name);
		}
		strlcpy(ip_addr, ip_tmp, sizeof(ip_tmp));
	}
#endif

	vendor_specific_tlv.m_nr = PLATFORM_STRLEN(ip_addr) + 1 + PLATFORM_STRLEN(DMgetDeviceName()) + 1;
	vendor_specific_tlv.m_nr += PLATFORM_STRLEN(MAP_VERSION) + 1;
	if (buffer) {
		vendor_specific_tlv.m_nr += PLATFORM_STRLEN(buffer) + 1;
	}
	char *str_buff           = (char *)PLATFORM_MALLOC(vendor_specific_tlv.m_nr);
	PLATFORM_MEMSET(str_buff, 0, vendor_specific_tlv.m_nr);
	PLATFORM_MEMCPY(str_buff, ip_addr, PLATFORM_STRLEN(ip_addr));
	PLATFORM_MEMCPY(str_buff + PLATFORM_STRLEN(ip_addr) + 1, DMgetDeviceName(), PLATFORM_STRLEN(DMgetDeviceName()));

	PLATFORM_MEMCPY(str_buff + PLATFORM_STRLEN(ip_addr) + 1 + PLATFORM_STRLEN(DMgetDeviceName()) + 1, MAP_VERSION, PLATFORM_STRLEN(MAP_VERSION));

	if (buffer) {
		PLATFORM_MEMCPY(str_buff + PLATFORM_STRLEN(ip_addr) + 1 + PLATFORM_STRLEN(DMgetDeviceName()) + 1 + PLATFORM_STRLEN(MAP_VERSION) + 1,
			buffer, PLATFORM_STRLEN(buffer));
	}

	vendor_specific_tlv.m = (INT8U *)str_buff;

	_vendorSpecificTLVInsertInCMDU(&response_message, &vendor_specific_tlv);

	//-------------Vendor specific tlv-------------------------

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &response_message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free all allocated (and no longer needed) memory
	//
	_freeLocalDeviceInfoTLV(&device_info);
	_freeLocalBridgingCapabilitiesTLV(&bridge_info);
	_freeLocalNeighborsTLV(&non_1905_neighbors, &non_1905_neighbors_nr, &neighbors, &neighbors_nr);

	_freeSupportedServiceTLV(&supported_service_tlv);
	_freeApOperationalBSSTLV(&ap_operational_bss_tlv);
	_freeAssociatedClientsTLV(&associated_clients_tlv);
	_freeVendorSpecificTLV(&vendor_specific_tlv);

#ifdef RTK_MULTI_AP_R3
	free_1905_TLV_structure((INT8U *)bss_conf_rep_tlv);
#endif
	if (buffer) {
		PLATFORM_FREE(buffer);
	}

	PLATFORM_FREE(response_message.list_of_TLVs);
	return ret;
}

INT8U send1905TopologyNotificationPacket(char *interface_name, INT16U mid, INT8U client_mac_addr[], INT8U bssid[], INT8U event)
{
	// The "topology discovery" message is a CMDU with one TLVs:
	//   - One AL MAC address type TLV

	INT8U ret;
	bool need_ack;

	INT8U mcast_address[] = MCAST_1905;

	struct CMDU                discovery_message;
	struct alMacAddressTypeTLV al_mac_addr_tlv;

	// for multi-ap
	struct ClientAssociationEventTLV client_association_event_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_TOPOLOGY_NOTIFICATION (%s) MID: (%04x)\n", interface_name, mid);

	// Fill all the needed TLVs
	//
	_obtainLocalAlMacAddressTLV(&al_mac_addr_tlv);

	if (NULL != client_mac_addr) {
		client_association_event_tlv.tlv_type = TLV_TYPE_CLIENT_ASSOCIATION_EVENT;
		PLATFORM_MEMCPY(client_association_event_tlv.mac_address, client_mac_addr, 6);
		PLATFORM_MEMCPY(client_association_event_tlv.bssid, bssid, 6);
		client_association_event_tlv.event = event;
	}
	// Build the CMDU
	//
	discovery_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	discovery_message.message_type    = CMDU_TYPE_TOPOLOGY_NOTIFICATION;
	discovery_message.message_id      = mid;
	discovery_message.relay_indicator = 1;

	if (NULL == client_mac_addr) {
		discovery_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);
		discovery_message.list_of_TLVs[0] = (INT8U *)&al_mac_addr_tlv;
		discovery_message.list_of_TLVs[1] = NULL;
	} else {
		discovery_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 3);
		discovery_message.list_of_TLVs[0] = (INT8U *)&al_mac_addr_tlv;
		discovery_message.list_of_TLVs[1] = (INT8U *)&client_association_event_tlv;
		discovery_message.list_of_TLVs[2] = NULL;
	}
	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, mcast_address, &discovery_message, false, 0)) {
		PLATFORM_PRINTF_ERROR("Could not send the 1905 packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	if (1 == ret) {
		need_ack = true;
		if (controller_interface && PLATFORM_STRSTR(interface_name, controller_interface))
			need_ack = false;
		_multicastToUnicast(interface_name, mid, &discovery_message, need_ack);
	}

	// Free all allocated (and no longer needed) memory
	//
	_freeLocalAlMacAddressTLV(&al_mac_addr_tlv);

	PLATFORM_FREE(discovery_message.list_of_TLVs);

	return ret;
}

INT8U send1905MetricsQueryPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U destination, INT8U type, INT8U *sta_mac)
{
	// The "metrics query" message is a CMDU with one TLV:
	//   - One link metric query TLV

	INT8U ret;

	struct CMDU               query_message;
	struct linkMetricQueryTLV metric_query_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_LINK_METRIC_QUERY (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", destination_al_mac_address[0], destination_al_mac_address[1], destination_al_mac_address[2], destination_al_mac_address[3], destination_al_mac_address[4], destination_al_mac_address[5]);

	// Fill all the needed TLVs
	//
	metric_query_tlv.tlv_type          = TLV_TYPE_LINK_METRIC_QUERY;
	metric_query_tlv.destination       = destination;
	metric_query_tlv.link_metrics_type = type;
	PLATFORM_MEMCPY(metric_query_tlv.specific_neighbor, sta_mac, MACADDRLEN);

	// Build the CMDU
	//
	query_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	query_message.message_type    = CMDU_TYPE_LINK_METRIC_QUERY;
	query_message.message_id      = mid;
	query_message.relay_indicator = 0;
	query_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);
	query_message.list_of_TLVs[0] = (INT8U *)&metric_query_tlv;
	query_message.list_of_TLVs[1] = NULL;

	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &query_message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free all allocated (and no longer needed) memory
	//
	PLATFORM_FREE(query_message.list_of_TLVs);

	return ret;
}

INT8U send1905MetricsResponsePacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U destination, INT8U *specific_neighbor, INT8U metrics_type)
{
	// The "metrics response" message can be either:
	//
	//   A) A CMDU containing either:
	//      - One Tx link metrics
	//      - One Rx link metrics
	//      - One Rx and one Tx link metrics
	//      ... containing info regarding the link between the current node
	//      and the AL entity whose AL MAC is 'specific_neighbor'
	//
	//   B) A CMDU made by concatenating many CMDUs of "type A" (one for each of
	//      its 1905 neighbors).
	//
	// Case (A) happens when
	//
	//   'destination' == LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR
	//
	// ...while case (B) takes place when
	//
	//   'destination' == LINK_METRIC_QUERY_TLV_ALL_NEIGHBORS
	//
	INT8U ret;

	struct CMDU                       response_message;
	struct transmitterLinkMetricTLV **tx_tlvs;
	struct receiverLinkMetricTLV **   rx_tlvs;

	INT8U total_tlvs;
	INT8U i, j;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_LINK_METRIC_RESPONSE (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", destination_al_mac_address[0], destination_al_mac_address[1], destination_al_mac_address[2], destination_al_mac_address[3], destination_al_mac_address[4], destination_al_mac_address[5]);

	// Fill all the needed TLVs
	//
	_obtainLocalMetricsTLVs(destination, specific_neighbor, metrics_type,
	                        &tx_tlvs, &rx_tlvs, &total_tlvs);

	// Build the CMDU
	//
	response_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	response_message.message_type    = CMDU_TYPE_LINK_METRIC_RESPONSE;
	response_message.message_id      = mid;
	response_message.relay_indicator = 0;

	if (NULL == tx_tlvs || NULL == rx_tlvs) {
		response_message.list_of_TLVs = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * (total_tlvs + 1));
	} else {
		response_message.list_of_TLVs = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * ((2 * total_tlvs) + 1));
	}

	j = 0;
	if (NULL != tx_tlvs) {
		for (i = 0; i < total_tlvs; i++) {
			response_message.list_of_TLVs[j++] = (INT8U *)(tx_tlvs[i]);
		}
	}
	if (NULL != rx_tlvs) {
		for (i = 0; i < total_tlvs; i++) {
			response_message.list_of_TLVs[j++] = (INT8U *)(rx_tlvs[i]);
		}
	}

	response_message.list_of_TLVs[j++] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &response_message, false, 0)) {
		PLATFORM_PRINTF_ERROR("Could not allocate memory for TLV buffer\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free all allocated (and no longer needed) memory
	//
	_freeLocalMetricsTLVs(&tx_tlvs, &rx_tlvs, &total_tlvs);

	PLATFORM_FREE(response_message.list_of_TLVs);

	return ret;
}

INT8U send1905PushButtonEventNotificationPacket(char *interface_name, INT16U mid, char **all_interfaces_names, INT8U *push_button_mask, INT8U nr)
{
	// The "push button event notification" message is a CMDU with two TLVs:
	//   - One AL MAC address type TLV
	//   - One push button event notification TLV
	//   - Zero or one push button generic phy event notification

	INT8U ret;

	INT8U mcast_address[] = MCAST_1905;

	INT8U media_types_nr;
	INT8U i, j;

	struct CMDU                           notification_message;
	struct alMacAddressTypeTLV            al_mac_addr_tlv;
	struct pushButtonEventNotificationTLV pb_event_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION (%s) MID: (%04x)\n", interface_name, mid);

	// Fill the AL MAC address type TLV
	//
	_obtainLocalAlMacAddressTLV(&al_mac_addr_tlv);

	// Fill the push button event notification TLV
	//
	media_types_nr = 0;
	for (i = 0; i < nr; i++) {
		if (0 == push_button_mask[i]) {
			media_types_nr++;
		}
	}

	pb_event_tlv.tlv_type       = TLV_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION;
	pb_event_tlv.media_types_nr = media_types_nr;

	if (media_types_nr > 0) {
		pb_event_tlv.media_types = (struct _mediaTypeEntries *)PLATFORM_MALLOC(sizeof(struct _mediaTypeEntries) * media_types_nr);
	} else {
		pb_event_tlv.media_types = NULL;
		return 0;
	}

	j = 0;
	for (i = 0; i < nr; i++) {
		if (0 == push_button_mask[i]) {
			struct interfaceInfo *x;

			x = PLATFORM_GET_1905_INTERFACE_INFO(all_interfaces_names[i]);
			if (NULL == x) {
				PLATFORM_PRINTF_WARNING("Could not retrieve info of interface %s\n", all_interfaces_names[i]);

				pb_event_tlv.media_types[j].media_type               = MEDIA_TYPE_UNKNOWN;
				pb_event_tlv.media_types[j].media_specific_data_size = 0;
			} else {
				// "Translate" from "INTERFACE_TYPE_*" to "MEDIA_TYPE_*"
				//
				switch (x->interface_type) {
				case INTERFACE_TYPE_IEEE_802_3U_FAST_ETHERNET: {
					pb_event_tlv.media_types[j].media_type = MEDIA_TYPE_IEEE_802_3U_FAST_ETHERNET;
					break;
				}
				case INTERFACE_TYPE_IEEE_802_3AB_GIGABIT_ETHERNET: {
					pb_event_tlv.media_types[j].media_type = MEDIA_TYPE_IEEE_802_3AB_GIGABIT_ETHERNET;
					break;
				}
				case INTERFACE_TYPE_IEEE_802_11B_2_4_GHZ: {
					pb_event_tlv.media_types[j].media_type = MEDIA_TYPE_IEEE_802_11B_2_4_GHZ;
					break;
				}
				case INTERFACE_TYPE_IEEE_802_11G_2_4_GHZ: {
					pb_event_tlv.media_types[j].media_type = MEDIA_TYPE_IEEE_802_11G_2_4_GHZ;
					break;
				}
				case INTERFACE_TYPE_IEEE_802_11A_5_GHZ: {
					pb_event_tlv.media_types[j].media_type = MEDIA_TYPE_IEEE_802_11A_5_GHZ;
					break;
				}
				case INTERFACE_TYPE_IEEE_802_11N_2_4_GHZ: {
					pb_event_tlv.media_types[j].media_type = MEDIA_TYPE_IEEE_802_11N_2_4_GHZ;
					break;
				}
				case INTERFACE_TYPE_IEEE_802_11N_5_GHZ: {
					pb_event_tlv.media_types[j].media_type = MEDIA_TYPE_IEEE_802_11N_5_GHZ;
					break;
				}
				case INTERFACE_TYPE_IEEE_802_11AC_5_GHZ: {
					pb_event_tlv.media_types[j].media_type = MEDIA_TYPE_IEEE_802_11AC_5_GHZ;
					break;
				}
				case INTERFACE_TYPE_IEEE_802_11AD_60_GHZ: {
					pb_event_tlv.media_types[j].media_type = MEDIA_TYPE_IEEE_802_11AD_60_GHZ;
					break;
				}
				case INTERFACE_TYPE_IEEE_802_11AF: {
					pb_event_tlv.media_types[j].media_type = MEDIA_TYPE_IEEE_802_11AF;
					break;
				}
				case INTERFACE_TYPE_IEEE_802_11AX: {
					pb_event_tlv.media_types[j].media_type = MEDIA_TYPE_IEEE_802_11AX;
					break;
				}
				case INTERFACE_TYPE_IEEE_1901_WAVELET: {
					pb_event_tlv.media_types[j].media_type = MEDIA_TYPE_IEEE_1901_WAVELET;
					break;
				}
				case INTERFACE_TYPE_IEEE_1901_FFT: {
					pb_event_tlv.media_types[j].media_type = MEDIA_TYPE_IEEE_1901_FFT;
					break;
				}
				case INTERFACE_TYPE_MOCA_V1_1: {
					pb_event_tlv.media_types[j].media_type = MEDIA_TYPE_MOCA_V1_1;
					break;
				}
				case INTERFACE_TYPE_UNKNOWN: {
					pb_event_tlv.media_types[j].media_type = MEDIA_TYPE_UNKNOWN;
					break;
				}
				}

				// Fill the rest of media specific fields
				//
				switch (pb_event_tlv.media_types[j].media_type) {
				case MEDIA_TYPE_IEEE_802_3U_FAST_ETHERNET:
				case MEDIA_TYPE_IEEE_802_3AB_GIGABIT_ETHERNET:
				case MEDIA_TYPE_MOCA_V1_1: {
					// These interfaces don't require extra data
					//
					pb_event_tlv.media_types[j].media_specific_data_size = 0;

					break;
				}
				case MEDIA_TYPE_IEEE_802_11B_2_4_GHZ:
				case MEDIA_TYPE_IEEE_802_11G_2_4_GHZ:
				case MEDIA_TYPE_IEEE_802_11A_5_GHZ:
				case MEDIA_TYPE_IEEE_802_11N_2_4_GHZ:
				case MEDIA_TYPE_IEEE_802_11N_5_GHZ:
				case MEDIA_TYPE_IEEE_802_11AC_5_GHZ:
				case MEDIA_TYPE_IEEE_802_11AD_60_GHZ:
				case MEDIA_TYPE_IEEE_802_11AF:
				case MEDIA_TYPE_IEEE_802_11AX: {
					pb_event_tlv.media_types[j].media_specific_data_size                                          = 10;
					pb_event_tlv.media_types[j].media_specific_data.ieee80211.network_membership[0]               = x->interface_type_data.ieee80211.bssid[0];
					pb_event_tlv.media_types[j].media_specific_data.ieee80211.network_membership[1]               = x->interface_type_data.ieee80211.bssid[1];
					pb_event_tlv.media_types[j].media_specific_data.ieee80211.network_membership[2]               = x->interface_type_data.ieee80211.bssid[2];
					pb_event_tlv.media_types[j].media_specific_data.ieee80211.network_membership[3]               = x->interface_type_data.ieee80211.bssid[3];
					pb_event_tlv.media_types[j].media_specific_data.ieee80211.network_membership[4]               = x->interface_type_data.ieee80211.bssid[4];
					pb_event_tlv.media_types[j].media_specific_data.ieee80211.network_membership[5]               = x->interface_type_data.ieee80211.bssid[5];
					pb_event_tlv.media_types[j].media_specific_data.ieee80211.role                                = x->interface_type_data.ieee80211.role;
					pb_event_tlv.media_types[j].media_specific_data.ieee80211.ap_channel_band                     = x->interface_type_data.ieee80211.ap_channel_band;
					pb_event_tlv.media_types[j].media_specific_data.ieee80211.ap_channel_center_frequency_index_1 = x->interface_type_data.ieee80211.ap_channel_center_frequency_index_1;
					pb_event_tlv.media_types[j].media_specific_data.ieee80211.ap_channel_center_frequency_index_2 = x->interface_type_data.ieee80211.ap_channel_center_frequency_index_2;

					break;
				}
				case MEDIA_TYPE_IEEE_1901_WAVELET:
				case MEDIA_TYPE_IEEE_1901_FFT: {
					pb_event_tlv.media_types[j].media_specific_data_size                           = 7;
					pb_event_tlv.media_types[j].media_specific_data.ieee1901.network_identifier[0] = x->interface_type_data.ieee1901.network_identifier[0];
					pb_event_tlv.media_types[j].media_specific_data.ieee1901.network_identifier[1] = x->interface_type_data.ieee1901.network_identifier[1];
					pb_event_tlv.media_types[j].media_specific_data.ieee1901.network_identifier[2] = x->interface_type_data.ieee1901.network_identifier[2];
					pb_event_tlv.media_types[j].media_specific_data.ieee1901.network_identifier[3] = x->interface_type_data.ieee1901.network_identifier[3];
					pb_event_tlv.media_types[j].media_specific_data.ieee1901.network_identifier[4] = x->interface_type_data.ieee1901.network_identifier[4];
					pb_event_tlv.media_types[j].media_specific_data.ieee1901.network_identifier[5] = x->interface_type_data.ieee1901.network_identifier[5];
					pb_event_tlv.media_types[j].media_specific_data.ieee1901.network_identifier[6] = x->interface_type_data.ieee1901.network_identifier[6];

					break;
				}
				case MEDIA_TYPE_UNKNOWN: {
					// Do not include extra data here. It will be included
					// in the accompanying "push button generic phy
					// notification TVL"
					//
					pb_event_tlv.media_types[j].media_specific_data_size = 0;

					break;
				}
				}
			}
			j++;

			if (NULL != x) {
				PLATFORM_FREE_1905_INTERFACE_INFO(x);
			}
		}
	}

	// Build the CMDU
	//
	notification_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	notification_message.message_type    = CMDU_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION;
	notification_message.message_id      = mid;
	notification_message.relay_indicator = 1;

	notification_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 3);
	notification_message.list_of_TLVs[2] = NULL;

	notification_message.list_of_TLVs[0] = (INT8U *)&al_mac_addr_tlv;
	notification_message.list_of_TLVs[1] = (INT8U *)&pb_event_tlv;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, mcast_address, &notification_message, false, 0)) {
		PLATFORM_PRINTF_ERROR("Could not send the 1905 packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	_freeLocalAlMacAddressTLV(&al_mac_addr_tlv);

	if (media_types_nr > 0) {
		PLATFORM_FREE(pb_event_tlv.media_types);
	}

	PLATFORM_FREE(notification_message.list_of_TLVs);

	return ret;
}

INT8U send1905PushButtonJoinNotificationPacket(char *interface_name, INT16U mid, INT8U *original_al_mac_address, INT16U original_mid, INT8U *local_mac_address, INT8U *new_mac_address)
{
	// The "push button join notification" message is a CMDU with two TLVs:
	//   - One AL MAC address type TLV
	//   - One push button join notification TLV

	INT8U ret;

	INT8U al_mac_address[6];
	INT8U mcast_address[] = MCAST_1905;

	struct CMDU                          notification_message;
	struct alMacAddressTypeTLV           al_mac_addr_tlv;
	struct pushButtonJoinNotificationTLV pb_join_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION (%s) MID: (%04x)\n", interface_name, mid);

	PLATFORM_MEMCPY(al_mac_address, DMalMacGet(), 6);

	// Fill the AL MAC address type TLV
	//
	_obtainLocalAlMacAddressTLV(&al_mac_addr_tlv);

	// Fill the push button join notification TLV
	//
	pb_join_tlv.tlv_type           = TLV_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION;
	pb_join_tlv.al_mac_address[0]  = original_al_mac_address[0];
	pb_join_tlv.al_mac_address[1]  = original_al_mac_address[1];
	pb_join_tlv.al_mac_address[2]  = original_al_mac_address[2];
	pb_join_tlv.al_mac_address[3]  = original_al_mac_address[3];
	pb_join_tlv.al_mac_address[4]  = original_al_mac_address[4];
	pb_join_tlv.al_mac_address[5]  = original_al_mac_address[5];
	pb_join_tlv.message_identifier = original_mid;
	pb_join_tlv.mac_address[0]     = local_mac_address[0];
	pb_join_tlv.mac_address[1]     = local_mac_address[1];
	pb_join_tlv.mac_address[2]     = local_mac_address[2];
	pb_join_tlv.mac_address[3]     = local_mac_address[3];
	pb_join_tlv.mac_address[4]     = local_mac_address[4];
	pb_join_tlv.mac_address[5]     = local_mac_address[5];
	pb_join_tlv.new_mac_address[0] = local_mac_address[0];
	pb_join_tlv.new_mac_address[1] = new_mac_address[1];
	pb_join_tlv.new_mac_address[2] = new_mac_address[2];
	pb_join_tlv.new_mac_address[3] = new_mac_address[3];
	pb_join_tlv.new_mac_address[4] = new_mac_address[4];
	pb_join_tlv.new_mac_address[5] = new_mac_address[5];

	// Build the CMDU
	//
	notification_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	notification_message.message_type    = CMDU_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION;
	notification_message.message_id      = mid;
	notification_message.relay_indicator = 1;
	notification_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 3);
	notification_message.list_of_TLVs[0] = (INT8U *)&al_mac_addr_tlv;
	notification_message.list_of_TLVs[1] = (INT8U *)&pb_join_tlv;
	notification_message.list_of_TLVs[2] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, mcast_address, &notification_message, false, 0)) {
		PLATFORM_PRINTF_ERROR("Could not send the 1905 packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	_freeLocalAlMacAddressTLV(&al_mac_addr_tlv);

	PLATFORM_FREE(notification_message.list_of_TLVs);

	return ret;
}

INT8U send1905APAutoconfigurationSearchPacket(char *interface_name, INT16U mid, INT8U freq_band)
{
	// The "AP-autoconfiguration search" message is a CMDU with three TLVs:
	//   - One AL MAC address type TLV
	//   - One searched role TLV
	//   - One autoconfig freq band TLV

	INT8U ret, tlvs = 0;

	INT8U mcast_address[] = MCAST_1905;

	struct CMDU                  search_message;
	struct alMacAddressTypeTLV   al_mac_addr_tlv;
	struct searchedRoleTLV       searched_role_tlv;
	struct autoconfigFreqBandTLV ac_freq_band_tlv;

	// For Multi-AP
	struct SupportedServiceTLV supported_service_tlv;
	struct SearchedServiceTLV  searched_service_tlv;
#ifdef RTK_MULTI_AP_R2
	struct MultiAPProfileTLV profile_tlv;
#endif
#ifdef RTK_MULTI_AP_R3
	struct DPPChirpValueTLV *chirp_value_tlv;
#endif

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_AP_AUTOCONFIGURATION_SEARCH (%s) MID: (%04x)\n", interface_name, mid);

	// Fill the AL MAC address type TLV
	//
	_obtainLocalAlMacAddressTLV(&al_mac_addr_tlv);

	// Fill the searched role TLV
	//
	searched_role_tlv.tlv_type = TLV_TYPE_SEARCHED_ROLE;
	searched_role_tlv.role     = IEEE80211_ROLE_AP;

	// Fill the autoconfig freq band TLV
	//
	ac_freq_band_tlv.tlv_type  = TLV_TYPE_AUTOCONFIG_FREQ_BAND;
	ac_freq_band_tlv.freq_band = freq_band;

	// Fill the Multi-AP TLVs
	_obtainSupportedServiceTLV(&supported_service_tlv);
	_obtainSearchedServiceTLV(&searched_service_tlv);
#ifdef RTK_MULTI_AP_R2
	_obtainProfileTLV(&profile_tlv);
#endif
#ifdef RTK_MULTI_AP_R3
	chirp_value_tlv = (struct DPPChirpValueTLV *)PLATFORM_MALLOC(sizeof(struct DPPChirpValueTLV));
	INT8U *chirp = DMgetOwnDPPBootstrapInfo()->pubkey_hash_chirp;
	buildDPPChirpValueTLV(chirp_value_tlv, NULL, chirp, SHA256_MAC_LEN);
#endif

	// Build the CMDU
	//
	search_message.message_version      = CMDU_MESSAGE_VERSION_1905_1_2013;
	search_message.message_type         = CMDU_TYPE_AP_AUTOCONFIGURATION_SEARCH;
	search_message.message_id           = mid;
	search_message.relay_indicator      = 1;
	search_message.list_of_TLVs         = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 6);
	search_message.list_of_TLVs[tlvs++] = (INT8U *)&al_mac_addr_tlv;
	search_message.list_of_TLVs[tlvs++] = (INT8U *)&searched_role_tlv;
	search_message.list_of_TLVs[tlvs++] = (INT8U *)&ac_freq_band_tlv;
	search_message.list_of_TLVs[tlvs++] = (INT8U *)&supported_service_tlv;
	search_message.list_of_TLVs[tlvs++] = (INT8U *)&searched_service_tlv;
	search_message.list_of_TLVs[tlvs++] = NULL;
#ifdef RTK_MULTI_AP_R2
	if (MULTI_AP_PROFILE_2 <= DMgetMultiAPProfile()) {
		search_message.list_of_TLVs           = (INT8U **)PLATFORM_REALLOC(search_message.list_of_TLVs, sizeof(INT8U *) * (tlvs + 1));
		search_message.list_of_TLVs[tlvs - 1] = (INT8U *)&profile_tlv;
		search_message.list_of_TLVs[tlvs++]   = NULL;
	}
#endif
#ifdef RTK_MULTI_AP_R3
	if (MULTI_AP_PROFILE_3 <= DMgetMultiAPProfile()) {
		search_message.list_of_TLVs           = (INT8U **)PLATFORM_REALLOC(search_message.list_of_TLVs, sizeof(INT8U *) * (tlvs + 1));
		search_message.list_of_TLVs[tlvs - 1] = (INT8U *)chirp_value_tlv;
		search_message.list_of_TLVs[tlvs++]   = NULL;
	}
#endif

	if (0 == send1905RawPacket(interface_name, mid, mcast_address, &search_message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	_freeLocalAlMacAddressTLV(&al_mac_addr_tlv);
	_freeSupportedServiceTLV(&supported_service_tlv);
	_freeSearchedServiceTLV(&searched_service_tlv);
#ifdef RTK_MULTI_AP_R3
	free_1905_TLV_structure((INT8U *)chirp_value_tlv);
#endif
	PLATFORM_FREE(search_message.list_of_TLVs);
	return ret;
}

INT8U send1905APAutoconfigurationRenewPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, unsigned char band)
{
	// The "AP-autoconfiguration search" message is a CMDU with three TLVs:
	//   - One AL MAC address type TLV
	//   - One searched role TLV
	//   - One autoconfig freq band TLV

	INT8U ret;

	INT8U mcast_address[] = MCAST_1905;

	struct CMDU                 search_message;
	struct alMacAddressTypeTLV  al_mac_addr_tlv;
	struct supportedRoleTLV     supported_role_tlv;
	struct supportedFreqBandTLV supported_freq_band_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_AP_AUTOCONFIGURATION_RENEW for radio (%d) (%s) MID: (%04x)\n", band, interface_name, mid);

	// Fill the AL MAC address type TLV
	//
	_obtainLocalAlMacAddressTLV(&al_mac_addr_tlv);

	// Fill the supported role TLV
	//
	supported_role_tlv.tlv_type = TLV_TYPE_SUPPORTED_ROLE;
	supported_role_tlv.role     = IEEE80211_ROLE_AP;

	// Fill the supported freq band TLV
	//
	supported_freq_band_tlv.tlv_type  = TLV_TYPE_SUPPORTED_FREQ_BAND;
	supported_freq_band_tlv.freq_band = band;

	// Build the CMDU
	//
	search_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	search_message.message_type    = CMDU_TYPE_AP_AUTOCONFIGURATION_RENEW;
	search_message.message_id      = mid;
	search_message.relay_indicator = 1;

	if (0 == PLATFORM_STRCMP("lo", interface_name) || PLATFORM_MEMCMP(mcast_address, destination_al_mac_address, 6)) {
		search_message.relay_indicator = 0;
	}

	search_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 4);
	search_message.list_of_TLVs[0] = (INT8U *)&al_mac_addr_tlv;
	search_message.list_of_TLVs[1] = (INT8U *)&supported_role_tlv;
	search_message.list_of_TLVs[2] = (INT8U *)&supported_freq_band_tlv;
	search_message.list_of_TLVs[3] = NULL;

	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &search_message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet for radio (%d)\n", supported_freq_band_tlv.freq_band);
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	PLATFORM_FREE(search_message.list_of_TLVs);

	return ret;
}

INT8U send1905APAutoconfigurationResponsePacket(char *interface_name, INT16U mid,
                                                INT8U *destination_al_mac_address, INT8U freq_band, INT8U dpp_chirp)
{
	// The "AP-autoconfiguration response" message is a CMDU with two TLVs:
	//   - One supported role TLV
	//   - One supported freq band TLV

	INT8U ret, tlvs = 0;

	struct CMDU                 response_message;
	struct supportedRoleTLV     supported_role_tlv;
	struct supportedFreqBandTLV supported_freq_band_tlv;

	// For Multi-AP
	struct SupportedServiceTLV supported_service_tlv;
#ifdef RTK_MULTI_AP_R3
	struct DPPChirpValueTLV *chirp_value_tlv;
	struct SecurityCapabilityTLV security_capability_tlv;
#endif

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_AP_AUTOCONFIGURATION_RESPONSE (%s) MID: (%04x)\n", interface_name, mid);

	// Fill the supported role TLV
	//
	supported_role_tlv.tlv_type = TLV_TYPE_SUPPORTED_ROLE;
	supported_role_tlv.role     = IEEE80211_ROLE_AP;

	// Fill the supported freq band TLV
	//
	supported_freq_band_tlv.tlv_type  = TLV_TYPE_SUPPORTED_FREQ_BAND;
	supported_freq_band_tlv.freq_band = freq_band;

	// Fill the Multi-AP TLVs

	// fill supported serviced TLV
	_obtainSupportedServiceTLV(&supported_service_tlv);

#ifdef RTK_MULTI_AP_R2
	struct MultiAPProfileTLV profile_tlv;
	_obtainProfileTLV(&profile_tlv);
#endif
#ifdef RTK_MULTI_AP_R3
	chirp_value_tlv = (struct DPPChirpValueTLV *)PLATFORM_MALLOC(sizeof(struct DPPChirpValueTLV));
	_obtainDPPChirpValueTLV(chirp_value_tlv, 0, NULL); // respond with enrollee's (peer) chirp value
	_obtainSecurityCapabilityTLV(&security_capability_tlv);
#endif

	// Build the CMDU
	//
	response_message.message_version      = CMDU_MESSAGE_VERSION_1905_1_2013;
	response_message.message_type         = CMDU_TYPE_AP_AUTOCONFIGURATION_RESPONSE;
	response_message.message_id           = mid;
	response_message.relay_indicator      = 0;
	response_message.list_of_TLVs         = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 4);
	response_message.list_of_TLVs[tlvs++] = (INT8U *)&supported_role_tlv;
	response_message.list_of_TLVs[tlvs++] = (INT8U *)&supported_freq_band_tlv;
	response_message.list_of_TLVs[tlvs++] = (INT8U *)&supported_service_tlv;
	response_message.list_of_TLVs[tlvs++] = NULL;
#ifdef RTK_MULTI_AP_R2
	if (checkTargetDeviceProfile2TLVCapability(destination_al_mac_address)) {
		response_message.list_of_TLVs           = (INT8U **)PLATFORM_REALLOC(response_message.list_of_TLVs, sizeof(INT8U *) * (tlvs + 1));
		response_message.list_of_TLVs[tlvs - 1] = (INT8U *)&profile_tlv;
		response_message.list_of_TLVs[tlvs++]   = NULL;
	}
#endif
	if (dpp_chirp) {
#ifdef RTK_MULTI_AP_R3
		response_message.list_of_TLVs           = (INT8U **)PLATFORM_REALLOC(response_message.list_of_TLVs, sizeof(INT8U *) * (tlvs + 2));
		response_message.list_of_TLVs[tlvs - 1] = (INT8U *)chirp_value_tlv;
		response_message.list_of_TLVs[tlvs++]   = (INT8U *)&security_capability_tlv;
		response_message.list_of_TLVs[tlvs++]   = NULL;
#endif
	}

	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &response_message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	_freeSupportedServiceTLV(&supported_service_tlv);
#ifdef RTK_MULTI_AP_R3
	free_1905_TLV_structure((INT8U *)chirp_value_tlv);
#endif
	PLATFORM_FREE(response_message.list_of_TLVs);

	return ret;
}

INT8U send1905APAutoconfigurationWSCPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address,
                                           INT8U **wsc_frame, INT16U *wsc_frame_size, INT8U wsc_nr,
                                           INT8U *radio_identifier, char *radio_name,
                                           struct TrafficSeparationPolicyTLV *traffic_separation_policy_tlv,
                                           INT8U vendor_tlv_nr, struct vendorSpecificTLV *vendor_tlvs)
{
	// The "AP-autoconfiguration WSC" message is a CMDU with just one TLVs:
	//   - One WSC TLV

	INT8U ret;

	INT8U total_tlv_nr = 0;

	struct CMDU                        data_message;
	struct wscTLV *                    wsc_tlv;
	struct ApRadioIdentifierTLV        radio_identifier_tlv;
	struct APRadioBasicCapabilitiesTLV radio_basic_tlv = { 0 };
#ifdef RTK_MULTI_AP_R2
	struct Profile2APCapabilityTLV        r2_ap_capability_tlv;
	struct APRadioAdvancedCapabilitiesTLV advanced_capabilities_tlv;
	struct Default8021QSettingsTLV        default_8021q_tlv;

	default_8021q_tlv.tlv_type  = TLV_TYPE_DEFAULT_802_1Q_SETTINGS;
	INT8U include_default_8021q = 0;
#endif
	int i = 0;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_AP_AUTOCONFIGURATION_WSC (%s) MID: (%04x)\n", interface_name, mid);

#ifdef RTK_MULTI_AP_R2
	DMgetDefault8021QSettings(&default_8021q_tlv.primary_vlan_id, &default_8021q_tlv.default_pcp);
	if (default_8021q_tlv.primary_vlan_id || default_8021q_tlv.default_pcp) {
		include_default_8021q = 1;
	}
#endif

	if (DMisController()) {
		total_tlv_nr = 2 + wsc_nr; // ApRadioIdentifierTLV + WSC TLVs + Profile-2 AP Capability TLV + AP Radio Advanced Capabilities TLV + End of Message TLV
#ifdef RTK_MULTI_AP_R2
		if (MULTI_AP_PROFILE_2 <= DMgetMultiAPProfile() && MULTI_AP_PROFILE_2 <= DMgetTargetDeviceMultiAPProfile(destination_al_mac_address)) {
			if (include_default_8021q) {
				total_tlv_nr += 1;
			}
			if (traffic_separation_policy_tlv) {
				total_tlv_nr += 1;
			}
		}
#else
		if (traffic_separation_policy_tlv) {
			PLATFORM_PRINTF_WARNING("There should not be traffic_separation_policy_tlv in R1\n");
		}
#endif
	} else {
		total_tlv_nr = 2 + wsc_nr; // APRadioBasicCapabilitiesTLV + WSC TLVs + End of Message TLV
#ifdef RTK_MULTI_AP_R2
		if (MULTI_AP_PROFILE_2 <= DMgetMultiAPProfile() && MULTI_AP_PROFILE_2 <= DMgetTargetDeviceMultiAPProfile(destination_al_mac_address)) {
			//Default 802.1Q + Traffic Separation Policy TLV
			total_tlv_nr += 2;
		}
#endif
	}

	total_tlv_nr += vendor_tlv_nr;

	// Build the CMDU
	//
	data_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	data_message.message_type    = CMDU_TYPE_AP_AUTOCONFIGURATION_WSC;
	data_message.message_id      = mid;
	data_message.relay_indicator = 0;
	data_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * total_tlv_nr);

	wsc_tlv = (struct wscTLV *)PLATFORM_MALLOC(sizeof(struct wscTLV) * wsc_nr);

	for (i = 0; i < wsc_nr; i++) {
		wsc_tlv[i].tlv_type       = TLV_TYPE_WSC;
		wsc_tlv[i].wsc_frame_size = wsc_frame_size[i];
		wsc_tlv[i].wsc_frame      = (INT8U *)PLATFORM_MALLOC(wsc_frame_size[i]);
		PLATFORM_MEMCPY(wsc_tlv[i].wsc_frame, wsc_frame[i], wsc_frame_size[i]);
		data_message.list_of_TLVs[i] = (INT8U *)&wsc_tlv[i];
	}

	// Build APRadioBasicCapabilitiesTLV or ApRadioIdentifierTLV
	INT8U tlv_index = wsc_nr;

	if (DMisController()) {
		radio_identifier_tlv.tlv_type = TLV_TYPE_AP_RADIO_IDENTIFIER;
		PLATFORM_MEMCPY(radio_identifier_tlv.radio_unique_id, radio_identifier, 6);
		data_message.list_of_TLVs[tlv_index++] = (INT8U *)&radio_identifier_tlv;
#ifdef RTK_MULTI_AP_R2
		if (MULTI_AP_PROFILE_2 <= DMgetMultiAPProfile() && MULTI_AP_PROFILE_2 <= DMgetTargetDeviceMultiAPProfile(destination_al_mac_address)) {
			if (include_default_8021q) {
				data_message.list_of_TLVs[tlv_index++] = (INT8U *)&default_8021q_tlv;
			}

			if (traffic_separation_policy_tlv) {
				data_message.list_of_TLVs[tlv_index++] = (INT8U *)traffic_separation_policy_tlv;
			}
		}
#endif
	} else {
		_obtainAPRadioBasicCapabilitiesTLV(&radio_basic_tlv, radio_name);
		data_message.list_of_TLVs[tlv_index++] = (INT8U *)&radio_basic_tlv;
#ifdef RTK_MULTI_AP_R2
		if (MULTI_AP_PROFILE_2 <= DMgetMultiAPProfile() && MULTI_AP_PROFILE_2 <= DMgetTargetDeviceMultiAPProfile(destination_al_mac_address)) {
			_obtainProfile2APCapabilityTLV(&r2_ap_capability_tlv);
			data_message.list_of_TLVs[tlv_index++] = (INT8U *)&r2_ap_capability_tlv;
			_obtainRadioAdvancedCapabilitiesTLV(&advanced_capabilities_tlv, radio_name);
			data_message.list_of_TLVs[tlv_index++] = (INT8U *)&advanced_capabilities_tlv;
		}
#endif
	}

	for (i = 0; i < vendor_tlv_nr; i++) {
		data_message.list_of_TLVs[tlv_index++] = (INT8U *)&vendor_tlvs[i];
	}

	data_message.list_of_TLVs[total_tlv_nr - 1] = NULL;

	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &data_message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	_freeAPRadioBasicCapabilitiesTLV(&radio_basic_tlv);
	for (i = 0; i < wsc_nr; i++) {
		PLATFORM_FREE(wsc_tlv[i].wsc_frame);
	}
	PLATFORM_FREE(wsc_tlv);
	PLATFORM_FREE(data_message.list_of_TLVs);

	return ret;
}

INT8U sendLLDPBridgeDiscoveryPacket(char *interface_name)
{
	INT8U al_mac_address[6];
	INT8U interface_mac_address[6];

	struct chassisIdTLV      chassis_id_tlv;
	struct portIdTLV         port_id_tlv;
	struct timeToLiveTypeTLV time_to_live_tlv;

	struct PAYLOAD payload;

	INT8U *stream;
	INT16U stream_len;

	PLATFORM_PRINTF_INFO("--> LLDP BRIDGE DISCOVERY (%s)\n", interface_name);

	PLATFORM_MEMCPY(al_mac_address, DMalMacGet(), 6);
	PLATFORM_MEMCPY(interface_mac_address, DMinterfaceNameToMac(interface_name), 6);

	// Fill the chassis ID TLV
	//
	chassis_id_tlv.tlv_type           = TLV_TYPE_CHASSIS_ID;
	chassis_id_tlv.chassis_id_subtype = CHASSIS_ID_TLV_SUBTYPE_MAC_ADDRESS;
	chassis_id_tlv.chassis_id[0]      = al_mac_address[0];
	chassis_id_tlv.chassis_id[1]      = al_mac_address[1];
	chassis_id_tlv.chassis_id[2]      = al_mac_address[2];
	chassis_id_tlv.chassis_id[3]      = al_mac_address[3];
	chassis_id_tlv.chassis_id[4]      = al_mac_address[4];
	chassis_id_tlv.chassis_id[5]      = al_mac_address[5];

	// Fill the port ID TLV
	//
	port_id_tlv.tlv_type        = TLV_TYPE_PORT_ID;
	port_id_tlv.port_id_subtype = PORT_ID_TLV_SUBTYPE_MAC_ADDRESS;
	port_id_tlv.port_id[0]      = interface_mac_address[0];
	port_id_tlv.port_id[1]      = interface_mac_address[1];
	port_id_tlv.port_id[2]      = interface_mac_address[2];
	port_id_tlv.port_id[3]      = interface_mac_address[3];
	port_id_tlv.port_id[4]      = interface_mac_address[4];
	port_id_tlv.port_id[5]      = interface_mac_address[5];

	// Fill the time to live TLV
	//
	time_to_live_tlv.tlv_type = TLV_TYPE_TIME_TO_LIVE;
	time_to_live_tlv.ttl      = TIME_TO_LIVE_TLV_1905_DEFAULT_VALUE;

	// Forge the LLDP payload containing all these TLVs
	//
	payload.list_of_TLVs[0] = (INT8U *)&chassis_id_tlv;
	payload.list_of_TLVs[1] = (INT8U *)&port_id_tlv;
	payload.list_of_TLVs[2] = (INT8U *)&time_to_live_tlv;
	payload.list_of_TLVs[3] = NULL;

	stream = forge_lldp_PAYLOAD_from_structure(&payload, &stream_len);

	// Finally, send the packet!
	//
	{
		INT8U mcast_address[] = MCAST_LLDP;

		PLATFORM_PRINTF_DETAIL("Sending LLDP bridge discovery message on interface %s\n", interface_name);
		if (0 == PLATFORM_SEND_RAW_PACKET(interface_name, mcast_address, interface_mac_address, ETHERTYPE_LLDP, stream, stream_len)) {
			PLATFORM_PRINTF_ERROR("Packet could not be sent!\n");
		}
	}

	// Free memory
	//
	free_lldp_PAYLOAD_packet(stream);

	return 1;
}

INT8U sendAPCapabilityQueryPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address)
{
	// The "AP capability query" message is a CMDU with no TLVs

	INT8U ret;

	struct CMDU query_message;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_AP_CAPABILITY_QUERY (%s) MID: (%04x)\n", interface_name, mid);

	// Build the CMDU
	//
	query_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	query_message.message_type    = CMDU_TYPE_AP_CAPABILITY_QUERY;
	query_message.message_id      = mid;
	query_message.relay_indicator = 0;
	query_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *));
	query_message.list_of_TLVs[0] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &query_message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	PLATFORM_FREE(query_message.list_of_TLVs);

	return ret;
}

INT8U sendAPCapabilityReportPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address)
{
	// The "AP capability report" message is a CMDU with TLVs

	INT8U ret, ifCount;
	INT8U basicCap_tlv_count = 0, htCap_tlv_count = 0, vhtCap_tlv_count = 0, heCap_tlv_count = 0;
#ifdef RTK_MULTI_AP_R3
	INT8U wifi6Cap_tlv_count = 0;
#endif

	struct CMDU                         report_message;
	struct APCapabilityTLV              ap_capability_tlv;
	struct APRadioBasicCapabilitiesTLV *ap_basic_capability_tlvs = NULL;
	struct APHTCapabilitiesTLV *        ap_ht_capability_tlvs    = NULL;
	struct APVHTCapabilitiesTLV *       ap_vht_capability_tlvs   = NULL;
	struct APHECapabilitiesTLV *        ap_he_capability_tlvs    = NULL;
#ifdef RTK_MULTI_AP_R2
	struct ChannelScanCapabilitiesTLV  channel_scan_capabilities_tlv  = { 0 };
	struct CACCapabilitiesTLV          cac_capabilities_tlv           = { 0 };
	struct Profile2APCapabilityTLV     r2_ap_capability_tlv           = { 0 };
	struct MetricCollectionIntervalTLV metric_collection_interval_tlv = { 0 };
#endif
#ifdef RTK_MULTI_AP_R3
	struct APWiFi6CapabilitiesTLV **ap_wifi6_capability_tlvs = NULL;
	struct DeviceInventoryTLV *     device_inventory_tlv     = NULL;
	struct SecurityCapabilityTLV    security_capability_tlv;
#endif
	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_AP_CAPABILITY_REPORT (%s) MID: (%04x)\n", interface_name, mid);

	char **interfaces_names;
	INT8U  interfaces_nr, i, variable_tlv_length, tlv_index;
	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_nr);
	ifCount          = 0;

	_obtainAPCapabilityTLV(&ap_capability_tlv);

	for (i = 0; i < interfaces_nr; i++) {
		struct interfaceInfo *x;
		x = PLATFORM_GET_1905_INTERFACE_INFO(interfaces_names[i]);
		if (NULL == x) {
			PLATFORM_PRINTF_WARNING("Could not retrieve info of interface %s\n", interfaces_names[i]);
			continue;
		}
		if (PLATFORM_STRSTR(x->name, map_vxd_prefix)
			|| PLATFORM_STRSTR(x->name, map_vap_prefix)
			|| PLATFORM_GET_VALID_INTERFACE_NAME(x->name, VALID_ETH_INTERFACES_IN_BRIDGE)
			|| (controller_interface && PLATFORM_STRSTR(x->name, controller_interface))) {
			//skip non main root interfaces
		} else {
			if (ifCount == 0) {
				ap_basic_capability_tlvs = PLATFORM_MALLOC(sizeof(struct APRadioBasicCapabilitiesTLV) * (ifCount + 1));
				ap_ht_capability_tlvs    = PLATFORM_MALLOC(sizeof(struct APHTCapabilitiesTLV) * (ifCount + 1));
				ap_vht_capability_tlvs   = PLATFORM_MALLOC(sizeof(struct APVHTCapabilitiesTLV) * (ifCount + 1));
				ap_he_capability_tlvs   = PLATFORM_MALLOC(sizeof(struct APHECapabilitiesTLV) * (ifCount + 1));
#ifdef RTK_MULTI_AP_R3
				ap_wifi6_capability_tlvs = PLATFORM_MALLOC(sizeof(struct APWiFi6CapabilitiesTLV *) * (ifCount + 1));
#endif
			} else {
				ap_basic_capability_tlvs = PLATFORM_REALLOC(ap_basic_capability_tlvs, sizeof(struct APRadioBasicCapabilitiesTLV) * (ifCount + 1));
				ap_ht_capability_tlvs    = PLATFORM_REALLOC(ap_ht_capability_tlvs, sizeof(struct APHTCapabilitiesTLV) * (ifCount + 1));
				ap_vht_capability_tlvs   = PLATFORM_REALLOC(ap_vht_capability_tlvs, sizeof(struct APVHTCapabilitiesTLV) * (ifCount + 1));
				ap_he_capability_tlvs	 = PLATFORM_REALLOC(ap_he_capability_tlvs, sizeof(struct APHECapabilitiesTLV) * (ifCount + 1));
#ifdef RTK_MULTI_AP_R3
				ap_wifi6_capability_tlvs = PLATFORM_REALLOC(ap_wifi6_capability_tlvs, sizeof(struct APWiFi6CapabilitiesTLV *) * (ifCount + 1));
#endif
			}

			_obtainAPRadioBasicCapabilitiesTLV(&ap_basic_capability_tlvs[ifCount], interfaces_names[i]);
			basicCap_tlv_count++;

			if (_obtainAPHTCapabilityTLV(&ap_ht_capability_tlvs[htCap_tlv_count], interfaces_names[i]) == 1) {
				htCap_tlv_count++;
			}
			if (_obtainAPVHTCapabilityTLV(&ap_vht_capability_tlvs[vhtCap_tlv_count], interfaces_names[i]) == 1) {
				vhtCap_tlv_count++;
			}
			if (_obtainAPHECapabilityTLV(&ap_he_capability_tlvs[heCap_tlv_count], interfaces_names[i]) == 1) {
				heCap_tlv_count++;
#ifdef RTK_MULTI_AP_R3
				// AP WiFi6 Capability TLV is also for radios that support 802.11 HE
				ap_wifi6_capability_tlvs[wifi6Cap_tlv_count] = (struct APWiFi6CapabilitiesTLV *)PLATFORM_MALLOC(sizeof(struct APWiFi6CapabilitiesTLV));
				_obtainAPWiFi6CapabilitiesTLV(ap_wifi6_capability_tlvs[wifi6Cap_tlv_count], interfaces_names[i]);
				wifi6Cap_tlv_count++;
#endif
			}

			ifCount++;
		}

		PLATFORM_FREE_1905_INTERFACE_INFO(x);
	}

	tlv_index           = 0;
	variable_tlv_length = 1 + basicCap_tlv_count + htCap_tlv_count + vhtCap_tlv_count + heCap_tlv_count + 1; // 1 ap capability + 2 variable tlv (should be 3) * interface nr + 1 null
#ifdef RTK_MULTI_AP_R2
	if (MULTI_AP_PROFILE_2 <= DMgetMultiAPProfile() && MULTI_AP_PROFILE_2 <= DMgetTargetDeviceMultiAPProfile(destination_al_mac_address)) {
		variable_tlv_length += 4; // Channel Scan Capabilities + CAC Capabilities + Profile-2 AP Capability + Metric Collection Interval
		_obtainChannelScanCapabilitiesTLV(&channel_scan_capabilities_tlv);
		_obtainCACCapabilitiesTLV(&cac_capabilities_tlv);
		_obtainProfile2APCapabilityTLV(&r2_ap_capability_tlv);
		_obtainMetricCollectionIntervalTLV(&metric_collection_interval_tlv);
	}
#endif
#ifdef RTK_MULTI_AP_R3
	if (MULTI_AP_PROFILE_3 <= DMgetMultiAPProfile() && MULTI_AP_PROFILE_3 <= DMgetTargetDeviceMultiAPProfile(destination_al_mac_address)) {
		variable_tlv_length += wifi6Cap_tlv_count;
		variable_tlv_length += 1; // Device Inventory List
		device_inventory_tlv = (struct DeviceInventoryTLV *)PLATFORM_MALLOC(sizeof(struct DeviceInventoryTLV));
		_obtainDeviceInventoryTLV(device_inventory_tlv);
		variable_tlv_length += 1; // 1905 Security Capability
		_obtainSecurityCapabilityTLV(&security_capability_tlv);
	}
#endif

	// Build the CMDU
	//
	report_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	report_message.message_type    = CMDU_TYPE_AP_CAPABILITY_REPORT;
	report_message.message_id      = mid;
	report_message.relay_indicator = 0;
	report_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * variable_tlv_length);

	report_message.list_of_TLVs[tlv_index] = (INT8U *)&ap_capability_tlv;
	tlv_index++;

	for (i = 0; i < ifCount; i++) {
		report_message.list_of_TLVs[tlv_index] = (INT8U *)&ap_basic_capability_tlvs[i];
		tlv_index++;
	}

	for (i = 0; i < htCap_tlv_count; i++) {
		report_message.list_of_TLVs[tlv_index] = (INT8U *)&ap_ht_capability_tlvs[i];
		tlv_index++;
	}

	for (i = 0; i < vhtCap_tlv_count; i++) {
		report_message.list_of_TLVs[tlv_index] = (INT8U *)&ap_vht_capability_tlvs[i];
		tlv_index++;
	}

	for (i = 0; i < heCap_tlv_count; i++) {
		report_message.list_of_TLVs[tlv_index] = (INT8U *)&ap_he_capability_tlvs[i];
		tlv_index++;
	}
#ifdef RTK_MULTI_AP_R3
	if (MULTI_AP_PROFILE_3 <= DMgetMultiAPProfile() && MULTI_AP_PROFILE_3 <= DMgetTargetDeviceMultiAPProfile(destination_al_mac_address)) {
		for (i = 0; i < wifi6Cap_tlv_count; i++) {
			report_message.list_of_TLVs[tlv_index] = (INT8U *)ap_wifi6_capability_tlvs[i];
			tlv_index++;
		}
		// Device Inventory TLV
		report_message.list_of_TLVs[tlv_index++] = (INT8U *)device_inventory_tlv;
		// 1905 Security Capability TLV
		report_message.list_of_TLVs[tlv_index++] = (INT8U *)&security_capability_tlv;
	}
#endif
#ifdef RTK_MULTI_AP_R2
	if (MULTI_AP_PROFILE_2 <= DMgetMultiAPProfile() && MULTI_AP_PROFILE_2 <= DMgetTargetDeviceMultiAPProfile(destination_al_mac_address)) {
		// Channel Scan Capabilites TLV
		report_message.list_of_TLVs[tlv_index++] = (INT8U *)&channel_scan_capabilities_tlv;
		// CAC Capabilites TLV
		report_message.list_of_TLVs[tlv_index++] = (INT8U *)&cac_capabilities_tlv;
		// Profile 2 AP Capabilites TLV
		report_message.list_of_TLVs[tlv_index++] = (INT8U *)&r2_ap_capability_tlv;
		// Metric Collection Interval TLV
		report_message.list_of_TLVs[tlv_index++] = (INT8U *)&metric_collection_interval_tlv;
	}
#endif
	//remember to null the last TLV
	report_message.list_of_TLVs[tlv_index++] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &report_message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	for (i = 0; i < basicCap_tlv_count; i++) {
		_freeAPRadioBasicCapabilitiesTLV(&ap_basic_capability_tlvs[i]);
	}

	PLATFORM_FREE(ap_basic_capability_tlvs);

	PLATFORM_FREE(ap_ht_capability_tlvs);
	PLATFORM_FREE(ap_vht_capability_tlvs);

	for (i = 0; i < heCap_tlv_count; i++) {
		if(ap_he_capability_tlvs[i].he_msc){
			PLATFORM_FREE(ap_he_capability_tlvs[i].he_msc);
		}
	}
	PLATFORM_FREE(ap_he_capability_tlvs);
#ifdef RTK_MULTI_AP_R2
	_freeChannelScanCapabilitiesTLV(&channel_scan_capabilities_tlv);
	_freeCACCapabilitiesTLV(&cac_capabilities_tlv);
#endif
#ifdef RTK_MULTI_AP_R3
	for (i = 0; i < wifi6Cap_tlv_count; i++) {
		free_1905_TLV_structure((INT8U *)ap_wifi6_capability_tlvs[i]);
	}
	PLATFORM_FREE(ap_wifi6_capability_tlvs);
	free_1905_TLV_structure((INT8U *)device_inventory_tlv);
#endif

	PLATFORM_FREE(report_message.list_of_TLVs);

	return ret;
}

INT8U sendClientCapabilityQueryPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U *client_bssid, INT8U *client_mac_address)
{
	// The "Client capability query" message is a CMDU with a Client Info TLVs

	INT8U ret;

	struct CMDU          query_message;
	struct ClientInfoTLV client_info_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_CLIENT_CAPABILITY_QUERY (%s) MID: (%04x)\n", interface_name, mid);

	//Fill up the TLV
	client_info_tlv.tlv_type = TLV_TYPE_CLIENT_INFO;
	PLATFORM_MEMCPY(client_info_tlv.bssid, client_bssid, 6);
	PLATFORM_MEMCPY(client_info_tlv.mac_address, client_mac_address, 6);

	// Build the CMDU
	//
	query_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	query_message.message_type    = CMDU_TYPE_CLIENT_CAPABILITY_QUERY;
	query_message.message_id      = mid;
	query_message.relay_indicator = 0;
	query_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);
	query_message.list_of_TLVs[0] = (INT8U *)&client_info_tlv;
	query_message.list_of_TLVs[1] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &query_message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	PLATFORM_FREE(query_message.list_of_TLVs);

	return ret;
}

INT8U sendClientCapabilityReportPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U *client_bssid, INT8U *client_mac_address)
{
	// The "Client capability report" message is a CMDU with a Client Info TLV and a Client Capability Report TLV

	INT8U                            ret, tlv_count = 0, error_tlv_count = 0;
	char *                           bssid_interface;
	struct CMDU                      report_message;
	struct ClientInfoTLV             client_info_tlv;
	struct ClientCapabilityReportTLV client_capability_tlv;
	struct ErrorCodeTLV              error_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_CLIENT_CAPABILITY_REPORT (%s) MID: (%04x)\n", interface_name, mid);

	//Fill the client info TLV
	client_info_tlv.tlv_type = TLV_TYPE_CLIENT_INFO;
	PLATFORM_MEMCPY(client_info_tlv.bssid, client_bssid, 6);
	PLATFORM_MEMCPY(client_info_tlv.mac_address, client_mac_address, 6);

	//Check if client is associated with any bssid operated by Agent
	//bssid = DMclientToBssid(client_mac_address);
	//interface_name = DMmacToInterfaceName(bssid);
	bssid_interface = DMmacToInterfaceName(client_bssid);

	//	if(bssid){
	if (bssid_interface) {
		//Fill the client capability report tlv
		if (_obtainClientCapabilityTLV(&client_capability_tlv, bssid_interface, client_mac_address) == 0) {
			PLATFORM_PRINTF_DEBUG("Error to get client capability\n");
			//If client association request body cannot be obtained
			error_tlv.tlv_type    = TLV_TYPE_ERROR_CODE;
			error_tlv.reason_code = 0x03;
			PLATFORM_MEMCPY(error_tlv.sta_mac_address, client_mac_address, MACADDRLEN);
			error_tlv_count++;
		}
	} else {
		//Client not associated to any operated BSS, create ErrorTLV and fill it with 0x02
		client_capability_tlv.tlv_type    = TLV_TYPE_CLIENT_CAPABLITY_REPORT;
		client_capability_tlv.result_code = 0x01;
		client_capability_tlv.frame_body  = NULL;

		error_tlv.tlv_type    = TLV_TYPE_ERROR_CODE;
		error_tlv.reason_code = 0x02;
		PLATFORM_MEMCPY(error_tlv.sta_mac_address, client_mac_address, MACADDRLEN);
		error_tlv_count++;
	}

	tlv_count = 2 + error_tlv_count + 1;
	// Build the CMDU
	//
	report_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	report_message.message_type    = CMDU_TYPE_CLIENT_CAPABILITY_REPORT;
	report_message.message_id      = mid;
	report_message.relay_indicator = 0;
	report_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * tlv_count);

	report_message.list_of_TLVs[0] = (INT8U *)&client_info_tlv;
	report_message.list_of_TLVs[1] = (INT8U *)&client_capability_tlv;

	if (error_tlv_count > 0) {
		report_message.list_of_TLVs[tlv_count - 2] = (INT8U *)&error_tlv;
	}

	report_message.list_of_TLVs[tlv_count - 1] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &report_message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	_freeClientCapabilityTLV(&client_capability_tlv);
	PLATFORM_FREE(report_message.list_of_TLVs);

	return ret;
}

INT8U sendAPMetricQueryPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U bssid_nr, INT8U (*bssid)[6])
{

	INT8U ret, i;

	struct CMDU             report_message;
	struct APMetricQueryTLV ap_metric_query_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_AP_METRICS_QUERY (%s) MID: (%04x)\n", interface_name, mid);

	//Fill the AP Metrics Query TLV
	ap_metric_query_tlv.tlv_type = TLV_TYPE_AP_METRIC_QUERY;
	ap_metric_query_tlv.bssid_nr = bssid_nr;

	ap_metric_query_tlv.bssid = PLATFORM_MALLOC(sizeof(*(ap_metric_query_tlv.bssid)) * bssid_nr);

	for (i = 0; i < bssid_nr; i++) {
		PLATFORM_MEMCPY((ap_metric_query_tlv.bssid[i]), (bssid[i]), 6);
		PLATFORM_PRINTF_DEBUG("%s (%d) BSSID: %02x%02x%02x%02x%02x%02x\n", __FUNCTION__, __LINE__,
		                      (*ap_metric_query_tlv.bssid)[0], (*ap_metric_query_tlv.bssid)[1], (*ap_metric_query_tlv.bssid)[2],
		                      (*ap_metric_query_tlv.bssid)[3], (*ap_metric_query_tlv.bssid)[4], (*ap_metric_query_tlv.bssid)[5]);
	}
	// Build the CMDU
	//
	report_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	report_message.message_type    = CMDU_TYPE_AP_METRICS_QUERY;
	report_message.message_id      = mid;
	report_message.relay_indicator = 0;
	report_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);

	report_message.list_of_TLVs[0] = (INT8U *)&ap_metric_query_tlv;
	report_message.list_of_TLVs[1] = NULL;

	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &report_message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	PLATFORM_FREE(ap_metric_query_tlv.bssid);

	PLATFORM_FREE(report_message.list_of_TLVs);

	return ret;
}

INT8U sendAPMetricResponsePacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U bssid_nr, INT8U (*bssid)[6], INT8U radio_nr, INT8U (*radio_mac)[6])
{
	INT8U ret, i, tlv_count = 0, tlv_index = 0;
	INT8U apMetric_tlv_count = 0, staLink_tlv_count = 0, staTraffic_tlv_count = 0;
	INT8U dummy_tlv = 0;

	INT8U is_r2_format = 0;

#ifdef RTK_MULTI_AP_R2
	INT8U apExtendedMetric_tlv_count = 0, staExtendedLink_tlv_count = 0, radioMetrics_tlv_count = 0;
#endif

#ifdef RTK_MULTI_AP_R3
	INT8U is_r3_format = 0;
	INT8U wifi6StaStatusReport_tlv_count = 0;
#endif

	struct CMDU                          report_message;
	struct APMetricsTLV *                ap_metric_tlv               = NULL;
	struct AssociatedSTALinkMetricsTLV * assoc_sta_link_metric_tlv   = NULL;
	struct AssociatedSTATrafficStatsTLV *assoc_sta_traffic_stats_tlv = NULL;

#ifdef RTK_MULTI_AP_R2
	struct APExtendedMetricsTLV *              ap_extended_metric_tlv             = NULL;
	struct AssociatedSTAExtendedLinkMericsTLV *assoc_sta_extended_link_metric_tlv = NULL;
	struct RadioMetricsTLV *                   radio_metrics_tlv                  = NULL;
#endif

#ifdef RTK_MULTI_AP_R3
	struct AssociatedWiFi6STAStatusReportTLV *assoc_wifi6_sta_status_report_tlv = NULL;
#endif

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_AP_METRICS_RESPONSE (%s) MID: (%04x)\n", interface_name, mid);

	if (interface_name == NULL) {
		PLATFORM_PRINTF_WARNING("%s - Interface name is NULL!\n", __FUNCTION__);
		return 0;
	}
	if (destination_al_mac_address == NULL) {
		PLATFORM_PRINTF_WARNING("%s - Destination mac is NULL!\n", __FUNCTION__);
		return 0;
	}

	if (DMgetWFATestMode() && bssid_nr < DMgetMaxBssPerRadio()) {
		ap_metric_tlv = PLATFORM_MALLOC(sizeof(struct APMetricsTLV) * DMgetMaxBssPerRadio());
	} else {
		if (bssid_nr)
			ap_metric_tlv = PLATFORM_MALLOC(sizeof(struct APMetricsTLV) * bssid_nr);
	}

#ifdef RTK_MULTI_AP_R2
	if (MULTI_AP_PROFILE_2 <= DMgetMultiAPProfile() && MULTI_AP_PROFILE_2 <= DMgetTargetDeviceMultiAPProfile(destination_al_mac_address)) {
		is_r2_format = 1;
	}

	if (is_r2_format) {
		ap_extended_metric_tlv = PLATFORM_MALLOC(sizeof(struct APExtendedMetricsTLV) * bssid_nr);
	}
#endif

#ifdef RTK_MULTI_AP_R3
	if (MULTI_AP_PROFILE_3 <= DMgetMultiAPProfile() && MULTI_AP_PROFILE_3 <= DMgetTargetDeviceMultiAPProfile(destination_al_mac_address)) {
		is_r3_format = 1;
	}
#endif

	//Fill the AP Metrics TLV
	for (i = 0; i < bssid_nr; i++) {
		INT8U                  k;
		struct LocalInterface *local_interface;

		_obtainAPMetricsTLV(&ap_metric_tlv[i], (INT8U *)bssid[i], dummy_tlv);
		apMetric_tlv_count++;

#ifdef RTK_MULTI_AP_R2
		if (is_r2_format) {
			_obtainAPExtendedMetricsTLV(&ap_extended_metric_tlv[i], (INT8U *)bssid[i]);
			apExtendedMetric_tlv_count++;
		}
#endif

		local_interface = DMnameToLocalInterfaceStruct(DMmacToInterfaceName((INT8U *)bssid[i]));

		if (local_interface) {
			struct interfaceInfo *x;
			x = PLATFORM_GET_1905_INTERFACE_INFO(local_interface->name);

			if (NULL == x) {
				PLATFORM_PRINTF_WARNING("Could not retrieve info of interface %s\n", local_interface->name);
				continue;
			}

			//Fill the Assoc STA Link Metrics TLV
			if (local_interface->metric_inclusion_policy & MASK_ASSOCIATED_STA_LINK_METRICS_INCLUSION_POLICY) {
				if (x->neighbor_mac_addresses_nr) {
					assoc_sta_link_metric_tlv = PLATFORM_REALLOC(assoc_sta_link_metric_tlv, sizeof(struct AssociatedSTALinkMetricsTLV) * (staLink_tlv_count + x->neighbor_mac_addresses_nr));
#ifdef RTK_MULTI_AP_R2
					if (is_r2_format) {
						assoc_sta_extended_link_metric_tlv = PLATFORM_REALLOC(assoc_sta_extended_link_metric_tlv, sizeof(struct AssociatedSTAExtendedLinkMericsTLV) * (staExtendedLink_tlv_count + x->neighbor_mac_addresses_nr));
					}
#endif
					for (k = 0; k < x->neighbor_mac_addresses_nr; k++) {
						_obtainAssociatedStaLinkMetricsTLV(&assoc_sta_link_metric_tlv[staLink_tlv_count], x->name, x->neighbor_list[k].mac_address);
						staLink_tlv_count++;

#ifdef RTK_MULTI_AP_R2
						if (is_r2_format) {
							_obtainAssociatedStaExtendedLinkMetricsTLV(&assoc_sta_extended_link_metric_tlv[staExtendedLink_tlv_count], x->name, x->neighbor_list[k].mac_address);
							staExtendedLink_tlv_count++;
						}
#endif
					}
				}
			}

			//Fill the Assoc STA Traffic Stats TLV
			if (local_interface->metric_inclusion_policy & MASK_ASSOCIATED_STA_TRAFFIC_STATS_INCLUSION_POLICY) {
				if (x->neighbor_mac_addresses_nr) {
					assoc_sta_traffic_stats_tlv = PLATFORM_REALLOC(assoc_sta_traffic_stats_tlv, sizeof(struct AssociatedSTATrafficStatsTLV) * (staTraffic_tlv_count + x->neighbor_mac_addresses_nr));
					for (k = 0; k < x->neighbor_mac_addresses_nr; k++) {
						_obtainAssociatedStaTrafficStatsTLV(&assoc_sta_traffic_stats_tlv[staTraffic_tlv_count], x->name, x->neighbor_list[k].mac_address);
						staTraffic_tlv_count++;
					}
				}
			}

#ifdef RTK_MULTI_AP_R3
			// Fill the WiFi 6 Assoc STA Traffic Status Report TLV
			if(is_r3_format){
				if(local_interface->metric_inclusion_policy & MASK_ASSOCIATED_WIFI6_STA_STATUS_REPORT_INCLUSION_POLICY){
					if(x->neighbor_mac_addresses_nr){
						assoc_wifi6_sta_status_report_tlv = PLATFORM_REALLOC(assoc_wifi6_sta_status_report_tlv, sizeof(struct AssociatedWiFi6STAStatusReportTLV) * (wifi6StaStatusReport_tlv_count + x->neighbor_mac_addresses_nr));
						for (k = 0; k < x->neighbor_mac_addresses_nr; k++) {
							_obtainAssociatedWiFi6StaStatusReportTLV(&assoc_wifi6_sta_status_report_tlv[wifi6StaStatusReport_tlv_count], x->name, x->neighbor_list[k].mac_address);
							wifi6StaStatusReport_tlv_count++;
						}
					}
				}
			}
#endif
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
		}
	}

	if (DMgetWFATestMode()) {
		dummy_tlv = 1;
		for (i = bssid_nr; i < DMgetMaxBssPerRadio(); i++) {
			_obtainAPMetricsTLV(&ap_metric_tlv[i], (INT8U *)bssid[i], dummy_tlv);
			apMetric_tlv_count++;
		}
	}

	tlv_count = apMetric_tlv_count + staLink_tlv_count + staTraffic_tlv_count + 1;

	// Pass unused paramater checking
	if (0 && is_r2_format) {
		for (i = 0; i < radio_nr; i++) {
			PLATFORM_PRINTF_DETAIL("Radio %d: %02x:%02x:%02x:%02x:%02x:%02x\n", i, radio_mac[i][0], radio_mac[i][1], radio_mac[i][2], radio_mac[i][3], radio_mac[i][4], radio_mac[i][5]);
		}
	}

#ifdef RTK_MULTI_AP_R2
	if (is_r2_format) {
		radioMetrics_tlv_count = radio_nr;
		radio_metrics_tlv      = PLATFORM_MALLOC(sizeof(struct RadioMetricsTLV) * radio_nr);
		for (i = 0; i < radio_nr; i++) {
			_obtainRadioMetricsTLV(&radio_metrics_tlv[i], (INT8U *)radio_mac[i]);
		}

		tlv_count += (apExtendedMetric_tlv_count + radioMetrics_tlv_count + staExtendedLink_tlv_count);
	}
#endif

#ifdef RTK_MULTI_AP_R3
	if(is_r3_format){
		tlv_count += wifi6StaStatusReport_tlv_count;
	}
#endif

	// Build the CMDU
	//
	report_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	report_message.message_type    = CMDU_TYPE_AP_METRICS_RESPONSE;
	report_message.message_id      = mid;
	report_message.relay_indicator = 0;
	report_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * tlv_count);

	for (i = 0; i < apMetric_tlv_count; i++) {
		report_message.list_of_TLVs[tlv_index] = (INT8U *)&ap_metric_tlv[i];
		tlv_index++;
	}

#ifdef RTK_MULTI_AP_R2
	for (i = 0; i < apExtendedMetric_tlv_count; i++) {
		report_message.list_of_TLVs[tlv_index] = (INT8U *)&ap_extended_metric_tlv[i];
		tlv_index++;
	}

	for (i = 0; i < radioMetrics_tlv_count; i++) {
		report_message.list_of_TLVs[tlv_index] = (INT8U *)&radio_metrics_tlv[i];
		tlv_index++;
	}
#endif

	for (i = 0; i < staTraffic_tlv_count; i++) {
		report_message.list_of_TLVs[tlv_index] = (INT8U *)&assoc_sta_traffic_stats_tlv[i];
		tlv_index++;
	}

	for (i = 0; i < staLink_tlv_count; i++) {
		report_message.list_of_TLVs[tlv_index] = (INT8U *)&assoc_sta_link_metric_tlv[i];
		tlv_index++;
	}

#ifdef RTK_MULTI_AP_R2
	for (i = 0; i < staExtendedLink_tlv_count; i++) {
		report_message.list_of_TLVs[tlv_index] = (INT8U *)&assoc_sta_extended_link_metric_tlv[i];
		tlv_index++;
	}
#endif

#ifdef RTK_MULTI_AP_R3
	if(is_r3_format){
		for (i = 0; i < wifi6StaStatusReport_tlv_count; i++) {
			report_message.list_of_TLVs[tlv_index] = (INT8U *)&assoc_wifi6_sta_status_report_tlv[i];
			tlv_index++;
		}
	}
#endif

	report_message.list_of_TLVs[tlv_count - 1] = NULL;

	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &report_message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	if (ap_metric_tlv)
		PLATFORM_FREE(ap_metric_tlv);

	if (staLink_tlv_count > 0) {
		for (i = 0; i < staLink_tlv_count; i++) {
			if (assoc_sta_link_metric_tlv[i].bssid_nr > 0 && assoc_sta_link_metric_tlv[i].assoc_sta_link_metrics)
				PLATFORM_FREE(assoc_sta_link_metric_tlv[i].assoc_sta_link_metrics);
		}
		PLATFORM_FREE(assoc_sta_link_metric_tlv);
	}

	if (staTraffic_tlv_count > 0) {
		PLATFORM_FREE(assoc_sta_traffic_stats_tlv);
	}

#ifdef RTK_MULTI_AP_R2
	if (ap_extended_metric_tlv) {
		PLATFORM_FREE(ap_extended_metric_tlv);
	}

	if (radio_metrics_tlv) {
		PLATFORM_FREE(radio_metrics_tlv);
	}

	if (staExtendedLink_tlv_count > 0) {
		for (i = 0; i < staExtendedLink_tlv_count; i++) {
			if (assoc_sta_extended_link_metric_tlv[i].bssid_nr > 0 && assoc_sta_extended_link_metric_tlv[i].reported_bss) {
				PLATFORM_FREE(assoc_sta_extended_link_metric_tlv[i].reported_bss);
			}
		}
		PLATFORM_FREE(assoc_sta_extended_link_metric_tlv);
	}
#endif

#ifdef RTK_MULTI_AP_R3
	if(is_r3_format){
		if (wifi6StaStatusReport_tlv_count > 0) {
			for (i = 0; i < wifi6StaStatusReport_tlv_count; i++) {
				if (assoc_wifi6_sta_status_report_tlv[i].queue_nr > 0 && assoc_wifi6_sta_status_report_tlv[i].queues) {
					PLATFORM_FREE(assoc_wifi6_sta_status_report_tlv[i].queues);
				}
			}
			PLATFORM_FREE(assoc_wifi6_sta_status_report_tlv);
		}
	}
#endif

	PLATFORM_FREE(report_message.list_of_TLVs);

	return ret;
}

INT8U sendAssocSTALinkMetricQueryPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U *macaddr)
{
	INT8U ret;

	struct CMDU                 report_message;
	struct STAMacAddressTypeTLV sta_mac_address_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_ASSOCIATED_STA_LINK_METRIC_QUERY (%s) MID: (%04x)\n", interface_name, mid);

	//Fill the STA Mac Address Type TLV
	sta_mac_address_tlv.tlv_type = TLV_TYPE_STA_MAC_ADDRESS_TYPE;
	PLATFORM_MEMCPY(sta_mac_address_tlv.mac_address, macaddr, 6);

	// Build the CMDU
	//
	report_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	report_message.message_type    = CMDU_TYPE_ASSOCIATED_STA_LINK_METRICS_QUERY;
	report_message.message_id      = mid;
	report_message.relay_indicator = 0;
	report_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);

	report_message.list_of_TLVs[0] = (INT8U *)&sta_mac_address_tlv;
	report_message.list_of_TLVs[1] = NULL;

	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &report_message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	PLATFORM_FREE(report_message.list_of_TLVs);

	return ret;
}

INT8U sendAssocSTALinkMetricResponsePacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U *macaddr)
{

	INT8U                              ret, tlv_count = 0, error_tlv_count = 0, extended_tlv_count = 0;
	INT8U                              is_r2_format = 0;
	struct CMDU                        report_message;
	struct AssociatedSTALinkMetricsTLV assoc_sta_link_metric_tlv;
#ifdef RTK_MULTI_AP_R2
	struct AssociatedSTAExtendedLinkMericsTLV assoc_sta_extended_link_metric_tlv = { 0 };
#endif
	struct ErrorCodeTLV error_tlv;
	INT8U               assoc_radio_mac[6] = { 0 };
	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_ASSOCIATED_STA_LINK_METRIC_RESPONSE (%s) MID: (%04x)\n", interface_name, mid);

	if (interface_name == NULL) {
		PLATFORM_PRINTF_WARNING("%s - Interface name is NULL!\n", __FUNCTION__);
		return 0;
	}
	if (destination_al_mac_address == NULL) {
		PLATFORM_PRINTF_WARNING("%s - Destination mac is NULL!\n", __FUNCTION__);
		return 0;
	}

#ifdef RTK_MULTI_AP_R2
	if (MULTI_AP_PROFILE_2 <= DMgetMultiAPProfile()
		&& MULTI_AP_PROFILE_2 <= DMgetTargetDeviceMultiAPProfile(destination_al_mac_address))
		is_r2_format = 1;
#endif

	//TODO: match client mac with local interface bssid, get interface_name
	//If STA is not associated with any of operated bss, include ErrorTLV, set result code 0x02
	DMclientMacToAssociatedRadioMac(macaddr, (INT8U *)&assoc_radio_mac);

	//Fill the STA Mac Address Type TLV
	if (_obtainAssociatedStaLinkMetricsTLV(&assoc_sta_link_metric_tlv, DMmacToInterfaceName(assoc_radio_mac), macaddr) == 0) {
		error_tlv.tlv_type = TLV_TYPE_ERROR_CODE;
		PLATFORM_MEMCPY(error_tlv.sta_mac_address, macaddr, MACADDRLEN);
		error_tlv.reason_code = 0x02;
		error_tlv_count++;
	}
#ifdef RTK_MULTI_AP_R2
	else if (is_r2_format && _obtainAssociatedStaExtendedLinkMetricsTLV(&assoc_sta_extended_link_metric_tlv, DMmacToInterfaceName(assoc_radio_mac), macaddr)) {
		extended_tlv_count++;
	}
#endif

	tlv_count = 1 + error_tlv_count + 1 + extended_tlv_count;

	// Build the CMDU
	//
	report_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	report_message.message_type    = CMDU_TYPE_ASSOCIATED_STA_LINK_METRICS_RESPONSE;
	report_message.message_id      = mid;
	report_message.relay_indicator = 0;
	report_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * tlv_count);

	report_message.list_of_TLVs[0] = (INT8U *)&assoc_sta_link_metric_tlv;
	if (error_tlv_count > 0) {
		report_message.list_of_TLVs[tlv_count - 2] = (INT8U *)&error_tlv;
	}
#ifdef RTK_MULTI_AP_R2
	else if (extended_tlv_count) {
		report_message.list_of_TLVs[tlv_count - 2] = (INT8U *)&assoc_sta_extended_link_metric_tlv;
	}
#endif

	report_message.list_of_TLVs[tlv_count - 1] = NULL;

	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &report_message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	if (assoc_sta_link_metric_tlv.bssid_nr > 0 && assoc_sta_link_metric_tlv.assoc_sta_link_metrics) {
		PLATFORM_FREE(assoc_sta_link_metric_tlv.assoc_sta_link_metrics);
	}

#ifdef RTK_MULTI_AP_R2
	if (assoc_sta_extended_link_metric_tlv.bssid_nr > 0) {
		PLATFORM_FREE(assoc_sta_extended_link_metric_tlv.reported_bss);
	}
#endif

	PLATFORM_FREE(report_message.list_of_TLVs);

	return ret;
}

INT8U sendUnassocSTALinkMetricQueryPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U op_class, INT8U channel_list_nr, struct ChannelListInfo *channel_list_entries)
{
	INT8U                                     ret;
	INT8U                                     i;
	struct CMDU                               report_message;
	struct UnassociatedSTALinkMetricsQueryTLV unassoc_sta_link_metric_query_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_UNASSOCIATED_STA_LINK_METRIC_QUERY (%s) MID: (%04x)\n", interface_name, mid);

	unassoc_sta_link_metric_query_tlv.tlv_type        = TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY;
	unassoc_sta_link_metric_query_tlv.op_class        = op_class;
	unassoc_sta_link_metric_query_tlv.channel_list_nr = channel_list_nr;

	PLATFORM_PRINTF_INFO("channel_list_nr = %d\n", channel_list_nr);
	if (channel_list_nr > 0) {
		//Fill the TLV
		_obtainUnassociatedStaLinkMetricsQueryTLV(&unassoc_sta_link_metric_query_tlv, channel_list_nr, channel_list_entries);
	}

	// Build the CMDU
	//
	report_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	report_message.message_type    = CMDU_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY;
	report_message.message_id      = mid;
	report_message.relay_indicator = 0;
	report_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);

	report_message.list_of_TLVs[0] = (INT8U *)&unassoc_sta_link_metric_query_tlv;
	report_message.list_of_TLVs[1] = NULL;

	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &report_message, true, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	//free unassociated sta link metric query
	if (unassoc_sta_link_metric_query_tlv.channel_list_nr > 0) {
		for (i = 0; i < unassoc_sta_link_metric_query_tlv.channel_list_nr; i++) {
			if (unassoc_sta_link_metric_query_tlv.channel_list_entry[i].sta_nr > 0) {
				PLATFORM_SAFE_FREE((void **)&(unassoc_sta_link_metric_query_tlv.channel_list_entry[i].sta_mac_address));
			}
		}
		PLATFORM_SAFE_FREE((void **)&unassoc_sta_link_metric_query_tlv.channel_list_entry);
	}

	PLATFORM_SAFE_FREE((void **)&report_message.list_of_TLVs);

	return ret;
}

INT8U sendUnassocSTALinkMetricResponsePacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U op_class, INT8U sta_nr, INT8U *buf)
{
	INT8U ret;

	struct CMDU                                  report_message;
	struct UnassociatedSTALinkMetricsResponseTLV unassoc_sta_link_metric_response_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_UNASSOCIATED_STA_LINK_METRIC_RESPONSE (%s) MID: (%04x)\n", interface_name, mid);

	if (interface_name == NULL) {
		PLATFORM_PRINTF_WARNING("%s - Interface name is NULL!\n", __FUNCTION__);
		return 0;
	}
	if (destination_al_mac_address == NULL) {
		PLATFORM_PRINTF_WARNING("%s - Destination mac is NULL!\n", __FUNCTION__);
		return 0;
	}

	//Fill the TLV
	_obtainUnassociatedStaLinkMetricsResponseTLV(&unassoc_sta_link_metric_response_tlv, op_class, sta_nr, buf);

	// Build the CMDU
	//
	report_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	report_message.message_type    = CMDU_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE;
	report_message.message_id      = mid;
	report_message.relay_indicator = 0;
	report_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);

	report_message.list_of_TLVs[0] = (INT8U *)&unassoc_sta_link_metric_response_tlv;
	report_message.list_of_TLVs[1] = NULL;

	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &report_message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	//free unassociated sta metric response
	if (unassoc_sta_link_metric_response_tlv.sta_nr > 0 && unassoc_sta_link_metric_response_tlv.unassoc_sta_link_metrics) {
		PLATFORM_FREE(unassoc_sta_link_metric_response_tlv.unassoc_sta_link_metrics);
	}

	PLATFORM_FREE(report_message.list_of_TLVs);

	return ret;
}

INT8U sendBeaconMetricsQueryPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, struct BeaconMetricsQueryTLV *query)
{
	INT8U                        ret;
	struct CMDU                  report_message;
	struct BeaconMetricsQueryTLV beacon_metrics_query_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_BEACON_METRICS_QUERY (%s) MID: (%04x)\n", interface_name, mid);

	//Fill the TLV
	_obtainBeaconMetricsQueryTLV(&beacon_metrics_query_tlv, query);

	// Build the CMDU
	//
	report_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	report_message.message_type    = CMDU_TYPE_BEACON_METRICS_QUERY;
	report_message.message_id      = mid;
	report_message.relay_indicator = 0;
	report_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);

	report_message.list_of_TLVs[0] = (INT8U *)&beacon_metrics_query_tlv;
	report_message.list_of_TLVs[1] = NULL;

	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &report_message, true, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	//free the beacon_metrics_query_tlv
	freeBeaconMetricsQueryTLV(&beacon_metrics_query_tlv);
	PLATFORM_FREE(report_message.list_of_TLVs);

	return ret;
}

INT8U sendBeaconMetricsResponsePacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U *buf, INT16U buf_len)
{
	INT8U                            ret;
	struct CMDU                      report_message;
	struct BeaconMetricsResponseTLV *beacon_metrics_response_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_BEACON_METRICS_RESPONSE (%s) MID: (%04x)\n", interface_name, mid);

	if (interface_name == NULL) {
		PLATFORM_PRINTF_WARNING("%s - Interface name is NULL!\n", __FUNCTION__);
		return 0;
	}
	if (destination_al_mac_address == NULL) {
		PLATFORM_PRINTF_WARNING("%s - Destination mac is NULL!\n", __FUNCTION__);
		return 0;
	}
	//Fill the TLV
	beacon_metrics_response_tlv = _obtainBeaconMetricsResponseTLV(buf, buf_len);
	// Build the CMDU
	//
	report_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	report_message.message_type    = CMDU_TYPE_BEACON_METRICS_RESPONSE;
	report_message.message_id      = mid;
	report_message.relay_indicator = 0;
	report_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);

	report_message.list_of_TLVs[0] = (INT8U *)beacon_metrics_response_tlv;
	report_message.list_of_TLVs[1] = NULL;
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &report_message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}
	//free beacon_metrics_response
	_freeBeaconMetricsResponseTLV(beacon_metrics_response_tlv);
	PLATFORM_FREE(report_message.list_of_TLVs);
	return ret;
}

INT8U sendCombinedInfrastructureMetricMessage(char *interface_name, INT16U mid, INT8U *destination_al_mac_address)
{

	INT8U                 ret, tlv_index, tlv_count = 0;
	INT8U                 ap_metrics_tlv_count = 0, tx_metric_tlv_count = 0, rx_metric_tlv_count = 0;
	INT8U                 i, j;
	struct networkDevice *network_devices;
	INT8U                 network_devices_nr;
	DMallNetworkDevicesGet(&network_devices, &network_devices_nr);

	struct CMDU report_message;
	INT8U **    ap_metrics_tlv          = NULL;
	INT8U **    transmitter_metrics_tlv = NULL;
	INT8U **    receiver_metric_tlv     = NULL;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_COMBINED_INFRASTRUCTURE_METRICS (%s) MID: (%04x)\n", interface_name, mid);

	//Collect lists of TLVs to be sent
	for (i = 1; i < network_devices_nr; i++) {

		if (network_devices[i].ap_metrics_nr > 0) {
			if (ap_metrics_tlv_count < 1) {
				ap_metrics_tlv = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * (ap_metrics_tlv_count + 1));
			} else {
				ap_metrics_tlv = (INT8U **)PLATFORM_REALLOC(ap_metrics_tlv, sizeof(INT8U *) * (ap_metrics_tlv_count + 1));
			}
			ap_metrics_tlv[ap_metrics_tlv_count] = (INT8U *)network_devices[i].ap_metrics;
			ap_metrics_tlv_count++;
		}

		for (j = 0; j < network_devices[i].metrics_with_neighbors_nr; j++) {

			if (network_devices[i].metrics_with_neighbors[j].tx_metrics) {
				transmitter_metrics_tlv                      = (INT8U **)PLATFORM_REALLOC(transmitter_metrics_tlv, sizeof(INT8U *) * (tx_metric_tlv_count + 1));
				transmitter_metrics_tlv[tx_metric_tlv_count] = (INT8U *)network_devices[i].metrics_with_neighbors[j].tx_metrics;
				tx_metric_tlv_count++;
			}

			if (network_devices[i].metrics_with_neighbors[j].rx_metrics) {
				receiver_metric_tlv                      = (INT8U **)PLATFORM_REALLOC(receiver_metric_tlv, sizeof(INT8U *) * (rx_metric_tlv_count + 1));
				receiver_metric_tlv[rx_metric_tlv_count] = (INT8U *)network_devices[i].metrics_with_neighbors[j].rx_metrics;
				rx_metric_tlv_count++;
			}
		}
	}

	tlv_count = ap_metrics_tlv_count + tx_metric_tlv_count + rx_metric_tlv_count + 1;

	// Build the CMDU
	//
	report_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	report_message.message_type    = CMDU_TYPE_COMBINED_INFRASTRUCTURE_METRICS;
	report_message.message_id      = mid;
	report_message.relay_indicator = 0;
	report_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * tlv_count);

	tlv_index = 0;

	for (i = 0; i < ap_metrics_tlv_count; i++) {
		report_message.list_of_TLVs[tlv_index] = (INT8U *)ap_metrics_tlv[i];
		tlv_index++;
	}

	for (i = 0; i < tx_metric_tlv_count; i++) {
		report_message.list_of_TLVs[tlv_index] = (INT8U *)transmitter_metrics_tlv[i];
		tlv_index++;
	}

	for (i = 0; i < rx_metric_tlv_count; i++) {
		report_message.list_of_TLVs[tlv_index] = (INT8U *)receiver_metric_tlv[i];
		tlv_index++;
	}

	report_message.list_of_TLVs[tlv_count - 1] = NULL;

	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &report_message, true, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	if (ap_metrics_tlv_count > 0 && ap_metrics_tlv)
		PLATFORM_FREE(ap_metrics_tlv);

	if (tx_metric_tlv_count > 0 && transmitter_metrics_tlv)
		PLATFORM_FREE(transmitter_metrics_tlv);

	if (rx_metric_tlv_count > 0 && receiver_metric_tlv)
		PLATFORM_FREE(receiver_metric_tlv);

	PLATFORM_FREE(report_message.list_of_TLVs);

	return ret;
}
//#error keith warning check
INT8U sendBackhaulSteeringRequestPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U *backhaul_sta_mac, INT8U *target_bss_mac, INT8U op_class, INT8U channel_number)
{
	INT8U                             ret;
	struct CMDU                       report_message;
	struct BackhaulSteeringRequestTLV backhaul_steering_request_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_BACKHAUL_STEERING_REQUEST (%s) MID: (%04x)\n", interface_name, mid);

	//Fill the TLV
	backhaul_steering_request_tlv.tlv_type = TLV_TYPE_BACKHAUL_STEERING_REQUEST;
	PLATFORM_MEMCPY(backhaul_steering_request_tlv.backhaul_mac, backhaul_sta_mac, MACADDRLEN);
	PLATFORM_MEMCPY(backhaul_steering_request_tlv.target_bssid, target_bss_mac, MACADDRLEN);
	backhaul_steering_request_tlv.op_class       = op_class;
	backhaul_steering_request_tlv.channel_number = channel_number;

	// Build the CMDU
	//
	report_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	report_message.message_type    = CMDU_TYPE_BACKHAUL_STEERING_REQUEST;
	report_message.message_id      = mid;
	report_message.relay_indicator = 0;
	report_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);

	report_message.list_of_TLVs[0] = (INT8U *)&backhaul_steering_request_tlv;
	report_message.list_of_TLVs[1] = NULL;

	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &report_message, true, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	PLATFORM_FREE(report_message.list_of_TLVs);

	return ret;
}

INT8U sendBackhaulSteeringResponsePacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U *backhaul_sta_mac, INT8U *target_bss_mac, INT8U result_code)
{
	INT8U ret;
	//INT8U *destination_al_mac_address;

	struct CMDU                        report_message;
	struct BackhaulSteeringResponseTLV backhaul_steering_response_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_BACKHAUL_STEERING_RESPONSE (%s) MID: (%04x)\n", interface_name, mid);

	if (interface_name == NULL) {
		PLATFORM_PRINTF_WARNING("%s - Interface name is NULL!\n", __FUNCTION__);
		return 0;
	}
	if (destination_al_mac_address == NULL) {
		PLATFORM_PRINTF_WARNING("%s - Destination mac is NULL!\n", __FUNCTION__);
		return 0;
	}

	//Fill the TLV
	backhaul_steering_response_tlv.tlv_type = TLV_TYPE_BACKHAUL_STEERING_RESPONSE;
	PLATFORM_MEMCPY(backhaul_steering_response_tlv.backhaul_mac, backhaul_sta_mac, MACADDRLEN);
	PLATFORM_MEMCPY(backhaul_steering_response_tlv.target_bssid, target_bss_mac, MACADDRLEN);
	backhaul_steering_response_tlv.result_code = result_code;

	// Build the CMDU
	//
	report_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	report_message.message_type    = CMDU_TYPE_BACKHAUL_STEERING_RESPONSE;
	report_message.message_id      = mid;
	report_message.relay_indicator = 0;
	report_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);

	report_message.list_of_TLVs[0] = (INT8U *)&backhaul_steering_response_tlv;
	report_message.list_of_TLVs[1] = NULL;

	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &report_message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	PLATFORM_FREE(report_message.list_of_TLVs);

	return ret;
}

INT8U send1905InterfaceListResponseALME(INT8U alme_client_id)
{
	INT8U ret;

	struct getIntfListResponseALME *out;

	char **ifs_names;
	INT8U  ifs_nr;

	PLATFORM_PRINTF_INFO("--> ALME_TYPE_GET_INTF_LIST_RESPONSE\n");

	// Fill the requested ALME response
	//
	out            = (struct getIntfListResponseALME *)PLATFORM_MALLOC(sizeof(struct getIntfListResponseALME));
	out->alme_type = ALME_TYPE_GET_INTF_LIST_RESPONSE;

	ifs_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&ifs_nr);
	if (0 == ifs_nr) {
		out->interface_descriptors_nr = 0;
		out->interface_descriptors    = NULL;
	} else {
		INT8U i;

		out->interface_descriptors_nr = ifs_nr;
		out->interface_descriptors    = (struct _intfDescriptorEntries *)PLATFORM_MALLOC(sizeof(struct _intfDescriptorEntries) * ifs_nr);

		for (i = 0; i < out->interface_descriptors_nr; i++) {
			struct interfaceInfo *x;

			x = PLATFORM_GET_1905_INTERFACE_INFO(ifs_names[i]);
			if (NULL == x) {
				PLATFORM_PRINTF_WARNING("Could not retrieve info of interface %s\n", ifs_names[i]);

				out->interface_descriptors[i].interface_address[0]    = 0x00;
				out->interface_descriptors[i].interface_address[1]    = 0x00;
				out->interface_descriptors[i].interface_address[2]    = 0x00;
				out->interface_descriptors[i].interface_address[3]    = 0x00;
				out->interface_descriptors[i].interface_address[4]    = 0x00;
				out->interface_descriptors[i].interface_address[5]    = 0x00;
				out->interface_descriptors[i].interface_type          = MEDIA_TYPE_UNKNOWN;
				out->interface_descriptors[i].bridge_flag             = 0;
				out->interface_descriptors[i].vendor_specific_info_nr = 0;
				out->interface_descriptors[i].vendor_specific_info    = NULL;
			} else {
				out->interface_descriptors[i].interface_address[0] = x->mac_address[0];
				out->interface_descriptors[i].interface_address[1] = x->mac_address[1];
				out->interface_descriptors[i].interface_address[2] = x->mac_address[2];
				out->interface_descriptors[i].interface_address[3] = x->mac_address[3];
				out->interface_descriptors[i].interface_address[4] = x->mac_address[4];
				out->interface_descriptors[i].interface_address[5] = x->mac_address[5];
				out->interface_descriptors[i].interface_type       = x->interface_type;

				out->interface_descriptors[i].bridge_flag = DMisInterfaceBridged(ifs_names[i]);

				out->interface_descriptors[i].vendor_specific_info_nr = 0;
				out->interface_descriptors[i].vendor_specific_info    = NULL;
			}

			if (NULL != x) {
				PLATFORM_FREE_1905_INTERFACE_INFO(x);
			}
		}
	}

	PLATFORM_FREE_LIST_OF_1905_INTERFACES(ifs_names, ifs_nr);

	// Send the packet
	//
	if (0 == send1905RawALME(alme_client_id, (INT8U *)out)) {
		PLATFORM_PRINTF_ERROR("Could not send the 1905 ALME reply\n");
		ret = 1;
	} else {
		ret = 0;
	}

	// Free memory
	//
	free_1905_ALME_structure((INT8U *)out);

	return ret;
}

INT8U send1905MetricsResponseALME(INT8U alme_client_id, INT8U *mac_address)
{
	INT8U ret;

	struct getMetricResponseALME *out;

	struct transmitterLinkMetricTLV **tx_tlvs;
	struct receiverLinkMetricTLV **   rx_tlvs;

	INT8U total_tlvs;
	INT8U res;

	INT8U i;

	PLATFORM_PRINTF_INFO("--> ALME_TYPE_GET_METRIC_RESPONSE\n");

	// Fill the requested ALME response
	//
	if (NULL == mac_address) {
		_obtainLocalMetricsTLVs(LINK_METRIC_QUERY_TLV_ALL_NEIGHBORS, NULL,
		                        LINK_METRIC_QUERY_TLV_BOTH_TX_AND_RX_LINK_METRICS,
		                        &tx_tlvs, &rx_tlvs, &total_tlvs);
	} else {
		_obtainLocalMetricsTLVs(LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR, mac_address,
		                        LINK_METRIC_QUERY_TLV_BOTH_TX_AND_RX_LINK_METRICS,
		                        &tx_tlvs, &rx_tlvs, &total_tlvs);
	}

	// Reorder Tx/Rx TLVs in the way they are expected inside an ALME metrics
	// response (which is different from what you have in a "regular" TLV for
	// some strange reason, maybe a "bug" in the standard)
	//
	res = _reStructureMetricsTLVs(&tx_tlvs, &rx_tlvs, &total_tlvs);

	out            = (struct getMetricResponseALME *)PLATFORM_MALLOC(sizeof(struct getMetricResponseALME));
	out->alme_type = ALME_TYPE_GET_METRIC_RESPONSE;

	if (0 == total_tlvs || 0 == res) {
		out->reason_code = REASON_CODE_UNMATCHED_NEIGHBOR_MAC_ADDRESS;
		out->metrics_nr  = 0;
		out->metrics     = NULL;
	} else {
		out->reason_code = REASON_CODE_SUCCESS;
		out->metrics_nr  = total_tlvs;

		out->metrics = (struct _metricDescriptorsEntries *)PLATFORM_MALLOC(sizeof(struct _metricDescriptorsEntries) * out->metrics_nr);

		for (i = 0; i < out->metrics_nr; i++) {
			PLATFORM_MEMCPY(out->metrics[i].neighbor_dev_address, tx_tlvs[i]->neighbor_al_address, 6);
			PLATFORM_MEMCPY(out->metrics[i].local_intf_address, tx_tlvs[i]->transmitter_link_metrics[0].local_interface_address, 6);
			out->metrics[i].bridge_flag = DMisLinkBridged(out->metrics[i].local_intf_address, out->metrics[i].neighbor_dev_address, tx_tlvs[i]->transmitter_link_metrics[0].neighbor_interface_address);
			out->metrics[i].tx_metric   = tx_tlvs[i];
			out->metrics[i].rx_metric   = rx_tlvs[i];
		}
	}

	// Send the packet
	//
	if (0 == send1905RawALME(alme_client_id, (INT8U *)out)) {
		PLATFORM_PRINTF_ERROR("Could not send the 1905 ALME reply\n");
		ret = 1;
	} else {
		ret = 0;
	}

	// Before free'ing the "out" structure, set Tx and Rx pointer to NULL so
	// that "free_1905_ALME_structure()" ignores them.
	// They will be freed later, in "_freeLocalMetricsTLVs()"
	//
	for (i = 0; i < out->metrics_nr; i++) {
		out->metrics[i].tx_metric = NULL;
		out->metrics[i].rx_metric = NULL;
	}
	out->metrics_nr = 0;
	PLATFORM_FREE(out->metrics);
	out->metrics = NULL;
	free_1905_ALME_structure((INT8U *)out);
// coverity[double_free:FALSE]
	_freeLocalMetricsTLVs(&tx_tlvs, &rx_tlvs, &total_tlvs);

	return ret;
}

INT8U send1905CustomCommandResponseALME(INT8U alme_client_id, INT8U command)
{
	INT8U ret;

	struct customCommandResponseALME *out;

	PLATFORM_PRINTF_INFO("--> ALME_TYPE_CUSTOM_COMMAND_RESPONSE\n");

	// Fill the requested ALME response
	//
	out            = (struct customCommandResponseALME *)PLATFORM_MALLOC(sizeof(struct customCommandResponseALME));
	out->alme_type = ALME_TYPE_CUSTOM_COMMAND_RESPONSE;

	switch (command) {
	case CUSTOM_COMMAND_DUMP_NETWORK_DEVICES: {
		// Update the information regarding the local node
		//
		_updateLocalDeviceData();

		// Dump the database (which contains information from the local and
		// remote nodes) into a text buffer and send that as a reponse
		//
		_memoryBufferWriterInit();

		DMdumpNetworkDevices(_memoryBufferWriter);

		memory_buffer[memory_buffer_i] = 0x0;

		out->bytes_nr = memory_buffer_i + 1;
		out->bytes    = memory_buffer;

		break;
	}
	case CUSTOM_COMMAND_GET_CONTROLLER_IP: {
		char *controller_ip = DMgetControllerIP();
		out->bytes_nr       = PLATFORM_STRLEN(controller_ip);
		out->bytes          = controller_ip;
		break;
	}
	case CUSTOM_COMMAND_GET_TOPOLOGY_JSON: {
		out->bytes_nr = 0;
		out->bytes    = NULL;
		if (!DMisController()) {
			break;
		}

		FILE *fp = fopen("/tmp/topology_json", "re");
		if (fp == NULL) {
			PLATFORM_PRINTF_WARNING("Error opening topology_json file!\n");
			break;
		}
		char * line = NULL;
		size_t len  = 0;

		if (getline(&line, &len, fp) != -1) {
			out->bytes_nr = PLATFORM_STRLEN(line);
			out->bytes    = line;
		}
		fclose(fp);

		break;
	}
	default: {
		out->bytes_nr = 0;
		out->bytes    = NULL;
		break;
	}
	}

	// Send the packet
	//
	if (0 == send1905RawALME(alme_client_id, (INT8U *)out)) {
		PLATFORM_PRINTF_ERROR("Could not send the 1905 ALME reply\n");
		ret = 1;
	} else {
		ret = 0;
	}

	// Free memory not needed anymore
	//
	if (CUSTOM_COMMAND_DUMP_NETWORK_DEVICES == command) {
		// Here we will free the memory buffer *and* set the pointer in the
		// "out" structure to NULL, so that later "free_1905_ALME_structure()"
		// doesn't try to free it again
		//
		_memoryBufferWriterEnd();
		out->bytes_nr = 0;
		out->bytes    = NULL;
	}
	free_1905_ALME_structure((INT8U *)out);

	return ret;
}

INT8U fillChannelPreferenceTLV(struct ChannelPreferenceTLV ***to_be_filled, struct ChannelPreferenceTLV **ch_prefs_1, INT8U ch_prefs_1_nr)
{
	if (ch_prefs_1_nr == 0) {
		*to_be_filled = NULL;
		return 0;
	}

	INT8U ch_prefs_2_nr;
	struct ChannelPreferenceTLV **ch_prefs_2;

	ch_prefs_2_nr = ch_prefs_1_nr;
	ch_prefs_2 = PLATFORM_MALLOC(ch_prefs_1_nr * sizeof(struct ChannelPreferenceTLV *));

	int i;
	for (i = 0; i < ch_prefs_1_nr; i++) {
		ch_prefs_2[i] = (struct ChannelPreferenceTLV *)copy_1905_TLV_structure((INT8U *)ch_prefs_1[i]);
	}


	*to_be_filled = ch_prefs_2;
	return ch_prefs_2_nr;
}

INT8U sendMultiApCommandResponseALME(INT8U alme_client_id, struct multiApCommandRequestALME *p)
{
	INT8U                              ret;
	INT16U                             mid;
	INT8U *                            stream;
	INT16U                             stream_len;
	struct multiApCommandResponseALME *out;

	// Send CMDU according to the request
	//
	PLATFORM_PRINTF_INFO("CMDU Type: %04x\n", p->cmdu_type);
	mid = getNextMid();

	char *                interfaceName = NULL;
	struct networkDevice *tmp           = DMnetworkDeviceGet(p->al_mac);

#ifdef RTK_MULTI_AP_R3
	encrypt1905Alme(p, mid);
#endif

	stream = forge_1905_CMDU_from_map_alme(mid, p->cmdu_type, p->tlv_buf, p->tlv_length, &stream_len);

	if (NULL != tmp) {
		// found the intercaface that can reach the multi-ap device with al mac adress(p->al_mac)
		interfaceName = tmp->receiving_interface;
		if (CMDU_TYPE_COMBINED_INFRASTRUCTURE_METRICS == p->cmdu_type) {
			if (0 == sendCombinedInfrastructureMetricMessage(interfaceName, mid, p->al_mac)) {
				PLATFORM_PRINTF_ERROR("CombinedInfrastructureMetricMessage could not be sent!\n");
				mid = 0;
			}
		} else if (CMDU_TYPE_CHANNEL_PREFERENCE_QUERY == p->cmdu_type) {
			if (0 == sendMultiAPChannelPreferenceQueryPacket(interfaceName, mid, p->al_mac)) {
				PLATFORM_PRINTF_ERROR("ChannelPreferenceQueryPacket could not be sent!\n");
				mid = 0;
			}
		} else if (DMgetWFATestMode() && CMDU_TYPE_CHANNEL_SELECTION_REQUEST == p->cmdu_type && p->tlv_length == 0) {
			struct ChannelPreferenceTLV **ch_prefs;
			INT8U ch_prefs_nr;
			ch_prefs_nr = fillChannelPreferenceTLV(&ch_prefs, tmp->last_ch_prefs, tmp->last_ch_prefs_nr);
			INT8U res = sendMultiAPChannelSelectionRequestPacket(interfaceName, mid, p->al_mac, ch_prefs, ch_prefs_nr, NULL, 0);
			freeChannelPreferenceTLV(ch_prefs, ch_prefs_nr);
			if (0 == res) {
				PLATFORM_PRINTF_ERROR("ChannelSelectionRequestPacket could not be sent!\n");
				mid = 0;
			}
		}
#ifdef RTK_MULTI_AP_R2
		else if (CMDU_TYPE_CHANNEL_SCAN_REQUEST == p->cmdu_type) {
			if (stream_len < 6) {
				if (0 == sendChannelScanRequestPacket(interfaceName, mid, NULL, p->al_mac)) {
					PLATFORM_PRINTF_ERROR("ChannelScanRequestPacket could not be sent!\n");
					PLATFORM_PRINTF_INFO("Sending 1905 message on interface %s, MID %04x \n", interfaceName, mid);
					if (0 == PLATFORM_SEND_RAW_PACKET(interfaceName, p->al_mac, DMalMacGet(), ETHERTYPE_1905, stream, stream_len)) {
						PLATFORM_PRINTF_ERROR("Packet could not be sent!\n");
						mid = 0;
					}
				}
			} else {
				PLATFORM_PRINTF_INFO("Sending 1905 message on interface %s, MID %04x \n", interfaceName, mid);
				if (0 == PLATFORM_SEND_RAW_PACKET(interfaceName, p->al_mac, DMalMacGet(), ETHERTYPE_1905, stream, stream_len)) {
					PLATFORM_PRINTF_ERROR("Packet could not be sent!\n");
					mid = 0;
				}
			}
		}
#endif
		else {
			PLATFORM_PRINTF_INFO("Sending 1905 message on interface %s, MID %04x \n", interfaceName, mid);
			if (0 == PLATFORM_SEND_RAW_PACKET(interfaceName, p->al_mac, DMalMacGet(), ETHERTYPE_1905, stream, stream_len)) {
				PLATFORM_PRINTF_ERROR("Packet could not be sent!\n");
				mid = 0;
			}
		}
	} else {
		// since we don't know which interface the the multi-ap device(p->al_mac) connect with,
		// we send the stream by backhaul station interface and all the backhaul BSS interfaces
		char **ifs_names;
		INT8U  ifs_nr;
		ifs_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&ifs_nr);

		int i;
		for (i = 0; i < ifs_nr; i++) {
			struct LocalInterface *l = DMnameToLocalInterfaceStruct(ifs_names[i]);
			if (NULL == l) {
				continue;
			} else if ((l->role & BACKHAUL_BSS) || (l->role & BACKHAUL_STA)) {
				interfaceName = l->name;
				if (CMDU_TYPE_COMBINED_INFRASTRUCTURE_METRICS == p->cmdu_type) {
					if (0 == sendCombinedInfrastructureMetricMessage(interfaceName, mid, p->al_mac)) {
						PLATFORM_PRINTF_ERROR("CombinedInfrastructureMetricMessage could not be sent!\n");
					}
				} else if (CMDU_TYPE_CHANNEL_PREFERENCE_QUERY == p->cmdu_type) {
					if (0 == sendMultiAPChannelPreferenceQueryPacket(interfaceName, mid, p->al_mac)) {
						PLATFORM_PRINTF_ERROR("ChannelPreferenceQueryPacket could not be sent!\n");
					}
				}
#ifdef RTK_MULTI_AP_R2
				else if (CMDU_TYPE_CHANNEL_SCAN_REQUEST == p->cmdu_type) {
					if (stream_len < 6) {
						if (0 == sendChannelScanRequestPacket(interfaceName, mid, NULL, p->al_mac)) {
							PLATFORM_PRINTF_ERROR("ChannelScanRequestPacket could not be sent!\n");
							PLATFORM_PRINTF_INFO("Sending 1905 message on interface %s, MID %04x \n", interfaceName, mid);
							if (0 == PLATFORM_SEND_RAW_PACKET(interfaceName, p->al_mac, DMalMacGet(), ETHERTYPE_1905, stream, stream_len)) {
								PLATFORM_PRINTF_ERROR("Packet could not be sent!\n");
							}
						}
					} else {
						PLATFORM_PRINTF_INFO("Sending 1905 message on interface %s, MID %04x \n", interfaceName, mid);
						if (0 == PLATFORM_SEND_RAW_PACKET(interfaceName, p->al_mac, DMalMacGet(), ETHERTYPE_1905, stream, stream_len)) {
							PLATFORM_PRINTF_ERROR("Packet could not be sent!\n");
						}
					}
				}
#endif
				else {
					PLATFORM_PRINTF_INFO("Sending 1905 message on interface %s, MID %04x \n", interfaceName, mid);
					if (0 == PLATFORM_SEND_RAW_PACKET(interfaceName, p->al_mac, DMalMacGet(), ETHERTYPE_1905, stream, stream_len)) {
						PLATFORM_PRINTF_ERROR("Packet could not be sent!\n");
					}
				}
			}
		}
	}

	free(stream);

	PLATFORM_PRINTF_INFO("--> ALME_TYPE_MULTIAP_COMMAND_RESPONSE\n");

	// Fill the requested ALME response
	//
	out            = (struct multiApCommandResponseALME *)PLATFORM_MALLOC(sizeof(struct multiApCommandResponseALME));
	out->alme_type = ALME_TYPE_MULTIAP_COMMAND_RESPONSE;

	out->mid = mid;

	// Send the packet
	//
	if (0 == send1905RawALME(alme_client_id, (INT8U *)out)) {
		PLATFORM_PRINTF_ERROR("Could not send the 1905 ALME reply\n");
		ret = 1;
	} else {
		ret = 0;
	}

	// Free memory not needed anymore
	//
	free_1905_ALME_structure((INT8U *)out);

	return ret;
}

INT8U sendMultiAPPolicyConfigRequestPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, INT8U policy_inclusion,
                                           struct SteeringPolicyTLV *             steer_policy,
                                           struct MetricReportingPolicyTLV *      metric_policy,
                                           struct ChannelScanReportingPolicyTLV * scan_policy,
                                           struct Default8021QSettingsTLV *       default8021q_config,
                                           struct TrafficSeparationPolicyTLV *    traffic_separation_policy)

{
	// The "Multi-AP Policy Config Request" message is a CMDU with two TLVs:
	// - Zero or one Steering Policy TLV (see section 17.2.11).
	// - Zero or one Metric Reporting Policy TLV (see section 17.2.12).
	// - Zero or one Channel Scan Reporting Policy TLV (see section 17.2.37).

	struct CMDU request_msg;
	INT8U       tlv_index = 0, tlv_count = 0;
	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_MULTI_AP_POLICY_CONFIG_REQUEST (%s) MID: (%04x)\n", interface_name, mid);

	if (policy_inclusion & 1) {
		tlv_count++;
	}
	if (policy_inclusion & 2) {
		tlv_count++;
	}
	if (policy_inclusion & 4) {
		tlv_count++;
	}
	if (policy_inclusion & 8) {
		tlv_count++;
	}
	if (policy_inclusion & 16) {
		tlv_count++;
	}

	// Build the CMDU
	//
	request_msg.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	request_msg.message_type    = CMDU_TYPE_MULTI_AP_POLICY_CONFIG_REQUEST;
	request_msg.message_id      = mid;
	request_msg.relay_indicator = 0;
	request_msg.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * (tlv_count + 1));
	if (policy_inclusion & 1) {
		request_msg.list_of_TLVs[tlv_index] = (INT8U *)steer_policy;
		tlv_index++;
	}
	if (policy_inclusion & 2) {
		request_msg.list_of_TLVs[tlv_index] = (INT8U *)metric_policy;
		tlv_index++;
	}
	if (policy_inclusion & 4) {
		request_msg.list_of_TLVs[tlv_index] = (INT8U *)scan_policy;
		tlv_index++;
	}
	if (policy_inclusion & 8) {
		request_msg.list_of_TLVs[tlv_index] = (INT8U *)default8021q_config;
		tlv_index++;
	}
	if (policy_inclusion & 16) {
		request_msg.list_of_TLVs[tlv_index] = (INT8U *)traffic_separation_policy;
		tlv_index++;
	}
	request_msg.list_of_TLVs[tlv_index] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, dest_mac_address, &request_msg, true, 0)) {
		PLATFORM_PRINTF_ERROR("Could not send the Multi-AP PolicyConfigRequest packet\n");
		PLATFORM_FREE(request_msg.list_of_TLVs);
		return 0;
	}
	api_send_de_tlvs_response(request_msg.message_type, dest_mac_address, request_msg.list_of_TLVs);
	// Free memory
	//
	PLATFORM_FREE(request_msg.list_of_TLVs);
	return 1;
}

INT8U sendMultiAPHigherLayerDataPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, INT8U protocol, INT8U *payload)
{
	// 17.1.31 Higher Layer Data message format
	// The following TLV shall be included in this message:
	//  One Higher Layer Data TLV (see section 17.2.34).

	struct CMDU               cmdu_msg;
	struct HigherLayerDataTLV higher_layer_data_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_MULTI_AP_Higher_Layer_Data (%s) MID: (%04x)\n", interface_name, mid);

	// Fill the HigherLayerDataTLV
	//
	higher_layer_data_tlv.tlv_type = TLV_TYPE_HIGHER_LAYER_DATA;
	higher_layer_data_tlv.protocol = protocol;

	// Higher layer protocol payload (To be defined for specific higher layer protocol).
	// todo get the payload length
	INT16U payload_len            = 0;
	higher_layer_data_tlv.payload = (INT8U *)PLATFORM_MALLOC(payload_len);
	PLATFORM_MEMCPY(higher_layer_data_tlv.payload, payload, payload_len);

	// Build the CMDU
	//
	cmdu_msg.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	cmdu_msg.message_type    = CMDU_TYPE_HIGHER_LAYER_DATA;
	cmdu_msg.message_id      = mid;
	cmdu_msg.relay_indicator = 0;
	cmdu_msg.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);
	cmdu_msg.list_of_TLVs[1] = (INT8U *)&higher_layer_data_tlv;
	cmdu_msg.list_of_TLVs[2] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, dest_mac_address, &cmdu_msg, true, 0)) {
		PLATFORM_PRINTF_ERROR("Could not send the Multi-AP Higher Layer Data packet\n");
		PLATFORM_FREE(cmdu_msg.list_of_TLVs);
		return 0;
	}
	// Free memory
	//
	PLATFORM_FREE(higher_layer_data_tlv.payload);
	PLATFORM_FREE(cmdu_msg.list_of_TLVs);
	return 1;
}

INT8U send1905ACKPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, INT8U reason_code, INT8U sta_mac_address[])
{
	// The "1906 ACK" message is a CMDU with zero or one TLVs:
	// - Zero or more Error Code TLVs (see section 17.2.36).

	struct CMDU         cmdu_msg;
	struct ErrorCodeTLV error_code_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_1905_ACK (%s) MID: %04x\n", interface_name, mid);

	// Fill the ErrorCodeTLV
	//
	error_code_tlv.reason_code = reason_code;
	error_code_tlv.tlv_type    = TLV_TYPE_ERROR_CODE;
	if (NULL != sta_mac_address)
		PLATFORM_MEMCPY(error_code_tlv.sta_mac_address, sta_mac_address, 6);
	else
		PLATFORM_MEMSET(error_code_tlv.sta_mac_address, 0x00, 6);

	// Build the CMDU
	//
	cmdu_msg.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	cmdu_msg.message_type    = CMDU_TYPE_1905_ACK;
	cmdu_msg.message_id      = mid;
	cmdu_msg.relay_indicator = 0;
	if (0 == reason_code) { // no need error code TLV
		cmdu_msg.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *));
		cmdu_msg.list_of_TLVs[0] = NULL;
	} else {
		cmdu_msg.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);
		cmdu_msg.list_of_TLVs[0] = (INT8U *)&error_code_tlv;
		cmdu_msg.list_of_TLVs[1] = NULL;
	}

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, dest_mac_address, &cmdu_msg, false, 0)) {
		PLATFORM_PRINTF_ERROR("Could not send the 1905 ACK packet MID: %04x\n", mid);
		PLATFORM_FREE(cmdu_msg.list_of_TLVs);
		return 0;
	}

	// Free memory
	//
	PLATFORM_FREE(cmdu_msg.list_of_TLVs);
	return 1;
}

INT8U sendMultiAPChannelPreferenceQueryPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address)
{
	// The "Multi-AP Channel Preference Query" message is a CMDU with no TLVs

	INT8U ret;

	struct CMDU query_message;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_MULTI_AP_CHANNEL_PREFERENCE_QUERY (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", dest_mac_address[0], dest_mac_address[1], dest_mac_address[2], dest_mac_address[3], dest_mac_address[4], dest_mac_address[5]);

	// Build the CMDU
	//
	query_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	query_message.message_type    = CMDU_TYPE_CHANNEL_PREFERENCE_QUERY;
	query_message.message_id      = mid;
	query_message.relay_indicator = 0;
	query_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *));
	query_message.list_of_TLVs[0] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, dest_mac_address, &query_message, true, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	DMchannelQueryStatusSet(1);

	// Free memory
	//
	PLATFORM_FREE(query_message.list_of_TLVs);

	return ret;
}

INT8U sendMultiAPChannelPreferenceReportPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, INT8U *cac_tlv, bool need_ack)
{
	// The "Multi-AP Channel Preference Report" message is a CMDU with the following TLVs:
	//   - Zero or more Channel Preference TLVs
	//   - Zero or more Radio Operation Restriction TLVs

	INT8U ret;

	struct CMDU                         report_message;
	INT8U                               channel_preference_tlv_nr;
	struct ChannelPreferenceTLV **      channel_preference_tlv;
	struct RadioOperationRestrictionTLV radio_operation_restriction_tlv;
	INT8U                               tlv_nr = 0;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_MULTI_AP_CHANNEL_PREFERENCE_REPORT (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", dest_mac_address[0], dest_mac_address[1], dest_mac_address[2], dest_mac_address[3], dest_mac_address[4], dest_mac_address[5]);

	// Fill the ChannelPreferenceTLV
	//
	channel_preference_tlv_nr = _obtainChannelPreferenceTLV(&channel_preference_tlv);

	// Fill the RadioOperationRestrictionTLV
	//
	_obtainRadioOperationRestrictionTLV(&radio_operation_restriction_tlv);
#ifdef RTK_MULTI_AP_R2
	struct CACStatusReportTLV *cac_status_report_tlv = PLATFORM_MALLOC(sizeof(struct CACStatusReportTLV));
	_obtainCACStatusTLV(cac_status_report_tlv);
#endif
	// Build the CMDU
	//
	report_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	report_message.message_type    = CMDU_TYPE_CHANNEL_PREFERENCE_REPORT;
	report_message.message_id      = mid;
	report_message.relay_indicator = 0;

	tlv_nr = channel_preference_tlv_nr + 1; // End of Message TLV
#ifdef RTK_MULTI_AP_R2
	if (MULTI_AP_PROFILE_2 <= DMgetMultiAPProfile() && MULTI_AP_PROFILE_2 <= DMgetTargetDeviceMultiAPProfile(dest_mac_address)) {
		if (cac_tlv) {
			tlv_nr += 1; // CAC Completion Report
		}
		tlv_nr += 1; // CAC Status Report
	}
#else
	if (cac_tlv) {
		PLATFORM_PRINTF_WARNING("There should not be CAC tlv in R1\n");
	}
#endif
	// Set channel preference tlv if any
	//
	int i;
	report_message.list_of_TLVs = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * (tlv_nr));
	for (i = 0; i < channel_preference_tlv_nr; i++) {
		report_message.list_of_TLVs[i] = (INT8U *)channel_preference_tlv[i];
	}
#ifdef RTK_MULTI_AP_R2
	if (MULTI_AP_PROFILE_2 <= DMgetMultiAPProfile() && MULTI_AP_PROFILE_2 <= DMgetTargetDeviceMultiAPProfile(dest_mac_address)) {
		if (cac_tlv) {
			report_message.list_of_TLVs[i++] = (INT8U *)cac_tlv;
		}
		report_message.list_of_TLVs[i++] = (INT8U *)cac_status_report_tlv;
	}
#endif
	report_message.list_of_TLVs[tlv_nr - 1] = NULL;
	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, dest_mac_address, &report_message, need_ack, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}
	// Free memory
	//
	_freeRadioOperationRestrictionTLV(&radio_operation_restriction_tlv);
	freeChannelPreferenceTLV(channel_preference_tlv, channel_preference_tlv_nr);
#ifdef RTK_MULTI_AP_R2
	free_1905_TLV_structure((INT8U *)cac_status_report_tlv);
#endif
	PLATFORM_FREE(report_message.list_of_TLVs);
	return ret;
}

INT8U sendMultiAPChannelSelectionRequestPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, struct ChannelPreferenceTLV **channel_preference_tlv, INT8U channel_preference_tlv_nr, struct TransmitPowerLimitTLV **transmit_power_limit_tlv, INT8U transmit_power_limit_tlv_nr)
{
	// The "Multi-AP Channel Selection Request" message is a CMDU with the following TLVs:
	//   - Zero or more Channel Preference TLVs
	//   - Zero or more Transmit Power Limit TLVs

	INT8U ret;

	struct CMDU request_message;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_MULTI_AP_CHANNEL_SELECTION_REQUEST (%s) MID: (%04x)\n", interface_name, mid);

	// Build the CMDU
	//
	request_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	request_message.message_type    = CMDU_TYPE_CHANNEL_SELECTION_REQUEST;
	request_message.message_id      = mid;
	request_message.relay_indicator = 0;

	// Set channel preference tlv if any
	//
	if (channel_preference_tlv_nr > 0 || transmit_power_limit_tlv_nr > 0) {
		int i;
		request_message.list_of_TLVs = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * (channel_preference_tlv_nr + transmit_power_limit_tlv_nr + 1));
		for (i = 0; i < channel_preference_tlv_nr; i++) {
			request_message.list_of_TLVs[i] = (INT8U *)channel_preference_tlv[i];
		}
		for (i = 0; i < transmit_power_limit_tlv_nr; i++) {
			request_message.list_of_TLVs[i + channel_preference_tlv_nr] = (INT8U *)transmit_power_limit_tlv[i];
		}
		request_message.list_of_TLVs[channel_preference_tlv_nr + transmit_power_limit_tlv_nr] = NULL;
	} else {
		request_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *));
		request_message.list_of_TLVs[0] = NULL;
	}

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, dest_mac_address, &request_message, true, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	PLATFORM_FREE(request_message.list_of_TLVs);

	return ret;
}

INT8U sendMultiAPChannelSelectionResponsePacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, INT8U response)
{
	// The "Multi-AP Channel Selection Response" message is a CMDU with the following TLVs:
	//   - One or more Channel Selection Response TLVs
	//   - Zero or more Spatial Reuse Config Response TLVs

	INT8U ret;

	struct CMDU                          response_message;
	INT8U                                channel_selection_response_tlv_nr = 0;
	struct ChannelSelectionResponseTLV **channel_selection_response_tlv = NULL;
	INT8U                                total_tlv_nr = 0;

#ifdef RTK_MULTI_AP_R4
	INT8U                                 spatial_reuse_response_tlv_nr = 0;
	struct SpatialReuseConfigResponseTLV *spatial_reuse_response_tlv = NULL;
#endif //RTK_MULTI_AP_R4

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_MULTI_AP_CHANNEL_SELECTION_RESPONSE (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", dest_mac_address[0], dest_mac_address[1], dest_mac_address[2], dest_mac_address[3], dest_mac_address[4], dest_mac_address[5]);

	// Fill the ChannelSelectionResponseTLV
	//
	channel_selection_response_tlv_nr = _obtainChannelSelectionResponseTLV(&channel_selection_response_tlv, response);
	total_tlv_nr += channel_selection_response_tlv_nr;
#ifdef RTK_MULTI_AP_R4
	if (DMgetMultiAPProfile() >= MULTI_AP_PROFILE_4) {
		spatial_reuse_response_tlv_nr = _obtainSpatialReuseConfigResponseTLV(&spatial_reuse_response_tlv);
		total_tlv_nr += spatial_reuse_response_tlv_nr;
	}
#endif //RTK_MULTI_AP_R4
	// Build the CMDU
	//
	response_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	response_message.message_type    = CMDU_TYPE_CHANNEL_SELECTION_RESPONSE;
	response_message.message_id      = mid;
	response_message.relay_indicator = 0;

	// Set channel preference tlv if any
	//
	if (total_tlv_nr > 0) {
		int total_tlv_index           = 0;
		int i                         = 0;
		response_message.list_of_TLVs = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * (total_tlv_nr + 1));
		for (i = 0; i < channel_selection_response_tlv_nr; i++) {
			response_message.list_of_TLVs[total_tlv_index++] = (INT8U *)channel_selection_response_tlv[i];
		}
#ifdef RTK_MULTI_AP_R4
		if (DMgetMultiAPProfile() >= MULTI_AP_PROFILE_4) {
			for (i = 0; i < spatial_reuse_response_tlv_nr; i++) {
				response_message.list_of_TLVs[total_tlv_index++] = (INT8U *)&spatial_reuse_response_tlv[i];
			}
		}
#endif //RTK_MULTI_AP_R4
		response_message.list_of_TLVs[total_tlv_nr] = NULL;
	} else {
		response_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *));
		response_message.list_of_TLVs[0] = NULL;
	}

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, dest_mac_address, &response_message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	_freeChannelSelectionResponseTLV(channel_selection_response_tlv, channel_selection_response_tlv_nr);
#ifdef RTK_MULTI_AP_R4
	if (DMgetMultiAPProfile() >= MULTI_AP_PROFILE_4) {
		PLATFORM_FREE(spatial_reuse_response_tlv);
	}
#endif //RTK_MULTI_AP_R4
	PLATFORM_FREE(response_message.list_of_TLVs);

	return ret;
}

INT8U sendMultiAPOperatingChannelReportPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address)
{
	// The "Multi-AP Operating Channel Report" message is a CMDU with the following TLVs:
	//   - One or more Operating Channel Report TLVs
	//   - Zero or more Spatial Reuse Report TLVs

	INT8U ret;

	struct CMDU                        report_message;
	INT8U                              operating_channel_report_tlv_nr = 0;
	struct OperatingChannelReportTLV **operating_channel_report_tlv    = NULL;
	INT8U total_tlv_nr = 0;

#ifdef RTK_MULTI_AP_R4
	INT8U                          spatial_reuse_report_tlv_nr = 0;
	struct SpatialReuseReportTLV **spatial_reuse_report_tlv    = NULL;
#endif //RTK_MULTI_AP_R4

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_MULTI_AP_OPERATING_CHANNEL_REPORT (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", dest_mac_address[0], dest_mac_address[1], dest_mac_address[2], dest_mac_address[3], dest_mac_address[4], dest_mac_address[5]);

	// Fill the OperatingChannelReportTLV and SpatialReuseReportTLV
	//
	operating_channel_report_tlv_nr = _obtainOperatingChannelReportTLV(&operating_channel_report_tlv);
	total_tlv_nr += operating_channel_report_tlv_nr;
#ifdef RTK_MULTI_AP_R4
	if (DMgetMultiAPProfile() >= MULTI_AP_PROFILE_4) {
		spatial_reuse_report_tlv_nr = _collectSpatialReuseReportTLVs(&spatial_reuse_report_tlv);
		total_tlv_nr += spatial_reuse_report_tlv_nr;
	}
#endif //RTK_MULTI_AP_R4
	// Build the CMDU
	//
	report_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	report_message.message_type    = CMDU_TYPE_OPERATING_CHANNEL_REPORT;
	report_message.message_id      = mid;
	report_message.relay_indicator = 0;

	// Set operating channel report tlv if any
	//
	if (total_tlv_nr) {
		int total_tlv_index         = 0;
		int i                       = 0;
		report_message.list_of_TLVs = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * (total_tlv_nr + 1));
		for (i = 0; i < operating_channel_report_tlv_nr; i++) {
			report_message.list_of_TLVs[total_tlv_index++] = (INT8U *)operating_channel_report_tlv[i];
		}
#ifdef RTK_MULTI_AP_R4
		if (DMgetMultiAPProfile() >= MULTI_AP_PROFILE_4) {
			for (i = 0; i < spatial_reuse_report_tlv_nr; i++) {
				report_message.list_of_TLVs[total_tlv_index++] = (INT8U *)spatial_reuse_report_tlv[i];
			}
		}
#endif //RTK_MULTI_AP_R4
		report_message.list_of_TLVs[total_tlv_nr] = NULL;
	} else {
		report_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *));
		report_message.list_of_TLVs[0] = NULL;
	}

	//signal agent_main for setting channel to apmib to keep channel changes even after reboot
	INT8U dummy_addr[6] = { 0 };
	api_send_tlvs_response(MAP_OPERATING_CHANNEL_REPORT_ELEMENT, dummy_addr, report_message.list_of_TLVs);

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, dest_mac_address, &report_message, true, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	_freeOperatingChannelReportTLV(operating_channel_report_tlv, operating_channel_report_tlv_nr);
#ifdef RTK_MULTI_AP_R4
	if (DMgetMultiAPProfile() >= MULTI_AP_PROFILE_4) {
		_freeSpatialReuseReportTLV(spatial_reuse_report_tlv, spatial_reuse_report_tlv_nr);
	}
#endif //RTK_MULTI_AP_R4
	PLATFORM_FREE(report_message.list_of_TLVs);

	return ret;
}

INT8U sendMultiAPClientSteeringRequestPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, struct steering_request *steering_req)
{
	// The "Multi-AP Client Steering Request" message is a CMDU with the following TLVs:
	//   - One Steering Request TLV

	// If the message is sent to a Multi-AP Agent that implements Profile-2 from a Multi-AP device that implements only Profile-2:
	// 	 - Zero or one Steering Request TLV to non-Agile Multiband capable STAs.
	//   - Zero or one Profile-2 Steering Request TLV to Agile Multiband capable STAs.

	INT8U ret, i, tlv_count, steering_req_nr, r2_steering_req_nr;

	struct CMDU                       request_message;
	struct SteeringRequestTLV         steering_request_tlv    = { 0 };
	struct Profile2SteeringRequestTLV r2_steering_request_tlv = { 0 };

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_MULTI_AP_CLIENT_STEERING_REQUEST (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", dest_mac_address[0], dest_mac_address[1], dest_mac_address[2], dest_mac_address[3], dest_mac_address[4], dest_mac_address[5]);

	if (steering_req->mbo_cap) {
		steering_req_nr    = 0;
		r2_steering_req_nr = 1;
	} else {
		steering_req_nr    = 1;
		r2_steering_req_nr = 0;
	}

	tlv_count = 0;
	// Fill the SteeringRequestTLV
	//
	if (steering_req_nr) {
		_obtainSteeringRequestTLV(&steering_request_tlv, steering_req);
		tlv_count += 1;
	}
	if (r2_steering_req_nr) {
		_obtainR2SteeringRequestTLV(&r2_steering_request_tlv, steering_req);
		tlv_count += 1;
	}

	// Build the CMDU
	//
	request_message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	request_message.message_type    = CMDU_TYPE_CLIENT_STEERING_REQUEST;
	request_message.message_id      = mid;
	request_message.relay_indicator = 0;
	request_message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * (tlv_count + 1));

	i = 0;
	if (steering_req_nr) {
		request_message.list_of_TLVs[i] = (INT8U *)&steering_request_tlv;
		i++;
	}
	if (r2_steering_req_nr) {
		request_message.list_of_TLVs[i] = (INT8U *)&r2_steering_request_tlv;
		i++;
	}

	request_message.list_of_TLVs[tlv_count] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, dest_mac_address, &request_message, true, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	if (steering_req_nr) {
		_freeSteeringRequestTLV(&steering_request_tlv);
	}
	if (r2_steering_req_nr) {
		_freeR2SteeringRequestTLV(&r2_steering_request_tlv);
	}
	PLATFORM_FREE(request_message.list_of_TLVs);

	return ret;
}

INT8U sendMultiAPClientSteeringBTMReportPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, struct bss_transition_response_para *response)
{
	// The "Multi-AP Client Steering BTM Report" message is a CMDU with the following TLVs:
	//   - One Steering BTM Report TLV

	INT8U ret;

	struct CMDU                 message;
	struct SteeringBTMReportTLV steering_btm_report_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_MULTI_AP_CLIENT_STEERING_BTM_REPORT (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", dest_mac_address[0], dest_mac_address[1], dest_mac_address[2], dest_mac_address[3], dest_mac_address[4], dest_mac_address[5]);

	// Fill the SteeringBTMReporttTLV
	//
	_obtainSteeringBTMReportTLV(&steering_btm_report_tlv, response);

	// Build the CMDU
	//
	message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	message.message_type    = CMDU_TYPE_CLIENT_STEERING_BTM_REPORT;
	message.message_id      = mid;
	message.relay_indicator = 0;
	message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);
	message.list_of_TLVs[0] = (INT8U *)&steering_btm_report_tlv;
	message.list_of_TLVs[1] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, dest_mac_address, &message, true, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	// _freeSteeringBTMReportTLV(&steering_btm_report_tlv);
	PLATFORM_FREE(message.list_of_TLVs);

	return ret;
}

INT8U sendMultiAPClientAssociationControlRequestPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, struct client_association_control_request *client_assoc_ctrl_req, INT8U client_assoc_ctrl_req_nr)
{
	// The "Multi-AP Client Association Control Request" message is a CMDU with the following TLVs:
	//   - One or more Client Association Control Request TLVs

	INT8U ret;

	struct CMDU                                message;
	struct ClientAssociationControlRequestTLV *client_association_control_request_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_MULTI_AP_CLIENT_ASSOCIATION_CONTROL_REQUEST (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", dest_mac_address[0], dest_mac_address[1], dest_mac_address[2], dest_mac_address[3], dest_mac_address[4], dest_mac_address[5]);

	// Fill the ClientAssociationControlRequestTLV
	//
	_obtainClientAssociationControlRequestTLV(&client_association_control_request_tlv, client_assoc_ctrl_req, client_assoc_ctrl_req_nr);

	// Build the CMDU
	//
	message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	message.message_type    = CMDU_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST;
	message.message_id      = mid;
	message.relay_indicator = 0;

	// Set operating channel report tlv if any
	//
	if (client_assoc_ctrl_req_nr > 0) {
		int i;
		message.list_of_TLVs = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * (client_assoc_ctrl_req_nr + 1));
		for (i = 0; i < client_assoc_ctrl_req_nr; i++) {
			message.list_of_TLVs[i] = (INT8U *)&client_association_control_request_tlv[i];
		}
		message.list_of_TLVs[client_assoc_ctrl_req_nr] = NULL;
	} else {
		message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *));
		message.list_of_TLVs[0] = NULL;
	}

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, dest_mac_address, &message, true, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	_freeClientAssociationControlRequestTLV(client_association_control_request_tlv, client_assoc_ctrl_req_nr);
	PLATFORM_FREE(message.list_of_TLVs);

	return ret;
}

INT8U sendMultiAPSteeringCompletedPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address)
{
	// The "Multi-AP Client Steering Request" message is a CMDU with no TLVs

	INT8U ret;

	struct CMDU message;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_MULTI_AP_STEERING_COMPLETED (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", dest_mac_address[0], dest_mac_address[1], dest_mac_address[2], dest_mac_address[3], dest_mac_address[4], dest_mac_address[5]);

	// Build the CMDU
	//
	message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	message.message_type    = CMDU_TYPE_STEERING_COMPLETED;
	message.message_id      = mid;
	message.relay_indicator = 0;
	message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *));
	message.list_of_TLVs[0] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, dest_mac_address, &message, true, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	PLATFORM_FREE(message.list_of_TLVs);

	return ret;
}

INT8U send1905VendorSpecificMessage(char *interface_name, INT16U mid, INT8U *dest_mac_address, INT8U relay_indicator, struct vendorSpecificTLV *vendor_tlv)
{
	// The "Multi-AP Client Steering Request" message is a CMDU with no TLVs

	INT8U ret;

	struct CMDU message;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_VENDOR_SPECIFIC (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", dest_mac_address[0], dest_mac_address[1], dest_mac_address[2], dest_mac_address[3], dest_mac_address[4], dest_mac_address[5]);

	// Build the CMDU
	//
	message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	message.message_type    = CMDU_TYPE_VENDOR_SPECIFIC;
	message.message_id      = mid;
	message.relay_indicator = relay_indicator;
	message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);
	message.list_of_TLVs[0] = (INT8U *)vendor_tlv;
	message.list_of_TLVs[1] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, dest_mac_address, &message, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	PLATFORM_FREE(message.list_of_TLVs);

	return ret;
}

#ifdef RTK_MULTI_AP_R2
//-------------------------------------------------------------------//
//                        Multi-AP Profile 2                         //
//-------------------------------------------------------------------//

INT8U sendCACTerminationPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address)
{
	INT8U ret;

	struct CACTerminationTLV cac_termination_tlv;
	cac_termination_tlv.tlv_type = TLV_TYPE_CAC_TERMINATION;

	cac_termination_tlv.radio_nr = 1;
	cac_termination_tlv.radios   = (struct CACTerminationRadio *)PLATFORM_MALLOC(1 * sizeof(struct CACTerminationRadio));

	{
		INT8U tmp_mac1[6] = { 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc };
		// INT8U tmp_mac1[6] = {0x00, 0xEE, 0xE3, 0x36, 0x75, 0x81};
		PLATFORM_MEMCPY(cac_termination_tlv.radios[0].radio_unique_identifier, tmp_mac1, 6);
		cac_termination_tlv.radios[0].op_class = 4;
		cac_termination_tlv.radios[0].channel  = 5;
	}
	// {
	// 	INT8U tmp_mac2[6] = {0x00, 0xEE, 0xE3, 0x36, 0x75, 0x81};
	// 	PLATFORM_MEMCPY(cac_termination_tlv.radios[1].radio_unique_identifier, tmp_mac2, 6);
	// 	cac_termination_tlv.radios[1].op_class_nr = 1;
	// 	cac_termination_tlv.radios[1].op_classes = (struct ChannelScanTargetOpClass*)PLATFORM_MALLOC(1 * sizeof(struct ChannelScanTargetOpClass));
	// 	{
	// 		cac_termination_tlv.radios[1].op_classes[0].op_class = 81;
	// 		cac_termination_tlv.radios[1].op_classes[0].channel_nr = 1;
	// 		// cac_termination_tlv.radios[1].op_classes[0].channel_list = NULL;
	// 		cac_termination_tlv.radios[1].op_classes[0].channel_list = (INT8U*)PLATFORM_MALLOC(1 * sizeof(INT8U));
	// 		cac_termination_tlv.radios[1].op_classes[0].channel_list[0] = 7;
	// 	}
	// }

	struct CMDU message;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_CAC_TERMINATION (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", destination_al_mac_address[0], destination_al_mac_address[1], destination_al_mac_address[2], destination_al_mac_address[3], destination_al_mac_address[4], destination_al_mac_address[5]);

	// Build the CMDU
	//
	message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	message.message_type    = CMDU_TYPE_CAC_TERMINATION;
	message.message_id      = mid;
	message.relay_indicator = 0;
	message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);
	message.list_of_TLVs[0] = (INT8U *)&cac_termination_tlv;
	message.list_of_TLVs[1] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &message, true, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	//free cac_termination_tlv

	if (cac_termination_tlv.radio_nr)
		PLATFORM_FREE(cac_termination_tlv.radios);

	PLATFORM_FREE(message.list_of_TLVs);

	return ret;
}

INT8U sendCACRequestPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, struct CACRequestTLV cac_request_tlv)
{
	INT8U ret;

	// {
	// 	INT8U tmp_mac2[6] = {0x00, 0xEE, 0xE3, 0x36, 0x75, 0x81};
	// 	PLATFORM_MEMCPY(cac_request_tlv.radios[1].radio_unique_identifier, tmp_mac2, 6);
	// 	cac_request_tlv.radios[1].op_class_nr = 1;
	// 	cac_request_tlv.radios[1].op_classes = (struct ChannelScanTargetOpClass*)PLATFORM_MALLOC(1 * sizeof(struct ChannelScanTargetOpClass));
	// 	{
	// 		cac_request_tlv.radios[1].op_classes[0].op_class = 81;
	// 		cac_request_tlv.radios[1].op_classes[0].channel_nr = 1;
	// 		// cac_request_tlv.radios[1].op_classes[0].channel_list = NULL;
	// 		cac_request_tlv.radios[1].op_classes[0].channel_list = (INT8U*)PLATFORM_MALLOC(1 * sizeof(INT8U));
	// 		cac_request_tlv.radios[1].op_classes[0].channel_list[0] = 7;
	// 	}
	// }

	struct CMDU message;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_CAC_REQUEST (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", destination_al_mac_address[0], destination_al_mac_address[1], destination_al_mac_address[2], destination_al_mac_address[3], destination_al_mac_address[4], destination_al_mac_address[5]);

	// Build the CMDU
	//
	message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	message.message_type    = CMDU_TYPE_CAC_REQUEST;
	message.message_id      = mid;
	message.relay_indicator = 0;
	message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);
	message.list_of_TLVs[0] = (INT8U *)&cac_request_tlv;
	message.list_of_TLVs[1] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &message, true, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	//free cac_request_tlv

	if (cac_request_tlv.radio_nr)
		PLATFORM_FREE(cac_request_tlv.radios);

	PLATFORM_FREE(message.list_of_TLVs);

	return ret;
}

//This function constructs a channel scan request from the stored channel scan capabilites received from that device
INT8U _construct_channel_scan_request(struct ChannelScanRequestTLV *t, INT8U *al_mac)
{
	struct ChannelScanCapabilitiesTLV *t_stored = DMgetChannelScanCapabilities(al_mac);

	if (t_stored == NULL) {
		PLATFORM_PRINTF_WARNING("%s - Channel Scan Capabilities for mac %02x%02x%02x%02x%02x%02x not found!\n",
		                        __FUNCTION__, al_mac[0], al_mac[1], al_mac[2], al_mac[3], al_mac[4], al_mac[5]);
		return 1;
	}

	INT8U i, j;
	if (0 == t_stored->radio_nr) {
		PLATFORM_PRINTF_WARNING("Empty Channel Scan Capabilites stored for this device...\n");
		return 1;
	}

	t->target_radio_nr = t_stored->radio_nr;
	t->target_radios   = (struct ChannelScanTargetRadio *)PLATFORM_MALLOC(t->target_radio_nr * sizeof(struct ChannelScanTargetRadio));
	for (i = 0; i < t_stored->radio_nr; i++) {
		PLATFORM_MEMCPY(t->target_radios[i].radio_unique_identifier, t_stored->radios[i].radio_unique_identifier, 6);
		t->target_radios[i].target_op_class_nr = t_stored->radios[i].op_class_nr;
		t->target_radios[i].target_op_classes  = (struct ChannelScanTargetOpClass *)PLATFORM_MALLOC(t->target_radios[i].target_op_class_nr * sizeof(struct ChannelScanTargetOpClass));
		for (j = 0; j < t->target_radios[i].target_op_class_nr; j++) {
			t->target_radios[i].target_op_classes[j].op_class            = t_stored->radios[i].op_classes[j].op_class;
			t->target_radios[i].target_op_classes[j].target_channel_nr   = t_stored->radios[i].op_classes[j].channel_nr;
			t->target_radios[i].target_op_classes[j].target_channel_list = (INT8U *)PLATFORM_MALLOC(t->target_radios[i].target_op_classes[j].target_channel_nr * sizeof(INT8U));
			PLATFORM_MEMCPY(t->target_radios[i].target_op_classes[j].target_channel_list, t_stored->radios[i].op_classes[j].channel_list, t->target_radios[i].target_op_classes[j].target_channel_nr);
		}
	}

	return 0;
}

INT8U sendChannelScanRequestPacket(char *interface_name, INT16U mid, struct ChannelScanRequestTLV *channel_scan_request_tlv, INT8U *destination_al_mac_address)
{
	INT8U ret;
	INT8U free_tlv = 0;
	// struct ChannelScanRequestTLV channel_scan_request_tlv;
	if (channel_scan_request_tlv == NULL) {
		free_tlv                           = 1;
		channel_scan_request_tlv           = (struct ChannelScanRequestTLV *)PLATFORM_MALLOC(sizeof(struct ChannelScanRequestTLV));
		channel_scan_request_tlv->tlv_type = TLV_TYPE_CHANNEL_SCAN_REQUEST;
		channel_scan_request_tlv->flags    = 0x80; //set Perform Fresh Scan to one
		if (_construct_channel_scan_request(channel_scan_request_tlv, destination_al_mac_address)) {
			PLATFORM_PRINTF_WARNING("Failed to retrieve channel scan request from received info for device :%02x:%02x:%02x:%02x:%02x:%02x...\n", destination_al_mac_address[0], destination_al_mac_address[1], destination_al_mac_address[2], destination_al_mac_address[3], destination_al_mac_address[4], destination_al_mac_address[5]);
			PLATFORM_FREE(channel_scan_request_tlv);
			return 0;
		}
	}
	struct CMDU message;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_CHANNEL_SCAN_REQUEST (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", destination_al_mac_address[0], destination_al_mac_address[1], destination_al_mac_address[2], destination_al_mac_address[3], destination_al_mac_address[4], destination_al_mac_address[5]);

	// Build the CMDU
	//
	message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	message.message_type    = CMDU_TYPE_CHANNEL_SCAN_REQUEST;
	message.message_id      = mid;
	message.relay_indicator = 0;
	message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);
	message.list_of_TLVs[0] = (INT8U *)channel_scan_request_tlv;
	message.list_of_TLVs[1] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &message, true, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	//Set fresh scan flag for differentiating expected type of ack
	if (PERFORM_FRESH_SCAN_FALSE == (channel_scan_request_tlv->flags & PERFORM_FRESH_SCAN_MASK)) {
		DMsetRequestStoredResultsFlag(1);
	}

	// Free memory
	//
	//free channel_scan_request_tlv
	if (free_tlv) {
		INT8U k, l;
		for (k = 0; k < channel_scan_request_tlv->target_radio_nr; k++) {
			for (l = 0; l < channel_scan_request_tlv->target_radios[k].target_op_class_nr; l++) {
				if (channel_scan_request_tlv->target_radios[k].target_op_classes[l].target_channel_list)
					PLATFORM_FREE(channel_scan_request_tlv->target_radios[k].target_op_classes[l].target_channel_list);
			}
			if (channel_scan_request_tlv->target_radios[k].target_op_classes)
				PLATFORM_FREE(channel_scan_request_tlv->target_radios[k].target_op_classes);
		}
		if (channel_scan_request_tlv->target_radios)
			PLATFORM_FREE(channel_scan_request_tlv->target_radios);
		PLATFORM_FREE(channel_scan_request_tlv);
	}
	PLATFORM_FREE(message.list_of_TLVs);

	return ret;
}

INT8U _obtainChannelScanResultTLVs(INT8U *channel_scan_result_nr, struct ChannelScanResultTLV **channel_scan_results)
{
	INT8U                        buffered_channel_scan_result_nr = 0;
	struct ChannelScanResultTLV *buffered_channel_scan_results   = NULL;
	INT8U *                      reported_buffered_scan;
	INT8U                        i = 0, j = 0;

	if (DMgetChannelScanResult(&buffered_channel_scan_result_nr, &buffered_channel_scan_results)) {
		PLATFORM_PRINTF_ERROR("Empty ChannelScanResults in al_datamodel! Aborting ChannelScanReport sending...\n");
		return 1; //error
	}

	struct ChannelScanRequestTLV *req = DMgetBufferedChannelScanRequest();

	if (NULL == req) {
		PLATFORM_PRINTF_ERROR("Empty Buffered ChannelScanRequest in al_datamodel! Aborting ChannelScanReport sending...\n");
		return 1;
	}

	reported_buffered_scan = PLATFORM_MALLOC(sizeof(INT8U) * buffered_channel_scan_result_nr);
	PLATFORM_MEMSET(reported_buffered_scan, 0, buffered_channel_scan_result_nr);

	for (j = 0; j < req->target_radio_nr; j++) {
		for (i = 0; i < buffered_channel_scan_result_nr; i++) {
			if (1 == reported_buffered_scan[i]) {
				continue;
			}
			if (0 == PLATFORM_MEMCMP((buffered_channel_scan_results)[i].radio_unique_identifier, req->target_radios[j].radio_unique_identifier, 6)) {
				reported_buffered_scan[i] = 1;
				*channel_scan_results     = (struct ChannelScanResultTLV *)PLATFORM_REALLOC(*channel_scan_results, ((*channel_scan_result_nr) + 1) * sizeof(struct ChannelScanResultTLV));

				(*channel_scan_results)[*channel_scan_result_nr].tlv_type = buffered_channel_scan_results[i].tlv_type;
				PLATFORM_MEMCPY((*channel_scan_results)[*channel_scan_result_nr].radio_unique_identifier, buffered_channel_scan_results[i].radio_unique_identifier, 6);
				(*channel_scan_results)[*channel_scan_result_nr].op_class                = buffered_channel_scan_results[i].op_class;
				(*channel_scan_results)[*channel_scan_result_nr].channel                 = buffered_channel_scan_results[i].channel;
				(*channel_scan_results)[*channel_scan_result_nr].scan_status             = buffered_channel_scan_results[i].scan_status;
				(*channel_scan_results)[*channel_scan_result_nr].timestamp_length        = buffered_channel_scan_results[i].timestamp_length;
				(*channel_scan_results)[*channel_scan_result_nr].timestamp               = buffered_channel_scan_results[i].timestamp;
				(*channel_scan_results)[*channel_scan_result_nr].channel_utilization     = buffered_channel_scan_results[i].channel_utilization;
				(*channel_scan_results)[*channel_scan_result_nr].noise                   = buffered_channel_scan_results[i].noise;
				(*channel_scan_results)[*channel_scan_result_nr].neighbor_nr             = buffered_channel_scan_results[i].neighbor_nr;
				(*channel_scan_results)[*channel_scan_result_nr].neighbors               = buffered_channel_scan_results[i].neighbors;
				(*channel_scan_results)[*channel_scan_result_nr].aggregate_scan_duration = buffered_channel_scan_results[i].aggregate_scan_duration;
				(*channel_scan_results)[*channel_scan_result_nr].scan_type_flags         = buffered_channel_scan_results[i].scan_type_flags;

				(*channel_scan_result_nr)++;
			}
		}
	}

	PLATFORM_FREE(reported_buffered_scan);

	return 0;
}

void _obtainTimestampTLV(struct TimestampTLV *timestamp_tlv)
{
	timestamp_tlv->tlv_type         = TLV_TYPE_TIMESTAMP;
	timestamp_tlv->timestamp_length = 31; //in our implementation this is always 31
	timestamp_tlv->timestamp        = generate_timestamp();
}

INT8U sendChannelScanReportPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, bool need_ack)
{

	INT8U ret;

	struct CMDU message;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_CHANNEL_SCAN_REPORT (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", destination_al_mac_address[0], destination_al_mac_address[1], destination_al_mac_address[2], destination_al_mac_address[3], destination_al_mac_address[4], destination_al_mac_address[5]);

	// Build the CMDU
	//
	INT8U                        channel_scan_result_nr = 0, i;
	struct ChannelScanResultTLV *channel_scan_results   = NULL;

	//Get Channel Scan Result TLV
	if (_obtainChannelScanResultTLVs(&channel_scan_result_nr, &channel_scan_results)) {
		return 0; //error
	}

	message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	message.message_type    = CMDU_TYPE_CHANNEL_SCAN_REPORT;
	message.message_id      = mid;
	message.relay_indicator = 0;
	message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * (channel_scan_result_nr + 1 + 1)); //1 timestamp tlv + 1 null termination tlv

	//Get Timestamp TLV
	struct TimestampTLV timestamp_tlv;
	_obtainTimestampTLV(&timestamp_tlv);
	message.list_of_TLVs[0] = (INT8U *)&timestamp_tlv;

	for (i = 0; i < channel_scan_result_nr; i++) {
		message.list_of_TLVs[i + 1] = (INT8U *)&channel_scan_results[i];
	}

	//Get Timestamp TLV
	// struct TimestampTLV timestamp_tlv;
	// _obtainTimestampTLV(&timestamp_tlv);
	// message.list_of_TLVs[channel_scan_result_nr] = (INT8U *)&timestamp_tlv;

	message.list_of_TLVs[channel_scan_result_nr + 1] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &message, need_ack, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	PLATFORM_FREE(timestamp_tlv.timestamp);
	PLATFORM_FREE(message.list_of_TLVs);
	return ret;
}

INT8U sendMultiAPAssociationStatusNotificationMessage(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, bool need_ack, INT8U bssid_nr, struct AssociationStatusNotificationBssInfo *associationStatusNotificationBssInfo)
{
	INT8U ret;

	struct CMDU message;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_ASSOCIATION_STATUS_NOTIFICATION MESSAGE (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", destination_al_mac_address[0], destination_al_mac_address[1], destination_al_mac_address[2], destination_al_mac_address[3], destination_al_mac_address[4], destination_al_mac_address[5]);

	// Build the CMDU
	//
	INT8U                                   tlv_count;
	struct AssociationStatusNotificationTLV associationStatusNotificationTlv;

	PLATFORM_MEMSET(&associationStatusNotificationTlv, 0, sizeof(struct AssociationStatusNotificationTLV));
	associationStatusNotificationTlv.bss_info = NULL;

	associationStatusNotificationTlv.tlv_type = TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION;
	associationStatusNotificationTlv.bssid_nr = bssid_nr;
	if (associationStatusNotificationTlv.bssid_nr) {
		associationStatusNotificationTlv.bss_info = associationStatusNotificationBssInfo;
	}

	tlv_count = (1 + 1); //1 association status notification tlv + 1 null termination tlv

	message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	message.message_type    = CMDU_TYPE_ASSOCIATION_STATUS_NOTIFICATION;
	message.message_id      = mid;
	message.relay_indicator = 0;
	message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * tlv_count);

	message.list_of_TLVs[0] = (INT8U *)&associationStatusNotificationTlv;
	message.list_of_TLVs[1] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &message, need_ack, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	PLATFORM_FREE(message.list_of_TLVs);
	return ret;
}

INT8U sendMultiAPTunneledMessage(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, bool need_ack, INT8U *mac_address, INT8U payload_type, INT8U tunneled_tlv_nr, struct TunneledTLV *tunneledTlv)
{
	INT8U ret;

	struct CMDU message;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_TUNNELED MESSAGE (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", destination_al_mac_address[0], destination_al_mac_address[1], destination_al_mac_address[2], destination_al_mac_address[3], destination_al_mac_address[4], destination_al_mac_address[5]);

	// Build the CMDU
	//
	INT8U                         tlv_count;
	struct SourceInfoTLV          sourceInfoTlv;
	struct TunneledMessageTypeTLV tunneledMessageTypeTlv;

	PLATFORM_MEMSET(&sourceInfoTlv, 0, sizeof(struct SourceInfoTLV));
	PLATFORM_MEMSET(&tunneledMessageTypeTlv, 0, sizeof(struct TunneledMessageTypeTLV));

	sourceInfoTlv.tlv_type = TLV_TYPE_SOURCE_INFO;
	PLATFORM_MEMCPY(sourceInfoTlv.mac_address, mac_address, 6);

	tunneledMessageTypeTlv.tlv_type                       = TLV_TYPE_TUNNELED_MESSAGE_TYPE;
	tunneledMessageTypeTlv.tunneled_protocol_type_payload = payload_type;

	tlv_count = (1 + 1 + tunneled_tlv_nr + 1); //1 source info tlv + tunneled message type tlv + 1 null termination tlv

	message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	message.message_type    = CMDU_TYPE_TUNNELED;
	message.message_id      = mid;
	message.relay_indicator = 0;
	message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * tlv_count);

	message.list_of_TLVs[0] = (INT8U *)&sourceInfoTlv;
	message.list_of_TLVs[1] = (INT8U *)&tunneledMessageTypeTlv;
	message.list_of_TLVs[2] = (INT8U *)tunneledTlv;
	message.list_of_TLVs[3] = NULL;
	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &message, need_ack, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}
	// Free memory
	//
	PLATFORM_FREE(message.list_of_TLVs);
	return ret;
}

INT8U sendFailedConnectionMessage(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, bool need_ack, INT8U *sta_mac, INT16U status_code, INT16U reason_code)
{
	INT8U ret;

	struct CMDU message;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_FAILED_CONNECTION (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", destination_al_mac_address[0], destination_al_mac_address[1], destination_al_mac_address[2], destination_al_mac_address[3], destination_al_mac_address[4], destination_al_mac_address[5]);

	// Build the CMDU
	//
	INT8U                       tlv_count;
	struct STAMacAddressTypeTLV sta_mac_address_type_tlv;
	struct StatusCodeTLV        status_code_tlv;
	struct ReasonCodeTLV        reason_code_tlv;

	PLATFORM_MEMSET(&sta_mac_address_type_tlv, 0, sizeof(struct STAMacAddressTypeTLV));
	PLATFORM_MEMSET(&status_code_tlv, 0, sizeof(struct StatusCodeTLV));
	PLATFORM_MEMSET(&reason_code_tlv, 0, sizeof(struct ReasonCodeTLV));

	sta_mac_address_type_tlv.tlv_type = TLV_TYPE_STA_MAC_ADDRESS_TYPE;
	PLATFORM_MEMCPY(sta_mac_address_type_tlv.mac_address, sta_mac, 6);

	status_code_tlv.tlv_type    = TLV_TYPE_STATUS_CODE;
	status_code_tlv.status_code = status_code;

	reason_code_tlv.tlv_type    = TLV_TYPE_REASON_CODE;
	reason_code_tlv.reason_code = reason_code;

	tlv_count = 3; // sta_mac_address_type_tlv + status_code_tlv

	// 0 is reserved value
	if (reason_code) {
		tlv_count += 1;
	}

	message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	message.message_type    = CMDU_TYPE_FAILED_CONNECTION;
	message.message_id      = mid;
	message.relay_indicator = 0;
	message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * tlv_count);

	message.list_of_TLVs[0]             = (INT8U *)&sta_mac_address_type_tlv;
	message.list_of_TLVs[1]             = (INT8U *)&status_code_tlv;
	message.list_of_TLVs[2]             = (INT8U *)&reason_code_tlv;
	message.list_of_TLVs[tlv_count - 1] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &message, need_ack, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}
	// Free memory
	//
	PLATFORM_FREE(message.list_of_TLVs);
	return ret;
}

INT8U sendClientDisassociationStats(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, bool need_ack, INT8U *sta_mac, INT16U reason_code, struct AssociatedSTATrafficStatsTLV *traffic_stats_tlv)
{
	INT8U ret;

	struct CMDU message;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_CLIENT_DISASSOCIATION_STATS (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", destination_al_mac_address[0], destination_al_mac_address[1], destination_al_mac_address[2], destination_al_mac_address[3], destination_al_mac_address[4], destination_al_mac_address[5]);

	// Build the CMDU
	//
	INT8U                       tlv_count;
	struct STAMacAddressTypeTLV sta_mac_address_type_tlv;
	struct ReasonCodeTLV        reason_code_tlv;

	PLATFORM_MEMSET(&sta_mac_address_type_tlv, 0, sizeof(struct STAMacAddressTypeTLV));
	PLATFORM_MEMSET(&reason_code_tlv, 0, sizeof(struct ReasonCodeTLV));

	sta_mac_address_type_tlv.tlv_type = TLV_TYPE_STA_MAC_ADDRESS_TYPE;
	PLATFORM_MEMCPY(sta_mac_address_type_tlv.mac_address, sta_mac, 6);

	reason_code_tlv.tlv_type    = TLV_TYPE_REASON_CODE;
	reason_code_tlv.reason_code = reason_code;

	tlv_count = 4; // sta_mac_address_type_tlv + reason_code_tlv + traffic_stats_tlv + end of message

	message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	message.message_type    = CMDU_TYPE_CLIENT_DISASSOCIATION_STATS;
	message.message_id      = mid;
	message.relay_indicator = 0;
	message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * tlv_count);

	message.list_of_TLVs[0] = (INT8U *)&sta_mac_address_type_tlv;
	message.list_of_TLVs[1] = (INT8U *)&reason_code_tlv;
	message.list_of_TLVs[2] = (INT8U *)traffic_stats_tlv;
	message.list_of_TLVs[3] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &message, need_ack, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}
	// Free memory
	//
	PLATFORM_FREE(message.list_of_TLVs);
	return ret;
}

INT8U sendErrorResponsePacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, INT8U p2_error_code_tlv_nr, struct Profile2ErrorCodeTLV *p2_error_code_tlvs)
{
	// The "Error Response" message is a CMDU with one or more TLVs:
	// - one or more Profile-2 Error Code TLVs (see section 17.2.51).

	struct CMDU message;
	INT8U       ret, i;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_ERROR_RESPONSE (%s) MID: %04x\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", dest_mac_address[0], dest_mac_address[1], dest_mac_address[2], dest_mac_address[3], dest_mac_address[4], dest_mac_address[5]);

	// Build the CMDU
	INT8U tlv_count;

	tlv_count = (p2_error_code_tlv_nr + 1); // profile-2 error code tlvs + 1 null termination tlv

	message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	message.message_type    = CMDU_TYPE_ERROR_RESPONSE;
	message.message_id      = mid;
	message.relay_indicator = 0;
	message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * tlv_count);

	for (i = 0; i < p2_error_code_tlv_nr; i++) {
		message.list_of_TLVs[i] = (INT8U *)&p2_error_code_tlvs[i];
	}
	message.list_of_TLVs[tlv_count - 1] = NULL;

	// Send the packet
	if (0 == send1905RawPacket(interface_name, mid, dest_mac_address, &message, 0, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	PLATFORM_FREE(message.list_of_TLVs);

	return ret;
}
#endif

#ifdef RTK_MULTI_AP_R3
//-------------------------------------------------------------------//
//                        Multi-AP Profile 3                         //
//-------------------------------------------------------------------//
INT8U send1905DirectEncapDPPPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, INT8U *dpp_frame, INT16U dpp_frame_len)
{
	INT8U                 ret;
	struct CMDU           encap_dpp_msg;
	struct DPPMessageTLV *dpp_msg_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_DIRECT_ENCAP_DPP (%s) MID: (%04x)\n", interface_name, mid);

	// Build the CMDU
	//
	encap_dpp_msg.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	encap_dpp_msg.message_type    = CMDU_TYPE_DIRECT_ENCAP_DPP;
	encap_dpp_msg.message_id      = mid;
	encap_dpp_msg.relay_indicator = 0;
	encap_dpp_msg.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);

	dpp_msg_tlv = (struct DPPMessageTLV *)PLATFORM_MALLOC(sizeof(struct DPPMessageTLV));
	_obtainDPPMessageTLV(dpp_msg_tlv, dpp_frame + 1, dpp_frame_len - 1); // omit first byte (0x04 Action Category Field)

	encap_dpp_msg.list_of_TLVs[0] = (INT8U *)dpp_msg_tlv;
	encap_dpp_msg.list_of_TLVs[1] = NULL;

	if (0 == send1905RawPacket(interface_name, mid, dest_mac_address, &encap_dpp_msg, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	free_1905_TLV_structure((INT8U *)dpp_msg_tlv);
	PLATFORM_FREE(encap_dpp_msg.list_of_TLVs);
	return ret;
}


// all tlvs should be built already by now before calling this function.
INT8U send1905ProxiedEncapDPPPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, struct Encap1905DPPTLV *encap_1905_dpp_tlv, struct DPPChirpValueTLV *chirp_value_tlv)
{
	INT8U       ret;
	struct CMDU proxied_encap_dpp_msg;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_PROXIED_ENCAP_DPP (%s) MID: (%04x)\n", interface_name, mid);

	// Build the CMDU
	INT8U tlv_count = 2; // encap DPP tlv and the NULL-terminiating one.

	if (!encap_1905_dpp_tlv) {
		PLATFORM_PRINTF_ERROR("ENCAP_1905_DPP_TLV not found, failed to send 1905 proxied encap DPP packet! \n");
		return 0;
	}

	if (chirp_value_tlv) {
		tlv_count++;
	}

	proxied_encap_dpp_msg.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	proxied_encap_dpp_msg.message_type    = CMDU_TYPE_PROXIED_ENCAP_DPP;
	proxied_encap_dpp_msg.message_id      = mid;
	proxied_encap_dpp_msg.relay_indicator = 0;
	proxied_encap_dpp_msg.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * tlv_count);

	proxied_encap_dpp_msg.list_of_TLVs[0] = (INT8U *)encap_1905_dpp_tlv;
	if (chirp_value_tlv) {
		proxied_encap_dpp_msg.list_of_TLVs[1] = (INT8U *)chirp_value_tlv;
	}
	proxied_encap_dpp_msg.list_of_TLVs[tlv_count-1] = NULL;

	if (0 == send1905RawPacket(interface_name, mid, dest_mac_address, &proxied_encap_dpp_msg, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	PLATFORM_FREE(proxied_encap_dpp_msg.list_of_TLVs);
	return ret;
}

INT8U send1905EncapEAPOLMessage(char *interface_name, INT16U mid, INT8U *dest_mac_address, INT8U *eapol_frame, INT16U eapol_frame_len)
{
	INT8U                     ret;
	struct CMDU               encap_eapol_msg;
	struct Encap1905EAPOLTLV *eapol_msg_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_1905_ENCAP_EAPOL (%s) MID: (%04x)\n", interface_name, mid);

	// Build the CMDU
	//
	encap_eapol_msg.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	encap_eapol_msg.message_type    = CMDU_TYPE_1905_ENCAP_EAPOL;
	encap_eapol_msg.message_id      = mid;
	encap_eapol_msg.relay_indicator = 0;
	encap_eapol_msg.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);

	eapol_msg_tlv = (struct Encap1905EAPOLTLV *)PLATFORM_MALLOC(sizeof(struct Encap1905EAPOLTLV));
	_obtain1905EncapEAPOLTLV(eapol_msg_tlv, eapol_frame, eapol_frame_len);

	encap_eapol_msg.list_of_TLVs[0] = (INT8U *)eapol_msg_tlv;
	encap_eapol_msg.list_of_TLVs[1] = NULL;

	if (0 == send1905RawPacket(interface_name, mid, dest_mac_address, &encap_eapol_msg, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send CMDU_TYPE_1905_ENCAP_EAPOL packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	free_1905_TLV_structure((INT8U *)eapol_msg_tlv);
	PLATFORM_FREE(encap_eapol_msg.list_of_TLVs);
	return ret;
}

INT8U send1905DecryptionFailureMessage(char *interface_name, INT16U mid, INT8U *dest_mac_addr)
{
	INT8U                       ret;
	struct CMDU                 decryption_failure_msg;
	struct alMacAddressTypeTLV *al_mac_addr_tlv;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_1905_DECRYPTION_FAILURE (%s) MID: (%04x)\n", interface_name, mid);

	// Build the CMDU
	//
	decryption_failure_msg.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	decryption_failure_msg.message_type    = CMDU_TYPE_1905_DECRYPTION_FAILURE;
	decryption_failure_msg.message_id      = mid;
	decryption_failure_msg.relay_indicator = 0;
	decryption_failure_msg.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);

	al_mac_addr_tlv = (struct alMacAddressTypeTLV *)PLATFORM_MALLOC(sizeof(struct alMacAddressTypeTLV));
	_obtainAlMacAddressTLV(al_mac_addr_tlv, dest_mac_addr);

	decryption_failure_msg.list_of_TLVs[0] = (INT8U *)al_mac_addr_tlv;
	decryption_failure_msg.list_of_TLVs[1] = NULL;

	if (0 == send1905RawPacket(interface_name, mid, dest_mac_addr, &decryption_failure_msg, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send CMDU_TYPE_1905_DECRYPTION_FAILURE packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	PLATFORM_FREE(decryption_failure_msg.list_of_TLVs);
	return ret;
}

INT8U sendBSSConfigRequestPacket(char *interface_name, INT16U mid, INT8U *dest_mac_addr)
{
	// The "BSS Configuration Request" message is a CMDU with these TLVs:
	//   - One Multi-AP Profile TLV
	//   - One Supported Service TLV
	//   - One Profile 2 AP Capability TLV
	//   - One (or more) AP Radio Basic Capabilities TLV
	//   - One (or more) AP Radio Advanced Capabilities TLV
	//   - One AKM Suite Capabilities TLV
	//   - One BSS Configuration Request TLV
	//   - Zero (or more) Backhaul STA Capabilities TLV  

	INT8U ret, i = 0, j = 0;

	struct CMDU request_msg;

	struct MultiAPProfileTLV              profile_tlv;
	struct SupportedServiceTLV            supported_service_tlv;
	struct Profile2APCapabilityTLV        r2_ap_capability_tlv;
	struct APRadioBasicCapabilitiesTLV    radio_basic_tlv[3];
	struct APRadioAdvancedCapabilitiesTLV advanced_capabilities_tlv[3];
	struct AKMSuiteCapabilitiesTLV *      akm_suite_tlv;
	struct BSSConfigRequestTLV *          bss_config_req_tlv;
	struct BackhaulSTARadioCapabilitiesTLV * backhaul_sta_radio_cap_tlv = NULL;

	PLATFORM_MEMSET(radio_basic_tlv, 0 , sizeof(radio_basic_tlv));
	PLATFORM_MEMSET(advanced_capabilities_tlv, 0 , sizeof(advanced_capabilities_tlv));

	INT8U tlv_count = 10; // All TLVs above (2 radio_basic_tlv, 2 advanced_capabilities_tlv) + NULL
	INT8U backhaul_sta_radio_cap_tlv_nr = 0;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_BSS_CONFIG_REQUEST (%s) MID: (%04x)\n", interface_name, mid);

	// Allocate TLVs using dynamic allocation
	//
	akm_suite_tlv      = (struct AKMSuiteCapabilitiesTLV *)PLATFORM_MALLOC(sizeof(struct AKMSuiteCapabilitiesTLV));
	bss_config_req_tlv = (struct BSSConfigRequestTLV *)PLATFORM_MALLOC(sizeof(struct BSSConfigRequestTLV));

	// Obtain TLVs information
	//
	_obtainProfileTLV(&profile_tlv);
	_obtainSupportedServiceTLV(&supported_service_tlv);
	_obtainProfile2APCapabilityTLV(&r2_ap_capability_tlv);
	_obtainAPRadioBasicCapabilitiesTLV(&radio_basic_tlv[0], map_radio_name_24g);
	_obtainAPRadioBasicCapabilitiesTLV(&radio_basic_tlv[1], map_radio_name_5gl);
	if (DMisTribandDevice()) {
		_obtainAPRadioBasicCapabilitiesTLV(&radio_basic_tlv[2], map_radio_name_5gh);
		tlv_count++;
	}
	_obtainRadioAdvancedCapabilitiesTLV(&advanced_capabilities_tlv[0], map_radio_name_24g);
	_obtainRadioAdvancedCapabilitiesTLV(&advanced_capabilities_tlv[1], map_radio_name_5gl);
	if (DMisTribandDevice()) {
		_obtainRadioAdvancedCapabilitiesTLV(&advanced_capabilities_tlv[2], map_radio_name_5gh);
		tlv_count++;
	}
	_obtainAKMSuiteCapabilitiesTLV(akm_suite_tlv);
	_obtainBSSConfigRequestTLV(bss_config_req_tlv);
	_obtainBackhaulSTARadioCapabilitiesTLV(&backhaul_sta_radio_cap_tlv, &backhaul_sta_radio_cap_tlv_nr);
	tlv_count += backhaul_sta_radio_cap_tlv_nr;

	// Build the CMDU
	//
	request_msg.message_version   = CMDU_MESSAGE_VERSION_1905_1_2013;
	request_msg.message_type      = CMDU_TYPE_BSS_CONFIG_REQUEST;
	request_msg.message_id        = mid;
	request_msg.relay_indicator   = 0;
	request_msg.list_of_TLVs      = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * (tlv_count + 1));
	request_msg.list_of_TLVs[i++] = (INT8U *)&profile_tlv;
	request_msg.list_of_TLVs[i++] = (INT8U *)&supported_service_tlv;
	request_msg.list_of_TLVs[i++] = (INT8U *)&r2_ap_capability_tlv;
	request_msg.list_of_TLVs[i++] = (INT8U *)&radio_basic_tlv[0];
	request_msg.list_of_TLVs[i++] = (INT8U *)&radio_basic_tlv[1];
	if (DMisTribandDevice())
		request_msg.list_of_TLVs[i++] = (INT8U *)&radio_basic_tlv[2];
	request_msg.list_of_TLVs[i++] = (INT8U *)&advanced_capabilities_tlv[0];
	request_msg.list_of_TLVs[i++] = (INT8U *)&advanced_capabilities_tlv[1];
	if (DMisTribandDevice())
		request_msg.list_of_TLVs[i++] = (INT8U *)&advanced_capabilities_tlv[2];
	request_msg.list_of_TLVs[i++] = (INT8U *)akm_suite_tlv;
	request_msg.list_of_TLVs[i++] = (INT8U *)bss_config_req_tlv;
	for (j=0; j<backhaul_sta_radio_cap_tlv_nr;j++) {
		request_msg.list_of_TLVs[i++] = (INT8U *)&backhaul_sta_radio_cap_tlv[j];
	}
	request_msg.list_of_TLVs[i++] = NULL;

	if (0 == send1905RawPacket(interface_name, mid, dest_mac_addr, &request_msg, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	for (i = 0; i < (sizeof(radio_basic_tlv)/sizeof(struct APRadioBasicCapabilitiesTLV)); i++) {
		_freeAPRadioBasicCapabilitiesTLV(&radio_basic_tlv[i]);
	}

	_freeSupportedServiceTLV(&supported_service_tlv);
	free_1905_TLV_structure((INT8U *)akm_suite_tlv);
	free_1905_TLV_structure((INT8U *)bss_config_req_tlv);
	PLATFORM_FREE(backhaul_sta_radio_cap_tlv);
	PLATFORM_FREE(request_msg.list_of_TLVs);
	return ret;
}

INT8U convert_map_config_to_dpp_config_obj(struct dpp_authentication *dpp_auth, struct configData *from, struct dpp_config_obj *to)
{
	INT8U copy_psk_hex = 0, copy_pass = 0;
	enum dpp_netrole net_role;

	// Configuration Object
	switch (from->network_type) {
	case MULTI_AP_FRONTHAUL_BSS_BIT:
		PLATFORM_STRCPY(to->wifi_tech, "inframap");
		break;
	case MULTI_AP_BACKHAUL_BSS_BIT:
		PLATFORM_STRCPY(to->wifi_tech, "map");
		break;
	case MULTI_AP_TEARDOWN_BIT:
		PLATFORM_STRCPY(to->wifi_tech, "inframap");
		return 0;
	default:
		PLATFORM_PRINTF_ERROR("unknown configuration type!\n");
		return 1;
	}

	// Discovery Object
	strlcpy(to->ssid, from->ssid, sizeof(to->ssid));
	to->ssid_len = PLATFORM_STRLEN(to->ssid);

	// Credential Object
	switch (from->auth_type) {
	case WPS_AUTH_WPAPSK:
	case WPS_AUTH_SHARED:
	case WPS_AUTH_WPA:
	case WPS_AUTH_WPA2:
	case WPS_AUTH_WPA2PSK: {
		PLATFORM_STRCPY(to->akm, "psk");
		copy_psk_hex = 1;
		break;
	}
	case WPS_AUTH_WPA3: {
		PLATFORM_STRCPY(to->akm, "sae");
		copy_pass = 1;
		break;
	}
	case WPS_AUTH_WPA2PSK | WPS_AUTH_WPA3: {
		PLATFORM_STRCPY(to->akm, "psk+sae");
		copy_psk_hex = 1;
		copy_pass    = 1;
		break;
	}
	case WPS_AUTH_DPP | WPS_AUTH_WPA2PSK | WPS_AUTH_WPA3: {
		PLATFORM_STRCPY(to->akm, "dpp+psk+sae");
		copy_psk_hex = 1;
		copy_pass    = 1;
		break;
	}
	default:
		PLATFORM_PRINTF_ERROR("unknown encryption type!\n");
		return 1;
	}

	if (copy_pass)
		strlcpy(to->pass, from->network_key, sizeof(to->pass));

	if (copy_psk_hex) {
		// check size, because one char takes 2 bytes in converted hex string
		if ((PLATFORM_STRLEN(from->network_key) * 2 + 1) > sizeof(to->psk_hex)) {
			PLATFORM_PRINTF_ERROR("network key is too long for dpp_config_obj!\n");
			return 1;
		}
		bin2hexstr(from->network_key, to->psk_hex, PLATFORM_STRLEN(from->network_key));
	}

	if (from->network_type == MULTI_AP_FRONTHAUL_BSS_BIT) {
		net_role = DPP_NETROLE_AP;
	} else if (from->network_type == MULTI_AP_BACKHAUL_BSS_BIT) {
		net_role = DPP_NETROLE_MAP_BACKHAUL_BSS;
	} else {
		PLATFORM_PRINTF_ERROR("cannot assign netrole for dpp_conf_obj!\n");
		return 1;
	}

	// Use Configurator to Generate DPP connectors
	// R3 spec did not mention about the source of dpp connector's private key.
	// Therefore, we assume it to be using agent's peer protocol key,
	// just like the net access provisioning key mentioned in DPP R2 6.1
	if (dpp_build_signed_connector(dpp_auth->conf,
	                               &dpp_auth->peer_protocol_key,
	                               "mapNW", net_role, &to->conn)
	    != 1) {
		PLATFORM_PRINTF_ERROR("cannot build signed connector for dpp_conf_obj!\n");
		return 1;
	}

	return 0;
}

INT8U sendBSSConfigResponsePacket(char *interface_name, INT16U mid, INT8U *src_addr, INT8U *dest_mac_addr,
                                  struct APRadioBasicCapabilitiesTLV *ap_radio_tlvs[], INT8U ap_radio_tlvs_nr)
{
	/* Given AP Radios, generate BSS Config Response CMDU accordingly */
	struct CMDU resp_msg;

	struct BSSConfigResponseTLV *      bss_config_resp_tlv = NULL;
	struct Default8021QSettingsTLV     default_8021q_tlv;
	struct TrafficSeparationPolicyTLV *traffic_sep_policy_tlv = NULL;

	INT8U include_default_8021q = 0;

	struct dpp_config_obj *dpp_conf_objs = (struct dpp_config_obj *)PLATFORM_MALLOC(sizeof(struct dpp_config_obj) * MAX_SEND_BSS_CONF_OBJ_NR);
	if (!dpp_conf_objs) {
		PLATFORM_PRINTF_ERROR("Fail to allocate buffer for dpp_config_obj, sendBssConfigResponsePacket failed. \n");
		return 0;
	}
	INT8U                 dpp_conf_objs_nr = 0;

	struct dpp_authentication *dpp_auth = NULL;

	INT8U i, j, tlv_count = 0, ret = 0;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_BSS_CONFIG_RESPONSE (%s) MID: (%04x)\n", interface_name, mid);

	// Initialization
	//
	PLATFORM_MEMSET(&resp_msg, 0, sizeof(struct CMDU));

	bss_config_resp_tlv = (struct BSSConfigResponseTLV *)PLATFORM_MALLOC(sizeof(struct BSSConfigResponseTLV));
	PLATFORM_MEMSET(bss_config_resp_tlv, 0, sizeof(struct BSSConfigResponseTLV));

	PLATFORM_MEMSET(&default_8021q_tlv, 0, sizeof(default_8021q_tlv));
	default_8021q_tlv.tlv_type = TLV_TYPE_DEFAULT_802_1Q_SETTINGS;

	PLATFORM_MEMSET(dpp_conf_objs, 0, sizeof(struct dpp_config_obj) * MAX_SEND_BSS_CONF_OBJ_NR);

	dpp_auth = DMgetDPPAuthenticationObject();

	// Parse AP radios to generate dpp config objects and traffic sep policies
	//
	for (i = 0; i < ap_radio_tlvs_nr; i++) {
		struct APRadioBasicCapabilitiesTLV *tlv = ap_radio_tlvs[i];

		struct configData *configs[MAX_SEND_BSS_CONFIG_NR];
		INT8U              configs_nr = 0;

		// Get config profiles of the specific radio band
		INT8U band = get_band_from_op_classes(tlv->operating_classes, tlv->operating_classes_nr);
		getConfigsForAutoconfig(band, src_addr, configs, &configs_nr, MAX_SEND_BSS_CONFIG_NR);
		if (configs_nr > tlv->max_BSSs_nr) {
			PLATFORM_PRINTF_WARNING("Number of BSS to configure exceeded max_bss of this radio!\n");
			configs_nr = tlv->max_BSSs_nr;
		}

		// Convert profles to dpp config objects
		for (j = 0; j < configs_nr; j++) {
			if (dpp_conf_objs_nr < MAX_SEND_BSS_CONF_OBJ_NR) {
				if (!convert_map_config_to_dpp_config_obj(dpp_auth, configs[j], &dpp_conf_objs[dpp_conf_objs_nr])) {
					bin2hexstr((char *)tlv->radio_unique_id, dpp_conf_objs[dpp_conf_objs_nr].ruid, 6);
					dpp_conf_objs[dpp_conf_objs_nr].ruid_enable = 1;
					dpp_conf_objs_nr++;
				}
			} else {
				PLATFORM_PRINTF_WARNING("Only support sending %d config objs\n", MAX_SEND_BSS_CONF_OBJ_NR);
				break;
			}
		}

		// Append Traffic Separation Policy TLV
		obtainTrafficSeparationPolicyTLV(&traffic_sep_policy_tlv, configs, configs_nr);
	}

	// Obtain TLV information
	//
	_obtainBSSConfigResponseTLV(bss_config_resp_tlv, dpp_conf_objs, dpp_conf_objs_nr);
	tlv_count++;

	DMgetDefault8021QSettings(&default_8021q_tlv.primary_vlan_id, &default_8021q_tlv.default_pcp);
	if (default_8021q_tlv.primary_vlan_id || default_8021q_tlv.default_pcp) {
		include_default_8021q = 1;
		tlv_count++;
	}

	if (traffic_sep_policy_tlv)
		tlv_count++;

	// Build the CMDU
	//
	i                          = 0;
	resp_msg.message_version   = CMDU_MESSAGE_VERSION_1905_1_2013;
	resp_msg.message_type      = CMDU_TYPE_BSS_CONFIG_RESPONSE;
	resp_msg.message_id        = mid;
	resp_msg.relay_indicator   = 0;
	resp_msg.list_of_TLVs      = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * (tlv_count + 1));
	resp_msg.list_of_TLVs[i++] = (INT8U *)bss_config_resp_tlv;
	if (include_default_8021q)
		resp_msg.list_of_TLVs[i++] = (INT8U *)&default_8021q_tlv;
	if (traffic_sep_policy_tlv)
		resp_msg.list_of_TLVs[i++] = (INT8U *)traffic_sep_policy_tlv;
	resp_msg.list_of_TLVs[i++] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, dest_mac_addr, &resp_msg, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	free_1905_TLV_structure((INT8U *)bss_config_resp_tlv);
	free_1905_TLV_structure((INT8U *)traffic_sep_policy_tlv);
	PLATFORM_FREE(resp_msg.list_of_TLVs);
	PLATFORM_FREE(dpp_conf_objs);
	return ret;
}

INT8U sendBSSConfigResultPacket(char *interface_name, INT16U mid, INT8U *dest_mac_addr)
{
	INT8U       ret = 0;
	struct CMDU result_msg;

	struct BSSConfigReportTLV *bss_conf_rep_tlv;

	// Obtain TLV information
	bss_conf_rep_tlv = (struct BSSConfigReportTLV *)PLATFORM_MALLOC(sizeof(struct BSSConfigReportTLV));
	_obtainBSSConfigReportTLV(bss_conf_rep_tlv);

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_BSS_CONFIG_RESULT  (%s) MID: (%04x)\n", interface_name, mid);

	// Build the CMDU
	//
	result_msg.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	result_msg.message_type    = CMDU_TYPE_BSS_CONFIG_RESULT;
	result_msg.message_id      = mid;
	result_msg.relay_indicator = 0;
	result_msg.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 2);
	result_msg.list_of_TLVs[0] = (INT8U *)bss_conf_rep_tlv;
	result_msg.list_of_TLVs[1] = NULL;

	// Send the packet
	//
	if (0 == send1905RawPacket(interface_name, mid, dest_mac_addr, &result_msg, false, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	// Free memory
	//
	free_1905_TLV_structure((INT8U *)bss_conf_rep_tlv);
	PLATFORM_FREE(result_msg.list_of_TLVs);
	return ret;
}

INT8U sendServicePrioritizationRequestMessage(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, bool need_ack, struct ServicePrioritizationRuleTLV *service_prioritization_rule_tlv, struct DSCPMappingTableTLV *dscp_mapping_table_tlv)
{
	INT8U ret;
	struct CMDU message;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_SERVICE_PRIORITIZATION_REQUEST (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", destination_al_mac_address[0], destination_al_mac_address[1], destination_al_mac_address[2], destination_al_mac_address[3], destination_al_mac_address[4], destination_al_mac_address[5]);

	// Build the CMDU
	INT8U tlv_count                = 1; // NULL-terminating TLV
	INT8U dscp_mapping_table_exist = 0;

	if (service_prioritization_rule_tlv) {
		tlv_count++;
	}

	if (dscp_mapping_table_tlv) {
		tlv_count++;
		dscp_mapping_table_exist = 1;
	}

	message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	message.message_type    = CMDU_TYPE_SERVICE_PRIORITIZATION_REQUEST;
	message.message_id      = mid;
	message.relay_indicator = 0;
	message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * tlv_count);

	message.list_of_TLVs[0] = (INT8U *)service_prioritization_rule_tlv;
	if (dscp_mapping_table_exist) {
		message.list_of_TLVs[1] = (INT8U *)dscp_mapping_table_tlv;
	}
	message.list_of_TLVs[tlv_count - 1] = NULL;

	// Send the packet
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &message, need_ack, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	PLATFORM_FREE(message.list_of_TLVs);

	return ret;
}

// If Controller wants to reconfigure bhBSS or fhBSS of agent(s)
INT8U sendReconfigurationTriggerMessage(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, bool need_ack)
{
	struct CMDU message;
	INT8U       ret;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_RECONFIG_TRIGGER (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_INFO("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", destination_al_mac_address[0], destination_al_mac_address[1], destination_al_mac_address[2], destination_al_mac_address[3], destination_al_mac_address[4], destination_al_mac_address[5]);

	// No TLVs are required in this message
	// Build CMDU
	message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	message.message_type    = CMDU_TYPE_RECONFIG_TRIGGER;
	message.message_id      = mid;
	message.relay_indicator = 0;
	message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *));
	message.list_of_TLVs[0] = NULL;

	// Send the packet
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &message, need_ack, 0)) {
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	PLATFORM_FREE(message.list_of_TLVs);

	return ret;
}

/* Upon receiving DPP URI (notification), this should be called to send CCE indication to all agents, and ourself through lo */
INT8U sendDPPCCEIndicationMessage(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U is_advertise_cce)
{
	INT8U ret;
	struct CMDU message;

	struct DPPCCEIndicationTLV *dpp_cce_indication_tlv = (struct DPPCCEIndicationTLV *)PLATFORM_MALLOC(sizeof(struct DPPCCEIndicationTLV));
	dpp_cce_indication_tlv->tlv_type                  = TLV_TYPE_DPP_CCE_INDICATION;
	dpp_cce_indication_tlv->advertise_cce             = is_advertise_cce;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_DPP_CCE_INDICATION (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", destination_al_mac_address[0], destination_al_mac_address[1], destination_al_mac_address[2], destination_al_mac_address[3], destination_al_mac_address[4], destination_al_mac_address[5]);

	// Build the CMDU
	INT8U tlv_count = 2; // NULL-terminating TLV and compulsory DPPCCEIndicationTLV

	message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	message.message_type    = CMDU_TYPE_DPP_CCE_INDICATION;
	message.message_id      = mid;
	message.relay_indicator = 0;
	message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * tlv_count);

	message.list_of_TLVs[0] = (INT8U *)dpp_cce_indication_tlv;
	message.list_of_TLVs[tlv_count - 1] = NULL;

	// Send the packet
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &message, false, 0)) { // ack not needed
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	free_1905_TLV_structure((INT8U *)dpp_cce_indication_tlv);
	PLATFORM_FREE(message.list_of_TLVs);

	return ret;
}

void triggerCCEAdvertisement(INT8U is_advertise_cce)
{
	INT8U  i;
	INT8U  mcast_address[] = MCAST_1905;
	INT16U mid             = getNextMid();

	INT8U  ifs_nr    = 0;
	char **ifs_names = get_backhaul_interfaces(&ifs_nr);

	if (is_advertise_cce)
		PLATFORM_PRINTF_INFO("[DPP] Controller instructs all agents to START CCE advertisement\n");
	else
		PLATFORM_PRINTF_INFO("[DPP] Controller instructs all agents to STOP CCE advertisement\n");

	PLATFORM_PRINTF_INFO("[DPP] Send CCE Indication message to all interfaces.\n");

	for (i = 0; i < ifs_nr; i++) {
		if (PLATFORM_IS_INTERFACE_UP(ifs_names[i])) {
			sendDPPCCEIndicationMessage(ifs_names[i], mid, mcast_address, is_advertise_cce);
		}
	}

	PLATFORM_FREE(ifs_names);

	if (!DMgetWFATestMode()) {
		sendDPPCCEIndicationMessage("lo", mid, DMcontrollerMacGet(), is_advertise_cce);
	}
}

// page 35 paragraph 3 of R3 spec.
// _obtainDPPChirpValueTLV is called to obtain DPPChirpValueTLV struct, then pass it into this function to send it from proxy agent to controller
INT8U sendChirpNotificationMessage(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, struct DPPChirpValueTLV *chirp_value_tlv)
{
	INT8U  ret;
	struct CMDU message;

	PLATFORM_PRINTF_INFO("--> CMDU_TYPE_CHIRP_NOTIFICATION_MESSAGE (%s) MID: (%04x)\n", interface_name, mid);
	PLATFORM_PRINTF_DETAIL("Sending to %02x:%02x:%02x:%02x:%02x:%02x\n", destination_al_mac_address[0], destination_al_mac_address[1], destination_al_mac_address[2], destination_al_mac_address[3], destination_al_mac_address[4], destination_al_mac_address[5]);

	// Build the CMDU
	INT8U tlv_count = 2; // In r3 only 1 tlv will be included.

	message.message_version = CMDU_MESSAGE_VERSION_1905_1_2013;
	message.message_type    = CMDU_TYPE_CHIRP_NOTIFICATION;
	message.message_id      = mid;
	message.relay_indicator = 0;
	message.list_of_TLVs    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * tlv_count);

	message.list_of_TLVs[0] = (INT8U *)chirp_value_tlv;
	message.list_of_TLVs[tlv_count - 1] = NULL;

	// Send the packet
	if (0 == send1905RawPacket(interface_name, mid, destination_al_mac_address, &message, false, 0)) { // ack not needed
		PLATFORM_PRINTF_WARNING("Could not send packet\n");
		ret = 0;
	} else {
		ret = 1;
	}

	PLATFORM_FREE(message.list_of_TLVs);

	return ret;
}

#endif