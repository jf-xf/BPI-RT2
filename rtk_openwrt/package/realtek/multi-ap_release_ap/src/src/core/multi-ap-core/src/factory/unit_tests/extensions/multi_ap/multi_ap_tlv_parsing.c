/*
 * Copyright (C) 2018 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

//
// This file tests the "parse_non_1905_TLV_from_packet()" function by providing
// some test input streams and checking the generated output structure.
//

#include "platform.h"
#include "utils.h"

#include "multi_ap_tlv_test_vectors.h"
#include "multi_ap_tlvs.h"

INT8U _check(const char *test_description, INT8U mode, INT8U *input, INT8U *expected_output)
{
	INT8U  result;
	INT8U *real_output;
	INT8U  comparison;

	// Parse the packet
	real_output = parse_multi_ap_TLV_from_packet(input);

	// Compare TLVs
	comparison = compare_multi_ap_TLV_structures(real_output, expected_output);

	if (mode == CHECK_TRUE) {
		if (0 == comparison) {
			result = 0;
			PLATFORM_PRINTF("%-100s: OK\n", test_description);
			// visit_multi_ap_TLV_structure(real_output, print_callback, PLATFORM_PRINTF, "");
		} else {
			result = 1;
			PLATFORM_PRINTF("%-100s: KO !!!\n", test_description);

			// CheckTrue error needs more debug info
			PLATFORM_PRINTF("  Expected output:\n");
			visit_multi_ap_TLV_structure(expected_output, print_callback, PLATFORM_PRINTF, "");
			PLATFORM_PRINTF("  Real output    :\n");
			visit_multi_ap_TLV_structure(real_output, print_callback, PLATFORM_PRINTF, "");
		}
	} else {
		if (0 == comparison) {
			result = 1;
			PLATFORM_PRINTF("%-100s: KO !!!\n", test_description);
		} else {
			result = 0;
			PLATFORM_PRINTF("%-100s: OK\n", test_description);
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

#define MULTIAP_TLVPARSE001 "MULTIAP_TLVPARSE001 - Parse SupportedServiceTLV TLV (multi_ap_tlv_stream_001)"
	result += _checkTrue(MULTIAP_TLVPARSE001, multi_ap_tlv_stream_001, (INT8U *)&multi_ap_tlv_structure_001);

#define MULTIAP_TLVPARSE002 "MULTIAP_TLVPARSE002 - Parse SupportedServiceTLV TLV (multi_ap_tlv_stream_001b)"
	result += _checkFalse(MULTIAP_TLVPARSE002, multi_ap_tlv_stream_001b, (INT8U *)&multi_ap_tlv_structure_001);

#define MULTIAP_TLVPARSE003 "MULTIAP_TLVPARSE003 - Parse SearchedServiceTLV TLV (multi_ap_tlv_stream_002)"
	result += _checkTrue(MULTIAP_TLVPARSE003, multi_ap_tlv_stream_002, (INT8U *)&multi_ap_tlv_structure_002);

#define MULTIAP_TLVPARSE004 "MULTIAP_TLVPARSE004 - Parse SearchedServiceTLV TLV (multi_ap_tlv_stream_002b)"
	result += _checkFalse(MULTIAP_TLVPARSE004, multi_ap_tlv_stream_002b, (INT8U *)&multi_ap_tlv_structure_002);

#define MULTIAP_TLVPARSE005 "MULTIAP_TLVPARSE005 - Parse ApRadioIdentifierTLV TLV (multi_ap_tlv_stream_003)"
	result += _checkTrue(MULTIAP_TLVPARSE005, multi_ap_tlv_stream_003, (INT8U *)&multi_ap_tlv_structure_003);

#define MULTIAP_TLVPARSE006 "MULTIAP_TLVPARSE006 - Parse ApRadioIdentifierTLV TLV (multi_ap_tlv_stream_003b)"
	result += _checkFalse(MULTIAP_TLVPARSE006, multi_ap_tlv_stream_003b, (INT8U *)&multi_ap_tlv_structure_003);

#define MULTIAP_TLVPARSE007 "MULTIAP_TLVPARSE007 - Parse ApOperationalBSSTLV TLV (multi_ap_tlv_stream_004)"
	result += _checkTrue(MULTIAP_TLVPARSE007, multi_ap_tlv_stream_004, (INT8U *)&multi_ap_tlv_structure_004);

#define MULTIAP_TLVPARSE008 "MULTIAP_TLVPARSE008 - Parse ApOperationalBSSTLV TLV (multi_ap_tlv_stream_004b)"
	result += _checkFalse(MULTIAP_TLVPARSE008, multi_ap_tlv_stream_004b, (INT8U *)&multi_ap_tlv_structure_004);

#define MULTIAP_TLVPARSE009 "MULTIAP_TLVPARSE009 - Parse AssociatedClientsTLV TLV (multi_ap_tlv_stream_005)"
	result += _checkTrue(MULTIAP_TLVPARSE009, multi_ap_tlv_stream_005, (INT8U *)&multi_ap_tlv_structure_005);

#define MULTIAP_TLVPARSE010 "MULTIAP_TLVPARSE010 - Parse AssociatedClientsTLV TLV (multi_ap_tlv_stream_005b)"
	result += _checkFalse(MULTIAP_TLVPARSE010, multi_ap_tlv_stream_005b, (INT8U *)&multi_ap_tlv_structure_005);

#define MULTIAP_TLVPARSE011 "MULTIAP_TLVPARSE011 - Parse APCapabilityTLV TLV (multi_ap_tlv_stream_006)"
	result += _checkTrue(MULTIAP_TLVPARSE011, multi_ap_tlv_stream_006, (INT8U *)&multi_ap_tlv_structure_006);

#define MULTIAP_TLVPARSE012 "MULTIAP_TLVPARSE012 - Parse APCapabilityTLV TLV (multi_ap_tlv_stream_006b)"
	result += _checkFalse(MULTIAP_TLVPARSE012, multi_ap_tlv_stream_006b, (INT8U *)&multi_ap_tlv_structure_006);

#define MULTIAP_TLVPARSE013 "MULTIAP_TLVPARSE013 - Parse APRadioBasicCapabilitiesTLV TLV (multi_ap_tlv_stream_007)"
	result += _checkTrue(MULTIAP_TLVPARSE013, multi_ap_tlv_stream_007, (INT8U *)&multi_ap_tlv_structure_007);

#define MULTIAP_TLVPARSE014 "MULTIAP_TLVPARSE014 - Parse APRadioBasicCapabilitiesTLV TLV (multi_ap_tlv_stream_007b)"
	result += _checkFalse(MULTIAP_TLVPARSE014, multi_ap_tlv_stream_007b, (INT8U *)&multi_ap_tlv_structure_007);

#define MULTIAP_TLVPARSE015 "MULTIAP_TLVPARSE015 - Parse APHTCapabilitiesTLV TLV (multi_ap_tlv_stream_008)"
	result += _checkTrue(MULTIAP_TLVPARSE015, multi_ap_tlv_stream_008, (INT8U *)&multi_ap_tlv_structure_008);

#define MULTIAP_TLVPARSE016 "MULTIAP_TLVPARSE016 - Parse APHTCapabilitiesTLV TLV (multi_ap_tlv_stream_008b)"
	result += _checkFalse(MULTIAP_TLVPARSE016, multi_ap_tlv_stream_008b, (INT8U *)&multi_ap_tlv_structure_008);

#define MULTIAP_TLVPARSE017 "MULTIAP_TLVPARSE017 - Parse APVHTCapabilitiesTLV TLV (multi_ap_tlv_stream_009)"
	result += _checkTrue(MULTIAP_TLVPARSE017, multi_ap_tlv_stream_009, (INT8U *)&multi_ap_tlv_structure_009);

#define MULTIAP_TLVPARSE018 "MULTIAP_TLVPARSE018 - Parse APVHTCapabilitiesTLV TLV (multi_ap_tlv_stream_009b)"
	result += _checkFalse(MULTIAP_TLVPARSE018, multi_ap_tlv_stream_009b, (INT8U *)&multi_ap_tlv_structure_009);

#define MULTIAP_TLVPARSE019 "MULTIAP_TLVPARSE019 - Parse APHECapabilitiesTLV TLV (multi_ap_tlv_stream_010)"
	result += _checkTrue(MULTIAP_TLVPARSE019, multi_ap_tlv_stream_010, (INT8U *)&multi_ap_tlv_structure_010);

#define MULTIAP_TLVPARSE020 "MULTIAP_TLVPARSE020 - Parse APHECapabilitiesTLV TLV (multi_ap_tlv_stream_010b)"
	result += _checkFalse(MULTIAP_TLVPARSE020, multi_ap_tlv_stream_010b, (INT8U *)&multi_ap_tlv_structure_010);

#define MULTIAP_TLVPARSE021 "MULTIAP_TLVPARSE021 - Parse ClientInfoTLV TLV (multi_ap_tlv_stream_011)"
	result += _checkTrue(MULTIAP_TLVPARSE021, multi_ap_tlv_stream_011, (INT8U *)&multi_ap_tlv_structure_011);

#define MULTIAP_TLVPARSE022 "MULTIAP_TLVPARSE022 - Parse ClientInfoTLV TLV (multi_ap_tlv_stream_011b)"
	result += _checkFalse(MULTIAP_TLVPARSE022, multi_ap_tlv_stream_011b, (INT8U *)&multi_ap_tlv_structure_011);

#define MULTIAP_TLVPARSE023 "MULTIAP_TLVPARSE023 - Parse ClientCapbilityReportTLV TLV (multi_ap_tlv_stream_012)"
	result += _checkTrue(MULTIAP_TLVPARSE023, multi_ap_tlv_stream_012, (INT8U *)&multi_ap_tlv_structure_012);

#define MULTIAP_TLVPARSE024 "MULTIAP_TLVPARSE024 - Parse ClientCapbilityReportTLV TLV (multi_ap_tlv_stream_012b)"
	result += _checkFalse(MULTIAP_TLVPARSE024, multi_ap_tlv_stream_012b, (INT8U *)&multi_ap_tlv_structure_012);

#define MULTIAP_TLVPARSE025 "MULTIAP_TLVPARSE025 - Parse ClientAssociationEventTLV TLV (multi_ap_tlv_stream_013)"
	result += _checkTrue(MULTIAP_TLVPARSE025, multi_ap_tlv_stream_013, (INT8U *)&multi_ap_tlv_structure_013);

#define MULTIAP_TLVPARSE026 "MULTIAP_TLVPARSE026 - Parse ClientAssociationEventTLV TLV (multi_ap_tlv_stream_013b)"
	result += _checkFalse(MULTIAP_TLVPARSE026, multi_ap_tlv_stream_013b, (INT8U *)&multi_ap_tlv_structure_013);

#define MULTIAP_TLVPARSE027 "MULTIAP_TLVPARSE027 - Parse STAMacAddressTypeTLV TLV (multi_ap_tlv_stream_014)"
	result += _checkTrue(MULTIAP_TLVPARSE027, multi_ap_tlv_stream_014, (INT8U *)&multi_ap_tlv_structure_014);

#define MULTIAP_TLVPARSE028 "MULTIAP_TLVPARSE028 - Parse STAMacAddressTypeTLV TLV (multi_ap_tlv_stream_014b)"
	result += _checkFalse(MULTIAP_TLVPARSE028, multi_ap_tlv_stream_014b, (INT8U *)&multi_ap_tlv_structure_014);

#define MULTIAP_TLVPARSE029 "MULTIAP_TLVPARSE029 - Parse ClientAssociationControlRequestTLV TLV (multi_ap_tlv_stream_015)"
	result += _checkTrue(MULTIAP_TLVPARSE029, multi_ap_tlv_stream_015, (INT8U *)&multi_ap_tlv_structure_015);

#define MULTIAP_TLVPARSE030 "MULTIAP_TLVPARSE030 - Parse ClientAssociationControlRequestTLV TLV (multi_ap_tlv_stream_015b)"
	result += _checkFalse(MULTIAP_TLVPARSE030, multi_ap_tlv_stream_015b, (INT8U *)&multi_ap_tlv_structure_015);

#define MULTIAP_TLVPARSE031 "MULTIAP_TLVPARSE031 - Parse ErrorCodeTLV TLV (multi_ap_tlv_stream_016)"
	result += _checkTrue(MULTIAP_TLVPARSE031, multi_ap_tlv_stream_016, (INT8U *)&multi_ap_tlv_structure_016);

#define MULTIAP_TLVPARSE032 "MULTIAP_TLVPARSE032 - Parse ErrorCodeTLV TLV (multi_ap_tlv_stream_016b)"
	result += _checkFalse(MULTIAP_TLVPARSE032, multi_ap_tlv_stream_016b, (INT8U *)&multi_ap_tlv_structure_016);

#define MULTIAP_TLVPARSE033 "MULTIAP_TLVPARSE033 - Parse SteeringPolicyTLV TLV (multi_ap_tlv_stream_017)"
	result += _checkTrue(MULTIAP_TLVPARSE033, multi_ap_tlv_stream_017, (INT8U *)&multi_ap_tlv_structure_017);

#define MULTIAP_TLVPARSE034 "MULTIAP_TLVPARSE034 - Parse SteeringPolicyTLV TLV (multi_ap_tlv_stream_017b)"
	result += _checkFalse(MULTIAP_TLVPARSE034, multi_ap_tlv_stream_017b, (INT8U *)&multi_ap_tlv_structure_017);

#define MULTIAP_TLVPARSE035 "MULTIAP_TLVPARSE035 - Parse MetricReportingPolicyTLV TLV (multi_ap_tlv_stream_018)"
	result += _checkTrue(MULTIAP_TLVPARSE035, multi_ap_tlv_stream_018, (INT8U *)&multi_ap_tlv_structure_018);

#define MULTIAP_TLVPARSE036 "MULTIAP_TLVPARSE036 - Parse MetricReportingPolicyTLV TLV (multi_ap_tlv_stream_018b)"
	result += _checkFalse(MULTIAP_TLVPARSE036, multi_ap_tlv_stream_018b, (INT8U *)&multi_ap_tlv_structure_018);

#define MULTIAP_TLVPARSE037 "MULTIAP_TLVPARSE037 - Parse ChannelPreferenceTLV TLV (multi_ap_tlv_stream_019)"
	result += _checkTrue(MULTIAP_TLVPARSE037, multi_ap_tlv_stream_019, (INT8U *)&multi_ap_tlv_structure_019);

#define MULTIAP_TLVPARSE038 "MULTIAP_TLVPARSE038 - Parse ChannelPreferenceTLV TLV (multi_ap_tlv_stream_019b)"
	result += _checkFalse(MULTIAP_TLVPARSE038, multi_ap_tlv_stream_019b, (INT8U *)&multi_ap_tlv_structure_019);

#define MULTIAP_TLVPARSE039 "MULTIAP_TLVPARSE039 - Parse RadioOperationRestrictionTLV TLV (multi_ap_tlv_stream_020)"
	result += _checkTrue(MULTIAP_TLVPARSE039, multi_ap_tlv_stream_020, (INT8U *)&multi_ap_tlv_structure_020);

#define MULTIAP_TLVPARSE040 "MULTIAP_TLVPARSE040 - Parse RadioOperationRestrictionTLV TLV (multi_ap_tlv_stream_020b)"
	result += _checkFalse(MULTIAP_TLVPARSE040, multi_ap_tlv_stream_020b, (INT8U *)&multi_ap_tlv_structure_020);

#define MULTIAP_TLVPARSE041 "MULTIAP_TLVPARSE041 - Parse TransmitPowerLimitTLV TLV (multi_ap_tlv_stream_021)"
	result += _checkTrue(MULTIAP_TLVPARSE041, multi_ap_tlv_stream_021, (INT8U *)&multi_ap_tlv_structure_021);

#define MULTIAP_TLVPARSE042 "MULTIAP_TLVPARSE042 - Parse TransmitPowerLimitTLV TLV (multi_ap_tlv_stream_021b)"
	result += _checkFalse(MULTIAP_TLVPARSE042, multi_ap_tlv_stream_021b, (INT8U *)&multi_ap_tlv_structure_021);

#define MULTIAP_TLVPARSE043 "MULTIAP_TLVPARSE043 - Parse ChannelSelectionResponseTLV TLV (multi_ap_tlv_stream_022)"
	result += _checkTrue(MULTIAP_TLVPARSE043, multi_ap_tlv_stream_022, (INT8U *)&multi_ap_tlv_structure_022);

#define MULTIAP_TLVPARSE044 "MULTIAP_TLVPARSE044 - Parse ChannelSelectionResponseTLV TLV (multi_ap_tlv_stream_022b)"
	result += _checkFalse(MULTIAP_TLVPARSE044, multi_ap_tlv_stream_022b, (INT8U *)&multi_ap_tlv_structure_022);

#define MULTIAP_TLVPARSE045 "MULTIAP_TLVPARSE045 - Parse OperatingChannelReportTLV TLV (multi_ap_tlv_stream_023)"
	result += _checkTrue(MULTIAP_TLVPARSE045, multi_ap_tlv_stream_023, (INT8U *)&multi_ap_tlv_structure_023);

#define MULTIAP_TLVPARSE046 "MULTIAP_TLVPARSE046 - Parse OperatingChannelReportTLV TLV (multi_ap_tlv_stream_023b)"
	result += _checkFalse(MULTIAP_TLVPARSE046, multi_ap_tlv_stream_023b, (INT8U *)&multi_ap_tlv_structure_023);

#define MULTIAP_TLVPARSE047 "MULTIAP_TLVPARSE047 - Parse APMetricQueryTLV TLV (multi_ap_tlv_stream_024)"
	result += _checkTrue(MULTIAP_TLVPARSE047, multi_ap_tlv_stream_024, (INT8U *)&multi_ap_tlv_structure_024);

#define MULTIAP_TLVPARSE048 "MULTIAP_TLVPARSE048 - Parse APMetricQueryTLV TLV (multi_ap_tlv_stream_024b)"
	result += _checkFalse(MULTIAP_TLVPARSE048, multi_ap_tlv_stream_024b, (INT8U *)&multi_ap_tlv_structure_024);

#define MULTIAP_TLVPARSE049 "MULTIAP_TLVPARSE049 - Parse APMetricsTLV TLV (multi_ap_tlv_stream_025)"
	result += _checkTrue(MULTIAP_TLVPARSE049, multi_ap_tlv_stream_025, (INT8U *)&multi_ap_tlv_structure_025);

#define MULTIAP_TLVPARSE050 "MULTIAP_TLVPARSE050 - Parse APMetricsTLV TLV (multi_ap_tlv_stream_025b)"
	result += _checkFalse(MULTIAP_TLVPARSE050, multi_ap_tlv_stream_025b, (INT8U *)&multi_ap_tlv_structure_025);

#define MULTIAP_TLVPARSE051 "MULTIAP_TLVPARSE051 - Parse AssociatedSTALinkMetricsTLV TLV (multi_ap_tlv_stream_026)"
	result += _checkTrue(MULTIAP_TLVPARSE051, multi_ap_tlv_stream_026, (INT8U *)&multi_ap_tlv_structure_026);

#define MULTIAP_TLVPARSE052 "MULTIAP_TLVPARSE052 - Parse AssociatedSTALinkMetricsTLV TLV (multi_ap_tlv_stream_026b)"
	result += _checkFalse(MULTIAP_TLVPARSE052, multi_ap_tlv_stream_026b, (INT8U *)&multi_ap_tlv_structure_026);

#define MULTIAP_TLVPARSE053 "MULTIAP_TLVPARSE053 - Parse UnassociatedSTALinkMetricsQueryTLV TLV (multi_ap_tlv_stream_027)"
	result += _checkTrue(MULTIAP_TLVPARSE053, multi_ap_tlv_stream_027, (INT8U *)&multi_ap_tlv_structure_027);

#define MULTIAP_TLVPARSE054 "MULTIAP_TLVPARSE054 - Parse UnassociatedSTALinkMetricsQueryTLV TLV (multi_ap_tlv_stream_027b)"
	result += _checkFalse(MULTIAP_TLVPARSE054, multi_ap_tlv_stream_027b, (INT8U *)&multi_ap_tlv_structure_027);

#define MULTIAP_TLVPARSE055 "MULTIAP_TLVPARSE055 - Parse UnassociatedSTALinkMetricsResponseTLV TLV (multi_ap_tlv_stream_028)"
	result += _checkTrue(MULTIAP_TLVPARSE055, multi_ap_tlv_stream_028, (INT8U *)&multi_ap_tlv_structure_028);

#define MULTIAP_TLVPARSE056 "MULTIAP_TLVPARSE056 - Parse UnassociatedSTALinkMetricsResponseTLV TLV (multi_ap_tlv_stream_028b)"
	result += _checkFalse(MULTIAP_TLVPARSE056, multi_ap_tlv_stream_028b, (INT8U *)&multi_ap_tlv_structure_028);

#define MULTIAP_TLVPARSE057 "MULTIAP_TLVPARSE057 - Parse BeaconMetricsQueryTLV TLV (multi_ap_tlv_stream_029)"
	result += _checkTrue(MULTIAP_TLVPARSE057, multi_ap_tlv_stream_029, (INT8U *)&multi_ap_tlv_structure_029);

#define MULTIAP_TLVPARSE058 "MULTIAP_TLVPARSE058 - Parse BeaconMetricsQueryTLV TLV (multi_ap_tlv_stream_029b)"
	result += _checkFalse(MULTIAP_TLVPARSE058, multi_ap_tlv_stream_029b, (INT8U *)&multi_ap_tlv_structure_029);

#define MULTIAP_TLVPARSE059 "MULTIAP_TLVPARSE059 - Parse BeaconMetricsResponseTLV TLV (multi_ap_tlv_stream_030)"
	result += _checkTrue(MULTIAP_TLVPARSE059, multi_ap_tlv_stream_030, (INT8U *)&multi_ap_tlv_structure_030);

#define MULTIAP_TLVPARSE060 "MULTIAP_TLVPARSE060 - Parse BeaconMetricsResponseTLV TLV (multi_ap_tlv_stream_030b)"
	result += _checkFalse(MULTIAP_TLVPARSE060, multi_ap_tlv_stream_030b, (INT8U *)&multi_ap_tlv_structure_030);

#define MULTIAP_TLVPARSE061 "MULTIAP_TLVPARSE061 - Parse SteeringRequestTLV TLV (multi_ap_tlv_stream_031)"
	result += _checkTrue(MULTIAP_TLVPARSE061, multi_ap_tlv_stream_031, (INT8U *)&multi_ap_tlv_structure_031);

#define MULTIAP_TLVPARSE062 "MULTIAP_TLVPARSE062 - Parse SteeringRequestTLV TLV (multi_ap_tlv_stream_031b)"
	result += _checkFalse(MULTIAP_TLVPARSE062, multi_ap_tlv_stream_031b, (INT8U *)&multi_ap_tlv_structure_031);

#define MULTIAP_TLVPARSE063 "MULTIAP_TLVPARSE063 - Parse SteeringBTMReportTLV TLV (multi_ap_tlv_stream_032)"
	result += _checkTrue(MULTIAP_TLVPARSE063, multi_ap_tlv_stream_032, (INT8U *)&multi_ap_tlv_structure_032);

#define MULTIAP_TLVPARSE064 "MULTIAP_TLVPARSE064 - Parse SteeringBTMReportTLV TLV (multi_ap_tlv_stream_032b)"
	result += _checkFalse(MULTIAP_TLVPARSE064, multi_ap_tlv_stream_032b, (INT8U *)&multi_ap_tlv_structure_032);

#define MULTIAP_TLVPARSE065 "MULTIAP_TLVPARSE065 - Parse SteeringBTMReportTLV TLV (multi_ap_tlv_stream_033)"
	result += _checkTrue(MULTIAP_TLVPARSE065, multi_ap_tlv_stream_033, (INT8U *)&multi_ap_tlv_structure_033);

#define MULTIAP_TLVPARSE066 "MULTIAP_TLVPARSE066 - Parse SteeringBTMReportTLV TLV (multi_ap_tlv_stream_033b)"
	result += _checkFalse(MULTIAP_TLVPARSE066, multi_ap_tlv_stream_033b, (INT8U *)&multi_ap_tlv_structure_033);

#define MULTIAP_TLVPARSE067 "MULTIAP_TLVPARSE067 - Parse BackhaulSteeringRequestTLV TLV (multi_ap_tlv_stream_034)"
	result += _checkTrue(MULTIAP_TLVPARSE067, multi_ap_tlv_stream_034, (INT8U *)&multi_ap_tlv_structure_034);

#define MULTIAP_TLVPARSE068 "MULTIAP_TLVPARSE068 - Parse BackhaulSteeringRequestTLV TLV (multi_ap_tlv_stream_034b)"
	result += _checkFalse(MULTIAP_TLVPARSE068, multi_ap_tlv_stream_034b, (INT8U *)&multi_ap_tlv_structure_034);

#define MULTIAP_TLVPARSE069 "MULTIAP_TLVPARSE069 - Parse BackhaulSteeringResponseTLV TLV (multi_ap_tlv_stream_035)"
	result += _checkTrue(MULTIAP_TLVPARSE069, multi_ap_tlv_stream_035, (INT8U *)&multi_ap_tlv_structure_035);

#define MULTIAP_TLVPARSE070 "MULTIAP_TLVPARSE070 - Parse BackhaulSteeringResponseTLV TLV (multi_ap_tlv_stream_035b)"
	result += _checkFalse(MULTIAP_TLVPARSE070, multi_ap_tlv_stream_035b, (INT8U *)&multi_ap_tlv_structure_035);

#define MULTIAP_TLVPARSE071 "MULTIAP_TLVPARSE071 - Parse HigherLayerDataTLV TLV (multi_ap_tlv_stream_036)"
	result += _checkTrue(MULTIAP_TLVPARSE071, multi_ap_tlv_stream_036, (INT8U *)&multi_ap_tlv_structure_036);

#define MULTIAP_TLVPARSE072 "MULTIAP_TLVPARSE072 - Parse HigherLayerDataTLV TLV (multi_ap_tlv_stream_036b)"
	result += _checkFalse(MULTIAP_TLVPARSE072, multi_ap_tlv_stream_036b, (INT8U *)&multi_ap_tlv_structure_036);

#define MULTIAP_TLVPARSE073 "MULTIAP_TLVPARSE073 - Parse AssociatedSTATrafficStatsTLV TLV (multi_ap_tlv_stream_037)"
	result += _checkTrue(MULTIAP_TLVPARSE073, multi_ap_tlv_stream_037, (INT8U *)&multi_ap_tlv_structure_037);

#define MULTIAP_TLVPARSE074 "MULTIAP_TLVPARSE074 - Parse AssociatedSTATrafficStatsTLV TLV (multi_ap_tlv_stream_037b)"
	result += _checkFalse(MULTIAP_TLVPARSE074, multi_ap_tlv_stream_037b, (INT8U *)&multi_ap_tlv_structure_037);

#define MULTIAP_TLVPARSE075 "MULTIAP_TLVPARSE075 - Parse Profile2APCapabilityTLV TLV (multi_ap_tlv_stream_038)"
	result += _checkTrue(MULTIAP_TLVPARSE075, multi_ap_tlv_stream_038, (INT8U *)&multi_ap_tlv_structure_038);

#define MULTIAP_TLVPARSE076 "MULTIAP_TLVPARSE076 - Parse Profile2APCapabilityTLV TLV (multi_ap_tlv_stream_038b)"
	result += _checkFalse(MULTIAP_TLVPARSE076, multi_ap_tlv_stream_038b, (INT8U *)&multi_ap_tlv_structure_038);

#define MULTIAP_TLVPARSE077 "MULTIAP_TLVPARSE077 - Parse ServicePrioritizationRuleTLV TLV (multi_ap_tlv_stream_039)"
	result += _checkTrue(MULTIAP_TLVPARSE077, multi_ap_tlv_stream_039, (INT8U *)&multi_ap_tlv_structure_039);

#define MULTIAP_TLVPARSE078 "MULTIAP_TLVPARSE078 - Parse ServicePrioritizationRuleTLV TLV (multi_ap_tlv_stream_039b)"
	result += _checkFalse(MULTIAP_TLVPARSE078, multi_ap_tlv_stream_039b, (INT8U *)&multi_ap_tlv_structure_039);

#define MULTIAP_TLVPARSE079 "MULTIAP_TLVPARSE079 - Parse DSCPMappingTableTLV TLV (multi_ap_tlv_stream_040)"
	result += _checkTrue(MULTIAP_TLVPARSE079, multi_ap_tlv_stream_040, (INT8U *)&multi_ap_tlv_structure_040);

#define MULTIAP_TLVPARSE080 "MULTIAP_TLVPARSE080 - Parse DSCPMappingTableTLV TLV (multi_ap_tlv_stream_040b)"
	result += _checkFalse(MULTIAP_TLVPARSE080, multi_ap_tlv_stream_040b, (INT8U *)&multi_ap_tlv_structure_040);

#define MULTIAP_TLVPARSE083 "MULTIAP_TLVPARSE083 - Parse Profile2ErrorCodeTLV TLV (multi_ap_tlv_stream_042)"
	result += _checkTrue(MULTIAP_TLVPARSE083, multi_ap_tlv_stream_042, (INT8U *)&multi_ap_tlv_structure_042);

#define MULTIAP_TLVPARSE084 "MULTIAP_TLVPARSE084 - Parse Profile2ErrorCodeTLV TLV (multi_ap_tlv_stream_042b)"
	result += _checkFalse(MULTIAP_TLVPARSE084, multi_ap_tlv_stream_042b, (INT8U *)&multi_ap_tlv_structure_042);

#define MULTIAP_TLVPARSE085 "MULTIAP_TLVPARSE085 - Parse APRadioAdvancedCapabilitiesTLV TLV (multi_ap_tlv_stream_043)"
	result += _checkTrue(MULTIAP_TLVPARSE085, multi_ap_tlv_stream_043, (INT8U *)&multi_ap_tlv_structure_043);

#define MULTIAP_TLVPARSE086 "MULTIAP_TLVPARSE086 - Parse APRadioAdvancedCapabilitiesTLV TLV (multi_ap_tlv_stream_043b)"
	result += _checkFalse(MULTIAP_TLVPARSE086, multi_ap_tlv_stream_043b, (INT8U *)&multi_ap_tlv_structure_043);

#define MULTIAP_TLVPARSE087 "MULTIAP_TLVPARSE087 - Parse UnsuccessfulAssociationPolicyTLV TLV (multi_ap_tlv_stream_044)"
	result += _checkTrue(MULTIAP_TLVPARSE087, multi_ap_tlv_stream_044, (INT8U *)&multi_ap_tlv_structure_044);

#define MULTIAP_TLVPARSE088 "MULTIAP_TLVPARSE088 - Parse UnsuccessfulAssociationPolicyTLV TLV (multi_ap_tlv_stream_044b)"
	result += _checkFalse(MULTIAP_TLVPARSE088, multi_ap_tlv_stream_044b, (INT8U *)&multi_ap_tlv_structure_044);

#define MULTIAP_TLVPARSE089 "MULTIAP_TLVPARSE089 - Parse MetricCollectionIntervalTLV TLV (multi_ap_tlv_stream_045)"
	result += _checkTrue(MULTIAP_TLVPARSE089, multi_ap_tlv_stream_045, (INT8U *)&multi_ap_tlv_structure_045);

#define MULTIAP_TLVPARSE090 "MULTIAP_TLVPARSE090 - Parse MetricCollectionIntervalTLV TLV (multi_ap_tlv_stream_045b)"
	result += _checkFalse(MULTIAP_TLVPARSE090, multi_ap_tlv_stream_045b, (INT8U *)&multi_ap_tlv_structure_045);

#define MULTIAP_TLVPARSE091 "MULTIAP_TLVPARSE091 - Parse RadioMetricsTLV TLV (multi_ap_tlv_stream_046)"
	result += _checkTrue(MULTIAP_TLVPARSE091, multi_ap_tlv_stream_046, (INT8U *)&multi_ap_tlv_structure_046);

#define MULTIAP_TLVPARSE092 "MULTIAP_TLVPARSE092 - Parse RadioMetricsTLV TLV (multi_ap_tlv_stream_046b)"
	result += _checkFalse(MULTIAP_TLVPARSE092, multi_ap_tlv_stream_046b, (INT8U *)&multi_ap_tlv_structure_046);

#define MULTIAP_TLVPARSE093 "MULTIAP_TLVPARSE093 - Parse APExtendedMetricsTLV TLV (multi_ap_tlv_stream_047)"
	result += _checkTrue(MULTIAP_TLVPARSE093, multi_ap_tlv_stream_047, (INT8U *)&multi_ap_tlv_structure_047);

#define MULTIAP_TLVPARSE094 "MULTIAP_TLVPARSE094 - Parse APExtendedMetricsTLV TLV (multi_ap_tlv_stream_047b)"
	result += _checkFalse(MULTIAP_TLVPARSE094, multi_ap_tlv_stream_047b, (INT8U *)&multi_ap_tlv_structure_047);

#define MULTIAP_TLVPARSE095 "MULTIAP_TLVPARSE095 - Parse AssociatedSTAExtendedLinkMericsTLV TLV (multi_ap_tlv_stream_048)"
	result += _checkTrue(MULTIAP_TLVPARSE095, multi_ap_tlv_stream_048, (INT8U *)&multi_ap_tlv_structure_048);

#define MULTIAP_TLVPARSE096 "MULTIAP_TLVPARSE096 - Parse AssociatedSTAExtendedLinkMericsTLV TLV (multi_ap_tlv_stream_048b)"
	result += _checkFalse(MULTIAP_TLVPARSE096, multi_ap_tlv_stream_048b, (INT8U *)&multi_ap_tlv_structure_048);

#define MULTIAP_TLVPARSE097 "MULTIAP_TLVPARSE097 - Parse StatusCodeTLV TLV (multi_ap_tlv_stream_049)"
	result += _checkTrue(MULTIAP_TLVPARSE097, multi_ap_tlv_stream_049, (INT8U *)&multi_ap_tlv_structure_049);

#define MULTIAP_TLVPARSE098 "MULTIAP_TLVPARSE098 - Parse StatusCodeTLV TLV (multi_ap_tlv_stream_049b)"
	result += _checkFalse(MULTIAP_TLVPARSE098, multi_ap_tlv_stream_049b, (INT8U *)&multi_ap_tlv_structure_049);

#define MULTIAP_TLVPARSE099 "MULTIAP_TLVPARSE099 - Parse ReasonCodeTLV TLV (multi_ap_tlv_stream_050)"
	result += _checkTrue(MULTIAP_TLVPARSE099, multi_ap_tlv_stream_050, (INT8U *)&multi_ap_tlv_structure_050);

#define MULTIAP_TLVPARSE0100 "MULTIAP_TLVPARSE0100 - Parse ReasonCodeTLV TLV (multi_ap_tlv_stream_050b)"
	result += _checkFalse(MULTIAP_TLVPARSE0100, multi_ap_tlv_stream_050b, (INT8U *)&multi_ap_tlv_structure_050);

#define MULTIAP_TLVPARSE0101 "MULTIAP_TLVPARSE0101 - Parse BackhaulSTARadioCapabilitiesTLV TLV (multi_ap_tlv_stream_051)"
	result += _checkTrue(MULTIAP_TLVPARSE0101, multi_ap_tlv_stream_051, (INT8U *)&multi_ap_tlv_structure_051);

#define MULTIAP_TLVPARSE0102 "MULTIAP_TLVPARSE0102 - Parse BackhaulSTARadioCapabilitiesTLV TLV (multi_ap_tlv_stream_051b)"
	result += _checkFalse(MULTIAP_TLVPARSE0102, multi_ap_tlv_stream_051b, (INT8U *)&multi_ap_tlv_structure_051);

#define MULTIAP_TLVPARSE0103 "MULTIAP_TLVPARSE0103 - Parse AKMSuiteCapabilitiesTLV TLV (multi_ap_tlv_stream_052)"
	result += _checkTrue(MULTIAP_TLVPARSE0103, multi_ap_tlv_stream_052, (INT8U *)&multi_ap_tlv_structure_052);

#define MULTIAP_TLVPARSE0104 "MULTIAP_TLVPARSE0104 - Parse AKMSuiteCapabilitiesTLV TLV (multi_ap_tlv_stream_052b)"
	result += _checkFalse(MULTIAP_TLVPARSE0104, multi_ap_tlv_stream_052b, (INT8U *)&multi_ap_tlv_structure_052);

#define MULTIAP_TLVPARSE0105 "MULTIAP_TLVPARSE0105 - Parse Encap1905DPPTLV TLV (multi_ap_tlv_stream_053)"
	result += _checkTrue(MULTIAP_TLVPARSE0105, multi_ap_tlv_stream_053, (INT8U *)&multi_ap_tlv_structure_053);

#define MULTIAP_TLVPARSE0106 "MULTIAP_TLVPARSE0106 - Parse Encap1905DPPTLV TLV (multi_ap_tlv_stream_053b)"
	result += _checkFalse(MULTIAP_TLVPARSE0106, multi_ap_tlv_stream_053b, (INT8U *)&multi_ap_tlv_structure_053);

#define MULTIAP_TLVPARSE0107 "MULTIAP_TLVPARSE0107 - Parse Encap1905EAPOLTLV TLV (multi_ap_tlv_stream_054)"
	result += _checkTrue(MULTIAP_TLVPARSE0107, multi_ap_tlv_stream_054, (INT8U *)&multi_ap_tlv_structure_054);

#define MULTIAP_TLVPARSE0108 "MULTIAP_TLVPARSE0108 - Parse Encap1905EAPOLTLV TLV (multi_ap_tlv_stream_054b)"
	result += _checkFalse(MULTIAP_TLVPARSE0108, multi_ap_tlv_stream_054b, (INT8U *)&multi_ap_tlv_structure_054);

#define MULTIAP_TLVPARSE0109 "MULTIAP_TLVPARSE0109 - Parse DPPBootstrappingURINotificationTLV TLV (multi_ap_tlv_stream_055)"
	result += _checkTrue(MULTIAP_TLVPARSE0109, multi_ap_tlv_stream_055, (INT8U *)&multi_ap_tlv_structure_055);

#define MULTIAP_TLVPARSE0110 "MULTIAP_TLVPARSE0110 - Parse DPPBootstrappingURINotificationTLV TLV (multi_ap_tlv_stream_055b)"
	result += _checkFalse(MULTIAP_TLVPARSE0110, multi_ap_tlv_stream_055b, (INT8U *)&multi_ap_tlv_structure_055);

#define MULTIAP_TLVPARSE0111 "MULTIAP_TLVPARSE0111 - Parse BackhaulBSSConfigurationTLV TLV (multi_ap_tlv_stream_056)"
	result += _checkTrue(MULTIAP_TLVPARSE0111, multi_ap_tlv_stream_056, (INT8U *)&multi_ap_tlv_structure_056);

#define MULTIAP_TLVPARSE0112 "MULTIAP_TLVPARSE0112 - Parse BackhaulBSSConfigurationTLV TLV (multi_ap_tlv_stream_056b)"
	result += _checkFalse(MULTIAP_TLVPARSE0112, multi_ap_tlv_stream_056b, (INT8U *)&multi_ap_tlv_structure_056);

#define MULTIAP_TLVPARSE0113 "MULTIAP_TLVPARSE0113 - Parse DPPChirpValueTLV TLV (multi_ap_tlv_stream_057)"
	result += _checkTrue(MULTIAP_TLVPARSE0113, multi_ap_tlv_stream_057, (INT8U *)&multi_ap_tlv_structure_057);

#define MULTIAP_TLVPARSE0114 "MULTIAP_TLVPARSE0114 - Parse DPPChirpValueTLV TLV (multi_ap_tlv_stream_057b)"
	result += _checkFalse(MULTIAP_TLVPARSE0114, multi_ap_tlv_stream_057b, (INT8U *)&multi_ap_tlv_structure_057);

#define MULTIAP_TLVPARSE0115 "MULTIAP_TLVPARSE0115 - Parse BSSConfigRequestTLV TLV (multi_ap_tlv_stream_058)"
	result += _checkTrue(MULTIAP_TLVPARSE0115, multi_ap_tlv_stream_058, (INT8U *)&multi_ap_tlv_structure_058);

#define MULTIAP_TLVPARSE0116 "MULTIAP_TLVPARSE0116 - Parse BSSConfigRequestTLV TLV (multi_ap_tlv_stream_058b)"
	result += _checkFalse(MULTIAP_TLVPARSE0116, multi_ap_tlv_stream_058b, (INT8U *)&multi_ap_tlv_structure_058);

#define MULTIAP_TLVPARSE0117 "MULTIAP_TLVPARSE0117 - Parse BSSConfigResponseTLV TLV (multi_ap_tlv_stream_059)"
	result += _checkTrue(MULTIAP_TLVPARSE0117, multi_ap_tlv_stream_059, (INT8U *)&multi_ap_tlv_structure_059);

#define MULTIAP_TLVPARSE0118 "MULTIAP_TLVPARSE0118 - Parse BSSConfigResponseTLV TLV (multi_ap_tlv_stream_059b)"
	result += _checkFalse(MULTIAP_TLVPARSE0118, multi_ap_tlv_stream_059b, (INT8U *)&multi_ap_tlv_structure_059);

#define MULTIAP_TLVPARSE0119 "MULTIAP_TLVPARSE0119 - Parse BSSConfigReportTLV TLV (multi_ap_tlv_stream_060)"
	result += _checkTrue(MULTIAP_TLVPARSE0119, multi_ap_tlv_stream_060, (INT8U *)&multi_ap_tlv_structure_060);

#define MULTIAP_TLVPARSE0120 "MULTIAP_TLVPARSE0120 - Parse BSSConfigReportTLV TLV (multi_ap_tlv_stream_060b)"
	result += _checkFalse(MULTIAP_TLVPARSE0120, multi_ap_tlv_stream_060b, (INT8U *)&multi_ap_tlv_structure_060);

#define MULTIAP_TLVPARSE0121 "MULTIAP_TLVPARSE0121 - Parse AnticipatedChannelPerferenceTLV TLV (multi_ap_tlv_stream_061)"
	result += _checkTrue(MULTIAP_TLVPARSE0121, multi_ap_tlv_stream_061, (INT8U *)&multi_ap_tlv_structure_061);

#define MULTIAP_TLVPARSE0122 "MULTIAP_TLVPARSE0122 - Parse AnticipatedChannelPerferenceTLV TLV (multi_ap_tlv_stream_061b)"
	result += _checkFalse(MULTIAP_TLVPARSE0122, multi_ap_tlv_stream_061b, (INT8U *)&multi_ap_tlv_structure_061);

#define MULTIAP_TLVPARSE0123 "MULTIAP_TLVPARSE0123 - Parse AnticipatedChannelUsageTLV TLV (multi_ap_tlv_stream_062)"
	result += _checkTrue(MULTIAP_TLVPARSE0123, multi_ap_tlv_stream_062, (INT8U *)&multi_ap_tlv_structure_062);

#define MULTIAP_TLVPARSE0124 "MULTIAP_TLVPARSE0124 - Parse AnticipatedChannelUsageTLV TLV (multi_ap_tlv_stream_062b)"
	result += _checkFalse(MULTIAP_TLVPARSE0124, multi_ap_tlv_stream_062b, (INT8U *)&multi_ap_tlv_structure_062);

#define MULTIAP_TLVPARSE0125 "MULTIAP_TLVPARSE0125 - Parse SpatialReuseRequestTLV TLV (multi_ap_tlv_stream_063)"
	result += _checkTrue(MULTIAP_TLVPARSE0125, multi_ap_tlv_stream_063, (INT8U *)&multi_ap_tlv_structure_063);

#define MULTIAP_TLVPARSE0126 "MULTIAP_TLVPARSE0126 - Parse SpatialReuseRequestTLV TLV (multi_ap_tlv_stream_063b)"
	result += _checkFalse(MULTIAP_TLVPARSE0126, multi_ap_tlv_stream_063b, (INT8U *)&multi_ap_tlv_structure_063);

#define MULTIAP_TLVPARSE0127 "MULTIAP_TLVPARSE0127 - Parse SpatialReuseReportTLV TLV (multi_ap_tlv_stream_064)"
	result += _checkTrue(MULTIAP_TLVPARSE0127, multi_ap_tlv_stream_064, (INT8U *)&multi_ap_tlv_structure_064);

#define MULTIAP_TLVPARSE0128 "MULTIAP_TLVPARSE0128 - Parse SpatialReuseReportTLV TLV (multi_ap_tlv_stream_064b)"
	result += _checkFalse(MULTIAP_TLVPARSE0128, multi_ap_tlv_stream_064b, (INT8U *)&multi_ap_tlv_structure_064);

#define MULTIAP_TLVPARSE0129 "MULTIAP_TLVPARSE0129 - Parse SpatialReuseConfigResponseTLV TLV (multi_ap_tlv_stream_065)"
	result += _checkTrue(MULTIAP_TLVPARSE0129, multi_ap_tlv_stream_065, (INT8U *)&multi_ap_tlv_structure_065);

#define MULTIAP_TLVPARSE0130 "MULTIAP_TLVPARSE0130 - Parse SpatialReuseConfigResponseTLV TLV (multi_ap_tlv_stream_065b)"
	result += _checkFalse(MULTIAP_TLVPARSE0130, multi_ap_tlv_stream_065b, (INT8U *)&multi_ap_tlv_structure_065);

#define MULTIAP_TLVPARSE0131 "MULTIAP_TLVPARSE0131 - Parse QoSManagementPolicyTLV TLV (multi_ap_tlv_stream_066)"
	result += _checkTrue(MULTIAP_TLVPARSE0131, multi_ap_tlv_stream_066, (INT8U *)&multi_ap_tlv_structure_066);

#define MULTIAP_TLVPARSE0132 "MULTIAP_TLVPARSE0132 - Parse QoSManagementPolicyTLV TLV (multi_ap_tlv_stream_066b)"
	result += _checkFalse(MULTIAP_TLVPARSE0132, multi_ap_tlv_stream_066b, (INT8U *)&multi_ap_tlv_structure_066);

#define MULTIAP_TLVPARSE0133 "MULTIAP_TLVPARSE0133 - Parse QoSManagementDescriptorTLV TLV (multi_ap_tlv_stream_067)"
	result += _checkTrue(MULTIAP_TLVPARSE0133, multi_ap_tlv_stream_067, (INT8U *)&multi_ap_tlv_structure_067);

#define MULTIAP_TLVPARSE0134 "MULTIAP_TLVPARSE0134 - Parse QoSManagementDescriptorTLV TLV (multi_ap_tlv_stream_067b)"
	result += _checkFalse(MULTIAP_TLVPARSE0134, multi_ap_tlv_stream_067b, (INT8U *)&multi_ap_tlv_structure_067);

#define MULTIAP_TLVPARSE0135 "MULTIAP_TLVPARSE0135 - Parse ControllerCapabilityTLV TLV (multi_ap_tlv_stream_068)"
	result += _checkTrue(MULTIAP_TLVPARSE0135, multi_ap_tlv_stream_068, (INT8U *)&multi_ap_tlv_structure_068);

#define MULTIAP_TLVPARSE0136 "MULTIAP_TLVPARSE0136 - Parse ControllerCapabilityTLV TLV (multi_ap_tlv_stream_068b)"
	result += _checkFalse(MULTIAP_TLVPARSE0136, multi_ap_tlv_stream_068b, (INT8U *)&multi_ap_tlv_structure_068);

	// Return the number of test cases that failed
	//
	return result;
}
