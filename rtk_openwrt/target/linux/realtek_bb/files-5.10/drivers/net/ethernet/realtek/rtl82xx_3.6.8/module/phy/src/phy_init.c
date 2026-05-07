/*
 * Copyright (C) 2009-2018 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Revision:
 * Date: 2018-06-15
 *
 * Purpose : PHY Init module.
 *
 * Feature : This Module is used to initial the separate PHY driver.
 *
 */

#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#include <hal/chipdef/driver.h>
#include <hal/common/halctrl.h>
#include <hal/phy/identify.h>
#include <hal/phy/phy_probe.h>
#include <hal/phy/phy_construct.h>
#include <phy/inc/util/phy_wrapped_miim.h>
#include <phy/inc/phy_hal.h>
#include <phy/inc/util/phy_waMon.h>
#include <dal/dal_mgmt.h>
#include <dal/dal_phy.h>

#include <phy_init.h>
#include <osal/lib.h>
#include <osal/memory.h>


extern hwp_hwProfile_t phy_driver_hwp;


hal_control_t   phy_hal_ctrl[RTK_MAX_NUM_OF_UNIT];


dal_mgmt_info_t     *pMgmt_node[1];


/* driver service APIs */
rt_macdrv_t phyMiiDrv =
{
    .fMdrv_init                             = NULL,
    .fMdrv_miim_read                        = wrapped_miim_read,
    .fMdrv_miim_write                       = wrapped_miim_write,
    .fMdrv_miim_park_read                   = wrapped_miim_park_read,
    .fMdrv_miim_park_write                  = wrapped_miim_park_write,
    .fMdrv_miim_broadcast_write             = NULL,
    .fMdrv_miim_extParkPage_read            = NULL,
    .fMdrv_miim_extParkPage_write           = NULL,
    .fMdrv_miim_extParkPage_portmask_write  = NULL,
    .fMdrv_miim_mmd_read                    = wrapped_mmd_read,
    .fMdrv_miim_mmd_write                   = wrapped_mmd_write,
    .fMdrv_miim_mmd_portmask_write          = NULL,
    .fMdrv_table_read                       = NULL,
    .fMdrv_table_write                      = NULL,
    .fMdrv_port_probe                       = NULL,
    .fMdrv_miim_portmask_write              = NULL,
    .fMdrv_miim_pollingEnable_get           = wrapped_miim_pollingEnable_get,
    .fMdrv_miim_pollingEnable_set           = wrapped_miim_pollingEnable_set,
    .fMdrv_mac_serdes_rst                   = NULL,
    .fMdrv_mac_serdes_read                  = NULL,
    .fMdrv_mac_serdes_write                 = NULL,
    .fMdrv_smi_read                         = NULL,
    .fMdrv_smi_write                        = NULL,
    .fMdrv_miim_portSmiMdxProto_get         = phy_hal_miim_portSmiMdxProto_get,
    .fMdrv_miim_portSmiMdxProto_set         = phy_hal_miim_portSmiMdxProto_set,
}; /* end of phyMiiDrv */


/* Function Name:
 *      phy_hwp_init
 * Description:
 *      fill HWP port and phy structure with customer's configuration.
 * Input:
 *      profile  - HWP pointer.
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
int32 phy_hwp_init(phy_hwp_portDescp_t *portDesc, phy_hwp_phyDescp_t  *phyDesc)
{
    int32   scan_idx;
    int32   empty_idx = 0;

    hwp_hwProfile_t *profile;

    profile = &phy_driver_hwp;
    if ((portDesc == NULL) || (phyDesc == NULL))
    {
        return RT_ERR_INPUT;
    }

    for (scan_idx = 0; scan_idx < RTK_MAX_PORT_PER_UNIT; scan_idx++)
    {
        if (empty_idx == 0)
        {
            if (portDesc[scan_idx].mac_id != HWP_END)
            {
                profile->swDescp[0]->port.descp[scan_idx].mac_id = portDesc[scan_idx].mac_id;
                profile->swDescp[0]->port.descp[scan_idx].phy_idx = portDesc[scan_idx].phy_idx;
                profile->swDescp[0]->port.descp[scan_idx].eth = portDesc[scan_idx].eth;
                profile->swDescp[0]->port.descp[scan_idx].medi = portDesc[scan_idx].medi;
                profile->swDescp[0]->port.descp[scan_idx].attr = HWP_ETH;
                profile->swDescp[0]->port.descp[scan_idx].smi = portDesc[scan_idx].smi;
                profile->swDescp[0]->port.descp[scan_idx].phy_addr = portDesc[scan_idx].phy_addr;
            }
            else
            {
                empty_idx = 1;
                profile->swDescp[0]->port.descp[scan_idx].mac_id = HWP_END;
            }
        }
        else
        {
            osal_memset(&profile->swDescp[0]->port.descp[scan_idx], 0, sizeof(hwp_portDescp_t));
        }
    }

    empty_idx = 0;
    for (scan_idx = 0; scan_idx < RTK_MAX_PHY_PER_UNIT; scan_idx++)
    {
        if (empty_idx == 0)
        {
            if (phyDesc[scan_idx].chip != HWP_END)
            {
                profile->swDescp[0]->phy.descp[scan_idx].chip = phyDesc[scan_idx].chip;
                profile->swDescp[0]->phy.descp[scan_idx].phy_max = phyDesc[scan_idx].phy_max;
                profile->swDescp[0]->phy.descp[scan_idx].mac_id = phyDesc[scan_idx].mac_id;

            }
            else
            {
                empty_idx = 1;
                profile->swDescp[0]->phy.descp[scan_idx].chip = HWP_END;
            }
        }
        else
        {
            osal_memset(&profile->swDescp[0]->phy.descp[scan_idx], 0, sizeof(hwp_phyDescp_t));
        }
    }

    hwp_myHwProfile = profile;
    hwp_myHwProfile->parsed_info = &parsedInfoEmpty;
    return hwp_init(NULL);

}



/* Function Name:
 *      _rtk_phy_hal_ctrl_init
 * Description:
 *      Init PHY HAL
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
_rtk_phy_hal_ctrl_init(uint32 unit)
{
    rt_driver_t     *pChip_driver;

    if (unit >= RTK_MAX_NUM_OF_UNIT)
    {
        return RT_ERR_UNIT_ID;
    }

    osal_memset(&phy_hal_ctrl[unit], 0, sizeof(hal_control_t));

    phy_hal_ctrl[unit].miim_sem = osal_sem_mutex_create();

    if ((pChip_driver = osal_alloc(sizeof(rt_driver_t))) == NULL)
    {
        return RT_ERR_MEM_ALLOC;
    }
    osal_memset(pChip_driver, 0, sizeof(rt_driver_t));

    /* hook MDC/MDIO driver */
    pChip_driver->pMacdrv = &phyMiiDrv;

    phy_hal_ctrl[unit].pChip_driver = pChip_driver;

    phy_hal_ctrl[unit].chip_flags |= HAL_CHIP_ATTACHED;

    return RT_ERR_OK;
}



/* Function Name:
 *      rtk_init
 * Description:
 *      Init RTK
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
rtk_init(rtk_phy_initInfo_t *pInitInfo)
{
    int32   ret;
    uint32  unit;

    pMgmt_node[0] = (dal_mgmt_info_t *)osal_alloc(sizeof(dal_mgmt_info_t));
    if (NULL == pMgmt_node[0])
    {
        return RT_ERR_MEM_ALLOC;
    }
    osal_memset(pMgmt_node[0], 0, sizeof(dal_mgmt_info_t));
    pMgmt_node[0]->pMapper = (dal_mapper_t *)osal_alloc(sizeof(dal_mapper_t));
    if (NULL == pMgmt_node[0]->pMapper)
    {
        return RT_ERR_MEM_ALLOC;
    }
    osal_memset(pMgmt_node[0]->pMapper, 0, sizeof(dal_mapper_t));

    if ((ret = phy_hwp_init(pInitInfo->port_desc, pInitInfo->phy_desc)) != RT_ERR_OK)
    {
        return ret;
    }


    phy_identify_init();

    HWP_UNIT_TRAVS_LOCAL(unit)
    {
        if ((ret = _rtk_phy_hal_ctrl_init(unit)) != RT_ERR_OK)
        {
            return ret;
        }
    }


    HWP_UNIT_TRAVS_LOCAL(unit)
    {
        /* Probe PHY */
        if ((ret = phy_probe(unit)) != RT_ERR_OK)
        {
            return ret;
        }

        /* Construct PHY */
        phy_construct_config_init(unit);

        /* Init PHY */
        if ((ret = phy_init(unit)) != RT_ERR_OK)
        {
            return ret;
        }

        /* Init PHY module */
        if ((ret = dal_phyMapper_init(pMgmt_node[0]->pMapper)) != RT_ERR_OK)
        {
            return ret;
        }
        pMgmt_node[0]->pMapper->phy_init(unit);
    }

    if ((ret = dal_waMon_init()) != RT_ERR_OK)
    {
        return ret;
    }

    if ((ret = dal_waMon_enable()) != RT_ERR_OK)
    {
        return ret;
    }

    return RT_ERR_OK;
}

