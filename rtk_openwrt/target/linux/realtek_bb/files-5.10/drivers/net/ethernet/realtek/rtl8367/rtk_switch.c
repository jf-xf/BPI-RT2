/*
 * Copyright (C) 2010 Realtek Semiconductor Corp.
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
 * Purpose : RTK switch high-level API
 * Feature : Here is a list of all functions and variables in this module.
 *
 */
#include <rtk_switch.h>
#include <rtk_error.h>
#include <string.h>

#include <dal/dal_mgmt.h>

static rtksw_init_state_t    init_state = RTKSW_INIT_NOT_COMPLETED;

#if defined(RTKSW_X86_CLE)
pthread_mutex_t api_mutex = PTHREAD_MUTEX_INITIALIZER;
#else
DEFINE_SPINLOCK(rtl8367_lock);
#endif

static rtksw_switch_halCtrl_t *halCtrl = NULL;


static rtksw_api_ret_t _rtksw_switch_init(void)
{
    rtksw_int32  retVal;
    switch_chip_t   switchChip;

    /* Find device */
    if((halCtrl = rtksw_hal_find_device()) == NULL)
        return RT_ERR_CHIP_NOT_FOUND;

    /* Attached DAL mapper */
    switchChip = halCtrl->switch_type;
    if((retVal = dal_mgmt_attachDevice(switchChip)) != RT_ERR_OK)
        return retVal;

    /* Set initial state */
    if((retVal = rtksw_switch_initialState_set(RTKSW_INIT_COMPLETED)) != RT_ERR_OK)
        return retVal;

    /* Call initial function */
    if((retVal = RT_MAPPER->switch_init()) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_switch_initialState_set
 * Description:
 *      Set initial status
 * Input:
 *      state   - Initial state;
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Initialized
 *      RT_ERR_FAILED   - Uninitialized
 * Note:
 *
 */
rtksw_api_ret_t rtksw_switch_initialState_set(rtksw_init_state_t state)
{
    if(state >= RTKSW_INIT_STATE_END)
        return RT_ERR_FAILED;

    init_state = state;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_switch_initialState_get
 * Description:
 *      Get initial status
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RTKSW_INIT_COMPLETED     - Initialized
 *      RTKSW_INIT_NOT_COMPLETED - Uninitialized
 * Note:
 *
 */
rtksw_init_state_t rtksw_switch_initialState_get(void)
{
    return init_state;
}

/* Function Name:
 *      rtksw_switch_logicalPortCheck
 * Description:
 *      Check logical port ID.
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is correct
 *      RT_ERR_FAILED   - Port ID is not correct
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
rtksw_api_ret_t rtksw_switch_logicalPortCheck(rtksw_port_t logicalPort)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(logicalPort >= RTKSW_SWITCH_PORT_NUM)
        return RT_ERR_FAILED;

    if(halCtrl->l2p_port[logicalPort] == 0xFF)
        return RT_ERR_FAILED;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_switch_isUtpPort
 * Description:
 *      Check is logical port a UTP port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a UTP port
 *      RT_ERR_FAILED   - Port ID is not a UTP port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
rtksw_api_ret_t rtksw_switch_isUtpPort(rtksw_port_t logicalPort)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(logicalPort >= RTKSW_SWITCH_PORT_NUM)
        return RT_ERR_FAILED;

    if(halCtrl->log_port_type[logicalPort] == UTP_PORT)
        return RT_ERR_OK;
    else
        return RT_ERR_FAILED;
}

/* Function Name:
 *      rtksw_switch_isExtPort
 * Description:
 *      Check is logical port a Extension port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a EXT port
 *      RT_ERR_FAILED   - Port ID is not a EXT port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
rtksw_api_ret_t rtksw_switch_isExtPort(rtksw_port_t logicalPort)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(logicalPort >= RTKSW_SWITCH_PORT_NUM)
        return RT_ERR_FAILED;

    if(halCtrl->log_port_type[logicalPort] == EXT_PORT)
        return RT_ERR_OK;
    else
        return RT_ERR_FAILED;
}


/* Function Name:
 *      rtksw_switch_isHsgPort
 * Description:
 *      Check is logical port a HSG port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a HSG port
 *      RT_ERR_FAILED   - Port ID is not a HSG port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
rtksw_api_ret_t rtksw_switch_isHsgPort(rtksw_port_t logicalPort)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(logicalPort >= RTKSW_SWITCH_PORT_NUM)
        return RT_ERR_FAILED;

    if( ((0x01 << logicalPort) & halCtrl->hsg_logical_portmask) != 0)
        return RT_ERR_OK;
    else
        return RT_ERR_FAILED;
}

/* Function Name:
 *      rtksw_switch_isSgmiiPort
 * Description:
 *      Check is logical port a SGMII port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a SGMII port
 *      RT_ERR_FAILED   - Port ID is not a SGMII port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
rtksw_api_ret_t rtksw_switch_isSgmiiPort(rtksw_port_t logicalPort)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(logicalPort >= RTKSW_SWITCH_PORT_NUM)
        return RT_ERR_FAILED;

    if( ((0x01 << logicalPort) & halCtrl->sg_logical_portmask) != 0)
        return RT_ERR_OK;
    else
        return RT_ERR_FAILED;
}

/* Function Name:
 *      rtksw_switch_isCPUPort
 * Description:
 *      Check is logical port a CPU port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a CPU port
 *      RT_ERR_FAILED   - Port ID is not a CPU port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
rtksw_api_ret_t rtksw_switch_isCPUPort(rtksw_port_t logicalPort)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(logicalPort >= RTKSW_SWITCH_PORT_NUM)
        return RT_ERR_FAILED;

    if( ((0x01 << logicalPort) & halCtrl->valid_cpu_portmask) != 0)
        return RT_ERR_OK;
    else
        return RT_ERR_FAILED;
}

/* Function Name:
 *      rtksw_switch_isComboPort
 * Description:
 *      Check is logical port a Combo port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a combo port
 *      RT_ERR_FAILED   - Port ID is not a combo port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
rtksw_api_ret_t rtksw_switch_isComboPort(rtksw_port_t logicalPort)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(logicalPort >= RTKSW_SWITCH_PORT_NUM)
        return RT_ERR_FAILED;

    if(halCtrl->combo_logical_port == logicalPort)
        return RT_ERR_OK;
    else
        return RT_ERR_FAILED;
}

/* Function Name:
 *      rtksw_switch_ComboPort_get
 * Description:
 *      Get Combo port ID
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      Port ID of combo port
 * Note:
 *
 */
rtksw_uint32 rtksw_switch_ComboPort_get(void)
{
    return halCtrl->combo_logical_port;
}

/* Function Name:
 *      rtksw_switch_isPtpPort
 * Description:
 *      Check is logical port a PTP port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a PTP port
 *      RT_ERR_FAILED   - Port ID is not a PTP port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
rtksw_api_ret_t rtksw_switch_isPtpPort(rtksw_port_t logicalPort)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(logicalPort >= RTKSW_SWITCH_PORT_NUM)
        return RT_ERR_FAILED;

    if(halCtrl->ptp_port[logicalPort] == 1)
        return RT_ERR_OK;
    else
        return RT_ERR_FAILED;
}

/* Function Name:
 *      rtksw_switch_port_L2P_get
 * Description:
 *      Get physical port ID
 * Input:
 *      logicalPort       - logical port ID
 * Output:
 *      None
 * Return:
 *      Physical port ID
 * Note:
 *
 */
rtksw_uint32 rtksw_switch_port_L2P_get(rtksw_port_t logicalPort)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return UNDEFINE_PHY_PORT;

    if(logicalPort >= RTKSW_SWITCH_PORT_NUM)
        return UNDEFINE_PHY_PORT;

    return (halCtrl->l2p_port[logicalPort]);
}

/* Function Name:
 *      rtksw_switch_port_P2L_get
 * Description:
 *      Get logical port ID
 * Input:
 *      physicalPort       - physical port ID
 * Output:
 *      None
 * Return:
 *      logical port ID
 * Note:
 *
 */
rtksw_port_t rtksw_switch_port_P2L_get(rtksw_uint32 physicalPort)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return UNDEFINE_PORT;

    if(physicalPort >= RTKSW_SWITCH_PORT_NUM)
        return UNDEFINE_PORT;

    return (halCtrl->p2l_port[physicalPort]);
}

/* Function Name:
 *      rtksw_switch_isPortMaskValid
 * Description:
 *      Check portmask is valid or not
 * Input:
 *      pPmask       - logical port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - port mask is valid
 *      RT_ERR_FAILED       - port mask is not valid
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 * Note:
 *
 */
rtksw_api_ret_t rtksw_switch_isPortMaskValid(rtksw_portmask_t *pPmask)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(NULL == pPmask)
        return RT_ERR_NULL_POINTER;

    if( (pPmask->bits[0] | halCtrl->valid_portmask) != halCtrl->valid_portmask )
        return RT_ERR_FAILED;
    else
        return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_switch_isPortMaskUtp
 * Description:
 *      Check all ports in portmask are only UTP port
 * Input:
 *      pPmask       - logical port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Only UTP port in port mask
 *      RT_ERR_FAILED       - Not only UTP port in port mask
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 * Note:
 *
 */
rtksw_api_ret_t rtksw_switch_isPortMaskUtp(rtksw_portmask_t *pPmask)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(NULL == pPmask)
        return RT_ERR_NULL_POINTER;

    if( (pPmask->bits[0] | halCtrl->valid_utp_portmask) != halCtrl->valid_utp_portmask )
        return RT_ERR_FAILED;
    else
        return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_switch_isPortMaskExt
 * Description:
 *      Check all ports in portmask are only EXT port
 * Input:
 *      pPmask       - logical port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Only EXT port in port mask
 *      RT_ERR_FAILED       - Not only EXT port in port mask
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 * Note:
 *
 */
rtksw_api_ret_t rtksw_switch_isPortMaskExt(rtksw_portmask_t *pPmask)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(NULL == pPmask)
        return RT_ERR_NULL_POINTER;

    if( (pPmask->bits[0] | halCtrl->valid_ext_portmask) != halCtrl->valid_ext_portmask )
        return RT_ERR_FAILED;
    else
        return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_switch_portmask_L2P_get
 * Description:
 *      Get physicl portmask from logical portmask
 * Input:
 *      pLogicalPmask       - logical port mask
 * Output:
 *      pPhysicalPortmask   - physical port mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 *      RT_ERR_PORT_MASK    - Error port mask
 * Note:
 *
 */
rtksw_api_ret_t rtksw_switch_portmask_L2P_get(rtksw_portmask_t *pLogicalPmask, rtksw_uint32 *pPhysicalPortmask)
{
    rtksw_uint32 log_port, phy_port;

    if(init_state != RTKSW_INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(NULL == pLogicalPmask)
        return RT_ERR_NULL_POINTER;

    if(NULL == pPhysicalPortmask)
        return RT_ERR_NULL_POINTER;

    if(rtksw_switch_isPortMaskValid(pLogicalPmask) != RT_ERR_OK)
        return RT_ERR_PORT_MASK;

    /* reset physical port mask */
    *pPhysicalPortmask = 0;

    RTKSW_PORTMASK_SCAN((*pLogicalPmask), log_port)
    {
        phy_port = rtksw_switch_port_L2P_get((rtksw_port_t)log_port);
        *pPhysicalPortmask |= (0x0001 << phy_port);
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_switch_portmask_P2L_get
 * Description:
 *      Get logical portmask from physical portmask
 * Input:
 *      physicalPortmask    - physical port mask
 * Output:
 *      pLogicalPmask       - logical port mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 *      RT_ERR_PORT_MASK    - Error port mask
 * Note:
 *
 */
rtksw_api_ret_t rtksw_switch_portmask_P2L_get(rtksw_uint32 physicalPortmask, rtksw_portmask_t *pLogicalPmask)
{
    rtksw_uint32 log_port, phy_port;

    if(init_state != RTKSW_INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(NULL == pLogicalPmask)
        return RT_ERR_NULL_POINTER;

    RTKSW_PORTMASK_CLEAR(*pLogicalPmask);

    for(phy_port = halCtrl->min_phy_port; phy_port <= halCtrl->max_phy_port; phy_port++)
    {
        if(physicalPortmask & (0x0001 << phy_port))
        {
            log_port = rtksw_switch_port_P2L_get(phy_port);
            if(log_port != UNDEFINE_PORT)
            {
                RTKSW_PORTMASK_PORT_SET(*pLogicalPmask, log_port);
            }
        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_switch_phyPortMask_get
 * Description:
 *      Get physical portmask
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      0x00                - Not Initialize
 *      Other value         - Physical port mask
 * Note:
 *
 */
rtksw_uint32 rtksw_switch_phyPortMask_get(void)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return 0x00; /* No port in portmask */

    return (halCtrl->phy_portmask);
}

/* Function Name:
 *      rtksw_switch_logPortMask_get
 * Description:
 *      Get Logical portmask
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 * Note:
 *
 */
rtksw_api_ret_t rtksw_switch_logPortMask_get(rtksw_portmask_t *pPortmask)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return RT_ERR_FAILED;

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    pPortmask->bits[0] = halCtrl->valid_portmask;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtksw_switch_init
 * Description:
 *      Set chip to default configuration enviroment
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      The API can set chip registers to default configuration for different release chip model.
 */
rtksw_api_ret_t rtksw_switch_init(void)
{
    rtksw_api_ret_t retVal;

    RTKSW_API_LOCK();
    retVal = _rtksw_switch_init();
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_switch_portMaxPktLen_set
 * Description:
 *      Set Max packet length
 * Input:
 *      port    - Port ID
 *      speed   - Speed
 *      cfgId   - Configuration ID
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 */
rtksw_api_ret_t rtksw_switch_portMaxPktLen_set(rtksw_port_t port, rtksw_switch_maxPktLen_linkSpeed_t speed, rtksw_uint32 cfgId)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->switch_portMaxPktLen_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->switch_portMaxPktLen_set(port, speed, cfgId);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_switch_portMaxPktLen_get
 * Description:
 *      Get Max packet length
 * Input:
 *      port    - Port ID
 *      speed   - Speed
 * Output:
 *      pCfgId  - Configuration ID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 */
rtksw_api_ret_t rtksw_switch_portMaxPktLen_get(rtksw_port_t port, rtksw_switch_maxPktLen_linkSpeed_t speed, rtksw_uint32 *pCfgId)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->switch_portMaxPktLen_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->switch_portMaxPktLen_get(port, speed, pCfgId);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_switch_maxPktLenCfg_set
 * Description:
 *      Set Max packet length configuration
 * Input:
 *      cfgId   - Configuration ID
 *      pktLen  - Max packet length
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 */
rtksw_api_ret_t rtksw_switch_maxPktLenCfg_set(rtksw_uint32 cfgId, rtksw_uint32 pktLen)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->switch_maxPktLenCfg_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->switch_maxPktLenCfg_set(cfgId, pktLen);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_switch_maxPktLenCfg_get
 * Description:
 *      Get Max packet length configuration
 * Input:
 *      cfgId   - Configuration ID
 *      pPktLen - Max packet length
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 */
rtksw_api_ret_t rtksw_switch_maxPktLenCfg_get(rtksw_uint32 cfgId, rtksw_uint32 *pPktLen)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->switch_maxPktLenCfg_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->switch_maxPktLenCfg_get(cfgId, pPktLen);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_switch_greenEthernet_set
 * Description:
 *      Set all Ports Green Ethernet state.
 * Input:
 *      enable - Green Ethernet state.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - OK
 *      RT_ERR_FAILED   - Failed
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_ENABLE   - Invalid enable input.
 * Note:
 *      This API can set all Ports Green Ethernet state.
 *      The configuration is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtksw_api_ret_t rtksw_switch_greenEthernet_set(rtksw_enable_t enable)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->switch_greenEthernet_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->switch_greenEthernet_set(enable);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_switch_greenEthernet_get
 * Description:
 *      Get all Ports Green Ethernet state.
 * Input:
 *      None
 * Output:
 *      pEnable - Green Ethernet state.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API can get Green Ethernet state.
 */
rtksw_api_ret_t rtksw_switch_greenEthernet_get(rtksw_enable_t *pEnable)
{
    rtksw_api_ret_t retVal;

    if (NULL == RT_MAPPER->switch_greenEthernet_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTKSW_API_LOCK();
    retVal = RT_MAPPER->switch_greenEthernet_get(pEnable);
    RTKSW_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtksw_switch_maxLogicalPort_get
 * Description:
 *      Get Max logical port ID
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      Max logical port
 * Note:
 *      This API can get max logical port
 */
rtksw_port_t rtksw_switch_maxLogicalPort_get(void)
{
    rtksw_port_t port, maxLogicalPort = 0;

    /* Check initialization state */
    if(rtksw_switch_initialState_get() != RTKSW_INIT_COMPLETED)
    {
        return UNDEFINE_PORT;
    }

    for(port = 0; port < RTKSW_SWITCH_PORT_NUM; port++)
    {
        if( (halCtrl->log_port_type[port] == UTP_PORT) || (halCtrl->log_port_type[port] == EXT_PORT) )
            maxLogicalPort = port;
    }

    return maxLogicalPort;
}

/* Function Name:
 *      rtksw_switch_maxMeterId_get
 * Description:
 *      Get Max Meter ID
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      0x00                - Not Initialize
 *      Other value         - Max Meter ID
 * Note:
 *
 */
rtksw_uint32 rtksw_switch_maxMeterId_get(void)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return 0x00;

    return (halCtrl->max_meter_id);
}

/* Function Name:
 *      rtksw_switch_maxLutAddrNumber_get
 * Description:
 *      Get Max LUT Address number
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      0x00                - Not Initialize
 *      Other value         - Max LUT Address number
 * Note:
 *
 */
rtksw_uint32 rtksw_switch_maxLutAddrNumber_get(void)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return 0x00;

    return (halCtrl->max_lut_addr_num);
}

/* Function Name:
 *      rtksw_switch_isValidTrunkGrpId
 * Description:
 *      Check if trunk group is valid or not
 * Input:
 *      grpId       - Group ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Trunk Group ID is valid
 *      RT_ERR_LA_TRUNK_ID  - Trunk Group ID is not valid
 * Note:
 *
 */
rtksw_uint32 rtksw_switch_isValidTrunkGrpId(rtksw_uint32 grpId)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return 0x00;

    if( (halCtrl->trunk_group_mask & (0x01 << grpId) ) != 0)
        return RT_ERR_OK;
    else
        return RT_ERR_LA_TRUNK_ID;

}

/* Function Name:
 *      rtksw_switch_maxBufferPageNum_get
 * Description:
 *      Get number of packet buffer page
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      Number of packet buffer page
 * Note:
 *
 */
rtksw_uint32 rtksw_switch_maxBufferPageNum_get(void)
{
    if(init_state != RTKSW_INIT_COMPLETED)
        return 0x00;

    return (halCtrl->packet_buffer_page_num);
}

/* Function Name:
 *      rtksw_switch_chipType_get
 * Description:
 *      Get switch chip type
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      CHIP_END    - Unknown chip type
 *      other       - Switch chip type
 * Note:
 *
 */
switch_chip_t rtksw_switch_chipType_get(void)
{
    if (halCtrl == NULL)
        return CHIP_END;

    return halCtrl->switch_type;
}
