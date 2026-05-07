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
 * Feature : Here is a list of all functions and variables in CPU module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <cpu.h>
#include <string.h>

#include <dal/dal_mgmt.h>

/* Function Name:
 *      rtksw_cpu_enable_set
 * Description:
 *      Set CPU port function enable/disable.
 * Input:
 *      enable - CPU port function enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can set CPU port function enable/disable.
 */
rtksw_api_ret_t rtksw_cpu_enable_set(rtksw_enable_t enable)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpu_enable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->cpu_enable_set(enable);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_cpu_enable_get
 * Description:
 *      Get CPU port and its setting.
 * Input:
 *      None
 * Output:
 *      pEnable - CPU port function enable
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_L2_NO_CPU_PORT   - CPU port is not exist
 * Note:
 *      The API can get CPU port function enable/disable.
 */
rtksw_api_ret_t rtksw_cpu_enable_get(rtksw_enable_t *pEnable)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpu_enable_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->cpu_enable_get(pEnable);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_cpu_tagPort_set
 * Description:
 *      Set CPU port and CPU tag insert mode.
 * Input:
 *      port - Port id.
 *      mode - CPU tag insert for packets egress from CPU port.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can set CPU port and inserting proprietary CPU tag mode (Length/Type 0x8899)
 *      to the frame that transmitting to CPU port.
 *      The inset cpu tag mode is as following:
 *      - CPU_INSERT_TO_ALL
 *      - CPU_INSERT_TO_TRAPPING
 *      - CPU_INSERT_TO_NONE
 */
rtksw_api_ret_t rtksw_cpu_tagPort_set(rtksw_port_t port, rtksw_cpu_insert_t mode)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpu_tagPort_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->cpu_tagPort_set(port, mode);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_cpu_tagPort_get
 * Description:
 *      Get CPU port and CPU tag insert mode.
 * Input:
 *      None
 * Output:
 *      pPort - Port id.
 *      pMode - CPU tag insert for packets egress from CPU port, 0:all insert 1:Only for trapped packets 2:no insert.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_L2_NO_CPU_PORT   - CPU port is not exist
 * Note:
 *      The API can get configured CPU port and its setting.
 *      The inset cpu tag mode is as following:
 *      - CPU_INSERT_TO_ALL
 *      - CPU_INSERT_TO_TRAPPING
 *      - CPU_INSERT_TO_NONE
 */
rtksw_api_ret_t rtksw_cpu_tagPort_get(rtksw_port_t *pPort, rtksw_cpu_insert_t *pMode)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpu_tagPort_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->cpu_tagPort_get(pPort, pMode);
    RTKSW_API_UNLOCK();

    return retVal;
}


/* Function Name:
 *      rtksw_cpu_awarePort_set
 * Description:
 *      Set CPU aware port mask.
 * Input:
 *      portmask - Port mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK      - Invalid port mask.
 * Note:
 *      The API can set configured CPU aware port mask.
 */
rtksw_api_ret_t rtksw_cpu_awarePort_set(rtksw_portmask_t *pPortmask)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpu_awarePort_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->cpu_awarePort_set(pPortmask);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_cpu_awarePort_get
 * Description:
 *      Get CPU aware port mask.
 * Input:
 *      None
 * Output:
 *      pPortmask - Port mask.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 * Note:
 *      The API can get configured CPU aware port mask.
 */
rtksw_api_ret_t rtksw_cpu_awarePort_get(rtksw_portmask_t *pPortmask)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpu_awarePort_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->cpu_awarePort_get(pPortmask);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_cpu_tagPosition_set
 * Description:
 *      Set CPU tag position.
 * Input:
 *      position - CPU tag position.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT      - Invalid input.
 * Note:
 *      The API can set CPU tag position.
 */
rtksw_api_ret_t rtksw_cpu_tagPosition_set(rtksw_cpu_position_t position)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpu_tagPosition_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->cpu_tagPosition_set(position);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_cpu_tagPosition_get
 * Description:
 *      Get CPU tag position.
 * Input:
 *      None
 * Output:
 *      pPosition - CPU tag position.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT      - Invalid input.
 * Note:
 *      The API can get CPU tag position.
 */
rtksw_api_ret_t rtksw_cpu_tagPosition_get(rtksw_cpu_position_t *pPosition)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpu_tagPosition_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->cpu_tagPosition_get(pPosition);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_cpu_tagLength_set
 * Description:
 *      Set CPU tag length.
 * Input:
 *      length - CPU tag length.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT      - Invalid input.
 * Note:
 *      The API can set CPU tag length.
 */
rtksw_api_ret_t rtksw_cpu_tagLength_set(rtksw_cpu_tag_length_t length)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpu_tagLength_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->cpu_tagLength_set(length);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_cpu_tagLength_get
 * Description:
 *      Get CPU tag length.
 * Input:
 *      None
 * Output:
 *      pLength - CPU tag length.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT      - Invalid input.
 * Note:
 *      The API can get CPU tag length.
 */
rtksw_api_ret_t rtksw_cpu_tagLength_get(rtksw_cpu_tag_length_t *pLength)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpu_tagLength_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->cpu_tagLength_get(pLength);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_cpu_priRemap_set
 * Description:
 *      Configure CPU priorities mapping to internal absolute priority.
 * Input:
 *      int_pri     - internal priority value.
 *      new_pri    - new internal priority value.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_VLAN_PRIORITY    - Invalid 1p priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      Priority of CPU tag assignment for internal asic priority, and it is used for queue usage and packet scheduling.
 */
rtksw_api_ret_t rtksw_cpu_priRemap_set(rtksw_pri_t int_pri, rtksw_pri_t new_pri)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpu_priRemap_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->cpu_priRemap_set(int_pri, new_pri);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_cpu_priRemap_get
 * Description:
 *      Configure CPU priorities mapping to internal absolute priority.
 * Input:
 *      int_pri     - internal priority value.
 * Output:
 *      pNew_pri    - new internal priority value.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_VLAN_PRIORITY    - Invalid 1p priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      Priority of CPU tag assignment for internal asic priority, and it is used for queue usage and packet scheduling.
 */
rtksw_api_ret_t rtksw_cpu_priRemap_get(rtksw_pri_t int_pri, rtksw_pri_t *pNew_pri)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpu_priRemap_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->cpu_priRemap_get(int_pri, pNew_pri);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_cpu_acceptLength_set
 * Description:
 *      Set CPU accept  length.
 * Input:
 *      length - CPU tag length.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT      - Invalid input.
 * Note:
 *      The API can set CPU accept length.
 */
rtksw_api_ret_t rtksw_cpu_acceptLength_set(rtksw_cpu_rx_length_t length)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpu_acceptLength_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->cpu_acceptLength_set(length);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_cpu_acceptLength_get
 * Description:
 *      Get CPU accept length.
 * Input:
 *      None
 * Output:
 *      pLength - CPU tag length.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT      - Invalid input.
 * Note:
 *      The API can get CPU accept length.
 */
rtksw_api_ret_t rtksw_cpu_acceptLength_get(rtksw_cpu_rx_length_t *pLength)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpu_acceptLength_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->cpu_acceptLength_get(pLength);
    RTKSW_API_UNLOCK();

    return retVal;
}

