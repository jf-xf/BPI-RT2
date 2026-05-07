#ifndef __RT_EDP_DEFINE_H__
#define __RT_EDP_DEFINE_H__

#include <rt_edp_port.h>

/* common */
#ifndef NULL
#define NULL 0
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

#ifndef FAIL
#define FAIL -1
#endif

#define EDP_CB_CMD_BUFF_SIZE 256

#define EDP_IP_UTILITY "ip"
#define EDP_IPTABLES_UTILITY "iptables"
#define EDP_IP6TABLES_UTILITY "ip6tables"
#define EDP_EBTABLES_UTILITY "ebtables"
#define EDP_LAN_DEVICE_START_INDEX 2
#define EDP_LAN_DEVICE_NAME "eth0"
#define EDP_WAN_DEVICE_NAME "nas0"
#define EDP_WAN_TUNNEL_DEVICE_NAME "ppp"
#define EDP_BR_DEVICE_NAME "br0"
#define EDP_LOOPBACK_DEVICE_NAME "lo"

/* HW & SW Combine tables */
#define EDP_MAX_VLAN_HW_TABLE_SIZE 4096
#define EDP_MAX_VLAN_SW_TABLE_SIZE 8192
#define EDP_MAX_BIND_SW_TABLE_SIZE 32
#define EDP_MAX_NETIF_HW_TABLE_SIZE_SHIFT 4
#define EDP_MAX_NETIF_HW_TABLE_SIZE (1 << EDP_MAX_NETIF_HW_TABLE_SIZE_SHIFT)
#define EDP_MAX_NETIF_SW_TABLE_SIZE EDP_MAX_NETIF_HW_TABLE_SIZE
#define EDP_MAX_VIRTUAL_SERVER_SW_TABLE_SIZE		32
#define EDP_MAX_DMZ_TABLE_SIZE					32
#define EDP_MAX_ALG_SERV_IN_LAN_NUM				8

#define EDP_MAX_URL_FILTER_STR_LENGTH			128
#define EDP_MAX_URL_FILTER_PATH_LENGTH			256
#define EDP_MAX_LANNET_DEV_NAME_LENGTH			32
#define EDP_MAX_LANNET_BRAND_NAME_LENGTH		16
#define EDP_MAX_LANNET_MODEL_NAME_LENGTH		16
#define EDP_MAX_LANNET_OS_NAME_LENGTH			16
#define EDP_MAX_LANNET_SUB_BRAND_SIZE			10
#define EDP_MAX_LANNET_SUB_MODEL_SIZE			10
#define EDP_MAX_LANNET_SUB_OS_SIZE				10
#define EDP_MAX_URL_FILTER_ENTRY_SIZE 			512
#define EDP_MAX_URL_FILTER_BUF_LENGTH 			1024
#define EDP_MAX_URL_FILTER_STR_LENGTH			128
#define EDP_MAX_URL_FILTER_PATH_LENGTH			256
#define EDP_MAX_REDIRECT_PUSH_WEB_SIZE			768
#define EDP_MAX_REDIRECT_URL_NUM				8
#define EDP_MAX_REDIRECT_WHITE_LIST_NUM			8
#define EDP_MAX_MC_API_CONFIG_NUM				256

#define EDP_ACL_SUPPORT_HOOK_CHECK 0	//also define in rt_edp_acl.h

#ifdef CONFIG_MASTER_WLAN0_ENABLE
#ifdef CONFIG_RTL_WDS_SUPPORT
#ifdef CONFIG_RTL_VAP_SUPPORT
#define EDP_DEFAULT_BIND_LAN_OFFSET		18			//0,1,2,3 ext-0, ext-1, root, vap1, vap2, vap3, vap4, wds5, wds6, wds7 ,wds8, wds9, wds10, wds11, wds12
#else
#define EDP_DEFAULT_BIND_LAN_OFFSET		14			//0,1,2,3 ext-0, ext-1, root, wds1, wds2, wds3 ,wds4, wds5, wds6, wds7, wds8
#endif
#else
#ifdef CONFIG_RTL_VAP_SUPPORT
#define EDP_DEFAULT_BIND_LAN_OFFSET		10			//0,1,2,3 ext-0, ext-1, root, vap1, vap2, vap3, vap4
#else
#define EDP_DEFAULT_BIND_LAN_OFFSET		5			//0,1,2,3 ext-0, ext-1
#endif
#endif
#else
#define EDP_DEFAULT_BIND_LAN_OFFSET		5			//0,1,2,3 ext-0, ext-1
#endif

#define RT_EDP_PPPoE_MTU	1492
#define RT_EDP_ETH_MTU		1500

/* Queue weights structure */
#define EDP_MAX_NUM_OF_QUEUE 8

#if defined(CONFIG_RG_G3_SERIES)||defined(CONFIG_APOLLO_MODEL)
#define POINTER_CAST	unsigned long
#else
#define POINTER_CAST	unsigned int
#endif

#define RT_EDP_POLICY_ROUTE_TABLE_START 0x50
#define RT_EDP_WAN_INTF_MARK_SIZE 31

#if defined(CONFIG_RG_RTL9607C_SERIES)
/* dual CPU definition for ApolloPro */

#define RT_EDP_MAC_PORT_MAINCPU			RT_EDP_MAC_PORT_MASTERCPU_CORE0
#define RT_EDP_MAC_PORT_LASTCPU			RT_EDP_MAC_PORT_MASTERCPU_CORE1
#define RT_EDP_MAC_PORT_CPU 			RT_EDP_MAC_PORT_MASTERCPU_CORE1				// indicate the last physical port

#define RT_EDP_PORT_MAINCPU				RT_EDP_PORT_MASTERCPU_CORE0
#define RT_EDP_PORT_LASTCPU				RT_EDP_PORT_MASTERCPU_CORE1
#define RT_EDP_PORT_CPU					RT_EDP_PORT_MASTERCPU_CORE0				// NOTE: RG should not use RT_EDP_PORT_CPU anymore.

#define RT_EDP_MAC_EXT_BASED_PORT		RT_EDP_MAC_EXT_PORT0
#define RT_EDP_EXT_BASED_PORT			RT_EDP_EXT_PORT0

#elif defined(CONFIG_RG_RTL9603CVD_SERIES)
/* single CPU definition for 9603D */

#define RT_EDP_MAC_PORT_MAINCPU			RT_EDP_MAC_PORT_CPU
#define RT_EDP_MAC_PORT_LASTCPU			RT_EDP_MAC_PORT_CPU
#define RT_EDP_MAC_PORT_CPU 			RT_EDP_MAC_PORT_CPU				// indicate the last physical port

#define RT_EDP_PORT_MAINCPU				RT_EDP_PORT_CPU
#define RT_EDP_PORT_LASTCPU				RT_EDP_PORT_CPU
#define RT_EDP_PORT_SLAVECPU 			RT_EDP_PORT_CPU

#define RT_EDP_MAC_EXT_BASED_PORT		RT_EDP_MAC_EXT_PORT0
#define RT_EDP_EXT_BASED_PORT			RT_EDP_EXT_PORT0


#elif defined(CONFIG_RG_G3_SERIES)
/* single CPU definition for G3 series */
#define RT_EDP_MAC_PORT_MAINCPU			RT_EDP_MAC_PORT_CPU
#define RT_EDP_MAC_PORT_LASTCPU			RT_EDP_MAC_PORT_CPU_WLAN1_AND_OTHERS
#define RT_EDP_MAC_PORT_SLAVECPU 		RT_EDP_MAC_PORT_CPU

#define RT_EDP_PORT_MAINCPU				RT_EDP_PORT_CPU
#define RT_EDP_PORT_LASTCPU				RT_EDP_PORT_CPU_WLAN1_AND_OTHERS
#define RT_EDP_PORT_SLAVECPU 			RT_EDP_PORT_CPU

#define RT_EDP_MAC_EXT_BASED_PORT		RT_EDP_MAC_EXT_PORT0
#define RT_EDP_EXT_BASED_PORT			RT_EDP_EXT_PORT0

#else
#error
#endif

#if defined(CONFIG_RG_RTL9607C_SERIES)
#define RT_EDP_MAX_EXT_PORT (RT_EDP_PORT_MAX - RT_EDP_EXT_PORT0)
#define RT_EDP_ALL_PORTMASK ((1<<RT_EDP_PORT_MAX) - 1)
#define RT_EDP_ALL_MAC_PORTMASK ((1<<RT_EDP_MAC_PORT_MAX) - 1)
#define RT_EDP_ALL_CPU_PORTMASK ((1<<RT_EDP_PORT_SLAVECPU) | (1<<RT_EDP_PORT_MASTERCPU_CORE0) | (1<<RT_EDP_PORT_MASTERCPU_CORE1))
#define RT_EDP_ALL_MAC_CPU_PORTMASK ((1<<RT_EDP_MAC_PORT_SLAVECPU) | (1<<RT_EDP_MAC_PORT_MASTERCPU_CORE0) | (1<<RT_EDP_MAC_PORT_MASTERCPU_CORE1))
#define RT_EDP_ALL_MAC_PORTMASK_WITHOUT_CPU (RT_EDP_ALL_MAC_PORTMASK & ~RT_EDP_ALL_MAC_CPU_PORTMASK)
#define RT_EDP_ALL_MASTER_CPU_PORTMASK ((1<<RT_EDP_PORT_MASTERCPU_CORE0) | (1<<RT_EDP_PORT_MASTERCPU_CORE1))
#define RT_EDP_ALL_MAC_MASTER_CPU_PORTMASK ((1<<RT_EDP_MAC_PORT_MASTERCPU_CORE0) | (1<<RT_EDP_MAC_PORT_MASTERCPU_CORE1))
#define RT_EDP_ALL_MAC_SLAVE_CPU_PORTMASK (1<<RT_EDP_MAC_PORT_SLAVECPU)
#define RT_EDP_ALL_EXT_PORTMASK ((1<<RT_EDP_EXT_PORT0)|(1<<RT_EDP_EXT_PORT1)|(1<<RT_EDP_EXT_PORT2)|(1<<RT_EDP_EXT_PORT3)|(1<<RT_EDP_EXT_PORT4)|(1<<RT_EDP_EXT_PORT5)|(1<<RT_EDP_MAC10_EXT_PORT0)|(1<<RT_EDP_MAC10_EXT_PORT1)|(1<<RT_EDP_MAC10_EXT_PORT2)|(1<<RT_EDP_MAC10_EXT_PORT3)|(1<<RT_EDP_MAC10_EXT_PORT4)|(1<<RT_EDP_MAC10_EXT_PORT5)|(1<<RT_EDP_MAC7_EXT_PORT0)|(1<<RT_EDP_MAC7_EXT_PORT1)|(1<<RT_EDP_MAC7_EXT_PORT2)|(1<<RT_EDP_MAC7_EXT_PORT3)|(1<<RT_EDP_MAC7_EXT_PORT4)|(1<<RT_EDP_MAC7_EXT_PORT5))
#define RT_EDP_ALL_VIRUAL_PORTMASK ((1<<(RT_EDP_PORT_MAX-RT_EDP_EXT_PORT0))-1)
#if defined(CONFIG_MASTER_WLAN0_ENABLE) && defined(CONFIG_RG_FLOW_NEW_WIFI_MODE)
#define RT_EDP_ALL_MASTER_EXT_PORTMASK ((1<<RT_EDP_EXT_PORT0)|(1<<RT_EDP_EXT_PORT1)|(1<<RT_EDP_EXT_PORT2)|(1<<RT_EDP_EXT_PORT3)|(1<<RT_EDP_EXT_PORT4)|(1<<RT_EDP_EXT_PORT5)|(1<<RT_EDP_MAC10_EXT_PORT0)|(1<<RT_EDP_MAC10_EXT_PORT1)|(1<<RT_EDP_MAC10_EXT_PORT2)|(1<<RT_EDP_MAC10_EXT_PORT3)|(1<<RT_EDP_MAC10_EXT_PORT4)|(1<<RT_EDP_MAC10_EXT_PORT5))
#define RT_EDP_ALL_SLAVE_EXT_PORTMASK ((1<<RT_EDP_MAC7_EXT_PORT0)|(1<<RT_EDP_MAC7_EXT_PORT1)|(1<<RT_EDP_MAC7_EXT_PORT2)|(1<<RT_EDP_MAC7_EXT_PORT3)|(1<<RT_EDP_MAC7_EXT_PORT4)|(1<<RT_EDP_MAC7_EXT_PORT5))
#define RT_EDP_ALL_USED_VIRUAL_PORTMASK RT_EDP_ALL_VIRUAL_PORTMASK // T.B.D.
#else
#define RT_EDP_ALL_MASTER_EXT_PORTMASK (1<<RT_EDP_EXT_PORT0)
#define RT_EDP_ALL_SLAVE_EXT_PORTMASK (1<<RT_EDP_EXT_PORT1)
#define RT_EDP_ALL_USED_VIRUAL_PORTMASK ((1<<(RT_EDP_EXT_PORT0-RT_EDP_EXT_BASED_PORT))|(1<<(RT_EDP_EXT_PORT1-RT_EDP_EXT_BASED_PORT)))
#endif
#define RT_EDP_ALL_REAL_MASTER_EXT_PORTMASK RT_EDP_ALL_MASTER_EXT_PORTMASK
#define RT_EDP_ALL_REAL_SLAVE_EXT_PORTMASK RT_EDP_ALL_SLAVE_EXT_PORTMASK
#define RT_EDP_ALL_VLAN_MASTER_EXT_PORTMASK (RT_EDP_ALL_MASTER_EXT_PORTMASK>>RT_EDP_EXT_BASED_PORT)
#define RT_EDP_ALL_VLAN_SLAVE_EXT_PORTMASK (RT_EDP_ALL_SLAVE_EXT_PORTMASK>>RT_EDP_EXT_BASED_PORT)
#define RT_EDP_ALL_LAN_PORTMASK ((1<<RT_EDP_MAC_PORT0)|(1<<RT_EDP_MAC_PORT1)|(1<<RT_EDP_MAC_PORT2)|(1<<RT_EDP_MAC_PORT3)|(1<<RT_EDP_MAC_PORT4))

#elif defined(CONFIG_RG_RTL9603CVD_SERIES)
#define RT_EDP_MAX_EXT_PORT (RT_EDP_PORT_MAX - RT_EDP_EXT_PORT0)
#define RT_EDP_ALL_PORTMASK ((1<<RT_EDP_PORT_MAX) - 1)
#define RT_EDP_ALL_MAC_PORTMASK ((1<<RT_EDP_MAC_PORT_MAX) - 1)
#define RT_EDP_ALL_CPU_PORTMASK ((1<<RT_EDP_PORT_CPU))
#define RT_EDP_ALL_MAC_CPU_PORTMASK ((1<<RT_EDP_MAC_PORT_CPU))
#define RT_EDP_ALL_MAC_PORTMASK_WITHOUT_CPU ((1<<RT_EDP_MAC_PORT0)|(1<<RT_EDP_MAC_PORT1)|(1<<RT_EDP_MAC_PORT2)|(1<<RT_EDP_MAC_PORT3)|(1<<RT_EDP_MAC_PORT_PON))
#define RT_EDP_ALL_MASTER_CPU_PORTMASK ((1<<RT_EDP_PORT_CPU))
#define RT_EDP_ALL_MAC_MASTER_CPU_PORTMASK ((1<<RT_EDP_MAC_PORT_CPU))
#define RT_EDP_ALL_MAC_SLAVE_CPU_PORTMASK (1<<RT_EDP_MAC_PORT_CPU)
#define RT_EDP_ALL_EXT_PORTMASK ((1<<RT_EDP_EXT_PORT0)|(1<<RT_EDP_EXT_PORT1)|(1<<RT_EDP_EXT_PORT2)|(1<<RT_EDP_EXT_PORT3)|(1<<RT_EDP_EXT_PORT4)|(1<<RT_EDP_EXT_PORT5))
#define RT_EDP_ALL_VIRUAL_PORTMASK ((1<<(RT_EDP_PORT_MAX-RT_EDP_EXT_PORT0))-1)
#if defined(CONFIG_MASTER_WLAN0_ENABLE) && defined(CONFIG_RG_FLOW_NEW_WIFI_MODE)
#define RT_EDP_ALL_MASTER_EXT_PORTMASK ((1<<RT_EDP_EXT_PORT0)|(1<<RT_EDP_EXT_PORT1)|(1<<RT_EDP_EXT_PORT2)|(1<<RT_EDP_EXT_PORT3)|(1<<RT_EDP_EXT_PORT4)|(1<<RT_EDP_EXT_PORT5))
#define RT_EDP_ALL_SLAVE_EXT_PORTMASK (0)
#define RT_EDP_ALL_USED_VIRUAL_PORTMASK RT_EDP_ALL_VIRUAL_PORTMASK // T.B.D.
#else
#define RT_EDP_ALL_MASTER_EXT_PORTMASK (1<<RT_EDP_EXT_PORT0)
#define RT_EDP_ALL_SLAVE_EXT_PORTMASK (1<<RT_EDP_EXT_PORT1)
#define RT_EDP_ALL_USED_VIRUAL_PORTMASK ((1<<(RT_EDP_EXT_PORT0-RT_EDP_EXT_BASED_PORT))|(1<<(RT_EDP_EXT_PORT1-RT_EDP_EXT_BASED_PORT)))
#endif
#define RT_EDP_ALL_REAL_MASTER_EXT_PORTMASK RT_EDP_ALL_MASTER_EXT_PORTMASK
#define RT_EDP_ALL_REAL_SLAVE_EXT_PORTMASK RT_EDP_ALL_SLAVE_EXT_PORTMASK
#define RT_EDP_ALL_VLAN_MASTER_EXT_PORTMASK (RT_EDP_ALL_MASTER_EXT_PORTMASK>>RT_EDP_EXT_BASED_PORT)
#define RT_EDP_ALL_VLAN_SLAVE_EXT_PORTMASK (RT_EDP_ALL_SLAVE_EXT_PORTMASK>>RT_EDP_EXT_BASED_PORT)
#define RT_EDP_ALL_LAN_PORTMASK ((1<<RT_EDP_MAC_PORT0)|(1<<RT_EDP_MAC_PORT1)|(1<<RT_EDP_MAC_PORT2)|(1<<RT_EDP_MAC_PORT3))

#elif defined(CONFIG_RG_G3_SERIES)
#define RT_EDP_MAX_EXT_PORT (RT_EDP_PORT_MAX - RT_EDP_EXT_PORT0)
#define RT_EDP_ALL_CPU_PORTMASK ((1<<RT_EDP_PORT_CPU) | (1<<RT_EDP_PORT_CPU_WLAN0_ROOT) | (1<<RT_EDP_PORT_CPU_WLAN0_VAP0) | (1<<RT_EDP_PORT_CPU_WLAN0_VAP1) | (1<<RT_EDP_PORT_CPU_WLAN0_VAP2) | (1<<RT_EDP_PORT_CPU_WLAN0_VAP3) | (1<<RT_EDP_PORT_CPU_WLAN1_AND_OTHERS))
#define RT_EDP_ALL_MAC_CPU_PORTMASK ((1<<RT_EDP_MAC_PORT_CPU) | (1<<RT_EDP_MAC_PORT_CPU_WLAN0_ROOT) | (1<<RT_EDP_MAC_PORT_CPU_WLAN0_VAP0) | (1<<RT_EDP_MAC_PORT_CPU_WLAN0_VAP1) | (1<<RT_EDP_MAC_PORT_CPU_WLAN0_VAP2) | (1<<RT_EDP_MAC_PORT_CPU_WLAN0_VAP3) | (1<<RT_EDP_MAC_PORT_CPU_WLAN1_AND_OTHERS))
#if defined(CONFIG_ARCH_CORTINA_G3HGU)
#define RT_EDP_ALL_PORTMASK ((1<<RT_EDP_PORT0) | (1<<RT_EDP_PORT1) | (1<<RT_EDP_PORT2) | (1<<RT_EDP_PORT3) | (1<<RT_EDP_PORT4) | (1<<RT_EDP_PORT5) | (1<<RT_EDP_PORT6) | (1<<RT_EDP_PORT_PON) | (1<<RT_EDP_PORT_CPU) | (1<<RT_EDP_PORT_CPU_WLAN0_ROOT) | (1<<RT_EDP_PORT_CPU_WLAN0_VAP0) | (1<<RT_EDP_PORT_CPU_WLAN0_VAP1) | (1<<RT_EDP_PORT_CPU_WLAN0_VAP2) | (1<<RT_EDP_PORT_CPU_WLAN0_VAP3) | (1<<RT_EDP_PORT_CPU_WLAN1_AND_OTHERS) | (1<<RT_EDP_EXT_PORT0) | (1<<RT_EDP_EXT_PORT1) | (1<<RT_EDP_EXT_PORT2) | (1<<RT_EDP_EXT_PORT3))
#define RT_EDP_ALL_MAC_PORTMASK ((1<<RT_EDP_MAC_PORT0) | (1<<RT_EDP_MAC_PORT1) | (1<<RT_EDP_MAC_PORT2) | (1<<RT_EDP_MAC_PORT3) | (1<<RT_EDP_MAC_PORT4) | (1<<RT_EDP_MAC_PORT5) | (1<<RT_EDP_MAC_PORT6) | (1<<RT_EDP_MAC_PORT_PON) | (1<<RT_EDP_MAC_PORT_CPU) | (1<<RT_EDP_MAC_PORT_CPU_WLAN0_ROOT) | (1<<RT_EDP_MAC_PORT_CPU_WLAN0_VAP0) | (1<<RT_EDP_MAC_PORT_CPU_WLAN0_VAP1) | (1<<RT_EDP_MAC_PORT_CPU_WLAN0_VAP2) | (1<<RT_EDP_MAC_PORT_CPU_WLAN0_VAP3) | (1<<RT_EDP_MAC_PORT_CPU_WLAN1_AND_OTHERS))
#define RT_EDP_ALL_MAC_PORTMASK_WITHOUT_CPU ((1<<RT_EDP_MAC_PORT0) | (1<<RT_EDP_MAC_PORT1) | (1<<RT_EDP_MAC_PORT2) | (1<<RT_EDP_MAC_PORT3) | (1<<RT_EDP_MAC_PORT4) | (1<<RT_EDP_MAC_PORT5) | (1<<RT_EDP_MAC_PORT6) | (1<<RT_EDP_MAC_PORT_PON))
#define RT_EDP_ALL_LAN_PORTMASK ((1<<RT_EDP_MAC_PORT0)|(1<<RT_EDP_MAC_PORT1)|(1<<RT_EDP_MAC_PORT2)|(1<<RT_EDP_MAC_PORT3)|(1<<RT_EDP_MAC_PORT4)|(1<<RT_EDP_MAC_PORT5)|(1<<RT_EDP_MAC_PORT6))
#else
#define RT_EDP_ALL_PORTMASK ((1<<RT_EDP_PORT0) | (1<<RT_EDP_PORT1) | (1<<RT_EDP_PORT2) | (1<<RT_EDP_PORT3) | (1<<RT_EDP_PORT_PON) | (1<<RT_EDP_PORT_CPU) | (1<<RT_EDP_PORT_CPU_WLAN0_ROOT) | (1<<RT_EDP_PORT_CPU_WLAN0_VAP0) | (1<<RT_EDP_PORT_CPU_WLAN0_VAP1) | (1<<RT_EDP_PORT_CPU_WLAN0_VAP2) | (1<<RT_EDP_PORT_CPU_WLAN0_VAP3) | (1<<RT_EDP_PORT_CPU_WLAN1_AND_OTHERS) | (1<<RT_EDP_EXT_PORT0) | (1<<RT_EDP_EXT_PORT1) | (1<<RT_EDP_EXT_PORT2) | (1<<RT_EDP_EXT_PORT3))
#define RT_EDP_ALL_MAC_PORTMASK ((1<<RT_EDP_MAC_PORT0) | (1<<RT_EDP_MAC_PORT1) | (1<<RT_EDP_MAC_PORT2) | (1<<RT_EDP_MAC_PORT3) | (1<<RT_EDP_MAC_PORT_PON) | (1<<RT_EDP_MAC_PORT_CPU) | (1<<RT_EDP_MAC_PORT_CPU_WLAN0_ROOT) | (1<<RT_EDP_MAC_PORT_CPU_WLAN0_VAP0) | (1<<RT_EDP_MAC_PORT_CPU_WLAN0_VAP1) | (1<<RT_EDP_MAC_PORT_CPU_WLAN0_VAP2) | (1<<RT_EDP_MAC_PORT_CPU_WLAN0_VAP3) | (1<<RT_EDP_MAC_PORT_CPU_WLAN1_AND_OTHERS))
#define RT_EDP_ALL_MAC_PORTMASK_WITHOUT_CPU ((1<<RT_EDP_MAC_PORT0) | (1<<RT_EDP_MAC_PORT1) | (1<<RT_EDP_MAC_PORT2) | (1<<RT_EDP_MAC_PORT3) | (1<<RT_EDP_MAC_PORT_PON))
#define RT_EDP_ALL_LAN_PORTMASK ((1<<RT_EDP_MAC_PORT0)|(1<<RT_EDP_MAC_PORT1)|(1<<RT_EDP_MAC_PORT2)|(1<<RT_EDP_MAC_PORT3))
#endif
#define RT_EDP_ALL_MASTER_CPU_PORTMASK (1<<RT_EDP_PORT_CPU)
#define RT_EDP_ALL_MAC_MASTER_CPU_PORTMASK (1<<RT_EDP_MAC_PORT_CPU)
#define RT_EDP_ALL_MAC_SLAVE_CPU_PORTMASK (1<<RT_EDP_MAC_PORT_CPU)
#define RT_EDP_ALL_EXT_PORTMASK ((1<<RT_EDP_EXT_PORT0)|(1<<RT_EDP_EXT_PORT1)|(1<<RT_EDP_EXT_PORT2)|(1<<RT_EDP_EXT_PORT3))
#define RT_EDP_ALL_MASTER_EXT_PORTMASK (1<<RT_EDP_EXT_PORT0)
#define RT_EDP_ALL_SLAVE_EXT_PORTMASK (1<<RT_EDP_EXT_PORT1)
#define RT_EDP_ALL_REAL_MASTER_EXT_PORTMASK ((1<<RT_EDP_PORT_CPU_WLAN0_ROOT) | (1<<RT_EDP_PORT_CPU_WLAN0_VAP0) | (1<<RT_EDP_PORT_CPU_WLAN0_VAP1) | (1<<RT_EDP_PORT_CPU_WLAN0_VAP2) | (1<<RT_EDP_PORT_CPU_WLAN0_VAP3) | (1<<RT_EDP_PORT_CPU_WLAN1_AND_OTHERS))
#define RT_EDP_ALL_REAL_SLAVE_EXT_PORTMASK (1<<RT_EDP_PORT_CPU_WLAN1_AND_OTHERS)
#define RT_EDP_ALL_VLAN_MASTER_EXT_PORTMASK (RT_EDP_ALL_MASTER_EXT_PORTMASK>>RT_EDP_EXT_BASED_PORT)
#define RT_EDP_ALL_VLAN_SLAVE_EXT_PORTMASK (RT_EDP_ALL_SLAVE_EXT_PORTMASK>>RT_EDP_EXT_BASED_PORT)
#define RT_EDP_ALL_VIRUAL_PORTMASK ((1<<(RT_EDP_PORT_MAX-RT_EDP_EXT_PORT0))-1)
#define RT_EDP_ALL_USED_VIRUAL_PORTMASK ((1<<(RT_EDP_EXT_PORT0-RT_EDP_EXT_BASED_PORT))|(1<<(RT_EDP_EXT_PORT1-RT_EDP_EXT_BASED_PORT)))

#else
#error
#endif

#if defined(EDP_ACL_SUPPORT_HOOK_CHECK) && (EDP_ACL_SUPPORT_HOOK_CHECK==1)
#define RT_EDP_IS_WAN_PORT(port) ((port==RT_EDP_PORT_PON) || (((0x1<<port) & rt_edp_db.systemGlobal.wanPortMask.portmask) != 0x0))
#endif

#define RT_EDP_INVALID_PORT(port) (((0x1<<port) & RT_EDP_ALL_PORTMASK)==0x0)
//#define RT_EDP_INVALID_MAC_PORT(port) (((0x1<<port) & RT_EDP_ALL_MAC_PORTMASK & rt_edp_db.systemGlobal.phyPortStatus)==0x0)
#define RT_EDP_INVALID_MAC_PORT(port) (((0x1<<port) & RT_EDP_ALL_MAC_PORTMASK)==0x0)
#define RT_EDP_INVALID_PORTMASK(portMsk) ((portMsk & RT_EDP_ALL_PORTMASK)!=portMsk)
//#define RT_EDP_INVALID_MAC_PORTMASK(portMsk) ((portMsk & RT_EDP_ALL_MAC_PORTMASK & rt_edp_db.systemGlobal.phyPortStatus)!=portMsk)
#define RT_EDP_INVALID_MAC_PORTMASK(portMsk) ((portMsk & RT_EDP_ALL_MAC_PORTMASK)!=portMsk)


/* For debug */
#define RT_EDP_RETURN_ERR(err_name)  do { return _rt_edp_return_err_mapping(err_name,#err_name,__LINE__); } while(0)




#if 0	//1 To prevent the data structure of parameter of RG API is changed, use orginal RG data structure temporarily.
//===============Fixme===============
#define rtk_rg_err_code_t rt_edp_err_code_t
#define rtk_rg_port_idx_t rt_edp_port_idx_t
#define rtk_rg_mac_port_idx_t rt_edp_mac_port_idx_t
#define rtk_rg_portmask_t rt_edp_portmask_t
#define rtk_rg_mac_portmask_t rt_edp_mac_portmask_t
#define rtk_rg_enable_t rt_edp_enable_t
#define rtk_rg_VersionString_t rt_edp_VersionString_t
#define rtk_rg_initParams_t rt_edp_initParams_t
#define rtk_rg_lanIntfConf_t rt_edp_lanIntfConf_t
#define rtk_rg_wanIntfConf_t rt_edp_wanIntfConf_t
#define rtk_rg_ipStaticInfo_t rt_edp_ipStaticInfo_t
#define rtk_rg_ipDslitStaticInfo_t rt_edp_ipDslitStaticInfo_t
#define rtk_rg_ipDhcpClientInfo_t rt_edp_ipDhcpClientInfo_t
#define rtk_rg_pppoeClientInfoBeforeDial_t rt_edp_pppoeClientInfoBeforeDial_t
#define rtk_rg_pppoeClientInfoAfterDial_t rt_edp_pppoeClientInfoAfterDial_t
#define rtk_rg_pptpClientInfoBeforeDial_t rt_edp_pptpClientInfoBeforeDial_t
#define rtk_rg_pptpClientInfoAfterDial_t rt_edp_pptpClientInfoAfterDial_t
#define rtk_rg_l2tpClientInfoBeforeDial_t rt_edp_l2tpClientInfoBeforeDial_t
#define rtk_rg_l2tpClientInfoAfterDial_t rt_edp_l2tpClientInfoAfterDial_t
#define rtk_rg_pppoeDsliteInfoAfterDial_t rt_edp_pppoeDsliteInfoAfterDial_t
#define rtk_rg_intfInfo_t rt_edp_intfInfo_t
#define rtk_rg_cvlan_info_t rt_edp_cvlan_info_t
#define rtk_rg_vlanBinding_t rt_edp_vlanBinding_t
#define rtk_rg_alg_serverIpMapping_t rt_edp_alg_serverIpMapping_t
#define rtk_rg_alg_type_t rt_edp_alg_type_t
#define rtk_rg_dmzInfo_t rt_edp_dmzInfo_t
#define rtk_rg_virtualServer_t rt_edp_virtualServer_t
#define rtk_rg_aclFilterAndQos_t rt_edp_aclFilterAndQos_t
#define rtk_rg_naptFilterAndQos_t rt_edp_naptFilterAndQos_t
#define rtk_rg_macFilterEntry_t rt_edp_macFilterEntry_t
#define rtk_rg_urlFilterString_t rt_edp_urlFilterString_t
#define rtk_rg_upnpConnection_t rt_edp_upnpConnection_t
#define rtk_rg_naptEntry_t rt_edp_naptEntry_t
#define rtk_rg_naptInfo_t rt_edp_naptInfo_t
#define rtk_rg_multicastFlow_t rt_edp_multicastFlow_t
#define rtk_rg_l2MulticastFlow_t rt_edp_l2MulticastFlow_t
#define rtk_rg_macEntry_t rt_edp_macEntry_t
#define rtk_rg_arpEntry_t rt_edp_arpEntry_t
#define rtk_rg_arpInfo_t rt_edp_arpInfo_t
#define rtk_rg_neighborEntry_t rt_edp_neighborEntry_t
#define rtk_rg_neighborInfo_t rt_edp_neighborInfo_t
#define rtk_rg_idle_time_type_t rt_edp_idle_time_type_t
#define rtk_rg_accessWanLimitData_t rt_edp_accessWanLimitData_t
#define rtk_rg_accessWanLimitCategory_t rt_edp_accessWanLimitCategory_t
#define rtk_rg_saLearningLimitInfo_t rt_edp_saLearningLimitInfo_t
#define rtk_rg_dos_type_t rt_edp_dos_type_t
#define rtk_rg_dos_action_t rt_edp_dos_action_t
#define rtk_rg_portMirrorInfo_t rt_edp_portMirrorInfo_t
#define rtk_rg_phyPortAbilityInfo_t rt_edp_phyPortAbilityInfo_t
#define rtk_rg_port_mib_info_t rt_edp_port_mib_info_t
#define rtk_rg_port_isolation_t rt_edp_port_isolation_t
#define rtk_rg_stormControlInfo_t rt_edp_stormControlInfo_t
#define rtk_rg_qos_queue_weights_t rt_edp_qos_queue_weights_t
#define rtk_rg_qos_priSelWeight_t rt_edp_qos_priSelWeight_t
#define rtk_rg_qos_dscpRmkSrc_t rt_edp_qos_dscpRmkSrc_t
#define rtk_rg_portStatusInfo_t rt_edp_portStatusInfo_t
#define rtk_rg_classifyEntry_t rt_edp_classifyEntry_t
#define rtk_rg_gatewayServicePortEntry_t rt_edp_gatewayServicePortEntry_t
#define rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t rt_edp_gpon_ds_bc_vlanfilterAndRemarking_t
#define rtk_rg_netifMib_entry_t rt_edp_netifMib_entry_t
#define rtk_rg_redirectHttpAll_t rt_edp_redirectHttpAll_t
#define rtk_rg_redirectHttpURL_t rt_edp_redirectHttpURL_t
#define rtk_rg_redirectHttpWhiteList_t rt_edp_redirectHttpWhiteList_t
#define rtk_rg_redirectHttpRsp_t rt_edp_redirectHttpRsp_t
#define rtk_rg_redirectHttpCount_t rt_edp_redirectHttpCount_t
#define rtk_rg_hostPoliceControl_t rt_edp_hostPoliceControl_t
#define rtk_rg_hostPoliceLogging_t rt_edp_hostPoliceLogging_t
#define rtk_rg_staticRoute_t rt_edp_staticRoute_t
#define rtk_rg_groupMacLimit_t rt_edp_groupMacLimit_t
#define rtk_rg_igmpMldSnoopingControl_t rt_edp_igmpMldSnoopingControl_t
#define rtk_rg_igmpMldSnoopingPortControl_t rt_edp_igmpMldSnoopingPortControl_t
#define rtk_rg_table_flowmib_t rt_edp_table_flowmib_t
#define rtk_rg_funcbasedMeterConf_t rt_edp_funcbasedMeterConf_t
#define rtk_rg_table_highPriPatten_t rt_edp_table_highPriPatten_t
#define rtk_rg_callbackFunctionPtrGet_t rt_edp_callbackFunctionPtrGet_t
#define rtk_rg_macFilterWhiteList_t rt_edp_macFilterWhiteList_t
#endif

#endif
