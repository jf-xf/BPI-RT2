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
#define COMPILE_RTK_L34_FC_MGR_MODULE 1

#include <rtk_fc_helper_wlan.h>
#include <rtk_fc_helper.h>
#include <rtk_fc_mgr.h>


int rtk_fc_rt_ext_mapper_register(rt_ext_mapper_t *pMapper)
{
#if defined(CONFIG_COMMON_RT_API)
	// diag shell mapper for RT API
	rt_ext_mapper_register(pMapper);
	return SUCCESS;
#else
	return FAIL;
#endif

}
int rtk_fc_rt_rate_shareMeter_mapping_hw_get(uint32 v_index,            rt_rate_meter_mapping_t* pMeterMap)
{

#if defined(CONFIG_COMMON_RT_API)
	int ret;
	// diag shell mapper for RT API
	ret = rt_rate_shareMeterMappingHw_get(v_index, pMeterMap);
	return ret;
#else
	return FAIL;
#endif
}

int rtk_fc_rt_cls_fwdPort_set(rt_port_t port, rt_port_t fwdPort)
{
#if defined(CONFIG_COMMON_RT_API) && (defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))
	int ret;
	// diag shell mapper for RT API
	ret = rt_cls_fwdPort_set(port, fwdPort);
	return ret;
#else
	return FAIL;
#endif
}

int rtk_fc_rt_switch_mode_get(rt_switch_mode_t *mode)
{
#if defined(CONFIG_COMMON_RT_API) && (defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))
	int ret;
	// diag shell mapper for RT API
	ret = rt_switch_mode_get(mode);
	return ret;
#else
	return FAIL;
#endif
}

