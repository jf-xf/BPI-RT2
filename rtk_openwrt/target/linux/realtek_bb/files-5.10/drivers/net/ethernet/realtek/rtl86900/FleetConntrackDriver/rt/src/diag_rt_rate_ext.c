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
 * $Revision: 93636 $
 * $Date: 2018-11-16 12:52:37 +0800 (Fri, 16 Nov 2018) $
 *
 * Purpose : Definition of RT API of Rate Extension
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of host policer
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

#include <rt_rate_ext.h>

void _diag_hostPoliceControlShow(rt_rate_host_policer_control_t *policerControl, int idx)
{
    diag_util_mprintf("Host Policer Index: %d\n",idx);
    diag_util_mprintf("MAC=%s\n", diag_util_inet_mactoa(policerControl->mac_addr));

    diag_util_mprintf("Ingress: %s\n",(policerControl->ingress_limit_control==1)?"Enable":"Disable");
	if(policerControl->ingress_limit_control)
	{
    	diag_util_mprintf("\tMeter Index: %d\n",policerControl->ingress_limit_meter_index);
	    diag_util_mprintf("\tMeter Rate: %d\n",policerControl->ingress_limit_rate);
	}

    diag_util_mprintf("Egress: %s\n",(policerControl->egress_limit_control==1)?"Enable":"Disable");
	if(policerControl->egress_limit_control)
	{
    	diag_util_mprintf("\tMeter Index: %d\n",policerControl->egress_limit_meter_index);
    	diag_util_mprintf("\tMeter Rate: %d\n",policerControl->egress_limit_rate);
	}

    diag_util_mprintf("Counter: %s\n",(policerControl->mib_count_control==1)?"Enable":"Disable");
}

/*
 * rt_rate set host-policer <UINT:index> mac <MACADDR:addr> counter_state <UINT:state> limit_state disable
 */
cparser_result_t
cparser_cmd_rt_rate_set_host_policer_index_mac_addr_counter_state_state_limit_state_disable(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    cparser_macaddr_t  *addr_ptr,
    uint32_t  *state_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_rate_host_policer_control_t policerControl;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&policerControl, 0x0, sizeof(rt_rate_host_policer_control_t));
    osal_memcpy(policerControl.mac_addr, addr_ptr->octet, ETHER_ADDR_LEN);
    if(*state_ptr == 0)
		policerControl.mib_count_control = DISABLED;
    else
		policerControl.mib_count_control = ENABLED;

	ret = rt_rate_hostPolicerControl_set(*index_ptr, policerControl);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("set host-policer[%d] failed. RT API return 0x%x \n", *index_ptr, ret);
		return CPARSER_NOT_OK;
	}else{
		ret = rt_rate_hostPolicerControl_get(*index_ptr, &policerControl);
		if(ret != RT_ERR_OK)
			diag_util_mprintf("get host-policer[%d] failed. RT API return 0x%x \n", *index_ptr, ret);
		else
			_diag_hostPoliceControlShow(&policerControl,*index_ptr);
	}

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_rate_set_host_policer_index_mac_addr_counter_state_state_limit_state_disable */

/*
 * rt_rate set host-policer <UINT:index> mac <MACADDR:addr> counter_state <UINT:state> limit_state ingress <UINT:igr_state> ingress_meter <UINT:igr_index> egress <UINT:egr_state> egress_meter <UINT:egr_index>
 */
cparser_result_t
cparser_cmd_rt_rate_set_host_policer_index_mac_addr_counter_state_state_limit_state_ingress_igr_state_ingress_meter_igr_index_egress_egr_state_egress_meter_egr_index(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    cparser_macaddr_t  *addr_ptr,
    uint32_t  *state_ptr,
    uint32_t  *igr_state_ptr,
    uint32_t  *igr_index_ptr,
    uint32_t  *egr_state_ptr,
    uint32_t  *egr_index_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_rate_host_policer_control_t policerControl;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&policerControl, 0x0, sizeof(rt_rate_host_policer_control_t));
	osal_memcpy(policerControl.mac_addr, addr_ptr->octet, ETHER_ADDR_LEN);
    if(*state_ptr == 0)
		policerControl.mib_count_control = DISABLED;
    else
		policerControl.mib_count_control = ENABLED;
    if(*igr_state_ptr == 0)
		policerControl.ingress_limit_control = DISABLED;
    else
		policerControl.ingress_limit_control = ENABLED;
	policerControl.ingress_limit_meter_index = *igr_index_ptr;
    if(*egr_state_ptr == 0)
		policerControl.egress_limit_control = DISABLED;
    else
		policerControl.egress_limit_control = ENABLED;
	policerControl.egress_limit_meter_index = *egr_index_ptr;

	policerControl.ingress_limit_rate = RT_RATE_HOST_METER_RATE_UNCHANGE;
	policerControl.egress_limit_rate = RT_RATE_HOST_METER_RATE_UNCHANGE;

	ret = rt_rate_hostPolicerControl_set(*index_ptr, policerControl);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("set host-policer[%d] failed. RT API return 0x%x \n",*index_ptr,ret);
		return CPARSER_NOT_OK;
	}else{
		ret = rt_rate_hostPolicerControl_get(*index_ptr, &policerControl);
		if(ret != RT_ERR_OK)
			diag_util_mprintf("get host-policer[%d] failed. RT API return 0x%x \n", *index_ptr, ret);
		else
			_diag_hostPoliceControlShow(&policerControl,*index_ptr);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_rate_set_host_policer_index_mac_addr_counter_state_state_limit_state_ingress_igr_state_ingress_meter_igr_index_egress_egr_state_egress_meter_egr_index */

/*
 * rt_rate set host-policer <UINT:index> mac <MACADDR:addr> counter_state <UINT:state> limit_state ingress <UINT:igr_state> ingress_meter <UINT:igr_index> ingress_rate <UINT:igr_rate> egress <UINT:egr_state> egress_meter <UINT:egr_index> egress_rate <UINT:egr_rate>
 */
cparser_result_t
cparser_cmd_rt_rate_set_host_policer_index_mac_addr_counter_state_state_limit_state_ingress_igr_state_ingress_meter_igr_index_ingress_rate_igr_rate_egress_egr_state_egress_meter_egr_index_egress_rate_egr_rate(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    cparser_macaddr_t  *addr_ptr,
    uint32_t  *state_ptr,
    uint32_t  *igr_state_ptr,
    uint32_t  *igr_index_ptr,
    uint32_t  *igr_rate_ptr,
    uint32_t  *egr_state_ptr,
    uint32_t  *egr_index_ptr,
    uint32_t  *egr_rate_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_rate_host_policer_control_t policerControl;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&policerControl, 0x0, sizeof(rt_rate_host_policer_control_t));
	osal_memcpy(policerControl.mac_addr, addr_ptr->octet, ETHER_ADDR_LEN);
	if(*state_ptr == 0)
		policerControl.mib_count_control = DISABLED;
	else
		policerControl.mib_count_control = ENABLED;
	if(*igr_state_ptr == 0)
		policerControl.ingress_limit_control = DISABLED;
	else
		policerControl.ingress_limit_control = ENABLED;
	policerControl.ingress_limit_meter_index = *igr_index_ptr;
	policerControl.ingress_limit_rate = *igr_rate_ptr;
	if(*egr_state_ptr == 0)
		policerControl.egress_limit_control = DISABLED;
	else
		policerControl.egress_limit_control = ENABLED;
	policerControl.egress_limit_meter_index = *egr_index_ptr;
	policerControl.egress_limit_rate = *egr_rate_ptr;

	ret = rt_rate_hostPolicerControl_set(*index_ptr, policerControl);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("set host-policer[%d] failed. RT API return 0x%x \n",*index_ptr,ret);
		return CPARSER_NOT_OK;
	}else{
		ret = rt_rate_hostPolicerControl_get(*index_ptr, &policerControl);
		if(ret != RT_ERR_OK)
			diag_util_mprintf("get host-policer[%d] failed. RT API return 0x%x \n", *index_ptr, ret);
		else
			_diag_hostPoliceControlShow(&policerControl,*index_ptr);
	}

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_rate_set_host_policer_index_mac_addr_counter_state_state_limit_state_ingress_igr_state_ingress_meter_igr_index_ingress_rate_igr_rate_egress_egr_state_egress_meter_egr_index_egress_rate_egr_rate */

/*
 * rt_rate get host-policer <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_rate_get_host_policer_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rt_rate_host_policer_control_t policerControl;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    ret = rt_rate_hostPolicerControl_get(*index_ptr, &policerControl);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("get host-policer[%d] failed. RT API return 0x%x \n",*index_ptr,ret);
		return CPARSER_NOT_OK;
	}else{
		_diag_hostPoliceControlShow(&policerControl,*index_ptr);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_rate_get_host_policer_index */

/*
 * rt_rate get host-policer <UINT:index> counter
 */
cparser_result_t
cparser_cmd_rt_rate_get_host_policer_index_counter(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rt_rate_host_policer_mib_t policerMib;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rt_rate_hostPolicerMib_get(*index_ptr, &policerMib), ret);

    diag_util_mprintf("Host Policer Index: %d\n",*index_ptr);
    diag_util_mprintf("Rx Counter: %llu\n",policerMib.rx_count);
    diag_util_mprintf("Tx Counter: %llu\n",policerMib.tx_count);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_rate_get_host_policer_index_counter */

/*
 * rt_rate clear host-policer <UINT:index> counter
 */
cparser_result_t
cparser_cmd_rt_rate_clear_host_policer_index_counter(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rt_rate_hostPolicerMib_clear(*index_ptr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_rate_clear_host_policer_index_counter */

/*
 * rt_rate set sw-rate-limit arp-request portmask <UINT:portmask> wlan0_dev_mask <UINT:wlan0_dev_mask> wlan1_dev_mask <UINT:wlan1_dev_mask> wlan2_dev_mask <UINT:wlan2_dev_mask> meter <INT:meter_index>
 */
cparser_result_t
cparser_cmd_rt_rate_set_sw_rate_limit_arp_request_portmask_portmask_wlan0_dev_mask_wlan0_dev_mask_wlan1_dev_mask_wlan1_dev_mask_wlan2_dev_mask_wlan2_dev_mask_meter_meter_index(
    cparser_context_t *context,
    uint32_t  *portmask_ptr,
    uint32_t  *wlan0_dev_mask_ptr,
    uint32_t  *wlan1_dev_mask_ptr,
    uint32_t  *wlan2_dev_mask_ptr,
    int32_t  *meter_index_ptr)
{

	int32 ret = RT_ERR_FAILED, i;
	rt_rate_sw_rate_limit_conf_t rateLimit_conf;

    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&rateLimit_conf, 0x0, sizeof(rt_rate_sw_rate_limit_conf_t));
	rateLimit_conf.ingress_port_mask = *portmask_ptr;
	rateLimit_conf.ingress_wlan_dev_mask[0] = *wlan0_dev_mask_ptr;
	rateLimit_conf.ingress_wlan_dev_mask[1] = *wlan1_dev_mask_ptr;
	rateLimit_conf.ingress_wlan_dev_mask[2] = *wlan2_dev_mask_ptr;
	rateLimit_conf.shareMeterIdx = *meter_index_ptr;

	ret = rt_rate_sw_rate_limit_set(RT_RATE_SW_RATE_LIMIT_TYPE_ARP_REQ, rateLimit_conf);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("Fail to set sw-rate-limit of arp request. RT API return 0x%x \n", ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set sw-rate-limit of arp request successfully: \n");
		diag_util_mprintf("\tport_mask=0x%x ", rateLimit_conf.ingress_port_mask);
		for(i=0; i<RT_WLAN_DEVICE_MAX; i++)
			diag_util_mprintf("wlan%d_dev_mask=0x%x ", i, rateLimit_conf.ingress_wlan_dev_mask[i]);
		diag_util_mprintf("meter_idx=%d \n", rateLimit_conf.shareMeterIdx);
	}

    return CPARSER_OK;
}	 /* end of cparser_cmd_rt_rate_set_sw_rate_limit_arp_request_portmask_portmask_wlan0_dev_mask_wlan0_dev_mask_wlan1_dev_mask_wlan1_dev_mask_wlan2_dev_mask_wlan2_dev_mask_meter_meter_index */

/*
 * rt_rate get sw-rate-limit arp-request
 */
cparser_result_t
cparser_cmd_rt_rate_get_sw_rate_limit_arp_request(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED, i;
	rt_rate_sw_rate_limit_conf_t rateLimit_conf;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&rateLimit_conf, 0x0, sizeof(rt_rate_sw_rate_limit_conf_t));

	ret = rt_rate_sw_rate_limit_get(RT_RATE_SW_RATE_LIMIT_TYPE_ARP_REQ, &rateLimit_conf);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("Fail to get sw-rate-limit of arp request. RT API return 0x%x \n", ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Configuration of sw-rate-limit of arp request: \n");
		diag_util_mprintf("\tport_mask=0x%x ", rateLimit_conf.ingress_port_mask);
		for(i=0; i<RT_WLAN_DEVICE_MAX; i++)
			diag_util_mprintf("wlan%d_dev_mask=0x%x ", i, rateLimit_conf.ingress_wlan_dev_mask[i]);
		diag_util_mprintf("meter_idx=%d \n", rateLimit_conf.shareMeterIdx);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_rate_get_sw_rate_limit_arp_request */

/*
 * rt_rate set sw-rate-limit specific-dos portmask <UINT:portmask> wlan0_dev_mask <UINT:wlan0_dev_mask> wlan1_dev_mask <UINT:wlan1_dev_mask> wlan2_dev_mask <UINT:wlan2_dev_mask> ingress_ctagif <UINT:ingress_ctagif> ingress_cvid <UINT:ingress_cvid> ingress_dmac <MACADDR:ingress_dmac> ingress_dip <IPV4ADDR:ingress_dip> isTCP <UINT:isTCP> packet_length_start <UINT:packet_length_start> packet_length_end <UINT:packet_length_end> meter <INT:meter_index>
 */
cparser_result_t
cparser_cmd_rt_rate_set_sw_rate_limit_specific_dos_portmask_portmask_wlan0_dev_mask_wlan0_dev_mask_wlan1_dev_mask_wlan1_dev_mask_wlan2_dev_mask_wlan2_dev_mask_ingress_ctagif_ingress_ctagif_ingress_cvid_ingress_cvid_ingress_dmac_ingress_dmac_ingress_dip_ingress_dip_isTCP_isTCP_packet_length_start_packet_length_start_packet_length_end_packet_length_end_meter_meter_index(
    cparser_context_t *context,
    uint32_t  *portmask_ptr,
    uint32_t  *wlan0_dev_mask_ptr,
    uint32_t  *wlan1_dev_mask_ptr,
    uint32_t  *wlan2_dev_mask_ptr,
    uint32_t  *ingress_ctagif_ptr,
    uint32_t  *ingress_cvid_ptr,
    cparser_macaddr_t  *ingress_dmac_ptr,
    uint32_t  *ingress_dip_ptr,
    uint32_t  *isTCP_ptr,
    uint32_t  *packet_length_start_ptr,
    uint32_t  *packet_length_end_ptr,
    int32_t  *meter_index_ptr)
{

	int32 ret = RT_ERR_FAILED, i;
	rt_rate_sw_rate_limit_conf_t rateLimit_conf;

    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&rateLimit_conf, 0x0, sizeof(rt_rate_sw_rate_limit_conf_t));
	rateLimit_conf.ingress_port_mask = *portmask_ptr;
	rateLimit_conf.ingress_wlan_dev_mask[0] = *wlan0_dev_mask_ptr;
	rateLimit_conf.ingress_wlan_dev_mask[1] = *wlan1_dev_mask_ptr;
	rateLimit_conf.ingress_wlan_dev_mask[2] = *wlan2_dev_mask_ptr;
	rateLimit_conf.shareMeterIdx = *meter_index_ptr;
	rateLimit_conf.specific_dos.ctagif = *ingress_ctagif_ptr;
	rateLimit_conf.specific_dos.ctagVid = *ingress_cvid_ptr;
	osal_memcpy(rateLimit_conf.specific_dos.ingressDmac, ingress_dmac_ptr->octet, ETHER_ADDR_LEN);
	rateLimit_conf.specific_dos.ingressDip = *ingress_dip_ptr;
	rateLimit_conf.specific_dos.isTcp = *isTCP_ptr;
	rateLimit_conf.specific_dos.pktLenStart = *packet_length_start_ptr;
	rateLimit_conf.specific_dos.pktLenEnd = *packet_length_end_ptr;

	ret = rt_rate_sw_rate_limit_set(RT_RATE_SW_RATE_LIMIT_TYPE_SPECIFIC_DOS, rateLimit_conf);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("Fail to set sw-rate-limit of specific dos. RT API return 0x%x \n", ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set sw-rate-limit of specific dos successfully: \n");
		diag_util_mprintf("\tport_mask=0x%x ", rateLimit_conf.ingress_port_mask);
		for(i=0; i<RT_WLAN_DEVICE_MAX; i++)
			diag_util_mprintf("wlan%d_dev_mask=0x%x ", i, rateLimit_conf.ingress_wlan_dev_mask[i]);
		diag_util_mprintf("meter_idx=%d \n", rateLimit_conf.shareMeterIdx);
		diag_util_mprintf("\tingress_ctagif=%d ingress_cvid=%d ingress_dmac=%02x:%02x:%02x:%02x:%02x:%02x \n", rateLimit_conf.specific_dos.ctagif, rateLimit_conf.specific_dos.ctagVid, rateLimit_conf.specific_dos.ingressDmac[0], rateLimit_conf.specific_dos.ingressDmac[1], rateLimit_conf.specific_dos.ingressDmac[2], rateLimit_conf.specific_dos.ingressDmac[3], rateLimit_conf.specific_dos.ingressDmac[4], rateLimit_conf.specific_dos.ingressDmac[5]);
		diag_util_mprintf("\tingress_dip=%d.%d.%d.%d isTCP=%d \n", (rateLimit_conf.specific_dos.ingressDip>>24)&0xff, (rateLimit_conf.specific_dos.ingressDip>>16)&0xff, (rateLimit_conf.specific_dos.ingressDip>>8)&0xff, (rateLimit_conf.specific_dos.ingressDip)&0xff, rateLimit_conf.specific_dos.isTcp);
		diag_util_mprintf("\tpacket_length_start=%d packet_length_end=%d \n", rateLimit_conf.specific_dos.pktLenStart, rateLimit_conf.specific_dos.pktLenEnd);
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rt_rate_set_sw_rate_limit_specific_dos_portmask_portmask_wlan0_dev_mask_wlan0_dev_mask_wlan1_dev_mask_wlan1_dev_mask_wlan2_dev_mask_wlan2_dev_mask_ingress_ctagif_ingress_ctagif_ingress_cvid_ingress_cvid_ingress_dmac_ingress_dmac_ingress_dip_ingress_dip_istcp_istcp_packet_length_start_packet_length_start_packet_length_end_packet_length_end_meter_meter_index */

/*
 * rt_rate get sw-rate-limit specific-dos
 */
cparser_result_t
cparser_cmd_rt_rate_get_sw_rate_limit_specific_dos(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED, i;
	rt_rate_sw_rate_limit_conf_t rateLimit_conf;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&rateLimit_conf, 0x0, sizeof(rt_rate_sw_rate_limit_conf_t));

	ret = rt_rate_sw_rate_limit_get(RT_RATE_SW_RATE_LIMIT_TYPE_SPECIFIC_DOS, &rateLimit_conf);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("Fail to get sw-rate-limit of specific dos. RT API return 0x%x \n", ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Configuration of sw-rate-limit of specific dos: \n");
		diag_util_mprintf("\tport_mask=0x%x ", rateLimit_conf.ingress_port_mask);
		for(i=0; i<RT_WLAN_DEVICE_MAX; i++)
			diag_util_mprintf("wlan%d_dev_mask=0x%x ", i, rateLimit_conf.ingress_wlan_dev_mask[i]);
		diag_util_mprintf("meter_idx=%d \n", rateLimit_conf.shareMeterIdx);
		diag_util_mprintf("\tingress_ctagif=%d ingress_cvid=%d ingress_dmac=%02x:%02x:%02x:%02x:%02x:%02x \n", rateLimit_conf.specific_dos.ctagif, rateLimit_conf.specific_dos.ctagVid, rateLimit_conf.specific_dos.ingressDmac[0], rateLimit_conf.specific_dos.ingressDmac[1], rateLimit_conf.specific_dos.ingressDmac[2], rateLimit_conf.specific_dos.ingressDmac[3], rateLimit_conf.specific_dos.ingressDmac[4], rateLimit_conf.specific_dos.ingressDmac[5]);
		diag_util_mprintf("\tingress_dip=%d.%d.%d.%d isTCP=%d \n", (rateLimit_conf.specific_dos.ingressDip>>24)&0xff, (rateLimit_conf.specific_dos.ingressDip>>16)&0xff, (rateLimit_conf.specific_dos.ingressDip>>8)&0xff, (rateLimit_conf.specific_dos.ingressDip)&0xff, rateLimit_conf.specific_dos.isTcp);
		diag_util_mprintf("\tpacket_length_start=%d packet_length_end=%d \n", rateLimit_conf.specific_dos.pktLenStart, rateLimit_conf.specific_dos.pktLenEnd);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_rate_get_sw_rate_limit_specific_dos */

/*
 * rt_rate set sw-rate-limit tcp-syn-only portmask <UINT:portmask> wlan0_dev_mask <UINT:wlan0_dev_mask> wlan1_dev_mask <UINT:wlan1_dev_mask> wlan2_dev_mask <UINT:wlan2_dev_mask> meter <INT:meter_index>
 */
cparser_result_t
cparser_cmd_rt_rate_set_sw_rate_limit_tcp_syn_only_portmask_portmask_wlan0_dev_mask_wlan0_dev_mask_wlan1_dev_mask_wlan1_dev_mask_wlan2_dev_mask_wlan2_dev_mask_meter_meter_index(
    cparser_context_t *context,
    uint32_t  *portmask_ptr,
    uint32_t  *wlan0_dev_mask_ptr,
    uint32_t  *wlan1_dev_mask_ptr,
    uint32_t  *wlan2_dev_mask_ptr,
    int32_t  *meter_index_ptr)
{

	int32 ret = RT_ERR_FAILED, i;
	rt_rate_sw_rate_limit_conf_t rateLimit_conf;

    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&rateLimit_conf, 0x0, sizeof(rt_rate_sw_rate_limit_conf_t));
	rateLimit_conf.ingress_port_mask = *portmask_ptr;
	rateLimit_conf.ingress_wlan_dev_mask[0] = *wlan0_dev_mask_ptr;
	rateLimit_conf.ingress_wlan_dev_mask[1] = *wlan1_dev_mask_ptr;
	rateLimit_conf.ingress_wlan_dev_mask[2] = *wlan2_dev_mask_ptr;
	rateLimit_conf.shareMeterIdx = *meter_index_ptr;

	ret = rt_rate_sw_rate_limit_set(RT_RATE_SW_RATE_LIMIT_TYPE_TCP_SYN_ONLY, rateLimit_conf);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("Fail to set sw-rate-limit of tcp syn only. RT API return 0x%x \n", ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set sw-rate-limit of tcp syn only successfully: \n");
		diag_util_mprintf("\tport_mask=0x%x ", rateLimit_conf.ingress_port_mask);
		for(i=0; i<RT_WLAN_DEVICE_MAX; i++)
			diag_util_mprintf("wlan%d_dev_mask=0x%x ", i, rateLimit_conf.ingress_wlan_dev_mask[i]);
		diag_util_mprintf("meter_idx=%d \n", rateLimit_conf.shareMeterIdx);
	}

    return CPARSER_OK;
}	 /* end of cparser_cmd_rt_rate_set_sw_rate_limit_tcp_syn_only_portmask_portmask_wlan0_dev_mask_wlan0_dev_mask_wlan1_dev_mask_wlan1_dev_mask_wlan2_dev_mask_wlan2_dev_mask_meter_meter_index */

/*
 * rt_rate get sw-rate-limit tcp-syn-only */
cparser_result_t
cparser_cmd_rt_rate_get_sw_rate_limit_tcp_syn_only(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED, i;
	rt_rate_sw_rate_limit_conf_t rateLimit_conf;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&rateLimit_conf, 0x0, sizeof(rt_rate_sw_rate_limit_conf_t));

	ret = rt_rate_sw_rate_limit_get(RT_RATE_SW_RATE_LIMIT_TYPE_TCP_SYN_ONLY, &rateLimit_conf);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("Fail to get sw-rate-limit of tcp syn only. RT API return 0x%x \n", ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Configuration of sw-rate-limit of tcp syn only: \n");
		diag_util_mprintf("\tport_mask=0x%x ", rateLimit_conf.ingress_port_mask);
		for(i=0; i<RT_WLAN_DEVICE_MAX; i++)
			diag_util_mprintf("wlan%d_dev_mask=0x%x ", i, rateLimit_conf.ingress_wlan_dev_mask[i]);
		diag_util_mprintf("meter_idx=%d \n", rateLimit_conf.shareMeterIdx);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_rate_get_sw_rate_limit_tcp_syn_only */


/*
 * rt_rate set sw-rate-limit unknown-unicast portmask <UINT:portmask> wlan0_dev_mask <UINT:wlan0_dev_mask> wlan1_dev_mask <UINT:wlan1_dev_mask> wlan2_dev_mask <UINT:wlan2_dev_mask> meter <INT:meter_index>
 */
cparser_result_t
cparser_cmd_rt_rate_set_sw_rate_limit_unknown_unicast_portmask_portmask_wlan0_dev_mask_wlan0_dev_mask_wlan1_dev_mask_wlan1_dev_mask_wlan2_dev_mask_wlan2_dev_mask_meter_meter_index(
    cparser_context_t *context,
    uint32_t  *portmask_ptr,
    uint32_t  *wlan0_dev_mask_ptr,
    uint32_t  *wlan1_dev_mask_ptr,
    uint32_t  *wlan2_dev_mask_ptr,
    int32_t  *meter_index_ptr)
{
	int32 ret = RT_ERR_FAILED, i;
	rt_rate_sw_rate_limit_conf_t rateLimit_conf;

    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&rateLimit_conf, 0x0, sizeof(rt_rate_sw_rate_limit_conf_t));
	rateLimit_conf.ingress_port_mask = *portmask_ptr;
	rateLimit_conf.ingress_wlan_dev_mask[0] = *wlan0_dev_mask_ptr;
	rateLimit_conf.ingress_wlan_dev_mask[1] = *wlan1_dev_mask_ptr;
	rateLimit_conf.ingress_wlan_dev_mask[2] = *wlan2_dev_mask_ptr;
	rateLimit_conf.shareMeterIdx = *meter_index_ptr;

	ret = rt_rate_sw_rate_limit_set(RT_RATE_SW_RATE_LIMIT_TYPE_UNKNOWN_UC, rateLimit_conf);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("Fail to set sw-rate-limit of unknown unicast. RT API return 0x%x \n", ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set sw-rate-limit of unknown unicast successfully: \n");
		diag_util_mprintf("\tport_mask=0x%x ", rateLimit_conf.ingress_port_mask);
		for(i=0; i<RT_WLAN_DEVICE_MAX; i++)
			diag_util_mprintf("wlan%d_dev_mask=0x%x ", i, rateLimit_conf.ingress_wlan_dev_mask[i]);
		diag_util_mprintf("meter_idx=%d \n", rateLimit_conf.shareMeterIdx);
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rt_rate_set_sw_rate_limit_unknow_unicast_portmask_portmask_wlan0_dev_mask_wlan0_dev_mask_wlan1_dev_mask_wlan1_dev_mask_wlan2_dev_mask_wlan2_dev_mask_meter_meter_index */

/*
 * rt_rate get sw-rate-limit unknown-unicast */
cparser_result_t
cparser_cmd_rt_rate_get_sw_rate_limit_unknown_unicast(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED, i;
	rt_rate_sw_rate_limit_conf_t rateLimit_conf;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&rateLimit_conf, 0x0, sizeof(rt_rate_sw_rate_limit_conf_t));

	ret = rt_rate_sw_rate_limit_get(RT_RATE_SW_RATE_LIMIT_TYPE_UNKNOWN_UC, &rateLimit_conf);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("Fail to get sw-rate-limit of unknown unicast. RT API return 0x%x \n", ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Configuration of sw-rate-limit of unknown unicast: \n");
		diag_util_mprintf("\tport_mask=0x%x ", rateLimit_conf.ingress_port_mask);
		for(i=0; i<RT_WLAN_DEVICE_MAX; i++)
			diag_util_mprintf("wlan%d_dev_mask=0x%x ", i, rateLimit_conf.ingress_wlan_dev_mask[i]);
		diag_util_mprintf("meter_idx=%d \n", rateLimit_conf.shareMeterIdx);
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rt_rate_get_sw_rate_limit_unknow_unicast */



