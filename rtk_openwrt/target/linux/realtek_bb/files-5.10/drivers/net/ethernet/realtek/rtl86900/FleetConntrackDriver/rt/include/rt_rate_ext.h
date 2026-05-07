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
 * Purpose : Definition of rate extension API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of host policer
 *
 */


#ifndef __RT_RATE_EXT_H__
#define __RT_RATE_EXT_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rt_port_ext.h>	//rt_port_phy_port_mask_t
#include <rt_wlan_ext.h>	//rt_wlan_mbssid_mask_t

/*
 * Symbol Definition
 */
#if defined(CONFIG_FC_RTL8198F_SERIES)
#define RT_RATE_HOSTPOLICING_TABLE_SIZE	16
#else
#define RT_RATE_HOSTPOLICING_TABLE_SIZE	32
#endif
#define RT_RATE_HOST_METER_RATE_UNCHANGE 0xFFFFFFFF


/*
 * Data Declaration
 */

typedef struct rt_rate_host_policer_control_s{
    char mac_addr[ETHER_ADDR_LEN];
    int ingress_limit_control;
    int egress_limit_control;
    int mib_count_control;
    int ingress_limit_meter_index;
    int egress_limit_meter_index;
    uint32_t ingress_limit_rate;	/* RT_RATE_HOST_METER_RATE_UNCHANGE means do not change meter[ingress_limit_meter_index] configuration */
    uint32_t egress_limit_rate;		/* RT_RATE_HOST_METER_RATE_UNCHANGE means do not change meter[egress_limit_meter_index] configuration */
}rt_rate_host_policer_control_t;

typedef struct rt_rate_host_policer_mib_s{
    uint64_t rx_count;
    uint64_t tx_count;
}rt_rate_host_policer_mib_t;

typedef struct rt_rate_sw_rate_limit_specific_dos_conf_s{
    unsigned int ctagif;
	unsigned int ctagVid;
	char ingressDmac[ETHER_ADDR_LEN];
	unsigned int ingressDip;
	unsigned int isTcp;	// 0: ucp, 1: tcp
	unsigned int pktLenStart;
	unsigned int pktLenEnd;
}rt_rate_sw_rate_limit_specific_dos_conf_t;

typedef struct rt_rate_sw_rate_limit_conf_s{
	rt_port_phy_port_mask_t ingress_port_mask;
	rt_wlan_mbssid_mask_t ingress_wlan_dev_mask[RT_WLAN_DEVICE_MAX];	//for wlan0, wlan1, wlan2...
	int shareMeterIdx;	// -1: disabled
	// extension configuration
	rt_rate_sw_rate_limit_specific_dos_conf_t specific_dos;
}rt_rate_sw_rate_limit_conf_t;

typedef enum rt_rate_sw_rate_limit_type_e{
	RT_RATE_SW_RATE_LIMIT_TYPE_ARP_REQ = 0,
	RT_RATE_SW_RATE_LIMIT_TYPE_SPECIFIC_DOS,
	RT_RATE_SW_RATE_LIMIT_TYPE_TCP_SYN_ONLY,
	RT_RATE_SW_RATE_LIMIT_TYPE_UNKNOWN_UC,
	RT_RATE_SW_RATE_LIMIT_TYPE_MAX,
}rt_rate_sw_rate_limit_type_t;

typedef enum rt_rate_ext_meter_type_e
{
    RT_RATE_EXT_METER_TYPE_STORM,
    RT_RATE_EXT_METER_TYPE_HOST,
    RT_RATE_EXT_METER_TYPE_FLOW,
    RT_RATE_EXT_METER_TYPE_ACL,
    RT_RATE_EXT_METER_TYPE_SW,
    RT_RATE_EXT_METER_TYPE_END
} rt_rate_ext_meter_type_t;



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
extern int32
rt_rate_hostPolicerControl_get (
    uint32 index,
    rt_rate_host_policer_control_t* pPolicerControl );

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
extern int32
rt_rate_hostPolicerControl_set (
    uint32 index,
    rt_rate_host_policer_control_t policerControl );

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
extern int32
rt_rate_hostPolicerMib_get (
    uint32 index,
    rt_rate_host_policer_mib_t* pPolicerMib );

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
extern int32
rt_rate_hostPolicerMib_clear (
    uint32 index );

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
extern int32
rt_rate_sw_rate_limit_get (
    rt_rate_sw_rate_limit_type_t type,
    rt_rate_sw_rate_limit_conf_t* pSwRateLimit_conf );

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
 *      RT_ERR_OK				- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can set configuration of software rate limit of the type
 */
extern int32
rt_rate_sw_rate_limit_set (
    rt_rate_sw_rate_limit_type_t type,
    rt_rate_sw_rate_limit_conf_t swRateLimit_conf );


#endif /* __RT_RATE_EXT_H__ */
