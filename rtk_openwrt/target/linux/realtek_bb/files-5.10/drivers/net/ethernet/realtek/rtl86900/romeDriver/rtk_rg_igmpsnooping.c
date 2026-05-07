/*
* Copyright c                  Realsil Semiconductor Corporation, 2009
* All rights reserved.
*
* Program :  igmp snooping function
* Abstract :
* Author :qinjunjie
* Email:qinjunjie1980@hotmail.com
*
*/
#if defined(CONFIG_APOLLO_ROMEDRIVER)

#include <rtk_rg_igmpsnooping.h>

#ifdef __KERNEL__
#ifdef __linux__

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 30)
#else
#include <linux/config.h>
#endif
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <linux/skbuff.h>
#ifdef CONFIG_PROC_FS
#include <linux/seq_file.h>
#endif
#endif

#else //model
#include <stdio.h>
#include <string.h>
#include <mem.h>
#include <stdlib.h>
#endif

#include <common/type.h>
#include <rtk_rg_fwdEngine.h>

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#include <rtk_rg_apolloPro_liteRomeDriver.h>
#endif

#if defined(CONFIG_RG_RTL9600_SERIES)
#include <rtk_rg_acl_apollo.h>
#include <rtk_rg_acl.h>
#endif


//static int IGMPQueryVersion = 3;	//Default is using IGMP V3 to Query.
//static int IGMPQuery3Length = 0;	//Default is using IGMP V3 to Query. // - delete


static unsigned char *igmpQueryBuf;

/*igmpv3 general query, 50*/
static unsigned char igmpQueryBufV3[64]={
	0x01,0x00,0x5e,0x00,0x00,0x01,	/*destination mac*/
	0x00,0x00,0x00,0x00,0x00,0x00,	/*offset:6*/
	0x08,0x00,			/*offset:12*/
	0x46,0x00,0x00,0x24,		/*offset:14*/
	//0x46,0x00,0x00,0x28,		/*offset:14*/
	0x00,0x00,0x40,0x00,		/*offset:18*/
	0x01,0x02,0x00,0x00,		/*offset:22*/
	0x00,0x00,0x00,0x00,		/*offset:26,source ip*/
	0xe0,0x00,0x00,0x01,		/*offset:30,destination ip*/
	0x94,0x04,0x00,0x00,		/*offset:34,router alert option*/
	0x11,0x64,0x00,0x00,		/*offset:38,max response time*/
	0x00,0x00,0x00,0x00,		/*offset:42,queried multicast ip address*/
	0x0a,0x3c,0x00,0x00,		/*offset:46*/
	0x00,0x00,0x00,0x00,		/*offset:50*/
	0x00,0x00,0x00,0x00,		/*offset:54*/
	0x00,0x00,0x00,0x00,		/*offset:58*/
	0x00,0x00			/*offset:62*/
	//0x00,0x00,0x00,0x00		/*offset:62*/
};

/* max source is about 243, skb no considerate CRC */
#define IGMPV3_QUERY_SOURCE_MAX_NO	((1024 - 52)>>2)
#define IGMPV3_QUERY_SOURCE_MIN_NO	((64 - 52)>>2)
#define IGMPV3_QUERY_MAX_SIZE	((IGMPV3_QUERY_SOURCE_MAX_NO<<2) + 52)

/*igmpv2 general query, 42*/
static unsigned char igmpQueryBufV2[64]={
	0x01,0x00,0x5e,0x00,0x00,0x01,	/*destination mac*/
	0x00,0x00,0x00,0x00,0x00,0x00,	/*offset:6*/
	0x08,0x00,			/*offset:12*/
	0x45,0x00,0x00,0x1c,		/*offset:14*/
	0x00,0x00,0x40,0x00,		/*offset:18*/
	0x01,0x02,0x00,0x00,		/*offset:22*/
	0x00,0x00,0x00,0x00,		/*offset:26*/
	0xe0,0x00,0x00,0x01,		/*offset:30*/
	0x11,0x64,0x0c,0xfa,		/*offset:34,max response time*/
	0x00,0x00,0x00,0x00,		/*offset:38*/
	0x00,0x00,0x00,0x00,		/*offset:42*/
	0x00,0x00,0x00,0x00,		/*offset:46*/
	0x00,0x00,0x00,0x00,		/*offset:50*/
	0x00,0x00,0x00,0x00,		/*offset:54*/
	0x00,0x00,0x00,0x00,		/*offset:58*/
	0x00,0x00			/*offset:62*/
};

static unsigned char mldQueryBuf[86]={
	0x33,0x33,0x00,0x00,0x00,0x01,	/*destination mac*/
	0x00,0x00,0x00,0x00,0x00,0x00,	/*source mac*/	/*offset:6*/
	0x86,0xdd,			/*ether type*/	/*offset:12*/
	0x60,0x00,0x00,0x00,		/*version(1 byte)-traffic cliass(1 byte)- flow label(2 bytes)*/	/*offset:14*/
	0x00,0x20,0x00,0x01,		/*payload length(2 bytes)-next header(1 byte)-hop limit(value:1 1byte)*//*offset:18*/
	0xfe,0x80,0x00,0x00,		/*source address*/	/*offset:22*/
	0x00,0x00,0x00,0x00,		/*be zero*/	/*offset:26*/
	0x00,0x00,0x00,			/*upper 3 bytes mac address |0x02*/ /*offset:30*/
	0xff,0xfe,			/*fixed*/
	0x00,0x00,0x00,			/*lowert 3 bytes mac address*/	 /*offset:35*/
	0xff,0x02,0x00,0x00,		/*destination address is fixed as FF02::1*/	/*offset:38*/
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x01,
	0x3a,0x00,			/*icmp type(1 byte)-length(1 byte)*/	 /*offset:54*/
	0x05,0x02,0x00,0x00,		/*router alert option*/
	0x01,0x00,			/*padN*/
	0x82,0x00,			/*type(query:0x82)-code(0)*/	/*offset:62*/
	0x00,0x00,			/*checksum*/	/*offset:64*/
	0x00,0x0a,			/*maximum reponse code*/
	0x00,0x00,			/*reserved*/
	0x00,0x00,0x00,0x00,		/*multicast address,fixed as 0*/
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00
};

static unsigned char ipv6PseudoHdrBuf[40]={
	0xfe,0x80,0x00,0x00,		/*source address*/
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0xff,
	0xfe,0x00,0x00,0x00,
	0xff,0x02,0x00,0x00,		/*destination address*/
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x01,
	0x00,0x00,0x00,0x18,		/*upper layer packet length*/
	0x00,0x00,0x00,0x3a		/*zero padding(3 bytes)-next header(1 byte)*/
};



struct rtl_multicastModule rtl_mCastModuleArray[MAX_MCAST_MODULE_NUM];

#if defined(__linux__) && defined(__KERNEL__)

#ifdef TIMER_AGG
rtk_rg_timer_t igmpSysTimer;	/*igmp timer*/
rtk_rg_timer_t mCastQuerytimer;	/*igmp query timer*/
rtk_rg_timer_t mCastForceReporttimer;	/*igmp mld force response timer*/
#else
struct timer_list igmpSysTimer;	/*igmp timer*/
struct timer_list mCastQuerytimer;	/*igmp query timer*/
struct timer_list mCastForceReporttimer;	/*igmp mld force response timer*/
#endif



#endif

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

#define NIP4QUAD(addr) \
	NIPQUAD((addr[0]))
	//NIPQUAD((addr))

#define NIP6QUAD(addr) \
	NIPQUAD((addr[0])), \
	NIPQUAD((addr[1])), \
	NIPQUAD((addr[2])), \
	NIPQUAD((addr[3]))

#define NMACQUAD(addr) \
	((unsigned char *)addr)[0], \
	((unsigned char *)addr)[1], \
	((unsigned char *)addr)[2], \
	((unsigned char *)addr)[3], \
	((unsigned char *)addr)[4], \
	((unsigned char *)addr)[5]

#define MACH "%02X:%02X:%02X:%02X:%02X:%02X"
#define IP4D "%d.%d.%d.%d"
#define IP4H "%X:%X:%X:%X"
#define IP6D IP4D" "IP4D" "IP4D" "IP4D
#define IP6H IP4H" "IP4H" "IP4H" "IP4H
#define IPV6ADDRLEN 16

/*global system resources declaration*/
static uint32 rtl_totalMaxGroupCnt;    /*maximum total group entry count,  default is 100*/
static uint32 rtl_totalMaxClientCnt;    /*maximum total group entry count,  default is 100*/
static uint32 rtl_totalMaxSourceCnt;   /*maximum total group entry count,  default is 300*/

void *rtl_groupMemory=NULL;
void *rtl_clientMemory=NULL;
void *rtl_sourceMemory=NULL;
void *rtl_mcastFlowMemory=NULL;

struct rtl_clientEntry clientEntryPool[DEFAULT_MAX_CLIENT_COUNT];
struct rtl_groupEntry groupEntryPool[DEFAULT_MAX_GROUP_COUNT];
struct rtl_sourceEntry sourceEntryPool[DEFAULT_MAX_SOURCE_COUNT];
#ifdef CONFIG_RECORD_MCAST_FLOW
struct rtl_mcastFlowEntry mcastFlowEntryPool[DEFAULT_MAX_FLOW_COUNT];

#endif

//static int  igmpsnoopenabled=1;
//static int mldSnoopEnabled=1;
static uint32 igmpSnoopingCounterVer_Port[RTK_RG_MAC_PORT_MAX][IGMP_MLD_VER_MAX];
#ifdef CONFIG_MASTER_WLAN0_ENABLE
static uint32 igmpSnoopingCounterVer_MBSSID[MAX_WLAN_DEVICE_NUM][IGMP_MLD_VER_MAX];
#endif
static uint8 igmpSnoopingCounterVer_1=0; ////: add
static uint8 igmpSnoopingCounterVer_2=0;
static uint8 igmpSnoopingCounterVer_3=0;
static uint8 MLDCounterVer_1=0;
static uint8 MLDCounterVer_2=0;

static struct rtl_groupEntry *rtl_groupEntryPool=NULL;
static struct rtl_clientEntry *rtl_clientEntryPool=NULL;
static struct rtl_sourceEntry *rtl_sourceEntryPool=NULL;
#ifdef CONFIG_RECORD_MCAST_FLOW
static struct rtl_mcastFlowEntry *rtl_mcastFlowEntryPool=NULL;
#endif
struct rtl_mCastTimerParameters rtl_mCastTimerParas;  /*IGMP snooping parameters */

static uint32 rtl_hashTableSize=0;
static uint32 rtl_hashMask=0;

static uint32 system_expire_type;


/*the system up time*/
static uint32 rtl_startTime;
static uint32 rtl_sysUpSeconds;
static unsigned long int last_query_jiffies;	/*record the system jiffie of last query send*/

static rtl_multicastEventContext_t reportEventContext;
static rtl_multicastEventContext_t timerEventContext;

#if defined (CONFIG_RG_HARDWARE_MULTICAST)
void dump_hwMappingEntry(void);
static rtl_multicastEventContext_t linkEventContext;
int rtl_handle_igmpgroup_change(rtl_multicastEventContext_t* param);
#ifdef CONFIG_RG_SIMPLE_IGMP_v3
int rtl_handle_igmpgroup_change_v3(rtl_multicastEventContext_t* param);
#endif
struct list_head rtl_multicast_index_mapping_listhead;
struct list_head hwMappingEntryFreeListHead;
rtl_multicast_index_mapping_t hwMappingEntry[DEFAULT_MAX_HW_MAPPING_COUNT];

#endif



/*******************************internal function declaration*****************************/


/**************************
	resource managment
**************************/
static  struct rtl_groupEntry* rtl_initGroupEntryPool(uint32 poolSize);
static  struct rtl_groupEntry* rtl_allocateGroupEntry(void);
static  void rtl_freeGroupEntry(struct rtl_groupEntry* groupEntryPtr) ;


static  struct rtl_clientEntry* rtl_initClientEntryPool(uint32 poolSize);
static  struct rtl_clientEntry* rtl_allocateClientEntry(void);
static  void rtl_freeClientEntry(struct rtl_clientEntry* clientEntryPtr) ;

static  struct rtl_sourceEntry* rtl_initSourceEntryPool(uint32 poolSize);
static  struct rtl_sourceEntry* rtl_allocateSourceEntry(void);
static  void rtl_freeSourceEntry(struct rtl_sourceEntry* sourceEntryPtr) ;
#ifdef CONFIG_RECORD_MCAST_FLOW
static  struct rtl_mcastFlowEntry* rtl_initMcastFlowEntryPool(uint32 poolSize);
static  struct rtl_mcastFlowEntry* rtl_allocateMcastFlowEntry(void);
static  void rtl_freeMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry) ;
#endif
/**********************************Structure Maintenance*************************/

static struct rtl_groupEntry* rtl_searchGroupEntry(uint32 moduleIndex, uint32 ipVersion,uint32 *multicastAddr, uint16 vlanId);
static void rtl_linkGroupEntry(struct rtl_groupEntry* entryNode ,  struct rtl_groupEntry ** hashTable);
static void rtl_unlinkGroupEntry(struct rtl_groupEntry* entryNode,  struct rtl_groupEntry ** hashTable);
static void rtl_clearGroupEntry(struct rtl_groupEntry* groupEntryPtr);


static struct rtl_clientEntry* rtl_searchClientEntry(uint32 ipVersion,struct rtl_groupEntry* groupEntry, uint32 portNum, uint32 *clientAddr);
static void rtl_linkClientEntry(struct rtl_groupEntry *groupEntry, struct rtl_clientEntry* clientEntry);
static void rtl_unlinkClientEntry(struct rtl_groupEntry *groupEntry, struct rtl_clientEntry* clientEntry);
static void rtl_clearClientEntry(struct rtl_clientEntry* clientEntryPtr);
static void rtl_deleteClientEntry(struct rtl_groupEntry * groupEntry, struct rtl_clientEntry * clientEntry);

static struct rtl_sourceEntry* rtl_searchSourceEntry(uint32 ipVersion, uint32 *sourceAddr, struct rtl_clientEntry *clientEntry);
static void rtl_linkSourceEntry(struct rtl_clientEntry *clientEntry,  struct rtl_sourceEntry* entryNode);
static void rtl_unlinkSourceEntry(struct rtl_clientEntry *clientEntry, struct rtl_sourceEntry* entryNode);
static void rtl_clearSourceEntry(struct rtl_sourceEntry* sourceEntryPtr);
static void rtl_deleteSourceEntry(struct rtl_clientEntry *clientEntry, struct rtl_sourceEntry* sourceEntry);
#ifdef CONFIG_RECORD_MCAST_FLOW
static struct rtl_mcastFlowEntry* rtl_searchMcastFlowEntry(uint32 moduleIndex, uint32 ipVersion, uint32 *serverAddr,uint32 *groupAddr,uint16 sport,uint16 dport);
static void  rtl_linkMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry ,  struct rtl_mcastFlowEntry ** hashTable);
static void rtl_unlinkMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry,  struct rtl_mcastFlowEntry ** hashTable);
static void rtl_clearMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry);
static void rtl_deleteMcastFlowEntry( struct rtl_mcastFlowEntry* mcastFlowEntry, struct rtl_mcastFlowEntry ** hashTable);
#endif
//static int32 rtl_checkMCastAddrMapping(uint32 ipVersion, uint32 *ipAddr, uint8* macAddr);

//static int32 rtl_compareIpv6Addr(uint32* ipv6Addr1, uint32* ipv6Addr2);
static uint16 rtl_ipv6L3Checksum(uint8 *pktBuf, uint32 pktLen, union pseudoHeader *ipv6PseudoHdr);

//static int32 rtl_compareMacAddr(uint8* macAddr1, uint8* macAddr2);
static uint16 rtl_checksum(uint8 *packetBuf, uint32 packetLen);
// Mason Yu. type error
//static uint8 rtl_getClientFwdPortMask(struct rtl_clientEntry * clientEntry,  uint32 sysTime);
static uint32 rtl_getClientFwdPortMask(struct rtl_clientEntry * clientEntry,  uint32 sysTime);
static int32 rtl_checkSourceTimer(struct rtl_clientEntry * clientEntry , struct rtl_sourceEntry * sourceEntry);


static void rtl_checkGroupEntryTimer(struct rtl_groupEntry * groupEntry, struct rtl_groupEntry ** hashTable);
static void rtl_checkClientEntryTimer(struct rtl_groupEntry * groupEntry, struct rtl_clientEntry * clientEntry);

static uint32  rtl_getMulticastRouterPortMask(uint32 moduleIndex, uint32 ipVersion, uint32 sysTime);


/*hash table operation*/
static int32 rtl_initHashTable(uint32 moduleIndex, uint32 hashTableSize);


/************************************Pkt Process**********************************/
/*MAC frame analyze function*/
static void  rtl_parseMacFrame(uint32 moduleIndex, uint8* MacFrame, rtk_rg_pktHdr_t *pPktHdr, uint32 verifyCheckSum, struct rtl_macFrameInfo* macInfo);

/*Process Query Packet*/
static void rtl_snoopQuerier(uint32 moduleIndex, uint32 ipVersion, uint32 portNum);
static uint32 rtl_processQueries(uint32 moduleIndex, rtk_rg_pktHdr_t *pPktHdr, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint8* pktBuf, uint32 pktLen);
/*Process Report Packet*/
static  uint32 rtl_processJoin(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8 *pktBuf); // process join report packet
static  uint32 rtl_processLeave(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8 *pktBuf); //process leave/done report packet
static  int32 rtl_processIsInclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8 *pktBuf); //process MODE_IS_INCLUDE report packet
static  int32 rtl_processIsExclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8 *pktBuf); //process MODE_IS_EXCLUDE report packet
static  int32 rtl_processToInclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8 *pktBuf); //process CHANGE_TO_INCLUDE_MODE report packet
static  int32 rtl_processToExclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8 *pktBuf); //process CHANGE_TO_EXCLUDE_MODE report packet
static  int32 rtl_processAllow(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8 *pktBuf); //process ALLOW_NEW_SOURCES report packet
static  int32 rtl_processBlock(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8 *pktBuf);//process BLOCK_OLD_SOURCES report packet
static  uint32 rtl_processIgmpv3Mldv2Reports(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8 *pktBuf);

/*******************different protocol process function**********************************/
static uint32 rtl_processIgmpMld(uint32 moduleIndex, rtk_rg_pktHdr_t *pPktHdr, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8* pktBuf, uint32 pktLen);
static uint32 rtl_processDvmrp(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8* pktBuf, uint32 pktLen);
static uint32 rtl_processMospf(uint32 moduleIndex, uint32 ipVersion, uint32 portNum,  uint8* pktBuf, uint32 pktLen);
static uint32 rtl_processPim(uint32 moduleIndex, uint32 ipVersion,  uint32 portNum, uint8* pktBuf, uint32 pktLen);

#ifdef CONFIG_RECORD_MCAST_FLOW
static int32 rtl_recordMcastFlow(uint32 moduleIndex,uint32 ipVersion, uint32 *sourceIpAddr, uint32 *groupAddr, struct rtl_multicastFwdInfo * multicastFwdInfo,uint16 sport,uint16 dport);
static void rtl_invalidateMCastFlow(uint32 moduleIndex,uint32 ipVersion, uint32 *groupAddr);
static void rtl_doMcastFlowRecycle(uint32 moduleIndex,uint32 ipVersion);
#endif

#if  defined(__linux__) && defined(__KERNEL__)
void rtl_multicastSysTimerExpired(uint32 expireDada);
void rtl_multicastSysTimerInit(void);
static void rtl_multicastSysTimerDestroy(void);
#endif

static void rtl_deleteGroupEntry( struct rtl_groupEntry* groupEntry,struct rtl_groupEntry ** hashTable);
static int32 rtl_igmp_filterWhiteListCheck(uint32 ipVersion, uint32 *groupAddr , uint32 *clientAddr,uint8 *saMac);
static int32 rtl_igmp_filterBlackListCheck(uint32 ipVersion, uint32 *groupAddr , uint32 *clientAddr,uint8 *saMac);


/************************************************
			Implementation
  ************************************************/

/**************************
	Initialize
**************************/


uint32 _check_isDummyClient( struct rtl_clientEntry* clientEntry)
{
	if(clientEntry->igmpVersion >=IGMP_V1 && clientEntry->igmpVersion <=IGMP_V3)
	{
		if(clientEntry->clientAddr[0]==clientEntry->portNum)
			return TRUE;
	}
	else
	{
		if(clientEntry->clientAddr[3]==clientEntry->portNum && clientEntry->clientAddr[2]==0 && clientEntry->clientAddr[1]==0 && clientEntry->clientAddr[0]==0)
			return TRUE;
	}
	return FALSE;
}



static unsigned long lastjiffies=0;
static uint32 lastTimer=0; //start for 0 sec


uint32 rtk_rg_getTimeSec(void)
{
	int32 jiffiesDiff;
	if(jiffies > lastjiffies)
		jiffiesDiff =jiffies-lastjiffies;
	else
		jiffiesDiff =(0xffffffff-(lastjiffies-jiffies));
	lastjiffies=jiffies;
	lastTimer +=(jiffiesDiff/CONFIG_HZ);
	return lastTimer;
}


static unsigned short  rtl_ipv4Checksum(unsigned char *pktBuf, unsigned int pktLen)
{
	/*note: the first bytes of  packetBuf should be two bytes aligned*/
	unsigned int  checksum=0;
	unsigned int  count=pktLen;
	unsigned short   *ptr= (unsigned short *)pktBuf;

	 while(count>1)
	 {
		  checksum+= ntohs(*ptr);
		  ptr++;
		  count -= 2;
	 }

	if(count>0)
	{
		checksum+= *(pktBuf+pktLen-1)<<8; /*the last odd byte is treated as bit 15~8 of unsigned short*/
	}

	/* Roll over carry bits */
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	/* Return checksum */
	return ((unsigned short) ~ checksum);

}


static unsigned char* rtl_generateIgmpQuery(char *src_mac, char *src_ip, int igmp_ver, char* grp, int no_src, struct rtl_sourceEntry *src_ent, char * src_list, const int q3_len)
{
	unsigned short checkSum=0;
	uint32 networkAddressTmp;
	//unsigned char xmac[]	={ 0x00, 0x00, 0x00, 0x00, 0x09, 0x09 };
	//unsigned char xip[]	={ 0xc0, 0xa8, 0x09, 0x09 };
	//unsigned char xgip[]	={ 0xea, 0xea, 0xea, 0xea };	//234.234.234.234
	/*
	unsigned char s1[]	={0x9, 0x8, 0x7, 0x6};	//9.8.7.6
	unsigned char s2[]	={0x5, 0x4, 0x3, 0x2};	//5.4.3.2
	unsigned char *s[]	={s1,s2};
	*/
	//unsigned char s[][]=	{ {0x9, 0x8, 0x7, 0x6}, {0x5, 0x4, 0x3, 0x2} };	//5.4.3.2
	//unsigned char *s[]=	{ {0x9, 0x8, 0x7, 0x6}, {0x5, 0x4, 0x3, 0x2} };	//5.4.3.2
	//unsigned char s[2][4]={ {0x9, 0x8, 0x7, 0x6}, {0x5, 0x4, 0x3, 0x2} };	//5.4.3.2
	//unsigned char s[]={ 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2 };		//9.8.7.6, 5.4.3.2,

	//src_mac = { 0x00, 0x00, 0x00, 0x00, 0x09, 0x09 }; compile error
	//src_mac = (unsigned char *) { 0x00, 0x00, 0x00, 0x00, 0x09, 0x09 }; excess elements in scalar initializer
	//src_mac = xmac;
	//src_mac = (unsigned char []) { 0x00, 0x00, 0x00, 0x00, 0x09, 0x09 };

	//IGMPQuery3Length = 0x0;
	//if (IGMPQueryVersion == 3)
	if (igmp_ver==IGMP_V3)
	{
		/*
		src_mac =	"000000000808";
		src_ip =	"c0a80808";
		grp =		"eaeaeaea";
		no_src=1;
		//src_list=	"c0a0404";
		src_list=	{"c0a0404"};
		*/

		/*
		grp =		xgip;//234.234.234.234
		no_src=		2;
		src_list=	s;
		*/

		memcpy(igmpQueryBuf, igmpQueryBufV3, 64);
		if (grp != NULL)
		{
			networkAddressTmp = htonl(*((uint32*) grp));
			memcpy(&igmpQueryBuf[42],&networkAddressTmp,4);	/*set group IP address*/
			if (no_src>=0)
			{
				unsigned int i;

				//i = 0x24 + q3_len; //20 + 4 + 12 = 36 =0x24
				i = 36 + (no_src<<2); //20 + 4 + 12 = 36 =0x24
				igmpQueryBuf[16]= i >> 8;
				igmpQueryBuf[17]= i & 0xff;

				igmpQueryBuf[48]= no_src >> 8;
				igmpQueryBuf[49]= no_src & 0xff;

				if (src_ent!=NULL)
				{
					i = 0;
					do
					{
						if (src_ent->setOpFlag)
						{
							networkAddressTmp = htonl(src_ent->sourceAddr[0]);
							memcpy(&igmpQueryBuf[50 + (i<<2)], &networkAddressTmp, 4);
							src_ent->setOpFlag=0;
							i++;
						}
						src_ent = src_ent->next;
					} while (src_ent!=NULL);
					//} while (src_ent!=NULL && i<no_src);
				}
				else
				{
					for (i=0;i<no_src;i++)
					{
						//src_list is a network buffer
						networkAddressTmp = *((uint32 *)(src_list+(i<<2)));
						memcpy(&igmpQueryBuf[50 + (i<<2)],&networkAddressTmp, 4); //50~53, 54~57, 58~61, 62~65
					}
				}
			} //no_src > 0
		}
	}
	else //igmp_ver is IGMP_V1 or IGMP_V2
	{
		memcpy(igmpQueryBuf,igmpQueryBufV2,64);
		if (grp != NULL)
		{
			networkAddressTmp = htonl(*((uint32*) grp));
			memcpy(&igmpQueryBuf[38],&networkAddressTmp,4);	/*set group IP address*/
		}
	}

	memcpy(&igmpQueryBuf[6],src_mac,6);			/*set source mac address*/
	networkAddressTmp = htonl(*((uint32 *)src_ip));
	memcpy(&igmpQueryBuf[26],&networkAddressTmp,4);			/*set source IP address*/

	igmpQueryBuf[24]=0;
	igmpQueryBuf[25]=0;

	if (igmp_ver==IGMP_V3)
		checkSum=rtl_ipv4Checksum(&igmpQueryBuf[14],24);
	else
		checkSum=rtl_ipv4Checksum(&igmpQueryBuf[14],20);

	igmpQueryBuf[24]=(checkSum&0xff00)>>8;
	igmpQueryBuf[25]=(checkSum&0x00ff);

	//if (IGMPQueryVersion == 3)
	if (igmp_ver==IGMP_V3)
	{
		igmpQueryBuf[40]=0;
		igmpQueryBuf[41]=0;
		//checkSum=rtl_ipv4Checksum(&igmpQueryBuf[38],12);
		//checkSum=rtl_ipv4Checksum(&igmpQueryBuf[38],12 + IGMPQuery3Length);
		//checkSum=rtl_ipv4Checksum(&igmpQueryBuf[38],12 + q3_len);
		checkSum=rtl_ipv4Checksum(&igmpQueryBuf[38],12 + (no_src<<2));
		igmpQueryBuf[40]=(checkSum&0xff00)>>8;
		igmpQueryBuf[41]=(checkSum&0x00ff);
	}
	else
	{
		igmpQueryBuf[36]=0;
		igmpQueryBuf[37]=0;
		checkSum=rtl_ipv4Checksum(&igmpQueryBuf[34],8);
		igmpQueryBuf[36]=(checkSum&0xff00)>>8;
		igmpQueryBuf[37]=(checkSum&0x00ff);
	}

	return igmpQueryBuf;
}



#ifdef __KERNEL__
/*
arg: 0, gen query after accessing report
     1, gen query as Timer expired

*/

static void rtl_igmpQueryTimerExpired(unsigned long arg, int igmp_ver, char *grp, int no_src, struct rtl_sourceEntry *src_ent, char *src_list)
{
	struct sk_buff *skb;
	int i,rg_fwdengine_ret_code=0;
	int q3_len = 0;
	//int tmp_intf_vlan_id;
	rg_kernel.tracefilterShow =0; //disable tracefilter show

	if (_rtk_rg_get_initState()!=RTK_RG_INIT_FINISHED)
		return;

	rg_db.pktHdr=&rg_db.systemGlobal.pktHeader_2;

	/**send packet***/
	for (i=0;i<rg_db.systemGlobal.lanIntfTotalNum;i++)
	{

		skb=(struct sk_buff *)_rtk_rg_getAlloc(RG_FWDENGINE_PKT_LEN);

		//call fwdEngineInput, the alloc counter will be added. so don't need to add again
		if ((skb)&&(rg_db.systemGlobal.fwdStatistic))
		{
			#if RTK_RG_SKB_PREALLOCATE
			rg_db.systemGlobal.statistic.perPortCnt_skb_pre_alloc_for_uc[rg_db.pktHdr->ingressPort]--;
			#else
			rg_db.systemGlobal.statistic.perPortCnt_skb_alloc[rg_db.pktHdr->ingressPort]--;
			#endif
		}

		if (skb==NULL)
		{
			WARNING("Error: skb null");
			return;
		}


		DEBUG("lanIntfTotalNum=%d\n",rg_db.systemGlobal.lanIntfTotalNum);

		//if (IGMPQueryVersion==3)
		if (igmp_ver==IGMP_V3)
		{
			if (no_src > IGMPV3_QUERY_SOURCE_MAX_NO)
			{
				q3_len = IGMPV3_QUERY_MAX_SIZE;
				no_src = IGMPV3_QUERY_SOURCE_MAX_NO;
			}

			else // >= 4
			{
				q3_len = (no_src<<2) + 50;//14+24+12
				//q3_len = (no_src<<2) + 12;
			}

			bzero((void*)skb->data, q3_len);
		}
		else //igmp query 1 or 2
		{
			q3_len=0;
			bzero((void*)skb->data, 64);
		}

		igmpQueryBuf = skb->data;
		rtl_generateIgmpQuery(rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet,(char *)&rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->ip_addr, igmp_ver, grp, no_src, src_ent, src_list, q3_len);

		skb->len = 0;
		//if (IGMPQueryVersion==3)
		if (igmp_ver==IGMP_V3)
		{
			//skb_put(skb, 50); //0 serverip
			//skb_put(skb, 50 + IGMPQuery3Length); //N serverip
			skb_put(skb, q3_len); //N serverip
		}else
			skb_put(skb, 42);

		rg_kernel.rxInfoFromIGMPMLD.rx_cvlan_tag=rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->intf_vlan_id;

		rg_db.pktHdr->IGMPQueryVer = igmp_ver; //IGMP_V1, IGMP_V2, IGMP_V3,
		rg_fwdengine_ret_code = rtk_rg_fwdEngineInput(skb, &rg_kernel.rxInfoFromIGMPMLD);

		//Processing packets
		if (rg_fwdengine_ret_code == RG_FWDENGINE_RET_TO_PS)
		{
			//FIXME:iPhone 5 change wireless connection from master to slave will send strange unicast ARP request for LAN gateway IP, and forwarded by protocol stack
			IGMP("IGMP_TX_O[%x]: To Protocol-Stack...FREE SKB!!\n",(POINTER_CAST)skb&0xffff);
			//dump_packet(skb->data,skb->len,"dump_back_to_PS");
			_rtk_rg_dev_kfree_skb_any(skb);

		}
		else if (rg_fwdengine_ret_code == RG_FWDENGINE_RET_DROP)
		{
			IGMP("IGMP_TX_O[%x]: Drop...FREE SKB!!\n",(POINTER_CAST)skb&0xffff);
			_rtk_rg_dev_kfree_skb_any(skb);
		}
		else
		{
			IGMP("IGMP_TX_O[%x]: strange return value %d....\n",(POINTER_CAST)skb&0xffff,rg_fwdengine_ret_code);
		}


	}
	//rg_kernel.rxInfoFromIGMPMLD.rx_cvlan_tag	= tmp_intf_vlan_id;

	rg_db.pktHdr=&rg_db_cache.pktHeader_1;
}

static unsigned short rtl_ipv6Checksum(unsigned char *pktBuf, unsigned int pktLen, unsigned char  *ipv6PseudoHdrBuf)
{
	unsigned int  checksum=0;
	unsigned int count=pktLen;
	unsigned short   *ptr;

	/*compute ipv6 pseudo-header checksum*/
	ptr= (unsigned short  *) (ipv6PseudoHdrBuf);
	for(count=0; count<20; count++) /*the pseudo header is 40 bytes long*/
	{
		checksum+= ntohs(*ptr);
		ptr++;
	}

	/*compute the checksum of mld buffer*/
	count=pktLen;
	ptr=(unsigned short  *) (pktBuf);
	while(count>1)
	{
		checksum+= ntohs(*ptr);
		ptr++;
		count -= 2;
	}

	if(count>0)
	{
		checksum+= *(pktBuf+pktLen-1)<<8; /*the last odd byte is treated as bit 15~8 of unsigned short*/
	}

	/* Roll over carry bits */
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	/* Return checksum */
	return ((uint16) ~ checksum);
}


static unsigned char* rtl_generateMldQuery(char *src_mac)
{

	unsigned short checkSum=0;

	memcpy(&mldQueryBuf[6],src_mac,6);			/*set source mac address*/

	memcpy(&mldQueryBuf[30],src_mac,3);		/*set  mld query packet source ip address*/
	mldQueryBuf[30]=mldQueryBuf[30]|0x02;
	memcpy(&mldQueryBuf[35],&src_mac[3],3);


	memcpy(ipv6PseudoHdrBuf,&mldQueryBuf[22],16);			/*set pseudo-header source ip*/

	mldQueryBuf[64]=0;/*reset checksum*/
	mldQueryBuf[65]=0;
	checkSum=rtl_ipv6Checksum(&mldQueryBuf[62],24,ipv6PseudoHdrBuf);

	mldQueryBuf[64]=(checkSum&0xff00)>>8;
	mldQueryBuf[65]=(checkSum&0x00ff);
	return mldQueryBuf;
}


void rtl_mldQueryTimerExpired(unsigned long arg)
{
	struct sk_buff *skb;
	unsigned char *mldBuf=NULL;
	int i,rg_fwdengine_ret_code;

	if (_rtk_rg_get_initState()!=RTK_RG_INIT_FINISHED)
		return;

	rg_db.pktHdr=&rg_db.systemGlobal.pktHeader_2;

	/**send packet***/
	for(i=0;i<rg_db.systemGlobal.lanIntfTotalNum;i++)
	{

		skb=(struct sk_buff *)_rtk_rg_getAlloc(RG_FWDENGINE_PKT_LEN);

		//call fwdEngineInput, the alloc counter will be added. so don't need to add again

		if((skb)&&(rg_db.systemGlobal.fwdStatistic))
		{
#if RTK_RG_SKB_PREALLOCATE
			rg_db.systemGlobal.statistic.perPortCnt_skb_pre_alloc_for_uc[rg_db.pktHdr->ingressPort]--;
#else
			rg_db.systemGlobal.statistic.perPortCnt_skb_alloc[rg_db.pktHdr->ingressPort]--;
#endif
		}

		if(skb==NULL)
			return;


		bzero((void*)skb->data,86);
		mldBuf=rtl_generateMldQuery(rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet);
		if(mldBuf==NULL)
			return;

		memcpy(skb->data,mldBuf,86);
		skb->len = 0;
		skb_put(skb, 86);


		rg_kernel.rxInfoFromIGMPMLD.rx_cvlan_tag=rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->intf_vlan_id;

		rg_fwdengine_ret_code = rtk_rg_fwdEngineInput(skb, &rg_kernel.rxInfoFromIGMPMLD);

		//Processing packets
		if(rg_fwdengine_ret_code == RG_FWDENGINE_RET_TO_PS)
		{
			//FIXME:iPhone 5 change wireless connection from master to slave will send strange unicast ARP request for LAN gateway IP, and forwarded by protocol stack
			IGMP("MLD_TX_O[%x]: To Protocol-Stack...FREE SKB!!\n",(POINTER_CAST)skb&0xffff);
			//dump_packet(skb->data,skb->len,"dump_back_to_PS");
			_rtk_rg_dev_kfree_skb_any(skb);
		}
		else if (rg_fwdengine_ret_code == RG_FWDENGINE_RET_DROP)
		{
			IGMP("MLD_TX_O[%x]: Drop...FREE SKB!!\n",(POINTER_CAST)skb&0xffff);
			_rtk_rg_dev_kfree_skb_any(skb);
		}
		else
		{
			IGMP("MLD_TX_O[%x]: strange return value %d....\n",(POINTER_CAST)skb&0xffff,rg_fwdengine_ret_code);
		}


	}

	rg_db.pktHdr=&rg_db_cache.pktHeader_1;
}
#endif

static unsigned int mCastQueryTimerCnt=0;

void rtl_mCastQueryTimerExpired(unsigned long arg)
{

#ifdef __KERNEL__
	IGMP("QueryTimerExpired\n");
	if (rg_db.systemGlobal.mcast_query_sec!=0) {
		_rtk_rg_mod_timer(&mCastQuerytimer, jiffies+rg_db.systemGlobal.mcast_query_sec*CONFIG_HZ);
	}

	last_query_jiffies = jiffies;
	if (rg_db.systemGlobal.multicastProtocol!=RG_MC_MLD_ONLY){//send IGMP query
		if (igmpSnoopingCounterVer_3>0){

			rtl_igmpQueryTimerExpired(arg, IGMP_V3, NULL, 0, NULL, NULL);

			IGMP("System send Query(IGMPv3), last_query_jiffies=%lu",last_query_jiffies);
		}
		if (igmpSnoopingCounterVer_2>0){

			rtl_igmpQueryTimerExpired(arg, IGMP_V2, NULL, 0, NULL, NULL);

			IGMP("System send Query(IGMPv2), last_query_jiffies=%lu",last_query_jiffies);
		}
		if (igmpSnoopingCounterVer_1>0){ ////++

			rtl_igmpQueryTimerExpired(arg, IGMP_V1, NULL, 0, NULL, NULL);

			IGMP("System send Query(IGMPv1), last_query_jiffies=%lu",last_query_jiffies);
		}
	}

	if (rg_db.systemGlobal.multicastProtocol!=RG_MC_IGMP_ONLY){//send MLD query
		if (MLDCounterVer_1>0 || MLDCounterVer_2>0){//we just consider MLDv1 and MLv2 as one type now.
			rtl_mldQueryTimerExpired(arg);
			IGMP("System send Query(MLDQuery), last_query_jiffies=%lu",last_query_jiffies);
		}
	}

	//if forceReportResponseTime enabled(!=0), the report must received in this peroid of time, or the client should be delete.
	if(rg_db.systemGlobal.forceReportResponseTime!=0){
#ifdef __KERNEL__
		_rtk_rg_del_timer(&mCastForceReporttimer);
		_rtk_rg_init_timer(&mCastForceReporttimer);
		mCastForceReporttimer.data = SYS_EXPIRED_USER_FORCE;
		mCastForceReporttimer.expires = jiffies+rg_db.systemGlobal.forceReportResponseTime*CONFIG_HZ;
		mCastForceReporttimer.function = (void*)rtl_multicastSysTimerExpired;
		_rtk_rg_mod_timer(&mCastForceReporttimer, jiffies+rg_db.systemGlobal.forceReportResponseTime*CONFIG_HZ);
		IGMP("Trigger a force report timer in %d sec (current jiffies is %d)",rg_db.systemGlobal.forceReportResponseTime,jiffies);
#endif
	}

	mCastQueryTimerCnt++;
#endif


	return;
}


#if defined (CONFIG_RG_HARDWARE_MULTICAST)
rtl_multicast_index_mapping_t * _rtk_hwMappingEntry_malloc(void)
{
	rtl_multicast_index_mapping_t *mapping_entry;
	rtl_multicast_index_mapping_t *mapping_entry_tmp;

	if(list_empty(&hwMappingEntryFreeListHead))
	{
		WARNING("all free IGMP hw mapping list are allocated...");
			return (NULL);
	}

	//IGMP("get a free Entry for hardware mapping entry \n");
	//Get one from free list
	list_for_each_entry_safe(mapping_entry,mapping_entry_tmp,&hwMappingEntryFreeListHead,entry)		//just return the first entry right behind of head
	{
		list_del_init(&mapping_entry->entry);
		break;
	}
	memset(mapping_entry,0,sizeof(rtl_multicast_index_mapping_t));
	list_add(&mapping_entry->entry, &rtl_multicast_index_mapping_listhead);
	return mapping_entry;

}

int _rtk_hwMappingEntry_free(rtl_multicast_index_mapping_t* mapping_entry)
{

	list_del_init(&mapping_entry->entry);
	list_add(&mapping_entry->entry, &hwMappingEntryFreeListHead);
	//IGMP("free hardware mapping entry  !!\n");
	return (RT_ERR_RG_OK);
}


int _rtk_hwMappingEntry_flashInit(void)
{
	int i;
	INIT_LIST_HEAD(&rtl_multicast_index_mapping_listhead);
	INIT_LIST_HEAD(&hwMappingEntryFreeListHead);
	for(i=0 ; i<DEFAULT_MAX_HW_MAPPING_COUNT;i++ )
	{
		memset(&hwMappingEntry[i],0,sizeof(rtl_multicast_index_mapping_t));
		INIT_LIST_HEAD(&hwMappingEntry[i].entry);
		list_add_tail(&hwMappingEntry[i].entry, &hwMappingEntryFreeListHead);
	}
	//IGMP("flash hardware mapping entry  !!\n");
	return (RT_ERR_RG_OK);
}
#endif



int32 rtl_initMulticastSnooping(struct rtl_mCastSnoopingGlobalConfig mCastSnoopingGlobalConfig)
{
	int i,j;
	uint32 maxHashTableSize=MAX_HASH_TABLE_SIZE;

	#ifdef __KERNEL__
	spin_lock_init(&rg_kernel.igmpsnoopingLock);
	#endif

	#ifdef CONFIG_RG_HARDWARE_MULTICAST
	_rtk_hwMappingEntry_flashInit();
	#endif

	for (i=0; i<MAX_MCAST_MODULE_NUM; i++)
	{
		memset(&(rtl_mCastModuleArray[i]), 0, sizeof(struct rtl_multicastModule));

		for (j=0; j<6; j++)
		{
			rtl_mCastModuleArray[i].rtl_gatewayMac[j]=0;
		}

		rtl_mCastModuleArray[i].rtl_gatewayIpv4Addr=0;
		rtl_mCastModuleArray[i].rtl_ipv4HashTable=NULL;

		for (j=0; j<4; j++)
		{
			rtl_mCastModuleArray[i].rtl_gatewayIpv6Addr[j]=0;
		}
		rtl_mCastModuleArray[i].rtl_ipv6HashTable=NULL;

		#ifdef CONFIG_RECORD_MCAST_FLOW
		rtl_mCastModuleArray[i].flowHashTable=NULL;
		#endif
		rtl_mCastModuleArray[i].enableSnooping=FALSE;
		rtl_mCastModuleArray[i].enableFastLeave=FALSE;
	}

	/*set multicast snooping parameters, use default value*/
	if(mCastSnoopingGlobalConfig.groupMemberAgingTime==0)
	{
		rtl_mCastTimerParas.groupMemberAgingTime= DEFAULT_GROUP_MEMBER_INTERVAL;
	}
	else
	{
		rtl_mCastTimerParas.groupMemberAgingTime= mCastSnoopingGlobalConfig.groupMemberAgingTime;
	}

	if(mCastSnoopingGlobalConfig.lastMemberAgingTime==0)
	{
		rtl_mCastTimerParas.lastMemberAgingTime= DEFAULT_LAST_MEMBER_INTERVAL;
	}
	else
	{
		rtl_mCastTimerParas.lastMemberAgingTime= mCastSnoopingGlobalConfig.lastMemberAgingTime;
	}

	if(mCastSnoopingGlobalConfig.querierPresentInterval==0)
	{
		rtl_mCastTimerParas.querierPresentInterval= DEFAULT_QUERIER_PRESENT_TIMEOUT;
	}
	else
	{
		rtl_mCastTimerParas.querierPresentInterval=mCastSnoopingGlobalConfig.querierPresentInterval;
	}

	if(mCastSnoopingGlobalConfig.dvmrpRouterAgingTime==0)
	{
		rtl_mCastTimerParas.dvmrpRouterAgingTime=DEFAULT_DVMRP_AGING_TIME;
	}
	else
	{
		rtl_mCastTimerParas.dvmrpRouterAgingTime=mCastSnoopingGlobalConfig.dvmrpRouterAgingTime;
	}

	if(mCastSnoopingGlobalConfig.mospfRouterAgingTime==0)
	{
		rtl_mCastTimerParas.mospfRouterAgingTime=DEFAULT_MOSPF_AGING_TIME;
	}
  	else
	{
		rtl_mCastTimerParas.mospfRouterAgingTime=mCastSnoopingGlobalConfig.mospfRouterAgingTime;
	}

	if(mCastSnoopingGlobalConfig.pimRouterAgingTime==0)
	{
		rtl_mCastTimerParas.pimRouterAgingTime=DEFAULT_PIM_AGING_TIME;
	}
	else
	{
		rtl_mCastTimerParas.pimRouterAgingTime=mCastSnoopingGlobalConfig.pimRouterAgingTime;
	}

	/* set hash table size and hash mask*/
	if(mCastSnoopingGlobalConfig.hashTableSize==0)
	{
		rtl_hashTableSize=DEFAULT_HASH_TABLE_SIZE;   /*default hash table size*/
	}
	else
	{
  	        for(i=0;i<11;i++)
		{
			if(mCastSnoopingGlobalConfig.hashTableSize>=maxHashTableSize)
			{
			      rtl_hashTableSize=maxHashTableSize;
			      break;
			}
			maxHashTableSize=maxHashTableSize>>1;

		}
	}
	rtl_hashMask=rtl_hashTableSize-1;


	rtl_groupMemory=NULL;
	rtl_clientMemory=NULL;
	rtl_sourceMemory=NULL;
	rtl_mcastFlowMemory=NULL;

	/*initialize group entry pool*/
	if(mCastSnoopingGlobalConfig.maxGroupNum==0)
	{
		rtl_totalMaxGroupCnt=DEFAULT_MAX_GROUP_COUNT;
	}
	else
	{
		rtl_totalMaxGroupCnt=mCastSnoopingGlobalConfig.maxGroupNum;
	}

	rtl_groupEntryPool=rtl_initGroupEntryPool(rtl_totalMaxGroupCnt);
	if(rtl_groupEntryPool==NULL)
	{
		return FAIL;
	}

	/*initialize client entry pool*/
	if(mCastSnoopingGlobalConfig.maxClientNum==0)
	{
		rtl_totalMaxClientCnt=DEFAULT_MAX_CLIENT_COUNT;
	}
	else
	{
		rtl_totalMaxClientCnt=mCastSnoopingGlobalConfig.maxClientNum;
	}

	rtl_clientEntryPool=rtl_initClientEntryPool(rtl_totalMaxClientCnt);
	if(rtl_clientEntryPool==NULL)
	{
		return FAIL;
	}
#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_mcastFlowEntryPool=rtl_initMcastFlowEntryPool(DEFAULT_MAX_FLOW_COUNT);
	if(rtl_mcastFlowEntryPool==NULL)
	{
		return FAIL;
	}
#endif
	/*initialize source entry pool*/
	if(mCastSnoopingGlobalConfig.maxSourceNum==0)
	{
		rtl_totalMaxSourceCnt=DEFAULT_MAX_SOURCE_COUNT;
	}
	else
	{
		rtl_totalMaxSourceCnt=mCastSnoopingGlobalConfig.maxSourceNum;
	}

	rtl_sourceEntryPool=rtl_initSourceEntryPool(rtl_totalMaxSourceCnt);
	if(rtl_sourceEntryPool==NULL)
	{
		rtl_totalMaxSourceCnt=0;
		return FAIL;
	}


#if defined(__linux__) && defined(__KERNEL__)
	rtl_multicastSysTimerInit();
	if(rg_db.systemGlobal.mcast_query_sec!=0)
	{
		_rtk_rg_init_timer(&mCastQuerytimer);
		//if(rg_db.systemGlobal.mcast_query_sec==0) rg_db.systemGlobal.mcast_query_sec=RTK_RG_DEFAULT_MCAST_QUERY_INTERVAL;
		mCastQuerytimer.expires=jiffies+rg_db.systemGlobal.mcast_query_sec*CONFIG_HZ;
		mCastQuerytimer.function=(void*)rtl_mCastQueryTimerExpired;
	}
#endif
	return SUCCESS;
}


int32 rtl_flushAllIgmpRecord(int forceFlush)
{
	/* maintain current time */
	uint32 i=0;
	struct rtl_groupEntry* groupEntryPtr=NULL;
	struct rtl_groupEntry* nextEntry=NULL;

	uint32 moduleIndex;

	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM; moduleIndex++)
	{
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{
			/*maintain ipv4 group entry  timer */
			//if ( !igmpsnoopenabled)

			if(forceFlush || (rg_db.systemGlobal.initParam.igmpSnoopingEnable==0) || (rg_db.systemGlobal.multicastProtocol==RG_MC_MLD_ONLY))
			{
				for(i=0; i<rtl_hashTableSize; i++)
				{
					/*scan the hash table*/
					if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!=NULL)
					{
						timerEventContext.ipVersion=IP_VERSION4;
						groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i];
						while(groupEntryPtr)              /*traverse each group list*/
						{
							nextEntry=groupEntryPtr->next;
							timerEventContext.vlanId=groupEntryPtr->vlanId;
							timerEventContext.groupAddr[0]=groupEntryPtr->groupAddr[0];
							timerEventContext.groupAddr[1]=0;
							timerEventContext.groupAddr[2]=0;
							timerEventContext.groupAddr[3]=0;
							rtl_deleteGroupEntry(groupEntryPtr, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
							groupEntryPtr=nextEntry;
						}
					}
				}
				igmpSnoopingCounterVer_1=0;
				igmpSnoopingCounterVer_2=0;
				igmpSnoopingCounterVer_3=0;
				for (i=IGMP_V1; i<=IGMP_V3; i++)
				{
					int j;
					for (j=0; j<RTK_RG_MAC_PORT_MAX; j++)
					{
						igmpSnoopingCounterVer_Port[j][i] = 0;
					}
					#ifdef CONFIG_MASTER_WLAN0_ENABLE
					for (j=0; j<MAX_WLAN_DEVICE_NUM; j++)
					{
						igmpSnoopingCounterVer_MBSSID[j][i]=0;
					}
					#endif
				}

			}

			/*maintain ipv6 group entry  timer */
			//if ( !mldSnoopEnabled)
			if(forceFlush || (rg_db.systemGlobal.initParam.igmpSnoopingEnable==0) || (rg_db.systemGlobal.multicastProtocol==RG_MC_IGMP_ONLY))
			{
				for(i=0; i<rtl_hashTableSize; i++)
				{
					/*scan the hash table*/
					if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable!=NULL)
					{
						timerEventContext.ipVersion=IP_VERSION6;
						groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[i];
						while(groupEntryPtr)              /*traverse each group list*/
						{
							nextEntry=groupEntryPtr->next;
							timerEventContext.vlanId=groupEntryPtr->vlanId;
							timerEventContext.groupAddr[0]=groupEntryPtr->groupAddr[0];
							timerEventContext.groupAddr[1]=groupEntryPtr->groupAddr[1];
							timerEventContext.groupAddr[2]=groupEntryPtr->groupAddr[2];
							timerEventContext.groupAddr[3]=groupEntryPtr->groupAddr[3];
							rtl_deleteGroupEntry(groupEntryPtr, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
							groupEntryPtr=nextEntry;
						}
					}
				}
				MLDCounterVer_1=0;
				MLDCounterVer_2=0;
				for (i=MLD_V1; i<=MLD_V2; i++)
				{
					int j;
					for (j=0; j<RTK_RG_MAC_PORT_MAX; j++)
					{
						igmpSnoopingCounterVer_Port[j][i] = 0;
					}
					#ifdef CONFIG_MASTER_WLAN0_ENABLE
					for (j=0; j<MAX_WLAN_DEVICE_NUM; j++)
					{
						igmpSnoopingCounterVer_MBSSID[j][i]=0;
					}
					#endif
				}
			}


		}
	}
	#ifdef CONFIG_RG_HARDWARE_MULTICAST
	_rtk_hwMappingEntry_flashInit();
	#endif



	return SUCCESS;
}

static inline uint32 rtl_igmpHashAlgorithm(uint32 ipVersion,uint32 *groupAddr)
{
	uint32 hashIndex=0;

	if (ipVersion==IP_VERSION4)
	{
		/*to do:change hash algorithm*/
		hashIndex=rtl_hashMask&groupAddr[0];
	}
	else
	{
		hashIndex=rtl_hashMask&groupAddr[3];
	}

	return hashIndex;
}

int32 rtl_exitMulticastSnooping(void)
{

	uint32 moduleIndex;
	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM; moduleIndex++)
	{
		rtl_unregisterIgmpSnoopingModule(moduleIndex);
	}

	rtl_hashTableSize=0;
	rtl_hashMask=0;
	memset(&rtl_mCastTimerParas,0,sizeof(struct rtl_mCastTimerParameters));

	if(rtl_groupMemory!=NULL)
	{
		rtk_rg_free(rtl_groupMemory);
	}

	rtl_totalMaxGroupCnt=0;
	rtl_groupMemory=NULL;
	rtl_groupEntryPool=NULL;

	if(rtl_clientMemory!=NULL)
	{
		rtk_rg_free(rtl_clientMemory);
	}

	rtl_totalMaxClientCnt=0;
	rtl_clientMemory=NULL;
	rtl_clientEntryPool=NULL;

	if(rtl_sourceMemory!=NULL)
	{
		rtk_rg_free(rtl_sourceMemory);
	}

	rtl_totalMaxSourceCnt=0;
	rtl_sourceMemory=NULL;
	rtl_sourceEntryPool=NULL;

#if defined(__linux__) && defined(__KERNEL__)
	rtl_multicastSysTimerDestroy();
#endif

	return SUCCESS;
}

/*group entry memory management*/
static  struct rtl_groupEntry* rtl_initGroupEntryPool(uint32 poolSize)
{
	uint32 idx=0;
	struct rtl_groupEntry *poolHead=NULL;
	struct rtl_groupEntry *entryPtr=NULL;
	rg_lock(&rg_kernel.igmpsnoopingLock);	/* Lock resource */
	if (poolSize != DEFAULT_MAX_GROUP_COUNT)
	{
		goto out;
	}

	/* Allocate memory */
	//poolHead = (struct rtl_groupEntry *)rtk_rg_malloc(sizeof(struct rtl_groupEntry) * DEFAULT_MAX_GROUP_COUNT);
	poolHead = &groupEntryPool[0];
	rtl_groupMemory=(void *)poolHead;

	memset(poolHead, 0,  (DEFAULT_MAX_GROUP_COUNT  * sizeof(struct rtl_groupEntry)));
	entryPtr = poolHead;

	/* link the whole group entry pool */
	for (idx = 0 ; idx < DEFAULT_MAX_GROUP_COUNT ; idx++, entryPtr++)
	{
		if(idx==0)
		{
			entryPtr->previous=NULL;
			if(DEFAULT_MAX_GROUP_COUNT == 1) //poolSize is 1
			{
				entryPtr->next=NULL;
			}
			else
			{
				entryPtr->next = entryPtr + 1;
			}
		}
		else
		{
			entryPtr->previous=entryPtr-1;
			if (idx == (DEFAULT_MAX_GROUP_COUNT - 1))
			{
				entryPtr->next = NULL;
			}
			else
			{
				entryPtr->next = entryPtr + 1;
			}
		}
	}


out:
	rg_unlock(&rg_kernel.igmpsnoopingLock);	/* UnLock resource */
	return poolHead;
}

// allocate a group entry from the group entry pool
static  struct rtl_groupEntry* rtl_allocateGroupEntry(void)
{
	struct rtl_groupEntry *ret = NULL;

	rg_lock(&rg_kernel.igmpsnoopingLock);
		if (rtl_groupEntryPool!=NULL)
		{
			ret = rtl_groupEntryPool;
#if !defined(CONFIG_RG_G3_SERIES)
			if((((uint32 )ret)& ((uint32 )0x80000000)) !=0x80000000)
			{
				WARNING("rtl_groupEntry Get Bad Address %p",ret);
				return NULL;
			}
#endif
			if(rtl_groupEntryPool->next!=NULL)
			{
#if !defined(CONFIG_RG_G3_SERIES)
				if(((((uint32 )ret->next)& ((uint32 )0x80000000)) !=0x80000000) || ((((uint32 )rtl_groupEntryPool->next->previous)& ((uint32 )0x80000000)) !=0x80000000))
				{
					WARNING("rtl_groupEntry Get Bad Address %p %p",ret->next,rtl_groupEntryPool->next->previous);
					return NULL;
				}
#endif
				rtl_groupEntryPool->next->previous=NULL;
			}
			rtl_groupEntryPool = rtl_groupEntryPool->next;
			memset(ret, 0, sizeof(struct rtl_groupEntry));
		}

	rg_unlock(&rg_kernel.igmpsnoopingLock);

	return ret;
}

// free a group entry and link it back to the group entry pool, default is link to the pool head
static  void rtl_freeGroupEntry(struct rtl_groupEntry* groupEntryPtr)
{
	if (!groupEntryPtr)
	{
		return;
	}

	rg_lock(&rg_kernel.igmpsnoopingLock);
		groupEntryPtr->next = rtl_groupEntryPool;
		if(rtl_groupEntryPool!=NULL)
		{
			rtl_groupEntryPool->previous=groupEntryPtr;
		}
		rtl_groupEntryPool=groupEntryPtr;
	rg_unlock(&rg_kernel.igmpsnoopingLock);
}

/*client entry memory management*/
static  struct rtl_clientEntry* rtl_initClientEntryPool(uint32 poolSize)
{

	uint32 idx=0;
	struct rtl_clientEntry *poolHead=NULL;
	struct rtl_clientEntry *entryPtr=NULL;
	rg_lock(&rg_kernel.igmpsnoopingLock);	/* Lock resource */
	if (poolSize != DEFAULT_MAX_CLIENT_COUNT)
	{
		goto out;
	}

	/* Allocate memory */
	//poolHead = (struct rtl_clientEntry *)rtk_rg_malloc(sizeof(struct rtl_clientEntry) * DEFAULT_MAX_CLIENT_COUNT);
	poolHead = &clientEntryPool[0];
	rtl_clientMemory=(void *)poolHead;

	if (poolHead != NULL)
	{
		memset(poolHead, 0,  (DEFAULT_MAX_CLIENT_COUNT  * sizeof(struct rtl_clientEntry)));
		entryPtr = poolHead;

		/* link the whole group entry pool */
		for (idx = 0 ; idx < DEFAULT_MAX_CLIENT_COUNT ; idx++, entryPtr++)
		{
			if(idx==0)
			{
				entryPtr->previous=NULL;
				if(DEFAULT_MAX_CLIENT_COUNT == 1)
				{
					entryPtr->next=NULL;
				}
				else
				{
					entryPtr->next = entryPtr + 1;
				}
			}
			else
			{
				entryPtr->previous=entryPtr-1;
				if (idx == (DEFAULT_MAX_CLIENT_COUNT - 1))
				{
					entryPtr->next = NULL;
				}
				else
				{
					entryPtr->next = entryPtr + 1;
				}
			}
		}
	}

out:

	rg_unlock(&rg_kernel.igmpsnoopingLock);	/* UnLock resource */
	return poolHead;

}


// allocate a client entry from the client entry pool
static  struct rtl_clientEntry* rtl_allocateClientEntry(void)
{
	struct rtl_clientEntry *ret = NULL;

	rg_lock(&rg_kernel.igmpsnoopingLock);
	if (rtl_clientEntryPool!=NULL)
	{
		ret = rtl_clientEntryPool;
#if !defined(CONFIG_RG_G3_SERIES)
		if((((uint32 )ret)& ((uint32 )0x80000000)) !=0x80000000)
		{
			WARNING("rtl_clientEntry Get Bad Address %p",ret);
			return NULL;
		}
#endif
		if(rtl_clientEntryPool->next!=NULL)
		{
#if !defined(CONFIG_RG_G3_SERIES)
			if(((((uint32 )ret->next)& ((uint32 )0x80000000)) !=0x80000000) || ((((uint32 )rtl_clientEntryPool->next->previous)& ((uint32 )0x80000000)) !=0x80000000))
			{
				WARNING("rtl_clientEntry Get Bad Address %p %p",ret->next,rtl_clientEntryPool->next->previous);
				return NULL;
			}
#endif
			rtl_clientEntryPool->next->previous=NULL;
		}
		rtl_clientEntryPool = rtl_clientEntryPool->next;
		memset(ret, 0, sizeof(struct rtl_clientEntry));
		ret->pppoeReservedAclIdx=FAIL;
	}

	rg_unlock(&rg_kernel.igmpsnoopingLock);

	return ret;
}

// free a client entry and link it back to the client entry pool, default is link to the pool head
static  void rtl_freeClientEntry(struct rtl_clientEntry* clientEntryPtr)
{
	if (!clientEntryPtr)
	{
		return;
	}

	rg_lock(&rg_kernel.igmpsnoopingLock);
#if defined(CONFIG_RG_RTL9600_SERIES)
	//delete reserved acl
	if(0 <= clientEntryPtr->pppoeReservedAclIdx  &&   clientEntryPtr->pppoeReservedAclIdx <= (RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE7_TRAP - RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE0_TRAP ))
	{
		_rtk_rg_aclAndCfReservedRuleDel(RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE0_TRAP +clientEntryPtr->pppoeReservedAclIdx );
		rg_db.systemGlobal.pppoe_passthrough_learning_reservedACL_bitmap &= (~(1<<clientEntryPtr->pppoeReservedAclIdx));
	}
#endif
	clientEntryPtr->next = rtl_clientEntryPool;
	if(rtl_clientEntryPool!=NULL)
	{
		rtl_clientEntryPool->previous=clientEntryPtr;
	}
	rtl_clientEntryPool=clientEntryPtr;
	rg_unlock(&rg_kernel.igmpsnoopingLock);
}

/*source entry memory management*/
static  struct rtl_sourceEntry* rtl_initSourceEntryPool(uint32 poolSize)
{
	uint32 idx=0;
	struct rtl_sourceEntry *poolHead=NULL;
	struct rtl_sourceEntry *entryPtr=NULL;
	rg_lock(&rg_kernel.igmpsnoopingLock);	/* Lock resource */
	if (poolSize != DEFAULT_MAX_SOURCE_COUNT)
	{
		goto out;
	}

	/* Allocate memory */
	//poolHead = (struct rtl_sourceEntry *)rtk_rg_malloc(sizeof(struct rtl_sourceEntry) * DEFAULT_MAX_SOURCE_COUNT);
	poolHead = &sourceEntryPool[0];
	rtl_sourceMemory=(void *)poolHead;
	if (poolHead != NULL)
	{
		memset(poolHead, 0,  (DEFAULT_MAX_SOURCE_COUNT  * sizeof(struct rtl_sourceEntry)));
		entryPtr = poolHead;

		/* link the whole source entry pool */
		for (idx = 0 ; idx < DEFAULT_MAX_SOURCE_COUNT ; idx++, entryPtr++)
		{
			if(idx==0)
			{
				entryPtr->previous=NULL;
				if(DEFAULT_MAX_SOURCE_COUNT == 1)
				{
					entryPtr->next=NULL;
				}
				else
				{
					entryPtr->next = entryPtr + 1;
				}
			}
			else
			{
				entryPtr->previous=entryPtr-1;
				if (idx == (DEFAULT_MAX_SOURCE_COUNT - 1))
				{
					entryPtr->next = NULL;
				}
				else
				{
					entryPtr->next = entryPtr + 1;
				}
			}

		}
	}

out:
	rg_unlock(&rg_kernel.igmpsnoopingLock);	/* UnLock resource */
	return poolHead;
}


// allocate a source entry from the source entry pool
static  struct rtl_sourceEntry* rtl_allocateSourceEntry(void)
{
	struct rtl_sourceEntry *ret = NULL;

	rg_lock(&rg_kernel.igmpsnoopingLock);
		if (rtl_sourceEntryPool!=NULL)
		{
			ret = rtl_sourceEntryPool;
#if !defined(CONFIG_RG_G3_SERIES)
			if((((uint32 )ret)& ((uint32 )0x80000000)) !=0x80000000)
			{
				WARNING("rtl_sourceEntry Get Bad Address %p",ret);
				return NULL;
			}
#endif
			if(rtl_sourceEntryPool->next!=NULL)
			{
#if !defined(CONFIG_RG_G3_SERIES)
				if(((((uint32 )ret->next)& ((uint32 )0x80000000)) !=0x80000000) || ((((uint32 )ret->next->previous)& ((uint32 )0x80000000)) !=0x80000000))
				{
					WARNING("rtl_sourceEntry Get Bad Address %p  %p",ret->next ,rtl_sourceEntryPool->next->previous);
					return NULL;
				}
#endif
				rtl_sourceEntryPool->next->previous=NULL;
			}
			rtl_sourceEntryPool = rtl_sourceEntryPool->next;
			memset(ret, 0, sizeof(struct rtl_sourceEntry));
		}

	rg_unlock(&rg_kernel.igmpsnoopingLock);

	return ret;
}

// free a source entry and link it back to the source entry pool, default is link to the pool head
static  void rtl_freeSourceEntry(struct rtl_sourceEntry* sourceEntryPtr)
{
	if (!sourceEntryPtr)
	{
		return;
	}

	rg_lock(&rg_kernel.igmpsnoopingLock);
		sourceEntryPtr->next = rtl_sourceEntryPool;
		if(rtl_sourceEntryPool!=NULL)
		{
			rtl_sourceEntryPool->previous=sourceEntryPtr;
		}

		rtl_sourceEntryPool=sourceEntryPtr;

	rg_unlock(&rg_kernel.igmpsnoopingLock);
}
#ifdef CONFIG_RECORD_MCAST_FLOW
/*multicast flow entry memory management*/
static  struct rtl_mcastFlowEntry* rtl_initMcastFlowEntryPool(uint32 poolSize)
{

	uint32 idx=0;
	struct rtl_mcastFlowEntry *poolHead=NULL;
	struct rtl_mcastFlowEntry *entryPtr=NULL;

	rg_lock(&rg_kernel.igmpsnoopingLock);	/* Lock resource */
	if (poolSize != DEFAULT_MAX_FLOW_COUNT)
	{
		goto out;
	}

	/* Allocate memory */
	//poolHead = (struct rtl_mcastFlowEntry *)rtk_rg_malloc(sizeof(struct rtl_mcastFlowEntry) * DEFAULT_MAX_FLOW_COUNT);
	poolHead = &mcastFlowEntryPool[0];
	rtl_mcastFlowMemory=(void *)poolHead;

	if (poolHead != NULL)
	{
		memset(poolHead, 0,  (DEFAULT_MAX_FLOW_COUNT  * sizeof(struct rtl_mcastFlowEntry)));
		entryPtr = poolHead;

		/* link the whole group entry pool */
		for (idx = 0 ; idx < DEFAULT_MAX_FLOW_COUNT ; idx++, entryPtr++)
		{
			if(idx==0)
			{
				entryPtr->previous=NULL;
				if(DEFAULT_MAX_FLOW_COUNT == 1)
				{
					entryPtr->next=NULL;
				}
				else
				{
					entryPtr->next = entryPtr + 1;
				}
			}
			else
			{
				entryPtr->previous=entryPtr-1;
				if (idx == (DEFAULT_MAX_FLOW_COUNT - 1))
				{
					entryPtr->next = NULL;
				}
				else
				{
					entryPtr->next = entryPtr + 1;
				}
			}
		}
	}

out:

	rg_unlock(&rg_kernel.igmpsnoopingLock);	/* UnLock resource */
	return poolHead;

}


// allocate a multicast flow entry  from the multicast flow pool
static  struct rtl_mcastFlowEntry* rtl_allocateMcastFlowEntry(void)
{
	struct rtl_mcastFlowEntry *ret = NULL;

	rg_lock(&rg_kernel.igmpsnoopingLock);
		if (rtl_mcastFlowEntryPool!=NULL)
		{
			ret = rtl_mcastFlowEntryPool;
#if !defined(CONFIG_RG_G3_SERIES)
			if((((uint32 )ret)& ((uint32 )0x80000000)) !=0x80000000)
			{
				WARNING("rtl_mcastFlowEntry Get Bad Address %p",ret);
				return NULL;
			}
#endif
			if(rtl_mcastFlowEntryPool->next!=NULL)
			{
#if !defined(CONFIG_RG_G3_SERIES)
				if(((((uint32 )ret->next)& ((uint32 )0x80000000)) !=0x80000000) || ((((uint32 )ret->next->previous)& ((uint32 )0x80000000)) !=0x80000000))
				{
					WARNING("rtl_mcastFlowEntry Get Bad Address %p  %p",ret->next,ret->next->previous);
					return NULL;
				}
#endif
				rtl_mcastFlowEntryPool->next->previous=NULL;
			}
			rtl_mcastFlowEntryPool = rtl_mcastFlowEntryPool->next;
			memset(ret, 0, sizeof(struct rtl_mcastFlowEntry));
#if  defined(CONFIG_RG_FLOW_BASED_PLATFORM)
			//init hw index
			ret->flowidx=-1;
#endif
		}

	rg_unlock(&rg_kernel.igmpsnoopingLock);

	return ret;
}

// free a multicast flow entry and link it back to the multicast flow entry pool, default is link to the pool head
static  void rtl_freeMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry)
{
	if (NULL==mcastFlowEntry)
	{
		return;
	}

	rg_lock(&rg_kernel.igmpsnoopingLock);
	mcastFlowEntry->next = rtl_mcastFlowEntryPool;
	if(rtl_mcastFlowEntryPool!=NULL)
	{
		rtl_mcastFlowEntryPool->previous=mcastFlowEntry;
	}
	rtl_mcastFlowEntryPool=mcastFlowEntry;
	rg_unlock(&rg_kernel.igmpsnoopingLock);
}

#endif

/*********************************************
			Group list operation
 *********************************************/

/*       find a group address in a group list    */

struct rtl_groupEntry* rtl_searchGroupEntry(uint32 moduleIndex, uint32 ipVersion,uint32 *multicastAddr, uint16 vlanId)
{
	struct rtl_groupEntry* groupPtr = NULL;
	int32 hashIndex=0;

	hashIndex=rtl_igmpHashAlgorithm(ipVersion, multicastAddr);

	if (ipVersion==IP_VERSION4)
	{
		groupPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];
	}
	else
	{
		groupPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[hashIndex];
	}

	while (groupPtr!=NULL)
	{
		if (ipVersion==IP_VERSION4)
		{

			if (multicastAddr[0]==groupPtr->groupAddr[0])
			{

				if (rg_db.systemGlobal.initParam.ivlMulticastSupport) //force path2
				{
					if (((rg_db.vlan[groupPtr->vlanId].fidMode==VLAN_FID_IVL)&&(groupPtr->vlanId!=vlanId))
					||((rg_db.vlan[groupPtr->vlanId].fidMode==VLAN_FID_SVL)&&(rg_db.vlan[groupPtr->vlanId].fid!=LAN_FID)))
					{
						goto NEXT;
					}
				}
				return groupPtr;

			}
		}
		else
		{
			if(	(multicastAddr[0]==groupPtr->groupAddr[0])&&
				(multicastAddr[1]==groupPtr->groupAddr[1])&&
				(multicastAddr[2]==groupPtr->groupAddr[2])&&
				(multicastAddr[3]==groupPtr->groupAddr[3])
			)
			{
				if (rg_db.systemGlobal.initParam.ivlMulticastSupport) //force path2
				{
					if (((rg_db.vlan[groupPtr->vlanId].fidMode==VLAN_FID_IVL)&&(groupPtr->vlanId!=vlanId))
					||((rg_db.vlan[groupPtr->vlanId].fidMode==VLAN_FID_SVL)&&(rg_db.vlan[groupPtr->vlanId].fid!=LAN_FID)))
					{
						goto NEXT;
					}
				}

				return groupPtr;
			}
		}

NEXT:
		groupPtr = groupPtr->next;

	}

	return NULL;
}

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
static rtk_rg_err_code_t _rtl_addGroup2hw(struct rtl_groupEntry* groupEntry)
{
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	{
		rtk_l2_mcastAddr_t mcastAddr;
		int32 mcL2Idx;
		memset(&mcastAddr,0,sizeof(mcastAddr));
		mcastAddr.portmask.bits[0]=0;
		mcastAddr.ext_portmask_idx=31;
		if(rg_db.systemGlobal.initParam.ivlMulticastSupport)
		{
			mcastAddr.flags= RTK_L2_MCAST_FLAG_IVL;
			mcastAddr.vid = groupEntry->vlanId ;
		}
		else
			mcastAddr.fid = LAN_FID;

		if(groupEntry->ipVersion==IP_VERSION4)
		{
			mcastAddr.mac.octet[0]=0x01;
			mcastAddr.mac.octet[1]=0x00;
			mcastAddr.mac.octet[2]=0x5e;
			mcastAddr.mac.octet[3]=(groupEntry->groupAddr[0]>>16)&0x7f;
			mcastAddr.mac.octet[4]=(groupEntry->groupAddr[0]>>8 )&0xff;
			mcastAddr.mac.octet[5]=(groupEntry->groupAddr[0]>>0 )&0xff;
		}
		else
		{
			mcastAddr.mac.octet[0]=0x33;
			mcastAddr.mac.octet[1]=0x33;
			mcastAddr.mac.octet[2]=(groupEntry->groupAddr[3]>>24)&0xff;
			mcastAddr.mac.octet[3]=(groupEntry->groupAddr[3]>>16)&0xff;
			mcastAddr.mac.octet[4]=(groupEntry->groupAddr[3]>>8 )&0xff;
			mcastAddr.mac.octet[5]=(groupEntry->groupAddr[3]>>0 )&0xff;

		}

		if(mcastAddr.flags&RTK_L2_MCAST_FLAG_IVL)
			mcL2Idx=_rtk_rg_findAndReclamL2mcEntryIVL(&mcastAddr.mac, mcastAddr.vid);
		else
			mcL2Idx=_rtk_rg_findAndReclamL2mcEntry(&mcastAddr.mac, mcastAddr.fid);

		ASSERT_EQ(RTK_L2_MCASTADDR_ADD(&mcastAddr),RT_ERR_OK);
		ASSERT_EQ(mcL2Idx,mcastAddr.index);
		rg_db.lut[mcL2Idx].mcStaticRefCnt++;
		TABLE("ADD Mc Lut  rg_db.lut[%d].mcStaticRefCnt=%d",mcL2Idx,rg_db.lut[mcL2Idx].mcStaticRefCnt);

		groupEntry->destLutIdx = mcastAddr.index ;
	}
#elif defined(CONFIG_RG_G3_SERIES)
	{
		int32 ret =FAIL;
		ca_l3_mcast_entry_t l3mcEnt;
		ca_l2_mcast_entry_t l2mcEnt;

		/*add IPv4 L3 MC group*/
		/*Add the egress interface of the flow's first packet as the L3 MC member. Otherwise, L2 member ports can not sync to L3 MC group */

		bzero(&l3mcEnt, sizeof(l3mcEnt));
		bzero(&l2mcEnt, sizeof(l2mcEnt));

		if(groupEntry->ipVersion==IP_VERSION4)
		{
			l3mcEnt.group_ip_addr.afi = CA_IPV4;
			l3mcEnt.group_ip_addr.ip_addr.ipv4_addr = groupEntry->groupAddr[0];
			l3mcEnt.group_ip_addr.addr_len = 32;

			l2mcEnt.group_ip_addr.afi = CA_IPV4;
			l2mcEnt.group_ip_addr.ip_addr.ipv4_addr = groupEntry->groupAddr[0];
			l2mcEnt.group_ip_addr.addr_len = 32;
		}
		else
		{
			l3mcEnt.group_ip_addr.afi = CA_IPV6;
			l3mcEnt.group_ip_addr.ip_addr.ipv6_addr[0] = ntohl(groupEntry->groupAddr[0]);
			l3mcEnt.group_ip_addr.ip_addr.ipv6_addr[1] = ntohl(groupEntry->groupAddr[1]);
			l3mcEnt.group_ip_addr.ip_addr.ipv6_addr[2] = ntohl(groupEntry->groupAddr[2]);
			l3mcEnt.group_ip_addr.ip_addr.ipv6_addr[3] = ntohl(groupEntry->groupAddr[3]);
			l3mcEnt.group_ip_addr.addr_len = 128;

			l2mcEnt.group_ip_addr.afi = CA_IPV6;
			l2mcEnt.group_ip_addr.ip_addr.ipv6_addr[0] = ntohl(groupEntry->groupAddr[0]);
			l2mcEnt.group_ip_addr.ip_addr.ipv6_addr[1] = ntohl(groupEntry->groupAddr[1]);
			l2mcEnt.group_ip_addr.ip_addr.ipv6_addr[2] = ntohl(groupEntry->groupAddr[2]);
			l2mcEnt.group_ip_addr.ip_addr.ipv6_addr[3] = ntohl(groupEntry->groupAddr[3]);
			l2mcEnt.group_ip_addr.addr_len = 128;
		}

		ASSERT_EQ(ret = ca_l3_mcast_group_add(G3_DEF_DEVID, &l3mcEnt), CA_E_OK);
		TRACE("Add L3 mcast group idx %d, ret = %d", l3mcEnt.mcg_id, ret);

		/*add L2 MC group*/
		l2mcEnt.mcast_vlan = CA_UINT16_INVALID;
		ASSERT_EQ(ret = ca_l2_mcast_group_add(G3_DEF_DEVID, &l2mcEnt), CA_E_OK);
		TRACE("Add L2 mcast group idx %d, ret = %d", l2mcEnt.mcg_id, ret);

		// setting hardware group and new a g3McEngine
		groupEntry->pMcEngineInfo = _rtk_rg_g3McEngineInfoGet(groupEntry->groupAddr,NULL,groupEntry->ipVersion==IP_VERSION6?1:0,l2mcEnt.mcg_id,l3mcEnt.mcg_id);
		if(groupEntry->pMcEngineInfo==NULL) {DEBUG("pMcEngineInfo NULL"); return RT_ERR_RG_ENTRY_FULL;}
	}
#endif

	return RT_ERR_RG_OK;
}


int32 rtk_rg_GrpSetToTrap(uint32 *gourpAddress ,uint32 isIpv6)
{

#if defined(CONFIG_RG_G3_SERIES)

	struct rtl_groupEntry* groupEntry=NULL;
	int32 ipVersion=IP_VERSION4;
	if(isIpv6)
		ipVersion= IP_VERSION6;

	groupEntry=rtl_searchGroupEntry(rg_db.systemGlobal.nicIgmpModuleIndex, ipVersion, gourpAddress, 0);

	if(groupEntry)
	{
		if(groupEntry->pMcEngineInfo->mcHwRefCount==1)
		{
			int32 ret=_rtk_rg_g3McEngineInfoDel(groupEntry->pMcEngineInfo);
			if(ret)
				TABLE("Del pMcEngineInfo FAIL");
			_rtl_addGroup2hw(groupEntry);
			IGMP("Setting Group Trap to CPU Success");
		}
		else
		{
			IGMP("WARNING mcHwRefCount!=1  [%d]",groupEntry->pMcEngineInfo->mcHwRefCount);
		}
	}
	else
		IGMP("Group Not Found !");

#endif
	return SUCCESS;
}


#endif

/* link group Entry in the front of a group list */
static void  rtl_linkGroupEntry(struct rtl_groupEntry* groupEntry ,  struct rtl_groupEntry ** hashTable)
{
	uint32 hashIndex=0;

	if(NULL==groupEntry)
	{
		return;
	}

	rg_lock(&rg_kernel.igmpsnoopingLock);//Lock resource

	hashIndex=rtl_igmpHashAlgorithm(groupEntry->ipVersion, groupEntry->groupAddr);
	if(hashTable[hashIndex]!=NULL)
	{
		hashTable[hashIndex]->previous=groupEntry;
	}
	groupEntry->next = hashTable[hashIndex];
	hashTable[hashIndex]=groupEntry;
	hashTable[hashIndex]->previous=NULL;
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	//new flow add	new lut entry or add refcount first
	/* add	pmsk=0 ext_pmsk=31 lut entry, point to this multicast ref to flow entry */
	_rtl_addGroup2hw(groupEntry);
#endif
	rg_unlock(&rg_kernel.igmpsnoopingLock);//UnLock resource

}


/* unlink a group entry from group list */
static void rtl_unlinkGroupEntry(struct rtl_groupEntry* groupEntry,  struct rtl_groupEntry ** hashTable)
{
	uint32 hashIndex=0;

	if(NULL==groupEntry)
	{
		return;
	}

	rg_lock(&rg_kernel.igmpsnoopingLock);  /* lock resource*/

	hashIndex=rtl_igmpHashAlgorithm(groupEntry->ipVersion, groupEntry->groupAddr);
	/* unlink entry node*/
	if(groupEntry==hashTable[hashIndex]) /*unlink group list head*/
	{
		hashTable[hashIndex]=groupEntry->next;
		if(hashTable[hashIndex]!=NULL)
		{
			hashTable[hashIndex]->previous=NULL;
		}
	}
	else
	{
		if(groupEntry->previous!=NULL)
		{
			groupEntry->previous->next=groupEntry->next;
		}

		if(groupEntry->next!=NULL)
		{
			groupEntry->next->previous=groupEntry->previous;
		}
	}

	groupEntry->previous=NULL;
	groupEntry->next=NULL;
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	if(groupEntry->destLutIdx >=0 && groupEntry->destLutIdx<MAX_LUT_SW_TABLE_SIZE)
	{
		rg_db.lut[groupEntry->destLutIdx].mcStaticRefCnt--;
		TABLE("LutMc entry [%d] mcStaticRefCnt=%d",groupEntry->destLutIdx,rg_db.lut[groupEntry->destLutIdx].mcStaticRefCnt);
		if(rg_db.lut[groupEntry->destLutIdx].mcStaticRefCnt<=0)
		{
			RTK_L2_MCASTADDR_DEL(&(rg_db.lut[groupEntry->destLutIdx].rtk_lut.entry.l2McEntry));
			TABLE("Del LutMc entry [%d]",groupEntry->destLutIdx);
		}
	}
#elif defined(CONFIG_RG_G3_SERIES)
	{
		int32 ret=_rtk_rg_g3McEngineInfoDel(groupEntry->pMcEngineInfo);
		if(ret)
			TABLE("Del pMcEngineInfo FAIL");
	}
#endif
#endif


	rg_unlock(&rg_kernel.igmpsnoopingLock);//UnLock resource
}


/* clear the content of group entry */
static void rtl_clearGroupEntry(struct rtl_groupEntry* groupEntry)
{
	rg_lock(&rg_kernel.igmpsnoopingLock);
	if (NULL!=groupEntry)
	{
		memset(groupEntry, 0, sizeof(struct rtl_groupEntry));
	}
	rg_unlock(&rg_kernel.igmpsnoopingLock);
}

/*********************************************
			Client list operation
 *********************************************/
static struct rtl_clientEntry* rtl_searchClientEntry(uint32 ipVersion, struct rtl_groupEntry* groupEntry, uint32 portNum, uint32 *clientAddr)
{
	struct rtl_clientEntry* clientPtr = groupEntry->clientList;

	if(clientAddr==NULL)
	{
		return NULL;
	}
	while (clientPtr!=NULL)
	{
		if(ipVersion==IP_VERSION4)
		{
			if((clientPtr->clientAddr[0]==clientAddr[0]))
			{
				if( (clientPtr->portNum != portNum) && !RG_INVALID_PORT(portNum))
				{
					/*update port number,in case of client change port*/
					clientPtr->portNum=portNum;
#ifdef CONFIG_RECORD_MCAST_FLOW
					IGMP("Update Client Port form %d to %d",clientPtr->portNum ,portNum);
					rtl_rg_mcDataReFlushAndAdd(groupEntry->groupAddr);
					//rtl_invalidateMCastFlow(rg_db.systemGlobal.nicIgmpModuleIndex, ipVersion, groupEntry->groupAddr);
#endif
				}
				return clientPtr;
			}

		}
		else
		{
			if(	((clientPtr->clientAddr[0]==clientAddr[0])
				&&(clientPtr->clientAddr[1]==clientAddr[1])
				&&(clientPtr->clientAddr[2]==clientAddr[2])
				&&(clientPtr->clientAddr[3]==clientAddr[3])))
			{

				if((clientPtr->portNum != portNum) && !RG_INVALID_PORT(portNum))
				{
					/*update port number,in case of client change port*/
					clientPtr->portNum=portNum;
#ifdef CONFIG_RECORD_MCAST_FLOW
					IGMP("Update Client Port form %d to %d",clientPtr->portNum ,portNum);
					rtl_rg_mcDataReFlushAndAdd(groupEntry->groupAddr);
					//rtl_invalidateMCastFlow(rg_db.systemGlobal.nicIgmpModuleIndex, ipVersion, groupEntry->groupAddr);
#endif
				}
				return clientPtr;
			}
		}

		clientPtr = clientPtr->next;

	}

	return NULL;
}


/* link client Entry in the front of group client list */
static void  rtl_linkClientEntry(struct rtl_groupEntry *groupEntry, struct rtl_clientEntry* clientEntry )
{
	if(NULL==clientEntry)
	{
		return;
	}

	if(NULL==groupEntry)
	{
		return;
	}

	rg_lock(&rg_kernel.igmpsnoopingLock);//Lock resource

#ifdef CONFIG_RG_SIMPLE_IGMP_v3
	if ((groupEntry->groupAddr[0] != 0xEFFFFFFA) && //239.255.255.250
		((groupEntry->groupAddr[0] & 0xFFFFFF00) != 0xE0000000) &&//224.0.0/24
		((groupEntry->groupAddr[0] & 0xFFFFFF00) != 0xE0000100))//224.0.1/24
		goto ADD_CLIENT_LINK;
#endif

	if(_check_isDummyClient(clientEntry))
		goto ADD_CLIENT_LINK;

	switch(clientEntry->igmpVersion)
	{
		case IGMP_V1:	////++
			igmpSnoopingCounterVer_1++;

			break;
		case IGMP_V2:
			igmpSnoopingCounterVer_2++;
			break;
		case IGMP_V3:
			igmpSnoopingCounterVer_3++;
			break;
		case MLD_V1:
			MLDCounterVer_1++;
			break;
		case MLD_V2:
			MLDCounterVer_2++;
			break;
		default:
			break;
	}

#ifdef CONFIG_MASTER_WLAN0_ENABLE
	if ( (1<< clientEntry->portNum)& RTK_RG_ALL_REAL_MASTER_EXT_PORTMASK )
	{
		rtk_rg_mac_port_idx_t mac_cpu_port=RTK_RG_MAC_PORT_MAINCPU;
#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		mac_cpu_port = RTK_RG_MAC_PORT_CPU;
#elif defined(CONFIG_RG_RTL9607C_SERIES)
		if(RTK_RG_EXT_PORT0<=clientEntry->portNum && clientEntry->portNum<=RTK_RG_EXT_PORT5)
			mac_cpu_port = RTK_RG_MAC_PORT_MASTERCPU_CORE0;
		else if(RTK_RG_MAC10_EXT_PORT0<=clientEntry->portNum && clientEntry->portNum<=RTK_RG_MAC10_EXT_PORT5)
			mac_cpu_port = RTK_RG_MAC_PORT_MASTERCPU_CORE1;
#ifdef CONFIG_DUALBAND_CONCURRENT
		else if(RTK_RG_MAC7_EXT_PORT0<=clientEntry->portNum && clientEntry->portNum<=RTK_RG_MAC7_EXT_PORT5)
			mac_cpu_port = RTK_RG_MAC_PORT_SLAVECPU;
#endif
#else
#error
#endif
		igmpSnoopingCounterVer_Port[mac_cpu_port][clientEntry->igmpVersion - IGMP_V1]++;
		if(clientEntry->wlan_dev_idx >=0 && clientEntry->wlan_dev_idx<MAX_WLAN_DEVICE_NUM)
			igmpSnoopingCounterVer_MBSSID[clientEntry->wlan_dev_idx][clientEntry->igmpVersion - IGMP_V1]++;

		IGMP("Add igmpSnoopingCounterVer_Port[%d][%d]", mac_cpu_port, clientEntry->igmpVersion - IGMP_V1);
		IGMP("Add igmpSnoopingCounterVer_MBSSID[%d][%d]", clientEntry->wlan_dev_idx, clientEntry->igmpVersion - IGMP_V1);
	}
	else
#endif
	{
		/* for coverity isuue we just want to access igmpSnoopingCounterVer_Port by clientEntry->portNum mac_port */
		if((int)RTK_RG_MAC_PORT0 <= clientEntry->portNum && clientEntry->portNum < (int)RTK_RG_MAC_PORT_MAX)
		{
			rtk_rg_mac_port_idx_t _portNum = (rtk_rg_mac_port_idx_t)clientEntry->portNum;
			if((int)RTK_RG_MAC_PORT0 <= _portNum && _portNum < (int)RTK_RG_MAC_PORT_MAX)
			{
				igmpSnoopingCounterVer_Port[_portNum][clientEntry->igmpVersion - IGMP_V1]++;
				IGMP("Add gmpSnoopingCounterVer_Port[%d][%d]", _portNum, clientEntry->igmpVersion - IGMP_V1);
			}
		}
	}
		


ADD_CLIENT_LINK:

	if(groupEntry->clientList!=NULL)
	{
		groupEntry->clientList->previous=clientEntry;
	}
	clientEntry->next = groupEntry->clientList;

	groupEntry->clientList=clientEntry;
	groupEntry->clientList->previous=NULL;

	rg_unlock(&rg_kernel.igmpsnoopingLock);//UnLock resource

}


/* unlink a client entry from group client list */
static void rtl_unlinkClientEntry(struct rtl_groupEntry *groupEntry, struct rtl_clientEntry* clientEntry)
{
	//IGMP("unlink %pI4",&clientEntry->clientAddr);
	if(NULL==clientEntry)
	{
		return;
	}

	if(NULL==groupEntry)
	{
		return;
	}

	rg_lock(&rg_kernel.igmpsnoopingLock);  /* lock resource*/

	/* unlink entry node*/
	if(clientEntry==groupEntry->clientList) /*unlink group list head*/
	{
		groupEntry->clientList=groupEntry->clientList->next;
		if(groupEntry->clientList!=NULL)
		{
			groupEntry->clientList->previous=NULL;
		}

	}
	else
	{
		if(clientEntry->previous!=NULL)
		{
			clientEntry->previous->next=clientEntry->next;
		}

		if(clientEntry->next!=NULL)
		{
			clientEntry->next->previous=clientEntry->previous;
		}
	}

	clientEntry->previous=NULL;
	clientEntry->next=NULL;

#ifdef CONFIG_RG_SIMPLE_IGMP_v3
	if ((groupEntry->groupAddr[0] != 0xEFFFFFFA) && //239.255.255.250
		((groupEntry->groupAddr[0] & 0xFFFFFF00) != 0xE0000000) &&//224.0.0/24
		((groupEntry->groupAddr[0] & 0xFFFFFF00) != 0xE0000100))//224.0.1/24
		goto DEL_CLIENT_UNLINK;
#endif

	if(_check_isDummyClient(clientEntry))
		goto DEL_CLIENT_UNLINK;

	{
		switch(clientEntry->igmpVersion)
		{
			case IGMP_V1:	////++
				igmpSnoopingCounterVer_1--;
				break;
			case IGMP_V2:
				igmpSnoopingCounterVer_2--;
				break;
			case IGMP_V3:
				igmpSnoopingCounterVer_3--;
				break;
			case MLD_V1:
				MLDCounterVer_1--;
				break;
			case MLD_V2:
				MLDCounterVer_2--;
				break;
			default:
				break;
		}

		#ifdef CONFIG_MASTER_WLAN0_ENABLE
		if ( (1<< clientEntry->portNum)& RTK_RG_ALL_REAL_MASTER_EXT_PORTMASK )
		{
			rtk_rg_mac_port_idx_t mac_cpu_port=RTK_RG_MAC_PORT_MAINCPU;
#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
			mac_cpu_port = RTK_RG_MAC_PORT_CPU;
#elif defined(CONFIG_RG_RTL9607C_SERIES)
			if(RTK_RG_EXT_PORT0<=clientEntry->portNum && clientEntry->portNum<=RTK_RG_EXT_PORT5)
				mac_cpu_port = RTK_RG_MAC_PORT_MASTERCPU_CORE0;
			else if(RTK_RG_MAC10_EXT_PORT0<=clientEntry->portNum && clientEntry->portNum<=RTK_RG_MAC10_EXT_PORT5)
				mac_cpu_port = RTK_RG_MAC_PORT_MASTERCPU_CORE1;
#ifdef CONFIG_DUALBAND_CONCURRENT
			else if(RTK_RG_MAC7_EXT_PORT0<=clientEntry->portNum && clientEntry->portNum<=RTK_RG_MAC7_EXT_PORT5)
				mac_cpu_port = RTK_RG_MAC_PORT_SLAVECPU;
#endif
#else
#error
#endif
			igmpSnoopingCounterVer_Port[mac_cpu_port][clientEntry->igmpVersion - IGMP_V1]--;
			if(clientEntry->wlan_dev_idx >=0 && clientEntry->wlan_dev_idx<MAX_WLAN_DEVICE_NUM)
				igmpSnoopingCounterVer_MBSSID[clientEntry->wlan_dev_idx][clientEntry->igmpVersion - IGMP_V1]--;
		}
		else
		#endif
		{
			igmpSnoopingCounterVer_Port[clientEntry->portNum][clientEntry->igmpVersion - IGMP_V1]--;
		}

	}
DEL_CLIENT_UNLINK:
	rg_unlock(&rg_kernel.igmpsnoopingLock);//UnLock resource

}


/* clear the content of client entry */
static void rtl_clearClientEntry(struct rtl_clientEntry* clientEntry)
{
	rg_lock(&rg_kernel.igmpsnoopingLock);
	if (NULL!=clientEntry)
	{
		memset(clientEntry, 0, sizeof(struct rtl_clientEntry));
	}
	rg_unlock(&rg_kernel.igmpsnoopingLock);
}


/*********************************************
			source list operation
 *********************************************/
static struct rtl_sourceEntry* rtl_searchSourceEntry(uint32 ipVersion, uint32 *sourceAddr, struct rtl_clientEntry *clientEntry)
{
	struct rtl_sourceEntry *sourcePtr=clientEntry->sourceList;
	while(sourcePtr!=NULL)
	{
		if(ipVersion==IP_VERSION4)
		{
			if(sourceAddr[0]==sourcePtr->sourceAddr[0])
			{
				//DEBUG("the SIP in GIP.Clt.SrcIP-List");
				DEBUG("SIP(%pI4h) in GIP(x).Clt(%pI4h).SrcIP-List", (sourceAddr), (clientEntry->clientAddr));
				return sourcePtr;
			}
			DEBUG("SIP(%pI4h) is diff from GIP.Clt.SrcIP(%pI4h), check next", (sourceAddr), (sourcePtr->sourceAddr));
		}
		else
		{
			/*DEBUG("check GIP.Clt.SrcIP=%x.%x.%x.%x vs SIP6=%x.%x.%x.%x",
			sourcePtr->sourceAddr[0], sourcePtr->sourceAddr[0], sourcePtr->sourceAddr[2], sourcePtr->sourceAddr[3],
			sourceAddr[0], sourceAddr[0], sourceAddr[2], sourceAddr[3]);*/
			if(	(sourceAddr[0]==sourcePtr->sourceAddr[0])&&
				(sourceAddr[1]==sourcePtr->sourceAddr[1])&&
				(sourceAddr[2]==sourcePtr->sourceAddr[2])&&
				(sourceAddr[3]==sourcePtr->sourceAddr[3])
			)
			{
				//DEBUG("the SIP6 in GIP.Clt.SrcIP6-List");
				DEBUG("SIP6("IP6H") in GIP6(x).Clt("IP6H").SrcIP6-List", NIP6QUAD(sourceAddr), NIP6QUAD(clientEntry->clientAddr));
				return sourcePtr;
			}
			DEBUG("SIP6(" IP6H ") is diff from GIP6.Clt.SrcIP6(" IP6H "), check next", NIP6QUAD(sourceAddr), NIP6QUAD(sourcePtr->sourceAddr));
		}

		sourcePtr=sourcePtr->next;
	}

	//DEBUG("Not find GIP.Clt.SrcIP-list");
	if(ipVersion==IP_VERSION4)
	{
		DEBUG("SIP(%pI4h) not in GIP(x).Clt(%pI4h).SrcIP-List", (sourceAddr), (clientEntry->clientAddr));
	}
	else
	{
		DEBUG("SIP6("IP6H") not in GIP6(x).Clt("IP6H").SrcIP6-List", NIP6QUAD(sourceAddr), NIP6QUAD(clientEntry->clientAddr));
	}

	return NULL;
}


static void rtl_linkSourceEntry(struct rtl_clientEntry *clientEntry,  struct rtl_sourceEntry* entryNode)
{
	if(NULL==entryNode)
	{
		return;
	}

	if(NULL==clientEntry)
	{
		return;
	}

	rg_lock(&rg_kernel.igmpsnoopingLock);  /* lock resource*/

	if(clientEntry->sourceList!=NULL)
	{
		clientEntry->sourceList->previous=entryNode;
	}
	entryNode->next=clientEntry->sourceList;
	clientEntry->sourceList=entryNode;
	clientEntry->sourceList->previous=NULL;

	rg_unlock(&rg_kernel.igmpsnoopingLock);  /* lock resource*/
	DEBUG("Add src");
}

static void rtl_unlinkSourceEntry(struct rtl_clientEntry *clientEntry, struct rtl_sourceEntry* sourceEntry)
{
	if(NULL==sourceEntry)
	{
		return;
	}

	if(NULL==clientEntry)
	{
		return;
	}

	rg_lock(&rg_kernel.igmpsnoopingLock);  /* lock resource*/
	/* unlink entry node*/
	if(sourceEntry==clientEntry->sourceList) /*unlink group list head*/
	{
		clientEntry->sourceList=sourceEntry->next;
		if(clientEntry->sourceList!=NULL)
		{
			clientEntry->sourceList ->previous=NULL;
		}
	}
	else
	{
		if(sourceEntry->previous!=NULL)
		{
			sourceEntry->previous->next=sourceEntry->next;
		}

		if(sourceEntry->next!=NULL)
		{
			sourceEntry->next->previous=sourceEntry->previous;
		}
	}

	sourceEntry->previous=NULL;
	sourceEntry->next=NULL;

	rg_unlock(&rg_kernel.igmpsnoopingLock);//UnLock resource
}

static void rtl_clearSourceEntry(struct rtl_sourceEntry* sourceEntryPtr)
{
	rg_lock(&rg_kernel.igmpsnoopingLock);
	if (NULL!=sourceEntryPtr)
	{
		memset(sourceEntryPtr, 0, sizeof(struct rtl_sourceEntry));
	}
	rg_unlock(&rg_kernel.igmpsnoopingLock);
}

/*********************************************
			multicast flow list operation
 *********************************************/

#ifdef CONFIG_RECORD_MCAST_FLOW
static struct rtl_mcastFlowEntry* rtl_searchMcastFlowEntry(uint32 moduleIndex, uint32 ipVersion, uint32 *serverAddr,uint32 *groupAddr,uint16 sport,uint16 dport)
{
	struct rtl_mcastFlowEntry* mcastFlowPtr = NULL;
	uint32 hashIndex=0;

	if(NULL==serverAddr)
	{
		return NULL;
	}

	if(NULL==groupAddr)
	{
		return NULL;
	}

	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddr);

	mcastFlowPtr=rtl_mCastModuleArray[moduleIndex].flowHashTable[hashIndex];
	while (mcastFlowPtr!=NULL)
	{

		if(mcastFlowPtr->ipVersion!=ipVersion)
		{
			goto nextFlow;
		}


		if(ipVersion==IP_VERSION4)
		{
			//IGMP("serverAddr =%pI4	mcastFlowPtr->serverAddr=%pI4",&serverAddr[0],&mcastFlowPtr->serverAddr[0]);
			//IGMP("groupAddr=%pI4	groupAddr->groupAddr=%pI4",&groupAddr[0],&mcastFlowPtr->groupAddr[0]);
			//IGMP("sport=%d dport=%d  mcastFlowPtr->sport=%d  mcastFlowPtr->dport=%d",sport,dport,mcastFlowPtr->sport,mcastFlowPtr->dport);

			if( (serverAddr[0]==mcastFlowPtr->serverAddr[0]) && (groupAddr[0]==mcastFlowPtr->groupAddr[0])&& (sport ==mcastFlowPtr->sport) && (dport ==mcastFlowPtr->dport) )
			{
				mcastFlowPtr->refreshTime=rtl_sysUpSeconds;
				return mcastFlowPtr;
			}
		}
		else
		{
			//IGMP("serverAddr =%pI6	mcastFlowPtr->serverAddr=%pI6",&serverAddr[0],&mcastFlowPtr->serverAddr[0]);
			//IGMP("groupAddr=%pI6	groupAddr->groupAddr=%pI6",&groupAddr[0],&mcastFlowPtr->groupAddr[0]);
			//IGMP("sport=%d dport=%d  mcastFlowPtr->sport=%d  mcastFlowPtr->dport=%d",sport,dport,mcastFlowPtr->sport,mcastFlowPtr->dport);
			if(	(serverAddr[0]==mcastFlowPtr->serverAddr[0])
				&&(serverAddr[1]==mcastFlowPtr->serverAddr[1])
				&&(serverAddr[2]==mcastFlowPtr->serverAddr[2])
				&&(serverAddr[3]==mcastFlowPtr->serverAddr[3])
				&&(groupAddr[0]==mcastFlowPtr->groupAddr[0])
				&&(groupAddr[1]==mcastFlowPtr->groupAddr[1])
				&&(groupAddr[2]==mcastFlowPtr->groupAddr[2])
				&&(groupAddr[3]==mcastFlowPtr->groupAddr[3])
				&&(sport ==mcastFlowPtr->sport)
				&&(dport ==mcastFlowPtr->dport))
			{
				mcastFlowPtr->refreshTime=rtl_sysUpSeconds;
				return mcastFlowPtr;
			}
		}

nextFlow:

		mcastFlowPtr = mcastFlowPtr->next;

	}

	return NULL;
}

/* link multicast flow entry in the front of a forwarding flow list */
static void  rtl_linkMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry ,  struct rtl_mcastFlowEntry ** hashTable)
{
	uint32 hashIndex=0;

	if(NULL==mcastFlowEntry)
	{
		return;
	}

	if(NULL==hashTable)
	{
		return;
	}

	rg_lock(&rg_kernel.igmpsnoopingLock);//Lock resource

	hashIndex=rtl_igmpHashAlgorithm(mcastFlowEntry->ipVersion, mcastFlowEntry->groupAddr);
	if(hashTable[hashIndex]!=NULL)
	{
		hashTable[hashIndex]->previous=mcastFlowEntry;
	}
	mcastFlowEntry->next = hashTable[hashIndex];
	hashTable[hashIndex]=mcastFlowEntry;
	hashTable[hashIndex]->previous=NULL;

	rg_unlock(&rg_kernel.igmpsnoopingLock);//UnLock resource
	return;

}

/* unlink a multicast flow entry*/
static void rtl_unlinkMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry,  struct rtl_mcastFlowEntry ** hashTable)
{
	uint32 hashIndex=0;
	if(NULL==mcastFlowEntry)
	{
		return;
	}

	rg_lock(&rg_kernel.igmpsnoopingLock);  /* lock resource*/

	hashIndex=rtl_igmpHashAlgorithm(mcastFlowEntry->ipVersion, mcastFlowEntry->groupAddr);
	/* unlink entry node*/
	if(mcastFlowEntry==hashTable[hashIndex]) /*unlink flow list head*/
	{
		hashTable[hashIndex]=mcastFlowEntry->next;
		if(hashTable[hashIndex]!=NULL)
		{
			hashTable[hashIndex]->previous=NULL;
		}
	}
	else
	{
		if(mcastFlowEntry->previous!=NULL)
		{
			mcastFlowEntry->previous->next=mcastFlowEntry->next;
		}

		if(mcastFlowEntry->next!=NULL)
		{
			mcastFlowEntry->next->previous=mcastFlowEntry->previous;
		}
	}

	mcastFlowEntry->previous=NULL;
	mcastFlowEntry->next=NULL;

	rg_unlock(&rg_kernel.igmpsnoopingLock);//UnLock resource

}


/* clear the content of multicast flow entry */
static void rtl_clearMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry)
{
	rg_lock(&rg_kernel.igmpsnoopingLock);
	if (NULL!=mcastFlowEntry)
	{
		memset(mcastFlowEntry, 0, sizeof(struct rtl_mcastFlowEntry));
	}
	rg_unlock(&rg_kernel.igmpsnoopingLock);
}



static void rtl_deleteMcastFlowEntry( struct rtl_mcastFlowEntry* mcastFlowEntry, struct rtl_mcastFlowEntry ** hashTable)
{
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	uint32 ret;
	uint32 flowidx=0; //store index before freeflowEntry

	if(mcastFlowEntry == NULL)
		return;

	rtl_unlinkMcastFlowEntry(mcastFlowEntry, hashTable);
	rtl_clearMcastFlowEntry(mcastFlowEntry);
	rtl_freeMcastFlowEntry(mcastFlowEntry);

	//WARNING rtk_multicastFlow_del should after rtl_freeMcastFlowEntry !!!!
	ret = rtl_igmp_multicastFlow_del(flowidx);
	if(ret == RT_ERR_RG_OK)
		IGMP("del mcflowIdxtbl[%d]",flowidx);
	else
		IGMP("del mcflowIdxtbl[%d] failed ret=%d",flowidx,ret);

#else

	if(mcastFlowEntry == NULL)
		return;
	rtl_unlinkMcastFlowEntry(mcastFlowEntry, hashTable);
	rtl_clearMcastFlowEntry(mcastFlowEntry);
	rtl_freeMcastFlowEntry(mcastFlowEntry);

#endif
	return;
}
#endif

/*****source entry/client entry/group entry/flow entry operation*****/
static void rtl_deleteSourceEntry(struct rtl_clientEntry *clientEntry, struct rtl_sourceEntry* sourceEntry)
{
	if (clientEntry==NULL)
	{
		return;
	}

	if (sourceEntry!=NULL)
	{
		rtl_unlinkSourceEntry(clientEntry,sourceEntry);
		rtl_clearSourceEntry(sourceEntry);
		rtl_freeSourceEntry(sourceEntry);
	}
}


static void rtl_deleteSourceList(struct rtl_clientEntry* clientEntry)
{
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *nextSourceEntry=NULL;

	sourceEntry=clientEntry->sourceList;
	while (sourceEntry!=NULL)
	{
		nextSourceEntry=sourceEntry->next;
		rtl_deleteSourceEntry(clientEntry,sourceEntry);
		sourceEntry=nextSourceEntry;
	}
}


static void rtl_deleteClientEntry(struct rtl_groupEntry* groupEntry,struct rtl_clientEntry *clientEntry)
{

	if (NULL==clientEntry)
	{
		return;
	}

	if (NULL==groupEntry)
	{
		return;
	}
	rtl_deleteSourceList(clientEntry);
	rtl_unlinkClientEntry(groupEntry,clientEntry);
	rtl_clearClientEntry(clientEntry);
	rtl_freeClientEntry(clientEntry);
	return;

}


static void rtl_deleteClientList(struct rtl_groupEntry* groupEntry)
{

	struct rtl_clientEntry *clientEntry=NULL;
	struct rtl_clientEntry *nextClientEntry=NULL;

	if (NULL==groupEntry)
	{
		return;
	}

	clientEntry=groupEntry->clientList;
	while (clientEntry!=NULL)
	{
		nextClientEntry=clientEntry->next;
		rtl_deleteClientEntry(groupEntry,clientEntry);
		clientEntry=nextClientEntry;
	}
}


static void rtl_deleteGroupEntry( struct rtl_groupEntry* groupEntry,struct rtl_groupEntry ** hashTable)
{
	if (groupEntry!=NULL)
	{
		timerEventContext.vlanId=groupEntry->vlanId;
		rtl_deleteClientList(groupEntry);
	#ifdef CONFIG_RECORD_MCAST_FLOW
		rtl_invalidateMCastFlow(rg_db.systemGlobal.nicIgmpModuleIndex, groupEntry->ipVersion, groupEntry->groupAddr);
	#endif
#if defined (CONFIG_RG_HARDWARE_MULTICAST)
	#ifdef CONFIG_PROC_FS
		rtl_mCastModuleArray[timerEventContext.moduleIndex].expireEventCnt++;
	#endif

	#ifndef CONFIG_RG_SIMPLE_IGMP_v3
		rtl_handle_igmpgroup_change(&timerEventContext);
	#else
		rtl_handle_igmpgroup_change_v3(&timerEventContext);
	#endif
#endif
		rtl_unlinkGroupEntry(groupEntry, hashTable);
		rtl_clearGroupEntry(groupEntry);
		rtl_freeGroupEntry(groupEntry);
	}

}


// ipAddr is internet format
int32 rtl_checkMCastAddrMapping(uint32 ipVersion, uint32 *ipAddr, uint8* macAddr)
{
	if(ipVersion==IP_VERSION4)
	{

#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		if(rg_db.systemGlobal.igmp_pppoe_passthrough_learning)
		{
			//do not check any mac
			return TRUE;
		}
#endif

		if(macAddr[0]!=0x01)
		{
			goto CHECK_FAIL;
		}

		if((macAddr[3]&0x7f)!= (((uint8*)ipAddr)[1]&0x7f) )
		{
			goto CHECK_FAIL;
		}

		if(macAddr[4]!=((uint8*)ipAddr)[2])
		{
			goto CHECK_FAIL;
		}

		if(macAddr[5]!=((uint8*)ipAddr)[3] )
		{
			goto CHECK_FAIL;
		}

	}
	else
	{
		if(macAddr[0]!=0x33)
		{
			goto CHECK_FAIL;
		}

		if(macAddr[1]!=0x33)
		{
			goto CHECK_FAIL;
		}

		if(macAddr[2]!=((uint8*)ipAddr)[12])
		{
			IGMP("%x != %x ",macAddr[2],((uint8*)ipAddr)[12]);
			goto CHECK_FAIL;
		}

		if(macAddr[3]!=((uint8*)ipAddr)[13])
		{
			IGMP("%x != %x ",macAddr[3],((uint8*)ipAddr)[13]);
			goto CHECK_FAIL;
		}

		if(macAddr[4]!=((uint8*)ipAddr)[14])
		{
			IGMP("%x != %x ",macAddr[4],((uint8*)ipAddr)[14]);
			goto CHECK_FAIL;
		}

		if(macAddr[5]!=((uint8*)ipAddr)[15])
		{
			IGMP("%x != %x ",macAddr[5],((uint8*)ipAddr)[15]);
			goto CHECK_FAIL;
		}

	}
	return TRUE;

CHECK_FAIL:
	IGMP("Mac-Dip mapping check failed");
	return FALSE;
}

int32 rtl_compareIpv6Addr(uint32* ipv6Addr1, uint32* ipv6Addr2)
{
	int i;
	for(i=0; i<4; i++)
	{
		if(ipv6Addr1[i]!=ipv6Addr2[i])
		{
			return FALSE;
		}
	}

	return TRUE;
}


//static int32 rtl_compareMacAddr(uint8* macAddr1, uint8* macAddr2)
int32 rtl_compareMacAddr(uint8* macAddr1, uint8* macAddr2)
{
	int i;
	for(i=0; i<6; i++)
	{
		if(macAddr1[i]!=macAddr2[i])
		{
			return FALSE;
		}
	}
	return TRUE;
}

static uint16 rtl_checksum(uint8 *packetBuf, uint32 packetLen)
{
	/*note: the first bytes of  packetBuf should be two bytes aligned*/
	uint32  checksum=0;
	uint32 count=packetLen;
	uint16   *ptr= (uint16 *) (packetBuf);

	 while(count>1)
	 {
		  checksum+= ntohs(*ptr);
		  ptr++;
		  count -= 2;
	 }

	if(count>0)
	{
		checksum+= *(packetBuf+packetLen-1)<<8; /*the last odd byte is treated as bit 15~8 of unsigned short*/
	}

	/* Roll over carry bits */
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	/* Return checksum */
	return ((uint16) ~ checksum);

}

static uint16 rtl_ipv6L3Checksum(uint8 *pktBuf, uint32 pktLen, union pseudoHeader *ipv6PseudoHdr)
{
	uint32  checksum=0;
	uint32 count=pktLen;
	uint16   *ptr;

	/*compute ipv6 pseudo-header checksum*/
	ptr= (uint16 *) (ipv6PseudoHdr);
	for(count=0; count<20; count++) /*the pseudo header is 40 bytes long*/
	{
		  checksum+= ntohs(*ptr);
		  ptr++;
	}

	/*compute the checksum of mld buffer*/
	 count=pktLen;
	 ptr=(uint16 *) (pktBuf);
	 while(count>1)
	 {
		  checksum+= ntohs(*ptr);
		  ptr++;
		  count -= 2;
	 }

	if(count>0)
	{
		checksum+= *(pktBuf+pktLen-1)<<8; /*the last odd byte is treated as bit 15~8 of unsigned short*/
	}

	/* Roll over carry bits */
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	/* Return checksum */
	return ((uint16) ~ checksum);

}



static uint32 rtl_getClientFwdPortMask(struct rtl_clientEntry * clientEntry, uint32 sysTime)
{
	uint32 portMask=(1<<clientEntry->portNum);
	uint32 fwdPortMask=0;
	struct rtl_sourceEntry * sourcePtr=NULL;;

	//IGMP("get CltFwdPortMask fwdPrtMsk>");

	if(_check_isDummyClient(clientEntry))
		goto ADD_PORTMSK;

	 /*exclude mode or dummyClient never expired*/
	if (clientEntry->groupFilterTimer>sysTime )
	{
ADD_PORTMSK:
		fwdPortMask|=portMask;
	}
	else/*include mode*/
	{
		sourcePtr=clientEntry->sourceList;
		while (sourcePtr!=NULL)
		{
			if (sourcePtr->portTimer>sysTime)
			{
				fwdPortMask|=portMask;
				break;
			}
			sourcePtr=sourcePtr->next;
		}
	}
	return fwdPortMask;
}

static int32 rtl_checkSourceTimer(struct rtl_clientEntry * clientEntry , struct rtl_sourceEntry * sourceEntry)
{
	uint8 deleteFlag=FALSE; //delte group? client? source?
	uint8 oldFwdState,newFwdState;

	//IGMP("Check GIP.Clt.Src Timer");
	oldFwdState=sourceEntry->fwdState;

	if(sourceEntry->portTimer<=rtl_sysUpSeconds) /*means time out*/
	{
		if (clientEntry->groupFilterTimer<=rtl_sysUpSeconds) /* include mode*/
		{
			deleteFlag=TRUE;
		}
		sourceEntry->fwdState=FALSE;
	}
	else
	{
		//deleteFlag=FALSE; // redundant code
		sourceEntry->fwdState=TRUE;
	}

	newFwdState=sourceEntry->fwdState;

	//IGMP("GIP.Clt.SrcIP fwdState by {%s}"
	//, (deleteFlag==TRUE)?"<del SrcIP>":((newFwdState!=oldFwdState)&&(newFwdState==TRUE))?"<SrcIP stop fwd>":"SrcIP go on fwd");

	if (deleteFlag==TRUE) /*means INCLUDE mode and expired*/
	{
		rtl_deleteSourceEntry(clientEntry,sourceEntry);
	}

	if ((deleteFlag==TRUE) || (newFwdState!=oldFwdState))
	{
		IGMP("sourceTimer change");
		return TRUE;
	}

	return FALSE;
}


static void rtl_checkClientEntryTimer(struct rtl_groupEntry * groupEntry, struct rtl_clientEntry * clientEntry)
{

	uint32 oldFwdPortMask=0;
	uint32 newFwdPortMask=0;
	struct rtl_sourceEntry *sourceEntry=clientEntry->sourceList;
	struct rtl_sourceEntry *nextSourceEntry=NULL;
	uint32 clientStatusChange=0,sourceStatusChange=0;
	//IGMP("Check GIP.Clientt Timer");

	if(_check_isDummyClient(clientEntry)) //do not timeout
	{
		//dont care source mode always update client timmer
		if(clientEntry->groupFilterTimer>rtl_sysUpSeconds)
			clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;

		while (sourceEntry!=NULL)
		{
			//always update timer
			nextSourceEntry=sourceEntry->next;
			sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			sourceEntry=nextSourceEntry;
		}
		return;
	}
	oldFwdPortMask=rtl_getClientFwdPortMask(clientEntry, rtl_sysUpSeconds);
	while (sourceEntry!=NULL)
	{
		nextSourceEntry=sourceEntry->next;
		if(rtl_checkSourceTimer(clientEntry, sourceEntry))
			sourceStatusChange=1; //if any source change ,we will change to hardware
		sourceEntry=nextSourceEntry;
	}

	newFwdPortMask=rtl_getClientFwdPortMask(clientEntry, rtl_sysUpSeconds);
	if (newFwdPortMask==0) /*none active port*/
	{
		rtl_deleteClientEntry(groupEntry,clientEntry);
		//timerEventContext.srcFilterMode = RTK_RG_IPV4MC_DONT_CARE_SRC; ////
	}

	//Client status exclude mode -> include mode
	if( (clientEntry->groupFilterTimer<=rtl_sysUpSeconds) &&  ((rtl_sysUpSeconds - clientEntry->groupFilterTimer) <=  rg_db.systemGlobal.igmp_sys_timer_sec))
		clientStatusChange=1;
	//IGMP("rtl_sysUpSeconds=%d groupFilterTimer=%d ",rtl_sysUpSeconds,clientEntry->groupFilterTimer);

	if ((oldFwdPortMask!=newFwdPortMask) || (newFwdPortMask==0) || clientStatusChange || sourceStatusChange)
	{

		IGMP("clientStatusChange=%d sourceStatusChange=%d oldFwdPortMask=%d newFwdPortMask=%d ",clientStatusChange,sourceStatusChange,oldFwdPortMask,newFwdPortMask);
		#ifdef CONFIG_RECORD_MCAST_FLOW
		rtl_rg_mcDataReFlushAndAdd(groupEntry->groupAddr);
		//rtl_invalidateMCastFlow(timerEventContext.moduleIndex, timerEventContext.ipVersion, timerEventContext.groupAddr);
		#endif

		#if defined (CONFIG_RG_HARDWARE_MULTICAST)
		{
			#ifdef CONFIG_PROC_FS
			rtl_mCastModuleArray[timerEventContext.moduleIndex].expireEventCnt++;
			#endif

			#ifndef CONFIG_RG_SIMPLE_IGMP_v3
			rtl_handle_igmpgroup_change(&timerEventContext);
			#else
			rtl_handle_igmpgroup_change_v3(&timerEventContext);
			#endif
		}
		#endif
	}

	//Check is client report time is larger then user define
	if( (system_expire_type==SYS_EXPIRED_USER_FORCE) && (rg_db.systemGlobal.forceReportResponseTime!=0)){
		//DEBUG("###Force check clien[0x%x] report time, client jiffie=%lu### (Hz=%d)",clientEntry->clientAddr[0],clientEntry->reportUpdateJiffies,CONFIG_HZ);
		if(((clientEntry->reportUpdateJiffies - last_query_jiffies) < rg_db.systemGlobal.forceReportResponseTime*CONFIG_HZ))
		{
			//recieved report in the force time internal, keep the client.
		}
		else
		{
			IGMP("###Force delete clien[0x%x] (client update jiffies is %d) ###",clientEntry->clientAddr[0],clientEntry->reportUpdateJiffies);
			rtl_deleteClientEntry(groupEntry,clientEntry);
#ifdef CONFIG_RECORD_MCAST_FLOW
			rtl_rg_mcDataReFlushAndAdd(groupEntry->groupAddr);
#endif
		}
	}
}

static void rtl_checkGroupEntryTimer(struct rtl_groupEntry * groupEntry, struct rtl_groupEntry ** hashTable)
{
	struct rtl_clientEntry *clientEntry=groupEntry->clientList;
	struct rtl_clientEntry *nextClientEntry=NULL;

	//IGMP("In rtl_checkGroupEntryTimer");
	while(clientEntry!=NULL)
	{
		nextClientEntry=clientEntry->next;
		timerEventContext.portMask=1<<(clientEntry->portNum);
		rtl_checkClientEntryTimer(groupEntry, clientEntry);
		clientEntry=nextClientEntry;
	}

	if(groupEntry->clientList==NULL) /*none active client*/
	{
		rtl_deleteGroupEntry(groupEntry,hashTable);
#if defined (CONFIG_RG_HARDWARE_MULTICAST)
		IGMP("Check GIP.Clientt Timer event");
		rtl_handle_igmpgroup_change(&timerEventContext);
#endif
	}

}



static int32 rtl_initHashTable(uint32 moduleIndex, uint32 hashTableSize)
{
	uint32 i=0;

	/* Allocate memory */
	rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable = (struct rtl_groupEntry **)rtk_rg_malloc(sizeof(struct rtl_groupEntry *) * hashTableSize);
	if (rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!= NULL)
	{
		for (i = 0 ; i < hashTableSize ; i++)
		{
			rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i]=NULL;
		}
	}
	else
	{
		return FAIL;
	}

	rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable=  (struct rtl_groupEntry **)rtk_rg_malloc(sizeof(struct rtl_groupEntry *) * hashTableSize);
	if (rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable!=NULL)
	{
		for (i = 0 ; i < hashTableSize ; i++)
		{
			rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[i]=NULL;
		}
	}
	else
	{
		if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!=NULL)
		{
			rtk_rg_free(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
		return FAIL;
	}

	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_mCastModuleArray[moduleIndex].flowHashTable= (struct rtl_mcastFlowEntry **)rtk_rg_malloc(sizeof(struct rtl_mcastFlowEntry *) * hashTableSize);
	if (rtl_mCastModuleArray[moduleIndex].flowHashTable!=NULL)
	{
		for (i = 0 ; i < hashTableSize ; i++)
		{
			rtl_mCastModuleArray[moduleIndex].flowHashTable[i]=NULL;
		}
	}
	else
	{
		if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!=NULL)
		{
			rtk_rg_free(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}

		if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable!=NULL)
		{
			rtk_rg_free(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}

		return FAIL;
	}
	#endif
	return SUCCESS;
}



/**************************
	Utility
**************************/
static void  rtl_parseMacFrame(uint32 moduleIndex, uint8* macFrame, rtk_rg_pktHdr_t *pPktHdr, uint32 verifyCheckSum, struct rtl_macFrameInfo* macInfo)
{

//MAC Frame :DA(6 bytes)+SA(6 bytes)+ CPU tag(4 bytes) + VlAN tag(Optional, 4 bytes)
//                   +Type(IPv4:0x0800, IPV6:0x86DD, PPPOE:0x8864, 2 bytes )+Data(46~1500 bytes)+CRC(4 bytes)

	int i=0;
	uint8 routingHead=FALSE;
	union pseudoHeader pHeader;

	memset(macInfo,0,sizeof(struct rtl_macFrameInfo));
	memset(&pHeader, 0, sizeof(union pseudoHeader));

	macInfo->ipBuf = macFrame + pPktHdr->l3Offset;
	if (pPktHdr->tagif & IPV4_TAGIF)
	{
		macInfo->ipVersion=IP_VERSION4;
	}
	else if (pPktHdr->tagif & IPV6_TAGIF)
	{
		macInfo->ipVersion=IP_VERSION6;
	}
	else
	{
		IGMP("ignore non-IPv4 or non-IPv6 packet");
		return;
	}

	macInfo->checksumFlag=FAIL;

	if(macInfo->ipVersion==IP_VERSION4)
	{

		IGMP("assign IP4 IGMP");
		macInfo->ipHdrLen=pPktHdr->ipv4HeaderLen;
		macInfo->l3PktLen=pPktHdr->l3Len - pPktHdr->ipv4HeaderLen;
		macInfo->l3PktBuf=macFrame + pPktHdr->l3Offset + pPktHdr->ipv4HeaderLen;
		macInfo->macFrameLen=(uint16)(pPktHdr->l3Offset + pPktHdr->l3Len);
		macInfo->srcIpAddr[0]=pPktHdr->ipv4Sip;
		macInfo->dstIpAddr[0]=pPktHdr->ipv4Dip;
		IGMP("SIP: %pI4h DIP:%pI4h",macInfo->srcIpAddr,macInfo->dstIpAddr);
		/*distinguish different IGMP packet:
		                       ip_header_length          destination_ip      igmp_packet_length   igmp_type   group_address
		IGMPv1_general_query:                20               224.0.0.1                       8        0x11               0
		IGMPv2_general_query:                24               224.0.0.1                       8        0x11               0
		IGMPv2_group_specific_query:         24               224.0.0.1                       8        0x11             !=0
		IGMPv3 _query:                       24               224.0.0.1                    >=12        0x11   according_to_different_situation

		IGMPv1_join:                         20 actual_multicast_address                      8        0x12   actual_multicast_address
		IGMPv2_join:                         24 actual_multicast_address                      8        0x16   actual_multicast_address
		IGMPv2_leave:                        24 actual_multicast_address                      8        0x17   actual_multicast_address
		IGMPv3_report:                       24 actual_multicast_address                   >=12        0x22   actual_multicast_address*/

		/* parse IGMP type and version*/
		if (pPktHdr->tagif&DVMRP_TAGIF)
		{
			macInfo->l3Protocol=DVMRP_PROTOCOL;
		}
		else if (pPktHdr->tagif&IGMP_TAGIF)
		{
			macInfo->l3Protocol=IGMP_PROTOCOL;
		}
		else if (pPktHdr->tagif&MOSPF_TAGIF)
		{
			macInfo->l3Protocol= MOSPF_PROTOCOL;
		}
		else if (pPktHdr->tagif&PIM_TAGIF)
		{
			macInfo->l3Protocol=PIM_PROTOCOL;
		}

		if(verifyCheckSum==TRUE)
		{
			IGMP("recalculate checksum");
			if(rtl_checksum(macInfo->l3PktBuf, macInfo->l3PktLen)!=0)
			{
				macInfo->checksumFlag=FAIL;
			}
			else
			{
				macInfo->checksumFlag=SUCCESS;
			}
		}
		else
		{
			IGMP("Does not recalculate checksum");
			macInfo->checksumFlag=SUCCESS;
		}
	}

	if(macInfo->ipVersion==IP_VERSION6)
	{
		IGMP("IP6 MLD");
		memcpy(macInfo->srcIpAddr,pPktHdr->pIpv6Sip,16);
		memcpy(macInfo->dstIpAddr,pPktHdr->pIpv6Dip,16);

		macInfo->macFrameLen=(uint16)(pPktHdr->l4Offset + pPktHdr->ipv6PayloadLen);

		macInfo->ipHdrLen=IPV6_HEADER_LENGTH;
		//IGMP("macFrameLen=%d ipHdrLen=%d(fixed 40)", macInfo->macFrameLen, macInfo->ipHdrLen);

		macInfo->l3PktLen=pPktHdr->ipv6PayloadLen;

		macInfo->l3PktBuf=macFrame + pPktHdr->l4Offset;

		macInfo->l3Protocol=pPktHdr->ipProtocol;

		if (pPktHdr->tagif&ICMPV6_TAGIF)
		{
			macInfo->l3Protocol=ICMP_PROTOCOL; //58, ICMPv6
		}
		else if (pPktHdr->tagif&PIM_TAGIF)
		{
			macInfo->l3Protocol=PIM_PROTOCOL;
		}
		else if (pPktHdr->tagif&MOSPF_TAGIF)
		{
			macInfo->l3Protocol= MOSPF_PROTOCOL;
		}
		else if (pPktHdr->tagif&PIM_TAGIF)
		{
			macInfo->l3Protocol=PIM_PROTOCOL;
		}


   		if(verifyCheckSum==TRUE)
       	{
	   		/*generate pseudo header*/
	 		for(i=0; i<4; i++)
			{
				pHeader.ipv6_pHdr.sourceAddr[i]=((struct ipv6Pkt *)(macInfo->ipBuf))->sourceAddr[i];
			}

			if(routingHead==FALSE)
			{
				for(i=0;i<4;i++)
				{
					pHeader.ipv6_pHdr.destinationAddr[i]=((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[i];
				}
			}

			pHeader.ipv6_pHdr.nextHeader=macInfo->l3Protocol;
			pHeader.ipv6_pHdr.upperLayerPacketLength=htonl((uint32)(macInfo->l3PktLen));
			pHeader.ipv6_pHdr.zeroData[0]=0;
			pHeader.ipv6_pHdr.zeroData[1]=0;
			pHeader.ipv6_pHdr.zeroData[2]=0;

			/*verify checksum*/
			if(rtl_ipv6L3Checksum(macInfo->l3PktBuf, macInfo->l3PktLen,&pHeader)!=0)
			{
				macInfo->checksumFlag=FAIL;
			}
			else
			{
				macInfo->checksumFlag=SUCCESS;
			}
       	}
		else
		{
			macInfo->checksumFlag=SUCCESS;
		}
	}


	return;
}


static uint32  rtl_getMulticastRouterPortMask(uint32 moduleIndex, uint32 ipVersion, uint32 sysTime)
{
	uint32 portIndex=0;
	uint8 portMaskn=PORT0_MASK;
	uint32 routerPortmask=0;

	if(ipVersion==IP_VERSION4)
	{
		for(portIndex=0; portIndex<RTK_RG_PORT_MAX; portIndex++)
		{
			if(RG_INVALID_PORT(portIndex)) continue;
			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.querier.portTimer[portIndex]>sysTime)
			{
				routerPortmask=routerPortmask|portMaskn;
			}

			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.dvmrpRouter.portTimer[portIndex]>sysTime)
			{
				routerPortmask=routerPortmask|portMaskn;
			}


			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.mospfRouter.portTimer[portIndex]>sysTime)
			{
				routerPortmask=routerPortmask|portMaskn;
			}


			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.pimRouter.portTimer[portIndex]>sysTime)
			{
				routerPortmask=routerPortmask|portMaskn;
			}

			portMaskn=portMaskn<<1;  /*shift to next port mask*/

		}

	}
	else
	{
		for(portIndex=0; portIndex<RTK_RG_PORT_MAX; portIndex++)
		{
			if(RG_INVALID_PORT(portIndex)) continue;
			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.querier.portTimer[portIndex]>sysTime)
			{

				routerPortmask=routerPortmask|portMaskn;
			}

			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.mospfRouter.portTimer[portIndex]>sysTime)
			{
				routerPortmask=routerPortmask|portMaskn;
			}

			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.pimRouter.portTimer[portIndex]>sysTime)
			{
				routerPortmask=routerPortmask|portMaskn;
			}

			portMaskn=portMaskn<<1;  /*shift to next port mask*/

		}

	}


	routerPortmask= routerPortmask |rtl_mCastModuleArray[moduleIndex].staticRouterPortMask;

	return routerPortmask;
}

static uint32 rtl_processQueries(uint32 moduleIndex, rtk_rg_pktHdr_t *pPktHdr, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint8* pktBuf, uint32 pktLen)
{
	struct rtl_groupEntry *groupEntry=NULL;
	struct rtl_clientEntry * clientEntry=NULL;
	struct rtl_sourceEntry*sourceEntry=NULL;
	uint32 groupAddress[4]={0,0,0,0};
	uint32 suppressFlag=0;
	uint32 *sourceAddr=NULL;
	uint32 numOfSrc=0;
	uint32 i=0;

	/*querier timer update and election process*/
	rtl_snoopQuerier(moduleIndex, ipVersion, portNum);

	// TODO: Move pPktHdr->IGMPQueryVer assigned by _rtk_rg_packetParser
	if (ipVersion==IP_VERSION4)
	{
		if (pktLen>=12) /*means igmpv3 query*/
		{
			groupAddress[0]=ntohl(((struct igmpv3Query*)pktBuf)->groupAddr);
			suppressFlag=((struct igmpv3Query*)pktBuf)->rsq & S_FLAG_MASK;
			sourceAddr=&(((struct igmpv3Query*)pktBuf)->srcList);
			numOfSrc=(uint32)ntohs(((struct igmpv3Query*)pktBuf)->numOfSrc);
			IGMP("IGMPv3 Query");
			pPktHdr->IGMPQueryVer = IGMP_V3;
		}
		else
		{
			groupAddress[0]=ntohl(((struct igmpv2Pkt *)pktBuf)->groupAddr);
			if (ntohl(((struct igmpv2Pkt *)pktBuf)->maxRespTime))// decide to 1, follow timeout field
			{
				IGMP("IGMPv2 Query");
				pPktHdr->IGMPQueryVer = IGMP_V2;
			}
			else
			{
				pPktHdr->IGMPQueryVer = IGMP_V1;
				IGMP("IGMPv1 Query");
			}
		}

		if (groupAddress[0]==0) /*means general query*/
		{
			IGMP("General-Query");
			goto out;
		}
		else
		{
			IGMP("Not General-Query");
		}

	}
	else
	{
		if (pktLen>=28) /*means mldv2 query*/
		{
			IGMP("MLDv2 Query");
			memcpy(groupAddress,((struct mldv2Query*)pktBuf)->mCastAddr,IPV6ADDRLEN);

			suppressFlag=((struct mldv2Query*)pktBuf)->rsq & S_FLAG_MASK;
			sourceAddr=&(((struct mldv2Query*)pktBuf)->srcList);
			numOfSrc=(uint32)ntohs(((struct mldv2Query*)pktBuf)->numOfSrc);

		}
		else /*means mldv1 query*/
		{
			IGMP("got MLDv1");
			memcpy(groupAddress,((struct mldv1Pkt *)pktBuf)->mCastAddr,IPV6ADDRLEN);
		}

		if ((groupAddress[0]==0)&&
		    (groupAddress[1]==0)&&
		    (groupAddress[2]==0)&&
		    (groupAddress[3]==0))/*means general query*/
		{
			IGMP("General-Query");
			goto out;
		}
		else
		{
			IGMP("Not General-Query");
		}
	}


	if(suppressFlag==0 && rg_db.systemGlobal.igmp_ingoreSpecificQueryUpdateTimer==0)
	{
		//IGMP("suppressFlag==0");
		groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress, rg_db.pktHdr->internalVlanID);

		if ((groupEntry!=NULL))
		{

			if(numOfSrc==0) /*means group specific query*/
			{
				IGMP("Group-Specific-Query");
				clientEntry=groupEntry->clientList;
				while(clientEntry!=NULL)
				{
					// TODO: may using maxRespTime
					if(clientEntry->groupFilterTimer>(rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime))
					{
//default disable update group lastMemberAgingTime when receive specific query
#if 0
						clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
#endif
					}
					clientEntry=clientEntry->next;
				}

			}
			else /*means group and source specific query*/
			{
				IGMP("Group-Source-Specific-Query");
				clientEntry=groupEntry->clientList;
				while (clientEntry!=NULL)
				{
					for (i=0; i<numOfSrc; i++)
					{

						sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr, clientEntry);

						if (sourceEntry!=NULL)
						{
							// TODO: may using maxRespTime
							if (sourceEntry->portTimer>(rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime))
							{
//default disable update group lastMemberAgingTime when receive specific query
#if 0
								sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
#endif
							}

						}

						if (ipVersion==IP_VERSION4)
						{
							sourceAddr++;
						}
						else
						{
							sourceAddr=sourceAddr+4;
						}

					}

					clientEntry=clientEntry->next;
				}
			}
		}
	} //if (suppressFlag==0)


	reportEventContext.ipVersion=ipVersion;
	memcpy(reportEventContext.groupAddr,groupAddress,IPV6ADDRLEN);

	#ifdef CONFIG_RECORD_MCAST_FLOW
	//query should not to invalidate Mc flow
	//rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif

	#if defined (CONFIG_RG_HARDWARE_MULTICAST)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl_handle_igmpgroup_change(&reportEventContext);
	}
	#endif

out:
	return (~(1<<portNum) & IGMP_SUPPORT_PORT_MASK);
}


static void rtl_snoopQuerier(uint32 moduleIndex, uint32 ipVersion, uint32 portNum)
{
	/*update timer value*/
	if(ipVersion==IP_VERSION4)
	{
		rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.querier.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.querierPresentInterval;
	}
	else
	{
		rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.querier.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.querierPresentInterval;
	}
	return;

}

int32 rtl_client_cnt(uint32 ipVersion, uint32 *groupAddr, uint32 *clientAddr)
{

	int32 hashIndex,moduleIndex=0;
	struct rtl_groupEntry *groupEntryPtr;
	struct rtl_clientEntry* clientEntry=NULL;

	if (rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
	{
		if (ipVersion==IP_VERSION4 && rg_db.systemGlobal.multicastProtocol!=RG_MC_MLD_ONLY)
		{

			for (hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
			{
				groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];
				while (groupEntryPtr!=NULL)
				{
					clientEntry=groupEntryPtr->clientList;
					while (clientEntry!=NULL)
					{
						if((clientAddr[0])==clientEntry->clientAddr[0])
						{
							if(groupAddr[0]==groupEntryPtr->groupAddr[0])
								return SUCCESS;
							else
								return FAIL;
						}
						clientEntry = clientEntry->next;
					}
					groupEntryPtr=groupEntryPtr->next;
				}

			}
		}


		if(ipVersion==IP_VERSION6 &&rg_db.systemGlobal.multicastProtocol!=RG_MC_IGMP_ONLY)
		{
			for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
			{
				groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[hashIndex];
				while(groupEntryPtr!=NULL)
				{
					clientEntry=groupEntryPtr->clientList;
					while (clientEntry!=NULL)
					{
						if(memcmp(clientAddr,clientEntry->clientAddr,sizeof(clientEntry->clientAddr)))
						{
							if(memcmp(groupAddr,groupEntryPtr->groupAddr,sizeof(groupEntryPtr->groupAddr)))
								return SUCCESS;
							else
								return FAIL;
						}
						clientEntry = clientEntry->next;
					}
					groupEntryPtr=groupEntryPtr->next;
				}
			}
		}

	}
	return FAIL;



}

/* we calculated igmp and mld separately ,but only support system (igmp+mld) setting now*/
int32 rtl_group_limitCheck(uint32 ipVersion, uint32 *groupAddr , uint32 *clientAddr,uint8 *saMac,uint32 vlanId ,rtk_rg_port_idx_t portNum)
{

	struct rtl_groupEntry* groupEntry=NULL;
	int32 hashIndex,moduleIndex=0;
	int32 igmpTotalGroupCnt=0,mldTotalGroupCnt=0,totalGroupCnt=0;
	int32 igmpTotalClientCnt=0,mldTotalClientCnt=0;					//we should deduct same Client
	int32 specificClientJoinIGMPCnt=0,specificClientJoinMLDCnt=0;
	int32 tmpCountFlag=FAIL,igmpPortGroupCnt=0,mldPortGroupCnt=0;
	struct rtl_groupEntry *groupEntryPtr;
	struct rtl_clientEntry* clientEntry=NULL;


	if((ipVersion==IP_VERSION4 && IN_MULTICAST_RESV_IPV4_ALL(groupAddr[0])) ||
		(ipVersion==IP_VERSION6 && IN_MULTICAST_RESV_IPV6_ALL (groupAddr[0])) ||
		(ipVersion==IP_VERSION6 && IN_MULTICAST6_DATA_NOT_FFXE(groupAddr[0])))
	{
		IGMP("Ingore Learning Reserved mutlicast address");
		return FAIL;
	}

	if(rtl_igmp_filterWhiteListCheck(ipVersion,groupAddr,clientAddr,saMac)!= SUCCESS)
	{
		IGMP("Ingore Learning By White List check");
		return FAIL;
	}

	//FIXME: if white has hightest priory ,we should disable blackListCheck.
	if(rtl_igmp_filterBlackListCheck(ipVersion,groupAddr,clientAddr,saMac)!= SUCCESS)
	{
		IGMP("Ingore Learning By Black List check");
		return FAIL;
	}


	if (rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
	{
		if (rg_db.systemGlobal.multicastProtocol!=RG_MC_MLD_ONLY)
		{

			for (hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
			{
				groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];
				while (groupEntryPtr!=NULL)
				{
					tmpCountFlag=FAIL;
					igmpTotalGroupCnt++;
					clientEntry=groupEntryPtr->clientList;
					while (clientEntry!=NULL)
					{
						if(rtl_client_cnt(IP_VERSION4,groupEntryPtr->groupAddr,clientEntry->clientAddr)==SUCCESS)
							igmpTotalClientCnt++;
						if(ipVersion==IP_VERSION4 &&  (clientAddr[0])==clientEntry->clientAddr[0])
							specificClientJoinIGMPCnt++;
						if(tmpCountFlag==FAIL && clientEntry->portNum == portNum)
							igmpPortGroupCnt++;

						clientEntry = clientEntry->next;
					}
					groupEntryPtr=groupEntryPtr->next;
				}

			}
		}


		if(rg_db.systemGlobal.multicastProtocol!=RG_MC_IGMP_ONLY)
		{
			for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
			{
				groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[hashIndex];
				while(groupEntryPtr!=NULL)
				{
					tmpCountFlag=FAIL;
					mldTotalGroupCnt++;
					clientEntry=groupEntryPtr->clientList;
					while (clientEntry!=NULL)
					{
						if(rtl_client_cnt(IP_VERSION6,groupEntryPtr->groupAddr,clientEntry->clientAddr)==SUCCESS)
							mldTotalClientCnt++;
						if(ipVersion==IP_VERSION6 &&memcmp(clientAddr,clientEntry->clientAddr,sizeof(clientEntry->clientAddr)))
							specificClientJoinMLDCnt++;
						if(tmpCountFlag==FAIL && clientEntry->portNum == portNum)
							mldPortGroupCnt++;
						clientEntry = clientEntry->next;
					}
					groupEntryPtr=groupEntryPtr->next;
				}
			}
		}

	}

	//IGMP("igmpTotalGroupCnt:%d mldTotalGroupCnt:%d",igmpTotalGroupCnt,mldTotalGroupCnt);
	//IGMP("igmpTotalClientCnt:%d mldTotalClientCnt:%d",igmpTotalClientCnt,mldTotalClientCnt);
	//IGMP("specificClientJoinIGMPCnt:%d specificClientJoinMLDCnt:%d",specificClientJoinIGMPCnt,specificClientJoinMLDCnt);

	/* Max system group check*/
	totalGroupCnt = igmpTotalGroupCnt+ mldTotalGroupCnt;
	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddr,vlanId);
	if(groupEntry==NULL) //this is new group
	{
		if(ipVersion==IP_VERSION4 && rg_db.systemGlobal.igmp_max_simultaneous_group_size_perPort[portNum]!=RTK_RG_DEFAULT_IGMP_PORT_MAX_SIMULTANEOUS_GROUP_SIZE_UNLIMIT && (igmpPortGroupCnt >= rg_db.systemGlobal.igmp_max_simultaneous_group_size_perPort[portNum]))
		{
			IGMP("GorupPer Port out of limit igmp_max_simultaneous_group_size_perPort[%d]=%d  igmpPortGroupCnt=%d",portNum,rg_db.systemGlobal.igmp_max_simultaneous_group_size_perPort[portNum],igmpPortGroupCnt);
			return FAIL;
		}

		if(ipVersion==IP_VERSION6 && rg_db.systemGlobal.mld_max_simultaneous_group_size_perPort[portNum]!=RTK_RG_DEFAULT_MLD_PORT_MAX_SIMULTANEOUS_GROUP_SIZE_UNLIMIT && (mldPortGroupCnt >= rg_db.systemGlobal.mld_max_simultaneous_group_size_perPort[portNum]))
		{
			IGMP("GorupPer Port out of limit mld_max_simultaneous_group_size_perPort[%d]=%d  mldPortGroupCnt=%d",portNum,rg_db.systemGlobal.mld_max_simultaneous_group_size_perPort[portNum],mldPortGroupCnt);
			return FAIL;
		}

		if( (rg_db.systemGlobal.igmp_max_simultaneous_group_size!=RTK_RG_DEFAULT_IGMP_SYS_MAX_SIMULTANEOUS_GROUP_SIZE_UNLIMIT) &&(totalGroupCnt >= rg_db.systemGlobal.igmp_max_simultaneous_group_size))
		{
			IGMP("Group out of Limit igmp_max_simultaneous_group_size=%d  totalGroupCnt=%d",rg_db.systemGlobal.igmp_max_simultaneous_group_size,totalGroupCnt);
			return FAIL;
		}
	}


	if(specificClientJoinIGMPCnt)
	{/* Old Client , Per Client join group Number limit check*/
		uint32 clientInGroup=0;
		if(groupEntry!=NULL)
		{
			clientEntry=groupEntry->clientList;
			while (clientEntry!=NULL)
			{
				if(groupEntry->ipVersion ==IP_VERSION4 &&  (clientAddr[0])==clientEntry->clientAddr[0])
				{
					clientInGroup=1;
				}
				else if (groupEntry->ipVersion ==IP_VERSION6  && memcmp(clientAddr,clientEntry->clientAddr,sizeof(clientEntry->clientAddr)))
				{
					clientInGroup=1;
				}
				clientEntry = clientEntry->next;
			}

		}

		if( (groupEntry==NULL || clientInGroup==0) && (rg_db.systemGlobal.igmp_max_specific_client_join_group_size!=0) && (specificClientJoinIGMPCnt >=rg_db.systemGlobal.igmp_max_specific_client_join_group_size))
		{
			IGMP("System Client out of Limit igmp_max_specific_client_join_group_size=%d",rg_db.systemGlobal.igmp_max_specific_client_join_group_size);
			return FAIL;
		}
	}
	else
	{ /*New Client Max system Client check*/
		if((rg_db.systemGlobal.igmp_max_system_client_size!=0) && (igmpTotalClientCnt>=rg_db.systemGlobal.igmp_max_system_client_size))
		{
			IGMP("System Client out of Limit igmp_max_system_client_size=%d",rg_db.systemGlobal.igmp_max_system_client_size);
			return FAIL;
		}

	}


	if(specificClientJoinMLDCnt)
	{/* Old Client , Per Client join group Number limit check*/
		uint32 clientInGroup=0;
		if(groupEntry!=NULL)
		{
			clientEntry=groupEntry->clientList;
			while (clientEntry!=NULL)
			{
				if(groupEntry->ipVersion ==IP_VERSION4 &&  (clientAddr[0])==clientEntry->clientAddr[0])
				{
					clientInGroup=1;
				}
				else if (groupEntry->ipVersion ==IP_VERSION6  && memcmp(clientAddr,clientEntry->clientAddr,sizeof(clientEntry->clientAddr)))
				{
					clientInGroup=1;
				}
				clientEntry = clientEntry->next;
			}

		}

		if( (groupEntry==NULL || clientInGroup==0) &&(rg_db.systemGlobal.mld_max_specific_client_join_group_size!=0) && (specificClientJoinMLDCnt >=rg_db.systemGlobal.mld_max_specific_client_join_group_size))
		{
			IGMP("System Client out of Limit mld_max_specific_client_join_group_size=%d",rg_db.systemGlobal.mld_max_specific_client_join_group_size);
			return FAIL;
		}
	}
	else
	{ /*New Client Max system Client check*/
		if((rg_db.systemGlobal.mld_max_system_client_size!=0) && (mldTotalClientCnt>=rg_db.systemGlobal.mld_max_system_client_size))
		{
			IGMP("System Client out of Limit mld_max_system_client_size=%d",rg_db.systemGlobal.mld_max_system_client_size);
			return FAIL;
		}

	}


	return SUCCESS;
}

/*Process Report Packet*/
static  uint32 rtl_processJoin(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8 *pktBuf)
{
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;
	uint32 groupReFlush=0;

	uint32 hashIndex=0;
	uint32 multicastRouterPortMask=rtl_getMulticastRouterPortMask(moduleIndex, ipVersion, rtl_sysUpSeconds);

	if(ipVersion==IP_VERSION4)
	{
		if(pktBuf[0]==0x12)
		{
			IGMP("IGMPv1_Report");
			groupAddress[0]=ntohl(((struct igmpv1Pkt *)pktBuf)->groupAddr);
		}

		if(pktBuf[0]==0x16)
		{
			IGMP("IGMPv2_Join");
			groupAddress[0]=ntohl(((struct igmpv2Pkt *)pktBuf)->groupAddr);
		}

		if ((groupAddress[0] < 0xe0000100) || (groupAddress[0] == 0xeffffffa)) goto out;
	}
	else
	{
		IGMP("MLDv1_Join");
		memcpy(groupAddress,((struct mldv1Pkt *)pktBuf)->mCastAddr,IPV6ADDRLEN);
	}

#if defined(CONFIG_MASTER_WLAN0_ENABLE) && defined(CONFIG_RG_FLOW_NEW_WIFI_MODE)
{
	rtk_rg_port_idx_t tmp_portNum=portNum;
	_rtk_rg_lutExtport_translator(&tmp_portNum);
	if ((1<<tmp_portNum)&(~rg_db.systemGlobal.igmpReportIngressPortmask))
	{
		rg_db.pktHdr->ingressIgmpMldDrop = 1;
		return 0; //Drop
	}
}
#else
	if ((1<<portNum)&(~rg_db.systemGlobal.igmpReportIngressPortmask))
	{
		rg_db.pktHdr->ingressIgmpMldDrop = 1;
		return 0; //Drop
	}
#endif

	if(rtl_group_limitCheck(ipVersion,groupAddress,clientAddr,rg_db.pktHdr->smac,rg_db.pktHdr->internalVlanID,portNum)!=SUCCESS)
	{
		IGMP("Limit check FAIL Ingore Snooping learning");
		return 0;
	}


	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress, rg_db.pktHdr->internalVlanID);


	if (groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		DEBUG("record new gip");
		newGroupEntry=rtl_allocateGroupEntry();

		if(newGroupEntry==NULL)
		{
			rtlglue_printf("run out of group entry!\n");
			goto out;
		}

		assert(newGroupEntry->clientList==NULL);

		newGroupEntry->groupAddr[0]=groupAddress[0];
		newGroupEntry->groupAddr[1]=groupAddress[1];
		newGroupEntry->groupAddr[2]=groupAddress[2];
		newGroupEntry->groupAddr[3]=groupAddress[3];

		newGroupEntry->ipVersion=ipVersion;
		newGroupEntry->vlanId=rg_db.pktHdr->internalVlanID;


		if(ipVersion==IP_VERSION4)
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
		else
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}

		groupEntry=newGroupEntry;
	}

	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
	if (clientEntry==NULL)
	{
		DEBUG("record new client");
		newClientEntry=rtl_allocateClientEntry();
		if (newClientEntry==NULL)
		{
			rtlglue_printf("run out of client entry!\n");
			goto out;
		}

		assert(newClientEntry->sourceList==NULL);
		newClientEntry->portNum=portNum;
	#ifdef CONFIG_MASTER_WLAN0_ENABLE
		newClientEntry->wlan_dev_idx=wlan_dev_idx;
	#endif
		if (pktBuf[0]==0x12) ////++
			newClientEntry->igmpVersion=IGMP_V1;	////++
		else	////++
			newClientEntry->igmpVersion=IGMP_V2;

		if (ipVersion==IP_VERSION4)
		{
			newClientEntry->clientAddr[0]=clientAddr[0];

		}
		else
		{
			newClientEntry->igmpVersion=MLD_V1;
			newClientEntry->clientAddr[0]=clientAddr[0];
			newClientEntry->clientAddr[1]=clientAddr[1];
			newClientEntry->clientAddr[2]=clientAddr[2];
			newClientEntry->clientAddr[3]=clientAddr[3];

		}

#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		newClientEntry->pppoePassthroughEntry = rg_db.pktHdr->IGMP_report_passthrough_pkt;
		memcpy(newClientEntry->clientMacAddr,rg_db.pktHdr->pSmac,6);
#endif

		rtl_linkClientEntry(groupEntry, newClientEntry);
		clientEntry=newClientEntry;
		groupReFlush =1;


	}

	if(rg_db.pktHdr->tagif & CVLAN_TAGIF)
	{
#ifdef CONFIG_RECORD_MCAST_FLOW
		uint8 ori_reportCtagif=clientEntry->reportCtagif;
		uint16 ori_reportVlanId=clientEntry->reportVlanId;
#endif
		clientEntry->reportCtagif = 1;
		clientEntry->reportVlanId = rg_db.pktHdr->ctagVid;
#ifdef CONFIG_RECORD_MCAST_FLOW
		if(rg_db.systemGlobal.igmp_auto_learn_ctagif)
		{
			if(ori_reportCtagif!=clientEntry->reportCtagif || ori_reportVlanId!=clientEntry->reportVlanId )
				groupReFlush=1;
		}
#endif
	}
	clientEntry->reportUpdateJiffies=jiffies;
	//DEBUG("update client[0x%x] jiffies to %lu",clientEntry->clientAddr[0],clientEntry->reportUpdateJiffies);

	rtl_deleteSourceList(clientEntry);
	//clientEntry->igmpVersion=IGMP_V2;
	clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;

	reportEventContext.ipVersion=ipVersion;
	reportEventContext.vlanId=rg_db.pktHdr->internalVlanID;

	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];

#ifdef CONFIG_RECORD_MCAST_FLOW
	if(groupReFlush)
		rtl_rg_mcDataReFlushAndAdd(groupAddress);
#endif

	#if defined (CONFIG_RG_HARDWARE_MULTICAST)
	{
		/*we only support ipv4 hardware multicast*/
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);

		rtl_handle_igmpgroup_change(&reportEventContext);
	}
	#endif

out:
	return (multicastRouterPortMask & (~(1<<portNum)) & IGMP_SUPPORT_PORT_MASK);
}

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)

rtk_rg_err_code_t rtl_igmp_multicastFlow_del(int flow_idx)
{
	if(flow_idx<0 || DEFAULT_MAX_FLOW_COUNT <= flow_idx)
		return RT_ERR_RG_INVALID_PARAM;
	if(rg_db.mcflowIdxtbl[flow_idx].vaild==0)
		return RT_ERR_RG_ENTRY_NOT_EXIST;


	if(rg_db.mcflowIdxtbl[flow_idx].path6Idx !=-1 )
		ASSERT_EQ(_rtk_rg_flow_del(rg_db.mcflowIdxtbl[flow_idx].path6Idx,1),RT_ERR_RG_OK);;
	if(rg_db.mcflowIdxtbl[flow_idx].path4Idx !=-1 )
		ASSERT_EQ(_rtk_rg_flow_del(rg_db.mcflowIdxtbl[flow_idx].path4Idx,1),RT_ERR_RG_OK);;
	if(rg_db.mcflowIdxtbl[flow_idx].path3Idx !=-1 )
		ASSERT_EQ(_rtk_rg_flow_del(rg_db.mcflowIdxtbl[flow_idx].path3Idx,1),RT_ERR_RG_OK);
	//rg_db.mcflowIdxtbl[flow_idx].vaild=0; //we auto invaild entry in _rtk_rg_flow_del

	return RT_ERR_RG_OK;

}



uint32 rtl_igmp_updateMCastFlowIdx(uint32* multicastAddr ,uint32* sourcdeAddr ,uint32 isIpv6 ,uint32 flowIdx,uint16 sport,uint16 dport)
{
	uint32 ipVersion=IP_VERSION4;
	struct rtl_mcastFlowEntry* mcastFlowPtr=NULL;
	if(isIpv6) ipVersion= IP_VERSION6;
	mcastFlowPtr = rtl_searchMcastFlowEntry(rg_db.systemGlobal.nicIgmpModuleIndex,ipVersion,sourcdeAddr,multicastAddr,sport,dport);
	if(mcastFlowPtr)
	{
		mcastFlowPtr->flowidx = flowIdx;
		IGMP("UPDATE IGMP flowIdx=%d",flowIdx);
	}
	else
		WARNING("Can't update flowIdx to Snooping");
	return SUCCESS;
}

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
uint32 rtl_igmp_lookupGroupLutIdx(uint32* multicastAddr,uint32 isIpv6,uint32* lutDaIdx)
{

	struct rtl_groupEntry* groupEntry=NULL;

	if(isIpv6)
		groupEntry=rtl_searchGroupEntry(rg_db.systemGlobal.nicIgmpModuleIndex,IP_VERSION6,multicastAddr,rg_db.pktHdr->internalVlanID);
	else
		groupEntry=rtl_searchGroupEntry(rg_db.systemGlobal.nicIgmpModuleIndex,IP_VERSION4,multicastAddr,rg_db.pktHdr->internalVlanID);

	if(groupEntry)
		*lutDaIdx = groupEntry->destLutIdx;
	else
		return FAIL;

	return SUCCESS;
	//rtl_searchGroupEntry(uint32 moduleIndex, uint32 ipVersion,uint32 *multicastAddr, uint16 vlanId)

}
#endif

#define DUMMY_SIP 0  //dummy address = 0.0.0.PortID

uint32 rtl_igmp_multicastGroupDel(uint32 *multicastAddr,uint32 isIpv6,uint8 careSip,uint32 *includeSip,uint32 delDummyOnly)
{
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_clientEntry*  clientEntry=NULL;
	struct rtl_clientEntry*  tmpclientEntry=NULL;
	struct rtl_clientEntry*  nextClientEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;

	uint32 moduleIndex = rg_db.systemGlobal.nicIgmpModuleIndex;

	uint32 ipVersion = IP_VERSION4;

	if(isIpv6) ipVersion = IP_VERSION6;

	groupEntry=rtl_searchGroupEntry(rg_db.systemGlobal.nicIgmpModuleIndex,ipVersion,multicastAddr,rg_db.pktHdr->internalVlanID);

	if(groupEntry)
	{
		if(delDummyOnly)
		{

			if(careSip)
			{
				if(includeSip==NULL)
					return FAIL;
				//delete old source of all client
				clientEntry=groupEntry->clientList;
				while (clientEntry!=NULL)
				{
					tmpclientEntry = clientEntry->next;
					if(_check_isDummyClient(clientEntry))
					{
						newSourceEntry = rtl_searchSourceEntry(ipVersion, includeSip, clientEntry);
						if(newSourceEntry)
							rtl_deleteSourceEntry(clientEntry,newSourceEntry);
						if(clientEntry->sourceList==NULL)
							rtl_deleteClientEntry(groupEntry,clientEntry);
					}
					clientEntry = tmpclientEntry;
				}
			}
			else
			{
				clientEntry= groupEntry->clientList;
				while (clientEntry!=NULL)
				{
					nextClientEntry=clientEntry->next;
					//dummy address 0.0.0.0 ~0.0.0.255
					if(_check_isDummyClient(clientEntry))
					{
						rtl_deleteClientEntry(groupEntry,clientEntry);
						IGMP("delete dummy entry ");
					}
					clientEntry=nextClientEntry;
				}
			}

		}
		else
		{	if(isIpv6)
				rtl_deleteGroupEntry(groupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
			else
				rtl_deleteGroupEntry(groupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
	}

#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(rg_db.systemGlobal.nicIgmpModuleIndex, ipVersion, multicastAddr);
#endif


	return SUCCESS;
}



uint32 rtl_igmp_addVirtualGroup(uint32 *multicastAddr, uint32 isIpv6, uint32 portMsk,uint32 isivl,uint32 vid_fid,uint8 careSip,uint32 *includeSip)
{

	uint32 dummyClientAddr[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* tmpclientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;
	uint32 ipVersion=IP_VERSION4;
	uint32 hashIndex=0,portNum;
	uint32 moduleIndex = rg_db.systemGlobal.nicIgmpModuleIndex;
	//uint32 _HsourceAddr[4]={0, 0, 0, 0};	//Host endian
	struct rtl_sourceEntry *newSourceEntry=NULL;

	if(isIpv6) ipVersion = IP_VERSION6;

	hashIndex=rtl_igmpHashAlgorithm(ipVersion, multicastAddr);

	groupEntry=rtl_searchGroupEntry(moduleIndex,ipVersion,multicastAddr,1);


	if (groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		IGMP("record new gip");
		newGroupEntry=rtl_allocateGroupEntry();

		if(newGroupEntry==NULL)
		{
			rtlglue_printf("run out of group entry!\n");
			rtk_rg_drv_event_record(RTK_RG_DRV_EVENT_REC_MC_TBL_FULL);
			return FAIL;
		}

		assert(newGroupEntry->clientList==NULL);

		newGroupEntry->groupAddr[0]=multicastAddr[0];
		newGroupEntry->groupAddr[1]=multicastAddr[1];
		newGroupEntry->groupAddr[2]=multicastAddr[2];
		newGroupEntry->groupAddr[3]=multicastAddr[3];
		newGroupEntry->ipVersion=ipVersion;

		newGroupEntry->vlanId=rg_db.systemGlobal.initParam.fwdVLAN_CPU;
		if(isivl)
			newGroupEntry->vlanId= vid_fid;

		if(ipVersion==IP_VERSION4)
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
		else
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}

		groupEntry=newGroupEntry;
	}

	//flush Client list and re-add
	if(careSip==0)
	{
		rtl_deleteClientList(groupEntry);

		//using portId to client ip address
		for(portNum=0 ; portNum<RTK_RG_PORT_MAX ; portNum++)
		{
			if(RG_INVALID_PORT(portNum)) continue;
			if(!(portMsk & (1<<portNum))) continue;

			bzero(dummyClientAddr,sizeof(dummyClientAddr));
			if(isIpv6)
				dummyClientAddr[3]=portNum;
			else
				dummyClientAddr[0]=portNum;

			clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, dummyClientAddr);
			if (clientEntry==NULL)
			{
				IGMP("record new client");
				newClientEntry=rtl_allocateClientEntry();
				if (newClientEntry==NULL)
				{
					WARNING("run out of client entry!\n");
					return FAIL;
				}

				assert(newClientEntry->sourceList==NULL);
				newClientEntry->portNum=portNum;
			#ifdef CONFIG_MASTER_WLAN0_ENABLE
				newClientEntry->wlan_dev_idx=-1;
			#endif

				if(isIpv6)
					newClientEntry->igmpVersion=MLD_V1;
				else
					newClientEntry->igmpVersion=IGMP_V2;

				memcpy(newClientEntry->clientAddr,dummyClientAddr,sizeof(dummyClientAddr));

				rtl_linkClientEntry(groupEntry, newClientEntry);
				clientEntry=newClientEntry;
			}

			clientEntry->reportUpdateJiffies=jiffies;
			rtl_deleteSourceList(clientEntry);
			clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;

		}
	}
	else
	{
		if(includeSip==NULL)
			return FAIL;

		IGMP("delete old source of all client");
		clientEntry=groupEntry->clientList;
		while (clientEntry!=NULL)
		{
			tmpclientEntry = clientEntry->next;
			if(_check_isDummyClient(clientEntry))
			{
				newSourceEntry = rtl_searchSourceEntry(ipVersion, includeSip, clientEntry);
				if(newSourceEntry)
					rtl_deleteSourceEntry(clientEntry,newSourceEntry);
				if(clientEntry->sourceList==NULL)
					rtl_deleteClientEntry(groupEntry,clientEntry);
			}
			clientEntry = tmpclientEntry;
		}


		//using portId to client ip address
		IGMP("add new include source list");
		for(portNum=0 ; portNum<RTK_RG_PORT_MAX ; portNum++)
		{
			if(RG_INVALID_PORT(portNum)) continue;
			if(!(portMsk & (1<<portNum))) continue;

			bzero(dummyClientAddr,sizeof(dummyClientAddr));
			if(isIpv6)
				dummyClientAddr[3]=portNum;
			else
				dummyClientAddr[0]=portNum;

			clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, dummyClientAddr);
			if (clientEntry==NULL)
			{
				IGMP("record new client");
				newClientEntry=rtl_allocateClientEntry();
				if (newClientEntry==NULL)
				{
					WARNING("run out of client entry!\n");
					return FAIL;
				}

				assert(newClientEntry->sourceList==NULL);
				newClientEntry->portNum=portNum;
	#ifdef CONFIG_MASTER_WLAN0_ENABLE
				newClientEntry->wlan_dev_idx=-1;
	#endif

				if(isIpv6)
					newClientEntry->igmpVersion=MLD_V1;
				else
					newClientEntry->igmpVersion=IGMP_V2;

				memcpy(newClientEntry->clientAddr,dummyClientAddr,sizeof(dummyClientAddr));

				rtl_linkClientEntry(groupEntry, newClientEntry);
				clientEntry=newClientEntry;
			}

			clientEntry->reportUpdateJiffies=jiffies;
			clientEntry->groupFilterTimer=rtl_sysUpSeconds;


			newSourceEntry=rtl_searchSourceEntry(ipVersion, includeSip, clientEntry);

			if (newSourceEntry!=NULL) //had recorded source-ip
			{
				newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			}
			else
			{
				newSourceEntry=rtl_allocateSourceEntry();
				if (newSourceEntry==NULL)
				{
					rtlglue_printf("run out of source entry!\n");
					return FAIL;
				}

				if (ipVersion==IP_VERSION4)
				{
					newSourceEntry->sourceAddr[0]=includeSip[0];
				}
				else
				{
					newSourceEntry->sourceAddr[0]=includeSip[0];
					newSourceEntry->sourceAddr[1]=includeSip[1];
					newSourceEntry->sourceAddr[2]=includeSip[2];
					newSourceEntry->sourceAddr[3]=includeSip[3];
				}
				rtl_linkSourceEntry(clientEntry,newSourceEntry);
				newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			}

		}


	}

#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_rg_mcDataReFlushAndAdd(groupEntry->groupAddr);
	//rtl_invalidateMCastFlow(moduleIndex, ipVersion, groupEntry->groupAddr);
#endif



	return SUCCESS;

}

uint32 rtl_igmp_flowDeleteSyncToSnooping(uint32 path346Idx)
{
	int32 i,swFlowIdx=-1;


	for(i=0 ; i < DEFAULT_MAX_FLOW_COUNT; i++)
	{
		if(rg_db.mcflowIdxtbl[i].vaild ==0)
			continue;

		if(rg_db.mcflowIdxtbl[i].path3Idx ==path346Idx)
		{
			rg_db.mcflowIdxtbl[i].path3Idx = FAIL;
			swFlowIdx=i;
			break;
		}
		if(rg_db.mcflowIdxtbl[i].path4Idx ==path346Idx)
		{
			rg_db.mcflowIdxtbl[i].path4Idx = FAIL;
			swFlowIdx=i;
			break;
		}
		if(rg_db.mcflowIdxtbl[i].path6Idx ==path346Idx)
		{
			rg_db.mcflowIdxtbl[i].path6Idx = FAIL;
			swFlowIdx=i;
			break;
		}
	}

	if(swFlowIdx!= FAIL  && rg_db.mcflowIdxtbl[swFlowIdx].path3Idx ==FAIL && rg_db.mcflowIdxtbl[swFlowIdx].path4Idx ==FAIL && rg_db.mcflowIdxtbl[swFlowIdx].path6Idx ==FAIL && rg_db.mcflowIdxtbl[swFlowIdx].pppoePassthroughEntry)
	{

		//pppoePassthrough decrease DA/SA reference count
		if(rg_db.mcflowIdxtbl[swFlowIdx].lutSaIdx!=FAIL)
			rg_db.lut[rg_db.mcflowIdxtbl[swFlowIdx].lutSaIdx].mcStaticRefCnt--;

		if(rg_db.mcflowIdxtbl[swFlowIdx].lutIdx!=FAIL)
			rg_db.lut[rg_db.mcflowIdxtbl[swFlowIdx].lutIdx].mcStaticRefCnt--;

		IGMP("invalid  pppoePassthroughEntry  rg_db.mcflowIdxtbl[%d]",swFlowIdx);
		rg_db.mcflowIdxtbl[swFlowIdx].vaild=0;
		return SUCCESS;
	}

	if(swFlowIdx!= FAIL  && rg_db.mcflowIdxtbl[swFlowIdx].path3Idx ==FAIL && rg_db.mcflowIdxtbl[swFlowIdx].path4Idx ==FAIL && rg_db.mcflowIdxtbl[swFlowIdx].path6Idx ==FAIL)
	{
		int32 ipVersion=IP_VERSION4;
		uint32 moduleIndex = rg_db.systemGlobal.nicIgmpModuleIndex;
		uint32 hashIndex;
		struct rtl_mcastFlowEntry* mcastFlowEntry = NULL;
		struct rtl_mcastFlowEntry* nextMcastFlowEntry = NULL;
		uint32 groupip[4]={0};

		memcpy(groupip,rg_db.mcflowIdxtbl[swFlowIdx].multicastAddress,sizeof(groupip));
		if(rg_db.mcflowIdxtbl[swFlowIdx].isIpv6) ipVersion=IP_VERSION6;

		hashIndex=rtl_igmpHashAlgorithm(ipVersion, rg_db.mcflowIdxtbl[swFlowIdx].multicastAddress);

		mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[hashIndex];

		//for igmpSnooping delete clear mcastFlowEntry first so will not find same entry
		while (mcastFlowEntry!=NULL)
		{
			nextMcastFlowEntry=mcastFlowEntry->next;

			if(ipVersion ==mcastFlowEntry->ipVersion && mcastFlowEntry->flowidx==swFlowIdx &&
				rg_db.mcflowIdxtbl[swFlowIdx].multicastAddress[0]==mcastFlowEntry->groupAddr[0] && rg_db.mcflowIdxtbl[swFlowIdx].multicastAddress[1]==mcastFlowEntry->groupAddr[1] &&
				rg_db.mcflowIdxtbl[swFlowIdx].multicastAddress[2]==mcastFlowEntry->groupAddr[2] &&rg_db.mcflowIdxtbl[swFlowIdx].multicastAddress[3]==mcastFlowEntry->groupAddr[3] )
			{
				rtl_deleteMcastFlowEntry(mcastFlowEntry, rtl_mCastModuleArray[moduleIndex].flowHashTable);
				break;
			}

			mcastFlowEntry = nextMcastFlowEntry;
		}
		if(rg_db.mcflowIdxtbl[swFlowIdx].staticEnty==0)
		{
			//invalid entry
#if defined(CONFIG_RG_G3_SERIES)
			_rtk_rg_g3McEngineInfoDel(rg_db.mcflowIdxtbl[swFlowIdx].pMcEngineInfo);
#endif
			IGMP("invalid entry rg_db.mcflowIdxtbl[%d]",swFlowIdx);
			rg_db.mcflowIdxtbl[swFlowIdx].vaild=0;
			//decrease static reference count
			if(rg_db.mcflowIdxtbl[swFlowIdx].lutSaIdx!=FAIL)
				rg_db.lut[rg_db.mcflowIdxtbl[swFlowIdx].lutSaIdx].mcStaticRefCnt--;

#if 0// defined(CONFIG_RG_G3_SERIES)
			//if all flow clear we should set mainhash trap to cpu
			{
				int i,staticEntry=0,staticEntryIdx=FAIL,flowEntry=0;
				IGMP("all flow clear we should set mainhash trap to cpu");
				for(i=0 ;i<DEFAULT_MAX_FLOW_COUNT ;i++)
				{
					if(rg_db.mcflowIdxtbl[i].vaild  && memcmp(groupip,rg_db.mcflowIdxtbl[i].multicastAddress,sizeof(groupip))==0)
					{
						if(rg_db.mcflowIdxtbl[i].staticEnty)
						{
							staticEntry++;
							staticEntryIdx=i;
						}
						else
							flowEntry++;
					}
				}
				if(staticEntry>0 && flowEntry==0)
				{
					rtl_igmp_multicastGroupDel(groupip,rg_db.mcflowIdxtbl[staticEntryIdx].isIpv6,0);
					ASSERT_EQ(rtl_igmp_addVirtualGroup(groupip,rg_db.mcflowIdxtbl[staticEntryIdx].isIpv6,rg_db.mcflowIdxtbl[staticEntryIdx].macPmsk,rg_db.mcflowIdxtbl[staticEntryIdx].isIVL,rg_db.mcflowIdxtbl[staticEntryIdx].fid_vlan),RT_ERR_RG_OK);
					//update staticEntry.pMcEngineInfo
					rg_db.mcflowIdxtbl[staticEntryIdx].pMcEngineInfo= _rtk_rg_g3McEngineInfoFind(groupip,NULL);
					if(rg_db.mcflowIdxtbl[staticEntryIdx].pMcEngineInfo)
						IGMP("readd G3 Group MainHash to Trap mcHwRefCount=%d",rg_db.mcflowIdxtbl[staticEntryIdx].pMcEngineInfo->mcHwRefCount);
				}
			}
#endif
		}

	}

	return SUCCESS;


}

/* Flow based HWNAT check IGMP entry timeout according gip & sip. */
int rtl_getIGMPGroupSourceFwdTimeoutSecs(uint32 gip, uint32 sip)
{
        int32 moduleIndex;
        int32 hashIndex;
        struct rtl_groupEntry *groupEntryPtr;
        struct rtl_clientEntry* clientEntry=NULL;

        for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM ;moduleIndex++)
        {
                if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
                {
                        for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
                        {
                                groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];
                                while(groupEntryPtr!=NULL)
                                {
                                        if(groupEntryPtr->groupAddr[0]==gip)
                                        {
                                                clientEntry=groupEntryPtr->clientList;
                                                while (clientEntry!=NULL)
                                                {
                                                        if(clientEntry->clientAddr[0] == sip)
                                                        {
                                                                return (clientEntry->groupFilterTimer-rtl_sysUpSeconds);
                                                        }
                                                        clientEntry = clientEntry->next;
                                                }
                                        }
                                        groupEntryPtr=groupEntryPtr->next;
                                }
                        }
                }
        }
        return 0;
}

#endif

static  uint32 rtl_processLeave(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8 *pktBuf)
{
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_clientEntry *clientEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *nextSourceEntry=NULL;

	uint32 hashIndex=0;
	uint32 multicastRouterPortMask=rtl_getMulticastRouterPortMask(moduleIndex, ipVersion, rtl_sysUpSeconds);
	IGMP("IGMP/MLD Leave packet");

	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct igmpv2Pkt *)pktBuf)->groupAddr);
	}
	else
	{
		memcpy(groupAddress,((struct mldv1Pkt *)pktBuf)->mCastAddr,IPV6ADDRLEN);
	}


	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress, rg_db.pktHdr->internalVlanID);

	if(groupEntry!=NULL)
	{
		clientEntry=rtl_searchClientEntry( ipVersion, groupEntry, portNum, clientAddr);
		if(clientEntry!=NULL)
		{
			if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
			{
				rtl_deleteClientEntry(groupEntry, clientEntry);
			}
			else
			{
				sourceEntry = clientEntry->sourceList;
				while(sourceEntry!=NULL)
				{
					nextSourceEntry=sourceEntry->next;
					if(sourceEntry->portTimer>rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)
					{
						sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
					}
					sourceEntry=nextSourceEntry;
				}

				if(clientEntry->groupFilterTimer>rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)
				{
					clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
				}

			}

		}

	}

	reportEventContext.ipVersion=ipVersion;
	reportEventContext.vlanId=rg_db.pktHdr->internalVlanID;

	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];

	#ifdef CONFIG_RECORD_MCAST_FLOW
	if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
		rtl_rg_mcDataReFlushAndAdd(groupAddress);
	#endif

	#if defined (CONFIG_RG_HARDWARE_MULTICAST)
	{
		/*we only support ipv4 hardware multicast*/
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
		{
			rtl_handle_igmpgroup_change(&reportEventContext);
		}

	}
	#endif

	if((groupEntry!=NULL) && (groupEntry->clientList==NULL))
	{
		if(ipVersion==IP_VERSION4)
		{
			rtl_deleteGroupEntry(groupEntry,rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
		else
		{
			rtl_deleteGroupEntry(groupEntry,rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}
	}

	//return (multicastRouterPortMask&(~(1<<portNum))&0x3f);
	return (multicastRouterPortMask & (~(1<<portNum)) & IGMP_SUPPORT_PORT_MASK);
}

static  int32 rtl_processIsInclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8 *pktBuf)
{

	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;

	uint32 hashIndex=0;

	uint16 numOfSrc=0;
	uint32 *p_NsourceAddr=NULL;				//Net endian
	uint32 _HsourceAddr[4]={0, 0, 0, 0};	//Host endian


	if(ipVersion==IP_VERSION4)
	{
		IGMP("process IGMPv3-IsInclude");
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		p_NsourceAddr=&(((struct groupRecord *)pktBuf)->srcList);

		if ((groupAddress[0] < 0xe0000100) || (groupAddress[0] == 0xeffffffa)) return FAIL;

	}
	else
	{
		IGMP("process MLDv2-IsInclude");
		memcpy(groupAddress,((struct mCastAddrRecord *)pktBuf)->mCastAddr,IPV6ADDRLEN);

		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		p_NsourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	}

	if(rtl_group_limitCheck(ipVersion,groupAddress,clientAddr,rg_db.pktHdr->smac,rg_db.pktHdr->internalVlanID,portNum)!=SUCCESS)
	{
		IGMP("Limit check FAIL Ingore Snooping learning");
		return 0;
	}

	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress, rg_db.pktHdr->internalVlanID);

	if (groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		DEBUG("New GIP");
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
			rtlglue_printf("run out of group entry!\n");
			return FAIL;
		}

		assert(newGroupEntry->clientList==NULL);
		/*set new multicast entry*/

		newGroupEntry->groupAddr[0]=groupAddress[0];
		newGroupEntry->groupAddr[1]=groupAddress[1];
		newGroupEntry->groupAddr[2]=groupAddress[2];
		newGroupEntry->groupAddr[3]=groupAddress[3];

		newGroupEntry->ipVersion=ipVersion;
		newGroupEntry->vlanId=rg_db.pktHdr->internalVlanID;

		if(ipVersion==IP_VERSION4)
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
		else
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}

		groupEntry=newGroupEntry;
	}

	/*from here groupEntry is the same as newGroupEntry*/
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
	if (clientEntry==NULL)
	{
		DEBUG("New GIP.Clt");
		newClientEntry=rtl_allocateClientEntry();
		if (newClientEntry==NULL)
		{
			rtlglue_printf("run out of client entry!\n");
			return FAIL;
		}

		assert(newClientEntry->sourceList==NULL);
		newClientEntry->sourceList=NULL;
		newClientEntry->igmpVersion=IGMP_V3;
		newClientEntry->portNum=portNum;
	#ifdef CONFIG_MASTER_WLAN0_ENABLE
		newClientEntry->wlan_dev_idx=wlan_dev_idx;
	#endif

		if(ipVersion==IP_VERSION4)
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
		}
		else
		{
			newClientEntry->igmpVersion=MLD_V2;
			newClientEntry->clientAddr[0]=clientAddr[0];
			newClientEntry->clientAddr[1]=clientAddr[1];
			newClientEntry->clientAddr[2]=clientAddr[2];
			newClientEntry->clientAddr[3]=clientAddr[3];
		}
#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		newClientEntry->pppoePassthroughEntry = rg_db.pktHdr->IGMP_report_passthrough_pkt;
		memcpy(newClientEntry->clientMacAddr,rg_db.pktHdr->pSmac,6);
#endif

		rtl_linkClientEntry(groupEntry, newClientEntry);
		clientEntry=newClientEntry;
		clientEntry->groupFilterTimer=rtl_sysUpSeconds; ////
	}

	if(rg_db.pktHdr->tagif & CVLAN_TAGIF)
	{
		clientEntry->reportCtagif = 1;
		clientEntry->reportVlanId = rg_db.pktHdr->ctagVid;

	}



	if (ipVersion==IP_VERSION4)
		clientEntry->igmpVersion=IGMP_V3;
	else //if (ipVersion==IP_VERSION6)
		clientEntry->igmpVersion=MLD_V2;


	DEBUG("GIP.Clt");
	/*here to handle the source list*/
	for (j=0; j<numOfSrc; j++)
	{
		if (ipVersion==IP_VERSION4)
			_HsourceAddr[0] = ntohl(*p_NsourceAddr);
		else
			memcpy(_HsourceAddr,p_NsourceAddr,sizeof(_HsourceAddr));
		newSourceEntry=rtl_searchSourceEntry(ipVersion, _HsourceAddr, clientEntry);
		if (clientEntry->groupFilterTimer>rtl_sysUpSeconds)
		{
			if (newSourceEntry!=NULL) //had recorded source-ip
			{

				newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			}
			else
			{
				DEBUG("New GIP.Clt.Src(%d)",j);
				newSourceEntry=rtl_allocateSourceEntry();
				if (newSourceEntry==NULL)
				{
					rtlglue_printf("run out of source entry!\n");
					return FAIL;
				}

				if (ipVersion==IP_VERSION4)
				{
					newSourceEntry->sourceAddr[0]=ntohl(p_NsourceAddr[0]);
					DEBUG("SrcList[%d]=%pI4h" , j, (newSourceEntry->sourceAddr));
				}
				else
				{
					newSourceEntry->sourceAddr[0]=p_NsourceAddr[0];
					newSourceEntry->sourceAddr[1]=p_NsourceAddr[1];
					newSourceEntry->sourceAddr[2]=p_NsourceAddr[2];
					newSourceEntry->sourceAddr[3]=p_NsourceAddr[3];
				}
				rtl_linkSourceEntry(clientEntry,newSourceEntry);
				newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			}
		}
		else
		{
			if (newSourceEntry==NULL)
			{
				DEBUG("New GIP.Clt.Src(%d)",j);
				newSourceEntry=rtl_allocateSourceEntry();
				if (newSourceEntry==NULL)
				{
					rtlglue_printf("run out of source entry!\n");
					return FAIL;
				}

				if (ipVersion==IP_VERSION4)
				{
					newSourceEntry->sourceAddr[0]=ntohl(p_NsourceAddr[0]);
					DEBUG("SrcList[%d]=%pI4h"  , j, (newSourceEntry->sourceAddr));
				}
				else
				{
					newSourceEntry->sourceAddr[0]=p_NsourceAddr[0];
					newSourceEntry->sourceAddr[1]=p_NsourceAddr[1];
					newSourceEntry->sourceAddr[2]=p_NsourceAddr[2];
					newSourceEntry->sourceAddr[3]=p_NsourceAddr[3];
				}

				rtl_linkSourceEntry(clientEntry,newSourceEntry);
			}
			newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
		}
		if (ipVersion==IP_VERSION4)
			p_NsourceAddr++;
		else
			p_NsourceAddr=p_NsourceAddr+4;

	} ////: considerate per M-SIP
		//reportEventContext.srcFilterMode = RTK_RG_IPV4MC_INCLUDE; ////
		reportEventContext.ipVersion=ipVersion;
		reportEventContext.vlanId=rg_db.pktHdr->internalVlanID;

		reportEventContext.groupAddr[0]=groupAddress[0];
		reportEventContext.groupAddr[1]=groupAddress[1];
		reportEventContext.groupAddr[2]=groupAddress[2];
		reportEventContext.groupAddr[3]=groupAddress[3];

	if (newSourceEntry)
	{
		reportEventContext.sourceAddr[0]=newSourceEntry->sourceAddr[0]; ////
		reportEventContext.sourceAddr[1]=newSourceEntry->sourceAddr[1]; ////
		reportEventContext.sourceAddr[2]=newSourceEntry->sourceAddr[2]; ////
		reportEventContext.sourceAddr[3]=newSourceEntry->sourceAddr[3]; ////

	}

#ifdef CONFIG_RECORD_MCAST_FLOW
		//IGMPv3 always update flow
		rtl_rg_mcDataReFlushAndAdd(groupAddress);
#endif


	#if defined (CONFIG_RG_HARDWARE_MULTICAST)
	{
		/*we only support ipv4 hardware multicast*/
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		#ifndef CONFIG_RG_SIMPLE_IGMP_v3
		rtl_handle_igmpgroup_change(&reportEventContext);
		#else
		rtl_handle_igmpgroup_change_v3(&reportEventContext);
		#endif
	}
	#endif
	//} ////: considerate per M-SIP

	return SUCCESS;
}

static  int32 rtl_processIsExclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8 *pktBuf)
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;

	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *p_NsourceAddr=NULL;				//Net endian
	uint32 _HsourceAddr[4]={0, 0, 0, 0};	//Host endian


	if(ipVersion==IP_VERSION4)
	{
		IGMP("process IGMPv3-IsExclude");
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		p_NsourceAddr=&(((struct groupRecord *)pktBuf)->srcList);

		if ((groupAddress[0] < 0xe0000100) || (groupAddress[0] == 0xeffffffa)) return FAIL;
	}
	else
	{
		IGMP("process MLDv2-IsExclude");
		memcpy(groupAddress,((struct mCastAddrRecord *)pktBuf)->mCastAddr,IPV6ADDRLEN);

		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		p_NsourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	}

	if(rtl_group_limitCheck(ipVersion,groupAddress,clientAddr,rg_db.pktHdr->smac,rg_db.pktHdr->internalVlanID,portNum)!=SUCCESS)
	{
		IGMP("Limit check FAIL Ingore Snooping learning");
		return 0;
	}

	hashIndex=rtl_igmpHashAlgorithm( ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress, rg_db.pktHdr->internalVlanID);
	if (groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if (newGroupEntry==NULL)
		{
			rtlglue_printf("run out of group entry!\n");
			return FAIL;
		}

		assert(newGroupEntry->clientList==NULL);
		/*set new multicast entry*/
		newGroupEntry->vlanId=rg_db.pktHdr->internalVlanID;
		newGroupEntry->ipVersion=ipVersion;
		if (ipVersion==IP_VERSION4)
		{
			newGroupEntry->groupAddr[0]=groupAddress[0];
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
		else
		{
			newGroupEntry->groupAddr[0]=groupAddress[0];
			newGroupEntry->groupAddr[1]=groupAddress[1];
			newGroupEntry->groupAddr[2]=groupAddress[2];
			newGroupEntry->groupAddr[3]=groupAddress[3];
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}

		groupEntry=newGroupEntry;
	}

	/*from here groupEntry is the same as  newGroupEntry*/
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
	if (clientEntry==NULL)
	{
		newClientEntry=rtl_allocateClientEntry();
		if (newClientEntry==NULL)
		{
			rtlglue_printf("run out of client entry!\n");
			return FAIL;
		}

		assert(newClientEntry->sourceList==NULL);

		newClientEntry->sourceList=NULL;
		newClientEntry->igmpVersion=IGMP_V3;
		newClientEntry->portNum=portNum;
	#ifdef CONFIG_MASTER_WLAN0_ENABLE
		newClientEntry->wlan_dev_idx=wlan_dev_idx;
	#endif

		if(ipVersion==IP_VERSION4)
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
		}
		else
		{
			newClientEntry->igmpVersion=MLD_V2;
			newClientEntry->clientAddr[0]=clientAddr[0];
			newClientEntry->clientAddr[1]=clientAddr[1];
			newClientEntry->clientAddr[2]=clientAddr[2];
			newClientEntry->clientAddr[3]=clientAddr[3];
		}
#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		newClientEntry->pppoePassthroughEntry = rg_db.pktHdr->IGMP_report_passthrough_pkt;
		memcpy(newClientEntry->clientMacAddr,rg_db.pktHdr->pSmac,6);
#endif

		rtl_linkClientEntry(groupEntry, newClientEntry);
		clientEntry=newClientEntry;
		//clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;////
	}

	if(rg_db.pktHdr->tagif & CVLAN_TAGIF)
	{
		clientEntry->reportCtagif = 1;
		clientEntry->reportVlanId = rg_db.pktHdr->ctagVid;
	}


	/*from here clientEntry  is the same as newClientEntry*/
	if (ipVersion==IP_VERSION4)
		clientEntry->igmpVersion=IGMP_V3;
	else //if (ipVersion==IP_VERSION6)
		clientEntry->igmpVersion=MLD_V2;


	/*flush the old source list*/
	// delete ((X-A) | (Y-A)) or (A-B)
	{
		struct rtl_sourceEntry *sourceEntry=clientEntry->sourceList;
		while (sourceEntry!=NULL)
		{
			sourceEntry->setOpFlag=1;
			sourceEntry=sourceEntry->next;
		}
	}


	/*link the new source list*/
	for (j=0; j<numOfSrc; j++)
	{
		if (ipVersion==IP_VERSION4)
			_HsourceAddr[0] = ntohl(*p_NsourceAddr);
		else
			memcpy(_HsourceAddr,p_NsourceAddr,sizeof(_HsourceAddr));

		newSourceEntry=rtl_searchSourceEntry(ipVersion, _HsourceAddr, clientEntry);
		if (clientEntry->groupFilterTimer>rtl_sysUpSeconds) //ex-mo
		{
			if (newSourceEntry!=NULL) // A*X or A*Y
			{
				if (newSourceEntry->portTimer <= rtl_sysUpSeconds) // A*Y
				{
					//rtl_deleteSourceEntry(clientEntry,newSourceEntry);
					newSourceEntry->portTimer=rtl_sysUpSeconds;
				}
				else //A*X
				{
					//newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
				}
				newSourceEntry->setOpFlag=0;
			}
			else //A-X-Y = GMI
			{
				DEBUG("New GIP.Clt.Src(%d)",j);
				newSourceEntry=rtl_allocateSourceEntry();
				if (newSourceEntry==NULL)
				{
					rtlglue_printf("run out of source entry!\n");
					return FAIL;
				}

				if (ipVersion==IP_VERSION4)
				{
					newSourceEntry->sourceAddr[0]=ntohl(p_NsourceAddr[0]);
					DEBUG("SrcList[%d]=%pI4h"  , j, (newSourceEntry->sourceAddr));
				}
				else
				{
					newSourceEntry->sourceAddr[0]=p_NsourceAddr[0];
					newSourceEntry->sourceAddr[1]=p_NsourceAddr[1];
					newSourceEntry->sourceAddr[2]=p_NsourceAddr[2];
					newSourceEntry->sourceAddr[3]=p_NsourceAddr[3];
				}
				rtl_linkSourceEntry(clientEntry,newSourceEntry);
				newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
				newSourceEntry->setOpFlag=0;
			}
		}
		else
		{
			if (newSourceEntry==NULL) //B-A
			{
				DEBUG("New GIP.Clt.Src(Record index:%d)",j);
				newSourceEntry=rtl_allocateSourceEntry();
				if (newSourceEntry==NULL)
				{
					rtlglue_printf("run out of source entry!\n");
					return FAIL;
				}

				if (ipVersion==IP_VERSION4)
				{
					newSourceEntry->sourceAddr[0]=ntohl(p_NsourceAddr[0]);
					DEBUG("SrcList[%d]=%pI4h"  , j, (newSourceEntry->sourceAddr));
				}
				else
				{
					newSourceEntry->sourceAddr[0]=p_NsourceAddr[0];
					newSourceEntry->sourceAddr[1]=p_NsourceAddr[1];
					newSourceEntry->sourceAddr[2]=p_NsourceAddr[2];
					newSourceEntry->sourceAddr[3]=p_NsourceAddr[3];
				}
				rtl_linkSourceEntry(clientEntry,newSourceEntry);
				newSourceEntry->portTimer=rtl_sysUpSeconds;
			}
			else //B*A
			{
				//newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			}
			newSourceEntry->setOpFlag=0;
		}
		if (ipVersion==IP_VERSION4)
			p_NsourceAddr++;
		else
			p_NsourceAddr=p_NsourceAddr+4;

	} ////: considerate per M-SIP

	// delete ((X-A) | (Y-A)) or (A-B)
	{
		struct rtl_sourceEntry *sourceEntry=clientEntry->sourceList;
		struct rtl_sourceEntry *nextSourceEntry;
		while (sourceEntry!=NULL)
		{
			nextSourceEntry=sourceEntry->next;
			if (sourceEntry->setOpFlag)
			{
				rtl_deleteSourceEntry(clientEntry, sourceEntry);
			}
			sourceEntry=nextSourceEntry;
		}
	}

	clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;

	//reportEventContext.srcFilterMode = RTK_RG_IPV4MC_EXCLUDE; ////
	reportEventContext.vlanId=rg_db.pktHdr->internalVlanID;
	reportEventContext.ipVersion=ipVersion;

	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];

	if (newSourceEntry)
	{
		reportEventContext.sourceAddr[0]=newSourceEntry->sourceAddr[0]; ////
		reportEventContext.sourceAddr[1]=newSourceEntry->sourceAddr[1]; ////
		reportEventContext.sourceAddr[2]=newSourceEntry->sourceAddr[2]; ////
		reportEventContext.sourceAddr[3]=newSourceEntry->sourceAddr[3]; ////

	}

#ifdef CONFIG_RECORD_MCAST_FLOW
	//IGMPv3 always update flow
	rtl_rg_mcDataReFlushAndAdd(groupAddress);
#endif
	#if defined (CONFIG_RG_HARDWARE_MULTICAST)
	{
		/*we only support ipv4 hardware multicast*/
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		#ifndef CONFIG_RG_SIMPLE_IGMP_v3
		rtl_handle_igmpgroup_change(&reportEventContext);
		#else
		rtl_handle_igmpgroup_change_v3(&reportEventContext);
		#endif
	}
	#endif
	//} ////: considerate per M-SIP

	return SUCCESS;

}

static int32 rtl_processToInclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8 *pktBuf)
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	//struct rtl_sourceEntry *nextSourceEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;

	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *p_NsourceAddr=NULL;				//Net endian
	uint32 _HsourceAddr[4]={0, 0, 0, 0};	//Host endian


	uint16 numOfQuerySrc=0;

	if (ipVersion==IP_VERSION4)
	{
		IGMP("process IGMPv3-ToInclude");
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		p_NsourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
		if ((groupAddress[0] < 0xe0000100) || (groupAddress[0] == 0xeffffffa)) return FAIL;
	}
	else
	{
		IGMP("process MLDv2-ToInclude");
		memcpy(groupAddress,((struct mCastAddrRecord *)pktBuf)->mCastAddr,IPV6ADDRLEN);

		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		p_NsourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	}

	if(rtl_group_limitCheck(ipVersion,groupAddress,clientAddr,rg_db.pktHdr->smac,rg_db.pktHdr->internalVlanID,portNum)!=SUCCESS)
	{
		IGMP("Limit check FAIL Ingore Snooping learning");
		return 0;
	}


	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress, rg_db.pktHdr->internalVlanID);
	if (groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if (newGroupEntry==NULL)
		{
			rtlglue_printf("run out of group entry!\n");
			return FAIL;
		}

		newGroupEntry->vlanId=rg_db.pktHdr->internalVlanID;
		newGroupEntry->ipVersion=ipVersion;
		if (ipVersion==IP_VERSION4)
		{
			newGroupEntry->groupAddr[0]=groupAddress[0];
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
		else
		{
			newGroupEntry->groupAddr[0]=groupAddress[0];
			newGroupEntry->groupAddr[1]=groupAddress[1];
			newGroupEntry->groupAddr[2]=groupAddress[2];
			newGroupEntry->groupAddr[3]=groupAddress[3];
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}

		groupEntry=newGroupEntry;
	}

	/*from here groupEntry is the same as newGroupEntry*/
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
	if (clientEntry==NULL)
	{
		newClientEntry=rtl_allocateClientEntry();
		if (newClientEntry==NULL)
		{
			rtlglue_printf("run out of client entry!\n");
			return FAIL;
		}

		assert(newClientEntry->sourceList==NULL);
		newClientEntry->sourceList=NULL;
		newClientEntry->portNum=portNum;
	#ifdef CONFIG_MASTER_WLAN0_ENABLE
		newClientEntry->wlan_dev_idx=wlan_dev_idx;
	#endif
		if (ipVersion==IP_VERSION4)
		{
			newClientEntry->igmpVersion=IGMP_V3;
			newClientEntry->clientAddr[0]=clientAddr[0];
		}
		else
		{
			newClientEntry->igmpVersion=MLD_V2;
			newClientEntry->clientAddr[0]=clientAddr[0];
			newClientEntry->clientAddr[1]=clientAddr[1];
			newClientEntry->clientAddr[2]=clientAddr[2];
			newClientEntry->clientAddr[3]=clientAddr[3];
		}
#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		newClientEntry->pppoePassthroughEntry = rg_db.pktHdr->IGMP_report_passthrough_pkt;
		memcpy(newClientEntry->clientMacAddr,rg_db.pktHdr->pSmac,6);
#endif

		rtl_linkClientEntry(groupEntry, newClientEntry);
		clientEntry=newClientEntry;
	}

	if(rg_db.pktHdr->tagif & CVLAN_TAGIF)
	{

		clientEntry->reportCtagif = 1;
		clientEntry->reportVlanId = rg_db.pktHdr->ctagVid;

	}



	/*here to handle the source list*/
	if (ipVersion==IP_VERSION4)
		clientEntry->igmpVersion=IGMP_V3;
	else //if (ipVersion==IP_VERSION6)
		clientEntry->igmpVersion=MLD_V2;


	if (rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
	{
		IGMP("enableFastLeave==TRUE");
		clientEntry->groupFilterTimer=rtl_sysUpSeconds;
		rtl_deleteSourceList(clientEntry);
		/*link the new source list*/
		for (j=0; j<numOfSrc; j++)
		{
			newSourceEntry=rtl_allocateSourceEntry();
			if (newSourceEntry==NULL)
			{
				rtlglue_printf("run out of source entry!\n");
				return FAIL;
			}

			if (ipVersion==IP_VERSION4)
			{
				newSourceEntry->sourceAddr[0]=ntohl(p_NsourceAddr[0]);
				p_NsourceAddr++;
			}
			else
			{
				newSourceEntry->sourceAddr[0]=p_NsourceAddr[0];
				newSourceEntry->sourceAddr[1]=p_NsourceAddr[1];
				newSourceEntry->sourceAddr[2]=p_NsourceAddr[2];
				newSourceEntry->sourceAddr[3]=p_NsourceAddr[3];
				p_NsourceAddr=p_NsourceAddr+4;
			}
			newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			rtl_linkSourceEntry(clientEntry,newSourceEntry);
		}
	}
	else
	{

		// query in-mo for Q(A-B)
		// query ex-mo for Q(G,X-A) & Q(G)
		//struct rtl_sourceEntry *sourceEntry=clientEntry->sourceList;
		sourceEntry=clientEntry->sourceList;
		while (sourceEntry!=NULL)
		{
			if (sourceEntry->portTimer>rtl_sysUpSeconds) //ex-mo X or in-mo A
			{
				sourceEntry->setOpFlag=1;
				numOfQuerySrc++;
			}
			else
				sourceEntry->setOpFlag=0;
			
			//sync all source timer to rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTimel
			if(sourceEntry->portTimer >rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)
			{
				sourceEntry->portTimer = rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
			}				
			sourceEntry=sourceEntry->next;
		}

		DEBUG("numOfQuerySrc=%d, numOfSrc=%d", numOfQuerySrc, numOfSrc);


		/*add new source list*/
		for (j=0; j<numOfSrc; j++)
		{
			if (ipVersion==IP_VERSION4)
				_HsourceAddr[0] = ntohl(*p_NsourceAddr);
			else
				memcpy(_HsourceAddr,p_NsourceAddr,sizeof(_HsourceAddr));

			sourceEntry=rtl_searchSourceEntry(ipVersion, _HsourceAddr, clientEntry);
			if (sourceEntry!=NULL)
			{
				sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
				if (sourceEntry->setOpFlag)
				{
					numOfQuerySrc--;
					sourceEntry->setOpFlag=0;
				}
			}
			else
			{
				newSourceEntry=rtl_allocateSourceEntry();
				if (newSourceEntry==NULL)
				{
					rtlglue_printf("run out of source entry!\n");
					return FAIL;
				}

				if (ipVersion==IP_VERSION4)
				{
					newSourceEntry->sourceAddr[0]=ntohl(p_NsourceAddr[0]);
				}
				else
				{
					newSourceEntry->sourceAddr[0]=p_NsourceAddr[0];
					newSourceEntry->sourceAddr[1]=p_NsourceAddr[1];
					newSourceEntry->sourceAddr[2]=p_NsourceAddr[2];
					newSourceEntry->sourceAddr[3]=p_NsourceAddr[3];
				}

				newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;


				newSourceEntry->setOpFlag=0;
				rtl_linkSourceEntry(clientEntry,newSourceEntry);
			}

			if (ipVersion==IP_VERSION4)
			{
				p_NsourceAddr++;
			}
			else
			{
				p_NsourceAddr=p_NsourceAddr+4;
			}

		}

		if (clientEntry->groupFilterTimer>rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime) //ex-mo
		{
			clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
			IGMP("Shrink ex-mo client to lastMemberAgingTime=%d groupFilterTimer=%d rtl_sysUpSeconds=%d", rtl_mCastTimerParas.lastMemberAgingTime,clientEntry->groupFilterTimer,rtl_sysUpSeconds);
		}

/*
		DEBUG("before query, check original rg_db.pktHdr->internalVlanID=%d",rg_db.pktHdr->internalVlanID);
		DEBUG("before query, check original rg_db.pktHdr->rx.src_port_num=%d,rg_db.pktHdr->ingressPort=%d",
			rg_db.pktHdr->pRxDesc->rx_src_port_num, rg_db.pktHdr->ingressPort);
		DEBUG("numOfQuerySrc=%d, numOfSrc=%d", numOfQuerySrc, numOfSrc);
*/
		// query in-mo for Q(G,A-B)
		// query ex-mo for Q(G,X-A) & Q(G)
		IGMP("Group-Source Specific Query");
		rtl_igmpQueryTimerExpired(0, IGMP_V3, (char *)groupAddress, numOfQuerySrc, clientEntry->sourceList, NULL);
		if (clientEntry->groupFilterTimer>rtl_sysUpSeconds && numOfQuerySrc!=0) //ex-mo
		{
			IGMP("Group Specific Query");
			rtl_igmpQueryTimerExpired(0, IGMP_V3, (char *)groupAddress, 0, NULL, NULL);
		}

	}

	//reportEventContext.srcFilterMode = RTK_RG_IPV4MC_INCLUDE; ////
	reportEventContext.ipVersion=ipVersion;
	//reportEventContext.vlanId=rg_db.pktHdr->internalVlanID;

	IGMP("After Send Query Done");
	/*
	DEBUG("after query, check curr rg_db.pktHdr->internalVlanID=%d",rg_db.pktHdr->internalVlanID);
	DEBUG("after query, check curr rg_db.pktHdr->rx.src_port_num=%d,rg_db.pktHdr->ingressPort=%d",
		rg_db.pktHdr->pRxDesc->rx_src_port_num, rg_db.pktHdr->ingressPort);
	DEBUG("after query, check curr rg_db.pktHdr->IGMPType=%d (v3 0x22)",rg_db.pktHdr->IGMPType);
	*/

	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];

#ifdef CONFIG_RECORD_MCAST_FLOW
	//IGMPv3 always update flow
	rtl_rg_mcDataReFlushAndAdd(groupAddress);
#endif
	#if defined (CONFIG_RG_HARDWARE_MULTICAST)
	{
		/*we only support ipv4 hardware multicast*/
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
		{
			#ifndef CONFIG_RG_SIMPLE_IGMP_v3
			rtl_handle_igmpgroup_change(&reportEventContext);
			#else
			rtl_handle_igmpgroup_change_v3(&reportEventContext);
			#endif
		}

	}
	#endif

	return SUCCESS;
}

static  int32 rtl_processToExclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8 *pktBuf)
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;

	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *p_NsourceAddr=NULL;				//Net endian
	uint32 _HsourceAddr[4]={0, 0, 0, 0};	//Host endian

	uint32 tmpAddress[4]={0, 0, 0, 0};
	uint16 numOfQuerySrc=0;

	if (ipVersion==IP_VERSION4)
	{
		IGMP("process IGMPv3-ToExclude");
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		p_NsourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
		if ((groupAddress[0] < 0xe0000100) || (groupAddress[0] == 0xeffffffa)) return FAIL;
	}
	else
	{
		IGMP("process MLDv2-ToExclude");
		memcpy(groupAddress,((struct mCastAddrRecord *)pktBuf)->mCastAddr,IPV6ADDRLEN);

		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		p_NsourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	}

	if(rtl_group_limitCheck(ipVersion,groupAddress,clientAddr,rg_db.pktHdr->smac,rg_db.pktHdr->internalVlanID,portNum)!=SUCCESS)
	{
		IGMP("Limit check FAIL Ingore Snooping learning");
		return 0;
	}


	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress, rg_db.pktHdr->internalVlanID);
	if (groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
			rtlglue_printf("run out of group entry!\n");
			return FAIL;
		}

		assert(newGroupEntry->clientList==NULL);
		newGroupEntry->vlanId=rg_db.pktHdr->internalVlanID;
		newGroupEntry->ipVersion=ipVersion;
		if (ipVersion==IP_VERSION4)
		{
			newGroupEntry->groupAddr[0]=groupAddress[0];
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
		else
		{
			newGroupEntry->groupAddr[0]=groupAddress[0];
			newGroupEntry->groupAddr[1]=groupAddress[1];
			newGroupEntry->groupAddr[2]=groupAddress[2];
			newGroupEntry->groupAddr[3]=groupAddress[3];
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}

		groupEntry=newGroupEntry;
	}

	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
	if (clientEntry==NULL)
	{
		newClientEntry=rtl_allocateClientEntry();
		if (newClientEntry==NULL)
		{
			rtlglue_printf("run out of client entry!\n");
			return FAIL;
		}
		assert(newClientEntry->sourceList==NULL);
		newClientEntry->sourceList=NULL;
		newClientEntry->portNum=portNum;
	#ifdef CONFIG_MASTER_WLAN0_ENABLE

		newClientEntry->wlan_dev_idx=wlan_dev_idx;
	#endif

		if (ipVersion==IP_VERSION4)
		{
			newClientEntry->igmpVersion=IGMP_V3;
			newClientEntry->clientAddr[0]=clientAddr[0];
		}
		else
		{
			newClientEntry->igmpVersion=MLD_V2;
			newClientEntry->clientAddr[0]=clientAddr[0];
			newClientEntry->clientAddr[1]=clientAddr[1];
			newClientEntry->clientAddr[2]=clientAddr[2];
			newClientEntry->clientAddr[3]=clientAddr[3];
		}

#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		newClientEntry->pppoePassthroughEntry = rg_db.pktHdr->IGMP_report_passthrough_pkt;
		memcpy(newClientEntry->clientMacAddr,rg_db.pktHdr->pSmac,6);
#endif
		rtl_linkClientEntry(groupEntry, newClientEntry);
		clientEntry=newClientEntry;
	}

	if(rg_db.pktHdr->tagif & CVLAN_TAGIF)
	{

		clientEntry->reportCtagif = 1;
		clientEntry->reportVlanId = rg_db.pktHdr->ctagVid;

	}



	/*flush the old source list*/

	// delete ((X-A) | (Y-A)) or (A-B)
	sourceEntry=clientEntry->sourceList;
	while (sourceEntry!=NULL)
	{
		sourceEntry->setOpFlag=1;
		sourceEntry=sourceEntry->next;
	}

	if (ipVersion==IP_VERSION4)
		clientEntry->igmpVersion=IGMP_V3;
	else //if(ipVersion==IP_VERSION6)
		clientEntry->igmpVersion=MLD_V2;

	/*link the new source list*/
	for (j=0; j<numOfSrc; j++)
	{
		if (ipVersion==IP_VERSION4)
			_HsourceAddr[0] = ntohl(*p_NsourceAddr);
		else
			memcpy(_HsourceAddr,p_NsourceAddr,sizeof(_HsourceAddr));

		sourceEntry=rtl_searchSourceEntry(ipVersion, _HsourceAddr, clientEntry);
		if (clientEntry->groupFilterTimer>rtl_sysUpSeconds) //ex-mo
		{
			if (sourceEntry!=NULL) // A*X + A*Y
			{
				sourceEntry->setOpFlag=0;
				if (sourceEntry->portTimer>rtl_sysUpSeconds) //A*X
				{
				}
				else
				{
					if (ipVersion==IP_VERSION4)
						p_NsourceAddr++;
					else
						p_NsourceAddr=p_NsourceAddr+4;

					continue;
				}
			}
			else	// A-X-Y
			{
				newSourceEntry=rtl_allocateSourceEntry();
				if (newSourceEntry==NULL)
				{
					rtlglue_printf("run out of source entry!\n");
					return FAIL;
				}

				if (ipVersion==IP_VERSION4)
				{
					newSourceEntry->sourceAddr[0]=ntohl(p_NsourceAddr[0]);
				}
				else //if (ipVersion==IP_VERSION4)
				{
					newSourceEntry->sourceAddr[0]=p_NsourceAddr[0];
					newSourceEntry->sourceAddr[1]=p_NsourceAddr[1];
					newSourceEntry->sourceAddr[2]=p_NsourceAddr[2];
					newSourceEntry->sourceAddr[3]=p_NsourceAddr[3];
				}

				/*time out the sources included in the MODE_IS_EXCLUDE report*/
				newSourceEntry->portTimer=rtl_sysUpSeconds;
				rtl_linkSourceEntry(clientEntry,newSourceEntry);

				//newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
				newSourceEntry->portTimer=clientEntry->groupFilterTimer;
				newSourceEntry->setOpFlag=0;
			}

			if (ipVersion==IP_VERSION4)
			{
				//querySourceAddr
				DEBUG("Block.GIP.Clt.SrcIP(%pI4n) in A-Y",(p_NsourceAddr));////

				*tmpAddress = *p_NsourceAddr;
				*p_NsourceAddr = *(p_NsourceAddr + (numOfSrc-j-1));
				*(p_NsourceAddr + (numOfSrc-j-1)) = *tmpAddress;
				DEBUG("sourceAddr Head(%pI4n) Tail(%pI4n)",(p_NsourceAddr), ((p_NsourceAddr+(numOfSrc-j-1))));
			}
			else
			{
				int i ;
				DEBUG("Block.GIP6.Clt.SrcIP6("IP6H") in A-Y",NIP6QUAD(p_NsourceAddr));////

				for (i=0; i<4; i++)
				{
					*(tmpAddress+i) = *(p_NsourceAddr+i);
					*(p_NsourceAddr+i) = *(p_NsourceAddr+i + ((numOfSrc -j - 1)));
					*(p_NsourceAddr+i + ((numOfSrc - j - 1))) = *(tmpAddress+i);
				}
			}


			j--;
			numOfSrc--;
			numOfQuerySrc++;
			continue;	//no need to move sourceAddr

		}
		else //in-mo
		{
			if (sourceEntry!=NULL) // A*B
			{
				sourceEntry->setOpFlag=0;

				if (ipVersion==IP_VERSION4)
				{
					//querySourceAddr
					DEBUG("To_Ex.GIP.Clt.SrcIP(%pI4n) in A*B",(p_NsourceAddr));////

					*tmpAddress = *p_NsourceAddr;
					*p_NsourceAddr = *(p_NsourceAddr + (numOfSrc-j-1));
					*(p_NsourceAddr + (numOfSrc-j-1)) = *tmpAddress;
					DEBUG("sourceAddr Head(%pI4n) Tail(%pI4n)",(p_NsourceAddr), ((p_NsourceAddr+(numOfSrc-j-1))));
				}
				else
				{
					int i ;
					DEBUG("To_Ex.GIP6.Clt.SrcIP6("IP6H") in A*B",NIP6QUAD(p_NsourceAddr));////

					for (i=0; i<4; i++)
					{
						*(tmpAddress+i) = *(p_NsourceAddr+i);
						*(p_NsourceAddr+i) = *(p_NsourceAddr+i + ((numOfSrc -j - 1)));
						*(p_NsourceAddr+i + ((numOfSrc - j - 1))) = *(tmpAddress+i);
					}
				}

				j--;
				numOfSrc--;
				numOfQuerySrc++;
				continue;	//no need to move sourceAddr

			}
			else //B-A
			{
				newSourceEntry=rtl_allocateSourceEntry();
				if (newSourceEntry==NULL)
				{
					rtlglue_printf("run out of source entry!\n");
					return FAIL;
				}

				if (ipVersion==IP_VERSION4)
				{
					newSourceEntry->sourceAddr[0]=ntohl(p_NsourceAddr[0]);
					p_NsourceAddr++;
				}
				else //if (ipVersion==IP_VERSION4)
				{
					newSourceEntry->sourceAddr[0]=p_NsourceAddr[0];
					newSourceEntry->sourceAddr[1]=p_NsourceAddr[1];
					newSourceEntry->sourceAddr[2]=p_NsourceAddr[2];
					newSourceEntry->sourceAddr[3]=p_NsourceAddr[3];
					p_NsourceAddr=p_NsourceAddr+4;
				}
				/*time out the sources included in the MODE_IS_EXCLUDE report*/
				newSourceEntry->portTimer=rtl_sysUpSeconds;
				rtl_linkSourceEntry(clientEntry,newSourceEntry);
				newSourceEntry->setOpFlag=0;
			}

		}
	}

	// delete ((X-A) | (Y-A)) or (A-B)
	{
		struct rtl_sourceEntry *nextSourceEntry;
		sourceEntry=clientEntry->sourceList;
		while (sourceEntry!=NULL)
		{
			nextSourceEntry=sourceEntry->next;
			if (sourceEntry->setOpFlag)
			{
				rtl_deleteSourceEntry(clientEntry, sourceEntry);
			}
			sourceEntry=nextSourceEntry;
		}
	}

	clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;

	if (ipVersion==IP_VERSION4)
	{
		int i ;
		for (i=0; i<numOfQuerySrc; i++)
		{
			DEBUG("Query GIP(%pI4h)-SrcIP(%pI4n)", (groupAddress), ((p_NsourceAddr+i)));
		}
		//IGMPQueryVersion=3;
		rtl_igmpQueryTimerExpired(0, IGMP_V3, (char *)groupAddress, numOfQuerySrc, NULL, (char *)p_NsourceAddr);
	}
	else
	{
		int i ;
		for (i=0; i<numOfQuerySrc; i+=4)
		{
			DEBUG("Query GIP6("IP6H")-SrcIP("IP6H")", NIP6QUAD(groupAddress), NIP6QUAD((p_NsourceAddr+i)));
		}
		//IGMPQueryVersion=3; ???????????????????????????????????????????????
		//rtl_mldQueryTimerExpired(0);
	}


	//reportEventContext.srcFilterMode = RTK_RG_IPV4MC_EXCLUDE; ////
	reportEventContext.ipVersion=ipVersion;
	reportEventContext.vlanId=rg_db.pktHdr->internalVlanID;

	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];

#ifdef CONFIG_RECORD_MCAST_FLOW
	//IGMPv3 always update flow
	rtl_rg_mcDataReFlushAndAdd(groupAddress);
#endif
	#if defined (CONFIG_RG_HARDWARE_MULTICAST)
	{
		/*we only support ipv4 hardware multicast*/
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		#ifndef CONFIG_RG_SIMPLE_IGMP_v3
		rtl_handle_igmpgroup_change(&reportEventContext);
		#else
		rtl_handle_igmpgroup_change_v3(&reportEventContext);
		#endif

	}
	#endif

	return SUCCESS;
}

static  int32 rtl_processAllow(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8 *pktBuf)
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;

	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *p_NsourceAddr=NULL;				//Net endian
	uint32 _HsourceAddr[4]={0, 0, 0, 0};	//Host endian


	if (ipVersion==IP_VERSION4)
	{
		IGMP("process IGMPv3 Allow packet");
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		p_NsourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
		if ((groupAddress[0] < 0xe0000100) || (groupAddress[0] == 0xeffffffa)) return FAIL;
	}
	else
	{
		IGMP("process MLDv2 Allow packet");
		memcpy(groupAddress,((struct mCastAddrRecord *)pktBuf)->mCastAddr,IPV6ADDRLEN);

		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		p_NsourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	}

	if(rtl_group_limitCheck(ipVersion,groupAddress,clientAddr,rg_db.pktHdr->smac,rg_db.pktHdr->internalVlanID,portNum)!=SUCCESS)
	{
		IGMP("Limit check FAIL Ingore Snooping learning");
		return 0;
	}


	hashIndex=rtl_igmpHashAlgorithm( ipVersion, groupAddress);
	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress, rg_db.pktHdr->internalVlanID);
	if (groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if (newGroupEntry==NULL)
		{
			rtlglue_printf("run out of group entry!\n");
			return FAIL;
		}

		newGroupEntry->vlanId=rg_db.pktHdr->internalVlanID;
		newGroupEntry->ipVersion=ipVersion;
		if (ipVersion==IP_VERSION4)
		{
			newGroupEntry->groupAddr[0]=groupAddress[0];
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
		else
		{
			newGroupEntry->groupAddr[0]=groupAddress[0];
			newGroupEntry->groupAddr[1]=groupAddress[1];
			newGroupEntry->groupAddr[2]=groupAddress[2];
			newGroupEntry->groupAddr[3]=groupAddress[3];
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}

		groupEntry=newGroupEntry;
	}

	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
	if (clientEntry==NULL)
	{
		newClientEntry=rtl_allocateClientEntry();
		if(newClientEntry==NULL)
		{
			rtlglue_printf("run out of client entry!\n");
			return FAIL;
		}

		assert(newClientEntry->sourceList==NULL);
		newClientEntry->sourceList=NULL;
		newClientEntry->portNum=portNum;
	#ifdef CONFIG_MASTER_WLAN0_ENABLE
		newClientEntry->wlan_dev_idx=wlan_dev_idx;
	#endif

		newClientEntry->igmpVersion=IGMP_V3;
		newClientEntry->groupFilterTimer=rtl_sysUpSeconds;
		if(ipVersion==IP_VERSION4)
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
		}
		else
		{
			newClientEntry->igmpVersion=MLD_V2;
			newClientEntry->clientAddr[0]=clientAddr[0];
			newClientEntry->clientAddr[1]=clientAddr[1];
			newClientEntry->clientAddr[2]=clientAddr[2];
			newClientEntry->clientAddr[3]=clientAddr[3];
		}
#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		newClientEntry->pppoePassthroughEntry = rg_db.pktHdr->IGMP_report_passthrough_pkt;
		memcpy(newClientEntry->clientMacAddr,rg_db.pktHdr->pSmac,6);
#endif

		rtl_linkClientEntry(groupEntry, newClientEntry);
		clientEntry=newClientEntry;
	}

	if(rg_db.pktHdr->tagif & CVLAN_TAGIF)
	{

		clientEntry->reportCtagif = 1;
		clientEntry->reportVlanId = rg_db.pktHdr->ctagVid;

	}


	if (ipVersion==IP_VERSION4)
		clientEntry->igmpVersion=IGMP_V3;
	else	//if (ipVersion==IP_VERSION6)
		clientEntry->igmpVersion=MLD_V2;


	/*here to handle the source list*/
	for (j=0; j<numOfSrc; j++)
	{
		if (ipVersion==IP_VERSION4)
			_HsourceAddr[0] = ntohl(*p_NsourceAddr);
		else
			memcpy(_HsourceAddr,p_NsourceAddr,sizeof(_HsourceAddr));

		sourceEntry=rtl_searchSourceEntry(ipVersion, _HsourceAddr, clientEntry);
		if (sourceEntry==NULL)
		{
			newSourceEntry=rtl_allocateSourceEntry();
			if (newSourceEntry==NULL)
			{
				rtlglue_printf("run out of source entry!\n");
				return FAIL;
			}

			if (ipVersion==IP_VERSION4)
			{
				newSourceEntry->sourceAddr[0]=ntohl(p_NsourceAddr[0]);
			}
			else
			{
				newSourceEntry->sourceAddr[0]=p_NsourceAddr[0];
				newSourceEntry->sourceAddr[1]=p_NsourceAddr[1];
				newSourceEntry->sourceAddr[2]=p_NsourceAddr[2];
				newSourceEntry->sourceAddr[3]=p_NsourceAddr[3];
			}
			newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			rtl_linkSourceEntry(clientEntry,newSourceEntry);
		}
		else
		{
			/*just update source timer*/
			sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
		}

		if (ipVersion==IP_VERSION4)
			p_NsourceAddr++;
		else
			p_NsourceAddr=p_NsourceAddr+4;

	}

	reportEventContext.ipVersion=ipVersion;
	reportEventContext.vlanId=rg_db.pktHdr->internalVlanID;
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];

#ifdef CONFIG_RECORD_MCAST_FLOW
	//IGMPv3 always update flow
	rtl_rg_mcDataReFlushAndAdd(groupAddress);
#endif
	#if defined (CONFIG_RG_HARDWARE_MULTICAST)
	{
		/*we only support ipv4 hardware multicast*/
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		#ifndef CONFIG_RG_SIMPLE_IGMP_v3
		rtl_handle_igmpgroup_change(&reportEventContext);
		#else
		rtl_handle_igmpgroup_change_v3(&reportEventContext);
		#endif

	}
	#endif
	return SUCCESS;
}

static int32 rtl_processBlock(uint32 moduleIndex, uint32 ipVersion,uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8 *pktBuf)
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};

	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	//struct rtl_clientEntry* newClientEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;

	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *p_NsourceAddr=NULL;				//Net endian
	uint32 _HsourceAddr[4]={0, 0, 0, 0};	//Host endian


	uint16 numOfQuerySrc=0;
	//uint32 *querySourceAddr=NULL;
	uint32 tmpAddress[4]={0, 0, 0, 0};

	if (ipVersion==IP_VERSION4)
	{
		IGMP("process IGMPv3-Block");
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		p_NsourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
		if ((groupAddress[0] < 0xe0000100) || (groupAddress[0] == 0xeffffffa)) return FAIL;
	}
	else
	{
		IGMP("process MLDv2-Block");
		memcpy(groupAddress,((struct mCastAddrRecord *)pktBuf)->mCastAddr,IPV6ADDRLEN);

		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		p_NsourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	}


	if(rtl_group_limitCheck(ipVersion,groupAddress,clientAddr,rg_db.pktHdr->smac,rg_db.pktHdr->internalVlanID,portNum)!=SUCCESS)
	{
		IGMP("Limit check FAIL Ingore Snooping learning");
		return 0;
	}


	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress, rg_db.pktHdr->internalVlanID);
	if (groupEntry==NULL)
	{
		goto out;
	}

	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
	if (clientEntry==NULL)
	{
		goto out;
	}

	if (ipVersion==IP_VERSION4)
		clientEntry->igmpVersion=IGMP_V3;
	else //if (ipVersion==IP_VERSION6)
		clientEntry->igmpVersion=MLD_V2;


	if (clientEntry->groupFilterTimer>rtl_sysUpSeconds) /*means exclude mode*/
	{
		DEBUG("<Ex-Mo> GIP.Clt");
		numOfQuerySrc=0;
		for (j=0; j<numOfSrc; j++)
		{
			DEBUG("current j(%d), numOfSrc(%d), numOfQuerySrc(%d)", j, numOfSrc, numOfQuerySrc);////
			if (ipVersion==IP_VERSION4)
				_HsourceAddr[0] = ntohl(*p_NsourceAddr);
			else
				memcpy(_HsourceAddr,p_NsourceAddr,sizeof(_HsourceAddr));

			sourceEntry=rtl_searchSourceEntry(ipVersion, _HsourceAddr, clientEntry);
			if (sourceEntry==NULL) // A-X-Y
			{
				newSourceEntry=rtl_allocateSourceEntry();
				if (newSourceEntry==NULL)
				{
					rtlglue_printf("run out of source entry!\n");
					return FAIL;
				}

				if (ipVersion==IP_VERSION4)
				{
					newSourceEntry->sourceAddr[0]=ntohl(p_NsourceAddr[0]);
				}
				else
				{
					newSourceEntry->sourceAddr[0]=p_NsourceAddr[0];
					newSourceEntry->sourceAddr[1]=p_NsourceAddr[1];
					newSourceEntry->sourceAddr[2]=p_NsourceAddr[2];
					newSourceEntry->sourceAddr[3]=p_NsourceAddr[3];
				}

				//newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
				newSourceEntry->portTimer=clientEntry->groupFilterTimer;

				rtl_linkSourceEntry(clientEntry,newSourceEntry);
			}
			else // A*X+A*Y
			{
				if (rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
				{
					sourceEntry->portTimer=rtl_sysUpSeconds;
				}


				if (sourceEntry->portTimer>rtl_sysUpSeconds) // A*X
				{
				}
				else	//A*Y
				{
					if (ipVersion==IP_VERSION4)
					{
						p_NsourceAddr++;
					}
					else
					{
						p_NsourceAddr=p_NsourceAddr+4;
					}
					continue;
				}

			}

			// A-X-Y + A*X = A-Y
			if (ipVersion==IP_VERSION4)
			{
				//querySourceAddr
				DEBUG("Block.GIP.Clt.SrcIP(%pI4n) in A-Y",(p_NsourceAddr));////

				*tmpAddress = *p_NsourceAddr;
				*p_NsourceAddr = *(p_NsourceAddr + (numOfSrc-j-1));
				*(p_NsourceAddr + (numOfSrc-j-1)) = *tmpAddress;
				DEBUG("sourceAddr Head(%pI4n) Tail(%pI4n)",(p_NsourceAddr), ((p_NsourceAddr+(numOfSrc-j-1))));
			}
			else
			{
				int i ;
				DEBUG("Block.GIP6.Clt.SrcIP6("IP6H") in A-Y",NIP6QUAD(p_NsourceAddr));////

				for (i=0; i<4; i++)
				{
					*(tmpAddress+i) = *(p_NsourceAddr+i);
					*(p_NsourceAddr+i) = *(p_NsourceAddr+i + ((numOfSrc -j - 1)));
					*(p_NsourceAddr+i + ((numOfSrc - j - 1))) = *(tmpAddress+i);
				}
			}

			j--;
			numOfSrc--;
			numOfQuerySrc++;
			continue;	//no need to move sourceAddr

/*
			if (ipVersion==IP_VERSION4)
			{
				p_NsourceAddr++;
			}
			else
			{
				p_NsourceAddr=p_NsourceAddr+4;
			}
*/
		}
	}
	else           /*means include mode*/
	{
		DEBUG("<In-Mo> GIP.Clt");

		numOfQuerySrc=0;
		for (j=0; j<numOfSrc; j++)
        {
			DEBUG("current j(%d), numOfSrc(%d), numOfQuerySrc(%d)", j, numOfSrc, numOfQuerySrc);////
			if (ipVersion==IP_VERSION4)
				_HsourceAddr[0] = ntohl(*p_NsourceAddr);
			else
				memcpy(_HsourceAddr,p_NsourceAddr,sizeof(_HsourceAddr));

			sourceEntry=rtl_searchSourceEntry(ipVersion, _HsourceAddr,clientEntry);
			if (sourceEntry!=NULL) //
			{
				if (rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
				{
					sourceEntry->portTimer=rtl_sysUpSeconds;
				}


				if (ipVersion==IP_VERSION4)
				{
					//querySourceAddr
					DEBUG("Block.GIP.Clt.SrcIP(%pI4n) in A*B",(p_NsourceAddr));////

					*tmpAddress = *p_NsourceAddr;

					*p_NsourceAddr = *(p_NsourceAddr + (numOfSrc-j-1));
					*(p_NsourceAddr + (numOfSrc-j-1)) = *tmpAddress;
					DEBUG("sourceAddr Head(%pI4n) Tail(%pI4n)",(p_NsourceAddr), ((p_NsourceAddr+(numOfSrc-j-1))));

				}
				else
				{
					int i ;
					DEBUG("Block.GIP6.Clt.SrcIP6("IP6H") in A*B",NIP6QUAD(p_NsourceAddr));////

					for (i=0; i<4; i++)
					{
						*(tmpAddress+i) = *(p_NsourceAddr+i);

						*(p_NsourceAddr+i) = *(p_NsourceAddr+i + ((numOfSrc-j-1)));
						*(p_NsourceAddr+i + ((numOfSrc-j-1))) = *(tmpAddress+i);

					}
				}
				j--;
				numOfSrc--;
				numOfQuerySrc++;
				continue;	//no need to move sourceAddr
			}
			else
			{
				if (ipVersion==IP_VERSION4)
				{
					//querySourceAddr
					DEBUG("Block.GIP.Clt.SrcIP(%pI4n) no in A*B",(p_NsourceAddr));////
				}
				else
				{
					DEBUG("Block.GIP6.Clt.SrcIP6("IP6H") no in A*B",NIP6QUAD(p_NsourceAddr));////
				}

			}

			if (ipVersion==IP_VERSION4)
			{
				p_NsourceAddr++;
			}
			else
			{
				p_NsourceAddr=p_NsourceAddr+4;
			}

			//DEBUG("");
 		}

	}

	if (ipVersion==IP_VERSION4)
	{
		int i ;
		for (i=0; i<numOfQuerySrc; i++)
		{
			DEBUG("Query GIP(%pI4h)-SrcIP(%pI4n)", (groupAddress), ((p_NsourceAddr+i)));
		}
		//IGMPQueryVersion=3;
		rtl_igmpQueryTimerExpired(0, IGMP_V3, (char *)groupAddress, numOfQuerySrc, NULL, (char *)p_NsourceAddr);
	}
	else
	{
		int i ;
		for (i=0; i<numOfQuerySrc; i+=4)
		{
			DEBUG("Query GIP6("IP6H")-SrcIP("IP6H")", NIP6QUAD(groupAddress), NIP6QUAD((p_NsourceAddr+i)));
		}
		//IGMPQueryVersion=3; ???????????????????????????????????????????????
		//rtl_mldQueryTimerExpired(0);
	}


out:
	reportEventContext.ipVersion=ipVersion;
	reportEventContext.vlanId=rg_db.pktHdr->internalVlanID;

	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];

#ifdef CONFIG_RECORD_MCAST_FLOW
	//IGMPv3 always update flow
	rtl_rg_mcDataReFlushAndAdd(groupAddress);
#endif

	#if defined (CONFIG_RG_HARDWARE_MULTICAST)
	{
		/*we only support ipv4 hardware multicast*/
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
		{
			#ifndef CONFIG_RG_SIMPLE_IGMP_v3
			rtl_handle_igmpgroup_change(&reportEventContext);
			#else
			rtl_handle_igmpgroup_change_v3(&reportEventContext);
			#endif
		}
	}
	#endif

	return SUCCESS;
}


static uint32 rtl_processIgmpv3Mldv2Reports(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8 *pktBuf)
{
	uint32 i=0;
	uint16 numOfRecords=0;
	uint8 *groupRecords=NULL;
	uint8 recordType=0xff;
	uint16 numOfSrc=0;
	int32 returnVal=0;
	uint32 multicastRouterPortMask=rtl_getMulticastRouterPortMask(moduleIndex, ipVersion, rtl_sysUpSeconds);
	IGMP("IGMPv3/MLDv2 Report packet");

	if (ipVersion==IP_VERSION4)
	{
		numOfRecords=ntohs(((struct igmpv3Report *)pktBuf)->numOfRecords);
		if (numOfRecords!=0)
		{
			groupRecords=(uint8 *)(&(((struct igmpv3Report *)pktBuf)->recordList));
		}
	}
	else
	{
		numOfRecords=ntohs(((struct mldv2Report *)pktBuf)->numOfRecords);
		if (numOfRecords!=0)
		{
			groupRecords=(uint8 *)(&(((struct mldv2Report *)pktBuf)->recordList));
		}
	}

#if defined(CONFIG_MASTER_WLAN0_ENABLE) && defined(CONFIG_RG_FLOW_NEW_WIFI_MODE)
	{
		rtk_rg_port_idx_t tmp_portNum=portNum;
		_rtk_rg_lutExtport_translator(&tmp_portNum);
		if ((1<<tmp_portNum)&(~rg_db.systemGlobal.igmpReportIngressPortmask))
		{
			rg_db.pktHdr->ingressIgmpMldDrop = 1;
			return 0; //Drop
		}
	}
#else
	if ((1<<portNum)&(~rg_db.systemGlobal.igmpReportIngressPortmask))
	{
		rg_db.pktHdr->ingressIgmpMldDrop = 1;
		return 0; //Drop
	}
#endif


	for (i=0; i<numOfRecords; i++)
	{
		if (ipVersion==IP_VERSION4)
		{
			recordType=((struct groupRecord *)groupRecords)->type;
		}
		else
		{
			recordType=((struct mCastAddrRecord *)groupRecords)->type;
		}


		switch (recordType)
		{
			case MODE_IS_INCLUDE:
				returnVal=rtl_processIsInclude(moduleIndex, ipVersion, portNum, wlan_dev_idx, clientAddr, groupRecords);
			break;

			case MODE_IS_EXCLUDE:
				returnVal=rtl_processIsExclude(moduleIndex, ipVersion, portNum, wlan_dev_idx, clientAddr, groupRecords);
			break;

			case CHANGE_TO_INCLUDE_MODE:
				returnVal=rtl_processToInclude(moduleIndex, ipVersion, portNum, wlan_dev_idx, clientAddr, groupRecords);
			break;

			case CHANGE_TO_EXCLUDE_MODE:
				returnVal=rtl_processToExclude(moduleIndex, ipVersion, portNum, wlan_dev_idx, clientAddr, groupRecords);
			break;

			case ALLOW_NEW_SOURCES:
				returnVal=rtl_processAllow(moduleIndex, ipVersion, portNum, wlan_dev_idx, clientAddr, groupRecords);
			break;

			case BLOCK_OLD_SOURCES:
				returnVal=rtl_processBlock(moduleIndex, ipVersion, portNum, wlan_dev_idx, clientAddr ,groupRecords);
			break;

			default:break;

		}

		if (ipVersion==IP_VERSION4)
		{
			numOfSrc=ntohs(((struct groupRecord *)groupRecords)->numOfSrc);
			/*shift pointer to another group record*/
			groupRecords=groupRecords+8+numOfSrc*4+(((struct groupRecord *)(groupRecords))->auxLen)*4;
		}
		else
		{
			numOfSrc=ntohs(((struct mCastAddrRecord *)groupRecords)->numOfSrc);
			/*shift pointer to another group record*/
			groupRecords=groupRecords+20+numOfSrc*16+(((struct mCastAddrRecord *)(groupRecords))->auxLen)*4;
		}

	}

	return (multicastRouterPortMask & (~(1<<portNum)) & IGMP_SUPPORT_PORT_MASK);

}

int32 rtl_igmp_filterWhiteListCheck(uint32 ipVersion, uint32 *groupAddr , uint32 *clientAddr,uint8 *saMac)
{

	int i,j;
	int ret=FAIL;
	int whiteListEnable=0;
	int smacHit=0,sipHit=0,dipHit=0;

	//not support mld whiteList
	if(ipVersion==IP_VERSION6)
		return SUCCESS;

	for(i=0 ; i<MAX_IGMP_WHITE_LIST_SIZE ;i++)
	{
		if(!rg_db.igmpWhiteList[i].valid) continue;
		whiteListEnable=1;

		if(rg_db.igmpWhiteList[i].smacChk)
		{
			smacHit=1;
			for(j=0 ;j<6 ;j++)
			{
				if((rg_db.igmpWhiteList[i].smac[j] & rg_db.igmpWhiteList[i].smacMask[j]) != ((saMac[j]) & rg_db.igmpWhiteList[i].smacMask[j]))
					smacHit=0;
			}
		}
		else
			smacHit=1; //ingore smac compare

		if(rg_db.igmpWhiteList[i].sipChk)
		{
			if( (rg_db.igmpWhiteList[i].ipv4Sip & rg_db.igmpWhiteList[i].ipv4SipMask )==((*clientAddr) & rg_db.igmpWhiteList[i].ipv4SipMask))
				sipHit=1;
		}
		else
			sipHit=1;//ingore sip compare

		if(rg_db.igmpWhiteList[i].dipChk)
		{
			if( (rg_db.igmpWhiteList[i].ipv4Dip& rg_db.igmpWhiteList[i].ipv4DipMask)==((*groupAddr)& rg_db.igmpWhiteList[i].ipv4DipMask))
				dipHit=1;
		}
		else
			dipHit=1;//ingore dip compare

		if(smacHit && sipHit && dipHit)
		{
			IGMP("Hit WhiteList[%d]",i);
			ret= SUCCESS;
			break;
		}

	}

	if(whiteListEnable==0)
		ret=SUCCESS;

	return ret;

}

int32 rtl_igmp_filterBlackListCheck(uint32 ipVersion, uint32 *groupAddr , uint32 *clientAddr,uint8 *saMac)
{

	int i,j;
	int ret=SUCCESS;
	int smacHit=0,sipHit=0,dipHit=0;

	//not support mld blackList
	if(ipVersion==IP_VERSION6)
		return SUCCESS;

	for(i=0 ; i<MAX_IGMP_BLACK_LIST_SIZE ;i++)
	{
		if(!rg_db.igmpBlackList[i].valid) continue;

		if(rg_db.igmpBlackList[i].smacChk)
		{
			smacHit=1;
			for(j=0 ;j<6 ;j++)
			{
				if((rg_db.igmpBlackList[i].smac[j] & rg_db.igmpBlackList[i].smacMask[j]) != ((saMac[j]) & rg_db.igmpBlackList[i].smacMask[j]))
					smacHit=0;
			}
		}
		else
			smacHit=1; //ingore smac compare

		if(rg_db.igmpBlackList[i].sipChk)
		{
			if( (rg_db.igmpBlackList[i].ipv4Sip & rg_db.igmpBlackList[i].ipv4SipMask )==((*clientAddr) & rg_db.igmpBlackList[i].ipv4SipMask))
				sipHit=1;
		}
		else
			sipHit=1;//ingore sip compare

		if(rg_db.igmpBlackList[i].dipChk)
		{
			if( (rg_db.igmpBlackList[i].ipv4Dip& rg_db.igmpBlackList[i].ipv4DipMask)==((*groupAddr)& rg_db.igmpBlackList[i].ipv4DipMask))
				dipHit=1;
		}
		else
			dipHit=1;//ingore dip compare

		if(smacHit && sipHit && dipHit)
		{
			IGMP("Hit BlackList[%d]",i);
			ret= FAIL;
			break;
		}

	}

	return ret;

}




static uint32 rtl_processIgmpMld(uint32 moduleIndex, rtk_rg_pktHdr_t *pPktHdr, uint32 ipVersion, uint32 portNum, uint32 wlan_dev_idx, uint32 *clientAddr, uint8* pktBuf, uint32 pktLen)
{
	uint32 fwdPortMask=0;

	//IGMP("pktLen=%d", pktLen);
	//IGMP("%02x %02x %02x %02x-%02x %02x %02x %02x",pktBuf[ 0],pktBuf[ 1],pktBuf[ 2],pktBuf[ 3],pktBuf[ 4],pktBuf[ 5],pktBuf[ 6],pktBuf[ 7]);
	//IGMP("%02x %02x %02x %02x-%02x %02x %02x %02x",pktBuf[ 8],pktBuf[ 9],pktBuf[10],pktBuf[11],pktBuf[12],pktBuf[13],pktBuf[14],pktBuf[15]);
	//IGMP("");
	//IGMP("%02x %02x %02x %02x-%02x %02x %02x %02x",pktBuf[16],pktBuf[17],pktBuf[18],pktBuf[19],pktBuf[20],pktBuf[21],pktBuf[22],pktBuf[23]);
	//IGMP("%02x %02x %02x %02x-%02x %02x %02x %02x",pktBuf[24],pktBuf[25],pktBuf[26],pktBuf[27],pktBuf[28],pktBuf[29],pktBuf[30],pktBuf[31]);
	reportEventContext.moduleIndex=moduleIndex;

	switch (pktBuf[0])
	{
		case IGMP_QUERY:
			IGMP("IGMP Queries");
			if(rg_db.systemGlobal.multicastVersionSupport & (RG_MC_IGMPV1| RG_MC_IGMPV2|RG_MC_IGMPV3))
				fwdPortMask=rtl_processQueries(moduleIndex, pPktHdr, ipVersion, portNum, wlan_dev_idx, pktBuf, pktLen);
			else
				IGMP("IGMP Snooping Version Deny");
		break;

		case IGMPV1_REPORT:
			IGMP("IGMPv1 Report");
			if(rg_db.systemGlobal.multicastVersionSupport & (RG_MC_IGMPV1| RG_MC_IGMPV2|RG_MC_IGMPV3))
				fwdPortMask=rtl_processJoin(moduleIndex, ipVersion, portNum, wlan_dev_idx, clientAddr,pktBuf);
			else
				IGMP("IGMP Snooping Version Deny");
		break;

		case IGMPV2_REPORT:
			IGMP("IGMPv2 Report");
			if(rg_db.systemGlobal.multicastVersionSupport & (RG_MC_IGMPV2|RG_MC_IGMPV3))
				fwdPortMask=rtl_processJoin(moduleIndex, ipVersion, portNum, wlan_dev_idx, clientAddr, pktBuf);
			else
				IGMP("IGMP Snooping Version Deny");
		break;

		case IGMPV2_LEAVE:
 			IGMP("IGMPv2 Leave");
			if(rg_db.systemGlobal.multicastVersionSupport & (RG_MC_IGMPV2|RG_MC_IGMPV3))
				fwdPortMask=rtl_processLeave(moduleIndex, ipVersion, portNum, wlan_dev_idx, clientAddr,pktBuf);
			else
				IGMP("IGMP Snooping Version Deny");
		break;

		case IGMPV3_REPORT:
			IGMP("IGMPv3 Report");
			if(rg_db.systemGlobal.multicastVersionSupport & (RG_MC_IGMPV3))
				fwdPortMask=rtl_processIgmpv3Mldv2Reports(moduleIndex, ipVersion, portNum, wlan_dev_idx, clientAddr, pktBuf);
			else
				IGMP("IGMP Snooping Version Deny");
		break;

		case MLD_QUERY:
			IGMP("MLD Query");
			if(rg_db.systemGlobal.multicastVersionSupport & (RG_MC_MLDV1 | RG_MC_MLDV2))
				fwdPortMask=rtl_processQueries(moduleIndex, pPktHdr, ipVersion, portNum, wlan_dev_idx, pktBuf, pktLen);
			else
				IGMP("MLD Snooping Version Deny");
		break;

		case MLDV1_REPORT:
 			 IGMP("MLDv1 Report");
			 if(rg_db.systemGlobal.multicastVersionSupport & (RG_MC_MLDV1 | RG_MC_MLDV2))
				 fwdPortMask=rtl_processJoin(moduleIndex, ipVersion, portNum, wlan_dev_idx, clientAddr, pktBuf);
			 else
			 	IGMP("MLD Snooping Version Deny");
		break;

		case MLDV1_DONE:
			 IGMP("MLDv1 Done");
			 if(rg_db.systemGlobal.multicastVersionSupport & (RG_MC_MLDV1 | RG_MC_MLDV2))
				 fwdPortMask=rtl_processLeave(moduleIndex, ipVersion, portNum, wlan_dev_idx, clientAddr, pktBuf);
			 else
			 	IGMP("MLD Snooping Version Deny");
		break;

		case MLDV2_REPORT:
			IGMP("MLDv2 Report");
			if(rg_db.systemGlobal.multicastVersionSupport & ( RG_MC_MLDV2))
				fwdPortMask=rtl_processIgmpv3Mldv2Reports(moduleIndex, ipVersion, portNum, wlan_dev_idx, clientAddr, pktBuf);
			else
				IGMP("MLD Snooping Version Deny");
		break;

		default:
			fwdPortMask=((~(1<<portNum)) & IGMP_SUPPORT_PORT_MASK);
			IGMP("fwdPortMask=0x%x",fwdPortMask);
		break;
	}

	return fwdPortMask;
}

static uint32 rtl_processDvmrp(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8* pktBuf, uint32 pktLen)
{
	IGMP("process Router Dvmrp packet");
	{
		rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.dvmrpRouter.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.dvmrpRouterAgingTime; /*update timer*/
	}

	{

		reportEventContext.ipVersion=ipVersion;
		reportEventContext.groupAddr[0]=0;
		reportEventContext.groupAddr[1]=0;
		reportEventContext.groupAddr[2]=0;
		reportEventContext.groupAddr[3]=0;

		#ifdef CONFIG_RECORD_MCAST_FLOW
		//rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
		#endif

		#if defined (CONFIG_RG_HARDWARE_MULTICAST)
		/*we only support ipv4 hardware multicast*/
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl_handle_igmpgroup_change(&reportEventContext);
		#endif
	}

	return ((~(1<<portNum)) & IGMP_SUPPORT_PORT_MASK);
}

static uint32 rtl_processMospf(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8* pktBuf, uint32 pktLen)
{
	struct ipv4MospfHdr *ipv4MospfHeader=(struct ipv4MospfHdr*)pktBuf;
	struct ipv4MospfHello *ipv4HelloPkt=(struct ipv4MospfHello*)pktBuf;

	struct ipv6MospfHdr *ipv6MospfHeader=(struct ipv6MospfHdr*)pktBuf;
	struct ipv6MospfHello *ipv6HelloPkt=(struct ipv6MospfHello*)pktBuf;

	IGMP("process Router Mospf packet");

	if(ipVersion==IP_VERSION4)
	{
		/*mospf is built based on ospfv2*/
		if((ipv4MospfHeader->version==2) && (ipv4MospfHeader->type==MOSPF_HELLO_TYPE))
		{
			if((ipv4HelloPkt->options & 0x04)!=0)
			{
				rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.mospfRouter.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.mospfRouterAgingTime; /*update timer*/
			}
		}
	}
	else
	{
		if((ipv6MospfHeader->version==3) && (ipv6MospfHeader->type==MOSPF_HELLO_TYPE))
		{
			if((ipv6HelloPkt->options[2] & 0x04)!=0)
			{
				rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.mospfRouter.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.mospfRouterAgingTime; /*update timer*/

			}
		}
	}




	reportEventContext.ipVersion=ipVersion;
	reportEventContext.groupAddr[0]=0;
	reportEventContext.groupAddr[1]=0;
	reportEventContext.groupAddr[2]=0;
	reportEventContext.groupAddr[3]=0;

	#ifdef CONFIG_RECORD_MCAST_FLOW
	//rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif

	#if defined (CONFIG_RG_HARDWARE_MULTICAST)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl_handle_igmpgroup_change(&reportEventContext);
	}
	#endif

	return ((~(1<<portNum)) & IGMP_SUPPORT_PORT_MASK);

}

static uint32 rtl_processPim(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8* pktBuf, uint32 pktLen)
{
	IGMP("process Router PIM packet");
	if(ipVersion==IP_VERSION4)
	{
		rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.pimRouter.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.pimRouterAgingTime; /*update timer*/

	}
	else
	{
		rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.pimRouter.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.pimRouterAgingTime; /*update timer*/
	}




	reportEventContext.ipVersion=ipVersion;
	reportEventContext.groupAddr[0]=0;
	reportEventContext.groupAddr[1]=0;
	reportEventContext.groupAddr[2]=0;
	reportEventContext.groupAddr[3]=0;

	#ifdef CONFIG_RECORD_MCAST_FLOW
	//rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif

	#if defined (CONFIG_RG_HARDWARE_MULTICAST)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl_handle_igmpgroup_change(&reportEventContext);
	}
	#endif

	return ((~(1<<portNum)) & IGMP_SUPPORT_PORT_MASK);
}



/*********************************************
External Function
*********************************************/


//External called function by high level program
int32 rtl_registerIgmpSnoopingModule(uint32 *moduleIndex)
{
	int32 i=0;
	uint32 index=0xFFFFFFFF;

	*moduleIndex=0xFFFFFFFF;

	for (i=0; i<MAX_MCAST_MODULE_NUM; i++)
	{
		if (rtl_mCastModuleArray[i].enableSnooping==FALSE)
		{
			index=i;
			break;
		}
	}

	if (i>=MAX_MCAST_MODULE_NUM)
	{
		IGMP("register IgmpSnooping Module Fail");
		return FAIL;
	}
	IGMP("register IgmpSnooping Module [%d]", i);

	if (rtl_mCastModuleArray[index].enableSnooping==FALSE)
	{
		/*initialize multicast Routers information*/
		for (i=0; i<RTK_RG_PORT_MAX; i++)
		{
			if(RG_INVALID_PORT(i)) continue;
			rtl_mCastModuleArray[index].rtl_ipv4MulticastRouters.querier.portTimer[i]=0;
			rtl_mCastModuleArray[index].rtl_ipv4MulticastRouters.dvmrpRouter.portTimer[i]=0;
			rtl_mCastModuleArray[index].rtl_ipv4MulticastRouters.pimRouter.portTimer[i]=0;
			rtl_mCastModuleArray[index].rtl_ipv4MulticastRouters.mospfRouter.portTimer[i]=0;

			rtl_mCastModuleArray[index].rtl_ipv6MulticastRouters.querier.portTimer[i]=0;
			rtl_mCastModuleArray[index].rtl_ipv6MulticastRouters.dvmrpRouter.portTimer[i]=0;
			rtl_mCastModuleArray[index].rtl_ipv6MulticastRouters.pimRouter.portTimer[i]=0;
			rtl_mCastModuleArray[index].rtl_ipv6MulticastRouters.mospfRouter.portTimer[i]=0;

		}

		/*initialize hash table*/
		rtl_initHashTable(index, rtl_hashTableSize);

		if((rtl_mCastModuleArray[index].rtl_ipv4HashTable==NULL) )
		{
			return FAIL;
		}

		if(rtl_mCastModuleArray[index].rtl_ipv6HashTable==NULL)
		{
			return FAIL;
		}


		#if defined (CONFIG_RG_HARDWARE_MULTICAST)
		memset(&rtl_mCastModuleArray[index].deviceInfo,0, sizeof(rtl_multicastDeviceInfo_t));
		#endif
      		for(i=0; i<6; i++)
      		{
			rtl_mCastModuleArray[index].rtl_gatewayMac[i]=0;
      		}

		rtl_mCastModuleArray[index]. rtl_gatewayIpv4Addr=0;

		for(i=0; i<4; i++)
		{
			rtl_mCastModuleArray[index].rtl_gatewayIpv6Addr[i]=0;
		}

		rtl_mCastModuleArray[index].enableFastLeave=FALSE;
		rtl_mCastModuleArray[index].enableSnooping=TRUE;
		rtl_mCastModuleArray[index].unknownMCastFloodMap=0;
		rtl_mCastModuleArray[index].staticRouterPortMask=0;
		#ifdef CONFIG_PROC_FS
		rtl_mCastModuleArray[index].expireEventCnt=0;
		#endif
		*moduleIndex=index;
		return SUCCESS;
	}
	else
	{
		return FAIL;
	}
}



int32 rtl_unregisterIgmpSnoopingModule(uint32 moduleIndex)
{
	uint32 i=0;
	struct rtl_groupEntry *groupEntryPtr=NULL;
	#ifdef CONFIG_RECORD_MCAST_FLOW
	struct rtl_mcastFlowEntry *mcastFlowEntryPtr=NULL;
	#endif
	IGMP("unregister IgmpSnooping Module[%d]", moduleIndex);

	if (moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAIL;
	}

	if (rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
	{
		rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv4Addr=0;

		for (i=0; i<6; i++)
     		{
	   		rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[i]=0;
		}

		for (i=0;i<4;i++)
		{
			rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[i]=0;
		}


	 	/*delete ipv4 multicast entry*/
	 	for (i=0;i<rtl_hashTableSize;i++)
	     	{
			groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i];

			while(groupEntryPtr!=NULL)
			{
				rtl_deleteGroupEntry(groupEntryPtr, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
				groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i];
			}
		}
		rtk_rg_free(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable=NULL;
		memset(&(rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters), 0, sizeof(struct rtl_multicastRouters));

		/*delete ipv6 multicast entry*/
		for(i=0; i<rtl_hashTableSize; i++)
		{

			groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[i];
			while(groupEntryPtr!=NULL)
			{
				rtl_deleteGroupEntry(groupEntryPtr, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
				groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[i];
			}
		}
		rtk_rg_free(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable=NULL;
		memset(&(rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters), 0, sizeof(struct rtl_multicastRouters));


		#ifdef CONFIG_RECORD_MCAST_FLOW
		/*delete multicast flow entry*/
	 	for (i=0;i<rtl_hashTableSize;i++)
	     	{
			mcastFlowEntryPtr=rtl_mCastModuleArray[moduleIndex].flowHashTable[i];

			while(mcastFlowEntryPtr!=NULL)
			{
				rtl_deleteMcastFlowEntry(mcastFlowEntryPtr, rtl_mCastModuleArray[moduleIndex].flowHashTable);
				mcastFlowEntryPtr=rtl_mCastModuleArray[moduleIndex].flowHashTable[i];
			}
		}
		rtk_rg_free(rtl_mCastModuleArray[moduleIndex].flowHashTable);
		rtl_mCastModuleArray[moduleIndex].flowHashTable=NULL;
		#endif
		rtl_mCastModuleArray[moduleIndex].enableSnooping=FALSE;
		rtl_mCastModuleArray[moduleIndex].enableFastLeave=FALSE;
		rtl_mCastModuleArray[moduleIndex].unknownMCastFloodMap=0;
		rtl_mCastModuleArray[moduleIndex].staticRouterPortMask=0;

		#if defined (CONFIG_RG_HARDWARE_MULTICAST)
		memset(&rtl_mCastModuleArray[moduleIndex].deviceInfo,0,sizeof(rtl_multicastDeviceInfo_t));
		#endif
		return SUCCESS;
	}

	return SUCCESS;
}

static void _rtl865x_configIgmpSnoopingExpire(int32 disableExpire)
{
	uint32 maxTime=0xffffffff;

	//IGMP("got in _rtl865x_configIgmpSnoopingExpire");
	if((rtl_mCastTimerParas.disableExpire==FALSE) && (disableExpire==TRUE))
	{
		rtl_mCastTimerParas.disableExpire=TRUE;
	}

	if((rtl_mCastTimerParas.disableExpire==TRUE) && (disableExpire==FALSE) )
	{

#if defined(__linux__) && defined(__KERNEL__)

		if(rtk_rg_getTimeSec()>=rtl_sysUpSeconds)
		{
			rtl_startTime=(uint32)(rtk_rg_getTimeSec())-rtl_sysUpSeconds;
		}
		else
		{
			/*avoid timer wrap back*/
			rtl_startTime=maxTime-rtl_sysUpSeconds+(uint32)(rtk_rg_getTimeSec())+1;
		}
#endif
		rtl_mCastTimerParas.disableExpire=FALSE;
	}


	return;
}

//External called function by high level program
void rtl_setMulticastParameters(struct rtl_mCastTimerParameters mCastTimerParameters)
{
	//IGMP("got in rtl_mCastTimerParameters");
	_rtl865x_configIgmpSnoopingExpire(mCastTimerParameters.disableExpire);

	if(mCastTimerParameters.groupMemberAgingTime!=0)
	{
		rtl_mCastTimerParas.groupMemberAgingTime= mCastTimerParameters.groupMemberAgingTime;
	}

	if(mCastTimerParameters.lastMemberAgingTime!=0)
	{
		rtl_mCastTimerParas.lastMemberAgingTime= mCastTimerParameters.lastMemberAgingTime;
	}

	if(mCastTimerParameters.querierPresentInterval!=0)
	{

		rtl_mCastTimerParas.querierPresentInterval=mCastTimerParameters.querierPresentInterval;
	}


	if(mCastTimerParameters.dvmrpRouterAgingTime!=0)
	{

		rtl_mCastTimerParas.dvmrpRouterAgingTime=mCastTimerParameters.dvmrpRouterAgingTime;
	}

	if(mCastTimerParameters.mospfRouterAgingTime!=0)
	{

		rtl_mCastTimerParas.mospfRouterAgingTime=mCastTimerParameters.mospfRouterAgingTime;
	}

	if(mCastTimerParameters.pimRouterAgingTime!=0)
	{

		rtl_mCastTimerParas.pimRouterAgingTime=mCastTimerParameters.pimRouterAgingTime;
	}

	return;
}


int32 rtl_configIgmpSnoopingModule(uint32 moduleIndex, struct rtl_mCastSnoopingLocalConfig *mCastSnoopingLocalConfig)
{
	//IGMP("got in rtl_configIgmpSnoopingModule");
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAIL;
	}

	if(mCastSnoopingLocalConfig==NULL)
	{
		return FAIL;
	}

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAIL;
	}

	rtl_mCastModuleArray[moduleIndex].enableFastLeave=mCastSnoopingLocalConfig->enableFastLeave;
	rtl_mCastModuleArray[moduleIndex].unknownMCastFloodMap=mCastSnoopingLocalConfig->unknownMcastFloodMap;
	rtl_mCastModuleArray[moduleIndex].staticRouterPortMask=mCastSnoopingLocalConfig->staticRouterPortMask;

	rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[0]=mCastSnoopingLocalConfig->gatewayMac[0];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[1]=mCastSnoopingLocalConfig->gatewayMac[1];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[2]=mCastSnoopingLocalConfig->gatewayMac[2];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[3]=mCastSnoopingLocalConfig->gatewayMac[3];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[4]=mCastSnoopingLocalConfig->gatewayMac[4];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[5]=mCastSnoopingLocalConfig->gatewayMac[5];

	rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv4Addr=mCastSnoopingLocalConfig->gatewayIpv4Addr;

	rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[0]=mCastSnoopingLocalConfig->gatewayIpv6Addr[0];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[1]=mCastSnoopingLocalConfig->gatewayIpv6Addr[1];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[2]=mCastSnoopingLocalConfig->gatewayIpv6Addr[2];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[3]=mCastSnoopingLocalConfig->gatewayIpv6Addr[3];

	return SUCCESS;
}

/*
@func int32	| rtl_maintainMulticastSnoopingTimerList	|   Multicast snooping timer list maintenance function.
@parm  uint32	| currentSystemTime	|The current system time (unit: seconds).
@rvalue SUCCESS	| Always return SUCCESS.
@comm
 This function should be called once a second to maintain multicast timer list.
*/
int32 rtl_maintainMulticastSnoopingTimerList(uint32 currentSystemTime)
{
	/* maintain current time */
	uint32 i=0;
	uint32 maxTime=0xffffffff;

	struct rtl_groupEntry* groupEntryPtr=NULL;
	struct rtl_groupEntry* nextEntry=NULL;

	uint32 moduleIndex;

//	#ifdef CONFIG_RECORD_MCAST_FLOW
// 	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM; moduleIndex++)
// 	{
// 		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
// 		{
// 			if((currentSystemTime%DEFAULT_MCAST_FLOW_EXPIRE_TIME)==0)
// 			{
// 				rtl_doMcastFlowRecycle(moduleIndex, BOTH_IPV4_IPV6);
// 			}
//
// 		}
// 	}
// 	#endif

	if(rtl_mCastTimerParas.disableExpire==TRUE)
	{
		return SUCCESS;
	}

	/*handle timer conter overflow*/
	if(currentSystemTime>rtl_startTime)
	{
		rtl_sysUpSeconds=currentSystemTime-rtl_startTime;
	}
	else
	{
		rtl_sysUpSeconds=(maxTime-rtl_startTime)+currentSystemTime+1;
	}

	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM; moduleIndex++)
	{
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{
			#if defined (CONFIG_RG_HARDWARE_MULTICAST)
			strcpy(timerEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
			timerEventContext.moduleIndex=moduleIndex;
			#endif

			/*maintain ipv4 group entry  timer */
			for(i=0; i<rtl_hashTableSize; i++)
			{
				/*scan the hash table*/
				if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!=NULL)
				{
				 	timerEventContext.ipVersion=IP_VERSION4;
					groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i];
					while(groupEntryPtr)              /*traverse each group list*/
					{
						nextEntry=groupEntryPtr->next;
						timerEventContext.vlanId=groupEntryPtr->vlanId;
						timerEventContext.groupAddr[0]=groupEntryPtr->groupAddr[0];
						timerEventContext.groupAddr[1]=0;
						timerEventContext.groupAddr[2]=0;
						timerEventContext.groupAddr[3]=0;
						//timerEventContext.srcFilterMode=groupEntryPtr->srcFilterMode; ////
						rtl_checkGroupEntryTimer(groupEntryPtr, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
						groupEntryPtr=nextEntry;/*because expired group entry  will be cleared*/
					}
				}
			}

			/*maintain ipv6 group entry  timer */
			for(i=0; i<rtl_hashTableSize; i++)
			{
				  /*scan the hash table*/
				if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable!=NULL)
				{
					timerEventContext.ipVersion=IP_VERSION6;
					groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[i];
					while(groupEntryPtr)              /*traverse each group list*/
					{
						nextEntry=groupEntryPtr->next;
						timerEventContext.vlanId=groupEntryPtr->vlanId;
						timerEventContext.groupAddr[0]=groupEntryPtr->groupAddr[0];
						timerEventContext.groupAddr[1]=groupEntryPtr->groupAddr[1];
						timerEventContext.groupAddr[2]=groupEntryPtr->groupAddr[2];
						timerEventContext.groupAddr[3]=groupEntryPtr->groupAddr[3];
						//timerEventContext.srcFilterMode=groupEntryPtr->srcFilterMode; ////
						rtl_checkGroupEntryTimer(groupEntryPtr, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
						groupEntryPtr=nextEntry;/*because expired group entry  will be cleared*/
					}
				}
			}

		}
	}
	return SUCCESS;
}


int32 rtl_getQueryPortMask(uint32 moduleIndex, rtk_rg_pktHdr_t * pPktHdr, uint32 *fwdPortMask, uint32 *fwdMbssidMask)
{
	int j;
	*fwdPortMask=0;
	*fwdMbssidMask=0;

	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		TRACE("Error: IGMP module id is invalid");
		return FAIL;
	}

	if (pPktHdr->IGMPQueryVer>=IGMP_MLD_VER_MAX)
	{
		TRACE("Error: Unknown Query Version");
		return FAIL;
	}

	if (pPktHdr->IGMPQueryVer>=MLD_V1)
		IGMP("IGMPQueryVer=MLD_V%d",  pPktHdr->IGMPQueryVer-MLD_V1+1);
	else
		IGMP("IGMPQueryVer=IGMP_V%d", pPktHdr->IGMPQueryVer-IGMP_V1+1);

	for (j=0; j<RTK_RG_MAC_PORT_MAX; j++)
	{
		if(RG_INVALID_MAC_PORT(j)) continue;
		if (igmpSnoopingCounterVer_Port[j][pPktHdr->IGMPQueryVer-IGMP_V1])
		{
			*fwdPortMask |= (1<<j);
		}
		DEBUG("igmpSnoopingCounterVer_Port[%d][%d]=%d", j, pPktHdr->IGMPQueryVer-IGMP_V1,igmpSnoopingCounterVer_Port[j][pPktHdr->IGMPQueryVer-IGMP_V1]);
	}
	//IGMP("igmpSnoopingVer[%d][%d] PortMask=0x%x", j, pPktHdr->IGMPQueryVer-IGMP_V1+1, *fwdPortMask);

	#ifdef CONFIG_MASTER_WLAN0_ENABLE
	for (j=0; j<MAX_WLAN_DEVICE_NUM; j++)
	{
		if (igmpSnoopingCounterVer_MBSSID[j][pPktHdr->IGMPQueryVer-IGMP_V1])
		{
			*fwdMbssidMask |= (1<<j);
		}
		DEBUG("igmpSnoopingCounterVer_MBSSID[%d][%d]=%d", j, pPktHdr->IGMPQueryVer-IGMP_V1,igmpSnoopingCounterVer_MBSSID[j][pPktHdr->IGMPQueryVer-IGMP_V1]);
	}
	//IGMP("igmpSnoopingVer[%d][%d] MBSSIDMask=0x%x", j, pPktHdr->IGMPQueryVer-IGMP_V1+1, *fwdPortMask);
	#endif
	/*if (pPktHdr->ingressPort) RTK_RG_MAC_PORT_CPU
	*fwdPortMask &= (~(1<<portNum));
	*/
	return SUCCESS;
}

int32 rtl_igmpMldProcess(uint32 moduleIndex, uint8 *macFrame, rtk_rg_pktHdr_t *pPktHdr, uint32 portNum, uint32 *fwdPortMask)
{

	struct rtl_macFrameInfo macFrameInfo;

	IGMP("IGMP&MLD Process(moduleIndex:%d portIdx=%d, wlanDevIdx=%d)\n",moduleIndex, portNum, pPktHdr->wlan_dev_idx);

	#if defined (CONFIG_RG_HARDWARE_MULTICAST)
	reportEventContext.portMask=1<<portNum;
	#endif

	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAIL;
	}

#if defined(CONFIG_RG_RTL9600_SERIES)
	//for specific query PPPoE passthrough	directTx will hit lut entry and remove pppoe Tag ,so we change to DIP 224.0.0.1 General_Query_DIP
	if( (rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT5)&& (pPktHdr->tagif & PPPOE_TAGIF)	&& (pPktHdr->tagif & IPV4_TAGIF) && (pPktHdr->tagif&IGMP_TAGIF)&& (!pPktHdr->isGatewayPacket) )
	{
		//bridge wan pppoe passthrough + routing wan multicast , we need to change query packet dip
		if (((macFrame + pPktHdr->l3Offset + pPktHdr->ipv4HeaderLen)[0] == IGMP_QUERY) && (pPktHdr->ipv4Dip != 0xe0000001))
		{
			*pPktHdr->pIpv4Dip = htonl(0xe0000001);
			pPktHdr->ipv4Dip = 0xe0000001;
			TRACE("old cs(%x)", ntohs(*pPktHdr->pIpv4Checksum));
			*pPktHdr->pIpv4Checksum=htons(_rtk_rg_fwdengine_L3checksumUpdate(ntohs(*pPktHdr->pIpv4Checksum),pPktHdr->ipv4Dip,pPktHdr->ipv4TTL,pPktHdr->ipProtocol,ntohl(*pPktHdr->pIpv4Dip),*pPktHdr->pIpv4TTL));
			TRACE("Modify packet to formal query new cs(%x)", ntohs(*pPktHdr->pIpv4Checksum));
		}
		rg_db.pktHdr->IGMP_report_passthrough_pkt=1;
		if(rg_db.systemGlobal.igmp_pppoe_passthrough_learning)
		{
			//continue to learning pppoePassthrough on snooping
			IGMP("continue to learning pppoePassthrough on snooping");
		}
		else
		{
			IGMP("IGMP PPPoE Passthrough packet skip learning snooping module !");
			return FAIL;
		}

	}
	else if ((pPktHdr->tagif & PPPOE_TAGIF) && (!pPktHdr->isGatewayPacket))
	{
		rg_db.pktHdr->IGMP_report_passthrough_pkt=1;
		if(rg_db.systemGlobal.igmp_pppoe_passthrough_learning)
		{
			//continue to learning pppoePassthrough on snooping
			IGMP("continue to learning pppoePassthrough on snooping");
		}
		else
		{
			IGMP("IGMP PPPoE Passthrough packet skip learning snooping module !");
			return FAIL;
		}
	}

#elif defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	if ((pPktHdr->tagif & PPPOE_TAGIF) && (!pPktHdr->isGatewayPacket))
		{
			rg_db.pktHdr->IGMP_report_passthrough_pkt=1;
			if(rg_db.systemGlobal.igmp_pppoe_passthrough_learning)
			{
				//continue to learning pppoePassthrough on snooping
				IGMP("continue to learning pppoePassthrough on snooping");
			}
			else
			{
				IGMP("IGMP PPPoE Passthrough packet skip learning snooping module !");
				return FAIL;
			}
		}
#else
	if ((pPktHdr->tagif & PPPOE_TAGIF) && (!pPktHdr->isGatewayPacket))
	{
		TRACE("IGMP PPPoE Passthrough packet skip learning snooping module !");
		return FAIL;
	}
#endif


	rtl_parseMacFrame(moduleIndex, macFrame, pPktHdr, FALSE, &macFrameInfo);


	if (rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
	{
		if((macFrameInfo.ipBuf==NULL) || RG_INVALID_PORT(portNum) || (macFrameInfo.checksumFlag!=SUCCESS) || ((macFrameInfo.ipVersion!=IP_VERSION4) && (macFrameInfo.ipVersion!=IP_VERSION6)))
		{
			IGMP(" Error portNum=%d  checksumFlag=%d ipVersion=%d",portNum,macFrameInfo.checksumFlag,macFrameInfo.ipVersion);
			return FAIL;
		}


		switch(macFrameInfo.l3Protocol)
		{
			case IGMP_PROTOCOL:
				IGMP("process IGMP");
				*fwdPortMask=rtl_processIgmpMld(moduleIndex, pPktHdr, (uint32)(macFrameInfo.ipVersion), portNum, pPktHdr->wlan_dev_idx, macFrameInfo.srcIpAddr, macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen);
			break;

			case ICMP_PROTOCOL:
				IGMP("process MLD");
				*fwdPortMask=rtl_processIgmpMld(moduleIndex, pPktHdr, (uint32)(macFrameInfo.ipVersion), portNum, pPktHdr->wlan_dev_idx, macFrameInfo.srcIpAddr, macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen);
			break;


			case DVMRP_PROTOCOL:
				IGMP("process DVMRP");
				*fwdPortMask=rtl_processDvmrp(moduleIndex, (uint32)(macFrameInfo.ipVersion), portNum, macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen);
			break;

			case MOSPF_PROTOCOL:
				IGMP("process MOSPF");
				*fwdPortMask=rtl_processMospf(moduleIndex, (uint32)(macFrameInfo.ipVersion), portNum, macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen);
			break;

			case PIM_PROTOCOL:
				IGMP("process PIM");
				*fwdPortMask=rtl_processPim(moduleIndex, (uint32)(macFrameInfo.ipVersion),portNum, macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen);
			break;

			default: break;
		}

	}

	return SUCCESS;
}

#ifdef CONFIG_RECORD_MCAST_FLOW

static int32 rtl_recordMcastFlow(uint32 moduleIndex,uint32 ipVersion, uint32 *sourceIpAddr, uint32 *groupAddr, struct rtl_multicastFwdInfo * multicastFwdInfo ,uint16 sport,uint16 dport)
{
	struct rtl_mcastFlowEntry *mcastFlowEntry=NULL;
	int32 ret,flowIdx;

	IGMP("got in rtl_recordMcastFlow");
	ret= FAIL;
	flowIdx = FAIL;

	if(multicastFwdInfo==NULL)
	{
		return FAIL;
	}

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	if( ( (ipVersion==IP_VERSION4) && (sourceIpAddr[0]==0 || groupAddr[0]==0)) ||
		( (ipVersion==IP_VERSION6) && ((sourceIpAddr[0]==0 && sourceIpAddr[1]==0 && sourceIpAddr[2]==0 && sourceIpAddr[3]==0) || (groupAddr[0]==0 && groupAddr[1]==0 && groupAddr[2]==0 && groupAddr[3]==0))))
		{IGMP("Error v4/v6 source/group address zero"); return FAIL; }
	//if(!((1 <<rg_db.pktHdr->ingressPort)&rg_db.systemGlobal.wanPortMask.portmask)){IGMP("not support sever in lan"); return FAIL;}
#endif

	mcastFlowEntry=rtl_searchMcastFlowEntry(moduleIndex, ipVersion, sourceIpAddr, groupAddr,sport,dport);

	if(mcastFlowEntry==NULL)
	{
		if(multicastFwdInfo->srcFilterMode ==RTK_RG_IPV4MC_EXCLUDE)
		{
			if(multicastFwdInfo->l2PortMask==0) {IGMP("Pmsk ==0 not record to igmpMcflow"); return FAIL;}
		}
		else
		{
			if(multicastFwdInfo->fwdPortMask ==0) {IGMP("Pmsk ==0 not record to igmpMcflow"); return FAIL;}
		}

		mcastFlowEntry=rtl_allocateMcastFlowEntry();
		if(mcastFlowEntry==NULL)
		{
			rtl_doMcastFlowRecycle(moduleIndex, ipVersion);

			mcastFlowEntry=rtl_allocateMcastFlowEntry();
			if(mcastFlowEntry==NULL)
			{
				rtlglue_printf("run out of multicast flow entry!\n");
				return FAIL;
			}
		}

		if(ipVersion==IP_VERSION4)
		{
			mcastFlowEntry->serverAddr[0]=sourceIpAddr[0];
			mcastFlowEntry->groupAddr[0]=groupAddr[0];

		}
		else
		{
			mcastFlowEntry->serverAddr[0]=sourceIpAddr[0];
			mcastFlowEntry->serverAddr[1]=sourceIpAddr[1];
			mcastFlowEntry->serverAddr[2]=sourceIpAddr[2];
			mcastFlowEntry->serverAddr[3]=sourceIpAddr[3];

			mcastFlowEntry->groupAddr[0]=groupAddr[0];
			mcastFlowEntry->groupAddr[1]=groupAddr[1];
			mcastFlowEntry->groupAddr[2]=groupAddr[2];
			mcastFlowEntry->groupAddr[3]=groupAddr[3];
		}


		mcastFlowEntry->ipVersion=ipVersion;
		mcastFlowEntry->sport=sport;
		mcastFlowEntry->dport=dport;

		memcpy(&mcastFlowEntry->multicastFwdInfo, multicastFwdInfo, sizeof(struct rtl_multicastFwdInfo ));

		mcastFlowEntry->refreshTime=rtl_sysUpSeconds;

		rtl_linkMcastFlowEntry(mcastFlowEntry, rtl_mCastModuleArray[moduleIndex].flowHashTable);

		//return SUCCESS;

	}
	else
	{
		if(multicastFwdInfo->srcFilterMode ==RTK_RG_IPV4MC_EXCLUDE)
		{
			if(multicastFwdInfo->l2PortMask==0)
			{
				IGMP("RTK_RG_IPV4MC_EXCLUDE forward to no-member in software should update to hardware");
				rtl_deleteMcastFlowEntry(mcastFlowEntry,rtl_mCastModuleArray[moduleIndex].flowHashTable); //forward to no-member in software should update to hardware
				return SUCCESS;
			}
		}
		else
		{
			if(multicastFwdInfo->fwdPortMask ==0)
			{
				IGMP("forward to no-member in software should update to hardware");
				rtl_deleteMcastFlowEntry(mcastFlowEntry,rtl_mCastModuleArray[moduleIndex].flowHashTable); //forward to no-member in software should update to hardware
				return SUCCESS;
			}
		}


		/*update forward port mask information */
		memcpy(&mcastFlowEntry->multicastFwdInfo, multicastFwdInfo, sizeof(struct rtl_multicastFwdInfo ));
		mcastFlowEntry->refreshTime=rtl_sysUpSeconds;
		//return SUCCESS;
	}

	return SUCCESS;
}

static void rtl_invalidateMCastFlow(uint32 moduleIndex,uint32 ipVersion, uint32 *groupAddr)
{
	uint32 hashIndex;
	struct rtl_mcastFlowEntry* mcastFlowEntry = NULL;
	struct rtl_mcastFlowEntry* nextMcastFlowEntry = NULL;

	if (NULL==groupAddr)
	{
		return ;
	}

	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddr);

	mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[hashIndex];

	while (mcastFlowEntry!=NULL)
	{
		nextMcastFlowEntry=mcastFlowEntry->next;

		if(ipVersion==mcastFlowEntry->ipVersion)
		{

			if ((groupAddr[0]==0)&&(groupAddr[1]==0)&&(groupAddr[2]==0)&&(groupAddr[3]==0))
			{
				rtl_deleteMcastFlowEntry(mcastFlowEntry, rtl_mCastModuleArray[moduleIndex].flowHashTable);
			}
			else
			{
				if(ipVersion ==IP_VERSION4)
				{
					if ((mcastFlowEntry->groupAddr[0]==groupAddr[0]))
					{
						rtl_deleteMcastFlowEntry(mcastFlowEntry, rtl_mCastModuleArray[moduleIndex].flowHashTable);
					}

				}
				else if (ipVersion ==IP_VERSION6)
				{
					if ((mcastFlowEntry->groupAddr[0]==groupAddr[0])&&(mcastFlowEntry->groupAddr[1]==groupAddr[1])&&
					(mcastFlowEntry->groupAddr[2]==groupAddr[2])&&(mcastFlowEntry->groupAddr[3]==groupAddr[3]))
					{
						rtl_deleteMcastFlowEntry(mcastFlowEntry, rtl_mCastModuleArray[moduleIndex].flowHashTable);
					}

				}
			}
		}

		mcastFlowEntry = nextMcastFlowEntry;
	}

	return ;
}

static void rtl_doMcastFlowRecycle(uint32 moduleIndex, uint32 ipVersion)
{
	uint32 i;
	uint32 freeCnt=0;
	struct rtl_mcastFlowEntry* mcastFlowEntry = NULL;
	struct rtl_mcastFlowEntry* nextMcastFlowEntry = NULL;
	struct rtl_mcastFlowEntry* oldestMcastFlowEntry = NULL;


	for (i = 0 ; i < rtl_hashTableSize ; i++)
	{
		mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[i];

		if (oldestMcastFlowEntry==NULL)
		{
			oldestMcastFlowEntry=mcastFlowEntry;
		}

		while (mcastFlowEntry!=NULL)
		{
			nextMcastFlowEntry=mcastFlowEntry->next;
			/*keep the most recently used entry*/
			if ((mcastFlowEntry->refreshTime+DEFAULT_MCAST_FLOW_EXPIRE_TIME) < rtl_sysUpSeconds)
			{
				rtl_deleteMcastFlowEntry(mcastFlowEntry,  rtl_mCastModuleArray[moduleIndex].flowHashTable);
				freeCnt++;
			}
			mcastFlowEntry=nextMcastFlowEntry;
		}
	}

	if(freeCnt>0)
	{
		return;
	}

	/*if too many concurrent flow,we have to do LRU*/
	for (i = 0 ; i < rtl_hashTableSize ; i++)
	{
		mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[i];

		if (oldestMcastFlowEntry==NULL)
		{
			oldestMcastFlowEntry=mcastFlowEntry;
		}

		while (mcastFlowEntry!=NULL)
		{
			nextMcastFlowEntry=mcastFlowEntry->next;
			if (mcastFlowEntry->refreshTime < oldestMcastFlowEntry->refreshTime)
			{
				oldestMcastFlowEntry=mcastFlowEntry;
			}

			mcastFlowEntry=nextMcastFlowEntry;
		}
	}

	if (oldestMcastFlowEntry!=NULL)
	{
		rtl_deleteMcastFlowEntry(oldestMcastFlowEntry,  rtl_mCastModuleArray[moduleIndex].flowHashTable);
	}

	return ;
}

#endif

int32 rtl_getMulticastDataFwdPortMask(uint32 moduleIndex, struct rtl_multicastDataInfo *multicastDataInfo, uint32 *fwdPortMask)
{
	int32 retVal=FAIL;
	struct rtl_multicastFwdInfo multicastFwdInfo;

	//DEBUG("got in rtl_getMulticastDataFwdPortMask");
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAIL;
	}

	if(multicastDataInfo==NULL)
	{
		return FAIL;
	}

	if(fwdPortMask==NULL)
	{
		return FAIL;
	}

	retVal=rtl_getMulticastDataFwdInfo( moduleIndex, multicastDataInfo, &multicastFwdInfo);

	*fwdPortMask=multicastFwdInfo.fwdPortMask;

	if(retVal==SUCCESS)
	{
		if(multicastFwdInfo.unknownMCast==TRUE)
		{
			return FAIL;
		}
		else
		{
			return SUCCESS;
		}
	}

	return FAIL;

}
/*ccwei- for debug*/
#define CCDBG 0
#if CCDBG
#define DD_PRINT(fmt, args...)  printk(fmt, ##args)
#else
#define DD_PRINT(fmt, args...)
#endif




int32 _setfwdMsktoWlan0(uint32* wlan0DevMask,uint32 portNum,int32 wlan_dev_idx)
{
#ifdef CONFIG_MASTER_WLAN0_ENABLE

#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	if(portNum > RTK_RG_PORT_CPU)
	{
		if(wlan_dev_idx >= 0)
			*wlan0DevMask |= 1<< wlan_dev_idx;
		else	//can not decide where to send, so flood!
			*wlan0DevMask=0xffffffff;
	}
#elif defined(CONFIG_RG_RTL9607C_SERIES)
	if (RTK_RG_EXT_PORT0<=portNum && portNum <=RTK_RG_MAC10_EXT_PORT5)
	{
		if(wlan_dev_idx >= 0)
			*wlan0DevMask |= 1<< wlan_dev_idx;
		else	//can not decide where to send, so flood!
			*wlan0DevMask=0xffffffff;
	}

#else
#error
#endif

#endif

	return SUCCESS;
}



#if defined(CONFIG_RG_RTL9600_SERIES)


void _rtl_PPPoE_clientAddReservedACL(struct rtl_clientEntry * clientEntry,uint32 dip)
{
	int i;
	if(clientEntry->pppoePassthroughEntry && clientEntry->pppoeReservedAclIdx==FAIL)
	{
		rtk_rg_aclAndCf_multicast_pppoe_trap_t	   mc_pppoe_trap;
		//get a reserved acl index to use
		for(i=0 ; i<=(RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE7_TRAP - RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE0_TRAP ) ;i++)
		{
			if((rg_db.systemGlobal.pppoe_passthrough_learning_reservedACL_bitmap & (1<<i))==0)
			{
				rg_db.systemGlobal.pppoe_passthrough_learning_reservedACL_bitmap |= (1<<i);
				break;
			}
		}
		if(i > (RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE7_TRAP - RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE0_TRAP ))
		{
			WARNING("reserved acl full");
			i=0;
		}
		memcpy(mc_pppoe_trap.dmac.octet,clientEntry->clientMacAddr,6);
		mc_pppoe_trap.dip = dip;
		_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE0_TRAP+i,&mc_pppoe_trap);
		IGMP("add reserved acl:%d to pppoeGroup:%pI4  %pM",i,&dip,clientEntry->clientMacAddr);
		clientEntry ->pppoeReservedAclIdx = i;

	}

}

#endif

int32 rtl_getMulticastDataFwdInfo(uint32 moduleIndex, struct rtl_multicastDataInfo *multicastDataInfo, struct rtl_multicastFwdInfo *multicastFwdInfo)
{

	struct rtl_groupEntry * groupEntry=NULL;
	unsigned int blkPortMask=0;

	//IGMP("in rtl_getMulticastDataFwdInfo");

	if (moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		IGMP("IGMP module index(%d) is invalid, return\n", moduleIndex);
		return FAIL;
	}

	if (multicastFwdInfo==NULL || multicastDataInfo==NULL)
	{
		IGMP("multicastFwdInfo or multicastDataInfo is null, return\n");
		return FAIL;
	}


	memset(multicastFwdInfo, 0, sizeof(struct rtl_multicastFwdInfo));

#ifdef CONFIG_RECORD_MCAST_FLOW
	if( rg_db.pktHdr->tagif &TCP_TAGIF) {IGMP("TCP Multicast Error packet type!?"); }
#endif

	if ( (multicastDataInfo->ipVersion==IP_VERSION4 && IN_MULTICAST_RESV_IPV4_ALL(multicastDataInfo->groupAddr[0])) ||
		 (multicastDataInfo->ipVersion==IP_VERSION6 && IN_MULTICAST_RESV_IPV6_ALL(multicastDataInfo->groupAddr[0])) ||
		 (multicastDataInfo->ipVersion==IP_VERSION6 && IN_MULTICAST6_DATA_NOT_FFXE(multicastDataInfo->groupAddr[0])) )
	{
		multicastFwdInfo->reservedMCast=TRUE;
		multicastFwdInfo->fwdPortMask=0xFFFFFFFF;
		multicastFwdInfo->l2PortMask=0xFFFFFFFF;
#ifdef CONFIG_MASTER_WLAN0_ENABLE
		multicastFwdInfo->wlan0DevMask=0xFFFFFFFF;
#endif
		multicastFwdInfo->cpuFlag=TRUE;
		return FAIL;
	}

	//always research database
#if 0
	#ifdef CONFIG_RECORD_MCAST_FLOW
	if(rg_db.pktHdr->ingressLocation != RG_IGR_MC_DATA_BUF || rg_db.systemGlobal.initParam.igmpSnoopingEnable!=1)
	{
		struct rtl_mcastFlowEntry *mcastFlowEntry=NULL;
		mcastFlowEntry=rtl_searchMcastFlowEntry(moduleIndex, multicastDataInfo->ipVersion, multicastDataInfo->sourceIp, multicastDataInfo->groupAddr,rg_db.pktHdr->sport,rg_db.pktHdr->dport);
		if (mcastFlowEntry!=NULL)
		{
			memcpy(multicastFwdInfo, &mcastFlowEntry->multicastFwdInfo, sizeof(struct rtl_multicastFwdInfo));
			return SUCCESS;
		}
	}
	#endif
#endif

	groupEntry=rtl_searchGroupEntry(moduleIndex, multicastDataInfo->ipVersion, multicastDataInfo->groupAddr, multicastDataInfo->vlanId);
	if (groupEntry==NULL)
	{
		IGMP("GIP had not recorded do unknownMCastFloodMap");
		multicastFwdInfo->unknownMCast=TRUE;
		multicastFwdInfo->fwdPortMask= rtl_mCastModuleArray[moduleIndex].unknownMCastFloodMap;

		#if defined (CONFIG_RG_HARDWARE_MULTICAST)
		if((multicastFwdInfo->fwdPortMask & rtl_mCastModuleArray[moduleIndex].deviceInfo.swPortMask)!=0)
		{
			multicastFwdInfo->cpuFlag=TRUE;
		}
		#endif

		if (rg_db.systemGlobal.initParam.igmpSnoopingEnable==1)
		{
			multicastDataInfo->srcFilterMode=RTK_RG_IPV4MC_DONT_CARE_SRC;
		}
		else if (rg_db.systemGlobal.initParam.igmpSnoopingEnable==2)
		{
			if (multicastDataInfo->ipVersion==IP_VERSION4)
			{
				multicastDataInfo->srcFilterMode=RTK_RG_IPV4MC_EXCLUDE;
			}else //IP_VERSION6
			{
				multicastDataInfo->srcFilterMode=RTK_RG_IPV4MC_DONT_CARE_SRC;
			}
		}
		IGMP("set GIP IN-Mode & multicastFwdInfo->fwdPortMask=0x%X  mode=%d, return\n",multicastFwdInfo->fwdPortMask,multicastDataInfo->srcFilterMode);
		return FAIL;
	}
	else
	{
		IGMP("GIP had recorded");
		/*here to get multicast router port mask and forward port mask*/
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
		if ((rg_db.systemGlobal.initParam.igmpSnoopingEnable==1))
#else
		if ((rg_db.systemGlobal.initParam.igmpSnoopingEnable==1)||(multicastDataInfo->ipVersion==IP_VERSION6))
#endif
		{
			struct rtl_clientEntry * clientEntry=NULL;
			struct rtl_sourceEntry * sourceEntry=NULL;
			uint8 anyNormalEntry=0,anyPPPoE_lockHwEntry=0;
			IGMP("Decide GIP-SrcIP is Dc-Mo(path4-fwd)");

			anyNormalEntry=0;
			anyPPPoE_lockHwEntry=0;


//CONFIG_RG_RTL9607C_SERIES not support rg_db.systemGlobal.igmp_pppoe_passthrough_learning==2
#if defined(CONFIG_RG_RTL9600_SERIES)
			if(rg_db.systemGlobal.igmp_pppoe_passthrough_learning==2 && groupEntry->ipVersion==IP_VERSION4)
			{

				for (clientEntry=groupEntry->clientList; clientEntry!=NULL; clientEntry=clientEntry->next)
				{
					if(!clientEntry->pppoePassthroughEntry)
						anyNormalEntry=1;
					else
					{
						if(clientEntry->pppoeLockHw)
							anyPPPoE_lockHwEntry=1;
					}
				}
				IGMP("anyNormalEntry:%d anyPPPoE_lockHwEntry:%d",anyNormalEntry,anyPPPoE_lockHwEntry);

				if(anyNormalEntry)
				{
					//add reserved acl to trap pppoe entry
					for (clientEntry=groupEntry->clientList; clientEntry!=NULL; clientEntry=clientEntry->next)
					{
						if(clientEntry->pppoeReservedAclIdx == FAIL)
						{
							clientEntry->pppoeLockHw=0;
							_rtl_PPPoE_clientAddReservedACL(clientEntry,groupEntry->groupAddr[0]);
						}
					}
				}
				else /* all pppoe passthrough Entry (one or more pppoe Entry) ¡Aadd least one to hw */
				{
					if(anyPPPoE_lockHwEntry)
					{
						//check lockEnty and other to reserved
						for (clientEntry=groupEntry->clientList; clientEntry!=NULL; clientEntry=clientEntry->next)
						{
							if(clientEntry->pppoeLockHw==0)
							{
								_rtl_PPPoE_clientAddReservedACL(clientEntry,groupEntry->groupAddr[0]);
							}
						}

					}
					else
					{
						//get first one to lock and other to reserved
						int nonfirstClient=0;
						for (clientEntry=groupEntry->clientList; clientEntry!=NULL; clientEntry=clientEntry->next)
						{
							if(nonfirstClient)
							{
								clientEntry->pppoeLockHw=0;
								_rtl_PPPoE_clientAddReservedACL(clientEntry,groupEntry->groupAddr[0]);
							}
							else
							{
								//first client lock Hardware and
								nonfirstClient=1;
								clientEntry->pppoeLockHw=1;
							}

						}

					}

				}

			}
#endif

			for (clientEntry=groupEntry->clientList; clientEntry!=NULL; clientEntry=clientEntry->next)
			{
#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
				if((rg_db.systemGlobal.igmp_pppoe_passthrough_learning==0  && clientEntry->pppoePassthroughEntry)
					|| (rg_db.systemGlobal.igmp_pppoe_passthrough_learning==2 && clientEntry->pppoeReservedAclIdx!=FAIL)
					)
				{
					//ingore this pppoe passthrough client
					continue;
				}
#endif

				if (clientEntry->groupFilterTimer<=rtl_sysUpSeconds) /*include mode*/
				{
					sourceEntry = clientEntry->sourceList;
					while (sourceEntry)
					{
						if (sourceEntry->portTimer>rtl_sysUpSeconds)
						{
							multicastFwdInfo->fwdPortMask |= (1<<clientEntry->portNum);
							_setfwdMsktoWlan0(&multicastFwdInfo->wlan0DevMask,clientEntry->portNum,clientEntry->wlan_dev_idx);
							if(clientEntry->reportCtagif)
							{
								multicastFwdInfo->McFlowEgressCtagifMsk |= (1<<clientEntry->portNum);
								if( (0 <= clientEntry->portNum)  && (clientEntry->portNum < RTK_RG_PORT_MAX))
									multicastFwdInfo->McFlowVlanTag[clientEntry->portNum] = clientEntry->reportVlanId;
							}

							IGMP("add in-mo clt.port(%d)", clientEntry->portNum);
							break;
						}
						sourceEntry = sourceEntry->next;
					}
				}
				else /*exclude mode*/
				{
					DEBUG("add ex-mo clt.port(%d)", clientEntry->portNum);
					multicastFwdInfo->fwdPortMask |= (1<<clientEntry->portNum);
					if(clientEntry->reportCtagif)
					{
						multicastFwdInfo->McFlowEgressCtagifMsk |= (1<<clientEntry->portNum);
						if( (0 <= clientEntry->portNum)	&& (clientEntry->portNum < RTK_RG_PORT_MAX))
							multicastFwdInfo->McFlowVlanTag[clientEntry->portNum] = clientEntry->reportVlanId;
					}
					_setfwdMsktoWlan0(&multicastFwdInfo->wlan0DevMask,clientEntry->portNum,clientEntry->wlan_dev_idx);
				}

				//for dummy client we always send to port
				if(_check_isDummyClient(clientEntry))
				{
					multicastFwdInfo->fwdPortMask |= (1<<clientEntry->portNum);
					if(clientEntry->reportCtagif)
					{
						multicastFwdInfo->McFlowEgressCtagifMsk |= (1<<clientEntry->portNum);
						if( (0 <= clientEntry->portNum)  && (clientEntry->portNum < RTK_RG_PORT_MAX))
							multicastFwdInfo->McFlowVlanTag[clientEntry->portNum] = clientEntry->reportVlanId;
					}
					_setfwdMsktoWlan0(&multicastFwdInfo->wlan0DevMask,clientEntry->portNum,clientEntry->wlan_dev_idx);
				}
			}

			multicastFwdInfo->srcFilterMode = RTK_RG_IPV4MC_DONT_CARE_SRC;
		}
		else if (rg_db.systemGlobal.initParam.igmpSnoopingEnable==2)
		{
			uint32 *sourceAddr = multicastDataInfo->sourceIp;
			struct rtl_clientEntry * clientEntry=NULL;
			struct rtl_sourceEntry * sourceEntry=NULL;
			IGMP("Decide GIP-SrcIP is In-Mo(path3-fwd,grpTb-blk) / Ex-Mo(path3-fwd/blk,grpTb-fwd/blk)"); ////

			multicastFwdInfo->srcFilterMode = RTK_RG_IPV4MC_INCLUDE;
			for (clientEntry=groupEntry->clientList; clientEntry!=NULL; clientEntry=clientEntry->next)
			{
				sourceEntry=NULL;
				if (clientEntry->groupFilterTimer<=rtl_sysUpSeconds) /*include mode*/
				{
					if (groupEntry->ipVersion == IP_VERSION4) ////
						DEBUG("GIP.Clt(%pI4h) is <In-Mo>",(clientEntry->clientAddr));////
					else////
						DEBUG("GIP.Clt(%pI4h) is <In-Mo>",NIP6QUAD(clientEntry->clientAddr));////

					sourceEntry = rtl_searchSourceEntry(groupEntry->ipVersion, sourceAddr, clientEntry);
					if (sourceEntry!=NULL)
					{
						if (sourceEntry->portTimer>rtl_sysUpSeconds)
						{
							if (groupEntry->ipVersion == IP_VERSION4) ////
								DEBUG("GIP.SrcIP(%pI4h) must be forward",(sourceAddr));////
							else////
								DEBUG("GIP.SrcIP(" IP6H ") must be forward",NIP6QUAD(sourceAddr));////

							multicastFwdInfo->fwdPortMask |= (1<<clientEntry->portNum);
							if(clientEntry->reportCtagif)
							{
								multicastFwdInfo->McFlowEgressCtagifMsk |= (1<<clientEntry->portNum);
								if( (0 <= clientEntry->portNum)  && (clientEntry->portNum < RTK_RG_PORT_MAX))
									multicastFwdInfo->McFlowVlanTag[clientEntry->portNum] = clientEntry->reportVlanId;
							}
							_setfwdMsktoWlan0(&multicastFwdInfo->wlan0DevMask,clientEntry->portNum,clientEntry->wlan_dev_idx);
							continue;
						}
					}

				}
				else /*exclude mode*/
				{
					if (groupEntry->ipVersion == IP_VERSION4)
						DEBUG("GIP.Clt(%pI4h) is <Ex-Mo>",(clientEntry->clientAddr));
					else
						DEBUG("GIP.Clt(" IP6H ") is <Ex-Mo>",NIP6QUAD(clientEntry->clientAddr));
					multicastFwdInfo->srcFilterMode = RTK_RG_IPV4MC_EXCLUDE;

					sourceEntry = rtl_searchSourceEntry(groupEntry->ipVersion, sourceAddr, clientEntry);
					if (sourceEntry == NULL)
					{
						if (groupEntry->ipVersion == IP_VERSION4)
							DEBUG("GIP.SrcIP(%pI4h) must be forward",(sourceAddr));
						else////
							DEBUG("GIP.SrcIP(" IP6H ") must be forward",NIP6QUAD(sourceAddr));

						multicastFwdInfo->fwdPortMask |= (1<<clientEntry->portNum);
						if(clientEntry->reportCtagif)
						{
							multicastFwdInfo->McFlowEgressCtagifMsk |= (1<<clientEntry->portNum);
							if( (0 <= clientEntry->portNum)  && (clientEntry->portNum < RTK_RG_PORT_MAX))
								multicastFwdInfo->McFlowVlanTag[clientEntry->portNum] = clientEntry->reportVlanId;
						}
						_setfwdMsktoWlan0(&multicastFwdInfo->wlan0DevMask,clientEntry->portNum,clientEntry->wlan_dev_idx);

					}
					else
					{
						if (sourceEntry->portTimer>rtl_sysUpSeconds)
						{
							if (groupEntry->ipVersion == IP_VERSION4)
								DEBUG("GIP.SrcIP(%pI4h) must be forward",(sourceAddr));
							else
								DEBUG("GIP.SrcIP(" IP6H ") must be forward",NIP6QUAD(sourceAddr));

							multicastFwdInfo->fwdPortMask |= (1<<clientEntry->portNum);
							if(clientEntry->reportCtagif)
							{
								multicastFwdInfo->McFlowEgressCtagifMsk |= (1<<clientEntry->portNum);
								if( (0 <= clientEntry->portNum)  && (clientEntry->portNum < RTK_RG_PORT_MAX))
									multicastFwdInfo->McFlowVlanTag[clientEntry->portNum] = clientEntry->reportVlanId;
							}
							_setfwdMsktoWlan0(&multicastFwdInfo->wlan0DevMask,clientEntry->portNum,clientEntry->wlan_dev_idx);

						}
						else
						{
							//blkPortMask |= (1<<clientEntry->portNum);
						}

					}
					blkPortMask |= (1<<clientEntry->portNum);

#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
					if (clientEntry->portNum > RTK_RG_PORT_CPU)
#elif defined(CONFIG_RG_RTL9607C_SERIES)
					if (RTK_RG_EXT_PORT0<=clientEntry->portNum && clientEntry->portNum<=RTK_RG_MAC10_EXT_PORT5)
#else
#error
#endif
					{
						blkPortMask |= RTK_RG_ALL_MAC_MASTER_CPU_PORTMASK; //cpu port
					}

					//for dummy client we always send to port
					if(_check_isDummyClient(clientEntry))
					{
						multicastFwdInfo->fwdPortMask |= (1<<clientEntry->portNum);
						multicastFwdInfo->l2PortMask  |= (1<<clientEntry->portNum);
						if(clientEntry->reportCtagif)
						{
							multicastFwdInfo->McFlowEgressCtagifMsk |= (1<<clientEntry->portNum);
							if( (0 <= clientEntry->portNum)  && (clientEntry->portNum < RTK_RG_PORT_MAX))
								multicastFwdInfo->McFlowVlanTag[clientEntry->portNum] = clientEntry->reportVlanId;
						}
						_setfwdMsktoWlan0(&multicastFwdInfo->wlan0DevMask,clientEntry->portNum,clientEntry->wlan_dev_idx);
					}


				}


			} //for

			if (multicastFwdInfo->srcFilterMode == RTK_RG_IPV4MC_EXCLUDE) //blkPortMask!=0
			{

				DEBUG("GIP : Ex-Mo fwd src");
				multicastFwdInfo->l2PortMask = multicastFwdInfo->fwdPortMask;
				multicastFwdInfo->fwdPortMask = blkPortMask;

			}else{	//pure include mode
				DEBUG("GIP : In-Mo");
			}
		}

		#if defined (CONFIG_RG_HARDWARE_MULTICAST)
		if ((multicastFwdInfo->fwdPortMask & rtl_mCastModuleArray[moduleIndex].deviceInfo.swPortMask)!=0)
		{
			multicastFwdInfo->cpuFlag=TRUE;
		}
		#endif

		#ifdef CONFIG_RECORD_MCAST_FLOW
		if(rg_db.pktHdr->tagif&UDP_TAGIF)
			rtl_recordMcastFlow(moduleIndex,multicastDataInfo->ipVersion, multicastDataInfo->sourceIp, multicastDataInfo->groupAddr, multicastFwdInfo,rg_db.pktHdr->sport,rg_db.pktHdr->dport);
		#endif

		DEBUG("multicastFwdInfo->fwdPortMask =0x%X, multicastFwdInfo->l2PortMask =0x%X, multicastFwdInfo->wlan0DevMask=0x%X\n",multicastFwdInfo->fwdPortMask, multicastFwdInfo->l2PortMask, multicastFwdInfo->wlan0DevMask);

		return SUCCESS;
	} //if (group == NULL)

	return SUCCESS;


}



#if defined(__linux__) && defined(__KERNEL__)

void rtl_multicastSysTimerExpired(uint32 expireDada)
{
	system_expire_type = expireDada; //distinguid the timer is triggered by normal schedual(querierPresentInterval) or user force(/proc/rg/mcast_force_report_sec)
	rtl_maintainMulticastSnoopingTimerList((uint32)rtk_rg_getTimeSec());

	if (system_expire_type==SYS_EXPIRED_NORMAL)
		_rtk_rg_mod_timer(&igmpSysTimer, jiffies+rg_db.systemGlobal.igmp_sys_timer_sec*CONFIG_HZ);
}

void rtl_multicastSysTimerInit(void)
{
	rtl_startTime=0;
	rtl_sysUpSeconds=0;
	lastjiffies=jiffies;
	_rtk_rg_init_timer(&igmpSysTimer);
	igmpSysTimer.data=SYS_EXPIRED_NORMAL;
	igmpSysTimer.expires=jiffies+rg_db.systemGlobal.igmp_sys_timer_sec*CONFIG_HZ;
	igmpSysTimer.function=(void*)rtl_multicastSysTimerExpired;

}

static void rtl_multicastSysTimerDestroy(void)
{
	_rtk_rg_del_timer(&igmpSysTimer);
}

#endif

int32 rtl_getDeviceIgmpSnoopingModuleIndex(rtl_multicastDeviceInfo_t *devInfo,uint32 *moduleIndex)
{
	int i;
	*moduleIndex=0xFFFFFFFF;
	if(devInfo==NULL)
	{
		return FAIL;
	}

	for(i=0; i<MAX_MCAST_MODULE_NUM; i++)
	{
		if(rtl_mCastModuleArray[i].enableSnooping==TRUE)
		{
			if(strcmp(rtl_mCastModuleArray[i].deviceInfo.devName, devInfo->devName)==0)
			{
				*moduleIndex=i;
				return SUCCESS;
			}
		}
	}

	return FAIL;
}

int32 rtl865x_getDeviceIgmpSnoopingModuleIndex(rtl_multicastDeviceInfo_t *devInfo,uint32 *moduleIndex)
{
	return rtl_getDeviceIgmpSnoopingModuleIndex(devInfo,moduleIndex);
}

int32 rtl_setIgmpSnoopingModuleDevInfo(uint32 moduleIndex, rtl_multicastDeviceInfo_t *devInfo)
{
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAIL;
	}

	if(devInfo==NULL)
	{
		return FAIL;
	}

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAIL;
	}

	memcpy(&rtl_mCastModuleArray[moduleIndex].deviceInfo, devInfo, sizeof(rtl_multicastDeviceInfo_t));
	return SUCCESS;
}

int32 rtl_getIgmpSnoopingModuleDevInfo(uint32 moduleIndex, rtl_multicastDeviceInfo_t *devInfo)
{
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAIL;
	}

	if(devInfo==NULL)
	{
		return FAIL;
	}

	memset(devInfo,0,sizeof(rtl_multicastDeviceInfo_t));

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAIL;
	}

	memcpy(devInfo,&rtl_mCastModuleArray[moduleIndex].deviceInfo, sizeof(rtl_multicastDeviceInfo_t));
	return SUCCESS;
}

int32 rtl_setIgmpSnoopingModuleStaticRouterPortMask(uint32 moduleIndex,uint32 staticRouterPortMask)
{
	//DEBUG("got in rtl_setIgmpSnoopingModuleStaticRouterPortMask");
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAIL;
	}

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAIL;
	}

	rtl_mCastModuleArray[moduleIndex].staticRouterPortMask=staticRouterPortMask;

	return SUCCESS;
}

int32 rtl_getIgmpSnoopingModuleStaticRouterPortMask(uint32 moduleIndex,uint32 *staticRouterPortMask)
{
	//DEBUG("got in rtl_getIgmpSnoopingModuleStaticRouterPortMask");
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAIL;
	}

	if(staticRouterPortMask==NULL)
	{
		return FAIL;
	}

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAIL;
	}

	*staticRouterPortMask=rtl_mCastModuleArray[moduleIndex].staticRouterPortMask;

	return SUCCESS;
}


int32 rtl_setIgmpSnoopingModuleUnknownMCastFloodMap(uint32 moduleIndex,uint32 unknownMCastFloodMap)
{
	//DEBUG("got in rtl_setIgmpSnoopingModuleUnknownMCastFloodMap");
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAIL;
	}

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAIL;
	}

	rtl_mCastModuleArray[moduleIndex].unknownMCastFloodMap=unknownMCastFloodMap;

	return SUCCESS;
}

int32 rtl_getIgmpSnoopingModuleUnknownMCastFloodMap(uint32 moduleIndex,uint32 *unknownMCastFloodMap)
{
	//DEBUG("got in rtl_getIgmpSnoopingModuleUnknownMCastFloodMap");
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAIL;
	}

	if(unknownMCastFloodMap==NULL)
	{
		return FAIL;
	}

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAIL;
	}

	*unknownMCastFloodMap=rtl_mCastModuleArray[moduleIndex].unknownMCastFloodMap;

	return SUCCESS;
}

#ifdef CONFIG_PROC_FS
int igmp_show(struct seq_file *s, void *v)
{
	int32 moduleIndex;
	int32 hashIndex,groupCnt,clientCnt;
	struct rtl_groupEntry *groupEntryPtr;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_sourceEntry *sourceEntryPtr;
	#ifdef CONFIG_RECORD_MCAST_FLOW
	int32 flowCnt;
	struct rtl_mcastFlowEntry *mcastFlowEntry=NULL;
	#endif
	int len=0;


	//PROC_PRINTF( "IGMP Snopping: %s\n",rg_db.systemGlobal.initParam.igmpSnoopingEnable?"on":"off");
	switch (rg_db.systemGlobal.initParam.igmpSnoopingEnable) {
	case 0: PROC_PRINTF("IGMP Snopping: Off\n");			break;
	case 1:	PROC_PRINTF("IGMP Snopping: Don`t Care Source\n");	break;
	case 2: PROC_PRINTF("IGMP Snopping: Care Source\n");		break;
	default:PROC_PRINTF("IGMP Snopping: Undefined Defined\n");	break;
	}

	/*PROC_PRINTF( "Multicast Protocol: %s\n",rg_db.systemGlobal.multicastProtocol==RG_MC_BOTH_IGMP_MLD?"IGMP and MLD":
		rg_db.systemGlobal.multicastProtocol==RG_MC_IGMP_ONLY?"IGMP Only":"MLD Only");*/
	switch (rg_db.systemGlobal.multicastProtocol) {
	case RG_MC_BOTH_IGMP_MLD:PROC_PRINTF("Multicast Protocol: (0)IGMP/MLD BOTH\n");	break;
	case RG_MC_IGMP_ONLY	:PROC_PRINTF("Multicast Protocol: (1)IGMP ONLY\n");	break;
	case RG_MC_MLD_ONLY	:PROC_PRINTF("Multicast Protocol: (2)MLD ONLY\n");	break;
	default			:PROC_PRINTF("Multicast Protocol: Undefined Defined\n");	break;
	}

	PROC_PRINTF( "fastLeave: %s\n",rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].enableFastLeave?"on":"off");
	PROC_PRINTF( "groupMemberAgingTime: %d\n",rtl_mCastTimerParas.groupMemberAgingTime);
	PROC_PRINTF( "lastMemberAgingTime: %d\n",rtl_mCastTimerParas.lastMemberAgingTime);
	PROC_PRINTF( "querierPresentInterval: %d\n",rtl_mCastTimerParas.querierPresentInterval);
	//PROC_PRINTF( "dvmrpRouterAgingTime: %d\n",rtl_mCastTimerParas.dvmrpRouterAgingTime);
	//PROC_PRINTF( "mospfRouterAgingTime: %d\n",rtl_mCastTimerParas.mospfRouterAgingTime);
	//PROC_PRINTF( "pimRouterAgingTime: %d\n",rtl_mCastTimerParas.pimRouterAgingTime);
	PROC_PRINTF( "forceReportResponseTime: %d\n",rg_db.systemGlobal.forceReportResponseTime);

	for (moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM ;moduleIndex++)
	{
		if (rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{
			PROC_PRINTF( "-------------------------------------------------------------------------\n");
			//PROC_PRINTF( "module index:%d, ",moduleIndex);
			#ifdef CONFIG_RG_HARDWARE_MULTICAST
			PROC_PRINTF( "device:%s, portMask:0x%x\n\n",rtl_mCastModuleArray[moduleIndex].deviceInfo.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.portMask);
			#else
			PROC_PRINTF( "\n\n");
			#endif
			if (rg_db.systemGlobal.multicastProtocol!=RG_MC_MLD_ONLY)
			{
				PROC_PRINTF("igmp list: V1=%d, V2=%d, V3=%d\n",igmpSnoopingCounterVer_1,igmpSnoopingCounterVer_2,igmpSnoopingCounterVer_3); ////**
				groupCnt=0;
				for (hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
				{
					groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];
					while (groupEntryPtr!=NULL)
					{
						groupCnt++;
						PROC_PRINTF( "    [%d] Group address:%d.%d.%d.%d vlanID[%d]\n",groupCnt,//hashIndex
						groupEntryPtr->groupAddr[0]>>24, (groupEntryPtr->groupAddr[0]&0x00ff0000)>>16,
						(groupEntryPtr->groupAddr[0]&0x0000ff00)>>8, (groupEntryPtr->groupAddr[0]&0xff),groupEntryPtr->vlanId);

						clientEntry=groupEntryPtr->clientList;

						clientCnt=0;
						while (clientEntry!=NULL)
						{

							clientCnt++;

#ifdef CONFIG_MASTER_WLAN0_ENABLE
#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
							if (clientEntry->portNum > RTK_RG_PORT_CPU)
#elif defined(CONFIG_RG_RTL9607C_SERIES)
							if (RTK_RG_EXT_PORT0<=clientEntry->portNum && clientEntry->portNum<=RTK_RG_MAC10_EXT_PORT5)
#else
#error
#endif
							{
								PROC_PRINTF( "        <%d>%pI4h\\port %d(mbssid %d)\\IGMPv%d\\",clientCnt, (clientEntry->clientAddr),clientEntry->portNum, clientEntry->wlan_dev_idx, (clientEntry->igmpVersion-IGMP_V1 + 1));
							}
							else
#endif
							{
								PROC_PRINTF( "        <%d>%pI4h\\port %d\\IGMPv%d\\",clientCnt, (clientEntry->clientAddr),clientEntry->portNum, (clientEntry->igmpVersion-IGMP_V1 + 1));
							}

							PROC_PRINTF( "%s",(clientEntry->groupFilterTimer>rtl_sysUpSeconds)?"EXCLUDE":"INCLUDE");
							if(clientEntry->groupFilterTimer>rtl_sysUpSeconds)
							{
								PROC_PRINTF( ":%ds",clientEntry->groupFilterTimer-rtl_sysUpSeconds);
							}
							else
							{
								PROC_PRINTF( ":0s");
							}
							PROC_PRINTF( ":%s ",clientEntry->pppoePassthroughEntry?"PPPoE Passthrough Entry":"");
							if(clientEntry->pppoePassthroughEntry)
								PROC_PRINTF( "pppoeReservedAclIdx:%d ",clientEntry->pppoeReservedAclIdx);
							if(clientEntry->reportCtagif)
								PROC_PRINTF( "CTag:%d",clientEntry->reportVlanId);
							else
								PROC_PRINTF( "unTag");

							sourceEntryPtr=clientEntry->sourceList;
							if(sourceEntryPtr!=NULL)
							{
								PROC_PRINTF( "\\source list:");
							}

							while(sourceEntryPtr!=NULL)
							{
								PROC_PRINTF( "%d.%d.%d.%d:",
										sourceEntryPtr->sourceAddr[0]>>24, (sourceEntryPtr->sourceAddr[0]&0x00ff0000)>>16,
										(sourceEntryPtr->sourceAddr[0]&0x0000ff00)>>8, (sourceEntryPtr->sourceAddr[0]&0xff));

								if(sourceEntryPtr->portTimer>rtl_sysUpSeconds)
								{
									PROC_PRINTF( "%ds",sourceEntryPtr->portTimer-rtl_sysUpSeconds);
								}
								else
								{
									PROC_PRINTF( "0s");
								}

								if(sourceEntryPtr->next!=NULL)
								{
									PROC_PRINTF( ", ");
								}

								sourceEntryPtr=sourceEntryPtr->next;
							}


							PROC_PRINTF( "\n");
							clientEntry = clientEntry->next;
						}

						PROC_PRINTF( "\n");
						groupEntryPtr=groupEntryPtr->next;
					}

				}
				if(groupCnt==0)
				{
					PROC_PRINTF("\tnone\n");
				}
			}


			if(rg_db.systemGlobal.multicastProtocol!=RG_MC_IGMP_ONLY)
			{
				PROC_PRINTF( "\n\n");
				PROC_PRINTF( "mld list: V1=%d, V2=%d\n",MLDCounterVer_1,MLDCounterVer_2);
				groupCnt=0;
				for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
			     	{
					groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[hashIndex];
					while(groupEntryPtr!=NULL)
					{
						groupCnt++;
						PROC_PRINTF("     [%d] Group address:%pI6 vlanID[%d] \n",groupCnt,&groupEntryPtr->groupAddr[0],groupEntryPtr->vlanId);

						clientEntry=groupEntryPtr->clientList;

						clientCnt=0;
						while (clientEntry!=NULL)
						{

							clientCnt++;
							PROC_PRINTF( "        <%d>%pI6\\port %d\\MLDv%d\\",clientCnt,&clientEntry->clientAddr[0],clientEntry->portNum, (clientEntry->igmpVersion-MLD_V1+1));

							PROC_PRINTF( "%s",(clientEntry->groupFilterTimer>rtl_sysUpSeconds)?"EXCLUDE":"INCLUDE");
							if(clientEntry->groupFilterTimer>rtl_sysUpSeconds)
							{
								PROC_PRINTF( ":%ds",clientEntry->groupFilterTimer-rtl_sysUpSeconds);
							}
							else
							{
								PROC_PRINTF( ":0s");
							}
							PROC_PRINTF( ":%s ",clientEntry->pppoePassthroughEntry?"PPPoE Passthrough Entry":"");
							if(clientEntry->pppoePassthroughEntry)
								PROC_PRINTF( "pppoeReservedAclIdx:%d ",clientEntry->pppoeReservedAclIdx);
							if(clientEntry->reportCtagif)
								PROC_PRINTF( "CTag:%d",clientEntry->reportVlanId);
							else
								PROC_PRINTF( "unTag");

							sourceEntryPtr=clientEntry->sourceList;
							if(sourceEntryPtr!=NULL)
							{
								PROC_PRINTF( "\\source list:");
							}

							while(sourceEntryPtr!=NULL)
							{
								PROC_PRINTF( "%pI6:",&sourceEntryPtr->sourceAddr[0]);

								if(sourceEntryPtr->portTimer>rtl_sysUpSeconds)
								{
									PROC_PRINTF( "%ds",sourceEntryPtr->portTimer-rtl_sysUpSeconds);
								}
								else
								{
									PROC_PRINTF( "0s");
								}

								if(sourceEntryPtr->next!=NULL)
								{
									PROC_PRINTF( ", ");
								}

								sourceEntryPtr=sourceEntryPtr->next;
							}

							PROC_PRINTF( "\n");
							clientEntry = clientEntry->next;
						}

						PROC_PRINTF( "\n");
						groupEntryPtr=groupEntryPtr->next;
					}

			       }
				if(groupCnt==0)
				{
					PROC_PRINTF("\tnone\n");
				}
			}

#ifdef CONFIG_RECORD_MCAST_FLOW
			PROC_PRINTF("ipv4 flow list:\n");
			flowCnt=1;
			for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
		    {

				/*to dump multicast flow information*/
		     		mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[hashIndex];

				while(mcastFlowEntry!=NULL)
				{
					if(mcastFlowEntry->ipVersion==IP_VERSION4)
					{
						PROC_PRINTF( "    [%d] %d.%d.%d.%d:%d-->",flowCnt,
						mcastFlowEntry->serverAddr[0]>>24, (mcastFlowEntry->serverAddr[0]&0x00ff0000)>>16,
						(mcastFlowEntry->serverAddr[0]&0x0000ff00)>>8, (mcastFlowEntry->serverAddr[0]&0xff),mcastFlowEntry->sport);

						PROC_PRINTF( "%d.%d.%d.%d:%d-->",
						mcastFlowEntry->groupAddr[0]>>24, (mcastFlowEntry->groupAddr[0]&0x00ff0000)>>16,
						(mcastFlowEntry->groupAddr[0]&0x0000ff00)>>8, (mcastFlowEntry->groupAddr[0]&0xff),mcastFlowEntry->dport);
						PROC_PRINTF( "port mask:0x%x  \n",mcastFlowEntry->multicastFwdInfo.srcFilterMode==RTK_RG_IPV4MC_EXCLUDE?mcastFlowEntry->multicastFwdInfo.l2PortMask:mcastFlowEntry->multicastFwdInfo.fwdPortMask);
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
						if(mcastFlowEntry->flowidx!=-1)
						{
							PROC_PRINTF("		 mcflowIdxtbl[%d] path3Idx=%d path4Idx=%d path6Idx=%d lutIdx=%d \n",
								mcastFlowEntry->flowidx,rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].path3Idx,rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].path4Idx,rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].path6Idx,rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].lutIdx);
							PROC_PRINTF("		  McAddr= %d.%d.%d.%d\n",(rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].multicastAddress[0]>>24)&0xff,(rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].multicastAddress[0]>>16)&0xff,
								(rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].multicastAddress[0]>>8)&0xff,(rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].multicastAddress[0])&0xff);
#if defined(CONFIG_RG_G3_SERIES)
							PROC_PRINTF("         pMcEngineInfo=%p ",rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].pMcEngineInfo);
							if(rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].pMcEngineInfo)
								PROC_PRINTF(" l2mcgid=%d l3mcgid=%d\n",rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].pMcEngineInfo->l2mcgid,rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].pMcEngineInfo->l3mcgid);
#endif
						}
#endif


					}

					flowCnt++;
					mcastFlowEntry=mcastFlowEntry->next;
				}

			}
			PROC_PRINTF( "ipv6 flow list:\n");
			flowCnt=1;
			for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
		     {

				/*to dump multicast flow information*/
		     		mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[hashIndex];

				while(mcastFlowEntry!=NULL)
				{
					if(mcastFlowEntry->ipVersion==IP_VERSION6)
					{
						PROC_PRINTF( "    [%d] %pI6:(%d)-->",flowCnt,&mcastFlowEntry->serverAddr[0],mcastFlowEntry->sport);
						PROC_PRINTF( "%pI6:(%d)",&mcastFlowEntry->groupAddr[0],mcastFlowEntry->dport);

						PROC_PRINTF( "port mask:0x%x \n",mcastFlowEntry->multicastFwdInfo.fwdPortMask);
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
						if(mcastFlowEntry->flowidx!=-1)
						{
							PROC_PRINTF("        mcflowIdxtbl[%d] path3Idx=%d path4Idx=%d path6Idx=%d lutIdx=%d \n",
								mcastFlowEntry->flowidx,rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].path3Idx,rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].path4Idx,rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].path6Idx,rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].lutIdx);
							if(rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].isIpv6)
								PROC_PRINTF("        McAddr= %x:%x:%x:%x\n",rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].multicastAddress[0],rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].multicastAddress[1],
								rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].multicastAddress[2],rg_db.mcflowIdxtbl[mcastFlowEntry->flowidx].multicastAddress[3]);
						}
#endif

					}
					flowCnt++;
					mcastFlowEntry=mcastFlowEntry->next;
				}

			}
#endif
		}
	}

	PROC_PRINTF( "------------------------------------------------------------------\n");
#if defined (CONFIG_RG_HARDWARE_MULTICAST)
	dump_hwMappingEntry();
#endif

	return len;
}

int igmp_groupList_memDump(struct seq_file *s, void *v)
{
	struct rtl_groupEntry *gp = NULL;
	int i;
	int len=0;

	rg_lock(&rg_kernel.igmpsnoopingLock);
	PROC_PRINTF("rtl_totalMaxGroupCnt=%d \n",rtl_totalMaxGroupCnt);
	if (rtl_groupMemory!=NULL)
	{
		if (rtl_groupEntryPool!=NULL)
			PROC_PRINTF("current rtl_groupEntryPool point to %p \n",rtl_groupEntryPool);

		gp = (struct rtl_groupEntry *)rtl_groupMemory;

		for (i=0;i<rtl_totalMaxGroupCnt;i++){
			PROC_PRINTF("rtl_groupMemory[%d]=%p (gip=0x%x, vlanId=%d) ", i, &gp[i],gp[i].groupAddr[0], gp[i].vlanId);
			if(gp[i].previous!=NULL)
				PROC_PRINTF("prev=%p ",gp[i].previous);
			else
				PROC_PRINTF("prev=NULL ");

			if(gp[i].next!=NULL)
				PROC_PRINTF("next=%p \n",gp[i].next);
			else
				PROC_PRINTF("next=NULL \n");
		}
	}
	rg_unlock(&rg_kernel.igmpsnoopingLock);
	return len;
}
#endif

void rtk_rg_igmpLinkStatusChange(uint32 moduleIndex, uint32 linkPortMask)
{
	int32 hashIndex;
	int32 clearFlag=FALSE;
	struct rtl_groupEntry *groupEntryPtr;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* nextClientEntry=NULL;
	#ifdef CONFIG_RECORD_MCAST_FLOW
	//struct rtl_mcastFlowEntry *mcastFlowEntry, *nextMcastFlowEntry;
	#endif

	DEBUG("In rtl865x_igmpLinkStatusChangeCallback");


	if (moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return ;
	}

#if 0
	#ifdef CONFIG_RECORD_MCAST_FLOW
	for (hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
     	{
     		mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[hashIndex];

		while(mcastFlowEntry!=NULL)
		{
			nextMcastFlowEntry=mcastFlowEntry->next;

			/*clear multicast forward flow cache*/
			 rtl_deleteMcastFlowEntry( mcastFlowEntry, rtl_mCastModuleArray[moduleIndex].flowHashTable);

			mcastFlowEntry=nextMcastFlowEntry;
		}
	}
	#endif
#endif

	if (rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
	{
		for (hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
	    {
			groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];
			while (groupEntryPtr!=NULL)
			{
				clientEntry=groupEntryPtr->clientList;
				while (clientEntry!=NULL)
				{
					/*save next client entry first*/
					nextClientEntry=clientEntry->next;
					if( ((1<<clientEntry->portNum)<RTK_RG_MAC_PORT_CPU) && (((1<<clientEntry->portNum) & linkPortMask)==0) && (!_check_isDummyClient(clientEntry)))
					{
						rtl_deleteClientEntry(groupEntryPtr,clientEntry);
						clearFlag=TRUE;
					}

					clientEntry = nextClientEntry;
				}

#ifdef CONFIG_RECORD_MCAST_FLOW
				if (clearFlag==TRUE)
					rtl_rg_mcDataReFlushAndAdd(groupEntryPtr->groupAddr);
#endif
				#if defined (CONFIG_RG_HARDWARE_MULTICAST)
				if (clearFlag==TRUE)
				{
					strcpy(linkEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
					linkEventContext.moduleIndex=moduleIndex;

					linkEventContext.groupAddr[0]=groupEntryPtr->groupAddr[0];
					linkEventContext.groupAddr[1]=groupEntryPtr->groupAddr[1];
					linkEventContext.groupAddr[2]=groupEntryPtr->groupAddr[2];
					linkEventContext.groupAddr[3]=groupEntryPtr->groupAddr[3];

					linkEventContext.sourceAddr[0]=0;
					linkEventContext.sourceAddr[1]=0;
					linkEventContext.sourceAddr[2]=0;
					linkEventContext.sourceAddr[3]=0;

					rtl_handle_igmpgroup_change(&linkEventContext);
				}
				#endif
				groupEntryPtr=groupEntryPtr->next;
				clearFlag=FALSE;
			}

	    }


	}

	return ;
}


int32 rtl_getGroupInfo(uint32 groupAddr, struct rtl_groupInfo * groupInfo)
{
	int32 moduleIndex;
	int32 hashIndex;
	struct rtl_groupEntry *groupEntryPtr;
	//DEBUG("got in rtl_getGroupInfo");

	if(groupInfo==NULL)
	{
		return FAIL;
	}

	memset(groupInfo, 0 , sizeof(struct rtl_groupInfo));

	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM ;moduleIndex++)
	{
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{
			hashIndex=rtl_hashMask&groupAddr;
			groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];

			while(groupEntryPtr!=NULL)
			{
				if(groupEntryPtr->groupAddr[0]==groupAddr)
				{
					groupInfo->ownerMask |= (1<<moduleIndex);
					break;
				}
				groupEntryPtr=groupEntryPtr->next;
			}

		}
	}

	return SUCCESS;
}



#if defined (CONFIG_RG_HARDWARE_MULTICAST)

static int _rtl_handle_igmpgroup_addhw(rtl_multicast_index_mapping_t* mapping_entry)
{
	rtk_rg_multicastFlow_t mcFlow;

	memset(&mcFlow,0,sizeof(rtk_rg_multicastFlow_t));

	mcFlow.port_mask.portmask = mapping_entry->fwdmembr;
	mcFlow.srcFilterMode = mapping_entry->srcFilterMode; ////

	if (mapping_entry->ipVersion == IP_VERSION4)
	{
		IGMP("Add IP4 lut info");
		mcFlow.isIPv6 = 0;
		mcFlow.multicast_ipv4_addr	= mapping_entry->groupIP[0];
		mcFlow.routingMode = mapping_entry->routingMode;

		if (RTK_RG_IPV4MC_DONT_CARE_SRC!=mapping_entry->srcFilterMode)
		{
			mcFlow.includeOrExcludeIp = mapping_entry->sourceIP[0];
			IGMP( "%pI4h", ((&mcFlow.includeOrExcludeIp)));
			//mcFlow.routingMode = RTK_RG_IPV4MC_DIS_ROUTING;
		}
	}
	else
	{
		IGMP("Add IP6 lut info");
		mcFlow.isIPv6 = 1;
		mcFlow.multicast_ipv6_addr[0]	= mapping_entry->groupIP[0];
		mcFlow.multicast_ipv6_addr[1]	= mapping_entry->groupIP[1];
		mcFlow.multicast_ipv6_addr[2]	= mapping_entry->groupIP[2];
		mcFlow.multicast_ipv6_addr[3]	= mapping_entry->groupIP[3];
	}
	mcFlow.port_mask.portmask	= mapping_entry->fwdmembr;


	//rev1668 already skip packets which from CPU port into igmpSnooping module.
	//skip joining CPU Port when IGMP Proxy send from CPU.
	//if(rg_db.pktHdr->ingressPort==RTK_RG_PORT_CPU)
	//	return FAIL;
	if(rg_db.systemGlobal.initParam.ivlMulticastSupport)
	{

		if(rg_db.vlan[mapping_entry->vlanId].fidMode==VLAN_FID_IVL)
		{
			mcFlow.isIVL=1;
			mcFlow.vlanID=mapping_entry->vlanId;
		}else{
			mcFlow.vlanID=rg_db.vlan[mapping_entry->vlanId].fid;
		}
	}
	if((pf.rtk_rg_multicastFlow_add)(&mcFlow, &mapping_entry->hwflow_index)!=RT_ERR_RG_OK){
		return FAIL;
	}else{
		IGMP("add multicast flow at L2[%d]",mapping_entry->hwflow_index);
		return SUCCESS;
	}
}


static int _rtl_handle_igmpgroup_delhw(rtl_multicast_index_mapping_t* mapping_entry)
{
	int32 ret;
	IGMP("#####delete multicast flow at L2[%d]######",mapping_entry->hwflow_index);
	ret = (pf.rtk_rg_multicastFlow_del)(mapping_entry->hwflow_index);
	_rtk_hwMappingEntry_free(mapping_entry);
	if(ret != SUCCESS)
		IGMP("DEl Entry FAILED Error code=%x",ret);
	return ret;
}

#ifdef CONFIG_RG_SIMPLE_IGMP_v3
/*cxy 2014-8-19:for IGMPv3 hw can't support include or exclude case, so we want to get the fwd mask of group that
			all situations can forward mc pkts.
			Firstly we want to get all specified src fwdmask of this group and for include we use OR op and
			for exclude we use AND op.
			But we can't specify src ip for group fwdmask lookup. Because it is complicated to find all different src ip
			under group entry.
				    I(g,<s1,s3>)   ____
			client1-------------|      |
				     I(g,<s1,s2>) |      |-------server1,server2            g:group,   s1,s2,s3:source
			client2-------------|____|

			so hw forward to that port if the port client is exclude mode or include mode with src.
*/
int rtl_handle_igmpgroup_change_v3(rtl_multicastEventContext_t* param)
{
	rtl_multicast_index_mapping_t* mapping_entry_rm=NULL;
	IGMP("IGMPv3 Group change.\n");

	if (strlen(param->devName)==0)
	{
    		DEBUG("IGMPv3 devname==NULL\n");
		return FAIL;
	}


	if (strcmp(param->devName, RG_IGMP_SNOOPING_MODULE_NAME)==0)
	{
		struct rtl_multicastDeviceInfo_s igmp_snooping_module;
		rtl_multicast_index_mapping_t* mapping_entry_temp;
		rtl_multicast_index_mapping_t* mapping_entry=NULL;
		int retVal;
		struct rtl_groupEntry* groupEntry;
		unsigned int fwdPortMask=0;
		struct	rtl_clientEntry *clientEntry;

		groupEntry=rtl_searchGroupEntry(param->moduleIndex, param->ipVersion, param->groupAddr, param->vlanId);
		if (groupEntry==NULL)
		{
			fwdPortMask = rtl_mCastModuleArray[param->moduleIndex].unknownMCastFloodMap;
		}
		else
		{
			clientEntry = groupEntry->clientList;
			while (clientEntry != NULL)
			{
				/* //:
				 * G.C.In.SrcList is empty  => do not add, means leave
				 * G.C.In.SrcList not empty => do not care src, add client spa
				 * G.C.Ex.SrcList is empty  => like v2 join,    add client spa
				 * G.C.Ex.Srclist not emtpy => do not care src, add client spa
				 */
				if (!((clientEntry->groupFilterTimer <= rtl_sysUpSeconds) &&
					(clientEntry->sourceList == NULL)) )
				{
					fwdPortMask|= (1<<clientEntry->portNum);
				}
				clientEntry=clientEntry->next;
			}
		}

		if (param->groupAddr[0] == 0xEFFFFFFA) {
			fwdPortMask |= 0x40;
		}

		//printk("in <%s>,add igmp hw mc --grp:%x fwd:%x\n",__func__,
			//param->groupAddr[0],fwdPortMask);

		retVal = rtl_getIgmpSnoopingModuleDevInfo(param->moduleIndex,&igmp_snooping_module);
		if(retVal!=SUCCESS)
		{
			DEBUG("FAIL: igmpv3 rtl_getIgmpSnoopingModuleDevInfo=%d",retVal);
			return FAIL;
		}

		list_for_each_entry_safe(mapping_entry_temp,mapping_entry_rm,&rtl_multicast_index_mapping_listhead,entry)
		{
			if(mapping_entry_temp->groupIP[0] == param->groupAddr[0]
			&& mapping_entry_temp->groupIP[1] == param->groupAddr[1]
			&& mapping_entry_temp->groupIP[2] == param->groupAddr[2]
			&& mapping_entry_temp->groupIP[3] == param->groupAddr[3]
			&& mapping_entry_temp->ipVersion == param->ipVersion)
			{
				if(rg_db.vlan[mapping_entry_temp->vlanId].fidMode!=VLAN_FID_SVL
					&& mapping_entry_temp->vlanId!=param->vlanId)
					continue;

				/* (rg_db.systemGlobal.initParam.ivlMulticastSupport) //force path2
				{
					if  (mapping_entry_temp->vlanId!=param->vlanId)
						goto NEXT;
					return groupPtr;
				}*/
				mapping_entry = mapping_entry_temp;
			}
		}

		if (!mapping_entry)
		{
			/* create sw_entry */
			mapping_entry = _rtk_hwMappingEntry_malloc();


			if (!mapping_entry)
			{
				DEBUG("IGMPv3 alloc fail!");
				return FAIL;
			}

			mapping_entry->groupIP[0] 	= param->groupAddr[0];
			mapping_entry->groupIP[1] 	= param->groupAddr[1];
			mapping_entry->groupIP[2] 	= param->groupAddr[2];
			mapping_entry->groupIP[3] 	= param->groupAddr[3];
			mapping_entry->ipVersion		= param->ipVersion;
			mapping_entry->vlanId		= param->vlanId;
			mapping_entry->fwdmembr	= fwdPortMask;

			/* add it into hw */
			if(mapping_entry->fwdmembr!=0) //have member port in this entry
			{
				int r=_rtl_handle_igmpgroup_addhw(mapping_entry);
				if (param->ipVersion==IP_VERSION4)
				{
					DEBUG("IGMPv3 SNOOPING ADD:IPv4(GIP:%d.%d.%d.%d) pMask:0x%X LUT_IDX=%d\n"
						,NIPQUAD(param->groupAddr[0]),fwdPortMask,mapping_entry->hwflow_index);
				}
				else
				{

					DEBUG("IGMPv3 SNOOPING CHANGE:IPv6(GIP:%X:%X:%X:%X %X:%X:%X:%X %X:%X:%X:%X %X:%X:%X:%X) pMask:0x%X LUT_IDX=%d\n"
						,NIPQUAD(param->groupAddr[0]),NIPQUAD(param->groupAddr[1]),NIPQUAD(param->groupAddr[2]),NIPQUAD(param->groupAddr[3]),fwdPortMask
						,mapping_entry->hwflow_index);
				}
				if (r!=SUCCESS)
				{
					DEBUG("IGMPv3 SNOOPING ADD:FAIL\n");
					return FAIL;
				}
			}


		}
		else
		{
			if(param->ipVersion==IP_VERSION4)
			{
				DEBUG("IGMPv3 SNOOPING CHANGE:IPv4(GIP:%d.%d.%d.%d) pMask:0x%X LUT_IDX=%d\n"
					,NIPQUAD(param->groupAddr[0]),fwdPortMask,mapping_entry->hwflow_index);
			}
			else
			{

				DEBUG("IGMPv3 SNOOPING CHANGE:IPv6(GIP:%X:%X:%X:%X %X:%X:%X:%X %X:%X:%X:%X %X:%X:%X:%X) pMask:0x%X LUT_IDX=%d\n"
					,NIPQUAD(param->groupAddr[0]),NIPQUAD(param->groupAddr[1]),NIPQUAD(param->groupAddr[2]),NIPQUAD(param->groupAddr[3]),fwdPortMask
					,mapping_entry->hwflow_index);
			}

			/* modify member port */
			mapping_entry->fwdmembr	= fwdPortMask;

			/* add it into hw */
			if(mapping_entry->fwdmembr!=0) //have member port in this entry
			{
				if(_rtl_handle_igmpgroup_addhw(mapping_entry)!=SUCCESS)
				{
					DEBUG("IGMPv3 SNOOPING ADD:FAIL\n");
					return FAIL;
				}
			}
			else
			{
				/* no member ports, clean it in hw */
				_rtl_handle_igmpgroup_delhw(mapping_entry);
			}
		}

		DEBUG("(%s %d) SUCCESS!! ",__func__,__LINE__);
		return SUCCESS;
	}
	return FAIL;
}
#endif


//if  sourceAddr=NULL return first entry ,if sourceAddr!=NULL compare this parameter
rtl_multicast_index_mapping_t * rtl_findMappingEntry(uint32 ipVersion,uint32 *multicastAddr,uint32 *sourceAddr, uint16 vlanId)
{
	rtl_multicast_index_mapping_t* mapping_entry_temp=NULL;
	rtl_multicast_index_mapping_t* mapping_entry_rm=NULL;
	if(multicastAddr ==NULL) return NULL;

	list_for_each_entry_safe(mapping_entry_temp,mapping_entry_rm,&rtl_multicast_index_mapping_listhead,entry)
	{

		if (mapping_entry_temp->ipVersion == ipVersion &&
			mapping_entry_temp->groupIP[0]== multicastAddr[0] &&
			mapping_entry_temp->groupIP[1]== multicastAddr[1] &&
			mapping_entry_temp->groupIP[2]== multicastAddr[2] &&
			mapping_entry_temp->groupIP[3]== multicastAddr[3])
		{

			//Hit
			if (rg_db.systemGlobal.initParam.ivlMulticastSupport) // if IVL compare vid
			{
				if (rg_db.vlan[mapping_entry_temp->vlanId].fidMode==VLAN_FID_IVL&& mapping_entry_temp->vlanId==vlanId)
				{
					if(sourceAddr==NULL)
						return mapping_entry_temp;
					if( mapping_entry_temp->sourceIP[0] == sourceAddr[0] &&
						mapping_entry_temp->sourceIP[1] == sourceAddr[1] &&
						mapping_entry_temp->sourceIP[2] == sourceAddr[2] &&
						mapping_entry_temp->sourceIP[3] == sourceAddr[3] )
					{
						return mapping_entry_temp;
					}
				}
			}
			else
			{
				if(sourceAddr==NULL)
					return mapping_entry_temp;
				if( mapping_entry_temp->sourceIP[0] == sourceAddr[0] &&
					mapping_entry_temp->sourceIP[1] == sourceAddr[1] &&
					mapping_entry_temp->sourceIP[2] == sourceAddr[2] &&
					mapping_entry_temp->sourceIP[3] == sourceAddr[3] )
				{
					return mapping_entry_temp;
				}
			}
		}
	}

	//not find
	return NULL;

}

void dump_hwMappingEntry(void)
{
	rtl_multicast_index_mapping_t* mapping_entry_temp=NULL;
	rtl_multicast_index_mapping_t* mapping_entry_rm=NULL;
	struct seq_file *s=NULL;

	PROC_PRINTF("===Start to Dump igmp mapping-hw-list====\n");
	list_for_each_entry_safe(mapping_entry_temp,mapping_entry_rm,&rtl_multicast_index_mapping_listhead,entry)
	{

		if (mapping_entry_temp!=NULL) {
			if (mapping_entry_temp->ipVersion==IP_VERSION4)
			{
				PROC_PRINTF("IGMP HW Entry: GIP4:%d.%d.%d.%d SIP4:%d.%d.%d.%d l2Mask:0x%X fwdMask:0x%X LUT_IDX=%d\n"
				,NIPQUAD(mapping_entry_temp->groupIP), NIPQUAD(mapping_entry_temp->sourceIP),
				mapping_entry_temp->l2membr, mapping_entry_temp->fwdmembr, mapping_entry_temp->hwflow_index);
			}
			else
			{
				PROC_PRINTF("IGMP HW Entry:GIP6:" IP6H "SIP6" IP6H "l2Mask:0x%X fwdMask:0x%X LUT_IDX=%d\n"
				,NIP6QUAD(mapping_entry_temp->groupIP), NIP6QUAD(mapping_entry_temp->sourceIP),
				mapping_entry_temp->l2membr, mapping_entry_temp->fwdmembr, mapping_entry_temp->hwflow_index);
			}

		}else{
			PROC_PRINTF("mapping_entry_temp is NULL\n");
		}
	}
	PROC_PRINTF("===End of Dump mapping-hw-list====\n");

}

int rtl_handle_igmpgroup_change(rtl_multicastEventContext_t* param)
{
	int retVal;

	struct rtl_multicastFwdInfo multicastFwdInfo;
	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastDeviceInfo_s igmp_snooping_module;

	struct rtl_groupEntry  *groupEntry = NULL;
	struct rtl_clientEntry *clientEntry= NULL;

	rtl_multicast_index_mapping_t* mapping_entry=NULL;
	rtl_multicast_index_mapping_t* mapping_entry_tmp=NULL;

	IGMP("=========Group change processing======\n");
	if(strlen(param->devName)==0)
	{
		DEBUG("param->devname==NULL, return fail\n");
		return FAIL;
	}

	/*case 1:this is multicast event from bridge(br0) module */
	if (strcmp(param->devName, RG_IGMP_SNOOPING_MODULE_NAME)!=0)
	{
		DEBUG("param->devname!=RG_IGMP_SNOOPING_MODULE_NAME, return fail\n");
		return FAIL;
	}

	groupEntry=rtl_searchGroupEntry(param->moduleIndex, param->ipVersion, param->groupAddr, param->vlanId);
	if (groupEntry==NULL)
	{
		IGMP("Group non-exist  clean this group hw_mapping");
		while(1)
		{
			mapping_entry =rtl_findMappingEntry(param->ipVersion,param->groupAddr,NULL, param->vlanId);
			if(mapping_entry==NULL)
				break; //clean done
			else
			{
				retVal = _rtl_handle_igmpgroup_delhw(mapping_entry);
			}
		}

	}
	else
	{

		IGMP("GIP exist");
		if (rg_db.systemGlobal.initParam.igmpSnoopingEnable==2 || NULL==groupEntry->clientList)
		{
			while(1)
			{
				mapping_entry =rtl_findMappingEntry(param->ipVersion,param->groupAddr,NULL, param->vlanId);
				if(mapping_entry==NULL)
					break; //clean done
				else
				{
					retVal = _rtl_handle_igmpgroup_delhw(mapping_entry);
				}
			}


			if(rg_db.systemGlobal.initParam.igmpSnoopingEnable==2)
				IGMP("Support IGMPv3 Mode clean done");
			else
				IGMP("GIP has no client => delete all GIP hw-entries");
		}


		mapping_entry_tmp=rtl_findMappingEntry(param->ipVersion,param->groupAddr,NULL, param->vlanId);


		clientEntry=groupEntry->clientList;
		while (clientEntry!=NULL)
		{
			struct rtl_sourceEntry *sourceEntry = NULL;
			sourceEntry=clientEntry->sourceList;
			do
			{
				//IGMP("for each source");

				bzero(&multicastDataInfo, sizeof(struct rtl_multicastDataInfo));
				multicastDataInfo.ipVersion=param->ipVersion;
				multicastDataInfo.vlanId=param->vlanId;
				multicastDataInfo.srcFilterMode = param->srcFilterMode; ////
				multicastDataInfo.groupAddr[0]= param->groupAddr[0];
				if (IP_VERSION6==param->ipVersion)
				{
					multicastDataInfo.groupAddr[1]= param->groupAddr[1];
					multicastDataInfo.groupAddr[2]= param->groupAddr[2];
					multicastDataInfo.groupAddr[3]= param->groupAddr[3];
				}

				if (sourceEntry!=NULL)
				{
					multicastDataInfo.sourceIp[0]=  sourceEntry->sourceAddr[0];
					if (IP_VERSION6==param->ipVersion)
					{
						multicastDataInfo.sourceIp[1]=  sourceEntry->sourceAddr[1];
						multicastDataInfo.sourceIp[2]=  sourceEntry->sourceAddr[2];
						multicastDataInfo.sourceIp[3]=  sourceEntry->sourceAddr[3];
						IGMP("test SrcIP(" IP6H ")", NIP6QUAD(sourceEntry->sourceAddr));
					}
					else
					{
						IGMP("test SrcIP(" IP4D ")", NIPQUAD(sourceEntry->sourceAddr));
					}
				}

				retVal = rtl_getMulticastDataFwdInfo(param->moduleIndex, &multicastDataInfo, &multicastFwdInfo);
				if (retVal!=SUCCESS)
				{
					IGMP("FAIL: rtl_getMulticastDataFwdInfo=%d, return\n",retVal);
					return FAIL;
				}

				retVal = rtl_getIgmpSnoopingModuleDevInfo(param->moduleIndex, &igmp_snooping_module);
				if (retVal!=SUCCESS)
				{
					IGMP("FAIL: rtl_getIgmpSnoopingModuleDevInfo=%d, return",retVal);
					return FAIL;
				}

				//dump_hwMappingEntry();

				mapping_entry = NULL;
				if (rg_db.systemGlobal.initParam.igmpSnoopingEnable==1)
					mapping_entry =rtl_findMappingEntry(param->ipVersion,param->groupAddr,NULL, param->vlanId);
				else
					mapping_entry =rtl_findMappingEntry(param->ipVersion,param->groupAddr,sourceEntry->sourceAddr, param->vlanId);

				if (!mapping_entry)
				{
					/* create sw_entry */
					//igmpSnoopingEnable==2 always clean all hw-entry and creat new entry for GIP-SIP
					//IGMP("Not found alloc a new mapping entry \n");
					mapping_entry = _rtk_hwMappingEntry_malloc();
					if (!mapping_entry){	IGMP("alloc fail, return!");return FAIL;}

					mapping_entry->groupIP[0] 	= param->groupAddr[0];
					mapping_entry->groupIP[1] 	= param->groupAddr[1];
					mapping_entry->groupIP[2] 	= param->groupAddr[2];
					mapping_entry->groupIP[3] 	= param->groupAddr[3];

					if (multicastFwdInfo.srcFilterMode != RTK_RG_IPV4MC_DONT_CARE_SRC)
					{
						if (sourceEntry!=NULL)
						{
							mapping_entry->sourceIP[0] 	= sourceEntry->sourceAddr[0];
							mapping_entry->sourceIP[1]	= sourceEntry->sourceAddr[1];
							mapping_entry->sourceIP[2]	= sourceEntry->sourceAddr[2];
							mapping_entry->sourceIP[3]	= sourceEntry->sourceAddr[3];
							IGMP("SIP4 : %pI4h"  , (mapping_entry->sourceIP));
						}

					#if defined(CONFIG_RG_RTL9600_SERIES)
						mapping_entry->routingMode = RTK_RG_IPV4MC_DIS_ROUTING;
					#else //CONFIG_RG_RTL9602C_SERIES & others new platform
						mapping_entry->routingMode = RTK_RG_IPV4MC_EN_ROUTING;
					#endif
					}
					else
					{
						mapping_entry->routingMode = RTK_RG_IPV4MC_EN_ROUTING;
						IGMP("the client is RTK_RG_IPV4MC_DONT_CARE_SRC");
					}

					mapping_entry->ipVersion	= param->ipVersion;
					mapping_entry->vlanId		= param->vlanId;
					if (multicastFwdInfo.srcFilterMode==RTK_RG_IPV4MC_EXCLUDE)
					{
					#if defined(CONFIG_RG_RTL9600_SERIES)
						mapping_entry->fwdmembr=multicastFwdInfo.fwdPortMask & RTK_RG_ALL_MAC_PORTMASK;
					#else
						mapping_entry->fwdmembr=multicastFwdInfo.fwdPortMask;
					#endif
					}
					else
					{
						mapping_entry->fwdmembr=multicastFwdInfo.fwdPortMask;
					}
					mapping_entry->srcFilterMode 	= multicastFwdInfo.srcFilterMode; ////

					/* add it into hw */
					if (mapping_entry->fwdmembr!=0) //have member port in this entry
					{
						int r=_rtl_handle_igmpgroup_addhw(mapping_entry);

						if (r!=SUCCESS)
						{
							IGMP("IGMP SNOOPING ADD:FAIL, return\n");
							//return FAIL;
						}
					}


					if (multicastFwdInfo.srcFilterMode==RTK_RG_IPV4MC_EXCLUDE && multicastFwdInfo.l2PortMask!=0)
					{

						mapping_entry->ipVersion	= param->ipVersion;
						mapping_entry->vlanId		= param->vlanId;
						mapping_entry->fwdmembr		= multicastFwdInfo.l2PortMask;
						mapping_entry->l2membr		= multicastFwdInfo.l2PortMask;
						mapping_entry->srcFilterMode 	= RTK_RG_IPV4MC_INCLUDE;
						if (mapping_entry->fwdmembr!=0)
						{
							int r=_rtl_handle_igmpgroup_addhw(mapping_entry);
							if (r!=SUCCESS)
							{
								IGMP("IGMP SNOOPING ADD:FAIL, return\n");
								//return FAIL;
							}
						}

						mapping_entry->srcFilterMode 	= RTK_RG_IPV4MC_EXCLUDE;
						mapping_entry->fwdmembr		= multicastFwdInfo.fwdPortMask;

					}


					if(param->ipVersion==IP_VERSION4)
					{
						IGMP("(GIP4: %pI4h) pMask:0x%X LUT_IDX=%d\n"
						,(param->groupAddr), multicastFwdInfo.fwdPortMask, mapping_entry->hwflow_index);
					}
					else
					{
						IGMP("(GIP6: " IP6H ") pMask:0x%X LUT_IDX=%d\n"
						,NIP6QUAD(param->groupAddr), multicastFwdInfo.fwdPortMask, mapping_entry->hwflow_index);
					}

				}
				else
				{

					//update dont care source mode forward portMask
					if(rg_db.systemGlobal.initParam.igmpSnoopingEnable==1)
					{

						if (multicastFwdInfo.fwdPortMask!=0 && (mapping_entry->fwdmembr!=multicastFwdInfo.fwdPortMask)) //have member port in this entry
						{
							mapping_entry->fwdmembr=multicastFwdInfo.fwdPortMask;
							if (_rtl_handle_igmpgroup_addhw(mapping_entry)!=SUCCESS)
							{
								IGMP("IGMP SNOOPING ADD:FAIL\n");
								//return FAIL;
							}
						}
						else if (multicastFwdInfo.fwdPortMask==0)
						{
							/* no member ports, clean it in hw */
							_rtl_handle_igmpgroup_delhw(mapping_entry);
						}

					}
					else
						IGMP("mapping_entry add by other Client Do nothing");

				}
				if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_TRACE_DUMP)
					dump_hwMappingEntry();


				if (sourceEntry!=NULL)
					sourceEntry=sourceEntry->next;
			} while (sourceEntry!=NULL);

			clientEntry=clientEntry->next;
			//IGMP("check nex client");
		} //while client != NULL


	}//group != NULL

	return SUCCESS;
}
#endif


//EXPORT_SYMBOL(igmp_show);
//EXPORT_SYMBOL(rtl_setIgmpSnoopingModuleDevInfo);
//EXPORT_SYMBOL(rtl_initMulticastSnooping);
//EXPORT_SYMBOL(rtl_registerIgmpSnoopingModule);
#endif

