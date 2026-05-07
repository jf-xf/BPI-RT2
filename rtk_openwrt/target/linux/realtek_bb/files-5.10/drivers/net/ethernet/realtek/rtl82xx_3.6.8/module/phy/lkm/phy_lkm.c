/*
 * Copyright (C) 2009-2015 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 71708 $
 * $Date: 2016-09-19 11:31:17 +0800 (Mon, 19 Sep 2016) $
 *
 * Purpose : Create stacking linux kernel module
 *
 */

/*
 * Include Files
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>

#include <phy/inc/phy_init.h>
#include <rtk/phy.h>
#include <example/src/test_phyApi.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */
static int __init phy_lkm_init(void)
{
    printk("PHY lkm init\n");

    return 0;
}

static void __exit phy_lkm_exit(void)
{
    printk("Exit PHY lkm\n");
}

module_init(phy_lkm_init);
module_exit(phy_lkm_exit);

MODULE_DESCRIPTION("phy lkm");
MODULE_LICENSE("GPL");

EXPORT_SYMBOL(rtk_init);
EXPORT_SYMBOL(rtk_port_linkMedia_get);
EXPORT_SYMBOL(rtk_port_speedDuplex_get);
EXPORT_SYMBOL(rtk_port_adminEnable_set);
EXPORT_SYMBOL(rtk_port_phyAutoNegoEnable_get);
EXPORT_SYMBOL(rtk_port_phyAutoNegoEnable_set);
EXPORT_SYMBOL(rtk_port_phyAutoNegoAbilityLocal_get);
EXPORT_SYMBOL(rtk_port_phyAutoNegoAbility_get);
EXPORT_SYMBOL(rtk_port_phyAutoNegoAbility_set);
EXPORT_SYMBOL(rtk_port_phyForceModeAbility_get);
EXPORT_SYMBOL(rtk_port_phyForceModeAbility_set);
EXPORT_SYMBOL(rtk_port_phyForceFlowctrlMode_get);
EXPORT_SYMBOL(rtk_port_phyForceFlowctrlMode_set);
EXPORT_SYMBOL(rtk_port_phyMasterSlave_get);
EXPORT_SYMBOL(rtk_port_phyMasterSlave_set);
EXPORT_SYMBOL(rtk_port_phyReg_get);
EXPORT_SYMBOL(rtk_port_phyReg_set);
EXPORT_SYMBOL(rtk_port_phyExtParkPageReg_get);
EXPORT_SYMBOL(rtk_port_phyExtParkPageReg_set);
EXPORT_SYMBOL(rtk_port_phymaskExtParkPageReg_set);
EXPORT_SYMBOL(rtk_port_phyMmdReg_get);
EXPORT_SYMBOL(rtk_port_phyMmdReg_set);
EXPORT_SYMBOL(rtk_port_phymaskMmdReg_set);
EXPORT_SYMBOL(rtk_port_phyComboPortMedia_get);
EXPORT_SYMBOL(rtk_port_phyComboPortMedia_set);
EXPORT_SYMBOL(rtk_port_greenEnable_get);
EXPORT_SYMBOL(rtk_port_greenEnable_set);
EXPORT_SYMBOL(rtk_port_phyCrossOverMode_get);
EXPORT_SYMBOL(rtk_port_phyCrossOverMode_set);
EXPORT_SYMBOL(rtk_port_phyCrossOverStatus_get);
EXPORT_SYMBOL(rtk_port_phyComboPortFiberMedia_get);
EXPORT_SYMBOL(rtk_port_phyComboPortFiberMedia_set);
EXPORT_SYMBOL(rtk_port_linkDownPowerSavingEnable_get);
EXPORT_SYMBOL(rtk_port_linkDownPowerSavingEnable_set);
EXPORT_SYMBOL(rtk_port_gigaLiteEnable_get);
EXPORT_SYMBOL(rtk_port_gigaLiteEnable_set);
EXPORT_SYMBOL(rtk_port_2pt5gLiteEnable_get);
EXPORT_SYMBOL(rtk_port_2pt5gLiteEnable_set);
EXPORT_SYMBOL(rtk_port_phyReconfig_register);
EXPORT_SYMBOL(rtk_port_phyReconfig_unregister);
EXPORT_SYMBOL(rtk_port_downSpeedEnable_get);
EXPORT_SYMBOL(rtk_port_downSpeedEnable_set);
EXPORT_SYMBOL(rtk_port_downSpeedStatus_get);
EXPORT_SYMBOL(rtk_port_fiberNwayForceLinkEnable_get);
EXPORT_SYMBOL(rtk_port_fiberNwayForceLinkEnable_set);
EXPORT_SYMBOL(rtk_port_phyLoopBackEnable_get);
EXPORT_SYMBOL(rtk_port_phyLoopBackEnable_set);
EXPORT_SYMBOL(rtk_port_10gMedia_set);
EXPORT_SYMBOL(rtk_port_10gMedia_get);
EXPORT_SYMBOL(rtk_port_phyFiberTxDis_set);
EXPORT_SYMBOL(rtk_port_phyFiberTxDisPin_set);
EXPORT_SYMBOL(rtk_port_fiberRxEnable_get);
EXPORT_SYMBOL(rtk_port_fiberRxEnable_set);
EXPORT_SYMBOL(rtk_port_phyIeeeTestMode_set);
EXPORT_SYMBOL(rtk_port_phyPolar_get);
EXPORT_SYMBOL(rtk_port_phyPolar_set);
EXPORT_SYMBOL(rtk_port_phyEyeMonitor_start);
EXPORT_SYMBOL(rtk_port_phyEyeMonitorInfo_get);
EXPORT_SYMBOL(rtk_port_imageFlash_load);
EXPORT_SYMBOL(rtk_port_phySdsRxCaliStatus_get);
EXPORT_SYMBOL(rtk_port_phyReset_set);
EXPORT_SYMBOL(rtk_port_phyLinkStatus_get);
EXPORT_SYMBOL(rtk_port_phyPeerAutoNegoAbility_get);
EXPORT_SYMBOL(rtk_port_phyMacIntfSerdesMode_get);
EXPORT_SYMBOL(rtk_port_phyLedMode_set);
EXPORT_SYMBOL(rtk_port_phyLedCtrl_get);
EXPORT_SYMBOL(rtk_port_phyLedCtrl_set);
EXPORT_SYMBOL(rtk_port_phyMacIntfSerdesLinkStatus_get);
EXPORT_SYMBOL(rtk_port_phySdsEyeParam_get);
EXPORT_SYMBOL(rtk_port_phySdsEyeParam_set);
EXPORT_SYMBOL(rtk_port_phyMdiLoopbackEnable_get);
EXPORT_SYMBOL(rtk_port_phyMdiLoopbackEnable_set);
EXPORT_SYMBOL(rtk_port_phyIntr_init);
EXPORT_SYMBOL(rtk_port_phyIntrEnable_get);
EXPORT_SYMBOL(rtk_port_phyIntrEnable_set);
EXPORT_SYMBOL(rtk_port_phyIntrStatus_get);
EXPORT_SYMBOL(rtk_port_phyIntrMask_get);
EXPORT_SYMBOL(rtk_port_phyIntrMask_set);
EXPORT_SYMBOL(rtk_port_phySdsTestMode_set);
EXPORT_SYMBOL(rtk_port_phySdsTestModeCnt_get);
EXPORT_SYMBOL(rtk_port_phySdsLeq_get);
EXPORT_SYMBOL(rtk_port_phySdsLeq_set);
EXPORT_SYMBOL(rtk_port_phySds_get);
EXPORT_SYMBOL(rtk_port_phySds_set);
EXPORT_SYMBOL(rtk_port_phyCtrl_get);
EXPORT_SYMBOL(rtk_port_phyCtrl_set);
EXPORT_SYMBOL(rtk_port_phyLiteEnable_get);
EXPORT_SYMBOL(rtk_port_phyLiteEnable_set);
EXPORT_SYMBOL(rtk_port_phyDbgCounter_get);
EXPORT_SYMBOL(rtk_eee_portEnable_get);
EXPORT_SYMBOL(rtk_eee_portEnable_set);
EXPORT_SYMBOL(rtk_diag_rtctEnable_set);


