/*
 * Copyright (C) 2018 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <stdint.h>
#include <stdlib.h>

#include "map_utils.h"
#include "1905_tlvs.h"
#include "packet_tools.h"

map_response_element *_parse_response_element(INT8U *message)
{
	INT8U *               parsed;
	int                   tlvs_nr = 0;
	map_response_element *element = (map_response_element *)PLATFORM_MALLOC(sizeof(map_response_element) * 1);
	element->list_of_tlvs         = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 1);
	element->list_of_tlvs[0]      = NULL;

	_E1B(&message, &element->element_id);
	PLATFORM_PRINTF_TRACE("%s Element ID %02x\n", __FUNCTION__, element->element_id);

	_EnB(&message, element->src_mac, 6);
	PLATFORM_PRINTF_TRACE("Element SRC %02x:%02x:%02x:%02x:%02x:%02x\n", element->src_mac[0], element->src_mac[1], element->src_mac[2], element->src_mac[3], element->src_mac[4], element->src_mac[5]);

	while (1) {
		parsed = parse_1905_TLV_from_packet(message);
		if (NULL == parsed) {
			INT8U *p2 = message;
			INT16U len;
			INT8U  aux;

			_E1B(&p2, &aux);
			_E2B(&p2, &len);

			PLATFORM_PRINTF_WARNING("Parsing error. TLV %02x with length %d: \n", aux, len);
			if (len > 200) {
				len = 200;
			}

			char  aux1[200];
			char  aux2[10];
			INT8U first_time;
			INT8U j;

			aux1[0]    = 0x0;
			aux2[0]    = 0x0;
			first_time = 1;
			for (j = 0; j < len + 3; j++) {
				PLATFORM_SNPRINTF(aux2, 6, "0x%02x ", message[j]);
				PLATFORM_STRNCAT(aux1, aux2, 200 - PLATFORM_STRLEN(aux1) - 1);

				if (0 != j && 0 == (j + 1) % 8) {
					if (1 == first_time) {
						PLATFORM_PRINTF_INFO("[PLATFORM]   - Payload        = %s\n", aux1);
						first_time = 0;
					} else {
						PLATFORM_PRINTF_INFO("[PLATFORM]                      %s\n", aux1);
					}
					aux1[0] = 0x0;
				}
			}
			break;
		}

		if (TLV_TYPE_END_OF_MESSAGE == *parsed) {
			// No more TLVs
			//
			free_1905_TLV_structure(parsed);
			break;
		}

		// Advance 'p' to the next TLV.
		//
		INT8U  tlv_type;
		INT16U tlv_len;

		_E1B(&message, &tlv_type);
		_E2B(&message, &tlv_len);

		message += tlv_len;

		// Add this new TLV to the list (the list needs to be re-allocated
		// with more space first)
		//
		tlvs_nr++;
		element->list_of_tlvs              = (INT8U **)PLATFORM_REALLOC(element->list_of_tlvs, sizeof(INT8U *) * (tlvs_nr + 1));
		element->list_of_tlvs[tlvs_nr - 1] = parsed;
		element->list_of_tlvs[tlvs_nr]     = NULL;
	}
	element->tlv_nr = tlvs_nr;
	return element;
}

uint8_t *map_parse_element(uint8_t *element)
{
	uint8_t *ret = NULL;
	INT8U    element_id;
	element_id = *element;
	switch (element_id) {
	case MAP_GENERAL_ACKNOWLEDGEMENT_ELEMENT: {
		struct map_general_acknowledgement *req = (struct map_general_acknowledgement *)PLATFORM_MALLOC(sizeof(struct map_general_acknowledgement) * 1);
		_E1B(&element, &req->element_id);
		_E1B(&element, &req->request_type);
		_EnB(&element, req->dest_mac, 6);
		_E1B(&element, &req->status);
		ret = (uint8_t *)req;
		break;
	}
	case MAP_TOPOLOGY_QUERY_REQUEST_ELEMENT: {
		struct map_topology_query_request *req = (struct map_topology_query_request *)PLATFORM_MALLOC(sizeof(struct map_topology_query_request) * 1);
		_E1B(&element, &req->element_id);
		_EnB(&element, req->target_al_mac, 6);
		ret = (uint8_t *)req;
		break;
	}
	case MAP_AUTOCONFIG_SEARCH_REQUEST_ELEMENT: {
		struct map_autoconfig_search_request *req = (struct map_autoconfig_search_request *)PLATFORM_MALLOC(sizeof(struct map_autoconfig_search_request) * 1);
		_E1B(&element, &req->element_id);
		ret = (uint8_t *)req;
		break;
	}
	case MAP_AUTOCONFIG_RENEW_REQUEST_ELEMENT: {
		struct map_autoconfig_renew_request *req = (struct map_autoconfig_renew_request *)PLATFORM_MALLOC(sizeof(struct map_autoconfig_renew_request) * 1);
		_E1B(&element, &req->element_id);
		_E1B(&element, &req->include_self);
		_EnB(&element, req->target_al_mac, 6);
		ret = (uint8_t *)req;
		break;
	}
	case MAP_BACKHAUL_STEERING_REQUEST_ELEMENT: {
		struct map_backhaul_steering_request *req = (struct map_backhaul_steering_request *)PLATFORM_MALLOC(sizeof(struct map_backhaul_steering_request) * 1);
		_E1B(&element, &req->element_id);
		_EnB(&element, &req->target_al_mac, 6);
		_EnB(&element, &req->backhaul_sta_mac, 6);
		_EnB(&element, &req->target_bssid, 6);
		_E1B(&element, &req->op_class);
		_E1B(&element, &req->channel);
		ret = (uint8_t *)req;
		break;
	}
	case MAP_LINK_METRIC_QUERY_ELEMENT: {
		struct map_link_metric_query *req = (struct map_link_metric_query *)PLATFORM_MALLOC(sizeof(struct map_link_metric_query) * 1);
		_E1B(&element, &req->element_id);
		_EnB(&element, &req->target_al_mac, 6);
		_E1B(&element, &req->destination);
		_E1B(&element, &req->type);
		_EnB(&element, &req->sta_mac, 6);

		ret = (uint8_t *)req;
		break;
	}
	case MAP_POLICY_CONFIG_REQUEST_ELEMENT: {
		struct map_policy_config_request *req = (struct map_policy_config_request *)PLATFORM_MALLOC(sizeof(struct map_policy_config_request) * 1);
		INT8U                             i;

		_E1B(&element, &req->element_id);
		_EnB(&element, &req->target_al_mac, 6);
		_E1B(&element, &req->steer_policy_inclusion);
		if (req->steer_policy_inclusion) {
			_E1B(&element, &req->steer_policy_data.ls_sta_nr);
			if (req->steer_policy_data.ls_sta_nr > 0) {
				req->steer_policy_data.ls_stas = PLATFORM_MALLOC(req->steer_policy_data.ls_sta_nr * 6);
				_EnB(&element, req->steer_policy_data.ls_stas, (6 * req->steer_policy_data.ls_sta_nr));
			}
			_E1B(&element, &req->steer_policy_data.btm_sta_nr);
			if (req->steer_policy_data.btm_sta_nr > 0) {
				req->steer_policy_data.btm_stas = PLATFORM_MALLOC(req->steer_policy_data.btm_sta_nr * 6);
				_EnB(&element, req->steer_policy_data.btm_stas, (6 * req->steer_policy_data.btm_sta_nr));
			}
			_E1B(&element, &req->steer_policy_data.radio_nr);
			if (req->steer_policy_data.radio_nr > 0) {
				req->steer_policy_data.radio_steer_policy_data = PLATFORM_MALLOC(sizeof(struct radio_steer_config) * req->steer_policy_data.radio_nr);
				for (i = 0; i < req->steer_policy_data.radio_nr; i++) {
					_EnB(&element, req->steer_policy_data.radio_steer_policy_data[i].radio_mac, 6);
					_E1B(&element, &req->steer_policy_data.radio_steer_policy_data[i].policy);
					_E1B(&element, &req->steer_policy_data.radio_steer_policy_data[i].cu_threshold);
					_E1B(&element, &req->steer_policy_data.radio_steer_policy_data[i].rcpi_threshold);
				}
			}
		}
		_E1B(&element, &req->metric_policy_inclusion);
		if (req->metric_policy_inclusion) {
			_E1B(&element, &req->metric_policy_data.report_interval);
			_E1B(&element, &req->metric_policy_data.radio_nr);
			if (req->metric_policy_data.radio_nr) {
				req->metric_policy_data.radio_metric_policy_data = PLATFORM_MALLOC(sizeof(struct radio_metric_config) * req->metric_policy_data.radio_nr);
				for (i = 0; i < req->metric_policy_data.radio_nr; i++) {
					_EnB(&element, req->metric_policy_data.radio_metric_policy_data[i].radio_mac, 6);
					_E1B(&element, &req->metric_policy_data.radio_metric_policy_data[i].rcpi_threshold);
					_E1B(&element, &req->metric_policy_data.radio_metric_policy_data[i].rcpi_hysteresis);
					_E1B(&element, &req->metric_policy_data.radio_metric_policy_data[i].cu_threshold);
					_E1B(&element, &req->metric_policy_data.radio_metric_policy_data[i].inclusion_policy);
				}
			}
		}

		_E1B(&element, &req->default8021q_inclusion);
		if (req->default8021q_inclusion) {
			_E2B(&element, &req->default8021q_data.primary_vlan_id);
			_E1B(&element, &req->default8021q_data.default_pcp);
		}

		_E1B(&element, &req->traffic_separation_inclusion);
		if (req->traffic_separation_inclusion) {
			_E1B(&element, &req->traffic_separation_data.ssid_nr);
			if (req->traffic_separation_data.ssid_nr) {
				req->traffic_separation_data.ssid_info = PLATFORM_MALLOC(sizeof(struct traffic_separation_ssidInfo) * req->traffic_separation_data.ssid_nr);
				for (i = 0; i < req->traffic_separation_data.ssid_nr; i++) {
					_E1B(&element, &req->traffic_separation_data.ssid_info[i].ssid_length);
					req->traffic_separation_data.ssid_info[i].ssid = PLATFORM_MALLOC(req->traffic_separation_data.ssid_info[i].ssid_length);
					_EnB(&element, req->traffic_separation_data.ssid_info[i].ssid, req->traffic_separation_data.ssid_info[i].ssid_length);
					_E2B(&element, &req->traffic_separation_data.ssid_info[i].vlan_id);
				}
			}
		}

		ret = (uint8_t *)req;
		break;
	}
	case MAP_AP_CAPABILITY_QUERY_ELEMENT: {
		struct map_ap_capability_query *req = (struct map_ap_capability_query *)PLATFORM_MALLOC(sizeof(struct map_ap_capability_query) * 1);
		_E1B(&element, &req->element_id);
		_EnB(&element, req->target_al_mac, 6);
		ret = (uint8_t *)req;
		break;
	}
	case MAP_CLIENT_CAPABILITY_QUERY_ELEMENT: {
		struct map_client_capability_query *req = (struct map_client_capability_query *)PLATFORM_MALLOC(sizeof(struct map_client_capability_query) * 1);
		_E1B(&element, &req->element_id);
		_EnB(&element, req->target_al_mac, 6);
		_EnB(&element, req->bssid, 6);
		_EnB(&element, req->sta_mac, 6);
		ret = (uint8_t *)req;
		break;
	}
	case MAP_AP_METRIC_QUERY_ELEMENT: {
		struct map_ap_metric_query *req = (struct map_ap_metric_query *)PLATFORM_MALLOC(sizeof(struct map_ap_metric_query) * 1);
		_E1B(&element, &req->element_id);
		_EnB(&element, req->target_al_mac, 6);
		_E1B(&element, &req->bssid_nr);
		if (req->bssid_nr > 0) {
			req->bssid = PLATFORM_MALLOC(6 * req->bssid_nr);
			_EnB(&element, req->bssid, 6 * req->bssid_nr);
		}
		ret = (uint8_t *)req;
		break;
	}
	case MAP_ASSOC_STA_METRIC_QUERY_ELEMENT: {
		struct map_assoc_sta_metric_query *req = (struct map_assoc_sta_metric_query *)PLATFORM_MALLOC(sizeof(struct map_assoc_sta_metric_query) * 1);
		_E1B(&element, &req->element_id);
		_EnB(&element, req->target_al_mac, 6);
		_EnB(&element, req->sta_mac, 6);
		ret = (uint8_t *)req;
		break;
	}
	case MAP_UNASSOC_STA_METRIC_QUERY_ELEMENT: {
		struct map_unassoc_sta_metric_query *req = (struct map_unassoc_sta_metric_query *)PLATFORM_MALLOC(sizeof(struct map_unassoc_sta_metric_query) * 1);
		INT8U                                i;

		_E1B(&element, &req->element_id);
		_EnB(&element, req->target_al_mac, 6);
		_E1B(&element, &req->op_class);
		_E1B(&element, &req->channel_data_nr);

		if (req->channel_data_nr > 0) {
			req->channel_data = PLATFORM_MALLOC(sizeof(struct unassoc_sta_channel) * req->channel_data_nr);
			for (i = 0; i < req->channel_data_nr; i++) {
				_E1B(&element, &req->channel_data[i].channel_nr);
				_E1B(&element, &req->channel_data[i].sta_nr);
				if (req->channel_data[i].sta_nr > 0) {
					req->channel_data[i].stas = PLATFORM_MALLOC(6 * req->channel_data[i].sta_nr);
					_EnB(&element, req->channel_data[i].stas, (6 * req->channel_data[i].sta_nr));
				}
			}
		}
		ret = (uint8_t *)req;
		break;
	}
	case MAP_BEACON_METRIC_QUERY_ELEMENT: {
		struct map_beacon_metric_query *req = (struct map_beacon_metric_query *)PLATFORM_MALLOC(sizeof(struct map_beacon_metric_query) * 1);
		INT8U                           i;

		_E1B(&element, &req->element_id);
		_EnB(&element, req->target_al_mac, 6);

		_EnB(&element, req->beacon_metric_query.sta_mac, 6);
		_E1B(&element, &req->beacon_metric_query.op_class);
		_E1B(&element, &req->beacon_metric_query.channel_nr);
		_EnB(&element, req->beacon_metric_query.bssid, 6);
		_E1B(&element, &req->beacon_metric_query.report_detail);
		_E1B(&element, &req->beacon_metric_query.ssid_len);
		if (req->beacon_metric_query.ssid_len > 0) {
			req->beacon_metric_query.ssid = PLATFORM_MALLOC(req->beacon_metric_query.ssid_len);
			_EnB(&element, req->beacon_metric_query.ssid, req->beacon_metric_query.ssid_len);
		}
		_E1B(&element, &req->beacon_metric_query.channel_report_nr);
		if (req->beacon_metric_query.channel_report_nr > 0) {
			req->beacon_metric_query.channel_report = PLATFORM_MALLOC(sizeof(struct unassoc_sta_channel) * req->beacon_metric_query.channel_report_nr);
			for (i = 0; i < req->beacon_metric_query.channel_report_nr; i++) {
				_E1B(&element, &req->beacon_metric_query.channel_report[i].len);
				_E1B(&element, &req->beacon_metric_query.channel_report[i].op_class);
				if ((req->beacon_metric_query.channel_report[i].len - 1) > 0) {
					req->beacon_metric_query.channel_report[i].channel_list = PLATFORM_MALLOC((req->beacon_metric_query.channel_report[i].len - 1));
					_EnB(&element, req->beacon_metric_query.channel_report[i].channel_list, (req->beacon_metric_query.channel_report[i].len - 1));
				}
			}
		}
		_E1B(&element, &req->beacon_metric_query.element_nr);
		if (req->beacon_metric_query.element_nr > 0) {
			req->beacon_metric_query.element_list = PLATFORM_MALLOC(req->beacon_metric_query.element_nr);
			_EnB(&element, req->beacon_metric_query.element_list, req->beacon_metric_query.element_nr);
		}

		ret = (uint8_t *)req;
		break;
	}
	case MAP_CHANNEL_PREFERENCE_QUERY_ELEMENT: {
		struct map_channel_preference_query *req = (struct map_channel_preference_query *)PLATFORM_MALLOC(sizeof(struct map_channel_preference_query));

		_E1B(&element, &req->element_id);
		_EnB(&element, req->target_al_mac, 6);
		ret = (uint8_t *)req;
		break;
	}
	case MAP_CHANNEL_SELECTION_REQUEST_ELEMENT: {
		struct map_channel_selection_request *req = (struct map_channel_selection_request *)PLATFORM_ZALLOC(sizeof(struct map_channel_selection_request));
		INT8U                                 i, j;

		_E1B(&element, &req->element_id);
		_EnB(&element, req->target_al_mac, 6);
		_E1B(&element, &req->ch_preference_nr);
		req->ch_preference = (struct channel_preference *)PLATFORM_ZALLOC(sizeof(struct channel_preference) * req->ch_preference_nr);
		for (i = 0; i < req->ch_preference_nr; i++) {
			_EnB(&element, req->ch_preference[i].radio_mac, 6);
			_E1B(&element, &req->ch_preference[i].op_class_nr);
			req->ch_preference[i].ch_pre_op_class = (struct channel_preference_op_class *)PLATFORM_ZALLOC(sizeof(struct channel_preference_op_class) * req->ch_preference[i].op_class_nr);
			for (j = 0; j < req->ch_preference[i].op_class_nr; j++) {
				_E1B(&element, &req->ch_preference[i].ch_pre_op_class[j].op_class);
				_E1B(&element, &req->ch_preference[i].ch_pre_op_class[j].channel_nr);
				if (req->ch_preference[i].ch_pre_op_class[j].channel_nr != 0) {
					req->ch_preference[i].ch_pre_op_class[j].channel_list = (INT8U *)PLATFORM_ZALLOC(sizeof(INT8U) * req->ch_preference[i].ch_pre_op_class[j].channel_nr);
					_EnB(&element, req->ch_preference[i].ch_pre_op_class[j].channel_list, req->ch_preference[i].ch_pre_op_class[j].channel_nr);
				}
				_E1B(&element, &req->ch_preference[i].ch_pre_op_class[j].preference_reason_code);
			}
		}
		_E1B(&element, &req->tx_pwr_limit_nr);
		req->tx_pwr_limit = (struct transmit_power_limit *)PLATFORM_MALLOC(sizeof(struct transmit_power_limit) * req->tx_pwr_limit_nr);
		for (i = 0; i < req->tx_pwr_limit_nr; i++) {
			_EnB(&element, req->tx_pwr_limit[i].radio_mac, 6);
			_E1B(&element, &req->tx_pwr_limit[i].tx_pwr_limit);
		}
		ret = (uint8_t *)req;
		break;
	}
#ifdef RTK_MULTI_AP_R2
	case MAP_CAC_REQUEST_ELEMENT: {
		struct map_cac_request *req = (struct map_cac_request *)PLATFORM_MALLOC(sizeof(struct map_cac_request));
		INT8U  					i;

		_E1B(&element, &req->element_id);
		_EnB(&element, req->target_al_mac, 6);
		_E1B(&element, &req->cac_req.radio_nr);
		req->cac_req.radios = (struct cac_request_radio *)PLATFORM_MALLOC(sizeof(struct cac_request_radio) * req->cac_req.radio_nr);
		for (i = 0; i < req->cac_req.radio_nr; i++) {
			_EnB(&element, req->cac_req.radios[i].radio_id, 6);
			_E1B(&element, &req->cac_req.radios[i].op_class);
			_E1B(&element, &req->cac_req.radios[i].channel);
			_E1B(&element, &req->cac_req.radios[i].flags);
		}
		ret = (uint8_t *)req;
		break;
	}
	case MAP_CHANNEL_SCAN_REQUEST_ELEMENT: {
		struct map_channel_scan_request *req = (struct map_channel_scan_request *)PLATFORM_MALLOC(sizeof(struct map_channel_scan_request));
		INT8U                               i, j;
		
		_E1B(&element, &req->element_id);
		_EnB(&element, req->target_al_mac, 6);
		_E1B(&element, &req->channel_scan_req.flags);
		_E1B(&element, &req->channel_scan_req.target_radio_nr);
		req->channel_scan_req.target_radios = (struct channel_scan_target_radio *)PLATFORM_MALLOC(sizeof(struct channel_scan_target_radio) * req->channel_scan_req.target_radio_nr);
		for (i = 0; i < req->channel_scan_req.target_radio_nr; i++) {
			_EnB(&element, req->channel_scan_req.target_radios[i].radio_id, 6);
			_E1B(&element, &req->channel_scan_req.target_radios[i].target_op_class_nr);
			req->channel_scan_req.target_radios[i].target_op_classes = (struct channel_scan_target_op_class *)PLATFORM_MALLOC(sizeof(struct channel_scan_target_op_class) * req->channel_scan_req.target_radios[i].target_op_class_nr);
			for (j = 0; j < req->channel_scan_req.target_radios[i].target_op_class_nr; j++) {
				_E1B(&element, &req->channel_scan_req.target_radios[i].target_op_classes[j].op_class);
				_E1B(&element, &req->channel_scan_req.target_radios[i].target_op_classes[j].target_channel_nr);
				if (req->channel_scan_req.target_radios[i].target_op_classes[j].target_channel_nr) {
					req->channel_scan_req.target_radios[i].target_op_classes[j].target_channel_list = (INT8U *)PLATFORM_MALLOC(req->channel_scan_req.target_radios[i].target_op_classes[j].target_channel_nr * sizeof(INT8U));
					_EnB(&element, req->channel_scan_req.target_radios[i].target_op_classes[j].target_channel_list, req->channel_scan_req.target_radios[i].target_op_classes[j].target_channel_nr);
				}
			}
		}
		ret = (uint8_t *)req;
		break;		
	}
#endif
	case MAP_CLIENT_STEERING_REQUEST_ELEMENT: {
		struct map_client_steering_request *req = (struct map_client_steering_request *)PLATFORM_MALLOC(sizeof(struct map_client_steering_request));
		INT8U                               i;

		_E1B(&element, &req->element_id);
		_EnB(&element, req->target_al_mac, 6);
		_EnB(&element, req->steering_req.bssid, 6);
		_E1B(&element, &req->steering_req.mode);
		_E2B(&element, &req->steering_req.window);
		_E2B(&element, &req->steering_req.disassoc_timer);
		_E1B(&element, &req->steering_req.sta_nr);
		req->steering_req.sta_mac_address = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(INT8U[6]) * req->steering_req.sta_nr);
		for (i = 0; i < req->steering_req.sta_nr; i++) {
			_EnB(&element, req->steering_req.sta_mac_address[i], 6);
		}
		_E1B(&element, &req->steering_req.mbo_cap);
		_E1B(&element, &req->steering_req.target_bss_nr);
		req->steering_req.target_bss = (struct target_bss *)PLATFORM_MALLOC(sizeof(struct target_bss) * req->steering_req.target_bss_nr);
		for (i = 0; i < req->steering_req.target_bss_nr; i++) {
			_EnB(&element, req->steering_req.target_bss[i].bssid, 6);
			_E1B(&element, &req->steering_req.target_bss[i].op_class);
			_E1B(&element, &req->steering_req.target_bss[i].channel);
			_E1B(&element, &req->steering_req.target_bss[i].reason_code);
		}
		ret = (uint8_t *)req;
		break;
	}
	case MAP_CLIENT_STEERING_REQUEST_OPPORTUNITY_ELEMENT: {
		struct map_client_steering_request *req = (struct map_client_steering_request *)PLATFORM_MALLOC(sizeof(struct map_client_steering_request));
		INT8U                               i;
		INT8U                               tlv_type;
		INT16U                              len;

		_E1B(&element, &req->element_id);
		_EnB(&element, req->target_al_mac, 6); /* src mac */
		_E1B(&element, &tlv_type);
		_E2B(&element, &len);
		_EnB(&element, req->steering_req.bssid, 6);
		_E1B(&element, &req->steering_req.mode);
		_E2B(&element, &req->steering_req.window);
		_E2B(&element, &req->steering_req.disassoc_timer);
		_E1B(&element, &req->steering_req.sta_nr);
		req->steering_req.sta_mac_address = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(INT8U[6]) * req->steering_req.sta_nr);
		for (i = 0; i < req->steering_req.sta_nr; i++) {
			_EnB(&element, req->steering_req.sta_mac_address[i], 6);
		}
		_E1B(&element, &req->steering_req.target_bss_nr);
		req->steering_req.target_bss = (struct target_bss *)PLATFORM_MALLOC(sizeof(struct target_bss) * req->steering_req.target_bss_nr);
		for (i = 0; i < req->steering_req.target_bss_nr; i++) {
			_EnB(&element, req->steering_req.target_bss[i].bssid, 6);
			_E1B(&element, &req->steering_req.target_bss[i].op_class);
			_E1B(&element, &req->steering_req.target_bss[i].channel);
		}
		ret = (uint8_t *)req;
		break;
	}
	case MAP_CLIENT_ASSOCIATION_CONTROL_REQUEST_ELEMENT: {
		struct map_client_association_control_request *req = (struct map_client_association_control_request *)PLATFORM_MALLOC(sizeof(struct map_client_association_control_request));
		INT8U                                          i, j;

		_E1B(&element, &req->element_id);
		_EnB(&element, req->target_al_mac, 6);
		_E1B(&element, &req->client_assoc_ctrl_req_nr);
		req->client_assoc_ctrl_req = (struct client_association_control_request *)PLATFORM_MALLOC(sizeof(struct client_association_control_request) * req->client_assoc_ctrl_req_nr);
		for (i = 0; i < req->client_assoc_ctrl_req_nr; i++) {
			_EnB(&element, req->client_assoc_ctrl_req[i].bssid, 6);
			_E1B(&element, &req->client_assoc_ctrl_req[i].assoc_ctrl);
			_E2B(&element, &req->client_assoc_ctrl_req[i].validity_period);
			_E1B(&element, &req->client_assoc_ctrl_req[i].sta_nr);
			req->client_assoc_ctrl_req[i].sta_mac_address = (INT8U(*)[6])PLATFORM_MALLOC(sizeof(INT8U[6]) * req->client_assoc_ctrl_req[i].sta_nr);
			for (j = 0; j < req->client_assoc_ctrl_req[i].sta_nr; j++) {
				_EnB(&element, req->client_assoc_ctrl_req[i].sta_mac_address[j], 6);
			}
		}
		ret = (uint8_t *)req;
		break;
	}
	case MAP_CONFIGURE_NOTIFICATION_ELEMENT: {
		struct map_configure_notification *req = (struct map_configure_notification *)PLATFORM_MALLOC(sizeof(struct map_configure_notification));
		INT8U                              i;
		INT8U                              j;
		INT8U                              len;
		INT16U                             aux16;

		_E1B(&element, &req->element_id);

		_E1B(&element, &req->config_nr);

		req->config_data = (struct radio_config_data *)PLATFORM_MALLOC(sizeof(struct radio_config_data) * req->config_nr);
		for (i = 0; i < req->config_nr; i++) {
			_E1B(&element, &req->config_data[i].radio_band);
			_E1B(&element, &req->config_data[i].bss_data_nr);

			if (req->config_data[i].bss_data_nr) {
				req->config_data[i].bss_data = (struct bss_config_data *)PLATFORM_ZALLOC(sizeof(struct bss_config_data) * req->config_data[i].bss_data_nr);
				for (j = 0; j < req->config_data[i].bss_data_nr; j++) {
					_E1B(&element, &len);
					req->config_data[i].bss_data[j].ssid = (char *)PLATFORM_MALLOC(sizeof(char) * (len + 1));
					_EnB(&element, req->config_data[i].bss_data[j].ssid, len);
					req->config_data[i].bss_data[j].ssid[len] = 0x00;
					_E1B(&element, &len);
					req->config_data[i].bss_data[j].network_key = (char *)PLATFORM_MALLOC(sizeof(char) * (len + 1));
					_EnB(&element, req->config_data[i].bss_data[j].network_key, len);
					req->config_data[i].bss_data[j].network_key[len] = 0x00;
					_E1B(&element, &req->config_data[i].bss_data[j].authentication_type);
					_E1B(&element, &req->config_data[i].bss_data[j].encryption_type);
					_E1B(&element, &req->config_data[i].bss_data[j].network_type);
					_E2B(&element, &req->config_data[i].bss_data[j].vid);
					_E2B(&element, &aux16);
					if (aux16) {
						req->config_data[i].bss_data[j].signed_connector = (char *)PLATFORM_ZALLOC(sizeof(char) * (aux16 + 1));
						_EnB(&element, req->config_data[i].bss_data[j].signed_connector, aux16);
					}
				}
			}
			_E1B(&element, &req->config_data[i].vendor_data_nr);
			req->config_data[i].vendor_datas = NULL;
			if (req->config_data[i].vendor_data_nr) {
				req->config_data[i].vendor_datas = (struct vendor_specific_data *)PLATFORM_MALLOC(sizeof(struct vendor_specific_data) * req->config_data[i].vendor_data_nr);
			}
			for (j = 0; j < req->config_data[i].vendor_data_nr; j++) {
				_EnB(&element, req->config_data[i].vendor_datas[j].vendor_oui, 3);
				_E2B(&element, &req->config_data[i].vendor_datas[j].payload_len);
				req->config_data[i].vendor_datas[j].payload = (unsigned char *)PLATFORM_MALLOC(sizeof(unsigned char) * (req->config_data[i].vendor_datas[j].payload_len));
				_EnB(&element, req->config_data[i].vendor_datas[j].payload, req->config_data[i].vendor_datas[j].payload_len);
			}
		}
		ret = (uint8_t *)req;
		break;
	}
	case MAP_OPERATING_CHANNEL_CHANGE_NOTIFICATION_ELEMENT: {
		struct map_operating_channel_change_notification *req = (struct map_operating_channel_change_notification *)PLATFORM_MALLOC(sizeof(struct map_operating_channel_change_notification) * 1);
		_E1B(&element, &req->element_id);
		ret = (uint8_t *)req;
		break;
	}
	case MAP_VENDOR_SPECIFIC_ELEMENT: {
		struct map_vendor_specific_message *req = (struct map_vendor_specific_message *)PLATFORM_MALLOC(sizeof(struct map_vendor_specific_message));

		_E1B(&element, &req->element_id);
		_EnB(&element, req->target_al_mac, 6);
		_E1B(&element, &req->relay_indicator);
		_EnB(&element, req->vendor_oui, 3);
		_E2B(&element, &req->vendor_data_len);
		req->vendor_data = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * req->vendor_data_len);
		_EnB(&element, req->vendor_data, req->vendor_data_len);

		ret = (uint8_t *)req;
		break;
	}
	case MAP_CONTROLLER_CHANGE_NOTIFICATION_ELEMENT: {
		struct map_controller_change_element *req = (struct map_controller_change_element *)PLATFORM_MALLOC(sizeof(struct map_controller_change_element));
		_E1B(&element, &req->element_id);
		_E1B(&element, &req->event_type);
		_EnB(&element, req->controller_al_mac, 6);
		INT8U interface_name_len = 0;
		_E1B(&element, &interface_name_len);
		if (0 == interface_name_len) {
			req->receiving_interface = NULL;
		} else {
			req->receiving_interface = (char *)PLATFORM_MALLOC(sizeof(char) * (interface_name_len + 1));
			_EnB(&element, req->receiving_interface, interface_name_len);
			req->receiving_interface[interface_name_len] = '\0';
		}
		ret = (uint8_t *)req;
		break;
	}
	case MAP_CUSTOMIZED_STATUS_NOTIFICATION_ELEMENT: {
		struct map_customized_status_notification *req = (struct map_customized_status_notification *)PLATFORM_MALLOC(sizeof(struct map_customized_status_notification));
		_E1B(&element, &req->element_id);
		_E1B(&element, &req->customized_status);
		ret = (uint8_t *)req;
		break;
	}
	case MAP_DPP_CONFIGURATION_ELEMENT: {
		struct map_dpp_configuration *req = (struct map_dpp_configuration *)PLATFORM_ZALLOC(sizeof(struct map_dpp_configuration));
		uint16_t aux16;
		_E1B(&element, &req->element_id);

		_E2B(&element, &aux16);
		if (aux16) {
			req->signed_connector_1905 = PLATFORM_ZALLOC(aux16 + 1);
			_EnB(&element, req->signed_connector_1905, aux16);
			// PLATFORM_PRINTF_DEBUG("signed_connector_1905: %s\n", req->signed_connector_1905);
		}

		_E2B(&element, &aux16);
		if (aux16) {
			req->netaccess_key = PLATFORM_ZALLOC(aux16 + 1);
			_EnB(&element, req->netaccess_key, aux16);
			// PLATFORM_PRINTF_DEBUG("netaccess_key: %s\n", req->netaccess_key);
		}

		_E2B(&element, &aux16);
		if (aux16) {
			req->csign_key = PLATFORM_ZALLOC(aux16 + 1);
			_EnB(&element, req->csign_key, aux16);
			// PLATFORM_PRINTF_DEBUG("csign_key: %s\n", req->csign_key);
		}

		_E2B(&element, &aux16);
		if (aux16) {
			req->pp_key = PLATFORM_ZALLOC(aux16 + 1);
			_EnB(&element, req->pp_key, aux16);
			// PLATFORM_PRINTF_DEBUG("pp_key: %s\n", req->pp_key);
		}
		ret = (uint8_t *)req;
		break;
	}
	default: {
		// Pure TLV responses
		if (element_id > 0x50) {
			if(element_id > MAP_VLAN_CLEAR_NOTIFICATION_ELEMENT) {
				PLATFORM_PRINTF_ERROR("Unknown element_id %d!!!\n", element_id);
				return ret;
			}
			ret = (uint8_t *)_parse_response_element(element);
		}
		break;
	}
	}
	return ret;
}

map_message *map_parse_message(uint8_t *message)
{
	INT8U        val;
	map_message *ret;
	INT8U        element_id;

	INT16U message_len;

	ret = (map_message *)PLATFORM_MALLOC(sizeof(map_message) * 1);
	_E1B(&message, &val);
	ret->message_type = val;
	PLATFORM_PRINTF_DETAIL("Message ID %02x\n", ret->message_type);

	_E2B(&message, &message_len);
	PLATFORM_PRINTF_DETAIL("Message length %d\n", message_len);
	ret->message_len = message_len;
	element_id       = *message;

	PLATFORM_PRINTF_DETAIL("Element ID %02x\n", element_id);

	ret->element = map_parse_element(message);

	return ret;
}

uint8_t map_free_element(uint8_t *element)
{
	INT8U element_id;
	if (NULL == element) {
		return 0;
	}
	element_id = *element;
	switch (element_id) {
	case MAP_GENERAL_ACKNOWLEDGEMENT_ELEMENT:
	case MAP_AUTOCONFIG_SEARCH_REQUEST_ELEMENT:
	case MAP_AUTOCONFIG_RENEW_REQUEST_ELEMENT:
	case MAP_CUSTOMIZED_STATUS_NOTIFICATION_ELEMENT:
	case MAP_OPERATING_CHANNEL_CHANGE_NOTIFICATION_ELEMENT: {
		break;
	}
	case MAP_POLICY_CONFIG_REQUEST_ELEMENT: {
		struct map_policy_config_request *pt = (struct map_policy_config_request *)element;

		if (pt->steer_policy_inclusion) {
			if (pt->steer_policy_data.ls_sta_nr > 0)
				PLATFORM_FREE(pt->steer_policy_data.ls_stas);
			if (pt->steer_policy_data.btm_sta_nr > 0)
				PLATFORM_FREE(pt->steer_policy_data.btm_stas);
			if (pt->steer_policy_data.radio_nr > 0)
				PLATFORM_FREE(pt->steer_policy_data.radio_steer_policy_data);
		}

		if (pt->metric_policy_inclusion) {
			if (pt->metric_policy_data.radio_nr > 0)
				PLATFORM_FREE(pt->metric_policy_data.radio_metric_policy_data);
		}

		break;
	}
	case MAP_AP_METRIC_QUERY_ELEMENT: {
		struct map_ap_metric_query *pt = (struct map_ap_metric_query *)element;
		if (pt->bssid_nr > 0)
			PLATFORM_FREE(pt->bssid);
		break;
	}
	case MAP_UNASSOC_STA_METRIC_QUERY_ELEMENT: {
		struct map_unassoc_sta_metric_query *pt = (struct map_unassoc_sta_metric_query *)element;
		if (pt->channel_data_nr > 0) {
			int i = 0;
			for (i = 0; i < pt->channel_data_nr; i++) {
				PLATFORM_FREE(pt->channel_data[i].stas);
			}
			PLATFORM_FREE(pt->channel_data);
		}
		break;
	}
	case MAP_BEACON_METRIC_QUERY_ELEMENT: {
		struct map_beacon_metric_query *pt = (struct map_beacon_metric_query *)element;
		INT8U                           i;

		if (pt->beacon_metric_query.ssid_len > 0) {
			PLATFORM_FREE(pt->beacon_metric_query.ssid);
		}
		if (pt->beacon_metric_query.channel_report_nr > 0) {
			for (i = 0; i < pt->beacon_metric_query.channel_report_nr; i++) {
				if (pt->beacon_metric_query.channel_report[i].len > 1)
					PLATFORM_FREE(pt->beacon_metric_query.channel_report[i].channel_list);
			}
			PLATFORM_FREE(pt->beacon_metric_query.channel_report);
		}
		if (pt->beacon_metric_query.element_nr > 0) {
			PLATFORM_FREE(pt->beacon_metric_query.element_list);
		}

		break;
	}
	case MAP_CHANNEL_SELECTION_REQUEST_ELEMENT: {
		struct map_channel_selection_request *pt = (struct map_channel_selection_request *)element;
		INT8U                                 i, j;

		for (i = 0; i < pt->ch_preference_nr; i++) {
			for (j = 0; j < pt->ch_preference[i].op_class_nr; j++) {
				PLATFORM_FREE(pt->ch_preference[i].ch_pre_op_class[j].channel_list);
			}
			PLATFORM_FREE(pt->ch_preference[i].ch_pre_op_class);
		}
		PLATFORM_FREE(pt->ch_preference);
		PLATFORM_FREE(pt->tx_pwr_limit);
		break;
	}
#ifdef RTK_MULTI_AP_R2
	case MAP_CAC_REQUEST_ELEMENT: {
		struct map_cac_request *pt = (struct map_cac_request *)element;
		PLATFORM_FREE(pt->cac_req.radios);
		break;
	}
	case MAP_CHANNEL_SCAN_REQUEST_ELEMENT: {
		struct map_channel_scan_request *pt = (struct map_channel_scan_request *)element;
		INT8U                                          i, j;

		for (i = 0; i < pt->channel_scan_req.target_radio_nr; i++) {
			for (j = 0; j < pt->channel_scan_req.target_radios[i].target_op_class_nr; j++) {
				if(pt->channel_scan_req.target_radios[i].target_op_classes[j].target_channel_nr) {
					PLATFORM_FREE(pt->channel_scan_req.target_radios[i].target_op_classes[j].target_channel_list);
				}
			}
			PLATFORM_FREE(pt->channel_scan_req.target_radios[i].target_op_classes);
		}
		PLATFORM_FREE(pt->channel_scan_req.target_radios);
		break;
	}
#endif
	case MAP_CLIENT_STEERING_REQUEST_ELEMENT: {
		struct map_client_steering_request *pt = (struct map_client_steering_request *)element;
		PLATFORM_FREE(pt->steering_req.sta_mac_address);
		PLATFORM_FREE(pt->steering_req.target_bss);
		break;
	}
	case MAP_CLIENT_ASSOCIATION_CONTROL_REQUEST_ELEMENT: {
		struct map_client_association_control_request *pt = (struct map_client_association_control_request *)element;
		INT8U                                          i;

		for (i = 0; i < pt->client_assoc_ctrl_req_nr; i++) {
			PLATFORM_FREE(pt->client_assoc_ctrl_req[i].sta_mac_address);
		}
		PLATFORM_FREE(pt->client_assoc_ctrl_req);
		break;
	}
	case MAP_CONFIGURE_NOTIFICATION_ELEMENT: {
		struct map_configure_notification *pt = (struct map_configure_notification *)element;
		INT8U                              i;
		INT8U                              j;
		for (i = 0; i < pt->config_nr; i++) {
			for (j = 0; j < pt->config_data[i].bss_data_nr; j++) {
				PLATFORM_FREE(pt->config_data[i].bss_data[j].ssid);
				PLATFORM_FREE(pt->config_data[i].bss_data[j].network_key);
				PLATFORM_FREE(pt->config_data[i].bss_data[j].signed_connector);
			}
			PLATFORM_FREE(pt->config_data[i].bss_data);

			for (j = 0; j < pt->config_data[i].vendor_data_nr; j++) {
				PLATFORM_FREE(pt->config_data[i].vendor_datas[j].payload);
			}
			PLATFORM_FREE(pt->config_data[i].vendor_datas);
		}
		PLATFORM_FREE(pt->config_data);
		break;
	}
	case MAP_VENDOR_SPECIFIC_ELEMENT: {
		struct map_vendor_specific_message *pt = (struct map_vendor_specific_message *)element;
		PLATFORM_FREE(pt->vendor_data);
		break;
	}
	case MAP_CONTROLLER_CHANGE_NOTIFICATION_ELEMENT: {
		struct map_controller_change_element *pt = (struct map_controller_change_element *)element;
		PLATFORM_FREE(pt->receiving_interface);
		break;
	}
	case MAP_DPP_CONFIGURATION_ELEMENT: {
		struct map_dpp_configuration *pt = (struct map_dpp_configuration *)element;
		PLATFORM_FREE(pt->signed_connector_1905);
		PLATFORM_FREE(pt->netaccess_key);
		PLATFORM_FREE(pt->csign_key);
		if (pt->pp_key)
			PLATFORM_FREE(pt->pp_key);
		break;
	}
	default: {
		//coredump due to null data, TBD, wait for hostapd easymesh
		if (element_id > 50) {
			if(element_id > MAP_VLAN_CLEAR_NOTIFICATION_ELEMENT) {
				PLATFORM_PRINTF_ERROR("Unknown element_id %d!!!\n", element_id);
				return 0;
			}
			int                   i = 0;
			map_response_element *pt;
			pt = (map_response_element *)element;
			for (i = 0; i < pt->tlv_nr; i++) {
				free_1905_TLV_structure(pt->list_of_tlvs[i]);
			}
			PLATFORM_FREE(pt->list_of_tlvs);
		}
	}
	}
	PLATFORM_FREE(element);
	return 0;
}

map_cmdu_message *_parse_response_de(uint8_t *message)
{
	INT8U *           parsed;
	int               tlvs_nr = 0;
	map_cmdu_message *element = (map_cmdu_message *)PLATFORM_MALLOC(sizeof(map_cmdu_message) * 1);
	element->list_of_tlvs     = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *) * 1);
	element->list_of_tlvs[0]  = NULL;

	_E2B(&message, &element->cmdu_type);
	//PLATFORM_PRINTF_TRACE("%s Element ID %02x\n", __FUNCTION__, element->cmdu_type);
	PLATFORM_PRINTF_TRACE("cmdu_type = %04x -----%d------------------------------------------\n", element->cmdu_type, element->cmdu_type);

	_EnB(&message, element->src_mac, 6);
	//PLATFORM_PRINTF_TRACE("Element SRC %02x:%02x:%02x:%02x:%02x:%02x\n", element->src_mac[0], element->src_mac[1], element->src_mac[2], element->src_mac[3], element->src_mac[4], element->src_mac[5]);
	PLATFORM_PRINTF_TRACE("Element SRC %02x:%02x:%02x:%02x:%02x:%02x\n", element->src_mac[0], element->src_mac[1], element->src_mac[2], element->src_mac[3], element->src_mac[4], element->src_mac[5]);

	while (1) {
		parsed = parse_1905_TLV_from_packet(message);
		if (NULL == parsed) {
			INT8U *p2 = message;
			INT16U len;
			INT8U  aux;

			_E1B(&p2, &aux);
			_E2B(&p2, &len);

			PLATFORM_PRINTF_WARNING("Parsing error. TLV %02x with length %d: \n", aux, len);
			if (len > 200) {
				len = 200;
			}

			char  aux1[200];
			char  aux2[10];
			INT8U first_time;
			INT8U j;

			aux1[0]    = 0x0;
			aux2[0]    = 0x0;
			first_time = 1;
			for (j = 0; j < len + 3; j++) {
				PLATFORM_SNPRINTF(aux2, 6, "0x%02x ", message[j]);
				PLATFORM_STRNCAT(aux1, aux2, 200 - PLATFORM_STRLEN(aux1) - 1);

				if (0 != j && 0 == (j + 1) % 8) {
					if (1 == first_time) {
						PLATFORM_PRINTF_INFO("[PLATFORM]   - Payload        = %s\n", aux1);
						first_time = 0;
					} else {
						PLATFORM_PRINTF_INFO("[PLATFORM]                      %s\n", aux1);
					}
					aux1[0] = 0x0;
				}
			}
			break;
		}

		if (TLV_TYPE_END_OF_MESSAGE == *parsed) {
			// No more TLVs
			//
			free_1905_TLV_structure(parsed);
			break;
		}

		// Advance 'p' to the next TLV.
		//
		INT8U  tlv_type;
		INT16U tlv_len;

		_E1B(&message, &tlv_type);
		_E2B(&message, &tlv_len);

		message += tlv_len;

		// Add this new TLV to the list (the list needs to be re-allocated
		// with more space first)
		//
		tlvs_nr++;
		element->list_of_tlvs              = (INT8U **)PLATFORM_REALLOC(element->list_of_tlvs, sizeof(INT8U *) * (tlvs_nr + 1));
		element->list_of_tlvs[tlvs_nr - 1] = parsed;
		element->list_of_tlvs[tlvs_nr]     = NULL;
	}
	element->tlv_nr = tlvs_nr;
	return element;
}

map_cmdu_message *map_parse_de_message(uint8_t *message)
{
	map_cmdu_message *ret = NULL;
	ret = _parse_response_de(message);

	return ret;
}

uint8_t map_free_de_element(map_cmdu_message *message)
{
	int i = 0;
	if (NULL == message) {
		return 1;
	}

	for (i = 0; i < message->tlv_nr; i++) {
		free_1905_TLV_structure(message->list_of_tlvs[i]);
	}

	PLATFORM_FREE(message->list_of_tlvs);

	PLATFORM_FREE(message);
	return 0;
}
