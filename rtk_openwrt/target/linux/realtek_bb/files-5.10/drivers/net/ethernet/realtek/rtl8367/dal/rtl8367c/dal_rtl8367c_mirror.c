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
 * Feature : Here is a list of all functions and variables in Mirror module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal/rtl8367c/dal_rtl8367c_mirror.h>
#include <string.h>
#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_mirror.h>

/* Function Name:
 *      dal_rtl8367c_mirror_portBased_set
 * Description:
 *      Set port mirror function.
 * Input:
 *      mirroring_port          - Monitor port.
 *      pMirrored_rx_portmask   - Rx mirror port mask.
 *      pMirrored_tx_portmask   - Tx mirror port mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_PORT_MASK    - Invalid portmask.
 * Note:
 *      The API is to set mirror function of source port and mirror port.
 *      The mirror port can only be set to one port and the TX and RX mirror ports
 *      should be identical.
 */
rtksw_api_ret_t dal_rtl8367c_mirror_portBased_set(rtksw_port_t mirroring_port, rtksw_portmask_t *pMirrored_rx_portmask, rtksw_portmask_t *pMirrored_tx_portmask)
{
    rtksw_api_ret_t retVal;
    rtksw_enable_t mirRx, mirTx;
    rtksw_uint32 i, pmask;
    rtksw_port_t source_port;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    /* Check port valid */
    RTKSW_CHK_PORT_VALID(mirroring_port);

    if(NULL == pMirrored_rx_portmask)
        return RT_ERR_NULL_POINTER;

    if(NULL == pMirrored_tx_portmask)
        return RT_ERR_NULL_POINTER;

    RTKSW_CHK_PORTMASK_VALID(pMirrored_rx_portmask);

    RTKSW_CHK_PORTMASK_VALID(pMirrored_tx_portmask);

    /*Mirror Sorce Port Mask Check*/
    if (pMirrored_tx_portmask->bits[0]!=pMirrored_rx_portmask->bits[0]&&pMirrored_tx_portmask->bits[0]!=0&&pMirrored_rx_portmask->bits[0]!=0)
        return RT_ERR_PORT_MASK;

     /*mirror port != source port*/
    if(RTKSW_PORTMASK_IS_PORT_SET((*pMirrored_tx_portmask), mirroring_port) || RTKSW_PORTMASK_IS_PORT_SET((*pMirrored_rx_portmask), mirroring_port))
        return RT_ERR_PORT_MASK;

    source_port = rtksw_switch_maxLogicalPort_get();

    RTKSW_SCAN_ALL_LOG_PORT(i)
    {
        if (pMirrored_tx_portmask->bits[0]&(1<<i))
        {
            source_port = i;
            break;
        }

        if (pMirrored_rx_portmask->bits[0]&(1<<i))
        {
            source_port = i;
            break;
        }
    }

    if ((retVal = rtl8367c_setAsicPortMirror(rtksw_switch_port_L2P_get(source_port), rtksw_switch_port_L2P_get(mirroring_port))) != RT_ERR_OK)
        return retVal;
    if(pMirrored_rx_portmask->bits[0] != 0)
    {
        if ((retVal = rtksw_switch_portmask_L2P_get(pMirrored_rx_portmask, &pmask)) != RT_ERR_OK)
            return retVal;
        if ((retVal = rtl8367c_setAsicPortMirrorMask(pmask)) != RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtksw_switch_portmask_L2P_get(pMirrored_tx_portmask, &pmask)) != RT_ERR_OK)
            return retVal;
        if ((retVal = rtl8367c_setAsicPortMirrorMask(pmask)) != RT_ERR_OK)
            return retVal;
    }


    if (pMirrored_rx_portmask->bits[0])
        mirRx = RTKSW_ENABLED;
    else
        mirRx = RTKSW_DISABLED;

    if ((retVal = rtl8367c_setAsicPortMirrorRxFunction(mirRx)) != RT_ERR_OK)
        return retVal;

    if (pMirrored_tx_portmask->bits[0])
        mirTx = RTKSW_ENABLED;
    else
        mirTx = RTKSW_DISABLED;

    if ((retVal = rtl8367c_setAsicPortMirrorTxFunction(mirTx)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;

}

/* Function Name:
 *      dal_rtl8367c_mirror_portBased_get
 * Description:
 *      Get port mirror function.
 * Input:
 *      None
 * Output:
 *      pMirroring_port         - Monitor port.
 *      pMirrored_rx_portmask   - Rx mirror port mask.
 *      pMirrored_tx_portmask   - Tx mirror port mask.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API is to get mirror function of source port and mirror port.
 */
rtksw_api_ret_t dal_rtl8367c_mirror_portBased_get(rtksw_port_t *pMirroring_port, rtksw_portmask_t *pMirrored_rx_portmask, rtksw_portmask_t *pMirrored_tx_portmask)
{
    rtksw_api_ret_t retVal;
    rtksw_port_t source_port;
    rtksw_enable_t mirRx, mirTx;
    rtksw_uint32 sport, mport, pmask;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(NULL == pMirrored_rx_portmask)
        return RT_ERR_NULL_POINTER;

    if(NULL == pMirrored_tx_portmask)
        return RT_ERR_NULL_POINTER;

    if(NULL == pMirroring_port)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortMirror(&sport, &mport)) != RT_ERR_OK)
        return retVal;
    source_port = rtksw_switch_port_P2L_get(sport);
    *pMirroring_port = rtksw_switch_port_P2L_get(mport);

    if ((retVal = rtl8367c_getAsicPortMirrorRxFunction((rtksw_uint32*)&mirRx)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_getAsicPortMirrorTxFunction((rtksw_uint32*)&mirTx)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_getAsicPortMirrorMask(&pmask)) != RT_ERR_OK)
        return retVal;

    if (RTKSW_DISABLED == mirRx)
        pMirrored_rx_portmask->bits[0]=0;
    else
    {
        if ((retVal = rtksw_switch_portmask_P2L_get(pmask, pMirrored_rx_portmask)) != RT_ERR_OK)
            return retVal;
        pMirrored_rx_portmask->bits[0] |= 1<<source_port;
    }

     if (RTKSW_DISABLED == mirTx)
        pMirrored_tx_portmask->bits[0]=0;
    else
    {
        if ((retVal = rtksw_switch_portmask_P2L_get(pmask, pMirrored_tx_portmask)) != RT_ERR_OK)
            return retVal;
        pMirrored_tx_portmask->bits[0] |= 1<<source_port;
    }

    return RT_ERR_OK;

}

/* Function Name:
 *      dal_rtl8367c_mirror_portIso_set
 * Description:
 *      Set mirror port isolation.
 * Input:
 *      enable |Mirror isolation status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid enable input
 * Note:
 *      The API is to set mirror isolation function that prevent normal forwarding packets to miror port.
 */
rtksw_api_ret_t dal_rtl8367c_mirror_portIso_set(rtksw_enable_t enable)
{
    rtksw_api_ret_t retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if (enable >= RTKSW_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_setAsicPortMirrorIsolation(enable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367c_mirror_portIso_get
 * Description:
 *      Get mirror port isolation.
 * Input:
 *      None
 * Output:
 *      pEnable |Mirror isolation status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API is to get mirror isolation status.
 */
rtksw_api_ret_t dal_rtl8367c_mirror_portIso_get(rtksw_enable_t *pEnable)
{
    rtksw_api_ret_t retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortMirrorIsolation(pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367c_mirror_vlanLeaky_set
 * Description:
 *      Set mirror VLAN leaky.
 * Input:
 *      txenable -TX leaky enable.
 *      rxenable - RX leaky enable.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid enable input
 * Note:
 *      The API is to set mirror VLAN leaky function forwarding packets to miror port.
 */
rtksw_api_ret_t dal_rtl8367c_mirror_vlanLeaky_set(rtksw_enable_t txenable, rtksw_enable_t rxenable)
{
    rtksw_api_ret_t retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if ((txenable >= RTKSW_ENABLE_END) ||(rxenable >= RTKSW_ENABLE_END))
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_setAsicPortMirrorVlanTxLeaky(txenable)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicPortMirrorVlanRxLeaky(rxenable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367c_mirror_vlanLeaky_get
 * Description:
 *      Get mirror VLAN leaky.
 * Input:
 *      None
 * Output:
 *      pTxenable - TX leaky enable.
 *      pRxenable - RX leaky enable.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API is to get mirror VLAN leaky status.
 */
rtksw_api_ret_t dal_rtl8367c_mirror_vlanLeaky_get(rtksw_enable_t *pTxenable, rtksw_enable_t *pRxenable)
{
    rtksw_api_ret_t retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if( (NULL == pTxenable) || (NULL == pRxenable) )
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortMirrorVlanTxLeaky(pTxenable)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_getAsicPortMirrorVlanRxLeaky(pRxenable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367c_mirror_isolationLeaky_set
 * Description:
 *      Set mirror Isolation leaky.
 * Input:
 *      txenable -TX leaky enable.
 *      rxenable - RX leaky enable.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid enable input
 * Note:
 *      The API is to set mirror VLAN leaky function forwarding packets to miror port.
 */
rtksw_api_ret_t dal_rtl8367c_mirror_isolationLeaky_set(rtksw_enable_t txenable, rtksw_enable_t rxenable)
{
    rtksw_api_ret_t retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if ((txenable >= RTKSW_ENABLE_END) ||(rxenable >= RTKSW_ENABLE_END))
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_setAsicPortMirrorIsolationTxLeaky(txenable)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicPortMirrorIsolationRxLeaky(rxenable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367c_mirror_isolationLeaky_get
 * Description:
 *      Get mirror isolation leaky.
 * Input:
 *      None
 * Output:
 *      pTxenable - TX leaky enable.
 *      pRxenable - RX leaky enable.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API is to get mirror isolation leaky status.
 */
rtksw_api_ret_t dal_rtl8367c_mirror_isolationLeaky_get(rtksw_enable_t *pTxenable, rtksw_enable_t *pRxenable)
{
    rtksw_api_ret_t retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if( (NULL == pTxenable) || (NULL == pRxenable) )
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortMirrorIsolationTxLeaky(pTxenable)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_getAsicPortMirrorIsolationRxLeaky(pRxenable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367c_mirror_keep_set
 * Description:
 *      Set mirror packet format keep.
 * Input:
 *      mode - -mirror keep mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid enable input
 * Note:
 *      The API is to set  -mirror keep mode.
 *      The mirror keep mode is as following:
 *      - MIRROR_FOLLOW_VLAN
 *      - MIRROR_KEEP_ORIGINAL
 *      - MIRROR_KEEP_END
 */
rtksw_api_ret_t dal_rtl8367c_mirror_keep_set(rtksw_mirror_keep_t mode)
{
    rtksw_api_ret_t retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if (mode >= MIRROR_KEEP_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_setAsicPortMirrorRealKeep(mode)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367c_mirror_keep_get
 * Description:
 *      Get mirror packet format keep.
 * Input:
 *      None
 * Output:
 *      pMode -mirror keep mode.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API is to get mirror keep mode.
 *      The mirror keep mode is as following:
 *      - MIRROR_FOLLOW_VLAN
 *      - MIRROR_KEEP_ORIGINAL
 *      - MIRROR_KEEP_END
 */
rtksw_api_ret_t dal_rtl8367c_mirror_keep_get(rtksw_mirror_keep_t *pMode)
{
    rtksw_api_ret_t retVal;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if(NULL == pMode)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortMirrorRealKeep(pMode)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367c_mirror_override_set
 * Description:
 *      Set port mirror override function.
 * Input:
 *      rxMirror        - 1: output mirrored packet, 0: output normal forward packet
 *      txMirror        - 1: output mirrored packet, 0: output normal forward packet
 *      aclMirror       - 1: output mirrored packet, 0: output normal forward packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      The API is to set mirror override function.
 *      This function control the output format when a port output
 *      normal forward & mirrored packet at the same time.
 */
rtksw_api_ret_t dal_rtl8367c_mirror_override_set(rtksw_enable_t rxMirror, rtksw_enable_t txMirror, rtksw_enable_t aclMirror)
{
    rtksw_api_ret_t retVal;

    if( (rxMirror >= RTKSW_ENABLE_END) || (txMirror >= RTKSW_ENABLE_END) || (aclMirror >= RTKSW_ENABLE_END))
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_setAsicPortMirrorOverride((rtksw_uint32)rxMirror, (rtksw_uint32)txMirror, (rtksw_uint32)aclMirror)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367c_mirror_override_get
 * Description:
 *      Get port mirror override function.
 * Input:
 *      None
 * Output:
 *      pRxMirror       - 1: output mirrored packet, 0: output normal forward packet
 *      pTxMirror       - 1: output mirrored packet, 0: output normal forward packet
 *      pAclMirror      - 1: output mirrored packet, 0: output normal forward packet
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - Null Pointer
 * Note:
 *      The API is to Get mirror override function.
 *      This function control the output format when a port output
 *      normal forward & mirrored packet at the same time.
 */
rtksw_api_ret_t dal_rtl8367c_mirror_override_get(rtksw_enable_t *pRxMirror, rtksw_enable_t *pTxMirror, rtksw_enable_t *pAclMirror)
{
    rtksw_api_ret_t retVal;

    if( (pRxMirror == NULL) || (pTxMirror == NULL) || (pAclMirror == NULL))
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_getAsicPortMirrorOverride((rtksw_uint32 *)pRxMirror, (rtksw_uint32 *)pTxMirror, (rtksw_uint32 *)pAclMirror)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


