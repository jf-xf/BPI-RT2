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


#ifndef _AL_SEND_H_
#define _AL_SEND_H_

#include "1905_cmdus.h"
#include "1905_alme.h"
#include "1905_tlvs.h"
#include "multi_ap_tlvs.h"
#include "multi_ap_tlvs_r2.h"
#include "multi_ap_tlvs_r3.h"
#include "multi_ap_tlvs_r4.h"

#include "map_utils.h"

#include "al.h"
#include "al_dpp_eapol.h"

////////////////////////////////////////////////////////////////////////////////
// Functions to send "raw" 1905 messages
////////////////////////////////////////////////////////////////////////////////

void freeChannelPreferenceTLV(struct ChannelPreferenceTLV **t, INT8U TLV_nr);

void freeTransmitPowerLimitTLV(struct TransmitPowerLimitTLV **t, INT8U TLV_nr);

// This function sends a "1905 packet" (the one represented by the provided
// 'cmdu' structure) on the provided interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8"
//
// 'dst_mac_address' is the destination MAC address that will be contained in
// the ETH header.
//
// Return '0' if there was a problem, '1' otherwise.
//
INT8U send1905RawPacket(char *interface_name, INT16U mid, INT8U *dst_mac_address, struct CMDU *cmdu, bool need_ack, INT8U is_forwarding);

INT8U send1905RawPacketWithSrc(char *interface_name, INT16U mid, INT8U *dst_mac_address, INT8U *src_mac_address, struct CMDU *cmdu, bool need_ack, INT8U is_forwarding);
// This function sends a "1905 ALME reply" (the one represented by the provided
// 'out' pointer, which must point to a "struct *ALME" structure).
//
// 'alme_client_id' is used to identify the ALME entity where the message must
// be send. It must then match the same number originally received in the
// original ALME request message that causes this response.
//
// Return '0' if there was a problem, '1' otherwise.
//
INT8U send1905RawALME(INT8U alme_client_id, INT8U *out);

// This function encrypts 1905 layer raw packets
// Encryption is done based on EasyMesh Specification v3.0 chapter 13.3
//
void encrypt1905Packet(INT8U *dst_mac_address, INT8U *mcast_address, struct CMDU *cmdu, INT8U is_forwarding);

// This function encrypts 1905 layer raw ALME
// Encryption is done based on EasyMesh Specification v3.0 chapter 13.3
//
void encrypt1905Alme(struct multiApCommandRequestALME *p, INT16U mid);

////////////////////////////////////////////////////////////////////////////////
// Functions to send "specialized" 1905 messages
////////////////////////////////////////////////////////////////////////////////

// This function sends a "1905 topology discovery packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8"
//
// The format of this packet is detailed in "Section 6.3.1"
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U send1905TopologyDiscoveryPacket(char *interface_name, INT16U mid);

// This function sends a "1905 topology query packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8"
//
// The format of this packet is detailed in "Section 6.3.2"
//
// 'destination_al_mac_address' is the neighbor AL MAC address where this query
// will be sent to.
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U send1905TopologyQueryPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U need_ack);

// This function sends a "1905 topology response packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8" (ie. it
// must be the same as the "messsage id" contained in the original "topology
// query" message)
//
// 'destination_al_mac_address' is the neighbor AL MAC address where this query
// will be sent to (ie. the AL MAC address of the node that sent the original
// "topology query" message)
//
// The format of this packet is detailed in "Section 6.3.3"
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U send1905TopologyResponsePacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address);

// This function sends a "1905 topology notification packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8"
//
// The format of this packet is detailed in "Section 6.3.4"
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U send1905TopologyNotificationPacket(char *interface_name, INT16U mid, INT8U client_mac_addr[], INT8U bssid[], INT8U event);

// This function sends a "1905 metrics query packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8"
//
// 'destination_al_mac_address' is the neighbor AL MAC address where this query
// will be sent to.
//
// The format of this packet is detailed in "Section 6.3.5"
//
// Note that the the query produced by this functions always asks for both the
// tx and rx metrics for all of the destination's neighbors.
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U send1905MetricsQueryPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U destination, INT8U type, INT8U *sta_mac);

// This function sends a "1905 metrics response packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8" (ie. it
// must be the same as the "messsage id" contained in the original "topology
// query" message)
//
// 'destination_al_mac_address' is the neighbor AL MAC address where this query
// will be sent to (ie. the AL MAC address of the node that sent the original
// "metrics query" message)
//
// 'destination' is either LINK_METRIC_QUERY_TLV_ALL_NEIGHBORS or
// LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR.
// In the first case, the metrics between 'destination_al_mac_address' and all
// of its neighbors are contained in the response.
// In the later case, only the metrics between 'destination_mac_address' and
// 'specific_neighbor' are contained in the response.
//
// 'specific_neighbor' is the AL MAC of a neighbor of 'destination_mac_address'.
// It is only used when 'destination' is set to
// LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR
//
// 'metrics_type' can be one of these:
//    - LINK_METRIC_QUERY_TLV_TX_LINK_METRICS_ONLY
//    - LINK_METRIC_QUERY_TLV_RX_LINK_METRICS_ONLY
//    - LINK_METRIC_QUERY_TLV_BOTH_TX_AND_RX_LINK_METRICS
//
// The format of this packet is detailed in "Section 6.3.6"
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U send1905MetricsResponsePacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U destination, INT8U *specific_neighbor, INT8U metrics_type);

// This function sends a "1905 push button event notification packet" on the
// provided interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8" (ie. it
// must be the same as the "messsage id" contained in the original "topology
// query" message)
//
// 'all_interfaces_names' is a list of interfaces names that will be used to
// build the contents of the "push button event" TLV that the message will
// contain.
// This list contains 'nr' elements and only those whose index ("i") satisfies
// 'push_button_mask[i] == 0' will actually be included in the TLV list.
//
// Example:
//   all_interfaces_names = { "eth0", "eth1" }  |  Only data regarding "eth0"
//   push_button_mask     = [      0,      1 }  |  will be included in the TLVs
//   nr                   = 2                   |
//
// The format of this packet is detailed in "Section 6.3.11"
//
// The destination MAC address of the packet will always be the 1905 multicast
// address ("01:80:C2:00:00:13")
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U send1905PushButtonEventNotificationPacket(char *interface_name, INT16U mid, char **all_interfaces_names, INT8U *push_button_mask, INT8U nr);

// This function sends a "1905 push join notification packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8" (ie. it
// must be the same as the "messsage id" contained in the original "topology
// query" message)
//
// 'original_al_mac_address' is the AL MAC address contained in the original
// "push button event notification" message that resulted in this "push button
// join notification" message beeing generated.
//
//'original_mid' is the "message ID" of the original "push button event
// notification" message that resulted in this "push button join notification"
// message beeing generated.
//
// 'local_mac_address' is the MAC address of the interface where the "push
// button" process successfully completed.
//
// 'new_mac_address' is the MAC address of the interface at the other end of the
// link which has just been secured.
// For "ethernet" interfaces this must be set to all zeros.
//
// The destination MAC address of the packet will always be the 1905 multicast
// address ("01:80:C2:00:00:13")
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U send1905PushButtonJoinNotificationPacket(char *interface_name, INT16U mid, INT8U *original_al_mac_address, INT16U original_mid, INT8U *local_mac_address, INT8U *new_mac_address);

// This function sends a "AP-autoconfiguration renew packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8"
//
// The destination MAC address of the packet will always be the 1905 multicast
// address ("01:80:C2:00:00:13")
//
// Return "0" if a problem was found. "1" otherwise.
INT8U send1905APAutoconfigurationRenewPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, unsigned char band);

// This function sends a "AP-autoconfiguration search packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8"
//
// 'freq_band' is the wifi frequency band of the unconfigured AP interface that
// is searching for configuration parameters. It can take any of the following
// values:
//   - IEEE80211_FREQUENCY_BAND_2_4_GHZ
//   - IEEE80211_FREQUENCY_BAND_5_GHZ
//   - IEEE80211_FREQUENCY_BAND_60_GHZ
//
// The destination MAC address of the packet will always be the 1905 multicast
// address ("01:80:C2:00:00:13")
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U send1905APAutoconfigurationSearchPacket(char *interface_name, INT16U mid, INT8U freq_band);

// This function sends a "AP-autoconfiguration response packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8". In other
// words it must match the value of the 'mid' field contained in the original
// "AP autoconfiguration response" message that resulted in this response being
// generated.
//
// 'destination_al_mac_address' is the neighbor AL MAC address where this query
// will be sent to. It must match the 'AL MAC' field contained in the original
// "AP autoconfiguration response" message that resulted in this response being
// generated.
//
// 'freq_band' is the wifi frequency band supported by this AP/registrar. It
// must match the 'freq_band' field contained in the original "AP
// autoconfiguration response" message that resulted in this response being
// generated.
//
// 'dpp_chirp' is the device provisioning protocol (Easy Connect) presence
// announcement information. since Multi-AP R3, this response should optionally
// include chirp information to tell agent that it has received and parsed
// the DPP chirp info from the "AP autoconfiguration search" message.
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U send1905APAutoconfigurationResponsePacket(char * interface_name,
                                                INT16U mid,
                                                INT8U *destination_al_mac_address,
                                                INT8U  freq_band,
                                                INT8U  dpp_chirp);

// This function sends a "AP-autoconfiguration WSC packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8". In other
// words it must always be a new MID (when either sending an M1 or an M2
// inside 'wsc_frame').
//
// 'destination_al_mac_address' is the neighbor AL MAC address where this query
// will be sent to.
// When 'wsc_frame' contains a message of type M1, this destination MAC must be
// the one of the node which previously sent the "AP-autoconfiguration
// response".
// When 'wsc_frame' contains a message of type M2, this destination MAC must be
// the one of the node which previously sent the "AP-autoconfiguration
// WSC" containing a message of type M1.
//
// 'wsc_frame' is a pointer to 'wsc_frame_size' bytes, containing either a
// message of type M1 or M2.
// The format of these messages is described in the "Wi-Fi simple configuration"
// document (TODO: actual name of the document)
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U send1905APAutoconfigurationWSCPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U **wsc_frame, INT16U *wsc_frame_size, INT8U wsc_nr, INT8U *radio_identifier, char *radio_name, struct TrafficSeparationPolicyTLV *traffic_separation_policy_tlv, INT8U vendor_tlv_nr, struct vendorSpecificTLV *vendor_tlvs);

// This function sends a "1905 generic phy query packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8"
//
// The format of this packet is detailed in "Section 6.3.14"
//
// 'destination_al_mac_address' is the neighbor AL MAC address where this query
// will be sent to.
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U send1905GenericPhyQueryPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address);

// This function sends a "1905 generic phy response packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8" (ie. it
// must be the same as the "messsage id" contained in the original "topology
// query" message)
//
// 'destination_al_mac_address' is the neighbor AL MAC address where this query
// will be sent to (ie. the AL MAC address of the node that sent the original
// "topology query" message)
//
// The format of this packet is detailed in "Section 6.3.15"
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U send1905GenericPhyResponsePacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address);

// This function sends a "1905 high layer query packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8"
//
// The format of this packet is detailed in "Section 6.3.16"
//
// 'destination_al_mac_address' is the neighbor AL MAC address where this query
// will be sent to.
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U send1905HighLayerQueryPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address);

// This function sends a "1905 high layer response packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8" (ie. it
// must be the same as the "messsage id" contained in the original "topology
// query" message)
//
// 'destination_al_mac_address' is the neighbor AL MAC address where this query
// will be sent to (ie. the AL MAC address of the node that sent the original
// "topology query" message)
//
// The format of this packet is detailed in "Section 6.3.17"
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U send1905HighLayerResponsePacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address);

// This function sends an "interface power change information packet" on the
// provided interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8"
//
// 'destination_al_mac_address' is the neighbor AL MAC address where this query
// will be sent to (ie. the AL MAC address of the node that sent the original
// "interface power change request" message)
//
// 'remote_interfaces' is a list of 'nr' remote mac addresses whose status is
// being requested to change.
//
// 'new_states' is a list of 'nr' states to be applied (each one to each of the
// remote interfaces specified in 'remote_interfaces').
// Each state can take any of the following values:
//   - POWER_STATE_REQUEST_OFF
//   - POWER_STATE_REQUEST_ON
//   - POWER_STATE_REQUEST_SAVE
//
// The format of this packet is detailed in "Section 6.3.18"
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U send1905InterfacePowerChangeRequestPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U (*remote_interfaces)[6], INT8U *new_states, INT8U nr);

// This function sends an "interface power change request packet" on the
// provided interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8" (ie. it
// must be the same as the "messsage id" contained in the original "topology
// query" message)
//
// 'destination_al_mac_address' is the neighbor AL MAC address where this query
// will be sent to (ie. the AL MAC address of the node that sent the original
// "interface power change request" message)
//
// 'local_interfaces' is a list of 'nr' local mac addresses whose state was
// recently tried to be changed (as a result of receiving an "interface power
// change information" message) and whose result code (from trying to apply
// that change) we are going to report back now
//
// 'results' is a list of 'nr' result codes (each one contains the resulting
// code from trying to apply a "power change" operation on the corresponding
// local interface from the 'local_interfaces' list)
// Each result code can take any of the following values:
//   -POWER_STATE_RESULT_COMPLETED
//   -POWER_STATE_RESULT_NO_CHANGE
//   -POWER_STATE_RESULT_ALTERNATIVE_CHANGE
//
// The format of this packet is detailed in "Section 6.3.19"
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U send1905InterfacePowerChangeResponsePacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U (*local_interfaces)[6], INT8U *results, INT8U nr);

////////////////////////////////////////////////////////////////////////////////
// Functions to send LLDP messages
////////////////////////////////////////////////////////////////////////////////

// This function sends a "LLDP bridge discovery packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
//
// The format of this packet is detailed in "Section 6.1"
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U sendLLDPBridgeDiscoveryPacket(char *interface_name);

////////////////////////////////////////////////////////////////////////////////
// Functions to send ALME reply messages
////////////////////////////////////////////////////////////////////////////////

// Compose and send an "ALME-GET-INTF-LIST.response" message
//
// 'alme_client_id' must be the same one used to receive the original request.
//

INT8U send1905InterfaceListResponseALME(INT8U alme_client_id);

// Compose and send an "ALME-GET-METRIC.response" message
//
// 'alme_client_id' must be the same one used to receive the original request.
//
// 'mac_address' is the neighbor AL MAC address at the other end of the link
// whose metrics are going to be reported.
// If 'mac_address' is NULL, all metrics between the current node and all its
// neighbors are reported instead.
//
INT8U send1905MetricsResponseALME(INT8U alme_client_id, INT8U *mac_address);

// Compose and send an "ALME-CUSTOM-COMMAND.response" message
//
// 'alme_client_id' must be the same one used to receive the original request.
//
// 'command' is the particular custom command whose response is going to be
// generated and sent back (ie. the 'command' contained in the original request)
// This 'command' can take any of the "CUSTOM_COMMAND_*" available values.
//
INT8U send1905CustomCommandResponseALME(INT8U alme_client_id, INT8U command);

INT8U sendMultiApCommandResponseALME(INT8U alme_client_id, struct multiApCommandRequestALME *p);

INT8U sendMultiAPPolicyConfigRequestPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, INT8U policy_inclusion, struct SteeringPolicyTLV *steer_policy, struct MetricReportingPolicyTLV *metric_policy, struct ChannelScanReportingPolicyTLV *scan_policy, struct Default8021QSettingsTLV *default8021q_config, struct TrafficSeparationPolicyTLV *traffic_separation_policy);

//
// set reason_code to 0 and sta_mac_address to NULL if no errer code TLV is needed
// mid should be set as the same MID that was received in the Multi-AP control message that this Ack message is acknowledging
//
INT8U send1905ACKPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, INT8U reason_code, INT8U sta_mac_address[]);

// This function sends a "Channel Preference Query packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8"
//
// 'dest_al_mac_address' is the neighbor AL MAC address where this query
// will be sent to.
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U sendMultiAPChannelPreferenceQueryPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address);

// This function sends a "Channel Preference Report packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8"
//
// 'dest_al_mac_address' is the neighbor AL MAC address where this query
// will be sent to.
//
// 'need_ack' is the boolean flag that whether the packet need a response packet
// because the unsolicited channel preference report packet need 1905 Ack response
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U sendMultiAPChannelPreferenceReportPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, INT8U *cac_tlv, bool need_ack);

// This function sends a "Channel Selection Request packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8"
//
// 'dest_al_mac_address' is the neighbor AL MAC address where this query
// will be sent to.
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U sendMultiAPChannelSelectionRequestPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, struct ChannelPreferenceTLV **channel_preference_tlv, INT8U channel_preference_tlv_nr, struct TransmitPowerLimitTLV **transmit_power_limit_tlv, INT8U                          transmit_power_limit_tlv_nr);

// This function sends a "Channel Selection Response packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8"
//
// 'dest_al_mac_address' is the neighbor AL MAC address where this query
// will be sent to.
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U sendMultiAPChannelSelectionResponsePacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, INT8U response);

// This function sends a "Operating Channel Report packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
// 'mid' is the "Message identifier" value we want this packet to contain. Its
// value must be calculated according to what is said on "Section 7.8"
//
// 'dest_al_mac_address' is the neighbor AL MAC address where this query
// will be sent to.
//
// Return "0" if a problem was found. "1" otherwise.
//
INT8U sendMultiAPOperatingChannelReportPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address);

INT8U sendMultiAPClientSteeringRequestPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, struct steering_request *steering_req);

INT8U sendMultiAPClientSteeringBTMReportPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, struct bss_transition_response_para *response);

INT8U sendMultiAPClientAssociationControlRequestPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, struct client_association_control_request *client_assoc_ctrl_req, INT8U client_assoc_ctrl_req_nr);

INT8U sendMultiAPSteeringCompletedPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address);

INT8U sendAPCapabilityQueryPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address);
INT8U sendAPCapabilityReportPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address);

INT8U sendClientCapabilityQueryPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U *client_bssid, INT8U *client_mac_address);
INT8U sendClientCapabilityReportPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U *client_bssid, INT8U *client_mac_address);
INT8U sendAPMetricQueryPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U bssid_nr, INT8U (*bssid)[6]);
INT8U sendAPMetricResponsePacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U bssid_nr, INT8U (*bssid)[6], INT8U radio_nr, INT8U (*radio_mac)[6]);
INT8U sendAssocSTALinkMetricQueryPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U *macaddr);
INT8U sendAssocSTALinkMetricResponsePacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U *macaddr);
INT8U sendUnassocSTALinkMetricQueryPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U op_class, INT8U channel_list_nr, struct ChannelListInfo *channel_list_entries);
INT8U sendUnassocSTALinkMetricResponsePacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U op_class, INT8U sta_nr, INT8U *buf);
INT8U sendBeaconMetricsQueryPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, struct BeaconMetricsQueryTLV *query);
INT8U sendBeaconMetricsResponsePacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U *buf, INT16U buf_len);
INT8U sendCombinedInfrastructureMetricMessage(char *interface_name, INT16U mid, INT8U *destination_al_mac_address);
INT8U sendBackhaulSteeringRequestPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U *backhaul_sta_mac, INT8U *target_bss_mac, INT8U op_class, INT8U channel_number);
INT8U sendBackhaulSteeringResponsePacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U *backhaul_sta_mac, INT8U *target_bss_mac, INT8U result_code);
INT8U send1905VendorSpecificMessage(char *interface_name, INT16U mid, INT8U *dest_mac_address, INT8U relay_indicator, struct vendorSpecificTLV *vendor_tlv);
#ifdef RTK_MULTI_AP_R2
void obtainTrafficSeparationPolicyTLV(struct TrafficSeparationPolicyTLV **tlv, struct configData *config[], INT8U config_nr);
INT8U sendChannelScanRequestPacket(char *interface_name, INT16U mid, struct ChannelScanRequestTLV *channel_scan_request_tlv, INT8U *destination_al_mac_address);
INT8U sendChannelScanReportPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, bool need_ack);
INT8U sendCACRequestPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, struct CACRequestTLV cac_request_tlv);
INT8U sendCACTerminationPacket(char *interface_name, INT16U mid, INT8U *destination_al_mac_address);
INT8U sendMultiAPAssociationStatusNotificationMessage(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, bool need_ack, INT8U bssid_nr, struct AssociationStatusNotificationBssInfo *associationStatusNotificationBssInfo);
INT8U sendMultiAPTunneledMessage(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, bool need_ack, INT8U *mac_address, INT8U payload_type, INT8U tunneled_tlv_nr, struct TunneledTLV *tunneledTlv);
INT8U sendFailedConnectionMessage(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, bool need_ack, INT8U *sta_mac, INT16U status_code, INT16U reason_code);
INT8U sendClientDisassociationStats(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, bool need_ack, INT8U *sta_mac, INT16U reason_code, struct AssociatedSTATrafficStatsTLV *traffic_stats_tlv);
INT8U sendErrorResponsePacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, INT8U p2_error_code_tlv_nr, struct Profile2ErrorCodeTLV *p2_error_code_tlvs);
#endif
#ifdef RTK_MULTI_AP_R3
void buildDPPChirpValueTLV(struct DPPChirpValueTLV *t, INT8U *enrollee_mac, INT8U *chirp_content, INT16U chirp_len);
INT8U send1905DirectEncapDPPPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, INT8U *dpp_frame, INT16U dpp_frame_len);
INT8U send1905EncapEAPOLMessage(char *interface_name, INT16U mid, INT8U *dest_mac_address, INT8U *eapol_frame, INT16U eapol_frame_len);
INT8U send1905DecryptionFailureMessage(char *interface_name, INT16U mid, INT8U *dest_mac_addr);
INT8U send1905ProxiedEncapDPPPacket(char *interface_name, INT16U mid, INT8U *dest_mac_address, struct Encap1905DPPTLV *encap_1905_dpp_tlv, struct DPPChirpValueTLV *chirp_value_tlv);
INT8U sendBSSConfigRequestPacket(char *interface_name, INT16U mid, INT8U *dest_mac_addr);
INT8U convert_map_config_to_dpp_config_obj(struct dpp_authentication *dpp_auth, struct configData *from, struct dpp_config_obj *to);
INT8U sendBSSConfigResponsePacket(char *interface_name, INT16U mid, INT8U *src_addr, INT8U *dest_mac_addr, struct APRadioBasicCapabilitiesTLV *ap_radio_tlvs[], INT8U ap_radio_tlvs_nr);
INT8U sendBSSConfigResultPacket(char *interface_name, INT16U mid, INT8U *dest_mac_addr);
INT8U sendServicePrioritizationRequestMessage(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, bool need_ack, struct ServicePrioritizationRuleTLV *service_prioritization_rule_tlv, struct DSCPMappingTableTLV *dscp_mapping_table_tlv);
INT8U sendDPPCCEIndicationMessage(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, INT8U is_advertise_cce);
void  triggerCCEAdvertisement(INT8U is_advertise_cce);
INT8U sendReconfigurationTriggerMessage(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, bool need_ack);
INT8U sendChirpNotificationMessage(char *interface_name, INT16U mid, INT8U *destination_al_mac_address, struct DPPChirpValueTLV *chirp_value_tlv);
#endif
#endif
