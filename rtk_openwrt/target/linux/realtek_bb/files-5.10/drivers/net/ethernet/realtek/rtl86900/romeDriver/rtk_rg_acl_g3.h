#ifndef RTK_RG_ACL_G3_H
#define RTK_RG_ACL_G3_H

#include <rtk_rg_struct.h>
#include <classifier.h>	//for ca_classifier_rule_add
#include <special_packet.h>	//for ca_special_packet_set

#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
#include <port.h>			//for loopback definition
#endif

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

#define PATTERN_CHECK_DMAC_IS_MC(acl_filter) \
	((acl_filter->ingress_dmac.octet[0]==0x1) && (acl_filter->ingress_dmac_mask.octet[0]==0x1) &&  \
		((acl_filter->ingress_dmac.octet[1]|acl_filter->ingress_dmac.octet[2]|acl_filter->ingress_dmac.octet[3]| \
			acl_filter->ingress_dmac.octet[4]|acl_filter->ingress_dmac.octet[5])==0x0) && \
		((acl_filter->ingress_dmac_mask.octet[1]|acl_filter->ingress_dmac_mask.octet[2]|acl_filter->ingress_dmac_mask.octet[3]| \
			acl_filter->ingress_dmac_mask.octet[4]|acl_filter->ingress_dmac_mask.octet[5])==0x0))
	
#define PATTERN_CHECK_DMAC_IS_MC_IPV4(acl_filter) \
		((acl_filter->ingress_dmac.octet[0]==0x1) && (acl_filter->ingress_dmac.octet[1]==0x0) && (acl_filter->ingress_dmac.octet[2]==0x5E) && \
			(acl_filter->ingress_dmac_mask.octet[0]==0xff) && (acl_filter->ingress_dmac_mask.octet[1]==0xff) && (acl_filter->ingress_dmac_mask.octet[2]==0xff) &&  \
			((acl_filter->ingress_dmac.octet[3]|acl_filter->ingress_dmac.octet[4]|acl_filter->ingress_dmac.octet[5])==0x0) && \
			((acl_filter->ingress_dmac_mask.octet[3]|acl_filter->ingress_dmac_mask.octet[4]|acl_filter->ingress_dmac_mask.octet[5])==0x0))
	
#define PATTERN_CHECK_DMAC_IS_MC_IPV6(acl_filter) \
		((acl_filter->ingress_dmac.octet[0]==0x33) && (acl_filter->ingress_dmac.octet[1]==0x33) && \
			(acl_filter->ingress_dmac_mask.octet[0]==0xff) && (acl_filter->ingress_dmac_mask.octet[1]==0xff) && \
			((acl_filter->ingress_dmac.octet[2]|acl_filter->ingress_dmac.octet[3]|acl_filter->ingress_dmac.octet[4]|acl_filter->ingress_dmac.octet[5])==0x0) && \
			((acl_filter->ingress_dmac_mask.octet[2]|acl_filter->ingress_dmac_mask.octet[3]|acl_filter->ingress_dmac_mask.octet[4]|acl_filter->ingress_dmac_mask.octet[5])==0x0))
	
#define PATTERN_CHECK_DMAC_IS_UC(acl_filter) \
		((acl_filter->ingress_dmac.octet[0]==0x0) && (acl_filter->ingress_dmac_mask.octet[0]==0x1) &&  \
			((acl_filter->ingress_dmac.octet[1]|acl_filter->ingress_dmac.octet[2]|acl_filter->ingress_dmac.octet[3]| \
				acl_filter->ingress_dmac.octet[4]|acl_filter->ingress_dmac.octet[5])==0x0) && \
			((acl_filter->ingress_dmac_mask.octet[1]|acl_filter->ingress_dmac_mask.octet[2]|acl_filter->ingress_dmac_mask.octet[3]| \
				acl_filter->ingress_dmac_mask.octet[4]|acl_filter->ingress_dmac_mask.octet[5])==0x0))
	
#define PATTERN_CHECK_DMAC_IS_RSVD(acl_filter) \
		((acl_filter->ingress_dmac.octet[0]==0x1) && (acl_filter->ingress_dmac.octet[1]==0x80) && (acl_filter->ingress_dmac.octet[2]==0xC2) && \
			(acl_filter->ingress_dmac_mask.octet[0]==0xff) && (acl_filter->ingress_dmac_mask.octet[1]==0xff) && (acl_filter->ingress_dmac_mask.octet[2]==0xff) && \
			(acl_filter->ingress_dmac_mask.octet[3]==0xff) && (acl_filter->ingress_dmac_mask.octet[4]==0xff) &&  \
			((acl_filter->ingress_dmac.octet[3]|acl_filter->ingress_dmac.octet[4]|acl_filter->ingress_dmac.octet[5]|acl_filter->ingress_dmac_mask.octet[5])==0x0))

#define ACTION_CHECK_HW_NEED(acl_filter) \
	((acl_filter->action_type==ACL_ACTION_TYPE_DROP)||(acl_filter->action_type==ACL_ACTION_TYPE_TRAP)||(acl_filter->action_type==ACL_ACTION_TYPE_PERMIT)|| \
	(acl_filter->action_type==ACL_ACTION_TYPE_TRAP_TO_PS)||(acl_filter->action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY))

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
#define MAX_ACL_TEMPLATE_SIZE 8

#if defined(CONFIG_APOLLO_FPGA_PHY_TEST)
#define MAX_ACL_ENTRY_SIZE 	8 // [FIXME] limit size for FPGA verify
//#define DEFAULT_CF_PATTERN0_ENTRY_SIZE 	6  // [FIXME] limit size for FPGA verify, pattern0 use CF[0~9], pattern1 use CF[10~15]
#else
#define MAX_ACL_ENTRY_SIZE 	128
//#define DEFAULT_CF_PATTERN0_ENTRY_SIZE 	128
#endif

#define TOTAL_CF_ENTRY_SIZE 256
#define CF_UNI_DEFAULT 0x3F	//Same mechanism as ApolloPro, if flood or multicast, uni default is 0x3f (reserve port 0~63)


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
INGRESS_IPV4_TAGIF_BIT|INGRESS_IPV6_TAGIF_BIT|INGRESS_L4_ICMPV6_BIT|INGRESS_CTAG_CFI_BIT|INGRESS_STAG_DEI_BIT|INGRESS_L4_NONE_TCP_NONE_UDP_BIT|INGRESS_IPV6_FLOWLABEL_BIT)


#define EGRESS_ACL_PATTERN_BITS \
(EGRESS_INTF_BIT|EGRESS_IPV4_SIP_RANGE_BIT|EGRESS_IPV4_DIP_RANGE_BIT|EGRESS_L4_SPORT_RANGE_BIT|EGRESS_L4_DPORT_RANGE_BIT| \
EGRESS_CTAG_PRI_BIT|EGRESS_CTAG_VID_BIT|EGRESS_IP4MC_IF|EGRESS_IP6MC_IF|EGRESS_DMAC_BIT|EGRESS_SMAC_BIT|EGRESS_WLANDEV_BIT|INGRESS_EGRESS_PORTIDX_BIT)

//G3 HW not support (1. reserve STAG for satern, 2. L3 CLS not support mask/ip range/tos/tc/reverse check)
#define HW_ACL_SUPPORT_PATTERN (INGRESS_ACL_PATTERN_BITS&(~(INGRESS_INTF_BIT|INGRESS_WLANDEV_BIT| \
INGRESS_STAG_PRI_BIT|INGRESS_STAG_VID_BIT|INGRESS_STAG_DEI_BIT|INGRESS_STAGIF_BIT| \
INGRESS_IPV6_SIP_RANGE_BIT|INGRESS_IPV6_DIP_RANGE_BIT| \
INGRESS_STREAM_ID_BIT|INGRESS_TOS_BIT|INGRESS_IPV6_TC_BIT|INGRESS_L4_NONE_TCP_NONE_UDP_BIT)))


//OringialACL actions
#define HW_ACL_SUPPORT_ACTION_BITS \
(ACL_ACTION_1P_REMARKING_BIT|ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT|ACL_ACTION_DSCP_REMARKING_BIT|ACL_ACTION_QUEUE_ID_BIT| \
ACL_ACTION_SHARE_METER_BIT| ACL_ACTION_ACL_PRIORITY_BIT|ACL_ACTION_ACL_INGRESS_VID_BIT|ACL_ACTION_REDIRECT_BIT|ACL_ACTION_LOG_COUNTER_BIT)

//Oringial CF actions, shoule be supported by FB in apolloPro
#define HW_ACL_NOT_SUPPORT_ACTION_BITS \
(ACL_ACTION_ACL_CVLANTAG_BIT|ACL_ACTION_ACL_SVLANTAG_BIT|ACL_ACTION_DS_UNIMASK_BIT|ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT|ACL_ACTION_STREAM_ID_OR_LLID_BIT)

#define MAX_ACL_CA_CLS_RULE_SIZE (128*4)	//this value is define in classifier.c: CLS_RULE_MAX=L3_CLS_KEY_TBL_ENTRY_MAX*4
#define MAX_ACL_SPECIAL_PACKET_RULE_SIZE CA_PKT_TYPE_MAX

#define CA_L3_CLS_PROFILE_LAN (1<<AAL_LPORT_ETH_NI0)
#define CA_L3_CLS_PROFILE_WAN (1<<AAL_LPORT_ETH_NI7)
//get the port index for trap to cpu, it is physical port, ext port
#define CA_SCAN_ALL_PORT(port)	for(port = RTK_RG_PORT0; port <= RTK_RG_PORT_LASTCPU; port++)  if((port != RTK_RG_MAC_PORT_CPU) && (((0x1<<port) & RTK_RG_ALL_MAC_PORTMASK) != 0x0))
//get the index from portmask, it maybe physical port, cpu port, ext port
#define CA_SCAN_PORTMASK(port, portmask)	for(port = RTK_RG_PORT0; port <= RTK_RG_PORT_LASTCPU; port++)  if(((0x1<<port) & portmask) != 0x0)

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
	USED_TABLE_END = 12,
} rtk_rg_aclSWEntry_used_tables_index_t;

typedef enum rtk_rg_aclSWEntry_icmp_trap_disable_status_s
{
	RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_ENABLE = 0,
	RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_IPV4_DISABLE,
	RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_IPV6_DISABLE,
	RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_DISABLE,
	RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_END,
} rtk_rg_aclSWEntry_icmp_trap_disable_status_t;


typedef enum rtk_acl_field_ca_type_e
{
    ACL_FIELD_CA_UNUSED = 0,
    ACL_FIELD_MAC_SA,				//CLS_KEY_MSK_MAC_SA, mac_sa
    ACL_FIELD_MAC_DA,				//CLS_KEY_MSK_MAC_DA, mac_da
    ACL_FIELD_IP_PROTOCOL,			//CLS_KEY_MSK_IP_PROTOCOL, ip_protocol
    ACL_FIELD_IPV4_SA,				//CLS_KEY_MSK_IPV4_SA, ip_sa
    ACL_FIELD_IPV4_DA,				//CLS_KEY_MSK_IPV4_DA, ip_da
    ACL_FIELD_IPV6_SA,				//CLS_KEY_MSK_IPV6_SA, ip_sa
    ACL_FIELD_IPV6_DA,				//CLS_KEY_MSK_IPV6_DA, ip_da
    ACL_FIELD_IP_VER,				//CLS_KEY_MSK_IP_VER
    ACL_FIELD_ETHERTYPE_ENC,		//CLS_KEY_MSK_ETHERTYPE_ENC, ethertype
    ACL_FIELD_L4_PORT,				//CLS_KEY_MSK_L4_PORT, src_port/dst_port
    ACL_FIELD_L4_VLD,				//CLS_KEY_MSK_L4_VLD
    ACL_FIELD_DSCP,					//CLS_KEY_MSK_DSCP, dscp
	ACL_FIELD_VLAN_CNT,				//CLS_KEY_MSK_VLAN_CNT, vlan_count
	ACL_FIELD_TOP_VL_802_1P,		//CLS_KEY_MSK_TOP_VL_802_1P, pri
	ACL_FIELD_TOP_VID,				//CLS_KEY_MSK_TOP_VID, vid
	ACL_FIELD_TOP_DEI,				//CLS_KEY_MSK_TOP_DEI, cfi or dei
	ACL_FIELD_IPV6_FLOW_LBL,		//CLS_KEY_MSK_IPV6_FLOW_LBL, flow_label

    ACL_FIELD_CA_END,
} rtk_acl_field_ca_type_t;

typedef enum rtk_acl_ca_template_s
{
    ACL_TEMPLATE_CL_IF_ID_KEY_MSK = 0,
	ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK,
	ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK,			//CL_IPV4_TUNNEL_ID_KEY_MSK
	ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK,			//CL_IPV4_TUNNEL_ID_KEY_MSK
	ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK,			//CL_IPV6_SHORT_KEY_MSK
	ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK,			//CL_IPV6_SHORT_KEY_MSK
	ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK,
	ACL_TEMPLATE_CL_MCST_MAC_DA_KEY_MSK,			//CL_MCST_KEY_MSK
	ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK,				//CL_MCST_KEY_MSK
	ACL_TEMPLATE_CL_FULL_KEY_MSK,

	ACL_TEMPLATE_CA_END,
} rtk_acl_ca_template_t;


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
	TEMPLATE_RSV_2_7			=23, //not used
	//related to Template[3]: rsv
	TEMPLATE_RSV_3_0			=24, //not used
	TEMPLATE_RSV_3_1			=25, //not used
	TEMPLATE_RSV_3_2			=26, //not used
	TEMPLATE_RSV_3_3			=27, //not used
	TEMPLATE_RSV_3_4			=28, //not used
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

	//related to Template[7]: Must sync to H/W. This template can not be changed.
	TEMPLATE_EGR_IP_RANGE_0_15	=56,
	TEMPLATE_EGR_IP_RANGE_16_31	=57, //used by ACL, do not used by CF
	TEMPLATE_EGR_L4PORT_RANGE_0_15	=58,
	TEMPLATE_EGR_L4PORT_RANGE_16_31 =59,
	TEMPLATE_EGR_DMAC0			=60,
	TEMPLATE_EGR_DMAC1			=61,
	TEMPLATE_EGR_DMAC2			=62,
	TEMPLATE_EGR_IP4_TOS_PROTO_IP6_TC_NH	=63,

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


//G3 implement rsvACL in file rtk_rg_acl_g3.c / rtk_rg_acl_g3.h
typedef enum rtk_rg_aclAndCf_reserved_type_e
{
	/*CLS Rule for G3*/
	RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE=0,
	RTK_CA_CLS_TYPE_IPV4_WITH_OPTION_TRAP,
	RTK_CA_CLS_TYPE_L2_INGRESS_MULTICAST_FORWARD_L2FE,
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
	RTK_CA_CLS_TYPE_L2_UPSTREAM_LOOPBACK_INGRESS_FORWARD_L3FE, //upstream from loopback port => forward to L3FE
	RTK_CA_CLS_TYPE_L2_UPSTREAM_LAN_INGRESS_FORWARD_LOOPBACK, //upstream from loopback port => forward to Loopback port
	RTK_CA_CLS_TYPE_L2_INGRESS_SA_HOSTPOLICING,
	RTK_CA_CLS_TYPE_L2_EGRESS_DA_HOSTPOLICING,
#endif
	RTK_CA_CLS_TYPE_REARRANGE_PROTECTION,

	/*HEAD PATCH: The lower index number, the higher acl&cf priority*/

	RTK_RG_ACLANDCF_RESERVED_ALL_TRAP,
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
	RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY,
	RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_TRAP_OR_ASSIGN_PRIORITY,
	RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP_OR_ASSIGN_PRIORITY,
	RTK_RG_ACLANDCF_RESERVED_IGMP_MLD_DROP,
	RTK_RG_ACLANDCF_RESERVED_MULTICAST_L2FE_FLOODING,

	RTK_RG_ACLANDCF_RESERVED_HEAD_END,

	/*TAIL PATCH: The lower index number, the higher acl&cf priority*/
	RTK_RG_ACLANDCF_RESERVED_MULTICAST_PROTOCOL_TRAP,
	RTK_RG_ACLANDCF_RESERVED_MULTICAST_IPV4_FORWARD_L2FE,
	RTK_RG_ACLANDCF_RESERVED_MULTICAST_IPV6_FORWARD_L2FE,
	RTK_RG_ACLANDCF_RESERVED_MULTICAST_DSLITE_TRAP,
	RTK_RG_ACLANDCF_RESERVED_L2TP_DATA_LOWER_PRIORITY,
	RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP,
	RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT,
	RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER,
	RTK_RG_ACLANDCF_RESERVED_ARP_PACKET_ASSIGN_SHARE_METER,
	RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_ASSIGN_SHARE_METER,
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

typedef struct rtk_rg_aclAndCf_reserved_syn_packet_assign_share_meter_s{
	int share_meter;
}rtk_rg_aclAndCf_reserved_syn_packet_assign_share_meter_t;

typedef struct rtk_rg_aclAndCf_reserved_arp_packet_assign_share_meter_s{
	uint32 portmask;
	int share_meter;
}rtk_rg_aclAndCf_reserved_arp_packet_assign_share_meter_t;

typedef struct rtk_rg_aclAndCf_reserved_intf_ipv4_fragment_trap_s{
	rtk_mac_t gmac;
}rtk_rg_aclAndCf_reserved_intf_ipv4_fragment_trap_t;

typedef struct rtk_rg_aclAndCf_reserved_ipv4_with_option_trap_s{
	uint16 src_port;
	uint8 dst_mac[ETHER_ADDR_LEN];
	uint32 cls_index;
}rtk_rg_aclAndCf_reserved_ipv4_with_option_trap_t;

#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
typedef struct rtk_rg_aclAndCf_reserved_hostPoliceSaDa_s{
	int16 ca_cls_index; //stored L2 CLS index (ca sw index) index for RX host policing
	int16 flow_id;
	rtk_mac_t hostMac;
}rtk_rg_aclAndCf_reserved_hostPoliceSaDa_t;
#endif

typedef struct rtk_rg_aclAndCf_reserved_global_s
{
	uint32 aclLowerBoundary; //The End index of acl reserve rules in the head
	uint32 aclUpperBoundary; //The start index of acl reserve rules in the tail
	uint32 cfLowerBoundary;  //The start index of cf reserve rules in the head
	uint32 cfUpperBoundary;  //The start index of cf reserve rules in the tail
	uint8 reservedMask[RTK_RG_ACLANDCF_RESERVED_TAIL_END];
	//rtk_rg_aclAndCf_reserved_type_t reservedMask;
	uint32 acl_wanPortMask;

	rtk_rg_aclAndCf_reserved_l2tp_control_lcp_trap_and_assign_priority_t l2tp_ctrl_lcp_assign_prioity;
	rtk_rg_aclAndCf_reserved_pppoe_lcp_assign_priority_t pppoe_lcp_assign_prioity;
	rtk_rg_aclAndCf_reserved_ack_packet_assign_priority_t ack_packet_assign_priority;
	rtk_rg_aclAndCf_reserved_ack_packet_trap_or_assign_priority_t ack_packet_trap_or_assign_priority;
	rtk_rg_aclAndCf_reserved_igmp_mld_drop_t igmp_mld_drop_portmask;
	rtk_rg_aclAndCf_reserved_intf_ipv4_fragment_trap_t intf_ipv4_trap[15];
	rtk_rg_aclAndCf_reserved_syn_packet_assign_share_meter_t syn_packet_assign_share_meter;
	rtk_rg_aclAndCf_reserved_arp_packet_assign_share_meter_t arp_packet_assign_share_meter;
	rtk_rg_aclAndCf_reserved_ipv4_with_option_trap_t ipv4_with_option_trap;

}rtk_rg_acl_reserved_global_t;

typedef enum rtk_rg_g3_caRulePriority_s
{
	RTK_CA_CLS_PRIORITY_INIT = -1,
	RTK_CA_CLS_PRIORITY_L2_0 = 0,
	RTK_CA_CLS_PRIORITY_L2_TAIL = 2,
	RTK_CA_CLS_PRIORITY_L2_3 = 3,
	RTK_CA_CLS_PRIORITY_L2_DOS = 4,			//this is reserve to DOS attack
	RTK_CA_CLS_PRIORITY_L2_STORM = 5,		//this is reserve to STORM control
	RTK_CA_CLS_PRIORITY_L2_HEAD = 6,
	RTK_CA_CLS_PRIORITY_L2_7 = 7,
	RTK_CA_CLS_PRIORITY_L3_TAIL = 10,		//better to higher than CL_RUL_PRIO_L3_INTF_MCAST 9
	RTK_CA_CLS_PRIORITY_L3_USER = 12,
	RTK_CA_CLS_PRIORITY_L3_HEAD = 14,
	RTK_CA_CLS_PRIORITY_L3_15 = 15,
	RTK_CA_CLS_PRIORITY_MAX,			//group ca rules by ca priority(range L2 CLS 0~7, L3 CLS 0~15). We will rearrange priority by ourself, L2 0~7 for rsv ACL, L3 8~15 for rsv ACL/User.
} rtk_rg_g3_caRulePriority_t;

typedef struct rtk_rg_g3_cls_entry_s
{
	rtk_enable_t				valid;
 	int							ruleType;	//Reserve_ACL: 0~RESERVED_TAIL_END, user SW_ACL[i]=RESERVED_TAIL_END+i
	rtk_rg_g3_caRulePriority_t	priority;
	unsigned int				aal_customize;
} rtk_rg_g3_cls_entry_t;

typedef struct rtk_rg_g3_special_pkt_rule_s
{
    ca_port_id_t     			source_port;
    ca_pkt_type_t 				special_packet;
    ca_uint8_t       			priority;
} rtk_rg_g3_special_pkt_rule_t;

typedef struct rtk_rg_g3_special_pkt_entry_s
{
	rtk_enable_t				valid;
 	int							ruleType;	//Reserve_ACL: 0~RESERVED_TAIL_END
} rtk_rg_g3_special_pkt_entry_t;

#endif //end of #ifndef RTK_RG_ACL_APOLLOFE_H

