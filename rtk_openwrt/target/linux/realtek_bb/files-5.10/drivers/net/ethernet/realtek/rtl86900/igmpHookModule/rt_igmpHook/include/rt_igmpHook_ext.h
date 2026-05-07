/*
 * Copyright (C) 2020 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 *
 * Purpose : Definition of IGMPHookModule extension API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of multicast whiteList
 *           (2) Configuration of igmp/mld device configuration
 *           (3) Configuration of trap to protocol-stack groups
 *           (4) Configuration of bridge device configuration
 *           (5) Configuration of weight table of group
 * 			 (6) Get statistic of multicast groups
 */



#ifndef _RT_IGMPHOOK_EXT_H
#define _RT_IGMPHOOK_EXT_H

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN  6
#endif

typedef enum rt_igmp_control_list_e
{
/* WARNING: don't move  start for diag using*/
	RT_DEV_INVAILD=0,					//illegal value
	RT_SNOOPING_DISABLE=1,				//0:enable snooping 1:disable snooping
	RT_FAST_LEAVE=2,					//0:disable fast leave 1:enable fast leave
	RT_MAX_GROUP_SIZE=3,				//max group of this device
	RT_MC_MEMBER_AGING_INTERVAL=4,		//group timeout. unit:second
	RT_CARE_SOURCE=5,					//0:don't multicast data source address(igmpv3 treat as igmpv2) 1: care mulitcast data source address(do igmpv3) 
	RT_RATE_LIMIT=6,					//rate limit igmp control packet. unit:packets per second
/* WARNING: don't move	End for diag using*/
	RT_MC_LAST_MEMBER_AGING_INTERVAL,	//last member angin interval. uint:second
	RT_FLUSH_MDB,						//1:flush multicast database by device
	RT_FLUSH_CONF,						//1:flush multicast deivce config to default
	RT_REPORT_DROP,						//1:drop all igmp report packets
	RT_DROP_IGMPV1,						//1:drop igmpv1 packets
	RT_DROP_IGMPV2,						//1:drop igmpv2 packets
	RT_DROP_IGMPV3,						//1:drop igmpv3 packets
	RT_IGNORE_IGMPV1,					//1:do not learning igmpv1 just accept this packet
	RT_IGNORE_IGMPV2,					//1:do not learning igmpv2 just accept this packet
	RT_IGNORE_IGMPV3,					//1:do not learning igmpv3 just accept this packet
	RT_DROP_MLDV1,						//1:drop mldv1 packets
	RT_DROP_MLDV2,						//1:drop mldv2 packets
	RT_IGNORE_MLDV1,					//1:do not learning mldv1 just accept this packet
	RT_IGNORE_MLDV2,					//1:do not learning mldv2 just accept this packet
	RT_MAX_WEIGHT,						//max weight of this device
	RT_DROP_OVER_WEIGHT,				//1:drop report packet if over weight
	RT_FORWARD_DISABLE,					//1:disable forward data to this dev
	RT_DMAC_MC_TO_UC,
//  RT_CONTROL_CONFIG_MAX should be last
	RT_IGMP_CONTROL_CONFIG_MAX,
}rt_igmpHook_control_list_t;

typedef enum rt_igmpHook_br_control_list_e
{	
/* WARNING: don't move	start for diag using*/
	RT_BR_INVAILD=0,					//illegal value
	RT_BR_MAX_GROUP_SIZE=1,				//max group of this bridge device (ipv4+ipv6)
	RT_BR_MAX_IP4_GROUP_SIZE=2,			//max group of this bridge device (ipv4)
	RT_BR_MAX_IP6_GROUP_SIZE=3,			//max group of this bridge device (ipv6)
	RT_BR_MAX_CLIENT_SIZE=4,			//max host count of this bridge device (ipv4)
	RT_BR_MAX_IP4_CLIENT_SIZE=5,		//max host count of this bridge device (ipv6)
	RT_BR_MAX_IP6_CLIENT_SIZE=6,
/* WARNING: don't move	End for diag using*/	
	RT_BR_PER_IP4_CLIENT_MAX_JOIN_GROUP,//max group join count per-host (ipv4)
	RT_BR_PER_IP6_CLIENT_MAX_JOIN_GROUP,//max group join count per-host (ipv6)
	RT_BR_SNOOPING_DISABLE,				//0:enable snooping 1:disable snooping
	RT_BR_FLUSH_CONF,					//1:flush multicast deivce config to default
	RT_BR_FLUSH_MDB,					//1:flush multicast IPv4/IPv6 database by device
	RT_BR_FLUSH_MDBv4,					//1:flush multicast IPv4 database by device
	RT_BR_FLUSH_MDBv6,					//1:flush multicast IPv6 database by device
	RT_BR_RATE_LIMIT,					//rate limit igmp control packet. unit:packets per second
	RT_BR_FAST_LEAVE,					//0:disable fast leave 1:enable fast leave
	RT_BR_DROP_IGMPV1,					//1:drop igmpv1 packets
	RT_BR_DROP_IGMPV2,					//1:drop igmpv2 packets
	RT_BR_DROP_IGMPV3,					//1:drop igmpv3 packets
	RT_BR_DROP_MLDV1,					//1:drop mldv1 packets
	RT_BR_DROP_MLDV2,					//1:drop mldv2 packets
	RT_BR_MAX_IP4_WEIGHT,				//max weight of this device(ipv4)
	RT_BR_MAX_IP6_WEIGHT,				//max weight of this device(ipv6)
	RT_BR_DROP_IP4_OVER_WEIGHT,			//1:drop report packet if over weight(ipv4)
	RT_BR_DROP_IP6_OVER_WEIGHT,			//1:drop report packet if over weight(ipv6)
	RT_BR_COMBINE_ID,					// 0:idisable  1-255 combine id
//	RT_BR_CONTROL_CONFIG_MAX should be last
	RT_BR_CONTROL_CONFIG_MAX,
}rt_igmpHook_br_control_list_t;


typedef struct rtk_igmpHook_whiteList_s
{
	unsigned char  smacChk:1;		//lan host smac
	unsigned char  sipChk:1;		//lan host sip
	unsigned char  gipChk:1;
	unsigned char  isIpv6:1;
	unsigned char  mcSipChk:1;		//multicast sip filter
	unsigned char  mcSipChkIgnoreNonIGMPv3MLDv2:1;	//only valid when mcSipChk=1 ignore igmpv1/igmpv2/mldv1 packet chcek
	unsigned char  smac[ETHER_ADDR_LEN];
	unsigned char  smacMask[ETHER_ADDR_LEN];

	unsigned int  wlSipStart[4];			//if v4 using host endina store in arry[0],if v6 using net endian store in array
	unsigned int  wlSipEnd[4];

	unsigned int  wlMcSip[4];
	unsigned int  wlMcSipEnd[4];

	unsigned int  wlGroupIpStart[4];
	unsigned int  wlGroupIpEnd[4];
}rt_igmpHook_whiteList_t;

typedef struct rtk_igmpHook_blackList_s
{
	unsigned char  smacChk:1;
	unsigned char  isIpv6:1;
	unsigned char  smac[ETHER_ADDR_LEN];
	unsigned char  smacMask[ETHER_ADDR_LEN];

}rt_igmpHook_blackList_t;


typedef struct rt_igmpHook_devInfo_s
{
	char devIfname[32];
	int devIfidx;
}rt_igmpHook_devInfo_t;


typedef struct rt_igmpHook_ignoreGroup_s
{
	unsigned char   isIpv6:1;
	/*
	// 0:snooping absolute ignore this group and send to PS only 
	// 1:do snooping to dev ports and send a copy to PS
	*/	
	unsigned char   isCopy2cpu:1;
	unsigned int	ignGroupIpStart[4];
	unsigned int	ignGroupIpEnd[4];
}rt_igmpHook_ignoreGroup_t;


typedef struct rt_igmpHook_groupWeight_s
{
    unsigned char	isIpv6:1;
    unsigned int	weightGroupIpStart[4];
    unsigned int	weightGroupIpEnd[4];
    unsigned int	weight;
}rt_igmpHook_groupWeight_t;

typedef struct rt_igmpHook_devStatistic_s
{
    unsigned int    curWeight;			//total weight of this device group 
    unsigned int    joinCnt;			//total of IGMP join receive
    unsigned int    bandwidthExceedCnt;	//exceed bandwidth and count IGMP join
    unsigned int    activeGroupCnt;		//total of active group counter
}rt_igmpHook_devStatistic_t;


typedef struct rt_igmpHook_groupStatistic_s
{
	unsigned int	groupAddr[4];
	unsigned int	sourceAddr[4];
	short			vlanId;
	short			svlanId;
	unsigned int	curClient[4];
	unsigned int	curClientExistTimesSec;
}rt_igmpHook_groupStatistic_t;


/*
 * Function Declaration
 */



/* Function Name:
 *      rt_igmpHook_whiteList_add
 * Description:
 *      add igmp white list
 * Input:
 *      patten					- white list pattens
 *		ifidx					- device name or device index
 * Output:
 *      index					- white list entry inedx
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 * Note:
 *      The API can add white list by device
 */
extern int 
rt_igmpHook_whiteList_add( 
	rt_igmpHook_whiteList_t *patten,rt_igmpHook_devInfo_t ifidx , int *index);

/* Function Name:
 *		rt_igmpHook_whiteList_del
 * Description:
 *		delete igmp white list
 * Input:
 *      patten					- white list pattens
 *		ifidx					- device name or device index
 *      index					- white list entry inedx
 * Output:
 *		None.
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_FAILED			- Failed
 * Note:
 *      The API can delete white list by device
 */
extern int 
rt_igmpHook_whiteList_del(
	rt_igmpHook_whiteList_t *patten,rt_igmpHook_devInfo_t ifidx,int index);


/* Function Name:
 *		rt_igmpHook_devConfig_set
 * Description:
 *		set igmp/mld device configuration
 * Input:
 *		devInfo					- device name or device index
 *		isIpv6					- Ipv4 or Ipv6
 *		type					- configuration type
 *		value					- configuration value
 * Output:
 *		None.
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_FAILED			- Failed
 * Note:
 *		The API can set igmp/mld device configuration 
 */
extern int
rt_igmpHook_devConfig_set(
	rt_igmpHook_devInfo_t devInfo , unsigned int isIpv6 ,rt_igmpHook_control_list_t type , unsigned int value);

/* Function Name:
 *		rt_igmpHook_devConfig_get
 * Description:
 *		get igmp/mld device configuration
 * Input:
 *		devInfo					 - device name or device index
 *		isIpv6 					 - Ipv4 or Ipv6
 *		type					 - configuration type
 * Output:
 *		value					 - configuration value
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_FAILED			- Failed
 * Note:
 *		The API can get igmp/mld device configuration 
 */
extern int
rt_igmpHook_devConfig_get(
	rt_igmpHook_devInfo_t devInfo , unsigned int isIpv6 ,rt_igmpHook_control_list_t type , unsigned int* value);

/* Function Name:
 *		rt_igmpHook_brDevConfig_set
 * Description:
 *		set bridge device configuration
 * Input:
 *		devInfo					- device name or device index
 *		type 					- configuration type
 *		value					- configuration value
 * Output:
 *		None.
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_FAILED			- Failed
 * Note:
 *		The API can set bridge device configuration 
 */
extern int
rt_igmpHook_brDevConfig_set(
	rt_igmpHook_devInfo_t devInfo ,rt_igmpHook_br_control_list_t type , unsigned int value);

/* Function Name:
 *		rt_igmpHook_brDevConfig_get
 * Description:
 *		get bridge device configuration
 * Input:
 *		devInfo					- device name or device index
 *		type 					- configuration type
 * Output:
 *		value					- configuration value
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_FAILED			- Failed
 * Note:
 *		The API can get bridge device configuration 
 */
extern int 
rt_igmpHook_brDevConfig_get(
	rt_igmpHook_devInfo_t devInfo ,rt_igmpHook_br_control_list_t type , unsigned int* value);

/* Function Name:
 *		rt_igmpHook_groupToPsTbl_add
 * Description:
 *		add trap group to protocol-stack table
 * Input:
 *		patten					- trap group range
 * Output:
 *		index					- trap group table index
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_FAILED			- Failed
 * Note:
 *		The API can add group to trap to protocol-stack table
 */
extern int 
rt_igmpHook_groupToPsTbl_add(
	rt_igmpHook_ignoreGroup_t* patten,unsigned int *index);

/* Function Name:
 *		rt_igmpHook_groupToPsTbl_del
 * Description:
 *		delete trap group to protocol-stack table
 * Input:
 *		patten					- trap group range
 *		index					- trap group table index
 * Output:
 *		None.
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_FAILED			- Failed
 * Note:
 *		The API can delete group to trap to protocol-stack table
 */
extern int 
rt_igmpHook_groupToPsTbl_del(
	rt_igmpHook_ignoreGroup_t* patten,unsigned int index);

/* Function Name:
 *		rt_igmpHook_nextValidGroupStatistic_get
 * Description:
 *		get next valid group statistic
 * Input:
 *		devInfo					 - device name or device index
 *		getIdx					 - start form zero. Get next valid entry inedx refill to getIdx if return value < 0 indicate more entry
 *		isIpv6 					 - Ipv4 or Ipv6
 * Output:
 *		getIdx					 - start form zero. Get next valid inedx entry refill to getIdx if return value < 0 indicate more entry
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_FAILED			- Failed no more entry 
 * Note:
 *		The API can get next valid group statistic
 */
extern int 
rt_igmpHook_nextValidGroupStatistic_get(
	rt_igmpHook_devInfo_t devInfo,int *getIdx,unsigned int isIpv6,rt_igmpHook_groupStatistic_t *statisticInfo);

/* Function Name:
 *		rt_igmpHook_devStatistic_get
 * Description:
 *		get device statistic 
 * Input:
 *		devInfo					 - device name or device index
 *		isIpv6 					 - Ipv4 or Ipv6
 * Output:
 *		statisticInfo			 - device staustic information
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_FAILED			- Failed
 * Note:
 *		The API can get device statistic 
 */
extern int 
rt_igmpHook_devStatistic_get(
	rt_igmpHook_devInfo_t devInfo,unsigned int isIpv6,rt_igmpHook_devStatistic_t *statisticInfo);

/* Function Name:
 *		rt_igmpHook_groupWeight_add
 * Description:
 *		add group weight table
 * Input:
 *		groupWeightCfg			 - group weight configuartion
 * Output:
 *		index					 - groupWeight table entry inedx
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_FAILED			- Failed
 * Note:
 *		The API can add group weight table
 */
extern int 
rt_igmpHook_groupWeight_add(
	rt_igmpHook_devInfo_t devInfo,rt_igmpHook_groupWeight_t* groupWeightCfg, int *index);

/* Function Name:
 *		rt_igmpHook_groupWeight_del
 * Description:
 *		delete group weight table
 * Input:
 *		groupWeightCfg			 - group weight configuartion
 *		index					 - groupWeight table entry inedx 
 * Output:
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_FAILED			- Failed
 * Note:
 *		The API can delete group weight table
 */
extern int 
	rt_igmpHook_groupWeight_del(
rt_igmpHook_devInfo_t devInfo,rt_igmpHook_groupWeight_t* groupWeightCfg, int  index);



#endif

