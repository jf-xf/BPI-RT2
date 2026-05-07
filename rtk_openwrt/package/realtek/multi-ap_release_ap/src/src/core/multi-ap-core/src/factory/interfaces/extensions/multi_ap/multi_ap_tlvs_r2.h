/*
 *  Realtek Semiconductor Corp.
 *
 */

#ifndef _MULTI_AP_TLVS_R2_H_
#define _MULTI_AP_TLVS_R2_H_

#include "platform.h"

////////////////////////////////////////////////////////////////////////////////
/// Profile 2 TLVs
////////////////////////////////////////////////////////////////////////////////
#define TLV_TYPE_CHANNEL_SCAN_REPORTING_POLICY                     (164) // 0xA4
#define TLV_TYPE_CHANNEL_SCAN_CAPABILITIES                         (165) // 0xA5
#define TLV_TYPE_CHANNEL_SCAN_REQUEST                              (166) // 0xA6
#define TLV_TYPE_CHANNEL_SCAN_RESULT                               (167) // 0xA7
#define TLV_TYPE_TIMESTAMP                                         (168) // 0xA8
#define TLV_TYPE_MIC                                               (171) // 0xAB
#define TLV_TYPE_CAC_REQUEST                                       (173) // 0xAD
#define TLV_TYPE_CAC_TERMINATION                                   (174) // 0xAE
#define TLV_TYPE_CAC_COMPLETION_REPORT                             (175) // 0xAF
#define TLV_TYPE_CAC_STATUS_REPORT                                 (177) // 0xB1
#define TLV_TYPE_CAC_CAPABILITIES                                  (178) // 0xB2
#define TLV_TYPE_MULTIAP_PROFILE                                   (179) // 0xB3
#define TLV_TYPE_PROFILE_2_CAPABILITY                              (180) // 0xB4
#define TLV_TYPE_DEFAULT_802_1Q_SETTINGS                           (181) // 0xB5
#define TLV_TYPE_TRAFFIC_SEPARATION_POLICY                         (182) // 0xB6
#define TLV_TYPE_PROFILE_2_ERROR_CODE                              (188) // 0xBC
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
#define TLV_TYPE_BACKHAUL_BSS_CONFIGURATION                        (208) // 0xD0

////////////////////////////////////////////////////////////////////////////////
/// Profile 2 TLVs
////////////////////////////////////////////////////////////////////////////////


#define INDEPENDENT_SCAN_FLAG_MASK           0x80 //1000 0000
#define REPORT_INDEPENDENT_SCAN_TRUE         1
#define REPORT_INDEPENDENT_SCAN_FALSE        0
struct ChannelScanReportingPolicyTLV
{
    INT8U	tlv_type;
    INT8U	independent_report_flag;
};

struct ChannelScanCapabilitiesOpClass
{
    INT8U	op_class;
    INT8U	channel_nr;
    INT8U*	channel_list;
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
    INT8U	radio_unique_identifier[6];
    INT8U	scan_capability_flags;
    INT32U	minimum_scan_interval;
    INT8U	op_class_nr;
    struct ChannelScanCapabilitiesOpClass* op_classes;
};

struct ChannelScanCapabilitiesTLV
{
    INT8U	tlv_type;
    INT8U	radio_nr;
    struct ChannelScanCapabilitiesRadio* radios;
};

struct ChannelScanTargetOpClass
{
    INT8U	op_class;
    INT8U	target_channel_nr;
    INT8U*	target_channel_list;
};

struct ChannelScanTargetRadio
{
    INT8U	radio_unique_identifier[6];
    INT8U	target_op_class_nr;
    struct  ChannelScanTargetOpClass* target_op_classes;
};

#define PERFORM_FRESH_SCAN_MASK   0x80   //1000 0000
#define PERFORM_FRESH_SCAN_TRUE   1
#define PERFORM_FRESH_SCAN_FALSE  0
struct ChannelScanRequestTLV
{
	INT8U	tlv_type;
	INT8U	flags;  //bit 7 - 1: Perform a fresh scan and return results 0: Return stored results of last successful scan     bits 6-0 - reserverd
	INT8U	target_radio_nr;
    struct  ChannelScanTargetRadio* target_radios;
};

#define CHANNELUTLIZATION_PRESENT_MASK   0x80   //1000 0000
#define STATIONCOUNT_PRESENT_MASK        0x40   //0100 0000
struct ChannelScanNeighbor
{
    INT8U   bssid[6];
    INT8U   ssid_length;
    char*   ssid;
    INT8U   signal_strength;
    INT8U   channel_bandwidth_length;
    char*   channel_bandwidth;
    INT8U   field_present_flags;
    INT8U   channel_utilization;
    INT16U  station_count;
};

struct ChannelScanResultTLV
{
    INT8U	tlv_type;
    INT8U	radio_unique_identifier[6];
    INT8U	op_class;
    INT8U	channel;
    INT8U	scan_status;
    //The following fields are only present if Scan Status is set to 0x00
    INT8U	timestamp_length;
    char*   timestamp;
    INT8U   channel_utilization;
    INT8U   noise;
    INT16U  neighbor_nr;
    struct  ChannelScanNeighbor* neighbors;
    INT32U  aggregate_scan_duration;
    INT8U   scan_type_flags; //bit7 - 1: Scan was an Active scan 0: Scan was Passive scan

};

struct TimestampTLV
{
    INT8U	tlv_type;
    INT8U   timestamp_length;     //length of the timestamp string
    char*   timestamp;
};

struct CACRequestRadio
{
    INT8U   radio_unique_identifier[6];
    INT8U   op_class;
    INT8U   channel;
    INT8U   flags;  //Bit 7-5 CAC method to be used.(0: Continuous CAC, 1: Continuous with dedicated radio, 2: MIMO dimension reduced, 3: Time sliced CAC, >3: Reserved)
                    //Bit 4-3 CAC Completion Action for Successful CAC.(0: Remain on channel and continue to monitor for radar, 1: Return the radio that was performing the CAC to its most recent operational configuration., >1: Reserved)
                    //Bits 2-0 (Reserved)
};

struct CACRequestTLV
{
    INT8U   tlv_type;
    INT8U   radio_nr;
    struct CACRequestRadio* radios;
};

struct CACTerminationRadio
{
    INT8U   radio_unique_identifier[6];
    INT8U   op_class;
    INT8U   channel;
};

struct CACTerminationTLV
{
    INT8U   tlv_type;
    INT8U   radio_nr;
    struct CACTerminationRadio* radios;
};

struct CACCompletionReportClassChannelPairs
{
    INT8U pairs_op_class;
    INT8U pairs_channel;
};

struct CACCompletionReportRadio
{
    INT8U   radio_unique_identifier[6];
    INT8U   op_class;
    INT8U   channel;
    INT8U   flags;  //CAC method to be used.
                    //(0: Successful, 1: Radar detected, 2: CAC not supported as requested (capability mismatch), 3: Radio too busy to perform CAC, 4: Request was considered to be non-conformant to regulations in the country in which the Multi-AP Agent is operating, 5: Other error, >5: Reserved)
    INT8U   pairs_nr;
    struct CACCompletionReportClassChannelPairs* pairs;
};

struct CACCompletionReportTLV
{
    INT8U   tlv_type;
    INT8U   radio_nr;
    struct CACCompletionReportRadio* radios;
};

struct CACCompletionReportActiveClassChannelPairs
{
    INT8U   active_op_class;
    INT8U   active_channel;
    INT8U   active_remaining_time[3];  //Seconds

};

struct CACCompletionReportNonOccupancyClassChannelPairs
{
    INT8U   nonoccup_op_class;
    INT8U   nonoccup_channel;
    INT16U  nonoccup_remaining_time;  //Seconds

};

struct CACCompletionReportAvailabelChannels
{
    INT8U   ac_op_class;
    INT8U   ac_channel;
    INT16U  identify_time;  //minutes
                            //Set to zero for non-DFS channels.
};

struct CACStatusReportTLV
{
    INT8U   tlv_type;
    INT8U   available_channel_nr;
    struct  CACCompletionReportAvailabelChannels* available_channels;
    INT8U   nonoccup_pair_nr;
    struct  CACCompletionReportNonOccupancyClassChannelPairs* nonoccup_pairs;
    INT8U   active_pair_nr;
    struct  CACCompletionReportActiveClassChannelPairs* active_pairs;
};

struct CACCapabilitiesOpClass
{
    INT8U   op_class;
    INT8U   channel_nr;
    INT8U*  channels;
};

struct CACCapabilitiesType
{
    INT8U   flags; //CAC mode supported (0: Continuous CAC, 1: Continuous with dedicated radio, 2: MIMO dimension reduced, 3: Time sliced CAC, >3: Reserved)
    INT8U   second_nr[3];
    INT8U   op_class_nr;
    struct  CACCapabilitiesOpClass* op_classes;
};

struct CACCapabilitiesRadio
{
    INT8U   radio_unique_identifier[6];
    INT8U   type_nr;
    struct  CACCapabilitiesType* types;
};

struct CACCapabilitiesTLV
{
    INT8U   tlv_type;
    char    country_code[2];
    INT8U   radio_nr;
    struct  CACCapabilitiesRadio* radios;
};

#define MULTI_AP_PROFILE_1  0x01
#define MULTI_AP_PROFILE_2  0x02
#define MULTI_AP_PROFILE_3  0x03
#define MULTI_AP_PROFILE_4  0x04

struct MultiAPProfileTLV
{
	INT8U	tlv_type;

	INT8U	profile;
};

#define BYTE_COUNTER_UNITS_MASK 0xC0
#define BYTE_COUNTER_UNITS_BYTE 0b00
#define BYTE_COUNTER_UNITS_KILOBYTE 0b01
#define BYTE_COUNTER_UNITS_MEGABYTE 0b10
#define SERVICE_PRIORITIZATION_BIT_MASK 0x20
struct Profile2APCapabilityTLV
{
    INT8U   tlv_type;

    INT8U   max_total_nr_service_prioritization_rules;

    INT8U   reserved;

    INT8U   units;  // Byte Counter Units - The units used for byte counters when the Agent reports traffic statistics.
                    // bits 7-6 : 0: bytes; 1: kibibytes (KiB); 2: mebibytes (MiB); 3: reserved
                    // bits 5   : Service Prioritization (0 or 1)
                    // bits 4-0 : Reserved

    INT8U   max_total_nr_VIDs;
};

struct Default8021QSettingsTLV
{
    INT8U   tlv_type;

    INT16U  primary_vlan_id;

    INT8U   default_pcp;
};

struct TrafficSeparationSsidInfo
{
    INT8U   ssid_length;

    char   *ssid_name;

    INT16U  vlan_id;
};

struct TrafficSeparationPolicyTLV
{
    INT8U   tlv_type;

    INT8U   ssid_nr;

    struct TrafficSeparationSsidInfo   *ssid_info;
};

struct Profile2ErrorCodeTLV
{
    INT8U   tlv_type;                           // Must always be set to
												// TLV_TYPE_PROFILE_2_ERROR_CODE

    INT8U   reason_code;

    INT32U  service_prioritization_rule_id;
};

struct APRadioAdvancedCapabilitiesTLV
{
    INT8U   tlv_type;                           // Must always be set to
							            		// TLV_TYPE_AP_RADIO_ADVANCED_CAPABILITIES

    INT8U   radio_id[6];

    INT8U   backhaul_bss_traffic_separation_mix_no_support;
};

struct AssociationStatusNotificationBssInfo
{
    INT8U   bssid[6];

    INT8U   association_allowance_status;
};

struct AssociationStatusNotificationTLV
{
    INT8U   tlv_type;                           // Must always be set to
							            		// TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION

    INT8U   bssid_nr;

    struct AssociationStatusNotificationBssInfo *bss_info;
};

struct SourceInfoTLV
{
    INT8U   tlv_type;                           // Must always be set to
							            		// TLV_TYPE_SOURCE_INFO

    INT8U   mac_address[6];
};

struct TunneledMessageTypeTLV
{
    INT8U   tlv_type;                           // Must always be set to
							            		// TLV_TYPE_TUNNELED_MESSAGE_TYPE

    INT8U   tunneled_protocol_type_payload;
};

struct TunneledTLV
{
    INT8U   tlv_type;                           // Must always be set to
							            		// TLV_TYPE_TUNNELED

    INT16U  tlv_length;

    INT8U   *payload;
};

struct Profile2SteeringRequestTargetBssidInfo
{
    INT8U   target_bssid[6];

    INT8U   target_bss_operating_class;

    INT8U   target_bss_channel_number;

    INT8U   reason_code;
};

struct Profile2SteeringRequestTLV
{
    INT8U   tlv_type;                           // Must always be set to
												// TLV_TYPE_PROFILE_2_STEERING_REQUEST

    INT8U   bssid[6];

    INT8U   steer_parameters;

    INT16U  steer_opportunity_window;

    INT16U  btm_disassociation_timer;

    INT8U   sta_list_count;

    INT8U   (*sta_list)[6];

    INT8U   target_bssid_list_count;

    struct  Profile2SteeringRequestTargetBssidInfo    *target_bssid_list;
};

struct UnsuccessfulAssociationPolicyTLV
{
    INT8U   tlv_type;                           // Must always be set to
							            		// TLV_TYPE_UNSUCCESSFUL_ASSOCIATION_POILCY
    INT8U   report_unsuccessful_associations;   // bit 7: Whether report -- 0: Do not report unsuccessful association attempts 1: Report unsuccessful association attempts
                                                // bit 6 - 0: Reserved
    INT32U  maximum_reporting_rate;             // Maximum rate for reporting unsuccessful association attempts (in attempts per minute)

};

struct MetricCollectionIntervalTLV
{
    INT8U   tlv_type;               // Must always be set to
							        // TLV_TYPE_METRIC_COLLECTION_INTERVAL
    INT32U  collection_interval;    // Device Collection Interval

};


struct RadioMetricsTLV
{
    INT8U   tlv_type;                        // Must always be set to
							                 // TLV_TYPE_RADIO_METRICS
    INT8U   radio_unique_identifier[6];      // Radio Unique Identifier of a radio of the Multi-AP Agent for which metrics are being reported.

    INT8U   noise;                           //Radio Noise

    INT8U   transmit;                        //Radio Transmit

    INT8U   receiveself;                     //Radio ReceiveSelf

    INT8U   receiveother;                    //Radio ReceiveOther

};

struct APExtendedMetricsTLV
{
    INT8U   tlv_type;                        // Must always be set to
							                 // TLV_TYPE_AP_EXTENDED_METRICS
    INT8U   radio_unique_identifier[6];      // Radio Unique Identifier of a radio of the Multi-AP Agent for which metrics are being reported.

    INT32U   unicast_byte_sent;              //UnicastBytesSent

    INT32U   unicast_byte_received;          //UnicastBytesReceived

    INT32U   multicast_byte_sent;            //MultiicastBytesSent

    INT32U   multicast_byte_received;        //MultiicastBytesReceived

    INT32U   broadcast_byte_sent;            //BroadicastBytesSent

    INT32U   broadcast_byte_received;        //BroadicastBytesReceived

};

struct ReportedBSSInfo
{
    INT8U   bssid[6];                        //BSSID of the BSS to which the STA is associated.

    INT32U   last_data_downlink_rate;         //LastDataDownlinkRate.

    INT32U   last_data_uplink_rate;           //LastDataUplinkRate.

    INT32U   utilization_receive;             //UtilizationReceive.

    INT32U   utilization_transmit;            //UtilizationTransmit.

};

struct AssociatedSTAExtendedLinkMericsTLV
{
    INT8U   tlv_type;                        // Must always be set to
							                 // TLV_TYPE_ASSOCIATED_STA_EXTENDED_LINK_METRICS
    INT8U   sta_mac_address[6];              // MAC address of the associated STA.

    INT8U   bssid_nr;                        //Number of BSSIDs reported for this STA.

    struct ReportedBSSInfo* reported_bss;    //Reported BSS info.

};

struct StatusCodeTLV
{
    INT8U   tlv_type;                        // Must always be set to
							                 // TLV_TYPE_STATUS_CODE
    INT16U   status_code;                    // Status Code.

};

struct ReasonCodeTLV
{
    INT8U   tlv_type;                        // Must always be set to
							                 // TLV_TYPE_REASON_CODE
    INT16U   reason_code;                    // Reason Code.

};

struct BackhaulSTARadioCapabilitiesTLV
{
    INT8U   tlv_type;                        // Must always be set to
							                 // TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES
    INT8U   radio_unique_identifier[6];      // Radio unique identifier of the radio for which capabilities are reported.

    INT8U   included_mac;                    // bit 7 -- 0: the MAC address is not included below; 1: the MAC address is included below
                                             // bits 6-0 -- Reserved.
    INT8U   sta_mac_address[6];              // MAC address of the backhaul STA on this radio.(This field is present if the MAC address include field is set to 1).

};

struct BackhaulBSSConfigurationTLV
{
    INT8U   tlv_type;                                       // Must always be set to
							                                // TLV_TYPE_BACKHAUL_BSS_CONFIGURATION
    INT8U   bssid[6];                                       // BSSID of a bss
    INT8U   association_disallowed;                          // bit 7 -- Profile-1 Backhaul STA association disallowed.
                                                            // bit 6 -- Profile-2 Backhaul STA association disallowed.
                                                            // bit 5-0 -- Reserved.

};
#endif // END _MULTI_AP_TLVS_H_
