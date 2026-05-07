/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _MULTI_AP_TLVS_H_
#define _MULTI_AP_TLVS_H_

#include "platform.h"

#if defined(_DRIVER_COMMON_STRUCTURE_)
#if defined(_BACKPORTS_DRIVER_)
#if defined(_USE_G6_DRIVER_)
#include "g6_wifi_driver/include/wifi_common.h"
#else
#include "wifi_common.h"
#endif
#else
#include "8192cd_common.h"
#endif
#endif

#ifdef CONFIG_RTK_PON_XDSL
#include "wifi_common.h"
#endif

////////////////////////////////////////////////////////////////////////////////
/// \brief TLV types as detailed in Multi-AP specification "Section 17.2"
////////////////////////////////////////////////////////////////////////////////
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

#define MASK_HAUL_BACKHAUL_STA     (0x80)
#define MASK_HAUL_BACKFAUL_BSS     (0x40)
#define MASK_HAUL_FRONTFAUL_BSS    (0x20)
#define MASK_HAUL_TEARDOWN         (0x10)

struct MultiAP_ExtensionSubelement
{
    INT8U subelement_id;             // multi-AP extension subelement identifier 0x06
    INT8U subelement_length;         // Number of Bytes in the subelement value
                                     // (subelement payload). 0x01

    INT8U haul;                // bit 7: backhaul STA  = 0x80
                                     // bit 6: backhaul BSS  = 0x40
                                     // bit 5: fronthaul BSS = 0x20
                                     // bit 4: tear down     = 0x10
                                     // bit 3-0 reserved
};

struct MultiAP_IE
{
    INT8U  element_id;                // always be 0xDD
                                      // IEEE 802.11 vendor specific information element
    INT8U *WIFI_ALLIANCE_OUI;         // pointer to WIFI_ALLIANCE_OUI
    INT8U  WIFI_ALLIANCE_OUI_TYPE;    // Wi-Fi Alliance specific OUI Type
                                      // identifying the type of the Multi-AP IE.
};

static const INT8U WIFI_ALLIANCE_OUI[3] = {0x50, 0x6F, 0x9A};
static const INT8U REALTEK_OUI[3]       = {0x00, 0xE0, 0x4C};

////////////////////////////////////////////////////////////////////////////////
/// \brief SupportedService TLV associated structures (Multi-AP "Section 17.2.1")
///        SearchedService TLV associated structures (Multi-AP "Section 17.2.2")
////////////////////////////////////////////////////////////////////////////////
#define MULTI_AP_CONTROLLER_SERVICE (0x00)
#define MULTI_AP_AGENT_SERVICE (0x01)
// 0x00 Multi-AP Controller
// 0x01 Multi-AP Agent
// 0x02~0xFF Reserved

struct SupportedServiceTLV
{
    INT8U  tlv_type;                  // Must always be set to
                                      // TLV_TYPE_SUPPORTED_SERVICE

    INT8U  supported_service_nr;        // The number of supported services
    INT8U  *supported_service;          // The list of supported services
                                        // length: supported_service_nr

};

struct SearchedServiceTLV
{
    INT8U   tlv_type;                  // Must always be set to
                                       // TLV_TYPE_SEARCHED_SERVICE

    INT8U   searched_service_nr;         // The number of searched services
    INT8U  *searched_service;            // The list of searched services
                                         // length: searched_service_nr
                                         // 0x00 Multi-AP Controller
                                         // 0x01~0xFF Reserved
};

////////////////////////////////////////////////////////////////////////////////
/// \brief AP Radio Identifier TLV associated structures (Multi-AP "Section 17.2.3")
////////////////////////////////////////////////////////////////////////////////

struct ApRadioIdentifierTLV
{
    INT8U   tlv_type;                    // Must always be set to
                                        // TLV_TYPE_AP_RADIO_IDENTIFIER

    INT8U   radio_unique_id[6];          // Radio Unique Identifier of a radio of the Multi-AP Agent.

};

////////////////////////////////////////////////////////////////////////////////
/// \brief AP Operational BSS TLV associated structures (Multi-AP "Section 17.2.4")
////////////////////////////////////////////////////////////////////////////////

struct BSS
{
    INT8U  mac_addr[6];
    INT8U  ssid_len;         // SSID length
    char  *ssid;             // This is the "friendly" name of the wifi network
};

struct RADIO
{
    INT8U       radio_unique_id[6];    // Radio Unique Identifier of a radio.
    INT8U       BSSs_nr;               // Number of BSS (802.11 Local interfaces) currently operating on the radio.
    struct BSS *BSSs;

};

struct ApOperationalBSSTLV
{
    INT8U         tlv_type;             // Must always be set to
                                        // TLV_TYPE_AP_OPERATIONAL_BSS

    INT8U         radios_nr;            // Number of radios reported.
    struct RADIO *radios;
};

////////////////////////////////////////////////////////////////////////////////
/// \brief Associated Clients TLV associated structures (Multi-AP "Section 17.2.5")
////////////////////////////////////////////////////////////////////////////////
struct Client
{
    INT8U    mac_addr[6];       // The MAC address of the associated 802.11 client.
    INT16U   left_time;         // Time since the 802.11 client’s last association to this Multi-AP device in
                                // seconds.
                                // 0x0000 – 0xFFFE: 0 - 65,534
                                // 0xFFFF: 65,535 or higher
};

struct BSSWithClients
{
    INT8U             bssid[6];           // Any EUI-48 value,
                                          // The BSSID of the BSS operated by the Multi-AP Agent in which the client is associated.
    INT16U            clients_nr;         // Number of clients associated to the BSS.
    struct Client    *clients;

};
struct AssociatedClientsTLV
{
    INT8U                    tlv_type;               // Must always be set to
                                                     // TLV_TYPE_ASSOCIATED_CLIENTS
    INT8U                    BSSs_nr;                // Number of BSSs included in this TLV.
    struct BSSWithClients   *BSSs;

};

////////////////////////////////////////////////////////////////////////////////
/// \brief AP Capability TLV associated structures (Multi-AP "Section 17.2.6")
////////////////////////////////////////////////////////////////////////////////

#define MASK_APCAPABILITY_METRICS_REPORTING_ON_BSS        (0x80)
#define MASK_APCAPABILITY_METRICS_REPORTING_NOT_ON_BSS    (0x40)
#define MASK_APCAPABILITY_SUPPORT_RCPI_BASED_STEERING     (0x20)

struct APCapabilityTLV
{
    INT8U   tlv_type;              // Must always be set to
                                   // TLV_TYPE_AP_CAPABILITY
    INT8U   ap_capability;
};

////////////////////////////////////////////////////////////////////////////////////////
/// \brief AP Radio Basic Capabilities TLV associated structures (Multi-AP "Section 17.2.7")
////////////////////////////////////////////////////////////////////////////////////////
struct OperatingClass
{
    INT8U   operating_class;            // Operating class per Table E-4 in [1], that this radio is capable of operating
                                        // on.

    INT8U   max_transmit_power;         // Maximum transmit power EIRP that this radio is capable of transmitting in
                                        // the current regulatory domain for the operating class.
                                        // The field is coded as a 2's complement signed integer in units of decibels
                                        // relative to 1 mW (dBm).

    INT8U    non_operable_channels_nr;  // Number of statically non-operable channels in the operating class.
                                        //    Other channels from this operating class which are not listed here are
                                        //    supported for the radio.

    INT8U   *non_operable_channels;     // Channel number which is statically non-operable in the operating class
                                        //    (i.e. the radio is never able to operate on this channel).
                                        //    This field is not present if non_operable_channels_nr = 0.
};

struct ChannelPreference
{
	INT8U	channel;
	INT8U	preference;
};

struct APRadioBasicCapabilitiesTLV
{
    INT8U                  tlv_type;                // Must always be set to
                                                    // TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES
    INT8U                  radio_unique_id[6];      // Radio Unique Identifier of a radio.
    INT8U                  max_BSSs_nr;             // Non-zero. Maximum number of BSSs supported by this radio.

    INT8U                  operating_classes_nr;       // Number of operating classes supported for the radio, defined per Table E-
                                                       // 4 in [1].
                                                       // All the supported operating classes are reported per regulatory
                                                       // restrictions.
    struct OperatingClass *operating_classes;
};

////////////////////////////////////////////////////////////////////////////////////////
/// \brief AP HT Capabilities TLV associated structures (Multi-AP "Section 17.2.8")
////////////////////////////////////////////////////////////////////////////////////////

#define MASK_APHTCAPABILITY_MAX_SUPPORTED_TX    (0xC0)
#define MASK_APHTCAPABILITY_MAX_SUPPORTED_RX    (0x30)
#define MASK_APHTCAPABILITY_SHORT_GI_20M        (0x08)
#define MASK_APHTCAPABILITY_SHORT_GI_40M        (0x04)
#define MASK_APHTCAPABILITY_HT_40M              (0x02)

struct APHTCapabilitiesTLV
{
    INT8U   tlv_type;                // Must always be set to
                                     // TLV_TYPE_AP_HT_CAPABILITIES
    INT8U   radio_unique_id[6];      // Radio unique identifier of the radio for which HT capabilities are reported.
    INT8U   ht_capability;
};

////////////////////////////////////////////////////////////////////////////////////////
/// \brief AP VHT Capabilities TLV associated structures (Multi-AP "Section 17.2.9")
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
    INT8U    tlv_type;                // Must always be set to
                                      // TLV_TYPE_AP_VHT_CAPABILITIES
    INT8U    radio_unique_id[6];      // Radio unique identifier of the radio for which VHT capabilities are reported.

    INT16U   vht_tx_msc;              // Supported VHT Tx MCS.
                                      // Supported set of VHT MCSs that can be received.
                                      // Set to Tx VHT MCS Map field per Figure 9-562 in [1].
    INT16U   vht_rx_msc;              // Supported VHT Rx MCS.
                                      // Supported set of VHT MCSs that can be transmitted.
                                      // Set to Rx VHT MCS Map field per Figure 9-562 in [1].
    INT8U    vht_capability_1;
    INT8U    vht_capability_2;
};

////////////////////////////////////////////////////////////////////////////////////////
/// \brief AP HE Capabilities TLV associated structures (Multi-AP "Section 17.2.10")
////////////////////////////////////////////////////////////////////////////////////////

#define MASK_HECAPABILITY_MAX_SUPPORTED_TX   (0xE0) // Maximum number of supported Tx spatial streams.
#define MASK_HECAPABILITY_MAX_SUPPORTED_RX   (0x1C) // Maximum number of supported Rx spatial streams.
#define MASK_HECAPABILITY_HE_SUPPORT_80_80   (0x02) // HE Support for 80+80 MHz.
#define MASK_HECAPABILITY_HE_SUPPORT_160M    (0x01) // HE Support for 160 MHz.

#define SHIFT_HECAPABILITY_MAX_SUPPORTED_TX (5)
#define SHIFT_HECAPABILITY_MAX_SUPPORTED_RX (2)

#define MASK_HECAPABILITY_SU_BEAMFORMER_CAPABLE       (0x80) // SU Beamformer Capable.
#define MASK_HECAPABILITY_MU_BEAMFORMER_CAPABLE       (0x40) // MU Beamformer Capable.
#define MASK_HECAPABILITY_UL_MU_MIMO_CAPABLE          (0x20) // UL MU-MIMO Capable.
#define MASK_HECAPABILITY_UL_MU_MIMO_OFDMA_CAPABLE    (0x10) // UL MU-MIMO + OFDMA Capable.
#define MASK_HECAPABILITY_DL_MU_MIMO_OFDMA_CAPABLE    (0x08) // DL MU-MIMO + OFDMA Capable.
#define MASK_HECAPABILITY_UL_OFDMA_CAPABLE            (0x04) // UL OFDMA Capable.
#define MASK_HECAPABILITY_DL_OFDMA_CAPABLE            (0x02) // DL OFDMA Capable.


// todo define HE MSC structure
struct APHECapabilitiesTLV
{
    INT8U   tlv_type;                // Must always be set to
                                     // TLV_TYPE_AP_HE_CAPABILITIES
    INT8U   radio_unique_id[6];      // Radio unique identifier of the radio for which HE capabilities are reported.

    INT8U    he_msc_len;              // Length of Supported HE MCS field.
    INT8U   *he_msc;                  // Supported HE MCS indicating set of supported HE Tx and Rx MCS.
                                      //    Set to Tx Rx HE MCS Support field from 802.11ax spec. Variable length
                                      //    from 2-12 bytes.

    INT8U   he_capability_1;
    INT8U   he_capability_2;
};


////////////////////////////////////////////////////////////////////////////////////////
/// \brief  Steering Policy TLV associated structures (Multi-AP "Section 17.2.11")
////////////////////////////////////////////////////////////////////////////////////////

#define AGENT_INITIATED_STEERING_DISALLOWED          0x00
#define AGENT_INITIATED_RCPI_BASED_STEERING_MANDATED 0x01
#define AGENT_INITIATED_RCPI_BASED_STEERING_ALLOWED  0x02

struct SteeringPolicy{
    INT8U    radio_unique_id[6];           // Radio unique identifier of an AP radio for which Multi-AP control policies are being provided.
    INT8U    policy;                       // Steering Policy
    INT8U    channel_util_threshold;       // Channel Utilization Threshold (defined per BSS Load element section 9.4.2.28 of [1]).
    INT8U    rcpi_steering_threshold;      // RCPI Steering Threshold
                                           // 0 – 220: RCPI threshold (encoded per Table 9-154 of [1]).
                                           // 221 – 255: Reserved
};

struct SteeringPolicyTLV
{
    INT8U    tlv_type;                // Must always be set to
                                      // TLV_TYPE_STEERING_POLICY
    INT8U    local_disallowed_sta_nr;      // Local Steering Disallowed STA count
                                           // Number of STA MAC addresses for which local steering is disallowed.
    INT8U   (*local_disallowed_sta_mac)[6];// STA MAC address for which local steering is disallowed.
                                           // Not included if previous field is set to zero.

    INT8U    btm_disallowed_sta_nr;        // BTM Steering Disallowed STA count.
                                           // Number of STA MAC addresses for which BTM steering is disallowed.
    INT8U   (*btm_disallowed_sta_mac)[6];  // STA MAC address for which BTM steering is disallowed.
                                           // Not included if previous field is set to zero.
    INT8U    radios_nr;                    // Number of radios for which control policy is being indicated.

    struct SteeringPolicy    *policies;              // Steering Policies for every radio
};

////////////////////////////////////////////////////////////////////////////////////////
/// \brief Metric Reporting TLV associated structures (Multi-AP "Section 17.2.12")
////////////////////////////////////////////////////////////////////////////////////////
#define MASK_ASSOCIATED_STA_TRAFFIC_STATS_INCLUSION_POLICY 0x80
#define MASK_ASSOCIATED_STA_LINK_METRICS_INCLUSION_POLICY 0x40
#define MASK_ASSOCIATED_WIFI6_STA_STATUS_REPORT_INCLUSION_POLICY 0x20

struct MetricReportingPolicy{
    INT8U    radio_unique_id[6];                   // Radio unique identifier
    INT8U    sta_rcpi_threshold;                   // STA Metrics Reporting RCPI Threshold.
    INT8U    sta_rcpi_hysteresis_margin_override;  // STA Metrics Reporting RCPI Hysteresis Margin Override.
    INT8U    ap_channel_utilization_threshold;      // AP Metrics Channel Utilization Reporting Threshold.
    INT8U    policy;
};

struct MetricReportingPolicyTLV
{
    INT8U                            tlv_type;    // Must always be set to
                                                  // TLV_TYPE_METRIC_REPORT_POLICY

    INT8U							 report_interval; //Metrics Reporting Interval
    INT8U                            radios_nr;   // Number of radios
    struct MetricReportingPolicy    *policies;    // Metric Reporting Policies for every radio
};


////////////////////////////////////////////////////////////////////////////////////////
/// \brief Client Info TLV associated structures (Multi-AP "Section 17.2.18")
////////////////////////////////////////////////////////////////////////////////////////

struct ClientInfoTLV
{
    INT8U   tlv_type;                // Must always be set to
                                     // TLV_TYPE_CLIENT_INFO
    INT8U   bssid[6];                // The BSSID of a BSS.
    INT8U   mac_address[6];
};

////////////////////////////////////////////////////////////////////////////////////////
/// \brief Client Capability Report TLV associated structures (Multi-AP "Section 17.2.19")
////////////////////////////////////////////////////////////////////////////////////////
#define CLIENT_CAPABILITY_REPORT_SUCCESS 0x00
#define CLIENT_CAPABILITY_REPORT_FAILURE 0x01
// todo define the frame body
struct ClientCapabilityReportTLV
{
    INT8U   tlv_type;               // Must always be set to
                                    // TLV_TYPE_CLIENT_CAPABLITY_REPORT
    INT8U   result_code;            // Result Code for the client capability report message.
                                    // 0x00: Success
                                    // 0x01:Failure
                                    // 0x02 – 0xFF: Reserved
    INT16U  frame_body_length;      // The length of the frame body
    INT8U   *frame_body;            // The frame body of the most recently received (Re)Association Request
                                    // frame from this client, as defined in Table 9-29 and Table 9-31 of [1]. If
                                    // Result Code is not equal to 0x00, this field is omitted.

};

////////////////////////////////////////////////////////////////////////////////////////
/// \brief Client Association Event TLV associated structures (Multi-AP "Section 17.2.20")
////////////////////////////////////////////////////////////////////////////////////////

#define CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_LEAVE 0x00
#define CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_JOIN 0x80
#define BTM_RESPONSE_EVENT 0x40

#define MASK_CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT   (0x80)
struct ClientAssociationEventTLV
{
    INT8U   tlv_type;               // Must always be set to
                                    // TLV_TYPE_CLIENT_ASSOCIATION_EVENT
    INT8U   mac_address[6];         // The MAC address of the client.
    INT8U   bssid[6];               // The BSSID of the BSS operated by the Multi-AP Agent for which the event
                                    // has occurred.
    INT8U   event;

};

////////////////////////////////////////////////////////////////////////////////////////
/// \brief STA MAC Address Type TLV associated structures (Multi-AP "Section 17.2.23")
////////////////////////////////////////////////////////////////////////////////////////

struct STAMacAddressTypeTLV
{
    INT8U                       tlv_type;               // Must always be set to
                                                        // TLV_TYPE_STA_MAC_ADDRESS_TYPE
    INT8U                       mac_address[6];         // MAC address of the associated STA.
};

////////////////////////////////////////////////////////////////////////////////////////
/// \brief Client Association Control Request TLV associated structures (Multi-AP "Section 17.2.31")
////////////////////////////////////////////////////////////////////////////////////////
#define ASSOCIATION_REQUEST_BLOCK   (0x00)
#define ASSOCIATION_REQUEST_UNBLOCK (0x01)
struct ClientAssociationControlRequestTLV
{
    INT8U   tlv_type;               // Must always be set to
                                    // TLV_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST
    INT8U   bssid[6];               // BSSID - unique identifier of the BSS
                                    // for which the client blocking request applies.
    INT8U   association_control;    // Indicates if the request is to block or
                                    // unblock the indicated STAs from associating.
                                    // 0x00: Block
                                    // 0x01: Unblock
                                    // 0x02-0xFF: Reserved
    INT16U  validity_period;        // Time period in seconds (from reception of the Client Association Control
                                    // Request message) for which a blocking request is valid.
    INT8U   sta_nr;                 // STA List Count Indicating one or more STA(s) for which Client Association
                                    // Control request applies.
    INT8U   (*sta_mac_address)[6];  // STA MAC address for which the Client Association Control request applies.
};

////////////////////////////////////////////////////////////////////////////////////////
/// \brief  Higher Layer Data TLV associated structures (Multi-AP "Section 17.2.34")
////////////////////////////////////////////////////////////////////////////////////////

struct HigherLayerDataTLV
{
    INT8U   tlv_type;               // Must always be set to
                                    // TLV_TYPE_HIGHER_LAYER_DATA
    INT8U   protocol;               // Higher layer protocol (see Appendix A.1).

    INT8U  *payload;                // Higher layer protocol payload (To be defined for specific higher layer
                                    // protocol).

};


////////////////////////////////////////////////////////////////////////////////////////
/// \brief Error Code TLV associated structures (Multi-AP "Section 17.2.36")
////////////////////////////////////////////////////////////////////////////////////////
#define REASON_CODE_STA_ASSOC                             (0x01)
#define REASON_CODE_STA_NOT_ASSOC                         (0x02)
#define REASON_CODE_CLIENT_CAP_RPT_FAIL                   (0x03)
#define REASON_CODE_BH_STA_CANNOT_OPERATE_ON_CH_SPECIFIED (0x04)
#define REASON_CODE_TARGET_BSS_TOO_WEAK_OR_NOT_FOUND      (0x05)
#define REASON_CODE_AUTH_OR_ASSOC_REJECTED_BY_TARGET_BSS  (0x06)

struct ErrorCodeTLV
{
    INT8U   tlv_type;               // Must always be set to
                                    // TLV_TYPE_ERROR_CODE
    INT8U   reason_code;            // 0x00: Reserved Reason code.
                                    //    0x01: STA associated with a BSS operated by the Agent.
                                    //    0x02: STA not associated with any BSS operated by the Agent.
                                    //    0x03: Client capability report unspecified failure
                                    //    0x04: Backhaul steering request rejected because the backhaul station cannot operate on the                                 channel specified.
                                    //    0x05: Backhaul steering request rejected because the target BSS signal is too weak or not found.
                                    //    0x06: Backhaul steering request authentication or association Rejected by the target BSS.
                                    //    0x07 – 0xFF: Reserved


    INT8U   sta_mac_address[6];     // The MAC address of the STA referred to in the previous field.
                                    // The value of this field is valid only if the reason code field is set to 0x01 or
                                    // 0x02. Otherwise this field is ignored by the recipient of this TLV.
};

struct OperationalBackhaulBSSEntry
{
    INT8U   bssid[6];

    INT8U   bss_configuration_mask;
};

struct OperationalBackhaulBSSInfo
{
    INT8U    radio_id[6];

    INT8U   backhaul_bss_nr;

    struct OperationalBackhaulBSSEntry *operationalBackhaulBSSEntry;
};

struct APOperationalBackhaulBSSTLV
{
    INT8U   tlv_type;

    INT8U   radio_nr;

    struct  OperationalBackhaulBSSInfo *operationalBackhaulBSSInfo;
};

struct ChannelPreferencePerOpClass
{
	INT8U	op_class;
	INT8U	channel_nr;				// Number of channels specified in the Channel List.
	INT8U	*channel_list;			// Each octet describes a single channel number in the Operating Class.
									// An empty Channel List field (channel_nr = 0) indicates that the indicated Preference applies to all channels in the Operating Class.
	INT8U	preference_reason_code;	// bits 7 - 4: Preference -- Indicates a preference value for the channels in the Channel List
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
                                    //		1011: Controller DFS Channel Clear Indication (Can only be specified by the Multi-AP Controller
									//		1110-1111: Reserved
};

struct ChannelPreferenceTLV
{
	INT8U	tlv_type;											// Must always be set to
																// TLV_TYPE_CHANNEL_PREFERENCE
	INT8U	radio_unique_id[6];									// Radio Unique identifier of a radio for which preferences are reported in this TLV.
	INT8U	op_class_nr;										// Number of operating classes for which preferences are reported in this TLV.
	struct ChannelPreferencePerOpClass	*channel_preferences;	// Channel preferences content
};

struct RadioOperationRestrictionPerChannel
{
	INT8U	channel;				// Channel number for which a restriction applies
	INT8U	min_freq_separation;	// The minimum frequency separation (in multiples of 10 MHz) that this radio would require when operating on the above channel number between the center frequency of that channel and the center operating frequency of another radio (operating simultaneous TX/RX) of the Multi-AP Agent
};

struct RadioOperationRestrictionPerOpClass
{
	INT8U	op_class;															// Operating Class
	INT8U	channel_nr;															// Number of channels specified
	struct RadioOperationRestrictionPerChannel *channel_operation_restriction;	// Channel operation restriction contect
};

struct RadioOperationRestrictionTLV
{
	INT8U	tlv_type;														// Must always be set to
																			// TLV_TYPE_RADIO_OPERATION_RESTRICTION
	INT8U	radio_unique_id[6];												// Radio Unique identifier of a radio
	INT8U	op_class_nr;													// Number of Operating Classes for which restrictions are reported in this TLV
	struct RadioOperationRestrictionPerOpClass *radio_operation_restriction;// Radio operation restriction content
};

struct TransmitPowerLimitTLV
{
	INT8U	tlv_type;														// Must always be set to
																			// TLV_TYPE_TRANSMIT_POWER_LIMIT
	INT8U	radio_unique_id[6];												// Radio Unique identifier
	INT8U	transmit_power_limit;											// Transmit Power Limit EIRP per 20 MHz bandwidth representing the nominal transimit power limit for this radio.
																			// The field is coded as a 2's complement signed integer in units of decibels relative to 1 mW(dBm)
};

struct ChannelSelectionResponseTLV
{
	INT8U	tlv_type;														// Must always be set to
																			// TLV_TYPE_CHANNEL_SELECTION_RESPONSE
	INT8U	radio_unique_id[6];												// Radio Unique identifier
	INT8U	response;														// Indicates the channel selection response code, with respect to the Channel Selection Request
																			// 0x00: Accept
																			// 0x01: Decline because request violates current preferences which have changed since last reported
																			// 0x02: Decline because request violates most recently reported preferences
																			// 0x03: Decline because request would prevent operation of a currently operating backhaul link (where backhaul STA and BSS share a radio)
																			// 0x04-0xFF: Reserved
};

struct OperatingChannelReportPerOpClass
{
	INT8U	op_class;														// Operating Class
	INT8U	cur_channel;													// Current operating channel number in the Operating Class
};

struct APMetricQueryTLV
{
    INT8U   tlv_type;               // Must always be set to
                                    // AP_METRIC_QUERY
    INT8U   bssid_nr;            	// Number of BSSIDs included in this TLV

	INT8U	(*bssid)[6];			// The BSSID of a BSS by the MAP device for which the metrics are to be reported.

};

struct APMetricsTLV
{
    INT8U   tlv_type;               // Must always be set to
                                    // TLV_TYPE_AP_METRICS
    INT8U   bssid[6];            	// Number of BSSIDs included in this TLV

    INT8U   ch_util;    			// Channel Utilization aas measured by the radio operating the BSS

    INT16U	sta_nr;					// Total number of STAs currently associated with this BSS

	INT8U	esp_ie;					//Estimated Service Parameters Information Field bit inclusion
									//Bit 7: AC = BE
									//Bit 6: AC = BK
									//Bit 5: AC = VO
									//Bit 4: AC = VI
									//Bit 3 -0 : Reserved

	INT8U	esp_acbe[3];			//ESP IE AC=BE
									//Data Format, BA Window Size, Estimated Air Time Fraction and Data PPDU Target Duration

	INT8U	esp_acbk[3];			//ESP IE AC=BK
									//Data Format, BA Window Size, Estimated Air Time Fraction and Data PPDU Target Duration

	INT8U	esp_acvo[3];			//ESP IE AC=VO
									//Data Format, BA Window Size, Estimated Air Time Fraction and Data PPDU Target Duration

	INT8U	esp_acvi[3];			//ESP IE AC=VI
									//Data Format, BA Window Size, Estimated Air Time Fraction and Data PPDU Target Duration
};

struct AssocSTALinkMetrics
{
	INT8U	bssid[6];

	INT32U	time_delta;

	INT32U	dataRate_downlink;

	INT32U	dataRate_uplink;

	INT8U	uplink_rcpi; //rcpi
};

struct AssociatedSTALinkMetricsTLV
{
	INT8U   tlv_type;

	INT8U	mac_address[6];

	INT8U	bssid_nr;

	struct AssocSTALinkMetrics *assoc_sta_link_metrics;
};

struct ChannelListInfo
{
	INT8U	channel_nr;

	INT8U	sta_nr;

	INT8U	(*sta_mac_address)[6];
};

struct UnassociatedSTALinkMetricsQueryTLV
{
	INT8U   tlv_type;

	INT8U	op_class;

	INT8U	channel_list_nr;

	struct	ChannelListInfo *channel_list_entry;

};

struct UnassocSTALinkMetrics
{
	INT8U	sta_mac_address[6];

	INT8U	channel_number;

	INT32U	time_delta;

	INT8U	uplink_rcpi;
};

struct UnassociatedSTALinkMetricsResponseTLV
{
	INT8U	tlv_type;

	INT8U	op_class;

	INT8U	sta_nr;

	struct UnassocSTALinkMetrics *unassoc_sta_link_metrics;
};

struct ApChannelReport
{
	INT8U	len;

	INT8U	op_class;

	INT8U	*channel_list;
};

#define MACADDRLEN					6
//#define MAX_SSID_LEN			    32
#define MAX_BEACON_SUBLEMENT_LEN           226
#define MAX_REQUEST_IE_LEN          16
#define MAX_AP_CHANNEL_REPORT       4
#define MAX_AP_CHANNEL_NUM          8

#pragma pack(1)

#if !defined(_DRIVER_COMMON_STRUCTURE_) && !defined(CONFIG_RTK_PON_XDSL)
struct dot11k_ap_channel_report
{
	INT8U len;
	INT8U op_class;
	INT8U channel[MAX_AP_CHANNEL_NUM];
};
#endif

struct BeaconMeasurementRequest
{
	INT8U                           op_class;
	INT8U                           channel;
	INT16U                          random_interval;
	INT16U                          measure_duration;
	INT8U                           mode;
	INT8U                           bssid[MACADDRLEN];
	char                            ssid[32 + 1];
	INT8U                           report_detail; // 0: no-fixed len field and element,
	                                               // 1: all fixed len field and elements in Request ie,
	                                               // 2: all fixed len field and elements (default)
	INT8U                           request_ie_len;
	INT8U                           request_ie[MAX_REQUEST_IE_LEN];
	struct dot11k_ap_channel_report ap_channel_report[MAX_AP_CHANNEL_REPORT];
};

struct OperatingChannelReportTLV
{
	INT8U	tlv_type;														// Must always be set to
																			// TLV_TYPE_OPERATING_CHANNEL_REPORT
	INT8U	radio_unique_id[6];												// Radio Unique identifier
	INT8U	cur_op_class_nr;												// Number of current operating classes
	struct OperatingChannelReportPerOpClass *operating_channels;			// Operating channels information
	INT8U	cur_transmit_power;												// Current Transmit Power EIRP representing the current nominal transmit power.
																			// This field is coded as a 2's complement signed integer in units of decibels relative to 1mW(dBm).
																			// This value is less than or equal to the Maximum Transmit Power specified in the AP Radio Basic Capabilities TLV for the current operating class.
};

struct TargetBSS
{
	INT8U	bssid[6];
	INT8U	opclass;
	INT8U	channel;
};

struct SteeringRequestTLV
{
	INT8U	tlv_type;				// Must always be set to
									// TLV_TYPE_STEERING_REQUEST
	INT8U   bssid[6];               // BSSID - unique identifier of the BSS
                                    // for which the steering request applies.
    INT8U	mode;					// bit 7: Request Mode -- 0: Steering Opportunity 1: Steering Mandate
									// bit 6: BTM Disassociation Imminent Bit
									// bit 5: BTM Abridged bit
									// bit 4 - 0: Reserved
    INT16U	window;					// Steering Opportunity window (seconds)
    INT16U	disassociation_timer;	// BTM Disassociation Timer (TUs)
    INT8U	sta_nr;					// STA List Count
    INT8U   (*sta_mac_address)[6];	// STA MAC address for which the steering request applies
    INT8U	target_bss_nr;			// Target BSSID List Count
    struct TargetBSS *target_bss;	// Target BSS info
};

struct SteeringBTMReportTLV
{
	INT8U	tlv_type;				// Must always be set to
									// TLV_TYPE_STEERING_BTM_REPORT
	INT8U	bssid[6];				// BSSID - unique identifier of the BSS
	INT8U	sta_mac_address[6];		// STA MAC address for which the steering BTM report applies
	INT8U	status_code;			// BTM Status Code
	INT8U	target_bssid[6];		// Target BSSID
};

#define MACADDRLEN				6

#if !defined(_DRIVER_COMMON_STRUCTURE_) && !defined(CONFIG_RTK_PON_XDSL)
struct target_transition_list {
	INT8U  addr[MACADDRLEN];
	INT8U  mode; // bit 7 -- request mode; bit 6 -- disassoc imminent bit; bit 5 -- abridged bit
	INT16U disassoc_timer;
	INT8U  target_bssid[MACADDRLEN];
	INT8U  opclass;
	INT8U  channel;
	INT8U  reason_code;
	//INT8U	disassoc;
};
#endif

struct bss_transition_response_para {
	INT8U addr[MACADDRLEN];
	INT8U bssid[MACADDRLEN];
	INT8U target_bssid[MACADDRLEN];
	INT8U status_code;
};

struct assoc_control_para {
	INT8U  addr[MACADDRLEN];
	INT16U period;
	INT8U  control; // 0x00 -- lock; 0x01 -- unlock
};

#define BEACON_REPORT_FIXED_IE 29
#define MEASUREMENT_REPORT_ELEMENT_ID 39

struct BeaconMeasurementReportInfo
{
    INT8U op_class;
    INT8U channel;
    INT32U  measure_time_hi;
    INT32U  measure_time_lo;
    INT16U measure_duration;
    INT8U frame_info;
    INT8U RCPI;
    INT8U RSNI;
    INT8U bssid[6];
    INT8U antenna_id;
    INT32U  parent_tsf;
};

struct BeaconMeasurementReportIE
{
	INT8U elementId;
	INT8U len;
	INT8U measurementToken;
	INT8U measurementReportMode;
	INT8U measurementType;
    struct BeaconMeasurementReportInfo info;
	INT8U subelement_id;
    INT8U subelements_len;
    INT8U *subelements;
    INT8U subelements_remain_len;
    INT8U subelements_remain[64];
};

#pragma pack(0)

struct BeaconMetricsQueryTLV
{
	INT8U	tlv_type;

	INT8U	mac_address[6];

	INT8U	op_class;

	INT8U	channel_number;

	INT8U	bssid[6];

	INT8U	report_detail;

	INT8U	ssid_len;

	INT8U	*ssid;

	INT8U	ap_channel_report_nr;

	struct ApChannelReport *ap_channel_report;

	INT8U	elementID_nr;

	INT8U	*element_list;

};

struct BeaconMetricsResponseTLV
{
	INT8U	tlv_type;

	INT8U	mac_address[6];

	INT8U	reserved_field;

	INT8U	measurement_report_nr;

	struct	BeaconMeasurementReportIE *measurement_reports;
};

struct BackhaulSteeringRequestTLV{

	INT8U	tlv_type;

	INT8U	backhaul_mac[6];

	INT8U	target_bssid[6];

	INT8U	op_class;

	INT8U	channel_number;

};

struct BackhaulSteeringResponseTLV{

	INT8U	tlv_type;

	INT8U	backhaul_mac[6];

	INT8U	target_bssid[6];

	INT8U	result_code;
};

struct AssociatedSTATrafficStatsTLV
{
	INT8U	tlv_type;

	INT8U	sta_mac_address[6];

	INT32U	bytes_sent;

	INT32U	bytes_received;

	INT32U	packets_sent;

	INT32U	packets_received;

	INT32U	tx_packets_errors;

	INT32U	rx_packets_errors;

	INT32U	retransmission_count;
};

////////////////////////////////////////////////////////////////////////////////
// Main API functions
////////////////////////////////////////////////////////////////////////////////

// This function receives a pointer to a stream of bytes representing a BBF TLV.
//
// It then returns a pointer to a structure whose fields have already been
// filled with the appropriate values extracted from the parsed stream.
//
// The actual type of the returned pointer structure depends on the value of
// the first byte pointed by "packet_stream" (ie. the "Type" field of the TLV):
//
//   TLV_TYPE_NON_1905_LINK_METRIC_QUERY        -->  struct linkMetricQueryTLV *
//   TLV_TYPE_NON_1905_TRANSMITTER_LINK_METRIC  -->  struct transmitterLinkMetricTLV *
//   TLV_TYPE_NON_1905_RECEIVER_LINK_METRIC     -->  struct receiverLinkMetricTLV *
//   TLV_TYPE_NON_1905_LINK_METRIC_RESULT_CODE  -->  struct linkMetricResultCodeTLV *
//
//
// If an error was encountered while parsing the stream, a NULL pointer is
// returned instead.
// Otherwise, the returned structure is dynamically allocated, and once it is
// no longer needed, the user must call the "free_bbf_TLV_structure()"
//
INT8U *parse_multi_ap_TLV_from_packet(INT8U *packet_stream);


 // This is the opposite of "parse_multi_ap_TLV_from_packet()": it receives a
 // pointer to a TLV structure and then returns a pointer to a buffer which:
 //   - is a packet representation of the TLV
 //   - has a length equal to the value returned in the "len" output argument
 //
 // "memory_structure" must point to a structure of one of the types returned by
 // "parse_multi_ap_TLV_from_packet()"
 //
 // If there is a problem this function returns NULL, otherwise the returned
 // buffer must be later freed by the caller (it is a regular, non-nested
 // buffer, so you just need to call "free()").
 //
 // Note that the input structure is *not* freed. You still need to later call
 // "free_bbf_TLV_structure()"
 //
INT8U *forge_multi_ap_TLV_from_structure(INT8U *memory_structure, INT16U *len);



////////////////////////////////////////////////////////////////////////////////
// Utility API functions
////////////////////////////////////////////////////////////////////////////////

// This function receives a pointer to a TLV structure and then traverses it
// and all nested structures, calling "free()" on each one of them
//
// "memory_structure" must point to a structure of one of the types returned by
// "parse_multi_ap_TLV_from_packet()"
//
void free_multi_ap_TLV_structure(INT8U *memory_structure);


// This function returns '0' if the two given pointers represent TLV structures
// of the same type and they contain the same data
//
// "memory_structure_1" and "memory_structure_2" must point (each) to a
// structure of one of the types returned by "parse_multi_ap_TLV_from_packet()"
//
INT8U compare_multi_ap_TLV_structures(INT8U *memory_structure_1, INT8U *memory_structure_2);


// The next function is used to call function "callback()" on each element of
// the "memory_structure" structure
//
// "memory_structure" must point to a structure of one of the types returned by
// "parse_multi_ap_TLV_from_packet()"
//
// It takes four arguments:
//   - The structure whose elements are going to be visited
//   - A callback function that will be executed on each element with the
//     following arguments:
//      * A pointer to the "write()" function that will be used to dump text.
//        This is always the "write_function()" pointer provided as third
//        argument to the "visit_bbf_TLV_structure()" function.
//      * The size of the element to print (1, 2, 4, n bytes)
//      * A prefix string.
//        This is always the "prefix" value provided as fourth argument to the
//        "visit_bbf_TLV_structure()" function
//      * The name of the element (ex: "mac_address")
//      * A 'fmt' string which must be used to print the contents of the element
//      * A pointer to the element itself
//   - The "write()" function that will be used when the callback is executed
//   - A "prefix" string argument that will be used when the callback is
//     executed (it usually contains "context" information that the callback
//     function prints before anything else to make it easy to follow the
//     structure traversing order)
//
void visit_multi_ap_TLV_structure(INT8U *memory_structure, void (*callback)(void (*write_function)(const char *fmt, ...), const char *prefix, INT8U size, const char *name, const char *fmt, void *p), void (*write_function)(const char *fmt, ...), const char *prefix);


// Use this function for debug purposes. It turns a TLV_TYPE_* variable into its
// string representation.
//
// Example: TLV_TYPE_AL_MAC_ADDRESS_TYPE --> "TLV_TYPE_AL_MAC_ADDRESS_TYPE"
//
// Return "Unknown" if the provided type does not exist.
//
char *convert_multi_ap_TLV_type_to_string(INT8U tlv_type);

#endif // END _MULTI_AP_TLVS_H_
