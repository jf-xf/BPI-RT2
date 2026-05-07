/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 93636 $
 * $Date: 2018-11-16 12:52:37 +0800 (Fri, 16 Nov 2018) $
 *
 * Purpose : Realtek Switch SDK Rtdrv Netfilter Extension Module in the SDK.
 *
 * Feature : Realtek Switch SDK Rtdrv Netfilter Extension Module
 *
 */


#ifndef __RTDRV_EXT_NETFILTER_H__
#define __RTDRV_EXT_NETFILTER_H__

/*
 * Include Files
*/
#if defined(__KERNEL__)
#include <net/sock.h>
#endif
#include <rtdrv/rtdrv_cmdoffset.h>

#include <rt_rate_ext.h>
#include <rt_stat_ext.h>
#include <rt_igmp_ext.h>
#include <rt_acl_ext.h>
#include <rt_misc_ext.h>
#include <rt_flow_ext.h>
#include <rt_qos_ext.h>
#include <rt_pe_ext.h>
#include <rt_crypto_ext.h>

#if defined(__KERNEL__) 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
int32 do_rtdrv_ext_set_ctl(struct sock *sk, int cmd, sockptr_t arg, unsigned int len);
#else
int32 do_rtdrv_ext_set_ctl(struct sock *sk, int cmd, void *user, unsigned int len);
#endif
int32 do_rtdrv_ext_get_ctl(struct sock *sk, int cmd, void *user, int *len);
#endif


/* RT EXT SET ENUM */
enum rtdrv_ext_set_e{
    RTDRV_RT_RATE_HOSTPOLICERCONTROL_SET=(RTDRV_BASE_CTL + RTDRV_EXT_OFFSET),
	RTDRV_RT_RATE_HOSTPOLICERMIB_CLEAR,
	RTDRV_RT_RATE_SW_RATE_LIMIT_SET,
    RTDRV_RT_IGMP_MULTICASTMODE_SET,
    RTDRV_RT_IGMP_UNKNOWN_MC_SET,
    RTDRV_RT_IGMP_MULTICASTGROUP_SET,
    RTDRV_RT_IGMP_MULTICASTGROUP_DEV_SET,
	RTDRV_RT_STAT_FLOWMIB_RESET,
	RTDRV_RT_STAT_FLOW_CACHE_MIB_RESET,
	RTDRV_RT_STAT_NETIFMIB_RESET,
    RTDRV_RT_STAT_GEMFILTER_SET,
    RTDRV_RT_STAT_GEMFILTERMIB_RESET,
	RTDRV_RT_STAT_ACLMIB_SET,
	RTDRV_RT_STAT_ACLMIB_RESET,
    RTDRV_RT_ACL_FILTERANDQOS_DEL,
    RTDRV_RT_ACL_RESERVED_STATUS_SET,
    RTDRV_RT_MISC_WLAN_MAC_LEARNING_LIMIT_SET,
    RTDRV_RT_MISC_WAN_ACCESS_LIMIT_SET,
    RTDRV_RT_MISC_BURST_PACKET_SET,
    RTDRV_RT_MISC_WANPHYPORTMASK_SET,
    RTDRV_RT_MISC_WANFASTFWDDEV_ADD,
    RTDRV_RT_MISC_WANFASTFWDDEV_DEL,
    RTDRV_RT_MISC_VLANGROUPMACLIMIT_SET,
    RTDRV_RT_MISC_VLANGROUPMACLIMIT_DEL,
    RTDRV_RT_FLOW_HWNATMODE_SET,
    RTDRV_RT_QOS_AQM_SET,
    RTDRV_RT_QOS_AQM_COS_SET,
    RTDRV_RT_QOS_AQM_THRESHOLD_SET,
    RTDRV_RT_QOS_AQM_TIMER_SET,
    RTDRV_RT_QOS_AQM_PIE_ENABLE_SET,
    RTDRV_RT_QOS_AQM_PIE_MATCH_SET,
    RTDRV_RT_QOS_AQM_PIE_MAPPING_SET,
    RTDRV_RT_QOS_AQM_PIE_THRESHOLD_SET,
    RTDRV_RT_QOS_AQM_PIE_DPSCALE_SET,
    RTDRV_RT_QOS_AQM_PIE_DPTHRESHOLD_SET,
    RTDRV_RT_QOS_AQM_PIE_DPBURST_SET,
    RTDRV_RT_QOS_AQM_PIE_QDELAY_SET,
    RTDRV_RT_QOS_AQM_PIE_RATE_SET,
    RTDRV_RT_QOS_AQM_PIE_CFG_SET,
};


/* RT EXT GET ENUM */
enum rtdrv_ext_get_e{
    RTDRV_RT_RATE_HOSTPOLICERCONTROL_GET =  (RTDRV_BASE_CTL + RTDRV_EXT_OFFSET),
    RTDRV_RT_RATE_HOSTPOLICERMIB_GET,
    RTDRV_RT_RATE_SW_RATE_LIMIT_GET,
    RTDRV_RT_STAT_FLOW_CACHE_MIB_GET,
    RTDRV_RT_STAT_FLOWMIB_GET,
    RTDRV_RT_STAT_NETIFMIB_GET,
    RTDRV_RT_STAT_GEMFILTER_GET,
    RTDRV_RT_STAT_GEMFILTERMIB_GET,
	RTDRV_RT_STAT_ACLMIB_GET,
    RTDRV_RT_ACL_FILTERANDQOS_ADD,
    RTDRV_RT_ACL_FILTERANDQOS_GET,
    RTDRV_RT_ACL_RESERVED_STATUS_GET,
    RTDRV_RT_MISC_WLAN_MAC_LEARNING_LIMIT_GET,
    RTDRV_RT_MISC_WAN_ACCESS_LIMIT_GET,
    RTDRV_RT_MISC_WANPHYPORTMASK_GET,
    RTDRV_RT_MISC_VLANGROUPMACLIMIT_ADD,
    RTDRV_RT_MISC_VLANGROUPMACLIMIT_GET,
    RTDRV_RT_MISC_VLANGROUPMACLIMIT_FIND,
    RTDRV_RT_FLOW_OPERATION_ADD,
    RTDRV_RT_FLOW_HWNATMODE_GET,
    RTDRV_RT_IGMP_MULTICASTGROUP_DEV_GET,
    RTDRV_RT_QOS_AQM_GET,
    RTDRV_RT_QOS_AQM_COS_GET,
    RTDRV_RT_QOS_AQM_THRESHOLD_GET,
    RTDRV_RT_QOS_AQM_TIMER_GET,
    RTDRV_RT_QOS_AQM_FLOW_STATISTIC_GET,
    RTDRV_RT_QOS_AQM_PIE_ENABLE_GET,
    RTDRV_RT_QOS_AQM_PIE_MATCH_GET,
    RTDRV_RT_QOS_AQM_PIE_MAPPING_GET,
    RTDRV_RT_QOS_AQM_PIE_THRESHOLD_GET,
    RTDRV_RT_QOS_AQM_PIE_DPSCALE_GET,
    RTDRV_RT_QOS_AQM_PIE_DPTHRESHOLD_GET,
    RTDRV_RT_QOS_AQM_PIE_DPBURST_GET,
    RTDRV_RT_QOS_AQM_PIE_QDELAY_GET,
    RTDRV_RT_QOS_AQM_PIE_RATE_GET,
    RTDRV_RT_QOS_AQM_PIE_CFG_GET,
    RTDRV_RT_PE_HTTP_TEST_GET,
    RTDRV_RT_CRYPTO_KEY_SET,
    RTDRV_RT_CRYPTO_KEY_GET,
    RTDRV_RT_CRYPTO_KEY_SET_BY_INDEX,
    RTDRV_RT_CRYPTO_KEY_DEL_BY_INDEX,
    RTDRV_RT_CRYPTO_KEYHASH_GET,
    RTDRV_RT_CRYPTO_KEYHASH_SET_BY_INDEX,
    RTDRV_RT_CRYPTO_KEYHASH_DEL_BY_INDEX,
};

typedef struct rtdrv_rt_rateCfg_ext_s
{
    uint32								index;
    rt_rate_host_policer_control_t		policerControl;
    rt_rate_host_policer_mib_t			policerMib;

	rt_rate_sw_rate_limit_type_t type;
    rt_rate_sw_rate_limit_conf_t swRateLimit_conf;
}rtdrv_rt_rateCfg_ext_t;

typedef struct rtdrv_rt_igmpCfg_ext_s
{
    rt_igmp_multicast_learning_mode_t	mcMode;
    rt_igmp_multicast_group_cfg_t		mcConfig;
	rt_igmp_group_devPort_cfg_t 		mcDevConfig;
	rt_igmp_unknownMc_cfg_t				mcUnknCfg;
}rtdrv_rt_igmpCfg_ext_t;

typedef struct rtdrv_rt_statCfg_ext_s
{
    uint32								index;
	rt_stat_flowCache_mib_t				flowCacheMib;
    rt_stat_flow_mib_t					flowMib;
    char								dev_name[16];
    rt_stat_netif_mib_t					netifMib;
    rt_stat_gemFilter_conf_t            		conf;
    rt_stat_port_cntr_t                			cntr;
    rt_stat_acl_mib_t					aclMib;

}rtdrv_rt_statCfg_ext_t;

typedef struct rtdrv_rt_aclCfg_ext_s
{
    uint32                      		acl_filter_idx;
    rt_acl_filterAndQos_t       		acl_filter;

    rt_acl_reserved_type_t         		reserved_type;
    uint32                      		reserved_status;
}rtdrv_rt_aclCfg_ext_t;

typedef struct rtdrv_rt_miscCfg_ext_s
{
    uint32                      		port_idx;
    uint32                      		wlan_idx;
	uint32                      		device_idx;
	rt_misc_macAddr_learning_limit_t	macAddr_learning_limit;
	rt_misc_wan_access_limit_t          wan_access_limit;
	rt_misc_burst_packet_t				burst_packet;
	rt_misc_wan_phyPortMask_t       	wan_portMask;
	char                                dev_name[16];
}rtdrv_rt_miscCfg_ext_t; 



typedef struct rtdrv_rt_vlanGroupMacLimit_add_s
{
    int               					port;
    int                                 groupLimit;
    int                                 pGroupIdx;
} rtdrv_rt_vlanGroupMacLimit_add_t;


typedef struct rtdrv_rt_vlanGroupMacLimit_set_s
{
    int                                 groupIdx;
    int                                 vlanId;
	int                                 groupLimit;
} rtdrv_rt_vlanGroupMacLimit_set_t;

typedef struct rtdrv_rt_vlanGroupMacLimit_del_s
{
    int                                 groupIdx;
    int                                 vlanId;
} rtdrv_rt_vlanGroupMacLimit_del_t;

typedef struct vvlanGroupMacLimit_get_s
{
    int                                 groupIdx;
    rt_misc_vlanGroupMacLimit_info_t     pGroupInfo;
} rtdrv_rt_vlanGroupMacLimit_get_t;

typedef struct rtdrv_rt_vlanGroupMacLimit_find_s
{
    int					               	port;
    int                                 vlanId;
    int                                 pGroupIdx;
} rtdrv_rt_vlanGroupMacLimit_find_t;



typedef struct rtdrv_rt_flowCfg_ext_s
{
    rt_flow_ops_t                       flowOp;
    rt_flow_ops_data_t					opData;
    rt_flow_hwnat_mode_t                hwnatMode;
}rtdrv_rt_flowCfg_ext_t;

typedef struct rtdrv_rt_qosCfg_ext_s
{
	rt_qos_aqm_cfg_t        		aqm_cfg;
    rt_qos_aqm_cos_cfg_t        	aqm_cos_cfg;
    rt_qos_aqm_threshold_cfg_t  	aqm_threshold_cfg;
    rt_qos_aqm_timer_cfg_t      	aqm_timer_cfg;
	rt_qos_aqm_flow_stat_t			aqm_flow_statistic;
	rt_qos_aqm_pie_enable_t			aqmPie_enable;
	rt_qos_aqm_pie_match_t			aqmPie_match;
	rt_qos_aqm_pie_mapping_t		aqmPie_mapping;
	rt_qos_aqm_pie_threshold_t		aqmPie_threshold;
	rt_qos_aqm_pie_dpScale_t		aqmPie_dpScale;
	rt_qos_aqm_pie_dpThreshold_t	aqmPie_dpThreshold;
	rt_qos_aqm_pie_dpBurst_t		aqmPie_dpBurst;
	rt_qos_aqm_pie_qdelay_t			aqmPie_qdelay;
	rt_qos_aqm_pie_rate_t			aqmPie_rate;
	rt_qos_aqm_pie_cfg_t			aqmPie_cfg;
}rtdrv_rt_qosCfg_ext_t;

typedef struct rtdrv_rt_peCfg_ext_s
{
    rt_pe_http_test_request_t		http_test_request;
	rt_pe_http_test_result_t		http_test_result;
}rtdrv_rt_peCfg_ext_t;

typedef struct rtdrv_rt_cryptoCfg_ext_s
{
    rt_crypto_key_set_t		rt_crypto_key_set_cfg;
}rtdrv_rt_cryptoCfg_ext_t;

typedef union rtdrv_ext_union_u
{
	rtdrv_rt_peCfg_ext_t					rt_pe_cfg;
	rtdrv_rt_rateCfg_ext_t					rt_rate_cfg;
	rtdrv_rt_igmpCfg_ext_t					rt_igmp_cfg;
	rtdrv_rt_statCfg_ext_t					rt_stat_cfg;
	rtdrv_rt_aclCfg_ext_t					rt_acl_cfg;
	rtdrv_rt_miscCfg_ext_t					rt_misc_cfg;
	rtdrv_rt_flowCfg_ext_t					rt_flow_cfg;
	rtdrv_rt_qosCfg_ext_t					rt_qos_cfg;
	rtdrv_rt_vlanGroupMacLimit_find_t		rt_vlanGroupMacLimit_find_cfg;
	rtdrv_rt_vlanGroupMacLimit_add_t		rt_vlanGroupMacLimit_add_cfg;
	rtdrv_rt_vlanGroupMacLimit_del_t		rt_vlanGroupMacLimit_del_cfg;
	rtdrv_rt_vlanGroupMacLimit_get_t		rt_vlanGroupMacLimit_get_cfg;
	rtdrv_rt_vlanGroupMacLimit_set_t		rt_vlanGroupMacLimit_set_cfg;
	rtdrv_rt_cryptoCfg_ext_t				rt_crypto_cfg;
} rtdrv_ext_union_t;


#endif // __RTDRV_EXT_NETFILTER_H__
