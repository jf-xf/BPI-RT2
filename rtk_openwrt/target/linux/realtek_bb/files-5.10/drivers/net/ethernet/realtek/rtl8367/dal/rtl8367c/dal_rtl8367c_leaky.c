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
 * Feature : Here is a list of all functions and variables in Leaky module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal/rtl8367c/dal_rtl8367c_leaky.h>
#include <string.h>

#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_portIsolation.h>
#include <rtl8367c_asicdrv_rma.h>
#include <rtl8367c_asicdrv_igmp.h>


/* Function Name:
 *      dal_rtl8367c_leaky_vlan_set
 * Description:
 *      Set VLAN leaky.
 * Input:
 *      type - Packet type for VLAN leaky.
 *      enable - Leaky status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_ENABLE       - Invalid enable input
 * Note:
 *      This API can set VLAN leaky for RMA ,IGMP/MLD, CDP, CSSTP, and LLDP packets.
 *      The leaky frame types are as following:
 *      - RTKSW_LEAKY_BRG_GROUP,
 *      - RTKSW_LEAKY_FD_PAUSE,
 *      - RTKSW_LEAKY_SP_MCAST,
 *      - RTKSW_LEAKY_1X_PAE,
 *      - RTKSW_LEAKY_UNDEF_BRG_04,
 *      - RTKSW_LEAKY_UNDEF_BRG_05,
 *      - RTKSW_LEAKY_UNDEF_BRG_06,
 *      - RTKSW_LEAKY_UNDEF_BRG_07,
 *      - RTKSW_LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - RTKSW_LEAKY_UNDEF_BRG_09,
 *      - RTKSW_LEAKY_UNDEF_BRG_0A,
 *      - RTKSW_LEAKY_UNDEF_BRG_0B,
 *      - RTKSW_LEAKY_UNDEF_BRG_0C,
 *      - RTKSW_LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - RTKSW_LEAKY_8021AB,
 *      - RTKSW_LEAKY_UNDEF_BRG_0F,
 *      - RTKSW_LEAKY_BRG_MNGEMENT,
 *      - RTKSW_LEAKY_UNDEFINED_11,
 *      - RTKSW_LEAKY_UNDEFINED_12,
 *      - RTKSW_LEAKY_UNDEFINED_13,
 *      - RTKSW_LEAKY_UNDEFINED_14,
 *      - RTKSW_LEAKY_UNDEFINED_15,
 *      - RTKSW_LEAKY_UNDEFINED_16,
 *      - RTKSW_LEAKY_UNDEFINED_17,
 *      - RTKSW_LEAKY_UNDEFINED_18,
 *      - RTKSW_LEAKY_UNDEFINED_19,
 *      - RTKSW_LEAKY_UNDEFINED_1A,
 *      - RTKSW_LEAKY_UNDEFINED_1B,
 *      - RTKSW_LEAKY_UNDEFINED_1C,
 *      - RTKSW_LEAKY_UNDEFINED_1D,
 *      - RTKSW_LEAKY_UNDEFINED_1E,
 *      - RTKSW_LEAKY_UNDEFINED_1F,
 *      - RTKSW_LEAKY_GMRP,
 *      - RTKSW_LEAKY_GVRP,
 *      - RTKSW_LEAKY_UNDEF_GARP_22,
 *      - RTKSW_LEAKY_UNDEF_GARP_23,
 *      - RTKSW_LEAKY_UNDEF_GARP_24,
 *      - RTKSW_LEAKY_UNDEF_GARP_25,
 *      - RTKSW_LEAKY_UNDEF_GARP_26,
 *      - RTKSW_LEAKY_UNDEF_GARP_27,
 *      - RTKSW_LEAKY_UNDEF_GARP_28,
 *      - RTKSW_LEAKY_UNDEF_GARP_29,
 *      - RTKSW_LEAKY_UNDEF_GARP_2A,
 *      - RTKSW_LEAKY_UNDEF_GARP_2B,
 *      - RTKSW_LEAKY_UNDEF_GARP_2C,
 *      - RTKSW_LEAKY_UNDEF_GARP_2D,
 *      - RTKSW_LEAKY_UNDEF_GARP_2E,
 *      - RTKSW_LEAKY_UNDEF_GARP_2F,
 *      - RTKSW_LEAKY_IGMP,
 *      - RTKSW_LEAKY_IPMULTICAST.
 *      - RTKSW_LEAKY_CDP,
 *      - RTKSW_LEAKY_CSSTP,
 *      - RTKSW_LEAKY_LLDP.
 */
rtksw_api_ret_t dal_rtl8367c_leaky_vlan_set(rtksw_leaky_type_t type, rtksw_enable_t enable)
{
    rtksw_api_ret_t retVal;
    rtksw_uint32 port;
    rtl8367c_rma_t rmacfg;
    rtksw_uint32 tmp;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if (type >= RTKSW_LEAKY_END)
        return RT_ERR_INPUT;

    if (enable >= RTKSW_ENABLE_END)
        return RT_ERR_INPUT;

    if (type >= 0 && type <= RTKSW_LEAKY_UNDEF_GARP_2F)
    {
        if ((retVal = rtl8367c_getAsicRma(type, &rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.vlan_leaky = enable;

        if ((retVal = rtl8367c_setAsicRma(type, &rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else if (RTKSW_LEAKY_IPMULTICAST == type)
    {
        for (port = 0; port <= RTKSW_PORT_ID_MAX; port++)
        {
            if ((retVal = rtl8367c_setAsicIpMulticastVlanLeaky(port,enable)) != RT_ERR_OK)
                return retVal;
        }
    }
    else if (RTKSW_LEAKY_IGMP == type)
    {
        if ((retVal = rtl8367c_setAsicIGMPVLANLeaky(enable)) != RT_ERR_OK)
            return retVal;
    }
    else if (RTKSW_LEAKY_CDP == type)
    {
        if ((retVal = rtl8367c_getAsicRmaCdp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.vlan_leaky = enable;

        if ((retVal = rtl8367c_setAsicRmaCdp(&rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else if (RTKSW_LEAKY_CSSTP == type)
    {
        if ((retVal = rtl8367c_getAsicRmaCsstp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.vlan_leaky = enable;

        if ((retVal = rtl8367c_setAsicRmaCsstp(&rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else if (RTKSW_LEAKY_LLDP == type)
    {
        if ((retVal = rtl8367c_getAsicRmaLldp(&tmp,&rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.vlan_leaky = enable;

        if ((retVal = rtl8367c_setAsicRmaLldp(tmp, &rmacfg)) != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367c_leaky_vlan_get
 * Description:
 *      Get VLAN leaky.
 * Input:
 *      type - Packet type for VLAN leaky.
 * Output:
 *      pEnable - Leaky status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can get VLAN leaky status for RMA ,IGMP/MLD, CDP, CSSTP, and LLDP  packets.
 *      The leaky frame types are as following:
 *      - RTKSW_LEAKY_BRG_GROUP,
 *      - RTKSW_LEAKY_FD_PAUSE,
 *      - RTKSW_LEAKY_SP_MCAST,
 *      - RTKSW_LEAKY_1X_PAE,
 *      - RTKSW_LEAKY_UNDEF_BRG_04,
 *      - RTKSW_LEAKY_UNDEF_BRG_05,
 *      - RTKSW_LEAKY_UNDEF_BRG_06,
 *      - RTKSW_LEAKY_UNDEF_BRG_07,
 *      - RTKSW_LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - RTKSW_LEAKY_UNDEF_BRG_09,
 *      - RTKSW_LEAKY_UNDEF_BRG_0A,
 *      - RTKSW_LEAKY_UNDEF_BRG_0B,
 *      - RTKSW_LEAKY_UNDEF_BRG_0C,
 *      - RTKSW_LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - RTKSW_LEAKY_8021AB,
 *      - RTKSW_LEAKY_UNDEF_BRG_0F,
 *      - RTKSW_LEAKY_BRG_MNGEMENT,
 *      - RTKSW_LEAKY_UNDEFINED_11,
 *      - RTKSW_LEAKY_UNDEFINED_12,
 *      - RTKSW_LEAKY_UNDEFINED_13,
 *      - RTKSW_LEAKY_UNDEFINED_14,
 *      - RTKSW_LEAKY_UNDEFINED_15,
 *      - RTKSW_LEAKY_UNDEFINED_16,
 *      - RTKSW_LEAKY_UNDEFINED_17,
 *      - RTKSW_LEAKY_UNDEFINED_18,
 *      - RTKSW_LEAKY_UNDEFINED_19,
 *      - RTKSW_LEAKY_UNDEFINED_1A,
 *      - RTKSW_LEAKY_UNDEFINED_1B,
 *      - RTKSW_LEAKY_UNDEFINED_1C,
 *      - RTKSW_LEAKY_UNDEFINED_1D,
 *      - RTKSW_LEAKY_UNDEFINED_1E,
 *      - RTKSW_LEAKY_UNDEFINED_1F,
 *      - RTKSW_LEAKY_GMRP,
 *      - RTKSW_LEAKY_GVRP,
 *      - RTKSW_LEAKY_UNDEF_GARP_22,
 *      - RTKSW_LEAKY_UNDEF_GARP_23,
 *      - RTKSW_LEAKY_UNDEF_GARP_24,
 *      - RTKSW_LEAKY_UNDEF_GARP_25,
 *      - RTKSW_LEAKY_UNDEF_GARP_26,
 *      - RTKSW_LEAKY_UNDEF_GARP_27,
 *      - RTKSW_LEAKY_UNDEF_GARP_28,
 *      - RTKSW_LEAKY_UNDEF_GARP_29,
 *      - RTKSW_LEAKY_UNDEF_GARP_2A,
 *      - RTKSW_LEAKY_UNDEF_GARP_2B,
 *      - RTKSW_LEAKY_UNDEF_GARP_2C,
 *      - RTKSW_LEAKY_UNDEF_GARP_2D,
 *      - RTKSW_LEAKY_UNDEF_GARP_2E,
 *      - RTKSW_LEAKY_UNDEF_GARP_2F,
 *      - RTKSW_LEAKY_IGMP,
 *      - RTKSW_LEAKY_IPMULTICAST.
 *      - RTKSW_LEAKY_CDP,
 *      - RTKSW_LEAKY_CSSTP,
 *      - RTKSW_LEAKY_LLDP.
 */
rtksw_api_ret_t dal_rtl8367c_leaky_vlan_get(rtksw_leaky_type_t type, rtksw_enable_t *pEnable)
{
    rtksw_api_ret_t retVal;
    rtksw_uint32 port,tmp;
    rtl8367c_rma_t rmacfg;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if (type >= RTKSW_LEAKY_END)
        return RT_ERR_INPUT;

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if (type >= 0 && type <= RTKSW_LEAKY_UNDEF_GARP_2F)
    {
        if ((retVal = rtl8367c_getAsicRma(type, &rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.vlan_leaky;

    }
    else if (RTKSW_LEAKY_IPMULTICAST == type)
    {
        for (port = 0; port <= RTKSW_PORT_ID_MAX; port++)
        {
            if ((retVal = rtl8367c_getAsicIpMulticastVlanLeaky(port, &tmp)) != RT_ERR_OK)
                return retVal;
            if (port>0&&(tmp!=*pEnable))
                return RT_ERR_FAILED;
            *pEnable = tmp;
        }
    }
    else if (RTKSW_LEAKY_IGMP == type)
    {
        if ((retVal = rtl8367c_getAsicIGMPVLANLeaky(&tmp)) != RT_ERR_OK)
            return retVal;

        *pEnable = tmp;
    }
    else if (RTKSW_LEAKY_CDP == type)
    {
        if ((retVal = rtl8367c_getAsicRmaCdp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.vlan_leaky;
    }
    else if (RTKSW_LEAKY_CSSTP == type)
    {
        if ((retVal = rtl8367c_getAsicRmaCsstp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.vlan_leaky;
    }
    else if (RTKSW_LEAKY_LLDP == type)
    {
        if ((retVal = rtl8367c_getAsicRmaLldp(&tmp, &rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.vlan_leaky;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367c_leaky_portIsolation_set
 * Description:
 *      Set port isolation leaky.
 * Input:
 *      type - Packet type for port isolation leaky.
 *      enable - Leaky status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_ENABLE       - Invalid enable input
 * Note:
 *      This API can set port isolation leaky for RMA ,IGMP/MLD, CDP, CSSTP, and LLDP  packets.
 *      The leaky frame types are as following:
 *      - RTKSW_LEAKY_BRG_GROUP,
 *      - RTKSW_LEAKY_FD_PAUSE,
 *      - RTKSW_LEAKY_SP_MCAST,
 *      - RTKSW_LEAKY_1X_PAE,
 *      - RTKSW_LEAKY_UNDEF_BRG_04,
 *      - RTKSW_LEAKY_UNDEF_BRG_05,
 *      - RTKSW_LEAKY_UNDEF_BRG_06,
 *      - RTKSW_LEAKY_UNDEF_BRG_07,
 *      - RTKSW_LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - RTKSW_LEAKY_UNDEF_BRG_09,
 *      - RTKSW_LEAKY_UNDEF_BRG_0A,
 *      - RTKSW_LEAKY_UNDEF_BRG_0B,
 *      - RTKSW_LEAKY_UNDEF_BRG_0C,
 *      - RTKSW_LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - RTKSW_LEAKY_8021AB,
 *      - RTKSW_LEAKY_UNDEF_BRG_0F,
 *      - RTKSW_LEAKY_BRG_MNGEMENT,
 *      - RTKSW_LEAKY_UNDEFINED_11,
 *      - RTKSW_LEAKY_UNDEFINED_12,
 *      - RTKSW_LEAKY_UNDEFINED_13,
 *      - RTKSW_LEAKY_UNDEFINED_14,
 *      - RTKSW_LEAKY_UNDEFINED_15,
 *      - RTKSW_LEAKY_UNDEFINED_16,
 *      - RTKSW_LEAKY_UNDEFINED_17,
 *      - RTKSW_LEAKY_UNDEFINED_18,
 *      - RTKSW_LEAKY_UNDEFINED_19,
 *      - RTKSW_LEAKY_UNDEFINED_1A,
 *      - RTKSW_LEAKY_UNDEFINED_1B,
 *      - RTKSW_LEAKY_UNDEFINED_1C,
 *      - RTKSW_LEAKY_UNDEFINED_1D,
 *      - RTKSW_LEAKY_UNDEFINED_1E,
 *      - RTKSW_LEAKY_UNDEFINED_1F,
 *      - RTKSW_LEAKY_GMRP,
 *      - RTKSW_LEAKY_GVRP,
 *      - RTKSW_LEAKY_UNDEF_GARP_22,
 *      - RTKSW_LEAKY_UNDEF_GARP_23,
 *      - RTKSW_LEAKY_UNDEF_GARP_24,
 *      - RTKSW_LEAKY_UNDEF_GARP_25,
 *      - RTKSW_LEAKY_UNDEF_GARP_26,
 *      - RTKSW_LEAKY_UNDEF_GARP_27,
 *      - RTKSW_LEAKY_UNDEF_GARP_28,
 *      - RTKSW_LEAKY_UNDEF_GARP_29,
 *      - RTKSW_LEAKY_UNDEF_GARP_2A,
 *      - RTKSW_LEAKY_UNDEF_GARP_2B,
 *      - RTKSW_LEAKY_UNDEF_GARP_2C,
 *      - RTKSW_LEAKY_UNDEF_GARP_2D,
 *      - RTKSW_LEAKY_UNDEF_GARP_2E,
 *      - RTKSW_LEAKY_UNDEF_GARP_2F,
 *      - RTKSW_LEAKY_IGMP,
 *      - RTKSW_LEAKY_IPMULTICAST.
 *      - RTKSW_LEAKY_CDP,
 *      - RTKSW_LEAKY_CSSTP,
 *      - RTKSW_LEAKY_LLDP.
 */
rtksw_api_ret_t dal_rtl8367c_leaky_portIsolation_set(rtksw_leaky_type_t type, rtksw_enable_t enable)
{
    rtksw_api_ret_t retVal;
    rtksw_uint32 port;
    rtl8367c_rma_t rmacfg;
    rtksw_uint32 tmp;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if (type >= RTKSW_LEAKY_END)
        return RT_ERR_INPUT;

    if (enable >= RTKSW_ENABLE_END)
        return RT_ERR_INPUT;

    if (type >= 0 && type <= RTKSW_LEAKY_UNDEF_GARP_2F)
    {
        if ((retVal = rtl8367c_getAsicRma(type, &rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.portiso_leaky = enable;

        if ((retVal = rtl8367c_setAsicRma(type, &rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else if (RTKSW_LEAKY_IPMULTICAST == type)
    {
        for (port = 0; port < RTKSW_MAX_NUM_OF_PORT; port++)
        {
            if ((retVal = rtl8367c_setAsicIpMulticastPortIsoLeaky(port,enable)) != RT_ERR_OK)
                return retVal;
        }
    }
    else if (RTKSW_LEAKY_IGMP == type)
    {
        if ((retVal = rtl8367c_setAsicIGMPIsoLeaky(enable)) != RT_ERR_OK)
            return retVal;
    }
    else if (RTKSW_LEAKY_CDP == type)
    {
        if ((retVal = rtl8367c_getAsicRmaCdp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.portiso_leaky = enable;

        if ((retVal = rtl8367c_setAsicRmaCdp(&rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else if (RTKSW_LEAKY_CSSTP == type)
    {
        if ((retVal = rtl8367c_getAsicRmaCsstp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.portiso_leaky = enable;

        if ((retVal = rtl8367c_setAsicRmaCsstp(&rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else if (RTKSW_LEAKY_LLDP == type)
    {
        if ((retVal = rtl8367c_getAsicRmaLldp(&tmp, &rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.portiso_leaky = enable;

        if ((retVal = rtl8367c_setAsicRmaLldp(tmp, &rmacfg)) != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367c_leaky_portIsolation_get
 * Description:
 *      Get port isolation leaky.
 * Input:
 *      type - Packet type for port isolation leaky.
 * Output:
 *      pEnable - Leaky status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can get port isolation leaky status for RMA ,IGMP/MLD, CDP, CSSTP, and LLDP  packets.
 *      The leaky frame types are as following:
 *      - RTKSW_LEAKY_BRG_GROUP,
 *      - RTKSW_LEAKY_FD_PAUSE,
 *      - RTKSW_LEAKY_SP_MCAST,
 *      - RTKSW_LEAKY_1X_PAE,
 *      - RTKSW_LEAKY_UNDEF_BRG_04,
 *      - RTKSW_LEAKY_UNDEF_BRG_05,
 *      - RTKSW_LEAKY_UNDEF_BRG_06,
 *      - RTKSW_LEAKY_UNDEF_BRG_07,
 *      - RTKSW_LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - RTKSW_LEAKY_UNDEF_BRG_09,
 *      - RTKSW_LEAKY_UNDEF_BRG_0A,
 *      - RTKSW_LEAKY_UNDEF_BRG_0B,
 *      - RTKSW_LEAKY_UNDEF_BRG_0C,
 *      - RTKSW_LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - RTKSW_LEAKY_8021AB,
 *      - RTKSW_LEAKY_UNDEF_BRG_0F,
 *      - RTKSW_LEAKY_BRG_MNGEMENT,
 *      - RTKSW_LEAKY_UNDEFINED_11,
 *      - RTKSW_LEAKY_UNDEFINED_12,
 *      - RTKSW_LEAKY_UNDEFINED_13,
 *      - RTKSW_LEAKY_UNDEFINED_14,
 *      - RTKSW_LEAKY_UNDEFINED_15,
 *      - RTKSW_LEAKY_UNDEFINED_16,
 *      - RTKSW_LEAKY_UNDEFINED_17,
 *      - RTKSW_LEAKY_UNDEFINED_18,
 *      - RTKSW_LEAKY_UNDEFINED_19,
 *      - RTKSW_LEAKY_UNDEFINED_1A,
 *      - RTKSW_LEAKY_UNDEFINED_1B,
 *      - RTKSW_LEAKY_UNDEFINED_1C,
 *      - RTKSW_LEAKY_UNDEFINED_1D,
 *      - RTKSW_LEAKY_UNDEFINED_1E,
 *      - RTKSW_LEAKY_UNDEFINED_1F,
 *      - RTKSW_LEAKY_GMRP,
 *      - RTKSW_LEAKY_GVRP,
 *      - RTKSW_LEAKY_UNDEF_GARP_22,
 *      - RTKSW_LEAKY_UNDEF_GARP_23,
 *      - RTKSW_LEAKY_UNDEF_GARP_24,
 *      - RTKSW_LEAKY_UNDEF_GARP_25,
 *      - RTKSW_LEAKY_UNDEF_GARP_26,
 *      - RTKSW_LEAKY_UNDEF_GARP_27,
 *      - RTKSW_LEAKY_UNDEF_GARP_28,
 *      - RTKSW_LEAKY_UNDEF_GARP_29,
 *      - RTKSW_LEAKY_UNDEF_GARP_2A,
 *      - RTKSW_LEAKY_UNDEF_GARP_2B,
 *      - RTKSW_LEAKY_UNDEF_GARP_2C,
 *      - RTKSW_LEAKY_UNDEF_GARP_2D,
 *      - RTKSW_LEAKY_UNDEF_GARP_2E,
 *      - RTKSW_LEAKY_UNDEF_GARP_2F,
 *      - RTKSW_LEAKY_IGMP,
 *      - RTKSW_LEAKY_IPMULTICAST.
 *      - RTKSW_LEAKY_CDP,
 *      - RTKSW_LEAKY_CSSTP,
 *      - RTKSW_LEAKY_LLDP.
 */
rtksw_api_ret_t dal_rtl8367c_leaky_portIsolation_get(rtksw_leaky_type_t type, rtksw_enable_t *pEnable)
{
    rtksw_api_ret_t retVal;
    rtksw_uint32 port, tmp;
    rtl8367c_rma_t rmacfg;

    /* Check initialization state */
    RTKSW_CHK_INIT_STATE();

    if (type >= RTKSW_LEAKY_END)
        return RT_ERR_INPUT;

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if (type >= 0 && type <= RTKSW_LEAKY_UNDEF_GARP_2F)
    {
        if ((retVal = rtl8367c_getAsicRma(type, &rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.portiso_leaky;

    }
    else if (RTKSW_LEAKY_IPMULTICAST == type)
    {
        for (port = 0; port < RTKSW_MAX_NUM_OF_PORT; port++)
        {
            if ((retVal = rtl8367c_getAsicIpMulticastPortIsoLeaky(port, &tmp)) != RT_ERR_OK)
                return retVal;
            if (port > 0 &&(tmp != *pEnable))
                return RT_ERR_FAILED;
            *pEnable = tmp;
        }
    }
    else if (RTKSW_LEAKY_IGMP == type)
    {
        if ((retVal = rtl8367c_getAsicIGMPIsoLeaky(&tmp)) != RT_ERR_OK)
            return retVal;

        *pEnable = tmp;
    }
    else if (RTKSW_LEAKY_CDP == type)
    {
        if ((retVal = rtl8367c_getAsicRmaCdp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.portiso_leaky;
    }
    else if (RTKSW_LEAKY_CSSTP == type)
    {
        if ((retVal = rtl8367c_getAsicRmaCsstp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.portiso_leaky;
    }
    else if (RTKSW_LEAKY_LLDP == type)
    {
        if ((retVal = rtl8367c_getAsicRmaLldp(&tmp, &rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.portiso_leaky;
    }


    return RT_ERR_OK;
}

