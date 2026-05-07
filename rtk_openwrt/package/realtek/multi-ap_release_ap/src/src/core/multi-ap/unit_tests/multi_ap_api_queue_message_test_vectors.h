/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _MULTI_AP_API_REQUEST_TEST_VECTORS_H_
#define _MULTI_AP_API_REQUEST_TEST_VECTORS_H_

#include "../map_utils.h"

#define CHECK_TRUE     0 // Check a successful parse/forge operation
#define CHECK_FALSE    1 // Check a wrong parse operation (malformed frame)

extern struct map_topology_query_request                multi_ap_api_request_structure_001;
extern INT8U                                            multi_ap_api_request_stream_001[];
extern INT8U                                            multi_ap_api_request_stream_001b[];
extern INT16U                                           multi_ap_api_request_stream_len_001;

extern struct map_link_metric_query                     multi_ap_api_request_structure_002;
extern INT8U                                            multi_ap_api_request_stream_002[];
extern INT8U                                            multi_ap_api_request_stream_002b[];
extern INT16U                                           multi_ap_api_request_stream_len_002;

extern struct map_autoconfig_renew_request                      multi_ap_api_request_structure_003;
extern INT8U                                            multi_ap_api_request_stream_003[];
extern INT8U                                            multi_ap_api_request_stream_003b[];
extern INT16U                                           multi_ap_api_request_stream_len_003;

extern struct map_channel_preference_query                       multi_ap_api_request_structure_004;
extern INT8U                                            multi_ap_api_request_stream_004[];
extern INT8U                                            multi_ap_api_request_stream_004b[];
extern INT16U                                           multi_ap_api_request_stream_len_004;

extern struct map_channel_selection_request                      multi_ap_api_request_structure_005;
extern INT8U                                            multi_ap_api_request_stream_005[];
extern INT8U                                            multi_ap_api_request_stream_005b[];
extern INT16U                                           multi_ap_api_request_stream_len_005;

extern struct map_client_steering_request                           multi_ap_api_request_structure_006;
extern INT8U                                            multi_ap_api_request_stream_006[];
extern INT8U                                            multi_ap_api_request_stream_006b[];
extern INT16U                                           multi_ap_api_request_stream_len_006;

extern struct map_client_association_control_request               multi_ap_api_request_structure_007;
extern INT8U                                            multi_ap_api_request_stream_007[];
extern INT8U                                            multi_ap_api_request_stream_007b[];
extern INT16U                                           multi_ap_api_request_stream_len_007;

extern struct map_backhaul_steering_request                       multi_ap_api_request_structure_008;
extern INT8U                                            multi_ap_api_request_stream_008[];
extern INT8U                                            multi_ap_api_request_stream_008b[];
extern INT16U                                           multi_ap_api_request_stream_len_008;

extern struct map_policy_config_request                      multi_ap_api_request_structure_009;
extern INT8U                                            multi_ap_api_request_stream_009[];
extern INT8U                                            multi_ap_api_request_stream_009b[];
extern INT16U                                           multi_ap_api_request_stream_len_009;

extern struct map_ap_capability_query                       multi_ap_api_request_structure_010;
extern INT8U                                            multi_ap_api_request_stream_010[];
extern INT8U                                            multi_ap_api_request_stream_010b[];
extern INT16U                                           multi_ap_api_request_stream_len_010;

extern struct map_client_capability_query                             multi_ap_api_request_structure_011;
extern INT8U                                            multi_ap_api_request_stream_011[];
extern INT8U                                            multi_ap_api_request_stream_011b[];
extern INT16U                                           multi_ap_api_request_stream_len_011;

extern struct map_ap_metric_query                  multi_ap_api_request_structure_012;
extern INT8U                                            multi_ap_api_request_stream_012[];
extern INT8U                                            multi_ap_api_request_stream_012b[];
extern INT16U                                           multi_ap_api_request_stream_len_012;

extern struct map_assoc_sta_metric_query                 multi_ap_api_request_structure_013;
extern INT8U                                            multi_ap_api_request_stream_013[];
extern INT8U                                            multi_ap_api_request_stream_013b[];
extern INT16U                                           multi_ap_api_request_stream_len_013;

extern struct map_unassoc_sta_metric_query                      multi_ap_api_request_structure_014;
extern INT8U                                            multi_ap_api_request_stream_014[];
extern INT8U                                            multi_ap_api_request_stream_014b[];
extern INT16U                                           multi_ap_api_request_stream_len_014;

extern struct map_beacon_metric_query               multi_ap_api_request_structure_015;
extern INT8U                                            multi_ap_api_request_stream_015[];
extern INT8U                                            multi_ap_api_request_stream_015b[];
extern INT16U                                           multi_ap_api_request_stream_len_015;

#endif

