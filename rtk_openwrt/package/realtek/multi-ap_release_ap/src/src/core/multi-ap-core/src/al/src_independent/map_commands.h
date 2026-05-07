/*
 * Copyright (C) 2018 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _MAP_COMMANDS_H_
#define _MAP_COMMANDS_H_
#include <stdint.h>
#include <unistd.h>
#include "map_initialization.h"

uint8_t update_config_data(uint8_t config_nr, struct radio_config_data *radio_config);

uint8_t send_topology_query_request(uint8_t target_al_mac[6]);

uint8_t send_autoconfig_search_request();

uint8_t send_autoconfig_renew_request(uint8_t target_al_mac[6]);

uint8_t send_channel_preference_query(uint8_t target_al_mac[6]);

uint8_t send_channel_selection_request(uint8_t                      target_al_mac[6],
                                       uint8_t                      ch_preference_nr,
                                       struct channel_preference *  ch_preference,
                                       uint8_t                      tx_pwr_limit_nr,
                                       struct transmit_power_limit *tx_pwr_limit);

uint8_t send_cac_request(uint8_t target_al_mac[6], struct cac_request cac_req);

uint8_t send_channel_scan_request(uint8_t target_al_mac[6], struct channel_scan_request channel_scan_req);

uint8_t send_client_steering_request(uint8_t target_al_mac[6], struct steering_request steering_req);

uint8_t send_client_association_control_request(uint8_t target_al_mac[6],
                                                uint8_t client_assoc_ctrl_req_nr,
                                                struct client_association_control_request *client_assoc_ctrl_req);

uint8_t send_backhaul_steering_request( uint8_t target_al_mac[6],
									    uint8_t backhaul_sta_mac[6],
                                        uint8_t target_bssid[6],
                                        uint8_t op_class,
                                        uint8_t channel);

#define LINK_METRIC_QUERY_DESTINATION_ALL_NEIGHBORS                (0x00)
#define LINK_METRIC_QUERY_DESTINATION_SPECIFIC_NEIGHBOR            (0x01)
#define LINK_METRIC_QUERY_TYPE_TX_LINK_METRICS_ONLY                (0x00)
#define LINK_METRIC_QUERY_TYPE_RX_LINK_METRICS_ONLY                (0x01)
#define LINK_METRIC_QUERY_TYPE_BOTH_TX_AND_RX_LINK_METRICS         (0x02)
uint8_t send_link_metric_query(         uint8_t target_al_mac[6],
									    uint8_t destination,
                                        uint8_t type,
                                        uint8_t sta_mac[6]);

uint8_t send_policy_config_request(uint8_t target_al_mac[6], uint8_t steer_policy_inclusion, struct steering_config steer_policy_data, uint8_t metric_policy_inclusion, struct metric_config metric_policy_data, uint8_t default8021q_inclusion, struct default8021q_config default8021q_data, uint8_t traffic_separation_policy_inclusion, struct traffic_separation_config traffic_separation_data);


uint8_t send_ap_capabilty_query(uint8_t target_al_mac[6]);

uint8_t send_client_capability_query(uint8_t target_al_mac[6], uint8_t bssid[6], uint8_t sta_mac[6]);

uint8_t send_ap_metric_query(uint8_t target_al_mac[6], uint8_t bssid_nr, uint8_t (*bssid)[6]);

uint8_t send_associated_sta_query(uint8_t target_al_mac[6], uint8_t sta_mac[6]);

uint8_t send_unassociated_sta_query(uint8_t target_al_mac[6], uint8_t op_class, uint8_t channel_data_nr, struct unassoc_sta_channel *channel_data);

uint8_t send_beacon_metric_query(uint8_t target_al_mac[6], struct beacon_query_data beacon_metric_query);

uint8_t send_vendor_specific_message(uint8_t target_al_mac[6], uint8_t relay_indicator, uint8_t vendor_oui[3], uint16_t data_len, uint8_t *data_payload);

uint8_t send_vlan_configured_notification();

uint8_t send_vlan_clear_notification();

uint8_t send_operating_channel_change_notification();

uint8_t register_vendor_message_receive_callback_function(void (*callback_func)(uint8_t **tlvs));

uint8_t register_append_vendor_tlv_callback_function(void (*tlv_append_callback_func)(uint8_t *vendor_tlv), void (*receive_callback_func)(uint8_t **tlvs));

#endif
