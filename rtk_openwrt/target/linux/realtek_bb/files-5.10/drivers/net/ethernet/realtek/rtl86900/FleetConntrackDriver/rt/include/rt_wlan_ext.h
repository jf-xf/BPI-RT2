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
 * Purpose : Definition of WLAN extension API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Parameter definition for wireless mbssid
 *
 */


#ifndef __RT_WLAN_EXT_H__
#define __RT_WLAN_EXT_H__


/*
 * Include Files
 */
#include <common/rt_type.h>


/*
 * Symbol Definition
 */

/* For WLAN Device Index */
typedef enum rt_wlan_index_e
{
	RT_WLAN_DEVICE0_INDEX = 0,
	RT_WLAN_DEVICE1_INDEX,
	RT_WLAN_DEVICE2_INDEX,
	RT_WLAN_DEVICE_MAX,
} rt_wlan_index_t;

/* For WLAN MBSSID Index */
typedef enum rt_wlan_mbssid_index_e
{
	RT_WLAN_MBSSID_ROOT_INTF_INDEX = 0,
	RT_WLAN_MBSSID_VAP0_INTF_INDEX,
	RT_WLAN_MBSSID_VAP1_INTF_INDEX,
	RT_WLAN_MBSSID_VAP2_INTF_INDEX,
	RT_WLAN_MBSSID_VAP3_INTF_INDEX,
	RT_WLAN_MBSSID_VAP4_INTF_INDEX,
	RT_WLAN_MBSSID_VAP5_INTF_INDEX,
	RT_WLAN_MBSSID_VAP6_INTF_INDEX,
	RT_WLAN_MBSSID_VAP7_INTF_INDEX,
	RT_WLAN_MBSSID_CLIENT_INTF_INDEX,
	RT_WLAN_MBSSID_MESH_INTF_INDEX,
	RT_WLAN_MBSSID_MAX,
} rt_wlan_mbssid_index_t;

/* For WLAN MBSSID Mask */
typedef enum rt_wlan_mbssid_mask_e
{
	RT_WLAN_MBSSID_NONE	     = 0x0,
	RT_WLAN_MBSSID_ROOT_INTF_BIT = (0x1<<RT_WLAN_MBSSID_ROOT_INTF_INDEX),
	RT_WLAN_MBSSID_VAP0_INTF_BIT = (0x1<<RT_WLAN_MBSSID_VAP0_INTF_INDEX),
	RT_WLAN_MBSSID_VAP1_INTF_BIT = (0x1<<RT_WLAN_MBSSID_VAP1_INTF_INDEX),
	RT_WLAN_MBSSID_VAP2_INTF_BIT = (0x1<<RT_WLAN_MBSSID_VAP2_INTF_INDEX),
	RT_WLAN_MBSSID_VAP3_INTF_BIT = (0x1<<RT_WLAN_MBSSID_VAP3_INTF_INDEX),
	RT_WLAN_MBSSID_VAP4_INTF_BIT = (0x1<<RT_WLAN_MBSSID_VAP4_INTF_INDEX),
	RT_WLAN_MBSSID_VAP5_INTF_BIT = (0x1<<RT_WLAN_MBSSID_VAP5_INTF_INDEX),
	RT_WLAN_MBSSID_VAP6_INTF_BIT = (0x1<<RT_WLAN_MBSSID_VAP6_INTF_INDEX),
	RT_WLAN_MBSSID_VAP7_INTF_BIT = (0x1<<RT_WLAN_MBSSID_VAP7_INTF_INDEX),
	RT_WLAN_MBSSID_CLIENT_INTF_BIT = (0x1<<RT_WLAN_MBSSID_CLIENT_INTF_INDEX),
	RT_WLAN_MBSSID_MESH_INTF_BIT = (0x1<<RT_WLAN_MBSSID_MESH_INTF_INDEX),
	RT_WLAN_MBSSID_ALL_BIT = ((0x1<<RT_WLAN_MBSSID_MAX)-1),
} rt_wlan_mbssid_mask_t;

/*
 * Data Declaration
 */


/*
 * Function Declaration
 */


#endif /* __RT_WLAN_EXT_H__ */
