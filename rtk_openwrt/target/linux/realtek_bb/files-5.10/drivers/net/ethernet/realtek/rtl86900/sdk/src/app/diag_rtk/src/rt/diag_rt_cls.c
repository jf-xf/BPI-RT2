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
 *
 * $Revision:  $
 * $Date:  $
 *
 * Purpose : Definition those RT cls command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *
 */

#include <stdio.h>
#include <string.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <diag_str.h>
#include <parser/cparser_priv.h>
#include <rtk/rt/rt_cls.h>

rt_cls_rule_t entry;

static uint32 _cls_rule_show(rt_cls_rule_t *pRule)
{
    int i;
    char ingressPort[64];
    rt_portmask_t portmask;
    rtk_portmask_t tmpPortmask;

    diag_util_printf("Index: %d\n", pRule->index);
    diag_util_printf("FlowID: %d\n", pRule->flowId);
    if(pRule->direction == RT_CLS_DIR_DS)
        diag_util_printf("Direction: Downstream\n");
    else
        diag_util_printf("Direction: Upstream\n");

    portmask.bits[0] = pRule->ingressPortMask;
    memcpy(tmpPortmask.bits,portmask.bits,RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST*sizeof(uint32));
    diag_util_lPortMask2str(ingressPort,&tmpPortmask);
    diag_util_printf("IngressPortMask: %s\n",ingressPort);

    /*Filter*/
    diag_util_printf("Filter:\n");
    if(RT_CLS_FILTER_FLOWID_BIT & pRule->filterRule.filterMask)
    {
        diag_util_printf("      FlowId: %d\n", pRule->filterRule.flowId);
    }

    if(RT_CLS_FILTER_OUTER_TAGIf_BIT & pRule->filterRule.filterMask)
    {
        diag_util_printf("    OuterTag: %d\n", pRule->filterRule.outerTagIf);
    }

    if(RT_CLS_FILTER_OUTER_TPID_BIT & pRule->filterRule.filterMask)
    {
        diag_util_printf("   OuterTpid: 0x%04x\n", pRule->filterRule.outerTagTpid);
    }

    if(RT_CLS_FILTER_OUTER_VID_BIT & pRule->filterRule.filterMask)
    {
        diag_util_printf("    OuterVid: %d\n", pRule->filterRule.outerTagVid);
    }

    if(RT_CLS_FILTER_OUTER_PRI_BIT & pRule->filterRule.filterMask)
    {
        diag_util_printf("    OuterPri: %d\n", pRule->filterRule.outerTagPri);
    }

    if(RT_CLS_FILTER_OUTER_DEI_BIT & pRule->filterRule.filterMask)
    {
        diag_util_printf("    OuterDei: %d\n", pRule->filterRule.outerTagDei);
    }

    if(RT_CLS_FILTER_INNER_TAGIf_BIT & pRule->filterRule.filterMask)
    {
        diag_util_printf("    InnerTag: %d\n", pRule->filterRule.innerTagIf);
    }

    if(RT_CLS_FILTER_INNER_TPID_BIT & pRule->filterRule.filterMask)
    {
        diag_util_printf("   InnerTpid: 0x%04x\n", pRule->filterRule.innerTagTpid);
    }

    if(RT_CLS_FILTER_INNER_VID_BIT & pRule->filterRule.filterMask)
    {
        diag_util_printf("    InnerVid: %d\n", pRule->filterRule.innerTagVid);
    }

    if(RT_CLS_FILTER_INNER_PRI_BIT & pRule->filterRule.filterMask)
    {
        diag_util_printf("    InnerPri: %d\n", pRule->filterRule.innerTagPri);
    }

    if(RT_CLS_FILTER_INNER_DEI_BIT & pRule->filterRule.filterMask)
    {
        diag_util_printf("    InnerDei: %d\n", pRule->filterRule.innerTagDei);
    }

    if(RT_CLS_FILTER_ETHERTYPE & pRule->filterRule.filterMask)
    {
        diag_util_printf("   Ethertype: 0x%04x\n", pRule->filterRule.etherType);
    }

    if(RT_CLS_FILTER_DSCP_BIT & pRule->filterRule.filterMask)
    {
        diag_util_printf("        DSCP: 0x");
        for(i=0;i<8;i++)
            diag_util_printf("%02x",pRule->filterRule.dscp[i]);
        diag_util_printf("\n");
    }

    if(RT_CLS_FILTER_EGRESS_PORT_BIT & pRule->filterRule.filterMask)
    {
        diag_util_printf("  EgressPort: 0x%x\n", pRule->filterRule.egressPortMask);
    }

    diag_util_printf("Action:\n");

    switch(pRule->outerTagAct.tagAction)
    {
        case RT_CLS_TAG_TRANSPARENT:
            diag_util_printf("    OuterTag: Transparent\n");
            break;
        case RT_CLS_TAG_MODIFY:
            diag_util_printf("    OuterTag: Modify\n");
            break;
        case RT_CLS_TAG_TAGGING:
            diag_util_printf("    OuterTag: Tagging\n");
            break;
        case RT_CLS_TAG_REMOVE:
            diag_util_printf("    OuterTag: Remove\n");
            break;
        default:
            diag_util_printf("    OuterTag: Unknown action\n");
            break;
    }

    if(pRule->outerTagAct.tagAction != RT_CLS_TAG_TRANSPARENT && pRule->outerTagAct.tagAction != RT_CLS_TAG_REMOVE)
    {
        switch(pRule->outerTagAct.tagTpidAction)
        {
            case RT_CLS_TPID_ASSIGN:
                diag_util_printf("   OuterTpid: Assign 0x%04x\n",pRule->outerTagAct.assignedTpid);
                break;
            case RT_CLS_TPID_COPY_FROM_1ST_TAG:
            diag_util_printf("   OuterTpid: Copy from 1st tag\n");
                break;
            case RT_CLS_TPID_COPY_FROM_2ND_TAG:
            diag_util_printf("   OuterTpid: Copy from 2nd tag\n");
                break;
            default:
            diag_util_printf("   OuterTpid: Unknown tpid action\n");
                break;
        }

        switch(pRule->outerTagAct.tagVidAction)
        {
            case RT_CLS_VID_ASSIGN:
                diag_util_printf("    OuterVid: Assign %d\n",pRule->outerTagAct.assignedVid);
                break;
            case RT_CLS_VID_COPY_FROM_1ST_TAG:
                diag_util_printf("    OuterVid: Copy from 1st tag\n");
                break;
            case RT_CLS_VID_COPY_FROM_2ND_TAG:
                diag_util_printf("    OuterVid: Copy from 2nd tag\n");
                break;
            case RT_CLS_VID_TRANSPARENT:
                diag_util_printf("    OuterVid: Transparent\n");
                break;
            default:
                diag_util_printf("    OuterVid: Unknown vid action\n");
                break;
        }

        switch(pRule->outerTagAct.tagPriAction)
        {
            case RT_CLS_PRI_ASSIGN:
                diag_util_printf("    OuterPri: Assign %d\n",pRule->outerTagAct.assignedPri);
                break;
            case RT_CLS_PRI_COPY_FROM_1ST_TAG:
                diag_util_printf("    OuterPri: Copy from 1st tag\n");
                break;
            case RT_CLS_PRI_COPY_FROM_2ND_TAG:
                diag_util_printf("    OuterPri: Copy from 2nd tag\n");
                break;
            case RT_CLS_PRI_COPY_FROM_DSCP_REMAP:
                diag_util_printf("    OuterPri: DSCP remapping\n");
                break;
            case RT_CLS_PRI_TRANSPARENT:
                diag_util_printf("    OuterPri: Transparent\n");
                break;
            default:
                diag_util_printf("    OuterPri: Unknown pri action\n");
                break;
        }

        switch(pRule->outerTagAct.tagDeiAction)
        {
            case RT_CLS_DEI_ASSIGN:
                diag_util_printf("    OuterDei: Assign %d\n",pRule->outerTagAct.assignedDei);
                break;
            case RT_CLS_DEI_COPY_FROM_1ST_TAG:
                diag_util_printf("    OuterDei: Copy from 1st tag\n");
                break;
            case RT_CLS_DEI_COPY_FROM_2ND_TAG:
                diag_util_printf("    OuterDei: Copy from 2nd tag\n");
                break;
            default:
                diag_util_printf("    OuterDei: Unknown dei action\n");
                break;
        }
    }

    switch(pRule->innerTagAct.tagAction)
    {
        case RT_CLS_TAG_TRANSPARENT:
            diag_util_printf("    InnerTag: Transparent\n");
            break;
        case RT_CLS_TAG_MODIFY:
            diag_util_printf("    InnerTag: Modify\n");
            break;
        case RT_CLS_TAG_TAGGING:
            diag_util_printf("    InnerTag: Tagging\n");
            break;
        case RT_CLS_TAG_REMOVE:
            diag_util_printf("    InnerTag: Remove\n");
            break;
        default:
            diag_util_printf("    InnerTag: Unknown action\n");
            break;
    }

    if(pRule->innerTagAct.tagAction != RT_CLS_TAG_TRANSPARENT && pRule->innerTagAct.tagAction != RT_CLS_TAG_REMOVE)
    {
        switch(pRule->innerTagAct.tagTpidAction)
        {
            case RT_CLS_TPID_ASSIGN:
                diag_util_printf("   InnerTpid: Assign 0x%04x\n",pRule->innerTagAct.assignedTpid);
                break;
            case RT_CLS_TPID_COPY_FROM_1ST_TAG:
            diag_util_printf("   InnerTpid: Copy from 1st tag\n");
                break;
            case RT_CLS_TPID_COPY_FROM_2ND_TAG:
            diag_util_printf("   InnerTpid: Copy from 2nd tag\n");
                break;
            default:
            diag_util_printf("   InnerTpid: Unknown tpid action\n");
                break;
        }

        switch(pRule->innerTagAct.tagVidAction)
        {
            case RT_CLS_VID_ASSIGN:
                diag_util_printf("    InnerVid: Assign %d\n",pRule->innerTagAct.assignedVid);
                break;
            case RT_CLS_VID_COPY_FROM_1ST_TAG:
                diag_util_printf("    InnerVid: Copy from 1st tag\n");
                break;
            case RT_CLS_VID_COPY_FROM_2ND_TAG:
                diag_util_printf("    InnerVid: Copy from 2nd tag\n");
                break;
            case RT_CLS_VID_TRANSPARENT:
                diag_util_printf("    InnerVid: Transparent\n");
                break;
            default:
                diag_util_printf("    InnerVid: Unknown vid action\n");
                break;
        }

        switch(pRule->innerTagAct.tagPriAction)
        {
            case RT_CLS_PRI_ASSIGN:
                diag_util_printf("    InnerPri: Assign %d\n",pRule->innerTagAct.assignedPri);
                break;
            case RT_CLS_PRI_COPY_FROM_1ST_TAG:
                diag_util_printf("    InnerPri: Copy from 1st tag\n");
                break;
            case RT_CLS_PRI_COPY_FROM_2ND_TAG:
                diag_util_printf("    InnerPri: Copy from 2nd tag\n");
                break;
            case RT_CLS_PRI_COPY_FROM_DSCP_REMAP:
                diag_util_printf("    InnerPri: DSCP remapping\n");
                break;
            case RT_CLS_PRI_TRANSPARENT:
                diag_util_printf("    InnerPri: Transparent\n");
                break;
            default:
                diag_util_printf("    InnerPri: Unknown pri action\n");
                break;
        }

        switch(pRule->innerTagAct.tagDeiAction)
        {
            case RT_CLS_DEI_ASSIGN:
                diag_util_printf("    InnerDei: Assign %d\n",pRule->innerTagAct.assignedDei);
                break;
            case RT_CLS_DEI_COPY_FROM_1ST_TAG:
                diag_util_printf("    InnerDei: Copy from 1st tag\n");
                break;
            case RT_CLS_DEI_COPY_FROM_2ND_TAG:
                diag_util_printf("    InnerDei: Copy from 2nd tag\n");
                break;
            default:
                diag_util_printf("    InnerDei: Unknown dei action\n");
                break;
        }
    }

    if(pRule->direction == RT_CLS_DIR_DS)
    {
        if(pRule->dsPriAct == RT_CLS_DS_PRI_ACT_NOP)
        {
            diag_util_printf("    DsPriAct: No action\n");
        }
        else
        {
            diag_util_printf("    DsPriAct: Assign priority %d\n",pRule->dsPri);
        }
    }
    return CPARSER_OK;
}

static uint32 _cls_hwIndex_show(uint16_t *pHwINDEX)
{
    char str[256]={0};
    int i,first=0;
    snprintf(str,256,"HW Index:");
    for(i=0;i<RT_CLS_HW_INDEX_MAX;i++)
    {
        if(pHwINDEX[i] != 0xffff)
        {
            snprintf(str,256,"%s%s %d",str,first ? "," : "" ,pHwINDEX[i]);
            first=1;
        }
    }
    diag_util_printf("%s\n",str);
    return CPARSER_OK;
}

/*
 * rt_cls clear
 */
cparser_result_t
cparser_cmd_rt_cls_clear(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
    memset(&entry, 0, sizeof(rt_cls_rule_t));
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_clear */

/*
 * rt_cls set rule direction ( upstream | downstream )
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_direction_upstream_downstream(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
    if ('u' == TOKEN_CHAR(4,0))
        entry.direction = RT_CLS_DIR_US;
    else
        entry.direction = RT_CLS_DIR_DS;
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_direction_upstream_downstream */

/*
 * rt_cls set rule connection-index <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_connection_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    entry.index = *index_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_connection_index_index */

/*
 * rt_cls set rule flow-id <UINT:flowId>
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_flow_id_flowId(
    cparser_context_t *context,
    uint32_t  *flowId_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*flowId_ptr >= HAL_CLASSIFY_SID_NUM()), CPARSER_ERR_INVALID_PARAMS);
    entry.flowId = *flowId_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_flow_id_flowid */

/*
 * rt_cls set rule ingress-port-mask ( <PORT_LIST:ingressports> | none )
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_ingress_port_mask_ingressports_none(
    cparser_context_t *context,
    char * *ingressports_ptr)
{
    diag_portlist_t portlist;
    int32 ret;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    entry.ingressPortMask = portlist.portmask.bits[0];
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_ingress_port_mask_ingressports_none */

/*
 * rt_cls set rule filter flow-id <UINT:flowId>
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_filter_flow_id_flowId(
    cparser_context_t *context,
    uint32_t  *flowId_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*flowId_ptr >= HAL_CLASSIFY_SID_NUM()), CPARSER_ERR_INVALID_PARAMS);
    entry.filterRule.filterMask |= RT_CLS_FILTER_FLOWID_BIT;
    entry.filterRule.flowId = *flowId_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_filter_flow_id_flowid */

/*
 * rt_cls set rule filter outer-tag <UINT:outerTag> { <UINT:tpid> }
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_filter_outer_tag_outerTag_tpid(
    cparser_context_t *context,
    uint32_t  *outerTag_ptr,
    uint32_t  *tpid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*outerTag_ptr > 1), CPARSER_ERR_INVALID_PARAMS);
    entry.filterRule.filterMask |= RT_CLS_FILTER_OUTER_TAGIf_BIT;
    entry.filterRule.outerTagIf = *outerTag_ptr;
    if(*outerTag_ptr && tpid_ptr != NULL)
    {
        DIAG_UTIL_PARAM_RANGE_CHK((*tpid_ptr > 0xffff), CPARSER_ERR_INVALID_PARAMS);
        entry.filterRule.filterMask |= RT_CLS_FILTER_OUTER_TPID_BIT;
        entry.filterRule.outerTagTpid = *tpid_ptr;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_filter_outer_tag_outertag_tpid */

/*
 * rt_cls set rule filter inner-tag <UINT:innerTag> { <UINT:tpid> }
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_filter_inner_tag_innerTag_tpid(
    cparser_context_t *context,
    uint32_t  *innerTag_ptr,
    uint32_t  *tpid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*innerTag_ptr > 1), CPARSER_ERR_INVALID_PARAMS);
    entry.filterRule.filterMask |= RT_CLS_FILTER_INNER_TAGIf_BIT;
    entry.filterRule.innerTagIf = *innerTag_ptr;
    if(*innerTag_ptr && tpid_ptr != NULL)
    {
        DIAG_UTIL_PARAM_RANGE_CHK((*tpid_ptr > 0xffff), CPARSER_ERR_INVALID_PARAMS);
        entry.filterRule.filterMask |= RT_CLS_FILTER_INNER_TPID_BIT;
        entry.filterRule.innerTagTpid = *tpid_ptr;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_filter_inner_tag_innertag_tpid */

/*
 * rt_cls set rule filter outer-vid <UINT:outerVid>
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_filter_outer_vid_outerVid(
    cparser_context_t *context,
    uint32_t  *outerVid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*outerVid_ptr > 4095), CPARSER_ERR_INVALID_PARAMS);
    entry.filterRule.filterMask |= RT_CLS_FILTER_OUTER_VID_BIT;
    entry.filterRule.outerTagVid = *outerVid_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_filter_outer_vid_outervid */

/*
 * rt_cls set rule filter outer-pri <UINT:outerPri>
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_filter_outer_pri_outerPri(
    cparser_context_t *context,
    uint32_t  *outerPri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*outerPri_ptr > 7), CPARSER_ERR_INVALID_PARAMS);
    entry.filterRule.filterMask |= RT_CLS_FILTER_OUTER_PRI_BIT;
    entry.filterRule.outerTagPri = *outerPri_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_filter_outer_pri_outerpri */

/*
 * rt_cls set rule filter outer-dei <UINT:outerDei>
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_filter_outer_dei_outerDei(
    cparser_context_t *context,
    uint32_t  *outerDei_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*outerDei_ptr > 1), CPARSER_ERR_INVALID_PARAMS);
    entry.filterRule.filterMask |= RT_CLS_FILTER_OUTER_DEI_BIT;
    entry.filterRule.outerTagDei = *outerDei_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_filter_outer_dei_outerdei */

/*
 * rt_cls set rule filter inner-vid <UINT:innerVid>
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_filter_inner_vid_innerVid(
    cparser_context_t *context,
    uint32_t  *innerVid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*innerVid_ptr > 4095), CPARSER_ERR_INVALID_PARAMS);
    entry.filterRule.filterMask |= RT_CLS_FILTER_INNER_VID_BIT;
    entry.filterRule.innerTagVid = *innerVid_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_filter_inner_vid_innervid */

/*
 * rt_cls set rule filter inner-pri <UINT:innerPri>
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_filter_inner_pri_innerPri(
    cparser_context_t *context,
    uint32_t  *innerPri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*innerPri_ptr > 7), CPARSER_ERR_INVALID_PARAMS);
    entry.filterRule.filterMask |= RT_CLS_FILTER_INNER_PRI_BIT;
    entry.filterRule.innerTagPri = *innerPri_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_filter_inner_pri_innerpri */

/*
 * rt_cls set rule filter inner-dei <UINT:innerDei>
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_filter_inner_dei_innerDei(
    cparser_context_t *context,
    uint32_t  *innerDei_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*innerDei_ptr > 1), CPARSER_ERR_INVALID_PARAMS);
    entry.filterRule.filterMask |= RT_CLS_FILTER_INNER_DEI_BIT;
    entry.filterRule.innerTagDei = *innerDei_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_filter_inner_dei_innerdei */

/*
 * rt_cls set rule filter ethertype <UINT:ethertype>
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_filter_ethertype_ethertype(
    cparser_context_t *context,
    uint32_t  *ethertype_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*ethertype_ptr > 0xffff), CPARSER_ERR_INVALID_PARAMS);
    entry.filterRule.filterMask |= RT_CLS_FILTER_ETHERTYPE;
    entry.filterRule.etherType = *ethertype_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_filter_ethertype_ethertype */

/*
 * rt_cls set rule filter dscp <UINT:dscp>
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_filter_dscp_dscp(
    cparser_context_t *context,
    uint32_t  *dscp_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*dscp_ptr > 63), CPARSER_ERR_INVALID_PARAMS);
    entry.filterRule.filterMask |= RT_CLS_FILTER_DSCP_BIT;
    entry.filterRule.dscp[*dscp_ptr/8] = (1 << (7-(*dscp_ptr%8)));
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_filter_dscp_dscp */

/*
 * rt_cls set rule filter egress-port <UINT:egressPort>
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_filter_egress_port_egressPort(
    cparser_context_t *context,
    uint32_t  *egressPort_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    entry.filterRule.filterMask |= RT_CLS_FILTER_EGRESS_PORT_BIT;
    entry.filterRule.egressPortMask = *egressPort_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_filter_egress_port_egressport */

/*
 * rt_cls set rule action outer-tag ( transparent | tagging | modify | remove )
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_action_outer_tag_transparent_tagging_modify_remove(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
    if(0 == osal_strcmp("transparent", TOKEN_STR(5)))
        entry.outerTagAct.tagAction = RT_CLS_TAG_TRANSPARENT;
    else if(0 == osal_strcmp("tagging", TOKEN_STR(5)))
        entry.outerTagAct.tagAction = RT_CLS_TAG_TAGGING;
    else if('m' == TOKEN_CHAR(5,0))
        entry.outerTagAct.tagAction = RT_CLS_TAG_MODIFY;
    else if('r' == TOKEN_CHAR(5,0))
        entry.outerTagAct.tagAction = RT_CLS_TAG_REMOVE;
    else
        return CPARSER_ERR_INVALID_PARAMS;

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_action_outer_tag_transparent_tagging_modify_remove */

/*
 * rt_cls set rule action outer-tpid ( assign | copy-outer | copy-inner ) { <UINT:tpid> }
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_action_outer_tpid_assign_copy_outer_copy_inner_tpid(
    cparser_context_t *context,
    uint32_t  *tpid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    if('a' == TOKEN_CHAR(5,0))
        entry.outerTagAct.tagTpidAction = RT_CLS_TPID_ASSIGN;
    else if(0 == osal_strcmp("copy-outer", TOKEN_STR(5)))
        entry.outerTagAct.tagTpidAction = RT_CLS_TPID_COPY_FROM_1ST_TAG;
    else if(0 == osal_strcmp("copy-inner", TOKEN_STR(5)))
        entry.outerTagAct.tagTpidAction = RT_CLS_TPID_COPY_FROM_2ND_TAG;
    else
        return CPARSER_ERR_INVALID_PARAMS;
    if(tpid_ptr != NULL)
    {
        DIAG_UTIL_PARAM_RANGE_CHK((*tpid_ptr > 0xffff), CPARSER_ERR_INVALID_PARAMS);
        entry.outerTagAct.assignedTpid = *tpid_ptr;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_action_outer_tpid_assign_copy_outer_copy_inner_tpid */

/*
 * rt_cls set rule action outer-vid ( assign | copy-outer | copy-inner | transparent ) { <UINT:vid> }
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_action_outer_vid_assign_copy_outer_copy_inner_transparent_vid(
    cparser_context_t *context,
    uint32_t  *vid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    if('a' == TOKEN_CHAR(5,0))
        entry.outerTagAct.tagVidAction = RT_CLS_VID_ASSIGN;
    else if(0 == osal_strcmp("copy-outer", TOKEN_STR(5)))
        entry.outerTagAct.tagVidAction = RT_CLS_VID_COPY_FROM_1ST_TAG;
    else if(0 == osal_strcmp("copy-inner", TOKEN_STR(5)))
        entry.outerTagAct.tagVidAction = RT_CLS_VID_COPY_FROM_2ND_TAG;
    else if('t' == TOKEN_CHAR(5,0))
        entry.outerTagAct.tagVidAction = RT_CLS_VID_TRANSPARENT;
    else
        return CPARSER_ERR_INVALID_PARAMS;
    if(vid_ptr != NULL)
    {
        DIAG_UTIL_PARAM_RANGE_CHK((*vid_ptr > 4095), CPARSER_ERR_INVALID_PARAMS);
        entry.outerTagAct.assignedVid = *vid_ptr;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_action_outer_vid_assign_copy_outer_copy_inner_transparent_vid */


/*
 * rt_cls set rule action outer-pri ( assign | copy-outer | copy-inner | dscp-remapping | transparent ) { <UINT:pri> }
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_action_outer_pri_assign_copy_outer_copy_inner_dscp_remapping_transparent_pri(
    cparser_context_t *context,
    uint32_t  *pri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    if('a' == TOKEN_CHAR(5,0))
        entry.outerTagAct.tagPriAction = RT_CLS_PRI_ASSIGN;
    else if(0 == osal_strcmp("copy-outer", TOKEN_STR(5)))
        entry.outerTagAct.tagPriAction = RT_CLS_PRI_COPY_FROM_1ST_TAG;
    else if(0 == osal_strcmp("copy-inner", TOKEN_STR(5)))
        entry.outerTagAct.tagPriAction = RT_CLS_PRI_COPY_FROM_2ND_TAG;
    else if('d' == TOKEN_CHAR(5,0))
        entry.outerTagAct.tagPriAction = RT_CLS_PRI_COPY_FROM_DSCP_REMAP;
    else if('t' == TOKEN_CHAR(5,0))
        entry.outerTagAct.tagPriAction = RT_CLS_PRI_TRANSPARENT;
    else
        return CPARSER_ERR_INVALID_PARAMS;
    if(pri_ptr != NULL)
    {
        DIAG_UTIL_PARAM_RANGE_CHK((*pri_ptr > 7), CPARSER_ERR_INVALID_PARAMS);
        entry.outerTagAct.assignedPri = *pri_ptr;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_action_outer_pri_assign_copy_outer_copy_inner_dscp_remapping_transparent_pri */

/*
 * rt_cls set rule action outer-dei ( assign | copy-outer | copy-inner ) { <UINT:dei> }
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_action_outer_dei_assign_copy_outer_copy_inner_dei(
    cparser_context_t *context,
    uint32_t  *dei_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    if('a' == TOKEN_CHAR(5,0))
        entry.outerTagAct.tagDeiAction = RT_CLS_DEI_ASSIGN;
    else if(0 == osal_strcmp("copy-outer", TOKEN_STR(5)))
        entry.outerTagAct.tagDeiAction = RT_CLS_DEI_COPY_FROM_1ST_TAG;
    else if(0 == osal_strcmp("copy-inner", TOKEN_STR(5)))
        entry.outerTagAct.tagDeiAction = RT_CLS_DEI_COPY_FROM_2ND_TAG;
    else
        return CPARSER_ERR_INVALID_PARAMS;
    if(dei_ptr != NULL)
    {
        DIAG_UTIL_PARAM_RANGE_CHK((*dei_ptr > 1), CPARSER_ERR_INVALID_PARAMS);
        entry.outerTagAct.assignedDei = *dei_ptr;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_action_outer_dei_assign_copy_outer_copy_inner_dei */

/*
 * rt_cls set rule action inner-tag ( transparent | tagging | modify | remove )
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_action_inner_tag_transparent_tagging_modify_remove(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
    if(0 == osal_strcmp("transparent", TOKEN_STR(5)))
        entry.innerTagAct.tagAction = RT_CLS_TAG_TRANSPARENT;
    else if(0 == osal_strcmp("tagging", TOKEN_STR(5)))
        entry.innerTagAct.tagAction = RT_CLS_TAG_TAGGING;
    else if('m' == TOKEN_CHAR(5,0))
        entry.innerTagAct.tagAction = RT_CLS_TAG_MODIFY;
    else if('r' == TOKEN_CHAR(5,0))
        entry.innerTagAct.tagAction = RT_CLS_TAG_REMOVE;
    else
        return CPARSER_ERR_INVALID_PARAMS;
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_action_inner_tag_transparent_tagging_modify_remove */

/*
 * rt_cls set rule action inner-tpid ( assign | copy-outer | copy-inner ) { <UINT:tpid> }
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_action_inner_tpid_assign_copy_outer_copy_inner_tpid(
    cparser_context_t *context,
    uint32_t  *tpid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    if('a' == TOKEN_CHAR(5,0))
        entry.innerTagAct.tagTpidAction = RT_CLS_TPID_ASSIGN;
    else if(0 == osal_strcmp("copy-outer", TOKEN_STR(5)))
        entry.innerTagAct.tagTpidAction = RT_CLS_TPID_COPY_FROM_1ST_TAG;
    else if(0 == osal_strcmp("copy-inner", TOKEN_STR(5)))
        entry.innerTagAct.tagTpidAction = RT_CLS_TPID_COPY_FROM_2ND_TAG;
    else
        return CPARSER_ERR_INVALID_PARAMS;
    if(tpid_ptr != NULL)
    {
        DIAG_UTIL_PARAM_RANGE_CHK((*tpid_ptr > 0xffff), CPARSER_ERR_INVALID_PARAMS);
        entry.innerTagAct.assignedTpid = *tpid_ptr;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_action_inner_tpid_assign_copy_outer_copy_inner_tpid */

/*
 * rt_cls set rule action inner-vid ( assign | copy-outer | copy-inner | transparent ) { <UINT:vid> }
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_action_inner_vid_assign_copy_outer_copy_inner_transparent_vid(
    cparser_context_t *context,
    uint32_t  *vid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    if('a' == TOKEN_CHAR(5,0))
        entry.innerTagAct.tagVidAction = RT_CLS_VID_ASSIGN;
    else if(0 == osal_strcmp("copy-outer", TOKEN_STR(5)))
        entry.innerTagAct.tagVidAction = RT_CLS_VID_COPY_FROM_1ST_TAG;
    else if(0 == osal_strcmp("copy-inner", TOKEN_STR(5)))
        entry.innerTagAct.tagVidAction = RT_CLS_VID_COPY_FROM_2ND_TAG;
    else if('t' == TOKEN_CHAR(5,0))
        entry.innerTagAct.tagVidAction = RT_CLS_VID_TRANSPARENT;
    else
        return CPARSER_ERR_INVALID_PARAMS;
    if(vid_ptr != NULL)
    {
        DIAG_UTIL_PARAM_RANGE_CHK((*vid_ptr > 4095), CPARSER_ERR_INVALID_PARAMS);
        entry.innerTagAct.assignedVid = *vid_ptr;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_action_inner_vid_assign_copy_outer_copy_inner_transparent_vid */

/*
 * rt_cls set rule action inner-pri ( assign | copy-outer | copy-inner | dscp-remapping | transparent ) { <UINT:pri> }
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_action_inner_pri_assign_copy_outer_copy_inner_dscp_remapping_transparent_pri(
    cparser_context_t *context,
    uint32_t  *pri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    if('a' == TOKEN_CHAR(5,0))
        entry.innerTagAct.tagPriAction = RT_CLS_PRI_ASSIGN;
    else if(0 == osal_strcmp("copy-outer", TOKEN_STR(5)))
        entry.innerTagAct.tagPriAction = RT_CLS_PRI_COPY_FROM_1ST_TAG;
    else if(0 == osal_strcmp("copy-inner", TOKEN_STR(5)))
        entry.innerTagAct.tagPriAction = RT_CLS_PRI_COPY_FROM_2ND_TAG;
    else if('d' == TOKEN_CHAR(5,0))
        entry.innerTagAct.tagPriAction = RT_CLS_PRI_COPY_FROM_DSCP_REMAP;
    else if('t' == TOKEN_CHAR(5,0))
        entry.innerTagAct.tagPriAction = RT_CLS_PRI_TRANSPARENT;
    else
        return CPARSER_ERR_INVALID_PARAMS;
    if(pri_ptr != NULL)
    {
        DIAG_UTIL_PARAM_RANGE_CHK((*pri_ptr > 7), CPARSER_ERR_INVALID_PARAMS);
        entry.innerTagAct.assignedPri = *pri_ptr;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_action_inner_pri_assign_copy_outer_copy_inner_dscp_remapping_transparent_pri */

/*
 * rt_cls set rule action inner-dei ( assign | copy-outer | copy-inner ) { <UINT:dei> }
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_action_inner_dei_assign_copy_outer_copy_inner_dei(
    cparser_context_t *context,
    uint32_t  *dei_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    if('a' == TOKEN_CHAR(5,0))
        entry.innerTagAct.tagDeiAction = RT_CLS_DEI_ASSIGN;
    else if(0 == osal_strcmp("copy-outer", TOKEN_STR(5)))
        entry.innerTagAct.tagDeiAction = RT_CLS_DEI_COPY_FROM_1ST_TAG;
    else if(0 == osal_strcmp("copy-inner", TOKEN_STR(5)))
        entry.innerTagAct.tagDeiAction = RT_CLS_DEI_COPY_FROM_2ND_TAG;
    else
        return CPARSER_ERR_INVALID_PARAMS;
    if(dei_ptr != NULL)
    {
        DIAG_UTIL_PARAM_RANGE_CHK((*dei_ptr > 1), CPARSER_ERR_INVALID_PARAMS);
        entry.innerTagAct.assignedDei = *dei_ptr;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_action_inner_dei_assign_copy_outer_copy_inner_dei */

/*
 * rt_cls set rule action downstream-pri-act ( assign | nop ) { <UINT:pri> }
 */
cparser_result_t
cparser_cmd_rt_cls_set_rule_action_downstream_pri_act_assign_nop_pri(
    cparser_context_t *context,
    uint32_t  *pri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    if('a' == TOKEN_CHAR(5,0))
        entry.dsPriAct =  RT_CLS_DS_PRI_ACT_ASSIGN;
    else if('n' == TOKEN_CHAR(5,0))
        entry.dsPriAct = RT_CLS_DS_PRI_ACT_NOP;
    else
        return CPARSER_ERR_INVALID_PARAMS;
    if(pri_ptr != NULL)
    {
        DIAG_UTIL_PARAM_RANGE_CHK((*pri_ptr > 7), CPARSER_ERR_INVALID_PARAMS);
        entry.dsPri = *pri_ptr;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_rule_action_downstream_pri_act_assign_nop_pri */

/*
 * rt_cls init
 */
cparser_result_t
cparser_cmd_rt_cls_init(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rt_cls_init(), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_init */

/*
 * rt_cls add rule
 */
cparser_result_t
cparser_cmd_rt_cls_add_rule(
    cparser_context_t *context)
{
    int32 ret;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(rt_cls_rule_add(&entry), ret);
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_add_rule */

/*
 * rt_cls delete rule connection-index <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_cls_delete_rule_connection_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(rt_cls_rule_delete(*index_ptr), ret);
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_delete_rule_connection_index_index */

/*
 * rt_cls show rule */
cparser_result_t
cparser_cmd_rt_cls_show_rule(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    _cls_rule_show(&entry);
    
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_show_rule */

/*
 * rt_cls set port ( <PORT_LIST:ports> | all ) fwdPort <UINT:fwdPort> */
cparser_result_t
cparser_cmd_rt_cls_set_port_ports_all_fwdPort_fwdPort(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *fwdPort_ptr)
{
    int32 ret;
    rt_port_t port;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {

        DIAG_UTIL_ERR_CHK(rt_cls_fwdPort_set(port,*fwdPort_ptr), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_set_port_ports_all_fwdport_fwdport */

/*
 * rt_cls get entry <UINT:index>
 */
cparser_result_t
cparser_cmd_rt_cls_get_entry_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    int32 ret,found=0;
    rt_cls_rule_info_t ruleInfo;
    uint32_t index;

    memset(&ruleInfo,0,sizeof(rt_cls_rule_info_t));
    memset(ruleInfo.hwIndex,0xff,sizeof(uint16_t)*RT_CLS_HW_INDEX_MAX);
    ret = rt_cls_rule_get(RT_CLS_GET_OP_INDEX, *index_ptr, RT_CLS_DIR_DS, &ruleInfo);
    if(ret == RT_ERR_OK)
    {
        found=1;
        diag_util_mprintf("***************************************\n");
        _cls_rule_show(&ruleInfo.rule);
        _cls_hwIndex_show(ruleInfo.hwIndex);
    }
    
    memset(&ruleInfo,0,sizeof(rt_cls_rule_info_t));
    memset(ruleInfo.hwIndex,0xff,sizeof(uint16_t)*RT_CLS_HW_INDEX_MAX);
    ret = rt_cls_rule_get(RT_CLS_GET_OP_INDEX, *index_ptr, RT_CLS_DIR_US, &ruleInfo);
    if(ret == RT_ERR_OK)
    {
        found=1;
        diag_util_mprintf("***************************************\n");
        _cls_rule_show(&ruleInfo.rule);
        _cls_hwIndex_show(ruleInfo.hwIndex);
    }
    
    if(found == 0)
        diag_util_mprintf("rt_cls rule not found\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_get_entry_index */

/*
 * rt_cls get entry valid
 */
cparser_result_t
cparser_cmd_rt_cls_get_entry_valid(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    int32 ret,found=0;
    rt_cls_rule_info_t ruleInfo;
    uint32_t index;

    memset(&ruleInfo,0,sizeof(rt_cls_rule_info_t));
    memset(ruleInfo.hwIndex,0xff,sizeof(uint16_t)*RT_CLS_HW_INDEX_MAX);
    //_cls_hwIndex_show(ruleInfo.hwIndex);
    ret = rt_cls_rule_get(RT_CLS_GET_OP_FIRST, 0, RT_CLS_DIR_DS, &ruleInfo);
    while(ret == RT_ERR_OK)
    {
        found=1;
        diag_util_mprintf("***************************************\n");
        _cls_rule_show(&ruleInfo.rule);
        _cls_hwIndex_show(ruleInfo.hwIndex);
        index = ruleInfo.rule.index;
        memset(&ruleInfo,0,sizeof(rt_cls_rule_info_t));
        memset(ruleInfo.hwIndex,0xff,sizeof(uint16_t)*RT_CLS_HW_INDEX_MAX);
        ret = rt_cls_rule_get(RT_CLS_GET_OP_NEXT, index, RT_CLS_DIR_DS, &ruleInfo);
    }

    memset(&ruleInfo,0,sizeof(rt_cls_rule_info_t));
    memset(ruleInfo.hwIndex,0xff,sizeof(uint16_t)*RT_CLS_HW_INDEX_MAX);
    ret = rt_cls_rule_get(RT_CLS_GET_OP_FIRST, 0, RT_CLS_DIR_US, &ruleInfo);
    while(ret == RT_ERR_OK)
    {
        found=1;
        diag_util_mprintf("***************************************\n");
        _cls_rule_show(&ruleInfo.rule);
        _cls_hwIndex_show(ruleInfo.hwIndex);
        index = ruleInfo.rule.index;
        memset(&ruleInfo,0,sizeof(rt_cls_rule_info_t));
        memset(ruleInfo.hwIndex,0xff,sizeof(uint16_t)*RT_CLS_HW_INDEX_MAX);
        ret = rt_cls_rule_get(RT_CLS_GET_OP_NEXT, index, RT_CLS_DIR_US, &ruleInfo);
    }

    if(found == 0)
        diag_util_mprintf("No any rt_cls rule\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_cls_get_entry_valid */

