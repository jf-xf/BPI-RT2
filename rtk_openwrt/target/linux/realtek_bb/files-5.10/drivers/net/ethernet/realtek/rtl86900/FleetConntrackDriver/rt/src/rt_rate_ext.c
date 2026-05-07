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
 * Purpose : Definition of RT API of Rate Extension
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of host policer
 *
 */




/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/rt/rt_port.h>
#include <rtk/init.h>
#include <rtk/default.h>
#include <dal/dal_mgmt.h>
#include <common/util/rt_util.h>
#include <hal/common/halctrl.h>

#include <rt_ext_mapper.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      rt_rate_hostPolicerControl_get
 * Description:
 *      Get Host Policer Control Configuration
 * Input:
 *      index				- Host Policer index
 * Output:
 *      pPolicerControl	- Host Policer Control Configuration
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can get Host Policer Control Configuration
 */
int32
rt_rate_hostPolicerControl_get (
    uint32 index,
    rt_rate_host_policer_control_t* pPolicerControl )
{
    int32   ret=RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->rate_hostPolicerControl_get == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->rate_hostPolicerControl_get(index, pPolicerControl))!=0)
        return RT_ERR_NOT_ALLOWED;
    return ret;
}   /* end of rt_rate_hostPolicerControl_get */

/* Function Name:
 *      rt_rate_hostPolicerControl_set
 * Description:
 *      Set Host Policer Control Configuration
 * Input:
 *      index			- Host Policer index
 *      policerControl	- Host Policer Control Configuration
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can set Host Policer Control Configuration.
 *      Supported RT meter type for host rate limiting: RT_METER_TYPE_HOST (HW policing), RT_METER_TYPE_SW (SW shaping)
 */
int32
rt_rate_hostPolicerControl_set (
    uint32 index,
    rt_rate_host_policer_control_t policerControl )
{
    int32   ret=RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->rate_hostPolicerControl_set == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->rate_hostPolicerControl_set(index, policerControl))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;
}   /* end of rt_rate_hostPolicerControl_set */

/* Function Name:
 *      rt_rate_hostPolicerMib_get
 * Description:
 *      Get Host Policer MIB
 * Input:
 *      index			- Host Policer index
 * Output:
 *      pPolicerMib	- Host Policer MIB
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can get Host Policer MIB
 */
int32
rt_rate_hostPolicerMib_get (
    uint32 index,
    rt_rate_host_policer_mib_t* pPolicerMib )
{
    int32   ret=RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->rate_hostPolicerMib_get == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;
	
    if((p_rt_ext->rate_hostPolicerMib_get(index, pPolicerMib))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;
}   /* end of rt_rate_hostPolicerMib_get */

/* Function Name:
 *      rt_rate_hostPolicerMib_clear
 * Description:
 *      Clear Host Policer MIB by Host Policer index
 * Input:
 *      index	- Host Policer index
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can clear Host Policer MIB
 */
int32
rt_rate_hostPolicerMib_clear (
    uint32 index )
{
    int32   ret=RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->rate_hostPolicerMib_clear == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;
    
    if((p_rt_ext->rate_hostPolicerMib_clear(index))!=0)
        return RT_ERR_NOT_ALLOWED;
    
    return ret;
}   /* end of rt_rate_hostPolicerMib_clear */

/* Function Name:
 *      rt_rate_sw_rate_limit_get
 * Description:
 *      Get configuration of software rate limit of the type
 * Input:
 *      type        			- Type of software rate limit
 * Output:
 *      pSwRateLimit_conf 	- Configuration of software rate limit of the type
 * Return:
 *      RT_ERR_OK			    - OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can get configuration of software rate limit of the type
 */
int32
rt_rate_sw_rate_limit_get (
    rt_rate_sw_rate_limit_type_t type,
    rt_rate_sw_rate_limit_conf_t* pSwRateLimit_conf )
{
	int32 ret = RT_ERR_OK;
	rt_rate_meter_mapping_t meterMapping;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->rate_sw_rate_limit_get == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;
	
    if((p_rt_ext->rate_sw_rate_limit_get(type, pSwRateLimit_conf))!=0)
        return RT_ERR_NOT_ALLOWED;

	if(pSwRateLimit_conf->shareMeterIdx>=0)
	{
		meterMapping.type = RT_METER_TYPE_SW;
		meterMapping.hw_index = pSwRateLimit_conf->shareMeterIdx;
		ret = rt_rate_shareMeterMappingVirtual_get(meterMapping, &pSwRateLimit_conf->shareMeterIdx);
		if(ret != RT_ERR_OK)
		{
			printk("Get virtual meter index Failed, return value: 0x%x, meter type: %d, l3Meter hw index: %d", ret, meterMapping.type, meterMapping.hw_index);
			return RT_ERR_NOT_ALLOWED;
		}
	}
	
    return ret;
}   /* end of rt_rate_sw_rate_limit_get */

/* Function Name:
 *      rt_rate_sw_rate_limit_set
 * Description:
 *      Set configuration of software rate limit of the type
 * Input:
 *      type        			- Type of software rate limit
 *      swRateLimit_conf 	- Configuration of software rate limit of the type
 * Output:
 *      
 * Return:
 *      RT_ERR_OK			    - OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can set configuration of software rate limit of the type
 */
int32
rt_rate_sw_rate_limit_set (
    rt_rate_sw_rate_limit_type_t type,
    rt_rate_sw_rate_limit_conf_t swRateLimit_conf )
{
	int32 ret = RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->rate_sw_rate_limit_set == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->rate_sw_rate_limit_set(type, swRateLimit_conf))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;
}   /* end of rt_rate_sw_rate_limit_set */


EXPORT_SYMBOL(rt_rate_hostPolicerControl_get);
EXPORT_SYMBOL(rt_rate_hostPolicerControl_set);
EXPORT_SYMBOL(rt_rate_hostPolicerMib_get);
EXPORT_SYMBOL(rt_rate_hostPolicerMib_clear);
EXPORT_SYMBOL(rt_rate_sw_rate_limit_get);
EXPORT_SYMBOL(rt_rate_sw_rate_limit_set);



