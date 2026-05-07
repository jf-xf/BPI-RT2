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
 * Feature : Here is a list of all functions and variables in EEE module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <eee.h>
#include <string.h>
#include <dal/dal_mgmt.h>

/* Function Name:
 *      rtksw_eee_init
 * Description:
 *      EEE function initialization.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API is used to initialize EEE status.
 */
rtksw_api_ret_t rtksw_eee_init(void)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->eee_init)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->eee_init();
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_eee_portEnable_set
 * Description:
 *      Set enable status of EEE function.
 * Input:
 *      port - port id.
 *      enable - enable EEE status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can set EEE function to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtksw_api_ret_t rtksw_eee_portEnable_set(rtksw_port_t port, rtksw_enable_t enable)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->eee_portEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->eee_portEnable_set(port, enable);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_eee_portEnable_get
 * Description:
 *      Get enable status of EEE function
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Back pressure status.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      This API can get EEE function to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */

rtksw_api_ret_t rtksw_eee_portEnable_get(rtksw_port_t port, rtksw_enable_t *pEnable)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->eee_portEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->eee_portEnable_get(port, pEnable);
    RTKSW_API_UNLOCK();

    return retVal;
}


