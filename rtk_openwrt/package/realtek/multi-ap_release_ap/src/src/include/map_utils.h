/*
 * Copyright (C) 2018 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _MAP_UTIL_H_
#define _MAP_UTIL_H_
#include <stdint.h>
#include "map_initialization.h"

#define CONTROLLER_LOST 1
#define CONTROLLER_FOUND 2

#define MAP_API_REQUEST                                    0x20
#define MAP_API_RESPONSE                                   0x21

#define MAP_TOPOLOGY_QUERY_REQUEST_ELEMENT                 0x01
#define MAP_LINK_METRIC_QUERY_ELEMENT                      0x02
#define MAP_AUTOCONFIG_RENEW_REQUEST_ELEMENT               0x03
#define MAP_CHANNEL_PREFERENCE_QUERY_ELEMENT               0x04
#define MAP_CHANNEL_SELECTION_REQUEST_ELEMENT              0x05
#define MAP_CLIENT_STEERING_REQUEST_ELEMENT                0x06
#define MAP_CLIENT_ASSOCIATION_CONTROL_REQUEST_ELEMENT     0x07
#define MAP_BACKHAUL_STEERING_REQUEST_ELEMENT              0x08
#define MAP_POLICY_CONFIG_REQUEST_ELEMENT                  0x09
#define MAP_AP_CAPABILITY_QUERY_ELEMENT                    0x0A
#define MAP_CLIENT_CAPABILITY_QUERY_ELEMENT                0x0B
#define MAP_AP_METRIC_QUERY_ELEMENT                        0x0C
#define MAP_ASSOC_STA_METRIC_QUERY_ELEMENT                 0x0D
#define MAP_UNASSOC_STA_METRIC_QUERY_ELEMENT               0x0E
#define MAP_BEACON_METRIC_QUERY_ELEMENT                    0x0F
#define MAP_VENDOR_SPECIFIC_ELEMENT                        0x10
#define MAP_AUTOCONFIG_SEARCH_REQUEST_ELEMENT              0x11
#define MAP_CAC_REQUEST_ELEMENT                            0x12
#define MAP_CHANNEL_SCAN_REQUEST_ELEMENT                   0x13
#define MAP_CLIENT_STEERING_REQUEST_OPPORTUNITY_ELEMENT    0x14

#define MAP_GENERAL_ACKNOWLEDGEMENT_ELEMENT                0x50
#define MAP_TOPOLOGY_NOTIFICATION_ELEMENT                  0x51
#define MAP_TOPOLOGY_RESPONSE_ELEMENT                      0x52
#define MAP_CHANNEL_PREFERENCE_REPORT_ELEMENT              0x53
#define MAP_CHANNEL_SELECTION_RESPONSE_ELEMENT             0x54
#define MAP_OPERATING_CHANNEL_REPORT_ELEMENT               0x55
#define MAP_AP_CAPABILITY_REPORT_ELEMENT                   0x56
#define MAP_CLIENT_CAPABILITY_REPORT_ELEMENT               0x57
#define MAP_LINK_METRIC_RESPONSE_ELEMENT                   0x58
#define MAP_AP_METRIC_RESPONSE_ELEMENT                     0x59
#define MAP_ASSOC_STA_METRIC_RESPONSE_ELEMENT              0x5A
#define MAP_UNASSOC_STA_METRIC_RESPONSE_ELEMENT            0x5B
#define MAP_BEACON_METRIC_RESPONSE_ELEMENT                 0x5C
#define MAP_CLIENT_STEERING_BTM_REPORT_ELEMENT             0x5D
#define MAP_STEERING_COMPLETE_ELEMENT                      0x5E
#define MAP_BACKHAUL_STEERING_RESPONSE_ELEMENT             0x5F
#define MAP_AUTOCONFIG_RENEW_NOTIFICATION_ELEMENT          0x60
#define MAP_1905_VENDOR_SPECIFIC_CMDU_ELEMENT              0x61
#define MAP_WSC_M1_ELEMENT                                 0x62
#define MAP_AUTOCONFIG_RESPONSE_ELEMENT                    0x63
#define MAP_POLICY_CONFIG_NOTIFICATION_ELEMENT             0x64
#define MAP_CHANNEL_SCAN_REPORT_ELEMENT                    0x65

#define MAP_PUSH_BUTTON_NOTIFICATION_ELEMENT               0x70
#define MAP_CONFIGURE_NOTIFICATION_ELEMENT                 0x71
#define MAP_CONTROLLER_CHANGE_NOTIFICATION_ELEMENT         0x72
#define MAP_OPERATING_CHANNEL_CHANGE_NOTIFICATION_ELEMENT  0x73
#define MAP_CUSTOMIZED_STATUS_NOTIFICATION_ELEMENT         0x74
#define MAP_DPP_CONFIGURATION_ELEMENT                      0x75

#define MAP_VLAN_CONFIGURATION_ELEMENT                     0x80
#define MAP_VLAN_APPLY_NOTIFICATION_ELEMENT                0x81
#define MAP_VLAN_CLEAR_NOTIFICATION_ELEMENT                0x82


#define MAP_CMDU_TLV_ELEMENT                               0x90

struct map_topology_query_request {
	uint8_t element_id;
	uint8_t target_al_mac[6];
};

struct map_autoconfig_search_request {
	uint8_t element_id;
};

struct map_autoconfig_renew_request {
	uint8_t element_id;
	uint8_t include_self;
	uint8_t target_al_mac[6];
};

typedef struct _map_response_element {
	uint8_t   element_id;
	uint8_t   src_mac[6];
	uint8_t   tlv_nr;
	uint8_t **list_of_tlvs;
} map_response_element;

// Indicate whether a message is successfully sent or not
struct map_general_acknowledgement {
	uint8_t element_id;
	uint8_t request_type;
	uint8_t dest_mac[6];
	uint8_t status;
};

typedef struct _map_message {
	uint8_t  message_type;
	uint16_t message_len;
	uint8_t *element;
} map_message;

struct map_channel_preference_query {
	uint8_t element_id;
	uint8_t target_al_mac[6];
};

struct channel_preference_op_class {
	uint8_t  op_class;
	uint8_t  channel_nr;
	uint8_t *channel_list;
	uint8_t  preference_reason_code;
};

struct channel_preference {
	uint8_t                             radio_mac[6];
	uint8_t                             op_class_nr;
	struct channel_preference_op_class *ch_pre_op_class;
};

struct transmit_power_limit {
	uint8_t radio_mac[6];
	uint8_t tx_pwr_limit;
};

struct map_channel_selection_request {
	uint8_t                      element_id;
	uint8_t                      target_al_mac[6];
	uint8_t                      ch_preference_nr;
	struct channel_preference *  ch_preference;
	uint8_t                      tx_pwr_limit_nr;
	struct transmit_power_limit *tx_pwr_limit;
};

struct cac_request_radio {
	uint8_t	radio_id[6];
	uint8_t	op_class;
	uint8_t	channel;
	uint8_t	flags;//Bit 7-5 CAC method to be used.(0: Continuous CAC, 1: Continuous with dedicated radio, 2: MIMO dimension reduced, 3: Time sliced CAC, >3: Reserved)
                  //Bit 4-3 CAC Completion Action for Successful CAC.(0: Remain on channel and continue to monitor for radar, 1: Return the radio that was performing the CAC to its most recent operational configuration., >1: Reserved)
                  //Bits 2-0 (Reserved)
};

struct cac_request {
	uint8_t radio_nr;
	struct  cac_request_radio* radios;
};

struct map_cac_request {
	uint8_t element_id;
	uint8_t target_al_mac[6];
    struct  cac_request cac_req;
};

struct cac_report_pair {
	uint8_t	op_class;
	uint8_t	channel;
};

struct cac_report_radio {
	uint8_t	radio_id[6];
	uint8_t	op_class;
	uint8_t	channel;
	uint8_t	flags;
	uint8_t pairs_nr;
	struct  cac_report_pair *pairs;
};

struct cac_completion_report {
	uint8_t radio_nr;
    struct  cac_report_radio *radios;
};

struct channel_scan_target_op_class
{
    uint8_t	 op_class;
    uint8_t	 target_channel_nr;
    uint8_t* target_channel_list;
};

struct channel_scan_target_radio
{
    uint8_t	radio_id[6];
    uint8_t	target_op_class_nr;
    struct  channel_scan_target_op_class* target_op_classes;
};

struct channel_scan_request
{
	uint8_t	flags;  //bit 7 - 1: Perform a fresh scan and return results 0: Return stored results of last successful scan     bits 6-0 - reserverd
	uint8_t	target_radio_nr;
    struct  channel_scan_target_radio* target_radios;
};

struct map_channel_scan_request {
	uint8_t element_id;
	uint8_t target_al_mac[6];
	struct  channel_scan_request channel_scan_req;
};

struct channel_list {
	uint8_t  op_class;
	uint8_t	 channel;
	uint8_t  scan_status;
	uint8_t  utilization;
	uint8_t  noise;
	uint16_t neighbor_nr;
};

struct channel_scan_report {
	uint8_t channel_nr;
	struct  channel_list *channels;
};

struct target_bss {
	uint8_t bssid[6];
	uint8_t op_class;
	uint8_t channel;
	uint8_t reason_code;
};

struct steering_request {
	uint8_t  bssid[6];
	uint8_t  mode;
	uint16_t window;
	uint16_t disassoc_timer;
	uint8_t  sta_nr;
	uint8_t (*sta_mac_address)[6];
	uint8_t	 mbo_cap;
	uint8_t            target_bss_nr;
	struct target_bss *target_bss;
};

struct map_client_steering_request {
	uint8_t                 element_id;
	uint8_t                 target_al_mac[6];
	struct steering_request steering_req;
};

struct client_association_control_request {
	uint8_t  bssid[6];
	uint8_t  assoc_ctrl;
	uint16_t validity_period;
	uint8_t  sta_nr;
	uint8_t (*sta_mac_address)[6];
};

struct map_client_association_control_request {
	uint8_t                                    element_id;
	uint8_t                                    target_al_mac[6];
	uint8_t                                    client_assoc_ctrl_req_nr;
	struct client_association_control_request *client_assoc_ctrl_req;
};

struct map_backhaul_steering_request {
	uint8_t element_id;
	uint8_t target_al_mac[6];
	uint8_t backhaul_sta_mac[6];
	uint8_t target_bssid[6];
	uint8_t op_class;
	uint8_t channel;
};

struct map_link_metric_query {
	uint8_t element_id;
	uint8_t target_al_mac[6];
	uint8_t destination;
	uint8_t type;
	uint8_t sta_mac[6];
};

struct radio_steer_config {
	uint8_t radio_mac[6];
	uint8_t policy;
	uint8_t cu_threshold;
	uint8_t rcpi_threshold;
};

struct steering_config {
	uint8_t ls_sta_nr;
	uint8_t (*ls_stas)[6];
	uint8_t btm_sta_nr;
	uint8_t (*btm_stas)[6];
	uint8_t                    radio_nr;
	struct radio_steer_config *radio_steer_policy_data;
};

struct radio_metric_config {
	uint8_t radio_mac[6];
	uint8_t rcpi_threshold;
	uint8_t rcpi_hysteresis;
	uint8_t cu_threshold;
	uint8_t inclusion_policy;
};

struct metric_config {
	uint8_t                     report_interval;
	uint8_t                     radio_nr;
	struct radio_metric_config *radio_metric_policy_data;
};

struct default8021q_config {
	uint16_t primary_vlan_id;
	uint8_t  default_pcp;
};

struct traffic_separation_ssidInfo {
	uint8_t  ssid_length;
	char *   ssid;
	uint16_t vlan_id;
};

struct traffic_separation_config {
	uint8_t                             ssid_nr;
	struct traffic_separation_ssidInfo *ssid_info;
};

struct map_policy_config_request {
	uint8_t                          element_id;
	uint8_t                          target_al_mac[6];
	uint8_t                          steer_policy_inclusion;
	struct steering_config           steer_policy_data;
	uint8_t                          metric_policy_inclusion;
	struct metric_config             metric_policy_data;
	uint8_t                          default8021q_inclusion;
	struct default8021q_config       default8021q_data;
	uint8_t                          traffic_separation_inclusion;
	struct traffic_separation_config traffic_separation_data;
};

struct map_ap_capability_query {
	uint8_t element_id;
	uint8_t target_al_mac[6];
};

struct map_client_capability_query {
	uint8_t element_id;
	uint8_t target_al_mac[6];
	uint8_t bssid[6];
	uint8_t sta_mac[6];
};

struct map_ap_metric_query {
	uint8_t element_id;
	uint8_t target_al_mac[6];
	uint8_t bssid_nr;
	uint8_t (*bssid)[6];
};

struct map_assoc_sta_metric_query {
	uint8_t element_id;
	uint8_t target_al_mac[6];
	uint8_t sta_mac[6];
};

struct unassoc_sta_channel {
	uint8_t channel_nr;
	uint8_t sta_nr;
	uint8_t (*stas)[6];
};

struct map_unassoc_sta_metric_query {
	uint8_t                     element_id;
	uint8_t                     target_al_mac[6];
	uint8_t                     op_class;
	uint8_t                     channel_data_nr;
	struct unassoc_sta_channel *channel_data;
};

struct ap_channel_report {
	uint8_t  len;
	uint8_t  op_class;
	uint8_t *channel_list;
};

struct beacon_query_data {
	uint8_t                   sta_mac[6];
	uint8_t                   op_class;
	uint8_t                   channel_nr;
	uint8_t                   bssid[6];
	uint8_t                   report_detail;
	uint8_t                   ssid_len;
	char *                    ssid;
	uint8_t                   channel_report_nr;
	struct ap_channel_report *channel_report;
	uint8_t                   element_nr;
	uint8_t *                 element_list;
};

struct map_beacon_metric_query {
	uint8_t                  element_id;
	uint8_t                  target_al_mac[6];
	struct beacon_query_data beacon_metric_query;
};

struct map_configure_notification {
	uint8_t                   element_id;
	uint8_t                   config_nr;
	struct radio_config_data *config_data;
};

struct map_vendor_specific_message {
	uint8_t                  element_id;
	uint8_t                  target_al_mac[6];
	uint8_t                  relay_indicator;
	uint8_t                  vendor_oui[3];   // Vendor specific OUI
	uint16_t                 vendor_data_len; // Bytes in the following field
	uint8_t                 *vendor_data;     // Vendor specific information
};

struct map_controller_change_element {
	uint8_t                  element_id;
	uint8_t                  event_type;
	uint8_t                  controller_al_mac[6];
	char*                    receiving_interface;
};

struct map_customized_status_notification{
	uint8_t                  element_id;
	uint8_t                  customized_status;
};

struct map_dpp_configuration {
	uint8_t element_id;
	char *  signed_connector_1905;
	char *  netaccess_key;
	char *  csign_key;
	char *  pp_key;
};

struct map_operating_channel_change_notification {
	uint8_t	element_id;
};

typedef struct _map_cmdu_message {
	uint16_t  cmdu_type;
	uint8_t   src_mac[6];
	uint8_t   tlv_nr;
	uint8_t **list_of_tlvs;
} map_cmdu_message;

enum STATUS {
	STATUS_CONTROLLER_LOST = 1,
	STATUS_M1_SEND         = 2,
	STATUS_M2_PROCESS_FAIL = 3,
	STATUS_AUTOCONFIG_DONE = 4,
};

uint8_t *map_parse_element(uint8_t *element);

map_message *map_parse_message(uint8_t *message);

uint8_t map_free_element(uint8_t *element);

map_cmdu_message *map_parse_de_message(uint8_t *message);

uint8_t map_free_de_element(map_cmdu_message *message);

#endif
