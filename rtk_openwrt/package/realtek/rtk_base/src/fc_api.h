#ifndef INCLUDE_FC_H
#define INCLUDE_FC_H

int rtk_fc_ctrl_init(void);
#ifdef CONFIG_COMMON_RT_API
typedef enum rtk_rate_application_type_e
{
	RTK_APP_TYPE_IP_QOS_RATE_SHAPING,
	RTK_APP_TYPE_DS_TCP_SYN_RATE_LIMIT,
	RTK_APP_TYPE_DS_UDP_ATTACK_RATE_LIMIT,
	RTK_APP_TYPE_CMCC_QOS_RATE_LIMIT,
	RTK_APP_TYPE_LANHOST_US_RATE_LIMIT_BY_FC,
	RTK_APP_TYPE_LANHOST_DS_RATE_LIMIT_BY_FC,
	RTK_APP_TYPE_DDOS_TCP_SYN_RATE_LIMIT,
	RTK_APP_TYPE_ACL_LIMIT_BANDWIDTH,
	RTK_APP_TYPE_MAX
} rtk_rate_application_type_t;

int rtk_qos_share_meter_set(rtk_rate_application_type_t application_type, unsigned int index, unsigned int rate);
int rtk_qos_share_meter_get(rtk_rate_application_type_t application_type, unsigned int index, unsigned int rate);
int rtk_qos_share_meter_delete(rtk_rate_application_type_t application_type, unsigned int index);
int rtk_qos_share_meter_flush(rtk_rate_application_type_t application_type);
int rtk_qos_share_meter_update_rate(rtk_rate_application_type_t application_type, unsigned int index, unsigned int rate);
#endif
#endif
