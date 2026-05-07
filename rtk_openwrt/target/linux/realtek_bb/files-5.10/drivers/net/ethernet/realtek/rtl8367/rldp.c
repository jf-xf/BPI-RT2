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
 * $Revision: $
 * $Date: $
 *
 * Purpose : Declaration of RLDP and RLPP API
 *
 * Feature : The file have include the following module and sub-modules
 *           1) RLDP and RLPP configuration and status
 *
 */


/*
 * Include Files
 */
#include <rtk_switch.h>
#include <rtk_error.h>

#include <dal/dal_mgmt.h>


/* Function Name:
 *      rtksw_rldp_config_set
 * Description:
 *      Set RLDP module configuration
 * Input:
 *      pConfig - configuration structure of RLDP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
rtksw_api_ret_t rtksw_rldp_config_set(rtksw_rldp_config_t *pConfig)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->rldp_config_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->rldp_config_set(pConfig);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_rldp_config_get
 * Description:
 *      Get RLDP module configuration
 * Input:
 *      None
 * Output:
 *      pConfig - configuration structure of RLDP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
rtksw_api_ret_t rtksw_rldp_config_get(rtksw_rldp_config_t *pConfig)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->rldp_config_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->rldp_config_get(pConfig);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_rldp_portConfig_set
 * Description:
 *      Set per port RLDP module configuration
 * Input:
 *      port   - port number to be configured
 *      pPortConfig - per port configuration structure of RLDP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
rtksw_api_ret_t rtksw_rldp_portConfig_set(rtksw_port_t port, rtksw_rldp_portConfig_t *pPortConfig)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->rldp_portConfig_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->rldp_portConfig_set(port, pPortConfig);
    RTKSW_API_UNLOCK();

    return retVal;
} /* end of rtksw_rldp_portConfig_set */


/* Function Name:
 *      rtksw_rldp_portConfig_get
 * Description:
 *      Get per port RLDP module configuration
 * Input:
 *      port    - port number to be get
 * Output:
 *      pPortConfig - per port configuration structure of RLDP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
rtksw_api_ret_t rtksw_rldp_portConfig_get(rtksw_port_t port, rtksw_rldp_portConfig_t *pPortConfig)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->rldp_portConfig_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->rldp_portConfig_get(port, pPortConfig);
    RTKSW_API_UNLOCK();

    return retVal;
} /* end of rtksw_rldp_portConfig_get */


/* Function Name:
 *      rtksw_rldp_status_get
 * Description:
 *      Get RLDP module status
 * Input:
 *      None
 * Output:
 *      pStatus - status structure of RLDP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
rtksw_api_ret_t rtksw_rldp_status_get(rtksw_rldp_status_t *pStatus)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->rldp_status_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->rldp_status_get(pStatus);
    RTKSW_API_UNLOCK();

    return retVal;
} /* end of rtksw_rldp_status_get */


/* Function Name:
 *      rtksw_rldp_portStatus_get
 * Description:
 *      Get RLDP module status
 * Input:
 *      port    - port number to be get
 * Output:
 *      pPortStatus - per port status structure of RLDP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
rtksw_api_ret_t rtksw_rldp_portStatus_get(rtksw_port_t port, rtksw_rldp_portStatus_t *pPortStatus)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->rldp_portStatus_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->rldp_portStatus_get(port, pPortStatus);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_rldp_portStatus_set
 * Description:
 *      Clear RLDP module status
 * Input:
 *      port    - port number to be clear
 *      pPortStatus - per port status structure of RLDP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      Clear operation effect loop_enter and loop_leave only, other field in
 *      the structure are don't care. Loop status cab't be clean.
 */
rtksw_api_ret_t rtksw_rldp_portStatus_set(rtksw_port_t port, rtksw_rldp_portStatus_t *pPortStatus)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->rldp_portStatus_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->rldp_portStatus_set(port, pPortStatus);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_rldp_portLoopPair_get
 * Description:
 *      Get RLDP port loop pairs
 * Input:
 *      port    - port number to be get
 * Output:
 *      pPortmask - per port related loop ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
rtksw_api_ret_t rtksw_rldp_portLoopPair_get(rtksw_port_t port, rtksw_portmask_t *pPortmask)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->rldp_portLoopPair_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->rldp_portLoopPair_get(port, pPortmask);
    RTKSW_API_UNLOCK();

    return retVal;
}



