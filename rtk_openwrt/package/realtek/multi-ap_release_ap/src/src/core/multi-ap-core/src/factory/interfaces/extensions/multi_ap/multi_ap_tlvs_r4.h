/*
 *  Realtek Semiconductor Corp.
 *
 */

#ifndef _MULTI_AP_TLVS_R4_H_
#define _MULTI_AP_TLVS_R4_H_

#include "platform.h"

////////////////////////////////////////////////////////////////////////////////
/// Profile 4 TLVs
////////////////////////////////////////////////////////////////////////////////
#define TLV_TYPE_ANTICIPATED_CHANNEL_PERFERENCE (214)
#define TLV_TYPE_ANTICIPATED_CHANNEL_USAGE (215)
#define TLV_TYPE_SPATIAL_REUSE_REQUEST (216)
#define TLV_TYPE_SPATIAL_REUSE_REPORT (217)
#define TLV_TYPE_SPATIAL_REUSE_CONFIG_RESPONSE (218)
#define TLV_TYPE_QOS_MANAGEMENT_POLICY (219)
#define TLV_TYPE_QOS_MANAGEMENT_DESCRIPTOR (220)
#define TLV_TYPE_CONTROLLER_CAPABILITY (221)

////////////////////////////////////////////////////////////////////////////////
/// Profile 4 TLVs
////////////////////////////////////////////////////////////////////////////////
struct OperatingChannelClass{
	INT8U operating_class;
	INT8U channel_list_nr;
	INT8U *channel_list;
	INT8U reserved[4];
};

struct AnticipatedChannelPerferenceTLV{
	INT8U tlv_type;
	INT8U operating_class_nr;
	struct OperatingChannelClass *operate_class;
};

struct UsageEntryClass{
	INT32U burst_start_time;
	INT32U burst_length;
	INT32U repititions;
	INT32U burst_interval;
	INT8U RU_bitmask_length;
	INT8U *RU_bitmask;
	INT8U transmitter_identifier[6];
	INT8U power_level;
	INT8U channel_usage_reason;
	INT8U reserved[4];
};

struct AnticipatedChannelUsageTLV{
	INT8U tlv_type;
	INT8U operating_class;
	INT8U channel_number;
	INT8U reference_bssid[6];
	INT8U usage_entry_nr;
	struct UsageEntryClass *usage_entry;
};

struct SpatialReuseRequestTLV{
	INT8U tlv_type;
	INT8U ruid[6];
	INT8U bss_color;
	INT8U valid_field;
	INT8U non_srg_obsspd_max_offset;
	INT8U srg_obsspd_min_offset;
	INT8U srg_obsspd_max_offset;
	INT8U srg_bss_color_bitmap[8];
	INT8U srg_partial_bssid_bitmap[8];
	INT8U reserved[2];
};

struct SpatialReuseReportTLV{
	INT8U tlv_type;
	INT8U ruid[6];
	INT8U bss_color;
	INT8U valid_field;
	INT8U non_srg_obsspd_max_offset;
	INT8U srg_obsspd_min_offset;
	INT8U srg_obsspd_max_offset;
	INT8U srg_bss_color_bitmap[8];
	INT8U srg_partial_bssid_bitmap[8];
	INT8U neighbor_bss_color_inuse_bitmap[8];
	INT8U reserved[2];
};

struct SpatialReuseConfigResponseTLV{
	INT8U tlv_type;
	INT8U ruid[6];
	INT8U response_code;
};

struct QoSManagementPolicyTLV{
	INT8U tlv_type;
	INT8U mscsd_disallowed_sta_nr;
	INT8U (*mscsd_disallowed_sta_mac_address)[6];
	INT8U scsd_disallowed_sta_nr;
	INT8U (*scsd_disallowed_sta_mac_address)[6];
	INT8U reserved[20];
};

struct QoSManagementDescriptorTLV{
	INT8U tlv_type;
	INT16U qmid;
	INT8U bssid[6];
	INT8U client_mac[6];
	INT8U *descriptor_element_1;
	INT16U descriptor_element_len_1;
	INT8U *descriptor_element_2;
	INT16U descriptor_element_len_2;
};

struct ControllerCapabilityTLV{
	INT8U tlv_type;
	INT8U KiBMiB_counter;
};

#endif