/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _MAP_TLVS_H_
#define _MAP_TLVS_H_
#include <stdint.h>

#define TLV_TYPE_END_OF_MESSAGE                         (0)
#define TLV_TYPE_AL_MAC_ADDRESS_TYPE                    (1)
#define TLV_TYPE_MAC_ADDRESS_TYPE                       (2)
#define TLV_TYPE_DEVICE_INFORMATION_TYPE                (3)
#define TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES           (4)
#define TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST          (6)
#define TLV_TYPE_NEIGHBOR_DEVICE_LIST                   (7)
#define TLV_TYPE_LINK_METRIC_QUERY                      (8)
#define TLV_TYPE_TRANSMITTER_LINK_METRIC                (9)
#define TLV_TYPE_RECEIVER_LINK_METRIC                   (10)
#define TLV_TYPE_VENDOR_SPECIFIC                        (11)
#define TLV_TYPE_LINK_METRIC_RESULT_CODE                (12)
#define TLV_TYPE_SEARCHED_ROLE                          (13)
#define TLV_TYPE_AUTOCONFIG_FREQ_BAND                   (14)
#define TLV_TYPE_SUPPORTED_ROLE                         (15)
#define TLV_TYPE_SUPPORTED_FREQ_BAND                    (16)
#define TLV_TYPE_WSC                                    (17)
#define TLV_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION         (18)
#define TLV_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION          (19)
#define TLV_TYPE_SUPPORTED_SERVICE                                 (128) // 0x80
#define TLV_TYPE_SEARCHED_SERVICE                                  (129) // 0x81
#define TLV_TYPE_AP_RADIO_IDENTIFIER                               (130) // 0x82
#define TLV_TYPE_AP_OPERATIONAL_BSS                                (131) // 0x83
#define TLV_TYPE_ASSOCIATED_CLIENTS                                (132) // 0x84
#define TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES                       (133) // 0x85
#define TLV_TYPE_AP_HT_CAPABILITIES                                (134) // 0x86
#define TLV_TYPE_AP_VHT_CAPABILITIES                               (135) // 0x87
#define TLV_TYPE_AP_HE_CAPABILITIES                                (136) // 0x88
#define TLV_TYPE_STEERING_POLICY                                   (137) // 0x89
#define TLV_TYPE_METRIC_REPORT_POLICY                              (138) // 0x8A
#define TLV_TYPE_CHANNEL_PREFERENCE                                (139) // 0x8B
#define TLV_TYPE_RADIO_OPERATION_RESTRICTION                       (140) // 0x8C
#define TLV_TYPE_TRANSMIT_POWER_LIMIT                              (141) // 0x8D
#define TLV_TYPE_CHANNEL_SELECTION_RESPONSE                        (142) // 0x8E
#define TLV_TYPE_OPERATING_CHANNEL_REPORT                          (143) // 0x8F
#define TLV_TYPE_CLIENT_INFO                                       (144) // 0x90
#define TLV_TYPE_CLIENT_CAPABLITY_REPORT                           (145) // 0x91
#define TLV_TYPE_CLIENT_ASSOCIATION_EVENT                          (146) // 0x92
#define TLV_TYPE_AP_METRIC_QUERY                                   (147) // 0x93
#define TLV_TYPE_AP_METRICS                                        (148) // 0x94
#define TLV_TYPE_STA_MAC_ADDRESS_TYPE                              (149) // 0x95
#define TLV_TYPE_ASSOCIATED_STA_LINK_METRICS                       (150) // 0x96
#define TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY               (151) // 0x97
#define TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE            (152) // 0x98
#define TLV_TYPE_BEACON_METRICS_QUERY                              (153) // 0x99
#define TLV_TYPE_BEACON_METRICS_RESPONSE                           (154) // 0x9A
#define TLV_TYPE_STEERING_REQUEST                                  (155) // 0x9B
#define TLV_TYPE_STEERING_BTM_REPORT                               (156) // 0x9C
#define TLV_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST                (157) // 0x9D
#define TLV_TYPE_BACKHAUL_STEERING_REQUEST                         (158) // 0x9E
#define TLV_TYPE_BACKHAUL_STEERING_RESPONSE                        (159) // 0x9F
#define TLV_TYPE_HIGHER_LAYER_DATA                                 (160) // 0xA0
#define TLV_TYPE_AP_CAPABILITY                                     (161) // 0xA1
#define TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS                      (162) // 0xA2
#define TLV_TYPE_ERROR_CODE                                        (163) // 0xA3
#define TLV_TYPE_CHANNEL_SCAN_REPORTING_POLICY                     (164) // 0xA4
#define TLV_TYPE_CHANNEL_SCAN_CAPABILITIES                         (165) // 0xA5
#define TLV_TYPE_CHANNEL_SCAN_REQUEST                              (166) // 0xA6
#define TLV_TYPE_CHANNEL_SCAN_RESULT                               (167) // 0xA7
#define TLV_TYPE_TIMESTAMP                                         (168) // 0xA8
#define TLV_TYPE_1905_LAYER_SECURITY_CAPABILITY                    (169) // 0xA9
#define TLV_TYPE_GROUP_INTEGRITY_KEY                               (170) // 0xAA
#define TLV_TYPE_MIC                                               (171) // 0xAB
#define TLV_TYPE_ENCRYPTED                                         (172) // 0xAC
#define TLV_TYPE_CAC_REQUEST                                       (173) // 0xAD
#define TLV_TYPE_CAC_TERMINATION                                   (174) // 0xAE
#define TLV_TYPE_CAC_COMPLETION_REPORT                             (175) // 0xAF
#define TLV_TYPE_CAC_STATUS_REQUEST                                (176) // 0xB0
#define TLV_TYPE_CAC_STATUS_REPORT                                 (177) // 0xB1
#define TLV_TYPE_CAC_CAPABILITIES                                  (178) // 0xB2
#define TLV_TYPE_MULTIAP_PROFILE                                   (179) // 0xB3
#define TLV_TYPE_PROFILE_2_CAPABILITY                              (180) // 0xB4
#define TLV_TYPE_DEFAULT_802_1Q_SETTINGS                           (181) // 0xB5
#define TLV_TYPE_TRAFFIC_SEPARATION_POLICY                         (182) // 0xB6
#define TLV_TYPE_PACKET_FILTERING_POLICY                           (183) // 0xB7
#define TLV_TYPE_ETHERNET_CONFIGURATION_POLICY                     (184) // 0xB8
#define TLV_TYPE_SERVICE_PRIORITIZATION_RULE                       (185) // 0xB9
#define TLV_TYPE_DSCP_MAPPING_TABLE                                (186) // 0xBA
#define TLV_TYPE_SERVICE_PRIORITIZATION_INTERFACE_EXCEPTION        (187) // 0xBB
#define TLV_TYPE_PROFILE_2_ERROR_CODE                              (188) // 0xBC
#define TLV_TYPE_AP_OPERATIONAL_BACKHAUL_BSS                       (189) // 0xBD
#define TLV_TYPE_AP_RADIO_ADVANCED_CAPABILITIES                    (190) // 0xBE
#define TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION                   (191) // 0xBF
#define TLV_TYPE_SOURCE_INFO                                       (192) // 0xC0
#define TLV_TYPE_TUNNELED_MESSAGE_TYPE                             (193) // 0xC1
#define TLV_TYPE_TUNNELED                                          (194) // 0xC2
#define TLV_TYPE_PROFILE_2_STEERING_REQUEST                        (195) // 0xC3
#define TLV_TYPE_UNSUCCESSFUL_ASSOCIATION_POILCY                   (196) // 0xC4
#define TLV_TYPE_METRIC_COLLECTION_INTERVAL                        (197) // 0xC5
#define TLV_TYPE_RADIO_METRICS                                     (198) // 0xC6
#define TLV_TYPE_AP_EXTENDED_METRICS                               (199) // 0xC7
#define TLV_TYPE_ASSOCIATED_STA_EXTENDED_LINK_METRICS              (200) // 0xC8
#define TLV_TYPE_STATUS_CODE                                       (201) // 0xC9
#define TLV_TYPE_REASON_CODE                                       (202) // 0xCA
#define TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES                   (203) // 0xCB
#define TLV_TYPE_AKM_SUITE_CAPABILITIES                            (204) // 0xCC
#define TLV_TYPE_1905_ENCAP_DPP                                    (205) // 0xCD
#define TLV_TYPE_1905_ENCAP_EAPOL                                  (206) // 0xCE
#define TLV_TYPE_DPP_BOOTSTRAPPING_URI_NOTIFICATION                (207) // 0xCF
#define TLV_TYPE_BACKHAUL_BSS_CONFIGURATION                        (208) // 0xD0
#define TLV_TYPE_BSSID_TO_UNIQUE_BSS_INDEX_MAPPING                 (209) // 0xD1


#define MULTI_AP_CONTROLLER_SERVICE                     (0x00)
#define MULTI_AP_AGENT_SERVICE                          (0x01)


#define MCAST_1905_B0 (0x01)
#define MCAST_1905_B1 (0x80)
#define MCAST_1905_B2 (0xC2)
#define MCAST_1905_B3 (0x00)
#define MCAST_1905_B4 (0x00)
#define MCAST_1905_B5 (0x13)
#define MCAST_1905  {MCAST_1905_B0, MCAST_1905_B1, MCAST_1905_B2, MCAST_1905_B3, MCAST_1905_B4, MCAST_1905_B5}

#define MULTI_AP_PROFILE_1                    0x01
#define MULTI_AP_PROFILE_2                    0x02
#define MULTI_AP_PROFILE_3                    0x03
#define MULTI_AP_PROFILE_4                    0x04

////////////////////////////////////////////////////////////////////////////////
// AL MAC address type TLV associated structures ("1905 Section 6.4.3")
////////////////////////////////////////////////////////////////////////////////
struct alMacAddressTypeTLV
{
    uint8_t   tlv_type;             // Must always be set to
                                  // TLV_TYPE_AL_MAC_ADDRESS_TYPE

    uint8_t   al_mac_address[6];    // 1905 AL MAC address of the transmitting
                                  // device
};

////////////////////////////////////////////////////////////////////////////////////////
// Client Association Event TLV associated structures (Multi-AP "Section 17.2.20")
////////////////////////////////////////////////////////////////////////////////////////

#define CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_LEAVE 0x00
#define CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_JOIN 0x80
#define BTM_RESPONSE_EVENT 0x40

#define MASK_CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT   (0x80)
struct ClientAssociationEventTLV
{
    uint8_t   tlv_type;               // Must always be set to
                                    // TLV_TYPE_CLIENT_ASSOCIATION_EVENT
    uint8_t   mac_address[6];         // The MAC address of the client.
    uint8_t   bssid[6];               // The BSSID of the BSS operated by the Multi-AP Agent for which the event
                                    // has occurred.
    uint8_t   event;

};

////////////////////////////////////////////////////////////////////////////////
// Device information type TLV associated structures ("1905 Section 6.4.5")
////////////////////////////////////////////////////////////////////////////////
struct _ieee80211SpecificInformation
{
    uint8_t  network_membership[6]; // BSSID

    #define IEEE80211_SPECIFIC_INFO_ROLE_AP                   (0x0)
    #define IEEE80211_SPECIFIC_INFO_ROLE_NON_AP_NON_PCP_STA   (0x4)
    #define IEEE80211_SPECIFIC_INFO_ROLE_WIFI_P2P_CLIENT      (0x8)
    #define IEEE80211_SPECIFIC_INFO_ROLE_WIFI_P2P_GROUP_OWNER (0x9)
    #define IEEE80211_SPECIFIC_INFO_ROLE_AD_PCP               (0xa)
    uint8_t  role;                  // One of the values from above

    uint8_t ap_channel_band;        // Hex value of dot11CurrentChannelBandwidth
                                  // (see "IEEE P802.11ac/D3.0" for description)

    uint8_t ap_channel_center_frequency_index_1;
                                  // Hex value of
                                  // dot11CurrentChannelCenterFrequencyIndex1
                                  // (see "IEEE P802.11ac/D3.0" for description)

    uint8_t ap_channel_center_frequency_index_2;
                                  // Hex value of
                                  // dot11CurrentChannelCenterFrequencyIndex2
                                  // (see "IEEE P802.11ac/D3.0" for description)
};

struct _ieee1901SpecificInformation
{
    uint8_t network_identifier[7];  // Network membership
};

union _mediaSpecificData
{
    uint8_t                                dummy;    // Empty placeholder
    struct _ieee80211SpecificInformation ieee80211;
    struct _ieee1901SpecificInformation  ieee1901;

};

struct _localInterfaceEntries
{
    uint8_t   mac_address[6];       // MAC address of the local interface

    uint16_t  media_type;           // One of the MEDIA_TYPE_* values

    uint8_t   media_specific_data_size;
                                  // Number of bytes in ensuing field
                                  // Its value is '10' when 'media_type' is one
                                  // of the valid MEDIA_TYPE_IEEE_802_11*
                                  // values.
                                  // Its value is '7' when 'media_type' is one
                                  // of the valid MEDIA_TYPE_IEEE_1901* values.

    union _mediaSpecificData media_specific_data;
                                  // Media specific data
                                  // It will contain a IEEE80211 structure
                                  // when 'media_type' is one of the valid
                                  // MEDIA_TYPE_IEEE_802_11* values
                                  // It will contain a IEE1905 structure
                                  // when 'media_type' is one of the valid
                                  // MEDIA_TYPE_IEEE_1901* values
                                  // It will be empty in the rest of cases

};

struct deviceInformationTypeTLV
{
    uint8_t   tlv_type;             // Must always be set to
                                  // TLV_TYPE_DEVICE_INFORMATION_TYPE

    uint8_t   al_mac_address[6];    // 1905 AL MAC address of the device

    uint8_t   local_interfaces_nr;  // Number of local interfaces

    struct _localInterfaceEntries  *local_interfaces;

};

////////////////////////////////////////////////////////////////////////////////
// Device bridging capability TLV associated structures ("1905 Section 6.4.6")
////////////////////////////////////////////////////////////////////////////////
struct _bridgingTupleMacEntries
{
    uint8_t   mac_address[6];       // MAC address of a 1905 device's network
                                  // interface that belongs to a bridging tuple
};
struct _bridgingTupleEntries
{
    uint8_t   bridging_tuple_macs_nr; // Number of MAC addresses in this bridging
                                    // tuple

    struct  _bridgingTupleMacEntries  *bridging_tuple_macs;
                                   // List of 'mac_nr' elements, each one
                                   // representing a MAC. All these MACs are
                                   // bridged together.
};
struct deviceBridgingCapabilityTLV
{
    uint8_t   tlv_type;             // Must always be set to
                                  // TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES

    uint8_t   bridging_tuples_nr;   // Number of MAC addresses in this bridging
                                  // tuple

    struct _bridgingTupleEntries  *bridging_tuples;
};

////////////////////////////////////////////////////////////////////////////////
// Non-1905 neighbor device list TLV associated structures ("1905 Section 6.4.8")
////////////////////////////////////////////////////////////////////////////////
struct _non1905neighborEntries
{
    uint8_t   mac_address[6];       // MAC address of the non-1905 device
};
struct non1905NeighborDeviceListTLV
{
    uint8_t   tlv_type;             // Must always be set to
                                  // TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST

    uint8_t   local_mac_address[6]; // MAC address of the local interface

    uint8_t                           non_1905_neighbors_nr;
    struct _non1905neighborEntries *non_1905_neighbors;
                                  // One entry for each non-1905 detected
                                  // neighbor
};


////////////////////////////////////////////////////////////////////////////////
// 1905 Neighbor device TLV associated structures ("1905 Section 6.4.9")
////////////////////////////////////////////////////////////////////////////////
struct _neighborEntries
{
    uint8_t   mac_address[6];       // AL MAC address of the 1905 neighbor

    uint8_t   bridge_flag;          // "0" --> no IEEE 802.1 bridge exists
                                  // "1" --> at least one IEEE 802.1 bridge
                                  //         exists between this device and the
                                  //         neighbor
};
struct neighborDeviceListTLV
{
    uint8_t   tlv_type;             // Must always be set to
                                  // TLV_TYPE_NEIGHBOR_DEVICE_LIST

    uint8_t   local_mac_address[6]; // MAC address of the local interface

    uint8_t                     neighbors_nr;
    struct _neighborEntries  *neighbors;
                                  // One entry for each 1905 detected neighbor
};

////////////////////////////////////////////////////////////////////////////////
// Supported role TLV associated structures ("1905 Section 6.4.16")
////////////////////////////////////////////////////////////////////////////////

struct supportedRoleTLV
{
    uint8_t   tlv_type;             // Must always be set to
                                  // TLV_TYPE_SUPPORTED_ROLE

    uint8_t   role;                 // One of the values from IEEE80211_ROLE_*
};

////////////////////////////////////////////////////////////////////////////////
// Supported frequency band TLV associated structures ("1905 Section 6.4.17")
////////////////////////////////////////////////////////////////////////////////

struct supportedFreqBandTLV
{
    uint8_t   tlv_type;             // Must always be set to
                                  // TLV_TYPE_SUPPORTED_FREQ_BAND

    uint8_t   freq_band;            // Frequency band of the unconfigured
                                  // interface requesting an autoconfiguration.
                                  // Use one of the values in
                                  // IEEE80211_FREQUENCY_BAND_*
};

////////////////////////////////////////////////////////////////////////////////
// WSC TLV associated structures ("1905 Section 6.4.18")
////////////////////////////////////////////////////////////////////////////////

#define ATTR_MANUFACTURER (0x1021)
#define ATTR_MODEL_NAME (0x1023)
#define ATTR_MODEL_NUMBER (0x1024)
#define ATTR_SERIAL_NUMBER (0x1042)

struct wscTLV
{
    uint8_t   tlv_type;             // Must always be set to
                                  // TLV_TYPE_WSC

    uint16_t wsc_frame_size;
    uint8_t  *wsc_frame;            // Pointer to a buffer containing the M1 or
                                  // M2 message.
};

////////////////////////////////////////////////////////////////////////////////
// Supported Service TLV associated structures (Multi-AP "Section 17.2.1")
////////////////////////////////////////////////////////////////////////////////
struct SupportedServiceTLV
{
    uint8_t  tlv_type;                  // Must always be set to
                                      // TLV_TYPE_SUPPORTED_SERVICE

    uint8_t  supported_service_nr;        // The number of supported services
    uint8_t  *supported_service;          // The list of supported services
                                        // length: supported_service_nr

};

////////////////////////////////////////////////////////////////////////////////
// AP Operational BSS TLV associated structures (Multi-AP "Section 17.2.4")
////////////////////////////////////////////////////////////////////////////////

struct BSS
{
    uint8_t  mac_addr[6];
    uint8_t  ssid_len;         // SSID length
    char  *ssid;             // This is the "friendly" name of the wifi network
};

struct RADIO
{
    uint8_t       radio_unique_id[6];    // Radio Unique Identifier of a radio.
    uint8_t       BSSs_nr;               // Number of BSS (802.11 Local interfaces) currently operating on the radio.
    struct BSS *BSSs;

};

struct ApOperationalBSSTLV
{
    uint8_t         tlv_type;             // Must always be set to
                                        // TLV_TYPE_AP_OPERATIONAL_BSS

    uint8_t         radios_nr;            // Number of radios reported.
    struct RADIO *radios;
};

////////////////////////////////////////////////////////////////////////////////
// Associated Clients TLV associated structures (Multi-AP "Section 17.2.5")
////////////////////////////////////////////////////////////////////////////////
struct Client
{
    uint8_t    mac_addr[6];       // The MAC address of the associated 802.11 client.
    uint16_t   left_time;         // Time since the 802.11 client’s last association to this Multi-AP device in
                                // seconds.
                                // 0x0000 – 0xFFFE: 0 - 65,534
                                // 0xFFFF: 65,535 or higher
};

struct BSSWithClients
{
    uint8_t             bssid[6];           // Any EUI-48 value,
                                          // The BSSID of the BSS operated by the Multi-AP Agent in which the client is associated.
    uint16_t            clients_nr;         // Number of clients associated to the BSS.
    struct Client    *clients;

};
struct AssociatedClientsTLV
{
    uint8_t                    tlv_type;               // Must always be set to
                                                     // TLV_TYPE_ASSOCIATED_CLIENTS
    uint8_t                    BSSs_nr;                // Number of BSSs included in this TLV.
    struct BSSWithClients   *BSSs;

};

////////////////////////////////////////////////////////////////////////////////
// Channel Prefernrce TLV associated structures (Multi-AP "Section 17.2.13")
////////////////////////////////////////////////////////////////////////////////

struct ChannelPreferencePerOpClass
{
	uint8_t	op_class;
	uint8_t	channel_nr;				// Number of channels specified in the Channel List.
	uint8_t	*channel_list;			// Each octet describes a single channel number in the Operating Class.
									// An empty Channel List field (channel_nr = 0) indicates that the indicated Preference applies to all channels in the Operating Class.
	uint8_t	preference_reason_code;	// bits 7 - 4: Preference -- Indicates a preference value for the channels in the Channel List
									//		0000: Non-operable
									//		0001-1110: Operable with preference score 1-14 ( where 1 is least preferred)
									//		1111: Reserved
									//		Note: the "most preferred" score 15 is inferred for all channels/operating classes that are not specified in the corresponding message.
									// bits 3 - 0: Reason Code -- Indicates the reason for the Preference
									//		0000: Unspecified
									//		0001: Proximate non-802.11 interferer in local environment
									//		0010: Intra-network 802.11 OBSS interference management
									//		0011: External network 802.11 OBSS interference management
									//		0100: Reduced coverage (e.g. due to limited transmit power)
									//		0101: Reduced throughput (e.g. due to limited channel bandwidth of the operating class, or high channel utilization measured on the channel)
									//		0110: In-device Interferer within AP (can only be specified by the Multi-AP Agent)
									//		0111: Operation disallowed due to radar detection on a DFS channel (can only be specified by the Multi-AP Agent)
									//		1000: Operation would prevent backhaul operation using shared radio (can only be spcified by the Multi-AP Agent)
									//		1001: Immediate operation possible on a DFS channel - CAC has been run and is still valid and channel has been cleared for use (can only be specified by the Multi-AP Agent)
									//		1010: DFS channel state unknown (CAC has not run or its validity period has expired) (can only be specified by the Multi-AP Agent)
									//		1011-1111: Reserved
};

struct ChannelPreferenceTLV
{
	uint8_t	tlv_type;											// Must always be set to
																// TLV_TYPE_CHANNEL_PREFERENCE
	uint8_t	radio_unique_id[6];									// Radio Unique identifier of a radio for which preferences are reported in this TLV.
	uint8_t	op_class_nr;										// Number of operating classes for which preferences are reported in this TLV.
	struct ChannelPreferencePerOpClass	*channel_preferences;	// Channel preferences content
};

////////////////////////////////////////////////////////////////////////////////
// Radio Operation Restriction TLV associated structures (Multi-AP "Section 17.2.14")
////////////////////////////////////////////////////////////////////////////////

struct RadioOperationRestrictionPerChannel
{
	uint8_t	channel;				// Channel number for which a restriction applies
	uint8_t	min_freq_separation;	// The minimum frequency separation (in multiples of 10 MHz) that this radio would require when operating on the above channel number between the center frequency of that channel and the center operating frequency of another radio (operating simultaneous TX/RX) of the Multi-AP Agent
};

struct RadioOperationRestrictionPerOpClass
{
	uint8_t	op_class;															// Operating Class
	uint8_t	channel_nr;															// Number of channels specified
	struct RadioOperationRestrictionPerChannel *channel_operation_restriction;	// Channel operation restriction contect
};

struct RadioOperationRestrictionTLV
{
	uint8_t	tlv_type;														// Must always be set to
																			// TLV_TYPE_RADIO_OPERATION_RESTRICTION
	uint8_t	radio_unique_id[6];												// Radio Unique identifier of a radio
	uint8_t	op_class_nr;													// Number of Operating Classes for which restrictions are reported in this TLV
	struct RadioOperationRestrictionPerOpClass *radio_operation_restriction;// Radio operation restriction content
};

////////////////////////////////////////////////////////////////////////////////
// Channel Selection Response TLV associated structures (Multi-AP "Section 17.2.16")
////////////////////////////////////////////////////////////////////////////////

struct ChannelSelectionResponseTLV
{
	uint8_t	tlv_type;														// Must always be set to
																			// TLV_TYPE_CHANNEL_SELECTION_RESPONSE
	uint8_t	radio_unique_id[6];												// Radio Unique identifier
	uint8_t	response;														// Indicates the channel selection response code, with respect to the Channel Selection Request
																			// 0x00: Accept
																			// 0x01: Decline because request violates current preferences which have changed since last reported
																			// 0x02: Decline because request violates most recently reported preferences
																			// 0x03: Decline because request would prevent operation of a currently operating backhaul link (where backhaul STA and BSS share a radio)
																			// 0x04-0xFF: Reserved
};

////////////////////////////////////////////////////////////////////////////////
// CAC Completion Report TLV associated structures (Multi-AP "Section 17.2.44")
////////////////////////////////////////////////////////////////////////////////

struct CACCompletionReportClassChannelPairs
{
    uint8_t pairs_op_class;
    uint8_t pairs_channel;
};

struct CACCompletionReportRadio
{
    uint8_t   radio_unique_identifier[6];
    uint8_t   op_class;
    uint8_t   channel;
    uint8_t   flags;  //CAC method to be used.
                    //(0: Successful, 1: Radar detected, 2: CAC not supported as requested (capability mismatch), 3: Radio too busy to perform CAC, 4: Request was considered to be non-conformant to regulations in the country in which the Multi-AP Agent is operating, 5: Other error, >5: Reserved)
    uint8_t   pairs_nr;
    struct CACCompletionReportClassChannelPairs* pairs;
};

struct CACCompletionReportTLV
{
    uint8_t   tlv_type;
    uint8_t   radio_nr;
    struct CACCompletionReportRadio* radios;
};

////////////////////////////////////////////////////////////////////////////////
// Operating Channel Report TLV associated structures (Multi-AP "Section 17.2.17")
////////////////////////////////////////////////////////////////////////////////

struct OperatingChannelReportPerOpClass
{
	uint8_t	op_class;														// Operating Class
	uint8_t	cur_channel;													// Current operating channel number in the Operating Class
};
struct OperatingChannelReportTLV
{
	uint8_t	tlv_type;														// Must always be set to
																			// TLV_TYPE_OPERATING_CHANNEL_REPORT
	uint8_t	radio_unique_id[6];												// Radio Unique identifier
	uint8_t	cur_op_class_nr;												// Number of current operating classes
	struct OperatingChannelReportPerOpClass *operating_channels;			// Operating channels information
	uint8_t	cur_transmit_power;												// Current Transmit Power EIRP representing the current nominal transmit power.
																			// This field is coded as a 2's complement signed integer in units of decibels relative to 1mW(dBm).
																			// This value is less than or equal to the Maximum Transmit Power specified in the AP Radio Basic Capabilities TLV for the current operating class.
};

////////////////////////////////////////////////////////////////////////////////
// Channel Scan Report TLV associated structures (Multi-AP "Section 17.2.40")
////////////////////////////////////////////////////////////////////////////////

struct ChannelScanNeighbor
{
    uint8_t  bssid[6];
    uint8_t  ssid_length;
    char*    ssid;
    uint8_t  signal_strength;
    uint8_t  channel_bandwidth_length;
    char*    channel_bandwidth;
    uint8_t  field_present_flags;
    uint8_t  channel_utilization;
    uint16_t station_count;
};

struct ChannelScanResultTLV
{
    uint8_t	 tlv_type;
    uint8_t	 radio_unique_identifier[6];
    uint8_t	 op_class;
    uint8_t	 channel;
    uint8_t	 scan_status;
    //The following fields are only present if Scan Status is set to 0x00
    uint8_t	 timestamp_length;
    char*    timestamp;
    uint8_t  channel_utilization;
    uint8_t  noise;
    uint16_t neighbor_nr;
    struct   ChannelScanNeighbor* neighbors;
    uint32_t aggregate_scan_duration;
    uint8_t  scan_type_flags; //bit7 - 1: Scan was an Active scan 0: Scan was Passive scan
};

////////////////////////////////////////////////////////////////////////////////////////
// Client Info TLV associated structures (Multi-AP "Section 17.2.18")
////////////////////////////////////////////////////////////////////////////////////////

struct ClientInfoTLV
{
    uint8_t   tlv_type;                // Must always be set to
                                       // TLV_TYPE_CLIENT_INFO
    uint8_t   bssid[6];                // The BSSID of a BSS.
    uint8_t   mac_address[6];
};

////////////////////////////////////////////////////////////////////////////////
// AP Capability TLV associated structures (Multi-AP "Section 17.2.6")
////////////////////////////////////////////////////////////////////////////////

#define MASK_APCAPABILITY_METRICS_REPORTING_ON_BSS        (0x80)
#define MASK_APCAPABILITY_METRICS_REPORTING_NOT_ON_BSS    (0x40)
#define MASK_APCAPABILITY_SUPPORT_RCPI_BASED_STEERING     (0x20)

struct APCapabilityTLV
{
    uint8_t   tlv_type;              // Must always be set to
                                     // TLV_TYPE_AP_CAPABILITY
    uint8_t   ap_capability;
};

////////////////////////////////////////////////////////////////////////////////
// AP Radio Basic Capability TLV associated structures (Multi-AP "Section 17.2.7")
////////////////////////////////////////////////////////////////////////////////
struct OperatingClass
{
    uint8_t   operating_class;            // Operating class per Table E-4 in [1], that this radio is capable of operating
                                          // on.

    uint8_t   max_transmit_power;         // Maximum transmit power EIRP that this radio is capable of transmitting in
                                          // the current regulatory domain for the operating class.
                                          // The field is coded as a 2's complement signed integer in units of decibels
                                          // relative to 1 mW (dBm).

    uint8_t    non_operable_channels_nr;  // Number of statically non-operable channels in the operating class.
                                          //    Other channels from this operating class which are not listed here are
                                          //    supported for the radio.

    uint8_t   *non_operable_channels;     // Channel number which is statically non-operable in the operating class
                                          //    (i.e. the radio is never able to operate on this channel).
                                          //    This field is not present if non_operable_channels_nr = 0.
};
struct APRadioBasicCapabilitiesTLV
{
    uint8_t                  tlv_type;                // Must always be set to
                                                    // TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES
    uint8_t                  radio_unique_id[6];      // Radio Unique Identifier of a radio.
    uint8_t                  max_BSSs_nr;             // Non-zero. Maximum number of BSSs supported by this radio.

    uint8_t                  operating_classes_nr;       // Number of operating classes supported for the radio, defined per Table E-
                                                       // 4 in [1].
                                                       // All the supported operating classes are reported per regulatory
                                                       // restrictions.
    struct OperatingClass *operating_classes;
};

////////////////////////////////////////////////////////////////////////////////////////
// AP HT Capabilities TLV associated structures (Multi-AP "Section 17.2.8")
////////////////////////////////////////////////////////////////////////////////////////

#define MASK_APHTCAPABILITY_MAX_SUPPORTED_TX    (0xC0)
#define MASK_APHTCAPABILITY_MAX_SUPPORTED_RX    (0x30)
#define MASK_APHTCAPABILITY_SHORT_GI_20M        (0x08)
#define MASK_APHTCAPABILITY_SHORT_GI_40M        (0x04)
#define MASK_APHTCAPABILITY_HT_40M              (0x02)

struct APHTCapabilitiesTLV
{
    uint8_t   tlv_type;                // Must always be set to
                                     // TLV_TYPE_AP_HT_CAPABILITIES
    uint8_t   radio_unique_id[6];      // Radio unique identifier of the radio for which HT capabilities are reported.
    uint8_t   ht_capability;
};

////////////////////////////////////////////////////////////////////////////////////////
// AP VHT Capabilities TLV associated structures (Multi-AP "Section 17.2.9")
////////////////////////////////////////////////////////////////////////////////////////
/// 000: 1 Tx spatial stream
/// 001: 2 Tx spatial stream
/// 010: 3 Tx spatial stream
/// 011: 4 Tx spatial stream
/// 100: 5 Tx spatial stream
/// 101: 6 Tx spatial stream
/// 110: 7 Tx spatial stream
/// 111: 8 Tx spatial stream
///
/// 000: 1 Rx spatial stream
/// 001: 2 Rx spatial stream
/// 010: 3 Rx spatial stream
/// 011: 4 Rx spatial stream
/// 100: 5 Rx spatial stream
/// 101: 6 Rx spatial stream
/// 110: 7 Rx spatial stream
/// 111: 8 Rx spatial stream
///


#define MASK_VHTCAPABILITY_MAX_SUPPORTED_TX   (0xE0) // Maximum number of supported Tx spatial streams.
#define MASK_VHTCAPABILITY_MAX_SUPPORTED_RX   (0x1C) // Maximum number of supported Rx spatial streams.
#define MASK_VHTCAPABILITY_SHORT_GI_80M       (0x02) // Short GI Support for 80 MHz.
#define MASK_VHTCAPABILITY_SHORT_GI_160M      (0x01) // Short GI Support for 160 MHz and 80+80 MHz.

#define MASK_VHTCAPABILITY_MAX_SUPPORTED_TX_SHIFT (5)
#define MASK_VHTCAPABILITY_MAX_SUPPORTED_RX_SHIFT (2)

#define MASK_VHTCAPABILITY_VHT_SUPPORT_80_80      (0x80) // VHT Support for 80+80 MHz.
#define MASK_VHTCAPABILITY_VHT_SUPPORT_160        (0x40) // VHT Support for 160 MHz.
#define MASK_VHTCAPABILITY_SU_BEAMFORMER_CAPABLE  (0x20) // SU Beamformer Capable.
#define MASK_VHTCAPABILITY_MU_BEAMFORMER_CAPABLE  (0x10) // MU Beamformer Capable.

struct APVHTCapabilitiesTLV
{
    uint8_t    tlv_type;                // Must always be set to
                                        // TLV_TYPE_AP_VHT_CAPABILITIES
    uint8_t    radio_unique_id[6];      // Radio unique identifier of the radio for which VHT capabilities are reported.

    uint16_t   vht_tx_msc;              // Supported VHT Tx MCS.
                                        // Supported set of VHT MCSs that can be received.
                                        // Set to Tx VHT MCS Map field per Figure 9-562 in [1].
    uint16_t   vht_rx_msc;              // Supported VHT Rx MCS.
                                        // Supported set of VHT MCSs that can be transmitted.
                                        // Set to Rx VHT MCS Map field per Figure 9-562 in [1].
    uint8_t    vht_capability_1;
    uint8_t    vht_capability_2;
};

////////////////////////////////////////////////////////////////////////////////////////
// Client Capability Report TLV associated structures (Multi-AP "Section 17.2.19")
////////////////////////////////////////////////////////////////////////////////////////
#define CLIENT_CAPABILITY_REPORT_SUCCESS 0x00
#define CLIENT_CAPABILITY_REPORT_FAILURE 0x01

struct ClientCapabilityReportTLV
{
    uint8_t   tlv_type;               // Must always be set to
                                      // TLV_TYPE_CLIENT_CAPABLITY_REPORT
    uint8_t   result_code;            // Result Code for the client capability report message.
                                      // 0x00: Success
                                      // 0x01:Failure
                                      // 0x02 – 0xFF: Reserved
    uint16_t  frame_body_length;      // The length of the frame body
    uint8_t   *frame_body;            // The frame body of the most recently received (Re)Association Request
                                      // frame from this client, as defined in Table 9-29 and Table 9-31 of [1]. If
                                      // Result Code is not equal to 0x00, this field is omitted.

};

////////////////////////////////////////////////////////////////////////////////
// Transmitter link metric TLV associated structures ("1905 Section 6.4.11")
////////////////////////////////////////////////////////////////////////////////
struct _transmitterLinkMetricEntries
{
    uint8_t   local_interface_address[6];      // MAC address of an interface in
                                             // the receiving AL, which connects
                                             // to an interface in the neighbor
                                             // AL

    uint8_t   neighbor_interface_address[6];   // MAC addres of an interface in a
                                             // neighbor AL, which connects to
                                             // an interface in the receiving
                                             // AL

    uint16_t  intf_type;                // Underlaying network technology
                                      // One of the MEDIA_TYPE_* values.

    uint8_t   bridge_flag;              // Indicates whether or not the 1905 link
                                      // includes one or more IEEE 802.11
                                      // bridges

    uint32_t  packet_errors;            // Estimated number of lost packets on the
                                      // transmitting side of the link during
                                      // the measurement period (5 seconds??)

    uint32_t  transmitted_packets;      // Estimated number of packets transmitted
                                      // on the same measurement period used to
                                      // estimate 'packet_errors'

    uint16_t  mac_throughput_capacity;  // The maximum MAC throughput of the link
                                      // estimated at the transmitter and
                                      // expressed in Mb/s

    uint16_t  link_availability;        // The estimated average percentage of
                                      // time that the link is available for
                                      // data transmissions

    uint16_t  phy_rate;                 // This value is the PHY rate estimated at
                                      // the transmitter of the link expressed
                                      // in Mb/s
};
struct transmitterLinkMetricTLV
{
    uint8_t  tlv_type;               // Must always be set to
                                   // TLV_TYPE_TRANSMITTER_LINK_METRIC

    uint8_t  local_al_address[6];    // AL MAC address of the device that
                                   // transmits the response message that
                                   // contains this TLV

    uint8_t  neighbor_al_address[6]; // AL MAC address of the neighbor whose
                                   // link metric is reported in this TLV

    uint8_t                                  transmitter_link_metrics_nr;
    struct _transmitterLinkMetricEntries  *transmitter_link_metrics;
                                   // Link metric information for the above
                                   // interface pair between the receiving AL
                                   // and the neighbor AL
};


////////////////////////////////////////////////////////////////////////////////
// Receiver link metric TLV associated structures ("1905 Section 6.4.12")
////////////////////////////////////////////////////////////////////////////////
struct  _receiverLinkMetricEntries
{
    uint8_t   local_interface_address[6];      // MAC address of an interface in
                                             // the receiving AL, which connects
                                             // to an interface in the neighbor
                                             // AL

    uint8_t   neighbor_interface_address[6];   // MAC addres of an interface in a
                                             // neighbor AL, which connects to
                                             // an interface in the receiving
                                             // AL

    uint16_t  intf_type;                // Underlaying network technology

    uint32_t  packet_errors;            // Estimated number of lost packets on the
                                      // receiving side of the link during
                                      // the measurement period (5 seconds??)

    uint32_t  packets_received;         // Estimated number of packets received on
                                      // the same measurement period used to
                                      // estimate 'packet_errors'

    uint8_t  rssi;                      // This value is the estimated RSSI at the
                                      // receive side of the link expressed in
                                      // dB
};

#define MEDIA_TYPE_IEEE_802_3U_FAST_ETHERNET       (0x0000)
#define MEDIA_TYPE_IEEE_802_3AB_GIGABIT_ETHERNET   (0x0001)
#define MEDIA_TYPE_IEEE_802_11B_2_4_GHZ            (0x0100)
#define MEDIA_TYPE_IEEE_802_11G_2_4_GHZ            (0x0101)
#define MEDIA_TYPE_IEEE_802_11A_5_GHZ              (0x0102)
#define MEDIA_TYPE_IEEE_802_11N_2_4_GHZ            (0x0103)
#define MEDIA_TYPE_IEEE_802_11N_5_GHZ              (0x0104)
#define MEDIA_TYPE_IEEE_802_11AC_5_GHZ             (0x0105)
#define MEDIA_TYPE_IEEE_802_11AD_60_GHZ            (0x0106)
#define MEDIA_TYPE_IEEE_802_11AF                   (0x0107)
#define MEDIA_TYPE_IEEE_802_11AX                   (0x0108)
#define MEDIA_TYPE_IEEE_1901_WAVELET               (0x0200)
#define MEDIA_TYPE_IEEE_1901_FFT                   (0x0201)
#define MEDIA_TYPE_MOCA_V1_1                       (0x0300)
#define MEDIA_TYPE_UNKNOWN                         (0xFFFF)

#define IS_IEEE_802_3_MEDIA(x)  (x >> 8 == 0)
#define IS_IEEE_802_11_MEDIA(x) (x >> 8 == 1)
#define IS_IEEE_1901_MEDIA(x)   (x >> 8 == 2)
#define IS_IEEE_MOCA_MEDIA(x)   (x >> 8 == 3)

#define IS_IEEE_802_11_2_4_GHZ_MEDIA(x) \
	(x == MEDIA_TYPE_IEEE_802_11B_2_4_GHZ \
	|| x == MEDIA_TYPE_IEEE_802_11G_2_4_GHZ \
	|| x == MEDIA_TYPE_IEEE_802_11N_2_4_GHZ)

#define IS_IEEE_802_11_5_GHZ_MEDIA(x) \
	(x == MEDIA_TYPE_IEEE_802_11A_5_GHZ \
	|| x == MEDIA_TYPE_IEEE_802_11N_5_GHZ \
	|| x == MEDIA_TYPE_IEEE_802_11AC_5_GHZ)

#define IS_IEEE_802_11AX_MEDIA(x) (x == MEDIA_TYPE_IEEE_802_11AX)

struct receiverLinkMetricTLV
{
    uint8_t   tlv_type;              // Must always be set to
                                   // TLV_TYPE_RECEIVER_LINK_METRIC

    uint8_t local_al_address[6];     // AL MAC address of the device that
                                   // transmits the response message that
                                   // contains this TLV

    uint8_t neighbor_al_address[6];  // AL MAC address of the neighbor whose
                                   // link metric is reported in this TLV

    uint8_t                               receiver_link_metrics_nr;
    struct _receiverLinkMetricEntries  *receiver_link_metrics;
                                   // Link metric information for the above
                                   // interface pair between the receiving AL
                                   // and the neighbor AL
};

////////////////////////////////////////////////////////////////////////////////////////
// AP Metrics TLV associated structures (Multi-AP "Section 17.2.22")
////////////////////////////////////////////////////////////////////////////////////////

struct APMetricsTLV
{
    uint8_t   tlv_type;               // Must always be set to
                                    // TLV_TYPE_AP_METRICS
    uint8_t   bssid[6];            	// Number of BSSIDs included in this TLV

    uint8_t   ch_util;    			// Channel Utilization aas measured by the radio operating the BSS

    uint16_t	sta_nr;					// Total number of STAs currently associated with this BSS

	uint8_t	esp_ie;					//Estimated Service Parameters Information Field bit inclusion
									//Bit 7: AC = BE
									//Bit 6: AC = BK
									//Bit 5: AC = VO
									//Bit 4: AC = VI
									//Bit 3 -0 : Reserved

	uint8_t	esp_acbe[3];			//ESP IE AC=BE
									//Data Format, BA Window Size, Estimated Air Time Fraction and Data PPDU Target Duration

	uint8_t	esp_acbk[3];			//ESP IE AC=BK
									//Data Format, BA Window Size, Estimated Air Time Fraction and Data PPDU Target Duration

	uint8_t	esp_acvo[3];			//ESP IE AC=VO
									//Data Format, BA Window Size, Estimated Air Time Fraction and Data PPDU Target Duration

	uint8_t	esp_acvi[3];			//ESP IE AC=VI
									//Data Format, BA Window Size, Estimated Air Time Fraction and Data PPDU Target Duration
};

////////////////////////////////////////////////////////////////////////////////////////
// Associated STA Traffic Stats TLV associated structures (Multi-AP "Section 17.2.35")
////////////////////////////////////////////////////////////////////////////////////////

struct AssociatedSTATrafficStatsTLV
{
	uint8_t	tlv_type;

	uint8_t	sta_mac_address[6];

	uint32_t	bytes_sent;

	uint32_t	bytes_received;

	uint32_t	packets_sent;

	uint32_t	packets_received;

	uint32_t	tx_packets_errors;

	uint32_t	rx_packets_errors;

	uint32_t	retransmission_count;
};

////////////////////////////////////////////////////////////////////////////////////////
// Associated STA Metrics TLV associated structures (Multi-AP "Section 17.2.24")
////////////////////////////////////////////////////////////////////////////////////////

struct AssocSTALinkMetrics
{
	uint8_t	bssid[6];

	uint32_t	time_delta;

	uint32_t	dataRate_downlink;

	uint32_t	dataRate_uplink;

	uint8_t	uplink_rcpi; //rcpi
};

struct AssociatedSTALinkMetricsTLV
{
	uint8_t   tlv_type;

	uint8_t	mac_address[6];

	uint8_t	bssid_nr;

	struct AssocSTALinkMetrics *assoc_sta_link_metrics;
};

////////////////////////////////////////////////////////////////////////////////////////
// Unassociated STA Link Metrics Response TLV associated structures (Multi-AP "Section 17.2.26")
////////////////////////////////////////////////////////////////////////////////////////

struct UnassocSTALinkMetrics
{
	uint8_t	sta_mac_address[6];

	uint8_t	channel_number;

	uint32_t	time_delta;

	uint8_t	uplink_rcpi;
};

struct UnassociatedSTALinkMetricsResponseTLV
{
	uint8_t	tlv_type;

	uint8_t	op_class;

	uint8_t	sta_nr;

	struct UnassocSTALinkMetrics *unassoc_sta_link_metrics;
};

////////////////////////////////////////////////////////////////////////////////////////
// Beacon Metrics Response TLV associated structures (Multi-AP "Section 17.2.28")
////////////////////////////////////////////////////////////////////////////////////////
#define BEACON_REPORT_FIXED_IE 29
#pragma pack(1)
struct BeaconMeasurementReportInfo
{
    uint8_t op_class;
    uint8_t channel;
    uint32_t  measure_time_hi;
    uint32_t  measure_time_lo;
    uint16_t measure_duration;
    uint8_t frame_info;
    uint8_t RCPI;
    uint8_t RSNI;
    uint8_t bssid[6];
    uint8_t antenna_id;
    uint32_t  parent_tsf;
};

struct BeaconMeasurementReportIE
{
	uint8_t elementId;
	uint8_t len;
	uint8_t measurementToken;
	uint8_t measurementReportMode;
	uint8_t measurementType;
    struct BeaconMeasurementReportInfo info;
	uint8_t subelement_id;
    uint8_t subelements_len;
    uint8_t *subelements;
};
#pragma pack(0)
struct BeaconMetricsResponseTLV
{
	uint8_t	tlv_type;

	uint8_t	mac_address[6];

	uint8_t	reserved_field;

	uint8_t	measurement_report_nr;

	struct	BeaconMeasurementReportIE *measurement_reports;
};

////////////////////////////////////////////////////////////////////////////////////////
// Steering BTM Report TLV associated structures (Multi-AP "Section 17.2.30")
////////////////////////////////////////////////////////////////////////////////////////

struct SteeringBTMReportTLV
{
	uint8_t	tlv_type;				// Must always be set to
									// TLV_TYPE_STEERING_BTM_REPORT
	uint8_t	bssid[6];				// BSSID - unique identifier of the BSS
	uint8_t	sta_mac_address[6];		// STA MAC address for which the steering BTM report applies
	uint8_t	status_code;			// BTM Status Code
	uint8_t	target_bssid[6];		// Target BSSID
};

////////////////////////////////////////////////////////////////////////////////////////
// Backhaul Steering Response TLV associated structures (Multi-AP "Section 17.2.33")
////////////////////////////////////////////////////////////////////////////////////////

struct BackhaulSteeringResponseTLV{

	uint8_t	tlv_type;

	uint8_t	backhaul_mac[6];

	uint8_t	target_bssid[6];

	uint8_t	result_code;
};

////////////////////////////////////////////////////////////////////////////////////////
// Error Code TLV associated structures (Multi-AP "Section 17.2.36")
////////////////////////////////////////////////////////////////////////////////////////

struct ErrorCodeTLV
{
    uint8_t   tlv_type;               // Must always be set to
                                    // TLV_TYPE_ERROR_CODE
    uint8_t   reason_code;            // 0x00: Reserved Reason code.
                                    //    0x01: STA associated with a BSS operated by the Agent.
                                    //    0x02: STA not associated with any BSS operated by the Agent.
                                    //    0x03: Client capability report unspecified failure
                                    //    0x04: Backhaul steering request rejected because the backhaul station cannot operate on the                                 channel specified.
                                    //    0x05: Backhaul steering request rejected because the target BSS signal is too weak or not found.
                                    //    0x06: Backhaul steering request authentication or association Rejected by the target BSS.
                                    //    0x07 – 0xFF: Reserved


    uint8_t   sta_mac_address[6];     // The MAC address of the STA referred to in the previous field.
                                    // The value of this field is valid only if the reason code field is set to 0x01 or
                                    // 0x02. Otherwise this field is ignored by the recipient of this TLV.
};

struct vendorSpecificTLV
{
    uint8_t   tlv_type;             // Must always be set to
                                  // TLV_TYPE_VENDOR_SPECIFIC

    uint8_t  vendor_oui[3];          // Vendor specific OUI, the value of the 24
                                  // bit globally unique IEEE-SA assigned number
                                  // to the vendor

    uint16_t  m_nr;                 // Bytes in the following field
    uint8_t  *m;                    // Vendor specific information
};

////////////////////////////////////////////////////////////////////////////////////////
// Multi-AP Profile TLV associated structures (Multi-AP "Section 17.2.46")
////////////////////////////////////////////////////////////////////////////////////////

struct MultiAPProfileTLV
{
    uint8_t   tlv_type;             // Must always be set to
                                    // TLV_TYPE_MULTIAP_PROFILE
    uint8_t   profile;              // 0x00: Reserved
                                    // 0x01: Multi-AP Profile-1
                                    // 0x02: Multi-AP Profile-2
                                    // 0x03 ~0xFF Reserved
};

////////////////////////////////////////////////////////////////////////////////////////
// Traffic Separation TLV associated structures
////////////////////////////////////////////////////////////////////////////////////////

struct Default8021QSettingsTLV
{
    uint8_t   tlv_type;

    uint16_t  primary_vlan_id;

    uint8_t   default_pcp;
};

struct TrafficSeparationSsidInfo
{
    uint8_t   ssid_length;

    char   *ssid_name;

    uint16_t  vlan_id;
};

struct TrafficSeparationPolicyTLV
{
    uint8_t   tlv_type;

    uint8_t   ssid_nr;

    struct TrafficSeparationSsidInfo   *ssid_info;
};

#define INDEPENDENT_SCAN_FLAG_MASK           0x80 //1000 0000
#define REPORT_INDEPENDENT_SCAN_TRUE         1
#define REPORT_INDEPENDENT_SCAN_FALSE        0
struct ChannelScanReportingPolicyTLV
{
    uint8_t	tlv_type;
    uint8_t	independent_report_flag;
};

struct ChannelScanCapabilitiesOpClass
{
    uint8_t	op_class;
    uint8_t	channel_nr;
    uint8_t*	channel_list;
};

#define ON_BOOT_ONLY_MASK                   0x80 //1000 0000
#define ON_BOOT_ONLY_FALSE                  0
#define ON_BOOT_ONLY_TRUE                   1

#define SCAN_IMPACT_MASK                    0x60  //0110 0000
#define NO_IMPACT                           0b00
#define REDUCED_SPATIAL_STREAMS             0b01
#define TIME_SLICING                        0b10
#define UNAVAILABLE_FOR_MORE_THAN_2_SECONDS 0b11

struct ChannelScanCapabilitiesRadio
{
    uint8_t	radio_unique_identifier[6];
    uint8_t	scan_capability_flags;
    uint32_t	minimum_scan_interval;
    uint8_t	op_class_nr;
    struct ChannelScanCapabilitiesOpClass* op_classes;
};

struct ChannelScanCapabilitiesTLV
{
    uint8_t 	tlv_type;
    uint8_t 	radio_nr;
    struct ChannelScanCapabilitiesRadio* radios;
};

struct ChannelScanTargetOpClass
{
    uint8_t 	op_class;
    uint8_t 	target_channel_nr;
    uint8_t*	target_channel_list;
};

struct ChannelScanTargetRadio
{
    uint8_t 	radio_unique_identifier[6];
    uint8_t 	target_op_class_nr;
    struct  ChannelScanTargetOpClass* target_op_classes;
};

#define PERFORM_FRESH_SCAN_MASK   0x80   //1000 0000
#define PERFORM_FRESH_SCAN_TRUE   1
#define PERFORM_FRESH_SCAN_FALSE  0
struct ChannelScanRequestTLV
{
	uint8_t 	tlv_type;
	uint8_t 	flags;  //bit 7 - 1: Perform a fresh scan and return results 0: Return stored results of last successful scan     bits 6-0 - reserverd
	uint8_t 	target_radio_nr;
    struct  ChannelScanTargetRadio* target_radios;
};

#define CHANNELUTLIZATION_PRESENT_MASK   0x80   //1000 0000
#define STATIONCOUNT_PRESENT_MASK        0x40   //0100 0000


struct TimestampTLV
{
    uint8_t	tlv_type;
    uint8_t   timestamp_length;     //length of the timestamp string
    char*   timestamp;
};

#define PROTOCOL_1905_DEVICE_PROVISIONING     0x00
#define ALGORITHM_HMAC_SHA254                 0x00
#define ENCRYTPTION_AES_SIV                   0x00

struct SecurityCapabilityTLV
{
	uint8_t	tlv_type;

	uint8_t	onboarding_protocol_supported;

	uint8_t	message_integrity_algorithm_supported;

	uint8_t	message_encryption_algorithm_supported;
};

struct GroupIntegrityKeyTLV
{
	uint8_t	tlv_type;

	uint8_t	key_identifier;

	uint8_t	key_len;

	uint8_t	*key;

	uint8_t	algorithm;
};

#define MIC_VERSION_1   0x0
struct MICTLV
{
	uint8_t		tlv_type;

	uint8_t		gtk_key_id_ver;

	uint64_t	integrity_transmission_counter;

	uint8_t		src_1905_al_mac_addr[6];

	uint16_t	mic_length;

	uint8_t		mic[64];
};

struct CACRequestRadio
{
    uint8_t   radio_unique_identifier[6];
    uint8_t   op_class;
    uint8_t   channel;
    uint8_t   flags;  //Bit 7-5 CAC method to be used.(0: Continuous CAC, 1: Continuous with dedicated radio, 2: MIMO dimension reduced, 3: Time sliced CAC, >3: Reserved)
                    //Bit 4-3 CAC Completion Action for Successful CAC.(0: Remain on channel and continue to monitor for radar, 1: Return the radio that was performing the CAC to its most recent operational configuration., >1: Reserved)
                    //Bits 2-0 (Reserved)
};

struct CACRequestTLV
{
    uint8_t   tlv_type;
    uint8_t   radio_nr;
    struct CACRequestRadio* radios;
};

struct CACTerminationRadio
{
    uint8_t   radio_unique_identifier[6];
    uint8_t   op_class;
    uint8_t   channel;
};

struct CACTerminationTLV
{
    uint8_t   tlv_type;
    uint8_t   radio_nr;
    struct CACTerminationRadio* radios;
};


struct CACCompletionReportActiveClassChannelPairs
{
    uint8_t   active_op_class;
    uint8_t   active_channel;
    uint8_t  active_remaining_time[3];  //Seconds

};

struct CACCompletionReportNonOccupancyClassChannelPairs
{
    uint8_t   nonoccup_op_class;
    uint8_t   nonoccup_channel;
    uint16_t  nonoccup_remaining_time;  //Seconds

};

struct CACCompletionReportAvailabelChannels
{
    uint8_t   ac_op_class;
    uint8_t   ac_channel;
    uint16_t  identify_time;  //minutes
                            //Set to zero for non-DFS channels.
};

struct CACStatusReportTLV
{
    uint8_t   tlv_type;
    uint8_t   available_channel_nr;
    struct  CACCompletionReportAvailabelChannels* available_channels;
    uint8_t   nonoccup_pair_nr;
    struct  CACCompletionReportNonOccupancyClassChannelPairs* nonoccup_pairs;
    uint8_t   active_pair_nr;
    struct  CACCompletionReportActiveClassChannelPairs* active_pairs;
};

struct CACCapabilitiesOpClass
{
    uint8_t   op_class;
    uint8_t   channel_nr;
    uint8_t*  channels;
};

struct CACCapabilitiesType
{
    uint8_t   flags; //CAC mode supported (0: Continuous CAC, 1: Continuous with dedicated radio, 2: MIMO dimension reduced, 3: Time sliced CAC, >3: Reserved)
    uint8_t   second_nr[3];
    uint8_t   op_class_nr;
    struct  CACCapabilitiesOpClass* op_classes;
};

struct CACCapabilitiesRadio
{
    uint8_t   radio_unique_identifier[6];
    uint8_t   type_nr;
    struct  CACCapabilitiesType* types;
};

struct CACCapabilitiesTLV
{
    uint8_t   tlv_type;
    char    country_code[2];
    uint8_t   radio_nr;
    struct  CACCapabilitiesRadio* radios;
};

struct Profile2APCapabilityTLV
{
    uint8_t   tlv_type;

    uint16_t  max_total_nr_service_prioritization_rules;

    uint8_t   units;  // Byte Counter Units - The units used for byte counters when the Agent reports traffic statistics.
                    // bits 7-6 : 0: bytes; 1: kibibytes (KiB); 2: mebibytes (MiB); 3: reserved
                    // bits 5-0 : Reserved
    uint8_t   max_total_nr_VIDs;
};


struct PacketFilteringBssidInfo
{
    uint8_t   bssid[6];

    uint8_t   permitted_destination_mac_address_nr;

    uint8_t   (*permitted_destination_mac_addresses)[6];
};

struct PacketFilteringPolicyTLV
{
    uint8_t   tlv_type;

    uint8_t   bssid_nr;

    struct PacketFilteringBssidInfo *bssid_info;
};


struct ServicePrioritizationRuleTLV
{
    uint8_t   tlv_type;

    uint32_t  service_prioritization_rule_id;

    uint8_t   add_remove_filter; // Add-Remove Filter bit. If set to 0x00 (remove this filter), all filter field provided bits shall be set to zero.
                               // bit 7 : 0b0: remove this filter; 0b1: add this filter
                               // bits 6-0 : Reserved

    uint8_t   rule_precedence; // higher number means higher priority
                             // 0x00 – 0xFE; 0xFF: Reserved.

    uint8_t   rule_output; // The value of or method used to select the 802.1Q C-TAG PCP value with which to mark the matched packet
                         // 0x00 – 0x0A;

    uint8_t   byte_one;

    uint8_t   byte_two;

    uint8_t   byte_three;

    uint8_t   byte_four;

    uint16_t  vlan_id;

    uint8_t   source_interface_id[6];

    uint8_t   tid;

    uint8_t   source_mac_address[6];

    uint8_t   destination_mac_address[6];

    uint8_t   ipv4_ipv6_dscp;

    uint8_t   ipv4_source_ip_cidr_address[4];

    uint8_t   ipv4_source_ip_cidr_subnet_mask;

    uint8_t   ipv6_source_ip_cidr_address[16];

    uint8_t   ipv6_source_ip_cidr_subnet_mask;

    uint8_t   ipv4_destination_ip_cidr_address[4];

    uint8_t   ipv4_destination_ip_cidr_subnet_mask;

    uint8_t   ipv6_destination_ip_cidr_address[16];

    uint8_t   ipv6_destination_ip_cidr_subnet_mask;

    uint16_t  source_port;

    uint16_t  destination_port;

    uint8_t   protocol_number;
};

struct DSCPMappingTableTLV
{
    uint8_t   tlv_type;                           // Must always be set to
							            		// TLV_TYPE_DSCP_MAPPING_TABLE

    uint8_t   dscp_to_pcp[64];
};

struct ServicePrioritizationInterfaceExceptionTLV
{
    uint8_t   tlv_type;                           // Must always be set to
							            		// TLV_TYPE_SERVICE_PRIORITIZATION_INTERFACE_EXCEPTION

    uint8_t   interface_exception_nr;

    uint8_t   (*interface_id)[6];
};

struct Profile2ErrorCodeTLV
{
    uint8_t   tlv_type;                           // Must always be set to
												// TLV_TYPE_PROFILE_2_ERROR_CODE

    uint8_t   reason_code;

    uint32_t  service_prioritization_rule_id;
};

struct APOperationalBackhaulBssRadioInfo
{
    uint8_t   radio_id[6];

    uint8_t   bss_nr;

    uint8_t   (*bssid)[6];

    uint8_t   bss_configuration;
};

struct APOperationalBackhaulBssTLV
{
    uint8_t   tlv_type;                           // Must always be set to
							            		// TLV_TYPE_AP_OPERATIONAL_BACKHAUL_BSS

    uint8_t   radio_nr;

    struct APOperationalBackhaulBssRadioInfo *radio_info;
};

struct APRadioAdvancedCapabilitiesTLV
{
    uint8_t   tlv_type;                           // Must always be set to
							            		// TLV_TYPE_AP_RADIO_ADVANCED_CAPABILITIES

    uint8_t   radio_id[6];

    uint8_t   backhaul_bss_traffic_separation_mix_no_support;
};

struct AssociationStatusNotificationBssInfo
{
    uint8_t   bssid[6];

    uint8_t   association_allowance_status;
};

struct AssociationStatusNotificationTLV
{
    uint8_t   tlv_type;                           // Must always be set to
							            		// TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION

    uint8_t   bssid_nr;

    struct AssociationStatusNotificationBssInfo *bss_info;
};

struct SourceInfoTLV
{
    uint8_t   tlv_type;                           // Must always be set to
							            		// TLV_TYPE_SOURCE_INFO

    uint8_t   mac_address[6];
};

struct TunneledMessageTypeTLV
{
    uint8_t   tlv_type;                           // Must always be set to
							            		// TLV_TYPE_TUNNELED_MESSAGE_TYPE

    uint8_t   tunneled_protocol_type_payload;
};

struct TunneledTLV
{
    uint8_t   tlv_type;                           // Must always be set to
							            		// TLV_TYPE_TUNNELED

    uint16_t  tlv_length;

    uint8_t   *payload;
};

struct Profile2SteeringRequestTargetBssidInfo
{
    uint8_t   target_bssid[6];

    uint8_t   target_bss_operating_class;

    uint8_t   target_bss_channel_number;

    uint8_t   reason_code;
};

struct Profile2SteeringRequestTLV
{
    uint8_t   tlv_type;                           // Must always be set to
												// TLV_TYPE_PROFILE_2_STEERING_REQUEST

    uint8_t   bssid[6];

    uint8_t   steer_parameters;

    uint16_t  steer_opportunity_window;

    uint16_t  btm_disassociation_timer;

    uint8_t   sta_list_count;

    uint8_t   (*sta_list)[6];

    uint8_t   target_bssid_list_count;

    struct  Profile2SteeringRequestTargetBssidInfo    *target_bssid_list;
};

struct UnsuccessfulAssociationPolicyTLV
{
    uint8_t   tlv_type;                           // Must always be set to
							            		// TLV_TYPE_UNSUCCESSFUL_ASSOCIATION_POILCY
    uint8_t   report_unsuccessful_associations;   // bit 7: Whether report -- 0: Do not report unsuccessful association attempts 1: Report unsuccessful association attempts
                                                // bit 6 - 0: Reserved
    uint32_t  maximum_reporting_rate;             // Maximum rate for reporting unsuccessful association attempts (in attempts per minute)
};

struct MetricCollectionIntervalTLV
{
    uint8_t   tlv_type;               // Must always be set to
							        // TLV_TYPE_METRIC_COLLECTION_INTERVAL
    uint32_t  collection_interval;    // Device Collection Interval
};


struct RadioMetricsTLV
{
    uint8_t   tlv_type;                        // Must always be set to
							                 // TLV_TYPE_RADIO_METRICS
    uint8_t   radio_unique_identifier[6];      // Radio Unique Identifier of a radio of the Multi-AP Agent for which metrics are being reported.

    uint8_t   noise;                           //Radio Noise

    uint8_t   transmit;                        //Radio Transmit

    uint8_t   receiveself;                     //Radio ReceiveSelf

    uint8_t   receiveother;                    //Radio ReceiveOther
};

struct APExtendedMetricsTLV
{
    uint8_t   tlv_type;                        // Must always be set to
							                 // TLV_TYPE_AP_EXTENDED_METRICS
    uint8_t   radio_unique_identifier[6];      // Radio Unique Identifier of a radio of the Multi-AP Agent for which metrics are being reported.

    uint32_t   unicast_byte_sent;              //UnicastBytesSent

    uint32_t   unicast_byte_received;          //UnicastBytesReceived

    uint32_t   multicast_byte_sent;            //MultiicastBytesSent

    uint32_t   multicast_byte_received;        //MultiicastBytesReceived

    uint32_t   broadcast_byte_sent;            //BroadicastBytesSent

    uint32_t   broadcast_byte_received;        //BroadicastBytesReceived

};

struct ReportedBSSInfo
{
    uint8_t   bssid[6];                        //BSSID of the BSS to which the STA is associated.

    uint32_t   last_data_downlink_rate;         //LastDataDownlinkRate.

    uint32_t   last_data_uplink_rate;           //LastDataUplinkRate.

    uint32_t   utilization_receive;             //UtilizationReceive.

    uint32_t   utilization_transmit;            //UtilizationTransmit.

};

struct AssociatedSTAExtendedLinkMericsTLV
{
    uint8_t   tlv_type;                        // Must always be set to
							                 // TLV_TYPE_ASSOCIATED_STA_EXTENDED_LINK_METRICS
    uint8_t   sta_mac_address[6];              // MAC address of the associated STA.

    uint8_t   bssid_nr;                        //Number of BSSIDs reported for this STA.

    struct ReportedBSSInfo* reported_bss;    //Reported BSS info.

};

struct StatusCodeTLV
{
    uint8_t   tlv_type;                        // Must always be set to
							                 // TLV_TYPE_STATUS_CODE
    uint16_t   status_code;                    // Status Code.

};

struct ReasonCodeTLV
{
    uint8_t   tlv_type;                        // Must always be set to
							                 // TLV_TYPE_REASON_CODE
    uint16_t   reason_code;                    // Reason Code.

};

struct BackhaulSTARadioCapabilitiesTLV
{
    uint8_t   tlv_type;                        // Must always be set to
							                 // TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES
    uint8_t   radio_unique_identifier[6];      // Radio unique identifier of the radio for which capabilities are reported.

    uint8_t   included_mac;                    // bit 7 -- 0: the MAC address is not included below; 1: the MAC address is included below
                                             // bits 6-0 -- Reserved.
    uint8_t   sta_mac_address[6];              // MAC address of the backhaul STA on this radio.(This field is present if the MAC address include field is set to 1).

};

struct AKMSuiteSeletorFHBSS
{
    uint8_t   oui_fronthaul_bss[3];

    uint8_t   akm_suite_type_fronthaul_bss;
};

struct AKMSuiteSeletorBHBSS
{
    uint8_t   oui_backhaul_bss[3];

    uint8_t   akm_suite_type_backhaul_bss;
};

struct AKMSuiteSeletorBHSTA
{
    uint8_t   oui_backhaul_sta[3];

    uint8_t   akm_suite_type_backhaul_sta;
};

struct AKMSuiteCapabilitiesTLV
{
    uint8_t   tlv_type;                                       // Must always be set to
							                                // TLV_TYPE_AKM_SUITE_CAPABILITIES
    uint8_t   akm_suite_selectors_backhaul_sta_nr;            // If the Multi-AP Profile-2 Agent does not support a backhaul STA, m is set to 0.

    struct  AKMSuiteSeletorBHSTA* akm_suite_selector_backhaul_sta;

    uint8_t   akm_suite_selectors_backhaul_bss_nr;            // If the Multi-AP Profile-2 Agent does not support a backhaul BSS, m is set to 0.

    struct  AKMSuiteSeletorBHBSS* akm_suite_selector_backhaul_bss;

    uint8_t   akm_suite_selectors_fronthaul_bss_nr;           // If the Multi-AP Profile-2 Agent does not support a backhaul BSS, m is set to 0.

    struct  AKMSuiteSeletorFHBSS* akm_suite_selector_fronthaul_bss;

};

struct Encap1905DPPOperatingClass
{
    uint8_t   channels_nr;                                    // This field is present if the Channel list field is set to 1.

    uint8_t   *channel;
};

struct Encap1905DPPTLV
{
    uint8_t   tlv_type;                                       // Must always be set to
							                                // TLV_TYPE_1905_ENCAP_DPP
    uint8_t   byte;                                           // bit 7 -- Final destination (0 or 1)
                                                            // bit 6 -- Channel list (0 or 1)
                                                            // bit 5 -- Content type of the encapsulated frame (0: DPP public action frame; 1: GAS frame)
                                                            // bit 4-0 -- Reserved (0)
    uint8_t   sta_mac_address[6];                             // This field is present if the Final destination field is set to 0.

    uint8_t   op_classes_nr;                                  // This field is present if the Channel list field is set to 1.

    struct Encap1905DPPOperatingClass* op_class;

    uint8_t   frame_type;                                     // If bit 5=0, this field is set to the DPP public action frame type.

    uint16_t   encapsulated_frame_len;

    uint8_t   *encapsulated_frame;                            // If bit 5=0, this field is set to the DPP public action frame, else this field is set to a GAS frame that carries the DPP Configuration Protocol payload.

};

struct Encap1905EAPOLTLV
{
    uint8_t   tlv_type;                                       // Must always be set to
							                                // TLV_TYPE_1905_ENCAP_EAPOL
    uint16_t   eapol_frame_payload_len;

    uint8_t   *eapol_frame_payload;                           // EAPOL frame payload.

};

struct DPPBootstrappingURINotificationTLV
{
    uint8_t   tlv_type;                                       // Must always be set to
							                                // TLV_TYPE_DPP_BOOTSTRAPPING_URI_NOTIFICATION
    uint8_t   radio_unique_identifier[6];                     // Radio Unique Identifier of a radio

    uint8_t   local_interface_mac_address[6];                 // MAC Address of Local Interface (equal to BSSID) operating on the radio, on which the URI was received during PBC onboarding.

    uint8_t   sta_mac_address[6];                             // MAC Address of bSTA from which the URI was received during PBC onboarding.

    uint16_t   dpp_bootstrapping_uri_len;

    uint8_t   *dpp_bootstrapping_uri;                          // DPP Bootstrapping URI received during PBC onboarding.

};

struct BackhaulBSSConfigurationTLV
{
    uint8_t   tlv_type;                                       // Must always be set to
							                                // TLV_TYPE_BACKHAUL_BSS_CONFIGURATION
    uint8_t   bssid[6];                                       // BSSID of a bss
    uint8_t   association_disallowed;                          // bit 7 -- Profile-1 Backhaul STA association disallowed.
                                                            // bit 6 -- Profile-2 Backhaul STA association disallowed.
                                                            // bit 5-0 -- Reserved.

};

struct BSSIDtoUniqueBSSIndexMappingOperatingBSS
{
    uint8_t   mac_address[6];                                 //MAC Address of Local Interface (equal to BSSID) operating on the radio.

    uint8_t   unique_bss_index;                               //Unique BSS index (assigned by a Multi-AP Controller).

};

struct BSSIDtoUniqueBSSIndexMappingReportedRadio
{
    uint8_t   radio_unique_identifier[6];                     //Radio Unique Identifier of a radio.

    uint8_t   operating_bss_nr;                               //Number of BSS (802.11 Local interfaces) currently operating on the radio.

    struct BSSIDtoUniqueBSSIndexMappingOperatingBSS* operating_bss;

};

struct BSSIDtoUniqueBSSIndexMappingTLV
{
    uint8_t   tlv_type;                                       // Must always be set to
							                                // TLV_TYPE_BSSID_TO_UNIQUE_BSS_INDEX_MAPPING
    uint8_t   radio_nr;                                      // Number of radios reported.

    struct  BSSIDtoUniqueBSSIndexMappingReportedRadio* reported_radio;

};

// todo define HE MSC structure
struct APHECapabilitiesTLV
{
    uint8_t   tlv_type;                // Must always be set to
                                     // TLV_TYPE_AP_HE_CAPABILITIES
    uint8_t   radio_unique_id[6];      // Radio unique identifier of the radio for which HE capabilities are reported.

    uint8_t    he_msc_len;              // Length of Supported HE MCS field.
    uint8_t   *he_msc;                  // Supported HE MCS indicating set of supported HE Tx and Rx MCS.
                                      //    Set to Tx Rx HE MCS Support field from 802.11ax spec. Variable length
                                      //    from 2-12 bytes.

    uint8_t   he_capability_1;
    uint8_t   he_capability_2;
};


////////////////////////////////////////////////////////////////////////////////////////
/// \brief  Steering Policy TLV associated structures (Multi-AP "Section 17.2.11")
////////////////////////////////////////////////////////////////////////////////////////

#define AGENT_INITIATED_STEERING_DISALLOWED          0x00
#define AGENT_INITIATED_RCPI_BASED_STEERING_MANDATED 0x01
#define AGENT_INITIATED_RCPI_BASED_STEERING_ALLOWED  0x02

struct SteeringPolicy{
    uint8_t    radio_unique_id[6];           // Radio unique identifier of an AP radio for which Multi-AP control policies are being provided.
    uint8_t    policy;                       // Steering Policy
    uint8_t    channel_util_threshold;       // Channel Utilization Threshold (defined per BSS Load element section 9.4.2.28 of [1]).
    uint8_t    rcpi_steering_threshold;      // RCPI Steering Threshold
                                           // 0 – 220: RCPI threshold (encoded per Table 9-154 of [1]).
                                           // 221 – 255: Reserved
};

struct SteeringPolicyTLV
{
    uint8_t    tlv_type;                // Must always be set to
                                      // TLV_TYPE_STEERING_POLICY
    uint8_t    local_disallowed_sta_nr;      // Local Steering Disallowed STA count
                                           // Number of STA MAC addresses for which local steering is disallowed.
    uint8_t   (*local_disallowed_sta_mac)[6];// STA MAC address for which local steering is disallowed.
                                           // Not included if previous field is set to zero.

    uint8_t    btm_disallowed_sta_nr;        // BTM Steering Disallowed STA count.
                                           // Number of STA MAC addresses for which BTM steering is disallowed.
    uint8_t   (*btm_disallowed_sta_mac)[6];  // STA MAC address for which BTM steering is disallowed.
                                           // Not included if previous field is set to zero.
    uint8_t    radios_nr;                    // Number of radios for which control policy is being indicated.

    struct SteeringPolicy    *policies;              // Steering Policies for every radio
};

////////////////////////////////////////////////////////////////////////////////////////
/// \brief Metric Reporting TLV associated structures (Multi-AP "Section 17.2.12")
////////////////////////////////////////////////////////////////////////////////////////
#define MASK_ASSOCIATED_STA_TRAFFIC_STATS_INCLUSION_POLICY    0x80
#define MASK_ASSOCIATED_STA_LINK_METRICS_INCLUSION_POLICY     0x40

struct MetricReportingPolicy{
    uint8_t    radio_unique_id[6];                   // Radio unique identifier
    uint8_t    sta_rcpi_threshold;                   // STA Metrics Reporting RCPI Threshold.
    uint8_t    sta_rcpi_hysteresis_margin_override;  // STA Metrics Reporting RCPI Hysteresis Margin Override.
    uint8_t    ap_chaeel_utilization_threshold;      // AP Metrics Channel Utilization Reporting Threshold.
    uint8_t    policy;
};

struct MetricReportingPolicyTLV
{
    uint8_t                            tlv_type;    // Must always be set to
                                                  // TLV_TYPE_METRIC_REPORT_POLICY

    uint8_t							 report_interval; //Metrics Reporting Interval
    uint8_t                            radios_nr;   // Number of radios
    struct MetricReportingPolicy    *policies;    // Metric Reporting Policies for every radio
};

struct STAMacAddressTypeTLV
{
    uint8_t                       tlv_type;               // Must always be set to
                                                        // TLV_TYPE_STA_MAC_ADDRESS_TYPE
    uint8_t                       mac_address[6];         // MAC address of the associated STA.
};



#endif