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
#include <igmp.h>
#include <string.h>

#include <dal/dal_mgmt.h>

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
rtksw_api_ret_t rtksw_igmp_init(void)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_init)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_init();
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_state_set(rtksw_enable_t enabled)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_state_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_state_set(enabled);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_state_get(rtksw_enable_t *pEnabled)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_state_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_state_get(pEnabled);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_static_router_port_set(rtksw_portmask_t *pPortmask)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_static_router_port_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_static_router_port_set(pPortmask);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_static_router_port_get(rtksw_portmask_t *pPortmask)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_static_router_port_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_static_router_port_get(pPortmask);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_protocol_set(rtksw_port_t port, rtksw_igmp_protocol_t protocol, rtksw_igmp_action_t action)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_protocol_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_protocol_set(port, protocol, action);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_protocol_get(rtksw_port_t port, rtksw_igmp_protocol_t protocol, rtksw_igmp_action_t *pAction)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_protocol_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_protocol_get(port, protocol, pAction);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_fastLeave_set(rtksw_enable_t state)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_fastLeave_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_fastLeave_set(state);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_fastLeave_get(rtksw_enable_t *pState)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_fastLeave_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_fastLeave_get(pState);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_maxGroup_set(rtksw_port_t port, rtksw_uint32 group)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_maxGroup_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_maxGroup_set(port, group);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_maxGroup_get(rtksw_port_t port, rtksw_uint32 *pGroup)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_maxGroup_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_maxGroup_get(port, pGroup);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_currentGroup_get(rtksw_port_t port, rtksw_uint32 *pGroup)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_currentGroup_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_currentGroup_get(port, pGroup);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_tableFullAction_set(rtksw_igmp_tableFullAction_t action)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_tableFullAction_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_tableFullAction_set(action);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_tableFullAction_get(rtksw_igmp_tableFullAction_t *pAction)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_tableFullAction_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_tableFullAction_get(pAction);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_checksumErrorAction_set(rtksw_igmp_checksumErrorAction_t action)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_checksumErrorAction_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_checksumErrorAction_set(action);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_checksumErrorAction_get(rtksw_igmp_checksumErrorAction_t *pAction)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_checksumErrorAction_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_checksumErrorAction_get(pAction);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_leaveTimer_set(rtksw_uint32 timer)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_leaveTimer_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_leaveTimer_set(timer);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_leaveTimer_get(rtksw_uint32 *pTimer)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_leaveTimer_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_leaveTimer_get(pTimer);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_queryInterval_set(rtksw_uint32 interval)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_queryInterval_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_queryInterval_set(interval);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_queryInterval_get(rtksw_uint32 *pInterval)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_queryInterval_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_queryInterval_get(pInterval);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_robustness_set(rtksw_uint32 robustness)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_robustness_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_robustness_set(robustness);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_robustness_get(rtksw_uint32 *pRobustness)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_robustness_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_robustness_get(pRobustness);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_dynamicRouterPortAllow_set(rtksw_portmask_t *pPortmask)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_dynamicRouterPortAllow_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_dynamicRouterPortAllow_set(pPortmask);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_dynamicRouterPortAllow_get(rtksw_portmask_t *pPortmask)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_dynamicRouterPortAllow_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_dynamicRouterPortAllow_get(pPortmask);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_dynamicRouterPort_get(rtksw_igmp_dynamicRouterPort_t *pDynamicRouterPort)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_dynamicRouterPort_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_dynamicRouterPort_get(pDynamicRouterPort);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_suppressionEnable_set(rtksw_enable_t reportSuppression, rtksw_enable_t leaveSuppression)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_suppressionEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_suppressionEnable_set(reportSuppression, leaveSuppression);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_suppressionEnable_get(rtksw_enable_t *pReportSuppression, rtksw_enable_t *pLeaveSuppression)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_suppressionEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_suppressionEnable_get(pReportSuppression, pLeaveSuppression);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_portRxPktEnable_set(rtksw_port_t port, rtksw_igmp_rxPktEnable_t *pRxCfg)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_portRxPktEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_portRxPktEnable_set(port, pRxCfg);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_portRxPktEnable_get(rtksw_port_t port, rtksw_igmp_rxPktEnable_t *pRxCfg)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_portRxPktEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_portRxPktEnable_get(port, pRxCfg);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_groupInfo_get(rtksw_uint32 index, rtksw_igmp_groupInfo_t *pGroup)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_groupInfo_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_groupInfo_get(index, pGroup);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_ReportLeaveFwdAction_set(rtksw_igmp_ReportLeaveFwdAct_t action)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_ReportLeaveFwdAction_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_ReportLeaveFwdAction_set(action);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_ReportLeaveFwdAction_get(rtksw_igmp_ReportLeaveFwdAct_t *pAction)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_ReportLeaveFwdAction_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_ReportLeaveFwdAction_get(pAction);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_dropLeaveZeroEnable_set(rtksw_enable_t enabled)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_dropLeaveZeroEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_dropLeaveZeroEnable_set(enabled);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_dropLeaveZeroEnable_get(rtksw_enable_t *pEnabled)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_dropLeaveZeroEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_dropLeaveZeroEnable_get(pEnabled);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_bypassGroupRange_set(rtksw_igmp_bypassGroup_t group, rtksw_enable_t enabled)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_bypassGroupRange_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_bypassGroupRange_set(group, enabled);
    RTKSW_API_UNLOCK();

    return retVal;
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
rtksw_api_ret_t rtksw_igmp_bypassGroupRange_get(rtksw_igmp_bypassGroup_t group, rtksw_enable_t *pEnable)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->igmp_bypassGroupRange_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->igmp_bypassGroupRange_get(group, pEnable);
    RTKSW_API_UNLOCK();

    return retVal;
}

