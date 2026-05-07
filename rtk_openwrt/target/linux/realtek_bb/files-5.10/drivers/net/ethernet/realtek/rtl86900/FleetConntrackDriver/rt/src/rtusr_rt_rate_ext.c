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
 * Purpose : Definition of Rate Extension API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of host policer
 *
 */


/*
 * Include Files
 */
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <osal/lib.h>
#include <rtk/rtusr/include/rtusr_util.h>

#include <rtdrv_ext_netfilter.h>

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
 *      Get Host Policer Control
 * Input:
 *      index        - Host Policer index
 * Output:
 *      pPolicerControl - Host Policer Control
 * Return:
 *      RT_ERR_OK               - OK
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
    rtdrv_rt_rateCfg_ext_t rt_rate_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pPolicerControl), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_rate_cfg.index, &index, sizeof(uint32));
    GETSOCKOPT(RTDRV_RT_RATE_HOSTPOLICERCONTROL_GET, &rt_rate_cfg, rtdrv_rt_rateCfg_ext_t, 1);
    osal_memcpy(pPolicerControl, &rt_rate_cfg.policerControl, sizeof(rt_rate_host_policer_control_t));

    return RT_ERR_OK;
}   /* end of rt_rate_hostPolicerControl_get */

/* Function Name:
 *      rt_rate_hostPolicerControl_set
 * Description:
 *      Set Host Policer Control
 * Input:
 *      index        - Host Policer index
 *      policerControl - Host Policer Control
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
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
    rtdrv_rt_rateCfg_ext_t rt_rate_cfg={0};

    /* function body */
    osal_memcpy(&rt_rate_cfg.index, &index, sizeof(uint32));
    osal_memcpy(&rt_rate_cfg.policerControl, &policerControl, sizeof(rt_rate_host_policer_control_t));
    SETSOCKOPT(RTDRV_RT_RATE_HOSTPOLICERCONTROL_SET, &rt_rate_cfg, rtdrv_rt_rateCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_rate_hostPolicerControl_set */

/* Function Name:
 *      rt_rate_hostPolicerMib_get
 * Description:
 *      Get Host Policer MIB
 * Input:
 *      index        - Host Policer index
 * Output:
 *      pPolicerMib - Host Policer MIB
 * Return:
 *      RT_ERR_OK               - OK
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
    rtdrv_rt_rateCfg_ext_t rt_rate_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pPolicerMib), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_rate_cfg.index, &index, sizeof(uint32));
    GETSOCKOPT(RTDRV_RT_RATE_HOSTPOLICERMIB_GET, &rt_rate_cfg, rtdrv_rt_rateCfg_ext_t, 1);
    osal_memcpy(pPolicerMib, &rt_rate_cfg.policerMib, sizeof(rt_rate_host_policer_mib_t));

    return RT_ERR_OK;
}   /* end of rt_rate_hostPolicerMib_get */

/* Function Name:
 *      rt_rate_hostPolicerMib_clear
 * Description:
 *      Clear Host Policer MIB by Host Policer index
 * Input:
 *      index        - Host Policer index
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can clear Host Policer MIB
 */
int32
rt_rate_hostPolicerMib_clear (
    uint32 index )
{
    rtdrv_rt_rateCfg_ext_t rt_rate_cfg={0};

    /* function body */
    osal_memcpy(&rt_rate_cfg.index, &index, sizeof(uint32));
    SETSOCKOPT(RTDRV_RT_RATE_HOSTPOLICERMIB_CLEAR, &rt_rate_cfg, rtdrv_rt_rateCfg_ext_t, 1);

    return RT_ERR_OK;
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
 *      RT_ERR_OK				- OK
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
	rtdrv_rt_rateCfg_ext_t rt_rate_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pSwRateLimit_conf), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_rate_cfg.type, &type, sizeof(rt_rate_sw_rate_limit_type_t));
    GETSOCKOPT(RTDRV_RT_RATE_SW_RATE_LIMIT_GET, &rt_rate_cfg, rtdrv_rt_rateCfg_ext_t, 1);
    osal_memcpy(pSwRateLimit_conf, &rt_rate_cfg.swRateLimit_conf, sizeof(rt_rate_sw_rate_limit_conf_t));

    return RT_ERR_OK;
}   /* end of rt_rate_sw_rate_limit_get */

/* Function Name:
 *      rt_rate_sw_rate_limit_set
 * Description:
 *      Set configuration of software rate limit of the type
 * Input:
 *      type        			- Type of software rate limit
 *      swRateLimit_conf 	- Configuration of software rate limit of the type
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK				- OK
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
	rtdrv_rt_rateCfg_ext_t rt_rate_cfg={0};

    /* function body */
    osal_memcpy(&rt_rate_cfg.type, &type, sizeof(rt_rate_sw_rate_limit_type_t));
    osal_memcpy(&rt_rate_cfg.swRateLimit_conf, &swRateLimit_conf, sizeof(rt_rate_sw_rate_limit_conf_t));
    SETSOCKOPT(RTDRV_RT_RATE_SW_RATE_LIMIT_SET, &rt_rate_cfg, rtdrv_rt_rateCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_rate_sw_rate_limit_set */