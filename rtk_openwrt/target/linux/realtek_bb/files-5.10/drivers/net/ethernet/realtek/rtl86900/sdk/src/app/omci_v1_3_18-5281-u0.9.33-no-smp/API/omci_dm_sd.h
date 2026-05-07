/*
 * Copyright (C) 2014 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition of Dual Management shared define
 */

#ifndef __OMCI_DM_SD_H__
#define __OMCI_DM_SD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "rtk/ponmac.h"
#include "omci_dm_mcast.h"

#define WAN_PONMAC_QUEUE_MAX        (8)
#define OMCI_IPV6_ADDR_LEN          16
#define OMCI_USERNAME_PASSWORD_LEN  25
#define OMCI_ACS_URL_LEN            375

typedef enum {
     OMCI_MODE_PPPOE = 0,
     OMCI_MODE_IPOE,
     OMCI_MODE_BRIDGE,
     OMCI_MODE_PPPOE_V4NAPT_V6,
     OMCI_MODE_IPOE_V4NAPT_V6,
} OMCI_MODE_T;

typedef enum patch_type_e
{
    PATCH_DS_9602C_A,   //DS = dual stack
    PATCH_DPI,
    PATCH_END
}patch_type_t;

typedef enum omci_cf_rule_rg_opt_e
{
    OMCI_CF_RULE_RG_OPT_ACL,
    OMCI_CF_RULE_RG_OPT_CF,
    OMCI_CF_RULE_RG_OPT_END
}omci_cf_rule_rg_opt_t;

typedef enum mgmt_cfg_op_e
{
    OP_RESET_ALL,
    OP_SET_IF,
    OP_RESET_ACS,
    OP_SET_ACS,
    OP_RESET_IF,
    OP_SET_PPPOE
}mgmt_cfg_op_t;

typedef enum if_service_e
{
    IF_SERVICE_DATA         = (1 << 0),
    IF_SERVICE_TR69         = (1 << 1),
    IF_SERVICE_SIP          = (1 << 2),
    IF_SERVICE_ALL          = (IF_SERVICE_DATA | IF_SERVICE_TR69 | IF_SERVICE_SIP)
}if_service_t;

typedef enum if_channel_mode_e
{
    IF_CHANNEL_MODE_BRIDGED,
    IF_CHANNEL_MODE_IPOE,
    IF_CHANNEL_MODE_PPPOE,
    IF_CHANNEL_MODE_END
}if_channel_mode_t;

typedef unsigned int omci_ipv4_addr_t;

typedef struct omci_ipv6_addr_s
{
    unsigned char ipv6_addr[OMCI_IPV6_ADDR_LEN];
} omci_ipv6_addr_t;

typedef struct if_info_s
{
    unsigned int if_id;
    unsigned int if_is_ipv6_B;
    unsigned int if_is_DHCP_B;
    unsigned int if_is_ip_stack_B;
    if_service_t if_service_type;
    unsigned short if_tci;

    union
    {
        omci_ipv4_addr_t ipv4_addr;
        omci_ipv6_addr_t ipv6_ddr;
    } ip_addr;

    union
    {
        omci_ipv4_addr_t ipv4_mask_addr;
        omci_ipv6_addr_t ipv6_mask_addr;
    } mask_addr;

    union
    {
        omci_ipv4_addr_t ipv4_gateway_addr;
        omci_ipv6_addr_t ipv6_gateway_addr;
    } gateway_addr;

    union
    {
        omci_ipv4_addr_t ipv4_primary_dns_addr;
        omci_ipv6_addr_t ipv6_primary_dns_addr;
    } primary_dns_addr;

    union
    {
        omci_ipv4_addr_t ipv4_second_dns_addr;
        omci_ipv6_addr_t ipv6_second_dns_addr;
    } second_dns_addr;

    if_channel_mode_t   wan_type;               //ignore dhcpB and ip statk should be set 0x8 if wan_type is pppoe
    unsigned char       if_mac_addr[6];
}if_info_t;

typedef struct acs_info_s
{
    unsigned int    related_if_id;
    unsigned char   acs_url[OMCI_ACS_URL_LEN];
    unsigned char   username[OMCI_USERNAME_PASSWORD_LEN];
    unsigned char   password[OMCI_USERNAME_PASSWORD_LEN];
}acs_info_t;

typedef struct pppoe_info_s
{
    unsigned int    related_if_id;
    unsigned int    nat_enabled;
    unsigned int    auth_method;
    unsigned int    conn_type;
    unsigned int    release_time;
    char   username[OMCI_USERNAME_PASSWORD_LEN+1];
    char   password[OMCI_USERNAME_PASSWORD_LEN+1];
}pppoe_info_t;

typedef struct mgmt_cfg_s
{
    if_info_t       if_entry;
    acs_info_t      acs;
    pppoe_info_t    pppoe;
    //others mgmt service info: sip
}mgmt_cfg_t;

typedef struct mgmt_cfg_msg_s
{
    mgmt_cfg_op_t op_id;
    mgmt_cfg_t cfg;
}mgmt_cfg_msg_t;

typedef enum omci_pon_wan_rule_position_e
{
    OMCI_PON_WAN_RULE_POS_RG_ACL,
    OMCI_PON_WAN_RULE_POS_RG_US_SW_ACL,
    OMCI_PON_WAN_RULE_POS_PATCH_DS_9602C_A,
    OMCI_PON_WAN_RULE_POS_PATCH_DPI,
    OMCI_PON_WAN_RULE_POS_END
}omci_pon_wan_rule_position_t;

typedef enum omci_pon_wan_vlan_tag_action_e
{
	OMCI_PON_WAN_VLAN_TAGIF_TRANSPARENT,
	OMCI_PON_WAN_VLAN_TAGIF_TAGGING,
	OMCI_PON_WAN_VLAN_TAGIF_REMOVE_1ST_TAG,
	OMCI_PON_WAN_VLAN_TAGIF_REMOVE_2ND_TAG,
	OMCI_PON_WAN_VLAN_TAGIF_END,
}omci_pon_wan_vlan_tag_action_t;

typedef enum omci_pon_wan_vlan_tpid_action_e{
	OMCI_PON_WAN_VLAN_TPID_ASSIGN,
	OMCI_PON_WAN_VLAN_TPID_COPY_FROM_1ST_TAG,	    //outer
	OMCI_PON_WAN_VLAN_TPID_COPY_FROM_2ND_TAG,	    //inner
	OMCI_PON_WAN_VLAN_TPID_END,
}omci_pon_wan_vlan_tpid_action_t;

typedef enum omci_pon_wan_vlan_vid_action_e
{
	OMCI_PON_WAN_VLAN_VID_ASSIGN,
	OMCI_PON_WAN_VLAN_VID_COPY_FROM_1ST_TAG,		//outer
	OMCI_PON_WAN_VLAN_VID_COPY_FROM_2ND_TAG,		//inner
	OMCI_PON_WAN_VLAN_VID_END,
}omci_pon_wan_vlan_vid_action_t;

typedef enum omci_pon_wan_vlan_pri_action_e{
	OMCI_PON_WAN_VLAN_PRI_ASSIGN,
	OMCI_PON_WAN_VLAN_PRI_COPY_FROM_1ST_TAG,		//outer
	OMCI_PON_WAN_VLAN_PRI_COPY_FROM_2ND_TAG,		//inner
	OMCI_PON_WAN_VLAN_PRI_COPY_FROM_DSCP_REMAP,
	OMCI_PON_WAN_VLAN_PRI_END,
}omci_pon_wan_vlan_pri_action_t;

typedef enum omci_pon_wan_vlan_dei_action_e{
	OMCI_PON_WAN_VLAN_DEI_ASSIGN,
	OMCI_PON_WAN_VLAN_DEI_COPY_FROM_1ST_TAG,		//outer
	OMCI_PON_WAN_VLAN_DEI_COPY_FROM_2ND_TAG,		//inner
	OMCI_PON_WAN_VLAN_DEI_END,
}omci_pon_wan_vlan_dei_action_t;

typedef struct omci_pon_wan_vlan_action_s
{
	omci_pon_wan_vlan_tag_action_t 		tagAction;
	omci_pon_wan_vlan_tpid_action_t		tagTpidAction;
	omci_pon_wan_vlan_vid_action_t		tagVidAction;
	omci_pon_wan_vlan_pri_action_t		tagPriAction;
	omci_pon_wan_vlan_dei_action_t		tagDeiAction;
	int assignedTpid;
	int assignedVid;
	int assignedPri;
	int assignedDei;
}omci_pon_wan_vlan_action_t;

typedef struct omci_pon_wan_rule_s
{
	int 							wanVid;
    int 							wanPri;
	omci_pon_wan_vlan_action_t 		outerTagAct;
	omci_pon_wan_vlan_action_t 		innerTagAct;
    int 							ponVid;						//the first outer vid format
    int 							ponPri;						//the first outer pri format
	int 							ponTpid;					//the first outer tpid format
	int 							flowId;
} omci_pon_wan_rule_t;

typedef struct
{
    unsigned int            valid;
    omci_cf_rule_rg_opt_t   ruleOpt;
    #if 0
    union
    {
        rtk_rg_aclFilterAndQos_t    aclEntry;
        rtk_rg_classifyEntry_t      cfEntry;
    } rule;                                                         // for 9602C test chip issue: dual stack or others
    #else
    omci_pon_wan_rule_t    rule;
    #endif

} omci_dm_patch_rule_t;

typedef struct
{
	unsigned int				wanIdx;								// zero-based wan idx
	unsigned char				wanType;							// refers to if_smux.h
	#if 0
	rtk_rg_aclFilterAndQos_t	rgAclEntry;							// rg us acl entry
	rtk_rg_aclFilterAndQos_t	rgUsSwAclEntry;						// for bridge wan only
    #else
    omci_pon_wan_rule_t        rgAclEntry;							// rg us acl entry
    omci_pon_wan_rule_t        rgUsSwAclEntry;						// for bridge wan only
    #endif
	omci_dm_patch_rule_t        rgPatchEntry[PATCH_END];            // for patch rule
	unsigned int				usFlowId[WAN_PONMAC_QUEUE_MAX];		// pre-allocate us flow id
	unsigned int				gemPortId;							// pon gem port id
	rtk_ponmac_queueCfg_t		queueCfg[WAN_PONMAC_QUEUE_MAX];		// pon queue cfg
	unsigned char				queueSts[WAN_PONMAC_QUEUE_MAX];		// pon queue disable/enable
	unsigned int				tcontId;							// pon t-cont id
} omci_dm_pon_wan_info_t;

typedef enum omci_dm_mode_mask_e
{
    OMCI_DM_MODE_OFF_BIT            =(0),
    OMCI_DM_MODE_WAN_QOS_BIT     = (1<<0),
    OMCI_DM_MODE_DS_BC_MC_BIT   = (1<<1),
}omci_dm_mode_mask_t;


typedef enum omci_dm_filter_mask_e
{
    OMCI_DM_FILTER_STREAMID_BIT               =(1<<0),
    OMCI_DM_FILTER_OUTER_TAGIf_BIT          =(1<<1),
    OMCI_DM_FILTER_INNER_TAGIf_BIT          =(1<<2),
    OMCI_DM_FILTER_OUTER_VID_BIT             =(1<<3),
    OMCI_DM_FILTER_OUTER_PRI_BIT             =(1<<4),
    OMCI_DM_FILTER_OUTER_DEI_BIT             =(1<<5),
    OMCI_DM_FILTER_INNER_VID_BIT             =(1<<6),
    OMCI_DM_FILTER_INNER_PRI_BIT             =(1<<7),
    OMCI_DM_FILTER_INNER_DEI_BIT             =(1<<8),
    OMCI_DM_FILTER_ETHERTYPE                    =(1<<9),
    OMCI_DM_FILTER_EGRESS_PORT_BIT          =(1<<10),
}omci_dm_filter_mask_t;

typedef struct omci_dm_filter_rule_s
{
    /* filter mask */
    omci_dm_filter_mask_t filterMask;
    /* filter values */
    unsigned int                streamID;
    unsigned int                outerTagIf;
    unsigned int                innerTagIf;
    unsigned int                outerTagVid;
    unsigned int                outerTagPri;
    unsigned int                outerTagDei;
    unsigned int                innerTagVid;
    unsigned int                innerTagPri;
    unsigned int                innerTagDei;
    unsigned int                etherType;
    unsigned int                egressPortMask;
} omci_dm_filter_rule_t;

typedef struct omci_dm_ds_bc_mc_info_s
{
    unsigned int                            relatedId;
    unsigned int                            isMcRule;
    omci_dm_filter_rule_t           filterRule;	
    omci_pon_wan_vlan_action_t  outerTagAct;
    omci_pon_wan_vlan_action_t  innerTagAct;
} omci_dm_ds_bc_mc_info_t;

unsigned int omci_cfg_set (mgmt_cfg_msg_t *pParam, unsigned int len);


#ifdef __cplusplus
}
#endif

#endif
