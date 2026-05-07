#include <stdio.h>
#include <string.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <diag_str.h>
#include <parser/cparser_priv.h>

#include <rt_flow_ext.h>

static rt_flow_ops_data_t delFlowkey;
static rt_flow_ops_data_t hiPriFlowPtn;

static char *hwnatMode_str[] =
{
	"RT_FLOW_HWNAT_MODE_DIS_ACC",
	"RT_FLOW_HWNAT_MODE_ORIGINAL",
	"RT_FLOW_HWNAT_MODE_SW_ONLY",
	"RT_FLOW_HWNAT_MODE_HW_ONLY",
	"RT_FLOW_HWNAT_MODE_TRAP_ALL",
};

/*
 * rt_flow clear operation_para delete_flow_key
 */
cparser_result_t
cparser_cmd_rt_flow_clear_operation_para_delete_flow_key(
    cparser_context_t *context)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&delFlowkey.data_delFlow, 0x0, sizeof(rt_flow_op_data_delete_flow_t));

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_clear_operation_para_delete_flow_key */

/*
 * rt_flow set operation_para delete_flow flowKeyType <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKeyType_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	if(*index_ptr >= RT_FLOW_OP_DEL_FLOW_KEY_TYPE_END)
	{
		diag_util_mprintf("invalid flowKeyType\n");
		return CPARSER_NOT_OK;
	}
	delFlowkey.data_delFlow.flowKeyType = *index_ptr;

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_set_operation_para_delete_flow_flowkeytype_index */

/*
 * rt_flow set operation_para delete_flow flowKey INDEX <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKey_INDEX_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	delFlowkey.data_delFlow.flowKey.flowIdx = *index_ptr;

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_set_operation_para_delete_flow_flowkey_index_index */

/*
 * rt_flow set operation_para delete_flow flowKey SIP4 <IPV4ADDR:ingress_sip>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKey_SIP4_ingress_sip(
    cparser_context_t *context,
    uint32_t  *ingress_sip_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	delFlowkey.data_delFlow.flowKey.flowPattern.sip4_valid = 1;
	delFlowkey.data_delFlow.flowKey.flowPattern.sip4 = *ingress_sip_ptr;

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_set_operation_para_delete_flow_flowkey_sip4_ingress_sip */

/*
 * rt_flow set operation_para delete_flow flowKey DIP4 <IPV4ADDR:ingress_dip>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKey_DIP4_ingress_dip(
    cparser_context_t *context,
    uint32_t  *ingress_dip_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	delFlowkey.data_delFlow.flowKey.flowPattern.dip4_valid = 1;
	delFlowkey.data_delFlow.flowKey.flowPattern.dip4 = *ingress_dip_ptr;

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_set_operation_para_delete_flow_flowkey_dip4_ingress_dip */

/*
 * rt_flow set operation_para delete_flow flowKey SIP6 <IPV6ADDR:ingress_sip>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKey_SIP6_ingress_sip(
    cparser_context_t *context,
    char * *ingress_sip_ptr)
{
	int32 ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	delFlowkey.data_delFlow.flowKey.flowPattern.sip6_valid = 1;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&(delFlowkey.data_delFlow.flowKey.flowPattern.sip6[0]), TOKEN_STR(6)), ret);

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_set_operation_para_delete_flow_flowkey_sip6_ingress_sip */

/*
 * rt_flow set operation_para delete_flow flowKey DIP6 <IPV6ADDR:ingress_dip>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKey_DIP6_ingress_dip(
    cparser_context_t *context,
    char * *ingress_dip_ptr)
{
	int32 ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	delFlowkey.data_delFlow.flowKey.flowPattern.dip6_valid = 1;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&(delFlowkey.data_delFlow.flowKey.flowPattern.dip6[0]), TOKEN_STR(6)), ret);

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_set_operation_para_delete_flow_flowkey_dip6_ingress_dip */

/*
 * rt_flow set operation_para delete_flow flowKey SPORT <UINT:ingress_sport>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKey_SPORT_ingress_sport(
    cparser_context_t *context,
    uint32_t  *ingress_sport_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	delFlowkey.data_delFlow.flowKey.flowPattern.sport_valid= 1;
	delFlowkey.data_delFlow.flowKey.flowPattern.sport = *ingress_sport_ptr;

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_set_operation_para_delete_flow_flowkey_sport_ingress_sport */

/*
 * rt_flow set operation_para delete_flow flowKey DPORT <UINT:ingress_dport>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKey_DPORT_ingress_dport(
    cparser_context_t *context,
    uint32_t  *ingress_dport_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	delFlowkey.data_delFlow.flowKey.flowPattern.dport_valid= 1;
	delFlowkey.data_delFlow.flowKey.flowPattern.dport = *ingress_dport_ptr;

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_set_operation_para_delete_flow_flowkey_dport_ingress_dport */

/*
 * rt_flow set operation_para delete_flow flowKey L4PROTO <UINT:l4_proto>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKey_L4PROTO_l4_proto(
    cparser_context_t *context,
    uint32_t  *l4_proto_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	delFlowkey.data_delFlow.flowKey.flowPattern.l4proto_valid= 1;
	delFlowkey.data_delFlow.flowKey.flowPattern.l4proto = *l4_proto_ptr;

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_set_operation_para_delete_flow_flowkey_l4proto_l4_proto */

/*
 * rt_flow set operation_para delete_flow flowKey MSIP4 <IPV4ADDR:modified_sip>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKey_MSIP4_modified_sip(
	cparser_context_t *context,
	uint32_t  *modified_sip_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	delFlowkey.data_delFlow.flowKey.flowPattern.modified_sip4_valid = 1;
	delFlowkey.data_delFlow.flowKey.flowPattern.modified_sip4 = *modified_sip_ptr;

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_flow_set_operation_para_delete_flow_flowkey_msip4_modified_sip */

/*
 * rt_flow set operation_para delete_flow flowKey MDIP4 <IPV4ADDR:modified_dip>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKey_MDIP4_modified_dip(
	cparser_context_t *context,
	uint32_t  *modified_dip_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	delFlowkey.data_delFlow.flowKey.flowPattern.modified_dip4_valid = 1;
	delFlowkey.data_delFlow.flowKey.flowPattern.modified_dip4 = *modified_dip_ptr;

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_flow_set_operation_para_delete_flow_flowkey_mdip4_modified_dip */

/*
 * rt_flow set operation_para delete_flow flowKey MSPORT <UINT:modified_sport>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKey_MSPORT_modified_sport(
	cparser_context_t *context,
	uint32_t  *modified_sport_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	delFlowkey.data_delFlow.flowKey.flowPattern.modified_sport_valid= 1;
	delFlowkey.data_delFlow.flowKey.flowPattern.modified_sport = *modified_sport_ptr;

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_flow_set_operation_para_delete_flow_flowkey_msport_modified_sport */

/*
 * rt_flow set operation_para delete_flow flowKey MDPORT <UINT:modified_dport>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKey_MDPORT_modified_dport(
	cparser_context_t *context,
	uint32_t  *modified_dport_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	delFlowkey.data_delFlow.flowKey.flowPattern.modified_dport_valid= 1;
	delFlowkey.data_delFlow.flowKey.flowPattern.modified_dport = *modified_dport_ptr;

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_flow_set_operation_para_delete_flow_flowkey_mdport_modified_dport */

/*
 * rt_flow set operation_para delete_flow flowKey MAC <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKey_MAC_mac(
	cparser_context_t *context,
	cparser_macaddr_t  *mac_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	delFlowkey.data_delFlow.flowKey.flowPattern.mac_valid = 1;
	osal_memcpy(delFlowkey.data_delFlow.flowKey.flowPattern.mac, mac_ptr->octet, ETHER_ADDR_LEN);

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKey_MAC_mac */

/*
 * rt_flow set operation_para delete_flow flowKey SMAC <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKey_SMAC_mac(
	cparser_context_t *context,
	cparser_macaddr_t  *mac_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	delFlowkey.data_delFlow.flowKey.flowPattern.smac_valid = 1;
	osal_memcpy(delFlowkey.data_delFlow.flowKey.flowPattern.smac, mac_ptr->octet, ETHER_ADDR_LEN);

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKey_SMAC_mac */

/*
 * rt_flow set operation_para delete_flow flowKey DMAC <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKey_DMAC_mac(
	cparser_context_t *context,
	cparser_macaddr_t  *mac_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	delFlowkey.data_delFlow.flowKey.flowPattern.dmac_valid = 1;
	osal_memcpy(delFlowkey.data_delFlow.flowKey.flowPattern.dmac, mac_ptr->octet, ETHER_ADDR_LEN);

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_flow_set_operation_para_delete_flow_flowKey_DMAC_mac */

/*
 * rt_flow show operation_para delete_flow flowKey
 */
cparser_result_t
cparser_cmd_rt_flow_show_operation_para_delete_flow_flowKey(
    cparser_context_t *context)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	if(delFlowkey.data_delFlow.flowKeyType == RT_FLOW_OP_FLOW_KEY_TYPE_INDEX)
	{
		diag_util_mprintf("flowKeyType: flow_index\n");
		diag_util_mprintf("   index: %d\n", delFlowkey.data_delFlow.flowKey.flowIdx);
	}
	else
	{
		diag_util_mprintf("flowKeyType: pattern\n");

		diag_util_mprintf("   %-15s %-5s", "l4proto_valid:", (delFlowkey.data_delFlow.flowKey.flowPattern.l4proto_valid)?"TRUE":"FALSE");
		if(delFlowkey.data_delFlow.flowKey.flowPattern.l4proto_valid)
			diag_util_mprintf("  l4proto: %s\n", (delFlowkey.data_delFlow.flowKey.flowPattern.l4proto == RT_FLOW_OP_FLOW_PATTERN_L4PROTO_UDP)?"UDP":"TCP");
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s", "SIP4_valid:", (delFlowkey.data_delFlow.flowKey.flowPattern.sip4_valid)?"TRUE":"FALSE");
		if(delFlowkey.data_delFlow.flowKey.flowPattern.sip4_valid)
			diag_util_mprintf("  SIP4: %s\n", diag_util_inet_ntoa(delFlowkey.data_delFlow.flowKey.flowPattern.sip4));
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s","DIP4_valid:", (delFlowkey.data_delFlow.flowKey.flowPattern.dip4_valid)?"TRUE":"FALSE");
		if(delFlowkey.data_delFlow.flowKey.flowPattern.dip4_valid)
			diag_util_mprintf("  DIP4: %s\n", diag_util_inet_ntoa(delFlowkey.data_delFlow.flowKey.flowPattern.dip4));
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s", "SIP6_valid:", (delFlowkey.data_delFlow.flowKey.flowPattern.sip6_valid)?"TRUE":"FALSE");
		if(delFlowkey.data_delFlow.flowKey.flowPattern.sip6_valid)
			diag_util_mprintf("  SIP6: %s\n", diag_util_inet_n6toa(&(delFlowkey.data_delFlow.flowKey.flowPattern.sip6[0])));
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s", "DIP6_valid:", (delFlowkey.data_delFlow.flowKey.flowPattern.dip6_valid)?"TRUE":"FALSE");
		if(delFlowkey.data_delFlow.flowKey.flowPattern.dip6_valid)
			diag_util_mprintf("  DIP6: %s\n", diag_util_inet_n6toa(&(delFlowkey.data_delFlow.flowKey.flowPattern.dip6[0])));
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s", "SPORT_valid:", (delFlowkey.data_delFlow.flowKey.flowPattern.sport_valid)?"TRUE":"FALSE");
		if(delFlowkey.data_delFlow.flowKey.flowPattern.sport_valid)
			diag_util_mprintf("  SPORT: %d\n", delFlowkey.data_delFlow.flowKey.flowPattern.sport);
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s", "DPORT_valid:", (delFlowkey.data_delFlow.flowKey.flowPattern.dport_valid)?"TRUE":"FALSE");
		if(delFlowkey.data_delFlow.flowKey.flowPattern.dport_valid)
			diag_util_mprintf("  DPORT: %d\n", delFlowkey.data_delFlow.flowKey.flowPattern.dport);
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s", "MSIP4_valid:", (delFlowkey.data_delFlow.flowKey.flowPattern.modified_sip4_valid)?"TRUE":"FALSE");
		if(delFlowkey.data_delFlow.flowKey.flowPattern.modified_sip4_valid)
			diag_util_mprintf("  MSIP4: %s\n", diag_util_inet_ntoa(delFlowkey.data_delFlow.flowKey.flowPattern.modified_sip4));
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s","MDIP4_valid:", (delFlowkey.data_delFlow.flowKey.flowPattern.modified_dip4_valid)?"TRUE":"FALSE");
		if(delFlowkey.data_delFlow.flowKey.flowPattern.modified_dip4_valid)
			diag_util_mprintf("  MDIP4: %s\n", diag_util_inet_ntoa(delFlowkey.data_delFlow.flowKey.flowPattern.modified_dip4));
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s", "MSPORT_valid:", (delFlowkey.data_delFlow.flowKey.flowPattern.modified_sport_valid)?"TRUE":"FALSE");
		if(delFlowkey.data_delFlow.flowKey.flowPattern.modified_sport_valid)
			diag_util_mprintf("  MSPORT: %d\n", delFlowkey.data_delFlow.flowKey.flowPattern.modified_sport);
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s", "MDPORT_valid:", (delFlowkey.data_delFlow.flowKey.flowPattern.modified_dport_valid)?"TRUE":"FALSE");
		if(delFlowkey.data_delFlow.flowKey.flowPattern.modified_dport_valid)
			diag_util_mprintf("  MDPORT: %d\n", delFlowkey.data_delFlow.flowKey.flowPattern.modified_dport);
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s", "MAC_valid:", (delFlowkey.data_delFlow.flowKey.flowPattern.mac_valid)?"TRUE":"FALSE");
		if(delFlowkey.data_delFlow.flowKey.flowPattern.mac_valid)
			diag_util_mprintf("  MAC: %-17s\n", diag_util_inet_mactoa(delFlowkey.data_delFlow.flowKey.flowPattern.mac));
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s", "SMAC_valid:", (delFlowkey.data_delFlow.flowKey.flowPattern.smac_valid)?"TRUE":"FALSE");
		if(delFlowkey.data_delFlow.flowKey.flowPattern.smac_valid)
			diag_util_mprintf("  SMAC: %-17s\n", diag_util_inet_mactoa(delFlowkey.data_delFlow.flowKey.flowPattern.smac));
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s", "DMAC_valid:", (delFlowkey.data_delFlow.flowKey.flowPattern.dmac_valid)?"TRUE":"FALSE");
		if(delFlowkey.data_delFlow.flowKey.flowPattern.dmac_valid)
			diag_util_mprintf("  DMAC: %-17s\n", diag_util_inet_mactoa(delFlowkey.data_delFlow.flowKey.flowPattern.dmac));
		else
			diag_util_mprintf("\n");
	}

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_show_operation_para_delete_flow_flowkey */

/*
 * rt_flow add operation delete_flow
 */
cparser_result_t
cparser_cmd_rt_flow_add_operation_delete_flow(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	DIAG_UTIL_ERR_CHK(rt_flow_operation_add(RT_FLOW_OPS_DELETE_FLOW, &delFlowkey), ret);

	if(delFlowkey.data_delFlow.flowKeyType == RT_FLOW_OP_FLOW_KEY_TYPE_INDEX)
	{
		diag_util_mprintf("delete flow by flow_index\n");
		diag_util_mprintf("   index: %d\n", delFlowkey.data_delFlow.flowKey.flowIdx);
	}
	else
	{
		diag_util_mprintf("delete flow by pattern\n");
		if(delFlowkey.data_delFlow.flowKey.flowPattern.l4proto_valid)
			diag_util_mprintf("   l4Proto: %s\n", (delFlowkey.data_delFlow.flowKey.flowPattern.l4proto == RT_FLOW_OP_FLOW_PATTERN_L4PROTO_UDP)?"UDP":"TCP");
		if(delFlowkey.data_delFlow.flowKey.flowPattern.sip4_valid)
			diag_util_mprintf("   SIP4: %s\n", diag_util_inet_ntoa(delFlowkey.data_delFlow.flowKey.flowPattern.sip4));
		if(delFlowkey.data_delFlow.flowKey.flowPattern.dip4_valid)
			diag_util_mprintf("   DIP4: %s\n", diag_util_inet_ntoa(delFlowkey.data_delFlow.flowKey.flowPattern.dip4));
		if(delFlowkey.data_delFlow.flowKey.flowPattern.sip6_valid)
			diag_util_mprintf("   SIP6: %s\n", diag_util_inet_n6toa(&(delFlowkey.data_delFlow.flowKey.flowPattern.sip6[0])));
		if(delFlowkey.data_delFlow.flowKey.flowPattern.dip6_valid)
			diag_util_mprintf("   DIP6: %s\n", diag_util_inet_n6toa(&(delFlowkey.data_delFlow.flowKey.flowPattern.dip6[0])));
		if(delFlowkey.data_delFlow.flowKey.flowPattern.sport_valid)
			diag_util_mprintf("   SPORT: %d\n", delFlowkey.data_delFlow.flowKey.flowPattern.sport);
		if(delFlowkey.data_delFlow.flowKey.flowPattern.dport_valid)
			diag_util_mprintf("   DPORT: %d\n", delFlowkey.data_delFlow.flowKey.flowPattern.dport);
		if(delFlowkey.data_delFlow.flowKey.flowPattern.mac_valid)
			diag_util_mprintf("   MAC: %-17s\n", diag_util_inet_mactoa(delFlowkey.data_delFlow.flowKey.flowPattern.mac));
		if(delFlowkey.data_delFlow.flowKey.flowPattern.smac_valid)
			diag_util_mprintf("   SMAC: %-17s\n", diag_util_inet_mactoa(delFlowkey.data_delFlow.flowKey.flowPattern.smac));
		if(delFlowkey.data_delFlow.flowKey.flowPattern.dmac_valid)
			diag_util_mprintf("   DMAC: %-17s\n", diag_util_inet_mactoa(delFlowkey.data_delFlow.flowKey.flowPattern.dmac));
	}

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_add_operation_delete_flow */

/*
 * rt_flow add operation flush_flow
 */
cparser_result_t
cparser_cmd_rt_flow_add_operation_flush_flow(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	static rt_flow_ops_data_t tempOpData;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	DIAG_UTIL_ERR_CHK(rt_flow_operation_add(RT_FLOW_OPS_FLUSH_FLOWTABLE, &tempOpData), ret);
	diag_util_mprintf("flush flow table\n");

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_add_operation_flush_flow */

/*
 * rt_flow add operation get_swFlow_mib <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_flow_add_operation_get_swFlow_mib_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
	rt_flow_ops_data_t opdata;
	int32 ret = RT_ERR_FAILED;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	opdata.data_getFlowMib.index = *index_ptr;
	DIAG_UTIL_ERR_CHK(rt_flow_operation_add(RT_FLOW_OPS_GET_SW_FLOW_MIB_BY_IDX, &opdata), ret);
	diag_util_mprintf("sw flow mib [%d]\n", opdata.data_getFlowMib.index);
	diag_util_mprintf("ingress packet count: %u\n", opdata.data_getFlowMib.swFlowCnt.ingress_packet_count);
	diag_util_mprintf("ingress byte count: %llu\n", opdata.data_getFlowMib.swFlowCnt.ingress_byte_count);

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_add_operation_get_swflow_mib_index */

/*
 * rt_flow add operation get_flowGrp_mib <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_flow_add_operation_get_flowGrp_mib_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    
	rt_flow_ops_data_t opdata;
	int32 ret = RT_ERR_FAILED;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	opdata.data_getFlowGrpMib.index = *index_ptr;
	DIAG_UTIL_ERR_CHK(rt_flow_operation_add(RT_FLOW_OPS_GET_FLOWGRP_MIB_BY_IDX, &opdata), ret);
	diag_util_mprintf("sw flow mib [%d]\n", opdata.data_getFlowGrpMib.index);
	diag_util_mprintf("ingress packet count: %u\n", opdata.data_getFlowGrpMib.flowGrpCnt.ingress_packet_count);
	diag_util_mprintf("ingress byte count: %llu\n", opdata.data_getFlowGrpMib.flowGrpCnt.ingress_byte_count);
	diag_util_mprintf("egress packet count: %u\n", opdata.data_getFlowGrpMib.flowGrpCnt.egress_packet_count);
	diag_util_mprintf("egress byte count: %llu\n", opdata.data_getFlowGrpMib.flowGrpCnt.egress_byte_count);

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_add_operation_get_flowgrp_mib_index */

/*
 * rt_flow add operation clear_allMib
 */
cparser_result_t
cparser_cmd_rt_flow_add_operation_clear_allMib(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	static rt_flow_ops_data_t tempOpData;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	DIAG_UTIL_ERR_CHK(rt_flow_operation_add(RT_FLOW_OPS_CLEAR_ALL_MIB, &tempOpData), ret);
	diag_util_mprintf("clear all mib\n");

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_add_operation_clear_allmib */

/*
 * rt_flow add operation clear_swFlow_mib <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_flow_add_operation_clear_swFlow_mib_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
	rt_flow_ops_data_t opdata;
   	int32 ret = RT_ERR_FAILED;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	opdata.data_clearFlowMib = *index_ptr;
	DIAG_UTIL_ERR_CHK(rt_flow_operation_add(RT_FLOW_OPS_CLEAR_SW_FLOW_MIB_BY_IDX, &opdata), ret);
	diag_util_mprintf("clear sw flow mib by flow index %d\n", opdata.data_clearFlowMib);

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_add_operation_clear_swflow_mib_index */

/*
 * rt_flow add operation clear_flowGrp_mib <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_flow_add_operation_clear_flowGrp_mib_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
	rt_flow_ops_data_t opdata;
   	int32 ret = RT_ERR_FAILED;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	opdata.data_clearFlowGrpMib = *index_ptr;
	DIAG_UTIL_ERR_CHK(rt_flow_operation_add(RT_FLOW_OPS_CLEAR_FLOWGRP_MIB_BY_IDX, &opdata), ret);
	diag_util_mprintf("clear flow group mib by flowmib index %d\n", opdata.data_clearFlowGrpMib);

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_add_operation_clear_flowgrp_mib_index */

/*
 * rt_flow add operation sync_CT
 */
cparser_result_t
cparser_cmd_rt_flow_add_operation_sync_CT(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	static rt_flow_ops_data_t tempOpData;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	DIAG_UTIL_ERR_CHK(rt_flow_operation_add(RT_FLOW_OPS_SYNC_CT_STATE, &tempOpData), ret);
	diag_util_mprintf("sync CT to flow\n");

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_add_operation_sync_ct */

/*
 * rt_flow clear operation_para hiPriFlow_pattern
 */
cparser_result_t
cparser_cmd_rt_flow_clear_operation_para_hiPriFlow_pattern(
    cparser_context_t *context)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&hiPriFlowPtn.data_hiPriflowPtnAct, 0x0, sizeof(rt_flow_op_hiPriflow_ptn_act_t));

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_clear_operation_para_hipriflow_pattern */


/*
 * rt_flow set operation_para hiPriFlow pattern SIP4 <IPV4ADDR:ingress_sip>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_hiPriFlow_pattern_SIP4_ingress_sip(
    cparser_context_t *context,
    uint32_t  *ingress_sip_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip4_valid = 1;
	hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip4 = *ingress_sip_ptr;

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_set_operation_para_hipriflow_pattern_sip4_ingress_sip */

/*
 * rt_flow set operation_para hiPriFlow pattern DIP4 <IPV4ADDR:ingress_dip>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_hiPriFlow_pattern_DIP4_ingress_dip(
    cparser_context_t *context,
    uint32_t  *ingress_dip_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip4_valid = 1;
	hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip4 = *ingress_dip_ptr;

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_set_operation_para_hipriflow_pattern_dip4_ingress_dip */

/*
 * rt_flow set operation_para hiPriFlow pattern SIP6 <IPV6ADDR:ingress_sip>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_hiPriFlow_pattern_SIP6_ingress_sip(
    cparser_context_t *context,
    char * *ingress_sip_ptr)
{
	int32 ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip6_valid = 1;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip6[0]), TOKEN_STR(6)), ret);

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_set_operation_para_hipriflow_pattern_sip6_ingress_sip */

/*
 * rt_flow set operation_para hiPriFlow pattern DIP6 <IPV6ADDR:ingress_dip>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_hiPriFlow_pattern_DIP6_ingress_dip(
    cparser_context_t *context,
    char * *ingress_dip_ptr)
{
	int32 ret = RT_ERR_FAILED;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip6_valid = 1;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip6[0]), TOKEN_STR(6)), ret);

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_set_operation_para_hipriflow_pattern_dip6_ingress_dip */

/*
 * rt_flow set operation_para hiPriFlow pattern SPORT <UINT:ingress_sport>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_hiPriFlow_pattern_SPORT_ingress_sport(
    cparser_context_t *context,
    uint32_t  *ingress_sport_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sport_valid= 1;
	hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sport = *ingress_sport_ptr;

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_set_operation_para_hipriflow_pattern_sport_ingress_sport */

/*
 * rt_flow set operation_para hiPriFlow pattern DPORT <UINT:ingress_dport>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_hiPriFlow_pattern_DPORT_ingress_dport(
    cparser_context_t *context,
    uint32_t  *ingress_dport_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dport_valid= 1;
	hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dport = *ingress_dport_ptr;

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_set_operation_para_hipriflow_pattern_dport_ingress_dport */

/*
 * rt_flow set operation_para hiPriFlow pattern L4PROTO <UINT:l4_proto>
 */
cparser_result_t
cparser_cmd_rt_flow_set_operation_para_hiPriFlow_pattern_L4PROTO_l4_proto(
    cparser_context_t *context,
    uint32_t  *l4_proto_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.l4proto_valid= 1;
	hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.l4proto = *l4_proto_ptr;

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_set_operation_para_hipriflow_pattern_l4proto_l4_proto */

/*
 * rt_flow show operation_para hiPriFlow pattern
 */
cparser_result_t
cparser_cmd_rt_flow_show_operation_para_hiPriFlow_pattern(
    cparser_context_t *context)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	diag_util_mprintf("   %-15s %-5s", "l4proto_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.l4proto_valid)?"TRUE":"FALSE");
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.l4proto_valid)
		diag_util_mprintf("  l4proto: %s\n", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.l4proto == RT_FLOW_OP_FLOW_PATTERN_L4PROTO_UDP)?"UDP":"TCP");
	else
		diag_util_mprintf("\n");

	diag_util_mprintf("   %-15s %-5s", "SIP4_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip4_valid)?"TRUE":"FALSE");
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip4_valid)
		diag_util_mprintf("  SIP4: %s\n", diag_util_inet_ntoa(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip4));
	else
		diag_util_mprintf("\n");

	diag_util_mprintf("   %-15s %-5s","DIP4_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip4_valid)?"TRUE":"FALSE");
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip4_valid)
		diag_util_mprintf("  DIP4: %s\n", diag_util_inet_ntoa(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip4));
	else
		diag_util_mprintf("\n");

	diag_util_mprintf("   %-15s %-5s", "SIP6_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip6_valid)?"TRUE":"FALSE");
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip6_valid)
		diag_util_mprintf("  SIP6: %s\n", diag_util_inet_n6toa(&(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip6[0])));
	else
		diag_util_mprintf("\n");

	diag_util_mprintf("   %-15s %-5s", "DIP6_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip6_valid)?"TRUE":"FALSE");
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip6_valid)
		diag_util_mprintf("  DIP6: %s\n", diag_util_inet_n6toa(&(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip6[0])));
	else
		diag_util_mprintf("\n");

	diag_util_mprintf("   %-15s %-5s", "SPORT_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sport_valid)?"TRUE":"FALSE");
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sport_valid)
		diag_util_mprintf("  SPORT: %d\n", hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sport);
	else
		diag_util_mprintf("\n");

	diag_util_mprintf("   %-15s %-5s", "DPORT_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dport_valid)?"TRUE":"FALSE");
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dport_valid)
		diag_util_mprintf("  DPORT: %d\n", hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dport);
	else
		diag_util_mprintf("\n");
	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_show_operation_para_hipriflow_pattern */

/*
 * rt_flow add operation hiPriFlowPtn_act add <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_flow_add_operation_hiPriFlowPtn_act_add_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
	int32 ret = RT_ERR_FAILED;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	hiPriFlowPtn.data_hiPriflowPtnAct.hipriflow_ptn_act = RT_FLOW_OP_HIPRIFLOW_PTN_ACT_ADD_BY_IDX;
	hiPriFlowPtn.data_hiPriflowPtnAct.index = *index_ptr;
	DIAG_UTIL_ERR_CHK(rt_flow_operation_add(RT_FLOW_OPS_HIPRIFLOW_PTN_ACT, &hiPriFlowPtn), ret);

	diag_util_mprintf("Add hiPriFlowPtn[%d]\n", hiPriFlowPtn.data_hiPriflowPtnAct.index);
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.l4proto_valid)
		diag_util_mprintf("   l4Proto: %s\n", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.l4proto == RT_FLOW_OP_FLOW_PATTERN_L4PROTO_UDP)?"UDP":"TCP");
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip4_valid)
		diag_util_mprintf("   SIP4: %s\n", diag_util_inet_ntoa(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip4));
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip4_valid)
		diag_util_mprintf("   DIP4: %s\n", diag_util_inet_ntoa(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip4));
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip6_valid)
		diag_util_mprintf("   SIP6: %s\n", diag_util_inet_n6toa(&(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip6[0])));
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip6_valid)
		diag_util_mprintf("   DIP6: %s\n", diag_util_inet_n6toa(&(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip6[0])));
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sport_valid)
		diag_util_mprintf("   SPORT: %d\n", hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sport);
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dport_valid)
		diag_util_mprintf("   DPORT: %d\n", hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dport);

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_add_operation_hipriflowptn_act_add_index */

/*
 * rt_flow add operation hiPriFlowPtn_act del <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_flow_add_operation_hiPriFlowPtn_act_del_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
	int32 ret = RT_ERR_FAILED;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	hiPriFlowPtn.data_hiPriflowPtnAct.hipriflow_ptn_act = RT_FLOW_OP_HIPRIFLOW_PTN_ACT_DEL_BY_IDX;
	hiPriFlowPtn.data_hiPriflowPtnAct.index = *index_ptr;
	DIAG_UTIL_ERR_CHK(rt_flow_operation_add(RT_FLOW_OPS_HIPRIFLOW_PTN_ACT, &hiPriFlowPtn), ret);

	diag_util_mprintf("Delete hiPriFlowPtn[%d]\n", hiPriFlowPtn.data_hiPriflowPtnAct.index);
	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_add_operation_hipriflowptn_act_del_index */

/*
 * rt_flow add operation hiPriFlowPtn_act get <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_flow_add_operation_hiPriFlowPtn_act_get_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
	int32 ret = RT_ERR_FAILED;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	hiPriFlowPtn.data_hiPriflowPtnAct.hipriflow_ptn_act = RT_FLOW_OP_HIPRIFLOW_PTN_ACT_GET_BY_IDX;
	hiPriFlowPtn.data_hiPriflowPtnAct.index = *index_ptr;
	DIAG_UTIL_ERR_CHK(rt_flow_operation_add(RT_FLOW_OPS_HIPRIFLOW_PTN_ACT, &hiPriFlowPtn), ret);

	diag_util_mprintf("hiPriFlowPtn[%d]\n", hiPriFlowPtn.data_hiPriflowPtnAct.index);
	diag_util_mprintf("   %-15s %-5s", "l4proto_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.l4proto_valid)?"TRUE":"FALSE");
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.l4proto_valid)
		diag_util_mprintf("  l4proto: %s\n", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.l4proto == RT_FLOW_OP_FLOW_PATTERN_L4PROTO_UDP)?"UDP":"TCP");
	else
		diag_util_mprintf("\n");

	diag_util_mprintf("   %-15s %-5s", "SIP4_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip4_valid)?"TRUE":"FALSE");
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip4_valid)
		diag_util_mprintf("  SIP4: %s\n", diag_util_inet_ntoa(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip4));
	else
		diag_util_mprintf("\n");

	diag_util_mprintf("   %-15s %-5s","DIP4_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip4_valid)?"TRUE":"FALSE");
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip4_valid)
		diag_util_mprintf("  DIP4: %s\n", diag_util_inet_ntoa(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip4));
	else
		diag_util_mprintf("\n");

	diag_util_mprintf("   %-15s %-5s", "SIP6_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip6_valid)?"TRUE":"FALSE");
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip6_valid)
		diag_util_mprintf("  SIP6: %s\n", diag_util_inet_n6toa(&(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip6[0])));
	else
		diag_util_mprintf("\n");

	diag_util_mprintf("   %-15s %-5s", "DIP6_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip6_valid)?"TRUE":"FALSE");
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip6_valid)
		diag_util_mprintf("  DIP6: %s\n", diag_util_inet_n6toa(&(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip6[0])));
	else
		diag_util_mprintf("\n");

	diag_util_mprintf("   %-15s %-5s", "SPORT_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sport_valid)?"TRUE":"FALSE");
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sport_valid)
		diag_util_mprintf("  SPORT: %d\n", hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sport);
	else
		diag_util_mprintf("\n");

	diag_util_mprintf("   %-15s %-5s", "DPORT_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dport_valid)?"TRUE":"FALSE");
	if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dport_valid)
		diag_util_mprintf("  DPORT: %d\n", hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dport);
	else
		diag_util_mprintf("\n");
	return CPARSER_OK;

}    /* end of cparser_cmd_rt_flow_add_operation_hipriflowptn_act_get_index */

/*
 * rt_flow add operation hiPriFlowPtn_act get all
 */
cparser_result_t
cparser_cmd_rt_flow_add_operation_hiPriFlowPtn_act_get_all(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED, i;
	DIAG_UTIL_PARAM_CHK();
	hiPriFlowPtn.data_hiPriflowPtnAct.hipriflow_ptn_act = RT_FLOW_OP_HIPRIFLOW_PTN_ACT_GET_BY_IDX;
	for(i = 0 ; i < RT_FLOW_HIGHPRIFLOW_PATTERN_SIZE ; i++)
	{
		hiPriFlowPtn.data_hiPriflowPtnAct.index = i;
		DIAG_UTIL_OUTPUT_INIT();
		DIAG_UTIL_ERR_CHK(rt_flow_operation_add(RT_FLOW_OPS_HIPRIFLOW_PTN_ACT, &hiPriFlowPtn), ret);

		diag_util_mprintf("hiPriFlowPtn[%d]\n", hiPriFlowPtn.data_hiPriflowPtnAct.index);
		diag_util_mprintf("   %-15s %-5s", "l4proto_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.l4proto_valid)?"TRUE":"FALSE");
		if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.l4proto_valid)
			diag_util_mprintf("  l4proto: %s\n", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.l4proto == RT_FLOW_OP_FLOW_PATTERN_L4PROTO_UDP)?"UDP":"TCP");
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s", "SIP4_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip4_valid)?"TRUE":"FALSE");
		if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip4_valid)
			diag_util_mprintf("  SIP4: %s\n", diag_util_inet_ntoa(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip4));
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s","DIP4_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip4_valid)?"TRUE":"FALSE");
		if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip4_valid)
			diag_util_mprintf("  DIP4: %s\n", diag_util_inet_ntoa(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip4));
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s", "SIP6_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip6_valid)?"TRUE":"FALSE");
		if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip6_valid)
			diag_util_mprintf("  SIP6: %s\n", diag_util_inet_n6toa(&(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sip6[0])));
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s", "DIP6_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip6_valid)?"TRUE":"FALSE");
		if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip6_valid)
			diag_util_mprintf("  DIP6: %s\n", diag_util_inet_n6toa(&(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dip6[0])));
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s", "SPORT_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sport_valid)?"TRUE":"FALSE");
		if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sport_valid)
			diag_util_mprintf("  SPORT: %d\n", hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.sport);
		else
			diag_util_mprintf("\n");

		diag_util_mprintf("   %-15s %-5s", "DPORT_valid:", (hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dport_valid)?"TRUE":"FALSE");
		if(hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dport_valid)
			diag_util_mprintf("  DPORT: %d\n", hiPriFlowPtn.data_hiPriflowPtnAct.flowPattern.dport);
		else
			diag_util_mprintf("\n");
	}
	return CPARSER_OK;

}    /* end of cparser_cmd_rt_flow_add_operation_hipriflowptn_act_get_all */

/*
 * rt_flow add operation hiPriFlowPtn_act flush
 */
cparser_result_t
cparser_cmd_rt_flow_add_operation_hiPriFlowPtn_act_flush(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	hiPriFlowPtn.data_hiPriflowPtnAct.hipriflow_ptn_act = RT_FLOW_OP_HIPRIFLOW_PTN_ACT_FLUSH;
	DIAG_UTIL_ERR_CHK(rt_flow_operation_add(RT_FLOW_OPS_HIPRIFLOW_PTN_ACT, &hiPriFlowPtn), ret);
	diag_util_mprintf("Flush hiPriFlowPtn table\n", hiPriFlowPtn.data_hiPriflowPtnAct.index);
	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_add_operation_hipriflowptn_act_flush */

/*
 * rt_flow set hwnatMode <UINT:mode>
 */
cparser_result_t
cparser_cmd_rt_flow_set_hwnatMode_mode(
    cparser_context_t *context,
    uint32_t  *mode_ptr)
{
	rt_flow_hwnat_mode_t hwnatMode;
	int32 ret = RT_ERR_FAILED;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	hwnatMode = *mode_ptr;
	if(hwnatMode == 100) hwnatMode = RT_FLOW_HWNAT_MODE_SPEC_TEST;
	DIAG_UTIL_ERR_CHK(rt_flow_hwnatMode_set(hwnatMode), ret);
	DIAG_UTIL_ERR_CHK(rt_flow_hwnatMode_get(&hwnatMode), ret);
	if(hwnatMode == RT_FLOW_HWNAT_MODE_SPEC_TEST)
		diag_util_mprintf("set hwnat mode: %d (%s)\n", hwnatMode, "RT_FLOW_HWNAT_MODE_SPEC_TEST");
	else
		diag_util_mprintf("set hwnat mode: %d (%s)\n", hwnatMode, hwnatMode_str[hwnatMode]);

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_set_hwnatmode_mode */

/*
 * rt_flow get hwnatMode
 */
cparser_result_t
cparser_cmd_rt_flow_get_hwnatMode(
    cparser_context_t *context)
{
	rt_flow_hwnat_mode_t hwnatMode;
	int32 ret = RT_ERR_FAILED;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	DIAG_UTIL_ERR_CHK(rt_flow_hwnatMode_get(&hwnatMode), ret);
	if(hwnatMode == RT_FLOW_HWNAT_MODE_SPEC_TEST)
		diag_util_mprintf("get hwnat mode: %d (%s)\n", hwnatMode, "RT_FLOW_HWNAT_MODE_SPEC_TEST");
	else
		diag_util_mprintf("get hwnat mode: %d (%s)\n", hwnatMode, hwnatMode_str[hwnatMode]);
	return CPARSER_OK;
}    /* end of cparser_cmd_rt_flow_get_hwnatmode */

