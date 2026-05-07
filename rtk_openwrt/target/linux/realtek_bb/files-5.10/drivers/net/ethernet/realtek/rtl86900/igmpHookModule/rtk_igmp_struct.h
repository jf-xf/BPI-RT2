#ifndef _RTK_IGMP_STRUCT_H
#define _RTK_IGMP_STRUCT_H

#include <common/rt_type.h>
//#include <common/type.h>
#include <linux/version.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
//#include <linux/if_vlan.h>
//#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
//#include <linux/tcp.h>
#include <linux/udp.h>
#include <uapi/linux/netfilter_bridge.h>
//#include <uapi/linux/netfilter_arp.h>
#include <linux/netfilter_bridge.h>
//#include <linux/inet.h>
#include <rt_igmpHook_ext.h>

#define IGMP_MAX_BR_MODULE_NUM 		8

#define IGMP_MAX_WAN_DEV_NUM 		32
#define IGMP_MAX_LAN_DEV_NUM 		128
#define IGMP_MAX_EXTDEV_PORT_NUM 	5
#define IGMP_MAX_EXTDEV_MAC_NUM 	8		//Max McToUc Cnt for a dev

#ifdef CONFIG_RTK_SOC_RTL8198D
#define IGMP_MAX_DEV_NUM 			32		
#else
#define IGMP_SUPPORT_MCTOUC	1
#define IGMP_MAX_DEV_NUM 			12		//Max forward dev cnt (4Lan+2wifi)
#endif

/* common */
#ifndef NULL
#define NULL  ((void *) 0)
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAILED
#define FAILED	-1
#endif

#ifndef FAIL
#define FAIL -1
#endif

#if 0
#ifndef ENABLE
#define ENABLE 1
#endif

#ifndef DISABLE
#define DISABLE 0
#endif
#endif


#define rtlglue_printf printk
#ifndef bzero
#define bzero(p,s) memset(p,0,s)
#endif

int igmp_assert_eq(int func_return,int expect_return,const char* func,int line);

#ifndef ASSERT_EQ
#define ASSERT_EQ(value1, value2) \
do { \
	int func_return=(value1); \
	if(igmp_assert_eq((func_return), (value2), __FUNCTION__,__LINE__)!=0) \
		return func_return; \
}while(0)
#endif


typedef enum rtk_igmp_debug_level_e
{
	RTK_IGMP_DEBUG_LEVEL_WARNING=			0x001,
	RTK_IGMP_DEBUG_LEVEL_HOOK_POINT=		0x002,
	RTK_IGMP_DEBUG_LEVEL_PARSER=			0x004,
	RTK_IGMP_DEBUG_LEVEL_IGMPCTRL=			0x008,
	RTK_IGMP_DEBUG_LEVEL_FLOWDATA=			0x010,
	RTK_IGMP_DEBUG_LEVEL_HWCB=				0x020,
	RTK_IGMP_DEBUG_LEVEL_DUMP=				0x040,
	RTK_IGMP_DEBUG_LEVEL_RATE_LIMIT_DROP=	0x080,
	RTK_IGMP_DEBUG_LEVEL_API=				0x100
} rtk_igmp_debug_level_t;


#define NF_RETURN	 0xfd
#define NF_CONTINUE  0xfe

typedef enum rtk_igmp_nf_ret_e
{
	RTK_IGMP_NF_DROP	=	NF_DROP,
	RTK_IGMP_NF_ACCEPT	=	NF_ACCEPT,
	RTK_IGMP_NF_CONTINUE=	NF_CONTINUE,	//continue to learning igmp packet
	RTK_IGMP_NF_RETURN	=	NF_RETURN,		//break this record and continue to next (for IGMPv3 multi-record)
}rtk_igmp_nf_ret_e_t;



#define IGMP_ERR_START 0x10000
/* ERROR NO =============================================================== */
typedef enum rtk_igmp_enum_err_e
{
	RTK_IGMP_ERR_OK=0,
	RTK_IGMP_ERR_FAILED = (IGMP_ERR_START+1), 									//0x10001
	RTK_IGMP_TABLE_FULL,
	RTK_IGMP_ENTRY_NOT_FOUND,
}rtk_igmp_enum_err_t;

#define RATE_LIMIT_CYCLE_DEFINE_SECONDS 1
typedef struct rate_limit_s
{
	uint32  rate; /*rate limit: Accept "rate" packets per time cycle*/
	uint32  counter; /*How many packes has been recevied in current time cycle.*/
	unsigned long cycle_start_time;
}rate_limit_t;

typedef enum rtk_rate_limit_ret_e
{
	RTK_RATE_LIMIT_ACCEPT_PACKET=0,
	RTK_RATE_LIMIT_DROP_PACKET
}rtk_rate_limit_ret_t;

typedef struct rtk_igmpv4_control_s
{
	/* should add mapping rt_igmpHook_control_list_t*/
	uint32 	igmpVerIgr_v1:1;
	uint32 	igmpVerIgr_v2:1;
	uint32 	igmpVerIgr_v3:1;
	uint32 	igmpVerDrp_v1:1;
	uint32 	igmpVerDrp_v2:1;
	uint32 	igmpVerDrp_v3:1;
	uint32	igmpReportDrop:1;
	uint32 	igmpSnoopingDisable:1;
	uint32 igmpForwardDisable:1;		//when Disable Snooping or unknown Flooding force disable forward 
	uint32 	igmpfastLeave:1;			//for igmpv2
	uint32 	igmpv3CareSource:1;
	uint32	igmpOverWeightDrop:1;
	uint32	igmpDmacMcToUc:1;
	uint16 	igmpMaxGroupSize:10;		//0:disable
	uint16	igmpMcMemberAgingInterval;		//groupAgingTime linux -> multicast_membership_interval
	uint16	igmpMcLastMemberAgingInterval;	//group short timeout linux -> multicast_last_member_interval
	uint32	igmpMaxWeight;
	struct  list_head white;				//point to a whiteListTblIdx(unsigned int) list(arranged by idx) and reference to whiteListHead
	struct  list_head black;
	rate_limit_t  ratelimit;
}rtk_igmpv4_control_t;


typedef struct rtk_igmpv6_control_s
{
	/* should add mapping rt_igmpHook_control_list_t*/
	uint32	igmp6VerIgr_v1:1;
	uint32	igmp6VerIgr_v2:1;
	uint32 	igmp6VerDrp_v1:1;
	uint32 	igmp6VerDrp_v2:1;
	uint32	igmp6ReportDrop:1;
	uint32 	igmp6SnoopingDisable:1;
	uint32 igmp6ForwardDisable:1;		//when Disable Snooping or unknown Flooding force disable forward 
	uint32 	igmp6fastLeave:1;			//for mldv1
	uint32 	mldv2CareSource:1;
	uint32	igmp6OverWeightDrop:1;
	uint32	igmp6DmacMcToUc:1;
	uint16 	igmp6MaxGroupSize:10;		//0:disable
	uint16	igmp6McMemberAgingInterval;		//groupAgingTime linux -> multicast_membership_interval(26000ms)
	uint16	igmp6McLastMemberAgingInterval;	//group short timeout linux -> multicast_last_member_interval
	uint32	igmp6MaxWeight;
	struct  list_head white;				//point to a whiteListTblIdx(unsigned int) list(arranged by idx) and reference to whiteListHead
	struct  list_head black;
	rate_limit_t  ratelimit;
}rtk_igmpv6_control_t;



typedef struct rtk_igmp_groupCbEvt_s
{
	int32 isIPv6;
	int32 vlan;
	int32 svlan;
	int32 asunknownNotFlush:1;
	uint32 group[4];
}rtk_igmp_groupCbEvt_t;

/* for extranal switch in same switch we need record extra-sub-port id*/
typedef struct rtk_igmp_devExtraPorts_s
{
	uint8  valid:1;
	uint8  devExtraPortId:7;
}rtk_igmp_devExtraPorts_t;

typedef struct rtk_igmp_devExtraTransMac_s
{
	uint8 valid:1;
	uint8 dmac[6];
}rtk_igmp_devExtraTransMac_t;


typedef struct rtk_igmp_flowToDevInfo_s
{
	uint16  valid:1;
	uint16  devIfIdx;
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE) && defined(IGMP_SUPPORT_MCTOUC)	
	rtk_igmp_devExtraTransMac_t subDmac[IGMP_MAX_EXTDEV_MAC_NUM];
#endif
#if defined(CONFIG_RTL8367_SUPPORT) && defined(CONFIG_RTL9607C_SERIES) && defined(CONFIG_RTK_SKB_MARK2)	
	rtk_igmp_devExtraPorts_t subPorts[IGMP_MAX_EXTDEV_PORT_NUM];
#endif
}rtk_igmp_flowToDevInfo_t;

typedef struct rtk_igmp_flowCbEvt_s
{
	uint32 group[4];
	uint32 sourceAddr[4];;
	int16 ingressCvlan;		//-1 untag
	int16 ingressSvlan;		//-1 untag	
	int32 isIPv6:1;
	int32 floodflowdelete:1;
	int32 copy2cpu:1;
	rtk_igmp_flowToDevInfo_t egressDevInfo[IGMP_MAX_DEV_NUM];
}rtk_igmp_flowCbEvt_t;

typedef enum rtk_igmp_unknownCtrl_s
{
	UNKNOWN_MC_DROP=0,
	UNKNOWN_MC_TRAP,
}rtk_igmp_unknownMcCtrl_t;


typedef struct rtk_igmp_unknownMcCbEvt_s
{
	uint32 isIpv6;
	rtk_igmp_unknownMcCtrl_t unknownAct;
}rtk_igmp_unknownMcCbEvt_t;


typedef struct rtk_igmp_hwCbEvt_s
{
	int (*hwRegInit)(void);
	int (*hwUnRegClear)(void);
	int (*hwUnknownIpMcAction) (rtk_igmp_unknownMcCbEvt_t *);
	int (*groupAddCbEvt)(rtk_igmp_groupCbEvt_t *groupCb);
	int (*groupDelCbEvt)(rtk_igmp_groupCbEvt_t *groupCb);
	int (*flowSetCbEvt)(rtk_igmp_flowCbEvt_t *flowCb);
	int (*flowDelCbEvt)(rtk_igmp_flowCbEvt_t *flowCb);
	int (*flowUpdateCbEvt)(rtk_igmp_flowCbEvt_t *flowCb);
}rtk_igmp_hwCbEvt_t;

typedef struct rtk_igmp_brConfdb_s
{
	//uint16 igmp6GenQuerySec;
	//uint16 igmpGenQuerySec;
	uint16 sysMaxGroupSize;			//0:disable
	uint16 sysIp4MaxGroupSize;		//0:disable
	uint16 sysIp6MaxGroupSize;		//0:disable

	uint16 sysMaxClientSize;		//0:disable
	uint16 sysIp4MaxClientSize;		//0:disable
	uint16 sysIp6MaxClientSize;		//0:disable

	uint16 perIp4ClientMaxJoinGroupSize;//0:disable
	uint16 perIp6ClientMaxJoinGroupSize;//0:disable

	uint32 sysIp4MaxWeight;		//0:disable
	uint32 sysIp6MaxWeight;		//0:disable

	uint32	sysIp4OverWeightDrop:1;
	uint32	sysIp6OverWeightDrop:1;

	uint8	combineId;	// combineId to other br.x   0:idisable  1-255 combine id
	//whiteList* ,blockList* System linkList

/*
	TODO List: kernel
	cat	sys/devices/virtual/net/br0/bridge/
	multicast_last_member_count (2)
	multicast_query_response_interval (1000ms)
	multicast_last_member_interval (100ms)
	multicast_query_use_ifaddr
	(v)multicast_membership_interval(26000ms)
	multicast_router(1 ?)
	multicast_querier(0 ?)
	(v)multicast_snooping (enable/disable)
	multicast_querier_interval(25500ms)
	multicast_startup_query_count(2)
	multicast_query_interval(12500ms)
	multicast_startup_query_interval(3125)

# cat sys/devices/virtual/net/nas0_0/brport/
	(v)multicast_fast_leave
	multicast_router

*/

}rtk_igmp_brConfdb_t;

typedef struct rtk_igmp_statistic_s
{

	uint32 igmp4JoinCnt;
	uint32 igmp6JoinCnt;

	uint32 igmp4WeightExceedCnt;
	uint32 igmp6WeightExceedCnt;

	
}rtk_igmp_statistic_t;


typedef struct rtk_igmp_globalConfdb_s
{
	uint32 igmp_sys_timer_sec;

}rtk_igmp_globalConfdb_t;

typedef struct rtk_igmp_DevConfdb_s
{
	rtk_igmpv4_control_t igmp;
	rtk_igmpv6_control_t igmp6;
}rtk_igmp_DevConfdb_t;

typedef struct rtk_igmp_multicastDeviceInfo_s
{
	char devName[32];
	struct net_device *dev;
	rtk_igmp_DevConfdb_t devConf;
	rtk_igmp_statistic_t devStatistic;
	// uint8 forceDevCTag:1;
	// uint8 forceDevCTagif:1;
	// int16 forceDevCvid;
	uint16 ifindex;
	/*gateway infomation*/
	uint8 rtk_igmp_gatewayMac[6];
	//uint32 rtk_igmp_gatewayIpv4Addr;
	//uint32 rtk_igmp_gatewayIpv6Addr[4];
	int16 bind2BrModuleIdx; //for wan dev only

}rtk_igmp_multicastDeviceInfo_t;


typedef struct rtk_igmp_pktHdr_s
{
	struct ethhdr *ethHdr;
	//struct vlan_hdr *cvh;
	struct ipv6hdr *ip6h;
	struct iphdr * iph;
	struct igmphdr * igmph;
	struct icmp6hdr *icmp6h;
	struct udphdr *udph;
	struct sk_buff *skb;
	uint8 smac[6];
	uint8 isDipMc:1;
	uint8 ingressCtagif:1;
	uint8 ingressStagif:1;	
	uint8 isFragmentPkt:1;
	uint8 isCopyToCpu:1;
	uint8 DevExtPortEn:1;
	uint8 DevExtPortId:7;	
	int16  ingressCvid;
	int16  ingressSvid;
	uint16 l4proto;
	int32 devIfindex;
	uint32 priflag;
	rtk_igmp_multicastDeviceInfo_t *devInfo;
	int32 moduleIndex;
} rtk_igmp_pktHdr_t;


typedef enum rtk_igmp_hwCbCtrlMode_e
{
	RTK_IGMP_CONFIG_BY_IFIDX 		= 0,
	RTK_IGMP_CONFIG_BY_PORT			= 1,
	RTK_IGMP_CONFIG_BY_PORTMASK 	= 2,
	RTK_IGMP_CONFIG_MAX,
}rtk_igmp_hwCbCtrlMode_t;


typedef struct rtk_igmp_Sysdb_s
{
	rtk_igmp_debug_level_t igmp_debug_level;
	spinlock_t lock_igmp;
	spinlock_t lock_callBack;
	rtk_igmp_hwCbCtrlMode_t hwCbMode;
	uint32 disHook:1;
	uint32 disHwctrl:1;
	uint32 regMdbCb:1;
	uint32 procConfigShow:1;
	uint32 mc2ucCopyByIGMP:1;
	
	int32 unknownFlood;
	int32 trapWlanToPS;
#if defined(CONFIG_RTL8367_SUPPORT) && defined(CONFIG_RTL9607C_SERIES) && defined(CONFIG_RTK_SKB_MARK2)
	int8 skbMark2_ExtPortEnStartbit;
	int8 skbMark2_ExtPortIDStartbit;
	int8 skbMark2_ExtPortIDLen;
#endif
	rtk_igmp_flowCbEvt_t flowCb;

}rtk_igmp_Sysdb_t;


typedef struct rtk_igmp_whiteListEntry_s
{
	uint32 whiteListIdx;
	uint16  whiteRefCnt;
	rt_igmpHook_whiteList_t patten;
	struct list_head entry;
}rtk_igmp_whiteListEntry_t;

typedef struct rtk_igmp_idxEntry_s
{
	uint32 entryIdx;
	struct list_head entry;
}rtk_igmp_idxEntry_t;


typedef struct rtk_igmp_blackListEntry_s
{
	uint32 blackListIdx;
	uint16  blackRefCnt;
	rt_igmpHook_blackList_t patten;
	struct list_head entry;
}rtk_igmp_blackListEntry_t;



typedef struct rtk_igmp_userResvGroup_s
{
	uint16  valid:1;
	uint16 cbKnownGrpToHw:1;
	uint16 resvGrpListIdx:14;
	rt_igmpHook_ignoreGroup_t patten;
	struct list_head entry;
}rtk_igmp_userResvGroup_t;

typedef struct rtk_igmp_groupWeight_s
{
	uint16  valid:1;
	uint16 groupWeightListIdx:15;
	uint16 devIdx;
	rt_igmpHook_groupWeight_t patten;
	struct list_head entry;
}rtk_igmp_groupWeight_t;



#endif
