/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _MAP_DATAELEMENT_H_
#define _MAP_DATAELEMENT_H_

#include <stdint.h>
#include "map_tlvs.h"

#define MAX_DEVICE_NUM_PER_NETWORK 16
#define MAX_RADIO_NUM_PER_DEVICE 3
#define MAX_BSS_NUM_PER_RADIO 9 // (root + 7vap + vxd)

#define NEED_CREATE_TRUE 1
#define NEED_CREATE_FALSE 0

#define MAX_KEEP_ALIVE 3

extern uint16_t max_sta_num_per_bss;
//----------------------------Events-------------------------
struct AssociationEventData {
	uint8_t  bssid[6];
	uint8_t  mac_address[6];
	uint16_t status_code;
	uint8_t  ht_capabilities;
	uint8_t  vht_capabilities[6];
	uint8_t  he_capabilities[14];
};

struct AssociationEvent {
	struct AssociationEventData assoc_data;
};

struct DisassociationEventData {
	uint8_t  bssid[6];
	uint8_t  mac_address[6];
	uint16_t reason_code;
	uint32_t bytes_sent;
	uint32_t bytes_received;
	uint32_t packets_sent;
	uint32_t packets_received;
	uint32_t errors_sent;
	uint32_t errors_received;
	uint32_t retrans_count;
};

struct DisassociationEvent {
	struct DisassociationEventData disassoc_data;
};
//----------------------------------------------------------

struct BackhaulSTA {
	uint8_t mac_address[6];
};

struct UnassociatedSTA {
	uint8_t mac_address[6];
	uint8_t signal_strength;
};

struct NeighborBSS {
	uint8_t  bssid[6];
	char *   ssid;
	uint8_t  signal_strength;
	uint8_t  channel_bandwidth;
	uint8_t  channel_utilization;
	uint16_t station_count;
};

struct ChannelScan {
	uint8_t             channel;
	uint8_t             utilization;
	uint8_t             noise;
	uint8_t             number_of_neighbors;
	struct NeighborBSS *neighbor_list;
};

struct OpClassScan {
	uint8_t             operating_class;
	struct ChannelScan *channel_scan_list;
	uint8_t             number_of_channel_scans;
};

#define WINDOW_MAX 90
#define ENTRY_MAX  8
#define STA_QUERY_TIMER 3
#define BTM_DISASSOC_TIMER 5000
#define BCN_REQ_TIMER 6
#define READY_TIME 15
#define READY_BACKOFF_TIME (READY_TIME / 3)
#define RCPI_STEERING_THRESHOLD 100
struct SteeringOpportunitySTA {
	uint8_t  mac[6];
	uint8_t  ready;
	uint8_t  sta_query_timer;
	/* Beacon Report */
	uint8_t  bcn_req_timer;
	uint8_t  bcn_rpt_nr;
	struct BeaconMeasurementReportInfo bcn_rpt[ENTRY_MAX];
};

struct SteeringOpportunityBSS {
	uint16_t window;
	uint16_t btm_disassoc_timer;
 	uint8_t  sta_nr;
 	struct   SteeringOpportunitySTA sta[ENTRY_MAX];
};

struct SteeringRcpiControl {
	uint8_t ready;
	uint8_t sta_query_timer;
	/* Beacon Report */
	uint8_t bcn_req_timer;
	uint8_t bcn_rpt_nr;
	struct BeaconMeasurementReportInfo bcn_rpt[ENTRY_MAX];
};

struct STA {
	uint8_t                           mac_address[6];
	uint32_t                          last_data_downlink_rate;
	uint32_t                          last_data_uplink_rate;
	uint32_t                          est_mac_data_rate_downlink;
	uint32_t                          est_mac_data_rate_uplink;
	uint8_t                           signal_strength;
	uint32_t                          bytes_sent;
	uint32_t                          bytes_received;
	uint32_t                          packets_sent;
	uint32_t                          packets_received;
	uint32_t                          errors_sent;
	uint32_t                          errors_received;
	uint32_t                          retrans_count;
	uint32_t                          throughput; // in kbps
	struct BeaconMeasurementReportIE *measurement_report;
	uint8_t                           number_of_measure_reports;
	uint8_t                           still_present;
	uint8_t                           is_backhaul_sta;
	uint16_t                          curr_bss_score;
	uint8_t                           best_bss_mac[6];
	uint8_t                           best_bss_count;
	uint16_t                          best_bss_score;
	uint8_t                           status;
	uint8_t                           passive_roam_count;
	uint8_t                           misc_count;
	uint8_t                           capability_recv;
	uint8_t                           rm_cap[5];
	uint8_t                           sent_11k;
	uint8_t                           support_bss_transition;
	uint8_t                           mbo_cap;
	struct SteeringRcpiControl        rcpi;
};

struct StructBSS { //Change to this name due to naming conflict
	uint8_t                       bssid[6];
	char *                        ssid;
	struct STA **                 sta_list;
	uint16_t                      number_of_sta;
	uint8_t                       est_service_parameters_be[3];
	uint8_t                       est_service_parameters_bk[3];
	uint8_t                       est_service_parameters_vi[3];
	uint8_t                       est_service_parameters_vo[3];
	struct SteeringOpportunityBSS steering_opportunity;
};

struct CurrentOperatingClassProfile {
	uint8_t op_class;
	uint8_t channel; //assume typo in spec
	int8_t  tx_power;
};

struct CapableOperatingClassProfile {
	uint8_t  op_class;
	int8_t   max_tx_power;
	uint8_t *non_operable;
	uint8_t  number_of_non_oper_chan;
};

struct Radio {
	uint8_t                              id[6];
	uint8_t                              band;
	struct CurrentOperatingClassProfile *current_operating_classes;
	uint8_t                              number_of_curr_op_class;
	uint8_t                              utilization;
	uint16_t                             number_of_bss;
	struct StructBSS *                   bss_list[MAX_BSS_NUM_PER_RADIO];
	struct BackhaulSTA                   backhaul_sta; //for RTK implementation vxd mac = wlan root mac
	struct UnassociatedSTA *             unassociated_sta_list;
	struct SteeringPolicy                policies;
	uint16_t                             number_of_unassoc_sta;
};

struct DeviceNeighbor {
	uint8_t al_mac[6];
	// uint8_t is_expired;
};

struct Non1905Client {
	uint8_t mac[6];
};

struct Device {
	uint8_t                id[6];
	uint8_t                parent_id[6];
	uint8_t                keep_alive;
	uint8_t                multi_ap_capabilities;
	uint16_t               number_of_radios;
	struct Radio *         radio_list[MAX_RADIO_NUM_PER_DEVICE];
	uint8_t                policy_config;
	uint8_t                received_channel_report;
	time_t                 updated_timestamp;
	uint8_t                number_of_direct_neighbors;
	struct DeviceNeighbor *direct_neighbor_list;
	char *                 device_name;
	char *                 ip_addr;
	uint16_t               path_score;
	uint8_t                renew_ack;
	uint8_t                is_visited;
	uint8_t                need_vlan_configure;
	uint8_t                logical_distance_to_controller;
	uint8_t                physical_distance_to_controller;
	uint8_t                multi_ap_profile;
	uint8_t                non_1905_client_nr;
	struct Non1905Client * non_1905_clients;
	uint8_t                renew_send_count;
	char *                 manufacturer;
	char *                 model_name;
	uint8_t                local_disallowed_sta_nr;
	uint8_t                (*local_disallowed_sta_mac)[6];
	uint8_t                btm_disallowed_sta_nr;
	uint8_t                (*btm_disallowed_sta_mac)[6];
};

struct Network {
	char *         id;
	uint8_t        controller_id[6]; //change from string to uint8_t to directly store controller mac as controller ID
	uint16_t       number_of_devices;
	uint8_t        max_device_reached;
	struct Device *device_list[MAX_DEVICE_NUM_PER_NETWORK];
};

struct vlan_need_configure_check {
	uint8_t deleted[10];
	char   *ssid[10];
};

typedef enum {
	VLAN_CONFIGURE_NOT_READY = 0,
	VLAN_CONFIGURE_PENDING   = 1,
	VLAN_CONFIGURE_NO_NEED   = 2,
} NEED_VLAN_CONFIGURE_STATUS;

uint8_t rssi_to_rcpi(uint8_t rssi);

uint8_t rcpi_to_rssi(uint8_t rcpi);

uint8_t update_network(struct Network *network, uint8_t *controller_mac);

struct Device *locate_device(struct Network *network, uint8_t *src_mac, uint8_t need_create_flag, uint16_t *index);

struct Device *locate_device_by_al_mac(struct Network *network, uint8_t *al_mac);

uint8_t update_device(struct Device *device, struct APCapabilityTLV *ap_capability_tlv, struct vendorSpecificTLV *vendor_specific_tlv, struct MultiAPProfileTLV *multiap_profile_tlv);

uint16_t find_band_by_backhaul_sta_mac(struct Network *network, uint8_t *sta_mac);

void update_device_info(struct Device *device, struct deviceInformationTypeTLV *info, uint8_t radio_number, struct Network *network);

uint8_t update_device_neighbor(struct Network network, struct Device *device, struct neighborDeviceListTLV *neighbor_device_list_tlv, uint32_t custom_features);

uint8_t update_device_operational_bss(struct Device *device, struct ApOperationalBSSTLV *operational_bss);

uint8_t update_device_associated_clients(struct Network network, struct Device *device, struct AssociatedClientsTLV *associated_clients);

struct Radio *locate_radio(struct Device *device, uint8_t *radio_mac, uint8_t cur_op_class, uint8_t *bssid, uint8_t need_create_flag, uint16_t *index);

struct Radio *locate_backhaul_radio(struct Device *local_device, struct Device *neighbor_device, uint8_t *neighbor_bssid);
uint8_t       update_radio(struct Radio *                                radio,
                           struct OperatingChannelReportTLV *            operatingChannelReportTlv,
                           struct APMetricsTLV *                         apMetricsTlv,
                           struct UnassociatedSTALinkMetricsResponseTLV *unassociatedStaLinkMetricsResponseTlv,
                           uint8_t *                                     backhaul_sta_mac);

struct Device *   get_controller_device(struct Network *network);
struct StructBSS *locate_bss_by_ssid(struct Radio *radio, char *ssid);
struct StructBSS *locate_bss(struct Radio *radio, uint8_t *bssid, uint8_t need_create_flag, uint16_t *index);
uint8_t           update_bss(struct StructBSS *   bss,
                             char *               ssid,
                             uint8_t              ssid_len,
                             struct APMetricsTLV *apMetricsTlv);

struct STA *locate_sta(struct StructBSS *bss, uint8_t *sta_mac, uint8_t need_create_flag, uint16_t *index);
uint8_t     update_sta(struct STA *                         sta,
                       struct AssocSTALinkMetrics *         assocStaLinkMetrics,
                       struct AssociatedSTATrafficStatsTLV *associatedStaTrafficStatsTlv,
                       struct BeaconMetricsResponseTLV *    beaconMetricsResponseTlv);
struct STA *find_sta(struct Network *network, uint8_t *sta_mac);
struct STA *locate_sta_by_mac(struct Network *network, struct Device *curr_device, uint8_t *sta_mac, struct Device **device, struct Radio **radio, struct StructBSS **bss);

uint8_t free_sta(struct StructBSS *bss, uint8_t *sta_mac);
uint8_t free_bss(struct Radio *radio, uint8_t *bss_mac);
uint8_t free_radio(struct Device *device, uint8_t *radio_mac);
uint8_t free_device(struct Network *network, uint8_t *device_mac);

uint8_t tree_topology_json_export(struct Network *network, struct Device *curr_device, uint8_t *parent_mac, FILE *fp, uint8_t depth, uint8_t hw_reg_domain, uint8_t physical_distance, uint32_t custom_features);

uint8_t database_garbage_collection(struct Network *network, uint8_t device_al_mac[6], uint16_t threshold);

void reset_renew_ack(struct Network *network);

void reset_visited(struct Network *network);

void optimize_neighbor(struct Network *network);

void optimize_datamodel(struct Network *network);

struct Device *locate_device_by_bss_mac(struct Network *network, uint8_t *mac_addr);

unsigned char *find_bsta_mac_by_al_mac(struct Network *network, unsigned char *al_mac, int band);

unsigned char *find_al_mac_by_bss_mac(struct Network *network, unsigned char *mac_addr);

uint8_t remove_neighbor(struct Device *device, uint8_t *neighbor_mac);

uint8_t insert_neighbor(struct Device *device, uint8_t *neighbor_mac);

void update_need_vlan_configure(struct Network *network, struct Device *device);

uint8_t topology_db_update(struct Network *network, struct Device *curr_device, uint8_t *parent_mac, uint8_t depth, uint8_t physical_distance);

uint8_t obtain_5g_bss_list(struct Network *network, char *interface_name, int is_agent);

uint8_t need_sync_back_channel(struct Radio *radio, struct OperatingChannelReportTLV *operatingChannelReportTlv);
#endif
