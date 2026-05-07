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
 * Purpose : Definition of IGMP extension API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of multicast learning mode
 *           (2) Configuration of multicast group
 *
 */


#ifndef __RT_IGMP_EXT_H__
#define __RT_IGMP_EXT_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rt_wlan_ext.h>
#include <rt_port_ext.h>


/*
 * Symbol Definition
 */

/* multicast mode */
typedef enum rt_igmp_multicast_learning_mode_e
{
	RT_IGMP_MULTICAST_SYNC_AUTO=0,
	RT_IGMP_MULTICAST_SYNC_MODE_USER,
	RT_IGMP_MULTICAST_SYNC_MODE_KERNEL,
	RT_IGMP_MULTICAST_END,
} rt_igmp_multicast_learning_mode_t;

/*
 * Data Declaration
 */
/* multicast group structure */
typedef enum rt_igmp_multicast_groupBehavior_e
{
	RT_MC_BEHAVIOR_FLOW_FORWAED		=0,		//flow forward behavior
	RT_MC_BEHAVIOR_GROUP_AS_KNOW	=1,		//treat this group as known group
	RT_MC_BEHAVIOR_GROUP_AS_UNKNOW	=2,		//treat this group as unknown group
	RT_MC_CHECK_UNKNOWN_FLOOD_FLOW	=3,		//check unknown flood flow is still exsit
	RT_MC_BEHAVIOR_FLOW_UPDATE		=4,		//group members may update 
	RT_MC_BEHAVIOR_GROUP_DEFAULT_FWD=5,		//for the group default forward ports/ifidxs
	RT_MC_BEHAVIOR_MAX,
} rt_igmp_multicast_groupBehavior_t;


typedef struct rt_igmp_multicast_group_cfg_s{

	unsigned char   groupBehavior;		// ref to rt_igmp_multicast_groupBehavior_t
	unsigned char	is_ipv6;	 		// 0: ipv4, 1: ipv6 
	unsigned char	careSourceAddress; 	// only valid when groupBehavior= RT_MC_BEHAVIOR_FLOW_FORWAED  0: do not care sourceAddress 1:should care sourceAddress
	unsigned char	careIngressSvid;	// only valid when groupBehavior= RT_MC_BEHAVIOR_FLOW_FORWAED  0: do not care Ingress Svid  1:Ingress Svid should Match
	unsigned char	careIngressCvid;	// only valid when groupBehavior= RT_MC_BEHAVIOR_FLOW_FORWAED  0: do not care Ingress Cvid  1:Ingress Cvid should Match
	union {
		unsigned int	ipv4;
	 	uint8   		ipv6[16];
	}group_addr;
	union {
		unsigned int	ipv4;
	 	uint8   		ipv6[16];
	}source_addr;
	unsigned char	ingress_stagif;
	unsigned int	ingress_svid;	
	unsigned char	ingress_ctagif;
	unsigned int	ingress_cvid;
	
	rt_port_phy_port_mask_t	first_act_portmask;
	rt_wlan_mbssid_mask_t	first_act_wlanMbssidMask[RT_WLAN_DEVICE_MAX];
	unsigned char	first_act_ctagif;
	unsigned int	first_act_cvid;
	unsigned char	first_act_cpri;
	rt_port_phy_port_mask_t	second_act_portmask;
	rt_wlan_mbssid_mask_t	second_act_wlanMbssidMask[RT_WLAN_DEVICE_MAX];
	unsigned char	second_act_ctagif;
	unsigned int	second_act_cvid;
	unsigned char	second_act_cpri;
	unsigned int floodFlowDelete;
} rt_igmp_multicast_group_cfg_t;


#define RT_MCCFG_MAX_NUM 16
#define EGRINFO_PID 0
#define EGRINFO_DEVIFIDX 1


typedef struct rt_wlan_devMbssid_s
{
	rt_wlan_index_t 		 wlanDevIdx;
	rt_wlan_mbssid_index_t	 wlanMbssid;
}rt_wlan_dev_mbssid_t;

typedef struct rt_igmp_groupToPortIdInfo_s
{
	unsigned char isWlan;		//0:reference to phyPortIdx 1:reference to wlanDevMbssid
	unsigned char resv0;		//just a reserved field
	unsigned char dmacTrans:1;
	unsigned char dmac[6];
	union {	
		rt_port_phy_port_index_t phyPortIdx;
		rt_wlan_dev_mbssid_t	 wlan;
	};
}rt_igmp_groupToPortIdInfo_t;

typedef struct rt_igmp_groupToDevInfo_s
{
	int				ifindex;		// (net_device)  -> ifindex
	unsigned char	resv0;			//just a reserved field
	unsigned char 	dmacTrans:1;	
	unsigned char 	dmac[6];
}rt_igmp_groupToDevInfo_t;

typedef struct rt_igmp_group_swIfidx_cfg_s
{
	unsigned char   groupBehavior;		// ref to rt_igmp_multicast_groupBehavior_t
	unsigned char	is_ipv6;	 		// 0: ipv4, 1: ipv6 
	unsigned char	careSourceAddress; 	// only valid when groupBehavior= RT_MC_BEHAVIOR_FLOW_FORWAED  0: do not care sourceAddress 1:should care sourceAddress
	unsigned char	careIngressSvid;	// only valid when groupBehavior= RT_MC_BEHAVIOR_FLOW_FORWAED  0: do not care Ingress Svid  1:Ingress Svid should Match
	unsigned char	careIngressCvid;	// only valid when groupBehavior= RT_MC_BEHAVIOR_FLOW_FORWAED  0: do not care Ingress Cvid  1:Ingress Cvid should Match
	union {
		unsigned int	ipv4;
	 	uint8   		ipv6[16];
	}group_addr;
	union {
		unsigned int	ipv4;
	 	uint8   		ipv6[16];
	}source_addr;
	unsigned char	ingress_stagif;
	unsigned int	ingress_svid;	
	unsigned char	ingress_ctagif;
	unsigned int	ingress_cvid;

	unsigned char egrInfoType;		// 0:portId mode(EGRINFO_PID)  1:devIfidx mode(EGRINFO_DEVIFIDX)
	unsigned int cntEgrInfo;	// count of egrInfo
	union {
		rt_igmp_groupToPortIdInfo_t	 portId[RT_MCCFG_MAX_NUM];
		rt_igmp_groupToDevInfo_t 	 devIfidx[RT_MCCFG_MAX_NUM];
	}egrInfo;
	unsigned char copy2cpu;

	unsigned int floodFlowDelete;	//for groupBehavior=RT_MC_CHECK_UNKNOWN_FLOOD_FLOW
	
} rt_igmp_group_devPort_cfg_t;


typedef struct rt_igmp_unknownMc_cfg_s{
	unsigned char isipv6;				// 0:IPv4  1:IPv6
	unsigned char trapPkt;				//default=[0:drop]  1:trap unknown multicast 2:forward
} rt_igmp_unknownMc_cfg_t;

typedef struct rtk_igmp_userGroup_cfg_s
{
	uint8 valid;
	uint8 isIpv6;
	int32 group[4];
}rtk_igmp_userGroup_cfg_t;

/*
 * Function Declaration
 */

/* Function Name:
 *      rt_igmp_multicastMode_set
 * Description:
 *      Set multicast learning Mode 
 * Input:
 *      mcMode        - Multicast mode
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 * 	 RT_ERR_DRIVER_NOT_FOUND    - Driver not found
 * 	 RT_ERR_NOT_ALLOWED 	    - Driver return fail
 * Note:
 *      The API can set multicast learning mode
 */
extern int32
rt_igmp_multicastMode_set (
    rt_igmp_multicast_learning_mode_t mcMode );

/* Function Name:
 *		rt_igmp_multicastGroupDev_set
 * Description:
 *		Set multicast groupDev.
 * Input:
 *		mcDevConfig		- Multicast groupDev
 * Output:
 *		None.
 * Return:
 *		RT_ERR_OK				- OK
 *	 RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *	 RT_ERR_NOT_ALLOWED 		- Driver return fail
 * Note:
 *		The API can set multicast groupDev
 */
extern int32
rt_igmp_multicastGroupDev_set (
	rt_igmp_group_devPort_cfg_t mcConfig );

/* Function Name:
 *		rt_igmp_multicastGroupDev_get
 * Description:
 *		Get multicast groupDev.
 * Input:
 *		mcDevConfig 	- Multicast groupDev
 * Output:
 *		mcDevConfig 	- Multicast groupDev
 * Return:
 *		RT_ERR_OK				- OK
 *	 RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *	 RT_ERR_NOT_ALLOWED 		- Driver return fail
 * Note:
 *		The API can get multicast groupDev
 */
extern int32
rt_igmp_multicastGroupDev_get (
	rt_igmp_group_devPort_cfg_t *mcConfig );


/* Function Name:
 *      rt_igmp_multicastGroup_set
 * Description:
 *      Set multicast group.
 * Input:
 *      mcConfig        - Multicast group
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 * 	 RT_ERR_DRIVER_NOT_FOUND    - Driver not found
 * 	 RT_ERR_NOT_ALLOWED 	    - Driver return fail
 * Note:
 *      The API can set multicast group
 */
extern int32
rt_igmp_multicastGroup_set (
    rt_igmp_multicast_group_cfg_t mcConfig );


/* Function Name:
 *		rt_igmp_unknownMcAction_set
 * Description:
 *		Set unknown Multicsaat default action
 * Input:
 *		rt_igmp_unknownMc_cfg_t 	- unknown 
multicast config
 * Output:
 *		None.
 * Return:
 *		RT_ERR_OK				- OK
 * Note:
 *		The API can set unknown multicast drop/trap
 */
extern int32
rt_igmp_unknownMcAction_set (
	rt_igmp_unknownMc_cfg_t mcUnknCfg );


/* Function Name:
 *              rt_igmp_unknownMcAction_set
 * Description:
 *              Check if Multicast flow is exsit ?
 * Input:
 *              rt_igmp_multicast_group_cfg_t        - Multicast config
 * Output:
 *              rt_igmp_multicast_group_cfg_t        - Multicast flow infomation
 * Return:
 *              RT_ERR_OK                            - OK
 *     		RT_ERR_DRIVER_NOT_FOUND              - Driver not found
 *              RT_ERR_NOT_ALLOWED                   - Driver return fail
 * Note:
 *              The API can check if flow is exist by multicast config.
 *             
 */
extern int32 
rt_igmp_multicastFlow_check(
	rt_igmp_multicast_group_cfg_t *pMcConfig );
/* Function Name:
 *		rt_igmp_userGroup_check
 * Description:
 *		check if user group address is vaild
 * Input:
 *		rt_igmp_unknownMc_cfg_t 	- user group infomation
 * Output:
 *		None.
 * Return:
 *		RT_ERR_OK				- OK
 * Note:
 *		The API can check if user group address is vaild
 */
int32
rt_igmp_userGroup_check (
	rtk_igmp_userGroup_cfg_t *userGroup );


#endif /* __RT_IGMP_EXT_H__ */
