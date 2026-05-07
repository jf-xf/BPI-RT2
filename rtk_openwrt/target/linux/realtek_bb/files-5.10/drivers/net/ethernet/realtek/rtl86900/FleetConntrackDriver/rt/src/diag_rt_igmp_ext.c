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
 * Purpose : Definition of RT API of IGMP Extension
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of multicast learning mode
 *           (2) Configuration of multicast group
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

#include <rt_igmp_ext.h>

void _diag_multicastGroupShow(rt_igmp_multicast_group_cfg_t *mcConfig)
{
    diag_util_mprintf("Multicast Group:\n");

	if(mcConfig->is_ipv6)
		diag_util_mprintf("IPv6 Addr: %s\n",diag_util_inet_n6toa(&mcConfig->group_addr.ipv6[0]));
	else
		diag_util_mprintf("IP Addr: %s \n",diag_util_inet_ntoa(mcConfig->group_addr.ipv4));

	if(mcConfig->first_act_portmask) {
		diag_util_mprintf("Action %s:\n", (mcConfig->second_act_portmask)?"1":"");
		diag_util_mprintf("\tPortmask: 0x%x\n", mcConfig->first_act_portmask);
		diag_util_mprintf("\tWlanDevmask: [0x%x/0x%x/0x%x]\n", mcConfig->first_act_wlanMbssidMask[0],mcConfig->first_act_wlanMbssidMask[1],mcConfig->first_act_wlanMbssidMask[2]);
		diag_util_mprintf("\tCtagif: %d\n", mcConfig->first_act_ctagif);
		diag_util_mprintf("\tCtag vid: %d\n", mcConfig->first_act_cvid);
		diag_util_mprintf("\tCtag pri: %d\n", mcConfig->first_act_cpri);
	}
	if(mcConfig->second_act_portmask) {
		diag_util_mprintf("Action %s:\n", (mcConfig->first_act_portmask)?"2":"1");
		diag_util_mprintf("\tPortmask: 0x%x\n", mcConfig->second_act_portmask);
		diag_util_mprintf("\tWlanDevmask: [0x%x/0x%x/0x%x]\n", mcConfig->second_act_wlanMbssidMask[0],mcConfig->second_act_wlanMbssidMask[1],mcConfig->second_act_wlanMbssidMask[2]);
		diag_util_mprintf("\tCtagif: %d\n", mcConfig->second_act_ctagif);
		diag_util_mprintf("\tCtag vid: %d\n", mcConfig->second_act_cvid);
		diag_util_mprintf("\tCtag pri: %d\n", mcConfig->second_act_cpri);
	}
}

/*
 * rt_igmp set multicast learning-mode <UINT:mode>
 */
cparser_result_t
cparser_cmd_rt_igmp_set_multicast_learning_mode_mode(
    cparser_context_t *context,
    uint32_t  *mode_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    ret = rt_igmp_multicastMode_set(*mode_ptr);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("set multicast-mode failed. RT API return 0x%x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Multicast Mode: %s\n", (*mode_ptr)?"User":"Kernel");
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_igmp_set_multicast_learning_mode_mode */


/*
 * rt_igmp set multicast unknowMc ipVersion <UINT:ipVersion> action <UINT:action>
 */
cparser_result_t
cparser_cmd_rt_igmp_set_multicast_unknowMc_ipVersion_ipVersion_action_action(
	cparser_context_t *context,
	uint32_t  *ipVersion_ptr,
	uint32_t  *action_ptr)
{
	rt_igmp_unknownMc_cfg_t unknownMcCfg;
		
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&unknownMcCfg, 0x0, sizeof(unknownMcCfg));
	unknownMcCfg.isipv6= (*ipVersion_ptr)?1:0;
	unknownMcCfg.trapPkt = (*action_ptr);

	rt_igmp_unknownMcAction_set(unknownMcCfg);

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_igmp_set_multicast_unknowmc_ipversion_ipversion_action_action */



/*
 * rt_igmp set multicast group ipv4 ip_address <IPV4ADDR:mc_gip_v4> port_mask <HEX:portmask> wlan_mbssid_mask0 <HEX:wlanMbssidMask0> wlan_mbssid_mask1 <HEX:wlanMbssidMask1> wlan_mbssid_mask2 <HEX:wlanMbssidMask2> action_ctag tagDecision <UINT:tagDecision> cvid <UINT:cvid> cpri <UINT:cpri>
 */
cparser_result_t
cparser_cmd_rt_igmp_set_multicast_group_ipv4_ip_address_mc_gip_v4_port_mask_portmask_wlan_mbssid_mask0_wlanMbssidMask0_wlan_mbssid_mask1_wlanMbssidMask1_wlan_mbssid_mask2_wlanMbssidMask2_action_ctag_tagDecision_tagDecision_cvid_cvid_cpri_cpri(
    cparser_context_t *context,
    uint32_t  *mc_gip_v4_ptr,
    uint32_t  *portmask_ptr,
    uint32_t  *wlanMbssidMask0_ptr,
    uint32_t  *wlanMbssidMask1_ptr,
    uint32_t  *wlanMbssidMask2_ptr,
    uint32_t  *tagDecision_ptr,
    uint32_t  *cvid_ptr,
    uint32_t  *cpri_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rt_igmp_multicast_group_cfg_t mcConfig;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&mcConfig, 0x0, sizeof(rt_igmp_multicast_group_cfg_t));
    mcConfig.is_ipv6						= 0;
    mcConfig.group_addr.ipv4				= *mc_gip_v4_ptr;
    mcConfig.first_act_portmask				= *portmask_ptr;
    mcConfig.first_act_wlanMbssidMask[0]	= *wlanMbssidMask0_ptr;
    mcConfig.first_act_wlanMbssidMask[1]	= *wlanMbssidMask1_ptr;
    mcConfig.first_act_wlanMbssidMask[2]	= *wlanMbssidMask2_ptr;	
    mcConfig.first_act_ctagif				= *tagDecision_ptr;
    mcConfig.first_act_cvid					= *cvid_ptr;
    mcConfig.first_act_cpri					= *cpri_ptr;

    ret = rt_igmp_multicastGroup_set(mcConfig);
	if(ret != RT_ERR_OK){
		_diag_multicastGroupShow(&mcConfig);
		diag_util_mprintf("Please config multicast mode as User first, then make sure the portmask should not overlap.\n",ret);
		diag_util_mprintf("set multicast-group failed. RT API return 0x%x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		_diag_multicastGroupShow(&mcConfig);
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rt_igmp_set_multicast_group_ipv4_ip_address_mc_gip_v4_port_mask_portmask_wlan_mbssid_mask0_wlanmbssidmask0_wlan_mbssid_mask1_wlanmbssidmask1_wlan_mbssid_mask2_wlanmbssidmask2_action_ctag_tagdecision_tagdecision_cvid_cvid_cpri_cpri */



/*
 * rt_igmp set multicast group ipv6 ip_address <IPV6ADDR:mc_gip_v6> port_mask <HEX:portmask> wlan_mbssid_mask0 <HEX:wlanMbssidMask0> wlan_mbssid_mask1 <HEX:wlanMbssidMask1> wlan_mbssid_mask2 <HEX:wlanMbssidMask2> action_ctag tagDecision <UINT:tagDecision> cvid <UINT:cvid> cpri <UINT:cpri>
 */
cparser_result_t
cparser_cmd_rt_igmp_set_multicast_group_ipv6_ip_address_mc_gip_v6_port_mask_portmask_wlan_mbssid_mask0_wlanMbssidMask0_wlan_mbssid_mask1_wlanMbssidMask1_wlan_mbssid_mask2_wlanMbssidMask2_action_ctag_tagDecision_tagDecision_cvid_cvid_cpri_cpri(
    cparser_context_t *context,
    char * *mc_gip_v6_ptr,
    uint32_t  *portmask_ptr,
    uint32_t  *wlanMbssidMask0_ptr,
    uint32_t  *wlanMbssidMask1_ptr,
    uint32_t  *wlanMbssidMask2_ptr,
    uint32_t  *tagDecision_ptr,
    uint32_t  *cvid_ptr,
    uint32_t  *cpri_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rt_igmp_multicast_group_cfg_t mcConfig;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&mcConfig, 0x0, sizeof(rt_igmp_multicast_group_cfg_t));
    mcConfig.is_ipv6				= 1;
    DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&mcConfig.group_addr.ipv6[0], TOKEN_STR(6)), ret);
    mcConfig.first_act_portmask		= *portmask_ptr;
    mcConfig.first_act_wlanMbssidMask[0]	= *wlanMbssidMask0_ptr;
    mcConfig.first_act_wlanMbssidMask[1]	= *wlanMbssidMask1_ptr;
    mcConfig.first_act_wlanMbssidMask[2]	= *wlanMbssidMask2_ptr;	
    mcConfig.first_act_ctagif			= *tagDecision_ptr;
    mcConfig.first_act_cvid			= *cvid_ptr;
    mcConfig.first_act_cpri			= *cpri_ptr;
    ret = rt_igmp_multicastGroup_set(mcConfig);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("Please config multicast mode as User first, then make sure the portmask should not overlap.\n",ret);
		diag_util_mprintf("set multicast-group failed. RT API return 0x%x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		_diag_multicastGroupShow(&mcConfig);
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rt_igmp_set_multicast_group_ipv6_ip_address_mc_gip_v6_port_mask_portmask_wlan_mbssid_mask0_wlanmbssidmask0_wlan_mbssid_mask1_wlanmbssidmask1_wlan_mbssid_mask2_wlanmbssidmask2_action_ctag_tagdecision_tagdecision_cvid_cvid_cpri_cpri */


/*
 * rt_igmp set multicast group ipv4 ip_address <IPV4ADDR:mc_gip_v4> port_mask <HEX:portmask> wlan_mbssid_mask0 <HEX:wlanMbssidMask0> wlan_mbssid_mask1 <HEX:wlanMbssidMask1> wlan_mbssid_mask2 <HEX:wlanMbssidMask2> port_mask2 <HEX:portmask2> sec_wlan_dev_mask0 <HEX:secWlandevmask0> sec_wlan_dev_mask1 <HEX:secWlandevmask1> sec_wlan_dev_mask2 <HEX:secWlandevmask2> action_ctag tagDecision <UINT:tagDecision> cvid <UINT:cvid> cpri <UINT:cpri> tagDecision2 <UINT:tagDecision2> cvid2 <UINT:cvid2> cpri2 <UINT:cpri2>
 */
cparser_result_t
cparser_cmd_rt_igmp_set_multicast_group_ipv4_ip_address_mc_gip_v4_port_mask_portmask_wlan_mbssid_mask0_wlanMbssidMask0_wlan_mbssid_mask1_wlanMbssidMask1_wlan_mbssid_mask2_wlanMbssidMask2_port_mask2_portmask2_sec_wlan_dev_mask0_secWlandevmask0_sec_wlan_dev_mask1_secWlandevmask1_sec_wlan_dev_mask2_secWlandevmask2_action_ctag_tagDecision_tagDecision_cvid_cvid_cpri_cpri_tagDecision2_tagDecision2_cvid2_cvid2_cpri2_cpri2(
    cparser_context_t *context,
    uint32_t  *mc_gip_v4_ptr,
    uint32_t  *portmask_ptr,
    uint32_t  *wlanMbssidMask0_ptr,
    uint32_t  *wlanMbssidMask1_ptr,
    uint32_t  *wlanMbssidMask2_ptr,
    uint32_t  *portmask2_ptr,
    uint32_t  *secWlandevmask0_ptr,
    uint32_t  *secWlandevmask1_ptr,
    uint32_t  *secWlandevmask2_ptr,
    uint32_t  *tagDecision_ptr,
    uint32_t  *cvid_ptr,
    uint32_t  *cpri_ptr,
    uint32_t  *tagDecision2_ptr,
    uint32_t  *cvid2_ptr,
    uint32_t  *cpri2_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rt_igmp_multicast_group_cfg_t mcConfig;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&mcConfig, 0x0, sizeof(rt_igmp_multicast_group_cfg_t));
    mcConfig.is_ipv6				= 0;
    mcConfig.group_addr.ipv4		= *mc_gip_v4_ptr;
    mcConfig.first_act_portmask		= *portmask_ptr;
    mcConfig.first_act_wlanMbssidMask[0]	= *wlanMbssidMask0_ptr;
    mcConfig.first_act_wlanMbssidMask[1]	= *wlanMbssidMask1_ptr;
    mcConfig.first_act_wlanMbssidMask[2]	= *wlanMbssidMask2_ptr;	
    mcConfig.second_act_portmask	= *portmask2_ptr;
    mcConfig.second_act_wlanMbssidMask[0]= *secWlandevmask0_ptr;
    mcConfig.second_act_wlanMbssidMask[1]= *secWlandevmask1_ptr;
    mcConfig.second_act_wlanMbssidMask[2]= *secWlandevmask2_ptr;	
    mcConfig.first_act_ctagif			= *tagDecision_ptr;
    mcConfig.first_act_cvid			= *cvid_ptr;
    mcConfig.first_act_cpri			= *cpri_ptr;
    mcConfig.second_act_ctagif 		= *tagDecision2_ptr;
    mcConfig.second_act_cvid		= *cvid2_ptr;
    mcConfig.second_act_cpri			= *cpri2_ptr;

    ret = rt_igmp_multicastGroup_set(mcConfig);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("Please config multicast mode as User first, then make sure the portmask should not overlap.\n",ret);
		diag_util_mprintf("set multicast-group failed. RT API return 0x%x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		_diag_multicastGroupShow(&mcConfig);
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rt_igmp_set_multicast_group_ipv4_ip_address_mc_gip_v4_port_mask_portmask_wlan_mbssid_mask0_wlanmbssidmask0_wlan_mbssid_mask1_wlanmbssidmask1_wlan_mbssid_mask2_wlanmbssidmask2_port_mask2_portmask2_sec_wlan_dev_mask0_secwlandevmask0_sec_wlan_dev_mask1_secwlandevmask1_sec_wlan_dev_mask2_secwlandevmask2_action_ctag_tagdecision_tagdecision_cvid_cvid_cpri_cpri_tagdecision2_tagdecision2_cvid2_cvid2_cpri2_cpri2 */


/*
 * rt_igmp set multicast group ipv6 ip_address <IPV6ADDR:mc_gip_v6> port_mask <HEX:portmask> wlan_mbssid_mask0 <HEX:wlanMbssidMask0> wlan_mbssid_mask1 <HEX:wlanMbssidMask1> wlan_mbssid_mask2 <HEX:wlanMbssidMask2> port_mask2 <HEX:portmask2> sec_wlan_dev_mask0 <HEX:secWlandevmask0> sec_wlan_dev_mask1 <HEX:secWlandevmask1> sec_wlan_dev_mask2 <HEX:secWlandevmask2> action_ctag tagDecision <UINT:tagDecision> cvid <UINT:cvid> cpri <UINT:cpri> tagDecision2 <UINT:tagDecision2> cvid2 <UINT:cvid2> cpri2 <UINT:cpri2>
 */
cparser_result_t
cparser_cmd_rt_igmp_set_multicast_group_ipv6_ip_address_mc_gip_v6_port_mask_portmask_wlan_mbssid_mask0_wlanMbssidMask0_wlan_mbssid_mask1_wlanMbssidMask1_wlan_mbssid_mask2_wlanMbssidMask2_port_mask2_portmask2_sec_wlan_dev_mask0_secWlandevmask0_sec_wlan_dev_mask1_secWlandevmask1_sec_wlan_dev_mask2_secWlandevmask2_action_ctag_tagDecision_tagDecision_cvid_cvid_cpri_cpri_tagDecision2_tagDecision2_cvid2_cvid2_cpri2_cpri2(
    cparser_context_t *context,
    char * *mc_gip_v6_ptr,
    uint32_t  *portmask_ptr,
    uint32_t  *wlanMbssidMask0_ptr,
    uint32_t  *wlanMbssidMask1_ptr,
    uint32_t  *wlanMbssidMask2_ptr,
    uint32_t  *portmask2_ptr,
    uint32_t  *secWlandevmask0_ptr,
    uint32_t  *secWlandevmask1_ptr,
    uint32_t  *secWlandevmask2_ptr,
    uint32_t  *tagDecision_ptr,
    uint32_t  *cvid_ptr,
    uint32_t  *cpri_ptr,
    uint32_t  *tagDecision2_ptr,
    uint32_t  *cvid2_ptr,
    uint32_t  *cpri2_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rt_igmp_multicast_group_cfg_t mcConfig;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&mcConfig, 0x0, sizeof(rt_igmp_multicast_group_cfg_t));
    mcConfig.is_ipv6				= 1;
    DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&mcConfig.group_addr.ipv6[0], TOKEN_STR(6)), ret);
    mcConfig.first_act_portmask		= *portmask_ptr;
    mcConfig.first_act_wlanMbssidMask[0]	= *wlanMbssidMask0_ptr;
    mcConfig.first_act_wlanMbssidMask[1]	= *wlanMbssidMask1_ptr;
    mcConfig.first_act_wlanMbssidMask[2]	= *wlanMbssidMask2_ptr;	
    mcConfig.second_act_portmask	= *portmask2_ptr;
    mcConfig.second_act_wlanMbssidMask[0]= *secWlandevmask0_ptr;
    mcConfig.second_act_wlanMbssidMask[1]= *secWlandevmask1_ptr;
    mcConfig.second_act_wlanMbssidMask[2]= *secWlandevmask2_ptr;		
    mcConfig.first_act_ctagif			= *tagDecision_ptr;
    mcConfig.first_act_cvid			= *cvid_ptr;
    mcConfig.first_act_cpri			= *cpri_ptr;
    mcConfig.second_act_ctagif 		= *tagDecision2_ptr;
    mcConfig.second_act_cvid		= *cvid2_ptr;
    mcConfig.second_act_cpri			= *cpri2_ptr;

    ret = rt_igmp_multicastGroup_set(mcConfig);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("Please config multicast mode as User first, then make sure the portmask should not overlap.\n",ret);
		diag_util_mprintf("set multicast-group failed. RT API return 0x%x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		_diag_multicastGroupShow(&mcConfig);
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rt_igmp_set_multicast_group_ipv6_ip_address_mc_gip_v6_port_mask_portmask_wlan_mbssid_mask0_wlanmbssidmask0_wlan_mbssid_mask1_wlanmbssidmask1_wlan_mbssid_mask2_wlanmbssidmask2_port_mask2_portmask2_sec_wlan_dev_mask0_secwlandevmask0_sec_wlan_dev_mask1_secwlandevmask1_sec_wlan_dev_mask2_secwlandevmask2_action_ctag_tagdecision_tagdecision_cvid_cvid_cpri_cpri_tagdecision2_tagdecision2_cvid2_cvid2_cpri2_cpri2 */


