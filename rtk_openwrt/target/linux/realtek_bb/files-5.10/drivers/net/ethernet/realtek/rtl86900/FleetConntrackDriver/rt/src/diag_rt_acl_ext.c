/*
 * Copyright (C) 2018 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 14393 $
 * $Date: 2018-11-22 11:32:16 +0800 (Thu, 22 Nov 2018) $
 *
 * Purpose : Definition of ACL extension API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Reset/display/configure acl rule structure and then add this rule.
 *           (2) Get/delete acl rule by rule index.
 *
 */


/*
 * Include Files
 */

#include <stdio.h>
#include <string.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <diag_str.h>
#include <parser/cparser_priv.h>

#include <rt_acl_ext.h>

/* variable */
static rt_acl_filterAndQos_t acl_parameter;

/* local function */
static uint32 _diag_rg_aclFilter_show_field(void)
{
	int i;
	
	diag_util_printf("ACL Weight \t: %d\n", acl_parameter.acl_weight);
	diag_util_printf("[ Patterns ] \n");	
	
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_PORT_MASK_BIT)
		diag_util_printf("  ingress_port_mask \t\t: 0x%x", acl_parameter.ingress_port_mask);
	else
		diag_util_printf("  ingress_port_mask \t\t: default all lan/wan port");
	if(acl_parameter.filter_fields & RT_ACL_CARE_PORT_UNITYPE_PPTP_BIT)
		diag_util_printf(" (care unitype pptp portmask)\n");
	else
		diag_util_printf("\n");

	if(acl_parameter.filter_fields & RT_ACL_INGRESS_WLAN_MBSSID_MASK_BIT) {
		for(i=0; i<RT_WLAN_DEVICE_MAX; i++)
			diag_util_printf("  ingress_wlan%d_mbssid_mask \t: 0x%x\n", i, acl_parameter.ingress_wlan_mbssid_mask[i]);
	}

	if(acl_parameter.filter_fields & RT_ACL_INGRESS_SMAC_BIT) {
		diag_util_printf("  ingress_smac \t\t\t: %s\n", diag_util_inet_mactoa(acl_parameter.ingress_smac));
		diag_util_printf("  ingress_smac_mask \t\t: %s\n", diag_util_inet_mactoa(acl_parameter.ingress_smac_mask));
	}
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_DMAC_BIT) {
		diag_util_printf("  ingress_dmac \t\t\t: %s\n", diag_util_inet_mactoa(acl_parameter.ingress_dmac));
		diag_util_printf("  ingress_dmac_mask \t\t: %s\n", diag_util_inet_mactoa(acl_parameter.ingress_dmac_mask));
	}

	if(acl_parameter.filter_fields & RT_ACL_INGRESS_ETHERTYPE_BIT) {
		diag_util_printf("  ingress_ethertype \t\t: 0x%x\n", acl_parameter.ingress_ethertype);
		diag_util_printf("  ingress_ethertype_mask \t: 0x%x\n", acl_parameter.ingress_ethertype_mask);
	}
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_STAGIF_BIT)
		diag_util_printf("  ingress_stagif \t\t: %s\n", acl_parameter.ingress_stagif?"Must have Stag":"Must not have Stag");
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_STAG_VID_BIT)
		diag_util_printf("  ingress_stag_vid \t\t: %d\n", acl_parameter.ingress_stag_vid);
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_STAG_PRI_BIT)
		diag_util_printf("  ingress_stag_pri \t\t: %d\n", acl_parameter.ingress_stag_pri);
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_STAG_DEI_BIT)
		diag_util_printf("  ingress_stag_dei \t\t: %d\n", acl_parameter.ingress_stag_dei);
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_CTAGIF_BIT)
		diag_util_printf("  ingress_ctagif \t\t: %s\n", acl_parameter.ingress_ctagif?"Must have Ctag":"Must not have Ctag");
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_CTAG_VID_BIT)
		diag_util_printf("  ingress_ctag_vid \t\t: %d\n", acl_parameter.ingress_ctag_vid);
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_CTAG_PRI_BIT)
		diag_util_printf("  ingress_ctag_pri \t\t: %d\n", acl_parameter.ingress_ctag_pri);
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_CTAG_CFI_BIT)
		diag_util_printf("  ingress_ctag_cfi \t\t: %d\n", acl_parameter.ingress_ctag_cfi);

	if(acl_parameter.filter_fields & RT_ACL_INGRESS_IPV4_TAGIF_BIT)
		diag_util_printf("  ingress_ipv4_tagif \t\t: %s\n", acl_parameter.ingress_ipv4_tagif?"Must be IPv4":"Must not be IPv4");
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_IPV4_SIP_RANGE_BIT) {
		diag_util_printf("  ingress_src_ipv4_addr_start \t: %s\n", diag_util_inet_ntoa(acl_parameter.ingress_src_ipv4_addr_start));
		diag_util_printf("  ingress_src_ipv4_addr_end \t: %s\n", diag_util_inet_ntoa(acl_parameter.ingress_src_ipv4_addr_end));
	}
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_IPV4_DIP_RANGE_BIT) {
		diag_util_printf("  ingress_dest_ipv4_addr_start \t: %s\n", diag_util_inet_ntoa(acl_parameter.ingress_dest_ipv4_addr_start));
		diag_util_printf("  ingress_dest_ipv4_addr_end \t: %s\n", diag_util_inet_ntoa(acl_parameter.ingress_dest_ipv4_addr_end));
	}
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_IPV4_DSCP_BIT)
		diag_util_printf("  ingress_ipv4_dscp \t\t: %d\n", acl_parameter.ingress_ipv4_dscp);
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_IPV4_TOS_BIT)
		diag_util_printf("  ingress_ipv4_tos \t\t: %d\n", acl_parameter.ingress_ipv4_tos);

	if(acl_parameter.filter_fields & RT_ACL_INGRESS_IPV6_TAGIF_BIT)
		diag_util_printf("  ingress_ipv6_tagif \t\t: %s\n", acl_parameter.ingress_ipv6_tagif?"Must be IPv6":"Must not be IPv6");
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_IPV6_SIP_BIT) {
		diag_util_printf("  ingress_src_ipv6_addr \t: %s\n", diag_util_inet_n6toa(&acl_parameter.ingress_src_ipv6_addr[0]));
		diag_util_printf("  ingress_src_ipv6_addr_mask \t: %s\n", diag_util_inet_n6toa(&acl_parameter.ingress_src_ipv6_addr_mask[0]));
	}
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_IPV6_DIP_BIT) {
		diag_util_printf("  ingress_dest_ipv6_addr \t: %s\n", diag_util_inet_n6toa(&acl_parameter.ingress_dest_ipv6_addr[0]));
		diag_util_printf("  ingress_dest_ipv6_addr_mask \t: %s\n", diag_util_inet_n6toa(&acl_parameter.ingress_dest_ipv6_addr_mask[0]));
	}
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_IPV6_DSCP_BIT)
		diag_util_printf("  ingress_ipv6_dscp \t\t: %d\n", acl_parameter.ingress_ipv6_dscp);
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_IPV6_TC_BIT)
		diag_util_printf("  ingress_ipv6_tc \t\t: %d\n", acl_parameter.ingress_ipv6_tc);

	if(acl_parameter.filter_fields & RT_ACL_INGRESS_L4_TCP_BIT) {
		diag_util_printf("  ingress_l4_protocal \t\t: tcp \n");
	}else if(acl_parameter.filter_fields & RT_ACL_INGRESS_L4_UDP_BIT) {
		diag_util_printf("  ingress_l4_protocal \t\t: udp \n");
	}else if(acl_parameter.filter_fields & RT_ACL_INGRESS_L4_ICMP_BIT) {
		diag_util_printf("  ingress_l4_protocal \t\t: icmp \n");
	}else if(acl_parameter.filter_fields & RT_ACL_INGRESS_L4_ICMPV6_BIT) {
		diag_util_printf("  ingress_l4_protocal \t\t: icmpv6 \n");
	}else if(acl_parameter.filter_fields & RT_ACL_INGRESS_L4_POROTCAL_VALUE_BIT) {
		diag_util_printf("  ingress_l4_protocal \t\t: %d\n", acl_parameter.ingress_l4_protocal);
	}
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_L4_SPORT_RANGE_BIT) {
		diag_util_printf("  ingress_src_l4_port_start \t: %d\n", acl_parameter.ingress_src_l4_port_start);
		diag_util_printf("  ingress_src_l4_port_end \t: %d\n", acl_parameter.ingress_src_l4_port_end);
	}
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_L4_DPORT_RANGE_BIT) {
		diag_util_printf("  ingress_dest_l4_port_start \t: %d\n", acl_parameter.ingress_dest_l4_port_start);
		diag_util_printf("  ingress_dest_l4_port_end \t: %d\n", acl_parameter.ingress_dest_l4_port_end);
	}

	if(acl_parameter.filter_fields & RT_ACL_INGRESS_STREAM_ID_BIT) {
		diag_util_printf("  ingress_stream_id \t\t: %d\n", acl_parameter.ingress_stream_id);
		diag_util_printf("  ingress_stream_id_mask \t: 0x%x\n", acl_parameter.ingress_stream_id_mask);
	}
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_TCP_FLAGS_BIT) {
		diag_util_printf("  ingress_tcp_flags \t\t: 0x%x\n", acl_parameter.ingress_tcp_flags);
		diag_util_printf("  ingress_tcp_flags_mask \t: 0x%x\n", acl_parameter.ingress_tcp_flags_mask);
	}
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_PKT_LEN_RANGE_BIT) {
		diag_util_printf("  ingress_packet_length_start \t: %d\n", acl_parameter.ingress_packet_length_start);
		diag_util_printf("  ingress_packet_length_end \t: %d\n", acl_parameter.ingress_packet_length_end);
	}
	if(acl_parameter.filter_fields & RT_ACL_INGRESS_ARP_TARGET_IP_BIT) {
		diag_util_printf("  ingress_arp_target_ip \t: %s\n", diag_util_inet_ntoa(acl_parameter.ingress_arp_target_ip));
	}

	diag_util_printf("[ Actions ] \n");
	if(acl_parameter.action_fields & RT_ACL_ACTION_FORWARD_GROUP_DROP_BIT)
		diag_util_printf("  FORWARD \t: drop \n");
	if(acl_parameter.action_fields & RT_ACL_ACTION_FORWARD_GROUP_PERMIT_BIT)
		diag_util_printf("  FORWARD \t: permit \n");
	if(acl_parameter.action_fields & RT_ACL_ACTION_FORWARD_GROUP_TRAP_BIT)
		diag_util_printf("  FORWARD \t: trap \n");
	if(acl_parameter.action_fields & RT_ACL_ACTION_FORWARD_GROUP_TRAP_SLAVECPU_BIT)
		diag_util_printf("  FORWARD \t: trap to slave cpu\n");
	if(acl_parameter.action_fields & RT_ACL_ACTION_FORWARD_GROUP_TRAP_TO_PS_BIT)
		diag_util_printf("  FORWARD \t: trap to ps \n");
	if(acl_parameter.action_fields & RT_ACL_ACTION_FORWARD_GROUP_REDIRECT_BIT)
		diag_util_printf("  FORWARD \t: redirect to port %d \n", acl_parameter.action_forward_group_redirect_port_idx);
	if(acl_parameter.action_fields & RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT)
		diag_util_printf("  PRIORITY \t: trap priority %d \n", acl_parameter.action_priority_group_trap_priority);
	if(acl_parameter.action_fields & RT_ACL_ACTION_PRIORITY_GROUP_ACL_PRIORITY_BIT)
		diag_util_printf("  PRIORITY \t: acl priority %d \n", acl_parameter.action_priority_group_acl_priority);
	if(acl_parameter.action_fields & RT_ACL_ACTION_METER_GROUP_SHARE_METER_BIT)
		diag_util_printf("  METER \t: shere meter with meter index %d \n", acl_parameter.action_meter_group_share_meter_index);
	if(acl_parameter.action_fields & RT_ACL_ACTION_LOGGING_GROUP_MIB_BIT)
		diag_util_printf("  LOGGING \t: mib index %d \n", acl_parameter.action_logging_group_mib_index);

	if(acl_parameter.action_fields == 0x0)
		diag_util_printf("  NULL\n");
	
	return CPARSER_OK;
}


/*
 * rt_acl add entry
 */
cparser_result_t
cparser_cmd_rt_acl_add_entry(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	int acl_filter_idx = 0;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rt_acl_filterAndQos_add(acl_parameter, &acl_filter_idx);
	if(ret == RT_ERR_OK)
		diag_util_mprintf("add acl entry[%d] success!\n", acl_filter_idx);
	else
		diag_util_mprintf("add acl entry failed! (ret=0x%x) \n",ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_add_entry */

/*
 * rt_acl get entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rt_acl_get_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int32 ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&acl_parameter, 0x0, sizeof(rt_acl_filterAndQos_t));
	ret = rt_acl_filterAndQos_get(*entry_index_ptr, &acl_parameter);
	if(ret == RT_ERR_OK){
		diag_util_printf("+++ ACL Entry[%d] +++\n",*entry_index_ptr);
		_diag_rg_aclFilter_show_field();
		diag_util_mprintf("get acl entry[%d] success!\n",*entry_index_ptr);
	}else{
		diag_util_mprintf("get acl entry[%d] failed!\n",*entry_index_ptr);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_get_entry_entry_index */

/*
 * rt_acl del entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rt_acl_del_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int32 ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rt_acl_filterAndQos_del(*entry_index_ptr);

	if(ret == RT_ERR_OK)
		diag_util_mprintf("del acl entry[%d] success!\n",*entry_index_ptr);
	else
		diag_util_mprintf("del acl entry[%d] failed!\n",*entry_index_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_del_entry_entry_index */

/*
 * rt_acl show
 */
cparser_result_t
cparser_cmd_rt_acl_show(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	diag_util_printf("+++ Current ACL Entry +++\n");
	_diag_rg_aclFilter_show_field();

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_show */

/*
 * rt_acl clear
 */
cparser_result_t
cparser_cmd_rt_acl_clear(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&acl_parameter, 0x0, sizeof(rt_acl_filterAndQos_t));
	diag_util_mprintf("acl entry clear!\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_clear */

/*
 * rt_acl set acl_weight <INT:acl_weight>
 */
cparser_result_t
cparser_cmd_rt_acl_set_acl_weight_acl_weight(
    cparser_context_t *context,
    int32_t  *acl_weight_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	
	acl_parameter.acl_weight = *acl_weight_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_acl_weight_acl_weight */

/*
 * rt_acl set pattern ingress_port_mask <HEX:portmask>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_port_mask_portmask(
    cparser_context_t *context,
    uint32_t  *portmask_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_PORT_MASK_BIT;
	acl_parameter.ingress_port_mask = *portmask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_port_mask_portmask */

/*
 * rt_acl set pattern care_port_unitype_pptp ( disable | enable )
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_care_port_unitype_pptp_disable_enable(
	cparser_context_t *context)
{
	DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(4, 0))
		acl_parameter.filter_fields |= RT_ACL_CARE_PORT_UNITYPE_PPTP_BIT;
    else if('d' == TOKEN_CHAR(4, 0))
		acl_parameter.filter_fields &= ~RT_ACL_CARE_PORT_UNITYPE_PPTP_BIT;
    else
        return CPARSER_NOT_OK;

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_acl_set_pattern_care_port_unitype_pptp_disable_enable */

/*
 * rt_acl set pattern ingress_wlan_mbssid_mask wlan0 <HEX:wlan0_mbssid_mask>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_wlan_mbssid_mask_wlan0_wlan0_mbssid_mask(
    cparser_context_t *context,
    uint32_t  *wlan0_mbssid_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_WLAN_MBSSID_MASK_BIT;
	acl_parameter.ingress_wlan_mbssid_mask[RT_WLAN_DEVICE0_INDEX] = *wlan0_mbssid_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_wlan_mbssid_mask_wlan0_wlan0_mbssid_mask */

/*
 * rt_acl set pattern ingress_wlan_mbssid_mask wlan0 <HEX:wlan0_mbssid_mask> wlan1 <HEX:wlan1_mbssid_mask>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_wlan_mbssid_mask_wlan0_wlan0_mbssid_mask_wlan1_wlan1_mbssid_mask(
    cparser_context_t *context,
    uint32_t  *wlan0_mbssid_mask_ptr,
    uint32_t  *wlan1_mbssid_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_WLAN_MBSSID_MASK_BIT;
	acl_parameter.ingress_wlan_mbssid_mask[RT_WLAN_DEVICE0_INDEX] = *wlan0_mbssid_mask_ptr;
	acl_parameter.ingress_wlan_mbssid_mask[RT_WLAN_DEVICE1_INDEX] = *wlan1_mbssid_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_wlan_mbssid_mask_wlan0_wlan0_mbssid_mask_wlan1_wlan1_mbssid_mask */

/*
 * rt_acl set pattern ingress_wlan_mbssid_mask wlan0 <HEX:wlan0_mbssid_mask> wlan1 <HEX:wlan1_mbssid_mask> wlan2 <HEX:wlan2_mbssid_mask>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_wlan_mbssid_mask_wlan0_wlan0_mbssid_mask_wlan1_wlan1_mbssid_mask_wlan2_wlan2_mbssid_mask(
    cparser_context_t *context,
    uint32_t  *wlan0_mbssid_mask_ptr,
    uint32_t  *wlan1_mbssid_mask_ptr,
    uint32_t  *wlan2_mbssid_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_WLAN_MBSSID_MASK_BIT;
	acl_parameter.ingress_wlan_mbssid_mask[RT_WLAN_DEVICE0_INDEX] = *wlan0_mbssid_mask_ptr;
	acl_parameter.ingress_wlan_mbssid_mask[RT_WLAN_DEVICE1_INDEX] = *wlan1_mbssid_mask_ptr;
	acl_parameter.ingress_wlan_mbssid_mask[RT_WLAN_DEVICE2_INDEX] = *wlan2_mbssid_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_wlan_mbssid_mask_wlan0_wlan0_mbssid_mask_wlan1_wlan1_mbssid_mask_wlan2_wlan2_mbssid_mask */

/*
 * rt_acl set pattern ingress_smac <MACADDR:smac>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_smac_smac(
    cparser_context_t *context,
    cparser_macaddr_t  *smac_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_SMAC_BIT;
	osal_memcpy(&acl_parameter.ingress_smac, smac_ptr->octet, ETHER_ADDR_LEN);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_smac_smac */

/*
 * rt_acl set pattern ingress_smac_mask <MACADDR:smac_mask>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_smac_mask_smac_mask(
    cparser_context_t *context,
    cparser_macaddr_t  *smac_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_SMAC_BIT;
	osal_memcpy(&acl_parameter.ingress_smac_mask, smac_mask_ptr->octet, ETHER_ADDR_LEN);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_smac_mask_smac_mask */

/*
 * rt_acl set pattern ingress_dmac <MACADDR:dmac>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_dmac_dmac(
    cparser_context_t *context,
    cparser_macaddr_t  *dmac_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_DMAC_BIT;
	osal_memcpy(&acl_parameter.ingress_dmac, dmac_ptr->octet, ETHER_ADDR_LEN);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_dmac_dmac */

/*
 * rt_acl set pattern ingress_dmac_mask <MACADDR:dmac_mask>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_dmac_mask_dmac_mask(
    cparser_context_t *context,
    cparser_macaddr_t  *dmac_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_DMAC_BIT;
	osal_memcpy(&acl_parameter.ingress_dmac_mask, dmac_mask_ptr->octet, ETHER_ADDR_LEN);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_dmac_mask_dmac_mask */

/*
 * rt_acl set pattern ingress_ethertype <HEX:ethertype>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_ethertype_ethertype(
    cparser_context_t *context,
    uint32_t  *ethertype_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_PARAM_RANGE_CHK((*ethertype_ptr > RTK_ETHERTYPE_MAX), CPARSER_ERR_INVALID_PARAMS);
	acl_parameter.filter_fields |= RT_ACL_INGRESS_ETHERTYPE_BIT;	
	acl_parameter.ingress_ethertype = *ethertype_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_ethertype_ethertype */

/*
 * rt_acl set pattern ingress_ethertype_mask <HEX:ethertype_mask>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_ethertype_mask_ethertype_mask(
    cparser_context_t *context,
    uint32_t  *ethertype_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_PARAM_RANGE_CHK((*ethertype_mask_ptr > RTK_ETHERTYPE_MAX), CPARSER_ERR_INVALID_PARAMS);
	acl_parameter.filter_fields |= RT_ACL_INGRESS_ETHERTYPE_BIT;	
	acl_parameter.ingress_ethertype_mask = *ethertype_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_ethertype_mask_ethertype_mask */

/*
 * rt_acl set pattern ingress_stagif <UINT:stagif>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_stagif_stagif(
    cparser_context_t *context,
    uint32_t  *stagif_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_STAGIF_BIT;
	acl_parameter.ingress_stagif = *stagif_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_stagif_stagif */

/*
 * rt_acl set pattern ingress_stag_vid <UINT:stag_vid>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_stag_vid_stag_vid(
    cparser_context_t *context,
    uint32_t  *stag_vid_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_STAG_VID_BIT;
	acl_parameter.ingress_stag_vid = *stag_vid_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_stag_vid_stag_vid */

/*
 * rt_acl set pattern ingress_stag_pri <UINT:stag_pri>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_stag_pri_stag_pri(
    cparser_context_t *context,
    uint32_t  *stag_pri_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_STAG_PRI_BIT;
	acl_parameter.ingress_stag_pri = *stag_pri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_stag_pri_stag_pri */

/*
 * rt_acl set pattern ingress_stag_dei <UINT:stag_dei>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_stag_dei_stag_dei(
	cparser_context_t *context,
	uint32_t  *stag_dei_ptr)
{
	DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_STAG_DEI_BIT;
	acl_parameter.ingress_stag_dei = *stag_dei_ptr;

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_acl_set_pattern_ingress_stag_dei_stag_dei */

/*
 * rt_acl set pattern ingress_ctagif <UINT:ctagif>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_ctagif_ctagif(
    cparser_context_t *context,
    uint32_t  *ctagif_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_CTAGIF_BIT;
	acl_parameter.ingress_ctagif = *ctagif_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_ctagif_ctagif */

/*
 * rt_acl set pattern ingress_ctag_vid <UINT:vid>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_ctag_vid_vid(
    cparser_context_t *context,
    uint32_t  *vid_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_CTAG_VID_BIT;
	acl_parameter.ingress_ctag_vid = *vid_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_ctag_vid_vid */

/*
 * rt_acl set pattern ingress_ctag_pri <UINT:pri>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_ctag_pri_pri(
    cparser_context_t *context,
    uint32_t  *pri_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_CTAG_PRI_BIT;
	acl_parameter.ingress_ctag_pri = *pri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_ctag_pri_pri */

/*
 * rt_acl set pattern ingress_ctag_cfi <UINT:cfi>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_ctag_cfi_cfi(
	cparser_context_t *context,
	uint32_t  *cfi_ptr)
{
	DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_CTAG_CFI_BIT;
	acl_parameter.ingress_ctag_cfi = *cfi_ptr;

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_acl_set_pattern_ingress_ctag_cfi_cfi */

/*
 * rt_acl set pattern ingress_ipv4_tagif <UINT:ipv4_tagif>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_ipv4_tagif_ipv4_tagif(
    cparser_context_t *context,
    uint32_t  *ipv4_tagif_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_IPV4_TAGIF_BIT;
	acl_parameter.ingress_ipv4_tagif = *ipv4_tagif_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_ipv4_tagif_ipv4_tagif */

/*
 * rt_acl set pattern ingress_src_ipv4_addr_start <IPV4ADDR:sip_low_bound> ingress_src_ipv4_addr_end <IPV4ADDR:sip_up_bound>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_src_ipv4_addr_start_sip_low_bound_ingress_src_ipv4_addr_end_sip_up_bound(
    cparser_context_t *context,
    uint32_t  *sip_low_bound_ptr,
    uint32_t  *sip_up_bound_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_IPV4_SIP_RANGE_BIT;
	acl_parameter.ingress_src_ipv4_addr_start = (ipaddr_t)*sip_low_bound_ptr;
	acl_parameter.ingress_src_ipv4_addr_end = (ipaddr_t)*sip_up_bound_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_src_ipv4_addr_start_sip_low_bound_ingress_src_ipv4_addr_end_sip_up_bound */

/*
 * rt_acl set pattern ingress_dest_ipv4_addr_start  <IPV4ADDR:dip_low_bound> ingress_dest_ipv4_addr_end  <IPV4ADDR:dip_up_bound>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_dest_ipv4_addr_start_dip_low_bound_ingress_dest_ipv4_addr_end_dip_up_bound(
    cparser_context_t *context,
    uint32_t  *dip_low_bound_ptr,
    uint32_t  *dip_up_bound_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_IPV4_DIP_RANGE_BIT;
	acl_parameter.ingress_dest_ipv4_addr_start = (ipaddr_t)*dip_low_bound_ptr;
	acl_parameter.ingress_dest_ipv4_addr_end = (ipaddr_t)*dip_up_bound_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_dest_ipv4_addr_start_dip_low_bound_ingress_dest_ipv4_addr_end_dip_up_bound */

/*
 * rt_acl set pattern ingress_ipv4_dscp <UINT:dscp>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_ipv4_dscp_dscp(
    cparser_context_t *context,
    uint32_t  *dscp_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_PARAM_RANGE_CHK((*dscp_ptr > RTK_VALUE_OF_DSCP_MAX), CPARSER_ERR_INVALID_PARAMS);
	acl_parameter.filter_fields |= RT_ACL_INGRESS_IPV4_DSCP_BIT;	
	acl_parameter.ingress_ipv4_dscp = *dscp_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_ipv4_dscp_dscp */

/*
 * rt_acl set pattern ingress_ipv4_tos <UINT:tos>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_ipv4_tos_tos(
    cparser_context_t *context,
    uint32_t  *tos_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_PARAM_RANGE_CHK((*tos_ptr > RTK_VALUE_OF_TOS_TC_MAX), CPARSER_ERR_INVALID_PARAMS);
	acl_parameter.filter_fields |= RT_ACL_INGRESS_IPV4_TOS_BIT;	
	acl_parameter.ingress_ipv4_tos = *tos_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_ipv4_tos_tos */

/*
 * rt_acl set pattern ingress_ipv6_tagif <UINT:ipv6_tagif>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_ipv6_tagif_ipv6_tagif(
    cparser_context_t *context,
    uint32_t  *ipv6_tagif_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_IPV6_TAGIF_BIT;
	acl_parameter.ingress_ipv6_tagif = *ipv6_tagif_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_ipv6_tagif_ipv6_tagif */

/*
 * rt_acl set pattern ingress_src_ipv6_addr  <IPV6ADDR:src_ipv6_addr> 
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_src_ipv6_addr_src_ipv6_addr(
    cparser_context_t *context,
    char * *src_ipv6_addr_ptr)
{
	int32 ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_IPV6_SIP_BIT;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&acl_parameter.ingress_src_ipv6_addr[0], TOKEN_STR(4)), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_src_ipv6_addr_src_ipv6_addr */

/*
 * rt_acl set pattern ingress_src_ipv6_addr_mask  <IPV6ADDR:src_ipv6_addr_mask> 
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_src_ipv6_addr_mask_src_ipv6_addr_mask(
    cparser_context_t *context,
    char * *src_ipv6_addr_mask_ptr)
{
	int32 ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_IPV6_SIP_BIT;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&acl_parameter.ingress_src_ipv6_addr_mask[0], TOKEN_STR(4)), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_src_ipv6_addr_mask_src_ipv6_addr_mask */

/*
 * rt_acl set pattern ingress_dest_ipv6_addr  <IPV6ADDR:dest_ipv6_addr> 
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_dest_ipv6_addr_dest_ipv6_addr(
    cparser_context_t *context,
    char * *dest_ipv6_addr_ptr)
{
	int32 ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_IPV6_DIP_BIT;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&acl_parameter.ingress_dest_ipv6_addr[0], TOKEN_STR(4)), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_dest_ipv6_addr_dest_ipv6_addr */

/*
 * rt_acl set pattern ingress_dest_ipv6_addr_mask  <IPV6ADDR:dest_ipv6_addr_mask> 
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_dest_ipv6_addr_mask_dest_ipv6_addr_mask(
    cparser_context_t *context,
    char * *dest_ipv6_addr_mask_ptr)
{
	int32 ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_IPV6_DIP_BIT;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&acl_parameter.ingress_dest_ipv6_addr_mask[0], TOKEN_STR(4)), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_dest_ipv6_addr_mask_dest_ipv6_addr_mask */

/*
 * rt_acl set pattern ingress_ipv6_dscp <UINT:dscp>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_ipv6_dscp_dscp(
    cparser_context_t *context,
    uint32_t  *dscp_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_PARAM_RANGE_CHK((*dscp_ptr > RTK_VALUE_OF_DSCP_MAX), CPARSER_ERR_INVALID_PARAMS);
	acl_parameter.filter_fields |= RT_ACL_INGRESS_IPV6_DSCP_BIT;	
	acl_parameter.ingress_ipv6_dscp = *dscp_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_ipv6_dscp_dscp */

/*
 * rt_acl set pattern ingress_ipv6_tc <UINT:tc>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_ipv6_tc_tc(
    cparser_context_t *context,
    uint32_t  *tc_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_PARAM_RANGE_CHK((*tc_ptr > RTK_VALUE_OF_TOS_TC_MAX), CPARSER_ERR_INVALID_PARAMS);
	acl_parameter.filter_fields |= RT_ACL_INGRESS_IPV6_TC_BIT;	
	acl_parameter.ingress_ipv6_tc = *tc_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_ipv6_tc_tc */

/*
 * rt_acl set pattern ingress_l4_protocal_value <HEX:protocal>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_l4_protocal_value_protocal(
    cparser_context_t *context,
    uint32_t  *protocal_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_L4_POROTCAL_VALUE_BIT;
	acl_parameter.ingress_l4_protocal = *protocal_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_l4_protocal_value_protocal */

/*
 * rt_acl set pattern ingress_l4_protocal <UINT:protocal>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_l4_protocal_protocal(
    cparser_context_t *context,
    uint32_t  *protocal_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	if(*protocal_ptr==0){
		acl_parameter.filter_fields |= RT_ACL_INGRESS_L4_UDP_BIT;
	}else if(*protocal_ptr==1){
		acl_parameter.filter_fields |= RT_ACL_INGRESS_L4_TCP_BIT;
	}else if(*protocal_ptr==2){
		acl_parameter.filter_fields |= RT_ACL_INGRESS_L4_ICMP_BIT;
	}else{
		diag_util_printf("not support this l4-protocal type, please use ingress_l4_protocal_value!");
		return CPARSER_ERR_INVALID_PARAMS;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_l4_protocal_protocal */

/*
 * rt_acl set pattern ingress_src_l4_port_start <UINT:l4sport_low_bound> ingress_src_l4_port_end <UINT:l4sport_up_bound>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_src_l4_port_start_l4sport_low_bound_ingress_src_l4_port_end_l4sport_up_bound(
    cparser_context_t *context,
    uint32_t  *l4sport_low_bound_ptr,
    uint32_t  *l4sport_up_bound_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_L4_SPORT_RANGE_BIT;
	acl_parameter.ingress_src_l4_port_start = *l4sport_low_bound_ptr;
	acl_parameter.ingress_src_l4_port_end = *l4sport_up_bound_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_src_l4_port_start_l4sport_low_bound_ingress_src_l4_port_end_l4sport_up_bound */

/*
 * rt_acl set pattern ingress_dest_l4_port_start <UINT:l4dport_low_bound> ingress_dest_l4_port_end <UINT:l4dport_up_bound>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_dest_l4_port_start_l4dport_low_bound_ingress_dest_l4_port_end_l4dport_up_bound(
    cparser_context_t *context,
    uint32_t  *l4dport_low_bound_ptr,
    uint32_t  *l4dport_up_bound_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_L4_DPORT_RANGE_BIT;
	acl_parameter.ingress_dest_l4_port_start = *l4dport_low_bound_ptr;
	acl_parameter.ingress_dest_l4_port_end = *l4dport_up_bound_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_dest_l4_port_start_l4dport_low_bound_ingress_dest_l4_port_end_l4dport_up_bound */

/*
 * rt_acl set pattern ingress_stream_id <UINT:stream_id>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_stream_id_stream_id(
    cparser_context_t *context,
    uint32_t  *stream_id_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_STREAM_ID_BIT;
	acl_parameter.ingress_stream_id = *stream_id_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_stream_id_stream_id */

/*
 * rt_acl set pattern ingress_stream_id_mask <HEX:stream_id_mask>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_stream_id_mask_stream_id_mask(
    cparser_context_t *context,
    uint32_t  *stream_id_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_PARAM_RANGE_CHK((*stream_id_mask_ptr > 0xffff), CPARSER_ERR_INVALID_PARAMS);
	acl_parameter.filter_fields |= RT_ACL_INGRESS_STREAM_ID_BIT;
	acl_parameter.ingress_stream_id_mask = *stream_id_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_stream_id_mask_stream_id_mask */

/*
 * rt_acl set pattern ingress_tcp_flags <UINT:tcp_flags>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_tcp_flags_tcp_flags(
    cparser_context_t *context,
    uint32_t  *tcp_flags_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_TCP_FLAGS_BIT;
	acl_parameter.ingress_tcp_flags = *tcp_flags_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_tcp_flags_tcp_flags */

/*
 * rt_acl set pattern ingress_tcp_flags_mask <HEX:tcp_flags_mask>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_tcp_flags_mask_tcp_flags_mask(
    cparser_context_t *context,
    uint32_t  *tcp_flags_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_TCP_FLAGS_BIT;
	acl_parameter.ingress_tcp_flags_mask = *tcp_flags_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_tcp_flags_mask_tcp_flags_mask */

/*
 * rt_acl set pattern ingress_packet_length_start <UINT:pkt_length_low_bound> ingress_packet_length_end <UINT:pkt_length_up_bound>
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_packet_length_start_pkt_length_low_bound_ingress_packet_length_end_pkt_length_up_bound(
    cparser_context_t *context,
    uint32_t  *pkt_length_low_bound_ptr,
    uint32_t  *pkt_length_up_bound_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_PKT_LEN_RANGE_BIT;
	acl_parameter.ingress_packet_length_start = *pkt_length_low_bound_ptr;
	acl_parameter.ingress_packet_length_end = *pkt_length_up_bound_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_packet_length_start_pkt_length_low_bound_ingress_packet_length_end_pkt_length_up_bound */

/*
 * rt_acl set pattern ingress_arp_target_ip  <IPV4ADDR:arp_target_ip> 
 */
cparser_result_t
cparser_cmd_rt_acl_set_pattern_ingress_arp_target_ip_arp_target_ip(
    cparser_context_t *context,
    uint32_t  *arp_target_ip_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= RT_ACL_INGRESS_ARP_TARGET_IP_BIT;
	acl_parameter.ingress_arp_target_ip = *arp_target_ip_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_pattern_ingress_stag_vid_stag_vid */

/*
 * rt_acl set action FORWARD drop
 */
cparser_result_t
cparser_cmd_rt_acl_set_action_FORWARD_drop(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.action_fields |= RT_ACL_ACTION_FORWARD_GROUP_DROP_BIT;
		
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_action_forward_drop */

/*
 * rt_acl set action FORWARD trap
 */
cparser_result_t
cparser_cmd_rt_acl_set_action_FORWARD_trap(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.action_fields |= RT_ACL_ACTION_FORWARD_GROUP_TRAP_BIT;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_action_forward_trap */

/*
 * rt_acl set action FORWARD trap_to_slavecpu
 */
cparser_result_t
cparser_cmd_rt_acl_set_action_FORWARD_trap_to_slavecpu(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.action_fields |= RT_ACL_ACTION_FORWARD_GROUP_TRAP_SLAVECPU_BIT;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_action_FORWARD_trap_to_slavecpu */

/*
 * rt_acl set action FORWARD trap_to_ps
 */
cparser_result_t
cparser_cmd_rt_acl_set_action_FORWARD_trap_to_ps(
	cparser_context_t *context)
{
	DIAG_UTIL_PARAM_CHK();

	acl_parameter.action_fields |= RT_ACL_ACTION_FORWARD_GROUP_TRAP_TO_PS_BIT;

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_acl_set_action_FORWARD_trap_to_ps */

/*
 * rt_acl set action FORWARD permit
 */
cparser_result_t
cparser_cmd_rt_acl_set_action_FORWARD_permit(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.action_fields |= RT_ACL_ACTION_FORWARD_GROUP_PERMIT_BIT;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_action_forward_permit */

/*
 * rt_acl set action FORWARD redirect port <UINT:port_idx>
 */
cparser_result_t
cparser_cmd_rt_acl_set_action_FORWARD_redirect_port_port_idx(
	cparser_context_t *context,
    uint32_t  *port_idx_ptr)
{
	DIAG_UTIL_PARAM_CHK();

	acl_parameter.action_fields |= RT_ACL_ACTION_FORWARD_GROUP_REDIRECT_BIT;
	acl_parameter.action_forward_group_redirect_port_idx = *port_idx_ptr;

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_acl_set_action_FORWARD_redirect_port_port_idx */

/*
 * rt_acl set action PRIORITY trap-priority <UINT:priority>
 */
cparser_result_t
cparser_cmd_rt_acl_set_action_PRIORITY_trap_priority_priority(
    cparser_context_t *context,
    uint32_t  *priority_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.action_fields |= RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT;
	acl_parameter.action_priority_group_trap_priority = *priority_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_action_priority_trap_priority_priority */

/*
 * rt_acl set action PRIORITY acl-priority <UINT:priority>
 */
cparser_result_t
cparser_cmd_rt_acl_set_action_PRIORITY_acl_priority_priority(
	cparser_context_t *context,
	uint32_t  *priority_ptr)
{
	DIAG_UTIL_PARAM_CHK();

	acl_parameter.action_fields |= RT_ACL_ACTION_PRIORITY_GROUP_ACL_PRIORITY_BIT;
	acl_parameter.action_priority_group_acl_priority = *priority_ptr;

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_acl_set_action_priority_acl_priority_priority */

/*
 * rt_acl set action METER share-meter <UINT:meter_idx>
 */
cparser_result_t
cparser_cmd_rt_acl_set_action_METER_share_meter_meter_idx(
    cparser_context_t *context,
    uint32_t  *meter_idx_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.action_fields |= RT_ACL_ACTION_METER_GROUP_SHARE_METER_BIT;
	acl_parameter.action_meter_group_share_meter_index = *meter_idx_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_acl_set_action_meter_share_meter_meter_idx */

/*
 * rt_acl set action LOGGING mib <UINT:mib_idx>
 */
cparser_result_t
cparser_cmd_rt_acl_set_action_LOGGING_mib_mib_idx(
	cparser_context_t *context,
	uint32_t  *mib_idx_ptr)
{
	DIAG_UTIL_PARAM_CHK();

	acl_parameter.action_fields |= RT_ACL_ACTION_LOGGING_GROUP_MIB_BIT;
	acl_parameter.action_logging_group_mib_index = *mib_idx_ptr;

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_acl_set_action_LOGGING_mib_mib_idx */

/*
 * rt_acl reserved_status get <UINT:reserved_type>
 */
cparser_result_t
cparser_cmd_rt_acl_reserved_status_get_reserved_type(
	cparser_context_t *context,
	uint32_t  *reserved_type)
{
	int32 ret = RT_ERR_FAILED;
	int status = 0;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	DIAG_UTIL_PARAM_RANGE_CHK((*reserved_type >= RT_ACL_RESERVED_TYPE_MAX), CPARSER_ERR_INVALID_PARAMS);

	ret = rt_acl_reserved_status_get(*reserved_type, &status);
	if(ret == RT_ERR_OK)
		diag_util_mprintf("ACL reserved type[%d] status is %s \n", *reserved_type, status?"enable":"disable");
	else
		diag_util_mprintf("get acl reserved status failed! (ret=0x%x) \n",ret);

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_acl_reserved_status_get_reserved_type */

/*
 * rt_acl reserved_status get all
 */
cparser_result_t
cparser_cmd_rt_acl_reserved_status_get_all(
	cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	int status = 0;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rt_acl_reserved_status_get(RT_ACL_RESERVED_TYPE_MAX, &status);
	if(ret != RT_ERR_OK)
		diag_util_mprintf("get acl reserved status failed! (ret=0x%x) \n",ret);

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_acl_reserved_status_get_all */

/*
 * rt_acl reserved_status set <UINT:reserved_type> status  ( disable | enable )
 */
cparser_result_t
cparser_cmd_rt_acl_reserved_status_set_reserved_type_status_disable_enable(
	cparser_context_t *context,
	uint32_t  *reserved_type)
{
	int32 ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	DIAG_UTIL_PARAM_RANGE_CHK((*reserved_type >= RT_ACL_RESERVED_TYPE_MAX), CPARSER_ERR_INVALID_PARAMS);

    if('e' == TOKEN_CHAR(5, 0))
		ret = rt_acl_reserved_status_set(*reserved_type, TRUE);
    else if('d' == TOKEN_CHAR(5, 0))
		ret = rt_acl_reserved_status_set(*reserved_type, FALSE);

	if(ret == RT_ERR_OK)
		diag_util_mprintf("%s acl reserved type[%d] success!\n", 'e' == TOKEN_CHAR(5, 0)?"Enable":"Disable", *reserved_type);
	else
		diag_util_mprintf("update acl reserved status failed! (ret=0x%x) \n",ret);

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_acl_reserved_status_set_reserved_type_status_disable_enable */

