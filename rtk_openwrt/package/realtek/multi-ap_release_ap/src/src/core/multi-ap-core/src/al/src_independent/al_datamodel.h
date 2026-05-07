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

#ifndef _AL_DATAMODEL_H_
#define _AL_DATAMODEL_H_

#include <stdint.h>

#include "1905_tlvs.h"
#include "multi_ap_tlvs.h"
#include "multi_ap_tlvs_r2.h"
#include "multi_ap_tlvs_r3.h"
#include "multi_ap_tlvs_r4.h"

#include "al_dpp.h"
#include "al_dpp_eapol.h"

#define FLAG_CHANNEL_SCAN           0x00000001
#define FLAG_ON_BOOT_CHANNEL_SCAN   0x00000002
#define FLAG_APPLY_VLAN             0x00000004
#define FLAG_PREFER_BSSID_SET       0x00000008
#define FLAG_DELAYED_AUTOCONFIG     0x00000010
#define FLAG_ON_EAPOL               0x00000020
#define FLAG_WAIT_FOR_M1            0x00000040
#define FLAG_DELAYED_RECONFIG       0x00000080


////////////////////////////////////////////////////////////////////////////////
// Data model initialization and general functions
////////////////////////////////////////////////////////////////////////////////
enum CAC_STATUS {
	CAC_ON_GOING,
	CAC_COMPLETED,
	CAC_RADAR_DETECTED,
	CAC_FAILED
};

struct CACStatusRecord {
	INT8U op_class;
	INT8U channel;
	INT8U cac_status;
	INT32U last_timestamp;
};

struct RadioCACStatus {
	INT8U radio_identifier[6];
	INT8U cac_record_nr;
	struct CACStatusRecord *cac_records;
};

struct LocalInterface {
	char *name;
	INT8U mac_address[6];
	INT8U role;

	INT8U neighbors_nr;

	struct _neighbor {
		INT8U al_mac_address[6];
		INT8U remote_interfaces_nr;

		struct _remoteInterface {
			INT8U  mac_address[6];
			INT32U last_topology_discovery_ts;
			INT32U last_bridge_discovery_ts;

		} * remote_interfaces;

	} * neighbors;

	INT8U metric_inclusion_policy; // bit 7 = Associated STA Traffic Stats Inclusion Policy
	                               // bit 6 = Associated STA Link Metrics Inclusion Policy
	                               // bit 5 = Associated Wi-Fi 6 STA Status Report TLV Inclusion Policy
	                               // bits 4-0 = Reserved

	// INT16U vlan_id;
	// INT8U interface_type;
};

typedef struct _multiap_version {
	INT8U generation;
	INT8U major;
	char minor;
	INT8U update;
	INT8U stage;
	INT8U reserved_1;
	INT8U reserved_2;
	INT8U reserved_3;
} MultiapVersion;

enum SECURITY{
	SECURITY_NOT_ENABLED,
	SECURITY_ENABLED
};
struct networkDevice {
	INT32U update_timestamp;

	INT32U topology_timestamp;

	INT32U discovery_timestamp;

	INT8U multiap_profile;

	MultiapVersion multiap_version;

	char receiving_interface[15];

	struct deviceInformationTypeTLV *info;

	INT8U                                bridges_nr;
	struct deviceBridgingCapabilityTLV **bridges;

	INT8U                                 non1905_neighbors_nr;
	struct non1905NeighborDeviceListTLV **non1905_neighbors;

	INT8U                          x1905_neighbors_nr;
	struct neighborDeviceListTLV **x1905_neighbors;

	INT8U metrics_with_neighbors_nr;
	struct _metricsWithNeighbor {
		INT8U neighbor_al_mac_address[6];

		INT32U                           tx_metrics_timestamp;
		struct transmitterLinkMetricTLV *tx_metrics;

		INT32U                        rx_metrics_timestamp;
		struct receiverLinkMetricTLV *rx_metrics;

	} * metrics_with_neighbors;

	INT8U                ap_metrics_nr;
	struct APMetricsTLV *ap_metrics;

	struct ApOperationalBSSTLV * operational_bss;
	struct SupportedServiceTLV * supported_services;
	struct AssociatedClientsTLV *associated_clients;

	INT8U                      extensions_nr;
	struct vendorSpecificTLV **extensions;

	INT8U                         last_ch_prefs_nr;
	struct ChannelPreferenceTLV **last_ch_prefs;

	struct ChannelScanCapabilitiesTLV *channel_scan_capabilities;

	INT8U byte_counter_units; // 0 : bytes
	                          // 1 : kibibytes (KiB) = 1,024 bytes
	                          // 2 : mebibytes (MiB) = 1,048,576 bytes
	                          // 3 : reserved

	INT64U integrity_transmission_counter;

	INT8U security;

};

struct dpp_proxy {
	struct map_list list;

	INT8U  chirp[SHA256_MAC_LEN];
	INT8U *proxy_auth_req_msg;
	INT32U proxy_auth_req_msg_len;

	INT8U  reconfig_announcement[SHA256_MAC_LEN];
	INT8U *proxy_reconfig_auth_req_msg;
	INT32U proxy_reconfig_auth_req_msg_len;
};

// This function must be called before any other function of this header file
//
void DMinit();

struct LocalInterface *DMnameToLocalInterfaceStruct(char *name);

// When the AL entity is initialized, it knows its AL MAC address. At this point
// the "DMalMacSet()" function must be called to store this value in the
// database.
// Later, anyone can consult this value with "DMalMacGet()"
//
void   DMalMacSet(INT8U *al_mac_address);
INT8U *DMalMacGet();

INT8U DMisTribandDevice();

// The device will be set to controller if one of its interface is set to registrar
INT8U DMisController();

void DMcontrollerInit(INT8U *controller_mac_address);

void   DMcontrollerMacSet(INT8U *controller_mac_address);
INT8U *DMcontrollerMacGet();

// When the AL entity is initialized, it knows whether the user want to map the
// whole network or only direct neighbors (using much less memory).
// the "DMalMacSet()" function must be called to store this value in the
// database.
// Later, anyone can consult this value with "DMmapWholeNetworkGet()"
//
void  DMmapWholeNetworkSet(INT8U map_whole_network_flag);
INT8U DMmapWholeNetworkGet();

// When a new local interface is made available to the AL entity, this function
// must be called to update the database.
// Returns '0' if there was a problem (out of memory, etc...), '1' otherwise
// (including if the interface had already been inserted)
//
INT8U DMinsertInterface(char *name, INT8U *mac_address, INT8U role);

// These are used to convert between names (ex: "eth0", "wlan1",...) and MAC
// addresses of local interfaces which have previously been inserted in the
// database using "DMinsertInterface()"
//
// Returned values must not be freed.
//
char * DMmacToInterfaceName(INT8U *mac_address);
INT8U *DMinterfaceNameToMac(char *interface_name);

// Returns a list of 6 bytes arrays with the AL MACs of all neighbors (on the
// provided interface) from where a "topology discovery" message has been
// received.
//
// The returned pointer, once it is no longer needed, must be freed by the
// caller with "PLATFORM_FREE()"
//
INT8U (*DMgetListOfInterfaceNeighbors(char *local_interface_name, INT8U *al_mac_addresses_nr))[6];

// Returns a list of 6 bytes arrays with the AL MACs of all neighbors (from
// *all* interfaces) from where a "topology discovery" message has been
// received.
//
// Each neighbor is reported at most once, even if it is reachable from
// different interfaces. Thus the returned list does not contain repeated
// elements.
//
// The returned pointer, once it is no longer needed, must be freed by the
// caller with "PLATFORM_FREE()"
//
INT8U (*DMgetListOfNeighbors(INT8U *al_mac_addresses_nr))[6];

// A given neighbor might be "reachable" in several ways:
//   - Just from one local interface (typical case)
//   - From one local interface *but* in two remote interfaces (ex: the remote
//     interface is connected to a HUB where two remote interfaces from one same
//     neighbor are connected)
//   - From several interfaces, each of them connected with one or more remote
//     interfaces.
//
// This function returns all the ways a neighbor is reachable:
//   - The output argument 'links_nr' contains the number of ways the neighbor
//     can be reached.
//   - The output argument 'interfaces' is an array of 'links_nr' pointers to
//     strings, each of them containing the name of a local interface (ex:
//     "eth0")
//   - The returned pointer contains 'links_nr' MAC addresses that correspond to
//     interfaces in the provided neighbour.
//
//  So, for example, if we have this:
//
//      eth0 -------------- eth0
//    A                          B
//      eth1 ---- HUB ----- eth1
//                 |
//                 |
//                 --------- eth0
//                                C
//                           eth1
//
//  ...and we are "A", then the following calls will return the following
//  results:
//
//    - DMgetListOfLinkswithNeighbor(B):
//        ret        = [<B_eth0_addr>, <B_eth1_addr>]
//        interfaces = ["eth0",        "eth1"]
//        links_nr   = 2
//
//    - DMgetListOfLinkswithNeighbor(C):
//        ret        = [<C_eth0_addr>]
//        interfaces = ["eth1"]
//        links_nr   = 1
//
//
// The returned pointers, once they are no longer needed, must be freed by the
// caller with "DMfreeListOfLinksWithNeighbor()". Example:
//
//   INT8U (*ret)[6];
//   char **interfaces;
//   INT8U links_nr;
//
//   ret = DMgetListOfLinksWithNeighbor(neighbor_al_mac_address, &interfaces, &links_nr);
//
//   <use it...>
//
//   DMfreeListOfLinksWithNeighbor(ret, interfaces, links_nr);
//
// If there is a problem, this function returns NULL and nothing needs to be
// freed by the caller
//
INT8U (*DMgetListOfLinksWithNeighbor(INT8U *neighbor_al_mac_address, char ***interfaces, INT8U *links_nr))
[6];

// Use this to free the two pointers returned by
// "DMgetListOfInterfaceNeighbors()" (ie. the "interfaces" pointer and the
// returned value pointer)
//
void DMfreeListOfLinksWithNeighbor(INT8U (*p)[6], char **interfaces, INT8U links_nr);

////////////////////////////////////////////////////////////////////////////////
// (Local / interface level) topology discovery related functions
////////////////////////////////////////////////////////////////////////////////

// Call this function when a new "discovery message" has been received on
// 'receiving_interface_addr' whose payload containing 'al_mac_address' and
// 'mac_address'.
// This function will then update the timestamps of that particular link so
// that they contain the current time.
//
// 'timestamp_type' can be either TIMESTAMP_TOPOLOGY_DISCOVERY (to be used when
// the received "discovery message" is a "1905 topology discovery message") or
// TIMESTAMP_BRIDGE_DISCOVERY (when receiving a "LLDP bridge discovery message")
//
// Return '0' if there was a problem, '1' if this is the first time updating
// the timestamp of neighbor 'al_mac_address' and '2' otherwise
//
// When the return value is '2', output variable 'ellapsed' contains the amount
// of milliseconds since the last update and this one (NOTE: if you are not
// interested in this value, use "ellapsed = NULL" when calling this function)
//
#define TIMESTAMP_TOPOLOGY_DISCOVERY 0
#define TIMESTAMP_BRIDGE_DISCOVERY 1
INT8U DMupdateDiscoveryTimeStamps(char *receiving_interface_name, INT8U *al_mac_address, INT8U *mac_address, INT8U timestamp_type, INT32U *ellapsed);

// These functions returns "1" or "0" according to the "bridge flag" rules
// detailed in "IEEE Std 1905.1-2013 Section 8.1"
//
// A link is bridged when "1905 topology discovery" and "LLDP bridge discovery"
// messages are received from the end point of that link in less than
// DISCOVERY_THRESHOLD_MS apart.
//
// A neighbor is bridged when at least one of the links between the local AL
// entity and that neighbor is bridged.
//
// An interface is bridged when at least one of its neighbors is bridged.
//
#define DISCOVERY_THRESHOLD_MS (120000) // 120 seconds
// #define DISCOVERY_THRESHOLD_MS (12000) // 12 seconds
INT8U DMisLinkBridged(char *local_interface_name, INT8U *neighbor_al_mac_address, INT8U *neighbor_mac_address);
INT8U DMisNeighborBridged(char *local_interface_name, INT8U *neighbor_al_mac_address);
INT8U DMisInterfaceBridged(char *local_interface_name);

INT8U DMisExistingNeighbor(char *local_interface_name, INT8U *neighbor_al_mac_address);

// Given the MAC of an interface (could be local or not) returns the AL MAC of
// the AL entity which owns that interface.
// All AL 1905 neighbors are considered (ie. first level neighbors, second level
// neighbors, ...)
//
// If the provided MAC is an AL MAC address itself (and it is either the local
// AL MAC address or a neighbor AL MAC address), then its value is returned.
//
// Returns NULL if no AL entity owning an interface with that MAC address was
// found.
//
// The returned pointer becomes the caller's responsability and must be freed
// with "PLATFORM_FREE()" once it is no longer needed.
//
INT8U *DMmacToAlMac(INT8U *mac_addresses);

////////////////////////////////////////////////////////////////////////////////
// (Global) network topology related functions
////////////////////////////////////////////////////////////////////////////////

INT8U DMaddNetworkDevice(INT8U *al_mac_address, char *receiving_interface);

// Update a "device" entry in the data model.
//
// Call this function every time new information regarding one network device
// is received (ie.  when receiving either a "topology response", a "generic
// phy response" or a "high layer response" message).
//
// As you can see, this function takes many arguments. You don't need to provide
// all of them at the same time (just leave the ones you don't want to update
// set to NULL and their associated "*_update" value set to "0").
// For example, when receiving a "generic phy response" message you will only
// want to update the "genericPhyDeviceInformationTypeTLV" structure, thus you
// would call this function like this:
//
//   DMupdateNetworkDeviceInfo(al_mac_address,
//                             0, NULL,
//                             0, NULL, 0,
//                             0, NULL, 0,
//                             0, NULL, 0,
//                             0, NULL, 0,
//                             0, NULL, 0,
//                             1, generic_phy,
//                             0, NULL,
//                             0, NULL,
//                             0, NULL, 0);
//
// The "*_update" argument is needed because sometimes you want to update
// something to be NULL (for example, when something disappears from the
// network)
//
// Note that for certain types of TLVs the data model can only contain *one*
// of them ("deviceInformationTypeTLV", "genericPhyDeviceInformationTypeTLV",
// "x1905ProfileVersionTLV", "deviceInformationTypeTLV" and
// "deviceInformationTypeTLV").
// For these TLVs you only need to provide a pointer to the structure.
//
// For all the others the data model might contain more than one element. That's
// why you need to provide a pointer to a list of elements and its length.
//
//   NOTE: The 1905 standard allows for more than one "powerOffInterfaceTLV"
//   and "l2NeighborDeviceTLV" TLVs to be received in the "topology response"
//   message, and that's why I require a pointer to a list (and its length) for
//   them. HOWEVER, this is probably an error in the standard (particularly in
//   "Section 6.3.3" after the corrections included in the 1a revision) because
//   one single of these is more than enough as it can contain as many
//   "subentries" (for the particular items they list) as desired.
//   If this is corrected in future revisions of the standard, I just need to
//   change the prototype of this function to accept regular pointers for
//   "power_off" and "l2_neighbors" (instead of pointers to pointers) and remove
//   their associated "power_off_nr" and "l2_neighbors_nr" variables.
//
// The only parameter you *must* provide is the first one, which identifies the
// 1905 node that is going to be udpated (or created, if this is the first time
// this node is being used in a call to this function).
//
// The pointers this function receives become its responsability, thus the
// caller must not free them at any point (they will automatically be freed the
// next time this function is called with new (updated) data)
//
//   NOTE: For metrics, a different function is used
//        ("DMupdateNetworkDeviceMetrics()"). The reason for this is that
//        "metrics" work in a slighlty different way: they are not overwritten
//        as a whole (as all the TLVs in this function are) every time they are
//        updated... instead only the "matching" (ie. same origin and
//        destination) metric is.
//        In other words:
//          - When using THIS function, every time you provide a new pointer to
//            a TLV, the data model is updated to contain that TLV (and free the
//            old one)
//          - When using "DMupdateNetworkDeviceMetrics()", every time you
//            provide a new pointer, the data model is updated to *add* it to
//            the already existing metrics information (unless it refers to an
//            already existing link, in which case it is updated)
//
//  TODO: Would it be worth to merge these functions in the future?
//
// Return '0' if there was a problem, '1' otherwise
//
INT8U DMupdateNetworkDeviceInfo(INT8U *al_mac_address, char *receiving_interface,
                                INT8U in_update,  struct deviceInformationTypeTLV             *info,
                                INT8U br_update,  struct deviceBridgingCapabilityTLV         **bridges,           INT8U bridges_nr,
                                INT8U no_update,  struct non1905NeighborDeviceListTLV        **non1905_neighbors, INT8U non1905_neighbors_nr,
                                INT8U x1_update,  struct neighborDeviceListTLV               **x1905_neighbors,   INT8U x1905_neighbors_nr,
                                INT8U ss_update,  struct SupportedServiceTLV *supported_services,
								INT8U op_update,  struct ApOperationalBSSTLV *operational_bss,
								INT8U as_update,  struct AssociatedClientsTLV *associated_clients);

// Given the AL MAC address of a node, returns "0" if the last time its "device
// info" was updated (ie. the last time someone called
// "DMupdateNetworkDeviceInfo()" on that node) was quite recently, indicating
// the caller should not initiate the process to re-fresh this information.
//
// Returns "1" otherwise.
//
// NOTE: "recently" means no longer than MAX_AGE seconds ago
//
#define MAX_AGE 50 // Must be smaller than the "TIMER_TOKEN_DISCOVERY" period (which is 60 seconds)
INT8U DMnetworkDeviceInfoNeedsUpdate(INT8U *al_mac_address);

// Update the "metrics" information of a neighbor node
//
// 'metrics' is a pointer to either a "struct transmitterLinkMetricTLV" or a
// "struct receiverLinkMetricTLV".
//
// Because 'metrics' contains all the needed information (including who are the
// two 1905 nodes the metrics information make reference to) no additional
// parameters are needed.
//
// The pointer this function receives become its responsability, thus the caller
// must not free them at any point.
// They will automatically be freed the next time this function is called with
// new (updated) data *that matches* the same metric (otherwise, a new "metrics"
// entry will be created).
//
// Return '0' if there was a problem, '1' otherwise
//
INT8U DMupdateNetworkDeviceMetrics(INT8U *metrics);

INT8U DMupdateNetworkDeviceAPMetric(INT8U *mac, INT8U *metrics);

INT8U DMupdateNetworkDeviceVersion(INT8U *al_mac, char *device_version);

uint64_t DMgetNetworkDeviceVersion(INT8U *al_mac);

// Print the contents of the "devices" database using the provided printf-like
// function.
//
void DMdumpNetworkDevices(void (*write_function)(const char *fmt, ...));

// This function must be called from time to time (every "x" seconds, where "x"
// should be a number slightly greate than "GC_MAX_AGE") to remove device
// entries from the database.
//
// If an entry is older than "GC_MAX_AGE" seconds, this function removes it.
//
// "GC_MAX_AGE" must be higher than 60 seconds, which is the network rediscovery
// period defined in the IEEE1905 standard.
//
// The return value is the number of entries deleted from the database (that
// means it will return "0" if no entry eas removed)
//
#define GC_MAX_AGE (90)
INT8U DMrunGarbageCollector(INT8U *is_controller_lost);

// Remove a neighbor from a particular local interface.
//
// 'al_mac_address' is the 1905 neighbour MAC address that you want to remove.
//
// 'interface_name' is the name of the local interface where you want to remove
// this neighbor from.
//
// Nodes are usually removed by calling "DMrunGarbageCollector()". Use this
// function to speed up the process when, somehow, you are sure that a specific
// AL node is no longer visible from a particular interface.
// This might happen, for example, when a L2-specific mechanism triggers a
// callback when a neighbour dissapears.
// In these cases you have to first call "DMremoveALNeighborFromInterface()"
// followed by "DMrunGarbageCollector()".
//
// Remember: you don't need to call this function if you don't want to and are
// ok with the ~60 seconds period of the garbage collector mechanism.
//
void DMremoveALNeighborFromInterface(INT8U *al_mac_address, char *interface_name);

// Get TLV extensions from a particular device
//
// The datamodel provides a list of TLV extensions per device (including
// itself).  Actually, the datamodel simply provides a pointer to an array of
// Vendor Specific TLVs. This pointer is really managed by third-party entities
// (like BBF), adding/removing TLVs.
// This function is used to obtain the TLV extensions pointer for a particular
// device.
//
// 'al_mac_address' is the mac address of the requested 1905 device
//
// 'nr' is the number of TLVs belonging to this 'al_mac _adress'
//
// Return a pointer to the datamodel extensions pointer. This will allow
// third-party extenders to create/resize the TLV list
//
struct vendorSpecificTLV ***DMextensionsGet(INT8U *al_mac_address, INT8U **nr);

// Get device TLVs received from 1905 Topology Respone with al mac address
// Read-only struct, shall only update with with DMupdateNetworkDeviceInfo
// NOTE: Do not free this pointer
struct networkDevice *DMnetworkDeviceGet(INT8U *al_mac_address);

INT8U *DMallNetworkDevicesGet(struct networkDevice **network_devices, INT8U *network_devices_nr);

INT8U DMupdateControllerChannelPreference(struct ChannelPreferenceTLV *report);

void DMgetControllerChannelPreference(struct ChannelPreferenceTLV **preferences, INT8U *preference_nr);

void DMswitchToBestChannel();

INT8U DMchangeTransmitPower(INT8U *report);

INT8U DMupdateSteeringPolicy(INT8U *report);

INT8U DMcheckBTMSteeringDisallow(INT8U *sta_mac);

struct networkDevice *DMlocalNetworkDeviceGet();

void DMclientMacToAssociatedRadioMac(INT8U *client_mac, INT8U *radio_mac);

void DMsetIntervalTime(INT8U interval);

INT8U DMintervalActionRequired();

void DMchannelQueryStatusSet(INT8U status);

INT8U DMchannelQueryStatusGet();

INT8U DMgetConfiguredState();

void DMsetConfiguredState(INT8U state);

void DMsetDeviceName(char* device_name);

char* DMgetDeviceName();

void DMsetSTPState(INT8U stp_state);

INT8U DMgetSTPState();

void DMsetControllerIP(char* ip);

char* DMgetControllerIP();

INT8U DMisListeningLoopback();

void DMsetListeningLoopback(INT8U state);

INT8U DMgetWFATestMode();

void DMsetWFATestMode(INT8U wfa_test_mode);

INT64U DMgetNextITC();

INT64U DMgetCurrentITC();

void DMresetITC();

#ifdef RTK_MULTI_AP_R2

INT8U DMsetChannelScanResult(INT8U channel_scan_result_nr, struct ChannelScanResultTLV *channel_scan_results);

INT8U DMgetChannelScanResult(INT8U *channel_scan_result_nr, struct ChannelScanResultTLV **channel_scan_results);

INT8U DMgetReportIndChannelScanFlag();

void DMsetReportIndChannelScanFlag(INT8U flag);

INT8U DMsetChannelScanCapabilities(struct ChannelScanCapabilitiesTLV* t, INT8U* al_mac);

struct ChannelScanCapabilitiesTLV* DMgetChannelScanCapabilities(INT8U* al_mac);

INT8U DMbufferChannelScanRequest(struct ChannelScanRequestTLV* t);

struct ChannelScanRequestTLV* DMgetBufferedChannelScanRequest();

INT8U DMsetChannelScanCompleted(INT8U band);

INT8U DMclearChannelScanCompleted(INT8U band);

INT8U DMgetChannelScanCompleted(INT8U band);

INT8U *DMgetChannelScanResultBuffer();

void DMappendChannelScanResultBuffer(int buffer_size, INT8U *content);

void DMclearChannelScanResultBuffer();

INT8U DMgetRequestStoredResultsFlag();

INT8U DMsetRequestStoredResultsFlag(INT8U flag);

INT8U DMsetOnBootScanFlag(INT8U flag);

INT8U DMgetOnBootScanFlag();

void DMsetDefault8021QSettings(INT16U primary_vlan_id, INT8U default_pcp);

void DMgetDefault8021QSettings(INT16U *primary_vlan_id, INT8U *default_pcp);

void DMsetTrafficSeparationPolicy(struct TrafficSeparationPolicyTLV *tlv);

INT16U DMgetVIDbySSID(char* ssid);

INT8U DMupdateCACRecord(INT8U *radio_identifier, INT8U op_class, INT8U channel, INT8U status);

void DMgetRadioCACStatus(INT8U *radio_identifier, struct RadioCACStatus* radio_cac);

void DMgetAllRadioCACStatus(struct RadioCACStatus **cac_status);

void DMdumpCACStatus();

#endif

void DMsetMultiAPProfile(INT8U profile);

INT8U DMgetMultiAPProfile();

void DMsetTargetDeviceMultiAPProfile(INT8U *al_mac, INT8U multiap_profile);

INT8U DMgetTargetDeviceMultiAPProfile(INT8U *al_mac);

void DMsetEventFlag(INT32U flag);

void DMresetEventFlag(INT32U flag);

INT8U DMisEventFlagSet(INT32U flag);

void DMsetNeighborRemovalThreshold(INT16U threshold);

INT8U DMaddNewListeningInterface(char *interface_name);

INT8U DMupdateAutoconfigVendorTLV(INT8U tlv_nr, struct vendorSpecificTLV *vendor_tlvs, INT8U radio_band);

struct vendorSpecificTLV *DMgetAutoconfigVendorTLV(INT8U *tlv_nr, INT8U radio_band);

void DMsetMultiAPCommonNetlink(INT8U common_nl);

INT8U DMgetMultiAPCommonNetlink();

void DMsetExcludedInterfaces(char *excluded_interfaces);

INT8U DMisInterfaceExcluded(char *interface_name);

void DMsetBackhaulSteeringTarget(INT8U *target_mac_address);

void *DMgetBackhaulSteeringTarget();

INT8U DMgetRCPISteeringThreshold(INT8U *radio_mac, INT8U *rcpi_steering_threshold);

INT8U *DMgetRootInterfaceMAC(char *interface_name);

void DMsetAutoconfigRenewBand(INT8U renew_band);

INT8U DMgetAutoconfigRenewBand();

void DMsetByteCounterUnits(INT8U unit);

INT8U DMgetByteCounterUnits();

INT8U DMgetTargetDeviceByteCounterUnits(INT8U *al_mac);

void DMsetReportUnsuccessfulAssociation(INT8U report_unsuccessful_association);

INT8U DMgetReportUnsuccessfulAssociation();

INT8U DMgetMaxBssPerRadio();

void DMsetMaxBssPerRadio(INT8U max_bss_per_radio);

void DMdppInit();

struct dpp_bootstrap_info *DMgetOwnDPPBootstrapInfo();

struct dpp_authentication *DMaddDPPAuth(INT8U *chirp, INT8U chirp_len);

struct dpp_authentication *DMgetDPPAuthenticationObject(void);

struct dpp_authentication *DMgetDPPAuthenticationObjectByChirp(INT8U *hash_value, INT8U hash_len);

struct dpp_proxy *DMgetChirpProxy(INT8U *chirp, INT8U chirp_len);

struct dpp_proxy *DMaddDPPChirpProxy(INT8U *chirp, INT8U chirp_len);

void DMgetDPPEAPOLStateMachine(struct dpp_eapol_sm **sm);

void DMsetMaxServicePrioritizationRules(INT8U max_nr_rules);

INT8U DMgetMaxServicePrioritizationRules();

INT8U DMsetServicePrioritizationRule(struct ServicePrioritizationRuleTLV *tlv);

struct ServicePrioritizationRuleTLV * findDominantServicePrioritizationRule();

char **DMgetRadioNames(int *radio_number);

void DMsetDPPEnrollee(INT8U *enrollee_mac_address);

void *DMgetDPPEnrollee();

void DMsetSecurity(INT8U *al_mac, INT8U security);

INT8U DMgetSecurity(INT8U *al_mac);

void DMsetM1Delay(INT8U delay);

INT8U DMgetM1Delay();

void DMsetDPPCreds(char *csign_key, char *netaccess_key, char *pp_key, char *signed_connector_1905);

INT8U *DMgetDPPCsignFromFile(INT16U *csign_len);

INT8U *DMgetDPPPpkFromFile(INT16U *ppk_len);
#endif
