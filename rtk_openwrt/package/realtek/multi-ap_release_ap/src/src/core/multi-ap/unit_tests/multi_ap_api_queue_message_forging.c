/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

//
// This file tests the "_forge_request_element()" function by
// providing some test input structures and checking the generated output
// stream.
//
#include <stdio.h>
#include <string.h>
#include "../map_utils.h"

#include "../map_commands.h"

#include "multi_ap_api_queue_message_test_vectors.h"

INT8U _check(const char *test_description, INT8U mode, INT8U *input, INT8U *expected_output, INT16U expected_output_len)
{
	INT8U  result;
	INT8U *real_output;
	INT16U real_output_len;

	// Build the packet
	real_output = _forge_request_element(input, &real_output_len);

	if (NULL == real_output) {
		printf("%-100s: KO !!!\n", test_description);
		printf("  _forge_request_element() returned a NULL pointer\n");

		return 1;
	}

	// Compare packets
	if (mode == CHECK_TRUE) {
		// Compare the packets
		if ((expected_output_len == real_output_len) && (0 == memcmp(expected_output, real_output, real_output_len))) {
			result = 0;
			printf("%-100s: OK\n", test_description);
		} else {
			INT16U i;

			result = 1;
			printf("%-100s: KO !!!\n", test_description);

			// CheckTrue error needs more debug info
			//
			printf("  Expected output: ");
			for (i = 0; i < expected_output_len; i++) {
				printf("%02x ", expected_output[i]);
			}
			printf("\n");
			printf("  Real output    : ");
			for (i = 0; i < real_output_len; i++) {
				printf("%02x ", real_output[i]);
			}
			printf("\n");
		}
	} else {
		if ((expected_output_len == real_output_len) && (0 == memcmp(expected_output, real_output, real_output_len))) {
			result = 1;
			printf("%-100s: KO !!!\n", test_description);

		} else {
			result = 0;
			printf("%-100s: OK\n", test_description);
		}
	}

	return result;
}

INT8U _checkTrue(const char *test_description, INT8U *input, INT8U *expected_output, INT16U expected_output_len)
{
	return _check(test_description, CHECK_TRUE, input, expected_output, expected_output_len);
}

INT8U _checkFalse(const char *test_description, INT8U *input, INT8U *expected_output, INT16U expected_output_len)
{
	return _check(test_description, CHECK_FALSE, input, expected_output, expected_output_len);
}

int main(void)
{
	INT8U result = 0;

#define MULTIAP_API_REQUEST_FORGE001 "MULTIAP_API_REQUEST_FORGE001 - Forge TOPOLOGY_QUERY_REQUEST Request (multi_ap_api_request_stream_001)"
	result += _checkTrue(MULTIAP_API_REQUEST_FORGE001, (INT8U *)&multi_ap_api_request_structure_001, multi_ap_api_request_stream_001, multi_ap_api_request_stream_len_001);

#define MULTIAP_API_REQUEST_FORGE002 "MULTIAP_API_REQUEST_FORGE002 - Forge TOPOLOGY_QUERY_REQUEST Request (multi_ap_api_request_stream_001b)"
	result += _checkFalse(MULTIAP_API_REQUEST_FORGE002, (INT8U *)&multi_ap_api_request_structure_001, multi_ap_api_request_stream_001b, multi_ap_api_request_stream_len_001);

#define MULTIAP_API_REQUEST_FORGE003 "MULTIAP_API_REQUEST_FORGE003 - Forge LINK_METRIC_QUERY Request (multi_ap_api_request_stream_002)"
	result += _checkTrue(MULTIAP_API_REQUEST_FORGE003, (INT8U *)&multi_ap_api_request_structure_002, multi_ap_api_request_stream_002, multi_ap_api_request_stream_len_002);

#define MULTIAP_API_REQUEST_FORGE004 "MULTIAP_API_REQUEST_FORGE004 - Forge LINK_METRIC_QUERY Request (multi_ap_api_request_stream_002b)"
	result += _checkFalse(MULTIAP_API_REQUEST_FORGE004, (INT8U *)&multi_ap_api_request_structure_002, multi_ap_api_request_stream_002b, multi_ap_api_request_stream_len_002);

#define MULTIAP_API_REQUEST_FORGE005 "MULTIAP_API_REQUEST_FORGE005 - Forge AUTOCONFIG_RENEW_REQUEST Request (multi_ap_api_request_stream_003)"
	result += _checkTrue(MULTIAP_API_REQUEST_FORGE005, (INT8U *)&multi_ap_api_request_structure_003, multi_ap_api_request_stream_003, multi_ap_api_request_stream_len_003);

#define MULTIAP_API_REQUEST_FORGE006 "MULTIAP_API_REQUEST_FORGE006 - Forge AUTOCONFIG_RENEW_REQUEST Request (multi_ap_api_request_stream_003b)"
	result += _checkFalse(MULTIAP_API_REQUEST_FORGE006, (INT8U *)&multi_ap_api_request_structure_003, multi_ap_api_request_stream_003b, multi_ap_api_request_stream_len_003);

#define MULTIAP_API_REQUEST_FORGE007 "MULTIAP_API_REQUEST_FORGE007 - Forge CHANNEL_PREFERENCE_QUERY Request (multi_ap_api_request_stream_004)"
	result += _checkTrue(MULTIAP_API_REQUEST_FORGE007, (INT8U *)&multi_ap_api_request_structure_004, multi_ap_api_request_stream_004, multi_ap_api_request_stream_len_004);

#define MULTIAP_API_REQUEST_FORGE008 "MULTIAP_API_REQUEST_FORGE008 - Forge CHANNEL_PREFERENCE_QUERY Request (multi_ap_api_request_stream_004b)"
	result += _checkFalse(MULTIAP_API_REQUEST_FORGE008, (INT8U *)&multi_ap_api_request_structure_004, multi_ap_api_request_stream_004b, multi_ap_api_request_stream_len_004);

#define MULTIAP_API_REQUEST_FORGE009 "MULTIAP_API_REQUEST_FORGE009 - Forge CHANNEL_SELECTION_REQUEST Request (multi_ap_api_request_stream_005)"
	result += _checkTrue(MULTIAP_API_REQUEST_FORGE009, (INT8U *)&multi_ap_api_request_structure_005, multi_ap_api_request_stream_005, multi_ap_api_request_stream_len_005);

#define MULTIAP_API_REQUEST_FORGE010 "MULTIAP_API_REQUEST_FORGE010 - Forge CHANNEL_SELECTION_REQUEST Request (multi_ap_api_request_stream_005b)"
	result += _checkFalse(MULTIAP_API_REQUEST_FORGE010, (INT8U *)&multi_ap_api_request_structure_005, multi_ap_api_request_stream_005b, multi_ap_api_request_stream_len_005);

#define MULTIAP_API_REQUEST_FORGE011 "MULTIAP_API_REQUEST_FORGE011 - Forge CLIENT_STEERING_REQUEST Request (multi_ap_api_request_stream_006)"
	result += _checkTrue(MULTIAP_API_REQUEST_FORGE011, (INT8U *)&multi_ap_api_request_structure_006, multi_ap_api_request_stream_006, multi_ap_api_request_stream_len_006);

#define MULTIAP_API_REQUEST_FORGE012 "MULTIAP_API_REQUEST_FORGE012 - Forge CLIENT_STEERING_REQUEST Request (multi_ap_api_request_stream_006b)"
	result += _checkFalse(MULTIAP_API_REQUEST_FORGE012, (INT8U *)&multi_ap_api_request_structure_006, multi_ap_api_request_stream_006b, multi_ap_api_request_stream_len_006);

#define MULTIAP_API_REQUEST_FORGE013 "MULTIAP_API_REQUEST_FORGE013 - Forge CLIENT_ASSOCIATION_CONTROL_REQUEST Request (multi_ap_api_request_stream_007)"
	result += _checkTrue(MULTIAP_API_REQUEST_FORGE013, (INT8U *)&multi_ap_api_request_structure_007, multi_ap_api_request_stream_007, multi_ap_api_request_stream_len_007);

#define MULTIAP_API_REQUEST_FORGE014 "MULTIAP_API_REQUEST_FORGE014 - Forge CLIENT_ASSOCIATION_CONTROL_REQUEST Request (multi_ap_api_request_stream_007b)"
	result += _checkFalse(MULTIAP_API_REQUEST_FORGE014, (INT8U *)&multi_ap_api_request_structure_007, multi_ap_api_request_stream_007b, multi_ap_api_request_stream_len_007);

#define MULTIAP_API_REQUEST_FORGE015 "MULTIAP_API_REQUEST_FORGE015 - Forge BACKHAUL_STEERING_REQUEST Request (multi_ap_api_request_stream_008)"
	result += _checkTrue(MULTIAP_API_REQUEST_FORGE015, (INT8U *)&multi_ap_api_request_structure_008, multi_ap_api_request_stream_008, multi_ap_api_request_stream_len_008);

#define MULTIAP_API_REQUEST_FORGE016 "MULTIAP_API_REQUEST_FORGE016 - Forge BACKHAUL_STEERING_REQUEST Request (multi_ap_api_request_stream_008b)"
	result += _checkFalse(MULTIAP_API_REQUEST_FORGE016, (INT8U *)&multi_ap_api_request_structure_008, multi_ap_api_request_stream_008b, multi_ap_api_request_stream_len_008);

#define MULTIAP_API_REQUEST_FORGE017 "MULTIAP_API_REQUEST_FORGE017 - Forge POLICY_CONFIG_REQUEST Request (multi_ap_api_request_stream_009)"
	result += _checkTrue(MULTIAP_API_REQUEST_FORGE017, (INT8U *)&multi_ap_api_request_structure_009, multi_ap_api_request_stream_009, multi_ap_api_request_stream_len_009);

#define MULTIAP_API_REQUEST_FORGE018 "MULTIAP_API_REQUEST_FORGE018 - Forge POLICY_CONFIG_REQUEST Request (multi_ap_api_request_stream_009b)"
	result += _checkFalse(MULTIAP_API_REQUEST_FORGE018, (INT8U *)&multi_ap_api_request_structure_009, multi_ap_api_request_stream_009b, multi_ap_api_request_stream_len_009);

#define MULTIAP_API_REQUEST_FORGE019 "MULTIAP_API_REQUEST_FORGE019 - Forge AP_CAPABILITY_QUERY Request (multi_ap_api_request_stream_010)"
	result += _checkTrue(MULTIAP_API_REQUEST_FORGE019, (INT8U *)&multi_ap_api_request_structure_010, multi_ap_api_request_stream_010, multi_ap_api_request_stream_len_010);

#define MULTIAP_API_REQUEST_FORGE020 "MULTIAP_API_REQUEST_FORGE020 - Forge AP_CAPABILITY_QUERY Request (multi_ap_api_request_stream_010b)"
	result += _checkFalse(MULTIAP_API_REQUEST_FORGE020, (INT8U *)&multi_ap_api_request_structure_010, multi_ap_api_request_stream_010b, multi_ap_api_request_stream_len_010);

#define MULTIAP_API_REQUEST_FORGE021 "MULTIAP_API_REQUEST_FORGE021 - Forge CLIENT_CAPABILITY_QUERY Request (multi_ap_api_request_stream_011)"
	result += _checkTrue(MULTIAP_API_REQUEST_FORGE021, (INT8U *)&multi_ap_api_request_structure_011, multi_ap_api_request_stream_011, multi_ap_api_request_stream_len_011);

#define MULTIAP_API_REQUEST_FORGE022 "MULTIAP_API_REQUEST_FORGE022 - Forge CLIENT_CAPABILITY_QUERY Request (multi_ap_api_request_stream_011b)"
	result += _checkFalse(MULTIAP_API_REQUEST_FORGE022, (INT8U *)&multi_ap_api_request_structure_011, multi_ap_api_request_stream_011b, multi_ap_api_request_stream_len_011);

#define MULTIAP_API_REQUEST_FORGE023 "MULTIAP_API_REQUEST_FORGE023 - Forge AP_METRIC_QUERY Request (multi_ap_api_request_stream_012)"
	result += _checkTrue(MULTIAP_API_REQUEST_FORGE023, (INT8U *)&multi_ap_api_request_structure_012, multi_ap_api_request_stream_012, multi_ap_api_request_stream_len_012);

#define MULTIAP_API_REQUEST_FORGE024 "MULTIAP_API_REQUEST_FORGE024 - Forge AP_METRIC_QUERY Request (multi_ap_api_request_stream_012b)"
	result += _checkFalse(MULTIAP_API_REQUEST_FORGE024, (INT8U *)&multi_ap_api_request_structure_012, multi_ap_api_request_stream_012b, multi_ap_api_request_stream_len_012);

#define MULTIAP_API_REQUEST_FORGE025 "MULTIAP_API_REQUEST_FORGE025 - Forge ASSOC_STA_METRIC_QUERY Request (multi_ap_api_request_stream_013)"
	result += _checkTrue(MULTIAP_API_REQUEST_FORGE025, (INT8U *)&multi_ap_api_request_structure_013, multi_ap_api_request_stream_013, multi_ap_api_request_stream_len_013);

#define MULTIAP_API_REQUEST_FORGE026 "MULTIAP_API_REQUEST_FORGE026 - Forge ASSOC_STA_METRIC_QUERY Request (multi_ap_api_request_stream_013b)"
	result += _checkFalse(MULTIAP_API_REQUEST_FORGE026, (INT8U *)&multi_ap_api_request_structure_013, multi_ap_api_request_stream_013b, multi_ap_api_request_stream_len_013);

#define MULTIAP_API_REQUEST_FORGE027 "MULTIAP_API_REQUEST_FORGE027 - Forge UNASSOC_STA_METRIC_QUERY Request (multi_ap_api_request_stream_014)"
	result += _checkTrue(MULTIAP_API_REQUEST_FORGE027, (INT8U *)&multi_ap_api_request_structure_014, multi_ap_api_request_stream_014, multi_ap_api_request_stream_len_014);

#define MULTIAP_API_REQUEST_FORGE028 "MULTIAP_API_REQUEST_FORGE028 - Forge UNASSOC_STA_METRIC_QUERY Request (multi_ap_api_request_stream_014b)"
	result += _checkFalse(MULTIAP_API_REQUEST_FORGE028, (INT8U *)&multi_ap_api_request_structure_014, multi_ap_api_request_stream_014b, multi_ap_api_request_stream_len_014);

#define MULTIAP_API_REQUEST_FORGE029 "MULTIAP_API_REQUEST_FORGE029 - Forge BEACON_METRIC_QUERY Request (multi_ap_api_request_stream_015)"
	result += _checkTrue(MULTIAP_API_REQUEST_FORGE029, (INT8U *)&multi_ap_api_request_structure_015, multi_ap_api_request_stream_015, multi_ap_api_request_stream_len_015);

#define MULTIAP_API_REQUEST_FORGE030 "MULTIAP_API_REQUEST_FORGE030 - Forge BEACON_METRIC_QUERY Request (multi_ap_api_request_stream_015b)"
	result += _checkFalse(MULTIAP_API_REQUEST_FORGE030, (INT8U *)&multi_ap_api_request_structure_015, multi_ap_api_request_stream_015b, multi_ap_api_request_stream_len_015);

	// Return the number of test cases that failed
	//
	return result;
}
