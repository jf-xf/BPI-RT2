#ifndef _RTK_IGMP_SNOOPING_H
#define _RTK_IGMP_SNOOPING_H

#include <rtk_igmp_struct.h>

/*
	br->hash_elasticity = 4;
	br->hash_max = 512;

	br->multicast_router = 1;
	br->multicast_querier = 0;
	br->multicast_query_use_ifaddr = 0;
	br->multicast_last_member_count = 2;
	br->multicast_startup_query_count = 2;

	br->multicast_last_member_interval = HZ;
	br->multicast_query_response_interval = 10 * HZ;
	br->multicast_startup_query_interval = 125 * HZ / 4;
	br->multicast_query_interval = 125 * HZ;
	br->multicast_querier_interval = 255 * HZ;
	br->multicast_membership_interval = 260 * HZ;

	br->ip4_other_query.delay_time = 0;
	br->ip4_querier.port = NULL;
#if IS_ENABLED(CONFIG_IPV6)
	br->ip6_other_query.delay_time = 0;
	br->ip6_querier.port = NULL;
#endif
*/

//default timer serc
#define V6QUERYSEC	125
#define V4QUERYSEC	125
#define IGMP_MEMBER_AGING  	260
#define IGMP6_MEMBER_AGING	260
#define IGMP_LAST_MEMBER_AGING  	10
#define IGMP6_LAST_MEMBER_AGING		10

#define IGMP_EXPIRE_TIMER	5

#define IP_VERSION4 4
#define IP_VERSION6 6
#define BOTH_IPV4_IPV6 0x46

/*            IGMP type              */
#define IGMP_QUERY	0x11
#define IGMPV1_REPORT	0x12
#define IGMPV2_REPORT	0x16
#define IGMPV2_LEAVE	0x17
#define IGMPV3_REPORT	0x22

/*             MLD type          */
#define MLD_QUERY	130
#define MLDV1_REPORT	131
#define MLDV1_DONE	132
#define MLDV2_REPORT	143
#define S_FLAG_MASK	0x08

#define MODE_IS_INCLUDE		1
#define MODE_IS_EXCLUDE		2
#define CHANGE_TO_INCLUDE_MODE	3
#define CHANGE_TO_EXCLUDE_MODE	4
#define ALLOW_NEW_SOURCES       5
#define BLOCK_OLD_SOURCES	6
#define IPV6ADDRLEN 16


#define IS_CLASSD_ADDR(ipv4addr)				((((uint32)(ipv4addr)) & 0xf0000000) == 0xe0000000)
#define RESERVE_MULTICAST_239_255_255_250 	0xEFFFFFFA	//239.255.255.250
#define RESERVE_MULTICAST_224_0_0_0 		0xE0000000	//224.0.0.0		 Base Address (Reserved)
#define RESERVE_MULTICAST_224_0_0_1			0xE0000001	//224.0.0.1		 All Systems on this Subnet
#define RESERVE_MULTICAST_224_0_0_2			0xE0000002	//224.0.0.2		 All Routers on this Subnet   
#define RESERVE_MULTICAST_224_0_0_22		0xE0000016	//224.0.0.22	 IGMPv3
// ipv6 all nodes + Solicited-Node
#define RESERVE_MULTICAST_FF02_00_00 "ff02::"
#define RESERVE_MULTICAST_FF02_02_FF "ff02::2:ffff:ffff"



#define IN_MULTICAST_RESV1(addr)	((((uint32)(addr)) & 0xFFFFFF00) == 0xe0000000)	// 224.0.0.x
#define IN_MULTICAST_RESV2(addr)	((((uint32)(addr)) & 0xFF000000) == 0xEF000000)	// 239.0.0.0~239.255.255.255
#define IN_MULTICAST_RESV3(addr)	((((uint32)(addr)) & 0xFFFFFF00) == 0xe0000100)	// 224.0.1.x

#define IN_MULTICAST_SSDP(addr)	((((uint32)(addr)) & 0xFFFFFFFF) == RESERVE_MULTICAST_239_255_255_250)	
#define IN_MULTICAST_BASE_RESV(addr)	((((uint32)(addr)) & 0xFFFFFFFF) == RESERVE_MULTICAST_224_0_0_0)	
#define IN_MULTICAST_ALL_SYSTEM(addr)	((((uint32)(addr)) & 0xFFFFFFFF) == RESERVE_MULTICAST_224_0_0_1)	
#define IN_MULTICAST_ALL_ROUTE(addr)	((((uint32)(addr)) & 0xFFFFFFFF) == RESERVE_MULTICAST_224_0_0_2)
#define IN_MULTICAST_IGMPv3(addr)	((((uint32)(addr)) & 0xFFFFFFFF) == RESERVE_MULTICAST_224_0_0_22)


#define IN_MULTICAST6_RESV1_FFO1(addr)   	 ((ntohl((uint32)(addr)) & 0xFFFF0000) == 0xFF010000)
#define IN_MULTICAST6_RESV1_FFO2(addr)   	 ((ntohl((uint32)(addr)) & 0xFFFF0000) == 0xFF020000)
#define IN_MULTICAST6_RESV1_FFO5(addr)  	 ((ntohl((uint32)(addr)) & 0xFFFF0000) == 0xFF050000)
#define IN_MULTICAST6_RESV1_FFO8(addr)   	 ((ntohl((uint32)(addr)) & 0xFFFF0000) == 0xFF080000)
#define IN_MULTICAST6_RESV1_FFOE(addr)  	 ((ntohl((uint32)(addr)) & 0xFFFF0000) == 0xFF0E0000)
#define IN_MULTICAST6_RESV1_SOLICITED_NODE(addr)    ((ntohl((uint32)(addr)) & 0xFFFF0000) == 0xFF000000)
#define IN_MULTICAST6_DATA_NOT_FFXE(addr)  	 ((ntohl((uint32)(addr)) & 0xFF0F0000) != 0xFF0E0000)
#define IN_MULTICAST6_RESV1_FFOX(addr)   	 ((ntohl((uint32)(addr)) & 0xFFF00000) == 0xFF000000)


#define IN_MULTICAST_RESV_SPECIAL_IPV4(addr)	(IN_MULTICAST_SSDP(addr) ||IN_MULTICAST_BASE_RESV(addr) || IN_MULTICAST_ALL_SYSTEM(addr) || IN_MULTICAST_ALL_ROUTE(addr) || IN_MULTICAST_IGMPv3(addr) )
#define IN_MULTICAST_RESV_IPV4_ALL(addr)	(IN_MULTICAST_RESV1(addr) ||IN_MULTICAST_RESV3(addr) || IN_MULTICAST_SSDP(addr) )
#define IN_MULTICAST_RESV_IPV6_ALL(addr)	(IN_MULTICAST6_RESV1_FFOX(addr))




/* IGMP version */
typedef enum {
	IGMP_V1=0,
	IGMP_V2,	//1
	IGMP_V3,	//2
	MLD_V1,		//3
	MLD_V2,		//4
	IGMP_MLD_VER_MAX//5
} IGMP_MLD_VER_t;

typedef enum rtk_igmp_ipv4MulticastFlowMode_e
{
	RTK_IGMP_IPV4MC_DONT_CARE_SRC=0,
	RTK_IGMP_IPV4MC_INCLUDE,
	RTK_IGMP_IPV4MC_EXCLUDE,
} rtk_igmp_ipv4MulticastFlowMode_t;


typedef struct rtk_igmp_multicastEventContext_s
{
	char devName[16];
	int32 moduleIndex;
	uint32 ipVersion;
	//uint16 vlanId;
	//uint16 svlanId;
	uint32 groupAddr[4];
	uint32 sourceAddr[4];
	uint32 portMask;
}rtk_igmp_multicastEventContext_t;


struct rtk_igmp_clientEntry
{
	uint8 igmpVersion:3;			// IGMP_MLD_VER_t
	int16 inIfidx;
	struct rtk_igmp_clientEntry *previous;
	struct rtk_igmp_clientEntry *next;	/*Pointer of next group entry*/
	uint32 clientAddr[4];		/*client ip address*/
	uint8  clientMacAddr[6];
	struct rtk_igmp_sourceEntry *sourceList;	/*this is the server source ip list*/
	uint32 groupFilterTimer;
	uint8 reportCtagif:1;
	int16 reportVlanId:13;
	uint8 reportStagif:1;
	int16 reportSVlanId:13;	
	rtk_igmp_devExtraPorts_t extraPortInfo;
	unsigned long joinSystemSecTime;	//store first join sec in system times
};


struct rtk_igmp_groupEntry
{
	struct rtk_igmp_groupEntry *previous;
	struct rtk_igmp_groupEntry *next;             /*Pointer of next group entry*/
	uint16 ipVersion:3;				// IP_VERSION4/IP_VERSION6
	int16 vlanId:13;				//noused now
	int16 svlanId:13;				//noused now
	uint32 groupAddr[4];
	int16  groupIdx;
	struct rtk_igmp_clientEntry *clientList;
};

struct rtk_igmp_sourceEntry
{
	uint32 sourceAddr[4];		/*D class IP multicast address*/
	uint32 fwdState;
	struct rtk_igmp_sourceEntry *previous;
	struct rtk_igmp_sourceEntry *next;	/*Pointer of next group entry*/
	uint32 portTimer;
	uint32 setOpFlag;
};



struct ipv4Pkt
{
	uint8	vhl;		/* version + Header Length */
	uint8	typeOfService;
	uint16	length;		/* total length */

	uint16	identification;	/* identification */
	uint16	offset;

	uint8	ttl;		/* time to live */
	uint8	protocol;
	uint16	checksum;	/* Header Checksum */
	uint32	sourceIp;
	uint32	destinationIp;
};

struct igmpPkt
{
	uint8 type;		/* type*/
	uint8 maxRespTime;	/*maximum response time,unit:0.1 seconds*/
	uint16 checksum;
	uint32 groupAddr;
};

struct igmpv1Pkt
{
	uint8 VersionType;	/*4bits: version, 4bits:type*/
	uint8 unused;
	uint16 checkSum;	/*IGMP packet checksum*/
	uint32 groupAddr;	/*multicast group address*/
};

struct igmpv2Pkt
{
	uint8 type;		/* type*/
	uint8 maxRespTime;	/*maximum response time,unit:0.1 seconds*/
	uint16 checkSum;
	uint32 groupAddr;
};

struct igmpv3Query
{
	uint8 type;		/*query type*/
	uint8 maxRespCode;	/*maximum response code*/
	uint16 checkSum;	/*IGMP checksum*/
	uint32 groupAddr;	/*multicast group address*/
	uint8 rsq;		/* 4bit: reserved, 1bit: suppress router-side processing, 3bit: querier's robustness variable*/
	uint8 qqic;		/* querier's query interval code */
	uint16 numOfSrc;	/* number of sources */
	uint32 srcList;		/* first entry of source list */
};

struct groupRecord
{
	uint8	type;		/* Record Type */
	uint8	auxLen;		/* auxiliary data length, in uints of 32 bit words*/
	uint16	numOfSrc;	/* number of sources */
	uint32	groupAddr;	/* group address being reported */
	uint32	srcList;	/* first entry of source list */
};

struct igmpv3Report
{
	uint8	type;		/* Report Type */
	uint8	reserved1;
	uint16	checkSum;	/*IGMP checksum*/
	uint16	reserved2;
	uint16	numOfRecords;	/* number of Group records */
	struct	groupRecord recordList;/*first entry of group record list*/
};

struct ipv6Pkt
{
	uint32  vtf;		/*version(4bits),  traffic class(8bits), and flow label(20bits)*/

	uint16	payloadLenth;	/* payload length */
	uint8	nextHeader;	/* next header */
	uint8	hopLimit;	/* hop limit*/

	uint32	sourceAddr[4];	/*source address*/
	uint32	destinationAddr[4];	/* destination address */
};


struct mldv1Pkt
{
	uint8 type;
	uint8 code;		/*initialize by sender, ignored by receiver*/
	uint16 checkSum;
	uint16 maxResDelay;	/*maximum response delay,unit:0.001 second*/
	uint16 reserved;
	uint32 mCastAddr[4];	/*ipv6 multicast address*/
};


struct mldv2Query
{
	uint8 type;
	uint8 code;		/*initialize by sender, ignored by receiver*/
	uint16 checkSum;
	uint16 maxResCode;	/*maximum response code,unit:0.001 second*/
	uint16 reserved;
	uint32 mCastAddr[4];
	uint8 rsq;		/* 4bits: reserved, 1bit: suppress router-side processing, 3bits: querier's robustness variable*/
	uint8 qqic;		/* querier's query interval code */
	uint16 numOfSrc;	/* number of sources */
	uint32 srcList;
};

struct mCastAddrRecord
{
	uint8	type;		/* Record Type */
	uint8	auxLen;		/* auxiliary data length, in uints of 32 bit words*/
	uint16	numOfSrc;	/* number of sources */
	uint32	mCastAddr[4];	/* group address being reported */
	uint32	srcList;	/* first entry of source list */
};

struct mldv2Report
{
	uint8 type;
	uint8  reserved1;
	uint16 checkSum;
	uint16 reserved2;
	uint16 numOfRecords;	/* number of multicast address records */
	struct mCastAddrRecord recordList;
};

struct ipv4MospfHdr
{
	uint8 version;
	uint8 type;
	uint16 pktLen;
	uint32 routerId;
	uint32 areaId;
	uint16 CheckSum;
	uint16 auType;
	uint32 authentication[2];
};

struct ipv4MospfHello
{
	struct ipv4MospfHdr hdr;
	uint32 netWorkMask;
	uint16 helloInterVal;
	uint8  options;          /*  X-X-DC-EA-N/P-MC-E-X */
	uint8 routerPriority;
	uint32 routerDeadInterval;
	uint32 designatedRouter;
	uint32 backupDesignatedRouter;
	uint32 neighbor;
};

struct ipv6MospfHdr
{
	uint8 version; /*vesion is 3*/
	uint8 type;
	uint16 pktLen;
	uint32 routerId;
	uint32 areaId;
	uint16 CheckSum;
	uint8 instanceId;
	uint8 zeroData;
};

struct ipv6MospfHello
{
	struct ipv6MospfHdr hdr;
	uint32 interfaceId;
	uint8 routerPriority;
	uint8  options[3];          /*X-X-DC-R-N-MC-E-V6 */
	uint16 helloInterval;
	uint16 routerDeadInterval;
	uint32 designatedRouter;
	uint32 backupDesignatedRouter;
	uint32 neighbor;
};

struct ipv6PseudoHeader
{
	uint32 sourceAddr[4];
	uint32 destinationAddr[4];
	uint32 upperLayerPacketLength;
	uint8  zeroData[3];
	uint8  nextHeader;
};

struct ipv4PseudoHeader
{
	uint32 sourceAddr;
	uint32 destinationAddr;
	uint8 zero;
	uint8  protocol;
	uint16  payloadLen;
};


union pseudoHeader
{
	struct ipv6PseudoHeader ipv6_pHdr;
	struct ipv4PseudoHeader ipv4_pHdr;
};



struct rtk_igmp_multicastFwdInfo
{
	uint8 unknownMCast:1;
	uint8 reservedMCast:1;
	uint8 hwCbAcc:1;
	uint8 dropPkt:1;
	uint8 cpuFlag:1;			//ignore this group
	uint8 copy2cpu:1;			//copy a packet to cpu
	rtk_igmp_flowToDevInfo_t egressDevInfo[IGMP_MAX_DEV_NUM];
};


struct rtk_igmp_mcastFlowEntry
{
	struct rtk_igmp_mcastFlowEntry *previous;
	struct rtk_igmp_mcastFlowEntry  *next;             /*Pointer of next group entry*/
	uint32 ipVersion:3;
	int32 ingressCvlan:13;		//-1:untag
	int32 ingressSvlan:13;		//-1:untag
	uint16 defineFwdPktCnt;
	uint32 fwdPktCnt;
	uint32 serverAddr[4];
	uint32 groupAddr[4];
	int16  l4proto;
	int32 sport;
	int32 dport;
	struct rtk_igmp_multicastFwdInfo multicastFwdInfo;
	uint32 refreshTime;
};



typedef struct rtk_igmp_multicastBrDeviceInfo_s
{
	char devName[32];
	struct net_device *dev;
	rtk_igmp_brConfdb_t brDevConf;
	rtk_igmp_statistic_t brStatistic;
	uint16 ifindex;
	//int16 vlanId;
	//int16 svlanId;
	/*gateway infomation*/
	uint8 rtk_igmp_gatewayMac[6];
	//uint32 rtk_igmp_gatewayIpv4Addr;
	//uint32 rtk_igmp_gatewayIpv6Addr[4];
}rtk_igmp_multicastBrDeviceInfo_t;


typedef struct rtk_igmp_multicastModule
{
	uint8 validBit:1;

	rtk_igmp_multicastBrDeviceInfo_t brDevInfo;

	/*hash table definition*/
	struct rtk_igmp_groupEntry ** rtk_igmp_ipv4HashTable;
	struct rtk_igmp_groupEntry ** rtk_igmp_ipv6HashTable;

	struct rtk_igmp_mcastFlowEntry **flowHashTable;


}rtk_igmp_multicastModule_t;

#define DUMMY_PKT (1<<0)
#define SKIP_LEARNING (1<<1)

struct rtk_igmp_multicastDataInfo
{
	uint32 	ipVersion;
	uint8	vlanTagif;
	int16 	vlanId;
	uint8	svlanTagif;
	int16 	svlanId;	
	uint32 	sourceIp[4];
	uint32 	groupAddr[4];
	int32	l4proto;
	int32	sport;
	int32	dport;
	int32	privateField;	//ref to DUMMY_PKT/SKIP_LEARNING
};


static inline bool rtk_igmp_ipv4_is_local_multicast(__be32 addr)
{
	return (addr & htonl(0xffffff00)) == htonl(0xe0000000);
}

//init
int rtk_igmp_init(void);
int rtk_igmp_exit(void);

//dump
int rtk_igmp_devDump(int ifidx,struct seq_file *s, void *v);
int rtk_igmp_allDump(struct seq_file *s, void *v);
int rtk_igmp_dump_devConfig(int ifidx,struct seq_file *s, void *v);
int rtk_igmp_dump_allDevConfig(struct seq_file *s, void *v);
int rtk_igmp_dump_memoryUsed(struct seq_file *s, void *v);
int show_igmp_whiteList_all(struct seq_file *s, void *v);
int show_igmp_blackList_all(struct seq_file *s, void *v);


//dev ifidx transfer
int rtk_igmp_wanIdxBindToBrIdx(int32 wanifidx,int32 brIfidx);
int rtk_igmp_devNameToDevIfidx(char *devName);
int rtk_igmp_devNameToBrDevIfidx(char *devName);
struct net_device * rtk_igmp_devIfidx_to_dev(int32 devIfindex);
int rtk_igmp_devInfoToIfIdx(rt_igmpHook_devInfo_t devInfo);



//IGMP/DATA Process
int rtk_igmp_getMulticastDataFwdInfo(const struct net_device *SrcDev,const struct net_device *DstDev,uint32 searchCacheFlowFirst, struct rtk_igmp_multicastDataInfo *multicastDataInfo, struct rtk_igmp_multicastFwdInfo *multicastFwdInfo);
rtk_igmp_nf_ret_e_t rtk_igmp_mld_process ( rtk_igmp_pktHdr_t *pkthdr, const struct net_device *SrcDev);


//white List API
int rtk_igmp_whiteListAdd( rt_igmpHook_whiteList_t *patten,int32 ifidx , int32 *index);
int rtk_igmp_whiteListDel( rt_igmpHook_whiteList_t *patten,int32 ifidx,int32 index);
int rtk_igmp_whiteListDel_byIdx(int32 ifidx,int32 index);


//black List API
int rtk_igmp_blackListAdd( rt_igmpHook_blackList_t *patten,int32 ifidx , int32 *index);
int rtk_igmp_blackListDel(rt_igmpHook_blackList_t *patten,int32 ifidx,int32 index);
int rtk_igmp_blackListDel_byIdx(int32 ifidx,int32 index);

//dec configuration API
int rtk_igmp_devConfig_set(int devifid , uint32 isIpv6 ,rt_igmpHook_control_list_t type , uint32 value);
int rtk_igmp_devConfig_get(int devifid , uint32 isIpv6 ,rt_igmpHook_control_list_t type , uint32* value);

int rtk_igmp_BrDevConfig_set(int devifid ,rt_igmpHook_br_control_list_t type , uint32 value);
int rtk_igmp_BrDevConfig_get(int devifid ,rt_igmpHook_br_control_list_t type , uint32* value);


//group trap to protocol stack
int rtk_igmp_groupToPsTblAdd(rt_igmpHook_ignoreGroup_t* patten,uint32 *index);
int rtk_igmp_groupToPsTblDel(rt_igmpHook_ignoreGroup_t* patten,uint32 index);
int rtk_igmp_groupToPsDump(struct seq_file *s, void *v);


int rtk_igmp_nextValidGroupStatistic_get(rt_igmpHook_devInfo_t devInfo,int *getIdx,unsigned int isIpv6,rt_igmpHook_groupStatistic_t *statisticInfo);
int rtk_igmp_devStatistic_get(rt_igmpHook_devInfo_t devInfo,unsigned int isIpv6,rt_igmpHook_devStatistic_t *statisticInfo);
int rtk_igmp_groupWeightTblAdd(rt_igmpHook_devInfo_t devInfo,rt_igmpHook_groupWeight_t* patten,uint32 *index);
int rtk_igmp_groupWeightTblDel(rt_igmpHook_devInfo_t devInfo,rt_igmpHook_groupWeight_t* patten,uint32 index);
int rtk_igmp_groupWeightDump(struct seq_file *s, void *v);


int rtk_igmp_wanIfNameBindToBrName(char *wanDevName , char *brDevName);
int rtk_igmp_anyDevDisSnooping(uint32 isIpv6);
rtk_igmp_userResvGroup_t * _rtk_igmp_checkGroupToPsTbl(uint32 ipVersion,uint32 *_groupIp);
int rtk_igmp_flowCbAppendOtherBrToCb(int32 moduleIndex, struct rtk_igmp_multicastDataInfo *multicastDataInfo,rtk_igmp_flowCbEvt_t *flowCb);
int rtk_igmp_flow_reflush_update_with_hw(int32 moduleIndex,uint32* groupAddr);


//tool
void* rtk_igmp_kmalloc(size_t size);
int rtk_igmp_kfree(const void *ptr,size_t size);


int32 rtk_igmp_hwCbReg(void);
int32 rtk_igmp_hwCbUnReg(void);


//hardware callback register by (yourHardware).c file
int32 rtk_igmp_flowDelCbEvt(rtk_igmp_flowCbEvt_t *flowCb);
int32 rtk_igmp_flowSetCbEvt(rtk_igmp_flowCbEvt_t *flowCb);
int32 rtk_igmp_flowUpdateCbEvt(rtk_igmp_flowCbEvt_t *flowCb);
int32 rtk_igmp_groupAddCbEvt(rtk_igmp_groupCbEvt_t *groupCb);
int32 rtk_igmp_groupDelCbEvt(rtk_igmp_groupCbEvt_t *groupCb);
int32 rtk_igmp_flowCheckCbEvt(rtk_igmp_flowCbEvt_t *flowCb);
int32 rtk_igmp_unknownIpMc_Action(rtk_igmp_unknownMcCbEvt_t *unknownCtrl);
int32 rtk_igmp_hwRegInit(void);
int32 rtk_igmp_hwUnRegClear(void);

int32 _rtk_igmp_flowDelCbEvt(rtk_igmp_flowCbEvt_t *flowCb);
int32 _rtk_igmp_flowSetCbEvt(rtk_igmp_flowCbEvt_t *flowCb);
int32 _rtk_igmp_flowUpdateCbEvt(rtk_igmp_flowCbEvt_t *flowCb);
int32 _rtk_igmp_groupAddCbEvt(rtk_igmp_groupCbEvt_t *groupCb);
int32 _rtk_igmp_groupDelCbEvt(rtk_igmp_groupCbEvt_t *groupCb);
int32 _rtk_igmp_flowCheckCbEvt(rtk_igmp_flowCbEvt_t *flowCb);
int32 _rtk_igmp_unknownIpMc_Action(rtk_igmp_unknownMcCbEvt_t *unknownCtrl);
int32 _rtk_igmp_hwRegInit(void);
int32 _rtk_igmp_hwUnRegClear(void);


#endif

