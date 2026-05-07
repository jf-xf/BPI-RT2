/*
 * Copyright (C) 2018 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <mqueue.h>

#include "packet_tools.h"
#include "map_utils.h"
#include "map_commands.h"
#include "api_response.h"
#include "1905_cmdus.h"

uint8_t *_forge_request_element(uint8_t *element, uint16_t *stream_len)
{
	if (NULL == element) {
		return NULL;
	}
	INT8U *ret;
	switch (*element) {
	case MAP_TOPOLOGY_QUERY_REQUEST_ELEMENT: {
		ret       = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * 7);
		INT8U *pt = &ret[0];

		struct map_topology_query_request *request = (struct map_topology_query_request *)element;
		_I1B(&request->element_id, &pt);
		_InB(request->target_al_mac, &pt, 6);

		*stream_len = 7;
		return ret;
	}

	case MAP_AUTOCONFIG_SEARCH_REQUEST_ELEMENT: {
		ret       = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * 1);
		INT8U *pt = &ret[0];

		struct map_autoconfig_search_request *request = (struct map_autoconfig_search_request *)element;

		_I1B(&request->element_id, &pt);

		*stream_len = 1;

		return ret;
	}

	case MAP_AUTOCONFIG_RENEW_REQUEST_ELEMENT: {
		ret       = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * 8);
		INT8U *pt = &ret[0];

		struct map_autoconfig_renew_request *request = (struct map_autoconfig_renew_request *)element;

		_I1B(&request->element_id, &pt);
		_I1B(&(request->include_self), &pt);
		_InB(request->target_al_mac, &pt, 6);

		*stream_len = 8;

		return ret;
	}

	case MAP_BACKHAUL_STEERING_REQUEST_ELEMENT: {
		ret       = (INT8U *)PLATFORM_MALLOC(sizeof(struct map_backhaul_steering_request));
		INT8U *pt = &ret[0];

		struct map_backhaul_steering_request *request = (struct map_backhaul_steering_request *)element;

		_I1B(&request->element_id, &pt);
		_InB(&request->target_al_mac, &pt, 6);
		_InB(&(request->backhaul_sta_mac), &pt, 6);
		_InB(&(request->target_bssid), &pt, 6);
		_I1B(&request->op_class, &pt);
		_I1B(&request->channel, &pt);

		*stream_len = sizeof(struct map_backhaul_steering_request);

		return ret;
	}

	case MAP_LINK_METRIC_QUERY_ELEMENT: {
		ret       = (INT8U *)PLATFORM_MALLOC(sizeof(struct map_link_metric_query));
		INT8U *pt = &ret[0];

		struct map_link_metric_query *request = (struct map_link_metric_query *)element;

		_I1B(&request->element_id, &pt);
		_InB(&request->target_al_mac, &pt, 6);
		_I1B(&request->destination, &pt);
		_I1B(&request->type, &pt);
		_InB(&(request->sta_mac), &pt, 6);

		*stream_len = sizeof(struct map_link_metric_query);

		return ret;
	}

	case MAP_CHANNEL_PREFERENCE_QUERY_ELEMENT: {
		ret       = (INT8U *)PLATFORM_MALLOC(7);
		INT8U *pt = &ret[0];

		struct map_channel_preference_query *request = (struct map_channel_preference_query *)element;

		_I1B(&request->element_id, &pt);
		_InB(request->target_al_mac, &pt, 6);

		*stream_len = 7;

		return ret;
	}
	case MAP_CHANNEL_SELECTION_REQUEST_ELEMENT: {
		INT16U len = 0;
		INT8U  i, j;

		struct map_channel_selection_request *request = (struct map_channel_selection_request *)element;

		len = 9;
		for (i = 0; i < request->ch_preference_nr; i++) {
			len += 7;
			for (j = 0; j < request->ch_preference[i].op_class_nr; j++) {
				len += 3 + request->ch_preference[i].ch_pre_op_class[j].channel_nr;
			}
		}
		len += request->tx_pwr_limit_nr * 7;

		ret       = (INT8U *)PLATFORM_MALLOC(len);
		INT8U *pt = &ret[0];

		_I1B(&request->element_id, &pt);
		_InB(request->target_al_mac, &pt, 6);
		_I1B(&request->ch_preference_nr, &pt);
		for (i = 0; i < request->ch_preference_nr; i++) {
			_InB(request->ch_preference[i].radio_mac, &pt, 6);
			_I1B(&request->ch_preference[i].op_class_nr, &pt);
			for (j = 0; j < request->ch_preference[i].op_class_nr; j++) {
				_I1B(&request->ch_preference[i].ch_pre_op_class[j].op_class, &pt);
				_I1B(&request->ch_preference[i].ch_pre_op_class[j].channel_nr, &pt);
				if (request->ch_preference[i].ch_pre_op_class[j].channel_nr)
					_InB(request->ch_preference[i].ch_pre_op_class[j].channel_list, &pt, request->ch_preference[i].ch_pre_op_class[j].channel_nr);
				_I1B(&request->ch_preference[i].ch_pre_op_class[j].preference_reason_code, &pt);
			}
		}
		_I1B(&request->tx_pwr_limit_nr, &pt);
		for (i = 0; i < request->tx_pwr_limit_nr; i++) {
			_InB(request->tx_pwr_limit[i].radio_mac, &pt, 6);
			_I1B(&request->tx_pwr_limit[i].tx_pwr_limit, &pt);
		}

		*stream_len = len;

		return ret;
	}
#ifdef RTK_MULTI_AP_R2
	case MAP_CAC_REQUEST_ELEMENT: {
		INT16U len = 0;
		INT8U  i;

		struct map_cac_request *request = (struct map_cac_request *)element;

		len = 7 + 1 + request->cac_req.radio_nr * 9;

		ret       = (INT8U *)PLATFORM_MALLOC(len);
		INT8U *pt = &ret[0];

		_I1B(&request->element_id, &pt);
		_InB(request->target_al_mac, &pt, 6);
		_I1B(&request->cac_req.radio_nr, &pt);
		for (i = 0; i < request->cac_req.radio_nr; i++) {
			_InB(request->cac_req.radios[i].radio_id, &pt, 6);
			_I1B(&request->cac_req.radios[i].op_class, &pt);
			_I1B(&request->cac_req.radios[i].channel, &pt);
			_I1B(&request->cac_req.radios[i].flags, &pt);
		}

		*stream_len = len;

		return ret;
	}
	case MAP_CHANNEL_SCAN_REQUEST_ELEMENT: {
		INT16U len = 0;
		INT8U  i, j;

		struct map_channel_scan_request *request = (struct map_channel_scan_request *)element;

		len = 7 + 2;
		for (i = 0; i < request->channel_scan_req.target_radio_nr; i++) {
			len += 7;
			for (j = 0; j < request->channel_scan_req.target_radios[i].target_op_class_nr; j++) {
				len += 2 + request->channel_scan_req.target_radios[i].target_op_classes[j].target_channel_nr;
			}
		}

		ret       = (INT8U *)PLATFORM_MALLOC(len);
		INT8U *pt = &ret[0];

		_I1B(&request->element_id, &pt);
		_InB(request->target_al_mac, &pt, 6);
		_I1B(&request->channel_scan_req.flags, &pt);
		_I1B(&request->channel_scan_req.target_radio_nr, &pt);
		for (i = 0; i < request->channel_scan_req.target_radio_nr; i++) {
			_InB(request->channel_scan_req.target_radios[i].radio_id, &pt, 6);
			_I1B(&request->channel_scan_req.target_radios[i].target_op_class_nr, &pt);
			for (j = 0; j < request->channel_scan_req.target_radios[i].target_op_class_nr; j++) {
				_I1B(&request->channel_scan_req.target_radios[i].target_op_classes[j].op_class, &pt);
				_I1B(&request->channel_scan_req.target_radios[i].target_op_classes[j].target_channel_nr, &pt);
				if (request->channel_scan_req.target_radios[i].target_op_classes[j].target_channel_nr) {
					_InB(request->channel_scan_req.target_radios[i].target_op_classes[j].target_channel_list, &pt, request->channel_scan_req.target_radios[i].target_op_classes[j].target_channel_nr);
				}
			}
		}

		*stream_len = len;

		return ret;
	}
#endif
	case MAP_CLIENT_STEERING_REQUEST_ELEMENT: {
		INT16U len = 0;
		INT8U  i;

		struct map_client_steering_request *request = (struct map_client_steering_request *)element;

		len = 7 + 14 + request->steering_req.sta_nr * 6 + request->steering_req.target_bss_nr * 9;

		ret       = (INT8U *)PLATFORM_MALLOC(len);
		INT8U *pt = &ret[0];

		_I1B(&request->element_id, &pt);
		_InB(request->target_al_mac, &pt, 6);
		_InB(request->steering_req.bssid, &pt, 6);
		_I1B(&request->steering_req.mode, &pt);
		_I2B(&request->steering_req.window, &pt);
		_I2B(&request->steering_req.disassoc_timer, &pt);
		_I1B(&request->steering_req.sta_nr, &pt);
		for (i = 0; i < request->steering_req.sta_nr; i++) {
			_InB(request->steering_req.sta_mac_address[i], &pt, 6);
		}
		_I1B(&request->steering_req.mbo_cap, &pt);
		_I1B(&request->steering_req.target_bss_nr, &pt);
		for (i = 0; i < request->steering_req.target_bss_nr; i++) {
			_InB(request->steering_req.target_bss[i].bssid, &pt, 6);
			_I1B(&request->steering_req.target_bss[i].op_class, &pt);
			_I1B(&request->steering_req.target_bss[i].channel, &pt);
			_I1B(&request->steering_req.target_bss[i].reason_code, &pt);
		}

		*stream_len = len;

		return ret;
	}
	case MAP_CLIENT_ASSOCIATION_CONTROL_REQUEST_ELEMENT: {
		INT16U len = 0;
		INT8U  i, j;

		struct map_client_association_control_request *request = (struct map_client_association_control_request *)element;

		len += 8;
		for (i = 0; i < request->client_assoc_ctrl_req_nr; i++) {
			len += 10 + request->client_assoc_ctrl_req[i].sta_nr * 6;
		}

		ret       = (INT8U *)PLATFORM_MALLOC(len);
		INT8U *pt = &ret[0];

		_I1B(&request->element_id, &pt);
		_InB(request->target_al_mac, &pt, 6);
		_I1B(&request->client_assoc_ctrl_req_nr, &pt);
		for (i = 0; i < request->client_assoc_ctrl_req_nr; i++) {
			_InB(request->client_assoc_ctrl_req[i].bssid, &pt, 6);
			_I1B(&request->client_assoc_ctrl_req[i].assoc_ctrl, &pt);
			_I2B(&request->client_assoc_ctrl_req[i].validity_period, &pt);
			_I1B(&request->client_assoc_ctrl_req[i].sta_nr, &pt);
			for (j = 0; j < request->client_assoc_ctrl_req[i].sta_nr; j++) {
				_InB(request->client_assoc_ctrl_req[i].sta_mac_address[j], &pt, 6);
			}
		}

		*stream_len = len;

		return ret;
	}

	case MAP_POLICY_CONFIG_REQUEST_ELEMENT: {

		INT8U                             i;
		INT16U                            len     = 0;
		struct map_policy_config_request *request = (struct map_policy_config_request *)element;

		len = 12;
		if (request->steer_policy_inclusion) {
			len += 3; //ls_sta_nr + btm_sta_nr + radio_nr
			if (request->steer_policy_data.ls_sta_nr)
				len += (request->steer_policy_data.ls_sta_nr * 6);
			if (request->steer_policy_data.btm_sta_nr)
				len += (request->steer_policy_data.btm_sta_nr * 6);
			if (request->steer_policy_data.radio_nr)
				len += (request->steer_policy_data.radio_nr * 9);
		}
		if (request->metric_policy_inclusion) {
			len += 2; //report interval + radio_nr
			if (request->metric_policy_data.radio_nr)
				len += (request->metric_policy_data.radio_nr * 10);
		}

		if (request->default8021q_inclusion) {
			len += 3; //pvid + default pcp
		}
		if (request->traffic_separation_inclusion) {
			len += 1;
			if (request->traffic_separation_data.ssid_nr) {
				for (i = 0; i < request->traffic_separation_data.ssid_nr; i++) {
					len += (1 + request->traffic_separation_data.ssid_info[i].ssid_length + 2);
				}
			}
		}

		ret       = (INT8U *)PLATFORM_MALLOC(len);
		INT8U *pt = &ret[0];

		_I1B(&request->element_id, &pt);
		_InB(request->target_al_mac, &pt, 6);

		_I1B(&request->steer_policy_inclusion, &pt);
		if (request->steer_policy_inclusion) {
			_I1B(&request->steer_policy_data.ls_sta_nr, &pt);
			if (request->steer_policy_data.ls_sta_nr)
				for (i = 0; i < request->steer_policy_data.ls_sta_nr; i++)
					_InB(request->steer_policy_data.ls_stas[i], &pt, 6);
			_I1B(&request->steer_policy_data.btm_sta_nr, &pt);
			if (request->steer_policy_data.btm_sta_nr)
				for (i = 0; i < request->steer_policy_data.btm_sta_nr; i++)
					_InB(request->steer_policy_data.btm_stas[i], &pt, 6);
			_I1B(&request->steer_policy_data.radio_nr, &pt);
			if (request->steer_policy_data.radio_nr)
				for (i = 0; i < request->steer_policy_data.radio_nr; i++)
					_InB(&request->steer_policy_data.radio_steer_policy_data[i], &pt, 9);
		}

		_I1B(&request->metric_policy_inclusion, &pt);
		if (request->metric_policy_inclusion) {
			_I1B(&request->metric_policy_data.report_interval, &pt);
			_I1B(&request->metric_policy_data.radio_nr, &pt);
			if (request->metric_policy_data.radio_nr) {
				for (i = 0; i < request->metric_policy_data.radio_nr; i++)
					_InB(&request->metric_policy_data.radio_metric_policy_data[i], &pt, 10);
			}
		}

		_I1B(&request->default8021q_inclusion, &pt);
		if (request->default8021q_inclusion) {
			_I2B(&request->default8021q_data.primary_vlan_id, &pt);
			_I1B(&request->default8021q_data.default_pcp, &pt);
		}

		_I1B(&request->traffic_separation_inclusion, &pt);
		if (request->traffic_separation_inclusion) {
			_I1B(&request->traffic_separation_data.ssid_nr, &pt);
			for (i = 0; i < request->traffic_separation_data.ssid_nr; i++) {
				_I1B(&request->traffic_separation_data.ssid_info[i].ssid_length, &pt);
				_InB(request->traffic_separation_data.ssid_info[i].ssid, &pt, request->traffic_separation_data.ssid_info[i].ssid_length);
				_I2B(&request->traffic_separation_data.ssid_info[i].vlan_id, &pt);
			}
		}

		*stream_len = len;

		return ret;
	}
	case MAP_AP_CAPABILITY_QUERY_ELEMENT: {
		ret       = (INT8U *)PLATFORM_MALLOC(7);
		INT8U *pt = &ret[0];

		struct map_ap_capability_query *request = (struct map_ap_capability_query *)element;

		_I1B(&request->element_id, &pt);
		_InB(request->target_al_mac, &pt, 6);

		*stream_len = 7;

		return ret;
	}
	case MAP_CLIENT_CAPABILITY_QUERY_ELEMENT: {
		ret       = (INT8U *)PLATFORM_MALLOC(19);
		INT8U *pt = &ret[0];

		struct map_client_capability_query *request = (struct map_client_capability_query *)element;

		_I1B(&request->element_id, &pt);
		_InB(request->target_al_mac, &pt, 6);
		_InB(request->bssid, &pt, 6);
		_InB(request->sta_mac, &pt, 6);

		*stream_len = 19;

		return ret;
	}
	case MAP_AP_METRIC_QUERY_ELEMENT: {
		INT8U                       i;
		INT16U                      len     = 0;
		struct map_ap_metric_query *request = (struct map_ap_metric_query *)element;

		len += 8; //element_id + target_agent_mac + bssid_nr
		len += (request->bssid_nr * 6);

		ret       = (INT8U *)PLATFORM_MALLOC(len);
		INT8U *pt = &ret[0];

		_I1B(&request->element_id, &pt);
		_InB(request->target_al_mac, &pt, 6);
		_I1B(&request->bssid_nr, &pt);
		for (i = 0; i < request->bssid_nr; i++)
			_InB(request->bssid[i], &pt, 6);

		*stream_len = len;

		return ret;
	}
	case MAP_ASSOC_STA_METRIC_QUERY_ELEMENT: {
		INT16U                             len     = 0;
		struct map_assoc_sta_metric_query *request = (struct map_assoc_sta_metric_query *)element;

		len += 13; //element_id + target_agent_mac + sta_mac

		ret       = (INT8U *)PLATFORM_MALLOC(len);
		INT8U *pt = &ret[0];

		_I1B(&request->element_id, &pt);
		_InB(request->target_al_mac, &pt, 6);
		_InB(&request->sta_mac, &pt, 6);

		*stream_len = len;

		return ret;
	}
	case MAP_UNASSOC_STA_METRIC_QUERY_ELEMENT: {
		INT8U                                i, k;
		INT16U                               len     = 0;
		struct map_unassoc_sta_metric_query *request = (struct map_unassoc_sta_metric_query *)element;

		len += 9; //element_id + target_agent_mac + op_class + channel_data_nr

		for (i = 0; i < request->channel_data_nr; i++) {
			len += 2;
			len += (request->channel_data[i].sta_nr * 6);
		}

		ret       = (INT8U *)PLATFORM_MALLOC(len);
		INT8U *pt = &ret[0];

		_I1B(&request->element_id, &pt);
		_InB(request->target_al_mac, &pt, 6);
		_I1B(&request->op_class, &pt);
		_I1B(&request->channel_data_nr, &pt);
		for (i = 0; i < request->channel_data_nr; i++) {
			_I1B(&request->channel_data[i].channel_nr, &pt);
			_I1B(&request->channel_data[i].sta_nr, &pt);
			for (k = 0; k < request->channel_data[i].sta_nr; k++)
				_InB(&request->channel_data[i].stas[k], &pt, 6);
		}

		*stream_len = len;

		return ret;
	}
	case MAP_BEACON_METRIC_QUERY_ELEMENT: {
		INT8U                           i;
		INT16U                          len     = 0;
		struct map_beacon_metric_query *request = (struct map_beacon_metric_query *)element;

		len += 7; //element_id + target_agent_mac
		len += 6 + 2 + 6 + 2 + 1 + 1;
		if (request->beacon_metric_query.ssid_len)
			len += request->beacon_metric_query.ssid_len;

		if (request->beacon_metric_query.channel_report_nr)
			for (i = 0; i < request->beacon_metric_query.channel_report_nr; i++)
				len += (request->beacon_metric_query.channel_report[i].len + 1);

		if (request->beacon_metric_query.element_nr)
			len += request->beacon_metric_query.element_nr;

		ret       = (INT8U *)PLATFORM_MALLOC(len);
		INT8U *pt = &ret[0];

		_I1B(&request->element_id, &pt);
		_InB(request->target_al_mac, &pt, 6);
		_InB(request->beacon_metric_query.sta_mac, &pt, 6);
		_I1B(&request->beacon_metric_query.op_class, &pt);
		_I1B(&request->beacon_metric_query.channel_nr, &pt);
		_InB(request->beacon_metric_query.bssid, &pt, 6);
		_I1B(&request->beacon_metric_query.report_detail, &pt);
		_I1B(&request->beacon_metric_query.ssid_len, &pt);
		_InB(request->beacon_metric_query.ssid, &pt, request->beacon_metric_query.ssid_len);
		_I1B(&request->beacon_metric_query.channel_report_nr, &pt);
		for (i = 0; i < request->beacon_metric_query.channel_report_nr; i++) {
			_I1B(&request->beacon_metric_query.channel_report[i].len, &pt);
			_I1B(&request->beacon_metric_query.channel_report[i].op_class, &pt);
			_InB(request->beacon_metric_query.channel_report[i].channel_list, &pt, (request->beacon_metric_query.channel_report[i].len - 1));
		}
		_I1B(&request->beacon_metric_query.element_nr, &pt);
		if (request->beacon_metric_query.element_nr)
			_InB(request->beacon_metric_query.element_list, &pt, request->beacon_metric_query.element_nr);

		*stream_len = len;

		return ret;
	}
	case MAP_CONFIGURE_NOTIFICATION_ELEMENT: {
		INT16U len = 0;
		len += 2; // element_id + config_nr
		struct map_configure_notification *request = (struct map_configure_notification *)element;
		int                                i       = 0;
		int                                j       = 0;
		INT8U                              aux8    = 0;
		INT16U                             aux16   = 0;

		for (i = 0; i < request->config_nr; i++) {
			len += 1; //radio_band
			len += 1; //bss_data_nr

			for (j = 0; j < request->config_data[i].bss_data_nr; j++) {
				len += strlen(request->config_data[i].bss_data[j].ssid);
				len += strlen(request->config_data[i].bss_data[j].network_key);
				len += 5; // ssid length + network_key length + auth_type + encrypt_type + network_type
				len += 2; // vid
				len += 2; // signed_connector_length
				if (request->config_data[i].bss_data[j].signed_connector) {
					len += strlen(request->config_data[i].bss_data[j].signed_connector);
				}
			}

			len += 1; // vendor data number
			for (j = 0; j < request->config_data[i].vendor_data_nr; j++) {
				len += 5; // Vendor OUI + payload_len
				len += request->config_data[i].vendor_datas[j].payload_len;
			}
		}

		ret       = (INT8U *)PLATFORM_MALLOC(len);
		INT8U *pt = &ret[0];
		_I1B(&request->element_id, &pt);

		_I1B(&request->config_nr, &pt);

		for (i = 0; i < request->config_nr; i++) {
			_I1B(&request->config_data[i].radio_band, &pt);
			_I1B(&request->config_data[i].bss_data_nr, &pt);
			for (j = 0; j < request->config_data[i].bss_data_nr; j++) {
				aux8 = (INT8U)strlen(request->config_data[i].bss_data[j].ssid);
				_I1B(&aux8, &pt);
				_InB(request->config_data[i].bss_data[j].ssid, &pt, strlen(request->config_data[i].bss_data[j].ssid));
				aux8 = (INT8U)strlen(request->config_data[i].bss_data[j].network_key);
				_I1B(&aux8, &pt);
				_InB(request->config_data[i].bss_data[j].network_key, &pt, strlen(request->config_data[i].bss_data[j].network_key));
				_I1B(&(request->config_data[i].bss_data[j].authentication_type), &pt);
				_I1B(&(request->config_data[i].bss_data[j].encryption_type), &pt);
				_I1B(&(request->config_data[i].bss_data[j].network_type), &pt);
				_I2B(&(request->config_data[i].bss_data[j].vid), &pt);
				if (request->config_data[i].bss_data[j].signed_connector) {
					aux16 = strlen(request->config_data[i].bss_data[j].signed_connector);
				} else {
					aux16 = 0;
				}
				_I2B(&aux16, &pt);
				if (request->config_data[i].bss_data[j].signed_connector) {
					_InB(request->config_data[i].bss_data[j].signed_connector, &pt, strlen(request->config_data[i].bss_data[j].signed_connector));
				}
			}

			_I1B(&request->config_data[i].vendor_data_nr, &pt);

			for (j = 0; j < request->config_data[i].vendor_data_nr; j++) {
				_InB(request->config_data[i].vendor_datas[j].vendor_oui, &pt, 3);
				_I2B(&request->config_data[i].vendor_datas[j].payload_len, &pt);
				_InB(request->config_data[i].vendor_datas[j].payload, &pt, request->config_data[i].vendor_datas[j].payload_len);
			}
		}

		*stream_len = len;

		return ret;
	}
	case MAP_OPERATING_CHANNEL_CHANGE_NOTIFICATION_ELEMENT: {
		ret       = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * 1);
		INT8U *pt = &ret[0];

		struct map_operating_channel_change_notification *request = (struct map_operating_channel_change_notification *)element;

		_I1B(&request->element_id, &pt);

		*stream_len = 1;
		return ret;
	}
	case MAP_VENDOR_SPECIFIC_ELEMENT: {
		INT16U len = 0;
		len += 8; // element_id + target_mac + relay_indicator
		len += 5; //vendor_oui + vendor_data_len
		struct map_vendor_specific_message *request = (struct map_vendor_specific_message *)element;

		len += request->vendor_data_len;

		ret       = (INT8U *)PLATFORM_MALLOC(len);
		INT8U *pt = &ret[0];
		_I1B(&request->element_id, &pt);
		_InB(request->target_al_mac, &pt, 6);
		_I1B(&request->relay_indicator, &pt);

		_InB(request->vendor_oui, &pt, 3);

		_I2B(&request->vendor_data_len, &pt);

		_InB(request->vendor_data, &pt, request->vendor_data_len);

		*stream_len = len;

		return ret;
	}
	default: {
		return NULL;
	}
		return NULL;
	}
}

/*********************************************
 * Send Functions
*********************************************/

uint8_t send_topology_query_request(uint8_t target_al_mac[6])
{
	struct map_topology_query_request request;
	request.element_id = MAP_TOPOLOGY_QUERY_REQUEST_ELEMENT;
	PLATFORM_MEMCPY(request.target_al_mac, target_al_mac, 6);

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);
	return 0;
}

uint8_t send_autoconfig_search_request()
{
	struct map_autoconfig_search_request request;
	request.element_id = MAP_AUTOCONFIG_SEARCH_REQUEST_ELEMENT;

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);
	return 0;
}

uint8_t send_autoconfig_renew_request(uint8_t target_al_mac[6])
{
	struct map_autoconfig_renew_request request;
	request.element_id   = MAP_AUTOCONFIG_RENEW_REQUEST_ELEMENT;
	request.include_self = 0;
	PLATFORM_MEMCPY(request.target_al_mac, target_al_mac, 6);

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);
	return 0;
}

uint8_t send_channel_preference_query(uint8_t target_al_mac[6])
{
	struct map_channel_preference_query request;
	request.element_id = MAP_CHANNEL_PREFERENCE_QUERY_ELEMENT;
	PLATFORM_MEMCPY(request.target_al_mac, target_al_mac, 6);

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);
	return 0;
}

uint8_t send_channel_selection_request(uint8_t                      target_al_mac[6],
                                       uint8_t                      ch_preference_nr,
                                       struct channel_preference *  ch_preference,
                                       uint8_t                      tx_pwr_limit_nr,
                                       struct transmit_power_limit *tx_pwr_limit)
{
	struct map_channel_selection_request request;
	request.element_id = MAP_CHANNEL_SELECTION_REQUEST_ELEMENT;
	PLATFORM_MEMCPY(request.target_al_mac, target_al_mac, 6);
	request.ch_preference_nr = ch_preference_nr;
	request.ch_preference    = ch_preference;
	request.tx_pwr_limit_nr  = tx_pwr_limit_nr;
	request.tx_pwr_limit     = tx_pwr_limit;

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);

	return 0;
}

uint8_t send_cac_request(uint8_t target_al_mac[6], struct cac_request cac_req)
{
	struct map_cac_request request;
	request.element_id = MAP_CAC_REQUEST_ELEMENT;
	PLATFORM_MEMCPY(request.target_al_mac, target_al_mac, 6);
	request.cac_req = cac_req;

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);

	return 0;
}

uint8_t send_channel_scan_request(uint8_t target_al_mac[6], struct channel_scan_request channel_scan_req)
{
	struct map_channel_scan_request request;
	request.element_id = MAP_CHANNEL_SCAN_REQUEST_ELEMENT;
	PLATFORM_MEMCPY(request.target_al_mac, target_al_mac, 6);
	request.channel_scan_req = channel_scan_req;

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);

	return 0;
}

uint8_t send_client_steering_request(uint8_t target_al_mac[6], struct steering_request steering_req)
{
	struct map_client_steering_request request;
	request.element_id = MAP_CLIENT_STEERING_REQUEST_ELEMENT;
	PLATFORM_MEMCPY(request.target_al_mac, target_al_mac, 6);
	request.steering_req = steering_req;

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);
	return 0;
}

uint8_t send_client_association_control_request(uint8_t target_al_mac[6], uint8_t client_assoc_ctrl_req_nr,
                                                struct client_association_control_request *client_assoc_ctrl_req)
{
	struct map_client_association_control_request request;
	request.element_id = MAP_CLIENT_ASSOCIATION_CONTROL_REQUEST_ELEMENT;
	PLATFORM_MEMCPY(request.target_al_mac, target_al_mac, 6);
	request.client_assoc_ctrl_req_nr = client_assoc_ctrl_req_nr;
	request.client_assoc_ctrl_req    = client_assoc_ctrl_req;

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);
	return 0;
}

uint8_t send_backhaul_steering_request(uint8_t target_al_mac[6],
                                       uint8_t backhaul_sta_mac[6],
                                       uint8_t target_bssid[6],
                                       uint8_t op_class,
                                       uint8_t channel)
{
	struct map_backhaul_steering_request request;
	request.element_id = MAP_BACKHAUL_STEERING_REQUEST_ELEMENT;
	PLATFORM_MEMCPY(request.target_al_mac, target_al_mac, 6);
	PLATFORM_MEMCPY(request.backhaul_sta_mac, backhaul_sta_mac, 6);
	PLATFORM_MEMCPY(request.target_bssid, target_bssid, 6);
	request.op_class = op_class;
	request.channel  = channel;

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);
	return 0;
}

uint8_t send_policy_config_request(uint8_t                          target_al_mac[6],
                                   uint8_t                          steer_policy_inclusion,
                                   struct steering_config           steer_policy_data,
                                   uint8_t                          metric_policy_inclusion,
                                   struct metric_config             metric_policy_data,
                                   uint8_t                          default8021q_inclusion,
                                   struct default8021q_config       default8021q_data,
                                   uint8_t                          traffic_separation_policy_inclusion,
                                   struct traffic_separation_config traffic_separation_data)
{
	struct map_policy_config_request request;
	request.element_id = MAP_POLICY_CONFIG_REQUEST_ELEMENT;
	memcpy(request.target_al_mac, target_al_mac, 6);
	request.steer_policy_inclusion           = steer_policy_inclusion;
	request.steer_policy_data                = steer_policy_data;
	request.metric_policy_inclusion          = metric_policy_inclusion;
	request.metric_policy_data               = metric_policy_data;
	request.default8021q_inclusion           = default8021q_inclusion;
	request.default8021q_data                = default8021q_data;
	request.traffic_separation_inclusion     = traffic_separation_policy_inclusion;
	request.traffic_separation_data          = traffic_separation_data;

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);
	return 0;
}

uint8_t send_link_metric_query(uint8_t target_al_mac[6],
                               uint8_t destination,
                               uint8_t type,
                               uint8_t sta_mac[6])
{
	struct map_link_metric_query request;
	request.element_id = MAP_LINK_METRIC_QUERY_ELEMENT;
	PLATFORM_MEMCPY(request.target_al_mac, target_al_mac, 6);
	request.destination = destination;
	request.type        = type;
	PLATFORM_MEMCPY(request.sta_mac, sta_mac, 6);
	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);
	return 0;
}

uint8_t send_ap_capabilty_query(uint8_t target_al_mac[6])
{
	struct map_ap_capability_query request;
	request.element_id = MAP_AP_CAPABILITY_QUERY_ELEMENT;
	memcpy(request.target_al_mac, target_al_mac, 6);

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);
	return 0;
}

uint8_t send_client_capability_query(uint8_t target_al_mac[6], uint8_t bssid[6], uint8_t sta_mac[6])
{
	struct map_client_capability_query request;
	request.element_id = MAP_CLIENT_CAPABILITY_QUERY_ELEMENT;
	memcpy(request.target_al_mac, target_al_mac, 6);
	memcpy(request.bssid, bssid, 6);
	memcpy(request.sta_mac, sta_mac, 6);

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);
	return 0;
}

uint8_t send_ap_metric_query(uint8_t target_al_mac[6], uint8_t bssid_nr, uint8_t (*bssid)[6])
{
	struct map_ap_metric_query request;
	request.element_id = MAP_AP_METRIC_QUERY_ELEMENT;
	memcpy(request.target_al_mac, target_al_mac, 6);
	request.bssid_nr = bssid_nr;
	request.bssid    = bssid;

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);
	return 0;
}

uint8_t send_associated_sta_query(uint8_t target_al_mac[6], uint8_t sta_mac[6])
{
	struct map_assoc_sta_metric_query request;
	request.element_id = MAP_ASSOC_STA_METRIC_QUERY_ELEMENT;
	memcpy(request.target_al_mac, target_al_mac, 6);
	memcpy(request.sta_mac, sta_mac, 6);

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);
	return 0;
}

uint8_t send_unassociated_sta_query(uint8_t target_al_mac[6], uint8_t op_class, uint8_t channel_data_nr, struct unassoc_sta_channel *channel_data)
{
	uint8_t                             i;
	struct map_unassoc_sta_metric_query request;
	request.element_id = MAP_UNASSOC_STA_METRIC_QUERY_ELEMENT;
	memcpy(request.target_al_mac, target_al_mac, 6);
	request.op_class        = op_class;
	request.channel_data_nr = channel_data_nr;
	//request.channel_data    = channel_data;
	request.channel_data = malloc(sizeof(struct unassoc_sta_channel) * request.channel_data_nr);

	for (i = 0; i < channel_data_nr; i++) {
		request.channel_data[i].channel_nr = channel_data[i].channel_nr;
		request.channel_data[i].sta_nr     = channel_data[i].sta_nr;
		if (request.channel_data[i].sta_nr) {
			request.channel_data[i].stas = malloc(6 * request.channel_data[i].sta_nr);
			memcpy(request.channel_data[i].stas, channel_data[i].stas, 6 * request.channel_data[i].sta_nr);
		}
	}

	INT16U msg_len = 0;
	INT8U *msg;
	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		if (request.channel_data_nr) {
			for (i = 0; i < request.channel_data_nr; i++) {
				if (request.channel_data[i].sta_nr)
					free(request.channel_data[i].stas);
			}
			free(request.channel_data);
		}
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);

	if (request.channel_data_nr) {
		for (i = 0; i < request.channel_data_nr; i++) {
			if (request.channel_data[i].sta_nr)
				free(request.channel_data[i].stas);
		}
		free(request.channel_data);
	}

	return 0;
}

uint8_t send_beacon_metric_query(uint8_t target_al_mac[6], struct beacon_query_data beacon_metric_query)
{
	struct map_beacon_metric_query request;
	request.element_id = MAP_BEACON_METRIC_QUERY_ELEMENT;
	memcpy(request.target_al_mac, target_al_mac, 6);
	request.beacon_metric_query = beacon_metric_query;

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);

	return 0;
}

uint8_t send_vendor_specific_message(uint8_t target_al_mac[6], uint8_t relay_indicator, uint8_t vendor_oui[3], uint16_t data_len, uint8_t *data_payload)
{
	struct map_vendor_specific_message request;
	request.element_id = MAP_VENDOR_SPECIFIC_ELEMENT;
	memcpy(request.target_al_mac, target_al_mac, 6);
	request.relay_indicator = relay_indicator;
	memcpy(request.vendor_oui, vendor_oui, 3);
	request.vendor_data_len = data_len;
	request.vendor_data     = data_payload;

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);

	return 0;
}

uint8_t send_vlan_configured_notification()
{
	INT16U msg_len = 10; // ID (1) + MAC (6) + EOM_TLV (3)
	INT8U *msg = PLATFORM_MALLOC(sizeof(INT8U) * msg_len);
	memset(msg, 0, msg_len);
	msg[0]= MAP_VLAN_APPLY_NOTIFICATION_ELEMENT;
	api_send_raw_request_message(msg, msg_len);
	return 0;
}

uint8_t send_vlan_clear_notification()
{
	INT16U msg_len = 10; // ID (1) + MAC (6) + EOM_TLV (3)
	INT8U *msg = PLATFORM_MALLOC(sizeof(INT8U) * msg_len);
	memset(msg, 0, msg_len);
	msg[0]= MAP_VLAN_CLEAR_NOTIFICATION_ELEMENT;
	api_send_raw_request_message(msg, msg_len);
	return 0;
}

uint8_t send_operating_channel_change_notification()
{
	struct map_operating_channel_change_notification request;
	request.element_id = MAP_OPERATING_CHANNEL_CHANGE_NOTIFICATION_ELEMENT;

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);
	return 0;
}

uint8_t update_config_data(uint8_t config_nr, struct radio_config_data *radio_config)
{
	struct map_configure_notification request;
	request.element_id  = MAP_CONFIGURE_NOTIFICATION_ELEMENT;
	request.config_nr   = config_nr;
	request.config_data = radio_config;

	INT16U msg_len = 0;
	INT8U *msg;

	msg = _forge_request_element((uint8_t *)&request, &msg_len);

	if (NULL == msg) {
		return 1;
	}

	api_send_raw_request_message(msg, msg_len);

	return 0;
}

uint8_t _register_message_receive_callback_function(uint8_t cmdu_type, void (*callback_func)(uint8_t **tlvs))
{
	int ret;
	ret = api_set_recv_callback(cmdu_type, callback_func);

	return ret;
}

uint8_t register_vendor_message_receive_callback_function(void (*callback_func)(uint8_t **tlvs))
{
	_register_message_receive_callback_function(CMDU_TYPE_VENDOR_SPECIFIC, callback_func);
	return 0;
}

uint8_t _register_message_append_tlv_callback_function(uint8_t cmdu_type, void (*callback_func)(uint8_t *vendor_tlv))
{
	int ret;
	ret = api_set_append_tlv_callback(cmdu_type, callback_func);

	return ret;
}

uint8_t register_append_vendor_tlv_callback_function(void (*tlv_append_callback_func)(uint8_t *vendor_tlv), void (*receive_callback_func)(uint8_t **tlvs))
{
	_register_message_append_tlv_callback_function(CMDU_TYPE_TOPOLOGY_RESPONSE, tlv_append_callback_func);
	_register_message_receive_callback_function(CMDU_TYPE_TOPOLOGY_RESPONSE, receive_callback_func);
	return 0;
}