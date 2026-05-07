/*
 * Copyright (C) 2018 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _MULTI_AP_TLV_TEST_VECTORS_H_
#define _MULTI_AP_TLV_TEST_VECTORS_H_

#include "multi_ap_tlvs.h"
#include "multi_ap_tlvs_r2.h"
#include "multi_ap_tlvs_r3.h"
#include "multi_ap_tlvs_r4.h"

#define CHECK_TRUE     0 // Check a successful parse/forge operation
#define CHECK_FALSE    1 // Check a wrong parse operation (malformed frame)

extern struct SupportedServiceTLV                       multi_ap_tlv_structure_001;
extern INT8U                                            multi_ap_tlv_stream_001[];
extern INT8U                                            multi_ap_tlv_stream_001b[];
extern INT16U                                           multi_ap_tlv_stream_len_001;

extern struct SearchedServiceTLV                        multi_ap_tlv_structure_002;
extern INT8U                                            multi_ap_tlv_stream_002[];
extern INT8U                                            multi_ap_tlv_stream_002b[];
extern INT16U                                           multi_ap_tlv_stream_len_002;

extern struct ApRadioIdentifierTLV                      multi_ap_tlv_structure_003;
extern INT8U                                            multi_ap_tlv_stream_003[];
extern INT8U                                            multi_ap_tlv_stream_003b[];
extern INT16U                                           multi_ap_tlv_stream_len_003;

extern struct ApOperationalBSSTLV                       multi_ap_tlv_structure_004;
extern INT8U                                            multi_ap_tlv_stream_004[];
extern INT8U                                            multi_ap_tlv_stream_004b[];
extern INT16U                                           multi_ap_tlv_stream_len_004;

extern struct AssociatedClientsTLV                      multi_ap_tlv_structure_005;
extern INT8U                                            multi_ap_tlv_stream_005[];
extern INT8U                                            multi_ap_tlv_stream_005b[];
extern INT16U                                           multi_ap_tlv_stream_len_005;

extern struct APCapabilityTLV                           multi_ap_tlv_structure_006;
extern INT8U                                            multi_ap_tlv_stream_006[];
extern INT8U                                            multi_ap_tlv_stream_006b[];
extern INT16U                                           multi_ap_tlv_stream_len_006;

extern struct APRadioBasicCapabilitiesTLV               multi_ap_tlv_structure_007;
extern INT8U                                            multi_ap_tlv_stream_007[];
extern INT8U                                            multi_ap_tlv_stream_007b[];
extern INT16U                                           multi_ap_tlv_stream_len_007;

extern struct APHTCapabilitiesTLV                       multi_ap_tlv_structure_008;
extern INT8U                                            multi_ap_tlv_stream_008[];
extern INT8U                                            multi_ap_tlv_stream_008b[];
extern INT16U                                           multi_ap_tlv_stream_len_008;

extern struct APVHTCapabilitiesTLV                      multi_ap_tlv_structure_009;
extern INT8U                                            multi_ap_tlv_stream_009[];
extern INT8U                                            multi_ap_tlv_stream_009b[];
extern INT16U                                           multi_ap_tlv_stream_len_009;

extern struct APHECapabilitiesTLV                       multi_ap_tlv_structure_010;
extern INT8U                                            multi_ap_tlv_stream_010[];
extern INT8U                                            multi_ap_tlv_stream_010b[];
extern INT16U                                           multi_ap_tlv_stream_len_010;

extern struct ClientInfoTLV                             multi_ap_tlv_structure_011;
extern INT8U                                            multi_ap_tlv_stream_011[];
extern INT8U                                            multi_ap_tlv_stream_011b[];
extern INT16U                                           multi_ap_tlv_stream_len_011;

extern struct ClientCapabilityReportTLV                  multi_ap_tlv_structure_012;
extern INT8U                                            multi_ap_tlv_stream_012[];
extern INT8U                                            multi_ap_tlv_stream_012b[];
extern INT16U                                           multi_ap_tlv_stream_len_012;

extern struct ClientAssociationEventTLV                 multi_ap_tlv_structure_013;
extern INT8U                                            multi_ap_tlv_stream_013[];
extern INT8U                                            multi_ap_tlv_stream_013b[];
extern INT16U                                           multi_ap_tlv_stream_len_013;

extern struct STAMacAddressTypeTLV                      multi_ap_tlv_structure_014;
extern INT8U                                            multi_ap_tlv_stream_014[];
extern INT8U                                            multi_ap_tlv_stream_014b[];
extern INT16U                                           multi_ap_tlv_stream_len_014;

extern struct ClientAssociationControlRequestTLV        multi_ap_tlv_structure_015;
extern INT8U                                            multi_ap_tlv_stream_015[];
extern INT8U                                            multi_ap_tlv_stream_015b[];
extern INT16U                                           multi_ap_tlv_stream_len_015;

extern struct ErrorCodeTLV                              multi_ap_tlv_structure_016;
extern INT8U                                            multi_ap_tlv_stream_016[];
extern INT8U                                            multi_ap_tlv_stream_016b[];
extern INT16U                                           multi_ap_tlv_stream_len_016;

extern struct SteeringPolicyTLV                         multi_ap_tlv_structure_017;
extern INT8U                                            multi_ap_tlv_stream_017[];
extern INT8U                                            multi_ap_tlv_stream_017b[];
extern INT16U                                           multi_ap_tlv_stream_len_017;

extern struct MetricReportingPolicyTLV                  multi_ap_tlv_structure_018;
extern INT8U                                            multi_ap_tlv_stream_018[];
extern INT8U                                            multi_ap_tlv_stream_018b[];
extern INT16U                                           multi_ap_tlv_stream_len_018;

extern struct ChannelPreferenceTLV                      multi_ap_tlv_structure_019;
extern INT8U                                            multi_ap_tlv_stream_019[];
extern INT8U                                            multi_ap_tlv_stream_019b[];
extern INT16U                                           multi_ap_tlv_stream_len_019;

extern struct RadioOperationRestrictionTLV              multi_ap_tlv_structure_020;
extern INT8U                                            multi_ap_tlv_stream_020[];
extern INT8U                                            multi_ap_tlv_stream_020b[];
extern INT16U                                           multi_ap_tlv_stream_len_020;

extern struct TransmitPowerLimitTLV                     multi_ap_tlv_structure_021;
extern INT8U                                            multi_ap_tlv_stream_021[];
extern INT8U                                            multi_ap_tlv_stream_021b[];
extern INT16U                                           multi_ap_tlv_stream_len_021;

extern struct ChannelSelectionResponseTLV               multi_ap_tlv_structure_022;
extern INT8U                                            multi_ap_tlv_stream_022[];
extern INT8U                                            multi_ap_tlv_stream_022b[];
extern INT16U                                           multi_ap_tlv_stream_len_022;

extern struct OperatingChannelReportTLV                 multi_ap_tlv_structure_023;
extern INT8U                                            multi_ap_tlv_stream_023[];
extern INT8U                                            multi_ap_tlv_stream_023b[];
extern INT16U                                           multi_ap_tlv_stream_len_023;

extern struct APMetricQueryTLV                          multi_ap_tlv_structure_024;
extern INT8U                                            multi_ap_tlv_stream_024[];
extern INT8U                                            multi_ap_tlv_stream_024b[];
extern INT16U                                           multi_ap_tlv_stream_len_024;

extern struct APMetricsTLV                              multi_ap_tlv_structure_025;
extern INT8U                                            multi_ap_tlv_stream_025[];
extern INT8U                                            multi_ap_tlv_stream_025b[];
extern INT16U                                           multi_ap_tlv_stream_len_025;

extern struct AssociatedSTALinkMetricsTLV               multi_ap_tlv_structure_026;
extern INT8U                                            multi_ap_tlv_stream_026[];
extern INT8U                                            multi_ap_tlv_stream_026b[];
extern INT16U                                           multi_ap_tlv_stream_len_026;

extern struct UnassociatedSTALinkMetricsQueryTLV        multi_ap_tlv_structure_027;
extern INT8U                                            multi_ap_tlv_stream_027[];
extern INT8U                                            multi_ap_tlv_stream_027b[];
extern INT16U                                           multi_ap_tlv_stream_len_027;

extern struct UnassociatedSTALinkMetricsResponseTLV     multi_ap_tlv_structure_028;
extern INT8U                                            multi_ap_tlv_stream_028[];
extern INT8U                                            multi_ap_tlv_stream_028b[];
extern INT16U                                           multi_ap_tlv_stream_len_028;

extern struct BeaconMetricsQueryTLV                     multi_ap_tlv_structure_029;
extern INT8U                                            multi_ap_tlv_stream_029[];
extern INT8U                                            multi_ap_tlv_stream_029b[];
extern INT16U                                           multi_ap_tlv_stream_len_029;

extern struct BeaconMetricsResponseTLV                  multi_ap_tlv_structure_030;
extern INT8U                                            multi_ap_tlv_stream_030[];
extern INT8U                                            multi_ap_tlv_stream_030b[];
extern INT16U                                           multi_ap_tlv_stream_len_030;

extern struct SteeringRequestTLV                        multi_ap_tlv_structure_031;
extern INT8U                                            multi_ap_tlv_stream_031[];
extern INT8U                                            multi_ap_tlv_stream_031b[];
extern INT16U                                           multi_ap_tlv_stream_len_031;

extern struct SteeringBTMReportTLV                      multi_ap_tlv_structure_032;
extern INT8U                                            multi_ap_tlv_stream_032[];
extern INT8U                                            multi_ap_tlv_stream_032b[];
extern INT16U                                           multi_ap_tlv_stream_len_032;

extern struct SteeringBTMReportTLV                      multi_ap_tlv_structure_033;
extern INT8U                                            multi_ap_tlv_stream_033[];
extern INT8U                                            multi_ap_tlv_stream_033b[];
extern INT16U                                           multi_ap_tlv_stream_len_033;

extern struct BackhaulSteeringRequestTLV                multi_ap_tlv_structure_034;
extern INT8U                                            multi_ap_tlv_stream_034[];
extern INT8U                                            multi_ap_tlv_stream_034b[];
extern INT16U                                           multi_ap_tlv_stream_len_034;

extern struct BackhaulSteeringResponseTLV               multi_ap_tlv_structure_035;
extern INT8U                                            multi_ap_tlv_stream_035[];
extern INT8U                                            multi_ap_tlv_stream_035b[];
extern INT16U                                           multi_ap_tlv_stream_len_035;

extern struct HigherLayerDataTLV                        multi_ap_tlv_structure_036;
extern INT8U                                            multi_ap_tlv_stream_036[];
extern INT8U                                            multi_ap_tlv_stream_036b[];
extern INT16U                                           multi_ap_tlv_stream_len_036;

extern struct AssociatedSTATrafficStatsTLV              multi_ap_tlv_structure_037;
extern INT8U                                            multi_ap_tlv_stream_037[];
extern INT8U                                            multi_ap_tlv_stream_037b[];
extern INT16U                                           multi_ap_tlv_stream_len_037;

extern struct Profile2APCapabilityTLV                   multi_ap_tlv_structure_038;
extern INT8U                                            multi_ap_tlv_stream_038[];
extern INT8U                                            multi_ap_tlv_stream_038b[];
extern INT16U                                           multi_ap_tlv_stream_len_038;

extern struct ServicePrioritizationRuleTLV              multi_ap_tlv_structure_039;
extern INT8U                                            multi_ap_tlv_stream_039[];
extern INT8U                                            multi_ap_tlv_stream_039b[];
extern INT16U                                           multi_ap_tlv_stream_len_039;

extern struct DSCPMappingTableTLV                       multi_ap_tlv_structure_040;
extern INT8U                                            multi_ap_tlv_stream_040[];
extern INT8U                                            multi_ap_tlv_stream_040b[];
extern INT16U                                           multi_ap_tlv_stream_len_040;

extern struct Profile2ErrorCodeTLV                          multi_ap_tlv_structure_042;
extern INT8U                                                multi_ap_tlv_stream_042[];
extern INT8U                                                multi_ap_tlv_stream_042b[];
extern INT16U                                               multi_ap_tlv_stream_len_042;

extern struct APRadioAdvancedCapabilitiesTLV                multi_ap_tlv_structure_043;
extern INT8U                                                multi_ap_tlv_stream_043[];
extern INT8U                                                multi_ap_tlv_stream_043b[];
extern INT16U                                               multi_ap_tlv_stream_len_043;

extern struct UnsuccessfulAssociationPolicyTLV              multi_ap_tlv_structure_044;
extern INT8U                                                multi_ap_tlv_stream_044[];
extern INT8U                                                multi_ap_tlv_stream_044b[];
extern INT16U                                               multi_ap_tlv_stream_len_044;

extern struct MetricCollectionIntervalTLV                   multi_ap_tlv_structure_045;
extern INT8U                                                multi_ap_tlv_stream_045[];
extern INT8U                                                multi_ap_tlv_stream_045b[];
extern INT16U                                               multi_ap_tlv_stream_len_045;

extern struct RadioMetricsTLV                                multi_ap_tlv_structure_046;
extern INT8U                                                multi_ap_tlv_stream_046[];
extern INT8U                                                multi_ap_tlv_stream_046b[];
extern INT16U                                               multi_ap_tlv_stream_len_046;

extern struct APExtendedMetricsTLV                          multi_ap_tlv_structure_047;
extern INT8U                                                multi_ap_tlv_stream_047[];
extern INT8U                                                multi_ap_tlv_stream_047b[];
extern INT16U                                               multi_ap_tlv_stream_len_047;

extern struct AssociatedSTAExtendedLinkMericsTLV            multi_ap_tlv_structure_048;
extern INT8U                                                multi_ap_tlv_stream_048[];
extern INT8U                                                multi_ap_tlv_stream_048b[];
extern INT16U                                               multi_ap_tlv_stream_len_048;

extern struct StatusCodeTLV                                 multi_ap_tlv_structure_049;
extern INT8U                                                multi_ap_tlv_stream_049[];
extern INT8U                                                multi_ap_tlv_stream_049b[];
extern INT16U                                               multi_ap_tlv_stream_len_049;

extern struct ReasonCodeTLV                                 multi_ap_tlv_structure_050;
extern INT8U                                                multi_ap_tlv_stream_050[];
extern INT8U                                                multi_ap_tlv_stream_050b[];
extern INT16U                                               multi_ap_tlv_stream_len_050;

extern struct BackhaulSTARadioCapabilitiesTLV               multi_ap_tlv_structure_051;
extern INT8U                                                multi_ap_tlv_stream_051[];
extern INT8U                                                multi_ap_tlv_stream_051b[];
extern INT16U                                               multi_ap_tlv_stream_len_051;

extern struct AKMSuiteCapabilitiesTLV                       multi_ap_tlv_structure_052;
extern INT8U                                                multi_ap_tlv_stream_052[];
extern INT8U                                                multi_ap_tlv_stream_052b[];
extern INT16U                                               multi_ap_tlv_stream_len_052;

extern struct Encap1905DPPTLV                               multi_ap_tlv_structure_053;
extern INT8U                                                multi_ap_tlv_stream_053[];
extern INT8U                                                multi_ap_tlv_stream_053b[];
extern INT16U                                               multi_ap_tlv_stream_len_053;

extern struct Encap1905EAPOLTLV                             multi_ap_tlv_structure_054;
extern INT8U                                                multi_ap_tlv_stream_054[];
extern INT8U                                                multi_ap_tlv_stream_054b[];
extern INT16U                                               multi_ap_tlv_stream_len_054;

extern struct DPPBootstrappingURINotificationTLV            multi_ap_tlv_structure_055;
extern INT8U                                                multi_ap_tlv_stream_055[];
extern INT8U                                                multi_ap_tlv_stream_055b[];
extern INT16U                                               multi_ap_tlv_stream_len_055;

extern struct BackhaulBSSConfigurationTLV                   multi_ap_tlv_structure_056;
extern INT8U                                                multi_ap_tlv_stream_056[];
extern INT8U                                                multi_ap_tlv_stream_056b[];
extern INT16U                                               multi_ap_tlv_stream_len_056;

extern struct DPPChirpValueTLV                              multi_ap_tlv_structure_057;
extern INT8U                                                multi_ap_tlv_stream_057[];
extern INT8U                                                multi_ap_tlv_stream_057b[];
extern INT16U                                               multi_ap_tlv_stream_len_057;

extern struct BSSConfigRequestTLV                           multi_ap_tlv_structure_058;
extern INT8U                                                multi_ap_tlv_stream_058[];
extern INT8U                                                multi_ap_tlv_stream_058b[];
extern INT16U                                               multi_ap_tlv_stream_len_058;

extern struct BSSConfigResponseTLV                          multi_ap_tlv_structure_059;
extern INT8U                                                multi_ap_tlv_stream_059[];
extern INT8U                                                multi_ap_tlv_stream_059b[];
extern INT16U                                               multi_ap_tlv_stream_len_059;

extern struct BSSConfigReportTLV                            multi_ap_tlv_structure_060;
extern INT8U                                                multi_ap_tlv_stream_060[];
extern INT8U                                                multi_ap_tlv_stream_060b[];
extern INT16U                                               multi_ap_tlv_stream_len_060;

extern struct AnticipatedChannelPerferenceTLV               multi_ap_tlv_structure_061;
extern INT8U                                                multi_ap_tlv_stream_061[];
extern INT8U                                                multi_ap_tlv_stream_061b[];
extern INT16U                                               multi_ap_tlv_stream_len_061;

extern struct AnticipatedChannelUsageTLV                    multi_ap_tlv_structure_062;
extern INT8U                                                multi_ap_tlv_stream_062[];
extern INT8U                                                multi_ap_tlv_stream_062b[];
extern INT16U                                               multi_ap_tlv_stream_len_062;

extern struct SpatialReuseRequestTLV                        multi_ap_tlv_structure_063;
extern INT8U                                                multi_ap_tlv_stream_063[];
extern INT8U                                                multi_ap_tlv_stream_063b[];
extern INT16U                                               multi_ap_tlv_stream_len_063;

extern struct SpatialReuseReportTLV                         multi_ap_tlv_structure_064;
extern INT8U                                                multi_ap_tlv_stream_064[];
extern INT8U                                                multi_ap_tlv_stream_064b[];
extern INT16U                                               multi_ap_tlv_stream_len_064;

extern struct SpatialReuseConfigResponseTLV                 multi_ap_tlv_structure_065;
extern INT8U                                                multi_ap_tlv_stream_065[];
extern INT8U                                                multi_ap_tlv_stream_065b[];
extern INT16U                                               multi_ap_tlv_stream_len_065;

extern struct QoSManagementPolicyTLV                        multi_ap_tlv_structure_066;
extern INT8U                                                multi_ap_tlv_stream_066[];
extern INT8U                                                multi_ap_tlv_stream_066b[];
extern INT16U                                               multi_ap_tlv_stream_len_066;

extern struct QoSManagementDescriptorTLV                    multi_ap_tlv_structure_067;
extern INT8U                                                multi_ap_tlv_stream_067[];
extern INT8U                                                multi_ap_tlv_stream_067b[];
extern INT16U                                               multi_ap_tlv_stream_len_067;

extern struct ControllerCapabilityTLV                       multi_ap_tlv_structure_068;
extern INT8U                                                multi_ap_tlv_stream_068[];
extern INT8U                                                multi_ap_tlv_stream_068b[];
extern INT16U                                               multi_ap_tlv_stream_len_068;

#endif

