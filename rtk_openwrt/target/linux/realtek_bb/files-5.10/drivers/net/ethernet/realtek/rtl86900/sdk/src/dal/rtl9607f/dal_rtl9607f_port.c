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
 * $Revision: 42067 $
 * $Date: 2013-08-15 14:30:04 +0800 (?Ÿæ??? 15 ?«æ? 2013) $
 *
 * Purpose : Definition of Port API
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) Parameter settings for the port-based view
 *           (2) RTCT
 */

/*
 * Include Files
 */

#include <common/rt_type.h>
#include <rtk/port.h>
#include <rtk/mdio.h>
#include <dal/rtl9607f/dal_rtl9607f.h>
#include <dal/rtl9607f/dal_rtl9607f_port.h>
#if defined(CONFIG_COMMON_RT_API)
#include <rtk/rt/rt_mdio.h>
#endif
#include <dal/rtl9607f/dal_rtl9607f_mdio.h>
#include <osal/time.h>

#include <scfg.h>
#include <cortina-api/include/port.h>
#include <cortina-api/include/eth_port.h>

#include <aal_common.h>
#include <aal_ni_l2.h>
#include <aal_port.h>
#include <aal_phy.h>
#include <aal_pon.h>
#include <aal_sds.h>

/*
 * Symbol Definition
 */
typedef struct dal_rtl9607f_phy_info_s {
    uint8   force_mode_speed[RTK_MAX_NUM_OF_PORTS];
    uint8   force_mode_duplex[RTK_MAX_NUM_OF_PORTS];
    uint8   force_mode_flowControl[RTK_MAX_NUM_OF_PORTS];
    uint8   auto_mode_pause[RTK_MAX_NUM_OF_PORTS];
    uint8   auto_mode_asy_pause[RTK_MAX_NUM_OF_PORTS];
} dal_rtl9607f_phy_info_t;

/*
 * Data Declaration
 */
 
static uint32 port_init = INIT_NOT_COMPLETED;

static ca_uint8_t eth_mdio_addr[RTK_MAX_NUM_OF_PORTS];
static dal_rtl9607f_phy_info_t   phy_info;

/*
 * Macro Declaration
 */

static int32 _dal_rtl9607f_port_phySpeed_set(uint32 port, uint32 speed)
{
    int32 ret;
    uint32 data;

    RT_PARAM_CHK(speed == PORT_SPEED_1000M, RT_ERR_CHIP_NOT_SUPPORTED);

    /* Speed is in standard register 0, bit [6,13] */
    if((ret = dal_rtl9607f_port_phyReg_get(port, 0, 0, &data)) != RT_ERR_OK)
    {
        return ret;
    }

    data &= ~((1<<13)|(1<<6));
    data |= ((speed & 0x1) << 13) | ((speed & 0x2) << 5);

    if((ret = dal_rtl9607f_port_phyReg_set(port, 0, 0, data)) != RT_ERR_OK)
    {
        return ret;
    }

    return RT_ERR_OK;
}
    
static int32 _dal_rtl9607f_port_phyDuplex_set(uint32 port, uint32 duplex)
{
    int32 ret;
    uint32 data;

    /* Duplex is in standard register 0, bit [8] */
    if((ret = dal_rtl9607f_port_phyReg_get(port, 0, 0, &data)) != RT_ERR_OK)
    {
        return ret;
    }

    data &= ~(1<<8);
    data |= (duplex & 0x1) << 8;

    if((ret = dal_rtl9607f_port_phyReg_set(port, 0, 0, data)) != RT_ERR_OK)
    {
        return ret;
    }

    return RT_ERR_OK;
}

static int32 _dal_rtl9607f_port_phyAutoNegoAbility_get(uint32 port, rtk_port_phy_ability_t *pAbility)
{
    int32 ret;
    uint32 data4, data9;

    /* Auto nego ability resides in register 4(10M/100M) & 9(1000M) */
    if((ret = dal_rtl9607f_port_phyReg_get(port, 0, 4, &data4)) != RT_ERR_OK)
    {
        return ret;
    }
    if((ret = dal_rtl9607f_port_phyReg_get(port, 0, 9, &data9)) != RT_ERR_OK)
    {
        return ret;
    }

    pAbility->FC = (data4 & (1<<10)) ? 1 : 0;
    pAbility->AsyFC = (data4 & (1<<11)) ? 1 : 0;
    pAbility->Full_100 = (data4 & (1<<8)) ? 1 : 0;
    pAbility->Half_100 = (data4 & (1<<7)) ? 1 : 0;
    pAbility->Full_10 = (data4 & (1<<6)) ? 1 : 0;
    pAbility->Half_10 = (data4 & (1<<5)) ? 1 : 0;
    pAbility->Full_1000 = (data9 & (1<<9)) ? 1 : 0;
    pAbility->Half_1000 = (data9 & (1<<8)) ? 1 : 0;

    return RT_ERR_OK;
}

static int32 _dal_rtl9607f_port_phyAutoNegoAbility_set(uint32 port, rtk_port_phy_ability_t ability)
{
    int32 ret;
    uint32 data4, data9;

    /* Auto nego ability resides in register 4(10M/100M) & 9(1000M) */
    if((ret = dal_rtl9607f_port_phyReg_get(port, 0, 4, &data4)) != RT_ERR_OK)
    {
        return ret;
    }
    if((ret = dal_rtl9607f_port_phyReg_get(port, 0, 9, &data9)) != RT_ERR_OK)
    {
        return ret;
    }

    data4 &= ~((1<<10) | (1<<11) | (1<<8) | (1<<7) | (1<<6) | (1<<5));
    data4 |= (ability.FC << 10) | (ability.AsyFC << 11) |
        (ability.Full_100 << 8) | (ability.Half_100 << 7) |
        (ability.Full_10 << 6) | (ability.Half_10 << 5);

    data9 &= ~((1<<9) | (1<<8));
    data9 |= (ability.Full_1000 << 9) | (ability.Half_1000 << 8);

    if((ret = dal_rtl9607f_port_phyReg_set(port, 0, 4, data4)) != RT_ERR_OK)
    {
        return ret;
    }
    if((ret = dal_rtl9607f_port_phyReg_set(port, 0, 9, data9)) != RT_ERR_OK)
    {
        return ret;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl9607f_port_init
 * Description:
 *      Initialize port module of the specified device.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Module must be initialized before using all of APIs in this module
 */
int32
dal_rtl9607f_port_init(void)
{
    ca_status_t ret=CA_E_OK;
    ca_device_config_tlv_t scfg_tlv;
    rtk_port_t port;
    rtk_port_phy_ability_t  ability;
    aal_ni_mac_autosync_mask_t mask;
    aal_ni_mac_autosync_cfg_t cfg;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    port_init = INIT_COMPLETED;

    osal_memset(&phy_info, 0x00, sizeof(dal_rtl9607f_phy_info_t));

    osal_memset(&scfg_tlv, 0, sizeof(scfg_tlv));
    scfg_tlv.type = CA_CFG_ID_PHY_BASE_ADDR;
    scfg_tlv.len = CFG_ID_PHY_BASE_ADDR_LEN;

    if((ret = ca_device_config_tlv_get(0, &scfg_tlv)) != CA_E_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        port_init = INIT_NOT_COMPLETED;
        return RT_ERR_FAILED;
    }

    if((ret = dal_rtl9607f_mdio_init()) != CA_E_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        port_init = INIT_NOT_COMPLETED;
        return RT_ERR_FAILED;
    }

    osal_memcpy(eth_mdio_addr,scfg_tlv.value, CFG_ID_PHY_BASE_ADDR_LEN);
    mask.s.sts_sync_enable = 1;
    mask.s.fc_sync_enable = 1;
    cfg.sts_sync_enable = 1;
    cfg.fc_sync_enable = 0;

    HAL_SCAN_ALL_PORT(port)
    {   
        if(HAL_IS_PON_PORT(port))
            continue;

        if(HAL_IS_SERDES_PORT(port))
            continue;

        if(HAL_IS_GE_PORT(port))
        {
            ability.Half_10=1;
            ability.Full_10=1;
            ability.Half_100=1;
            ability.Full_100=1;
            ability.Half_1000=1;
            ability.Full_1000=1;
            ability.Full_2500=0;
            ability.Full_10000=0;
            ability.FC=0;
            ability.AsyFC=0;
            if((ret = dal_rtl9607f_port_phyAutoNegoAbility_set(port,&ability))!= RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                port_init = INIT_NOT_COMPLETED;
                return RT_ERR_FAILED;
            }

            /* Turn on PHY/MAC auto sync function */
            if((ret = aal_ni_mac_autosync_cfg_set(0, port, mask, &cfg)) != CA_E_OK)
            {
                RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                port_init = INIT_NOT_COMPLETED;
                return RT_ERR_FAILED;
            }
        }
        else if(HAL_IS_FE_PORT(port))
        {
            ability.Half_10=1;
            ability.Full_10=1;
            ability.Half_100=1;
            ability.Full_100=1;
            ability.Half_1000=0;
            ability.Full_1000=0;
            ability.Full_2500=0;
            ability.Full_10000=0;
            ability.FC=0;
            ability.AsyFC=0;
            if((ret = dal_rtl9607f_port_phyAutoNegoAbility_set(port,&ability))!= RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                port_init = INIT_NOT_COMPLETED;
                return RT_ERR_FAILED;
            }

            /* Turn on PHY/MAC auto sync function */
            if((ret = aal_ni_mac_autosync_cfg_set(0, port, mask, &cfg)) != CA_E_OK)
            {
                RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                port_init = INIT_NOT_COMPLETED;
                return RT_ERR_FAILED;
            }
        }
    }


#if defined(CONFIG_COMMON_RT_API)
    /* Init port define for RT API */
    RT_PORT0 = 0;
    RT_PORT1 = 1;
    RT_PORT2 = 2;
    RT_PORT3 = 3;
    RT_PORT4 = 4;

    RT_PORT_PON = 7;

    RT_PORT_CPU0 = 0x10;
    RT_PORT_CPU1 = 0x11;
    RT_PORT_CPU2 = 0x12;
    RT_PORT_CPU3 = 0x13;
    RT_PORT_CPU4 = 0x14;
    RT_PORT_CPU5 = 0x15;
    RT_PORT_CPU6 = 0x16;
    RT_PORT_CPU7 = 0x17;
    RT_PORT_MAX = 0x18;
#endif

    return RT_ERR_OK;
} /* end of dal_rtl9607f_port_init */


/* Function Name:
 *      dal_rtl9607f_port_link_get
 * Description:
 *      Get the link status of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pStatus - pointer to the link status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The link status of the port is as following:
 *      - LINKDOWN
 *      - LINKUP
 */
int32
dal_rtl9607f_port_link_get(rtk_port_t port, rtk_port_linkStatus_t *pStatus)
{
    ca_status_t ret=CA_E_OK;
    ca_port_id_t port_id;
    aal_eth_port_status_t status;
    aal_ni_eth_mac_config_t config;
    aal_ni_mac_autosync_cfg_t autosync_cfg;
    aal_phy_link_status_t link_status;
    ca_boolean_t ps=0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    port_id = RTK2CA_PORT_ID(port);

    /* parameter check */
    RT_PARAM_CHK((port_id == CAERR_PORT_ID), RT_ERR_PORT_ID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pStatus), RT_ERR_NULL_POINTER);

    if((ret = aal_port_physical_port_enable_get(0,port,&ps)) != CA_E_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    if(ps == 1)
    {
        /* For different kinds ports, the source would be differnt
         * For internal PHY with auto-sync, the link status will sync from PHY
         * For all others, the MAC is always link-up
         */
        switch(port)
        {
        case CA_PORT_ID_NI0:
        case CA_PORT_ID_NI1:
        case CA_PORT_ID_NI2:
        case CA_PORT_ID_NI3:
        	/* get hardware auto sync configuration */
            if((ret = aal_ni_mac_autosync_cfg_get(0, port, &autosync_cfg)) != CA_E_OK)
            {
                RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                return RT_ERR_FAILED;
            }

            if(autosync_cfg.sts_sync_enable)
            {
                if((ret = aal_ni_eth_port_fnl_status_get(0, port, &status)) != CA_E_OK)
                {
                    RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                    return RT_ERR_FAILED;
                }
                *pStatus = ((status.link_status == CA_PORT_LINK_UP) ? PORT_LINKUP : PORT_LINKDOWN);
            }
            else
            {
                if((ret = aal_eth_phy_link_status_get(0, /* __ETH_PHY_ADDR(port) */ port + 1, &link_status)) != CA_E_OK)
                {
                    RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                    return RT_ERR_FAILED;
                }
                *pStatus = ((link_status.link_up) ? PORT_LINKUP : PORT_LINKDOWN);
            }
            break;
        case CA_PORT_ID_NI4:
        case CA_PORT_ID_NI5:
        case CA_PORT_ID_NI6:
            *pStatus = PORT_LINKUP;
            break;
        case CA_PORT_ID_NI7:
            *pStatus = PORT_LINKUP;
            break;
        }
    }
    else
    {
        *pStatus = PORT_LINKDOWN;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9607f_port_link_get */

/* Function Name:
 *      dal_rtl9607f_port_speedDuplex_get
 * Description:
 *      Get the negotiated port speed and duplex status of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pSpeed  - pointer to the port speed
 *      pDuplex - pointer to the port duplex
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN - link down port status
 * Note:
 *      (1) The speed type of the port is as following:
 *          - PORT_SPEED_10M
 *          - PORT_SPEED_100M
 *          - PORT_SPEED_1000M
 *
 *      (2) The duplex mode of the port is as following:
 *          - HALF_DUPLEX
 *          - FULL_DUPLEX
 */
int32
dal_rtl9607f_port_speedDuplex_get(
    rtk_port_t        port,
    rtk_port_speed_t  *pSpeed,
    rtk_port_duplex_t *pDuplex)
{
    ca_status_t ret=CA_E_OK;
    uint32 spd_ability;
    ca_port_id_t port_id;
    aal_eth_port_status_t status;
    aal_ni_eth_if_config_t if_config;
    aal_ni_eth_mac_config_t mac_config;
    aal_ni_mac_autosync_cfg_t autosync_cfg;
    aal_phy_link_status_t link_status;
#if !defined(CONFIG_CORTINA_BOARD_FPGA) && !defined(CONFIG_REALTEK_BOARD_FPGA)
    PSDS_MODE_t psds_mode;
#endif
    aal_sds_mode_t sds_mode;
    aal_sds_speed_t sds_speed;
    ca_boolean_t ps=0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    port_id = RTK2CA_PORT_ID(port);

    /* parameter check */
    RT_PARAM_CHK((port_id == CAERR_PORT_ID), RT_ERR_PORT_ID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pSpeed), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pDuplex), RT_ERR_NULL_POINTER);

    if((ret = aal_port_physical_port_enable_get(0,port,&ps)) != CA_E_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    if(ps == 1)
    {
        /* For different kinds ports, the source would be differnt
         * For internal PHY with auto-sync, the speed/duplex will sync from PHY
         * For other ports, each port is treated differently
         */
        switch(port)
        {
        case CA_PORT_ID_NI0:
        case CA_PORT_ID_NI1:
        case CA_PORT_ID_NI2:
        case CA_PORT_ID_NI3:
        	/* get hardware auto sync configuration */
            if((ret = aal_ni_mac_autosync_cfg_get(0, port, &autosync_cfg)) != CA_E_OK)
            {
                RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                return RT_ERR_FAILED;
            }

            if(autosync_cfg.sts_sync_enable)
            {
            if((ret = aal_ni_eth_port_fnl_status_get(0, port, &status)) != CA_E_OK)
            {
                RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                return RT_ERR_FAILED;
            }
            *pDuplex = ((status.duplex == AAL_NI_MAC_DUPLEX_FULL) ? PORT_FULL_DUPLEX : PORT_HALF_DUPLEX);
            *pSpeed = ((status.speed == AAL_NI_MAC_SPEED_1000M) ? PORT_SPEED_1000M : (status.speed == AAL_NI_MAC_SPEED_100M) ? PORT_SPEED_100M : PORT_SPEED_10M);
            }
            else
            {
                if((ret = aal_eth_phy_link_status_get(0, /* __ETH_PHY_ADDR(port) */ port + 1, &link_status)) != CA_E_OK)
                {
                    RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                    return RT_ERR_FAILED;
                }
                *pDuplex = ((link_status.duplex) ? PORT_FULL_DUPLEX : PORT_HALF_DUPLEX);
                *pSpeed = ((link_status.speed == AAL_PHY_SPEED_1000) ? PORT_SPEED_1000M : (link_status.speed == AAL_PHY_SPEED_100) ? PORT_SPEED_100M : PORT_SPEED_10M);
            }
            break;
        case CA_PORT_ID_NI4:
        case CA_PORT_ID_NI5:
        case CA_PORT_ID_NI6:
    #if defined(CONFIG_CORTINA_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA)|| defined(CONFIG_REALTEK_BOARD_FPGA_V8)
            /* No related registers in FPGA */
            *pSpeed = PORT_SPEED_10M;
    #else
            if((ret = aal_ni_eth_if_get(0, port, &if_config)) != CA_E_OK)
            {
                RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                return RT_ERR_FAILED;
            }
            *pDuplex = ((if_config.gmii_like_half_duplex == 0) ? PORT_FULL_DUPLEX : PORT_HALF_DUPLEX);

            if((ret = aal_sds_mode_get(0, port, &sds_mode, &sds_speed)) != CA_E_OK)
            {
                RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                return RT_ERR_FAILED;
            }
          
            switch(sds_speed)
            {
            case AAL_SDS_SPEED_10M:
                *pSpeed = PORT_SPEED_10M;
                break;
            case AAL_SDS_SPEED_100M:
                *pSpeed = PORT_SPEED_100M;
                break;
            case AAL_SDS_SPEED_1G:
                *pSpeed = PORT_SPEED_1000M;
                break;
            case AAL_SDS_SPEED_2G5:
                *pSpeed = PORT_SPEED_2G5;
                break;
            case AAL_SDS_SPEED_5G:
                *pSpeed = PORT_SPEED_5G;
                break;
            case AAL_SDS_SPEED_10G:
                *pSpeed = PORT_SPEED_10G;
                break;
            default:
                /* All unknown mode or disable mode return 10M as default */
                *pSpeed = PORT_SPEED_10M;
                break;
            }
    #endif
            break;
        case CA_PORT_ID_NI7:
    #if defined(CONFIG_CORTINA_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA)|| defined(CONFIG_REALTEK_BOARD_FPGA_V8)
            /* No related registers in FPGA */
            *pSpeed = PORT_SPEED_10M;
    #else
            /* MAC 7 doesn't support half duplex */
            *pDuplex = PORT_FULL_DUPLEX;

            // MODIFY ME - modify for correct READ macro
            //psds_mode.wrd  = CA_NE_QX_REG_READ(PSDS_MODE);
            psds_mode.wrd = 0;
            switch(psds_mode.bf.sds_mode_s0)
            {
            case 0x2: /* SGMII */
                /* For SGMII, it is possible that it connect to PHY so theh actual speed is controlled by below ability */
                // MODIFY ME - modify for correct READ macro
                //psds_ability.wrd = CA_NE_QX_REG_READ(PSDS_CH0_ABILITY) & 0xfff; /* Bit 16 is not used currently */
                //psds_ability.wrd = 0;
                //spd_ability = (psds_ability.bf.spd_3 << 3) | (psds_ability.bf.spd_2 << 2) | (psds_ability.bf.spd_1 << 1) | psds_ability.bf.spd_0;
                switch(spd_ability)
                {
                case AAL_SDS_CHAN_SPD_10M:
                    *pSpeed = PORT_SPEED_10M;
                    break;
                case AAL_SDS_CHAN_SPD_100M:
                    *pSpeed = PORT_SPEED_100M;
                    break;
                case AAL_SDS_CHAN_SPD_1G:
                    *pSpeed = PORT_SPEED_1000M;
                    break;
                default:
                    /* All unknown mode or disable mode return 10M as default */
                    *pSpeed = PORT_SPEED_10M;
                    break;
                }
                break;
            case 0x4: /* 1000Base-X */
                *pSpeed = PORT_SPEED_1000M;
                break;
            case 0x5: /* 100Base-FX */
                *pSpeed = PORT_SPEED_100M;
                break;
            case 0x12: /* HISGMII */
                *pSpeed = PORT_SPEED_2G5;
                break;
            case 0xd: /* USXGMII */
            case 0x1a: /* XFI */
            case 0x8: /* GPON / XGPON */
            case 0xc: /* EPON / XGEPON */
                *pSpeed = PORT_SPEED_10G;
                break;
            default:
                /* All unknown mode or disable mode return 10M as default */
                *pSpeed = PORT_SPEED_10M;
                break;
            }
    #endif
            break;
        default:
            return RT_ERR_PORT_ID;
        }
    }
    else
    {
        /* For thost port not enabled, return 10h */
        *pSpeed = PORT_SPEED_10M;
        *pDuplex = PORT_HALF_DUPLEX;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9607f_port_speedDuplex_get */

/* Function Name:
 *      dal_rtl9607f_port_flowctrl_get
 * Description:
 *      Get the negotiated flow control status of the specific port
 * Input:
 *      port      - port id
 * Output:
 *      pTxStatus - pointer to the negotiation result of the Tx flow control
 *      pRxStatus - pointer to the negotiation result of the Rx flow control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN - link down port status
 * Note:
 *      None
 */
int32
dal_rtl9607f_port_flowctrl_get(
    rtk_port_t  port,
    uint32      *pTxStatus,
    uint32      *pRxStatus)
{
    ca_status_t ret=CA_E_OK;
    uint32 spd_ability;
    ca_port_id_t port_id;
    ca_boolean_t pfc_enable, pause_tx, pause_rx;
    aal_ni_eth_mac_config_t mac_config;
    aal_ni_xge_flowctrl_t xge_flowctrl;
    ca_boolean_t ps=0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    port_id = RTK2CA_PORT_ID(port);

    /* parameter check */
    RT_PARAM_CHK((port_id == CAERR_PORT_ID), RT_ERR_PORT_ID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pTxStatus), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pRxStatus), RT_ERR_NULL_POINTER);

    if((ret = aal_port_physical_port_enable_get(0,port,&ps)) != CA_E_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    if(ps == 1)
    {
        /* For different kinds ports, the source would be differnt
         * For internal PHY with auto-sync, the speed/duplex will sync from PHY
         * For other ports, each port is treated differently
         */
        switch(port)
        {
        case CA_PORT_ID_NI0:
        case CA_PORT_ID_NI1:
        case CA_PORT_ID_NI2:
        case CA_PORT_ID_NI3:
            if((ret = aal_ni_eth_port_fnl_fc_get(0, port, &pfc_enable, &pause_tx, &pause_rx)) != CA_E_OK)
            {
                RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                return RT_ERR_FAILED;
            }
            *pTxStatus = (pause_tx == 1) ? ENABLED : DISABLED;
            *pRxStatus = (pause_rx == 1) ? ENABLED : DISABLED;
            break;
        case CA_PORT_ID_NI4:
        case CA_PORT_ID_NI5:
        case CA_PORT_ID_NI6:
            if((ret = aal_ni_eth_port_mac_get(0, port, &mac_config)) != CA_E_OK)
            {
                RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                return RT_ERR_FAILED;
            }
            *pTxStatus = (mac_config.tx_flow_disable == 0) ? ENABLED : DISABLED;
            *pRxStatus = (mac_config.rx_flow_disable == 0) ? ENABLED : DISABLED;
            
            break;
        case CA_PORT_ID_NI7:
    #if defined(CONFIG_CORTINA_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA)
            *pTxStatus = DISABLED;
            *pRxStatus = DISABLED;
    #else
            if((ret = aal_ni_pxge_flowctrl_get(0, 0, &xge_flowctrl)) != CA_E_OK)
            {
                RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                return RT_ERR_FAILED;
            }
            *pTxStatus = (xge_flowctrl.pau_tx_en == 1) ? ENABLED : DISABLED;
            *pRxStatus = (xge_flowctrl.pau_rx_en == 1) ? ENABLED : DISABLED;
    #endif
            break;
        default:
            return RT_ERR_PORT_ID;
        }
    }
    else
    {
        *pTxStatus=0;
        *pRxStatus=0;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9607f_port_flowctrl_get */

/* Function Name:
 *      dal_rtl9607f_port_phyAutoNegoEnable_get
 * Description:
 *      Get PHY ability of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pEnable - pointer to PHY auto negotiation status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_rtl9607f_port_phyAutoNegoEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    ca_status_t ret=CA_E_OK;
    ca_port_id_t port_id;
    uint32 value;
    ca_boolean_t ps=0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    port_id = RTK2CA_PORT_ID(port);

    /* parameter check */
    RT_PARAM_CHK((port_id == CAERR_PORT_ID), RT_ERR_PORT_ID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    if((ret = aal_port_physical_port_enable_get(0,port,&ps)) != CA_E_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    if(ps == 1)
    {
        if(port >= CA_PORT_ID_NI0 && port <= CA_PORT_ID_NI3)
        {
            
                /* Set/reset standard register 0 */
                if((ret = dal_rtl9607f_port_phyReg_get(port, 0, 0, &value)) != CA_E_OK)
                {
                    RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                    return RT_ERR_FAILED;
                }
                *pEnable = (value & (1<<12)) ? ENABLED : DISABLED;
        }
        else
        {
            return RT_ERR_PORT_ID;
        }
    }
    else
    {
        *pEnable = DISABLED;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9607f_port_phyAutoNegoEnable_get */

/* Function Name:
 *      dal_rtl9607f_port_phyAutoNegoEnable_set
 * Description:
 *      Set PHY ability of the specific port
 * Input:
 *      port   - port id
 *      enable - enable PHY auto negotiation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - input parameter out of range
 * Note:
 *      - ENABLED : switch to PHY auto negotiation mode
 *      - DISABLED: switch to PHY force mode
 *      - Once the abilities of both auto-nego and force mode are set,
 *        you can freely swtich the mode without calling ability setting API again
 */
int32
dal_rtl9607f_port_phyAutoNegoEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    ca_status_t ret=CA_E_OK;
    ca_port_id_t port_id;
    uint32 value;
    rtk_port_phy_ability_t ability;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    port_id = RTK2CA_PORT_ID(port);

    /* parameter check */
    RT_PARAM_CHK((port_id == CAERR_PORT_ID), RT_ERR_PORT_ID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);
    RT_PARAM_CHK(HAL_IS_PON_PORT(port), RT_ERR_CHIP_NOT_SUPPORTED);

    /* Only support for MAC with embedded PHY */
    if(port >= CA_PORT_ID_NI0 && port <= CA_PORT_ID_NI3)
    {
        /* Set/reset standard register 0 */
        if((ret = dal_rtl9607f_port_phyReg_get(port, 0, 0, &value)) != CA_E_OK)
        {
            RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
            return RT_ERR_FAILED;
        }
        if(ENABLED == enable)
        {
            value |= 1 << 12;
        }
        else
        {
            value &= ~(1<<12);
        }
        if((ret = dal_rtl9607f_port_phyReg_set(port, 0, 0, value)) != CA_E_OK)
        {
            RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
            return RT_ERR_FAILED;
        }

        /* Set the PHY to force mode ability when disable auto nego */
        if(DISABLED == enable)
        {
            if ((ret = _dal_rtl9607f_port_phySpeed_set(port, phy_info.force_mode_speed)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
            if ((ret = _dal_rtl9607f_port_phyDuplex_set(port, phy_info.force_mode_duplex)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }

            if ((ret = _dal_rtl9607f_port_phyAutoNegoAbility_get(port, &ability)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }

            ability.FC = phy_info.force_mode_flowControl[port];
            ability.AsyFC = phy_info.force_mode_flowControl[port];

            if ((ret = _dal_rtl9607f_port_phyAutoNegoAbility_set(port, ability)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
        }
        else
        {
            if ((ret = _dal_rtl9607f_port_phyAutoNegoAbility_get(port, &ability)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }

            /* Restore the auto mode FC abilities */
            ability.FC = phy_info.auto_mode_pause[port];
            ability.AsyFC = phy_info.auto_mode_pause[port];

            if ((ret = _dal_rtl9607f_port_phyAutoNegoAbility_set(port, ability)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
        }
    }
    else
    {
        return RT_ERR_PORT_ID;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9607f_port_phyAutoNegoEnable_set */

/* Function Name:
 *      dal_rtl9607f_port_phyAutoNegoAbility_get
 * Description:
 *      Get PHY auto negotiation ability of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pAbility - pointer to the PHY ability
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_rtl9607f_port_phyAutoNegoAbility_get(
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility)
{
    int32   ret;
    rtk_enable_t enable;
    rtk_port_phy_ability_t ability;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAbility), RT_ERR_NULL_POINTER);

    if ((ret = _dal_rtl9607f_port_phyAutoNegoAbility_get(port, pAbility)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
        return ret;
    }

    if ((ret = dal_rtl9607f_port_phyAutoNegoEnable_get(port, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
        return ret;
    }
    if(DISABLED == enable)
    {
        /* Retrieve from stored FC ability when in force mode */
        pAbility->FC = phy_info.auto_mode_pause[port];
        pAbility->AsyFC = phy_info.auto_mode_asy_pause[port];
    }

    return RT_ERR_OK;
} /* end of dal_rtl9607f_port_phyAutoNegoAbility_get */

/* Function Name:
 *      dal_rtl9607f_port_phyAutoNegoAbility_set
 * Description:
 *      Set PHY auto negotiation ability of the specific port
 * Input:
 *      port     - port id
 *      pAbility - pointer to the PHY ability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      You can set these abilities no matter which mode PHY currently stays on
 */
int32
dal_rtl9607f_port_phyAutoNegoAbility_set(
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility)
{
    int32   ret;
    rtk_enable_t enable;
    rtk_port_phy_ability_t ability;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAbility), RT_ERR_NULL_POINTER);

    if ((ret = _dal_rtl9607f_port_phyAutoNegoAbility_get(port, &ability)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
        return ret;
    }

    /* Store FC ability for latter usage */
    phy_info.auto_mode_pause[port] = pAbility->FC;
    phy_info.auto_mode_asy_pause[port] = pAbility->AsyFC;
    
    if ((ret = dal_rtl9607f_port_phyAutoNegoEnable_get(port, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
        return ret;
    }

    ability = *pAbility;
    if(DISABLED == enable)
    {
        /* Overwrite FC ability from force mode database */
        ability.FC = phy_info.force_mode_flowControl[port];
        ability.AsyFC = phy_info.force_mode_flowControl[port];
    }

    if ((ret = _dal_rtl9607f_port_phyAutoNegoAbility_set(port, ability)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9607f_port_phyAutoNegoAbility_set */

/* Function Name:
 *      dal_rtl9607f_port_phyForceModeAbility_get
 * Description:
 *      Get PHY ability status of the specific port
 * Input:
 *      port         - port id
 * Output:
 *      pSpeed       - pointer to the port speed
 *      pDuplex      - pointer to the port duplex
 *      pFlowControl - pointer to the flow control enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None.
 */
int32
dal_rtl9607f_port_phyForceModeAbility_get(
    rtk_port_t          port,
    rtk_port_speed_t    *pSpeed,
    rtk_port_duplex_t   *pDuplex,
    rtk_enable_t        *pFlowControl)
{
    int32   ret;
    rtk_enable_t enable;
    rtk_port_phy_ability_t ability;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pSpeed), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pDuplex), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pFlowControl), RT_ERR_NULL_POINTER);

    *pSpeed = phy_info.force_mode_speed[port];
    *pDuplex = phy_info.force_mode_duplex[port];
    *pFlowControl = phy_info.force_mode_flowControl[port];

    return RT_ERR_OK;
} /* end of dal_rtl9607f_port_phyForceModeAbility_get */

/* Function Name:
 *      dal_rtl9607f_port_phyForceModeAbility_set
 * Description:
 *      Set the port speed/duplex mode/pause/asy_pause in the PHY force mode
 * Input:
 *      port        - port id
 *      speed       - port speed
 *      duplex      - port duplex mode
 *      flowControl - enable flow control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_PHY_SPEED  - invalid PHY speed setting
 *      RT_ERR_PHY_DUPLEX - invalid PHY duplex setting
 *      RT_ERR_INPUT      - invalid input parameter
 * Note:
 *      (1) You can set these abilities no matter which mode PHY stays currently.
 *          However, these abilities only take effect when the PHY is in Force mode
 *
 *      (2) The speed type of the port is as following:
 *          - PORT_SPEED_10M
 *          - PORT_SPEED_100M
 *          - PORT_SPEED_1000M
 *
 *      (3) The duplex mode of the port is as following:
 *          - HALF_DUPLEX
 *          - FULL_DUPLEX
 */
int32
dal_rtl9607f_port_phyForceModeAbility_set(
    rtk_port_t          port,
    rtk_port_speed_t    speed,
    rtk_port_duplex_t   duplex,
    rtk_enable_t        flowControl)
{

    int32   ret;
    rtk_enable_t enable;
    rtk_port_phy_ability_t ability;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((PORT_SPEED_END <= speed), RT_ERR_INPUT);
    RT_PARAM_CHK((PORT_DUPLEX_END <= duplex), RT_ERR_INPUT);
    RT_PARAM_CHK((RTK_ENABLE_END <= flowControl), RT_ERR_INPUT);

    if ((ret = dal_rtl9607f_port_phyAutoNegoEnable_get(port, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
        return ret;
    }

    phy_info.force_mode_speed[port] = speed;
    phy_info.force_mode_duplex[port] = duplex;
    phy_info.force_mode_flowControl[port] = flowControl;

    if (DISABLED == enable)
    {
        if ((ret = _dal_rtl9607f_port_phySpeed_set(port, speed)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        if ((ret = _dal_rtl9607f_port_phyDuplex_set(port, duplex)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        if ((ret = _dal_rtl9607f_port_phyAutoNegoAbility_get(port, &ability)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        if (ENABLED == flowControl)
        {
            ability.FC = ENABLED;
            ability.AsyFC = ENABLED;
        }
        else
        {
            ability.FC = DISABLED;
            ability.AsyFC = DISABLED;
        }

        if ((ret = _dal_rtl9607f_port_phyAutoNegoAbility_set(port, ability)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of dal_rtl9607f_port_phyForceModeAbility_set */

/* Function Name:
 *      dal_rtl9607f_port_phyReg_get
 * Description:
 *      Get PHY register data of the specific port
 * Input:
 *      port  - port id
 *      page  - page id
 *      reg   - reg id
 * Output:
 *      pData - pointer to the PHY reg data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */

int32
dal_rtl9607f_port_phyReg_get(
    rtk_port_t          port,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              *pData)
{
    ca_status_t ret=CA_E_OK;
    ca_uint16_t data;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((reg > HAL_MIIM_REG_ID_MAX()), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(HAL_IS_PON_PORT(port), RT_ERR_CHIP_NOT_SUPPORTED);

    if((ret = dal_rtl9607f_mdio_cfg_set(0, 0, eth_mdio_addr[port], MDIO_FMT_C22)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    /* Write page first */
    if((ret = dal_rtl9607f_mdio_c22_write(31, page)) != CA_E_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    if((ret = dal_rtl9607f_mdio_c22_read(reg, &data)) != CA_E_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    *pData = data;

    return ret;
} /* end of dal_rtl9607f_port_phyReg_get */

/* Function Name:
 *      dal_rtl9607f_port_phyReg_set
 * Description:
 *      Set PHY register data of the specific port
 * Input:
 *      port - port id
 *      page - page id
 *      reg  - reg id
 *      data - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID     - invalid port id
 *      RT_ERR_PHY_PAGE_ID - invalid page id
 *      RT_ERR_PHY_REG_ID  - invalid reg id
 * Note:
 *      None
 */
int32
dal_rtl9607f_port_phyReg_set(
    rtk_port_t          port,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              data)
{
    ca_status_t ret=CA_E_OK;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((reg > HAL_MIIM_REG_ID_MAX()), RT_ERR_INPUT);
    RT_PARAM_CHK((data > HAL_MIIM_DATA_MAX()), RT_ERR_INPUT);
    RT_PARAM_CHK(HAL_IS_PON_PORT(port), RT_ERR_CHIP_NOT_SUPPORTED);

    if((ret = dal_rtl9607f_mdio_cfg_set(0, 0, eth_mdio_addr[port], MDIO_FMT_C22)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    /* Write page first */
    if((ret = dal_rtl9607f_mdio_c22_write(31, page)) != CA_E_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    if((ret = dal_rtl9607f_mdio_c22_write(reg, data)) != CA_E_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    return ret;
} /* end of dal_rtl9607f_port_phyReg_set */

/* Function Name:
 *      dal_rtl9607f_port_adminEnable_get
 * Description:
 *      Get port admin status of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pEnable - pointer to the port admin status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_rtl9607f_port_adminEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    ca_status_t ret=CA_E_OK;
    ca_port_id_t port_id;
    ca_boolean_t enable;
    aal_ni_eth_mac_config_t eth_mac_config;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    port_id = RTK2CA_PORT_ID(port);

    /* parameter check */
    RT_PARAM_CHK((port_id == CAERR_PORT_ID), RT_ERR_PORT_ID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    switch(port)
    {
    case CA_PORT_ID_NI0:
    case CA_PORT_ID_NI1:
    case CA_PORT_ID_NI2:
    case CA_PORT_ID_NI3:
    case CA_PORT_ID_NI4:
    case CA_PORT_ID_NI5:
    case CA_PORT_ID_NI6:
        memset(&eth_mac_config, 0, sizeof(aal_ni_eth_mac_config_t));

        if((ret = aal_ni_eth_port_mac_get(0, port, &eth_mac_config)) != CA_E_OK)
        {
            RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
            return RT_ERR_FAILED;
        }

        enable = eth_mac_config.tx_en & eth_mac_config.rx_en;

        if(enable == 1)
            *pEnable = ENABLED;
        else
            *pEnable = DISABLED;

        break;
    default:
        return RT_ERR_FEATURE_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9607f_port_adminEnable_get */

/* Function Name:
 *      dal_rtl9607f_port_adminEnable_set
 * Description:
 *      Set port admin status of the specific port
 * Input:
 *      port    - port id
 *      enable  - port admin status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
int32
dal_rtl9607f_port_adminEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    ca_status_t ret=CA_E_OK;
    ca_port_id_t port_id;
    ca_boolean_t ca_enable;
    aal_ni_eth_mac_config_mask_t eth_mac_mask;
    aal_ni_eth_mac_config_t eth_mac_config;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    port_id = RTK2CA_PORT_ID(port);

    /* parameter check */
    RT_PARAM_CHK((port_id == CAERR_PORT_ID), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    if(enable == ENABLED)
        ca_enable = 1;
    else
        ca_enable = 0;

    switch(port)
    {
    case CA_PORT_ID_NI0:
    case CA_PORT_ID_NI1:
    case CA_PORT_ID_NI2:
    case CA_PORT_ID_NI3:
    case CA_PORT_ID_NI4:
    case CA_PORT_ID_NI5:
    case CA_PORT_ID_NI6:
        memset(&eth_mac_mask, 0, sizeof(aal_ni_eth_mac_config_mask_t));
        memset(&eth_mac_config, 0, sizeof(aal_ni_eth_mac_config_t));

        eth_mac_mask.s.mac_tx_rst = 1;
        eth_mac_mask.s.tx_en      = 1;
        eth_mac_mask.s.mac_rx_rst = 1;
        eth_mac_mask.s.rx_en      = 1;

        eth_mac_config.mac_tx_rst = !ca_enable;
        eth_mac_config.tx_en      = ca_enable;
        eth_mac_config.mac_rx_rst = !ca_enable;
        eth_mac_config.rx_en      = ca_enable;

        if((ret = aal_ni_eth_port_mac_set(0, port, eth_mac_mask, &eth_mac_config)) != CA_E_OK)
        {
            RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
            return RT_ERR_FAILED;
        }
        break;
    default:
        return RT_ERR_FEATURE_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9607f_port_adminEnable_set */

/* Function Name:
 *      dal_rtl9607f_port_phyPowerDown_get
 * Description:
 *      Get PHY power down state of the specified port.
 * Input:
 *      port    - port id
 * Output:
 *      pEnable - pointer to power down enable state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      none
 */
int32
dal_rtl9607f_port_phyPowerDown_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    int32 ret=RT_ERR_OK;
    ca_port_id_t port_id;
    uint32 data;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    port_id = RTK2CA_PORT_ID(port);

    /* parameter check */
    RT_PARAM_CHK((port_id == CAERR_PORT_ID), RT_ERR_PORT_ID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(HAL_IS_PON_PORT(port), RT_ERR_CHIP_NOT_SUPPORTED);

    if(port>=AAL_NI_PORT_GE0 && port<=AAL_NI_PORT_GE3)
    {
        if((ret = dal_rtl9607f_port_phyReg_get(port,0,0,&data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
            return RT_ERR_FAILED;
        }

        if(data&(1<<11))
        {
            *pEnable = ENABLED;
        }
        else
        {
            *pEnable = DISABLED;
        }
    }
    else
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9607f_port_phyPowerDown_get */

/* Function Name:
 *      dal_rtl9607f_port_phyPowerDown_set
 * Description:
 *      Set PHY power down state of the specified port.
 * Input:
 *      port   - port id
 *      enable - PHY power down enable state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      none
 */
int32
dal_rtl9607f_port_phyPowerDown_set(rtk_port_t port, rtk_enable_t enable)
{
    int32 ret=CA_E_OK;
    ca_port_id_t port_id;
    uint32 data;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    port_id = RTK2CA_PORT_ID(port);

    /* parameter check */
    RT_PARAM_CHK((port_id == CAERR_PORT_ID), RT_ERR_PORT_ID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(HAL_IS_PON_PORT(port), RT_ERR_CHIP_NOT_SUPPORTED);

    if(port>=AAL_NI_PORT_GE0 && port<=AAL_NI_PORT_GE3)
    {
        if((ret = dal_rtl9607f_port_phyReg_get(port,0,0,&data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
            return RT_ERR_FAILED;
        }
        if(enable == ENABLED)
        {
            data |= (1<<11);

            if((ret = dal_rtl9607f_port_adminEnable_set(port,DISABLED)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                return RT_ERR_FAILED;
            }
        }
        else
        {
            data &= ~(1<<11);

            if((ret = dal_rtl9607f_port_adminEnable_set(port,ENABLED)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                return RT_ERR_FAILED;
            }
        }

        if((ret = dal_rtl9607f_port_phyReg_set(port,0,0,data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
            return RT_ERR_FAILED;
        }

        
    }
    else
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9607f_port_phyPowerDown_set */

/* Function Name:
 *      dal_rtl9607f_port_isolationEntry_get
 * Description:
 *      Get Port isolation portmask
 * Input:
 *      mode            - Configuration 0 or 1
 *      port            - Ingress port
 * Output:
 *      pPortmask       - Isolation portmask for specified ingress port.
 *      pExtPortmask    - Isolation extension portmask for specified ingress port.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None.
 */
int32
dal_rtl9607f_port_isolationEntry_get(rtk_port_isoConfig_t mode, rtk_port_t port, rtk_portmask_t *pPortmask, rtk_portmask_t *pExtPortmask)
{
    ca_status_t ret=CA_E_OK;
    ca_port_id_t port_id;
    ca_port_id_t dst_ports;
    ca_uint32_t lpid = AAL_LPORT_INVALID;
    ca_uint64_t mmshp_bmp = 0;
    ca_uint32_t isolation_bmp = -1;
    uint8 i;
    rtk_portmask_t blockPortMask;
    ca_uint16_t port_type = CA_PORT_TYPE_INVALID;

    /* check Init status */
    RT_INIT_CHK(port_init);

    port_id = RTK2CA_PORT_ID(port);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pExtPortmask), RT_ERR_NULL_POINTER);
    
    lpid = PORT_ID(port_id);
    ret = aal_port_mmshp_check_get(0, lpid, &mmshp_bmp);
    isolation_bmp &= ~mmshp_bmp;

    memset(&blockPortMask,0,sizeof(rtk_portmask_t));

    for (i = 0; i < 64; i++) {
        if (isolation_bmp & (ca_uint64) 1 << i) {
            port_type = CA_PORT_TYPE_INVALID;
            if (lpid >= CA_PORT_ID_NI0 && lpid <= CA_PORT_ID_NI7)
                port_type = CA_PORT_TYPE_ETHERNET;
            else if (lpid == CA_PORT_ID_L3_LAN || CA_PORT_ID_L3_WAN)
                port_type = CA_PORT_TYPE_L2RP;
            else if (lpid >= CA_PORT_ID_CPU0 && lpid <= CA_PORT_ID_CPU7)
                port_type = CA_PORT_TYPE_CPU;
            dst_ports = CA_PORT_ID(port_type, i);

            RT_PARAM_CHK(!HAL_IS_PORT_EXIST(PORT_ID(dst_ports)), RT_ERR_PORT_ID);

            blockPortMask.bits[0] |= 1<<PORT_ID(dst_ports);
        }
    }

    HAL_GET_ALL_PORTMASK(*pPortmask);

    for(i=0;i<32;i++)
    {
        if(blockPortMask.bits[0]&(1<<i))
        {
            pPortmask->bits[0] = pPortmask->bits[0]&~(1<<i);
        }
    }

    memset(pExtPortmask,0,sizeof(rtk_portmask_t));

    return RT_ERR_OK;
} /* end of dal_rtl9607f_port_isolationEntry_get */

/* Function Name:
 *      dal_rtl9607f_port_isolationEntry_set
 * Description:
 *      Set Port isolation portmask
 * Input:
 *      mode            - Configuration 0 or 1
 *      port            - Ingress port
 *      pPortmask       - Isolation portmask for specified ingress port.
 *      pExtPortmask    - Isolation extension portmask for specified ingress port.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None.
 */
int32
dal_rtl9607f_port_isolationEntry_set(rtk_port_isoConfig_t mode, rtk_port_t port, rtk_portmask_t *pPortmask, rtk_portmask_t *pExtPortmask)
{
    ca_status_t ret=CA_E_OK;
    ca_port_id_t port_id;
    ca_port_id_t dst_ports[RTK_MAX_NUM_OF_PORTS];
    ca_uint32_t lpid = AAL_LPORT_INVALID;
    uint8 i;
    rtk_portmask_t blockPortMask;
    ca_uint64_t isolation_bmp = 0;
    ca_uint64_t mmshp_bmp = -1;

    /* check Init status */
    RT_INIT_CHK(port_init);

    port_id = RTK2CA_PORT_ID(port);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(!HAL_IS_PORTMASKPRT_VALID(pPortmask), RT_ERR_PORT_ID);
    RT_PARAM_CHK(!HAL_IS_EXTPORTMASK_VALID((*pExtPortmask)), RT_ERR_PORT_ID);

    HAL_GET_ALL_PORTMASK(blockPortMask);

    for(i=0;i<32;i++)
    {
        if(pPortmask->bits[0]&(1<<i))
        {
            blockPortMask.bits[0] = blockPortMask.bits[0]&~(1<<i);
        }
    }

    for(i=0;i<32;i++)
    {
        if(blockPortMask.bits[0]&(1<<i))
        {
            lpid = PORT_ID(RTK2CA_PORT_ID(i));
            isolation_bmp |= (ca_uint64) 1 << lpid;
        }
    }

    mmshp_bmp &= ~isolation_bmp;
    lpid = PORT_ID(port_id);

    if((ret = aal_port_mmshp_check_set(0, lpid, mmshp_bmp)) != CA_E_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9607f_port_isolationEntry_set */


#if defined(CONFIG_SDK_KERNEL_LINUX)
static int _rtl9607f_lan_sds0_mode_set(uint32 mode)
{
    int ret;

#if !defined(CONFIG_REALTEK_BOARD_FPGA) && !defined(CONFIG_REALTEK_BOARD_FPGA_V8)
    switch(mode)
    {
    case LAN_SDS_MODE_GE_FE_PHY:
        ret = aal_sds_mode_set(0, 4, AAL_SDS_MODE_NONE, AAL_SDS_SPEED_DEFAULT);
        break;        
    case LAN_SDS_MODE_SGMII_MAC:
        ret = aal_sds_mode_set(0, 4, AAL_SDS_MODE_SGMII, AAL_SDS_SPEED_1G);
        break;
    case LAN_SDS_MODE_HSGMII_MAC:
        ret = aal_sds_mode_set(0, 4, AAL_SDS_MODE_SGMII, AAL_SDS_SPEED_2G5);
        break;
    default:
        return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    if(ret)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return RT_ERR_FAILED;
    }
#endif

    return RT_ERR_OK;
}

static int _rtl9607f_lan_sds1_mode_set(uint32 mode)
{
    int ret;

#if !defined(CONFIG_REALTEK_BOARD_FPGA) && !defined(CONFIG_REALTEK_BOARD_FPGA_V8)
    switch(mode)
    {
    case LAN_SDS_MODE_GE_FE_PHY:
        ret = aal_sds_mode_set(0, 5, AAL_SDS_MODE_NONE, AAL_SDS_SPEED_DEFAULT);
        break;        
    case LAN_SDS_MODE_RXAUI:
        ret = aal_sds_mode_set(0, 5, AAL_SDS_MODE_RXAUI, AAL_SDS_SPEED_10G);
        break;
    case LAN_SDS_MODE_RGMII:
        ret = aal_sds_mode_set(0, 5, AAL_SDS_MODE_RGMII, AAL_SDS_SPEED_1G);
        break;
    default:
        return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    if(ret)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return RT_ERR_FAILED;
    }
#endif

    return RT_ERR_OK;
}

static int _rtl9607f_lan_sds2_mode_set(uint32 mode)
{
    int ret;

#if !defined(CONFIG_REALTEK_BOARD_FPGA) && !defined(CONFIG_REALTEK_BOARD_FPGA_V8)
    switch(mode)
    {
    case LAN_SDS_MODE_GE_FE_PHY:
        ret = aal_sds_mode_set(0, 6, AAL_SDS_MODE_NONE, AAL_SDS_SPEED_DEFAULT);
        break;        
    case LAN_SDS_MODE_SGMII_MAC:
        ret = aal_sds_mode_set(0, 6, AAL_SDS_MODE_SGMII, AAL_SDS_SPEED_1G);
        break;
    case LAN_SDS_MODE_FIBER_1G:
        ret = aal_sds_mode_set(0, 6, AAL_SDS_MODE_FIBER, AAL_SDS_SPEED_1G);
        break;
    case LAN_SDS_MODE_HSGMII_MAC:
        ret = aal_sds_mode_set(0, 6, AAL_SDS_MODE_SGMII, AAL_SDS_SPEED_2G5);
        break;
    case LAN_SDS_MODE_USXGMII:
        ret = aal_sds_mode_set(0, 6, AAL_SDS_MODE_USXGMII, AAL_SDS_SPEED_10G);
        break;
    case LAN_SDS_MODE_QXGMII:
        ret = aal_sds_mode_set(0, 6, AAL_SDS_MODE_QXGMII, AAL_SDS_SPEED_DEFAULT);
        break;
    default:
        return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    if(ret)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return RT_ERR_FAILED;
    }
#endif

    return RT_ERR_OK;
}
#endif

static uint8 lan_sds_mode[3] = { LAN_SDS_MODE_END, LAN_SDS_MODE_END, LAN_SDS_MODE_END};
static rtk_port_speed_t lan_sds_speed[3] = { PORT_SPEED_END, PORT_SPEED_END, PORT_SPEED_END};

/* Function Name:
 *      dal_rtl9607f_port_serdesMode_set
 * Description:
 *      Set Serdes Mode
 * Input:
 *      num   - serdes number
 *      cfg   - serdes mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 * Note:
 *      For RTL9607f, num is 0~2 for port 4, 5, 6 SDS configuration
 */
int32
dal_rtl9607f_port_serdesMode_set(uint8 num, uint8 cfg)
{
    int32 ret;
    uint32 chip, rev, subtype;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "num=%d,cfg=%d",num, cfg);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK((LAN_SDS_MODE_END <=cfg), RT_ERR_INPUT);

    /* function body */
#if defined(CONFIG_SDK_KERNEL_LINUX)
    switch(num)
    {
    case 0:
        if((ret = _rtl9607f_lan_sds0_mode_set(cfg)) != RT_ERR_OK)
        {
            return RT_ERR_CHIP_NOT_SUPPORTED;
        }
        break;
    case 1:
        if((ret = _rtl9607f_lan_sds1_mode_set(cfg)) != RT_ERR_OK)
    {
        return RT_ERR_CHIP_NOT_SUPPORTED;
    }
        break;
    case 2:
        if((ret = _rtl9607f_lan_sds2_mode_set(cfg)) != RT_ERR_OK)
        {
            return RT_ERR_CHIP_NOT_SUPPORTED;
        }
        break;
    default:
        return RT_ERR_INPUT;
    }
    lan_sds_mode[num] = cfg;
#endif

    return RT_ERR_OK;
}   /* end of dal_rtl9607f_port_serdesMode_set */

/* Function Name:
 *      dal_rtl9607f_port_serdesMode_get
 * Description:
 *      Get Serdes Mode
 * Input:
 *      num   - serdes number
 * Output:
 *      cfg   - serdes mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      For RTL9607f, num is 0~2 for port 4, 5, 6 SDS configuration
 */
int32
dal_rtl9607f_port_serdesMode_get(uint8 num, uint8 *cfg)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "num=%d",num);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK((3 <=num), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == cfg), RT_ERR_NULL_POINTER);

    /* function body */
    *cfg = lan_sds_mode[num];
    
    return RT_ERR_OK;
}   /* end of dal_rtl9607f_port_serdesMode_get */

/* Function Name:
 *      dal_rtl9607f_port_serdesModeSpeed_set
 * Description:
 *      Set Serdes Mode
 * Input:
 *      num   - serdes number
 *      mode  - serdes mode
 *      speed - serdes speed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT
 * Note:
 *      For RTL9607f, num is 0~2 for port 4, 5, 6 SDS configuration
 *      Each Serdes mode only suppport specific speeds
 */
int32
dal_rtl9607f_port_serdesModeSpeed_set(uint8 num, rtk_port_sds_mode_t mode, rtk_port_speed_t speed)
{
    int32 ret;
    uint32 chip, rev, subtype;
    aal_sds_speed_t aal_speed;
    aal_sds_mode_t aal_mode;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "num=%d,mode=%d,speed=%d\n",num, mode,speed);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK((LAN_SDS_MODE_END <= mode), RT_ERR_INPUT);
    RT_PARAM_CHK((PORT_SPEED_END <= speed), RT_ERR_INPUT);

    /* function body */
    /* Translate RTK speed to AAL speed */
    switch(speed)
    {
    case PORT_SPEED_10M:
        aal_speed = AAL_SDS_SPEED_10M;
        break;
    case PORT_SPEED_100M:
        aal_speed = AAL_SDS_SPEED_100M;
        break;
    case PORT_SPEED_1000M:
        aal_speed = AAL_SDS_SPEED_1G;
        break;
    case PORT_SPEED_10G:
        aal_speed = AAL_SDS_SPEED_10G;
        break;
    case PORT_SPEED_2G5:
        aal_speed = AAL_SDS_SPEED_2G5;
        break;
    case PORT_SPEED_5G:
        aal_speed = AAL_SDS_SPEED_5G;
        break;
    default:
        return RT_ERR_INPUT;
    }

    switch(mode)
    {
    case LAN_SDS_MODE_FIBER_1G:
        aal_mode = AAL_SDS_MODE_FIBER;
        break;
    case LAN_SDS_MODE_SGMII_MAC:
        aal_mode = AAL_SDS_MODE_SGMII;
        break;
    case LAN_SDS_MODE_HSGMII_MAC:
        aal_mode = AAL_SDS_MODE_SGMII;
        break;
    case LAN_SDS_MODE_RXAUI:
        aal_mode = AAL_SDS_MODE_RXAUI;
        break;
    case LAN_SDS_MODE_XFI:
        aal_mode = AAL_SDS_MODE_XFI;
        break;
    case LAN_SDS_MODE_USXGMII:
        aal_mode = AAL_SDS_MODE_USXGMII;
        break;
    case LAN_SDS_MODE_RGMII:
        aal_mode = AAL_SDS_MODE_RGMII;
        break;
    default:
        return RT_ERR_INPUT;
    }

    switch(num)
    {
    case 0:
        if((ret = aal_sds_mode_set(0, 4, aal_mode, aal_speed)) != RT_ERR_OK)
        {
            return RT_ERR_CHIP_NOT_SUPPORTED;
        }
        break;
    case 1:
        if((ret = aal_sds_mode_set(0, 5, aal_mode, aal_speed)) != RT_ERR_OK)
        {
            return RT_ERR_CHIP_NOT_SUPPORTED;
        }
        break;
    case 2:
        if((ret = aal_sds_mode_set(0, 6, aal_mode, aal_speed)) != RT_ERR_OK)
        {
            return RT_ERR_CHIP_NOT_SUPPORTED;
        }
        break;
    default:
        return RT_ERR_INPUT;
    }
    lan_sds_mode[num] = mode;
    lan_sds_speed[num] = speed;

    return RT_ERR_OK;
}   /* end of dal_rtl9607f_port_serdesModeSpeed_set */

/* Function Name:
 *      dal_rtl9607f_port_serdesModeSpeed_get
 * Description:
 *      Get Serdes Mode
 * Input:
 *      num   - serdes number
 * Output:
 *      pMode - serdes mode
 *      pSpeed - serdes speed
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      For RTL9607f, num is 0~2 for port 4, 5, 6 SDS configuration
 */
int32
dal_rtl9607f_port_serdesModeSpeed_get(uint8 num, rtk_port_sds_mode_t *pMode, rtk_port_speed_t *pSpeed)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "num=%d",num);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK((3 <=num), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);

    /* function body */
    *pMode = lan_sds_mode[num];
    *pSpeed = lan_sds_mode[num];
    
    return RT_ERR_OK;
}   /* end of dal_rtl9607f_port_serdesModeSpeed_get */

/* Function Name:
 *      dal_rtl9607f_port_serdesNWay_set
 * Description:
 *      Set Serdes N-Way Mode
 * Input:
 *      num   - serdes number
 *      cfg   - serdes n-way mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 * Note:
 *      None
 */
int32
dal_rtl9607f_port_serdesNWay_set(uint8 num, uint8 cfg)
{
    int ret;
    uint32 aal_port;
    uint32 aal_enable;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "num=%d,cfg=%d",num, cfg);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK((3 <=num), RT_ERR_INPUT);
    RT_PARAM_CHK((LAN_SDS_NWAY_END <=cfg), RT_ERR_INPUT);

    /* function body */
    switch(num)
    {
    case 0:
        aal_port = 4;
        break;
    case 1:
        aal_port = 5;
        break;
    case 2:
        aal_port = 6;
        break;
    default:
        return RT_ERR_INPUT;
    }

    if(LAN_SDS_NWAY_AUTO == cfg)
    {
        aal_enable = 1;
    }
    else
    {
        aal_enable = 0;
    }

    if((ret = aal_sds_nway_set(0, aal_port, aal_enable) != CA_E_OK))
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl9607f_port_serdesNWay_get
 * Description:
 *      Get Serdes N-Way Mode
 * Input:
 *      num   - serdes number
 * Output:
 *      cfg   - serdes n-way mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_rtl9607f_port_serdesNWay_get(uint8 num, uint8 *cfg)
{
    int ret;
    uint32 aal_port;
    uint32 aal_enable;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "num=%d",num);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK((3 <=num), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == cfg), RT_ERR_NULL_POINTER);

    /* function body */
    switch(num)
    {
    case 0:
        aal_port = 4;
        break;
    case 1:
        aal_port = 5;
        break;
    case 2:
        aal_port = 6;
        break;
    default:
        return RT_ERR_INPUT;
    }

    if((ret = aal_sds_nway_get(0, aal_port, &aal_enable) != CA_E_OK))
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    if(aal_enable)
    {
        *cfg = LAN_SDS_NWAY_AUTO;
    }
    else
    {
        *cfg = LAN_SDS_NWAY_FORCE;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl9607f_port_eeeEnable_set
 * Description:
 *      Set EEE enable/disable state
 * Input:
 *      port   - port id
 *      enable - enable status of EEE
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 * Note:
 *      None
 */
int32
dal_rtl9607f_port_eeeEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    int32 ret;
    uint32 rData, wData;
    rtk_enable_t nwayEnable;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);
    RT_PARAM_CHK(HAL_IS_PON_PORT(port), RT_ERR_PORT_ID);

    if(ENABLED == enable)
    {
        /* PHY page 0xa5d reg 16 bit 1 & 2 set to 1 */
        if((ret = dal_rtl9607f_port_phyReg_get(port, 0xa5d, 16, &rData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PORT|MOD_DAL), "");
            return ret;
        }
        wData = rData | ((1 << 2) | (1 << 1));
        if((ret = dal_rtl9607f_port_phyReg_set(port, 0xa5d, 16, wData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PORT|MOD_DAL), "");
            return ret;
        }

        /* PHY page 0xa43 reg 25 bit 4 set to 1 */
        if((ret = dal_rtl9607f_port_phyReg_get(port, 0xa43, 25, &rData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PORT|MOD_DAL), "");
            return ret;
        }
        wData = rData | (1 << 4);
        if((ret = dal_rtl9607f_port_phyReg_set(port, 0xa43, 25, wData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PORT|MOD_DAL), "");
            return ret;
        }
    }
    else
    {
        /* PHY page 0xa5d reg 16 bit 1 & 2 set to 0 */
        if((ret = dal_rtl9607f_port_phyReg_get(port, 0xa5d, 16, &rData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PORT|MOD_DAL), "");
            return ret;
        }
        wData = rData & ~((1 << 2) | (1 << 1));
        if((ret = dal_rtl9607f_port_phyReg_set(port, 0xa5d, 16, wData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PORT|MOD_DAL), "");
            return ret;
        }
        /* PHY page 0xa43 reg 25 bit 4 set to 0 */
        if((ret = dal_rtl9607f_port_phyReg_get(port, 0xa43, 25, &rData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PORT|MOD_DAL), "");
            return ret;
        }
        wData = rData & ~(1 << 4);
        if((ret = dal_rtl9607f_port_phyReg_set(port, 0xa43, 25, wData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PORT|MOD_DAL), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of dal_rtl9607f_port_eeeEnable_set */

/* Function Name:
 *      dal_rtl9607f_port_eeeEnable_get
 * Description:
 *      Get EEE enable/disable state
 * Input:
 *      port   - port id
 * Output:
 *      pEnable - enable status of EEE
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_rtl9607f_port_eeeEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32 rData;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(HAL_IS_PON_PORT(port), RT_ERR_PORT_ID);

    /* Use some of the configured bits to represent the enable state */
    /* Check if PHY page 0xa5d reg 16 bit 1 & 2 are 1 */
    if((ret = dal_rtl9607f_port_phyReg_get(port, 0xa5d, 16, &rData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

    if((rData & 0x6) != 0)
    {
        *pEnable = ENABLED;
    }
    else
    {
        *pEnable = DISABLED;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9607f_port_eeeEnable_get */

