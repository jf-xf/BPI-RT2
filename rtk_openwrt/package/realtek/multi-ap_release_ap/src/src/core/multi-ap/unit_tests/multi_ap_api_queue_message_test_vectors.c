/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "../map_utils.h"

// This file contains test vectors than can be used to check the
// "_forge_request_element()" and "map_parse_message()"
// functions
//
// Each test vector is made up of three variables:
//
//   - A TLV structure
//   - An array of bits representing the network packet
//   - An variable holding the length of the packet
//
//
// This is how you use these test vectors:
//
//   A) stream = _forge_request_element(tlv_xxx, &stream_len);
//
//   B) tlv = map_parse_message(stream_xxx);
//

////////////////////////////////////////////////////////////////////////////////
// Test vector 001 (TLV <--> packet)
////////////////////////////////////////////////////////////////////////////////

struct map_topology_query_request multi_ap_api_request_structure_001 = {
	.element_id    = MAP_TOPOLOGY_QUERY_REQUEST_ELEMENT,
	.target_al_mac = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 },
};

// CheckTrue (TLV --> packet)
INT8U multi_ap_api_request_stream_001[] = {
	0x01,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};

// CheckFalse (TLV <-- packet)
//   'supported_service' should only contains 0x00 or 0x01
INT8U multi_ap_api_request_stream_001b[] = {
	0x01,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x06
};

INT16U multi_ap_api_request_stream_len_001 = 7;

////////////////////////////////////////////////////////////////////////////////
// Test vector 002 (TLV <--> packet)
////////////////////////////////////////////////////////////////////////////////

struct map_link_metric_query multi_ap_api_request_structure_002 = {
	.element_id    = MAP_LINK_METRIC_QUERY_ELEMENT,
	.target_al_mac = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 },
	.destination   = 0x01,
	.type          = 0x02,
	.sta_mac       = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 },
};

// CheckTrue (TLV --> packet)
INT8U multi_ap_api_request_stream_002[] = {
	0x02,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
	0x01,
	0x02,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};

// CheckFalse (TLV <-- packet)
//   'searched_service' should only contains 0x00
INT8U multi_ap_api_request_stream_002b[] = {
	0x02,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
	0x01,
	0x02,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x06
};

INT16U multi_ap_api_request_stream_len_002 = 15;

////////////////////////////////////////////////////////////////////////////////
// Test vector 003 (TLV <--> packet)
////////////////////////////////////////////////////////////////////////////////
struct map_autoconfig_renew_request multi_ap_api_request_structure_003 = {
	.element_id   = MAP_AUTOCONFIG_RENEW_REQUEST_ELEMENT,
	.include_self = 0x01,
};

// CheckTrue (TLV --> packet)
INT8U multi_ap_api_request_stream_003[] = {
	0x03,
	0x01
};

// CheckFalse (TLV <-- packet)
INT8U multi_ap_api_request_stream_003b[] = {
	0x03,
	0x02
};

INT16U multi_ap_api_request_stream_len_003 = 2;

////////////////////////////////////////////////////////////////////////////////
// Test vector 004 (TLV <--> packet)
////////////////////////////////////////////////////////////////////////////////

struct map_channel_preference_query multi_ap_api_request_structure_004 = {
	.element_id    = MAP_CHANNEL_PREFERENCE_QUERY_ELEMENT,
	.target_al_mac = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
};

// CheckTrue (TLV --> packet)
INT8U multi_ap_api_request_stream_004[] = {
	0x04,
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06
};

// CheckFalse (TLV <-- packet)
INT8U multi_ap_api_request_stream_004b[] = {
	0x04,
	0x01, 0x02, 0x03, 0x04, 0x05, 0x07
};

INT16U multi_ap_api_request_stream_len_004 = 7;

////////////////////////////////////////////////////////////////////////////////
// Test vector 005 (TLV <--> packet)
////////////////////////////////////////////////////////////////////////////////

struct transmit_power_limit tx_pwr_limit_005[1] = {
	{
	    .radio_mac    = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	    .tx_pwr_limit = 0x01,
	}
};

INT8U channel_list_005[1] = { 0x01 };

struct channel_preference_op_class ch_pre_op_class_005[1] = {
	{ .op_class               = 0x01,
	  .channel_nr             = 0x01,
	  .channel_list           = channel_list_005,
	  .preference_reason_code = 0x02 }
};

struct channel_preference ch_preference_005[1] = {
	{ .radio_mac       = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	  .op_class_nr     = 0x01,
	  .ch_pre_op_class = ch_pre_op_class_005 }
};

struct map_channel_selection_request multi_ap_api_request_structure_005 = {
	.element_id       = MAP_CHANNEL_SELECTION_REQUEST_ELEMENT,
	.target_al_mac    = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	.ch_preference_nr = 0x01,
	.ch_preference    = ch_preference_005,
	.tx_pwr_limit_nr  = 0x01,
	.tx_pwr_limit     = tx_pwr_limit_005,

};

// CheckTrue (TLV --> packet)
INT8U multi_ap_api_request_stream_005[] = {
	0x05,
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	0x01,

	0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	0x01,

	0x01,
	0x01,
	0x01,
	0x02,
	0x01,
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	0x01
};

// CheckFalse (TLV <-- packet)
INT8U multi_ap_api_request_stream_005b[] = {
	0x05,
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	0x01,

	0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	0x01,

	0x01,
	0x01,
	0x01,
	0x02,
	0x01,
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	0x02
};

INT16U multi_ap_api_request_stream_len_005 = 27;

////////////////////////////////////////////////////////////////////////////////
// Test vector 006 (TLV <--> packet)
////////////////////////////////////////////////////////////////////////////////

INT8U sta_mac_address_006[1][6] = { { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 } };

struct target_bss target_bss_006[1] = {
	{
	    .bssid    = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	    .op_class = 0x01,
	    .channel  = 0x02,
	}
};

struct map_client_steering_request multi_ap_api_request_structure_006 = {
	.element_id    = MAP_CLIENT_STEERING_REQUEST_ELEMENT,
	.target_al_mac = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	.steering_req  = {
        .bssid           = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
        .mode            = 0x01,
        .window          = 0x0002,
        .disassoc_timer  = 0x0003,
        .sta_nr          = 0x01,
        .sta_mac_address = sta_mac_address_006,
        .target_bss_nr   = 0x01,
        .target_bss      = target_bss_006,
    },
};

// CheckTrue (TLV --> packet)
INT8U multi_ap_api_request_stream_006[] = {
	0x06,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,

	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x00,
	0x02,
	0x00,
	0x03,
	0x01,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,

	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x02,
};

// CheckFalse (TLV <-- packet)
INT8U multi_ap_api_request_stream_006b[] = {
	0x06,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,

	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x00,
	0x02,
	0x00,
	0x03,
	0x01,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,

	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x03,
};

INT16U multi_ap_api_request_stream_len_006 = 34;

////////////////////////////////////////////////////////////////////////////////
// Test vector 007 (TLV <--> packet)
////////////////////////////////////////////////////////////////////////////////

INT8U sta_mac_address_007[1][6] = { { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 } };

struct client_association_control_request client_assoc_ctrl_req_007[1] = {
	{
	    .bssid           = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	    .assoc_ctrl      = 0x01,
	    .validity_period = 0x0002,
	    .sta_nr          = 0x01,
	    .sta_mac_address = sta_mac_address_007,
	}
};

struct map_client_association_control_request multi_ap_api_request_structure_007 = {
	.element_id               = MAP_CLIENT_ASSOCIATION_CONTROL_REQUEST_ELEMENT,
	.target_al_mac            = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	.client_assoc_ctrl_req_nr = 0x01,
	.client_assoc_ctrl_req    = client_assoc_ctrl_req_007,
};

// CheckTrue (TLV --> packet)
INT8U multi_ap_api_request_stream_007[] = {
	0x07,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,

	0x01,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x00,
	0x02,
	0x01,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
};

// CheckFalse (TLV <-- packet)
INT8U multi_ap_api_request_stream_007b[] = {
	0x07,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,

	0x01,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x00,
	0x02,
	0x01,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x07,
};

INT16U multi_ap_api_request_stream_len_007 = 24;

////////////////////////////////////////////////////////////////////////////////
// Test vector 008 (TLV <--> packet)
////////////////////////////////////////////////////////////////////////////////
struct map_backhaul_steering_request multi_ap_api_request_structure_008 = {
	.element_id       = MAP_BACKHAUL_STEERING_REQUEST_ELEMENT,
	.target_al_mac    = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	.backhaul_sta_mac = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	.target_bssid     = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	.op_class         = 0x01,
	.channel          = 0x02,
};

// CheckTrue (TLV --> packet)
INT8U multi_ap_api_request_stream_008[] = {
	0x08,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x02,
};

// CheckFalse (TLV <-- packet)
INT8U multi_ap_api_request_stream_008b[] = {
	0x08,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x03,
};

INT16U multi_ap_api_request_stream_len_008 = 21;

////////////////////////////////////////////////////////////////////////////////
// Test vector 009 (TLV <--> packet)
////////////////////////////////////////////////////////////////////////////////

struct radio_metric_config radio_metric_policy_data_009[1] = {
	{
	    .radio_mac        = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	    .rcpi_threshold   = 0x01,
	    .rcpi_hysteresis  = 0x02,
	    .cu_threshold     = 0x03,
	    .inclusion_policy = 0x04,
	}
};

struct radio_steer_config radio_steer_policy_data_009[1] = {
	{
	    .radio_mac      = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	    .policy         = 0x01,
	    .cu_threshold   = 0x02,
	    .rcpi_threshold = 0x03,
	}
};

INT8U btm_stas_009[1][6] = { { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 } };

INT8U ls_stas_009[1][6] = { { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 } };

struct map_policy_config_request multi_ap_api_request_structure_009 = {
	.element_id             = MAP_POLICY_CONFIG_REQUEST_ELEMENT,
	.target_al_mac          = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	.steer_policy_inclusion = 0x01,
	.steer_policy_data      = {
        .ls_sta_nr               = 0x01,
        .ls_stas                 = ls_stas_009,
        .btm_sta_nr              = 0x01,
        .btm_stas                = btm_stas_009,
        .radio_nr                = 0x01,
        .radio_steer_policy_data = radio_steer_policy_data_009,
    },
	.metric_policy_inclusion = 0x01,
	.metric_policy_data      = {
        .report_interval          = 0x01,
        .radio_nr                 = 0x01,
        .radio_metric_policy_data = radio_metric_policy_data_009,
    },
};

// CheckTrue (TLV --> packet)
INT8U multi_ap_api_request_stream_009[] = {
	0x09,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,

	0x01,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,

	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x02,
	0x03,

	0x01,
	0x01,
	0x01,

	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x02,
	0x03,
	0x04,
};

// CheckFalse (TLV <-- packet)
INT8U multi_ap_api_request_stream_009b[] = {
	0x09,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,

	0x01,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,

	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x02,
	0x03,

	0x01,
	0x01,
	0x01,

	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x02,
	0x03,
	0x05,
};

INT16U multi_ap_api_request_stream_len_009 = 45;

////////////////////////////////////////////////////////////////////////////////
// Test vector 010 (TLV <--> packet)
////////////////////////////////////////////////////////////////////////////////

struct map_ap_capability_query multi_ap_api_request_structure_010 = {
	.element_id    = MAP_AP_CAPABILITY_QUERY_ELEMENT,
	.target_al_mac = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
};

// CheckTrue (TLV --> packet)
INT8U multi_ap_api_request_stream_010[] = {
	0x0A,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
};

// CheckFalse (TLV <-- packet)
INT8U multi_ap_api_request_stream_010b[] = {
	0x0A,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x07,
};

INT16U multi_ap_api_request_stream_len_010 = 7;

////////////////////////////////////////////////////////////////////////////////
// Test vector 011 (TLV <--> packet)
////////////////////////////////////////////////////////////////////////////////
struct map_client_capability_query multi_ap_api_request_structure_011 = {
	.element_id    = MAP_CLIENT_CAPABILITY_QUERY_ELEMENT,
	.target_al_mac = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	.bssid         = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	.sta_mac       = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
};

// CheckTrue (TLV --> packet)
INT8U multi_ap_api_request_stream_011[] = {
	0x0B,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
};

// CheckFalse (TLV <-- packet)
INT8U multi_ap_api_request_stream_011b[] = {
	0x0B,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x07,
};

INT16U multi_ap_api_request_stream_len_011 = 19;

////////////////////////////////////////////////////////////////////////////////
// Test vector 012 (TLV <--> packet)
////////////////////////////////////////////////////////////////////////////////
INT8U bssid_012[1][6] = { { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 } };

struct map_ap_metric_query multi_ap_api_request_structure_012 = {
	.element_id    = MAP_AP_METRIC_QUERY_ELEMENT,
	.target_al_mac = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	.bssid_nr      = 0x01,
	.bssid         = bssid_012,
};

// CheckTrue (TLV --> packet)
INT8U multi_ap_api_request_stream_012[] = {
	0x0C,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
};

// CheckFalse (TLV <-- packet)
INT8U multi_ap_api_request_stream_012b[] = {
	0x0C,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x07,
};

INT16U multi_ap_api_request_stream_len_012 = 14;

////////////////////////////////////////////////////////////////////////////////
// Test vector 013 (TLV <--> packet)
////////////////////////////////////////////////////////////////////////////////
struct map_assoc_sta_metric_query multi_ap_api_request_structure_013 = {
	.element_id    = MAP_ASSOC_STA_METRIC_QUERY_ELEMENT,
	.target_al_mac = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	.sta_mac       = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
};

// CheckTrue (TLV --> packet)
INT8U multi_ap_api_request_stream_013[] = {
	0x0D,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
};

// CheckFalse (TLV <-- packet)
INT8U multi_ap_api_request_stream_013b[] = {
	0x0D,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x07,
};

INT16U multi_ap_api_request_stream_len_013 = 13;

////////////////////////////////////////////////////////////////////////////////
// Test vector 014 (TLV <--> packet)
////////////////////////////////////////////////////////////////////////////////

INT8U stas_014[1][6] = { { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 } };

struct unassoc_sta_channel channel_data_014[1] = {
	{
	    .channel_nr = 0x01,
	    .sta_nr     = 0x01,
	    .stas       = stas_014,
	}
};

struct map_unassoc_sta_metric_query multi_ap_api_request_structure_014 = {
	.element_id      = MAP_UNASSOC_STA_METRIC_QUERY_ELEMENT,
	.target_al_mac   = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	.op_class        = 0x01,
	.channel_data_nr = 0x01,
	.channel_data    = channel_data_014,
};

// CheckTrue (TLV --> packet)
INT8U multi_ap_api_request_stream_014[] = {
	0x0E,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x01,

	0x01,
	0x01,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
};

// CheckFalse (TLV <-- packet)
INT8U multi_ap_api_request_stream_014b[] = {
	0x0E,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x01,

	0x01,
	0x01,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x07,
};

INT16U multi_ap_api_request_stream_len_014 = 17;

////////////////////////////////////////////////////////////////////////////////
// Test vector 015 (TLV <--> packet)
////////////////////////////////////////////////////////////////////////////////

INT8U element_list_015[2] = { 0x01, 0x02 };

INT8U channel_list_015[3] = { 0x01, 0x02, 0x03 };

struct ap_channel_report channel_report_015[1] = {
	{
	    .len          = 0x04,
	    .op_class     = 0x02,
	    .channel_list = channel_list_015,
	}
};

struct map_beacon_metric_query multi_ap_api_request_structure_015 = {
	.element_id          = MAP_BEACON_METRIC_QUERY_ELEMENT,
	.target_al_mac       = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	.beacon_metric_query = {
	    .sta_mac           = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	    .op_class          = 0x01,
	    .channel_nr        = 0x01,
	    .bssid             = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
	    .report_detail     = 0x01,
	    .ssid_len          = 0x05,
	    .ssid              = "ABCDE",
	    .channel_report_nr = 0x01,
	    .channel_report    = channel_report_015,
	    .element_nr        = 0x02,
	    .element_list      = element_list_015,

	},
};

// CheckTrue (TLV --> packet)
INT8U multi_ap_api_request_stream_015[] = {
	0x0F,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,

	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x01,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x05,
	0x41,
	0x42,
	0x43,
	0x44,
	0x45,
	0x01,

	0x04,
	0x02,
	0x01,
	0x02,
	0x03,

	0x02,
	0x01,
	0x02,
};

// CheckFalse (TLV <-- packet)
INT8U multi_ap_api_request_stream_015b[] = {
	0x0F,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,

	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x01,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x01,
	0x05,
	0x41,
	0x42,
	0x43,
	0x44,
	0x45,
	0x01,

	0x04,
	0x02,
	0x01,
	0x02,
	0x03,

	0x02,
	0x01,
	0x03,
};

INT16U multi_ap_api_request_stream_len_015 = 37;
