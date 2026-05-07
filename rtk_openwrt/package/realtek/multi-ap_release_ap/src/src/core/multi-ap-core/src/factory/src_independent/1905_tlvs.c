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

#include "1905_tlvs.h"
#include "packet_tools.h"
#include "multi_ap_tlvs.h"
#include "multi_ap_tlvs_r2.h"
#include "multi_ap_tlvs_r3.h"
#include "multi_ap_tlvs_r4.h"

////////////////////////////////////////////////////////////////////////////////
// Actual API functions
////////////////////////////////////////////////////////////////////////////////

INT8U *parse_1905_TLV_from_packet(INT8U *packet_stream)
{
	if (NULL == packet_stream) {
		return NULL;
	}

	// The first byte of the stream is the "Type" field from the TLV structure.
	// Valid values for this byte are the following ones...
	//
	switch (*packet_stream) {
	case TLV_TYPE_END_OF_MESSAGE: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.1"

		struct endOfMessageTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct endOfMessageTLV *)PLATFORM_MALLOC(sizeof(struct endOfMessageTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		// According to the standard, the length *must* be 0
		//
		if (0 != len) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		ret->tlv_type = TLV_TYPE_END_OF_MESSAGE;

		return (INT8U *)ret;
	}

	case TLV_TYPE_VENDOR_SPECIFIC: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.2"

		struct vendorSpecificTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct vendorSpecificTLV *)PLATFORM_MALLOC(sizeof(struct vendorSpecificTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		// According to the standard, the length *must* be at least "3"
		//
		if (len < 3) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		ret->tlv_type = TLV_TYPE_VENDOR_SPECIFIC;

		_E1B(&p, &ret->vendorOUI[0]);
		_E1B(&p, &ret->vendorOUI[1]);
		_E1B(&p, &ret->vendorOUI[2]);

		ret->m_nr = len - 3;

		if (ret->m_nr) {
			ret->m = (INT8U *)PLATFORM_MALLOC(ret->m_nr);

			_EnB(&p, ret->m, ret->m_nr);
		} else {
			ret->m = NULL;
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_AL_MAC_ADDRESS_TYPE: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.3"

		struct alMacAddressTypeTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct alMacAddressTypeTLV *)PLATFORM_MALLOC(sizeof(struct alMacAddressTypeTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		// According to the standard, the length *must* be 6
		//
		if (6 != len) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		ret->tlv_type = TLV_TYPE_AL_MAC_ADDRESS_TYPE;

		_EnB(&p, ret->al_mac_address, 6);

		return (INT8U *)ret;
	}

	case TLV_TYPE_MAC_ADDRESS_TYPE: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.4"

		struct macAddressTypeTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct macAddressTypeTLV *)PLATFORM_MALLOC(sizeof(struct macAddressTypeTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		// According to the standard, the length *must* be 6
		//
		if (6 != len) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		ret->tlv_type = TLV_TYPE_MAC_ADDRESS_TYPE;

		_EnB(&p, ret->mac_address, 6);

		return (INT8U *)ret;
	}

	case TLV_TYPE_DEVICE_INFORMATION_TYPE: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.5"

		struct deviceInformationTypeTLV *ret;

		INT8U *p;
		INT16U len;
		INT8U  i;

		ret = (struct deviceInformationTypeTLV *)PLATFORM_MALLOC(sizeof(struct deviceInformationTypeTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_DEVICE_INFORMATION_TYPE;

		_EnB(&p, ret->al_mac_address, 6);
		_E1B(&p, &ret->local_interfaces_nr);

		ret->local_interfaces = (struct _localInterfaceEntries *)PLATFORM_MALLOC(sizeof(struct _localInterfaceEntries) * ret->local_interfaces_nr);
		PLATFORM_MEMSET(ret->local_interfaces, 0, sizeof(struct _localInterfaceEntries) * ret->local_interfaces_nr);

		for (i = 0; i < ret->local_interfaces_nr; i++) {
			_EnB(&p, ret->local_interfaces[i].mac_address, 6);
			_E2B(&p, &ret->local_interfaces[i].media_type);
			_E1B(&p, &ret->local_interfaces[i].media_specific_data_size);

			if (IS_IEEE_802_11_MEDIA(ret->local_interfaces[i].media_type)) {
				INT8U aux;

				// Multi-AP Specification v3.0 Chapter 6, Table 5. Extension of 1905 Media type for 802.11AX
				// When media_type == 0x0108, media_specific_data_size must == 0 and no media specific data
				// We are compatible for some agent with media_specific_data_size == 10
				if (IS_IEEE_802_11AX_MEDIA(ret->local_interfaces[i].media_type)
				    && 0 == ret->local_interfaces[i].media_specific_data_size)
					continue;

				if (0 == ret->local_interfaces[i].media_specific_data_size) {
					// allow 0 to pass due to Fiberhome customer issues.
					continue;
				}

				if (10 != ret->local_interfaces[i].media_specific_data_size) {
					// Malformed packet
					//
					PLATFORM_FREE(ret->local_interfaces);
					PLATFORM_FREE(ret);
					return NULL;
				}

				_EnB(&p, ret->local_interfaces[i].media_specific_data.ieee80211.network_membership, 6);
				_E1B(&p, &aux);
				ret->local_interfaces[i].media_specific_data.ieee80211.role = aux >> 4;
				_E1B(&p, &ret->local_interfaces[i].media_specific_data.ieee80211.ap_channel_band);
				_E1B(&p, &ret->local_interfaces[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_1);
				_E1B(&p, &ret->local_interfaces[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_2);

			} else if (IS_IEEE_1901_MEDIA(ret->local_interfaces[i].media_type)) {
				if (7 != ret->local_interfaces[i].media_specific_data_size) {
					// Malformed packet
					//
					PLATFORM_FREE(ret->local_interfaces);
					PLATFORM_FREE(ret);
					return NULL;
				}
				_EnB(&p, ret->local_interfaces[i].media_specific_data.ieee1901.network_identifier, 7);
			} else {
				if (10 == ret->local_interfaces[i].media_specific_data_size) {
					// allow 0 to pass due to NEC customer issues.
					ret->local_interfaces[i].media_specific_data_size = 0;
					p = p + 10;
					continue;
				}
				if (0 != ret->local_interfaces[i].media_specific_data_size) {
					// Malformed packet
					//
					PLATFORM_FREE(ret->local_interfaces);
					PLATFORM_FREE(ret);
					return NULL;
				}
			}
		}

		if (p - (packet_stream + 3) != len) {
			// Malformed packet
			//
			PLATFORM_FREE(ret->local_interfaces);
			PLATFORM_FREE(ret);
			return NULL;
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.6"

		struct deviceBridgingCapabilityTLV *ret;

		INT8U *p;
		INT16U len;
		INT8U  i, j;

		ret = (struct deviceBridgingCapabilityTLV *)PLATFORM_ZALLOC(sizeof(struct deviceBridgingCapabilityTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES;

		if (0 == len) {
#ifdef FIX_BROKEN_TLVS
			// Malformed packet. Even if there are NO bridging tuples, the
			// length should be "1" (which is the length of the next field,
			// that would containing a "zero", indicating the number of
			// bridging tuples).
			// *However*, because at least one other implementation sets
			// the 'length' to zero to indicate "no bridging tuples", we
			// will also accept this type of "malformed" packet.
			//
			ret->bridging_tuples_nr = 0;
			return (INT8U *)ret;
#else
			PLATFORM_FREE(ret);
			return NULL;
#endif
		}

		_E1B(&p, &ret->bridging_tuples_nr);

		if (ret->bridging_tuples_nr > 0) {
			ret->bridging_tuples = (struct _bridgingTupleEntries *)PLATFORM_ZALLOC(sizeof(struct _bridgingTupleEntries) * ret->bridging_tuples_nr);

			for (i = 0; i < ret->bridging_tuples_nr; i++) {
				_E1B(&p, &ret->bridging_tuples[i].bridging_tuple_macs_nr);

				if (ret->bridging_tuples[i].bridging_tuple_macs_nr > 0) {
					ret->bridging_tuples[i].bridging_tuple_macs = (struct _bridgingTupleMacEntries *)PLATFORM_ZALLOC(sizeof(struct _bridgingTupleMacEntries) * ret->bridging_tuples[i].bridging_tuple_macs_nr);

					for (j = 0; j < ret->bridging_tuples[i].bridging_tuple_macs_nr; j++) {
						_EnB(&p, ret->bridging_tuples[i].bridging_tuple_macs[j].mac_address, 6);
					}
				}
			}
		}

		if (p - (packet_stream + 3) != len) {
			// Malformed packet
			//
			for (i = 0; i < ret->bridging_tuples_nr; i++) {
				PLATFORM_FREE(ret->bridging_tuples[i].bridging_tuple_macs);
			}
			PLATFORM_FREE(ret->bridging_tuples);
			PLATFORM_FREE(ret);
			return NULL;
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.8"

		struct non1905NeighborDeviceListTLV *ret;

		INT8U *p;
		INT16U len;
		INT8U  i;

		ret = (struct non1905NeighborDeviceListTLV *)PLATFORM_MALLOC(sizeof(struct non1905NeighborDeviceListTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		// According to the standard, the length *must* be "6 + 6*n"
		//
		if (0 != ((len - 6) % 6)) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}
		ret->tlv_type = TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST;

		_EnB(&p, ret->local_mac_address, 6);

		ret->non_1905_neighbors_nr = (len - 6) / 6;

		ret->non_1905_neighbors = (struct _non1905neighborEntries *)PLATFORM_MALLOC(sizeof(struct _non1905neighborEntries) * ret->non_1905_neighbors_nr);

		for (i = 0; i < ret->non_1905_neighbors_nr; i++) {
			_EnB(&p, ret->non_1905_neighbors[i].mac_address, 6);
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_NEIGHBOR_DEVICE_LIST: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.9"

		struct neighborDeviceListTLV *ret;

		INT8U *p;
		INT16U len;
		INT8U  i;

		ret = (struct neighborDeviceListTLV *)PLATFORM_MALLOC(sizeof(struct neighborDeviceListTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		// According to the standard, the length *must* be "6 + 7*n"
		// "6+1"
		//
		if (0 != ((len - 6) % 7)) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}
		ret->tlv_type = TLV_TYPE_NEIGHBOR_DEVICE_LIST;

		_EnB(&p, ret->local_mac_address, 6);

		ret->neighbors_nr = (len - 6) / 7;

		ret->neighbors = (struct _neighborEntries *)PLATFORM_MALLOC(sizeof(struct _neighborEntries) * ret->neighbors_nr);

		for (i = 0; i < ret->neighbors_nr; i++) {
			INT8U aux;

			_EnB(&p, ret->neighbors[i].mac_address, 6);
			_E1B(&p, &aux);

			if (aux & 0x80) {
				ret->neighbors[i].bridge_flag = 1;
			} else {
				ret->neighbors[i].bridge_flag = 0;
			}
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_LINK_METRIC_QUERY: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.10"

		struct linkMetricQueryTLV *ret;

		INT8U *p;
		INT16U len;

		INT8U destination;
		INT8U link_metrics_type;

		ret = (struct linkMetricQueryTLV *)PLATFORM_MALLOC(sizeof(struct linkMetricQueryTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		// According to the standard, the length *must* be 8
		// But.... Keith: Multi AP spec test plan requires length 2 to be accepted
		//
		/*
		if (8 != len) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}
*/
		ret->tlv_type = TLV_TYPE_LINK_METRIC_QUERY;

		_E1B(&p, &destination);

		if (destination >= 2) {
			// Reserved (invalid) value received
			//
			PLATFORM_FREE(ret);
			return NULL;
		} else if (0 == destination && len == 2) {
			INT8U dummy_address[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

			ret->destination = LINK_METRIC_QUERY_TLV_ALL_NEIGHBORS;
			PLATFORM_MEMCPY(ret->specific_neighbor, dummy_address, 6);
		} else if (1 == destination && len == 8) {
			_EnB(&p, ret->specific_neighbor, 6);
			ret->destination = LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR;
		} else {
			// This code cannot be reached
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		_E1B(&p, &link_metrics_type);

		if (link_metrics_type >= 3) {
			// Reserved (invalid) value received
			//
			PLATFORM_FREE(ret);
			return NULL;
		} else if (0 == link_metrics_type) {
			ret->link_metrics_type = LINK_METRIC_QUERY_TLV_TX_LINK_METRICS_ONLY;
		} else if (1 == link_metrics_type) {
			ret->link_metrics_type = LINK_METRIC_QUERY_TLV_RX_LINK_METRICS_ONLY;
		} else if (2 == link_metrics_type) {
			ret->link_metrics_type = LINK_METRIC_QUERY_TLV_BOTH_TX_AND_RX_LINK_METRICS;
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_TRANSMITTER_LINK_METRIC: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.11"

		struct transmitterLinkMetricTLV *ret;

		INT8U *p;
		INT16U len;
		INT8U  i;

		ret = (struct transmitterLinkMetricTLV *)PLATFORM_MALLOC(sizeof(struct transmitterLinkMetricTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		// According to the standard, the length *must* be "12+29*n" where
		// "n" is "1" or greater or "0"
		//
		if ((12 + 29 * 1) > len && 12 != len) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}
		if (0 != (len - 12) % 29) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		ret->tlv_type = TLV_TYPE_TRANSMITTER_LINK_METRIC;

		_EnB(&p, ret->local_al_address, 6);
		_EnB(&p, ret->neighbor_al_address, 6);

		ret->transmitter_link_metrics_nr = (len - 12) / 29;

		ret->transmitter_link_metrics = (struct _transmitterLinkMetricEntries *)PLATFORM_MALLOC(sizeof(struct _transmitterLinkMetricEntries) * ret->transmitter_link_metrics_nr);

		for (i = 0; i < ret->transmitter_link_metrics_nr; i++) {
			_EnB(&p, ret->transmitter_link_metrics[i].local_interface_address, 6);
			_EnB(&p, ret->transmitter_link_metrics[i].neighbor_interface_address, 6);

			_E2B(&p, &ret->transmitter_link_metrics[i].intf_type);
			_E1B(&p, &ret->transmitter_link_metrics[i].bridge_flag);
			_E4B(&p, &ret->transmitter_link_metrics[i].packet_errors);
			_E4B(&p, &ret->transmitter_link_metrics[i].transmitted_packets);
			_E2B(&p, &ret->transmitter_link_metrics[i].mac_throughput_capacity);
			_E2B(&p, &ret->transmitter_link_metrics[i].link_availability);
			_E2B(&p, &ret->transmitter_link_metrics[i].phy_rate);
		}

		if (p - (packet_stream + 3) != len) {
			// Malformed packet
			//
			PLATFORM_FREE(ret->transmitter_link_metrics);
			PLATFORM_FREE(ret);
			return NULL;
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_RECEIVER_LINK_METRIC: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.12"

		struct receiverLinkMetricTLV *ret;

		INT8U *p;
		INT16U len;
		INT8U  i;

		ret = (struct receiverLinkMetricTLV *)PLATFORM_MALLOC(sizeof(struct receiverLinkMetricTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		// According to the standard, the length *must* be "12+23*n" where
		// "n" is "1" or greater or "0"
		//
		if ((12 + 23 * 1) > len && 12 != len) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}
		if (0 != (len - 12) % 23) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		ret->tlv_type = TLV_TYPE_RECEIVER_LINK_METRIC;

		_EnB(&p, ret->local_al_address, 6);
		_EnB(&p, ret->neighbor_al_address, 6);

		ret->receiver_link_metrics_nr = (len - 12) / 23;

		ret->receiver_link_metrics = (struct _receiverLinkMetricEntries *)PLATFORM_MALLOC(sizeof(struct _receiverLinkMetricEntries) * ret->receiver_link_metrics_nr);

		for (i = 0; i < ret->receiver_link_metrics_nr; i++) {
			_EnB(&p, ret->receiver_link_metrics[i].local_interface_address, 6);
			_EnB(&p, ret->receiver_link_metrics[i].neighbor_interface_address, 6);

			_E2B(&p, &ret->receiver_link_metrics[i].intf_type);
			_E4B(&p, &ret->receiver_link_metrics[i].packet_errors);
			_E4B(&p, &ret->receiver_link_metrics[i].packets_received);
			_E1B(&p, &ret->receiver_link_metrics[i].rssi);
		}

		if (p - (packet_stream + 3) != len) {
			// Malformed packet
			//
			PLATFORM_FREE(ret->receiver_link_metrics);
			PLATFORM_FREE(ret);
			return NULL;
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_LINK_METRIC_RESULT_CODE: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.13"

		struct linkMetricResultCodeTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct linkMetricResultCodeTLV *)PLATFORM_MALLOC(sizeof(struct linkMetricResultCodeTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		// According to the standard, the length *must* be 1
		//
		if (1 != len) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		ret->tlv_type = TLV_TYPE_LINK_METRIC_RESULT_CODE;

		_E1B(&p, &ret->result_code);

		return (INT8U *)ret;
	}

	case TLV_TYPE_SEARCHED_ROLE: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.14"

		struct searchedRoleTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct searchedRoleTLV *)PLATFORM_MALLOC(sizeof(struct searchedRoleTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		// According to the standard, the length *must* be 1
		//
		if (1 != len) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		ret->tlv_type = TLV_TYPE_SEARCHED_ROLE;

		_E1B(&p, &ret->role);

		return (INT8U *)ret;
	}

	case TLV_TYPE_AUTOCONFIG_FREQ_BAND: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.15"

		struct autoconfigFreqBandTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct autoconfigFreqBandTLV *)PLATFORM_MALLOC(sizeof(struct autoconfigFreqBandTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		// According to the standard, the length *must* be 1
		//
		if (1 != len) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		ret->tlv_type = TLV_TYPE_AUTOCONFIG_FREQ_BAND;

		_E1B(&p, &ret->freq_band);

		return (INT8U *)ret;
	}

	case TLV_TYPE_SUPPORTED_ROLE: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.16"

		struct supportedRoleTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct supportedRoleTLV *)PLATFORM_MALLOC(sizeof(struct supportedRoleTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		// According to the standard, the length *must* be 1
		//
		if (1 != len) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		ret->tlv_type = TLV_TYPE_SUPPORTED_ROLE;

		_E1B(&p, &ret->role);

		return (INT8U *)ret;
	}

	case TLV_TYPE_SUPPORTED_FREQ_BAND: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.17"

		struct supportedFreqBandTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct supportedFreqBandTLV *)PLATFORM_MALLOC(sizeof(struct supportedFreqBandTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		// According to the standard, the length *must* be 1
		//
		if (1 != len) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		ret->tlv_type = TLV_TYPE_SUPPORTED_FREQ_BAND;

		_E1B(&p, &ret->freq_band);

		return (INT8U *)ret;
	}

	case TLV_TYPE_WSC: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.18"

		struct wscTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct wscTLV *)PLATFORM_MALLOC(sizeof(struct wscTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type       = TLV_TYPE_WSC;
		ret->wsc_frame_size = len;

		if (len > 0) {
			ret->wsc_frame = (INT8U *)PLATFORM_MALLOC(len);
			_EnB(&p, ret->wsc_frame, len);
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.19"

		struct pushButtonEventNotificationTLV *ret;

		INT8U *p;
		INT16U len;
		INT8U  i;

		ret = (struct pushButtonEventNotificationTLV *)PLATFORM_MALLOC(sizeof(struct pushButtonEventNotificationTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION;

		if (0 == len) {
#ifdef FIX_BROKEN_TLVS
			// Malformed packet. Even if there are NO bridging tuples, the
			// Malformed packet. Even if there are NO media types, the
			// length should be "1" (which is the length of the next field,
			// that would containing a "zero", indicating the number of
			// media types).
			// *However*, because at least one other implementation sets
			// the 'length' to zero to indicate "no media types", we will
			// also accept this type of "malformed" packet.
			//
			ret->media_types_nr = 0;
			return (INT8U *)ret;
#else
			PLATFORM_FREE(ret);
			return NULL;
#endif
		}

		_E1B(&p, &ret->media_types_nr);

		ret->media_types = (struct _mediaTypeEntries *)PLATFORM_MALLOC(sizeof(struct _mediaTypeEntries) * ret->media_types_nr);

		for (i = 0; i < ret->media_types_nr; i++) {
			_E2B(&p, &ret->media_types[i].media_type);
			_E1B(&p, &ret->media_types[i].media_specific_data_size);

			if (IS_IEEE_802_11_MEDIA(ret->media_types[i].media_type)) {
				INT8U aux;

				// Multi-AP Specification v3.0 Chapter 6, Table 5. Extension of 1905 Media type for 802.11AX
				// When media_type == 0x0108, media_specific_data_size must == 0 and no media specific data
				// We are compatible for some agent with media_specific_data_size == 10
				if (IS_IEEE_802_11AX_MEDIA(ret->media_types[i].media_type)
				    && 0 == ret->media_types[i].media_specific_data_size)
					continue;

				if (10 != ret->media_types[i].media_specific_data_size) {
					// Malformed packet
					//
					PLATFORM_FREE(ret->media_types);
					PLATFORM_FREE(ret);
					return NULL;
				}

				_EnB(&p, ret->media_types[i].media_specific_data.ieee80211.network_membership, 6);
				_E1B(&p, &aux);
				ret->media_types[i].media_specific_data.ieee80211.role = aux >> 4;
				_E1B(&p, &ret->media_types[i].media_specific_data.ieee80211.ap_channel_band);
				_E1B(&p, &ret->media_types[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_1);
				_E1B(&p, &ret->media_types[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_2);

			} else if (IS_IEEE_1901_MEDIA(ret->media_types[i].media_type)) {
				if (7 != ret->media_types[i].media_specific_data_size) {
					// Malformed packet
					//
					PLATFORM_FREE(ret->media_types);
					PLATFORM_FREE(ret);
					return NULL;
				}
				_EnB(&p, ret->media_types[i].media_specific_data.ieee1901.network_identifier, 7);
			} else {
				if (0 != ret->media_types[i].media_specific_data_size) {
					// Malformed packet
					//
					PLATFORM_FREE(ret->media_types);
					PLATFORM_FREE(ret);
					return NULL;
				}
			}
		}

		if (p - (packet_stream + 3) != len) {
			// Malformed packet
			//
			PLATFORM_FREE(ret->media_types);
			PLATFORM_FREE(ret);
			return NULL;
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION: {
		// This parsing is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.20"

		struct pushButtonJoinNotificationTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct pushButtonJoinNotificationTLV *)PLATFORM_MALLOC(sizeof(struct pushButtonJoinNotificationTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		// According to the standard, the length *must* be 20
		//
		if (20 != len) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		ret->tlv_type = TLV_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION;

		_EnB(&p, ret->al_mac_address, 6);
		_E2B(&p, &ret->message_identifier);
		_EnB(&p, ret->mac_address, 6);
		_EnB(&p, ret->new_mac_address, 6);

		return (INT8U *)ret;
	}

	case TLV_TYPE_SUPPORTED_SERVICE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.1"
		struct SupportedServiceTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct SupportedServiceTLV *)PLATFORM_MALLOC(sizeof(struct SupportedServiceTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_SUPPORTED_SERVICE;

		_E1B(&p, &ret->supported_service_nr);

		if (ret->supported_service_nr) {
			ret->supported_service = (INT8U *)PLATFORM_MALLOC(ret->supported_service_nr);
			_EnB(&p, ret->supported_service, ret->supported_service_nr);
		} else {
			ret->supported_service = NULL;
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_SEARCHED_SERVICE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.2"
		struct SearchedServiceTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct SearchedServiceTLV *)PLATFORM_MALLOC(sizeof(struct SearchedServiceTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_SEARCHED_SERVICE;

		_E1B(&p, &ret->searched_service_nr);

		if (ret->searched_service_nr) {
			ret->searched_service = (INT8U *)PLATFORM_MALLOC(ret->searched_service_nr);
			_EnB(&p, ret->searched_service, ret->searched_service_nr);
		} else {
			ret->searched_service = NULL;
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_AP_RADIO_IDENTIFIER: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.3"
		struct ApRadioIdentifierTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct ApRadioIdentifierTLV *)PLATFORM_MALLOC(sizeof(struct ApRadioIdentifierTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_AP_RADIO_IDENTIFIER;

		_EnB(&p, ret->radio_unique_id, 6);

		return (INT8U *)ret;
	}

	case TLV_TYPE_AP_OPERATIONAL_BSS: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.4"
		struct ApOperationalBSSTLV *ret;

		INT8U *p;
		INT16U len;
		INT8U  k, m;

		ret = (struct ApOperationalBSSTLV *)PLATFORM_MALLOC(sizeof(struct ApOperationalBSSTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_AP_OPERATIONAL_BSS;

		_E1B(&p, &ret->radios_nr);

		if (ret->radios_nr) {
			ret->radios = (struct RADIO *)PLATFORM_MALLOC(ret->radios_nr * sizeof(struct RADIO));

			for (k = 0; k < ret->radios_nr; k++) {
				_EnB(&p, ret->radios[k].radio_unique_id, 6);
				_E1B(&p, &ret->radios[k].BSSs_nr);

				if (ret->radios[k].BSSs_nr) {
					ret->radios[k].BSSs = (struct BSS *)PLATFORM_MALLOC(ret->radios[k].BSSs_nr * sizeof(struct BSS));

					for (m = 0; m < ret->radios[k].BSSs_nr; m++) {
						_EnB(&p, ret->radios[k].BSSs[m].mac_addr, 6);
						_E1B(&p, &ret->radios[k].BSSs[m].ssid_len);

						if (ret->radios[k].BSSs[m].ssid_len) {
							ret->radios[k].BSSs[m].ssid = (char *)PLATFORM_MALLOC(ret->radios[k].BSSs[m].ssid_len);
							_EnB(&p, ret->radios[k].BSSs[m].ssid, ret->radios[k].BSSs[m].ssid_len);
						}
					}
				}
			}
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_ASSOCIATED_CLIENTS: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.5"
		struct AssociatedClientsTLV *ret;

		INT8U *p;
		INT16U len;
		INT8U  k, m;

		ret = (struct AssociatedClientsTLV *)PLATFORM_MALLOC(sizeof(struct AssociatedClientsTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_ASSOCIATED_CLIENTS;

		_E1B(&p, &ret->BSSs_nr);

		if (ret->BSSs_nr) {
			ret->BSSs = (struct BSSWithClients *)PLATFORM_MALLOC(ret->BSSs_nr * sizeof(struct BSSWithClients));

			for (k = 0; k < ret->BSSs_nr; k++) {
				_EnB(&p, ret->BSSs[k].bssid, 6);
				_E2B(&p, &ret->BSSs[k].clients_nr);

				if (ret->BSSs[k].clients_nr) {
					ret->BSSs[k].clients = (struct Client *)PLATFORM_MALLOC(ret->BSSs[k].clients_nr * sizeof(struct Client));

					for (m = 0; m < ret->BSSs[k].clients_nr; m++) {
						_EnB(&p, ret->BSSs[k].clients[m].mac_addr, 6);
						_E2B(&p, &ret->BSSs[k].clients[m].left_time);
					}
				}
			}
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_AP_CAPABILITY: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.6"
		struct APCapabilityTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct APCapabilityTLV *)PLATFORM_MALLOC(sizeof(struct APCapabilityTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_AP_CAPABILITY;
		_E1B(&p, &ret->ap_capability);
		return (INT8U *)ret;
	}

	case TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.7"
		struct APRadioBasicCapabilitiesTLV *ret;

		INT8U *p;
		INT16U len;
		INT8U  k;

		ret = (struct APRadioBasicCapabilitiesTLV *)PLATFORM_MALLOC(sizeof(struct APRadioBasicCapabilitiesTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES;
		_EnB(&p, ret->radio_unique_id, 6);
		_E1B(&p, &ret->max_BSSs_nr);

		if (ret->max_BSSs_nr <= 0) {
			PLATFORM_PRINTF_ERROR(
			    "The Maximum number of BSSs supported by this radio(%02x:%02x:%02x:%02x:%02x:%02x) is %d ",
			    ret->radio_unique_id[0], ret->radio_unique_id[1], ret->radio_unique_id[2],
			    ret->radio_unique_id[3], ret->radio_unique_id[4], ret->radio_unique_id[5], ret->max_BSSs_nr);
			PLATFORM_FREE(ret);
			return NULL;
		}

		_E1B(&p, &ret->operating_classes_nr);

		if (ret->operating_classes_nr) {
			ret->operating_classes = (struct OperatingClass *)PLATFORM_MALLOC(ret->operating_classes_nr * sizeof(struct OperatingClass));

			for (k = 0; k < ret->operating_classes_nr; k++) {
				_E1B(&p, &ret->operating_classes[k].operating_class);
				_E1B(&p, &ret->operating_classes[k].max_transmit_power);
				_E1B(&p, &ret->operating_classes[k].non_operable_channels_nr);

				if (ret->operating_classes[k].non_operable_channels_nr) {
					ret->operating_classes[k].non_operable_channels = (INT8U *)PLATFORM_MALLOC(ret->operating_classes[k].non_operable_channels_nr);
					_EnB(&p, ret->operating_classes[k].non_operable_channels,
					     ret->operating_classes[k].non_operable_channels_nr);
				}
			}
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_AP_HT_CAPABILITIES: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.8"
		struct APHTCapabilitiesTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct APHTCapabilitiesTLV *)PLATFORM_MALLOC(sizeof(struct APHTCapabilitiesTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_AP_HT_CAPABILITIES;
		_EnB(&p, ret->radio_unique_id, 6);
		_E1B(&p, &ret->ht_capability);
		return (INT8U *)ret;
	}

	case TLV_TYPE_AP_VHT_CAPABILITIES: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.9"
		struct APVHTCapabilitiesTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct APVHTCapabilitiesTLV *)PLATFORM_MALLOC(sizeof(struct APVHTCapabilitiesTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_AP_VHT_CAPABILITIES;
		_EnB(&p, ret->radio_unique_id, 6);
		_E2B(&p, &ret->vht_tx_msc);
		_E2B(&p, &ret->vht_rx_msc);
		_E1B(&p, &ret->vht_capability_1);
		_E1B(&p, &ret->vht_capability_2);
		return (INT8U *)ret;
	}

	case TLV_TYPE_AP_HE_CAPABILITIES: {
		// This parsing is done according to the information detailed in
		//  Multi-AP "Section 17.2.10"
		struct APHECapabilitiesTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct APHECapabilitiesTLV *)PLATFORM_MALLOC(sizeof(struct APHECapabilitiesTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_AP_HE_CAPABILITIES;
		_EnB(&p, ret->radio_unique_id, 6);
		_E1B(&p, &ret->he_msc_len);
		ret->he_msc = (INT8U *)PLATFORM_MALLOC(ret->he_msc_len);
		_EnB(&p, ret->he_msc, ret->he_msc_len);
		_E1B(&p, &ret->he_capability_1);
		_E1B(&p, &ret->he_capability_2);
		return (INT8U *)ret;
	}

	case TLV_TYPE_STEERING_POLICY: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.11"
		struct SteeringPolicyTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct SteeringPolicyTLV *)PLATFORM_MALLOC(sizeof(struct SteeringPolicyTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_STEERING_POLICY;

		_E1B(&p, &ret->local_disallowed_sta_nr);
		ret->local_disallowed_sta_mac = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(*(ret->local_disallowed_sta_mac)) * ret->local_disallowed_sta_nr);
		int h;
		for (h = 0; h < ret->local_disallowed_sta_nr; h++) {
			_EnB(&p, ret->local_disallowed_sta_mac[h], 6);
		}

		_E1B(&p, &ret->btm_disallowed_sta_nr);
		ret->btm_disallowed_sta_mac = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(*(ret->btm_disallowed_sta_mac)) * ret->btm_disallowed_sta_nr);
		int k;
		for (k = 0; k < ret->btm_disallowed_sta_nr; k++) {
			_EnB(&p, ret->btm_disallowed_sta_mac[k], 6);
		}

		_E1B(&p, &ret->radios_nr);
		ret->policies = (struct SteeringPolicy *)PLATFORM_MALLOC(sizeof(struct SteeringPolicy) * ret->radios_nr);
		int m;
		for (m = 0; m < ret->radios_nr; m++) {
			_EnB(&p, ret->policies[m].radio_unique_id, 6);
			_E1B(&p, &ret->policies[m].policy);
			_E1B(&p, &ret->policies[m].channel_util_threshold);
			_E1B(&p, &ret->policies[m].rcpi_steering_threshold);
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_METRIC_REPORT_POLICY: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.12"
		struct MetricReportingPolicyTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct MetricReportingPolicyTLV *)PLATFORM_MALLOC(sizeof(struct MetricReportingPolicyTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_METRIC_REPORT_POLICY;

		_E1B(&p, &ret->report_interval);
		_E1B(&p, &ret->radios_nr);
		ret->policies = (struct MetricReportingPolicy *)PLATFORM_MALLOC(sizeof(struct MetricReportingPolicy) * ret->radios_nr);
		int k;
		for (k = 0; k < ret->radios_nr; k++) {
			_EnB(&p, ret->policies[k].radio_unique_id, 6);
			_E1B(&p, &ret->policies[k].sta_rcpi_threshold);
			_E1B(&p, &ret->policies[k].sta_rcpi_hysteresis_margin_override);
			_E1B(&p, &ret->policies[k].ap_channel_utilization_threshold);
			_E1B(&p, &ret->policies[k].policy);
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_CHANNEL_PREFERENCE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.13"
		struct ChannelPreferenceTLV *ret;

		INT8U *p;
		INT16U len;
		ret = (struct ChannelPreferenceTLV *)PLATFORM_MALLOC(sizeof(struct ChannelPreferenceTLV));
		p   = packet_stream + 1;
		_E2B(&p, &len);
		ret->tlv_type = TLV_TYPE_CHANNEL_PREFERENCE;

		_EnB(&p, ret->radio_unique_id, 6);
		_E1B(&p, &ret->op_class_nr);
		ret->channel_preferences = (struct ChannelPreferencePerOpClass *)PLATFORM_MALLOC(sizeof(struct ChannelPreferencePerOpClass) * ret->op_class_nr);
		int k;
		for (k = 0; k < ret->op_class_nr; k++) {
			_E1B(&p, &ret->channel_preferences[k].op_class);
			_E1B(&p, &ret->channel_preferences[k].channel_nr);
			ret->channel_preferences[k].channel_list = NULL;
			if (ret->channel_preferences[k].channel_nr) {
				ret->channel_preferences[k].channel_list = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * ret->channel_preferences[k].channel_nr);
				_EnB(&p, ret->channel_preferences[k].channel_list, ret->channel_preferences[k].channel_nr);
			}
			_E1B(&p, &ret->channel_preferences[k].preference_reason_code);
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_RADIO_OPERATION_RESTRICTION: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.14"
		struct RadioOperationRestrictionTLV *ret;

		INT8U *p;
		INT16U len;
		ret = (struct RadioOperationRestrictionTLV *)PLATFORM_MALLOC(sizeof(struct RadioOperationRestrictionTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_RADIO_OPERATION_RESTRICTION;

		_EnB(&p, ret->radio_unique_id, 6);
		_E1B(&p, &ret->op_class_nr);
		ret->radio_operation_restriction = (struct RadioOperationRestrictionPerOpClass *)PLATFORM_MALLOC(sizeof(struct RadioOperationRestrictionPerOpClass) * ret->op_class_nr);
		int k;
		for (k = 0; k < ret->op_class_nr; k++) {
			_E1B(&p, &ret->radio_operation_restriction[k].op_class);
			_E1B(&p, &ret->radio_operation_restriction[k].channel_nr);
			ret->radio_operation_restriction[k].channel_operation_restriction = (struct RadioOperationRestrictionPerChannel *)PLATFORM_MALLOC(sizeof(struct RadioOperationRestrictionPerChannel) * ret->radio_operation_restriction[k].channel_nr);
			int i;
			for (i = 0; i < ret->radio_operation_restriction[k].channel_nr; i++) {
				_E1B(&p, &ret->radio_operation_restriction[k].channel_operation_restriction[i].channel);
				_E1B(&p, &ret->radio_operation_restriction[k].channel_operation_restriction[i].min_freq_separation);
			}
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_TRANSMIT_POWER_LIMIT: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.15"
		struct TransmitPowerLimitTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct TransmitPowerLimitTLV *)PLATFORM_MALLOC(sizeof(struct TransmitPowerLimitTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_TRANSMIT_POWER_LIMIT;

		_EnB(&p, ret->radio_unique_id, 6);
		_E1B(&p, &ret->transmit_power_limit);

		return (INT8U *)ret;
	}

	case TLV_TYPE_CHANNEL_SELECTION_RESPONSE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.16"
		struct ChannelSelectionResponseTLV *ret;

		INT8U *p;
		INT16U len;
		ret = (struct ChannelSelectionResponseTLV *)PLATFORM_MALLOC(sizeof(struct ChannelSelectionResponseTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_CHANNEL_SELECTION_RESPONSE;

		_EnB(&p, ret->radio_unique_id, 6);
		_E1B(&p, &ret->response);

		return (INT8U *)ret;
	}

	case TLV_TYPE_OPERATING_CHANNEL_REPORT: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.17"
		struct OperatingChannelReportTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct OperatingChannelReportTLV *)PLATFORM_MALLOC(sizeof(struct OperatingChannelReportTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_OPERATING_CHANNEL_REPORT;

		_EnB(&p, ret->radio_unique_id, 6);
		_E1B(&p, &ret->cur_op_class_nr);
		ret->operating_channels = (struct OperatingChannelReportPerOpClass *)PLATFORM_MALLOC(sizeof(struct OperatingChannelReportPerOpClass) * ret->cur_op_class_nr);
		int k;
		for (k = 0; k < ret->cur_op_class_nr; k++) {
			_E1B(&p, &ret->operating_channels[k].op_class);
			_E1B(&p, &ret->operating_channels[k].cur_channel);
		}
		_E1B(&p, &ret->cur_transmit_power);

		return (INT8U *)ret;
	}

	case TLV_TYPE_CLIENT_INFO: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.18"
		struct ClientInfoTLV *ret;

		INT8U *p;
		INT16U len;
		ret = (struct ClientInfoTLV *)PLATFORM_MALLOC(sizeof(struct ClientInfoTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);
		ret->tlv_type = TLV_TYPE_CLIENT_INFO;
		_EnB(&p, ret->bssid, 6);
		_EnB(&p, ret->mac_address, 6);

		return (INT8U *)ret;
	}

	case TLV_TYPE_CLIENT_CAPABLITY_REPORT: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.19"
		struct ClientCapabilityReportTLV *ret;

		INT8U *p;
		INT16U len;
		ret = (struct ClientCapabilityReportTLV *)PLATFORM_MALLOC(sizeof(struct ClientCapabilityReportTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_CLIENT_CAPABLITY_REPORT;
		_E1B(&p, &ret->result_code);
		//PLATFORM_PRINTF("%s - tlv length: %d\n", __FUNCTION__, len);
		//PLATFORM_PRINTF("%s - result code: %d\n", __FUNCTION__, ret->result_code);
		//PLATFORM_PRINTF("%s - client capability length: %d\n", __FUNCTION__, (len-1));
		if (ret->result_code == 0x00) {
			//PLATFORM_PRINTF("%s - Result code is 0x00 - has frame body\n", __FUNCTION__);
			ret->frame_body_length = (len - 1);
			ret->frame_body        = (INT8U *)PLATFORM_MALLOC(len - 1);
			_EnB(&p, ret->frame_body, ret->frame_body_length);
		} else {
			//PLATFORM_PRINTF("%s - Result code not 0x00 - no frame body\n", __FUNCTION__);
			ret->frame_body_length = 0;
			ret->frame_body        = NULL;
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_CLIENT_ASSOCIATION_EVENT: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.20"
		struct ClientAssociationEventTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct ClientAssociationEventTLV *)PLATFORM_MALLOC(sizeof(struct ClientAssociationEventTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_CLIENT_ASSOCIATION_EVENT;
		_EnB(&p, ret->mac_address, 6);
		_EnB(&p, ret->bssid, 6);
		_E1B(&p, &ret->event);

		return (INT8U *)ret;
	}

	case TLV_TYPE_AP_METRIC_QUERY: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.21"
		struct APMetricQueryTLV *ret;

		INT8U *p;
		INT16U len;
		INT8U  k;
		ret = (struct APMetricQueryTLV *)PLATFORM_MALLOC(sizeof(struct APMetricQueryTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);
		ret->tlv_type = TLV_TYPE_AP_METRIC_QUERY;

		_E1B(&p, &ret->bssid_nr);

		ret->bssid = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(*(ret->bssid)) * ret->bssid_nr);

		for (k = 0; k < ret->bssid_nr; k++) {
			_EnB(&p, ret->bssid[k], 6);
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_AP_METRICS: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.22"
		struct APMetricsTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct APMetricsTLV *)PLATFORM_MALLOC(sizeof(struct APMetricsTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);
		ret->tlv_type = TLV_TYPE_AP_METRICS;

		_EnB(&p, ret->bssid, 6);
		_E1B(&p, &ret->ch_util);
		_E2B(&p, &ret->sta_nr);
		_E1B(&p, &ret->esp_ie);
		_EnB(&p, ret->esp_acbe, 3);

		if (ret->esp_ie & 64) //bit 6
			_EnB(&p, ret->esp_acbk, 3);
		if (ret->esp_ie & 32) //bit 5
			_EnB(&p, ret->esp_acvo, 3);
		if (ret->esp_ie & 16) //bit 4
			_EnB(&p, ret->esp_acvi, 3);

		return (INT8U *)ret;
	}

	case TLV_TYPE_STA_MAC_ADDRESS_TYPE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.23"
		struct STAMacAddressTypeTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct STAMacAddressTypeTLV *)PLATFORM_MALLOC(sizeof(struct STAMacAddressTypeTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);
		ret->tlv_type = TLV_TYPE_STA_MAC_ADDRESS_TYPE;
		_EnB(&p, ret->mac_address, 6);

		return (INT8U *)ret;
	}

	case TLV_TYPE_ASSOCIATED_STA_LINK_METRICS: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.24"
		struct AssociatedSTALinkMetricsTLV *ret;

		INT8U *p;
		INT16U len;
		INT8U  k;

		ret = (struct AssociatedSTALinkMetricsTLV *)PLATFORM_MALLOC(sizeof(struct AssociatedSTALinkMetricsTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);
		ret->tlv_type = TLV_TYPE_ASSOCIATED_STA_LINK_METRICS;

		_EnB(&p, ret->mac_address, 6);
		_E1B(&p, &ret->bssid_nr);

		if (ret->bssid_nr > 0) {

			ret->assoc_sta_link_metrics = (struct AssocSTALinkMetrics *)PLATFORM_MALLOC(sizeof(struct AssocSTALinkMetrics) * ret->bssid_nr);

			for (k = 0; k < ret->bssid_nr; k++) {
				_EnB(&p, ret->assoc_sta_link_metrics[k].bssid, 6);
				_E4B(&p, &ret->assoc_sta_link_metrics[k].time_delta);
				_E4B(&p, &ret->assoc_sta_link_metrics[k].dataRate_downlink);
				_E4B(&p, &ret->assoc_sta_link_metrics[k].dataRate_uplink);
				_E1B(&p, &ret->assoc_sta_link_metrics[k].uplink_rcpi);
			}
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.25"
		struct UnassociatedSTALinkMetricsQueryTLV *ret;

		INT8U *p;
		INT16U len;
		INT8U  k, m;
		ret = (struct UnassociatedSTALinkMetricsQueryTLV *)PLATFORM_MALLOC(sizeof(struct UnassociatedSTALinkMetricsQueryTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);
		ret->tlv_type = TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY;

		_E1B(&p, &ret->op_class);
		_E1B(&p, &ret->channel_list_nr);

		if (ret->channel_list_nr > 0) {
			ret->channel_list_entry = PLATFORM_MALLOC(sizeof(struct ChannelListInfo) * ret->channel_list_nr);

			for (k = 0; k < ret->channel_list_nr; k++) {
				_E1B(&p, &ret->channel_list_entry[k].channel_nr);
				_E1B(&p, &ret->channel_list_entry[k].sta_nr);

				if (ret->channel_list_entry[k].sta_nr > 0) {
					ret->channel_list_entry[k].sta_mac_address = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(*(ret->channel_list_entry[k].sta_mac_address)) * ret->channel_list_entry[k].sta_nr);

					for (m = 0; m < ret->channel_list_entry[k].sta_nr; m++) {
						_EnB(&p, ret->channel_list_entry[k].sta_mac_address[m], 6);
					}
				}
			}
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.26"
		struct UnassociatedSTALinkMetricsResponseTLV *ret;

		INT8U *p;
		INT16U len;
		INT8U  k;
		ret = (struct UnassociatedSTALinkMetricsResponseTLV *)PLATFORM_MALLOC(sizeof(struct UnassociatedSTALinkMetricsResponseTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);
		ret->tlv_type = TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE;

		_E1B(&p, &ret->op_class);
		_E1B(&p, &ret->sta_nr);

		if (ret->sta_nr > 0) {
			ret->unassoc_sta_link_metrics = PLATFORM_MALLOC(sizeof(struct UnassocSTALinkMetrics) * ret->sta_nr);

			for (k = 0; k < ret->sta_nr; k++) {
				_EnB(&p, ret->unassoc_sta_link_metrics[k].sta_mac_address, 6);
				_E1B(&p, &ret->unassoc_sta_link_metrics[k].channel_number);
				_E4B(&p, &ret->unassoc_sta_link_metrics[k].time_delta);
				_E1B(&p, &ret->unassoc_sta_link_metrics[k].uplink_rcpi);
			}
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_BEACON_METRICS_QUERY: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.27"
		struct BeaconMetricsQueryTLV *ret;

		INT8U *p;
		INT16U len;
		INT8U  k;

		ret = (struct BeaconMetricsQueryTLV *)PLATFORM_MALLOC(sizeof(struct BeaconMetricsQueryTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);
		ret->tlv_type = TLV_TYPE_BEACON_METRICS_QUERY;

		_EnB(&p, ret->mac_address, 6);
		_E1B(&p, &ret->op_class);
		_E1B(&p, &ret->channel_number);
		_EnB(&p, ret->bssid, 6);
		_E1B(&p, &ret->report_detail);

		_E1B(&p, &ret->ssid_len);
		if (ret->ssid_len) {
			ret->ssid = PLATFORM_MALLOC(ret->ssid_len + 1);
			_EnB(&p, ret->ssid, ret->ssid_len);
			ret->ssid[ret->ssid_len] = '\0';
		}

		_E1B(&p, &ret->ap_channel_report_nr);

		if (ret->ap_channel_report_nr > 0) {
			ret->ap_channel_report = PLATFORM_MALLOC(sizeof(struct ApChannelReport) * ret->ap_channel_report_nr);

			for (k = 0; k < ret->ap_channel_report_nr; k++) {
				_E1B(&p, &ret->ap_channel_report[k].len);
				_E1B(&p, &ret->ap_channel_report[k].op_class);
				if (ret->ap_channel_report[k].len > 1) {
					ret->ap_channel_report[k].channel_list = PLATFORM_MALLOC(ret->ap_channel_report[k].len - 1);
					_EnB(&p, ret->ap_channel_report[k].channel_list, ret->ap_channel_report[k].len - 1);
				}
			}
		}

		_E1B(&p, &ret->elementID_nr);

		if (ret->elementID_nr > 0) {
			ret->element_list = PLATFORM_MALLOC(ret->elementID_nr);
			_EnB(&p, ret->element_list, ret->elementID_nr);
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_BEACON_METRICS_RESPONSE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.28"
		struct BeaconMetricsResponseTLV *ret;

		INT8U *p;
		INT16U len, rpt_len = 8;
		INT8U  k;
		ret = (struct BeaconMetricsResponseTLV *)PLATFORM_ZALLOC(sizeof(struct BeaconMetricsResponseTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);
		ret->tlv_type = TLV_TYPE_BEACON_METRICS_RESPONSE;

		_EnB(&p, ret->mac_address, 6);
		_E1B(&p, &ret->reserved_field);
		_E1B(&p, &ret->measurement_report_nr);
		if (ret->measurement_report_nr > 0)
			ret->measurement_reports = PLATFORM_ZALLOC(sizeof(struct BeaconMeasurementReportIE) * ret->measurement_report_nr);
		else
			ret->measurement_reports = NULL;

		for (k = 0; k < ret->measurement_report_nr; k++) {
			_E1B(&p, &ret->measurement_reports[k].elementId);
			_E1B(&p, &ret->measurement_reports[k].len);

			if (rpt_len + 2 + ret->measurement_reports[k].len > len) {
				PLATFORM_FREE(ret->measurement_reports);
				ret->measurement_report_nr = 0;
				ret->measurement_reports   = NULL;
				break;
			} else
				rpt_len += (2 + ret->measurement_reports[k].len);

			_E1B(&p, &ret->measurement_reports[k].measurementToken);
			_E1B(&p, &ret->measurement_reports[k].measurementReportMode);
			_E1B(&p, &ret->measurement_reports[k].measurementType);

			if (ret->measurement_reports[k].measurementReportMode != 0) {
				if (ret->measurement_reports[k].len <= 3) {
					ret->measurement_reports[k].subelements_len = 0;
					ret->measurement_reports[k].subelements     = NULL;
					break;
				}
			}
			_E1B(&p, &ret->measurement_reports[k].info.op_class);
			_E1B(&p, &ret->measurement_reports[k].info.channel);
			_E4B(&p, &ret->measurement_reports[k].info.measure_time_hi);
			_E4B(&p, &ret->measurement_reports[k].info.measure_time_lo);
			_E2B(&p, &ret->measurement_reports[k].info.measure_duration);
			_E1B(&p, &ret->measurement_reports[k].info.frame_info);
			_E1B(&p, &ret->measurement_reports[k].info.RCPI);
			_E1B(&p, &ret->measurement_reports[k].info.RSNI);
			_EnB(&p, ret->measurement_reports[k].info.bssid, MACADDRLEN);
			_E1B(&p, &ret->measurement_reports[k].info.antenna_id);
			_E4B(&p, &ret->measurement_reports[k].info.parent_tsf);

			if (ret->measurement_reports[k].len > BEACON_REPORT_FIXED_IE) {
				_E1B(&p, &ret->measurement_reports[k].subelement_id);
				_E1B(&p, &ret->measurement_reports[k].subelements_len);

				if (BEACON_REPORT_FIXED_IE + 2 + ret->measurement_reports[k].subelements_len > ret->measurement_reports[k].len) {
					ret->measurement_reports[k].subelements_len = 0;
					ret->measurement_reports[k].subelements     = NULL;
					break;
				}
				if (ret->measurement_reports[k].subelements_len > 0) {
					ret->measurement_reports[k].subelements = PLATFORM_MALLOC(ret->measurement_reports[k].subelements_len);
					_EnB(&p, ret->measurement_reports[k].subelements, ret->measurement_reports[k].subelements_len);
				} else {
					ret->measurement_reports[k].subelements_len = 0;
					ret->measurement_reports[k].subelements     = NULL;
				}
				ret->measurement_reports[k].subelements_remain_len = ret->measurement_reports[k].len - 31 - ret->measurement_reports[k].subelements_len;
				if (ret->measurement_reports[k].subelements_remain_len > 0) {
					_EnB(&p, ret->measurement_reports[k].subelements_remain, ret->measurement_reports[k].subelements_remain_len);
				}
			} else {
				ret->measurement_reports[k].subelements_len = 0;
				ret->measurement_reports[k].subelements     = NULL;
			}
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_STEERING_REQUEST: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.29"
		struct SteeringRequestTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct SteeringRequestTLV *)PLATFORM_MALLOC(sizeof(struct SteeringRequestTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_STEERING_REQUEST;

		_EnB(&p, ret->bssid, 6);
		_E1B(&p, &ret->mode);
		_E2B(&p, &ret->window);
		_E2B(&p, &ret->disassociation_timer);
		_E1B(&p, &ret->sta_nr);
		ret->sta_mac_address = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(INT8U[6]) * ret->sta_nr);
		int k;
		for (k = 0; k < ret->sta_nr; k++) {
			_EnB(&p, ret->sta_mac_address[k], 6);
		}
		_E1B(&p, &ret->target_bss_nr);
		ret->target_bss = (struct TargetBSS *)PLATFORM_MALLOC(sizeof(struct TargetBSS) * ret->target_bss_nr);
		for (k = 0; k < ret->target_bss_nr; k++) {
			_EnB(&p, ret->target_bss[k].bssid, 6);
			_E1B(&p, &ret->target_bss[k].opclass);
			_E1B(&p, &ret->target_bss[k].channel);
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_STEERING_BTM_REPORT: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.30"
		struct SteeringBTMReportTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct SteeringBTMReportTLV *)PLATFORM_MALLOC(sizeof(struct SteeringBTMReportTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_STEERING_BTM_REPORT;

		_EnB(&p, ret->bssid, 6);
		_EnB(&p, ret->sta_mac_address, 6);
		_E1B(&p, &ret->status_code);
		if (len == 19) {
			_EnB(&p, ret->target_bssid, 6);
		}
		return (INT8U *)ret;
	}

	case TLV_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.31"
		struct ClientAssociationControlRequestTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct ClientAssociationControlRequestTLV *)PLATFORM_MALLOC(sizeof(struct ClientAssociationControlRequestTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST;

		_EnB(&p, ret->bssid, 6);
		_E1B(&p, &ret->association_control);
		_E2B(&p, &ret->validity_period);
		_E1B(&p, &ret->sta_nr);
		ret->sta_mac_address = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(INT8U[6]) * ret->sta_nr);
		int k;
		for (k = 0; k < ret->sta_nr; k++) {
			_EnB(&p, ret->sta_mac_address[k], 6);
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_BACKHAUL_STEERING_REQUEST: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.32"
		struct BackhaulSteeringRequestTLV *ret;

		INT8U *p;
		INT16U len;
		ret = (struct BackhaulSteeringRequestTLV *)PLATFORM_MALLOC(sizeof(struct BackhaulSteeringRequestTLV));

		p = packet_stream + 1;

		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_BACKHAUL_STEERING_REQUEST;

		_EnB(&p, ret->backhaul_mac, 6);
		_EnB(&p, ret->target_bssid, 6);
		_E1B(&p, &ret->op_class);
		_E1B(&p, &ret->channel_number);

		return (INT8U *)ret;
	}

	case TLV_TYPE_BACKHAUL_STEERING_RESPONSE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.33"
		struct BackhaulSteeringResponseTLV *ret;

		INT8U *p;
		INT16U len;
		ret = (struct BackhaulSteeringResponseTLV *)PLATFORM_MALLOC(sizeof(struct BackhaulSteeringResponseTLV));

		p = packet_stream + 1;

		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_BACKHAUL_STEERING_RESPONSE;

		_EnB(&p, ret->backhaul_mac, 6);
		_EnB(&p, ret->target_bssid, 6);
		_E1B(&p, &ret->result_code);

		return (INT8U *)ret;
	}

	case TLV_TYPE_HIGHER_LAYER_DATA: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.34"
		struct HigherLayerDataTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct HigherLayerDataTLV *)PLATFORM_MALLOC(sizeof(struct HigherLayerDataTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_HIGHER_LAYER_DATA;

		_E1B(&p, &ret->protocol);
		INT16U payload_len = 0;
		// Todo: calculate the payload length
		ret->payload = (INT8U *)PLATFORM_MALLOC(payload_len);
		_EnB(&p, ret->payload, payload_len);

		return (INT8U *)ret;
	}

	case TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.35"
		struct AssociatedSTATrafficStatsTLV *ret;
		INT8U *                              p;
		INT16U                               len;

		ret = (struct AssociatedSTATrafficStatsTLV *)PLATFORM_MALLOC(sizeof(struct AssociatedSTATrafficStatsTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);
		ret->tlv_type = TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS;

		_EnB(&p, ret->sta_mac_address, 6);
		_E4B(&p, &ret->bytes_sent);
		_E4B(&p, &ret->bytes_received);
		_E4B(&p, &ret->packets_sent);
		_E4B(&p, &ret->packets_received);
		_E4B(&p, &ret->tx_packets_errors);
		_E4B(&p, &ret->rx_packets_errors);
		_E4B(&p, &ret->retransmission_count);

		return (INT8U *)ret;
	}

	case TLV_TYPE_ERROR_CODE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.36"
		struct ErrorCodeTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct ErrorCodeTLV *)PLATFORM_MALLOC(sizeof(struct ErrorCodeTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_ERROR_CODE;
		_E1B(&p, &ret->reason_code);

		// The value of this field is valid only if the reason code field is set to 0x01 or
		// 0x02. Otherwise this field is ignored by the recipient of this TLV.
		if (ret->reason_code == 0x01 || ret->reason_code == 0x02)
			_EnB(&p, ret->sta_mac_address, 6);
		return (INT8U *)ret;
	}

		////////////////////////////////////////////////////////////////////////////////
		/// Profile 2 & Profile 3 TLVs
		////////////////////////////////////////////////////////////////////////////////

	case TLV_TYPE_CHANNEL_SCAN_REPORTING_POLICY: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.37"
		struct ChannelScanReportingPolicyTLV *ret;

		INT8U *p;
		INT16U len;
		ret = (struct ChannelScanReportingPolicyTLV *)PLATFORM_MALLOC(sizeof(struct ChannelScanReportingPolicyTLV));

		p = packet_stream + 1;

		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_CHANNEL_SCAN_REPORTING_POLICY;

		_E1B(&p, &ret->independent_report_flag);

		return (INT8U *)ret;
	}

	case TLV_TYPE_CHANNEL_SCAN_CAPABILITIES: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.38"
		struct ChannelScanCapabilitiesTLV *ret;

		INT8U *p, k, l;
		INT16U len;
		ret = (struct ChannelScanCapabilitiesTLV *)PLATFORM_MALLOC(sizeof(struct ChannelScanCapabilitiesTLV));

		p = packet_stream + 1;

		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_CHANNEL_SCAN_CAPABILITIES;

		_E1B(&p, &ret->radio_nr);
		ret->radios = (struct ChannelScanCapabilitiesRadio *)PLATFORM_MALLOC(ret->radio_nr * sizeof(struct ChannelScanCapabilitiesRadio));
		for (k = 0; k < ret->radio_nr; k++) {
			_EnB(&p, ret->radios[k].radio_unique_identifier, 6);
			_E1B(&p, &ret->radios[k].scan_capability_flags);
			_E4B(&p, &ret->radios[k].minimum_scan_interval);
			_E1B(&p, &ret->radios[k].op_class_nr);
			ret->radios[k].op_classes = (struct ChannelScanCapabilitiesOpClass *)PLATFORM_MALLOC(ret->radios[k].op_class_nr * sizeof(struct ChannelScanCapabilitiesOpClass));
			for (l = 0; l < ret->radios[k].op_class_nr; l++) {
				_E1B(&p, &ret->radios[k].op_classes[l].op_class);
				_E1B(&p, &ret->radios[k].op_classes[l].channel_nr);
				ret->radios[k].op_classes[l].channel_list = (INT8U *)PLATFORM_MALLOC(ret->radios[k].op_classes[l].channel_nr * sizeof(INT8U));
				_EnB(&p, ret->radios[k].op_classes[l].channel_list, ret->radios[k].op_classes[l].channel_nr);
			}
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_CHANNEL_SCAN_REQUEST: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.39"
		struct ChannelScanRequestTLV *ret;

		INT8U *p, k, l;
		INT16U len;
		ret = (struct ChannelScanRequestTLV *)PLATFORM_MALLOC(sizeof(struct ChannelScanRequestTLV));

		p = packet_stream + 1;

		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_CHANNEL_SCAN_REQUEST;

		_E1B(&p, &ret->flags);
		_E1B(&p, &ret->target_radio_nr);

		ret->target_radios = (struct ChannelScanTargetRadio *)PLATFORM_MALLOC(sizeof(struct ChannelScanTargetRadio) * ret->target_radio_nr);
		for (k = 0; k < ret->target_radio_nr; k++) {
			_EnB(&p, ret->target_radios[k].radio_unique_identifier, 6);
			_E1B(&p, &ret->target_radios[k].target_op_class_nr);
			ret->target_radios[k].target_op_classes = (struct ChannelScanTargetOpClass *)PLATFORM_MALLOC(sizeof(struct ChannelScanTargetOpClass) * ret->target_radios[k].target_op_class_nr);
			for (l = 0; l < ret->target_radios[k].target_op_class_nr; l++) {
				_E1B(&p, &ret->target_radios[k].target_op_classes[l].op_class);
				_E1B(&p, &ret->target_radios[k].target_op_classes[l].target_channel_nr);
				ret->target_radios[k].target_op_classes[l].target_channel_list = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * ret->target_radios[k].target_op_classes[l].target_channel_nr);
				if (ret->target_radios[k].target_op_classes[l].target_channel_nr > 0) {
					_EnB(&p, ret->target_radios[k].target_op_classes[l].target_channel_list, ret->target_radios[k].target_op_classes[l].target_channel_nr);
				}
			}
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_CHANNEL_SCAN_RESULT: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.40"
		struct ChannelScanResultTLV *ret;

		INT8U *p, k;
		INT16U len;
		ret = (struct ChannelScanResultTLV *)PLATFORM_MALLOC(sizeof(struct ChannelScanResultTLV));

		p = packet_stream + 1;

		_E2B(&p, &len);
		ret->tlv_type = TLV_TYPE_CHANNEL_SCAN_RESULT;
		_EnB(&p, ret->radio_unique_identifier, 6);
		_E1B(&p, &ret->op_class);
		_E1B(&p, &ret->channel);
		_E1B(&p, &ret->scan_status);
		if (!ret->scan_status) {
			_E1B(&p, &ret->timestamp_length);
			ret->timestamp                        = (char *)PLATFORM_MALLOC(ret->timestamp_length + 1);
			ret->timestamp[ret->timestamp_length] = '\0';
			_EnB(&p, ret->timestamp, ret->timestamp_length);
			_E1B(&p, &ret->channel_utilization);
			_E1B(&p, &ret->noise);
			_E2B(&p, &ret->neighbor_nr);
			ret->neighbors = (struct ChannelScanNeighbor *)PLATFORM_MALLOC(ret->neighbor_nr * sizeof(struct ChannelScanNeighbor));
			for (k = 0; k < ret->neighbor_nr; k++) {
				_EnB(&p, ret->neighbors[k].bssid, 6);
				_E1B(&p, &ret->neighbors[k].ssid_length);
				ret->neighbors[k].ssid                                = (char *)PLATFORM_MALLOC(ret->neighbors[k].ssid_length + 1);
				ret->neighbors[k].ssid[ret->neighbors[k].ssid_length] = '\0';
				_EnB(&p, ret->neighbors[k].ssid, ret->neighbors[k].ssid_length);
				_E1B(&p, &ret->neighbors[k].signal_strength);
				_E1B(&p, &ret->neighbors[k].channel_bandwidth_length);
				ret->neighbors[k].channel_bandwidth                                             = (char *)PLATFORM_MALLOC(ret->neighbors[k].channel_bandwidth_length + 1);
				ret->neighbors[k].channel_bandwidth[ret->neighbors[k].channel_bandwidth_length] = '\0';
				_EnB(&p, ret->neighbors[k].channel_bandwidth, ret->neighbors[k].channel_bandwidth_length);
				_E1B(&p, &ret->neighbors[k].field_present_flags);
				if (CHANNELUTLIZATION_PRESENT_MASK & ret->neighbors[k].field_present_flags)
					_E1B(&p, &ret->neighbors[k].channel_utilization);
				if (STATIONCOUNT_PRESENT_MASK & ret->neighbors[k].field_present_flags)
					_E2B(&p, &ret->neighbors[k].station_count);
			}
			_E4B(&p, &ret->aggregate_scan_duration);
			_E1B(&p, &ret->scan_type_flags);
		}
		return (INT8U *)ret;
	}

	case TLV_TYPE_TIMESTAMP: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.41"
		struct TimestampTLV *ret;

		INT8U *p;
		INT16U len;
		ret = (struct TimestampTLV *)PLATFORM_MALLOC(sizeof(struct TimestampTLV));

		p = packet_stream + 1;

		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_TIMESTAMP;

		_E1B(&p, &ret->timestamp_length);
		ret->timestamp                        = (char *)PLATFORM_MALLOC(ret->timestamp_length + 1);
		ret->timestamp[ret->timestamp_length] = '\0';
		_EnB(&p, ret->timestamp, ret->timestamp_length);

		return (INT8U *)ret;
	}

	case TLV_TYPE_CAC_REQUEST: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.42"
		struct CACRequestTLV *ret;

		INT8U *p, k;
		INT16U len;
		ret = (struct CACRequestTLV *)PLATFORM_MALLOC(sizeof(struct CACRequestTLV));

		p = packet_stream + 1;

		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_CAC_REQUEST;

		_E1B(&p, &ret->radio_nr);

		ret->radios = (struct CACRequestRadio *)PLATFORM_MALLOC(sizeof(struct CACRequestRadio) * ret->radio_nr);
		for (k = 0; k < ret->radio_nr; k++) {
			_EnB(&p, ret->radios[k].radio_unique_identifier, 6);
			_E1B(&p, &ret->radios[k].op_class);
			_E1B(&p, &ret->radios[k].channel);
			_E1B(&p, &ret->radios[k].flags);
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_CAC_TERMINATION: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.43"
		struct CACTerminationTLV *ret;

		INT8U *p, k;
		INT16U len;
		ret = (struct CACTerminationTLV *)PLATFORM_MALLOC(sizeof(struct CACTerminationTLV));

		p = packet_stream + 1;

		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_CAC_TERMINATION;

		_E1B(&p, &ret->radio_nr);

		ret->radios = (struct CACTerminationRadio *)PLATFORM_MALLOC(sizeof(struct CACTerminationRadio) * ret->radio_nr);
		for (k = 0; k < ret->radio_nr; k++) {
			_EnB(&p, ret->radios[k].radio_unique_identifier, 6);
			_E1B(&p, &ret->radios[k].op_class);
			_E1B(&p, &ret->radios[k].channel);
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_CAC_COMPLETION_REPORT: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.44"
		struct CACCompletionReportTLV *ret;

		INT8U *p, k, l;
		INT16U len;
		ret = (struct CACCompletionReportTLV *)PLATFORM_MALLOC(sizeof(struct CACCompletionReportTLV));

		p = packet_stream + 1;

		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_CAC_COMPLETION_REPORT;

		_E1B(&p, &ret->radio_nr);

		ret->radios = (struct CACCompletionReportRadio *)PLATFORM_MALLOC(sizeof(struct CACCompletionReportRadio) * ret->radio_nr);
		for (k = 0; k < ret->radio_nr; k++) {
			_EnB(&p, ret->radios[k].radio_unique_identifier, 6);
			_E1B(&p, &ret->radios[k].op_class);
			_E1B(&p, &ret->radios[k].channel);
			_E1B(&p, &ret->radios[k].flags);
			_E1B(&p, &ret->radios[k].pairs_nr);
			ret->radios[k].pairs = (struct CACCompletionReportClassChannelPairs *)PLATFORM_MALLOC(sizeof(struct CACCompletionReportClassChannelPairs) * ret->radios[k].pairs_nr);
			for (l = 0; l < ret->radios[k].pairs_nr; l++) {
				_E1B(&p, &ret->radios[k].pairs[l].pairs_op_class);
				_E1B(&p, &ret->radios[k].pairs[l].pairs_channel);
			}
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_CAC_STATUS_REPORT: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.45"
		struct CACStatusReportTLV *ret;

		INT8U *p, k, l, m;
		INT16U len;
		ret = (struct CACStatusReportTLV *)PLATFORM_MALLOC(sizeof(struct CACStatusReportTLV));

		p = packet_stream + 1;

		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_CAC_STATUS_REPORT;

		_E1B(&p, &ret->available_channel_nr);

		ret->available_channels = (struct CACCompletionReportAvailabelChannels *)PLATFORM_MALLOC(sizeof(struct CACCompletionReportAvailabelChannels) * ret->available_channel_nr);
		for (k = 0; k < ret->available_channel_nr; k++) {
			_E1B(&p, &ret->available_channels[k].ac_op_class);
			_E1B(&p, &ret->available_channels[k].ac_channel);
			_E2B(&p, &ret->available_channels[k].identify_time);
		}

		_E1B(&p, &ret->nonoccup_pair_nr);

		if (ret->nonoccup_pair_nr)
			ret->nonoccup_pairs = (struct CACCompletionReportNonOccupancyClassChannelPairs *)PLATFORM_MALLOC(sizeof(struct CACCompletionReportNonOccupancyClassChannelPairs) * ret->nonoccup_pair_nr);
		for (l = 0; l < ret->nonoccup_pair_nr; l++) {
			_E1B(&p, &ret->nonoccup_pairs[l].nonoccup_op_class);
			_E1B(&p, &ret->nonoccup_pairs[l].nonoccup_channel);
			_E2B(&p, &ret->nonoccup_pairs[l].nonoccup_remaining_time);
		}

		_E1B(&p, &ret->active_pair_nr);

		if (ret->active_pair_nr)
			ret->active_pairs = (struct CACCompletionReportActiveClassChannelPairs *)PLATFORM_MALLOC(sizeof(struct CACCompletionReportActiveClassChannelPairs) * ret->active_pair_nr);
		for (m = 0; m < ret->active_pair_nr; m++) {
			_E1B(&p, &ret->active_pairs[m].active_op_class);
			_E1B(&p, &ret->active_pairs[m].active_channel);
			_EnB(&p, ret->active_pairs[m].active_remaining_time, 3);
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_CAC_CAPABILITIES: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.46"
		struct CACCapabilitiesTLV *ret;

		INT8U *p, k, l, m;
		INT16U len;
		ret = (struct CACCapabilitiesTLV *)PLATFORM_MALLOC(sizeof(struct CACCapabilitiesTLV));

		p = packet_stream + 1;

		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_CAC_CAPABILITIES;

		_E2B(&p, (INT16U *)ret->country_code);
		_E1B(&p, &ret->radio_nr);

		ret->radios = (struct CACCapabilitiesRadio *)PLATFORM_MALLOC(sizeof(struct CACCapabilitiesRadio) * ret->radio_nr);
		for (k = 0; k < ret->radio_nr; k++) {
			_EnB(&p, ret->radios[k].radio_unique_identifier, 6);
			_E1B(&p, &ret->radios[k].type_nr);
			ret->radios[k].types = (struct CACCapabilitiesType *)PLATFORM_MALLOC(sizeof(struct CACCapabilitiesType) * ret->radios[k].type_nr);
			for (l = 0; l < ret->radios[k].type_nr; l++) {
				_E1B(&p, &ret->radios[k].types[l].flags);
				_EnB(&p, ret->radios[k].types[l].second_nr, 3);
				_E1B(&p, &ret->radios[k].types[l].op_class_nr);
				ret->radios[k].types[l].op_classes = (struct CACCapabilitiesOpClass *)PLATFORM_MALLOC(sizeof(struct CACCapabilitiesOpClass) * ret->radios[k].types[l].op_class_nr);
				for (m = 0; m < ret->radios[k].types[l].op_class_nr; m++) {
					_E1B(&p, &ret->radios[k].types[l].op_classes[m].op_class);
					_E1B(&p, &ret->radios[k].types[l].op_classes[m].channel_nr);
					ret->radios[k].types[l].op_classes[m].channels = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * ret->radios[k].types[l].op_classes[m].channel_nr);
					if (ret->radios[k].types[l].op_classes[m].channel_nr > 0) {
						_EnB(&p, ret->radios[k].types[l].op_classes[m].channels, ret->radios[k].types[l].op_classes[m].channel_nr);
					}
				}
			}
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_MULTIAP_PROFILE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.47"
		struct MultiAPProfileTLV *ret;
		INT8U *                   p;
		INT16U                    len;
		ret = (struct MultiAPProfileTLV *)PLATFORM_MALLOC(sizeof(struct MultiAPProfileTLV));

		p = packet_stream + 1;

		_E2B(&p, &len);

		if (1 != len) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		ret->tlv_type = TLV_TYPE_MULTIAP_PROFILE;

		_E1B(&p, &ret->profile);

		return (INT8U *)ret;
	}

	case TLV_TYPE_PROFILE_2_CAPABILITY: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.48"
		struct Profile2APCapabilityTLV *ret;
		INT8U *                         p;
		INT16U                          len;
		ret = (struct Profile2APCapabilityTLV *)PLATFORM_MALLOC(sizeof(struct Profile2APCapabilityTLV));

		p = packet_stream + 1;

		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_PROFILE_2_CAPABILITY;

		_E1B(&p, &ret->max_total_nr_service_prioritization_rules);
		_E1B(&p, &ret->reserved);
		_E1B(&p, &ret->units);
		_E1B(&p, &ret->max_total_nr_VIDs);

		return (INT8U *)ret;
	}

	case TLV_TYPE_DEFAULT_802_1Q_SETTINGS: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.49"
		struct Default8021QSettingsTLV *ret;
		INT8U *                         p;
		INT16U                          len;
		ret = (struct Default8021QSettingsTLV *)PLATFORM_MALLOC(sizeof(struct Default8021QSettingsTLV));

		p = packet_stream + 1;

		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_DEFAULT_802_1Q_SETTINGS;

		_E2B(&p, &ret->primary_vlan_id);
		_E1B(&p, &ret->default_pcp);

		return (INT8U *)ret;
	}

	case TLV_TYPE_TRAFFIC_SEPARATION_POLICY: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.50"
		struct TrafficSeparationPolicyTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct TrafficSeparationPolicyTLV *)PLATFORM_MALLOC(sizeof(struct TrafficSeparationPolicyTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_TRAFFIC_SEPARATION_POLICY;

		_E1B(&p, &ret->ssid_nr);
		ret->ssid_info = (struct TrafficSeparationSsidInfo *)PLATFORM_MALLOC(sizeof(struct TrafficSeparationSsidInfo) * ret->ssid_nr);
		int k;
		for (k = 0; k < ret->ssid_nr; k++) {
			_E1B(&p, &ret->ssid_info[k].ssid_length);
			if (ret->ssid_info[k].ssid_length) {
				ret->ssid_info[k].ssid_name = PLATFORM_MALLOC(ret->ssid_info[k].ssid_length + 1);
				_EnB(&p, ret->ssid_info[k].ssid_name, ret->ssid_info[k].ssid_length);
				ret->ssid_info[k].ssid_name[ret->ssid_info[k].ssid_length] = '\0';
			}
			_E2B(&p, &ret->ssid_info[k].vlan_id);
		}
		return (INT8U *)ret;
	}

	case TLV_TYPE_PROFILE_2_ERROR_CODE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.51"
		struct Profile2ErrorCodeTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct Profile2ErrorCodeTLV *)PLATFORM_MALLOC(sizeof(struct Profile2ErrorCodeTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_PROFILE_2_ERROR_CODE;

		_E1B(&p, &ret->reason_code);
		_E4B(&p, &ret->service_prioritization_rule_id);

		return (INT8U *)ret;
	}

	case TLV_TYPE_AP_RADIO_ADVANCED_CAPABILITIES: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.52"
		struct APRadioAdvancedCapabilitiesTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct APRadioAdvancedCapabilitiesTLV *)PLATFORM_MALLOC(sizeof(struct APRadioAdvancedCapabilitiesTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_AP_RADIO_ADVANCED_CAPABILITIES;

		_EnB(&p, ret->radio_id, 6);
		_E1B(&p, &ret->backhaul_bss_traffic_separation_mix_no_support);

		return (INT8U *)ret;
	}

	case TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.53"
		struct AssociationStatusNotificationTLV *ret;
		INT8U *                                  p;
		INT16U                                   len;
		int                                      k;

		ret = (struct AssociationStatusNotificationTLV *)PLATFORM_MALLOC(sizeof(struct AssociationStatusNotificationTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION;

		_E1B(&p, &ret->bssid_nr);
		if (ret->bssid_nr) {
			ret->bss_info = (struct AssociationStatusNotificationBssInfo *)PLATFORM_MALLOC(sizeof(struct AssociationStatusNotificationBssInfo) * ret->bssid_nr);
			for (k = 0; k < ret->bssid_nr; k++) {
				_EnB(&p, ret->bss_info[k].bssid, 6);
				_E1B(&p, &ret->bss_info[k].association_allowance_status);
			}
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_SOURCE_INFO: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.54"
		struct SourceInfoTLV *ret;
		INT8U *               p;
		INT16U                len;

		ret = (struct SourceInfoTLV *)PLATFORM_MALLOC(sizeof(struct SourceInfoTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_SOURCE_INFO;

		_EnB(&p, ret->mac_address, 6);

		return (INT8U *)ret;
	}

	case TLV_TYPE_TUNNELED_MESSAGE_TYPE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.55"
		struct TunneledMessageTypeTLV *ret;
		INT8U *                        p;
		INT16U                         len;

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret = (struct TunneledMessageTypeTLV *)PLATFORM_MALLOC(sizeof(struct TunneledMessageTypeTLV));

		ret->tlv_type = TLV_TYPE_TUNNELED_MESSAGE_TYPE;

		_E1B(&p, &ret->tunneled_protocol_type_payload);

		return (INT8U *)ret;
	}

	case TLV_TYPE_TUNNELED: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.56"
		struct TunneledTLV *ret;
		INT8U *             p;
		INT16U              len;

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret             = (struct TunneledTLV *)PLATFORM_MALLOC(sizeof(struct TunneledTLV));
		ret->tlv_type   = TLV_TYPE_TUNNELED;
		ret->tlv_length = len;
		ret->payload    = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * len);
		_EnB(&p, ret->payload, len);

		return (INT8U *)ret;
	}

	case TLV_TYPE_PROFILE_2_STEERING_REQUEST: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.57"
		struct Profile2SteeringRequestTLV *ret;
		INT8U *                            p;
		INT16U                             len;

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret = (struct Profile2SteeringRequestTLV *)PLATFORM_MALLOC(sizeof(struct Profile2SteeringRequestTLV));

		ret->tlv_type = TLV_TYPE_PROFILE_2_STEERING_REQUEST;

		_EnB(&p, ret->bssid, 6);
		_E1B(&p, &ret->steer_parameters);
		_E2B(&p, &ret->steer_opportunity_window);
		_E2B(&p, &ret->btm_disassociation_timer);
		_E1B(&p, &ret->sta_list_count);

		int k;

		ret->sta_list = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(INT8U[6]) * ret->sta_list_count);

		for (k = 0; k < ret->sta_list_count; k++) {
			_EnB(&p, ret->sta_list[k], 6);
		}

		_E1B(&p, &ret->target_bssid_list_count);
		if (ret->target_bssid_list_count) {
			ret->target_bssid_list = PLATFORM_MALLOC(sizeof(struct Profile2SteeringRequestTargetBssidInfo) * ret->target_bssid_list_count);
			for (k = 0; k < ret->target_bssid_list_count; k++) {
				_EnB(&p, ret->target_bssid_list[k].target_bssid, 6);
				_E1B(&p, &ret->target_bssid_list[k].target_bss_operating_class);
				_E1B(&p, &ret->target_bssid_list[k].target_bss_channel_number);
				_E1B(&p, &ret->target_bssid_list[k].reason_code);
			}
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_UNSUCCESSFUL_ASSOCIATION_POILCY: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.58"
		struct UnsuccessfulAssociationPolicyTLV *ret;
		INT8U *                                  p;
		INT16U                                   len;

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret = (struct UnsuccessfulAssociationPolicyTLV *)PLATFORM_MALLOC(sizeof(struct UnsuccessfulAssociationPolicyTLV));

		ret->tlv_type = TLV_TYPE_UNSUCCESSFUL_ASSOCIATION_POILCY;

		_E1B(&p, &ret->report_unsuccessful_associations);
		_E4B(&p, &ret->maximum_reporting_rate);

		return (INT8U *)ret;
	}

	case TLV_TYPE_METRIC_COLLECTION_INTERVAL: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.59"
		struct MetricCollectionIntervalTLV *ret;
		INT8U *                             p;
		INT16U                              len;

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret = (struct MetricCollectionIntervalTLV *)PLATFORM_MALLOC(sizeof(struct MetricCollectionIntervalTLV));

		ret->tlv_type = TLV_TYPE_METRIC_COLLECTION_INTERVAL;

		_E4B(&p, &ret->collection_interval);

		return (INT8U *)ret;
	}

	case TLV_TYPE_RADIO_METRICS: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.60"
		struct RadioMetricsTLV *ret;
		INT8U *                 p;
		INT16U                  len;

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret = (struct RadioMetricsTLV *)PLATFORM_MALLOC(sizeof(struct RadioMetricsTLV));

		ret->tlv_type = TLV_TYPE_RADIO_METRICS;

		_EnB(&p, ret->radio_unique_identifier, 6);
		_E1B(&p, &ret->noise);
		_E1B(&p, &ret->transmit);
		_E1B(&p, &ret->receiveself);
		_E1B(&p, &ret->receiveother);

		return (INT8U *)ret;
	}

	case TLV_TYPE_AP_EXTENDED_METRICS: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.61"
		struct APExtendedMetricsTLV *ret;
		INT8U *                      p;
		INT16U                       len;

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret = (struct APExtendedMetricsTLV *)PLATFORM_MALLOC(sizeof(struct APExtendedMetricsTLV));

		ret->tlv_type = TLV_TYPE_AP_EXTENDED_METRICS;

		_EnB(&p, ret->radio_unique_identifier, 6);
		_E4B(&p, &ret->unicast_byte_sent);
		_E4B(&p, &ret->unicast_byte_received);
		_E4B(&p, &ret->multicast_byte_sent);
		_E4B(&p, &ret->multicast_byte_received);
		_E4B(&p, &ret->broadcast_byte_sent);
		_E4B(&p, &ret->broadcast_byte_received);

		return (INT8U *)ret;
	}

	case TLV_TYPE_ASSOCIATED_STA_EXTENDED_LINK_METRICS: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.62"
		struct AssociatedSTAExtendedLinkMericsTLV *ret;
		INT8U *                                    p;
		INT16U                                     len;

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret = (struct AssociatedSTAExtendedLinkMericsTLV *)PLATFORM_MALLOC(sizeof(struct AssociatedSTAExtendedLinkMericsTLV));

		ret->tlv_type = TLV_TYPE_ASSOCIATED_STA_EXTENDED_LINK_METRICS;

		_EnB(&p, ret->sta_mac_address, 6);
		_E1B(&p, &ret->bssid_nr);

		if (ret->bssid_nr) {
			ret->reported_bss = (struct ReportedBSSInfo *)PLATFORM_MALLOC(sizeof(struct ReportedBSSInfo) * ret->bssid_nr);
			int k;
			for (k = 0; k < ret->bssid_nr; k++) {
				_EnB(&p, ret->reported_bss[k].bssid, 6);
				_E4B(&p, &ret->reported_bss[k].last_data_downlink_rate);
				_E4B(&p, &ret->reported_bss[k].last_data_uplink_rate);
				_E4B(&p, &ret->reported_bss[k].utilization_receive);
				_E4B(&p, &ret->reported_bss[k].utilization_transmit);
			}
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_STATUS_CODE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.63"
		struct StatusCodeTLV *ret;
		INT8U *               p;
		INT16U                len;

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret = (struct StatusCodeTLV *)PLATFORM_MALLOC(sizeof(struct StatusCodeTLV));

		ret->tlv_type = TLV_TYPE_STATUS_CODE;

		_E2B(&p, &ret->status_code);

		return (INT8U *)ret;
	}

	case TLV_TYPE_REASON_CODE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.64"
		struct ReasonCodeTLV *ret;
		INT8U *               p;
		INT16U                len;

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret = (struct ReasonCodeTLV *)PLATFORM_MALLOC(sizeof(struct ReasonCodeTLV));

		ret->tlv_type = TLV_TYPE_REASON_CODE;

		_E2B(&p, &ret->reason_code);

		return (INT8U *)ret;
	}

	case TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.65"
		struct BackhaulSTARadioCapabilitiesTLV *ret;
		INT8U *                                 p;
		INT16U                                  len;

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret = (struct BackhaulSTARadioCapabilitiesTLV *)PLATFORM_MALLOC(sizeof(struct BackhaulSTARadioCapabilitiesTLV));

		ret->tlv_type = TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES;

		_EnB(&p, ret->radio_unique_identifier, 6);
		_E1B(&p, &ret->included_mac);
		_EnB(&p, ret->sta_mac_address, 6);

		return (INT8U *)ret;
	}

	case TLV_TYPE_BACKHAUL_BSS_CONFIGURATION: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.66"
		struct BackhaulBSSConfigurationTLV *ret;
		INT8U *                             p;
		INT16U                              len;

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret = (struct BackhaulBSSConfigurationTLV *)PLATFORM_MALLOC(sizeof(struct BackhaulBSSConfigurationTLV));

		ret->tlv_type = TLV_TYPE_BACKHAUL_BSS_CONFIGURATION;

		_EnB(&p, ret->bssid, 6);
		_E1B(&p, &ret->association_disallowed);

		return (INT8U *)ret;
	}

	case TLV_TYPE_1905_LAYER_SECURITY_CAPABILITY: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.67"
		struct SecurityCapabilityTLV *ret;
		INT8U *                       p;
		INT16U                        len;
		ret = (struct SecurityCapabilityTLV *)PLATFORM_MALLOC(sizeof(struct SecurityCapabilityTLV));

		p = packet_stream + 1;

		_E2B(&p, &len);

		if (3 != len) {
			// Malformed packet
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		ret->tlv_type = TLV_TYPE_1905_LAYER_SECURITY_CAPABILITY;

		_E1B(&p, &ret->onboarding_protocol_supported);
		_E1B(&p, &ret->message_integrity_algorithm_supported);
		_E1B(&p, &ret->message_encryption_algorithm_supported);

		return (INT8U *)ret;
	}

	case TLV_TYPE_MIC: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.68"
		struct MICTLV *ret;
		INT8U *        p;
		INT16U         len;
		ret = (struct MICTLV *)PLATFORM_MALLOC(sizeof(struct MICTLV));

		p = packet_stream + 1;

		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_MIC;

		_E1B(&p, &ret->gtk_key_id_ver);
		_INT64_E6B(&p, &ret->integrity_transmission_counter);
		_EnB(&p, ret->src_1905_al_mac_addr, 6);
		_E2B(&p, &ret->mic_length);
		if (ret->mic_length > sizeof(ret->mic)) {
			// Too large to be saved
			PLATFORM_FREE(ret);
			return NULL;
		}
		_EnB(&p, ret->mic, ret->mic_length);

		return (INT8U *)ret;
	}

	case TLV_TYPE_ENCRYPTED_PAYLOAD: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.69"
		struct EncryptedPayloadTLV *ret;
		INT8U *                     p;
		INT16U                      len;

		ret = (struct EncryptedPayloadTLV *)PLATFORM_MALLOC(sizeof(struct EncryptedPayloadTLV));

		p = packet_stream + 1;

		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_ENCRYPTED_PAYLOAD;

		_INT64_E6B(&p, &ret->encryption_transmission_counter);
		_EnB(&p, ret->source_1905_mac_address, 6);
		_EnB(&p, ret->dest_1905_mac_address, 6);
		_E2B(&p, &ret->aes_siv_len);

		ret->aes_siv = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * ret->aes_siv_len);
		_EnB(&p, ret->aes_siv, ret->aes_siv_len);

		return (INT8U *)ret;
	}

	case TLV_TYPE_SERVICE_PRIORITIZATION_RULE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.70"
		struct ServicePrioritizationRuleTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct ServicePrioritizationRuleTLV *)PLATFORM_MALLOC(sizeof(struct ServicePrioritizationRuleTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_SERVICE_PRIORITIZATION_RULE;
		_E4B(&p, &ret->service_prioritization_rule_id);
		_E1B(&p, &ret->add_remove_rule);
		_E1B(&p, &ret->rule_precedence);
		_E1B(&p, &ret->rule_output);
		_E1B(&p, &ret->always_match);

		return (INT8U *)ret;
	}

	case TLV_TYPE_DSCP_MAPPING_TABLE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.71"
		struct DSCPMappingTableTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct DSCPMappingTableTLV *)PLATFORM_MALLOC(sizeof(struct DSCPMappingTableTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_DSCP_MAPPING_TABLE;

		_EnB(&p, ret->dscp_to_pcp, 64);

		return (INT8U *)ret;
	}

	case TLV_TYPE_AP_WIFI_6_CAPABILITIES: {
		// This parsing is done according to the information detailed in
		//  Multi-AP "Section 17.2.72"
		struct APWiFi6CapabilitiesTLV *ret;

		INT8U *p;
		INT16U len;
		INT8U  i, mcs_nss_nr;

		ret = (struct APWiFi6CapabilitiesTLV *)PLATFORM_MALLOC(sizeof(struct APWiFi6CapabilitiesTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_AP_WIFI_6_CAPABILITIES;
		_EnB(&p, ret->radio_unique_identifier, 6);
		_E1B(&p, &ret->role_nr);
		ret->agent_roles = (struct AgentRole *)PLATFORM_MALLOC(sizeof(struct AgentRole) * ret->role_nr);
		for (i = 0; i < ret->role_nr; i++) {
			_E1B(&p, &ret->agent_roles[i].flags);
			mcs_nss_nr                  = ret->agent_roles[i].flags & MCS_NSS_NUMBER_MASK;
			ret->agent_roles[i].mcs_nss = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * mcs_nss_nr);
			_EnB(&p, ret->agent_roles[i].mcs_nss, mcs_nss_nr);
			_E1B(&p, &ret->agent_roles[i].wifi6_capability_1);
			_E1B(&p, &ret->agent_roles[i].wifi6_capability_2);
			_E1B(&p, &ret->agent_roles[i].wifi6_capability_3);
			_E1B(&p, &ret->agent_roles[i].wifi6_capability_4);
			_E1B(&p, &ret->agent_roles[i].wifi6_capability_5);
		}
		return (INT8U *)ret;
	}

	case TLV_TYPE_ASSOCIATED_WIFI_6_STA_STATUS_REPORT: {
		// This parsing is done according to the information detailed in
		//  Multi-AP "Section 17.2.73"
		struct AssociatedWiFi6STAStatusReportTLV *ret;

		INT8U *p;
		INT8U  i;
		INT16U len;

		ret = (struct AssociatedWiFi6STAStatusReportTLV *)PLATFORM_MALLOC(sizeof(struct AssociatedWiFi6STAStatusReportTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_ASSOCIATED_WIFI_6_STA_STATUS_REPORT;
		_EnB(&p, ret->mac_address, 6);
		_E1B(&p, &ret->queue_nr);
		ret->queues = (struct Queue *)PLATFORM_MALLOC(sizeof(struct Queue) * ret->queue_nr);
		for (i = 0; i < ret->queue_nr; i++) {
			_E1B(&p, &ret->queues[i].tid);
			_E1B(&p, &ret->queues[i].queue_size);
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_BSS_CONFIG_REPORT: {
		// This parsing is done according to the information detailed in
		//  Multi-AP "Section 17.2.75"
		struct BSSConfigReportTLV *  ret;
		struct BSSConfigRadioReport *radio_report;
		struct BSSConfigBSSReport *  bss_report;

		INT8U *p, k, m;
		INT16U len;

		ret = (struct BSSConfigReportTLV *)PLATFORM_MALLOC(sizeof(struct BSSConfigReportTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_BSS_CONFIG_REPORT;
		_E1B(&p, &ret->radio_report_nr);
		ret->radio_reports = (struct BSSConfigRadioReport *)
		    PLATFORM_MALLOC(sizeof(struct BSSConfigRadioReport) * ret->radio_report_nr);
		for (k = 0; k < ret->radio_report_nr; k++) {
			radio_report = &ret->radio_reports[k];
			_EnB(&p, radio_report->radio_unique_identifier, 6);
			_E1B(&p, &radio_report->bss_report_nr);
			radio_report->bss_reports = (struct BSSConfigBSSReport *)
			    PLATFORM_MALLOC(sizeof(struct BSSConfigBSSReport) * radio_report->bss_report_nr);
			for (m = 0; m < radio_report->bss_report_nr; m++) {
				bss_report = &radio_report->bss_reports[m];
				_EnB(&p, bss_report->bssid, 6);
				_E1B(&p, &bss_report->status_flag);
				_E1B(&p, &bss_report->reserved);
				_E1B(&p, &bss_report->ssid_length);
				if (bss_report->ssid_length) {
					bss_report->ssid = (char *)PLATFORM_MALLOC(sizeof(INT8U) * bss_report->ssid_length);
					_EnB(&p, bss_report->ssid, bss_report->ssid_length);
				} else {
					bss_report->ssid = NULL;
				}
			}
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_DEVICE_INVENTORY: {
		// This parsing is done according to the information detailed in
		//  Multi-AP "Section 17.2.76"
		struct DeviceInventoryTLV *ret;

		INT8U *p;
		INT16U len;
		INT8U  i;

		ret = (struct DeviceInventoryTLV *)PLATFORM_MALLOC(sizeof(struct DeviceInventoryTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_DEVICE_INVENTORY;
		_E1B(&p, &ret->lsn);
		ret->serial_number = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * ret->lsn);
		_EnB(&p, ret->serial_number, ret->lsn);
		_E1B(&p, &ret->lsv);
		ret->software_version = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * ret->lsv);
		_EnB(&p, ret->software_version, ret->lsv);
		_E1B(&p, &ret->lee);
		ret->execution_env = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * ret->lee);
		_EnB(&p, ret->execution_env, ret->lee);
		_E1B(&p, &ret->radio_nr);
		ret->radios = (struct DeviceInventoryRadio *)PLATFORM_MALLOC(sizeof(struct DeviceInventoryRadio) * ret->radio_nr);
		for (i = 0; i < ret->radio_nr; i++) {
			_EnB(&p, ret->radios[i].radio_unique_identifier, 6);
			_E1B(&p, &ret->radios[i].lcv);
			ret->radios[i].chipset_vendor = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * ret->radios[i].lcv);
			_EnB(&p, ret->radios[i].chipset_vendor, ret->radios[i].lcv);
		}
		return (INT8U *)ret;
	}

	case TLV_TYPE_AGENT_LIST: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.77"
		struct AgentListTLV *ret;

		INT8U *p;
		INT16U len;
		INT8U  i;

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret           = (struct AgentListTLV *)PLATFORM_MALLOC(sizeof(struct AgentListTLV));
		ret->tlv_type = TLV_TYPE_AGENT_LIST;
		_E1B(&p, &ret->agents_nr);
		ret->agents = (struct Agent *)PLATFORM_MALLOC(sizeof(struct Agent) * ret->agents_nr);
		for (i = 0; i < ret->agents_nr; i++) {
			_EnB(&p, ret->agents[i].al_mac_address, 6);
			_E1B(&p, &ret->agents[i].map_profile);
			_E1B(&p, &ret->agents[i].security);
		}
		return (INT8U *)ret;
	}

	case TLV_TYPE_AKM_SUITE_CAPABILITIES: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.78"
		struct AKMSuiteCapabilitiesTLV *ret;
		INT8U *                         p, i;
		INT16U                          len;

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret = (struct AKMSuiteCapabilitiesTLV *)PLATFORM_MALLOC(sizeof(struct AKMSuiteCapabilitiesTLV));

		ret->tlv_type = TLV_TYPE_AKM_SUITE_CAPABILITIES;

		_E1B(&p, &ret->backhaul_bss_akm_suite_selectors_nr);
		if (ret->backhaul_bss_akm_suite_selectors_nr) {
			ret->backhaul_bss_akm_suite_selectors = (struct AKMSuiteSelector *)
			    PLATFORM_MALLOC(sizeof(struct AKMSuiteSelector) * ret->backhaul_bss_akm_suite_selectors_nr);
			for (i = 0; i < ret->backhaul_bss_akm_suite_selectors_nr; i++) {
				_EnB(&p, ret->backhaul_bss_akm_suite_selectors[i].oui, 3);
				_E1B(&p, &ret->backhaul_bss_akm_suite_selectors[i].akm_suite_type);
			}
		} else {
			ret->backhaul_bss_akm_suite_selectors = NULL;
		}

		_E1B(&p, &ret->fronthaul_bss_akm_suite_selectors_nr);
		if (ret->fronthaul_bss_akm_suite_selectors_nr) {
			ret->fronthaul_bss_akm_suite_selectors = (struct AKMSuiteSelector *)
			    PLATFORM_MALLOC(sizeof(struct AKMSuiteSelector) * ret->fronthaul_bss_akm_suite_selectors_nr);
			for (i = 0; i < ret->fronthaul_bss_akm_suite_selectors_nr; i++) {
				_EnB(&p, ret->fronthaul_bss_akm_suite_selectors[i].oui, 3);
				_E1B(&p, &ret->fronthaul_bss_akm_suite_selectors[i].akm_suite_type);
			}
		} else {
			ret->fronthaul_bss_akm_suite_selectors = NULL;
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_1905_ENCAP_DPP: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.79"
		struct Encap1905DPPTLV *ret;
		INT8U *                 p;
		INT16U                  len;

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret           = (struct Encap1905DPPTLV *)PLATFORM_MALLOC(sizeof(struct Encap1905DPPTLV));
		ret->tlv_type = TLV_TYPE_1905_ENCAP_DPP;
		_E1B(&p, &ret->enrollee_mac_address_present);
		_EnB(&p, ret->dest_sta_mac_address, 6);
		_E1B(&p, &ret->frame_type);
		_E2B(&p, &ret->encapsulated_frame_len);
		ret->encapsulated_frame = (INT8U *)PLATFORM_MALLOC(ret->encapsulated_frame_len);
		_EnB(&p, ret->encapsulated_frame, ret->encapsulated_frame_len);

		return (INT8U *)ret;
	}

	case TLV_TYPE_1905_ENCAP_EAPOL: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.80"
		struct Encap1905EAPOLTLV *ret;
		INT8U *                   p;

		p = packet_stream + 1;

		ret           = (struct Encap1905EAPOLTLV *)PLATFORM_MALLOC(sizeof(struct Encap1905EAPOLTLV));
		ret->tlv_type = TLV_TYPE_1905_ENCAP_EAPOL;
		_E2B(&p, &(ret->eapol_frame_payload_len));
		ret->eapol_frame_payload = (INT8U *)PLATFORM_MALLOC(ret->eapol_frame_payload_len);
		_EnB(&p, ret->eapol_frame_payload, ret->eapol_frame_payload_len);

		return (INT8U *)ret;
	}

	case TLV_TYPE_DPP_BOOTSTRAPPING_URI_NOTIFICATION: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.81"
		struct DPPBootstrappingURINotificationTLV *ret;
		INT8U *                                    p;
		INT16U                                     len;

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret = (struct DPPBootstrappingURINotificationTLV *)PLATFORM_MALLOC(sizeof(struct DPPBootstrappingURINotificationTLV));

		ret->tlv_type = TLV_TYPE_DPP_BOOTSTRAPPING_URI_NOTIFICATION;

		_EnB(&p, ret->radio_unique_identifier, 6);
		_EnB(&p, ret->local_interface_mac_address, 6);
		_EnB(&p, ret->sta_mac_address, 6);

		ret->dpp_bootstrapping_uri_len = len - 18;
		ret->dpp_bootstrapping_uri     = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * ret->dpp_bootstrapping_uri_len);
		_EnB(&p, ret->dpp_bootstrapping_uri, ret->dpp_bootstrapping_uri_len);

		return (INT8U *)ret;
	}

	case TLV_TYPE_DPP_CCE_INDICATION: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.82"

		struct DPPCCEIndicationTLV *ret;
		INT8U                      *p;
		INT16U                      len;

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret = (struct DPPCCEIndicationTLV *)PLATFORM_MALLOC(sizeof(struct DPPCCEIndicationTLV));
		ret->tlv_type = TLV_TYPE_DPP_CCE_INDICATION;

		_E1B(&p, &ret->advertise_cce);

		return (INT8U *)ret;
	}

	case TLV_TYPE_DPP_CHIRP_VALUE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.83"
		struct DPPChirpValueTLV *ret;
		INT8U *                  p;
		INT16U                   len;

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret = (struct DPPChirpValueTLV *)PLATFORM_MALLOC(sizeof(struct DPPChirpValueTLV));

		ret->tlv_type = TLV_TYPE_DPP_CHIRP_VALUE;

		_E1B(&p, &ret->hash_validity);
		if (ret->hash_validity & DPP_CHIRP_ENROLLEE_ADDR_PRESENT)
			_EnB(&p, ret->sta_mac_address, 6);
		_E1B(&p, &ret->hash_len);
		ret->hash_value = (INT8U *)PLATFORM_MALLOC(ret->hash_len);
		_EnB(&p, ret->hash_value, ret->hash_len);

		return (INT8U *)ret;
	}

	case TLV_TYPE_BSS_CONFIG_REQUEST: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.84"
		struct BSSConfigRequestTLV *ret;
		INT8U *                     p;

		p = packet_stream + 1;

		ret           = (struct BSSConfigRequestTLV *)PLATFORM_MALLOC(sizeof(struct BSSConfigRequestTLV));
		ret->tlv_type = TLV_TYPE_BSS_CONFIG_REQUEST;
		_E2B(&p, &(ret->dpp_config_request_object_len));
		if (ret->dpp_config_request_object_len) {
			ret->dpp_config_request_object = (INT8U *)PLATFORM_MALLOC(ret->dpp_config_request_object_len);
			_EnB(&p, ret->dpp_config_request_object, ret->dpp_config_request_object_len);
		} else {
			ret->dpp_config_request_object = NULL;
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_BSS_CONFIG_RESPONSE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.85"
		struct BSSConfigResponseTLV *ret;
		INT8U *                      p;

		p = packet_stream + 1;

		ret           = (struct BSSConfigResponseTLV *)PLATFORM_MALLOC(sizeof(struct BSSConfigResponseTLV));
		ret->tlv_type = TLV_TYPE_BSS_CONFIG_RESPONSE;
		_E2B(&p, &(ret->dpp_config_object_len));
		if (ret->dpp_config_object_len) {
			ret->dpp_config_object = (INT8U *)PLATFORM_MALLOC(ret->dpp_config_object_len);
			_EnB(&p, ret->dpp_config_object, ret->dpp_config_object_len);
		} else {
			ret->dpp_config_object = NULL;
		}

		return (INT8U *)ret;
	}

	case TLV_TYPE_DPP_MESSAGE: {
		// This parsing is done according to the information detailed in
		// Multi-AP "Section 17.2.86"
		struct DPPMessageTLV *ret;
		INT8U *               p;

		p = packet_stream + 1;

		ret           = (struct DPPMessageTLV *)PLATFORM_MALLOC(sizeof(struct DPPMessageTLV));
		ret->tlv_type = TLV_TYPE_DPP_MESSAGE;
		_E2B(&p, &(ret->dpp_frame_len));
		ret->dpp_frame = (INT8U *)PLATFORM_MALLOC(ret->dpp_frame_len);
		_EnB(&p, ret->dpp_frame, ret->dpp_frame_len);

		return (INT8U *)ret;
	}
	case TLV_TYPE_ANTICIPATED_CHANNEL_PERFERENCE: {
		struct AnticipatedChannelPerferenceTLV *ret;
		struct OperatingChannelClass *operate_class;

		INT8U *p;
		INT16U len;

		ret = (struct AnticipatedChannelPerferenceTLV *)PLATFORM_MALLOC(sizeof(struct AnticipatedChannelPerferenceTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_ANTICIPATED_CHANNEL_PERFERENCE;

		_E1B(&p, &ret->operating_class_nr);
		ret->operate_class = (struct OperatingChannelClass *)PLATFORM_MALLOC(sizeof(struct OperatingChannelClass) * ret->operating_class_nr);
		int k;
		for (k = 0; k < ret->operating_class_nr; k++) {
			operate_class = &ret->operate_class[k];
			_E1B(&p, &operate_class->operating_class);
			_E1B(&p, &operate_class->channel_list_nr);
			operate_class->channel_list = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * operate_class->channel_list_nr);
			int m;
			for (m = 0; m < operate_class->channel_list_nr; m++) {
				_E1B(&p, &operate_class->channel_list[m]);
			}
			_EnB(&p, operate_class->reserved, 4);
		}
		return (INT8U *)ret;
	}
	case TLV_TYPE_ANTICIPATED_CHANNEL_USAGE: {
		struct AnticipatedChannelUsageTLV *ret;
		struct UsageEntryClass *usage_entry;

		INT8U *p;
		INT16U len;

		ret = (struct AnticipatedChannelUsageTLV *)PLATFORM_MALLOC(sizeof(struct AnticipatedChannelUsageTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_ANTICIPATED_CHANNEL_USAGE;

		_E1B(&p, &ret->operating_class);
		_E1B(&p, &ret->channel_number);
		_EnB(&p, ret->reference_bssid, 6);
		_E1B(&p, &ret->usage_entry_nr);
		ret->usage_entry = (struct UsageEntryClass *)PLATFORM_MALLOC(sizeof(struct UsageEntryClass) * ret->usage_entry_nr);
		int k;
		for (k = 0; k < ret->usage_entry_nr; k++) {
			usage_entry = &ret->usage_entry[k];
			_E4B(&p, &usage_entry->burst_start_time);
			_E4B(&p, &usage_entry->burst_length);
			_E4B(&p, &usage_entry->repititions);
			_E4B(&p, &usage_entry->burst_interval);
			_E1B(&p, &usage_entry->RU_bitmask_length);
			usage_entry->RU_bitmask = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * usage_entry->RU_bitmask_length);
			int m;
			for (m = 0; m < usage_entry->RU_bitmask_length; m++) {
				_E1B(&p, &usage_entry->RU_bitmask[m]);
			}
			_EnB(&p, usage_entry->transmitter_identifier, 6);
			_E1B(&p, &usage_entry->power_level);
			_E1B(&p, &usage_entry->channel_usage_reason);
			_EnB(&p, usage_entry->reserved, 4);
		}
		return (INT8U *)ret;
	}
	case TLV_TYPE_SPATIAL_REUSE_REQUEST: {
		struct SpatialReuseRequestTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct SpatialReuseRequestTLV *)PLATFORM_MALLOC(sizeof(struct SpatialReuseRequestTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_SPATIAL_REUSE_REQUEST;

		_EnB(&p, ret->ruid, 6);
		_E1B(&p, &ret->bss_color);
		_E1B(&p, &ret->valid_field);
		_E1B(&p, &ret->non_srg_obsspd_max_offset);
		_E1B(&p, &ret->srg_obsspd_min_offset);
		_E1B(&p, &ret->srg_obsspd_max_offset);
		_EnB(&p, ret->srg_bss_color_bitmap, 8);
		_EnB(&p, ret->srg_partial_bssid_bitmap, 8);
		_EnB(&p, ret->reserved, 2);
		return (INT8U *)ret;
	}
	case TLV_TYPE_SPATIAL_REUSE_REPORT: {
		struct SpatialReuseReportTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct SpatialReuseReportTLV *)PLATFORM_MALLOC(sizeof(struct SpatialReuseReportTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_SPATIAL_REUSE_REPORT;

		_EnB(&p, ret->ruid, 6);
		_E1B(&p, &ret->bss_color);
		_E1B(&p, &ret->valid_field);
		_E1B(&p, &ret->non_srg_obsspd_max_offset);
		_E1B(&p, &ret->srg_obsspd_min_offset);
		_E1B(&p, &ret->srg_obsspd_max_offset);
		_EnB(&p, ret->srg_bss_color_bitmap, 8);
		_EnB(&p, ret->srg_partial_bssid_bitmap, 8);
		_EnB(&p, ret->neighbor_bss_color_inuse_bitmap, 8);
		_EnB(&p, ret->reserved, 2);
		return (INT8U *)ret;
	}
	case TLV_TYPE_SPATIAL_REUSE_CONFIG_RESPONSE: {
		struct SpatialReuseConfigResponseTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct SpatialReuseConfigResponseTLV *)PLATFORM_MALLOC(sizeof(struct SpatialReuseConfigResponseTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_SPATIAL_REUSE_CONFIG_RESPONSE;

		_EnB(&p, ret->ruid, 6);
		_E1B(&p, &ret->response_code);
		return (INT8U *)ret;
	}
	case TLV_TYPE_QOS_MANAGEMENT_POLICY: {
		struct QoSManagementPolicyTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct QoSManagementPolicyTLV *)PLATFORM_MALLOC(sizeof(struct QoSManagementPolicyTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_QOS_MANAGEMENT_POLICY;

		_E1B(&p, &ret->mscsd_disallowed_sta_nr);
		ret->mscsd_disallowed_sta_mac_address = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(INT8U[6]) * ret->mscsd_disallowed_sta_nr);
		int k;
		for (k = 0; k < ret->mscsd_disallowed_sta_nr; k++) {
			_EnB(&p, ret->mscsd_disallowed_sta_mac_address[k], 6);
		}
		_E1B(&p, &ret->scsd_disallowed_sta_nr);
		ret->scsd_disallowed_sta_mac_address = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(INT8U[6]) * ret->scsd_disallowed_sta_nr);
		int m;
		for (m = 0; m < ret->scsd_disallowed_sta_nr; m++) {
			_EnB(&p, ret->scsd_disallowed_sta_mac_address[m], 6);
		}
		_EnB(&p, ret->reserved, 20);
		return (INT8U *)ret;
	}
	case TLV_TYPE_QOS_MANAGEMENT_DESCRIPTOR: {
		struct QoSManagementDescriptorTLV *ret;
		INT8U *p;
		INT16U len;
		INT8U temp_descriptor_element_len;

		ret = (struct QoSManagementDescriptorTLV *)PLATFORM_MALLOC(sizeof(struct QoSManagementDescriptorTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_QOS_MANAGEMENT_DESCRIPTOR;

		_E2B(&p, &ret->qmid);
		_EnB(&p, ret->bssid, 6);
		_EnB(&p, ret->client_mac, 6);

		p = p + 1; // to jump over the Element ID
		_E1B(&p, &temp_descriptor_element_len); //just need the
		ret->descriptor_element_len_1 = temp_descriptor_element_len + 2;
		p = p - 2; // to jump at the start of the descriptor element
		if (temp_descriptor_element_len) {
			ret->descriptor_element_1 = (INT8U *)PLATFORM_MALLOC(ret->descriptor_element_len_1);
			_EnB(&p, ret->descriptor_element_1, ret->descriptor_element_len_1);
		} else {
			_EnB(&p, ret->descriptor_element_1, 2);
		}

		p = p + 1; // to jump over the Element ID
		_E1B(&p, &temp_descriptor_element_len); //just need the
		ret->descriptor_element_len_2 = temp_descriptor_element_len + 2;
		p = p - 2; // to jump at the start of the descriptor element

		if (temp_descriptor_element_len) {
			ret->descriptor_element_2 = (INT8U *)PLATFORM_MALLOC(ret->descriptor_element_len_2);
			_EnB(&p, ret->descriptor_element_2, ret->descriptor_element_len_2);
		} else {
			_EnB(&p, ret->descriptor_element_2, 2);
		}
	
		return (INT8U *)ret;
	}

	case TLV_TYPE_CONTROLLER_CAPABILITY: {
		struct ControllerCapabilityTLV *ret;

		INT8U *p;
		INT16U len;

		ret = (struct ControllerCapabilityTLV *)PLATFORM_MALLOC(sizeof(struct ControllerCapabilityTLV));

		p = packet_stream + 1;
		_E2B(&p, &len);

		ret->tlv_type = TLV_TYPE_CONTROLLER_CAPABILITY;

		_E1B(&p, &ret->KiBMiB_counter);

		p = p + len -1;
		return (INT8U *)ret;
	}

	
	default: {
		// Ignore
		//
		return NULL;
	}
	}

	// This code cannot be reached
	//
	return NULL;
}

INT8U *forge_1905_TLV_from_structure(INT8U *memory_structure, INT16U *len, INT8U forwarding)
{
	if (NULL == memory_structure) {
		return NULL;
	}

	// The first byte of any of the valid structures is always the "tlv_type"
	// field.
	//
	switch (*memory_structure) {
	case TLV_TYPE_END_OF_MESSAGE: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.1"

		INT8U *                 ret, *p;
		struct endOfMessageTLV *m;

		INT16U tlv_length;

		m = (struct endOfMessageTLV *)memory_structure;

		tlv_length = 0;
		*len       = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		return ret;
	}

	case TLV_TYPE_VENDOR_SPECIFIC: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.2"

		INT8U *                   ret, *p;
		struct vendorSpecificTLV *m;

		INT16U tlv_length;

		m = (struct vendorSpecificTLV *)memory_structure;

		tlv_length = 3 + m->m_nr;
		*len       = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->vendorOUI[0], &p);
		_I1B(&m->vendorOUI[1], &p);
		_I1B(&m->vendorOUI[2], &p);
		_InB(m->m, &p, m->m_nr);

		return ret;
	}

	case TLV_TYPE_AL_MAC_ADDRESS_TYPE: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.3"

		INT8U *                     ret, *p;
		struct alMacAddressTypeTLV *m;

		INT16U tlv_length;

		m = (struct alMacAddressTypeTLV *)memory_structure;

		tlv_length = 6;
		*len       = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->al_mac_address, &p, 6);

		return ret;
	}

	case TLV_TYPE_MAC_ADDRESS_TYPE: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.4"

		INT8U *                   ret, *p;
		struct macAddressTypeTLV *m;

		INT16U tlv_length;

		m = (struct macAddressTypeTLV *)memory_structure;

		tlv_length = 6;
		*len       = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->mac_address, &p, 6);

		return ret;
	}

	case TLV_TYPE_DEVICE_INFORMATION_TYPE: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.5"

		INT8U *                          ret, *p;
		struct deviceInformationTypeTLV *m;

		INT16U tlv_length;

		INT8U i;

		m = (struct deviceInformationTypeTLV *)memory_structure;

		tlv_length = 7; // AL MAC (6 bytes) + number of ifaces (1 bytes)
		for (i = 0; i < m->local_interfaces_nr; i++) {
			tlv_length += 6 + 2 + 1; // MAC (6 bytes) + media type (2
			                         // bytes) + number of octets (1 byte)

			tlv_length += m->local_interfaces[i].media_specific_data_size;
		}
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->al_mac_address, &p, 6);
		_I1B(&m->local_interfaces_nr, &p);

		for (i = 0; i < m->local_interfaces_nr; i++) {
			_InB(m->local_interfaces[i].mac_address, &p, 6);
			_I2B(&m->local_interfaces[i].media_type, &p);
			_I1B(&m->local_interfaces[i].media_specific_data_size, &p);

			if (IS_IEEE_802_11_MEDIA(m->local_interfaces[i].media_type)) {
				INT8U aux;

				// Multi-AP Specification v3.0 Chapter 6, Table 5. Extension of 1905 Media type for 802.11AX
				// When media_type == 0x0108, media_specific_data_size must == 0 and no media specific data
				// We are compatible for some agent with media_specific_data_size == 10
				if (IS_IEEE_802_11AX_MEDIA(m->local_interfaces[i].media_type)
				    && 0 == m->local_interfaces[i].media_specific_data_size)
					continue;

				if (0 == m->local_interfaces[i].media_specific_data_size) {
					// allow 0 to pass due to Fiberhome customer issues.
					continue;
				}

				if (10 != m->local_interfaces[i].media_specific_data_size) {
					// Malformed structure
					//
					PLATFORM_FREE(ret);
					return NULL;
				}

				_InB(m->local_interfaces[i].media_specific_data.ieee80211.network_membership, &p, 6);
				aux = m->local_interfaces[i].media_specific_data.ieee80211.role << 4;
				_I1B(&aux, &p);
				_I1B(&m->local_interfaces[i].media_specific_data.ieee80211.ap_channel_band, &p);
				_I1B(&m->local_interfaces[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_1, &p);
				_I1B(&m->local_interfaces[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_2, &p);

			} else if (IS_IEEE_1901_MEDIA(m->local_interfaces[i].media_type)) {
				if (7 != m->local_interfaces[i].media_specific_data_size) {
					// Malformed structure
					//
					PLATFORM_FREE(ret);
					return NULL;
				}
				_InB(m->local_interfaces[i].media_specific_data.ieee1901.network_identifier, &p, 7);
			} else {
				if (0 != m->local_interfaces[i].media_specific_data_size) {
					// Malformed structure
					//
					PLATFORM_FREE(ret);
					return NULL;
				}
			}
		}

		return ret;
	}

	case TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.6"

		INT8U *                             ret, *p;
		struct deviceBridgingCapabilityTLV *m;

		INT16U tlv_length;

		INT8U i, j;

		m = (struct deviceBridgingCapabilityTLV *)memory_structure;

		tlv_length = 1; // number of bridging tuples (1 bytes)
		for (i = 0; i < m->bridging_tuples_nr; i++) {
			tlv_length += 1; // number of MAC addresses (1 bytes)
			tlv_length += 6 * m->bridging_tuples[i].bridging_tuple_macs_nr;
		}
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->bridging_tuples_nr, &p);

		for (i = 0; i < m->bridging_tuples_nr; i++) {
			_I1B(&m->bridging_tuples[i].bridging_tuple_macs_nr, &p);

			for (j = 0; j < m->bridging_tuples[i].bridging_tuple_macs_nr; j++) {
				_InB(m->bridging_tuples[i].bridging_tuple_macs[j].mac_address, &p, 6);
			}
		}

		return ret;
	}

	case TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.8"

		INT8U *                              ret, *p;
		struct non1905NeighborDeviceListTLV *m;

		INT16U tlv_length;

		INT8U i;

		m = (struct non1905NeighborDeviceListTLV *)memory_structure;

		tlv_length = 6 + 6 * m->non_1905_neighbors_nr;
		*len       = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->local_mac_address, &p, 6);

		for (i = 0; i < m->non_1905_neighbors_nr; i++) {
			_InB(m->non_1905_neighbors[i].mac_address, &p, 6);
		}

		return ret;
	}

	case TLV_TYPE_NEIGHBOR_DEVICE_LIST: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.9"

		INT8U *                       ret, *p;
		struct neighborDeviceListTLV *m;

		INT16U tlv_length;

		INT8U i;

		m = (struct neighborDeviceListTLV *)memory_structure;

		tlv_length = 6 + 7 * m->neighbors_nr;
		*len       = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->local_mac_address, &p, 6);

		for (i = 0; i < m->neighbors_nr; i++) {
			INT8U aux;

			_InB(m->neighbors[i].mac_address, &p, 6);

			if (1 == m->neighbors[i].bridge_flag) {
				aux = 1 << 7;
				_I1B(&aux, &p);
			} else {
				aux = 0;
				_I1B(&aux, &p);
			}
		}

		return ret;
	}

	case TLV_TYPE_LINK_METRIC_QUERY: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.10"

		INT8U *                    ret, *p;
		struct linkMetricQueryTLV *m;

		INT16U tlv_length;

		m = (struct linkMetricQueryTLV *)memory_structure;

		if (LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR == m->destination)
			tlv_length = 8;
		else
			tlv_length = 2;

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->destination, &p);

		if (LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR == m->destination) {
			_InB(m->specific_neighbor, &p, 6);
		} else {
			// INT8U empty_address[] = { m->link_metrics_type, 0x00, 0x00, 0x00, 0x00, 0x00 };
			//_InB(empty_address, &p, 6); //comment out by keith, 071118 as required by Multi AP Test Plan

			// Ugh? Why is the first value set to "m->link_metrics_type"
			// instead of "0x00"? What kind of black magic is this?
			//
			// Well... it turns out there is a reason for this. Take a
			// chair and let me explain.
			//
			// The original 1905 standard document (and also its later "1a"
			// update) describe the "metric query TLV" fields like this:
			//
			//   - Field #1: 1 octet set to "8" (tlv_type)
			//   - Field #2: 1 octet set to "8" (tlv_length)
			//   - Field #3: 1 octet set to "0" or "1" (destination)
			//   - Field #4: 6 octets set to the MAC address of a neighbour
			//               when field #3 is set "1"
			//   - Field #5: 1 octet set to "0", "1", "2" or "3" (link_
			//               _metrics_type)
			//
			// The problem is that we don't know what to put inside field
			// #4 when Field #3 is set to "0" ("all neighbors") instead of
			// "1" ("specific neighbor").
			//
			// A "reasonable" solution would be to set all bytes from field
			// #4 to "0x00". *However*, one could also think that the
			// correct thing to do is to not include the field at all (ie.
			// skip from field #3 to field #5).
			//
			// Now... this is actually insane. Typically protocols have a
			// fixed number of fields (whenever possible) to make it easier
			// for parsers (in fact, this would be the only exception to
			// this rule in the whole 1905 standard). Then... why would
			// someone think that not including field #4 is a good idea?
			//
			// Well... because this is what the "description" of field #3
			// reads on the standard:
			//
			//   "If the value is 0, then the EUI-48 field is not present;
			//    if the value is 1, then the EUI-48 field shall be present"
			//
			// ...and "not present" seems to imply not to include it
			// (although one could argue that it could also mean "set all
			// bytes to zero).
			//
			// I really think the standard means "set to zero" instead of
			// "not including it" (even if the wording seems to imply
			// otherwise). Why? For two reasons:
			//
			//   1. The standard says field #2 must *always* be "8" (and if
			//      field #4 could not be included, this value should be
			//      allowed to also take the value of 6)
			//
			//   2. There is no other place in the whole standard where a
			//      field can be present or not.
			//
			// Despite what I have just said, *some implementations* seem
			// to have taken the other route, and expect field #4 *not* to
			// be present (even if field #2 is set to "8"!!).
			//
			// When we send one "all neighbors" topology query to one of
			// these implementations they will interpret the first byte of
			// field #4 as the contents of field #5.
			//
			// And that's why when querying for all neighbors, because the
			// contents of field #4 don't really matter, we are going to
			// set its first byte to the same value as field #5.
			// This way all implementations, no matter how they decided to
			// interpret the standard, will work :)
		}

		_I1B(&m->link_metrics_type, &p);

		return ret;
	}

	case TLV_TYPE_TRANSMITTER_LINK_METRIC: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.11"

		INT8U *                          ret, *p;
		struct transmitterLinkMetricTLV *m;

		INT16U tlv_length;

		INT8U i;

		m = (struct transmitterLinkMetricTLV *)memory_structure;

		tlv_length = 12 + 29 * m->transmitter_link_metrics_nr;
		*len       = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->local_al_address, &p, 6);
		_InB(m->neighbor_al_address, &p, 6);

		for (i = 0; i < m->transmitter_link_metrics_nr; i++) {
			_InB(m->transmitter_link_metrics[i].local_interface_address, &p, 6);
			_InB(m->transmitter_link_metrics[i].neighbor_interface_address, &p, 6);
			_I2B(&m->transmitter_link_metrics[i].intf_type, &p);
			_I1B(&m->transmitter_link_metrics[i].bridge_flag, &p);
			_I4B(&m->transmitter_link_metrics[i].packet_errors, &p);
			_I4B(&m->transmitter_link_metrics[i].transmitted_packets, &p);
			_I2B(&m->transmitter_link_metrics[i].mac_throughput_capacity, &p);
			_I2B(&m->transmitter_link_metrics[i].link_availability, &p);
			_I2B(&m->transmitter_link_metrics[i].phy_rate, &p);
		}

		return ret;
	}

	case TLV_TYPE_RECEIVER_LINK_METRIC: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.12"

		INT8U *                       ret, *p;
		struct receiverLinkMetricTLV *m;

		INT16U tlv_length;

		INT8U i;

		m = (struct receiverLinkMetricTLV *)memory_structure;

		tlv_length = 12 + 23 * m->receiver_link_metrics_nr;
		*len       = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->local_al_address, &p, 6);
		_InB(m->neighbor_al_address, &p, 6);

		for (i = 0; i < m->receiver_link_metrics_nr; i++) {
			_InB(m->receiver_link_metrics[i].local_interface_address, &p, 6);
			_InB(m->receiver_link_metrics[i].neighbor_interface_address, &p, 6);
			_I2B(&m->receiver_link_metrics[i].intf_type, &p);
			_I4B(&m->receiver_link_metrics[i].packet_errors, &p);
			_I4B(&m->receiver_link_metrics[i].packets_received, &p);
			_I1B(&m->receiver_link_metrics[i].rssi, &p);
		}

		return ret;
	}

	case TLV_TYPE_LINK_METRIC_RESULT_CODE: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.13"

		INT8U *                         ret, *p;
		struct linkMetricResultCodeTLV *m;

		INT16U tlv_length;

		m = (struct linkMetricResultCodeTLV *)memory_structure;

		tlv_length = 1;
		*len       = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		if (!forwarding && (m->result_code != LINK_METRIC_RESULT_CODE_TLV_INVALID_NEIGHBOR)) {
			// Malformed structure
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		_I1B(&m->result_code, &p);

		return ret;
	}

	case TLV_TYPE_SEARCHED_ROLE: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.14"

		INT8U *                 ret, *p;
		struct searchedRoleTLV *m;

		INT16U tlv_length;

		m = (struct searchedRoleTLV *)memory_structure;

		tlv_length = 1;
		*len       = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		if (!forwarding && (m->role != IEEE80211_ROLE_REGISTRAR)) {
			// Malformed structure
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		_I1B(&m->role, &p);

		return ret;
	}

	case TLV_TYPE_AUTOCONFIG_FREQ_BAND: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.14"

		INT8U *                       ret, *p;
		struct autoconfigFreqBandTLV *m;

		INT16U tlv_length;

		m = (struct autoconfigFreqBandTLV *)memory_structure;

		tlv_length = 1;
		*len       = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		if (!forwarding && (m->freq_band != IEEE80211_FREQUENCY_BAND_2_4_GHZ) && (m->freq_band != IEEE80211_FREQUENCY_BAND_5_GHZ) && (m->freq_band != IEEE80211_FREQUENCY_BAND_60_GHZ)) {
			// Malformed structure
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		_I1B(&m->freq_band, &p);

		return ret;
	}

	case TLV_TYPE_SUPPORTED_ROLE: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.16"

		INT8U *                  ret, *p;
		struct supportedRoleTLV *m;

		INT16U tlv_length;

		m = (struct supportedRoleTLV *)memory_structure;

		tlv_length = 1;
		*len       = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		if (!forwarding && (m->role != IEEE80211_ROLE_REGISTRAR)) {
			// Malformed structure
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		_I1B(&m->role, &p);

		return ret;
	}

	case TLV_TYPE_SUPPORTED_FREQ_BAND: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.17"

		INT8U *                      ret, *p;
		struct supportedFreqBandTLV *m;

		INT16U tlv_length;

		m = (struct supportedFreqBandTLV *)memory_structure;

		tlv_length = 1;
		*len       = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		if (!forwarding && (m->freq_band != IEEE80211_FREQUENCY_BAND_2_4_GHZ) && (m->freq_band != IEEE80211_FREQUENCY_BAND_5_GHZ) && (m->freq_band != IEEE80211_FREQUENCY_BAND_60_GHZ)) {
			// Malformed structure
			//
			PLATFORM_FREE(ret);
			return NULL;
		}

		_I1B(&m->freq_band, &p);

		return ret;
	}

	case TLV_TYPE_WSC: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.18"

		INT8U *        ret, *p;
		struct wscTLV *m;

		INT16U tlv_length;

		m = (struct wscTLV *)memory_structure;

		tlv_length = m->wsc_frame_size;
		*len       = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->wsc_frame, &p, m->wsc_frame_size);

		return ret;
	}

	case TLV_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.19"

		INT8U *                                ret, *p;
		struct pushButtonEventNotificationTLV *m;

		INT16U tlv_length;

		INT8U i;

		m = (struct pushButtonEventNotificationTLV *)memory_structure;

		tlv_length = 1; // number of media types (1 byte)
		for (i = 0; i < m->media_types_nr; i++) {
			tlv_length += 2 + 1; //  media type (2 bytes) +
			                     //  number of octets (1 byte)

			tlv_length += m->media_types[i].media_specific_data_size;
		}
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->media_types_nr, &p);

		for (i = 0; i < m->media_types_nr; i++) {
			_I2B(&m->media_types[i].media_type, &p);
			_I1B(&m->media_types[i].media_specific_data_size, &p);

			if (IS_IEEE_802_11_MEDIA(m->media_types[i].media_type)) {
				INT8U aux;

				// Multi-AP Specification v3.0 Chapter 6, Table 5. Extension of 1905 Media type for 802.11AX
				// When media_type == 0x0108, media_specific_data_size must == 0 and no media specific data
				// We are compatible for some agent with media_specific_data_size == 10
				if (IS_IEEE_802_11AX_MEDIA(m->media_types[i].media_type)
				    && 0 == m->media_types[i].media_specific_data_size)
					continue;

				if (10 != m->media_types[i].media_specific_data_size) {
					// Malformed structure
					//
					PLATFORM_FREE(ret);
					return NULL;
				}

				_InB(m->media_types[i].media_specific_data.ieee80211.network_membership, &p, 6);
				aux = m->media_types[i].media_specific_data.ieee80211.role << 4;
				_I1B(&aux, &p);
				_I1B(&m->media_types[i].media_specific_data.ieee80211.ap_channel_band, &p);
				_I1B(&m->media_types[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_1, &p);
				_I1B(&m->media_types[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_2, &p);

			} else if (IS_IEEE_1901_MEDIA(m->media_types[i].media_type)) {
				if (7 != m->media_types[i].media_specific_data_size) {
					// Malformed structure
					//
					PLATFORM_FREE(ret);
					return NULL;
				}
				_InB(m->media_types[i].media_specific_data.ieee1901.network_identifier, &p, 7);
			} else {
				if (0 != m->media_types[i].media_specific_data_size) {
					// Malformed structure
					//
					PLATFORM_FREE(ret);
					return NULL;
				}
			}
		}

		return ret;
	}

	case TLV_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION: {
		// This forging is done according to the information detailed in
		// "IEEE Std 1905.1-2013 Section 6.4.20"

		INT8U *                               ret, *p;
		struct pushButtonJoinNotificationTLV *m;

		INT16U tlv_length;

		m = (struct pushButtonJoinNotificationTLV *)memory_structure;

		tlv_length = 20;
		*len       = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->al_mac_address, &p, 6);
		_I2B(&m->message_identifier, &p);
		_InB(m->mac_address, &p, 6);
		_InB(m->new_mac_address, &p, 6);

		return ret;
	}

	case TLV_TYPE_SUPPORTED_SERVICE: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.1"

		INT8U *                     ret, *p;
		struct SupportedServiceTLV *m;
		m = (struct SupportedServiceTLV *)memory_structure;

		INT8U  k;
		INT16U tlv_length = 0;

		/////////// Calculate TLV length begin////////////////////
		// supported_service_nr
		tlv_length += 1;

		// supported_service
		for (k = 0; k < m->supported_service_nr; k++) {
			tlv_length += 1;
		}

		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->supported_service_nr, &p);
		_InB(m->supported_service, &p, m->supported_service_nr);

		return ret;
	}

	case TLV_TYPE_SEARCHED_SERVICE: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.2"

		INT8U *                    ret, *p;
		struct SearchedServiceTLV *m;
		m = (struct SearchedServiceTLV *)memory_structure;

		INT8U  k;
		INT16U tlv_length = 0;

		/////////// Calculate TLV length begin////////////////////
		// supported_service_nr
		tlv_length += 1;

		// supported_service
		for (k = 0; k < m->searched_service_nr; k++) {
			tlv_length += 1;
		}

		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->searched_service_nr, &p);
		_InB(m->searched_service, &p, m->searched_service_nr);

		return ret;
	}

	case TLV_TYPE_AP_RADIO_IDENTIFIER: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.3"

		INT8U *                      ret, *p;
		struct ApRadioIdentifierTLV *m;
		m = (struct ApRadioIdentifierTLV *)memory_structure;

		INT16U tlv_length = 0;

		/////////// Calculate TLV length begin////////////////////
		// radio_unique_id
		tlv_length += 6;

		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->radio_unique_id, &p, 6);

		return ret;
	}

	case TLV_TYPE_AP_OPERATIONAL_BSS: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.4"

		INT8U *                     ret, *p;
		struct ApOperationalBSSTLV *m;
		m = (struct ApOperationalBSSTLV *)memory_structure;

		INT16U tlv_length = 0;
		INT8U  k1, m1;
		/////////// Calculate TLV length begin////////////////////
		// radios_nr
		tlv_length += 1;
		for (k1 = 0; k1 < m->radios_nr; k1++) {
			//  radio_unique_id, BSSs_nr
			tlv_length = tlv_length + 6 + 1;
			for (m1 = 0; m1 < m->radios[k1].BSSs_nr; m1++) {
				// mac_addr
				tlv_length += 6;
				// ssid_len
				tlv_length += 1;
				// ssid
				tlv_length += m->radios[k1].BSSs[m1].ssid_len;
			}
		}
		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->radios_nr, &p);
		for (k1 = 0; k1 < m->radios_nr; k1++) {
			_InB(m->radios[k1].radio_unique_id, &p, 6);
			_I1B(&m->radios[k1].BSSs_nr, &p);
			for (m1 = 0; m1 < m->radios[k1].BSSs_nr; m1++) {
				_InB(m->radios[k1].BSSs[m1].mac_addr, &p, 6);
				_I1B(&m->radios[k1].BSSs[m1].ssid_len, &p);
				_InB(m->radios[k1].BSSs[m1].ssid, &p, m->radios[k1].BSSs[m1].ssid_len);
			}
		}
		return ret;
	}
	case TLV_TYPE_ASSOCIATED_CLIENTS: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.5"

		INT8U *                      ret, *p;
		struct AssociatedClientsTLV *m;
		m = (struct AssociatedClientsTLV *)memory_structure;

		INT16U tlv_length = 0;
		INT8U  k1, m1;
		/////////// Calculate TLV length begin////////////////////
		// BSSs_nr
		tlv_length += 1;
		for (k1 = 0; k1 < m->BSSs_nr; k1++) {
			//  bssid, clients_nr
			tlv_length = tlv_length + 6 + 2;
			for (m1 = 0; m1 < m->BSSs[k1].clients_nr; m1++) {
				// mac_addr, left_time
				tlv_length = tlv_length + 6 + 2;
			}
		}
		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->BSSs_nr, &p);
		for (k1 = 0; k1 < m->BSSs_nr; k1++) {
			_InB(m->BSSs[k1].bssid, &p, 6);
			_I2B(&m->BSSs[k1].clients_nr, &p);
			for (m1 = 0; m1 < m->BSSs[k1].clients_nr; m1++) {
				_InB(&m->BSSs[k1].clients[m1].mac_addr, &p, 6);
				_I2B(&m->BSSs[k1].clients[m1].left_time, &p);
			}
		}
		return ret;
	}

	case TLV_TYPE_AP_CAPABILITY: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.6"

		INT8U *                 ret, *p;
		struct APCapabilityTLV *m;
		m = (struct APCapabilityTLV *)memory_structure;

		INT16U tlv_length = 0;

		tlv_length = 1;
		*len       = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->ap_capability, &p);
		//PLATFORM_PRINTF("%s AP capability: %d\n", __FUNCTION__, m->ap_capability);
		return ret;
	}

	case TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.7"

		INT8U *                             ret, *p;
		struct APRadioBasicCapabilitiesTLV *m;
		m = (struct APRadioBasicCapabilitiesTLV *)memory_structure;

		INT16U tlv_length = 0;
		INT8U  k1, m1;
		/////////// Calculate TLV length begin////////////////////
		// Radio UID
		tlv_length += 6;
		// Max BSS nr
		tlv_length += 1;
		// operating_classes_nr
		tlv_length += 1;
		for (k1 = 0; k1 < m->operating_classes_nr; k1++) {
			//  operating class, max transmit power
			tlv_length += 3;
			for (m1 = 0; m1 < m->operating_classes[k1].non_operable_channels_nr; m1++) {
				// non_operable_channel
				tlv_length += 1;
			}
		}
		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(&m->radio_unique_id, &p, 6);
		_I1B(&m->max_BSSs_nr, &p);
		_I1B(&m->operating_classes_nr, &p);

		for (k1 = 0; k1 < m->operating_classes_nr; k1++) {
			_I1B(&m->operating_classes[k1].operating_class, &p);
			_I1B(&m->operating_classes[k1].max_transmit_power, &p);
			_I1B(&m->operating_classes[k1].non_operable_channels_nr, &p);
			for (m1 = 0; m1 < m->operating_classes[k1].non_operable_channels_nr; m1++) {
				_I1B(&m->operating_classes[k1].non_operable_channels[m1], &p);
			}
		}
		return ret;
	}

	case TLV_TYPE_AP_HT_CAPABILITIES: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.8"

		INT8U *                     ret, *p;
		struct APHTCapabilitiesTLV *m;
		m = (struct APHTCapabilitiesTLV *)memory_structure;

		INT16U tlv_length = 0;

		tlv_length = 7;
		*len       = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(&m->radio_unique_id, &p, 6);
		_I1B(&m->ht_capability, &p);

		return ret;
	}

	case TLV_TYPE_AP_VHT_CAPABILITIES: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.9"

		INT8U *                      ret, *p;
		struct APVHTCapabilitiesTLV *m;
		m = (struct APVHTCapabilitiesTLV *)memory_structure;

		INT16U tlv_length = 0;

		tlv_length = 12;
		*len       = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(&m->radio_unique_id, &p, 6);
		_I2B(&m->vht_tx_msc, &p);
		_I2B(&m->vht_rx_msc, &p);
		_I1B(&m->vht_capability_1, &p);
		_I1B(&m->vht_capability_2, &p);

		return ret;
	}

	case TLV_TYPE_AP_HE_CAPABILITIES: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.10"
		INT8U *                     ret, *p;
		struct APHECapabilitiesTLV *m;
		m = (struct APHECapabilitiesTLV *)memory_structure;

		INT16U tlv_length = 0;

		tlv_length = 9 + m->he_msc_len;
		*len       = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->radio_unique_id, &p, 6);
		_I1B(&m->he_msc_len, &p);
		_InB(m->he_msc, &p, m->he_msc_len);
		_I1B(&m->he_capability_1, &p);
		_I1B(&m->he_capability_2, &p);

		return ret;
	}

	case TLV_TYPE_STEERING_POLICY: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.11"

		INT8U *                   ret, *p;
		struct SteeringPolicyTLV *m;
		m = (struct SteeringPolicyTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		// local_disallowed_sta_nr, local_disallowed_sta_mac(s)
		tlv_length = tlv_length + 1 + 6 * m->local_disallowed_sta_nr;
		// btm_disallowed_sta_nr, btm_disallowed_sta_mac(s)
		tlv_length = tlv_length + 1 + 6 * m->btm_disallowed_sta_nr;
		// radios_nr
		tlv_length = tlv_length + 1;
		// SteeringPolicy
		tlv_length = tlv_length + sizeof(struct SteeringPolicy) * m->radios_nr;

		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_I1B(&m->local_disallowed_sta_nr, &p);
		int h;
		for (h = 0; h < m->local_disallowed_sta_nr; h++) {
			_InB(m->local_disallowed_sta_mac[h], &p, 6);
		}

		_I1B(&m->btm_disallowed_sta_nr, &p);
		int k;
		for (k = 0; k < m->btm_disallowed_sta_nr; k++) {
			_InB(m->btm_disallowed_sta_mac[k], &p, 6);
		}

		_I1B(&m->radios_nr, &p);
		int m_;
		for (m_ = 0; m_ < m->radios_nr; m_++) {
			_InB(m->policies[m_].radio_unique_id, &p, 6);
			_I1B(&m->policies[m_].policy, &p);
			_I1B(&m->policies[m_].channel_util_threshold, &p);
			_I1B(&m->policies[m_].rcpi_steering_threshold, &p);
		}

		return ret;
	}

	case TLV_TYPE_METRIC_REPORT_POLICY: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.12"

		INT8U *                          ret, *p;
		struct MetricReportingPolicyTLV *m;
		m = (struct MetricReportingPolicyTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		// interval, radios_nr, policies
		tlv_length = tlv_length + 2 + sizeof(struct MetricReportingPolicy) * m->radios_nr;

		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_I1B(&m->report_interval, &p);
		_I1B(&m->radios_nr, &p);
		int h;
		for (h = 0; h < m->radios_nr; h++) {
			_InB(m->policies[h].radio_unique_id, &p, 6);
			_I1B(&m->policies[h].sta_rcpi_threshold, &p);
			_I1B(&m->policies[h].sta_rcpi_hysteresis_margin_override, &p);
			_I1B(&m->policies[h].ap_channel_utilization_threshold, &p);
			_I1B(&m->policies[h].policy, &p);
		}

		return ret;
	}

	case TLV_TYPE_CHANNEL_PREFERENCE: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.13"

		INT8U *                      ret, *p;
		struct ChannelPreferenceTLV *m;
		m = (struct ChannelPreferenceTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		// radio unique identifier
		tlv_length = tlv_length + 6;
		// number of opclass
		tlv_length = tlv_length + 1;
		// channel preferences including each opclass
		int h;
		for (h = 0; h < m->op_class_nr; h++) {
			tlv_length = tlv_length + m->channel_preferences[h].channel_nr + 3;
		}

		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_InB(m->radio_unique_id, &p, 6);
		_I1B(&m->op_class_nr, &p);
		for (h = 0; h < m->op_class_nr; h++) {
			_I1B(&m->channel_preferences[h].op_class, &p);
			_I1B(&m->channel_preferences[h].channel_nr, &p);
			if (m->channel_preferences[h].channel_nr)
				_InB(m->channel_preferences[h].channel_list, &p, m->channel_preferences[h].channel_nr);
			//int i;
			//for (i = 0; i < m->channel_preferences[h].channel_nr; i++) {
			//	_I1B(&m->channel_preferences[h].channel_list[i], &p);
			//}
			_I1B(&m->channel_preferences[h].preference_reason_code, &p);
		}

		return ret;
	}

	case TLV_TYPE_RADIO_OPERATION_RESTRICTION: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.14"

		INT8U *                              ret, *p;
		struct RadioOperationRestrictionTLV *m;
		m = (struct RadioOperationRestrictionTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		// radio unique identifier
		tlv_length = tlv_length + 6;
		// number of opclass
		tlv_length = tlv_length + 1;
		// radio operation restriction including each opclass
		int h;
		for (h = 0; h < m->op_class_nr; h++) {
			tlv_length = tlv_length + m->radio_operation_restriction[h].channel_nr * 2 + 2;
		}

		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_InB(m->radio_unique_id, &p, 6);
		_I1B(&m->op_class_nr, &p);
		for (h = 0; h < m->op_class_nr; h++) {
			_I1B(&m->radio_operation_restriction[h].op_class, &p);
			_I1B(&m->radio_operation_restriction[h].channel_nr, &p);
			int i;
			for (i = 0; i < m->radio_operation_restriction[h].channel_nr; i++) {
				_I1B(&m->radio_operation_restriction[h].channel_operation_restriction[i].channel, &p);
				_I1B(&m->radio_operation_restriction[h].channel_operation_restriction[i].min_freq_separation, &p);
			}
		}

		return ret;
	}

	case TLV_TYPE_TRANSMIT_POWER_LIMIT: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.15"

		INT8U *                       ret, *p;
		struct TransmitPowerLimitTLV *m;
		m = (struct TransmitPowerLimitTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		// radio unique identifier
		tlv_length = tlv_length + 6;
		// transmit power limit
		tlv_length = tlv_length + 1;

		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_InB(m->radio_unique_id, &p, 6);
		_I1B(&m->transmit_power_limit, &p);

		return ret;
	}

	case TLV_TYPE_CHANNEL_SELECTION_RESPONSE: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.16"

		INT8U *                             ret, *p;
		struct ChannelSelectionResponseTLV *m;
		m = (struct ChannelSelectionResponseTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		// radio unique identifier
		tlv_length = tlv_length + 6;
		// response
		tlv_length = tlv_length + 1;

		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_InB(m->radio_unique_id, &p, 6);
		_I1B(&m->response, &p);

		return ret;
	}

	case TLV_TYPE_OPERATING_CHANNEL_REPORT: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.17"

		INT8U *                           ret, *p;
		struct OperatingChannelReportTLV *m;
		m = (struct OperatingChannelReportTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		// radio unique identifier
		tlv_length = tlv_length + 6;
		// number of current opclass
		tlv_length = tlv_length + 1;
		// operating channels information including each opclass
		tlv_length = tlv_length + sizeof(struct OperatingChannelReportPerOpClass) * m->cur_op_class_nr;
		// current transmit power
		tlv_length = tlv_length + 1;

		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_InB(m->radio_unique_id, &p, 6);
		_I1B(&m->cur_op_class_nr, &p);
		int k;
		for (k = 0; k < m->cur_op_class_nr; k++) {
			_I1B(&m->operating_channels[k].op_class, &p);
			_I1B(&m->operating_channels[k].cur_channel, &p);
		}
		_I1B(&m->cur_transmit_power, &p);

		return ret;
	}

	case TLV_TYPE_CLIENT_INFO: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.18"
		INT8U *               ret, *p;
		struct ClientInfoTLV *m;
		m = (struct ClientInfoTLV *)memory_structure;

		INT16U tlv_length = 0;

		tlv_length = 12;
		*len       = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->bssid, &p, 6);
		_InB(m->mac_address, &p, 6);

		return ret;
	}

	case TLV_TYPE_CLIENT_CAPABLITY_REPORT: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.19"
		INT8U *                           ret, *p;
		struct ClientCapabilityReportTLV *m;
		m = (struct ClientCapabilityReportTLV *)memory_structure;

		INT16U tlv_length        = 0;
		INT16U  capability_length = m->frame_body_length;

		//PLATFORM_PRINTF("%s - capability length: %d\n", __FUNCTION__, capability_length);

		tlv_length = 1 + capability_length; //TODO: length
		*len       = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		//PLATFORM_PRINTF("%s - tlv_length: %d\n", __FUNCTION__, tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->result_code, &p);
		if (m->result_code == 0x00 && capability_length)
			_InB(m->frame_body, &p, capability_length);

		return ret;
	}

	case TLV_TYPE_CLIENT_ASSOCIATION_EVENT: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.20"

		INT8U *                           ret, *p;
		struct ClientAssociationEventTLV *m;
		m = (struct ClientAssociationEventTLV *)memory_structure;

		INT16U tlv_length = 0;

		/////////// Calculate TLV length begin////////////////////
		// MAC address, BSSID, Event
		tlv_length = 6 + 6 + 1;

		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->mac_address, &p, 6);
		_InB(m->bssid, &p, 6);
		_I1B(&m->event, &p);
		return ret;
	}

	case TLV_TYPE_AP_METRIC_QUERY: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.21"
		INT8U *                  ret, *p;
		INT8U                    k;
		struct APMetricQueryTLV *m;
		m = (struct APMetricQueryTLV *)memory_structure;

		INT16U tlv_length = 0;

		tlv_length = 1 + (sizeof(*(m->bssid)) * m->bssid_nr);
		*len       = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->bssid_nr, &p);
		for (k = 0; k < m->bssid_nr; k++) {
			_InB(m->bssid[k], &p, 6);
		}

		return ret;
	}

	case TLV_TYPE_AP_METRICS: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.22"
		INT8U *              ret, *p;
		struct APMetricsTLV *m;
		m = (struct APMetricsTLV *)memory_structure;

		INT16U tlv_length = 0;
		tlv_length        = 6 + 1 + 2 + 1 + 3;

		if (m->esp_ie & 64)
			tlv_length += 3;
		if (m->esp_ie & 32)
			tlv_length += 3;
		if (m->esp_ie & 16)
			tlv_length += 3;

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->bssid, &p, 6);
		_I1B(&m->ch_util, &p);
		_I2B(&m->sta_nr, &p);
		_I1B(&m->esp_ie, &p);
		_InB(m->esp_acbe, &p, 3);

		if (m->esp_ie & 64)
			_InB(m->esp_acbk, &p, 3);
		if (m->esp_ie & 32)
			_InB(m->esp_acvo, &p, 3);
		if (m->esp_ie & 16)
			_InB(m->esp_acvi, &p, 3);

		return ret;
	}

	case TLV_TYPE_STA_MAC_ADDRESS_TYPE: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.23"
		INT8U *                      ret, *p;
		struct STAMacAddressTypeTLV *m;
		m = (struct STAMacAddressTypeTLV *)memory_structure;

		INT16U tlv_length = 0;
		tlv_length        = 6;

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->mac_address, &p, 6);

		return ret;
	}

	case TLV_TYPE_ASSOCIATED_STA_LINK_METRICS: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.24"
		INT8U *                             ret, *p;
		INT8U                               k;
		struct AssociatedSTALinkMetricsTLV *m;
		m = (struct AssociatedSTALinkMetricsTLV *)memory_structure;

		INT16U tlv_length = 0;
		tlv_length        = 6 + 1;

		for (k = 0; k < m->bssid_nr; k++) {
			tlv_length += 6; //STA associated BSSID
			tlv_length += 4; //time delta
			tlv_length += 4; //downlink mac data rate
			tlv_length += 4; //uplink mac data rate
			tlv_length += 1; //uplink rssi
		}

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->mac_address, &p, 6);
		_I1B(&m->bssid_nr, &p);

		for (k = 0; k < m->bssid_nr; k++) {
			_InB(m->assoc_sta_link_metrics[k].bssid, &p, 6);
			_I4B(&m->assoc_sta_link_metrics[k].time_delta, &p);
			_I4B(&m->assoc_sta_link_metrics[k].dataRate_downlink, &p);
			_I4B(&m->assoc_sta_link_metrics[k].dataRate_uplink, &p);
			_I1B(&m->assoc_sta_link_metrics[k].uplink_rcpi, &p);
		}

		return ret;
	}

	case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.25"
		INT8U *                                    ret, *p;
		INT8U                                      k, j;
		struct UnassociatedSTALinkMetricsQueryTLV *m;
		m = (struct UnassociatedSTALinkMetricsQueryTLV *)memory_structure;

		INT16U tlv_length = 0;
		tlv_length        = 2; //Op class + Number of channel element
		for (k = 0; k < m->channel_list_nr; k++) {
			tlv_length += 2;                                              //Channel number + number of sta mac
			tlv_length += (MACADDRLEN * m->channel_list_entry[k].sta_nr); //mac addr len * sta nr
		}

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->op_class, &p);
		_I1B(&m->channel_list_nr, &p);
		if (m->channel_list_nr > 0) {
			for (k = 0; k < m->channel_list_nr; k++) {
				_I1B(&m->channel_list_entry[k].channel_nr, &p);
				_I1B(&m->channel_list_entry[k].sta_nr, &p);
				if (m->channel_list_entry[k].sta_nr > 0) {
					for (j = 0; j < m->channel_list_entry[k].sta_nr; j++) {
						_InB(m->channel_list_entry[k].sta_mac_address[j], &p, MACADDRLEN);
					}
				}
			}
		}
		return ret;
	}
	case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.26"
		INT8U *                                       ret, *p;
		INT8U                                         k;
		struct UnassociatedSTALinkMetricsResponseTLV *m;
		m = (struct UnassociatedSTALinkMetricsResponseTLV *)memory_structure;

		INT16U tlv_length = 0;
		tlv_length        = 1 + 1;
		for (k = 0; k < m->sta_nr; k++) {
			tlv_length += 6; //STA MAC
			tlv_length += 1; //Single channel number
			tlv_length += 4; //time delta
			tlv_length += 1; //uplink rssi
		}

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->op_class, &p);
		_I1B(&m->sta_nr, &p);

		if (m->sta_nr > 0) {
			for (k = 0; k < m->sta_nr; k++) {
				_InB(m->unassoc_sta_link_metrics[k].sta_mac_address, &p, 6);
				_I1B(&m->unassoc_sta_link_metrics[k].channel_number, &p);
				_I4B(&m->unassoc_sta_link_metrics[k].time_delta, &p);
				_I1B(&m->unassoc_sta_link_metrics[k].uplink_rcpi, &p);
			}
		}

		return ret;
	}
	case TLV_TYPE_BEACON_METRICS_QUERY: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.27"
		INT8U *                       ret, *p;
		INT8U                         k;
		struct BeaconMetricsQueryTLV *m;
		m = (struct BeaconMetricsQueryTLV *)memory_structure;

		INT16U tlv_length = 0;
		tlv_length        = 6 + 1 + 1 + 6 + 1; // MAC, Op class, Channel nr, BSSID, Report detail
		tlv_length += (1 + m->ssid_len);       //SSID len, SSID
		tlv_length += 1;
		for (k = 0; k < m->ap_channel_report_nr; k++) {
			tlv_length += 1;                                 //ap channel report len
			tlv_length += 1;                                 //op class
			tlv_length += (m->ap_channel_report[k].len - 1); //channel list
		}

		tlv_length += (1 + m->elementID_nr); //Element IDs

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_InB(m->mac_address, &p, 6);
		_I1B(&m->op_class, &p);
		_I1B(&m->channel_number, &p);
		_InB(m->bssid, &p, 6);
		_I1B(&m->report_detail, &p);
		_I1B(&m->ssid_len, &p);
		if (m->ssid_len > 0)
			_InB(m->ssid, &p, m->ssid_len);
		_I1B(&m->ap_channel_report_nr, &p);
		for (k = 0; k < m->ap_channel_report_nr; k++) {
			_I1B(&m->ap_channel_report[k].len, &p);
			_I1B(&m->ap_channel_report[k].op_class, &p);
			if (m->ap_channel_report[k].len > 1)
				_InB(m->ap_channel_report[k].channel_list, &p, (m->ap_channel_report[k].len - 1));
		}

		_I1B(&m->elementID_nr, &p);
		if (m->elementID_nr > 0)
			_InB(m->element_list, &p, m->elementID_nr);

		return ret;
	}
	case TLV_TYPE_BEACON_METRICS_RESPONSE: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.28"
		INT8U *                          ret, *p;
		INT8U                            k;
		struct BeaconMetricsResponseTLV *m;
		m = (struct BeaconMetricsResponseTLV *)memory_structure;

		INT16U tlv_length = 0;
		tlv_length        = 6 + 1 + 1; //mac address + reserved field + measurement report nr
		for (k = 0; k < m->measurement_report_nr; k++) {
			if (m->measurement_reports[k].measurementReportMode == 0) {
				tlv_length += 31; //exclude sublement id, sublement len, subelements
				if (m->measurement_reports[k].len > BEACON_REPORT_FIXED_IE) {
					tlv_length += 2; //include subelement id, subelement len
					tlv_length += m->measurement_reports[k].subelements_len;
					if (m->measurement_reports[k].subelements_remain_len > 0) {
						tlv_length += m->measurement_reports[k].subelements_remain_len;
					}
				}
			} else {
				if (m->measurement_reports[k].len > 3) {
					tlv_length += 31;
				} else {
					tlv_length += 5;
				}
				break;
			}
		}

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->mac_address, &p, 6);
		_I1B(&m->reserved_field, &p);
		_I1B(&m->measurement_report_nr, &p);

		for (k = 0; k < m->measurement_report_nr; k++) {
			_I1B(&m->measurement_reports[k].elementId, &p);
			_I1B(&m->measurement_reports[k].len, &p);
			_I1B(&m->measurement_reports[k].measurementToken, &p);
			_I1B(&m->measurement_reports[k].measurementReportMode, &p);
			_I1B(&m->measurement_reports[k].measurementType, &p);
			if (m->measurement_reports[k].measurementReportMode != 0) {
				if (m->measurement_reports[k].len <= 3)
					break;
			}
			_I1B(&m->measurement_reports[k].info.op_class, &p);
			_I1B(&m->measurement_reports[k].info.channel, &p);
			_I4B(&m->measurement_reports[k].info.measure_time_hi, &p);
			_I4B(&m->measurement_reports[k].info.measure_time_lo, &p);
			_I2B(&m->measurement_reports[k].info.measure_duration, &p);
			_I1B(&m->measurement_reports[k].info.frame_info, &p);
			_I1B(&m->measurement_reports[k].info.RCPI, &p);
			_I1B(&m->measurement_reports[k].info.RSNI, &p);
			_InB(&m->measurement_reports[k].info.bssid, &p, 6);
			_I1B(&m->measurement_reports[k].info.antenna_id, &p);
			_I4B(&m->measurement_reports[k].info.parent_tsf, &p);

			if (m->measurement_reports[k].len > BEACON_REPORT_FIXED_IE) {
				_I1B(&m->measurement_reports[k].subelement_id, &p);
				_I1B(&m->measurement_reports[k].subelements_len, &p);
				if (m->measurement_reports[k].subelements_len > 0)
					_InB(m->measurement_reports[k].subelements, &p, m->measurement_reports[k].subelements_len);
				if (m->measurement_reports[k].subelements_remain_len) {
					_InB(m->measurement_reports[k].subelements_remain, &p, m->measurement_reports[k].subelements_remain_len);
				}
			}
		}

		return ret;
	}

	case TLV_TYPE_STEERING_REQUEST: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.29"

		INT8U *                    ret, *p;
		struct SteeringRequestTLV *m;
		m = (struct SteeringRequestTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		// BSSID
		tlv_length = tlv_length + 6;
		// mode
		tlv_length = tlv_length + 1;
		// steering opportunity window
		tlv_length = tlv_length + 2;
		// BTM disassociation timer
		tlv_length = tlv_length + 2;
		// STA list count
		tlv_length = tlv_length + 1;
		// STA MAC address for which the steering request applies
		tlv_length = tlv_length + sizeof(INT8U[6]) * m->sta_nr;
		// target BSSID list count
		tlv_length = tlv_length + 1;
		// target BSS info
		tlv_length = tlv_length + sizeof(struct TargetBSS) * m->target_bss_nr;

		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_InB(m->bssid, &p, 6);
		_I1B(&m->mode, &p);
		_I2B(&m->window, &p);
		_I2B(&m->disassociation_timer, &p);
		_I1B(&m->sta_nr, &p);
		int k;
		for (k = 0; k < m->sta_nr; k++) {
			_InB(m->sta_mac_address[k], &p, 6);
		}
		_I1B(&m->target_bss_nr, &p);
		for (k = 0; k < m->target_bss_nr; k++) {
			_InB(m->target_bss[k].bssid, &p, 6);
			_I1B(&m->target_bss[k].opclass, &p);
			_I1B(&m->target_bss[k].channel, &p);
		}

		return ret;
	}

	case TLV_TYPE_STEERING_BTM_REPORT: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.30"

		INT8U *                      ret, *p;
		struct SteeringBTMReportTLV *m;
		m = (struct SteeringBTMReportTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		// BSSID
		tlv_length = tlv_length + 6;
		// STA MAC address
		tlv_length = tlv_length + 6;
		// BTM status code
		tlv_length = tlv_length + 1;
		// target BSSID
		tlv_length = tlv_length + 6;

		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_InB(m->bssid, &p, 6);
		_InB(m->sta_mac_address, &p, 6);
		_I1B(&m->status_code, &p);
		_InB(m->target_bssid, &p, 6);

		return ret;
	}

	case TLV_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.31"

		INT8U *                                    ret, *p;
		struct ClientAssociationControlRequestTLV *m;
		m = (struct ClientAssociationControlRequestTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		// BSSID
		tlv_length = tlv_length + 6;
		// association control
		tlv_length = tlv_length + 1;
		// validity period
		tlv_length = tlv_length + 2;
		// STA list count
		tlv_length = tlv_length + 1;
		// STA MAC address
		tlv_length = tlv_length + 6 * m->sta_nr;

		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->bssid, &p, 6);
		_I1B(&m->association_control, &p);
		_I2B(&m->validity_period, &p);
		_I1B(&m->sta_nr, &p);
		int k;
		for (k = 0; k < m->sta_nr; k++) {
			_InB(m->sta_mac_address[k], &p, 6);
		}

		return ret;
	}

	case TLV_TYPE_BACKHAUL_STEERING_REQUEST: {
		// This forging is done according to the information detailed in
		//	Multi-AP "Section 17.2.32"
		INT8U *                            ret, *p;
		struct BackhaulSteeringRequestTLV *m;
		m = (struct BackhaulSteeringRequestTLV *)memory_structure;

		INT16U tlv_length = 0;
		tlv_length        = 14;

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->backhaul_mac, &p, 6);
		_InB(m->target_bssid, &p, 6);
		_I1B(&m->op_class, &p);
		_I1B(&m->channel_number, &p);

		return ret;
	}

	case TLV_TYPE_BACKHAUL_STEERING_RESPONSE: {
		// This forging is done according to the information detailed in
		//	Multi-AP "Section 17.2.33"
		INT8U *                             ret, *p;
		struct BackhaulSteeringResponseTLV *m;
		m = (struct BackhaulSteeringResponseTLV *)memory_structure;

		INT16U tlv_length = 0;
		tlv_length        = 13;

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->backhaul_mac, &p, 6);
		_InB(m->target_bssid, &p, 6);
		_I1B(&m->result_code, &p);

		return ret;
	}

	case TLV_TYPE_HIGHER_LAYER_DATA: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.34"

		INT8U *                    ret, *p;
		struct HigherLayerDataTLV *m = (struct HigherLayerDataTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		// protocol
		tlv_length = tlv_length + 1;

		// Higher layer protocol payload (To be defined for specific higher layer protocol).
		INT16U payload_len = 0;
		// Todo: calculate the payload length
		tlv_length = tlv_length + payload_len;

		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->protocol, &p);
		_InB(m->payload, &p, payload_len);

		return ret;
	}

	case TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.35"
		INT8U *                              ret, *p;
		struct AssociatedSTATrafficStatsTLV *m;
		m = (struct AssociatedSTATrafficStatsTLV *)memory_structure;

		INT16U tlv_length = 0;
		tlv_length        = 34;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		*len = 1 + 2 + tlv_length;

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->sta_mac_address, &p, 6);
		_I4B(&m->bytes_sent, &p);
		_I4B(&m->bytes_received, &p);
		_I4B(&m->packets_sent, &p);
		_I4B(&m->packets_received, &p);
		_I4B(&m->tx_packets_errors, &p);
		_I4B(&m->rx_packets_errors, &p);
		_I4B(&m->retransmission_count, &p);

		return ret;
	}

	case TLV_TYPE_ERROR_CODE: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.36"

		INT8U *              ret, *p;
		struct ErrorCodeTLV *m;
		m = (struct ErrorCodeTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		// reason_code, sta_mac_address
		tlv_length = tlv_length + 1 + 6;

		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->reason_code, &p);
		if (m->reason_code == 0x01 || m->reason_code == 0x02 || m->reason_code == 0x03) {
			_InB(m->sta_mac_address, &p, 6);
		}

		return ret;
	}

		////////////////////////////////////////////////////////////////////////////////
		/// Profile 2 & Profile 3 TLVs
		////////////////////////////////////////////////////////////////////////////////

	case TLV_TYPE_CHANNEL_SCAN_REPORTING_POLICY: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.37"

		INT8U *                               ret, *p;
		struct ChannelScanReportingPolicyTLV *m;
		m = (struct ChannelScanReportingPolicyTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		// timestamp_length
		tlv_length += 1;
		// independent_report_flag
		tlv_length += m->independent_report_flag;
		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_I1B(&m->independent_report_flag, &p);

		return ret;
	}

	case TLV_TYPE_CHANNEL_SCAN_CAPABILITIES: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.38"

		INT8U *                            ret, *p;
		struct ChannelScanCapabilitiesTLV *m;
		m = (struct ChannelScanCapabilitiesTLV *)memory_structure;

		INT16U tlv_length = 0;
		INT8U  k, l;
		/////////// Calculate TLV length begin////////////////////
		tlv_length += 1; // radio_nr
		for (k = 0; k < m->radio_nr; k++) {
			tlv_length += 6; // radio_unique_identifier
			tlv_length += 1; // scan_capability_flags
			tlv_length += 4; // minimum_scan_interval
			tlv_length += 1; // op_class_nr
			for (l = 0; l < m->radios[k].op_class_nr; l++) {
				tlv_length += 1;                                     // op_class
				tlv_length += 1;                                     // channel_nr
				tlv_length += m->radios[k].op_classes[l].channel_nr; // channel_nr
			}
		}
		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_I1B(&m->radio_nr, &p);
		for (k = 0; k < m->radio_nr; k++) {
			_InB(m->radios[k].radio_unique_identifier, &p, 6);
			_I1B(&m->radios[k].scan_capability_flags, &p);
			_I4B(&m->radios[k].minimum_scan_interval, &p);
			_I1B(&m->radios[k].op_class_nr, &p);
			for (l = 0; l < m->radios[k].op_class_nr; l++) {
				_I1B(&m->radios[k].op_classes[l].op_class, &p);
				_I1B(&m->radios[k].op_classes[l].channel_nr, &p);
				_InB(m->radios[k].op_classes[l].channel_list, &p, m->radios[k].op_classes[l].channel_nr);
			}
		}

		return ret;
	}

	case TLV_TYPE_CHANNEL_SCAN_REQUEST: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.39"

		INT8U *                       ret, *p;
		struct ChannelScanRequestTLV *m;
		m = (struct ChannelScanRequestTLV *)memory_structure;

		INT16U tlv_length = 0;
		INT8U  k, l;
		/////////// Calculate TLV length begin////////////////////
		// flags (1 byte)
		tlv_length += 1;
		// radio nr
		tlv_length += 1;
		for (k = 0; k < m->target_radio_nr; k++) {
			//  radio id
			tlv_length += 6;
			//  op_class nr
			tlv_length += 1;
			for (l = 0; l < m->target_radios[k].target_op_class_nr; l++) {
				// op_class
				tlv_length += 1;
				// channel nr
				tlv_length += 1;
				// channel list
				tlv_length += m->target_radios[k].target_op_classes[l].target_channel_nr * sizeof(INT8U);
			}
		}
		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->flags, &p);
		_I1B(&m->target_radio_nr, &p);

		for (k = 0; k < m->target_radio_nr; k++) {
			//  radio id
			_InB(m->target_radios[k].radio_unique_identifier, &p, 6);
			//  op_class nr
			_I1B(&m->target_radios[k].target_op_class_nr, &p);
			for (l = 0; l < m->target_radios[k].target_op_class_nr; l++) {
				// op_class
				_I1B(&m->target_radios[k].target_op_classes[l].op_class, &p);
				// channel nr
				_I1B(&m->target_radios[k].target_op_classes[l].target_channel_nr, &p);
				// channel list
				if (m->target_radios[k].target_op_classes[l].target_channel_nr > 0) {
					_InB(m->target_radios[k].target_op_classes[l].target_channel_list, &p, m->target_radios[k].target_op_classes[l].target_channel_nr);
				}
			}
		}
		return ret;
	}

	case TLV_TYPE_CHANNEL_SCAN_RESULT: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.40"

		INT8U *                      ret, *p;
		struct ChannelScanResultTLV *m;
		m = (struct ChannelScanResultTLV *)memory_structure;

		INT16U tlv_length = 0;
		INT8U  k;
		/////////// Calculate TLV length begin////////////////////
		tlv_length += 6; // radio id
		tlv_length += 1; // op class
		tlv_length += 1; // channel
		tlv_length += 1; // scan status
		//The following fields are only present if Scan Status is set to 0x00
		if (!m->scan_status) {
			tlv_length += 1;                   // timestamp length
			tlv_length += m->timestamp_length; //timestamp (excluding null termination)
			tlv_length += 1;                   // utilazation
			tlv_length += 1;                   // noise
			tlv_length += 2;                   // neighbor nr
			for (k = 0; k < m->neighbor_nr; k++) {
				tlv_length += 6;                                        // bssid
				tlv_length += 1;                                        // ssid length
				tlv_length += m->neighbors[k].ssid_length;              // ssid
				tlv_length += 1;                                        // signal strength
				tlv_length += 1;                                        // channel bandwidth length
				tlv_length += m->neighbors[k].channel_bandwidth_length; // channel bandwidth length
				tlv_length += 1;                                        // field_present_flags
				if (CHANNELUTLIZATION_PRESENT_MASK & m->neighbors[k].field_present_flags)
					tlv_length += 1; // channel_utilization
				if (STATIONCOUNT_PRESENT_MASK & m->neighbors[k].field_present_flags)
					tlv_length += 2; // station_count
			}
			tlv_length += 4; // aggregate_scan_duration
			tlv_length += 1; // scan_type_flags
		}
		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_InB(m->radio_unique_identifier, &p, 6);
		_I1B(&m->op_class, &p);
		_I1B(&m->channel, &p);
		_I1B(&m->scan_status, &p);
		//The following fields are only present if Scan Status is set to 0x00
		if (!m->scan_status) {
			_I1B(&m->timestamp_length, &p);
			_InB(m->timestamp, &p, m->timestamp_length);
			_I1B(&m->channel_utilization, &p);
			_I1B(&m->noise, &p);
			_I2B(&m->neighbor_nr, &p);
			for (k = 0; k < m->neighbor_nr; k++) {
				_InB(m->neighbors[k].bssid, &p, 6);
				_I1B(&m->neighbors[k].ssid_length, &p);
				_InB(m->neighbors[k].ssid, &p, m->neighbors[k].ssid_length);
				_I1B(&m->neighbors[k].signal_strength, &p);
				_I1B(&m->neighbors[k].channel_bandwidth_length, &p);
				_InB(m->neighbors[k].channel_bandwidth, &p, m->neighbors[k].channel_bandwidth_length);
				_I1B(&m->neighbors[k].field_present_flags, &p);
				if (CHANNELUTLIZATION_PRESENT_MASK & m->neighbors[k].field_present_flags)
					_I1B(&m->neighbors[k].channel_utilization, &p);
				if (STATIONCOUNT_PRESENT_MASK & m->neighbors[k].field_present_flags)
					_I2B(&m->neighbors[k].station_count, &p);
			}
			_InB(&m->aggregate_scan_duration, &p, 4);
			_I1B(&m->scan_type_flags, &p);
		}

		return ret;
	}

	case TLV_TYPE_TIMESTAMP: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.41"

		INT8U *              ret, *p;
		struct TimestampTLV *m;
		m = (struct TimestampTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		// timestamp_length
		tlv_length += 1;
		// radio nr
		tlv_length += m->timestamp_length;
		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_I1B(&m->timestamp_length, &p);
		_InB(m->timestamp, &p, m->timestamp_length);

		return ret;
	}

	case TLV_TYPE_CAC_REQUEST: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.42"

		INT8U *               ret, *p;
		struct CACRequestTLV *m;
		m = (struct CACRequestTLV *)memory_structure;

		INT16U tlv_length = 0;
		INT8U  k;
		/////////// Calculate TLV length begin////////////////////
		// radio nr
		tlv_length += 1;
		for (k = 0; k < m->radio_nr; k++) {
			//  radio id
			tlv_length += 6;
			//  op_class
			tlv_length += 1;
			//  channel
			tlv_length += 1;
			//  flags
			tlv_length += 1;
		}
		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->radio_nr, &p);

		for (k = 0; k < m->radio_nr; k++) {
			//  radio id
			_InB(m->radios[k].radio_unique_identifier, &p, 6);
			//  op_class nr
			_I1B(&m->radios[k].op_class, &p);
			//  channel
			_I1B(&m->radios[k].channel, &p);
			//  flags
			_I1B(&m->radios[k].flags, &p);
		}
		return ret;
	}

	case TLV_TYPE_CAC_TERMINATION: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.43"

		INT8U *                   ret, *p;
		struct CACTerminationTLV *m;
		m = (struct CACTerminationTLV *)memory_structure;

		INT16U tlv_length = 0;
		INT8U  k;
		/////////// Calculate TLV length begin////////////////////
		// radio nr
		tlv_length += 1;
		for (k = 0; k < m->radio_nr; k++) {
			//  radio id
			tlv_length += 6;
			//  op_class
			tlv_length += 1;
			//  channel
			tlv_length += 1;
		}
		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->radio_nr, &p);

		for (k = 0; k < m->radio_nr; k++) {
			//  radio id
			_InB(m->radios[k].radio_unique_identifier, &p, 6);
			//  op_class nr
			_I1B(&m->radios[k].op_class, &p);
			//  channel
			_I1B(&m->radios[k].channel, &p);
		}
		return ret;
	}

	case TLV_TYPE_CAC_COMPLETION_REPORT: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.47"

		INT8U *                        ret, *p;
		struct CACCompletionReportTLV *m;
		m = (struct CACCompletionReportTLV *)memory_structure;

		INT16U tlv_length = 0;
		INT8U  k, l;
		/////////// Calculate TLV length begin////////////////////
		// radio nr
		tlv_length += 1;
		for (k = 0; k < m->radio_nr; k++) {
			//  radio id
			tlv_length += 6;
			//  op_class
			tlv_length += 1;
			//  channel
			tlv_length += 1;
			//  flags
			tlv_length += 1;
			//  pairs_nr
			tlv_length += 1;
			for (l = 0; l < m->radios[k].pairs_nr; l++) {
				//  pairs_op_class
				tlv_length += 1;
				//  pairs_channel
				tlv_length += 1;
			}
		}
		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->radio_nr, &p);

		for (k = 0; k < m->radio_nr; k++) {
			//  radio id
			_InB(m->radios[k].radio_unique_identifier, &p, 6);
			//  op_class nr
			_I1B(&m->radios[k].op_class, &p);
			//  channel
			_I1B(&m->radios[k].channel, &p);
			//  flags
			_I1B(&m->radios[k].flags, &p);
			//  pairs_nr
			_I1B(&m->radios[k].pairs_nr, &p);
			for (l = 0; l < m->radios[k].pairs_nr; l++) {
				//  pairs_op_class
				_I1B(&m->radios[k].pairs[l].pairs_op_class, &p);
				//  pairs_channel
				_I1B(&m->radios[k].pairs[l].pairs_channel, &p);
			}
		}
		return ret;
	}

	case TLV_TYPE_CAC_STATUS_REPORT: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.45"

		INT8U *                    ret, *p;
		struct CACStatusReportTLV *m;
		m = (struct CACStatusReportTLV *)memory_structure;

		INT16U tlv_length = 0;
		INT8U  k;
		/////////// Calculate TLV length begin////////////////////
		// available_channel_nr
		tlv_length += 1;
		for (k = 0; k < m->available_channel_nr; k++) {
			//  ac_op_class
			tlv_length += 1;
			//  ac_channel
			tlv_length += 1;
			//  identify_time
			tlv_length += 2;
		}
		// nonoccup_pair_nr
		tlv_length += 1;
		for (k = 0; k < m->nonoccup_pair_nr; k++) {
			//  nonoccup_op_class
			tlv_length += 1;
			//  nonoccup_channel
			tlv_length += 1;
			//  nonoccup_remaining_time
			tlv_length += 2;
		}
		// active_pair_nr
		tlv_length += 1;
		for (k = 0; k < m->active_pair_nr; k++) {
			//  active_op_class
			tlv_length += 1;
			//  active_channel
			tlv_length += 1;
			//  active_remaining_time
			tlv_length += 3;
		}
		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_I1B(&m->available_channel_nr, &p);

		for (k = 0; k < m->available_channel_nr; k++) {
			//  ac_op_class
			_I1B(&m->available_channels[k].ac_op_class, &p);
			//  ac_channel
			_I1B(&m->available_channels[k].ac_channel, &p);
			//  identify_time
			_I2B(&m->available_channels[k].identify_time, &p);
		}

		_I1B(&m->nonoccup_pair_nr, &p);

		for (k = 0; k < m->nonoccup_pair_nr; k++) {
			//  nonoccup_op_class
			_I1B(&m->nonoccup_pairs[k].nonoccup_op_class, &p);
			//  nonoccup_channel
			_I1B(&m->nonoccup_pairs[k].nonoccup_channel, &p);
			//  nonoccup_remaining_time
			_I2B(&m->nonoccup_pairs[k].nonoccup_remaining_time, &p);
		}

		_I1B(&m->active_pair_nr, &p);

		for (k = 0; k < m->active_pair_nr; k++) {
			//  active_op_class
			_I1B(&m->active_pairs[k].active_op_class, &p);
			//  active_channel
			_I1B(&m->active_pairs[k].active_channel, &p);
			//  active_remaining_time
			_InB(m->active_pairs[k].active_remaining_time, &p, 3);
		}
		return ret;
	}

	case TLV_TYPE_CAC_CAPABILITIES: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.46"

		INT8U *                    ret, *p;
		struct CACCapabilitiesTLV *m;
		m = (struct CACCapabilitiesTLV *)memory_structure;

		INT16U tlv_length = 0;
		INT8U  k, l, n;

		/////////// Calculate TLV length begin////////////////////
		// country_code
		tlv_length += 2;
		// radio_nr
		tlv_length += 1;

		for (k = 0; k < m->radio_nr; k++) {
			//  radio id
			tlv_length += 6;
			//  type_nr
			tlv_length += 1;
			for (l = 0; l < m->radios[k].type_nr; l++) {
				//  flags
				tlv_length += 1;
				//  second_nr
				tlv_length += 3;
				//  op_class_nr
				tlv_length += 1;
				for (n = 0; n < m->radios[k].types[l].op_class_nr; n++) {
					//  op_class
					tlv_length += 1;
					//  channel_nr
					tlv_length += 1;
					//  channels
					tlv_length += m->radios[k].types[l].op_classes[n].channel_nr * sizeof(INT8U);
				}
			}
		}
		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I2B((INT16U *)m->country_code, &p);

		_I1B(&m->radio_nr, &p);

		for (k = 0; k < m->radio_nr; k++) {
			//  radio id
			_InB(m->radios[k].radio_unique_identifier, &p, 6);
			//  type_nr
			_I1B(&m->radios[k].type_nr, &p);
			for (l = 0; l < m->radios[k].type_nr; l++) {
				//  flags
				_I1B(&m->radios[k].types[l].flags, &p);
				//  second_nr
				_InB(m->radios[k].types[l].second_nr, &p, 3);
				//  op_class_nr
				_I1B(&m->radios[k].types[l].op_class_nr, &p);
				for (n = 0; n < m->radios[k].types[l].op_class_nr; n++) {
					//  op_class
					_I1B(&m->radios[k].types[l].op_classes[n].op_class, &p);
					//  channel_nr
					_I1B(&m->radios[k].types[l].op_classes[n].channel_nr, &p);
					if (m->radios[k].types[l].op_classes[n].channel_nr > 0) {
						_InB(m->radios[k].types[l].op_classes[n].channels, &p, m->radios[k].types[l].op_classes[n].channel_nr);
					}
				}
			}
		}
		return ret;
	}

	case TLV_TYPE_MULTIAP_PROFILE: {
		// This forging is done according to the information detailed in
		//	Multi-AP "Section 17.2.47"
		INT8U *                   ret, *p;
		struct MultiAPProfileTLV *m;
		m = (struct MultiAPProfileTLV *)memory_structure;

		INT16U tlv_length = 1;

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->profile, &p);

		return ret;
	}

	case TLV_TYPE_PROFILE_2_CAPABILITY: {
		// This forging is done according to the information detailed in
		//	Multi-AP "Section 17.2.48"
		INT8U *                         ret, *p;
		struct Profile2APCapabilityTLV *m;
		m = (struct Profile2APCapabilityTLV *)memory_structure;

		INT16U tlv_length = 0;
		// INT8U  k;

		/////////// Calculate TLV length begin////////////////////

		// max_total_nr_service_prioritization_rules
		tlv_length += 1;
		// reserved
		tlv_length += 1;
		// units
		tlv_length += 1;
		// max_total_nr_VIDs
		tlv_length += 1;

		/////////// Calculate TLV length end////////////////////
		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->max_total_nr_service_prioritization_rules, &p);
		_I1B(&m->reserved, &p);
		_I1B(&m->units, &p);
		_I1B(&m->max_total_nr_VIDs, &p);

		return ret;
	}

	case TLV_TYPE_DEFAULT_802_1Q_SETTINGS: {
		// This forging is done according to the information detailed in
		//	Multi-AP "Section 17.2.49"
		INT8U *                         ret, *p;
		struct Default8021QSettingsTLV *m;
		m = (struct Default8021QSettingsTLV *)memory_structure;

		INT16U tlv_length = 3;

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I2B(&m->primary_vlan_id, &p);
		_I1B(&m->default_pcp, &p);

		return ret;
	}

	case TLV_TYPE_TRAFFIC_SEPARATION_POLICY: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.50"
		int                                h;
		INT8U *                            ret, *p;
		struct TrafficSeparationPolicyTLV *m;
		m = (struct TrafficSeparationPolicyTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length = 1; //ssid_nr
		for (h = 0; h < m->ssid_nr; h++) {
			tlv_length += 1;                           // ssid length
			tlv_length += m->ssid_info[h].ssid_length; // ssid_name
			tlv_length += 2;                           // vlan id
		}
		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_I1B(&m->ssid_nr, &p);

		for (h = 0; h < m->ssid_nr; h++) {
			_I1B(&m->ssid_info[h].ssid_length, &p);
			_InB(m->ssid_info[h].ssid_name, &p, m->ssid_info[h].ssid_length);
			_I2B(&m->ssid_info[h].vlan_id, &p);
		}
		return ret;
	}

	case TLV_TYPE_PROFILE_2_ERROR_CODE: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.51"
		INT8U *                      ret, *p;
		struct Profile2ErrorCodeTLV *m;
		m = (struct Profile2ErrorCodeTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length += 1; //reason_code
		tlv_length += 4; //service_prioritization_rule_id

		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->reason_code, &p);
		_I4B(&m->service_prioritization_rule_id, &p);

		return ret;
	}

	case TLV_TYPE_AP_RADIO_ADVANCED_CAPABILITIES: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.52"
		INT8U *                                ret, *p;
		struct APRadioAdvancedCapabilitiesTLV *m;
		m = (struct APRadioAdvancedCapabilitiesTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length += 6; //radio_id
		tlv_length += 1; //backhaul_bss_traffic_separation_mix_no_support

		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->radio_id, &p, 6);
		_I1B(&m->backhaul_bss_traffic_separation_mix_no_support, &p);

		return ret;
	}

	case TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.53"
		int                                      h;
		INT8U *                                  ret, *p;
		struct AssociationStatusNotificationTLV *m;
		m = (struct AssociationStatusNotificationTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length = 1; //bssid nr
		for (h = 0; h < m->bssid_nr; h++) {
			tlv_length += 6; // bssid
			tlv_length += 1; // association_allowance_status type
		}
		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_I1B(&m->bssid_nr, &p);

		for (h = 0; h < m->bssid_nr; h++) {
			_InB(m->bss_info[h].bssid, &p, 6);
			_I1B(&m->bss_info[h].association_allowance_status, &p);
		}
		return ret;
	}

	case TLV_TYPE_SOURCE_INFO: {
		// This forging is done according to the information detailed in
		//	Multi-AP "Section 17.2.54"
		INT8U *               ret, *p;
		struct SourceInfoTLV *m;
		m = (struct SourceInfoTLV *)memory_structure;

		INT16U tlv_length = 6;

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->mac_address, &p, 6);

		return ret;
	}

	case TLV_TYPE_TUNNELED_MESSAGE_TYPE: {
		// This forging is done according to the information detailed in
		//	Multi-AP "Section 17.2.55"
		INT8U *                        ret, *p;
		struct TunneledMessageTypeTLV *m;
		m = (struct TunneledMessageTypeTLV *)memory_structure;

		INT16U tlv_length = 1;

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->tunneled_protocol_type_payload, &p);

		return ret;
	}

	case TLV_TYPE_TUNNELED: {
		// This forging is done according to the information detailed in
		//	Multi-AP "Section 17.2.56"
		INT8U *             ret, *p;
		struct TunneledTLV *m;
		m = (struct TunneledTLV *)memory_structure;

		INT16U tlv_length = m->tlv_length;

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->payload, &p, tlv_length);

		return ret;
	}

	case TLV_TYPE_PROFILE_2_STEERING_REQUEST: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.57"
		int                                h;
		INT8U *                            ret, *p;
		struct Profile2SteeringRequestTLV *m;
		m = (struct Profile2SteeringRequestTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length = (6 + 1 + 2 + 2 + 1 + 1); //bssid, steer parameter, steer opporutnity window, btm_disassociation_timer, sta_list_count, target_bssid_count
		tlv_length += (6 * m->sta_list_count);
		tlv_length += (9 * m->target_bssid_list_count);
		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_InB(m->bssid, &p, 6);
		_I1B(&m->steer_parameters, &p);
		_I2B(&m->steer_opportunity_window, &p);
		_I2B(&m->btm_disassociation_timer, &p);
		_I1B(&m->sta_list_count, &p);

		for (h = 0; h < m->sta_list_count; h++) {
			_InB(m->sta_list[h], &p, 6);
		}

		_I1B(&m->target_bssid_list_count, &p);

		for (h = 0; h < m->target_bssid_list_count; h++) {
			_InB(m->target_bssid_list[h].target_bssid, &p, 6);
			_I1B(&m->target_bssid_list[h].target_bss_operating_class, &p);
			_I1B(&m->target_bssid_list[h].target_bss_channel_number, &p);
			_I1B(&m->target_bssid_list[h].reason_code, &p);
		}

		return ret;
	}

	case TLV_TYPE_UNSUCCESSFUL_ASSOCIATION_POILCY: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.58"
		INT8U *                                  ret, *p;
		struct UnsuccessfulAssociationPolicyTLV *m;
		m = (struct UnsuccessfulAssociationPolicyTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length += 1; //report_unsuccessful_associations
		tlv_length += 4; //maximum_reporting_rate
		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_I1B(&m->report_unsuccessful_associations, &p);
		_I4B(&m->maximum_reporting_rate, &p);

		return ret;
	}

	case TLV_TYPE_METRIC_COLLECTION_INTERVAL: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.59"
		INT8U *                             ret, *p;
		struct MetricCollectionIntervalTLV *m;
		m = (struct MetricCollectionIntervalTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length += 4; //collection_interval
		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_I4B(&m->collection_interval, &p);

		return ret;
	}

	case TLV_TYPE_RADIO_METRICS: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.60"
		INT8U *                 ret, *p;
		struct RadioMetricsTLV *m;
		m = (struct RadioMetricsTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length += 6; //radio_unique_identifier
		tlv_length += 1; //noise
		tlv_length += 1; //transmit
		tlv_length += 1; //receiveself
		tlv_length += 1; //receiveother

		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_InB(m->radio_unique_identifier, &p, 6);
		_I1B(&m->noise, &p);
		_I1B(&m->transmit, &p);
		_I1B(&m->receiveself, &p);
		_I1B(&m->receiveother, &p);

		return ret;
	}

	case TLV_TYPE_AP_EXTENDED_METRICS: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.61"
		INT8U *                      ret, *p;
		struct APExtendedMetricsTLV *m;
		m = (struct APExtendedMetricsTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length += 6; //radio_unique_identifier
		tlv_length += 4; //unicast_byte_sent
		tlv_length += 4; //unicast_byte_received
		tlv_length += 4; //multicast_byte_sent
		tlv_length += 4; //multicast_byte_received
		tlv_length += 4; //broadcast_byte_sent
		tlv_length += 4; //broadcast_byte_received

		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_InB(m->radio_unique_identifier, &p, 6);
		_I4B(&m->unicast_byte_sent, &p);
		_I4B(&m->unicast_byte_received, &p);
		_I4B(&m->multicast_byte_sent, &p);
		_I4B(&m->multicast_byte_received, &p);
		_I4B(&m->broadcast_byte_sent, &p);
		_I4B(&m->broadcast_byte_received, &p);

		return ret;
	}

	case TLV_TYPE_ASSOCIATED_STA_EXTENDED_LINK_METRICS: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.62"
		INT8U *                                    ret, *p;
		int                                        h;
		struct AssociatedSTAExtendedLinkMericsTLV *m;
		m = (struct AssociatedSTAExtendedLinkMericsTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length += 6; //sta_mac_address
		tlv_length += 1; //bssid_nr
		for (h = 0; h < m->bssid_nr; h++) {
			tlv_length += 6; //bssid
			tlv_length += 4; //last_data_downlink_rate
			tlv_length += 4; //last_data_uplink_rate
			tlv_length += 4; //utilization_receive
			tlv_length += 4; //utilization_transmit
		}

		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_InB(m->sta_mac_address, &p, 6);
		_I1B(&m->bssid_nr, &p);
		for (h = 0; h < m->bssid_nr; h++) {
			_InB(m->reported_bss[h].bssid, &p, 6);
			_I4B(&m->reported_bss[h].last_data_downlink_rate, &p);
			_I4B(&m->reported_bss[h].last_data_uplink_rate, &p);
			_I4B(&m->reported_bss[h].utilization_receive, &p);
			_I4B(&m->reported_bss[h].utilization_transmit, &p);
		}

		return ret;
	}

	case TLV_TYPE_STATUS_CODE: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.63"
		INT8U *               ret, *p;
		struct StatusCodeTLV *m;
		m = (struct StatusCodeTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length += 2; //status_code

		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_I2B(&m->status_code, &p);

		return ret;
	}

	case TLV_TYPE_REASON_CODE: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.64"
		INT8U *               ret, *p;
		struct ReasonCodeTLV *m;
		m = (struct ReasonCodeTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length += 2; //reason_code

		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_I2B(&m->reason_code, &p);

		return ret;
	}

	case TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.65"
		INT8U *                                 ret, *p;
		struct BackhaulSTARadioCapabilitiesTLV *m;
		m = (struct BackhaulSTARadioCapabilitiesTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length += 6; //radio_unique_identifier
		tlv_length += 1; //included_mac
		tlv_length += 6; //sta_mac_address

		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_InB(m->radio_unique_identifier, &p, 6);
		_I1B(&m->included_mac, &p);
		_InB(m->sta_mac_address, &p, 6);

		return ret;
	}

	case TLV_TYPE_BACKHAUL_BSS_CONFIGURATION: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.66"
		INT8U *ret, *p;

		struct BackhaulBSSConfigurationTLV *m;
		m = (struct BackhaulBSSConfigurationTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length += 6; // bssid
		tlv_length += 1; // association_disallowed

		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->bssid, &p, 6);
		_I1B(&m->association_disallowed, &p);

		return ret;
	}

	case TLV_TYPE_1905_LAYER_SECURITY_CAPABILITY: {
		// This forging is done according to the information detailed in
		//	Multi-AP "Section 17.2.67"
		INT8U *                       ret, *p;
		struct SecurityCapabilityTLV *m;
		m = (struct SecurityCapabilityTLV *)memory_structure;

		INT16U tlv_length = 3;

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->onboarding_protocol_supported, &p);
		_I1B(&m->message_integrity_algorithm_supported, &p);
		_I1B(&m->message_encryption_algorithm_supported, &p);

		return ret;
	}

	case TLV_TYPE_MIC: {
		// This forging is done according to the information detailed in
		//	Multi-AP "Section 17.2.68"
		INT8U *        ret, *p;
		struct MICTLV *m;
		m = (struct MICTLV *)memory_structure;

		INT16U tlv_length = 1 + 6 + 6 + 2 + m->mic_length;

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->gtk_key_id_ver, &p);
		_INT64_I6B(&m->integrity_transmission_counter, &p);
		_InB(m->src_1905_al_mac_addr, &p, 6);
		_I2B(&m->mic_length, &p);
		_InB(m->mic, &p, m->mic_length);

		return ret;
	}

	case TLV_TYPE_ENCRYPTED_PAYLOAD: {
		// This forging is done according to the information detailed in
		//	Multi-AP "Section 17.2.69"
		INT8U *                     ret, *p;
		struct EncryptedPayloadTLV *m;
		m = (struct EncryptedPayloadTLV *)memory_structure;

		INT16U tlv_length = 0;
		tlv_length        = tlv_length + 6;                  // encryption transmission counter
		tlv_length        = tlv_length + 6 + 6;              // src mac addr + dst mac addr
		tlv_length        = tlv_length + 2 + m->aes_siv_len; // aes siv len + aes siv

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_INT64_I6B(&m->encryption_transmission_counter, &p);
		_InB(m->source_1905_mac_address, &p, 6);
		_InB(m->dest_1905_mac_address, &p, 6);
		_I2B(&m->aes_siv_len, &p);
		_InB(m->aes_siv, &p, m->aes_siv_len);

		return ret;
	}

	case TLV_TYPE_SERVICE_PRIORITIZATION_RULE: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.70"
		INT8U *                              ret, *p;
		struct ServicePrioritizationRuleTLV *m;
		m = (struct ServicePrioritizationRuleTLV *)memory_structure;

		INT16U tlv_length = 0;
		// Calculate TLV length
		tlv_length += 4; // SP Rule ID
		tlv_length += 4; // add remove + rule precedence + rule output + always match

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I4B(&m->service_prioritization_rule_id, &p);
		_I1B(&m->add_remove_rule, &p);
		_I1B(&m->rule_precedence, &p);
		_I1B(&m->rule_output, &p);
		_I1B(&m->always_match, &p);

		return ret;
	}

	case TLV_TYPE_DSCP_MAPPING_TABLE: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.71"
		INT8U *                     ret, *p;
		struct DSCPMappingTableTLV *m;
		m = (struct DSCPMappingTableTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length += 64; //dscp_to_pcp

		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_InB(m->dscp_to_pcp, &p, 64);

		return ret;
	}

	case TLV_TYPE_AP_WIFI_6_CAPABILITIES: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.72"
		INT8U *                        ret, *p;
		INT8U                          i, mcs_nss_nr;
		struct APWiFi6CapabilitiesTLV *m;
		m = (struct APWiFi6CapabilitiesTLV *)memory_structure;

		INT16U tlv_length = 0;

		// Calculate TLV length
		tlv_length = tlv_length + 6 + 1; // RUID + number of roles
		for (i = 0; i < m->role_nr; i++) {
			mcs_nss_nr = m->agent_roles[i].flags & MCS_NSS_NUMBER_MASK;
			tlv_length = tlv_length + 1;          // flags
			tlv_length = tlv_length + mcs_nss_nr; // mcs nss
			tlv_length = tlv_length + 5;          // wifi6 cap 1-5
		}

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->radio_unique_identifier, &p, 6);
		_I1B(&m->role_nr, &p);
		for (i = 0; i < m->role_nr; i++) {
			_I1B(&m->agent_roles[i].flags, &p);
			mcs_nss_nr = m->agent_roles[i].flags & MCS_NSS_NUMBER_MASK;
			_InB(m->agent_roles[i].mcs_nss, &p, mcs_nss_nr);
			_I1B(&m->agent_roles[i].wifi6_capability_1, &p);
			_I1B(&m->agent_roles[i].wifi6_capability_2, &p);
			_I1B(&m->agent_roles[i].wifi6_capability_3, &p);
			_I1B(&m->agent_roles[i].wifi6_capability_4, &p);
			_I1B(&m->agent_roles[i].wifi6_capability_5, &p);
		}

		return ret;
	}

	case TLV_TYPE_ASSOCIATED_WIFI_6_STA_STATUS_REPORT: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.73"
		INT8U *                                   ret, *p;
		INT8U                                     i;
		struct AssociatedWiFi6STAStatusReportTLV *m;
		m = (struct AssociatedWiFi6STAStatusReportTLV *)memory_structure;

		INT16U tlv_length = 0;

		// Calculate TLV length
		tlv_length = tlv_length + 6 + 1; // mac_addr + number of queues
		for (i = 0; i < m->queue_nr; i++) {
			tlv_length += 2; // tid + queue size
		}

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->mac_address, &p, 6);
		_I1B(&m->queue_nr, &p);
		for (i = 0; i < m->queue_nr; i++) {
			_I1B(&m->queues[i].tid, &p);
			_I1B(&m->queues[i].queue_size, &p);
		}

		return ret;
	}

	case TLV_TYPE_BSS_CONFIG_REPORT: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.75"
		INT8U *ret, *p;
		INT8U  k, m;

		struct BSSConfigReportTLV *  t;
		struct BSSConfigRadioReport *radio_report;
		struct BSSConfigBSSReport *  bss_report;

		t = (struct BSSConfigReportTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length += 1; // No. of Radios (k)
		for (k = 0; k < t->radio_report_nr; k++) {
			radio_report = &t->radio_reports[k];
			tlv_length += 6; // RUID
			tlv_length += 1; // No. of BSS (m)
			for (m = 0; m < radio_report->bss_report_nr; m++) {
				bss_report = &radio_report->bss_reports[m];
				tlv_length += 6;                       // BSSID
				tlv_length += 1;                       // Status Flags
				tlv_length += 1;                       // Reserved
				tlv_length += 1;                       // SSID Length
				tlv_length += bss_report->ssid_length; // SSID
			}
		}
		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&t->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&t->radio_report_nr, &p); // No. of Radios (k)
		for (k = 0; k < t->radio_report_nr; k++) {
			radio_report = &t->radio_reports[k];
			_InB(radio_report->radio_unique_identifier, &p, 6); // RUID
			_I1B(&radio_report->bss_report_nr, &p);             // No. of BSS (m)
			for (m = 0; m < radio_report->bss_report_nr; m++) {
				bss_report = &radio_report->bss_reports[m];
				_InB(bss_report->bssid, &p, 6);                      // BSSID
				_I1B(&bss_report->status_flag, &p);                  // Status Flags
				_I1B(&bss_report->reserved, &p);                     // Reserved
				_I1B(&bss_report->ssid_length, &p);                  // SSID Length
				_InB(bss_report->ssid, &p, bss_report->ssid_length); // SSID
			}
		}

		return ret;
	}

	case TLV_TYPE_DEVICE_INVENTORY: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.76"
		INT8U *                    ret, *p;
		INT8U                      i;
		struct DeviceInventoryTLV *m;
		m = (struct DeviceInventoryTLV *)memory_structure;

		INT16U tlv_length = 0;

		// Calculate TLV length
		tlv_length = tlv_length + 1 + m->lsn; // lsn, serial_number
		tlv_length = tlv_length + 1 + m->lsv; // lsv, software_version
		tlv_length = tlv_length + 1 + m->lee; // lee, execution_env
		tlv_length = tlv_length + 1;          // radio_nr
		for (i = 0; i < m->radio_nr; i++) {
			tlv_length = tlv_length + 6;                    // RUID
			tlv_length = tlv_length + 1 + m->radios[i].lcv; // lcv, chipset_vendor
		}

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->lsn, &p);
		_InB(m->serial_number, &p, m->lsn);
		_I1B(&m->lsv, &p);
		_InB(m->software_version, &p, m->lsv);
		_I1B(&m->lee, &p);
		_InB(m->execution_env, &p, m->lee);
		_I1B(&m->radio_nr, &p);
		for (i = 0; i < m->radio_nr; i++) {
			_InB(m->radios[i].radio_unique_identifier, &p, 6);
			_I1B(&m->radios[i].lcv, &p);
			_InB(m->radios[i].chipset_vendor, &p, m->radios[i].lcv);
		}

		return ret;
	}

	case TLV_TYPE_AGENT_LIST: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.77"
		INT8U *ret, *p;
		INT8U  i;

		struct AgentListTLV *m;
		m = (struct AgentListTLV *)memory_structure;

		INT16U tlv_length = 0;

		// Calculate TLV length
		tlv_length = tlv_length + 1; // agents_nr
		for (i = 0; i < m->agents_nr; i++) {
			tlv_length = tlv_length + 6; // al_mac_address
			tlv_length = tlv_length + 1; // map_profile
			tlv_length = tlv_length + 1; // security
		}

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->agents_nr, &p);
		for (i = 0; i < m->agents_nr; i++) {
			_InB(m->agents[i].al_mac_address, &p, 6);
			_I1B(&m->agents[i].map_profile, &p);
			_I1B(&m->agents[i].security, &p);
		}

		return ret;
	}

	case TLV_TYPE_AKM_SUITE_CAPABILITIES: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.78"
		INT8U *ret, *p, i;

		struct AKMSuiteCapabilitiesTLV *m;
		m = (struct AKMSuiteCapabilitiesTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin ////////////////////
		tlv_length += 1; // Num Backhaul BSS AKM Suite Selector
		for (i = 0; i < m->backhaul_bss_akm_suite_selectors_nr; i++) {
			tlv_length += 3; // OUI
			tlv_length += 1; // AKM Suite Type
		}
		tlv_length += 1; // Num Fronthaul BSS AKM Suite Selector
		for (i = 0; i < m->fronthaul_bss_akm_suite_selectors_nr; i++) {
			tlv_length += 3; // OUI
			tlv_length += 1; // AKM Suite Type
		}
		/////////// Calculate TLV length end //////////////////////

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->backhaul_bss_akm_suite_selectors_nr, &p);
		for (i = 0; i < m->backhaul_bss_akm_suite_selectors_nr; i++) {
			_InB(m->backhaul_bss_akm_suite_selectors[i].oui, &p, 3);
			_I1B(&m->backhaul_bss_akm_suite_selectors[i].akm_suite_type, &p);
		}
		_I1B(&m->fronthaul_bss_akm_suite_selectors_nr, &p);
		for (i = 0; i < m->fronthaul_bss_akm_suite_selectors_nr; i++) {
			_InB(m->fronthaul_bss_akm_suite_selectors[i].oui, &p, 3);
			_I1B(&m->fronthaul_bss_akm_suite_selectors[i].akm_suite_type, &p);
		}

		return ret;
	}

	case TLV_TYPE_1905_ENCAP_DPP: {
		// This forging is done according to the information detailed in
		// Multi-AP "Section 17.2.79"
		INT8U *ret, *p;

		struct Encap1905DPPTLV *m;
		m = (struct Encap1905DPPTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length += 1;                         // enrollee_mac_address_present
		tlv_length += 6;                         // dest_sta_mac_address
		tlv_length += 1;                         // frame_type
		tlv_length += 2;                         // encapsulated_frame_len
		tlv_length += m->encapsulated_frame_len; // encapsulated_frame
		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_I1B(&m->enrollee_mac_address_present, &p);
		_InB(m->dest_sta_mac_address, &p, 6);
		_I1B(&m->frame_type, &p);
		_I2B(&m->encapsulated_frame_len, &p);
		_InB(m->encapsulated_frame, &p, m->encapsulated_frame_len);

		return ret;
	}

	case TLV_TYPE_1905_ENCAP_EAPOL: {
		// This forging is done according to the information detailed in
		// Multi-AP "Section 17.2.80"
		INT8U *ret, *p;

		struct Encap1905EAPOLTLV *m;
		m = (struct Encap1905EAPOLTLV *)memory_structure;

		/////////// Calculate TLV length begin////////////////////
		INT16U tlv_length = m->eapol_frame_payload_len;
		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->eapol_frame_payload, &p, m->eapol_frame_payload_len);

		return ret;
	}

	case TLV_TYPE_DPP_BOOTSTRAPPING_URI_NOTIFICATION: {
		// This forging is done according to the information detailed in
		//  Multi-AP "Section 17.2.81"
		INT8U *ret, *p;

		struct DPPBootstrappingURINotificationTLV *m;
		m = (struct DPPBootstrappingURINotificationTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length += 6;                            //radio_unique_identifier
		tlv_length += 6;                            //local_interface_mac_address
		tlv_length += 6;                            //sta_mac_address
		tlv_length += m->dpp_bootstrapping_uri_len; //dpp_bootstrapping_uri

		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;

		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);

		_InB(m->radio_unique_identifier, &p, 6);
		_InB(m->local_interface_mac_address, &p, 6);
		_InB(m->sta_mac_address, &p, 6);
		_InB(m->dpp_bootstrapping_uri, &p, m->dpp_bootstrapping_uri_len);

		return ret;
	}

	case TLV_TYPE_DPP_CCE_INDICATION: {
		// This forging is done according to the information detailed in
		// Multi-AP "Section 17.2.83"
		INT8U *ret, *p;

		struct DPPCCEIndicationTLV *m;
		m = (struct DPPCCEIndicationTLV *)memory_structure;

		INT16U tlv_length = 1;	// fixed for R3 spec version.

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->advertise_cce, &p);

		return ret;
	}

	case TLV_TYPE_DPP_CHIRP_VALUE: {
		// This forging is done according to the information detailed in
		// Multi-AP "Section 17.2.83"
		INT8U *ret, *p;

		struct DPPChirpValueTLV *m;
		m = (struct DPPChirpValueTLV *)memory_structure;

		INT16U tlv_length = 0;
		/////////// Calculate TLV length begin////////////////////
		tlv_length += 1;                                                            // enrollee mac addr presence / hash validity
		tlv_length += (m->hash_validity & DPP_CHIRP_ENROLLEE_ADDR_PRESENT) ? 6 : 0; // destination sta mac addr
		tlv_length += 1;                                                            // hash length
		tlv_length += m->hash_len;                                                  // hash value
		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&m->hash_validity, &p);
		if (m->hash_validity & DPP_CHIRP_ENROLLEE_ADDR_PRESENT)
			_InB(&m->sta_mac_address, &p, 6);
		_I1B(&m->hash_len, &p);
		_InB(m->hash_value, &p, m->hash_len);

		return ret;
	}

	case TLV_TYPE_BSS_CONFIG_REQUEST: {
		// This forging is done according to the information detailed in
		// Multi-AP "Section 17.2.84"
		INT8U *ret, *p;

		struct BSSConfigRequestTLV *m;
		m = (struct BSSConfigRequestTLV *)memory_structure;

		/////////// Calculate TLV length begin////////////////////
		INT16U tlv_length = m->dpp_config_request_object_len;
		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->dpp_config_request_object, &p, m->dpp_config_request_object_len);

		return ret;
	}

	case TLV_TYPE_BSS_CONFIG_RESPONSE: {
		// This forging is done according to the information detailed in
		// Multi-AP "Section 17.2.85"
		INT8U *ret, *p;

		struct BSSConfigResponseTLV *m;
		m = (struct BSSConfigResponseTLV *)memory_structure;

		/////////// Calculate TLV length begin////////////////////
		INT16U tlv_length = m->dpp_config_object_len;
		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->dpp_config_object, &p, m->dpp_config_object_len);

		return ret;
	}

	case TLV_TYPE_DPP_MESSAGE: {
		// This forging is done according to the information detailed in
		// Multi-AP "Section 17.2.86"
		INT8U *ret, *p;

		struct DPPMessageTLV *m;
		m = (struct DPPMessageTLV *)memory_structure;

		/////////// Calculate TLV length begin////////////////////
		INT16U tlv_length = m->dpp_frame_len; // hash value
		/////////// Calculate TLV length end////////////////////

		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);

		_I1B(&m->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(m->dpp_frame, &p, m->dpp_frame_len);

		return ret;
	}

	case TLV_TYPE_ANTICIPATED_CHANNEL_PERFERENCE: {

		INT8U *ret, *p;

		struct AnticipatedChannelPerferenceTLV *t;
		t = (struct AnticipatedChannelPerferenceTLV *)memory_structure;
		struct OperatingChannelClass *operate_class;

		INT16U tlv_length = 0;
		tlv_length += 1;
		int k;
		for (k = 0; k < t->operating_class_nr; k++) {
			operate_class = &t->operate_class[k];
			tlv_length += 1;
			tlv_length += 1;
			tlv_length += sizeof(INT8U) * operate_class->channel_list_nr;
			tlv_length += 4;
		}
		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&t->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&t->operating_class_nr, &p);
		int m;
		for (m = 0; m < t->operating_class_nr; m++) {
			operate_class = &t->operate_class[m];
			_I1B(&operate_class->operating_class, &p);
			_I1B(&operate_class->channel_list_nr, &p);
			_InB(operate_class->channel_list, &p, operate_class->channel_list_nr);
			_InB(operate_class->reserved, &p, 4);
		}
		return ret;
	}
	case TLV_TYPE_ANTICIPATED_CHANNEL_USAGE: {

		INT8U *ret, *p;

		struct AnticipatedChannelUsageTLV *t;
		t = (struct AnticipatedChannelUsageTLV *)memory_structure;
		struct UsageEntryClass *usage_entry;

		INT16U tlv_length = 0;
		tlv_length += 1;
		tlv_length += 1;
		tlv_length += 6;
		tlv_length += 1;
		int k;
		for (k = 0; k < t->usage_entry_nr; k++) {
			usage_entry = &t->usage_entry[k];
			tlv_length += 4;
			tlv_length += 4;
			tlv_length += 4;
			tlv_length += 4;
			tlv_length += 1;
			tlv_length += sizeof(INT8U) * usage_entry->RU_bitmask_length;
			tlv_length += 6;
			tlv_length += 1;
			tlv_length += 1;
			tlv_length += 4;
		}
		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&t->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&t->operating_class, &p);
		_I1B(&t->channel_number, &p);
		_InB(t->reference_bssid, &p, 6);
		_I1B(&t->usage_entry_nr, &p);
		int m;
		for (m = 0; m < t->usage_entry_nr; m++) {
			usage_entry = &t->usage_entry[m];
			_I4B(&usage_entry->burst_start_time, &p);
			_I4B(&usage_entry->burst_length, &p);
			_I4B(&usage_entry->repititions, &p);
			_I4B(&usage_entry->burst_interval, &p);
			_I1B(&usage_entry->RU_bitmask_length, &p);
			_InB(usage_entry->RU_bitmask, &p, usage_entry->RU_bitmask_length);
			_InB(usage_entry->transmitter_identifier, &p, 6);
			_I1B(&usage_entry->power_level, &p);
			_I1B(&usage_entry->channel_usage_reason, &p);
			_InB(usage_entry->reserved, &p, 4);
		}
		return ret;
	}
	case TLV_TYPE_SPATIAL_REUSE_REQUEST: {

		INT8U *ret, *p;

		struct SpatialReuseRequestTLV *t;
		t = (struct SpatialReuseRequestTLV *)memory_structure;

		INT16U tlv_length = 0;
		tlv_length += 6;
		tlv_length += 1;
		tlv_length += 1;
		tlv_length += 1;
		tlv_length += 1;
		tlv_length += 1;
		tlv_length += 8;
		tlv_length += 8;
		tlv_length += 2;
		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&t->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(t->ruid, &p, 6);
		_I1B(&t->bss_color, &p);
		_I1B(&t->valid_field, &p);
		_I1B(&t->non_srg_obsspd_max_offset, &p);
		_I1B(&t->srg_obsspd_min_offset, &p);
		_I1B(&t->srg_obsspd_max_offset, &p);
		_InB(t->srg_bss_color_bitmap, &p, 8);
		_InB(t->srg_partial_bssid_bitmap, &p, 8);
		_InB(t->reserved, &p, 2);
		return ret;
	}
	case TLV_TYPE_SPATIAL_REUSE_REPORT: {

		INT8U *ret, *p;

		struct SpatialReuseReportTLV *t;
		t = (struct SpatialReuseReportTLV *)memory_structure;

		INT16U tlv_length = 0;
		tlv_length += 6;
		tlv_length += 1;
		tlv_length += 1;
		tlv_length += 1;
		tlv_length += 1;
		tlv_length += 1;
		tlv_length += 8;
		tlv_length += 8;
		tlv_length += 8;
		tlv_length += 2;
		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&t->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(t->ruid, &p, 6);
		_I1B(&t->bss_color, &p);
		_I1B(&t->valid_field, &p);
		_I1B(&t->non_srg_obsspd_max_offset, &p);
		_I1B(&t->srg_obsspd_min_offset, &p);
		_I1B(&t->srg_obsspd_max_offset, &p);
		_InB(t->srg_bss_color_bitmap, &p, 8);
		_InB(t->srg_partial_bssid_bitmap, &p, 8);
		_InB(t->neighbor_bss_color_inuse_bitmap, &p, 8);
		_InB(t->reserved, &p, 2);
		return ret;
	}
	case TLV_TYPE_SPATIAL_REUSE_CONFIG_RESPONSE: {

		INT8U *ret, *p;

		struct SpatialReuseConfigResponseTLV *t;
		t = (struct SpatialReuseConfigResponseTLV *)memory_structure;

		INT16U tlv_length = 0;
		tlv_length += 6;
		tlv_length += 1;
		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&t->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_InB(t->ruid, &p, 6);
		_I1B(&t->response_code, &p);
		return ret;
	}
	case TLV_TYPE_QOS_MANAGEMENT_POLICY: {

		INT8U *ret, *p;

		struct QoSManagementPolicyTLV *t;
		t = (struct QoSManagementPolicyTLV *)memory_structure;

		INT16U tlv_length = 0;
		tlv_length += 1;
		tlv_length += sizeof(INT8U[6]) * t->mscsd_disallowed_sta_nr;
		tlv_length += 1;
		tlv_length += sizeof(INT8U[6]) * t->scsd_disallowed_sta_nr;
		tlv_length += 20;
		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&t->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&t->mscsd_disallowed_sta_nr, &p);
		int k;
		for (k = 0; k < t->mscsd_disallowed_sta_nr; k++) {
			_InB(t->mscsd_disallowed_sta_mac_address[k], &p, 6);
		}
		_I1B(&t->scsd_disallowed_sta_nr, &p);
		int m;
		for (m = 0; m < t->scsd_disallowed_sta_nr; m++) {
			_InB(t->scsd_disallowed_sta_mac_address[m], &p, 6);
		}
		_InB(t->reserved, &p, 20);
		return ret;
	}
	case TLV_TYPE_QOS_MANAGEMENT_DESCRIPTOR: {

		INT8U *ret, *p;

		struct QoSManagementDescriptorTLV *t;
		t = (struct QoSManagementDescriptorTLV *)memory_structure;

		INT16U tlv_length = 0;
		tlv_length += 2;
		tlv_length += 6;
		tlv_length += 6;
		tlv_length += t->descriptor_element_len_1;
		tlv_length += t->descriptor_element_len_2;
		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&t->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I2B(&t->qmid, &p);
		_InB(t->bssid, &p, 6);
		_InB(t->client_mac, &p, 6);
		_InB(t->descriptor_element_1, &p, t->descriptor_element_len_1);
		_InB(t->descriptor_element_2, &p, t->descriptor_element_len_2);
		return ret;
	}
	case TLV_TYPE_CONTROLLER_CAPABILITY: {

		INT8U *ret, *p;

		struct ControllerCapabilityTLV *t;
		t = (struct ControllerCapabilityTLV *)memory_structure;

		INT16U tlv_length = 0;
		tlv_length += 1;
		*len = 1 + 2 + tlv_length;
		p = ret = (INT8U *)PLATFORM_MALLOC(1 + 2 + tlv_length);
		_I1B(&t->tlv_type, &p);
		_I2B(&tlv_length, &p);
		_I1B(&t->KiBMiB_counter, &p);
		return ret;
	}

	default: {
		// Ignore
		//
		return NULL;
	}
	}

	// This code cannot be reached
	//
	return NULL;
}

void *copy_1905_TLV_structure(INT8U *memory_structure) {
	if (NULL == memory_structure) {
		return NULL;
	}
	int i;
	switch (*memory_structure) {
	case TLV_TYPE_END_OF_MESSAGE:
	case TLV_TYPE_AL_MAC_ADDRESS_TYPE:
	case TLV_TYPE_MAC_ADDRESS_TYPE:
	case TLV_TYPE_LINK_METRIC_QUERY:
	case TLV_TYPE_LINK_METRIC_RESULT_CODE:
	case TLV_TYPE_SEARCHED_ROLE:
	case TLV_TYPE_AUTOCONFIG_FREQ_BAND:
	case TLV_TYPE_SUPPORTED_ROLE:
	case TLV_TYPE_SUPPORTED_FREQ_BAND:
	case TLV_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION:
	case TLV_TYPE_CLIENT_ASSOCIATION_EVENT:
	case TLV_TYPE_BACKHAUL_STEERING_REQUEST:
	case TLV_TYPE_BACKHAUL_STEERING_RESPONSE:
	case TLV_TYPE_ERROR_CODE:
	case TLV_TYPE_AP_CAPABILITY:
	case TLV_TYPE_AP_RADIO_IDENTIFIER:
	case TLV_TYPE_TRANSMIT_POWER_LIMIT:
	case TLV_TYPE_CHANNEL_SELECTION_RESPONSE:
	case TLV_TYPE_STEERING_BTM_REPORT:
	case TLV_TYPE_AP_HT_CAPABILITIES:
	case TLV_TYPE_AP_VHT_CAPABILITIES:
	case TLV_TYPE_CLIENT_INFO:
	case TLV_TYPE_AP_METRICS:
	case TLV_TYPE_STA_MAC_ADDRESS_TYPE:
	case TLV_TYPE_1905_LAYER_SECURITY_CAPABILITY:
	case TLV_TYPE_MULTIAP_PROFILE:
	case TLV_TYPE_DEFAULT_802_1Q_SETTINGS:
	case TLV_TYPE_SOURCE_INFO:
	case TLV_TYPE_TUNNELED_MESSAGE_TYPE:
	case TLV_TYPE_REASON_CODE:
	case TLV_TYPE_VENDOR_SPECIFIC:
	case TLV_TYPE_DEVICE_INFORMATION_TYPE:
	case TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES:
	case TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST:
	case TLV_TYPE_NEIGHBOR_DEVICE_LIST:
	case TLV_TYPE_TRANSMITTER_LINK_METRIC:
	case TLV_TYPE_RECEIVER_LINK_METRIC:
	case TLV_TYPE_WSC:
	case TLV_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION:
	case TLV_TYPE_SUPPORTED_SERVICE:
	case TLV_TYPE_SEARCHED_SERVICE:
	case TLV_TYPE_AP_OPERATIONAL_BSS:
	case TLV_TYPE_ASSOCIATED_CLIENTS:
	case TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES:
	case TLV_TYPE_AP_HE_CAPABILITIES:
	case TLV_TYPE_STEERING_POLICY:
	case TLV_TYPE_METRIC_REPORT_POLICY:
		// TODO
		return memory_structure;
	case TLV_TYPE_CHANNEL_PREFERENCE: {
		struct ChannelPreferenceTLV *m, *res;
		m = (struct ChannelPreferenceTLV *)memory_structure;
		res = (struct ChannelPreferenceTLV *)PLATFORM_MALLOC(sizeof(struct ChannelPreferenceTLV));

		res->tlv_type = m->tlv_type;
		PLATFORM_MEMCPY(res->radio_unique_id, m->radio_unique_id, 6);
		res->op_class_nr = m->op_class_nr;
		res->channel_preferences = (struct ChannelPreferencePerOpClass *)PLATFORM_MALLOC(m->op_class_nr * sizeof(struct ChannelPreferencePerOpClass));
		for (i = 0; i < m->op_class_nr; i++) {
			struct ChannelPreferencePerOpClass *ch_pref_dst, *ch_pref_src;
			ch_pref_dst = &res->channel_preferences[i];
			ch_pref_src = &m->channel_preferences[i];
			ch_pref_dst->op_class = ch_pref_src->op_class;
			ch_pref_dst->channel_nr = ch_pref_src->channel_nr;
			ch_pref_dst->preference_reason_code = ch_pref_src->preference_reason_code;
			ch_pref_dst->channel_list = (INT8U *)PLATFORM_MALLOC(ch_pref_src->channel_nr * sizeof(INT8U));
			PLATFORM_MEMCPY(ch_pref_dst->channel_list, ch_pref_src->channel_list, ch_pref_src->channel_nr * sizeof(INT8U));
		}
		return res;
	}
	case TLV_TYPE_RADIO_OPERATION_RESTRICTION:
	case TLV_TYPE_OPERATING_CHANNEL_REPORT:
	case TLV_TYPE_HIGHER_LAYER_DATA:
	case TLV_TYPE_STEERING_REQUEST:
	case TLV_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST:
	case TLV_TYPE_CLIENT_CAPABLITY_REPORT:
	case TLV_TYPE_AP_METRIC_QUERY:
	case TLV_TYPE_ASSOCIATED_STA_LINK_METRICS:
	case TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS:
	case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY:
	case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE:
	case TLV_TYPE_BEACON_METRICS_QUERY:
		return memory_structure;
	case TLV_TYPE_BEACON_METRICS_RESPONSE: {
		struct BeaconMetricsResponseTLV *m = NULL;
		struct BeaconMetricsResponseTLV *res = (struct BeaconMetricsResponseTLV *)PLATFORM_MALLOC(sizeof(struct BeaconMetricsResponseTLV));
		if (!res) {
			PLATFORM_PRINTF_ERROR("[%s] Failed to malloc bm report TLV struct ! \n", __FUNCTION__);
		}
		m = (struct BeaconMetricsResponseTLV *)memory_structure;
		res->tlv_type = m->tlv_type;
		PLATFORM_MEMCPY(res->mac_address, m->mac_address, 6);
		res->reserved_field = m->reserved_field;
		res->measurement_report_nr = m->measurement_report_nr;
		res->measurement_reports = (struct BeaconMeasurementReportIE *)PLATFORM_MALLOC(sizeof(struct BeaconMeasurementReportIE) * res->measurement_report_nr);
		if (!res->measurement_reports) {
			PLATFORM_PRINTF_ERROR("[%s] Failed to malloc bm report IE struct ! \n", __FUNCTION__);
			PLATFORM_FREE(res);
			return NULL;
		}
		PLATFORM_MEMSET(res->measurement_reports, 0, sizeof(struct BeaconMeasurementReportIE) * res->measurement_report_nr);
		int i = 0;
		for (i = 0; i < m->measurement_report_nr; i++) {
			res->measurement_reports[i].elementId = m->measurement_reports[i].elementId;
			res->measurement_reports[i].len = m->measurement_reports[i].len;
			res->measurement_reports[i].measurementToken = m->measurement_reports[i].measurementToken;
			res->measurement_reports[i].measurementReportMode = m->measurement_reports[i].measurementReportMode;
			res->measurement_reports[i].measurementType = m->measurement_reports[i].measurementType;
			res->measurement_reports[i].info = m->measurement_reports[i].info;
			res->measurement_reports[i].subelement_id = m->measurement_reports[i].subelement_id;
			res->measurement_reports[i].subelements_len = m->measurement_reports[i].subelements_len;
			if (res->measurement_reports[i].subelements_len > 0 && m->measurement_reports[i].subelements) {
				res->measurement_reports[i].subelements = (INT8U *)PLATFORM_MALLOC(res->measurement_reports[i].subelements_len);
				if (!res->measurement_reports[i].subelements) {
					PLATFORM_PRINTF_ERROR("[%s] Failed to malloc bm report IE struct subelements buffer! skip this. \n", __FUNCTION__);
					res->measurement_reports[i].subelements_len = 0;
					continue;
				}
				PLATFORM_MEMCPY(res->measurement_reports[i].subelements, m->measurement_reports[i].subelements, res->measurement_reports[i].subelements_len);
			}
			if (res->measurement_reports[i].subelements_remain_len > 0) {
				PLATFORM_MEMCPY(res->measurement_reports[i].subelements_remain, m->measurement_reports[i].subelements_remain, res->measurement_reports[i].subelements_remain_len);
			}
		}

		return (void *)res;

	}
	case TLV_TYPE_CAC_CAPABILITIES:
	case TLV_TYPE_CAC_STATUS_REPORT:
	case TLV_TYPE_CAC_COMPLETION_REPORT:
	case TLV_TYPE_CAC_TERMINATION:
	case TLV_TYPE_CAC_REQUEST:
	case TLV_TYPE_CHANNEL_SCAN_REPORTING_POLICY:
	case TLV_TYPE_CHANNEL_SCAN_CAPABILITIES:
	case TLV_TYPE_CHANNEL_SCAN_REQUEST:
	case TLV_TYPE_CHANNEL_SCAN_RESULT:
	case TLV_TYPE_TIMESTAMP:
	//case TLV_TYPE_GROUP_INTEGRITY_KEY:
	case TLV_TYPE_MIC:
	case TLV_TYPE_PROFILE_2_CAPABILITY:
	case TLV_TYPE_TRAFFIC_SEPARATION_POLICY:
	//case TLV_TYPE_PACKET_FILTERING_POLICY:
	//case TLV_TYPE_ETHERNET_CONFIGURATION_POLICY:
	//case TLV_TYPE_SERVICE_PRIORITIZATION_RULE:
	case TLV_TYPE_DSCP_MAPPING_TABLE:
	//case TLV_TYPE_SERVICE_PRIORITIZATION_INTERFACE_EXCEPTION:
	case TLV_TYPE_PROFILE_2_ERROR_CODE:
	//case TLV_TYPE_AP_OPERATIONAL_BACKHAUL_BSS:
	case TLV_TYPE_AP_RADIO_ADVANCED_CAPABILITIES:
	case TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION:
	case TLV_TYPE_TUNNELED:
	case TLV_TYPE_PROFILE_2_STEERING_REQUEST:
	case TLV_TYPE_UNSUCCESSFUL_ASSOCIATION_POILCY:
	case TLV_TYPE_METRIC_COLLECTION_INTERVAL:
	case TLV_TYPE_RADIO_METRICS:
	case TLV_TYPE_AP_EXTENDED_METRICS:
	case TLV_TYPE_ASSOCIATED_STA_EXTENDED_LINK_METRICS:
	case TLV_TYPE_STATUS_CODE:
	case TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES:
	case TLV_TYPE_AKM_SUITE_CAPABILITIES:
	case TLV_TYPE_1905_ENCAP_DPP:
	case TLV_TYPE_1905_ENCAP_EAPOL:
	case TLV_TYPE_DPP_BOOTSTRAPPING_URI_NOTIFICATION:
	case TLV_TYPE_BACKHAUL_BSS_CONFIGURATION:
	case TLV_TYPE_AP_WIFI_6_CAPABILITIES:
	case TLV_TYPE_ASSOCIATED_WIFI_6_STA_STATUS_REPORT:
	case TLV_TYPE_BSS_CONFIG_REPORT:
	case TLV_TYPE_DEVICE_INVENTORY:
	case TLV_TYPE_AGENT_LIST:
	case TLV_TYPE_DPP_CHIRP_VALUE:
	case TLV_TYPE_BSS_CONFIG_REQUEST:
	case TLV_TYPE_BSS_CONFIG_RESPONSE:
	case TLV_TYPE_DPP_MESSAGE:
	case TLV_TYPE_SERVICE_PRIORITIZATION_RULE:
	case TLV_TYPE_DPP_CCE_INDICATION:
	//case TLV_TYPE_BSSID_TO_UNIQUE_BSS_INDEX_MAPPING:
		// TODO
		return memory_structure;
	default: {
		return NULL;
	}
	}
}

void free_1905_TLV_structure(INT8U *memory_structure)
{
	if (NULL == memory_structure) {
		return;
	}

	// The first byte of any of the valid structures is always the "tlv_type"
	// field.
	//
	switch (*memory_structure) {
	case TLV_TYPE_END_OF_MESSAGE:
	case TLV_TYPE_AL_MAC_ADDRESS_TYPE:
	case TLV_TYPE_MAC_ADDRESS_TYPE:
	case TLV_TYPE_LINK_METRIC_QUERY:
	case TLV_TYPE_LINK_METRIC_RESULT_CODE:
	case TLV_TYPE_SEARCHED_ROLE:
	case TLV_TYPE_AUTOCONFIG_FREQ_BAND:
	case TLV_TYPE_SUPPORTED_ROLE:
	case TLV_TYPE_SUPPORTED_FREQ_BAND:
	case TLV_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION:
	case TLV_TYPE_CLIENT_ASSOCIATION_EVENT:
	case TLV_TYPE_BACKHAUL_STEERING_REQUEST:
	case TLV_TYPE_BACKHAUL_STEERING_RESPONSE:
	case TLV_TYPE_ERROR_CODE:
	case TLV_TYPE_AP_CAPABILITY:
	case TLV_TYPE_AP_RADIO_IDENTIFIER:
	case TLV_TYPE_TRANSMIT_POWER_LIMIT:
	case TLV_TYPE_CHANNEL_SELECTION_RESPONSE:
	case TLV_TYPE_STEERING_BTM_REPORT:
	case TLV_TYPE_AP_HT_CAPABILITIES:
	case TLV_TYPE_AP_VHT_CAPABILITIES:
	case TLV_TYPE_CLIENT_INFO:
	case TLV_TYPE_AP_METRICS:
	case TLV_TYPE_STA_MAC_ADDRESS_TYPE:
	case TLV_TYPE_1905_LAYER_SECURITY_CAPABILITY:
	case TLV_TYPE_MULTIAP_PROFILE:
	case TLV_TYPE_DEFAULT_802_1Q_SETTINGS:
	case TLV_TYPE_SOURCE_INFO:
	case TLV_TYPE_TUNNELED_MESSAGE_TYPE:
	case TLV_TYPE_DPP_CCE_INDICATION:
	case TLV_TYPE_REASON_CODE: {
		PLATFORM_FREE(memory_structure);
		return;
	}
	case TLV_TYPE_VENDOR_SPECIFIC: {
		struct vendorSpecificTLV *m;

		m = (struct vendorSpecificTLV *)memory_structure;

		if (m->m_nr > 0 && NULL != m->m) {
			PLATFORM_FREE(m->m);
		}
		PLATFORM_FREE(m);

		return;
	}

	case TLV_TYPE_DEVICE_INFORMATION_TYPE: {
		struct deviceInformationTypeTLV *m;

		m = (struct deviceInformationTypeTLV *)memory_structure;

		if (m->local_interfaces_nr > 0 && NULL != m->local_interfaces) {
			PLATFORM_FREE(m->local_interfaces);
		}
		PLATFORM_FREE(m);

		return;
	}

	case TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES: {
		struct deviceBridgingCapabilityTLV *m;
		INT8U                               i;

		m = (struct deviceBridgingCapabilityTLV *)memory_structure;

		for (i = 0; i < m->bridging_tuples_nr; i++) {
			if (m->bridging_tuples[i].bridging_tuple_macs_nr > 0 && NULL != m->bridging_tuples[i].bridging_tuple_macs) {
				PLATFORM_FREE(m->bridging_tuples[i].bridging_tuple_macs);
			}
		}
		if (m->bridging_tuples_nr > 0 && NULL != m->bridging_tuples) {
			PLATFORM_FREE(m->bridging_tuples);
		}
		PLATFORM_FREE(m);

		return;
	}

	case TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST: {
		struct non1905NeighborDeviceListTLV *m;

		m = (struct non1905NeighborDeviceListTLV *)memory_structure;

		if (m->non_1905_neighbors_nr > 0 && NULL != m->non_1905_neighbors) {
			PLATFORM_FREE(m->non_1905_neighbors);
		}
		PLATFORM_FREE(m);

		return;
	}

	case TLV_TYPE_NEIGHBOR_DEVICE_LIST: {
		struct neighborDeviceListTLV *m;

		m = (struct neighborDeviceListTLV *)memory_structure;

		if (m->neighbors_nr > 0 && NULL != m->neighbors) {
			PLATFORM_FREE(m->neighbors);
		}
		PLATFORM_FREE(m);

		return;
	}

	case TLV_TYPE_TRANSMITTER_LINK_METRIC: {
		struct transmitterLinkMetricTLV *m;

		m = (struct transmitterLinkMetricTLV *)memory_structure;

		if (m->transmitter_link_metrics_nr > 0 && NULL != m->transmitter_link_metrics) {
			PLATFORM_FREE(m->transmitter_link_metrics);
		}
		PLATFORM_FREE(m);

		return;
	}

	case TLV_TYPE_RECEIVER_LINK_METRIC: {
		struct receiverLinkMetricTLV *m;

		m = (struct receiverLinkMetricTLV *)memory_structure;

		if (m->receiver_link_metrics_nr > 0 && NULL != m->receiver_link_metrics) {
			PLATFORM_FREE(m->receiver_link_metrics);
		}
		PLATFORM_FREE(m);

		return;
	}

	case TLV_TYPE_WSC: {
		struct wscTLV *m;

		m = (struct wscTLV *)memory_structure;

		if (m->wsc_frame_size > 0 && NULL != m->wsc_frame) {
			PLATFORM_FREE(m->wsc_frame);
		}
		PLATFORM_FREE(m);

		return;
	}

	case TLV_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION: {
		struct pushButtonEventNotificationTLV *m;

		m = (struct pushButtonEventNotificationTLV *)memory_structure;

		if (m->media_types_nr > 0 && NULL != m->media_types) {
			PLATFORM_FREE(m->media_types);
		}
		PLATFORM_FREE(m);

		return;
	}

	case TLV_TYPE_SUPPORTED_SERVICE: {
		struct SupportedServiceTLV *m;

		m = (struct SupportedServiceTLV *)memory_structure;

		if (m->supported_service_nr > 0 && NULL != m->supported_service) {
			PLATFORM_FREE(m->supported_service);
		}
		PLATFORM_FREE(m);

		return;
	}
	case TLV_TYPE_SEARCHED_SERVICE: {
		struct SearchedServiceTLV *m;
		m = (struct SearchedServiceTLV *)memory_structure;

		if (m->searched_service_nr > 0 && NULL != m->searched_service) {
			PLATFORM_FREE(m->searched_service);
		}

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_AP_OPERATIONAL_BSS: {
		struct ApOperationalBSSTLV *m;
		INT8U                       k1, m1;

		m = (struct ApOperationalBSSTLV *)memory_structure;

		if (m->radios_nr > 0 && NULL != m->radios) {
			for (k1 = 0; k1 < m->radios_nr; k1++) {
				if (m->radios[k1].BSSs_nr > 0 && NULL != m->radios[k1].BSSs) {
					for (m1 = 0; m1 < m->radios[k1].BSSs_nr; m1++) {
						if (m->radios[k1].BSSs[m1].ssid_len)
							PLATFORM_FREE(m->radios[k1].BSSs[m1].ssid);
					}
					PLATFORM_FREE(m->radios[k1].BSSs);
				}
			}
			PLATFORM_FREE(m->radios);
		}

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_ASSOCIATED_CLIENTS: {
		struct AssociatedClientsTLV *m;
		INT8U                        k1;

		m = (struct AssociatedClientsTLV *)memory_structure;

		if (m->BSSs_nr > 0 && NULL != m->BSSs) {
			for (k1 = 0; k1 < m->BSSs_nr; k1++) {
				if (m->BSSs[k1].clients_nr > 0 && NULL != m->BSSs[k1].clients) {
					PLATFORM_FREE(m->BSSs[k1].clients);
				}
			}
			PLATFORM_FREE(m->BSSs);
		}

		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES: {
		struct APRadioBasicCapabilitiesTLV *m;
		INT8U                               k1;

		m = (struct APRadioBasicCapabilitiesTLV *)memory_structure;

		if (m->operating_classes_nr > 0 && NULL != m->operating_classes) {
			for (k1 = 0; k1 < m->operating_classes_nr; k1++) {
				if (m->operating_classes[k1].non_operable_channels_nr > 0 && NULL != m->operating_classes[k1].non_operable_channels) {
					PLATFORM_FREE(m->operating_classes[k1].non_operable_channels);
				}
			}
			PLATFORM_FREE(m->operating_classes);
		}

		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_AP_HE_CAPABILITIES: {
		struct APHECapabilitiesTLV *m;

		m = (struct APHECapabilitiesTLV *)memory_structure;

		if (m->he_msc_len > 0 && NULL != m->he_msc) {
			PLATFORM_FREE(m->he_msc);
		}
		PLATFORM_FREE(m);

		return;
	}

	case TLV_TYPE_STEERING_POLICY: {
		struct SteeringPolicyTLV *m;
		m = (struct SteeringPolicyTLV *)memory_structure;

		PLATFORM_FREE(m->local_disallowed_sta_mac);
		PLATFORM_FREE(m->btm_disallowed_sta_mac);
		PLATFORM_FREE(m->policies);

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_METRIC_REPORT_POLICY: {
		struct MetricReportingPolicyTLV *m;
		m = (struct MetricReportingPolicyTLV *)memory_structure;

		PLATFORM_FREE(m->policies);

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_CHANNEL_PREFERENCE: {
		struct ChannelPreferenceTLV *m;
		INT8U                        k1;
		m = (struct ChannelPreferenceTLV *)memory_structure;

		for (k1 = 0; k1 < m->op_class_nr; k1++) {
			PLATFORM_FREE(m->channel_preferences[k1].channel_list);
		}
		PLATFORM_FREE(m->channel_preferences);

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_RADIO_OPERATION_RESTRICTION: {
		struct RadioOperationRestrictionTLV *m;
		INT8U                                k1;
		m = (struct RadioOperationRestrictionTLV *)memory_structure;

		for (k1 = 0; k1 < m->op_class_nr; k1++) {
			PLATFORM_FREE(m->radio_operation_restriction[k1].channel_operation_restriction);
		}

		PLATFORM_FREE(m->radio_operation_restriction);

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_OPERATING_CHANNEL_REPORT: {
		struct OperatingChannelReportTLV *m;
		m = (struct OperatingChannelReportTLV *)memory_structure;

		PLATFORM_FREE(m->operating_channels);

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_HIGHER_LAYER_DATA: {
		struct HigherLayerDataTLV *m;
		m = (struct HigherLayerDataTLV *)memory_structure;

		PLATFORM_FREE(m->payload);
		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_STEERING_REQUEST: {
		struct SteeringRequestTLV *m;
		m = (struct SteeringRequestTLV *)memory_structure;

		PLATFORM_FREE(m->sta_mac_address);
		PLATFORM_FREE(m->target_bss);
		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST: {
		struct ClientAssociationControlRequestTLV *m;
		m = (struct ClientAssociationControlRequestTLV *)memory_structure;

		PLATFORM_FREE(m->sta_mac_address);
		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_CLIENT_CAPABLITY_REPORT: {
		struct ClientCapabilityReportTLV *m;
		m = (struct ClientCapabilityReportTLV *)memory_structure;
		if (m->frame_body_length && m->frame_body != NULL)
			PLATFORM_FREE(m->frame_body);
		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_AP_METRIC_QUERY: {
		struct APMetricQueryTLV *m;

		m = (struct APMetricQueryTLV *)memory_structure;

		if (m->bssid_nr) {
			PLATFORM_FREE(m->bssid);
		}

		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_ASSOCIATED_STA_LINK_METRICS: {
		struct AssociatedSTALinkMetricsTLV *m;

		m = (struct AssociatedSTALinkMetricsTLV *)memory_structure;

		if (m->bssid_nr > 0 && m->assoc_sta_link_metrics)
			PLATFORM_FREE(m->assoc_sta_link_metrics);

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS: {
		struct AssociatedSTATrafficStatsTLV *m;

		m = (struct AssociatedSTATrafficStatsTLV *)memory_structure;

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY: {
		struct UnassociatedSTALinkMetricsQueryTLV *m;
		INT8U                                      k;
		m = (struct UnassociatedSTALinkMetricsQueryTLV *)memory_structure;

		if (m->channel_list_nr > 0 && m->channel_list_entry) {
			for (k = 0; k < m->channel_list_nr; k++) {
				if (m->channel_list_entry[k].sta_nr > 0 && m->channel_list_entry[k].sta_mac_address)
					PLATFORM_FREE(m->channel_list_entry[k].sta_mac_address);
			}
			PLATFORM_FREE(m->channel_list_entry);
		}
		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE: {
		struct UnassociatedSTALinkMetricsResponseTLV *m;

		m = (struct UnassociatedSTALinkMetricsResponseTLV *)memory_structure;

		if (m->sta_nr && m->unassoc_sta_link_metrics)
			PLATFORM_FREE(m->unassoc_sta_link_metrics);

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_BEACON_METRICS_QUERY: {
		struct BeaconMetricsQueryTLV *m;
		INT8U                         k;

		m = (struct BeaconMetricsQueryTLV *)memory_structure;
		if (m->ssid_len > 0 && m->ssid) {
			PLATFORM_FREE(m->ssid);
		}

		if (m->ap_channel_report_nr > 0) {

			for (k = 0; k < m->ap_channel_report_nr; k++) {
				if (m->ap_channel_report[k].len > 1 && m->ap_channel_report[k].channel_list)
					PLATFORM_FREE(m->ap_channel_report[k].channel_list);
			}

			PLATFORM_FREE(m->ap_channel_report);
		}

		if (m->elementID_nr > 0 && m->element_list) {
			PLATFORM_FREE(m->element_list);
		}
		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_BEACON_METRICS_RESPONSE: {
		struct BeaconMetricsResponseTLV *m;
		INT8U                            k;

		m = (struct BeaconMetricsResponseTLV *)memory_structure;

		if (m->measurement_report_nr > 0 && m->measurement_reports) {
			for (k = 0; k < m->measurement_report_nr; k++) {
				if (m->measurement_reports[k].subelements_len > 0 && m->measurement_reports[k].subelements)
					PLATFORM_FREE(m->measurement_reports[k].subelements);
			}
			if (m->measurement_reports)
				PLATFORM_FREE(m->measurement_reports);
		}

		PLATFORM_FREE(m);
		return;
	}

		////////////////////////////////////////////////////////////////////////////////
		/// Profile 2 TLVs
		////////////////////////////////////////////////////////////////////////////////

	case TLV_TYPE_CAC_CAPABILITIES: {
		struct CACCapabilitiesTLV *m = (struct CACCapabilitiesTLV *)memory_structure;

		INT8U k, l, n;

		for (k = 0; k < m->radio_nr; k++) {
			if (m->radios[k].type_nr) {
				for (l = 0; l < m->radios[k].type_nr; l++) {
					if (m->radios[k].types[l].op_class_nr) {
						for (n = 0; n < m->radios[k].types[l].op_class_nr; n++) {
							if (m->radios[k].types[l].op_classes[n].channel_nr)
								PLATFORM_FREE(m->radios[k].types[l].op_classes[n].channels);
						}
						PLATFORM_FREE(m->radios[k].types[l].op_classes);
					}
				}
				PLATFORM_FREE(m->radios[k].types);
			}
		}

		if (m->radio_nr) {
			PLATFORM_FREE(m->radios);
		}

		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_CAC_STATUS_REPORT: {
		struct CACStatusReportTLV *m = (struct CACStatusReportTLV *)memory_structure;

		if (m->available_channel_nr) {
			PLATFORM_FREE(m->available_channels);
		}

		if (m->nonoccup_pair_nr) {
			PLATFORM_FREE(m->nonoccup_pairs);
		}

		if (m->active_pair_nr) {
			PLATFORM_FREE(m->active_pairs);
		}

		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_CAC_COMPLETION_REPORT: {
		struct CACCompletionReportTLV *m = (struct CACCompletionReportTLV *)memory_structure;

		INT8U k;

		for (k = 0; k < m->radio_nr; k++) {
			if (m->radios[k].pairs_nr) {
				PLATFORM_FREE(m->radios[k].pairs);
			}
		}

		if (m->radio_nr) {
			PLATFORM_FREE(m->radios);
		}

		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_CAC_TERMINATION: {
		struct CACTerminationTLV *m = (struct CACTerminationTLV *)memory_structure;

		if (m->radio_nr) {
			PLATFORM_FREE(m->radios);
		}

		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_CAC_REQUEST: {
		struct CACRequestTLV *m = (struct CACRequestTLV *)memory_structure;

		if (m->radio_nr) {
			PLATFORM_FREE(m->radios);
		}

		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_CHANNEL_SCAN_REPORTING_POLICY: {
		struct ChannelScanReportingPolicyTLV *m = (struct ChannelScanReportingPolicyTLV *)memory_structure;
		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_CHANNEL_SCAN_CAPABILITIES: {
		struct ChannelScanCapabilitiesTLV *m = (struct ChannelScanCapabilitiesTLV *)memory_structure;

		INT8U k, l;

		for (k = 0; k < m->radio_nr; k++) {
			if (m->radios[k].op_classes) {
				for (l = 0; l < m->radios[k].op_class_nr; l++) {
					if (m->radios[k].op_classes[l].channel_list)
						PLATFORM_FREE(m->radios[k].op_classes[l].channel_list);
				}
				PLATFORM_FREE(m->radios[k].op_classes);
			}
		}

		if (m->radio_nr) {
			PLATFORM_FREE(m->radios);
		}

		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_CHANNEL_SCAN_REQUEST: {
		struct ChannelScanRequestTLV *m = (struct ChannelScanRequestTLV *)memory_structure;

		INT8U k, l;

		for (k = 0; k < m->target_radio_nr; k++) {
			if (m->target_radios[k].target_op_classes) {
				for (l = 0; l < m->target_radios[k].target_op_class_nr; l++) {
					if (m->target_radios[k].target_op_classes[l].target_channel_nr) {
						PLATFORM_FREE(m->target_radios[k].target_op_classes[l].target_channel_list);
					}
				}
				PLATFORM_FREE(m->target_radios[k].target_op_classes);
			}
		}

		if (m->target_radio_nr) {
			PLATFORM_FREE(m->target_radios);
		}

		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_CHANNEL_SCAN_RESULT: {
		struct ChannelScanResultTLV *m = (struct ChannelScanResultTLV *)memory_structure;

		INT8U k;

		if (!m->scan_status) {
			PLATFORM_FREE(m->timestamp);
			for (k = 0; k < m->neighbor_nr; k++) {
				if (m->neighbors[k].ssid)
					PLATFORM_FREE(m->neighbors[k].ssid);
				if (m->neighbors[k].channel_bandwidth)
					PLATFORM_FREE(m->neighbors[k].channel_bandwidth);
			}
			if (m->neighbors)
				PLATFORM_FREE(m->neighbors);
		}

		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_TIMESTAMP: {
		struct TimestampTLV *m = (struct TimestampTLV *)memory_structure;

		if (m->timestamp)
			PLATFORM_FREE(m->timestamp);

		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_MIC: {
		struct MICTLV *m = (struct MICTLV *)memory_structure;
		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_PROFILE_2_CAPABILITY: {
		struct Profile2APCapabilityTLV *m;
		m = (struct Profile2APCapabilityTLV *)memory_structure;

		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_TRAFFIC_SEPARATION_POLICY: {
		struct TrafficSeparationPolicyTLV *m;
		INT8U                              k;
		m = (struct TrafficSeparationPolicyTLV *)memory_structure;

		if (m->ssid_nr) {
			for (k = 0; k < m->ssid_nr; k++) {
				if (m->ssid_info[k].ssid_length)
					PLATFORM_FREE(m->ssid_info[k].ssid_name);
			}
			PLATFORM_FREE(m->ssid_info);
		}
		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_DSCP_MAPPING_TABLE: {
		struct DSCPMappingTableTLV *m;
		m = (struct DSCPMappingTableTLV *)memory_structure;
		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_PROFILE_2_ERROR_CODE: {
		struct Profile2ErrorCodeTLV *m;
		m = (struct Profile2ErrorCodeTLV *)memory_structure;

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_AP_RADIO_ADVANCED_CAPABILITIES: {
		struct APRadioAdvancedCapabilitiesTLV *m;
		m = (struct APRadioAdvancedCapabilitiesTLV *)memory_structure;

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION: {
		struct AssociationStatusNotificationTLV *m;
		m = (struct AssociationStatusNotificationTLV *)memory_structure;
		if (m->bssid_nr) {
			PLATFORM_FREE(m->bss_info);
		}
		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_TUNNELED: {
		struct TunneledTLV *m;
		m = (struct TunneledTLV *)memory_structure;
		if (m->tlv_length) {
			PLATFORM_FREE(m->payload);
		}
		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_PROFILE_2_STEERING_REQUEST: {
		struct Profile2SteeringRequestTLV *m;
		m = (struct Profile2SteeringRequestTLV *)memory_structure;
		if (m->sta_list_count) {
			PLATFORM_FREE(m->sta_list);
		}
		if (m->target_bssid_list_count) {
			PLATFORM_FREE(m->target_bssid_list);
		}
		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_UNSUCCESSFUL_ASSOCIATION_POILCY: {
		struct UnsuccessfulAssociationPolicyTLV *m;
		m = (struct UnsuccessfulAssociationPolicyTLV *)memory_structure;

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_METRIC_COLLECTION_INTERVAL: {
		struct MetricCollectionIntervalTLV *m;
		m = (struct MetricCollectionIntervalTLV *)memory_structure;

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_RADIO_METRICS: {
		struct RadioMetricsTLV *m;
		m = (struct RadioMetricsTLV *)memory_structure;

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_AP_EXTENDED_METRICS: {
		struct APExtendedMetricsTLV *m;
		m = (struct APExtendedMetricsTLV *)memory_structure;

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_ASSOCIATED_STA_EXTENDED_LINK_METRICS: {
		struct AssociatedSTAExtendedLinkMericsTLV *m;
		m = (struct AssociatedSTAExtendedLinkMericsTLV *)memory_structure;
		if (m->bssid_nr) {
			PLATFORM_FREE(m->reported_bss);
		}

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_STATUS_CODE: {
		struct StatusCodeTLV *m;
		m = (struct StatusCodeTLV *)memory_structure;

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES: {
		struct BackhaulSTARadioCapabilitiesTLV *m;
		m = (struct BackhaulSTARadioCapabilitiesTLV *)memory_structure;

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_1905_ENCAP_DPP: {
		struct Encap1905DPPTLV *m;
		m = (struct Encap1905DPPTLV *)memory_structure;
		if (m->encapsulated_frame_len) {
			PLATFORM_FREE(m->encapsulated_frame);
		}

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_1905_ENCAP_EAPOL: {
		struct Encap1905EAPOLTLV *m;
		m = (struct Encap1905EAPOLTLV *)memory_structure;

		PLATFORM_FREE(m->eapol_frame_payload);
		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_DPP_BOOTSTRAPPING_URI_NOTIFICATION: {
		struct DPPBootstrappingURINotificationTLV *m;
		m = (struct DPPBootstrappingURINotificationTLV *)memory_structure;
		if (m->dpp_bootstrapping_uri_len) {
			PLATFORM_FREE(m->dpp_bootstrapping_uri);
		}

		PLATFORM_FREE(m);
		return;
	}
	case TLV_TYPE_BACKHAUL_BSS_CONFIGURATION: {
		struct BackhaulBSSConfigurationTLV *m;
		m = (struct BackhaulBSSConfigurationTLV *)memory_structure;

		PLATFORM_FREE(m);
		return;
	}

		////////////////////////////////////////////////////////////////////////////////
		/// Profile 3 TLVs
		////////////////////////////////////////////////////////////////////////////////

	case TLV_TYPE_SERVICE_PRIORITIZATION_RULE: {
		struct ServicePrioritizationRuleTLV *m;
		m = (struct ServicePrioritizationRuleTLV *)memory_structure;

		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_AP_WIFI_6_CAPABILITIES: {
		struct APWiFi6CapabilitiesTLV *m;
		m = (struct APWiFi6CapabilitiesTLV *)memory_structure;

		INT8U i;
		if (m->role_nr) {
			for (i = 0; i < m->role_nr; i++) {
				PLATFORM_FREE(m->agent_roles[i].mcs_nss);
			}
			PLATFORM_FREE(m->agent_roles);
		}
		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_BSS_CONFIG_REPORT: {
		struct BSSConfigReportTLV *  t;
		struct BSSConfigRadioReport *radio_report;
		struct BSSConfigBSSReport *  bss_report;

		t = (struct BSSConfigReportTLV *)memory_structure;

		INT8U k, m;
		for (k = 0; k < t->radio_report_nr; k++) {
			radio_report = &t->radio_reports[k];
			for (m = 0; m < radio_report->bss_report_nr; m++) {
				bss_report = &radio_report->bss_reports[m];
				PLATFORM_FREE(bss_report->ssid);
			}
			PLATFORM_FREE(radio_report->bss_reports);
		}
		PLATFORM_FREE(t->radio_reports);
		PLATFORM_FREE(t);

		return;
	}

	case TLV_TYPE_DEVICE_INVENTORY: {
		struct DeviceInventoryTLV *m;
		m = (struct DeviceInventoryTLV *)memory_structure;

		INT8U i;
		PLATFORM_FREE(m->serial_number);
		PLATFORM_FREE(m->software_version);
		PLATFORM_FREE(m->execution_env);
		if (m->radio_nr) {
			for (i = 0; i < m->radio_nr; i++) {
				PLATFORM_FREE(m->radios[i].chipset_vendor);
			}
			PLATFORM_FREE(m->radios);
		}
		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_AGENT_LIST: {
		struct AgentListTLV *m;
		m = (struct AgentListTLV *)memory_structure;

		if (m->agents) {
			PLATFORM_FREE(m->agents);
		}
		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_DPP_CHIRP_VALUE: {
		struct DPPChirpValueTLV *m;
		m = (struct DPPChirpValueTLV *)memory_structure;

		PLATFORM_FREE(m->hash_value);
		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_BSS_CONFIG_REQUEST: {
		struct BSSConfigRequestTLV *m;
		m = (struct BSSConfigRequestTLV *)memory_structure;

		PLATFORM_FREE(m->dpp_config_request_object);
		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_BSS_CONFIG_RESPONSE: {
		struct BSSConfigResponseTLV *m;
		m = (struct BSSConfigResponseTLV *)memory_structure;

		PLATFORM_FREE(m->dpp_config_object);
		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_AKM_SUITE_CAPABILITIES: {
		struct AKMSuiteCapabilitiesTLV *m;
		m = (struct AKMSuiteCapabilitiesTLV *)memory_structure;

		PLATFORM_FREE(m->backhaul_bss_akm_suite_selectors);
		PLATFORM_FREE(m->fronthaul_bss_akm_suite_selectors);
		return;
	}

	case TLV_TYPE_DPP_MESSAGE: {
		struct DPPMessageTLV *m;
		m = (struct DPPMessageTLV *)memory_structure;

		PLATFORM_FREE(m->dpp_frame);
		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_ENCRYPTED_PAYLOAD: {
		struct EncryptedPayloadTLV *m;
		m = (struct EncryptedPayloadTLV *)memory_structure;

		PLATFORM_FREE(m->aes_siv);
		PLATFORM_FREE(m);
		return;
	}

	case TLV_TYPE_ANTICIPATED_CHANNEL_PERFERENCE: {
		struct AnticipatedChannelPerferenceTLV *t;
		struct OperatingChannelClass *operate_class;
		t = (struct AnticipatedChannelPerferenceTLV *)memory_structure;

		int k;
		for (k = 0; k < t->operating_class_nr; k++) {
			operate_class = &t->operate_class[k];
			if( (operate_class->channel_list_nr >0) && operate_class->channel_list){
				PLATFORM_FREE(operate_class->channel_list);
			}
		}
		PLATFORM_FREE(t->operate_class);
		PLATFORM_FREE(t);
		return;
	}

	case TLV_TYPE_ANTICIPATED_CHANNEL_USAGE: {
		struct AnticipatedChannelUsageTLV *t;
		struct UsageEntryClass *usage_entry;
		t = (struct AnticipatedChannelUsageTLV *)memory_structure;

		int k;
		for (k = 0; k < t->usage_entry_nr; k++) {
			usage_entry = &t->usage_entry[k];
			if( (usage_entry->RU_bitmask_length >0) && usage_entry->RU_bitmask){
				PLATFORM_FREE(usage_entry->RU_bitmask);
			}
		}
		PLATFORM_FREE(t->usage_entry);
		PLATFORM_FREE(t);
		return;
	}

	case TLV_TYPE_SPATIAL_REUSE_REQUEST: {
		struct SpatialReuseRequestTLV *t;
		t = (struct SpatialReuseRequestTLV *)memory_structure;

		PLATFORM_FREE(t);
		return;
	}

	case TLV_TYPE_SPATIAL_REUSE_REPORT: {
		struct SpatialReuseReportTLV *t;
		t = (struct SpatialReuseReportTLV *)memory_structure;

		PLATFORM_FREE(t);
		return;
	}

	case TLV_TYPE_SPATIAL_REUSE_CONFIG_RESPONSE: {
		struct SpatialReuseConfigResponseTLV *t;
		t = (struct SpatialReuseConfigResponseTLV *)memory_structure;

		PLATFORM_FREE(t);
		return;
	}

	case TLV_TYPE_QOS_MANAGEMENT_POLICY: {
		struct QoSManagementPolicyTLV *t;
		t = (struct QoSManagementPolicyTLV *)memory_structure;

		if( (t->mscsd_disallowed_sta_nr >0) && t->mscsd_disallowed_sta_mac_address){
			PLATFORM_FREE(t->mscsd_disallowed_sta_mac_address);
		}
		if( (t->scsd_disallowed_sta_nr >0) && t->scsd_disallowed_sta_mac_address){
			PLATFORM_FREE(t->scsd_disallowed_sta_mac_address);
		}
		PLATFORM_FREE(t);
		return;
	}

	case TLV_TYPE_QOS_MANAGEMENT_DESCRIPTOR: {
		struct QoSManagementDescriptorTLV *t;
		t = (struct QoSManagementDescriptorTLV *)memory_structure;

		PLATFORM_FREE(t);
		return;
	}

	case TLV_TYPE_CONTROLLER_CAPABILITY: {
		struct ControllerCapabilityTLV *t;
		t = (struct ControllerCapabilityTLV *)memory_structure;

		PLATFORM_FREE(t);
		return;
	}

	default: {
		// Ignore
		//
		return;
	}
	}

	// This code cannot be reached
	//
	return;
}

INT8U compare_1905_TLV_structures(INT8U *memory_structure_1, INT8U *memory_structure_2)
{
	if (NULL == memory_structure_1 || NULL == memory_structure_2) {
		return 1;
	}

	// The first byte of any of the valid structures is always the "tlv_type"
	// field.
	//
	if (*memory_structure_1 != *memory_structure_2) {
		return 1;
	}
	switch (*memory_structure_1) {
	case TLV_TYPE_END_OF_MESSAGE: {
		// Nothing to compare (this TLV is always empty)
		//
		return 0;
	}

	case TLV_TYPE_VENDOR_SPECIFIC: {
		struct vendorSpecificTLV *p1, *p2;

		p1 = (struct vendorSpecificTLV *)memory_structure_1;
		p2 = (struct vendorSpecificTLV *)memory_structure_2;

		if (
		    p1->vendorOUI[0] != p2->vendorOUI[0] || p1->vendorOUI[1] != p2->vendorOUI[1] || p1->vendorOUI[2] != p2->vendorOUI[2] || p1->m_nr != p2->m_nr || (PLATFORM_MEMCMP(p1->m, p2->m, p1->m_nr) != 0)) {
			return 1;
		} else {
			return 0;
		}
	}

	case TLV_TYPE_AL_MAC_ADDRESS_TYPE: {
		struct alMacAddressTypeTLV *p1, *p2;

		p1 = (struct alMacAddressTypeTLV *)memory_structure_1;
		p2 = (struct alMacAddressTypeTLV *)memory_structure_2;

		if (
		    (PLATFORM_MEMCMP(p1->al_mac_address, p2->al_mac_address, 6) != 0)) {
			return 1;
		} else {
			return 0;
		}
	}

	case TLV_TYPE_MAC_ADDRESS_TYPE: {
		struct macAddressTypeTLV *p1, *p2;

		p1 = (struct macAddressTypeTLV *)memory_structure_1;
		p2 = (struct macAddressTypeTLV *)memory_structure_2;

		if (
		    (PLATFORM_MEMCMP(p1->mac_address, p2->mac_address, 6) != 0)) {
			return 1;
		} else {
			return 0;
		}
	}

	case TLV_TYPE_DEVICE_INFORMATION_TYPE: {
		struct deviceInformationTypeTLV *p1, *p2;
		INT8U                            i;

		p1 = (struct deviceInformationTypeTLV *)memory_structure_1;
		p2 = (struct deviceInformationTypeTLV *)memory_structure_2;

		if (
		    PLATFORM_MEMCMP(p1->al_mac_address, p2->al_mac_address, 6) != 0 || p1->local_interfaces_nr != p2->local_interfaces_nr) {
			return 1;
		}

		if (p1->local_interfaces_nr > 0 && (NULL == p1->local_interfaces || NULL == p2->local_interfaces)) {
			// Malformed structure
			//
			return 1;
		}

		for (i = 0; i < p1->local_interfaces_nr; i++) {
			if (
			    PLATFORM_MEMCMP(p1->local_interfaces[i].mac_address, p2->local_interfaces[i].mac_address, 6) != 0 || p1->local_interfaces[i].media_type != p2->local_interfaces[i].media_type || p1->local_interfaces[i].media_specific_data_size != p2->local_interfaces[i].media_specific_data_size) {
				return 1;
			}

			if (IS_IEEE_802_11_MEDIA(p1->local_interfaces[i].media_type)) {
				if (
				    PLATFORM_MEMCMP(p1->local_interfaces[i].media_specific_data.ieee80211.network_membership, p2->local_interfaces[i].media_specific_data.ieee80211.network_membership, 6) != 0 || p1->local_interfaces[i].media_specific_data.ieee80211.role != p2->local_interfaces[i].media_specific_data.ieee80211.role || p1->local_interfaces[i].media_specific_data.ieee80211.ap_channel_band != p2->local_interfaces[i].media_specific_data.ieee80211.ap_channel_band || p1->local_interfaces[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_1 != p2->local_interfaces[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_1) {
					return 1;
				}

			} else if (IS_IEEE_1901_MEDIA(p1->local_interfaces[i].media_type)) {
				if (
				    PLATFORM_MEMCMP(p1->local_interfaces[i].media_specific_data.ieee1901.network_identifier, p2->local_interfaces[i].media_specific_data.ieee1901.network_identifier, 6) != 0) {
					return 1;
				}
			}
		}

		return 0;
	}

	case TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES: {
		struct deviceBridgingCapabilityTLV *p1, *p2;
		INT8U                               i, j;

		p1 = (struct deviceBridgingCapabilityTLV *)memory_structure_1;
		p2 = (struct deviceBridgingCapabilityTLV *)memory_structure_2;

		if (
		    p1->bridging_tuples_nr != p2->bridging_tuples_nr) {
			return 1;
		}

		if (p1->bridging_tuples_nr > 0 && (NULL == p1->bridging_tuples || NULL == p2->bridging_tuples)) {
			// Malformed structure
			//
			return 1;
		}

		for (i = 0; i < p1->bridging_tuples_nr; i++) {
			if (
			    p1->bridging_tuples[i].bridging_tuple_macs_nr != p2->bridging_tuples[i].bridging_tuple_macs_nr) {
				return 1;
			}

			for (j = 0; j < p1->bridging_tuples[i].bridging_tuple_macs_nr; j++) {
				if (
				    PLATFORM_MEMCMP(p1->bridging_tuples[i].bridging_tuple_macs[j].mac_address, p2->bridging_tuples[i].bridging_tuple_macs[j].mac_address, 6) != 0) {
					return 1;
				}
			}
		}

		return 0;
	}

	case TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST: {
		struct non1905NeighborDeviceListTLV *p1, *p2;
		INT8U                                i;

		p1 = (struct non1905NeighborDeviceListTLV *)memory_structure_1;
		p2 = (struct non1905NeighborDeviceListTLV *)memory_structure_2;

		if (
		    PLATFORM_MEMCMP(p1->local_mac_address, p2->local_mac_address, 6) != 0 || p1->non_1905_neighbors_nr != p2->non_1905_neighbors_nr) {
			return 1;
		}

		if (p1->non_1905_neighbors_nr > 0 && (NULL == p1->non_1905_neighbors || NULL == p2->non_1905_neighbors)) {
			// Malformed structure
			//
			return 1;
		}

		for (i = 0; i < p1->non_1905_neighbors_nr; i++) {
			if (
			    PLATFORM_MEMCMP(p1->non_1905_neighbors[i].mac_address, p2->non_1905_neighbors[i].mac_address, 6) != 0) {
				return 1;
			}
		}

		return 0;
	}

	case TLV_TYPE_NEIGHBOR_DEVICE_LIST: {
		struct neighborDeviceListTLV *p1, *p2;
		INT8U                         i;

		p1 = (struct neighborDeviceListTLV *)memory_structure_1;
		p2 = (struct neighborDeviceListTLV *)memory_structure_2;

		if (
		    PLATFORM_MEMCMP(p1->local_mac_address, p2->local_mac_address, 6) != 0 || p1->neighbors_nr != p2->neighbors_nr) {
			return 1;
		}

		if (p1->neighbors_nr > 0 && (NULL == p1->neighbors || NULL == p2->neighbors)) {
			// Malformed structure
			//
			return 1;
		}

		for (i = 0; i < p1->neighbors_nr; i++) {
			if (
			    PLATFORM_MEMCMP(p1->neighbors[i].mac_address, p2->neighbors[i].mac_address, 6) != 0 || p1->neighbors[i].bridge_flag != p2->neighbors[i].bridge_flag) {
				return 1;
			}
		}

		return 0;
	}

	case TLV_TYPE_LINK_METRIC_QUERY: {
		struct linkMetricQueryTLV *p1, *p2;

		p1 = (struct linkMetricQueryTLV *)memory_structure_1;
		p2 = (struct linkMetricQueryTLV *)memory_structure_2;

		if (
		    p1->destination != p2->destination || PLATFORM_MEMCMP(p1->specific_neighbor, p2->specific_neighbor, 6) != 0 || p1->link_metrics_type != p2->link_metrics_type) {
			return 1;
		} else {
			return 0;
		}
	}

	case TLV_TYPE_TRANSMITTER_LINK_METRIC: {
		struct transmitterLinkMetricTLV *p1, *p2;
		INT8U                            i;

		p1 = (struct transmitterLinkMetricTLV *)memory_structure_1;
		p2 = (struct transmitterLinkMetricTLV *)memory_structure_2;

		if (
		    PLATFORM_MEMCMP(p1->local_al_address, p2->local_al_address, 6) != 0 || PLATFORM_MEMCMP(p1->neighbor_al_address, p2->neighbor_al_address, 6) != 0 || p1->transmitter_link_metrics_nr != p2->transmitter_link_metrics_nr) {
			return 1;
		}

		if (p1->transmitter_link_metrics_nr > 0 && (NULL == p1->transmitter_link_metrics || NULL == p2->transmitter_link_metrics)) {
			// Malformed structure
			//
			return 1;
		}

		for (i = 0; i < p1->transmitter_link_metrics_nr; i++) {
			if (
			    PLATFORM_MEMCMP(p1->transmitter_link_metrics[i].local_interface_address, p2->transmitter_link_metrics[i].local_interface_address, 6) != 0 || PLATFORM_MEMCMP(p1->transmitter_link_metrics[i].neighbor_interface_address, p2->transmitter_link_metrics[i].neighbor_interface_address, 6) != 0 || p1->transmitter_link_metrics[i].intf_type != p2->transmitter_link_metrics[i].intf_type || p1->transmitter_link_metrics[i].bridge_flag != p2->transmitter_link_metrics[i].bridge_flag || p1->transmitter_link_metrics[i].packet_errors != p2->transmitter_link_metrics[i].packet_errors || p1->transmitter_link_metrics[i].transmitted_packets != p2->transmitter_link_metrics[i].transmitted_packets || p1->transmitter_link_metrics[i].mac_throughput_capacity != p2->transmitter_link_metrics[i].mac_throughput_capacity || p1->transmitter_link_metrics[i].link_availability != p2->transmitter_link_metrics[i].link_availability || p1->transmitter_link_metrics[i].phy_rate != p2->transmitter_link_metrics[i].phy_rate) {
				return 1;
			}
		}

		return 0;
	}

	case TLV_TYPE_RECEIVER_LINK_METRIC: {
		struct receiverLinkMetricTLV *p1, *p2;
		INT8U                         i;

		p1 = (struct receiverLinkMetricTLV *)memory_structure_1;
		p2 = (struct receiverLinkMetricTLV *)memory_structure_2;

		if (
		    PLATFORM_MEMCMP(p1->local_al_address, p2->local_al_address, 6) != 0 || PLATFORM_MEMCMP(p1->neighbor_al_address, p2->neighbor_al_address, 6) != 0 || p1->receiver_link_metrics_nr != p2->receiver_link_metrics_nr) {
			return 1;
		}

		if (p1->receiver_link_metrics_nr > 0 && (NULL == p1->receiver_link_metrics || NULL == p2->receiver_link_metrics)) {
			// Malformed structure
			//
			return 1;
		}

		for (i = 0; i < p1->receiver_link_metrics_nr; i++) {
			if (
			    PLATFORM_MEMCMP(p1->receiver_link_metrics[i].local_interface_address, p2->receiver_link_metrics[i].local_interface_address, 6) != 0 || PLATFORM_MEMCMP(p1->receiver_link_metrics[i].neighbor_interface_address, p2->receiver_link_metrics[i].neighbor_interface_address, 6) != 0 || p1->receiver_link_metrics[i].intf_type != p2->receiver_link_metrics[i].intf_type || p1->receiver_link_metrics[i].packet_errors != p2->receiver_link_metrics[i].packet_errors || p1->receiver_link_metrics[i].packets_received != p2->receiver_link_metrics[i].packets_received || p1->receiver_link_metrics[i].rssi != p2->receiver_link_metrics[i].rssi) {
				return 1;
			}
		}

		return 0;
	}

	case TLV_TYPE_LINK_METRIC_RESULT_CODE: {
		struct linkMetricResultCodeTLV *p1, *p2;

		p1 = (struct linkMetricResultCodeTLV *)memory_structure_1;
		p2 = (struct linkMetricResultCodeTLV *)memory_structure_2;

		if (
		    p1->result_code != p2->result_code) {
			return 1;
		} else {
			return 0;
		}
	}

	case TLV_TYPE_SEARCHED_ROLE: {
		struct searchedRoleTLV *p1, *p2;

		p1 = (struct searchedRoleTLV *)memory_structure_1;
		p2 = (struct searchedRoleTLV *)memory_structure_2;

		if (
		    p1->role != p2->role) {
			return 1;
		} else {
			return 0;
		}
	}

	case TLV_TYPE_AUTOCONFIG_FREQ_BAND: {
		struct autoconfigFreqBandTLV *p1, *p2;

		p1 = (struct autoconfigFreqBandTLV *)memory_structure_1;
		p2 = (struct autoconfigFreqBandTLV *)memory_structure_2;

		if (
		    p1->freq_band != p2->freq_band) {
			return 1;
		} else {
			return 0;
		}
	}

	case TLV_TYPE_SUPPORTED_ROLE: {
		struct supportedRoleTLV *p1, *p2;

		p1 = (struct supportedRoleTLV *)memory_structure_1;
		p2 = (struct supportedRoleTLV *)memory_structure_2;

		if (
		    p1->role != p2->role) {
			return 1;
		} else {
			return 0;
		}
	}

	case TLV_TYPE_SUPPORTED_FREQ_BAND: {
		struct supportedFreqBandTLV *p1, *p2;

		p1 = (struct supportedFreqBandTLV *)memory_structure_1;
		p2 = (struct supportedFreqBandTLV *)memory_structure_2;

		if (
		    p1->freq_band != p2->freq_band) {
			return 1;
		} else {
			return 0;
		}
	}

	case TLV_TYPE_WSC: {
		struct wscTLV *p1, *p2;

		p1 = (struct wscTLV *)memory_structure_1;
		p2 = (struct wscTLV *)memory_structure_2;

		if (
		    p1->wsc_frame_size != p2->wsc_frame_size || PLATFORM_MEMCMP(p1->wsc_frame, p2->wsc_frame, p1->wsc_frame_size) != 0) {
			return 1;
		}

		return 0;
	}

	case TLV_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION: {
		struct pushButtonEventNotificationTLV *p1, *p2;
		INT8U                                  i;

		p1 = (struct pushButtonEventNotificationTLV *)memory_structure_1;
		p2 = (struct pushButtonEventNotificationTLV *)memory_structure_2;

		if (p1->media_types_nr != p2->media_types_nr) {
			return 1;
		}

		if (p1->media_types_nr > 0 && (NULL == p1->media_types || NULL == p2->media_types)) {
			// Malformed structure
			//
			return 1;
		}

		for (i = 0; i < p1->media_types_nr; i++) {
			if (
			    p1->media_types[i].media_type != p2->media_types[i].media_type || p1->media_types[i].media_specific_data_size != p2->media_types[i].media_specific_data_size) {
				return 1;
			}

			if (IS_IEEE_802_11_MEDIA(p1->media_types[i].media_type)) {
				if (
				    PLATFORM_MEMCMP(p1->media_types[i].media_specific_data.ieee80211.network_membership, p2->media_types[i].media_specific_data.ieee80211.network_membership, 6) != 0 || p1->media_types[i].media_specific_data.ieee80211.role != p2->media_types[i].media_specific_data.ieee80211.role || p1->media_types[i].media_specific_data.ieee80211.ap_channel_band != p2->media_types[i].media_specific_data.ieee80211.ap_channel_band || p1->media_types[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_1 != p2->media_types[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_1) {
					return 1;
				}

			} else if (IS_IEEE_1901_MEDIA(p1->media_types[i].media_type)) {
				if (
				    PLATFORM_MEMCMP(p1->media_types[i].media_specific_data.ieee1901.network_identifier, p2->media_types[i].media_specific_data.ieee1901.network_identifier, 6) != 0) {
					return 1;
				}
			}
		}

		return 0;
	}

	case TLV_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION: {
		struct pushButtonJoinNotificationTLV *p1, *p2;

		p1 = (struct pushButtonJoinNotificationTLV *)memory_structure_1;
		p2 = (struct pushButtonJoinNotificationTLV *)memory_structure_2;

		if (
		    PLATFORM_MEMCMP(p1->al_mac_address, p2->al_mac_address, 6) != 0 || p1->message_identifier != p2->message_identifier || PLATFORM_MEMCMP(p1->mac_address, p2->al_mac_address, 6) != 0 || PLATFORM_MEMCMP(p1->new_mac_address, p2->al_mac_address, 6) != 0) {
			return 1;
		} else {
			return 0;
		}
	}

	case TLV_TYPE_CLIENT_ASSOCIATION_EVENT: {
		struct ClientAssociationEventTLV *p1, *p2;

		p1 = (struct ClientAssociationEventTLV *)memory_structure_1;
		p2 = (struct ClientAssociationEventTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}

		if ((PLATFORM_MEMCMP(p1->mac_address, p2->mac_address, 6) != 0) || (PLATFORM_MEMCMP(p1->bssid, p2->bssid, 6) != 0)) {
			return 1;
		}

		if (p1->event != p2->event) {
			return 1;
		}

		return 0;
	}

	case TLV_TYPE_SUPPORTED_SERVICE: {
		struct SupportedServiceTLV *p1, *p2;

		p1 = (struct SupportedServiceTLV *)memory_structure_1;
		p2 = (struct SupportedServiceTLV *)memory_structure_2;

		if (
		    p1->supported_service_nr != p2->supported_service_nr
		    || PLATFORM_MEMCMP(p1->supported_service, p2->supported_service, p2->supported_service_nr) != 0) {
			return 1;
		} else {
			return 0;
		}
	}

	case TLV_TYPE_SEARCHED_SERVICE: {
		struct SearchedServiceTLV *p1, *p2;

		p1 = (struct SearchedServiceTLV *)memory_structure_1;
		p2 = (struct SearchedServiceTLV *)memory_structure_2;

		if (
		    p1->searched_service_nr != p2->searched_service_nr
		    || PLATFORM_MEMCMP(p1->searched_service, p2->searched_service, p2->searched_service_nr) != 0) {
			return 1;
		} else {
			return 0;
		}
	}

	case TLV_TYPE_AP_OPERATIONAL_BSS: {
		struct ApOperationalBSSTLV *p1, *p2;

		p1 = (struct ApOperationalBSSTLV *)memory_structure_1;
		p2 = (struct ApOperationalBSSTLV *)memory_structure_2;

		if (p1->radios_nr != p2->radios_nr)
			return 1;

		INT8U k1, m1;
		for (k1 = 0; k1 < p1->radios_nr; k1++) {
			//  radio_unique_id, BSSs_nr
			if (PLATFORM_MEMCMP(p1->radios[k1].radio_unique_id, p2->radios[k1].radio_unique_id, 6) != 0
			    || p1->radios[k1].BSSs_nr != p2->radios[k1].BSSs_nr)
				return 1;
			for (m1 = 0; m1 < p1->radios[k1].BSSs_nr; m1++) {
				// ssid_len, ssid
				if (PLATFORM_MEMCMP(p1->radios[k1].BSSs[m1].mac_addr, p2->radios[k1].BSSs[m1].mac_addr, 6)
				    || p1->radios[k1].BSSs[m1].ssid_len != p2->radios[k1].BSSs[m1].ssid_len
				    || PLATFORM_MEMCMP(p1->radios[k1].BSSs[m1].ssid, p2->radios[k1].BSSs[m1].ssid, p1->radios[k1].BSSs[m1].ssid_len))
					return 1;
			}
		}
		// else
		return 0;
	}
	case TLV_TYPE_ASSOCIATED_CLIENTS: {
		struct AssociatedClientsTLV *p1, *p2;

		p1 = (struct AssociatedClientsTLV *)memory_structure_1;
		p2 = (struct AssociatedClientsTLV *)memory_structure_2;

		// BSSs_nr
		if (p1->BSSs_nr != p2->BSSs_nr)
			return 1;

		INT8U k1, m1;
		for (k1 = 0; k1 < p1->BSSs_nr; k1++) {
			//  bssid, clients_nr
			if (PLATFORM_MEMCMP(p1->BSSs[k1].bssid, p2->BSSs[k1].bssid, 6) != 0
			    || p1->BSSs[k1].clients_nr != p2->BSSs[k1].clients_nr)
				return 1;
			for (m1 = 0; m1 < p1->BSSs[k1].clients_nr; m1++) {
				// mac_addr, left_time
				if (PLATFORM_MEMCMP(p1->BSSs[k1].clients[m1].mac_addr, p2->BSSs[k1].clients[m1].mac_addr, 6) != 0
				    || p1->BSSs[k1].clients[m1].left_time != p2->BSSs[k1].clients[m1].left_time)
					return 1;
			}
		}
		// else
		return 0;
	}
	case TLV_TYPE_ERROR_CODE: {
		struct ErrorCodeTLV *p1, *p2;

		p1 = (struct ErrorCodeTLV *)memory_structure_1;
		p2 = (struct ErrorCodeTLV *)memory_structure_2;

		if (p1->reason_code != p2->reason_code)
			return 1;

		if (p1->reason_code == 0x01 || p1->reason_code == 0x02) {
			if (PLATFORM_MEMCMP(p1->sta_mac_address, p2->sta_mac_address, 6) != 0)
				return 1;
		}

		//else
		return 0;
	}

	case TLV_TYPE_AP_RADIO_IDENTIFIER: {
		struct ApRadioIdentifierTLV *p1, *p2;

		p1 = (struct ApRadioIdentifierTLV *)memory_structure_1;
		p2 = (struct ApRadioIdentifierTLV *)memory_structure_2;

		if (PLATFORM_MEMCMP(p1->radio_unique_id, p2->radio_unique_id, 6) != 0) {
			return 1;
		} else {
			return 0;
		}
	}

	case TLV_TYPE_AP_CAPABILITY: {

		struct APCapabilityTLV *p1, *p2;

		p1 = (struct APCapabilityTLV *)memory_structure_1;
		p2 = (struct APCapabilityTLV *)memory_structure_2;

		if (p1->ap_capability != p2->ap_capability)
			return 1;

		return 0;
	}

	case TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES: {
		struct APRadioBasicCapabilitiesTLV *p1, *p2;

		p1 = (struct APRadioBasicCapabilitiesTLV *)memory_structure_1;
		p2 = (struct APRadioBasicCapabilitiesTLV *)memory_structure_2;

		// Radio UID
		if (PLATFORM_MEMCMP(p1->radio_unique_id, p2->radio_unique_id, 6) != 0)
			return 1;

		if (p1->max_BSSs_nr != p2->max_BSSs_nr)
			return 1;

		if (p1->operating_classes_nr != p2->operating_classes_nr)
			return 1;

		INT8U k1, m1;
		for (k1 = 0; k1 < p1->operating_classes_nr; k1++) {
			if (p1->operating_classes[k1].operating_class != p2->operating_classes[k1].operating_class)
				return 1;

			if (p1->operating_classes[k1].max_transmit_power != p2->operating_classes[k1].max_transmit_power)
				return 1;

			if (p1->operating_classes[k1].non_operable_channels_nr != p2->operating_classes[k1].non_operable_channels_nr)
				return 1;

			for (m1 = 0; m1 < p1->operating_classes[k1].non_operable_channels_nr; m1++) {
				if (p1->operating_classes[k1].non_operable_channels[m1] != p2->operating_classes[k1].non_operable_channels[m1])
					return 1;
			}
		}
		// else
		return 0;
	}

	case TLV_TYPE_AP_HT_CAPABILITIES: {

		struct APHTCapabilitiesTLV *p1, *p2;

		p1 = (struct APHTCapabilitiesTLV *)memory_structure_1;
		p2 = (struct APHTCapabilitiesTLV *)memory_structure_2;

		if (PLATFORM_MEMCMP(p1->radio_unique_id, p2->radio_unique_id, 6) != 0)
			return 1;

		if (p1->ht_capability != p2->ht_capability)
			return 1;

		return 0;
	}

	case TLV_TYPE_AP_VHT_CAPABILITIES: {

		struct APVHTCapabilitiesTLV *p1, *p2;

		p1 = (struct APVHTCapabilitiesTLV *)memory_structure_1;
		p2 = (struct APVHTCapabilitiesTLV *)memory_structure_2;

		if (PLATFORM_MEMCMP(p1->radio_unique_id, p2->radio_unique_id, 6) != 0)
			return 1;

		if ((p1->vht_tx_msc != p2->vht_tx_msc) || (p1->vht_rx_msc != p2->vht_rx_msc) || (p1->vht_capability_1 != p2->vht_capability_1) || (p1->vht_capability_2 != p2->vht_capability_2))
			return 1;

		return 0;
	}

	case TLV_TYPE_AP_HE_CAPABILITIES: {

		struct APHECapabilitiesTLV *p1, *p2;

		p1 = (struct APHECapabilitiesTLV *)memory_structure_1;
		p2 = (struct APHECapabilitiesTLV *)memory_structure_2;

		if (PLATFORM_MEMCMP(p1->radio_unique_id, p2->radio_unique_id, 6) != 0)
			return 1;

		if ((p1->he_msc_len != p2->he_msc_len) || (p1->he_capability_1 != p2->he_capability_1) || (p1->he_capability_2 != p2->he_capability_2))
			return 1;

		if (PLATFORM_MEMCMP(p1->he_msc, p2->he_msc, p1->he_msc_len) != 0)
			return 1;

		return 0;
	}

	case TLV_TYPE_STEERING_POLICY: {
		struct SteeringPolicyTLV *p1, *p2;

		p1 = (struct SteeringPolicyTLV *)memory_structure_1;
		p2 = (struct SteeringPolicyTLV *)memory_structure_2;

		if (p1->local_disallowed_sta_nr != p2->local_disallowed_sta_nr) {
			return 1;
		} else {
			int i;
			for (i = 0; i < p1->local_disallowed_sta_nr; i++) {
				if (0 != PLATFORM_MEMCMP(p1->local_disallowed_sta_mac[i], p2->local_disallowed_sta_mac[i], 6))
					return 1;
			}
		}

		if (p1->btm_disallowed_sta_nr != p2->btm_disallowed_sta_nr) {
			return 1;
		} else {
			int i;
			for (i = 0; i < p1->btm_disallowed_sta_nr; i++) {
				if (0 != PLATFORM_MEMCMP(p1->btm_disallowed_sta_mac[i], p2->btm_disallowed_sta_mac[i], 6))
					return 1;
			}
		}
		if (p1->radios_nr != p2->radios_nr) {
			return 1;
		} else {
			int i;
			for (i = 0; i < p1->radios_nr; i++) {
				if (0 != PLATFORM_MEMCMP(p1->policies[i].radio_unique_id, p2->policies[i].radio_unique_id, 6)
				    || p1->policies[i].policy != p2->policies[i].policy
				    || p1->policies[i].channel_util_threshold != p2->policies[i].channel_util_threshold
				    || p1->policies[i].rcpi_steering_threshold != p2->policies[i].rcpi_steering_threshold)
					return 1;
			}
		}

		//else
		return 0;
	}
	case TLV_TYPE_METRIC_REPORT_POLICY: {
		struct MetricReportingPolicyTLV *p1, *p2;

		p1 = (struct MetricReportingPolicyTLV *)memory_structure_1;
		p2 = (struct MetricReportingPolicyTLV *)memory_structure_2;

		if (p1->report_interval != p2->report_interval) {
			return 1;
		} else if (p1->radios_nr != p2->radios_nr) {
			return 1;
		} else {
			int i;
			for (i = 0; i < p1->radios_nr; i++) {
				if (0 != PLATFORM_MEMCMP(p1->policies[i].radio_unique_id, p2->policies[i].radio_unique_id, 6)
				    || p1->policies[i].sta_rcpi_threshold != p2->policies[i].sta_rcpi_threshold
				    || p1->policies[i].sta_rcpi_hysteresis_margin_override != p2->policies[i].sta_rcpi_hysteresis_margin_override
				    || p1->policies[i].ap_channel_utilization_threshold != p2->policies[i].ap_channel_utilization_threshold
				    || p1->policies[i].policy != p2->policies[i].policy)
					return 1;
			}
		}

		//else
		return 0;
	}
	case TLV_TYPE_CHANNEL_PREFERENCE: {
		struct ChannelPreferenceTLV *p1, *p2;

		p1 = (struct ChannelPreferenceTLV *)memory_structure_1;
		p2 = (struct ChannelPreferenceTLV *)memory_structure_2;

		if (0 != PLATFORM_MEMCMP(p1->radio_unique_id, p2->radio_unique_id, 6)) {
			return 1;
		}
		if (p1->op_class_nr != p2->op_class_nr) {
			return 1;
		} else {
			int i;
			for (i = 0; i < p1->op_class_nr; i++) {
				if (p1->channel_preferences[i].op_class != p2->channel_preferences[i].op_class
				    || p1->channel_preferences[i].channel_nr != p2->channel_preferences[i].channel_nr
				    || 0 != PLATFORM_MEMCMP(p1->channel_preferences[i].channel_list, p2->channel_preferences[i].channel_list, p1->channel_preferences[i].channel_nr)
				    || p1->channel_preferences[i].preference_reason_code != p2->channel_preferences[i].preference_reason_code)
					return 1;
			}
		}

		//else
		return 0;
	}
	case TLV_TYPE_RADIO_OPERATION_RESTRICTION: {
		struct RadioOperationRestrictionTLV *p1, *p2;

		p1 = (struct RadioOperationRestrictionTLV *)memory_structure_1;
		p2 = (struct RadioOperationRestrictionTLV *)memory_structure_2;

		if (0 != PLATFORM_MEMCMP(p1->radio_unique_id, p2->radio_unique_id, 6)) {
			return 1;
		}
		if (p1->op_class_nr != p2->op_class_nr) {
			return 1;
		} else {
			int i;
			for (i = 0; i < p1->op_class_nr; i++) {
				if (p1->radio_operation_restriction[i].op_class != p2->radio_operation_restriction[i].op_class
				    || p1->radio_operation_restriction[i].channel_nr != p2->radio_operation_restriction[i].channel_nr)
					return 1;
				else {
					int k;
					for (k = 0; k < p1->radio_operation_restriction[i].channel_nr; k++) {
						if (p1->radio_operation_restriction[i].channel_operation_restriction[k].channel != p2->radio_operation_restriction[i].channel_operation_restriction[k].channel
						    || p1->radio_operation_restriction[i].channel_operation_restriction[k].min_freq_separation != p2->radio_operation_restriction[i].channel_operation_restriction[k].min_freq_separation)
							return 1;
					}
				}
			}
		}

		//else
		return 0;
	}
	case TLV_TYPE_TRANSMIT_POWER_LIMIT: {
		struct TransmitPowerLimitTLV *p1, *p2;

		p1 = (struct TransmitPowerLimitTLV *)memory_structure_1;
		p2 = (struct TransmitPowerLimitTLV *)memory_structure_2;

		if (0 != PLATFORM_MEMCMP(p1->radio_unique_id, p2->radio_unique_id, 6)) {
			return 1;
		}
		if (p1->transmit_power_limit != p2->transmit_power_limit) {
			return 1;
		}

		//else
		return 0;
	}
	case TLV_TYPE_CHANNEL_SELECTION_RESPONSE: {
		struct ChannelSelectionResponseTLV *p1, *p2;

		p1 = (struct ChannelSelectionResponseTLV *)memory_structure_1;
		p2 = (struct ChannelSelectionResponseTLV *)memory_structure_2;

		if (0 != PLATFORM_MEMCMP(p1->radio_unique_id, p2->radio_unique_id, 6)) {
			return 1;
		}
		if (p1->response != p2->response) {
			return 1;
		}

		//else
		return 0;
	}
	case TLV_TYPE_OPERATING_CHANNEL_REPORT: {
		struct OperatingChannelReportTLV *p1, *p2;

		p1 = (struct OperatingChannelReportTLV *)memory_structure_1;
		p2 = (struct OperatingChannelReportTLV *)memory_structure_2;

		if (0 != PLATFORM_MEMCMP(p1->radio_unique_id, p2->radio_unique_id, 6)) {
			return 1;
		}
		if (p1->cur_op_class_nr != p2->cur_op_class_nr) {
			return 1;
		} else {
			int i;
			for (i = 0; i < p1->cur_op_class_nr; i++) {
				if (p1->operating_channels[i].op_class != p2->operating_channels[i].op_class
				    || p1->operating_channels[i].cur_channel != p2->operating_channels[i].cur_channel)
					return 1;
			}
		}

		//else
		return 0;
	}
	case TLV_TYPE_STEERING_REQUEST: {
		struct SteeringRequestTLV *p1, *p2;

		p1 = (struct SteeringRequestTLV *)memory_structure_1;
		p2 = (struct SteeringRequestTLV *)memory_structure_2;

		if (0 != PLATFORM_MEMCMP(p1->bssid, p2->bssid, 6)) {
			return 1;
		}
		if (p1->mode != p2->mode || p1->window != p2->window || p1->disassociation_timer != p2->disassociation_timer) {
			return 1;
		}
		if (p1->sta_nr != p2->sta_nr) {
			return 1;
		} else {
			int i;
			for (i = 0; i < p1->sta_nr; i++) {
				if (0 != PLATFORM_MEMCMP(p1->sta_mac_address[i], p2->sta_mac_address[i], 6)) {
					return 1;
				}
			}
		}
		if (p1->target_bss_nr != p2->target_bss_nr) {
			return 1;
		} else {
			int i;
			for (i = 0; i < p1->target_bss_nr; i++) {
				if (0 != PLATFORM_MEMCMP(p1->target_bss[i].bssid, p2->target_bss[i].bssid, 6))
					return 1;
				if (p1->target_bss[i].opclass != p2->target_bss[i].opclass || p1->target_bss[i].channel != p2->target_bss[i].channel) {
					return 1;
				}
			}
		}

		//else
		return 0;
	}
	case TLV_TYPE_STEERING_BTM_REPORT: {
		struct SteeringBTMReportTLV *p1, *p2;

		p1 = (struct SteeringBTMReportTLV *)memory_structure_1;
		p2 = (struct SteeringBTMReportTLV *)memory_structure_2;

		if (0 != PLATFORM_MEMCMP(p1->bssid, p2->bssid, 6)) {
			return 1;
		}
		if (0 != PLATFORM_MEMCMP(p1->sta_mac_address, p2->sta_mac_address, 6)) {
			return 1;
		}
		if (p1->status_code != p2->status_code) {
			return 1;
		}
		if (0 != PLATFORM_MEMCMP(p1->target_bssid, p2->target_bssid, 6)) {
			return 1;
		}

		//else
		return 0;
	}
	case TLV_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST: {
		struct ClientAssociationControlRequestTLV *p1, *p2;

		p1 = (struct ClientAssociationControlRequestTLV *)memory_structure_1;
		p2 = (struct ClientAssociationControlRequestTLV *)memory_structure_2;

		if (0 != PLATFORM_MEMCMP(p1->bssid, p2->bssid, 6)) {
			return 1;
		}
		if (p1->association_control != p2->association_control || p1->validity_period != p2->validity_period) {
			return 1;
		}
		if (p1->sta_nr != p2->sta_nr) {
			return 1;
		} else {
			int i;
			for (i = 0; i < p1->sta_nr; i++) {
				if (0 != PLATFORM_MEMCMP(p1->sta_mac_address[i], p2->sta_mac_address[i], 6)) {
					return 1;
				}
			}
		}

		//else
		return 0;
	}
	case TLV_TYPE_CLIENT_INFO: {
		struct ClientInfoTLV *p1, *p2;

		p1 = (struct ClientInfoTLV *)memory_structure_1;
		p2 = (struct ClientInfoTLV *)memory_structure_2;

		if (PLATFORM_MEMCMP(p1->bssid, p2->bssid, 6) != 0)
			return 1;

		if (PLATFORM_MEMCMP(p1->mac_address, p2->mac_address, 6) != 0)
			return 1;

		return 0;
	}

	case TLV_TYPE_CLIENT_CAPABLITY_REPORT: {

		struct ClientCapabilityReportTLV *p1, *p2;

		p1 = (struct ClientCapabilityReportTLV *)memory_structure_1;
		p2 = (struct ClientCapabilityReportTLV *)memory_structure_2;

		if (p1->result_code != p2->result_code)
			return 1;

		return 0;
	}
	case TLV_TYPE_AP_METRIC_QUERY: {
		struct APMetricQueryTLV *p1, *p2;
		p1 = (struct APMetricQueryTLV *)memory_structure_1;
		p2 = (struct APMetricQueryTLV *)memory_structure_2;

		int k;

		if (p1->bssid_nr != p2->bssid_nr)
			return 1;

		for (k = 0; k < p1->bssid_nr; k++) {
			if (PLATFORM_MEMCMP(p1->bssid[k], p2->bssid[k], 6) != 0)
				return 1;
		}

		return 0;
	}

	case TLV_TYPE_AP_METRICS: {
		struct APMetricsTLV *p1, *p2;
		p1 = (struct APMetricsTLV *)memory_structure_1;
		p2 = (struct APMetricsTLV *)memory_structure_2;

		if (PLATFORM_MEMCMP(p1->bssid, p2->bssid, 6) != 0)
			return 1;

		if (p1->ch_util != p2->ch_util)
			return 1;

		if (p1->sta_nr != p2->sta_nr)
			return 1;

		if (p1->esp_ie != p2->esp_ie)
			return 1;

		if (PLATFORM_MEMCMP(p1->esp_acbe, p2->esp_acbe, 3) != 0)
			return 1;

		if (PLATFORM_MEMCMP(p1->esp_acbk, p2->esp_acbk, 3) != 0)
			return 1;

		if (PLATFORM_MEMCMP(p1->esp_acvo, p2->esp_acvo, 3) != 0)
			return 1;

		if (PLATFORM_MEMCMP(p1->esp_acvi, p2->esp_acvi, 3) != 0)
			return 1;

		return 0;
	}

	case TLV_TYPE_STA_MAC_ADDRESS_TYPE: {
		struct STAMacAddressTypeTLV *p1, *p2;
		p1 = (struct STAMacAddressTypeTLV *)memory_structure_1;
		p2 = (struct STAMacAddressTypeTLV *)memory_structure_2;

		if (PLATFORM_MEMCMP(p1->mac_address, p2->mac_address, 6) != 0)
			return 1;

		return 0;
	}

	case TLV_TYPE_ASSOCIATED_STA_LINK_METRICS: {
		struct AssociatedSTALinkMetricsTLV *p1, *p2;
		int                                 k;
		p1 = (struct AssociatedSTALinkMetricsTLV *)memory_structure_1;
		p2 = (struct AssociatedSTALinkMetricsTLV *)memory_structure_2;

		if (PLATFORM_MEMCMP(p1->mac_address, p2->mac_address, 6) != 0)
			return 1;

		if (p1->bssid_nr != p2->bssid_nr)
			return 1;

		for (k = 0; k < p1->bssid_nr; k++) {
			if (PLATFORM_MEMCMP(p1->assoc_sta_link_metrics[k].bssid, p2->assoc_sta_link_metrics[k].bssid, 6) != 0)
				return 1;
			if (p1->assoc_sta_link_metrics[k].time_delta != p2->assoc_sta_link_metrics[k].time_delta)
				return 1;
			if (p1->assoc_sta_link_metrics[k].dataRate_downlink != p2->assoc_sta_link_metrics[k].dataRate_downlink)
				return 1;

			if (p1->assoc_sta_link_metrics[k].dataRate_uplink != p2->assoc_sta_link_metrics[k].dataRate_uplink)
				return 1;

			if (p1->assoc_sta_link_metrics[k].uplink_rcpi != p2->assoc_sta_link_metrics[k].uplink_rcpi)
				return 1;
		}

		return 0;
	}
	case TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS: {
		struct AssociatedSTATrafficStatsTLV *p1, *p2;
		p1 = (struct AssociatedSTATrafficStatsTLV *)memory_structure_1;
		p2 = (struct AssociatedSTATrafficStatsTLV *)memory_structure_2;

		if (PLATFORM_MEMCMP(p1->sta_mac_address, p2->sta_mac_address, 6) != 0)
			return 1;

		if (p1->bytes_sent != p2->bytes_sent)
			return 1;

		if (p1->bytes_received != p2->bytes_received)
			return 1;

		if (p1->packets_sent != p2->packets_sent)
			return 1;

		if (p1->packets_received != p2->packets_received)
			return 1;

		if (p1->tx_packets_errors != p2->tx_packets_errors)
			return 1;

		if (p1->rx_packets_errors != p2->rx_packets_errors)
			return 1;

		if (p1->retransmission_count != p2->retransmission_count)
			return 1;

		return 0;
	}
	case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY: {
		struct UnassociatedSTALinkMetricsQueryTLV *p1, *p2;
		int                                        k, j;
		p1 = (struct UnassociatedSTALinkMetricsQueryTLV *)memory_structure_1;
		p2 = (struct UnassociatedSTALinkMetricsQueryTLV *)memory_structure_2;

		if (p1->op_class != p2->op_class)
			return 1;

		if (p1->channel_list_nr != p2->channel_list_nr)
			return 1;

		for (k = 0; k < p1->channel_list_nr; k++) {

			if (p1->channel_list_entry[k].channel_nr != p2->channel_list_entry[k].channel_nr)
				return 1;

			if (p1->channel_list_entry[k].sta_nr != p2->channel_list_entry[k].sta_nr)
				return 1;

			for (j = 0; j < p1->channel_list_entry[k].sta_nr; j++) {
				if (PLATFORM_MEMCMP(p1->channel_list_entry[k].sta_mac_address[j], p2->channel_list_entry[k].sta_mac_address[j], 6) != 0)
					return 1;
			}
		}

		return 0;
	}

	case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE: {
		struct UnassociatedSTALinkMetricsResponseTLV *p1, *p2;
		int                                           k;
		p1 = (struct UnassociatedSTALinkMetricsResponseTLV *)memory_structure_1;
		p2 = (struct UnassociatedSTALinkMetricsResponseTLV *)memory_structure_2;

		if (p1->op_class != p2->op_class)
			return 1;

		if (p1->sta_nr != p2->sta_nr)
			return 1;

		for (k = 0; k < p1->sta_nr; k++) {
			if (PLATFORM_MEMCMP(p1->unassoc_sta_link_metrics[k].sta_mac_address, p2->unassoc_sta_link_metrics[k].sta_mac_address, 6) != 0)
				return 1;
			if (p1->unassoc_sta_link_metrics[k].channel_number != p2->unassoc_sta_link_metrics[k].channel_number)
				return 1;
			if (p1->unassoc_sta_link_metrics[k].time_delta != p2->unassoc_sta_link_metrics[k].time_delta)
				return 1;
			if (p1->unassoc_sta_link_metrics[k].uplink_rcpi != p2->unassoc_sta_link_metrics[k].uplink_rcpi)
				return 1;
		}

		return 0;
	}

	case TLV_TYPE_BEACON_METRICS_QUERY: {
		struct BeaconMetricsQueryTLV *p1, *p2;
		int                           k;
		p1 = (struct BeaconMetricsQueryTLV *)memory_structure_1;
		p2 = (struct BeaconMetricsQueryTLV *)memory_structure_2;

		if (PLATFORM_MEMCMP(p1->mac_address, p2->mac_address, 6) != 0)
			return 1;

		if (p1->op_class != p2->op_class)
			return 1;

		if (p1->channel_number != p2->channel_number)
			return 1;

		if (PLATFORM_MEMCMP(p1->bssid, p2->bssid, 6) != 0)
			return 1;

		if (p1->report_detail != p2->report_detail)
			return 1;

		if (p1->ssid_len != p2->ssid_len)
			return 1;

		if (PLATFORM_MEMCMP(p1->ssid, p2->ssid, p1->ssid_len) != 0)
			return 1;

		if (p1->ap_channel_report_nr != p2->ap_channel_report_nr)
			return 1;

		for (k = 0; k < p1->ap_channel_report_nr; k++) {

			if (p1->ap_channel_report[k].len != p2->ap_channel_report[k].len)
				return 1;

			if (p1->ap_channel_report[k].op_class != p2->ap_channel_report[k].op_class)
				return 1;

			if (PLATFORM_MEMCMP(p1->ap_channel_report[k].channel_list, p2->ap_channel_report[k].channel_list, (p1->ap_channel_report[k].len - 1)) != 0)
				return 1;
		}

		if (p1->elementID_nr != p2->elementID_nr)
			return 1;

		if (PLATFORM_MEMCMP(p1->element_list, p2->element_list, p1->elementID_nr) != 0)
			return 1;

		return 0;
	}

	case TLV_TYPE_BEACON_METRICS_RESPONSE: {
		struct BeaconMetricsResponseTLV *p1, *p2;
		// int                              k;
		p1 = (struct BeaconMetricsResponseTLV *)memory_structure_1;
		p2 = (struct BeaconMetricsResponseTLV *)memory_structure_2;

		if ((PLATFORM_MEMCMP(p1->mac_address, p2->mac_address, 6)) != 0)
			return 1;

		if (p1->reserved_field != p2->reserved_field)
			return 1;

		if (p1->measurement_report_nr != p2->measurement_report_nr)
			return 1;

		// for (k = 0; k < p1->measurement_report_nr; k++) {

		// 	if (p1->measurement_reports[k].elementId != p2->measurement_reports[k].elementId)
		// 		return 1;
		// 	if (p1->measurement_reports[k].len != p2->measurement_reports[k].len)
		// 		return 1;
		// 	if (p1->measurement_reports[k].measurementToken != p2->measurement_reports[k].measurementToken)
		// 		return 1;
		// 	if (p1->measurement_reports[k].measurementReportMode != p2->measurement_reports[k].measurementReportMode)
		// 		return 1;
		// 	if (p1->measurement_reports[k].measurementType != p2->measurement_reports[k].measurementType)
		// 		return 1;

		// 	if (p1->measurement_reports[k].info.op_class != p2->measurement_reports[k].info.op_class)
		// 		return 1;
		// 	if (p1->measurement_reports[k].info.channel != p2->measurement_reports[k].info.channel)
		// 		return 1;
		// 	if (p1->measurement_reports[k].info.measure_time_hi != p2->measurement_reports[k].info.measure_time_hi)
		// 		return 1;
		// 	if (p1->measurement_reports[k].info.measure_time_lo != p2->measurement_reports[k].info.measure_time_lo)
		// 		return 1;
		// 	if (p1->measurement_reports[k].info.measure_duration != p2->measurement_reports[k].info.measure_duration)
		// 		return 1;
		// 	if (p1->measurement_reports[k].info.frame_info != p2->measurement_reports[k].info.frame_info)
		// 		return 1;
		// 	if (p1->measurement_reports[k].info.RCPI != p2->measurement_reports[k].info.RCPI)
		// 		return 1;
		// 	if (p1->measurement_reports[k].info.RSNI != p2->measurement_reports[k].info.RSNI)
		// 		return 1;
		// 	if (PLATFORM_MEMCMP(p1->measurement_reports[k].info.bssid, p2->measurement_reports[k].info.bssid, 6) != 0)
		// 		return 1;
		// 	if (p1->measurement_reports[k].info.antenna_id != p2->measurement_reports[k].info.antenna_id)
		// 		return 1;
		// 	if (p1->measurement_reports[k].info.parent_tsf != p2->measurement_reports[k].info.parent_tsf)
		// 		return 1;

		// 	if (p1->measurement_reports[k].subelements_len != p2->measurement_reports[k].subelements_len)
		// 		return 1;
		// 	if (PLATFORM_MEMCMP(p1->measurement_reports[k].subelements, p2->measurement_reports[k].subelements, 6) != 0)
		// 		return 1;
		// }

		return 0;
	}

	case TLV_TYPE_BACKHAUL_STEERING_REQUEST: {
		struct BackhaulSteeringRequestTLV *p1, *p2;

		p1 = (struct BackhaulSteeringRequestTLV *)memory_structure_1;
		p2 = (struct BackhaulSteeringRequestTLV *)memory_structure_2;

		if (PLATFORM_MEMCMP(p1->backhaul_mac, p2->backhaul_mac, 6) != 0)
			return 1;

		if (PLATFORM_MEMCMP(p1->target_bssid, p2->target_bssid, 6) != 0)
			return 1;

		if (p1->op_class != p2->op_class)
			return 1;

		if (p1->channel_number != p2->channel_number)
			return 1;

		return 0;
	}

	case TLV_TYPE_BACKHAUL_STEERING_RESPONSE: {
		struct BackhaulSteeringResponseTLV *p1, *p2;

		p1 = (struct BackhaulSteeringResponseTLV *)memory_structure_1;
		p2 = (struct BackhaulSteeringResponseTLV *)memory_structure_2;

		if (PLATFORM_MEMCMP(p1->backhaul_mac, p2->backhaul_mac, 6) != 0)
			return 1;

		if (PLATFORM_MEMCMP(p1->target_bssid, p2->target_bssid, 6) != 0)
			return 1;

		if (p1->result_code != p2->result_code)
			return 1;

		return 0;
	}

	case TLV_TYPE_HIGHER_LAYER_DATA: {
		struct HigherLayerDataTLV *p1, *p2;
		p1 = (struct HigherLayerDataTLV *)memory_structure_1;
		p2 = (struct HigherLayerDataTLV *)memory_structure_2;
		if (p1->tlv_type != p2->tlv_type || p1->protocol != p2->protocol) {
			return 1;
		}
		return 0;
	}

		////////////////////////////////////////////////////////////////////////////////
		/// Profile 2 TLVs
		////////////////////////////////////////////////////////////////////////////////

	case TLV_TYPE_1905_LAYER_SECURITY_CAPABILITY: {
		struct SecurityCapabilityTLV *p1, *p2;
		p1 = (struct SecurityCapabilityTLV *)memory_structure_1;
		p2 = (struct SecurityCapabilityTLV *)memory_structure_2;
		if (p1->tlv_type != p2->tlv_type || p1->onboarding_protocol_supported != p2->onboarding_protocol_supported) {
			return 1;
		}
		if (p1->message_integrity_algorithm_supported != p2->message_integrity_algorithm_supported || p1->message_encryption_algorithm_supported != p2->message_encryption_algorithm_supported) {
			return 1;
		}
		return 0;
	}

	case TLV_TYPE_MIC: {
		struct MICTLV *p1, *p2;
		p1 = (struct MICTLV *)memory_structure_1;
		p2 = (struct MICTLV *)memory_structure_2;
		if (p1->tlv_type != p2->tlv_type || p1->gtk_key_id_ver != p2->gtk_key_id_ver) {
			return 1;
		}
		if (p1->integrity_transmission_counter != p2->integrity_transmission_counter) {
			return 1;
		}
		if (PLATFORM_MEMCMP(p1->src_1905_al_mac_addr, p2->src_1905_al_mac_addr, 6) != 0) {
			return 1;
		}
		if (p1->mic_length != p2->mic_length) {
			return 1;
		}
		if (PLATFORM_MEMCMP(p1->mic, p2->mic, p1->mic_length) != 0) {
			return 1;
		}
		return 0;
	}

	case TLV_TYPE_PROFILE_2_CAPABILITY: {
		struct Profile2APCapabilityTLV *p1, *p2;
		p1 = (struct Profile2APCapabilityTLV *)memory_structure_1;
		p2 = (struct Profile2APCapabilityTLV *)memory_structure_2;
		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (p1->max_total_nr_service_prioritization_rules != p2->max_total_nr_service_prioritization_rules) {
			return 1;
		}
		// if (p1->max_nr_advance_service_prioritization_rules != p2->max_nr_advance_service_prioritization_rules) {
		// 	return 1;
		// }
		// if (p1->max_nr_permitted_destination_mac_addresses != p2->max_nr_permitted_destination_mac_addresses) {
		// 	return 1;
		// }
		if (p1->units != p2->units || p1->max_total_nr_VIDs != p2->max_total_nr_VIDs) {
			return 1;
		}
		// if (p1->eth_nr != p2->eth_nr) {
		// 	return 1;
		// } else {
		// 	int i;
		// 	for (i = 0; i < p1->eth_nr; i++) {
		// 		if (0 != PLATFORM_MEMCMP(p1->interface_identifier[i], p2->interface_identifier[i], 6))
		// 			return 1;
		// 	}
		// }
		return 0;
	}

	case TLV_TYPE_DEFAULT_802_1Q_SETTINGS: {
		struct Default8021QSettingsTLV *p1, *p2;
		p1 = (struct Default8021QSettingsTLV *)memory_structure_1;
		p2 = (struct Default8021QSettingsTLV *)memory_structure_2;
		if (p1->tlv_type != p2->tlv_type || p1->primary_vlan_id != p2->primary_vlan_id) {
			return 1;
		}
		if (p1->primary_vlan_id != p2->primary_vlan_id) {
			return 1;
		}
		if (p1->default_pcp != p2->default_pcp) {
			return 1;
		}
		return 0;
	}

	case TLV_TYPE_TRAFFIC_SEPARATION_POLICY: {
		struct TrafficSeparationPolicyTLV *p1, *p2;
		int                                k;
		p1 = (struct TrafficSeparationPolicyTLV *)memory_structure_1;
		p2 = (struct TrafficSeparationPolicyTLV *)memory_structure_2;
		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (p1->ssid_nr != p2->ssid_nr) {
			return 1;
		}
		for (k = 0; k < p1->ssid_nr; k++) {
			if (p1->ssid_info[k].ssid_length != p2->ssid_info[k].ssid_length) {
				return 1;
			}
			if (PLATFORM_MEMCMP(p1->ssid_info[k].ssid_name, p2->ssid_info[k].ssid_name, p1->ssid_info[k].ssid_length) != 0) {
				return 1;
			}
			if (p1->ssid_info[k].vlan_id != p2->ssid_info[k].vlan_id) {
				return 1;
			}
		}
		return 0;
	}

	case TLV_TYPE_BSS_CONFIG_REPORT: {
		struct BSSConfigReportTLV *  p1, *p2;
		struct BSSConfigRadioReport *rp1, *rp2;
		struct BSSConfigBSSReport *  bp1, *bp2;

		p1 = (struct BSSConfigReportTLV *)memory_structure_1;
		p2 = (struct BSSConfigReportTLV *)memory_structure_2;

		INT8U k, m;

		if (p1->tlv_type != p2->tlv_type)
			return 1;
		if (p1->radio_report_nr != p2->radio_report_nr)
			return 1;

		for (k = 0; k < p1->radio_report_nr; k++) {
			rp1 = &p1->radio_reports[k];
			rp2 = &p2->radio_reports[k];
			if (rp1->bss_report_nr != rp2->bss_report_nr)
				return 1;
			if (PLATFORM_MEMCMP(rp1->radio_unique_identifier, rp2->radio_unique_identifier, 6) != 0)
				return 1;
			for (m = 0; m < rp1->bss_report_nr; m++) {
				bp1 = &rp1->bss_reports[m];
				bp2 = &rp2->bss_reports[m];
				if (PLATFORM_MEMCMP(bp1->bssid, bp2->bssid, 6) != 0)
					return 1;
				if (bp1->status_flag != bp2->status_flag)
					return 1;
				if (bp1->ssid_length != bp2->ssid_length)
					return 1;
				if (bp1->ssid_length
				    && bp2->ssid_length
				    && PLATFORM_MEMCMP(bp1->ssid, bp2->ssid, bp1->ssid_length) != 0)
					return 1;
			}
		}

		return 0;
	}

	case TLV_TYPE_SERVICE_PRIORITIZATION_RULE: {
		struct ServicePrioritizationRuleTLV *p1, *p2;
		p1 = (struct ServicePrioritizationRuleTLV *)memory_structure_1;
		p2 = (struct ServicePrioritizationRuleTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}

		if (p1->service_prioritization_rule_id != p2->service_prioritization_rule_id) {
			return 1;
		}
		if (p1->add_remove_rule != p2->add_remove_rule) {
			return 1;
		}
		if (p1->rule_precedence != p2->rule_precedence) {
			return 1;
		}
		if (p1->rule_output != p2->rule_output) {
			return 1;
		}
		if (p1->always_match != p2->always_match) {
			return 1;
		}

		return 0;
	}

	case TLV_TYPE_DSCP_MAPPING_TABLE: {
		struct DSCPMappingTableTLV *p1, *p2;
		p1 = (struct DSCPMappingTableTLV *)memory_structure_1;
		p2 = (struct DSCPMappingTableTLV *)memory_structure_2;
		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (0 != PLATFORM_MEMCMP(p1->dscp_to_pcp, p2->dscp_to_pcp, 64)) {
			return 1;
		}
		return 0;
	}

	case TLV_TYPE_BSS_CONFIG_REQUEST: {
		struct BSSConfigRequestTLV *p1, *p2;
		p1 = (struct BSSConfigRequestTLV *)memory_structure_1;
		p2 = (struct BSSConfigRequestTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type)
			return 1;
		if (p1->dpp_config_request_object_len
		    != p2->dpp_config_request_object_len)
			return 1;
		if (p1->dpp_config_request_object_len
		    && p2->dpp_config_request_object_len
		    && PLATFORM_MEMCMP(p1->dpp_config_request_object,
		                       p2->dpp_config_request_object,
		                       p1->dpp_config_request_object_len)
		           != 0)
			return 1;

		return 0;
	}

	case TLV_TYPE_BSS_CONFIG_RESPONSE: {
		struct BSSConfigResponseTLV *p1, *p2;
		p1 = (struct BSSConfigResponseTLV *)memory_structure_1;
		p2 = (struct BSSConfigResponseTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type)
			return 1;
		if (p1->dpp_config_object_len != p2->dpp_config_object_len)
			return 1;
		if (p1->dpp_config_object_len
		    && p2->dpp_config_object_len
		    && PLATFORM_MEMCMP(p1->dpp_config_object,
		                       p2->dpp_config_object,
		                       p1->dpp_config_object_len)
		           != 0)
			return 1;

		return 0;
	}

	case TLV_TYPE_PROFILE_2_ERROR_CODE: {
		struct Profile2ErrorCodeTLV *p1, *p2;
		p1 = (struct Profile2ErrorCodeTLV *)memory_structure_1;
		p2 = (struct Profile2ErrorCodeTLV *)memory_structure_2;
		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (p1->reason_code != p2->reason_code) {
			return 1;
		}
		if (p1->service_prioritization_rule_id != p2->service_prioritization_rule_id) {
			return 1;
		}

		return 0;
	}

	case TLV_TYPE_AP_RADIO_ADVANCED_CAPABILITIES: {
		struct APRadioAdvancedCapabilitiesTLV *p1, *p2;
		p1 = (struct APRadioAdvancedCapabilitiesTLV *)memory_structure_1;
		p2 = (struct APRadioAdvancedCapabilitiesTLV *)memory_structure_2;
		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (0 != PLATFORM_MEMCMP(p1->radio_id, p2->radio_id, 6)) {
			return 1;
		}
		if (p1->backhaul_bss_traffic_separation_mix_no_support != p2->backhaul_bss_traffic_separation_mix_no_support) {
			return 1;
		}

		return 0;
	}

	case TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION: {
		struct AssociationStatusNotificationTLV *p1, *p2;
		int                                      k;
		p1 = (struct AssociationStatusNotificationTLV *)memory_structure_1;
		p2 = (struct AssociationStatusNotificationTLV *)memory_structure_2;
		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (p1->bssid_nr != p2->bssid_nr) {
			return 1;
		}
		for (k = 0; k < p1->bssid_nr; k++) {
			if (PLATFORM_MEMCMP(p1->bss_info[k].bssid, p2->bss_info[k].bssid, 6) != 0) {
				return 1;
			}
			if (p1->bss_info[k].association_allowance_status != p2->bss_info[k].association_allowance_status) {
				return 1;
			}
		}
		return 0;
	}

	case TLV_TYPE_SOURCE_INFO: {
		struct SourceInfoTLV *p1, *p2;
		p1 = (struct SourceInfoTLV *)memory_structure_1;
		p2 = (struct SourceInfoTLV *)memory_structure_2;
		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (PLATFORM_MEMCMP(p1->mac_address, p2->mac_address, 6) != 0) {
			return 1;
		}
		return 0;
	}

	case TLV_TYPE_TUNNELED_MESSAGE_TYPE: {
		struct TunneledMessageTypeTLV *p1, *p2;
		p1 = (struct TunneledMessageTypeTLV *)memory_structure_1;
		p2 = (struct TunneledMessageTypeTLV *)memory_structure_2;
		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (p1->tunneled_protocol_type_payload != p2->tunneled_protocol_type_payload) {
			return 1;
		}
		return 0;
	}
	case TLV_TYPE_TUNNELED: {
		struct TunneledTLV *p1, *p2;

		p1 = (struct TunneledTLV *)memory_structure_1;
		p2 = (struct TunneledTLV *)memory_structure_2;
		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (p1->tlv_length != p2->tlv_length) {
			return 1;
		}
		if (PLATFORM_MEMCMP(p1->payload, p2->payload, p1->tlv_length) != 0) {
			return 1;
		}
		return 0;
	}

	case TLV_TYPE_PROFILE_2_STEERING_REQUEST: {
		struct Profile2SteeringRequestTLV *p1, *p2;
		int                                k;
		p1 = (struct Profile2SteeringRequestTLV *)memory_structure_1;
		p2 = (struct Profile2SteeringRequestTLV *)memory_structure_2;
		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (PLATFORM_MEMCMP(p1->bssid, p2->bssid, 6) != 0) {
			return 1;
		}
		if (p1->steer_parameters != p2->steer_parameters) {
			return 1;
		}
		if (p1->steer_opportunity_window != p2->steer_opportunity_window) {
			return 1;
		}
		if (p1->btm_disassociation_timer != p2->btm_disassociation_timer) {
			return 1;
		}
		if (p1->sta_list_count != p2->sta_list_count) {
			return 1;
		}
		for (k = 0; k < p1->sta_list_count; k++) {
			if (PLATFORM_MEMCMP(p1->sta_list[k], p2->sta_list[k], 6) != 0) {
				return 1;
			}
		}
		if (p1->target_bssid_list_count != p2->target_bssid_list_count) {
			return 1;
		}
		for (k = 0; k < p1->target_bssid_list_count; k++) {
			if (PLATFORM_MEMCMP(p1->target_bssid_list[k].target_bssid, p2->target_bssid_list[k].target_bssid, 6) != 0) {
				return 1;
			}
			if (p1->target_bssid_list[k].target_bss_operating_class != p2->target_bssid_list[k].target_bss_operating_class) {
				return 1;
			}
			if (p1->target_bssid_list[k].target_bss_channel_number != p2->target_bssid_list[k].target_bss_channel_number) {
				return 1;
			}
			if (p1->target_bssid_list[k].reason_code != p2->target_bssid_list[k].reason_code) {
				return 1;
			}
		}
		return 0;
	}

	case TLV_TYPE_UNSUCCESSFUL_ASSOCIATION_POILCY: {
		struct UnsuccessfulAssociationPolicyTLV *p1, *p2;
		p1 = (struct UnsuccessfulAssociationPolicyTLV *)memory_structure_1;
		p2 = (struct UnsuccessfulAssociationPolicyTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (p1->report_unsuccessful_associations != p2->report_unsuccessful_associations) {
			return 1;
		}
		if (p1->maximum_reporting_rate != p2->maximum_reporting_rate) {
			return 1;
		}

		return 0;
	}

	case TLV_TYPE_METRIC_COLLECTION_INTERVAL: {
		struct MetricCollectionIntervalTLV *p1, *p2;
		p1 = (struct MetricCollectionIntervalTLV *)memory_structure_1;
		p2 = (struct MetricCollectionIntervalTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (p1->collection_interval != p2->collection_interval) {
			return 1;
		}

		return 0;
	}

	case TLV_TYPE_RADIO_METRICS: {
		struct RadioMetricsTLV *p1, *p2;
		p1 = (struct RadioMetricsTLV *)memory_structure_1;
		p2 = (struct RadioMetricsTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if ((PLATFORM_MEMCMP(p1->radio_unique_identifier, p2->radio_unique_identifier, 6) != 0)) {
			return 1;
		}
		if (p1->noise != p2->noise) {
			return 1;
		}
		if (p1->transmit != p2->transmit) {
			return 1;
		}
		if (p1->receiveself != p2->receiveself) {
			return 1;
		}
		if (p1->receiveother != p2->receiveother) {
			return 1;
		}

		return 0;
	}

	case TLV_TYPE_AP_EXTENDED_METRICS: {
		struct APExtendedMetricsTLV *p1, *p2;
		p1 = (struct APExtendedMetricsTLV *)memory_structure_1;
		p2 = (struct APExtendedMetricsTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if ((PLATFORM_MEMCMP(p1->radio_unique_identifier, p2->radio_unique_identifier, 6) != 0)) {
			return 1;
		}
		if (p1->unicast_byte_sent != p2->unicast_byte_sent) {
			return 1;
		}
		if (p1->unicast_byte_received != p2->unicast_byte_received) {
			return 1;
		}
		if (p1->multicast_byte_sent != p2->multicast_byte_sent) {
			return 1;
		}
		if (p1->multicast_byte_received != p2->multicast_byte_received) {
			return 1;
		}
		if (p1->broadcast_byte_sent != p2->broadcast_byte_sent) {
			return 1;
		}
		if (p1->broadcast_byte_received != p2->broadcast_byte_received) {
			return 1;
		}

		return 0;
	}

	case TLV_TYPE_ASSOCIATED_STA_EXTENDED_LINK_METRICS: {
		struct AssociatedSTAExtendedLinkMericsTLV *p1, *p2;
		int                                        k;
		p1 = (struct AssociatedSTAExtendedLinkMericsTLV *)memory_structure_1;
		p2 = (struct AssociatedSTAExtendedLinkMericsTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if ((PLATFORM_MEMCMP(p1->sta_mac_address, p2->sta_mac_address, 6) != 0)) {
			return 1;
		}
		if (p1->bssid_nr != p2->bssid_nr) {
			return 1;
		}
		for (k = 0; k < p1->bssid_nr; k++) {
			if (PLATFORM_MEMCMP(p1->reported_bss[k].bssid, p2->reported_bss[k].bssid, 6) != 0) {
				return 1;
			}
			if (p1->reported_bss[k].last_data_downlink_rate != p2->reported_bss[k].last_data_downlink_rate) {
				return 1;
			}
			if (p1->reported_bss[k].last_data_uplink_rate != p2->reported_bss[k].last_data_uplink_rate) {
				return 1;
			}
			if (p1->reported_bss[k].utilization_receive != p2->reported_bss[k].utilization_receive) {
				return 1;
			}
			if (p1->reported_bss[k].utilization_transmit != p2->reported_bss[k].utilization_transmit) {
				return 1;
			}
		}

		return 0;
	}

	case TLV_TYPE_STATUS_CODE: {
		struct StatusCodeTLV *p1, *p2;
		p1 = (struct StatusCodeTLV *)memory_structure_1;
		p2 = (struct StatusCodeTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (p1->status_code != p2->status_code) {
			return 1;
		}

		return 0;
	}

	case TLV_TYPE_REASON_CODE: {
		struct ReasonCodeTLV *p1, *p2;
		p1 = (struct ReasonCodeTLV *)memory_structure_1;
		p2 = (struct ReasonCodeTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (p1->reason_code != p2->reason_code) {
			return 1;
		}

		return 0;
	}

	case TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES: {
		struct BackhaulSTARadioCapabilitiesTLV *p1, *p2;
		p1 = (struct BackhaulSTARadioCapabilitiesTLV *)memory_structure_1;
		p2 = (struct BackhaulSTARadioCapabilitiesTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (PLATFORM_MEMCMP(p1->radio_unique_identifier, p2->radio_unique_identifier, 6) != 0) {
			return 1;
		}
		if (p1->included_mac != p2->included_mac) {
			return 1;
		}
		if (PLATFORM_MEMCMP(p1->sta_mac_address, p2->sta_mac_address, 6) != 0) {
			return 1;
		}

		return 0;
	}

	case TLV_TYPE_AKM_SUITE_CAPABILITIES: {
		struct AKMSuiteCapabilitiesTLV *p1, *p2;
		INT8U                           i;
		p1 = (struct AKMSuiteCapabilitiesTLV *)memory_structure_1;
		p2 = (struct AKMSuiteCapabilitiesTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (p1->backhaul_bss_akm_suite_selectors_nr != p2->backhaul_bss_akm_suite_selectors_nr) {
			return 1;
		}
		if (p1->fronthaul_bss_akm_suite_selectors_nr != p2->fronthaul_bss_akm_suite_selectors_nr) {
			return 1;
		}
		for (i = 0; i < p1->backhaul_bss_akm_suite_selectors_nr; i++) {
			if (PLATFORM_MEMCMP(&p1->backhaul_bss_akm_suite_selectors[i],
			                    &p2->backhaul_bss_akm_suite_selectors[i],
			                    sizeof(struct AKMSuiteSelector))
			    != 0) {
				return 1;
			}
		}
		for (i = 0; i < p1->fronthaul_bss_akm_suite_selectors_nr; i++) {
			if (PLATFORM_MEMCMP(&p1->fronthaul_bss_akm_suite_selectors[i],
			                    &p2->fronthaul_bss_akm_suite_selectors[i],
			                    sizeof(struct AKMSuiteSelector))
			    != 0) {
				return 1;
			}
		}

		return 0;
	}

	case TLV_TYPE_1905_ENCAP_DPP: {
		struct Encap1905DPPTLV *p1, *p2;
		p1 = (struct Encap1905DPPTLV *)memory_structure_1;
		p2 = (struct Encap1905DPPTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (p1->enrollee_mac_address_present != p2->enrollee_mac_address_present) {
			return 1;
		}
		if (PLATFORM_MEMCMP(p1->dest_sta_mac_address, p2->dest_sta_mac_address, 6) != 0) {
			return 1;
		}
		if (p1->frame_type != p2->frame_type) {
			return 1;
		}
		if (p1->encapsulated_frame_len != p2->encapsulated_frame_len) {
			return 1;
		}
		if (PLATFORM_MEMCMP(p1->encapsulated_frame, p2->encapsulated_frame, p1->encapsulated_frame_len) != 0) {
			return 1;
		}

		return 0;
	}

	case TLV_TYPE_1905_ENCAP_EAPOL: {
		struct Encap1905EAPOLTLV *p1, *p2;
		p1 = (struct Encap1905EAPOLTLV *)memory_structure_1;
		p2 = (struct Encap1905EAPOLTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (PLATFORM_MEMCMP(p1->eapol_frame_payload, p2->eapol_frame_payload, p1->eapol_frame_payload_len) != 0) {
			return 1;
		}

		return 0;
	}

	case TLV_TYPE_DPP_BOOTSTRAPPING_URI_NOTIFICATION: {
		struct DPPBootstrappingURINotificationTLV *p1, *p2;
		p1 = (struct DPPBootstrappingURINotificationTLV *)memory_structure_1;
		p2 = (struct DPPBootstrappingURINotificationTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (PLATFORM_MEMCMP(p1->radio_unique_identifier, p2->radio_unique_identifier, 6) != 0) {
			return 1;
		}
		if (PLATFORM_MEMCMP(p1->local_interface_mac_address, p2->local_interface_mac_address, 6) != 0) {
			return 1;
		}
		if (PLATFORM_MEMCMP(p1->sta_mac_address, p2->sta_mac_address, 6) != 0) {
			return 1;
		}
		if (PLATFORM_MEMCMP(p1->dpp_bootstrapping_uri, p2->dpp_bootstrapping_uri, p2->dpp_bootstrapping_uri_len) != 0) {
			return 1;
		}

		return 0;
	}

	case TLV_TYPE_DPP_CCE_INDICATION: {
		struct DPPCCEIndicationTLV *p1, *p2;
		p1 = (struct DPPCCEIndicationTLV *)memory_structure_1;
		p2 = (struct DPPCCEIndicationTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (p1->advertise_cce != p2->advertise_cce) {
			return 1;
		}

		return 0;
	}

	case TLV_TYPE_BACKHAUL_BSS_CONFIGURATION: {
		struct BackhaulBSSConfigurationTLV *p1, *p2;
		p1 = (struct BackhaulBSSConfigurationTLV *)memory_structure_1;
		p2 = (struct BackhaulBSSConfigurationTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type) {
			return 1;
		}
		if (PLATFORM_MEMCMP(p1->bssid, p2->bssid, 6) != 0) {
			return 1;
		}
		if (p1->association_disallowed != p2->association_disallowed)
			return 1;

		return 0;
	}

	case TLV_TYPE_DPP_CHIRP_VALUE: {
		struct DPPChirpValueTLV *p1, *p2;
		p1 = (struct DPPChirpValueTLV *)memory_structure_1;
		p2 = (struct DPPChirpValueTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type)
			return 1;
		if (p1->hash_validity != p2->hash_validity)
			return 1;
		if (p1->hash_validity & DPP_CHIRP_ENROLLEE_ADDR_PRESENT) {
			if (PLATFORM_MEMCMP(p1->sta_mac_address, p2->sta_mac_address, 6) != 0)
				return 1;
		}
		if (p1->hash_len != p2->hash_len)
			return 1;
		if (PLATFORM_MEMCMP(p1->hash_value, p2->hash_value, p1->hash_len) != 0)
			return 1;

		return 0;
	}

	case TLV_TYPE_DPP_MESSAGE: {
		struct DPPMessageTLV *p1, *p2;
		p1 = (struct DPPMessageTLV *)memory_structure_1;
		p2 = (struct DPPMessageTLV *)memory_structure_2;

		if (p1->dpp_frame_len != p2->dpp_frame_len)
			return 1;
		if (PLATFORM_MEMCMP(p1->dpp_frame, p2->dpp_frame, p1->dpp_frame_len) != 0)
			return 1;

		return 0;
	}
	case TLV_TYPE_ANTICIPATED_CHANNEL_PERFERENCE: {
		struct AnticipatedChannelPerferenceTLV * p1, *p2;
		struct OperatingChannelClass *op1, *op2;

		p1 = (struct AnticipatedChannelPerferenceTLV *)memory_structure_1;
		p2 = (struct AnticipatedChannelPerferenceTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type)
			return 1;
		
		int k;
		for (k = 0; k < p1->operating_class_nr; k++) {
			op1 = &p1->operate_class[k];
			op2 = &p2->operate_class[k];
			if (op1->operating_class != op2->operating_class)
				return 1;
			if (op1->channel_list_nr != op2->channel_list_nr)
				return 1;
			if (op1->channel_list_nr && op2->channel_list_nr && PLATFORM_MEMCMP(op1->channel_list, op2->channel_list, op1->channel_list_nr) != 0)
				return 1;
		}
		return 0;
	}


	case TLV_TYPE_ANTICIPATED_CHANNEL_USAGE: {
		struct AnticipatedChannelUsageTLV * p1, *p2;
		struct UsageEntryClass *up1, *up2;

		p1 = (struct AnticipatedChannelUsageTLV *)memory_structure_1;
		p2 = (struct AnticipatedChannelUsageTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type)
			return 1;
		
		if (p1->operating_class != p2->operating_class)
			return 1;
		if (p1->channel_number != p2->channel_number)
			return 1;
		if (PLATFORM_MEMCMP(p1->reference_bssid, p2->reference_bssid, 6) != 0)
			return 1;
		int k;
		for (k = 0; k < p1->usage_entry_nr; k++) {
			up1 = &p1->usage_entry[k];
			up2 = &p2->usage_entry[k];
			if (up1->burst_start_time != up2->burst_start_time)
				return 1;
			if (up1->burst_length != up2->burst_length)
				return 1;
			if (up1->repititions != up2->repititions)
				return 1;
			if (up1->burst_interval != up2->burst_interval)
				return 1;
			if (up1->RU_bitmask_length != up2->RU_bitmask_length)
				return 1;
			if (up1->RU_bitmask_length && up2->RU_bitmask_length && PLATFORM_MEMCMP(up1->RU_bitmask, up2->RU_bitmask, up1->RU_bitmask_length) != 0)
				return 1;
			if (PLATFORM_MEMCMP(up1->transmitter_identifier, up2->transmitter_identifier, 6) != 0)
				return 1;
			if (up1->power_level != up2->power_level)
				return 1;
			if (up1->channel_usage_reason != up2->channel_usage_reason)
				return 1;
		}
		return 0;
	}

	case TLV_TYPE_SPATIAL_REUSE_REQUEST: {
		struct SpatialReuseRequestTLV * p1, *p2;

		p1 = (struct SpatialReuseRequestTLV *)memory_structure_1;
		p2 = (struct SpatialReuseRequestTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type)
			return 1;
		
		if (PLATFORM_MEMCMP(p1->ruid, p2->ruid, 6) != 0)
			return 1;
		if (p1->bss_color != p2->bss_color)
			return 1;
		if (p1->valid_field != p2->valid_field)
			return 1;
		if (p1->non_srg_obsspd_max_offset != p2->non_srg_obsspd_max_offset)
			return 1;
		if (p1->srg_obsspd_min_offset != p2->srg_obsspd_min_offset)
			return 1;
		if (p1->srg_obsspd_max_offset != p2->srg_obsspd_max_offset)
			return 1;
		if (PLATFORM_MEMCMP(p1->srg_bss_color_bitmap, p2->srg_bss_color_bitmap, 8) != 0)
			return 1;
		if (PLATFORM_MEMCMP(p1->srg_partial_bssid_bitmap, p2->srg_partial_bssid_bitmap, 8) != 0)
			return 1;
		return 0;
	}

	case TLV_TYPE_SPATIAL_REUSE_REPORT: {
		struct SpatialReuseReportTLV * p1, *p2;

		p1 = (struct SpatialReuseReportTLV *)memory_structure_1;
		p2 = (struct SpatialReuseReportTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type)
			return 1;
		
		if (PLATFORM_MEMCMP(p1->ruid, p2->ruid, 6) != 0)
			return 1;
		if (p1->bss_color != p2->bss_color)
			return 1;
		if (p1->valid_field != p2->valid_field)
			return 1;
		if (p1->non_srg_obsspd_max_offset != p2->non_srg_obsspd_max_offset)
			return 1;
		if (p1->srg_obsspd_min_offset != p2->srg_obsspd_min_offset)
			return 1;
		if (p1->srg_obsspd_max_offset != p2->srg_obsspd_max_offset)
			return 1;
		if (PLATFORM_MEMCMP(p1->srg_bss_color_bitmap, p2->srg_bss_color_bitmap, 8) != 0)
			return 1;
		if (PLATFORM_MEMCMP(p1->srg_partial_bssid_bitmap, p2->srg_partial_bssid_bitmap, 8) != 0)
			return 1;
		if (PLATFORM_MEMCMP(p1->neighbor_bss_color_inuse_bitmap, p2->neighbor_bss_color_inuse_bitmap, 8) != 0)
			return 1;
		return 0;
	}

	case TLV_TYPE_SPATIAL_REUSE_CONFIG_RESPONSE: {
		struct SpatialReuseConfigResponseTLV * p1, *p2;

		p1 = (struct SpatialReuseConfigResponseTLV *)memory_structure_1;
		p2 = (struct SpatialReuseConfigResponseTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type)
			return 1;
		
		if (PLATFORM_MEMCMP(p1->ruid, p2->ruid, 6) != 0)
			return 1;
		if (p1->response_code != p2->response_code)
			return 1;
		return 0;
	}

	case TLV_TYPE_QOS_MANAGEMENT_POLICY: {
		struct QoSManagementPolicyTLV * p1, *p2;

		p1 = (struct QoSManagementPolicyTLV *)memory_structure_1;
		p2 = (struct QoSManagementPolicyTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type)
			return 1;
		
		if (p1->mscsd_disallowed_sta_nr != p2->mscsd_disallowed_sta_nr){
			return 1;
		} else {
			int k;
			for (k = 0; k < p1->mscsd_disallowed_sta_nr; k++) {
				if (0 != PLATFORM_MEMCMP(p1->mscsd_disallowed_sta_mac_address[k], p2->mscsd_disallowed_sta_mac_address[k], 6))
					return 1;
			}
		}
		
		if (p1->scsd_disallowed_sta_nr != p2->scsd_disallowed_sta_nr){
			return 1;
		} else {
			int m;
			for (m = 0; m < p1->scsd_disallowed_sta_nr; m++) {
				if (0 != PLATFORM_MEMCMP(p1->scsd_disallowed_sta_mac_address[m], p2->scsd_disallowed_sta_mac_address[m], 6))
					return 1;
			}
		}
		
		return 0;
	}

	case TLV_TYPE_QOS_MANAGEMENT_DESCRIPTOR: {
		struct QoSManagementDescriptorTLV *p1, *p2;

		p1 = (struct QoSManagementDescriptorTLV *)memory_structure_1;
		p2 = (struct QoSManagementDescriptorTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type)
			return 1;

		if (p1->qmid != p2->qmid)
			return 1;
		if (PLATFORM_MEMCMP(p1->bssid, p2->bssid, 6) != 0)
			return 1;
		if (PLATFORM_MEMCMP(p1->client_mac, p2->client_mac, 6) != 0)
			return 1;
		if (p1->descriptor_element_len_1 != p2->descriptor_element_len_1)
			return 1;
		if (PLATFORM_MEMCMP(p1->descriptor_element_1, p2->descriptor_element_1, p1->descriptor_element_len_1) != 0)
			return 1;
		if (p1->descriptor_element_len_2 != p2->descriptor_element_len_2)
			return 1;
		if (PLATFORM_MEMCMP(p1->descriptor_element_2, p2->descriptor_element_2, p1->descriptor_element_len_2) != 0)
			return 1;
		return 0;
	}

	case TLV_TYPE_CONTROLLER_CAPABILITY: {
		struct ControllerCapabilityTLV * p1, *p2;

		p1 = (struct ControllerCapabilityTLV *)memory_structure_1;
		p2 = (struct ControllerCapabilityTLV *)memory_structure_2;

		if (p1->tlv_type != p2->tlv_type)
			return 1;
		
		if (p1->KiBMiB_counter != p2->KiBMiB_counter)
			return 1;
		return 0;
	}
	
	default: {
		// Unknown structure type
		//
		return 1;
	}
	}

	// This code cannot be reached
	//
	return 1;
}

void visit_1905_TLV_structure(INT8U *memory_structure, void (*callback)(void (*write_function)(const char *fmt, ...), const char *prefix, INT8U size, const char *name, const char *fmt, void *p), void (*write_function)(const char *fmt, ...), const char *prefix)
{
// Buffer size to store a prefix string that will be used to show each
// element of a structure on screen
//
#define MAX_PREFIX 100

	if (NULL == memory_structure) {
		return;
	}

	// The first byte of any of the valid structures is always the "tlv_type"
	// field.
	//
	switch (*memory_structure) {
	case TLV_TYPE_END_OF_MESSAGE: {
		// There is nothing to visit. This TLV is always empty
		//
		return;
	}

	case TLV_TYPE_VENDOR_SPECIFIC: {
		struct vendorSpecificTLV *p;

		p = (struct vendorSpecificTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->vendorOUI), "vendorOUI", "0x%02x", p->vendorOUI);
		callback(write_function, prefix, sizeof(p->m_nr), "m_nr", "%d", &p->m_nr);
		callback(write_function, prefix, p->m_nr, "m", "0x%02x", p->m);

		return;
	}

	case TLV_TYPE_AL_MAC_ADDRESS_TYPE: {
		struct alMacAddressTypeTLV *p;

		p = (struct alMacAddressTypeTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->al_mac_address), "al_mac_address", "0x%02x", p->al_mac_address);

		return;
	}

	case TLV_TYPE_MAC_ADDRESS_TYPE: {
		struct macAddressTypeTLV *p;

		p = (struct macAddressTypeTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->mac_address), "mac_address", "0x%02x", p->mac_address);

		return;
	}

	case TLV_TYPE_DEVICE_INFORMATION_TYPE: {
		struct deviceInformationTypeTLV *p;
		INT8U                            i;

		p = (struct deviceInformationTypeTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->al_mac_address), "al_mac_address", "0x%02x", p->al_mac_address);
		callback(write_function, prefix, sizeof(p->local_interfaces_nr), "local_interfaces_nr", "%d", &p->local_interfaces_nr);
		for (i = 0; i < p->local_interfaces_nr; i++) {
			char new_prefix[MAX_PREFIX];

			PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "%slocal_interfaces[%d]->", prefix, i);
			new_prefix[MAX_PREFIX - 1] = 0x0;

			callback(write_function, new_prefix, sizeof(p->local_interfaces[i].mac_address), "mac_address", "0x%02x", p->local_interfaces[i].mac_address);
			callback(write_function, new_prefix, sizeof(p->local_interfaces[i].media_type), "media_type", "0x%04x", &p->local_interfaces[i].media_type);
			callback(write_function, new_prefix, sizeof(p->local_interfaces[i].media_specific_data_size), "media_specific_data_size", "%d", &p->local_interfaces[i].media_specific_data_size);

			if (IS_IEEE_802_11_MEDIA(p->local_interfaces[i].media_type)) {
				callback(write_function, new_prefix, sizeof(p->local_interfaces[i].media_specific_data.ieee80211.network_membership), "network_membership", "0x%02x", p->local_interfaces[i].media_specific_data.ieee80211.network_membership);
				callback(write_function, new_prefix, sizeof(p->local_interfaces[i].media_specific_data.ieee80211.role), "role", "%d", &p->local_interfaces[i].media_specific_data.ieee80211.role);
				callback(write_function, new_prefix, sizeof(p->local_interfaces[i].media_specific_data.ieee80211.ap_channel_band), "ap_channel_band", "%d", &p->local_interfaces[i].media_specific_data.ieee80211.ap_channel_band);
				callback(write_function, new_prefix, sizeof(p->local_interfaces[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_1), "ap_channel_center_frequency_index_1", "%d", &p->local_interfaces[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_1);
				callback(write_function, new_prefix, sizeof(p->local_interfaces[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_2), "ap_channel_center_frequency_index_2", "%d", &p->local_interfaces[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_2);
			} else if (IS_IEEE_1901_MEDIA(p->local_interfaces[i].media_type)) {
				callback(write_function, new_prefix, sizeof(p->local_interfaces[i].media_specific_data.ieee1901.network_identifier), "network_identifier", "0x%02x", p->local_interfaces[i].media_specific_data.ieee1901.network_identifier);
			}
		}

		return;
	}

	case TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES: {
		struct deviceBridgingCapabilityTLV *p;
		INT8U                               i, j;

		p = (struct deviceBridgingCapabilityTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->bridging_tuples_nr), "bridging_tuples_nr", "%d", &p->bridging_tuples_nr);
		for (i = 0; i < p->bridging_tuples_nr; i++) {
			char new_prefix[MAX_PREFIX];

			PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "%sbridging_tuples[%d]->", prefix, i);
			new_prefix[MAX_PREFIX - 1] = 0x0;

			callback(write_function, new_prefix, sizeof(p->bridging_tuples[i].bridging_tuple_macs_nr), "bridging_tuple_macs_nr", "%d", &p->bridging_tuples[i].bridging_tuple_macs_nr);

			for (j = 0; j < p->bridging_tuples[i].bridging_tuple_macs_nr; j++) {
				PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "%sbridging_tuples[%d]->bridging_tuple_macs[%d]->", prefix, i, j);
				new_prefix[MAX_PREFIX - 1] = 0x0;

				callback(write_function, new_prefix, sizeof(p->bridging_tuples[i].bridging_tuple_macs[j].mac_address), "mac_address", "0x%02x", p->bridging_tuples[i].bridging_tuple_macs[j].mac_address);
			}
		}

		return;
	}

	case TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST: {
		struct non1905NeighborDeviceListTLV *p;
		INT8U                                i;

		p = (struct non1905NeighborDeviceListTLV *)memory_structure;

		if (p->non_1905_neighbors_nr > 0 && NULL == p->non_1905_neighbors) {
			// Malformed structure
			return;
		}

		callback(write_function, prefix, sizeof(p->local_mac_address), "local_mac_address", "0x%02x", p->local_mac_address);
		callback(write_function, prefix, sizeof(p->non_1905_neighbors_nr), "non_1905_neighbors_nr", "%d", &p->non_1905_neighbors_nr);
		for (i = 0; i < p->non_1905_neighbors_nr; i++) {
			char new_prefix[MAX_PREFIX];

			PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "%snon_1905_neighbors[%d]->", prefix, i);
			new_prefix[MAX_PREFIX - 1] = 0x0;

			callback(write_function, new_prefix, sizeof(p->non_1905_neighbors[i].mac_address), "mac_address", "0x%02x", p->non_1905_neighbors[i].mac_address);
		}

		return;
	}

	case TLV_TYPE_NEIGHBOR_DEVICE_LIST: {
		struct neighborDeviceListTLV *p;
		INT8U                         i;

		p = (struct neighborDeviceListTLV *)memory_structure;

		if (p->neighbors_nr > 0 && NULL == p->neighbors) {
			// Malformed structure
			return;
		}

		callback(write_function, prefix, sizeof(p->local_mac_address), "local_mac_address", "0x%02x", p->local_mac_address);
		callback(write_function, prefix, sizeof(p->neighbors_nr), "neighbors_nr", "%d", &p->neighbors_nr);
		for (i = 0; i < p->neighbors_nr; i++) {
			char new_prefix[MAX_PREFIX];

			PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "%sneighbors[%d]->", prefix, i);
			new_prefix[MAX_PREFIX - 1] = 0x0;

			callback(write_function, new_prefix, sizeof(p->neighbors[i].mac_address), "mac_address", "0x%02x", p->neighbors[i].mac_address);
			callback(write_function, new_prefix, sizeof(p->neighbors[i].bridge_flag), "bridge_flag", "%d", &p->neighbors[i].bridge_flag);
		}

		return;
	}

	case TLV_TYPE_LINK_METRIC_QUERY: {
		struct linkMetricQueryTLV *p;

		p = (struct linkMetricQueryTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->destination), "destination", "%d", &p->destination);
		callback(write_function, prefix, sizeof(p->specific_neighbor), "specific_neighbor", "0x%02x", p->specific_neighbor);
		callback(write_function, prefix, sizeof(p->link_metrics_type), "link_metrics_type", "%d", &p->link_metrics_type);

		return;
	}

	case TLV_TYPE_TRANSMITTER_LINK_METRIC: {
		struct transmitterLinkMetricTLV *p;
		INT8U                            i;

		p = (struct transmitterLinkMetricTLV *)memory_structure;

		if (NULL == p->transmitter_link_metrics) {
			// Malformed structure
			return;
		}

		callback(write_function, prefix, sizeof(p->local_al_address), "local_al_address", "0x%02x", p->local_al_address);
		callback(write_function, prefix, sizeof(p->neighbor_al_address), "neighbor_al_address", "0x%02x", p->neighbor_al_address);
		callback(write_function, prefix, sizeof(p->transmitter_link_metrics_nr), "transmitter_link_metrics_nr", "%d", &p->transmitter_link_metrics_nr);
		for (i = 0; i < p->transmitter_link_metrics_nr; i++) {
			char new_prefix[MAX_PREFIX];

			PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "%stransmitter_link_metrics[%d]->", prefix, i);
			new_prefix[MAX_PREFIX - 1] = 0x0;

			callback(write_function, new_prefix, sizeof(p->transmitter_link_metrics[i].local_interface_address), "local_interface_address", "0x%02x", p->transmitter_link_metrics[i].local_interface_address);
			callback(write_function, new_prefix, sizeof(p->transmitter_link_metrics[i].neighbor_interface_address), "neighbor_interface_address", "0x%02x", p->transmitter_link_metrics[i].neighbor_interface_address);
			callback(write_function, new_prefix, sizeof(p->transmitter_link_metrics[i].intf_type), "intf_type", "0x%04x", &p->transmitter_link_metrics[i].intf_type);
			callback(write_function, new_prefix, sizeof(p->transmitter_link_metrics[i].bridge_flag), "bridge_flag", "%d", &p->transmitter_link_metrics[i].bridge_flag);
			callback(write_function, new_prefix, sizeof(p->transmitter_link_metrics[i].packet_errors), "packet_errors", "%d", &p->transmitter_link_metrics[i].packet_errors);
			callback(write_function, new_prefix, sizeof(p->transmitter_link_metrics[i].transmitted_packets), "transmitted_packets", "%d", &p->transmitter_link_metrics[i].transmitted_packets);
			callback(write_function, new_prefix, sizeof(p->transmitter_link_metrics[i].mac_throughput_capacity), "mac_throughput_capacity", "%d", &p->transmitter_link_metrics[i].mac_throughput_capacity);
			callback(write_function, new_prefix, sizeof(p->transmitter_link_metrics[i].link_availability), "link_availability", "%d", &p->transmitter_link_metrics[i].link_availability);
			callback(write_function, new_prefix, sizeof(p->transmitter_link_metrics[i].phy_rate), "phy_rate", "%d", &p->transmitter_link_metrics[i].phy_rate);
		}

		return;
	}

	case TLV_TYPE_RECEIVER_LINK_METRIC: {
		struct receiverLinkMetricTLV *p;
		INT8U                         i;

		p = (struct receiverLinkMetricTLV *)memory_structure;

		if (NULL == p->receiver_link_metrics) {
			// Malformed structure
			return;
		}

		callback(write_function, prefix, sizeof(p->local_al_address), "local_al_address", "0x%02x", p->local_al_address);
		callback(write_function, prefix, sizeof(p->neighbor_al_address), "neighbor_al_address", "0x%02x", p->neighbor_al_address);
		callback(write_function, prefix, sizeof(p->receiver_link_metrics_nr), "receiver_link_metrics_nr", "%d", &p->receiver_link_metrics_nr);
		for (i = 0; i < p->receiver_link_metrics_nr; i++) {
			char new_prefix[MAX_PREFIX];

			PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "%sreceiver_link_metrics[%d]->", prefix, i);
			new_prefix[MAX_PREFIX - 1] = 0x0;

			callback(write_function, new_prefix, sizeof(p->receiver_link_metrics[i].local_interface_address), "local_interface_address", "0x%02x", p->receiver_link_metrics[i].local_interface_address);
			callback(write_function, new_prefix, sizeof(p->receiver_link_metrics[i].neighbor_interface_address), "neighbor_interface_address", "0x%02x", p->receiver_link_metrics[i].neighbor_interface_address);
			callback(write_function, new_prefix, sizeof(p->receiver_link_metrics[i].intf_type), "intf_type", "0x%04x", &p->receiver_link_metrics[i].intf_type);
			callback(write_function, new_prefix, sizeof(p->receiver_link_metrics[i].packet_errors), "packet_errors", "%d", &p->receiver_link_metrics[i].packet_errors);
			callback(write_function, new_prefix, sizeof(p->receiver_link_metrics[i].packets_received), "packets_received", "%d", &p->receiver_link_metrics[i].packets_received);
			callback(write_function, new_prefix, sizeof(p->receiver_link_metrics[i].rssi), "rssi", "%d", &p->receiver_link_metrics[i].rssi);
		}

		return;
	}

	case TLV_TYPE_LINK_METRIC_RESULT_CODE: {
		struct linkMetricResultCodeTLV *p;

		p = (struct linkMetricResultCodeTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->result_code), "result_code", "%d", &p->result_code);

		return;
	}

	case TLV_TYPE_SEARCHED_ROLE: {
		struct searchedRoleTLV *p;

		p = (struct searchedRoleTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->role), "role", "%d", &p->role);

		return;
	}

	case TLV_TYPE_AUTOCONFIG_FREQ_BAND: {
		struct autoconfigFreqBandTLV *p;

		p = (struct autoconfigFreqBandTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->freq_band), "freq_band", "%d", &p->freq_band);

		return;
	}

	case TLV_TYPE_SUPPORTED_ROLE: {
		struct supportedRoleTLV *p;

		p = (struct supportedRoleTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->role), "role", "%d", &p->role);

		return;
	}

	case TLV_TYPE_SUPPORTED_FREQ_BAND: {
		struct supportedFreqBandTLV *p;

		p = (struct supportedFreqBandTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->freq_band), "freq_band", "%d", &p->freq_band);

		return;
	}

	case TLV_TYPE_WSC: {
		struct wscTLV *p;

		p = (struct wscTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->wsc_frame_size), "wsc_frame_size", "%d", &p->wsc_frame_size);
		callback(write_function, prefix, p->wsc_frame_size, "wsc_frame", "0x%02x", p->wsc_frame);

		return;
	}

	case TLV_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION: {
		struct pushButtonEventNotificationTLV *p;
		INT8U                                  i;

		p = (struct pushButtonEventNotificationTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->media_types_nr), "media_types_nr", "0x%02x", &p->media_types_nr);
		for (i = 0; i < p->media_types_nr; i++) {
			char new_prefix[MAX_PREFIX];

			PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "%smedia_types[%d]->", prefix, i);
			new_prefix[MAX_PREFIX - 1] = 0x0;

			callback(write_function, new_prefix, sizeof(p->media_types[i].media_type), "media_type", "0x%04x", &p->media_types[i].media_type);
			callback(write_function, new_prefix, sizeof(p->media_types[i].media_specific_data_size), "media_specific_data_size", "%d", &p->media_types[i].media_specific_data_size);

			if (IS_IEEE_802_11_MEDIA(p->media_types[i].media_type)) {
				callback(write_function, new_prefix, sizeof(p->media_types[i].media_specific_data.ieee80211.network_membership), "network_membership", "0x%02x", p->media_types[i].media_specific_data.ieee80211.network_membership);
				callback(write_function, new_prefix, sizeof(p->media_types[i].media_specific_data.ieee80211.role), "role", "%d", &p->media_types[i].media_specific_data.ieee80211.role);
				callback(write_function, new_prefix, sizeof(p->media_types[i].media_specific_data.ieee80211.ap_channel_band), "ap_channel_band", "%d", &p->media_types[i].media_specific_data.ieee80211.ap_channel_band);
				callback(write_function, new_prefix, sizeof(p->media_types[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_1), "ap_channel_center_frequency_index_1", "%d", &p->media_types[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_1);
				callback(write_function, new_prefix, sizeof(p->media_types[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_2), "ap_channel_center_frequency_index_2", "%d", &p->media_types[i].media_specific_data.ieee80211.ap_channel_center_frequency_index_2);
			} else if (IS_IEEE_1901_MEDIA(p->media_types[i].media_type)) {
				callback(write_function, new_prefix, sizeof(p->media_types[i].media_specific_data.ieee1901.network_identifier), "network_identifier", "0x%02x", p->media_types[i].media_specific_data.ieee1901.network_identifier);
			}
		}

		return;
	}

	case TLV_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION: {
		struct pushButtonJoinNotificationTLV *p;

		p = (struct pushButtonJoinNotificationTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->al_mac_address), "al_mac_address", "0x%02x", p->al_mac_address);
		callback(write_function, prefix, sizeof(p->message_identifier), "message_identifier", "%d", &p->message_identifier);
		callback(write_function, prefix, sizeof(p->mac_address), "mac_address", "0x%02x", p->mac_address);
		callback(write_function, prefix, sizeof(p->new_mac_address), "new_mac_address", "0x%02x", p->new_mac_address);
		return;
	}

	case TLV_TYPE_SUPPORTED_SERVICE: {
		struct SupportedServiceTLV *p;

		p = (struct SupportedServiceTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->supported_service_nr), "supported service number", "%d", &p->supported_service_nr);
		INT8U i;
		for (i = 0; i < p->supported_service_nr; i++) {
			callback(write_function, prefix, sizeof(p->supported_service[i]), "supported service", "%d", &p->supported_service[i]);
		}
		return;
	}
	case TLV_TYPE_SEARCHED_SERVICE: {
		struct SearchedServiceTLV *p;

		p = (struct SearchedServiceTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->searched_service_nr), "searched service number", "%d", &p->searched_service_nr);
		INT8U i;
		for (i = 0; i < p->searched_service_nr; i++) {
			callback(write_function, prefix, sizeof(p->searched_service[i]), "searched service", "%d", &p->searched_service[i]);
		}
		return;
	}
	case TLV_TYPE_AP_OPERATIONAL_BSS: {
		struct ApOperationalBSSTLV *p;

		p = (struct ApOperationalBSSTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->radios_nr), "radio number", "%d", &p->radios_nr);

		INT8U k1, m1;
		for (k1 = 0; k1 < p->radios_nr; k1++) {
			callback(write_function, prefix, sizeof(p->radios[k1].radio_unique_id), "radio unique id", "%02x", p->radios[k1].radio_unique_id);
			callback(write_function, prefix, sizeof(p->radios[k1].BSSs_nr), "BSS number", "%d", &p->radios[k1].BSSs_nr);

			for (m1 = 0; m1 < p->radios[k1].BSSs_nr; m1++) {
				callback(write_function, prefix, sizeof(p->radios[k1].BSSs[m1].mac_addr), "MAC addr(BSSID)", "%02x", p->radios[k1].BSSs[m1].mac_addr);
				callback(write_function, prefix, sizeof(p->radios[k1].BSSs[m1].ssid_len), "SSID length", "%d", &p->radios[k1].BSSs[m1].ssid_len);
				callback(write_function, prefix, p->radios[k1].BSSs[m1].ssid_len, "SSID", "%s", p->radios[k1].BSSs[m1].ssid);
			}
		}
		return;
	}
	case TLV_TYPE_ASSOCIATED_CLIENTS: {
		struct AssociatedClientsTLV *p;

		p = (struct AssociatedClientsTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->BSSs_nr), "BSS number", "%d", &p->BSSs_nr);

		INT8U k1, m1;
		for (k1 = 0; k1 < p->BSSs_nr; k1++) {

			callback(write_function, prefix, sizeof(p->BSSs[k1].bssid), "BSSID", "%02x", p->BSSs[k1].bssid);
			callback(write_function, prefix, sizeof(p->BSSs[k1].clients_nr), "BSS client number", "%d", &p->BSSs[k1].clients_nr);

			for (m1 = 0; m1 < p->BSSs[k1].clients_nr; m1++) {
				callback(write_function, prefix, sizeof(p->BSSs[k1].clients[m1].mac_addr), "MAC address", "%02x", p->BSSs[k1].clients[m1].mac_addr);
				callback(write_function, prefix, sizeof(p->BSSs[k1].clients[m1].left_time), "Left time", "%d", &p->BSSs[k1].clients[m1].left_time);
			}
		}
		return;
	}
	case TLV_TYPE_CLIENT_ASSOCIATION_EVENT: {
		struct ClientAssociationEventTLV *p;

		p = (struct ClientAssociationEventTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->mac_address), "Client MAC", "%02x", p->mac_address);

		callback(write_function, prefix, sizeof(p->bssid), "BSSID", "%02x", p->bssid);

		if (CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_JOIN == p->event)
			callback(write_function, prefix, sizeof(p->event), "Event JOIN", "%02x", &p->event);
		else
			callback(write_function, prefix, sizeof(p->event), "Event LEAVE", "%02x", &p->event);

		return;
	}

	case TLV_TYPE_AP_RADIO_IDENTIFIER: {
		struct ApRadioIdentifierTLV *p;

		p = (struct ApRadioIdentifierTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->radio_unique_id), "radio unique id:", "%02x", p->radio_unique_id);

		return;
	}

	case TLV_TYPE_AP_CAPABILITY: {

		struct APCapabilityTLV *p;

		p = (struct APCapabilityTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->ap_capability), "ap_capability:", "%02x", &p->ap_capability);

		return;
	}

	case TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES: {
		struct APRadioBasicCapabilitiesTLV *p;

		p = (struct APRadioBasicCapabilitiesTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->radio_unique_id), "Radio unique identifier:",
		         "%02x", &p->radio_unique_id);

		callback(write_function, prefix, sizeof(p->max_BSSs_nr), "Max BSS supported:", "%d", &p->max_BSSs_nr);

		callback(write_function, prefix, sizeof(p->operating_classes_nr), "Operating classes number:", "%d", &p->operating_classes_nr);

		INT8U k1, m1;
		for (k1 = 0; k1 < p->operating_classes_nr; k1++) {

			callback(write_function, prefix, sizeof(p->operating_classes[k1].operating_class), "Operating class:", "%02x", &p->operating_classes[k1].operating_class);
			callback(write_function, prefix, sizeof(p->operating_classes[k1].max_transmit_power), "Max transmit power:", "%02x", &p->operating_classes[k1].max_transmit_power);

			for (m1 = 0; m1 < p->operating_classes[k1].non_operable_channels_nr; m1++) {
				callback(write_function, prefix, sizeof(p->operating_classes[k1].non_operable_channels[m1]), "Non operating channel:", "%d", &p->operating_classes[k1].non_operable_channels[m1]);
			}
		}
		return;
	}

	case TLV_TYPE_AP_HT_CAPABILITIES: {

		struct APHTCapabilitiesTLV *p;

		p = (struct APHTCapabilitiesTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->radio_unique_id), "Radio unique identifier:",
		         "%02x", &p->radio_unique_id);

		callback(write_function, prefix, sizeof(p->ht_capability), "ap_ht_capability:", "%02x", &p->ht_capability);

		return;
	}

	case TLV_TYPE_AP_VHT_CAPABILITIES: {

		struct APVHTCapabilitiesTLV *p;

		p = (struct APVHTCapabilitiesTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->radio_unique_id), "Radio unique identifier:",
		         "%02x", &p->radio_unique_id);

		callback(write_function, prefix, sizeof(p->vht_tx_msc), "ap_vht_tx_msc:", "%02x", &p->vht_tx_msc);
		callback(write_function, prefix, sizeof(p->vht_rx_msc), "ap_vht_rx_msc:", "%02x", &p->vht_rx_msc);
		callback(write_function, prefix, sizeof(p->vht_capability_1), "ap_vht_capability 1:", "%02x", &p->vht_capability_1);
		callback(write_function, prefix, sizeof(p->vht_capability_2), "ap_vht_capability 2:", "%02x", &p->vht_capability_2);

		return;
	}

	case TLV_TYPE_AP_HE_CAPABILITIES: {

		struct APHECapabilitiesTLV *p;

		INT8U m1;

		p = (struct APHECapabilitiesTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->radio_unique_id), "Radio unique identifier:",
		         "%02x", &p->radio_unique_id);
		callback(write_function, prefix, sizeof(p->he_msc_len), "he_msc_len:", "%02x", &p->he_msc_len);
		for (m1 = 0; m1 < p->he_msc_len; m1++) {
			callback(write_function, prefix, sizeof(p->he_msc[m1]), "he_msc:", "%02x", &p->he_msc[m1]);
		}
		callback(write_function, prefix, sizeof(p->he_capability_1), "ap_vht_capability 1:", "%02x", &p->he_capability_1);
		callback(write_function, prefix, sizeof(p->he_capability_2), "ap_vht_capability 2:", "%02x", &p->he_capability_2);

		return;
	}

	case TLV_TYPE_ERROR_CODE: {
		struct ErrorCodeTLV *p;

		p = (struct ErrorCodeTLV *)memory_structure;
		callback(write_function, prefix, sizeof(p->reason_code), "Reason code:", "%d", &p->reason_code);
		if (p->reason_code == 0x01 || p->reason_code == 0x02) {
			callback(write_function, prefix, sizeof(p->sta_mac_address), "Related STA MAC:", "%02x", p->sta_mac_address);
		}
		return;
	}

	case TLV_TYPE_STEERING_POLICY: {
		struct SteeringPolicyTLV *p;
		p = (struct SteeringPolicyTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->local_disallowed_sta_nr), "local_disallowed_sta_nr", "%d", &p->local_disallowed_sta_nr);
		int h = 0;
		for (; h < p->local_disallowed_sta_nr; h++) {
			callback(write_function, prefix, sizeof(*(p->local_disallowed_sta_mac)), "local_disallowed_sta_mac", "%02x", p->local_disallowed_sta_mac[h]);
		}

		callback(write_function, prefix, sizeof(p->btm_disallowed_sta_nr), "btm_disallowed_sta_nr", "%d", &p->btm_disallowed_sta_nr);
		int k = 0;
		for (; k < p->btm_disallowed_sta_nr; k++) {
			callback(write_function, prefix, sizeof(*(p->btm_disallowed_sta_mac)), "btm_disallowed_sta_mac", "%02x", p->btm_disallowed_sta_mac[k]);
		}

		callback(write_function, prefix, sizeof(p->radios_nr), "radios_nr", "%d", &p->radios_nr);
		int m = 0;
		for (; m < p->radios_nr; m++) {
			callback(write_function, prefix, sizeof(p->policies[m].radio_unique_id), "radio_unique_id", "%02x", p->policies[m].radio_unique_id);
			callback(write_function, prefix, sizeof(p->policies[m].policy), "policy", "%d", &p->policies[m].policy);
			callback(write_function, prefix, sizeof(p->policies[m].channel_util_threshold), "channel_util_threshold", "%d", &p->policies[m].channel_util_threshold);
			callback(write_function, prefix, sizeof(p->policies[m].rcpi_steering_threshold), "rssi_steering_threshold", "%d", &p->policies[m].rcpi_steering_threshold);
		}
		return;
	}

	case TLV_TYPE_METRIC_REPORT_POLICY: {
		struct MetricReportingPolicyTLV *p;
		p = (struct MetricReportingPolicyTLV *)memory_structure;
		callback(write_function, prefix, sizeof(p->report_interval), "report_interval", "%d", &p->report_interval);
		callback(write_function, prefix, sizeof(p->radios_nr), "radios_nr", "%d", &p->radios_nr);
		int m = 0;
		for (; m < p->radios_nr; m++) {
			callback(write_function, prefix, sizeof(p->policies[m].radio_unique_id), "radio_unique_id", "%02x", p->policies[m].radio_unique_id);
			callback(write_function, prefix, sizeof(p->policies[m].sta_rcpi_threshold), "sta_rssi_threshold", "%d", &p->policies[m].sta_rcpi_threshold);
			callback(write_function, prefix, sizeof(p->policies[m].sta_rcpi_hysteresis_margin_override), "sta_rssi_hysteresis_margin_override", "%d", &p->policies[m].sta_rcpi_hysteresis_margin_override);
			callback(write_function, prefix, sizeof(p->policies[m].ap_channel_utilization_threshold), "ap_channel_utilization_threshold", "%d", &p->policies[m].ap_channel_utilization_threshold);
			callback(write_function, prefix, sizeof(p->policies[m].policy), "policy", "%d", &p->policies[m].policy);
		}
		return;
	}

	case TLV_TYPE_CHANNEL_PREFERENCE: {
		struct ChannelPreferenceTLV *p;
		p = (struct ChannelPreferenceTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->radio_unique_id), "radio_unique_id", "%02x", p->radio_unique_id);
		callback(write_function, prefix, sizeof(p->op_class_nr), "op_class_nr", "%d", &p->op_class_nr);
		int m = 0;
		for (; m < p->op_class_nr; m++) {
			callback(write_function, prefix, sizeof(p->channel_preferences[m].op_class), "op_class", "%d", &p->channel_preferences[m].op_class);
			callback(write_function, prefix, sizeof(p->channel_preferences[m].channel_nr), "channel_nr", "%d", &p->channel_preferences[m].channel_nr);
			callback(write_function, prefix, p->channel_preferences[m].channel_nr * sizeof(INT8U), "channel_list", "%02x", p->channel_preferences[m].channel_list);
			callback(write_function, prefix, sizeof(p->channel_preferences[m].preference_reason_code), "preference_reason_code", "%d", &p->channel_preferences[m].preference_reason_code);
		}
		return;
	}

	case TLV_TYPE_RADIO_OPERATION_RESTRICTION: {
		struct RadioOperationRestrictionTLV *p;
		p = (struct RadioOperationRestrictionTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->radio_unique_id), "radio_unique_id", "%02x", p->radio_unique_id);
		callback(write_function, prefix, sizeof(p->op_class_nr), "op_class_nr", "%d", &p->op_class_nr);
		int m = 0;
		for (; m < p->op_class_nr; m++) {
			callback(write_function, prefix, sizeof(p->radio_operation_restriction[m].op_class), "op_class", "%d", &p->radio_operation_restriction[m].op_class);
			callback(write_function, prefix, sizeof(p->radio_operation_restriction[m].channel_nr), "channel_nr", "%d", &p->radio_operation_restriction[m].channel_nr);
			int k;
			for (k = 0; k < p->radio_operation_restriction[m].channel_nr; k++) {
				callback(write_function, prefix, sizeof(p->radio_operation_restriction[m].channel_operation_restriction[k].channel), "channel", "%d", &p->radio_operation_restriction[m].channel_operation_restriction[k].channel);
				callback(write_function, prefix, sizeof(p->radio_operation_restriction[m].channel_operation_restriction[k].min_freq_separation), "min_freq_separation", "%d", &p->radio_operation_restriction[m].channel_operation_restriction[k].min_freq_separation);
			}
		}
		return;
	}

	case TLV_TYPE_TRANSMIT_POWER_LIMIT: {
		struct TransmitPowerLimitTLV *p;
		p = (struct TransmitPowerLimitTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->radio_unique_id), "radio_unique_id", "%02x", p->radio_unique_id);
		callback(write_function, prefix, sizeof(p->transmit_power_limit), "transmit_power_limit", "%d", &p->transmit_power_limit);
		return;
	}

	case TLV_TYPE_CHANNEL_SELECTION_RESPONSE: {
		struct ChannelSelectionResponseTLV *p;
		p = (struct ChannelSelectionResponseTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->radio_unique_id), "radio_unique_id", "%02x", p->radio_unique_id);
		callback(write_function, prefix, sizeof(p->response), "response", "%d", &p->response);
		return;
	}

	case TLV_TYPE_OPERATING_CHANNEL_REPORT: {
		struct OperatingChannelReportTLV *p;
		p = (struct OperatingChannelReportTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->radio_unique_id), "radio_unique_id", "%02x", p->radio_unique_id);
		callback(write_function, prefix, sizeof(p->cur_op_class_nr), "cur_op_class_nr", "%d", &p->cur_op_class_nr);
		int m = 0;
		for (; m < p->cur_op_class_nr; m++) {
			callback(write_function, prefix, sizeof(p->operating_channels[m].op_class), "op_class", "%d", &p->operating_channels[m].op_class);
			callback(write_function, prefix, sizeof(p->operating_channels[m].cur_channel), "cur_channel", "%d", &p->operating_channels[m].cur_channel);
		}
		return;
	}

	case TLV_TYPE_STEERING_REQUEST: {
		struct SteeringRequestTLV *p;
		p = (struct SteeringRequestTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->bssid), "bssid", "%02x", p->bssid);
		callback(write_function, prefix, sizeof(p->mode), "mode", "%d", &p->mode);
		callback(write_function, prefix, sizeof(p->window), "window", "%d", &p->window);
		callback(write_function, prefix, sizeof(p->disassociation_timer), "disassociation_timer", "%d", &p->disassociation_timer);
		callback(write_function, prefix, sizeof(p->sta_nr), "sta_nr", "%d", &p->sta_nr);
		int m = 0;
		for (; m < p->sta_nr; m++) {
			callback(write_function, prefix, sizeof(p->sta_mac_address[m]), "sta_mac_address", "%02x", p->sta_mac_address[m]);
		}
		callback(write_function, prefix, sizeof(p->target_bss_nr), "target_bss_nr", "%d", &p->target_bss_nr);
		m = 0;
		for (; m < p->target_bss_nr; m++) {
			callback(write_function, prefix, sizeof(p->target_bss[m].bssid), "target_bssid", "%02x", p->target_bss[m].bssid);
			callback(write_function, prefix, sizeof(p->target_bss[m].opclass), "opclass", "%d", &p->target_bss[m].opclass);
			callback(write_function, prefix, sizeof(p->target_bss[m].channel), "channel", "%d", &p->target_bss[m].channel);
		}
		return;
	}

	case TLV_TYPE_STEERING_BTM_REPORT: {
		struct SteeringBTMReportTLV *p;
		p = (struct SteeringBTMReportTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->bssid), "bssid", "%02x", p->bssid);
		callback(write_function, prefix, sizeof(p->sta_mac_address), "sta_mac_address", "%02x", p->sta_mac_address);
		callback(write_function, prefix, sizeof(p->status_code), "status_code", "%d", &p->status_code);
		callback(write_function, prefix, sizeof(p->target_bssid), "target_bssid", "%02x", p->target_bssid);
		return;
	}

	case TLV_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST: {
		struct ClientAssociationControlRequestTLV *p;
		p = (struct ClientAssociationControlRequestTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->bssid), "bssid", "%02x", p->bssid);
		callback(write_function, prefix, sizeof(p->association_control), "association_control", "%d", &p->association_control);
		callback(write_function, prefix, sizeof(p->validity_period), "validity_period", "%d", &p->validity_period);
		callback(write_function, prefix, sizeof(p->sta_nr), "sta_nr", "%d", &p->sta_nr);
		int m = 0;
		for (; m < p->sta_nr; m++) {
			callback(write_function, prefix, sizeof(p->sta_mac_address[m]), "sta_mac_address", "%02x", p->sta_mac_address[m]);
		}
		return;
	}

	case TLV_TYPE_CLIENT_INFO: {

		struct ClientInfoTLV *p;

		p = (struct ClientInfoTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->bssid), "BSSID", "%02x", p->bssid);
		callback(write_function, prefix, sizeof(p->mac_address), "Client MAC", "%02x", p->mac_address);

		return;
	}

	case TLV_TYPE_CLIENT_CAPABLITY_REPORT: {

		struct ClientCapabilityReportTLV *p;

		p = (struct ClientCapabilityReportTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->result_code), "Result Code:", "%d", &p->result_code);

		return;
	}

	case TLV_TYPE_AP_METRIC_QUERY: {
		struct APMetricQueryTLV *p;
		int                      k;
		p = (struct APMetricQueryTLV *)memory_structure;
		callback(write_function, prefix, sizeof(p->bssid_nr), "BSSID Number:", "%d", &p->bssid_nr);

		for (k = 0; k < p->bssid_nr; k++) {
			callback(write_function, prefix, sizeof(p->bssid[k]), "BSSID:", "%02x", p->bssid[k]);
		}

		return;
	}

	case TLV_TYPE_AP_METRICS: {
		struct APMetricsTLV *p;
		p = (struct APMetricsTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->bssid), "BSSID:", "%02x", p->bssid);
		callback(write_function, prefix, sizeof(p->ch_util), "Ch_util:", "%d", &p->ch_util);
		callback(write_function, prefix, sizeof(p->sta_nr), "Station Nr:", "%d", &p->sta_nr);
		callback(write_function, prefix, sizeof(p->esp_ie), "ESP IE:", "%d", &p->esp_ie);
		callback(write_function, prefix, sizeof(p->esp_acbe), "ESP ACBE:", "%02x", p->esp_acbe);
		callback(write_function, prefix, sizeof(p->esp_acbk), "ESP ACBK:", "%02x", p->esp_acbk);
		callback(write_function, prefix, sizeof(p->esp_acvo), "ESP ACVO:", "%02x", p->esp_acvo);
		callback(write_function, prefix, sizeof(p->esp_acvi), "ESP ACVI:", "%02x", p->esp_acvi);
		return;
	}

	case TLV_TYPE_STA_MAC_ADDRESS_TYPE: {
		struct STAMacAddressTypeTLV *p;
		p = (struct STAMacAddressTypeTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->mac_address), "MAC address:", "%02x", p->mac_address);
		return;
	}

	case TLV_TYPE_ASSOCIATED_STA_LINK_METRICS: {
		struct AssociatedSTALinkMetricsTLV *p;
		INT8U                               k;
		p = (struct AssociatedSTALinkMetricsTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->mac_address), "MAC address:", "%02x", p->mac_address);

		callback(write_function, prefix, sizeof(p->bssid_nr), "BSSID number:", "%d", &p->bssid_nr);

		for (k = 0; k < p->bssid_nr; k++) {
			char new_prefix[MAX_PREFIX];
			PLATFORM_SNPRINTF(new_prefix, MAX_PREFIX - 1, "%ssta_link_metric[%d]->", prefix, k);
			new_prefix[MAX_PREFIX - 1] = 0x0;

			callback(write_function, new_prefix, sizeof(p->assoc_sta_link_metrics[k].bssid), "BSSID", "%02x", p->assoc_sta_link_metrics[k].bssid);
			callback(write_function, new_prefix, sizeof(p->assoc_sta_link_metrics[k].time_delta), "Time Delta", "%d", &p->assoc_sta_link_metrics[k].time_delta);
			callback(write_function, new_prefix, sizeof(p->assoc_sta_link_metrics[k].dataRate_downlink), "Downlink Data Rate", "%d", &p->assoc_sta_link_metrics[k].dataRate_downlink);
			callback(write_function, new_prefix, sizeof(p->assoc_sta_link_metrics[k].dataRate_uplink), "Uplink Data Rate", "%d", &p->assoc_sta_link_metrics[k].dataRate_uplink);
			callback(write_function, new_prefix, sizeof(p->assoc_sta_link_metrics[k].uplink_rcpi), "Uplink RSSI", "%d", &p->assoc_sta_link_metrics[k].uplink_rcpi);
		}

		return;
	}
	case TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS: {
		struct AssociatedSTATrafficStatsTLV *p;
		p = (struct AssociatedSTATrafficStatsTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->sta_mac_address), "STA MAC address:", "%02x", p->sta_mac_address);
		callback(write_function, prefix, sizeof(p->bytes_sent), "Bytes sent:", "%lu", &p->bytes_sent);
		callback(write_function, prefix, sizeof(p->bytes_received), "Bytes received:", "%lu", &p->bytes_received);
		callback(write_function, prefix, sizeof(p->packets_sent), "Packets sent:", "%lu", &p->packets_sent);
		callback(write_function, prefix, sizeof(p->packets_received), "Packets received:", "%lu", &p->packets_received);
		callback(write_function, prefix, sizeof(p->tx_packets_errors), "Tx packets error:", "%lu", &p->tx_packets_errors);
		callback(write_function, prefix, sizeof(p->rx_packets_errors), "Rx packets error:", "%lu", &p->rx_packets_errors);
		callback(write_function, prefix, sizeof(p->retransmission_count), "Retransmission count:", "%lu", &p->retransmission_count);

		return;
	}

	case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY: {
		struct UnassociatedSTALinkMetricsQueryTLV *p;
		INT8U                                      k, j;
		p = (struct UnassociatedSTALinkMetricsQueryTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->op_class), "Op Class:", "%d", &p->op_class);
		callback(write_function, prefix, sizeof(p->channel_list_nr), "Channel List number:", "%d", &p->channel_list_nr);

		for (k = 0; k < p->channel_list_nr; k++) {
			callback(write_function, prefix, sizeof(p->channel_list_entry[k].channel_nr), "Channel:", "%d", &p->channel_list_entry[k].channel_nr);
			callback(write_function, prefix, sizeof(p->channel_list_entry[k].sta_nr), "Sta nr:", "%d", &p->channel_list_entry[k].sta_nr);
			for (j = 0; j < p->channel_list_entry[k].sta_nr; j++) {
				callback(write_function, prefix, sizeof(p->channel_list_entry[k].sta_mac_address[j]), "Sta MAC:", "%02x", p->channel_list_entry[k].sta_mac_address[j]);
			}
		}

		return;
	}

	case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE: {
		struct UnassociatedSTALinkMetricsResponseTLV *p;
		INT8U                                         k;
		p = (struct UnassociatedSTALinkMetricsResponseTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->op_class), "Op Class:", "%d", &p->op_class);
		callback(write_function, prefix, sizeof(p->sta_nr), "Sta number:", "%d", &p->sta_nr);

		for (k = 0; k < p->sta_nr; k++) {
			callback(write_function, prefix, sizeof(p->unassoc_sta_link_metrics[k].sta_mac_address), "Sta MAC:", "%02x", p->unassoc_sta_link_metrics[k].sta_mac_address);
			callback(write_function, prefix, sizeof(p->unassoc_sta_link_metrics[k].channel_number), "Channel nr:", "%d", &p->unassoc_sta_link_metrics[k].channel_number);
			callback(write_function, prefix, sizeof(p->unassoc_sta_link_metrics[k].time_delta), "Time delta:", "%d", &p->unassoc_sta_link_metrics[k].time_delta);
			callback(write_function, prefix, sizeof(p->unassoc_sta_link_metrics[k].uplink_rcpi), "Uplink Rssi:", "%d", &p->unassoc_sta_link_metrics[k].uplink_rcpi);
		}

		return;
	}

	case TLV_TYPE_BEACON_METRICS_QUERY: {
		struct BeaconMetricsQueryTLV *p;
		INT8U                         k;
		p = (struct BeaconMetricsQueryTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->mac_address), "MAC:", "%02x", p->mac_address);
		callback(write_function, prefix, sizeof(p->op_class), "Op Class:", "%d", &p->op_class);
		callback(write_function, prefix, sizeof(p->channel_number), "Channel number:", "%d", &p->channel_number);
		callback(write_function, prefix, sizeof(p->bssid), "BSSID:", "%02x", p->bssid);
		callback(write_function, prefix, sizeof(p->report_detail), "Report detail:", "%d", &p->report_detail);
		callback(write_function, prefix, sizeof(p->ssid_len), "SSID Length:", "%d", &p->ssid_len);
		callback(write_function, prefix, p->ssid_len, "SSID:", "%02x", p->ssid);

		for (k = 0; k < p->ap_channel_report_nr; k++) {
			callback(write_function, prefix, sizeof(p->ap_channel_report[k].len), "Length:", "%d", &p->ap_channel_report[k].len);
			callback(write_function, prefix, sizeof(p->ap_channel_report[k].op_class), "Op Class:", "%d", &p->ap_channel_report[k].op_class);
			callback(write_function, prefix, (p->ap_channel_report[k].len - 1), "Channel List:", "%d", p->ap_channel_report[k].channel_list);
		}

		callback(write_function, prefix, sizeof(p->elementID_nr), "Element Nr:", "%d", &p->elementID_nr);

		for (k = 0; k < p->elementID_nr; k++) {
			callback(write_function, prefix, sizeof(p->element_list[k]), "Element List:", "%02x", &p->element_list[k]);
		}

		return;
	}

	case TLV_TYPE_BEACON_METRICS_RESPONSE: {
		struct BeaconMetricsResponseTLV *p;
		INT8U                            k;
		p = (struct BeaconMetricsResponseTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->mac_address), "MAC:", "%02x", p->mac_address);
		callback(write_function, prefix, sizeof(p->reserved_field), "Reserved:", "%02x", &p->reserved_field);
		callback(write_function, prefix, sizeof(p->measurement_report_nr), "Measurement Report nr:", "%d", &p->measurement_report_nr);

		for (k = 0; k < p->measurement_report_nr; k++) {
			callback(write_function, prefix, sizeof(p->measurement_reports[k].elementId), "Element ID:", "%d", &p->measurement_reports[k].elementId);
			callback(write_function, prefix, sizeof(p->measurement_reports[k].len), "Len:", "%d", &p->measurement_reports[k].len);
			callback(write_function, prefix, sizeof(p->measurement_reports[k].measurementToken), "Measurement Token:", "%d", &p->measurement_reports[k].measurementToken);
			callback(write_function, prefix, sizeof(p->measurement_reports[k].measurementReportMode), "Measurement Report:", "%d", &p->measurement_reports[k].measurementReportMode);
			callback(write_function, prefix, sizeof(p->measurement_reports[k].measurementType), "Measurement Type:", "%d", &p->measurement_reports[k].measurementType);
			//callback(write_function, prefix, sizeof(p->measurement_reports[k].info.), "", "%", );
			if (p->measurement_reports[k].len > BEACON_REPORT_FIXED_IE) {
				callback(write_function, prefix, sizeof(p->measurement_reports[k].subelements_len), "Subelement len:", "%d", &p->measurement_reports[k].subelements_len);
				if (p->measurement_reports[k].subelements_len > 0)
					callback(write_function, prefix, p->measurement_reports[k].subelements_len, "Subelements:", "%02x", p->measurement_reports[k].subelements);
				if (p->measurement_reports[k].subelements_remain_len > 0)
					callback(write_function, prefix, p->measurement_reports[k].subelements_remain_len, "Subelements_remain:", "%02x", p->measurement_reports[k].subelements_remain);
			}
		}

		return;
	}

	case TLV_TYPE_BACKHAUL_STEERING_REQUEST: {
		struct BackhaulSteeringRequestTLV *p;

		p = (struct BackhaulSteeringRequestTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->backhaul_mac), "Backhaul Mac:", "%02x", p->backhaul_mac);

		callback(write_function, prefix, sizeof(p->target_bssid), "Target BSSID:", "%02x", p->target_bssid);

		callback(write_function, prefix, sizeof(p->op_class), "OP Class:", "%d", &p->op_class);

		callback(write_function, prefix, sizeof(p->channel_number), "Channel number:", "%d", &p->channel_number);

		return;
	}

	case TLV_TYPE_BACKHAUL_STEERING_RESPONSE: {
		struct BackhaulSteeringResponseTLV *p;

		p = (struct BackhaulSteeringResponseTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->backhaul_mac), "Backhaul Mac:", "%02x", p->backhaul_mac);

		callback(write_function, prefix, sizeof(p->target_bssid), "Target BSSID:", "%02x", p->target_bssid);

		callback(write_function, prefix, sizeof(p->result_code), "Result code:", "%d", &p->result_code);

		return;
	}

	case TLV_TYPE_CHANNEL_SCAN_RESULT: {
		struct ChannelScanResultTLV *p;

		p = (struct ChannelScanResultTLV *)memory_structure;

		int k = 0;

		callback(write_function, prefix, sizeof(p->radio_unique_identifier), "Radio id:", "%02x", p->radio_unique_identifier);
		callback(write_function, prefix, sizeof(p->op_class), "Op class:", "%d", &p->op_class);
		callback(write_function, prefix, sizeof(p->channel), "Channel:", "%d", &p->channel);
		callback(write_function, prefix, sizeof(p->scan_status), "Scan Status:", "%d", &p->scan_status);

		//skip the remaining fields if the scan status is not zero
		if (p->scan_status) {
			return;
		}

		callback(write_function, prefix, sizeof(p->timestamp_length), "Timestamp Length:", "%d", &p->timestamp_length);
		callback(write_function, prefix, p->timestamp_length, "Timestamp:", "%02x", p->timestamp);
		callback(write_function, prefix, sizeof(p->channel_utilization), "Channel Utilization:", "%d", &p->channel_utilization);
		callback(write_function, prefix, sizeof(p->noise), "Noise:", "%d", &p->noise);
		callback(write_function, prefix, sizeof(p->neighbor_nr), "Neighbor Number:", "%d", &p->neighbor_nr);

		for (k = 0; k < p->neighbor_nr; k++) {
			callback(write_function, prefix, sizeof(p->neighbors[k].bssid), "Neighbor bssid:", "%02x", p->neighbors[k].bssid);
			callback(write_function, prefix, sizeof(p->neighbors[k].ssid_length), "SSID Length:", "%d", &p->neighbors[k].ssid_length);
			callback(write_function, prefix, p->neighbors[k].ssid_length, "SSID:", "%02x", p->neighbors[k].ssid);
			callback(write_function, prefix, sizeof(p->neighbors[k].signal_strength), "Signal Strength:", "%d", &p->neighbors[k].signal_strength);
			callback(write_function, prefix, sizeof(p->neighbors[k].channel_bandwidth_length), "Channel Bandwidth Length:", "%d", &p->neighbors[k].channel_bandwidth_length);
			callback(write_function, prefix, p->neighbors[k].channel_bandwidth_length, "Channel Bandwidth:", "%02x", p->neighbors[k].channel_bandwidth);
			callback(write_function, prefix, sizeof(p->neighbors[k].field_present_flags), "Field Present Flags:", "%02x", &p->neighbors[k].field_present_flags);
			if (CHANNELUTLIZATION_PRESENT_MASK & p->neighbors[k].field_present_flags)
				callback(write_function, prefix, sizeof(p->neighbors[k].channel_utilization), "Channel Utilization:", "%d", &p->neighbors[k].channel_utilization);
			if (STATIONCOUNT_PRESENT_MASK & p->neighbors[k].field_present_flags)
				callback(write_function, prefix, sizeof(p->neighbors[k].station_count), "Station Count:", "%d", &p->neighbors[k].station_count);
		}

		callback(write_function, prefix, sizeof(p->aggregate_scan_duration), "Aggregate Scan Duration:", "%d", &p->aggregate_scan_duration);
		callback(write_function, prefix, sizeof(p->scan_type_flags), "Scan Type Flags:", "%02x", &p->scan_type_flags);

		return;
	}

	case TLV_TYPE_1905_LAYER_SECURITY_CAPABILITY: {
		struct SecurityCapabilityTLV *p;

		p = (struct SecurityCapabilityTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->onboarding_protocol_supported), "Onboarding Protocol:", "%02x", &p->onboarding_protocol_supported);

		callback(write_function, prefix, sizeof(p->message_integrity_algorithm_supported), "Message integrity algo:", "%02x", &p->message_integrity_algorithm_supported);

		callback(write_function, prefix, sizeof(p->message_encryption_algorithm_supported), "Message encrypt algo:", "%02x", &p->message_encryption_algorithm_supported);

		return;
	}

	case TLV_TYPE_MIC: {
		struct MICTLV *p;

		p = (struct MICTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->gtk_key_id_ver), "GTK ID and Ver:", "%02x", &p->gtk_key_id_ver);

		callback(write_function, prefix, sizeof(p->integrity_transmission_counter), "ITC:", "%llu", &p->integrity_transmission_counter);

		callback(write_function, prefix, sizeof(p->src_1905_al_mac_addr), "Src 1905 AL MAC:", "%02x", p->src_1905_al_mac_addr);

		callback(write_function, prefix, sizeof(p->mic_length), "MIC Len:", "%d", &p->mic_length);

		callback(write_function, prefix, p->mic_length * sizeof(INT8U), "MIC:", "%02x", p->mic);

		return;
	}

	case TLV_TYPE_MULTIAP_PROFILE: {
		struct MultiAPProfileTLV *p;

		p = (struct MultiAPProfileTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->profile), "Profile:", "%02x", &p->profile);
		return;
	}

	case TLV_TYPE_PROFILE_2_CAPABILITY: {
		struct Profile2APCapabilityTLV *p;

		p = (struct Profile2APCapabilityTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->max_total_nr_service_prioritization_rules), "max_total_nr_service_prioritization_rules:", "%d", &p->max_total_nr_service_prioritization_rules);
		// callback(write_function, prefix, sizeof(p->max_nr_advance_service_prioritization_rules), "max_nr_advance_service_prioritization_rules:", "%d", &p->max_nr_advance_service_prioritization_rules);
		// callback(write_function, prefix, sizeof(p->max_nr_permitted_destination_mac_addresses), "max_nr_permitted_destination_mac_addresses:", "%d", &p->max_nr_permitted_destination_mac_addresses);
		callback(write_function, prefix, sizeof(p->units), "units:", "%d", &p->units);
		callback(write_function, prefix, sizeof(p->max_total_nr_VIDs), "max_total_nr_VIDs:", "%d", &p->max_total_nr_VIDs);

		// callback(write_function, prefix, sizeof(p->eth_nr), "eth_nr", "%d", &p->eth_nr);
		// int h = 0;
		// for (; h < p->eth_nr; h++) {
		// 	callback(write_function, prefix, sizeof(*(p->interface_identifier)), "interface_identifier", "%02x", p->interface_identifier[h]);
		// }
		return;
	}

	case TLV_TYPE_DEFAULT_802_1Q_SETTINGS: {
		struct Default8021QSettingsTLV *p;

		p = (struct Default8021QSettingsTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->primary_vlan_id), "Primary vlan id:", "%d", &p->primary_vlan_id);
		callback(write_function, prefix, sizeof(p->default_pcp), "Default pcp:", "%d", &p->default_pcp);
		return;
	}

	case TLV_TYPE_TRAFFIC_SEPARATION_POLICY: {
		struct TrafficSeparationPolicyTLV *p;
		INT8U                              k;
		p = (struct TrafficSeparationPolicyTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->ssid_nr), "SSID nr:", "%d", &p->ssid_nr);

		for (k = 0; k < p->ssid_nr; k++) {
			callback(write_function, prefix, sizeof(p->ssid_info[k].ssid_length), "SSID length:", "%d", &p->ssid_info[k].ssid_length);
			callback(write_function, prefix, (sizeof(INT8U) * p->ssid_info[k].ssid_length), "SSID:", "%s", p->ssid_info[k].ssid_name);
			callback(write_function, prefix, sizeof(p->ssid_info[k].vlan_id), "Default pcp:", "%d", &p->ssid_info[k].vlan_id);
		}

		return;
	}

	case TLV_TYPE_BSS_CONFIG_REPORT: {
		struct BSSConfigReportTLV *  p;
		struct BSSConfigRadioReport *radio_report;
		struct BSSConfigBSSReport *  bss_report;

		INT8U k, m;

		p = (struct BSSConfigReportTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->radio_report_nr), "radio report nr:", "%d", &p->radio_report_nr);
		for (k = 0; k < p->radio_report_nr; k++) {
			radio_report = &p->radio_reports[k];
			callback(write_function, prefix, sizeof(radio_report->radio_unique_identifier), "ruid:", "%02X", radio_report->radio_unique_identifier);
			callback(write_function, prefix, sizeof(radio_report->bss_report_nr), "bss report nr:", "%d", &radio_report->bss_report_nr);
			for (m = 0; m < radio_report->bss_report_nr; m++) {
				bss_report = &radio_report->bss_reports[m];
				callback(write_function, prefix, sizeof(bss_report->bssid), "BSSID:", "%02x", bss_report->bssid);
				callback(write_function, prefix, sizeof(bss_report->status_flag), "status flag:", "%d", &bss_report->status_flag);
				callback(write_function, prefix, bss_report->ssid_length, "SSID:", "%02x", bss_report->ssid);
			}
		}

		return;
	}

	case TLV_TYPE_DSCP_MAPPING_TABLE: {
		struct DSCPMappingTableTLV *p;
		p = (struct DSCPMappingTableTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->dscp_to_pcp), "dscp_to_pcp:", "%02x", &p->dscp_to_pcp);

		return;
	}

	case TLV_TYPE_BSS_CONFIG_REQUEST: {
		struct BSSConfigRequestTLV *p;
		p = (struct BSSConfigRequestTLV *)memory_structure;

		callback(write_function, prefix,
		         p->dpp_config_request_object_len, "dpp_config_req_obj :", "%02x",
		         p->dpp_config_request_object);

		return;
	}

	case TLV_TYPE_BSS_CONFIG_RESPONSE: {
		struct BSSConfigResponseTLV *p;
		p = (struct BSSConfigResponseTLV *)memory_structure;

		callback(write_function, prefix,
		         p->dpp_config_object_len, "dpp_config_obj :", "%02x",
		         p->dpp_config_object);

		return;
	}

	case TLV_TYPE_PROFILE_2_ERROR_CODE: {
		struct Profile2ErrorCodeTLV *p;
		p = (struct Profile2ErrorCodeTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->reason_code), "reason_code:", "%d", &p->reason_code);

		callback(write_function, prefix, sizeof(p->service_prioritization_rule_id), "service_prioritization_rule_id:", "%02x", &p->service_prioritization_rule_id);

		return;
	}

	case TLV_TYPE_AP_RADIO_ADVANCED_CAPABILITIES: {
		struct APRadioAdvancedCapabilitiesTLV *p;
		p = (struct APRadioAdvancedCapabilitiesTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->radio_id), "radio_id:", "%02x", &p->radio_id);

		callback(write_function, prefix, sizeof(p->backhaul_bss_traffic_separation_mix_no_support), "backhaul_bss_traffic_separation_mix_no_support:", "%d", &p->backhaul_bss_traffic_separation_mix_no_support);

		return;
	}

	case TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION: {
		struct AssociationStatusNotificationTLV *p;
		INT8U                                    k;
		p = (struct AssociationStatusNotificationTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->bssid_nr), "BSSID nr:", "%d", &p->bssid_nr);

		for (k = 0; k < p->bssid_nr; k++) {
			callback(write_function, prefix, sizeof(p->bss_info[k].bssid), "BSSID:", "%02x", p->bss_info[k].bssid);
			callback(write_function, prefix, sizeof(p->bss_info[k].association_allowance_status), "Association allowance status:", "%d", &p->bss_info[k].association_allowance_status);
		}
		return;
	}

	case TLV_TYPE_SOURCE_INFO: {
		struct SourceInfoTLV *p;
		p = (struct SourceInfoTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->mac_address), "Mac address:", "%02x", p->mac_address);
		return;
	}

	case TLV_TYPE_TUNNELED_MESSAGE_TYPE: {
		struct TunneledMessageTypeTLV *p;
		p = (struct TunneledMessageTypeTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->tunneled_protocol_type_payload), "Tunneled protocol payload type:", "%d", &p->tunneled_protocol_type_payload);
		return;
	}

	case TLV_TYPE_TUNNELED: {
		struct TunneledTLV *p;
		p = (struct TunneledTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->tlv_length), "Payload length:", "%d", &p->tlv_length);
		callback(write_function, prefix, (sizeof(INT8U) * p->tlv_length), "Payload :", "%02x", p->payload);
		return;
	}

	case TLV_TYPE_PROFILE_2_STEERING_REQUEST: {
		struct Profile2SteeringRequestTLV *p;
		INT8U                              k;
		p = (struct Profile2SteeringRequestTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->bssid), "BSSID:", "%02x", p->bssid);
		callback(write_function, prefix, sizeof(p->steer_parameters), "Steer parameters :", "%d", &p->steer_parameters);
		callback(write_function, prefix, sizeof(p->steer_opportunity_window), "Steer opportunity window :", "%d", &p->steer_opportunity_window);
		callback(write_function, prefix, sizeof(p->btm_disassociation_timer), "BTM disassociation timer :", "%d", &p->btm_disassociation_timer);
		callback(write_function, prefix, sizeof(p->sta_list_count), "Sta list count :", "%d", &p->sta_list_count);

		for (k = 0; k < p->sta_list_count; k++) {
			callback(write_function, prefix, sizeof(p->sta_list[k]), "Sta mac :", "%02x", p->sta_list[k]);
		}

		callback(write_function, prefix, sizeof(p->target_bssid_list_count), "Target BSSID list count :", "%d", &p->target_bssid_list_count);

		for (k = 0; k < p->target_bssid_list_count; k++) {
			callback(write_function, prefix, sizeof(p->target_bssid_list[k].target_bssid), "Target BSSID :", "%02x", p->target_bssid_list[k].target_bssid);
			callback(write_function, prefix, sizeof(p->target_bssid_list[k].target_bss_operating_class), "Target BSS operating class:", "%d", &p->target_bssid_list[k].target_bss_operating_class);
			callback(write_function, prefix, sizeof(p->target_bssid_list[k].target_bss_channel_number), "Target BSS channel nr :", "%d", &p->target_bssid_list[k].target_bss_channel_number);
			callback(write_function, prefix, sizeof(p->target_bssid_list[k].reason_code), "Reason code :", "%d", &p->target_bssid_list[k].reason_code);
		}
		return;
	}

	case TLV_TYPE_UNSUCCESSFUL_ASSOCIATION_POILCY: {
		struct UnsuccessfulAssociationPolicyTLV *p;
		p = (struct UnsuccessfulAssociationPolicyTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->report_unsuccessful_associations), "report_unsuccessful_associations :", "%d", &p->report_unsuccessful_associations);
		callback(write_function, prefix, sizeof(p->maximum_reporting_rate), "maximum_reporting_rate :", "%d", &p->maximum_reporting_rate);

		return;
	}

	case TLV_TYPE_METRIC_COLLECTION_INTERVAL: {
		struct MetricCollectionIntervalTLV *p;
		p = (struct MetricCollectionIntervalTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->collection_interval), "collection_interval :", "%d", &p->collection_interval);

		return;
	}

	case TLV_TYPE_RADIO_METRICS: {
		struct RadioMetricsTLV *p;
		p = (struct RadioMetricsTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->radio_unique_identifier), "radio_unique_identifier :", "%02x", &p->radio_unique_identifier);
		callback(write_function, prefix, sizeof(p->noise), "noise :", "%d", &p->noise);
		callback(write_function, prefix, sizeof(p->transmit), "transmit :", "%d", &p->transmit);
		callback(write_function, prefix, sizeof(p->receiveself), "receiveself :", "%d", &p->receiveself);
		callback(write_function, prefix, sizeof(p->receiveother), "receiveother :", "%d", &p->receiveother);

		return;
	}

	case TLV_TYPE_AP_EXTENDED_METRICS: {
		struct APExtendedMetricsTLV *p;
		p = (struct APExtendedMetricsTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->radio_unique_identifier), "radio_unique_identifier :", "%02x", &p->radio_unique_identifier);
		callback(write_function, prefix, sizeof(p->unicast_byte_sent), "unicast_byte_sent :", "%d", &p->unicast_byte_sent);
		callback(write_function, prefix, sizeof(p->unicast_byte_received), "unicast_byte_received :", "%d", &p->unicast_byte_received);
		callback(write_function, prefix, sizeof(p->multicast_byte_sent), "multicast_byte_sent :", "%d", &p->multicast_byte_sent);
		callback(write_function, prefix, sizeof(p->multicast_byte_received), "multicast_byte_received :", "%d", &p->multicast_byte_received);
		callback(write_function, prefix, sizeof(p->broadcast_byte_sent), "broadcast_byte_sent :", "%d", &p->broadcast_byte_sent);
		callback(write_function, prefix, sizeof(p->broadcast_byte_received), "broadcast_byte_received :", "%d", &p->broadcast_byte_received);

		return;
	}

	case TLV_TYPE_ASSOCIATED_STA_EXTENDED_LINK_METRICS: {
		struct AssociatedSTAExtendedLinkMericsTLV *p;
		int                                        k;
		p = (struct AssociatedSTAExtendedLinkMericsTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->sta_mac_address), "sta_mac_address :", "%02x", &p->sta_mac_address);
		callback(write_function, prefix, sizeof(p->bssid_nr), "bssid_nr :", "%d", &p->bssid_nr);

		for (k = 0; k < p->bssid_nr; k++) {
			callback(write_function, prefix, sizeof(p->reported_bss[k].bssid), "reported_bssid :", "%02x", p->reported_bss[k].bssid);
			callback(write_function, prefix, sizeof(p->reported_bss[k].last_data_downlink_rate), "Target BSS operating class:", "%d", &p->reported_bss[k].last_data_downlink_rate);
			callback(write_function, prefix, sizeof(p->reported_bss[k].last_data_uplink_rate), "last_data_uplink_rate :", "%d", &p->reported_bss[k].last_data_uplink_rate);
			callback(write_function, prefix, sizeof(p->reported_bss[k].utilization_receive), "utilization_receive:", "%d", &p->reported_bss[k].utilization_receive);
			callback(write_function, prefix, sizeof(p->reported_bss[k].utilization_transmit), "utilization_transmit:", "%d", &p->reported_bss[k].utilization_transmit);
		}
		return;
	}

	case TLV_TYPE_STATUS_CODE: {
		struct StatusCodeTLV *p;
		p = (struct StatusCodeTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->status_code), "status_code :", "%d", &p->status_code);

		return;
	}

	case TLV_TYPE_REASON_CODE: {
		struct ReasonCodeTLV *p;
		p = (struct ReasonCodeTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->reason_code), "reason_code :", "%d", &p->reason_code);

		return;
	}

	case TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES: {
		struct BackhaulSTARadioCapabilitiesTLV *p;
		p = (struct BackhaulSTARadioCapabilitiesTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->radio_unique_identifier), "radio_unique_identifier :", "%02x", p->radio_unique_identifier);
		callback(write_function, prefix, sizeof(p->included_mac), "included_mac :", "%d", &p->included_mac);
		callback(write_function, prefix, sizeof(p->sta_mac_address), "sta_mac_address :", "%02x", p->sta_mac_address);

		return;
	}

	case TLV_TYPE_AKM_SUITE_CAPABILITIES: {
		struct AKMSuiteCapabilitiesTLV *p;
		INT8U                           i;
		p = (struct AKMSuiteCapabilitiesTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->backhaul_bss_akm_suite_selectors_nr),
		         "backhaul_bss_akm_suite_selectors_nr :", "%d", &p->backhaul_bss_akm_suite_selectors_nr);
		for (i = 0; i < p->backhaul_bss_akm_suite_selectors_nr; i++) {
			callback(write_function, prefix, sizeof(p->backhaul_bss_akm_suite_selectors[i].oui),
			         "oui :", "%02x", p->backhaul_bss_akm_suite_selectors[i].oui);
			callback(write_function, prefix, sizeof(p->backhaul_bss_akm_suite_selectors[i].akm_suite_type),
			         "akm_suite_type :", "%d", &p->backhaul_bss_akm_suite_selectors[i].akm_suite_type);
		}

		callback(write_function, prefix, sizeof(p->fronthaul_bss_akm_suite_selectors_nr),
		         "fronthaul_bss_akm_suite_selectors_nr :", "%d", &p->fronthaul_bss_akm_suite_selectors_nr);
		for (i = 0; i < p->fronthaul_bss_akm_suite_selectors_nr; i++) {
			callback(write_function, prefix, sizeof(p->fronthaul_bss_akm_suite_selectors[i].oui),
			         "oui :", "%02x", p->fronthaul_bss_akm_suite_selectors[i].oui);
			callback(write_function, prefix, sizeof(p->fronthaul_bss_akm_suite_selectors[i].akm_suite_type),
			         "akm_suite_type :", "%d", &p->fronthaul_bss_akm_suite_selectors[i].akm_suite_type);
		}

		return;
	}

	case TLV_TYPE_1905_ENCAP_DPP: {
		struct Encap1905DPPTLV *p;
		p = (struct Encap1905DPPTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->enrollee_mac_address_present), "enrollee_mac_address_present :", "%d", &p->enrollee_mac_address_present);
		callback(write_function, prefix, sizeof(p->dest_sta_mac_address), "dest_sta_mac_address :", "%02x", p->dest_sta_mac_address);
		callback(write_function, prefix, sizeof(p->frame_type), "frame_type :", "%d", &p->frame_type);
		callback(write_function, prefix, sizeof(p->encapsulated_frame_len), "encapsulated_frame_len :", "%d", &p->encapsulated_frame_len);
		callback(write_function, prefix, p->encapsulated_frame_len, "encapsulated_frame :", "%02x", p->encapsulated_frame);

		return;
	}

	case TLV_TYPE_1905_ENCAP_EAPOL: {
		struct Encap1905EAPOLTLV *p;
		p = (struct Encap1905EAPOLTLV *)memory_structure;

		callback(write_function, prefix, p->eapol_frame_payload_len, "eapol_frame_payload :", "%02x", p->eapol_frame_payload);

		return;
	}

	case TLV_TYPE_DPP_BOOTSTRAPPING_URI_NOTIFICATION: {
		struct DPPBootstrappingURINotificationTLV *p;
		p = (struct DPPBootstrappingURINotificationTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->radio_unique_identifier), "radio_unique_identifier :", "%02x", p->radio_unique_identifier);
		callback(write_function, prefix, sizeof(p->local_interface_mac_address), "local_interface_mac_address :", "%02x", p->local_interface_mac_address);
		callback(write_function, prefix, sizeof(p->sta_mac_address), "sta_mac_address :", "%02x", p->sta_mac_address);
		callback(write_function, prefix, p->dpp_bootstrapping_uri_len, "dpp_bootstrapping_uri :", "%02x", p->dpp_bootstrapping_uri);

		return;
	}

	case TLV_TYPE_BACKHAUL_BSS_CONFIGURATION: {
		struct BackhaulBSSConfigurationTLV *p;
		p = (struct BackhaulBSSConfigurationTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->bssid), "bssid :", "%02x", p->bssid);
		callback(write_function, prefix, sizeof(p->association_disallowed), "association_disallowed :", "%d", &p->association_disallowed);

		return;
	}

	case TLV_TYPE_DPP_MESSAGE: {
		struct DPPMessageTLV *p;
		p = (struct DPPMessageTLV *)memory_structure;

		callback(write_function, prefix, p->dpp_frame_len, "dpp_frame :", "%02x", p->dpp_frame);

		return;
	}

	case TLV_TYPE_ANTICIPATED_CHANNEL_PERFERENCE: {
		struct AnticipatedChannelPerferenceTLV * p;
		struct OperatingChannelClass *operate_class;

		p = (struct AnticipatedChannelPerferenceTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->operating_class_nr), "operating_class_nr:", "%d", &p->operating_class_nr);
		int k;
		for (k = 0; k < p->operating_class_nr; k++) {
			operate_class = &p->operate_class[k];
			callback(write_function, prefix, sizeof(operate_class->operating_class), "operating_class:", "%d", &operate_class->operating_class);
			callback(write_function, prefix, sizeof(operate_class->channel_list_nr), "channel_list_nr:", "%d", &operate_class->channel_list_nr);
			int m;
			for (m = 0; m < operate_class->channel_list_nr; m++) {
				callback(write_function, prefix, sizeof(operate_class->channel_list[m]), "channel_list[m]:", "%d", &operate_class->channel_list[m]);
			}
		}
		return;
	}

	case TLV_TYPE_ANTICIPATED_CHANNEL_USAGE: {
		struct AnticipatedChannelUsageTLV * p;
		struct UsageEntryClass *usage_entry;

		p = (struct AnticipatedChannelUsageTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->operating_class), "operating_class:", "%d", &p->operating_class);
		callback(write_function, prefix, sizeof(p->channel_number), "channel_number:", "%d", &p->channel_number);
		callback(write_function, prefix, sizeof(p->reference_bssid), "reference_bssid:", "%02x", p->reference_bssid);
		callback(write_function, prefix, sizeof(p->usage_entry_nr), "usage_entry_nr:", "%d", &p->usage_entry_nr);
		int k;
		for (k = 0; k < p->usage_entry_nr; k++) {
			usage_entry = &p->usage_entry[k];
			callback(write_function, prefix, sizeof(usage_entry->burst_start_time), "burst_start_time:", "%d", &usage_entry->burst_start_time);
			callback(write_function, prefix, sizeof(usage_entry->burst_length), "burst_length:", "%d", &usage_entry->burst_length);
			callback(write_function, prefix, sizeof(usage_entry->repititions), "repititions:", "%d", &usage_entry->repititions);
			callback(write_function, prefix, sizeof(usage_entry->burst_interval), "burst_interval:", "%d", &usage_entry->burst_interval);
			callback(write_function, prefix, sizeof(usage_entry->RU_bitmask_length), "RU_bitmask_length:", "%d", &usage_entry->RU_bitmask_length);
			int m;
			for (m = 0; m < usage_entry->RU_bitmask_length; m++) {
				callback(write_function, prefix, sizeof(usage_entry->RU_bitmask[m]), "RU_bitmask[m]:", "%d", &usage_entry->RU_bitmask[m]);
			}
			callback(write_function, prefix, sizeof(usage_entry->transmitter_identifier), "transmitter_identifier:", "%02x", usage_entry->transmitter_identifier);
			callback(write_function, prefix, sizeof(usage_entry->power_level), "power_level:", "%d", &usage_entry->power_level);
			callback(write_function, prefix, sizeof(usage_entry->channel_usage_reason), "channel_usage_reason:", "%d", &usage_entry->channel_usage_reason);
		}
		return;
	}

	case TLV_TYPE_SPATIAL_REUSE_REQUEST: {
		struct SpatialReuseRequestTLV * p;

		p = (struct SpatialReuseRequestTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->ruid), "ruid:", "%02x", p->ruid);
		callback(write_function, prefix, sizeof(p->bss_color), "bss_color:", "%d", &p->bss_color);
		callback(write_function, prefix, sizeof(p->valid_field), "valid_field:", "%02x", &p->valid_field);
		callback(write_function, prefix, sizeof(p->non_srg_obsspd_max_offset), "non_srg_obsspd_max_offset:", "%d", &p->non_srg_obsspd_max_offset);
		callback(write_function, prefix, sizeof(p->srg_obsspd_min_offset), "srg_obsspd_min_offset:", "%d", &p->srg_obsspd_min_offset);
		callback(write_function, prefix, sizeof(p->srg_obsspd_max_offset), "srg_obsspd_max_offset:", "%d", &p->srg_obsspd_max_offset);
		callback(write_function, prefix, sizeof(p->srg_bss_color_bitmap), "srg_bss_color_bitmap:", "%d", &p->srg_bss_color_bitmap);
		callback(write_function, prefix, sizeof(p->srg_partial_bssid_bitmap), "srg_partial_bssid_bitmap:", "%02x", p->srg_partial_bssid_bitmap);
		return;
	}

	case TLV_TYPE_SPATIAL_REUSE_REPORT: {
		struct SpatialReuseReportTLV * p;

		p = (struct SpatialReuseReportTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->ruid), "ruid:", "%02x", p->ruid);
		callback(write_function, prefix, sizeof(p->bss_color), "bss_color:", "%d", &p->bss_color);
		callback(write_function, prefix, sizeof(p->valid_field), "valid_field:", "%02x", &p->valid_field);
		callback(write_function, prefix, sizeof(p->non_srg_obsspd_max_offset), "non_srg_obsspd_max_offset:", "%d", &p->non_srg_obsspd_max_offset);
		callback(write_function, prefix, sizeof(p->srg_obsspd_min_offset), "srg_obsspd_min_offset:", "%d", &p->srg_obsspd_min_offset);
		callback(write_function, prefix, sizeof(p->srg_obsspd_max_offset), "srg_obsspd_max_offset:", "%d", &p->srg_obsspd_max_offset);
		callback(write_function, prefix, sizeof(p->srg_bss_color_bitmap), "srg_bss_color_bitmap:", "%d", &p->srg_bss_color_bitmap);
		callback(write_function, prefix, sizeof(p->srg_partial_bssid_bitmap), "srg_partial_bssid_bitmap:", "%02x", p->srg_partial_bssid_bitmap);
		callback(write_function, prefix, sizeof(p->neighbor_bss_color_inuse_bitmap), "neighbor_bss_color_inuse_bitmap:", "%d", &p->neighbor_bss_color_inuse_bitmap);
		return;
	}

	case TLV_TYPE_SPATIAL_REUSE_CONFIG_RESPONSE: {
		struct SpatialReuseConfigResponseTLV * p;

		p = (struct SpatialReuseConfigResponseTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->ruid), "ruid:", "%02x", p->ruid);
		callback(write_function, prefix, sizeof(p->response_code), "response_code:", "%d", &p->response_code);
		return;
	}

	case TLV_TYPE_QOS_MANAGEMENT_POLICY: {
		struct QoSManagementPolicyTLV * p;

		p = (struct QoSManagementPolicyTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->mscsd_disallowed_sta_nr), "mscsd_disallowed_sta_nr:", "%d", &p->mscsd_disallowed_sta_nr);
		int k;
		for (k = 0; k < p->mscsd_disallowed_sta_nr; k++) {
			callback(write_function, prefix, sizeof(p->mscsd_disallowed_sta_mac_address[k]), "mscsd_disallowed_sta_mac_address[k]:", "%02x", p->mscsd_disallowed_sta_mac_address[k]);
		}
		callback(write_function, prefix, sizeof(p->scsd_disallowed_sta_nr), "scsd_disallowed_sta_nr:", "%d", &p->scsd_disallowed_sta_nr);
		int m;
		for (m = 0; m < p->scsd_disallowed_sta_nr; m++) {
			callback(write_function, prefix, sizeof(p->scsd_disallowed_sta_mac_address[m]), "scsd_disallowed_sta_mac_address[m]:", "%02x", p->scsd_disallowed_sta_mac_address[m]);
		}
		return;
	}

	case TLV_TYPE_QOS_MANAGEMENT_DESCRIPTOR: {
		struct QoSManagementDescriptorTLV *p;

		p = (struct QoSManagementDescriptorTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->qmid), "qmid:", "%02x", &p->qmid);
		callback(write_function, prefix, sizeof(p->bssid), "bssid:", "%02x", p->bssid);
		callback(write_function, prefix, sizeof(p->client_mac), "client_mac:", "%02x", &p->client_mac);
		callback(write_function, prefix, sizeof(p->descriptor_element_len_1), "descriptor_element_len_1:", "%d", &p->descriptor_element_len_1);
		callback(write_function, prefix, sizeof(p->descriptor_element_1), "descriptor_element_1:", "%02x", &p->descriptor_element_1);
		callback(write_function, prefix, sizeof(p->descriptor_element_len_2), "descriptor_element_len_2:", "%d", &p->descriptor_element_len_2);
		callback(write_function, prefix, sizeof(p->descriptor_element_2), "descriptor_element_2:", "%02x", &p->descriptor_element_2);
		return;
	}

	case TLV_TYPE_CONTROLLER_CAPABILITY: {
		struct ControllerCapabilityTLV * p;

		p = (struct ControllerCapabilityTLV *)memory_structure;

		callback(write_function, prefix, sizeof(p->KiBMiB_counter), "KiBMiB_counter:", "%d", &p->KiBMiB_counter);
		return;
	}


	default: {
		// Ignore
		//
		return;
	}
	}

	// This code cannot be reached
	//
	return;
}

char *convert_1905_TLV_type_to_string(INT8U tlv_type)
{
	switch (tlv_type) {
	case TLV_TYPE_END_OF_MESSAGE:
		return "TLV_TYPE_END_OF_MESSAGE";
	case TLV_TYPE_VENDOR_SPECIFIC:
		return "TLV_TYPE_VENDOR_SPECIFIC";
	case TLV_TYPE_AL_MAC_ADDRESS_TYPE:
		return "TLV_TYPE_AL_MAC_ADDRESS_TYPE";
	case TLV_TYPE_MAC_ADDRESS_TYPE:
		return "TLV_TYPE_MAC_ADDRESS_TYPE";
	case TLV_TYPE_DEVICE_INFORMATION_TYPE:
		return "TLV_TYPE_DEVICE_INFORMATION_TYPE";
	case TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES:
		return "TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES";
	case TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST:
		return "TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST";
	case TLV_TYPE_NEIGHBOR_DEVICE_LIST:
		return "TLV_TYPE_NEIGHBOR_DEVICE_LIST";
	case TLV_TYPE_LINK_METRIC_QUERY:
		return "TLV_TYPE_LINK_METRIC_QUERY";
	case TLV_TYPE_TRANSMITTER_LINK_METRIC:
		return "TLV_TYPE_TRANSMITTER_LINK_METRIC";
	case TLV_TYPE_RECEIVER_LINK_METRIC:
		return "TLV_TYPE_RECEIVER_LINK_METRIC";
	case TLV_TYPE_LINK_METRIC_RESULT_CODE:
		return "TLV_TYPE_LINK_METRIC_RESULT_CODE";
	case TLV_TYPE_SEARCHED_ROLE:
		return "TLV_TYPE_SEARCHED_ROLE";
	case TLV_TYPE_AUTOCONFIG_FREQ_BAND:
		return "TLV_TYPE_AUTOCONFIG_FREQ_BAND";
	case TLV_TYPE_SUPPORTED_ROLE:
		return "TLV_TYPE_SUPPORTED_ROLE";
	case TLV_TYPE_SUPPORTED_FREQ_BAND:
		return "TLV_TYPE_SUPPORTED_FREQ_BAND";
	case TLV_TYPE_WSC:
		return "TLV_TYPE_WSC";
	case TLV_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION:
		return "TLV_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION";
	case TLV_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION:
		return "TLV_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION";
	case TLV_TYPE_SUPPORTED_SERVICE:
		return "TLV_TYPE_SUPPORTED_SERVICE";
	case TLV_TYPE_SEARCHED_SERVICE:
		return "TLV_TYPE_SEARCHED_SERVICE";
	case TLV_TYPE_AP_RADIO_IDENTIFIER:
		return "TLV_TYPE_AP_RADIO_IDENTIFIER";
	case TLV_TYPE_AP_OPERATIONAL_BSS:
		return "TLV_TYPE_AP_OPERATIONAL_BSS";
	case TLV_TYPE_ASSOCIATED_CLIENTS:
		return "TLV_TYPE_ASSOCIATED_CLIENTS";
	case TLV_TYPE_AP_CAPABILITY:
		return "TLV_TYPE_AP_CAPABILITY";
	case TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES:
		return "TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES";
	case TLV_TYPE_AP_HT_CAPABILITIES:
		return "TLV_TYPE_AP_HT_CAPABILITIES";
	case TLV_TYPE_AP_VHT_CAPABILITIES:
		return "TLV_TYPE_AP_VHT_CAPABILITIES";
	case TLV_TYPE_AP_HE_CAPABILITIES:
		return "TLV_TYPE_AP_HE_CAPABILITIES";
	case TLV_TYPE_STEERING_POLICY:
		return "TLV_TYPE_STEERING_POLICY";
	case TLV_TYPE_METRIC_REPORT_POLICY:
		return "TLV_TYPE_METRIC_REPORT_POLICY";
	case TLV_TYPE_CHANNEL_PREFERENCE:
		return "TLV_TYPE_CHANNEL_PREFERENCE";
	case TLV_TYPE_RADIO_OPERATION_RESTRICTION:
		return "TLV_TYPE_RADIO_OPERATION_RESTRICTION";
	case TLV_TYPE_TRANSMIT_POWER_LIMIT:
		return "TLV_TYPE_TRANSMIT_POWER_LIMIT";
	case TLV_TYPE_CHANNEL_SELECTION_RESPONSE:
		return "TLV_TYPE_CHANNEL_SELECTION_RESPONSE";
	case TLV_TYPE_OPERATING_CHANNEL_REPORT:
		return "TLV_TYPE_OPERATING_CHANNEL_REPORT";
	case TLV_TYPE_CLIENT_INFO:
		return "TLV_TYPE_CLIENT_INFO";
	case TLV_TYPE_CLIENT_CAPABLITY_REPORT:
		return "TLV_TYPE_CLIENT_CAPABLITY_REPORT";
	case TLV_TYPE_CLIENT_ASSOCIATION_EVENT:
		return "TLV_TYPE_CLIENT_ASSOCIATION_EVENT";
	case TLV_TYPE_AP_METRIC_QUERY:
		return "TLV_TYPE_AP_METRIC_QUERY";
	case TLV_TYPE_AP_METRICS:
		return "TLV_TYPE_AP_METRICS";
	case TLV_TYPE_STA_MAC_ADDRESS_TYPE:
		return "TLV_TYPE_STA_MAC_ADDRESS_TYPE";
	case TLV_TYPE_ASSOCIATED_STA_LINK_METRICS:
		return "TLV_TYPE_ASSOCIATED_STA_LINK_METRICS";
	case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY:
		return "TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY";
	case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE:
		return "TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE";
	case TLV_TYPE_BEACON_METRICS_QUERY:
		return "TLV_TYPE_BEACON_METRICS_QUERY";
	case TLV_TYPE_BEACON_METRICS_RESPONSE:
		return "TLV_TYPE_BEACON_METRICS_RESPONSE";
	case TLV_TYPE_STEERING_REQUEST:
		return "TLV_TYPE_STEERING_REQUEST";
	case TLV_TYPE_STEERING_BTM_REPORT:
		return "TLV_TYPE_STEERING_BTM_REPORT";
	case TLV_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST:
		return "TLV_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST";
	case TLV_TYPE_BACKHAUL_STEERING_REQUEST:
		return "TLV_TYPE_BACKHAUL_STEERING_REQUEST";
	case TLV_TYPE_BACKHAUL_STEERING_RESPONSE:
		return "TLV_TYPE_BACKHAUL_STEERING_RESPONSE";
	case TLV_TYPE_HIGHER_LAYER_DATA:
		return "TLV_TYPE_HIGHER_LAYER_DATA";
	case TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS:
		return "TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS";
	case TLV_TYPE_ERROR_CODE:
		return "TLV_TYPE_ERROR_CODE";
	case TLV_TYPE_CHANNEL_SCAN_REPORTING_POLICY:
		return "TLV_TYPE_CHANNEL_SCAN_REPORTING_POLICY";
	case TLV_TYPE_CHANNEL_SCAN_CAPABILITIES:
		return "TLV_TYPE_CHANNEL_SCAN_CAPABILITIES";
	case TLV_TYPE_CHANNEL_SCAN_REQUEST:
		return "TLV_TYPE_CHANNEL_SCAN_REQUEST";
	case TLV_TYPE_CHANNEL_SCAN_RESULT:
		return "TLV_TYPE_CHANNEL_SCAN_RESULT";
	case TLV_TYPE_TIMESTAMP:
		return "TLV_TYPE_TIMESTAMP";
	case TLV_TYPE_CAC_REQUEST:
		return "TLV_TYPE_CAC_REQUEST";
	case TLV_TYPE_CAC_TERMINATION:
		return "TLV_TYPE_CAC_TERMINATION";
	case TLV_TYPE_CAC_COMPLETION_REPORT:
		return "TLV_TYPE_CAC_COMPLETION_REPORT ";
	case TLV_TYPE_CAC_STATUS_REPORT:
		return "TLV_TYPE_CAC_STATUS_REPORT";
	case TLV_TYPE_CAC_CAPABILITIES:
		return "TLV_TYPE_CAC_CAPABILITIES";
	case TLV_TYPE_1905_LAYER_SECURITY_CAPABILITY:
		return "TLV_TYPE_1905_LAYER_SECURITY_CAPABILITY";
	case TLV_TYPE_MIC:
		return "TLV_TYPE_MIC";
	case TLV_TYPE_MULTIAP_PROFILE:
		return "TLV_TYPE_MULTIAP_PROFILE";
	case TLV_TYPE_DEFAULT_802_1Q_SETTINGS:
		return "TLV_TYPE_DEFAULT_802_1Q_SETTINGS";
	case TLV_TYPE_TRAFFIC_SEPARATION_POLICY:
		return "TLV_TYPE_TRAFFIC_SEPARATION_POLICY";
	case TLV_TYPE_DSCP_MAPPING_TABLE:
		return "TLV_TYPE_DSCP_MAPPING_TABLE";
	case TLV_TYPE_PROFILE_2_ERROR_CODE:
		return "TLV_TYPE_PROFILE_2_ERROR_CODE";
	case TLV_TYPE_AP_RADIO_ADVANCED_CAPABILITIES:
		return "TLV_TYPE_AP_RADIO_ADVANCED_CAPABILITIES";
	case TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION:
		return "TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION";
	case TLV_TYPE_SOURCE_INFO:
		return "TLV_TYPE_SOURCE_INFO";
	case TLV_TYPE_TUNNELED_MESSAGE_TYPE:
		return "TLV_TYPE_TUNNELED_MESSAGE_TYPE";
	case TLV_TYPE_TUNNELED:
		return "TLV_TYPE_TUNNELED";
	case TLV_TYPE_PROFILE_2_STEERING_REQUEST:
		return "TLV_TYPE_PROFILE_2_STEERING_REQUEST";
	case TLV_TYPE_UNSUCCESSFUL_ASSOCIATION_POILCY:
		return "TLV_TYPE_UNSUCCESSFUL_ASSOCIATION_POILCY";
	case TLV_TYPE_METRIC_COLLECTION_INTERVAL:
		return "TLV_TYPE_METRIC_COLLECTION_INTERVAL";
	case TLV_TYPE_RADIO_METRICS:
		return "TLV_TYPE_RADIO_METRICS";
	case TLV_TYPE_AP_EXTENDED_METRICS:
		return "TLV_TYPE_AP_EXTENDED_METRICS";
	case TLV_TYPE_ASSOCIATED_STA_EXTENDED_LINK_METRICS:
		return "TLV_TYPE_ASSOCIATED_STA_EXTENDED_LINK_METRICS";
	case TLV_TYPE_STATUS_CODE:
		return "TLV_TYPE_STATUS_CODE";
	case TLV_TYPE_REASON_CODE:
		return "TLV_TYPE_REASON_CODE";
	case TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES:
		return "TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES";
	case TLV_TYPE_AKM_SUITE_CAPABILITIES:
		return "TLV_TYPE_AKM_SUITE_CAPABILITIES";
	case TLV_TYPE_1905_ENCAP_DPP:
		return "TLV_TYPE_1905_ENCAP_DPP";
	case TLV_TYPE_1905_ENCAP_EAPOL:
		return "TLV_TYPE_1905_ENCAP_EAPOL";
	case TLV_TYPE_DPP_BOOTSTRAPPING_URI_NOTIFICATION:
		return "TLV_TYPE_DPP_BOOTSTRAPPING_URI_NOTIFICATION";
	case TLV_TYPE_BACKHAUL_BSS_CONFIGURATION:
		return "TLV_TYPE_BACKHAUL_BSS_CONFIGURATION";
	case TLV_TYPE_AP_WIFI_6_CAPABILITIES:
		return "TLV_TYPE_AP_WIFI_6_CAPABILITIES";
	case TLV_TYPE_ASSOCIATED_WIFI_6_STA_STATUS_REPORT:
		return "TLV_TYPE_ASSOCIATED_WIFI_6_STA_STATUS_REPORT";
	case TLV_TYPE_BSS_CONFIG_REPORT:
		return "TLV_TYPE_BSS_CONFIG_REPORT";
	case TLV_TYPE_DEVICE_INVENTORY:
		return "TLV_TYPE_DEVICE_INVENTORY";
	case TLV_TYPE_AGENT_LIST:
		return "TLV_TYPE_AGENT_LIST";
	case TLV_TYPE_DPP_CHIRP_VALUE:
		return "TLV_TYPE_DPP_CHIRP_VALUE";
	case TLV_TYPE_BSS_CONFIG_REQUEST:
		return "TLV_TYPE_BSS_CONFIG_REQUEST";
	case TLV_TYPE_BSS_CONFIG_RESPONSE:
		return "TLV_TYPE_BSS_CONFIG_RESPONSE";
	case TLV_TYPE_DPP_MESSAGE:
		return "TLV_TYPE_DPP_MESSAGE";
	case TLV_TYPE_SERVICE_PRIORITIZATION_RULE:
		return "TLV_TYPE_SERVICE_PRIORITIZATION_RULE";
	case TLV_TYPE_DPP_CCE_INDICATION:
		return "TLV_TYPE_DPP_CCE_INDICATION";
	case TLV_TYPE_SPATIAL_REUSE_REQUEST:
		return "TLV_TYPE_SPATIAL_REUSE_REQUEST";
	case TLV_TYPE_SPATIAL_REUSE_REPORT:
		return "TLV_TYPE_SPATIAL_REUSE_REPORT";

	default:
		return "Unknown";
	}

	// This code cannot be reached
	//
	return "";
}
