/*
 * Copyright (C) 2018 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

//
// This file tests the "forge_non_1905_TLV_from_structure()" function by
// providing some test input structures and checking the generated output
// stream.
//

#include "platform.h"
#include "multi_ap_tlvs.h"
#include "multi_ap_tlvs_r2.h"
#include "multi_ap_tlvs_r3.h"
#include "multi_ap_tlvs_r4.h"

#include "multi_ap_tlv_test_vectors.h"

INT8U _check(const char *test_description, INT8U mode, INT8U *input, INT8U *expected_output, INT16U expected_output_len)
{
	INT8U  result;
	INT8U *real_output;
	INT16U real_output_len;

	// Build the packet
	real_output = forge_multi_ap_TLV_from_structure((INT8U *)input, &real_output_len);

	if (NULL == real_output) {
		PLATFORM_PRINTF("%-100s: KO !!!\n", test_description);
		PLATFORM_PRINTF("  forge_non_1905_TLV_from_structure() returned a NULL pointer\n");

		return 1;
	}

	// Compare packets
	if (mode == CHECK_TRUE) {
		// Compare the packets
		if ((expected_output_len == real_output_len) && (0 == PLATFORM_MEMCMP(expected_output, real_output, real_output_len))) {
			result = 0;
			PLATFORM_PRINTF("%-100s: OK\n", test_description);
		} else {
			INT16U i;

			result = 1;
			PLATFORM_PRINTF("%-100s: KO !!!\n", test_description);

			// CheckTrue error needs more debug info
			//
			PLATFORM_PRINTF("  Expected output: ");
			for (i = 0; i < expected_output_len; i++) {
				PLATFORM_PRINTF("%02x ", expected_output[i]);
			}
			PLATFORM_PRINTF("\n");
			PLATFORM_PRINTF("  Real output    : ");
			for (i = 0; i < real_output_len; i++) {
				PLATFORM_PRINTF("%02x ", real_output[i]);
			}
			PLATFORM_PRINTF("\n");
		}
	} else {
		if ((expected_output_len == real_output_len) && (0 == PLATFORM_MEMCMP(expected_output, real_output, real_output_len))) {
			result = 1;
			PLATFORM_PRINTF("%-100s: KO !!!\n", test_description);

		} else {
			result = 0;
			PLATFORM_PRINTF("%-100s: OK\n", test_description);
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

#define MULTIAP_TLVFORGE001 "MULTIAP_TLVFORGE001 - Forge SupportedServiceTLV TLV (multi_ap_tlv_stream_001)"
	result += _checkTrue(MULTIAP_TLVFORGE001, (INT8U *)&multi_ap_tlv_structure_001, multi_ap_tlv_stream_001, multi_ap_tlv_stream_len_001);

#define MULTIAP_TLVFORGE002 "MULTIAP_TLVFORGE002 - Forge SupportedServiceTLV TLV (multi_ap_tlv_stream_001b)"
	result += _checkFalse(MULTIAP_TLVFORGE002, (INT8U *)&multi_ap_tlv_structure_001, multi_ap_tlv_stream_001b, multi_ap_tlv_stream_len_001);

#define MULTIAP_TLVFORGE003 "MULTIAP_TLVFORGE003 - Forge SearchedServiceTLV TLV (multi_ap_tlv_stream_002)"
	result += _checkTrue(MULTIAP_TLVFORGE003, (INT8U *)&multi_ap_tlv_structure_002, multi_ap_tlv_stream_002, multi_ap_tlv_stream_len_002);

#define MULTIAP_TLVFORGE004 "MULTIAP_TLVFORGE004 - Forge SearchedServiceTLV TLV (multi_ap_tlv_stream_002b)"
	result += _checkFalse(MULTIAP_TLVFORGE004, (INT8U *)&multi_ap_tlv_structure_002, multi_ap_tlv_stream_002b, multi_ap_tlv_stream_len_002);

#define MULTIAP_TLVFORGE005 "MULTIAP_TLVFORGE005 - Forge ApRadioIdentifierTLV TLV (multi_ap_tlv_stream_003)"
	result += _checkTrue(MULTIAP_TLVFORGE005, (INT8U *)&multi_ap_tlv_structure_003, multi_ap_tlv_stream_003, multi_ap_tlv_stream_len_003);

#define MULTIAP_TLVFORGE006 "MULTIAP_TLVFORGE006 - Forge ApRadioIdentifierTLV TLV (multi_ap_tlv_stream_003b)"
	result += _checkFalse(MULTIAP_TLVFORGE006, (INT8U *)&multi_ap_tlv_structure_003, multi_ap_tlv_stream_003b, multi_ap_tlv_stream_len_003);

#define MULTIAP_TLVFORGE007 "MULTIAP_TLVFORGE007 - Forge ApOperationalBSSTLV TLV (multi_ap_tlv_stream_004)"
	result += _checkTrue(MULTIAP_TLVFORGE007, (INT8U *)&multi_ap_tlv_structure_004, multi_ap_tlv_stream_004, multi_ap_tlv_stream_len_004);

#define MULTIAP_TLVFORGE008 "MULTIAP_TLVFORGE008 - Forge ApOperationalBSSTLV TLV (multi_ap_tlv_stream_004b)"
	result += _checkFalse(MULTIAP_TLVFORGE008, (INT8U *)&multi_ap_tlv_structure_004, multi_ap_tlv_stream_004b, multi_ap_tlv_stream_len_004);

#define MULTIAP_TLVFORGE009 "MULTIAP_TLVFORGE009 - Forge AssociatedClientsTLV TLV (multi_ap_tlv_stream_005)"
	result += _checkTrue(MULTIAP_TLVFORGE009, (INT8U *)&multi_ap_tlv_structure_005, multi_ap_tlv_stream_005, multi_ap_tlv_stream_len_005);

#define MULTIAP_TLVFORGE010 "MULTIAP_TLVFORGE010 - Forge AssociatedClientsTLV TLV (multi_ap_tlv_stream_005b)"
	result += _checkFalse(MULTIAP_TLVFORGE010, (INT8U *)&multi_ap_tlv_structure_005, multi_ap_tlv_stream_005b, multi_ap_tlv_stream_len_005);

#define MULTIAP_TLVFORGE011 "MULTIAP_TLVFORGE011 - Forge APCapabilityTLV TLV (multi_ap_tlv_stream_006)"
	result += _checkTrue(MULTIAP_TLVFORGE011, (INT8U *)&multi_ap_tlv_structure_006, multi_ap_tlv_stream_006, multi_ap_tlv_stream_len_006);

#define MULTIAP_TLVFORGE012 "MULTIAP_TLVFORGE012 - Forge APCapabilityTLV TLV (multi_ap_tlv_stream_006b)"
	result += _checkFalse(MULTIAP_TLVFORGE012, (INT8U *)&multi_ap_tlv_structure_006, multi_ap_tlv_stream_006b, multi_ap_tlv_stream_len_006);

#define MULTIAP_TLVFORGE013 "MULTIAP_TLVFORGE013 - Forge APRadioBasicCapabilitiesTLV TLV (multi_ap_tlv_stream_007)"
	result += _checkTrue(MULTIAP_TLVFORGE013, (INT8U *)&multi_ap_tlv_structure_007, multi_ap_tlv_stream_007, multi_ap_tlv_stream_len_007);

#define MULTIAP_TLVFORGE014 "MULTIAP_TLVFORGE014 - Forge APRadioBasicCapabilitiesTLV TLV (multi_ap_tlv_stream_007b)"
	result += _checkFalse(MULTIAP_TLVFORGE014, (INT8U *)&multi_ap_tlv_structure_007, multi_ap_tlv_stream_007b, multi_ap_tlv_stream_len_007);

#define MULTIAP_TLVFORGE015 "MULTIAP_TLVFORGE015 - Forge APHTCapabilitiesTLV TLV (multi_ap_tlv_stream_008)"
	result += _checkTrue(MULTIAP_TLVFORGE015, (INT8U *)&multi_ap_tlv_structure_008, multi_ap_tlv_stream_008, multi_ap_tlv_stream_len_008);

#define MULTIAP_TLVFORGE016 "MULTIAP_TLVFORGE016 - Forge APHTCapabilitiesTLV TLV (multi_ap_tlv_stream_008b)"
	result += _checkFalse(MULTIAP_TLVFORGE016, (INT8U *)&multi_ap_tlv_structure_008, multi_ap_tlv_stream_008b, multi_ap_tlv_stream_len_008);

#define MULTIAP_TLVFORGE017 "MULTIAP_TLVFORGE017 - Forge APVHTCapabilitiesTLV TLV (multi_ap_tlv_stream_009)"
	result += _checkTrue(MULTIAP_TLVFORGE017, (INT8U *)&multi_ap_tlv_structure_009, multi_ap_tlv_stream_009, multi_ap_tlv_stream_len_009);

#define MULTIAP_TLVFORGE018 "MULTIAP_TLVFORGE018 - Forge APVHTCapabilitiesTLV TLV (multi_ap_tlv_stream_009b)"
	result += _checkFalse(MULTIAP_TLVFORGE018, (INT8U *)&multi_ap_tlv_structure_009, multi_ap_tlv_stream_009b, multi_ap_tlv_stream_len_009);

#define MULTIAP_TLVFORGE019 "MULTIAP_TLVFORGE019 - Forge APHECapabilitiesTLV TLV (multi_ap_tlv_stream_010)"
	result += _checkTrue(MULTIAP_TLVFORGE019, (INT8U *)&multi_ap_tlv_structure_010, multi_ap_tlv_stream_010, multi_ap_tlv_stream_len_010);

#define MULTIAP_TLVFORGE020 "MULTIAP_TLVFORGE020 - Forge APHECapabilitiesTLV TLV (multi_ap_tlv_stream_010b)"
	result += _checkFalse(MULTIAP_TLVFORGE020, (INT8U *)&multi_ap_tlv_structure_010, multi_ap_tlv_stream_010b, multi_ap_tlv_stream_len_010);

#define MULTIAP_TLVFORGE021 "MULTIAP_TLVFORGE021 - Forge ClientInfoTLV TLV (multi_ap_tlv_stream_011)"
	result += _checkTrue(MULTIAP_TLVFORGE021, (INT8U *)&multi_ap_tlv_structure_011, multi_ap_tlv_stream_011, multi_ap_tlv_stream_len_011);

#define MULTIAP_TLVFORGE022 "MULTIAP_TLVFORGE022 - Forge ClientInfoTLV TLV (multi_ap_tlv_stream_011b)"
	result += _checkFalse(MULTIAP_TLVFORGE022, (INT8U *)&multi_ap_tlv_structure_011, multi_ap_tlv_stream_011b, multi_ap_tlv_stream_len_011);

#define MULTIAP_TLVFORGE023 "MULTIAP_TLVFORGE023 - Forge ClientCapbilityReportTLV TLV (multi_ap_tlv_stream_012)"
	result += _checkTrue(MULTIAP_TLVFORGE023, (INT8U *)&multi_ap_tlv_structure_012, multi_ap_tlv_stream_012, multi_ap_tlv_stream_len_012);

#define MULTIAP_TLVFORGE024 "MULTIAP_TLVFORGE024 - Forge ClientCapbilityReportTLV TLV (multi_ap_tlv_stream_012b)"
	result += _checkFalse(MULTIAP_TLVFORGE024, (INT8U *)&multi_ap_tlv_structure_012, multi_ap_tlv_stream_012b, multi_ap_tlv_stream_len_012);

#define MULTIAP_TLVFORGE025 "MULTIAP_TLVFORGE025 - Forge ClientAssociationEventTLV TLV (multi_ap_tlv_stream_013)"
	result += _checkTrue(MULTIAP_TLVFORGE025, (INT8U *)&multi_ap_tlv_structure_013, multi_ap_tlv_stream_013, multi_ap_tlv_stream_len_013);

#define MULTIAP_TLVFORGE026 "MULTIAP_TLVFORGE026 - Forge ClientAssociationEventTLV TLV (multi_ap_tlv_stream_013b)"
	result += _checkFalse(MULTIAP_TLVFORGE026, (INT8U *)&multi_ap_tlv_structure_013, multi_ap_tlv_stream_013b, multi_ap_tlv_stream_len_013);

#define MULTIAP_TLVFORGE027 "MULTIAP_TLVFORGE027 - Forge STAMacAddressTypeTLV TLV (multi_ap_tlv_stream_014)"
	result += _checkTrue(MULTIAP_TLVFORGE027, (INT8U *)&multi_ap_tlv_structure_014, multi_ap_tlv_stream_014, multi_ap_tlv_stream_len_014);

#define MULTIAP_TLVFORGE028 "MULTIAP_TLVFORGE028 - Forge STAMacAddressTypeTLV TLV (multi_ap_tlv_stream_014b)"
	result += _checkFalse(MULTIAP_TLVFORGE028, (INT8U *)&multi_ap_tlv_structure_014, multi_ap_tlv_stream_014b, multi_ap_tlv_stream_len_014);

#define MULTIAP_TLVFORGE029 "MULTIAP_TLVFORGE029 - Forge ClientAssociationControlRequestTLV TLV (multi_ap_tlv_stream_015)"
	result += _checkTrue(MULTIAP_TLVFORGE029, (INT8U *)&multi_ap_tlv_structure_015, multi_ap_tlv_stream_015, multi_ap_tlv_stream_len_015);

#define MULTIAP_TLVFORGE030 "MULTIAP_TLVFORGE030 - Forge ClientAssociationControlRequestTLV TLV (multi_ap_tlv_stream_015b)"
	result += _checkFalse(MULTIAP_TLVFORGE030, (INT8U *)&multi_ap_tlv_structure_015, multi_ap_tlv_stream_015b, multi_ap_tlv_stream_len_015);

#define MULTIAP_TLVFORGE031 "MULTIAP_TLVFORGE031 - Forge ErrorCodeTLV TLV (multi_ap_tlv_stream_016)"
	result += _checkTrue(MULTIAP_TLVFORGE031, (INT8U *)&multi_ap_tlv_structure_016, multi_ap_tlv_stream_016, multi_ap_tlv_stream_len_016);

#define MULTIAP_TLVFORGE032 "MULTIAP_TLVFORGE032 - Forge ErrorCodeTLV TLV (multi_ap_tlv_stream_016b)"
	result += _checkFalse(MULTIAP_TLVFORGE032, (INT8U *)&multi_ap_tlv_structure_016, multi_ap_tlv_stream_016b, multi_ap_tlv_stream_len_016);

#define MULTIAP_TLVFORGE033 "MULTIAP_TLVFORGE033 - Forge SteeringPolicyTLV TLV (multi_ap_tlv_stream_017)"
	result += _checkTrue(MULTIAP_TLVFORGE033, (INT8U *)&multi_ap_tlv_structure_017, multi_ap_tlv_stream_017, multi_ap_tlv_stream_len_017);

#define MULTIAP_TLVFORGE034 "MULTIAP_TLVFORGE034 - Forge SteeringPolicyTLV TLV (multi_ap_tlv_stream_017b)"
	result += _checkFalse(MULTIAP_TLVFORGE034, (INT8U *)&multi_ap_tlv_structure_017, multi_ap_tlv_stream_017b, multi_ap_tlv_stream_len_017);

#define MULTIAP_TLVFORGE035 "MULTIAP_TLVFORGE035 - Forge MetricReportingPolicyTLV TLV (multi_ap_tlv_stream_018)"
	result += _checkTrue(MULTIAP_TLVFORGE035, (INT8U *)&multi_ap_tlv_structure_018, multi_ap_tlv_stream_018, multi_ap_tlv_stream_len_018);

#define MULTIAP_TLVFORGE036 "MULTIAP_TLVFORGE036 - Forge MetricReportingPolicyTLV TLV (multi_ap_tlv_stream_018b)"
	result += _checkFalse(MULTIAP_TLVFORGE036, (INT8U *)&multi_ap_tlv_structure_018, multi_ap_tlv_stream_018b, multi_ap_tlv_stream_len_018);

#define MULTIAP_TLVFORGE037 "MULTIAP_TLVFORGE037 - Forge ChannelPreferenceTLV TLV (multi_ap_tlv_stream_019)"
	result += _checkTrue(MULTIAP_TLVFORGE037, (INT8U *)&multi_ap_tlv_structure_019, multi_ap_tlv_stream_019, multi_ap_tlv_stream_len_019);

#define MULTIAP_TLVFORGE038 "MULTIAP_TLVFORGE038 - Forge ChannelPreferenceTLV TLV (multi_ap_tlv_stream_019b)"
	result += _checkFalse(MULTIAP_TLVFORGE038, (INT8U *)&multi_ap_tlv_structure_019, multi_ap_tlv_stream_019b, multi_ap_tlv_stream_len_019);

#define MULTIAP_TLVFORGE039 "MULTIAP_TLVFORGE039 - Forge RadioOperationRestrictionTLV TLV (multi_ap_tlv_stream_020)"
	result += _checkTrue(MULTIAP_TLVFORGE039, (INT8U *)&multi_ap_tlv_structure_020, multi_ap_tlv_stream_020, multi_ap_tlv_stream_len_020);

#define MULTIAP_TLVFORGE040 "MULTIAP_TLVFORGE040 - Forge RadioOperationRestrictionTLV TLV (multi_ap_tlv_stream_020b)"
	result += _checkFalse(MULTIAP_TLVFORGE040, (INT8U *)&multi_ap_tlv_structure_020, multi_ap_tlv_stream_020b, multi_ap_tlv_stream_len_020);

#define MULTIAP_TLVFORGE041 "MULTIAP_TLVFORGE041 - Forge TransmitPowerLimitTLV TLV (multi_ap_tlv_stream_021)"
	result += _checkTrue(MULTIAP_TLVFORGE041, (INT8U *)&multi_ap_tlv_structure_021, multi_ap_tlv_stream_021, multi_ap_tlv_stream_len_021);

#define MULTIAP_TLVFORGE042 "MULTIAP_TLVFORGE042 - Forge TransmitPowerLimitTLV TLV (multi_ap_tlv_stream_021b)"
	result += _checkFalse(MULTIAP_TLVFORGE042, (INT8U *)&multi_ap_tlv_structure_021, multi_ap_tlv_stream_021b, multi_ap_tlv_stream_len_021);

#define MULTIAP_TLVFORGE043 "MULTIAP_TLVFORGE043 - Forge ChannelSelectionResponseTLV TLV (multi_ap_tlv_stream_022)"
	result += _checkTrue(MULTIAP_TLVFORGE043, (INT8U *)&multi_ap_tlv_structure_022, multi_ap_tlv_stream_022, multi_ap_tlv_stream_len_022);

#define MULTIAP_TLVFORGE044 "MULTIAP_TLVFORGE044 - Forge ChannelSelectionResponseTLV TLV (multi_ap_tlv_stream_022b)"
	result += _checkFalse(MULTIAP_TLVFORGE044, (INT8U *)&multi_ap_tlv_structure_022, multi_ap_tlv_stream_022b, multi_ap_tlv_stream_len_022);

#define MULTIAP_TLVFORGE045 "MULTIAP_TLVFORGE045 - Forge OperatingChannelReportTLV TLV (multi_ap_tlv_stream_023)"
	result += _checkTrue(MULTIAP_TLVFORGE045, (INT8U *)&multi_ap_tlv_structure_023, multi_ap_tlv_stream_023, multi_ap_tlv_stream_len_023);

#define MULTIAP_TLVFORGE046 "MULTIAP_TLVFORGE046 - Forge OperatingChannelReportTLV TLV (multi_ap_tlv_stream_023b)"
	result += _checkFalse(MULTIAP_TLVFORGE046, (INT8U *)&multi_ap_tlv_structure_023, multi_ap_tlv_stream_023b, multi_ap_tlv_stream_len_023);

#define MULTIAP_TLVFORGE047 "MULTIAP_TLVFORGE047 - Forge APMetricQueryTLV TLV (multi_ap_tlv_stream_024)"
	result += _checkTrue(MULTIAP_TLVFORGE047, (INT8U *)&multi_ap_tlv_structure_024, multi_ap_tlv_stream_024, multi_ap_tlv_stream_len_024);

#define MULTIAP_TLVFORGE048 "MULTIAP_TLVFORGE048 - Forge APMetricQueryTLV TLV (multi_ap_tlv_stream_024b)"
	result += _checkFalse(MULTIAP_TLVFORGE048, (INT8U *)&multi_ap_tlv_structure_024, multi_ap_tlv_stream_024b, multi_ap_tlv_stream_len_024);

#define MULTIAP_TLVFORGE049 "MULTIAP_TLVFORGE049 - Forge APMetricsTLV TLV (multi_ap_tlv_stream_025)"
	result += _checkTrue(MULTIAP_TLVFORGE049, (INT8U *)&multi_ap_tlv_structure_025, multi_ap_tlv_stream_025, multi_ap_tlv_stream_len_025);

#define MULTIAP_TLVFORGE050 "MULTIAP_TLVFORGE050 - Forge APMetricsTLV TLV (multi_ap_tlv_stream_025b)"
	result += _checkFalse(MULTIAP_TLVFORGE050, (INT8U *)&multi_ap_tlv_structure_025, multi_ap_tlv_stream_025b, multi_ap_tlv_stream_len_025);

#define MULTIAP_TLVFORGE051 "MULTIAP_TLVFORGE051 - Forge AssociatedSTALinkMetricsTLV TLV (multi_ap_tlv_stream_026)"
	result += _checkTrue(MULTIAP_TLVFORGE051, (INT8U *)&multi_ap_tlv_structure_026, multi_ap_tlv_stream_026, multi_ap_tlv_stream_len_026);

#define MULTIAP_TLVFORGE052 "MULTIAP_TLVFORGE052 - Forge AssociatedSTALinkMetricsTLV TLV (multi_ap_tlv_stream_026b)"
	result += _checkFalse(MULTIAP_TLVFORGE052, (INT8U *)&multi_ap_tlv_structure_026, multi_ap_tlv_stream_026b, multi_ap_tlv_stream_len_026);

#define MULTIAP_TLVFORGE053 "MULTIAP_TLVFORGE053 - Forge UnassociatedSTALinkMetricsQueryTLV TLV (multi_ap_tlv_stream_027)"
	result += _checkTrue(MULTIAP_TLVFORGE053, (INT8U *)&multi_ap_tlv_structure_027, multi_ap_tlv_stream_027, multi_ap_tlv_stream_len_027);

#define MULTIAP_TLVFORGE054 "MULTIAP_TLVFORGE054 - Forge UnassociatedSTALinkMetricsQueryTLV TLV (multi_ap_tlv_stream_027b)"
	result += _checkFalse(MULTIAP_TLVFORGE054, (INT8U *)&multi_ap_tlv_structure_027, multi_ap_tlv_stream_027b, multi_ap_tlv_stream_len_027);

#define MULTIAP_TLVFORGE055 "MULTIAP_TLVFORGE055 - Forge UnassociatedSTALinkMetricsResponseTLV TLV (multi_ap_tlv_stream_028)"
	result += _checkTrue(MULTIAP_TLVFORGE055, (INT8U *)&multi_ap_tlv_structure_028, multi_ap_tlv_stream_028, multi_ap_tlv_stream_len_028);

#define MULTIAP_TLVFORGE056 "MULTIAP_TLVFORGE056 - Forge UnassociatedSTALinkMetricsResponseTLV TLV (multi_ap_tlv_stream_028b)"
	result += _checkFalse(MULTIAP_TLVFORGE056, (INT8U *)&multi_ap_tlv_structure_028, multi_ap_tlv_stream_028b, multi_ap_tlv_stream_len_028);

#define MULTIAP_TLVFORGE057 "MULTIAP_TLVFORGE057 - Forge BeaconMetricsQueryTLV TLV (multi_ap_tlv_stream_029)"
	result += _checkTrue(MULTIAP_TLVFORGE057, (INT8U *)&multi_ap_tlv_structure_029, multi_ap_tlv_stream_029, multi_ap_tlv_stream_len_029);

#define MULTIAP_TLVFORGE058 "MULTIAP_TLVFORGE058 - Forge BeaconMetricsQueryTLV TLV (multi_ap_tlv_stream_029b)"
	result += _checkFalse(MULTIAP_TLVFORGE058, (INT8U *)&multi_ap_tlv_structure_029, multi_ap_tlv_stream_029b, multi_ap_tlv_stream_len_029);

#define MULTIAP_TLVFORGE059 "MULTIAP_TLVFORGE059 - Forge BeaconMetricsResponseTLV TLV (multi_ap_tlv_stream_030)"
	result += _checkTrue(MULTIAP_TLVFORGE059, (INT8U *)&multi_ap_tlv_structure_030, multi_ap_tlv_stream_030, multi_ap_tlv_stream_len_030);

#define MULTIAP_TLVFORGE060 "MULTIAP_TLVFORGE060 - Forge BeaconMetricsResponseTLV TLV (multi_ap_tlv_stream_030b)"
	result += _checkFalse(MULTIAP_TLVFORGE060, (INT8U *)&multi_ap_tlv_structure_030, multi_ap_tlv_stream_030b, multi_ap_tlv_stream_len_030);

#define MULTIAP_TLVFORGE061 "MULTIAP_TLVFORGE061 - Forge SteeringRequestTLV TLV (multi_ap_tlv_stream_031)"
	result += _checkTrue(MULTIAP_TLVFORGE061, (INT8U *)&multi_ap_tlv_structure_031, multi_ap_tlv_stream_031, multi_ap_tlv_stream_len_031);

#define MULTIAP_TLVFORGE062 "MULTIAP_TLVFORGE062 - Forge SteeringRequestTLV TLV (multi_ap_tlv_stream_031b)"
	result += _checkFalse(MULTIAP_TLVFORGE062, (INT8U *)&multi_ap_tlv_structure_031, multi_ap_tlv_stream_031b, multi_ap_tlv_stream_len_031);

#define MULTIAP_TLVFORGE063 "MULTIAP_TLVFORGE063 - Forge SteeringBTMReportTLV TLV (multi_ap_tlv_stream_032)"
	result += _checkTrue(MULTIAP_TLVFORGE063, (INT8U *)&multi_ap_tlv_structure_032, multi_ap_tlv_stream_032, multi_ap_tlv_stream_len_032);

#define MULTIAP_TLVFORGE064 "MULTIAP_TLVFORGE064 - Forge SteeringBTMReportTLV TLV (multi_ap_tlv_stream_032b)"
	result += _checkFalse(MULTIAP_TLVFORGE064, (INT8U *)&multi_ap_tlv_structure_032, multi_ap_tlv_stream_032b, multi_ap_tlv_stream_len_032);

#define MULTIAP_TLVFORGE065 "MULTIAP_TLVFORGE065 - Forge SteeringBTMReportTLV TLV (multi_ap_tlv_stream_033)"
	result += _checkTrue(MULTIAP_TLVFORGE065, (INT8U *)&multi_ap_tlv_structure_033, multi_ap_tlv_stream_033, multi_ap_tlv_stream_len_033);

#define MULTIAP_TLVFORGE066 "MULTIAP_TLVFORGE066 - Forge SteeringBTMReportTLV TLV (multi_ap_tlv_stream_033b)"
	result += _checkFalse(MULTIAP_TLVFORGE066, (INT8U *)&multi_ap_tlv_structure_033, multi_ap_tlv_stream_033b, multi_ap_tlv_stream_len_033);

#define MULTIAP_TLVFORGE067 "MULTIAP_TLVFORGE067 - Forge BackhaulSteeringRequestTLV TLV (multi_ap_tlv_stream_034)"
	result += _checkTrue(MULTIAP_TLVFORGE067, (INT8U *)&multi_ap_tlv_structure_034, multi_ap_tlv_stream_034, multi_ap_tlv_stream_len_034);

#define MULTIAP_TLVFORGE068 "MULTIAP_TLVFORGE068 - Forge BackhaulSteeringRequestTLV TLV (multi_ap_tlv_stream_034b)"
	result += _checkFalse(MULTIAP_TLVFORGE068, (INT8U *)&multi_ap_tlv_structure_034, multi_ap_tlv_stream_034b, multi_ap_tlv_stream_len_034);

#define MULTIAP_TLVFORGE069 "MULTIAP_TLVFORGE069 - Forge BackhaulSteeringResponseTLV TLV (multi_ap_tlv_stream_035)"
	result += _checkTrue(MULTIAP_TLVFORGE069, (INT8U *)&multi_ap_tlv_structure_035, multi_ap_tlv_stream_035, multi_ap_tlv_stream_len_035);

#define MULTIAP_TLVFORGE070 "MULTIAP_TLVFORGE070 - Forge BackhaulSteeringResponseTLV TLV (multi_ap_tlv_stream_035b)"
	result += _checkFalse(MULTIAP_TLVFORGE070, (INT8U *)&multi_ap_tlv_structure_035, multi_ap_tlv_stream_035b, multi_ap_tlv_stream_len_035);

#define MULTIAP_TLVFORGE071 "MULTIAP_TLVFORGE071 - Forge HigherLayerDataTLV TLV (multi_ap_tlv_stream_036)"
	result += _checkTrue(MULTIAP_TLVFORGE071, (INT8U *)&multi_ap_tlv_structure_036, multi_ap_tlv_stream_036, multi_ap_tlv_stream_len_036);

#define MULTIAP_TLVFORGE072 "MULTIAP_TLVFORGE072 - Forge HigherLayerDataTLV TLV (multi_ap_tlv_stream_036b)"
	result += _checkFalse(MULTIAP_TLVFORGE072, (INT8U *)&multi_ap_tlv_structure_036, multi_ap_tlv_stream_036b, multi_ap_tlv_stream_len_036);

#define MULTIAP_TLVFORGE073 "MULTIAP_TLVFORGE073 - Forge AssociatedSTATrafficStatsTLV TLV (multi_ap_tlv_stream_037)"
	result += _checkTrue(MULTIAP_TLVFORGE073, (INT8U *)&multi_ap_tlv_structure_037, multi_ap_tlv_stream_037, multi_ap_tlv_stream_len_037);

#define MULTIAP_TLVFORGE074 "MULTIAP_TLVFORGE074 - Forge AssociatedSTATrafficStatsTLV TLV (multi_ap_tlv_stream_037b)"
	result += _checkFalse(MULTIAP_TLVFORGE074, (INT8U *)&multi_ap_tlv_structure_037, multi_ap_tlv_stream_037b, multi_ap_tlv_stream_len_037);

#define MULTIAP_TLVFORGE075 "MULTIAP_TLVFORGE075 - Forge Profile2APCapabilityTLV TLV (multi_ap_tlv_stream_038)"
	result += _checkTrue(MULTIAP_TLVFORGE075, (INT8U *)&multi_ap_tlv_structure_038, multi_ap_tlv_stream_038, multi_ap_tlv_stream_len_038);

#define MULTIAP_TLVFORGE076 "MULTIAP_TLVFORGE076 - Forge Profile2APCapabilityTLV TLV (multi_ap_tlv_stream_038b)"
	result += _checkFalse(MULTIAP_TLVFORGE076, (INT8U *)&multi_ap_tlv_structure_038, multi_ap_tlv_stream_038b, multi_ap_tlv_stream_len_038);

#define MULTIAP_TLVFORGE077 "MULTIAP_TLVFORGE077 - Forge ServicePrioritizationRuleTLV TLV (multi_ap_tlv_stream_039)"
	result += _checkTrue(MULTIAP_TLVFORGE077, (INT8U *)&multi_ap_tlv_structure_039, multi_ap_tlv_stream_039, multi_ap_tlv_stream_len_039);

#define MULTIAP_TLVFORGE078 "MULTIAP_TLVFORGE078 - Forge ServicePrioritizationRuleTLV TLV (multi_ap_tlv_stream_039b)"
	result += _checkFalse(MULTIAP_TLVFORGE078, (INT8U *)&multi_ap_tlv_structure_039, multi_ap_tlv_stream_039b, multi_ap_tlv_stream_len_039);

#define MULTIAP_TLVFORGE079 "MULTIAP_TLVFORGE079 - Forge DSCPMappingTableTLV TLV (multi_ap_tlv_stream_040)"
	result += _checkTrue(MULTIAP_TLVFORGE079, (INT8U *)&multi_ap_tlv_structure_040, multi_ap_tlv_stream_040, multi_ap_tlv_stream_len_040);

#define MULTIAP_TLVFORGE080 "MULTIAP_TLVFORGE080 - Forge DSCPMappingTableTLV TLV (multi_ap_tlv_stream_040b)"
	result += _checkFalse(MULTIAP_TLVFORGE080, (INT8U *)&multi_ap_tlv_structure_040, multi_ap_tlv_stream_040b, multi_ap_tlv_stream_len_040);

#define MULTIAP_TLVFORGE083 "MULTIAP_TLVFORGE083 - Forge Profile2ErrorCodeTLV TLV (multi_ap_tlv_stream_042)"
	result += _checkTrue(MULTIAP_TLVFORGE083, (INT8U *)&multi_ap_tlv_structure_042, multi_ap_tlv_stream_042, multi_ap_tlv_stream_len_042);

#define MULTIAP_TLVFORGE084 "MULTIAP_TLVFORGE084 - Forge Profile2ErrorCodeTLV TLV (multi_ap_tlv_stream_042b)"
	result += _checkFalse(MULTIAP_TLVFORGE084, (INT8U *)&multi_ap_tlv_structure_042, multi_ap_tlv_stream_042b, multi_ap_tlv_stream_len_042);

#define MULTIAP_TLVFORGE085 "MULTIAP_TLVFORGE085 - Forge APRadioAdvancedCapabilitiesTLV TLV (multi_ap_tlv_stream_043)"
	result += _checkTrue(MULTIAP_TLVFORGE085, (INT8U *)&multi_ap_tlv_structure_043, multi_ap_tlv_stream_043, multi_ap_tlv_stream_len_043);

#define MULTIAP_TLVFORGE086 "MULTIAP_TLVFORGE086 - Forge APRadioAdvancedCapabilitiesTLV TLV (multi_ap_tlv_stream_043b)"
	result += _checkFalse(MULTIAP_TLVFORGE086, (INT8U *)&multi_ap_tlv_structure_043, multi_ap_tlv_stream_043b, multi_ap_tlv_stream_len_043);

#define MULTIAP_TLVFORGE087 "MULTIAP_TLVFORGE087 - Forge UnsuccessfulAssociationPolicyTLV TLV (multi_ap_tlv_stream_044)"
	result += _checkTrue(MULTIAP_TLVFORGE087, (INT8U *)&multi_ap_tlv_structure_044, multi_ap_tlv_stream_044, multi_ap_tlv_stream_len_044);

#define MULTIAP_TLVFORGE088 "MULTIAP_TLVFORGE088 - Forge UnsuccessfulAssociationPolicyTLV TLV (multi_ap_tlv_stream_044b)"
	result += _checkFalse(MULTIAP_TLVFORGE088, (INT8U *)&multi_ap_tlv_structure_044, multi_ap_tlv_stream_044b, multi_ap_tlv_stream_len_044);

#define MULTIAP_TLVFORGE089 "MULTIAP_TLVFORGE089 - Forge MetricCollectionIntervalTLV TLV (multi_ap_tlv_stream_045)"
	result += _checkTrue(MULTIAP_TLVFORGE089, (INT8U *)&multi_ap_tlv_structure_045, multi_ap_tlv_stream_045, multi_ap_tlv_stream_len_045);

#define MULTIAP_TLVFORGE090 "MULTIAP_TLVFORGE090 - Forge MetricCollectionIntervalTLV TLV (multi_ap_tlv_stream_045b)"
	result += _checkFalse(MULTIAP_TLVFORGE090, (INT8U *)&multi_ap_tlv_structure_045, multi_ap_tlv_stream_045b, multi_ap_tlv_stream_len_045);

#define MULTIAP_TLVFORGE091 "MULTIAP_TLVFORGE091 - Forge RadioMetricsTLV TLV (multi_ap_tlv_stream_046)"
	result += _checkTrue(MULTIAP_TLVFORGE091, (INT8U *)&multi_ap_tlv_structure_046, multi_ap_tlv_stream_046, multi_ap_tlv_stream_len_046);

#define MULTIAP_TLVFORGE092 "MULTIAP_TLVFORGE092 - Forge RadioMetricsTLV TLV (multi_ap_tlv_stream_046b)"
	result += _checkFalse(MULTIAP_TLVFORGE092, (INT8U *)&multi_ap_tlv_structure_046, multi_ap_tlv_stream_046b, multi_ap_tlv_stream_len_046);

#define MULTIAP_TLVFORGE093 "MULTIAP_TLVFORGE093 - Forge APExtendedMetricsTLV TLV (multi_ap_tlv_stream_047)"
	result += _checkTrue(MULTIAP_TLVFORGE093, (INT8U *)&multi_ap_tlv_structure_047, multi_ap_tlv_stream_047, multi_ap_tlv_stream_len_047);

#define MULTIAP_TLVFORGE094 "MULTIAP_TLVFORGE094 - Forge APExtendedMetricsTLV TLV (multi_ap_tlv_stream_047b)"
	result += _checkFalse(MULTIAP_TLVFORGE094, (INT8U *)&multi_ap_tlv_structure_047, multi_ap_tlv_stream_047b, multi_ap_tlv_stream_len_047);

#define MULTIAP_TLVFORGE095 "MULTIAP_TLVFORGE095 - Forge AssociatedSTAExtendedLinkMericsTLV TLV (multi_ap_tlv_stream_048)"
	result += _checkTrue(MULTIAP_TLVFORGE095, (INT8U *)&multi_ap_tlv_structure_048, multi_ap_tlv_stream_048, multi_ap_tlv_stream_len_048);

#define MULTIAP_TLVFORGE096 "MULTIAP_TLVFORGE096 - Forge AssociatedSTAExtendedLinkMericsTLV TLV (multi_ap_tlv_stream_048b)"
	result += _checkFalse(MULTIAP_TLVFORGE096, (INT8U *)&multi_ap_tlv_structure_048, multi_ap_tlv_stream_048b, multi_ap_tlv_stream_len_048);

#define MULTIAP_TLVFORGE097 "MULTIAP_TLVFORGE097 - Forge StatusCodeTLV TLV (multi_ap_tlv_stream_049)"
	result += _checkTrue(MULTIAP_TLVFORGE097, (INT8U *)&multi_ap_tlv_structure_049, multi_ap_tlv_stream_049, multi_ap_tlv_stream_len_049);

#define MULTIAP_TLVFORGE098 "MULTIAP_TLVFORGE098 - Forge StatusCodeTLV TLV (multi_ap_tlv_stream_049b)"
	result += _checkFalse(MULTIAP_TLVFORGE098, (INT8U *)&multi_ap_tlv_structure_049, multi_ap_tlv_stream_049b, multi_ap_tlv_stream_len_049);

#define MULTIAP_TLVFORGE099 "MULTIAP_TLVFORGE099 - Forge ReasonCodeTLV TLV (multi_ap_tlv_stream_050)"
	result += _checkTrue(MULTIAP_TLVFORGE099, (INT8U *)&multi_ap_tlv_structure_050, multi_ap_tlv_stream_050, multi_ap_tlv_stream_len_050);

#define MULTIAP_TLVFORGE0100 "MULTIAP_TLVFORGE0100 - Forge ReasonCodeTLV TLV (multi_ap_tlv_stream_050b)"
	result += _checkFalse(MULTIAP_TLVFORGE0100, (INT8U *)&multi_ap_tlv_structure_050, multi_ap_tlv_stream_050b, multi_ap_tlv_stream_len_050);

#define MULTIAP_TLVFORGE0101 "MULTIAP_TLVFORGE0101 - Forge BackhaulSTARadioCapabilitiesTLV TLV (multi_ap_tlv_stream_051)"
	result += _checkTrue(MULTIAP_TLVFORGE0101, (INT8U *)&multi_ap_tlv_structure_051, multi_ap_tlv_stream_051, multi_ap_tlv_stream_len_051);

#define MULTIAP_TLVFORGE0102 "MULTIAP_TLVFORGE0102 - Forge BackhaulSTARadioCapabilitiesTLV TLV (multi_ap_tlv_stream_051b)"
	result += _checkFalse(MULTIAP_TLVFORGE0102, (INT8U *)&multi_ap_tlv_structure_051, multi_ap_tlv_stream_051b, multi_ap_tlv_stream_len_051);

#define MULTIAP_TLVFORGE0103 "MULTIAP_TLVFORGE0103 - Forge AKMSuiteCapabilitiesTLV TLV (multi_ap_tlv_stream_052)"
	result += _checkTrue(MULTIAP_TLVFORGE0103, (INT8U *)&multi_ap_tlv_structure_052, multi_ap_tlv_stream_052, multi_ap_tlv_stream_len_052);

#define MULTIAP_TLVFORGE0104 "MULTIAP_TLVFORGE0104 - Forge AKMSuiteCapabilitiesTLV TLV (multi_ap_tlv_stream_052b)"
	result += _checkFalse(MULTIAP_TLVFORGE0104, (INT8U *)&multi_ap_tlv_structure_052, multi_ap_tlv_stream_052b, multi_ap_tlv_stream_len_052);

#define MULTIAP_TLVFORGE0105 "MULTIAP_TLVFORGE0105 - Forge Encap1905DPPTLV TLV (multi_ap_tlv_stream_053)"
	result += _checkTrue(MULTIAP_TLVFORGE0105, (INT8U *)&multi_ap_tlv_structure_053, multi_ap_tlv_stream_053, multi_ap_tlv_stream_len_053);

#define MULTIAP_TLVFORGE0106 "MULTIAP_TLVFORGE0106 - Forge Encap1905DPPTLV TLV (multi_ap_tlv_stream_053b)"
	result += _checkFalse(MULTIAP_TLVFORGE0106, (INT8U *)&multi_ap_tlv_structure_053, multi_ap_tlv_stream_053b, multi_ap_tlv_stream_len_053);

#define MULTIAP_TLVFORGE0107 "MULTIAP_TLVFORGE0107 - Forge Encap1905EAPOLTLV TLV (multi_ap_tlv_stream_054)"
	result += _checkTrue(MULTIAP_TLVFORGE0107, (INT8U *)&multi_ap_tlv_structure_054, multi_ap_tlv_stream_054, multi_ap_tlv_stream_len_054);

#define MULTIAP_TLVFORGE0108 "MULTIAP_TLVFORGE0108 - Forge Encap1905EAPOLTLV TLV (multi_ap_tlv_stream_054b)"
	result += _checkFalse(MULTIAP_TLVFORGE0108, (INT8U *)&multi_ap_tlv_structure_054, multi_ap_tlv_stream_054b, multi_ap_tlv_stream_len_054);

#define MULTIAP_TLVFORGE0109 "MULTIAP_TLVFORGE0109 - Forge DPPBootstrappingURINotificationTLV TLV (multi_ap_tlv_stream_055)"
	result += _checkTrue(MULTIAP_TLVFORGE0109, (INT8U *)&multi_ap_tlv_structure_055, multi_ap_tlv_stream_055, multi_ap_tlv_stream_len_055);

#define MULTIAP_TLVFORGE0110 "MULTIAP_TLVFORGE0110 - Forge DPPBootstrappingURINotificationTLV TLV (multi_ap_tlv_stream_055b)"
	result += _checkFalse(MULTIAP_TLVFORGE0110, (INT8U *)&multi_ap_tlv_structure_055, multi_ap_tlv_stream_055b, multi_ap_tlv_stream_len_055);

#define MULTIAP_TLVFORGE0111 "MULTIAP_TLVFORGE0111 - Forge BackhaulBSSConfigurationTLV TLV (multi_ap_tlv_stream_056)"
	result += _checkTrue(MULTIAP_TLVFORGE0111, (INT8U *)&multi_ap_tlv_structure_056, multi_ap_tlv_stream_056, multi_ap_tlv_stream_len_056);

#define MULTIAP_TLVFORGE0112 "MULTIAP_TLVFORGE0112 - Forge BackhaulBSSConfigurationTLV TLV (multi_ap_tlv_stream_056b)"
	result += _checkFalse(MULTIAP_TLVFORGE0112, (INT8U *)&multi_ap_tlv_structure_056, multi_ap_tlv_stream_056b, multi_ap_tlv_stream_len_056);

#define MULTIAP_TLVFORGE0113 "MULTIAP_TLVFORGE0113 - Forge DPPChirpValueTLV TLV (multi_ap_tlv_stream_057)"
	result += _checkTrue(MULTIAP_TLVFORGE0113, (INT8U *)&multi_ap_tlv_structure_057, multi_ap_tlv_stream_057, multi_ap_tlv_stream_len_057);

#define MULTIAP_TLVFORGE0114 "MULTIAP_TLVFORGE0114 - Forge DPPChirpValueTLV TLV (multi_ap_tlv_stream_057b)"
	result += _checkFalse(MULTIAP_TLVFORGE0114, (INT8U *)&multi_ap_tlv_structure_057, multi_ap_tlv_stream_057b, multi_ap_tlv_stream_len_057);

#define MULTIAP_TLVFORGE0115 "MULTIAP_TLVFORGE0115 - Forge BSSConfigRequestTLV TLV (multi_ap_tlv_stream_058)"
	result += _checkTrue(MULTIAP_TLVFORGE0115, (INT8U *)&multi_ap_tlv_structure_058, multi_ap_tlv_stream_058, multi_ap_tlv_stream_len_058);

#define MULTIAP_TLVFORGE0116 "MULTIAP_TLVFORGE0116 - Forge BSSConfigRequestTLV TLV (multi_ap_tlv_stream_058b)"
	result += _checkFalse(MULTIAP_TLVFORGE0116, (INT8U *)&multi_ap_tlv_structure_058, multi_ap_tlv_stream_058b, multi_ap_tlv_stream_len_058);

#define MULTIAP_TLVFORGE0117 "MULTIAP_TLVFORGE0117 - Forge BSSConfigResponseTLV TLV (multi_ap_tlv_stream_059)"
	result += _checkTrue(MULTIAP_TLVFORGE0117, (INT8U *)&multi_ap_tlv_structure_059, multi_ap_tlv_stream_059, multi_ap_tlv_stream_len_059);

#define MULTIAP_TLVFORGE0118 "MULTIAP_TLVFORGE0118 - Forge BSSConfigResponseTLV TLV (multi_ap_tlv_stream_059b)"
	result += _checkFalse(MULTIAP_TLVFORGE0118, (INT8U *)&multi_ap_tlv_structure_059, multi_ap_tlv_stream_059b, multi_ap_tlv_stream_len_059);

#define MULTIAP_TLVFORGE0119 "MULTIAP_TLVFORGE0119 - Forge BSSConfigReportTLV TLV (multi_ap_tlv_stream_060)"
	result += _checkTrue(MULTIAP_TLVFORGE0119, (INT8U *)&multi_ap_tlv_structure_060, multi_ap_tlv_stream_060, multi_ap_tlv_stream_len_060);

#define MULTIAP_TLVFORGE0120 "MULTIAP_TLVFORGE0120 - Forge BSSConfigReportTLV TLV (multi_ap_tlv_stream_060b)"
	result += _checkFalse(MULTIAP_TLVFORGE0120, (INT8U *)&multi_ap_tlv_structure_060, multi_ap_tlv_stream_060b, multi_ap_tlv_stream_len_060);

#define MULTIAP_TLVFORGE0121 "MULTIAP_TLVFORGE0121 - Forge AnticipatedChannelPerferenceTLV TLV (multi_ap_tlv_stream_061)"
	result += _checkTrue(MULTIAP_TLVFORGE0121, (INT8U *)&multi_ap_tlv_structure_061, multi_ap_tlv_stream_061, multi_ap_tlv_stream_len_061);

#define MULTIAP_TLVFORGE0122 "MULTIAP_TLVFORGE0122 - Forge AnticipatedChannelPerferenceTLV TLV (multi_ap_tlv_stream_061b)"
	result += _checkFalse(MULTIAP_TLVFORGE0122, (INT8U *)&multi_ap_tlv_structure_061, multi_ap_tlv_stream_061b, multi_ap_tlv_stream_len_061);

#define MULTIAP_TLVFORGE0123 "MULTIAP_TLVFORGE0123 - Forge AnticipatedChannelUsageTLV TLV (multi_ap_tlv_stream_062)"
	result += _checkTrue(MULTIAP_TLVFORGE0123, (INT8U *)&multi_ap_tlv_structure_062, multi_ap_tlv_stream_062, multi_ap_tlv_stream_len_062);

#define MULTIAP_TLVFORGE0124 "MULTIAP_TLVFORGE0124 - Forge AnticipatedChannelUsageTLV TLV (multi_ap_tlv_stream_062b)"
	result += _checkFalse(MULTIAP_TLVFORGE0124, (INT8U *)&multi_ap_tlv_structure_062, multi_ap_tlv_stream_062b, multi_ap_tlv_stream_len_062);

#define MULTIAP_TLVFORGE0125 "MULTIAP_TLVFORGE0125 - Forge SpatialReuseRequestTLV TLV (multi_ap_tlv_stream_063)"
	result += _checkTrue(MULTIAP_TLVFORGE0125, (INT8U *)&multi_ap_tlv_structure_063, multi_ap_tlv_stream_063, multi_ap_tlv_stream_len_063);

#define MULTIAP_TLVFORGE0126 "MULTIAP_TLVFORGE0126 - Forge SpatialReuseRequestTLV TLV (multi_ap_tlv_stream_063b)"
	result += _checkFalse(MULTIAP_TLVFORGE0126, (INT8U *)&multi_ap_tlv_structure_063, multi_ap_tlv_stream_063b, multi_ap_tlv_stream_len_063);

#define MULTIAP_TLVFORGE0127 "MULTIAP_TLVFORGE0127 - Forge SpatialReuseReportTLV TLV (multi_ap_tlv_stream_064)"
	result += _checkTrue(MULTIAP_TLVFORGE0127, (INT8U *)&multi_ap_tlv_structure_064, multi_ap_tlv_stream_064, multi_ap_tlv_stream_len_064);

#define MULTIAP_TLVFORGE0128 "MULTIAP_TLVFORGE0128 - Forge SpatialReuseReportTLV TLV (multi_ap_tlv_stream_064b)"
	result += _checkFalse(MULTIAP_TLVFORGE0128, (INT8U *)&multi_ap_tlv_structure_064, multi_ap_tlv_stream_064b, multi_ap_tlv_stream_len_064);

#define MULTIAP_TLVFORGE0129 "MULTIAP_TLVFORGE0129 - Forge SpatialReuseConfigResponseTLV TLV (multi_ap_tlv_stream_065)"
	result += _checkTrue(MULTIAP_TLVFORGE0129, (INT8U *)&multi_ap_tlv_structure_065, multi_ap_tlv_stream_065, multi_ap_tlv_stream_len_065);

#define MULTIAP_TLVFORGE0130 "MULTIAP_TLVFORGE0130 - Forge SpatialReuseConfigResponseTLV TLV (multi_ap_tlv_stream_065b)"
	result += _checkFalse(MULTIAP_TLVFORGE0130, (INT8U *)&multi_ap_tlv_structure_065, multi_ap_tlv_stream_065b, multi_ap_tlv_stream_len_065);

#define MULTIAP_TLVFORGE0131 "MULTIAP_TLVFORGE0131 - Forge QoSManagementPolicyTLV TLV (multi_ap_tlv_stream_066)"
	result += _checkTrue(MULTIAP_TLVFORGE0131, (INT8U *)&multi_ap_tlv_structure_066, multi_ap_tlv_stream_066, multi_ap_tlv_stream_len_066);

#define MULTIAP_TLVFORGE0132 "MULTIAP_TLVFORGE0132 - Forge QoSManagementPolicyTLV TLV (multi_ap_tlv_stream_066b)"
	result += _checkFalse(MULTIAP_TLVFORGE0132, (INT8U *)&multi_ap_tlv_structure_066, multi_ap_tlv_stream_066b, multi_ap_tlv_stream_len_066);

#define MULTIAP_TLVFORGE0133 "MULTIAP_TLVFORGE0133 - Forge QoSManagementDescriptorTLV TLV (multi_ap_tlv_stream_067)"
	result += _checkTrue(MULTIAP_TLVFORGE0133, (INT8U *)&multi_ap_tlv_structure_067, multi_ap_tlv_stream_067, multi_ap_tlv_stream_len_067);

#define MULTIAP_TLVFORGE0134 "MULTIAP_TLVFORGE0134 - Forge QoSManagementDescriptorTLV TLV (multi_ap_tlv_stream_067b)"
	result += _checkFalse(MULTIAP_TLVFORGE0134, (INT8U *)&multi_ap_tlv_structure_067, multi_ap_tlv_stream_067b, multi_ap_tlv_stream_len_067);

#define MULTIAP_TLVFORGE0135 "MULTIAP_TLVFORGE0135 - Forge ControllerCapabilityTLV TLV (multi_ap_tlv_stream_068)"
	result += _checkTrue(MULTIAP_TLVFORGE0135, (INT8U *)&multi_ap_tlv_structure_068, multi_ap_tlv_stream_068, multi_ap_tlv_stream_len_068);

#define MULTIAP_TLVFORGE0136 "MULTIAP_TLVFORGE0136 - Forge ControllerCapabilityTLV TLV (multi_ap_tlv_stream_068b)"
	result += _checkFalse(MULTIAP_TLVFORGE0136, (INT8U *)&multi_ap_tlv_structure_068, multi_ap_tlv_stream_068b, multi_ap_tlv_stream_len_068);

	// Return the number of test cases that failed
	//
	return result;
}
