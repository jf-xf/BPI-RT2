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


#ifndef __RT_STAT_EXT_H__
#define __RT_STAT_EXT_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/rt/rt_stat.h>


/*
 * Symbol Definition
 */
#define RT_STAT_FLOWMIB_TABLE_SIZE 32
#define RT_STAT_FLOW_TABLE_SIZE 65536
#define RT_STAT_GEMMIB_TABLE_SIZE 32
#define RT_STAT_GEMMIB_FILTER_VID_INVAILD 0xffff
#define RT_STAT_GEMMIB_FILTER_PRI_INVAILD 8
#define RT_STAT_GEMMIB_FILTER_PKTTYPE_INVAILD 0
#define RT_STAT_ACLMIB_TABLE_SIZE 32


/*
 * Data Declaration
 */
typedef struct rt_stat_flowCache_mib_s{
	uint32 packet_count;
	uint64 byte_count;
}rt_stat_flowCache_mib_t;

typedef struct rt_stat_flow_mib_s{
	uint32 ingress_packet_count;
	uint64 ingress_byte_count;
	uint32 egress_packet_count;
	uint64 egress_byte_count;
}rt_stat_flow_mib_t;

typedef struct rt_stat_netif_mib_s{
	/* uc+mc+bc counter */
	uint32 ingress_packet_count;
	uint64 ingress_byte_count;
	uint32 egress_packet_count;
	uint64 egress_byte_count;
	/* uc counter */
	uint32 ucCntValid;
	uint32 ingress_uc_packet_count;
	uint64 ingress_uc_byte_count;
	uint32 egress_uc_packet_count;
	uint64 egress_uc_byte_count;	
	/* mc counter */
	uint32 mcCntValid;
	uint32 ingress_mc_packet_count;
	uint64 ingress_mc_byte_count;
	uint32 egress_mc_packet_count;
	uint64 egress_mc_byte_count;	
	/* bc counter */
	uint32 bcCntValid;
	uint32 ingress_bc_packet_count;
	uint64 ingress_bc_byte_count;
	uint32 egress_bc_packet_count;
	uint64 egress_bc_byte_count;	
	
	int8 swNetifIdx;		/* -1: invalid */
	int8 hwNetifIdx;		/* -1: invalid */
}rt_stat_netif_mib_t;

typedef enum rt_stat_gemFilter_type_e{
	RT_STAT_FILTER_FLOW,
	RT_STAT_FILTER_PORT,
	RT_STAT_FILTER_END
}rt_stat_gemFilter_type_t;

typedef enum rt_stat_gemFilter_pkttype_e{
	RT_STAT_FILTER_PKTTYPE_DISABLE = RT_STAT_GEMMIB_FILTER_PKTTYPE_INVAILD,
	RT_STAT_FILTER_PKTTYPE_UC_ONLY,
	RT_STAT_FILTER_PKTTYPE_MC_ONLY,
	RT_STAT_FILTER_PKTTYPE_BC_ONLY,
}rt_stat_gemFilter_pkttype_t;

typedef struct rt_stat_gemFilter_conf_s{
	uint8 enable;
	rt_stat_gemFilter_type_t type;
	union {
		uint16 type_flowId; 		// sw stream idx
		uint16 type_portId;		// uni port idx
	};
	uint16 filter_vid;				// RT_STAT_GEMMIB_FILTER_VID_INVAILD
	uint8 filter_pri;				// RT_STAT_GEMMIB_FILTER_PRI_INVAILD
	rt_stat_gemFilter_pkttype_t filter_pkttype;
}rt_stat_gemFilter_conf_t;

typedef struct rt_stat_acl_mib_s{
	uint8 type;					//0:byte count 1:packet count
	uint32 packet_count;
	uint64 byte_count;
}rt_stat_acl_mib_t;

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
extern int32
rt_stat_flowCache_mib_get (
    uint32 flow_idx,
    rt_stat_flowCache_mib_t* pFlowCacheMib );

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
extern int32
rt_stat_flowCache_mib_reset (
    uint32 flow_idx );


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
extern int32
rt_stat_flowMib_get (
    uint32 index,
    rt_stat_flow_mib_t* pFlowMib );

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
extern int32
rt_stat_flowMib_reset (
    uint32 index );

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
 *      The API can get netif counters
 */
extern int32
rt_stat_netifMib_get (
    char* dev_name,
    rt_stat_netif_mib_t* pNetifMib );

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
 *      The API can reset netif counters
 */
extern int32
rt_stat_netifMib_reset (
    char* dev_name );

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
extern int32 
rt_stat_gemFilter_set (
	uint32 index, 
	rt_stat_gemFilter_conf_t conf );

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
extern int32 
rt_stat_gemFilter_get (
	uint32 index, 
	rt_stat_gemFilter_conf_t* pConf );

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
extern int32 
rt_stat_gemFilterMib_reset (
	uint32 index );

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
extern int32 
rt_stat_gemFilterMib_get (
	uint32 index,
	rt_stat_port_cntr_t* pCntr);


/* Function Name:
 *		rt_stat_aclMib_get
 * Description:
 *		Get acl counters by acl mib index.
 * Input:
 *		index		- Acl mib index
 * Output:
 *		pAclMib	- Acl MIB
 * Return:
 *		RT_ERR_OK					- OK
 *		RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *		RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *		The API can get acl counters
 */
extern int32
rt_stat_aclMib_get (
	uint32 index,
	rt_stat_acl_mib_t* pAclMib );

/* Function Name:
 *		rt_stat_aclMib_set
 * Description:
 *		Set acl counters type by acl mib index.
 * Input:
 *		index		- Acl mib index
 * Output:
 *		pAclMib - Acl MIB
 * Return:
 *		RT_ERR_OK					- OK
 *		RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *		RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *		The API can get acl counters
 */
extern int32
rt_stat_aclMib_set (
	uint32 index,
	rt_stat_acl_mib_t pAclMib );

/* Function Name:
 *		rt_stat_aclMib_reset
 * Description:
 *		Reset acl counters by acl mib index.
 * Input:
 *		index	- Acl mib index
 * Output:
 *		None.
 * Return:
 *		RT_ERR_OK					- OK
 *		RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *		RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *		The API can reset acl counters
 */
extern int32
rt_stat_aclMib_reset (
	uint32 index );


#if defined(CONFIG_RTK_SOC_RTL8198D)
/*
 * eth traffic idx:						[0, 7]
 * 5g wlan traffic idx, count by hw		[8, 13]
 * 2g wlan traffic idx, count by hw		[14, 15]
 * wlan traffic idx, count by sw:		[16, 71]
 */
#define RT_STAT_EXT_HW_FLOWMIB_ENTRY_SIZE				(RT_STAT_FLOWMIB_TABLE_SIZE/2)
#define RT_STAT_EXT_FLOWMIB_ETH_ENTRY_SIZE				(6)
#define RT_STAT_EXT_FLOWMIB_HW_5G_WLAN_ENTRY_SIZE		(6)
#define RT_STAT_EXT_FLOWMIB_HW_2G_WLAN_ENTRY_SIZE		(4)

#define RT_STAT_EXT_FLOWMIB_WLAN_ENTRY_SIZE				(64)		// hw + sw
#define RT_STAT_EXT_FLOWMIB_TABLE_SIZE					(RT_STAT_EXT_FLOWMIB_ETH_ENTRY_SIZE+RT_STAT_EXT_FLOWMIB_WLAN_ENTRY_SIZE)

#define RT_STAT_UP_HW_FLOW_MIB_MAPPED_FROM_EXT_FLOW_MIB(idx)			(2*idx)
#define RT_STAT_DOWN_HW_FLOW_MIB_MAPPED_FROM_EXT_FLOW_MIB(idx)			(2*idx+1)

/* Function Name:
 *		rt_stat_hwFlowMib_get
 * Description:
 *		Get hw flow counters by hw flow mib index.
 * Input:
 *		index			- hw Flow mib index
 * Output:
 *		pFlowMib		- Flow MIB
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *		The API can get hw flow counters
 */

extern int32
rt_stat_hwFlowMib_get (
	uint32 index,
	rt_stat_flow_mib_t* pFlowMib );

/* Function Name:
 *		rt_stat_brNetifInfo_get
 * Description:
 *		get bridge netif info.
 * Input:
 *		br_name 	- bridge netif name.
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
extern int32
rt_stat_brNetifInfo_get(char *br_name, rtk_mac_t *br_mac, uint32 *br_ip);

#endif
#endif /* __RT_STAT_EXT_H__ */
