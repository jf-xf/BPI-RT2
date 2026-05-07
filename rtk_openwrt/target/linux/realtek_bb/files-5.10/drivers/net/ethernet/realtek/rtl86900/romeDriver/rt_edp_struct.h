#ifndef __RT_EDP_STRUCT_H__
#define __RT_EDP_STRUCT_H__

#include <rtk/l2.h>
#include <rtk/l34.h>
#include <rtk/rate.h>
#include <rt_edp_error.h>
#include <rt_edp_define.h>
#include <rt_edp_port.h>
//1 ===== FIXME: To prevent the data structure of parameter of RG API is changed, use orginal RG data structure temporarily. =====
#include <rtk_rg_struct.h>
extern rtk_rg_globalKernel_t rg_kernel;
#undef WARNING
#undef IGMP
#undef DEBUG
//1 ===== end FIXME =====

#include <rtk_fc_api.h>
#include <rt_edp_acl.h>	// need rtk_rg_struct definition
#include <rt_edp_acl_ext.h>




typedef struct rt_edp_dev_port_mapping_s
{
    int port_idx;
    char dev_name[16];
}rt_edp_dev_port_mapping_t;

typedef struct rt_edp_mac_portmask_s
{
	uint32 portmask;	//the portmask bit should be defined by rt_edp_mac_port_idx_t.
}rt_edp_mac_portmask_t;

typedef struct rt_edp_portmask_e
{
	uint32 portmask;	//the portmask bit should be defined by rt_edp_port_idx_t.
}rt_edp_portmask_t;

typedef struct rt_edp_VersionString_s
{
	char version_string[128];
} rt_edp_VersionString_t;

//System
typedef enum rt_edp_ip_version_e
{
	RT_EDP_IPVER_V4ONLY=0,
	RT_EDP_IPVER_V6ONLY=1,
	RT_EDP_IPVER_V4V6=2,
	RT_EDP_IPVER_END,
}rt_edp_ip_version_t;

typedef enum rt_edp_bridgeBindWan_ip_version_e
{
	RT_EDP_BGBWP_NONE=0,
	RT_EDP_BGBWP_V4ONLY,
	RT_EDP_BGBWP_V6ONLY,
	RT_EDP_BGBWP_V4V6,
	RT_EDP_BGBWP_END,
}rt_edp_bridgeBindWan_ip_version_t;

typedef enum rt_edp_ip_updated_e
{
	RT_EDP_ONLY_IPV4_UPDATED=0,
	RT_EDP_ONLY_IPV6_UPDATED,
	RT_EDP_IPV4_IPV6_UPDATED,
	RT_EDP_NO_IP_UPDATED,
}rt_edp_ip_updated_t;

typedef enum rt_edp_entryStatus_e
{
	RT_EDP_INVALID_ENTRY=0,
	RT_EDP_SOFTWARE_HARDWARE_SYNC_ENTRY=1,
	RT_EDP_SOFTWARE_ONLY_ENTRY=2,
	RT_EDP_HARDWARE_ONLY_ENTRY=3,
} rt_edp_entryStatus_t;

typedef enum rt_edp_interface_entryStatus_e
{
//valid flag
#define VALID_ENTRY 1
#define SOFTWARE4_ENTRY 2
#define SOFTWARE6_ENTRY 3

	RT_EDP_IF_INVALID_ENTRY		= 0,						// 0
	RT_EDP_IF_VALID_ENTRY	  	= 1<<VALID_ENTRY,		// 1
	RT_EDP_IF_SOFTWARE4_ENTRY	= 1<<SOFTWARE4_ENTRY,	// 2
	RT_EDP_IF_SOFTWARE6_ENTRY	= 1<<SOFTWARE6_ENTRY,	// 4
} rt_edp_interface_Status_t;

typedef enum rt_edp_enable_e
{
    RT_EDP_DISABLED = 0,
    RT_EDP_ENABLED,
    RT_EDP_ENABLE_END
} rt_edp_enable_t;

typedef enum rt_edp_wan_dev_type_e
{
    RT_EDP_WAN_DEV_TYPE_NAS = 0,	// means nas0_x
    RT_EDP_WAN_DEV_TYPE_PPP,		// means pppx
    RT_EDP_WAN_DEV_TYPE_END
} rt_edp_wan_dev_type_t;


typedef struct rt_edp_table_bind_s
{
	rtk_binding_entry_t rtk_bind;

	//SW table specific
	rt_edp_entryStatus_t valid; // used to software entry
} rt_edp_table_bind_t;

typedef struct rt_edp_table_vlan_s
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
	int valid;
	int addedAsCustomerVLAN;		//used to check this vlan created by cvlan apis or not
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	unsigned int wlan0DevMask;
	unsigned int wlan0UntagMask;
#endif
#if defined (CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	uint8 extPMaskIdx;
#endif
} rt_edp_table_vlan_t;


typedef struct rt_edp_table_mcapi_s
{
	int valid:1;
	rtk_fc_mcGroup_cfg_t mcConfig;
} rt_edp_table_mcapi_t;



//DMZ
typedef struct rt_edp_dmzInfo_s
{
	unsigned int enabled:1;
	unsigned int mac_mapping_enabled:1;
	rt_edp_ip_version_t ipversion; 	//0:v4_only 1:v6_only 2:both_v4_v6
	union
	{
		ipaddr_t private_ip; 		//only used when mac_mapping_enabled=0 && (ipversion==0 || ipversion==2)
		rtk_mac_t mac; 				//only used when mac_mapping_enabled=1
	};
	rtk_ipv6_addr_t	private_ipv6;	//only used when mac_mapping_enabled=0 && (ipversion==1 || ipversion==2)

} rt_edp_dmzInfo_t;

typedef struct rt_edp_initParams_s
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

} rt_edp_initParams_t;

// LAN Interface
typedef struct rt_edp_lanIntfConf_s
{
	rt_edp_ip_version_t ip_version;		//0: ipv4, 1: ipv6, 2:both v4 & v6
	rtk_mac_t gmac;
	ipaddr_t ip_addr;
	ipaddr_t ip_network_mask;
	rtk_ipv6_addr_t ipv6_addr;
	int ipv6_network_mask_length;
	rt_edp_portmask_t port_mask;
	rt_edp_mac_portmask_t untag_mask;
	int intf_vlan_id;
	rt_edp_enable_t vlan_based_pri_enable; //xdsl not support vlan pri, should set DISABLED
	int vlan_based_pri;
	unsigned int mtu;
	unsigned int isIVL:1;				//0: SVL, 1:IVL
	unsigned int replace_subnet:1;		//0: add one more subnet, 1:delete old, add new one
} rt_edp_lanIntfConf_t;

//WAN Interface
typedef enum rt_edp_wan_type_e
{
	RT_EDP_STATIC=0,
	RT_EDP_DHCP,
	RT_EDP_PPPoE,
	RT_EDP_BRIDGE,
	RT_EDP_PPTP,
	RT_EDP_L2TP,		//5
	RT_EDP_DSLITE,
	RT_EDP_PPPoE_DSLITE,
	RT_EDP_WAN_TYPE_END,
} rt_edp_wan_type_t;

typedef enum rt_edp_wirelessWan_e
{
	RT_EDP_WWAN_WIRED=0,
	RT_EDP_WWAN_WLAN0_VXD=13,		//wlan0dev-idx
	RT_EDP_WWAN_WLAN1_VXD=27,		//wlan0dev-idx
}rt_edp_wirelessWan_t;

typedef struct rt_edp_wanIntfConf_s
{
	rt_edp_wan_type_t wan_type;
	rtk_mac_t gmac;
	rt_edp_port_idx_t wan_port_idx;
	rt_edp_portmask_t port_binding_mask;
	rt_edp_portmask_t vlan_binding_mask;	//update by vlan-binding api
	unsigned int egress_vlan_tag_on:1;
	int egress_vlan_id;
	rt_edp_enable_t vlan_based_pri_enable;
	int vlan_based_pri;
	unsigned int isIVL:1;				//0: SVL, 1:IVL
	unsigned int none_internet:1;		//0: internet, 1:other
	unsigned int forcedAddNewIntf:1;	//0:normal, 1:forced create new interface
	unsigned int wlan0_dev_binding_mask;
	rt_edp_bridgeBindWan_ip_version_t bridgeToBindingWanByProtocol;	//0: none, 1: ipv4, 2: ipv6, 3:both v4 & v6
} rt_edp_wanIntfConf_t;

typedef struct rt_edp_ipStaticInfo_s
{
	rt_edp_ip_version_t ip_version;		//0: ipv4, 1: ipv6, 2:both v4 & v6
	unsigned int  napt_enable:1; // L3 or L4
	unsigned int  ipv6_napt_enable:1;//support for ipv6 napt
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
} rt_edp_ipStaticInfo_t;

typedef enum rt_edp_dhcp_status_e
{
	RT_EDP_DHCP_STATUS_LEASED=0,
	RT_EDP_DHCP_STATUS_RELEASED=1,
	RT_EDP_DHCP_STATUS_END
} rt_edp_dhcp_status_t;

typedef struct rt_edp_ipDhcpClientInfo_s
{
	rt_edp_ipStaticInfo_t hw_info;
	rt_edp_dhcp_status_t stauts; //leased, released
} rt_edp_ipDhcpClientInfo_t;

typedef enum rt_edp_ppp_auth_type_e
{
	RT_EDP_PPP_AUTH_TYPE_PAP=0,
	RT_EDP_PPP_AUTH_TYPE_CHAP=1,
	RT_EDP_PPP_AUTH_TYPE_END
} rt_edp_ppp_auth_type_t;

typedef int (*p_rtDialOnDemondCallBack)(unsigned int);
typedef int (*p_rtIdleTimeOutCallBack)(unsigned int);

typedef enum rt_edp_ppp_status_e
{
	RT_EDP_PPP_STATUS_DISCONNECT=0,
	RT_EDP_PPP_STATUS_CONNECT=1,
	RT_EDP_PPP_STATUS_END
} rt_edp_ppp_status_t;

typedef struct rt_edp_pppoeClientInfoBeforeDial_s
{
	char username[32];
	char password[32];
	rt_edp_ppp_auth_type_t auth_type;
	int pppoe_proxy_enable;
	int max_pppoe_proxy_num;
	int auto_reconnect;
	int dial_on_demond;
	int idle_timeout_secs;
	rt_edp_ppp_status_t stauts; //connect/disconnect?
	p_rtDialOnDemondCallBack dialOnDemondCallBack;
	p_rtIdleTimeOutCallBack idleTimeOutCallBack;
} rt_edp_pppoeClientInfoBeforeDial_t;

typedef struct rt_edp_pppoeClientInfoAfterDial_s
{
	rt_edp_ipStaticInfo_t hw_info;
	unsigned short int sessionId;
	//unsigned short int gpon_pppoe_status;
} rt_edp_pppoeClientInfoAfterDial_t;

typedef struct rt_edp_ipPppoeClientInfo_s
{
	rt_edp_pppoeClientInfoBeforeDial_t before_dial;
	rt_edp_pppoeClientInfoAfterDial_t after_dial;
} rt_edp_ipPppoeClientInfo_t;

typedef struct rt_edp_ipDsliteStaticInfo_s
{
	rt_edp_ipStaticInfo_t static_info;
	rtk_l34_dsliteInf_entry_t rtk_dslite;
	uint32 aftr_mac_auto_learn;
	rtk_mac_t aftr_mac_addr;
} rt_edp_ipDslitStaticInfo_t;

typedef struct rt_edp_pppoeDsliteInfoAfterDial_s
{
	rt_edp_ipDslitStaticInfo_t dslite_hw_info;
	unsigned short int sessionId;
} rt_edp_pppoeDsliteInfoAfterDial_t;

typedef struct rt_edp_ipPppoeDsliteInfo_s
{
	rt_edp_pppoeClientInfoBeforeDial_t before_dial;
	rt_edp_pppoeDsliteInfoAfterDial_t after_dial;
} rt_edp_ipPppoeDsliteInfo_t;

typedef struct rt_edp_pptpClientInfoBeforeDial_s
{
	char username[32];
	char password[32];
	ipaddr_t pptp_ipv4_addr;
} rt_edp_pptpClientInfoBeforeDial_t;

typedef struct rt_edp_pptpClientInfoAfterDial_s
{
	rt_edp_ipStaticInfo_t hw_info;

	unsigned short callId;
	unsigned short gateway_callId;
} rt_edp_pptpClientInfoAfterDial_t;

typedef struct rt_edp_ipPPTPClientInfo_s
{
	rt_edp_pptpClientInfoBeforeDial_t before_dial;
	rt_edp_pptpClientInfoAfterDial_t after_dial;

	unsigned int gre_header_sequence;
	unsigned int gre_header_acknowledgment;
	unsigned short ipv4_header_identifier;
	unsigned int sw_gre_header_sequence;	//from protocol stack output sequence
	unsigned int sw_gre_header_server_sequence;	//from server input sequence
	unsigned int sw_gre_header_server_sequence_started:1;
} rt_edp_ipPPTPClientInfo_t;

typedef struct rt_edp_l2tpClientInfoBeforeDial_s
{
	char username[32];
	char password[32];
	ipaddr_t l2tp_ipv4_addr;
} rt_edp_l2tpClientInfoBeforeDial_t;

typedef struct rt_edp_l2tpClientInfoAfterDial_s
{
	rt_edp_ipStaticInfo_t hw_info;

	unsigned short outer_port;				//used in outer UDP encapsulation
	unsigned short gateway_outer_port;		//used in outer UDP encapsulation
	unsigned short tunnelId;
	unsigned short gateway_tunnelId;
	unsigned short sessionId;
	unsigned short gateway_sessionId;
} rt_edp_l2tpClientInfoAfterDial_t;

typedef struct rt_edp_ipL2TPClientInfo_s
{
	rt_edp_l2tpClientInfoBeforeDial_t before_dial;
	rt_edp_l2tpClientInfoAfterDial_t after_dial;

	unsigned short ipv4_header_identifier;
} rt_edp_ipL2TPClientInfo_t;

typedef struct rt_edp_wanIntfInfo_s
{
	rt_edp_wanIntfConf_t wan_intf_conf;
	union{
		rt_edp_ipStaticInfo_t static_info;
		rt_edp_ipDhcpClientInfo_t dhcp_client_info;
		rt_edp_ipPppoeClientInfo_t pppoe_info;
		rt_edp_ipPPTPClientInfo_t pptp_info;
		rt_edp_ipL2TPClientInfo_t l2tp_info;
		rt_edp_ipDslitStaticInfo_t dslite_info;
		rt_edp_ipPppoeDsliteInfo_t pppoe_dslite_info;
	};
	int bind_wan_type_ipv4;
	int bind_wan_type_ipv6;
	rt_edp_wirelessWan_t wirelessWan;	//0:wire WAN, 13:wireless WAN(wlan0-vxd), 27:wireless WAN(wlan1-vxd)
	int nexthop_ipv4;
	int nexthop_ipv6;
	int pppoe_idx;		//idx for pppoe table
	int extip_idx;		//idx for ext ip table
	int baseIntf_idx;	//for pptp/l2tp
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	// for dual header
	int flowEntryIdx;				// path6 flow entry index, initialized as -1
	int16 extraTagActionListIdx;	// action list index, starts from 1 and initialized as 0
#endif
	int devIdx; 		// idx for nas0_x
	int tunnelDevIdx; 	// idx for pppx
	char devName[IFNAMSIZ]; //device name, ex:nas0_0
	char tunnelDevName[IFNAMSIZ]; //tunnel device name, ex:pppx
} rt_edp_wanIntfInfo_t;

typedef struct rt_edp_intfInfo_s
{
	char intf_name[32];
	int is_wan;
	union
	{
		rt_edp_lanIntfConf_t lan_intf;
		rt_edp_wanIntfInfo_t wan_intf;
	};
} rt_edp_intfInfo_t;

typedef struct rt_edp_interface_info_global_s
{
	rt_edp_intfInfo_t storedInfo;
	rt_edp_interface_Status_t valid;	// vaild mask indicate v4/v6 software entry
	int lan_or_wan_index;		//index of lan or wan group
	union{
		rt_edp_lanIntfConf_t *p_lanIntfConf;
		rt_edp_ipStaticInfo_t *p_wanStaticInfo;
	};
#if defined(CONFIG_RG_G3_SERIES)
	int32 clsIdx_option_trap[RT_EDP_PORT_MAX];			// 0 if invalid
#endif
} rt_edp_interface_info_global_t;

typedef struct rt_edp_wan_interface_group_info_s
{
	rt_edp_interface_info_global_t *p_intfInfo;
	rt_edp_wanIntfConf_t *p_wanIntfConf;
	unsigned int disableBroadcast:1;		//only for BD WAN which has same VLANID as LAN intf
	int index;		//index of netif table
} rt_edp_wan_interface_group_info_t;

typedef struct rt_edp_lan_interface_group_info_s
{
	rt_edp_interface_info_global_t *p_intfInfo;
	int index;		//index of netif table
} rt_edp_lan_interface_group_info_t;

//+++++++++++++++ ACL Related Structure Start +++++++++++++++
#if defined(EDP_ACL_SUPPORT_HOOK_CHECK) && (EDP_ACL_SUPPORT_HOOK_CHECK==1)
typedef struct rt_edp_aclCheck_data_s
{
	char dev_name[IFNAMSIZ];			// for ingress_port_mask
	char fromDev_name[IFNAMSIZ];		// for ingress_port_mask
	int devPort;
	int fromDevPort;
	unsigned int tagif;			// rt_edp_pkthdr_tagif_t, ref to rtk_rg_fc_enum_pktHdrTagif_t
	unsigned char smac[ETHER_ADDR_LEN];
	unsigned char dmac[ETHER_ADDR_LEN];
	int ethertype;
	unsigned short stagVid:12;			//ref to SVLAN_TAGIF
	unsigned short stagPri:3;
	unsigned short stagDei:1;
	unsigned short ctagVid:12;			//ref to CVLAN_TAGIF
	unsigned short ctagPri:3;
	unsigned short ctagCfi:1;
	unsigned int ipv4Sip;
	unsigned char ipv6Sip[16];
	unsigned int ipv4Dip;
	unsigned char ipv6Dip[16];
	char ipProtocol;
	unsigned int sport;
	unsigned int dport;
	char ipv4_tos;						//ref to IPV4_TAGIF
	char ipv6_tc;						//ref to IPV6_TAGIF
	unsigned int ipv6FlowLebal;		//ref to IPV6_TAGIF
	int wlan_dev_idx;
	int stream_id;
	int intf_idx;
}rt_edp_aclCheck_data_t;
#endif	//EDP_ACL_SUPPORT_HOOK_CHECK

typedef struct rt_edp_aclFilterEntry_s
{
	//HW ACL information
#if defined(CONFIG_RG_RTL9607C_SERIES)
	uint32 hw_aclEntry_start;
	uint32 hw_aclEntry_size;
	uint32 hw_cfEntry_start;
	uint32 hw_cfEntry_size;
	rtk_rg_aclSWEntry_used_tables_field_t hw_used_table;//record which range tables are used
	uint8 hw_used_table_index[USED_TABLE_END];//record  used range tables index
#elif defined(CONFIG_RG_G3_SERIES)
	uint32 hw_aclEntry_count;
	rtk_portmask_t hw_aclEntry_port;
	unsigned char hw_aclEntry_index[128];//use string to record the hw acl related index
#endif

	//Linux ACL information
	rt_edp_aclSWEntry_linux_used_tables_field_t linux_used_table;//record which linux tables are used
	uint8 linux_used_table_index[RT_EDP_ACL_USED_TABLE_END][2];//record linux used tables index in [0] and count in [1]
	rt_edp_aclSWEntry_internal_check_field_t tables_internal_check;
	rt_edp_aclSWEntry_internal_check_chain_t ptr_nsup_chain;
	rt_edp_aclSWEntry_internal_check_table_t ptr_nsup_table;
	rt_edp_aclSWEntry_internal_check_chain_t act_nsup_chain;
	rt_edp_aclSWEntry_internal_check_table_t act_nsup_table;

	//RG ACL information
//1 ===== FIXME: To prevent the data structure of parameter of RG API is changed, use orginal RG data structure temporarily. =====
	rtk_rg_aclFilterAndQos_t acl_filter;	//FIXME: rt_edp_aclFilterAndQos_t
//1 ===== end FIXME =====
	rt_edp_enable_t valid;
}rt_edp_aclFilterEntry_t;
//+++++++++++++++ ACL Related Structure End +++++++++++++++

typedef struct rt_edp_system_s
{
	rt_edp_initParams_t initParam;
	unsigned int wanIntfTotalNum;
	unsigned int lanIntfTotalNum;
	rt_edp_interface_info_global_t interfaceInfo[EDP_MAX_NETIF_SW_TABLE_SIZE];		//store each interface information, LAN or WAN
	rt_edp_lan_interface_group_info_t lanIntfGroup[EDP_MAX_NETIF_SW_TABLE_SIZE];
	rt_edp_wan_interface_group_info_t wanIntfGroup[EDP_MAX_NETIF_SW_TABLE_SIZE];
	rt_edp_portmask_t wanPortMask, lanPortMask;
	int wanDevUsed[RT_EDP_WAN_DEV_TYPE_END][EDP_MAX_NETIF_SW_TABLE_SIZE]; //record nas0_x/pppx is uesd or not, e.g., wanDevUsed[RT_EDP_WAN_DEV_TYPE_NAS][0]=1 means nas0_0 is used. wanDevUsed[RT_EDP_WAN_DEV_TYPE_PPP][0]=1 means ppp0 is used

	//VLAN
	int portBasedVID[RTK_RG_PORT_MAX];

	//maintain ACL in linux
	int acl_SW_table_entry_size; //using for speed up RG_ACL which compared in rt edp.
	int acl_SWindex_sorting_by_weight[MAX_ACL_SW_ENTRY_SIZE];	//The array record the RG_ACL rule index which sorted by weight. The ACL rule priority should handled by this sorting result.
	rt_edp_aclFilterEntry_t acl_SW_table_entry[MAX_ACL_SW_ENTRY_SIZE]; //record all ACL informations(including RG_ACL & HW_ACL), the array index is the RG_ACL index
	uint32 stop_add_hw_acl;//0:it can try to add drop rule to HW ACL,   1:stop rest drop rule add to HW ACL.   use for apolloPro to determind add rest ACL drop rule to HW or not.
	uint8 stop_add_acl;//This parameter only check during add new rule. 0:it can continue to add acl, 1:stop add this rule due to rearrange fail

	int mcast_query_sec;


//1 ===== FIXME: To prevent the data structure of parameter of RG API is changed, use orginal RG data structure temporarily. =====
	rtk_rg_aclFilterAndQos_t acl_filter_temp[MAX_ACL_SW_ENTRY_SIZE]; //for acl del entry using
	rtk_rg_aclFilterAndQos_t empty_aclFilter;
//1 ===== end FIXME =====
	uint32 acl_rg_add_parameter_dump;	//debug tool for dump RG ACL parameter when calling add
	rt_edp_aclFilterEntry_t aclSWEntry,aclSWEntry_temp,empty_aclSWEntry;
	unsigned char acl_cmd_buff[EDP_CB_CMD_BUFF_SIZE];

} rt_edp_system_t;

typedef struct rt_edp_globalDatabase_s
{
	rt_edp_system_t systemGlobal;
	rt_edp_dev_port_mapping_t dev_port_map[RT_EDP_MAC_PORT_MAX];
	char wan_intf_mark[RT_EDP_WAN_INTF_MARK_SIZE][EDP_CB_CMD_BUFF_SIZE];
	rt_edp_table_mcapi_t mcApiMapping[EDP_MAX_MC_API_CONFIG_NUM];
	rt_edp_table_vlan_t	vlan[EDP_MAX_VLAN_HW_TABLE_SIZE];
	rt_edp_table_bind_t bind[EDP_MAX_BIND_SW_TABLE_SIZE];
	rtk_rg_dmzInfo_t dmzInfo[EDP_MAX_DMZ_TABLE_SIZE];
	rtk_rg_virtualServer_t virtualServer[EDP_MAX_VIRTUAL_SERVER_SW_TABLE_SIZE];
	rtk_rg_alg_type_t algServInLanIpMask;
	rtk_rg_alg_serverIpMapping_t algServInLanIpMapping[EDP_MAX_ALG_SERV_IN_LAN_NUM];
	rtk_rg_alg_type_t algFunctionMask;

}rt_edp_globalDatabase_t;

typedef enum rt_edp_debug_level_e
{
	RT_EDP_DEBUG_LEVEL_WARN=0x1,
	RT_EDP_DEBUG_LEVEL_IGMP=0x2,
	RT_EDP_DEBUG_LEVEL_DEBUG=0x4,
	RT_EDP_DEBUG_LEVEL_CMD=0x8,
	RT_EDP_DEBUG_LEVEL_ACL=0x10,
	RT_EDP_DEBUG_LEVEL_ACL_CONTROL_PATH=0x40,
	RT_EDP_DEBUG_LEVEL_TRACE_PREROUTING=0x100,
	RT_EDP_DEBUG_LEVEL_TRACE_FORWARD=0x200,
	RT_EDP_DEBUG_LEVEL_TRACE_POSTROUTING=0x400,
	RT_EDP_DEBUG_LEVEL_TRACE_LOCAL_IN=0x800,
	RT_EDP_DEBUG_LEVEL_TRACE_LOCAL_OUT=0x1000,
	RT_EDP_DEBUG_LEVEL_TRACE_PREROUTING_DUMP=0x10000,
	RT_EDP_DEBUG_LEVEL_TRACE_FORWARD_DUMP=0x20000,
	RT_EDP_DEBUG_LEVEL_TRACE_POSTROUTING_DUMP=0x40000,
	RT_EDP_DEBUG_LEVEL_TRACE_LOCAL_IN_DUMP=0x80000,
	RT_EDP_DEBUG_LEVEL_TRACE_LOCAL_OUT_DUMP=0x100000,
	RT_EDP_DEBUG_LEVEL_ALL=0xffffffff,
} rt_edp_debug_level_t;

typedef struct rt_edp_globalKernel_s
{
	rt_edp_debug_level_t debug_level;
	rt_edp_debug_level_t filter_level;	//used to filter display message based on trace_filter
}rt_edp_globalKernel_t;

extern rt_edp_globalDatabase_t	rt_edp_db;
extern rt_edp_globalKernel_t rt_edp_kernel;
/* End of Tables ========================================================== */

#if 0	//1 To prevent the data structure of parameter of RG API is changed, use orginal RG data structure temporarily.
typedef struct rt_edp_port_isolation_s
{
	rt_edp_port_idx_t port;
	rt_edp_portmask_t portmask;
}rt_edp_port_isolation_t;

typedef struct rt_edp_macEntry_s
{
	rtk_mac_t mac;
	uint32 isIVL:1; 	//0:SVL, 1:IVL
	uint32 fid; 		//only used in SVL
	int vlan_id; 		//egress to this MAC, add a CVLAN tag. (vlan_id=0, untag)
	rt_edp_port_idx_t port_idx;
	uint32 arp_used:1;
	uint32 static_entry:1;
	uint32 sa_block:1;
	uint32 da_block:1;
	uint32 fix_l34_vlan:1;
	uint32 auth:1;
	uint32 ctag_if:1;	//0:untag, 1:tagged
	uint32 idleSecs;
	int8 wlan_device_idx;	//used only when port is CPU and extport is 1 or 2  (G3 platform: -2 stands for lan interface, -3 stands for wan interface)
}rt_edp_macEntry_t;

typedef struct rt_edp_arpEntry_s
{
	int macEntryIdx;
	ipaddr_t ipv4Addr;
	uint32 staticEntry:1;
}rt_edp_arpEntry_t;

typedef enum rt_edp_lanNet_device_type_e
{
	RT_EDP_LANNET_TYPE_OTHER,
	RT_EDP_LANNET_TYPE_PHONE,
	RT_EDP_LANNET_TYPE_COMPUTER,
}rt_edp_lanNet_device_type_t;

typedef enum rt_edp_lanNet_brand_e
{
	RT_EDP_BRAND_OTHER=0,
	RT_EDP_BRAND_HUAWEI,
	RT_EDP_BRAND_XIAOMI,
	RT_EDP_BRAND_MEIZU,
	RT_EDP_BRAND_IPHONE,
	RT_EDP_BRAND_NOKIA,
	RT_EDP_BRAND_SAMSUNG,
	RT_EDP_BRAND_SONY,
	RT_EDP_BRAND_ERICSSON,
	RT_EDP_BRAND_MOT,
	RT_EDP_BRAND_HTC,
	RT_EDP_BRAND_SGH,
	RT_EDP_BRAND_LG,
	RT_EDP_BRAND_SHARP,
	RT_EDP_BRAND_PHILIPS,
	RT_EDP_BRAND_PANASONIC,
	RT_EDP_BRAND_ALCATEL,
	RT_EDP_BARND_LENOVO,
	RT_EDP_BARND_OPPO,
	RT_EDP_BRAND_END,
}rt_edp_lanNet_brand_t;

typedef enum rt_edp_lanNet_model_e
{
	RT_EDP_MODEL_OTHER=0,
	RT_EDP_MODEL_HONOR,
	RT_EDP_MODEL_P7,
	RT_EDP_MODEL_R7,
	RT_EDP_MODEL_MI4LTE,
	RT_EDP_MODEL_END,
}rt_edp_lanNet_model_t;

typedef enum rt_edp_lanNet_os_e
{
	RT_EDP_OS_OTHER=0,
	RT_EDP_OS_WINDOWS_NT,
	RT_EDP_OS_MACINTOSH,
	RT_EDP_OS_IOS,
	RT_EDP_OS_ANDROID,
	RT_EDP_OS_WINDOWS_PHONE,
	RT_EDP_OS_LINUX,
	RT_EDP_OS_END,
}rt_edp_lanNet_os_t;

typedef enum rt_edp_lanNet_connect_type_e
{
	RT_EDP_CONN_MAC_PORT=0,
	RT_EDP_CONN_WIFI,
}rt_edp_lanNet_connect_type_t;

typedef struct rt_edp_lanNetInfo_s
{
	//connection type, Dev Name,  brand, OS, device type
	char dev_name[EDP_MAX_LANNET_DEV_NAME_LENGTH];	//for LANNetInfo
	rt_edp_lanNet_device_type_t dev_type;
	rt_edp_lanNet_brand_t brand;
	rt_edp_lanNet_model_t model;
	rt_edp_lanNet_os_t os;
	rt_edp_lanNet_connect_type_t conn_type;		//for LANNetInfo
	unsigned int checked;	//at most check MAX_LANNET_PARSE_COUNT times
}rt_edp_lanNetInfo_t;

typedef struct rt_edp_arpInfo_s
{
	rt_edp_arpEntry_t arpEntry;
	int valid;
	int idleSecs;
	//lan device info
	rt_edp_lanNetInfo_t lanNetInfo;
	char brandStr[EDP_MAX_LANNET_BRAND_NAME_LENGTH];
	char modelStr[EDP_MAX_LANNET_MODEL_NAME_LENGTH];
	char osStr[EDP_MAX_LANNET_OS_NAME_LENGTH];
}rt_edp_arpInfo_t;

typedef struct rt_edp_neighborEntry_s
{
	uint32 l2Idx;
	uint8 matchRouteIdx;
	uint8 interfaceId[IPV6_ADDR_LEN];
	uint8 valid:1;
	uint8 staticEntry:1;
}rt_edp_neighborEntry_t;

typedef struct rt_edp_neighborInfo_s
{
	rt_edp_neighborEntry_t neighborEntry;
	int idleSecs;
}rt_edp_neighborInfo_t;

typedef rt_edp_neighborInfo_t rt_edp_table_v6neighbor_t;

typedef enum rt_edp_portMirrorInfo_direction_e
{
	RT_EDP_MIRROR_TX_RX_BOTH=0,
	RT_EDP_MIRROR_RX_ONLY,
	RT_EDP_MIRROR_TX_ONLY,
	RT_EDP_MIRROR_END
} rt_edp_portMirrorInfo_direction_t;

typedef struct rt_edp_portMirrorInfo_s
{
	uint32 monitorPort;
	rt_edp_mac_portmask_t enabledPortMask;
	rt_edp_portMirrorInfo_direction_t direct;
}rt_edp_portMirrorInfo_t;

typedef enum rt_edp_port_speed_e
{
    RT_EDP_PORT_SPEED_10M = 0,
    RT_EDP_PORT_SPEED_100M,
    RT_EDP_PORT_SPEED_1000M,
    RT_EDP_PORT_SPEED_END,
} rt_edp_port_speed_t;
typedef enum rt_edp_port_duplex_e
{
    RT_EDP_PORT_HALF_DUPLEX = 0,
    RT_EDP_PORT_FULL_DUPLEX,
    RT_EDP_PORT_DUPLEX_END
} rt_edp_port_duplex_t;
typedef enum rt_edp_port_linkStatus_e
{
    RT_EDP_PORT_LINKDOWN = 0,
    RT_EDP_PORT_LINKUP,
    RT_EDP_PORT_LINKSTATUS_END
} rt_edp_port_linkStatus_t;

typedef struct rt_edp_phyPortAbilityInfo_s
{
	rt_edp_enable_t			force_disable_phy;
	rt_edp_enable_t 		valid;
	rt_edp_port_speed_t		speed;
	rt_edp_port_duplex_t	duplex;
	rt_edp_enable_t			flowCtrl;
	rt_edp_enable_t			fc; //valid when flowCtrl is enabled
	rt_edp_enable_t			asym_fc;// valid when flowCtrl is enabled
}rt_edp_phyPortAbilityInfo_t;

typedef struct rt_edp_portStatusInfo_s
{
	rt_edp_port_linkStatus_t linkStatus;
	rt_edp_port_speed_t		linkSpeed;
	rt_edp_port_duplex_t	linkDuplex;
}rt_edp_portStatusInfo_t;

typedef enum rt_edp_storm_type_e
{
    RT_EDP_STORM_TYPE_UNKNOWN_UNICAST = 0,
    RT_EDP_STORM_TYPE_UNKNOWN_MULTICAST,
    RT_EDP_STORM_TYPE_MULTICAST,
    RT_EDP_STORM_TYPE_BROADCAST,
    RT_EDP_STORM_TYPE_DHCP,
    RT_EDP_STORM_TYPE_ARP,
    RT_EDP_STORM_TYPE_IGMP_MLD,
    RT_EDP_STORM_TYPE_END
} rt_edp_storm_type_t;

typedef struct rt_edp_stormControlInfo_s
{
	rt_edp_enable_t		valid;
	rt_edp_port_idx_t	port;
	rt_edp_storm_type_t stormType;
	uint32 				meterIdx;
}rt_edp_stormControlInfo_t;

typedef struct rt_edp_qos_queue_weights_s
{
    uint32 weights[EDP_MAX_NUM_OF_QUEUE];
} rt_edp_qos_queue_weights_t;

typedef enum rt_edp_naptInType_e
{
	RT_EDP_NAPT_IN_TYPE_SYMMETRIC_NAPT=0,
	RT_EDP_NAPT_IN_TYPE_RESTRICTED_CONE=1,
	RT_EDP_NAPT_IN_TYPE_FULL_CONE=2,
	RT_EDP_NAPT_IN_TYPE_END,
} rt_edp_naptInType_t;

typedef struct rt_edp_naptEntry_s
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
	rt_edp_naptInType_t coneType;	//default 0 as symmetric cone type
} rt_edp_naptEntry_t;

typedef enum rt_edp_naptState_e
{
	RT_EDP_INVALID			=0,
	RT_EDP_SYN_RECV			=1,
	RT_EDP_UDP_FIRST		=2,
	RT_EDP_SYN_ACK_RECV 	=3,
	RT_EDP_UDP_SECOND		=4,
	RT_EDP_TCP_CONNECTED	=5,
	RT_EDP_UDP_CONNECTED	=6,
	RT_EDP_FIRST_FIN		=7,
	RT_EDP_RST_RECV			=8,
	RT_EDP_FIN_SEND_AND_RECV	=9,
	RT_EDP_LAST_ACK			=10,
	RT_EDP_FIRST_FIN_IN		=11,
	RT_EDP_FIRST_FIN_OUT	=12,
} rt_edp_naptState_t;

typedef struct rt_edp_naptInfo_s
{
	rt_edp_naptEntry_t naptTuples;
	uint32	idleSecs;
	rt_edp_naptState_t	state;
	void **pContext;
} rt_edp_naptInfo_t;

typedef enum rt_edp_idle_time_type_e
{
	RT_EDP_IDLETIME_TYPE_ARP=0,
	RT_EDP_IDLETIME_TYPE_FLOW,
	RT_EDP_IDLETIME_TYPE_FRAGMENT,
	RT_EDP_IDLETIME_TYPE_ICMP,
	RT_EDP_IDLETIME_TYPE_IPV4_SHORTCUT,
	RT_EDP_IDLETIME_TYPE_IPV6_SHORTCUT,
	RT_EDP_IDLETIME_TYPE_LUT,
	RT_EDP_IDLETIME_TYPE_NEIGHBOR,
	RT_EDP_IDLETIME_TYPE_TCP_LONG,
	RT_EDP_IDLETIME_TYPE_TCP_SHORT,
	RT_EDP_IDLETIME_TYPE_TCP_SHORT_HOUSEKEEP_JIFFIES,
	RT_EDP_IDLETIME_TYPE_UDP_LONG,
	RT_EDP_IDLETIME_TYPE_UDP_SHORT,
}rt_edp_idle_time_type_t;

//VLAN function
typedef struct rt_edp_cvlan_info_s
{
	int vlanId;
	unsigned int isIVL:1;		//0: SVL, 1:IVL
	rt_edp_portmask_t memberPortMask;
	rt_edp_mac_portmask_t untagPortMask;
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	unsigned int wlan0DevMask;
	unsigned int wlan0UntagMask;
#endif
	rt_edp_enable_t vlan_based_pri_enable;
	int vlan_based_pri;
	int addedAsCustomerVLAN;		//used to check this vlan created by cvlan apis or not
}rt_edp_cvlan_info_t;

//VLAN Binding
typedef struct rt_edp_vlanBinding_s
{
	rt_edp_port_idx_t port_idx;
	int ingress_vid;
	int wan_intf_idx;
}rt_edp_vlanBinding_t;

typedef enum rt_edp_ipv4MulticastFlowMode_e
{
	RT_EDP_IPV4MC_DONT_CARE_SRC=0,
	RT_EDP_IPV4MC_INCLUDE,
	RT_EDP_IPV4MC_EXCLUDE,
} rt_edp_ipv4MulticastFlowMode_t;

typedef enum rt_edp_ipv4MulticastFlowRouting_e
{
	RT_EDP_IPV4MC_EN_ROUTING=0,
	RT_EDP_IPV4MC_DIS_ROUTING,
} rt_edp_ipv4MulticastFlowRouting_t;

//Multicast flow
typedef struct rt_edp_multicastFlow_s
{
	ipaddr_t 	multicast_ipv4_addr;
	ipaddr_t	multicast_ipv6_addr[4];
	unsigned int isIPv6:1;
	rt_edp_ipv4MulticastFlowMode_t srcFilterMode;		//for 9602C/9600series
	rt_edp_ipv4MulticastFlowRouting_t routingMode;
	rt_edp_portmask_t port_mask;
	ipaddr_t includeOrExcludeIp; //for v4 only			//for 9602C/9600series
	unsigned int isIVL:1;
	unsigned short vlanID;	//used for IVL only 		//for 9607Series is internalVid
} rt_edp_multicastFlow_t;


typedef struct rt_edp_l2MulticastFlow_s
{
	rtk_mac_t mac;
	rt_edp_portmask_t port_mask;
	unsigned int isIVL:1;
	unsigned short vlanID;	//used for IVL only
} rt_edp_l2MulticastFlow_t;

typedef struct rt_edp_igmpMldSnoopingPortControl_s
{
	uint32 enableIGMPSnooping:1;
	uint32 enableMLDSnooping:1;
	uint32 enablefastLeave:1;
	uint16 igmp_PortMaxGroupSize;
	uint16 mld_PortMaxGroupSize;
}rt_edp_igmpMldSnoopingPortControl_t;

typedef struct rt_edp_igmpMldSnoopingControl_s
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

}rt_edp_igmpMldSnoopingControl_t;

typedef enum rt_edp_alg_type_e
{
	//Server in WAN
	RT_EDP_ALG_SIP_TCP_BIT					=0x1,
	RT_EDP_ALG_SIP_UDP_BIT					=0x2,
	RT_EDP_ALG_H323_TCP_BIT					=0x4,
	RT_EDP_ALG_H323_UDP_BIT					=0x8,
	RT_EDP_ALG_RTSP_TCP_BIT					=0x10,
	RT_EDP_ALG_RTSP_UDP_BIT					=0x20,
	RT_EDP_ALG_FTP_TCP_BIT					=0x40,
	RT_EDP_ALG_FTP_UDP_BIT					=0x80,			//8

	//Server in LAN
	RT_EDP_ALG_SIP_TCP_SRV_IN_LAN_BIT		=0x100,
	RT_EDP_ALG_SIP_UDP_SRV_IN_LAN_BIT		=0x200,
	RT_EDP_ALG_H323_TCP_SRV_IN_LAN_BIT		=0x400,
	RT_EDP_ALG_H323_UDP_SRV_IN_LAN_BIT		=0x800,
	RT_EDP_ALG_RTSP_TCP_SRV_IN_LAN_BIT		=0x1000,
	RT_EDP_ALG_RTSP_UDP_SRV_IN_LAN_BIT		=0x2000,
	RT_EDP_ALG_FTP_TCP_SRV_IN_LAN_BIT		=0x4000,
	RT_EDP_ALG_FTP_UDP_SRV_IN_LAN_BIT		=0x8000,		//16

	//Pass through
	RT_EDP_ALG_PPTP_TCP_PASSTHROUGH_BIT		=0x10000,
	RT_EDP_ALG_PPTP_UDP_PASSTHROUGH_BIT		=0x20000,
	RT_EDP_ALG_L2TP_TCP_PASSTHROUGH_BIT		=0x40000,
	RT_EDP_ALG_L2TP_UDP_PASSTHROUGH_BIT		=0x80000,
	RT_EDP_ALG_IPSEC_TCP_PASSTHROUGH_BIT	=0x100000,
	RT_EDP_ALG_IPSEC_UDP_PASSTHROUGH_BIT	=0x200000,		//22

	RT_EDP_ALG_PPPOE_PASSTHROUGH_BIT		=0x400000,

	//BattleNet
#ifdef CONFIG_RG_ROMEDRIVER_ALG_BATTLENET_SUPPORT
	RT_EDP_ALG_BATTLENET_TCP_BIT 			=0x800000,
#endif
	//TFTP
	RT_EDP_ALG_TFTP_UDP_BIT					=0x1000000,
	RT_EDP_ALG_TYPE_END						=RT_EDP_ALG_TFTP_UDP_BIT<<1,
}rt_edp_alg_type_t;

typedef struct rt_edp_alg_serverIpMapping_s
{
	rt_edp_alg_type_t algType;
	ipaddr_t serverAddress;
}rt_edp_alg_serverIpMapping_t;

typedef enum  rt_edp_virtualServerMappingType_e
{
	RT_EDP_VS_MAPPING_N_TO_N	=0,
	RT_EDP_VS_MAPPING_N_TO_1	=1,
}rt_edp_virtualServerMappingType_t;

//VirtualServer(PortForward)
typedef struct rt_edp_virtualServer_s
{
	rt_edp_ip_version_t ipversion; 	//0:v4_only 1:v6_only 2:both_v4_v6
	unsigned int is_tcp:1;
	int wan_intf_idx;	 			//for gateway ip
	unsigned short int gateway_port_start;
	ipaddr_t local_ip;				//only used when (ipversion==0 || ipversion==2)
	ipaddr_t remote_ip;
	rtk_ipv6_addr_t	local_ipv6;		//only used when (ipversion==1 || ipversion==2)
	unsigned short int local_port_start;
	unsigned int mappingPortRangeCnt;
	rt_edp_virtualServerMappingType_t mappingType;
	unsigned int valid:1;
	rt_edp_alg_type_t hookAlgType;	//only for server-in-lan
	unsigned int disable_wan_check;	//0: turn-on wan interface check, 1: disable wan interface check
	int8 enable_limit_remote_src_port;
	uint16 remote_src_port_start;
	uint16 remote_src_port_end;
} rt_edp_virtualServer_t;

typedef enum rt_edp_gpon_ds_bc_tag_decision_e
{
	RT_EDP_GPON_BC_FORCE_UNATG=0,
	RT_EDP_GPON_BC_FORCE_TAGGIN_WITH_CVID,
	RT_EDP_GPON_BC_FORCE_TAGGIN_WITH_CVID_CPRI,
	RT_EDP_GPON_BC_FORCE_TRANSPARENT,
	RT_EDP_GPON_BC_FORCE_END,
}rt_edp_gpon_ds_bc_tag_decision_t;

typedef struct rt_edp_gpon_ds_bc_action_s
{
	rt_edp_gpon_ds_bc_tag_decision_t ctag_decision;
	uint16 assigned_ctag_cvid;
	uint16 assigned_ctag_cpri;//valid only when ctag_decision is RT_EDP_GPON_BC_FORCE_TAGGIN_WITH_CVID_CPRI
}rt_edp_gpon_ds_bc_action_t;

typedef struct rt_edp_gpon_ds_bc_vlanfilterAndRemarking_s
{
	//care patterns
	unsigned int filter_fields; // please refer to rt_edp_gpon_ds_bc_filter_fields_t typedef.

	//assign value for care patterns
	uint16 ingress_stream_id;
	uint8 ingress_stagIf;
	uint8 ingress_ctagIf;
	uint16 ingress_stag_svid;
	uint16 ingress_ctag_cvid;
	rt_edp_portmask_t egress_portmask;

	//action: force assign ctag
	rt_edp_gpon_ds_bc_action_t ctag_action;
}rt_edp_gpon_ds_bc_vlanfilterAndRemarking_t;

typedef enum rt_edp_acl_fwding_type_direction_e{
	RT_EDP_ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET=0,
	RT_EDP_ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP,
	RT_EDP_ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP,
	RT_EDP_ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN,
	RT_EDP_ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN,
	RT_EDP_ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP, //apolloFE
	RT_EDP_ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP, //apolloFE
	RT_EDP_ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT, //apolloFE
	RT_EDP_ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT, //apolloFE
}rt_edp_acl_fwding_type_direction_t;

typedef enum rt_edp_acl_action_type_e
{
	//all platform supported
	RT_EDP_ACL_ACTION_TYPE_DROP=0,	// 0
	RT_EDP_ACL_ACTION_TYPE_PERMIT,
	RT_EDP_ACL_ACTION_TYPE_TRAP,
	RT_EDP_ACL_ACTION_TYPE_QOS,	//action of stream_id, CVLAN, SVLAN also belong to this type
	RT_EDP_ACL_ACTION_TYPE_TRAP_TO_PS,
	RT_EDP_ACL_ACTION_TYPE_POLICY_ROUTE,		//sw only

	//apollo platform ony
	RT_EDP_ACL_ACTION_TYPE_SW_PERMIT,	//6: sw only, and apollo only

	//apolloFE platform only, none for now.

	//apolloPro platform only
	RT_EDP_ACL_ACTION_TYPE_SW_MIRROR_WITH_UDP_ENCAP, //7: sw only , and 9607C only
	RT_EDP_ACL_ACTION_TYPE_FLOW_MIB,					//8:  9607C_MP only

	//apolloPro
	RT_EDP_ACL_ACTION_TYPE_TRAP_WITH_PRIORITY,

	RT_EDP_ACL_ACTION_TYPE_END
} rt_edp_acl_action_type_t;

#endif
#if defined(EDP_ACL_SUPPORT_HOOK_CHECK) && (EDP_ACL_SUPPORT_HOOK_CHECK==1)
typedef enum rt_edp_acl_filter_and_qos_action_e
{
	RT_EDP_ACL_ACTION_NOP_BIT=(1<<0),
	RT_EDP_ACL_ACTION_1P_REMARKING_BIT=(1<<1),
	RT_EDP_ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT=(1<<2),
	RT_EDP_ACL_ACTION_DSCP_REMARKING_BIT=(1<<3),
	RT_EDP_ACL_ACTION_QUEUE_ID_BIT=(1<<4),
	RT_EDP_ACL_ACTION_SHARE_METER_BIT=(1<<5),
	RT_EDP_ACL_ACTION_STREAM_ID_OR_LLID_BIT=(1<<6),
	RT_EDP_ACL_ACTION_ACL_PRIORITY_BIT=(1<<7),
	RT_EDP_ACL_ACTION_ACL_CVLANTAG_BIT=(1<<8),
	RT_EDP_ACL_ACTION_ACL_SVLANTAG_BIT=(1<<9),
	RT_EDP_ACL_ACTION_ACL_INGRESS_VID_BIT=(1<<10),
	RT_EDP_ACL_ACTION_DS_UNIMASK_BIT	= (1<<11),		//only support in ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN(type 4)
	RT_EDP_ACL_ACTION_REDIRECT_BIT = (1<<12), //only support in ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET(type 0)
	RT_EDP_ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT = (1<<13), //only support in ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_XXXX(type1~4)
	RT_EDP_ACL_ACTION_TOS_TC_REMARKING_BIT=(1<<14),
	RT_EDP_ACL_ACTION_LOG_COUNTER_BIT=(1<<15),
#if defined(CONFIG_CMCC)||defined(CONFIG_CU_BASEON_CMCC)
	RT_EDP_ACL_ACTION_CF_LOG_COUNTER_BIT=(1<<16),
#endif
	RT_EDP_ACL_ACTION_QOS_END=(1<<17),
} rt_edp_acl_qos_action_t;

typedef enum rt_edp_acl_cvlan_tagif_decision_e{
	//The new action should alway put on later
	RT_EDP_ACL_CVLAN_TAGIF_NOP,
	RT_EDP_ACL_CVLAN_TAGIF_TAGGING,
	RT_EDP_ACL_CVLAN_TAGIF_TAGGING_WITH_C2S, //apollo only
	RT_EDP_ACL_CVLAN_TAGIF_TAGGING_WITH_SP2C, //apollo only
	RT_EDP_ACL_CVLAN_TAGIF_UNTAG,
	RT_EDP_ACL_CVLAN_TAGIF_TRANSPARENT,
	RT_EDP_ACL_CVLAN_TAGIF_END,
}rt_edp_acl_cvlan_tagif_decision_t;

typedef enum rt_edp_acl_cvlan_cvid_decision_e{
	//The new action should alway put on later
	RT_EDP_ACL_CVLAN_CVID_ASSIGN,
	RT_EDP_ACL_CVLAN_CVID_COPY_FROM_1ST_TAG,
	RT_EDP_ACL_CVLAN_CVID_COPY_FROM_2ND_TAG,
	RT_EDP_ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID, //(upstream only)
	RT_EDP_ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID, //(downstream only)
	RT_EDP_ACL_CVLAN_CVID_NOP, //apolloFE  (downstream only)
	RT_EDP_ACL_CVLAN_CVID_CPOY_FROM_SP2C, //apolloFE
	RT_EDP_ACL_CVLAN_CVID_END,
}rt_edp_acl_cvlan_cvid_decision_t;

typedef enum rt_edp_acl_cvlan_cpri_decision_e{
	//The new action should alway put on later
	RT_EDP_ACL_CVLAN_CPRI_ASSIGN,
	RT_EDP_ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG,
	RT_EDP_ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG,
	RT_EDP_ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI,
	RT_EDP_ACL_CVLAN_CPRI_NOP,//apolloFE
	RT_EDP_ACL_CVLAN_CPRI_COPY_FROM_DSCP_REMAP,//apolloFE
	RT_EDP_ACL_CVLAN_CPRI_COPY_FROM_SP2C, //apolloFE (downstream only)
	RT_EDP_ACL_CVLAN_CPRI_END,
}rt_edp_acl_cvlan_cpri_decision_t;

typedef enum rt_edp_acl_svlan_tagif_decision_e{
	//The new action should alway put on later
	RT_EDP_ACL_SVLAN_TAGIF_NOP,
	RT_EDP_ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID,
	RT_EDP_ACL_SVLAN_TAGIF_TAGGING_WITH_8100, //apollo
	RT_EDP_ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C, //apollo (downstream only)
	RT_EDP_ACL_SVLAN_TAGIF_UNTAG,
	RT_EDP_ACL_SVLAN_TAGIF_TRANSPARENT,
	RT_EDP_ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2, //apolloFE
	RT_EDP_ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID, //apolloFE
	RT_EDP_ACL_SVLAN_TAGIF_END,
}rt_edp_acl_svlan_tagif_decision_t;

typedef enum rt_edp_acl_svlan_svid_decision_e{
	//The new action should alway put on later
	RT_EDP_ACL_SVLAN_SVID_ASSIGN,
	RT_EDP_ACL_SVLAN_SVID_COPY_FROM_1ST_TAG,
	RT_EDP_ACL_SVLAN_SVID_COPY_FROM_2ND_TAG,
	RT_EDP_ACL_SVLAN_SVID_NOP, //apolloFE
	RT_EDP_ACL_SVLAN_SVID_SP2C, //apolloFE
	RT_EDP_ACL_SVLAN_SVID_END,
}rt_edp_acl_svlan_svid_decision_t;

typedef enum rt_edp_acl_svlan_spri_decision_e{
	//The new action should alway put on later
	RT_EDP_ACL_SVLAN_SPRI_ASSIGN,
	RT_EDP_ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG,
	RT_EDP_ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG,
	RT_EDP_ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI,
	RT_EDP_ACL_SVLAN_SPRI_NOP,//apolloFE
	RT_EDP_ACL_SVLAN_SPRI_COPY_FROM_DSCP_REMAP, //apolloFE (downstream only)
	RT_EDP_ACL_SVLAN_SPRI_COPY_FROM_SP2C, //apolloFE (downstream only)
	RT_EDP_ACL_SVLAN_SPRI_END,
}rt_edp_acl_svlan_spri_decision_t;

typedef struct rt_edp_cvlan_tag_action_s{
	rt_edp_acl_cvlan_tagif_decision_t 	cvlanTagIfDecision;
	rt_edp_acl_cvlan_cvid_decision_t	cvlanCvidDecision;
	rt_edp_acl_cvlan_cpri_decision_t	cvlanCpriDecision;
	uint32 assignedCvid;
	uint8 assignedCpri;
}rt_edp_cvlan_tag_action_t;

typedef struct rt_edp_svlan_tag_action_s{
	rt_edp_acl_svlan_tagif_decision_t 	svlanTagIfDecision;
	rt_edp_acl_svlan_svid_decision_t	svlanSvidDecision;
	rt_edp_acl_svlan_spri_decision_t	svlanSpriDecision;
	uint32 assignedSvid;
	uint8 assignedSpri;
}rt_edp_svlan_tag_action_t;
#endif	//EDP_ACL_SUPPORT_HOOK_CHECK
#if 0	//1 To prevent the data structure of parameter of RG API is changed, use orginal RG data structure temporarily.

typedef struct rt_edp_mirror_udp_encap_tag_action_s{
	rtk_mac_t encap_smac;
	rtk_mac_t encap_dmac;
	uint32 encap_Sip;
	uint32 encap_Dip;
	uint32 encap_Dport;
}rt_edp_mirror_udp_encap_tag_action_t;

typedef struct rt_edp_aclFilterAndQos_s
{
	//unsigned int filter_fields;
	unsigned long long int filter_fields; // please refer to rt_edp_acl_filter_fields_t typedef.
	unsigned long long int filter_fields_inverse; // please refer to rt_edp_acl_filter_fields_t typedef. supporting for inverse pattern compare result, 9607C only

	rt_edp_acl_fwding_type_direction_t fwding_type_and_direction;
	int acl_weight;	//the larger number, the higher priority.

	//pattern
	rt_edp_portmask_t ingress_port_mask;
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
	rt_edp_acl_action_type_t action_type;
	rt_edp_acl_qos_action_t qos_actions; /* only used for action_type=ACL_ACTION_TYPE_QOS */

	unsigned char action_dot1p_remarking_pri;
	unsigned char action_ip_precedence_remarking_pri;
	unsigned char action_dscp_remarking_pri;
	unsigned char action_tos_tc_remarking_pri;
	unsigned char action_queue_id;
	unsigned char action_share_meter;
	unsigned char action_log_counter;
	unsigned char action_stream_id_or_llid;
	unsigned char action_acl_priority;
	rt_edp_cvlan_tag_action_t action_acl_cvlan;
	rt_edp_svlan_tag_action_t action_acl_svlan;
	unsigned char action_policy_route_wan;
	int action_acl_ingress_vid;
	uint32 downstream_uni_portmask;
	uint32 redirect_portmask;
	uint32 egress_internal_priority;
	rt_edp_mirror_udp_encap_tag_action_t action_encap_udp;
	uint32 action_flowmib_counter_idx;
	uint32 action_trap_with_priority;
#if defined(CONFIG_CMCC)||defined(CONFIG_CU_BASEON_CMCC)
	unsigned char action_cf_log_counter;
#endif
} rt_edp_aclFilterAndQos_t;

typedef enum rt_edp_napt_filter_fields_e
{
	RT_EDP_INGRESS_SIP=(1<<0),
	RT_EDP_EGRESS_SIP=(1<<1),
	RT_EDP_INGRESS_DIP=(1<<2),
	RT_EDP_EGRESS_DIP=(1<<3),
	RT_EDP_INGRESS_SPORT=(1<<4),
	RT_EDP_EGRESS_SPORT=(1<<5),
	RT_EDP_INGRESS_DPORT=(1<<6),
	RT_EDP_EGRESS_DPORT=(1<<7),
	RT_EDP_L4_PROTOCAL=(1<<8),
	RT_EDP_INGRESS_SIP_RANGE=(1<<9),
	RT_EDP_INGRESS_DIP_RANGE=(1<<10),
	RT_EDP_INGRESS_SPORT_RANGE=(1<<11),
	RT_EDP_INGRESS_DPORT_RANGE=(1<<12),
	RT_EDP_EGRESS_SIP_RANGE=(1<<13),
	RT_EDP_EGRESS_DIP_RANGE=(1<<14),
	RT_EDP_EGRESS_SPORT_RANGE=(1<<15),
	RT_EDP_EGRESS_DPORT_RANGE=(1<<16),
	RT_EDP_INGRESS_SMAC = (1<<17),
	RT_EDP_EGRESS_DMAC = (1<<18),
}rt_edp_napt_filter_fields_t;

typedef enum rt_edp_napt_action_fields_e
{
	RT_EDP_ASSIGN_NAPT_PRIORITY_BIT=(1<<0),
	RT_EDP_NAPT_DROP_BIT=(1<<1),
	RT_EDP_NAPT_PERMIT_BIT=(1<<2),
	RT_EDP_NAPT_SW_RATE_LIMIT_BIT=(1<<3), //hit this action will always fowraed by SW(skip add H/W NAPT entry) and do SW rate limit. (if flowed by HWNAT, this action is useless)
	RT_EDP_NAPT_SW_TRAP_TO_PS=(1<<4),//hit this action will always fowraed by SW(skip add H/W NAPT entry) and do SW trap to PS. (if flowed by HWNAT, this action is useless)
	RT_EDP_NAPT_SW_PACKET_COUNT=(1<<5),//hit this action will always fowraed by SW(skip add H/W NAPT entry), and update the naptFilter.packet_count
	RT_EDP_NAPT_SW_BYTE_COUNT=(1<<5),//hit this action will always fowraed by SW(skip add H/W NAPT entry), and update the naptFilter.byte_count
}rt_edp_napt_action_fields_t;

typedef enum rt_edp_napt_fwding_direction_e{
	RT_EDP_NAPT_FILTER_OUTBOUND=0,
	RT_EDP_NAPT_FILTER_INBOUND,
	RT_EDP_NAPT_FILTER_DIRECTION_END,
}rt_edp_napt_fwding_direction_t;

typedef enum rt_edp_napt_fwding_ruleType_e{
	RT_EDP_NAPT_FILTER_PERSIST=0,
	RT_EDP_NAPT_FILTER_ONE_SHOT,
	RT_EDP_NAPT_FILTER_RULE_TYPE_END,
}rt_edp_napt_fwding_ruleType_t;

typedef struct rt_edp_naptFilterAndQos_s
{
	rt_edp_napt_fwding_direction_t direction;
	//compare patterns
	uint32 weight;
	rt_edp_napt_filter_fields_t filter_fields;
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

	rt_edp_napt_action_fields_t action_fields;
	//assigned priority
	uint32 assign_priority;

	//assgined limit rate
	uint32 assign_rate;

	//get the accumulate packet count that hit this rule
	uint32 packet_count;
	uint32 byte_count;

	rt_edp_napt_fwding_ruleType_t ruleType; //0:persist, 1:one shot

}rt_edp_naptFilterAndQos_t;

typedef enum rt_edp_cf_direction_type_e
{
	RT_EDP_CLASSIFY_DIRECTION_UPSTREAM,
	RT_EDP_CLASSIFY_DIRECTION_DOWNSTREAM,
	RT_EDP_CLASSIFY_DIRECTION_END,
} rt_edp_cf_direction_type_t;

typedef enum rt_edp_cf_filter_fields_e
{
	RT_EDP_EGRESS_ETHERTYPR_BIT=(1<<0),
	RT_EDP_EGRESS_GEMIDX_BIT=(1<<1),
	RT_EDP_EGRESS_LLID_BIT=(1<<2),
	RT_EDP_EGRESS_TAGVID_BIT=(1<<3),
	RT_EDP_EGRESS_TAGPRI_BIT=(1<<4),
	RT_EDP_EGRESS_INTERNALPRI_BIT=(1<<5),
	RT_EDP_EGRESS_STAGIF_BIT=(1<<6),
	RT_EDP_EGRESS_CTAGIF_BIT=(1<<7),
	RT_EDP_EGRESS_UNI_BIT=(1<<8),
} rt_edp_cf_filter_fields_t;

typedef enum rt_edp_cf_us_action_type_e
{
	RT_EDP_CF_US_ACTION_STAG_BIT = (1ULL<<0),
	RT_EDP_CF_US_ACTION_CTAG_BIT = (1ULL<<1),
	RT_EDP_CF_US_ACTION_CFPRI_BIT=(1ULL<<2),
	RT_EDP_CF_US_ACTION_DSCP_BIT=(1ULL<<3),//DSCP:apollo 9600Series have side effect!
	RT_EDP_CF_US_ACTION_SID_BIT=(1ULL<<4),
	RT_EDP_CF_US_ACTION_FWD_BIT=(1ULL<<5),//apolloFE change drop action to fwd action
	RT_EDP_CF_US_ACTION_DROP_BIT=(1ULL<<6),
	RT_EDP_CF_US_ACTION_LOG_BIT=(1ULL<<7),//LOG:apollo 9600Series have side effect!
	RT_EDP_CF_US_ACTION_END_BIT=(1ULL<<8),
} rt_edp_cf_us_action_type_t;


typedef enum rt_edp_cf_ds_action_type_e
{
	RT_EDP_CF_DS_ACTION_STAG_BIT = (1ULL<<0),
	RT_EDP_CF_DS_ACTION_CTAG_BIT = (1ULL<<1),
	RT_EDP_CF_DS_ACTION_CFPRI_BIT=(1ULL<<2),
	RT_EDP_CF_DS_ACTION_DSCP_BIT=(1ULL<<3),//DSCP:apollo 9600Series have side effect!
	RT_EDP_CF_DS_ACTION_UNI_MASK_BIT=(1ULL<<4),
	RT_EDP_CF_DS_ACTION_DROP_BIT=(1ULL<<5),//DROP: using uni to achieve this action!
	RT_EDP_CF_DS_ACTION_END_BIT=(1ULL<<6),
} rt_edp_cf_ds_action_type_t;

typedef enum rt_edp_cf_pattern_type_e{
	RT_EDP_CF_PATTERN_ETHERTYPE=0,
	RT_EDP_CF_PATTERN_GEMIDX_OR_LLID,
	RT_EDP_CF_PATTERN_OUTTERTAGVID,
	RT_EDP_CF_PATTERN_OUTTERTAGPRI,
	RT_EDP_CF_PATTERN_INTERNALPRI,
	RT_EDP_CF_PATTERN_STAGFLAG,
	RT_EDP_CF_PATTERN_CTAGFLAG,
	RT_EDP_CF_PATTERN_UNI,
	RT_EDP_CF_PATTERN_END,
}rt_edp_cf_pattern_type_t;

typedef enum rt_edp_cfpri_decision_e{
	RT_EDP_ACL_CFPRI_ASSIGN,
	RT_EDP_ACL_CFPRI_NOP,
}rt_edp_cfpri_decision_t;

typedef struct rt_edp_cfpri_action_s{
	rt_edp_cfpri_decision_t cfPriDecision;
	uint8 assignedCfPri;
}rt_edp_cfpri_action_t;

typedef enum rt_edp_sid_llid_decision_e{
	RT_EDP_ACL_SID_LLID_ASSIGN,
	RT_EDP_ACL_SID_LLID_NOP,
}rt_edp_sid_llid_decision_t;

typedef struct rt_edp_sid_llid_action_s{
	rt_edp_sid_llid_decision_t sidDecision;
	uint32 assignedSid_or_llid;
}rt_edp_sid_llid_action_t;

typedef enum rt_edp_dscp_decision_e{
	RT_EDP_ACL_DSCP_ASSIGN,
	RT_EDP_ACL_DSCP_NOP,
}rt_edp_dscp_decision_t;

typedef struct rt_edp_dscp_action_s{
	rt_edp_dscp_decision_t dscpDecision;
	uint8 assignedDscp;
}rt_edp_dscp_action_t;

typedef struct rt_edp_log_action_s{
	uint8 assignedCounterIdx;
}rt_edp_log_action_t;

typedef enum rt_edp_acl_uni_decision_e{
	RT_EDP_ACL_UNI_FWD_TO_PORTMASK_ONLY,
	RT_EDP_ACL_UNI_FORCE_BY_MASK,
	RT_EDP_ACL_UNI_TRAP_TO_CPU,
	RT_EDP_AL_UNI_NOP,
}rt_edp_acl_uni_decision_t;

typedef struct rt_edp_uni_action_s{
	rt_edp_acl_uni_decision_t	uniActionDecision;
	uint32 assignedUniPortMask;
}rt_edp_uni_action_t;

typedef enum rt_edp_acl_fwd_decision_e{
	RT_EDP_ACL_FWD_NOP,
	RT_EDP_ACL_FWD_DROP,
	RT_EDP_ACL_FWD_TRAP_TO_CPU,
	RT_EDP_ACL_FWD_DROP_TO_PON,
}rt_edp_acl_fwd_decision_t;

typedef struct rt_edp_fwd_action_s{
	rt_edp_acl_fwd_decision_t	fwdDecision;
}rt_edp_fwd_action_t;

typedef struct rt_edp_classifyEntry_s
{
	int index; //limit in 64-511,  0:invalid   else:should be the asic index,   apolloPro: FAIL means invalid
	rt_edp_cf_direction_type_t direction;

	//patterns
	rt_edp_cf_filter_fields_t filter_fields;

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
	rt_edp_cf_us_action_type_t us_action_field;
	//DS actions  mask
	rt_edp_cf_ds_action_type_t ds_action_field;

	rt_edp_cvlan_tag_action_t action_cvlan;
	rt_edp_svlan_tag_action_t action_svlan;
	rt_edp_cfpri_action_t	action_cfpri;
	rt_edp_sid_llid_action_t action_sid_or_llid;
	rt_edp_dscp_action_t action_dscp;
	rt_edp_fwd_action_t action_fwd;
	rt_edp_log_action_t action_log;
	rt_edp_uni_action_t action_uni;
}rt_edp_classifyEntry_t;

//URL Filter
typedef enum rt_edp_filterControlType_e
{
	RT_EDP_FILTER_BLACK=0,
	RT_EDP_FILTER_WHITE,
	RT_EDP_FILTER_NONE,
}rt_edp_filterControlType_t;

typedef struct rt_edp_urlFilterString_s
{
	unsigned char url_filter_string[EDP_MAX_URL_FILTER_STR_LENGTH];
	unsigned char path_filter_string[EDP_MAX_URL_FILTER_PATH_LENGTH];
	int path_exactly_match;
	int wan_intf;
	uint8 urlfilterSmacCheck;
	rt_edp_filterControlType_t urlfilterSmacMode; //only valid when urlfilterSmacCheck Enable
	uint8 urlfilterSamc[ETHER_ADDR_LEN];
	uint32 urlBlockAllowTimes;
} rt_edp_urlFilterString_t;

//UPnP
typedef enum rt_edp_upnp_type_e
{
	RT_EDP_UPNP_TYPE_ONESHOT=0,
	RT_EDP_UPNP_TYPE_PERSIST=1,
	RT_EDP_UPNP_TYPE_END
} rt_edp_upnp_type_t;

typedef struct rt_edp_upnpConnection_s
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
	rt_edp_upnp_type_t type; //one shot?
	uint32 timeout; //auto-delete after timeout, 0:disable auto-delete
	uint32 idle;	//idle time
	uint32 conn_create_idle;	// idle time since last time created napt-connection by this upnp.
} rt_edp_upnpConnection_t;

typedef enum rt_edp_macFilterEntry_direction_s
{
	RT_EDP_MACFILTER_FILTER_SRC_DEST_MAC_BOTH,
	RT_EDP_MACFILTER_FILTER_SRC_MAC_ONLY,
	RT_EDP_MACFILTER_FILTER_DEST_MAC_ONLY,
	RT_EDP_MACFILTER_FILTER_END
} rt_edp_macFilterEntry_direction_t;

typedef struct rt_edp_macFilterEntry_s
{
	rtk_mac_t mac;
	uint32 isIVL:1; //0:SVL, 1:IVL
	int vlan_id;
	rt_edp_macFilterEntry_direction_t direct;
}rt_edp_macFilterEntry_t;

/* MIB counter */
typedef struct rt_edp_port_mib_info_s
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
}rt_edp_port_mib_info_t;

/* Weight of each priority source */
typedef struct rt_edp_qos_priSelWeight_s
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
}rt_edp_qos_priSelWeight_t;

/* Types of DSCP remarking source */
typedef enum rt_edp_qos_dscpRmkSrc_e
{
    RT_EDP_DSCP_RMK_SRC_INT_PRI,
    RT_EDP_DSCP_RMK_SRC_DSCP,
    RT_EDP_DSCP_RMK_SRC_END
} rt_edp_qos_dscpRmkSrc_t;

/* DOS Port Security*/
typedef enum rt_edp_dos_type_e
{
    RT_EDP_DOS_DAEQSA_DENY = 0,
    RT_EDP_DOS_LAND_DENY,
    RT_EDP_DOS_BLAT_DENY,
    RT_EDP_DOS_SYNFIN_DENY,
    RT_EDP_DOS_XMA_DENY,
    RT_EDP_DOS_NULLSCAN_DENY,
    RT_EDP_DOS_SYN_SPORTL1024_DENY,
    RT_EDP_DOS_TCPHDR_MIN_CHECK,
    RT_EDP_DOS_TCP_FRAG_OFF_MIN_CHECK,
    RT_EDP_DOS_ICMP_FRAG_PKTS_DENY,
    RT_EDP_DOS_POD_DENY,
    RT_EDP_DOS_UDPBOMB_DENY,
    RT_EDP_DOS_SYNWITHDATA_DENY,
    RT_EDP_DOS_SYNFLOOD_DENY,		//13
    RT_EDP_DOS_FINFLOOD_DENY,
    RT_EDP_DOS_ICMPFLOOD_DENY,
    RT_EDP_DOS_TYPE_MAX
}rt_edp_dos_type_t;

typedef enum rt_edp_dos_action_e
{
	RT_EDP_DOS_ACTION_FORWARD = 0,
    RT_EDP_DOS_ACTION_DROP,
    RT_EDP_DOS_ACTION_TRAP,
}rt_edp_dos_action_t;

typedef enum rt_edp_meter_type_e
{
    RT_EDP_METER_ACL = 0,                   // acl policing
    RT_EDP_METER_HOSTPOL,               // host policing and logging
    RT_EDP_METER_STORMCTL,              // storm filtering control
    RT_EDP_METER_PROC,                  // meters that used for proc rate limit
    RT_EDP_METER_MAX,
}rt_edp_meter_type_t;

typedef struct rt_edp_funcbasedMeterConf_s
{
    rt_edp_meter_type_t type;
    uint32 idx;
    rt_edp_enable_t state;
    rt_edp_enable_t ifgInclude;
    uint32 rate;
	rtk_rate_metet_mode_t meterMode;
}rt_edp_funcbasedMeterConf_t;

typedef struct rt_edp_table_highPriPatten_e
{
	uint32 valid:1;
	uint32 isIpv6:1;
	uint32 careSipEn:1;
	uint32 careDipEn:1;
	uint32 careSportEn:1;
	uint32 careDportEn:1;
	uint32 careL4ProtoEn:1;
	uint32 hp_sip[4];
	uint32 hp_dip[4];
	uint16 hp_sport;
	uint16 hp_dport;
	uint8  hp_tcpudp;			//0:UDP 1:TCP
}rt_edp_table_highPriPatten_t;
#endif
#if defined(EDP_ACL_SUPPORT_HOOK_CHECK) && (EDP_ACL_SUPPORT_HOOK_CHECK==1)
typedef enum rt_edp_pkthdr_tagif_e
{
	RT_EDP_SVLAN_TAGIF=(1<<0),
	RT_EDP_CVLAN_TAGIF=(1<<1),
	RT_EDP_PPPOE_TAGIF=(1<<2),
	RT_EDP_IPV4_TAGIF=(1<<3),
	RT_EDP_IPV6_TAGIF=(1<<4),
	RT_EDP_TCP_TAGIF=(1<<5),
	RT_EDP_UDP_TAGIF=(1<<6),
	RT_EDP_IGMP_TAGIF=(1<<7),
	RT_EDP_PPTP_TAGIF=(1<<8),
	RT_EDP_GRE_TAGIF=(1<<9),
	RT_EDP_ICMPV6_TAGIF=(1<<10),
	RT_EDP_ICMP_TAGIF=(1<<11),
	RT_EDP_ESP_TAGIF=(1<<12),  /*siyuan add for alg IPsec passthrough*/
	RT_EDP_MSS_TAGIF=(1<<13),
	RT_EDP_ARP_TAGIF=(1<<14),
	RT_EDP_V6FRAG_TAGIF=(1<<15),
	RT_EDP_IPV6_MLD_TAGIF=(1<<16),
	RT_EDP_GRE_SEQ_TAGIF=(1<<17),
	RT_EDP_GRE_ACK_TAGIF=(1<<18),
	RT_EDP_PPTP_INNER_TAGIF=(1<<19),
	RT_EDP_L2TP_INNER_TAGIF=(1<<20),
	RT_EDP_L2TP_TAGIF=(1<<21),
	RT_EDP_DSLITE_TAGIF=(1<<22),		//egress is dslite packet
	RT_EDP_DSLITE_INNER_TAGIF=(1<<23), //ingress is dslite packet
	RT_EDP_V6TRAP_TAGIF=(1<<24),
	RT_EDP_DVMRP_TAGIF=(1<<25),
	RT_EDP_MOSPF_TAGIF=(1<<26),
	RT_EDP_PIM_TAGIF=(1<<27),
	RT_EDP_DSLITEMC_INNER_TAGIF=(1<<28), //hit dslite Mc table
	RT_EDP_UNKNOWN_L4_TAGIF=(1<<29),
} rt_edp_pkthdr_tagif_t;

typedef enum rt_edp_igrAction_controlBit_e
{
	RT_EDP_ACL_IGR_SVLAN_ACT_DONE_BIT = 	(1<<0),
	RT_EDP_ACL_IGR_CVLAN_ACT_DONE_BIT = 	(1<<1),
	RT_EDP_ACL_IGR_PRIORITY_ACT_DONE_BIT = 	(1<<2),
	RT_EDP_ACL_IGR_POLICE_ACT_DONE_BIT = 	(1<<3),
	RT_EDP_ACL_IGR_FORWARD_ACT_DONE_BIT = 	(1<<4),
	RT_EDP_ACL_IGR_INTCF_ACT_DONE_BIT = 	(1<<5),
	RT_EDP_ACL_IGR_PROUTE_ACT_DONE_BIT = 	(1<<6),
	RT_EDP_ACL_IGR_MIRROR_UDP_ENCAP_ACT_DONE_BIT = (1<<7),
}rt_edp_igrAction_controlBit_t;

typedef enum rt_edp_egrAction_controlBit_e
{
	RT_EDP_ACL_EGR_SVLAN_ACT_DONE_BIT = (1<<0),
	RT_EDP_ACL_EGR_CVLAN_ACT_DONE_BIT = (1<<1),
	RT_EDP_ACL_EGR_CFPRI_ACT_DONE_BIT = (1<<2),
	RT_EDP_ACL_EGR_DSCP_ACT_DONE_BIT = (1<<3),
	RT_EDP_ACL_EGR_FWD_ACT_DONE_BIT = (1<<4), //UNI act or Drop act
	RT_EDP_ACL_EGR_SID_ACT_DONE_BIT = (1<<5),
	RT_EDP_ACL_EGR_MIRROR_UDP_ENCAP_ACT_DONE_BIT = (1<<6),
	RT_EDP_ACL_EGR_FLOWMIB_ACT_DONE_BIT = (1<<7),
}rt_edp_egrAction_controlBit_t;
#endif	//EDP_ACL_SUPPORT_HOOK_CHECK
#if 0	//1 To prevent the data structure of parameter of RG API is changed, use orginal RG data structure temporarily.
typedef struct rt_edp_table_flowmib_e
{
	uint32 in_packet_cnt;
	uint64 in_byte_cnt;
	uint32 out_packet_cnt;
	uint64 out_byte_cnt;
}rt_edp_table_flowmib_t;

typedef struct rt_edp_netifMib_entry_s
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
}rt_edp_netifMib_entry_t;

typedef struct rt_edp_staticRoute_s{
	union{
		struct{
			ipaddr_t addr;
			ipaddr_t mask;
			ipaddr_t nexthop;
		}ipv4;
		struct{
			rtk_ipv6_addr_t addr;
			int mask_length;
			rtk_ipv6_addr_t nexthop;
		}ipv6;
	};
	rtk_mac_t nexthop_mac;
	rt_edp_port_idx_t nexthop_port;	//for non-autolearn

	unsigned int ip_version:1;		//0: ipv4, 1: ipv6
	unsigned int nexthop_mac_auto_learn:1;
}rt_edp_staticRoute_t;

typedef enum  rt_edp_gatewayServiceType_e
{
	RT_EDP_GATEWAY_SERVER_SERVICE	= 0,	//lookup dport
	RT_EDP_GATEWAY_CLIENT_SERVICE	= 1,	//lookup sport
}rt_edp_gatewayServiceType_t;

typedef struct rt_edp_gatewayServicePortEntry_s
{
	int valid;
	unsigned short int port_num;
	rt_edp_gatewayServiceType_t type;
}rt_edp_gatewayServicePortEntry_t;

typedef struct rt_edp_redirectHttpAll_s
{
	char pushweb[EDP_MAX_REDIRECT_PUSH_WEB_SIZE];
	unsigned int enable:2;	//0:disable, 1:enable, 2:enable with URL
	int8 count;	//redirect times, -1: unlimit
}rt_edp_redirectHttpAll_t;

typedef struct rt_edp_redirectHttpURL_s
{
	char url_str[EDP_MAX_URL_FILTER_STR_LENGTH];
	char dst_url_str[EDP_MAX_URL_FILTER_STR_LENGTH];
	int16 count;	//-1: always redirect, 0: stop redirect
}rt_edp_redirectHttpURL_t;

typedef struct rt_edp_redirectHttpWhiteList_s
{
	char url_str[EDP_MAX_URL_FILTER_STR_LENGTH];
	char keyword_str[EDP_MAX_URL_FILTER_STR_LENGTH];
}rt_edp_redirectHttpWhiteList_t;

typedef struct rt_edp_redirectHttpRsp_s
{
	char url_str[EDP_MAX_URL_FILTER_STR_LENGTH];
	unsigned int enable:1;	//0:disable, 1:enable
	unsigned int statusCode:10;
}rt_edp_redirectHttpRsp_t;

typedef struct rt_edp_redirectHttpCount_s
{
	char pushweb[EDP_MAX_REDIRECT_PUSH_WEB_SIZE];
	unsigned int enable:2;	//0:disable, 1:enable, 2:enable with URL
	unsigned int denialSecs:4;
	int8 count;	//redirect times, -1: unlimit
}rt_edp_redirectHttpCount_t;

typedef struct rt_edp_hostPoliceControl_s
{
	rtk_mac_t macAddr;
	rtk_enable_t ingressLimitCtrl;
	rtk_enable_t egressLimitCtrl;
	rtk_enable_t mibCountCtrl;
	int limitMeterIdx;
}rt_edp_hostPoliceControl_t;

typedef struct rt_edp_hostPoliceLogging_s
{
	uint64 rx_count;
	uint64 tx_count;
}rt_edp_hostPoliceLogging_t;

typedef enum rt_edp_sa_learning_exceed_action_e
{
	RT_EDP_SA_LEARN_EXCEED_ACTION_PERMIT=0,
	RT_EDP_SA_LEARN_EXCEED_ACTION_PERMIT_L2,
	RT_EDP_SA_LEARN_EXCEED_ACTION_DROP,
	RT_EDP_SA_LEARN_EXCEED_ACTION_END,
}rt_edp_sa_learning_exceed_action_t;

typedef enum rt_edp_accessWanLimitType_e
{
	RT_EDP_ACCESSWAN_TYPE_UNLIMIT=0,	//turn off
	RT_EDP_ACCESSWAN_TYPE_PORT,			//deprecated, keep for backward-compatible
	RT_EDP_ACCESSWAN_TYPE_PORTMASK,
	RT_EDP_ACCESSWAN_TYPE_CATEGORY,
	RT_EDP_ACCESSWAN_TYPE_END,
}rt_edp_accessWanLimitType_t;

typedef enum rt_edp_accessWanLimitField_e
{
	RT_EDP_ACCESSWAN_LIMIT_BY_SMAC=0,
	RT_EDP_ACCESSWAN_LIMIT_BY_SIP,
	RT_EDP_ACCESSWAN_LIMIT_END,
}rt_edp_accessWanLimitField_t;

typedef struct rt_edp_saLearningLimitInfo_s
{
	int learningLimitNumber;
	rt_edp_sa_learning_exceed_action_t action;
}rt_edp_saLearningLimitInfo_t;

typedef struct rt_edp_accessWanLimitData_s
{
	rt_edp_accessWanLimitType_t type;
	rt_edp_accessWanLimitField_t field;
	union
	{
		unsigned char category;
		rt_edp_portmask_t port_mask;
	}data;
	int learningLimitNumber;
	int learningCount;
	rt_edp_sa_learning_exceed_action_t action;
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	unsigned int wlan0_dev_mask;	//used for WLAN0 device access limit
#endif
}rt_edp_accessWanLimitData_t;

typedef struct rt_edp_accessWanLimitCategory_s
{
	unsigned char category;
	rtk_mac_t mac;
}rt_edp_accessWanLimitCategory_t;

typedef struct rt_edp_groupMacLimit_s
{
	rt_edp_portmask_t port_mask;
	int learningLimitNumber;
	int learningCount;
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	unsigned int wlan0_dev_mask;	//used for WLAN0 device access limit
#endif
}rt_edp_groupMacLimit_t;

typedef enum rt_edp_callback_function_idx_e
{
	RT_EDP_INIT_BY_HW_CALLBACK_IDX = 0,						// _rtk_rg_initParameterSetByHwCallBack
	RT_EDP_ARP_ADD_BY_HW_CALLBACK_IDX,						// _rtk_rg_arpAddByHwCallBack
	RT_EDP_ARP_DEL_BY_HW_CALLBACK_IDX,						// _rtk_rg_arpDelByHwCallBack
	RT_EDP_MAC_ADD_BY_HW_CALLBACK_IDX,						// _rtk_rg_macAddByHwCallBack
	RT_EDP_MAC_DEL_BY_HW_CALLBACK_IDX,						// _rtk_rg_macDelByHwCallBack
	RT_EDP_ROUTING_ADD_BY_HW_CALLBACK_IDX,						// _rtk_rg_routingAddByHwCallBack
	RT_EDP_ROUTING_DEL_BY_HW_CALLBACK_IDX,						// _rtk_rg_routingDelByHwCallBack
	RT_EDP_NAPT_ADD_BY_HW_CALLBACK_IDX,						// _rtk_rg_naptAddByHwCallBack
	RT_EDP_NAPT_DEL_BY_HW_CALLBACK_IDX,						// _rtk_rg_naptDelByHwCallBack
	RT_EDP_BINDING_ADD_BY_HW_CALLBACK_IDX,						// _rtk_rg_bindingAddByHwCallBack
	RT_EDP_BINDING_DEL_BY_HW_CALLBACK_IDX,						// _rtk_rg_bindingDelByHwCallBack
	RT_EDP_INTERFACE_ADD_BY_HW_CALLBACK_IDX,					// _rtk_rg_interfaceAddByHwCallBack
	RT_EDP_INTERFACE_DEL_BY_HW_CALLBACK_IDX,					// _rtk_rg_interfaceDelByHwCallBack
	RT_EDP_NEIGHBOR_ADD_BY_HW_CALLBACK_IDX,					// _rtk_rg_neighborAddByHwCallBack
	RT_EDP_NEIGHBOR_DEL_BY_HW_CALLBACK_IDX,					// _rtk_rg_neighborDelByHwCallBack
	RT_EDP_V6_ROUTING_ADD_BY_HW_CALLBACK_IDX,					// _rtk_rg_v6RoutingAddByHwCallBack
	RT_EDP_V6_ROUTING_DEL_BY_HW_CALLBACK_IDX,					// _rtk_rg_v6RoutingDelByHwCallBack
	RT_EDP_DHCP_REQUEST_BY_HW_CALLBACK_IDX,					// _rtk_rg_dhcpRequestByHwCallBack
	RT_EDP_PPPOE_BEFORE_DIAG_BY_HW_CALLBACK_IDX,					// _rtk_rg_pppoeBeforeDiagByHwCallBack
	RT_EDP_PPTP_BEFORE_DIAG_BY_HW_CALLBACK_IDX,					// _rtk_rg_pptpBeforeDialByHwCallBack
	RT_EDP_L2TP_BEFORE_DIAG_BY_HW_CALLBACK_IDX,					// _rtk_rg_l2tpBeforeDialByHwCallBack
	RT_EDP_PPPOE_DSLITE_BEFORE_DIAG_BY_HW_CALLBACK_IDX,				// _rtk_rg_pppoeDsliteBeforeDialByHwCallBack
	RT_EDP_SOFTWARE_NAPT_INFO_ADD_CALLBACK_IDX,					// _rtk_rg_softwareNaptInfoAddCallBack
	RT_EDP_SOFTWARE_NAPT_INFO_DEL_CALLBACK_IDX,					// _rtk_rg_softwareNaptInfoDeleteCallBack
	RT_EDP_NAPT_PREROUTE_DPI_CALLBACK_IDX,						// _rtk_rg_naptPreRouteDPICallBack
	RT_EDP_NAPT_FORWARD_DPI_CALLBACK_IDX,						// _rtk_rg_naptForwardDPICallBack
	RT_EDP_PPPOE_LCP_STATE_CALLBACK_IDX,						// _rtk_rg_pppoeLCPStateCallBack
#ifdef CONFIG_YUEME_DPI
	RT_EDP_DPI_NAPT_INFO_ADD_CALLBACK_IDX,						// DPI_naptInfoAddCallBack
	RT_EDP_DPI_NAPT_INFO_DEL_CALLBACK_IDX,						// DPI_naptInfoDeleteCallBack
	RT_EDP_DPI_NAPT_PREROUTING_CALLBACK_IDX,					// DPI_naptPreRouteCallBack
	RT_EDP_DPI_NAPT_FORWARDING_CALLBACK_IDX					// DPI_naptForwardCallBack
#endif
} rt_edp_callback_function_idx_t;

typedef struct rt_edp_callbackFunctionPtrGet_s
{
    rt_edp_callback_function_idx_t	callback_function_idx;
    POINTER_CAST callback_function_pointer;
} rt_edp_callbackFunctionPtrGet_t;

typedef enum rt_edp_macFilterWhiteListDelFlag_e
{
	RT_EDP_MACF_DEL_ONE = 0,
	RT_EDP_MACF_DEL_ALL
} rt_edp_macFilterWhiteListDelFlag_t;

typedef struct rt_edp_macFilterWhiteList_s
{
	rt_edp_macFilterWhiteListDelFlag_t	del_flag;
	rtk_mac_t mac;
} rt_edp_macFilterWhiteList_t;


struct RT_EDP_PLATFORM
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
//194

};
#endif

#endif
