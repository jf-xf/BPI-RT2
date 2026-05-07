/*
 * Copyright (C) 2018 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _PLATFORM_INTERFACES_RTK_PRIV_H_
#define _PLATFORM_INTERFACES_RTK_PRIV_H_


// Fill the "interfaceInfo" structure (associated to the provided
// "interface_name") by obtaining information from the OpenWRT UCI subsystem.
//
//
INT8U rtk_linux_get_interface_info(char *interface_name, struct interfaceInfo *m, INT8U need_wps);

// Modify the current Wifi configuration according to the values passed as
// parameters. Modifications take effect immediately.
//
INT8U rtk_linux_apply_80211_configuration(char *interface_name, char *ssid, INT16U auth_type, INT16U encryption_type, char *network_key, INT8U network_type);

INT8U rtk_linux_bss_ioctl(char *interface_name, INT8U type);

INT8U rtk_linux_set_mib(char *interface_name, char *item_str, char *value_str);

INT8U rtk_linux_get_mib(char *interface_name, char *mibname, void *result, int size);

INT8U rtk_linux_get_mib_from_radio_data(char *interface_name, char *mibname, void *result, int size);

INT8U rtk_linux_send_disassoc(char * interfacename, INT8U * sta_mac);

INT8U rtk_linux_del_sta(char *interfacename, INT8U *sta_mac);

INT8U rtk_linux_assoc_control(char * interfacename,INT8U * sta_mac,INT16U timer, INT8U control);

INT8U rtk_linux_get_ap_capability_info(char *interface_name, INT8U tlv_type, INT8U *result);

int rtk_linux_map_general(char *interface_name, INT8U operation_type, INT8U *buffer, INT16U buffer_len, INT16U content_len);

void rtk_linux_issue_cce_ie_indication(char *interface_name, INT8U include_cce_ie);

void rtk_linux_start_reconfig(char *interface_name);

INT8U rtk_linux_get_client_capability_info(char *interface_name, INT8U *macaddr, INT8U *result);

int rtk_linux_get_ap_metric(char *ifname, INT8U tlv_type, INT8U *result);

int rtk_linux_get_assocSta_metric(char *ifname, INT8U tlv_type, INT8U *macaddr, INT8U *results);

int rtk_linux_record_unAssocSta_metric(char *ifname, INT8U op_class, INT8U channel_nr, INT8U sta_nr, INT8U (*macaddr)[6]);

int rtk_linux_issue_beacon_measurement(char *ifname, unsigned char * macaddr, struct BeaconMeasurementRequest *beacon_req);

int rtk_linux_do_backhaul_steering(char *ifname, unsigned char * backhaul_bss, unsigned char * target_bssid, unsigned char op_class, unsigned char channel);

int rtk_linux_do_channel_scan(char* interface_name, unsigned char channel_nr, unsigned char* channels);

int rtk_linux_do_cac(char *interface_name, unsigned char channel_nr, unsigned char *channels, unsigned char type);

INT8U rtk_linux_send_btm_request(char *ifname, INT8U *macaddr, INT8U mode, INT8U *target_bssid, INT8U target_opclass, INT8U target_channel, INT16U disassoc_timer, INT8U reason_code);

int rtk_linux_metric_policy_ioctl(char *interface_name, INT8U rcpi_threshold, INT8U rcpi_hysteris_margin, INT8U channel_utilization_threshold);

int rtk_linux_steering_policy_ioctl(char *interface_name, INT8U policy, INT8U channel_utilization_threshold, INT8U rcpi_threshold);

int rtk_linux_set_tx_max_power_ioctl(char *interface_name, INT8U tx_max_power);

INT8U rtk_linux_get_configured_band();

INT8U rtk_linux_set_configured_band(INT8U configured_band);

INT8U rtk_linux_get_bss_type(char *interface_name);

OP_CLASS *rtk_linux_get_op_class(int *op_classes_len);

INT8U rtk_linux_get_available_channels(char *ifname, INT8U *buffer, INT16U buffer_size);

INT8U rtk_linux_convert_to_global_op_class(INT8U op_class);

INT8U rtk_linux_convert_to_local_op_class(INT8U op_class);

int rtk_get_is_ax_support(char *interface);

INT8U rtk_linux_update_ethernet_clients(INT8U *neighbor_mac_addresses_nr, struct ethernetNeighborInfo **neighbor_list, char *connected_interface);

INT8U rtk_linux_trigger_wps(char *interface);

INT8U rtk_linux_need_hapd();

INT32U rtk_get_prefer_bssid_interval(void);

void rtk_reset_prefer_bssid(void);

int rtk_linux_get_link_metric_ioctl(char *ifname, INT8U *mac, struct linkMetrics *link_metrics);

void rtk_linux_apply_spatial_reuse_config(struct SpatialReuseRequestTLV *request, char* interface_name);

void rtk_linux_get_spatial_reuse_config_response(struct SpatialReuseConfigResponseTLV **response, INT8U *response_nr);

INT8U rtk_linux_get_spatial_reuse_report_tlv(INT8U *buffer, char *interface_name);

#ifdef _RTK_LINUX_
INT8U * rtk_linux_get_clients_rssi(char *interface_name, INT8U *client_address, INT8U clients_nr, INT8U allow_sta_mode);
#endif
#endif
