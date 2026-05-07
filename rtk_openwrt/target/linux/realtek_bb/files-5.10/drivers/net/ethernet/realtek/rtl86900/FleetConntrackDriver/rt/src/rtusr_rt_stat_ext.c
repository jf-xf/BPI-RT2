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
 * $Revision: 68395 $
 * $Date: 2016-05-27 16:38:35 +0800 (Fri, 27 May 2016) $
 *
 * Purpose : Definition of Statistic extension API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Statistic Counter of Flow Reset
 *           (2) Statistic Counter of Flow Get
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
#include <common/rt_type.h>

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
 *      rt_stat_flowCache_mib_get
 * Description:
 *      Get flow counters by flow index.
 * Input:
 *      flow_idx			- Flow index
 * Output:
 *      pFlowCacheMib	- Flow cache MIB
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get flow cache counters
 */
int32
rt_stat_flowCache_mib_get (
    uint32 flow_idx,
    rt_stat_flowCache_mib_t* pFlowCacheMib )
{
	rtdrv_rt_statCfg_ext_t rt_stat_cfg;

	/* parameter check */
	RT_PARAM_CHK((NULL == pFlowCacheMib), RT_ERR_NULL_POINTER);

	/* function body */
	osal_memcpy(&rt_stat_cfg.index, &flow_idx, sizeof(uint32));
	GETSOCKOPT(RTDRV_RT_STAT_FLOW_CACHE_MIB_GET, &rt_stat_cfg, rtdrv_rt_statCfg_ext_t, 1);
	osal_memcpy(pFlowCacheMib, &rt_stat_cfg.flowCacheMib, sizeof(rt_stat_flowCache_mib_t));

	return RT_ERR_OK;
}

/* Function Name:
 *      rt_stat_flowCache_mib_reset
 * Description:
 *      Reset flow counters by flow index.
 * Input:
 *      flow_idx	- Flow index
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can reset flow counters
 */
int32
rt_stat_flowCache_mib_reset (
    uint32 flow_idx )
{
	rtdrv_rt_statCfg_ext_t rt_stat_cfg={0};

	/* function body */
	osal_memcpy(&rt_stat_cfg.index, &flow_idx, sizeof(uint32));
	SETSOCKOPT(RTDRV_RT_STAT_FLOW_CACHE_MIB_RESET, &rt_stat_cfg, rtdrv_rt_statCfg_ext_t, 1);

	return RT_ERR_OK;
}


/* Function Name:
 *      rt_stat_flowMib_get
 * Description:
 *      Get flow counters by flow mib index.
 * Input:
 *      index        - Flow mib index
 * Output:
 *      pFlowMib - Flow MIB
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can get flow counters
 */
int32
rt_stat_flowMib_get (
    uint32 index,
    rt_stat_flow_mib_t* pFlowMib )
{
    rtdrv_rt_statCfg_ext_t rt_stat_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pFlowMib), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_stat_cfg.index, &index, sizeof(uint32));
    GETSOCKOPT(RTDRV_RT_STAT_FLOWMIB_GET, &rt_stat_cfg, rtdrv_rt_statCfg_ext_t, 1);
    osal_memcpy(pFlowMib, &rt_stat_cfg.flowMib, sizeof(rt_stat_flow_mib_t));

    return RT_ERR_OK;
}   /* end of rt_stat_flowMib_get */

/* Function Name:
 *      rt_stat_flowMib_reset
 * Description:
 *      Reset flow counters by flow mib index.
 * Input:
 *      index        - Flow mib index
 * Output:
 *      
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can reset flow counters
 */
int32
rt_stat_flowMib_reset (
    uint32 index )
{
    rtdrv_rt_statCfg_ext_t rt_stat_cfg={0};

    /* function body */
    osal_memcpy(&rt_stat_cfg.index, &index, sizeof(uint32));
    SETSOCKOPT(RTDRV_RT_STAT_FLOWMIB_RESET, &rt_stat_cfg, rtdrv_rt_statCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_stat_flowMib_reset */

/* Function Name:
 *		rt_stat_netifMib_get
 * Description:
 *		Get netif counters by device name.
 * Input:
 *		dev_name		- Device name
 * Output:
 *		pNetifMib - Netif MIB
 * Return:
 *		RT_ERR_OK				- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *		The API can get netif counters
 */
int32
rt_stat_netifMib_get (
	char* dev_name,
	rt_stat_netif_mib_t* pNetifMib )
{
    rtdrv_rt_statCfg_ext_t rt_stat_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pNetifMib), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(rt_stat_cfg.dev_name, dev_name, sizeof(rt_stat_cfg.dev_name));
    GETSOCKOPT(RTDRV_RT_STAT_NETIFMIB_GET, &rt_stat_cfg, rtdrv_rt_statCfg_ext_t, 1);
    osal_memcpy(pNetifMib, &rt_stat_cfg.netifMib, sizeof(rt_stat_netif_mib_t));

    return RT_ERR_OK;
}	/* end of rt_stat_netifMib_get */

/* Function Name:
 *		rt_stat_netifMib_reset
 * Description:
 *		Reset netif counters by device name.
 * Input:
 *		dev_name		- Device name
 * Output:
 *		None.
 * Return:
 *		RT_ERR_OK				- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *		The API can reset netif counters
 */
int32
rt_stat_netifMib_reset (
	char* dev_name )
{
    rtdrv_rt_statCfg_ext_t rt_stat_cfg={0};

    /* parameter check */
    RT_PARAM_CHK((NULL == dev_name), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(rt_stat_cfg.dev_name, dev_name, sizeof(rt_stat_cfg.dev_name));
    SETSOCKOPT(RTDRV_RT_STAT_NETIFMIB_RESET, &rt_stat_cfg, rtdrv_rt_statCfg_ext_t, 1);

    return RT_ERR_OK;
}	/* end of rt_stat_netifMib_reset */


/* Function Name:
 *      rt_stat_gemFilter_set
 * Description:
 *      Set gem filter rule.
 * Input:
 *      index		- rule index, 0-31
 *      conf		- enabled state, type, vlan filter
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can configure rule details.
 */
int32 
rt_stat_gemFilter_set (
	uint32 index, 
	rt_stat_gemFilter_conf_t conf )
{
    rtdrv_rt_statCfg_ext_t rt_stat_cfg={0};

    /* function body */
    osal_memcpy(&rt_stat_cfg.index, &index, sizeof(uint32));
    osal_memcpy(&rt_stat_cfg.conf, &conf, sizeof(rt_stat_gemFilter_conf_t));
    SETSOCKOPT(RTDRV_RT_STAT_GEMFILTER_SET, &rt_stat_cfg, rtdrv_rt_statCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_stat_gemFilter_set */

/* Function Name:
 *      rt_stat_gemFilter_get
 * Description:
 *      Get gem filter rule.
 * Input:
 *      index		- rule index, 0-31
 * Output:
 *      pConf		- enabled state, type, vlan filter
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can get rule details.
 */
int32 
rt_stat_gemFilter_get (
	uint32 index, 
	rt_stat_gemFilter_conf_t* pConf )
{
    rtdrv_rt_statCfg_ext_t rt_stat_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pConf), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_stat_cfg.index, &index, sizeof(uint32));
    GETSOCKOPT(RTDRV_RT_STAT_GEMFILTER_GET, &rt_stat_cfg, rtdrv_rt_statCfg_ext_t, 1);
    osal_memcpy(pConf, &rt_stat_cfg.conf, sizeof(rt_stat_gemFilter_conf_t));

    return RT_ERR_OK;
}   /* end of rt_stat_gemFilter_get */

/* Function Name:
 *      rt_stat_gemFilterMib_reset
 * Description:
 *      Set gem filter rule.
 * Input:
 *      index		- rule index, 0-31
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can rese gem mib.
 */
int32 
rt_stat_gemFilterMib_reset (
	uint32 index )
{
    rtdrv_rt_statCfg_ext_t rt_stat_cfg={0};

    /* function body */
    osal_memcpy(&rt_stat_cfg.index, &index, sizeof(uint32));
    SETSOCKOPT(RTDRV_RT_STAT_GEMFILTERMIB_RESET, &rt_stat_cfg, rtdrv_rt_statCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_stat_gemFilterMib_reset */

/* Function Name:
 *      rt_stat_gemFilterMib_get
 * Description:
 *      Get gem filter mib counter.
 * Input:
 *      index		- rule index, 0-31
 * Output:
 *      pCntr		- mib
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can get gem mib.
 */
int32 
rt_stat_gemFilterMib_get (
	uint32 index,
	rt_stat_port_cntr_t* pCntr)
{
    rtdrv_rt_statCfg_ext_t rt_stat_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pCntr), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_stat_cfg.index, &index, sizeof(uint32));
    GETSOCKOPT(RTDRV_RT_STAT_GEMFILTERMIB_GET, &rt_stat_cfg, rtdrv_rt_statCfg_ext_t, 1);
    osal_memcpy(pCntr, &rt_stat_cfg.cntr, sizeof(rt_stat_port_cntr_t));

    return RT_ERR_OK;
}   /* end of rt_stat_gemFilterMib_get */


/* Function Name:
 *		rt_stat_aclMib_get
 * Description:
 *		Get acl counters by acl mib index.
 * Input:
 *		index		 - ACL mib index
 * Output:
 *		pAclMib 	- ACL MIB
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *		RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *		The API can get acl counters
 */
int32
rt_stat_aclMib_get (
	uint32 index,
	rt_stat_acl_mib_t* pAclMib )
{
	rtdrv_rt_statCfg_ext_t rt_stat_cfg;

	/* parameter check */
	RT_PARAM_CHK((NULL == pAclMib), RT_ERR_NULL_POINTER);

	/* function body */
	osal_memcpy(&rt_stat_cfg.index, &index, sizeof(uint32));
	GETSOCKOPT(RTDRV_RT_STAT_ACLMIB_GET, &rt_stat_cfg, rtdrv_rt_statCfg_ext_t, 1);
	osal_memcpy(pAclMib, &rt_stat_cfg.aclMib, sizeof(rt_stat_acl_mib_t));

	return RT_ERR_OK;
}	/* end of rt_stat_aclMib_get */

/* Function Name:
 *		rt_stat_aclMib_set
 * Description:
 *		Set acl counters type by acl mib index.
 * Input:
 *		index		 - ACL mib index
 * Output:
 *		pAclMib 	- ACL MIB
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *		RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *		The API can set acl counters type
 */
int32
rt_stat_aclMib_set (
	uint32 index,
	rt_stat_acl_mib_t pAclMib )
{
	rtdrv_rt_statCfg_ext_t rt_stat_cfg={0};

	/* function body */
	osal_memcpy(&rt_stat_cfg.index, &index, sizeof(uint32));
    osal_memcpy(&rt_stat_cfg.aclMib, &pAclMib, sizeof(rt_stat_acl_mib_t));
	SETSOCKOPT(RTDRV_RT_STAT_ACLMIB_SET, &rt_stat_cfg, rtdrv_rt_statCfg_ext_t, 1);

	return RT_ERR_OK;
}	/* end of rt_stat_aclMib_set */

/* Function Name:
 *		rt_stat_aclMib_reset
 * Description:
 *		Reset acl counters by acl mib index.
 * Input:
 *		index		 - ACL mib index
 * Output:
 *		
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *		RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *		The API can reset acl counters
 */
int32
rt_stat_aclMib_reset (
	uint32 index )
{
	rtdrv_rt_statCfg_ext_t rt_stat_cfg={0};

	/* function body */
	osal_memcpy(&rt_stat_cfg.index, &index, sizeof(uint32));
	SETSOCKOPT(RTDRV_RT_STAT_ACLMIB_RESET, &rt_stat_cfg, rtdrv_rt_statCfg_ext_t, 1);

	return RT_ERR_OK;
}	/* end of rt_stat_aclMib_reset */

