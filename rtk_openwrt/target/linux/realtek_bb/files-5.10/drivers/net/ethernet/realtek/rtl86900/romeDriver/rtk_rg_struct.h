#ifndef RTK_RG_STRUCT_H
#define RTK_RG_STRUCT_H

#include <rtk_rg_error.h>
#include <rtk_rg_define.h>
#include <rtk_rg_port.h>

#ifdef __KERNEL__
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/semaphore.h>		//used to lock WAN interface L3 APIs
#include <linux/list.h>		//list_head and routines
#include <linux/fs.h>
#include <linux/workqueue.h> //for wq

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 30)
#else
#include <linux/config.h>
#endif

#ifdef CONFIG_RTL8686NIC
#if defined(CONFIG_RG_RTL9602C_SERIES)
	#include <re8686_rtl9602c.h>
#elif defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#if defined( CONFIG_APOLLO_FPGA_PHY_TEST)
	//APOLLO_FPGA_PHY_TEST(for 9607C FPGA build code) beloing to linux-2.6.30 platform, and it doesn't have such re8686_rtl9607c.h file, use re8686_rtl9602c.h
	#include <re8686_nic.h>
#else
	#include <re8686_rtl9607c.h>
#endif

#else
	#include <re8686.h>
#endif

#elif defined(CONFIG_RG_G3_SERIES)	// CONFIG_RG_G3_SERIES_DEVELOPMENT
	#include <linux/irq.h>	//for smp affinity
	#include <ca_ni.h>
	#include <re8686_rtl9607c.h>
#endif	// end CONFIG_RTL8686NIC

#else //model

#include <re8686_sim.h>
#endif

#include <rtk/acl.h>
#include <rtk/classify.h>
#include <rtk/l2.h>
#include <rtk/l34.h>
#ifdef CONFIG_GPON_FEATURE
#include <module/gpon/gpon.h>
#endif
#ifdef CONFIG_EPON_FEATURE
#include <rtk/epon.h>
#endif
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
#include <rtk/ponmac.h>
#endif


#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RTL9601B_SERIES) //CF action register is different between Apollo adn ApolloFE
#include <rtk_rg_acl_apollo.h>
#elif defined(CONFIG_RG_RTL9602C_SERIES)
#include <rtk_rg_acl_apolloFE.h>
#elif defined(CONFIG_RG_G3_SERIES)	// CONFIG_RG_G3_SERIES_DEVELOPMENT
#include <rtk_rg_acl_g3.h>
#include <asicDriver/rtk_rg_apolloPro_asicDriver.h>
#include <mcast.h>
#elif defined(CONFIG_RG_RTL9607C_SERIES)
#include <rtk_rg_apolloPro_acl.h>
#include <asicDriver/rtk_rg_apolloPro_asicDriver.h>
#elif defined(CONFIG_RG_RTL9603CVD_SERIES)
#include <rtk_rg_apolloPro_acl.h>
#include <asicDriver/rtk_rg_rtl9603cvd_asicDriver.h>
#endif

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#ifndef RG_PFLOW
#if defined(CONFIG_RG_FLOW_4K_MODE) || defined(CONFIG_RG_G3_SERIES)
#define RG_PFLOW(idx) ((idx<MAX_FLOW_HW_TABLE_SIZE)?(&rg_db.flowPathHwEntry[idx]):(&rg_db.flowPathSwEntry[idx-MAX_FLOW_HW_TABLE_SIZE]))
#else
#define RG_PFLOW(idx) ((idx<MAX_FLOW_HW_TABLE_SIZE)?((rgpro_db.ddrBusAlign32==ENABLED)?((rtk_rg_table_flowEntry_t *)(rgpro_db.ddrMemBase+(idx<<5))):((rtk_rg_table_flowEntry_t *)(rgpro_db.ddrMemBase+(idx*28)))):(&rg_db.flowPathSwEntry[idx-MAX_FLOW_HW_TABLE_SIZE]))
#endif
#endif
#endif


#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES)
/* single CPU definition for Apollo series */
#define RTK_RG_MAC_PORT_MAINCPU			RTK_RG_MAC_PORT_CPU
#define RTK_RG_MAC_PORT_LASTCPU			RTK_RG_MAC_PORT_CPU
#define RTK_RG_MAC_PORT_SLAVECPU 		RTK_RG_MAC_PORT_CPU

#define RTK_RG_PORT_MAINCPU				RTK_RG_PORT_CPU
#define RTK_RG_PORT_LASTCPU				RTK_RG_PORT_CPU
#define RTK_RG_PORT_SLAVECPU 			RTK_RG_PORT_CPU

#define RTK_RG_MAC_EXT_BASED_PORT		RTK_RG_MAC_EXT_CPU
#define RTK_RG_EXT_BASED_PORT			RTK_RG_PORT_CPU

#elif defined(CONFIG_RG_RTL9607C_SERIES)
/* dual CPU definition for ApolloPro */

#define RTK_RG_MAC_PORT_MAINCPU			RTK_RG_MAC_PORT_MASTERCPU_CORE0
#define RTK_RG_MAC_PORT_LASTCPU			RTK_RG_MAC_PORT_MASTERCPU_CORE1
#define RTK_RG_MAC_PORT_CPU 			RTK_RG_MAC_PORT_MASTERCPU_CORE1				// indicate the last physical port

#define RTK_RG_PORT_MAINCPU				RTK_RG_PORT_MASTERCPU_CORE0
#define RTK_RG_PORT_LASTCPU				RTK_RG_PORT_MASTERCPU_CORE1
#define RTK_RG_PORT_CPU					RTK_RG_PORT_MASTERCPU_CORE0				// NOTE: RG should not use RTK_RG_PORT_CPU anymore.

#define RTK_RG_MAC_EXT_BASED_PORT		RTK_RG_MAC_EXT_PORT0
#define RTK_RG_EXT_BASED_PORT			RTK_RG_EXT_PORT0

#elif defined(CONFIG_RG_RTL9603CVD_SERIES)
/* single CPU definition for 9603D */

#define RTK_RG_MAC_PORT_MAINCPU			RTK_RG_MAC_PORT_CPU
#define RTK_RG_MAC_PORT_LASTCPU			RTK_RG_MAC_PORT_CPU
#define RTK_RG_MAC_PORT_SLAVECPU 			RTK_RG_MAC_PORT_CPU				// indicate the last physical port

#define RTK_RG_PORT_MAINCPU				RTK_RG_PORT_CPU
#define RTK_RG_PORT_LASTCPU				RTK_RG_PORT_CPU
#define RTK_RG_PORT_SLAVECPU 				RTK_RG_PORT_CPU

#define RTK_RG_MAC_EXT_BASED_PORT		RTK_RG_MAC_EXT_PORT0
#define RTK_RG_EXT_BASED_PORT				RTK_RG_EXT_PORT0


#elif defined(CONFIG_RG_G3_SERIES)
/* single CPU definition for G3 series */
#define RTK_RG_MAC_PORT_MAINCPU			RTK_RG_MAC_PORT_CPU
#define RTK_RG_MAC_PORT_LASTCPU			RTK_RG_MAC_PORT_CPU_WLAN1_AND_OTHERS
#define RTK_RG_MAC_PORT_SLAVECPU 		RTK_RG_MAC_PORT_CPU

#define RTK_RG_PORT_MAINCPU				RTK_RG_PORT_CPU
#define RTK_RG_PORT_LASTCPU				RTK_RG_PORT_CPU_WLAN1_AND_OTHERS
#define RTK_RG_PORT_SLAVECPU 			RTK_RG_PORT_CPU

#define RTK_RG_MAC_EXT_BASED_PORT		RTK_RG_MAC_EXT_PORT0
#define RTK_RG_EXT_BASED_PORT			RTK_RG_EXT_PORT0

#else
#error
#endif

#ifndef CONFIG_RG_CALLBACK_WQ_TOTAL_SIZE
#define CONFIG_RG_CALLBACK_WQ_TOTAL_SIZE 1
#endif

/* DEBUG ================================================================= */

typedef enum rtk_rg_debug_level_e
{
	RTK_RG_DEBUG_LEVEL_DEBUG=0x1,
	RTK_RG_DEBUG_LEVEL_FIXME=0x2,
	RTK_RG_DEBUG_LEVEL_CALLBACK=0x4,
	RTK_RG_DEBUG_LEVEL_TRACE=0x8,
	RTK_RG_DEBUG_LEVEL_ACL=0x10,
	RTK_RG_DEBUG_LEVEL_WARN=0x20,
	RTK_RG_DEBUG_LEVEL_TRACE_DUMP=0x40,
	RTK_RG_DEBUG_LEVEL_WMUX=0x80,
	RTK_RG_DEBUG_LEVEL_MACLEARN=0x100,
	RTK_RG_DEBUG_LEVEL_TABLE=0x200,
	RTK_RG_DEBUG_LEVEL_ALG=0x400,
	RTK_RG_DEBUG_LEVEL_IGMP=0x800,
	RTK_RG_DEBUG_LEVEL_ACL_RRESERVED=0x1000,
	RTK_RG_DEBUG_LEVEL_RG_API=0x2000,
	RTK_RG_DEBUG_LEVEL_TIMER=0x4000,
	RTK_RG_DEBUG_LEVEL_FBDBG=0x10000,
	RTK_RG_DEBUG_LEVEL_FBIGR=0x20000,
	RTK_RG_DEBUG_LEVEL_FBEGR=0x40000,
	RTK_RG_DEBUG_LEVEL_ACL_CONTROL_PATH=0x80000,
	RTK_RG_DEBUG_LEVEL_TABLE_FULL=0x100000,
	RTK_RG_DEBUG_LEVEL_WIFI_FAST_FORWARD=0x200000,
	RTK_RG_DEBUG_LEVEL_HTTP=0x400000,
	RTK_RG_DEBUG_LEVEL_ALL=0xffffffff,
} rtk_rg_debug_level_t;

#define TRACFILTER_MAX 4


typedef enum rtk_rg_debug_trace_filter_bitmask_e
{
	RTK_RG_DEBUG_TRACE_FILTER_SPA=0x1, //source port 0~5 phyiscal Port, 6:CPU, 7:EXT0, 8:EXT1
	RTK_RG_DEBUG_TRACE_FILTER_DA=0x2,	//DNAC
	RTK_RG_DEBUG_TRACE_FILTER_SA=0x4,	//SMAC
	RTK_RG_DEBUG_TRACE_FILTER_ETH=0x8, //ethertype
	RTK_RG_DEBUG_TRACE_FILTER_SIP=0x10, //src IP
	RTK_RG_DEBUG_TRACE_FILTER_DIP=0x20, //dest IP
	RTK_RG_DEBUG_TRACE_FILTER_IP=0x40, //src IP or dest IP
	RTK_RG_DEBUG_TRACE_FILTER_L4PROTO=0x80, // Layer4 protocol
	RTK_RG_DEBUG_TRACE_FILTER_SPORT=0x100, // L4 Src Port
	RTK_RG_DEBUG_TRACE_FILTER_DPORT=0x200, // L4 Dst Port
	RTK_RG_DEBUG_TRACE_FILTER_REASON=0x400, // Trap reason
	RTK_RG_DEBUG_TRACE_FILTER_CVLAN=0x800, // CVLAN
	RTK_RG_DEBUG_TRACE_FILTER_SVLAN=0x1000, // SVLAN
	RTK_RG_DEBUG_TRACE_FILTER_PPPOESESSIONID=0x2000, // PPPOE sessionID
	RTK_RG_DEBUG_TRACE_FILTER_V6DIP=0x4000, // IPV6 DIP
	RTK_RG_DEBUG_TRACE_FILTER_V6SIP=0x8000, // IPV6 SIP
	RTK_RG_DEBUG_TRACE_FILTER_SHOWNUMBEROFTIMES=0x10000, //TRACEFILTER SHOW NUMBER OF TIMES
	RTK_RG_DEBUG_TRACE_FILTER_FLOW_CHANGE=0x20000, 		 //SHOW Debug Message When Flow Change
	RTK_RG_DEBUG_TRACE_FILTER_SIP_RANGE=0x40000, //src IP range
	RTK_RG_DEBUG_TRACE_FILTER_DIP_RANGE=0x80000, //dst IP range
	RTK_RG_DEBUG_TRACE_FILTER_V6SIP_RANGE=0x100000, //src IPV6 range
	RTK_RG_DEBUG_TRACE_FILTER_V6DIP_RANGE=0x200000, //dst IPV6 range
	RTK_RG_DEBUG_TRACE_FILTER_LEN_RANGE=0x400000, //len range
	RTK_RG_DEBUG_TRACE_FILTER_TCP_FLAGS_MSK=0x800000, //TCP flags mask
} rtk_rg_debug_trace_filter_bitmask_t;


typedef struct rtk_rgDebugTraceFilter_s
{
	uint16		spa;
	rtk_mac_t	dmac;
	rtk_mac_t	dmac_mask;
	rtk_mac_t	smac;
	rtk_mac_t	smac_mask;
	uint16		ethertype;
	uint32		sip;
	uint32		dip;
	uint8		sipv6[16];
	uint8		dipv6[16];
	uint32		ip;
	uint16		cvlanid;
	uint16		svlanid;
	uint16		sessionid;
	uint16		showNumberOfTimes;
	uint16		showNumberOfTimesCounter;
	uint16		l4proto;
	uint16		sport;
	uint32		flowIdxCh;		//flow change
	uint16		dport;
	uint8		reason;
	uint32		sip_lowbound;
	uint32		sip_highbound;
	uint32		dip_lowbound;
	uint32		dip_highbound;
	uint8		sipv6_lowbound[16];
	uint8		sipv6_highbound[16];
	uint8		dipv6_lowbound[16];
	uint8		dipv6_highbound[16];
	uint32		len_lowbound;
	uint32		len_highbound;
	uint16		tcp_flags_enable;	//12bits
	uint16		tcp_flags_value;	//12bits
}rtk_rgDebugTraceFilter_t;

typedef enum rtk_rg_hwnatState_e
{
	RG_HWNAT_DISABLE=0,
	RG_HWNAT_ENABLE,
	RG_HWNAT_PROTOCOL_STACK,
	RG_HWNAT_UC_FORCE_HW_FWD,
	RG_HWNAT_UC_FORCE_PROTOCOL_STACK,
}rtk_rg_hwnatState_t;

/* End of DEBUG ========================================================== */


/* RTK RG API ============================================================= */

typedef struct rtk_rg_mac_portmask_s
{
	uint32 portmask;	//the portmask bit should be defined by rtk_rg_mac_port_idx_t.
}rtk_rg_mac_portmask_t;

typedef struct rtk_rg_portmask_e
{
	uint32 portmask;	//the portmask bit should be defined by rtk_rg_port_idx_t.
}rtk_rg_portmask_t;

typedef enum rtk_rg_binding_idx_e
{
	RTK_RG_BD_EXT_PORT0=0,
	RTK_RG_BD_EXT_PORT1=1,
	RTK_RG_BD_EXT_PORT2=2,
	RTK_RG_BD_EXT_PORT3=3,
	RTK_RG_BD_EXT_PORT4=4,
	RTK_RG_BD_EXT_PORT5=5,
#if defined(CONFIG_RG_RTL9607C_SERIES)
	RTK_RG_BD_MAC10_EXT_PORT0=6,
	RTK_RG_BD_MAC10_EXT_PORT1,
	RTK_RG_BD_MAC10_EXT_PORT2,
	RTK_RG_BD_MAC10_EXT_PORT3,
	RTK_RG_BD_MAC10_EXT_PORT4,
	RTK_RG_BD_MAC10_EXT_PORT5,
	RTK_RG_BD_MAC7_EXT_PORT0=12,
	RTK_RG_BD_MAC7_EXT_PORT1,
	RTK_RG_BD_MAC7_EXT_PORT2,
	RTK_RG_BD_MAC7_EXT_PORT3,
	RTK_RG_BD_MAC7_EXT_PORT4,
	RTK_RG_BD_MAC7_EXT_PORT5,
#endif
	RTK_RG_BD_PORT_MAX,
} rtk_rg_binding_idx_t;

typedef struct rtk_rg_port_isolation_s
{
	rtk_rg_port_idx_t port;
	rtk_rg_portmask_t portmask;
}rtk_rg_port_isolation_t;

typedef struct rtk_rg_cached_port_isolation_s
{
	uint8 valid;	//1st flag
	rtk_rg_portmask_t mask;	//record portmask need to modified: 0-0x7f
	rtk_rg_portmask_t portmask[RTK_RG_PORT_MAX];	//CPU LAN+WAN port only
}rtk_rg_cached_port_isolation_t;

//System
typedef enum rtk_rg_ip_version_e
{
	IPVER_V4ONLY=0,
	IPVER_V6ONLY=1,
	IPVER_V4V6=2,
	IPVER_END,
}rtk_rg_ip_version_t;

typedef enum rtk_rg_bridgeBindWan_ip_version_e
{
	BGBWP_NONE=0,
	BGBWP_V4ONLY,
	BGBWP_V6ONLY,
	BGBWP_V4V6,
	BGBWP_END,
}rtk_rg_bridgeBindWan_ip_version_t;

typedef enum rtk_rg_ip_updated_e
{
	ONLY_IPV4_UPDATED=0,
	ONLY_IPV6_UPDATED,
	IPV4_IPV6_UPDATED,
	NO_IP_UPDATED,
}rtk_rg_ip_updated_t;

typedef struct rtk_rg_VersionString_s
{
	char version_string[128];
} rtk_rg_VersionString_t;

typedef struct rtk_rg_macEntry_s
{
	rtk_mac_t mac;
	uint32 isIVL:1; //0:SVL, 1:IVL
	uint32 fid; //only used in SVL
	int vlan_id; //egress to this MAC, add a CVLAN tag. (vlan_id=0, untag)
	rtk_rg_port_idx_t port_idx;
	uint32 arp_used:1;
	uint32 static_entry:1;
	uint32 sa_block:1;
	uint32 da_block:1;
	uint32 fix_l34_vlan:1;
	uint32 auth:1;
#if defined(CONFIG_RG_RTL9600_SERIES)
#else	// support ctag_if
	uint32 ctag_if:1;	//0:untag, 1:tagged
#endif
#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support lut traffic bit
	uint32 idleSecs;
#endif

	int8 wlan_device_idx;	//used only when port is CPU and extport is 1 or 2  (G3 platform: -2 stands for lan interface, -3 stands for wan interface)
	uint32 countingInLearningLimit: 1;
}rtk_rg_macEntry_t;

typedef struct rtk_rg_arpEntry_s
{
	int macEntryIdx;
	ipaddr_t ipv4Addr;
	uint32 staticEntry:1;
}rtk_rg_arpEntry_t;

//LANNefInfo
extern const char* rg_lanNet_phone_type[];
extern const char* rg_lanNet_computer_type[];
extern const char* rg_lanNet_brand[][MAX_LANNET_SUB_BRAND_SIZE];
extern const char* rg_lanNet_model[][MAX_LANNET_SUB_MODEL_SIZE];
extern const char* rg_lanNet_os[][MAX_LANNET_SUB_OS_SIZE];

//ForcePortal
extern const char* rg_forcePortal_browser_type[];

typedef enum rtk_rg_lanNet_device_type_e
{
	RG_LANNET_TYPE_OTHER,
	RG_LANNET_TYPE_PHONE,
	RG_LANNET_TYPE_COMPUTER,
}rtk_rg_lanNet_device_type_t;

typedef enum rtk_rg_lanNet_brand_e
{
	RG_BRAND_OTHER=0,
	RG_BRAND_HUAWEI,
	RG_BRAND_XIAOMI,
	RG_BRAND_MEIZU,
	RG_BRAND_IPHONE,
	RG_BRAND_NOKIA,
	RG_BRAND_SAMSUNG,
	RG_BRAND_SONY,
	RG_BRAND_ERICSSON,
	RG_BRAND_MOT,
	RG_BRAND_HTC,
	RG_BRAND_SGH,
	RG_BRAND_LG,
	RG_BRAND_SHARP,
	RG_BRAND_PHILIPS,
	RG_BRAND_PANASONIC,
	RG_BRAND_ALCATEL,
	RG_BARND_LENOVO,
	RG_BARND_OPPO,
	RG_BRAND_END,
}rtk_rg_lanNet_brand_t;

typedef enum rtk_rg_lanNet_model_e
{
	RG_MODEL_OTHER=0,
	RG_MODEL_HONOR,
	RG_MODEL_P7,
	RG_MODEL_R7,
	RG_MODEL_MI4LTE,
	RG_MODEL_END,
}rtk_rg_lanNet_model_t;

typedef enum rtk_rg_lanNet_os_e
{
	RG_OS_OTHER=0,
	RG_OS_WINDOWS_NT,
	RG_OS_MACINTOSH,
	RG_OS_IOS,
	RG_OS_ANDROID,
	RG_OS_WINDOWS_PHONE,
	RG_OS_LINUX,
	RG_OS_END,
}rtk_rg_lanNet_os_t;

typedef enum rtk_rg_lanNet_connect_type_e
{
	RG_CONN_MAC_PORT=0,
	RG_CONN_WIFI,
}rtk_rg_lanNet_connect_type_t;


typedef enum rtk_rg_proc_group_e
{
	RTK_RG_PROC_GROUP_NON,
	RTK_RG_PROC_GROUP_ACL,
	RTK_RG_PROC_GROUP_RATELIMIT,
	RTK_RG_PROC_GROUP_MULTICAST,
	RTK_RG_PROC_GROUP_DROP,
	RTK_RG_PROC_GROUP_TRAP,
	RTK_RG_PROC_GROUP_DEBUG,
	RTK_RG_PROC_GROUP_VLAN,
	RTK_RG_PROC_GROUP_AGING,
	RTK_RG_PROC_GROUP_QOS,
	RTK_RG_PROC_GROUP_TCPUDP,
	RTK_RG_PROC_GROUP_CTC,
	RTK_RG_PROC_GROUP_CONGESTIONCTRL,
	RTK_RG_PROC_GROUP_WIFI,
	RTK_RG_PROC_GROUP_ARP,
	RTK_RG_PROC_GROUP_ALG,
	RTK_RG_PROC_GROUP_PS,
	RTK_RG_PROC_GROUP_PON,
	RTK_RG_PROC_GROUP_WMUX,
	RTK_RG_PROC_GROUP_TEST,
	RTK_RG_PROC_GROUP_L2,
	RTK_RG_PROC_GROUP_SMP,
	RTK_RG_PROC_GROUP_CMCC,
	RTK_RG_PROC_GROUP_MAX,
} rtk_rg_proc_group_t;


typedef enum rtk_rg_proc_group_bitmask_e
{
	RTK_RG_PROC_GROUP_ACL_BITMASK=(1<<RTK_RG_PROC_GROUP_ACL),
	RTK_RG_PROC_GROUP_RATELIMIT_BITMASK=(1<<RTK_RG_PROC_GROUP_RATELIMIT),
	RTK_RG_PROC_GROUP_MULTICAST_BITMASK=(1<<RTK_RG_PROC_GROUP_MULTICAST),
	RTK_RG_PROC_GROUP_DROP_BITMASK=(1<<RTK_RG_PROC_GROUP_DROP),
	RTK_RG_PROC_GROUP_TRAP_BITMASK=(1<<RTK_RG_PROC_GROUP_TRAP),
	RTK_RG_PROC_GROUP_DEBUG_BITMASK=(1<<RTK_RG_PROC_GROUP_DEBUG),
	RTK_RG_PROC_GROUP_VLAN_BITMASK=(1<<RTK_RG_PROC_GROUP_VLAN),
	RTK_RG_PROC_GROUP_AGING_BITMASK=(1<<RTK_RG_PROC_GROUP_AGING),
	RTK_RG_PROC_GROUP_QOS_BITMASK=(1<<RTK_RG_PROC_GROUP_QOS),
	RTK_RG_PROC_GROUP_TCPUDP_BITMASK=(1<<RTK_RG_PROC_GROUP_TCPUDP),
	RTK_RG_PROC_GROUP_CTC_BITMASK=(1<<RTK_RG_PROC_GROUP_CTC),
	RTK_RG_PROC_GROUP_CONGESTIONCTRL_BITMASK=(1<<RTK_RG_PROC_GROUP_CONGESTIONCTRL),
	RTK_RG_PROC_GROUP_WIFI_BITMASK=(1<<RTK_RG_PROC_GROUP_WIFI),
	RTK_RG_PROC_GROUP_ARP_BITMASK=(1<<RTK_RG_PROC_GROUP_ARP),
	RTK_RG_PROC_GROUP_ALG_BITMASK=(1<<RTK_RG_PROC_GROUP_ALG),
	RTK_RG_PROC_GROUP_PS_BITMASK=(1<<RTK_RG_PROC_GROUP_PS),
	RTK_RG_PROC_GROUP_PON_BITMASK=(1<<RTK_RG_PROC_GROUP_PON),
	RTK_RG_PROC_GROUP_WMUX_BITMASK=(1<<RTK_RG_PROC_GROUP_WMUX),
	RTK_RG_PROC_GROUP_TEST_BITMASK=(1<<RTK_RG_PROC_GROUP_TEST),
	RTK_RG_PROC_GROUP_L2_BITMASK=(1<<RTK_RG_PROC_GROUP_L2),
	RTK_RG_PROC_GROUP_SMP_BITMASK=(1<<RTK_RG_PROC_GROUP_SMP),
	RTK_RG_PROC_GROUP_CMCC_BITMASK=(1<<RTK_RG_PROC_GROUP_CMCC),
} rtk_rg_proc_group_bitmask_t;


typedef struct rtk_rg_lanNetInfo_s
{
	//connection type, Dev Name,  brand, OS, device type
	char dev_name[MAX_LANNET_DEV_NAME_LENGTH];	//for LANNetInfo
	rtk_rg_lanNet_device_type_t dev_type;
	rtk_rg_lanNet_brand_t brand;
	rtk_rg_lanNet_model_t model;
	rtk_rg_lanNet_os_t os;
	rtk_rg_lanNet_connect_type_t conn_type;		//for LANNetInfo
	unsigned int checked;	//at most check MAX_LANNET_PARSE_COUNT times
}rtk_rg_lanNetInfo_t;

typedef struct rtk_rg_arpInfo_s
{
	rtk_rg_arpEntry_t arpEntry;
	int valid;
	int idleSecs;
	//lan device info
	rtk_rg_lanNetInfo_t lanNetInfo;
	char brandStr[MAX_LANNET_BRAND_NAME_LENGTH];
	char modelStr[MAX_LANNET_MODEL_NAME_LENGTH];
	char osStr[MAX_LANNET_OS_NAME_LENGTH];
}rtk_rg_arpInfo_t;

typedef struct rtk_rg_neighborEntry_s
{
	uint32 l2Idx;
	uint8 matchRouteIdx;
	uint8 interfaceId[IPV6_ADDR_LEN];
	uint8 valid:1;
	uint8 staticEntry:1;
}rtk_rg_neighborEntry_t;

typedef struct rtk_rg_neighborInfo_s
{
	rtk_rg_neighborEntry_t neighborEntry;
	int idleSecs;
}rtk_rg_neighborInfo_t;


typedef enum rtk_rg_portMirrorInfo_direction_e
{
	RTK_RG_MIRROR_TX_RX_BOTH=0,
	RTK_RG_MIRROR_RX_ONLY,
	RTK_RG_MIRROR_TX_ONLY,
	RTK_RG_MIRROR_END
} rtk_rg_portMirrorInfo_direction_t;

typedef struct rtk_rg_portMirrorInfo_s
{
	uint32 monitorPort;
	rtk_rg_mac_portmask_t enabledPortMask;
	rtk_rg_portMirrorInfo_direction_t direct;
}rtk_rg_portMirrorInfo_t;

typedef enum rtk_rg_enable_e
{
    RTK_RG_DISABLED = 0,
    RTK_RG_ENABLED,
    RTK_RG_ENABLE_END
} rtk_rg_enable_t;
typedef enum rtk_rg_port_speed_e
{
    RTK_RG_PORT_SPEED_10M = 0,
    RTK_RG_PORT_SPEED_100M,
    RTK_RG_PORT_SPEED_1000M,
    RTK_RG_PORT_SPEED_END,
} rtk_rg_port_speed_t;
typedef enum rtk_rg_port_duplex_e
{
    RTK_RG_PORT_HALF_DUPLEX = 0,
    RTK_RG_PORT_FULL_DUPLEX,
    RTK_RG_PORT_DUPLEX_END
} rtk_rg_port_duplex_t;
typedef enum rtk_rg_port_linkStatus_e
{
    RTK_RG_PORT_LINKDOWN = 0,
    RTK_RG_PORT_LINKUP,
    RTK_RG_PORT_LINKSTATUS_END
} rtk_rg_port_linkStatus_t;

typedef struct rtk_rg_phyPortAbilityInfo_s
{
	rtk_rg_enable_t			force_disable_phy;
	rtk_rg_enable_t 		valid;
	rtk_rg_port_speed_t		speed;
	rtk_rg_port_duplex_t	duplex;
	rtk_rg_enable_t			flowCtrl;
	rtk_rg_enable_t			fc; //valid when flowCtrl is enabled
	rtk_rg_enable_t			asym_fc;// valid when flowCtrl is enabled
}rtk_rg_phyPortAbilityInfo_t;

typedef struct rtk_rg_portStatusInfo_s
{
	rtk_rg_port_linkStatus_t linkStatus;
	rtk_rg_port_speed_t		linkSpeed;
	rtk_rg_port_duplex_t	linkDuplex;
}rtk_rg_portStatusInfo_t;


typedef enum rtk_rg_storm_type_e
{
    RTK_RG_STORM_TYPE_UNKNOWN_UNICAST = 0,
    RTK_RG_STORM_TYPE_UNKNOWN_MULTICAST,
    RTK_RG_STORM_TYPE_MULTICAST,
    RTK_RG_STORM_TYPE_BROADCAST,
    RTK_RG_STORM_TYPE_DHCP,
    RTK_RG_STORM_TYPE_ARP,
    RTK_RG_STORM_TYPE_IGMP_MLD,
    RTK_RG_STORM_TYPE_END
} rtk_rg_storm_type_t;

typedef struct rtk_rg_stormControlInfo_s
{
	rtk_rg_enable_t		valid;
	rtk_rg_port_idx_t	port;
	rtk_rg_storm_type_t stormType;
	uint32 				meterIdx;
}rtk_rg_stormControlInfo_t;

typedef struct rtk_rg_qos_queue_weights_s
{
    uint32 weights[RTK_RG_MAX_NUM_OF_QUEUE];
} rtk_rg_qos_queue_weights_t;


typedef struct rtk_rg_ipv4RoutingEntry_s
{
	ipaddr_t dest_ip;
	ipaddr_t ip_mask;
	ipaddr_t nexthop; //0:for Interface route
	int wan_intf_idx;
} rtk_rg_ipv4RoutingEntry_t;

typedef struct rtk_rg_ipv6RoutingEntry_s
{
	rtk_ipv6_addr_t dest_ip;
	int prefix_len;
	int NhOrIntfIdx;
	rtk_l34_ipv6RouteType_t type;
} rtk_rg_ipv6RoutingEntry_t;

typedef enum rtk_rg_binding_type_e
{
	BIND_TYPE_PORT,
	BIND_TYPE_VLAN,
} rtk_rg_binding_type_t;

typedef struct rtk_rg_vlanBindingEntry_e
{
	rtk_rg_port_idx_t vlan_bind_port_idx;
	int vlan_bind_vlan_id;
} rtk_rg_vlanBindingEntry_t;


typedef struct rtk_rg_bindingEntry_s
{
	rtk_rg_binding_type_t type;
	union
	{
		rtk_rg_portmask_t port_bind_pmask;
		rtk_rg_vlanBindingEntry_t vlan;
	};
	int wan_intf_idx;
} rtk_rg_bindingEntry_t;

//For TCP bridge/routing Flow
typedef enum rtk_rg_tcpState_e
{
	TCP_STATE_INVALID			=0,
	TCP_STATE_SYN_RECV			=1,
	TCP_STATE_SYN_ACK_RECV		=2,
	TCP_STATE_TCP_CONNECTED		=3,
	TCP_STATE_RST_RECV			=4,
	TCP_STATE_FIRST_FIN			=5,
	TCP_STATE_FIN_SEND_AND_RECV	=6,
	TCP_STATE_LAST_ACK			=7
} rtk_rg_tcpState_t;

//For TCP napt Flow
typedef enum rtk_rg_naptState_e
{
	INVALID		=0,
	SYN_RECV	=1,
	UDP_FIRST	=2,
	SYN_ACK_RECV =3,
	UDP_SECOND	=4,
	TCP_CONNECTED	=5,
	UDP_CONNECTED	=6,
	FIRST_FIN	=7,
	RST_RECV	=8,

	FIN_SEND_AND_RECV	=9,
	LAST_ACK	=10,
	FIRST_FIN_IN=11,
	FIRST_FIN_OUT=12,
} rtk_rg_naptState_t;


typedef enum rtk_rg_naptInType_e
{
	NAPT_IN_TYPE_SYMMETRIC_NAPT=0,
	NAPT_IN_TYPE_RESTRICTED_CONE=1,
	NAPT_IN_TYPE_FULL_CONE=2,
	NAPT_IN_TYPE_END,
} rtk_rg_naptInType_t;

typedef enum rtk_rg_asicNaptInType_e
{
	ASIC_NAPT_IN_TYPE_INVALID=0,
	ASIC_NAPT_IN_TYPE_FULL_CONE=1,
	ASIC_NAPT_IN_TYPE_PORT_RESTRICTED_CONE=2,
	ASIC_NAPT_IN_TYPE_RESTRICTED_CONE=3,
	ASIC_NAPT_IN_TYPE_END
} rtk_rg_asicNaptInType_t;


typedef struct rtk_rg_naptEntry_s
{
	uint32 is_tcp:1;
	ipaddr_t local_ip;
	ipaddr_t remote_ip;
	int wan_intf_idx;
	unsigned short int local_port;
	unsigned short int remote_port;
	unsigned short int external_port;
	uint8 outbound_pri_valid:1;
	uint8 outbound_priority;
	uint8 inbound_pri_valid:1;
	uint8 inbound_priority;
	rtk_rg_naptInType_t coneType;	//default 0 as symmetric cone type
} rtk_rg_naptEntry_t;


typedef enum  rtk_rg_naptDirection_e
{
	NAPT_DIRECTION_OUTBOUND	=0,
	NAPT_DIRECTION_INBOUND	=1,
	NAPT_DIRECTION_ROUTING	=2, //bi-direction
	IPV6_ROUTE_OUTBOUND =3,	//support for IPV6 stateful
	IPV6_ROUTE_INBOUND =4,	//support for IPv6 stateful
}rtk_rg_naptDirection_t;

typedef struct rtk_rg_naptInfo_s
{
	rtk_rg_naptEntry_t naptTuples;
	uint32	idleSecs;
	rtk_rg_naptState_t	state;
	void **pContext;
} rtk_rg_naptInfo_t;


typedef enum  rtk_rg_virtualServerMappingType_e
{
	VS_MAPPING_N_TO_N	=0,
	VS_MAPPING_N_TO_1	=1,
}rtk_rg_virtualServerMappingType_t;

typedef enum  rtk_rg_portTriggerNatType_e
{
	PT_NATTYPE_INCOMING	=0,
	PT_NATTYPE_OUTGOING	=1,
}rtk_rg_portTriggerNatType_t;

#if defined(RTK_RG_INGRESS_QOS_TEST_PATCH) && defined(CONFIG_RG_RTL9600_SERIES)
typedef enum rtk_rg_ingress_qos_testing_type_s
{
	RTK_RG_INGRESS_QOS_ORIGINAL = 0,
	RTK_RG_INGRESS_QOS_ALL_HIGH_QUEUE,
} rtk_rg_ingress_qos_testing_type_t;
#endif

#if 1
typedef enum rtk_rg_lease_time_type_e
{
	LEASE_TIME_TYPE_1WEEK=0,
	LEASE_TIME_TYPE_1DAY=1,
	LEASE_TIME_TYPE_1HOUR=2,
	LEASE_TIME_TYPE_1MIN=3,
} rtk_rg_lease_time_type_t;
#endif

typedef enum rtk_rg_house_keep_select_e
{
#if defined(CONFIG_RG_FLOW_AUTO_AGEOUT)
	//RTK_RG_FLOW_HOUSE_KEEP_SELECT,
	RTK_RG_L2L3_TCP_UDP_CONNECTION_HOUSE_KEEP_SELECT,
#else	//not CONFIG_RG_FLOW_AUTO_AGEOUT
#if defined(CONFIG_ROME_NAPT_SHORTCUT) || defined(CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT)
	RTK_RG_SHORTCUT_HOUSE_KEEP_SELECT,
#endif
#endif
#if defined(CONFIG_RG_LAYER2_SOFTWARE_LEARN)
	RTK_RG_LAYER2_HOUSE_KEEP_SELECT,
#endif
#if defined(CONFIG_RG_NAPT_AUTO_AGEOUT)
	RTK_RG_NAPT_HOUSE_KEEP_SELECT,
#endif
#if defined(CONFIG_RG_ARP_AUTO_AGEOUT)
	RTK_RG_ARP_HOUSE_KEEP_SELECT,
	RTK_RG_NEIGHBOR_HOUSE_KEEP_SELECT,
#endif
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	RTK_RG_IPV6_STATEFUL_HOUSE_KEEP_SELECT,
	RTK_RG_IPV6_FRAGMENT_HOUSE_KEEP_SELECT,
#endif
	RTK_RG_IPV4_FRAGMENT_HOUSE_KEEP_SELECT,
	RTK_RG_ALG_HOUSE_KEEP_SELECT,
#if defined(CONFIG_RTK_RG_DRV_EVENT_RECORD_VERSION) && (CONFIG_RTK_RG_DRV_EVENT_RECORD_VERSION > 0)
	RTK_RG_DRV_EVENT_RECORD_SELECT,
#endif
	RTK_RG_MAX_HOUSE_KEEP_SELECT,
} rtk_rg_house_keep_select_t;

typedef enum rtk_rg_idle_time_type_e
{
	RG_IDLETIME_TYPE_ARP=0,
	RG_IDLETIME_TYPE_FLOW,
	RG_IDLETIME_TYPE_FRAGMENT,
	RG_IDLETIME_TYPE_ICMP,
	RG_IDLETIME_TYPE_IPV4_SHORTCUT,
	RG_IDLETIME_TYPE_IPV6_SHORTCUT,
	RG_IDLETIME_TYPE_LUT,
	RG_IDLETIME_TYPE_NEIGHBOR,
	RG_IDLETIME_TYPE_TCP_LONG,
	RG_IDLETIME_TYPE_TCP_SHORT,
	RG_IDLETIME_TYPE_TCP_SHORT_HOUSEKEEP_JIFFIES,
	RG_IDLETIME_TYPE_UDP_LONG,
	RG_IDLETIME_TYPE_UDP_SHORT,
}rtk_rg_idle_time_type_t;


// LAN Interface
typedef struct rtk_rg_lanIntfConf_s
{
	rtk_rg_ip_version_t ip_version;		//0: ipv4, 1: ipv6, 2:both v4 & v6
	rtk_mac_t gmac;
	ipaddr_t ip_addr;
	ipaddr_t ip_network_mask;
	rtk_ipv6_addr_t ipv6_addr;
	int ipv6_network_mask_length;
	rtk_rg_portmask_t port_mask;
	rtk_rg_mac_portmask_t untag_mask;
	//rtk_portmask_t port_mask;
	//rtk_portmask_t extport_mask;
	int intf_vlan_id;
	rtk_rg_enable_t vlan_based_pri_enable; //xdsl not support vlan pri, should set DISABLED
	int vlan_based_pri;
#if 0
	int dhcp_server_enable;
	rtk_rg_lease_time_type_t lease_time;
	ipaddr_t dhcp_start_ip_addr;
	ipaddr_t dhcp_end_ip_addr;
	rtk_rg_portmask_t dhcp_port_binding_mask;
	//rtk_portmask_t dhcp_port_binding_mask;
	//rtk_portmask_t dhcp_extport_binding_mask;
#endif
	unsigned int mtu;
	//int pppoe_passThrough;		//1:turn on, 0:turn off
	unsigned int isIVL:1;		//0: SVL, 1:IVL
	unsigned int replace_subnet:1;		//0: add one more subnet, 1:delete old, add new one
#if defined(CONFIG_RG_RTL9600_SERIES)
	unsigned int add_sw_arp:1;		//1: forcibly add sw arp
#endif
} rtk_rg_lanIntfConf_t;



//WAN Interface
typedef enum rtk_rg_wan_type_e
{
	RTK_RG_STATIC=0,
	RTK_RG_DHCP,
	RTK_RG_PPPoE,
	RTK_RG_BRIDGE,
	RTK_RG_PPTP,
	RTK_RG_L2TP,		//5
	RTK_RG_DSLITE,
	RTK_RG_PPPoE_DSLITE,
	RTK_RG_VXLAN,
	RTK_RG_WAN_TYPE_END,
} rtk_rg_wan_type_t;

typedef enum rtk_rg_wirelessWan_e
{
	RG_WWAN_WIRED=0,
	RG_WWAN_WLAN0_VXD=WLAN_DEVICE_NUM-1,		//wlan0 dev-idx
	RG_WWAN_WLAN1_VXD=MAX_WLAN_DEVICE_NUM-1,	//wlan1 dev-idx
}rtk_rg_wirelessWan_t;

typedef struct rtk_rg_wanIntfConf_s
{
	rtk_rg_wan_type_t wan_type;
	rtk_mac_t gmac;
	//rtk_portmask_t wan_port_mask;	//PON or RGMII
	rtk_rg_port_idx_t wan_port_idx;
	rtk_rg_portmask_t port_binding_mask;
	rtk_rg_portmask_t vlan_binding_mask;	//update by vlan-binding api
	//rtk_portmask_t port_binding_mask; //LAN port
	//rtk_portmask_t extport_binding_mask; //WLAN port
	unsigned int egress_vlan_tag_on:1;
	int egress_vlan_id;
	rtk_rg_enable_t vlan_based_pri_enable;
	int vlan_based_pri;
	unsigned int isIVL:1;				//0: SVL, 1:IVL
	unsigned int none_internet:1;		//0: internet, 1:other
	unsigned int forcedAddNewIntf:1;	//0:normal, 1:forced create new interface
	unsigned int forbiddenUnbindMLD:1;		//0:normal, 1:forbid unbind MLD for this interface
	unsigned int forbiddenUnbindIGMP:1;		//0:normal, 1:forbid unbind IGMP for this interface
	unsigned int wlan0_dev_binding_mask;
	rtk_rg_bridgeBindWan_ip_version_t bridgeToBindingWanByProtocol;	//0: none, 1: ipv4, 2: ipv6, 3:both v4 & v6
} rtk_rg_wanIntfConf_t;

typedef struct rtk_rg_ipStaticInfo_s
{
	rtk_rg_ip_version_t ip_version;		//0: ipv4, 1: ipv6, 2:both v4 & v6
	unsigned int  napt_enable:1; // L3 or L4
	unsigned int  ipv6_napt_enable;// 0: disable ipv6 napt/NPTv6, 1: enable ipv6 napt, 2: enable NPTv6
	ipaddr_t ip_addr;
	ipaddr_t remote_host_ip_addr;
	ipaddr_t ip_network_mask;
	unsigned int ipv4_default_gateway_on:1;				//1:should set default route, 0:otherwise
	ipaddr_t gateway_ipv4_addr;
	rtk_ipv6_addr_t ipv6_addr;
	unsigned int ipv6_mask_length;
	unsigned int ipv6_default_gateway_on:1;			//1:should set default route, 0:otherwise
	rtk_ipv6_addr_t gateway_ipv6_addr;
	unsigned int mtu;
	unsigned int gw_mac_auto_learn_for_ipv4:1;
	unsigned int gw_mac_auto_learn_for_ipv6:1;
	rtk_mac_t gateway_mac_addr_for_ipv4;
	rtk_mac_t gateway_mac_addr_for_ipv6;
	unsigned int static_route_with_arp:1;	//routing by ARP when gateway_ipv4_addr != 0
	//For NPTv6
	rtk_ipv6_addr_t nptv6_ipv6_addr;
	unsigned int nptv6_ipv6_mask_length;
} rtk_rg_ipStaticInfo_t;

typedef struct rtk_rg_ipDsliteStaticInfo_s
{
	rtk_rg_ipStaticInfo_t static_info;
	rtk_l34_dsliteInf_entry_t rtk_dslite;
	uint32 aftr_mac_auto_learn;
	rtk_mac_t aftr_mac_addr;
} rtk_rg_ipDslitStaticInfo_t;

typedef enum rtk_rg_dhcp_status_e
{
	DHCP_STATUS_LEASED=0,
	DHCP_STATUS_RELEASED=1,
	DHCP_STATUS_END
} rtk_rg_dhcp_status_t;

typedef struct rtk_rg_ipDhcpClientInfo_s
{
	rtk_rg_ipStaticInfo_t hw_info;
	rtk_rg_dhcp_status_t stauts; //leased, released
} rtk_rg_ipDhcpClientInfo_t;

typedef enum rtk_rg_ppp_auth_type_e
{
	PPP_AUTH_TYPE_PAP=0,
	PPP_AUTH_TYPE_CHAP=1,
	PPP_AUTH_TYPE_END
} rtk_rg_ppp_auth_type_t;

typedef int (*p_dialOnDemondCallBack)(unsigned int);
typedef int (*p_idleTimeOutCallBack)(unsigned int);

typedef enum rtk_rg_ppp_status_e
{
	PPP_STATUS_DISCONNECT=0,
	PPP_STATUS_CONNECT=1,
	PPP_STATUS_END
} rtk_rg_ppp_status_t;

typedef struct rtk_rg_pppoeClientInfoBeforeDial_s
{
	char username[32];
	char password[32];
	rtk_rg_ppp_auth_type_t auth_type;
	int pppoe_proxy_enable;
	int max_pppoe_proxy_num;
	int auto_reconnect;
	int dial_on_demond;
	int idle_timeout_secs;
	rtk_rg_ppp_status_t stauts; //connect/disconnect?
	p_dialOnDemondCallBack dialOnDemondCallBack;
	p_idleTimeOutCallBack idleTimeOutCallBack;
} rtk_rg_pppoeClientInfoBeforeDial_t;

typedef struct rtk_rg_pppoeClientInfoAfterDial_s
{
	rtk_rg_ipStaticInfo_t hw_info;
	unsigned short int sessionId;
	unsigned short int gpon_pppoe_status;
} rtk_rg_pppoeClientInfoAfterDial_t;

typedef struct rtk_rg_pppoeDsliteInfoAfterDial_s
{
	rtk_rg_ipDslitStaticInfo_t dslite_hw_info;
	unsigned short int sessionId;
} rtk_rg_pppoeDsliteInfoAfterDial_t;

typedef struct rtk_rg_ipPppoeClientInfo_s
{
	rtk_rg_pppoeClientInfoBeforeDial_t before_dial;
	rtk_rg_pppoeClientInfoAfterDial_t after_dial;
} rtk_rg_ipPppoeClientInfo_t;

typedef struct rtk_rg_pptpClientInfoBeforeDial_s
{
	char username[32];
	char password[32];
	ipaddr_t pptp_ipv4_addr;
} rtk_rg_pptpClientInfoBeforeDial_t;

typedef struct rtk_rg_pptpClientInfoAfterDial_s
{
	rtk_rg_ipStaticInfo_t hw_info;

	unsigned short callId;
	unsigned short gateway_callId;
} rtk_rg_pptpClientInfoAfterDial_t;

typedef struct rtk_rg_ipPPTPClientInfo_s
{
	rtk_rg_pptpClientInfoBeforeDial_t before_dial;
	rtk_rg_pptpClientInfoAfterDial_t after_dial;

	unsigned int gre_header_sequence;
	unsigned int gre_header_acknowledgment;
	unsigned short ipv4_header_identifier;
	unsigned int sw_gre_header_sequence;	//from protocol stack output sequence
	unsigned int sw_gre_header_server_sequence;	//from server input sequence
	unsigned int sw_gre_header_server_sequence_started:1;
} rtk_rg_ipPPTPClientInfo_t;

typedef struct rtk_rg_l2tpClientInfoBeforeDial_s
{
	char username[32];
	char password[32];
	ipaddr_t l2tp_ipv4_addr;
} rtk_rg_l2tpClientInfoBeforeDial_t;

typedef struct rtk_rg_l2tpClientInfoAfterDial_s
{
	rtk_rg_ipStaticInfo_t hw_info;

	unsigned short outer_port;				//used in outer UDP encapsulation
	unsigned short gateway_outer_port;		//used in outer UDP encapsulation
	unsigned short tunnelId;
	unsigned short gateway_tunnelId;
	unsigned short sessionId;
	unsigned short gateway_sessionId;
} rtk_rg_l2tpClientInfoAfterDial_t;

typedef struct rtk_rg_ipL2TPClientInfo_s
{
	rtk_rg_l2tpClientInfoBeforeDial_t before_dial;
	rtk_rg_l2tpClientInfoAfterDial_t after_dial;

	unsigned short ipv4_header_identifier;
} rtk_rg_ipL2TPClientInfo_t;

typedef enum rtk_rg_vxlan_mode_e
{
	VXLAN_MODE_L2=0,
	VXLAN_MODE_L3=1
} rtk_rg_vxlan_mode_t;

typedef struct rtk_rg_vxlanClientInfoAfterDial_s
{
	rtk_rg_ipStaticInfo_t hw_info;

	rtk_rg_vxlan_mode_t vxlan_mode;
	ipaddr_t vxlan_remote_ipv4_addr;			//IPv4 addr of remote VTEP 
	rtk_mac_t vxlan_remote_ipv4_gatewayMac; 	//gateway mac of remote VTEP for IPv4 addr
	unsigned short outer_udp_sport;				//used in outer UDP encapsulation
	unsigned short outer_udp_dport;				//used in outer UDP encapsulation
	unsigned int vxlan_network_identifier; 		//VNI value
	unsigned int vxlan_baseIntf_idx;			//used for outer source MAC, source IP address, VLAN value
} rtk_rg_vxlanClientInfoAfterDial_t;

typedef struct rtk_rg_vxlanClientInfo_s
{
	rtk_rg_vxlanClientInfoAfterDial_t after_dial;

	int vxlan_remote_ipv4_gatewayMac_l2Idx;
	unsigned short ipv4_header_identifier;
	short vxlan_accelerate_upstreamL2Idx;
	short vxlan_accelerate_extra_upstreamL2Idx;
	short vxlan_accelerate_downstreamL2Idx;
	short vxlan_accelerate_extra_downstreamL2Idx;
} rtk_rg_vxlanClientInfo_t;

typedef struct rtk_rg_ipPppoeDsliteInfo_s
{
	rtk_rg_pppoeClientInfoBeforeDial_t before_dial;
	rtk_rg_pppoeDsliteInfoAfterDial_t after_dial;
} rtk_rg_ipPppoeDsliteInfo_t;
#if defined(CONFIG_RG_WAN_MSS_CACHE)
typedef struct rtk_rg_netif_mssCacheEntry_s
{
	union{
		ipaddr_t dest;
		unsigned char v6dest[IPV6_ADDR_LEN];
	};
	unsigned short advmss;

	struct list_head msscache_list;
}rtk_rg_netif_mssCacheEntry_t;

typedef struct rtk_rg_netif_mssCache_s
{
	struct list_head hash[MAX_NETIF_MSSCACHE_HASH];
	struct list_head free;
	rtk_rg_netif_mssCacheEntry_t pool[MAX_NETIF_MSSCACHE_POOL];
}rtk_rg_netif_mssCache_t;
#endif
typedef struct rtk_rg_wanIntfInfo_s
{
	rtk_rg_wanIntfConf_t wan_intf_conf;
	union{
		rtk_rg_ipStaticInfo_t static_info;
		rtk_rg_ipDhcpClientInfo_t	dhcp_client_info;
		rtk_rg_ipPppoeClientInfo_t pppoe_info;
		rtk_rg_ipPPTPClientInfo_t pptp_info;
		rtk_rg_ipL2TPClientInfo_t l2tp_info;
		rtk_rg_ipDslitStaticInfo_t dslite_info;
		rtk_rg_ipPppoeDsliteInfo_t pppoe_dslite_info;
		rtk_rg_vxlanClientInfo_t vxlan_info;
	};
	int bind_wan_type_ipv4;
	int bind_wan_type_ipv6;
	rtk_rg_wirelessWan_t wirelessWan;			//0:wire WAN, 13:wireless WAN(wlan0-vxd), 27:wireless WAN(wlan1-vxd)
	int nexthop_ipv4;
	int nexthop_ipv6;
	int pppoe_idx;	//idx for pppoe table
	int extip_idx;	//idx for ext ip table
	int baseIntf_idx;	//for pptp/l2tp
#if defined(CONFIG_RG_RTL9602C_SERIES)
	int dslite_idx;	//for dslite/pppoe_dslite
#endif
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	// for dual header
	int flowEntryIdx;				// path6 flow entry index, initialized as -1
	int16 extraTagActionListIdx;	// action list index, starts from 1 and initialized as 0
#endif
} rtk_rg_wanIntfInfo_t;

typedef struct rtk_rg_intfInfo_s
{
	char intf_name[32];
	int is_wan;
	union
	{
		rtk_rg_lanIntfConf_t lan_intf;
		rtk_rg_wanIntfInfo_t wan_intf;
	};
} rtk_rg_intfInfo_t;

//IPv6 server in Lan: upnp, virtual server, dmz
typedef enum rtk_rg_ipv6_connLookup_hitIf_s
{
	RTK_RG_IPV6_LOOKUP_NONE_HIT=0,
	RTK_RG_IPV6_LOOKUP_UPNP_HIT,
	RTK_RG_IPV6_LOOKUP_VIRTUALSERVER_HIT,
	RTK_RG_IPV6_LOOKUP_DMZ_HIT,

}rtk_rg_ipv6_connLookup_hitIf_t;

typedef struct rtk_rg_ipv6_connLookup_s
{
	rtk_rg_ipv6_connLookup_hitIf_t serverInLanHit;
	int hitIndex;	//valid while serverInLanHit is 1~3, represent the hit upnp or virtual_server or dmz rule index.
	rtk_ipv6_addr_t transIP;
	int16 transPort;
}rtk_rg_ipv6_connLookup_t;


//VLAN function
typedef struct rtk_rg_cvlan_info_s
{
	int vlanId;
	unsigned int isIVL:1;		//0: SVL, 1:IVL
	rtk_rg_portmask_t memberPortMask;
	rtk_rg_mac_portmask_t untagPortMask;
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	unsigned int wlan0DevMask;
	unsigned int wlan0UntagMask;
#endif
	rtk_rg_enable_t vlan_based_pri_enable;
	int vlan_based_pri;
	int addedAsCustomerVLAN;		//used to check this vlan created by cvlan apis or not
}rtk_rg_cvlan_info_t;

//VLAN Binding
typedef struct rtk_rg_vlanBinding_s
{
	rtk_rg_port_idx_t port_idx;
	//rtk_portmask_t port_mask;
	//rtk_portmask_t ext_port_mask;
	int ingress_vid;
	int wan_intf_idx;
}rtk_rg_vlanBinding_t;

//DMZ
typedef struct rtk_rg_dmzInfo_s
{
	unsigned int enabled:1;
	unsigned int mac_mapping_enabled:1;
	rtk_rg_ip_version_t ipversion; //0:v4_only 1:v6_only 2:both_v4_v6
	union
	{
		ipaddr_t private_ip; //only used when mac_mapping_enabled=0 && (ipversion==0 || ipversion==2)
		rtk_mac_t mac; //only used when mac_mapping_enabled=1
	};
	rtk_ipv6_addr_t	private_ipv6;//only used when mac_mapping_enabled=0 && (ipversion==1 || ipversion==2)

} rtk_rg_dmzInfo_t;

typedef enum rtk_rg_ipv4MulticastFlowMode_e
{
	RTK_RG_IPV4MC_DONT_CARE_SRC=0,
	RTK_RG_IPV4MC_INCLUDE,
	RTK_RG_IPV4MC_EXCLUDE,
} rtk_rg_ipv4MulticastFlowMode_t;

typedef enum rtk_rg_ipv4MulticastFlowRouting_e
{
	RTK_RG_IPV4MC_EN_ROUTING=0,
	RTK_RG_IPV4MC_DIS_ROUTING,
} rtk_rg_ipv4MulticastFlowRouting_t;


//Multicast flow
typedef struct rtk_rg_multicastFlow_s
{
	ipaddr_t 	multicast_ipv4_addr;
	ipaddr_t	multicast_ipv6_addr[4];
	unsigned int isIPv6:1;
	rtk_rg_ipv4MulticastFlowMode_t srcFilterMode;		//for 9602C/9600series
	rtk_rg_ipv4MulticastFlowRouting_t routingMode;
	rtk_rg_portmask_t port_mask;
	ipaddr_t includeOrExcludeIp; 		//v4 only for 9602C/9600series is includeOrExclude Ip for flowbase is include ip
	ipaddr_t includeOrExclude_v6Ip[4];  //v6 only for 9602C/9600series is includeOrExclude Ip for flowbase is include ip
	unsigned int isIVL:1;
	unsigned short vlanID;	//used for IVL only 		//for 9607Series is internalVid
} rtk_rg_multicastFlow_t;


typedef struct rtk_rg_l2MulticastFlow_s
{
	rtk_mac_t mac;
	rtk_rg_portmask_t port_mask;
	unsigned int isIVL:1;
	unsigned short vlanID;	//used for IVL only
} rtk_rg_l2MulticastFlow_t;

typedef struct rtk_rg_ipv4MulticastFlow_s
{
	ipaddr_t groupIp;
	rtk_rg_ipv4MulticastFlowMode_t srcFilterMode;
	rtk_rg_ipv4MulticastFlowRouting_t routingMode;
	ipaddr_t includeOrExcludeIp;
	rtk_rg_portmask_t ipm_portmask;

} rtk_rg_ipv4MulticastFlow_t;

#if defined(CONFIG_RG_RTL9602C_SERIES)
typedef struct rtk_rg_ipv6MulticastFlow_s
{
    rtk_ipv6_addr_t groupIp6;
	rtk_rg_portmask_t portMaskIPMC6;

} rtk_rg_ipv6MulticastFlow_t;

#endif


typedef struct rtk_rg_igmpMldSnoopingPortControl_s
{
	uint32 enableIGMPSnooping:1;
	uint32 enableMLDSnooping:1;
	uint32 enablefastLeave:1;
	uint16 igmp_PortMaxGroupSize;
	uint16 mld_PortMaxGroupSize;
}rtk_rg_igmpMldSnoopingPortControl_t;

typedef struct rtk_rg_igmpMldSnoopingPortControl_Conf_s
{
	uint32 enablePortConf:1;
	rtk_rg_igmpMldSnoopingPortControl_t conf;

}rtk_rg_igmpMldSnoopingPortControl_Conf_t;



typedef struct rtk_rg_igmpMldSnoopingControl_s
{
    //IGMP
    uint32 disableIgmpV1Snooping:1;
	uint32 disableIgmpV2Snooping:1;
	uint32 disableIgmpV3Snooping:1;
	uint32 enableIgmpV3SourceFilter:1;
	//uint32 enableIgmpSnoopingWithProxyReporting:1;
    uint32 enableFastLeave:1;
    uint32 trapIgmpToPS:1;

    //MLD
	uint32 disableMldV1Snooping:1;
	uint32 disableMldV2Snooping:1;
    uint32 trapMldToPS:1;

    //Custom TableSize/Timer/FilterPortmask
    uint32 customSysMaxSimultaneousGroupSize:1;
    uint32 customLastMbrQueryInterval:1;
    uint32 customGroupMemberAgingTime:1;
    uint32 customSysTimerSec:1;
    uint32 customGenQuerySec:1;
    uint32 customQueryEgressFilterPmsk:1;
    uint32 customReportEgressFilterPmsk:1;
	uint32 customLeaveEgressFilterPmsk:1;
	uint32 customReportIngressFilterPmsk:1;


	uint16 sysMaxSimultaneousGroupSize; // igmp_max_simultaneous_group_size
	uint16 lastMbrQueryInterval;
	uint16 groupMemberAgingTime;
	uint16 sysTimerSec;
	uint16 genQuerySec;
	uint16 queryEgressFilterPmsk;
	uint16 reportEgressFilterPmsk;
	uint16 leaveEgressFilterPmsk;
	uint16 reportIngressFilterPmsk;

}rtk_rg_igmpMldSnoopingControl_t;

/* IGMP version */
typedef enum {
	IGMP_V1=0,
	IGMP_V2,	//1
	IGMP_V3,	//2
	MLD_V1,		//3
	MLD_V2,		//4
	IGMP_MLD_VER_MAX//5
} IGMP_MLD_VER_t;

typedef enum rtk_rg_multicastVersionSupport_e
{
	RG_MC_IGMPV1=1<<IGMP_V1,
	RG_MC_IGMPV2=1<<IGMP_V2,
	RG_MC_IGMPV3=1<<IGMP_V3,
	RG_MC_MLDV1 =1<<MLD_V1,
	RG_MC_MLDV2 =1<<MLD_V2,
	RG_MC_VERSION_SUPPORT_END=1<<IGMP_MLD_VER_MAX,
}rtk_rg_multicastVersionSupport_t;



typedef enum rtk_rg_multicastProtocol_e
{
	RG_MC_BOTH_IGMP_MLD=0,
	RG_MC_IGMP_ONLY,
	RG_MC_MLD_ONLY,
	RG_MC_PROTOCOL_END,
}rtk_rg_multicastProtocol_t;

//ALG
typedef enum rtk_rg_alg_function_sequence_e
{
	//Server in WAN
	RTK_RG_ALG_SIP_TCP=0,
	RTK_RG_ALG_SIP_UDP,
	RTK_RG_ALG_H323_TCP,
	RTK_RG_ALG_H323_UDP,
	RTK_RG_ALG_RTSP_TCP,
	RTK_RG_ALG_RTSP_UDP,
	RTK_RG_ALG_FTP_TCP,
	RTK_RG_ALG_FTP_UDP,			//7

	//Server in LAN
	RTK_RG_ALG_SIP_TCP_SRV_IN_LAN,
	RTK_RG_ALG_SIP_UDP_SRV_IN_LAN,
	RTK_RG_ALG_H323_TCP_SRV_IN_LAN,
	RTK_RG_ALG_H323_UDP_SRV_IN_LAN,
	RTK_RG_ALG_RTSP_TCP_SRV_IN_LAN,
	RTK_RG_ALG_RTSP_UDP_SRV_IN_LAN,
	RTK_RG_ALG_FTP_TCP_SRV_IN_LAN,
	RTK_RG_ALG_FTP_UDP_SRV_IN_LAN,		//15

	//Pass through
	RTK_RG_ALG_PPTP_TCP_PASSTHROUGH,
	RTK_RG_ALG_PPTP_UDP_PASSTHROUGH,
	RTK_RG_ALG_L2TP_TCP_PASSTHROUGH,
	RTK_RG_ALG_L2TP_UDP_PASSTHROUGH,
	RTK_RG_ALG_IPSEC_TCP_PASSTHROUGH,
	RTK_RG_ALG_IPSEC_UDP_PASSTHROUGH,		//21

	RTK_RG_ALG_PPPOE_PASSTHROUGH,
#ifdef CONFIG_RG_ROMEDRIVER_ALG_BATTLENET_SUPPORT
	RTK_RG_ALG_BATTLENET_TCP,
#endif
	RTK_RG_ALG_TFTP_UDP,
	RTK_RG_ALG_FCC_UDP,					//25
	RTK_RG_ALG_FUNCTION_END,
}rtk_rg_alg_function_sequence_t;

typedef enum rtk_rg_alg_type_e
{
	//Server in WAN
	RTK_RG_ALG_SIP_TCP_BIT					=0x1,
	RTK_RG_ALG_SIP_UDP_BIT					=0x2,
	RTK_RG_ALG_H323_TCP_BIT					=0x4,
	RTK_RG_ALG_H323_UDP_BIT					=0x8,
	RTK_RG_ALG_RTSP_TCP_BIT					=0x10,
	RTK_RG_ALG_RTSP_UDP_BIT					=0x20,
	RTK_RG_ALG_FTP_TCP_BIT					=0x40,
	RTK_RG_ALG_FTP_UDP_BIT					=0x80,			//8

	//Server in LAN
	RTK_RG_ALG_SIP_TCP_SRV_IN_LAN_BIT		=0x100,
	RTK_RG_ALG_SIP_UDP_SRV_IN_LAN_BIT		=0x200,
	RTK_RG_ALG_H323_TCP_SRV_IN_LAN_BIT		=0x400,
	RTK_RG_ALG_H323_UDP_SRV_IN_LAN_BIT		=0x800,
	RTK_RG_ALG_RTSP_TCP_SRV_IN_LAN_BIT		=0x1000,
	RTK_RG_ALG_RTSP_UDP_SRV_IN_LAN_BIT		=0x2000,
	RTK_RG_ALG_FTP_TCP_SRV_IN_LAN_BIT		=0x4000,
	RTK_RG_ALG_FTP_UDP_SRV_IN_LAN_BIT		=0x8000,		//16

	//Pass through
	RTK_RG_ALG_PPTP_TCP_PASSTHROUGH_BIT		=0x10000,
	RTK_RG_ALG_PPTP_UDP_PASSTHROUGH_BIT		=0x20000,
	RTK_RG_ALG_L2TP_TCP_PASSTHROUGH_BIT		=0x40000,
	RTK_RG_ALG_L2TP_UDP_PASSTHROUGH_BIT		=0x80000,
	RTK_RG_ALG_IPSEC_TCP_PASSTHROUGH_BIT	=0x100000,
	RTK_RG_ALG_IPSEC_UDP_PASSTHROUGH_BIT	=0x200000,		//22

	RTK_RG_ALG_PPPOE_PASSTHROUGH_BIT		=0x400000,

	//BattleNet
#ifdef CONFIG_RG_ROMEDRIVER_ALG_BATTLENET_SUPPORT
	RTK_RG_ALG_BATTLENET_TCP_BIT 			=0x800000,
#endif
	//TFTP
	RTK_RG_ALG_TFTP_UDP_BIT					=0x1000000,
	//FCC
	RTK_RG_ALG_FCC_UDP_BIT					=0x2000000,
	RTK_RG_ALG_TYPE_END						=RTK_RG_ALG_FCC_UDP_BIT<<1,
}rtk_rg_alg_type_t;

typedef enum rtk_rg_alg_tcpKnownPort_e
{
	RTK_RG_ALG_SIP_TCP_PORT=5060,
	RTK_RG_ALG_H323_TCP_PORT=1720,
	RTK_RG_ALG_RTSP_TCP_PORT=554,
	RTK_RG_ALG_PPTP_PASSTHROUGH_TCP_PORT=1723,
	RTK_RG_ALG_L2TP_PASSTHROUGH_TCP_PORT=1701,
	RTK_RG_ALG_IPSEC_PASSTHROUGH_TCP_PORT=500,		//used by ISAKMP
	RTK_RG_ALG_FTP_TCP_PORT=21,
	RTK_RG_ALG_FTP_DATA_TCP_PORT=20,
#ifdef CONFIG_RG_ROMEDRIVER_ALG_BATTLENET_SUPPORT
	RTK_RG_ALG_BATTLENET_TCP_PORT=6112,
#endif
}rtk_rg_alg_tcpKnownPort_t;

typedef enum rtk_rg_alg_udpKnownPort_e
{
	RTK_RG_ALG_SIP_UDP_PORT=5060,
	RTK_RG_ALG_H323_UDP_PORT=1720,
	RTK_RG_ALG_RTSP_UDP_PORT=554,
	RTK_RG_ALG_PPTP_PASSTHROUGH_UDP_PORT=1723,
	RTK_RG_ALG_L2TP_PASSTHROUGH_UDP_PORT=1701,
	RTK_RG_ALG_IPSEC_PASSTHROUGH_UDP_PORT=500,		//used by ISAKMP
	RTK_RG_ALG_FTP_UDP_PORT=21,
	RTK_RG_ALG_FTP_DATA_UDP_PORT=20,
	RTK_RG_ALG_TFTP_UDP_PORT=69,
	RTK_RG_ALG_FCC_UDP_PORT=15970,
}rtk_rg_alg_udpKnownPort_t;

typedef int (*p_algRegisterFunction)(int,int,unsigned char*,unsigned char*);	//the third is sk_buff* ,the fourth is type rtk_rg_pktHdr_t*

typedef struct rtk_rg_alg_funcMapping_s
{
	unsigned short int portNum;
	p_algRegisterFunction registerFunction;
	uint8 keepExtPort;
}rtk_rg_alg_funcMapping_t;

typedef struct rtk_rg_alg_serverIpMapping_s
{
	rtk_rg_alg_type_t algType;
	ipaddr_t serverAddress;
}rtk_rg_alg_serverIpMapping_t;

typedef struct rtk_rg_alg_dynamicPort_s
{
	p_algRegisterFunction algFun;
	unsigned short int portNum;
	int timeout;
	int isTCP;
	int serverInLan;	//0:Server In Wan, 1:Server In Lan
	ipaddr_t intIP;		//for serverInLan's internal server ip

	struct list_head alg_list;
}rtk_rg_alg_dynamicPort_t;

typedef struct rtk_rg_alg_preserveExtPort_s
{
	uint32 internalIp;
	uint16 internalPort;
	uint16 portNum;
	int timeout;
	int isTCP;

	struct list_head alg_list;
}rtk_rg_alg_preserveExtPort_t;

#ifdef __KERNEL__
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 30)
//copy from fs/proc/internal.h
struct proc_dir_entry {
	unsigned int low_ino;
	umode_t mode;
	nlink_t nlink;
	kuid_t uid;
	kgid_t gid;
	loff_t size;
	const struct inode_operations *proc_iops;
	const struct file_operations *proc_fops;
	struct proc_dir_entry *next, *parent, *subdir;
	void *data;
	atomic_t count;		/* use count */
	atomic_t in_use;	/* number of callers into module in progress; */
			/* negative -> it's going away RSN */
	struct completion *pde_unload_completion;
	struct list_head pde_openers;	/* who did ->open, but not ->release */
	spinlock_t pde_unload_lock; /* proc_fops checks and pde_users bumps */
	u8 namelen;
	char name[];
};
#endif
typedef struct rtk_rg_proc_s
{
	char *name;
	int (*get) (struct seq_file *s, void *v);
	int (*set) ( struct file *, const char *,unsigned long, void *);
	unsigned int inode_id[RTK_RG_MAX_PROC_GROUP];
	unsigned int unlockBefortWrite;
	struct file_operations proc_fops;
	rtk_rg_proc_group_bitmask_t group_bitmask;
}rtk_rg_proc_t;
#endif

/* FTP */
//LUKE20130816: move to separate ALG file
#if 0
//entry for each FTP session
typedef struct rtk_rg_ftpCtrlFlowEntry_s {
	unsigned int remoteIpAddr;		//remote host ip
	unsigned int internalIpAddr;			//NPI internal ip

	unsigned short int remotePort;	//remote port
	unsigned short int internalPort;		//internal port

	//if outbound enlarge packet, Delta is positive value;
	//if outbound shrink packet, Delta is negative value
	int Delta;								//used to sync acknowledgement

	struct rtk_rg_ftpCtrlFlowEntry_s *pNext,*pPrev;
}rtk_rg_ftpCtrlFlowEntry_t;
#endif

/* DHCP */
//bootp message type
typedef struct rtk_rg_dhcpMsgType_s
{
	uint8 op;
	uint8 htype;
	uint8 hlen;
	uint8 hops;
	uint32 xid;
	uint16 secs;
	uint16 flags;
	uint32 ciaddr;
	uint32 yiaddr;
	uint32 siaddr;
	uint32 giaddr;
	uint8 chaddr[16];
	uint8 sname[64];
	uint8 file[128];
	uint32 cookie;
	//uint8 options[COOKIE_LEN]; /* 312 - cookie */
}rtk_rg_dhcpMsgType_t;


/* PPTP */
//PPTP message type
typedef enum rtk_rg_pptpCtrlMsgType_e
{
	PPTP_StartCtrlConnRequest 	= 1,
	PPTP_StartCtrlConnReply 	= 2,
	PPTP_StopCtrlConnRequest 	= 3,
	PPTP_StopCtrlConnReply 		= 4,
	PPTP_EchoRequest 			= 5,
	PPTP_EchoReply 				= 6,
	PPTP_OutCallRequest 		= 7,
	PPTP_OutCallReply 			= 8,
	PPTP_InCallRequest 			= 9,
	PPTP_InCallReply 			= 10,
	PPTP_InCallConn 			= 11,
	PPTP_CallClearRequest 		= 12,
	PPTP_CallDiscNotify 		= 13,
	PPTP_WanErrorNotify 		= 14,
	PPTP_SetLinkInfo 			= 15
}rtk_rg_pptpCtrlMsgType_t;

//Message structures
typedef struct rtk_rg_pptpMsgHead_s
{
	unsigned short int    length;			/* total length */
	unsigned short int    msgType;			/* PPTP message type */
	unsigned int      	  magic;			/* magic cookie */
	unsigned short int    type;				/* control message type */
	unsigned short int    resv0;			/* reserved */
}rtk_rg_pptpMsgHead_t;

typedef struct rtk_rg_pptpCallIds_s
{
	unsigned short int    cid1;				/* Call ID field #1 */
	unsigned short int    cid2;				/* Call ID field #2 */
}rtk_rg_pptpCallIds_t;

typedef struct rtk_rg_pptpCodes_s
{
	unsigned char     resCode;				/* Result Code */
	unsigned char     errCode;				/* Error Code */
}rtk_rg_pptpCodes_t;

//20130821LUKE:close it because implemented by separate module
#if 0
//GRE entry for each PPTP session
typedef struct rtk_rg_pptpGreEntry_s
{
	unsigned int remoteIpAddr;
	rtk_mac_t remoteMacAddr;
	unsigned int internalIpAddr;
	rtk_mac_t internalMacAddr;

	unsigned short int remoteCallID;
	unsigned short int externalCallID;
	unsigned short int internalCallID;

	unsigned char valid;
}rtk_rg_pptpGreEntry_t;

typedef struct rtk_rg_pptpGreLinkList_s
{
	rtk_rg_pptpGreEntry_t greEntry;
	struct rtk_rg_pptpGreLinkList_s *pPrev, *pNext;
}rtk_rg_pptpGreLinkList_t;
#endif

/* L2TP */
typedef struct rtk_rg_alg_l2tp_ctrlHeader_s
{
	uint16 flag;		//must be 0xc802
	uint16 length;
	uint16 tunnel_id;
	uint16 session_id;
	uint16 numberSent;
	uint16 numberReceived;
}rtk_rg_alg_l2tp_ctrlHeader_t;

typedef struct rtk_rg_alg_l2tp_avpHeader_s
{
	uint8 MH;
	uint8 length;
	uint16 vendor_id;	//normal are zero
	uint16 attType;
}rtk_rg_alg_l2tp_avpHeader_t;

typedef struct rtk_rg_alg_l2tp_flow_s
{
	uint8 valid;
	ipaddr_t internalIP;
	ipaddr_t remoteIP;
	uint16 IntTulID;	//internal tunnel ID
	uint16 ExtTulID;	//external tunnel ID
}rtk_rg_alg_l2tp_flow_t;

typedef struct rtk_rg_alg_l2tp_linkList_s
{
	rtk_rg_alg_l2tp_flow_t l2tpFlow;
	struct rtk_rg_alg_l2tp_linkList_s *pPrev, *pNext;
}rtk_rg_alg_l2tp_linkList_t;

//VirtualServer(PortForward)
typedef struct rtk_rg_virtualServer_s
{
	rtk_rg_ip_version_t ipversion; //0:v4_only 1:v6_only 2:both_v4_v6
	unsigned int is_tcp:1;
	int wan_intf_idx; //for gateway ip
	unsigned short int gateway_port_start;
	ipaddr_t local_ip;//only used when (ipversion==0 || ipversion==2)
	ipaddr_t remote_ip;
	rtk_ipv6_addr_t	local_ipv6;//only used when (ipversion==1 || ipversion==2)
	unsigned short int local_port_start;
	unsigned int mappingPortRangeCnt;
	rtk_rg_virtualServerMappingType_t mappingType;
	unsigned int valid:1;
	rtk_rg_alg_type_t hookAlgType;	//only for server-in-lan
	unsigned int disable_wan_check;	//0: turn-on wan interface check, 1: disable wan interface check
	int8 enable_limit_remote_src_port;
	uint16 remote_src_port_start;
	uint16 remote_src_port_end;

} rtk_rg_virtualServer_t;

//port trigger
typedef struct rtk_rg_portTrigger_info_s
{
	unsigned short int match_port_start;
	unsigned short int match_port_end;
	
	rtk_rg_portTriggerNatType_t natType;
	
	unsigned short int relate_port_start;
	unsigned short int relate_port_end;

	unsigned int match_is_tcp:1;
	unsigned int relate_is_tcp:1;
}rtk_rg_portTrigger_info_t;

typedef struct rtk_rg_portTrigger_list_s
{
	union{
		struct{
			unsigned short int match_port_start;		//used for match
			unsigned short int match_port_end;			//used for match
			unsigned int relate_is_tcp;					//used for match
		};
		struct{
			ipaddr_t local_ip;							//used for relate
			unsigned int referenceCount;				//used for relate, if port_range excatly same
		};
	};

	unsigned short int relate_port_start;
	unsigned short int relate_port_end;
	
	

	struct list_head trigger_list;
}rtk_rg_portTrigger_list_t;


/*
Use rtk_rg_list_t to construct linked list by record the element idx.
*/
typedef struct rtk_rg_list_s
{
	uint16 nextIdx, prevIdx;
}rtk_rg_list_t; // if index may large then 1<<16, please use rtk_rg_list_long_t

typedef struct rtk_rg_list_longIdx_s
{
	uint32 nextIdx, prevIdx;
}rtk_rg_list_longIdx_t;

typedef struct rtk_rg_list_head_s
{
	void *first;
}rtk_rg_list_head_t;


//ipset feature - CONFIG_RG_IPSET_VERSION==1
typedef enum rtk_rg_ipset_hash_key_e
{
	RTK_RG_IPSET_HASH_KEY_IP = 0,
	RTK_RG_IPSET_HASH_KEY_L4_PORT,
	RTK_RG_IPSET_HASH_KEY_END,
}rtk_rg_ipset_hash_key_t;

typedef enum rtk_rg_ipset_filter_type_e
{
	RTK_RG_IPSET_FILTER_TYPE_REMOTE_IPV4_IP = 0,
	RTK_RG_IPSET_FILTER_TYPE_REMOTE_L4P_TCP,
	RTK_RG_IPSET_FILTER_TYPE_REMOTE_L4P_UDP,
	RTK_RG_IPSET_FILTER_TYPE_REMOTE_IPV6_IP,
	RTK_RG_IPSET_FILTER_TYPE_END,
}rtk_rg_ipset_filter_type_t;

typedef enum rtk_rg_ipset_action_type_e
{
	RTK_RG_IPSET_ACTION_TYPE_TRAP_TO_PS = 0,
	RTK_RG_IPSET_ACTION_TYPE_END,
}rtk_rg_ipset_action_type_t;

typedef struct rtk_rg_ipset_check_s
{
	uint32 rule_idx;
	rtk_rg_ipset_hash_key_t hash_key;
	rtk_rg_ipset_action_type_t action_type;

	ipaddr_t ipv4_address;
	uint8 ipv6_address[16];
	unsigned short int l4_port;
}rtk_rg_ipset_check_t;

typedef struct rtk_rg_ipset_rule_s
{
	unsigned short int set_idx;
	rtk_rg_ipset_filter_type_t filter_type;
	rtk_rg_ipset_action_type_t action_type;

	ipaddr_t ipv4_address;				//filter_type = IPV4_IP
	uint8 ipv6_address[16];				//filter_type = IPV6_IP
	uint8 ip_suffix_len;				//filter_type = XXX_IP

	unsigned short int l4_port_start;	//filter_type = L4_PORT
	unsigned short int l4_port_end;		//filter_type = L4_PORT

}rtk_rg_ipset_rule_t;

typedef struct rtk_rg_ipset_info_s
{
	uint32 set_idx:1;					//reference from MAX_IPSET_IDX_WIDTH
	rtk_rg_ipset_filter_type_t filter_type:4;
	rtk_rg_ipset_action_type_t action_type:2;
	uint32 ip_suffix_len:8;
	uint32 rsv_bit:(32-1-4-2-8);

	union{
		struct{ 	//IPV4_IP
			ipaddr_t ipv4_address;
		};
		struct{ 	//IPV6_IP
			uint8 ipv6_address[16];
		};
		struct{ 	//L4_PORT
			unsigned short int l4_port_start;
			unsigned short int l4_port_end;
		};
	};
}rtk_rg_ipset_info_t;

typedef struct rtk_rg_ipset_list_s
{
	rtk_rg_ipset_info_t ipsetRule;

	struct list_head ipset_list;
}rtk_rg_ipset_list_t;

//ipset feature - CONFIG_RG_IPSET_VERSION==2
typedef enum rtk_rg_ipsets_operation_e
{
	RTK_RG_IPSETS_OPERA_AND = 0,
	RTK_RG_IPSETS_OPERA_OR,
	RTK_RG_IPSETS_OPERA_END,
}rtk_rg_ipsets_operation_t;

typedef enum rtk_rg_ipsets_target_e
{
	RTK_RG_IPSETS_TARGET_ACCEPT = 0,	//only check upstream
	RTK_RG_IPSETS_TARGET_DROP,
	RTK_RG_IPSETS_TARGET_REGECT,		//to ps
	RTK_RG_IPSETS_TARGET_LEVEL,			//only check downstream
	RTK_RG_IPSETS_TARGET_END,
}rtk_rg_ipsets_target_t;

typedef enum rtk_rg_ipsets_set_proto_e
{
	RTK_RG_IPSETS_SET_PROTO_IPV4 = 0,
	RTK_RG_IPSETS_SET_PROTO_IPV6,
	RTK_RG_IPSETS_SET_PROTO_END,
}rtk_rg_ipsets_set_proto_t;

typedef enum rtk_rg_ipsets_set_dir_e
{
	RTK_RG_IPSETS_SET_DIR_SRC = 0,
	RTK_RG_IPSETS_SET_DIR_DEST,
	RTK_RG_IPSETS_SET_DIR_END,
}rtk_rg_ipsets_set_dir_t;

typedef enum rtk_rg_ipsets_set_type_e
{
	RTK_RG_IPSETS_SET_TYPE_MAC = 0,
	RTK_RG_IPSETS_SET_TYPE_IP,
	RTK_RG_IPSETS_SET_TYPE_IP_SUBNET,
	RTK_RG_IPSETS_SET_TYPE_IP_L4PORT,
	RTK_RG_IPSETS_SET_TYPE_L4PORT_RANGE,
	RTK_RG_IPSETS_SET_TYPE_END,
}rtk_rg_ipsets_set_type_t;

typedef enum rtk_rg_ipsets_rule_proto_e
{
	RTK_RG_IPSETS_RULE_PROTO_ALL = 0,
	RTK_RG_IPSETS_RULE_PROTO_TCP,
	RTK_RG_IPSETS_RULE_PROTO_UDP,
	RTK_RG_IPSETS_RULE_PROTO_END,
}rtk_rg_ipsets_rule_proto_t;

typedef struct rtk_rg_ipsets_group_cfg_s
{
	rtk_rg_ipsets_operation_t operation:4;
	rtk_rg_ipsets_target_t target:4;
	uint8 level:3;						//target is level for flow priority
	uint8 egr_wlan_dev_idx:4;			//target is level for egress wlan dev index check
	uint8 netifIdx;						//target is accept for policy route
	uint8 priority;
}rtk_rg_ipsets_group_cfg_t;

typedef struct rtk_rg_ipsets_set_cfg_s
{
	uint8 group_idx;
	rtk_rg_ipsets_set_proto_t protocol:4;
	rtk_rg_ipsets_set_dir_t direction:4;
	rtk_rg_ipsets_set_type_t set_type:4;
	uint8 force_check:1;
	uint8 valid:1;
}rtk_rg_ipsets_set_cfg_t;

typedef struct rtk_rg_ipsets_rule_s
{
	unsigned short int set_idx;

	rtk_mac_t mac;						//sets_type = MAC

	ipaddr_t ipv4_address;				//protocol=0, sets_type = IP prefix
	uint8 ipv6_address[16];				//protocol=1, sets_type = IP prefix
	uint8 ip_suffix_len;				//sets_type = IP_SUBNET

	unsigned short int l4_port;			//single port usage, sets_type = IP_L4PORT
	unsigned short int l4_port_start;	//sets_type = L4PORT_RANGE
	unsigned short int l4_port_end;		//sets_type = L4PORT_RANGE
	rtk_rg_ipsets_rule_proto_t l4_proto;	//sets_type = L4PORT_RANGE
}rtk_rg_ipsets_rule_t;

typedef struct rtk_rg_ipsets_info_s
{
	uint32 ip_suffix_len:8;		//for ip + subnet case
	uint32 l4_port:16;			//for ip + port case
	uint32 rsv_bit:(32-8-16);

	union{
		struct{ 	//L2_MAC
			rtk_mac_t mac;
		};
		struct{ 	//IPV4_IP
			ipaddr_t ipv4_address;
		};
		struct{ 	//IPV6_IP
			uint8 ipv6_address[16];
		};
		struct{ 	//L4_PORT
			unsigned short int l4_port_start;
			unsigned short int l4_port_end;
			uint8 l4_proto;
		};
	};
}rtk_rg_ipsets_info_t;

typedef struct rtk_rg_ipsets_list_s
{
	rtk_rg_ipsets_info_t ipsetsRule;

	rtk_rg_list_t ipsets_idxList;
}rtk_rg_ipsets_list_t;

typedef struct rtk_rg_ipsets_group_list_s
{
	rtk_rg_ipsets_set_cfg_t ipsetsCfg;

	rtk_rg_list_t ipsets_group_idxList;
}rtk_rg_ipsets_group_list_t;


//GPON downstream BC remarking

typedef enum rtk_rg_gpon_ds_bc_filter_fields_e
{
	GPON_DS_BC_FILTER_INGRESS_STREAMID_BIT=(1<<0),
	GPON_DS_BC_FILTER_INGRESS_STAGIf_BIT=(1<<1),
	GPON_DS_BC_FILTER_INGRESS_CTAGIf_BIT=(1<<2),
	GPON_DS_BC_FILTER_INGRESS_SVID_BIT=(1<<3),
	GPON_DS_BC_FILTER_INGRESS_CVID_BIT=(1<<4),
	GPON_DS_BC_FILTER_EGRESS_PORT_BIT=(1<<5),
}rtk_rg_gpon_ds_bc_filter_fields_t;

typedef enum rtk_rg_gpon_ds_bc_action_fields_e
{
	GPON_DS_BC_FILTER_ONLY_CTAG_ACTION=0,
	GPON_DS_BC_FILTER_ONLY_STAG_ACTION=1,
	GPON_DS_BC_FILTER_CTAG_AND_STAG_ACTION=2,
	GPON_DS_BC_FILTER_ACTION_END,
}rtk_rg_gpon_ds_bc_action_fields_t;


typedef enum rtk_rg_gpon_ds_bc_tag_decision_e
{
	RTK_RG_GPON_BC_FORCE_UNATG=0,
	RTK_RG_GPON_BC_FORCE_TAGGIN_WITH_CVID,
	RTK_RG_GPON_BC_FORCE_TAGGIN_WITH_CVID_CPRI,
	RTK_RG_GPON_BC_FORCE_TRANSPARENT,
	RTK_RG_GPON_BC_FORCE_END,
}rtk_rg_gpon_ds_bc_tag_decision_t;

typedef enum rtk_rg_gpon_ds_bc_stag_decision_e
{
	RTK_RG_GPON_BC_FORCE_STAG_UNATG=0,
	RTK_RG_GPON_BC_FORCE_STAG_TAGGIN_WITH_SVID_TPID=1,
	RTK_RG_GPON_BC_FORCE_STAG_TAGGIN_WITH_SVID_CPRI_TPID=2,

	RTK_RG_GPON_BC_FORCE_STAG_TAGGIN_WITH_SVID_TPID2=3,
	RTK_RG_GPON_BC_FORCE_STAG_TAGGIN_WITH_SVID_CPRI_TPID2=4,

	RTK_RG_GPON_BC_FORCE_STAG_TAGGIN_WITH_SVID_ORITPID=5,
	RTK_RG_GPON_BC_FORCE_STAG_TAGGIN_WITH_SVID_CPRI_ORITPID=6,

	RTK_RG_GPON_BC_FORCE_STAG_TRANSPARENT=7,
	RTK_RG_GPON_BC_FORCE_STAG_END,
}rtk_rg_gpon_ds_bc_stag_decision_t;


typedef struct rtk_rg_gpon_ds_bc_action_s
{
	rtk_rg_gpon_ds_bc_tag_decision_t ctag_decision;
	uint16 assigned_ctag_cvid;
	uint16 assigned_ctag_cpri;//valid only when ctag_decision is RTK_RG_GPON_BC_FORCE_TAGGIN_WITH_CVID_CPRI
}rtk_rg_gpon_ds_bc_action_t;

typedef struct rtk_rg_gpon_ds_bc_stag_action_s
{
	rtk_rg_gpon_ds_bc_stag_decision_t stag_decision;
	uint16 assigned_stag_svid;
	uint16 assigned_stag_spri;//valid only when ctag_decision is RTK_RG_GPON_BC_FORCE_TAGGIN_WITH_SVID_CPRI_TPID/RTK_RG_GPON_BC_FORCE_TAGGIN_WITH_SVID_CPRI_TPID1/RTK_RG_GPON_BC_FORCE_TAGGIN_WITH_SVID_CPRI_ORITPID
}rtk_rg_gpon_ds_bc_stag_action_t;


typedef struct rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_s
{
	//care patterns
	unsigned int filter_fields; // please refer to rtk_rg_gpon_ds_bc_filter_fields_t typedef.

	//assign value for care patterns
	uint16 ingress_stream_id;
	uint8 ingress_stagIf;
	uint8 ingress_ctagIf;
	uint16 ingress_stag_svid;
	uint16 ingress_ctag_cvid;
	rtk_rg_portmask_t egress_portmask;

	//action: force assign ctag
	rtk_rg_gpon_ds_bc_action_fields_t action_type;
	rtk_rg_gpon_ds_bc_action_t ctag_action;
	rtk_rg_gpon_ds_bc_stag_action_t stag_action;
}rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t;


typedef struct rtk_rg_sw_gpon_ds_bc_vlanfilterAndRemarking_s
{
	uint32 valid;
	rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t filterRule;
}rtk_rg_sw_gpon_ds_bc_vlanfilterAndRemarking_t;


//ACL
#if 1
typedef enum rtk_rg_cfpri_decision_e{
	ACL_CFPRI_ASSIGN,
	ACL_CFPRI_NOP,
}rtk_rg_cfpri_decision_t;

typedef struct rtk_rg_cfpri_action_s{
	rtk_rg_cfpri_decision_t cfPriDecision;
	uint8 assignedCfPri;
}rtk_rg_cfpri_action_t;

typedef enum rtk_rg_sid_llid_decision_e{
	ACL_SID_LLID_ASSIGN,
	ACL_SID_LLID_NOP,
}rtk_rg_sid_llid_decision_t;

typedef struct rtk_rg_sid_llid_action_s{
	rtk_rg_sid_llid_decision_t sidDecision;
	uint32 assignedSid_or_llid;
}rtk_rg_sid_llid_action_t;

typedef enum rtk_rg_dscp_decision_e{
	ACL_DSCP_ASSIGN,
	ACL_DSCP_NOP,
}rtk_rg_dscp_decision_t;

typedef struct rtk_rg_dscp_action_s{
	rtk_rg_dscp_decision_t dscpDecision;
	uint8 assignedDscp;
}rtk_rg_dscp_action_t;


typedef struct rtk_rg_log_action_s{
	uint8 assignedCounterIdx;
}rtk_rg_log_action_t;

typedef enum rtk_rg_acl_uni_decision_e{
	ACL_UNI_FWD_TO_PORTMASK_ONLY,
	ACL_UNI_FORCE_BY_MASK,
	ACL_UNI_TRAP_TO_CPU,
	AL_UNI_NOP,
}rtk_rg_acl_uni_decision_t;

typedef struct rtk_rg_uni_action_s{
	rtk_rg_acl_uni_decision_t	uniActionDecision;
	uint32 assignedUniPortMask;
}rtk_rg_uni_action_t;

typedef enum rtk_rg_acl_fwd_decision_e{
	ACL_FWD_NOP,
	ACL_FWD_DROP,
	ACL_FWD_TRAP_TO_CPU,
	ACL_FWD_DROP_TO_PON,
}rtk_rg_acl_fwd_decision_t;

typedef struct rtk_rg_fwd_action_s{
	rtk_rg_acl_fwd_decision_t	fwdDecision;
}rtk_rg_fwd_action_t;


typedef enum rtk_rg_acl_cvlan_tagif_decision_e{
	//The new action should alway put on later
	ACL_CVLAN_TAGIF_NOP,
	ACL_CVLAN_TAGIF_TAGGING,
	ACL_CVLAN_TAGIF_TAGGING_WITH_C2S, //apollo only
	ACL_CVLAN_TAGIF_TAGGING_WITH_SP2C, //apollo only
	ACL_CVLAN_TAGIF_UNTAG,
	ACL_CVLAN_TAGIF_TRANSPARENT,
	ACL_CVLAN_TAGIF_END,
}rtk_rg_acl_cvlan_tagif_decision_t;

typedef enum rtk_rg_acl_cvlan_cvid_decision_e{
	//The new action should alway put on later
	ACL_CVLAN_CVID_ASSIGN,
	ACL_CVLAN_CVID_COPY_FROM_1ST_TAG,
	ACL_CVLAN_CVID_COPY_FROM_2ND_TAG,
	ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID, //(upstream only)
	ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID, //(downstream only)
	ACL_CVLAN_CVID_NOP, //apolloFE  (downstream only)
	ACL_CVLAN_CVID_CPOY_FROM_SP2C, //apolloFE

	ACL_CVLAN_CVID_END,
}rtk_rg_acl_cvlan_cvid_decision_t;

typedef enum rtk_rg_acl_cvlan_cpri_decision_e{
	//The new action should alway put on later
	ACL_CVLAN_CPRI_ASSIGN,
	ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG,
	ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG,
	ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI,
	ACL_CVLAN_CPRI_NOP,//apolloFE
	ACL_CVLAN_CPRI_COPY_FROM_DSCP_REMAP,//apolloFE
	ACL_CVLAN_CPRI_COPY_FROM_SP2C, //apolloFE (downstream only)
	ACL_CVLAN_CPRI_END,
}rtk_rg_acl_cvlan_cpri_decision_t;

typedef enum rtk_rg_acl_svlan_tagif_decision_e{
	//The new action should alway put on later
	ACL_SVLAN_TAGIF_NOP,
	ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID,
	ACL_SVLAN_TAGIF_TAGGING_WITH_8100, //apollo
	ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C, //apollo (downstream only)
	ACL_SVLAN_TAGIF_UNTAG,
	ACL_SVLAN_TAGIF_TRANSPARENT,
	ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2, //apolloFE
	ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID, //apolloFE
	ACL_SVLAN_TAGIF_END,
}rtk_rg_acl_svlan_tagif_decision_t;

typedef enum rtk_rg_acl_svlan_svid_decision_e{
	//The new action should alway put on later
	ACL_SVLAN_SVID_ASSIGN,
	ACL_SVLAN_SVID_COPY_FROM_1ST_TAG,
	ACL_SVLAN_SVID_COPY_FROM_2ND_TAG,
	ACL_SVLAN_SVID_NOP, //apolloFE
	ACL_SVLAN_SVID_SP2C, //apolloFE
	ACL_SVLAN_SVID_END,
}rtk_rg_acl_svlan_svid_decision_t;

typedef enum rtk_rg_acl_svlan_spri_decision_e{
	//The new action should alway put on later
	ACL_SVLAN_SPRI_ASSIGN,
	ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG,
	ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG,
	ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI,
	ACL_SVLAN_SPRI_NOP,//apolloFE
	ACL_SVLAN_SPRI_COPY_FROM_DSCP_REMAP, //apolloFE (downstream only)
	ACL_SVLAN_SPRI_COPY_FROM_SP2C, //apolloFE (downstream only)
	ACL_SVLAN_SPRI_END,
}rtk_rg_acl_svlan_spri_decision_t;
#endif

typedef enum rtk_rg_acl_action_type_e
{


	//all platform supported
	ACL_ACTION_TYPE_DROP=0,	// 0
	ACL_ACTION_TYPE_PERMIT,
	ACL_ACTION_TYPE_TRAP,
	ACL_ACTION_TYPE_QOS,	//action of stream_id, CVLAN, SVLAN also belong to this type
	ACL_ACTION_TYPE_TRAP_TO_PS,
	ACL_ACTION_TYPE_POLICY_ROUTE,		//sw only

	//apollo platform ony
	ACL_ACTION_TYPE_SW_PERMIT,	//6: sw only, and apollo only

	//apolloFE platform only, none for now.

	//apolloPro platform only
	ACL_ACTION_TYPE_SW_MIRROR_WITH_UDP_ENCAP, //7: sw only , and 9607C only
	ACL_ACTION_TYPE_FLOW_MIB,					//8:  9607C_MP only

	ACL_ACTION_TYPE_TRAP_WITH_PRIORITY,			//9: hw only, and flow based only
	ACL_ACTION_TYPE_SW_DROP,					//10: sw only, and flow based only

	ACL_ACTION_TYPE_TRAP2SLAVE_WITH_PRIORITY,	//11: hw only, and flow based only

	ACL_ACTION_TYPE_END
} rtk_rg_acl_action_type_t;

typedef enum rtk_rg_acl_filter_and_qos_action_e
{
	ACL_ACTION_NOP_BIT=(1<<0),
	ACL_ACTION_1P_REMARKING_BIT=(1<<1),
	ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT=(1<<2),
	ACL_ACTION_DSCP_REMARKING_BIT=(1<<3),
	ACL_ACTION_QUEUE_ID_BIT=(1<<4),
	ACL_ACTION_SHARE_METER_BIT=(1<<5),
	ACL_ACTION_STREAM_ID_OR_LLID_BIT=(1<<6),
	ACL_ACTION_ACL_PRIORITY_BIT=(1<<7),
	ACL_ACTION_ACL_CVLANTAG_BIT=(1<<8),
	ACL_ACTION_ACL_SVLANTAG_BIT=(1<<9),
	ACL_ACTION_ACL_INGRESS_VID_BIT=(1<<10),
	ACL_ACTION_DS_UNIMASK_BIT	= (1<<11),		//only support in ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN(type 4)
	ACL_ACTION_REDIRECT_BIT = (1<<12), //only support in ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET(type 0)
	ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT = (1<<13), //only support in ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_XXXX(type1~4)
	ACL_ACTION_TOS_TC_REMARKING_BIT=(1<<14),
	ACL_ACTION_LOG_COUNTER_BIT=(1<<15),
#if defined(CONFIG_CMCC)||defined(CONFIG_CU_BASEON_CMCC)
	ACL_ACTION_CF_LOG_COUNTER_BIT=(1<<16),
#endif
	ACL_ACTION_QOS_END=(1<<17),
} rtk_rg_acl_qos_action_t;


typedef enum rtk_rg_acl_filter_fields_e
{
	INGRESS_PORT_BIT=0x1,
	INGRESS_INTF_BIT=0x2,
	EGRESS_INTF_BIT=0x4,		//not support in 0371 testchip
	INGRESS_ETHERTYPE_BIT=0x8,
	INGRESS_CTAG_PRI_BIT=0x10,
	INGRESS_CTAG_VID_BIT=0x20,
	INGRESS_SMAC_BIT=0x40,
	INGRESS_DMAC_BIT=0x80,
	INGRESS_DSCP_BIT=0x100,
	INGRESS_L4_TCP_BIT=0x200,
	INGRESS_L4_UDP_BIT=0x400,
	INGRESS_IPV6_SIP_RANGE_BIT=0x800,
	INGRESS_IPV6_DIP_RANGE_BIT=0x1000,
	INGRESS_IPV4_SIP_RANGE_BIT=0x2000,
	INGRESS_IPV4_DIP_RANGE_BIT=0x4000,
	INGRESS_L4_SPORT_RANGE_BIT=0x8000,
	INGRESS_L4_DPORT_RANGE_BIT=0x10000,
	EGRESS_IPV4_SIP_RANGE_BIT=0x20000,	//not support in 0371 testchip
	EGRESS_IPV4_DIP_RANGE_BIT=0x40000,	//not support in 0371 testchip
	EGRESS_L4_SPORT_RANGE_BIT=0x80000,	//not support in 0371 testchip
	EGRESS_L4_DPORT_RANGE_BIT=0x100000,	//not support in 0371 testchip
	INGRESS_L4_ICMP_BIT=0x200000,	//not support in 0371 testchip,  IPv4 ICMP only
	EGRESS_CTAG_PRI_BIT=0x400000,
	EGRESS_CTAG_VID_BIT=0x800000,
	INGRESS_IPV6_DSCP_BIT=0x1000000,			//Only support while PPPoE Passthrought disabled.
	INGRESS_STREAM_ID_BIT=0x2000000,
	INGRESS_STAG_PRI_BIT=0x4000000,
	INGRESS_STAG_VID_BIT=0x8000000,
	INGRESS_STAGIF_BIT=0x10000000,
	INGRESS_CTAGIF_BIT=0x20000000,
	INGRESS_EGRESS_PORTIDX_BIT=0x40000000,	//cf [2:0]uni pattern: ingress_port_idx for US, egress_port_idx for DS
	INTERNAL_PRI_BIT=0x80000000,	//cf [7:5]IntPri pattern
	INGRESS_L4_POROTCAL_VALUE_BIT=(1ULL<<32),
	INGRESS_TOS_BIT=(1ULL<<33),
	INGRESS_IPV6_TC_BIT=(1ULL<<34),
	INGRESS_IPV6_SIP_BIT =(1ULL<<35),
	INGRESS_IPV6_DIP_BIT =(1ULL<<36),
	INGRESS_WLANDEV_BIT =(1ULL<<37),	//only supported in fwdEngine, and limit ingress_port_mask is (1<<RTK_RG_EXT_PORT0)
	INGRESS_IPV4_TAGIF_BIT =(1ULL<<38),
	INGRESS_IPV6_TAGIF_BIT =(1ULL<<39),
	INGRESS_L4_ICMPV6_BIT = (1ULL<<40),	//IPv6 ICMPv6 only
	INGRESS_CTAG_CFI_BIT = (1ULL<<41),
	INGRESS_STAG_DEI_BIT = (1ULL<<42),
	EGRESS_IP4MC_IF =  (1ULL<<43),
	EGRESS_IP6MC_IF =  (1ULL<<44),
	INGRESS_L4_NONE_TCP_NONE_UDP_BIT = (1ULL<<45),
	EGRESS_DMAC_BIT =  (1ULL<<46),
	EGRESS_SMAC_BIT =  (1ULL<<47),
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	INGRESS_IPV6_FLOWLABEL_BIT = (1ULL<<48),
	EGRESS_WLANDEV_BIT =(1ULL<<49),	//only supported in fwdEngine, and limit ingress_port_mask is (1<<RTK_RG_EXT_PORT0)
	INGRESS_TCP_FLAGS_BIT = (1ULL<<50),
	INGRESS_PKT_LEN_RANGE_BIT = (1ULL<<51),
#elif defined(CONFIG_RG_RTL9602C_SERIES)
	INGRESS_PKT_LEN_RANGE_BIT = (1ULL<<48),
#endif
} rtk_rg_acl_filter_fields_t;


typedef enum rtk_rg_acl_fwding_type_direction_e{
	ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET=0,
	ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP,
	ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP,
	ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN,
	ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN,
	ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP, //apolloFE
	ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP, //apolloFE
	ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT, //apolloFE
	ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT, //apolloFE
}rtk_rg_acl_fwding_type_direction_t;


typedef struct rtk_rg_mirror_udp_encap_tag_action_s{
	uint8 encap_ip_ipv6;	//0: encap ip with ipv4, 1: encap ip with ipv6
	int8 mirror_count;		//-1:unlimit
	uint8 no_encap:1;		//0:encap with UDP, 1:no encapsulate
	uint8 remake_smac:1;	//used when no_encap, 0:do not remake smac, 1:remake smac
	uint8 tcp_header_only:1;	//0:whole packet, 1:cut tcp payload(if !tcp, do nothing)
	uint8 tcp_rule_append:1;	//0:disabled, 1:append tcp_rule_idx to tcp packet(if !tcp, do nothing)
	uint16 tcp_rule_idx;		//only append if tcp_rule_addend is 1
	
	rtk_mac_t encap_smac;
	rtk_mac_t encap_dmac;
	uint32 encap_Sip;
	uint32 encap_Dip;
	uint32 encap_Dport;
	uint8 encap_Sip_ipv6[16];
	uint8 encap_Dip_ipv6[16];
}rtk_rg_mirror_udp_encap_tag_action_t;


typedef struct rtk_rg_cvlan_tag_action_s{
	rtk_rg_acl_cvlan_tagif_decision_t 	cvlanTagIfDecision;
	rtk_rg_acl_cvlan_cvid_decision_t	cvlanCvidDecision;
	rtk_rg_acl_cvlan_cpri_decision_t	cvlanCpriDecision;
	uint32 assignedCvid;
	uint8 assignedCpri;
}rtk_rg_cvlan_tag_action_t;

typedef struct rtk_rg_svlan_tag_action_s{
	rtk_rg_acl_svlan_tagif_decision_t 	svlanTagIfDecision;
	rtk_rg_acl_svlan_svid_decision_t	svlanSvidDecision;
	rtk_rg_acl_svlan_spri_decision_t	svlanSpriDecision;
	uint32 assignedSvid;
	uint8 assignedSpri;
}rtk_rg_svlan_tag_action_t;

typedef struct rtk_rg_aclFilterAndQos_s
{
	//unsigned int filter_fields;
	unsigned long long int filter_fields; // please refer to rtk_rg_acl_filter_fields_t typedef.
	unsigned long long int filter_fields_inverse; // please refer to rtk_rg_acl_filter_fields_t typedef. supporting for inverse pattern compare result, 9607C only

	rtk_rg_acl_fwding_type_direction_t fwding_type_and_direction;
	int acl_weight;	//the larger number, the higher priority.

	//pattern
	rtk_rg_portmask_t ingress_port_mask;
	int ingress_port_idx;	//only support in upstream type(CF uni pattern, fwding_type_and_direction is 1 or 3)
	int egress_port_idx;	//only support in downstream type(CF uni pattern, fwding_type_and_direction is 2 or 4); in L34 mode is CPU port

	int ingress_dscp;
	int ingress_tos;
	int ingress_ipv6_dscp;	//Only support while PPPoE Passthrought disabled.
	int ingress_ipv6_tc;	//Only support while PPPoE Passthrought disabled.
	int ingress_intf_idx;
	int egress_intf_idx;
	int ingress_ethertype;
	int ingress_ctag_vid;
	int ingress_ctag_pri;
	int ingress_ctag_cfi;
	int ingress_stag_vid;
	int ingress_stag_pri;
	int ingress_stag_dei;
	int ingress_stream_id;
	int egress_ctag_vid;
	int egress_ctag_pri;
	int ingress_ctagIf;		//if  INGRESS_CTAGIF_BIT valid => 0:must not  have ctag   1:must have ctag
	int ingress_stagIf;		//if  INGRESS_STAGIF_BIT valid => 0:must not have stag   1:must have stag
	rtk_mac_t ingress_smac;
	rtk_mac_t ingress_dmac;
	ipaddr_t ingress_src_ipv4_addr_start;
	ipaddr_t ingress_src_ipv4_addr_end;
	ipaddr_t ingress_dest_ipv4_addr_start;
	ipaddr_t ingress_dest_ipv4_addr_end;
	uint8 ingress_src_ipv6_addr_start[16];
	uint8 ingress_src_ipv6_addr_end[16];
	uint8 ingress_dest_ipv6_addr_start[16];
	uint8 ingress_dest_ipv6_addr_end[16];
	unsigned short int ingress_src_l4_port_start;
	unsigned short int ingress_src_l4_port_end;
	unsigned short int ingress_dest_l4_port_start;
	unsigned short int ingress_dest_l4_port_end;
	ipaddr_t egress_src_ipv4_addr_start;
	ipaddr_t egress_src_ipv4_addr_end;
	ipaddr_t egress_dest_ipv4_addr_start;
	ipaddr_t egress_dest_ipv4_addr_end;
	unsigned short int egress_src_l4_port_start;
	unsigned short int egress_src_l4_port_end;
	unsigned short int egress_dest_l4_port_start;
	unsigned short int egress_dest_l4_port_end;
	int internal_pri;
	uint16 ingress_l4_protocal;
	uint8 ingress_src_ipv6_addr[16]; //using with ingress_src_ipv6_addr_mask
	uint8 ingress_dest_ipv6_addr[16]; //using with ingress_dest_ipv6_addr_mask
	uint32 ingress_wlanDevMask; //bit[0]:EXT0 root, bit[1~4]:EXT0 vap, bit[5~12]:EXT0 wps,  bit[13]:EXT1 root, bit[14~17]:EXT1 vap, bit[18~25]:EXT1 wps.
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	uint32 egress_wlanDevMask; //bit[0]:EXT0 root, bit[1~4]:EXT0 vap, bit[5~12]:EXT0 wps,  bit[13]:EXT1 root, bit[14~17]:EXT1 vap, bit[18~25]:EXT1 wps.
#endif
	uint16 ingress_ipv4_tagif;	//0:must not be ipv4,  1:must be ipv4,
	uint16 ingress_ipv6_tagif;	//0:must not be ipv6,  1:must be ipv6,
	uint16 egress_ip4mc_if; //0:must not be ipv4 multicast, not include IGMP
	uint16 egress_ip6mc_if; //0:must not be ipv6 multicast, not include MLD
	rtk_mac_t egress_smac;
	rtk_mac_t egress_dmac;
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	uint32 ingress_ipv6_flow_label;	//9607C FS[3], FS[4] for such pattern, G3 ip.flow_label
	uint16 ingress_tcp_flags;
	uint16 ingress_tcp_flags_mask;
	uint16 ingress_packet_length_start;
	uint16 ingress_packet_length_end;
#elif defined(CONFIG_RG_RTL9602C_SERIES)
	uint16 ingress_packet_length_start;
	uint16 ingress_packet_length_end;
#endif

	//patter mask
	rtk_mac_t ingress_smac_mask;
	rtk_mac_t ingress_dmac_mask;
	int ingress_stream_id_mask;
	int ingress_ethertype_mask;
	//int egress_uni_mask;
	int ingress_port_idx_mask;	//only support in upstream type(CF uni pattern, fwding_type_and_direction is 1 or 3) , relate to INGRESS_EGRESS_PORTIDX_BIT
	int egress_port_idx_mask;	//only support in downstream type(CF uni pattern, fwding_type_and_direction is 2 or 4), relate to INGRESS_EGRESS_PORTIDX_BIT; in L34 mode is CPU port
	uint8 ingress_src_ipv6_addr_mask[16];
	uint8 ingress_dest_ipv6_addr_mask[16];
	uint16 egress_ctag_vid_mask;


	//action
	rtk_rg_acl_action_type_t action_type;
	rtk_rg_acl_qos_action_t qos_actions; /* only used for action_type=ACL_ACTION_TYPE_QOS */

	unsigned char action_dot1p_remarking_pri;
	unsigned char action_ip_precedence_remarking_pri;
	unsigned char action_dscp_remarking_pri;
	unsigned char action_tos_tc_remarking_pri;
	unsigned char action_queue_id;
	unsigned char action_share_meter;
	unsigned char action_log_counter;
	unsigned char action_stream_id_or_llid;
	unsigned char action_acl_priority;
	rtk_rg_cvlan_tag_action_t action_acl_cvlan;
	rtk_rg_svlan_tag_action_t action_acl_svlan;
	unsigned char action_policy_route_wan;
	int action_acl_ingress_vid;
	uint32 downstream_uni_portmask;
	uint32 redirect_portmask;
	uint32 egress_internal_priority;
	rtk_rg_mirror_udp_encap_tag_action_t action_encap_udp;
	uint32 action_flowmib_counter_idx;
	uint32 action_trap_with_priority;
#if defined(CONFIG_CMCC)||defined(CONFIG_CU_BASEON_CMCC)
	unsigned char action_cf_log_counter;
#endif
} rtk_rg_aclFilterAndQos_t;


#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RTL9601B_SERIES)

typedef struct rtk_rg_aclFilterEntry_s
{
	uint32 hw_aclEntry_start;
	uint32 hw_aclEntry_size;
	uint32 hw_cfEntry_start;
	uint32 hw_cfEntry_size;
	uint32 hw_aclEntry_for_streamID_start;
	uint32 hw_aclEntry_for_streamID_size;

	rtk_rg_aclSWEntry_used_tables_field_t hw_used_table;//record which range tables are used
	uint8 hw_used_table_index[USED_TABLE_END];//record  used range tables index
	rtk_rg_aclSWEntry_type_t type;
	rtk_rg_aclFilterAndQos_t acl_filter;
	rtk_rg_enable_t valid;
}rtk_rg_aclFilterEntry_t;

#elif defined(CONFIG_RG_RTL9602C_SERIES)

typedef struct rtk_rg_aclFilterEntry_s
{
	uint32 hw_aclEntry_start;
	uint32 hw_aclEntry_size;
	uint32 hw_cfEntry_start;
	uint32 hw_cfEntry_size;
	rtk_rg_aclSWEntry_used_tables_field_t hw_used_table;//record which range tables are used
	uint8 hw_used_table_index[USED_TABLE_END];//record  used range tables index
	rtk_rg_aclFilterAndQos_t acl_filter;
	rtk_rg_enable_t valid;
}rtk_rg_aclFilterEntry_t;

#elif defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)

typedef struct rtk_rg_aclFilterEntry_s
{
	uint32 hw_aclEntry_start;
	uint32 hw_aclEntry_size;
	rtk_rg_aclSWEntry_used_tables_field_t hw_used_table;//record which range tables are used
	uint8 hw_used_table_index[USED_TABLE_END];//record  used range tables index
	uint8 COUNT_HWACL_LENGTH_FIELD;		//CAUTION!!this field will be used to count hw related field size, DO NOT MOVE IT!!
	rtk_rg_aclFilterAndQos_t acl_filter;
	rtk_rg_enable_t valid;
}rtk_rg_aclFilterEntry_t;

#elif defined(CONFIG_RG_G3_SERIES)

typedef struct rtk_rg_aclFilterEntry_s
{
	uint32 hw_aclEntry_count;
	rtk_portmask_t hw_aclEntry_port;
	unsigned char hw_aclEntry_index[128];//use string to record the hw acl related index
	uint8 COUNT_HWACL_LENGTH_FIELD;		//CAUTION!!this field will be used to count hw related field size, DO NOT MOVE IT!!
	rtk_rg_aclFilterAndQos_t acl_filter;
	rtk_rg_enable_t valid;
}rtk_rg_aclFilterEntry_t;

#endif

typedef enum rtk_rg_napt_filter_direction_check_e
{
	CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE = 0,
	CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE,
	CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE,
	CHECK_INBOUND_PKT_WITH_INBOUND_RULE,
	CHECK_BOUND_PKT_WITH_BOUND_RULE_END,
}rtk_rg_napt_filter_direction_check_t;

typedef enum rtk_rg_napt_filter_fields_e
{
	INGRESS_SIP=(1<<0),
	EGRESS_SIP=(1<<1),
	INGRESS_DIP=(1<<2),
	EGRESS_DIP=(1<<3),
	INGRESS_SPORT=(1<<4),
	EGRESS_SPORT=(1<<5),
	INGRESS_DPORT=(1<<6),
	EGRESS_DPORT=(1<<7),
	L4_PROTOCAL=(1<<8),
	INGRESS_SIP_RANGE=(1<<9),
	INGRESS_DIP_RANGE=(1<<10),
	INGRESS_SPORT_RANGE=(1<<11),
	INGRESS_DPORT_RANGE=(1<<12),
	EGRESS_SIP_RANGE=(1<<13),
	EGRESS_DIP_RANGE=(1<<14),
	EGRESS_SPORT_RANGE=(1<<15),
	EGRESS_DPORT_RANGE=(1<<16),
	INGRESS_SMAC = (1<<17),
	EGRESS_DMAC = (1<<18),
}rtk_rg_napt_filter_fields_t;

typedef enum rtk_rg_napt_action_fields_e
{
	ASSIGN_NAPT_PRIORITY_BIT=(1<<0),
	NAPT_DROP_BIT=(1<<1),
	NAPT_PERMIT_BIT=(1<<2),
	NAPT_SW_RATE_LIMIT_BIT=(1<<3), //hit this action will always fowraed by SW(skip add H/W NAPT entry) and do SW rate limit. (if flowed by HWNAT, this action is useless)
	NAPT_SW_TRAP_TO_PS=(1<<4),//hit this action will always fowraed by SW(skip add H/W NAPT entry) and do SW trap to PS. (if flowed by HWNAT, this action is useless)
	NAPT_SW_PACKET_COUNT=(1<<5),//hit this action will always fowraed by SW(skip add H/W NAPT entry), and update the naptFilter.packet_count
	NAPT_SW_BYTE_COUNT=(1<<5),//hit this action will always fowraed by SW(skip add H/W NAPT entry), and update the naptFilter.byte_count
	NAPT_SW_COPY_TO_PS=(1<<6),//hit this action will forwarded by SW(skip add H/W NAPT entry) and do SW copy to PS. (if flowed by HWNAT, this action is useless)
}rtk_rg_napt_action_fields_t;

typedef enum rtk_rg_napt_fwding_direction_e{
	RTK_RG_NAPT_FILTER_OUTBOUND=0,
	RTK_RG_NAPT_FILTER_INBOUND,
	RTK_RG_NAPT_FILTER_DIRECTION_END,
}rtk_rg_napt_fwding_direction_t;

typedef enum rtk_rg_napt_fwding_ruleType_e{
	RTK_RG_NAPT_FILTER_PERSIST=0,
	RTK_RG_NAPT_FILTER_ONE_SHOT,
	RTK_RG_NAPT_FILTER_RULE_TYPE_END,
}rtk_rg_napt_fwding_ruleType_t;


typedef struct rtk_rg_naptFilterAndQos_s
{
	rtk_rg_napt_fwding_direction_t direction;
	unsigned int followDirectionMatch:1;		//0:check both direction, 1:match naptFilter.direction only
	//compare patterns
	uint32 weight;
	rtk_rg_napt_filter_fields_t filter_fields;
	ipaddr_t ingress_src_ipv4_addr;
	ipaddr_t egress_src_ipv4_addr;
	ipaddr_t ingress_dest_ipv4_addr;
	ipaddr_t egress_dest_ipv4_addr;
	unsigned short int ingress_src_l4_port;
	unsigned short int egress_src_l4_port;
	unsigned short int ingress_dest_l4_port;
	unsigned short int egress_dest_l4_port;
	uint32 ingress_l4_protocal;

	ipaddr_t ingress_src_ipv4_addr_range_start;
	ipaddr_t ingress_src_ipv4_addr_range_end;
	ipaddr_t ingress_dest_ipv4_addr_range_start;
	ipaddr_t ingress_dest_ipv4_addr_range_end;
	unsigned short int ingress_src_l4_port_range_start;
	unsigned short int ingress_src_l4_port_range_end;
	unsigned short int ingress_dest_l4_port_range_start;
	unsigned short int ingress_dest_l4_port_range_end;

	ipaddr_t egress_src_ipv4_addr_range_start;
	ipaddr_t egress_src_ipv4_addr_range_end;
	ipaddr_t egress_dest_ipv4_addr_range_start;
	ipaddr_t egress_dest_ipv4_addr_range_end;
	unsigned short int egress_src_l4_port_range_start;
	unsigned short int egress_src_l4_port_range_end;
	unsigned short int egress_dest_l4_port_range_start;
	unsigned short int egress_dest_l4_port_range_end;

	rtk_mac_t ingress_smac;
	rtk_mac_t egress_dmac;

	rtk_rg_napt_action_fields_t action_fields;
	//assigned priority
	uint32 assign_priority;

	//assgined limit rate
	uint32 assign_rate;

	//assigned count which each flow need to be copied to protocol stack
	uint32 assign_copy_num;

	//get the accumulate packet count that hit this rule
	uint32 packet_count;
	uint32 byte_count;

	rtk_rg_napt_fwding_ruleType_t ruleType; //0:persist, 1:one shot

}rtk_rg_naptFilterAndQos_t;

typedef struct rtk_rg_sw_naptFilterAndQos_s
{
	uint32 valid;
	uint32 sw_index;
	rtk_rg_naptFilterAndQos_t naptFilter;
	struct rtk_rg_sw_naptFilterAndQos_s *pNextValid;
}rtk_rg_sw_naptFilterAndQos_t;

typedef struct rtk_rg_sw_copyToPSWork_s
{
#ifdef __KERNEL__
	struct sk_buff *copy_skb;
	struct work_struct copy_work;
	struct rx_info copy_rxInfo;
#if defined(CONFIG_MASTER_WLAN0_ENABLE)
	rtk_rg_mbssidDev_t copy_wlan_idx;
#endif
#endif
}rtk_rg_sw_copyToPSWork_t;

#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
typedef enum rtk_rg_acl_transform_type_e
{
	RG_ACL_TRANS_NONE=0,
	RG_ACL_TRANS_L2,
	RG_ACL_TRANS_v6_OTHER,
	RG_ACL_TRANS_v4_OTHER,
	RG_ACL_TRANS_v6_SUBNET,
	RG_ACL_TRANS_v4_SUBNET,				//5
	RG_ACL_TRANS_v4_OTHER_v6_OTHER,
	RG_ACL_TRANS_v4_SUBNET_v6_OTHER,
	RG_ACL_TRANS_v4_OTHER_v6_SUBNET,
	RG_ACL_TRANS_v4_SUBNET_v6_SUBNET,
	RG_ACL_TRANS_L2_PORT_BIND,			//10
	RG_ACL_TRANS_L2_VLAN_BIND,
	RG_ACL_TRANS_L34_PORT_BIND,
	RG_ACL_TRANS_L34_VLAN_BIND,
	RG_ACL_TRANS_L34_NOT_READY,
}rtk_rg_acl_transform_type_t;
#endif

//L2 classify

typedef enum rtk_rg_cf_direction_type_e
{
	RTK_RG_CLASSIFY_DIRECTION_UPSTREAM,
	RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM,
	RTK_RG_CLASSIFY_DIRECTION_END,
} rtk_rg_cf_direction_type_t;

typedef enum rtk_rg_cf_filter_fields_e
{
	EGRESS_ETHERTYPR_BIT=(1<<0),
	EGRESS_GEMIDX_BIT=(1<<1),
	EGRESS_LLID_BIT=(1<<2),
	EGRESS_TAGVID_BIT=(1<<3),
	EGRESS_TAGPRI_BIT=(1<<4),
	EGRESS_INTERNALPRI_BIT=(1<<5),
	EGRESS_STAGIF_BIT=(1<<6),
	EGRESS_CTAGIF_BIT=(1<<7),
	EGRESS_UNI_BIT=(1<<8),
} rtk_rg_cf_filter_fields_t;

typedef enum rtk_rg_cf_us_action_type_e
{
	CF_US_ACTION_STAG_BIT = (1ULL<<0),
	CF_US_ACTION_CTAG_BIT = (1ULL<<1),
	CF_US_ACTION_CFPRI_BIT=(1ULL<<2),
	CF_US_ACTION_DSCP_BIT=(1ULL<<3),//DSCP:apollo 9600Series have side effect!
	CF_US_ACTION_SID_BIT=(1ULL<<4),
	CF_US_ACTION_FWD_BIT=(1ULL<<5),//apolloFE change drop action to fwd action
	CF_US_ACTION_DROP_BIT=(1ULL<<6),
	CF_US_ACTION_LOG_BIT=(1ULL<<7),//LOG:apollo 9600Series have side effect!
	CF_US_ACTION_END_BIT=(1ULL<<8),
} rtk_rg_cf_us_action_type_t;


typedef enum rtk_rg_cf_ds_action_type_e
{
	CF_DS_ACTION_STAG_BIT = (1ULL<<0),
	CF_DS_ACTION_CTAG_BIT = (1ULL<<1),
	CF_DS_ACTION_CFPRI_BIT=(1ULL<<2),
	CF_DS_ACTION_DSCP_BIT=(1ULL<<3),//DSCP:apollo 9600Series have side effect!
	CF_DS_ACTION_UNI_MASK_BIT=(1ULL<<4),
	CF_DS_ACTION_DROP_BIT=(1ULL<<5),//DROP: using uni to achieve this action!
	CF_DS_ACTION_END_BIT=(1ULL<<6),
} rtk_rg_cf_ds_action_type_t;



typedef enum rtk_rg_cf_pattern_type_e{
	CF_PATTERN_ETHERTYPE=0,
	CF_PATTERN_GEMIDX_OR_LLID,
	CF_PATTERN_OUTTERTAGVID,
	CF_PATTERN_OUTTERTAGPRI,
	CF_PATTERN_INTERNALPRI,
	CF_PATTERN_STAGFLAG,
	CF_PATTERN_CTAGFLAG,
	CF_PATTERN_UNI,
	CF_PATTERN_END,
}rtk_rg_cf_pattern_type_t;


typedef struct rtk_rg_classifyEntry_s
{
	int index; //limit in 64-511,  0:invalid   else:should be the asic index,   apolloPro: FAIL means invalid
	rtk_rg_cf_direction_type_t direction;

	//patterns
	rtk_rg_cf_filter_fields_t filter_fields;

	uint32 etherType;
	uint32 gemidx;
	uint32 llid;
	uint32 outterTagVid;
	uint32 outterTagPri;
	uint32 internalPri;
	uint32 stagIf;
	uint32 ctagIf;
	uint32 uni;

	//pattern mask
	uint32 etherType_mask;
	uint32 gemidx_mask;
	uint32 uni_mask;

	//US actions mask
	rtk_rg_cf_us_action_type_t us_action_field;
	//DS actions  mask
	rtk_rg_cf_ds_action_type_t ds_action_field;

	rtk_rg_cvlan_tag_action_t action_cvlan;
	rtk_rg_svlan_tag_action_t action_svlan;
	rtk_rg_cfpri_action_t	action_cfpri;
	rtk_rg_sid_llid_action_t action_sid_or_llid;
	rtk_rg_dscp_action_t action_dscp;
	rtk_rg_fwd_action_t action_fwd;
	rtk_rg_log_action_t action_log;
	rtk_rg_uni_action_t action_uni;
}rtk_rg_classifyEntry_t;

typedef enum rtk_rg_flow_filter_fields_e
{
	INGRESS_FLOW_L4_PROTOCAL	=(1<<0),
	INGRESS_FLOW_SMAC 			=(1<<1),
	INGRESS_FLOW_SIP			=(1<<2),
	INGRESS_FLOW_DIP			=(1<<3),
	INGRESS_FLOW_SIP_RANGE		=(1<<4),
	INGRESS_FLOW_DIP_RANGE		=(1<<5),
	INGRESS_FLOW_SPORT			=(1<<6),
	INGRESS_FLOW_DPORT			=(1<<7),
	INGRESS_FLOW_SPORT_RANGE	=(1<<8),
	INGRESS_FLOW_DPORT_RANGE	=(1<<9),
	EGRESS_FLOW_DMAC 			=(1<<10),
	EGRESS_FLOW_SIP				=(1<<11),
	EGRESS_FLOW_DIP				=(1<<12),
	EGRESS_FLOW_SIP_RANGE		=(1<<13),
	EGRESS_FLOW_DIP_RANGE		=(1<<14),
	EGRESS_FLOW_SPORT			=(1<<15),
	EGRESS_FLOW_DPORT			=(1<<16),
	EGRESS_FLOW_SPORT_RANGE		=(1<<17),
	EGRESS_FLOW_DPORT_RANGE		=(1<<18),
}rtk_rg_flow_filter_fields_t;

typedef struct rtk_rg_flowFilterInfo_s
{
#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	uint8 delL2ShortCut;	// 0: do not delete L2 shortcut
	uint8 delL3ShortCut;	// 0: do not delete L3 shortcut
	uint8 delL34ShortCut;	// 0: do not delete L34 shortcut
	uint8 delHwFlow;		// 0: do not delete hw table(i.e., napt/naptr table or flow table)
#endif
	uint8 ipProtocol;		// 0: ipv4, 1: ipv6
	rtk_rg_napt_fwding_direction_t 	direction;
	rtk_rg_flow_filter_fields_t 	filter_fields;
	// ingress patterns
	uint8 		ingress_l4_protocal; //0: udp, 1: tcp
	rtk_mac_t 	ingress_smac;
	ipaddr_t 	ingress_src_ipv4_addr;
	ipaddr_t 	ingress_dst_ipv4_addr;
	ipaddr_t 	ingress_src_ipv4_addr_range_start;
	ipaddr_t 	ingress_src_ipv4_addr_range_end;
	ipaddr_t 	ingress_dst_ipv4_addr_range_start;
	ipaddr_t 	ingress_dst_ipv4_addr_range_end;
	rtk_ipv6_addr_t		ingress_src_ipv6_addr;
	rtk_ipv6_addr_t 	ingress_dst_ipv6_addr;
	rtk_ipv6_addr_t 	ingress_src_ipv6_addr_range_start;
	rtk_ipv6_addr_t 	ingress_src_ipv6_addr_range_end;
	rtk_ipv6_addr_t 	ingress_dst_ipv6_addr_range_start;
	rtk_ipv6_addr_t 	ingress_dst_ipv6_addr_range_end;
	uint16		ingress_src_l4_port;
	uint16		ingress_dst_l4_port;
	uint16		ingress_src_l4_port_range_start;
	uint16 		ingress_src_l4_port_range_end;
	uint16 		ingress_dest_l4_port_range_start;
	uint16 		ingress_dest_l4_port_range_end;

	// egress patterns
	rtk_mac_t 	egress_dmac;
	ipaddr_t 	egress_src_ipv4_addr;
	ipaddr_t 	egress_dst_ipv4_addr;
	ipaddr_t 	egress_src_ipv4_addr_range_start;
	ipaddr_t 	egress_src_ipv4_addr_range_end;
	ipaddr_t 	egress_dst_ipv4_addr_range_start;
	ipaddr_t 	egress_dst_ipv4_addr_range_end;
	rtk_ipv6_addr_t		egress_src_ipv6_addr;
	rtk_ipv6_addr_t 	egress_dst_ipv6_addr;
	rtk_ipv6_addr_t 	egress_src_ipv6_addr_range_start;
	rtk_ipv6_addr_t 	egress_src_ipv6_addr_range_end;
	rtk_ipv6_addr_t 	egress_dst_ipv6_addr_range_start;
	rtk_ipv6_addr_t 	egress_dst_ipv6_addr_range_end;
	uint16		egress_src_l4_port;
	uint16		egress_dst_l4_port;
	uint16		egress_src_l4_port_range_start;
	uint16 		egress_src_l4_port_range_end;
	uint16 		egress_dest_l4_port_range_start;
	uint16 		egress_dest_l4_port_range_end;
}rtk_rg_flowFilterInfo_t;

//Force Portal
typedef struct rtk_rg_forcePortalURL_s
{
	unsigned char url_string[MAX_URL_FILTER_STR_LENGTH];
	unsigned int valid:1;
	unsigned int attach_orig_url:1;		//0: not attached, 1: attache original URL after the url_string
	unsigned int manually_clear_req:1;	//0: atomatically clear redirect_http_req after redirect, 1:do not touch redirect_http_req
}rtk_rg_forcePortalURL_t;

//URL Filter
typedef enum rtk_rg_filterControlType_e
{
	RG_FILTER_BLACK=0,
	RG_FILTER_WHITE,
	RG_FILTER_NONE,
}rtk_rg_filterControlType_t;

typedef struct rtk_rg_hiPriEntryIdx_s
{	
	int connectId;
	int hiPriEntryIdx;
	int hiPriEntryIdx2;
	struct list_head hiPriEntryIdx_list;
} rtk_rg_hiPriEntryIdx_t;


typedef struct rtk_rg_urlHighPri_s
{
	unsigned char url_filter_string[MAX_URL_FILTER_STR_LENGTH];
	unsigned char path_filter_string[MAX_URL_FILTER_PATH_LENGTH];
	int path_exactly_match;
	int32 urlpri;
} rtk_rg_urlHighPri_t;


typedef struct rtk_rg_seUrlHighPri_s
{
	uint8 valid:1;
	rtk_rg_urlHighPri_t urlHighPriEt;
	struct list_head hiPriEntryIdxListHdr;
} rtk_rg_swUrlHighPri_t;


typedef struct rtk_rg_urlFilterString_s
{
	unsigned char url_filter_string[MAX_URL_FILTER_STR_LENGTH];
	unsigned char path_filter_string[MAX_URL_FILTER_PATH_LENGTH];
	int path_exactly_match;
	int wan_intf;
	uint8 urlfilterSmacCheck;
	rtk_rg_filterControlType_t urlfilterSmacMode; //only valid when urlfilterSmacCheck Enable
	uint8 urlfilterSamc[ETHER_ADDR_LEN];
	uint32 urlBlockAllowTimes;
} rtk_rg_urlFilterString_t;

/*URL Module*/
typedef struct rtk_rg_urlFilterEntry_s
{
	uint8 valid:1;
	uint16 urlEntryIdx;		//(read only)do not change it 1-1 mapping to array idx
	rtk_rg_urlFilterString_t urlFilter;

	struct list_head urlfilter_list;
}rtk_rg_urlFilterEntry_t;


//UPnP
typedef enum rtk_rg_upnp_type_e
{
	UPNP_TYPE_ONESHOT=0,
	UPNP_TYPE_PERSIST=1,
	UPNP_TYPE_END
} rtk_rg_upnp_type_t;

typedef struct rtk_rg_upnpConnection_s
{
	uint32 is_tcp:1;
	uint32 valid:1;
	int wan_intf_idx; //for gateway ip
	unsigned short int gateway_port;
	ipaddr_t local_ip;
	unsigned short int local_port;
	int limit_remote_ip;
	int limit_remote_port;
	ipaddr_t remote_ip;
	unsigned short int remote_port;
	rtk_rg_upnp_type_t type; //one shot?
	uint32 timeout; //auto-delete after timeout, 0:disable auto-delete
	uint32 idle;	//idle time
	uint32 conn_create_idle;	// idle time since last time created napt-connection by this upnp.
} rtk_rg_upnpConnection_t;

typedef enum rtk_rg_drv_event_record_id_e
{
	RTK_RG_DRV_EVENT_REC_VLAN_INSERT_FAIL,
	RTK_RG_DRV_EVENT_REC_PPPOE_INSERT_FAIL,
	RTK_RG_DRV_EVENT_REC_DUAL_INSERT_FAIL,
	RTK_RG_DRV_EVENT_REC_VLAN_MODIFY_FAIL,
	RTK_RG_DRV_EVENT_REC_VLAN_REMOVE_FAIL,
	RTK_RG_DRV_EVENT_REC_TAIL_DROP,
	RTK_RG_DRV_EVENT_REC_PP_LEN_INVALID,
	RTK_RG_DRV_EVENT_REC_FLOW_LRU,
	RTK_RG_DRV_EVENT_REC_MC_TBL_FULL,
	RTK_RG_DRV_EVENT_REC_LUT_LRU,
	RTK_RG_DRV_EVENT_REC_FLOW_ADD_FAIL,
	RTK_RG_DRV_EVENT_REC_MAX,
}rtk_rg_drv_event_record_id_t;

typedef struct rtk_rg_drv_event_record_s
{
	rtk_rg_drv_event_record_id_t eventIdx;
	unsigned char eventName[64];
	unsigned int event_id;
}rtk_rg_drv_event_record_t;


/* Init */
typedef int (*p_initByHwCallBack)(void);
typedef int (*p_arpAddByHwCallBack)(rtk_rg_arpInfo_t*);
typedef int (*p_arpDelByHwCallBack)(rtk_rg_arpInfo_t*);
typedef int (*p_macAddByHwCallBack)(rtk_rg_macEntry_t*);
typedef int (*p_macDelByHwCallBack)(rtk_rg_macEntry_t*);
//5
typedef int (*p_routingAddByHwCallBack)(rtk_rg_ipv4RoutingEntry_t*);
typedef int (*p_routingDelByHwCallBack)(rtk_rg_ipv4RoutingEntry_t*);
typedef int (*p_naptAddByHwCallBack)(rtk_rg_naptInfo_t*);
typedef int (*p_naptDelByHwCallBack)(rtk_rg_naptInfo_t*);
typedef int (*P_bindAddByHwCallBack)(rtk_rg_bindingEntry_t*);
//10
typedef int (*P_bindDelByHwCallBack)(rtk_rg_bindingEntry_t*);
typedef int (*p_interfaceAddByHwCallBack)(rtk_rg_intfInfo_t*,int*);
typedef int (*p_interfaceDelByHwCallBack)(rtk_rg_intfInfo_t*,int*);
typedef int (*p_neighborAddByHwCallBack)(rtk_rg_neighborInfo_t*);
typedef int (*p_neighborDelByHwCallBack)(rtk_rg_neighborInfo_t*);
//15
typedef int (*p_v6RoutingAddByHwCallBack)(rtk_rg_ipv6RoutingEntry_t*);
typedef int (*p_v6RoutingDelByHwCallBack)(rtk_rg_ipv6RoutingEntry_t*);
typedef int (*p_naptInboundConnLookupFirstCallBack)(void*,ipaddr_t*,uint16*);
typedef int (*p_naptInboundConnLookupSecondCallBack)(void*,ipaddr_t*,uint16*);
typedef int (*p_naptInboundConnLookupThirdCallBack)(void*,ipaddr_t*,uint16*);
//20
typedef int (*p_dhcpRequestByHwCallBack)(int*);
typedef int (*p_pppoeBeforeDiagByHwCallBack)(rtk_rg_pppoeClientInfoBeforeDial_t*,int*);
typedef int (*p_pptpBeforeDialByHwCallBack)(rtk_rg_pptpClientInfoBeforeDial_t*,int*);
typedef int (*p_l2tpBeforeDialByHwCallBack)(rtk_rg_l2tpClientInfoBeforeDial_t*,int*);
typedef int (*p_pppoeDsliteBeforeDialByHwCallBack)(rtk_rg_pppoeClientInfoBeforeDial_t*,int*);
//25
typedef int (*p_ipv6NaptInboundConnLookupFirstCallBack)(void*,rtk_ipv6_addr_t*,uint16*);
typedef int (*p_ipv6NaptInboundConnLookupSecondCallBack)(void*,rtk_ipv6_addr_t*,uint16*);
typedef int (*p_ipv6NaptInboundConnLookupThirdCallBack)(void*,rtk_ipv6_addr_t*,uint16*);
typedef int (*p_softwareNaptInfoAddCallBack)(rtk_rg_naptInfo_t*);
typedef int (*p_softwareNaptInfoDeleteCallBack)(rtk_rg_naptInfo_t*);
//30
typedef int (*p_naptPreRouteDPICallBack)(void*,rtk_rg_naptDirection_t);
typedef int (*p_naptForwardDPICallBack)(void*,rtk_rg_naptDirection_t);
typedef int (*p_pppoeLCPStateCallBack)(void**);

#ifdef __KERNEL__
typedef struct rtk_rg_wq_union_s
{
	union{
		//struct{}initByHwCallBack;
		//struct{
			rtk_rg_arpInfo_t arpInfo;
		//}arpCallBack;							//for arpAddByHwCallBack and arpDelByHwCallBack
		//struct{
			rtk_rg_macEntry_t macInfo;
		//}macCallBack;					//for macAddByHwCallBack and macDelByHwCallBack
		//struct{
			rtk_rg_ipv4RoutingEntry_t v4RoutingInfo;
		//}routingCallBack;				//for routingAddByHwCallBack and routingDelByHwCallBack
		//struct{
			rtk_rg_naptInfo_t naptInfo;
		//}naptCallBack;					//for naptAddByHwCallBack and naptDelByHwCallBack
		//struct{
			rtk_rg_bindingEntry_t bindInfo;
		//}bindingCallBack;				//for bindingAddByHwCallBack and bindingDelByHwCallBack
		struct {
			rtk_rg_intfInfo_t intfInfo;
			int intfIdx;
		}interfaceCallBack;				//for interfaceAddByHwCallBack and interfaceDelByHwCallBack
		//struct {
			rtk_rg_neighborInfo_t neighborInfo;
		//}neighborCallBack;				//for neighborAddByHwCallBack and neighborDelByHwCallBack
		//struct {
			rtk_rg_ipv6RoutingEntry_t v6RoutingInfo;
		//}v6RoutingCallBack;				//for v6RoutingAddByHwCallBack and v6RoutingDelByHwCallBack
		struct {
			rtk_rg_pppoeClientInfoBeforeDial_t pppoeBeforeInfo;
			int wanIdx;
		}pppoeBeforeDialCallBack;		//for pppoeBeforeDialByHwCallBack
		//struct {
			int wanIdx;
		//}dhcpRequestCallBack;			//for dhcpRequestByHwCallBack
		struct {
			rtk_rg_pptpClientInfoBeforeDial_t pptpBeforeInfo;
			int wanIdx;
		}pptpBeforeDialCallBack;		//for pptpBeforeDialByHwCallBack
		struct {
			rtk_rg_l2tpClientInfoBeforeDial_t l2tpBeforeInfo;
			int wanIdx;
		}l2tpBeforeDialCallBack;		//for l2tpBeforeDialByHwCallBack
		struct {
			rtk_rg_pppoeClientInfoBeforeDial_t pppoeDsliteBeforeInfo;
			int wanIdx;
		}pppoeDsliteBeforeDialCallBack;	//for pppoeDsliteBeforeDialByHwCallBack
		//struct {
			//rtk_rg_naptInfo_t naptInfo;
		//}softwareNaptCallBack;			//for softwareNaptAddCallBack and softwareNaptDelCallBack
		//struct {
			void *data;
		//}pppoeLCPStateCallBack;			//for pppoeLCPStateCallBack
#if defined(CONFIG_RG_G3_SERIES)
		struct {
			uint32 reg;
			uint32 value;
		}devmemCallBack;
#endif
		struct {
			struct sk_buff *skb;
			uint16 port_num;
		}udpMirrorCallBack;
	};
	struct work_struct work;
	atomic_t not_used;	//1: not be used, 0: be used
}rtk_rg_wq_union_t;
#endif

typedef struct rtk_rg_initParams_s
{
	uint32 igmpSnoopingEnable:2;
	uint32 macBasedTagDecision:1;		//control DMAC2CVID per-port and forced state registers
	uint32 wanPortGponMode:1;		//control wan port is GPON mode or EPON/UTP mode
	uint32 ivlMulticastSupport:1;	//control l2 multicast support IVL mode(hash by VLAN)
	uint32 hybridMode:1;			// used for hybrid mode

	uint32 fwdVLAN_CPU;
	uint32 fwdVLAN_CPU_SVLAN;
	uint32 fwdVLAN_Proto_Block;		//used for IP_version only_mode, as PVID to block traffic to other port
	uint32 fwdVLAN_BIND_INTERNET;	//used for LAN to transparent with internet WAN
	uint32 fwdVLAN_BIND_OTHER;		//used for other WAN separate with un-binding ports or internet WAN

	p_initByHwCallBack initByHwCallBack;
	p_arpAddByHwCallBack arpAddByHwCallBack;
	p_arpDelByHwCallBack arpDelByHwCallBack;
	p_macAddByHwCallBack macAddByHwCallBack;
	p_macDelByHwCallBack macDelByHwCallBack;
//5
	p_routingAddByHwCallBack routingAddByHwCallBack;
	p_routingDelByHwCallBack routingDelByHwCallBack;
	p_naptAddByHwCallBack naptAddByHwCallBack;
	p_naptDelByHwCallBack naptDelByHwCallBack;
	P_bindAddByHwCallBack bindingAddByHwCallBack;
//10
	P_bindDelByHwCallBack bindingDelByHwCallBack;
	p_interfaceAddByHwCallBack interfaceAddByHwCallBack;
	p_interfaceDelByHwCallBack interfaceDelByHwCallBack;
	p_neighborAddByHwCallBack neighborAddByHwCallBack;
	p_neighborDelByHwCallBack neighborDelByHwCallBack;
//15
	p_v6RoutingAddByHwCallBack v6RoutingAddByHwCallBack;
	p_v6RoutingDelByHwCallBack v6RoutingDelByHwCallBack;
	p_naptInboundConnLookupFirstCallBack naptInboundConnLookupFirstCallBack;
	p_naptInboundConnLookupSecondCallBack naptInboundConnLookupSecondCallBack;
	p_naptInboundConnLookupThirdCallBack naptInboundConnLookupThirdCallBack;
//20
	p_dhcpRequestByHwCallBack dhcpRequestByHwCallBack;
	p_pppoeBeforeDiagByHwCallBack pppoeBeforeDiagByHwCallBack;
	p_pptpBeforeDialByHwCallBack pptpBeforeDialByHwCallBack;
	p_l2tpBeforeDialByHwCallBack l2tpBeforeDialByHwCallBack;
	p_pppoeDsliteBeforeDialByHwCallBack pppoeDsliteBeforeDialByHwCallBack;
//25
	p_ipv6NaptInboundConnLookupFirstCallBack ipv6NaptInboundConnLookupFirstCallBack;
	p_ipv6NaptInboundConnLookupSecondCallBack ipv6NaptInboundConnLookupSecondCallBack;
	p_ipv6NaptInboundConnLookupThirdCallBack ipv6NaptInboundConnLookupThirdCallBack;
	p_softwareNaptInfoAddCallBack softwareNaptInfoAddCallBack;
	p_softwareNaptInfoDeleteCallBack softwareNaptInfoDeleteCallBack;
//30
	p_naptPreRouteDPICallBack naptPreRouteDPICallBack;
	p_naptForwardDPICallBack naptForwardDPICallBack;
	p_pppoeLCPStateCallBack pppoeLCPStateCallBack;
} rtk_rg_initParams_t;


/* internal */

typedef enum rtk_rg_flow_direction_s
{
	FLOW_DIRECTION_CAN_NOT_DECIDE=0,
	FLOW_DIRECTION_UPSTREAM, 	//LAN_TO_WAN
	FLOW_DIRECTION_DOWNSTREAM, //WAN_TO_LAN
	FLOW_DIRECTION_LAN_TO_LAN,
	FLOW_DIRECTION_WAN_TO_WAN,
}rtk_rg_flow_direction_t;



/*LUT Module*/
typedef enum rtk_rg_macfilter_interface_e
{
	INTERFACE_FOR_LAN=0,
	INTERFACE_FOR_WAN,
	INTERFACE_END
}rtk_rg_macfilter_interface_t;

typedef enum rtk_rg_macFilterEntry_direction_s
{
	RTK_RG_MACFILTER_FILTER_SRC_DEST_MAC_BOTH,
	RTK_RG_MACFILTER_FILTER_SRC_MAC_ONLY,
	RTK_RG_MACFILTER_FILTER_DEST_MAC_ONLY,
	RTK_RG_MACFILTER_FILTER_END
} rtk_rg_macFilterEntry_direction_t;

typedef struct rtk_rg_macFilterEntry_s
{
	rtk_mac_t mac;
	uint32 isIVL:1; //0:SVL, 1:IVL
	int vlan_id;
	rtk_rg_macFilterEntry_direction_t direct;
}rtk_rg_macFilterEntry_t;

typedef struct rtk_rg_macFilterSWEntry_s
{
	int valid;
	int l2_table_entry_index;
	int l2_table_entry_index_for_lan;
	int l2_table_entry_index_for_wan;
	rtk_rg_macFilterEntry_t macFilterEntry;
}rtk_rg_macFilterSWEntry_t;

/* MIB counter */
typedef struct rtk_rg_port_mib_info_s
{
    uint64 ifInOctets;
    uint32 ifInUcastPkts;
    uint32 ifInMulticastPkts;
    uint32 ifInBroadcastPkts;
    uint32 ifInDiscards;
    uint64 ifOutOctets;
    uint32 ifOutDiscards;
    uint32 ifOutUcastPkts;
    uint32 ifOutMulticastPkts;
    uint32 ifOutBrocastPkts;
    uint32 dot1dBasePortDelayExceededDiscards;
    uint32 dot1dTpPortInDiscards;
    uint32 dot1dTpHcPortInDiscards;
    uint32 dot3InPauseFrames;
    uint32 dot3OutPauseFrames;
    //uint32 dot3OutPauseOnFrames;
    uint32 dot3StatsAligmentErrors;
    uint32 dot3StatsFCSErrors;
    uint32 dot3StatsSingleCollisionFrames;
    uint32 dot3StatsMultipleCollisionFrames;
    uint32 dot3StatsDeferredTransmissions;
    uint32 dot3StatsLateCollisions;
    uint32 dot3StatsExcessiveCollisions;
    uint32 dot3StatsFrameTooLongs;
    uint32 dot3StatsSymbolErrors;
    uint32 dot3ControlInUnknownOpcodes;
    uint32 etherStatsDropEvents;
    uint64 etherStatsOctets;
    uint32 etherStatsBcastPkts;
    uint32 etherStatsMcastPkts;
    uint32 etherStatsUndersizePkts;
    uint32 etherStatsOversizePkts;
    uint32 etherStatsFragments;
    uint32 etherStatsJabbers;
    uint32 etherStatsCollisions;
    uint32 etherStatsCRCAlignErrors;
    uint32 etherStatsPkts64Octets;
    uint32 etherStatsPkts65to127Octets;
    uint32 etherStatsPkts128to255Octets;
    uint32 etherStatsPkts256to511Octets;
    uint32 etherStatsPkts512to1023Octets;
    uint32 etherStatsPkts1024to1518Octets;
    uint64 etherStatsTxOctets;
    uint32 etherStatsTxUndersizePkts;
    uint32 etherStatsTxOversizePkts;
    uint32 etherStatsTxPkts64Octets;
    uint32 etherStatsTxPkts65to127Octets;
    uint32 etherStatsTxPkts128to255Octets;
    uint32 etherStatsTxPkts256to511Octets;
    uint32 etherStatsTxPkts512to1023Octets;
    uint32 etherStatsTxPkts1024to1518Octets;
    uint32 etherStatsTxPkts1519toMaxOctets;
    uint32 etherStatsTxBcastPkts;
    uint32 etherStatsTxMcastPkts;
    uint32 etherStatsTxFragments;
    uint32 etherStatsTxJabbers;
    uint32 etherStatsTxCRCAlignErrors;
    uint32 etherStatsRxUndersizePkts;
    uint32 etherStatsRxUndersizeDropPkts;
    uint32 etherStatsRxOversizePkts;
    uint32 etherStatsRxPkts64Octets;
    uint32 etherStatsRxPkts65to127Octets;
    uint32 etherStatsRxPkts128to255Octets;
    uint32 etherStatsRxPkts256to511Octets;
    uint32 etherStatsRxPkts512to1023Octets;
    uint32 etherStatsRxPkts1024to1518Octets;
    uint32 etherStatsRxPkts1519toMaxOctets;
    uint32 inOampduPkts;
    uint32 outOampduPkts;
	//sw mib info
    uint32 ifInSwUcastPkts;
    uint32 ifInSwMassUcastPkts;		//len > MASS_PACKET_SIZE
    uint32 ifInSwMulticastPkts;
    uint32 ifInSwMassMulticastPkts;	//len > MASS_PACKET_SIZE
    uint32 ifInSwBroadcastPkts;
    uint32 ifInSwMassBroadcastPkts;	//len > MASS_PACKET_SIZE
    uint32 ifOutSwUcastPkts;
    uint32 ifOutSwMassUcastPkts;		//len > MASS_PACKET_SIZE
    uint32 ifOutSwMulticastPkts;
    uint32 ifOutSwMassMulticastPkts;	//len > MASS_PACKET_SIZE
    uint32 ifOutSwBroadcastPkts;
    uint32 ifOutSwMassBroadcastPkts;	//len > MASS_PACKET_SIZE
}rtk_rg_port_mib_info_t;

/* Priority to queue mapping */
typedef struct rtk_rg_qos_pri2queue_s
{
	 uint32 pri2queue[RTK_MAX_NUM_OF_PRIORITY];
}rtk_rg_qos_pri2queue_t;

/* Weight of each priority source */
typedef struct rtk_rg_qos_priSelWeight_s
{
    uint32 weight_of_portBased;
    uint32 weight_of_dot1q;
    uint32 weight_of_dscp;
    uint32 weight_of_acl;
    uint32 weight_of_lutFwd;
    uint32 weight_of_saBaed;
    uint32 weight_of_vlanBased;
    uint32 weight_of_svlanBased;
    uint32 weight_of_l4Based;
}rtk_rg_qos_priSelWeight_t;

/* Types of DSCP remarking source */
typedef enum rtk_rg_qos_dscpRmkSrc_e
{
    RTK_RG_DSCP_RMK_SRC_INT_PRI,
    RTK_RG_DSCP_RMK_SRC_DSCP,
    RTK_RG_DSCP_RMK_SRC_END
} rtk_rg_qos_dscpRmkSrc_t;

/* DOS Port Security*/
typedef enum rtk_rg_dos_type_e
{
    RTK_RG_DOS_DAEQSA_DENY = 0,
    RTK_RG_DOS_LAND_DENY,
    RTK_RG_DOS_BLAT_DENY,
    RTK_RG_DOS_SYNFIN_DENY,
    RTK_RG_DOS_XMA_DENY,
    RTK_RG_DOS_NULLSCAN_DENY,
    RTK_RG_DOS_SYN_SPORTL1024_DENY,
    RTK_RG_DOS_TCPHDR_MIN_CHECK,
    RTK_RG_DOS_TCP_FRAG_OFF_MIN_CHECK,
    RTK_RG_DOS_ICMP_FRAG_PKTS_DENY,
    RTK_RG_DOS_POD_DENY,
    RTK_RG_DOS_UDPBOMB_DENY,
    RTK_RG_DOS_SYNWITHDATA_DENY,
    RTK_RG_DOS_SYNFLOOD_DENY,		//13
    RTK_RG_DOS_FINFLOOD_DENY,
    RTK_RG_DOS_ICMPFLOOD_DENY,
    RTK_RG_DOS_TYPE_MAX
}rtk_rg_dos_type_t;

typedef enum rtk_rg_dosFlood_Type_e
{
    RTK_RG_DOSFLOOD_ICMP,
   	RTK_RG_DOSFLOOD_SYN,
    RTK_RG_DOSFLOOD_FIN,
    RTK_RG_DOSFLOOD_TYPE_MAX
} rtk_rg_dosFlood_Type_t;

typedef enum rtk_rg_dos_action_e
{
	RTK_RG_DOS_ACTION_FORWARD = 0,
    RTK_RG_DOS_ACTION_DROP,
    RTK_RG_DOS_ACTION_TRAP,
}rtk_rg_dos_action_t;

typedef enum rtk_rg_cputagpppoe_action_e
{
    RTK_RG_CPUTAG_PPPOEACT_KEEP = 0,	// tagif=1: keep; tagif=0: keep
    RTK_RG_CPUTAG_PPPOEACT_ADD, 		// tagif=1: keep; tagif=0: add
    RTK_RG_CPUTAG_PPPOEACT_REMOVE,	// tagif=1: remove; tagif=0: keep
    RTK_RG_CPUTAG_PPPOEACT_MODIFY,	// tagif=1: modify; tagif=0: add
    RTK_RG_CPUTAG_PPPOEACT_END
}rtk_rg_cputagpppoe_action_t;

typedef enum rtk_rg_meter_type_e
{
    RTK_RG_METER_ACL = 0,                   // acl policing
    RTK_RG_METER_HOSTPOL,               // host policing and logging
    RTK_RG_METER_STORMCTL,              // storm filtering control
    RTK_RG_METER_PROC,                  // meters that used for proc rate limit
    RTK_RG_METER_MAX,
}rtk_rg_meter_type_t;

typedef enum rtk_rg_meterMode_e
{
	RTK_RG_METERMODE_NOT_INIT = 0,
	RTK_RG_METERMODE_HW_INDEX, //funcbased meter disabled
	RTK_RG_METERMODE_SW_INDEX, //funcbased meter enabled
}rtk_rg_meterMode_t;


typedef struct rtk_rg_funcbasedMeterConf_s
{
    rtk_rg_meter_type_t type;
    uint32 idx;
    rtk_rg_enable_t state;
    rtk_rg_enable_t ifgInclude;
    uint32 rate;
	rtk_rate_metet_mode_t meterMode;
}rtk_rg_funcbasedMeterConf_t;

typedef struct rtk_rg_table_highPriPatten_e
{
	uint32 isIpv6:1;
	uint32 careSipEn:1;
	uint32 careDipEn:1;
	uint32 careSportEn:1;
	uint32 careDportEn:1;
	uint32 careL4ProtoEn:1;
	uint32 assignFlowPriEn:1;
	uint32 hp_sip[4];
	uint32 hp_dip[4];
	uint16 hp_sport;
	uint16 hp_dport;
	uint8  hp_tcpudp;			//0:UDP 1:TCP
	uint8  hp_flowPri;
}rtk_rg_table_highPriPatten_t;

typedef struct rtk_rg_table_swHighPriPatten_e
{
	uint32 valid:1;
	uint32 hiPriRefCnt;
	rtk_rg_table_highPriPatten_t highPriPatten;

}rtk_rg_table_swHighPriPatten_t;


typedef struct rtk_rg_funcbasedMeter_s
{
		rtk_rg_enable_t state;
		int32 hwIdx;
}rtk_rg_funcbasedMeter_t;


#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
typedef struct rtk_rg_shareMeterState_s
{
		uint32 used:1;
		//rtk_rg_meter_type_t usedFuncbasedMeterType; // this L2 sharemeter is used by which type of funcbasedMeter
}rtk_rg_shareMeterState_t;

typedef struct rtk_rg_downstream_loopback_s
{
	rtk_rg_enable_t valid;
	uint16 igr_port;	// untag bridge wan port
	uint16 loopback_port;
	uint16 ctag_vid;	// untag bridge wan cvid
}rtk_rg_downstream_loopback_t;
#endif

#if defined(CONFIG_RG_G3_SERIES)
typedef enum rtk_rg_g3_pilicer_group_e
{
	RTK_RG_G3_POLICER_GROUP_FLOWMTR = 0,//flow policer used for flow meter
	RTK_RG_G3_POLICER_GROUP_FLOWMIB,	//flow policer used for flow mib
	RTK_RG_G3_POLICER_GROUP_HOSTPOLMTR,	//flow policer used for host policing meter
	RTK_RG_G3_POLICER_GROUP_HPLOGRX,	//flow policer used for host policing pure rx logging
	RTK_RG_G3_POLICER_GROUP_HPLOGTX,	//flow policer used for host policing pure tx logging
	RTK_RG_G3_POLICER_GROUP_STORMCTL,	//flow policer used for storm control
	RTK_RG_G3_POLICER_GROUP_PROC,		//flow policer used for proc rate limit
	RTK_RG_G3_POLICER_GROUP_MAX,
}rtk_rg_g3_pilicer_group_t;
#endif

typedef struct rtk_rg_timer_s
{
	unsigned long int expires; 
	int valid;
	void (*function)(unsigned long); //register timer callback function
	unsigned long data;
} rtk_rg_timer_t;

typedef struct rtk_rg_timer_table_s
{
	rtk_rg_timer_t *pRgTimer;
	struct rtk_rg_timer_table_s *pNext;
} rtk_rg_timer_table_t;

typedef struct rtk_rg_netifMib_entry_s
{
	uint32 netifIdx;
	uint32 in_intf_uc_packet_cnt;
	uint64 in_intf_uc_byte_cnt;
	uint32 in_intf_mc_packet_cnt;
	uint64 in_intf_mc_byte_cnt;
	uint32 in_intf_bc_packet_cnt;
	uint64 in_intf_bc_byte_cnt;
	uint32 out_intf_uc_packet_cnt;
	uint64 out_intf_uc_byte_cnt;
	uint32 out_intf_mc_packet_cnt;
	uint64 out_intf_mc_byte_cnt;
	uint32 out_intf_bc_packet_cnt;
	uint64 out_intf_bc_byte_cnt;
}rtk_rg_netifMib_entry_t;

typedef enum rtk_rg_pkt_type_e
{
	RTK_RG_PKT_TYPE_UC=0,
	RTK_RG_PKT_TYPE_MC=1,
	RTK_RG_PKT_TYPE_BC=2,
	RTK_RG_PKT_TYPE_MASS_UC=4,
	RTK_RG_PKT_TYPE_MASS_MC=5,
	RTK_RG_PKT_TYPE_MASS_BC=6,
	RTK_RG_PKT_TYPE_MAX=7
} rtk_rg_pkt_type_t;

typedef struct rtk_rg_mibUpdate_entry_s
{
	rtk_rg_pkt_type_t pkt_type;
	int16 srcIntfIdx;
	int16 dstIntfIdx;
	uint16 igrPktLen;
	uint16 egrPktLen;
	
	int16 dstHostIdx;
	uint16 egrHostPktLen;

	rtk_rg_mac_port_idx_t	egrMacPort;
}rtk_rg_mibUpdate_entry_t;

typedef enum rtk_rg_smp_work_state_e
{
	SMP_WORK_STATE_FREE,
	SMP_WORK_STATE_SCHED,
}rtk_rg_smp_work_state_t;

typedef struct rtk_rg_smp_nicTx_private_s
{
	struct sk_buff *skb;
	struct tx_info txInfo;
	uint32 ring_num;
	rtk_rg_port_idx_t ingressPort;
	rtk_rg_mibUpdate_entry_t mibUpdateInfo;
}__attribute__((packed)) rtk_rg_smp_nicTx_private_t;

typedef struct rtk_rg_smp_wifiTx_private_s
{
	struct sk_buff *skb;
	struct net_device *dev;
	rtk_rg_mibUpdate_entry_t mibUpdateInfo;
}__attribute__((packed)) rtk_rg_smp_wifiTx_private_t;

typedef struct rtk_rg_shaping_bucket_s
{
	uint8 wifiTx;		// 1:to wifi; 0:to nic
	uint16 skbLen;
	union{
		rtk_rg_smp_nicTx_private_t nicTxPriv;
		rtk_rg_smp_wifiTx_private_t wifiTxPriv;
	};
	atomic_t state;		// rtk_rg_smp_work_state_t
}__attribute__((packed))rtk_rg_shaping_bucket_t;

typedef struct rtk_rg_shaping_ctrl_s
{
	//spinlock_t lock;
	atomic_t free_idx;
	atomic_t sched_idx;
	atomic_t congestion;
#ifdef __KERNEL__	
#ifdef TIMER_AGG
	rtk_rg_timer_t kicktxtimer;
#else
	struct timer_list kicktxtimer;
#endif
#endif
	uint32 enququcnt;
	uint32 pausetxcnt;
	rtk_rg_shaping_bucket_t queue[MAX_SW_SHAPER_QUEUE_SIZE];
	uint32 timerCnt;
}rtk_rg_shaping_ctrl_t;

typedef struct rtk_rg_meterCount_s
{
	uint32 packetCount;
	uint32 byteCount;
}rtk_rg_meterCount_t;

typedef struct rtk_rg_meterConf_s
{
	rtk_rate_metet_mode_t meterMode;
	rtk_rg_enable_t ifgInclude;
	uint32 rate;
	rtk_rg_meterCount_t *meterCount; //per RTK_RG_SWRATELIMIT_JIFFIES/CONFIG_HZ second
	unsigned long int lastJiffies;
	rtk_rg_meterCount_t meterCountPersec;//per 1 second
	unsigned long int lastJiffiesPersec;
}rtk_rg_meterConf_t;

typedef struct rtk_rg_systemMeterTable_s
{
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	rtk_rg_meterConf_t flowMeter[FLOWBASED_TABLESIZE_SHAREMTR]; //flow-based meter
	rtk_rg_meterCount_t flowMeterCounterTable[FLOWBASED_TABLESIZE_SHAREMTR]; //counter for flow-based meter
#endif
	rtk_rg_meterConf_t hwMeter[MAX_HW_SHAREMETER_TABLE_SIZE]; //switch meter
	rtk_rg_meterCount_t hwMeterCounterTable[MAX_HW_SHAREMETER_TABLE_SIZE];//counter for switch meter and pure SW meter
	rtk_rg_meterConf_t swMeter[PURE_SW_SHAREMETER_TABLE_SIZE]; //pure SW meter
	rtk_rg_meterCount_t swMeterCounterTable[PURE_SW_SHAREMETER_TABLE_SIZE];//counter for switch meter and pure SW meter
	//software shpaing rate control
	rtk_rg_shaping_ctrl_t shapingCtrl[MAX_SW_SHAPER_TABLE_SIZE];
}rtk_rg_systemMeterTable_t;


/* End of RTK RG API ====================================================== */

/* NIC ==================================================================== */
typedef struct rtk_rg_rxdesc_s{
#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES)
	union{
		struct{
			uint32 own:1;//31
			uint32 eor:1;//30
			uint32 fs:1;//29
			uint32 ls:1;//28
			uint32 crcerr:1;//27
			uint32 ipv4csf:1;//26
			uint32 l4csf:1;//25
			uint32 rcdf:1;//24
			uint32 ipfrag:1;//23
			uint32 pppoetag:1;//22
			uint32 rwt:1;//21
			uint32 pkttype:4;//17~20
			uint32 l3routing:1;//16
			uint32 origformat:1;//15
			uint32 pctrl:1;//14
		#if defined(CONFIG_RG_RTL9602C_SERIES)
			uint32 issb:1;//13
			uint32 rsvd:1;//12
		#else
			uint32 rsvd:2;//12~13
		#endif
			uint32 data_length:12;//0~11
		}bit;
		uint32 dw;//double word
	}opts1;
	uint32 addr;
	union{
		struct{
			uint32 cputag:1;//31
			uint32 ptp_in_cpu_tag_exist:1;//30
			uint32 svlan_tag_exist:1;//29
			uint32 rsvd_2:2;//27~28 ----> used for software: 0:Hit ACL trap to Protocol-Stack rule, 1:Other
			uint32 pon_stream_id:7;//20~26
			uint32 rsvd_1:3;//17~19
			uint32 ctagva:1;//16
			uint32 cvlan_tag:16;//0~15
		}bit;
		uint32 dw;//double word
	}opts2;
	union{
		struct{
#if defined(CONFIG_RG_RTL9602C_SERIES)
			uint32 src_port_num:4;//28~31
			uint32 dst_port_mask:7;//21~27
#else
			uint32 src_port_num:5;//27~31
			uint32 dst_port_mask:6;//21~26
#endif
			uint32 reason:8;//13~20
			uint32 internal_priority:3;//10~12
			uint32 ext_port_ttl_1:5;//5~9
			uint32 rsvd:5;//0~4
		}bit;
		uint32 dw;//double word
	}opts3;

	#define rx_fs					opts1.bit.fs
	#define rx_ls					opts1.bit.ls
	#define rx_l3routing			opts1.bit.l3routing
	#define rx_origformat			opts1.bit.origformat
#if defined(CONFIG_RG_RTL9602C_SERIES)
	#define rx_issb					opts1.bit.issb
#endif
	#define rx_pctrl				opts1.bit.pctrl
	#define rx_data_length			opts1.bit.data_length

	#define rx_cputag				opts2.bit.cputag
	#define rx_pon_stream_id		opts2.bit.pon_stream_id
	#define rx_igrLocation			opts2.bit.rsvd_1
	#define rx_ctagva				opts2.bit.ctagva
	#define rx_cvlan_tag			opts2.bit.cvlan_tag

	#define rx_src_port_num			opts3.bit.src_port_num
	#define rx_dst_port_mask		opts3.bit.dst_port_mask
	#define rx_reason				opts3.bit.reason
	#define rx_internal_priority	opts3.bit.internal_priority
	#define rx_netIfIdx				opts3.bit.rsvd
//----------------------------------------------------------------
#elif defined(CONFIG_RG_RTL9607C_SERIES)
	union{
		struct{
			uint32 own:1;//31
			uint32 eor:1;//30
			uint32 fs:1;//29
			uint32 ls:1;//28
			uint32 crcerr:1;//27
			uint32 ipv4csf:1;//26
			uint32 l4csf:1;//25
			uint32 rcdf:1;//24
			uint32 ipfrag:1;//23
			uint32 pppoetag:1;//22
			uint32 rwt:1;//21
			uint32 rsvd0:2;//19~20
			uint32 rsvd1:5;//14~18
#ifdef CONFIG_RG_JUMBO_FRAME
			uint32 data_length:14;//0~13
#else
			uint32 rsvd2:2;//12~13
			uint32 data_length:12;//0~11
#endif
		}bit;
		uint32 dw;//double word
	}opts1;
	uint32 addr;
	union{
		struct{
			uint32 cputag:1;//31
			uint32 ptp_in_cpu_tag_exist:1;//30
			uint32 svlan_tag_exist:1;//29
			uint32 reason:8;//21~28
			uint32 rsvd_1:4;//17~20
			uint32 ctagva:1;//16
			uint32 cvlan_tag:16;//0~15
		}bit;
		uint32 dw;//double word
	}opts2;
	union{
		struct{
			uint32 internal_priority:3;//29~31
			uint32 pon_sid_or_extspa:7;//22~28 or 26~28
			uint32 l3routing:1;//21
			uint32 origformat:1;//20
			uint32 src_port_num:4;//16~19
			uint32 fbi:1;//15
			uint32 fb_hash_or_dst_portmsk:15;//0~14 or 0~6
		}bit;
		struct{
			uint32 internal_priority:3;//29~31
			uint32 extspa:3;//26~28
			uint32 rsvd_1:4;//22~26
			uint32 l3routing:1;//21
			uint32 origformat:1;//20
			uint32 src_port_num:4;//16~19
			uint32 fbi:1;//15
			uint32 fb_hash_or_dst_portmsk:15;//0~14 or 0~6
		}bit1;
		uint32 dw;//double word
	}opts3;

	#define rx_fs						opts1.bit.fs
	#define rx_ls						opts1.bit.ls
	#define rx_netIfIdx					opts1.bit.rsvd1
	#define rx_data_length				opts1.bit.data_length
	#define rx_gmac						opts1.bit.rsvd0
	#define rx_ipcsf					opts1.bit.ipv4csf
	#define rx_l4csf					opts1.bit.l4csf

	#define rx_cputag					opts2.bit.cputag
	#define rx_reason					opts2.bit.reason
	#define rx_igrLocation				opts2.bit.rsvd_1
	#define rx_ctagva					opts2.bit.ctagva
	#define rx_cvlan_tag				opts2.bit.cvlan_tag

	#define rx_internal_priority		opts3.bit.internal_priority
	#define rx_l3routing				opts3.bit.l3routing
	#define rx_origformat				opts3.bit.origformat
	#define rx_src_port_num				opts3.bit.src_port_num
	#define rx_fbi						opts3.bit.fbi
	#define rx_fb_hash_or_dst_portmsk	opts3.bit.fb_hash_or_dst_portmsk

	#define rx_pon_stream_id			opts3.bit.pon_sid_or_extspa
	#define rx_extspa					opts3.bit1.extspa
	#define rx_fb_hash					rx_fb_hash_or_dst_portmsk
	#define rx_dst_port_mask			rx_fb_hash_or_dst_portmsk
//----------------------------------------------------------------
#elif defined(CONFIG_RG_RTL9603CVD_SERIES)
	// TODO: fixme
	union{
		struct{
			uint32 own:1;//31
			uint32 eor:1;//30
			uint32 fs:1;//29
			uint32 ls:1;//28
			uint32 crcerr:1;//27
			uint32 ipv4csf:1;//26
			uint32 l4csf:1;//25
			uint32 rcdf:1;//24
			uint32 ipfrag:1;//23
			uint32 pppoetag:1;//22
			uint32 rwt:1;//21
			uint32 rsvd0:1;//20
			uint32 rsvd1:6;//14~19
#ifdef CONFIG_RG_JUMBO_FRAME
			uint32 data_length:14;//0~13
#else
			uint32 rsvd2:2;//12~13
			uint32 data_length:12;//0~11
#endif
		}bit;
		uint32 dw;//double word
	}opts1;
	uint32 addr;
	union{
		struct{
			uint32 cputag:1;//31
			uint32 ptp_in_cpu_tag_exist:1;//30
			uint32 svlan_tag_exist:1;//29
			uint32 reason:8;//21~28
			uint32 rsvd_1:4;//17~20
			uint32 ctagva:1;//16
			uint32 cvlan_tag:16;//0~15
		}bit;
		uint32 dw;//double word
	}opts2;
	union{
		struct{
			uint32 internal_priority:3;//29~31
			uint32 pon_sid_or_extspa:7;//22~28 or 26~28
			uint32 l3routing:1;//21
			uint32 origformat:1;//20
			uint32 src_port_num:4;//16~19
			uint32 fbi:1;//15
			uint32 fb_hash_or_dst_portmsk:15;//0~14 or 0~6
		}bit;
		struct{
			uint32 internal_priority:3;//29~31
			uint32 extspa:3;//26~28
			uint32 rsvd_1:4;//22~26
			uint32 l3routing:1;//21
			uint32 origformat:1;//20
			uint32 src_port_num:4;//16~19
			uint32 fbi:1;//15
			uint32 fb_hash_or_dst_portmsk:15;//0~14 or 0~6
		}bit1;
		uint32 dw;//double word
	}opts3;

	#define rx_fs						opts1.bit.fs
	#define rx_ls						opts1.bit.ls
	#define rx_netIfIdx					opts1.bit.rsvd1
	#define rx_data_length				opts1.bit.data_length
	#define rx_gmac						opts1.bit.rsvd0

	#define rx_cputag					opts2.bit.cputag
	#define rx_reason					opts2.bit.reason
	#define rx_igrLocation				opts2.bit.rsvd_1
	#define rx_ctagva					opts2.bit.ctagva
	#define rx_cvlan_tag				opts2.bit.cvlan_tag

	#define rx_internal_priority		opts3.bit.internal_priority
	#define rx_l3routing				opts3.bit.l3routing
	#define rx_origformat				opts3.bit.origformat
	#define rx_src_port_num				opts3.bit.src_port_num
	#define rx_fbi						opts3.bit.fbi
	#define rx_fb_hash_or_dst_portmsk	opts3.bit.fb_hash_or_dst_portmsk

	#define rx_pon_stream_id			opts3.bit.pon_sid_or_extspa
	#define rx_extspa					opts3.bit1.extspa
	#define rx_fb_hash					rx_fb_hash_or_dst_portmsk
	#define rx_dst_port_mask			rx_fb_hash_or_dst_portmsk
//----------------------------------------------------------------
#elif defined(CONFIG_RG_G3_SERIES)
	union{
		struct{
			uint32 own:1;//31
			uint32 eor:1;//30
			uint32 fs:1;//29
			uint32 ls:1;//28
			uint32 crcerr:1;//27
			uint32 ipv4csf:1;//26
			uint32 l4csf:1;//25
			uint32 rcdf:1;//24
			uint32 ipfrag:1;//23
			uint32 pppoetag:1;//22
			uint32 rwt:1;//21
			uint32 rsvd0:1;//20
			uint32 rsvd1:6;//14~19
#ifdef CONFIG_RG_JUMBO_FRAME
			uint32 data_length:14;//0~13
#else
			uint32 rsvd2:2;//12~13
			uint32 data_length:12;//0~11
#endif
		}bit;
		uint32 dw;//double word
	}opts1;
	uint32 addr;
	union{
		struct{
			uint32 cputag:1;//31
			uint32 ptp_in_cpu_tag_exist:1;//30
			uint32 svlan_tag_exist:1;//29
			uint32 reason:8;//21~28
			uint32 rsvd_1:4;//17~20
			uint32 ctagva:1;//16
			uint32 cvlan_tag:16;//0~15
		}bit;
		uint32 dw;//double word
	}opts2;
	union{
		struct{
			uint32 internal_priority:3;//29~31
			uint32 pon_sid_or_extspa:7;//22~28 or 26~28
			uint32 l3routing:1;//21
			uint32 origformat:1;//20
			uint32 src_port_num:6;			// 4->6 for G3
			uint32 fbi:1;
			uint32 fb_hash_or_dst_portmsk:13;
		}bit;
		struct{
			uint32 internal_priority:3;//29~31
			uint32 extspa:7;				// 3->7 for G3
			uint32 l3routing:1;
			uint32 origformat:1;
			uint32 src_port_num:6;			// 4->6 for G3
			uint32 fbi:1;
			uint32 fb_hash_or_dst_portmsk:13;
		}bit1;
		uint32 dw;//double word
	}opts3;

	#define rx_fs						opts1.bit.fs
	#define rx_ls						opts1.bit.ls
	#define rx_netIfIdx					opts1.bit.rsvd1
	#define rx_data_length				opts1.bit.data_length
	#define rx_gmac						opts1.bit.rsvd0

	#define rx_cputag					opts2.bit.cputag
	#define rx_reason					opts2.bit.reason
	#define rx_igrLocation				opts2.bit.rsvd_1
	#define rx_ctagva					opts2.bit.ctagva
	#define rx_cvlan_tag				opts2.bit.cvlan_tag

	#define rx_internal_priority		opts3.bit.internal_priority
	#define rx_l3routing				opts3.bit.l3routing
	#define rx_origformat				opts3.bit.origformat
	#define rx_src_port_num				opts3.bit.src_port_num
	#define rx_fbi						opts3.bit.fbi
	#define rx_fb_hash_or_dst_portmsk	opts3.bit.fb_hash_or_dst_portmsk

	#define rx_pon_stream_id			opts3.bit.pon_sid_or_extspa
	#define rx_extspa					opts3.bit1.extspa
	#define rx_fb_hash					rx_fb_hash_or_dst_portmsk
	#define rx_dst_port_mask			rx_fb_hash_or_dst_portmsk
//----------------------------------------------------------------
#else
#error
#endif
}rtk_rg_rxdesc_t;

typedef struct rtk_rg_txdesc_s{
#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES)
	union{
		struct{
			uint32 own:1;//31
			uint32 eor:1;//30
			uint32 fs:1;//29
			uint32 ls:1;//28
			uint32 ipcs:1;//27
			uint32 l4cs:1;//26
			uint32 keep:1;//25
			uint32 blu:1;//24
			uint32 crc:1;//23
			uint32 vsel:1;//22
			uint32 dislrn:1;//21
			uint32 cputag_ipcs:1;//20
			uint32 cputag_l4cs:1;//19
			uint32 cputag_psel:1;//18
			uint32 rsvd:1;//17
			uint32 data_length:17;//0~16
		}bit;
		uint32 dw;//double word
	}opts1;
	uint32 addr;
	union{
		struct{
			uint32 cputag:1;//31
			uint32 aspri:1;//30
			uint32 cputag_pri:3;//27~29
			uint32 tx_vlan_action:2;//25~26
			uint32 tx_pppoe_action:2;//23~24
			uint32 tx_pppoe_idx:3;//20~22
			uint32 efid:1;//19
			uint32 enhance_fid:3;//16~18
			uint32 vidl:8;//8~15
			uint32 prio:3;//5~7
			uint32 cfi:1;// 4
			uint32 vidh:4;//0~3
		}bit;
		uint32 dw;//double word
	}opts2;
	union{
		struct{
			uint32 extspa:3;//29~31
			uint32 tx_portmask:6;//23~28
			uint32 tx_dst_stream_id:7;//16~22
#if defined(CONFIG_RG_RTL9602C_SERIES)
			uint32 rsvd:12;// 4~15
			uint32 rsv1:1;// 3
			uint32 rsv0:1;// 2
			uint32 l34_keep:1;// 1
			uint32 ptp:1;//0
#else
			uint32 reserved:14; // 2~15
			uint32 l34_keep:1;// 1
			uint32 PTP:1;//0
#endif
		}bit;
		uint32 dw;//double word
	}opts3;
	union{
		uint32 dw;
	}opts4;

	#define tx_fs					opts1.bit.fs
	#define tx_ls					opts1.bit.ls
	#define tx_ipcs					opts1.bit.ipcs
	#define tx_l4cs					opts1.bit.l4cs
	#define tx_keep					opts1.bit.keep
	#define tx_dislrn				opts1.bit.dislrn
	#define tx_cputag_ipcs			opts1.bit.cputag_ipcs
	#define tx_cputag_l4cs			opts1.bit.cputag_l4cs
	#define tx_cputag_psel			opts1.bit.cputag_psel
	#define tx_data_length			opts1.bit.data_length

	#define tx_cputag				opts2.bit.cputag
	#define tx_aspri				opts2.bit.aspri
	#define tx_cputag_pri			opts2.bit.cputag_pri
	#define tx_tx_vlan_action		opts2.bit.tx_vlan_action
	#define tx_tx_pppoe_action		opts2.bit.tx_pppoe_action
	#define tx_tx_pppoe_idx			opts2.bit.tx_pppoe_idx
	#define tx_vidl					opts2.bit.vidl
	#define tx_prio					opts2.bit.prio
	#define tx_cfi					opts2.bit.cfi
	#define tx_vidh					opts2.bit.vidh

	#define tx_extspa				opts3.bit.extspa
	#define tx_tx_portmask			opts3.bit.tx_portmask
	#define tx_tx_dst_stream_id		opts3.bit.tx_dst_stream_id
	#define tx_l34_keep				opts3.bit.l34_keep

	#define tx_tx_cvlan_action		tx_tx_vlan_action
	#define tx_cvlan_vidl			tx_vidl
	#define tx_cvlan_prio			tx_prio
	#define tx_cvlan_cfi			tx_cfi
	#define tx_cvlan_vidh			tx_vidh
//----------------------------------------------------------------
#elif defined(CONFIG_RG_RTL9607C_SERIES)
	union{
		struct{
			uint32 own:1;//31
			uint32 eor:1;//30
			uint32 fs:1;//29
			uint32 ls:1;//28
			uint32 ipcs:1;//27
			uint32 l4cs:1;//26
			uint32 tpid_sel:1;//25
			uint32 stag_aware:1;//24
			uint32 crc:1;//23
			uint32 rsvd:6;//17~22
			uint32 data_length:17;//0~16
		}bit;
		uint32 dw;//double word
	}opts1;
	uint32 addr;
	union{
		struct{
			uint32 cputag:1;//31
			uint32 tx_svlan_action:2;//29~30
			uint32 tx_cvlan_action:2;//27~28
			uint32 tx_portmask:11;//16~26
			uint32 cvlan_vidl:8;//8~15
			uint32 cvlan_prio:3;//5~7
			uint32 cvlan_cfi:1;// 4
			uint32 cvlan_vidh:4;//0~3
		}bit;
		uint32 dw;//double word
	}opts2;
	union{
		struct{
			uint32 rsvd1:4;//28~31
			uint32 aspri:1;//27
			uint32 cputag_pri:3;//24~26
			uint32 keep:1;//23
			uint32 rsvd2:1;//22
			uint32 dislrn:1;//21
			uint32 cputag_psel:1;//20
			uint32 gmac_id:2;//18~19 //software used for gmac_tx_idx(0:gmac9, 1:gmac10, 2:gmac7)
			uint32 l34_keep:1;//17
			uint32 rsvd3:1;//16
			uint32 extspa:3;//13~15
			uint32 tx_pppoe_action:2;//11~12
			uint32 tx_pppoe_idx:4;//7~10
			uint32 tx_dst_stream_id:7;//0~6
		}bit;
		uint32 dw;//double word
	}opts3;
	union{
		struct{
			uint32 lgsen:1;//31
			uint32 lgmtu:11;//20~30
			uint32 rsvd:4;//16~19
			uint32 svlan_vidl:8;//8~15
			uint32 svlan_prio:3;//5~7
			uint32 svlan_cfi:1;// 4
			uint32 svlan_vidh:4;//0~3
		}bit;
		uint32 dw;//double word
	}opts4;

	#define tx_fs					opts1.bit.fs
	#define tx_ls					opts1.bit.ls
	#define tx_ipcs					opts1.bit.ipcs
	#define tx_l4cs					opts1.bit.l4cs
	#define tx_tpid_sel				opts1.bit.tpid_sel
	#define tx_stag_aware			opts1.bit.stag_aware
	#define tx_data_length			opts1.bit.data_length

	#define tx_cputag				opts2.bit.cputag
	#define tx_tx_svlan_action		opts2.bit.tx_svlan_action
	#define tx_tx_cvlan_action		opts2.bit.tx_cvlan_action
	#define tx_tx_portmask			opts2.bit.tx_portmask
	#define tx_cvlan_vidl			opts2.bit.cvlan_vidl
	#define tx_cvlan_prio			opts2.bit.cvlan_prio
	#define tx_cvlan_cfi			opts2.bit.cvlan_cfi
	#define tx_cvlan_vidh			opts2.bit.cvlan_vidh

	#define tx_aspri				opts3.bit.aspri
	#define tx_cputag_pri			opts3.bit.cputag_pri
	#define tx_keep					opts3.bit.keep
	#define tx_dislrn				opts3.bit.dislrn
	#define tx_cputag_psel			opts3.bit.cputag_psel
	#define tx_l34_keep				opts3.bit.l34_keep				//direct tx
	#define tx_gmac_id				opts3.bit.gmac_id				//software used for gmac_tx_idx(0:gmac9, 1:gmac10)
	#define tx_extspa				opts3.bit.extspa
	#define tx_tx_pppoe_action		opts3.bit.tx_pppoe_action
	#define tx_tx_pppoe_idx			opts3.bit.tx_pppoe_idx
	#define tx_tx_dst_stream_id		opts3.bit.tx_dst_stream_id

	#define tx_svlan_vidl			opts4.bit.svlan_vidl
	#define tx_svlan_prio			opts4.bit.svlan_prio
	#define tx_svlan_cfi			opts4.bit.svlan_cfi
	#define tx_svlan_vidh			opts4.bit.svlan_vidh
//----------------------------------------------------------------
#elif defined(CONFIG_RG_RTL9603CVD_SERIES)
	union{
		struct{
			uint32 own:1;//31
			uint32 eor:1;//30
			uint32 fs:1;//29
			uint32 ls:1;//28
			uint32 ipcs:1;//27
			uint32 l4cs:1;//26
			uint32 tpid_sel:1;//25
			uint32 stag_aware:1;//24
			uint32 crc:1;//23
			uint32 rsvd:6;//17~22
			uint32 data_length:17;//0~16
		}bit;
		uint32 dw;//double word
	}opts1;
	uint32 addr;
	union{
		struct{
			uint32 cputag:1;//31
			uint32 tx_svlan_action:2;//29~30
			uint32 tx_cvlan_action:2;//27~28
			uint32 tx_portmask:11;//16~26
			uint32 cvlan_vidl:8;//8~15
			uint32 cvlan_prio:3;//5~7
			uint32 cvlan_cfi:1;// 4
			uint32 cvlan_vidh:4;//0~3
		}bit;
		uint32 dw;//double word
	}opts2;
	union{
		struct{
			uint32 rsvd1:4;//28~31
			uint32 aspri:1;//27
			uint32 cputag_pri:3;//24~26
			uint32 keep:1;//23
			uint32 rsvd2:1;//22
			uint32 dislrn:1;//21
			uint32 cputag_psel:1;//20
			uint32 rsvd3:2;//18~19
			uint32 l34_keep:1;//17
			uint32 gmac_id:1;//16  //software used for gmac_tx_idx(0:gmac9, 1:gmac10)
			uint32 extspa:3;//13~15
			uint32 tx_pppoe_action:2;//11~12
			uint32 tx_pppoe_idx:4;//7~10
			uint32 tx_dst_stream_id:7;//0~6
		}bit;
		uint32 dw;//double word
	}opts3;
	union{
		struct{
			uint32 lgsen:1;//31
			uint32 lgmtu:11;//20~30
			uint32 rsvd:4;//16~19
			uint32 svlan_vidl:8;//8~15
			uint32 svlan_prio:3;//5~7
			uint32 svlan_cfi:1;// 4
			uint32 svlan_vidh:4;//0~3
		}bit;
		uint32 dw;//double word
	}opts4;

	#define tx_fs					opts1.bit.fs
	#define tx_ls					opts1.bit.ls
	#define tx_ipcs					opts1.bit.ipcs
	#define tx_l4cs					opts1.bit.l4cs
	#define tx_tpid_sel				opts1.bit.tpid_sel
	#define tx_stag_aware			opts1.bit.stag_aware
	#define tx_data_length			opts1.bit.data_length

	#define tx_cputag				opts2.bit.cputag
	#define tx_tx_svlan_action		opts2.bit.tx_svlan_action
	#define tx_tx_cvlan_action		opts2.bit.tx_cvlan_action
	#define tx_tx_portmask			opts2.bit.tx_portmask
	#define tx_cvlan_vidl			opts2.bit.cvlan_vidl
	#define tx_cvlan_prio			opts2.bit.cvlan_prio
	#define tx_cvlan_cfi			opts2.bit.cvlan_cfi
	#define tx_cvlan_vidh			opts2.bit.cvlan_vidh

	#define tx_aspri				opts3.bit.aspri
	#define tx_cputag_pri			opts3.bit.cputag_pri
	#define tx_keep					opts3.bit.keep
	#define tx_dislrn				opts3.bit.dislrn
	#define tx_cputag_psel			opts3.bit.cputag_psel
	#define tx_l34_keep				opts3.bit.l34_keep				//direct tx
	#define tx_gmac_id				opts3.bit.gmac_id				//software used for gmac_tx_idx(0:gmac9, 1:gmac10)
	#define tx_extspa				opts3.bit.extspa
	#define tx_tx_pppoe_action		opts3.bit.tx_pppoe_action
	#define tx_tx_pppoe_idx			opts3.bit.tx_pppoe_idx
	#define tx_tx_dst_stream_id		opts3.bit.tx_dst_stream_id

	#define tx_svlan_vidl			opts4.bit.svlan_vidl
	#define tx_svlan_prio			opts4.bit.svlan_prio
	#define tx_svlan_cfi			opts4.bit.svlan_cfi
	#define tx_svlan_vidh			opts4.bit.svlan_vidh
//----------------------------------------------------------------
#elif defined(CONFIG_RG_G3_SERIES)
	union{
		struct{
			uint32 own:1;//31
			uint32 eor:1;//30
			uint32 fs:1;//29
			uint32 ls:1;//28
			uint32 ipcs:1;//27
			uint32 l4cs:1;//26
			uint32 tpid_sel:1;//25
			uint32 stag_aware:1;//24
			uint32 crc:1;//23
			uint32 rsvd:6;//17~22
			uint32 data_length:17;//0~16
		}bit;
		uint32 dw;//double word
	}opts1;
	uint32 addr;
	union{
		struct{
			uint32 cputag:1;//31
			uint32 tx_svlan_action:2;//29~30
			uint32 tx_cvlan_action:2;//27~28
			uint32 tx_portmask:11;//16~26
			uint32 cvlan_vidl:8;//8~15
			uint32 cvlan_prio:3;//5~7
			uint32 cvlan_cfi:1;// 4
			uint32 cvlan_vidh:4;//0~3
		}bit;
		uint32 dw;//double word
	}opts2;
	union{
		struct{
			uint32 extspa:4;//28~31
			uint32 aspri:1;//27
			uint32 cputag_pri:3;//24~26
			uint32 keep:1;//23
			uint32 rsvd2:1;//22
			uint32 dislrn:1;//21
			uint32 cputag_psel:1;//20
			uint32 rsvd3:2;//18~19
			uint32 l34_keep:1;//17
			uint32 gmac_id:1;//16  //software used for gmac_tx_idx(0:gmac9, 1:gmac10)
			uint32 rsvd1:3;//13~15
			uint32 tx_pppoe_action:2;//11~12
			uint32 tx_pppoe_idx:4;//7~10
			uint32 tx_dst_stream_id:7;//0~6
		}bit;
		uint32 dw;//double word
	}opts3;
	union{
		struct{
			uint32 lgsen:1;//31
			uint32 lgmtu:11;//20~30
			uint32 rsvd:2;//18~19
			uint32 isIpv6:1;//17
			uint32 isTcp:1;//16
			uint32 svlan_vidl:8;//8~15
			uint32 svlan_prio:3;//5~7
			uint32 svlan_cfi:1;// 4
			uint32 svlan_vidh:4;//0~3
		}bit;
		uint32 dw;//double word
	}opts4;

	#define tx_fs					opts1.bit.fs
	#define tx_ls					opts1.bit.ls
	#define tx_ipcs					opts1.bit.ipcs
	#define tx_l4cs					opts1.bit.l4cs
	#define tx_tpid_sel				opts1.bit.tpid_sel
	#define tx_stag_aware			opts1.bit.stag_aware
	#define tx_data_length			opts1.bit.data_length

	#define tx_cputag				opts2.bit.cputag
	#define tx_tx_svlan_action		opts2.bit.tx_svlan_action
	#define tx_tx_cvlan_action		opts2.bit.tx_cvlan_action
	#define tx_tx_portmask			opts2.bit.tx_portmask
	#define tx_cvlan_vidl			opts2.bit.cvlan_vidl
	#define tx_cvlan_prio			opts2.bit.cvlan_prio
	#define tx_cvlan_cfi			opts2.bit.cvlan_cfi
	#define tx_cvlan_vidh			opts2.bit.cvlan_vidh

	#define tx_aspri				opts3.bit.aspri
	#define tx_cputag_pri			opts3.bit.cputag_pri
	#define tx_keep					opts3.bit.keep
	#define tx_dislrn				opts3.bit.dislrn
	#define tx_cputag_psel			opts3.bit.cputag_psel
	#define tx_l34_keep				opts3.bit.l34_keep				//direct tx
	#define tx_gmac_id				opts3.bit.gmac_id				//software used for gmac_tx_idx(0:gmac9, 1:gmac10)
	#define tx_extspa				opts3.bit.extspa
	#define tx_tx_pppoe_action		opts3.bit.tx_pppoe_action
	#define tx_tx_pppoe_idx			opts3.bit.tx_pppoe_idx
	#define tx_tx_dst_stream_id		opts3.bit.tx_dst_stream_id

	#define tx_isIpv6				opts4.bit.isIpv6
	#define tx_isTcp				opts4.bit.isTcp
	#define tx_svlan_vidl			opts4.bit.svlan_vidl
	#define tx_svlan_prio			opts4.bit.svlan_prio
	#define tx_svlan_cfi			opts4.bit.svlan_cfi
	#define tx_svlan_vidh			opts4.bit.svlan_vidh
//----------------------------------------------------------------
#else
#error
#endif
}rtk_rg_txdesc_t;
/* End of NIC ============================================================= */


/* Forward Engine ========================================================= */
typedef enum rtk_rg_fwdEngineReturn_e
{

	//No Free skb [return state]
	RG_FWDENGINE_RET_DIRECT_TX			=0x0,
	RG_FWDENGINE_RET_UN_INIT			,
	RG_FWDENGINE_RET_QUEUE_FRAG			,
	RG_FWDENGINE_RET_FRAGMENT_ONE		,
	RG_FWDENGINE_RET_FRAGMENT			,
	RG_FWDENGINE_RET_NAPT_OK			,	//5
	RG_FWDENGINE_RET_L2FORWARDED		,
	RG_FWDENGINE_RET_CONTINUE			,
	RG_FWDENGINE_RET_BROADCAST			,
	RG_FWDENGINE_RET_FRAG_ONE_PS		,
	RG_FWDENGINE_RET_FRAG_ONE_DROP		,	//10
	RG_FWDENGINE_RET_HWLOOKUP			,
	RG_FWDENGINE_RET_SEND_TO_WIFI		,
	RG_FWDENGINE_RET_HIT_BINDING		,
	RG_FWDENGINE_RET_DIRECT_TX_SLOW		,
	RG_FWDENGINE_RET_ERROR				,	//15
	RG_FWDENGINE_RET_ROUTING_TRAP		,
	RG_FWDENGINE_RET_ROUTING_DROP		,
	RG_FWDENGINE_RET_SLOWPATH           ,
	RG_FWDENGINE_RET_RATE_LIMIT_DROP	,

	//To PS (Free by PS) [return Action]
	RG_FWDENGINE_RET_TO_PS				, // Do ACL action in fwdEngine_rx_skb

	//Drop [return Action]
	RG_FWDENGINE_RET_DROP				, // Free skb in fwdEngine_rx_skb
}rtk_rg_fwdEngineReturn_t;

typedef enum rtk_rg_fwdEngineAclAccTypeReturn_e
{
	RG_FWDENGINE_ACL_ACC_TYPE_TX_PKTBUFF		=0x0, //more efficiency
	RG_FWDENGINE_ACL_ACC_TYPE_TX_DESC			=0x1,
}rtk_rg_fwdEngineAclAccTypeReturn_t;

typedef enum rtk_rg_fwdEngineAlgReturn_e
{
	RG_FWDENGINE_ALG_RET_FAIL		=-1,
	RG_FWDENGINE_ALG_RET_SUCCESS	=0,
	RG_FWDENGINE_ALG_RET_DROP		=1,
}rtk_rg_fwdEngineAlgReturn_t;

typedef enum rtk_rg_successFailReturn_e
{
	RG_RET_FAIL		=-1,
	RG_RET_SUCCESS	=0,

}rtk_rg_successFailReturn_t;

typedef enum rtk_rg_naptPreRouteCallBackReturn_e
{
	RG_FWDENGINE_PREROUTECB_CONTINUE	=0,
	RG_FWDENGINE_PREROUTECB_DROP		=1,
	RG_FWDENGINE_PREROUTECB_TRAP		=2,
	RG_FWDENGINE_PREROUTECB_END			=3,
}rtk_rg_naptPreRouteCallBackReturn_t;

typedef enum rtk_rg_naptForwardCallBackReturn_e
{
	RG_FWDENGINE_FORWARDCB_FINISH_DPI	=0,		//the flow no need check anymore, add to hw
	RG_FWDENGINE_FORWARDCB_CONTINUE_DPI	=1,		//the flow has to be checked next time
	RG_FWDENGINE_FORWARDCB_DROP			=2,
	RG_FWDENGINE_FORWARDCB_SW_COUNTING	=3,		//the flow no need check but forwarded by sw for counting
	RG_FWDENGINE_FORWARDCB_END			=4,
}rtk_rg_naptForwardCallBackReturn_t;

typedef enum rtk_rg_lutIdx_return_e
{
	RG_RET_LUTIDX_FOUND  		=0, // found, return the matched lut index
	RG_RET_LUTIDX_NOT_FOUND		=1,	// not found, return a free lut index
	RG_RET_LUTIDX_NOFREE  		=2, // not found and no any free lut index
}rtk_rg_lutIdx_return_t;

typedef enum rtk_rg_lookupIdxReturn_e
{
	RG_RET_LOOKUPIDX_NOT_FOUND		=-1,
	RG_RET_LOOKUPIDX_ONLY_INBOUND_FOUND  =-2,
}rtk_rg_lookupIdxReturn_t;

typedef enum rtk_rg_extPortGetReturn_e
{
	RG_RET_EXTPORT_NOT_GET		=-1,
}rtk_rg_extPortGetReturn_t;

typedef enum rtk_rg_entryGetReturn_e
{
	RG_RET_ENTRY_NOT_GET		=-1,
}rtk_rg_entryGetReturn_t;

#if 0
/* Protocol */
struct	rg_proto_ether_header {
	unsigned char	ether_dhost[6];
	unsigned char	ether_shost[6];
	unsigned short int	ether_type;
};

struct rg_proto_iphdr {
	unsigned char	ihl_protocol;
	unsigned char	tos;
	unsigned short int	tot_len;
	unsigned short int	id;
	unsigned short int	frag_off;
	unsigned char	ttl;
	unsigned char	protocol;
	unsigned short int	check;
	unsigned int	saddr;
	unsigned int	daddr;
};

struct rg_proto_tcphdr {
	unsigned short int	source;
	unsigned short int	dest;
	unsigned int	seq;
	unsigned int	ack_seq;
	unsigned char	doff;
	unsigned char	flag;
	unsigned short int	window;
	unsigned short int	check;
	unsigned short int	urg_ptr;
};

struct rg_proto_udphdr {
	unsigned short int	source;
	unsigned short int	dest;
	unsigned short int	len;
	unsigned short int	check;
};
#endif
/* End of Forward Engine ================================================== */



/* Parser ================================================================= */
typedef enum rtk_rg_pkthdr_tagif_e
{
	SVLAN_TAGIF=(1<<0),
	CVLAN_TAGIF=(1<<1),
	PPPOE_TAGIF=(1<<2),
	IPV4_TAGIF=(1<<3),
	IPV6_TAGIF=(1<<4),
	TCP_TAGIF=(1<<5),
	UDP_TAGIF=(1<<6),
	IGMP_TAGIF=(1<<7),
	PPTP_TAGIF=(1<<8),
	GRE_TAGIF=(1<<9),
	ICMPV6_TAGIF=(1<<10),
	ICMP_TAGIF=(1<<11),
	ESP_TAGIF=(1<<12),  /*siyuan add for alg IPsec passthrough*/
	MSS_TAGIF=(1<<13),
	ARP_TAGIF=(1<<14),
	V6FRAG_TAGIF=(1<<15),
	IPV6_MLD_TAGIF=(1<<16),
	GRE_SEQ_TAGIF=(1<<17),
	GRE_ACK_TAGIF=(1<<18),
	PPTP_INNER_TAGIF=(1<<19),
	L2TP_INNER_TAGIF=(1<<20),
	L2TP_TAGIF=(1<<21),
	DSLITE_TAGIF=(1<<22),		//egress is dslite packet
	DSLITE_INNER_TAGIF=(1<<23), //ingress is dslite packet
	V6TRAP_TAGIF=(1<<24),
	DVMRP_TAGIF=(1<<25),
	MOSPF_TAGIF=(1<<26),
	PIM_TAGIF=(1<<27),
	DSLITEMC_INNER_TAGIF=(1<<28), //hit dslite Mc table
	UNKNOWN_L4_TAGIF=(1<<29),
	RTSP_NON_INTERLEAVED_FRAME_TAGIF=(1<<30),
	VXLAN_TAGIF=(1<<31),
} rtk_rg_pkthdr_tagif_t;


typedef struct rtk_rg_tcpFlags_s{

#ifdef __LITTLE_ENDIAN

	uint8	fin:1;
	uint8	syn:1;
	uint8	reset:1;
	uint8	push:1;
	uint8	ack:1;
	uint8	urg:1;
	uint8	reserved:2;

#else
	uint8	reserved:2;
	uint8	urg:1;
	uint8	ack:1;
	uint8	push:1;
	uint8	reset:1;
	uint8	syn:1;
	uint8	fin:1;

#endif
}rtk_rg_tcpFlags_t;





#if defined(CONFIG_RG_RTL9602C_SERIES) || defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)

typedef enum rtk_rg_igrAction_controlBit_e
{
	RG_IGR_SVLAN_ACT_DONE_BIT = 	(1<<0),
	RG_IGR_CVLAN_ACT_DONE_BIT = 	(1<<1),
	RG_IGR_PRIORITY_ACT_DONE_BIT = 	(1<<2),
	RG_IGR_POLICE_ACT_DONE_BIT = 	(1<<3),
	RG_IGR_FORWARD_ACT_DONE_BIT = 	(1<<4),
	RG_IGR_INTCF_ACT_DONE_BIT = 	(1<<5),
	RG_IGR_PROUTE_ACT_DONE_BIT = 	(1<<6),
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	RG_IGR_MIRROR_UDP_ENCAP_ACT_DONE_BIT = (1<<7),
	RG_IGR_FORWARD_ACT_POSTPONE_BIT = 	(1<<8),
#endif
}rtk_rg_igrAction_controlBit_t;

typedef enum rtk_rg_egrAction_controlBit_e
{
	RG_EGR_SVLAN_ACT_DONE_BIT = (1<<0),
	RG_EGR_CVLAN_ACT_DONE_BIT = (1<<1),
	RG_EGR_CFPRI_ACT_DONE_BIT = (1<<2),
	RG_EGR_DSCP_ACT_DONE_BIT = (1<<3),
	RG_EGR_FWD_ACT_DONE_BIT = (1<<4), //UNI act or Drop act
	RG_EGR_SID_ACT_DONE_BIT = (1<<5),
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	RG_EGR_MIRROR_UDP_ENCAP_ACT_DONE_BIT = (1<<6),
	RG_EGR_FLOWMIB_ACT_DONE_BIT = (1<<7),
#endif
}rtk_rg_egrAction_controlBit_t;

typedef struct rtk_rg_aclHitAndAction_s
{
	uint32 aclIgrHitMask[(MAX_ACL_SW_ENTRY_SIZE/32)+1]; //check RG_ACL  ingressPart hit or not:  1ULL & ruleNo is 0: not hit  1: hit
	uint32 aclEgrHitMask[(MAX_ACL_SW_ENTRY_SIZE/32)+1]; //check RG_ACL  egressPart hit or not:   1ULL & ruleNo is 0: not hit  1: hit
	uint32 aclEgrPattern1HitMask[(TOTAL_CF_ENTRY_SIZE/32)+1]; //check CF pattern1 hit or not:  1ULL & ruleNo is 0: not hit  1: hit   PS:aclEgrL2HitMask[0] is reserved now
	int aclEgrHaveToCheckRuleIdx[MAX_ACL_SW_ENTRY_SIZE];//record ingress part hit rule index, for speeding up egress module pattern check. (-1 means invalid index, and rest array doen't need to check)


	int aclIgrRuleChecked; //record the  _rtk_rg_ingressACLPatternCheck has called! 0:not called before.  1:it has called, it cab be pass if called twice.

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	int32 igr_encap_acl_idx;		//FAIL: not used
	rtk_rg_mirror_udp_encap_tag_action_t action_igr_encap_udp;	//keep content for flow
	int32 egr_encap_acl_idx;		//FAIL: not used
	rtk_rg_mirror_udp_encap_tag_action_t action_egr_encap_udp;	//keep content for flow
#endif

	//***********[PER PACKET CLEAR DECISION PART]*********//
	uint8 ACL_DECISION_EGR_PART_CLEAR; //[NOTE]used to clean acl egress module decision for each BC/MC data path. each decision need to clrear should be put later then this parameter!!!

	int aclIgrDoneAction; //refer to rtk_rg_igrAction_controlBit_t
	int aclEgrDoneAction; //refert to rtk_rg_egrAction_controlBit_t.

	/*RG ACL final decided actions*/
	rtk_rg_acl_action_type_t action_type;
	rtk_rg_acl_qos_action_t qos_actions; /* only used for action_type=ACL_ACTION_TYPE_QOS */

	unsigned char action_dot1p_remarking_pri;
	unsigned char action_ip_precedence_remarking_pri;
	unsigned char action_dscp_remarking_pri;
	unsigned char action_tos_tc_remarking_pri;
	unsigned char action_queue_id;
	unsigned char action_share_meter;
	unsigned char action_log_counter;
	unsigned char action_stream_id_or_llid;
	unsigned char action_acl_priority;
	unsigned char action_redirect_portmask;
	unsigned char  action_acl_egress_internal_priority;
	rtk_rg_cvlan_tag_action_t action_acl_cvlan;
	rtk_rg_svlan_tag_action_t action_acl_svlan;
	rtk_rg_uni_action_t action_acl_uni;


	/*CF64~511 final decided action*/
	rtk_rg_cf_direction_type_t direction;
	//US actions
	rtk_rg_cf_us_action_type_t us_action_field;
	//DS actions
	rtk_rg_cf_ds_action_type_t ds_action_field;

	rtk_rg_cvlan_tag_action_t action_cvlan;
	rtk_rg_svlan_tag_action_t action_svlan;
	rtk_rg_cfpri_action_t	action_cfpri;
	rtk_rg_sid_llid_action_t action_sid_or_llid;
	rtk_rg_dscp_action_t action_dscp;
	rtk_rg_uni_action_t action_uni;
	rtk_rg_fwd_action_t action_fwd;
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	unsigned char action_flowmib_counter_idx;
#endif

}rtk_rg_aclHitAndAction_t;



#else

typedef enum rtk_rg_igrAction_controlBit_e
{
	RG_IGR_MIRROR_UDP_ENCAP_ACT_DONE_BIT = (1<<7),
}rtk_rg_igrAction_controlBit_t;


typedef enum rtk_rg_egrAction_controlBit_e
{
	RG_EGR_MIRROR_UDP_ENCAP_ACT_DONE_BIT = (1<<6),
}rtk_rg_egrAction_controlBit_t;


typedef enum rtk_rg_egrAction_e
{
	EGR_CSACT_DONE = 0,
	EGR_CACT_DONE ,
	EGR_CFPRI_ACT_DONE ,
	EGR_DSCP_REMARK_DONE ,
	EGR_UNI_ACT_DONE ,
	EGR_SID_ACT_DONE ,
	EGR_DROP_ACT_DONE ,
	EGR_LOG_ACT_DONE ,
	EGR_MIRROR_UDP_ENCAP_ACT_DONE,
}rtk_rg_egrAction_t;

typedef struct rtk_rg_aclHitAndAction_s
{
	uint32 aclIgrHitMask[(MAX_ACL_SW_ENTRY_SIZE/32)+1]; //check RG_ACL 0~63 ingressPart hit or not:  1ULL & ruleNo is 0: not hit  1: hit
	uint32 aclEgrHitMask[(MAX_ACL_SW_ENTRY_SIZE/32)+1]; //check RG_ACL 0~63 egressPart hit or not:   1ULL & ruleNo is 0: not hit  1: hit
	uint32 aclEgrL2HitMask[(TOTAL_CF_ENTRY_SIZE/32)+1]; //check RG_CF 64~511 hit or not:  1ULL & ruleNo is 0: not hit  1: hit   PS:aclEgrL2HitMask[0] is reserved now
	int aclEgrHaveToCheckRuleIdx[MAX_ACL_SW_ENTRY_SIZE];//record which ingress part hit rule index, for speeding up egress pattern check. (-1 means invalid index, and rest array doen't need to check)


	int aclIgrRuleChecked; //record the  _rtk_rg_ingressACLPatternCheck has called! 0:not called before.  1:it has called, it cab be pass if called twice.
	int l34CFRuleHit; //0: none CF:0-63  rule hit.  1: at least one CF:0-63 rule hit
	uint32 l2CFHitAction;	//for CF[64-511] double hit check, forbidden the action to do again when related bit is valid. refer to struct rtk_rg_egrAction_t.
	int aclIgrDoneAction; //refer to rtk_rg_igrAction_controlBit_t
	int aclEgrDoneAction; //refert to rtk_rg_egrAction_controlBit_t.  which pure software actions is extend in later platform(ex: apolloFE or apolloPro)


	/*ACL + CF0~63 final decided action*/
	rtk_rg_acl_action_type_t action_type;
	rtk_rg_acl_qos_action_t qos_actions; /* only used for action_type=ACL_ACTION_TYPE_QOS */

	unsigned char action_dot1p_remarking_pri;
	unsigned char action_ip_precedence_remarking_pri;
	unsigned char action_dscp_remarking_pri;
	unsigned char action_tos_tc_remarking_pri;
	unsigned char action_queue_id;
	unsigned char action_share_meter;
	unsigned char action_log_counter;
	unsigned char action_stream_id_or_llid;
	unsigned char action_acl_priority;
	unsigned char action_redirect_portmask;
	unsigned char  action_acl_egress_internal_priority;
	rtk_rg_cvlan_tag_action_t action_acl_cvlan;
	rtk_rg_svlan_tag_action_t action_acl_svlan;
	rtk_rg_uni_action_t action_acl_uni;


	/*CF64~511 final decided action*/
	int cf64to511RuleHit; //0:none cf64-511 rule hit.   1:any cf64-511 rule hit.  using for speed up need to do egressACL or not.
	rtk_rg_cf_direction_type_t direction;
	//US actions
	rtk_rg_cf_us_action_type_t us_action_field;
	//DS actions
	rtk_rg_cf_ds_action_type_t ds_action_field;

	rtk_rg_cvlan_tag_action_t action_cvlan;
	rtk_rg_svlan_tag_action_t action_svlan;
	rtk_rg_cfpri_action_t	action_cfpri;
	rtk_rg_sid_llid_action_t action_sid_or_llid;
	rtk_rg_dscp_action_t action_dscp;
	rtk_rg_log_action_t action_log;
	rtk_rg_uni_action_t action_uni;

	int32 igr_encap_acl_idx;		//FAIL: not used
	rtk_rg_mirror_udp_encap_tag_action_t action_igr_encap_udp;	//keep content for flow
	int32 egr_encap_acl_idx;		//FAIL: not used
	rtk_rg_mirror_udp_encap_tag_action_t action_egr_encap_udp;	//keep content for flow
}rtk_rg_aclHitAndAction_t;
#endif

typedef struct rtk_rg_igmp_whiteList_s
{
	uint8  valid:1;
	uint8  smacChk:1;
	uint8  sipChk:1;
	uint8  dipChk:1;
	uint8  smac[ETHER_ADDR_LEN];
	uint8  smacMask[ETHER_ADDR_LEN];
	uint32	ipv4Sip;
	uint32	ipv4SipMask;
	uint32	ipv4Dip;
	uint32	ipv4DipMask;
}rtk_rg_igmp_whiteList_t;

typedef struct rtk_rg_igmp_blackList_s
{
	uint8  valid:1;
	uint8  smacChk:1;
	uint8  sipChk:1;
	uint8  dipChk:1;
	uint8  smac[ETHER_ADDR_LEN];
	uint8  smacMask[ETHER_ADDR_LEN];
	uint32	ipv4Sip;
	uint32	ipv4SipMask;
	uint32	ipv4Dip;
	uint32	ipv4DipMask;
}rtk_rg_igmp_blackList_t;


typedef enum rtk_rg_cfDirection_e
{
	CF_DOWNSTREAM,
	CF_UPSTREAM,
	CF_LAN_TO_LAN,
}rtk_rg_cfDirection_t;

typedef enum rtk_rg_algAction_e
{
	RG_ALG_ACT_NORMAL,				//non-ALG packets
	RG_ALG_ACT_TO_FWDENGINE,		//ALG handled by fwdEngine packets
	RG_ALG_ACT_TO_PS,				//ALG handled by protocol stack packets
	RG_ALG_ACT_SW_SC,				//ALG handled by shortcut packets
}rtk_rg_algAction_t;

typedef enum rtk_rg_l4Direction_e
{
	RG_L4_NOT_NAPT_FLOW=0, //non-layer4 flow packets
	RG_NAPT_OUTBOUND_FLOW,
	RG_NAPTR_INBOUND_FLOW,
}rtk_rg_l4Direction_t;

typedef enum rtk_rg_ingressLocation_e
{
	RG_IGR_PHY_PORT=0,
	RG_IGR_PROTOCOL_STACK,
	RG_IGR_ARP_OR_ND,
	RG_IGR_IGMP_OR_MLD,
	RG_IGR_MC_DATA_BUF,
	RG_IGR_DROP,
}rtk_rg_ingressLocation_t;

#if defined(CONFIG_RG_RTL9602C_SERIES) || defined(CONFIG_RG_RTL9600_SERIES)
#define FWD_DECISION_IS_MC(fwdDecision) ((fwdDecision==RG_FWD_DECISION_DSLITE_MC)||(fwdDecision==RG_FWD_DECISION_PPPOE_MC)|| (fwdDecision==RG_FWD_DECISION_NORMAL_MC))
#elif defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#define FWD_DECISION_IS_MC(fwdDecision) ((fwdDecision==RG_FWD_DECISION_FLOW_MC)||fwdDecision==RG_FWD_DECISION_FLOW_RESERVED_MC)
#else
##error
#endif

typedef enum rtk_rg_fwdDecision_e
{
	RG_FWD_DECISION_INITIAL=0,
	RG_FWD_DECISION_BRIDGING,
	RG_FWD_DECISION_ROUTING,
	RG_FWD_DECISION_V6ROUTING,
	RG_FWD_DECISION_NAPT,
	RG_FWD_DECISION_NAPTR,
	RG_FWD_DECISION_NORMAL_BC,		//broadcast or multicast
	RG_FWD_DECISION_NO_PS_BC,		//from protocol stack bc or mc, unknown DA, etc., don't go to protocol stack again
	RG_FWD_DECISION_TO_PS,			//after DA lookup, the destination port is CPU port
	RG_FWD_DECISION_V6NAPT,
	RG_FWD_DECISION_V6NAPTR,
	RG_FWD_DECISION_PPPOE_MC,		//pppoe multicast, change DA to multicast DA and remove pppoe tag
	RG_FWD_DECISION_DSLITE_MC,		//Ds-lite Multicast
	RG_FWD_DECISION_NORMAL_MC,		//multicast follow 9600_Series/9602C_series hw forward decision
	RG_FWD_DECISION_FLOW_MC,		// multicast data-flow for  defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	RG_FWD_DECISION_FLOW_RESERVED_MC,

}rtk_rg_fwdDecision_t;

typedef enum rtk_rg_bindingDecision_e
{
	RG_BINDING_NOT_FINISHED,
	RG_BINDING_FINISHED,
	RG_BINDING_LAYER2,		//layer2 will do hw lookup, therefore this case is not necessary
	RG_BINDING_LAYER3,
	RG_BINDING_LAYER4,
}rtk_rg_bindingDecision_t;

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
typedef struct rtk_rg_table_flowEntry_e
{
	union{
		rtk_rg_asic_path1_entry_t path1;
		rtk_rg_asic_path2_entry_t path2;
		rtk_rg_asic_path3_entry_t path3;
		rtk_rg_asic_path4_entry_t path4;
		rtk_rg_asic_path5_entry_t path5;
		rtk_rg_asic_path6_entry_t path6;
	};
}rtk_rg_table_flowEntry_t;


typedef struct rtk_rg_table_flow_e
{
	//1 Must update related function if the following sw fields is updated.
	//Related function: _rtk_rg_flow_add(), _rtk_rg_flow_swField_init()
	//1 word 0
	int32 arpOrNeighborIdx_src	:MAX_FLOW_ARP_IDX_WIDTH; 			// 11, for updating idle time of sip, -1 for non-arp/non-neighbor.
	uint32 idleSecs				:MAX_FLOW_IDLESEC_WIDTH; 			// 12
	uint32 ingressPort			:MAX_FLOW_SPA_WIDTH;				// 5
	uint32 staticEntry			:1;									// multicast/PATH6 enable
	uint32 canBeReplaced		:1;									// can be replaced entry
	uint32 isLocalInNapt		:1;									// local in napt/routing
	uint32 isHairpinNat			:1;									// hairpin nat
	//1 word 1
	int32 naptOrTcpUdpGroupIdx	:MAX_FLOW_NAPT_IDX_WIDTH;			// 16, -1 for non-napt  (max: 17 bits)
	uint32 isSwOnly				:1;									// Sw flow only
	uint32 isMulticast			:1;									// DIP is multicast
	uint32 isToGmac				:1;									// DMAC is Gmac
	uint32 isUsedByFrag			:1;									// referenced by L2/L3/L4 fragment connection
	uint32 needDPI				:1; 								// should be checked with DPI callback
	uint32 needUpdate			:1;									// should be deleted when this flow's idle time is not zero
	uint32 egressTagAccType		:1;									// 1: tag by Gmac,  0: tag by Fwdengine
	uint32 highPri2HwEntry		:1;									// high priority add to hw Entry
	int32 mibTagDelta			:6;									// 6: need support -16~+16 (4[ctag]+4[stag]+8[pppoe tag])
	uint32 sw_valid 			:1; 								// software valid bit
#if	(MAX_FLOW_NAPT_IDX_WIDTH<17)
	uint32 reserve1				:(32-MAX_FLOW_NAPT_IDX_WIDTH-1-1-1-1-1-1-1-1-6-1);
#endif
	//1 word 2
	uint32 smacL2Idx			:MAX_FLOW_LUT_IDX_WIDTH;			// 12
	uint32 dmacL2Idx			:MAX_FLOW_LUT_IDX_WIDTH;			// 12
	int32 srcWlanDevIdx			:MAX_FLOW_WLAN_DEVICE_IDX_WIDTH;	// 6
	uint32 byDmac2cvid			:1;									// egress cvlan is decided by dmac2cvid	
	uint32 reserve2				:1;									// padding to 32 bits
	//1 word 3
#ifndef CONFIG_RG_G3_SERIES
	int32 arpOrNeighborIdx_dst	:MAX_FLOW_ARP_IDX_WIDTH; 			// 11, for updating idle time of dip, -1 for non-arp/non-neighbor.
	uint32 isToSTCmodify		:1;									// ToS/TC should update 8 bits
	uint32 tos_tc				:8;									// ipv4 tos or ipv6 tc
	uint32 isMeterAct			:1;									// Should check pure sw share meter
	uint32 swMeterOffsetIdx		:MAX_FLOW_SW_SHAREMETER_IDX_WIDTH;	// share meter idx 0~79 - sw offset 48
	uint32 isFlowMibAct			:1;									// Should check pure sw flow mib
	uint32 swFlowMibOffsetIdx	:5;									// flow mib idx 0~63 - sw offset 32
#else	// CONFIG_RG_G3_SERIES
	int32 arpOrNeighborIdx_dst	:MAX_FLOW_ARP_IDX_WIDTH; 			// 12, for updating idle time of dip, -1 for non-arp/non-neighbor.
	int32 mainHash_hwFlowIdx	:(MAX_FLOW_HW_MAIN_HASH_WIDTH+1);	// 16+1, -1 for sw flow
	int32 hitHostPolingState	:2;
	uint32 isToSTCmodify		:1;									// ToS/TC should update 8 bits
	
	uint32 tos_tc				:8;									// ipv4 tos or ipv6 tc
	uint32 isMeterAct			:1;									// Should check pure sw share meter
	uint32 swMeterOffsetIdx		:MAX_FLOW_SW_SHAREMETER_IDX_WIDTH;	// share meter idx 0~79 - sw offset 48
	uint32 isFlowMibAct			:1;									// Should check pure sw flow mib
	uint32 swFlowMibOffsetIdx	:5;									// flow mib idx 0~63 - sw offset 32
	uint32 reserve4				:(32-8-1-5-1-1-MAX_FLOW_SW_SHAREMETER_IDX_WIDTH); // padding to 32 bits
#endif	
	//1 word 4
	uint32 isTrapToPs			:1;									// trap to ps acc
	uint32 hasTinyAckStreamId	:1;
	uint32 tinyAckStreamId		:7;
	uint32 reserve5				:(32-1-1-7); // padding to 32 bits
	
}rtk_rg_table_flow_t;

typedef struct rtk_rg_table_extport_e
{
	rtk_rg_asic_extPortMask_entry_t extPortEnt;
	int32 extPortRefCount;
}rtk_rg_table_extport_t;

typedef struct rtk_rg_table_indmac_e
{
	rtk_rg_asic_indirectMac_entry_t indMacEnt;
	int32 indMacRefCount;
}rtk_rg_table_indmac_t;

typedef struct rtk_rg_table_ethtype_e
{
	rtk_rg_asic_etherType_entry_t ethTypeEnt;
	int32 ethTypeRefCount;
}rtk_rg_table_ethtype_t;

typedef struct rtk_rg_table_wanaccesslimit_e
{
	rtk_rg_asic_wanAccessLimit_entry_t wanAllowEnt;
}rtk_rg_table_wanaccesslimit_t;
#endif

typedef enum rtk_rg_L2L3TcpUdpStatefulTracking_e
{
	RG_STATEFUL_TRACKING_DISABLE=0,
	RG_STATEFUL_TRACKING_ENABLE,
	RG_STATEFUL_TRACKING_ENABLE_FIREWALL,
}rtk_rg_L2L3TcpUdpStatefulTracking_t;
#endif

typedef struct rtk_rg_table_flowmib_e
{
	uint32 in_packet_cnt;
	uint64 in_byte_cnt;
	uint32 out_packet_cnt;
	uint64 out_byte_cnt;
}rtk_rg_table_flowmib_t;

#if defined(CONFIG_ROME_NAPT_SHORTCUT) || defined(CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT)
typedef enum rtk_rg_shortcutDecision_e
{
	RG_SC_NORMAL_PATH=0,
	RG_SC_MATCH,
	RG_SC_NEED_UPDATE,
	RG_SC_NEED_UPDATE_BEFORE_SEND,
	RG_SC_V6_NEED_UPDATE_BEFORE_SEND,
	RG_SC_STATEFUL_NEED_UPDATE_BEFORE_SEND,
}rtk_rg_shortcutDecision_t;

typedef enum rtk_rg_shortcutBitFieldWidth_e
{
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES)
	BFW_SPA				=5,
#else
	BFW_SPA				=4,
#endif
	BFW_WLANDEVIDX		=6,
	BFW_DIR				=3,
	BFW_ISTCP			=1,
	BFW_ISNAPT			=1,
	BFW_ISBRIDGE 		=1,
	BFW_ISLOCALINNAPT   =1,
	BFW_ISHAIRPINNAT 	=1,
	BFW_NOTFINUPDATE 	=1,
	BFW_VLANTAGIF		=1,
	BFW_SVLANTAGIF		=2,
	BFW_DMAC2CVLANTAGIF	=1,
	BFW_TAGACCTYPE		=1,
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	BFW_INTERVLANID		=14,
	BFW_VLANID			=14,
	BFW_DMAC2CVLANID 	=14,
	BFW_SVLANID			=14,
#else
	BFW_INTERVLANID		=13,
	BFW_VLANID			=13,
	BFW_DMAC2CVLANID 	=13,
	BFW_SVLANID			=13,
#endif
	BFW_DSCP 			=(6+1),
	BFW_PRIORITY 		=3,
	BFW_SPRIORITY		=3,
	BFW_INTERCFPRI		=3,
	BFW_STREAMID 		=7,
	BFW_SMACL2IDX		=13,
	BFW_MACPORT			=4,
	BFW_EXTPORT			=4,
	BFW_NEIGHBORIDX		=MAX_IPV6_NEIGHBOR_HASH_WAY_SHIFT+4+1,		//neighbor size calculated by hash way multiply 16 head index, plus one for negative value.
	BFW_IDLETIME 		=16,
	BFW_INTFIDX			=MAX_NETIF_SW_TABLE_SIZE_SHIFT+1,
	BFW_EIPIDX			=MAX_EXTIP_SW_TABLE_SIZE_SHIFT+1,
	BFW_LUTIDX			=LUT_HW_TABLE_SIZE_SHIFT+2,
	BFW_ARPIDX			=MAX_ARP_SW_TABLE_SIZE_SHIFT+1,
	BFW_NAPTIDX			=MAX_NAPT_IN_SW_TABLE_SHIFT+1,
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES)
	BFW_MIBDIRECT		=2,
	BFW_MIBIFIDX 		=5,
	BFW_MIBTAGDELTA		=4,
	BFW_UNIPORTMASK		=11,
#elif defined(CONFIG_RG_RTL9603CVD_SERIES)
	BFW_MIBDIRECT		=2,
	BFW_MIBIFIDX 		=5,
	BFW_MIBTAGDELTA		=4,
	BFW_UNIPORTMASK		=6,
#elif defined(CONFIG_RG_RTL9602C_SERIES)
	BFW_MIBDIRECT		=2,
	BFW_MIBIFIDX 		=5,
	BFW_MIBTAGDELTA		=4,
	BFW_UNIPORTMASK		=8,
#elif defined(CONFIG_RG_RTL9600_SERIES)
	BFW_UNIPORTMASK		=8,
#endif

} rtk_rg_shortcutBitFieldWidth_t;

#endif //defined(CONFIG_ROME_NAPT_SHORTCUT) || defined(CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT)



#ifdef CONFIG_ROME_NAPT_SHORTCUT
typedef struct rtk_rg_napt_shortcut_s
{
	uint16	sport;
	uint16	dport;
	ipaddr_t sip;
	ipaddr_t dip;

	uint32 spa					:BFW_SPA; // 4
	uint32 direction			:BFW_DIR; // 3
	uint32 isTcp				:BFW_ISTCP;  // 1
	uint32 isNapt				:BFW_ISNAPT; //L3 or L4 //1
	uint32 isBridge 			:BFW_ISBRIDGE;	  //for L2 //1
	uint32 isHairpinNat 		:BFW_ISHAIRPINNAT; // 1
	uint32 notFinishUpdated 	:BFW_NOTFINUPDATE; //indicate that this shortcut entry is not ready // 1
	uint32 vlanTagif			:BFW_VLANTAGIF; // 1
	uint32 serviceVlanTagif 	:BFW_SVLANTAGIF; // 2
	uint32 dmac2cvlanTagif		:BFW_DMAC2CVLANTAGIF; // 1
	uint32 internalCFPri		:BFW_INTERCFPRI;		//internal priority after CF module //3
	uint32 serviceVlanID		:BFW_SVLANID; // 13 or 14

	uint32 internalVlanID		:BFW_INTERVLANID;	//internal vlan for filtering // 13 or 14
	uint32 vlanID				:BFW_VLANID; // 13 or 14
	int32 dscp					:BFW_DSCP; // 7, -1 means invalid dscp remarking

	int32 dmac2cvlanID			:BFW_DMAC2CVLANID;	   //-1 means not apply // 13 or 14
	uint32 tagAccType			:BFW_TAGACCTYPE; // 1
	uint32 priority 			:BFW_PRIORITY;	//cvlan priority //3
	uint32 servicePriority		:BFW_SPRIORITY; //svlan priority // 3
#ifdef CONFIG_GPON_FEATURE
	uint32 streamID 			:BFW_STREAMID;	 //for GPON WAN, 7
	uint32 isLocalInNapt		:BFW_ISLOCALINNAPT;	// 1	
	uint32 reserve0 			:4; // padding to 32 bit

	int32 srcWlanDevIdx			:BFW_WLANDEVIDX;	// 6
	uint32 reserve1 			:26; // padding to 32 bit
#else
	uint32 isLocalInNapt		:BFW_ISLOCALINNAPT;	// 1
	int32 srcWlanDevIdx 		:BFW_WLANDEVIDX;	// 6	
	uint32 reserve0 			:5; // padding to 32 bit
#endif

	int		naptFilterRateLimitIdx	:(MAX_NAPT_FILER_SW_ENTRY_SIZE_SHIFT+1+1); //the naptFilter rule index which using to limit the rate of this shortcut, -1: not hit rate limit rule
	int		naptFilterPktCntIdx		:(MAX_NAPT_FILER_SW_ENTRY_SIZE_SHIFT+1+1); //the naptFilter rule index which using to add packet count, -1: no need to add packet count
	int		naptFilterByteCnttIdx	:(MAX_NAPT_FILER_SW_ENTRY_SIZE_SHIFT+1+1); //the naptFilter rule index which using to add byte count, -1: no need to add bye count
	uint32	naptFilterCopyCnt; //the copy count from naptFilter rule which using to send packet copies, 0: no need to send copies
	void *pAlgConn;
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)

	uint32 mibDirect			:BFW_MIBDIRECT;// 2
	int32 mibNetifIdx			:BFW_MIBIFIDX;	  //for update mib counter // 5
	int32 mibTagDelta			:BFW_MIBTAGDELTA;	//add or minus for NIC offload vlan tag //4
	int32 new_lut_idx			:BFW_LUTIDX;  //for da	  // 13
	int32 new_intf_idx			:BFW_INTFIDX; //for sa // 5
	uint32 reserve2 			:3; // padding to 32 bit

	int32 arpIdx				:BFW_ARPIDX;   //for updating idle time, -1 for non-arp // 11
	int32 smacL2Idx 			:BFW_LUTIDX;   //for updating idle time // 13
	int8 flowmib_idx 			:8;

	int32 naptIdx				:BFW_NAPTIDX; //for inbound dip,dport (naptInIdx), for outbound sport // 16
	uint32 idleSecs 			:BFW_IDLETIME; //16

	int32 new_eip_idx			:BFW_EIPIDX; //for outbound sip // 5
	uint32 uniPortmask			:BFW_UNIPORTMASK; // 11
	int32 src_intf_idx			:BFW_INTFIDX; // 5
	uint32 igr_pppoeTagif		:1;
	uint32 igr_svlanTagif		:1;
	uint32 igr_cvlanTagif		:1;
	uint32 reserve3 			:8; // padding to 32 bit

#elif defined(CONFIG_RG_RTL9602C_SERIES)

	uint32 mibDirect			:BFW_MIBDIRECT;// 2
	int32 new_lut_idx			:BFW_LUTIDX;  //for da	  // 12
	int32 new_intf_idx			:BFW_INTFIDX; //for sa // 5
	int32 new_eip_idx			:BFW_EIPIDX; //for outbound sip // 5
	uint32 uniPortmask			:BFW_UNIPORTMASK; // 8

	int32 arpIdx				:BFW_ARPIDX;   //for updating idle time, -1 for non-arp // 11
	int32 smacL2Idx 			:BFW_LUTIDX;   //for updating idle time // 12
	int32 mibNetifIdx			:BFW_MIBIFIDX;	  //for update mib counter // 5
	int32 mibTagDelta			:BFW_MIBTAGDELTA;	//add or minus for NIC offload vlan tag //4

	int32 naptIdx				:BFW_NAPTIDX; //for inbound dip,dport (naptInIdx), for outbound sport // 16
	uint32 idleSecs 			:BFW_IDLETIME; //16

#elif defined(CONFIG_RG_RTL9600_SERIES)

	int32 new_eip_idx			:BFW_EIPIDX; //for outbound sip // 5
	int32 naptIdx				:BFW_NAPTIDX; //for inbound dip,dport (naptInIdx), for outbound sport // 16
	uint32 uniPortmask			:BFW_UNIPORTMASK; // 8
	uint32 reserve2 			:3; // padding to 32 bit

	int32 new_lut_idx			:BFW_LUTIDX;   //for da    // 13
	int32 arpIdx				:BFW_ARPIDX;	//for updating idle time, -1 for non-arp // 11
	int32 new_intf_idx			:BFW_INTFIDX; //for sa // 5
	uint32 reserve3 			:3; // padding to 32 bit

	uint32 idleSecs 			:BFW_IDLETIME; //16
	int32 smacL2Idx				:BFW_LUTIDX;   //for sa    // 13
	uint32 reserve4 			:3; // padding to 32 bit

#else
#error
#endif

} rtk_rg_napt_shortcut_t;
#endif

#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
typedef struct rtk_rg_naptv6_shortcut_s
{
	uint16	sport;
	uint16	dport;
	rtk_ipv6_addr_t	sip;
	rtk_ipv6_addr_t	dip;

	uint32 spa					:BFW_SPA; // 4
	uint32 macPort				:BFW_MACPORT; // 4
	uint32 extPort				:BFW_EXTPORT; // 4
	uint32 servicePriority		:BFW_SPRIORITY;	  //svlan priority // 3
	uint32 internalCFPri		:BFW_INTERCFPRI;  //internal priority after CF module // 3
	uint32 isTcp				:BFW_ISTCP; // 1
	uint32 vlanID				:BFW_VLANID; // 13 or 14


	uint32 isBridge 			:BFW_ISBRIDGE;	//for L2 //1
	uint32 internalVlanID		:BFW_INTERVLANID;   //internal vlan for filtering // 13 or 14
	uint32 serviceVlanID		:BFW_SVLANID; // 13 or 14
	uint32 priority 			:BFW_PRIORITY;	//cvlan priority // 3
	int32 neighborIdx			:BFW_NEIGHBORIDX;   //for updating idle time, -1 for non-neighbor   // 8 or 10
	int32 dscp					:BFW_DSCP; // 6

	uint32 notFinishUpdated 	:BFW_NOTFINUPDATE;	//indicate that this shortcut entry is not ready // 1
	uint32 tagAccType			:BFW_TAGACCTYPE; // 1
	uint32 vlanTagif			:BFW_VLANTAGIF; // 1
	uint32 serviceVlanTagif 	:BFW_SVLANTAGIF; // 2
	uint32 dmac2cvlanTagif		:BFW_DMAC2CVLANTAGIF; // 1
	int32 dmac2cvlanID			:BFW_DMAC2CVLANID;	   //-1 means not apply // 13 or 14

#ifdef CONFIG_GPON_FEATURE
	uint32 streamID 			:BFW_STREAMID;		//for GPON WAN //7
	int32 srcWlanDevIdx			:BFW_WLANDEVIDX;	// 6
#else
	int32 srcWlanDevIdx			:BFW_WLANDEVIDX;	// 6
	uint32 reserve0 			:7; // padding to 32 bit
#endif

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)

	uint32 mibDirect			:BFW_MIBDIRECT; // 2
	int32 mibNetifIdx			:BFW_MIBIFIDX;	 //for update mib counter // 5
	int32 mibTagDelta			:BFW_MIBTAGDELTA;	//add or minus for NIC offload vlan tag // 4
	int32 smacL2Idx 			:BFW_LUTIDX; //for updating idle time //13
	int8 flowmib_idx 			:8;

	int32 new_lut_idx			:BFW_LUTIDX;  //for da  // 13
	int32 new_intf_idx			:BFW_INTFIDX; //for sa // 5
	uint32 uniPortmask			:BFW_UNIPORTMASK; // 11
	uint32 reserve2 			:3; // padding to 32 bit

	uint32 idleSecs 			:BFW_IDLETIME; //16
	uint32 reserve3 			:16; // padding to 32 bit

#elif defined(CONFIG_RG_RTL9602C_SERIES)

	uint32 mibDirect			:BFW_MIBDIRECT; // 2
	uint32 uniPortmask			:BFW_UNIPORTMASK; //8
	int32 mibNetifIdx			:BFW_MIBIFIDX;	 //for update mib counter // 5
	int32 mibTagDelta			:BFW_MIBTAGDELTA;	//add or minus for NIC offload vlan tag // 4
	int32 smacL2Idx 			:BFW_LUTIDX; //for updating idle time //12
	uint32 reserve1 			:1; // padding to 32 bit

	int32 new_lut_idx			:BFW_LUTIDX;  //for da  // 12
	int32 new_intf_idx			:BFW_INTFIDX; //for sa // 5
	uint32 reserve2 			:15; // padding to 32 bit

	uint32 idleSecs 			:BFW_IDLETIME; //16
	uint32 reserve3 			:16; // padding to 32 bit

#elif defined(CONFIG_RG_RTL9600_SERIES)
	int32 new_intf_idx			:BFW_INTFIDX; //for sa // 5
	int32 new_lut_idx			:BFW_LUTIDX;  //for da	 //13
	uint32 uniPortmask			:BFW_UNIPORTMASK; // 8
	uint32 reserve1 			:6; // padding to 32 bit

	uint32 idleSecs 			:BFW_IDLETIME; //16
	int32 smacL2Idx				:BFW_LUTIDX;  //for sa	 //13
	uint32 reserve2 			:3; // padding to 32 bit

#else
#error
#endif
} rtk_rg_naptv6_shortcut_t;
#endif


typedef struct rtk_rg_table_icmp_flow_s
{
	uint8 valid;
	uint8 psConnection;	//from protocol-stack icmp connection
	ipaddr_t internalIP;
	ipaddr_t remoteIP;
	uint16 IPID;		//identifier field in IP header
	uint16 inboundIPID;		//identifier field in IP header
	uint8 ICMPType;
	uint8 ICMPCode;
	uint16 ICMPID;		//identifier field in ICMP header (some type without this field)
	uint16 ICMPSeqNum;
	unsigned long int jiffies;		//the time that this flow create.
}rtk_rg_table_icmp_flow_t;

typedef struct rtk_rg_table_icmp_linkList_s
{
	rtk_rg_table_icmp_flow_t icmpFlow;
	struct rtk_rg_table_icmp_linkList_s *pPrev, *pNext;
}rtk_rg_table_icmp_linkList_t;

//IPv6 Stateful routing
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
typedef struct rtk_rg_ipv6_layer4_linkList_s
{
	//5 tuple
	rtk_ipv6_addr_t		srcIP;
	rtk_ipv6_addr_t		destIP;
	uint16				srcPort;
	uint16				destPort;
	uint8				isTCP;		//0:UDP, 1:TCP

	rtk_rg_port_idx_t 	spa;
	rtk_rg_mbssidDev_t	srcWlanDevIdx;

	//additional info for IPv6 NAPT
	rtk_ipv6_addr_t		internalIP;		//internal IP: supported by ipv6 NAPTR
	uint16				externalPort;	//l4 external port: supported by ipv6 NAPT
	uint16				internalPort;	//l4 internal port: supported by ipv6 NAPTR
	uint8				extipIdx;	//supported by ipv6 napt

	rtk_rg_naptState_t 	state;
	struct rtk_rg_ipv6_layer4_linkList_s *pPair_list;
	uint32 				idleSecs;

	uint8 				netifIdx;
	uint16 				dmacL2Idx;
	int smacL2Idx; 		//for updating idle time
#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support mib counter of interface
	//mib counter and cf decision
	int mibNetifIdx; //for update mib counter
	rtk_rg_cf_direction_type_t mibDirect;
	int8 mibTagDelta;	//add or minus for NIC offload vlan tag
	int8 flowmib_idx 			:8;
#endif
	rtk_l34_nexthop_type_t	wanType;
	rtk_rg_naptDirection_t	direction;
#ifdef CONFIG_GPON_FEATURE
	uint8 streamID;	//for GPON WAN
#endif
	uint8 priority;
	uint8 servicePriority;
	uint16 internalVlanID;	//internal vlan for filtering
	uint8 internalCFPri;	//internal priority after CF module
	int16 dscp;
	uint16 vlanID;
	uint8 vlanTagif;
	uint16 serviceVlanID;
	uint8 serviceVlanTagif;
	int16 dmac2cvlanID;		//-1 means not apply
	uint8 dmac2cvlanTagif;
	int8 macPort;
	int8 extPort;	//hw extention port
	int neighborIdx;	//for updating idle time, -1 for non-neighbor
	uint32 uniPortmask;
	uint8 tagAccType;

	//fragment data
	uint8 isFrag;
	uint32 fragAction;
	unsigned long beginIdleTime;
	uint16 receivedLength;	//already received packet length
	uint16 totalLength;	//calculated by last fragment
	uint8 queueCount;

	uint8 valid;
	//indicate that this shortcut entry is not ready
	uint8 notFinishUpdated;

	uint16 idx;
	struct list_head layer4_list;
}rtk_rg_ipv6_layer4_linkList_t;
#endif

typedef struct rtk_rg_pktHdr_s
{
	struct sk_buff *skb;

	/* NIC RX Desc */
	rtk_rg_rxdesc_t	*pRxDesc;

	/* Tag Info */
	rtk_rg_pkthdr_tagif_t	tagif;
	rtk_rg_pkthdr_tagif_t	egressTagif;
	uint8 *pDmac;
#if defined(CONFIG_RG_RTL9602C_SERIES)
	//mib counter and cf decision
	uint8  dmac[ETHER_ADDR_LEN];	//used for mib counter update  & naptFilter
#endif
	uint8 *pSmac;

	/* Packet Parsing Info */
	// STAG
	uint8 	*pSVlanTag;
	uint8	stagPri;
	uint8	stagDei;
	uint16	stagVid;
	uint16	stagTpid;

	// CTAG
	uint8 	*pCVlanTag;
	uint8	ctagPri;
	uint8	ctagCfi;
	uint16	ctagVid;

	uint16 	etherType;

	// etherType=PPPoE(0x8863/0x8864)
	uint16 pppoeVerTypeCode;
	uint16 sessionId;
	uint16 *pPppoeLength;
	uint16 pppProtocal; //IPv4:0x0021, LCP: 0xC021, IPCP: 0x8021, IPv6:0x0057, IP6CP: 0x8057

	// etherType=IPv4(0x0800),ARP(0806)
	uint8	ipv4HeaderLen;
	uint16 	l3Offset;
	uint8 	tos;
	uint8 	*pTos;
	uint16 	l3Len;		//include v4/v6 hdr Len
	uint16 	*pL3Len;	//v4 Hdr->L3Len
	uint32 	ipv4Sip;	 //hairpin nat will modify this value
	uint32 	ipv4Dip;
	uint32 	*pIpv4Sip;
	uint32 	*pIpv4Dip;
	uint16 	ipv4Checksum;
	uint16 	*pIpv4Checksum;
	uint8 	ipv4FragPacket; //ipv4MoreFragment==1 or ipv4FragmentOffset!=0
	uint8 	ipv4DontFragment;
	uint8 	ipv4MoreFragment;
	uint16 	ipv4FragmentOffset;
	uint16 	ipv4Identification;
	uint8 	ipv4TTL;
	uint8 	*pIpv4TTL;

	// only for PPTP/L2TP
	uint8	outer_ipv4HeaderLen;
	uint16 	outer_l3Offset;

	uint8	*pIpv6FlowLebal; //first 4 bits are IPv6's traffic class,
	uint32	ipv6FlowLebal;

	// etherType=IPv6(0x086dd)
	uint16 ipv6PayloadLen;
	uint8 *pIPv6HopLimit;
	uint8 *pIpv6Sip;
	uint8 *pIpv6Dip;
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	uint32 ipv6Sip_hash;
	uint32 ipv6Dip_hash;
	uint32 tcpUdpTracking_ipv6SipHash;
	uint32 tcpUdpTracking_ipv6DipHash;
#endif
	uint8 ipv6FragPacket; //ipv6MoreFragment==1 or ipv6FragmentOffset!=0
	uint8 ipv6MoreFragment;
	uint16 ipv6FragmentOffset;
	uint16 ipv6FragId_First;
	uint16 ipv6FragId_Second;
	uint32 ipv6FragId;
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	//20141208LUKE: keep the fragment list pointer after _rtk_rg_ipv6StatefulDecision
	rtk_rg_ipv6_layer4_linkList_t	*pIPv6FragmentList; 	//used for update information after send fragment_one
#endif

	//ipv4/ipv6
	uint8 ipProtocol;
	uint16 l4Offset;

	//ARP
	uint16	arpOpCode;

	// ipProtocol=TCP/UDP
	uint16 sport;		//hairpin nat will modify this value
	uint16 dport;
	uint16 *pSport;
	uint16 *pDport;
	uint16 l4Checksum;
	uint16 *pL4Checksum;

	//TCP
	rtk_rg_tcpFlags_t tcpFlags;
	uint32  tcpFlags_high:4;
	uint32	tcpSeq;
	uint32	tcpAck;
	uint32	*pTcpSeq;
	uint32	*pTcpAck;
	uint8 	*pL4Payload;
	uint16	*pMssLength;
	uint8	headerLen;
	uint16 	tcpWindow;
	//HTTP(GET,POST) for URL Filter
	int		httpFirstPacket;
	//HTTPS(Client Hello) for URL Filter
	int		httpsClientHelloPacket;

	//UDP
	//siyuan add for alg function which may change the udp data length
	uint16 * pL4Len;
	uint16 l4Len;

	//PPTP decision
	uint16 pptpCtrlType;
	rtk_rg_pptpCallIds_t *pPptpCallId;
	rtk_rg_pptpCodes_t pptpCodes;
	uint16 *pGRECallID;		//receiver's callID
	uint32 GRESequence;
	uint32 GREAcknowledgment;
	uint32 *pGRESequence;
	uint32 *pGREAcknowledgment;
	uint32 *pServerGRESequence;		//pointer to server's sequence number

	//L2TP decision
	uint16 l2tpFlagVersion;

	//IGMP
	uint8	IGMPType;
	uint8	IGMPQueryVer;
	uint8   ingressIgmpMldDrop;
#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	uint8	IGMP_report_passthrough_pkt;
#endif

	//IGMPv6
	uint8	IGMPv6Type;

	//ICMPv4
	uint8	ICMPType;
	uint8	ICMPCode;
	uint16	ICMPIdentifier;
	uint16	ICMPSeqNum;
	//ICMPv6
	uint8	ICMPv6Type;
	uint8	ICMPv6Flag;
	uint8	*pICMPv6TargetAddr;
	uint8	*pICMPv6SourceLinkAddr;
	uint8	*pICMPv6TargetLinkAddr;

	/* Ingress Information */
	rtk_rg_port_idx_t ingressPort;
	rtk_rg_mac_port_idx_t ingressMacPort;
	rtk_rg_mac_ext_port_idx_t ingressMacExtPort;
	rtk_rg_ingressLocation_t ingressLocation;	

	/* WLAN */
	rtk_rg_mbssidDev_t wlan_dev_idx;

	/* FWD Decision */
	rtk_rg_fwdDecision_t	fwdDecision;
	int l3Modify;
	int l4Modify;

	// whether packet match trace filter or not
	uint8 hitTraceFilter; //0: unmatch, 1: match

#ifdef CONFIG_MASTER_WLAN0_ENABLE
	rtk_rg_sw_copyToPSWork_t *pWifiIngressCopyWork; 	//work used to schedule
	rtk_rg_sw_copyToPSWork_t *pWifiEgressCopyWork;		//work used to schedule
	unsigned int wifiIngressPacketCopied:1; // 0: not copy yet. 1: copied
	unsigned int wifiEgressPacketCopied:1;	// 0: not copy yet. 1: copied
#endif

	// data path is hairpinNat
	unsigned int isHairpinNat:1;

	// data path is local in napt/routing
	unsigned int isLocalInNapt:1;

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	// add pure L2 flow or not
	unsigned int addPureL2Flow:1;
	//L2/L3 tcp tracking done
	unsigned int tcpUdpTrackingDone:1;
	//sw flow only
	unsigned int swFlowOnlyByAcl:1; 
	// data path is tiny ack
	unsigned int isTinyAck:1;
#endif
	//directly send to PS without do vlan decision or ACL again.
	unsigned int byPassToPsVlanAclDecision:1;

	//directly send to Mac-binding wan without entering protocol stack.	
	unsigned int isDHCP:1;		//v4 or v6
	unsigned int optXXDHCPbyPassToWAN:1;

	uint32 urlPriHit:1;
	uint32 urlPriHitIdx;

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM) || defined(CONFIG_RG_RTL9600_SERIES)
	uint8 cpSkbToPsbyBC; //0:not broadcast 1: already get copied skb through bc path
	uint8 cpSkbToPsSucc; //0:no action  1:need to copy one more packet to PS, and make sure skb copy success.
#endif
	//We need to record the following value for being used after directTx
	uint32 skbLen;
	uint8 ipv6_sip_addr[IPV6_ADDR_LEN];
	uint8 ipv6_dip_addr[IPV6_ADDR_LEN];
	uint8 egr_tos;

	int16 swMeterOffsetIdx;	// share meter idx 0~79 - sw offset 48

	//vxlan acceleration
	uint8 *pVxlan_inner_data;
	rtk_rg_napt_shortcut_t *pVxlanInNaptShortcutEntry;
	rtk_rg_napt_shortcut_t *pVxlanOutNaptShortcutEntry;
	unsigned int isVXLAN_and_flowMiss:1;	//burst input, do not readd flow
	unsigned int isVXLAN_and_hwAcc:1;		//both forward and hwlookup flow can add to hw

	//nptv6 acceleration mechanism
	int8 nptv6_acceleration_upstream_idx;
	int8 nptv6_acceleration_downstream_idx;
	int8 nptv6_acceleration_upstream0_idx;
	int8 nptv6_acceleration_downstream0_idx;
	unsigned int isNPTv6_and_flowMiss:1;
	unsigned int isNPTv6_and_Hwlookup:1;
	unsigned int isNPTv6_and_HwAcc:1;	
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 30)
	//NPTv6
	unsigned int isNPTv6:2; // 1: outbound, 2: inbound, 3: checked and trap to PS
	rtk_ipv6_addr_t ipv6Dip_addr; // for NPTv6 to recover DIP if packet go into slowpath after shortcut check 
	rtk_ipv6_addr_t ipv6Sip_addr;
#endif
	
	rtk_rg_mibUpdate_entry_t mibUpdateInfo;

#if defined(CONFIG_RG_RTL9607C_SERIES)
	unsigned int downstream_loopback_untag_packet: 1;
#endif
//==================================================================================

	//20140829LUKE: CAUTION!!this field will be used to count pktHdr size, DO NOT MOVE IT!!
	int COUNT_LENGTH_FIELD;

	//SA
#if !defined(CONFIG_RG_RTL9602C_SERIES)
	//mib counter and cf decision
	uint8  dmac[ETHER_ADDR_LEN];	//used for mib counter update  & naptFilter,   Apollo this value only used when slow path
#endif
	uint8  smac[ETHER_ADDR_LEN];	//used for naptFilter

	//NAPT/NAPTR result
	rtk_rg_l4Direction_t l4Direction;
	int32 naptOutboundIndx;
	int32 naptrInboundIndx;
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	int32 hairpinNat_naptOutboundIndx;
	int32 tcpUdpTrackingGroupIdx;	// record bridge/routing tcp tracking group index
#endif
	uint8 naptrLookupHit; //0:non-hit, 1:hit first-callback(UPnP), 2:hit second-callback(Virtual Server), 3:hit third-callback(DMZ)

	/* Routing decision */
	int		sipL3Idx; //decision in _rtk_rg_sip_classification
	int		dipL3Idx; //decision in _rtk_rg_dip_classification
	int		netifIdx; //egress wan intf
	int		extipIdx;
	int		dipArpOrNBIdx;
	int		nexthopIdx;
	int		isGatewayPacket;
	int		dmacL2Idx;
	int		naptFilterRateLimitIdx; //the naptFilter rule index which using to limit the rate of this shortcut, -1: not hot rate limit rule
	int		naptFilterPktCntIdx; //the naptFilter rule index which using to add packet count, -1: no need to add packet count
	int		naptFilterByteCnttIdx; //the naptFilter rule index which using to add byte count, -1: no need to add bye count
	uint32	naptFilterCopyCnt;	//the count of naptFilter rule assign which using to send packet copies to protocol stack, 0: no need to send copies
	uint32	*pFlowCopyCnt;		//pointer of copy count per flow
	rtk_rg_sw_copyToPSWork_t *pFlowCopyWork;		//work used to schedule
	int8	*pFlowIgrACLMirrorCnt;	//pointer of ingress mirror count per flow
	int8	*pFlowEgrACLMirrorCnt;	//pointer of egress mirror count per flow
	uint32  flowListIdx;

	int		dualHdrNetifIdx;
	//20140811LUKE: used for tcp_hw_learning_at_syn
	int		srcNetifIdx; //src intf
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	int		path6Idx;
	int		hairpinNatr_srcNetifIdx;
	int		L2L3FragListIdx;
#endif
	int		smacL2Idx;
#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support mib counter of interface
	//mib counter and cf decision
	int 	mibNetifIdx; //init by FAIL
	rtk_rg_cf_direction_type_t mibDirect;
	int8 	mibTagDelta;	//add or minus for NIC offload vlan tag
#endif
	/* Binding */
	int		bindNextHopIdx;
	int		layer2BindNetifIdx; //layer2 binding wan intf decision, only apply when destination port is WAN port(in rg_db.systemGlobal.wanPortMask.portmask)

	/* VLAN decision*/
	uint8	dmac2VlanTagif;
	uint8 	egressVlanTagif; //0:untag  1:tagged
#if defined(CONFIG_RG_RTL9600_SERIES)
	uint8 	egressServiceVlanTagif;	//0:untag,  1:tagged with tpid, 2:tag with 0x8100
#else // support tpid2
	uint8 	egressServiceVlanTagif;	//0:untag  1:tagged with tpid, 2:tag with tpid2,  3:tagged with original S-tag-tpid
#endif
	uint16 	internalVlanID;
	int		dmac2VlanID;
	uint16 	egressVlanID;
	uint16	egressServiceVlanID;
	uint16	egressVlanCfi;
	uint16	egressServiceVlanDei;

	uint16 	ingressDecideVlanID; //record the ingress decision result(acl,1q,port-based,PPB), only used for proc/rg/tcp_hw_learning_at_syn now.

	//uint8	IGMPQueryVer;
	/*ACL*/
	rtk_rg_aclHitAndAction_t aclDecision;
	int egressIntfIdxPreTrans;

	/*QOS*/
	uint8	internalPriority;
	uint8	egressPriority;
	uint8	egressServicePriority;
	uint8	egressDSCPRemarking;		//ref to rtk_rg_qosDscpRemarkSrcSelect_t
	int16	egressDSCP;
	int16	egressECN;
	int	aclPriority;//record ACL priotity for internal priority decision
	int naptPriorityHitIf;//record is any naptPriority Rule hit
	int naptPriority;//record the napt assigned priority (valid when naptPriorityHitIf==1)

	/* ALG */
	rtk_rg_algAction_t algAction;
	uint8 algKeepExtPort;		//should I choose another port for replicate one?
	p_algRegisterFunction algRegFun;
	void *pAlgConn;

#ifdef CONFIG_GPON_FEATURE
	/* Stream ID */
	uint8	streamID;
#endif

	uint8	tinyAckStreamID;

	/* FWD Decision */
	rtk_rg_mac_port_idx_t	egressMacPort;
	rtk_rg_mac_ext_port_idx_t	egressMacExtPort;
	rtk_rg_portmask_t	multicastMacPortMask;
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	rtk_rg_table_flowEntry_t McFlowPath3action;
	rtk_rg_table_flowEntry_t McFlowPath4action;
	uint32 McFlowPath3PortMsk;
	uint32 McFlowPath4PortMsk;
	uint32 McFlowPath3ExtPortMsk;
	uint32 McFlowPath4ExtPortMsk;
	uint8  McFlowAddTohw;
	int16  flowDataSkbLen;
	uint8  flowDataBuf[DEFAULT_MAX_DATA_PATTEN];
	uint32 McFlowEgressCtagifMsk;
	uint16 McFlowVlanTag[RTK_RG_PORT_MAX];
#endif
	uint8	overMTU;		//if the packet is bigger than interface's MTU
	uint8	aclHit;	//0:no any rule hit(can be added into short cut)  1:at lease one rule hit(can not add to short cut)
	int8	aclPolicyRoute;		//-1: no policy route hit
	int8	aclPolicyRoute_arp2Dmac;		//0: use policy route's NH as DMAC, 1: use ARP's LUT as DMAC
	int8	swLutL2Only;		//0 means permit all, 1 means permit L2 only
//	uint8	egressHasSmallerMTU;	//if egress intf has smaller MTU, check if we need to split packet before send
	uint8	gponDsBcModuleRuleHit;	//notify master wifi vlanTagshould follow this hit decision
	uint32	egressUniPortmask; //ACL/CF force redirect egress port
	uint8	egressTagAccType;	//used for _rtk_rg_get_stagCtagAccelerationType
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	rtk_rg_mbssidDev_t 	egressWlanDevIdx; 			//for rate limit, 0:root, 1:vap0, 2:vap1, 3:vap2, 4:vap3
#endif

#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	rtk_rg_ipv6_connLookup_t ipv6_serverInLanLookup;	//upnp, virtual server, dmz hit or not
#endif

#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	int32	ipv6StatefulHashValue;	//keep the hash value for fill software data structure
	rtk_rg_ipv6_layer4_linkList_t	*pIPv6StatefulList;		//used for update information after normal path
#endif
	rtk_rg_bindingDecision_t 	bindingDecision;
	rtk_rg_fwdEngineReturn_t	directTxResult;		//do directTX may need broadcast, therefore we need to keep it's return value
#if defined(CONFIG_ROME_NAPT_SHORTCUT) || defined(CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT)
	rtk_rg_shortcutDecision_t	shortcutStatus;
	//uint32	currentShortcutIndex;
#ifdef CONFIG_ROME_NAPT_SHORTCUT
	rtk_rg_napt_shortcut_t *pCurrentShortcutEntry;
	rtk_rg_napt_shortcut_t *pInboundShortcutEntry;	//used when tcp_in_shortcut_learning_at_syn is enabled
	rtk_rg_napt_shortcut_t *pLocalInNaptShortcutEntry;
	int currentShortcutIdx;
	int inboundShortcutIdx;
	//int localInNaptShortcutIdx;
#endif
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
	rtk_rg_naptv6_shortcut_t *pCurrentV6ShortcutEntry;
	int currentV6ShortcutIdx;
#endif
#endif
	int unmatched_cf_act;

	int direct;		//0: NAPT upstream  1:NAPT downstream  2:L2 Bridge 3:IPv6 Routing
	int naptIdx;	//pktHdr->direct is 0 : naptOutboundIdx  pktHdr->direct is 1 :naptInboundIdx
	unsigned int l2AgentCalled:1;	// 0: not call yet.  1: called
	unsigned int arpAgentCalled:1;	// 0: not call yet.  1: called
	unsigned int neighborAgentCalled:1;	// 0: not call yet.  1: called
	unsigned int autoNeighborCalled:1;	// 0: not call yet.  1: called

	// delete before sending
	unsigned int delNaptConnection:1; //0: don't delete before send.  1: delete before send.

	// add after sending
	unsigned int addNaptAfterNicTx:1;

	// enable when PPTP or L2TP to save hw napt table usage
	unsigned int addNaptSwOnly:1;

	// enable when Http request need to stay in sw for further check
	unsigned int checkHttpKeepInSw:1;
	unsigned int checkHttpsKeepInSw:1;
#if defined(CONFIG_RG_IPSET_VERSION) && (CONFIG_RG_IPSET_VERSION==2)
	unsigned int ipsetsPriority:4;
	unsigned int ipsetsEgrWlanDevIdx:4;
#endif

	//naptFilterAndQoS checking ICMP
	rtk_rg_table_icmp_flow_t *icmpCtrlFlowForNaptFilter;

	//record number of forwarded packets, it is used to check MC/BC drop
	uint32 forwardCount;

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	//disable out_egress_cvid_act of flow when it hits cvid remarking of cf (defaultly enable)
	uint8 flow_cvid_with_egress_filter;
	// flow mib index for packet/byte counting
	int8 flowMIBCounterIdx;
#endif

	uint16 sport_firstFrag;
	uint16 dport_firstFrag;

	uint8 aclTrapToPs;
} rtk_rg_pktHdr_t;
/* End of Parser ========================================================== */

/* ALE ==================================================================== */

typedef enum rtk_rg_ipClassification_e
{
	IP_CLASS_FAIL = -1,
	IP_CLASS_NPI =0,
	IP_CLASS_NI  =1,
	IP_CLASS_LP  =2,
	IP_CLASS_RP  =3,
	IP_CLASS_NPE =4,
	IP_CLASS_NE  =5,
	MAX_SIP_CLASS =4,
	MAX_DIP_CLASS =6
} rtk_rg_ipClassification_t;


typedef enum rtk_rg_sipDipClassification_e
{
	SIP_DIP_CLASS_ROUTING=0,
	SIP_DIP_CLASS_NAT=1,
	SIP_DIP_CLASS_NAPT=2,
	SIP_DIP_CLASS_NATR=3,
	SIP_DIP_CLASS_NAPTR=4,
	SIP_DIP_CLASS_CPU=5,
	SIP_DIP_CLASS_DROP=6,
	SIP_DIP_CLASS_HAIRPIN_NAT=7
} rtk_rg_sipDipClassification_t;


typedef enum rtk_rg_entryStatus_e
{
	INVALID_ENTRY=0,
	SOFTWARE_HARDWARE_SYNC_ENTRY=1,
	SOFTWARE_ONLY_ENTRY=2,
	HARDWARE_ONLY_ENTRY=3,
} rtk_rg_entryStatus_t;


//valid flag
#define VALID_ENTRY						1
#define SOFTWARE4_ENTRY					2
#define SOFTWARE6_ENTRY					3

typedef enum rtk_rg_interface_entryStatus_e
{
	IF_INVALID_ENTRY=0,						// 0
	IF_VALID_ENTRY	  =1<<VALID_ENTRY,		// 1
	IF_SOFTWARE4_ENTRY=1<<SOFTWARE4_ENTRY,	// 2
	IF_SOFTWARE6_ENTRY=1<<SOFTWARE6_ENTRY,	// 4

} rtk_rg_interface_Status_t;



/* End of ALE ============================================================== */

/* Tables ================================================================= */
/*typedef struct rtk_rg_table_mac_s
{
	//HW table
	rtk_mac_t macAddr;
} rtk_rg_table_mac_t;*/

typedef struct rtk_rg_table_vlan_s
{
	//HW table
    rtk_portmask_t MemberPortmask; /*bit'6 CPU port */
    rtk_portmask_t UntagPortmask;
	rtk_portmask_t Ext_portmask;   /*bit'0 CPU prot */	/*Note!! For FB: bit'0 stands for ext0 not cpu port*/
	rtk_fid_t fid;
    rtk_fidMode_t fidMode;
	rtk_pri_t priority;
	rtk_enable_t priorityEn;


	//SW table specific
	uint32 valid:1;
	uint32 addedAsCustomerVLAN:1;		//used to check this vlan created by cvlan apis or not
#if defined (CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	uint8 extPMaskIdx;
#endif
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	unsigned int wlan0DevMask;
	unsigned int wlan0UntagMask;
#endif
} rtk_rg_table_vlan_t;

typedef struct rtk_rg_table_lut_s
{
	rtk_l2_addr_table_t rtk_lut;

	uint32 valid:1;
	uint32 fix_l34_vlan:1;
	uint32 permit_for_l34_forward:1;	//used for wan access limit
#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support host policy
	uint32 host_idx_valid:1;	//1:host_idx is meaningful, 0:host_idx is meaningless
	uint32 host_idx;			//keep host policing index
	uint32 host_dmac_rateLimit:1;	// 1: do dmac rate limit, 0: no need to do dmac rate limit
#endif
#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support lut traffic bit
	uint32 idleSecs;
	uint32 arp_refCount;
#endif

	uint16 mcStaticRefCnt; //for pppoeMc/multicast keep lut hihg priority
	uint8 vxlanStaticRefCnt; //for vxlan remote mac
	uint8 redirect_http_req;
	unsigned long redirect_http_jiffies;
	int8 category;
	int8 wlan_device_idx;	//used only when rtk_lut is unicast and port is CPU and extport is 1 or 2
	uint32 lutIP4addr;
	uint32 lutIP6addr[4];
	char dev_name[MAX_LANNET_DEV_NAME_LENGTH];	//for LANNetInfo
	rtk_rg_lanNet_connect_type_t conn_type;		//for LANNetInfo
	int32 lutGroupIdx;
#if defined(CONFIG_RG_G3_SERIES)
	int32 hashLiteIdx;
#endif
	uint32 countingInLearningLimit: 1;
} rtk_rg_table_lut_t;

typedef struct rtk_rg_table_netif_s
{
	rtk_l34_netif_entry_t	rtk_netif;
	int	l2_idx;
#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support mib counter of interface
	rtk_rg_netifMib_entry_t netifMib;
#endif
} rtk_rg_table_netif_t;

typedef struct rtk_rg_table_l3_s
{

	rtk_l34_routing_entry_t	rtk_l3;
	ipaddr_t	gateway_ip;		//20140703LUKE: used to compare gateway IP or not
	ipaddr_t	netmask;
	rtk_rg_entryStatus_t valid; // used to software entry
} rtk_rg_table_l3_t;

typedef struct rtk_rg_table_extip_s
{
	rtk_l34_ext_intip_entry_t	rtk_extip;
	rtk_rg_entryStatus_t valid; // used to software entry

} rtk_rg_table_extip_t;

typedef struct rtk_rg_table_nexthop_s
{

	rtk_l34_nexthop_entry_t	rtk_nexthop;
	rtk_rg_entryStatus_t valid; // used to software entry
	unsigned int extIPMask;
	unsigned int staticRouteWanIdxMask;	// indicate which WAN interface index this nexthop belonged
} rtk_rg_table_nexthop_t;

typedef struct rtk_rg_table_pppoe_s
{

	rtk_l34_pppoe_entry_t	rtk_pppoe;
	rtk_rg_entryStatus_t valid; // used to software entry
	uint32					idleSecs;
} rtk_rg_table_pppoe_t;

typedef struct rtk_rg_table_arp_s
{

	rtk_l34_arp_entry_t	rtk_arp;

	ipaddr_t ipv4Addr;
	int idleSecs;
	uint32 staticEntry:1;
	int8 sendReqCount;
	int8 routingIdx;
	uint32 permit_for_l34_forward:2;	//used for wan access limit, 0:not permit, 1:non-limited permit, 2:limited permit
	//lan device info
	rtk_rg_lanNetInfo_t lanNetInfo;
} rtk_rg_table_arp_t;

typedef struct rtk_rg_vbind_linkList_s
{
	int8	wanIdx;
	uint16	vlanId;

	struct list_head vbd_list;
}rtk_rg_vbind_linkList_t;

typedef struct rtk_rg_arp_linkList_s
{
	uint16 idx;		//from MAX_ARP_HW_TABLE_SIZE to (MAX_ARP_SW_TABLE_SIZE - MAX_ARP_HW_TABLE_SIZE)

	struct list_head arp_list;
}rtk_rg_arp_linkList_t;

typedef struct rtk_rg_lut_linkList_s
{
	uint16 idx;		//from MAX_LUT_HW_TABLE_SIZE to MAX_LUT_HW_TABLE_SIZE+MAX_LUT_BCAM_TABLE_SIZE

	struct list_head lut_list;
}rtk_rg_lut_linkList_t;

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
typedef enum rtk_rg_shortcut_type_e
{
	RTK_RG_IPV4_SHORTCUT=0,
	RTK_RG_IPV6_SHORTCUT,
	RTK_RG_IPV6_STATEFUL_LIST,
} rtk_rg_shortcut_type_t;
typedef struct rtk_rg_flow_naptFilterInfo_s
{
#if MAX_FLOW_SW_NAPT_FILTER_IDX_WIDTH > 10
#error "Out of sw napt filter size"
#endif
	int32 naptFilterRateLimitIdx	:MAX_FLOW_SW_NAPT_FILTER_IDX_WIDTH;
	int32 naptFilterPktCntIdx		:MAX_FLOW_SW_NAPT_FILTER_IDX_WIDTH;
	int32 naptFilterByteCnttIdx		:MAX_FLOW_SW_NAPT_FILTER_IDX_WIDTH;
	uint32 reserve0					:(32-3*MAX_FLOW_SW_NAPT_FILTER_IDX_WIDTH);

	uint32 naptFilterCopyCnt;
}rtk_rg_flow_naptFilterInfo_t;
typedef struct rtk_rg_flow_ACLFilterInfo_s
{
	int16 acl_index;
	int8 mirrorCnt;			//-1:unlimit
	uint8 noEncap:1;		//1:no encapsulate
	uint8 remakeSmac:1;		//1:remake smac
}rtk_rg_flow_ACLFilterInfo_t;

#if defined(CONFIG_RG_G3_SERIES)
/* This is g3 mcEngine hardware entry  care pattens */
typedef struct rtk_rg_mcEngine_action_s
{
	uint32	vlanActionMsk:1;
	uint32	smacTrans:1;
	uint8	vlanAction;
	uint16	cvlanID;
	uint8	cPri;
	uint8	userPri;
}rtk_rg_mcEngine_action_t;


typedef struct rtk_rg_mcEngine_info_s
{
	uint32 valid:1;
	uint32 isipv6:1;
	uint32 groupAddress[4]; 	//v4 -> groupAddress[0]=hostEndian  v6 -> networkEndian
	uint32 sourceAddress[4];	//v4 -> groupAddress[0]=hostEndian  v6 -> networkEndian
	uint32 l3mcgid;
	uint32 l2mcgid;
	uint64 p3Pmsk;
	rtk_rg_mcEngine_action_t p3Action;
	uint64 p4Pmsk;
	rtk_rg_mcEngine_action_t p4Action;
	uint32 mcHwRefCount;
}rtk_rg_mcEngine_info_t;

/*G3 mac host policing info*/
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
typedef struct rtk_rg_hostPoliceControl_l2_cls_idx_s
{
	int16 idx_ingress;	//stored L2 CLS index (ca sw index) index for RX (ingress SMAC) host policing, -1 mean n.a.
	int16 idx_egress;	//stored L2 CLS index (ca sw index) index for TX (ingress DMAC) host policing, -1 mean n.a.
}rtk_rg_hostPoliceControl_l2_cls_idx_t;
#endif

typedef struct rtk_rg_g3_mac_hostPolicing_info_s
{
	rtk_rg_enable_t ingressRateLimit_en;//the satus of ingress rate limit function for the MAC address
	rtk_rg_enable_t egressRateLimit_en;	//the satus of egress rate limit function for the MAC address
	rtk_rg_enable_t logging_en;			//the satus of mib function for the MAC address
	int8 ingressRateLimit_mtrIdx;		//the sharemeter index for the ingress rate limit function for the MAC address
	int8 egressRateLimit_mtrIdx;		//the sharemeter index for the egress rate limit function for the MAC address
	int8 logging_hostPolEntryIdx;		//the host policing index for the mib function for the MAC address
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
	rtk_rg_hostPoliceControl_l2_cls_idx_t l2ClsIndex; //the related L2 CLS indexs of host policing for the MAC address
#endif
}rtk_rg_g3_mac_hostPolicing_info_t;

typedef enum rtk_rg_g3_flow_hostPolicing_hitState_e
{
	RTK_RG_G3_FLOW_HOSTPOLICING_HIT_NONE=0,
	RTK_RG_G3_FLOW_HOSTPOLICING_HIT_SA,
	RTK_RG_G3_FLOW_HOSTPOLICING_HIT_DA,
}rtk_rg_g3_flow_hostPolicing_hitState_t;

typedef struct rtk_rg_swFlowIdx_s
{
#if MAX_FLOW_SW_TABLE_SIZE > 65536
	uint32 swFlowIdx;
#else
	uint16 swFlowIdx;
#endif
}rtk_rg_swFlowIdx_t;
#endif

typedef struct rtk_rg_flow_extraInfo_s
{
	rtk_ipv6_addr_t v6Sip;
	rtk_ipv6_addr_t v6Dip;
	uint16 igrSVID;
	uint16 igrSPRI;
	uint16 igrCVID;
	uint16 lutSaIdx;
	uint16 lutDaIdx;
	uint16 path34_isGmac;
	int8 tinyAckStreamId;
	int32 naptOrTcpUdpGroupIdx;
	int32 arpOrNeighborIdx_src;
	int32 arpOrNeighborIdx_dst;
	uint8 isMulticast;
	uint8 staticEntry;
	uint8 addSwOnly;
	uint8 byDmac2cvid;
	rtk_rg_flow_naptFilterInfo_t *pNaptFilterInfo; /*useful if addSwOnly=1*/
	rtk_rg_flow_ACLFilterInfo_t	*pIgrACLFilterInfo;	/*useful if addSwOnly=1*/
	rtk_rg_flow_ACLFilterInfo_t	*pEgrACLFilterInfo;	/*useful if addSwOnly=1*/
	void *pAlgConn;	/*useful if addSwOnly=1*/
}rtk_rg_flow_extraInfo_t;


typedef struct rtk_rg_flow_linkList_s
{
	uint16 hashIdx;	//does not shift  ->  4K mode: 10bits, 8K mode: 13bits, 16K mode: 14bits, 32Kmode: 15bits.

	rtk_rg_flow_naptFilterInfo_t naptFilterInfo;
	rtk_rg_flow_ACLFilterInfo_t igrACLFilterInfo;
	rtk_rg_flow_ACLFilterInfo_t egrACLFilterInfo;
	void *pAlgConn;

#if MAX_FLOW_SW_TABLE_SIZE > 65536
	rtk_rg_list_longIdx_t flow_idxList;
#else
	rtk_rg_list_t flow_idxList;
#endif
}rtk_rg_flow_linkList_t;

typedef struct rtk_rg_flow_tcpUdpTracking_group_linkList_s
{
	rtk_rg_list_t group_idxList;
	rtk_rg_list_head_t flowTcpUdpTrackingHead;

	uint32 sip;
	uint32 dip;

	uint16 sport;
	uint16 dport;

	uint32 isL2OrL3				:1;	// 0: L2, 1: L3
	uint32 isV4OrV6				:1;	// 0: IPv4, 1: IPv6
	uint32 isTcp			    :1;	// 0: UDP, 1: TCP
	rtk_rg_tcpState_t tcpState	:3;
	uint32 group_idx 			:MAX_FLOW_TCP_UDP_TRACKING_GROUP_SHIFT;	// 14
	uint32 idleSecs				:12;

	uint32 src_cpri_en			:1;
	uint32 src_cpri				:3;
	uint32 dst_cpri_en			:1;
	uint32 dst_cpri				:3;

}rtk_rg_flow_tcpUdpTracking_group_linkList_t;
typedef struct rtk_rg_flow_tcpUdpTracking_linkList_s
{
#if MAX_FLOW_SW_TABLE_SIZE > 65536
	rtk_rg_list_longIdx_t flow_idxList;
#else
	rtk_rg_list_t flow_idxList;
#endif
}rtk_rg_flow_tcpUdpTracking_linkList_t;

typedef struct rtk_rg_naptRecorded_flow_linkList_s
{
	//struct list_head flow_list;
#if MAX_FLOW_SW_TABLE_SIZE > 65536
	rtk_rg_list_longIdx_t flow_idxList;
#else
	rtk_rg_list_t flow_idxList;
#endif
}rtk_rg_naptRecorded_flow_linkList_t;
#if defined(CONFIG_RG_FLOW_4K_MODE)
typedef struct rtk_rg_flowTcam_linkList_s
{
	uint16 idx;		//from MAX_FLOW_TABLE_SIZE to MAX_FLOW_TABLE_SIZE+MAX_FLOW_TCAM_TABLE_SIZE
	uint16 hashIdx;	//does not shift  ->  4K mode: 10bits, 8K mode: 13bits, 16K mode: 14bits, 32Kmode: 15bits.

	struct list_head flowTcam_list;
}rtk_rg_flowTcam_linkList_t;
#endif
typedef struct rtk_rg_log_connection_list_s
{
	unsigned char connection[RTK_RG_LOG_FLOW_STRING_SIZE];
	struct list_head log_list;
}rtk_rg_log_connection_list_t;
#endif

#if defined(CONFIG_APOLLO_GPON_FPGATEST)
typedef struct rtk_rg_vmac_skb_linlList_s
{
	struct sk_buff *skb;

	struct list_head vmac_list;
}rtk_rg_vmac_skb_linlList_t;
#endif

typedef struct rtk_rg_table_wantype_s
{

	rtk_wanType_entry_t rtk_wantype;
	//sw
	rtk_rg_entryStatus_t valid; // used to software entry
} rtk_rg_table_wantype_t;
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
typedef struct rtk_rg_naptInbound_entry_s {
	uint32  intIp;
    uint16  intPort;
    uint16  remHash;
    uint8   extIpIdx:MAX_EXTIP_SW_TABLE_SIZE_SHIFT;
    uint8	extPortLSB;
    uint8	extPortHSB;
	uint8	isTcp:1;
    uint8 	valid:2;
    uint8  	priValid:1;
    uint8  	priId:3;
} rtk_rg_naptInbound_entry_t;
typedef struct rtk_rg_naptOutbound_entry_s {
    uint16	    hashIdx:MAX_NAPT_IN_SW_TABLE_SHIFT;
    uint8	    valid:1;
    uint8	    priValid:1;
    uint8	    priValue:3;
} rtk_rg_naptOutbound_entry_t;
#endif
typedef struct rtk_rg_table_naptIn_s
{
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	rtk_rg_naptInbound_entry_t rtk_naptIn; //use self-defined data sture for saving memory usage
#else
    rtk_l34_naptInbound_entry_t rtk_naptIn;
#endif
    uint32 hashIdx: MAX_NAPT_IN_SW_ENTRY_WIDTH; //naptIn hashed value, range from 0 ~ MAX_NAPT_IN_HW_TABLE_SIZE>>2 //13
    uint32 idleSecs: MAX_NAPT_IN_IDLESEC_WIDTH; // 12
    uint32 refCount: MAX_NAPT_IN_REFCOUNT_WIDTH;    // 1
    uint32 coneType: MAX_NAPT_IN_CONETYPE_WIDTH;    // 2
    uint32 canBeReplaced: MAX_NAPT_IN_CANBEREPLACE_WIDTH;   // 1
    uint32 priValid: MAX_NAPT_IN_PRIVALID_WIDTH;            // 1
    uint32 cannotAddToHw: MAX_NAPT_IN_CANTADDTOHW_WIDTH;    //0: add to hw if necessary, 1: do not add to hw due to DPI check // 1
    uint32 priValue: MAX_NAPT_IN_PRIVALUE_WIDTH;    // 3
    uint32 symmetricNaptOutIdx: MAX_NAPT_IN_NAPTOUTIDX_WIDTH;   // 15
} rtk_rg_table_naptIn_t;

typedef struct rtk_rg_table_naptOut_s
{
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	rtk_rg_naptOutbound_entry_t	rtk_naptOut; //use self-defined data sture for saving memory usage
#else
	rtk_l34_naptOutbound_entry_t rtk_naptOut;
#endif
	uint16 remotePort;
	uint32 urlPriEn:1;
	uint32 urlPriIdx:8;
	ipaddr_t remoteIp;
	void *pContext;
	uint32 state: MAX_NAPT_OUT_STATE_WIDTH;     // 4
	uint32 extPort: MAX_NAPT_OUT_PORT_WIDTH;    // 16
  	uint32 idleSecs: MAX_NAPT_OUT_IDLESEC_WIDTH;    // 12

	uint32 hashOutIdx: MAX_NAPT_OUT_SW_ENTRY_WIDTH;	//naptOut hashed value, range from 0 ~ MAX_NAPT_OUT_HW_TABLE_SIZE>>2 // 13
	uint32 canBeReplaced: MAX_NAPT_OUT_CANBEREPLACE_WIDTH; // 1
	uint32 priValid: MAX_NAPT_OUT_PRIVALID_WIDTH;   // 1
	uint32 priValue: MAX_NAPT_OUT_PRIVALUE_WIDTH;   // 3
	uint32 cannotAddToHw: MAX_NAPT_OUT_CANTADDTOHW_WIDTH;	//0: add to hw if necessary, 1: do not add to hw due to DPI check, 2: do not add to hw but forward by shortcut  // 2
	uint32 recordedInLimitCount: MAX_NAPT_OUT_RECORD_WIDTH;	// 1: recoreded in napt access limit count
	uint32 naptrLookupHit: MAX_NAPT_OUT_LOOKUPHIT_WIDTH;  //0:non-hit, 1:hit first-callback(UPnP), 2:hit second-callback(Virtual Server), 3:hit third-callback(DMZ)
	uint32 forceExtPort: MAX_NAPT_OUT_FORCE_EXTPORT_WIDTH; // 1: forcibly use the specific ext port

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	uint32 src_cpri_en: 		MAX_NAPT_OUT_CPRI_EN_WIDTH;	// for local side
	uint32 src_cpri:			MAX_NAPT_OUT_CPRI_WIDTH;
	uint32 dst_cpri_en:			MAX_NAPT_OUT_CPRI_EN_WIDTH;	// for remote side
	uint32 dst_cpri:			MAX_NAPT_OUT_CPRI_WIDTH;
	rtk_rg_list_head_t flowListHead;
#endif
} rtk_rg_table_naptOut_t;

#if defined(CONFIG_RG_NAPT_NEW_EXTPORT_MECHANISM)
typedef struct rtk_rg_napt_extPort_record_s
{
	uint16 refCnt; // valid if refCnt > 0
	uint16 refCnt_force;
	uint32 internalIp;
	uint16 internalPort;

	struct list_head extPort_list;
}rtk_rg_napt_extPort_record_t;
#endif

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)

/*
	This Table store
	1.normal_multicast_entry
	2.static_multicast(API ADD)
	3.pppoePassthrough(unicast but need to rtl_rg_mcDataReFlushAndAdd )
*/
typedef struct rtk_rg_table_mcFlowIdxMapping_s
{
	int8  	vaild;
	int32	path3Idx;	//-1 invaild
	int32	path4Idx;	//-1 invaild
	int32	path6Idx;	//-1 invaild
	int16	lutIdx;		//-1 invaild
	int16   lutSaIdx;	//-1 invaild
	int8	path3ExtMskIdx;
	int8	path4ExtMskIdx;
	uint8 	isIpv6;
	uint8	staticEnty;	//for rg_api add
	uint8	pppoePassthroughEntry;
	uint32  multicastAddress[4];
	uint32  multicastSrcAddr[4];
	uint32  macPmsk;
	uint8	careSip;
	uint8	isIVL;		//not support
	uint16  fid_vlan;	//not support
#if defined(CONFIG_RG_G3_SERIES)
	rtk_rg_mcEngine_info_t *pMcEngineInfo;  //for hardware infomation
#endif
	int16   skbLen;
	uint8	flowDataSpa;
	uint8	streamId_llid;
	uint8 	flowDataBuf[DEFAULT_MAX_DATA_PATTEN];
} rtk_rg_table_mcFlowIdxMapping_t;

#endif

typedef struct rtk_rg_table_naptOut_linkList_s
{
	struct rtk_rg_table_naptOut_linkList_s *pNext;
#if 0 //use address-base to get index for saving memory usages
	uint16 idx;
#endif
} rtk_rg_table_naptOut_linkList_t;

typedef struct rtk_rg_table_naptIn_linkList_s
{
	struct rtk_rg_table_naptIn_linkList_s *pNext;
#if 0 //use address-base to get index for saving memory usages
	uint16 idx;
#endif
} rtk_rg_table_naptIn_linkList_t;

typedef struct rtk_rg_table_bind_s
{
	rtk_binding_entry_t rtk_bind;

	//SW table specific
	rtk_rg_entryStatus_t valid; // used to software entry
} rtk_rg_table_bind_t;

typedef struct rtk_rg_table_v6route_s
{
	rtk_ipv6Routing_entry_t rtk_v6route;
	int internal; //support for IPv6 NAPT, 0:internal  1:external
	rtk_ipv6_addr_t gateway_ipv6Addr;	//20160601LUKE: used to compare gateway IP or not
	rtk_rg_entryStatus_t valid; // used to software entry
} rtk_rg_table_v6route_t;

typedef struct rtk_rg_table_v6ExtIp_s
{
	int valid;
	rtk_ipv6_addr_t	externalIp;
	int nextHopIdx;
} rtk_rg_table_v6ExtIp_t;//support for IPv6 NAPT

typedef rtk_rg_neighborInfo_t rtk_rg_table_v6neighbor_t;


#if defined(CONFIG_RG_RTL9602C_SERIES)
typedef struct rtk_rg_table_dslite_s
{
	rtk_l34_dsliteInf_entry_t rtk_dslite;
	int intfIdx;	//which interface point to this DSlite entry
} rtk_rg_table_dslite_t;
#endif
typedef struct rtk_rg_table_dsliteMc_s
{
	rtk_l34_dsliteMc_entry_t rtk_dsliteMc;
	//rtk_ipv6_addr_t ipUPrefix64_AND_mask;
	//rtk_ipv6_addr_t ipMPrefix64_AND_mask;
} rtk_rg_table_dsliteMc_t;

typedef struct rtk_rg_staticRoute_s{
	union{
		struct{
			ipaddr_t addr;
			ipaddr_t mask;
			ipaddr_t nexthop;
			unsigned int localroute_intfidx;	//for local-route interface
			unsigned int exclude_subnet:1;		//1: force the IP range as RP
		}ipv4;
		struct{
			rtk_ipv6_addr_t addr;
			int mask_length;
			rtk_ipv6_addr_t nexthop;
			unsigned int linklocal_intfidx;	//for non-autolearn v6 link-local address or local-route interface
		}ipv6;
	};
	rtk_mac_t nexthop_mac;
	rtk_rg_port_idx_t nexthop_port;	//for non-autolearn

	unsigned int ip_version:1;		//0: ipv4, 1: ipv6
	unsigned int nexthop_mac_auto_learn:1;
}rtk_rg_staticRoute_t;

typedef struct rtk_rg_table_staticRoute_s
{
	rtk_rg_staticRoute_t info;

	rtk_rg_entryStatus_t valid;
	unsigned int route_idx:MAX_L3_SW_TABLE_SIZE_SHIFT;		//v4 or v6
	unsigned int nxtip_rtidx:MAX_L3_SW_TABLE_SIZE_SHIFT;		//v4 or v6, used for autolearn
	unsigned int nxtip_intfidx:MAX_NETIF_SW_TABLE_SIZE_SHIFT;	//v4 or v6, used for autolearn
	int nxtip_ipidx:MAX_EXTIP_SW_TABLE_SIZE_SHIFT+1;		//-1 means no iptable
}rtk_rg_table_staticRoute_t;

/* End of Tables ============================================================== */

/* System ============================================================== */
#ifdef CONFIG_SMP

typedef enum rtk_rg_inbound_queue_type_e
{
	RG_TASKLET_TYPE_FROM_NIC=0,
	RG_TASKLET_TYPE_FROM_WIFI,
	RG_TASKLET_TYPE_FROM_PS,
	RG_TASKLET_TYPE_TIMER,
}rtk_rg_inbound_queue_type_t;

typedef enum rtk_rg_smp_jobs_type_e
{

	RG_SMP_WIFI_RX=0,
	RG_SMP_GMAC_RX,
	RG_SMP_FROM_PS_RX,

	RG_SMP_WIFI_RX_IN_RG,
	RG_SMP_GMAC_RX_IN_RG,
	RG_SMP_FROM_PS_RX_IN_RG,
	RG_SMP_IPI_WIFI_RX_IN_RG,
	RG_SMP_IPI_GMAC_RX_IN_RG,
	RG_SMP_IPI_FROM_PS_RX_IN_RG,
	RG_SMP_TIMER,
	RG_SMP_WAIT_WQRX_UNLOCK,
	RG_SMP_WAIT_RG_UNLOCK,
	RG_SMP_IPI_WAIT_RX_UNLOCK,
	RG_SMP_IPI_WAIT_RX_HI_UNLOCK,
	RG_SMP_WIFI_RX_DROP,
	RG_SMP_GMAC_RX_DROP,
	RG_SMP_TIMER_DROP,
	RG_SMP_FROM_PS_DROP,

	RG_SMP_IPI_WIFI_RX_DROP,
	RG_SMP_IPI_GMAC_RX_DROP,
	RG_SMP_IPI_TIMER_DROP,
	RG_SMP_IPI_FROM_PS_DROP,
	RG_SMP_GMAC0_TX,
	RG_SMP_GMAC1_TX,
	RG_SMP_GMAC2_TX,
	RG_SMP_WLAN0_TX,
	RG_SMP_WLAN1_TX,
	RG_SMP_IPI_TO_PS_TX,

	RG_SMP_WAIT_GMAC0_UNLOCK,
	RG_SMP_WAIT_GMAC1_UNLOCK,
	RG_SMP_WAIT_GMAC2_UNLOCK,
	RG_SMP_WAIT_WLAN0_UNLOCK,
	RG_SMP_WAIT_WLAN1_UNLOCK,
	RG_SMP_GMAC0_TX_DROP,
	RG_SMP_GMAC1_TX_DROP,
	RG_SMP_GMAC2_TX_DROP,
	RG_SMP_WLAN0_TX_DROP,
	RG_SMP_WLAN1_TX_DROP,
//workqueue
	RG_SMP_GMAC9_TX,
	RG_SMP_GMAC10_TX,
	RG_SMP_WIFI_11AC_TX,
	RG_SMP_WIFI_11N_TX,
	RG_SMP_TO_PS_TX,

	RG_SMP_WAIT_GMAC9_UNLOCK,
	RG_SMP_WAIT_GMAC10_UNLOCK,
	RG_SMP_WAIT_WIFI_11AC_UNLOCK,
	RG_SMP_WAIT_WIFI_11N_UNLOCK,
	RG_SMP_GMAC9_TX_DROP,
	RG_SMP_GMAC10_TX_DROP,
	RG_SMP_WIFI_11AC_TX_DROP,
	RG_SMP_WIFI_11N_TX_DROP,

	RG_SMP_JOBS_TYPE_MAX
} rtk_rg_smp_jobs_type_t;
#if 1
#ifdef __KERNEL__
typedef struct rtk_rg_smp_rx_private_s
{
	struct sk_buff *skb;
	rtk_rg_rxdesc_t rxInfo;
	rtk_rg_inbound_queue_type_t tasklet_type;

	struct list_head rxpriv_list;
}rtk_rg_smp_rx_private_t;

typedef struct rtk_rg_smp_tx_private_s
{
	struct sk_buff *skb;
	union{
		struct{
			struct tx_info txInfo;
			int ring_num;
		}gmac;
		struct net_device *wlanDev;
	};

	struct list_head txpriv_list;
}rtk_rg_smp_tx_private_t;

typedef struct rtk_rg_smp_ctrl_s
{
	spinlock_t queue_lock;
	atomic_t queue_entrance;
	struct tasklet_struct tasklet;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 3)
	call_single_data_t csd;
#else
	struct call_single_data csd;
#endif
	atomic_t csd_pending;
}rtk_rg_smp_ctrl_t;
#endif
//workqueue
struct rg_private
{
	atomic_t start_index,end_index;
	struct sk_buff *skb[MAX_RG_INBOUND_QUEUE_SIZE];
	rtk_rg_rxdesc_t rxInfo[MAX_RG_INBOUND_QUEUE_SIZE];
	rtk_rg_inbound_queue_type_t tasklet_type[MAX_RG_INBOUND_QUEUE_SIZE];	//0:from NIC, 1:from WIFI, 2:from timer, 3:from ps
#ifdef RG_BY_TASKLET
#else
#ifdef __KERNEL__
	struct work_struct rg_work[MAX_RG_INBOUND_QUEUE_SIZE];
#endif
	int work_busy[MAX_RG_INBOUND_QUEUE_SIZE];
#endif
};

struct rg_timer_private
{
	atomic_t start_index,end_index;
#ifdef RG_BY_TASKLET
#else
#ifdef __KERNEL__
	struct work_struct rg_work[MAX_RG_TIMER_QUEUE_SIZE];
#endif
	int work_busy[MAX_RG_TIMER_QUEUE_SIZE];
#endif
};

struct rg_gmac9_outbound_queue_private
{
	atomic_t start_index,end_index;
	struct sk_buff *skb[MAX_RG_GMAC9_OUTBOUND_QUEUE_SIZE];
	struct tx_info ptxInfo[MAX_RG_GMAC9_OUTBOUND_QUEUE_SIZE];
	int ring_num[MAX_RG_GMAC9_OUTBOUND_QUEUE_SIZE];
	int work_busy[MAX_RG_GMAC9_OUTBOUND_QUEUE_SIZE];
#ifdef __KERNEL__
#ifdef OUTBOUND_BY_TASKLET
#else
#ifdef __KERNEL__
	struct work_struct gmac_tx_work[MAX_RG_GMAC9_OUTBOUND_QUEUE_SIZE];
#endif
#endif
#endif
};

struct rg_gmac10_outbound_queue_private
{
	atomic_t start_index,end_index;
	struct sk_buff *skb[MAX_RG_GMAC10_OUTBOUND_QUEUE_SIZE];
	struct tx_info ptxInfo[MAX_RG_GMAC10_OUTBOUND_QUEUE_SIZE];
	int ring_num[MAX_RG_GMAC10_OUTBOUND_QUEUE_SIZE];
	int work_busy[MAX_RG_GMAC9_OUTBOUND_QUEUE_SIZE];
#ifdef __KERNEL__
#ifdef OUTBOUND_BY_TASKLET
#else
#ifdef __KERNEL__
	struct work_struct gmac_tx_work[MAX_RG_GMAC10_OUTBOUND_QUEUE_SIZE];
#endif
#endif
#endif
};


struct rg_wifi_11ac_outbound_queue_private
{
	atomic_t start_index,end_index;

#ifdef __KERNEL__
struct sk_buff *skb[MAX_RG_WIFI_11AC_OUTBOUND_QUEUE_SIZE];
struct net_device *dev[MAX_RG_WIFI_11AC_OUTBOUND_QUEUE_SIZE];

#ifdef OUTBOUND_BY_TASKLET
#else
#ifdef __KERNEL__
	struct work_struct wifi_11ac_tx_work[MAX_RG_WIFI_11AC_OUTBOUND_QUEUE_SIZE];
#endif
	int work_busy[MAX_RG_WIFI_11AC_OUTBOUND_QUEUE_SIZE];
#endif
#endif

};

struct rg_wifi_11n_outbound_queue_private
{
	atomic_t start_index,end_index;

#ifdef __KERNEL__
struct sk_buff *skb[MAX_RG_WIFI_11N_OUTBOUND_QUEUE_SIZE];
struct net_device *dev[MAX_RG_WIFI_11N_OUTBOUND_QUEUE_SIZE];

#ifdef OUTBOUND_BY_TASKLET
#else
#ifdef __KERNEL__
	struct work_struct wifi_11n_tx_work[MAX_RG_WIFI_11N_OUTBOUND_QUEUE_SIZE];
#endif
	int work_busy[MAX_RG_WIFI_11N_OUTBOUND_QUEUE_SIZE];
#endif
#endif

};
#endif
#endif

typedef int (*p_GWMACRequestCallBack)(ipaddr_t, int);		//IP address, Lut table idx

typedef struct rtk_rg_arp_request_s
{
	ipaddr_t reqIp;
	int volatile finished; //used to indicate the ARP request return or not
	p_GWMACRequestCallBack	gwMacReqCallBack;
	unsigned int disableL3Inspect:1;		//using passing parameters directly
} rtk_rg_arp_request_t;

typedef int (*p_IPV6GWMACRequestCallBack)(unsigned char*, int);		//IP address, Lut table idx

typedef struct rtk_rg_neighbor_discovery_s
{
	rtk_ipv6_addr_t reqIp;
	int volatile finished; //used to indicate the Neighbor Discovery return or not
	p_IPV6GWMACRequestCallBack	ipv6GwMacReqCallBack;
} rtk_rg_neighbor_discovery_t;


typedef struct rtk_rg_interface_info_global_s
{
	rtk_rg_intfInfo_t storedInfo;
	rtk_rg_interface_Status_t valid;	// vaild mask indicate v4/v6 software entry
	int lan_or_wan_index;		//index of lan or wan group
	union{
		rtk_rg_lanIntfConf_t *p_lanIntfConf;
		rtk_rg_ipStaticInfo_t *p_wanStaticInfo;
	};
#if defined(CONFIG_RG_G3_SERIES)
	int32 clsIdx_option_trap[RTK_RG_PORT_MAX];			// 0 if invalid
	int32 lanWan_groupIdx_for_genericIntf;	//for maintain generic interface
#endif
} rtk_rg_interface_info_global_t;

typedef struct rtk_rg_wan_interface_group_info_s
{
	rtk_rg_interface_info_global_t *p_intfInfo;
	int index;		//index of netif table
	rtk_rg_wanIntfConf_t *p_wanIntfConf;
	unsigned int disableBroadcast:1;		//only for BD WAN which has same VLANID as LAN intf
} rtk_rg_wan_interface_group_info_t;

typedef struct rtk_rg_lan_interface_group_info_s
{
	rtk_rg_interface_info_global_t *p_intfInfo;
	int index;
} rtk_rg_lan_interface_group_info_t;

typedef struct rtk_rg_virtual_server_info_s
{
	rtk_rg_virtualServer_t* p_virtualServer;
	int index;
} rtk_rg_virtual_server_info_t;

typedef struct rtk_rg_upnp_info_s
{
	rtk_rg_upnpConnection_t* p_upnp;
	int index;
} rtk_rg_upnp_info_t;

typedef struct rtk_rg_port_proto_vid_s
{
	rtk_vlan_protoVlanCfg_t protoVLANCfg[MAX_PORT_PROTO_GROUP_SIZE];

}rtk_rg_port_proto_vid_t;

typedef struct rtk_rg_routing_arpInfo_s
{
	int routingIdx;
	int intfIdx;
	ipaddr_t notMask;		//0000..111, the "NOT" result of network mask
	int bitNum;				//the bits number of "zero" in network mask, /24=8, /25=7,.../30=2 => 31 minus ipMask in routing_entry
	int arpStart;
	int	arpEnd;
	unsigned char isLan;		//for LAN:1, for WAN:0
} rtk_rg_routing_arpInfo_t;

typedef struct rtk_rg_routing_linkList_s		//used when transfer sw ARP to hw ARP
{
	uint8 idx;
	uint8 bitNum;
	struct list_head route_list;
}rtk_rg_routing_linkList_t;

typedef enum rtk_rg_sa_learning_exceed_action_e
{
	SA_LEARN_EXCEED_ACTION_PERMIT=0,
	SA_LEARN_EXCEED_ACTION_PERMIT_L2,
	SA_LEARN_EXCEED_ACTION_DROP,
	SA_LEARN_EXCEED_ACTION_END,
}rtk_rg_sa_learning_exceed_action_t;

typedef enum rtk_rg_accessWanLimitType_e
{
	RG_ACCESSWAN_TYPE_UNLIMIT=0,	//turn off
	RG_ACCESSWAN_TYPE_PORT,			//deprecated, keep for backward-compatible
	RG_ACCESSWAN_TYPE_PORTMASK,
	RG_ACCESSWAN_TYPE_CATEGORY,
	RG_ACCESSWAN_TYPE_END,
}rtk_rg_accessWanLimitType_t;

typedef enum rtk_rg_accessWanLimitField_e
{
	RG_ACCESSWAN_LIMIT_BY_SMAC=0,
	RG_ACCESSWAN_LIMIT_BY_SIP,
	RG_ACCESSWAN_LIMIT_END,
}rtk_rg_accessWanLimitField_t;

//802.1x Port based network access control 
typedef struct rtk_rg_accessDot1xCtrl_s
{	//Port based access blocking (pormask or port index): enable or disable
	rtk_rg_portmask_t port_mask;
	rtk_rg_mac_port_idx_t port_idx;	//if port_mask = 0x0, check port_idx
	rtk_rg_enable_t port_blocking;
} rtk_rg_accessDot1xCtrl_t;

typedef enum rtk_rg_accessDot1xFilterFlag_e
{
	ACCESS_DOT1X_FILTER_FLAG_ADD = 0,
	ACCESS_DOT1X_FILTER_FLAG_DEL_ONE,
	ACCESS_DOT1X_FILTER_FLAG_DEL_ALL,
	ACCESS_DOT1X_FILTER_FLAG_DUMP,
	ACCESS_DOT1X_FILTER_FLAG_MAX
} rtk_rg_accessDot1xFilterFlag_t;

typedef struct rtk_rg_accessDot1xFilter_s
{	//MAC based access unblocking (whitelist)
	rtk_rg_accessDot1xFilterFlag_t flag;
	rtk_mac_t mac;
} rtk_rg_accessDot1xFilter_t;

#if defined(CONFIG_RG_8021X_MAC_TABLE_SIZE) && (CONFIG_RG_8021X_MAC_TABLE_SIZE!=0)
typedef struct rtk_rg_accessDot1xCfg_s
{	//rg db global structure
	rtk_rg_portmask_t blockingPortmask;
	uint32 unblockingMacValid[(RTK_RG_MAX_DOT1X_MAC_ENTRY_SIZE/32)+1]; //valid bit for unblockingMac
	rtk_mac_t unblockingMac[RTK_RG_MAX_DOT1X_MAC_ENTRY_SIZE];
} rtk_rg_accessDot1xCfg_t;
#endif

typedef enum rtk_rg_qosWeightSelection_e
{
	WEIGHT_OF_PORTBASED,
	WEIGHT_OF_DOT1Q,
	WEIGHT_OF_DSCP,
	WEIGHT_OF_ACL,
	WEIGHT_OF_LUTFWD,
	WEIGHT_OF_SABASED,
	WEIGHT_OF_VLANBASED,
	WEIGHT_OF_SVLANBASED,
	WEIGHT_OF_L4BASED,
	WEIGHT_OF_END,
} rtk_rg_qosWeightSelection_t;


typedef enum rtk_rg_qosDscpRemarkSrcSelect_e
{
	DISABLED_DSCP_REMARK=0,
	ENABLED_DSCP_REMARK_AND_SRC_FROM_INTERNALPRI,
	ENABLED_DSCP_REMARK_AND_SRC_FROM_DSCP,
	ENABLED_DSCP_REMARK_AND_SRC_FROM_ACL,
} rtk_rg_qosDscpRemarkSrcSelect_t;


typedef struct rtk_rg_qosInternalDecision_s		//used when SW Qos decision
{
	//internal pri decision
	uint8 internalPri;
	uint8 internalPriSelectWeight[WEIGHT_OF_END];
	uint8 qosDot1pPriRemapToInternalPriTbl[8];
	uint8 qosDscpRemapToInternalPri[64];
	uint8 qosPortBasedPriority[RTK_RG_MAC_PORT_MAX];

	//remarking by internal
	rtk_rg_qosDscpRemarkSrcSelect_t qosDscpRemarkEgressPortEnableAndSrcSelect[RTK_RG_MAC_PORT_MAX];
	rtk_rg_enable_t qosDot1pPriRemarkByInternalPriEgressPortEnable[RTK_RG_MAC_PORT_MAX];
	uint8 qosDot1pPriRemarkByInternalPri[8];
	uint8 qosDscpRemarkByInternalPri[8];
	uint8 qosDscpRemarkByDscp[64];


}rtk_rg_qosInternalDecision_t;

typedef struct rtk_rg_cvidCpri2SidMapping_s{
	uint16 cvid;
	uint8 cpri;
	uint8 sid;
}rtk_rg_cvidCpri2SidMapping_t;

typedef enum  rtk_rg_gatewayServiceType_e
{
	GATEWAY_SERVER_SERVICE	= 0,	//lookup dport
	GATEWAY_CLIENT_SERVICE	= 1,	//lookup sport
}rtk_rg_gatewayServiceType_t;

typedef struct rtk_rg_gatewayServicePortEntry_s{
	int valid;
	unsigned short int port_num;
	rtk_rg_gatewayServiceType_t type;
}rtk_rg_gatewayServicePortEntry_t;

//for get and set rg init state APIs
typedef enum rtk_rg_initState_e
{
	RTK_RG_DURING_INIT=0,
	RTK_RG_INIT_FINISHED,
}rtk_rg_initState_t;


#define REG_SHIFT_BASE_1 0xa0046266
#define REG_SHIFT_BASE_2 0xa0010676
#define REG_SHIFT_BASE_3 0xa0016485
#define REG_SHIFT_1 (REG_SHIFT_BASE_1 - 0x00020000)
#define REG_SHIFT_2 (REG_SHIFT_BASE_1 - 0x00010000)
#define REG_SHIFT_3_0 0x00000000
#define REG_SHIFT_3_1 0x00001000
#define REG_SHIFT_3_2 0x00002000
#define REG_SHIFT_3_3 0x00003000



typedef enum rtk_rg_internal_support_bit_e
{
	RTK_RG_INTERNAL_SUPPORT_BIT0=(1<<0),
	RTK_RG_INTERNAL_SUPPORT_BIT1=(1<<1),
	RTK_RG_INTERNAL_SUPPORT_BIT2=(1<<2),
	RTK_RG_INTERNAL_SUPPORT_BIT3=(1<<3),
	RTK_RG_INTERNAL_SUPPORT_BIT4=(1<<4),
	RTK_RG_INTERNAL_SUPPORT_BIT5=(1<<5),
}rtk_rg_internal_support_bit_t;



typedef struct rtk_rg_wlan_binding_s
{
	unsigned int exist:1;			//0:device not exist, 1:device exist
	unsigned int set_bind:1;		//0:not binding to WAN, 1:binding

	int bind_wanIntf;
} rtk_rg_wlan_binding_t;


typedef enum rtk_rg_ipsec_passthru_e
{
    PASS_OLD = 0,		//old style: not work now!
    PASS_ENABLE,		//enable passthru
    PASS_DISABLE,		//disable passthru
    PASS_END			//end value
} rtk_rg_ipsec_passthru_t;

typedef enum rtk_rg_callback_function_idx_e
{
	INIT_BY_HW_CALLBACK_IDX = 0,						// _rtk_rg_initParameterSetByHwCallBack
	ARP_ADD_BY_HW_CALLBACK_IDX,						// _rtk_rg_arpAddByHwCallBack
	ARP_DEL_BY_HW_CALLBACK_IDX,						// _rtk_rg_arpDelByHwCallBack
	MAC_ADD_BY_HW_CALLBACK_IDX,						// _rtk_rg_macAddByHwCallBack
	MAC_DEL_BY_HW_CALLBACK_IDX,						// _rtk_rg_macDelByHwCallBack
	ROUTING_ADD_BY_HW_CALLBACK_IDX,						// _rtk_rg_routingAddByHwCallBack
	ROUTING_DEL_BY_HW_CALLBACK_IDX,						// _rtk_rg_routingDelByHwCallBack
	NAPT_ADD_BY_HW_CALLBACK_IDX,						// _rtk_rg_naptAddByHwCallBack
	NAPT_DEL_BY_HW_CALLBACK_IDX,						// _rtk_rg_naptDelByHwCallBack
	BINDING_ADD_BY_HW_CALLBACK_IDX,						// _rtk_rg_bindingAddByHwCallBack
	BINDING_DEL_BY_HW_CALLBACK_IDX,						// _rtk_rg_bindingDelByHwCallBack
	INTERFACE_ADD_BY_HW_CALLBACK_IDX,					// _rtk_rg_interfaceAddByHwCallBack
	INTERFACE_DEL_BY_HW_CALLBACK_IDX,					// _rtk_rg_interfaceDelByHwCallBack
	NEIGHBOR_ADD_BY_HW_CALLBACK_IDX,					// _rtk_rg_neighborAddByHwCallBack
	NEIGHBOR_DEL_BY_HW_CALLBACK_IDX,					// _rtk_rg_neighborDelByHwCallBack
	V6_ROUTING_ADD_BY_HW_CALLBACK_IDX,					// _rtk_rg_v6RoutingAddByHwCallBack
	V6_ROUTING_DEL_BY_HW_CALLBACK_IDX,					// _rtk_rg_v6RoutingDelByHwCallBack
	DHCP_REQUEST_BY_HW_CALLBACK_IDX,					// _rtk_rg_dhcpRequestByHwCallBack
	PPPOE_BEFORE_DIAG_BY_HW_CALLBACK_IDX,					// _rtk_rg_pppoeBeforeDiagByHwCallBack
	PPTP_BEFORE_DIAG_BY_HW_CALLBACK_IDX,					// _rtk_rg_pptpBeforeDialByHwCallBack
	L2TP_BEFORE_DIAG_BY_HW_CALLBACK_IDX,					// _rtk_rg_l2tpBeforeDialByHwCallBack
	PPPOE_DSLITE_BEFORE_DIAG_BY_HW_CALLBACK_IDX,				// _rtk_rg_pppoeDsliteBeforeDialByHwCallBack
	SOFTWARE_NAPT_INFO_ADD_CALLBACK_IDX,					// _rtk_rg_softwareNaptInfoAddCallBack
	SOFTWARE_NAPT_INFO_DEL_CALLBACK_IDX,					// _rtk_rg_softwareNaptInfoDeleteCallBack
	NAPT_PREROUTE_DPI_CALLBACK_IDX,						// _rtk_rg_naptPreRouteDPICallBack
	NAPT_FORWARD_DPI_CALLBACK_IDX,						// _rtk_rg_naptForwardDPICallBack
	PPPOE_LCP_STATE_CALLBACK_IDX,						// _rtk_rg_pppoeLCPStateCallBack
#ifdef CONFIG_YUEME_DPI
	DPI_NAPT_INFO_ADD_CALLBACK_IDX,						// DPI_naptInfoAddCallBack
	DPI_NAPT_INFO_DEL_CALLBACK_IDX,						// DPI_naptInfoDeleteCallBack
	DPI_NAPT_PREROUTING_CALLBACK_IDX,					// DPI_naptPreRouteCallBack
	DPI_NAPT_FORWARDING_CALLBACK_IDX					// DPI_naptForwardCallBack
#endif
} rtk_rg_callback_function_idx_t;

typedef struct rtk_rg_statistic_s
{
	//Packet Type
	uint32 perPortCnt_broadcast[RTK_RG_PORT_MAX];
	uint32 perPortCnt_mass_broadcast[RTK_RG_PORT_MAX];		//count which broadcast packet size bigger or equal than MASS_PACKET_SIZE
	uint32 perPortCnt_multicast[RTK_RG_PORT_MAX];
	uint32 perPortCnt_mass_multicast[RTK_RG_PORT_MAX];		//count which multicast packet size bigger or equal than MASS_PACKET_SIZE
	uint32 perPortCnt_unicast[RTK_RG_PORT_MAX];
	uint32 perPortCnt_mass_unicast[RTK_RG_PORT_MAX];		//count which unicast packet size bigger or equal than MASS_PACKET_SIZE
	uint32 perPortCnt_swLimt[RTK_RG_PORT_MAX];
	uint32 perPortCnt_zeroLen[RTK_RG_PORT_MAX];
	uint32 perPortCnt_UDP[RTK_RG_PORT_MAX];
	uint32 perPortCnt_TCP[RTK_RG_PORT_MAX];
	uint32 perPortCnt_ICMP_REQ[RTK_RG_PORT_MAX];
	uint32 perPortCnt_ICMP_REP[RTK_RG_PORT_MAX];	
	uint32 perPortCnt_Tx_broadcast[RTK_RG_PORT_MAX];
	uint32 perPortCnt_Tx_mass_broadcast[RTK_RG_PORT_MAX];		//count which broadcast packet size bigger or equal than MASS_PACKET_SIZE
	uint32 perPortCnt_Tx_multicast[RTK_RG_PORT_MAX];
	uint32 perPortCnt_Tx_mass_multicast[RTK_RG_PORT_MAX];		//count which multicast packet size bigger or equal than MASS_PACKET_SIZE
	uint32 perPortCnt_Tx_unicast[RTK_RG_PORT_MAX];
	uint32 perPortCnt_Tx_mass_unicast[RTK_RG_PORT_MAX]; 	//count which unicast packet size bigger or equal than MASS_PACKET_SIZE

	//TCP Flags
	uint32 perPortCnt_SYN[RTK_RG_PORT_MAX];
	uint32 perPortCnt_SYN_ACK[RTK_RG_PORT_MAX];
	uint32 perPortCnt_FIN[RTK_RG_PORT_MAX];
	uint32 perPortCnt_FIN_ACK[RTK_RG_PORT_MAX];
	uint32 perPortCnt_FIN_PSH_ACK[RTK_RG_PORT_MAX];
	uint32 perPortCnt_RST[RTK_RG_PORT_MAX];
	uint32 perPortCnt_RST_ACK[RTK_RG_PORT_MAX];
	uint32 perPortCnt_ACK[RTK_RG_PORT_MAX];

	//ARP/NB
	uint32 perPortCnt_ARP_request[RTK_RG_PORT_MAX];
	uint32 perPortCnt_ARP_reply[RTK_RG_PORT_MAX];
	uint32 perPortCnt_NB_solicitation[RTK_RG_PORT_MAX];
	uint32 perPortCnt_NB_advertisement[RTK_RG_PORT_MAX];

	//Reason
	uint32 perPortCnt_Reason[256][RTK_RG_EXT_PORT2]; //TTL

	//forwarding
	uint32 perPortCnt_slowPath[RTK_RG_PORT_MAX];
	uint32 perPortCnt_shortcut[RTK_RG_PORT_MAX];
	uint32 perPortCnt_shortcutv6[RTK_RG_PORT_MAX];
	uint32 perPortCnt_L2FWD[RTK_RG_PORT_MAX];
	uint32 perPortCnt_IPv4_L3FWD[RTK_RG_PORT_MAX];
	uint32 perPortCnt_IPv6_L3FWD[RTK_RG_PORT_MAX];
	uint32 perPortCnt_L4FWD[RTK_RG_PORT_MAX];
	uint32 perPortCnt_Drop[RTK_RG_PORT_MAX];
	uint32 perPortCnt_MC_SMAC_Drop[RTK_RG_PORT_MAX];	//packet with source mac as multicast mac address
	uint32 perPortCnt_MCBC_DROP[RTK_RG_PORT_MAX];	//packet is not forward to any port(excludes cpu port)
	uint32 perPortCnt_MC_DATA_FWD[RTK_RG_PORT_MAX];
	uint32 perPortCnt_ToPS[RTK_RG_PORT_MAX];
	uint32 perPortCnt_CopyToPS[RTK_RG_PORT_MAX];
	uint32 perPortCnt_naptOutLRU[RTK_RG_PORT_MAX];
	uint32 perPortCnt_naptInLRU[RTK_RG_PORT_MAX];
	uint32 perPortCnt_v4ShortcutLRU[RTK_RG_PORT_MAX];
	uint32 perPortCnt_v6ShortcutLRU[RTK_RG_PORT_MAX];
	uint32 perPortCnt_dpiPreRouteHook[RTK_RG_PORT_MAX];
	uint32 perPortCnt_dpiForwardHook[RTK_RG_PORT_MAX];

	//skb prealloc,alloc,free
	uint32 perPortCnt_skb_pre_alloc_for_jumbo[RTK_RG_PORT_MAX];
	uint32 perPortCnt_skb_pre_alloc_for_uc[RTK_RG_PORT_MAX];
	uint32 perPortCnt_skb_pre_alloc_for_mc_bc[RTK_RG_PORT_MAX];
	uint32 perPortCnt_skb_alloc[RTK_RG_PORT_MAX];
	uint32 perPortCnt_skb_free[RTK_RG_PORT_MAX];

	//nicTx, wifiTx
	uint32 perPortCnt_NIC_TX[RTK_RG_PORT_MAX];
	uint32 perPortCnt_WIFI_TX[RTK_RG_PORT_MAX];

	//IPv4 fragment queued
	uint32 perPortCnt_v4FragQueued[RTK_RG_PORT_MAX];
	uint32 perPortCnt_L2L3FragQueuedReentrance[RTK_RG_PORT_MAX];	//count the L2L3 queued packet reenter fwdEngine
	uint32 perPortCnt_L4FragQueuedReentrance[RTK_RG_PORT_MAX];		//count the L4queued packet reenter fwdEngine

#if defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION)&&defined(CONFIG_MASTER_WLAN0_ENABLE)
	//masterWifiFastForward
	uint32 perPortCnt_MWFF_TX[RTK_RG_PORT_MAX];
#endif
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	uint32 perPortCnt_NPTv6_FF_TX[RTK_RG_PORT_MAX];
	uint32 perPortCnt_NPTv6_NIC_FF_TX[RTK_RG_PORT_MAX];
#endif
}rtk_rg_statistic_t;

typedef struct rtk_rg_redirectHttpAll_s
{
	char pushweb[MAX_REDIRECT_PUSH_WEB_SIZE];
	unsigned int enable:2;	//0:disable, 1:enable, 2:enable with URL
	int8 count;	//redirect times, -1: unlimit
}rtk_rg_redirectHttpAll_t;

typedef struct rtk_rg_redirectHttpURL_s
{
	char url_str[MAX_URL_FILTER_STR_LENGTH];
	char dst_url_str[MAX_URL_FILTER_STR_LENGTH];
	int16 count;	//-1: always redirect, 0: stop redirect
}rtk_rg_redirectHttpURL_t;

typedef struct rtk_rg_redirectHttpURL_linkList_s
{
	rtk_rg_redirectHttpURL_t url_data;
	atomic_t count;	//for atomic operation
	int16 url_len;
	struct list_head url_list;
}rtk_rg_redirectHttpURL_linkList_t;

typedef struct rtk_rg_redirectHttpWhiteList_s
{
	char url_str[MAX_URL_FILTER_STR_LENGTH];
	char keyword_str[MAX_URL_FILTER_STR_LENGTH];
}rtk_rg_redirectHttpWhiteList_t;

typedef struct rtk_rg_redirectHttpWhiteList_linkList_s
{
	rtk_rg_redirectHttpWhiteList_t white_data;
	int16 url_len;
	int16 keyword_len;
	struct list_head white_list;
}rtk_rg_redirectHttpWhiteList_linkList_t;

typedef struct rtk_rg_redirectHttpRsp_s
{
	char url_str[MAX_URL_FILTER_STR_LENGTH];
	unsigned int enable:1;	//0:disable, 1:enable
	unsigned int statusCode:10;
}rtk_rg_redirectHttpRsp_t;

typedef struct rtk_rg_redirectHttpCount_s
{
	char pushweb[MAX_REDIRECT_PUSH_WEB_SIZE];
	unsigned int enable:2;	//0:disable, 1:enable, 2:enable with URL
	unsigned int denialSecs:4;
	int8 count;	//redirect times, -1: unlimit
}rtk_rg_redirectHttpCount_t;

typedef struct rtk_rg_avoidPortalURL_linkList_s
{
	char url_str[MAX_URL_FILTER_STR_LENGTH];
	int16 url_len;
	struct list_head avoidPortal_list;
}rtk_rg_avoidPortalURL_linkList_t;

typedef struct rtk_rg_hostPoliceControl_s
{
	rtk_mac_t macAddr;
	rtk_enable_t ingressLimitCtrl;
	rtk_enable_t egressLimitCtrl;
	rtk_enable_t mibCountCtrl;
	int limitMeterIdx;
}rtk_rg_hostPoliceControl_t;

typedef struct rtk_rg_hostPoliceLogging_s
{
	uint64 rx_count;
	uint64 tx_count;
}rtk_rg_hostPoliceLogging_t;

typedef struct rtk_rg_hostPoliceLinkList_s
{
	unsigned int idx;
	unsigned int lut_idx;
	unsigned int lut_idx_learned:1;	//1: lut_index is valid, 0:lut_index is invalid
#if defined(CONFIG_RG_G3_SERIES)
	int32 loggingRx_policerIdx:10; //stored flow policer index for RX counting the of MAC address, -1 mean n.a.
	int32 loggingTx_policerIdx:10; //stored flow policer index for TX counting the of MAC address, -1 mean n.a.
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
	rtk_rg_hostPoliceControl_l2_cls_idx_t l2ClsIndex;
#endif
#endif
	rtk_rg_hostPoliceControl_t info;
	rtk_rg_hostPoliceLogging_t count;
	struct list_head host_list;
}rtk_rg_hostPoliceLinkList_t;

typedef struct rtk_rg_dosRateLimit_s
{
	//action
	int32 shareMeterIdx;	 // -1:diable rate limit,  0~31: shareMeter index
	int32 shareMeterIdx_hw;	// for hw acl
	//int32 byteCount;
	// pattern
	uint32 portmask;
	uint8 ctagif;
	uint32 ctagVid;
	rtk_mac_t ingressDmac;
	uint32 ingressDip;
	uint8 isTcp;	// 0: UDP, 1: TCP
	uint32 pktLenStart;
	uint32 pktLenEnd;
}rtk_rg_dosRateLimit_t;

typedef struct rtk_rg_trapSpecificLengthAck_s
{
	uint32 portMask;
	uint32 pktLenStart;
	uint32 pktLenEnd;
	uint8 ackOnly;
	uint8 isTrap;
	int priority;
}rtk_rg_trapSpecificLengthAck_t;

typedef struct rtk_rg_trapSpecificLengthSyn_s
{
	uint32 portMask;
	uint32 pktLenStart;
	uint32 pktLenEnd;
	uint8 synOnly;
	uint8 isTrap;
	int priority;
}rtk_rg_trapSpecificLengthSyn_t;

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
typedef struct rtk_rg_wifi_flow_crtl_func_s
{
	uint16 wlan0_flow_ctrl_on_threshold_mbps; //on threshold for wlan0 (unit: mbps)
	uint16 wlan0_flow_ctrl_off_threshold_mbps; //off threshold for wlan0 (unit: mbps)
	uint16 wlan1_flow_ctrl_on_threshold_mbps; //on threshold for wlan1 (unit: mbps)
	uint16 wlan1_flow_ctrl_off_threshold_mbps; //off threshold for wlan1 (unit: mbps)
	uint32 wlan0_accumulate_bit; //wlan0 accumulated packet bit count between wifi flow control detect interval
	uint32 wlan1_accumulate_bit; //wlan1 accumulated packet bit count between wifi flow control detect interval
	rtk_rg_enable_t wifi_flow_ctrl_auto_en; //if enable wifi flow control auto enabling mechanism
}rtk_rg_wifi_flow_crtl_func_t;
#endif

typedef struct rtk_rg_vlanGroupMacLimit_info_s
{
	unsigned int vlanMask[MAX_VLAN_HW_TABLE_SIZE>>5];
	unsigned int untag:1;

	unsigned int valid:1;

	rtk_rg_mac_port_idx_t port;
	int mac_limit_number;
	atomic_t mac_count;
}rtk_rg_vlanGroupMacLimit_info_t;

typedef struct rtk_rg_vlanGroupMacLimit_group_s
{
	rtk_rg_vlanGroupMacLimit_info_t group_info;
	struct list_head mac_head;
}rtk_rg_vlanGroupMacLimit_group_t;

typedef struct rtk_rg_vlanGroupMacLimit_mac_s
{
	rtk_mac_t mac;
	int vlanId;	//-1 means untag
	struct list_head mac_list;
	struct list_head group_list;

	rtk_rg_vlanGroupMacLimit_group_t *pGroup;
}rtk_rg_vlanGroupMacLimit_mac_t;

#if 1
typedef struct rtk_rg_nptv6_acceleration_s
{
	rtk_mac_t dmac;
	uint16 vaild:1;
	uint16 ipv6_hdr_offset:15;
	rtk_ipv6_addr_t ipv6_addr; //sip for upstream, dip for downstream
	int32 virtual_dmacL2Idx;
	struct tx_info txInfo;
}rtk_rg_nptv6_acceleration_t;

typedef struct rtk_rg_nptv6_acceleration_nic_flow_s
{
	unsigned char preallocated_outer[64];
	unsigned char preallocated_outer_length;
	rtk_mac_t smac;
	rtk_mac_t dmac;
	rtk_ipv6_addr_t sipv6;
	rtk_ipv6_addr_t dipv6;
	uint8 isTcp;
}rtk_rg_nptv6_acceleration_nic_flow_t;

typedef struct rtk_rg_nptv6_acceleration_data_s
{
	//nptv6 acceleration mechanism
	rtk_rg_enable_t nptv6_acceleration_mechanism;
	rtk_rg_nptv6_acceleration_t nptv6_acceleration_upstream[MAX_NPTV6_ACC_UPSTREAM_SIZE];
	rtk_rg_nptv6_acceleration_t nptv6_acceleration_downstream[MAX_NPTV6_ACC_DOWNSTREAM_SIZE];
	rtk_rg_nptv6_acceleration_t nptv6_acceleration_upstream0[MAX_NPTV6_ACC_UPSTREAM0_SIZE];
	rtk_rg_nptv6_acceleration_t nptv6_acceleration_downstream0[MAX_NPTV6_ACC_DOWNSTREAM0_SIZE];
	
	rtk_rg_nptv6_acceleration_nic_flow_t nptv6_acceleration_nic_flow_upstream[MAX_NPTV6_ACC_UPSTREAM_NIC_FLOW_NUM];
	rtk_rg_nptv6_acceleration_nic_flow_t nptv6_acceleration_nic_flow_downstream[MAX_NPTV6_ACC_DOWNSTREAM_NIC_FLOW_NUM];
	rtk_rg_nptv6_acceleration_nic_flow_t nptv6_acceleration_nic_flow_upstream0[MAX_NPTV6_ACC_UPSTREAM0_NIC_FLOW_NUM];
	rtk_rg_nptv6_acceleration_nic_flow_t nptv6_acceleration_nic_flow_downstream0[MAX_NPTV6_ACC_DOWNSTREAM0_NIC_FLOW_NUM];

	unsigned char preallocated_outer[64];
	unsigned char preallocated_outer_length;

	uint8 upstreamNicFlowNum;
	uint8 upstreamExtport;
	uint8 upstreamCpuport;
	uint8 upstreamGmac;
	uint8 upstreamRxRingNum;
	uint8 upstreamTxRingNum;
	uint8 upstreamPriority;
	uint8 upstreamPriority_rg;
	
	uint8 downstreamNicFlowNum;
	uint8 downstreamExtport;
	uint8 downstreamCpuport;
	uint8 downstreamGmac;
	uint8 downstreamRxRingNum;
	uint8 downstreamTxRingNum;
	uint8 downstreamPriority;
	uint8 downstreamPriority_rg;

	uint8 upstream0NicFlowNum;
	uint8 upstream0Extport;
	uint8 upstream0Cpuport;
	uint8 upstream0Gmac;
	uint8 upstream0RxRingNum;
	uint8 upstream0TxRingNum;
	uint8 upstream0Priority;
	uint8 upstream0Priority_rg;
	
	uint8 downstream0NicFlowNum;
	uint8 downstream0Extport;
	uint8 downstream0Cpuport;
	uint8 downstream0Gmac;
	uint8 downstream0RxRingNum;
	uint8 downstream0TxRingNum;
	uint8 downstream0Priority;
	uint8 downstream0Priority_rg;

	int32 meterIdx_fromLan[4];
}rtk_rg_nptv6_acceleration_data_t;

typedef struct rtk_rg_vxlan_acceleration_data_s
{
	int vxlan_upstream_txAddrOffset;
	int vxlan_downstream_txAddrOffset;
	unsigned int vxlan_accelerated_intf_idx;
	unsigned char vxlan_upstream_dmac[4];
	
	unsigned char vxlan_preallocated_outer[64];				//dmac6 + smac6 + eth2 + cvlan4 + svlan4 + ip20 + udp8
	unsigned char vxlan_extra_preallocated_outer[64];		//dmac6 + smac6 + eth2 + cvlan4 + svlan4 + ip20 + udp8
	unsigned char vxlan_preallocated_outer_length;
	unsigned char vxlan_preallocated_outer_ip_length_offset;
	unsigned char vxlan_preallocated_outer_udp_length_offset;

	unsigned char vxlan_upstreamGmac;
	unsigned char vxlan_upstreamRxRingNum;
	unsigned char vxlan_upstreamTxRingNum;
	//unsigned char vxlan_upstreamExtport;
	unsigned char vxlan_upstreamPriority;
	
	unsigned char vxlan_downstreamGmac;
	unsigned char vxlan_downstreamRxRingNum;
	unsigned char vxlan_downstreamTxRingNum;
	//unsigned char vxlan_downstreamExtport;
	unsigned char vxlan_downstreamPriority;

	unsigned char vxlan_extraGmac;
	unsigned char vxlan_extra_upstreamRxRingNum;
	unsigned char vxlan_extra_upstreamTxRingNum;
	unsigned char vxlan_extra_upstreamPriority;
	unsigned char vxlan_extra_downstreamRxRingNum;
	unsigned char vxlan_extra_downstreamTxRingNum;
	unsigned char vxlan_extra_downstreamPriority;

	unsigned int vxlan_upstreamInfo_prepared:1;
	unsigned int vxlan_downstreamInfo_prepared:1;
	unsigned int vxlan_upstreamRing_modified:1;
	unsigned int vxlan_downstreamRing_modified:1;
	unsigned int vxlan_extra_upstreamInfo_prepared:1;
	unsigned int vxlan_extra_downstreamInfo_prepared:1;
	unsigned int vxlan_extra_upstreamRing_modified:1;
	unsigned int vxlan_extra_downstreamRing_modified:1;
	unsigned int vxlan_acceleration_mechanism:1;
	unsigned int vxlan_acceleration_extraPmsk:5;		//lan port0 ~ port 4
	int vxlan_acceleration_extraMeter:7;

	rtk_rg_txdesc_t vxlan_upstream_txInfo;
	rtk_rg_txdesc_t vxlan_downstream_txInfo;
}rtk_rg_vxlan_acceleration_data_t;

typedef struct rtk_rg_dynamic_sram_data_s
{
	rtk_rg_nptv6_acceleration_data_t nptv6_acc;
	rtk_rg_vxlan_acceleration_data_t vxlan_acc;
}rtk_rg_dynamic_sram_data_t;
#endif

typedef struct rtk_rg_forciblyMacBindingToWan_s
{
	unsigned char valid;
	unsigned char src_mac[ETHER_ADDR_LEN];
}rtk_rg_forciblyMacBindingToWan_t;

typedef enum rtk_rg_forciblyMacBindingModeBitMask_e
{
	FORCIBLY_BIND_MODE_V4_BITMASK			=0x1,
	FORCIBLY_BIND_MODE_V6_BITMASK			=0x2,
	FORCIBLY_BIND_MODE_STRIP_BITMASK		=0x4,
	FORCIBLY_BIND_MODE_DROPUNBIND_BITMASK	=0x8,
	FORCIBLY_BIND_MODE_BITMASK_END			=0x10
}rtk_rg_forciblyMacBindingModeBitMask_t;

typedef enum rtk_rg_forciblyMacBindingOptLenOP_e
{
	FORCIBLY_BIND_OP_EQUAL_BITMASK		=0x1,
	FORCIBLY_BIND_OP_LARGER_BITMASK		=0x2,
	FORCIBLY_BIND_OP_SMALLER_BITMASK	=0x4,
	FORCIBLY_BIND_OP_BITMASK_END		=0x8
}rtk_rg_forciblyMacBindingOptLenOP_t;

typedef struct rtk_rg_glb_system_s
{
	/* System Module */
	rtk_rg_sipDipClassification_t sipDipClass[MAX_SIP_CLASS][MAX_DIP_CLASS];
	unsigned int phyPortStatus;	//0:disable 1:enable

	rtk_rg_initParams_t initParam;
	unsigned int nxpRefCount[MAX_NEXTHOP_SW_TABLE_SIZE];		//store how many struct reference each nexthop entry
	unsigned int eipRefCount[MAX_EXTIP_SW_TABLE_SIZE];			//store how many struct reference each extip entry
	//int bindToIntf[MAX_BIND_HW_TABLE_SIZE];					//store what interface index this binding connected with
	//int bindWithVLAN[MAX_BIND_HW_TABLE_SIZE];				//store what vlan this binding rule used
	rtk_rg_arp_request_t intfArpRequest[MAX_NETIF_SW_TABLE_SIZE<<1];	//each interface may issue one ARP request		//over MAX_NETIF_SIZE belong to PPTP&L2TP
	rtk_rg_neighbor_discovery_t intfNeighborDiscovery[MAX_NETIF_SW_TABLE_SIZE<<1];	//each interface may issue one Neighbor Discovery	//over MAX_NETIF_SIZE belong to DSLITE
	rtk_rg_arp_request_t staticRouteArpReq[MAX_STATIC_ROUTE_SIZE];	//each static route may issue one ARP request
	rtk_rg_neighbor_discovery_t staticRouteNBDiscovery[MAX_STATIC_ROUTE_SIZE];	//each static route may issue one ARP request
	unsigned int rgInit;
	unsigned int vlanInit;
	unsigned int wanIntfTotalNum;
	unsigned int lanIntfTotalNum;
	unsigned int vlanBindTotalNum;			//indicate how many vlan-binding we have
	unsigned int ipv4FragmentQueueNum;		//indicate how many fragment packets in the queue
	//unsigned int pppoeBeforeCalled;			//indicate if we had called PPPoE Before
	unsigned int defaultTrapLUTIdx;			//indicate the index of default LUT used to trap to CPU
	unsigned int wanInfoSet;				//indicate which wan has been set info after add
	unsigned int nicIgmpModuleIndex;		//indicate IGMP snooping module's index after registration
	int defaultRouteSet;					//indicate which interface had set default route
	int defaultIPV6RouteSet;				//indicate which interface had set ipv6 default route
	int intfIdxForReset;						//indicate which wan has been reset and need to re-add in the same index
	rtk_rg_interface_info_global_t interfaceInfo[MAX_NETIF_SW_TABLE_SIZE];		//store each interface information, LAN or WAN
	rtk_rg_lan_interface_group_info_t lanIntfGroup[MAX_NETIF_SW_TABLE_SIZE];
	rtk_rg_wan_interface_group_info_t wanIntfGroup[MAX_NETIF_SW_TABLE_SIZE];
#if defined(CONFIG_RG_G3_SERIES)
	uint8 lanIntfGroup_for_genericIntf[MAX_LAN_INTERFACE_SIZE];
	uint8 wanIntfGroup_for_genericIntf[MAX_WAN_INTERFACE_SIZE];
#endif	
	int otherWanVlan[MAX_NETIF_SW_TABLE_SIZE];		//store Other Wan's VLANID used for traffic isolation
	rtk_rg_portmask_t wanPortMask,lanPortMask;
	//int bridgeWanNum;								//how many bridge WAN interface in system
	int8 vlan_proto_block_created;		//indicate the fwdVLAN_Proto_Block had been created or not
	int portBasedVID[RTK_RG_PORT_MAX];				//indicate which port-based VLAN ID should be used when untag(contain extension port)
	rtk_rg_port_proto_vid_t protoBasedVID[RTK_RG_PORT_MAX];	//indicate which port-and-protocol-based VLAN ID and GROUP ID would be used when match GROUP setting

#ifdef CONFIG_MASTER_WLAN0_ENABLE
	int wlan0DeviceBasedVID[MAX_WLAN_DEVICE_NUM];	//indicate which VLAN ID should be given when packet coming from WLAN
	rtk_rg_wlan_binding_t	wlan0BindDecision[MAX_WLAN_DEVICE_NUM];
	unsigned int	wlan0BindingUsed;	//record which wlan0 device is binded in bit mask.
#endif
	//rtk_rg_wlan_binding_t 	wlan1BindDecision[MAX_WLAN_DEVICE_NUM];

	rtk_vlan_protoGroup_t protoGroup[MAX_PORT_PROTO_GROUP_SIZE];	//port and protocol group settings

	int layer2HouseKeepIndex;						//indicate the index last house keep end
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	int 			v6StatefulHouseKeepIndex;					//indicate the list last house keep end
	atomic_t 		v6StatefulConnectionNum;				//total allocated connection number
	unsigned int 	ipv6FragmentQueueNum;				//indicate how many fragment packets in the queue
#endif
	int8 portbinding_wan_idx[RTK_RG_PORT_MAX];		//record this port is port-binding to which WAN
	rtk_rg_portmask_t	non_binding_pmsk;				//record which port is not binding to any WAN
	unsigned int	bindingUsed;					//record which bind entry is used or not in bit mask.

	rtk_l34_bindAct_t	l34BindAction[L34_BIND_TYPE_END];
#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	rtk_enable_t		l34GlobalState[L34_GLOBAL_STATE_END];
#else
	rtk_enable_t		fbGlobalState[FB_GLOBAL_STATE_END];
	rtk_enable_t 		flowCheckState[FB_FLOW_CHECK_END];
	uint32				preHashPtn[FB_PREHASH_PTN_END];
	rtk_rg_portmask_t wanAccessLimit_pmask;
	int extraTagActionList_netifIdx[MAX_EXTRATAG_TABLE_SIZE];	// save each action list was occupied by which wan interface, initialized as -1
#endif

	rtk_rg_portmask_t wanVlanMemAppend[MAX_NETIF_SW_TABLE_SIZE];	//append pmsk to wan's vlan member when RG update wan info
	rtk_portmask_t wanVlanMemAppend_origMem[MAX_NETIF_SW_TABLE_SIZE];	//keep original vlan member
	rtk_portmask_t wanVlanMemAppend_origExt[MAX_NETIF_SW_TABLE_SIZE];	//keep original vlan ext member

#if 0//def CONFIG_GPON_FEATURE
	int8 untagBridgeGponWanIdx;		//only needed for PON port
#endif

	//Control Path switches
	int ctrlPathByProtocolStack_broadcast;		//deafult 0, let fwdEngine handle broadcast packets
	int ctrlPathByProtocolStack_ICMP;			//default 0, let fwdEngine handle ICMP packets(except OverMTU packets)

	//Napt access limit
	int32 naptAccessLimitNumber;
	atomic_t naptAccessLimitCount;

	rtk_rg_accessWanLimitType_t activeLimitFunction;
	rtk_rg_accessWanLimitField_t activeLimitField;

	//Access wan limit
	rtk_rg_portmask_t accessWanLimitPortMask_member;
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	unsigned int accessWanLimitPortMask_wlan0member;
#endif
	int accessWanLimitPortMask;
	int accessWanLimitCategory[WanAccessCategoryNum];
	rtk_rg_sa_learning_exceed_action_t accessWanLimitPortMaskAction;
	rtk_rg_sa_learning_exceed_action_t accessWanLimitCategoryAction[WanAccessCategoryNum];
	atomic_t accessWanLimitPortMaskCount;
	atomic_t accessWanLimitCategoryCount[WanAccessCategoryNum];
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	int accessWanLimitPortBased[RTK_RG_PORT_MAX];
	rtk_rg_sa_learning_exceed_action_t accessWanLimitPortBasedAction[RTK_RG_PORT_MAX];
	atomic_t accessWanLimitPortBasedCount[RTK_RG_PORT_MAX];
#endif

#if defined(CONFIG_RG_8021X_MAC_TABLE_SIZE) && (CONFIG_RG_8021X_MAC_TABLE_SIZE!=0)
	//Access network control 802.1x
	rtk_rg_accessDot1xCfg_t accessDot1xCfg;
#endif

	//SA learning limit number - per port and control action - per port
	int sourceAddrLearningLimitNumber[RTK_RG_PORT_MAX];
	rtk_rg_sa_learning_exceed_action_t sourceAddrLearningAction[RTK_RG_PORT_MAX];
	atomic_t sourceAddrLearningCount[RTK_RG_PORT_MAX];
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	int wlan0SourceAddrLearningLimitNumber[MAX_WLAN_DEVICE_NUM];
	rtk_rg_sa_learning_exceed_action_t wlan0SourceAddrLearningAction[MAX_WLAN_DEVICE_NUM];
	atomic_t wlan0SourceAddrLearningCount[MAX_WLAN_DEVICE_NUM];
#endif

	//20161123LUKE: used to control all MACs in the same group of port or ext-port
	rtk_rg_portmask_t groupMACLimit_member;
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	unsigned int groupMACLimit_wlan0member;
#endif
	int groupMACLimit;
	atomic_t groupMACLimitCount;

	struct list_head vlanGroupMACLimit_macHead[MAX_LUT_HW_HASH_HEAD];		//indexed by HASH(IVL or SVL)
	rtk_rg_vlanGroupMacLimit_group_t vlanGroupMACLimit_group[MAX_VLAN_GROUP_MAC_LIMIT_NUMBER];

	unsigned int virtualServerTotalNum;
	unsigned int upnpTotalNum;
	rtk_rg_virtual_server_info_t virtualServerGroup[MAX_VIRTUAL_SERVER_SW_TABLE_SIZE];
	rtk_rg_upnp_info_t upnpGroup[MAX_UPNP_SW_TABLE_SIZE];
	struct list_head portTrigger_matched_head[4];		//00:incoming_UDP,01:incoming_TCP,10:outgoing_UDP,11:outgoing_TCP
	struct list_head portTrigger_related_head[2];		//0:UDP,1:TCP
	struct list_head *pPortTrigger_matched_head[4];		//00:incoming_UDP,01:incoming_TCP,10:outgoing_UDP,11:outgoing_TCP
	struct list_head *pPortTrigger_related_head[2];		//0:UDP,1:TCP

	//maintain ACL in fwdEngine
	int acl_SW_table_entry_size; //using for speed up RG_ACL which compared in fwdEninge.
	uint32 acl_SW_empty_start_index;		//using for speed up search RG_ACL empty rule
	int acl_SWindex_sorting_by_weight[MAX_ACL_SW_ENTRY_SIZE];	//The array record the RG_ACL rule index which sorted by weight. The ACL rule priority should handled by this sorting result.
	int acl_SWindex_sorting_by_weight_and_ingress_cvid_action[MAX_ACL_SW_ENTRY_SIZE];	//The array record the RG_ACL rule index which sorted by weight, and the acl ingress_cvid action is valid. use for speed up fwdEngine internalVid decision.
#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
	int acl_SW_egress_intf_type_zero_num;	//number of ACL rules of type zero with egress intf idx
#endif
#if defined(CONFIG_RG_RTL9600_SERIES)
	int acl_SW_egress_intf_transform;		//reorder acl_weight in _rtk_rg_rearrange_ACL_weight_for_egress_wan with egress intf idx
#endif
	uint32 cf_valid_mask[(TOTAL_CF_ENTRY_SIZE/32)+1]; //use for speed up CF[64-511] (or pattern 1) is need to check or not.
	uint32 stop_add_hw_acl;//0:it can try to add drop rule to HW ACL,   1:stop rest drop rule add to HW ACL.   use for apolloPro to determind add rest ACL drop rule to HW or not.
	uint8 stop_add_acl;//This parameter only check during add new rule. 0:it can continue to add acl, 1:stop add this rule due to rearrange fail

	rtk_rg_filterControlType_t urlFilterMode;

	rtk_rg_macFilterSWEntry_t macFilter_table_Entry[MAX_MAC_FILTER_ENTRY_SIZE];

	rtk_rg_hwnatState_t hwnat_enable, hwnat_enable_previous_state;
	rtk_rg_enable_t hwnat_statechange_without_flush_flow;
	rtk_rg_enable_t fragment_ipv4_intf_trap;
	rtk_rg_enable_t unknownDA_Trap_to_PS_enable;
	rtk_rg_enable_t igmp_Trap_to_PS_enable;
	rtk_rg_enable_t mld_Trap_to_PS_enable;
	rtk_rg_enable_t bc_dip_with_non_bc_dmac_Trap_to_PS_enable;
	rtk_rg_enable_t ipv6MC_translate_ingressVID_enable;
	rtk_rg_hwnatState_t strangeSA_drop;	//learned in LAN but come back in WAN
	rtk_rg_hwnatState_t fix_l34_to_untag_enable;
	int port_binding_by_protocal; //0:IPv4+IPv6 both bridge  1:binding IPv4, IPv6 Bridge 2:binding IPv6, IPv4 Bridge
	unsigned int port_binding_by_protocal_filter_vid; //filter port_binding_by_protocal with vid for downstream unicast. (for IPCP/IP6CP + vid donstream filter)
	rtk_rg_multicastProtocol_t multicastProtocol;	//0: IGMP & MLD, 1: IGMP, 2: MLD
	rtk_rg_multicastVersionSupport_t multicastVersionSupport;
	int ssdp_trap_lut_idx;
	rtk_rg_enable_t pppoe_bc_passthrought_to_bindingWan_enable; //learned in LAN but come back in WAN
	rtk_rg_enable_t pppoe_mc_routing_trap; //trap PPPoE DS mutlicast routing packet (DA=gmac, DIP=multicast)
#if defined(RTK_RG_INGRESS_QOS_TEST_PATCH) && defined(CONFIG_RG_RTL9600_SERIES)
	int qos_type;
	int qos_acl_patch[RTK_RG_MAC_PORT_MAX][9];
	int qos_acl_total_patch;
	int qos_ingress_rate[RTK_RG_MAC_PORT_MAX];
	int qos_ingress_total_rate;
#endif
	int acl_filter_idx_for_hwnat;		//Default set to 0 now
	int not_disconnect_ppp;		//use for callback delete interface, in case "Server disconnect ppp" will not need to call spppctl.
#ifdef CONFIG_RG_CALLBACK
	int callback_regist; //for check callback function is registered, then /bin/spppctl & /bin/udhcpc  should not do hwnat again! : Chuck
#endif
	rtk_rg_stormControlInfo_t  stormControlInfoEntry[MAX_STORMCONTROL_ENTRY_SIZE];
	rtk_rg_qosInternalDecision_t qosInternalDecision;

	rtk_rg_acl_reserved_global_t aclAndCfReservedRule;
	int aclPolicingMode;
#if defined(CONFIG_RG_RTL9602C_SERIES)
	int mib_l2_wanif_idx;
	int mib_mc_wanif_idx;
	int cf_pattern0_size;
#endif

	unsigned int tpid;	//record SVLAN VS_TPID
	unsigned int tpid2;	//record SVLAN VS_TPID2
	unsigned int tpid2_en;	//record SVLAN VS_TPID2 enabled or not
	rtk_rg_portmask_t service_pmsk;				//record Service PortMask
	rtk_rg_portmask_t svlan_EP_DMAC_CTRL_pmsk;	//record SVLAN_EP_DMAC_CTRL PortMask

	//timeout
#if defined(CONFIG_ROME_NAPT_SHORTCUT)
	int v4ShortCut_timeout;
	int v4Shoutcut_HouseKeepIndex; //indicate the index last house keep end
#endif
#if defined(CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT)
	int v6ShortCut_timeout;
	int v6Shoutcut_HouseKeepIndex; //indicate the index last house keep end
#endif
#if defined(CONFIG_RG_FLOW_AUTO_AGEOUT)
	int flow_timeout;
	int flow_HouseKeepIndex; //indicate the index last house keep end
	int swFlow_HouseKeepIndex; //indicate the index last house keep end
#endif
	int arp_timeout;
	int neighbor_timeout;
	int tcp_long_timeout;
	int tcp_short_timeout;
	int udp_long_timeout;
	int udp_short_timeout;
#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support lut traffic bit
	int l2_timeout;
#endif
	unsigned long fragment_timeout;
	int alg_sip_presreve_timeout;

	//Disable TCP stateful tracking of napt/L2L3 tcp connection, create connection while any packet send from LAN.
	unsigned int tcpDisableStatefulTracking:1;

	//Disable TCP simultaneous SYN packet from existing connection.
	unsigned int tcpDisableSimultaneousSYN:1;

	//Keep original cvlan for packets from protocol stack (can be modified by ACL/CF)
	unsigned int keepPsOrigCvlan:1;

	//check http mechanism(redirect, force_portal...) for http request to protocol stack.
	unsigned int checkHttpMechanismBeforeEnterPs:1;

	//pppoe proxy only accept dial packet which is binding to pppoe WAN
	unsigned int pppoeProxyAllowBindingOnly:1;

	uint32 httpMonitorPort;
	uint32 httpsMonitorPort;

	//Choice other inbound hashIdx when inbound 4-way full.
	uint32 enableL4ChoiceHwIn;
	uint32 tcpDoNotDelWhenRstFin:1;
	uint32 tcpSwapFinDelRst:1;

	//NAPT short-timeout house keep
	uint32 tcpShortTimeoutHousekeepJiffies;  //0:disable

	// statistic
	uint32 fwdStatistic;
	rtk_rg_portmask_t fwdStatisticZeroPmsk;

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	// flow statistic
	uint32 flowStatistic;
#endif

	//wan access limit discovery off-line host interval
	int wanAccessLimit_interval;

	//ARP Request Timer
	int arp_requset_interval_sec;

	//ARP for auto test fail Timer
	int auto_test_fail_arp_interval_sec;

	//ICMP tracking timeout
	int icmp_timeout;

	//egressPort Qid for auto tect check
	rtk_rg_mac_port_idx_t egrPortCheckForAutoTest;
	int intPriCheckForAutoTest;

	//fwdRsn for auto tect check
	int fwdrsnCheckForAutoTest;

	//HouseKeeping Timer
	int house_keep_sec;

	//IGMP Snooping Timer
	int igmp_sys_timer_sec;

	//IGMP max flow size
	int igmp_max_simultaneous_group_size;

	//IGMP/MLD PerPort Max Group size   0=> unlimit
	int16 igmp_max_simultaneous_group_size_perPort[RTK_RG_PORT_MAX];
	int16 mld_max_simultaneous_group_size_perPort[RTK_RG_PORT_MAX];

	//IGMP System Client Limit
	int igmp_max_system_client_size;

	//MLD System Client Limit
	int mld_max_system_client_size;

	//IGMP specificClient join to group limit
	int igmp_max_specific_client_join_group_size;

	//MLD specificClient join to group limit
	int mld_max_specific_client_join_group_size;

#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	int igmp_pppoe_passthrough_learning;					//9607C support Dmac==Uc and dip==Mc and do not care pppoe header
	uint16 pppoe_passthrough_learning_reservedACL_bitmap;
#endif

	//Multicast Query Timer
	int mcast_query_sec;

	// forward reserved mutlicast address to pptp port by hardware
	int mcast_resvedAddr_hwFwd2pptp;

	int mcast_resvedMac_hwFwd2pptp;

	//auto learn report packet ctagif
	int igmp_auto_learn_ctagif;

	int igmp_ingoreSpecificQueryUpdateTimer;

	//User defined force Client report time after query send
	int forceReportResponseTime;

	// TCP hw learning at SYN
	int tcp_hw_learning_at_syn;

	// TCP inbound shortcut learning at SYN
	unsigned int tcp_in_shortcut_learning_at_syn:1;
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM) && !defined(CONFIG_RG_G3_SERIES)
	// TCP SYN ACK forward or trap
	unsigned int tcp_hw_syn_ack_forward:1;		//0:trap, 1:forward if hit flow
	uint8 tcp_max_tiny_ack_size;					//0:normal, others: tiny ack within this packet size
#endif
#ifdef CONFIG_ROME_NAPT_SHORTCUT
	//IPv4 shortcut switch
	unsigned int ipv4_shortcut_off:1;
#endif
	//Fragment shortcut switch
	unsigned int fragShortcut_off:1;
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
	//IPv6 shortcut switch
	unsigned int ipv6_shortcut_off:1;
#endif

	unsigned int gatherLanNetInfo:1;
	unsigned int forciblyMacBindingToWan:1;
	unsigned int forciblyMacBindingToWanModeMask:4;		//bit0:v4 bit1:v6 bit2:strip option bit3:dropUnBind
	unsigned int forciblyMacBindingToWanKeyMatch:1;		//0:disabled 1:compare key
	unsigned int forciblyMacBindingWanIdx:MAX_NETIF_SW_TABLE_SIZE_SHIFT;

	unsigned int forceWifiUntag:1;
	unsigned int keepWifiUntagToPS:1;		//if wifi send untag packet and need to enter protocol stack, we can keep it untag by this.
#if defined(CONFIG_MASTER_WLAN0_ENABLE) && defined(CONFIG_RG_FLOW_NEW_WIFI_MODE)
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	unsigned int disableWifiRxAcc:1;
	unsigned int disableWifiRxDistributed:1;
	unsigned int disableWifiTxDistributed:1;
#if defined(CONFIG_RG_FLOW_ENHANCED_WIFI_MODE)
	unsigned int disableSlaveWifiRxAcc:1;
	unsigned int disableWifiRxHwlookupHash:2;
	unsigned int masterWifiRRFlag:1;
#endif
#endif
#endif
#if defined(CONFIG_SMP)
	unsigned int distributedMechanism:1;		//0:IPI, 1:work queue
	unsigned int disableNicTxDistributed:1;
	unsigned int protocolStackBypassRxQueue:1;
	unsigned int disableFwdEngineDistributed:1;
	unsigned int criticalPacketBypassRxQueue:3;
#endif
	unsigned int pppoeGponSmallbandwithControl:1;

	unsigned int pppoeGponSmallbandwithControlSupportBrigeSameMac:1;

	unsigned int gpon_pppoe_status:2;

	unsigned int enableSlaveSSIDBind:1;

	unsigned int checkWifiDev:1;

	unsigned int remoteGatewayMacStatically:1;		//0:always update, 1:do not update

	unsigned int localInNaptAddToShortcut:1;

	unsigned int whiteListState:1;

	unsigned int inboundL4UnknownUdpConnDrop:1;		//0:trap, 1:drop

	unsigned int disableSWPBOAutoConf:1;

	unsigned int trapICMPWhenNeighMiss:1;		//0:default drop, 1:trap

	unsigned int forcedlyTrapICMPv6Echo:1;		//0:normal, 1:always trap

	unsigned int forcedlyDropICMPv6RSFromWAN:1;		//0:normal, 1:force drop

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	unsigned int forcedlyCFpattern1Override:1;	//0:normal, 1:override cf action

	unsigned int replaceHwFlow:1;	// replace Hw flow with Sw flow when it is deleted.
	unsigned int L2TcpUdpStatefulTracking:2;	// 0: disable check, 1: enable stateful, 2: enable firewall
	unsigned int L3TcpUdpStatefulTracking:2;	// 0: disable check, 1: enable stateful, 2: enable firewall
	unsigned int flow_not_update_in_real_time:1;	// skip update flow if it's busy
	unsigned int flow_based_cpri_decision_portmask;
	unsigned int flow_flush_disable_by_feature:3;	// skip flush flow by feature, please refer RTK_RG_FFD_BIT_xxx
	unsigned int aclTrapToPs_acc:1;
#else
	unsigned int shortcut_flush_disable_by_feature:3;	// skip flush shortcut by feature, please refer RTK_RG_FSD_BIT_xxx
#endif

	//software port isolation
	rtk_rg_portmask_t portIsolation[RTK_RG_PORT_MAX];

	//software rate limit
	int BCRateLimitPortMask;
	int BCRateLimitShareMeterIdx; // -1:diable rate limit,  0~31: shareMeter index
	//int BCByteCount;
	int IPv6MCRateLimitPortMask;
	int IPv6MCRateLimitShareMeterIdx; // -1:diable rate limit,  0~31: shareMeter index
	//int IPv6MCByteCount;
	int IPv4MCRateLimitPortMask;
	int IPv4MCRateLimitShareMeterIdx; // -1:diable rate limit,  0~31: shareMeter index
	//int IPv4MCByteCount;
	int unKnownDARateLimitPortMask;
	int unKnownDARateLimitShareMeterIdx; // -1:diable rate limit,  0~31: shareMeter index
	//int unKnownDAByteCount;
	unsigned long unlearnedSAJiffies;
	int unlearnedSALimit;		// -1:disable limit
	int unlearnedSACount;
	int overMTURateLimitPortMask;
	int overMTURateLimitShareMeterIdx; // -1:diable rate limit,  0~31: shareMeter index
	//int overMTUByteCount;
	int ArpReqRateLimitPortMask;
	int ArpReqRateLimitShareMeterIdx; // -1:diable rate limit,  0~31: shareMeter index
	//int ArpReqByteCount;
	int igmpRateLimitPortMask;
	int igmpRateLimitShareMeterIdx; // -1:diable rate limit,  0~31: shareMeter index
	//int igmpByteCount;
	int dhcpRateLimitPortMask;
	int dhcpRateLimitShareMeterIdx; // -1:diable rate limit,  0~31: shareMeter index
	//int dhcpByteCount;
	rtk_rg_dosRateLimit_t dosRateLimit[RTK_RG_MAX_DOS_RATE_LIMIT_TABLE_SIZE];
	int dosRateLimitCount;
	int synRateLimitShareMeterIdx; // -1:diable rate limit,  0~31: shareMeter index

	//software dos
	rtk_rg_mac_portmask_t swDosPortMask;
	rtk_rg_dos_action_t swDosAction[RTK_RG_DOS_TYPE_MAX];
	//uint32 swDosFloodThreshold[RTK_RG_DOSFLOOD_TYPE_MAX];

	unsigned int dos_TCPSYNPktThreshold;		//0:disabled
	unsigned int dos_TCPSYNPktThresholdUnit;
	unsigned long dos_TCPSYNJiffies;			//record
	unsigned int dos_TCPSYNPktCnt;
	unsigned int dos_TCPFINPktThreshold;		//0:disabled
	unsigned int dos_TCPFINPktThresholdUnit;
	unsigned long dos_TCPFINJiffies;			//record
	unsigned int dos_TCPFINPktCnt;
	unsigned int dos_ICMPPktThreshold;		//0:disabled
	unsigned int dos_ICMPPktThresholdUnit;
	unsigned long dos_ICMPJiffies;			//record
	unsigned int dos_ICMPPktCnt;

	uint32 naptSwRateLimitTriggered; //0: no naptFilter rule enabled rate limit, no need to enabled timer to clear naptSwRateLimitByteCount[]
	uint32 naptSwRateLimitSpeed[MAX_NAPT_FILER_SW_ENTRY_SIZE];	//0:unlimit, other value(unit Kbps): the speed that should be limited for all flows that hit such naptFilter. (per naptFilter entry has one)
	uint32 naptSwRateLimitByteCount[MAX_NAPT_FILER_SW_ENTRY_SIZE];	//accumulate byte count for all flows such hit naptFilter: per naptFilter entry has one (per naptFilter entry has one)

#ifdef CONFIG_MASTER_WLAN0_ENABLE
	int wifiIngressRateLimitDevMask;
	int wifiIngressRateLimitDevOverMask;	//record which device had over meter
	int wifiIngressRateLimitMeter[MAX_WLAN_DEVICE_NUM];
	int wifiIngressByteCount[MAX_WLAN_DEVICE_NUM];
	int wifiEgressRateLimitDevMask;
	int wifiEgressRateLimitDevOverMask;	//record which device had over meter
	int wifiEgressRateLimitMeter[MAX_WLAN_DEVICE_NUM];
	int wifiEgressByteCount[MAX_WLAN_DEVICE_NUM];
	
	uint32 copyToPSWifiDevMask; 	//copy packet from or to these wifi device toward protocol stack.
#endif

	//L2 Interface p-bit remarking (Using QoS lutFwd to acheived)
	int IntfRmkEnabled;	//The mechanism enabed or not, 0:disabled  1:enabled
	int intfMappingToPbit[MAX_NETIF_SW_TABLE_SIZE]; //Each Interfcae default remark p-bit value;

#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES)
	//the assigned ingress_cvlan value when ACL asic rearrange.  -1:follow fwdVLAN_CPU
	int acl_rearrange_force_mc_ingress_cvid;
#endif

	//ACL have to compare wlanDev. Only pure software can do this pattern.(skip HWLOOKUP if valid)
	int wlanDevPatternValidInACL;

	//trap ACL IP range pattern with drop/permit action, let fwdEngine handle it (the trapped IP range could be larger)
	int aclDropIpRangeBySwEnable;
	int aclPermitIpRangeBySwEnable;

	int aclRearrangeProtectWithAllPermit;

	//trap all packet(exclude multicast) to software, and skip rearrange H/W ACL,CF when enabled
	int aclSkipRearrangeHWAclCf;

	// NAPT hash max ways.
	uint32	enableL4MaxWays;
	uint32	enableL4WaysList;
	uint8	l4InboundMaxWays[MAX_NAPT_IN_HASH_SIZE];
	uint8	l4OutboundMaxWays[MAX_NAPT_OUT_HASH_SIZE];

	// Congestion ctrl timer
	uint32 congestionCtrlIntervalMicroSecs; //0:disable
	uint32 congestionCtrlPortMask; //0:disable
	uint32 congestionCtrlSendTimesPerPort;
	uint32 congestionCtrlSendBytesPerSec[MAX_CONGESTION_CTRL_PORTS];
	uint32 congestionCtrlSendBytesInterval[MAX_CONGESTION_CTRL_PORTS];
	uint32 congestionCtrlInboundAckToHighQueue;
	uint32 congestionCtrlSendRemainderInNextGap;

	//ARP traffic table reference switch
	unsigned int arp_traffic_off:1;
	unsigned int arp_max_request_count;

	// anti arp spoofing
	rtk_rg_enable_t antiMacSpoofStatus;

	// anti ip spoofing
	rtk_rg_enable_t antiIpSpoofStatus;

	//WIFI TX redirect to port0
	uint32 wifiTxRedirect; //0:disable

	//pon port unmatch drop
	uint32 ponPortUnmatchCfDrop;

	//Protocol-Stack RX mirror to port0
	uint32 psRxMirrorToPort0;

	rtk_rg_igmpMldSnoopingPortControl_Conf_t snoopingPortConf[RTK_RG_PORT_MAX];

	//igmp report packet egress filter portmask
	uint32 igmpReportPortmask;

	//igmp leave packet egress filter portmask
	uint32 igmpLeavePortmask;

	//igmp report packet ingress filter portmask
	uint32 igmpReportIngressPortmask;

	//if igmp snooping disable and proxy enable only flooding to wifi
	uint32 igmpProxyOnly2Wifi;

	//igmp query packet filter portmask
	uint32 igmpMldQueryPortmask;
#if defined(CONFIG_MASTER_WLAN0_ENABLE)
	//igmp is referenced with wifi
	uint32 igmpWifiRefEnable;
#endif

	//proc support read to pipe
	uint32 proc_to_pipe;

	//only support ping request/reply
	rtk_rg_enable_t icmpRedirectToDMZ;

	//proc support store rx packets (filtered by trace filter)
	uint32 log_rx_pcap;
	struct file *log_rx_pcap_fp;

	//proc support store rx packets (filtered by trace filter)
	uint32 log_to_PS_pcap;
	struct file *log_to_PS_pcap_fp;

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	//proc support store connection estabilish or destory (filtered by trace filter)
	uint32 log_connections_enable:2;
	struct list_head *pLog_connections_list;
	struct list_head log_connections_list;
	uint32 log_connection_num;
#endif

	//proc support keep original format for specific ether type, default zero means not keep
	unsigned short keep_ps_eth;

	unsigned int dpi_accelerate_enable;
	unsigned int dpi_accelerate_shortcut:1;
	unsigned long dpi_accelerate_jiffies;

#ifdef __KERNEL__
	struct tasklet_struct congestionCtrlTasklets;
#endif
	uint8 bridge_netIfIdx_drop_by_portocal[MAX_NETIF_SW_TABLE_SIZE]; //0:Both pass, 1:Drop IPv6(IPv4 Pass) 2:Drop IPv4(IPv6 Pass)
	uint8 wanIntf_disable_linkLocal_rsvACL[MAX_NETIF_SW_TABLE_SIZE]; //0:do nothing 1:force diabled reserved ACL(RTK_RG_ACLANDCF_RESERVED_IPV6_INTFn_LINK_LOCAL_TRAP) for this interface

	//Over SRAM===================================================================
	rtk_rg_pktHdr_t	pktHeader_2;
	rtk_rg_pktHdr_t	pktHeader_frag;	// for queued fragment packet
	rtk_rg_pktHdr_t	pktHeader_broadcast;
	rtk_rg_pktHdr_t	pktHeader_dpi;
	rtk_rg_statistic_t statistic;
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	uint16 flowHashCount[MAX_FLOW_TABLE_SIZE>>MAX_FLOW_WAYS_SHIFT];
	uint16 flowHashHwCollisionCount[MAX_FLOW_TABLE_SIZE>>MAX_FLOW_WAYS_SHIFT];

	uint8 intfIdx_dipList;
#endif

#ifdef CONFIG_SMP
	uint32 smp_statistic[RG_SMP_JOBS_TYPE_MAX][4];
	uint8 smpNicTxCpuFromCpu[4];
#if defined(CONFIG_RG_FLOW_ENHANCED_WIFI_MODE)
	uint8 smpNicTxCpuFromGMAC0[4];
	uint8 smpNicTxCpuFromGMAC1[4];
	uint8 smpNicTxCpuFromGMAC2[4];
#endif
	uint8 smpWifi11acTxCpuFromCpu[4];
	uint8 smpWifi11nTxCpuFromCpu[4];
	uint8 smpSelectTxGmacFromCpu[4];
#ifdef RG_BY_TASKLET
#else
	int8 smpRgHighPriCpu;		// assign priority 1~7 packets to cpu0~3, init as -1(disable)
	uint8 smpRgCpuFromCpu[4];
#endif
	rtk_rg_pktHdr_t smpPktHeader[4];
#endif

	// NAT single binding(same ext port) for TCP/UDP session if same source IP and source port
	uint8 srcAddrBindingEn;

	uint32 superfluousPacketDrop:1;

	uint32 hybrid_pptp_portmask;	// refer to rtk_rg_port_idx_t

	//maintain ipset in fwdEngine
#if defined(CONFIG_RG_IPSET_VERSION) && (CONFIG_RG_IPSET_VERSION==1)
	struct list_head ipsetFreeListHead[MAX_IPSET_IDX_SIZE];							//free list
	struct list_head ipsetHashListHead[MAX_IPSET_IDX_SIZE][MAX_IPSET_HASH_SIZE];	//hash list
	struct list_head ipsetListHead[MAX_IPSET_IDX_SIZE];								//non hash list
	rtk_rg_ipset_list_t ipset_entry[MAX_IPSET_IDX_SIZE][MAX_IPSET_SW_ENTRY_SIZE];	//total entry = free list + hash list + non hash list
	uint32 ipsetHashRefCnt[MAX_IPSET_IDX_SIZE];										//hash valid entry count to speed up fwdEngine check
	uint32 ipsetReverseCheck[(MAX_IPSET_IDX_SIZE/32)+1];							//per ipset should forward check or reverse check
#endif
#if defined(CONFIG_RG_IPSET_VERSION) && (CONFIG_RG_IPSET_VERSION==2)
	rtk_rg_list_head_t ipsetsFreeGroupListHead;
	rtk_rg_list_head_t ipsetsGroupListHead[MAX_IPSETS_GROUP_SIZE];
	rtk_rg_list_head_t ipsetsFreeListHead[MAX_IPSETS_IDX_SIZE];
	rtk_rg_list_head_t ipsetsHashListHead[MAX_IPSETS_IDX_SIZE][MAX_IPSETS_HASH_SIZE];
	rtk_rg_ipsets_group_cfg_t ipsets_grpCfg[MAX_IPSETS_GROUP_SIZE];
	rtk_rg_ipsets_group_list_t ipsets_setCfg[MAX_IPSETS_IDX_SIZE];
	rtk_rg_ipsets_list_t ipsets_entry[MAX_IPSETS_IDX_SIZE][MAX_IPSETS_SW_ENTRY_SIZE];
	uint16 ipsetsRefCnt[RTK_RG_IPSETS_SET_PROTO_END][RTK_RG_IPSETS_SET_DIR_END][RTK_RG_IPSETS_SET_TYPE_END];
	uint8 ipsetsGroupPriority[MAX_IPSETS_GROUP_SIZE];
#endif

	//maintain ACL in fwdEngine
	rtk_rg_aclFilterEntry_t acl_SW_table_entry[MAX_ACL_SW_ENTRY_SIZE]; //record all ACL informations(including RG_ACL & HW_ACL), the array index is the RG_ACL index
	rtk_rg_aclFilterAndQos_t acl_filter_temp[MAX_ACL_SW_ENTRY_SIZE]; //for acl del entry using
	uint32 acl_filter_temp_valid[(MAX_ACL_SW_ENTRY_SIZE/32)+1]; //for acl del faster so record acl entry valid bit
	rtk_enable_t check_acl_priority_action_for_rg_acl_of_l34_type; //0:all acl priority action is in fwdtype= ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET , _rtk_rg_ingressACLAction() can speed up    1 : acl priority action exist in fwdtype=L34 related type, _rtk_rg_ingressACLAction() have to check all rules.

	//maintain CF in fwdEngine
	rtk_rg_classifyEntry_t classify_SW_table_entry[TOTAL_CF_ENTRY_SIZE];


	//maintain naptPriority in fwdEngine (using link list)
	rtk_rg_sw_naptFilterAndQos_t napt_SW_table_entry[MAX_NAPT_FILER_SW_ENTRY_SIZE];
	rtk_rg_sw_naptFilterAndQos_t *pValidUsNaptPriorityRuleStart;
	rtk_rg_sw_naptFilterAndQos_t *pValidDsNaptPriorityRuleStart;

	//maintain Gpon Broadcast to Lan in fwdEngine
	rtk_rg_sw_gpon_ds_bc_vlanfilterAndRemarking_t gpon_SW_ds_bc_filter_table_entry[MAX_GPON_DS_BC_FILTER_SW_ENTRY_SIZE];

	struct list_head urlFilterfreeEntry;
	struct list_head urlFilterByString;
	struct list_head urlFilterBySmacHash[URL_FILTER_MAC_HASH_SIZE];

	struct list_head urlHiPriIdxfreeEntry;
	rtk_rg_hiPriEntryIdx_t urlHiPriIdx_table[MAX_FLOW_URLPRI_IDX_TABLE_SIZE];
	uint32 urlHiPri_En:1;
	rtk_rg_swUrlHighPri_t urlHiPri_table_entry[MAX_URL_HiPri_ENTRY_SIZE];
	rtk_rg_urlFilterEntry_t urlFilter_table_entry[MAX_URL_FILTER_ENTRY_SIZE];
	char urlFilter_parsingBuf[MAX_URL_FILTER_BUF_LENGTH];

	rtk_rg_forcePortalURL_t forcePortal_url_list[MAX_FORCE_PORTAL_URL_NUM+1];

	//dhcp trap
	int hwAclIdx_for_trap_dhcp[MAX_NETIF_SW_TABLE_SIZE]; //each intf can reserved one acl to trap DHCP packet to PS.

	int dsliteControlSet[L34_DSLITE_CTRL_END];	//romeDriver dslite Control (software)

	rtk_rg_ipsec_passthru_t		ipsec_passthru;
	rtk_rg_portmask_t stpBlockingPortmask;
	rtk_rg_cached_port_isolation_t storedInfo;	//cached isolation value

	//gponDsBCModule (for gpon downstream braodcast by port remarking vlan)
	uint32 gponDsBCModuleEnable;

	rtk_rg_portmask_t virtualMAC_with_PON_switch_mask;
	int32 virtualMAC_with_PON_cputag_reg;		//keep the MAC_CPU_TAG_CTRL setting and restore when the function is disabled

	rtk_rg_internal_support_bit_t internalSupportMask;

	uint32 demo_dpiPreRouteCallback_retValue;
	uint32 demo_dpiFwdCallback_retValue;

	//debug tool for dump RG ACL parameter when calling add
	uint32 acl_rg_add_parameter_dump;

	//debug tool for dump RG ACL parameter when calling add
	uint32 acl_debug_udp_encap_packet_dump:1;

	//disabled dmac2cvid portmask (refer to the usage of rg_kernel.wanDmac2cvidDisabled)
	uint32 dmac2cvidDisabledPortmask;

	//record Cf port status
	uint32 pon_is_cfport;
	uint32 rgmii_is_cfport;

#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES)
	//using ACL or Routong entry to trap local-in/out packet instread of l3.
	uint32 trap_routing_wan_by_acl;
#endif
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	uint8 icmp_trap_by_acl_disable;	//0:trap icmp by reserve acl; 1: disable icmpv4; 2: disable icmpv6; 3: disable icmpv4 and icmpv6
#endif

//ACL tempory parameters, due to struct size is too big that may cause function stack overflow
#if defined(CONFIG_RG_RTL9600_SERIES)
	rtk_rg_aclFilterEntry_t aclSWEntry,aclSWEntry_for_find,empty_aclSWEntry;
	rtk_rg_intfInfo_t ingress_intf_info, egress_intf_info, empty_intf_info;
	int flag_add_aclRule[MAX_ACL_TEMPLATE_SIZE];
	rtk_acl_ingress_entry_t aclRule[MAX_ACL_TEMPLATE_SIZE];
	rtk_acl_field_t aclField[RTK_MAX_NUM_OF_ACL_RULE_FIELD*MAX_ACL_TEMPLATE_SIZE];
	rtk_acl_igr_act_t acl_action;
	rtk_acl_field_t empty_aclField;
	rtk_classify_field_t classifyField_1[CLASSIFY_FIELD_END];
	rtk_classify_field_t classifyField_2[CLASSIFY_FIELD_END];
	rtk_classify_cfg_t cfRule_1, cfRule_2; //cfRule_1 for upstream. cfRule_2 for downstream
	rtk_classify_field_t empty_classifyField;
	rtk_acl_rangeCheck_ip_t aclSIPv4RangeEntry,aclDIPv4RangeEntry,aclSIPv6RangeEntry,aclDIPv6RangeEntry;
	rtk_acl_rangeCheck_l4Port_t aclSportRangeEntry,aclDportRangeEntry;
	rtk_classify_rangeCheck_ip_t cfIpRangeEntry;
	rtk_classify_rangeCheck_l4Port_t cfPortRangeEntry;
	rtk_rg_acl_fwding_type_direction_t flow_direction;
	rtk_classify_field_t classifyField_acl;
	rtk_acl_field_t aclField_ar[RTK_MAX_NUM_OF_ACL_RULE_FIELD];
#elif defined(CONFIG_RG_RTL9602C_SERIES)
	rtk_rg_intfInfo_t egress_intf_info;
	uint8 aclRuleValid[GLOBAL_ACL_RULE_SIZE];
	rtk_acl_field_t aclFieldEmpty;
	rtk_acl_field_t aclField[GLOBAL_ACL_FIELD_SIZE];
	rtk_acl_ingress_entry_t aclRule[GLOBAL_ACL_RULE_SIZE];
	rtk_acl_ingress_entry_t aclRuleEmpty;
	rtk_classify_field_t  classifyFieldEmpty;
	rtk_classify_field_t classifyField[GLOBAL_CF_FIELD_SIZE];
	rtk_classify_cfg_t classifyEntry[GLOBAL_CF_RULE_SIZE];
	rtk_acl_rangeCheck_l4Port_t aclSportRangeEntry,aclDportRangeEntry;
	rtk_acl_rangeCheck_ip_t aclSIPRangeEntry,aclDIPRangeEntry;
	rtk_rg_intfInfo_t ingress_intf_info;
	rtk_acl_field_t aclField_ar[RTK_MAX_NUM_OF_ACL_RULE_FIELD];
	rtk_rg_aclFilterEntry_t aclSWEntry,aclSWEntry_for_find,empty_aclSWEntry,aclSWEntryEmpty;
	rtk_rg_aclFilterAndQos_t empty_aclFilter;
#elif defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	rtk_rg_aclFilterEntry_t aclSWEntry,aclSWEntry_for_find,empty_aclSWEntry,aclSWEntry_rearrange;
	uint8 aclRuleValid[GLOBAL_ACL_RULE_SIZE];
	rtk_acl_field_t aclFieldEmpty;
	rtk_acl_field_t aclField[GLOBAL_ACL_FIELD_SIZE];
	rtk_acl_ingress_entry_t aclRule[GLOBAL_ACL_RULE_SIZE];
	rtk_acl_ingress_entry_t aclRuleEmpty;
	rtk_acl_rangeCheck_l4Port_t aclSportRangeEntry,aclDportRangeEntry;
	rtk_acl_rangeCheck_ip_t aclSIPRangeEntry,aclDIPRangeEntry;
#elif defined(CONFIG_RG_G3_SERIES)
	rtk_rg_aclFilterEntry_t aclSWEntry,aclSWEntry_for_find,empty_aclSWEntry,aclSWEntry_rearrange;
	rtk_rg_g3_cls_entry_t ca_cls_rule_record[MAX_ACL_CA_CLS_RULE_SIZE];
	uint8 ca_cls_used_count;
	rtk_rg_g3_special_pkt_entry_t ca_special_pkt_record[MAX_ACL_SPECIAL_PACKET_RULE_SIZE];
	uint32 ca_cls_template[ACL_TEMPLATE_CA_END];
#endif

	uint32 event_rec_ctrl:1;

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	rtk_rg_enable_t prevent_control_packet_drop;//0:disable 1:enable; be used to enable NIC suspend mechanism
	rtk_rg_enable_t tunnelHwAcceleration_enable;//turn on to allow adding pptp/l2tp flow to hw
	rtk_rg_enable_t dsliteHwAcceleration_disable;//turn on to disable dslite flow to hw
#endif
	rtk_rg_enable_t debug_message_display_to_tty;//0: disable, use printk to output debug message; 1: enable, use tty write function to output debig message
	struct task_struct *tty_display_cur_sh_process;//use to store /bin/sh that currently used
	int tty_display_cur_sh_process_pid;////use to store the pid of /bin/sh that currently used

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	rtk_rg_enable_t fiber_mode_qos_enabled;//0:GPON/EPON mode  1:Fiber mode,  due to PBO fixed, that egressPortRate limit should use different RTK API.
#endif

#if defined(CONFIG_RG_RTL9607C_SERIES)
	rtk_rg_downstream_loopback_t downstream_loopback;
	rtk_rg_enable_t downstream_rate_limit_by_shaper;
	rtk_port_t cpuTrapHashPort[MAX_TRAP_HASH_RESULT];
#endif

#if defined(CONFIG_RG_G3_SERIES)
	uint32 rg_mirror_ingress_pkt_to_port; //-1: not need to mirror,  0~mac port max: mirror the received packet to such physical port.

	//multicast tempory parameters
	ca_l3_mcast_group_members_t l3mcGMember;
	ca_l2_mcast_group_members_t l2mcGMember;
	ca_l2_mcast_group_members_t l2mcGMember_2;
#endif
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	rtk_rg_meterMode_t funbasedMeter_mode; //0:not init; 1: not use funbasedMeter; 2: use funbasedMeter
	rtk_rg_funcbasedMeter_t		funbasedMeter[RTK_RG_METER_MAX][MAX_FUNCBASEDMETER_SIZE];
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	rtk_rg_shareMeterState_t	hw_shareMeterUsedState[MAX_HW_SHAREMETER_TABLE_SIZE];
#endif
#endif
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	rtk_rg_wifi_flow_crtl_func_t wifi_flow_crtl_func; // configuration for wifi flow control auto enabling mechanism
#endif
	rtk_rg_trapSpecificLengthAck_t trapSpecificLengthAck;
	rtk_rg_trapSpecificLengthSyn_t trapSpecificLengthSyn;
	rtk_rg_systemMeterTable_t systemMeter;

#if defined(CONFIG_RG_G3_SERIES)
	uint32 auto_detect_hw_lut_mode:1;	// 0: always maintain lut table in sw, 1: auto detect that lut table should be maintained in sw or hw(all SVL).
	uint32 doNotAddHwLut:1;
	uint32 flow_meter_mib_conf_dependence:1; //If enable, the mib configuration depends on meter configuration for every flow; Otherwise, the flow mib and flow meter configuration are independent.
#endif
#if	defined(CONFIG_RG_RTL9602C_SERIES)
	uint32 excludeNAT_refCnt:MAX_L3_SW_TABLE_SIZE_SHIFT;
#endif
	//dnf game mechanism
	uint16 dnf_game_mechanism_local_port;
	uint16 dnf_game_mechanism_remote_port;
#if defined(CONFIG_RG_RTL9607C_SERIES)
	uint32 disable_lanIntf_overMtu_trap:1;
#endif
#if	defined(CONFIG_RG_NAPT_DMZ_SUPPORT)
	uint32 enhanced_dmz:1;
#endif
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	uint32 phy2ExtPort_mappingMsk;
#endif

#ifdef CONFIG_SMP
	//default GMAC9/10/7 affinity
	uint32 gmac9_affinity_cpumask;
	uint32 gmac10_affinity_cpumask;
	uint32 gmac7_affinity_cpumask;
#endif

	//wifi tx qos mapping
	int wifi_tx_qos_mapping[RTK_MAX_NUM_OF_PRIORITY];

	//mac binding
	unsigned char forciblyMacBinding_dhcpV4Opt60_len;
	unsigned char forciblyMacBinding_dhcpV6Opt16_len;
	unsigned char forciblyMacBinding_dhcpV4Opt60_opcode;	//bit0:= bit1:> bit2:<
	unsigned char forciblyMacBinding_dhcpV6Opt16_opcode;	//bit0:= bit1:> bit2:<
	rtk_rg_forciblyMacBindingToWan_t forciblyBindingMacList[MAX_FORCIBLY_BINDING_MAC_SIZE];
	unsigned char forciblyMacBinding_key[MAX_FORCIBLY_BINDING_KEY_LEN];
} rtk_rg_glb_system_t;


typedef struct rtk_rg_tcpShortTimeout_s
{
	uint32 timeoutJiffies;
	int naptOutIdx;
} rtk_rg_tcpShotTimeout_t;

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
typedef struct rtk_rg_L2L3_fragment_queue_s
{
	struct sk_buff *queue_skb;
	rtk_rg_rxdesc_t queue_rx_info;

	struct list_head fragQueue_list;
} rtk_rg_L2L3_fragment_queue_t;

typedef struct rtk_rg_L2L3_fragment_s
{
	uint8 isV4OrV6;
	uint32 sip;
	uint32 dip;
	uint16 sport;
	uint16 dport;
	uint32 ipId;	//IP header's identification, IPv4: 16 bits, IPv6: 32 bits
	uint8 ipProto;

	rtk_rg_fwdEngineReturn_t fragAction;
	unsigned long beginIdleTime;
	uint16 receivedLength;	//already received packet length
	uint16 totalLength;		//calculated by last fragment
	int32 flowHitIdx;					//CAUTION! the content may change, check 5-tuples before use it.

	uint16 queueCount;
	struct list_head fragQueueHead;
	struct list_head frag_list;
	uint16 frag_idx;
} rtk_rg_L2L3_fragment_t;
#endif

typedef struct rtk_rg_fragment_queue_s
{
	struct{
		rtk_rg_port_idx_t ingressPort;
		uint32 ipv4Sip;
		uint16 ipv4Identification;
	}queue_pktInfo;
	struct sk_buff *queue_skb;
	rtk_rg_rxdesc_t queue_rx_info;

	unsigned long queue_time;		//jiffies
	int8 occupied;
	rtk_rg_naptDirection_t direction;
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	rtk_l34_nexthop_type_t	wanType;
#endif
} rtk_rg_fragment_queue_t;

typedef struct rtk_rg_ipv4_fragment_out_s
{
	union{
		struct{
			int NaptOutboundEntryIndex;
			unsigned short sport;	//int port
			unsigned short dport;	//remote port
		}napt;	//for TCP, UDP
	}pktInfo;
	ipaddr_t intIp;
	rtk_rg_pkthdr_tagif_t layer4Type;
	rtk_rg_fwdEngineReturn_t fragAction;
	unsigned long beginIdleTime;
	unsigned short receivedLength;	//already received packet length
	unsigned short totalLength;	//calculated by last fragment
	unsigned char queueCount;
	unsigned short identification;		//IP header's identification
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM) || defined(CONFIG_ROME_NAPT_SHORTCUT)
	int32 flowHitIdx;					//CAUTION! the content may change, check 5-tuples before use it.
#endif
	struct list_head fragout_list;
} rtk_rg_ipv4_fragment_out_t;

typedef struct rtk_rg_ipv4_fragment_in_s
{
	union{
		struct{
			int NaptOutboundEntryIndex;
			unsigned short sport;	//remote port
			unsigned short dport;	//ext port
		}napt;	//for TCP, UDP
		struct{
			ipaddr_t intIp;
		}other;
	}pktInfo;
	ipaddr_t remoteIp;
	rtk_rg_pkthdr_tagif_t layer4Type;
	rtk_rg_fwdEngineReturn_t fragAction;
	unsigned long beginIdleTime;
	unsigned short receivedLength;	//already received packet length
	unsigned short totalLength;	//calculated by last fragment
	unsigned char queueCount;
	unsigned short identification;		//IP header's identification
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM) || defined(CONFIG_ROME_NAPT_SHORTCUT)
	int32 flowHitIdx;					//CAUTION! the content may change, check 5-tuples before use it.
#endif
	struct list_head fragin_list;
} rtk_rg_ipv4_fragment_in_t;

typedef struct rtk_rg_saLearningLimitInfo_s
{
	int learningLimitNumber;
	rtk_rg_sa_learning_exceed_action_t action;
}rtk_rg_saLearningLimitInfo_t;

typedef struct rtk_rg_accessWanLimitData_s
{
	rtk_rg_accessWanLimitType_t type;
	rtk_rg_accessWanLimitField_t field;
	union
	{
		unsigned char category;
		rtk_rg_portmask_t port_mask;
	}data;
	int learningLimitNumber;
	int learningCount;
	rtk_rg_sa_learning_exceed_action_t action;
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	unsigned int wlan0_dev_mask;	//used for WLAN0 device access limit
#endif
}rtk_rg_accessWanLimitData_t;

typedef struct rtk_rg_accessWanLimitCategory_s
{
	unsigned char category;
	rtk_mac_t mac;
}rtk_rg_accessWanLimitCategory_t;

typedef struct rtk_rg_groupMacLimit_s
{
	rtk_rg_portmask_t port_mask;
	int learningLimitNumber;
	int learningCount;
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	unsigned int wlan0_dev_mask;	//used for WLAN0 device access limit
#endif
}rtk_rg_groupMacLimit_t;

typedef struct rtk_rg_callbackFunctionPtrGet_s
{
    rtk_rg_callback_function_idx_t	callback_function_idx;
    POINTER_CAST callback_function_pointer;
} rtk_rg_callbackFunctionPtrGet_t;

typedef enum rtk_rg_macFilterWhiteListDelFlag_e
{
	MACF_DEL_ONE = 0,
	MACF_DEL_ALL
} rtk_rg_macFilterWhiteListDelFlag_t;

typedef struct rtk_rg_macFilterWhiteList_s
{
	rtk_rg_macFilterWhiteListDelFlag_t	del_flag;
	rtk_mac_t mac;
} rtk_rg_macFilterWhiteList_t;

typedef struct rtk_rg_saLearningLimitProbe_s
{
#ifdef __KERNEL__

#ifdef TIMER_AGG
	rtk_rg_timer_t timer;
#else
	struct timer_list timer;
#endif
#endif
	short l2Idx;
	short arpIdx;
	int8 arpCounter;
	rtk_rg_arp_request_t arpReq;
	short neighborIdx;
	int8 neighborCounter;
	rtk_rg_neighbor_discovery_t neighborReq;
	ipaddr_t v4IP;
	rtk_ipv6_addr_t v6IP;
	int v6Route;
	atomic_t activity;
}rtk_rg_saLearningLimitProbe_t;




typedef struct rtk_rg_table_ipfilter_s
{
	ipaddr_t filterIp;
	int32 ipFilterRefCount;
	int valid;
} rtk_rg_table_ipfilter_t;



typedef struct rtk_rg_table_ipmcgrp_s
{
	ipaddr_t groupIp;
	rtk_portmask_t portMsk;
	int32 	ipmRefCount;
	int valid;
} rtk_rg_table_ipmcgrp_t;

typedef struct rtk_rg_table_wlan_mbssid_s
{
	rtk_mac_t			mac;
	int					wlan_dev_idx;
	int					vlan_tag_if; //support for WIFI DMAC2CVID
	int					vid; //support for WIFI DMAC2CVID
	long unsigned int	learn_jiffies;
} rtk_rg_table_wlan_mbssid_t;

typedef struct rtk_rg_nexthop_lan_host_s
{
	ipaddr_t ipAddr;
	rtk_mac_t macAddr;
	rtk_rg_mac_port_idx_t port;

	int rtIdx;
	int intfIdx;
	int macIdx;

	unsigned int valid:1;
}rtk_rg_nexthop_lan_host_t;


/* End of System ============================================================== */

typedef struct rtk_rg_globalDatabase_cache_s
{
	rtk_rg_pktHdr_t	pktHeader_1;
	rtk_rg_rxdesc_t rxInfoFromWLAN; //for WIFI to fwdEngineInput
#ifdef CONFIG_DUALBAND_CONCURRENT
	rtk_rg_rxdesc_t rxInfoFromSlaveWLAN; //for Slave WIFI to fwdEngineInput
#endif
}rtk_rg_globalDatabase_cache_t;
#if defined(CONFIG_RG_RTL9600_SERIES)
typedef struct rtk_rg_congestionCtrlRing_s
{
	struct tx_info ptxInfo;
	struct sk_buff *pSkb;
	rtk_rg_port_idx_t ingressPort;

} rtk_rg_congestionCtrlRing_t;
#endif
typedef struct rtk_rg_globalDatabase_s
{
	//20140819LUKE: only first 32KB reside in sram!
	rtk_rg_pktHdr_t *pktHdr;
	rtk_rg_glb_system_t systemGlobal;

	//rtk_rg_table_mac_t 			mac[MAX_LUT_SW_TABLE_SIZE];
	rtk_rg_table_vlan_t			vlan[MAX_VLAN_SW_TABLE_SIZE];
	rtk_rg_table_lut_t			lut[MAX_LUT_SW_TABLE_SIZE];
	rtk_rg_table_netif_t		netif[MAX_NETIF_SW_TABLE_SIZE];
	rtk_rg_table_l3_t			l3[MAX_L3_SW_TABLE_SIZE];
	rtk_rg_table_extip_t		extip[MAX_EXTIP_SW_TABLE_SIZE];
	rtk_rg_table_nexthop_t		nexthop[MAX_NEXTHOP_SW_TABLE_SIZE];
	rtk_rg_table_pppoe_t		pppoe[MAX_PPPOE_SW_TABLE_SIZE];
	rtk_rg_table_arp_t			arp[MAX_ARP_SW_TABLE_SIZE];
	rtk_rg_table_wantype_t  	wantype[MAX_WANTYPE_SW_TABLE_SIZE];
	rtk_rg_table_naptIn_t		naptIn[MAX_NAPT_IN_SW_TABLE_SIZE];
	rtk_rg_table_naptOut_t		naptOut[MAX_NAPT_OUT_SW_TABLE_SIZE];
	rtk_rg_table_bind_t 		bind[MAX_BIND_SW_TABLE_SIZE];
	rtk_rg_table_v6route_t 		v6route[MAX_IPV6_ROUTING_SW_TABLE_SIZE];
	rtk_rg_table_v6ExtIp_t		v6Extip[MAX_NETIF_SW_TABLE_SIZE];//1-to-1 mapping to interface table
	rtk_rg_table_v6neighbor_t	v6neighbor[MAX_IPV6_NEIGHBOR_SW_TABLE_SIZE];
#if !defined(CONFIG_RG_RTL9607C_SERIES) && !defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	rtk_rg_table_ipfilter_t		ipmcfilter[MAX_IPMCFILTER_HW_TABLE_SIZE];
#endif
	rtk_rg_table_ipmcgrp_t		ipmcgrp[MAX_IPMCGRP_SW_TABLE_SIZE];
	rtk_rg_table_wlan_mbssid_t	wlanMbssid[MAX_WLAN_MBSSID_SW_TABLE_SIZE];
	int wlanMbssidHeadIdx; //the first lookup entry of the wlanMbssid table.
#if defined(CONFIG_RG_RTL9602C_SERIES)
	rtk_rg_table_dslite_t		dslite[MAX_DSLITE_SW_TABLE_SIZE];
#endif
	rtk_rg_table_dsliteMc_t		dsliteMc[MAX_DSLITEMC_SW_TABLE_SIZE];

	rtk_rg_table_staticRoute_t	staticRoute[MAX_STATIC_ROUTE_SIZE];
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	rtk_rg_table_mcFlowIdxMapping_t mcflowIdxtbl[DEFAULT_MAX_FLOW_COUNT];
#if defined(CONFIG_RG_G3_SERIES)
	rtk_rg_mcEngine_info_t mcHwEngineInfoTbl[DEFAULT_MAX_FLOW_COUNT];
#endif

#endif
	rtk_rg_table_naptOut_linkList_t	*pNaptOutFreeListHead;
	rtk_rg_table_naptOut_linkList_t	naptOutFreeList[MAX_NAPT_OUT_SW_TABLE_SIZE-MAX_NAPT_OUT_HW_TABLE_SIZE];
	rtk_rg_table_naptOut_linkList_t	*pNaptOutHashListHead[MAX_NAPT_OUT_HASH_SIZE];

	rtk_rg_table_naptIn_linkList_t	*pNaptInFreeListHead;
	rtk_rg_table_naptIn_linkList_t	naptInFreeList[MAX_NAPT_IN_SW_TABLE_SIZE-MAX_NAPT_IN_HW_TABLE_SIZE];
	rtk_rg_table_naptIn_linkList_t	*pNaptInHashListHead[MAX_NAPT_IN_HASH_SIZE];

	rtk_rg_table_icmp_linkList_t *pICMPCtrlFlowHead;
	rtk_rg_table_icmp_linkList_t icmpCtrlFlowLinkList[MAX_ICMPCTRLFLOW_SIZE];

#if defined(CONFIG_RG_NAPT_NEW_EXTPORT_MECHANISM)
	rtk_rg_napt_extPort_record_t naptTcpExtPortRecord[65536];
	struct list_head naptTcpExtPortRecord_head[4096];	// index by HASH(internal IP, internal port)
	rtk_rg_napt_extPort_record_t naptUdpExtPortRecord[65536];
	struct list_head naptUdpExtPortRecord_head[4096];	// index by HASH(internal IP, internal port)
#else
	uint32	naptTcpExternPortUsed[65536/32]; // naptTcpExternPortUsed[0] bit0==>TCP PORT 0, bit31==>TCP PORT 31, naptTcpExternPortUsed[1] bit0==>TCP PORT 32...
	uint32	naptUdpExternPortUsed[65536/32]; // naptUdpExternPortUsed[0] bit0==>UDP PORT 0, bit31==>UDP PORT 31, naptTcpExternPortUsed[1] bit0==>UDP PORT 32...
	uint8	naptTcpExternPortUsedRefCount[65536];
	uint8	naptUdpExternPortUsedRefCount[65536];
#endif
#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	uint32	ipv6naptTcpExternPortUsed[65536/32]; // naptTcpExternPortUsed[0] bit0==>TCP PORT 0, bit31==>TCP PORT 31, naptTcpExternPortUsed[1] bit0==>TCP PORT 32...
	uint32	ipv6naptUdpExternPortUsed[65536/32]; // naptUdpExternPortUsed[0] bit0==>UDP PORT 0, bit31==>UDP PORT 31, naptTcpExternPortUsed[1] bit0==>UDP PORT 32...
#endif

	atomic_t naptProtcolStackEntryNumber[2]; //Array 0 for UDP, array 1 for TCP
	atomic_t naptForwardEngineEntryNumber[2]; //Array 0 for UDP, array 1 for TCP
	uint32 longestIdleSecs;
	int32 longestIdleNaptIdx;
	uint8 portLinkStatusInitDone;
	uint32 portLinkupMask,portLinkupSWPBOMask,portLinkupSWPBOCnt,disableSWPBOAutoConfIgnorePortmask;

#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	struct list_head 				ipv6Layer4FreeListHead;
	rtk_rg_ipv6_layer4_linkList_t	ipv6Layer4FreeList[MAX_IPV6_STATEFUL_TABLE_SIZE];
	struct list_head				ipv6Layer4HashListHead[MAX_IPV6_STATEFUL_HASH_HEAD_SIZE];

	//fragment handling
	rtk_rg_fragment_queue_t 		ipv6FragmentQueue[MAX_IPV6_FRAGMENT_QUEUE_SIZE];
#endif
#if defined(CONFIG_RG_WAN_MSS_CACHE)
	rtk_rg_netif_mssCache_t msscache[MAX_NETIF_SW_TABLE_SIZE];
#endif
	//Routing and ARP related
	int routingArpInfoNum;	//numer of routing entry in routingArpInfoArray
	int *p_tempRoutingVlanInfoArray;
	int *p_routingVlanInfoArray;
	int routingVlanInfoArray_1[MAX_L3_SW_TABLE_SIZE];
	int routingVlanInfoArray_2[MAX_L3_SW_TABLE_SIZE];
	rtk_rg_routing_arpInfo_t *p_routingArpInfoArray;
	rtk_rg_routing_arpInfo_t *p_tempRoutingArpInfoArray;
	rtk_rg_routing_arpInfo_t routingArpInfoArray_1[MAX_L3_SW_TABLE_SIZE];		//used in ARP rearrangement mechanism
	rtk_rg_routing_arpInfo_t routingArpInfoArray_2[MAX_L3_SW_TABLE_SIZE];		//used in ARP rearrangement mechanism
#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	uint8 arpTableCopied[MAX_ARP_HW_TABLE_SIZE];
	rtk_rg_table_arp_t tempArpTable[MAX_ARP_HW_TABLE_SIZE];
#endif
	rtk_l34_routing_entry_t tempL3Table[MAX_L3_SW_TABLE_SIZE];

	uint32	arpTrfIndicator[512/32];//arpTrfIndicator[0] bit0==>ARP[0], bit31==>ARP[31], arpTrfIndicator[1] bit0==>ARP[32]...

	uint32	naptValidSet[MAX_NAPT_OUT_SW_TABLE_SIZE/32];
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#if defined(CONFIG_RG_G3_SERIES)
	uint32  flowValidSet[MAX_FLOW_SW_VALID_BITS_ARRAY_SIZE];
	uint32	mainHashValidSet[MAX_FLOW_HW_MAIN_HASH_SIZE/32];
	rtk_rg_swFlowIdx_t swFlowIdx_mainHashMapping[MAX_FLOW_HW_MAIN_HASH_SIZE];
#else	// not CONFIG_RG_G3_SERIES
	// only for hw flow entry
	uint32  flowValidSet[MAX_FLOW_TABLE_SIZE/32];
#endif	// end defined(CONFIG_RG_G3_SERIES)
#endif
#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	uint32	arpValidSet[MAX_ARP_HW_TABLE_SIZE/32];
	uint32	neighborValidSet[MAX_IPV6_NEIGHBOR_HW_TABLE_SIZE/32];
#endif

	uint32	lutValidSet[MAX_LUT_HW_TABLE_SIZE/32];

	rtk_rg_upnpConnection_t upnp[MAX_UPNP_SW_TABLE_SIZE];
	rtk_rg_virtualServer_t virtualServer[MAX_VIRTUAL_SERVER_SW_TABLE_SIZE];
#ifdef CONFIG_RG_NAPT_DMZ_SUPPORT
	rtk_rg_dmzInfo_t dmzInfo[MAX_DMZ_TABLE_SIZE];
#endif
	rtk_rg_gatewayServicePortEntry_t gatewayServicePortEntry[MAX_GATEWAYSERVICEPORT_TABLE_SIZE];
	rtk_rg_fragment_queue_t ipv4FragmentQueue[MAX_IPV4_FRAGMENT_QUEUE_SIZE];
	struct list_head	fragOutFreeListHead;
	rtk_rg_ipv4_fragment_out_t	fragOutFreeList[MAX_FRAG_OUT_FREE_TABLE_SIZE];
	struct list_head	fragOutHashListHead[IPV4_FRAGMENT_OUT_HASH_MASK_SHIFT];

	struct list_head	fragInFreeListHead;
	rtk_rg_ipv4_fragment_in_t	fragInFreeList[MAX_FRAG_IN_FREE_TABLE_SIZE];
	struct list_head	fragInHashListHead[IPV4_FRAGMENT_IN_HASH_MASK_SHIFT];
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	rtk_rg_table_swHighPriPatten_t flowHighPriTbl[MAX_FLOW_HIGHPRI_TABLE_SIZE];
	rtk_rg_table_extport_t extPortTbl[MAX_EXTPORT_TABLE_SIZE];
	rtk_rg_table_indmac_t indMacTbl[MAX_INDMAC_TABLE_SIZE];
	rtk_rg_table_ethtype_t ethTypeTbl[MAX_ETHTYPE_TABLE_SIZE];
	rtk_rg_table_wanaccesslimit_t wanAccessLimit[MAX_WANACCESSLIMIT_TABLE_SIZE];
	rtk_rg_table_flowmib_t flowMIBTbl[MAX_FLOWMIB_TABLE_SIZE+PURE_SW_FLOWMIB_TABLE_SIZE]; //HW+pure SW table
	rtk_rg_table_flow_t flow[MAX_FLOW_SW_TABLE_SIZE];
#if MAX_FLOW_HW_TABLE_SIZE >= MAX_FLOW_SW_TABLE_SIZE
#error "[Configuration error] Hw flow table size is not less than Sw flow table size."
#error "[Configuration error] Please shrink Hw flow size by using CONFIG_RG_FLOW_4K_MODE/CONFIG_RG_FLOW_8K_MODE/CONFIG_RG_FLOW_16K_MODE/CONFIG_RG_FLOW_32K_MODE"
#error "[Configuration error] Or enlarge Sw flow table size by using CONFIG_RG_FLOW_SW_TABLE_SIZE."
#endif
#if defined(CONFIG_RG_G3_SERIES)
	rtk_rg_table_flowEntry_t flowPathHwEntry[MAX_FLOW_HW_TABLE_SIZE];
#endif
	rtk_rg_table_flowEntry_t flowPathSwEntry[MAX_FLOW_SW_TABLE_SIZE-MAX_FLOW_HW_TABLE_SIZE];
	//sw flow list
	rtk_rg_list_head_t flowListHead[MAX_FLOW_TABLE_SIZE>>MAX_FLOW_WAYS_SHIFT];			//indexed by flow hash index
	rtk_rg_list_head_t flowFreeListHead;
	rtk_rg_flow_linkList_t flowList[MAX_FLOW_SW_TABLE_SIZE-MAX_FLOW_HW_TABLE_SIZE];
	//flow list of napt recorded
	rtk_rg_naptRecorded_flow_linkList_t naptRecordedFlowList[MAX_FLOW_SW_TABLE_SIZE];

	rtk_rg_list_head_t flowTcpUdpTrackingGroupHead[MAX_FLOW_TCP_UDP_TRACKING_TABLE_SIZE];
	rtk_rg_list_head_t flowTcpUdpTrackingFreeGroupHead;
	rtk_rg_flow_tcpUdpTracking_group_linkList_t flowTcpUdpTrackingGroupList[MAX_FLOW_TCP_UDP_TRACKING_GROUP_SIZE];
	rtk_rg_flow_tcpUdpTracking_linkList_t flowTcpUdpTrackingList[MAX_FLOW_SW_TABLE_SIZE];

	struct list_head L2L3FragFreeListHead;
	struct list_head L2L3FragHashListHead[MAX_L2L3_FRAGMENT_HASH_BUCKET];
	rtk_rg_L2L3_fragment_t L2L3FragList[MAX_L2L3_FRAGMENT_TABLE_SIZE];
	struct list_head L2L3FragQueueFreeListHead;
	rtk_rg_L2L3_fragment_queue_t L2L3FragQueueList[MAX_L2L3_FRAGMENT_QUEUE_SIZE];

#if defined(CONFIG_RG_FLOW_4K_MODE)
	rtk_rg_table_flowEntry_t flowPathHwEntry[MAX_FLOW_HW_TABLE_SIZE];
	//flow tcam list
	struct list_head flowTcamListHead[MAX_FLOW_TABLE_SIZE>>MAX_FLOW_WAYS_SHIFT];		//indexed by flow hash index
	struct list_head flowTcamFreeListHead;
	rtk_rg_flowTcam_linkList_t flowTcamList[MAX_FLOW_TCAM_TABLE_SIZE];
#endif	//CONFIG_RG_FLOW_4K_MODE

#ifdef CONFIG_ROME_NAPT_SHORTCUT
	rtk_rg_napt_shortcut_t currentShortcut;
	rtk_rg_napt_shortcut_t inboundShortcut;
	rtk_rg_napt_shortcut_t localInNaptShortcut;
	rtk_rg_napt_shortcut_t hairpinNatShortcut;
	rtk_rg_napt_shortcut_t vxlanInNaptShortcut;
	rtk_rg_napt_shortcut_t vxlanOutNaptShortcut;
#endif
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
	rtk_rg_naptv6_shortcut_t currentV6Shortcut;
	rtk_rg_naptv6_shortcut_t nptv6HwlookupShortcut;
#endif

#else	//1 not CONFIG_RG_FLOW_BASED_PLATFORM

#ifdef CONFIG_ROME_NAPT_SHORTCUT
	uint32 v4ShortCutValidSet[MAX_NAPT_SHORTCUT_SIZE/32];
	uint8 v4ShortCut_lastAddIdx[MAX_NAPT_SHORTCUT_SIZE>>MAX_NAPT_SHORTCUT_WAYS_SHIFT]; //last added index of ipv4 shortcut in the same way
	rtk_rg_napt_shortcut_t naptShortCut[MAX_NAPT_SHORTCUT_SIZE];
#endif
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
	uint32 v6ShortCutValidSet[MAX_NAPT_V6_SHORTCUT_SIZE/32];
	uint8 v6ShortCut_lastAddIdx[MAX_NAPT_V6_SHORTCUT_SIZE>>MAX_NAPT_V6_SHORTCUT_WAYS_SHIFT]; //last added index of ipv6 shortcut in the same way
	rtk_rg_naptv6_shortcut_t naptv6ShortCut[MAX_NAPT_V6_SHORTCUT_SIZE];
#endif

#endif	//CONFIG_RG_FLOW_BASED_PLATFORM

	struct list_head softwareArpTableHead[MAX_ARP_SW_TABLE_HEAD];		//indexed by IP&0xff
	struct list_head softwareArpFreeListHead;
	rtk_rg_arp_linkList_t softwareArpFreeList[MAX_ARP_SW_TABLE_SIZE-MAX_ARP_HW_TABLE_SIZE];
#if defined(CONFIG_RG_RTL9602C_SERIES)
	struct list_head hardwareArpTableHead[MAX_ARP_HW_TABLE_HEAD];		//indexed by IP&0xff
	struct list_head hardwareArpFreeListHead;
	rtk_rg_arp_linkList_t hardwareArpFreeList[MAX_ARP_HW_TABLE_SIZE];
#endif
	struct list_head softwareLutTableHead[MAX_LUT_SW_TABLE_HEAD];		//indexed by HASH(IVL or SVL)
	struct list_head softwareLutFreeListHead;
	rtk_rg_lut_linkList_t softwareLutFreeList[MAX_LUT_SW_TABLE_SIZE-MAX_LUT_HW_TABLE_SIZE];

#if defined(CONFIG_RG_RTL9600_SERIES)
	struct list_head lutBCAMLinkListHead;
	struct list_head lutBCAMChosenLinkListHead; 	//the chosen victim will add to this list
#else	//support lut traffic bit
	struct list_head lutBCAMFreeListHead;
	struct list_head lutBCAMTableHead[(MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE)>>MAX_LUT_HASH_WAY_SHIFT];		//indexed by L2 hash
#endif
	rtk_rg_lut_linkList_t lutBCAMLinkList[MAX_LUT_BCAM_TABLE_SIZE];

	struct list_head lutGroupTableHead[MAX_LUT_HW_TABLE_SIZE];
	rtk_rg_lut_linkList_t lutGroupLinkList[MAX_LUT_HW_TABLE_SIZE];

	struct list_head vlanBindingListHead[RTK_RG_PORT_MAX];
	struct list_head vlanBindingFreeListHead;
	struct list_head vmacSkbListHead;		//store the virtualMAC skb for examination of PON's funcitionality
	struct list_head vmacSkbListFreeListHead;

	rtk_rg_redirectHttpAll_t redirectHttpAll;
	rtk_rg_redirectHttpRsp_t redirectHttpRsp;
	rtk_rg_redirectHttpCount_t redirectHttpCount;
	rtk_rg_redirectHttpURL_linkList_t redirectHttpURLFreeList[MAX_REDIRECT_URL_NUM];
	rtk_rg_redirectHttpWhiteList_linkList_t redirectHttpWhiteListFreeList[MAX_REDIRECT_WHITE_LIST_NUM];
	rtk_rg_avoidPortalURL_linkList_t avoidPortalURLFreeList[MAX_AVOID_PORTAL_URL_NUM];
	struct list_head redirectHttpURLListHead;
	struct list_head redirectHttpURLFreeListHead;
	struct list_head redirectHttpWhiteListListHead;
	struct list_head redirectHttpWhiteListFreeListHead;
	struct list_head avoidPortalURLListHead;
	struct list_head avoidPortalURLFreeListHead;

	rtk_rg_vbind_linkList_t	vlanBindingFreeList[MAX_BIND_SW_TABLE_SIZE];
#if defined(CONFIG_APOLLO_GPON_FPGATEST)
	rtk_rg_vmac_skb_linlList_t vmacSkbFreeList[MAX_VMAC_SKB_QUEUE_SIZE];
#endif


	uint8 layer2NextOfNewestCountIdx[(MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE)>>MAX_LUT_HASH_WAY_SHIFT];

#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support host policy
	//support host policing
	rtk_rg_hostPoliceLinkList_t hostPoliceList[HOST_POLICING_TABLE_SIZE];
	struct list_head hostPoliceCountListHead;
#endif

	//ALG module
	rtk_rg_alg_type_t algFunctionMask;	//1: enable, 0: disable
	rtk_rg_alg_type_t algServInLanIpMask;	//1: ip setuped, 0: no ip
	rtk_rg_alg_serverIpMapping_t algServInLanIpMapping[MAX_ALG_SERV_IN_LAN_NUM];
	uint32	algTcpExternPortEnabled[65536/32]; // algTcpExternPortEnabled[0] bit0==>TCP PORT 0 has alg, bit31==>TCP PORT 31 has alg, algTcpExternPortEnabled[1] bit0==>TCP PORT 32 has alg...
	uint32	algUdpExternPortEnabled[65536/32]; // algUdpExternPortEnabled[0] bit0==>UDP PORT 0 has alg, bit31==>UDP PORT 31 has alg, algUdpExternPortEnabled[1] bit0==>UDP PORT 32 has alg...
	uint32	algTcpExternPortEnabled_SrvInLan[65536/32]; // algTcpExternPortEnabled_SrvInLan[0] bit0==>TCP PORT 0 has alg, bit31==>TCP PORT 31 has alg, algTcpExternPortEnabled_SrvInLan[1] bit0==>TCP PORT 32 has alg...
	uint32	algUdpExternPortEnabled_SrvInLan[65536/32]; // algUdpExternPortEnabled_SrvInLan[0] bit0==>UDP PORT 0 has alg, bit31==>UDP PORT 31 has alg, algUdpExternPortEnabled_SrvInLan[1] bit0==>UDP PORT 32 has alg...
	rtk_rg_alg_funcMapping_t algTcpFunctionMapping[MAX_ALG_FUNCTIONS];
	rtk_rg_alg_funcMapping_t algUdpFunctionMapping[MAX_ALG_FUNCTIONS];
	uint32  algUserDefinedPort[MAX_ALG_FUNCTIONS];
	int		algUserDefinedTimeout[MAX_ALG_FUNCTIONS];

	//Dynamic Port ALG assign
	rtk_rg_alg_dynamicPort_t algDynamicFreeList[MAX_ALG_DYNAMIC_PORT_NUM];
	struct list_head algDynamicFreeListHead;
	struct list_head algDynamicCheckListHead;

	//Preserve ext port for ALG
	rtk_rg_alg_preserveExtPort_t algPreExtFreeList[MAX_ALG_PRESERVE_EXTPORT_NUM];
	struct list_head algPreExtFreeListHead;
	struct list_head algPreExtListHead;

	//tcpShortTimeoutHouseKeeping
	rtk_rg_tcpShotTimeout_t tcpShortTimeoutRing[MAX_TCP_SHORT_TIMEOUT_HOUSEKEEP_RING_SIZE];
	int tcpShortTimeoutFreedIdx;
	int tcpShortTimeoutRecycleIdx;
	unsigned int tcpShortTimeoutRecycleCnt;
	unsigned int tcpShortTimeoutForceRecycleCnt;

#if defined(CONFIG_RG_RTL9600_SERIES)
	//Congestion control timer
	uint32 congestionCtrlQueueIdx[2][MAX_CONGESTION_CTRL_PORTS]; //only support port 0~3
	uint32 congestionCtrlSendIdx[2][MAX_CONGESTION_CTRL_PORTS];
	uint32 congestionCtrlQueueCounter[2][MAX_CONGESTION_CTRL_PORTS];
	uint32 congestionCtrlMaxQueueCounter[2][MAX_CONGESTION_CTRL_PORTS];
	uint32 congestionCtrlFullDrop[2][MAX_CONGESTION_CTRL_PORTS];
	uint32 congestionCtrlSendedRemainder[MAX_CONGESTION_CTRL_PORTS];
	rtk_rg_congestionCtrlRing_t congestionCtrlRing[2][MAX_CONGESTION_CTRL_PORTS][MAX_CONGESTION_CTRL_RING_SIZE]; //[0][][]=low queue, [1][][]=high queue.
#endif

	//Nexthop LAN data
	rtk_rg_nexthop_lan_host_t nexthop_lan_table[MAX_NETIF_SW_TABLE_SIZE];

	rtk_rg_alg_l2tp_linkList_t *pAlgL2TPCtrlFlowHead;
	rtk_rg_alg_l2tp_linkList_t algL2TPCtrlFlowLinkList[MAX_L2TP_CTRL_FLOW_SIZE];
	uint32	algL2TPExternTulIDUsed[65536/32]; // l2tpExternTulIDUsed[0] bit0==>TunnelID 0, bit31==>TunnelID 31, l2tpExternTulIDUsed[1] bit0==>TunnelID 32...

#ifdef CONFIG_RG_ROMEDRIVER_ALG_BATTLENET_SUPPORT
	ipaddr_t algBnetSCClient[RTK_RG_MAX_SC_CLIENT];
#endif
	//ACL decision backup
	rtk_rg_aclHitAndAction_t aclDecisionBackup;
	rtk_rg_igmp_whiteList_t igmpWhiteList[MAX_IGMP_WHITE_LIST_SIZE];
	rtk_rg_igmp_whiteList_t igmpBlackList[MAX_IGMP_BLACK_LIST_SIZE];

#if defined(CONFIG_RG_RTL9600_SERIES)
	//HWNAT backup
	rtk_ipv6Routing_entry_t 	hw_v6route[MAX_IPV6_ROUTING_HW_TABLE_SIZE];
	rtk_l34_netif_entry_t		hw_netif[MAX_NETIF_HW_TABLE_SIZE];
	rtk_l34_ext_intip_entry_t	hw_extip[MAX_EXTIP_HW_TABLE_SIZE];
	rtk_l34_routing_entry_t		hw_l3[MAX_L3_HW_TABLE_SIZE];
	rtk_l34_pppoe_entry_t		hw_pppoe[MAX_PPPOE_HW_TABLE_SIZE];
	rtk_wanType_entry_t 		hw_wantype[MAX_WANTYPE_HW_TABLE_SIZE];
	rtk_l34_nexthop_entry_t		hw_nexthop[MAX_NEXTHOP_HW_TABLE_SIZE];
	rtk_binding_entry_t 		hw_bind[MAX_BIND_HW_TABLE_SIZE];
	uint32 						hw_REG_NAT_CTRL;
	uint32						hw_REG_V6_BD_CTRL;
	uint32						hw_REG_BD_CFG;
#endif
} rtk_rg_globalDatabase_t;

//Global variable that used in Linux kernel or only modified when system begin
typedef struct rtk_rg_globalKernel_s
{
	rtk_rg_initState_t init_state;
	rtk_rg_debug_level_t debug_level;
	rtk_rg_debug_level_t filter_level;	//used to filter display message based on trace_filter
	rtk_rg_debug_trace_filter_bitmask_t trace_filter_bitmask[TRACFILTER_MAX];
	rtk_rgDebugTraceFilter_t trace_filter[TRACFILTER_MAX];
	uint32 traceFilterRuleMask; // enable trace filter rule mask
	int tracefilterShow;		//show this packet
	int l2_hw_aging;
	int cp3_execute_count;
	int cp3_execute_count_state;

	//int ppp_diaged[MAX_NETIF_HW_TABLE_SIZE];//record which intf is PPPoE & diag on,  use when init disconnect pppoe judgement

	int timer_selector;
#if defined(CONFIG_RG_FLOW_AUTO_AGEOUT)
	int flow_timer_tick;
#endif

	rtk_rg_rxdesc_t rxInfoFromPS; //for Protocol Stack to fwdEngineInput
	rtk_rg_rxdesc_t rxInfoFromARPND; //for ARP or ND or Redirection to fwdEngineInput
	rtk_rg_rxdesc_t rxInfoFromIGMPMLD; //for IGMP or MLD to fwdEngineInput
	rtk_rg_rxdesc_t rxInfoFromMcDataBuf; //for McDataBuf to fwdEngineInput

#ifdef __KERNEL__
	rtk_rg_txdesc_t txDesc;

	uint32 protocolStackTxPortMask;	//keep the NIC dev_port_mapping from rtk_rg_fwdEngine_xmit to rtk_rg_fwdEngineInput
	struct proc_dir_entry *proc_rg;
	struct proc_dir_entry *proc_rg_group;
	struct proc_dir_entry *proc_rg_group_list[RTK_RG_PROC_GROUP_MAX];
	unsigned char proc_parsing_buf[512];

#ifdef TIMER_AGG
	unsigned int rgSystemTimerDisable:1;
	struct timer_list rgSystemTimer; // 1/16 sec timer
	rtk_rg_timer_table_t rgSystemTimerTableHead;
	rtk_rg_timer_table_t rgSystemTimerTableFreeHead;
	rtk_rg_timer_table_t rgSystemTimerTableFreeList[MAX_RG_TIMER_SIZE];

	rtk_rg_timer_t fwdEngineTcpShortTimeoutHouseKeepingTimer;
	rtk_rg_timer_t fwdEngineHouseKeepingTimer;
	rtk_rg_timer_t arpRequestTimer[MAX_NETIF_SW_TABLE_SIZE<<1]; 	//over MAX_NETIF_SIZE belong to PPTP&L2TP
	rtk_rg_timer_t neighborDiscoveryTimer[MAX_NETIF_SW_TABLE_SIZE<<1]; 	//over MAX_NETIF_SIZE belong to DSLITE
	rtk_rg_timer_t swRateLimitTimer;
	rtk_rg_timer_t wifi_flow_ctrl_detect_timer;
#else
	struct timer_list fwdEngineTcpShortTimeoutHouseKeepingTimer;
	struct timer_list fwdEngineHouseKeepingTimer;
	struct timer_list arpRequestTimer[MAX_NETIF_SW_TABLE_SIZE<<1]; 	//over MAX_NETIF_SIZE belong to PPTP&L2TP
	struct timer_list neighborDiscoveryTimer[MAX_NETIF_SW_TABLE_SIZE<<1];		//over MAX_NETIF_SIZE belong to DSLITE
	struct timer_list swRateLimitTimer;
	struct timer_list wifi_flow_ctrl_detect_timer;
#endif

	int arpRequestTimerCounter[MAX_NETIF_SW_TABLE_SIZE<<1]; //over MAX_NETIF_SIZE belong to PPTP&L2TP
	int neighborDiscoveryTimerCounter[MAX_NETIF_SW_TABLE_SIZE<<1];	//over MAX_NETIF_SIZE belong to DSLITE
	int arpForAutoTestTimerCounter;
#ifdef TIMER_AGG
	rtk_rg_timer_t arpForAutoTestTimer;
	rtk_rg_timer_t staticRouteArpOrNBReqTimer[MAX_STATIC_ROUTE_SIZE];
#else
	struct timer_list arpForAutoTestTimer;
	struct timer_list staticRouteArpOrNBReqTimer[MAX_STATIC_ROUTE_SIZE];
#endif
	int staticRouteArpOrNBTimerCounter[MAX_STATIC_ROUTE_SIZE];

	//Mac Learning limit
	rtk_rg_saLearningLimitProbe_t lutReachLimit_portmask;
	rtk_rg_saLearningLimitProbe_t lutReachLimit_category[WanAccessCategoryNum];

	//struct semaphore wanStaticCalled;
	//struct semaphore wanDsliteCalled;
	//struct semaphore wanDHCPCalled;
	//struct semaphore wanPPPOEAfterCalled;
	//struct semaphore wanPPTPAfterCalled;
	//struct semaphore wanL2TPAfterCalled;
	//struct semaphore wanPPPOEDSLITEAfterCalled;
	//struct semaphore interfaceLock;			//used when interface add or del
	//spinlock_t ipv4FragQueueLock;		//used when ipv4 fragment packet queue or dequeue
	spinlock_t igmpsnoopingLock;
	spinlock_t initLock; 					//used when manipulate rg_kernel to mark as init transaction
	//struct semaphore saLearningLimitLock;		//used when check or add source address learning count
	//struct semaphore linkChangeHandlerLock;		//used when check and clear link-down indicator register
	spinlock_t naptTableLock;			//used when read/write napt table by rg API.

#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	spinlock_t ipv6StatefulLock;		//used when access IPv6 stateful link-list data structure
	spinlock_t ipv6FragQueueLock;		//used when ipv6 fragment packet queue or dequeue
#endif
	spinlock_t algDynamicLock;
	spinlock_t urlFilterByMACLock;		//used when manipulate list structure of urlFireByMac
	spinlock_t rg_inbound_queue_lock;	//lock for manipulate rg internal data when receive packet or trigger timer, proc, etc. up or smp
	unsigned long rg_lock_flags;

	struct workqueue_struct *rg_callbackWQ;
	rtk_rg_wq_union_t		 rg_cbUnionArray[CONFIG_RG_CALLBACK_WQ_TOTAL_SIZE];
	int						 rg_cbUnionCurrentIdx;

#ifdef CONFIG_SMP
#if 1
	struct list_head smp_rx_sched_tasks_head;
	struct list_head smp_rx_non_sched_tasks_head;
	struct list_head smp_rx_sched_hi_tasks_head;
	struct list_head smp_rx_non_sched_hi_tasks_head;

	struct list_head smp_tx_gmac0_sched_packets_head;
	struct list_head smp_tx_gmac0_non_sched_packets_head;
	struct list_head smp_tx_gmac1_sched_packets_head;
	struct list_head smp_tx_gmac1_non_sched_packets_head;
#if defined(CONFIG_GMAC2_USABLE)
	struct list_head smp_tx_gmac2_sched_packets_head;
	struct list_head smp_tx_gmac2_non_sched_packets_head;
#endif
	struct list_head smp_tx_wlan0_sched_packets_head;
	struct list_head smp_tx_wlan0_non_sched_packets_head;
	struct list_head smp_tx_wlan1_sched_packets_head;
	struct list_head smp_tx_wlan1_non_sched_packets_head;

	rtk_rg_smp_rx_private_t rg_rx_queue_data[MAX_RG_RX_QUEUE_SIZE];
	rtk_rg_smp_rx_private_t rg_rx_hi_queue_data[MAX_RG_RX_HI_QUEUE_SIZE];

	rtk_rg_smp_tx_private_t rg_gmac0_tx_queue_data[MAX_RG_GMAC0_TX_QUEUE_SIZE];
	rtk_rg_smp_tx_private_t rg_gmac1_tx_queue_data[MAX_RG_GMAC1_TX_QUEUE_SIZE];
#if defined(CONFIG_GMAC2_USABLE)
	rtk_rg_smp_tx_private_t rg_gmac2_tx_queue_data[MAX_RG_GMAC2_TX_QUEUE_SIZE];
#endif
	rtk_rg_smp_tx_private_t rg_wlan0_tx_queue_data[MAX_RG_WLAN0_TX_QUEUE_SIZE];
	rtk_rg_smp_tx_private_t rg_wlan1_tx_queue_data[MAX_RG_WLAN1_TX_QUEUE_SIZE];

	//lock
	atomic_t rg_inbound_queue_entrance;

	rtk_rg_smp_ctrl_t rx_ctrl;
	rtk_rg_smp_ctrl_t rx_hi_ctrl;
	rtk_rg_smp_ctrl_t gmac0_tx_ctrl;
	rtk_rg_smp_ctrl_t gmac1_tx_ctrl;
#if defined(CONFIG_GMAC2_USABLE)
	rtk_rg_smp_ctrl_t gmac2_tx_ctrl;
#endif
	rtk_rg_smp_ctrl_t wlan0_tx_ctrl;
	rtk_rg_smp_ctrl_t wlan1_tx_ctrl;
//workqueue
	struct rg_private rg_inbound_queue_data;
	struct rg_timer_private rg_timer_queue_data;

#ifdef RG_BY_TASKLET
	struct tasklet_struct rg_tasklets;
#else
	struct workqueue_struct *rg_wq;

#endif

	//spinlock_t rg_inbound_queue_lock;
	//atomic_t rg_inbound_queue_entrance;
	spinlock_t rg_wq_rx_queue_lock;
	atomic_t rg_wq_rx_queue_entrance;

#ifdef OUTBOUND_BY_TASKLET
	struct tasklet_struct rg_gmac9_outbound_tasklets;
	struct tasklet_struct rg_gmac10_outbound_tasklets;

#else
	struct workqueue_struct *rg_gmac9_outbound_wq;
	struct workqueue_struct *rg_gmac10_outbound_wq;
	struct workqueue_struct *rg_wifi_11ac_outbound_wq;
	struct workqueue_struct *rg_wifi_11n_outbound_wq;

	struct rg_gmac9_outbound_queue_private rg_gmac9_outbound_queue_data;
	spinlock_t rg_gmac9_outbound_queue_lock;
	atomic_t rg_gmac9_outbound_queue_entrance;

	struct rg_gmac10_outbound_queue_private rg_gmac10_outbound_queue_data;
	spinlock_t rg_gmac10_outbound_queue_lock;
	atomic_t rg_gmac10_outbound_queue_entrance;

	struct rg_wifi_11ac_outbound_queue_private rg_wifi_11ac_outbound_queue_data;
	spinlock_t rg_wifi_11ac_outbound_queue_lock;
	atomic_t rg_wifi_11ac_outbound_queue_entrance;


	struct rg_wifi_11n_outbound_queue_private rg_wifi_11n_outbound_queue_data;
	spinlock_t rg_wifi_11n_outbound_queue_lock;
	atomic_t rg_wifi_11n_outbound_queue_entrance;

#if !defined(CONFIG_RG_G3_SERIES)
	unsigned long rg_inbound_queue_lock_flags[4];
	unsigned long rg_gmac9_outbound_queue_lock_flags[4];
	unsigned long rg_gmac10_outbound_queue_lock_flags[4];
	unsigned long rg_wifi_11ac_outbound_queue_lock_flags[4];
	unsigned long rg_wifi_11n_outbound_queue_lock_flags[4];
#endif

#endif
#endif

#endif //end of SMP

#else // not __KERNEL__
	struct tx_info txDesc;
	//void *wanStaticCalled;
	//void *wanDsliteCalled;
	//void *wanDHCPCalled;
	//void *wanPPPOEAfterCalled;
	//void *wanPPTPAfterCalled;
	//void *wanL2TPAfterCalled;
	//void *wanPPPOEDSLITEAfterCalled;
	//void *interfaceLock;
	//void *ipv4FragQueueLock;
	void *igmpsnoopingLock;
	void *initLock;
	//void *saLearningLimitLock;
	//void *linkChangeHandlerLock;
	void *naptTableLock;


#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	void *ipv6StatefulLock;
	void *ipv6FragQueueLock;
#endif
	void *algDynamicLock;
#endif
	unsigned int apolloChipId;
	unsigned int apolloRev;
	unsigned int apolloSubtype;

	unsigned short arp_number_for_LAN;
	unsigned short arp_number_for_WAN;

	rtk_rg_enable_t stag_enable;
	rtk_rg_enable_t layer2LookupMissFlood2CPU;

#if defined(CONFIG_RG_RTL9602C_SERIES)
	int force_cf_pattern0_size_enable;
	int cf_pattern0_size;
	uint8 block_communication_between_internet_and_other;
#endif

	//forcely disable dmac2cvid of pon port
	uint32 wanDmac2cvidDisabled:1;
	//forcely disable dmac2cvid of lan port
	uint32 lanDmac2cvidDisabled:1;

	//port range used by protocol stack
	uint16 lowerBoundPortUsedByPS;
	uint16 upperBoundPortUsedByPS;

#if defined(CONFIG_MASTER_WLAN0_ENABLE) && defined(CONFIG_RG_FLOW_NEW_WIFI_MODE) && !defined(CONFIG_RG_FLOW_ENHANCED_WIFI_MODE)
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	unsigned int disableSlaveWifiRxAcc_and_enableForwardHash:1;
#endif
#endif
	unsigned int autoTestMode:1;
}rtk_rg_globalKernel_t;

/* End of Tables ========================================================== */


struct platform
{
	int (*rtk_rg_api_module_init)(void);
	rtk_rg_err_code_t (*rtk_rg_driverVersion_get)(rtk_rg_VersionString_t *version_string);
	rtk_rg_err_code_t (*rtk_rg_initParam_get)(rtk_rg_initParams_t *init_param);
	rtk_rg_err_code_t (*rtk_rg_initParam_set)(rtk_rg_initParams_t *init_param);
	rtk_rg_err_code_t (*rtk_rg_lanInterface_add)(rtk_rg_lanIntfConf_t *lan_info,int *intf_idx);
//5
	rtk_rg_err_code_t (*rtk_rg_wanInterface_add)(rtk_rg_wanIntfConf_t *wanintf, int *wan_intf_idx);
	rtk_rg_err_code_t (*rtk_rg_staticInfo_set)(int wan_intf_idx, rtk_rg_ipStaticInfo_t *static_info);
	rtk_rg_err_code_t (*rtk_rg_dhcpRequest_set)(int wan_intf_idx);
	rtk_rg_err_code_t (*rtk_rg_dhcpClientInfo_set)(int wan_intf_idx, rtk_rg_ipDhcpClientInfo_t *dhcpClient_info);
	rtk_rg_err_code_t (*rtk_rg_pppoeClientInfoBeforeDial_set)(int wan_intf_idx, rtk_rg_pppoeClientInfoBeforeDial_t *app_info);
//10
	rtk_rg_err_code_t (*rtk_rg_pppoeClientInfoAfterDial_set)(int wan_intf_idx, rtk_rg_pppoeClientInfoAfterDial_t *clientPppoe_info);
	rtk_rg_err_code_t (*rtk_rg_interface_del)(int lan_or_wan_intf_idx);
	rtk_rg_err_code_t (*rtk_rg_intfInfo_find)(rtk_rg_intfInfo_t *intf_info, int *valid_lan_or_wan_intf_idx);
	rtk_rg_err_code_t (*rtk_rg_cvlan_add)(rtk_rg_cvlan_info_t *cvlan_info);
	rtk_rg_err_code_t (*rtk_rg_cvlan_del)(int cvlan_id);
//15
	rtk_rg_err_code_t (*rtk_rg_cvlan_get)(rtk_rg_cvlan_info_t *cvlan_info);
	rtk_rg_err_code_t (*rtk_rg_vlanBinding_add)(rtk_rg_vlanBinding_t *vlan_binding_info, int *vlan_binding_idx);
	rtk_rg_err_code_t (*rtk_rg_vlanBinding_del)(int vlan_binding_idx);
	rtk_rg_err_code_t (*rtk_rg_vlanBinding_find)(rtk_rg_vlanBinding_t *vlan_binding_info, int *valid_idx);
	rtk_rg_err_code_t (*rtk_rg_algServerInLanAppsIpAddr_add)(rtk_rg_alg_serverIpMapping_t *srvIpMapping);
//20
	rtk_rg_err_code_t (*rtk_rg_algServerInLanAppsIpAddr_del)(rtk_rg_alg_type_t delServerMapping);
	rtk_rg_err_code_t (*rtk_rg_algApps_set)(rtk_rg_alg_type_t alg_app);
	rtk_rg_err_code_t (*rtk_rg_algApps_get)(rtk_rg_alg_type_t *alg_app);
	rtk_rg_err_code_t (*rtk_rg_dmzHost_set)(int wan_intf_idx, rtk_rg_dmzInfo_t *dmz_info);
	rtk_rg_err_code_t (*rtk_rg_dmzHost_get)(int wan_intf_idx, rtk_rg_dmzInfo_t *dmz_info);
//25
	rtk_rg_err_code_t (*rtk_rg_virtualServer_add)(rtk_rg_virtualServer_t *virtual_server, int *virtual_server_idx);
	rtk_rg_err_code_t (*rtk_rg_virtualServer_del)(int virtual_server_idx);
	rtk_rg_err_code_t (*rtk_rg_virtualServer_find)(rtk_rg_virtualServer_t *virtual_server, int *valid_idx);
	rtk_rg_err_code_t (*rtk_rg_aclFilterAndQos_add)(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx);
	rtk_rg_err_code_t (*rtk_rg_aclFilterAndQos_del)(int acl_filter_idx);
//30
	rtk_rg_err_code_t (*rtk_rg_aclFilterAndQos_find)(rtk_rg_aclFilterAndQos_t *acl_filter, int *valid_idx);
	rtk_rg_err_code_t (*rtk_rg_macFilter_add)(rtk_rg_macFilterEntry_t *macFilterEntry,int *mac_filter_idx);
	rtk_rg_err_code_t (*rtk_rg_macFilter_del)(int mac_filter_idx);
	rtk_rg_err_code_t (*rtk_rg_macFilter_find)(rtk_rg_macFilterEntry_t *macFilterEntry, int *valid_idx);
	rtk_rg_err_code_t (*rtk_rg_mac_filter_whitelist_add)(rtk_rg_macFilterWhiteList_t *mac_filter_whitelist_info);
//35
	rtk_rg_err_code_t (*rtk_rg_mac_filter_whitelist_del)(rtk_rg_macFilterWhiteList_t *mac_filter_whitelist_info);
	rtk_rg_err_code_t (*rtk_rg_urlFilterString_add)(rtk_rg_urlFilterString_t *filter,int *url_idx);
	rtk_rg_err_code_t (*rtk_rg_urlFilterString_del)(int url_idx);
	rtk_rg_err_code_t (*rtk_rg_urlFilterString_find)(rtk_rg_urlFilterString_t *filter, int *valid_idx);
	rtk_rg_err_code_t (*rtk_rg_upnpConnection_add)(rtk_rg_upnpConnection_t *upnp, int *upnp_idx);
//40
	rtk_rg_err_code_t (*rtk_rg_upnpConnection_del)(int upnp_idx);
	rtk_rg_err_code_t (*rtk_rg_upnpConnection_find)(rtk_rg_upnpConnection_t *upnp, int *valid_idx);
	rtk_rg_err_code_t (*rtk_rg_naptConnection_add)(rtk_rg_naptEntry_t *naptFlow, int *flow_idx);
	rtk_rg_err_code_t (*rtk_rg_naptConnection_del)(int flow_idx);
	rtk_rg_err_code_t (*rtk_rg_naptConnection_find)(rtk_rg_naptInfo_t *naptInfo,int *valid_idx);
//45
	rtk_rg_err_code_t (*rtk_rg_multicastFlow_add)(rtk_rg_multicastFlow_t *mcFlow, int *flow_idx);
	rtk_rg_err_code_t (*rtk_rg_multicastFlow_del)(int flow_idx);
	/* martin ZHU add */
	rtk_rg_err_code_t (*rtk_rg_l2MultiCastFlow_add)(rtk_rg_l2MulticastFlow_t *l2McFlow,int *flow_idx);
	rtk_rg_err_code_t (*rtk_rg_multicastFlow_find)(rtk_rg_multicastFlow_t *mcFlow, int *valid_idx);
	rtk_rg_err_code_t (*rtk_rg_macEntry_add)(rtk_rg_macEntry_t *macEntry, int *entry_idx);
//50
	rtk_rg_err_code_t (*rtk_rg_macEntry_del)(int entry_idx);
	rtk_rg_err_code_t (*rtk_rg_macEntry_find)(rtk_rg_macEntry_t *macEntry,int *valid_idx);
	rtk_rg_err_code_t (*rtk_rg_arpEntry_add)(rtk_rg_arpEntry_t *arpEntry, int *arp_entry_idx);
	rtk_rg_err_code_t (*rtk_rg_arpEntry_del)(int arp_entry_idx);
	rtk_rg_err_code_t (*rtk_rg_arpEntry_find)(rtk_rg_arpInfo_t *arpInfo,int *arp_valid_idx);
//55
	rtk_rg_err_code_t (*rtk_rg_neighborEntry_add)(rtk_rg_neighborEntry_t *neighborEntry,int *neighbor_idx);
	rtk_rg_err_code_t (*rtk_rg_neighborEntry_del)(int neighbor_idx);
	rtk_rg_err_code_t (*rtk_rg_neighborEntry_find)(rtk_rg_neighborInfo_t *neighborInfo,int *neighbor_valid_idx);
	rtk_rg_err_code_t (*rtk_rg_accessWanLimit_set)(rtk_rg_accessWanLimitData_t access_wan_info);
	rtk_rg_err_code_t (*rtk_rg_accessWanLimit_get)(rtk_rg_accessWanLimitData_t *access_wan_info);
//60
	rtk_rg_err_code_t (*rtk_rg_accessWanLimitCategory_set)(rtk_rg_accessWanLimitCategory_t macCategory_info);
	rtk_rg_err_code_t (*rtk_rg_accessWanLimitCategory_get)(rtk_rg_accessWanLimitCategory_t *macCategory_info);
	rtk_rg_err_code_t (*rtk_rg_softwareSourceAddrLearningLimit_set)(rtk_rg_saLearningLimitInfo_t sa_learnLimit_info, rtk_rg_port_idx_t port_idx);
	rtk_rg_err_code_t (*rtk_rg_softwareSourceAddrLearningLimit_get)(rtk_rg_saLearningLimitInfo_t *sa_learnLimit_info, rtk_rg_port_idx_t port_idx);
	rtk_rg_err_code_t (*rtk_rg_dosPortMaskEnable_set)(rtk_rg_mac_portmask_t dos_port_mask);
//65
	rtk_rg_err_code_t (*rtk_rg_dosPortMaskEnable_get)(rtk_rg_mac_portmask_t *dos_port_mask);
	rtk_rg_err_code_t (*rtk_rg_dosType_set)(rtk_rg_dos_type_t dos_type,int dos_enabled,rtk_rg_dos_action_t dos_action);
	rtk_rg_err_code_t (*rtk_rg_dosType_get)(rtk_rg_dos_type_t dos_type,int *dos_enabled,rtk_rg_dos_action_t *dos_action);
	rtk_rg_err_code_t (*rtk_rg_dosFloodType_set)(rtk_rg_dos_type_t dos_type,int dos_enabled,rtk_rg_dos_action_t dos_action,int dos_threshold);
	rtk_rg_err_code_t (*rtk_rg_dosFloodType_get)(rtk_rg_dos_type_t dos_type,int *dos_enabled,rtk_rg_dos_action_t *dos_action,int *dos_threshold);
//70
	rtk_rg_err_code_t (*rtk_rg_portMirror_set)(rtk_rg_portMirrorInfo_t portMirrorInfo);
	rtk_rg_err_code_t (*rtk_rg_portMirror_get)(rtk_rg_portMirrorInfo_t *portMirrorInfo);
	rtk_rg_err_code_t (*rtk_rg_portMirror_clear)(void);
	rtk_rg_err_code_t (*rtk_rg_portEgrBandwidthCtrlRate_set)(rtk_rg_mac_port_idx_t port, uint32 rate);
	rtk_rg_err_code_t (*rtk_rg_portIgrBandwidthCtrlRate_set)(rtk_rg_mac_port_idx_t port, uint32 rate);
//75
	rtk_rg_err_code_t (*rtk_rg_portEgrBandwidthCtrlRate_get)(rtk_rg_mac_port_idx_t port, uint32 *rate);
	rtk_rg_err_code_t (*rtk_rg_portIgrBandwidthCtrlRate_get)(rtk_rg_mac_port_idx_t port, uint32 *rate);
	rtk_rg_err_code_t (*rtk_rg_phyPortForceAbility_set)(rtk_rg_mac_port_idx_t port, rtk_rg_phyPortAbilityInfo_t ability);
	rtk_rg_err_code_t (*rtk_rg_phyPortForceAbility_get)(rtk_rg_mac_port_idx_t port, rtk_rg_phyPortAbilityInfo_t *ability);
	rtk_rg_err_code_t (*rtk_rg_cpuPortForceTrafficCtrl_set)(rtk_rg_enable_t tx_fc_state,	rtk_rg_enable_t rx_fc_state);
//80
	rtk_rg_err_code_t (*rtk_rg_cpuPortForceTrafficCtrl_get)(rtk_rg_enable_t *pTx_fc_state,	rtk_rg_enable_t *pRx_fc_state);
	rtk_rg_err_code_t (*rtk_rg_portMibInfo_get)(rtk_rg_mac_port_idx_t port, rtk_rg_port_mib_info_t *mibInfo);
	rtk_rg_err_code_t (*rtk_rg_portMibInfo_clear)(rtk_rg_mac_port_idx_t port);
	rtk_rg_err_code_t (*rtk_rg_stormControl_add)(rtk_rg_stormControlInfo_t *stormInfo,int *stormInfo_idx);
	rtk_rg_err_code_t (*rtk_rg_stormControl_del)(int stormInfo_idx);
//85
	rtk_rg_err_code_t (*rtk_rg_stormControl_find)(rtk_rg_stormControlInfo_t *stormInfo,int *stormInfo_idx);
	rtk_rg_err_code_t (*rtk_rg_shareMeter_set)(uint32 index, uint32 rate, rtk_rg_enable_t ifgInclude);
	rtk_rg_err_code_t (*rtk_rg_shareMeter_get)(uint32 index, uint32 *pRate , rtk_rg_enable_t *pIfgInclude);
	rtk_rg_err_code_t (*rtk_rg_shareMeterMode_set)(uint32 index,rtk_rate_metet_mode_t meterMode);
	rtk_rg_err_code_t (*rtk_rg_shareMeterMode_get)(uint32 index,rtk_rate_metet_mode_t *pMeterMode);
//90
	rtk_rg_err_code_t (*rtk_rg_qosStrictPriorityOrWeightFairQueue_set)(rtk_rg_mac_port_idx_t port_idx,rtk_rg_qos_queue_weights_t q_weight);
	rtk_rg_err_code_t (*rtk_rg_qosStrictPriorityOrWeightFairQueue_get)(rtk_rg_mac_port_idx_t port_idx,rtk_rg_qos_queue_weights_t *pQ_weight);
	rtk_rg_err_code_t (*rtk_rg_qosInternalPriMapToQueueId_set)(int int_pri, int queue_id);
	rtk_rg_err_code_t (*rtk_rg_qosInternalPriMapToQueueId_get)(int int_pri, int *pQueue_id);
	rtk_rg_err_code_t (*rtk_rg_qosInternalPriDecisionByWeight_set)(rtk_rg_qos_priSelWeight_t weightOfPriSel);
//95
	rtk_rg_err_code_t (*rtk_rg_qosInternalPriDecisionByWeight_get)(rtk_rg_qos_priSelWeight_t *pWeightOfPriSel);
	rtk_rg_err_code_t (*rtk_rg_qosDscpRemapToInternalPri_set)(uint32 dscp,uint32 int_pri);
	rtk_rg_err_code_t (*rtk_rg_qosDscpRemapToInternalPri_get)(uint32 dscp,uint32 *pInt_pri);
	rtk_rg_err_code_t (*rtk_rg_qosPortBasedPriority_set)(rtk_rg_mac_port_idx_t port_idx,uint32 int_pri);
	rtk_rg_err_code_t (*rtk_rg_qosPortBasedPriority_get)(rtk_rg_mac_port_idx_t port_idx,uint32 *pInt_pri);
//100
	rtk_rg_err_code_t (*rtk_rg_qosDot1pPriRemapToInternalPri_set)(uint32 dot1p,uint32 int_pri);
	rtk_rg_err_code_t (*rtk_rg_qosDot1pPriRemapToInternalPri_get)(uint32 dot1p,uint32 *pInt_pri);
	rtk_rg_err_code_t (*rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_set)(rtk_rg_mac_port_idx_t rmk_port,rtk_rg_enable_t rmk_enable, rtk_rg_qos_dscpRmkSrc_t rmk_src_select);
	rtk_rg_err_code_t (*rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_get)(rtk_rg_mac_port_idx_t rmk_port,rtk_rg_enable_t *pRmk_enable, rtk_rg_qos_dscpRmkSrc_t *pRmk_src_select);
	rtk_rg_err_code_t (*rtk_rg_qosDscpRemarkByInternalPri_set)(int int_pri,int rmk_dscp);
//105
	rtk_rg_err_code_t (*rtk_rg_qosDscpRemarkByInternalPri_get)(int int_pri,int *pRmk_dscp);
	rtk_rg_err_code_t (*rtk_rg_qosDscpRemarkByDscp_set)(int dscp,int rmk_dscp);
	rtk_rg_err_code_t (*rtk_rg_qosDscpRemarkByDscp_get)(int dscp,int *pRmk_dscp);
	rtk_rg_err_code_t (*rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_set)(rtk_rg_mac_port_idx_t rmk_port, rtk_rg_enable_t rmk_enable);
	rtk_rg_err_code_t (*rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_get)(rtk_rg_mac_port_idx_t rmk_port, rtk_rg_enable_t *pRmk_enable);
//110
	rtk_rg_err_code_t (*rtk_rg_qosDot1pPriRemarkByInternalPri_set)(int int_pri,int rmk_dot1p);
	rtk_rg_err_code_t (*rtk_rg_qosDot1pPriRemarkByInternalPri_get)(int int_pri,int *pRmk_dot1p);
	rtk_rg_err_code_t (*rtk_rg_portBasedCVlanId_set)(rtk_rg_port_idx_t port_idx,int pvid);
	rtk_rg_err_code_t (*rtk_rg_portBasedCVlanId_get)(rtk_rg_port_idx_t port_idx,int *pPvid);
	rtk_rg_err_code_t (*rtk_rg_portStatus_get)(rtk_rg_mac_port_idx_t port, rtk_rg_portStatusInfo_t *portInfo);
//115
#ifdef CONFIG_RG_NAPT_PORT_COLLISION_PREVENTION
	rtk_rg_err_code_t (*rtk_rg_naptExtPortGet)(int isTcp,uint16 *pPort);
	rtk_rg_err_code_t (*rtk_rg_naptExtPortFree)(int isTcp,uint16 port);
#endif
	rtk_rg_err_code_t (*rtk_rg_classifyEntry_add)(rtk_rg_classifyEntry_t *classifyFilter);
	rtk_rg_err_code_t (*rtk_rg_classifyEntry_find)(int index, rtk_rg_classifyEntry_t *classifyFilter);
	rtk_rg_err_code_t (*rtk_rg_classifyEntry_del)(int index);
//120
	rtk_rg_err_code_t (*rtk_rg_svlanTpid_set)(uint32 svlan_tag_id);
	rtk_rg_err_code_t (*rtk_rg_svlanTpid_get)(uint32 *pSvlanTagId);
	rtk_rg_err_code_t (*rtk_rg_svlanServicePort_set)(rtk_port_t port, rtk_enable_t enable);
	rtk_rg_err_code_t (*rtk_rg_svlanServicePort_get)(rtk_port_t port, rtk_enable_t *pEnable);
	rtk_rg_err_code_t (*rtk_rg_pppoeInterfaceIdleTime_get)(int intfIdx,uint32 *idleSec);
//125
	rtk_rg_err_code_t (*rtk_rg_gatewayServicePortRegister_add)(rtk_rg_gatewayServicePortEntry_t *serviceEntry, int *index);
	rtk_rg_err_code_t (*rtk_rg_gatewayServicePortRegister_del)(int index);
	rtk_rg_err_code_t (*rtk_rg_gatewayServicePortRegister_find)(rtk_rg_gatewayServicePortEntry_t *serviceEntry, int *index);
	rtk_rg_err_code_t (*rtk_rg_wlanDevBasedCVlanId_set)(int wlan_idx,int dev_idx,int dvid);
	rtk_rg_err_code_t (*rtk_rg_wlanDevBasedCVlanId_get)(int wlan_idx,int dev_idx,int *pDvid);
//130
	rtk_rg_err_code_t (*rtk_rg_wlanSoftwareSourceAddrLearningLimit_set)(rtk_rg_saLearningLimitInfo_t sa_learnLimit_info, int wlan_idx, int dev_idx);
	rtk_rg_err_code_t (*rtk_rg_wlanSoftwareSourceAddrLearningLimit_get)(rtk_rg_saLearningLimitInfo_t *sa_learnLimit_info, int wlan_idx, int dev_idx);
	rtk_rg_err_code_t (*rtk_rg_naptFilterAndQos_add)(int *index,rtk_rg_naptFilterAndQos_t *napt_filter);
	rtk_rg_err_code_t (*rtk_rg_naptFilterAndQos_del)(int index);
	rtk_rg_err_code_t (*rtk_rg_naptFilterAndQos_find)(int *index,rtk_rg_naptFilterAndQos_t *napt_filter);
//135
	rtk_rg_err_code_t (*rtk_rg_pptpClientInfoBeforeDial_set)(int wan_intf_idx, rtk_rg_pptpClientInfoBeforeDial_t *app_info);
	rtk_rg_err_code_t (*rtk_rg_pptpClientInfoAfterDial_set)(int wan_intf_idx, rtk_rg_pptpClientInfoAfterDial_t *clientPptp_info);
	rtk_rg_err_code_t (*rtk_rg_l2tpClientInfoBeforeDial_set)(int wan_intf_idx, rtk_rg_l2tpClientInfoBeforeDial_t *app_info);
	rtk_rg_err_code_t (*rtk_rg_l2tpClientInfoAfterDial_set)(int wan_intf_idx, rtk_rg_l2tpClientInfoAfterDial_t *clientL2tp_info);
	rtk_rg_err_code_t (*rtk_rg_stpBlockingPortmask_set)(rtk_rg_portmask_t Mask);
//140
	rtk_rg_err_code_t (*rtk_rg_stpBlockingPortmask_get)(rtk_rg_portmask_t *pMask);
	rtk_rg_err_code_t (*rtk_rg_portIsolation_set)(rtk_rg_port_isolation_t isolationSetting);
	rtk_rg_err_code_t (*rtk_rg_portIsolation_get)(rtk_rg_port_isolation_t *isolationSetting);
	rtk_rg_err_code_t (*rtk_rg_dsliteInfo_set)(int wan_intf_idx, rtk_rg_ipDslitStaticInfo_t *dslite_info);
	rtk_rg_err_code_t (*rtk_rg_pppoeDsliteInfoBeforeDial_set)(int wan_intf_idx, rtk_rg_pppoeClientInfoBeforeDial_t *app_info);
//145
	rtk_rg_err_code_t (*rtk_rg_pppoeDsliteInfoAfterDial_set)(int wan_intf_idx, rtk_rg_pppoeDsliteInfoAfterDial_t *pppoeDslite_info);
	rtk_rg_err_code_t (*rtk_rg_gponDsBcFilterAndRemarking_add)(rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t *filterRule,int *index);
	rtk_rg_err_code_t (*rtk_rg_gponDsBcFilterAndRemarking_del)(int index);
	rtk_rg_err_code_t (*rtk_rg_gponDsBcFilterAndRemarking_find)(int *index,rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t *filterRule);
	rtk_rg_err_code_t (*rtk_rg_gponDsBcFilterAndRemarking_del_all)(void);
//150
	rtk_rg_err_code_t (*rtk_rg_gponDsBcFilterAndRemarking_Enable)(rtk_rg_enable_t enable);
	rtk_rg_err_code_t (*rtk_rg_interfaceMibCounter_del)(int intf_idx);
	rtk_rg_err_code_t (*rtk_rg_interfaceMibCounter_get)(rtk_rg_netifMib_entry_t *pNetifMib);
	rtk_rg_err_code_t (*rtk_rg_redirectHttpAll_set)(rtk_rg_redirectHttpAll_t *pRedirectHttpAll);
	rtk_rg_err_code_t (*rtk_rg_redirectHttpAll_get)(rtk_rg_redirectHttpAll_t *pRedirectHttpAll);
//155
	rtk_rg_err_code_t (*rtk_rg_redirectHttpURL_add)(rtk_rg_redirectHttpURL_t *pRedirectHttpURL);
	rtk_rg_err_code_t (*rtk_rg_redirectHttpURL_del)(rtk_rg_redirectHttpURL_t *pRedirectHttpURL);
	rtk_rg_err_code_t (*rtk_rg_redirectHttpWhiteList_add)(rtk_rg_redirectHttpWhiteList_t *pRedirectHttpWhiteList);
	rtk_rg_err_code_t (*rtk_rg_redirectHttpWhiteList_del)(rtk_rg_redirectHttpWhiteList_t *pRedirectHttpWhiteList);
	rtk_rg_err_code_t (*rtk_rg_redirectHttpRsp_set)(rtk_rg_redirectHttpRsp_t *pRedirectHttpRsp);
//160
	rtk_rg_err_code_t (*rtk_rg_redirectHttpRsp_get)(rtk_rg_redirectHttpRsp_t *pRedirectHttpRsp);
	rtk_rg_err_code_t (*rtk_rg_svlanTpid2_enable_set)(rtk_rg_enable_t enable);
	rtk_rg_err_code_t (*rtk_rg_svlanTpid2_enable_get)(rtk_rg_enable_t *pEnable);
	rtk_rg_err_code_t (*rtk_rg_svlanTpid2_set)(uint32 svlan_tag_id);
	rtk_rg_err_code_t (*rtk_rg_svlanTpid2_get)(uint32 *pSvlanTagId);
//165
	rtk_rg_err_code_t (*rtk_rg_hostPoliceControl_set)(rtk_rg_hostPoliceControl_t *pHostPoliceControl, int host_idx);
	rtk_rg_err_code_t (*rtk_rg_hostPoliceControl_get)(rtk_rg_hostPoliceControl_t *pHostPoliceControl, int host_idx);
	rtk_rg_err_code_t (*rtk_rg_hostPoliceLogging_get)(rtk_rg_hostPoliceLogging_t *pHostMibCnt, int host_idx);
	rtk_rg_err_code_t (*rtk_rg_hostPoliceLogging_del)(int host_idx);
	rtk_rg_err_code_t (*rtk_rg_redirectHttpCount_set)(rtk_rg_redirectHttpCount_t *pRedirectHttpCount);
//170
	rtk_rg_err_code_t (*rtk_rg_redirectHttpCount_get)(rtk_rg_redirectHttpCount_t *pRedirectHttpCount);
	rtk_rg_err_code_t (*rtk_rg_staticRoute_add)(rtk_rg_staticRoute_t *pStaticRoute, int *index);
	rtk_rg_err_code_t (*rtk_rg_staticRoute_del)(int index);
	rtk_rg_err_code_t (*rtk_rg_staticRoute_find)(rtk_rg_staticRoute_t *pStaticRoute, int *index);
	rtk_rg_err_code_t (*rtk_rg_aclLogCounterControl_get)(int index, int *type, int *mode);
//175
	rtk_rg_err_code_t (*rtk_rg_aclLogCounterControl_set)(int index, int type, int mode);
	rtk_rg_err_code_t (*rtk_rg_aclLogCounter_get)(int index, uint64 *count);
	rtk_rg_err_code_t (*rtk_rg_aclLogCounter_reset)(int index);
	rtk_rg_err_code_t (*rtk_rg_groupMacLimit_set)(rtk_rg_groupMacLimit_t group_mac_info);
	rtk_rg_err_code_t (*rtk_rg_groupMacLimit_get)(rtk_rg_groupMacLimit_t *group_mac_info);
//180
	rtk_rg_err_code_t (*rtk_rg_igmpMldSnoopingControl_set)(rtk_rg_igmpMldSnoopingControl_t *config );
	rtk_rg_err_code_t (*rtk_rg_igmpMldSnoopingControl_get)(rtk_rg_igmpMldSnoopingControl_t *config );
	rtk_rg_err_code_t (*rtk_rg_flowMibCounter_get)(int index, rtk_rg_table_flowmib_t *pCounter);
	rtk_rg_err_code_t (*rtk_rg_flowMibCounter_reset)(int index);
	rtk_rg_err_code_t (*rtk_rg_softwareIdleTime_set)(rtk_rg_idle_time_type_t idleTimeType, int idleTime);
//185
	rtk_rg_err_code_t (*rtk_rg_softwareIdleTime_get)(rtk_rg_idle_time_type_t idleTimeType, int *pIdleTime);
	rtk_rg_err_code_t (*rtk_rg_funcbasedMeter_set)(rtk_rg_funcbasedMeterConf_t meterConf);
	rtk_rg_err_code_t (*rtk_rg_funcbasedMeter_get)(rtk_rg_funcbasedMeterConf_t *meterConf);
	rtk_rg_err_code_t (*rtk_rg_flowHiPriEntry_add)(rtk_rg_table_highPriPatten_t hiPriEntry,int *entry_idx);
	rtk_rg_err_code_t (*rtk_rg_flowHiPriEntry_del)(int entry_idx);
//190
	rtk_rg_err_code_t (*rtk_rg_igmpMldSnoopingPortControl_add)(rtk_rg_port_idx_t port_idx,rtk_rg_igmpMldSnoopingPortControl_t *config );
	rtk_rg_err_code_t (*rtk_rg_igmpMldSnoopingPortControl_del)(rtk_rg_port_idx_t port_idx );
	rtk_rg_err_code_t (*rtk_rg_igmpMldSnoopingPortControl_find)(rtk_rg_port_idx_t port_idx,rtk_rg_igmpMldSnoopingPortControl_t *config );
	rtk_rg_err_code_t (*rtk_rg_callback_function_ptr_get)(rtk_rg_callbackFunctionPtrGet_t *callback_function_ptr_get_info);
	rtk_rg_err_code_t (*rtk_rg_vlanGroupMacLimit_add)(rtk_rg_mac_port_idx_t port, int groupLimit, int *pGroupIdx);
//195
	rtk_rg_err_code_t (*rtk_rg_vlanGroupMacLimit_set)(int groupIdx, int vlanId, int groupLimit);
	rtk_rg_err_code_t (*rtk_rg_vlanGroupMacLimit_del)(int groupIdx, int vlanId);
	rtk_rg_err_code_t (*rtk_rg_vlanGroupMacLimit_get)(int groupIdx, rtk_rg_vlanGroupMacLimit_info_t *pGroup);
	rtk_rg_err_code_t (*rtk_rg_vlanGroupMacLimit_find)(rtk_rg_mac_port_idx_t port, int vlanId, int *pGroupIdx);
	rtk_rg_err_code_t (*rtk_rg_dosFloodThresholdUnit_set)(rtk_rg_dos_type_t dos_type, int dos_threshUnit);
//200
	rtk_rg_err_code_t (*rtk_rg_dosFloodThresholdUnit_get)(rtk_rg_dos_type_t dos_type, int *pDos_threshUnit);
	rtk_rg_err_code_t (*rtk_rg_accessDot1xControl_set)(rtk_rg_accessDot1xCtrl_t blocking_ctrl);
	rtk_rg_err_code_t (*rtk_rg_accessDot1xFilter_set)(rtk_rg_accessDot1xFilter_t unblocking_filter);
	rtk_rg_err_code_t (*rtk_rg_urlflowPri_add)(rtk_rg_urlHighPri_t* urlPriEt,int *entry_idx);
	rtk_rg_err_code_t (*rtk_rg_urlflowPri_del)(int entry_idx);	
//205
	rtk_rg_err_code_t (*rtk_rg_portTrigger_add)(rtk_rg_portTrigger_info_t *port_trigger);
	rtk_rg_err_code_t (*rtk_rg_portTrigger_del)(rtk_rg_portTrigger_info_t *port_trigger);
	rtk_rg_err_code_t (*rtk_rg_vxlanClientInfo_set)(int wan_intf_idx, rtk_rg_vxlanClientInfoAfterDial_t *vxlanClient_info);
	rtk_rg_err_code_t (*rtk_rg_ipset_add)(rtk_rg_ipset_rule_t *ipset_rule, int *entry_idx);
	rtk_rg_err_code_t (*rtk_rg_ipset_del)(int set_idx, int index);
//210
	rtk_rg_err_code_t (*rtk_rg_ipsets_group_set)(rtk_rg_ipsets_group_cfg_t *ipsets_group_cfg, int index);
	rtk_rg_err_code_t (*rtk_rg_ipsets_set_add)(rtk_rg_ipsets_set_cfg_t *ipsets_set_cfg, int *index);
	rtk_rg_err_code_t (*rtk_rg_ipsets_set_del)(int index);
	rtk_rg_err_code_t (*rtk_rg_ipsets_rule_add)(rtk_rg_ipsets_rule_t *ipsets_rule_cfg, int *index);
	rtk_rg_err_code_t (*rtk_rg_ipsets_rule_del)(int set_idx, int index);
//215
};



#endif

