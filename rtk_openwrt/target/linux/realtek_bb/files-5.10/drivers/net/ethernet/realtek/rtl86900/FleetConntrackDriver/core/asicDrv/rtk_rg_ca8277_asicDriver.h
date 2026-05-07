/*
 * Copyright (C) 2019 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/

#ifndef __RTK_RG_CA8277_ASICDRIVER__
#define __RTK_RG_CA8277_ASICDRIVER__


#if defined(CONFIG_CA8279_SERIES) || defined(CONFIG_ARCH_RTL8198F)
#ifndef CONFIG_CA_G3_G3LITE_SERIES
#define CONFIG_CA_G3_G3LITE_SERIES 1
#endif
#endif


/*
 * Include Files
 */
 /* Linux */
#include <linux/types.h>

/* RTK */
#include <common/type.h>
#include <common/rt_type.h>

/* CA */
#include <flow.h>
#include <mcast.h>
#include <vlan.h>
#include <l2.h>
#include <nat.h>


/*
 * Symbol Definition
 */
 
#define CA_DEF_DEVID		0

#define FLOW_V6HASHADDR_MC_BIT 0x80000000
#define FLOW_V4ADDR_MC_LO_BOUND 0xe0000000
#define FLOW_V4ADDR_MC_UP_BOUND 0xf0000000

/*
 * Table size
 */

#define ASIC_TABLESIZE_LUT_BITS	13
#define ASIC_TABLESIZE_INTF_BITS	5
#define ASIC_TABLESIZE_DUALHEADER_INTF_BITS ASIC_TABLESIZE_INTF_BITS
#define ASIC_DUAL_CONTENT_BUFFER_SIZE_BITS 9

#define LUTTABLE_SRAM_SIZE 4096
#define LUTTABLE_BCAM_SIZE 32

/*
 * Data Declaration
 */


/* flow-based global state */
typedef enum rtk_rg_asic_globalStateType_s
{
    	FB_GLOBAL_STATE_START = 0,
	FB_GLOBAL_TTL_1 = FB_GLOBAL_STATE_START,
	FB_GLOBAL_TRAP_TCP_SYN_FIN_REST,
	FB_GLOBAL_TRAP_TCP_SYN_ACK,
	FB_GLOBAL_TRAP_FRAGMENT,
	FB_GLOBAL_L3_CS_CHK,
	FB_GLOBAL_L4_CS_CHK,
	FB_GLOBAL_PATH12_SKIP_CVID,
	FB_GLOBAL_PATH12_SKIP_CPRI,
	FB_GLOBAL_PATH12_SKIP_SVID,
	FB_GLOBAL_PATH34_UCBC_SKIP_CVID,
	FB_GLOBAL_PATH34_UCBC_SKIP_CPRI,
	FB_GLOBAL_PATH34_UCBC_SKIP_SVID,
	FB_GLOBAL_PATH34_MC_SKIP_CVID,
	FB_GLOBAL_PATH34_MC_SKIP_CPRI,
	FB_GLOBAL_PATH34_MC_SKIP_SVID,
	FB_GLOBAL_PATH34_SKIP_DA,
	FB_GLOBAL_PATH5_SKIP_CVID,
	FB_GLOBAL_PATH5_SKIP_CPRI,
	FB_GLOBAL_PATH5_SKIP_SVID,
	FB_GLOBAL_PATH6_SKIP_CVID,
	FB_GLOBAL_PATH6_SKIP_CPRI,
	FB_GLOBAL_PATH6_SKIP_SVID,
	FB_GLOBAL_PATH6_SKIP_DA,
	FB_GLOBAL_PATH6_SKIP_SA,
	FB_GLOBAL_PATHALL_SKIP_DSCP,
	FB_GLOBAL_L2_FLOW_LOOKUP_BY_MAC,
	FB_GLOBAL_CMP_TOS,
	FB_GLOBAL_PATHALL_SKIP_SPRI,		//if disabled, apolloPro: support SW flow to identify svlan priority; 8277 series: support HW and SW flow to identify svlan priority
	FB_GLOBAL_PATHALL_SKIP_VLAN_DEICFI,	//if disabled, apolloPro: support SW flow to identify vlan DEI/CFI; 8277 series: support HW and SW flow to identify vlan DEI/CFI
	FB_GLOBAL_STATE_END
}rtk_rg_asic_globalStateType_t;

typedef enum rtk_rg_asic_fbMode_s 
{
	FB_MODE_1K = 0,				// SRAM mode
	FB_MODE_4K = FB_MODE_1K,	// SRAM mode
	FB_MODE_8K = 1,
	FB_MODE_16K = 2,
	FB_MODE_32K = 3,
}rtk_rg_asic_fbMode_t;

typedef enum rtk_rg_asic_preHashPtn_s
{
	FB_PREHASH_PTN_SPORT = 0,
	FB_PREHASH_PTN_DPORT,
	FB_PREHASH_PTN_SIP,
	FB_PREHASH_PTN_DIP,
	FB_PREHASH_PTN_END,
}rtk_rg_asic_preHashPtn_t;

/* flow-based dual header used registers */
typedef enum rtk_rg_asic_dualHdrInfo_s
{
    	FB_DUALHDR_GRESEQ = 0,
    	FB_DUALHDR_GREACK,
	FB_DUALHDR_OUTER_IPV4ID,
	FB_DUALHDR_END,
}rtk_rg_asic_dualHdrInfo_t;

typedef enum rtk_rg_asic_pathValue_s 
{
	FB_PATH_12	 = 0,
	FB_PATH_34	 = 1,
	FB_PATH_5	 = 2,
	FB_PATH_6	 = 3,
}rtk_rg_asic_pathValue_t;

/* flow-based forwarding action */
typedef enum rtk_rg_asic_forwardAction_s
{
   	FB_ACTION_FORWARD = 0,
	FB_ACTION_TRAP2CPU,
	FB_ACTION_DROP,
	FB_ACTION_END
}rtk_rg_asic_forwardAction_t;

/* flow-based pppoe action */
typedef enum rtk_rg_asic_netifpppoeAct_e
{
	FB_NETIFPPPOE_ACT_KEEP  = 0,		//tagif=1: keep; tagif=0: keep
	FB_NETIFPPPOE_ACT_ADD,			// tagif=1: keep; tagif=0: add
	FB_NETIFPPPOE_ACT_MODIFY,		// tagif=1: modify; tagif=0: keep
	FB_NETIFPPPOE_ACT_REMOVE,		// tagif=1: remove; tagif=0: keep
	FB_NETIFPPPOE_ACT_END
} rtk_rg_asic_netifpppoeAct_t;

typedef enum rtk_rg_asic_extraTagAct_type_e
{
	FB_EXTG_ACTBIT_NOACTION=0,	//no-more action
	FB_EXTG_ACTBIT_1,			//extra tag srcaddr_offset and len
	FB_EXTG_ACTBIT_2,			//ethertype
	FB_EXTG_ACTBIT_3,			//HSB.L2LEN + VALUE
	FB_EXTG_ACTBIT_4,			//Gre
	FB_EXTG_ACTBIT_5,			//offload l3 chksum
	FB_EXTG_ACTBIT_6,			//offload l4 chksum
	FB_EXTG_ACTBIT_7,			//sw only
}rtk_rg_asic_extraTagAct_type_t;

typedef enum rtk_rg_asic_inprotocol_type_e
{
	FB_INPROTOCOL_ETHTBL_0 = 0,
	FB_INPROTOCOL_ETHTBL_1,
	FB_INPROTOCOL_ETHTBL_2,
	FB_INPROTOCOL_ETHTBL_3,
	FB_INPROTOCOL_ETHTBL_4,
	FB_INPROTOCOL_ETHTBL_5,
	FB_INPROTOCOL_ETHTBL_6,
	FB_INPROTOCOL_RESERVED,
	FB_INPROTOCOL_INNER_IPV4 = 8,
	FB_INPROTOCOL_INNER_IPV6 = 9,
	FB_INPROTOCOL_OUTER_0800 = 10,
	FB_INPROTOCOL_OUTER_8863 = 11,
	FB_INPROTOCOL_OUTER_8864 = 12,
	FB_INPROTOCOL_OUTER_86DD = 13,
	FB_INPROTOCOL_OUTER_PPPOE = 14,
	FB_INPROTOCOL_ALL_ACCEPT = 15,
}rtk_rg_asic_inprotocol_type_t;

typedef enum
{
	// reserved error code for ca return value
	/*
	CA_E_ERROR          = -1,
	CA_E_OK             = 0x0,
	CA_E_RESOURCE       = 0x1,
	CA_E_PARAM          = 0x2,
	CA_E_NOT_FOUND      = 0x3,
	CA_E_CONFLICT       = 0x4,
	CA_E_TIMEOUT        = 0x5,
	CA_E_INTERNAL       = 0x6,
	CA_E_NOT_SUPPORT    = 0x7,
	CA_E_CONFIG         = 0x8,
	CA_E_UNAVAIL        = 0x9,
	CA_E_MEMORY         = 0xa,
	CA_E_BUSY           = 0xb,
	CA_E_FULL           = 0xc,
	CA_E_EMPTY          = 0xd,
	CA_E_EXISTS         = 0xe,
	CA_E_DEV            = 0xf,
	CA_E_PORT           = 0x10,
	CA_E_LLID           = 0x11,
	CA_E_VLAN           = 0x12,
	CA_E_INIT           = 0x13,
	CA_E_INTF           = 0x14,
	CA_E_NEXTHOP        = 0x15,
	CA_E_ROUTE          = 0x16,
	CA_E_DB_CHANGED     = 0x17,
	CA_E_INACTIVE       = 0x18,
	CA_E_ALREADY_SET    = 0x19,
	*/

	ASIC_RET_FAIL				=-1,
	ASIC_RET_SUCCESS			=0,
	
	ASIC_RET_NULL_POINTER		=0xE001,
	ASIC_RET_OUT_OF_RANGE	=0xE002,
	ASIC_RET_FULL				=0xE003,
	ASIC_RET_NOT_FOUND		=0xE004,
	ASIC_RET_EXIST				=0xE005,

	ASIC_RET_NOT_SUPPORT		=0xE006,
} asic_ret_t;

/**************************************************************************************************
	* Physical Port ID(PPID) definition (ref: aal_port.h) *
***************************************************************************************************/
typedef enum
{
	ASIC_PPORT_L3_WAN		= 0x0a,
} asic_pport_t;

/**************************************************************************************************
	* Logical Port ID(LPID) definition (ref: aal_port.h) *
***************************************************************************************************/
typedef enum
{
	ASIC_LPORT_ETH_NI0		= 0x00,
	ASIC_LPORT_ETH_NI1		= 0x01,
	ASIC_LPORT_ETH_NI2		= 0x02,
	ASIC_LPORT_ETH_NI3		= 0x03,
	ASIC_LPORT_ETH_NI4		= 0x04,
	ASIC_LPORT_ETH_NI5		= 0x05,
	ASIC_LPORT_ETH_NI6		= 0x06,
	ASIC_LPORT_ETH_NI7		= 0x07,
	ASIC_LPORT_CPU_0		= 0x10,
	ASIC_LPORT_CPU_2		= 0x12,
	ASIC_LPORT_CPU_7		= 0x17,
	ASIC_LPORT_L3_WAN		= 0x18,	/* LSPID of LAN_L3 and LDPID of L3_LAN in L3 Engine */
	ASIC_LPORT_L3_LAN		= 0x19,		/* LSPID of WAN_L3 and LDPID of L3_WAN in L3 Engine */
	ASIC_LPORT_MC			= 0x1b,
	ASIC_LPORT_PON_US_0		= 0x20,
	ASIC_LPORT_PON_US_4		= 0x24,
	ASIC_LPORT_PON_US_8		= 0x28,
	ASIC_LPORT_PON_US_15	= 0x2f,
	ASIC_LPORT_PON_US_16	= 0x30,
	ASIC_LPORT_PON_US_31	= 0x3f,
} asic_lport_t;


/* flow-based table - for path 1 */
typedef struct rtk_8277_asic_path1_entry_s
{
//__LITTLE_ENDIAN
	/* word 0 */
	uint32 valid:1;
	uint32 out_dmac_trans:1;
	uint32 in_path:2;
	uint32 in_spa_check:1;
	uint32 in_ctagif:1;
	uint32 in_stagif:1;
	uint32 in_pppoeif:1;
	uint32 in_out_stream_idx_check_act:1;
	uint32 in_multiple_act:1;
	uint32 in_intf_idx		: ASIC_TABLESIZE_INTF_BITS;
	uint32 reserved0:4;
	uint32 out_intf_idx	: ASIC_TABLESIZE_INTF_BITS;
	uint32 in_tos:8;
	/* word 1 */
	uint32 in_protocol:4;
	uint32 in_smac_lut_idx: ASIC_TABLESIZE_LUT_BITS;		// G3 l2 table size is 4K+32
	uint32 reserved1:2;
	uint32 in_dmac_lut_idx: ASIC_TABLESIZE_LUT_BITS;
	/* word 2 */
	uint32 reserved2:4;
	uint32 in_svlan_id:12;
	uint32 reserved3:4;
	uint32 in_cvlan_id:12;
	/* word 3 */
	uint32 in_spa:8;
	uint32 in_ext_spa:8;
	uint32 in_pppoe_sid:16;
	/* word 4 */
	uint32 out_dmac_idx:8;
	uint32 out_svid_format_act:1;
	uint32 out_user_priority:3;
	uint32 out_cvid_format_act:1;
	uint32 out_extra_tag_index:3;
	uint32 out_ext_portmask_idx:5;
	uint32 out_portmask:11;
	/* word 5 */
	uint32 out_spri_format_act:1;
	uint32 out_spri:3;
	uint32 out_svlan_id:12;
	uint32 out_cpri_format_act:1;
	uint32 out_cpri:3;
	uint32 out_cvlan_id:12;
	/* word 6 */
	uint32 in_pppoe_sid_check:1;
	uint32 in_out_stream_idx:7;
	uint32 out_multiple_act:1;
	uint32 out_dscp_act:1;
	uint32 out_dscp:6;
	uint32 out_drop:1;
	uint32 out_egress_cvid_act:1;
	uint32 out_egress_svid_act:1;
	uint32 out_ctag_format_act:1;
	uint32 out_stag_format_act:1;
	uint32 out_user_pri_act:1;
	uint32 out_egress_port_to_vid_act:2;  //b'00:none  b'01:SP2C  b'10:SP2S   b'11:CP2C  refto rtk_rg_out_egress_port_to_vid_act_t
	uint32 out_smac_trans:1;
	uint32 out_uc_lut_lookup:1;
	uint32 reserved6:1;
	uint32 in_tos_check:1;
	uint32 lock:1;
	uint32 in_cvlan_pri:3;
	/* word 7 */
	uint32 out_flow_counter_act:1;
	uint32 out_flow_counter_idx:5;
	uint32 out_share_meter_act:1;
	uint32 out_share_meter_idx:5;
	uint32 reserved7:20;
}rtk_8277_asic_path1_entry_t;

/* flow-based table - for path 2  */
typedef struct rtk_8277_asic_path2_entry_s
{
//__LITTLE_ENDIAN
	/* word 0 */
	uint32 valid:1;
	uint32 reserved1:1;
	uint32 in_path:2;
	uint32 in_spa_check:1;
	uint32 in_ctagif:1;
	uint32 in_stagif:1;
	uint32 in_pppoeif:1;
	uint32 in_stream_idx_check:1;
	uint32 in_multiple_act:1;
	uint32 reserved0:9;
	uint32 out_intf_idx	: ASIC_TABLESIZE_INTF_BITS;
	uint32 in_tos:8;
	/* word 1 */
	uint32 in_protocol:4;
	uint32 in_smac_lut_idx: ASIC_TABLESIZE_LUT_BITS;		// G3 l2 table size is 4K+32
	uint32 reserved4:2;
	uint32 in_dmac_lut_idx: ASIC_TABLESIZE_LUT_BITS;
	/* word 2 */
	uint32 reserved5:4;
	uint32 in_svlan_id:12;
	uint32 reserved6:4;
	uint32 in_cvlan_id:12;
	/* word 3 */
	uint32 in_spa:8;
	uint32 in_ext_spa:8;
	uint32 in_pppoe_sid:16;
	/* word 4 */
	uint32 reserved9:8;
	uint32 out_svid_format_act:1;
	uint32 out_user_priority:3;
	uint32 out_cvid_format_act:1;
	uint32 reserved10:3;
	uint32 out_ext_portmask_idx:5;
	uint32 out_portmask:11;
	/* word 5 */
	uint32 out_spri_format_act:1;
	uint32 out_spri:3;
	uint32 out_svlan_id:12;
	uint32 out_cpri_format_act:1;
	uint32 out_cpri:3;
	uint32 out_cvlan_id:12;
	/* word 6 */
	uint32 in_pppoe_sid_check:1;
	uint32 in_stream_idx:7;
	uint32 reserved11:1;
	uint32 out_dscp_act:1;
	uint32 out_dscp:6;
	uint32 reserved12:1;
	uint32 out_egress_cvid_act:1;
	uint32 out_egress_svid_act:1;
	uint32 out_ctag_format_act:1;
	uint32 out_stag_format_act:1;
	uint32 out_user_pri_act:1;
	uint32 out_egress_port_to_vid_act:2;//b'00:none  b'01:SP2C	b'10:SP2S	b'11:CP2C  refto rtk_rg_out_egress_port_to_vid_act_t
	uint32 out_smac_trans:1;
	uint32 reserved13:2;
	uint32 in_tos_check:1;
	uint32 lock:1;
	uint32 in_cvlan_pri:3;
	/* word 7 */
	uint32 reserved14;
}rtk_8277_asic_path2_entry_t;

/* flow-based table - for path 3 */
typedef struct rtk_8277_asic_path3_entry_s
{
//__LITTLE_ENDIAN
	/* word 0 */
	uint32 valid:1;
	uint32 out_dmac_trans:1;
	uint32 in_path:2;
	uint32 in_ipv4_or_ipv6:1;
	uint32 in_ctagif:1;
	uint32 in_stagif:1;
	uint32 in_pppoeif:1;
	uint32 out_stream_idx_act:1;
	uint32 in_multiple_act:1;
	uint32 in_intf_idx		: ASIC_TABLESIZE_INTF_BITS;
	uint32 reserved0:4;
	uint32 out_intf_idx	: ASIC_TABLESIZE_INTF_BITS;
	uint32 in_tos:8;
	/* word 1 */
	union
	{
		uint32 in_src_ipv4_addr:32;
		uint32 in_src_ipv6_addr_hash:32;
	};
	/* word 2 */
	union
	{
		uint32 in_dst_ipv4_addr:32;
		uint32 in_dst_ipv6_addr_hash:32;
	};
	/* word 3 */
	uint32 in_l4_src_port:16;
	uint32 in_l4_dst_port:16;
	/* word 4 */
	uint32 out_dmac_idx:8;
	uint32 out_svid_format_act:1;
	uint32 out_user_priority:3;
	uint32 out_cvid_format_act:1;
	uint32 out_extra_tag_index:3;
	uint32 out_ext_portmask_idx:5;
	uint32 out_portmask:11;
	/* word 5 */
	uint32 out_spri_format_act:1;
	uint32 out_spri:3;
	uint32 out_svlan_id:12;
	uint32 out_cpri_format_act:1;
	uint32 out_cpri:3;
	uint32 out_cvlan_id:12;
	/* word 6 */
	uint32 in_pppoe_sid_check:1;
	uint32 out_stream_idx:7;
	uint32 out_multiple_act:1;
	uint32 out_dscp_act:1;
	uint32 out_dscp:6;
	uint32 out_drop:1;
	uint32 out_egress_cvid_act:1;
	uint32 out_egress_svid_act:1;
	uint32 out_ctag_format_act:1;
	uint32 out_stag_format_act:1;
	uint32 out_user_pri_act:1;
	uint32 out_egress_port_to_vid_act:2;//b'00:none  b'01:SP2C	b'10:SP2S	b'11:CP2C  refto rtk_rg_out_egress_port_to_vid_act_t
	uint32 out_smac_trans:1;
	uint32 out_uc_lut_lookup:1;
	uint32 reserved1:1;
	uint32 in_tos_check:1;
	uint32 lock:1;
	uint32 in_cvlan_pri:3;
	/* word 7 */
	uint32 out_flow_counter_act:1;
	uint32 out_flow_counter_idx:5;
	uint32 out_share_meter_act:1;
	uint32 out_share_meter_idx:5;
	uint32 in_l4proto_num:8; // l4_protocol_num
	uint32 reserved2:12;
}rtk_8277_asic_path3_entry_t;

/* flow-based table - for path 4 */
typedef struct rtk_8277_asic_path4_entry_s
{
//__LITTLE_ENDIAN
	/* word 0 */
	uint32 valid:1;
	uint32 reserved1:1;
	uint32 in_path:2;
	uint32 in_ipv4_or_ipv6:1;
	uint32 in_ctagif:1;
	uint32 in_stagif:1;
	uint32 in_pppoeif:1;
	uint32 reserved2:1;
	uint32 in_multiple_act:1;
	uint32 in_intf_idx		: ASIC_TABLESIZE_INTF_BITS;
	uint32 reserved0:4;
	uint32 out_intf_idx	: ASIC_TABLESIZE_INTF_BITS;
	uint32 in_tos:8;
	/* word 1 */
	union
	{
		uint32 in_src_ipv4_addr:32;
		uint32 in_src_ipv6_addr_hash:32;
	};
	/* word 2 */
	union
	{
		uint32 in_dst_ipv4_addr:32;
		uint32 in_dst_ipv6_addr_hash:32;
	};
	/* word 3 */
	uint32 in_l4_src_port:16;
	uint32 in_l4_dst_port:16;
	/* word 4 */
	uint32 reserved4:8;
	uint32 out_svid_format_act:1;
	uint32 out_user_priority:3;
	uint32 out_cvid_format_act:1;
	uint32 reserved5:3;
	uint32 out_ext_portmask_idx:5;
	uint32 out_portmask:11;
	/* word 5 */
	uint32 out_spri_format_act:1;
	uint32 out_spri:3;
	uint32 out_svlan_id:12;
	uint32 out_cpri_format_act:1;
	uint32 out_cpri:3;
	uint32 out_cvlan_id:12;
	/* word 6 */
	uint32 in_pppoe_sid_check:1;
	uint32 reserved6:8;
	uint32 out_dscp_act:1;
	uint32 out_dscp:6;
	uint32 reserved7:1;
	uint32 out_egress_cvid_act:1;
	uint32 out_egress_svid_act:1;
	uint32 out_ctag_format_act:1;
	uint32 out_stag_format_act:1;
	uint32 out_user_pri_act:1;
	uint32 out_egress_port_to_vid_act:2;//b'00:none  b'01:SP2C	b'10:SP2S	b'11:CP2C  refto rtk_rg_out_egress_port_to_vid_act_t
	uint32 out_smac_trans:1;
	uint32 reserved8:2;
	uint32 in_tos_check:1;
	uint32 lock:1;
	uint32 in_cvlan_pri:3;
	/* word 7 */
	uint32 in_l4proto_num:8; // l4_protocol_num
	uint32 reserved9:24;
}rtk_8277_asic_path4_entry_t;

/* flow-based table - unicast l34 routing/napt, for path 5	*/
typedef struct rtk_8277_asic_path5_entry_s
{
//__LITTLE_ENDIAN
	/* word 0 */
	uint32 valid:1;
	uint32 reserved1:1;
	uint32 in_path:2;
	uint32 in_ipv4_or_ipv6:1;
	uint32 in_ctagif:1;
	uint32 in_stagif:1;
	uint32 in_pppoeif:1;
	uint32 out_stream_idx_act:1;
	uint32 reserved2:1;
	uint32 in_intf_idx		: ASIC_TABLESIZE_INTF_BITS;
	uint32 reserved0:4;
	uint32 out_intf_idx	: ASIC_TABLESIZE_INTF_BITS;
	uint32 in_tos:8;
	/* word 1 */
	union
	{
		uint32 in_src_ipv4_addr:32;
		uint32 in_src_ipv6_addr_hash:32;
	};
	/* word 2 */
	union
	{
		uint32 in_dst_ipv4_addr:32; 	//routing/napt mode
		uint32 in_dst_ipv6_addr_hash:32;	//routing/napt mode
		uint32 out_dst_ipv4_addr:32;		//napt-r mode
	};
	/* word 3 */
	uint32 in_l4_src_port:16;
	uint32 in_l4_dst_port:16;
	/* word 4 */
	uint32 out_dmac_idx:8;
	uint32 out_svid_format_act:1;
	uint32 out_user_priority:3;
	uint32 out_cvid_format_act:1;
	uint32 out_extra_tag_index:3;
	uint32 out_l4_port:16;
	/* word 5 */
	uint32 out_spri_format_act:1;
	uint32 out_spri:3;
	uint32 out_svlan_id:12;
	uint32 out_cpri_format_act:1;
	uint32 out_cpri:3;
	uint32 out_cvlan_id:12;
	/* word 6 */
	uint32 out_l4_direction:1;				// 0:inbound, 1:outbound
	uint32 out_stream_idx:7;
	uint32 out_l4_act:1;
	uint32 out_dscp_act:1;
	uint32 out_dscp:6;
	uint32 out_drop:1;
	uint32 out_egress_cvid_act:1;
	uint32 out_egress_svid_act:1;
	uint32 out_ctag_format_act:1;
	uint32 out_stag_format_act:1;
	uint32 out_user_pri_act:1;
	uint32 out_egress_port_to_vid_act:2;//b'00:none  b'01:SP2C	b'10:SP2S	b'11:CP2C  refto rtk_rg_out_egress_port_to_vid_act_t
	uint32 reserved3:3;
	uint32 in_tos_check:1;
	uint32 lock:1;
	uint32 in_cvlan_pri:3;
	/* word 7 */
	uint32 out_flow_counter_act:1;
	uint32 out_flow_counter_idx:5;
	uint32 out_share_meter_act:1;
	uint32 out_share_meter_idx:5;
	uint32 in_l4proto_num:8; // l4_protocol_num
	uint32 reserved4:12;
}rtk_8277_asic_path5_entry_t;

/* flow-based table - dual header, for path 6  */
typedef struct rtk_8277_asic_path6_entry_s
{
//__LITTLE_ENDIAN
	/* word 0 */
	uint32 valid:1;
	uint32 reserved0:1;
	uint32 in_path:2;
	uint32 reserved1:1;
	uint32 in_ctagif:1;
	uint32 in_stagif:1;
	uint32 in_pppoeif:1;
	uint32 in_l4_src_port_check:1;
	uint32 in_l4_dst_port_check:1;
	uint32 in_intf_idx		: ASIC_TABLESIZE_INTF_BITS;
	uint32 in_l2tp_tunnel_id_check:1;
	uint32 in_l2tp_session_id_check:1;
	uint32 reserved2:2;
	uint32 in_dsliteif:1;
	uint32 in_l2tpif:1;
	uint32 in_pptpif:1;
	uint32 in_pppoe_sid_check:1;
	uint32 rsvd_in_6rdif:1;
	uint32 in_tos:8;
	/* word 1 */
	union
	{
		uint32 in_src_ipv4_addr:32;
		uint32 in_src_ipv6_addr_hash:32;
	};
	/* word 2 */
	union
	{
		uint32 in_dst_ipv4_addr:32;
		uint32 in_dst_ipv6_addr_hash:32;
	};
	/* word 3 */
	uint32 in_l4_src_port:16;
	uint32 in_l4_dst_port:16;
	/* word 4 */
	uint32 in_protocol:4;
	uint32 in_src_mac_check:1;
	uint32 in_dst_mac_check:1;
	uint32 in_src_ip_check:1;
	uint32 in_dst_ip_check:1;
	uint32 in_greEthBrif:1;
	uint32 in_vxlanif:1;
	uint32 reserved3:22;
	/* word 5 */
	uint32 in_l2tp_tunnel_id:16;
	union
	{
		uint16 in_l2tp_session_id:16;	//l2tp mode
		uint16 in_gre_call_id:16;		//pptp mode
	};
	/* word 6 */
	uint32 in_pppoe_sid:16;
	uint32 out_drop:1;
	uint32 in_outer_v6:1;
	uint32 reserved5:8;
	uint32 in_gre_call_id_check:1;
	uint32 in_tos_check:1;
	uint32 lock:1;
	uint32 in_cvlan_pri:3;
	/* word 7 */
	uint32 in_smac_lut_idx: ASIC_TABLESIZE_LUT_BITS;
	uint32 in_dmac_lut_idx: ASIC_TABLESIZE_LUT_BITS;
	uint32 reserved6: 6;
}rtk_8277_asic_path6_entry_t;

/* backward compatibility */
typedef rtk_8277_asic_path1_entry_t rtk_rg_asic_path1_entry_t;
typedef rtk_8277_asic_path2_entry_t rtk_rg_asic_path2_entry_t;
typedef rtk_8277_asic_path3_entry_t rtk_rg_asic_path3_entry_t;
typedef rtk_8277_asic_path4_entry_t rtk_rg_asic_path4_entry_t;
typedef rtk_8277_asic_path5_entry_t rtk_rg_asic_path5_entry_t;
typedef rtk_8277_asic_path6_entry_t rtk_rg_asic_path6_entry_t;

/* interface table */
typedef struct rtk_rg_asic_netif_entry_s
{
	uint8 valid;
	rtk_mac_t gateway_mac_addr;
	ipaddr_t gateway_ipv4_addr;
	uint16 intf_mtu;
	uint8 intf_mtu_check;
	uint16 out_pppoe_sid;
	uint8 out_pppoe_act;
	uint8 deny_ipv4;
	uint8 deny_ipv6;
	rtk_rg_asic_forwardAction_t ingress_action;
	rtk_rg_asic_forwardAction_t egress_action;
	rtk_portmask_t allow_ingress_portmask;
	rtk_portmask_t allow_ingress_ext_portmask;
}rtk_rg_asic_netif_entry_t;

/* interface mib table */
typedef struct rtk_rg_asic_netifMib_entry_s
{
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
}rtk_rg_asic_netifMib_entry_t;

/* extension port mask table */
typedef struct rtk_rg_asic_extPortMask_entry_s
{
	uint32 extpmask:18;
	uint16 reserved:14;

}rtk_rg_asic_extPortMask_entry_t;


/* extra tag action */
typedef struct rtk_rg_asic_extraTagActType1_s
{	// Inser extra tag to specific location
	uint32 reserve1: 4;
	uint32 src_addr_offset: ASIC_DUAL_CONTENT_BUFFER_SIZE_BITS;		// MUST be 4-bytes alignment
	uint32 length: 8;
	uint32 reserve0: 8;										// Use for upate outer_ip pointer
	uint32 act_bit: 3;
}rtk_rg_asic_extraTagActType1_t;

typedef struct rtk_rg_asic_extraTagActType2_s{
	// Update Ethertype & MTU
	uint32 reserve0: 13;
	uint32 ethertype: 16;
	uint32 act_bit: 3;
}rtk_rg_asic_extraTagActType2_t;

typedef struct rtk_rg_asic_extraTagActType3_s{
	// Update Outer IP header length
	uint32 reserve0: 2;
	uint32 operation: 2;			// 0:+; 1:-; 2:*; 3:/;
	uint32 value: 9;
	uint32 length: 8;
	uint32 pkt_buff_offset: 8;		// replaced offset
	uint32 act_bit: 3;
}rtk_rg_asic_extraTagActType3_t;

typedef struct rtk_rg_asic_extraTagActType4_s{
	// Update GRE seg or IPv4 ID
	uint32 reserve1: 17-ASIC_TABLESIZE_DUALHEADER_INTF_BITS;
	uint32 data_src_type: 1;		// 0: GRE seq/ack 1:IP Identification
	uint32 reduce_ack:1;
	uint32 reduce_seq:1;
	uint32 reserve0: 1;
	uint32 seq_ack_reg_idx: ASIC_TABLESIZE_DUALHEADER_INTF_BITS;		//should satisfy dual header interface index range
	uint32 pkt_buff_offset: 8;
	uint32 act_bit: 3;
}rtk_rg_asic_extraTagActType4_t;

typedef struct rtk_rg_asic_extraTagActType5_s{							// Caculate outer L3 header checksum (L3 checksum offload)
	uint32 reserve0: 21;
	uint32 pkt_buff_offset: 8;
	uint32 act_bit: 3;
}rtk_rg_asic_extraTagActType5_t;

typedef struct rtk_rg_asic_extraTagActType6_s{							// Caculate outer L4 header checksum (L4 checksum offload)
	uint32 reserve0: 21;
	uint32 pkt_buff_offset: 8;
	uint32 act_bit: 3;
}rtk_rg_asic_extraTagActType6_t;	

typedef struct rtk_rg_asic_extraTagActType7_s{							// SW only
	uint32 outer_tag_len: 7;
	uint32 outer_udp_offset:7;
	uint32 outer_ppp_offset:5;
	uint32 is_l2dual:1;												//SW only
	uint32 outer_is_v6:1;											//SW only
	uint32 outer_ip_offset: 8;
	uint32 act_bit: 3;
}rtk_rg_asic_extraTagActType7_t;	

typedef struct rtk_rg_asic_extraTagAction_s
{
	union{
		rtk_rg_asic_extraTagActType1_t type1;
		rtk_rg_asic_extraTagActType2_t type2;
		rtk_rg_asic_extraTagActType3_t type3;
		rtk_rg_asic_extraTagActType4_t type4;
		rtk_rg_asic_extraTagActType5_t type5;
		rtk_rg_asic_extraTagActType6_t type6;
		rtk_rg_asic_extraTagActType7_t type7; // sw_only
	};	
}rtk_rg_asic_extraTagAction_t;

/* mac address indirect access table */
typedef struct rtk_rg_asic_indirectMac_entry_s
{
	uint32 l2_idx: ASIC_TABLESIZE_LUT_BITS;
	uint32 reserved:19;
}rtk_rg_asic_indirectMac_entry_t;

typedef struct rtk_rg_asic_netif_ref_s
{
	uint8_t valid;
	rtk_mac_t netif_mac_addr;
	uint32_t mtu_idx;
	uint32_t mac_idx;
#if defined(CONFIG_CA_G3_G3LITE_SERIES)
	uint32_t acl_info;
#elif defined(CONFIG_CA8277B_SERIES)
	uint32_t acl_info;
	uint32_t acl_info_flow_acc;
	uint32_t acl_info_flow_acc_wan;
	uint32_t hash_idx;
#endif
}rtk_rg_asic_netif_ref_t;

#if defined(CONFIG_CA8277B_SERIES)
#define DMA_AFT_FIB_SIZE 16
typedef struct rtk_rg_asic_dmaAftFib_s
{
	uint32_t session_id           : 16;
    uint32_t pppoe_cmd            :  2;
    uint32_t inner_1p_sel         :  2;
    uint32_t inner_802_1p         :  3;
    uint32_t inner_dei_sel        :  2;
    uint32_t inner_dei            :  1;
    uint32_t inner_vid            :  12;
    uint32_t inner_tpid_sel       :  2 ;
    uint32_t inner_tpid_enc       :  3 ;
    uint32_t top_1p_sel           :  2 ;
    uint32_t top_802_1p           :  3 ;
    uint32_t top_dei_sel          :  2 ;
    uint32_t top_dei              :  1 ;
    uint32_t top_vid              : 12 ;
    uint32_t top_tpid_sel         :  2 ;
    uint32_t top_tpid_enc         :  3 ;
    uint32_t inner_vlan_cmd       :  2 ;
    uint32_t vlan_cnt             :  2 ;
    uint32_t vlan_vld             :  1 ;
}rtk_rg_asic_dmaAftFib_t;

typedef struct rtk_rg_asic_dmaAftTpid_s
{
	uint16_t tpid_0;
	uint16_t tpid_1;
	uint16_t tpid_2;
	uint16_t tpid_3;
}rtk_rg_asic_dmaAftTpid_t;

#endif

typedef enum
{
	ASIC_POL_RATEMODE_BPS = 0,	// bits per second
	ASIC_POL_RATEMODE_PPS,		// packet per second
	ASIC_POL_RATEMODE_END,
} asic_pol_rateMode_t;

/*
 * Function Declaration
 */
asic_ret_t rtk_rg_asic_l3qm_p4_init(void);
#if 0
asic_ret_t rtk_rg_asic_l3qm_p3EQ_used_by_p4(void);
#endif
asic_ret_t rtk_rg_asic_netifTable_add(uint32_t idx, rtk_rg_asic_netif_entry_t *pNetif);
asic_ret_t rtk_rg_asic_netifTable_del(uint32_t idx);

#if defined(CONFIG_CA8277B_SERIES)
asic_ret_t rtk_rg_asic_netifTable_get(uint32_t idx, rtk_rg_asic_netif_ref_t *pAsicNetif);
asic_ret_t rtk_rg_asic_netifTable_set(uint32_t idx, rtk_rg_asic_netif_ref_t *pAsicNetif);

asic_ret_t rtk_rg_asic_dmaAftFib_set(uint32_t idx, rtk_rg_asic_dmaAftFib_t fib);
asic_ret_t rtk_rg_asic_dmaAftFib_get(uint32_t idx, rtk_rg_asic_dmaAftFib_t *pFib);
asic_ret_t rtk_rg_asic_dmaAftTpid_set(char tpid_set_msk, rtk_rg_asic_dmaAftTpid_t aftTpid);
asic_ret_t rtk_rg_asic_dmaAftTpid_get(rtk_rg_asic_dmaAftTpid_t *pAftTpid);
#endif
asic_ret_t rtk_fc_asic_l2te_policer_enable_set(uint8_t enable);
asic_ret_t rtk_rg_l3_policer_ifg_config_set(bool ifgInclude);
asic_ret_t rtk_rg_l3_policer_ifg_config_get(    bool *pIfgInclude);
asic_ret_t rtk_rg_l3_flow_policer_mode_set(uint32_t pol_idx, bool if_enable);
asic_ret_t rtk_rg_l3_flow_policer_mode_get(uint32_t pol_idx, bool *if_enable);
asic_ret_t rtk_rg_l3_flow_policer_rate_set(uint32_t pol_idx, uint32_t committed_rate, uint32_t peak_rate);
asic_ret_t rtk_rg_l3_flow_policer_rate_get(uint32_t pol_idx, uint32_t *committed_rate, uint32_t *peak_rate);
asic_ret_t rtk_rg_l3_flow_policer_burstSize_set(uint32_t pol_idx, uint32_t committed_burstSize, uint32_t peak_burstSize);
asic_ret_t rtk_rg_l3_flow_policer_burstSize_get(uint32_t pol_idx, uint32_t *committed_burstSize, uint32_t *peak_burstSize);
asic_ret_t rtk_rg_l3_flow_policer_rateMode_set(uint32_t pol_idx, asic_pol_rateMode_t rate_mode);
asic_ret_t rtk_rg_l3_flow_policer_rateMode_get(uint32_t pol_idx, asic_pol_rateMode_t *rate_mode);
#if defined(CONFIG_CA8277B_SERIES)
asic_ret_t rtk_rg_l3_flow_policer2_mode_set(uint32_t pol_idx, bool if_enable);
asic_ret_t rtk_rg_l3_flow_policer2_mode_get(    uint32_t pol_idx, bool *if_enable);
asic_ret_t rtk_rg_l3_flow_policer2_rate_set(uint32_t pol_idx, uint32_t committed_rate, uint32_t peak_rate);
asic_ret_t rtk_rg_l3_flow_policer2_rate_get(uint32_t pol_idx, uint32_t *committed_rate, uint32_t *peak_rate);
asic_ret_t rtk_rg_l3_flow_policer2_burstSize_set(uint32_t pol_idx, uint32_t committed_burstSize, uint32_t peak_burstSize);
asic_ret_t rtk_rg_l3_flow_policer2_burstSize_get(uint32_t pol_idx, uint32_t *committed_burstSize, uint32_t *peak_burstSize);
asic_ret_t rtk_rg_l3_flow_policer2_rateMode_set(uint32_t pol_idx, asic_pol_rateMode_t rate_mode);
asic_ret_t rtk_rg_l3_flow_policer2_rateMode_get(uint32_t pol_idx, asic_pol_rateMode_t *rate_mode);
asic_ret_t rtk_rg_l3_flow_policer3_mode_set(uint32_t pol_idx, bool if_enable);
asic_ret_t rtk_rg_l3_flow_policer3_mode_get(uint32_t pol_idx, bool *if_enable);
asic_ret_t rtk_rg_l3_flow_policer3_rate_set(uint32_t pol_idx, uint32_t committed_rate, uint32_t peak_rate);
asic_ret_t rtk_rg_l3_flow_policer3_rate_get(uint32_t pol_idx, uint32_t *committed_rate, uint32_t *peak_rate);
asic_ret_t rtk_rg_l3_flow_policer3_burstSize_set(uint32_t pol_idx, uint32_t committed_burstSize, uint32_t peak_burstSize);
asic_ret_t rtk_rg_l3_flow_policer3_burstSize_get(uint32_t pol_idx, uint32_t *committed_burstSize, uint32_t *peak_burstSize);
asic_ret_t rtk_rg_l3_flow_policer3_rateMode_set(uint32_t pol_idx, asic_pol_rateMode_t rate_mode);
asic_ret_t rtk_rg_l3_flow_policer3_rateMode_get(uint32_t pol_idx, asic_pol_rateMode_t *rate_mode);
#endif

asic_ret_t rtk_rg_asic_fb_init(void);
	
#endif
