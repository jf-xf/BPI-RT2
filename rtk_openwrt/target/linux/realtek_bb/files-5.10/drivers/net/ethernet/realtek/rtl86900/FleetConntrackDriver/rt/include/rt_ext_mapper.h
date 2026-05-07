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
 * Purpose : Definition of Rtdrv Extension Mapper
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Mapper register
 *           (2) Mapper get
 *
 */

 
#ifndef __RT_EXT_MAPPER_H__
#define __RT_EXT_MAPPER_H__

/*
 * Include Files
 */
#include <linux/types.h>
#include <rt_rate_ext.h>
#include <rt_igmp_ext.h>
#include <rt_stat_ext.h>
#include <rt_acl_ext.h>
#include <rt_misc_ext.h>
#include <rt_flow_ext.h>
#include <rt_qos_ext.h>
#include <rt_pe_ext.h>
#include <rt_crypto_ext.h>

#ifndef rt_enable_t
#define rt_enable_t rtk_enable_t
typedef struct  rt_mac_s
{
    uint8 octet[ETHER_ADDR_LEN];
} rt_mac_t;
#endif

#include <rtk/rt/rt_cls.h>
#include <rtk/rt/rt_rate.h>
#include <rtk/rt/rt_l2.h>
#include <rtk/rt/rt_ponmisc.h>
#include <rtk/rt/rt_switch.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*data structure for internal use (communication between FC & RT)*/
typedef struct rt_internal_meterConfig_s{
	uint32_t rate;
	uint32_t bucket_size;
	rtk_enable_t ifgInclude;
	rt_rate_meter_mode_t mode;
}rt_internal_meterConfig_t;

typedef struct rt_internal_meterSize_s{
	uint32_t l3_meter_size;
	uint32_t sw_meter_size;
}rt_internal_meterSize_t;

typedef struct rt_ext_mapper_s {
	int (*rate_hostPolicerControl_get)(int index, rt_rate_host_policer_control_t *pPolicerControl);
	int (*rate_hostPolicerControl_set)(int index, rt_rate_host_policer_control_t policerControl);
	int (*rate_hostPolicerMib_get)(int index, rt_rate_host_policer_mib_t *pPolicerMib);
	int (*rate_hostPolicerMib_clear)(int index);
	int (*rate_sw_rate_limit_get)(rt_rate_sw_rate_limit_type_t type, rt_rate_sw_rate_limit_conf_t *pSwRateLimit_conf);
	int (*rate_sw_rate_limit_set)(rt_rate_sw_rate_limit_type_t type, rt_rate_sw_rate_limit_conf_t swRateLimit_conf);
	int (*igmp_multicastMode_set)(rt_igmp_multicast_learning_mode_t mcMode);
	int (*igmp_unknownMulticastAction_set)(rt_igmp_unknownMc_cfg_t	mcUnknCfg);
	int (*igmp_multicastGroup_set)(rt_igmp_multicast_group_cfg_t mcConfig);
	int (*igmp_multicastGroupDev_set)(rt_igmp_group_devPort_cfg_t *mcDevConfig);
	int (*igmp_multicastGroupDev_get)(rt_igmp_group_devPort_cfg_t *mcDevConfig);	
	int (*igmp_multicastFlow_check)(rt_igmp_multicast_group_cfg_t *pmcConfig);
	int (*igmp_userGroup_check)(rtk_igmp_userGroup_cfg_t *userGroup );
	int (*stat_flowCache_mib_get)(uint32 flow_idx, rt_stat_flowCache_mib_t* pFlowCacheMib);
	int (*stat_flowCache_mib_reset)(uint32 flow_idx);
	int (*stat_flowMib_get)(int index, rt_stat_flow_mib_t *pFlowMib);
	int (*stat_flowMib_reset)(int index);
	int (*stat_gemFilter_set)(uint32 index, rt_stat_gemFilter_conf_t conf);
	int (*stat_gemFilter_get)(uint32 index, rt_stat_gemFilter_conf_t* pConf);
	int (*stat_gemFilterMib_reset)(uint32 index);
	int (*stat_gemFilterMib_get)(uint32 index, rt_stat_port_cntr_t* pCntr);
#if defined(CONFIG_RTK_SOC_RTL8198D)
	int (*stat_hwFlowMib_get)(int idx, rt_stat_flow_mib_t *pflowMibCnt);
	int (*stat_brNetifInfo_get)(char *br_name, rtk_mac_t *br_mac, uint32 *br_ip);
#endif
	int (*stat_netifMib_get)(char* dev_name, rt_stat_netif_mib_t* pNetifMib);
	int (*stat_netifMib_reset)(char* dev_name);
	int (*stat_aclMib_get)(int index, rt_stat_acl_mib_t *pAclMib);
	int (*stat_aclMib_set)(int index, rt_stat_acl_mib_t pAclMib);
	int (*stat_aclMib_reset)(int index);
	int (*acl_filterAndQos_add)(rt_acl_filterAndQos_t *acl_filter, int *acl_filter_idx);
	int (*acl_filterAndQos_del)(int acl_filter_idx);
	int (*acl_filterAndQos_get)(int acl_filter_idx, rt_acl_filterAndQos_t *acl_filter);
	int (*acl_reserved_status_get)(rt_acl_reserved_type_t reserved_type, int *status);
	int (*acl_reserved_status_set)(rt_acl_reserved_type_t reserved_type, int status);
	int (*misc_wlan_macAddr_learning_limit_get)(rt_wlan_index_t wlan_idx, rt_wlan_mbssid_index_t device_idx, rt_misc_macAddr_learning_limit_t *pMacAddr_learning_limit);
	int (*misc_wlan_macAddr_learning_limit_set)(rt_wlan_index_t wlan_idx, rt_wlan_mbssid_index_t device_idx, rt_misc_macAddr_learning_limit_t macAddr_learning_limit);
	int (*misc_wan_access_limit_get)(rt_misc_wan_access_limit_t *pWan_access_limit);
	int (*misc_wan_access_limit_set)(rt_misc_wan_access_limit_t wan_access_limit);
	int (*misc_burst_packet_set)(rt_misc_burst_packet_t burst_packet);
	int (*misc_wanPhyPortMask_set)(rt_misc_wan_phyPortMask_t wan_portMask);
	int (*misc_wanPhyPortMask_get)(rt_misc_wan_phyPortMask_t *pWan_portMask);
	int (*misc_wanFastFwdDev_add)(char *dev_name);
	int (*misc_wanFastFwdDev_del)(char *dev_name);
	int (*misc_vlanGroupMacLimit_add)(int port, int groupLimit, int *pGroupIdx);
	int (*misc_vlanGroupMacLimit_del)(int groupIdx, int vlanId);
	int (*misc_vlanGroupMacLimit_set)(int groupIdx, int vlanId, int groupLimit);
	int (*misc_vlanGroupMacLimit_find)(int port, int vlanId, int *pGroupIdx);
	int (*misc_vlanGroupMacLimit_get)(int groupIdx, rt_misc_vlanGroupMacLimit_info_t *pGroup);
	int (*flow_callbackFunc_register)(rt_flow_callback_func_t cbFuncType, void* pCbFunc);
	int (*flow_operation_add)(rt_flow_ops_t flowOp, rt_flow_ops_data_t* pOpData);
	int (*flow_hwnatMode_set)(rt_flow_hwnat_mode_t hwnatMode);
	int (*flow_hwnatMode_get)(rt_flow_hwnat_mode_t *pHwnatMode);
	int (*qos_aqm_set)(rt_qos_aqm_cfg_t aqm_cfg);
	int (*qos_aqm_get)(rt_qos_aqm_cfg_t* pAqm_cfg);
	int (*qos_aqm_cos_set)(rt_qos_aqm_cos_cfg_t aqm_cos_cfg);
	int (*qos_aqm_cos_get)(rt_qos_aqm_cos_cfg_t* pAqm_cos_cfg);
	int (*qos_aqm_threshold_set)(rt_qos_aqm_threshold_cfg_t aqm_threshold_cfg);
	int (*qos_aqm_threshold_get)(rt_qos_aqm_threshold_cfg_t* pAqm_threshold_cfg);
	int (*qos_aqm_timer_set)(rt_qos_aqm_timer_cfg_t aqm_timer_cfg);
	int (*qos_aqm_timer_get)(rt_qos_aqm_timer_cfg_t* pAqm_timer_cfg);
	int (*qos_aqm_flow_statistic_get)(rt_qos_aqm_flow_stat_t* pAqm_flow_stat);
	int (*qos_aqm_pie_enable_get)(rt_qos_aqm_pie_enable_t* pAqmPie_enable);
	int (*qos_aqm_pie_enable_set)(rt_qos_aqm_pie_enable_t aqmPie_enable);
	int (*qos_aqm_pie_match_get)(rt_qos_aqm_pie_match_t* pAqmPie_match);
	int (*qos_aqm_pie_match_set)(rt_qos_aqm_pie_match_t aqmPie_match);
	int (*qos_aqm_pie_mapping_get)(rt_qos_aqm_pie_mapping_t* pAqmPie_mapping);
	int (*qos_aqm_pie_mapping_set)(rt_qos_aqm_pie_mapping_t aqmPie_mapping);
	int (*qos_aqm_pie_threshold_get)(rt_qos_aqm_pie_threshold_t* pAqmPie_threshold);
	int (*qos_aqm_pie_threshold_set)(rt_qos_aqm_pie_threshold_t aqmPie_threshold);
	int (*qos_aqm_pie_dpScale_get)(rt_qos_aqm_pie_dpScale_t* pAqmPie_dpScale);
	int (*qos_aqm_pie_dpScale_set)(rt_qos_aqm_pie_dpScale_t aqmPie_dpScale);
	int (*qos_aqm_pie_dpThreshold_get)(rt_qos_aqm_pie_dpThreshold_t* pAqmPie_dpThreshold);
	int (*qos_aqm_pie_dpThreshold_set)(rt_qos_aqm_pie_dpThreshold_t aqmPie_dpThreshold);
	int (*qos_aqm_pie_dpBurst_get)(rt_qos_aqm_pie_dpBurst_t* pAqmPie_dpBurst);
	int (*qos_aqm_pie_dpBurst_set)(rt_qos_aqm_pie_dpBurst_t aqmPie_dpBurst);
	int (*qos_aqm_pie_qdelay_get)(rt_qos_aqm_pie_qdelay_t* pAqmPie_qdelay);
	int (*qos_aqm_pie_qdelay_set)(rt_qos_aqm_pie_qdelay_t aqmPie_qdelay);
	int (*qos_aqm_pie_rate_get)(rt_qos_aqm_pie_rate_t* pAqmPie_rate);
	int (*qos_aqm_pie_rate_set)(rt_qos_aqm_pie_rate_t aqmPie_rate);
	int (*qos_aqm_pie_cfg_get)(rt_qos_aqm_pie_cfg_t* pAqmPie_cfg);
	int (*qos_aqm_pie_cfg_set)(rt_qos_aqm_pie_cfg_t aqmPie_cfg);
	int (*pe_http_test)(rt_pe_http_test_request_t http_test_req, rt_pe_http_test_result_t *pHttp_test_result);
	int (*crypto_key_set)(rt_crypto_key_set_t *rt_crypto_key_set_cfg, int *key_index);
	int (*crypto_key_get_index)(int *key_index);
	int (*crypto_key_add_by_index)(rt_crypto_key_set_t *rt_crypto_key_set_cfg);
	int (*crypto_key_del_by_index)(int *key_index);
	int (*crypto_keyhash_get_index)(int *key_index);
	int (*crypto_keyhash_add_by_index)(rt_crypto_key_set_t *rt_crypto_key_set_cfg);
	int (*crypto_keyhash_del_by_index)(int *key_index);
	

	/* functions are for internal use (communication between FC & RT) */
	int (*internal_l3SwMeter_set)(rt_rate_meter_type_t vir_meterType, int hw_index, rt_internal_meterConfig_t meterConf);
	int (*internal_l3SwMeter_get)(rt_rate_meter_type_t vir_meterType, int hw_index, rt_internal_meterConfig_t *meterConf);
	int (*internal_l3SwMeterSize_get)(rt_internal_meterSize_t *meterSize);
	int (*internal_l3SwMeterUsedState_get)(int vir_index, bool *ifUsed);
	int (*internal_l3PortUniType_set)(rt_port_t port, rt_port_uniType_t uniType);
	int (*internal_macAddr_learning_limit_get)(rt_phy_port_t port_idx, int *pLearningLimitNumber);
	int (*internal_macAddr_learning_limit_set)(rt_phy_port_t port_idx, int learningLimitNumber);
	int (*internal_g3_l3fe_unknownDAStormCtrl_set)(rt_port_t port, bool enable, int l3fe_pol_idx);
	int (*internal_g3_l3fe_unknownDAStormCtrl_get)(rt_port_t port, bool *enable, int *l3fe_pol_idx);
	int (*internal_l2_addr_add)(rt_l2_ucastAddr_t *pL2Addr);
	int (*internal_l2_addr_del)(rt_l2_ucastAddr_t *pL2Addr);
	int (*internal_l2_addr_get)(int32 *pScanIdx, rt_l2_ucastAddr_t *pL2Addr);
	int (*internal_l2_ageTime_set)(uint32 ageTime);
	int (*internal_pon_sid_set)(uint16 sid,rt_ponmisc_sidInfo_t sidInfo);
	int (*internal_mirror_port_set)(bool enable, uint32 mirroring_port, uint32 mirrored_port);
} rt_ext_mapper_t;

void rt_ext_mapper_register(rt_ext_mapper_t *pMapper);

rt_ext_mapper_t* rt_ext_mapper_get(void);
#endif /*__RT_EXT_MAPPER_H__*/
