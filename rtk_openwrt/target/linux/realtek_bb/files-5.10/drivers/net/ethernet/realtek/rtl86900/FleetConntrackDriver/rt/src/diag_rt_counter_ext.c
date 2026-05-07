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
#include <rtk/stat.h>

#include <rt_stat_ext.h>

/*
 * rt_mib get counter flow_cache_mib <UINT:flow_idx>
 */
cparser_result_t
cparser_cmd_rt_mib_get_counter_flow_cache_mib_flow_idx(
    cparser_context_t *context,
    uint32_t  *flow_idx_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rt_stat_flowCache_mib_t flowCacheMib;

	if(*flow_idx_ptr >= RT_STAT_FLOW_TABLE_SIZE)
	{
		diag_util_mprintf("Fail to get flow cache mib ... flow_idx[%d] is out of range(0~%d) !! \n", *flow_idx_ptr, RT_STAT_FLOW_TABLE_SIZE-1);
		return CPARSER_NOT_OK;
	}
	
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rt_stat_flowCache_mib_get(*flow_idx_ptr, &flowCacheMib), ret);

    diag_util_mprintf("Flow_Cache_Mib of flow index[%d]\n",*flow_idx_ptr);
    diag_util_mprintf("packet count = %-7llu, byte count = %-7llu\n", flowCacheMib.packet_count, flowCacheMib.byte_count);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_mib_get_counter_flow_cache_mib_flow_idx */

/*
 * rt_mib reset counter flow_cache_mib <UINT:flow_idx>
 */
cparser_result_t
cparser_cmd_rt_mib_reset_counter_flow_cache_mib_flow_idx(
    cparser_context_t *context,
    uint32_t  *flow_idx_ptr)
{
    int32 ret = RT_ERR_FAILED;

	if(*flow_idx_ptr >= RT_STAT_FLOW_TABLE_SIZE)
	{
		diag_util_mprintf("Fail to get flow cache mib ... flow_idx[%d] is out of range(0~%d) !! \n", *flow_idx_ptr, RT_STAT_FLOW_TABLE_SIZE-1);
		return CPARSER_NOT_OK;
	}
	
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rt_stat_flowCache_mib_reset(*flow_idx_ptr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_mib_reset_counter_flow_cache_mib_flow_idx */


/*
 * rt_mib get counter flow <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_mib_get_counter_flow_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rt_stat_flow_mib_t flowMib;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rt_stat_flowMib_get(*index_ptr, &flowMib), ret);

    diag_util_mprintf("FlowMib[%d]\n",*index_ptr);
	diag_util_mprintf("ingress:  packet count = %-6u, byte count = %-7llu\n", flowMib.ingress_packet_count, flowMib.ingress_byte_count);
    diag_util_mprintf("egress :  packet count = %-6u, byte count = %-7llu\n", flowMib.egress_packet_count, flowMib.egress_byte_count);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_mib_get_flow_index */

/*
 * rt_mib reset counter flow <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_mib_reset_counter_flow_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rt_stat_flowMib_reset(*index_ptr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_mib_reset_counter_flow_index */

/*
 * rt_mib get counter netif <STRING:dev_name>
 */
cparser_result_t
cparser_cmd_rt_mib_get_counter_netif_dev_name(
    cparser_context_t *context,
    char * *dev_name_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_stat_netif_mib_t netifMib;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rt_stat_netifMib_get(*dev_name_ptr, &netifMib), ret);

    diag_util_mprintf("Netif Device Name\t: %s\n",*dev_name_ptr);
	diag_util_mprintf("ingress:  packet count = %-6u, byte count = %-7llu\n", netifMib.ingress_packet_count, netifMib.ingress_byte_count);
    diag_util_mprintf("egress :  packet count = %-6u, byte count = %-7llu\n", netifMib.egress_packet_count, netifMib.egress_byte_count);
	if(netifMib.ucCntValid)
	{
		diag_util_mprintf("ingress: UC packet count = %-6u, byte count = %-7llu\n", netifMib.ingress_uc_packet_count, netifMib.ingress_uc_byte_count);
		diag_util_mprintf("egress : UC packet count = %-6u, byte count = %-7llu\n", netifMib.egress_uc_packet_count, netifMib.egress_uc_byte_count);
	}
	if(netifMib.mcCntValid)
	{
		diag_util_mprintf("ingress: MC packet count = %-6u, byte count = %-7llu\n", netifMib.ingress_mc_packet_count, netifMib.ingress_mc_byte_count);
		diag_util_mprintf("egress : MC packet count = %-6u, byte count = %-7llu\n", netifMib.egress_mc_packet_count, netifMib.egress_mc_byte_count);
	}
	if(netifMib.bcCntValid)
	{
		diag_util_mprintf("ingress: BC packet count = %-6u, byte count = %-7llu\n", netifMib.ingress_bc_packet_count, netifMib.ingress_bc_byte_count);
		diag_util_mprintf("egress : BC packet count = %-6u, byte count = %-7llu\n", netifMib.egress_bc_packet_count, netifMib.egress_bc_byte_count);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_mib_get_netif_dev_name */

/*
 * rt_mib reset counter netif <STRING:dev_name>
 */
cparser_result_t
cparser_cmd_rt_mib_reset_counter_netif_dev_name(
    cparser_context_t *context,
    char * *dev_name_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rt_stat_netifMib_reset(*dev_name_ptr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_mib_reset_counter_netif_dev_name */


/*
 * rt_mib get counter acl <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_mib_get_counter_acl_index(
	cparser_context_t *context,
	uint32_t  *index_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_stat_acl_mib_t aclMib;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	DIAG_UTIL_ERR_CHK(rt_stat_aclMib_get(*index_ptr, &aclMib), ret);

	if(aclMib.type == 0)
		diag_util_mprintf("ACL Mib[%d]: byte count = %-7llu\n",*index_ptr, aclMib.byte_count);
	else
		diag_util_mprintf("ACL Mib[%d]: packet count = %-6u\n",*index_ptr, aclMib.packet_count);

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_mib_get_counter_acl_index */

/*
 * rt_mib set acl index <UINT:index> type <UINT:type>
 */
cparser_result_t
cparser_cmd_rt_mib_set_acl_index_index_type_type(
	cparser_context_t *context,
	uint32_t  *index_ptr,
	uint32_t  *type_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_stat_acl_mib_t aclMib={0};

	DIAG_UTIL_PARAM_CHK();
	if(*type_ptr == 0)
		aclMib.type = 0;
	else
		aclMib.type = 1;

	DIAG_UTIL_ERR_CHK(rt_stat_aclMib_set(*index_ptr, aclMib), ret);

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_mib_set_acl_index_index_type_type */

/*
 * rt_mib reset counter acl <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_mib_reset_counter_acl_index(
	cparser_context_t *context,
	uint32_t  *index_ptr)
{
	int32 ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_ERR_CHK(rt_stat_aclMib_reset(*index_ptr), ret);

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_mib_reset_counter_acl_index */


/*
 * rt_mib get counter gem-filter <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_mib_get_counter_gem_filter_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rt_stat_port_cntr_t gemMib;
    int32 type = 0, show = 0;
    uint64 cntr;
	
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rt_stat_gemFilterMib_get(*index_ptr, &gemMib), ret);

    for (type = 0; type < RT_MIB_PORT_CNTR_END; type++)
    {
        switch(type)
        {
            case DOT1D_TP_PORT_IN_DISCARDS_INDEX:
                cntr = gemMib.ifInDiscards;
                break;
            case IF_IN_OCTETS_INDEX:
                cntr = gemMib.ifInOctets;
                break;
            case IF_IN_UCAST_PKTS_INDEX:
                cntr = gemMib.ifInUcastPkts;
                break;
            case IF_IN_MULTICAST_PKTS_INDEX:
                cntr = gemMib.ifInMulticastPkts;
                break;
            case IF_IN_BROADCAST_PKTS_INDEX:
                cntr = gemMib.ifInBroadcastPkts;
                break;
            case IF_OUT_OCTETS_INDEX:
                cntr = gemMib.ifOutOctets;
                break;
            case IF_OUT_UCAST_PKTS_CNT_INDEX:
                cntr = gemMib.ifOutUcastPkts;
                break;
            case IF_OUT_MULTICAST_PKTS_CNT_INDEX:
                cntr = gemMib.ifOutMulticastPkts;
                break;
            case IF_OUT_BROADCAST_PKTS_CNT_INDEX:
                cntr = gemMib.ifOutBrocastPkts;
                break;
            case IF_OUT_DISCARDS_INDEX:
                cntr = gemMib.ifOutDiscards;
                break;
            case DOT3_STATS_SINGLE_COLLISION_FRAMES_INDEX:
                cntr = gemMib.dot3StatsSingleCollisionFrames;
                break;
            case DOT3_STATS_MULTIPLE_COLLISION_FRAMES_INDEX:
                cntr = gemMib.dot3StatsMultipleCollisionFrames;
                break;
            case DOT3_STATS_DEFERRED_TRANSMISSIONS_INDEX:
                cntr = gemMib.dot3StatsDeferredTransmissions;
                break;
            case DOT3_STATS_LATE_COLLISIONS_INDEX:
                cntr = gemMib.dot3StatsLateCollisions;
                break;
            case DOT3_STATS_EXCESSIVE_COLLISIONS_INDEX:
                cntr = gemMib.dot3StatsExcessiveCollisions;
                break;
            case DOT3_STATS_SYMBOL_ERRORS_INDEX:
                cntr = gemMib.dot3StatsSymbolErrors;
                break;
            case DOT3_CONTROL_IN_UNKNOWN_OPCODES_INDEX:
                cntr = gemMib.dot3ControlInUnknownOpcodes;
                break;
            case DOT3_IN_PAUSE_FRAMES_INDEX:
                cntr = gemMib.dot3InPauseFrames;
                break;
            case DOT3_OUT_PAUSE_FRAMES_INDEX:
                cntr = gemMib.dot3OutPauseFrames;
                break;
            case ETHER_STATS_DROP_EVENTS_INDEX:
                cntr = gemMib.etherStatsDropEvents;
                break;
            case ETHER_STATS_TX_BROADCAST_PKTS_INDEX:
                cntr = gemMib.etherStatsTxBcastPkts;
                break;
            case ETHER_STATS_TX_MULTICAST_PKTS_INDEX:
                cntr = gemMib.etherStatsTxMcastPkts;
                break;
            case ETHER_STATS_CRC_ALIGN_ERRORS_INDEX:
                cntr = gemMib.etherStatsCRCAlignErrors;
                break;
            case ETHER_STATS_TX_UNDER_SIZE_PKTS_INDEX:
                cntr = gemMib.etherStatsTxUndersizePkts;
                break;
            case ETHER_STATS_RX_UNDER_SIZE_PKTS_INDEX:
                cntr = gemMib.etherStatsRxUndersizePkts;
                break;
            case ETHER_STATS_TX_OVERSIZE_PKTS_INDEX:
                cntr = gemMib.etherStatsTxOversizePkts;
                break;
            case ETHER_STATS_RX_OVERSIZE_PKTS_INDEX:
                cntr = gemMib.etherStatsRxOversizePkts;
                break;
            case ETHER_STATS_FRAGMENTS_INDEX:
                cntr = gemMib.etherStatsFragments;
                break;
            case ETHER_STATS_JABBERS_INDEX:
                cntr = gemMib.etherStatsJabbers;
                break;
            case ETHER_STATS_COLLISIONS_INDEX:
                cntr = gemMib.etherStatsCollisions;
                break;
            case ETHER_STATS_TX_PKTS_64OCTETS_INDEX:
                cntr = gemMib.etherStatsTxPkts64Octets;
                break;
            case ETHER_STATS_RX_PKTS_64OCTETS_INDEX:
                cntr = gemMib.etherStatsRxPkts64Octets;
                break;
            case ETHER_STATS_TX_PKTS_65TO127OCTETS_INDEX:
                cntr = gemMib.etherStatsTxPkts65to127Octets;
                break;
            case ETHER_STATS_RX_PKTS_65TO127OCTETS_INDEX:
                cntr = gemMib.etherStatsRxPkts65to127Octets;
                break;
            case ETHER_STATS_TX_PKTS_128TO255OCTETS_INDEX:
                cntr = gemMib.etherStatsTxPkts128to255Octets;
                break;
            case ETHER_STATS_RX_PKTS_128TO255OCTETS_INDEX:
                cntr = gemMib.etherStatsRxPkts128to255Octets;
                break;
            case ETHER_STATS_TX_PKTS_256TO511OCTETS_INDEX:
                cntr = gemMib.etherStatsTxPkts256to511Octets;
                break;
            case ETHER_STATS_RX_PKTS_256TO511OCTETS_INDEX:
                cntr = gemMib.etherStatsRxPkts256to511Octets;
                break;
            case ETHER_STATS_TX_PKTS_512TO1023OCTETS_INDEX:
                cntr = gemMib.etherStatsTxPkts512to1023Octets;
                break;
            case ETHER_STATS_RX_PKTS_512TO1023OCTETS_INDEX:
                cntr = gemMib.etherStatsRxPkts512to1023Octets;
                break;
            case ETHER_STATS_TX_PKTS_1024TO1518OCTETS_INDEX:
                cntr = gemMib.etherStatsTxPkts1024to1518Octets;
                break;
            case ETHER_STATS_RX_PKTS_1024TO1518OCTETS_INDEX:
                cntr = gemMib.etherStatsRxPkts1024to1518Octets;
                break;
            case ETHER_STATS_TX_PKTS_1519TOMAXOCTETS_INDEX:
                cntr = gemMib.etherStatsTxPkts1519toMaxOctets;
                break;
            case ETHER_STATS_RX_PKTS_1519TOMAXOCTETS_INDEX:
                cntr = gemMib.etherStatsRxPkts1519toMaxOctets;
                break;
            case IN_OAM_PDU_PKTS_INDEX:
                cntr = gemMib.inOampduPkts;
                break;
            case OUT_OAM_PDU_PKTS_INDEX:
                cntr = gemMib.outOampduPkts;
                break;
            default:
		  cntr = 0;
                break;
        }
     
        if(cntr == 0)
            continue;

        show = 1;
        
        diag_util_printf("%-35s: ",diagStr_mibName[type]);
        diag_util_printf("%25llu\n", cntr);
    }

    if(show)
	 diag_util_printf("\n");
    else
        diag_util_printf("  -- all zero --\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_mib_get_counter_gem_filter_index */

/*
 * rt_mib reset counter gem-filter <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_mib_reset_counter_gem_filter_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
	
    DIAG_UTIL_PARAM_CHK();
	
    DIAG_UTIL_ERR_CHK(rt_stat_gemFilterMib_reset(*index_ptr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_mib_reset_counter_gem_filter_index */

/*
 * rt_mib set gem-filter index <UINT:index> state <UINT:state>
 */
cparser_result_t
cparser_cmd_rt_mib_set_gem_filter_index_index_state_state(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *state_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rt_stat_gemFilter_conf_t gemFilter_conf;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rt_stat_gemFilter_get(*index_ptr, &gemFilter_conf), ret);
    gemFilter_conf.enable = !!(*state_ptr);
    DIAG_UTIL_ERR_CHK(rt_stat_gemFilter_set(*index_ptr, gemFilter_conf), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_mib_set_gem_filter_index_index_state_state */

/*
 * rt_mib set gem-filter index <UINT:index> state <UINT:state> type <UINT:type> id <UINT:id>
 */
cparser_result_t
cparser_cmd_rt_mib_set_gem_filter_index_index_state_state_type_type_id_id(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *state_ptr,
    uint32_t  *type_ptr,
    uint32_t  *id_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rt_stat_gemFilter_conf_t gemFilter_conf={0};
	
    DIAG_UTIL_PARAM_CHK();
	
    gemFilter_conf.enable = !!(*state_ptr);
    gemFilter_conf.type = *type_ptr;
    gemFilter_conf.type_portId = *id_ptr;
    gemFilter_conf.filter_vid = RT_STAT_GEMMIB_FILTER_VID_INVAILD;
    gemFilter_conf.filter_pri = RT_STAT_GEMMIB_FILTER_PRI_INVAILD;
    DIAG_UTIL_ERR_CHK(rt_stat_gemFilter_set(*index_ptr, gemFilter_conf), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_mib_set_gem_filter_index_index_state_state_type_type_id_id */

/*
 * rt_mib set gem-filter index <UINT:index> state <UINT:state> type <UINT:type> id <UINT:id> filter-vid <UINT:vid> filter-pri <UINT:pri>
 */
cparser_result_t
cparser_cmd_rt_mib_set_gem_filter_index_index_state_state_type_type_id_id_filter_vid_vid_filter_pri_pri_filter_pkttype_pkttype(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *state_ptr,
    uint32_t  *type_ptr,
    uint32_t  *id_ptr,
    uint32_t  *vid_ptr,
    uint32_t  *pri_ptr,
    uint32_t  *pkttype_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rt_stat_gemFilter_conf_t gemFilter_conf;
	
    DIAG_UTIL_PARAM_CHK();
	
    gemFilter_conf.enable = !!(*state_ptr);
    gemFilter_conf.type = *type_ptr;
    gemFilter_conf.type_portId= *id_ptr;
    gemFilter_conf.filter_vid = *vid_ptr;
    gemFilter_conf.filter_pri = *pri_ptr;
    gemFilter_conf.filter_pri = *pri_ptr;
    gemFilter_conf.filter_pkttype = *pkttype_ptr;
    DIAG_UTIL_ERR_CHK(rt_stat_gemFilter_set(*index_ptr, gemFilter_conf), ret);
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_mib_set_gem_filter_index_index_state_state_type_type_id_id_filter_vid_vid_filter_pri_pri */

/*
 * rt_mib get gem-filter index <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_mib_get_gem_filter_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rt_stat_gemFilter_conf_t gemFilter_conf;
	
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
	
    DIAG_UTIL_ERR_CHK(rt_stat_gemFilter_get(*index_ptr, &gemFilter_conf), ret);

    diag_util_mprintf("  %-15s: %s \n", "state", gemFilter_conf.enable ? "enable" : "disable");
    if(gemFilter_conf.enable) {
		diag_util_mprintf("  %-15s: %s = %d\n", "filter", gemFilter_conf.type == RT_STAT_FILTER_FLOW ? "flow/sid" : "port", gemFilter_conf.type_portId);
		
		if(gemFilter_conf.filter_vid == RT_STAT_GEMMIB_FILTER_VID_INVAILD)
			diag_util_mprintf("  %-15s: disable\n", "filter vlan id");
		else
			diag_util_mprintf("  %-15s: %d\n", "filter vlan id", gemFilter_conf.filter_vid);
		if(gemFilter_conf.filter_pri == RT_STAT_GEMMIB_FILTER_PRI_INVAILD)
			diag_util_mprintf("  %-15s: disable\n", "filter vlan pri");
		else
			diag_util_mprintf("  %-15s: %d\n", "filter vlan pri", gemFilter_conf.filter_pri);
		
		if(gemFilter_conf.filter_pkttype == RT_STAT_FILTER_PKTTYPE_DISABLE)
			diag_util_mprintf("  %-15s: disable\n", "filter pkt type");
		else
			diag_util_mprintf("  %-15s: %d (%s)\n", "filter pkt type", 
				gemFilter_conf.filter_pkttype, (gemFilter_conf.filter_pkttype==RT_STAT_FILTER_PKTTYPE_UC_ONLY) ? "UC" : ( (gemFilter_conf.filter_pkttype==RT_STAT_FILTER_PKTTYPE_MC_ONLY) ? "MC" : "BC"));
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_mib_get_gem_filter_index_index */

