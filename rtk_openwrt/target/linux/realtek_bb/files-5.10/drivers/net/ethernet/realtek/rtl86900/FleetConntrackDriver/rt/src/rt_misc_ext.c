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
 * $Revision$
 * $Date$
 *
 * Purpose : Definition of RT API of Misc Extension
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of mac leraning limit
 *           (2) Configuration of wan access limit
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
 *      rt_misc_wlan_macAddr_learning_limit_get
 * Description:
 *      Get configuration of learning limit of MAC address of the wlan device
 * Input:
 *      wlan_idx        			- wlan index
 *      device_idx        			- deivce index
 * Output:
 *      pMacAddr_learning_limit 	- Configuration of learning limit of MAC address of the wlan device
 * Return:
 *      RT_ERR_OK               		- OK
 *      RT_ERR_DRIVER_NOT_FOUND 		- Driver not found
 *      RT_ERR_NOT_ALLOWED       		- Driver return fail
 * Note:
 *      The API can get configuration of learning limit of MAC address of the wlan device
 */
int32
rt_misc_wlan_macAddr_learning_limit_get (
    rt_wlan_index_t wlan_idx,
    rt_wlan_mbssid_index_t device_idx,
    rt_misc_macAddr_learning_limit_t *pMacAddr_learning_limit )
{
    int32   ret=RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->misc_wlan_macAddr_learning_limit_get == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->misc_wlan_macAddr_learning_limit_get(wlan_idx, device_idx, pMacAddr_learning_limit))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;
}   /* end of rt_misc_wlan_macAddr_learning_limit_get */

/* Function Name:
 *      rt_misc_wlan_macAddr_learning_limit_set
 * Description:
 *      Set configuration of learning limit of MAC address of the wlan device
 * Input:
 *      wlan_idx        			- wlan index
 *      device_idx        			- deivce index
 *      policerControl 			- Configuration of learning limit of MAC address of the wlan device
 * Output:
 *      
 * Return:
 *      RT_ERR_OK               		- OK
 * 	 RT_ERR_DRIVER_NOT_FOUND 			- Driver not found
 * 	 RT_ERR_NOT_ALLOWED 	  			- Driver return fail
 * Note:
 *      The API can set configuration of learning limit of MAC address of the wlan device
 */
int32
rt_misc_wlan_macAddr_learning_limit_set (
    rt_wlan_index_t wlan_idx,
    rt_wlan_mbssid_index_t device_idx,
    rt_misc_macAddr_learning_limit_t macAddr_learning_limit )
{
    int32   ret=RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->misc_wlan_macAddr_learning_limit_set == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->misc_wlan_macAddr_learning_limit_set(wlan_idx, device_idx, macAddr_learning_limit))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;
}   /* end of rt_misc_wlan_macAddr_learning_limit_set */

/* Function Name:
 *		rt_misc_wan_access_limit_get
 * Description:
 *		Get configuration of wan access limit
 * Input:
 * Output:
 *		pWan_access_limit	- Configuration of wan access limit
 * Return:
 *		RT_ERR_OK						- OK
 *		RT_ERR_DRIVER_NOT_FOUND 		- Driver not found
 *		RT_ERR_NOT_ALLOWED				- Driver return fail
 * Note:
 *		The API can get configuration of wan access limit
 */
int32
rt_misc_wan_access_limit_get (
	rt_misc_wan_access_limit_t *pWan_access_limit )
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->misc_wan_access_limit_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->misc_wan_access_limit_get(pWan_access_limit))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}
	
/* Function Name:
 *		rt_misc_wan_access_limit_set
 * Description:
 *		Set configuration of wan access limit
 * Input:
 *		wan_access_limit	- Configuration of wan access limit
 * Output:
 *		
 * Return:
 *		RT_ERR_OK						- OK
 *		RT_ERR_DRIVER_NOT_FOUND 		- Driver not found
 *		RT_ERR_NOT_ALLOWED				- Driver return fail
 * Note:
 *		The API can set configuration of wan access limit
 */
int32
rt_misc_wan_access_limit_set (
	rt_misc_wan_access_limit_t wan_access_limit )
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->misc_wan_access_limit_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->misc_wan_access_limit_set(wan_access_limit))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}

/* Function Name:
 *		rt_misc_burst_packet_set
 * Description:
 *		Setup user-defined packet sending to desired port for predetermined number and rate.
 * Input:
 *		burst_packet	- Configuration of burst packet
 * Output:
 *		
 * Return:
 *		RT_ERR_OK						- OK
 *		RT_ERR_DRIVER_NOT_FOUND 		- Driver not found
 *		RT_ERR_NOT_ALLOWED				- Driver return fail
 * Note:
 *		The API can set configuration of burst packet creation or obliteration
 */
int32
rt_misc_burst_packet_set (
	rt_misc_burst_packet_t burst_packet )
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;
	
	if(p_rt_ext->misc_burst_packet_set == NULL)
			return RT_ERR_DRIVER_NOT_FOUND;
	
	if((p_rt_ext->misc_burst_packet_set(burst_packet))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}

/* Function Name:
 *      rt_misc_wanPhyPortMask_set
 * Description:
 *      Set system wan portMask
 * Input:
 *      wan_portMask		- system wan portMask
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can set system wan portMask
 */
int32
rt_misc_wanPhyPortMask_set (
	rt_misc_wan_phyPortMask_t wan_portMask)
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->misc_wanPhyPortMask_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->misc_wanPhyPortMask_set(wan_portMask))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}

/* Function Name:
 *      rt_misc_wanPhyPortMask_get
 * Description:
 *      Get system wan portMask
 * Input:
 *      None.
 * Output:
 *      pWan_portMask	- system wan portMask
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can get system wan portMask
 */
int32
rt_misc_wanPhyPortMask_get (
	rt_misc_wan_phyPortMask_t *pWan_portMask)
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->misc_wanPhyPortMask_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->misc_wanPhyPortMask_get(pWan_portMask))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}

/* Function Name:
 *      rt_misc_wanFastFwdDev_add
 * Description:
 *      Add one fast forward dev to system wan configuration
 * Input:
 *      dev_name		- fast forward dev name
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can add one fast forward dev to system wan configuration.
 */
int32
rt_misc_wanFastFwdDev_add (
	char* dev_name)
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->misc_wanFastFwdDev_add == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->misc_wanFastFwdDev_add(dev_name))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}

/* Function Name:
 *      rt_misc_wanFastFwdDev_del
 * Description:
 *      Delete one fast forward dev from system wan configuration
 * Input:
 *      dev_name		- fast forward dev name
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can delete one wlan from wlanMask of system wan configuration.
 *      This configuration will update wlanMask of system wan configuration
 */
int32
rt_misc_wanFastFwdDev_del (
	char* dev_name)
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->misc_wanFastFwdDev_del == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->misc_wanFastFwdDev_del(dev_name))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}
/* Function Name:
 *      rt_misc_vlanGroupMacLimit_add
 * Description:
 *      Add vlan group mac limit
 * Input:
 *      port		- port index
 *		groupLimit  - group limit
 *      pGroupIdx   - Group index pointer
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *     
 */


int32 rt_misc_vlanGroupMacLimit_add(int port, int groupLimit, int *pGroupIdx)
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->misc_vlanGroupMacLimit_add == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->misc_vlanGroupMacLimit_add(port, groupLimit, pGroupIdx))!=0)
			return RT_ERR_NOT_ALLOWED;

	return ret;
}


/* Function Name:
 *      rt_misc_vlanGroupMacLimit_set
 * Description:
 *      Set vlan group mac limit
 * Input:
 *      groupIdx	- group index
 *		groupLimit  - group limit
 *      vlanId   	- vlanId
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *     
 */

int32 rt_misc_vlanGroupMacLimit_set(int groupIdx, int vlanId, int groupLimit)
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->misc_vlanGroupMacLimit_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->misc_vlanGroupMacLimit_set(groupIdx, vlanId, groupLimit))!=0)
			return RT_ERR_NOT_ALLOWED;

	return ret;
}


/* Function Name:
 *      rt_misc_vlanGroupMacLimit_del
 * Description:
 *    	Delete vlan group mac limit
 * Input:
 *      groupIdx	- group index
 *		groupLimit  - group limit
 *      vlanId   	- vlanId
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *     
 */

int32 rt_misc_vlanGroupMacLimit_del(int groupIdx, int vlanId)
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->misc_vlanGroupMacLimit_del == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->misc_vlanGroupMacLimit_del(groupIdx, vlanId))!=0)
			return RT_ERR_NOT_ALLOWED;

	return ret;
}


/* Function Name:
 *      rt_misc_vlanGroupMacLimit_get
 * Description:
 *    	Get vlan group mac limit
 * Input:
 *      groupIdx	- group index
 *		pGroup  	- group info pointer
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *     
 */

int32 rt_misc_vlanGroupMacLimit_get(int groupIdx, rt_misc_vlanGroupMacLimit_info_t *pGroup)
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->misc_vlanGroupMacLimit_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;
	
	if((p_rt_ext->misc_vlanGroupMacLimit_get(groupIdx, pGroup))!=0)
			return RT_ERR_NOT_ALLOWED;


	return ret;
}


/* Function Name:
 *      rt_misc_vlanGroupMacLimit_find
 * Description:
 *    	Find vlan group mac limit
 * Input:
 *      port		- port index
 *      vlanId		- vlan index
 *		pGroupIdx  	- group index pointer
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *     
 */

int32 rt_misc_vlanGroupMacLimit_find(int port, int vlanId, int *pGroupIdx)
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->misc_vlanGroupMacLimit_find == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->misc_vlanGroupMacLimit_find(port, vlanId, pGroupIdx))!=0)
			return RT_ERR_NOT_ALLOWED;

	return ret;
}



EXPORT_SYMBOL(rt_misc_wlan_macAddr_learning_limit_get);
EXPORT_SYMBOL(rt_misc_wlan_macAddr_learning_limit_set);
EXPORT_SYMBOL(rt_misc_wan_access_limit_get);
EXPORT_SYMBOL(rt_misc_wan_access_limit_set);
EXPORT_SYMBOL(rt_misc_burst_packet_set);
EXPORT_SYMBOL(rt_misc_wanPhyPortMask_set);
EXPORT_SYMBOL(rt_misc_wanPhyPortMask_get);
EXPORT_SYMBOL(rt_misc_wanFastFwdDev_add);
EXPORT_SYMBOL(rt_misc_wanFastFwdDev_del);
EXPORT_SYMBOL(rt_misc_vlanGroupMacLimit_add);
EXPORT_SYMBOL(rt_misc_vlanGroupMacLimit_del);
EXPORT_SYMBOL(rt_misc_vlanGroupMacLimit_set);
EXPORT_SYMBOL(rt_misc_vlanGroupMacLimit_get);
EXPORT_SYMBOL(rt_misc_vlanGroupMacLimit_find);

