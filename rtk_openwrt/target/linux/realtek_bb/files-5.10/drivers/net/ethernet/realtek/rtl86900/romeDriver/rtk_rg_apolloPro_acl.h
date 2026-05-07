#ifndef RTK_RG_ACL_APOLLOFE_H
#define RTK_RG_ACL_APOLLOFE_H

#include <rtk_rg_struct.h>


#define ACL_PATTERN_HIT  1
#define ACL_PATTERN_UNHIT  0

#define RANGE_TABLE_SEARCH_FOR_ACL 0
#define RANGE_TABLE_SEARCH_FOR_CF 1

//avoid function size larger than 1024 byte
#define ACL_PER_RULE_FIELD_SIZE 8
#define GLOBAL_ACL_FIELD_SIZE APOLLOFE_RG_ACL_TEMPLATE_END
#define GLOBAL_ACL_RULE_SIZE 8

#ifndef PATTERN_CHECK_INIT
#define PATTERN_CHECK_INIT(result) { result = ACL_PATTERN_HIT; }
#endif


#ifndef PATTERN_CHECK_RESULT
#define PATTERN_CHECK_RESULT( ruleIdx, inverse, result, comment ,arg... ) { \
		if(!((inverse) ^ (result))){ ACL(comment,ruleIdx); continue;} \
	}
#endif

#define ACTION_CHECK_HW_NEED(acl_filter) \
	((acl_filter->action_type==ACL_ACTION_TYPE_DROP)||(acl_filter->action_type==ACL_ACTION_TYPE_TRAP)||(acl_filter->action_type==ACL_ACTION_TYPE_PERMIT)|| \
	(acl_filter->action_type==ACL_ACTION_TYPE_TRAP_TO_PS)||(acl_filter->action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY)||(acl_filter->action_type==ACL_ACTION_TYPE_TRAP2SLAVE_WITH_PRIORITY)|| \
	(acl_filter->action_type==ACL_ACTION_TYPE_QOS && (acl_filter->qos_actions&ACL_ACTION_TOS_TC_REMARKING_BIT)))

#define ACTION_CHECK_STOP_HW(acl_filter) \
	((acl_filter->action_type==ACL_ACTION_TYPE_PERMIT)||(acl_filter->action_type==ACL_ACTION_TYPE_TRAP)||(acl_filter->action_type==ACL_ACTION_TYPE_TRAP_TO_PS))

#define ACTION_CHECK_ACT_QOS_IGR_VID(acl_filter) \
	((acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)&&(acl_filter.action_type==ACL_ACTION_TYPE_QOS && (acl_filter.qos_actions&ACL_ACTION_ACL_INGRESS_VID_BIT)))

#define RG_ACL_EGRESS_BYPASS_PORT_ESPECIAL_TO_PS_CHECK 0xffff

#if defined(CONFIG_CMCC)||defined(CONFIG_CU_BASEON_CMCC)
#define MAX_ACL_SW_ENTRY_SIZE 512
#else
#define MAX_ACL_SW_ENTRY_SIZE 128
#endif
#define MAX_ACL_IPRANGETABLE_SIZE 	16
#define MAX_ACL_PORTRANGETABLE_SIZE 16
#define MAX_ACL_PKTLENRANGETABLE_SIZE RTL9607C_MAX_NUM_OF_RANGE_CHECK_PKTLEN
#define USER_ACL_PKTLENRANGETABLE_START 4
#define MAX_ACL_TEMPLATE_SIZE 8

#if defined(CONFIG_APOLLO_FPGA_PHY_TEST)
#define MAX_ACL_ENTRY_SIZE 	8 // [FIXME] limit size for FPGA verify
//#define DEFAULT_CF_PATTERN0_ENTRY_SIZE 	6  // [FIXME] limit size for FPGA verify, pattern0 use CF[0~9], pattern1 use CF[10~15]
#elif defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_ACL_ENTRY_SIZE 	64
#else
#define MAX_ACL_ENTRY_SIZE 	128
//#define DEFAULT_CF_PATTERN0_ENTRY_SIZE 	128
#endif

#define TOTAL_CF_ENTRY_SIZE 256
#if defined(CONFIG_RG_RTL9603CVD_SERIES)
#define CF_UNI_DEFAULT 0x7	//if flood or multicast, uni default is 0b111(uni 3 bit)
#else
#define CF_UNI_DEFAULT 0xF	//if flood or multicast, uni default is 0b1111(uni 4 bit)
#endif
#define MAX_CF_GEMIDX_VALUE 0x7f
#define SW_CF_GEMIDX_MASK 0xffff
#define MAX_CF_LLID_VALUE 0xf


#define MIN_ACL_ENTRY_INDEX RESERVED_ACL_BEFORE
#define MAX_ACL_ENTRY_INDEX RESERVED_ACL_AFTER

#define RESERVED_ACL_BEFORE rg_db.systemGlobal.aclAndCfReservedRule.aclLowerBoundary
#define RESERVED_ACL_AFTER rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary




#define INGRESS_ACL_PATTERN_BITS \
(INGRESS_PORT_BIT|INGRESS_INTF_BIT|INGRESS_ETHERTYPE_BIT|INGRESS_CTAG_PRI_BIT|INGRESS_CTAG_VID_BIT| \
INGRESS_SMAC_BIT|INGRESS_DMAC_BIT|INGRESS_DSCP_BIT|INGRESS_L4_TCP_BIT|INGRESS_L4_UDP_BIT| \
INGRESS_IPV6_SIP_RANGE_BIT|INGRESS_IPV6_DIP_RANGE_BIT|INGRESS_IPV4_SIP_RANGE_BIT|INGRESS_IPV4_DIP_RANGE_BIT|INGRESS_L4_SPORT_RANGE_BIT|INGRESS_L4_DPORT_RANGE_BIT| \
INGRESS_L4_ICMP_BIT|INGRESS_IPV6_DSCP_BIT| INGRESS_STREAM_ID_BIT|INGRESS_STAG_PRI_BIT|INGRESS_STAG_VID_BIT|INGRESS_STAGIF_BIT|INGRESS_CTAGIF_BIT| \
INGRESS_L4_POROTCAL_VALUE_BIT|INGRESS_TOS_BIT|INGRESS_IPV6_TC_BIT|INGRESS_IPV6_SIP_BIT|INGRESS_IPV6_DIP_BIT|INGRESS_WLANDEV_BIT| \
INGRESS_IPV4_TAGIF_BIT|INGRESS_IPV6_TAGIF_BIT|INGRESS_L4_ICMPV6_BIT|INGRESS_CTAG_CFI_BIT|INGRESS_STAG_DEI_BIT|INGRESS_L4_NONE_TCP_NONE_UDP_BIT| \
INGRESS_IPV6_FLOWLABEL_BIT|INGRESS_TCP_FLAGS_BIT|INGRESS_PKT_LEN_RANGE_BIT)


#define EGRESS_ACL_PATTERN_BITS \
(EGRESS_INTF_BIT|EGRESS_IPV4_SIP_RANGE_BIT|EGRESS_IPV4_DIP_RANGE_BIT|EGRESS_L4_SPORT_RANGE_BIT|EGRESS_L4_DPORT_RANGE_BIT| \
EGRESS_CTAG_PRI_BIT|EGRESS_CTAG_VID_BIT|EGRESS_IP4MC_IF|EGRESS_IP6MC_IF|EGRESS_DMAC_BIT|EGRESS_SMAC_BIT|EGRESS_WLANDEV_BIT|INGRESS_EGRESS_PORTIDX_BIT)
//20170512: ACL rules whitch include INGRESS_IPV6_SIP_RANGE_BIT or INGRESS_IPV6_DIP_RANGE_BIT pattern should not be add to HW
#define HW_ACL_SUPPORT_PATTERN (INGRESS_ACL_PATTERN_BITS&(~(INGRESS_INTF_BIT|INGRESS_IPV6_SIP_RANGE_BIT|INGRESS_IPV6_DIP_RANGE_BIT|INGRESS_L4_POROTCAL_VALUE_BIT|INGRESS_WLANDEV_BIT)))
#define HW_ACL_SUPPORT_PATTERN_9607C (INGRESS_ACL_PATTERN_BITS&(~(INGRESS_INTF_BIT|INGRESS_IPV6_SIP_RANGE_BIT|INGRESS_IPV6_DIP_RANGE_BIT|INGRESS_WLANDEV_BIT)))


//OringialACL actions
#define HW_ACL_SUPPORT_ACTION_BITS \
(ACL_ACTION_1P_REMARKING_BIT|ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT|ACL_ACTION_DSCP_REMARKING_BIT|ACL_ACTION_QUEUE_ID_BIT| \
ACL_ACTION_SHARE_METER_BIT| ACL_ACTION_ACL_PRIORITY_BIT|ACL_ACTION_ACL_INGRESS_VID_BIT|ACL_ACTION_REDIRECT_BIT|ACL_ACTION_LOG_COUNTER_BIT)

//Oringial CF actions, shoule be supported by FB in apolloPro
#define HW_ACL_NOT_SUPPORT_ACTION_BITS \
(ACL_ACTION_ACL_CVLANTAG_BIT|ACL_ACTION_ACL_SVLANTAG_BIT|ACL_ACTION_DS_UNIMASK_BIT|ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT|ACL_ACTION_STREAM_ID_OR_LLID_BIT)


#define HW_ACL_REARRANGE_PROTECT_VERSION 2
#if defined(HW_ACL_REARRANGE_PROTECT_VERSION) && (HW_ACL_REARRANGE_PROTECT_VERSION==2)
typedef enum rtk_rg_aclHWRearrange_protect_rule_idx_s
{
	HW_ACL_REARRANGE_PROTECT_V4MC_PERMIT = 0,
	HW_ACL_REARRANGE_PROTECT_V6MC_PERMIT = 1,
	HW_ACL_REARRANGE_PROTECT_OTHERS_TRAP = 2,
	HW_ACL_REARRANGE_PROTECT_RSV_HEAD = 3,
} rtk_rg_aclHWRearrange_protect_rule_idx_t;
#endif

typedef enum rtk_rg_aclSWEntry_used_tables_field_s
{
	APOLLOFE_RG_ACL_USED_INGRESS_SIP4TABLE = (1<<0),
	APOLLOFE_RG_ACL_USED_INGRESS_DIP4TABLE = (1<<1),
	APOLLOFE_RG_ACL_USED_INGRESS_SIP6TABLE = (1<<2),
	APOLLOFE_RG_ACL_USED_INGRESS_DIP6TABLE = (1<<3),
	APOLLOFE_RG_ACL_USED_INGRESS_SPORTTABLE = (1<<4),
	APOLLOFE_RG_ACL_USED_INGRESS_DPORTTABLE = (1<<5),
	APOLLOFE_RG_ACL_USED_EGRESS_SIP4TABLE = (1<<6),
	APOLLOFE_RG_ACL_USED_EGRESS_DIP4TABLE = (1<<7),
	APOLLOFE_RG_ACL_USED_EGRESS_SIP6TABLE = (1<<8),
	APOLLOFE_RG_ACL_USED_EGRESS_DIP6TABLE = (1<<9),
	APOLLOFE_RG_ACL_USED_EGRESS_SPORTTABLE = (1<<10),
	APOLLOFE_RG_ACL_USED_EGRESS_DPORTTABLE = (1<<11),
	APOLLOFE_RG_ACL_USED_INGRESS_PKTLENTABLE = (1<<12),
} rtk_rg_aclSWEntry_used_tables_field_t;

typedef enum rtk_rg_aclSWEntry_used_tables_index_s
{
	APOLLOFE_RG_ACL_USED_INGRESS_SIP4TABLE_INDEX = 0,
	APOLLOFE_RG_ACL_USED_INGRESS_DIP4TABLE_INDEX = 1,
	APOLLOFE_RG_ACL_USED_INGRESS_SIP6TABLE_INDEX = 2,
	APOLLOFE_RG_ACL_USED_INGRESS_DIP6TABLE_INDEX = 3,
	APOLLOFE_RG_ACL_USED_INGRESS_SPORTTABLE_INDEX = 4,
	APOLLOFE_RG_ACL_USED_INGRESS_DPORTTABLE_INDEX = 5,
	APOLLOFE_RG_ACL_USED_EGRESS_SIP4TABLE_INDEX = 6,
	APOLLOFE_RG_ACL_USED_EGRESS_DIP4TABLE_INDEX = 7,
	APOLLOFE_RG_ACL_USED_EGRESS_SIP6TABLE_INDEX = 8,
	APOLLOFE_RG_ACL_USED_EGRESS_DIP6TABLE_INDEX = 9,
	APOLLOFE_RG_ACL_USED_EGRESS_SPORTTABLE_INDEX = 10,
	APOLLOFE_RG_ACL_USED_EGRESS_DPORTTABLE_INDEX = 11,
	APOLLOFE_RG_ACL_USED_INGRESS_PKTLENTABLE_INDEX = 12,
	USED_TABLE_END = 13,
} rtk_rg_aclSWEntry_used_tables_index_t;

typedef enum rtk_rg_aclSWEntry_icmp_trap_disable_status_s
{
	RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_ENABLE = 0,
	RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_IPV4_DISABLE,
	RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_IPV6_DISABLE,
	RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_DISABLE,
	RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_END,
} rtk_rg_aclSWEntry_icmp_trap_disable_status_t;


typedef enum rtk_rg_aclField_Teamplate_s
{
	//related to Template[0]: used for L2 pattern
	TEMPLATE_DMAC0 				=0,
	TEMPLATE_DMAC1 				=1,
	TEMPLATE_DMAC2 				=2,
	TEMPLATE_SMAC0 				=3,
	TEMPLATE_SMAC1 				=4,
	TEMPLATE_SMAC2 				=5,
	TEMPLATE_CTAG 				=6,
	TEMPLATE_STAG					=7,
	//related to Template[1]: used for L3 pattern
	TEMPLATE_IPv4DIP0 				=8,
	TEMPLATE_IPv4DIP1 				=9,
	TEMPLATE_IPv4SIP0 				=10,
	TEMPLATE_IPv4SIP1				=11,
	TEMPLATE_IP_RANGE				=12,
	TEMPLATE_ETHERTYPE 			=13,
	TEMPLATE_EXTPORTMASK			=14,
	TEMPLATE_GEMPORT				=15,
	//related to Template[2]: used for L4 pattern
	TEMPLATE_L4_DPORT				=16, //use Field selector[1]
	TEMPLATE_L4_SPORT				=17, //use Field selector[0]
	TEMPLATE_L4PORT_RANGE		=18,
	TEMPLATE_IP4_TOS_PROTO		=19,
	TEMPLATE_IP6_TC_NH 			=20,
	TEMPLATE_FRAME_TYPE_TAGS	=21,
	TEMPLATE_RSV_2_6 			=22, //not used
	TEMPLATE_PKT_LEN_RANGE		=23, //not used
	//related to Template[3]: rsv
	TEMPLATE_IPCP_IP6CP			=24, //FS[0]: init for IPCP / IP6CP
	TEMPLATE_FLAG_AND_OFFSET	=25, //FS[1]: init for ipv4 Flags and Fragment offset
	TEMPLATE_TCP_FLAGS			=26, //FS[2]: init for TCP Flags
	TEMPLATE_IPV6_FLOW_LABEL_0	=27, //FS[3]: init for ipv6 floeLabel[0:3]
	TEMPLATE_IPV6_FLOW_LABEL_1	=28, //FS[4]: init for ipv6 floeLabel[4:19]
	TEMPLATE_RSV_3_5			=29, //not used
	TEMPLATE_RSV_3_6			=30, //not used
	TEMPLATE_RSV_3_7			=31, //not used
	//related to Template[4]: rsv
	TEMPLATE_RSV_4_0			=32, //not used
	TEMPLATE_RSV_4_1			=33, //not used
	TEMPLATE_RSV_4_2			=34, //not used
	TEMPLATE_RSV_4_3			=35, //not used
	TEMPLATE_RSV_4_4			=36, //not used
	TEMPLATE_RSV_4_5			=37, //not used
	TEMPLATE_RSV_4_6			=38, //not used
	TEMPLATE_RSV_4_7			=39, //not used

	//related to Template[5]: SIPv6
	TEMPLATE_IPv6SIP0			=40,
	TEMPLATE_IPv6SIP1			=41,
	TEMPLATE_IPv6SIP2			=42,
	TEMPLATE_IPv6SIP3			=43,
	TEMPLATE_IPv6SIP4			=44,
	TEMPLATE_IPv6SIP5			=45,
	TEMPLATE_IPv6SIP6			=46,
	TEMPLATE_IPv6SIP7			=47,

	//related to Template[6]: DIPv6
	TEMPLATE_IPv6DIP0			=48,
	TEMPLATE_IPv6DIP1			=49,
	TEMPLATE_IPv6DIP2			=50,
	TEMPLATE_IPv6DIP3			=51,
	TEMPLATE_IPv6DIP4			=52,
	TEMPLATE_IPv6DIP5			=53,
	TEMPLATE_IPv6DIP6			=54,
	TEMPLATE_IPv6DIP7			=55,

	//related to Template[7]: flexible use for port range in user ACL
	TEMPLATE_TCP_DPORT			=56,
	TEMPLATE_TCP_SPORT			=57,
	TEMPLATE_UDP_DPORT			=58,
	TEMPLATE_UDP_SPORT 			=59,
	TEMPLATE_IP4SIP0_INNER		=60,
	TEMPLATE_IP4SIP1_INNER		=61,
	TEMPLATE_IP4DIP0_INNER		=62,
	TEMPLATE_IP4DIP1_INNER		=63,

	APOLLOFE_RG_ACL_TEMPLATE_END,
}rtk_rg_aclField_Teamplate_t;


typedef enum rtk_rg_acl_TagIf_bit_e
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
}rtk_rg_acl_TagIf_bit_t; //use for H/W acl Framte type and Tags mapping, do not change the order!


#if !defined(CONFIG_RG_G3_SERIES)	// CONFIG_RG_G3_SERIES_DEVELOPMENT
//G3 implement rsvACL in file rtk_rg_acl_g3.c / rtk_rg_acl_g3.h
typedef enum rtk_rg_aclAndCf_reserved_type_e
{
	/*HEAD PATCH: The lower index number, the higher acl&cf priority*/

	RTK_RG_ACLANDCF_RESERVED_ALL_TRAP=0,
	RTK_RG_ACLANDCF_RESERVED_UNICAST_TRAP,
	RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP,
	RTK_RG_ACLANDCF_RESERVED_STPBLOCKING,
	RTK_RG_ACLANDCF_RESERVED_DOT1X_EAPOL_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF0_IPV4_FRAGMENT_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF1_IPV4_FRAGMENT_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF2_IPV4_FRAGMENT_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF3_IPV4_FRAGMENT_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF4_IPV4_FRAGMENT_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF5_IPV4_FRAGMENT_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF6_IPV4_FRAGMENT_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF7_IPV4_FRAGMENT_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF8_IPV4_FRAGMENT_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF9_IPV4_FRAGMENT_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF10_IPV4_FRAGMENT_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF11_IPV4_FRAGMENT_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF12_IPV4_FRAGMENT_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF13_IPV4_FRAGMENT_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF14_IPV4_FRAGMENT_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF15_IPV4_FRAGMENT_TRAP,
	RTK_RG_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY,
	RTK_RG_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY,
	RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE0_TRAP,
	RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE1_TRAP,
	RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE2_TRAP,
	RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE3_TRAP,
	RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE4_TRAP,
	RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE5_TRAP,
	RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE6_TRAP,
	RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE7_TRAP,
	RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY,
	RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_TRAP_OR_ASSIGN_PRIORITY,
	RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP_OR_ASSIGN_PRIORITY,
	RTK_RG_ACLANDCF_RESERVED_DSLITE_TRAP,
	RTK_RG_ACLANDCF_RESERVED_IGMP_MLD_DROP,
	RTK_RG_ACLANDCF_RESERVED_UNICAST_REDIRECT,
	RTK_RG_ACLANDCF_RESERVED_UNTAG_UNICAST_REDIRECT,

	RTK_RG_ACLANDCF_RESERVED_HEAD_END,

	/*TAIL PATCH: The lower index number, the lower acl&cf priority*/
	RTK_RG_ACLANDCF_RESERVED_DHCP_PACKET_ASSIGN_SHARE_METER,
	RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_ASSIGN_SHARE_METER,
	RTK_RG_ACLANDCF_RESERVED_ARP_PACKET_ASSIGN_SHARE_METER,
	RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER,
	RTK_RG_ACLANDCF_RESERVED_EGRESS_VLAN_FILTER_DISABLE,
	RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT,
	RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP,
	RTK_RG_ACLANDCF_RESERVED_NPTV6_ACC_UPSTREAM_FROM_LAN0_ASSIGN_SHARE_METER,
	RTK_RG_ACLANDCF_RESERVED_NPTV6_ACC_UPSTREAM_FROM_LAN1_ASSIGN_SHARE_METER,
	RTK_RG_ACLANDCF_RESERVED_NPTV6_ACC_UPSTREAM_FROM_LAN2_ASSIGN_SHARE_METER,
	RTK_RG_ACLANDCF_RESERVED_NPTV6_ACC_UPSTREAM_FROM_LAN3_ASSIGN_SHARE_METER,
    RTK_RG_ACLANDCF_RESERVED_TAIL_END,

}rtk_rg_aclAndCf_reserved_type_t;


typedef struct rtk_rg_aclAndCf_reserved_l2tp_control_lcp_trap_and_assign_priority_s{
	int priority;
}rtk_rg_aclAndCf_reserved_l2tp_control_lcp_trap_and_assign_priority_t;

typedef struct rtk_rg_aclAndCf_reserved_pppoe_lcp_assign_priority_s{
	int priority;
}rtk_rg_aclAndCf_reserved_pppoe_lcp_assign_priority_t;

typedef struct rtk_rg_aclAndCf_reserved_ack_packet_assign_priority_s{
	int priority;
}rtk_rg_aclAndCf_reserved_ack_packet_assign_priority_t;

typedef struct rtk_rg_aclAndCf_reserved_ack_packet_trap_or_assign_priority_s{
	uint8 action;	//0: trap; 1: assign priority; 2: trap with priority
	int priority;	//only available when action is 1 or 2
	uint32 portmask;
	uint32 pktLenStart;
	uint32 pktLenEnd;
	uint16 tcp_flag_data;
	uint16 tcp_flag_mask;
}rtk_rg_aclAndCf_reserved_ack_packet_trap_or_assign_priority_t;

typedef struct rtk_rg_aclAndCf_reserved_igmp_mld_drop_s{
	uint32 portmask;
}rtk_rg_aclAndCf_reserved_igmp_mld_drop_t;

typedef struct rtk_rg_aclAndCf_reserved_unicast_redirect_s{
	int redirect_to_port;
	int cvlan_cvid;
}rtk_rg_aclAndCf_reserved_unicast_redirect_t;

typedef struct rtk_rg_aclAndCf_reserved_untag_unicast_redirect_s{
	int ingress_portmask;
	int redirect_to_port;
	int acl_igr_cvlan_cvid;
	int cf_egr_cvlan_cvid;
}rtk_rg_aclAndCf_reserved_untag_unicast_redirect_t;

typedef struct rtk_rg_aclAndCf_reserved_syn_packet_assign_share_meter_s{
	int share_meter;
}rtk_rg_aclAndCf_reserved_syn_packet_assign_share_meter_t;

typedef struct rtk_rg_aclAndCf_reserved_arp_packet_assign_share_meter_s{
	uint32 portmask;
	int share_meter;
}rtk_rg_aclAndCf_reserved_arp_packet_assign_share_meter_t;

typedef struct rtk_rg_aclAndCf_reserved_dhcp_packet_assign_share_meter_s{
	uint32 portmask;
	int share_meter;
}rtk_rg_aclAndCf_reserved_dhcp_packet_assign_share_meter_t;

typedef struct rtk_rg_aclAndCf_reserved_intf_ipv4_fragment_trap_s{
	rtk_mac_t gmac;
}rtk_rg_aclAndCf_reserved_intf_ipv4_fragment_trap_t;

typedef struct rtk_rg_aclAndCf_multicast_pppoe_trap_s{
	rtk_mac_t dmac;
	uint32 dip;
}rtk_rg_aclAndCf_multicast_pppoe_trap_t;

//backward compatible for trap with priority related feature: assign higher rx priority for below control packet
typedef struct rtk_rg_aclAndCf_reserved_arp_assign_priority_s{
	int user_acl_idx;
	int priority;
}rtk_rg_aclAndCf_reserved_arp_assign_priority_t;

typedef struct rtk_rg_aclAndCf_reserved_loop_detect_assign_priority_s{
	int user_acl_idx;
	int priority;
}rtk_rg_aclAndCf_reserved_loop_detect_assign_priority_t;

typedef struct rtk_rg_aclAndCf_reserved_snmp_assign_priority_s{
	int user_acl_idx;
	int priority;
}rtk_rg_aclAndCf_reserved_snmp_assign_priority_t;

typedef struct rtk_rg_aclAndCf_reserved_igmp_assign_priority_s{
	int user_acl_idx;
	int user_acl_idx_mld;
	int priority;
}rtk_rg_aclAndCf_reserved_igmp_assign_priority_t;

typedef struct rtk_rg_aclAndCf_reserved_dhcp_assign_priority_s{
	int user_acl_idx;
	int priority;
}rtk_rg_aclAndCf_reserved_dhcp_assign_priority_t;

typedef struct rtk_rg_aclAndCf_reserved_dhcpv6_assign_priority_s{
	int user_acl_idx;
	int priority;
}rtk_rg_aclAndCf_reserved_dhcpv6_assign_priority_t;

typedef struct rtk_rg_aclAndCf_reserved_accessIP_assign_priority_s{
	int user_acl_idx;
	int priority;
	uint32 ip_addr;
}rtk_rg_aclAndCf_reserved_accessIP_assign_priority_t;

typedef struct rtk_rg_aclAndCf_reserved_nptv6_acc_upstream_assign_share_meter_s{
	int share_meter;
}rtk_rg_aclAndCf_reserved_nptv6_acc_upstream_assign_share_meter_t;

typedef struct rtk_rg_aclAndCf_reserved_global_s
{
	uint32 aclLowerBoundary; //The End index of acl reserve rules in the head
	uint32 aclUpperBoundary; //The start index of acl reserve rules in the tail
	uint32 cfLowerBoundary;  //The start index of cf reserve rules in the head
	uint32 cfUpperBoundary;  //The start index of cf reserve rules in the tail
	uint8 reservedMask[RTK_RG_ACLANDCF_RESERVED_TAIL_END];
	//rtk_rg_aclAndCf_reserved_type_t reservedMask;

	uint32 acl_MC_temp_permit_idx; //this index is reserved for user ACL rearrange, and reserved by type RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP or RTK_RG_ACLANDCF_RESERVED_ALL_TRAP

	rtk_rg_aclAndCf_reserved_l2tp_control_lcp_trap_and_assign_priority_t l2tp_ctrl_lcp_assign_prioity;
	rtk_rg_aclAndCf_reserved_pppoe_lcp_assign_priority_t pppoe_lcp_assign_prioity;
	rtk_rg_aclAndCf_reserved_ack_packet_assign_priority_t ack_packet_assign_priority;
	rtk_rg_aclAndCf_reserved_ack_packet_trap_or_assign_priority_t ack_packet_trap_or_assign_priority;
	rtk_rg_aclAndCf_reserved_igmp_mld_drop_t igmp_mld_drop_portmask;
	rtk_rg_aclAndCf_reserved_intf_ipv4_fragment_trap_t intf_ipv4_trap[15];
	rtk_rg_aclAndCf_reserved_unicast_redirect_t unicast_redirect;
	rtk_rg_aclAndCf_reserved_untag_unicast_redirect_t untag_unicast_redirect;
	rtk_rg_aclAndCf_reserved_syn_packet_assign_share_meter_t syn_packet_assign_share_meter;
	rtk_rg_aclAndCf_reserved_arp_packet_assign_share_meter_t arp_packet_assign_share_meter;
	rtk_rg_aclAndCf_reserved_dhcp_packet_assign_share_meter_t dhcp_packet_assign_share_meter;
	rtk_rg_aclAndCf_multicast_pppoe_trap_t	mc_pppoe_trap[8];

	//backward compatible for trap with priority related feature: assign higher rx priority for below control packet
	rtk_rg_aclAndCf_reserved_arp_assign_priority_t arp_packet_assign_priority;
	rtk_rg_aclAndCf_reserved_loop_detect_assign_priority_t loop_detect_packet_assign_priority;
	rtk_rg_aclAndCf_reserved_snmp_assign_priority_t snmp_packet_assign_priority;
	rtk_rg_aclAndCf_reserved_igmp_assign_priority_t igmp_packet_assign_priority;
	rtk_rg_aclAndCf_reserved_dhcp_assign_priority_t dhcp_packet_assign_priority;
	rtk_rg_aclAndCf_reserved_dhcpv6_assign_priority_t dhcpv6_packet_assign_priority;
	rtk_rg_aclAndCf_reserved_accessIP_assign_priority_t accessIP_packet_assign_priority_and_addr;
	rtk_rg_aclAndCf_reserved_nptv6_acc_upstream_assign_share_meter_t nptv6_acc_upstream_assign_share_meter[4];
	
}rtk_rg_acl_reserved_global_t;

#endif //end of #if !defined(CONFIG_RG_G3_SERIES_DEVELOPMENT)

#endif //end of #ifndef RTK_RG_ACL_APOLLOFE_H

