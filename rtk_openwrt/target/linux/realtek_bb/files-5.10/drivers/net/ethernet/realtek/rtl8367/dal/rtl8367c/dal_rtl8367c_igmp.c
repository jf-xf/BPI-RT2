/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision$
 * $Date$
 *
 * Purpose : RTK switch high-level API for RTL8367/RTL8367C
 * Feature : Here is a list of all functions and variables in IGMP module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal/rtl8367c/dal_rtl8367c_igmp.h>
#include <string.h>

#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_igmp.h>
#include <rtl8367c_asicdrv_lut.h>

/* Function Name:
 *      rtksw_igmp_init
 * Description:
 *      This API enables H/W IGMP and set a default initial configuration.
 * Input:
 *      None.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API enables H/W IGMP and set a default initial configuration.
 */
rtksw_api_ret_t dal_rtl8367c_igmp_init(void)
{
    rtksw_api_ret_t retVal;
    rtksw_port_t port;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if ((retVal = rtl8367c_setAsicLutIpMulticastLookup(RTKSW_ENABLED))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicLutIpLookupMethod(1))!=RT_ERR_OK)
        return retVal;

    RTKSW_SCAN_ALL_PHY_PORTMASK(port)
    {
        if ((retVal = rtl8367c_setAsicIGMPv1Opeartion(port, PROTOCOL_OP_ASIC))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicIGMPv2Opeartion(port, PROTOCOL_OP_ASIC))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicIGMPv3Opeartion(port, PROTOCOL_OP_FLOOD))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicMLDv1Opeartion(port, PROTOCOL_OP_ASIC))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicMLDv2Opeartion(port, PROTOCOL_OP_FLOOD))!=RT_ERR_OK)
            return retVal;
    }

    if ((retVal = rtl8367c_setAsicIGMPAllowDynamicRouterPort(rtksw_switch_phyPortMask_get()))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicIGMPFastLeaveEn(RTKSW_ENABLED))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicIGMPReportLeaveFlood(1))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicIgmp(RTKSW_ENABLED))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_state_set
 * Description:
 *      This API set H/W IGMP state.
 * Input:
 *      enabled     - H/W IGMP state
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error parameter
 * Note:
 *      This API set H/W IGMP state.
 */
rtksw_api_ret_t dal_rtl8367c_igmp_state_set(rtksw_enable_t enabled)
{
    rtksw_api_ret_t retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if (enabled >= RTKSW_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicIgmp(enabled))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_state_get
 * Description:
 *      This API get H/W IGMP state.
 * Input:
 *      None.
 * Output:
 *      pEnabled        - H/W IGMP state
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error parameter
 * Note:
 *      This API set current H/W IGMP state.
 */
rtksw_api_ret_t dal_rtl8367c_igmp_state_get(rtksw_enable_t *pEnabled)
{
    rtksw_api_ret_t retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(pEnabled == NULL)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicIgmp(pEnabled))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_static_router_port_set
 * Description:
 *      Configure static router port
 * Input:
 *      pPortmask    - Static Port mask
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *      This API set static router port
 */
rtksw_api_ret_t dal_rtl8367c_igmp_static_router_port_set(rtksw_portmask_t *pPortmask)
{
    rtksw_api_ret_t retVal;
    rtksw_uint32 pmask;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    /* Check Valid port mask */
    if(pPortmask == NULL)
        return RT_ERR_NULL_POINTER;

    RTKSW_CHK_PORTMASK_VALID(pPortmask);

    if ((retVal = rtksw_switch_portmask_L2P_get(pPortmask, &pmask))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicIGMPStaticRouterPort(pmask))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_static_router_port_get
 * Description:
 *      Get static router port
 * Input:
 *      None.
 * Output:
 *      pPortmask       - Static port mask
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *      This API get static router port
 */
rtksw_api_ret_t dal_rtl8367c_igmp_static_router_port_get(rtksw_portmask_t *pPortmask)
{
    rtksw_api_ret_t retVal;
    rtksw_uint32 pmask;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(pPortmask == NULL)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicIGMPStaticRouterPort(&pmask))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtksw_switch_portmask_P2L_get(pmask, pPortmask))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_protocol_set
 * Description:
 *      set IGMP/MLD protocol action
 * Input:
 *      port        - Port ID
 *      protocol    - IGMP/MLD protocol
 *      action      - Per-port and per-protocol IGMP action seeting
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *      This API set IGMP/MLD protocol action
 */
rtksw_api_ret_t dal_rtl8367c_igmp_protocol_set(rtksw_port_t port, rtksw_igmp_protocol_t protocol, rtksw_igmp_action_t action)
{
    rtksw_uint32      operation;
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    /* Check port valid */
    RTKSW_CHK_PORT_VALID(port);

    if(protocol >= PROTOCOL_END)
        return RT_ERR_INPUT;

    if(action >= IGMP_ACTION_END)
        return RT_ERR_INPUT;

    switch(action)
    {
        case IGMP_ACTION_FORWARD:
            operation = PROTOCOL_OP_FLOOD;
            break;
        case IGMP_ACTION_TRAP2CPU:
            operation = PROTOCOL_OP_TRAP;
            break;
        case IGMP_ACTION_DROP:
            operation = PROTOCOL_OP_DROP;
            break;
        case IGMP_ACTION_ASIC:
            operation = PROTOCOL_OP_ASIC;
            break;
        default:
            return RT_ERR_INPUT;
    }

    switch(protocol)
    {
        case PROTOCOL_IGMPv1:
            if ((retVal = rtl8367c_setAsicIGMPv1Opeartion(rtksw_switch_port_L2P_get(port), operation))!=RT_ERR_OK)
                return retVal;

            break;
        case PROTOCOL_IGMPv2:
            if ((retVal = rtl8367c_setAsicIGMPv2Opeartion(rtksw_switch_port_L2P_get(port), operation))!=RT_ERR_OK)
                return retVal;

            break;
        case PROTOCOL_IGMPv3:
            if ((retVal = rtl8367c_setAsicIGMPv3Opeartion(rtksw_switch_port_L2P_get(port), operation))!=RT_ERR_OK)
                return retVal;

            break;
        case PROTOCOL_MLDv1:
            if ((retVal = rtl8367c_setAsicMLDv1Opeartion(rtksw_switch_port_L2P_get(port), operation))!=RT_ERR_OK)
                return retVal;

            break;
        case PROTOCOL_MLDv2:
            if ((retVal = rtl8367c_setAsicMLDv2Opeartion(rtksw_switch_port_L2P_get(port), operation))!=RT_ERR_OK)
                return retVal;

            break;
        default:
            return RT_ERR_INPUT;

    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_protocol_get
 * Description:
 *      set IGMP/MLD protocol action
 * Input:
 *      port        - Port ID
 *      protocol    - IGMP/MLD protocol
 *      action      - Per-port and per-protocol IGMP action seeting
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *      This API set IGMP/MLD protocol action
 */
rtksw_api_ret_t dal_rtl8367c_igmp_protocol_get(rtksw_port_t port, rtksw_igmp_protocol_t protocol, rtksw_igmp_action_t *pAction)
{
    rtksw_uint32      operation;
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    /* Check port valid */
    RTKSW_CHK_PORT_VALID(port);

    if(protocol >= PROTOCOL_END)
        return RT_ERR_INPUT;

    if(pAction == NULL)
        return RT_ERR_NULL_POINTER;

    switch(protocol)
    {
        case PROTOCOL_IGMPv1:
            if ((retVal = rtl8367c_getAsicIGMPv1Opeartion(rtksw_switch_port_L2P_get(port), &operation))!=RT_ERR_OK)
                return retVal;

            break;
        case PROTOCOL_IGMPv2:
            if ((retVal = rtl8367c_getAsicIGMPv2Opeartion(rtksw_switch_port_L2P_get(port), &operation))!=RT_ERR_OK)
                return retVal;

            break;
        case PROTOCOL_IGMPv3:
            if ((retVal = rtl8367c_getAsicIGMPv3Opeartion(rtksw_switch_port_L2P_get(port), &operation))!=RT_ERR_OK)
                return retVal;

            break;
        case PROTOCOL_MLDv1:
            if ((retVal = rtl8367c_getAsicMLDv1Opeartion(rtksw_switch_port_L2P_get(port), &operation))!=RT_ERR_OK)
                return retVal;

            break;
        case PROTOCOL_MLDv2:
            if ((retVal = rtl8367c_getAsicMLDv2Opeartion(rtksw_switch_port_L2P_get(port), &operation))!=RT_ERR_OK)
                return retVal;

            break;
        default:
            return RT_ERR_INPUT;

    }

    switch(operation)
    {
        case PROTOCOL_OP_FLOOD:
            *pAction = IGMP_ACTION_FORWARD;
            break;
        case PROTOCOL_OP_TRAP:
            *pAction = IGMP_ACTION_TRAP2CPU;
            break;
        case PROTOCOL_OP_DROP:
            *pAction = IGMP_ACTION_DROP;
            break;
        case PROTOCOL_OP_ASIC:
            *pAction = IGMP_ACTION_ASIC;
            break;
        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_fastLeave_set
 * Description:
 *      set IGMP/MLD FastLeave state
 * Input:
 *      state       - RTKSW_ENABLED: Enable FastLeave, RTKSW_DISABLED: disable FastLeave
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API set IGMP/MLD FastLeave state
 */
rtksw_api_ret_t dal_rtl8367c_igmp_fastLeave_set(rtksw_enable_t state)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(state >= RTKSW_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicIGMPFastLeaveEn((rtksw_uint32)state))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_fastLeave_get
 * Description:
 *      get IGMP/MLD FastLeave state
 * Input:
 *      None
 * Output:
 *      pState      - RTKSW_ENABLED: Enable FastLeave, RTKSW_DISABLED: disable FastLeave
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_NULL_POINTER    - NULL pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API get IGMP/MLD FastLeave state
 */
rtksw_api_ret_t dal_rtl8367c_igmp_fastLeave_get(rtksw_enable_t *pState)
{
    rtksw_uint32      fast_leave;
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(pState == NULL)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicIGMPFastLeaveEn(&fast_leave))!=RT_ERR_OK)
        return retVal;

    *pState = ((fast_leave == 1) ? RTKSW_ENABLED : RTKSW_DISABLED);
    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_maxGroup_set
 * Description:
 *      Set per port multicast group learning limit.
 * Input:
 *      port        - Port ID
 *      group       - The number of multicast group learning limit.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_PORT_ID         - Error Port ID
 *      RT_ERR_OUT_OF_RANGE    - parameter out of range
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API set per port multicast group learning limit.
 */
rtksw_api_ret_t dal_rtl8367c_igmp_maxGroup_set(rtksw_port_t port, rtksw_uint32 group)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    /* Check port valid */
    RTKSW_CHK_PORT_VALID(port);

    if(group > RTL8367C_IGMP_MAX_GOUP)
        return RT_ERR_OUT_OF_RANGE;

    if ((retVal = rtl8367c_setAsicIGMPPortMAXGroup(rtksw_switch_port_L2P_get(port), group))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_maxGroup_get
 * Description:
 *      Get per port multicast group learning limit.
 * Input:
 *      port        - Port ID
 * Output:
 *      pGroup      - The number of multicast group learning limit.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_PORT_ID         - Error Port ID
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API get per port multicast group learning limit.
 */
rtksw_api_ret_t dal_rtl8367c_igmp_maxGroup_get(rtksw_port_t port, rtksw_uint32 *pGroup)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    /* Check port valid */
    RTKSW_CHK_PORT_VALID(port);

    if(pGroup == NULL)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicIGMPPortMAXGroup(rtksw_switch_port_L2P_get(port), pGroup))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_currentGroup_get
 * Description:
 *      Get per port multicast group learning count.
 * Input:
 *      port        - Port ID
 * Output:
 *      pGroup      - The number of multicast group learning count.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_PORT_ID         - Error Port ID
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API get per port multicast group learning count.
 */
rtksw_api_ret_t dal_rtl8367c_igmp_currentGroup_get(rtksw_port_t port, rtksw_uint32 *pGroup)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    /* Check port valid */
    RTKSW_CHK_PORT_VALID(port);

    if(pGroup == NULL)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicIGMPPortCurrentGroup(rtksw_switch_port_L2P_get(port), pGroup))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_tableFullAction_set
 * Description:
 *      set IGMP/MLD Table Full Action
 * Input:
 *      action      - Table Full Action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
rtksw_api_ret_t dal_rtl8367c_igmp_tableFullAction_set(rtksw_igmp_tableFullAction_t action)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(action >= IGMP_TABLE_FULL_OP_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicIGMPTableFullOP((rtksw_uint32)action))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_tableFullAction_get
 * Description:
 *      get IGMP/MLD Table Full Action
 * Input:
 *      None
 * Output:
 *      pAction     - Table Full Action
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
rtksw_api_ret_t dal_rtl8367c_igmp_tableFullAction_get(rtksw_igmp_tableFullAction_t *pAction)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(NULL == pAction)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicIGMPTableFullOP((rtksw_uint32 *)pAction))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_checksumErrorAction_set
 * Description:
 *      set IGMP/MLD Checksum Error Action
 * Input:
 *      action      - Checksum error Action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
rtksw_api_ret_t dal_rtl8367c_igmp_checksumErrorAction_set(rtksw_igmp_checksumErrorAction_t action)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(action >= IGMP_CRC_ERR_OP_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicIGMPCRCErrOP((rtksw_uint32)action))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_checksumErrorAction_get
 * Description:
 *      get IGMP/MLD Checksum Error Action
 * Input:
 *      None
 * Output:
 *      pAction     - Checksum error Action
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
rtksw_api_ret_t dal_rtl8367c_igmp_checksumErrorAction_get(rtksw_igmp_checksumErrorAction_t *pAction)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(NULL == pAction)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicIGMPCRCErrOP((rtksw_uint32 *)pAction))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_leaveTimer_set
 * Description:
 *      set IGMP/MLD Leave timer
 * Input:
 *      timer       - Leave timer
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
rtksw_api_ret_t dal_rtl8367c_igmp_leaveTimer_set(rtksw_uint32 timer)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(timer > RTL8367C_MAX_LEAVE_TIMER)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicIGMPLeaveTimer(timer))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_leaveTimer_get
 * Description:
 *      get IGMP/MLD Leave timer
 * Input:
 *      None
 * Output:
 *      pTimer      - Leave Timer.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
rtksw_api_ret_t dal_rtl8367c_igmp_leaveTimer_get(rtksw_uint32 *pTimer)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(NULL == pTimer)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicIGMPLeaveTimer(pTimer))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_queryInterval_set
 * Description:
 *      set IGMP/MLD Query Interval
 * Input:
 *      interval     - Query Interval
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
rtksw_api_ret_t dal_rtl8367c_igmp_queryInterval_set(rtksw_uint32 interval)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(interval > RTL8367C_MAX_QUERY_INT)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicIGMPQueryInterval(interval))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_queryInterval_get
 * Description:
 *      get IGMP/MLD Query Interval
 * Input:
 *      None.
 * Output:
 *      pInterval   - Query Interval
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
rtksw_api_ret_t dal_rtl8367c_igmp_queryInterval_get(rtksw_uint32 *pInterval)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(NULL == pInterval)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicIGMPQueryInterval(pInterval))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_robustness_set
 * Description:
 *      set IGMP/MLD Robustness value
 * Input:
 *      robustness     - Robustness value
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
rtksw_api_ret_t dal_rtl8367c_igmp_robustness_set(rtksw_uint32 robustness)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(robustness > RTL8367C_MAX_ROB_VAR)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicIGMPRobVar(robustness))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_robustness_get
 * Description:
 *      get IGMP/MLD Robustness value
 * Input:
 *      None
 * Output:
 *      pRobustness     - Robustness value.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
rtksw_api_ret_t dal_rtl8367c_igmp_robustness_get(rtksw_uint32 *pRobustness)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(NULL == pRobustness)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicIGMPRobVar(pRobustness))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_dynamicRouterRortAllow_set
 * Description:
 *      Configure dynamic router port allow option
 * Input:
 *      pPortmask    - Dynamic Port allow mask
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *
 */
rtksw_api_ret_t dal_rtl8367c_igmp_dynamicRouterPortAllow_set(rtksw_portmask_t *pPortmask)
{
    rtksw_api_ret_t   retVal;
    rtksw_uint32 pmask;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    RTKSW_CHK_PORTMASK_VALID(pPortmask);

    if ((retVal = rtksw_switch_portmask_L2P_get(pPortmask, &pmask))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicIGMPAllowDynamicRouterPort(pmask))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_dynamicRouterRortAllow_get
 * Description:
 *      Get dynamic router port allow option
 * Input:
 *      None.
 * Output:
 *      pPortmask    - Dynamic Port allow mask
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *
 */
rtksw_api_ret_t dal_rtl8367c_igmp_dynamicRouterPortAllow_get(rtksw_portmask_t *pPortmask)
{
    rtksw_api_ret_t   retVal;
    rtksw_uint32 pmask;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicIGMPAllowDynamicRouterPort(&pmask))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtksw_switch_portmask_P2L_get(pmask, pPortmask))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_dynamicRouterPort_get
 * Description:
 *      Get dynamic router port
 * Input:
 *      None.
 * Output:
 *      pDynamicRouterPort    - Dynamic Router Port
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *
 */
rtksw_api_ret_t dal_rtl8367c_igmp_dynamicRouterPort_get(rtksw_igmp_dynamicRouterPort_t *pDynamicRouterPort)
{
    rtksw_api_ret_t   retVal;
    rtksw_uint32 port;
    rtksw_uint32 timer;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(NULL == pDynamicRouterPort)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicIGMPdynamicRouterPort1(&port, &timer))!= RT_ERR_OK)
        return retVal;

    if (port == RTL8367C_ROUTER_PORT_INVALID)
    {
        pDynamicRouterPort->dynamicRouterPort0Valid = RTKSW_DISABLED;
        pDynamicRouterPort->dynamicRouterPort0      = 0;
        pDynamicRouterPort->dynamicRouterPort0Timer = 0;
    }
    else
    {
        pDynamicRouterPort->dynamicRouterPort0Valid = RTKSW_ENABLED;
        pDynamicRouterPort->dynamicRouterPort0      = rtksw_switch_port_P2L_get(port);
        pDynamicRouterPort->dynamicRouterPort0Timer = timer;
    }

    if ((retVal = rtl8367c_getAsicIGMPdynamicRouterPort2(&port, &timer))!= RT_ERR_OK)
        return retVal;

    if (port == RTL8367C_ROUTER_PORT_INVALID)
    {
        pDynamicRouterPort->dynamicRouterPort1Valid = RTKSW_DISABLED;
        pDynamicRouterPort->dynamicRouterPort1      = 0;
        pDynamicRouterPort->dynamicRouterPort1Timer = 0;
    }
    else
    {
        pDynamicRouterPort->dynamicRouterPort1Valid = RTKSW_ENABLED;
        pDynamicRouterPort->dynamicRouterPort1      = rtksw_switch_port_P2L_get(port);
        pDynamicRouterPort->dynamicRouterPort1Timer = timer;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_suppressionEnable_set
 * Description:
 *      Configure IGMPv1/v2 & MLDv1 Report/Leave/Done suppression
 * Input:
 *      reportSuppression   - Report suppression
 *      leaveSuppression    - Leave suppression
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 * Note:
 *
 */
rtksw_api_ret_t dal_rtl8367c_igmp_suppressionEnable_set(rtksw_enable_t reportSuppression, rtksw_enable_t leaveSuppression)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(reportSuppression >= RTKSW_ENABLE_END)
        return RT_ERR_INPUT;

    if(leaveSuppression >= RTKSW_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicIGMPSuppression((rtksw_uint32)reportSuppression, (rtksw_uint32)leaveSuppression))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_suppressionEnable_get
 * Description:
 *      Get IGMPv1/v2 & MLDv1 Report/Leave/Done suppression
 * Input:
 *      None
 * Output:
 *      pReportSuppression  - Report suppression
 *      pLeaveSuppression   - Leave suppression
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_NULL_POINTER    - Null pointer
 * Note:
 *
 */
rtksw_api_ret_t dal_rtl8367c_igmp_suppressionEnable_get(rtksw_enable_t *pReportSuppression, rtksw_enable_t *pLeaveSuppression)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(NULL == pReportSuppression)
        return RT_ERR_NULL_POINTER;

    if(NULL == pLeaveSuppression)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicIGMPSuppression((rtksw_uint32 *)pReportSuppression, (rtksw_uint32 *)pLeaveSuppression))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_portRxPktEnable_set
 * Description:
 *      Configure IGMP/MLD RX Packet configuration
 * Input:
 *      port       - Port ID
 *      pRxCfg     - RX Packet Configuration
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_NULL_POINTER    - Null pointer
 * Note:
 *
 */
rtksw_api_ret_t dal_rtl8367c_igmp_portRxPktEnable_set(rtksw_port_t port, rtksw_igmp_rxPktEnable_t *pRxCfg)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    /* Check port valid */
    RTKSW_CHK_PORT_VALID(port);

    if(NULL == pRxCfg)
        return RT_ERR_NULL_POINTER;

    if(pRxCfg->rxQuery >= RTKSW_ENABLE_END)
        return RT_ERR_INPUT;

    if(pRxCfg->rxReport >= RTKSW_ENABLE_END)
        return RT_ERR_INPUT;

    if(pRxCfg->rxLeave >= RTKSW_ENABLE_END)
        return RT_ERR_INPUT;

    if(pRxCfg->rxMRP >= RTKSW_ENABLE_END)
        return RT_ERR_INPUT;

    if(pRxCfg->rxMcast >= RTKSW_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicIGMPQueryRX(rtksw_switch_port_L2P_get(port), (rtksw_uint32)pRxCfg->rxQuery))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicIGMPReportRX(rtksw_switch_port_L2P_get(port), (rtksw_uint32)pRxCfg->rxReport))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicIGMPLeaveRX(rtksw_switch_port_L2P_get(port), (rtksw_uint32)pRxCfg->rxLeave))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicIGMPMRPRX(rtksw_switch_port_L2P_get(port), (rtksw_uint32)pRxCfg->rxMRP))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicIGMPMcDataRX(rtksw_switch_port_L2P_get(port), (rtksw_uint32)pRxCfg->rxMcast))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_portRxPktEnable_get
 * Description:
 *      Get IGMP/MLD RX Packet configuration
 * Input:
 *      port       - Port ID
 *      pRxCfg     - RX Packet Configuration
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_NULL_POINTER    - Null pointer
 * Note:
 *
 */
rtksw_api_ret_t dal_rtl8367c_igmp_portRxPktEnable_get(rtksw_port_t port, rtksw_igmp_rxPktEnable_t *pRxCfg)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    /* Check port valid */
    RTKSW_CHK_PORT_VALID(port);

    if(NULL == pRxCfg)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicIGMPQueryRX(rtksw_switch_port_L2P_get(port), (rtksw_uint32 *)&(pRxCfg->rxQuery)))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_getAsicIGMPReportRX(rtksw_switch_port_L2P_get(port), (rtksw_uint32 *)&(pRxCfg->rxReport)))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_getAsicIGMPLeaveRX(rtksw_switch_port_L2P_get(port), (rtksw_uint32 *)&(pRxCfg->rxLeave)))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_getAsicIGMPMRPRX(rtksw_switch_port_L2P_get(port), (rtksw_uint32 *)&(pRxCfg->rxMRP)))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_getAsicIGMPMcDataRX(rtksw_switch_port_L2P_get(port), (rtksw_uint32 *)&(pRxCfg->rxMcast)))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_groupInfo_get
 * Description:
 *      Get IGMP/MLD Group database
 * Input:
 *      indes       - Index (0~255)
 * Output:
 *      pGroup      - Group database information.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_NULL_POINTER    - Null pointer
 * Note:
 *
 */
rtksw_api_ret_t dal_rtl8367c_igmp_groupInfo_get(rtksw_uint32 index, rtksw_igmp_groupInfo_t *pGroup)
{
    rtksw_api_ret_t   retVal;
    rtksw_uint32      valid;
    rtl8367c_igmpgroup  grp;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    /* Check index */
    if(index > RTL8367C_IGMP_MAX_GOUP)
        return RT_ERR_INPUT;

    if(NULL == pGroup)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicIGMPGroup(index, &valid, &grp))!=RT_ERR_OK)
        return retVal;

    memset(pGroup, 0x00, sizeof(rtksw_igmp_groupInfo_t));
    pGroup->valid = valid;
    pGroup->reportSuppFlag = grp.report_supp_flag;

    if(grp.p0_timer != 0)
    {
        RTKSW_PORTMASK_PORT_SET((pGroup->member), rtksw_switch_port_P2L_get(0));
        pGroup->timer[rtksw_switch_port_P2L_get(0)] = grp.p0_timer;
    }

    if(grp.p1_timer != 0)
    {
        RTKSW_PORTMASK_PORT_SET((pGroup->member), rtksw_switch_port_P2L_get(1));
        pGroup->timer[rtksw_switch_port_P2L_get(1)] = grp.p1_timer;
    }

    if(grp.p2_timer != 0)
    {
        RTKSW_PORTMASK_PORT_SET((pGroup->member), rtksw_switch_port_P2L_get(2));
        pGroup->timer[rtksw_switch_port_P2L_get(2)] = grp.p2_timer;
    }

    if(grp.p3_timer != 0)
    {
        RTKSW_PORTMASK_PORT_SET((pGroup->member), rtksw_switch_port_P2L_get(3));
        pGroup->timer[rtksw_switch_port_P2L_get(3)] = grp.p3_timer;
    }

    if(grp.p4_timer != 0)
    {
        RTKSW_PORTMASK_PORT_SET((pGroup->member), rtksw_switch_port_P2L_get(4));
        pGroup->timer[rtksw_switch_port_P2L_get(4)] = grp.p4_timer;
    }

    if(grp.p5_timer != 0)
    {
        RTKSW_PORTMASK_PORT_SET((pGroup->member), rtksw_switch_port_P2L_get(5));
        pGroup->timer[rtksw_switch_port_P2L_get(5)] = grp.p5_timer;
    }

    if(grp.p6_timer != 0)
    {
        RTKSW_PORTMASK_PORT_SET((pGroup->member), rtksw_switch_port_P2L_get(6));
        pGroup->timer[rtksw_switch_port_P2L_get(6)] = grp.p6_timer;
    }

    if(grp.p7_timer != 0)
    {
        RTKSW_PORTMASK_PORT_SET((pGroup->member), rtksw_switch_port_P2L_get(7));
        pGroup->timer[rtksw_switch_port_P2L_get(7)] = grp.p7_timer;
    }

    if(grp.p8_timer != 0)
    {
        RTKSW_PORTMASK_PORT_SET((pGroup->member), rtksw_switch_port_P2L_get(8));
        pGroup->timer[rtksw_switch_port_P2L_get(8)] = grp.p8_timer;
    }

    if(grp.p9_timer != 0)
    {
        RTKSW_PORTMASK_PORT_SET((pGroup->member), rtksw_switch_port_P2L_get(9));
        pGroup->timer[rtksw_switch_port_P2L_get(9)] = grp.p9_timer;
    }

    if(grp.p10_timer != 0)
    {
        RTKSW_PORTMASK_PORT_SET((pGroup->member), rtksw_switch_port_P2L_get(10));
        pGroup->timer[rtksw_switch_port_P2L_get(10)] = grp.p10_timer;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_ReportLeaveFwdAction_set
 * Description:
 *      Set Report Leave packet forwarding action
 * Input:
 *      action      - Action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 * Note:
 *
 */
rtksw_api_ret_t dal_rtl8367c_igmp_ReportLeaveFwdAction_set(rtksw_igmp_ReportLeaveFwdAct_t action)
{
    rtksw_api_ret_t   retVal;
    rtksw_uint32      regData;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    switch(action)
    {
        case IGMP_REPORT_LEAVE_TO_ROUTER:
            regData = 1;
            break;
        case IGMP_REPORT_LEAVE_TO_ALLPORT:
            regData = 2;
            break;
        case IGMP_REPORT_LEAVE_TO_ROUTER_PORT_ADV:
            regData = 3;
            break;
        default:
            return RT_ERR_INPUT;
    }

    if ((retVal = rtl8367c_setAsicIGMPReportLeaveFlood(regData))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_ReportLeaveFwdAction_get
 * Description:
 *      Get Report Leave packet forwarding action
 * Input:
 *      action      - Action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_NULL_POINTER    - Null Pointer
 * Note:
 *
 */
rtksw_api_ret_t dal_rtl8367c_igmp_ReportLeaveFwdAction_get(rtksw_igmp_ReportLeaveFwdAct_t *pAction)
{
    rtksw_api_ret_t   retVal;
    rtksw_uint32      regData;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(NULL == pAction)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicIGMPReportLeaveFlood(&regData))!=RT_ERR_OK)
        return retVal;

    switch(regData)
    {
        case 1:
            *pAction = IGMP_REPORT_LEAVE_TO_ROUTER;
            break;
        case 2:
            *pAction = IGMP_REPORT_LEAVE_TO_ALLPORT;
            break;
        case 3:
            *pAction = IGMP_REPORT_LEAVE_TO_ROUTER_PORT_ADV;
            break;
        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_dropLeaveZeroEnable_set
 * Description:
 *      Set the function of droppping Leave packet with group IP = 0.0.0.0
 * Input:
 *      enabled      - Action 1: drop, 0:pass
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 * Note:
 *
 */
rtksw_api_ret_t dal_rtl8367c_igmp_dropLeaveZeroEnable_set(rtksw_enable_t enabled)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(enabled >= RTKSW_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicIGMPDropLeaveZero(enabled))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;

}

/* Function Name:
 *      rtksw_igmp_dropLeaveZeroEnable_get
 * Description:
 *      Get the function of droppping Leave packet with group IP = 0.0.0.0
 * Input:
 *      None
 * Output:
 *      pEnabled.   - Action 1: drop, 0:pass
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_NULL_POINTER    - Null Pointer
 * Note:
 *
 */
rtksw_api_ret_t dal_rtl8367c_igmp_dropLeaveZeroEnable_get(rtksw_enable_t *pEnabled)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(NULL == pEnabled)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicIGMPDropLeaveZero((rtksw_uint32 *)pEnabled))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;

}

/* Function Name:
 *      rtksw_igmp_bypassGroupRange_set
 * Description:
 *      Set Bypass group
 * Input:
 *      group       - bypassed group
 *      enabled     - enabled 1: Bypassed, 0: not bypass
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 * Note:
 *
 */
rtksw_api_ret_t dal_rtl8367c_igmp_bypassGroupRange_set(rtksw_igmp_bypassGroup_t group, rtksw_enable_t enabled)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(group >= IGMP_BYPASS_GROUP_END)
        return RT_ERR_INPUT;

    if(enabled >= RTKSW_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicIGMPBypassGroup(group, enabled))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_igmp_bypassGroupRange_get
 * Description:
 *      get Bypass group
 * Input:
 *      group       - bypassed group
 * Output:
 *      pEnable     - enabled 1: Bypassed, 0: not bypass
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_NULL_POINTER    - Null Pointer
 * Note:
 *
 */
rtksw_api_ret_t dal_rtl8367c_igmp_bypassGroupRange_get(rtksw_igmp_bypassGroup_t group, rtksw_enable_t *pEnable)
{
    rtksw_api_ret_t   retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(group >= IGMP_BYPASS_GROUP_END)
        return RT_ERR_INPUT;

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicIGMPBypassGroup(group, pEnable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

