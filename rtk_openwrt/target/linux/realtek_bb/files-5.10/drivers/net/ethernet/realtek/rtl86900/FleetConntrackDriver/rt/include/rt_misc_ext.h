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
 * Purpose : Definition of Misc extension API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of mac leraning limit
 *           (2) Configuration of wan access limit
 *           (3) Configuration of burst packet transmission
 *
 */


#ifndef __RT_MISC_EXT_H__
#define __RT_MISC_EXT_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rt_port_ext.h>	//rt_port_phy_port_mask_t
#include <rt_wlan_ext.h>	//rt_wlan_mbssid_mask_t

/*
 * Symbol Definition
 */

#define RT_MISC_JUMBO_BUF_SIZE	(13312+2)		//IXIA max packet size.
#define RT_MISC_MAX_VLAN_HW_TABLE_SIZE (4096)

typedef enum rt_wan_access_limit_fields_e
{
	RT_MISC_WAN_ACCESS_LIMIT_BY_MAC = 0,
	RT_MISC_WAN_ACCESS_LIMIT_BY_IP,
} rt_wan_access_limit_fields_t;

/*
 * Macro Declaration
 */
 
#define rt_misc_port_mask_t       rt_port_phy_port_mask_t
#define rt_misc_wlan_mbssid_mask_t       rt_wlan_mbssid_mask_t

/*
 * Data Declaration
 */

typedef struct rt_misc_macAddr_learning_limit_s{
    int learningLimitNumber;	// -1: unlimit
}rt_misc_macAddr_learning_limit_t;

typedef struct rt_misc_wan_access_limit_s{
	rt_misc_port_mask_t portMask;
	rt_misc_wlan_mbssid_mask_t wlanMask[RT_WLAN_DEVICE_MAX];
	int accessLimitNumber;	// -1: unlimit
	rt_wan_access_limit_fields_t limitField;
}rt_misc_wan_access_limit_t;

typedef struct rt_misc_burst_packet_s{
	rt_port_t destPort;
	char *pktData;
	int pktLen;
	int burstCount;					//-1 for infinite mode, 0 for stop
	unsigned char burstRate;		//range from 0 to 100 percentage of line rate
	unsigned char dest_stream_id;	//for RT_PORT_PON
}rt_misc_burst_packet_t;

typedef struct rt_misc_wan_phyPortMask_s{
	rt_misc_port_mask_t mask;
}rt_misc_wan_phyPortMask_t;

typedef struct rt_misc_vlanGroupMacLimit_info_s
{
	unsigned int vlanMask[RT_MISC_MAX_VLAN_HW_TABLE_SIZE>>5];
	unsigned int untag:1;

	unsigned int valid:1;

	int port;
	int mac_limit_number;
	//atomic_t mac_count;
}rt_misc_vlanGroupMacLimit_info_t;



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
extern int32
rt_misc_wlan_macAddr_learning_limit_get (
    rt_wlan_index_t wlan_idx,
    rt_wlan_mbssid_index_t device_idx,
    rt_misc_macAddr_learning_limit_t *pMacAddr_learning_limit );

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
extern int32
rt_misc_wlan_macAddr_learning_limit_set (
    rt_wlan_index_t wlan_idx,
    rt_wlan_mbssid_index_t device_idx,
    rt_misc_macAddr_learning_limit_t macAddr_learning_limit );

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
extern int32
rt_misc_wan_access_limit_get (
	rt_misc_wan_access_limit_t *pWan_access_limit );
	
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
extern int32
rt_misc_wan_access_limit_set (
	rt_misc_wan_access_limit_t wan_access_limit );

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
extern int32
rt_misc_burst_packet_set (
	rt_misc_burst_packet_t burst_packet );

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
extern int32
rt_misc_wanPhyPortMask_set (
	rt_misc_wan_phyPortMask_t wan_portMask);

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

extern int32
rt_misc_wanPhyPortMask_get (
	rt_misc_wan_phyPortMask_t *pWan_portMask);

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
extern int32
rt_misc_wanFastFwdDev_add (
	char* dev_name);

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
extern int32
rt_misc_wanFastFwdDev_del (
	char* dev_name);




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
extern int32
rt_misc_vlanGroupMacLimit_add(int port, int groupLimit, int *pGroupIdx);

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
extern int32
rt_misc_vlanGroupMacLimit_set(int groupIdx, int vlanId, int groupLimit);

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
extern int32
rt_misc_vlanGroupMacLimit_del(int groupIdx, int vlanId);

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
extern int32
rt_misc_vlanGroupMacLimit_get(int groupIdx, rt_misc_vlanGroupMacLimit_info_t *pGroupInfo);

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
extern int32
rt_misc_vlanGroupMacLimit_find(int port, int vlanId, int *pGroupIdx);

		


#endif /* __RT_MISC_EXT_H__ */
