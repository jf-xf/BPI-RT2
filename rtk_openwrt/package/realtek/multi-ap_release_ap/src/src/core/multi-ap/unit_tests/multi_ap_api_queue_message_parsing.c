/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

//
// This file tests the "map_parse_message()" function by providing
// some test input streams and checking the generated output structure.
//
#include <stdio.h>
#include <string.h>
#include "../map_utils.h"
#include "../map_commands.h"

#include "multi_ap_api_queue_message_test_vectors.h"

uint8_t _compare_multi_ap_api_request_structures(uint8_t *memory_structure_1, uint8_t *memory_structure_2)
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

	case MAP_TOPOLOGY_QUERY_REQUEST_ELEMENT: {
		struct map_topology_query_request *p1, *p2;

		p1 = (struct map_topology_query_request *)memory_structure_1;
		p2 = (struct map_topology_query_request *)memory_structure_2;

		if (memcmp(p1, p2, sizeof(struct map_topology_query_request)) != 0) {
			//return 1 for not equal
			return 1;
		} else {
			return 0;
		}
	}

	case MAP_LINK_METRIC_QUERY_ELEMENT: {
		struct map_link_metric_query *p1, *p2;

		p1 = (struct map_link_metric_query *)memory_structure_1;
		p2 = (struct map_link_metric_query *)memory_structure_2;

		if (memcmp(p1, p2, sizeof(struct map_link_metric_query)) != 0) {
			//return 1 for not equal
			return 1;
		} else {
			return 0;
		}
	}

	case MAP_AUTOCONFIG_RENEW_REQUEST_ELEMENT: {
		struct map_autoconfig_renew_request *p1, *p2;

		p1 = (struct map_autoconfig_renew_request *)memory_structure_1;
		p2 = (struct map_autoconfig_renew_request *)memory_structure_2;

		if (memcmp(p1, p2, sizeof(struct map_autoconfig_renew_request)) != 0) {
			//return 1 for not equal
			return 1;
		} else {
			return 0;
		}
	}

	case MAP_CHANNEL_PREFERENCE_QUERY_ELEMENT: {
		struct map_channel_preference_query *p1, *p2;

		p1 = (struct map_channel_preference_query *)memory_structure_1;
		p2 = (struct map_channel_preference_query *)memory_structure_2;

		if (memcmp(p1, p2, sizeof(struct map_channel_preference_query)) != 0) {
			//return 1 for not equal
			return 1;
		} else {
			return 0;
		}
	}

	case MAP_CHANNEL_SELECTION_REQUEST_ELEMENT: {
		struct map_channel_selection_request *p1, *p2;
		INT8U                                 i, j;

		p1 = (struct map_channel_selection_request *)memory_structure_1;
		p2 = (struct map_channel_selection_request *)memory_structure_2;

		if (memcmp(p1->target_al_mac, p2->target_al_mac, 6) != 0 || p1->ch_preference_nr != p2->ch_preference_nr) {
			//return 1 for not equal
			return 1;
		}

		for (i = 0; i < p1->ch_preference_nr; i++) {
			if (memcmp(p1->ch_preference[i].radio_mac, p2->ch_preference[i].radio_mac, 6) != 0 || p1->ch_preference[i].op_class_nr != p2->ch_preference[i].op_class_nr) {
				return 1;
			}

			for (j = 0; j < p1->ch_preference[i].op_class_nr; j++) {
				if (p1->ch_preference[i].ch_pre_op_class[j].op_class != p2->ch_preference[i].ch_pre_op_class[j].op_class || p1->ch_preference[i].ch_pre_op_class[j].channel_nr != p2->ch_preference[i].ch_pre_op_class[j].channel_nr || memcmp(p1->ch_preference[i].ch_pre_op_class[j].channel_list, p2->ch_preference[i].ch_pre_op_class[j].channel_list, p1->ch_preference[i].ch_pre_op_class[j].channel_nr) != 0 || p1->ch_preference[i].ch_pre_op_class[j].preference_reason_code != p2->ch_preference[i].ch_pre_op_class[j].preference_reason_code) {
					return 1;
				}
			}
		}

		if (p1->tx_pwr_limit_nr != p2->tx_pwr_limit_nr) {
			return 1;
		}

		for (i = 0; i < p1->tx_pwr_limit_nr; i++) {
			if (memcmp(p1->tx_pwr_limit[i].radio_mac, p2->tx_pwr_limit[i].radio_mac, 6) != 0 || p1->tx_pwr_limit[i].tx_pwr_limit != p2->tx_pwr_limit[i].tx_pwr_limit) {
				return 1;
			}
		}

		return 0;
	}

	case MAP_CLIENT_STEERING_REQUEST_ELEMENT: {
		struct map_client_steering_request *p1, *p2;
		INT8U                               i;

		p1 = (struct map_client_steering_request *)memory_structure_1;
		p2 = (struct map_client_steering_request *)memory_structure_2;

		if (memcmp(p1->target_al_mac, p2->target_al_mac, 6) != 0) {
			//return 1 for not equal
			return 1;
		}

		if (memcmp(p1->steering_req.bssid, p2->steering_req.bssid, 6) != 0 || p1->steering_req.mode != p2->steering_req.mode || p1->steering_req.window != p2->steering_req.window || p1->steering_req.disassoc_timer != p2->steering_req.disassoc_timer || p1->steering_req.sta_nr != p2->steering_req.sta_nr) {
			return 1;
		}

		for (i = 0; i < p1->steering_req.sta_nr; i++) {
			if (memcmp(p1->steering_req.sta_mac_address[i], p2->steering_req.sta_mac_address[i], 6) != 0) {
				return 1;
			}
		}

		if (p1->steering_req.target_bss_nr != p2->steering_req.target_bss_nr) {
			return 1;
		}

		for (i = 0; i < p1->steering_req.target_bss_nr; i++) {
			if (memcmp(&p1->steering_req.target_bss[i], &p2->steering_req.target_bss[i], 8) != 0) {
				return 1;
			}
		}

		return 0;
	}

	case MAP_CLIENT_ASSOCIATION_CONTROL_REQUEST_ELEMENT: {
		struct map_client_association_control_request *p1, *p2;

		INT8U i;

		p1 = (struct map_client_association_control_request *)memory_structure_1;
		p2 = (struct map_client_association_control_request *)memory_structure_2;

		if (memcmp(p1->target_al_mac, p2->target_al_mac, 6) != 0 || p1->client_assoc_ctrl_req_nr != p2->client_assoc_ctrl_req_nr) {
			//return 1 for not equal
			return 1;
		}

		for (i = 0; i < p1->client_assoc_ctrl_req_nr; i++) {
			if (memcmp(p1->client_assoc_ctrl_req[i].bssid, p2->client_assoc_ctrl_req[i].bssid, 6) != 0 || p1->client_assoc_ctrl_req[i].assoc_ctrl != p2->client_assoc_ctrl_req[i].assoc_ctrl || p1->client_assoc_ctrl_req[i].validity_period != p2->client_assoc_ctrl_req[i].validity_period || p1->client_assoc_ctrl_req[i].sta_nr != p2->client_assoc_ctrl_req[i].sta_nr) {
				return 1;
			}

			if (memcmp(p1->client_assoc_ctrl_req[i].sta_mac_address, p2->client_assoc_ctrl_req[i].sta_mac_address, p1->client_assoc_ctrl_req[i].sta_nr * 6) != 0) {
				return 1;
			}
		}

		return 0;
	}

	case MAP_BACKHAUL_STEERING_REQUEST_ELEMENT: {
		struct map_backhaul_steering_request *p1, *p2;

		p1 = (struct map_backhaul_steering_request *)memory_structure_1;
		p2 = (struct map_backhaul_steering_request *)memory_structure_2;

		if (memcmp(p1, p2, sizeof(struct map_backhaul_steering_request)) != 0) {
			//return 1 for not equal
			return 1;
		} else {
			return 0;
		}
	}

	case MAP_POLICY_CONFIG_REQUEST_ELEMENT: {
		struct map_policy_config_request *p1, *p2;

		p1 = (struct map_policy_config_request *)memory_structure_1;
		p2 = (struct map_policy_config_request *)memory_structure_2;

		if (memcmp(p1->target_al_mac, p2->target_al_mac, 6) != 0 || p1->steer_policy_inclusion != p2->steer_policy_inclusion) {
			//return 1 for not equal
			return 1;
		}

		if (p1->steer_policy_inclusion) {
			if (p1->steer_policy_data.ls_sta_nr != p2->steer_policy_data.ls_sta_nr) {
				return 1;
			}

			if (memcmp(p1->steer_policy_data.ls_stas, p2->steer_policy_data.ls_stas, p1->steer_policy_data.ls_sta_nr * 6) != 0) {
				return 1;
			}

			if (p1->steer_policy_data.btm_sta_nr != p2->steer_policy_data.btm_sta_nr) {
				return 1;
			}

			if (memcmp(p1->steer_policy_data.btm_stas, p2->steer_policy_data.btm_stas, p1->steer_policy_data.btm_sta_nr * 6) != 0) {
				return 1;
			}

			if (p1->steer_policy_data.radio_nr != p2->steer_policy_data.radio_nr) {
				return 1;
			}

			if (memcmp(p1->steer_policy_data.radio_steer_policy_data, p2->steer_policy_data.radio_steer_policy_data, p1->steer_policy_data.radio_nr * 9) != 0) {
				return 1;
			}
		}

		if (p1->metric_policy_inclusion != p2->metric_policy_inclusion) {
			return 1;
		}

		if (p1->metric_policy_inclusion) {
			if (p1->metric_policy_data.report_interval != p2->metric_policy_data.report_interval || p1->metric_policy_data.radio_nr != p2->metric_policy_data.radio_nr) {
				return 1;
			}

			if (memcmp(p1->metric_policy_data.radio_metric_policy_data, p2->metric_policy_data.radio_metric_policy_data, p1->metric_policy_data.radio_nr * 10) != 0) {
				return 1;
			}
		}

		return 0;
	}

	case MAP_AP_CAPABILITY_QUERY_ELEMENT: {
		struct map_ap_capability_query *p1, *p2;

		p1 = (struct map_ap_capability_query *)memory_structure_1;
		p2 = (struct map_ap_capability_query *)memory_structure_2;

		if (memcmp(p1, p2, sizeof(struct map_ap_capability_query)) != 0) {
			//return 1 for not equal
			return 1;
		} else {
			return 0;
		}
	}

	case MAP_CLIENT_CAPABILITY_QUERY_ELEMENT: {
		struct map_client_capability_query *p1, *p2;

		p1 = (struct map_client_capability_query *)memory_structure_1;
		p2 = (struct map_client_capability_query *)memory_structure_2;

		if (memcmp(p1, p2, sizeof(struct map_client_capability_query)) != 0) {
			//return 1 for not equal
			return 1;
		} else {
			return 0;
		}
	}

	case MAP_AP_METRIC_QUERY_ELEMENT: {
		struct map_ap_metric_query *p1, *p2;

		p1 = (struct map_ap_metric_query *)memory_structure_1;
		p2 = (struct map_ap_metric_query *)memory_structure_2;

		if (memcmp(p1->target_al_mac, p2->target_al_mac, 6) != 0 || p1->bssid_nr != p2->bssid_nr) {
			//return 1 for not equal
			return 1;
		}

		if (memcmp(p1->bssid, p2->bssid, p1->bssid_nr * 6) != 0) {
			return 1;
		}

		return 0;
	}

	case MAP_ASSOC_STA_METRIC_QUERY_ELEMENT: {
		struct map_assoc_sta_metric_query *p1, *p2;

		p1 = (struct map_assoc_sta_metric_query *)memory_structure_1;
		p2 = (struct map_assoc_sta_metric_query *)memory_structure_2;

		if (memcmp(p1, p2, sizeof(struct map_assoc_sta_metric_query)) != 0) {
			//return 1 for not equal
			return 1;
		} else {
			return 0;
		}
	}

	case MAP_UNASSOC_STA_METRIC_QUERY_ELEMENT: {
		struct map_unassoc_sta_metric_query *p1, *p2;
		INT8U                                i;

		p1 = (struct map_unassoc_sta_metric_query *)memory_structure_1;
		p2 = (struct map_unassoc_sta_metric_query *)memory_structure_2;

		if (memcmp(p1->target_al_mac, p2->target_al_mac, 6) != 0 || p1->op_class != p2->op_class || p1->channel_data_nr != p2->channel_data_nr) {
			//return 1 for not equal
			return 1;
		}

		for (i = 0; i < p1->channel_data_nr; i++) {
			if (p1->channel_data[i].channel_nr != p2->channel_data[i].channel_nr || p1->channel_data[i].sta_nr != p2->channel_data[i].sta_nr) {
				return 1;
			}

			if (memcmp(p1->channel_data[i].stas, p2->channel_data[i].stas, p1->channel_data[i].sta_nr * 6) != 0) {
				return 1;
			}
		}

		return 0;
	}

	case MAP_BEACON_METRIC_QUERY_ELEMENT: {
		struct map_beacon_metric_query *p1, *p2;
		INT8U                           i;

		p1 = (struct map_beacon_metric_query *)memory_structure_1;
		p2 = (struct map_beacon_metric_query *)memory_structure_2;

		if (memcmp(p1->target_al_mac, p2->target_al_mac, 6) != 0) {
			//return 1 for not equal
			return 1;
		}

		if (memcmp(p1->beacon_metric_query.sta_mac, p2->beacon_metric_query.sta_mac, 6) != 0 || p1->beacon_metric_query.op_class != p2->beacon_metric_query.op_class || p1->beacon_metric_query.channel_nr != p2->beacon_metric_query.channel_nr || memcmp(p1->beacon_metric_query.bssid, p2->beacon_metric_query.bssid, 6) != 0 || p1->beacon_metric_query.report_detail != p2->beacon_metric_query.report_detail || p1->beacon_metric_query.ssid_len != p2->beacon_metric_query.ssid_len || memcmp(p1->beacon_metric_query.ssid, p2->beacon_metric_query.ssid, p1->beacon_metric_query.ssid_len) != 0 || p1->beacon_metric_query.channel_report_nr != p2->beacon_metric_query.channel_report_nr) {
			return 1;
		}

		for (i = 0; i < p1->beacon_metric_query.channel_report_nr; i++) {
			if (p1->beacon_metric_query.channel_report[i].len != p2->beacon_metric_query.channel_report[i].len || p1->beacon_metric_query.channel_report[i].op_class != p2->beacon_metric_query.channel_report[i].op_class || memcmp(p1->beacon_metric_query.channel_report[i].channel_list, p2->beacon_metric_query.channel_report[i].channel_list, p1->beacon_metric_query.channel_report[i].len) != 0) {
				return 1;
			}
		}

		if (p1->beacon_metric_query.element_nr != p2->beacon_metric_query.element_nr || memcmp(p1->beacon_metric_query.element_list, p2->beacon_metric_query.element_list, p1->beacon_metric_query.element_nr) != 0) {
			return 1;
		}

		return 0;
	}

	default: {
		// Unknown structure type
		//
		return 1;
	}
	}
}

INT8U _check(const char *test_description, INT8U mode, INT8U *input, INT8U *expected_output)
{
	INT8U  result;
	INT8U *real_output;
	INT8U  comparison;

	// Parse the packet
	real_output = map_parse_element(input);

	// Compare TLVs
	comparison = _compare_multi_ap_api_request_structures(real_output, expected_output);

	if (mode == CHECK_TRUE) {
		if (0 == comparison) {
			result = 0;
			printf("%-100s: OK\n", test_description);
			// visit_multi_ap_api_request_structure(real_output, print_callback, printf, "");
		} else {
			result = 1;
			printf("%-100s: KO !!!\n", test_description);

			// CheckTrue error needs more debug info
			printf("  Expected output:\n");
			// visit_multi_ap_api_request_structure(expected_output, print_callback, printf, "");
			printf("  Real output    :\n");
			// visit_multi_ap_api_request_structure(real_output, print_callback, printf, "");
		}
	} else {
		if (0 == comparison) {
			result = 1;
			printf("%-100s: KO !!!\n", test_description);
		} else {
			result = 0;
			printf("%-100s: OK\n", test_description);
		}
	}

	return result;
}

INT8U _checkTrue(const char *test_description, INT8U *input, INT8U *expected_output)
{
	return _check(test_description, CHECK_TRUE, input, expected_output);
}

INT8U _checkFalse(const char *test_description, INT8U *input, INT8U *expected_output)
{
	return _check(test_description, CHECK_FALSE, input, expected_output);
}

int main(void)
{
	INT8U result = 0;

#define MULTIAP_API_REQUEST_PARSE001 "MULTIAP_API_REQUEST_PARSE001 - Parse TOPOLOGY_QUERY_REQUEST TLV (multi_ap_api_request_stream_001)"
	result += _checkTrue(MULTIAP_API_REQUEST_PARSE001, multi_ap_api_request_stream_001, (INT8U *)&multi_ap_api_request_structure_001);

#define MULTIAP_API_REQUEST_PARSE002 "MULTIAP_API_REQUEST_PARSE002 - Parse TOPOLOGY_QUERY_REQUEST TLV (multi_ap_api_request_stream_001b)"
	result += _checkFalse(MULTIAP_API_REQUEST_PARSE002, multi_ap_api_request_stream_001b, (INT8U *)&multi_ap_api_request_structure_001);

#define MULTIAP_API_REQUEST_PARSE003 "MULTIAP_API_REQUEST_PARSE003 - Parse LINK_METRIC_QUERY TLV (multi_ap_api_request_stream_002)"
	result += _checkTrue(MULTIAP_API_REQUEST_PARSE003, multi_ap_api_request_stream_002, (INT8U *)&multi_ap_api_request_structure_002);

#define MULTIAP_API_REQUEST_PARSE004 "MULTIAP_API_REQUEST_PARSE004 - Parse LINK_METRIC_QUERY TLV (multi_ap_api_request_stream_002b)"
	result += _checkFalse(MULTIAP_API_REQUEST_PARSE004, multi_ap_api_request_stream_002b, (INT8U *)&multi_ap_api_request_structure_002);

#define MULTIAP_API_REQUEST_PARSE005 "MULTIAP_API_REQUEST_PARSE005 - Parse AUTOCONFIG_RENEW_REQUEST TLV (multi_ap_api_request_stream_003)"
	result += _checkTrue(MULTIAP_API_REQUEST_PARSE005, multi_ap_api_request_stream_003, (INT8U *)&multi_ap_api_request_structure_003);

#define MULTIAP_API_REQUEST_PARSE006 "MULTIAP_API_REQUEST_PARSE006 - Parse AUTOCONFIG_RENEW_REQUEST TLV (multi_ap_api_request_stream_003b)"
	result += _checkFalse(MULTIAP_API_REQUEST_PARSE006, multi_ap_api_request_stream_003b, (INT8U *)&multi_ap_api_request_structure_003);

#define MULTIAP_API_REQUEST_PARSE007 "MULTIAP_API_REQUEST_PARSE007 - Parse CHANNEL_PREFERENCE_QUERY TLV (multi_ap_api_request_stream_004)"
	result += _checkTrue(MULTIAP_API_REQUEST_PARSE007, multi_ap_api_request_stream_004, (INT8U *)&multi_ap_api_request_structure_004);

#define MULTIAP_API_REQUEST_PARSE008 "MULTIAP_API_REQUEST_PARSE008 - Parse CHANNEL_PREFERENCE_QUERY TLV (multi_ap_api_request_stream_004b)"
	result += _checkFalse(MULTIAP_API_REQUEST_PARSE008, multi_ap_api_request_stream_004b, (INT8U *)&multi_ap_api_request_structure_004);

#define MULTIAP_API_REQUEST_PARSE009 "MULTIAP_API_REQUEST_PARSE009 - Parse CHANNEL_SELECTION_REQUEST TLV (multi_ap_api_request_stream_005)"
	result += _checkTrue(MULTIAP_API_REQUEST_PARSE009, multi_ap_api_request_stream_005, (INT8U *)&multi_ap_api_request_structure_005);

#define MULTIAP_API_REQUEST_PARSE010 "MULTIAP_API_REQUEST_PARSE010 - Parse CHANNEL_SELECTION_REQUEST TLV (multi_ap_api_request_stream_005b)"
	result += _checkFalse(MULTIAP_API_REQUEST_PARSE010, multi_ap_api_request_stream_005b, (INT8U *)&multi_ap_api_request_structure_005);

#define MULTIAP_API_REQUEST_PARSE011 "MULTIAP_API_REQUEST_PARSE011 - Parse CLIENT_STEERING_REQUEST TLV (multi_ap_api_request_stream_006)"
	result += _checkTrue(MULTIAP_API_REQUEST_PARSE011, multi_ap_api_request_stream_006, (INT8U *)&multi_ap_api_request_structure_006);

#define MULTIAP_API_REQUEST_PARSE012 "MULTIAP_API_REQUEST_PARSE012 - Parse CLIENT_STEERING_REQUEST TLV (multi_ap_api_request_stream_006b)"
	result += _checkFalse(MULTIAP_API_REQUEST_PARSE012, multi_ap_api_request_stream_006b, (INT8U *)&multi_ap_api_request_structure_006);

#define MULTIAP_API_REQUEST_PARSE013 "MULTIAP_API_REQUEST_PARSE013 - Parse CLIENT_ASSOCIATION_CONTROL_REQUEST TLV (multi_ap_api_request_stream_007)"
	result += _checkTrue(MULTIAP_API_REQUEST_PARSE013, multi_ap_api_request_stream_007, (INT8U *)&multi_ap_api_request_structure_007);

#define MULTIAP_API_REQUEST_PARSE014 "MULTIAP_API_REQUEST_PARSE014 - Parse CLIENT_ASSOCIATION_CONTROL_REQUEST TLV (multi_ap_api_request_stream_007b)"
	result += _checkFalse(MULTIAP_API_REQUEST_PARSE014, multi_ap_api_request_stream_007b, (INT8U *)&multi_ap_api_request_structure_007);

#define MULTIAP_API_REQUEST_PARSE015 "MULTIAP_API_REQUEST_PARSE015 - Parse BACKHAUL_STEERING_REQUEST TLV (multi_ap_api_request_stream_008)"
	result += _checkTrue(MULTIAP_API_REQUEST_PARSE015, multi_ap_api_request_stream_008, (INT8U *)&multi_ap_api_request_structure_008);

#define MULTIAP_API_REQUEST_PARSE016 "MULTIAP_API_REQUEST_PARSE016 - Parse BACKHAUL_STEERING_REQUEST TLV (multi_ap_api_request_stream_008b)"
	result += _checkFalse(MULTIAP_API_REQUEST_PARSE016, multi_ap_api_request_stream_008b, (INT8U *)&multi_ap_api_request_structure_008);

#define MULTIAP_API_REQUEST_PARSE017 "MULTIAP_API_REQUEST_PARSE017 - Parse POLICY_CONFIG_REQUEST TLV (multi_ap_api_request_stream_009)"
	result += _checkTrue(MULTIAP_API_REQUEST_PARSE017, multi_ap_api_request_stream_009, (INT8U *)&multi_ap_api_request_structure_009);

#define MULTIAP_API_REQUEST_PARSE018 "MULTIAP_API_REQUEST_PARSE018 - Parse POLICY_CONFIG_REQUEST TLV (multi_ap_api_request_stream_009b)"
	result += _checkFalse(MULTIAP_API_REQUEST_PARSE018, multi_ap_api_request_stream_009b, (INT8U *)&multi_ap_api_request_structure_009);

#define MULTIAP_API_REQUEST_PARSE019 "MULTIAP_API_REQUEST_PARSE019 - Parse AP_CAPABILITY_QUERY TLV (multi_ap_api_request_stream_010)"
	result += _checkTrue(MULTIAP_API_REQUEST_PARSE019, multi_ap_api_request_stream_010, (INT8U *)&multi_ap_api_request_structure_010);

#define MULTIAP_API_REQUEST_PARSE020 "MULTIAP_API_REQUEST_PARSE020 - Parse AP_CAPABILITY_QUERY TLV (multi_ap_api_request_stream_010b)"
	result += _checkFalse(MULTIAP_API_REQUEST_PARSE020, multi_ap_api_request_stream_010b, (INT8U *)&multi_ap_api_request_structure_010);

#define MULTIAP_API_REQUEST_PARSE021 "MULTIAP_API_REQUEST_PARSE021 - Parse CLIENT_CAPABILITY_QUERY TLV (multi_ap_api_request_stream_011)"
	result += _checkTrue(MULTIAP_API_REQUEST_PARSE021, multi_ap_api_request_stream_011, (INT8U *)&multi_ap_api_request_structure_011);

#define MULTIAP_API_REQUEST_PARSE022 "MULTIAP_API_REQUEST_PARSE022 - Parse CLIENT_CAPABILITY_QUERY TLV (multi_ap_api_request_stream_011b)"
	result += _checkFalse(MULTIAP_API_REQUEST_PARSE022, multi_ap_api_request_stream_011b, (INT8U *)&multi_ap_api_request_structure_011);

#define MULTIAP_API_REQUEST_PARSE023 "MULTIAP_API_REQUEST_PARSE023 - Parse AP_METRIC_QUERY TLV (multi_ap_api_request_stream_012)"
	result += _checkTrue(MULTIAP_API_REQUEST_PARSE023, multi_ap_api_request_stream_012, (INT8U *)&multi_ap_api_request_structure_012);

#define MULTIAP_API_REQUEST_PARSE024 "MULTIAP_API_REQUEST_PARSE024 - Parse AP_METRIC_QUERY TLV (multi_ap_api_request_stream_012b)"
	result += _checkFalse(MULTIAP_API_REQUEST_PARSE024, multi_ap_api_request_stream_012b, (INT8U *)&multi_ap_api_request_structure_012);

#define MULTIAP_API_REQUEST_PARSE025 "MULTIAP_API_REQUEST_PARSE025 - Parse ASSOC_STA_METRIC_QUERY TLV (multi_ap_api_request_stream_013)"
	result += _checkTrue(MULTIAP_API_REQUEST_PARSE025, multi_ap_api_request_stream_013, (INT8U *)&multi_ap_api_request_structure_013);

#define MULTIAP_API_REQUEST_PARSE026 "MULTIAP_API_REQUEST_PARSE026 - Parse ASSOC_STA_METRIC_QUERY TLV (multi_ap_api_request_stream_013b)"
	result += _checkFalse(MULTIAP_API_REQUEST_PARSE026, multi_ap_api_request_stream_013b, (INT8U *)&multi_ap_api_request_structure_013);

#define MULTIAP_API_REQUEST_PARSE027 "MULTIAP_API_REQUEST_PARSE027 - Parse UNASSOC_STA_METRIC_QUERY TLV (multi_ap_api_request_stream_014)"
	result += _checkTrue(MULTIAP_API_REQUEST_PARSE027, multi_ap_api_request_stream_014, (INT8U *)&multi_ap_api_request_structure_014);

#define MULTIAP_API_REQUEST_PARSE028 "MULTIAP_API_REQUEST_PARSE028 - Parse UNASSOC_STA_METRIC_QUERY TLV (multi_ap_api_request_stream_014b)"
	result += _checkFalse(MULTIAP_API_REQUEST_PARSE028, multi_ap_api_request_stream_014b, (INT8U *)&multi_ap_api_request_structure_014);

#define MULTIAP_API_REQUEST_PARSE029 "MULTIAP_API_REQUEST_PARSE029 - Parse BEACON_METRIC_QUERY TLV (multi_ap_api_request_stream_015)"
	result += _checkTrue(MULTIAP_API_REQUEST_PARSE029, multi_ap_api_request_stream_015, (INT8U *)&multi_ap_api_request_structure_015);

#define MULTIAP_API_REQUEST_PARSE030 "MULTIAP_API_REQUEST_PARSE030 - Parse BEACON_METRIC_QUERY TLV (multi_ap_api_request_stream_015b)"
	result += _checkFalse(MULTIAP_API_REQUEST_PARSE030, multi_ap_api_request_stream_015b, (INT8U *)&multi_ap_api_request_structure_015);

	// Return the number of test cases that failed
	//
	return result;
}
