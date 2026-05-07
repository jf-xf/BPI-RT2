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
 *
 * $Revision: 68395 $
 * $Date: 2016-05-27 16:38:35 +0800 (Fri, 27 May 2016) $
 *
 * Purpose : Definition of ACL extension API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Reset/display/configure acl rule structure and then add this rule.
 *           (2) Get/delete acl rule by rule index.
 *
 */


#ifndef __RT_ACL_EXT_H__
#define __RT_ACL_EXT_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rt_port_ext.h>	//rt_port_phy_port_mask_t
#include <rt_wlan_ext.h>	//rt_wlan_mbssid_mask_t

/*
 * Symbol Definition
 */

/* For Filter Fields Index */
typedef enum rt_acl_filter_fields_index_e
{	/*ACL filter fields index for internal use*/
	RT_ACL_INGRESS_PORT_MASK_INDEX = 0,
	RT_ACL_CARE_PORT_UNITYPE_PPTP_INDEX,
	RT_ACL_INGRESS_WLAN_MBSSID_MASK_INDEX,
	RT_ACL_INGRESS_SMAC_INDEX,
	RT_ACL_INGRESS_DMAC_INDEX,
	RT_ACL_INGRESS_ETHERTYPE_INDEX,
	RT_ACL_INGRESS_STAGIF_INDEX,
	RT_ACL_INGRESS_STAG_VID_INDEX,
	RT_ACL_INGRESS_STAG_PRI_INDEX,
	RT_ACL_INGRESS_STAG_DEI_INDEX,
	RT_ACL_INGRESS_CTAGIF_INDEX,
	RT_ACL_INGRESS_CTAG_VID_INDEX,
	RT_ACL_INGRESS_CTAG_PRI_INDEX,
	RT_ACL_INGRESS_CTAG_CFI_INDEX,
	RT_ACL_INGRESS_IPV4_TAGIF_INDEX,
	RT_ACL_INGRESS_IPV4_SIP_RANGE_INDEX,
	RT_ACL_INGRESS_IPV4_DIP_RANGE_INDEX,
	RT_ACL_INGRESS_IPV4_DSCP_INDEX,
	RT_ACL_INGRESS_IPV4_TOS_INDEX,
	RT_ACL_INGRESS_IPV6_TAGIF_INDEX,
	RT_ACL_INGRESS_IPV6_SIP_INDEX,
	RT_ACL_INGRESS_IPV6_DIP_INDEX,
	RT_ACL_INGRESS_IPV6_DSCP_INDEX,
	RT_ACL_INGRESS_IPV6_TC_INDEX,
	RT_ACL_INGRESS_L4_POROTCAL_VALUE_INDEX,
	RT_ACL_INGRESS_L4_TCP_INDEX,
	RT_ACL_INGRESS_L4_UDP_INDEX,
	RT_ACL_INGRESS_L4_ICMP_INDEX,
	RT_ACL_INGRESS_L4_ICMPV6_INDEX,
	RT_ACL_INGRESS_L4_SPORT_RANGE_INDEX,
	RT_ACL_INGRESS_L4_DPORT_RANGE_INDEX,
	RT_ACL_INGRESS_STREAM_ID_INDEX,
	RT_ACL_INGRESS_TCP_FLAGS_INDEX,
	RT_ACL_INGRESS_PKT_LEN_RANGE_INDEX,
	RT_ACL_INGRESS_ARP_TARGET_IP_INDEX,
	RT_ACL_VXLAN_RELATED_ACL,

} rt_acl_filter_fields_index_t;

/* For Filter Field Mask */
typedef enum rt_acl_filter_fields_e
{	/*ACL filter fields mask per bit definition*/
	RT_ACL_INGRESS_PORT_MASK_BIT = (1ULL<<RT_ACL_INGRESS_PORT_MASK_INDEX),
	RT_ACL_CARE_PORT_UNITYPE_PPTP_BIT = (1ULL<<RT_ACL_CARE_PORT_UNITYPE_PPTP_INDEX),
	RT_ACL_INGRESS_WLAN_MBSSID_MASK_BIT = (1ULL<<RT_ACL_INGRESS_WLAN_MBSSID_MASK_INDEX),
	RT_ACL_INGRESS_SMAC_BIT = (1ULL<<RT_ACL_INGRESS_SMAC_INDEX),
	RT_ACL_INGRESS_DMAC_BIT = (1ULL<<RT_ACL_INGRESS_DMAC_INDEX),
	RT_ACL_INGRESS_ETHERTYPE_BIT = (1ULL<<RT_ACL_INGRESS_ETHERTYPE_INDEX),
	RT_ACL_INGRESS_STAGIF_BIT = (1ULL<<RT_ACL_INGRESS_STAGIF_INDEX),
	RT_ACL_INGRESS_STAG_VID_BIT = (1ULL<<RT_ACL_INGRESS_STAG_VID_INDEX),
	RT_ACL_INGRESS_STAG_PRI_BIT = (1ULL<<RT_ACL_INGRESS_STAG_PRI_INDEX),
	RT_ACL_INGRESS_STAG_DEI_BIT = (1ULL<<RT_ACL_INGRESS_STAG_DEI_INDEX),
	RT_ACL_INGRESS_CTAGIF_BIT = (1ULL<<RT_ACL_INGRESS_CTAGIF_INDEX),
	RT_ACL_INGRESS_CTAG_VID_BIT = (1ULL<<RT_ACL_INGRESS_CTAG_VID_INDEX),
	RT_ACL_INGRESS_CTAG_PRI_BIT = (1ULL<<RT_ACL_INGRESS_CTAG_PRI_INDEX),
	RT_ACL_INGRESS_CTAG_CFI_BIT = (1ULL<<RT_ACL_INGRESS_CTAG_CFI_INDEX),
	RT_ACL_INGRESS_IPV4_TAGIF_BIT = (1ULL<<RT_ACL_INGRESS_IPV4_TAGIF_INDEX),
	RT_ACL_INGRESS_IPV4_SIP_RANGE_BIT = (1ULL<<RT_ACL_INGRESS_IPV4_SIP_RANGE_INDEX),
	RT_ACL_INGRESS_IPV4_DIP_RANGE_BIT = (1ULL<<RT_ACL_INGRESS_IPV4_DIP_RANGE_INDEX),
	RT_ACL_INGRESS_IPV4_DSCP_BIT = (1ULL<<RT_ACL_INGRESS_IPV4_DSCP_INDEX),
	RT_ACL_INGRESS_IPV4_TOS_BIT = (1ULL<<RT_ACL_INGRESS_IPV4_TOS_INDEX),
	RT_ACL_INGRESS_IPV6_TAGIF_BIT = (1ULL<<RT_ACL_INGRESS_IPV6_TAGIF_INDEX),
	RT_ACL_INGRESS_IPV6_SIP_BIT = (1ULL<<RT_ACL_INGRESS_IPV6_SIP_INDEX),
	RT_ACL_INGRESS_IPV6_DIP_BIT = (1ULL<<RT_ACL_INGRESS_IPV6_DIP_INDEX),
	RT_ACL_INGRESS_IPV6_DSCP_BIT = (1ULL<<RT_ACL_INGRESS_IPV6_DSCP_INDEX),
	RT_ACL_INGRESS_IPV6_TC_BIT = (1ULL<<RT_ACL_INGRESS_IPV6_TC_INDEX),
	RT_ACL_INGRESS_L4_POROTCAL_VALUE_BIT = (1ULL<<RT_ACL_INGRESS_L4_POROTCAL_VALUE_INDEX),
	RT_ACL_INGRESS_L4_TCP_BIT = (1ULL<<RT_ACL_INGRESS_L4_TCP_INDEX),
	RT_ACL_INGRESS_L4_UDP_BIT = (1ULL<<RT_ACL_INGRESS_L4_UDP_INDEX),
	RT_ACL_INGRESS_L4_ICMP_BIT = (1ULL<<RT_ACL_INGRESS_L4_ICMP_INDEX),
	RT_ACL_INGRESS_L4_ICMPV6_BIT = (1ULL<<RT_ACL_INGRESS_L4_ICMPV6_INDEX),
	RT_ACL_INGRESS_L4_SPORT_RANGE_BIT = (1ULL<<RT_ACL_INGRESS_L4_SPORT_RANGE_INDEX),
	RT_ACL_INGRESS_L4_DPORT_RANGE_BIT = (1ULL<<RT_ACL_INGRESS_L4_DPORT_RANGE_INDEX),
	RT_ACL_INGRESS_STREAM_ID_BIT = (1ULL<<RT_ACL_INGRESS_STREAM_ID_INDEX),
	RT_ACL_INGRESS_TCP_FLAGS_BIT = (1ULL<<RT_ACL_INGRESS_TCP_FLAGS_INDEX),
	RT_ACL_INGRESS_PKT_LEN_RANGE_BIT = (1ULL<<RT_ACL_INGRESS_PKT_LEN_RANGE_INDEX),
	RT_ACL_INGRESS_ARP_TARGET_IP_BIT = (1ULL<<RT_ACL_INGRESS_ARP_TARGET_IP_INDEX),
	RT_ACL_VXLAN_RELATED_ACL_BIT = (1ULL<<RT_ACL_VXLAN_RELATED_ACL),
} rt_acl_filter_fields_t;

/* For Ingress TCP Flags Definition */
typedef enum rt_acl_ingress_tcp_flag_e
{	/* ACL TCP Flags definition */
	RT_ACL_INGRESS_TCP_FLAG_FIN = (0x1<<0),
	RT_ACL_INGRESS_TCP_FLAG_SYN = (0x1<<1),
	RT_ACL_INGRESS_TCP_FLAG_RESET = (0x1<<2),
	RT_ACL_INGRESS_TCP_FLAG_PUSH = (0x1<<3),
	RT_ACL_INGRESS_TCP_FLAG_ACK = (0x1<<4),
	RT_ACL_INGRESS_TCP_FLAG_URG = (0x1<<5),
	RT_ACL_INGRESS_TCP_FLAG_ECN = (0x1<<6),
	RT_ACL_INGRESS_TCP_FLAG_CWR = (0x1<<7),
	RT_ACL_INGRESS_TCP_FLAG_NS = (0x1<<8),
	RT_ACL_INGRESS_TCP_FLAG_MAX,
} rt_acl_ingress_tcp_flag_t;

/* For Action Fields Index */
typedef enum rt_acl_action_fields_index_e
{	/*ACL action fields index for internal use*/
	RT_ACL_ACTION_FORWARD_GROUP_DROP_INDEX = 0,
	RT_ACL_ACTION_FORWARD_GROUP_TRAP_INDEX,
	RT_ACL_ACTION_FORWARD_GROUP_TRAP_SLAVECPU_INDEX,		// 9607C only
	RT_ACL_ACTION_FORWARD_GROUP_TRAP_TO_PS_INDEX,		// 8277 series only
	RT_ACL_ACTION_FORWARD_GROUP_PERMIT_INDEX,
	RT_ACL_ACTION_FORWARD_GROUP_REDIRECT_INDEX,
	RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_INDEX,
	RT_ACL_ACTION_PRIORITY_GROUP_ACL_PRIORITY_INDEX,
	RT_ACL_ACTION_METER_GROUP_SHARE_METER_INDEX,
	RT_ACL_ACTION_LOGGING_GROUP_MIB_INDEX,
	RT_ACL_ACTION_FIELDS_INDEX_MAX,
} rt_acl_action_fields_index_t;

/* For Action Field Mask */
typedef enum rt_acl_action_fields_e
{	/*ACL action fields mask per bit definition*/
	RT_ACL_ACTION_FORWARD_GROUP_DROP_BIT = (0x1<<RT_ACL_ACTION_FORWARD_GROUP_DROP_INDEX),
	RT_ACL_ACTION_FORWARD_GROUP_TRAP_BIT = (0x1<<RT_ACL_ACTION_FORWARD_GROUP_TRAP_INDEX),
	RT_ACL_ACTION_FORWARD_GROUP_TRAP_SLAVECPU_BIT = (0x1<<RT_ACL_ACTION_FORWARD_GROUP_TRAP_SLAVECPU_INDEX),	// 9607C only
	RT_ACL_ACTION_FORWARD_GROUP_TRAP_TO_PS_BIT = (0x1<<RT_ACL_ACTION_FORWARD_GROUP_TRAP_TO_PS_INDEX), // 8277 series only
	RT_ACL_ACTION_FORWARD_GROUP_PERMIT_BIT = (0x1<<RT_ACL_ACTION_FORWARD_GROUP_PERMIT_INDEX),
	RT_ACL_ACTION_FORWARD_GROUP_REDIRECT_BIT = (0x1<<RT_ACL_ACTION_FORWARD_GROUP_REDIRECT_INDEX),
	RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT = (0x1<<RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_INDEX),
	RT_ACL_ACTION_PRIORITY_GROUP_ACL_PRIORITY_BIT = (0x1<<RT_ACL_ACTION_PRIORITY_GROUP_ACL_PRIORITY_INDEX),
	RT_ACL_ACTION_METER_GROUP_SHARE_METER_BIT = (0x1<<RT_ACL_ACTION_METER_GROUP_SHARE_METER_INDEX),
	RT_ACL_ACTION_LOGGING_GROUP_MIB_BIT = (0x1<<RT_ACL_ACTION_LOGGING_GROUP_MIB_INDEX),
} rt_acl_action_fields_t;

/* For Reserve ACL Type Index */
typedef enum rt_acl_reserved_type_e
{	/*Reserve ACL type definition*/
	RT_ACL_RESERVED_TYPE_ICMPV4_TRAP,
	RT_ACL_RESERVED_TYPE_ICMPV6_TRAP,
	RT_ACL_RESERVED_TYPE_MAX,
} rt_acl_reserved_type_t;

/*
 * Macro Declaration
 */
#define rt_acl_port_mask_t       rt_port_phy_port_mask_t
#define rt_acl_wlan_mbssid_mask_t       rt_wlan_mbssid_mask_t

/*
 * Data Declaration
 */
/* acl rule structure */
typedef struct rt_acl_filterAndQos_s
{
	int acl_weight;									/*ACL internal sorting, the larger number, the higher priority, the lower hw index.*/

	unsigned long long int filter_fields;			/*pattern filter mask, please refer to rt_acl_filter_fields_t typedef.*/

	rt_acl_port_mask_t ingress_port_mask;			/*Per port enable/disable ACL lookup, ex: port_mask=0x6 means filter ingress port from port 1 or port 2*/

	/* filter pattern & mask */
	rt_acl_wlan_mbssid_mask_t ingress_wlan_mbssid_mask[RT_WLAN_DEVICE_MAX];	/*wlan mbssid mask, ex: mbssid[1]=0x3 means filter from wlan1 root or wlan1 vap0...*/

	uint8 ingress_smac[ETHER_ADDR_LEN];				/*source mac*/
	uint8 ingress_smac_mask[ETHER_ADDR_LEN];		/*source mac mask, ex: smac=01:00:5e:00:00:00, mask=ff:ff:ff:00:00:00, filter smac=01:00:5e:00:00:00 to 01:00:5e:ff:ff:ff*/
	uint8 ingress_dmac[ETHER_ADDR_LEN];				/*destination mac*/
	uint8 ingress_dmac_mask[ETHER_ADDR_LEN];		/*destination mac mask, ex: dmac=01:80:c2:00:00:00, mask=ff:ff:ff:ff:ff:00, filter dmac=01:80:c2:00:00:00 to 01:80:c2:00:00:ff*/

	uint16 ingress_ethertype;						/*ethertype*/
	uint16 ingress_ethertype_mask;					/*ethertype mask, ex: ethertype=0x8863, mask=0xfff8, filter ethertype=0x8860 to 0x8867*/
	uint8 ingress_stagif;							/*0:must not include svlan, 1:must include svlan*/
	uint16 ingress_stag_vid;						/*svlan id*/
	uint16 ingress_stag_pri;						/*svlan 802.1p, PCP*/
	uint8 ingress_stag_dei;							/*svlan dei*/
	uint8 ingress_ctagif;							/*0:must not include cvlan, 1:must include cvlan*/
	uint16 ingress_ctag_vid;						/*cvlan id*/
	uint16 ingress_ctag_pri;						/*cvlan 802.1p, PCP*/
	uint8 ingress_ctag_cfi;							/*cvlan cfi*/
	
	uint8 ingress_ipv4_tagif;						/*0:must not be ipv4, 1:must be ipv4*/
	ipaddr_t ingress_src_ipv4_addr_start;			/*ipv4 source address range lower bound*/
	ipaddr_t ingress_src_ipv4_addr_end;				/*ipv4 source address range upper bound*/
	ipaddr_t ingress_dest_ipv4_addr_start;			/*ipv4 destination address range lower bound*/
	ipaddr_t ingress_dest_ipv4_addr_end;			/*ipv4 destination address range upper bound*/
	uint16 ingress_ipv4_dscp;						/*ipv4 dscp*/
	uint16 ingress_ipv4_tos;						/*ipv4 tos*/

	uint8 ingress_ipv6_tagif;							/*0:must not be ipv6, 1:must be ipv6*/
	uint8 ingress_src_ipv6_addr[IPV6_ADDR_LEN];			/*ipv6 source address*/
	uint8 ingress_src_ipv6_addr_mask[IPV6_ADDR_LEN];	/*ipv6 source address mask*/
	uint8 ingress_dest_ipv6_addr[IPV6_ADDR_LEN];		/*ipv6 destination address*/
	uint8 ingress_dest_ipv6_addr_mask[IPV6_ADDR_LEN];	/*ipv6 source destination mask*/
	uint16 ingress_ipv6_dscp;							/*ipv6 dscp*/
	uint16 ingress_ipv6_tc;								/*ipv6 tos*/

	uint16 ingress_l4_protocal;						/*layer 4 protocol, ex: 0x6 for tcp, 0x11 for udp*/
	uint16 ingress_src_l4_port_start;				/*layer 4 source port range lower bound*/
	uint16 ingress_src_l4_port_end;					/*layer 4 source port range upper bound*/
	uint16 ingress_dest_l4_port_start;				/*layer 4 destination port range lower bound*/
	uint16 ingress_dest_l4_port_end;				/*layer 4 destination port range upper bound*/

	uint16 ingress_stream_id;							/*pon stream id*/
	uint16 ingress_stream_id_mask;						/*pon stream id mask*/
	rt_acl_ingress_tcp_flag_t ingress_tcp_flags;		/*tcp flags*/
	rt_acl_ingress_tcp_flag_t ingress_tcp_flags_mask;	/*tcp flags mask, ex: tcp flags=0x2, mask=0x1ff, filter tcp syn only*/
	uint16 ingress_packet_length_start;					/*packet length range lower bound*/
	uint16 ingress_packet_length_end;					/*packet length range upper bound*/

	ipaddr_t ingress_arp_target_ip;					/*arp target ip, rule must set stagif or ctagif according to packet format due to system only support one l2 format!*/
	
	/* action & option */
	rt_acl_action_fields_t action_fields;			/*action mask, each group only support one action bit*/

	uint16 action_forward_group_redirect_port_idx;	/*option of RT_ACL_ACTION_FORWARD_GROUP_REDIRECT_BIT*/
	uint16 action_priority_group_trap_priority;		/*option of RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT*/
	uint16 action_priority_group_acl_priority;		/*option of RT_ACL_ACTION_PRIORITY_GROUP_ACL_PRIORITY_BIT*/
	uint32 action_meter_group_share_meter_index;	/*option of RT_ACL_ACTION_METER_GROUP_SHARE_METER_BIT*/
	uint16 action_logging_group_mib_index;			/*option of RT_ACL_ACTION_LOGGING_GROUP_MIB_BIT*/
} rt_acl_filterAndQos_t;


/*
 * Function Declaration
 */

/* Function Name:
 *      rt_acl_filterAndQos_add
 * Description:
 *      Add acl rule 
 * Input:
 *      acl_filter        - a acl rule include the patterns which need to be filtered, and related action
 * Output:
 *      acl_filter_idx        - the index of the added acl rule
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *      RT_ERR_NOT_ALLOWED      - Driver return fail
 * Note:
 *      The API can add acl filter and qos remarking rule
 */
extern int32
rt_acl_filterAndQos_add (
    rt_acl_filterAndQos_t acl_filter, uint32 *acl_filter_idx );

/* Function Name:
 *      rt_acl_filterAndQos_del
 * Description:
 *      Delete acl filter rule 
 * Input:
 *      acl_filter_idx        - the index of the acl rule which need to be delete
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *      RT_ERR_NOT_ALLOWED      - Driver return fail
 * Note:
 *      The API can delete acl rule
 */
extern int32
rt_acl_filterAndQos_del (
    uint32 acl_filter_idx );

/* Function Name:
 *      rt_acl_filterAndQos_get
 * Description:
 *      Get acl filter rule 
 * Input:
 *      acl_filter_idx        - the index of the acl rule which need to get
 * Output:
 *      acl_filter        - the acl rule which be found
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *      RT_ERR_NOT_ALLOWED      - Driver return fail
 * Note:
 *      The API can get acl filter rule by rule index
 */
extern int32
rt_acl_filterAndQos_get (
    uint32 acl_filter_idx, rt_acl_filterAndQos_t *acl_filter );

/* Function Name:
 *		rt_acl_reserved_status_get
 * Description:
 *		Get reserved acl status by reserved type 
 * Input:
 *		reserved type		  	- the type of the reserve acl which need to get
 * Output:
 *		status		  			- the reserved acl status which be found
 * Return:
 *		RT_ERR_OK				- OK
 *      RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *      RT_ERR_NOT_ALLOWED      - Driver return fail
 * Note:
 *		The API can get reserved acl status by rule type
 */
extern int32
rt_acl_reserved_status_get (
	rt_acl_reserved_type_t reserved_type, uint32 *status );

/* Function Name:
 *		rt_acl_reserved_status_set
 * Description:
 *		Set reserved acl status by reserved type 
 * Input:
 *		reserved type			- the type of the reserve acl which need to set
 * Output:
 *		status					- the reserved acl status should be update
 * Return:
 *		RT_ERR_OK				- OK
 *      RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *      RT_ERR_NOT_ALLOWED      - Driver return fail
 * Note:
 *		The API can update reserved acl status by rule type
 */
extern int32
rt_acl_reserved_status_set (
	rt_acl_reserved_type_t reserved_type, uint32 status );

#endif /* __RT_ACL_EXT_H__ */
