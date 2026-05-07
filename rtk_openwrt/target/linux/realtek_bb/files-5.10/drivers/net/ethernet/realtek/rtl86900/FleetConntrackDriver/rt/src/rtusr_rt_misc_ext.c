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
 *		RT_ERR_DRIVER_NOT_FOUND			- Driver not found
 *		RT_ERR_NOT_ALLOWED				- Driver return fail
 * Note:
 *      The API can get configuration of learning limit of MAC address of the wlan device
 */
int32
rt_misc_wlan_macAddr_learning_limit_get (
    unsigned int wlan_idx,
    unsigned int device_idx,
    rt_misc_macAddr_learning_limit_t *pMacAddr_learning_limit )
{
	rtdrv_rt_miscCfg_ext_t rt_misc_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pMacAddr_learning_limit), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_misc_cfg.wlan_idx, &wlan_idx, sizeof(unsigned int));
	osal_memcpy(&rt_misc_cfg.device_idx, &device_idx, sizeof(unsigned int));
    GETSOCKOPT(RTDRV_RT_MISC_WLAN_MAC_LEARNING_LIMIT_GET, &rt_misc_cfg, rtdrv_rt_miscCfg_ext_t, 1);
    osal_memcpy(pMacAddr_learning_limit, &rt_misc_cfg.macAddr_learning_limit, sizeof(rt_misc_macAddr_learning_limit_t));

    return RT_ERR_OK;
}

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
 *		RT_ERR_DRIVER_NOT_FOUND			- Driver not found
 *		RT_ERR_NOT_ALLOWED				- Driver return fail
 * Note:
 *      The API can set configuration of learning limit of MAC address of the wlan device
 */
int32
rt_misc_wlan_macAddr_learning_limit_set (
    unsigned int wlan_idx,
    unsigned int device_idx,
    rt_misc_macAddr_learning_limit_t macAddr_learning_limit )
{
	rtdrv_rt_miscCfg_ext_t rt_misc_cfg={0};

    /* function body */
    osal_memcpy(&rt_misc_cfg.wlan_idx, &wlan_idx, sizeof(unsigned int));
	osal_memcpy(&rt_misc_cfg.device_idx, &device_idx, sizeof(unsigned int));
    osal_memcpy(&rt_misc_cfg.macAddr_learning_limit, &macAddr_learning_limit, sizeof(rt_misc_macAddr_learning_limit_t));
    SETSOCKOPT(RTDRV_RT_MISC_WLAN_MAC_LEARNING_LIMIT_SET, &rt_misc_cfg, rtdrv_rt_miscCfg_ext_t, 1);

    return RT_ERR_OK;
}

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
	rtdrv_rt_miscCfg_ext_t rt_misc_cfg;

	/* parameter check */
	RT_PARAM_CHK((NULL == pWan_access_limit), RT_ERR_NULL_POINTER);

	/* function body */
	GETSOCKOPT(RTDRV_RT_MISC_WAN_ACCESS_LIMIT_GET, &rt_misc_cfg, rtdrv_rt_miscCfg_ext_t, 1);
	osal_memcpy(pWan_access_limit, &rt_misc_cfg.wan_access_limit, sizeof(rt_misc_wan_access_limit_t));

	return RT_ERR_OK;
}	/* end of rt_misc_wan_access_limit_get */
	
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
	rtdrv_rt_miscCfg_ext_t rt_misc_cfg;

	osal_memset(&rt_misc_cfg,0,sizeof(rtdrv_rt_miscCfg_ext_t));

	/* function body */
	osal_memcpy(&rt_misc_cfg.wan_access_limit, &wan_access_limit, sizeof(rt_misc_wan_access_limit_t));
	SETSOCKOPT(RTDRV_RT_MISC_WAN_ACCESS_LIMIT_SET, &rt_misc_cfg, rtdrv_rt_miscCfg_ext_t, 1);

	return RT_ERR_OK;
}	/* end of rt_misc_wan_access_limit_set */

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
	rtdrv_rt_miscCfg_ext_t rt_misc_cfg;

	osal_memset(&rt_misc_cfg,0,sizeof(rtdrv_rt_miscCfg_ext_t));

	/* function body */
	osal_memcpy(&rt_misc_cfg.burst_packet, &burst_packet, sizeof(rt_misc_burst_packet_t));
	SETSOCKOPT(RTDRV_RT_MISC_BURST_PACKET_SET, &rt_misc_cfg, rtdrv_rt_miscCfg_ext_t, 1);

	return RT_ERR_OK;
}	/* end of rt_misc_burst_packet_set */

/* Function Name:
 *              rt_misc_wanPhyPortMask_set
 * Description:
 *              Set system wan portMask
 * Input:
 *              wan_portMask - system wan portMask
 * Output:
 *
 * Return:
 *              RT_ERR_OK                                               - OK
 *              RT_ERR_DRIVER_NOT_FOUND	                                - Driver not found
 *              RT_ERR_NOT_ALLOWED		                                - Driver return fail
 * Note:
 *              The API can set system wan portMask
 */
int32
rt_misc_wanPhyPortMask_set (
        rt_misc_wan_phyPortMask_t wan_portMask)
{
    rtdrv_rt_miscCfg_ext_t rt_misc_cfg = {0};

    /* function body */
    osal_memcpy(&rt_misc_cfg.wan_portMask, &wan_portMask, sizeof(rt_misc_wan_phyPortMask_t));
    SETSOCKOPT(RTDRV_RT_MISC_WANPHYPORTMASK_SET, &rt_misc_cfg, rtdrv_rt_miscCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_misc_wanPhyPortMask_set */

/* Function Name:
 *              rt_misc_wanPhyPortMask_get
 * Description:
 *              Get system wan portMask
 * Input:
 *
 * Output:
 *              pWan_portMask   - system wan portMask
 * Return:
 *              RT_ERR_OK                                               - OK
 *              RT_ERR_DRIVER_NOT_FOUND	                                - Driver not found
 *              RT_ERR_NOT_ALLOWED		                                - Driver return fail
 * Note:
 *              The API can get system wan portMask
 */
int32
rt_misc_wanPhyPortMask_get (
        rt_misc_wan_phyPortMask_t *pWan_portMask)
{
    rtdrv_rt_miscCfg_ext_t rt_misc_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pWan_portMask), RT_ERR_NULL_POINTER);

    /* function body */
    GETSOCKOPT(RTDRV_RT_MISC_WANPHYPORTMASK_GET, &rt_misc_cfg, rtdrv_rt_miscCfg_ext_t, 1);
    osal_memcpy(pWan_portMask, &rt_misc_cfg.wan_portMask, sizeof(rt_misc_wan_phyPortMask_t));
	return RT_ERR_OK;
}	/* end of rt_misc_wanPhyPortMask_get */


/* Function Name:
 *              rt_misc_wanFastFwdDev_add
 * Description:
 *              Add one fast forward dev to system wan configuration
 * Input:
 *              dev_name        - fast forward dev name
 * Output:
 *
 * Return:
 *              RT_ERR_OK                                               - OK
 *              RT_ERR_DRIVER_NOT_FOUND	                                - Driver not found
 *              RT_ERR_NOT_ALLOWED		                                - Driver return fail
 * Note:
 *              The API can add one fast forward dev to system wan configuration.
 */
int32
rt_misc_wanFastFwdDev_add (
        char* dev_name)
{
    rtdrv_rt_miscCfg_ext_t rt_misc_cfg = {0};

    /* parameter check */
    RT_PARAM_CHK((NULL == dev_name), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(rt_misc_cfg.dev_name, dev_name, sizeof(rt_misc_cfg.dev_name));
    SETSOCKOPT(RTDRV_RT_MISC_WANFASTFWDDEV_ADD, &rt_misc_cfg, rtdrv_rt_miscCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_misc_wanFastFwdDev_add */

/* Function Name:
 *              rt_misc_wanFastFwdDev_del
 * Description:
 *              delete one fast forward dev from system wan configuration
 * Input:
 *              dev_name        - fast forward dev name
 * Output:
 *
 * Return:
 *              RT_ERR_OK                                       - OK
 *              RT_ERR_DRIVER_NOT_FOUND	                        - Driver not found
 *              RT_ERR_NOT_ALLOWED		                        - Driver return fail
 * Note:
 *              The API can delete one wlan from wlanMask of system wan configuration.
 *              This configuration will update wlanMask of system wan configuration
 */
int32
rt_misc_wanFastFwdDev_del (
        char* dev_name)
{
    rtdrv_rt_miscCfg_ext_t rt_misc_cfg = {0};

    /* parameter check */
    RT_PARAM_CHK((NULL == dev_name), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(rt_misc_cfg.dev_name, dev_name, sizeof(rt_misc_cfg.dev_name));
    SETSOCKOPT(RTDRV_RT_MISC_WANFASTFWDDEV_DEL, &rt_misc_cfg, rtdrv_rt_miscCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_misc_wanFastFwdDev_del */


/* Function Name:
 *      rt_misc_vlanGroupMacLimit_add
 * Description:
 *      Create vlan group for MAC limit.
 * Input:
 *      port - [in]<tab>The port limited by this group.
 *      groupLimit - [in]<tab>The limit number of the new group.
 * Output:
 *		pGroupIdx - [out]<tab>The created group index.
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *		RT_ERR_NOT_ALLOWED 	    - Driver return fail
 * Note:
 *      None.
 */
int32
rt_misc_vlanGroupMacLimit_add(int port, int groupLimit, int *pGroupIdx)
{
    rtdrv_rt_vlanGroupMacLimit_add_t cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pGroupIdx), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&cfg.port, &port, sizeof(int));
    osal_memcpy(&cfg.groupLimit, &groupLimit, sizeof(int));
    GETSOCKOPT(RTDRV_RT_MISC_VLANGROUPMACLIMIT_ADD, &cfg, rtdrv_rt_vlanGroupMacLimit_add_t, 1);
    osal_memcpy(pGroupIdx, &cfg.pGroupIdx, sizeof(int));

    return RT_ERR_OK;
}   /* end of rt_misc_vlanGroupMacLimit_add */

/* Function Name:
 *      rt_misc_vlanGroupMacLimit_set
 * Description:
 *      Add vlan to exist vlan group for MAC limit.
 * Input:
 *      groupIdx - [in]<tab>The group index which needed to add.
 *      vlanId - [in]<tab>The vlanID add to the group, use -1 if untag, use -2 if disabled.
 *      groupLimit - [in]<tab>The limit number of the group, use -1 if disabled.
 * Output:
 *      None
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *		RT_ERR_NOT_ALLOWED 	    - Driver return fail
 * Note:
 *      None.
 */
int32
rt_misc_vlanGroupMacLimit_set(int groupIdx, int vlanId, int groupLimit)
{
    rtdrv_rt_vlanGroupMacLimit_set_t cfg;

    /* function body */
    osal_memcpy(&cfg.groupIdx, &groupIdx, sizeof(int));
    osal_memcpy(&cfg.vlanId, &vlanId, sizeof(int));
    osal_memcpy(&cfg.groupLimit, &groupLimit, sizeof(int));
    SETSOCKOPT(RTDRV_RT_MISC_VLANGROUPMACLIMIT_SET, &cfg, rtdrv_rt_vlanGroupMacLimit_set_t, 1);

    return RT_ERR_OK;
}   /* end of rt_misc_vlanGroupMacLimit_set */

/* Function Name:
 *      rt_misc_vlanGroupMacLimit_del
 * Description:
 *      Delete vlan group for MAC limit.
 * Input:
 *      groupIdx - [in]<tab>The group index which needed to delete.
 *      vlanId - [in]<tab>The vlanID add to the group, use -1 if untag, use -2 for delete all.
 * Output:
 *      None
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *		RT_ERR_NOT_ALLOWED 	    - Driver return fail
 * Note:
 *      The group will be deleted when the last one vlan had been removed from it.
 */
int32
rt_misc_vlanGroupMacLimit_del(int groupIdx, int vlanId)
{
    rtdrv_rt_vlanGroupMacLimit_del_t cfg;

    /* function body */
    osal_memcpy(&cfg.groupIdx, &groupIdx, sizeof(int));
    osal_memcpy(&cfg.vlanId, &vlanId, sizeof(int));
    SETSOCKOPT(RTDRV_RT_MISC_VLANGROUPMACLIMIT_DEL, &cfg, rtdrv_rt_vlanGroupMacLimit_del_t, 1);

    return RT_ERR_OK;
}   /* end of rt_misc_vlanGroupMacLimit_del */

/* Function Name:
 *      rt_misc_vlanGroupMacLimit_get
 * Description:
 *      Get vlan group for MAC limit by index.
 * Input:
 *      groupIdx - [in]<tab>The group index to get.
 * Output:
 *      pGroup - [out]<tab>The information contained.
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *		RT_ERR_NOT_ALLOWED 	    - Driver return fail
 * Note:
 *      None.
 */
int32
rt_misc_vlanGroupMacLimit_get(int groupIdx, rt_misc_vlanGroupMacLimit_info_t *pGroupInfo)
{
    rtdrv_rt_vlanGroupMacLimit_get_t cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pGroupInfo), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&cfg.groupIdx, &groupIdx, sizeof(int));
    GETSOCKOPT(RTDRV_RT_MISC_VLANGROUPMACLIMIT_GET, &cfg, rtdrv_rt_vlanGroupMacLimit_get_t, 1);
    osal_memcpy(pGroupInfo, &cfg.pGroupInfo, sizeof(rt_misc_vlanGroupMacLimit_info_t));

    return RT_ERR_OK;
}   /* end of rt_misc_vlanGroupMacLimit_get */

/* Function Name:
 *      rt_misc_vlanGroupMacLimit_find
 * Description:
 *      Find vlan group index for MAC limit.
 * Input:
 *      port - [in]<tab>The port limited by this group.
 *      vlanId - [in]<tab>The vlanID try to find out the group, use -1 if untag.
 * Output:
 *      pGroupIdx - [out]<tab>The group index which contained the vlan.
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *		RT_ERR_NOT_ALLOWED 	    - Driver return fail
 * Note:
 *      None.
 */
int32
rt_misc_vlanGroupMacLimit_find(int port, int vlanId, int *pGroupIdx)
{
    rtdrv_rt_vlanGroupMacLimit_find_t cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pGroupIdx), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&cfg.port, &port, sizeof(int));
    osal_memcpy(&cfg.vlanId, &vlanId, sizeof(int));
    GETSOCKOPT(RTDRV_RT_MISC_VLANGROUPMACLIMIT_FIND, &cfg, rtdrv_rt_vlanGroupMacLimit_find_t, 1);
    osal_memcpy(pGroupIdx, &cfg.pGroupIdx, sizeof(int));

    return RT_ERR_OK;
}   /* end of rt_misc_vlanGroupMacLimit_find */

		
		
