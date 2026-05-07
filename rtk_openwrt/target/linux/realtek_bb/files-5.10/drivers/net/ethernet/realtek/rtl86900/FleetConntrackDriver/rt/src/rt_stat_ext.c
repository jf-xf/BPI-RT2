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
    int32   ret=RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->stat_flowCache_mib_get == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->stat_flowCache_mib_get(flow_idx, pFlowCacheMib))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;
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
    int32   ret=RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->stat_flowCache_mib_reset == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->stat_flowCache_mib_reset(flow_idx))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;
}

/*
 * Function Declaration
 */

/* Function Name:
 *      rt_stat_flowMib_get
 * Description:
 *      Get flow counters by flow mib index.
 * Input:
 *      index		- Flow mib index
 * Output:
 *      pFlowMib	- Flow MIB
 * Return:
 *      RT_ERR_OK					- OK
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
    int32   ret=RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->stat_flowMib_get == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->stat_flowMib_get(index, pFlowMib))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;
}   /* end of rt_stat_flowMib_get */

/* Function Name:
 *      rt_stat_flowMib_reset
 * Description:
 *      Reset flow counters by flow mib index.
 * Input:
 *      index	- Flow mib index
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can reset flow counters
 */
int32
rt_stat_flowMib_reset (
    uint32 index )
{
    int32   ret=RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->stat_flowMib_reset == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->stat_flowMib_reset(index))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;
}   /* end of rt_stat_flowMib_reset */

/* Function Name:
 *      rt_stat_netifMib_get
 * Description:
 *      Get netif counters by device name.
 * Input:
 *      dev_name		- Device name
 * Output:
 *      pNetifMib - Netif MIB
 * Return:
 *      RT_ERR_OK					- OK
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
    int32	ret=RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();
	
    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;
	
    if(p_rt_ext->stat_netifMib_get == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;
	
    if((p_rt_ext->stat_netifMib_get(dev_name, pNetifMib))!=0)
        return RT_ERR_NOT_ALLOWED;

	return ret;
}	/* end of rt_stat_netifMib_get */

/* Function Name:
 *      rt_stat_netifMib_reset
 * Description:
 *      Reset netif counters by device name.
 * Input:
 *      dev_name		- Device name
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *		The API can reset netif counters
 */
int32
rt_stat_netifMib_reset (
	char* dev_name )
{
    int32	ret=RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();
	
    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;
	
    if(p_rt_ext->stat_netifMib_reset == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;
	
    if((p_rt_ext->stat_netifMib_reset(dev_name))!=0)
        return RT_ERR_NOT_ALLOWED;

	return ret;
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
    int32	ret=RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();
	
    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;
	
    if(p_rt_ext->stat_gemFilter_set== NULL)
        return RT_ERR_DRIVER_NOT_FOUND;
	
    if((p_rt_ext->stat_gemFilter_set(index, conf))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;
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
    int32	ret=RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();
	
    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;
	
    if(p_rt_ext->stat_gemFilter_get == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;
	
    if((p_rt_ext->stat_gemFilter_get(index, pConf))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;
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
    int32	ret=RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();
	
    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;
	
    if(p_rt_ext->stat_gemFilterMib_reset == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;
	
    if((p_rt_ext->stat_gemFilterMib_reset(index))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;
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
    int32	ret=RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();
	
    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;
	
    if(p_rt_ext->stat_gemFilterMib_get == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;
	
    if((p_rt_ext->stat_gemFilterMib_get(index, pCntr))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;
}   /* end of rt_stat_gemFilterMib_get */


/* Function Name:
 *		rt_stat_aclMib_get
 * Description:
 *		Get acl counters by acl mib index.
 * Input:
 *		index		- ACL mib index
 * Output:
 *		pAclMib		- ACL MIB
 * Return:
 *		RT_ERR_OK					- OK
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
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->stat_aclMib_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->stat_aclMib_get(index, pAclMib))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}	/* end of rt_stat_aclMib_get */

/* Function Name:
 *		rt_stat_aclMib_set
 * Description:
 *		Set acl counters type by acl mib index.
 * Input:
 *		index		- ACL mib index
 * Output:
 *		pAclMib 	- ACL MIB
 * Return:
 *		RT_ERR_OK					- OK
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
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->stat_aclMib_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->stat_aclMib_set(index, pAclMib))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}	/* end of rt_stat_aclMib_set */

/* Function Name:
 *		rt_stat_aclMib_reset
 * Description:
 *		Reset acl counters by acl mib index.
 * Input:
 *		index	- ACL mib index
 * Output:
 *		None.
 * Return:
 *		RT_ERR_OK					- OK
 *		RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *		RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *		The API can reset acl counters
 */
int32
rt_stat_aclMib_reset (
	uint32 index )
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->stat_aclMib_reset == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->stat_aclMib_reset(index))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}	/* end of rt_stat_aclMib_reset */


#if defined(CONFIG_RTK_SOC_RTL8198D)
/* Function Name:
 *      rt_stat_hwFlowMib_get
 * Description:
 *      Get flow counters by hw flow mib index.
 * Input:
 *      index		- Flow mib index
 * Output:
 *      pFlowMib	- Flow MIB
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can get flow counters
 */
int32
rt_stat_hwFlowMib_get (
	uint32 index,
	rt_stat_flow_mib_t* pFlowMib )
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->stat_hwFlowMib_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->stat_hwFlowMib_get(index, pFlowMib))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}	/* end of rt_stat_hwFlowMib_get */

/* Function Name:
 *		rt_stat_brNetifInfo_get
 * Description:
 *		get bridge netif info.
 * Input:
 *		br_name		- bridge netif name.
 * Output:
 *		br_mac		- bridge netif mac address.
 *		br_ip		- bridge netif ip address.
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *		The API can get bridge netif info.
 */
int32
rt_stat_brNetifInfo_get(char *br_name, rtk_mac_t *br_mac, uint32 *br_ip)
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->stat_brNetifInfo_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->stat_brNetifInfo_get(br_name, br_mac, br_ip))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}

EXPORT_SYMBOL(rt_stat_hwFlowMib_get);
EXPORT_SYMBOL(rt_stat_brNetifInfo_get);
#endif

EXPORT_SYMBOL(rt_stat_flowMib_get);
EXPORT_SYMBOL(rt_stat_flowMib_reset);
EXPORT_SYMBOL(rt_stat_netifMib_get);
EXPORT_SYMBOL(rt_stat_netifMib_reset);
EXPORT_SYMBOL(rt_stat_gemFilter_set);
EXPORT_SYMBOL(rt_stat_gemFilter_get);
EXPORT_SYMBOL(rt_stat_gemFilterMib_reset);
EXPORT_SYMBOL(rt_stat_gemFilterMib_get);
EXPORT_SYMBOL(rt_stat_aclMib_get);
EXPORT_SYMBOL(rt_stat_aclMib_set);
EXPORT_SYMBOL(rt_stat_aclMib_reset);
