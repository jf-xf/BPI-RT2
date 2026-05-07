/*
 * Copyright (C) 2019 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/

#include "rtk_fc_mapper_api.h"
#include "rtk_fc_define.h"
#include "rtk_fc_debug.h"
#include "rtk_fc_internal.h"
#include "rtk_fc_driver.h"
#include "rtk_fc_acl.h"
#include "rtk_fc_multicast.h"
#include "rtk_fc_mappingAPI.h"
#include "rtk_fc_external.h"
#include "rtk_fc_multicast.h"
#include <rtk/switch.h>
#if defined(CONFIG_REALTEK_IPC2RCPU)
#include "ca_ipc_pe.h"
#include <rtk/rt/rt_ponmisc.h>
#endif
#include "rtk_fc_ipsec.h"

#define BURST_10G_MBPS	10000
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#include "aal_mcast.h"
static uint16 sw_mcgid=CA_UINT16_INVALID, hw_mcgid=CA_UINT16_INVALID;
#endif

static rt_ext_mapper_t pf_mapper =
{
	.rate_hostPolicerControl_get 		= 	rtk_fc_hostPoliceControl_get,
	.rate_hostPolicerControl_set 		= 	rtk_fc_hostPoliceControl_set,
	.rate_hostPolicerMib_get 		= 	rtk_fc_hostPoliceMib_get,
	.rate_hostPolicerMib_clear 		= 	rtk_fc_hostPoliceMib_clear,
	.rate_sw_rate_limit_get			= 	rtk_fc_sw_rate_limit_get,
	.rate_sw_rate_limit_set			= 	rtk_fc_sw_rate_limit_set,
	.igmp_multicastMode_set		= 	rtk_fc_igmp_mcMode,
	.igmp_multicastGroup_set		= 	rtk_fc_igmp_mcGroupSet,
	.igmp_multicastGroupDev_set		= 	rtk_fc_igmp_mcGroupDevSet,
	.igmp_multicastGroupDev_get		= 	rtk_fc_igmp_mcGroupDevGet,
	.igmp_multicastFlow_check       =   rtk_fc_igmp_mcflowCheck,
	.igmp_unknownMulticastAction_set	= 	rtk_fc_igmp_unknownMulticast_action,
	.igmp_userGroup_check = rtk_fc_igmp_check_userGroup,
#if defined(CONFIG_FC_RTL8277C_SERIES)
	//TODO: 9607F flow cache mib
	.stat_flowCache_mib_get			=	rtk_fc_flowCache_mib_get,
	.stat_flowCache_mib_reset 		= 	rtk_fc_flowCache_mib_reset,
#else
	.stat_flowCache_mib_get			=	NULL,
	.stat_flowCache_mib_reset 		= 	NULL,
#endif
	.stat_flowMib_get				=	rtk_fc_flowMib_get,
	.stat_flowMib_reset 			= 	rtk_fc_flowMib_clear,
	.stat_gemFilter_set				=	rtk_fc_stat_gemFilter_set,
	.stat_gemFilter_get				= 	rtk_fc_stat_gemFilter_get,
	.stat_gemFilterMib_reset		=	rtk_fc_stat_gemFilterMib_reset,
	.stat_gemFilterMib_get			= 	rtk_fc_stat_gemFilterMib_get,
#if defined(CONFIG_RTK_SOC_RTL8198D)
	.stat_hwFlowMib_get 			=	rtk_fc_hwFlowMib_get,
	.stat_brNetifInfo_get			= 	rtk_fc_brNetifInfo_get,
#endif
	.stat_netifMib_get				=	rtk_fc_netifMib_get,
	.stat_netifMib_reset			=	rtk_fc_netifMib_clear,
	.stat_aclMib_get				=	rtk_fc_aclMib_get,
	.stat_aclMib_set				=	rtk_fc_aclMib_set,
	.stat_aclMib_reset 				=	rtk_fc_aclMib_clear,
	.acl_filterAndQos_add			=	rtk_fc_aclFilterAndQos_add,
	.acl_filterAndQos_del			=	rtk_fc_aclFilterAndQos_del,
	.acl_filterAndQos_get			=	rtk_fc_aclFilterAndQos_get,
	.acl_reserved_status_get		=	rtk_fc_acl_reserved_status_get,
	.acl_reserved_status_set		=	rtk_fc_acl_reserved_status_set,
	.misc_wlan_macAddr_learning_limit_get	=	rtk_fc_wlan_macAddr_learning_limit_get,
	.misc_wlan_macAddr_learning_limit_set	=	rtk_fc_wlan_macAddr_learning_limit_set,
	.misc_wan_access_limit_get		=	rtk_fc_wan_access_limit_get,
	.misc_wan_access_limit_set		=	rtk_fc_wan_access_limit_set,
	.misc_burst_packet_set			=	rtk_fc_burst_packet_set,
	.misc_wanPhyPortMask_set		=	rtk_fc_wanPhyPortMask_set,
	.misc_wanPhyPortMask_get		=	rtk_fc_wanPhyPortMask_get,
	.misc_wanFastFwdDev_add			=	rtk_fc_wanFastFwdDev_add,
	.misc_wanFastFwdDev_del			=	rtk_fc_wanFastFwdDev_del,
	.misc_vlanGroupMacLimit_add		=	rtk_fc_vlanGroupLrnLimit_add,
	.misc_vlanGroupMacLimit_del		=	rtk_fc_vlanGroupLrnLimit_del,
	.misc_vlanGroupMacLimit_set		=	rtk_fc_vlanGroupLrnLimit_set,
	.misc_vlanGroupMacLimit_get		=	rtk_fc_vlanGroupLrnLimit_get,
	.misc_vlanGroupMacLimit_find	=	rtk_fc_vlanGroupLrnLimit_find,
	.flow_callbackFunc_register		=	rtk_fc_flow_callbackFunc_register,
	.flow_operation_add				=	rtk_fc_flow_operation_add,
	.flow_hwnatMode_set				=	rtk_fc_flow_hwnatMode_set,
	.flow_hwnatMode_get				=	rtk_fc_flow_hwnatMode_get,
#if defined(CONFIG_FC_RTL8277C_SERIES)
	//TODO: 9607F aqm
	.qos_aqm_set					=	rtk_fc_qos_aqm_set,
	.qos_aqm_get					=	rtk_fc_qos_aqm_get,
	.qos_aqm_cos_set				=	rtk_fc_qos_aqm_cos_set,
	.qos_aqm_cos_get				=	rtk_fc_qos_aqm_cos_get,
	.qos_aqm_threshold_set			=	rtk_fc_qos_aqm_threshold_set,
	.qos_aqm_threshold_get			=	rtk_fc_qos_aqm_threshold_get,
	.qos_aqm_timer_set				=	rtk_fc_qos_aqm_timer_set,
	.qos_aqm_timer_get				=	rtk_fc_qos_aqm_timer_get,
	.qos_aqm_flow_statistic_get		=	rtk_fc_qos_aqm_flow_statistic_get,
	.qos_aqm_pie_enable_get			=	rtk_fc_qos_aqm_pie_enable_get,
	.qos_aqm_pie_enable_set			=	rtk_fc_qos_aqm_pie_enable_set,
	.qos_aqm_pie_match_get			=	rtk_fc_qos_aqm_pie_match_get,
	.qos_aqm_pie_match_set			=	rtk_fc_qos_aqm_pie_match_set,
	.qos_aqm_pie_mapping_get		=	rtk_fc_qos_aqm_pie_mapping_get,
	.qos_aqm_pie_mapping_set		=	rtk_fc_qos_aqm_pie_mapping_set,
	.qos_aqm_pie_threshold_get		=	rtk_fc_qos_aqm_pie_threshold_get,
	.qos_aqm_pie_threshold_set		=	rtk_fc_qos_aqm_pie_threshold_set,
	.qos_aqm_pie_dpScale_get		=	rtk_fc_qos_aqm_pie_dpScale_get,
	.qos_aqm_pie_dpScale_set		=	rtk_fc_qos_aqm_pie_dpScale_set,
	.qos_aqm_pie_dpThreshold_get	=	rtk_fc_qos_aqm_pie_dpThreshold_get,
	.qos_aqm_pie_dpThreshold_set	=	rtk_fc_qos_aqm_pie_dpThreshold_set,
	.qos_aqm_pie_dpBurst_get		=	rtk_fc_qos_aqm_pie_dpBurst_get,
	.qos_aqm_pie_dpBurst_set		=	rtk_fc_qos_aqm_pie_dpBurst_set,
	.qos_aqm_pie_qdelay_get			=	rtk_fc_qos_aqm_pie_qdelay_get,
	.qos_aqm_pie_qdelay_set			=	rtk_fc_qos_aqm_pie_qdelay_set,
	.qos_aqm_pie_rate_get			=	rtk_fc_qos_aqm_pie_rate_get,
	.qos_aqm_pie_rate_set			=	rtk_fc_qos_aqm_pie_rate_set,
	.qos_aqm_pie_cfg_get			=	rtk_fc_qos_aqm_pie_cfg_get,
	.qos_aqm_pie_cfg_set			=	rtk_fc_qos_aqm_pie_cfg_set,
#else
	.qos_aqm_set					=	NULL,
	.qos_aqm_get					=	NULL,
	.qos_aqm_cos_set				=	NULL,
	.qos_aqm_cos_get				=	NULL,
	.qos_aqm_threshold_set			=	NULL,
	.qos_aqm_threshold_get			=	NULL,
	.qos_aqm_timer_set				=	NULL,
	.qos_aqm_timer_get				=	NULL,
	.qos_aqm_flow_statistic_get		=	NULL,
	.qos_aqm_pie_enable_get			=	NULL,
	.qos_aqm_pie_enable_set			=	NULL,
	.qos_aqm_pie_match_get			=	NULL,
	.qos_aqm_pie_match_set			=	NULL,
	.qos_aqm_pie_mapping_get		=	NULL,
	.qos_aqm_pie_mapping_set		=	NULL,
	.qos_aqm_pie_threshold_get		=	NULL,
	.qos_aqm_pie_threshold_set		=	NULL,
	.qos_aqm_pie_dpScale_get		=	NULL,
	.qos_aqm_pie_dpScale_set		=	NULL,
	.qos_aqm_pie_dpThreshold_get	=	NULL,
	.qos_aqm_pie_dpThreshold_set	=	NULL,
	.qos_aqm_pie_dpBurst_get		=	NULL,
	.qos_aqm_pie_dpBurst_set		=	NULL,
	.qos_aqm_pie_qdelay_get			=	NULL,
	.qos_aqm_pie_qdelay_set			=	NULL,
	.qos_aqm_pie_rate_get			=	NULL,
	.qos_aqm_pie_rate_set			=	NULL,
	.qos_aqm_pie_cfg_get			=	NULL,
	.qos_aqm_pie_cfg_set			=	NULL,
#endif
#if defined(CONFIG_RTK_FC_HTTP_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
	.pe_http_test					=	rtk_fc_pe_http_test,
#else
	.pe_http_test					=	NULL,
#endif
	/*the follow functions are for internal use (between FC & RT)*/
	.internal_l3SwMeter_set			=	rtk_fc_l3SwMeter_set,
	.internal_l3SwMeter_get			=	rtk_fc_l3SwMeter_get,
	.internal_l3SwMeterSize_get		= 	rtk_fc_l3SwMeterSize_get,
	.internal_l3SwMeterUsedState_get= 	rtk_fc_l3SwMeterUsedState_get,
	.internal_l3PortUniType_set		= 	rtk_fc_l3PortUniType_set,
	.internal_macAddr_learning_limit_get 		=	rtk_fc_macAddr_learning_limit_get,
	.internal_macAddr_learning_limit_set 		=	rtk_fc_macAddr_learning_limit_set,
	.internal_g3_l3fe_unknownDAStormCtrl_set	= 	rtk_fc_l3fe_unknownDAStormCtrl_set,
	.internal_g3_l3fe_unknownDAStormCtrl_get=	rtk_fc_l3fe_unknownDAStormCtrl_get,
	.internal_l2_addr_add 			= rtk_fc_l2_addr_add,
	.internal_l2_addr_del 			= rtk_fc_l2_addr_del,
	.internal_l2_addr_get			= rtk_fc_l2_addr_get,
	.internal_l2_ageTime_set		= rtk_fc_l2_ageTime_set,
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	.internal_pon_sid_set			= rtk_fc_pon_sid_set,
	.internal_mirror_port_set		= rtk_fc_mirror_port_set,
#endif
	.crypto_key_set					= rtk_fc_crytpo_key_set,
	.crypto_key_get_index			= rtk_fc_crytpo_key_get_index,
	.crypto_key_add_by_index		= rtk_fc_crytpo_key_add_by_index,
	.crypto_key_del_by_index		= rtk_fc_crytpo_key_del_by_index,
	.crypto_keyhash_get_index			= rtk_fc_crytpo_keyhash_get_index,
	.crypto_keyhash_add_by_index		= rtk_fc_crytpo_keyhash_add_by_index,
	.crypto_keyhash_del_by_index		= rtk_fc_crytpo_keyhash_del_by_index,
};


/****************************
*	APIs for Internal usage		*
*****************************/
int rtk_fc_l3SwMeter_set(rt_rate_meter_type_t vir_meterType, int hw_index, rt_internal_meterConfig_t meterConf)
{
	int ret = SUCCESS;
	rtk_rate_metet_mode_t meterMode;

	if((meterConf.rate > METER_RATE_MAX))
	{
		WARNING("Invalid rate configuration (%d). It should be %d-%d.", meterConf.rate, METER_RATE_MIN, METER_RATE_MAX);
		return(RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	}

	// except ACL meter: support RT_METER_MODE_BIT_RATE only
	if(vir_meterType == RT_METER_TYPE_ACL)
	{
		if(meterConf.mode >= RT_METER_MODE_END)
		{
			WARNING("Invalid mode configuration (%d)", meterConf.mode);
			return(RTK_FC_RET_ERR_INVALID_PARAM);
		}
	}
	else
	{
		if(meterConf.mode != RT_METER_MODE_BIT_RATE)
		{
			WARNING("Invalid mode configuration (%d), support RT_METER_MODE_BIT_RATE only", meterConf.mode);
			return(RT_ERR_FEATURE_NOT_SUPPORTED);
		}
	}

	meterMode = (meterConf.mode == RT_METER_MODE_BIT_RATE)?METER_MODE_BIT_RATE:METER_MODE_PACKET_RATE;

	if(vir_meterType == RT_METER_TYPE_FLOW)
	{
		API("set flowMeter[%d]", hw_index);
		if((hw_index < 0) || (hw_index >= RTK_FC_TABLESIZE_FBMTR))
		{
			WARNING("Invalid flow meter index (%d). It should be 0-%d.", hw_index, RTK_FC_TABLESIZE_FBMTR-1);
			return(RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
		}

		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

		ret = _rtk_fc_l3Meter_set(RT_RATE_EXT_METER_TYPE_FLOW, hw_index, meterConf.rate, meterConf.ifgInclude);
		if(ret != SUCCESS)
			WARNING("set l3 meter rate failed! (hw_index: %d, rate = %d, ret = %d)", hw_index, meterConf.rate, ret);

		ret = _rtk_fc_l3MeterBucket_set(RT_RATE_EXT_METER_TYPE_FLOW, hw_index, meterConf.bucket_size);
		if(ret != SUCCESS)
			WARNING("set l3 meter burst size failed! (hw_index: %d, bucket_size = %d, ret = %d)", hw_index, meterConf.bucket_size, ret);

		ret = _rtk_fc_l3MeterRateMode_set(RT_RATE_EXT_METER_TYPE_FLOW, hw_index, meterMode);
		if(ret != SUCCESS)
			WARNING("set l3 meter rate mode failed! (hw_index: %d, ret = %d)", hw_index, ret);
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
		//sync to policer[hwidx+G3_FLOW_POLICER_IDXSHIFT_FLOWMTR]
		ret = _rtk_fc_l3Meter_set(RT_RATE_EXT_METER_TYPE_FLOW, hw_index + G3_FLOW_POLICER_IDXSHIFT_FLOWMTR, meterConf.rate, meterConf.ifgInclude);
		if(ret != SUCCESS)
			WARNING("set l3 meter rate failed! (hw_index: %d, rate = %d, ret = %d)", hw_index, meterConf.rate, ret);

		ret = _rtk_fc_l3MeterBucket_set(RT_RATE_EXT_METER_TYPE_FLOW, hw_index + G3_FLOW_POLICER_IDXSHIFT_FLOWMTR, meterConf.bucket_size);
		if(ret != SUCCESS)
			WARNING("set l3 meter burst size failed! (hw_index: %d, bucket_size = %d, ret = %d)", hw_index, meterConf.bucket_size, ret);
		
		ret = _rtk_fc_l3MeterRateMode_set(RT_RATE_EXT_METER_TYPE_FLOW, hw_index + G3_FLOW_POLICER_IDXSHIFT_FLOWMTR, meterMode);
		if(ret != SUCCESS)
			WARNING("set l3 meter rate mode failed! (hw_index: %d, ret = %d)", hw_index, ret);
#endif
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	}
	else if(vir_meterType == RT_METER_TYPE_SW)
	{
		API("set swMeter[%d]", hw_index);
		if((hw_index < 0) || (hw_index >= RTK_FC_TABLESIZE_SW_SHAREMTR))
		{
			WARNING("Invalid flow meter index (%d). It should be 0-%d.", hw_index, RTK_FC_TABLESIZE_SW_SHAREMTR-1);
			return(RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
		}


		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

		fc_db.swMeter[hw_index].rate = meterConf.rate;
		fc_db.swMeter[hw_index].ifgInclude = meterConf.ifgInclude;

		//SW not support burst size, meter mode

		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	}
#if defined(CONFIG_RTK_L34_G3_PLATFORM) //host policing and ACL =>9607C: l2 meter, G3: L3 policer
	else if((vir_meterType == RT_METER_TYPE_HOST) || (vir_meterType == RT_METER_TYPE_ACL))
	{
		rt_rate_ext_meter_type_t meterType;

		meterType = (vir_meterType == RT_METER_TYPE_HOST)?RT_RATE_EXT_METER_TYPE_HOST:RT_RATE_EXT_METER_TYPE_ACL;

		API("set l3Meter[%d]", hw_index);
		if((hw_index < RTK_FC_TABLESIZE_FBMTR) || (hw_index >= G3_L3_FLOW_POLICER_SIZE_METER))
		{
			//hwIndx: 0~31(reserved for flow meter)
			WARNING("Invalid flow meter index (%d). It should be %d-%d.", hw_index, RTK_FC_TABLESIZE_FBMTR, G3_L3_FLOW_POLICER_SIZE_METER-1);
			return(RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
		}

		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();


		ret = _rtk_fc_l3Meter_set(meterType, hw_index, meterConf.rate, meterConf.ifgInclude);
		if(ret != SUCCESS)
			WARNING("set l3 meter rate failed! (hw_index: %d, rate = %d, ret = %d)", hw_index, meterConf.rate, ret);

		ret = _rtk_fc_l3MeterBucket_set(meterType, hw_index, meterConf.bucket_size);
		if(ret != SUCCESS)
			WARNING("set l3 meter burst size failed! (hw_index: %d, bucket_size = %d, ret = %d)", hw_index, meterConf.bucket_size, ret);

		ret = _rtk_fc_l3MeterRateMode_set(meterType, hw_index, meterMode);
		if(ret != SUCCESS)
			WARNING("set l3 meter rate mode failed! (hw_index: %d, ret = %d)", hw_index, ret);


		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	}
#endif
	else
	{
		WARNING("invalid virtual meter type: %d", vir_meterType);
	}

	return ret;
}

int rtk_fc_l3SwMeter_get(rt_rate_meter_type_t vir_meterType, int hw_index, rt_internal_meterConfig_t *meterConf)
{
	int ret = SUCCESS;
	rtk_rate_metet_mode_t meterMode;

	if(vir_meterType == RT_METER_TYPE_FLOW)
	{
		API("get flowMeter[%d]", hw_index);
		if((hw_index < 0) || (hw_index >= RTK_FC_TABLESIZE_FBMTR))
		{
			WARNING("Invalid flow meter index (%d). It should be 0-%d.", hw_index, RTK_FC_TABLESIZE_FBMTR-1);
			return(RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
		}

		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

		ret = _rtk_fc_l3Meter_get(RT_RATE_EXT_METER_TYPE_FLOW, hw_index, &meterConf->rate, &meterConf->ifgInclude);
		if(ret != SUCCESS)
			WARNING("get l3 meter rate failed! (hw_index: %d, ret = %d)", hw_index, ret);

		ret = _rtk_fc_l3MeterBucket_get(RT_RATE_EXT_METER_TYPE_FLOW, hw_index, &meterConf->bucket_size);
		if(ret != SUCCESS)
			WARNING("get l3 meter burst size failed! (hw_index: %d, ret = %d)", hw_index, ret);

		ret = _rtk_fc_l3MeterRateMode_get(RT_RATE_EXT_METER_TYPE_FLOW, hw_index, &meterMode);
		if(ret != SUCCESS)
			WARNING("get l3 meter rate mode failed! (hw_index: %d, ret = %d)", hw_index, ret);
		meterConf->mode = (meterMode == METER_MODE_BIT_RATE)?RT_METER_MODE_BIT_RATE:RT_METER_MODE_PACKET_RATE;

		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	}
	else if(vir_meterType == RT_METER_TYPE_SW)
	{
		API("get swMeter[%d]", hw_index);
		if((hw_index < 0) || (hw_index >= RTK_FC_TABLESIZE_SW_SHAREMTR))
		{
			WARNING("Invalid sw meter index (%d). It should be 0-%d.", hw_index, RTK_FC_TABLESIZE_SW_SHAREMTR-1);
			return(RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
		}

		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

		meterConf->rate = fc_db.swMeter[hw_index].rate;
		meterConf->bucket_size = 0;
		meterConf->ifgInclude = fc_db.swMeter[hw_index].ifgInclude;
		meterConf->mode = RT_METER_MODE_BIT_RATE;

		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	}
#if defined(CONFIG_RTK_L34_G3_PLATFORM)	//host policing and ACL =>9607C: l2 meter, G3: L3 policer
	else if((vir_meterType == RT_METER_TYPE_HOST) || (vir_meterType == RT_METER_TYPE_ACL))
	{
		rt_rate_ext_meter_type_t meterType;

		meterType = (vir_meterType == RT_METER_TYPE_HOST)?RT_RATE_EXT_METER_TYPE_HOST:RT_RATE_EXT_METER_TYPE_ACL;

		API("get l3Meter[%d]", hw_index);
		if((hw_index < RTK_FC_TABLESIZE_FBMTR) || (hw_index >= G3_L3_FLOW_POLICER_SIZE_METER))
		{
			//hwIndx: 0~31(reserved for flow meter)
			WARNING("Invalid flow meter index (%d). It should be %d-%d.", hw_index, RTK_FC_TABLESIZE_FBMTR, G3_L3_FLOW_POLICER_SIZE_METER-1);
			return(RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
		}

		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

		ret = _rtk_fc_l3Meter_get(meterType, hw_index, &meterConf->rate, &meterConf->ifgInclude);
		if(ret != SUCCESS)
			WARNING("get l3 meter rate failed! (hw_index: %d, ret = %d)", hw_index, ret);

		ret = _rtk_fc_l3MeterBucket_get(meterType, hw_index, &meterConf->bucket_size);
		if(ret != SUCCESS)
			WARNING("get l3 meter burst size failed! (hw_index: %d, ret = %d)", hw_index, ret);

		ret = _rtk_fc_l3MeterRateMode_get(meterType, hw_index, &meterMode);
		if(ret != SUCCESS)
			WARNING("get l3 meter rate mode failed! (hw_index: %d, ret = %d)", hw_index, ret);
		meterConf->mode = (meterMode == METER_MODE_BIT_RATE)?RT_METER_MODE_BIT_RATE:RT_METER_MODE_PACKET_RATE;

		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	}
#endif
	else
	{
		WARNING("invalid virtual meter type: %d", vir_meterType);
	}

	return ret;
}

int rtk_fc_l3SwMeterSize_get(rt_internal_meterSize_t *meterSize)
{
	int ret = SUCCESS;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	meterSize->l3_meter_size = RTK_FC_TABLESIZE_FBMTR;
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	meterSize->l3_meter_size = G3_L3_FLOW_POLICER_SIZE_METER;
#endif
	meterSize->sw_meter_size = RTK_FC_TABLESIZE_SW_SHAREMTR;

	return ret;
}
int rtk_fc_l3SwMeterUsedState_get(int vir_index, bool *ifUsed)
{
	int ret = SUCCESS, i;
	rt_rate_meter_mapping_t meterMap;
	*ifUsed = FALSE;

	//rt_rate_shareMeterMappingHw_get(vir_index, &meterMap);

	ret = RTK_FC_HELPER_RT_RATE_SHAREMETER_MAPPING_HW_GET(vir_index, &meterMap);
	if(meterMap.type == RT_METER_TYPE_FLOW)
	{
		//check if hw index valid
		if(meterMap.hw_index >= RTK_FC_TABLESIZE_FBMTR)
		{
			WARNING("Invalid flow meter hw index (%d). It should be 0-%d.", meterMap.hw_index, RTK_FC_TABLESIZE_FBMTR-1);
			return(RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
		}
		/* always return FALSE */
	}
	else if(meterMap.type == RT_METER_TYPE_SW)
	{
		//check if hw index valid
		if(meterMap.hw_index >= RTK_FC_TABLESIZE_SW_SHAREMTR)
		{
			WARNING("Invalid sw meter index (%d). It should be 0-%d.", meterMap.hw_index, RTK_FC_TABLESIZE_SW_SHAREMTR-1);
			return(RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
		}
		
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
		for(i = 0 ; i < RT_RATE_SW_RATE_LIMIT_TYPE_MAX ; i++)
		{
			if(fc_db.swRateLimit[i].rateLimit_conf.shareMeterIdx == meterMap.hw_index)
			{
				*ifUsed = TRUE;
				break;
			}
		}
		for(i = 0 ; i < RT_RATE_HOSTPOLICING_TABLE_SIZE ; i++)
		{
			if((fc_db.hostPoliceTable[i].ingress_limit_by_swShaper && (meterMap.hw_index == fc_db.hostPoliceTable[i].ingress_limit_meter_hwIndex)) ||
				(fc_db.hostPoliceTable[i].egress_limit_by_swShaper && (meterMap.hw_index == fc_db.hostPoliceTable[i].egress_limit_meter_hwIndex)))
			{
				*ifUsed = TRUE;
				break;
			}
		}
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	}
	else if(meterMap.type == RT_METER_TYPE_HOST)
	{
		rtk_mac_t zeroMac;
		memset(&zeroMac, 0 , sizeof(rtk_mac_t));

		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
		for(i = 0 ; i < RT_RATE_HOSTPOLICING_TABLE_SIZE ; i++)
		{
			if(!memcmp(&zeroMac, fc_db.hostPoliceTable[i].hostPoliceControl.mac_addr, sizeof(rtk_mac_t)))
				continue;
			if((fc_db.hostPoliceTable[i].hostPoliceControl.ingress_limit_meter_index== vir_index) || (fc_db.hostPoliceTable[i].hostPoliceControl.egress_limit_meter_index== vir_index))
			{
				*ifUsed = TRUE;
				break;
			}
		}

		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
		
	}
	else if(meterMap.type == RT_METER_TYPE_ACL)
	{
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
		for(i = 0 ; i < MAX_ACL_SW_ENTRY_SIZE ; i++)
		{
			if(fc_db.acl_SW_table_entry[i].valid!=ENABLED)
				continue;

			if((fc_db.acl_SW_table_entry[i].acl_filter.action_fields & RT_ACL_ACTION_METER_GROUP_SHARE_METER_BIT) &&
				(fc_db.acl_SW_table_entry[i].hw_share_meter_index == meterMap.hw_index))
			{
				*ifUsed = TRUE;
				break;
			}
		}
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	}

	else
	{
		WARNING("invalid virtual meter type: %d", meterMap.type);
	}

	return ret;
}

int rtk_fc_l3PortUniType_set(rt_port_t port, rt_port_uniType_t uniType)
{	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	if(uniType == RT_PORT_UNI_TYPE_VEIP) {
		fc_db.hypridPPTP.portmask &= ~(1<<port);
	} else if(uniType == RT_PORT_UNI_TYPE_PPTP) {
		fc_db.hypridPPTP.portmask |= (1<<port);
	}

	API("PPTP portmaks: 0x%llx", fc_db.hypridPPTP.portmask);

	//reflash ACL
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	_rtk_fc_aclAndCfReservedRule_intfRsvUpdate(FALSE, FALSE);	//update intf information for gw mac
#endif
	_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_TAIL_END, NULL);
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	
	return SUCCESS;
}

int rtk_fc_macAddr_learning_limit_get(rt_phy_port_t port_idx, int *pLearningLimitNumber)
{
	FC_PARAM_CHK((port_idx>=RT_PORT_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	FC_PARAM_CHK((pLearningLimitNumber == NULL), RTK_FC_RET_ERR_NULL_POINTER);


	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	*pLearningLimitNumber = fc_db.macAddrLearningLimit[port_idx].learningLimit_conf.learningLimitNumber;
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return SUCCESS;
}

int rtk_fc_macAddr_learning_limit_set(rt_phy_port_t port_idx, int learningLimitNumber)
{
	FC_PARAM_CHK((port_idx>=RT_PORT_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	
	fc_db.macAddrLearningLimit[port_idx].learningLimit_conf.learningLimitNumber = learningLimitNumber;
	
	if((learningLimitNumber!= SIGNED_INVALID) && 
		(learningLimitNumber < atomic_read(&fc_db.macAddrLearningLimit[port_idx].learningCount))) {
		// over port learning limit

		DEBUG("over macAddrLearningLimit - delete port[%d] l2 entries", port_idx);
		_rtk_fc_lut_linkdown_del(port_idx);
	}

	rtk_fc_macAddr_portGroup_lrnCnt_config(fc_db.macAddr_portGroup.group.portmask);	// re-config group info
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return SUCCESS;
}

int rtk_fc_l3fe_unknownDAStormCtrl_set(rt_port_t port, bool enable, int l3fe_pol_idx)
{
	int ret = SUCCESS;

#if defined(CONFIG_FC_G3_G3LITE_SERIES)
	FC_PARAM_CHK((port>RT_PORT_PON), RTK_FC_RET_ERR_INVALID_PARAM);
	FC_PARAM_CHK((enable>ENABLED), RTK_FC_RET_ERR_INVALID_PARAM);

	ret = rtk_rg_g3_l3fe_unknownDAStormCtrl(port, enable, l3fe_pol_idx);

	if(!ret) {
		fc_db.uucStorm[port].state 			= enable;
		fc_db.uucStorm[port].l3PolicerIdx 	= l3fe_pol_idx;
	}

#else
	ret = RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;

#endif

	return ret;
}
int rtk_fc_l3fe_unknownDAStormCtrl_get(rt_port_t port, bool *enable, int *l3fe_pol_idx)
{
	int ret = SUCCESS;

#if defined(CONFIG_FC_G3_G3LITE_SERIES)
	FC_PARAM_CHK((port>RT_PORT_PON), RTK_FC_RET_ERR_INVALID_PARAM);

	*enable 			= fc_db.uucStorm[port].state;
	*l3fe_pol_idx 	= fc_db.uucStorm[port].l3PolicerIdx;

#else
	ret = RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;

#endif

	return ret;
}

int rtk_fc_l2_addr_add(rt_l2_ucastAddr_t *pL2Addr)
{
	int ret = SUCCESS;

	return ret;
}

int rtk_fc_l2_addr_del(rt_l2_ucastAddr_t *pL2Addr)
{
	int ret = RTK_FC_RET_ERR;
	int16 lutidx = 0;
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	
	if(_rtk_fc_lut_find(&pL2Addr->mac.octet[0], &lutidx) == RTK_FC_RET_OK) {
		
		API("try to del lut[%d] mac %pM", lutidx, &pL2Addr->mac.octet[0]);
		ret = RTK_FC_LUT_DEL(fc_db.lutTbl[lutidx], FALSE);
	}
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return ret;
}

int rtk_fc_l2_addr_get(int32 *pScanIdx, rt_l2_ucastAddr_t *pL2Addr)
{
	int ret = RTK_FC_RET_ERR;
	int age = 0;
	int lutidx = 0;
	int startIdx = *pScanIdx;
	int32 timeOutsec=-1;
	
	FC_PARAM_CHK((*pScanIdx>=RTK_FC_TABLESIZE_LUT), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	for(lutidx = startIdx; lutidx < RTK_FC_TABLESIZE_LUT; lutidx++) {
		
		if(fc_db.lutTbl[lutidx] != NULL) {
			*pScanIdx = lutidx;
			
			// convert idle time to age time
			RTK_FC_HELPER_JIFFIES_TO_SECS((jiffies - fc_db.lutTbl[lutidx]->last_jiffies), &timeOutsec);
			if(timeOutsec >= RTK_FC_LUTENTRY_TIMEOUT_SECONDS)
				age = 1;
			else 
				age = RTK_FC_LUTENTRY_TIMEOUT_SECONDS - timeOutsec;

			memcpy(&pL2Addr->mac.octet, &fc_db.lutTbl[lutidx]->l2Addr, ETH_ALEN);
			pL2Addr->port = fc_db.lutTbl[lutidx]->spa;
			pL2Addr->age = age;
			pL2Addr->filterFlag = FALSE;
			pL2Addr->staticFlag = fc_db.lutTbl[lutidx]->isStatic ? TRUE : FALSE;
			pL2Addr->vid = fc_db.lutTbl[lutidx]->CTAG_IF ?  fc_db.lutTbl[lutidx]->CVID : 0;

#if defined(CONFIG_RTK_FC_EXTERNAL_SWITCH_PORT)	
			if(fc_db.controlFuc.external_switch_en && (pL2Addr->port == CONFIG_RTK_FC_EXTERNAL_SWITCH_PORT))
			{
				pL2Addr->port = fc_db.controlFuc.external_switch_port_offset + fc_db.lutTbl[lutidx]->extspa;
			}
#endif
			
			ret = RTK_FC_RET_OK;
			break;
		}
	}

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	
	return ret;
}

int rtk_fc_l2_ageTime_set(uint32 ageTime)
{
	int ret = RTK_FC_RET_ERR;
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	fc_db.ucTimeout_unit1s = ageTime;
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return ret;
}

#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
int rtk_fc_pon_sid_set(uint16 sid, rt_ponmisc_sidInfo_t sidInfo)
{
	int ret = SUCCESS;
	int find = 0, i = 0;

	FC_PARAM_CHK((sid>=RTK_FC_TABLESIZE_STREAMID), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	fc_db.streamidTbl[sid].valid = ((sidInfo.enFlag == ENABLED)? TRUE :FALSE);
	fc_db.streamidTbl[sid].ldpid = sidInfo.dstPort;
	fc_db.streamidTbl[sid].cos = sidInfo.pri;
	fc_db.streamidTbl[sid].gemid = sidInfo.flowId;
	//sidInfo.dqFlag is ignored
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	{	//for direct tx without headers
		aal_ni_dma_lso_stream_id_tbl_cfg_t stream_tbl_cfg;
		stream_tbl_cfg.en_flag = ((sidInfo.enFlag == ENABLED)? TRUE :FALSE);
		stream_tbl_cfg.ldpid = sidInfo.dstPort;
		stream_tbl_cfg.cos = sidInfo.pri;
		stream_tbl_cfg.pol_id = sidInfo.flowId;
		stream_tbl_cfg.deep_q = sidInfo.dqFlag;
		aal_ni_dma_lso_set_streamid_tbl(sid, &stream_tbl_cfg);
	}
#endif

	for(i=0; i <RTK_FC_TABLESIZE_STREAMID;i++) {
		if(fc_db.streamidTbl[i].valid) {
			find = TRUE;
			break;
		}
	}
	fc_db.systemGlobal.pon_sidTbl_en = find?TRUE:FALSE;		
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	
	return ret;
}

int rtk_fc_mirror_port_set(bool enable, uint32 mirroring_port, uint32 mirrored_port)
{
	int ret = SUCCESS;
	uint32 mrr_port = enable ? mirroring_port : AAL_LPORT_BLACKHOLE;
	aal_ni_hv_glb_internal_port_id_cfg_mask_t niGlbCfgMask={0};
	aal_ni_hv_glb_internal_port_id_cfg_t niGlbCfg={0};
	rtk_fc_aclAndCf_reserved_generic_intf_mirrored_cfg_t cls_mir_cfg = {0};

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	// mirroring port config
	niGlbCfgMask.wrd = 0;
	niGlbCfgMask.bf.mrr_ldpid = TRUE;
	niGlbCfgMask.bf.mrr_dsel = TRUE;
	niGlbCfg.mrr_ldpid = mrr_port;
	niGlbCfg.mrr_dsel = enable ? 0x2 : 0x3; // // mirrored packet destination is L2TM(0x2) if mirror is enabled, destination is L2FE(0x3) if mirror is disabled.
	aal_ni_hv_glb_internal_port_id_cfg_set(CA_DEF_DEVID, niGlbCfgMask, &niGlbCfg);

	// mirrored port config
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	rtk_rg_asic_mirrored_port_set(enable, (mirrored_port&0x3f));
#else
	ca_flow_egress_mirrored_port_set(enable, (mirrored_port&0x3f));
#endif

#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	if(fc_db.controlFuc.p6_xfi_en) {
		// p7_rxsel_l3fe should be ready first for ACL reference
		if(enable && mirrored_port==RTK_FC_MAC_PORT_PON)
			fc_db.controlFuc.p7_rxsel_l3fe = 0;
		else
			fc_db.controlFuc.p7_rxsel_l3fe = 1;

		RTK_FC_ASIC_PONRX_TO_L3FE_SET(fc_db.controlFuc.p7_rxsel_l3fe);
	}
#endif

	RTK_FC_HELPER_MGR_MIRROR_CONFIG_SET(enable, mirrored_port);

	cls_mir_cfg.mirror_en = enable;
	cls_mir_cfg.mirrord_port = mirrored_port&0x3f;
	// L3CLS control if mirror cpu (trap)
	_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CA_CLS_TYPE_GENERIC_INTF_HASH_PROFILE_DECISION, NULL);
	if(enable && (mirrored_port == AAL_LPORT_CPU_0)) {
		// cls enable mrr fib action
		cls_mir_cfg.cls_mir_en = 1;
	}else {
		// cls disable mrr fib action
		cls_mir_cfg.cls_mir_en = 0;
	}
	_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_GENERIC_INTF_HASH_PROFILE_DECISION, &cls_mir_cfg);
	
	rtk_fc_flow_flush();

	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	
	return ret;
}
#endif

#if 1
/****************************
*	APIs for RT EXT fcuntions	*
*****************************/
int rtk_fc_sw_rate_limit_get(rt_rate_sw_rate_limit_type_t type, rt_rate_sw_rate_limit_conf_t *pSwRateLimit_conf)
{
	FC_PARAM_CHK((type<0 || type>=RT_RATE_SW_RATE_LIMIT_TYPE_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	FC_PARAM_CHK((pSwRateLimit_conf == NULL), RTK_FC_RET_ERR_NULL_POINTER);


	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	memcpy(pSwRateLimit_conf, &fc_db.swRateLimit[type].rateLimit_conf, sizeof(rt_rate_sw_rate_limit_conf_t));
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();


	return SUCCESS;
}

int rtk_fc_sw_rate_limit_set(rt_rate_sw_rate_limit_type_t type, rt_rate_sw_rate_limit_conf_t swRateLimit_conf)
{
	int32 ret, i, wlan_dev_mask_enable=0;
	rt_rate_meter_mapping_t meterMapping;

	FC_PARAM_CHK((type<0 || type>=RT_RATE_SW_RATE_LIMIT_TYPE_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	FC_PARAM_CHK((swRateLimit_conf.shareMeterIdx<FAIL), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	FC_PARAM_CHK((type==RT_RATE_SW_RATE_LIMIT_TYPE_SPECIFIC_DOS && !(swRateLimit_conf.specific_dos.ctagif<=1)), RTK_FC_RET_ERR_INVALID_PARAM);
	FC_PARAM_CHK((type==RT_RATE_SW_RATE_LIMIT_TYPE_SPECIFIC_DOS && !(swRateLimit_conf.specific_dos.ctagVid<=4095)), RTK_FC_RET_ERR_INVALID_PARAM);
	FC_PARAM_CHK((type==RT_RATE_SW_RATE_LIMIT_TYPE_SPECIFIC_DOS && !(swRateLimit_conf.specific_dos.isTcp<=1)), RTK_FC_RET_ERR_INVALID_PARAM);
	FC_PARAM_CHK((type==RT_RATE_SW_RATE_LIMIT_TYPE_SPECIFIC_DOS && (swRateLimit_conf.specific_dos.pktLenEnd < swRateLimit_conf.specific_dos.pktLenStart)), RTK_FC_RET_ERR_INVALID_PARAM);

	for(i=0; i<RT_WLAN_DEVICE_MAX; i++)
	{
		if(swRateLimit_conf.ingress_wlan_dev_mask[i] != 0x0)
		{
			wlan_dev_mask_enable = 1;
			break;
		}
	}
	if(swRateLimit_conf.shareMeterIdx==FAIL || (swRateLimit_conf.ingress_port_mask==0x0 && wlan_dev_mask_enable==0))
	{
		API("Delete sw rate limit, type=%d", type);
		memset(&swRateLimit_conf, 0, sizeof(rt_rate_sw_rate_limit_conf_t));
		swRateLimit_conf.shareMeterIdx = FAIL;
	}
	else
	{
		//ret = rt_rate_shareMeterMappingHw_get(swRateLimit_conf.shareMeterIdx, &meterMapping);
		ret = RTK_FC_HELPER_RT_RATE_SHAREMETER_MAPPING_HW_GET(swRateLimit_conf.shareMeterIdx, &meterMapping);
		if((ret == RT_ERR_OK) && (meterMapping.type == RT_METER_TYPE_SW))
		{
			swRateLimit_conf.shareMeterIdx = meterMapping.hw_index;
		}
		else
		{
			WARNING("Get l3Meter hw index Failed, return value: 0x%x, meter type: %d", ret, meterMapping.type);
			return RTK_FC_RET_ERR;
		}
	}
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	memcpy(&fc_db.swRateLimit[type].rateLimit_conf, &swRateLimit_conf, sizeof(rt_rate_sw_rate_limit_conf_t));

	fc_db.controlFuc.swRateLimit_en = FALSE;
	for(i=0; i<RT_RATE_SW_RATE_LIMIT_TYPE_MAX; i++) {
		if(fc_db.swRateLimit[i].rateLimit_conf.shareMeterIdx != SIGNED_INVALID)
			fc_db.controlFuc.swRateLimit_en = TRUE;
	}
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	
	return SUCCESS;
}

int rtk_fc_crytpo_key_set(rt_crypto_key_set_t *rt_crypto_key_set_cfg, int *key_index)
{
	int32 ret = SUCCESS;

#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)

	API("add key entry");

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	
	ret = _rtk_fc_crypto_key_set(rt_crypto_key_set_cfg, key_index);
	if(ret != RT_ERR_RG_OK)
		WARNING("ERROR, ret = 0x%x", ret);
	
	printk("add key entry[%d] with return value 0x%x!\n", rt_crypto_key_set_cfg->key_index, ret);


	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
#endif	


	return ret;
}

int rtk_fc_crytpo_key_get_index(int *key_index)
{
	int32 ret = SUCCESS;

#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)

	API("add key entry");

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	ret = _rtk_fc_crypto_key_get_index(key_index);
	if(ret != RT_ERR_RG_OK)
		WARNING("ERROR, ret = 0x%x", ret);
	
	printk("Occupy key entry[%d] with return value 0x%x!\n", *key_index, ret);


	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
#endif	



	return ret;

}
int rtk_fc_crytpo_key_add_by_index(rt_crypto_key_set_t *rt_crypto_key_set_cfg)
{
	int32 ret = SUCCESS;

#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)

	API("add key entry");

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	ret = _rtk_fc_crypto_key_add_by_index(rt_crypto_key_set_cfg);
	if(ret != RT_ERR_RG_OK)
		WARNING("ERROR, ret = 0x%x", ret);
	
	printk("Add key entry[%d] with return value 0x%x!\n", rt_crypto_key_set_cfg->key_index, ret);


	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
#endif	



	return ret;

}

int rtk_fc_crytpo_key_del_by_index(int *key_index)
{
	int32 ret = SUCCESS;

#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)

	API("add key entry");

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	ret = _rtk_fc_crytpo_key_del_by_index(*key_index);
	if(ret != RT_ERR_RG_OK)
		WARNING("ERROR, ret = 0x%x", ret);
	
	printk("Delete key entry[%d] with return value 0x%x!\n", *key_index, ret);


	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
#endif	



	return ret;

}



int rtk_fc_crytpo_keyhash_get_index(int *key_index)
{
	int32 ret = SUCCESS;

#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)

	API("add key entry");

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	ret = _rtk_fc_crypto_keyhash_get_index(key_index);
	if(ret != RT_ERR_RG_OK)
		WARNING("ERROR, ret = 0x%x", ret);
	
	printk("Occupy key entry[%d] with return value 0x%x!\n", *key_index, ret);


	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
#endif	



	return ret;

}
int rtk_fc_crytpo_keyhash_add_by_index(rt_crypto_key_set_t *rt_crypto_key_set_cfg)
{
	int32 ret = SUCCESS;

#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)

	API("add key entry");

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	ret = _rtk_fc_crypto_keyhash_add_by_index(rt_crypto_key_set_cfg);
	if(ret != RT_ERR_RG_OK)
		WARNING("ERROR, ret = 0x%x", ret);
	
	printk("Add key entry[%d] with return value 0x%x!\n", rt_crypto_key_set_cfg->key_index, ret);


	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
#endif	



	return ret;

}

int rtk_fc_crytpo_keyhash_del_by_index(int *key_index)
{
	int32 ret = SUCCESS;

#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)

	API("add key entry");

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	ret = _rtk_fc_crytpo_keyhash_del_by_index(*key_index);
	if(ret != RT_ERR_RG_OK)
		WARNING("ERROR, ret = 0x%x", ret);
	
	printk("Delete key entry[%d] with return value 0x%x!\n", *key_index, ret);


	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
#endif	



	return ret;

}


int rtk_fc_aclFilterAndQos_add(rt_acl_filterAndQos_t *acl_filter, int *acl_filter_idx)
{
	int32 ret = SUCCESS;
	API("add acl entry");

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	
	ret = _rtk_fc_aclFilterAndQos_add(acl_filter, acl_filter_idx);
	if(ret != RT_ERR_RG_OK)
		WARNING("ERROR, ret = 0x%x", ret);
	API("add acl entry[%d] with return value 0x%x!", *acl_filter_idx, ret);


	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return ret;
}

int rtk_fc_aclFilterAndQos_del(int acl_filter_idx)
{
	int32 ret = SUCCESS;
	API("del acl entry[%d]", acl_filter_idx);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	ret = _rtk_fc_aclFilterAndQos_del(acl_filter_idx);
	if(ret != RT_ERR_RG_OK)
		WARNING("ERROR, ret = 0x%x", ret);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return ret;
}

int rtk_fc_aclFilterAndQos_get(int acl_filter_idx, rt_acl_filterAndQos_t *acl_filter)
{
	int32 ret = SUCCESS;
	API("get acl entry[%d]", acl_filter_idx);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	ret = _rtk_fc_aclFilterAndQos_get(acl_filter_idx, acl_filter);
	if(ret != RT_ERR_RG_OK)
		WARNING("ERROR, ret = 0x%x", ret);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return ret;
}

int rtk_fc_acl_reserved_status_get(rt_acl_reserved_type_t reserved_type, int *status)
{
	int32 ret = SUCCESS;
	API("get acl reserved status type[%d]", reserved_type);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	ret = _rtk_fc_acl_reserved_status_get(reserved_type, status);
	if(ret != RT_ERR_RG_OK)
		WARNING("ERROR, ret = 0x%x", ret);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return ret;
}

int rtk_fc_acl_reserved_status_set(rt_acl_reserved_type_t reserved_type, int status)
{
	int32 ret = SUCCESS;
	API("set acl reserved status type[%d] status %d", reserved_type, status);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	ret = _rtk_fc_acl_reserved_status_set(reserved_type, status);
	if(ret != RT_ERR_RG_OK)
		WARNING("ERROR, ret = 0x%x", ret);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return ret;
}

int rtk_fc_wlan_macAddr_learning_limit_get(rt_wlan_index_t wlan_idx, rt_wlan_mbssid_index_t device_idx, rt_misc_macAddr_learning_limit_t *pMacAddr_learning_limit)
{
	rtk_fc_ret_t ret = RTK_FC_RET_OK;
	rtk_fc_wlan_devidx_t wlanDevidx;

	FC_PARAM_CHK((wlan_idx>=RT_WLAN_DEVICE_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	FC_PARAM_CHK((device_idx>=RT_WLAN_MBSSID_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	FC_PARAM_CHK((pMacAddr_learning_limit == NULL), RTK_FC_RET_ERR_NULL_POINTER);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	wlanDevidx = rtk_fc_wlan_rtmbssid_to_devidx(wlan_idx, device_idx);

	if (wlanDevidx < RTK_FC_WLANX_END_INTF) {

		memcpy(pMacAddr_learning_limit, &fc_db.wlanMacAddrLearningLimit[wlanDevidx].learningLimit_conf, sizeof(rt_misc_macAddr_learning_limit_t));
	}else {

		ret = RTK_FC_RET_ERR_ENTRY_NOT_FOUND;
	}
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return ret;

}

int rtk_fc_wlan_macAddr_learning_limit_set(rt_wlan_index_t wlan_idx, rt_wlan_mbssid_index_t device_idx, rt_misc_macAddr_learning_limit_t macAddr_learning_limit)
{
	rtk_fc_ret_t ret = RTK_FC_RET_OK;
	rtk_fc_wlan_devidx_t wlanDevidx;

	FC_PARAM_CHK((wlan_idx>=RT_WLAN_DEVICE_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	FC_PARAM_CHK((device_idx>=RT_WLAN_MBSSID_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	wlanDevidx = rtk_fc_wlan_rtmbssid_to_devidx(wlan_idx, device_idx);
	if (wlanDevidx < RTK_FC_WLANX_END_INTF) {

		memcpy(&fc_db.wlanMacAddrLearningLimit[wlanDevidx].learningLimit_conf, &macAddr_learning_limit, sizeof(rt_misc_macAddr_learning_limit_t));

		if((fc_db.wlanMacAddrLearningLimit[wlanDevidx].learningLimit_conf.learningLimitNumber!= SIGNED_INVALID) && 
			(fc_db.wlanMacAddrLearningLimit[wlanDevidx].learningLimit_conf.learningLimitNumber < atomic_read(&fc_db.wlanMacAddrLearningLimit[wlanDevidx].learningCount))) {
			// over wlandev learning limit	
			rtk_fc_lut_entry_t *pLutEntry=NULL;
			int i = 0;
			for(i = 0; i < RTK_FC_LUT_BUCKET_SIZE; i++){
				list_for_each_entry_rcu(pLutEntry, &fc_db.lut_quickhash_list_head[i], lutQuickList)				{
					if(pLutEntry && pLutEntry->wlan_dev_idx == wlanDevidx)
					{
						DEBUG("over wlanMacAddrLearningLimit - delete lut[%d]", pLutEntry->lutIdx);
						RTK_FC_LUT_DEL(pLutEntry, TRUE);
					}
				}
			}

		}
			
	}else {

		ret = RTK_FC_RET_ERR_ENTRY_NOT_FOUND;
	}
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return ret;

}

int rtk_fc_wan_access_limit_get(rt_misc_wan_access_limit_t *pWan_access_limit)
{
	FC_PARAM_CHK((pWan_access_limit == NULL), RTK_FC_RET_ERR_NULL_POINTER);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	pWan_access_limit->portMask=fc_db.wanAccessLimit.portMask;
	rtk_fc_wlan_devMask_to_rtmbssidMask(&fc_db.wanAccessLimit.wlanMask, pWan_access_limit->wlanMask);
	pWan_access_limit->accessLimitNumber=fc_db.wanAccessLimit.accessLimitNumber;
	pWan_access_limit->limitField=fc_db.wanAccessLimit.limitField;
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return SUCCESS;
}

int rtk_fc_wan_access_limit_set(rt_misc_wan_access_limit_t wan_access_limit)
{
	int32 i = 0;
	rtk_fc_lut_entry_t *pLutEntry=NULL;

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	//clear all lut and flow
	for(i = 0; i < RTK_FC_LUT_BUCKET_SIZE; i++)
	{
		list_for_each_entry_rcu(pLutEntry, &fc_db.lut_hash_list_head[i], lutList)
		{
			RTK_FC_LUT_DEL(pLutEntry, FALSE);
		}
	}
	//clear all IP
	_rtk_fc_wanAccessLimit_IP_reset();
	atomic_set(&fc_db.wanAccessLimit.learningCount, 0);
	if(RTK_FC_HELPER_TIMER_PENDING(fc_db.wanAccessLimit.neighbor_probe_timer))
		RTK_FC_HELPER_DEL_TIMER(fc_db.wanAccessLimit.neighbor_probe_timer);
	atomic_set(&fc_db.wanAccessLimit.timer_activity, 0);
	fc_db.wanAccessLimit.portMask=wan_access_limit.portMask;
	fc_db.wanAccessLimit.wlanMask=0LL;
	rtk_fc_wlan_rtmbssidMask_to_devMask(wan_access_limit.wlanMask, &fc_db.wanAccessLimit.wlanMask);
	fc_db.wanAccessLimit.accessLimitNumber=wan_access_limit.accessLimitNumber;
	fc_db.wanAccessLimit.limitField=wan_access_limit.limitField;
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return SUCCESS;
}

int rtk_fc_burst_packet_set(rt_misc_burst_packet_t burst_packet)
{
	FC_PARAM_CHK(((burst_packet.pktData == NULL)&&(burst_packet.pktLen!=0)), RTK_FC_RET_ERR_NULL_POINTER);
	FC_PARAM_CHK((burst_packet.pktLen>RT_MISC_JUMBO_BUF_SIZE),RTK_FC_RET_ERR_BUF_OVERFLOW);
	FC_PARAM_CHK((burst_packet.burstRate>100), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	FC_PARAM_CHK((burst_packet.destPort>=RT_PORT_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if(burst_packet.destPort==RT_PORT_PON)FC_PARAM_CHK((burst_packet.dest_stream_id>=128), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	
	if(burst_packet.burstCount==0){
		struct tx_info txInfo;
		memset(&txInfo,0,sizeof(txInfo));

#ifdef CONFIG_GMAC1_USABLE
		txInfo.opts3.bit.gmac_id=1;	//gmac1		
#endif
		re8686_send_with_txInfo_and_mask_burst(NULL,0,&txInfo,0,NULL,0,0);
	}else{
		struct tx_info txInfo,txInfoMask;
		
		memset(&txInfo,0,sizeof(txInfo));
		memset(&txInfoMask,0,sizeof(txInfoMask));

		txInfoMask.opts1.bit.data_length=0x1ffff;
		txInfoMask.opts2.bit.tx_portmask=0x7ff; 
		txInfoMask.opts2.bit.cputag=1;
		txInfoMask.opts3.bit.keep=1;
		txInfoMask.opts3.bit.l34_keep=1;	
		txInfoMask.opts3.bit.dislrn=1;
		txInfoMask.opts3.bit.gmac_id=3;

		txInfo.opts1.bit.data_length=burst_packet.pktLen;
		txInfo.opts2.bit.cputag=1;	
		
		if(burst_packet.destPort==RT_PORT_PON)
		{
			int real_port = 0;
			
			if( rtk_switch_phyPortId_get(RTK_PORT_PON, &real_port)==RT_ERR_OK) // RTK_FC_MAC_PORT_PON = 4 in 03cvd, RTK_FC_MAC_PORT_PON = 5 in 07c,
			{
				txInfoMask.opts3.bit.cputag_psel=1;
				txInfoMask.opts3.bit.tx_dst_stream_id=0x7f;

				txInfo.opts3.bit.cputag_psel=1;
				txInfo.opts3.bit.tx_dst_stream_id=burst_packet.dest_stream_id;		

				burst_packet.destPort = real_port;
			}
		}
		txInfo.opts2.bit.tx_portmask=0x1<<burst_packet.destPort;
		txInfo.opts3.bit.keep=1;
		txInfo.opts3.bit.l34_keep=1;
		txInfo.opts3.bit.dislrn=1;
#ifdef CONFIG_GMAC1_USABLE
		txInfo.opts3.bit.gmac_id=1;	//gmac1
#endif
		re8686_send_with_txInfo_and_mask_burst(burst_packet.pktData,burst_packet.pktLen,&txInfo,0,&txInfoMask,burst_packet.burstCount,burst_packet.burstRate);
	}

#elif defined(CONFIG_RTK_L34_G3_PLATFORM)

	{
		rtk_fc_ret_t ret = RTK_FC_RET_OK;
		aal_l2_te_shaper_tbc_cfg_msk_t profile_msk={0};
		aal_l2_te_shaper_tbc_cfg_t     profile={0};
		//L2FE_ARB_ARB_CTRL_t l2fe_arb_ctrl;
		uint32 lspid = AAL_LPORT_MC;
		rtk_fc_burstPktSend_conf_t conf={0};

		conf.sw_mcgid = sw_mcgid;
		conf.hw_mcgid = hw_mcgid;
		conf.pkt_lspid = lspid;
		
		if(burst_packet.burstCount==0){
			// disabledMode to stop packet replication
			goto CONFIG_CLEAR;
			
		}else if(burst_packet.burstCount==-1){
			// infiniteMode
			uint32 ldpid = 0, cos = 0, swid = 0;
			uint32 rate_m = (BURST_10G_MBPS * burst_packet.burstRate) /100;
			uint32 config_rate_m = rate_m >> 1;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
			portForwardActionVlanExtend_t mcFib={0};
#else
			portForwardAction_t mcFib={0};
#endif

			if(hw_mcgid!=CA_UINT16_INVALID && (sw_mcgid!=CA_UINT16_INVALID)) {
				WARNING("burst traffic is still running, stop it now.");
				goto CONFIG_CLEAR;
			}
#if !defined(CONFIG_FC_G3_G3LITE_SERIES)
			if(burst_packet.destPort==RT_PORT_PON && 
				fc_db.streamidTbl[burst_packet.dest_stream_id].valid) {

				ldpid = fc_db.streamidTbl[burst_packet.dest_stream_id].ldpid;
				cos = fc_db.streamidTbl[burst_packet.dest_stream_id].cos;
				swid = fc_db.streamidTbl[burst_packet.dest_stream_id].gemid;
			}else
#endif
			{
				ldpid = burst_packet.destPort;
				cos = 0;
				swid = 0;
			}

			// MC FIB
			{
				mcFib.ldpid = ldpid;
				mcFib.cos = cos;
				mcFib.cos_en = TRUE;
				mcFib.wifi_flowid = swid;
				mcFib.src_port_filter_dis = FALSE;
				if((ret = aal_sw_mcgid_alloc(&sw_mcgid)) != AAL_E_OK) {
					goto CONFIG_CLEAR;
				}
				// 1st replicated packet 
				aal_port_forward_action_add(sw_mcgid, &mcFib);

				// 2nd replicated packet 				
				mcFib.src_port_filter_dis = TRUE;
				aal_port_forward_action_add(sw_mcgid, &mcFib);

				
				// 3rd replicated packet for looping
				mcFib.ldpid = AAL_LPORT_L3_LAN;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
				mcFib.wifi_flowid = RTK_FC_WIFI_HWLOOKUP_ID_BURST_PKT;
#else
				mcFib.wifi_flowid = 0;
#endif
				mcFib.src_port_filter_dis = TRUE;
				aal_port_forward_action_add(sw_mcgid, &mcFib);
				
				if((ret = aal_hw_mcgid_new(sw_mcgid, &hw_mcgid)) != AAL_E_OK) {
					goto CONFIG_CLEAR;
				}

				// store config
				conf.sw_mcgid = sw_mcgid;
				conf.hw_mcgid = hw_mcgid;

				
				//l2fe_arb_ctrl.wrd = rtk_ne_reg_read(L2FE_ARB_ARB_CTRL);
				//l2fe_arb_ctrl.bf.keep_o_lspid_en_for_mc = 1;
				//rtk_ne_reg_write(l2fe_arb_ctrl.wrd, L2FE_ARB_ARB_CTRL);
			}

			// L2TE MC port rate control
			{
				memset((void *)&profile, 0, sizeof(profile));
				memset((void *)&profile_msk, 0, sizeof(profile_msk));

				/* configure L2 traffic shaper */
				profile_msk.s.bs     = 1;
				profile_msk.s.rate_k = 1;
				profile_msk.s.rate_m = 1;
				profile_msk.s.mode   = 1;
				profile_msk.s.state  = 1;

				profile.bs     = ((config_rate_m + 511) >> 9) + 1;
				profile.rate_k = 0;
				profile.rate_m = config_rate_m;
				profile.mode   = 0; //CA_AAL_SHAPER_MODE_BPS
				profile.state  = 2; //CA_AAL_SHAPER_ADMIN_STATE_SHAPER;

				if (aal_l2_te_shaper_port_tbc_set(0, AAL_PPORT_MC, profile_msk, &profile) != CA_E_OK)
				{
					WARNING("fail to config l2 te port shaper");
					goto CONFIG_CLEAR;
				}
			}
			//L3CLS - ACL
			{
				if((ret = _rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_BURST_PACKET_SEND_MCE, &conf)) != RTK_FC_RET_OK)
					goto CONFIG_CLEAR;
					
			}

			API("enable MCE swmcgid %d hwmcgid %d", sw_mcgid, hw_mcgid);

			// CPU TX
			{
				int sendpkt = 0;
				struct sk_buff *skb = NULL;
				struct rt_skbuff *rtskb, rtNewSkb;
				ca_ni_tx_config_t tx_config={0};
				
				// if send 1 pkt, get 7xx mbps only
				for(sendpkt = 0 ; sendpkt<20; sendpkt++) {
					skb = dev_alloc_skb(burst_packet.pktLen);
					
					RTK_FC_HOOK_PS_SKB_DEV_ALLOC_SKB(burst_packet.pktLen, &skb);
					if(skb==NULL) {
						WARNING("oom? fail to alloc skb");
						goto CONFIG_CLEAR;
					}
					rtskb = &rtNewSkb;
					RTK_FC_HOOK_CONVERTER_SKB(skb, rtskb);
				
					memcpy(RTSKB_DATA(rtskb), burst_packet.pktData, burst_packet.pktLen);
					RTK_FC_HOOK_PS_SKB_SKB_PUT(RTSKB_SKB(rtskb), burst_packet.pktLen, &RTSKB_DATA(rtskb));

					tx_config.core_config.bf.is_from_ca_tx = TRUE;
					tx_config.core_config.bf.bypass_fwd_engine = TRUE;
					tx_config.bypass_lso = TRUE;
					tx_config.core_config.bf.lspid = lspid; //((burst_packet.destPort==RT_PORT_PON)) ? AAL_LPORT_CPU_1 : AAL_LPORT_CPU_0;
					tx_config.core_config.bf.ldpid = AAL_LPORT_L3_LAN; // AAL_LPORT_MC
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
					tx_config.core_config.bf.flow_id_set = TRUE;
					tx_config.flow_id = RTK_FC_WIFI_HWLOOKUP_ID_BURST_PKT;
#endif


					API("NIC tx skb len %d from lspid 0x%x", burst_packet.pktLen, tx_config.core_config.bf.lspid);
			
					ret = nic_egress_start_xmit_for_fc_wifi_hw_lookup(skb, NULL, &tx_config);
				}
			}
			
		}else{
			// not support
			return RTK_FC_RET_ERR_CHIP_NOT_SUPPORT;
		}

		
		return SUCCESS;
		
CONFIG_CLEAR:

		API("clear related config...");
		
		//l2fe_arb_ctrl.wrd = rtk_ne_reg_read(L2FE_ARB_ARB_CTRL);
		//l2fe_arb_ctrl.bf.keep_o_lspid_en_for_mc = 0;
		//rtk_ne_reg_write(l2fe_arb_ctrl.wrd, L2FE_ARB_ARB_CTRL);
		
		// CLERA MC FIB
		if(hw_mcgid!=CA_UINT16_INVALID && (sw_mcgid!=CA_UINT16_INVALID)) {
			aal_hw_mcgid_free(sw_mcgid,hw_mcgid);
			hw_mcgid = CA_UINT16_INVALID;
		}
		if(sw_mcgid!=CA_UINT16_INVALID) {
			aal_sw_mcgid_free(sw_mcgid);
			sw_mcgid = CA_UINT16_INVALID;
		}

		// CLEAR MC PORT SHAPER
		profile_msk.s.state  = 1;
		profile.state  = 0; //CA_AAL_SHAPER_ADMIN_STATE_FORWARD;

		if (aal_l2_te_shaper_port_tbc_set(0, AAL_PPORT_MC, profile_msk, &profile) != CA_E_OK)
		{
			WARNING("fail to disable l2 te port shaper");
		}
		
		_rtk_rg_aclAndCfReservedRuleDel(RTK_FC_ACLANDCF_RESERVED_BURST_PACKET_SEND_MCE);
		
	}
#endif
	
	return SUCCESS;
}

int rtk_fc_wanPhyPortMask_set(rt_misc_wan_phyPortMask_t wan_portMask)
{
	API("wan_portMask 0x%x", wan_portMask.mask);
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	fc_db.wanPortMask.portmask = wan_portMask.mask;
	RTK_FC_HELPER_MGR_WAN_PORT_MASK_SET(fc_db.wanPortMask.portmask);
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	return SUCCESS;
}

int rtk_fc_wanPhyPortMask_get(rt_misc_wan_phyPortMask_t *pWan_portMask)
{
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	pWan_portMask->mask = fc_db.wanPortMask.portmask;
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	API("wan_portMask 0x%x", pWan_portMask->mask);
	return SUCCESS;
}

int rtk_fc_wanFastFwdDev_add(char *dev_name)
{
	rtk_fc_wlan_devidx_t wlanDevIdx;
	rtk_fc_wlan_devmask_t wlanDevIdMask;
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	if(RTK_FC_HELPER_WLAN_DEVNAME_TO_DEVID(dev_name, &wlanDevIdx, &wlanDevIdMask) == SUCCESS)
		fc_db.wanPortWifidevMask |= wlanDevIdMask;
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	if(wlanDevIdx != RTK_FC_WLANX_NOT_FOUND)
	{
		API("Add dev %s to WAN (wlanDevIdx is %d)", dev_name, wlanDevIdx);
		return SUCCESS;
	}
	else
	{
		API("dev %s is not fastforward dev", dev_name);
		return RTK_FC_RET_ERR_ENTRY_NOT_FOUND;
	}
}

int rtk_fc_wanFastFwdDev_del(char *dev_name)
{
	rtk_fc_wlan_devidx_t wlanDevIdx;
	rtk_fc_wlan_devmask_t wlanDevIdMask;
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	if(RTK_FC_HELPER_WLAN_DEVNAME_TO_DEVID(dev_name, &wlanDevIdx, &wlanDevIdMask) == SUCCESS)
	{
		fc_db.wanPortWifidevMask &= ~wlanDevIdMask;
	}
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	if(wlanDevIdx != RTK_FC_WLANX_NOT_FOUND)
	{
		API("Delete dev %s to WAN (wlanDevIdx is %d)", dev_name, wlanDevIdx);
		return SUCCESS;
	}
	else
	{
		API("dev %s is not fastforward dev", dev_name);
		return RTK_FC_RET_ERR_ENTRY_NOT_FOUND;
	}
}



int rtk_fc_vlanGroupLrnLimit_add(int port, int groupLimit, int *pGroupIdx)
{
	int i;

	if(port<0||port>=RTK_FC_MAC_PORT_MAX)return (RTK_FC_RET_ERR);
	if(groupLimit<0)return (RTK_FC_RET_ERR);
	if(pGroupIdx==NULL)return (RTK_FC_RET_ERR_NULL_POINTER);
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	for(i=0;i<MAX_VLAN_GROUP_MAC_LIMIT_NUMBER;i++)
	{
		if(!fc_db.vlanGroupMACLimit_group[i].group_info.valid)
			break;
	}
	if(i==MAX_VLAN_GROUP_MAC_LIMIT_NUMBER)
	{
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
		return RTK_FC_RET_ERR_ENTRY_FULL;
	}
	//init group
	bzero(&fc_db.vlanGroupMACLimit_group[i], sizeof(rtk_fc_vlanGroupMacLimit_group_t));
	fc_db.vlanGroupMACLimit_group[i].group_info.valid=1;
	fc_db.vlanGroupMACLimit_group[i].group_info.port=port;
	fc_db.vlanGroupMACLimit_group[i].group_info.mac_limit_number=groupLimit;
	atomic_set(&fc_db.vlanGroupMACLimit_group[i].group_info.mac_count, 0);
	INIT_LIST_HEAD(&fc_db.vlanGroupMACLimit_group[i].mac_head);
	fc_db.vlanGroupMACLimit_group_is_set =1;

	*pGroupIdx=i;
	//printk("*pGroupIdx = %d\n",*pGroupIdx);
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return RTK_FC_RET_OK;
}



int rtk_fc_vlanGroupLrnLimit_del(int groupIdx, int vlanId)
{
	rtk_fc_vlanGroupMacLimit_mac_t *pMacEntry,*pMacNextEntry;
	int i, group_is_set = 0;
	if(groupIdx<0||groupIdx>=MAX_VLAN_GROUP_MAC_LIMIT_NUMBER)return (RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	if(vlanId<FAIL-1||vlanId==0||vlanId>=MAX_VLAN_HW_TABLE_SIZE)return (RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	if(!fc_db.vlanGroupMACLimit_group[groupIdx].group_info.valid)
	{
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
		return (RTK_FC_RET_ERR_ENTRY_NOT_FOUND);
	}

	//travel all mac under this group, release vlan-matched entries or delete them all.
	if(!list_empty(&fc_db.vlanGroupMACLimit_group[groupIdx].mac_head))
	{
		list_for_each_entry_safe(pMacEntry, pMacNextEntry, &fc_db.vlanGroupMACLimit_group[groupIdx].mac_head, group_list)
		{
			if(vlanId==FAIL-1||pMacEntry->vlanId==vlanId)
			{
				atomic_dec(&fc_db.vlanGroupMACLimit_group[groupIdx].group_info.mac_count);
				list_del(&pMacEntry->group_list);
				list_del(&pMacEntry->mac_list);
				RTK_FC_HELPER_FC_KFREE(pMacEntry, sizeof(rtk_fc_vlanGroupMacLimit_mac_t));
			}
		}
	}

	if(vlanId==FAIL-1)		//clear all vlan in this group
	{
		bzero(&fc_db.vlanGroupMACLimit_group[groupIdx].group_info,sizeof(rtk_fc_vlanGroupMacLimit_info_t));
		INIT_LIST_HEAD(&fc_db.vlanGroupMACLimit_group[groupIdx].mac_head);
	}
	else if(vlanId==FAIL)
		fc_db.vlanGroupMACLimit_group[groupIdx].group_info.untag=0;
	else
		rtk_fc_clear_bit(vlanId&0x1f,(void *)&fc_db.vlanGroupMACLimit_group[groupIdx].group_info.vlanMask[vlanId>>5]);

	for(i = 0 ; i < MAX_VLAN_GROUP_MAC_LIMIT_NUMBER ; i++)
	{
		if(fc_db.vlanGroupMACLimit_group[i].group_info.valid)
		{
			group_is_set = 1;
			break;
		}
	}
	if(!group_is_set)
		fc_db.vlanGroupMACLimit_group_is_set = 0;
		
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return RTK_FC_RET_OK;
}


int rtk_fc_vlanGroupLrnLimit_set(int groupIdx, int vlanId, int groupLimit)
{
	int i;

	if(groupIdx<0||groupIdx>=MAX_VLAN_GROUP_MAC_LIMIT_NUMBER)return (RT_ERR_RG_INDEX_OUT_OF_RANGE);
	if(vlanId<FAIL-1||vlanId==0||vlanId>=MAX_VLAN_HW_TABLE_SIZE)return (RT_ERR_RG_INDEX_OUT_OF_RANGE);
	if(groupLimit<FAIL)return (RT_ERR_RG_INDEX_OUT_OF_RANGE);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	if(!fc_db.vlanGroupMACLimit_group[groupIdx].group_info.valid)
	{
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
		return (RT_ERR_RG_ENTRY_NOT_EXIST);
	}

	if(vlanId!=FAIL-1)
	{
		//check if the vlan had been added to other group of same port
		for(i=0;i<MAX_VLAN_GROUP_MAC_LIMIT_NUMBER;i++)
		{
			if(i!=groupIdx && fc_db.vlanGroupMACLimit_group[i].group_info.valid &&
				fc_db.vlanGroupMACLimit_group[i].group_info.port==fc_db.vlanGroupMACLimit_group[groupIdx].group_info.port)
			{
				if(vlanId==-1)
				{
					if(fc_db.vlanGroupMACLimit_group[i].group_info.untag)
					{
						API("Untag could only exist in one group of same port.");
						RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
						return (RT_ERR_RG_FAILED);
					}
				}
				else
				{
					if(rtk_fc_test_bit(vlanId&0x1f,(void *)&fc_db.vlanGroupMACLimit_group[i].group_info.vlanMask[vlanId>>5]))
					{
						API("Vlan%d could only exist in one group of same port.",vlanId);
						
						RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
						return (RT_ERR_RG_FAILED);
					}
				}
			}
		}

		if(vlanId==-1)
			fc_db.vlanGroupMACLimit_group[groupIdx].group_info.untag=1;
		else
			rtk_fc_set_bit(vlanId&0x1f,(void *)&fc_db.vlanGroupMACLimit_group[groupIdx].group_info.vlanMask[vlanId>>5]);
	}
	if(groupLimit>=0)
		fc_db.vlanGroupMACLimit_group[groupIdx].group_info.mac_limit_number=groupLimit;

	//travel all mac under this group, release them.
	if(!list_empty(&fc_db.vlanGroupMACLimit_group[groupIdx].mac_head))
	{
		rtk_fc_vlanGroupMacLimit_mac_t *pMacEntry,*pMacNextEntry;
		list_for_each_entry_safe(pMacEntry, pMacNextEntry, &fc_db.vlanGroupMACLimit_group[groupIdx].mac_head, group_list)
		{
			atomic_dec(&fc_db.vlanGroupMACLimit_group[groupIdx].group_info.mac_count);
			list_del(&pMacEntry->group_list);
			list_del(&pMacEntry->mac_list);
			RTK_FC_HELPER_FC_KFREE(pMacEntry, sizeof(rtk_fc_vlanGroupMacLimit_mac_t));
		}
	}

	//clear up all mac entry of the port.
	_rtk_fc_lut_linkdown_del(fc_db.vlanGroupMACLimit_group[groupIdx].group_info.port);
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return RTK_FC_RET_OK;
}


int rtk_fc_vlanGroupLrnLimit_get(int groupIdx, rt_misc_vlanGroupMacLimit_info_t *pGroupInfo)
{
	if(pGroupInfo==NULL)return (RT_ERR_RG_NULL_POINTER);
	if(groupIdx<0||groupIdx>=MAX_VLAN_GROUP_MAC_LIMIT_NUMBER)return (RT_ERR_RG_INDEX_OUT_OF_RANGE);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	if(!fc_db.vlanGroupMACLimit_group[groupIdx].group_info.valid){
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
		return (RT_ERR_RG_ENTRY_NOT_EXIST);
	}

	memcpy(pGroupInfo,&fc_db.vlanGroupMACLimit_group[groupIdx].group_info,sizeof(rt_misc_vlanGroupMacLimit_info_t));
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return RT_ERR_RG_OK;
}


int rtk_fc_vlanGroupLrnLimit_find(int port, int vlanId, int *pGroupIdx)
{
	int i,groupExist=0;

	if(port<0||port>=RTK_FC_MAC_PORT_MAX)return (RT_ERR_RG_INVALID_PARAM);
	if(vlanId<-1||vlanId==0||vlanId>=MAX_VLAN_HW_TABLE_SIZE)return (RT_ERR_RG_INDEX_OUT_OF_RANGE);
	if(pGroupIdx==NULL)return (RT_ERR_RG_NULL_POINTER);

	//RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	//find the vlan of port controlled by which group
	for(i=0;i<MAX_VLAN_GROUP_MAC_LIMIT_NUMBER;i++)
	{
		if(fc_db.vlanGroupMACLimit_group[i].group_info.valid &&
			fc_db.vlanGroupMACLimit_group[i].group_info.port==port)
		{
			if(vlanId==-1)
			{
				if(fc_db.vlanGroupMACLimit_group[i].group_info.untag)
				{
					*pGroupIdx=i;
					//RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
					return RT_ERR_RG_OK;
				}
			}
			else if(rtk_fc_test_bit(vlanId&0x1f,(void *)&fc_db.vlanGroupMACLimit_group[i].group_info.vlanMask[vlanId>>5]))
			{
				*pGroupIdx=i;
				//RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
				return RT_ERR_RG_OK;
			}
			groupExist=1;
		}
	}
	if(groupExist)
	{
		//RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
		return RT_ERR_RG_ENTRY_NOT_FOUND; //has group, but not match
	}
	else
	{
		//RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
		return RT_ERR_RG_ENTRY_NOT_EXIST;
	}
}


int rtk_fc_igmp_mcflowCheck(rt_igmp_multicast_group_cfg_t* pmcConfig) 
{
	
	int32 ret;

	API("check multicast flow");
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	ret =_rtk_fc_igmp_mc_set_to_dirver(pmcConfig);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	
	return ret;
}



int rtk_fc_igmp_mcGroupDevSet(rt_igmp_group_devPort_cfg_t *mcDevConfig)
{
	int32 ret;

	API("set multicast group Dev");

	if(unlikely(fc_db.mcapiMode==RTK_MC_MODE_INIT))
		fc_db.mcapiMode=RTK_MC_MODE_SET_BY_PERDEVPORT;
	if(fc_db.mcapiMode!=RTK_MC_MODE_SET_BY_PERDEVPORT)
		WARNING("PlEASE DO NOT USE THIS API ");
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	ret =_rtk_fc_igmp_mc_set_dev_to_dirver(mcDevConfig);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	
	return ret;
}

int rtk_fc_igmp_mcGroupDevGet(rt_igmp_group_devPort_cfg_t *pMcDevConfig)
{
	int32 ret;

	API("get multicast group Dev");

	if(unlikely(fc_db.mcapiMode==RTK_MC_MODE_INIT))
		fc_db.mcapiMode=RTK_MC_MODE_SET_BY_PERDEVPORT;
	if(fc_db.mcapiMode!=RTK_MC_MODE_SET_BY_PERDEVPORT)
		WARNING("PlEASE DO NOT USE THIS API ");

	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	ret =_rtk_fc_igmp_mc_get_devConfig(pMcDevConfig);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	
	return ret;
}





int rtk_fc_igmp_mcGroupSet(rt_igmp_multicast_group_cfg_t mcConfig)
{
	int32 ret;

	API("set multicast group");

	if(unlikely(fc_db.mcapiMode==RTK_MC_MODE_INIT))
		fc_db.mcapiMode=RTK_MC_MODE_SET_BY_PORTMASK;
	if(fc_db.mcapiMode!=RTK_MC_MODE_SET_BY_PORTMASK)
		WARNING("PlEASE DO NOT USE THIS API ");

	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	ret =_rtk_fc_igmp_mc_set_to_dirver(&mcConfig);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	
	return ret;
}


int rtk_fc_igmp_unknownMulticast_action(rt_igmp_unknownMc_cfg_t mcUnknCfg)
{

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	rtk_fc_igmp_unknownMulticast_action_set(mcUnknCfg);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	
	return SUCCESS;

}

int rtk_fc_igmp_check_userGroup(rtk_igmp_userGroup_cfg_t *userGroup)
{
#if defined(CONFIG_RTK_SOC_RTL8198D)
	if(rtk_fc_get_user_group(userGroup->group, userGroup->isIpv6)== 0)
		userGroup->valid =1;
#endif
	return SUCCESS;
}

int rtk_fc_igmp_mcMode(rt_igmp_multicast_learning_mode_t mcMode)
{
	int32  oriMode;
	
	API("set multicast mode");

	oriMode=fc_db.mcSetMode;

	if(mcMode <0 || mcMode>=RT_IGMP_MULTICAST_END)
		return FAILED;

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	fc_db.mcSetMode = mcMode;

	if(fc_db.mcSetMode !=oriMode)
	{
		rt_igmp_group_devPort_cfg_t sw_mcConfig;
		//flush all
		bzero(&sw_mcConfig,sizeof(sw_mcConfig));
		_rtk_fc_igmp_mc_set_dev_to_dirver(&sw_mcConfig);
	}
	
	if(mcMode == RT_IGMP_MULTICAST_SYNC_MODE_KERNEL)
		rtk_fc_igmp_setupKernelSyncTimerPeriod();

	if(mcMode == RT_IGMP_MULTICAST_SYNC_AUTO)
	{
		if(!RTK_FC_HELPER_IGMP_IS_MDB_REG())
			WARNING("please insert igmpHook module");
#if !defined(CONFIG_RTK_IGMP_MLD_SNOOPING_MODULE)
		WARNING("please enable CONFIG_RTK_IGMP_MLD_SNOOPING_MODULE");
#endif
	}
	else
	{
		if(RTK_FC_HELPER_IGMP_IS_MDB_REG())
		{
			WARNING("please remove module or disable CONFIG_RTK_IGMP_MLD_SNOOPING_MODULE to reduce memory using");
		}
	}

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();	


	if(mcMode == RT_IGMP_MULTICAST_SYNC_AUTO)
		rtk_fc_user_pipe_cmd("echo IGMPHOOK enable > proc/igmp/ctrl/igmphook");
	else
		rtk_fc_user_pipe_cmd("echo IGMPHOOK disable > proc/igmp/ctrl/igmphook");


	return SUCCESS;

}

#if defined(CONFIG_FC_RTL8277C_SERIES)
//TODO: 9607F flow cache mib
int32 rtk_fc_flowCache_mib_get ( uint32 flow_idx, rt_stat_flowCache_mib_t* pFlowCacheMib )
{
	int aal_ret, flow_cache_idx=SIGNED_INVALID;
	l3fe_hash_aqm_flow_mib_tbl_entry_t aqm_flow_mib_entry;
	
	API("Get flow cache mib, flow_idx[%d]", flow_idx);

	FC_PARAM_CHK((flow_idx >= RT_STAT_FLOW_TABLE_SIZE), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	FC_PARAM_CHK((pFlowCacheMib == NULL), RTK_FC_RET_ERR_NULL_POINTER);	

	memset(pFlowCacheMib, 0, sizeof(rt_stat_flowCache_mib_t));
	//get flow cache index
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	if(fc_db.flowTbl[flow_idx].pFlowEntry->path1.valid && fc_db.flowTbl[flow_idx].pFlowEntry->path1.flow_cache_mib_en)
		flow_cache_idx = fc_db.flowTbl[flow_idx].pFlowEntry->path1.flow_cache_mib_idx;
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	if(flow_cache_idx==SIGNED_INVALID)
	{
		WARNING("flow cache mib of flow_idx[%d] is not found.", flow_idx);
		return RTK_FC_RET_ERR_ENTRY_NOT_FOUND;
	}
	
	/* get HW counter */
	memset(&aqm_flow_mib_entry, 0, sizeof(aqm_flow_mib_entry));
	aal_ret = aal_l3fe_hash_aqm_flow_mib_entry_get(flow_cache_idx, &aqm_flow_mib_entry);
	if(aal_ret)
	{
		WARNING("Fail to get aqm flow mib entry[%d], ret = %d", flow_cache_idx, aal_ret);
		return RTK_FC_RET_ERR;
	}
	/* return HW counter */
	pFlowCacheMib->packet_count	= aqm_flow_mib_entry.pktCnt;
	pFlowCacheMib->byte_count 	= aqm_flow_mib_entry.byteCnt;

	return SUCCESS;
}


int rtk_fc_flowCache_mib_reset ( uint32 flow_idx )
{
	int aal_ret, flow_cache_idx=SIGNED_INVALID;
	l3fe_hash_aqm_flow_mib_tbl_entry_t aqm_flow_mib_entry;
	
	API("Clear flow cache mib, flow_idx[%d]", flow_idx);

	FC_PARAM_CHK((flow_idx >= RT_STAT_FLOW_TABLE_SIZE), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	//get flow cache index
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	if(fc_db.flowTbl[flow_idx].pFlowEntry->path1.valid && fc_db.flowTbl[flow_idx].pFlowEntry->path1.flow_cache_mib_en)
		flow_cache_idx = fc_db.flowTbl[flow_idx].pFlowEntry->path1.flow_cache_mib_idx;
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	if(flow_cache_idx==SIGNED_INVALID)
	{
		WARNING("flow cache mib of flow_idx[%d] is not found.", flow_idx);
		return RTK_FC_RET_ERR_ENTRY_NOT_FOUND;
	}

	/*clear HW counter*/
	memset(&aqm_flow_mib_entry, 0, sizeof(aqm_flow_mib_entry));
	aal_ret = aal_l3fe_hash_aqm_flow_mib_entry_set(flow_cache_idx, &aqm_flow_mib_entry);
	if(aal_ret)
	{
		WARNING("Fail to set aqm flow mib entry[%d], ret = %d", flow_cache_idx, aal_ret);
		return RTK_FC_RET_ERR;
	}

	return SUCCESS;
}
#endif

int rtk_fc_flowMib_get(int idx, rt_stat_flow_mib_t *pflowMibCnt)
{
	rtk_fc_flowOrHostmib_counter_t hwCounter;
	API("get flowMib[%d]", idx);

	FC_PARAM_CHK((idx<0)||(idx >= RT_STAT_FLOWMIB_TABLE_SIZE), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	FC_PARAM_CHK((pflowMibCnt == NULL), RTK_FC_RET_ERR_NULL_POINTER);


	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	memset(pflowMibCnt, 0, sizeof(rt_stat_flow_mib_t));

	/*get HW counter*/
	_rtk_fc_hwFlowMib_get(idx, &hwCounter);

	/*return HW counter + SW counter*/
	pflowMibCnt->ingress_packet_count = hwCounter.in_packet_cnt +  _rtk_fc_sw_flowMib_all_cpu_get(idx, IN_PKT_CNT);
	pflowMibCnt->ingress_byte_count = hwCounter.in_byte_cnt +  _rtk_fc_sw_flowMib_all_cpu_get(idx, IN_BYTE_CNT);
	pflowMibCnt->egress_packet_count = hwCounter.out_packet_cnt +  _rtk_fc_sw_flowMib_all_cpu_get(idx, OUT_PKT_CNT);
	pflowMibCnt->egress_byte_count = hwCounter.out_byte_cnt + _rtk_fc_sw_flowMib_all_cpu_get(idx, OUT_BYTE_CNT);
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return SUCCESS;
}

int rtk_fc_flowMib_clear(int idx)
{
	API("clear flowMib[%d]", idx);

	FC_PARAM_CHK((idx<0)||(idx >= RT_STAT_FLOWMIB_TABLE_SIZE), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	/*clear HW counter*/
	_rtk_fc_hwFlowMib_clear(idx);

	/*clear SW counter*/
	memset(&fc_db.flowSWMib, 0 , sizeof(fc_db.flowSWMib));

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return SUCCESS;
}

int rtk_fc_hostPoliceControl_set(int idx, rt_rate_host_policer_control_t hostPoliceControl)
{
	int i;
	rtk_mac_t tempMac, oriMac, zeroMac;
	int ingress_limit_meter_hwIndex = 0, egress_limit_meter_hwIndex = 0;
	bool ingress_limit_by_swShaper = FALSE, egress_limit_by_swShaper = FALSE;

	API("set hostPoliceControl[%d]", idx);

	FC_PARAM_CHK((idx<0 || idx>=RT_RATE_HOSTPOLICING_TABLE_SIZE), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	/*get hw meter index*/
	if(fc_db.controlFuc.rt_api_is_enabled)
	{
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
		int meterIdxHwRange_min; //initial below
		int meterIdxHwRange_max; //initial below
#elif defined(CONFIG_FC_CA8277B_SERIES)
		//8277B not support HW host policing
#else
		int meterIdxHwRange_min = 0;
		int meterIdxHwRange_max = (G3_L3_FLOW_POLICER_SIZE - G3_L3_FLOW_POLICER_SIZE_MIB)-1;
#endif
#elif defined (CONFIG_RTK_L34_XPON_PLATFORM)
		int meterIdxHwRange_min = 0;
		int meterIdxHwRange_max = RTK_FC_TABLESIZE_SHAREMTR-1;
#endif
		int32 ret;
		rt_rate_meter_mapping_t meterMapping;

#if defined(CONFIG_FC_G3_G3LITE_SERIES)
		if(fc_db.controlFuc.loopbackMode_is_enabled)
		{
			//loopback mode: L2 policer
			meterIdxHwRange_min = 0;
			meterIdxHwRange_max = CA_AAL_MAX_FLOW_ID;
		}
		else
		{
			//flow_policer[0:31] reserved for flow meter
			meterIdxHwRange_min = 32;
			meterIdxHwRange_max = (G3_L3_FLOW_POLICER_SIZE - G3_L3_FLOW_POLICER_SIZE_MIB)-1;
		}
#endif
		if(hostPoliceControl.ingress_limit_control)
		{
			//ret = rt_rate_shareMeterMappingHw_get(hostPoliceControl.ingress_limit_meter_index, &meterMapping);
			ret = RTK_FC_HELPER_RT_RATE_SHAREMETER_MAPPING_HW_GET(hostPoliceControl.ingress_limit_meter_index, &meterMapping);
			if((ret == 0) && (meterMapping.type == RT_METER_TYPE_HOST))
			{
#if defined(CONFIG_FC_CA8277B_SERIES)
				WARNING("Do not support HW host rate policing");
				return RTK_FC_RET_ERR;
#else
				ingress_limit_meter_hwIndex = meterMapping.hw_index;
				//check if hw index is valid
				if((meterIdxHwRange_min > ingress_limit_meter_hwIndex) || (ingress_limit_meter_hwIndex > meterIdxHwRange_max))
				{
					WARNING("Invalid hw meter index (%d) for host ingress rate limiting (rt meter type: host)", ingress_limit_meter_hwIndex);
					return(RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
				}
#endif
			}
			else if((ret == 0) && (meterMapping.type == RT_METER_TYPE_SW))
			{
				ingress_limit_by_swShaper = TRUE;
				ingress_limit_meter_hwIndex = meterMapping.hw_index;
				if((0 > ingress_limit_meter_hwIndex) || (ingress_limit_meter_hwIndex >= RTK_FC_TABLESIZE_SW_SHAREMTR))
				{
					WARNING("Invalid hw meter index (%d) for host ingress rate limiting (rt meter type: sw)", ingress_limit_meter_hwIndex);
					return(RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
				}
			}
			else
			{
				WARNING("get l3Meter hw index Failed, return value: 0x%x, virtual meter type: %d", ret, meterMapping.type);
				return RTK_FC_RET_ERR;
			}

		}

		if(hostPoliceControl.egress_limit_control)
		{
			//ret = rt_rate_shareMeterMappingHw_get(hostPoliceControl.egress_limit_meter_index, &meterMapping);
			ret = RTK_FC_HELPER_RT_RATE_SHAREMETER_MAPPING_HW_GET(hostPoliceControl.egress_limit_meter_index, &meterMapping);
			if((ret == 0) && (meterMapping.type == RT_METER_TYPE_HOST))
			{
#if defined(CONFIG_FC_CA8277B_SERIES)
				WARNING("Do not support HW host rate policing");
				return RTK_FC_RET_ERR;
#else
				egress_limit_meter_hwIndex = meterMapping.hw_index;
				//check if hw index is valid
				if((meterIdxHwRange_min > egress_limit_meter_hwIndex) || (egress_limit_meter_hwIndex > meterIdxHwRange_max))
				{
					WARNING("Invalid meter index (%d) for host egress rate limiting (rt meter type: host)", egress_limit_meter_hwIndex);
					return(RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
				}
#endif
			}
			else if((ret == 0) && (meterMapping.type == RT_METER_TYPE_SW))
			{
				egress_limit_by_swShaper = TRUE;
				egress_limit_meter_hwIndex = meterMapping.hw_index;
				if((0 > egress_limit_meter_hwIndex) || (egress_limit_meter_hwIndex >= RTK_FC_TABLESIZE_SW_SHAREMTR))
				{
					WARNING("Invalid hw meter index (%d) for host ingress rate limiting (rt meter type: sw)", egress_limit_meter_hwIndex);
					return(RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
				}
			}
			else
			{
				WARNING("get l3Meter hw index Failed, return value: 0x%x, virtual meter type: %d", ret, meterMapping.type);
				return RTK_FC_RET_ERR;
			}

		}

	}
	else
	{
		ingress_limit_meter_hwIndex = hostPoliceControl.ingress_limit_meter_index;
		egress_limit_meter_hwIndex = hostPoliceControl.egress_limit_meter_index;

		if((0 > ingress_limit_meter_hwIndex) || (ingress_limit_meter_hwIndex >= HOST_POLICING_METER_SIZE))
		{
			WARNING("Invalid meter index (%d) for host ingress rate limiting", ingress_limit_meter_hwIndex);
			return(RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
		}
		if((0 > egress_limit_meter_hwIndex) || (egress_limit_meter_hwIndex >= HOST_POLICING_METER_SIZE))
		{
			WARNING("Invalid meter index (%d) for host egress rate limiting", egress_limit_meter_hwIndex);
			return(RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
		}
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
		if(fc_db.controlFuc.loopbackMode_is_enabled)
		{
			ingress_limit_meter_hwIndex += G3_LOOPBACKMODE_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR;
			egress_limit_meter_hwIndex += G3_LOOPBACKMODE_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR;
		}
		else
		{
			ingress_limit_meter_hwIndex += G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR;
			egress_limit_meter_hwIndex += G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR;
		}
#endif
	}

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	
	memset(zeroMac.octet, 0, sizeof(zeroMac));
	memcpy(oriMac.octet, fc_db.hostPoliceTable[idx].hostPoliceControl.mac_addr, sizeof(oriMac.octet)); //record original packet
	memcpy(tempMac.octet, hostPoliceControl.mac_addr, sizeof(tempMac.octet));

	//check if duplicate MAC exist in other host policing entries
	if(memcmp(tempMac.octet, zeroMac.octet, sizeof(tempMac.octet)))
	{
		for(i = 0 ; i < RT_RATE_HOSTPOLICING_TABLE_SIZE ; i++)
		{
			if(i == idx)
				continue;
			if(!memcmp(&fc_db.hostPoliceTable[i].hostPoliceControl.mac_addr, hostPoliceControl.mac_addr, sizeof(fc_db.hostPoliceTable[idx].hostPoliceControl.mac_addr)))
			{
				WARNING("Set host policing entry failed! The MAC %pM has been set in entry [%d].", hostPoliceControl.mac_addr, i);
				RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
				return FAILED;
			}
		}
	}
	memcpy(&fc_db.hostPoliceTable[idx].hostPoliceControl, &hostPoliceControl, sizeof(rt_rate_host_policer_control_t));
	fc_db.hostPoliceTable[idx].ingress_limit_meter_hwIndex = ingress_limit_meter_hwIndex;
	fc_db.hostPoliceTable[idx].egress_limit_meter_hwIndex = egress_limit_meter_hwIndex;
	fc_db.hostPoliceTable[idx].ingress_limit_by_swShaper = ingress_limit_by_swShaper;
	fc_db.hostPoliceTable[idx].egress_limit_by_swShaper = egress_limit_by_swShaper;
#if defined (CONFIG_RTK_L34_XPON_PLATFORM)
	if(rtk_rate_hostMacAddr_set(idx, &tempMac) != RT_ERR_OK)
	{
		WARNING("set host MAC address into HW failed!");
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
		return FAILED;
	}
	if(rtk_stat_hostState_set(idx, hostPoliceControl.mib_count_control) != RT_ERR_OK)
	{
		WARNING("set host states config to HW failed!");
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
		return FAILED;
	}
	if(ingress_limit_by_swShaper)
	{
		//use SW to do ingress rate limit, HW ingress rate limit set to disabled
		if(rtk_rate_hostIgrBwCtrlState_set(idx, DISABLED) != RT_ERR_OK)
		{
			WARNING("set host ingress bandwidth control config to HW failed!");
			RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
			return FAILED;
		}
	}
	else
	{
		if(rtk_rate_hostIgrBwCtrlState_set(idx, hostPoliceControl.ingress_limit_control) != RT_ERR_OK)
		{
			WARNING("set host ingress bandwidth control config to HW failed!");
			RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
			return FAILED;
		}
	}
	if(egress_limit_by_swShaper)
	{
		//use SW to do egress rate limit, HW egress rate limit set to disabled
		if(rtk_rate_hostEgrBwCtrlState_set(idx, DISABLED) != RT_ERR_OK)
		{
			WARNING("set host egress bandwidth control config to HW failed!");
			RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
			return FAILED;
		}
	}
	else
	{
		if(rtk_rate_hostEgrBwCtrlState_set(idx, hostPoliceControl.egress_limit_control) != RT_ERR_OK)
		{
			WARNING("set host egress bandwidth control config to HW failed!");
			RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
			return FAILED;
		}
	}
	if(rtk_rate_hostIgrBwCtrlMeterIdx_set(idx, ingress_limit_meter_hwIndex) != RT_ERR_OK)
	{
		WARNING("set host ingress bandwidth meter config to HW failed!");
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
		return FAILED;
	}
	if(rtk_rate_hostEgrBwCtrlMeterIdx_set(idx, egress_limit_meter_hwIndex) != RT_ERR_OK)
	{
		WARNING("set host egress bandwidth meter config to HW failed!");
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
		return FAILED;
	}

	if(hostPoliceControl.ingress_limit_control)
	{
		if(hostPoliceControl.ingress_limit_rate == RT_RATE_HOST_METER_RATE_UNCHANGE)
			DEBUG("ingressLimit: hw meter[%d] rate config is not changed. (ingress_limit_rate is RT_RATE_HOST_METER_RATE_UNCHANGE)", ingress_limit_meter_hwIndex);
		else
		{
			if(ingress_limit_by_swShaper)
			{
				fc_db.swMeter[ingress_limit_meter_hwIndex].rate = hostPoliceControl.ingress_limit_rate;
				fc_db.swMeter[ingress_limit_meter_hwIndex].ifgInclude = ENABLED;
			}
			else
			{
				if(rtk_rate_shareMeter_set(ingress_limit_meter_hwIndex, hostPoliceControl.ingress_limit_rate, ENABLED) == RT_ERR_OK)
					DEBUG("ingressLimit: hw meter[%d] rate config is changed to %d", ingress_limit_meter_hwIndex, hostPoliceControl.ingress_limit_rate);
				else
					DEBUG("ingressLimit: hw meter[%d] rate config is not changed. (meter set failed)", ingress_limit_meter_hwIndex);
			}
		}
	}

	if(hostPoliceControl.egress_limit_control)
	{
		if(hostPoliceControl.egress_limit_rate == RT_RATE_HOST_METER_RATE_UNCHANGE)
			DEBUG("egressLimit: hw meter[%d] rate config is not changed. (ingress_limit_rate is RT_RATE_HOST_METER_RATE_UNCHANGE)", egress_limit_meter_hwIndex);
		else
		{
			if(egress_limit_by_swShaper)
			{
				fc_db.swMeter[egress_limit_meter_hwIndex].rate = hostPoliceControl.egress_limit_rate;
				fc_db.swMeter[egress_limit_meter_hwIndex].ifgInclude = ENABLED;
			}
			else
			{
				if(rtk_rate_shareMeter_set(egress_limit_meter_hwIndex, hostPoliceControl.egress_limit_rate, ENABLED) == RT_ERR_OK)
					DEBUG("egressLimit: hw meter[%d] rate config is changed to %d", egress_limit_meter_hwIndex, hostPoliceControl.egress_limit_rate);
				else
					DEBUG("egressLimit: hw meter[%d] rate config is not changed. (meter set failed)", egress_limit_meter_hwIndex);
			}
		}
	}

#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	//mib_count_control is enabled: record its rx and tx policer index
	if(fc_db.hostPoliceTable[idx].hostPoliceControl.mib_count_control == DISABLED)
	{
		fc_db.hostPoliceTable[idx].loggingRx_policerIdx = FAIL;
		fc_db.hostPoliceTable[idx].loggingTx_policerIdx = FAIL;
	}
	else
	{
		// if ingress_limit_by_swShaper or egress_limit_by_swShaper, its flow will be add to SW, no use hw policer
		if(ingress_limit_by_swShaper)
			fc_db.hostPoliceTable[idx].loggingRx_policerIdx = FAIL;
		else
		{
			if(fc_db.hostPoliceTable[idx].hostPoliceControl.ingress_limit_control)
				fc_db.hostPoliceTable[idx].loggingRx_policerIdx = ingress_limit_meter_hwIndex;
			else
			{
				if(fc_db.controlFuc.loopbackMode_is_enabled)
					fc_db.hostPoliceTable[idx].loggingRx_policerIdx = idx + G3_LOOPBACKMODE_FLOW_POLICER_IDXSHIFT_HPLOGRX;
				else
					fc_db.hostPoliceTable[idx].loggingRx_policerIdx = idx + G3_FLOW_POLICER_IDXSHIFT_HPLOGRX;
			}
		}
		if(egress_limit_by_swShaper)
			fc_db.hostPoliceTable[idx].loggingTx_policerIdx = FAIL;
		else
		{
			if(fc_db.hostPoliceTable[idx].hostPoliceControl.egress_limit_control)
				fc_db.hostPoliceTable[idx].loggingTx_policerIdx = egress_limit_meter_hwIndex;
			else
			{
				if(fc_db.controlFuc.loopbackMode_is_enabled)
					fc_db.hostPoliceTable[idx].loggingTx_policerIdx = idx + G3_LOOPBACKMODE_FLOW_POLICER_IDXSHIFT_HPLOGTX;
				else
					fc_db.hostPoliceTable[idx].loggingTx_policerIdx = idx + G3_FLOW_POLICER_IDXSHIFT_HPLOGTX;
			}
		}
	}

	if(hostPoliceControl.ingress_limit_control)
	{
		if(hostPoliceControl.ingress_limit_rate == RT_RATE_HOST_METER_RATE_UNCHANGE)
			DEBUG("ingressLimit: hw meter[%d] rate config is not changed. (ingress_limit_rate is RT_RATE_HOST_METER_RATE_UNCHANGE)", ingress_limit_meter_hwIndex);
		else
		{
			if(ingress_limit_by_swShaper)
			{
				// sw meter
				fc_db.swMeter[ingress_limit_meter_hwIndex].rate = hostPoliceControl.ingress_limit_rate;
				fc_db.swMeter[ingress_limit_meter_hwIndex].ifgInclude = ENABLED;
			}
			else
			{
				if(fc_db.controlFuc.loopbackMode_is_enabled)
				{
					//L2 policer
					if(rtk_rate_shareMeter_set(ingress_limit_meter_hwIndex, hostPoliceControl.ingress_limit_rate, ENABLED) == RT_ERR_OK)
						DEBUG("ingressLimit: hw meter[%d] rate config is changed to %d", ingress_limit_meter_hwIndex, hostPoliceControl.ingress_limit_rate);
					else
						DEBUG("ingressLimit: hw meter[%d] rate config is not changed. (meter set failed)", ingress_limit_meter_hwIndex);
				}
				else
				{
					if(_rtk_fc_g3L3Policer_set(ingress_limit_meter_hwIndex, hostPoliceControl.ingress_limit_rate, ENABLED) == RT_ERR_OK)
						DEBUG("ingressLimit: hw meter[%d] rate config is changed to %d", ingress_limit_meter_hwIndex, hostPoliceControl.ingress_limit_rate);
					else
						DEBUG("ingressLimit: hw meter[%d] rate config is not changed. (meter set failed)", ingress_limit_meter_hwIndex);
				}
			}
		}
	}

	if(hostPoliceControl.egress_limit_control)
	{
		if(hostPoliceControl.egress_limit_rate == RT_RATE_HOST_METER_RATE_UNCHANGE)
			DEBUG("egressLimit: hw meter[%d] rate config is not changed. (ingress_limit_rate is RT_RATE_HOST_METER_RATE_UNCHANGE)", egress_limit_meter_hwIndex);
		else
		{
			if(egress_limit_by_swShaper)
			{
				// sw meter
				fc_db.swMeter[egress_limit_meter_hwIndex].rate = hostPoliceControl.egress_limit_rate;
				fc_db.swMeter[egress_limit_meter_hwIndex].ifgInclude = ENABLED;
			}
			else
			{
				if(fc_db.controlFuc.loopbackMode_is_enabled)
				{
					//L2 policer
					if(rtk_rate_shareMeter_set(egress_limit_meter_hwIndex, hostPoliceControl.egress_limit_rate, ENABLED) == RT_ERR_OK)
						DEBUG("egressLimit: hw meter[%d] rate config is changed to %d", egress_limit_meter_hwIndex, hostPoliceControl.egress_limit_rate);
					else
						DEBUG("egressLimit: hw meter[%d] rate config is not changed. (meter set failed)", egress_limit_meter_hwIndex);
				}
				else
				{
					if(_rtk_fc_g3L3Policer_set(egress_limit_meter_hwIndex, hostPoliceControl.egress_limit_rate, ENABLED) == RT_ERR_OK)
						DEBUG("egressLimit: hw meter[%d] rate config is changed to %d", egress_limit_meter_hwIndex, hostPoliceControl.egress_limit_rate);
					else
						DEBUG("egressLimit: hw meter[%d] rate config is not changed. (meter set failed)", egress_limit_meter_hwIndex);
				}
			}
		}
	}

	if(fc_db.controlFuc.loopbackMode_is_enabled)
	{
		/*loopback mode G3 use L2 CLS to achieve host policing*/
		if(memcmp(oriMac.octet, zeroMac.octet, sizeof(oriMac.octet)))
		{
			_rtk_fc_g3_l2cls_for_hostPoliceControl_del(idx);
		}

		// if ingress_limit_by_swShaper or egress_limit_by_swShaper, its flow will be add to SW, no use hw policer
		if(!(ingress_limit_by_swShaper || egress_limit_by_swShaper))
		{
			if(memcmp(tempMac.octet, zeroMac.octet, sizeof(tempMac.octet)))
			{
				_rtk_fc_g3_l2cls_for_hostPoliceControl_add(idx);
			}
		}
	}
#endif
	{
		/* host policing may be rate limited by HW or SW, thus flush flow after host policing setting changed*/
		if(memcmp(oriMac.octet, zeroMac.octet, sizeof(oriMac.octet)))
		{
			int16 lutIdx;
			if(_rtk_fc_lut_find(oriMac.octet, &lutIdx) == RTK_FC_RET_OK)
			{
				DEBUG("Clear flow by hostPolControl[%d] original MAC %pM", idx, oriMac.octet);
				if(rtk_fc_flow_delete_by_l2Idx(lutIdx) != RTK_FC_RET_OK)
					WARNING("Delete flow by lut index %d failed", lutIdx);
			}
		}
		if(memcmp(tempMac.octet, zeroMac.octet, sizeof(tempMac.octet)))
		{
			int16 lutIdx;
			if(_rtk_fc_lut_find(tempMac.octet, &lutIdx) == RTK_FC_RET_OK)
			{
				DEBUG("Clear flow by hostPolControl[%d] original MAC %pM", idx, tempMac.octet);
				if(rtk_fc_flow_delete_by_l2Idx(lutIdx) != RTK_FC_RET_OK)
					WARNING("Delete flow by lut index %d failed", lutIdx);
			}
		}
	}

	_rtk_fc_hwHostPolingMib_clear(idx); //clear HW counter
	bzero(&fc_db.hostPoliceTable[idx].swHostPoliceCounter, sizeof(fc_db.hostPoliceTable[idx].swHostPoliceCounter)); // clear SW counter

	/*update host policing entry index of new and original mac setting*/
	if(memcmp(oriMac.octet, zeroMac.octet, sizeof(oriMac.octet)))
		_rtk_fc_lutHostPolicingIdxUpdate(oriMac.octet);
	if(memcmp(tempMac.octet, zeroMac.octet, sizeof(tempMac.octet)))
		_rtk_fc_lutHostPolicingIdxUpdate(tempMac.octet);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return SUCCESS;

}

int rtk_fc_hostPoliceControl_get(int idx, rt_rate_host_policer_control_t *pHostPoliceControl)
{
	rtk_enable_t ifgIncludeTmp;
	API("get hostPoliceControl[%d]", idx);
	FC_PARAM_CHK((pHostPoliceControl==NULL), RTK_FC_RET_ERR_NULL_POINTER);
	FC_PARAM_CHK((idx<0 || idx>=RT_RATE_HOSTPOLICING_TABLE_SIZE), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	
	memcpy(pHostPoliceControl, &fc_db.hostPoliceTable[idx].hostPoliceControl, sizeof(rt_rate_host_policer_control_t));

	if(fc_db.hostPoliceTable[idx].hostPoliceControl.ingress_limit_control)
	{
		if(fc_db.hostPoliceTable[idx].ingress_limit_by_swShaper)
		{
			pHostPoliceControl->ingress_limit_rate = fc_db.swMeter[fc_db.hostPoliceTable[idx].ingress_limit_meter_hwIndex].rate;
		}
		else
		{
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
			if(fc_db.controlFuc.loopbackMode_is_enabled)
				rtk_rate_shareMeter_get(fc_db.hostPoliceTable[idx].ingress_limit_meter_hwIndex, &pHostPoliceControl->ingress_limit_rate, &ifgIncludeTmp); //L2 policer
			else
				_rtk_fc_l3Meter_get(RT_RATE_EXT_METER_TYPE_HOST, fc_db.hostPoliceTable[idx].ingress_limit_meter_hwIndex, &pHostPoliceControl->ingress_limit_rate, &ifgIncludeTmp);//get L3 meter rate by hw index
#elif defined(CONFIG_RTK_L34_XPON_PLATFORM)
			rtk_rate_shareMeter_get(fc_db.hostPoliceTable[idx].ingress_limit_meter_hwIndex, &pHostPoliceControl->ingress_limit_rate, &ifgIncludeTmp); //get L2 meter rate by hw index
#endif
		}
	}
	else
	{
		pHostPoliceControl->ingress_limit_meter_index = 0;
		pHostPoliceControl->ingress_limit_rate = 0;
	}

	if(fc_db.hostPoliceTable[idx].hostPoliceControl.egress_limit_control)
	{
		if(fc_db.hostPoliceTable[idx].egress_limit_by_swShaper)
		{
			pHostPoliceControl->egress_limit_rate = fc_db.swMeter[fc_db.hostPoliceTable[idx].egress_limit_meter_hwIndex].rate;
		}
		else
		{
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
			if(fc_db.controlFuc.loopbackMode_is_enabled)
				rtk_rate_shareMeter_get(fc_db.hostPoliceTable[idx].egress_limit_meter_hwIndex, &pHostPoliceControl->egress_limit_rate, &ifgIncludeTmp); //L2 policer
			else
				_rtk_fc_l3Meter_get(RT_RATE_EXT_METER_TYPE_HOST, fc_db.hostPoliceTable[idx].egress_limit_meter_hwIndex, &pHostPoliceControl->egress_limit_rate, &ifgIncludeTmp);//get L3 meter rate by hw index
#elif defined(CONFIG_RTK_L34_XPON_PLATFORM)
			rtk_rate_shareMeter_get(fc_db.hostPoliceTable[idx].egress_limit_meter_hwIndex, &pHostPoliceControl->egress_limit_rate, &ifgIncludeTmp);//get L2 meter rate by hw index
#endif
		}
	}
	else
	{
		pHostPoliceControl->egress_limit_meter_index = 0;
		pHostPoliceControl->egress_limit_rate = 0;
	}
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return SUCCESS;
}

int rtk_fc_hostPoliceMib_get(int idx, rt_rate_host_policer_mib_t *pHostMibCnt)
{
	API("get hostPoliceMib[%d]", idx);

	FC_PARAM_CHK((pHostMibCnt==NULL), RTK_FC_RET_ERR_NULL_POINTER);
	FC_PARAM_CHK((idx<0 || idx>=RT_RATE_HOSTPOLICING_TABLE_SIZE), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	memset(pHostMibCnt, 0, sizeof(rt_rate_host_policer_mib_t));

	//get HW counter
	_rtk_fc_hwHostPolingMib_get(idx, &pHostMibCnt->rx_count, &pHostMibCnt->tx_count);

	//add SW counter
	pHostMibCnt->rx_count +=  _rtk_fc_sw_hostMib_all_cpu_get(idx, RX_BYTE_CNT);
	pHostMibCnt->tx_count +=  _rtk_fc_sw_hostMib_all_cpu_get(idx, TX_BYTE_CNT);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return SUCCESS;
}


int rtk_fc_hostPoliceMib_clear(int idx)
{
	API("clear hostPoliceMib[%d]", idx);
	FC_PARAM_CHK((idx<0 || idx>=RT_RATE_HOSTPOLICING_TABLE_SIZE), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	/*clear HW counter*/
	_rtk_fc_hwHostPolingMib_clear(idx);
	/*clear SW counter*/
	bzero(&fc_db.hostPoliceTable[idx].swHostPoliceCounter, sizeof(fc_db.hostPoliceTable[idx].swHostPoliceCounter));

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return SUCCESS;
}

int rtk_fc_netifMib_get(char *devName, rt_stat_netif_mib_t *pNetifMib)
{
	int32 i,j, ret,stackCnt;
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	rtk_rg_asic_netifMib_entry_t intfMib;
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	l3pe_cntr_tx_t tx_cntr;
	l3pe_cntr_rx_t rx_cntr;
#if defined(CONFIG_FC_CA8277B_SERIES)
	l3pe_cntr_tx_t mc_tx_cntr;
	l3pe_cntr_rx_t mc_rx_cntr;
#endif
#endif
	struct net_device *p_PScacheDev[DEV_STACK_MAX]={NULL,NULL,NULL,NULL};

	FC_PARAM_CHK((pNetifMib == NULL), RTK_FC_RET_ERR_NULL_POINTER);

	API("get netifMib: %s", devName);
	memset(pNetifMib, 0, sizeof(rt_stat_netif_mib_t));
	pNetifMib->swNetifIdx = SIGNED_INVALID;
	pNetifMib->hwNetifIdx = SIGNED_INVALID;

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	
	for(i=0 ; i < RTK_FC_TABLESIZE_INTF_SW ; i++)
	{
		if (!fc_db.netifTbl[i].intf.valid)
			continue;
		stackCnt = rtk_fc_psIfidxStackKey_to_dev(fc_db.netifTbl[i].psIfidxStackKey,p_PScacheDev);
		API("stackCnt:%d",stackCnt);
		for(j=0;j<stackCnt;j++)
		{
			if(p_PScacheDev[j]==NULL)
				continue;
			API("p_PScacheDev[%d]->name:%s devName:%s",j,p_PScacheDev[j]->name,devName);
			if(!strcmp(p_PScacheDev[j]->name, devName))
			{

				pNetifMib->swNetifIdx = i;
				API("hit sw netif[%d]  hwIdx:%d",i,fc_db.netifTbl[i].hwIdx);
				if (fc_db.netifTbl[i].hwIdx != -1)
				{
					pNetifMib->ucCntValid = 1;
					pNetifMib->mcCntValid = 1;
					pNetifMib->bcCntValid = 1;

					pNetifMib->hwNetifIdx = fc_db.netifTbl[i].hwIdx;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
					memset(&intfMib,0,sizeof(intfMib));
					ret = rtk_rg_asic_netifMib_get(fc_db.netifTbl[i].hwIdx, &intfMib);
					if(ret != RT_ERR_OK)
					{
						WARNING("get hw netif mib failed, ret=%d", ret);
						RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
						return FAILED;
					}
					//get hw netif mib
					pNetifMib->ingress_packet_count += intfMib.in_intf_uc_packet_cnt + intfMib.in_intf_mc_packet_cnt + intfMib.in_intf_bc_packet_cnt;
					pNetifMib->ingress_byte_count += intfMib.in_intf_uc_byte_cnt + intfMib.in_intf_mc_byte_cnt + intfMib.in_intf_bc_byte_cnt;
					pNetifMib->egress_packet_count += intfMib.out_intf_uc_packet_cnt + intfMib.out_intf_mc_packet_cnt + intfMib.out_intf_bc_packet_cnt;
					pNetifMib->egress_byte_count += intfMib.out_intf_uc_byte_cnt + intfMib.out_intf_mc_byte_cnt + intfMib.out_intf_bc_byte_cnt;

					pNetifMib->ingress_uc_packet_count += intfMib.in_intf_uc_packet_cnt ;
					pNetifMib->ingress_uc_byte_count += intfMib.in_intf_uc_byte_cnt ;
					pNetifMib->egress_uc_packet_count += intfMib.out_intf_uc_packet_cnt ;
					pNetifMib->egress_uc_byte_count += intfMib.out_intf_uc_byte_cnt ;

					pNetifMib->ingress_mc_packet_count +=    intfMib.in_intf_mc_packet_cnt ;
					pNetifMib->ingress_mc_byte_count += intfMib.in_intf_mc_byte_cnt ;
					pNetifMib->egress_mc_packet_count +=   intfMib.out_intf_mc_packet_cnt ;
					pNetifMib->egress_mc_byte_count += intfMib.out_intf_mc_byte_cnt ;
					
					pNetifMib->ingress_bc_packet_count += intfMib.in_intf_bc_packet_cnt;
					pNetifMib->ingress_bc_byte_count += intfMib.in_intf_bc_byte_cnt;
					pNetifMib->egress_bc_packet_count += intfMib.out_intf_bc_packet_cnt;
					pNetifMib->egress_bc_byte_count += intfMib.out_intf_bc_byte_cnt;


#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
					//Note. G3 hw only supports for uc packets counting
					memset(&tx_cntr,0,sizeof(tx_cntr));
					memset(&rx_cntr,0,sizeof(rx_cntr));

					ret = aal_l3pe_cntr_rx_get(G3_DEF_DEVID, fc_db.netifTbl[i].hwIdx, &rx_cntr);
					if(ret != AAL_E_OK)
					{
						WARNING("get hw netif rx mib failed, ret=%d", ret);
						RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
						return FAILED;
					}

					ret = aal_l3pe_cntr_tx_get(G3_DEF_DEVID, fc_db.netifTbl[i].hwIdx, &tx_cntr);
					if(ret != AAL_E_OK)
					{
						WARNING("get hw netif tx mib failed, ret=%d", ret);
						RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
						return FAILED;
					}
#if defined(CONFIG_FC_CA8277B_SERIES)
					memset(&mc_tx_cntr,0,sizeof(mc_tx_cntr));
					memset(&mc_rx_cntr,0,sizeof(mc_rx_cntr));

					ret = aal_l3pe_cntr_rx_get(G3_DEF_DEVID, fc_db.netifTbl[i].hwIdx + RTK_FC_MC_HW_NETIF_IDXSHIFT, &mc_rx_cntr);
					if(ret != AAL_E_OK)
					{
						WARNING("get hw netif rx mib for mc pkt failed, ret=%d", ret);
						RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
						return FAILED;
					}

					ret = aal_l3pe_cntr_tx_get(G3_DEF_DEVID, fc_db.netifTbl[i].hwIdx + RTK_FC_MC_HW_NETIF_IDXSHIFT, &mc_tx_cntr);
					if(ret != AAL_E_OK)
					{
						WARNING("get hw netif tx mib for mc pkt failed, ret=%d", ret);
						RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
						return FAILED;
					}
					pNetifMib->ingress_packet_count += (rx_cntr.uc_pkt + rx_cntr.nuc_pkt) + rx_cntr.drop_pkt + rx_cntr.error_pkt + (mc_rx_cntr.uc_pkt + mc_rx_cntr.nuc_pkt) + mc_rx_cntr.drop_pkt + mc_rx_cntr.error_pkt;
					pNetifMib->ingress_byte_count += (rx_cntr.byte + (4 * (rx_cntr.uc_pkt + rx_cntr.nuc_pkt)/*CRC*/) + (rx_cntr.drop_byte + (4 * rx_cntr.drop_pkt)/*CRC*/)) + (mc_rx_cntr.byte + (4 * (mc_rx_cntr.uc_pkt + mc_rx_cntr.nuc_pkt)/*CRC*/) + (mc_rx_cntr.drop_byte + (4 * mc_rx_cntr.drop_pkt)/*CRC*/));
					pNetifMib->egress_packet_count += (tx_cntr.uc_pkt + tx_cntr.nuc_pkt) + tx_cntr.drop_pkt + (mc_tx_cntr.uc_pkt + mc_tx_cntr.nuc_pkt) + mc_tx_cntr.drop_pkt ;
					pNetifMib->egress_byte_count += (tx_cntr.byte + (4 * (tx_cntr.uc_pkt + tx_cntr.nuc_pkt))/*CRC*/) + (tx_cntr.drop_byte + (4 * tx_cntr.drop_pkt)/*CRC*/) + (mc_tx_cntr.byte + (4 * (mc_tx_cntr.uc_pkt + mc_tx_cntr.nuc_pkt))/*CRC*/) + (mc_tx_cntr.drop_byte + (4 * mc_tx_cntr.drop_pkt)/*CRC*/);

					if(pNetifMib->ucCntValid)
					{
						pNetifMib->ingress_uc_packet_count += (rx_cntr.uc_pkt + rx_cntr.nuc_pkt) + rx_cntr.drop_pkt + rx_cntr.error_pkt;
						pNetifMib->ingress_uc_byte_count += (rx_cntr.byte + (4 * (rx_cntr.uc_pkt + rx_cntr.nuc_pkt)/*CRC*/) + (rx_cntr.drop_byte + (4 * rx_cntr.drop_pkt)/*CRC*/));
						pNetifMib->egress_uc_packet_count += (tx_cntr.uc_pkt + tx_cntr.nuc_pkt) + tx_cntr.drop_pkt ;
						pNetifMib->egress_uc_byte_count += (tx_cntr.byte + (4 * (tx_cntr.uc_pkt + tx_cntr.nuc_pkt))/*CRC*/) + (tx_cntr.drop_byte + (4 * tx_cntr.drop_pkt)/*CRC*/) ;
					}
					if(pNetifMib->mcCntValid)
					{
						pNetifMib->ingress_mc_packet_count += (mc_rx_cntr.uc_pkt + mc_rx_cntr.nuc_pkt) + mc_rx_cntr.drop_pkt + mc_rx_cntr.error_pkt;
						pNetifMib->ingress_mc_byte_count += (mc_rx_cntr.byte + (4 * (mc_rx_cntr.uc_pkt + mc_rx_cntr.nuc_pkt)/*CRC*/) + (mc_rx_cntr.drop_byte + (4 * mc_rx_cntr.drop_pkt)/*CRC*/));
						pNetifMib->egress_mc_packet_count += (mc_tx_cntr.uc_pkt + mc_tx_cntr.nuc_pkt) + mc_tx_cntr.drop_pkt ;
						pNetifMib->egress_mc_byte_count += (mc_tx_cntr.drop_byte + (4 * mc_tx_cntr.drop_pkt)/*CRC*/);
					}

#elif defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
					{
						// get MC netif counter (care rx only)
						// policer counter is read clear, stored in HW after read
						aal_l3_te_pm_policer_t l3_pm_data;
						ret = aal_l3_te_pm_policer_flow_get(G3_DEF_DEVID, fc_db.netifTbl[i].hwIdx + G3_FLOW_POLICER_IDXSHIFT_MC_NETIF_RXMIB, &l3_pm_data);
						if(ret != CA_E_OK)
						{
							WARNING("get hw netif rx mib for mc pkt failed (netif idx %d, real hw policr idx %d), ret=%d", fc_db.netifTbl[i].hwIdx, fc_db.netifTbl[i].hwIdx + G3_FLOW_POLICER_IDXSHIFT_MC_NETIF_RXMIB, ret);
							RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
							return FAILED;
						}
						fc_db.netifTbl[i].hwMcNetifMib.in_intf_mc_packet_cnt += l3_pm_data.total_pkt;
						fc_db.netifTbl[i].hwMcNetifMib.in_intf_mc_byte_cnt += l3_pm_data.total_bytes;
					}
					pNetifMib->ingress_packet_count += (rx_cntr.uc_pkt + rx_cntr.nuc_pkt) + rx_cntr.drop_pkt + rx_cntr.error_pkt + fc_db.netifTbl[i].hwMcNetifMib.in_intf_mc_packet_cnt;
					pNetifMib->ingress_byte_count += (rx_cntr.byte + (4 * (rx_cntr.uc_pkt + rx_cntr.nuc_pkt)/*CRC*/) + (rx_cntr.drop_byte + (4 * rx_cntr.drop_pkt)/*CRC*/)) + fc_db.netifTbl[i].hwMcNetifMib.in_intf_mc_byte_cnt;
					pNetifMib->egress_packet_count += (tx_cntr.uc_pkt + tx_cntr.nuc_pkt) + tx_cntr.drop_pkt;
					pNetifMib->egress_byte_count += (tx_cntr.byte + (4 * (tx_cntr.uc_pkt + tx_cntr.nuc_pkt))/*CRC*/) + (tx_cntr.drop_byte + (4 * tx_cntr.drop_pkt)/*CRC*/);

					if(pNetifMib->ucCntValid)
					{
						pNetifMib->ingress_uc_packet_count += (rx_cntr.uc_pkt + rx_cntr.nuc_pkt) + rx_cntr.drop_pkt ;
						pNetifMib->ingress_uc_byte_count += (rx_cntr.byte + (4 * (rx_cntr.uc_pkt + rx_cntr.nuc_pkt)/*CRC*/) + (rx_cntr.drop_byte + (4 * rx_cntr.drop_pkt)/*CRC*/));
						pNetifMib->egress_uc_packet_count += (tx_cntr.uc_pkt + tx_cntr.nuc_pkt) + tx_cntr.drop_pkt;
						pNetifMib->egress_uc_byte_count += (tx_cntr.byte + (4 * (tx_cntr.uc_pkt + tx_cntr.nuc_pkt))/*CRC*/) + (tx_cntr.drop_byte + (4 * tx_cntr.drop_pkt)/*CRC*/);
					}
					if(pNetifMib->mcCntValid)
					{
						pNetifMib->ingress_mc_packet_count += fc_db.netifTbl[i].hwMcNetifMib.in_intf_mc_packet_cnt;
						pNetifMib->ingress_mc_byte_count +=  fc_db.netifTbl[i].hwMcNetifMib.in_intf_mc_byte_cnt;
						pNetifMib->egress_mc_packet_count += 0;
						pNetifMib->egress_mc_byte_count += 0;
					}

#else
					pNetifMib->mcCntValid=0;
					pNetifMib->ingress_packet_count += (rx_cntr.uc_pkt + rx_cntr.nuc_pkt) + rx_cntr.drop_pkt + rx_cntr.error_pkt;
					pNetifMib->ingress_byte_count += (rx_cntr.byte + (4 * (rx_cntr.uc_pkt + rx_cntr.nuc_pkt)/*CRC*/) + (rx_cntr.drop_byte + (4 * rx_cntr.drop_pkt)/*CRC*/));
					pNetifMib->egress_packet_count += (tx_cntr.uc_pkt + tx_cntr.nuc_pkt) + tx_cntr.drop_pkt;
					pNetifMib->egress_byte_count += (tx_cntr.byte + (4 * (tx_cntr.uc_pkt + tx_cntr.nuc_pkt))/*CRC*/) + (tx_cntr.drop_byte + (4 * tx_cntr.drop_pkt)/*CRC*/);
#endif
#endif
					//add sw netif mib
					pNetifMib->ingress_packet_count += _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, IN_UC_PKT_CNT) + _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, IN_MC_PKT_CNT) + _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, IN_BC_PKT_CNT);
					pNetifMib->ingress_byte_count += _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, IN_UC_BYTE_CNT) + _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, IN_MC_BYTE_CNT) + _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, IN_BC_BYTE_CNT);
					pNetifMib->egress_packet_count += _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, OUT_UC_PKT_CNT) + _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, OUT_MC_PKT_CNT) + _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, OUT_BC_PKT_CNT);
					pNetifMib->egress_byte_count += _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, OUT_UC_BYTE_CNT) + _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, OUT_MC_BYTE_CNT) + _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, OUT_BC_BYTE_CNT);

					if(pNetifMib->ucCntValid)
					{
						pNetifMib->ingress_uc_packet_count +=     _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, IN_UC_PKT_CNT) ;
						pNetifMib->ingress_uc_byte_count +=    _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, IN_UC_BYTE_CNT);
						pNetifMib->egress_uc_packet_count +=    _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, OUT_UC_PKT_CNT);
						pNetifMib->egress_uc_byte_count +=    _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, OUT_UC_BYTE_CNT);
					}
					if(pNetifMib->mcCntValid)
					{
						pNetifMib->ingress_mc_packet_count +=     _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, IN_MC_PKT_CNT) ;
						pNetifMib->ingress_mc_byte_count +=    _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, IN_MC_BYTE_CNT);
						pNetifMib->egress_mc_packet_count +=    _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, OUT_MC_PKT_CNT);
						pNetifMib->egress_mc_byte_count +=    _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, OUT_MC_BYTE_CNT);
					}
					if(pNetifMib->bcCntValid)
					{
						pNetifMib->ingress_bc_packet_count +=     _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, IN_BC_PKT_CNT) ;
						pNetifMib->ingress_bc_byte_count +=    _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, IN_BC_BYTE_CNT);
						pNetifMib->egress_bc_packet_count +=    _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, OUT_BC_PKT_CNT);
						pNetifMib->egress_bc_byte_count +=    _rtk_fc_sw_netifMib_all_cpu_get(fc_db.netifTbl[i].hwIdx, OUT_BC_BYTE_CNT);
					}

					
				}
				else
					DEBUG("dev %s does not exists in hw netif table", devName);
			}
		}

	}

	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	return SUCCESS;

}

int rtk_fc_netifMib_clear(char *devName)
{
	int32 i,j, ret,stackCnt;
	struct net_device *p_PScacheDev[DEV_STACK_MAX]={NULL,NULL,NULL,NULL};

	API("clear netifMib: %s", devName);
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	for(i=0 ; i  <RTK_FC_TABLESIZE_INTF_SW ; i++)
	{
		if (!fc_db.netifTbl[i].intf.valid)
			continue;
		stackCnt = rtk_fc_psIfidxStackKey_to_dev(fc_db.netifTbl[i].psIfidxStackKey,p_PScacheDev);

		for(j=0;j<stackCnt;j++)
		{
			if(p_PScacheDev[j]==NULL)
				continue;
			if(!strcmp(p_PScacheDev[j]->name, devName))
			{
				if (fc_db.netifTbl[i].hwIdx != -1)
				{
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
					ret = rtk_rg_asic_netifMib_reset(fc_db.netifTbl[i].hwIdx);// hw
					if(ret != RT_ERR_RG_OK)
						WARNING("clear hw netif mib failed, ret=%d", ret);
					bzero(&fc_db.netifTbl[i].intfMib, NR_CPUS*sizeof(rtk_rg_asic_netifMib_entry_t));// sw

#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
					// hw
					{
						l3pe_cntr_tx_t tx_cntr;
						l3pe_cntr_rx_t rx_cntr;

						memset(&tx_cntr, 0, sizeof(tx_cntr));
						memset(&rx_cntr, 0, sizeof(rx_cntr));

						ret = aal_l3pe_cntr_tx_set(G3_DEF_DEVID, fc_db.netifTbl[i].hwIdx, &tx_cntr);
						if(ret != AAL_E_OK)
							WARNING("clear hw netif[%d] tx mib failed, ret=%d", fc_db.netifTbl[i].hwIdx, ret);
						ret = aal_l3pe_cntr_rx_set(G3_DEF_DEVID, fc_db.netifTbl[i].hwIdx, &rx_cntr);
						if(ret != AAL_E_OK)
							WARNING("clear hw netif[%d] rx mib failed, ret=%d", fc_db.netifTbl[i].hwIdx, ret);
#if defined(CONFIG_FC_CA8277B_SERIES)
						// clear MC netif counter
						ret = aal_l3pe_cntr_tx_set(G3_DEF_DEVID, fc_db.netifTbl[i].hwIdx + RTK_FC_MC_HW_NETIF_IDXSHIFT, &tx_cntr);
						if(ret != AAL_E_OK)
							WARNING("clear hw netif[%d] (real hw index %d) tx mib for mc packet failed, ret=%d", fc_db.netifTbl[i].hwIdx, fc_db.netifTbl[i].hwIdx + RTK_FC_MC_HW_NETIF_IDXSHIFT, ret);
						ret = aal_l3pe_cntr_rx_set(G3_DEF_DEVID, fc_db.netifTbl[i].hwIdx + RTK_FC_MC_HW_NETIF_IDXSHIFT, &rx_cntr);
						if(ret != AAL_E_OK)
							WARNING("clear hw netif[%d] (real hw index %d) rx mib for mc packet failed, ret=%d", fc_db.netifTbl[i].hwIdx, fc_db.netifTbl[i].hwIdx + RTK_FC_MC_HW_NETIF_IDXSHIFT, ret);

#elif defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
						// clear MC netif counter (care rx counter only)
						{
							aal_l3_te_pm_policer_t l3_pm_data;
							aal_l3_te_pm_policer_flow_get(G3_DEF_DEVID, fc_db.netifTbl[i].hwIdx + G3_FLOW_POLICER_IDXSHIFT_MC_NETIF_RXMIB, &l3_pm_data); //read clear
							if(ret != AAL_E_OK)
								WARNING("clear hw netif[%d] (real hw pol_idx %d) rx mib for mc packet failed, ret=%d", fc_db.netifTbl[i].hwIdx, fc_db.netifTbl[i].hwIdx + G3_FLOW_POLICER_IDXSHIFT_MC_NETIF_RXMIB, ret);
						}
						bzero(&fc_db.netifTbl[i].hwMcNetifMib, sizeof(fc_db.netifTbl[i].hwMcNetifMib));
#endif
					}
					bzero(&fc_db.netifTbl[i].intfMib, sizeof(rtk_rg_asic_netifMib_entry_t));// sw
#endif
				}
				else
					DEBUG("dev %s does not exists in hw netif table", devName);
			}
		}
	}
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return SUCCESS;
}

int rtk_fc_aclMib_get(int idx, rt_stat_acl_mib_t *paclMibCnt)
{
	int32 ret;
	API("get aclMib[%d]", idx);

	FC_PARAM_CHK((idx<0)||(idx >= RT_STAT_ACLMIB_TABLE_SIZE), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	FC_PARAM_CHK((paclMibCnt == NULL), RTK_FC_RET_ERR_NULL_POINTER);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	ret = _rtk_fc_acl_mib_get(idx, paclMibCnt);
	if(ret != RT_ERR_OK)
		WARNING("fail to get acl mib counter %d", idx);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return SUCCESS;
}

int rtk_fc_aclMib_set(int idx, rt_stat_acl_mib_t paclMibCnt)
{
	int32 ret;
	API("set aclMib[%d]", idx);

	FC_PARAM_CHK((idx<0)||(idx >= RT_STAT_ACLMIB_TABLE_SIZE), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	ret = _rtk_fc_acl_mib_set(idx, paclMibCnt);
	if(ret != RT_ERR_OK)
		WARNING("fail to set acl mib counter %d", idx);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return SUCCESS;
}

int rtk_fc_aclMib_clear(int idx)
{
	int32 ret;
	API("clear aclMib[%d]", idx);

	FC_PARAM_CHK((idx<0)||(idx >= RT_STAT_ACLMIB_TABLE_SIZE), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	ret = _rtk_fc_acl_mib_clear(idx);
	if(ret != RT_ERR_OK)
		WARNING("fail to reset acl mib counter %d", idx);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return SUCCESS;
}

int rtk_fc_flow_callbackFunc_register(rt_flow_callback_func_t cbFuncType, void* pCbFunc)
{
	switch(cbFuncType)
	{
		case RT_FLOW_CB_FUNC_FLOW_ADD:
			fc_db.flow_callback_func.cbFlowAdd = (cbFlowAdd_t)pCbFunc;
			break;
		case RT_FLOW_CB_FUNC_FLOW_DEL:
                        fc_db.flow_callback_func.cbFlowDel = (cbFlowDel_t)pCbFunc;
                        break;
#if defined(FC_F_SHORTCUT_CALLBACK)
		case RT_FLOW_CB_FUNC_SHORTCUT_BEFORE:
			fc_db.flow_callback_func.cbShortcut_before = (cbShortcut_t)pCbFunc;
			break;
		case RT_FLOW_CB_FUNC_SHORTCUT_AFTER:
			fc_db.flow_callback_func.cbShortcut_after = (cbShortcut_t)pCbFunc;
			break;
#endif
		default:
			WARNING("unknown callback function type %d", cbFuncType);
			return RTK_FC_RET_ERR_INVALID_PARAM;
	}
	return SUCCESS;
}

int rtk_fc_flow_operation_add(rt_flow_ops_t flowOp, rt_flow_ops_data_t* pOpData)
{
	int ret = SUCCESS, i;
	switch(flowOp)
	{
		case RT_FLOW_OPS_DELETE_FLOW:
			// delete flow by flow key
			
			if(pOpData == NULL)
			{
				WARNING("[flowOp] delete flow with NULL flow operation data");
				return RTK_FC_RET_ERR_NULL_POINTER;
			}
			switch(pOpData->data_delFlow.flowKeyType)
			{
				case RT_FLOW_OP_FLOW_KEY_TYPE_INDEX:
					RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
					rtk_fc_flow_delete_by_entryIdx(pOpData->data_delFlow.flowKey.flowIdx);
					API("[flowOp] delete flow by index %d", pOpData->data_delFlow.flowKey.flowIdx);
					RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
					break;
				case RT_FLOW_OP_FLOW_KEY_TYPE_PATTERN:
					RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
					rtk_fc_flow_delete_by_pattern(pOpData->data_delFlow.flowKey.flowPattern);
					API("[flowOp] delete flow by pattern");
					API("[flowOp] pattern l4proto_valid: %d l4proto: %s", (pOpData->data_delFlow.flowKey.flowPattern.l4proto_valid)?1:0, (pOpData->data_delFlow.flowKey.flowPattern.l4proto == RT_FLOW_OP_FLOW_PATTERN_L4PROTO_UDP)?"UDP":"TCP");
					API("[flowOp] pattern SIP4_valid: %d SIP4: %pI4h", (pOpData->data_delFlow.flowKey.flowPattern.sip4_valid)?1:0, &(pOpData->data_delFlow.flowKey.flowPattern.sip4));
					API("[flowOp] pattern DIP4_valid: %d DIP4: %pI4h", (pOpData->data_delFlow.flowKey.flowPattern.dip4_valid)?1:0, &(pOpData->data_delFlow.flowKey.flowPattern.dip4));
					{
						uint32 v6_saddr_hash = _rtk_rg_flowHashIPv6SrcAddr_get(pOpData->data_delFlow.flowKey.flowPattern.sip6);
						uint32 v6_daddr_hash = _rtk_rg_flowHashIPv6DstAddr_get(pOpData->data_delFlow.flowKey.flowPattern.dip6);
						API("[flowOp] pattern SIP6_valid: %d SIP6: %pI6c (v6_addr_hash: 0x%x)", (pOpData->data_delFlow.flowKey.flowPattern.sip6_valid)?1:0, &(pOpData->data_delFlow.flowKey.flowPattern.sip6), v6_saddr_hash);
						API("[flowOp] pattern DIP6_valid: %d DIP6: %pI6c (v6_addr_hash: 0x%x)", (pOpData->data_delFlow.flowKey.flowPattern.dip6_valid)?1:0, &(pOpData->data_delFlow.flowKey.flowPattern.dip6), v6_daddr_hash);
					}
					API("[flowOp] pattern SPORT_valid: %d SPORT: %d", (pOpData->data_delFlow.flowKey.flowPattern.sport_valid)?1:0, pOpData->data_delFlow.flowKey.flowPattern.sport);
					API("[flowOp] pattern DPORT_valid: %d DPORT: %d", (pOpData->data_delFlow.flowKey.flowPattern.dport_valid)?1:0, pOpData->data_delFlow.flowKey.flowPattern.dport);
					API("[flowOp] pattern MAC_valid: %d MAC: %pM", pOpData->data_delFlow.flowKey.flowPattern.mac_valid,pOpData->data_delFlow.flowKey.flowPattern.mac);
					API("[flowOp] pattern SMAC_valid: %d SMAC: %pM", pOpData->data_delFlow.flowKey.flowPattern.smac_valid,pOpData->data_delFlow.flowKey.flowPattern.smac);
					API("[flowOp] pattern DMAC_valid: %d DMAC: %pM", pOpData->data_delFlow.flowKey.flowPattern.dmac_valid,pOpData->data_delFlow.flowKey.flowPattern.dmac);
					API("[flowOp] pattern MSIP4_valid: %d MSIP4: %pI4h", (pOpData->data_delFlow.flowKey.flowPattern.modified_sip4_valid)?1:0, &(pOpData->data_delFlow.flowKey.flowPattern.modified_sip4));
					API("[flowOp] pattern MDIP4_valid: %d MDIP4: %pI4h", (pOpData->data_delFlow.flowKey.flowPattern.modified_dip4_valid)?1:0, &(pOpData->data_delFlow.flowKey.flowPattern.modified_dip4));
					API("[flowOp] pattern MSPORT_valid: %d MSPORT: %d", (pOpData->data_delFlow.flowKey.flowPattern.modified_sport_valid)?1:0, pOpData->data_delFlow.flowKey.flowPattern.modified_sport);
					API("[flowOp] pattern MDPORT_valid: %d MDPORT: %d", (pOpData->data_delFlow.flowKey.flowPattern.modified_dport_valid)?1:0, pOpData->data_delFlow.flowKey.flowPattern.modified_dport);
					RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
					break;
				default:
					WARNING("[flowOp] delete flow by unknown key type");
					break;
			}
			break;
		case RT_FLOW_OPS_FLUSH_FLOWTABLE:
			// flush flow table, including SW and HW flows
			RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
			ret = rtk_fc_flow_flush();
			API("[flowOp] flow flush");
			RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
			break;
		case RT_FLOW_OPS_GET_SW_FLOW_MIB_BY_IDX:
			// get flow mib by flow index (support in HWNAT_MODE_SW_ONLY mode only)
			if(pOpData == NULL)
			{
				WARNING("[flowOp] get SW flow mib with NULL flow operation data");
				return RTK_FC_RET_ERR_NULL_POINTER;
			}
			
			if(fc_db.controlFuc.hwnat_mode==RT_FLOW_HWNAT_MODE_SW_ONLY)
			{
				if(pOpData->data_getFlowMib.index >= fc_db.flowSwTableSize)
				{
					WARNING("[flowOp] Invalid parameter: flowIdx %d", pOpData->data_getFlowMib.index);
					return (RTK_FC_RET_ERR_INVALID_PARAM);
				}
				memset(&(pOpData->data_getFlowMib.swFlowCnt), 0, sizeof(pOpData->data_getFlowMib.swFlowCnt));
				if(!fc_db.flowTbl[pOpData->data_getFlowMib.index].pFlowEntry->path1.valid)
				{
					WARNING("[flowOp] No flow is recorded in this entry[%d]", pOpData->data_getFlowMib.index);
					return (RTK_FC_RET_ERR_ENTRY_NOT_FOUND);
				}

				RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
				RTK_FC_HELPER_MGR_SW_FLOW_MIB_GET(pOpData->data_getFlowMib.index, &(pOpData->data_getFlowMib.swFlowCnt));
				API("[flowOp] Get sw flow mib [%d]", pOpData->data_getFlowMib.index);
				RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
				ret = SUCCESS;
			}
			else
			{
				WARNING("[flowOp] SW flow mib can only be used in SW-only acceleration mode\n");
			}
			break;
		case RT_FLOW_OPS_GET_FLOWGRP_MIB_BY_IDX:
			// get flow group mib by flowmib index
			if(pOpData == NULL)
			{
				WARNING("[flowOp] get flow group mib with NULL flow operation data");
				return RTK_FC_RET_ERR_NULL_POINTER;
			}
			if(pOpData->data_getFlowGrpMib.index >= RT_STAT_FLOWMIB_TABLE_SIZE)
			{
				WARNING("[flowOp]  Invalid parameter: flow group mib index %d (it should between 0~%d)\n", pOpData->data_getFlowGrpMib.index, RT_STAT_FLOWMIB_TABLE_SIZE);
				return RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE;
			}
			/*===no need to lock, lock in rtk_fc_flowMib_get()===*/
			ret = rtk_fc_flowMib_get(pOpData->data_getFlowGrpMib.index, &(pOpData->data_getFlowGrpMib.flowGrpCnt));
			API("[flowOp] Get flow group mib [%d]", pOpData->data_getFlowGrpMib.index);
			/*=================================*/
			break;
			
		case RT_FLOW_OPS_CLEAR_ALL_MIB:
			// clear all mib, including flow mib and flow group mib
			RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
			RTK_FC_HELPER_MGR_SW_FLOW_MIB_CLEARALL();
			RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

			/*===no need to lock, lock in rtk_fc_flowMib_clear()===*/
			for(i = 0 ; i < RT_STAT_FLOWMIB_TABLE_SIZE ; i++)
				rtk_fc_flowMib_clear(i);
			/*=================================*/
			API("[flowOp] clear all sw flow mib and flow group mib");
			break;
		case RT_FLOW_OPS_CLEAR_SW_FLOW_MIB_BY_IDX:
			// clear flow mib by flow index (support in HWNAT_MODE_SW_ONLY mode only)
			if(pOpData == NULL)
			{
				WARNING("[flowOp] clear SW flow mib by index with NULL flow operation data");
				return RTK_FC_RET_ERR_NULL_POINTER;
			}
			RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
			RTK_FC_HELPER_MGR_SW_FLOW_MIB_CLEAR(pOpData->data_clearFlowMib);
			API("[flowOp] clear sw flow mib [%d]", pOpData->data_clearFlowMib);
			RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
			break;
		case RT_FLOW_OPS_CLEAR_FLOWGRP_MIB_BY_IDX:
			// clear flow group mib by flowmib index
			if(pOpData == NULL)
			{
				WARNING("[flowOp] clear flow group mib by index with NULL flow operation data");
				return RTK_FC_RET_ERR_NULL_POINTER;
			}
			/*===no need to lock, lock in rtk_fc_flowMib_clear()===*/
			rtk_fc_flowMib_clear(pOpData->data_clearFlowGrpMib);
			API("[flowOp] clear flow group mib [%d]", pOpData->data_clearFlowGrpMib);
			/*=================================*/
			break;
		case RT_FLOW_OPS_SYNC_CT_STATE:
			// sync ct state to flow table immediately
			RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
			for(i = 0 ; i < fc_db.flowSync_groupCnt ; i++)
			{			
				_rtk_fc_flow_timeoutCheck();
				API("[flowOp] sync ct state to flow");
			}			
			RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
			break;
		case RT_FLOW_OPS_HIPRIFLOW_PTN_ACT:
			if(pOpData == NULL)
			{
				WARNING("[flowOp] high priority flow pattern action with NULL flow operation data");
				return RTK_FC_RET_ERR_NULL_POINTER;
			}
			switch(pOpData->data_hiPriflowPtnAct.hipriflow_ptn_act)
			{
				case RT_FLOW_OP_HIPRIFLOW_PTN_ACT_ADD_BY_IDX:
					if(pOpData->data_hiPriflowPtnAct.index >= RT_FLOW_HIGHPRIFLOW_PATTERN_SIZE)
					{
						WARNING("[flowOp] Invalid parameter: hiPriflowPtnAct Index %d", pOpData->data_hiPriflowPtnAct.index);
						return (RTK_FC_RET_ERR_INVALID_PARAM);
					}
					if(fc_db.highPriFlowPattern[pOpData->data_hiPriflowPtnAct.index].l4proto_valid ||
						fc_db.highPriFlowPattern[pOpData->data_hiPriflowPtnAct.index].sport_valid || fc_db.highPriFlowPattern[pOpData->data_hiPriflowPtnAct.index].dport_valid ||
						fc_db.highPriFlowPattern[pOpData->data_hiPriflowPtnAct.index].sip4_valid || fc_db.highPriFlowPattern[pOpData->data_hiPriflowPtnAct.index].dip4_valid ||
						fc_db.highPriFlowPattern[pOpData->data_hiPriflowPtnAct.index].sip6_valid || fc_db.highPriFlowPattern[pOpData->data_hiPriflowPtnAct.index].dip6_valid)
					{
						WARNING("[flowOp] Invalid parameter: hiPriflowPtnAct[%d] is not empty", pOpData->data_hiPriflowPtnAct.index);
						return (RTK_FC_RET_ERR_INVALID_PARAM);
					}

					RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
					memcpy(&fc_db.highPriFlowPattern[pOpData->data_hiPriflowPtnAct.index], &pOpData->data_hiPriflowPtnAct.flowPattern, sizeof(fc_db.highPriFlowPattern[pOpData->data_hiPriflowPtnAct.index]));
					RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
					break;
				case RT_FLOW_OP_HIPRIFLOW_PTN_ACT_DEL_BY_IDX:
					if(pOpData->data_hiPriflowPtnAct.index >= RT_FLOW_HIGHPRIFLOW_PATTERN_SIZE)
					{
						WARNING("[flowOp] Invalid parameter: hiPriflowPtnAct Index %d", pOpData->data_hiPriflowPtnAct.index);
						return (RTK_FC_RET_ERR_INVALID_PARAM);
					}
					RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
					memset(&fc_db.highPriFlowPattern[pOpData->data_hiPriflowPtnAct.index], 0, sizeof(fc_db.highPriFlowPattern[pOpData->data_hiPriflowPtnAct.index]));
					RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
					break;
				case RT_FLOW_OP_HIPRIFLOW_PTN_ACT_GET_BY_IDX:
					if(pOpData->data_hiPriflowPtnAct.index >= RT_FLOW_HIGHPRIFLOW_PATTERN_SIZE)
					{
						WARNING("[flowOp] Invalid parameter: hiPriflowPtnAct Index %d", pOpData->data_hiPriflowPtnAct.index);
						return (RTK_FC_RET_ERR_INVALID_PARAM);
					}
					RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
					memcpy(&pOpData->data_hiPriflowPtnAct.flowPattern, &fc_db.highPriFlowPattern[pOpData->data_hiPriflowPtnAct.index], sizeof(pOpData->data_hiPriflowPtnAct.flowPattern));
					RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
					break;
				case RT_FLOW_OP_HIPRIFLOW_PTN_ACT_FLUSH:
					RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
					memset(fc_db.highPriFlowPattern, 0, sizeof(fc_db.highPriFlowPattern[0]) * RT_FLOW_HIGHPRIFLOW_PATTERN_SIZE);
					RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
					break;
				default:
					WARNING("[flowOp] high priority flow pattern action with unknown action type");
					break;
			}
			break;
		default:
			WARNING("unknown flow operation");
			return RTK_FC_RET_ERR_INVALID_PARAM;
			break;
	}
	return SUCCESS;
}

int rtk_fc_flow_hwnatMode_set(rt_flow_hwnat_mode_t hwnatMode)
{
	fc_db.controlFuc.hwnat_mode = hwnatMode;

	if(hwnatMode == 100) hwnatMode = RT_FLOW_HWNAT_MODE_SPEC_TEST;

	if(hwnatMode >= RT_FLOW_HWNAT_MODE_END && hwnatMode != RT_FLOW_HWNAT_MODE_SPEC_TEST)
		return RTK_FC_RET_ERR_INVALID_PARAM;
	
	if(hwnatMode == RT_FLOW_HWNAT_MODE_TRAP_ALL)	//trap by acl to make sure sw only
		assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_ALL_TRAP, NULL));
	else
		assert_ok(_rtk_rg_aclAndCfReservedRuleDel(RTK_FC_ACLANDCF_RESERVED_ALL_TRAP));
	
	RTK_FC_HELPER_MGR_HWNAT_MODE_SET(hwnatMode);

	if(hwnatMode!=RT_FLOW_HWNAT_MODE_SPEC_TEST)
		rtk_fc_flow_flush();//clear all flow

	return SUCCESS;
}

int rtk_fc_flow_hwnatMode_get(rt_flow_hwnat_mode_t *pHwnatMode)
{
	if(pHwnatMode == NULL)
		return RTK_FC_RET_ERR_NULL_POINTER;
		
	*pHwnatMode = fc_db.controlFuc.hwnat_mode;
	return SUCCESS;
}

#if defined(CONFIG_FC_RTL8277C_SERIES)
//TODO: 9607F aqm
int rtk_fc_qos_aqm_get(rt_qos_aqm_cfg_t* pAqm_cfg)
{
	int aal_ret;
	l3fe_hash_aqm_cfg_t aal_aqm_cfg;
	
	if(pAqm_cfg == NULL)
		return RTK_FC_RET_ERR_NULL_POINTER;

	aal_ret = aal_l3fe_hash_aqm_cfg_get(&aal_aqm_cfg);
	if(aal_ret)
	{
		WARNING("Fail to get aqm cfg, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}

	pAqm_cfg->enable 		= aal_aqm_cfg.aqm_enable;
	pAqm_cfg->action_type 	= aal_aqm_cfg.action_type;

	return SUCCESS;
}

int rtk_fc_qos_aqm_set(rt_qos_aqm_cfg_t aqm_cfg)
{
	int aal_ret;
	l3fe_hash_aqm_cfg_t aal_aqm_cfg;
	aal_l3_te_glb_cfg_t te_cfg;
	aal_l3_te_glb_cfg_msk_t te_msk;

	if(aqm_cfg.enable > 1)
		return RTK_FC_RET_ERR_INVALID_PARAM;
	if(aqm_cfg.action_type >= RT_QOS_AQM_ACTION_TYPE_MAX)
		return RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE;

	aal_ret = aal_l3fe_hash_aqm_cfg_get(&aal_aqm_cfg);
	if(aal_ret)
	{
		WARNING("Fail to get aqm cfg, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	
	aal_aqm_cfg.aqm_enable 		= aqm_cfg.enable;
	aal_aqm_cfg.action_type		= aqm_cfg.action_type;
	
	aal_ret = aal_l3fe_hash_aqm_cfg_set(aal_aqm_cfg);
	if(aal_ret)
	{
		WARNING("Fail to set aqm cfg, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}

	//manipulate PIE settings
	memset((void *)&te_cfg, 0, sizeof(te_cfg));
	te_msk.u32 = 0;
	te_msk.s.cm_selection  = 1;
	if(aqm_cfg.enable && aqm_cfg.action_type == RT_QOS_AQM_ACTION_TYPE_PIE_DROP)		
		te_cfg.cm_selection  = CA_TRUE;
	else
		te_cfg.cm_selection  = CA_FALSE;
	aal_ret=aal_l3_te_glb_cfg_set(G3_DEF_DEVID, te_msk, &te_cfg);
	if(aal_ret)
	{
		WARNING("Fail to set pie cfg, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	
	return SUCCESS;
}

int rtk_fc_qos_aqm_cos_get(rt_qos_aqm_cos_cfg_t* pAqm_cos_cfg)
{
	int aal_ret;
	l3fe_hash_aqm_cfg_t aal_aqm_cfg;
	
	if(pAqm_cos_cfg == NULL)
		return RTK_FC_RET_ERR_NULL_POINTER;

	aal_ret = aal_l3fe_hash_aqm_cfg_get(&aal_aqm_cfg);
	if(aal_ret)
	{
		WARNING("Fail to get aqm cfg, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}

	pAqm_cos_cfg->flow_type_mask	= aal_aqm_cfg.cos_enable_mask;
	pAqm_cos_cfg->big_cos			= aal_aqm_cfg.big_cos;
	pAqm_cos_cfg->middle_cos		= aal_aqm_cfg.middle_cos;
	pAqm_cos_cfg->small_cos			= aal_aqm_cfg.small_cos;
		
	return SUCCESS;
}

int rtk_fc_qos_aqm_cos_set(rt_qos_aqm_cos_cfg_t aqm_cos_cfg)
{
	int aal_ret;
	l3fe_hash_aqm_cfg_t aal_aqm_cfg;

	if(aqm_cos_cfg.flow_type_mask >= RT_QOS_AQM_FLOW_TYPE_BITMASK_MAX)
		return RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE;
	
	aal_ret = aal_l3fe_hash_aqm_cfg_get(&aal_aqm_cfg);
	if(aal_ret)
	{
		WARNING("Fail to get aqm cfg, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	
	aal_aqm_cfg.cos_enable_mask 	= aqm_cos_cfg.flow_type_mask;
	aal_aqm_cfg.big_cos 			= aqm_cos_cfg.big_cos;
	aal_aqm_cfg.middle_cos 			= aqm_cos_cfg.middle_cos;
	aal_aqm_cfg.small_cos 			= aqm_cos_cfg.small_cos;
	
	aal_ret = aal_l3fe_hash_aqm_cfg_set(aal_aqm_cfg);
	if(aal_ret)
	{
		WARNING("Fail to set aqm cfg, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	
	return SUCCESS;
}


int rtk_fc_qos_aqm_threshold_get(rt_qos_aqm_threshold_cfg_t* pAqm_threshold_cfg)
{
	int aal_ret;
	l3fe_hash_aqm_cfg_t aal_aqm_cfg;
	
	if(pAqm_threshold_cfg == NULL)
		return RTK_FC_RET_ERR_NULL_POINTER;

	aal_ret = aal_l3fe_hash_aqm_cfg_get(&aal_aqm_cfg);
	if(aal_ret)
	{
		WARNING("Fail to get aqm cfg, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}

	switch(pAqm_threshold_cfg->flow_type)
	{
		case RT_QOS_AQM_FLOW_TYPE_BIG:
		{
			l3fe_hash_aqm_big_cfg_t aal_aqm_big_cfg;

			aal_ret = aal_l3fe_hash_aqm_big_cfg_get(&aal_aqm_big_cfg);
			if(aal_ret)
			{
				WARNING("Fail to get aqm big cfg, aal_ret = %d", aal_ret);
				return RTK_FC_RET_ERR;
			}
			pAqm_threshold_cfg->protocol_mask 			= aal_aqm_cfg.big_protocol_mask;
			pAqm_threshold_cfg->pktSize_threshold 		= aal_aqm_big_cfg.pktSize_threshold_on;
			pAqm_threshold_cfg->byteCnt_threshold_on 	= aal_aqm_big_cfg.byteCnt_threshold_on;
			pAqm_threshold_cfg->byteCnt_threshold_off 	= aal_aqm_big_cfg.byteCnt_threshold_off;
			break;
		}
		case RT_QOS_AQM_FLOW_TYPE_MIDDLE:
		{
			l3fe_hash_aqm_middle_cfg_t aal_aqm_middle_cfg;

			aal_ret = aal_l3fe_hash_aqm_middle_cfg_get(&aal_aqm_middle_cfg);
			if(aal_ret)
			{
				WARNING("Fail to get aqm middle cfg, aal_ret = %d", aal_ret);
				return RTK_FC_RET_ERR;
			}
			pAqm_threshold_cfg->protocol_mask 			= aal_aqm_cfg.middle_protocol_mask;
			pAqm_threshold_cfg->pktSize_threshold 		= aal_aqm_middle_cfg.pktSize_threshold_on;
			pAqm_threshold_cfg->byteCnt_threshold_on 	= aal_aqm_middle_cfg.byteCnt_threshold_on;
			pAqm_threshold_cfg->byteCnt_threshold_off 	= aal_aqm_middle_cfg.byteCnt_threshold_off;
			break;
		}
		default:
			return RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE;
			break;
	}
		
	return SUCCESS;
}

int rtk_fc_qos_aqm_threshold_set(rt_qos_aqm_threshold_cfg_t aqm_threshold_cfg)
{
	int aal_ret;
	l3fe_hash_aqm_cfg_t aal_aqm_cfg;

	aal_ret = aal_l3fe_hash_aqm_cfg_get(&aal_aqm_cfg);
	if(aal_ret)
	{
		WARNING("Fail to get aqm cfg, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	
	switch(aqm_threshold_cfg.flow_type)
	{
		case RT_QOS_AQM_FLOW_TYPE_BIG:
		{
			l3fe_hash_aqm_big_cfg_t aal_aqm_big_cfg;

			aal_aqm_cfg.big_protocol_mask = aqm_threshold_cfg.protocol_mask;
			aal_ret = aal_l3fe_hash_aqm_cfg_set(aal_aqm_cfg);
			if(aal_ret)
			{
				WARNING("Fail to set aqm cfg, aal_ret = %d", aal_ret);
				return RTK_FC_RET_ERR;
			}
			//threshold
			aal_aqm_big_cfg.pktSize_threshold_on = aqm_threshold_cfg.pktSize_threshold;
			aal_aqm_big_cfg.byteCnt_threshold_on = aqm_threshold_cfg.byteCnt_threshold_on;
			aal_aqm_big_cfg.byteCnt_threshold_off = aqm_threshold_cfg.byteCnt_threshold_off;
			aal_ret = aal_l3fe_hash_aqm_big_cfg_set(aal_aqm_big_cfg);
			if(aal_ret)
			{
				WARNING("Fail to set aqm big cfg, aal_ret = %d", aal_ret);
				return RTK_FC_RET_ERR;
			}		
			break;
		}
		case RT_QOS_AQM_FLOW_TYPE_MIDDLE:
		{
			l3fe_hash_aqm_middle_cfg_t aal_aqm_middle_cfg;

			aal_aqm_cfg.middle_protocol_mask = aqm_threshold_cfg.protocol_mask;
			aal_ret = aal_l3fe_hash_aqm_cfg_set(aal_aqm_cfg);
			if(aal_ret)
			{
				WARNING("Fail to set aqm cfg, aal_ret = %d", aal_ret);
				return RTK_FC_RET_ERR;
			}
			//threshold
			aal_aqm_middle_cfg.pktSize_threshold_on = aqm_threshold_cfg.pktSize_threshold;
			aal_aqm_middle_cfg.byteCnt_threshold_on = aqm_threshold_cfg.byteCnt_threshold_on;
			aal_aqm_middle_cfg.byteCnt_threshold_off = aqm_threshold_cfg.byteCnt_threshold_off;
			aal_ret = aal_l3fe_hash_aqm_middle_cfg_set(aal_aqm_middle_cfg);
			if(aal_ret)
			{
				WARNING("Fail to set aqm middle cfg, aal_ret = %d", aal_ret);
				return RTK_FC_RET_ERR;
			}
			break;
		}
		default:
			return RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE;
			break;
	}
	
	return SUCCESS;
}

int rtk_fc_qos_aqm_timer_get(rt_qos_aqm_timer_cfg_t* pAqm_timer_cfg)
{
	int aal_ret;
	l3fe_hash_aqm_timer_cfg_t aal_aqm_timer_cfg;
	uint64 timer_interval_tmp;
	
	if(pAqm_timer_cfg == NULL)
		return RTK_FC_RET_ERR_NULL_POINTER;

	aal_ret = aal_l3fe_hash_aqm_timer_cfg_get(&aal_aqm_timer_cfg);
	if(aal_ret)
	{
		WARNING("Fail to get aqm timer cfg, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	timer_interval_tmp = ((uint64)aal_aqm_timer_cfg.timer_interval) * 2048;
	do_div(timer_interval_tmp, RTK_FC_L3FE_CLOCK_MS);
	pAqm_timer_cfg->timer_interval 			= timer_interval_tmp;
	pAqm_timer_cfg->pktSize_window_shift 	= aal_aqm_timer_cfg.pktSize_window_shift;
	pAqm_timer_cfg->byteCnt_weight_shift 	= aal_aqm_timer_cfg.byteCnt_weight_shift;
		
	return SUCCESS;
}

int rtk_fc_qos_aqm_timer_set(rt_qos_aqm_timer_cfg_t aqm_timer_cfg)
{
	int aal_ret;
	l3fe_hash_aqm_timer_cfg_t aal_aqm_timer_cfg;
	uint64 timer_interval_tmp;

	if(aqm_timer_cfg.timer_interval<RT_QOS_AQM_TIMER_INTERVAL_MS_MIN || aqm_timer_cfg.timer_interval>RT_QOS_AQM_TIMER_INTERVAL_MS_MAX)
		return RTK_FC_RET_ERR_INVALID_PARAM;
	if(aqm_timer_cfg.pktSize_window_shift > RT_QOS_AQM_TIMER_PKTSIZE_WINDOW_SHIFT_MAX)
		return RTK_FC_RET_ERR_INVALID_PARAM;
	if(aqm_timer_cfg.byteCnt_weight_shift > RT_QOS_AQM_TIMER_BYTECNT_WEIGHT_SHIFT_MAX)
		return RTK_FC_RET_ERR_INVALID_PARAM;

	timer_interval_tmp = aqm_timer_cfg.timer_interval;
	timer_interval_tmp = timer_interval_tmp * RTK_FC_L3FE_CLOCK_MS;
	timer_interval_tmp = (timer_interval_tmp%2048) ? (timer_interval_tmp/2048 + 1) : (timer_interval_tmp/2048);
	aal_aqm_timer_cfg.timer_interval 		= timer_interval_tmp;
	aal_aqm_timer_cfg.pktSize_window_shift 	= aqm_timer_cfg.pktSize_window_shift;
	aal_aqm_timer_cfg.byteCnt_weight_shift 	= aqm_timer_cfg.byteCnt_weight_shift;
	
	aal_ret = aal_l3fe_hash_aqm_timer_cfg_set(aal_aqm_timer_cfg);
	if(aal_ret)
	{
		WARNING("Fail to set aqm timer cfg, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	
	return SUCCESS;
}

int rtk_fc_qos_aqm_flow_statistic_get(rt_qos_aqm_flow_stat_t* pAqm_flow_stat)
{
	int aal_ret;
	uint32 flow_cache_idx = 0;
	l3fe_hash_aqm_flow_stat_tbl_entry_t aal_aqm_flow_stat_entry;

	FC_PARAM_CHK(pAqm_flow_stat == NULL, RTK_FC_RET_ERR_NULL_POINTER);
	FC_PARAM_CHK(pAqm_flow_stat->flow_idx >= RT_STAT_FLOW_TABLE_SIZE, RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	
	aal_ret = aal_hash_cache_hash_index_get(pAqm_flow_stat->flow_idx, &flow_cache_idx);
	if(aal_ret)
	{
		if(aal_ret==AAL_E_ENTRYNOTFOUND)
			WARNING("Fail to get aqm flow statistic ... flow[%d] is not in cache", pAqm_flow_stat->flow_idx);
		else
			WARNING("Fail to get flow[%d]'s cache_idx, aal_ret = %d", pAqm_flow_stat->flow_idx, aal_ret);
		
		return RTK_FC_RET_ERR;
	}

	aal_ret = aal_l3fe_hash_aqm_flow_stat_entry_get(flow_cache_idx, &aal_aqm_flow_stat_entry);
	if(aal_ret)
	{
		WARNING("Fail to get aqm flow statistic, aal_ret = %d, flow_cache_idx=%d", aal_ret, flow_cache_idx);
		return RTK_FC_RET_ERR;
	}

	pAqm_flow_stat->flow_proto 		= aal_aqm_flow_stat_entry.flow_proto;
	pAqm_flow_stat->flow_type 		= aal_aqm_flow_stat_entry.flow_type;
	pAqm_flow_stat->avg_byteCnt 	= aal_aqm_flow_stat_entry.avg_byteCnt;
	pAqm_flow_stat->cur_byteCnt 	= aal_aqm_flow_stat_entry.cur_byteCnt;
	pAqm_flow_stat->window_pktSize 	= aal_aqm_flow_stat_entry.window_pktSize;

	return SUCCESS;
}


int32 rtk_fc_qos_aqm_pie_enable_get(rt_qos_aqm_pie_enable_t* pAqmPie_enable)
{
	int aal_ret;
	unsigned int voq;
	unsigned int en;

	FC_PARAM_CHK((pAqmPie_enable == NULL), RTK_FC_RET_ERR_NULL_POINTER);
	FC_PARAM_CHK((pAqmPie_enable->deepq>1), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	FC_PARAM_CHK((pAqmPie_enable->ldpid>=64), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	voq=pAqmPie_enable->ldpid|(pAqmPie_enable->deepq<<6);

	aal_ret=aal_l3_te_glb_pie_wred_port_enable_get(G3_DEF_DEVID, (unsigned char)(voq/32), &en);
	if(aal_ret){
		WARNING("Fail to get aqm pie enable, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}

	pAqmPie_enable->enable=(en>>(voq%32))&0x1;	
	
	return SUCCESS;
}

int32 rtk_fc_qos_aqm_pie_enable_set(rt_qos_aqm_pie_enable_t aqmPie_enable)
{
	int aal_ret;
	unsigned int voq;
	unsigned int voq_bit;
	unsigned int en;

	FC_PARAM_CHK((aqmPie_enable.enable>1), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	FC_PARAM_CHK((aqmPie_enable.deepq>1), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	FC_PARAM_CHK((aqmPie_enable.ldpid>=64), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	voq=aqmPie_enable.ldpid|(aqmPie_enable.deepq<<6);
	voq_bit=0x1<<(voq%32);

	aal_ret=aal_l3_te_glb_pie_wred_port_enable_get(G3_DEF_DEVID, voq/32, &en);
	if(aal_ret){
		WARNING("Fail to get aqm pie enable, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}

	if(aqmPie_enable.enable)
		en|=voq_bit;
	else
		en&=(~voq_bit);

	aal_ret=aal_l3_te_glb_pie_wred_port_enable_set(G3_DEF_DEVID, (unsigned char)(voq/32), en);
	if(aal_ret){
		WARNING("Fail to set aqm pie enable, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}

	return SUCCESS;
}

int32 rtk_fc_qos_aqm_pie_match_get(rt_qos_aqm_pie_match_t* pAqmPie_match)
{
	int aal_ret;
	aal_l3_te_glb_match_t cfg;

	FC_PARAM_CHK((pAqmPie_match == NULL), RTK_FC_RET_ERR_NULL_POINTER);

	aal_ret=aal_l3_te_glb_match_get(G3_DEF_DEVID, pAqmPie_match->profile, &cfg);
	if(aal_ret){
		WARNING("Fail to get aqm pie match, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}

	pAqmPie_match->match=cfg.ena;
	pAqmPie_match->ldpid=cfg.ldpid;
	pAqmPie_match->ldpid_msk=cfg.ldpid_msk;
	pAqmPie_match->cos=cfg.cos;
	pAqmPie_match->cos_msk=cfg.cos_msk;

	return SUCCESS;
}


int32 rtk_fc_qos_aqm_pie_match_set(rt_qos_aqm_pie_match_t aqmPie_match)
{
	int aal_ret;
	aal_l3_te_glb_match_msk_t msk;
	aal_l3_te_glb_match_t cfg;

	FC_PARAM_CHK((aqmPie_match.match>1), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	bzero(&msk, sizeof(msk));
	bzero(&cfg, sizeof(cfg));
	msk.s.ena=1;
	msk.s.ldpid=1;
	msk.s.ldpid_msk=1;
	msk.s.cos=1;
	msk.s.cos_msk=1;
	cfg.ena=aqmPie_match.match;
	cfg.ldpid=aqmPie_match.ldpid;
	cfg.ldpid_msk=aqmPie_match.ldpid_msk;
	cfg.cos=aqmPie_match.cos;
	cfg.cos_msk=aqmPie_match.cos_msk;

	aal_ret=aal_l3_te_glb_match_set(G3_DEF_DEVID, aqmPie_match.profile, msk, &cfg);
	if(aal_ret){
		WARNING("Fail to set aqm pie match, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}

	return SUCCESS;
}

int32 rtk_fc_qos_aqm_pie_mapping_get(rt_qos_aqm_pie_mapping_t* pAqmPie_mapping)
{
	int aal_ret;
	aal_l3_te_pie_lpid_qos_voq_t cfg;

	FC_PARAM_CHK((pAqmPie_mapping == NULL), RTK_FC_RET_ERR_NULL_POINTER);

	aal_ret=aal_l3_te_pie_lpid_qos_voq_get(G3_DEF_DEVID, pAqmPie_mapping->ldpid, pAqmPie_mapping->cos, &cfg);
	if(aal_ret){
		WARNING("Fail to get aqm pie mapping, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}

	pAqmPie_mapping->mapping=cfg.ena;
	pAqmPie_mapping->pie_voq=cfg.voq;

	return SUCCESS;
}


int32 rtk_fc_qos_aqm_pie_mapping_set(rt_qos_aqm_pie_mapping_t aqmPie_mapping)
{
	int aal_ret;
	aal_l3_te_pie_lpid_qos_voq_msk_t msk;
	aal_l3_te_pie_lpid_qos_voq_t cfg;

	FC_PARAM_CHK((aqmPie_mapping.mapping>1), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	bzero(&msk, sizeof(msk));
	bzero(&cfg, sizeof(cfg));
	msk.s.ena=1;
	msk.s.voq=1;
	cfg.ena=aqmPie_mapping.mapping;
	cfg.voq=aqmPie_mapping.pie_voq;

	aal_ret=aal_l3_te_pie_lpid_qos_voq_set(G3_DEF_DEVID, aqmPie_mapping.ldpid, aqmPie_mapping.cos, msk, &cfg);
	if(aal_ret){
		WARNING("Fail to set aqm pie mapping, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}

	return SUCCESS;
}

int32 rtk_fc_qos_aqm_pie_threshold_get(rt_qos_aqm_pie_threshold_t* pAqmPie_threshold)
{
	int aal_ret;

	FC_PARAM_CHK((pAqmPie_threshold == NULL), RTK_FC_RET_ERR_NULL_POINTER);
	FC_PARAM_CHK((pAqmPie_threshold->flow_type>=RT_QOS_AQM_FLOW_TYPE_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	aal_ret=aal_l3_te_pie_buff_threshold_get(G3_DEF_DEVID, pAqmPie_threshold->flow_type, &pAqmPie_threshold->trigger);
	if(aal_ret){
		WARNING("Fail to get aqm pie threshold, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	aal_ret=aal_l3_te_pie_dp_qfull_get(G3_DEF_DEVID, pAqmPie_threshold->flow_type, &pAqmPie_threshold->full);
	if(aal_ret){
		WARNING("Fail to get aqm pie threshold, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	aal_ret=aal_l3_te_pie_mean_pkt_get(G3_DEF_DEVID, pAqmPie_threshold->flow_type, &pAqmPie_threshold->mean);
	if(aal_ret){
		WARNING("Fail to get aqm pie threshold, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}

	return SUCCESS;
}

int32 rtk_fc_qos_aqm_pie_threshold_set(rt_qos_aqm_pie_threshold_t aqmPie_threshold)
{
	int aal_ret;

	FC_PARAM_CHK((aqmPie_threshold.flow_type>=RT_QOS_AQM_FLOW_TYPE_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	aal_ret=aal_l3_te_pie_buff_threshold_set(G3_DEF_DEVID, aqmPie_threshold.flow_type, aqmPie_threshold.trigger);
	if(aal_ret){
		WARNING("Fail to set aqm pie threshold, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	aal_ret=aal_l3_te_pie_dp_qfull_set(G3_DEF_DEVID, aqmPie_threshold.flow_type, aqmPie_threshold.full);
	if(aal_ret){
		WARNING("Fail to set aqm pie threshold, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	aal_ret=aal_l3_te_pie_mean_pkt_set(G3_DEF_DEVID, aqmPie_threshold.flow_type, aqmPie_threshold.mean);
	if(aal_ret){
		WARNING("Fail to set aqm pie threshold, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	
	return SUCCESS;
}


int32 rtk_fc_qos_aqm_pie_dpScale_get(rt_qos_aqm_pie_dpScale_t* pAqmPie_dpScale)
{
	int aal_ret;
	aal_l3_te_pie_dp_scaling_t scale;

	FC_PARAM_CHK((pAqmPie_dpScale == NULL), RTK_FC_RET_ERR_NULL_POINTER);
	FC_PARAM_CHK((pAqmPie_dpScale->flow_type>=RT_QOS_AQM_FLOW_TYPE_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	aal_ret=aal_l3_te_pie_dp_scaling_get(G3_DEF_DEVID, pAqmPie_dpScale->flow_type, &scale);
	if(aal_ret){
		WARNING("Fail to get aqm pie dpScale, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	pAqmPie_dpScale->alpha=scale.value_a;
	pAqmPie_dpScale->beta=scale.value_b;
	
	return SUCCESS;
}


int32 rtk_fc_qos_aqm_pie_dpScale_set(rt_qos_aqm_pie_dpScale_t aqmPie_dpScale)
{
	int aal_ret;
	aal_l3_te_pie_dp_scaling_msk_t msk;
	aal_l3_te_pie_dp_scaling_t scale;

	FC_PARAM_CHK((aqmPie_dpScale.flow_type>=RT_QOS_AQM_FLOW_TYPE_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	bzero(&msk, sizeof(msk));
	bzero(&scale, sizeof(scale));
	msk.s.value_a=1;
	msk.s.value_b=1;
	scale.value_a=aqmPie_dpScale.alpha;
	scale.value_b=aqmPie_dpScale.beta;
	aal_ret=aal_l3_te_pie_dp_scaling_set(G3_DEF_DEVID, aqmPie_dpScale.flow_type, msk, &scale);
	if(aal_ret){
		WARNING("Fail to set aqm pie dpScale, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	
	return SUCCESS;
}

int32 rtk_fc_qos_aqm_pie_dpThreshold_get(rt_qos_aqm_pie_dpThreshold_t* pAqmPie_dpThreshold)
{
	int aal_ret;
	aal_l3_te_pie_dp_glb_cfg_t cfg;

	FC_PARAM_CHK((pAqmPie_dpThreshold == NULL), RTK_FC_RET_ERR_NULL_POINTER);
	FC_PARAM_CHK((pAqmPie_dpThreshold->flow_type>=RT_QOS_AQM_FLOW_TYPE_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	aal_ret=aal_l3_te_pie_dp_ceiling_get(G3_DEF_DEVID, pAqmPie_dpThreshold->flow_type, &pAqmPie_dpThreshold->ceiling);
	if(aal_ret){
		WARNING("Fail to get aqm pie dpThreshold, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	aal_ret=aal_l3_te_pie_dp_glb_cfg_get(G3_DEF_DEVID, pAqmPie_dpThreshold->flow_type, &cfg);
	if(aal_ret){
		WARNING("Fail to get aqm pie dpThreshold, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	pAqmPie_dpThreshold->accu_ceiling=cfg.high;
	pAqmPie_dpThreshold->accu_floor=cfg.low;

	return SUCCESS;
}

int32 rtk_fc_qos_aqm_pie_dpThreshold_set(rt_qos_aqm_pie_dpThreshold_t aqmPie_dpThreshold)
{
	int aal_ret;
	aal_l3_te_pie_dp_glb_msk_t msk;
	aal_l3_te_pie_dp_glb_cfg_t cfg;

	FC_PARAM_CHK((aqmPie_dpThreshold.flow_type>=RT_QOS_AQM_FLOW_TYPE_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	bzero(&msk, sizeof(msk));
	bzero(&cfg, sizeof(cfg));
	msk.s.high=1;
	msk.s.low=1;
	cfg.high=aqmPie_dpThreshold.accu_ceiling;
	cfg.low=aqmPie_dpThreshold.accu_floor;
	aal_ret=aal_l3_te_pie_dp_ceiling_set(G3_DEF_DEVID, aqmPie_dpThreshold.flow_type, aqmPie_dpThreshold.ceiling);
	if(aal_ret){
		WARNING("Fail to set aqm pie dpThreshold, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	aal_ret=aal_l3_te_pie_dp_glb_cfg_set(G3_DEF_DEVID, aqmPie_dpThreshold.flow_type, msk, &cfg);
	if(aal_ret){
		WARNING("Fail to set aqm pie dpThreshold, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	
	return SUCCESS;
}

int32 rtk_fc_qos_aqm_pie_dpBurst_get(rt_qos_aqm_pie_dpBurst_t* pAqmPie_dpBurst)
{
	int aal_ret;
	aal_l3_te_pie_dp_glb_cfg_t cfg;
	aal_l3_te_pie_timer_cfg_t timer;

	FC_PARAM_CHK((pAqmPie_dpBurst == NULL), RTK_FC_RET_ERR_NULL_POINTER);
	FC_PARAM_CHK((pAqmPie_dpBurst->flow_type>=RT_QOS_AQM_FLOW_TYPE_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	aal_ret=aal_l3_te_pie_dp_glb_cfg_get(G3_DEF_DEVID, pAqmPie_dpBurst->flow_type, &cfg);
	if(aal_ret){
		WARNING("Fail to get aqm pie dpBurst, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	aal_ret=aal_l3_te_pie_timer_get(G3_DEF_DEVID, pAqmPie_dpBurst->flow_type, &timer);
	if(aal_ret){
		WARNING("Fail to get aqm pie dpBurst, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	pAqmPie_dpBurst->allowance=cfg.burst;
	pAqmPie_dpBurst->decrement=timer.burst_dec;
	
	return SUCCESS;
}

int32 rtk_fc_qos_aqm_pie_dpBurst_set(rt_qos_aqm_pie_dpBurst_t aqmPie_dpBurst)
{
	int aal_ret;
	aal_l3_te_pie_dp_glb_msk_t msk;
	aal_l3_te_pie_dp_glb_cfg_t cfg;
	aal_l3_te_pie_timer_msk_t timer_msk;
	aal_l3_te_pie_timer_cfg_t timer;

	FC_PARAM_CHK((aqmPie_dpBurst.flow_type>=RT_QOS_AQM_FLOW_TYPE_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	bzero(&msk, sizeof(msk));
	bzero(&cfg, sizeof(cfg));
	msk.s.burst=1;
	cfg.burst=aqmPie_dpBurst.allowance;
	aal_ret=aal_l3_te_pie_dp_glb_cfg_set(G3_DEF_DEVID, aqmPie_dpBurst.flow_type, msk, &cfg);
	if(aal_ret){
		WARNING("Fail to set aqm pie dpBurst, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	bzero(&timer_msk, sizeof(timer_msk));
	bzero(&timer, sizeof(timer));
	timer_msk.s.burst_dec=1;
	timer.burst_dec=aqmPie_dpBurst.decrement;
	aal_ret=aal_l3_te_pie_timer_set(G3_DEF_DEVID, aqmPie_dpBurst.flow_type, timer_msk, &timer);
	if(aal_ret){
		WARNING("Fail to set aqm pie dpBurst, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}

	return SUCCESS;
}


int32 rtk_fc_qos_aqm_pie_qdelay_get(rt_qos_aqm_pie_qdelay_t* pAqmPie_qdelay)
{
	int aal_ret;
	aal_l3_te_pie_dp_cfg_t cfg;

	FC_PARAM_CHK((pAqmPie_qdelay == NULL), RTK_FC_RET_ERR_NULL_POINTER);
	FC_PARAM_CHK((pAqmPie_qdelay->flow_type>=RT_QOS_AQM_FLOW_TYPE_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	aal_ret=aal_l3_te_pie_dp_cfg_get(G3_DEF_DEVID, pAqmPie_qdelay->flow_type, pAqmPie_qdelay->pie_voq, &cfg);
	if(aal_ret){
		WARNING("Fail to get aqm pie qdelay, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	pAqmPie_qdelay->burst_reset=cfg.burst_reset_timeout;
	pAqmPie_qdelay->high_latency=cfg.latency_threshold_hi;
	pAqmPie_qdelay->low_latency=cfg.latency_threshold_lo;
	pAqmPie_qdelay->minimum_target=cfg.latency_target;
	
	return SUCCESS;
}

int32 rtk_fc_qos_aqm_pie_qdelay_set(rt_qos_aqm_pie_qdelay_t aqmPie_qdelay)
{
	int aal_ret;
	aal_l3_te_pie_dp_msk_t msk;
	aal_l3_te_pie_dp_cfg_t cfg;

	FC_PARAM_CHK((aqmPie_qdelay.flow_type>=RT_QOS_AQM_FLOW_TYPE_MAX), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	bzero(&msk, sizeof(msk));
	bzero(&cfg, sizeof(cfg));
	msk.s.burst_reset_timeout=1;
	msk.s.latency_threshold_hi=1;
	msk.s.latency_threshold_lo=1;
	msk.s.latency_target=1;
	cfg.burst_reset_timeout=aqmPie_qdelay.burst_reset;
	cfg.latency_threshold_hi=aqmPie_qdelay.high_latency;
	cfg.latency_threshold_lo=aqmPie_qdelay.low_latency;
	cfg.latency_target=aqmPie_qdelay.minimum_target;
	aal_ret=aal_l3_te_pie_dp_cfg_set(G3_DEF_DEVID, aqmPie_qdelay.flow_type, aqmPie_qdelay.pie_voq, msk, &cfg);
	if(aal_ret){
		WARNING("Fail to set aqm pie qdelay, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}

	return SUCCESS;
}


int32 rtk_fc_qos_aqm_pie_rate_get(rt_qos_aqm_pie_rate_t* pAqmPie_rate)
{
	int aal_ret;
	aal_l3_te_pie_dp_cfg_t cfg;

	FC_PARAM_CHK((pAqmPie_rate == NULL), RTK_FC_RET_ERR_NULL_POINTER);

	aal_ret=aal_l3_te_pie_dp_cfg_get(G3_DEF_DEVID, RT_QOS_AQM_FLOW_TYPE_SMALL, pAqmPie_rate->pie_voq, &cfg);
	if(aal_ret){
		WARNING("Fail to get aqm pie rate, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	pAqmPie_rate->msr=cfg.max_service_rate;
	pAqmPie_rate->peak=cfg.peak_rate;

	return SUCCESS;
}

int32 rtk_fc_qos_aqm_pie_rate_set(rt_qos_aqm_pie_rate_t aqmPie_rate)
{
	int aal_ret;
	aal_l3_te_pie_dp_msk_t msk;
	aal_l3_te_pie_dp_cfg_t cfg;

	bzero(&msk, sizeof(msk));
	bzero(&cfg, sizeof(cfg));
	msk.s.max_service_rate=1;
	msk.s.peak_rate=1;
	cfg.max_service_rate=aqmPie_rate.msr;
	cfg.peak_rate=aqmPie_rate.peak;
	aal_ret=aal_l3_te_pie_dp_cfg_set(G3_DEF_DEVID, RT_QOS_AQM_FLOW_TYPE_SMALL, aqmPie_rate.pie_voq, msk, &cfg);
	if(aal_ret){
		WARNING("Fail to set aqm pie rate, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}

	return SUCCESS;
}

int32 rtk_fc_qos_aqm_pie_cfg_get(rt_qos_aqm_pie_cfg_t* pAqmPie_cfg)
{
	int aal_ret;
	aal_l3_te_pie_dp_glb_cfg_t cfg;
	aal_l3_te_pie_timer_cfg_t timer;

	FC_PARAM_CHK((pAqmPie_cfg == NULL), RTK_FC_RET_ERR_NULL_POINTER);

	aal_ret=aal_l3_te_pie_dp_glb_cfg_get(G3_DEF_DEVID, RT_QOS_AQM_FLOW_TYPE_SMALL, &cfg);
	if(aal_ret){
		WARNING("Fail to get aqm pie cfg, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	aal_ret=aal_l3_te_pie_timer_get(G3_DEF_DEVID, RT_QOS_AQM_FLOW_TYPE_SMALL, &timer);
	if(aal_ret){
		WARNING("Fail to get aqm pie cfg, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	pAqmPie_cfg->mode=cfg.tbc_mode;
	pAqmPie_cfg->timer_interval=timer.interval;
	pAqmPie_cfg->timer_prescalar=timer.scalar;
	pAqmPie_cfg->dp_scale=cfg.drop_prob_scale;
	pAqmPie_cfg->random=cfg.random_disable;
	pAqmPie_cfg->fix_LFSR=cfg.threshold;
	
	return SUCCESS;
}

int32 rtk_fc_qos_aqm_pie_cfg_set(rt_qos_aqm_pie_cfg_t aqmPie_cfg)
{
	int aal_ret;
	aal_l3_te_pie_dp_glb_msk_t msk;
	aal_l3_te_pie_dp_glb_cfg_t cfg;
	aal_l3_te_pie_timer_msk_t timer_msk;
	aal_l3_te_pie_timer_cfg_t timer;

	bzero(&msk, sizeof(msk));
	bzero(&cfg, sizeof(cfg));
	msk.s.tbc_mode=1;
	msk.s.drop_prob_scale=1;
	msk.s.random_disable=1;
	msk.s.threshold=1;
	cfg.tbc_mode=aqmPie_cfg.mode;
	cfg.drop_prob_scale=aqmPie_cfg.dp_scale;
	cfg.random_disable=aqmPie_cfg.random;
	cfg.threshold=aqmPie_cfg.fix_LFSR;
	aal_ret=aal_l3_te_pie_dp_glb_cfg_set(G3_DEF_DEVID, RT_QOS_AQM_FLOW_TYPE_SMALL, msk, &cfg);
	if(aal_ret){
		WARNING("Fail to set aqm pie cfg, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}
	bzero(&timer_msk, sizeof(timer_msk));
	bzero(&timer, sizeof(timer));
	timer_msk.s.interval=1;
	timer_msk.s.scalar=1;
	timer.interval=aqmPie_cfg.timer_interval;
	timer.scalar=aqmPie_cfg.timer_prescalar;
	aal_ret=aal_l3_te_pie_timer_set(G3_DEF_DEVID, RT_QOS_AQM_FLOW_TYPE_SMALL, timer_msk, &timer);
	if(aal_ret){
		WARNING("Fail to set aqm pie dpBurst, aal_ret = %d", aal_ret);
		return RTK_FC_RET_ERR;
	}

	return SUCCESS;
}
#endif

#if defined(CONFIG_RTK_FC_HTTP_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
int rtk_fc_pe_http_test(rt_pe_http_test_request_t http_test_req, rt_pe_http_test_result_t *pHttp_test_result)
{
	ca_status_t ret;
	ca_ipc_pkt_t ipc_pkt;
	ca_uint16_t result_size = sizeof(rt_pe_http_test_result_t);
	rtk_http_test_request_t http_req_info;
	uint32 ret_fail = 0, i;
	
	memset(&http_req_info, 0, sizeof(rtk_http_test_request_t));
	memset(pHttp_test_result, 0, sizeof(rt_pe_http_test_result_t));	

	EVENT("req_cmd=%d", http_test_req.req_cmd);
	
	if(http_test_req.req_cmd==RT_PE_HTTP_TEST_CMD_DOWNLOAD_START 
		|| http_test_req.req_cmd==RT_PE_HTTP_TEST_CMD_UPLOAD_START
		|| http_test_req.req_cmd==RT_PE_HTTP_TEST_CMD_LATENCY_START)
	{
		uint32 max_connection_num = (http_test_req.req_cmd==RT_PE_HTTP_TEST_CMD_DOWNLOAD_START) ? MAX_PE_HTTP_DOWNLOAD_CONNECTION_NUM : MAX_PE_HTTP_UPLOAD_CONNECTION_NUM;

		//allocate buffer
		if(fc_db.pe_http_pageInfo.pPages)
		{
			WARNING("[HTTP Test] Fail to start http ... due to connection is existed !");
			return FAILED;
		}
		if(_rtk_fc_page_alloc(&fc_db.pe_http_pageInfo) != RTK_FC_RET_OK)
		{
			WARNING("[HTTP Test] Fail to allocate page buffer !");
			return FAILED;
		}
		http_req_info.buf_phy_addr = fc_db.pe_http_pageInfo.buf_phy_addr;
		http_req_info.buf_size = fc_db.pe_http_pageInfo.buf_size;
#if 1
		if(http_test_req.test_mode >= RTK_PE_HTTP_TEST_MODE_MAX)
		{
			WARNING("[HTTP Test] test_mode %d is not support !!", http_test_req.test_mode);
			return FAILED;
		}
#endif
		http_req_info.test_mode = http_test_req.test_mode;
		memcpy(&http_req_info.client_mac, &http_test_req.client_mac, sizeof(rtk_mac_t));
		memcpy(&http_req_info.server_mac, &http_test_req.server_mac, sizeof(rtk_mac_t));
		http_req_info.isIPv4OrIpv6 = http_test_req.isIPv4OrIpv6;
		if(http_req_info.isIPv4OrIpv6==0)
		{
			memcpy(&http_req_info.client_ip.ipv4_addr, &http_test_req.client_ip.ipv4_addr, sizeof(rtk_ip_addr_t));
			memcpy(&http_req_info.server_ip.ipv4_addr, &http_test_req.server_ip.ipv4_addr, sizeof(rtk_ip_addr_t));
		}
		else
		{
			memcpy(&http_req_info.client_ip.ipv6_addr, &http_test_req.client_ip.ipv6_addr, sizeof(rtk_ipv6_addr_t));
			memcpy(&http_req_info.server_ip.ipv6_addr, &http_test_req.server_ip.ipv6_addr, sizeof(rtk_ipv6_addr_t));
		}
		http_req_info.client_l4port = http_test_req.client_l4port;
		http_req_info.server_l4port = http_test_req.server_l4port;
		http_req_info.ldpid = http_test_req.ldpid;
#if 0	
		memcpy(http_req_info.http_req_url, http_test_req.http_req_url, MAX_PE_HTTP_REQ_URL_STR_LENGTH);
#else
		{
			char *strptr=http_test_req.http_req_url, *split_str;
			int idx=0;
			do{			// /garbage.php%3FckSize=5000
				split_str=strsep(&strptr, "%");
				strncpy(&http_req_info.http_req_url[idx], split_str, strlen(split_str));
				idx += strlen(split_str);
				if(strptr!=NULL && strlen(strptr)>=2)
				{
					int val;
					char strValue[2];
			
					strncpy(strValue, strptr, 2);
					strptr +=2;
					val = simple_strtol(strValue, NULL, 16);
					http_req_info.http_req_url[idx] = val;
					idx++;
					
				}
			}while(strptr!=NULL);
			EVENT("http_req_url=%s", http_req_info.http_req_url);
		}
#endif
		memcpy(http_req_info.http_version, http_test_req.http_version, MAX_PE_HTTP_VERSION_STR_LENGTH);
		http_req_info.non_congestion_mode = http_test_req.non_congestion_mode;
		http_req_info.upload_content_length = http_test_req.upload_content_length;
		http_req_info.tcp_window_size = http_test_req.tcp_window_size;
		//tcp mss size
		if(http_test_req.tcp_mss_size == 0)
		{
			http_req_info.tcp_mss_size = 1500 - (http_test_req.isIPv4OrIpv6 ? 40 : 20)/*ip hdr*/ - 20/*tcp hdr*/;
			if(http_test_req.pppoe_sid != FAIL)
				http_req_info.tcp_mss_size -= 8 /*pppoe hdr*/ ;
		}
		else
		{
			http_req_info.tcp_mss_size = (uint32)http_test_req.tcp_mss_size;
		}

		if(http_test_req.req_cmd==RT_PE_HTTP_TEST_CMD_LATENCY_START)
		{
			if(http_test_req.connection_number != 1)
			{
				WARNING("[HTTP Test] Latency test ... forcibly set its connection number to 1");
			}
			http_req_info.connection_number = 1;
		}
		else
		{
			if(http_test_req.connection_number == 0)
			{
				WARNING("[HTTP Test] connection_number(%u) can not be zero ... forcibly set it to 1", http_test_req.connection_number);
				http_req_info.connection_number = 1;
			}
			else if(http_test_req.connection_number > max_connection_num)
			{
				WARNING("[HTTP Test] connection_number(%u) is large than max value(%u) ... forcibly set it to max value", http_test_req.connection_number, max_connection_num);
				http_req_info.connection_number = max_connection_num;
			}
			else
			{
				http_req_info.connection_number = http_test_req.connection_number;
			}
		}

		//reset to default
		http_req_info.dma_aft_idx = FAIL;
		http_req_info.streamId.sid = FAIL;
		http_req_info.streamId.ldpid = FAIL;
		if(http_test_req.pppoe_sid!=FAIL || http_test_req.svlan_vid!=FAIL || http_test_req.cvlan_vid!=FAIL)
		{
			uint32 fibIdx;
			rtk_rg_asic_dmaAftFib_t fib;

			memset(&fib, 0, sizeof(rtk_rg_asic_dmaAftFib_t));

			if(http_test_req.svlan_vid!=FAIL && http_test_req.cvlan_vid!=FAIL)
			{
				fib.vlan_vld = 1;
				//egress: stag + ctag
				fib.vlan_cnt = 2;
				fib.top_tpid_enc = 2; // TPID_0: 0x88a8
				fib.top_tpid_sel = 3; // fib
				fib.top_vid = http_test_req.svlan_vid;
				fib.top_802_1p = 0;
				fib.top_1p_sel = 3; // fib
				fib.top_dei = 0;
				fib.top_dei_sel = 3; //fib

				fib.inner_tpid_enc = 1; // TPID_0: 0x8100
				fib.inner_tpid_sel = 3; // fib
				fib.inner_vid = http_test_req.cvlan_vid;
				fib.inner_802_1p = 0;
				fib.inner_1p_sel = 3; // fib
				fib.inner_dei = 0;
				fib.inner_dei_sel = 3; //fib

				http_req_info.dma_aft_hdr_offset += 8;
			}
			else if(http_test_req.svlan_vid!=FAIL)
			{
				fib.vlan_vld = 1;
				//egress:  only stag
				fib.vlan_cnt = 1;
				fib.top_tpid_enc = 2; // TPID_0: 0x88a8
				fib.top_tpid_sel = 3; // fib
				fib.top_vid = http_test_req.svlan_vid;
				fib.top_802_1p = 0;
				fib.top_1p_sel = 3; // fib
				fib.top_dei = 0;
				fib.top_dei_sel = 3; //fib

				http_req_info.dma_aft_hdr_offset += 4;
			}
			else if(http_test_req.cvlan_vid!=FAIL)
			{
				fib.vlan_vld = 1;
				//egress:  ctag
				fib.vlan_cnt = 1;
				fib.top_tpid_enc = 1; // TPID_0: 0x8100
				fib.top_tpid_sel = 3; // fib
				fib.top_vid = http_test_req.cvlan_vid;
				fib.top_802_1p = 0;
				fib.top_1p_sel = 3; // fib
				fib.top_dei = 0;
				fib.top_dei_sel = 3; //fib

				http_req_info.dma_aft_hdr_offset += 4;
			}
			else
			{
				//egress: untag
			}

			if(http_test_req.pppoe_sid!=FAIL)
			{
				fib.pppoe_cmd = 1; // push
				fib.session_id = http_test_req.pppoe_sid;

				http_req_info.dma_aft_hdr_offset += 8;
			}

			if(_rtk_fc_dmaAftAction_add(&fibIdx, fib, TRUE) != RTK_FC_RET_OK)
			{
				WARNING("Fail to add dma AFT table!!");
				return FAILED;
			}
			http_req_info.dma_aft_idx = fibIdx;
			DEBUG("[HTTP] dma_aft_idx=%d", http_req_info.dma_aft_idx);
			//store dma_aft index
			fc_db.http_test_dma_aft_idx = fibIdx;
		}

		if(http_test_req.pon_streamId!=FAIL && http_test_req.pon_streamId<RTK_FC_TABLESIZE_STREAMID)
		{
			http_req_info.streamId.sid = http_test_req.pon_streamId;
			http_req_info.streamId.ldpid 	= fc_db.streamidTbl[http_test_req.pon_streamId].ldpid;
			http_req_info.streamId.cos 		= fc_db.streamidTbl[http_test_req.pon_streamId].cos;
			http_req_info.streamId.flowid 	= fc_db.streamidTbl[http_test_req.pon_streamId].gemid;		
		}
		//add reserved acl 
		{
			rtk_fc_aclAndCf_reserved_accelerate_by_pe_t acc_by_pe;

			for(i=0; i<http_req_info.connection_number; i++)
			{
				memset(&acc_by_pe, 0, sizeof(acc_by_pe));
				acc_by_pe.igr_port_idx = http_req_info.ldpid;
				if(http_req_info.isIPv4OrIpv6==0)
				{
					acc_by_pe.is_ipv4 = 1;
					memcpy(&acc_by_pe.ipv4_src_ip, &http_req_info.server_ip.ipv4_addr, sizeof(rtk_ip_addr_t));
					memcpy(&acc_by_pe.ipv4_dst_ip, &http_req_info.client_ip.ipv4_addr, sizeof(rtk_ip_addr_t));
				}
				else
				{
					acc_by_pe.is_ipv4 = 0;
					memcpy(&acc_by_pe.ipv6_src_ip, &http_req_info.server_ip.ipv6_addr, sizeof(rtk_ipv6_addr_t));
					memcpy(&acc_by_pe.ipv6_dst_ip, &http_req_info.client_ip.ipv6_addr, sizeof(rtk_ipv6_addr_t));
				}
				acc_by_pe.l4_proto_value = 0x6; //tcp
				acc_by_pe.l4_sport = http_req_info.server_l4port;
				acc_by_pe.l4_dport = http_req_info.client_l4port+i;
				acc_by_pe.action_redirect_ldpid = RT_PE_HTTP_TEST_CPU_PORT;
				acc_by_pe.action_pol_id = i;
				_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_HTTP_ACCELERATE_BY_PE, &acc_by_pe);
				ACL_RSV("[HTTP] cls rslt=0x%x acc_by_pe.action_pol_id %d", acc_by_pe.rslt_idx, acc_by_pe.action_pol_id);
				//store cls index
				fc_db.http_test_cls_idx[i] = acc_by_pe.rslt_idx;
			}
		}
		if(http_test_req.req_cmd==RT_PE_HTTP_TEST_CMD_UPLOAD_START)
		{
			rt_ponmisc_ponMode_t ponMode = RT_PONMODE_END;
    		rt_ponmisc_ponSpeed_t ponSpeed = RT_PONSPEED_END;
			int32 regValue, ret;
#if defined(RT_PE_HTTP_TEST_DMA_LSO_BACKPRESSURE)
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
			//dma_lso backpressure
			fc_db.http_test_dma_lso_regValue[0] = rtk_dma_lso_reg_read(DMA_SEC_DMA_LSO_VP_CONTROL + (DMA_SEC_DMA_LSO_VP_STRIDE * RT_PE_HTTP_TEST_DMA_LSO_CPU_VP_ID));
			fc_db.http_test_dma_lso_regValue[1] = rtk_ne_reg_read(TE_CB_WIFI_SSID_MAP); // only works on 8277B
			fc_db.http_test_dma_lso_regValue[2] = rtk_ne_reg_read(TE_CB_WIFI_CTRL);
			fc_db.http_test_dma_lso_regValue[3] = rtk_ne_reg_read(TE_CB_WIFI_BUF_THRSH);
			rtk_dma_lso_reg_write(fc_db.http_test_dma_lso_regValue[0]|0x100, DMA_SEC_DMA_LSO_VP_CONTROL + (DMA_SEC_DMA_LSO_VP_STRIDE * RT_PE_HTTP_TEST_DMA_LSO_CPU_VP_ID)); //enable backpressure from L3 QM
			rtk_ne_reg_write(fc_db.http_test_dma_lso_regValue[1]|0x1, TE_CB_WIFI_SSID_MAP); //for RT_PE_HTTP_TEST_DMA_LSO_LSPID
			rtk_ne_reg_write(fc_db.http_test_dma_lso_regValue[2]|0x1, TE_CB_WIFI_CTRL); //enable dma_lso backpressure
			rtk_ne_reg_write(0x07D003E8, TE_CB_WIFI_BUF_THRSH); //2000~1000
#else	//CONFIG_FC_RTL9607F_SERIES
{
			rtk_asic_dmalso_bp_conf_t vp_bp_conf = {0};		 
			vp_bp_conf.lspid_vld[0] 	= 1;
			vp_bp_conf.lspid[0] 		= RT_PE_HTTP_TEST_DMA_LSO_LSPID;
			vp_bp_conf.vpId_vld[0] 		= 1;
			vp_bp_conf.vpId[0] 			= RT_PE_HTTP_TEST_DMA_LSO_CPU_VP_ID;
			vp_bp_conf.threshold_hi 	= 2000;
			vp_bp_conf.threshold_lo 	= 1000;
			if(rtk_asic_dmalso_backpressure_rule_add(&vp_bp_conf)==ASIC_RET_SUCCESS)
			{
				EVENT("Success to add dmalso backpressure ... rule_idx=%u", vp_bp_conf.rule_idx);
				fc_db.http_test_dma_lso_regValue[0] = vp_bp_conf.rule_idx;
			}
			else
			{
				WARNING("Fail to set dmalso backpressure.");
				fc_db.http_test_dma_lso_regValue[0] = FAIL;
			}
}
#endif
#endif		
			//dma lso shaper for upload
			//backup original value
			regValue = RT_PE_DMA_LSO_SHAPER_RATE_10G;
			ret = rt_ponmisc_modeSpeed_get(&ponMode,&ponSpeed);
			if(ret==RT_ERR_OK)
			{
				EVENT("========== [PON mode] ==========");
			    if(ponMode == RT_GPON_MODE)
			    {
			        if(ponSpeed == RT_1G25G_SPEED)
			        {
			            EVENT("GPON 2.5G/1.25G");
						regValue = RT_PE_DMA_LSO_SHAPER_RATE_1DOT25G;
			        }
			        else if(ponSpeed == RT_DN10G_SPEED)
			        {
			            EVENT("XG-PON 10G/2.5G");
						regValue = RT_PE_DMA_LSO_SHAPER_RATE_2DOT5G;
			        }
			        else if(ponSpeed == RT_BOTH10G_SPEED)
			        {
			            EVENT("XGS-PON 10G/10G");
						regValue = RT_PE_DMA_LSO_SHAPER_RATE_10G;
			        }
			    }
			    else if(ponMode == RT_EPON_MODE)
			    {
			        if(ponSpeed == RT_1G25G_SPEED)
			        {
			            EVENT("EPON 2.5G/1.25G");
						regValue = RT_PE_DMA_LSO_SHAPER_RATE_1DOT25G;
			        }
			        else if(ponSpeed == RT_DN10G_SPEED)
			        {
			            EVENT("EPON 10G/1.25G");
						regValue = RT_PE_DMA_LSO_SHAPER_RATE_1DOT25G;
			        }
			        else if(ponSpeed == RT_BOTH10G_SPEED)
			        {
			            EVENT("EPON 10G/10G");
						regValue = RT_PE_DMA_LSO_SHAPER_RATE_10G;
			        }
			    }
#if 0				
			    else if(ponMode == RT_NGPON2_MODE)
			    {
			        if(ponSpeed == RT_DN10G_SPEED)
			        {
			            EVENT("NG-PON2 10G/2.5G");
						regValue = RT_PE_DMA_LSO_SHAPER_RATE_2DOT5G;
			        }
			        else if(ponSpeed == RT_BOTH10G_SPEED)
			        {
			            EVENT("NG-PON2 10G/10G");
						regValue = RT_PE_DMA_LSO_SHAPER_RATE_10G;
			        }
			    }
#endif				
				else
					EVENT("Not GPON/EPON");
			}
			else
			{
				WARNING("Fail to rt_ponmisc_modeSpeed_get, ret=0x%x", ret);
			}
			fc_db.http_test_dma_lso_regValue[4] = rtk_dma_lso_reg_read(DMA_SEC_SS_CTRL);
			fc_db.http_test_dma_lso_regValue[5] = rtk_dma_lso_reg_read(DMA_SEC_SS_SHP_PORT_TBC_MEM_DATA1);
			fc_db.http_test_dma_lso_regValue[6] = rtk_dma_lso_reg_read(DMA_SEC_SS_SHP_PORT_TBC_MEM_DATA0);
			fc_db.http_test_dma_lso_regValue[7] = rtk_dma_lso_reg_read(DMA_SEC_SS_SHP_PORT_TBC_MEM_ACCESS);
			rtk_dma_lso_reg_write(0xF0000FFF, DMA_SEC_SS_CTRL);
			rtk_dma_lso_reg_write(0x00000090, DMA_SEC_SS_SHP_PORT_TBC_MEM_DATA1);
			rtk_dma_lso_reg_write(regValue, DMA_SEC_SS_SHP_PORT_TBC_MEM_DATA0);
			rtk_dma_lso_reg_write(0xc0000000|RT_PE_HTTP_TEST_DMA_LSO_CPU_VP_ID, DMA_SEC_SS_SHP_PORT_TBC_MEM_ACCESS);	
			//dma lso axi
			fc_db.http_test_dma_lso_regValue[8] = rtk_dma_lso_reg_read(DMA_SEC_DMA_GLB_DMA_LSO_AXI_USER_SEL0);
			fc_db.http_test_dma_lso_regValue[9] = rtk_dma_lso_reg_read(DMA_SEC_DMA_GLB_AXI_USER_PAT3);
			rtk_dma_lso_reg_write(fc_db.http_test_dma_lso_regValue[8]|(0x3<<(RT_PE_HTTP_TEST_DMA_LSO_CPU_VP_ID*2)), DMA_SEC_DMA_GLB_DMA_LSO_AXI_USER_SEL0);
			rtk_dma_lso_reg_write(fc_db.http_test_dma_lso_regValue[9]|(0xf<<18), DMA_SEC_DMA_GLB_AXI_USER_PAT3);
		}
	}

	//send ipc msg
	ipc_pkt.session_id = CA_IPC_SESSION_HTTP_TEST;
	ipc_pkt.dst_cpu_id = fc_db.controlFuc.func_on_pe_cpu_num[RT_PE_FUNC_NUM_HTTP] + CA_IPC_CPU_PE0;
	ipc_pkt.priority = 0;
	ipc_pkt.msg_no = http_test_req.req_cmd;
	ipc_pkt.msg_data = &http_req_info;						/* the message data to transmit. */
    ipc_pkt.msg_size = sizeof(rtk_http_test_request_t);		/* the length of message data. */
	ret = ca_ipc_msg_sync_send(&ipc_pkt, pHttp_test_result, &result_size);
	if (ret != CA_E_OK)
	{
		WARNING("ca_ipc_msg_sync_send() fail ret %d\n", ret);
		ret_fail = 1;
	}
	else
	{
		EVENT("Success to send ipc sync msg to PE");
		if(pHttp_test_result->ret_val != RT_PE_RET_OK)
		{
			if((http_test_req.req_cmd==RT_PE_HTTP_TEST_CMD_DOWNLOAD_STOP 
				|| http_test_req.req_cmd==RT_PE_HTTP_TEST_CMD_UPLOAD_STOP
				|| http_test_req.req_cmd==RT_PE_HTTP_TEST_CMD_LATENCY_STOP)
				&& (pHttp_test_result->ret_val==RT_PE_RET_NOT_FOUND))
			{
				EVENT("[HTTP Test] Try to stop connection but not found ...");
				pHttp_test_result->ret_val = RT_PE_RET_OK;
				if(fc_db.pe_http_pageInfo.pPages)
					WARNING("[HTTP Test] Forcibly free page due to connection is not existed !!");
				_rtk_fc_pe_http_test_free_resource();
			}
			else
			{
				WARNING("PE return failed ret %d\n", pHttp_test_result->ret_val);
				ret_fail = 1;
			}
		}
	}	
	
	if(ret_fail)
	{
		if(http_test_req.req_cmd==RT_PE_HTTP_TEST_CMD_DOWNLOAD_START 
			|| http_test_req.req_cmd==RT_PE_HTTP_TEST_CMD_UPLOAD_START
			|| http_test_req.req_cmd==RT_PE_HTTP_TEST_CMD_LATENCY_START)
		{
			_rtk_fc_pe_http_test_free_resource();
		}
	}
	
	return SUCCESS;
}
#endif

#if defined(CONFIG_RTK_FC_CRYPTO_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
rt_pe_ret_t rtk_fc_pe_crypto_test(rt_pe_crypto_request_t crypto_req, int32 fc_saInfo_idx, uint32 wait_pe_return)
{
	rt_pe_ret_t ret_val = RT_PE_RET_FAIL;
	int16 pppoe_sid, svlan_vid, cvlan_vid, pon_streamId;
	ca_status_t ret;
	ca_ipc_pkt_t ipc_pkt;
	ca_uint16_t result_size = sizeof(rt_pe_ret_t);
	rtk_fc_aclAndCf_reserved_ipsec_accelerate_by_pe_t ipsec_accelerate_by_pe = {0};
	uint32 ret_fail = 0;

	switch(crypto_req.req_cmd)
	{
		case RT_PE_CRYPTO_ENGINE_CMD_ENABLE:
			//allocate buffer
			if(fc_db.pe_crypto_pageInfo.pPages)
			{
				WARNING("[PE IPSEC] Fail to start PE IPSEC ... due to it is existed !");
				return RT_PE_RET_FAIL;
			}
			if(_rtk_fc_page_alloc(&fc_db.pe_crypto_pageInfo) != RTK_FC_RET_OK)
			{
				WARNING("[PE IPSEC] Fail to allocate page buffer !");
				return RT_PE_RET_FAIL;
			}
			crypto_req.buf_phy_addr = fc_db.pe_crypto_pageInfo.buf_phy_addr;
			crypto_req.buf_size = fc_db.pe_crypto_pageInfo.buf_size;
			crypto_req.ps_wait_desc_done_phy_addr = fc_db.pe_crypto_ps_desc_info.wait_desc_done_phy_addr;
			crypto_req.ps_wait_pop_done_phy_addr = fc_db.pe_crypto_ps_desc_info.wait_pop_done_phy_addr;
			EVENT("[PE IPSEC] ps_wait_desc_done_phy_addr 0x%x ps_wait_pop_done_phy_addr 0x%x", crypto_req.ps_wait_desc_done_phy_addr, crypto_req.ps_wait_pop_done_phy_addr);
			break;
		case RT_PE_CRYPTO_ENGINE_CMD_DISABLE:
			break;
		case RT_PE_CRYPTO_ENGINE_CMD_ENCRYPT_CONNECTION_ADD:
			if((fc_saInfo_idx < 0) || (fc_saInfo_idx >= RTK_FC_SPEC_XFRM_INFO_MAX_NUM))
			{
				WARNING("[Encrypt] fc_saInfo_idx(%u) is out of range 0~%u", fc_saInfo_idx, RTK_FC_SPEC_XFRM_INFO_MAX_NUM-1);
				return RT_PE_RET_FAIL;
			}
			if(crypto_req.connection_idx >= MAX_PE_IPSEC_ENCRYPTION_CONNECTION_NUM)
			{
				WARNING("[Encrypt] connection_idx(%u) is out of range 0~%u", crypto_req.connection_idx, MAX_PE_IPSEC_ENCRYPTION_CONNECTION_NUM-1);
				return RT_PE_RET_FAIL;
			}
			if((crypto_req.connection.encrypt_info.key_idx < 0) || (crypto_req.connection.encrypt_info.key_idx >= MAX_PE_IPSEC_HW_KEY_NUM))
			{
				WARNING("[Encrypt] key_idx(%u) is out of range 0~%u", crypto_req.connection.encrypt_info.key_idx, MAX_PE_IPSEC_HW_KEY_NUM-1);
				return RT_PE_RET_FAIL;
			}
			pppoe_sid = crypto_req.connection.encrypt_info.tag_info.external_used.pppoe_sid;
			svlan_vid = crypto_req.connection.encrypt_info.tag_info.external_used.svlan_vid;
			cvlan_vid = crypto_req.connection.encrypt_info.tag_info.external_used.cvlan_vid;
			pon_streamId = crypto_req.connection.encrypt_info.tag_info.external_used.pon_streamId;

			//reset to default
			crypto_req.connection.encrypt_info.tag_info.internal_used.dma_aft_idx = FAIL;
			crypto_req.connection.encrypt_info.tag_info.internal_used.streamId.sid = FAIL;
			crypto_req.connection.encrypt_info.tag_info.internal_used.streamId.ldpid = FAIL;
			if(pppoe_sid!=FAIL || svlan_vid!=FAIL || cvlan_vid!=FAIL)
			{
				uint32 fibIdx;
				rtk_rg_asic_dmaAftFib_t fib;

				memset(&fib, 0, sizeof(rtk_rg_asic_dmaAftFib_t));

				if(svlan_vid!=FAIL && cvlan_vid!=FAIL)
				{
					fib.vlan_vld = 1;
					//egress: stag + ctag
					fib.vlan_cnt = 2;
					fib.top_tpid_enc = 2; // TPID_0: 0x88a8
					fib.top_tpid_sel = 3; // fib
					fib.top_vid = svlan_vid;
					fib.top_802_1p = 0;
					fib.top_1p_sel = 3; // fib
					fib.top_dei = 0;
					fib.top_dei_sel = 3; //fib

					fib.inner_tpid_enc = 1; // TPID_0: 0x8100
					fib.inner_tpid_sel = 3; // fib
					fib.inner_vid = cvlan_vid;
					fib.inner_802_1p = 0;
					fib.inner_1p_sel = 3; // fib
					fib.inner_dei = 0;
					fib.inner_dei_sel = 3; //fib
				}
				else if(svlan_vid!=FAIL)
				{
					fib.vlan_vld = 1;
					//egress:  only stag
					fib.vlan_cnt = 1;
					fib.top_tpid_enc = 2; // TPID_0: 0x88a8
					fib.top_tpid_sel = 3; // fib
					fib.top_vid = svlan_vid;
					fib.top_802_1p = 0;
					fib.top_1p_sel = 3; // fib
					fib.top_dei = 0;
					fib.top_dei_sel = 3; //fib
				}
				else if(cvlan_vid!=FAIL)
				{
					fib.vlan_vld = 1;
					//egress:  ctag
					fib.vlan_cnt = 1;
					fib.top_tpid_enc = 1; // TPID_0: 0x8100
					fib.top_tpid_sel = 3; // fib
					fib.top_vid = cvlan_vid;
					fib.top_802_1p = 0;
					fib.top_1p_sel = 3; // fib
					fib.top_dei = 0;
					fib.top_dei_sel = 3; //fib
				}
				else
				{
					//egress: untag
				}

				if(pppoe_sid!=FAIL)
				{
					fib.pppoe_cmd = 1; // push
					fib.session_id = pppoe_sid;
				}

				if(_rtk_fc_dmaAftAction_add(&fibIdx, fib, TRUE) != RTK_FC_RET_OK)
				{
					WARNING("Fail to add dma AFT table!!");
					return RT_PE_RET_FAIL;
				}
				crypto_req.connection.encrypt_info.tag_info.internal_used.dma_aft_idx = fibIdx;
				DEBUG("[Crypto] dma_aft_idx=%d", crypto_req.connection.encrypt_info.tag_info.internal_used.dma_aft_idx);	
			}
			if(pon_streamId!=FAIL && pon_streamId<RTK_FC_TABLESIZE_STREAMID)
			{
				crypto_req.connection.encrypt_info.tag_info.internal_used.streamId.sid		= pon_streamId;
				crypto_req.connection.encrypt_info.tag_info.internal_used.streamId.ldpid	= fc_db.streamidTbl[pon_streamId].ldpid;
				crypto_req.connection.encrypt_info.tag_info.internal_used.streamId.cos		= fc_db.streamidTbl[pon_streamId].cos;
				crypto_req.connection.encrypt_info.tag_info.internal_used.streamId.flowid	= fc_db.streamidTbl[pon_streamId].gemid;		
			}
			break;
		case RT_PE_CRYPTO_ENGINE_CMD_ENCRYPT_CONNECTION_DEL:
			if(crypto_req.connection_idx >= MAX_PE_IPSEC_ENCRYPTION_CONNECTION_NUM)
			{
				WARNING("[Encrypt] connection_idx(%u) is out of range 0~%u", crypto_req.connection_idx, MAX_PE_IPSEC_ENCRYPTION_CONNECTION_NUM-1);
				return RT_PE_RET_FAIL;
			}
			break;
		case RT_PE_CRYPTO_ENGINE_CMD_DECRYPT_CONNECTION_ADD:
			if((fc_saInfo_idx < 0) || (fc_saInfo_idx >= RTK_FC_SPEC_XFRM_INFO_MAX_NUM))
			{
				WARNING("[Decrypt] fc_saInfo_idx(%u) is out of range 0~%u", fc_saInfo_idx, RTK_FC_SPEC_XFRM_INFO_MAX_NUM-1);
				return RT_PE_RET_FAIL;
			}
			if(crypto_req.connection_idx >= MAX_PE_IPSEC_DECRYPTION_CONNECTION_NUM)
			{
				WARNING("[Decrypt] connection_idx(%u) is out of range 0~%u", crypto_req.connection_idx, MAX_PE_IPSEC_DECRYPTION_CONNECTION_NUM-1);
				return RT_PE_RET_FAIL;
			}
			if((crypto_req.connection.decrypt_info.key_idx < 0) || (crypto_req.connection.decrypt_info.key_idx >= MAX_PE_IPSEC_HW_KEY_NUM))
			{
				WARNING("[Decrypt] key_idx(%u) is out of range 0~%u", crypto_req.connection.decrypt_info.key_idx, MAX_PE_IPSEC_HW_KEY_NUM-1);
				return RT_PE_RET_FAIL;
			}
			break;
		case RT_PE_CRYPTO_ENGINE_CMD_DECRYPT_CONNECTION_DEL:
			if(crypto_req.connection_idx >= MAX_PE_IPSEC_DECRYPTION_CONNECTION_NUM)
			{
				WARNING("[Decrypt] connection_idx(%u) is out of range 0~%u", crypto_req.connection_idx, MAX_PE_IPSEC_DECRYPTION_CONNECTION_NUM-1);
				return RT_PE_RET_FAIL;
			}
			break;	
		case RT_PE_CRYPTO_ENGINE_CMD_STATUS_GET:	
			break;	
		default:
			WARNING("req_cmd is undefined !!");
			return RT_PE_RET_FAIL;
	}

	//send ipc msg
	ipc_pkt.session_id = CA_IPC_SESSION_CRYPTO_ENGINE;
	ipc_pkt.dst_cpu_id = fc_db.controlFuc.func_on_pe_cpu_num[RT_PE_FUNC_NUM_CRYPTO] + CA_IPC_CPU_PE0;
	ipc_pkt.priority = 0;
	ipc_pkt.msg_no = crypto_req.req_cmd;
	ipc_pkt.msg_data = &crypto_req; 						/* the message data to transmit. */
	ipc_pkt.msg_size = sizeof(rt_pe_crypto_request_t);		/* the length of message data. */
	if(wait_pe_return)
	{
		ret = ca_ipc_msg_sync_send(&ipc_pkt, &ret_val, &result_size);
	}
	else
	{
		ret_val = RT_PE_RET_OK;
		ret = ca_ipc_msg_async_send(&ipc_pkt);
	}

	if (ret != CA_E_OK)
	{
		WARNING("Fail to call %s, ret %d\n", (wait_pe_return)?"ca_ipc_msg_sync_send()":"ca_ipc_msg_async_send()", ret);
		ret_fail = 1;
	}
	else
	{
		EVENT("Success to send ipc %s msg to PE, req_cmd=%d", (wait_pe_return)?"sync":"async", crypto_req.req_cmd);
		IPSEC_HOOK("Success to send ipc %s msg to PE, req_cmd=%d", (wait_pe_return)?"sync":"async", crypto_req.req_cmd);
		if(ret_val != RT_PE_RET_OK)
		{
			if(crypto_req.req_cmd!=RT_PE_CRYPTO_ENGINE_CMD_STATUS_GET)
				WARNING("PE return failed ret %d\n", ret_val);
			ret_fail = 1;
		}
	}
	
	if (ret_fail)
	{
		if(crypto_req.req_cmd == RT_PE_CRYPTO_ENGINE_CMD_ENABLE)
		{
			//free buffer
			if(fc_db.pe_crypto_pageInfo.pPages)
				_rtk_fc_page_free(&fc_db.pe_crypto_pageInfo);
			else
				WARNING("Fail to free page buffer of PE IPSEC ... due to pPages is NULL !!");
		}
		else if(crypto_req.req_cmd == RT_PE_CRYPTO_ENGINE_CMD_ENCRYPT_CONNECTION_ADD)
		{
			//del dma aft
			if(crypto_req.connection.encrypt_info.tag_info.internal_used.dma_aft_idx!=FAIL && crypto_req.connection.encrypt_info.tag_info.internal_used.dma_aft_idx<RTK_FC_TABLESIZE_DMAAFTACTION)
				_rtk_fc_dmaAftAction_del(crypto_req.connection.encrypt_info.tag_info.internal_used.dma_aft_idx);
		}
	}
	else
	{
		switch(crypto_req.req_cmd)
		{
			case RT_PE_CRYPTO_ENGINE_CMD_ENABLE:
				ipsec_accelerate_by_pe.igr_port_idx = RT_PE_IPSEC_DMA_LSO_FIRST_ENCRYPT_HWLOOKUP_LSPID;
				ipsec_accelerate_by_pe.action_redirect_ldpid = RT_PE_IPSEC_CPU_PORT;
				_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_IPSEC_ACCELERATE_BY_PE, &ipsec_accelerate_by_pe);	
				break;
			case RT_PE_CRYPTO_ENGINE_CMD_DISABLE:
				_rtk_rg_aclAndCfReservedRuleDel(RTK_FC_ACLANDCF_RESERVED_IPSEC_ACCELERATE_BY_PE);
				break;
			case RT_PE_CRYPTO_ENGINE_CMD_ENCRYPT_CONNECTION_ADD:
				TABLE("[PE] Add encrypt connection idx %d, fc_saInfo_idx %d", crypto_req.connection_idx, fc_saInfo_idx);
				fc_db.pe_encrypt_info[crypto_req.connection_idx].fc_saInfo_idx = fc_saInfo_idx;
				memcpy(&fc_db.pe_encrypt_info[crypto_req.connection_idx].pe_data, &crypto_req.connection.encrypt_info, sizeof(rt_pe_crypto_encrypt_info_t));
				break;
			case RT_PE_CRYPTO_ENGINE_CMD_ENCRYPT_CONNECTION_DEL:
				if(fc_db.pe_encrypt_info[crypto_req.connection_idx].fc_saInfo_idx != FAIL) //valid
				{
					TABLE("[PE] Del encrypt connection idx %d, fc_saInfo_idx %d", crypto_req.connection_idx, fc_db.pe_encrypt_info[crypto_req.connection_idx].fc_saInfo_idx);
					//del dma aft
					if(fc_db.pe_encrypt_info[crypto_req.connection_idx].pe_data.tag_info.internal_used.dma_aft_idx!=FAIL && fc_db.pe_encrypt_info[crypto_req.connection_idx].pe_data.tag_info.internal_used.dma_aft_idx<RTK_FC_TABLESIZE_DMAAFTACTION)
						_rtk_fc_dmaAftAction_del(fc_db.pe_encrypt_info[crypto_req.connection_idx].pe_data.tag_info.internal_used.dma_aft_idx);
					fc_db.pe_encrypt_info[crypto_req.connection_idx].fc_saInfo_idx = FAIL;
				}
				break;
			case RT_PE_CRYPTO_ENGINE_CMD_DECRYPT_CONNECTION_ADD:
				TABLE("[PE] Add decrypt connection idx %d, fc_saInfo_idx %d", crypto_req.connection_idx, fc_saInfo_idx);
				fc_db.pe_decrypt_info[crypto_req.connection_idx].fc_saInfo_idx = fc_saInfo_idx;
				memcpy(&fc_db.pe_decrypt_info[crypto_req.connection_idx].pe_data, &crypto_req.connection.decrypt_info, sizeof(rt_pe_crypto_decrypt_info_t));
				break;
			case RT_PE_CRYPTO_ENGINE_CMD_DECRYPT_CONNECTION_DEL:
				if(fc_db.pe_decrypt_info[crypto_req.connection_idx].fc_saInfo_idx != FAIL) //valid
				{	
					TABLE("[PE] Del decrypt connection idx %d, fc_saInfo_idx %d", crypto_req.connection_idx, fc_db.pe_decrypt_info[crypto_req.connection_idx].fc_saInfo_idx);
					fc_db.pe_decrypt_info[crypto_req.connection_idx].fc_saInfo_idx = FAIL;
				}
				break;	
			default:
				break;
		}	
	}

	return ret_val;
}
#endif

int rtk_fc_stat_gemFilter_set(uint32 index, rt_stat_gemFilter_conf_t conf)
{	
	int i =0;
	FC_PARAM_CHK((index>=RT_STAT_GEMMIB_TABLE_SIZE), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	memcpy(&fc_db.gemFilter_conf[index], &conf, sizeof(rt_stat_gemFilter_conf_t));

	// refresh gemFilter_conf_en state
	fc_db.controlFuc.gemFilter_conf_en = 0;
	for(i = 0; i < RT_STAT_GEMMIB_TABLE_SIZE; i++) {
		if(fc_db.gemFilter_conf[i].enable == TRUE) {
			fc_db.controlFuc.gemFilter_conf_en = 1;
			break;
		}
	}
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();

	return SUCCESS;
}
int rtk_fc_stat_gemFilter_get(uint32 index, rt_stat_gemFilter_conf_t* pConf)
{
	FC_PARAM_CHK((index>=RT_STAT_GEMMIB_TABLE_SIZE), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	memcpy(pConf, &fc_db.gemFilter_conf[index], sizeof(rt_stat_gemFilter_conf_t));
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	
	return SUCCESS;
}
int rtk_fc_stat_gemFilterMib_reset(uint32 index)
{
	FC_PARAM_CHK((index>=RT_STAT_GEMMIB_TABLE_SIZE), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);

	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
	bzero(&fc_db.gemFilter_mib[index][0], sizeof(rt_stat_port_cntr_t)*NR_CPUS);
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	
	return SUCCESS;
}
int rtk_fc_stat_gemFilterMib_get(uint32 index, rt_stat_port_cntr_t* pCntr)
{
	rt_stat_port_cntr_t tmpCntr={0};
	uint32 *pData_cpusum = NULL, *pData = NULL, i = 0, cpu = 0;
	FC_PARAM_CHK((index>=RT_STAT_GEMMIB_TABLE_SIZE), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();

	bzero(&tmpCntr, sizeof(tmpCntr));
	pData_cpusum = (uint32 *)&tmpCntr;
			
	for(i = 0; i < sizeof(rt_stat_port_cntr_t)/sizeof(uint32); i ++) {
		for(cpu = 0; cpu < NR_CPUS; cpu++) {
			pData = (uint32 *)&fc_db.gemFilter_mib[index][cpu];

			pData_cpusum[i] += pData[i];
		}
	}
	
	memcpy(pCntr, &tmpCntr, sizeof(rt_stat_port_cntr_t));
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
	
	return SUCCESS;
}

#if defined(CONFIG_RTK_SOC_RTL8198D)
int rtk_fc_hwFlowMib_get(int idx, rt_stat_flow_mib_t *pflowMibCnt)
{
	rtk_fc_flowOrHostmib_counter_t hwCounter;
	API("get flowMib[%d]", idx);

	FC_PARAM_CHK((idx<0)||(idx >= RT_STAT_FLOWMIB_TABLE_SIZE), RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	FC_PARAM_CHK((pflowMibCnt == NULL), RTK_FC_RET_ERR_NULL_POINTER);


	memset(pflowMibCnt, 0, sizeof(rt_stat_flow_mib_t));

	/*get HW counter*/
	_rtk_fc_hwFlowMib_get(idx, &hwCounter);

	/*return HW counter*/
	pflowMibCnt->ingress_packet_count = hwCounter.in_packet_cnt;
	pflowMibCnt->ingress_byte_count = hwCounter.in_byte_cnt;
	pflowMibCnt->egress_packet_count = hwCounter.out_packet_cnt;
	pflowMibCnt->egress_byte_count = hwCounter.out_byte_cnt;

	return SUCCESS;
}

int rtk_fc_brNetifInfo_get(char *br_name, rtk_mac_t *br_mac, uint32 *br_ip)
{
	int i;

	if (unlikely(!br_name || !br_mac || !br_ip))
		return RTK_FC_RET_ERR_NULL_POINTER;

	for (i = DEVIFIDX_VALID_MIN; i < RTK_FC_DEV_GW_MAC_TBL; i++) 
	{
		if (fc_db.devGwMacTbl[i].dev && !memcmp(fc_db.devGwMacTbl[i].dev->name, br_name, strlen(br_name)))
		{

			memcpy(br_mac, fc_db.devGwMacTbl[i].dev->dev_addr , ETH_ALEN);
			RTK_FC_HELPER_RCU_INDEV_GET_IPADDR(fc_db.devGwMacTbl[i].dev,br_ip);
			return SUCCESS;
		}
	}

	return RTK_FC_RET_ERR;
}
#endif

int rtk_fc_rt_mapper_api_init(void)
{
	int ret = FAILED;
	ret = RTK_FC_HELPER_RT_EXT_MAPPER_REGISTER(&pf_mapper);
	

	fc_db.controlFuc.rt_api_is_enabled = RTK_FC_HELPER_MGR_CHECK_CONFIG_SETTING(FC_MGR_GET_CONFIG_RT_API);
	return SUCCESS;
}

#endif

