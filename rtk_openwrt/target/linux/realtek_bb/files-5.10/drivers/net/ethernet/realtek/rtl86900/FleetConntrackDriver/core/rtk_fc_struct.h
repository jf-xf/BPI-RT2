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
*/

#ifndef __RTK_FC_STRUCT__
#define __RTK_FC_STRUCT__

#if defined(FC_VER_CONTROL)
#include "rtk_fc_ver.h"
#else
#define FC_GIT_VERSION 		"dev.0.0"
#define FC_GIT_SHA1 		"00000000"
#define FC_GIT_SHA1_DATE 	"0000-00-00-0000"
#define FC_GIT_AUTHOR 		"NA"
#define FC_COMPILE_BY 		"NA"
#define FC_COMPILE_DATE 	"0000-00-00-0000"
#endif

#define FC_INTERNAL_VERSION	3

#include <uapi/linux/if_ether.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/gre.h>

#include <linux/workqueue.h> //for wq
#include "rtk_fc_mapper_api.h"		// rt mapper
#include "rtk_fc_api.h"				// fc external api
#include "rtk_fc_define.h"
#include "rtk_fc_error.h"
#include "rtk_fc_mgr.h"
#include "rtk_fc_mgrTRx.h"
#include <rt_ext_mapper.h>
#include <uapi/linux/icmp.h>
#include <linux/proc_fs.h>

#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
#include <net/xfrm.h>
#include <crypto/aead.h>
#include <crypto/internal/geniv.h>
#endif


#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#if defined(CONFIG_FC_RTL8277C_SERIES)
#include <rtk_8277c_asicDriver.h>
#elif defined(CONFIG_FC_RTL9607F_SERIES)
#include <rtk_9607f_asicDriver.h>
#else
#include <rtk_rg_ca8277_asicDriver.h>
#endif
#include <rtk_rg_asic_tblSize.h>
#include "ca_ni.h"
#include <module/intr_bcaster/intr_bcaster.h>
#include <special_packet.h>	//<<FC ACL>> for ca_pkt_type_t, ca_special_packet_set
#endif
#include <rtk_fc_asic_common.h>

#ifdef CONFIG_RTK_L34_XPON_PLATFORM

#if defined(CONFIG_RTL9607C_SERIES)
#include "rtk_rg_apolloPro_asicDriver.h"
#elif defined(CONFIG_RTL9603CVD_SERIES)
#include "rtk_rg_rtl9603cvd_asicDriver.h"
#endif

#if defined(CONFIG_RTL9607C_SERIES)
#include <dal/rtl9607c/dal_rtl9607c.h>
#include <dal/rtl9607c/dal_rtl9607c_hwmisc.h>
#endif
#if defined(CONFIG_RTL9603CVD_SERIES)
#include <dal/rtl9603cvd/dal_rtl9603cvd.h>
#include <dal/rtl9603cvd/dal_rtl9603cvd_hwmisc.h>
#endif

#include <rtk_rg_apolloPro_internal.h>
#include "re8686_nic.h"

// included for link change handler
#include <module/intr_bcaster/intr_bcaster.h>

#include <rtk/acl.h>					//<<FC ACL>> for rtk acl

extern rtk_rg_fbDatabase_t rgpro_db;	// to access apollopro asic related api

#endif

#if defined(CONFIG_RTK_L34_XPON_PLATFORM) || defined(CONFIG_RTK_L34_G3_PLATFORM)

typedef enum rtk_fc_cputagpppoe_action_e
{
    RTK_FC_CPUTAG_PPPOEACT_KEEP = 0,	// tagif=1: keep; tagif=0: keep
    RTK_FC_CPUTAG_PPPOEACT_ADD, 		// tagif=1: keep; tagif=0: add
    RTK_FC_CPUTAG_PPPOEACT_REMOVE,	// tagif=1: remove; tagif=0: keep
    RTK_FC_CPUTAG_PPPOEACT_MODIFY,	// tagif=1: modify; tagif=0: add
    RTK_FC_CPUTAG_PPPOEACT_END
}rtk_fc_cputagpppoe_action_t;

#endif //CONFIG_RTK_L34_XPON_PLATFORM

typedef enum rtk_fc_wifi_lut_fid_e
{
	RTK_FC_WIFI_FID_0 = 0,		// gmac 0
	RTK_FC_WIFI_FID_1 = 1,		// gmac 1
	RTK_FC_WIFI_FID_2 = 2,		// gmac 2
	RTK_FC_WIFI_FID_END = 3,
}rtk_fc_wifi_lut_fid_t;

typedef enum rtk_fc_packetType_e
{
	RTK_FC_PKTTYPE_UC,
	RTK_FC_PKTTYPE_MC,
	RTK_FC_PKTTYPE_BC,
}rtk_fc_packetType_t;

typedef enum rtk_fc_hostMibType_e
{
	RX_BYTE_CNT,
	TX_BYTE_CNT,
}rtk_fc_hostMibType_t;

typedef enum rtk_fc_flowMibType_e
{
	IN_PKT_CNT,
	IN_BYTE_CNT,
	OUT_PKT_CNT,
	OUT_BYTE_CNT,
}rtk_fc_flowMibType_t;

typedef enum rtk_fc_netifMibType_e
{
	IN_UC_PKT_CNT,
	IN_UC_BYTE_CNT,
	IN_MC_PKT_CNT,
	IN_MC_BYTE_CNT,
	IN_BC_PKT_CNT,
	IN_BC_BYTE_CNT,
	OUT_UC_PKT_CNT,
	OUT_UC_BYTE_CNT,
	OUT_MC_PKT_CNT,
	OUT_MC_BYTE_CNT,
	OUT_BC_PKT_CNT,
	OUT_BC_BYTE_CNT,
}rtk_fc_netifMibType_t;

typedef enum rtk_fc_enum_pppCompressMode_e
{
	RTK_FC_COMP_PPP_FF0300XX=4,
	RTK_FC_COMP_PPP_FF03XX=3,
	RTK_FC_COMP_PPP_00XX=2,
	RTK_FC_COMP_PPP_XX=1,
	RTK_FC_COMP_PPP_END=0,
}rtk_fc_enum_pppCompressMode_t;

/* NIC ==================================================================== */
#if defined(CONFIG_LUNA_G3_SERIES)


#define SKB_BUF_SIZE  1800


typedef enum {
	RE8686_RXPRI_DEFAULT=0,
	RE8686_RXPRI_RG,
	RE8686_RXPRI_L34LITE,
	RE8686_RXPRI_VOIP,
	RE8686_RXPRI_OMCI,
	RE8686_RXPRI_OAM,
	RE8686_RXPRI_MPCP,
	RE8686_RXPRI_MUTICAST,
	RE8686_RXPRI_DUMP,
	RE8686_RXPRI_PATCH,	
	RE8686_RXPRI_NPTV6_FF,
	RE8686_RXPRI_WIFI,
}RE8686_RX_PRI_T;
#if 0 // refer ca_ext.h
enum {
	RE8670_RX_STOP=0,
	RE8670_RX_CONTINUE,
	RE8670_RX_STOP_SKBNOFREE,
	RE8670_RX_END
};
#endif
// fake rx_info

struct tx_info{
	union{
		struct{
			u32 own:1;//31
			u32 eor:1;//30
			u32 fs:1;//29
			u32 ls:1;//28
			u32 ipcs:1;//27
			u32 l4cs:1;//26
			u32 tpid_sel:1;//25
			u32 stag_aware:1;//24
			u32 crc:1;//23
			u32 rsvd:6;//17~22
			u32 data_length:17;//0~16
		}bit;
		u32 dw;//double word
	}opts1;
	u32 addr;
	union{
		struct{
			u32 cputag:1;//31
			u32 tx_svlan_action:2;//29~30
			u32 tx_cvlan_action:2;//27~28
			u32 tx_portmask:11;//16~26
			u32 cvlan_vidl:8;//8~15
			u32 cvlan_prio:3;//5~7
			u32 cvlan_cfi:1;// 4
			u32 cvlan_vidh:4;//0~3
		}bit;
		u32 dw;//double word
	}opts2;
	union{
		struct{
			u32 extspa:4;//13~15	=> 28~31 for G3
			u32 aspri:1;//27
			u32 cputag_pri:3;//24~26
			u32 keep:1;//23
			u32 rsvd2:1;//22
			u32 dislrn:1;//21
			u32 cputag_psel:1;//20
			u32 rsvd1:2;//18~19
			u32 l34_keep:1;//17
			u32 gmac_id:4;//13~16		 //software used for gmac_tx_idx(0:gmac9, 1:gmac10, 2:gmac7)
			u32 tx_pppoe_action:2;//11~12
			u32 tx_pppoe_idx:4;//7~10
			u32 tx_dst_stream_id:7;//0~6
		}bit;
		u32 dw;//double word
	}opts3;
	union{
		struct{
			u32 lgsen:1;//31
			u32 lgmtu:11;//20~30
			u32 rsvd:4;//16~19
			u32 svlan_vidl:8;//8~15
			u32 svlan_prio:3;//5~7
			u32 svlan_cfi:1;// 4
			u32 svlan_vidh:4;//0~3
		}bit;
		u32 dw;//double word
	}opts4;
#if defined(CONFIG_FC_RTL8198F_SERIES)
	union{
		struct{
			u32 pe_portmask:23; //9~31
			u32 reserved:9; //0~8
		}bit;
		u32 dw;//double word
	}opts5;
#endif
};

#endif


/* End of NIC ============================================================= */


typedef enum rtk_fc_enum_macType_e
{
    RG_FC_MACTYPE_SA = 0,
    RG_FC_MACTYPE_DA,
} rtk_fc_enum_macType_t;


// Link Status

typedef enum rtk_fc_port_speed_e
{
    RTK_RG_PORT_SPEED_10M = 0,
    RTK_RG_PORT_SPEED_100M,
    RTK_RG_PORT_SPEED_1000M,
    RTK_RG_PORT_SPEED_END,
} rtk_fc_port_speed_t;
typedef enum rtk_fc_port_duplex_e
{
    RTK_RG_PORT_HALF_DUPLEX = 0,
    RTK_RG_PORT_FULL_DUPLEX,
    RTK_RG_PORT_DUPLEX_END
} rtk_fc_port_duplex_t;
typedef enum rtk_fc_port_linkStatus_e
{
    RTK_RG_PORT_LINKDOWN = 0,
    RTK_RG_PORT_LINKUP,
    RTK_RG_PORT_LINKSTATUS_END
} rtk_fc_port_linkStatus_t;

typedef struct rtk_fc_portStatusInfo_s
{
	rtk_fc_port_linkStatus_t linkStatus;
	rtk_fc_port_speed_t		linkSpeed;
	rtk_fc_port_duplex_t	linkDuplex;
}rtk_fc_portStatusInfo_t;

typedef struct rtk_fc_burstPktSend_conf_s
{
	uint16 sw_mcgid;			// init as CA_UINT16_INVALID
	uint16 hw_mcgid;			// init as CA_UINT16_INVALID
	uint32 pkt_lspid;
}rtk_fc_burstPktSend_conf_t;

//+++++++++++++++ ACL Related Structure Start +++++++++++++++
/*Internal Use*/
typedef enum rtk_fc_aclSWEntry_used_tables_index_s
{
	RTK_FC_ACL_USED_INGRESS_SIP4TABLE_INDEX = 0,
	RTK_FC_ACL_USED_INGRESS_DIP4TABLE_INDEX = 1,
	RTK_FC_ACL_USED_INGRESS_SIP6TABLE_INDEX = 2,
	RTK_FC_ACL_USED_INGRESS_DIP6TABLE_INDEX = 3,
	RTK_FC_ACL_USED_INGRESS_SPORTTABLE_INDEX = 4,
	RTK_FC_ACL_USED_INGRESS_DPORTTABLE_INDEX = 5,
	RTK_FC_ACL_USED_INGRESS_PKTLENTABLE_INDEX = 6,
	RTK_FC_ACL_USED_TABLE_END = 7,
} rtk_fc_aclSWEntry_used_tables_index_t;

typedef enum rtk_fc_aclSWEntry_used_tables_field_s
{
	RTK_FC_ACL_USED_INGRESS_SIP4TABLE = (1<<RTK_FC_ACL_USED_INGRESS_SIP4TABLE_INDEX),
	RTK_FC_ACL_USED_INGRESS_DIP4TABLE = (1<<RTK_FC_ACL_USED_INGRESS_DIP4TABLE_INDEX),
	RTK_FC_ACL_USED_INGRESS_SIP6TABLE = (1<<RTK_FC_ACL_USED_INGRESS_SIP6TABLE_INDEX),
	RTK_FC_ACL_USED_INGRESS_DIP6TABLE = (1<<RTK_FC_ACL_USED_INGRESS_DIP6TABLE_INDEX),
	RTK_FC_ACL_USED_INGRESS_SPORTTABLE = (1<<RTK_FC_ACL_USED_INGRESS_SPORTTABLE_INDEX),
	RTK_FC_ACL_USED_INGRESS_DPORTTABLE = (1<<RTK_FC_ACL_USED_INGRESS_DPORTTABLE_INDEX),
	RTK_FC_ACL_USED_INGRESS_PKTLENTABLE = (1<<RTK_FC_ACL_USED_INGRESS_PKTLENTABLE_INDEX),
} rtk_fc_aclSWEntry_used_tables_field_t;



/*User Define ACL*/
/*struct rt_acl_filterAndQos_t: please refer rt_acl_ext.h*/
typedef enum rt_acl_action_group_e
{
	RT_ACL_ACTION_GROUP_FORWARD = 0,
	RT_ACL_ACTION_GROUP_PRIORITY,
	RT_ACL_ACTION_GROUP_METER,
	RT_ACL_ACTION_GROUP_LOGGING,
	RT_ACL_ACTION_GROUP_END,
} rt_acl_action_group_t;

typedef struct rtk_fc_aclFilterEntry_s
{
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	uint32 hw_aclEntry_count;
	rtk_portmask_t hw_aclEntry_port;
	unsigned char hw_aclEntry_index[128];//use string to record the hw acl related index
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
	uint32 hw_hash_index;
#endif	//CONFIG_FC_G3_G3LITE_SERIES
#else
	uint32 hw_aclEntry_start;
	uint32 hw_aclEntry_size;
	rtk_fc_aclSWEntry_used_tables_field_t hw_used_table;//record which range tables are used
	uint8 hw_used_table_index[RTK_FC_ACL_USED_TABLE_END];//record used range tables index
#endif
	uint16 hw_share_meter_index;		//record the hw meter index transform by action_meter_group_share_meter_index
	uint16 hw_mib_index;				//record the hw mib index transform by action_logging_group_mib_index
	rt_acl_filterAndQos_t acl_filter;
	rtk_enable_t valid;
}rtk_fc_aclFilterEntry_t;

/*HW ACL*/
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
typedef enum rtk_fc_aclField_Teamplate_s
{
	//related to Template[0]: used for L2 pattern
	RTK_ACL_TEMPLATE_DMAC0 				=0,
	RTK_ACL_TEMPLATE_DMAC1 				=1,
	RTK_ACL_TEMPLATE_DMAC2 				=2,
	RTK_ACL_TEMPLATE_SMAC0 				=3,
	RTK_ACL_TEMPLATE_SMAC1 				=4,
	RTK_ACL_TEMPLATE_SMAC2 				=5,
	RTK_ACL_TEMPLATE_CTAG 				=6,
	RTK_ACL_TEMPLATE_STAG					=7,

	//related to Template[1]: used for L3 pattern
	RTK_ACL_TEMPLATE_IPv4DIP0 				=8,
	RTK_ACL_TEMPLATE_IPv4DIP1 				=9,
	RTK_ACL_TEMPLATE_IPv4SIP0 				=10,
	RTK_ACL_TEMPLATE_IPv4SIP1				=11,
	RTK_ACL_TEMPLATE_IP_RANGE				=12,
	RTK_ACL_TEMPLATE_ETHERTYPE 			=13,
	RTK_ACL_TEMPLATE_EXTPORTMASK			=14,
	RTK_ACL_TEMPLATE_GEMPORT				=15,

	//related to Template[2]: used for L4 pattern
	RTK_ACL_TEMPLATE_L4_DPORT				=16, //use Field selector[1]
	RTK_ACL_TEMPLATE_L4_SPORT				=17, //use Field selector[0]
	RTK_ACL_TEMPLATE_L4PORT_RANGE		=18,
	RTK_ACL_TEMPLATE_IP4_TOS_PROTO		=19,
	RTK_ACL_TEMPLATE_IP6_TC_NH 			=20,
	RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS	=21,
	RTK_ACL_TEMPLATE_RSV_2_6 			=22, //not used
	RTK_ACL_TEMPLATE_PKTLEN_RANGE		=23,

	//related to Template[3]: used for field select pattern
	RTK_ACL_TEMPLATE_IPCP_IP6CP			=24, //FS[0]: init for IPCP / IP6CP					// now rsv
	RTK_ACL_TEMPLATE_FLAG_AND_OFFSET	=25, //FS[1]: init for ipv4 Flags and Fragment offset
	RTK_ACL_TEMPLATE_TCP_FLAGS			=26, //FS[2]: init for TCP Flags
	RTK_ACL_TEMPLATE_IPV6_FLOW_LABEL_0	=27, //FS[3]: init for ipv6 floeLabel[0:3]				// now rsv
	RTK_ACL_TEMPLATE_IPV6_FLOW_LABEL_1	=28, //FS[4]: init for ipv6 floeLabel[4:19]				// now rsv
	RTK_ACL_TEMPLATE_RSV_3_5			=29, //not used
	RTK_ACL_TEMPLATE_ARP_TARGET_IP_UPPER	=30, // FS[6]: init for arp target ip[31:16]
	RTK_ACL_TEMPLATE_ARP_TARGET_IP_LOWER	=31, // FC[7]: init for arp target ip[15:0]

	//related to Template[4]: rsv
	RTK_ACL_TEMPLATE_RSV_4_0			=32, //not used
	RTK_ACL_TEMPLATE_RSV_4_1			=33, //not used
	RTK_ACL_TEMPLATE_RSV_4_2			=34, //not used
	RTK_ACL_TEMPLATE_RSV_4_3			=35, //not used
	RTK_ACL_TEMPLATE_RSV_4_4			=36, //not used
	RTK_ACL_TEMPLATE_RSV_4_5			=37, //not used
	RTK_ACL_TEMPLATE_RSV_4_6			=38, //not used
	RTK_ACL_TEMPLATE_RSV_4_7			=39, //not used

	//related to Template[5]: SIPv6
	RTK_ACL_TEMPLATE_IPv6SIP0			=40,
	RTK_ACL_TEMPLATE_IPv6SIP1			=41,
	RTK_ACL_TEMPLATE_IPv6SIP2			=42,
	RTK_ACL_TEMPLATE_IPv6SIP3			=43,
	RTK_ACL_TEMPLATE_IPv6SIP4			=44,
	RTK_ACL_TEMPLATE_IPv6SIP5			=45,
	RTK_ACL_TEMPLATE_IPv6SIP6			=46,
	RTK_ACL_TEMPLATE_IPv6SIP7			=47,

	//related to Template[6]: DIPv6
	RTK_ACL_TEMPLATE_IPv6DIP0			=48,
	RTK_ACL_TEMPLATE_IPv6DIP1			=49,
	RTK_ACL_TEMPLATE_IPv6DIP2			=50,
	RTK_ACL_TEMPLATE_IPv6DIP3			=51,
	RTK_ACL_TEMPLATE_IPv6DIP4			=52,
	RTK_ACL_TEMPLATE_IPv6DIP5			=53,
	RTK_ACL_TEMPLATE_IPv6DIP6			=54,
	RTK_ACL_TEMPLATE_IPv6DIP7			=55,

	//related to Template[7]: flexible use for port range in user ACL
	RTK_ACL_TEMPLATE_TCP_DPORT			=56,
	RTK_ACL_TEMPLATE_TCP_SPORT			=57,
	RTK_ACL_TEMPLATE_UDP_DPORT			=58,
	RTK_ACL_TEMPLATE_UDP_SPORT 			=59,
	RTK_ACL_TEMPLATE_IP4SIP0_INNER		=60,
	RTK_ACL_TEMPLATE_IP4SIP1_INNER		=61,
	RTK_ACL_TEMPLATE_IP4DIP0_INNER		=62,
	RTK_ACL_TEMPLATE_IP4DIP1_INNER		=63,

	RTK_ACL_TEMPLATE_END,
}rtk_fc_aclField_Teamplate_t;
#endif
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
typedef enum rtk_acl_field_ca_type_e
{
    RTK_ACL_FIELD_CA_UNUSED = 0,
    RTK_ACL_FIELD_MAC_SA,				//CLS_KEY_MSK_MAC_SA, mac_sa
    RTK_ACL_FIELD_MAC_DA,				//CLS_KEY_MSK_MAC_DA, mac_da
    RTK_ACL_FIELD_IP_PROTOCOL,			//CLS_KEY_MSK_IP_PROTOCOL, ip_protocol
    RTK_ACL_FIELD_IPV4_SA,				//CLS_KEY_MSK_IPV4_SA, ip_sa
    RTK_ACL_FIELD_IPV4_DA,				//CLS_KEY_MSK_IPV4_DA, ip_da
    RTK_ACL_FIELD_IPV6_SA,				//CLS_KEY_MSK_IPV6_SA, ip_sa
    RTK_ACL_FIELD_IPV6_DA,				//CLS_KEY_MSK_IPV6_DA, ip_da
    RTK_ACL_FIELD_IP_VER,				//CLS_KEY_MSK_IP_VER
    RTK_ACL_FIELD_ETHERTYPE_ENC,		//CLS_KEY_MSK_ETHERTYPE_ENC, ethertype
    RTK_ACL_FIELD_L4_PORT,				//CLS_KEY_MSK_L4_PORT, src_port/dst_port
    RTK_ACL_FIELD_L4_VLD,				//CLS_KEY_MSK_L4_VLD
    RTK_ACL_FIELD_DSCP,					//CLS_KEY_MSK_DSCP, dscp
	RTK_ACL_FIELD_VLAN_CNT,				//CLS_KEY_MSK_VLAN_CNT, vlan_count
	RTK_ACL_FIELD_TOP_VL_802_1P,		//CLS_KEY_MSK_TOP_VL_802_1P, pri
	RTK_ACL_FIELD_TOP_VID,				//CLS_KEY_MSK_TOP_VID, vid
	RTK_ACL_FIELD_TOP_DEI,				//CLS_KEY_MSK_TOP_DEI, cfi or dei
	RTK_ACL_FIELD_IPV6_FLOW_LBL,		//CLS_KEY_MSK_IPV6_FLOW_LBL, flow_label
	RTK_ACL_FIELD_TCP_RDP_CTRL,			//CLS_KEY_MSK_TCP_RDP_CTRL, tcp_flags
	RTK_ACL_FIELD_PKTLEN_RNG_MATCH_VEC,	//CLS_KEY_MSK_PKTLEN_RNG_MATCH_VEC, packet length range
	RTK_ACL_FIELD_MAC_DA_MC, 			//CLS_KEY_MSK_MAC_DA_MC
	RTK_ACL_FIELD_MAC_DA_BC,			//CLS_KEY_MSK_MAC_DA_BC
	RTK_ACL_FIELD_MAC_DA_IP_MC,			//CLS_KEY_MSK_MAC_DA_IP_MC
	RTK_ACL_FIELD_MAC_DA_RSVD,			//CLS_KEY_MSK_MAC_DA_RSVD
	RTK_ACL_FIELD_MAC_RANGE,			//CLS_KEY_MSK_MAC_RANGE, mac range
	RTK_ACL_FIELD_IP_RANGE,				//CLS_KEY_MSK_IP_RANG, ip range

    RTK_ACL_FIELD_CA_END,
} rtk_acl_field_ca_type_t;

typedef enum rtk_acl_ca_template_s
{
    RTK_ACL_TEMPLATE_CL_IF_ID_KEY_MSK = 0,
	RTK_ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK,
	RTK_ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK,			//CL_IPV4_TUNNEL_ID_KEY_MSK
	RTK_ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK,			//CL_IPV4_TUNNEL_ID_KEY_MSK
	RTK_ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK,			//CL_IPV6_SHORT_KEY_MSK
	RTK_ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK,			//CL_IPV6_SHORT_KEY_MSK
	RTK_ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK,
	RTK_ACL_TEMPLATE_CL_MCST_MAC_DA_KEY_MSK,			//CL_MCST_KEY_MSK
	RTK_ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK,				//CL_MCST_KEY_MSK
	RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK,
#if !defined(CONFIG_FC_CAG3_SERIES)
	RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK,			//CL_FLOW_KEY_MSK
	RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK,			//CL_FLOW_KEY_MSK
	RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK,	//CL_FLOW_KEY_MSK
	RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK,	//CL_FLOW_KEY_MSK
#endif

	RTK_ACL_TEMPLATE_CA_END,
} rtk_acl_ca_template_t;

#if defined(CONFIG_FC_G3_G3LITE_SERIES)
typedef enum rtk_acl_remap_hash_tcp_type_s
{
    RTK_FC_ACL_HASH_TCP_TYPE_SYN_ONLY = 0,
	RTK_FC_ACL_HASH_TCP_TYPE_SYN_ACK,
	RTK_FC_ACL_HASH_TCP_TYPE_MAX,
} rtk_acl_remap_hash_tcp_type_t;
#endif	//CONFIG_FC_G3_G3LITE_SERIES

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
typedef struct rtk_fc_aclMib_s
{
	uint8 mib_type;	//0:byte count 1:packet count
	uint32 pkt_cnt;
	uint64 bytes_cnt;
}rtk_fc_aclMib_t;
#endif	//CONFIG_FC_RTL8277C_SERIES || CONFIG_FC_RTL9607F_SERIES

#endif

/*reserved ACL*/
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
typedef enum rtk_rg_aclAndCf_reserved_type_e
{
	/*CLS Rule for G3*/
	RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE=0,
	RTK_CA_CLS_TYPE_GENERIC_INTF_HASH_PROFILE_DECISION,
	RTK_CA_CLS_TYPE_GENERIC_INTF_MC_PROFILE_WITH_DMAC,
	RTK_CA_CLS_TYPE_L2_UPSTREAM_LOOPBACK_INGRESS_FORWARD_L3FE, //upstream from loopback port => forward to L3FE
	RTK_CA_CLS_TYPE_L2_UPSTREAM_LAN_INGRESS_FORWARD_LOOPBACK, //upstream from loopback port => forward to Loopback port
	RTK_CA_CLS_TYPE_L2_INGRESS_SA_HOSTPOLICING,
	RTK_CA_CLS_TYPE_L2_EGRESS_DA_HOSTPOLICING,
	RTK_CA_CLS_TYPE_REARRANGE_PROTECTION,
	RTK_CA_CLS_TYPE_BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE,
	RTK_CA_CLS_TYPE_BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE_GW_MAC_UPDATE,
	RTK_CA_CLS_TYPE_VXLAN_FFD_US,
	RTK_CA_CLS_TYPE_VXLAN_FFD_DS,
	RTK_CA_CLS_TYPE_VXLAN_FFD_EXTRA_DS,
	RTK_CLS_TYPE_USER_ACL_REARRANGE,
	RTK_CLS_TYPE_HTTP_ACCELERATE_BY_PE,
	RTK_CLS_TYPE_SRV6_ACCELERATE_BY_PE,
	RTK_CLS_TYPE_RESET_L2_L3_CLS,
	RTK_CLS_TYPE_L2_INGRESS_FORWARD_PORT,
	RTK_CLS_TYPE_L2_INGRESS_MC_FORWARD_L3FE,
	RTK_CLS_TYPE_L2_EGRESS_FORWARD_L3FE,
	RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK,

	RTK_FC_ACLANDCF_RESERVED_SPECIAL_END,

	/*HEAD PATCH: The lower index number, the higher acl&cf priority*/

	RTK_FC_ACLANDCF_RESERVED_ALL_TRAP,
	RTK_FC_ACLANDCF_RESERVED_SNAP_OTHER_TRAP,
	RTK_FC_ACLANDCF_RESERVED_BURST_PACKET_SEND_MCE,
	RTK_FC_ACLANDCF_RESERVED_WIFI_SNAP_TRANS,
	RTK_FC_ACLANDCF_RESERVED_WIFI_TXAMSDU_HW_LOOKUP,
	RTK_FC_ACLANDCF_RESERVED_WIFI_TXAMSDU_MC_TO_UC,
	RTK_FC_ACLANDCF_RESERVED_IPSEC_ACCELERATE_BY_PE,
	RTK_FC_ACLANDCF_RESERVED_ACL_LAN_PORTMASK_RESET,
	RTK_FC_ACLANDCF_RESERVED_SMALL_TCP_ACK_TRAP,
	RTK_FC_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP,
	RTK_FC_ACLANDCF_RESERVED_MULTICAST_RIP_TRAP,
	RTK_FC_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY,
	RTK_FC_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY,
	RTK_FC_ACLANDCF_RESERVED_PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY,

	RTK_FC_ACLANDCF_RESERVED_HEAD_END,

	/*TAIL PATCH: The lower index number, the higher acl&cf priority*/
	RTK_FC_ACLANDCF_RESERVED_MC_JUMBO_DROP,
	RTK_FC_ACLANDCF_RESERVED_TTL_LESS_ONE_TRAP,
	RTK_FC_ACLANDCF_RESERVED_SMP_DISPATCH_NIC_RX_BY_ACL,
	RTK_FC_ACLANDCF_RESERVED_MULTICAST_DSLITE_TRAP,
	RTK_FC_ACLANDCF_RESERVED_L2TP_DATA_LOWER_PRIORITY,
	RTK_FC_ACLANDCF_RESERVED_ICMPV4_TRAP,
	RTK_FC_ACLANDCF_RESERVED_ICMPV6_TRAP,
	RTK_FC_ACLANDCF_RESERVED_DHCPV4_TRAP,
	RTK_FC_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP,
	RTK_FC_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT,
    RTK_FC_ACLANDCF_RESERVED_TAIL_END,

}rtk_rg_aclAndCf_reserved_type_t;

typedef struct rtk_fc_aclAndCf_reserved_assign_priority_s{
	int priority;
}rtk_fc_aclAndCf_reserved_assign_priority_t;

typedef struct rtk_fc_aclAndCf_reserved_generic_intf_mc_dmac_s{
	uint32 netif_idx;
	uint32 acl_info;
	rtk_mac_t gateway_mac_addr;
}rtk_fc_aclAndCf_reserved_generic_intf_mc_dmac_t;

typedef struct rtk_fc_aclAndCf_reserved_generic_intf_mirrored_cfg_s{
	uint8 mirror_en;
	uint8 mirrord_port;
	uint8 cls_mir_en;
}rtk_fc_aclAndCf_reserved_generic_intf_mirrored_cfg_t;

typedef struct rtk_fc_aclAndCf_reserved_vxlan_ffd_s{
	uint16 l4_sport;
	uint16 l4_dport;
	uint16 pkt_len_start;	// = pkt_len_end
	uint8 lan_port_idx;
}rtk_fc_aclAndCf_reserved_vxlan_ffd_t;

typedef struct rtk_fc_aclAndCf_reserved_accelerate_by_pe_s{
	//input, HPPT_ACCELERATE_BY_PE
	uint8 igr_port_idx;
	uint32 is_ipv4:1;
	union{
		uint32 ipv4_src_ip;
		rtk_ipv6_addr_t ipv6_src_ip;
	};
	union{
		uint32 ipv4_dst_ip;
		rtk_ipv6_addr_t ipv6_dst_ip;
	};
	uint16 l4_proto_value;
	uint16 l4_sport;
	uint16 l4_dport;
	uint8 action_redirect_ldpid;
	uint8 action_pol_id;
	//output
	uint32 rslt_idx;
}rtk_fc_aclAndCf_reserved_accelerate_by_pe_t;

typedef struct rtk_fc_aclAndCf_reserved_srv6_accelerate_by_pe_s{
	//input, SRV6_ACCELERATE_BY_PE
	rtk_ipv6_addr_t ipv6_dst_ip;
	uint8 action_redirect_ldpid;
	uint8 action_mdata_dev_id;
	//output
	uint32 rslt_idx;
}rtk_fc_aclAndCf_reserved_srv6_accelerate_by_pe_t;

typedef struct rtk_fc_aclAndCf_reserved_ipsec_accelerate_by_pe_s{
	uint8 igr_port_idx;
	uint8 action_redirect_ldpid;
}rtk_fc_aclAndCf_reserved_ipsec_accelerate_by_pe_t;

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
typedef enum rtk_fc_aclAndCf_reserved_pkt_type_e
{
	RTK_PP_PKT_TYPE_OTHERS=0,
	RTK_PP_PKT_TYPE_AH,
	RTK_PP_PKT_TYPE_ESP,
	RTK_PP_PKT_TYPE_L2TP_IP_CTRL,
	RTK_PP_PKT_TYPE_L2TP_IP_DATA,
	RTK_PP_PKT_TYPE_L2TP_UDP_CTRL,
	RTK_PP_PKT_TYPE_L2TP_UDP_DATA,
	RTK_PP_PKT_TYPE_VXLAN_IP,
	RTK_PP_PKT_TYPE_PPTP_GRE,
	RTK_PP_PKT_TYPE_IPV4_IN_V6,
	RTK_PP_PKT_TYPE_IPV6_IN_V4,
	RTK_PP_PKT_TYPE_IPV4_ONLY,
	RTK_PP_PKT_TYPE_IPV6_ONLY,
	RTK_PP_PKT_TYPE_VXLAN_NIP,
	RTK_PP_PKT_TYPE_SRV6_IP,		//9607F only
	RTK_PP_PKT_TYPE_SRV6_NIP,		//9607F only
	RTK_PP_PKT_TYPE_L2GRE_IP,		//9607F only
	RTK_PP_PKT_TYPE_L2GRE_NIP,		//9607F only
}rtk_fc_aclAndCf_reserved_pkt_type_t;

typedef enum rtk_fc_aclAndCf_reserved_dual_hdr_type_e
{
	RTK_FC_CLS_DUAL_L2TP = 0,
	RTK_FC_CLS_DUAL_VXLAN,
	RTK_FC_CLS_DUAL_VXLAN_NON_IP,
	RTK_FC_CLS_DUAL_PPTP_GRE,
	RTK_FC_CLS_DUAL_MAP_E,
	RTK_FC_CLS_DUAL_DSLITE,
	RTK_FC_CLS_DUAL_6RD,
	RTK_FC_CLS_DUAL_MAP_T,
	RTK_FC_CLS_DUAL_MAP_T_TCP_FLAG0,
	RTK_FC_CLS_DUAL_SRV6,				//9607F only
	RTK_FC_CLS_DUAL_SRV6_NON_IP,		//9607F only
	RTK_FC_CLS_DUAL_L2GRE,				//9607F only
	RTK_FC_CLS_DUAL_L2GRE_NON_IP,		//9607F only
	RTK_FC_CLS_DUAL_XLAT,				//9607F only
	RTK_FC_CLS_DUAL_MAX,
}rtk_fc_aclAndCf_reserved_dual_hdr_type_t;

typedef struct rtk_fc_aclAndCf_reserved_dual_hdr_vlan_s
{
	uint32 vlan_check_en:1;
	uint32 has_stag:1;
	uint32 svlan_id:12;
	uint32 has_ctag:1;
	uint32 cvlan_id:12;
}rtk_fc_aclAndCf_reserved_dual_hdr_vlan_t;

typedef struct rtk_fc_aclAndCf_reserved_dual_hdr_l2tp_s
{	//packet_type = 6
	rtk_fc_aclAndCf_reserved_dual_hdr_vlan_t vlan;
	uint32 tunnel_id:16;
	uint32 session_id:16;
	uint32 is_ipv4:1;
	union{
		uint32 ipv4_dst_ip;
		uint32 ipv6_dst_ip[4];
	};
}rtk_fc_aclAndCf_reserved_dual_hdr_l2tp_t;

typedef struct rtk_fc_aclAndCf_reserved_dual_hdr_vxlan_s
{	//packet_type = 7
	uint32 dst_ip[4];
	rtk_fc_aclAndCf_reserved_dual_hdr_vlan_t vlan;
	uint32 vni:24;
}rtk_fc_aclAndCf_reserved_dual_hdr_vxlan_t;

typedef struct rtk_fc_aclAndCf_reserved_dual_hdr_pptp_gre_s
{	//packet_type = 8
	rtk_fc_aclAndCf_reserved_dual_hdr_vlan_t vlan;
	uint32 gre_call_id:16;
	uint32 is_ipv4:1;
	union{
		uint32 ipv4_dst_ip;
		uint32 ipv6_dst_ip[4];
	};
}rtk_fc_aclAndCf_reserved_dual_hdr_pptp_gre_t;

typedef struct rtk_fc_aclAndCf_reserved_dual_hdr_map_e_s
{	//packet_type = 9(IPv4 in IPv6)
	uint32 dst_ip[4];
	rtk_fc_aclAndCf_reserved_dual_hdr_vlan_t vlan;
}rtk_fc_aclAndCf_reserved_dual_hdr_map_e_t;

typedef struct rtk_fc_aclAndCf_reserved_dual_hdr_dslite_s
{	//packet_type = 9(IPv4 in IPv6)
	uint32 src_ip[4];
	rtk_fc_aclAndCf_reserved_dual_hdr_vlan_t vlan;
}rtk_fc_aclAndCf_reserved_dual_hdr_dslite_t;

typedef struct rtk_fc_aclAndCf_reserved_dual_hdr_6rd_s
{	//packet_type = 10(IPv6 in IPv4)
	uint32 src_ip;
	uint32 dst_ip;
	rtk_fc_aclAndCf_reserved_dual_hdr_vlan_t vlan;
}rtk_fc_aclAndCf_reserved_dual_hdr_6rd_t;

typedef struct rtk_fc_aclAndCf_reserved_dual_hdr_map_t_s
{	//packet_type = 12(IPv6 only)
	uint32 dst_ip[4];
	rtk_fc_aclAndCf_reserved_dual_hdr_vlan_t vlan;
}rtk_fc_aclAndCf_reserved_dual_hdr_map_t_t;

typedef struct rtk_fc_aclAndCf_reserved_dual_hdr_srv6_s
{	//packet_type = 0xe(SRV6)/0xf(SRV6_non_ip)
	uint32 dst_ip[4];
	rtk_fc_aclAndCf_reserved_dual_hdr_vlan_t vlan;
}rtk_fc_aclAndCf_reserved_dual_hdr_srv6_t;

typedef struct rtk_fc_aclAndCf_reserved_dual_hdr_l2_gre_s
{	//packet_type = 0x10(L2GRE)/0x11(L2GRE_non_ip)
	rtk_fc_aclAndCf_reserved_dual_hdr_vlan_t vlan;
	uint32 is_ipv4:1;
	union{
		uint32 ipv4_src_ip;
		uint32 ipv6_src_ip[4];
	};
	union{
		uint32 ipv4_dst_ip;
		uint32 ipv6_dst_ip[4];
	};
}rtk_fc_aclAndCf_reserved_dual_hdr_l2_gre_t;

typedef struct rtk_fc_aclAndCf_reserved_dual_hdr_xlat_s
{	//packet_type = 12(IPv6 only)
	uint8 src_ip_subnet_size;
	uint8 dst_ip_subnet_size;
	uint32 src_ip[4];
	uint32 dst_ip[4];
	rtk_fc_aclAndCf_reserved_dual_hdr_vlan_t vlan;
}rtk_fc_aclAndCf_reserved_dual_hdr_xlat_t;

typedef struct rtk_fc_aclAndCf_reserved_dual_hdr_chk_s{
	//input
	rtk_fc_aclAndCf_reserved_dual_hdr_type_t packet_type;
	rtk_rg_asic_flow_profile_t hash_tuple_idx;
	uint8 profile_idx;	//0: wan, 1: lan
	uint8 igr_intf_idx;
	union
	{
		rtk_fc_aclAndCf_reserved_dual_hdr_l2tp_t l2tp;
		rtk_fc_aclAndCf_reserved_dual_hdr_vxlan_t vxlan;
		rtk_fc_aclAndCf_reserved_dual_hdr_pptp_gre_t pptp_gre;
		rtk_fc_aclAndCf_reserved_dual_hdr_map_e_t map_e;
		rtk_fc_aclAndCf_reserved_dual_hdr_dslite_t dslite;
		rtk_fc_aclAndCf_reserved_dual_hdr_6rd_t six_rd;
		rtk_fc_aclAndCf_reserved_dual_hdr_map_t_t map_t;
		rtk_fc_aclAndCf_reserved_dual_hdr_srv6_t srv6;		//9607F only
		rtk_fc_aclAndCf_reserved_dual_hdr_l2_gre_t l2_gre;	//9607F only
		rtk_fc_aclAndCf_reserved_dual_hdr_xlat_t xlat;		//9607F only
	};
	//output
	uint32 rslt_idx;
}rtk_fc_aclAndCf_reserved_dual_hdr_chk_t;
#endif

typedef struct rtk_fc_aclAndCf_reserved_global_s
{
	uint8 reservedMask[RTK_FC_ACLANDCF_RESERVED_TAIL_END];
	uint8 reservedMap[RT_ACL_RESERVED_TYPE_MAX];

	uint64 acl_wanPortMask;	//rtk_fc_portmask_t

	rtk_fc_burstPktSend_conf_t burstPktSend_conf;
	rtk_fc_aclAndCf_reserved_ipsec_accelerate_by_pe_t ipsec_accelerate_by_pe;
	rtk_fc_aclAndCf_reserved_assign_priority_t l2tp_ctrl_lcp_assign_prioity;
	rtk_fc_aclAndCf_reserved_assign_priority_t pppoe_lcp_assign_prioity;
	rtk_fc_aclAndCf_reserved_assign_priority_t pptp_gre_ctrl_lcp_assign_prioity;
	rtk_fc_aclAndCf_reserved_generic_intf_mc_dmac_t generic_intf_mc_dmac[RTK_FC_TABLESIZE_INTF_MC_ACL];
	uint32 dslite_intf_count;
}rtk_fc_acl_reserved_global_t;

typedef enum rtk_fc_g3_caRulePriority_s
{
	RTK_CA_CLS_PRIORITY_INIT = -1,
	RTK_CA_CLS_PRIORITY_L2_0 = 0,
	RTK_CA_CLS_PRIORITY_L2_TAIL = 2,
	RTK_CA_CLS_PRIORITY_L2_3 = 3,
	RTK_CA_CLS_PRIORITY_L2_DOS = 4,			//this is reserve to DOS attack
	RTK_CA_CLS_PRIORITY_L2_STORM = 5,		//this is reserve to STORM control
	RTK_CA_CLS_PRIORITY_L2_HEAD = 6,
	RTK_CA_CLS_PRIORITY_L2_7 = 7,
	RTK_CA_CLS_PRIORITY_L3_0 = 0,			//generic intf low
	RTK_CA_CLS_PRIORITY_L3_1 = 1,			//generic intf middle
	RTK_CA_CLS_PRIORITY_L3_2 = 2,			//generic intf high
	RTK_CA_CLS_PRIORITY_L3_3 = 3,			//5 tuple to 2 tuple flow acc
	RTK_CA_CLS_PRIORITY_L3_4 = 4,			//5 tuple to 2 tuple flow acc
	RTK_CA_CLS_PRIORITY_L3_8 = 8,			//generic intf dos policing
	RTK_CA_CLS_PRIORITY_L3_9 = 9,			//generic intf mc
	RTK_CA_CLS_PRIORITY_L3_TAIL = 10,		//better to higher than CL_RUL_PRIO_L3_INTF_MCAST 9
	RTK_CA_CLS_PRIORITY_MC_MIRROR = 11,		//better to higher than CL_RUL_PRIO_L3_INTF_MCAST 9 and RTK_CA_CLS_PRIORITY_MC_MIRROR
	RTK_CA_CLS_PRIORITY_L3_USER = 12,
	RTK_CA_CLS_PRIORITY_L3_13 = 13,
	RTK_CA_CLS_PRIORITY_L3_HEAD = 14,
	RTK_CA_CLS_PRIORITY_L3_15 = 15,
	RTK_CA_CLS_PRIORITY_MAX,			//group ca rules by ca priority(range L2 CLS 0~7, L3 CLS 0~15). We will rearrange priority by ourself, L2 0~7 for rsv ACL, L3 8~15 for rsv ACL/User.
} rtk_fc_g3_caRulePriority_t;

typedef enum rtk_rg_aclAndCf_reserved_cam_type_e
{
	RTK_ACL_CLS_RUL_INFO_CAM_PKT_LEN,
	RTK_ACL_CLS_RUL_INFO_CAM_SPCL_HDR,
	RTK_ACL_CLS_RUL_INFO_CAM_ETHERTYPE,
	RTK_ACL_CLS_RUL_INFO_CAM_PPP_PROTO,
	RTK_ACL_CLS_RUL_INFO_CAM_L4_SPORT,
	RTK_ACL_CLS_RUL_INFO_CAM_L4_DPORT,
}rtk_rg_aclAndCf_reserved_cam_type_t;

typedef struct rtk_fc_aclAndCf_reserved_cam_info_s{
	rtk_rg_aclAndCf_reserved_cam_type_t cam_type;
	uint32 pkt_len_vec;
	uint32 spec_hdr_vec;
	uint16 l4_sport_vec;
	uint16 l4_dport_vec;
	uint8 ethertype_idx;
	uint8 ppp_proto_idx;
}rtk_fc_aclAndCf_reserved_cam_info_t;

typedef struct rtk_fc_aclAndCf_reserved_hostPoliceSaDa_s{
	int16 ca_cls_index; //stored L2 CLS index (ca sw index) index for RX host policing
	int16 flow_id;
	rtk_mac_t hostMac;
}rtk_fc_aclAndCf_reserved_hostPoliceSaDa_t;
	

typedef struct rtk_fc_g3_cls_entry_s
{
	rtk_enable_t				valid;
	int							ruleType;
	rtk_fc_g3_caRulePriority_t	priority;
	uint32_t 					aal_customize;
} rtk_fc_g3_cls_entry_t;

typedef struct rtk_fc_ca_aal_cls_entry_s
{
	rtk_enable_t				valid;
	int							ruleType;
	rtk_fc_g3_caRulePriority_t	priority;
	uint32_t 					rslt_idx;		//ref: aal_l3_cls.h CLS_RUL_ID; aal_l2_cls.h L2_CLS_RUL_ID;
	uint8						is_l2_rule;
	uint8						profile_id;
	rtk_fc_aclAndCf_reserved_cam_info_t	cam_info;
} rtk_fc_ca_aal_cls_entry_t;
#endif

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
typedef enum rtk_fc_acl_TagIf_bit_e
{
	ACL_TAGIF_RSV0_BIT=(1<<0),
	ACL_TAGIF_RSV1_BIT=(1<<1),
	ACL_TAGIF_PPPoE_8863_8864_BIT=(1<<2),
	ACL_TAGIF_PPPoE_8863_BIT=(1<<3),
	ACL_TAGIF_PPPoE_8864_BIT=(1<<4),
	ACL_TAGIF_OUT_IP4_BIT=(1<<5),
	ACL_TAGIF_IP6_BIT=(1<<6),
	ACL_TAGIF_IN_IP4_BIT=(1<<7),
	ACL_TAGIF_TCP_UDP_BIT=(1<<8),
	ACL_TAGIF_UDP_BIT=(1<<9),
	ACL_TAGIF_TCP_BIT=(1<<10),
	ACL_TAGIF_IGMP_MLD_BIT=(1<<11),
	ACL_TAGIF_ARP_BIT=(1<<12),
	ACL_TAGIF_DSLite_BIT=(1<<13),
	ACL_TAGIF_PPTP_BIT=(1<<14),
	ACL_TAGIF_L2TP_BIT=(1<<15),
}rtk_fc_acl_TagIf_bit_t; //use for H/W acl Framte type and Tags mapping, do not change the order!

typedef enum rtk_rg_aclAndCf_reserved_type_e
{
	/*HEAD PATCH: The lower index number, the higher acl&cf priority*/

	RTK_FC_ACLANDCF_RESERVED_ALL_TRAP,
	RTK_FC_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP,
	RTK_FC_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY,
	RTK_FC_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY,
	RTK_FC_ACLANDCF_RESERVED_PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY,

	RTK_FC_ACLANDCF_RESERVED_HEAD_END,

	/*TAIL PATCH: The lower index number, the lower acl&cf priority*/
	RTK_FC_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT,
	RTK_FC_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP,
	RTK_FC_ACLANDCF_RESERVED_ICMPV6_TRAP,
	RTK_FC_ACLANDCF_RESERVED_ICMPV4_TRAP,
    RTK_FC_ACLANDCF_RESERVED_TAIL_END,

}rtk_rg_aclAndCf_reserved_type_t;


typedef struct rtk_fc_aclAndCf_reserved_hybrid_pptp_func_s{
	uint32 intf_count;
	rtk_mac_t gateway_mac_addr[RTK_FC_TABLESIZE_INTF_SW];
}rtk_fc_aclAndCf_reserved_hybrid_pptp_func_t;

typedef struct rtk_fc_aclAndCf_reserved_assign_priority_s{
	int priority;
}rtk_fc_aclAndCf_reserved_assign_priority_t;

typedef struct rtk_fc_aclAndCf_reserved_global_s
{
	uint32 aclProtectLowerBoundary; //The start index of acl reserve rules in the head for protection, default is 0
	uint32 aclProtectUpperBoundary; //The end index of acl reserve rules in the head for protection
	uint32 aclLowerBoundary; //The End index of acl reserve rules in the head
	uint32 aclUpperBoundary; //The start index of acl reserve rules in the tail
	uint32 aclValidTableSize;	//fc acl size = total size MAX_ACL_ENTRY_SIZE - RT_ACL_RESERVED_NUMBER
	uint32 cfLowerBoundary;  //The start index of cf reserve rules in the head
	uint32 cfUpperBoundary;  //The start index of cf reserve rules in the tail
	uint8 reservedMask[RTK_FC_ACLANDCF_RESERVED_TAIL_END];
	uint8 reservedMap[RT_ACL_RESERVED_TYPE_MAX];

	rtk_fc_aclAndCf_reserved_hybrid_pptp_func_t hybrid_pptp_func;

	rtk_fc_aclAndCf_reserved_assign_priority_t l2tp_ctrl_lcp_assign_prioity;
	rtk_fc_aclAndCf_reserved_assign_priority_t pppoe_lcp_assign_prioity;
	rtk_fc_aclAndCf_reserved_assign_priority_t pptp_gre_ctrl_lcp_assign_prioity;
}rtk_fc_acl_reserved_global_t;
#endif
//+++++++++++++++ ACL Related Structure End +++++++++++++++

typedef struct rtk_fc_portmask_e
{
	rtk_fc_port_mask_t portmask;
}rtk_fc_portmask_t;

typedef struct rtk_fc_flowOrHostmib_counter_s{
	uint32 in_packet_cnt;
	uint64 in_byte_cnt;
	uint32 out_packet_cnt;
	uint64 out_byte_cnt;
}rtk_fc_flowOrHostmib_counter_t;

typedef struct rtk_fc_hostPoliceControl_l2_cls_idx_s
{
	int16 idx_ingress;	//stored L2 CLS index (ca sw index) index for RX (ingress SMAC) host policing, -1 mean n.a.
	int16 idx_egress;	//stored L2 CLS index (ca sw index) index for TX (ingress DMAC) host policing, -1 mean n.a.
}rtk_fc_hostPoliceControl_l2_cls_idx_t;

typedef struct rtk_fc_hostPoliceInfo_s
{
	rt_rate_host_policer_control_t hostPoliceControl;
	rt_rate_host_policer_mib_t swHostPoliceCounter[NR_CPUS]; //sw counter
	int ingress_limit_meter_hwIndex;
    int egress_limit_meter_hwIndex;
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	rt_rate_host_policer_mib_t hwHostPoliceCounter; //hw couter (hw is read clear, thus store into sw)
	int32 loggingRx_policerIdx:10; //stored flow policer index for RX counting the of MAC address, -1 mean n.a.
	int32 loggingTx_policerIdx:10; //stored flow policer index for TX counting the of MAC address, -1 mean n.a.
#endif
	int32 ingress_limit_by_swShaper:1;	//if ingress rate limit by sw shaper
	int32 egress_limit_by_swShaper:1;	//if egress rate limit by sw shaper
	rtk_fc_hostPoliceControl_l2_cls_idx_t l2ClsIndex;	// loopback mode: the related L2 CLS indexs of host policing for the MAC address
}rtk_fc_hostPoliceInfo_t;


#if defined(CONFIG_RTK_L34_G3_PLATFORM)
typedef struct rtk_fc_stormCtrl_s
{
	rt_enable_t state;
	uint32 l3PolicerIdx;
}rtk_fc_stormCtrl_t;
#endif

/* the same with rtk_rg_asic_dualHdrInfo_t */
typedef enum rtk_fc_fbDualHdrInfoType_e
{
	RTK_FC_FB_DUALHDR_GRESEQ = 0, //FB_DUALHDR_GRESEQ
    RTK_FC_FB_DUALHDR_GREACK, //FB_DUALHDR_GREACK
	RTK_FC_FB_DUALHDR_OUTER_IPV4ID, //FB_DUALHDR_OUTER_IPV4ID
	RTK_FC_FB_DUALHDR_END, //FB_DUALHDR_END
}rtk_fc_fbDualHdrInfoType_t;

typedef struct rtk_fc_dualhdr_greIpv4idInfo_s
{
	uint32 greSeq;
	uint32 greAck;
	uint32 fbIpId;
}rtk_fc_dualhdr_greIpv4idInfo_t;


// VLAN

// Table Size
#ifdef CONFIG_RTK_L34_XPON_PLATFORM
#define RTK_FC_TABLESIZE		RTL9607C_TABLESIZE_FLOWSRAM
#endif


typedef enum rtk_fc_trafilter_stage_e
{
	RTK_FC_TRACEFILTER_EGRESS = 0,
	RTK_FC_TRACEFILTER_INGRESS = 1,
	RTK_FC_TRACEFILTER_INGRESS_SHORTCUT_EARLYCHECK = 2,
} rtk_fc_trafilter_stage_t;

typedef enum rtk_fc_house_keep_select_e
{
	RTK_FC_HOUSEKEEP_LUT_SELECT,
	RTK_FC_HOUSEKEEP_FLOW_SELECT,
	RTK_FC_HOUSEKEEP_DRV_EVENT_REC,
	RTK_FC_HOUSEKEEP_MAX_SELECT,
} rtk_fc_house_keep_select_t;

/* flow-based check state */
typedef enum rtk_fc_flowCheckType_s
{
	FB_FLOW_CHECK_PATH12_TOS = 0,
	FB_FLOW_CHECK_PATH34_TOS,
	FB_FLOW_CHECK_PATH5_TOS,
	FB_FLOW_CHECK_PATH6_TOS,
	FB_FLOW_CHECK_PATH12_PROTOCOL,
	FB_FLOW_CHECK_PATH6_PROTOCOL,
	FB_FLOW_CHECK_PATH12_SPA,
	FB_FLOW_CHECK_PATH12_STREAM_IDX,	
	FB_FLOW_CHECK_PATH345_STREAM_IDX,
	FB_FLOW_CHECK_PATH6_SMAC_IDX,
	FB_FLOW_CHECK_PATH6_DMAC_IDX,
	FB_FLOW_CHECK_PATH6_SIP,
	FB_FLOW_CHECK_PATH6_DIP,
	FB_FLOW_CHECK_PATH6_SPORT,
	FB_FLOW_CHECK_PATH6_DPORT,
	FB_FLOW_CHECK_PATH6_GRE_CALL_ID,
	FB_FLOW_CHECK_PATH6_L2TP_TUNNEL_ID,
	FB_FLOW_CHECK_PATH6_L2TP_SID,
	FB_FLOW_CHECK_END
}rtk_fc_flowCheckType_t;

typedef enum rtk_fc_flowType_e{
	RTK_FC_FLOW_TYPE_UNKNOWN			= 0,
	RTK_FC_FLOW_TYPE_L2Unicast		= 1,
	RTK_FC_FLOW_TYPE_L34NAPT			= 2,
	RTK_FC_FLOW_TYPE_IPv4Multicast	= 3,
	RTK_FC_FLOW_TYPE_IPv6Unicast		= 4,
	RTK_FC_FLOW_TYPE_IPv6Multicast	= 5,
	RTK_FC_FLOW_TYPE_DSLITE			= 6,
	RTK_FC_FLOW_TYPE_DSLITE_4OVER6	= 7,
	RTK_FC_FLOW_TYPE_MAX,
}rtk_fc_flowType_t;

typedef enum rtk_fc_flow_direction_e{
	RTK_FC_FLOW_DIRECTION_NA			= 0,
	RTK_FC_FLOW_DIRECTION_UPSTREAM		= 1,
	RTK_FC_FLOW_DIRECTION_DOWNSTREAM	= 2,
	RTK_FC_FLOW_DIRECTION_LANBRIDGE	= 3,
	RTK_FC_FLOW_DIRECTION_WAN_TO_WAN	= 4,
	RTK_FC_FLOW_DIRECTION_END,
}rtk_fc_flow_direction_t;

typedef enum rtk_fc_flow_dual_direction_e{
	RTK_FC_FLOW_DUAL_DIRECTION_UPSTREAM_EGR			= 0,	//packet upstream to wan (FC egress)
	RTK_FC_FLOW_DUAL_DIRECTION_DOWNSTREAM_IGR		= 1,	//packet downstream from wan (FC ingress)
}rtk_fc_flow_dual_direction_t;


typedef enum rtk_fc_forwardingType_e
{
	RTK_FC_FWDTYPE_BRIDGE = 0,
	RTK_FC_FWDTYPE_ROUTING_BASED,				// upper is bridge forwarding without smac trans; below is routing based forwarding.
	RTK_FC_FWDTYPE_ROUTING = RTK_FC_FWDTYPE_ROUTING_BASED,
	RTK_FC_FWDTYPE_NAPT,
	RTK_FC_FWDTYPE_NATLOOPBACK,					// ipv4 only, could be any direction in rtk_fc_flow_direction_t
	RTK_FC_FWDTYPE_END,
}rtk_fc_forwardingType_t;

enum rtk_fc_flow_mode_e{
	RTK_FC_FLOW_MODE_UNIQUE_UPSTREAM		= 1,
	RTK_FC_FLOW_MODE_UNIQUE_DOWNSTREAM	= 2,
	RTK_FC_FLOW_MODE_BOTH					= 3,
};

enum rtk_fc_flow_learing_act_e{
	RTK_FC_FLOW_LEARNING_ACT_INGRESS = 1,
	RTK_FC_FLOW_LEARNING_ACT_EGRESS = 2,
	RTK_FC_FLOW_LEARNING_ACT_MAX,
};

typedef enum rtk_fc_flowEntSearchReturn_e
{
	RTK_FC_FLOWENT_SEARCH_FAIL,
	RTK_FC_FLOWENT_SEARCH_HW,
	RTK_FC_FLOWENT_SEARCH_SW,
	RTK_FC_FLOWENT_SEARCH_END,
}rtk_fc_flowEntSearchReturn_t;

typedef enum rtk_fc_skbmarkTarget_e
{
	RTK_FC_SKBMARK_QID,
	RTK_FC_SKBMARK_STREAMID,
	RTK_FC_SKBMARK_STREAMID_EN,
	RTK_FC_SKBMARK_SWSHAPER_EN,
	RTK_FC_SKBMARK_METERIDX,
	RTK_FC_SKBMARK_METERIDX_EN,
	RTK_FC_SKBMARK_FWDBYPS,
	RTK_FC_SKBMARK_MIBIDX,
	RTK_FC_SKBMARK_MIBIDX_EN,
	RTK_FC_SKBMARK_SKIPFCFUNC,
	RTK_FC_SKBMARK_CVLAN_ID,
	RTK_FC_SKBMARK_CVLAN_PRI,
	RTK_FC_SKBMARK_CVLAN_ACTION,
	RTK_FC_SKBMARK_SVLAN_ID,
	RTK_FC_SKBMARK_SVLAN_PRI,
	RTK_FC_SKBMARK_SVLAN_ACTION,
	RTK_FC_SKBMARK_SVLAN_TPID,
	RTK_FC_SKBMARK_WANDSLOOPBACK_EN,
	RTK_FC_SKBMARK_SWINTFIDX_ASSIGN_EN,
	RTK_FC_SKBMARK_SWINTFIDX_ASSIGN_INOUT,
	RTK_FC_SKBMARK_SWINTFIDX_ASSIGN_IDX,
	RTK_FC_SKBMARK_SKIP_FC_ALG_CHECK,
	RTK_FC_SKBMARK_FLOW_CACHE_MIB_EN,
	RTK_FC_SKBMARK_PSFLOODFWDACC,
	RTK_FC_SKBMARK_MAPET_FMR,
	RTK_FC_SKBMARK_END
}rtk_fc_skbmarkTarget_t;

#define DUAL_MAPT_HASH_MASK	(0xffff0000)
#define DUAL_XLAT_HASH_MASK	(0xffff)

//please sync with name_of_dualHdrType
typedef enum rtk_fc_dualHdrtype_e
{
	RTK_FC_DUALTYPE_NONE=0,		// (not dual header) or (dual but bridging)
	RTK_FC_DUALTYPE_IP4INIP6,	/* MAPE & DSLITE (not include passthough packet)/lw4o6 */
	RTK_FC_DUALTYPE_PPTP,
	RTK_FC_DUALTYPE_L2TP,
	RTK_FC_DUALTYPE_IPSEC,		// L2TP+IPSec passthrough routing/nat. packet format is single header because the inner header is encapsulated in ESP hdr
	RTK_FC_DUALTYPE_GRE_ETH_BR, // For GRE pakcet and protocol type is Trans Ether Bridging (0x6558)
	RTK_FC_DUALTYPE_VXLAN,
	RTK_FC_DUALTYPE_6RD,
	RTK_FC_DUALTYPE_MAPT,
	RTK_FC_DUALTYPE_XLAT464,
	RTK_FC_DUALTYPE_END,
}rtk_fc_dualHdrtype_t;

typedef enum rtk_fc_l2Dual_action_e
{
	RTK_FC_L2DUAL_ACT_REMOVE = 0,
	RTK_FC_L2DUAL_ACT_ADD,
}rtk_fc_l2Dual_action_t;

typedef enum rtk_fc_dev_type_e
{
	RTK_FC_DEV_TYPE_INGRESS = 0,
	RTK_FC_DEV_TYPE_EGRESS,
}rtk_fc_dev_type_t;

typedef enum rtk_fc_l2Dual_type_e
{
	RTK_FC_L2DUAL_TYPE_NONE = 0,
	RTK_FC_L2DUAL_TYPE_VXLAN,		// (not dual header) or (dual but bridging),
	RTK_FC_L2DUAL_END,
}rtk_fc_l2Dual_type_t;

typedef enum rtk_fc_tcp_lrn_state_e{
	TCP_LEARN_SYN_SENT,
	TCP_LEARN_SYN_RECV,
	TCP_LEARN_ESTABLISHED,
	TCP_LEARN_END,
}rtk_fc_tcp_lrn_state_t;

typedef struct rtk_fc_ipFrag_info_s
{
	uint32	src_ip;
	uint32	dst_ip;
	uint16	frag_id;
	uint8	protocol;
}rtk_fc_ipFrag_info_t;

typedef struct rtk_fc_ipFrag_linkList_s
{
	uint8	valid:1;
	uint8	hash_idx_valid:1;
	uint16	src_port;
	uint16	dst_port;
	uint16 	rcv_data_len;
	uint16	total_data_len;
	uint16	tmp_data_len;
	uint32	hash_idx;
	rtk_fc_ipFrag_info_t frag_info;

	struct list_head entry;
	rtk_fc_timer_list_t *timer;
}rtk_fc_ipFrag_linkList_t;

typedef struct rtk_fc_negativeIpFrag_linkList_s
{
	uint8	valid;
	struct list_head entry;
	rtk_fc_timer_list_t *timer;
	rtk_fc_ipFrag_info_t frag_info;
}rtk_fc_negativeIpFrag_linkList_t;


#if defined(CONFIG_RTK_L34_G3_PLATFORM)
typedef struct rtk_fc_mcEngine_info_s
{
	uint32 valid:1;
	uint32 isIpv6:1;
	uint32 hwAcc:1; 			//1: this MC group has added twice and start to hw acceleration; 0: this MC group has added just once, the following data packet will be trap
	uint32 groupAddress[4]; 	//v4 -> groupAddress[0]=hostEndian	v6 -> networkEndian
	uint32 sourceAddress[4];	//v4 -> groupAddress[0]=hostEndian	v6 -> networkEndian
	uint32 l2mcgid;
	uint32 p3Pmsk;//Store current MC member port in Hw, used to change members
	rtk_fc_wlan_devmask_t p3Wlanmsk;
	uint32 p4Pmsk;//Store current MC member port in Hw, used to change members
	rtk_fc_wlan_devmask_t p4Wlanmsk;
	uint16 cvid;	//ingress cvid  ==>untag == CA_UINT16_INVALID
	uint16 svid;	//ingress svid
	uint32 mcHwRefCount;
	unsigned long last_jiffies; //used for timeout mechnism of 0 reference count entry
}rtk_fc_mcEngine_info_t;

#endif

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
typedef struct rtk_fc_wifi_flow_crtl_threshold_s
{
	uint16 wlan0_flow_ctrl_on_threshold_mbps; //on threshold for wlan0 (unit: mbps)
	uint16 wlan0_flow_ctrl_off_threshold_mbps; //off threshold for wlan0 (unit: mbps)
	uint16 wlan1_flow_ctrl_on_threshold_mbps; //on threshold for wlan1 (unit: mbps)
	uint16 wlan1_flow_ctrl_off_threshold_mbps; //off threshold for wlan1 (unit: mbps)
}rtk_fc_wifi_flow_crtl_threshold_t;
#endif

typedef struct rtk_fc_pptpInfo_s
{
	uint16 grecallid;
	uint16 outerFlowReady:1;		// path6 is valid and pattern is matched
	//uint16 greseqTagif:1;
	//uint16 greackTagif:1;
	uint32 greseq;
	uint32 greack;
	uint32 *pGRESeq;
	uint32 *pGREAck;
	u8 *pCallId;
}rtk_fc_pptpInfo_t;

typedef struct rtk_fc_l2tpInfo_s
{
	uint16 tunnelid;
	uint16 sessionid;
}rtk_fc_l2tpInfo_t;

typedef struct rtk_fc_tunnelInfo_s
{
	rtk_fc_enum_pktHdrTagif_t tunnelTag;
	union{
		rtk_fc_pptpInfo_t pptp;
		rtk_fc_l2tpInfo_t l2tp;
	};
	//uint32 ingressWanIntfIdx;
	rtk_fc_enum_pppCompressMode_t pppMode;
}rtk_fc_tunnelInfo_t;

typedef struct rtk_fc_netif_greEthBr_info_s
{
	uint8 outerTag_len;
	uint8 outer_ipversion:1;	//0:v4, 1:v6
	union{
		struct in6_addr localV6Ip;
		uint32 localIp;
	};
	union{
		struct in6_addr remoteV6Ip;
		uint32 remoteIp;
	};
}rtk_fc_netif_greEthBr_info_t;

typedef struct rtk_fc_netif_psGre_Info_s
{
	uint16 rx_server_seq_start_tracking:1;
	uint32 rx_server_seq; 					// seq recv from server
	uint32 tx_greseq;
	uint32 tx_greack;
	uint16 peer_call_id;		//peer call_id, for upstream
	uint16 call_id;				//self call_id, for downstream
	uint8 outer_ipversion:1;	//0:v4, 1:v6
	union{
		struct in6_addr localV6Ip;
		uint32 localIp;
	};
	union{
		struct in6_addr remoteV6Ip;
		uint32 remoteIp;
	};
}rtk_fc_netif_psGre_Info_t;

typedef struct rtk_fc_netif_l2tp_Info_s
{
	uint16 peer_tunnel_id;	// for upstream
	uint16 peer_session_id;	// for upstream
	uint16 tunnel_id;		// for downstream
	uint16 session_id;		// for downstream
	uint8 outer_ipversion:1;	//0:v4, 1:v6
	union{
		struct in6_addr localV6Ip;
		uint32 localIp;
	};
	union{
		struct in6_addr remoteV6Ip;
		uint32 remoteIp;
	};
	uint16 local_l4port;
	uint16 remote_l4port;
}rtk_fc_netif_l2tp_Info_t;

typedef struct rtk_fc_netif_mapt_Info_s
{
	uint8 hwDraftVer:1;
	uint8 hwPrefxLen:7;
	struct in6_addr localV6Ip;
	struct in6_addr remoteV6Ip;
}rtk_fc_netif_mapt_Info_t;

typedef struct rtk_fc_netif_xlat464_Info_s
{
	struct in6_addr Ipv6Addr_remote;
	struct in6_addr Ipv6Addr_local;
	uint8 Ipv6Addr_remotePreifx ; 	//rtk_fc_dual_ipv6HashMask_t
	uint8 Ipv6Addr_localPreifx ; 	//rtk_fc_dual_ipv6HashMask_t
}rtk_fc_netif_xlat464_Info_t;

typedef struct rtk_fc_psFragIPID_s
{
	uint8 valid:1;
	uint16 oriIPID;
	uint16 newIPID;
}rtk_fc_psFragIPID_t;

typedef struct rtk_fc_remarkDecision_s
{
	uint32  fwdByPS: RTK_FC_RMK_FWDBYPS_LEN_DEF;
	uint32 outputQid: RTK_FC_RMK_QID_LEN_DEF;
	uint32 wanDsLoopback_en: RTK_FC_RMK_WANDSLOOPBACK_EN_LEN_DEF;	// for downstream packet, this bit indicate wan_downstream_loopback_enable)
	uint32 streamId_en: RTK_FC_RMK_STREAMID_EN_LEN_DEF;
	uint32 streamId: RTK_FC_RMK_STREAMID_LEN_DEF;
	uint32 swShaper_en: RTK_FC_RMK_SWSHAPER_EN_LEN_DEF;
	uint32 meterIdx_en: RTK_FC_RMK_METERIDX_EN_LEN_DEF;
	uint32 meterIdx: RTK_FC_RMK_METERIDX_LEN_DEF;
	uint32 mibIdx_en: RTK_FC_RMK_MIBIDX_EN_LEN_DEF;
	uint32 mibIdx: RTK_FC_RMK_METERIDX_LEN_DEF;
	uint32 cvlan_trans:1;
	uint32 cvlantag:1;		// valid if cvlan_trans=TRUE
	uint32 cvlanid:12;			// valid if cvlan_trans=TRUE
	uint32 cvlan_action_from_skbmark:RTK_FC_RMK_CVLAN_ACTION_LEN_DEF;
	uint32 svlan_action_from_skbmark:RTK_FC_RMK_SVLAN_ACTION_LEN_DEF;
	uint32 svlan_tpid_from_skbmark:RTK_FC_RMK_SVLAN_TPID_LEN_DEF;
	uint32 swIntfIdx_assign_en:RTK_FC_RMK_SWINTFIDX_ASSIGN_EN_LEN_DEF;
	uint32 swIntfIdx_assign_inOut:RTK_FC_RMK_SWINTFIDX_ASSIGN_INOUT_LEN_DEF;
	uint32 swIntfIdx_assign_idx:RTK_FC_RMK_SWINTFIDX_ASSIGN_IDX_LEN_DEF;
	uint32 skip_fc_alg_check:RTK_FC_RMK_SKIP_FC_ALG_CHECK_LEN_DEF;
	uint32 flow_cache_mib_en: RTK_FC_RMK_FLOW_CACHE_MIB_EN_LEN_DEF;
	uint32 psFloodfwdAcc:RTK_FC_RMK_PSFLOODFWDACC_LEN_DEF;
	uint32 mapet_fmr:RTK_FC_RMK_MAPET_FMR_LEN_DEF;
}rtk_fc_remarkDecision_t;

typedef enum rtk_fc_skbMark_vlan_action_e
{
	RTK_FC_SKBMARK_VLAN_KEEP_ORIGINAL = 0,		// (not dual header) or (dual but bridging)
	RTK_FC_SKBMARK_VLAN_UNTAG,
	RTK_FC_SKBMARK_VLAN_REMARK,
	RTK_FC_SKBMARK_VLAN_MAX,	
}rtk_fc_skbMark_vlan_action_t;
	
typedef struct rtk_fc_pptp_gre_header_s {
	__be16 flags;
	__be16 protocol;
	__be16 payload_len;
	__be16 call_id;
	__be32 seq;
}rtk_fc_pptp_gre_header_t;

typedef struct rtk_fc_l2tp_header_s {
	__be16 pack_type;
	__be16 len;
	__be16 tunnul_id;
	__be16 session_id;
}rtk_fc_l2tp_header_t;
typedef struct rtk_fc_ppp_header_s {
	__u8 address;
	__u8 control;
	__be16 protocol;
}rtk_fc_ppp_header_t;

typedef struct rtk_fc_igrDummyData_s
{
	uint8 data[MAX_AUTOEXT_PKT_SIZE];	// dummy packet data buffer
}rtk_fc_igrDummyData_t;


/*
START:
MC_STATE_NEED_UPDATE_DATA_BUF ->MC_STATE_NEED_UPDATE_FWDINFO_FIRST_ADD -->

MC_STATE_NEED_UPDATE_FWDINFO --> MC_STATE_UPDATE_TO_HARDWAEW_DONE -->
			^				|										|
			|				|										V
			|				 --> MC_STATE_CANT_TO_HARDWAEW_DONE   -->
			|														|
			|														V
			<------------------------------------------------------<-
*/

typedef enum rtk_fc_multicast_state_e
{
	MC_STATE_NEED_UPDATE_DATA_BUF= 0,
	MC_STATE_NEED_UPDATE_FWDINFO_FIRST_ADD,
	MC_STATE_NEED_UPDATE_FWDINFO,
	MC_STATE_UPDATE_TO_HARDWAEW_DONE,		//action <=2
	MC_STATE_CANT_TO_HARDWAEW_DONE,			//action  >2 or other reason
	MC_STATE_END ,
}rtk_fc_multicast_state_t;


typedef struct rtk_fc_mcBuff_autoUpdate_s
{
	int8 forwardStatus;				//reference to rtk_fc_multicast_state_t
	rtk_fc_igrDummyData_t dataBuf;
	uint32 skb_mark;
	uint64 skb_mark2;
	rtk_fc_ingress_data_t fcIngressData;

	uint32  hw_path3Pmsk;				//ref rtk_rg_port_idx_e
	rtk_fc_wlan_devmask_t hw_path3WlanDevmsk;
	uint32  hw_path4Pmsk;
	rtk_fc_wlan_devmask_t hw_path4WlanDevmsk;

	rtk_fc_timer_list_t *updateTimer;
}mcBuff_autoUpdate_s;

typedef struct rtk_fc_table_mcGroupTbl_s
{
	uint32	mcGroup[4];
	uint16	groupRefCnt;
	int32	hwIdx;					//9607C Series is lut idx / G3 Series is hash Idx
	int32	hwIdx2;					//9607C Series is FAIL / G3 Series 8277C is lutIdx
	int8  	valid:1;
	uint8	isIpv6:1;	
} rtk_fc_table_mcGroupTbl_t;



#define ABSTR_BC_PREFIX_NUM 0
#define ABSTR_BC_PREFIX_MASK 0x1
#define ABSTR_BC_PREFIX 0x1
#define ABSTR_SWFLOW_ACTION_SIZE			32
#define ABSTR_SWFLOW_ACTION_RESVED_SIZE		3
#define ABSTR_SWFLOW_PATTERN_SIZE			32
#define ABSTR_SWFLOW_TYPE_SIZE_BITS			3
#define ABSTR_SWFLOW_TYPE_SIZE				(1<<ABSTR_SWFLOW_TYPE_SIZE_BITS)
#define ABSTR_SWFLOW_PATTERN_RESVED_SIZE	1


/* 
====if chnage also check=====
rtk_fc_abstrSwFlowModule_init
patternSizeMapping
rtk_fc_abstrSwFlowPatternGenBySkb
rtk_fc_proc_abstrSwFlowTemplateType_set
rtk_fc_parseAbstrSwPattenField
dump_abstrSwflowType_table
dump_abstrSwFlowPatten_table
rtk_fc_abstrSwFlowPattenField_entry_t
*/
#define TYPETOMSK(x)	(1<<x)
typedef enum rtk_fc_swFlowPatternFlag_e
{
	FLOW_PHY_LSPID			=0,
	FLOW_PHY_STREAMID		,
	FLOW_L2_DMAC			,
	FLOW_L2_SMAC			,
	FLOW_L2_ETH				,	
	FLOW_L2_CVLANTAG		,
	FLOW_L2_CVLAN_TPID		,
	FLOW_L2_CVLAN			,
	FLOW_L2_CPRI			,
	FLOW_L2_CDEI			,
	FLOW_L2_SVLAN_TPID		,
	FLOW_L2_SVLANTAG		,
	FLOW_L2_SVLAN			,
	FLOW_L2_SPRI			,
	FLOW_L2_SDEI			,	
	FLOW_L2_PPPOETAG		,
	FLOW_L2_PPPOESID		,
	FLOW_L3_IPVER46			,
	FLOW_L3_DIP				,
	FLOW_L3_SIP				,
	FLOW_L3_TOS				,
	FLOW_L4_PROTO			,
	FLOW_L4_DPORT			,
	FLOW_L4_SPORT			,
	FLOW_PATTERN_MAX,
} rtk_fc_swFlowPatternFlag_t;



typedef struct rtk_fc_AbstrSwFlowType_entry_s
{
	uint8	valid;
	uint8	patternSize;
	uint32	patternFlagMsk;
}rtk_fc_abstrSwFlowType_entry_t;


//total size uint8[ABSTR_SWFLOW_PATTERN_SIZE]
typedef struct rtk_fc_abstrSwFlowPattern_entry_s
{
	union
	{
		struct
		{
			uint8 flowtype:ABSTR_SWFLOW_TYPE_SIZE_BITS; //3
			uint8 ctagif:1;				//untag:0 tag:1
			uint8 stagif:1;				//untag:0 tag:1
			uint8 pppoetagif:1;			//untag:0 tag:1
			uint8 isIpv6:1;				//ipv4:0 ipv6:1
			uint8 resved:1;
		}bits;
		uint8 uint8bits[ABSTR_SWFLOW_PATTERN_RESVED_SIZE];
	};
	uint8 swPattern[ABSTR_SWFLOW_PATTERN_SIZE-ABSTR_SWFLOW_PATTERN_RESVED_SIZE];

}rtk_fc_abstrSwFlowPattern_entry_t;

//#define SWFLOW_EGACT_NOP 		0
#define SWFLOW_EGACT_UNTAG		0
#define SWFLOW_EGACT_TAG		1
//total size uint8[ABSTR_SWFLOW_ACTION_SIZE]


#define FC_IP_DSCP_MASK 0xFC
#define FC_IP_ECN_MASK  0x03
#define FC_IP_DSCP_SHIFT 2

typedef struct vlan_hdr_format_s {
	__be16	tpid;
	__be16	vlan_TCI;
}vlan_hdr_format_t;


typedef struct rtk_fc_AbstrSwFlowPattenField_entry_s
{
	uint8	*pDmac;
	uint8	*pSmac;
	uint16	*pEth;
	uint16	*pSvlanTPID;
	uint16	*pSvlanTCI;		//Vpri:3 CFI:1 VLAN:12 VLAN_PRIO_MASK/VLAN_CFI_MASK/VLAN_VID_MASK
	uint16	*pCvlanTPID;
	uint16	*pCvlanTCI;		//Vpri:3 CFI:1 VLAN:12 VLAN_PRIO_MASK/VLAN_CFI_MASK/VLAN_VID_MASK
	uint16	*pPppoeSid;
	uint32	*pDip;
	uint32	*pSip;
	uint8	*pTos;			//dscp:6 ecn:2 FC_IP_DSCP_MASK/FC_IP_ECN_MASK
	uint16	*pL4Dport;
	uint16	*pL4Sport;
	uint8	*pUserPri;
	uint8	*pPonStreaId;
	uint8	*pLspid;
	uint8	*pL4Porto;
}rtk_fc_abstrSwFlowPattenField_entry_t;


typedef struct rtk_fc_abstrSwFlowActionField_entry_s
{
	uint8	*pDmac;
	uint8	*pSmac;
	uint16	*pSvlanTpid;
	uint16	*pSvlanTCI;		//Vpri:3 CFI:1 VLAN:12 VLAN_PRIO_MASK/VLAN_CFI_MASK/VLAN_VID_MASK
	uint16	*pCvlanTpid;
	uint16	*pCvlanTCI;		//Vpri:3 CFI:1 VLAN:12 VLAN_PRIO_MASK/VLAN_CFI_MASK/VLAN_VID_MASK
	uint16	*pPppoeSid;
	uint32	*pDip;
	uint32	*pSip;
	uint8	*pTos;			//dscp:6 ecn:2 FC_IP_DSCP_MASK/FC_IP_ECN_MASK
	uint16	*pL4Dport;
	uint16	*pL4Sport;
	uint8	*pUserPri;
	uint8	*pPonStreaId;
}rtk_fc_abstrSwFlowActionField_entry_t;


typedef struct rtk_fc_abstrSwFlowAction_entry_s
{
	union
	{
		struct
		{
			uint32 dmacTrans:1;		//smac trans(insert 6byte)
			uint32 smacTrans:1;		//smac trans(insert 6byte)			
			uint32 stagCmd:1;		//0:untag 1:egress Stag(insert 2byte)					
			uint32 ctagCmd:1;		//0:untag 1:egress Ctag(insert 2byte)
			uint32 pppoeCmd:1;		//0:untag 1:egress with PPPoESessionID(insert 2byte)
			uint32 dipTrans:1;		//0:NOP 1:egress with dip (insert 4/16byte)
			uint32 sipTrans:1;		//0:NOP 1:egress with sip (insert 4/16byte)

			uint32 dscpCmd:1;		//0:NOP 1:egress with dscp (insert 1byte)  always assign to one
			uint32 ecnCmd:1;		//0:NOP 1:egress with ecn
			uint32 l4dportCmd:1;	//0:NOP 1:egress with dport (insert 2byte)
			uint32 l4sportCmd:1;	//0:NOP 1:egress with sport (insert 2byte)
			uint32 userPriCmd:1;	//0:NOP 1:egress with priority (insert 1 byte)
			uint32 ponStreamIdCmd:1;//0:NOP 1:egress with Pon StreamId (insert 1byte)
			uint32 resved:3;
		}bits;
		uint8 uint8bits[ABSTR_SWFLOW_ACTION_RESVED_SIZE];
	};

	uint8 swAction[ABSTR_SWFLOW_ACTION_SIZE-ABSTR_SWFLOW_ACTION_RESVED_SIZE];	//store modify information dmac/smac/vlan/ip/ ..etc

}rtk_fc_abstrSwFlowAction_entry_t;

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
typedef struct rtk_fc_swFlowLdpid_G3_portHwPrivate_entry_s
{
	uint32 hwFibAcc	:1;
	uint32 hwFibIdx :9;			//no-used now
	uint32 hwFibHashAcc	:1;
	uint32 hwFibHashAcc_tmp:1;
	uint32 swMacIdEn:1;
	uint32 swMacId:6;
	uint32 swMacIdEn_tmp:1;
	uint32 swMacId_tmp:6;
	uint32 resved	:6;

	uint32 hwFibHashIdx:16;		//for loopback hashIdx maintain
	uint32 hwFibHashIdx_tmp:16;		//for loopback hashIdx maintain
}rtk_fc_swFlowLdpid_G3_portHwPrivate_entry_t;

typedef struct rtk_fc_swFlowActionList_G3_actHwPrivate_entry_s
{
	uint32 resved:32;
}rtk_fc_swFlowActionList_G3_actHwPrivate_entry_t;

typedef struct rtk_fc_swFlowList_G3_keyHwPrivate_entry_s
{
	uint32 swMcgid	:16;
	uint32 hwMcgid	:16;
	uint32 hwHashIdx:16;

	uint32 hwacc	:1;
	uint32 resved	:15;
}rtk_fc_swFlowList_G3_keyHwPrivate_entry_t;


#elif defined(CONFIG_RTK_L34_XPON_PLATFORM)
typedef struct rtk_fc_swFlowLdpid_XPON_portHwPrivate_entry_s
{
	uint32 resved:32;
}rtk_fc_swFlowLdpid_XPON_portHwPrivate_entry_t;

typedef struct rtk_fc_swFlowActionList_XPON_actHwPrivate_entry_s
{
	uint32 hwIdx	:16;
	uint32 isPath24	:1;	//path3 or path4
	uint32 hwacc	:1;
	uint32 resved	:14;
}rtk_fc_swFlowActionList_XPON_actHwPrivate_entry_t;

typedef struct rtk_fc_swFlowList_XPON_keyHwPrivate_entry_s
{
	uint32 hwacc		:1;
	uint32 flowP13hwIdx	:16;
	uint32 resved		:15;
}rtk_fc_swFlowList_XPON_keyHwPrivate_entry_t;


#endif
#define PORT_STATUS_UPDATE_DONE		0
#define PORT_STATUS_SHOULD_BE_ADD		1
#define PORT_STATUS_SHOULD_BE_DELETE	2

typedef struct rtk_fc_abstrSwFlowLdpid_entry_s
{
	uint32 flowLdpid:8;				//if isWlan flowLdpid referenct to rtk_fc_wlan_devidx_t
	uint32 devIfIdx:8;				//for multicast using
	uint32 isWlan:1;				//if isWlan flowLdpid referenct to rtk_fc_wlan_devidx_t
	union
	{	
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
		rtk_fc_swFlowLdpid_G3_portHwPrivate_entry_t g3PortPriv;
#else
		rtk_fc_swFlowLdpid_XPON_portHwPrivate_entry_t xponPortPriv;
#endif		
		uint32 portHwPrivate;						//point to rtk_fc_swFlowLdpid_XPON_portHwPrivate_entry_t/rtk_fc_swFlowLdpid_G3_portHwPrivate_entry_t
	};
	struct list_head ldpidList;
}rtk_fc_abstrSwFlowLdpid_entry_t;


typedef struct rtk_fc_abstrSwFlowActionList_entry_s
{
	rtk_fc_abstrSwFlowAction_entry_t swFlowAction;
	struct list_head swFlowActionList;			//point to rtk_fc_abstrSwFlowActionList_entry_t  list for multicast action
	struct list_head ldpidListHdr;				//point to rtk_fc_abstrSwFlowLdpid_entry_t		list for portmask
	union
	{
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
		rtk_fc_swFlowActionList_G3_actHwPrivate_entry_t g3ActPriv;
#else
		rtk_fc_swFlowActionList_XPON_actHwPrivate_entry_t xponActPriv;
#endif
		uint32 actHwPrivate;
	};
	uint32 igrIntf:6;
	uint32 egrIntf:6;
	uint32 flowMibEn:1;		//todo:move to swFlowAction latter
	uint32 flowMibIdx:6;
}rtk_fc_abstrSwFlowActionList_entry_t;

typedef struct rtk_fc_abstrSwFlowList_entry_s
{
	uint32 swFlowIdx;					//point to self fc_db.flowTbl[swFlowIdx]
	union
	{
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
		rtk_fc_swFlowList_G3_keyHwPrivate_entry_t g3KeyHwPriv;
#else
		rtk_fc_swFlowList_XPON_keyHwPrivate_entry_t xponKeyHwPriv;
#endif
		uint32 keyHwPrivate;
	};	
	rtk_fc_abstrSwFlowPattern_entry_t swFlowKey;
	uint32 copy2cpu:1;
	struct list_head swFlowActionHdr;			//point to rtk_fc_abstrSwFlowActionList_entry_t
}rtk_fc_abstrSwFlowList_entry_t;


typedef struct rtk_fc_mc_dummypktBuff_s
{
	rtk_fc_igrDummyData_t dataBuf;
	uint32 skb_mark;
	uint64 skb_mark2;
	rtk_fc_ingress_data_t fcIngressData;
}mc_dummypktBuff_s;


typedef struct rtk_fc_mcExtraSwFlowIdx_entry_s
{
	uint32 valid:1;
	uint32 isIpv6:1;
	//uint32 entryIdx:14;				//a constant point this entry index
	uint32 dummySyncTimes:3;		//if mcConfig and abstr-flow PortCnt not sync send dummyPacket and dummySyncTimes++
	uint32 isGrpDftEntry:1;			//default forward flow
	uint32 swFlowIdx;				//point to fc_db.flowTbl

	//for fastIdx search
	uint32 multicastAddress[4];		//extra for ipv6 info
	uint32 sourceAddress[4];		//extra for ipv6 info
	int16  ingressCvid;				//-1 UNTAG
	int16  ingressSvid;				//-1 UNTAG
	mc_dummypktBuff_s mcDataBuf;
	struct list_head entry;
}rtk_fc_mcExtraSwFlowIdx_entry_t;

typedef enum rtk_fc_multicast_flush_e
{
	MC_FLWO_FLUSH_ALL= 0,
	MC_FLWO_FLUSH_BY_GROUP= 1,
	MC_FLWO_FLUSH_BY_DMAC_TRANS_ACTION= 2,
	MC_FLWO_FLUSH_END ,
}rtk_fc_multicast_flow_flush_t;

typedef struct rtk_fc_multicast_flush_patten_s
{
	uint32 isIpv6:1;
	//for fastIdx search
	uint32 gip[4];		//extra for ipv6 info
}rtk_fc_multicast_flush_patten;


typedef struct rtk_fc_vxlanhdr_s {
	__be32 vx_flags;
	__be32 vx_vni;
}rtk_fc_vxlanhdr_t;

typedef struct rtk_fc_pktHdr_vxlan_info_s
{
	rtk_fc_vxlanhdr_t *vxlanHdr;
	uint32 vni;
	uint8 outerTag_off;
	uint16 outer_cvlanid;
	uint16 outer_svlanid;
	uint16 outer_ip_len_off;
	uint16 outer_udp_len_off;
	uint16 outer_ip6h_len_off;
	uint8 outer_isV6:1;
	uint8 outer_ppph:1;
	uint8 outer_cvh:1;
	uint8 outer_svh:1;
	unsigned char vxlan_DA[6];
	union{
		struct	in6_addr	gateway_v6_ip;
		uint32	gateway_v4_ip;
	};
	union{
		struct	in6_addr	remote_v6_ip;
		uint32	remote_v4_ip;
	};
	u8 outer_content[86]; // max = l2 + pppoe + 2*vlan + ipv6 40 + udp + vxlan hdr
	u8 inner_extra_len;
}rtk_fc_pktHdr_vxlan_info_t;

typedef struct rtk_fc_pktHdr_greEthBr_info_s
{
	uint8 outerTag_off;
}rtk_fc_pktHdr_greEthBr_info_t;

typedef struct rtk_fc_flowPath_param_s{
	uint8 igrNetifIdx;
	uint8 egrNetifIdx;
	int saIdx;
	int	daIdx;
	int32 tunnelExtraTagIdx;// for path5
	uint16 igrSVID;
	uint16 igrCVID;
	uint16 egrCVID;
	uint8 igrSVlanTagif;
	uint8 egrSVlanTagif;
	uint8 igrCVlanTagif;
	uint8 egrCVlanTagif;
	uint8 igrPPPoETagif;
	uint8 igr_tos;
	uint8 egr_tos;
	uint8 smacTrans;
	uint8 isNAPT; // for path5
	uint8 isMAPT; // for path5
	uint8 para_isXLAT; // for path5
	uint8 isIpv6;
}rtk_fc_flowPath_param_t;

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	typedef struct rtk_fc_pureIp_l4_Info_s{
		/*
			HW PP parsing UDP-lite/RDPv1/RDPv2 src/dst port into header_i. If support pure IP HW acceleration, need to set src/dst port info of these protocol.
			***dual header outer must not UDP-lite/RDPv1/RDPv2***

			HW PP parsing RDP ctrl bits (first 8 bits) into header_i. If RDP ctrl match hw key fin or rst, SW need to recalculate CRC
			***dual header outer must not RDPv1/RDPv2***
		*/

		uint16 src_port;
		uint16 dst_port;
#if !defined(CONFIG_FC_RTL9607F_SERIES) // 9607F hash key do not care tcp_rdp_ctrl
		uint8 rdp_recal_need_hw_crc:1;
#endif
	}rtk_fc_pureIp_l4_Info_t;
#endif
typedef struct rtk_fc_ipsec_SCInfo_s{
	int valid;
	int direction;
	int cipher_mode;
	int hash_mode;
	uint32 crypto_plaintext_length;
	uint32 total_crypto_len;
	uint32 decrypt_ori_len;
	uint16 ivsize;
	uint16 auth_sizes;
	uint16 devGwMacIdx;
	uint16 key_size;
	uint16 key_hash_size;
	uint8 isESN;
	uint8 doHwlookup;
	uint8 isSlowPath;
	uint8 table_index;
	uint8 skb_recovery_for_hwLookup;
	uint8 ring_index;
	uint8* pIcv_addr;
	uint8* pCiphertext_addr;
}rtk_fc_ipsec_SCInfo_t;


typedef enum rtk_fc_ipsec_hash_algo_enum_s{
    RTK_FC_HM_MD5,
    RTK_FC_HM_SHA1,
    RTK_FC_HM_SHA2_224,
    RTK_FC_HM_SHA2_256
}rtk_fc_ipsec_hash_algo_enum_t;



typedef struct rtk_fc_cryptoEngine_command_s {
#ifdef CONFIG_CPU_BIG_ENDIAN
	union 
	{
		uint32_t wrd;
		struct {
			//word 0
	      uint32_t resv:1;          //reserved bit
	      uint32_t icvtl:7;         //Initial Check Vector Total Length(Unit = 1 byte)
	      uint32_t shnw:1;          //Sequential Hash No Write Back
	      uint32_t sh:1;            //Sequential Hash
	      uint32_t shf:1;           //Sequential Hash First
	      uint32_t em:3;            //Engine Mode
	      uint32_t shl:1;           //Sequential Hash Last
	      uint32_t hm:3;            //HMAC Mode
	      uint32_t he:1;            //HMAC Enable
	      uint32_t ckbs:1;          //Cipher Key Byte Swap
	      uint32_t tridt:1;         //3DES Type
	      uint32_t tride:1;         //3DES Enable
	      uint32_t aks:2;           //AES Key Type Select
	      uint32_t ce:1;            //Cipher Encrypt
	      uint32_t resv1:1;         //reserved bit
	      uint32_t ces:2;           //Cipher engine select
	      uint32_t cm:4;            //Cipher mode
		}bits;
	}wrd0;
	union 
	{
		uint32_t wrd;
		struct {
			// word 1
	      uint32_t hkbs:1;          //Hash Key Byte Swap
	      uint32_t hkws:1;          //Hash Key Word Swap
	      uint32_t hobs:1;          //Hash Output Byte Swap
	      uint32_t hows:1;          //Hash Output Word Swap
	      uint32_t hibs:1;          //Hash Input Byte Swap
	      uint32_t hiws:1;          //Hash Input Word Swap
	      uint32_t habs:1;          //Hash Aligned Byte Swap
	      uint32_t haws:1;          //Hash Aligned Word Swap
	      uint32_t cidws:1;         //Cipher Input Double Word Swap
	      uint32_t codws:1;         //Cipher Output Double Word Swap
	      uint32_t cobs:1;          //Cipher Output Byte Swap
	      uint32_t cows:1;          //Cipher Output Word Swap
	      uint32_t cibs:1;          //Cipher Input Byte Swap
	      uint32_t ciws:1;          //Cipher Input Word Swap
	      uint32_t cabs:1;          //Cipher Aligned Byte Swap
	      uint32_t caws:1;          //Cipher Aligned Word Swap
	      uint32_t hsibs:1;         //Hash Sequential Initial Byte Swap
	      uint32_t eplds:3;         //Encryption Padding Last Data Size.(For Mix mode:SSL_TLS-enc)
	      uint32_t cwks:1;          //Cipher Key Word Swap
	      uint32_t plds:3;          //Hash Padding Last Data size.(For mix mode Hash padding)
	      uint32_t elds:4;          //Encryption Last Data Size
	      uint32_t aadlds:4;        //AAD Last Data Size
		}bits;
	}wrd1;
    
	union 
	{
		uint32_t wrd;
		struct {
			// word 2
	      uint32_t etl:16;          //Encryption Total Length
	      uint32_t eptl:2;          //Encryption Padding Total Length
	      uint32_t htl:6;           //Header Total Length (AAD total length)
	      uint32_t hptl:8;          //Hash Padding Total Length
		}bits;
	}wrd2;
	
#else
	union 
	{
		uint32_t wrd;
		struct {
			uint32_t cm:4;            //Cipher mode
			uint32_t ces:2;           //Cipher engine select
			uint32_t resv1:1;         //reserved bit
			uint32_t ce:1;            //Cipher Encrypt
			uint32_t aks:2;           //AES Key Type Select
			uint32_t tride:1;         //3DES Enable
			uint32_t tridt:1;         //3DES Type
			uint32_t ckbs:1;          //Cipher Key Byte Swap
			uint32_t he:1;            //HMAC Enable
			uint32_t hm:3;            //HMAC Mode
			uint32_t shl:1;           //Sequential Hash Last
			uint32_t em:3;            //Engine Mode
			uint32_t shf:1;           //Sequential Hash First
			uint32_t sh:1;            //Sequential Hash
			uint32_t shnw:1;          //Sequential Hash No Write Back
			uint32_t icvtl:7;         //Initial Check Vector Total Length(Unit = 1 byte)
			uint32_t resv:1;          //reserved bit
		}bits;
	}wrd0;
	
	union 
	{
		uint32_t wrd;
		struct {
			uint32_t aadlds:4;        //AAD Last Data Size
			uint32_t elds:4;          //Encryption Last Data Size
			uint32_t plds:3;          //Hash Padding Last Data size.(For mix mode Hash padding)
			uint32_t cwks:1;          //Cipher Key Word Swap
			uint32_t eplds:3;         //Encryption Padding Last Data Size.(For Mix mode:SSL_TLS-enc)
			uint32_t hsibs:1;         //Hash Sequential Initial Byte Swap
			uint32_t caws:1;          //Cipher Aligned Word Swap
			uint32_t cabs:1;          //Cipher Aligned Byte Swap
			uint32_t ciws:1;          //Cipher Input Word Swap
			uint32_t cibs:1;          //Cipher Input Byte Swap
			uint32_t cows:1;          //Cipher Output Word Swap
			uint32_t cobs:1;          //Cipher Output Byte Swap
			uint32_t codws:1;         //Cipher Output Double Word Swap
			uint32_t cidws:1;         //Cipher Input Double Word Swap
			uint32_t haws:1;          //Hash Aligned Word Swap
			uint32_t habs:1;          //Hash Aligned Byte Swap
			uint32_t hiws:1;          //Hash Input Word Swap
			uint32_t hibs:1;          //Hash Input Byte Swap
			uint32_t hows:1;          //Hash Output Word Swap
			uint32_t hobs:1;          //Hash Output Byte Swap
			uint32_t hkws:1;          //Hash Key Word Swap
			uint32_t hkbs:1;          //Hash Key Byte Swap
		}bits;
	}wrd1;
	
	union 
	{
		uint32_t wrd;
		struct {
			uint32_t hptl:8;          //Hash Padding Total Length
			uint32_t htl:6;           //Header Total Length (AAD total length)
			uint32_t eptl:2;          //Encryption Padding Total Length
			uint32_t etl:16;          //Encryption Total Length
		}bits;
	}wrd2;
#endif
      uint32_t apl1;                //All Message Length1 For Auto Padding Information.
      uint32_t apl2;
      uint32_t apl3;
      uint32_t apl4;
}rtk_fc_cryptoEngine_command_t;



// private ring buffer
typedef struct rtk_fc_ipsec_ipi_private_s
{
	struct sk_buff *skb;
	rtk_fc_smp_nicTx_private_t nicTx_priv;
	uint16 crypto_len;
	uint16 cipher_mode;
	uint16 ipsec_hook_table_index;
	uint16 macPortIdx;
	uint8 iv_size;
	uint8 aead_size;
	uint8 isEsn_encrypt;
	uint8 isEsn_hash;
	uint8 icv_size;
	uint8 isSlowPath;
	uint8 encrypt;
	uint8 *pCipher_text_address;
	uint8 *pIcv_address;
	uint32_t iv_dma;
	u8 *iv_pointer;
	uint32_t cmdsetting_dma;
	uint32_t aead_dma;
	u8 *aead_pointer;
	rtk_fc_cryptoEngine_command_t *cmdsetting_pointer;
}__attribute__((packed)) rtk_fc_ipsec_ipi_private_t;

typedef struct rtk_fc_ipsec_ipi_work_info_s
{
	rtk_fc_ipsec_ipi_private_t fc_ipi_ring_info;	
}rtk_fc_ipsec_ipi_work_info_t;

typedef struct rtk_fc_ipsec_ring_ctrl_s
{
	rtk_fc_ipsec_ipi_work_info_t priv_work[MAX_FC_IPSEC_QUEUE_SIZE];
}rtk_fc_ipsec_ring_ctrl_t;

typedef struct rtk_fc_ipsec_ipi_ctrl_s{

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
	struct __call_single_data csd ____cacheline_aligned_in_smp;
#else
	struct call_single_data csd ____cacheline_aligned_in_smp;
#endif
	atomic_t csd_available;

	struct tasklet_struct tasklet;
	
	atomic_t priv_free_idx; 
	atomic_t priv_sched_idx; 
	uint32 hw_done_pkt_cnt;
	uint32 priv_work_cnt;
	rtk_fc_ipsec_ipi_work_info_t *priv_work;

}rtk_fc_ipsec_ipi_ctrl_t;




typedef struct rtk_fc_pktHdr_s
{
	struct ethhdr *eth;
	struct vlan_hdr *svh;
	struct vlan_hdr *cvh;
	struct pppoe_hdr *ppph; // NULL: untag
	struct iphdr *iph; //NULL: not ipv4 header
	struct ipv6hdr *ip6h; //NULL: not ipv6 header
	struct tcphdr *tcph; //NULL: not tcp header
	struct udphdr *udph; //NULL: not udp header
	struct igmphdr *igmph; //NULL:not igmp header
	struct icmp6hdr *icmp6h;	//NULL:not icmp6hdr header
	struct arphdr *arph;
	struct ip_esp_hdr *esph;
	struct iphdr *outer_iph; //NULL: not dual header format
	struct udphdr *outer_udph; //NULL: not dual header format
	struct ipv6hdr *outer_ip6h; //NULL: not ipv6 header
	struct ethhdr *outer_eth;
	struct vlan_hdr *outer_svh;
	struct vlan_hdr *outer_cvh;
	struct pppoe_hdr *outer_ppph; // NULL: untag
	/*
		single header:
		- l4_proto can be get from pktHdr.l4protol
		dual header:
		- outer IPv4: l4_proto can be get from outer_l4protol
		- outer IPv6: l4_proto can be get from outer_l4protol
		- inner IPv4: l4_proto can be get from pktHdr.l4protol
		- inner IPv6: l4_proto can be get from pktHdr.l4protol

		HW PP parsing UDP-lite/RDPv1/RDPv2 src/dst port into header_i. If support pure IP HW acceleration, need to set src/dst port info of these protocol.
		***dual header outer must not UDP-lite/RDPv1/RDPv2***

		HW PP parsing RDP ctrl bits (first 8 bits) into header_i. If RDP ctrl match hw key fin or rst, SW need to recalculate CRC
		***dual header outer must not RDPv1/RDPv2***
	*/
	uint8 outer_l4proto;

	struct gre_base_hdr *greh;	//for gre trans ethertype bridge
	struct icmphdr *icmph;	//NULL:not icmp6hdr header
	unsigned int ipv6Sip_hash;	//meanful if ip6h is not NULL
	unsigned int ipv6Dip_hash;	//meanful if ip6h is not NULL
	unsigned int outer_ipv6Sip_hash;	//meanful if ip6h is not NULL
	unsigned int outer_ipv6Dip_hash;	//meanful if ip6h is not NULL

	rtk_fc_pmap_t ingressPort;				// ingress
	rtk_fc_wlan_devidx_t	igrWlanDevIdx;		// ingress info (if pkt rx from wifi)
	rtk_fc_pmap_t egressPort;				// egress
	rtk_fc_wlan_devidx_t	egrWlanDevIdx;		// egress
	struct net_device *dstDev;
	unsigned char logicalInDevIfidx[DEV_STACK_MAX]	;	//index to devGwMacTbl
	unsigned char logicalOutDevIfidx[DEV_STACK_MAX]	;	//index to devGwMacTbl
	rtk_fc_dualHdrtype_t dualHdrType;		//parser result (include passthrough)
	uint32 dualHdrHashKey;					//non-dual is zero (assign by packet parser)
	rtk_fc_dualHdrtype_t dual_passthrough_type;
	rtk_fc_tunnelInfo_t tunnelInfo;
	rtk_fc_remarkDecision_t remarkDec;

	rtk_fc_abstrSwFlowPattern_entry_t swFlowPattern;
	rtk_fc_abstrSwFlowActionList_entry_t *pCurSwFlowAction;		// valid if hit abst sw shortcut, referenced by egress tx
	unsigned short svlanid:12;
	unsigned short stpid_sel:1;
	unsigned short svlanpri:3;
	unsigned short svlandei:1;
	unsigned short cvlanid:12;
	unsigned short cvlanpri:3;
	unsigned short cvlancfi:1;
	unsigned short outer_svlanid:12;
	unsigned short outer_svlanpri:3;
	unsigned short outer_svlandei:1;
	unsigned short outer_cvlanid:12;
	unsigned short outer_cvlanpri:3;
	unsigned short outer_cvlancfi:1;

	uint8 l4protol;
	uint8 shortcut_earlycheck:1;		// indication bit for shortcut earlycheck procedure
	uint8 shortcut_earlycheck_hit:1;	// indication bit for shortcut earlycheck hit state
	uint8 abstrSwFlowSearch:1;
	uint8 abstrSwFlowFlooding:1; //if new action update(BC/MC/UUC/UMC) portmask to flow or replace(UC) flow
	uint8 isMulticast:1;		//L3 IPMC packet
	uint8 dmacToGatewayMAC:1; 	// ingress info
	uint8 smacEgrChange:1;		// egress info to indicate smac is not the same with ingress mac
	uint8 outerHdrMiss:1;		//single:False dual_outerHdrMiss(really dual routing):False  dual_outerHdrMiss(passthrough):True
	uint8 isFragment:1;
	uint8 fromWlan_noHWlookup:1;			// 1: wifi slowpath, non-unicast or not belong to root, vap0, vap1, vap2, vap3; 0: wifi hwlook miss from NIC.
	uint8 isPermitWanAccess:1;
	uint8 isHitShortcut:1;
	uint8 dualFailed:1;
	uint8 isHitSwFwdedAclTrapPri:1;
	uint8 egressFlowAddComplete:1;
	uint8 nptv6_outerFlow:1;
	uint8 isMAPT:1;
	uint8 isXLAT:1;
	uint8 isP5DS:1;
	uint8 mcPortInConfig:1;
	uint8 mcIsGrpDftFwd:1;
	uint8 abstrFlowCopy2cpu:1;
	uint8 force_to_bridge_flow:1;

	uint8 reason;
	uint8 ponstreamid;				// ingress only from rx_info
	uint16 ethtype;					// after eth/svlan/cvlan
	uint16 ethtype_proto;				// after eth/svlan/cvlan/pppoe(0x8864)
	uint16 skbLen_ingress;
	uint16 skbLen_egress;
	uint16 l2PayloadLen;
	uint16 payloadLen;	//length without L2/L3/L4 header length
	uint16 l3Offset;
	uint16 l4Offset;
	
	// egress info
	rtk_fc_forwardingType_t fwdType;
	rtk_fc_flow_direction_t direction;	//setting in _rtk_fc_flow_direction
	uint8 isV6_NAT;
	uint8 skip_special_fastFwd;
	union
	{	// Notice: please check corresponding dualHdrType
		rtk_fc_pktHdr_vxlan_info_t vxlan_info;
		rtk_fc_pktHdr_greEthBr_info_t greEthBr_info;
	};

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	dma_lsopara_0_t txlso_para0;
	rtk_fc_pureIp_l4_Info_t pureIp_l4_info; // for pure IP HW acceleration
#endif

	unsigned short outer_stpid_sel:1;
	uint8 ipFragFlag:3;						// for fragment shortcut only
	rtk_fc_ipFrag_linkList_t cacheIpFragInfo;
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)	
	rtk_fc_ipsec_SCInfo_t ipsec_sc_info;
	uint32 crypto_plaintext_length;
	uint8 skip_replace_seqno;	
	int ipsec_hook_table_index;
#endif

	uint8 RESET_BOUNDARY;	//3NOTICE: init above members as 0, below members as -1
	
	int32 flowHashIdx;	// save Entry Idx for both sram and dram mode. FAIL or 0~N; (crc16 for 8277C)
	uint32 flowIdx;		// flow index

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	uint32 crc32;
	int8 pkt_fmr_idx:3;	//for MAP-T/MAP-E, -1 means don't care  #define L3PE_DUAL_FMR_TBL_ENTRY_MAX 3
#endif
	//int16 l2dual_outer_tag_len;
	int16 smacL2Idx;	// used by path1 in ingress phase, -1: not found in lut table.	[ingress smac] idx in egress phase
	int16 dmacL2Idx;	// used by path1 in ingress phase, -1: not found in lut table	[egress dmac] idx in egress phase
	int16 smacHostPolIdx; //SMAC host policing index, -1: not found in host policing ctrl table
	int16 dmacHostPolIdx; //DMAC host policing index, -1: not found in host policing ctrl table
	
	int16 outer_smacL2Idx;  // used by path1 in ingress phase, -1: not found in lut table.  [ingress smac] idx in egress phase
	//int16 outer_dmacL2Idx; // no reference

	int nptv6_acceleration_upstream_idx; // For Src Port = even
	int nptv6_acceleration_upstream0_idx;// For Src Port = odd
	int nptv6_acceleration_downstream_idx;
	int nptv6_acceleration_downstream0_idx;

	uint8 inDev_internal_vid_fdb_find;
	uint8 outDev_internal_vid_fdb_find;
	uint8 path5_downStream_flow_bucket_idx;
	uint16 inDev_internal_vid;			// vlan filtering pvid
	uint16 outDev_internal_vid;			// vlan filtering pvid
}rtk_fc_pktHdr_t;

typedef struct rtk_fc_lut_ivl_entry_s
{
	uint32 valid:1;
	uint32 ivl_pvid:12;		// associated from flow entry
	uint32 flow_ref_cnt;		// 0: invalid; others: valid;
}rtk_fc_lut_ivl_entry_t;

typedef struct rtk_fc_lut_ivl_sel_s
{
	uint8 valid:1;
	uint8 traffic_bit:1;
	uint8 ivl_tbl_idx:RTK_FC_TABLESIZE_LUT_IVL_BITS;		// associated from flow entry
}__attribute__((packed))rtk_fc_lut_ivl_sel_t;

typedef struct rtk_fc_mcToUc_macid_s
{
	uint16 valid:1;
	uint16 lutIdx:15;
	uint16 refCnt;
}rtk_fc_mcToUc_macid_entry_t;

typedef struct rtk_fc_lut_entry_s
{
	unsigned char l2Addr[ETH_ALEN];
	unsigned char spa;
	unsigned char extspa;
	rtk_fc_wlan_devidx_t wlan_dev_idx;			// rtk_fc_wlan_devidx_t, init as RTK_FC_WLANX_NOT_FOUND

	unsigned char CTAG_IF;
	unsigned short CVID;
	unsigned long last_jiffies;		//for non-fdb-unicast and non-static mulitcast timeout jiffies
	uint16 lutIdx:MAX_FLOW_LUT_IDX_WIDTH;

	uint16 staticRefCount;
	unsigned char valid:1;				// 1: valid entry; 0: invalid entry or entry is deleted but will be freed later sine RCU protection.
	unsigned char rcuFreeEnt_skipDel:1;		// if the new l2 is created, skip hw delete action when free_entry_function is executing.
	unsigned char isStatic:1;
	unsigned char isStaticWiFi:1;			// 1: add/delete wifi lut by wifi driver
	unsigned char isUnknowUcMc:1;			//if unknown UC/L2MC/L3MC entry we do't known it's spa/extspa, just add for hardware flow accelerate
	unsigned char isSync2PsEntry:1;//for lut entries that are sync to Linux FDB/Neighbor.
	unsigned char isPermitWanAccess:1;	//for wan access limit when limited by mac. 0:check, 1:permitted
	unsigned char anyFlowEstablish:1;		// do not check flow table if 0 to speedup processing time
	unsigned char isGMAC:1;
	unsigned char duplicateEntry_exist:1;	// indicate duplicate l2/mac exist with different svl_fid, used by wifi sta
	unsigned char svl_fid:2;				// filter id for duplicate sa learning, add/del by the parent entry which duplicateEntry_exist = 1

	int16 hostPolIdx;// for store host policing entry index
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	uint8 wifiMacId;// for store mac_id, 0 means invalid
#endif
	rtk_fc_lut_ivl_sel_t	ivl_sel[RTK_FC_TABLESIZE_LUT_IVL];
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
	int daHashLiteIdx;					// init as SIGNED_INVALID
#endif
	struct rcu_head lutRcuHead;
	struct list_head lutList;
	struct list_head lutQuickList;
}rtk_fc_lut_entry_t;

typedef struct rtk_fc_tableEthType_s
{
	uint8 valid:1;
	uint8 hwIdx:6;
	uint8 flow_ref:1;		// 1: enabled by proc; 0: disabled.
	uint8 acl_ref;			// !=0 if acl rule set ethtype pattern
	uint16 ethType;
}rtk_fc_tableEthType_t;

typedef enum rtk_fc_sw_shaperMib_update_modeType_s
{
	SW_SHAPERMIB_UPDATE_BY_PKTLEN = 0,
	SW_SHAPERMIB_UPDATE_BY_PAYLOADLEN,
	SW_SHAPER_UPDATE_BY_PAYLOADLEN_MIB_UPDATE_BY_PKTLEN,
	SW_SHAPER_UPDATE_BY_PKTLEN_MIB_UPDATE_BY_PAYLOADLEN,
	SW_SHAPER_UPDATE_END,
}rtk_fc_sw_shaperMib_update_modeType_t;
typedef enum rtk_fc_shaping_destType_e
{
	SHAPING_DESTTYPE_NIC = 0,	// to NIC
	SHAPING_DESTTYPE_WIFI,		// to WIFI
}rtk_fc_shaping_destType_t;

typedef struct rtk_fc_shaping_buffInfo_s
{
	rtk_fc_shaping_destType_t txDest:1;		// 1:to wifi; 0:to nic
	uint32 smacHostPolIdx_vld		:1;
	uint32 dmacHostPolIdx_vld		:1;
	uint32 flowTblIdx_vld			:1;
	uint32 smacHostPolIdx			:5;		// 32 entries
	uint32 dmacHostPolIdx			:5;		// 32 entries (-1 represent invalid)
	uint32 flowTblIdx				:17;	// 9607: 32k(HW)+32k(SW) entries, 8277/8277B: 64k(HW)+64/256(HW overflow)+32k(SW) entris (-1 represent invalid)
}rtk_fc_shaping_buffInfo_t;

typedef enum
{
    RTK_FC_IPV6_TYPE_NORMAL,
    RTK_FC_IPV6_TYPE_NAT66,
    RTK_FC_IPV6_TYPE_NPTv6,
    RTK_FC_IPV6_TYPE_MAX
} RTK_FC_IPV6_NAT_TYPE;
	
typedef enum rtk_fc_nptv6_acc_enable_e
{
    RTK_FC_NPTV6_ACC_DISABLE = 0,
    RTK_FC_NPTV6_ACC_ENABLE_FOR_4LAN_PON_WAN = 1,
    RTK_FC_NPTV6_ACC_ENABLE_FOR_2LAN_PON_WAN = 2,
    RTK_FC_NPTV6_ACC_ENABLE_FOR_2LAN_ETHERNET_WAN = 3,
} rtk_fc_nptv6_acc_enable_t;


typedef struct rtk_fc_shaping_bucket_s
{
	rtk_fc_shaping_buffInfo_t buffInfo;
	uint16 payLoadLen;
	union{
		rtk_fc_smp_nicTx_private_t nicTxPriv;
		rtk_fc_smp_wifiTx_private_t wifiTxPriv;
	};
	atomic_t state;		// rtk_fc_smp_work_state_t
}__attribute__((packed))rtk_fc_shaping_bucket_t;

typedef struct rtk_fc_sw_shaping_ctrl_s
{
	//spinlock_t lock;
	atomic_t free_idx;
	atomic_t sched_idx;
	atomic_t congestion;
	rtk_fc_timer_list_t *kicktxtimer;
	uint32 enququcnt;
	uint32 pausetxcnt;
	unsigned long int lastJiffies_kickTimer;	// record jiffies when kicking timer
	rtk_fc_shaping_bucket_t queue[RTK_FC_MAX_SHAPING_QUEUE_SIZE];
}rtk_fc_sw_shaping_ctrl_t;

typedef struct rtk_fc_ackDelay_entry_s
{
	uint32 list_idx		:16;
	uint32 valid		:1;

	uint32 src_port_idx	:6;
	uint32 l4_offset	:9;
	uint32 crc16		:16;
	uint32 crc32;
	uint32 seq_no;
	struct sk_buff *skb;

	uint32 pktNum;
	rtk_fc_timer_list_t *kicktxTimer;
	unsigned long int expireJiffies;

	struct list_head entry_list;
}rtk_fc_ackDelay_linkList_t;

typedef struct rtk_fc_sw_ackDelay_ctrl_s
{
	uint32 enable				:1;
	uint32 pktNum_timeout		:16;
	uint32 jiffies_timeout		:16;
	uint32 timer_timeout		:16;	//ms

	uint32 igr_portmask;		//check by acl
	uint32 igr_pkt_len_min		: 8;
	uint32 igr_pkt_len_max		: 8;
	uint32 sw_fwd_acl_trap_pri	: 3;
	uint32 egr_portmask;		//check by hash
}rtk_fc_sw_ackDelay_ctrl_t;

typedef struct rtk_fc_meterCount_s
{
	uint32 packetCount;
	uint32 byteCount;
}rtk_fc_meterCount_t;

#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
typedef struct rtk_fc_hw_pol_id_remap_s
{
	unsigned char valid:1;
	uint32 pol_id:9;
}rtk_fc_hw_pol_id_remap_t;
#endif

typedef struct rtk_fc_sw_meter_s
{
	rtk_enable_t ifgInclude;
	uint32 rate;	// K bps
	rtk_fc_meterCount_t meterCount; //per RTK_RG_SWRATELIMIT_JIFFIES/CONFIG_HZ second
	unsigned long int lastJiffies;
	rtk_fc_meterCount_t meterCountPersec;//per 1 second
	unsigned long int lastJiffiesPersec;
	uint16 lastBandwthByte_preInterval;
	uint16 lastBandwthByte_curInterval;
}rtk_fc_sw_meter_t;

typedef struct rtk_fc_sw_rate_limit_s
{
	rt_rate_sw_rate_limit_conf_t rateLimit_conf;
}rtk_fc_sw_rate_limit_t;

typedef struct rtk_fc_macAddr_learning_limit_s
{
	rt_misc_macAddr_learning_limit_t learningLimit_conf;
	atomic_t learningCount;
}rtk_fc_macAddr_learning_limit_t;

typedef struct rtk_fc_macAddr_learning_limit_group_s
{
	rtk_fc_macAddr_learning_limit_t limitCfg;
	rtk_fc_portmask_t group;
}rtk_fc_macAddr_learning_limit_group_t;

typedef struct rtk_fc_wan_access_limit_s
{
	rtk_fc_port_mask_t portMask;
	rtk_fc_wlan_devmask_t wlanMask;
	int accessLimitNumber;	// -1: unlimit
	rt_wan_access_limit_fields_t limitField;
	atomic_t learningCount;
	atomic_t timer_activity;
	rtk_fc_timer_list_t *neighbor_probe_timer;
	union
	{
		int probing_l2Idx;
		unsigned int probing_IP;
	};
}rtk_fc_wan_access_limit_t;

typedef struct rtk_fc_wan_access_limit_IP_list_s
{
	uint32 sip;
	struct list_head accessIP_list;
}rtk_fc_wan_access_limit_IP_list_t;

#define RTK_FC_API_NAME_LENGTH	30
typedef struct rtk_fc_api_module_s
{
	char rtk_fc_api_name[RTK_FC_API_NAME_LENGTH];

	/* init func */
	int (*rtk_fc_api_init)(void);
	int (*rtk_fc_api_exit)(void);

	/* Netif Event */
	int (*rtk_fc_api_Netif_inetAddrEvent)(void *ptr, unsigned long event);
	int (*rtk_fc_api_Netif_inet6AddrEvent)(void *ptr, unsigned long event);
	int (*rtk_fc_api_Netif_netDevEvent)(void *ptr, unsigned long event);

	/* Lut learning callback func */
	int (*rtk_fc_api_lut_add)(void* input_data);
	int (*rtk_fc_api_lut_del)(void* input_data);
	int (*rtk_fc_api_lut_query)(unsigned char *mac, int *lutIdx);



}rtk_fc_api_module_t;

typedef struct rtk_fc_icmpCtrlInfo_s
{
	uint16 ingress_icmp_id;
	uint16 egress_icmp_id;
	uint16 icmp_seqence_num;
}rtk_fc_icmpCtrlInfo_t;

typedef struct rtk_fc_ipv6_nat_mappingTbl_s
{
	struct in6_addr	addr;
	atomic_t refCount;
	uint16 oriCVID;
	uint16 oriCPRI;
	uint16 oriSVID;
	uint16 oriSPRI;
	uint8 direction;
	uint8 isNPTv6;
	uint32 wan_prefix_len;
	uint32 lan_prefix_len;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	uint32 prefix_idx;
#endif		
}rtk_fc_ipv6_nat_mappingTbl_t;

typedef struct rtk_fc_l2DualTbl_vxlan_us_s
{
	uint8 outer_tag_len;
	uint8 streamid;
	uint8 streamid_en;
	int16 dmacL2Idx;
	int16 outer_is_ipv6;
	uint8 outer_ppph_tag_off;	
	uint8 outer_svh_if;
	uint8 outer_cvh_if;
	uint16 outer_iph_tag_off;
	uint16 outer_ip_len_off;	//ip header total len field
	uint16 outer_udph_tag_off;
	uint16 outer_udph_sport;
	uint16 outer_udp_len_off;
	uint16 inner_cvlan_cvid;
	uint16 inner_cvlan_pri;
	uint16 inner_cvlan_cfi;
	uint16 inner_cvlan_tagif;
	uint32 vni;
	u8 contentBuffer[86];
	u8 fastFwd_outer_contentBuffer[70];
	int inner_extra_len;
	int offset_without_vlan_pppoe;
	int ingress_intf_idx;
	int egress_intf_idx;
}rtk_fc_l2DualTbl_vxlan_us_t;

typedef enum rtk_fc_vxlan_fastFWD_flow_ret_e{
	FC_VXLAN_FLOW_OK = 0,             
	//---------------------------------------------------------By-Pass VXLAN Adding Process
	FC_VXLAN_FLOW_BYPASS_VXLAN_ADD = 0x1000,
	FC_VXLAN_FLOW_BYPASS_SWONLY_MODE,
	FC_VXLAN_FLOW_DOWNSTREAM_FULL,
	FC_VXLAN_FLOW_NOT_VXLAN_NIC_ACC_MODE,
	FC_VXLAN_FLOW_NOT_SPECIAL_FASTFWD_MODE,
	FC_VXLAN_FLOW_UPSTREAM_FULL,										//1005
	FC_VXLAN_FLOW_UPSTREAM_INTERFACE_ERR,
	FC_VXLAN_FLOW_NOT_VXLAN_CANT_BE_ADDED,
	FC_VXLAN_FLOW_UPSTREAM_FASTFWD_DB_SET_FAILED, 
	FC_VXLAN_FLOW_UPSTREAM_PREALLOCATE_SKB_FAILED, 
	FC_VXLAN_FLOW_DOWNSTREAM_FASTFWD_DB_SET_FAILED,						//100a
	FC_VXLAN_FLOW_UPSTREAM_INNER_FLOW_EXCEED_PORT_LEARNING_LIMIT,
	FC_VXLAN_FLOW_UPSTREAM_INNER_ADD_FLOW_FAILED,
	FC_VXLAN_FLOW_UPSTREAM_OUTER_ADD_FLOW_FAILED,
	FC_VXLAN_FLOW_FLOW_EXISTS,
	FC_VXLAN_FLOW_DOWNSTREAM_INNERFLOW_FAILED,							//100f
	FC_VXLAN_FLOW_DOWNSTREAM_OUTERFLOW_FAILED,							//1010
	FC_VXLAN_FLOW_CPU_PORT_IN_USE,
	FC_VXLAN_FLOW_NOT_SUPPORT_OUTERV6,
	FC_VXLAN_FLOW_CPU_PORT_SEL_ERROR,
	FC_VXLAN_FLOW_EXTRA_SETTING_FAIL,
	FC_VXLAN_FLOW_EXTRA_GETVOQ_FAIL,
	FC_VXLAN_FLOW_RET_MAX,
}rtk_fc_vxlan_fastFWD_flow_ret_t;

typedef struct rtk_fc_l2DualTbl_vxlan_ds_s
{
	uint8 outer_tag_len;
	uint32 vni;
	int inner_extra_len;
	int ingress_intf_idx;
	int egress_intf_idx;
}rtk_fc_l2DualTbl_vxlan_ds_t;



typedef struct rtk_fc_ipsec_shortCut_info_s
{
	uint8 valid;
	uint16 direction;//up or down
	uint16 cipher_mode;
	uint16 auth_mode;
	uint32 egress_sip;
	uint32 egress_dip;
	uint32 spi;
	u8 contentBuffer[48];//40(ip/ip6 max,) + 8 (esp)
	uint16 hook_table_index;
	uint32 seq_num;
	uint32 isESN;
	uint32 seq_num_hi;
	uint8 encrypt;
	atomic_t refCount;
}rtk_fc_ipsec_shortCut_info_t;


typedef struct rtk_fc_l2DualTbl_s
{
	rtk_fc_l2Dual_type_t type;
	rtk_fc_l2Dual_action_t action;
	atomic_t refCount;
	union
	{
		rtk_fc_l2DualTbl_vxlan_us_t vxlan_us;//vxlan upstream
		rtk_fc_l2DualTbl_vxlan_ds_t vxlan_ds;//vxlan downstream
	};
}rtk_fc_l2DualTbl_t;

typedef enum rtk_fc_dualHeaderHWState_e
{
	FC_DUALHDR_HW_SETUP  			  =0<<1,
	FC_DUALHDR_HW_CONTENT_BUF_SETUP   =1<<2,
	FC_DUALHDR_HW_CTRL_UP_REG_SETUP   =1<<3,
	FC_DUALHDR_HW_CTRL_DOWN_REG_SETUP =1<<4,
	FC_DUALHDR_HW_CTRL_REG_MAX 		 =1<<7, //max 8 bits
}rtk_fc_dualHeaderHWState_t;

typedef enum rtk_fc_dualHdr_buf_insert_pos_e
{
	FC_DUALHDR_BUF_INSERT_POS_AFTER_L2 =0,	// insert dual header buffer after L2 header of original packet.
	FC_DUALHDR_HW_CTRL_PKT_BEGINIG = 1,		// insert dual header buffer at the begining of original packet. (e.g. GRE_ETH)
}rtk_fc_dualHdr_buf_insert_pos_t;

typedef enum rtk_fc_maptStyle_e
{
	FC_MAPT_STYLE_NONE = 0,
	FC_MAPT_STYLE_MAP,
	FC_MAPT_STYLE_MAP0,
	FC_MAPT_STYLE_RFC6052
}rtk_fc_maptStyle_t;

typedef struct rtk_fc_mapeInfo_s
{
	uint32 mape_tun_ipv4_addr;			// cache MAP-E tunnel IPv4 address  single->dual(upsatrem:sipv4) dual->single(downstream:dipv4)
	uint16 mape_out_dst6_idx;			// software fmridx
}rtk_fc_mapeInfo_t;



typedef struct rtk_fc_maptIpv6Info_s
{
	uint32 sip_indirect_mapping_index:RTK_FC_TABLESIZE_I6MAPT_MAPPING_TABLE_SHIFT_BITS;
	uint32 dip_indirect_mapping_index:RTK_FC_TABLESIZE_I6MAPT_MAPPING_TABLE_SHIFT_BITS;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	uint8 l4checksum_valid;
	uint16 l4checksum_diff;
#endif
}rtk_fc_maptIpv6Info_t;

typedef struct rtk_fc_ipv6_mapt_mappingTbl_s
{
	uint8 isHash:1;
	union{
		uint32 v6_hash_value;
		struct in6_addr	addr;
	};
	atomic_t refCount;
}rtk_fc_ipv6_mapt_mappingTbl_t;

typedef struct rtk_fc_pppoe_sync_info_s
{
	uint8 valid:1;
	uint8 syned:1;
}rtk_fc_pppoe_sync_info_t;

typedef struct rtk_fc_hook_func_s
{
	rtk_fc_converter_api_t *converter;
	rtk_fc_ps_api_t *ps;
	rtk_fc_mgr_api_t *mgr;
	rtk_fc_multi_wan_api_t *rtk_multi_wan;
	rtk_fc_vlan_api_t *rtk_vlan;
	rtk_fc_wlan_api_t *wlan;
	rtk_fc_rt_helper_api_t *rt_help_api;
	rtk_ext_phy_qos_t *rtk_ext_phy_qos;
	rtk_fc_init_all_mac_portmask_t *rtk_all_mac_portmask;
	
#if defined(CONFIG_FC_RTL8198F_SERIES)
	rtk_fc_83xx_qos_api_t *rtk83xxqos;
	rtk_fc_8367r_relay_mc_api_t *rtk_8367r_relay_mc;
#endif
	rtk_fc_mcast_api_t *rtk_mcast_api;
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_RTL8198XB_SERIES)
	rtk_fc_ext_flow_mib_t *rtk_ext_flow_mib;
#endif
	rtk_fc_ipfrag_helper_t *rtk_ipfrag;
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
	rtk_fc_tcp_helper_t *rtk_fc_tcp_helper;
#endif
}rtk_fc_hook_func_t;

typedef struct rtk_fc_flow_callback_func_s
{
	cbFlowAdd_t cbFlowAdd;
	cbFlowDel_t cbFlowDel;
	cbShortcut_t cbShortcut_before;
	cbShortcut_t cbShortcut_after;
}rtk_fc_flow_callback_func_t;
typedef struct rtk_fc_dynamic_prehash_info_s{
	uint32 sip_ptn;
	uint32 dip_ptn;
	uint32 sport_ptn;
	uint32 dport_ptn;
	int collision;
}rtk_fc_dynamic_prehash_info_t;

typedef struct rtk_fc_flow_info_natloopback_s {
	uint32 vaild;			// key and ref_cnt
	uint32 flow_ent_dx;	// key
	uint32 key_l3_dip;	// key if lan-to-lan case; don't care if all zero
	uint32 l3_ip;			// action fib. may be sip if parent flow is naptr; dip if parent flow is napt
	uint16 l4_port;		// action fib. may be sport if parent flow is naptr; dport if parent flow is napt
}rtk_fc_flow_info_natloopback_t;

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
typedef struct rtk_fc_natloopback_act_info_s
{
	uint32 is_ipv4_natloopback:1;
	uint32 l3_ip;
	uint16 l4_port;
}rtk_fc_natloopback_act_info_t;
#endif

typedef struct rtk_fc_glb_system_s
{
	uint8 	fbGlobalState[FB_GLOBAL_STATE_END];	//0:disable 1:enable
	uint8 	flowCheckState[FB_FLOW_CHECK_END];	//0:disable 1:enable
#if !(defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))
	uint32 	preHashPtn[FB_PREHASH_PTN_END];
#endif
	uint64 	phyPortStatus;	//0:disable 1:enable
	uint16	bypass_ethertypeProto[RTK_FC_ETHERTYPE_BYPASS_NUM];
	uint8	bypass_ethertypeProto_bitmap;
	uint16 	stagTPID[RTK_FC_TPID_SELECTION_MAX_NUM];		// default ->stagTPID[0]: 0x88a8, support up to 2 different tpid setting
	uint32 	ifstagTPID1_enabled:1;							// 1: enable stagTPID[1], 0: disable stagTPID[1]
	uint32 	flow_not_update_in_real_time:1;
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	uint32	pon_sidTbl_en:1;								// 1: if fc_db.streamidTbl is not empty
#endif
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	uint16 	l3te_voq_thrsh_profile_3_hth;
	uint16 	l3te_port_thrsh_profile_3_lth;
#endif
#if defined(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID)

	uint8 macport_0;
	uint8 macport_1;
	uint8 macport_2;
	uint8 macport_3;
	uint8 macport_4;
	uint8 macport_pon;
	uint8 macport_inic;
	uint8 macport_scpu;
	uint8 macport_rgmii;
	uint8 macport_mcpu_0;
	uint8 macport_mcpu_1;

	uint8 mac10extport_0;
	uint8 mac7extport_0;

#endif
	
}__attribute__((packed))rtk_fc_glb_system_t;

typedef struct rtk_fc_glb_controlfunc_s
{
	//3 === datapath reference ===
	rtk_fc_tcp_lrn_state_t flow_tcp_learning_state;	// learn to SW first and write to HW according to this config. default is TCP_LEARN_SYN_RECV.
	rtk_fc_port_mask_t l2_macEgrLearning;			//0: sa learning at ingress phase; 1: sa learning at egress phase
	rt_flow_hwnat_mode_t hwnat_mode; 					//0: disable acceleration, 1(default): enable acceleration, 2: disable hw acc(sw only), 3: disable sw acc (hw only)
	uint32 flow_sessionAutoExtend:1;
	uint32 l2_skipPsTracking:1;			// 0: default learning l2 bridge by associating ct state; 1: learn bridge flow even without ct info; learn L2/LUT even without fdb info
	uint32 flow_skipAllPsTracking:1;			// 0: default associate with Linux CT/FDB; 1: maintain flow by idle timeout.
	uint32 flow_tcp_bidirection_learning:1;		// 0: default learning by each tcp flow; 1: learn bidirection flow at same time.
	uint32 wifiSlowPathRxbyNic:1;				// 0: default call netif_rx; 1: continue rx by NIC hook func
	uint32 prevent_control_packet_drop:1; //0:disable 1:enable; be used to enable NIC suspend mechanism

	uint32 epon_llid_format:1;				//0: remarking LLID as SID;  1: remarking LLID merge qid as whole SID
	uint32 sw_shaperMib_update_mode_flow:2;		// ref: rtk_fc_sw_shaperMib_update_modeType_t
	uint32 sw_shaperMib_update_mode_host:1;		//0: default, SW shaper and mib update by total packet length (include header length), 1: SW shaper and mib update by payload length (exclude L2/L3/L4 header length)
	uint32 sw_per_flow_mib:1;
	uint32 gemFilter_conf_en:1;						// 1 if any fc_db.gemFilter_conf[] is enabled.
	uint32 flow_swFwded_aclTrapPri_mask:8;	//acl trap priority mask, if packet is trapped by ACL and the trap priority hit this mask, FC will add a distinct sw flow for it.
	uint32 bridge_5tuple_flow_accelerate_by_2tuple:1; //If enabled, 5 tuple bridge flow will be accerlerated by 2 tuple flow key
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	uint32 bc_hwflow_accelerate:1;					   //only valid when bridge_5tuple_flow_accelerate_by_2tuple=TRUE for /* L2MC || BC */
	uint32 dsliteHwAcceleration_disable:1;
#endif
	uint32 dsliteV6TosFromV4:1;
	uint32 v6RDSwAcceleration_disable:1;
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	uint32 flow_meter_mib_conf_dependence:1; //If enable, the mib configuration depends on meter configuration for every flow; Otherwise, the flow mib and flow meter configuration are independent.
	uint32 port4_hsgmii_en:1;						// port 4 is enabled by CONFIG_LAN_SDS_FEATURE
	uint32 p6_xfi_en:1;							// 10G XFI port en
	uint32 p7_rxsel_l3fe:1;						// PON rx to L2FE or L3FE
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	uint32 if_hash_cache_age0_hit:1;
	uint32 if_hash_fib_384_bits:1;			// support HW IPv6 NAPT and IPv4 SNAT+DNAT
	uint32 hash_crc_debug:1;
	uint32 tx_with_hdr_debug:1;
	uint32 en_vxlan_v6_hwacc:1;
	uint32 ipsec_en_shortCut:1;
	uint32 ipsec_pe_offload:1;
	uint32 hw_l3l4_error_pkt_check:1;
#endif
#endif

#if defined(CONFIG_FC_RTL8198F_SERIES)
	uint32 pe_fc_enable:1;	//0 : default pe fc is not enabled; 1: pe fc is enabled
	uint32 pe_port:6; // used to store which ca ne cpu port is pe port
#endif

	//3 === control setting only in proc or timer func ===
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	uint32 headeri_latch_mode:1;//0: not enabled, 1: enabled.
	uint32 headeri_check_afterPE:1;//0: check before, 1:  check After
#endif

	uint32 rt_api_is_enabled:1;//0: not enabled, 1: enabled.
	uint32 loopbackMode_is_enabled:1;//0: not enabled, 1: enabled.
	uint32 pppoe_connectionAutoExtend:1;//used for extend expire time of pppoe wan
	uint32 debug_message_display_to_tty:1;//0: disable, use printk to output debug message; 1: enable, use tty write function to output debug message
	uint32 module_probe_log:1;
	uint32 special_fast_forward_mode:1;
	uint32 hwnat_customize:1;
	uint32 hwnat_customize_nptv6_sram_acc_v2:1;
	uint32 house_keeping_flow_delete_immediatly:1;
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	uint32 dynamic_calculate_prehash_pattern:1;
	uint32 gmac2_usable:1;
	uint32 wifi_tx_trap_hash:1;
	uint32 wifi_tx_gmac_trunking_num:2;
	uint32 wifi_tx_gmac_auto_sel_en:1;
	uint32 wifi_tx_gmac_sel_spa:3;
	uint32 wifi_tx_gmac_sel_current:8;
	uint8 wifi_tx_gmac_sel[RTK_FC_MAC_PORT_MAX]; 		// rtk_fc_wifi_gmac_sel_t
#endif
	uint32 wifi_tx_qos_enable:1;
	uint32 mem_leak_debug:1;
	uint32 shortcut_earlycheck_en:2;	//0:disable 1:enable unicast(non-dual) 2:enalbe unicast and dual-hdr
	uint32 external_switch_en:2;		// rtk_fc_external_porttype_t
	uint32 vlan_filtering:1;
	uint16 external_switch_port_offset;
	uint32 ipsec_fastFwd:1;

	uint8 wifi_tx_qos_mapping[8];			// internal pri to wifi pri (0-7)
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	int8 wifi_tx_asmdu_queue_mapping[8];	// internal pri to amsdu PE queue, -1 means disable amsdu offload (TC mode: -1, 0, WMM mode: -1, 0,1,2,3)
#endif
	rtk_fc_timer_list_t *pppoe_connectionAutoExtend_timer;
	uint32 pppoe_connectionAutoExtend_period_unit1s;

	struct task_struct *tty_display_cur_sh_process;//use to store /bin/sh that currently used
	pid_t tty_display_cur_sh_process_pid;//use to store the pid of /bin/sh that currently used

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	rtk_fc_wifi_flow_crtl_threshold_t wifi_flow_crtl_threshold; // configuration for wifi flow control auto enabling mechanism
	rtk_fc_timer_list_t *wifi_flow_ctrl_detect_timer;
#endif
	rtk_fc_flow_deley_mode_t flow_delay_mode;
	uint32 flow_delay_hit_num:RTK_FC_HIT_NUMBER_TO_DELAY_BITS; // Support at most hit RTK_FC_HIT_NUMBER_TO_DELAY_MAX times and finally to fc flow.
	uint32 flow_delay_tcp_dis;		// tcp packets delay conrol.
	uint32 flow_delay_other_dis;		// !tcp packets delay control
	uint32 swAckDelay_en:1;
	uint32 swRateLimit_en:1;
	uint32 ip_frag_shortcut:1;
	uint32 acl_multiple_hit_cfg:1;	//0: disable, 1: enable
	uint32 acl_arp_targetip_offset:4;	//0: rule not exist, others: arp vlan offset(0/4/8)+1
	uint32 acl_para_dump:1;				//0: disable, 1: enable
	uint32 event_rec_ctrl:1;			//0: disable, 1: enable
	uint32 unknowv4McTrap:2;
	uint32 unknowv6McTrap:2;
	uint32 mc2uc_en:1;
	uint32 lan_queue_ability_enhance:1;

	uint32 func_on_pe_cpu_num[RT_PE_FUNC_NUM_MAX];
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	uint32 wan_ingress_shaper;	// 0: disabled; others: rate kbps.
	int8 flow_miss_meter_idx:8;	//0-31: use flow meter 0-31, SIGNED_INVALID: disabled
#endif
}__attribute__((packed)) rtk_fc_glb_controlfunc_t;


typedef struct rtk_fc_pptp_passthrough_flowMapping_e
{
	uint16 in_gre_call_id;	   	//pptp mode
	uint16 out_gre_call_id;
}rtk_fc_pptp_passthrough_flowMapping_t;

typedef struct rtk_fc_l2tp_passthrough_flowMapping_e
{
	uint32 in_l2tp_tunnel_id:16;
	uint32 in_l2tp_session_id:16;
}rtk_fc_l2tp_passthrough_flowMapping_t;

typedef struct rtk_fc_ipsec_passthrough_flowMapping_e
{
	uint32 esp_spi;	// save/compare network byte order
}rtk_fc_ipsec_passthrough_flowMapping_t;

typedef struct rtk_fc_dual_passthrough_flowMapping_e
{
	rtk_fc_dualHdrtype_t dual_type;
	uint32 refFlowIdx;
	union
	{
		rtk_fc_pptp_passthrough_flowMapping_t pptp_flowMapping;
		rtk_fc_l2tp_passthrough_flowMapping_t l2tp_flowMapping;
		rtk_fc_ipsec_passthrough_flowMapping_t ipsec_flowMapping;
	};
}rtk_fc_dual_passthrough_flowMapping_t;

typedef struct rtk_fc_default_route_info_e
{
	uint32	valid;
	char	dev_name[IFNAMSIZ];
	uint8	rt_tbl_id;
	__be32	gw_addr;
}rtk_fc_default_route_info_t;

typedef struct rtk_fc_flow_limit_config_e
{
	int	hw_path12_limit;		// -1 means unlimit
	int	hw_path34_limit;
	int	hw_path5_limit;
	int	hw_path6_limit;
	int	sw_path12_limit;
	int	sw_path34_limit;
	int	sw_path5_limit;
	int	sw_path6_limit;

	atomic_t	hw_path12_count;
	atomic_t	hw_path34_count;
	atomic_t	hw_path5_count;
	atomic_t	hw_path6_count;
	atomic_t	sw_path12_count;
	atomic_t	sw_path34_count;
	atomic_t	sw_path5_count;
	atomic_t	sw_path6_count;
}rtk_fc_flow_limit_config_t;

typedef enum rtk_fc_swShperType_e{
	SWSHAPER_TYPE_FLOW = 0,
	SWSHAPER_TYPE_SMAC_HOST = 1,
	SWSHAPER_TYPE_DMAC_HOST = 2,
	SWSHAPER_TYPE_MAX = 3,
}rtk_fc_swShperType_t;

typedef struct rtk_fc_vxlan_igrExtraInfo_s
{
	unsigned char outer_v4tos;				//ref to IPV4_TAGIF
	unsigned char outer_tag_len;       // For vxlan down stream
	signed short  outer_sa_idx;
	unsigned short outer_srcPort;			// For vxlan down stream
	unsigned int outer_srcIp;				// For vxlan down stream
	unsigned int outer_dstIp;				// For vxlan down stream
	//unsigned int vni;						// For vxlan down stream
	unsigned char vxlan_DA[6];
	uint8 is_vxlanDS_outerFlow;
	uint8 is_vxlanDS_innerFlow;
	uint8 is_vxlanUS_innerFlow;
	uint8 is_vxlanUS_outerFlow;
	uint8 ingress_inner_ctag ;
	uint8 ingress_inner_stag ;
	uint8 ingress_inner_pppoeTag ;
	uint8 is_l2_vxlan;
	//uint32 l2DualTableIndex;
	uint32 vxlan_downstream_inner_flow_idx;
	uint32 vxlan_upstream_inner_flow_idx;
#if defined(CONFIG_FC_CA8277B_SERIES)
	int cpuPortIdx;
	int extraIndex;
	uint8 useExtra;
	uint8 extraVOQ;
	uint8 innerExtra_len;
#endif	
	
	uint8 outer_isV6;
	uint8 outer_ctag;
	uint8 outer_stag;
	uint8 outer_pppoeTag;
	u8 outer_content[RTK_FC_L2DUAL_CONTENTBUFFER_SIZE];

	uint32 ingress_intf_idx;
	uint32 egress_intf_idx;
}rtk_fc_vxlan_igrExtraInfo_t;

typedef struct rtk_fc_igrExtraInfo_s
{
	int16 lutIgrSaIdx:(MAX_FLOW_LUT_IDX_WIDTH+1);		// -1 to indicate didn't care. [ingress sa] for lut sync usage
	int16 lutEgrDaIdx:(MAX_FLOW_LUT_IDX_WIDTH+1);		// -1 to indicate didn't care. [egress da] for lut sync usage
	uint32 swShaperEn:SWSHAPER_TYPE_MAX;
#if defined(CONFIG_FC_FLOW_AUTO_EXTEND)
	// start -- ingress info to sync flow timeout
	struct net_device *srcDev;
	rtk_fc_igrDummyData_t *dummyPkt;
	// end -- ingress info to sync flow timeout
#endif
	rtk_fc_maptIpv6Info_t maptInfo;
	rtk_fc_ingress_data_t *pFcIngressData;
}rtk_fc_igrExtraInfo_t;

typedef struct rtk_fc_nptv6_flow_info_s{
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	uint32 prefix_idx;
#endif
	uint8 lan_prefix_len;
	uint8 wan_prefix_len;	
	uint8 isNPTv6:1;
	
}rtk_fc_nptv6_flow_info_t;

typedef struct rtk_fc_g3IgrExtraInfo_s
{
	int16 lutIgrSaIdx:(MAX_FLOW_LUT_IDX_WIDTH+1); // -1 to indicate didn't care. [ingress sa] for lut sync usage
	int16 lutEgrDaIdx:(MAX_FLOW_LUT_IDX_WIDTH+1); // -1 to indicate didn't care. [ingress sa] for lut sync usage
	uint16 pppoe_session_id;
	uint16 svlan_id;
	uint16 cvlan_id;
	uint16 svlan_pri:3;
	uint16 igr_svlan_tpid_sel:1; // 0: fc_db.systemGlobal.stagTPID[0], 1: fc_db.systemGlobal.stagTPID[1]
	uint16 egr_svlan_tpid_sel:1; // 0: fc_db.systemGlobal.stagTPID[0], 1: fc_db.systemGlobal.stagTPID[1]
	uint16 igr_svlan_dei:1;
	uint16 egr_svlan_dei:1;
	uint16 igr_cvlan_cfi:1;
	uint16 egr_cvlan_cfi:1;
	uint16 egr_tos_ecn_remark:1;
	uint16 egr_tos_ecn:2;
	uint16 pon_stream_id:8; // WAN rx stream i
	uint16 bypass_flowDelay:1;
	uint16 ingress_icmp_valid:1;
	uint16 ingress_icmp_id;
	struct in6_addr ipv6_Saddr; //IPv6 address: network byte order ; IPv4 address: use s6_addr32[0], host byte order
	struct in6_addr ipv6_Daddr; //IPv6 address: network byte order ; IPv4 address: use s6_addr32[0], host byte order
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	struct in6_addr ipv6_nat_addr;
#endif
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	rtk_fc_natloopback_act_info_t natloopback_act_info;
#endif
	int16 ingressSaHostPolIdx; //host policing entry index that matches ingress SA (-1 means ingress SA does not hit any host policing entry)
	int16 egressDaHostPolIdx; //host policing entry index that matches egress DA (-1 means egress DA does not hit any host policing entry)
	uint8 isHitSwFwdedAclTrapPri;
	rtk_fc_dual_passthrough_flowMapping_t dualPtFlowMapping_info; //sw flow pattern

	rtk_fc_flow_direction_t direction;
	//rtk_fc_l2Dual_action_t l2Dual_action;
	//rtk_fc_l2Dual_type_t l2Dual_type;
	rtk_fc_vxlan_igrExtraInfo_t vxlan_info;

	
	int16 nptv6_hwlookup_flow;
	int16 nptv6_info_index;
	rtk_fc_nptv6_flow_info_t naptv6_info;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	uint16 crc16;									// CRC16 is necessary for overflow entry search
	uint32 crc32;
#endif
	rtk_fc_dualHdrtype_t dualHdrType;
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)	
	int ipsec_hook_table_index;
	uint16 remote_tunnel_lut_idx;
	uint16 cipher_mode;
	int ipsec_direction;
	int isIpsec_acc;
	uint32 ipsec_seq_num;
	int ipsec_spi;
	int ipsec_pe_offload;
#endif
}rtk_fc_g3IgrExtraInfo_t;

typedef struct rtk_fc_tableFlowEntry_s
{
	union{
		rtk_rg_asic_path1_entry_t path1;
		rtk_rg_asic_path2_entry_t path2;
		rtk_rg_asic_path3_entry_t path3;
		rtk_rg_asic_path4_entry_t path4;
		rtk_rg_asic_path5_entry_t path5;
		rtk_rg_asic_path6_entry_t path6;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
		rtk_rg_asic_pathMc_wifi_amsdu_tx_entry_t pathMc_wifi_amsdu_tx;
#endif
#endif
	};
}rtk_fc_tableFlowEntry_t;
typedef struct rtk_fc_ct_hash_info_s{
	struct list_head ct_hash_list;

	uint32 upstream_flow_index;
	uint32 upstream_flow_hash_index;

	uint32 downstream_flow_index;
	uint32 downstream_flow_hash_index;

	struct nf_conn *ct;
}rtk_fc_ct_hash_info_t;

 
typedef struct rtk_fc_saInfo_flowIdx_s{
	struct list_head flow_idx_list;
	uint32 flow_index;
	uint32 hash_index;

}rtk_fc_saInfo_flowIdx_t;

/* for setting structure*/
typedef union {
	uint32 wrd;
	struct {
#ifdef CONFIG_CPU_BIG_ENDIAN
      uint32_t resv:1;          //reserved bit
      uint32_t rs:1;            //!<  [30..30] Read data from Non-Secure/Secure master port: 1=Secure port, 0=Non-secure port
      uint32_t fs:1;            //First segment descriptor
      uint32_t ls:1;            //Last segment descriptor
      uint32_t wk:1;            //Write back secure key
      uint32_t sk:1;            //Secure key
      uint32_t cl:2;            //Command length
      uint32_t ap:2;            //Auto-padding
      uint32_t shivl:6;         //Sequential Hash Initial Value Length
      uint32_t pk:1;            //Key Padding
      uint32_t pl:7;            //Key Padding Length
      uint32_t il:4;            //Initial Vector Length
      uint32_t kl:4;            //Key Length
#else
      uint32_t kl:4;            //Key Length
      uint32_t il:4;            //Initial Vector Length
      uint32_t pl:7;            //Key Padding Length
      uint32_t pk:1;            //Key Padding
      uint32_t shivl:6;         //Sequential Hash Initial Value Length
      uint32_t ap:2;            //Auto-padding
      uint32_t cl:2;            //Command length
      uint32_t sk:1;            //Secure key
      uint32_t wk:1;            //Write back secure key
      uint32_t ls:1;            //Last segment descriptor
      uint32_t fs:1;            //First segment descriptor
      uint32_t rs:1;            //!<  [30..30] Read data from Non-Secure/Secure master port: 1=Secure port, 0=Non-secure port
      uint32_t resv:1;          //reserved bit
#endif
	}bits;
}rtk_fc_cryptoEngine_source_type1_t;

/* for data transfer structure*/
typedef union {
	uint32 wrd;
	struct {
#ifdef CONFIG_CPU_BIG_ENDIAN
	      uint32_t resv:1;          //reserved bit
	      uint32_t rs:1;            //!<  [30..30] Read data from Non-Secure/Secure master port: 1=Secure port, 0=Non-secure port
	      uint32_t fs:1;            //First segment descriptor
	      uint32_t ls:1;            //Last segment descriptor
	      uint32_t enl:14;          //Encryption Data Length, Length is 1byte alignment, the maximum is 16383 bytes.
	      uint32_t zero:1;          //Decide the bit field [12-8] content is A2EO or EPL
	      uint32_t a2eo_epl:5;      //Authentication to Encryption Offset (Additional Authentication Data Length) or EPL( Encryption Padding Length)
	      uint32_t apl:8;           //Authentication(Hash) Padding Length
#else
	      uint32_t apl:8;           //Authentication(Hash) Padding Length
	      uint32_t a2eo_epl:5;      //Authentication to Encryption Offset (Additional Authentication Data Length) or EPL( Encryption Padding Length)
	      uint32_t zero:1;          //Decide the bit field [12-8] content is A2EO or EPL
	      uint32_t enl:14;          //Encryption Data Length, Length is 1byte alignment, the maximum is 16383 bytes.
	      uint32_t ls:1;            //Last segment descriptor
	      uint32_t fs:1;            //First segment descriptor
	      uint32_t rs:1;            //!<  [30..30] Read data from Non-Secure/Secure master port: 1=Secure port, 0=Non-secure port
    	  uint32_t resv:1;          //reserved bit
#endif
	}bits;
}rtk_fc_cryptoEngine_source_type2_t;



union rtk_fc_cryptoEngine_source_ctrl_u{
      rtk_fc_cryptoEngine_source_type1_t type1;
      rtk_fc_cryptoEngine_source_type2_t type2;
};

typedef union rtk_fc_cryptoEngine_source_ctrl_u rtk_fc_cryptoEngine_source_ctrl_t;

typedef struct rtk_fc_cryptoEngine_source_desc_s {
      rtk_fc_cryptoEngine_source_ctrl_t ctrl;
      uint32_t sdbp;
}rtk_fc_cryptoEngine_source_desc_t;

typedef union  {
	uint32 wrd;
	struct {
#ifdef CONFIG_CPU_BIG_ENDIAN
      uint32_t resv:1;          //reserved bit
      uint32_t ws:1;              //!<  [30..30] Write data from Non-Secure/Secure master port: 1=Secure port, 0=Non-secure port
      uint32_t fs:1;            //First segment descriptor
      uint32_t ls:1;            //Last segment descriptor
      uint32_t enc:1;           //Encryption Flag
      uint32_t resv2:19;             //Reserved bits
      uint32_t adl:8;               //Authentication Data Length
#else
      uint32_t adl:8;               //Authentication Data Length
      uint32_t resv2:19;             //Reserved bits
      uint32_t enc:1;           //Encryption Flag
      uint32_t ls:1;            //Last segment descriptor
      uint32_t fs:1;            //First segment descriptor
      uint32_t ws:1;              //!<  [30..30] Write data from Non-Secure/Secure master port: 1=Secure port, 0=Non-secure port
      uint32_t resv:1;          //reserved bit
#endif
	}bits;
}rtk_fc_cryptoEngine_dest_type1_t;

typedef union {
	uint32 wrd;
	struct {
#ifdef CONFIG_CPU_BIG_ENDIAN
      uint32_t resv:1;          //reserved bit
      uint32_t ws:1;              //!<  [30..30] Write data from Non-Secure/Secure master port: 1=Secure port, 0=Non-secure port
      uint32_t fs:1;            //First segment descriptor
      uint32_t ls:1;            //Last segment descriptor
      uint32_t enc:1;           //Encryption Flag
      uint32_t resv2:3;              //Reserved bits
      uint32_t enl:24;              //Encryption Length
#else
      uint32_t enl:24;              //Encryption Length
      uint32_t resv2:3;              //Reserved bits
      uint32_t enc:1;           //Encryption Flag
      uint32_t ls:1;            //Last segment descriptor
      uint32_t fs:1;            //First segment descriptor
      uint32_t ws:1;              //!<  [30..30] Write data from Non-Secure/Secure master port: 1=Secure port, 0=Non-secure port
      uint32_t resv:1;          //reserved bit
#endif
	}bits;
}rtk_fc_cryptoEngine_dest_type2_t;

union rtk_fc_cryptoEngine_dest_ctrl_u{
      rtk_fc_cryptoEngine_dest_type1_t type1;
      rtk_fc_cryptoEngine_dest_type2_t type2;
};

typedef union rtk_fc_cryptoEngine_dest_ctrl_u rtk_fc_cryptoEngine_dest_ctrl_t;

typedef struct rtk_fc_cryptoEngine_dest_desc_s {
      rtk_fc_cryptoEngine_dest_ctrl_t ctrl;
      uint32_t ddbp;
}rtk_fc_cryptoEngine_dest_desc_t;


typedef struct rtk_fc_cryptoEngine_keypad_lock_s {
#ifdef CONFIG_CPU_BIG_ENDIAN
      uint32_t lock:1;          //This bit is used to control the write lock of keypad
      uint32_t resv:26;              //Reserved bits
      uint32_t len:5;              //Secure keypad length
#else
      uint32_t len:5;              //Secure keypad length
      uint32_t resv:26;              //Reserved bits
      uint32_t lock:1;          //This bit is used to control the write lock of keypad
#endif
}rtk_fc_cryptoEngine_keypad_lock_t;

typedef struct rtk_fc_ipsec_hash_padding_array_s{
	uint8 hash_padding[96] __attribute__((aligned(32)));
	uint32 hash_padlen;
	
}rtk_fc_ipsec_hash_padding_array_t;

typedef struct rtk_fc_crypto_key_usage_s{
	u8 key[64];
	int key_size;
	int valid;
}rtk_fc_crypto_key_usage_t;


typedef enum rtk_fc_flow_proto_ctrl_e
{
	FLOW_PROTO_CTRL_NONE=0,
		
	FLOW_PROTO_CTRL_DUAL_PT,
	FLOW_PROTO_CTRL_V6NAT,
	FLOW_PROTO_CTRL_NPTV6_SFF,		// 9607C special fast forward
	FLOW_PROTO_CTRL_VXLAN_ADD,			// support vxlan
	FLOW_PROTO_CTRL_VXLAN_REMOVE,			// support vxlan
	FLOW_PROTO_CTRL_LOOPBACK_ACC,	// support single header bridge case.
	FLOW_PROTO_CTRL_ICMP_ACC,			// support single header routing case.
	FLOW_PROTO_CTRL_MAPE_ACC,			// support mape inner v4 address value.
	FLOW_PROTO_CTRL_MAPT_ACC,			// support mapt transformation case.
	FLOW_PROTO_CTRL_XLAT_ACC,			// support xlat transformation case.	
	FLOW_PROTO_CTRL_IPSEC,
	FLOW_PROTO_CTRL_IPSEC_ENDPOINT,
	
} rtk_fc_flow_proto_ctrl_t;
typedef struct rtk_fc_l2Dual_info_s
{
	uint16 out_src_port;						// rtk_fc_l2DualTbl_t, 				include	vlan / gre_ethbr
	uint16 sw_in_intf_idx;
	uint16 sw_out_intf_idx;
#if defined(CONFIG_FC_CA8277B_SERIES)
	uint16 vxlan_extra_index;
#endif
#if defined(CONFIG_FC_CA8277B_SERIES)||defined(CONFIG_FC_RTL9607C_SERIES)
	uint8 is_outer_flow;
#endif

}rtk_fc_l2Dual_info_t;


typedef struct rtk_fc_ipsec_flow_info_s{
	int ipsec_shortCut_info_table_index;
	uint32 spi;
	uint8 ipsec_direction;
}rtk_fc_ipsec_flow_info_t;

typedef struct rtk_fc_tableFlow_s
{
	rtk_fc_tableFlowEntry_t *pFlowEntry;
	rtk_fc_abstrSwFlowList_entry_t *pAbstrSwFlowEt;				//
#if defined(CONFIG_FC_FLOW_AUTO_EXTEND)
	struct net_device *srcDev;
	rtk_fc_igrDummyData_t *dummyPkt;
#endif
#if defined(CONFIG_RTK_L34_G3_PLATFORM)	
	int16 ingressSaHostPolIdx; 													//host policing entry index that matches flow ingress SA (-1 means ingress SA does not hit any host policing entry)
	int16 egressDaHostPolIdx; 													//host policing entry index that matches flow egress DA (-1 means egress DA does not hit any host policing entry)
#if !(defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))	// 8277C/9607F mainHash index should the same with SW flow index, no need to strore mainHashIdx
	uint32 mainHashIdx;															// the hw index saving in G3 MainHash Table, or G3_FLOWIDX_INVALID statnds for FC sw only
#endif
#endif

	//-------------------This Block will be memset to 0 when delete---------------------------------------------//
	struct nf_conn *cachedCt;							// always get ct by "rtk_fc_flow_ct_get(&flowTable)"
	rt_flow_extra_info_t flow_extra_info;				// rt_flow_extra_info_t, check by FLOW_INFO_IS_SET
	___ALIGN___	
	int16 lutIgrSaIdx:(MAX_FLOW_LUT_IDX_WIDTH+1);	// (14b) -1 to indicate didn't care. [ingress sa] for lut sync usage
	int16 lutEgrDaIdx:(MAX_FLOW_LUT_IDX_WIDTH+1);	// (14b) -1 to indicate didn't care. [egress da] for lut sync usage
#if defined(CONFIG_RTK_L34_CANDIDATE_FLOW)
	uint32 candidateState:2;							// rtk_fc_candidate_state_t
#endif
	uint32 swTrfBit:1;									// traffic bit for non-HW flow entries
	uint32 needUpdate:1;								// should be deleted when this flow's idle time is not zero
	___ALIGN___
	uint32 idleSecs:MAX_FLOW_IDLESEC_WIDTH;  		// (12b) Flow entry idle time, for non-CT entry timeout mechanism
	uint32 pon_stream_id:8;							// flow key: pon steam id as flow key
	uint32 igr_stpid_sel:1;
	uint32 egr_stpid_sel:1;
	uint32 svlan_pri:3;
	uint32 igr_svlan_dei:1;
	uint32 egr_svlan_dei:1;
	uint32 igr_cvlan_cfi:1;
	uint32 egr_cvlan_cfi:1;
	uint32 egr_tos_ecn_remark:1;
	uint32 egr_tos_ecn:2;
	___ALIGN___ 	// -1
	uint32 swShaperEn:SWSHAPER_TYPE_MAX;			// (3b) rtk_fc_swShperType_t
	uint32 flow_hit_times:RTK_FC_HIT_NUMBER_TO_DELAY_BITS;	// (6b)
	uint32 vlan_filtering_pvid_lutivlsel:RTK_FC_TABLESIZE_LUT_IVL_BITS;		// valid if  flow_extra_info.FLOW_INFO_VLAN_FILTERING_PVID_ENTRY assert; refresh (ivl) fdb idle time by {lutIgrSaIdx, pvid}; 
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
	uint32 dmaAftFibCtrlEn:1;
	uint32 dmaAftFibIdx:RTK_FC_DMAAFTACTION_SIZE_BITS;
#endif
#if defined(CONFIG_FC_RTL9607F_SERIES)
	uint32 dmaAftMapCtrlEn:1;		//0 means fail to use DMAAFT MAP entry, should not use new format(hp=11)
	uint32 dmaAftMapIdx:RTK_FC_DMAAFTMAP_SIZE_BITS;
#endif
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	int32 mapeMaptFmrIdx:(2+1);						// used for mape/mapt upstream decide FMR table, -1 means don't care



	___ALIGN___
	uint32 crc32;
	uint16 crc16;									// CRC16 is necessary for overflow entry search
#endif	
	uint16 pppoe_sid; 								// for pppoe passthrough session ID comparision (path34.in_pppoe_sid_check is 1=> compare igr_netif.pppoe_sid, else compare flow.pppoe_sid)
	___ALIGN___	// -2

	/* === Special Protocol Handling === */
	rtk_fc_flow_proto_ctrl_t protoCtrl;					// rtk_fc_flow_proto_ctrl_t
	union {
		// table
		uint32 dual_pt_flowMapTbl_idx:RTK_FC_DUAL_PASSTHROUGH_FLOWMAPPING_SIZE_BITS;			// rtk_fc_dual_passthrough_flowMapping_t, include 	pptp / l2pt / ipsec
		uint32 ipv6_nat_indirect_mapping_index:RTK_FC_TABLESIZE_I6NAT_MAPPING_TABLE_SHIFT_BITS;	// rtk_fc_ipv6_nat_mappingTbl_t, 		include	nptv6 / nat66
		int16 nptv6_info_index;																	// rtk_fc_nptv6_fastFwd_record_t, 1: there exists a flow which has the same pattern with this flow, but has different isHitSwFwdedAclTrapPri value
		// value
		rtk_fc_icmpCtrlInfo_t icmpInfo;
		int32 loopbackRevFlowIdx;			// flow index for WAN dowmstream loopback reverse flow, -1 means there's no loopback flow for this flow
		rtk_fc_mapeInfo_t mapeInfo;
		rtk_fc_maptIpv6Info_t maptInfo;		//for mapt and xlat464
		rtk_fc_l2Dual_info_t l2Dual_table_info;
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)		
		rtk_fc_ipsec_flow_info_t ipsec_flow_info;
#endif
	};
	uint8 path5_ds_bucket_idx;
	/* ======================== */
		
	//-------------------Above Block will be memset to 0 when delete---------------------------------------------//
} __attribute__((packed)) rtk_fc_tableFlow_t;

typedef struct rtk_fc_flowTcam_linkList_s
{
	uint16 idx;

	struct list_head flowTcam_list;
}rtk_fc_flowTcam_linkList_t;


#if defined(FC_F_SWFLOWLIST_IDX_MODE) //====================================================
//2 sw flow list by idx(uint16)
#define SWFLOW_LIST_END		(1<<CONFIG_RTK_FC_SW_FLOW_ENTRY_BITS)		// entry&value 32768 to indicate the end.
#define SWFLOW_LIST_ENT_CNT	(fc_db.flowSwTableSize-fc_db.flowHwTableSize)

#if (CONFIG_RTK_FC_SW_FLOW_ENTRY_BITS<16)		// default
typedef uint16 SWFLOW_NEXT_IDX_t;
#else
typedef uint32 SWFLOW_NEXT_IDX_t;
#endif
struct swflow_idx_node_s
{
	union {
		SWFLOW_NEXT_IDX_t first;			// for head
		SWFLOW_NEXT_IDX_t next;			// for node
	};
};
typedef struct swflow_idx_node_s SWFLOW_LIST_HEAD_t;
typedef struct swflow_idx_node_s SWFLOW_LIST_NODE_t;

#define SWFLOW_LIST_HEAD_INIT(list_head)	(((list_head)->first) = SWFLOW_LIST_END)
#define SWFLOW_LIST_NODE_INIT(list_node)	(((list_node)->next) = SWFLOW_LIST_END)
#define SWFLOW_LIST_FIRST_NODE(list_head)		(((list_head)->first))
#define SWFLOW_LIST_EMPTY(list_head)		(((list_head)->first) == SWFLOW_LIST_END)

#define SWFLOW_LIST_FOR_EACH_ENTRY(pos, head, member)				\
	for ((pos) = &fc_db.swFlowList[SWFLOW_LIST_FIRST_NODE(head)];	\
	     pos!=NULL && (pos!=&fc_db.swFlowList_end);			\
	     (pos) = (pos->member.next!=SWFLOW_LIST_END) ? &fc_db.swFlowList[pos->member.next] : &fc_db.swFlowList_end )

#define SWFLOW_LIST_FOR_EACH_ENTRY_SAFE(pos, n, head, member)			       \
	for ((pos) = &fc_db.swFlowList[SWFLOW_LIST_FIRST_NODE(head)];		       \
	     pos!=NULL && (pos!=&fc_db.swFlowList_end) &&			       \
	        (n = (pos->member.next!=SWFLOW_LIST_END) ? &fc_db.swFlowList[pos->member.next] : &fc_db.swFlowList_end); \
	     pos = n)

#define SWFLOW_LIST_ADD(new, list_head)\
do { \
	rtk_fc_swFlow_linkList_t *new_swFlow = container_of(new, rtk_fc_swFlow_linkList_t, flow_list);\
	(new)->next = (list_head)->first;\
	(list_head)->first = (new_swFlow->idx-fc_db.flowHwTableSize);\
} while(0)

#define SWFLOW_LIST_DEL(target, list_head)\
do { \
	rtk_fc_swFlow_linkList_t *target_swFlow = container_of(target, rtk_fc_swFlow_linkList_t, flow_list);\
	SWFLOW_LIST_NODE_t *node = (SWFLOW_LIST_NODE_t *)list_head;	\
	while (node->next != (target_swFlow->idx-fc_db.flowHwTableSize))	\
		node = &fc_db.swFlowList[node->next].flow_list;	\
	node->next = (target)->next; \
}while(0)

#else //====================================================================================
//2 sw flow list by link list
#define SWFLOW_LIST_END		(&fc_db.swFlow_hashListHead[i])
#define SWFLOW_LIST_ENT_CNT	(fc_db.flowSwTableSize-fc_db.flowHwTableSize)
typedef struct list_head SWFLOW_LIST_HEAD_t;
typedef struct list_head SWFLOW_LIST_NODE_t;
#define SWFLOW_LIST_HEAD_INIT					INIT_LIST_HEAD
#define SWFLOW_LIST_NODE_INIT					INIT_LIST_HEAD
#define SWFLOW_LIST_EMPTY						list_empty
#define SWFLOW_LIST_FOR_EACH_ENTRY			list_for_each_entry
#define SWFLOW_LIST_FOR_EACH_ENTRY_SAFE		list_for_each_entry_safe
#define SWFLOW_LIST_ADD(new, list_head)        	do { list_add_tail(new, list_head); } while(0)
#define SWFLOW_LIST_DEL(node, head)            		do { list_del_init(node); } while(0)
#endif //===================================================================================


typedef struct rtk_fc_swFlow_linkList_s
{
	uint32 idx;						// hw entry cnt + N;
	SWFLOW_LIST_NODE_t flow_list;
} __attribute__((packed))rtk_fc_swFlow_linkList_t;

#if defined(CONFIG_FC_RTL8277C_SERIES)
typedef struct rtk_fc_overFlowHash_linkList_s
{
	uint32 idx;

	struct list_head flow_list;
}rtk_fc_overFlowHash_linkList_t;
#endif

#if defined(CONFIG_RTK_L34_CANDIDATE_FLOW)
typedef enum rtk_fc_candidate_state_e{
	CANDIDATE_STATE_NONE = 0,
	CANDIDATE_STATE_NEW = 1,			// first packet		(adding to HW but disable valid bit)
	CANDIDATE_STATE_READY = 2,		// second packet 	(adding to HW by turning on valid bit)
	CANDIDATE_STATE_AGING = 3,		// aging, will be deleted if shortcut hits.	BE CAREFUL: MAX, 2 bits only
}rtk_fc_candidate_state_t;
#endif

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
typedef enum rtk_fc_g3_flow_add_state_e{
	G3_FLOW_ADD_STATE_HW = 0,
	G3_FLOW_ADD_STATE_PURE_SW,
	G3_FLOW_ADD_STATE_FAIL,
}rtk_fc_g3_flow_add_state_t;
#endif
 
typedef struct rtk_fc_netif_vxlan_info_s
{
	
	uint32 vxlan_vni;
	unsigned char vxlan_DA[6];
	union{
		struct in6_addr outer_v6Ip;
		uint32 outer_ip;
	};
	union{
		struct in6_addr outer_remote_v6Ip;
		uint32 outer_remote_ip;
	};
	//uint16 outer_sa_idx;
	uint16 outer_pppoe_sid;
	uint16 outer_svlan_id;
	uint16 outer_cvlan_id;
	uint8 outer_tag_len;
	uint8 outer_ctag_pri:3;
	uint8 outer_srcCVlanCfi:1;
	uint8 outer_stag_pri:3;
	uint8 outer_srcSVlanDei:1;
	uint8 outer_is_v6:1;
	uint8 outer_stag_if:1;
	uint8 outer_ctag_if:1;
	uint8 outer_srcSVlanTpid_sel:1;
	uint8 outer_pppoe_tag:1;
	uint8 valid:1;
}rtk_fc_netif_vxlan_info_t;


/* outer v6 inner v4 (dslite/lw4o6/mape)*/
typedef struct rtk_fc_netif_4In6_info_s
{
	uint32 isMape:1;			// Use the isMape field to distinguish the downstream between Ds-Lite(non-NAT) and MAP-E(NAPT)
	struct in6_addr remote6Addr;
	struct in6_addr local6Addr;
}
rtk_fc_netif_4In6_info_t;

/* outer v4 inner v6 (6rd/)*/
typedef struct rtk_fc_netif_6In4_info_s
{
	struct in_addr remoteAddr;
	struct in_addr localAddr;
	uint32 upstream_seen;
}
rtk_fc_netif_6In4_info_t;

typedef struct rtk_fc_dualHashKey_s
{
	uint32 local_ip_addr[4];	//if you need mask ,please netMask ipaddress before write to this field
	uint32 remote_ip_addr[4];   //if you need mask ,please netMask ipaddress before write to this field
	uint32 otherParameter[4];	//call id

	//======== always hash below =========
	int16	cvlanId;			//SIGNED_INVALID => untag
	int16	svlanId;			//SIGNED_INVALID => untag
	uint8	dualType;
}
rtk_fc_dualHashKey_t;

typedef struct rtk_fc_callBackRegFunc_s
{
	/* 
		fill your dual patten
		imagine patten is ingresCLS outer patten or path6 keys
		Careful: should reference dual_direct to get local_ip_addr/remote_ip_addr
	*/
	void (*dual_hash_patten_fill)(rtk_fc_dualHashKey_t* patten,rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct);
	/*
		dual_direct = RTK_FC_FLOW_DUAL_DIRECTION_UPSTREAM_EGR/RTK_FC_FLOW_DUAL_DIRECTION_DOWNSTREAM_IGR
		Packet patten hit netif we have all info from packet content ,update dual-info to swNetif.
		if you want get some info from struct net_dev(dev->netdev_priv) store them in the fucntion.
	*/
	int (*dual_netifInfo_collect)(uint32 swNetifIdx,rtk_fc_flow_dual_direction_t dual_direct, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr);

	/* 
		FC egress add dual-hardware 
		downstream/upstream hardware add share this function
		There will be differences between the upstream and downstream in the pPktHdr and skb->fcIngressData .
	*/
	int (*dual_hw_setting)(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, int hwNetifIdx);
}
rtk_fc_dual_callBackRegFunc_t;


#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
typedef struct rtk_fc_hwMcNetifMib_s
{
	uint32 in_intf_mc_packet_cnt;
	uint64 in_intf_mc_byte_cnt;
}rtk_fc_hwMcNetifMib_t;
#endif

typedef struct rtk_fc_tableNetif_s
{
	uint32 psIfidxStackKey;		//uint8 [ifidx3][ifidx2][ifidx1][ifidx0]
	rtk_rg_asic_netif_entry_t intf;

	//================ dual-info ==========================
	rtk_fc_dualHdrtype_t dualType;
	uint32 dualHashKey;
	uint8 dualUniInfo_syncedUP:1;		// avoid to update dual-info every time
	uint8 dualUniInfo_syncedDOWN:1;		// avoid to update dual-info every time
	uint8 dualHdr_buf_insert_pos;	// refer to rtk_fc_dualHdr_buf_insert_pos_t
 	// dual netif protocol detail info
	union
	{
		rtk_fc_netif_vxlan_info_t vxlan_netif_info;		//RTK_FC_DUALTYPE_VXLAN
		rtk_fc_netif_4In6_info_t ip4Inip6_info;			//RTK_FC_DUALTYPE_IP4INIP6
		rtk_fc_netif_6In4_info_t ip6Inip4_info;			//RTK_FC_DUALTYPE_IP4INIP6
		rtk_fc_netif_psGre_Info_t psGreInfo;			//RTK_FC_DUALTYPE_PPTP
		rtk_fc_netif_l2tp_Info_t l2tpInfo;				//RTK_FC_DUALTYPE_L2TP
		rtk_fc_netif_greEthBr_info_t greEthBrInfo;		//RTK_FC_DUALTYPE_GRE_ETH_BR
		rtk_fc_netif_mapt_Info_t maptInfo;				//RTK_FC_DUALTYPE_MAPT
		rtk_fc_netif_xlat464_Info_t xlatInfo;			//RTK_FC_DUALTYPE_XLAT
	}dualUniInfo;

	uint8 dualHdr_downstream_earlyChk:1; //owner sholud setting:  /dualHdr_outer_ipversion/dualHdr_outer_L3_Offset/dualHdr_outer_length
	uint8 dualHdr_outer_ipversion:1;	 //0:v4, 1:v6
	uint8 dualHdr_outer_L3_Offset;		 //only valid when dualHdr_buf_insert_pos is FC_DUALHDR_HW_CTRL_PKT_BEGINIG
	uint8 dualHdr_outer_length;			 //zero: not ready to enter dual early check

	//for RTK_FC_DUALTYPE_PPTP/RTK_FC_DUALTYPE_L2TP/RTK_FC_DUALTYPE_GRE_ETH_BR 
	uint16 ipid;

	//=============== Hw-depend-info =====================
	int16 lutIdx;				// SIGNED_INVALID (-1) no lutidx
	int8 hwIdx;					// SIGNED_INVALID (-1) indicate sw only
	int16 pppoeGwLutIdx;		// lut index of pppoe's gateway(nexthop), -1 indicate invalid
	int8 outerHdrExtratagIdx;		//init to SIGNED_INVALID(-1) 9607C series valid if >0, 8277C series if >=0, ,it's for egress usage.
	uint8 use_two_extraTag;		// For 2 extraTag usage, e.g. vxlan outer ipv6
	int32 outerHdrFlowIdx;			// valid if !=SIGNED_INVALID (-1), it's for ingress usage

	uint8 hwEntryNum;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	uint8 dualHdr_HWState;		// refer to rtk_fc_dualHeaderHWState_t
	uint32 dualHdr_ds_clsIdx;	// downstream dual header CLS index, CA_UINT32_INVALID indicate invalid
	uint32 dualHdr_ds_extra_clsIdx; // downstream dual header CLS index, CA_UINT32_INVALID indicate invalid
	rtk_fc_hwMcNetifMib_t hwMcNetifMib;
	int8 hwFmrIdx;			// SIGNED_INVALID (-1)indicate invalid
#endif	
	rtk_rg_asic_netifMib_entry_t intfMib[NR_CPUS];

}rtk_fc_tableNetif_t;


typedef struct rtk_fc_hwTableNetifPointer_s
{
	uint8 hwNetifValid:1;
	uint8 swNetifIdx:7;					//point to rtk_fc_tableNetif_t netifTbl
}rtk_fc_hwTableNetifPointer_t;


typedef struct rtk_fc_devGwMac_s
{
	struct net_device *dev;
	char macportidx;
	char macextportidx;
	char wlanidx;
	uint8 forcePortConfig:1;
	uint8 isXLAT464:1;	//for ip4ToIp6 protocol decision, 0:MAPT 1:XLAT464
	uint8 maptInfoValid:1;	//for MAPT DMR
	uint8 maptDraftVer:1;	//for MAPT DMR
	uint8 maptPrefxLen:7;	//for MAPT DMR
	int8 mapet_fmr_hwidx_valid:1;
	int8 mapet_fmr_hwidx:2;	//for MAPE/MAPT FMR
	int16 hwlutIdx;						//init to SIGNED_INVALID
	int8 myMacIdx;						//init to SIGNED_INVALID

	//for RTK_FC_DUALTYPE_PPTP/RTK_FC_DUALTYPE_L2TP/RTK_FC_DUALTYPE_GRE_ETH_BR
	rtk_fc_psFragIPID_t psFragIPID[MAX_SW_FRAGIPID_SIZE];
}rtk_fc_devGwMac_t;


typedef struct rtk_fc_netifDummyPacket_s
{
	rtk_fc_igrDummyData_t *dummyPkt;
	uint32 dummyPktInit:1;
}rtk_fc_netifDummyPacket_t;


typedef struct rtk_fc_tableIndMac_s
{
	uint8 valid;
	uint8 hwIdx;
	uint32 indMacRefCount;
	rtk_rg_asic_indirectMac_entry_t indMac;

	struct list_head hashList;
}rtk_fc_tableIndMac_t;

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
typedef struct rtk_fc_wifi_dev_mask_s
{
	union {
		rtk_fc_wlan_devmask_t extpmask;			// for compiler
		rtk_fc_wlan_devmask_t wlandevmask;		// real definition
	};
}rtk_fc_wifi_dev_mask_t;
#endif

typedef struct rtk_fc_tableExtPort_s
{
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	rtk_rg_asic_extPortMask_entry_t extPortEnt;
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	union {
		rtk_fc_wifi_dev_mask_t extPortEnt;			// for compiler
		rtk_fc_wifi_dev_mask_t wifiDevMaskEnt;		// real definition
	};
#endif
	int32 extPortRefCount;
}rtk_fc_tableExtPort_t;

typedef struct rtk_fc_tableExtraTag_s
{
	uint8 valid;
	uint8 bufferOff;
	uint8 contentLen;
	rtk_rg_asic_extraTagAction_t actions[RTK_FC_TABLESIZE_EXTRATAG_ACTIONS]; 
}rtk_fc_tableExtraTag_t;

/*****MAP-E IPv6 dst addresses in upstream*****/
#define RTK_FC_MAPE_FMR_DST6_MAX_NUMS 64
typedef struct rtk_fc_mape_fmr_dst6_s{
	u16 index;
	u32 ref_cnt;//add flow5: ref_cnt++, del flow5: ref_cnt--
	struct in6_addr d_addr6;//IPv6 dst addr
	struct list_head list;
}rtk_fc_mape_fmr_dst6_t;

typedef struct rtk_fc_mape_dst6_s{
	uint8 in_used;
	rtk_fc_mape_fmr_dst6_t dst6_info[RTK_FC_MAPE_FMR_DST6_MAX_NUMS];
	struct list_head used_list;
	struct list_head free_list;
}rtk_fc_mape_dst6_t;
/*****MAP-E IPv6 dst addresses in upstream*****/


typedef struct rtk_fc_igmp_timer_s
{

	int nextTimerPeriod;		//ms
	rtk_fc_timer_list_t *igmpTimer;
	unsigned long stopjiffies;
} rtk_fc_igmp_timer_t;

typedef struct rtk_fc_lutCam_linkList_s
{
	uint16 idx;		//from LUTTABLE_SRAM_SIZE to LUTTABLE_SRAM_SIZE+LUTTABLE_BCAM_SIZE

	struct list_head __rcu lut_list;
}rtk_fc_lutCam_linkList_t;

// =========== RG to RG-FC =========== //
// |||							 		|||//


typedef enum rtk_fc_toPsReason_e
{
	FC_DBUG_TOPSREASON_PKTPARSE_FAIL	= 0x1,
	FC_DBUG_TOPSREASON_BYPASS_ETHTYPE	= 0x2,
	FC_DBUG_TOPSREASON_UNKNOWN_SMAC		= 0x4,
	FC_DBUG_TOPSREASON_CPU_REASON		= 0x8,
	FC_DBUG_TOPSREASON_SKIP_SHORTCUT	= 0x10,
	FC_DBUG_TOPSREASON_DIS_SWFWD		= 0x20,
	FC_DBUG_TOPSREASON_SHORTCUT_DPI		= 0x40,
	FC_DBUG_TOPSREASON_FLOWIDX_FAIL		= 0x80,
	FC_DBUG_TOPSREASON_SHORTCUT_MISS	= 0x100,
	FC_DBUG_TOPSREASON_COPY2CPU			= 0x200,	
}rtk_fc_toPsReason_t;

typedef enum rtk_fc_debugLevel_e
{
	FC_DEBUG_LEVEL_NONE=0x0,
	FC_DEBUG_LEVEL_DEBUG=0x1,
	FC_DEBUG_LEVEL_FIXME=0x2,
	FC_DEBUG_LEVEL_CALLBACK=0x4,
	FC_DEBUG_LEVEL_TRACE=0x8,
	//FC_DEBUG_LEVEL_ACL=0x10,
	FC_DEBUG_LEVEL_WARN=0x20,
	FC_DEBUG_LEVEL_TRACE_DUMP=0x40,
	FC_DEBUG_LEVEL_EVENT=0x80,
	//FC_DEBUG_LEVEL_MACLEARN=0x100,
	FC_DEBUG_LEVEL_TABLE=0x200,
	FC_DEBUG_LEVEL_ALG=0x400,
	FC_DEBUG_LEVEL_IGMP=0x800,
	FC_DEBUG_LEVEL_ACL_RRESERVED=0x1000,
	FC_DEBUG_LEVEL_API=0x2000,
	FC_DEBUG_LEVEL_TIMER=0x4000,
	FC_DEBUG_LEVEL_PS=0x8000,
	FC_DEBUG_LEVEL_IGR=0x10000,
	FC_DEBUG_LEVEL_EGR=0x20000,
	FC_DEBUG_LEVEL_WIFI=0x40000,
	FC_DEBUG_LEVEL_ACL_CONTROL_PATH=0x80000,
	FC_DEBUG_LEVEL_DSLITE=0x100000,
	FC_DEBUG_LEVEL_TOPS=0x200000,
	FC_DEBUG_LEVEL_FRAGMENT=0x400000,
	FC_DEBUG_LEVEL_IPSEC   			= 0x1000000,
	FC_DEBUG_LEVEL_IPSEC_HOOK   		= 0x2000000,
	FC_DEBUG_LEVEL_TOPSTRACE=0x4000000,	
	FC_DEBUG_LEVEL_ALL=0xffffffff,
} rtk_fc_debugLevel_t;


typedef enum rtk_fc_debug_trace_filter_bitmask_e
{
	FC_DEBUG_TRACE_FILTER_SPA=0x1, //source port 0~5 phyiscal Port, 6:CPU, 7:EXT0, 8:EXT1
	FC_DEBUG_TRACE_FILTER_DA=0x2,	//DNAC
	FC_DEBUG_TRACE_FILTER_SA=0x4,	//SMAC
	FC_DEBUG_TRACE_FILTER_ETH=0x8, //ethertype
	FC_DEBUG_TRACE_FILTER_SIP=0x10, //src IP
	FC_DEBUG_TRACE_FILTER_DIP=0x20, //dest IP
	FC_DEBUG_TRACE_FILTER_IP=0x40, //src IP or dest IP
	FC_DEBUG_TRACE_FILTER_L4PROTO=0x80, // Layer4 protocol
	FC_DEBUG_TRACE_FILTER_SPORT=0x100, // L4 Src Port
	FC_DEBUG_TRACE_FILTER_DPORT=0x200, // L4 Dst Port
	FC_DEBUG_TRACE_FILTER_REASON=0x400, // Trap reason
	FC_DEBUG_TRACE_FILTER_CVLAN=0x800, // CVLAN
	FC_DEBUG_TRACE_FILTER_SVLAN=0x1000, // SVLAN
	FC_DEBUG_TRACE_FILTER_PPPOESESSIONID=0x2000, // PPPOE sessionID
	FC_DEBUG_TRACE_FILTER_V6DIP=0x4000, // IPV6 DIP
	FC_DEBUG_TRACE_FILTER_V6SIP=0x8000, // IPV6 SIP
	FC_DEBUG_TRACE_FILTER_SHOWNUMBEROFTIMES=0x10000, //TRACEFILTER SHOW NUMBER OF TIMES
	FC_DEBUG_TRACE_FILTER_DUMMYMC=0x20000, // DUMMY MC PACKET
	FC_DEBUG_TRACE_FILTER_WLAN_INDEX=0x40000, // WLAN INGRESS INDEX
	FC_DEBUG_TRACE_FILTER_FLOWINDEX=0x80000, // WLAN INGRESS INDEX
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	FC_DEBUG_TRACE_FILTER_LDPID=0x100000, // WLAN INGRESS INDEX
#endif
	FC_DEBUG_TRACE_FILTER_SIP_RANGE=0x200000, //src IP range
	FC_DEBUG_TRACE_FILTER_DIP_RANGE=0x400000, //dst IP range
	FC_DEBUG_TRACE_FILTER_V6SIP_RANGE=0x800000, //src IPV6 range
	FC_DEBUG_TRACE_FILTER_V6DIP_RANGE=0x1000000, //dst IPV6 range
	FC_DEBUG_TRACE_FILTER_LEN_RANGE=0x2000000, //len range
	FC_DEBUG_TRACE_FILTER_TCP_FLAGS_MSK=0x4000000, //TCP flags mask
} rtk_fc_debug_trace_filter_bitmask_t;


typedef enum rtk_fc_test_mode_e
{
	
	FC_TEST_MODE_DISABLE=0,
	FC_TEST_MODE_ENABLE,
	FC_TEST_MODE_MAX,
} rtk_fc_test_mode_t;

typedef struct rtk_fc_debugTraceFilter_s
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
	uint16		dport;
	uint8		reason;
	uint8		wlan_index;
	uint16		flowindex;
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
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	uint32 		ldpid;
#endif
}rtk_fc_debugTraceFilter_t;
#define TRACFILTER_MAX 4
#define FLOWDELTRACE_MAX 4


#define MAX_VLAN_HW_TABLE_SIZE (4096)

typedef struct rtk_fc_vlanGroupMacLimit_info_s
{
	unsigned int vlanMask[MAX_VLAN_HW_TABLE_SIZE>>5];
	unsigned int untag:1;

	unsigned int valid:1;

	int port;
	int mac_limit_number;
	atomic_t mac_count;
}rtk_fc_vlanGroupMacLimit_info_t;


typedef struct rtk_fc_statistic_s
{
	//Packet Type
	atomic_t perPortCnt_broadcast[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_multicast[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_unicast[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_UDP[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_TCP[RTK_FC_MAC_PORT_MAX];

	atomic_t perPortCnt_sc_UDP[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_sc_TCP[RTK_FC_MAC_PORT_MAX];

	atomic_t perPortCnt_egress_UDP[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_egress_TCP[RTK_FC_MAC_PORT_MAX];

	//Fragment
	atomic_t perPortCnt_fragment[RTK_FC_MAC_PORT_MAX];
	
	//TCP Flags
	atomic_t perPortCnt_SYN[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_SYN_ACK[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_FIN[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_FIN_ACK[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_FIN_PSH_ACK[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_RST[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_RST_ACK[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_ACK[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_PUSH_ACK[RTK_FC_MAC_PORT_MAX];
	
	atomic_t perPortCnt_sc_SYN[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_sc_SYN_ACK[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_sc_FIN[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_sc_FIN_ACK[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_sc_FIN_PSH_ACK[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_sc_RST[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_sc_RST_ACK[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_sc_ACK[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_sc_PUSH_ACK[RTK_FC_MAC_PORT_MAX];
	
	atomic_t perPortCnt_egress_SYN[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_egress_SYN_ACK[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_egress_FIN[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_egress_FIN_ACK[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_egress_FIN_PSH_ACK[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_egress_RST[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_egress_RST_ACK[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_egress_ACK[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_egress_PUSH_ACK[RTK_FC_MAC_PORT_MAX];

	//Reason
	atomic_t perPortCnt_Reason[256][RTK_FC_MAC_PORT_MAX]; //TTL

	//forwarding
	atomic_t perPortCnt_shortcut[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_shortcutV6[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_shortcut_icmp4[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_shortcut_icmp6[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_shortcut_ipsec_encrypt[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_shortcut_ipsec_decrypt[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_shortcut_ipsec_decrypt_hwlookup[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_shortcut_ipsec_decrypt_hwlookup_miss[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_shortcut_ipsec_ipi_drop[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_shortcut_ipsec_ipi_enqueue[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_shortcut_ipsec_desc_not_enough[RTK_FC_MAC_PORT_MAX];

#if defined(FC_F_SHORTCUT_EARLYCHECK)
	atomic_t perPortCnt_shortcut_earlycheck[RTK_FC_MAC_PORT_MAX];
#endif


	atomic_t perPortCnt_L2FWD[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_IPv4_L3FWD[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_IPv6_L3FWD[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_L4FWD[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_Drop[RTK_FC_MAC_PORT_MAX][RTK_FC_RET_DROP_END-RTK_FC_RET_DROP];
	atomic_t perPortCnt_ToPS_unicast[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_ToPS_multicast[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_ToPS_broadcast[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_FromPS_unicast[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_FromPS_multicast[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_FromPS_broadcast[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_LocalOut[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_dummpPkt[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_dummpPktAlloc[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_ipsec_change_seq_num[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_ipsec_change_seq_num_drop[RTK_FC_MAC_PORT_MAX];

	atomic_t perPortCnt_L2LRU[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_FlowLRU[RTK_FC_MAC_PORT_MAX];
	/*
	//nicTx, wifiTx
	uint32 perPortCnt_NIC_TX[RTK_FC_MAC_PORT_MAX];
	uint32 perPortCnt_WIFI_TX[RTK_FC_MAC_PORT_MAX];
	*/
	//masterWifiFastForward
	atomic_t perPortCnt_MWFF_TX[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_NPTv6_FF_TX[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_NPTv6_NIC_FF_TX[RTK_FC_MAC_PORT_MAX];
	
	atomic_t perPortCnt_ipFrag[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_ipFrag_cached[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_hit_ipFrag_cache[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_ipFrag_cache_tbl_full[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_shortcut_ipFrag[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_shortcut_ipFrag_fail[RTK_FC_MAC_PORT_MAX];
	
	atomic_t perPortCnt_negative_ipFrag_cached[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_hit_negative_ipFrag_cache[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_negative_ipFrag_cache_tbl_full[RTK_FC_MAC_PORT_MAX];

	atomic_t perPortCnt_ackDelayEQ[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_ackDelayFree[RTK_FC_MAC_PORT_MAX];
	atomic_t perPortCnt_ackDelayDQ[RTK_FC_MAC_PORT_MAX];

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	atomic_t mcPortCnt_WIFI_FF_TX;
#endif
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
	atomic_t totalCnt_tcp_in_window;
	atomic_t okCnt_tcp_in_window;
	atomic_t failCnt_tcp_in_window;
	atomic_t abnormalCnt_tcp_in_window;
#endif

}rtk_fc_statistic_t;

typedef enum rtk_fc_smp_jobs_type_e
{
	FC_SMP_WIFI_RX_DROP = 0,
	FC_SMP_FC_RX,
	FC_SMP_FROM_PS_RX,
	FC_SMP_TO_PS_TX,
	FC_SMP_JOBS_TYPE_MAX,
} rtk_fc_smp_jobs_type_t;

typedef enum rtk_fc_smp_statistics_dump_type_e
{
	FC_SMP_DUMP_WIFI_RX=0,
#if defined(CONFIG_RTK_NIC_HWLOOKUP_DEAMSDU_OFFLOAD)
	FC_SMP_DUMP_WIFI_AMSDU_RX,
#endif
	FC_SMP_DUMP_GMAC_RX_SKIP_FC,
	FC_SMP_DUMP_GMAC_RX,
	FC_SMP_DUMP_GMAC_IPI_TO_FC_RX,
	FC_SMP_DUMP_GMAC_FC_RX,
	FC_SMP_DUMP_FROM_PS_RX_SKIP_FC,
	FC_SMP_DUMP_FROM_PS_RX,

	FC_SMP_DUMP_IPI_TO_FC_RX_DROP,
	FC_SMP_DUMP_IPI_BAND0_HW_LOOKUP_DROP,
	FC_SMP_DUMP_IPI_BAND1_HW_LOOKUP_DROP,
	FC_SMP_DUMP_IPI_BAND2_HW_LOOKUP_DROP,
	FC_SMP_DUMP_WIFI_RX_DROP,

#ifdef CONFIG_RTK_L34_XPON_PLATFORM
	FC_SMP_DUMP_GMAC9_TX,
	FC_SMP_DUMP_GMAC10_TX,
	FC_SMP_DUMP_GMAC7_TX,
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	FC_SMP_DUMP_PORT_0x10_TX,
	FC_SMP_DUMP_PORT_0x11_TX,
	FC_SMP_DUMP_PORT_0x12_TX,
	FC_SMP_DUMP_PORT_0x13_TX,
	FC_SMP_DUMP_PORT_0x14_TX,
	FC_SMP_DUMP_PORT_0x15_TX,
	FC_SMP_DUMP_PORT_0x16_TX,
	FC_SMP_DUMP_PORT_0x17_TX,
#endif
	FC_SMP_DUMP_WIFI_BAND0_TX,
	FC_SMP_DUMP_WIFI_BAND1_TX,
	FC_SMP_DUMP_WIFI_BAND2_TX,
	FC_MGR_DUMP_WIFI_BANDx_TX_FF,
	FC_MGR_DUMP_WIFI_BANDx_TX_FF_AGG,
	FC_SMP_DUMP_TO_PS_TX,

	FC_SMP_DUMP_IPI_PS_NETIF_RX,
	FC_SMP_DUMP_IPI_PS_NETIF_RX_DROP,

	FC_SMP_DUMP_IPI_TO_NIC_TX_DROP,
	FC_SMP_DUMP_IPI_TO_WIFI_BAND0_TX_DROP,
	FC_SMP_DUMP_IPI_TO_WIFI_BAND1_TX_DROP,
	FC_SMP_DUMP_IPI_TO_WIFI_BAND2_TX_DROP,
	FC_SMP_DUMP_WIFI_BAND0_TX_DROP,
	FC_SMP_DUMP_WIFI_BAND1_TX_DROP,
	FC_SMP_DUMP_WIFI_BAND2_TX_DROP,
	FC_SMP_DUMP_WIFI_BAND0_TXQ_STOPPED_TIMES,
	FC_SMP_DUMP_WIFI_BAND1_TXQ_STOPPED_TIMES,
	FC_SMP_DUMP_WIFI_BAND2_TXQ_STOPPED_TIMES,
	FC_SMP_DUMP_RPS_CPU_DISTRIBUTE,
	FC_SMP_DUMP_STA_OFFLINE_AMSDU_DROP,

	FC_SMP_STATISTICS_DUMP_TYPE_MAX,
} rtk_fc_smp_statistics_dump_type_t;

typedef enum rtk_fc_smp_statistics_dump_mode_e
{
	FC_SMP_STATISTICS_DUMP_MODE_DISABLED = 0,
	FC_SMP_STATISTICS_DUMP_MODE_BRIEF,
	FC_SMP_STATISTICS_DUMP_MODE_DETAILED,
	FC_SMP_STATISTICS_DUMP_MODE_MAX,
} rtk_fc_smp_statistics_dump_mode_t;

typedef enum rtk_fc_proc_global_ctrl_flag_e
{
	FC_PROC_GLB_CTRL_LAN_QUEUE_ABILITY_ENHANCE = 0,
	FC_PROC_GLB_CTRL_ASIC_DRV_DEBUG,
	FC_PROC_GLB_CTRL_ACL_LAN_PORTMASK,
	FC_PROC_GLB_CTRL_HASH_CRC_DEBUG,
	FC_PROC_GLB_CTRL_TX_WITH_HDR_DEBUG,
	FC_PROC_GLB_CTRL_WIFI_RX_HASH,
	FC_PROC_GLB_CTRL_WIFI_RX_GMAC_AUTO_SEL,
	FC_PROC_GLB_CTRL_WIFI_TX_TRAP_HASH,
	FC_PROC_GLB_CTRL_WIFI_TX_GMAC_AUTO_SEL,
	FC_PROC_GLB_CTRL_PON_DS_P7_LOOPBACK,
	FC_PROC_GLB_CTRL_SHORTCUT_EARLYCHECK,
	FC_PROC_GLB_CTRL_ACL_PARA_DUMP,
	FC_PROC_GLB_CTRL_EVENT_REC_CTRL,
	FC_PROC_GLB_CTRL_MC2UC,
	FC_PROC_GLB_CTRL_MAX,
} rtk_fc_proc_global_ctrl_flag_t;

typedef enum rtk_fc_rtnl_job_array_type_e
{
	RTK_FC_RTNL_JOB_NEIGH 	= 0,
	RTK_FC_RTNL_JOB_ROUTE   = 1,
}rtk_fc_rtnl_job_array_type_t;
typedef enum rtk_fc_rtnl_job_array_family_e
{
	RTK_FC_RTNL_JOB_BRIDGE 	= 0,
	RTK_FC_RTNL_JOB_ARP   = 1,
}rtk_fc_rtnl_job_array_family_t;
	


typedef enum rtk_fc_rtnl_job_array_action_type_e
{
	RTK_FC_RTNL_JOB_ACT_DEFAULT 	= 0,
	RTK_FC_RTNL_JOB_ACT_CHECK   	= 1,
	RTK_FC_RTNL_JOB_ACT_DEL     	= 2,
	RTK_FC_RTNL_JOB_ACT_OTHER   	= 3,
	RTK_FC_RTNL_JOB_ACT_ADD     	= 4,
	RTK_FC_RTNL_JOB_ACT_CHANGE_MAC  = 5,
}rtk_fc_rtnl_job_array_action_type_t;

typedef enum rtk_fc_lut_learning_state_e
{
	LUT_DIR_SA_LEARNING = 0,
	LUT_DIR_DA_LEARNING   = 1,
	LUT_DIR_DONT_CARE     = 2,
}rtk_fc_lut_learning_state_t;

typedef struct rtk_fc_rtnl_job_array_s
{
	uint8 valid;
	uint8 addr[6];
	rtk_fc_rtnl_job_array_type_t type;
	rtk_fc_rtnl_job_array_action_type_t action;
	rtk_fc_rtnl_job_array_family_t family;
	struct net *sock_net;
	__s32 ifindex;
	uint32 ip_addr;

	__be32 gw_addr;
	uint8 rt_tbl_id;

}rtk_fc_rtnl_job_array_t;

typedef struct rtk_fc_rtnl_jobs_s
{
	rtk_fc_rtnl_job_array_t rtnlJobList[RTK_FC_RTNL_JOB_ARRAY_SIZE];
	rtk_fc_timer_list_t *rtnetlinkEventTimer;
}rtk_fc_rtnl_jobs_t;

typedef enum rtk_fc_drv_event_record_id_e
{
	RTK_FC_DRV_EVENT_REC_VLAN_INSERT_FAIL,
	RTK_FC_DRV_EVENT_REC_PPPOE_INSERT_FAIL,
	RTK_FC_DRV_EVENT_REC_PPPOE_REMOVE_FAIL,
	RTK_FC_DRV_EVENT_REC_TAIL_DROP,
	RTK_FC_DRV_EVENT_REC_PP_IPVER_UNMATCH,
	RTK_FC_DRV_EVENT_REC_FLOW_LRU,
	RTK_FC_DRV_EVENT_REC_AFT_TBL_FULL,
	RTK_FC_DRV_EVENT_REC_MC_TBL_FULL,
	RTK_FC_DRV_EVENT_REC_LUT_LRU,
	RTK_FC_DRV_EVENT_REC_FLOW_ADD_FAIL,
	RTK_FC_DRV_EVENT_REC_MAX,
}rtk_fc_drv_event_record_id_t;

typedef struct rtk_fc_drv_event_record_s
{
	rtk_fc_drv_event_record_id_t eventIdx;
	unsigned char eventName[64];
	unsigned int event_id;
}rtk_fc_drv_event_record_t;

#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
typedef struct rtk_fc_tableDmaAftTbl_s
{
	rtk_rg_asic_dmaAftFib_t fib;
	uint32 refCount;
	unsigned long last_jiffies_when_deleted;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	uint32 syncRsvEntry:1;
	uint32 syncRsvEntryIdx:4;
#endif
}rtk_fc_tableDmaAftTbl_t;
#if defined(CONFIG_FC_RTL9607F_SERIES)
typedef struct rtk_fc_tableDmaAftMapTbl_s
{
	uint32 refCount;
	unsigned long last_jiffies_when_deleted;
	uint32 lspid;
	uint8 aft_en;
	uint8 aft_fib_idx;
}rtk_fc_tableDmaAftMapTbl_t;
#endif
#endif

typedef struct rtk_fc_vxlan_cpuPort_info_s{
	uint32 cpuPort;
}rtk_fc_vxlan_cpuPort_info_t;

typedef struct rtk_fc_nptv6_fastFwd_record_s{
	uint32 inner_flow_index;
	uint32 outer_flow_index;
	uint32 sw_flow_index;
	uint32 isSet;
}rtk_fc_nptv6_fastFwd_record_t;


typedef struct rtk_fc_special_fastFwd_upstream_record_s{
	uint32 sw_flow_index;
	uint32 inner_flow_index;
	uint32 outer_flow_index;
	uint32 cpuPort;
	uint32 isSet;
	uint16 extraVoq_index;
}rtk_fc_special_fastFwd_upstream_record_t;

typedef struct rtk_fc_special_fastFwd_downstream_record_s{
	uint32 isSet;
	uint32 sw_flow_index;
	uint32 outer_flow_index;
	uint32 inner_flow_index;
	uint32 cpuPort;
}rtk_fc_special_fastFwd_downstream_record_t;

typedef struct rtk_fc_vxlan_l3cls_idx_s{
	uint32 pktlen_ds_68_aclInfo[4];
	uint32 pktlen_ds_others_aclInfo[4];
	uint32 pktlen_us_68_aclInfo[4];
	uint32 pktlen_us_128_aclInfo[4];
}rtk_fc_vxlan_l3cls_idx_t;

#if defined(CONFIG_FC_RTL9607C_SERIES)
typedef enum rtk_fc_enable_e
{
    RTK_FC_DISABLED = 0,
    RTK_FC_ENABLED,
    RTK_FC_ENABLE_END
} rtk_fc_enable_t;

typedef struct rtk_fc_nptv6_acceleration_s
{
	rtk_mac_t dmac;
	uint16 vaild:1;
	uint16 ipv6_hdr_offset:15;
	rtk_ipv6_addr_t ipv6_addr; //sip for upstream, dip for downstream
	int32 virtual_dmacL2Idx;
	fc_tx_info_t txInfo;
}rtk_fc_nptv6_acceleration_t;

typedef struct rtk_fc_nptv6_acceleration_nic_flow_s
{
	unsigned char preallocated_outer[MAX_NPTV6_OUTER_LEN];
	unsigned char preallocated_outer_length;
	rtk_mac_t smac;
	rtk_mac_t dmac;
	rtk_ipv6_addr_t sipv6;
	rtk_ipv6_addr_t dipv6;
	uint8 isTcp;
}rtk_fc_nptv6_acceleration_nic_flow_t;

typedef struct rtk_fc_nptv6_acceleration_data_s
{
	//nptv6 acceleration mechanism
	rtk_fc_nptv6_acc_enable_t nptv6_acceleration_mechanism;
	rtk_fc_nptv6_acceleration_t nptv6_acceleration_upstream[MAX_NPTV6_ACC_UPSTREAM_SIZE];
	rtk_fc_nptv6_acceleration_t nptv6_acceleration_downstream[MAX_NPTV6_ACC_DOWNSTREAM_SIZE];
	rtk_fc_nptv6_acceleration_t nptv6_acceleration_upstream0[MAX_NPTV6_ACC_UPSTREAM0_SIZE];
	rtk_fc_nptv6_acceleration_t nptv6_acceleration_downstream0[MAX_NPTV6_ACC_DOWNSTREAM0_SIZE];
	
	rtk_fc_nptv6_acceleration_nic_flow_t nptv6_acceleration_nic_flow_upstream[MAX_NPTV6_ACC_UPSTREAM_NIC_FLOW_NUM];
	rtk_fc_nptv6_acceleration_nic_flow_t nptv6_acceleration_nic_flow_downstream[MAX_NPTV6_ACC_DOWNSTREAM_NIC_FLOW_NUM];
	rtk_fc_nptv6_acceleration_nic_flow_t nptv6_acceleration_nic_flow_upstream0[MAX_NPTV6_ACC_UPSTREAM0_NIC_FLOW_NUM];
	rtk_fc_nptv6_acceleration_nic_flow_t nptv6_acceleration_nic_flow_downstream0[MAX_NPTV6_ACC_DOWNSTREAM0_NIC_FLOW_NUM];

	unsigned char preallocated_outer[MAX_NPTV6_OUTER_LEN];
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

	//int32 meterIdx_fromLan[4];
	//int32 meterACLIdx_fromLan[4];
}rtk_fc_nptv6_acceleration_data_t;

typedef struct rtk_fc_vxlan_acceleration_data_s
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

	fc_tx_info_t vxlan_upstream_txInfo;
	fc_tx_info_t vxlan_downstream_txInfo;
}rtk_fc_vxlan_acceleration_data_t;

typedef struct rtk_fc_special_fastFwd_data_s
{
	rtk_fc_nptv6_acceleration_data_t nptv6_acc;
	rtk_fc_vxlan_acceleration_data_t vxlan_acc;
}rtk_fc_special_fastFwd_data_t;
extern rtk_fc_special_fastFwd_data_t fc_db_fastFwd_data;

#endif

typedef struct rtk_fc_dynacmic_prehash_5tuple_s{
	uint32 ds_sip;
	uint32 ds_dip;
	uint32 ds_sport;
	uint32 ds_dport;
	uint32 ds_tos;
	uint32 ds_svid;
	uint32 ds_cvid;
	uint32 ds_cpri;
	uint32 ds_in_l4proto;
	uint32 us_sip;
	uint32 us_dip;
	uint32 us_sport;
	uint32 us_dport;
	uint32 us_tos;
	uint32 us_svid;
	uint32 us_cvid;
	uint32 us_cpri;
	uint32 us_in_l4proto;

}rtk_fc_dynacmic_prehash_5tuple_t;

typedef enum rtk_fc_mcapi_mode_e
{
	RTK_MC_MODE_INIT=0,
	RTK_MC_MODE_SET_BY_PORTMASK,
	RTK_MC_MODE_SET_BY_PERDEVPORT,
	RTK_MC_MODE_END,
} rtk_fc_mcapi_mode_t;

typedef struct rtk_fc_vlanGroupMacLimit_group_s
{
	rtk_fc_vlanGroupMacLimit_info_t group_info;
	struct list_head mac_head;
}rtk_fc_vlanGroupMacLimit_group_t;


typedef struct rtk_fc_vlanGroupMacLimit_mac_s
{
	rtk_mac_t mac;
	int vlanId;	//-1 means untag
	struct list_head mac_list;
	struct list_head group_list;

	rtk_fc_vlanGroupMacLimit_group_t *pGroup;
}rtk_fc_vlanGroupMacLimit_mac_t;
	
typedef struct rtk_fc_path5_downStream_flowList_s{
	struct list_head flow_list;
	uint32 flow_index;
	//uint32 hash_index;

}rtk_fc_path5_downStream_flowList_t;


#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
typedef struct rtk_fc_ipsec_uncache_addr_s{
	u8 *ipsec_hash_padding_ptr;
	u8 *ipsec_crypt_padding_ptr;
	u8 *ipsec_icv_padding_ptr;
	u8 *ipsec_icv_ptr;
	u8 *iv_ptr;
	u8 *aad_padding_ptr;
	u8 *aad_ptr;
	u8 *cmd_setting_ptr;
	u8 *plain_text_ptr;
	u8 *mix_mode_cmd_iv_ptr;
	u8 *key_ptr;
	
	dma_addr_t icv_dma;
	dma_addr_t ipsec_hash_padding_dma;
	dma_addr_t ipsec_crypt_padding_dma;
	dma_addr_t iv_dma;
	dma_addr_t aad_padding_dma;
	dma_addr_t ipsec_icv_padding_dma;
	dma_addr_t aad_dma;
	dma_addr_t cmd_setting_dma;
	dma_addr_t plain_text_dma;
	dma_addr_t mix_mode_cmd_iv_dma;
	dma_addr_t key_dma;
}rtk_fc_ipsec_uncache_addr_t;
	
typedef struct rtk_fc_ipsec_flowindex_list_s{
	struct list_head flow_list;
	uint32 flow_index;
	//uint32 hash_index;

}rtk_fc_ipsec_flowindex_list_t;

typedef struct rtk_fc_ipsec_xfrm_info_s{
//	struct xfrm_policy *x_policy;
	struct xfrm_state *x_state;
//	struct xfrm_id		id;
	uint32 saddr;
	uint32 daddr;
	struct in6_addr	saddr6;
	struct in6_addr	daddr6;
	int iv_len;
	u32 spi;
	u32 seq_no;
	u32 seq_no_hi;
	u32 block_size;
	unsigned int auth_len;
	u32 header_len;
	u8 replay_window;
	u8 is_ESN;
	u8 is_NATT;
	uint16		encap_sport;
	uint16		encap_dport;
	int valid;
	int direction;
	//int hash_alg;
	//int ciph_alg;
	u8 	key[64];
	int key_sz;
	u8 	key_hash[64];
	int key_hash_sz;
	u8 aalgo;
	u8 ealgo;
	u8 tunnel_mode;
	u8 ip_version;
	u8 	nonce[4]; // gcm usage
	u8 contentBuffer[48];//40(ip/ip6 max,) + 8 (esp)
	uint8 encrypt;
	uint8 dying;
	uint8 key_is_set;
	uint32 ipid;
	uint32 mark_v;
	uint32 mark_m;
	uint16 remote_lut_idx;
	int16 devGwMacIdx;
	u8 salt_in[32];// __attribute__ ((aligned(__alignof__(u32))));
	uint32 pe_crypto_sw_id; //RT_PE_IPSEC_SW_ID_DECRYPTION_BIT, RT_PE_IPSEC_SW_ID_CONNECTION_IDX_MASK
	uint8 key_index;
	uint8 hash_key_index;
}rtk_fc_ipsec_xfrm_info_t;
#endif

typedef enum rtk_fc_wifi_tx_amsdu_queueid_e
{
	RTK_FC_WIFI_TX_AMSDU_DISABLE = -1,
	RTK_FC_WIFI_TX_AMSDU_QUEUE0 = 0,
	RTK_FC_WIFI_TX_AMSDU_QUEUE1 = 1,
	RTK_FC_WIFI_TX_AMSDU_QUEUE2 = 2,
	RTK_FC_WIFI_TX_AMSDU_QUEUE3 = 3,

	RTK_FC_WIFI_TX_AMSDU_QUEUE_MAX_TC_EPP64_MODE = RTK_FC_WIFI_TX_AMSDU_QUEUE0,
	RTK_FC_WIFI_TX_AMSDU_QUEUE_MAX_WMM_EPP64_MODE = RTK_FC_WIFI_TX_AMSDU_QUEUE3,
}rtk_fc_wifi_tx_amsdu_queueid_t;

#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
typedef enum rtk_fc_wifi_amsdu_pe_offload_sta_info_update_mask_e
{
	WIFI_AMSDU_PE_OFF_STA_INFO_UPDATE_POWER_SAVING = (1<<0),
	WIFI_AMSDU_PE_OFF_STA_INFO_UPDATE_P_WLAN_DEV = (1<<1),
#if defined(CONFIG_REALTEK_IPC2RCPU)
	WIFI_AMSDU_PE_OFF_STA_INFO_UPDATE_AMSDU_PKT_SIZE = (1<<31),
#endif
}rtk_fc_wifi_amsdu_pe_offload_sta_info_update_mask_t;

typedef struct rtk_fc_wifi_amsdu_pe_offload_mac_id_tbl_s
{
	uint8 valid:1;
	unsigned char l2Addr[ETH_ALEN];
	uint8 ldpid:6;
	uint8 cos:3;
	rtk_fc_wifi_amsdu_pe_offload_sta_info_t sta_info;
}rtk_fc_wifi_amsdu_pe_offload_mac_id_tbl_t;
#endif

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
typedef struct rtk_fc_wifiFF_mc_to_uc_s
{
	unsigned char macRefCnt;
	unsigned char l2Addr[ETH_ALEN];
}rtk_fc_wifiFF_mc_to_uc_t;
#endif

#if defined(CONFIG_RTK_FC_CRYPTO_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
typedef struct rtk_pe_crypto_encrypt_info_s
{
	int32 fc_saInfo_idx; // -1 invalid
	rt_pe_crypto_encrypt_info_t pe_data;
}rtk_pe_crypto_encrypt_info_t;

typedef struct rtk_pe_crypto_decrypt_info_s
{
	int32 fc_saInfo_idx; // -1 invalid
	rt_pe_crypto_decrypt_info_t pe_data;
}rtk_pe_crypto_decrypt_info_t;

typedef struct rtk_pe_crypto_ps_desc_info_s
{
	volatile uint32 *pWait_desc_done;	// non-cached
	uint32 wait_desc_done_phy_addr;
	volatile uint32 *pWait_pop_done;	// non-cached
	uint32 wait_pop_done_phy_addr;
	//ps desc info
	uint32 crypto_array_idx;
	rtk_fc_crypto_desc crypto_desc_array[RTK_FC_CRYPTO_MAX_ARRAY_SIZE];
	rtk_fc_crypto_hw_crypto_ready_cnt_callback crypto_hw_crypto_ready_cnt_cb_func;
}rtk_pe_crypto_ps_desc_info_t;
#endif //CRYPTO and IPC

#if defined(CONFIG_RTK_FC_HTTP_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
typedef struct rtk_http_test_request_s
{
	rtk_mac_t server_mac;
	rtk_mac_t client_mac;
	uint8 isIPv4OrIpv6; // 0: ipv4, 1: ipv6
	union{
		rtk_ip_addr_t ipv4_addr;
		rtk_ipv6_addr_t ipv6_addr;
	}client_ip;
	union{
		rtk_ip_addr_t ipv4_addr;
		rtk_ipv6_addr_t ipv6_addr;
	}server_ip;
	uint16 client_l4port;
	uint16 server_l4port;

	uint32 ldpid;
	int32 dma_aft_idx; // -1 invalid, HW offload for PPPoE sid, svlan, cvlan 
	uint32 dma_aft_hdr_offset;
	rt_stream_id_t streamId;

	uint8 http_req_url[MAX_PE_HTTP_REQ_URL_STR_LENGTH]; 	//e.g., "/garbage.php?ckSize=5000"
	uint8 http_version[MAX_PE_HTTP_VERSION_STR_LENGTH]; 	//e.g., "HTTP/1.1"

	uint8 non_congestion_mode;		// for download

	uint64 upload_content_length; 	// unit: bytes
	
	uint32 tcp_window_size; 		// unit: bytes, 0: means it uses default window size
	uint32 tcp_mss_size;			// unit: bytes, 0: means it uses default value

	uint32 connection_number;		// MAX_PE_HTTP_DOWNLOAD_CONNECTION_NUM, MAX_PE_HTTP_UPLOAD_CONNECTION_NUM

	rtk_pe_http_test_mode_t test_mode;

	uint32 buf_phy_addr; 	/* physical address of allocated buffer */
	uint32 buf_size;	 	/* total allocated buffer size */
}rtk_http_test_request_t;

typedef rt_pe_http_test_result_t rtk_http_test_result_t;
#endif //HTTP and IPC

typedef struct rtk_fc_page_s {
	uint32 order; 			/* each page size is 4KB, allocate 2^order consecutive pages => if order is 10, it allocates 4MB(4KB * 2^10) . */
	struct page *pPages;
	uint32 buf_phy_addr; 	/* physical address of allocated buffer */
	uint32 buf_size;	 	/* total allocated buffer size */
}rtk_fc_page_t;

#define FLOWDATAPOOL_SIZE ((RTK_FC_TABLESIZE_HW_FLOW*sizeof(rtk_fc_tableFlowEntry_t)) + RTK_FC_FLOWENT_ALIGNBUF)
typedef enum rtk_fc_dual_ipv6HashMask_e
{
	//local ip hash mask
	//bit 0:mapt, bit 1:96 xlat, bit 2:64 xlat, bit 3:56 xlat, bit 4:48 xlat, bit 5:40 xlat, bit 6:32 xlat
	DUAL_IPV6_HASHMASK_ALL			= 1<<0,	//MAPT 128bits hash
	DUAL_IPV6_HASHMASK_PREFIX_96	= 1<<1,	
	DUAL_IPV6_HASHMASK_PREFIX_64	= 1<<2,
	DUAL_IPV6_HASHMASK_PREFIX_56	= 1<<3,
	DUAL_IPV6_HASHMASK_PREFIX_48	= 1<<4,
	DUAL_IPV6_HASHMASK_PREFIX_40	= 1<<5,
	DUAL_IPV6_HASHMASK_PREFIX_32	= 1<<6,
	DUAL_IPV6_HASHMASK_MAX,
} rtk_fc_dual_ipv6HashMask_t;

/* init by rtk_fc_param_init()*/
typedef struct rtk_fc_globalDatabase_s
{
	/* init & statistic */
	uint8 rtk_fc_init:1;
	uint8 rtk_fc_mgr_init:1;
	uint8 smpStatistic:2; // 0: turn off, 1: turn on, 2: turn on (show ipi queues state and counter)
	uint8 flowStatistic:1; // 0: turn off, 1: turn on
	uint8 fwdStatistic:1; // 0: turn off, 1: turn on
	uint8 igmp_unknown_flood:1;

	/*packet header*/
	rtk_fc_pktHdr_t smp_pktHdr[NR_CPUS];
	
	/*trace*/
	rtk_fc_debugLevel_t debugLevel;
	rtk_fc_debugLevel_t filterLevel;	//used to filter display message based on trace_filter
	uint8 tracefilterCpuOwner[NR_CPUS];
	
	/* func */
	rtk_fc_glb_controlfunc_t controlFuc;	// any control setting for driver behavior
	rtk_fc_glb_system_t systemGlobal;
	rtk_fc_hook_func_t hook_func;
	rtk_fc_flow_callback_func_t flow_callback_func;

	/*trace filter*/
	rtk_fc_debugLevel_t debugLevel_tmp; // for ingress flow learning to filter to PS packets
	rtk_fc_debug_trace_filter_bitmask_t trace_filter_bitmask[TRACFILTER_MAX];
	rtk_fc_debugTraceFilter_t trace_filter[TRACFILTER_MAX];
	uint32 traceFilterRuleMask; // enable trace filter rule mask
	uint32 tracefilterShow;		//show this packet
	int32 flow_del_trace_index[FLOWDELTRACE_MAX];	// support 4 flow index to be tracked once they are deleted
	
	/* link status */
	atomic_t portLinkupMask;

	/* flow */
	uint32 flowSyncPeriod_unit1s;		//time period to scan one flow entry, default 30s. call _rtk_fc_flow_chenkingTime_set to set new value
	uint32 flowIdleTimeout_unit1s;        //nonCtFlowTimeout, e.g. PATH 1/2 flow entry, default 30s
	/* Lut */
	int16 bcMacIdx;					//This is lutidx if flow compare need using HW_BC_IDX (4094)
	uint32 ucTimeout_unit1s;			//for non-fdb-unicast mac timeout default 300s
	uint32 mcTimeout_unit1s;			//for non-static-multicast mac timeout default 450s
	uint32 shortcut_flow_count;			// number of pure sw flows
	uint32 flowHwTableSize;				// 07c: SRAM 4k+64 or DRAM 8k/16k/32k; 77x: mainhash 64K + overflow 32 or 64
	uint32 flowSwTableSize;				// 32K+shortcut_flow_count; 64K+64+shortcut_flow_count for G3
	uint16 flowSync_groupCnt;			// how many groups(/buckets) housekeeping needs to check
	uint16 flowSync_GRANULARITY;		// buckets per timer tick
//	uint16 hwFlow_hkCurIdx;				// record house keeping current progress/index of hw flow
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	// Flow tables - house keeping
	int *fb_hwFlow_validBitsArray;						// array size: (4k+64/8k/16k/32k)/32
	int *fb_hwFlow_trfBitsArray;							// array size: (4k+64/8k/16k/32k)/32
	int *fb_hwFlow_tmpValidBitsArray;					// same as fb_hwFlow_validBitsArray
#endif
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	int *g3_mHashTbl_validBitsArray;						// array size: ((64K+64)/32) hw
	uint32 *g3_mHash_trfBitsArray;
	uint32 g3_mHash_trf_hkCurIdx;				// record house keeping current progress/index of mainHash traffic
#endif
	uint32 flowHashBuckets;						// flow entry buckets. 4k:1024, 8k/16k/32k:8k/16k/32k
	uint16 flowHashWayShift;
	uint16 flowLockGroupSize;						// default: 64
	uint16 swflowlistBucketExtendShift;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	uint16 flowEntIdxMask;						// crc16 & flowEntIdxMask will be base hash index
#endif
	// flow tables
#if (defined(CONFIG_RTK_L34_XPON_PLATFORM) && IS_BUILTIN(CONFIG_RTK_L34_FC_KERNEL_MODULE)) || defined(CONFIG_RTK_L34_G3_PLATFORM) 
	char flowEntryDataPool[FLOWDATAPOOL_SIZE];															//for dynamic allocation of pure sw flow entries
#endif
	rtk_fc_tableFlow_t flowTbl[RTK_FC_TABLESIZE_HW_FLOW + RTK_FC_MAX_SHORTCUT_FLOW_SIZE];
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	struct list_head *flowTcam_hashListHead;		// flow tcam entries list for each hash way
	struct list_head flowTcam_freeListHead;
	rtk_fc_flowTcam_linkList_t *flowTcamList;
#endif
	rtk_fc_abstrSwFlowType_entry_t abstrSwFlowType[ABSTR_SWFLOW_TYPE_SIZE];
	rtk_fc_ipv6_nat_mappingTbl_t ipv6_nat_mappingTbl[RTK_FC_TABLESIZE_I6NAT_MAPPING_TABLE];
	rtk_fc_ipv6_mapt_mappingTbl_t ipv6_mapt_mappingTbl[RTK_FC_TABLESIZE_I6MAPT_MAPPING_TABLE];
	//atomic_t swflow_only;
	rtk_fc_l2DualTbl_t l2DualTbl[RTK_FC_TABLESIZE_L2DUAL_TABLE];

#if defined(CONFIG_FC_RTL8277C_SERIES) && RTK_FC_TABLESIZE_OVERFLOW_FLOW
	// Flow tables - overflow
	struct list_head overFlowHash_inUseListHead;
	struct list_head overFlowHash_freeListHead;
	rtk_fc_overFlowHash_linkList_t *overFlowHashList;
#endif

	// Flow tables - sw
	SWFLOW_LIST_HEAD_t *swFlow_hashListHead;		// sw flow entries list for each hash way
	SWFLOW_LIST_HEAD_t swFlow_freeListHead;
	rtk_fc_swFlow_linkList_t *swFlowList;
#if defined(FC_F_SWFLOWLIST_IDX_MODE)
	rtk_fc_swFlow_linkList_t	swFlowList_end;				// the end node of free list and hash list
#endif
	uint32 swFlow_hkCurIdx;				// record house keeping current bucket progress/index of sw flow

	struct vlan_hdr cvh_from_skbmark;
	struct vlan_hdr svh_from_skbmark;

	rtk_fc_flow_info_natloopback_t flow_natloopbackTbl[RTK_FC_TABLESIZE_NATLOOPBACK_BUCKET][RTK_FC_TABLESIZE_NATLOOPBACK_WAY];	// if FLOW_INFO_NATLOOPBACK_ENTRY, indexed by flow entry indx

	// netif table - hw
	rtk_fc_devGwMac_t devGwMacTbl[RTK_FC_DEV_GW_MAC_TBL];
	rtk_fc_tableNetif_t netifTbl[RTK_FC_TABLESIZE_INTF_SW];		// include all real and virtual interfaces
	rtk_fc_hwTableNetifPointer_t netifHwTblVaild[RTK_FC_TABLESIZE_INTF];
	rtk_fc_dual_callBackRegFunc_t dualCallBackFunc[RTK_FC_DUALTYPE_END];
	uint8 dualIpv6HashMask;	//ref_to rtk_fc_dual_ipv6HashMask_t bit 0:mapt, bit 1:96 xlat, bit 2:64 xlat, bit 3:56 xlat, bit 4:48 xlat, bit 5:40 xlat, bit 6:32 xlat

	rtk_fc_netifDummyPacket_t     netifDummyPacket[RTK_FC_TABLESIZE_INTF]; //store dummy packet for each netif
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	uint32 netifTrfBit[(RTK_FC_TABLESIZE_INTF/32)+1]; // bit n represents the traffic bit of hw interface n
#endif

#if !(defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)) // 8277C/9607F no need indMac, pFlowPath5->out_dmac_idx is meaningless
	// indmac table - hw
	rtk_fc_tableIndMac_t indMacTbl[RTK_FC_TABLESIZE_INDMAC];
	struct list_head listHead_indMacHash[RTK_FLOWBASE_BUCKETSIZE_INDMAC]; //RTK_FLOWBASE_INDMAC_REF_HASH_LIST_HEAD[RTK_FLOWBASE_INDMAC_REF_BUCKET_SIZE];
#endif

	// extPort table - hw
	rtk_fc_tableExtPort_t extPortTbl[RTK_FC_TABLESIZE_EXTPORT];

	// ethtype table - hw
	rtk_fc_tableEthType_t ethertypeTbl[RTK_FC_TABLESIZE_ETHERTYPE];

	// mc table - hw
	rtk_fc_table_mcGroupTbl_t		mcGroupTbl[RTK_FC_TABLESIZE_MCFLOW];
	//rtk_fc_table_mcConfigTbl_t		mcCfgTbl[RTK_FC_TABLESIZE_MCFLOW];
	//rtk_fc_table_mcConfigTbl_t		mcCfgTbGrplDft[RTK_FC_TABLESIZE_MC_GROUP_DFTFWD];
	struct list_head listHead_mcCfgTbl;
	struct list_head listHead_mcCfgTbGrplDft;
	struct list_head listHead_mcExtraFlowIdxHashTbl[RTK_FC_TABLESIZE_MCFLOW_HASH];
	struct list_head listFree_mcExtraFlowIdxHashTbl;
	rtk_fc_mcExtraSwFlowIdx_entry_t mcExtraFlowIdxTbl[RTK_FC_TABLESIZE_MCFLOW];

	rt_igmp_multicast_learning_mode_t mcSetMode;		   //0:auto mode 1:set mc by API/Proc mode 
	rtk_fc_mcapi_mode_t mcapiMode;
	rtk_fc_igmp_timer_t igmpDummyPktDetectorTimer;
	rtk_fc_igmp_timer_t igmpKernelSyncTimerEvent;
	rtk_fc_igmp_timer_t igmpKernelSyncTimerPeriod;

	struct socket *rtnl_socket;
	//struct timer_list *rtnetlinkEventTimer;
	rtk_fc_rtnl_jobs_t rtnlJobs;
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)	
	rtk_fc_ipsec_xfrm_info_t fc_ipsec_info[RTK_FC_SPEC_XFRM_INFO_MAX_NUM];
	struct list_head ipsec_swFlow_list[RTK_FC_SPEC_XFRM_INFO_MAX_NUM];
	rtk_fc_ipsec_hash_padding_array_t rtk_fc_ipsec_hash_padding_ary[RTK_FC_SPEC_HSAH_PADDING_MAX_NUM];    		//aes
	rtk_fc_ipsec_hash_padding_array_t rtk_fc_ipsec_md5_hash_padding_ary[RTK_FC_SPEC_HSAH_PADDING_MAX_NUM];		//aes
	rtk_fc_ipsec_hash_padding_array_t rtk_fc_ipsec_des_hash_padding_ary[RTK_FC_SPEC_HSAH_PADDING_MAX_NUM];		//des
	rtk_fc_ipsec_hash_padding_array_t rtk_fc_ipsec_des_md5_hash_padding_ary[RTK_FC_SPEC_HSAH_PADDING_MAX_NUM];	//des
	rtk_fc_ipsec_uncache_addr_t ipsec_addr;
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
#else // support lspid_map table
	int16 lspid_map_idx_decrypt_hwlookup;			// for RTK_FC_IPSEC_HWLOOKUP_MACPORT
	int16 lspid_map_idx_pe_first_encrypt_hwlookup;	// for RT_PE_IPSEC_DMA_LSO_FIRST_ENCRYPT_HWLOOKUP_LSPID
#endif
#endif

	struct list_head path5_downStream_flowList[RTK_FC_PATH5_DS_FLOWLIST_BUCKET_MAX_NUM];

#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	//hardware pol id reampping to pol_2(pol grp id in 77B) or pol_3(77C)
	rtk_fc_hw_pol_id_remap_t hwPolRemap[RTK_FC_TABLESIZE_POLREMAP];
#endif

	//pure software rate limit and used by sw shaping rate control (shapingCtrl) (shapingCtrl)
	rtk_fc_sw_meter_t swMeter[RTK_FC_TABLESIZE_SW_SHAREMTR];
	rtk_fc_sw_rate_limit_t swRateLimit[RT_RATE_SW_RATE_LIMIT_TYPE_MAX];
	//l34 rate limit - sync from hw l34 flow meter and used by sw shaping rate control (shapingCtrl)
	rtk_fc_sw_meter_t l34Meter[RTK_FC_TABLESIZE_FBMTR];
	//software shpaing rate control
	rtk_fc_sw_shaping_ctrl_t shapingCtrl[RTK_FC_TABLESIZE_SW_SHAPING];

	rtk_fc_sw_ackDelay_ctrl_t ackDelayCtrl;
	rtk_fc_ackDelay_linkList_t *ackDelayList;
	struct list_head ackDelay_freeListHead;
	struct list_head *ackDelay_hashListHead;

	//SA learning limit number - per port and control action - per port
	rtk_fc_macAddr_learning_limit_t macAddrLearningLimit[RTK_FC_MAC_PORT_MAX];
	rtk_fc_macAddr_learning_limit_group_t macAddr_portGroup;
	rtk_fc_macAddr_learning_limit_t wlanMacAddrLearningLimit[RTK_FC_WLANX_END_INTF];

	//20190411LUKE: wan access limit configuration data
	rtk_fc_wan_access_limit_t wanAccessLimit;
	struct list_head wanAccessLimitIP_head[RTK_FC_WAN_ACCESS_IP_BUCKET_SIZE];
	uint32 wanAccessLimitProbeInterval_unit1s;

	// extratag table
	rtk_fc_tableExtraTag_t extraTagList[RTK_FC_TABLESIZE_EXTRATAG];
	u8 extraTagContentBuffer[RTK_FC_DUAL_CONTENT_BUF_SIZE];

	atomic_t smp_statistic[FC_SMP_JOBS_TYPE_MAX][NR_CPUS];

	rt_flow_op_flow_pattern_t highPriFlowPattern[RT_FLOW_HIGHPRIFLOW_PATTERN_SIZE];


	// Lut
	//****sync with atomic
	unsigned int lutEntry_pool_max;
	unsigned int lutEntry_pool_free;
	rtk_fc_lut_entry_t *lutEntry_pool_ring[RTK_FC_LUTENTRY_POOL_SIZE];
	struct list_head __rcu lut_hash_list_head[RTK_FC_LUT_BUCKET_SIZE];			// used by LRU searching.
	struct list_head __rcu lut_quickhash_list_head[RTK_FC_LUT_BUCKET_SIZE];		// used by regular searching

	rtk_fc_lut_entry_t *lutTbl[RTK_FC_TABLESIZE_LUT];	// pointer to hw lut entries, all lut entry are preallocate. ref. lutEntry_pool_ring
	rtk_fc_lut_ivl_entry_t lutIvlTbl[RTK_FC_TABLESIZE_LUT_IVL];
#if (defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))	
	rtk_fc_mcToUc_macid_entry_t mcToUcMacId[WLAN_MC_TO_UC_MAC_POOL_MAX];
#endif
	struct list_head lutCam_freeListHead;
	struct list_head lutCam_hashListHead[RTK_FC_LUT_BUCKET_SIZE];
	rtk_fc_lutCam_linkList_t lutCamList[LUTTABLE_BCAM_SIZE];

	uint16 l2_hkCurIdx;				// record house keeping current progress/index of LUT entry

	// skb mark / format translation
	rtk_fc_skbmark_t skbmark[RTK_FC_SKBMARK_END];

	// statistic
	//****sync with statistic lock
	rtk_fc_statistic_t statistic;
	uint32 memUsage;
	struct list_head memAlloc;
	spinlock_t lock_memDebug;

	uint16 *flowHashCount;				// array size is fc_db.flowHashBuckets
	uint16 *flowHashHwCollisionCount;	// array size is fc_db.flowHashBuckets

	/* tables and corresponding lists */
	rtk_fc_portmask_t wanPortMask;		// set by func: rtk_fc_wan_portmask_set
	rtk_fc_wlan_devmask_t wanPortWifidevMask;	// support wifi interface as wan. e.g. 4gLET
	rtk_fc_portmask_t lanPortMask,cpuPortMask,hypridPPTP;
	//****sync with atomic
	//atomic_t trapToPs;				//0:disable, 1: trap ingress and egress

#if defined(CONFIG_FC_RTL8198F_SERIES)
	atomic_t dsliteUdpFrag;		//0:disable, 1: Do dslite udp fragment when upstream size is over mtu after add IPv6 header
#endif


//+++++++++++++++ ACL Related Structure Start +++++++++++++++
	/*user defined ACL*/
	rtk_fc_aclFilterEntry_t acl_SW_table_entry[MAX_ACL_SW_ENTRY_SIZE]; //record all ACL informations(including RG_ACL & HW_ACL), the array index is the RG_ACL index

	rtk_fc_aclFilterEntry_t aclSWEntry,aclSWEntry_temp,empty_aclSWEntry;
	rt_acl_filterAndQos_t acl_filter_temp[MAX_ACL_SW_ENTRY_SIZE]; //for acl del entry using
	uint32 acl_filter_temp_valid[(MAX_ACL_SW_ENTRY_SIZE/32)+1]; //for acl del faster so record acl entry valid bit
	int acl_SW_table_entry_size; //used acl count
	int acl_SWindex_sorting_by_weight[MAX_ACL_SW_ENTRY_SIZE];	//The array record the RG_ACL rule index which sorted by weight. The ACL rule priority should handled by this sorting result.
	uint8 stop_add_acl;//This parameter only check during add new rule. 0:it can continue to add acl, 1:stop add this rule due to rearrange fail

	/*HW ACL*/
//ACL tempory parameters, due to struct size is too big that may cause function stack overflow
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	rtk_acl_field_t aclField[GLOBAL_ACL_FIELD_SIZE];
	rtk_acl_ingress_entry_t aclRule[GLOBAL_ACL_RULE_SIZE];
#endif
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	uint8 acl_lan_portmask;
#endif
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
	uint32 acl_remap_hash_idx[RTK_FC_ACL_HASH_TCP_TYPE_MAX][RTK_FC_MAC_PORT_CPU];	//per port enable syn related function by hash
#endif
#if defined(CONFIG_RTK_L34_G3_PLATFORM) && (defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))
	rtk_fc_aclMib_t acl_mib[RT_STAT_ACLMIB_TABLE_SIZE];
#endif

	/*reserved ACL*/
	rtk_fc_acl_reserved_global_t aclAndCfReservedRule;
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	char nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_TAIL_END][64];
#if defined(FC_USER_ACL_CA_CLS_SUPPORT)
	rtk_fc_g3_cls_entry_t ca_cls_rule_record[MAX_ACL_CA_CLS_RULE_SIZE];			//using ca cls api as index
#endif
	rtk_fc_ca_aal_cls_entry_t ca_aal_cls_rule_record[MAX_ACL_CA_AAL_CLS_RULE_SIZE];	//search empty rule
	uint8 ca_cls_used_count;
	uint32 ca_cls_template[RTK_ACL_TEMPLATE_CA_END];
	uint8 cls_l2_port_info[4][2];	//[0 is lan, 1 is wan, 2 is cpu, 3 is patch][0 is cls_start, 1 is cls_length]
#endif
//+++++++++++++++ ACL Related Structure End +++++++++++++++


#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	rtk_fc_flowOrHostmib_counter_t flowHWMib[RT_STAT_FLOWMIB_TABLE_SIZE];
	rtk_fc_flowOrHostmib_counter_t flowSWMib[RT_STAT_FLOWMIB_TABLE_SIZE][NR_CPUS]; //SW mib
#elif defined(CONFIG_RTK_L34_XPON_PLATFORM)
	rtk_fc_flowOrHostmib_counter_t flowSWMib[RT_STAT_FLOWMIB_TABLE_SIZE][NR_CPUS]; //SW mib
#endif
	rtk_fc_hostPoliceInfo_t hostPoliceTable[RT_RATE_HOSTPOLICING_TABLE_SIZE];

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	rtk_fc_dualhdr_greIpv4idInfo_t greIpidInfo[RTK_FC_TABLESIZE_INTF];

	rtk_fc_stormCtrl_t uucStorm[RTK_FC_MAC_PORT_MAX];
#if defined(CONFIG_RTK_L34_G3_PLATFORM) && !defined(CONFIG_FC_G3_G3LITE_SERIES)
	rtk_fc_tableStreamId_t streamidTbl[RTK_FC_TABLESIZE_STREAMID];
	rtk_fc_tableDmaAftTbl_t dmaAftActionTbl[RTK_FC_TABLESIZE_DMAAFTACTION];
#if defined(CONFIG_FC_RTL9607F_SERIES)
	rtk_fc_tableDmaAftMapTbl_t dmaAftMapTbl[RTK_FC_TABLESIZE_DMAAFTMAP];
#endif
#endif
#endif
	rt_stat_gemFilter_conf_t gemFilter_conf[RT_STAT_GEMMIB_TABLE_SIZE];
	rt_stat_port_cntr_t gemFilter_mib[RT_STAT_GEMMIB_TABLE_SIZE][NR_CPUS];

	unsigned int all_mac_portmask_without_cpu;

	rtk_fc_dual_passthrough_flowMapping_t dual_pt_flowMapTbl[RTK_FC_DUAL_PASSTHROUGH_FLOWMAPPING_SIZE];	
	struct list_head *ipFrag_hashListHead;
	struct list_head ipFrag_freeListHead;
	rtk_fc_ipFrag_linkList_t *ipFragList;
	
	struct list_head *negativeIpFrag_hashListHead;
	struct list_head negativeIpFrag_freeListHead;
	rtk_fc_negativeIpFrag_linkList_t *negativeIpFragList;
#if defined(CONFIG_RTK_SOC_RTL8198D)
	rtk_fc_flow_limit_config_t	flow_limit;
#endif
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
	rtk_fc_default_route_info_t defaultRouteInfo[MAX_DEFAULT_ROUTE_INFO_ENTRY_SIZE];
	uint8 tcp_in_window_shortcut_check;
	uint8 tcp_in_window_shortcut_fail_action;
#endif	
	uint8 out_stream_idx;
	uint8 out_user_pri_act;
	uint8 out_user_priority;
//-------------VXLAN SPECIAL FAST FORWARD---------------------
	uint16 vxlan_ds_fastForward_num;
	uint16 vxlan_us_fastForward_num;
	int vxlan_us_fastForward_wan_intf_id;
#if defined(CONFIG_FC_CA8277B_SERIES)
	uint8 txBuffInitDone;
	uint8 vxlan_hw_issue_work_around_setting;
	rtk_fc_vxlan_l3cls_idx_t vxlan_l3cls_info;
	uint8 vxlan_specialFastFwd_setup_l3cls;
	uint8 vxlan_upstream_ca_extra_set;
	uint8 vxlan_downstream_ca_extra_set;
	uint8 vxlan_extraVOQ_usage[4][2];  // [0][0]: cpu port 4, voq 5, [0][1]: cpu port 4, voq 6, [1][0]: cpu port 5, voq 5, [1][10]: cpu port 5, voq 6 ... and so on
	uint32 vxlan_fastFwd_RoundRobin_cpuPort_array[4];
	uint16 vxlan_ds_extra_fastForward_num;
	uint16 vxlan_us_extra_fastForward_num;
	uint8 vxlan_ds_extra_RoundRobin_cpuPort_index;
	uint8 vxlan_us_extra_RoundRobin_cpuPort_index;
	uint8 vxlan_ds_extraCLS_isSet;
	uint32 vxlan_ds_extraCLS_Info;
	rtk_fc_special_fastFwd_upstream_record_t vxlan_extra_upStream_record[RTK_FC_VXLAN_EXTRA_MAX_NUM];   // start from index 1, 0: reserved
	rtk_fc_special_fastFwd_downstream_record_t vxlan_extra_downStream_record[RTK_FC_VXLAN_EXTRA_MAX_NUM]; // start from index 1, 0: reserved
#endif

	rtk_fc_special_fastFwd_downstream_record_t vxlan_downStream_record[4];
	rtk_fc_special_fastFwd_upstream_record_t vxlan_upStream_record[4];
	rtk_fc_l2DualTbl_t vxlan_us_SpecialFastFwdTbl[4]; // Per cpu port use 1 entry, will copy vxlan table info to corresponding entry
	uint8 vxlan_l3cls_upstream_isSet[4];
	uint8 vxlan_l3cls_downstream_isSet[4];
#if	defined(CONFIG_FC_RTL9607C_SERIES)
	short vxlan_accelerate_upstreamL2Idx;
	short vxlan_accelerate_extra_upstreamL2Idx;
	short vxlan_accelerate_downstreamL2Idx;
	short vxlan_accelerate_extra_downstreamL2Idx;
	uint8 vxlan_upstream_set_ok;
	uint8 vxlan_downstream_set_ok;
	int vxlan_rt_meter_index;
	struct rtl8686_hwnat_customized_entry vxlan_nicInfo_customized_entry;
	struct rtl8686_hwnat_customized_entry vxlan_port0_nicInfo_customized_entry;
	struct rtl8686_hwnat_customized_entry vxlan_down_nicInfo_customized_entry;
	struct rtl8686_hwnat_customized_entry vxlan_downextra_port0_nicInfo_customized_entry;
	uint8 vxlan_nicInfo_customized_entry_isSet;
	uint8 vxlan_port0_nicInfo_customized_entry_isSet;
	uint8 vxlan_downstream_port0_nicInfo_customized_entry_isSet;
	uint8 vxlan_downstream_extra_nicInfo_customized_entry_isSet;
#endif

//------------------------------------------------------------


//-------------NPTV6 SPECIAL FAST FORWARD---------------------

#if	defined(CONFIG_FC_RTL9607C_SERIES)
	int nptv6_acc_upstream_assign_share_meter[4];
	struct tx_info txDesc;
	rtk_fc_nptv6_fastFwd_record_t nptv6_flow_info[MAX_NPTV6_ACC_UPSTREAM_SIZE+MAX_NPTV6_ACC_DOWNSTREAM_SIZE];
	uint32 nptv6_setMeterRate;
	uint32 nptv6_setMeter_macPortIdx;
	int32 meterIdx_fromLan[4];
	int32 meterACLIdx_fromLan[4];
	int8 meterACLIdx_fromLan_isSet[4];

#endif

	rtk_fc_mape_dst6_t mapeDst6s[RTK_FC_TABLESIZE_INTF];
//------------------------------------------------------------
	uint8 l2_port_moving_check_with_vlan;
	uint8 disable_port_moving_action_forward; // 0: drop; 1: forward
	uint32 disable_port_moving_port_bitmask; // 1 port need 1 bit: 0: allow port moving, 1: disable port moving, should drop packet if port moving.
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	uint8 dynamic_calculate_prehash_count;
	uint8 dynamic_calculate_prehash_timeout_sec;
	unsigned long dynamic_calculate_prehash_jiffies;
	uint8 dynamic_calculate_prehash_isSet;
	uint32 dynamic_calculate_prehash_port_diff;
	rtk_fc_dynacmic_prehash_5tuple_t dynamic_prehash_5tuple_info;
	rtk_fc_timer_list_t *dynamic_prehashPtn_eventTimer;
	//spinlock_t dynamic_lock;
#endif
	uint8 vxlan_fastFwd_data_general_setting_isSet;
	uint16 external_switch_vlan_start;
	uint16 external_switch_lan_port_num;
	

	rtk_fc_vlanGroupMacLimit_group_t vlanGroupMACLimit_group[MAX_VLAN_GROUP_MAC_LIMIT_NUMBER];
	struct list_head vlanGroupMACLimit_macHead[RTK_FC_TABLESIZE_LUT>>RTK_FC_LUT_HASH_WAY_SHIFT];		//indexed by HASH(IVL or SVL)
	uint8 vlanGroupMACLimit_group_is_set;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)

	struct list_head ct_hash_info_ListHead[RTK_FC_CT_HASH_INFO_BUCKET_SIZE];
#endif

#if defined(CONFIG_RTK_FC_HTTP_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
	//http
	uint32 http_test_dma_aft_idx;
	uint32 http_test_cls_idx[MAX_PE_HTTP_CONNECTION_NUM];
	uint32 http_test_dma_lso_regValue[10];
	rtk_fc_page_t pe_http_pageInfo;
#endif	
#if defined(CONFIG_RTK_FC_CRYPTO_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
	//crypto
	rtk_pe_crypto_encrypt_info_t pe_encrypt_info[MAX_PE_IPSEC_ENCRYPTION_CONNECTION_NUM];
	rtk_pe_crypto_decrypt_info_t pe_decrypt_info[MAX_PE_IPSEC_DECRYPTION_CONNECTION_NUM];
	rtk_fc_page_t pe_crypto_pageInfo;
	rtk_pe_crypto_ps_desc_info_t pe_crypto_ps_desc_info;
#endif
	uint32 pe_offload_wifi_amsdu_en;
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	uint8 wifi_amsdu_pe_offload_mode:3; //rtk_fc_wifi_amsdu_pe_offload_mode_t
	uint8 wifi_amsdu_pe_offload_sta_count;	//10 or 40
	rtk_fc_wifi_amsdu_pe_offload_mac_id_tbl_t wifi_amsdu_pe_offload_mac_id_tbl[RTK_FC_WIFI_PE_OFFLOAD_MAC_ID_SIZE];
#endif
	// end
	unsigned char cmd_buf[CMD_BUFF_SIZE];		// string to integer, user pipe cmd
	unsigned char cmd_proc_path[CMD_BUFF_PROC_LEN];
}rtk_fc_globalDatabase_t;
extern rtk_fc_globalDatabase_t fc_db;
extern void *rtk_fc_cryptoEngine_baseaddr;

typedef struct rtk_fc_pathDecision_global_var_s{
	rtk_fc_g3IgrExtraInfo_t g3IgrExtraInfo; // 208 bytes
	rtk_fc_igrExtraInfo_t extraInfo;
	rtk_fc_flowPath_param_t flowPath_param;
	struct rt_nfconn rtct;
	rtk_rg_asic_path1_entry_t flowEntry;
	rtk_rg_asic_path1_entry_t flowEntryMultiAction;
	rtk_rg_asic_path5_entry_t tmp_pFlowPath5;
	rtk_fc_g3IgrExtraInfo_t tmp_pG3IgrExtraInfo;	
}rtk_fc_pathDecision_global_var_t;

#if 0 //LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 30)
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

#ifndef HWNAT_CUSTOMIZE
//For compiler happy
enum RTL8686_customize_type{
	CUSTOMIZE_TYPE_NONE=0,
	CUSTOMIZE_TYPE_NPTV6_UP=1,
	CUSTOMIZE_TYPE_NPTV6_DOWN=2,
	CUSTOMIZE_TYPE_VXLAN_UP=3,
	CUSTOMIZE_TYPE_VXLAN_DOWN=4,
	CUSTOMIZE_TYPE_MAX=5,
};
#endif



/* =========== PROC file system  =========== */

typedef enum rtk_rg_procDir_e
{
	PROC_DIR_HW_DUMP,
	PROC_DIR_SW_DUMP,
	PROC_DIR_TRACE,
	PROC_DIR_CTRL,
	PROC_DIR_MAX //this field must put at last.
} rtk_rg_procDir_t;


typedef struct rtk_fc_proc_s
{
	char *name;
	int (*get) (struct seq_file *s, void *v);
	int (*set) ( struct file *, const char *,unsigned long, void *);
	unsigned int inode_id;
	unsigned char unlockBefortWrite;
	unsigned char unlockBeforeRead;
	unsigned char overPageSizeDisplay;
	void *pProc_fops;
	rtk_rg_procDir_t dir;
}rtk_fc_proc_t;

typedef struct mem_alloc_debug_s
{
	struct list_head mem_alloc_list;
	void* malloc_addr;
	void* alloc_func_addr;
	void* alloc_ret_addr;
	unsigned int alloc_size;
}rtk_fc_mem_alloc_debug;

typedef struct mem_alloc_debug_dump_s
{
	struct list_head mem_alloc_list;
	void* alloc_func_addr;
	void* alloc_ret_addr;
	unsigned int alloc_size;
	unsigned int alloc_num;
}rtk_fc_mem_alloc_debug_dump;

#if defined(CONFIG_FC_RTL8277C_SERIES)
#include "dal/rtl8277c/dal_rt_rtl8277c_cls.h"
#ifdef RTL8277C_CLS_EXT_WAN_PORT
#define	RTK_FC_MAC_PORT_PON_LSPID_TO_L2FE	RTL8277C_CLS_EXT_WAN_PORT		// from RT CLS DAL
#else
#define	RTK_FC_MAC_PORT_PON_LSPID_TO_L2FE	RTK_FC_MAC_PORT5
#endif
#endif


#endif // __RTK_FC_STRUCT__

