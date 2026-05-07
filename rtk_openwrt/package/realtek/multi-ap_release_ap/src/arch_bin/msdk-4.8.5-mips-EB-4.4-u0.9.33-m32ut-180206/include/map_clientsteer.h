/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _MAP_CLIENTSTEER_H_
#define _MAP_CLIENTSTEER_H_

#include "map_utils.h"

#define STA_IDLE 0
#define STA_11V_ROAM 1
#define STA_DISASSOC_ROAM 2
#define STA_POST_ROAM 3

//Conduct 11k measurement if below threshold in kbps, for gaming protection (latency)
#define DOT11K_TRIGGER_THRESHOLD 3

//11k beacon request channel setting
#define DOT11K_BEACON_REQ_CH_ALL_BY_DOMAIN (0)
#define DOT11K_BEACON_REQ_CH_ALL_BY_CH_RPT (1)
#define DOT11K_BEACON_REQ_CH_ALL_BY_STA_CH (2)

#define CLIENT_STEER_ENABLED 1

//---------User configurable values--------------
extern uint8_t  roaming_enable;
extern uint8_t  roam_score_difference;
extern uint8_t  min_evaluation_interval;
extern uint8_t  min_roam_interval;
extern uint8_t  rssi_weightage;
extern uint8_t  path_weightage;
extern uint8_t  cu_weightage;
extern uint8_t  max_num_device_allowed;
extern uint32_t throughput_threshold;
//---------User configurable values--------------

void process_sta_roam(struct Device *device, uint8_t op_class, uint8_t channel, struct StructBSS *bss, struct STA *station);
void update_roam_info(struct Network *network, struct Radio *updated_radio, int path_score);
void update_11k_roam_info(struct Network *network, struct Device *device, struct BeaconMetricsResponseTLV *beaconMetricResponseTlv, struct StructBSS *bss, struct STA *station);
void populate_control_request(struct Network *network, struct StructBSS *bss, struct client_association_control_request **control_req, uint8_t *control_req_nr);
void populate_channel_data(struct Network *network, struct Device *device, struct StructBSS *bss, struct unassoc_sta_channel *channel_data, uint8_t radio_index);
void pre_steer_process(struct Network *network, uint8_t i);
void client_steer_process(struct Network *network, uint8_t beacon_req_ch);
int manual_roam_check(struct Network *network);
void reset_steer_state(struct Network *network);
void steering_opportunity_init(struct Device *dev, struct StructBSS *bss, uint8_t *p);
void steering_opportunity_collect_beacon_report(struct Network *network, struct StructBSS *bss, uint8_t *p);
void steering_opportunity_watchdog(struct Device *dev);
void steering_rcpi_watchdog(struct Device *dev);
void steering_rcpi_init(struct Device *dev, uint8_t *p);
void steering_rcpi_collect_beacon_report(struct Network *network, struct StructBSS *bss, uint8_t *p);
#endif
