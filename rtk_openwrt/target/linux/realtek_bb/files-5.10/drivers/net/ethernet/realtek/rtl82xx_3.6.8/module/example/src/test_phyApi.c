

//#include <common/debug/rt_log.h>
#include <hal/phy/phydef.h>
#include <hal/common/miim.h>
#include <phy/inc/phy_init.h>
#include <osal/lib.h>
#include <osal/print.h>


/*
 * This table is used to describe your hardware board design, especially for mapping relation between port and phy.
 * Port related information
 * .mac_id      = port id.
 * .phy_idex    = used to indicate which PHY entry is used by this port in glued_phy_Descp[].
 * .eth         = Ethernet speed type (refer to rt_port_ethType_t).
 * .medi        = Port media type (refer to rt_port_medium_t).
 */
phy_hwp_portDescp_t  my_port_descp[] = {
        { .mac_id =  0,  .phy_idx = 0, .eth = HWP_XGE,   .medi = HWP_COPPER,},
        { .mac_id =  1,  .phy_idx = 0, .eth = HWP_XGE,   .medi = HWP_COPPER,},
        { .mac_id =  2,  .phy_idx = 0, .eth = HWP_XGE,   .medi = HWP_COPPER,},
        { .mac_id =  3,  .phy_idx = 0, .eth = HWP_XGE,   .medi = HWP_COPPER,},
        { .mac_id =  4,  .phy_idx = 1, .eth = HWP_XGE,   .medi = HWP_COPPER,},
        { .mac_id =  5,  .phy_idx = 1, .eth = HWP_XGE,   .medi = HWP_COPPER,},
        { .mac_id =  6,  .phy_idx = 1, .eth = HWP_XGE,   .medi = HWP_COPPER,},
        { .mac_id =  7,  .phy_idx = 1, .eth = HWP_XGE,   .medi = HWP_COPPER,},
        { .mac_id =  8,  .phy_idx = 2, .eth = HWP_XGE,   .medi = HWP_COPPER,},
        { .mac_id =  9,  .phy_idx = 3, .eth = HWP_XGE,   .medi = HWP_COPPER,},
        { .mac_id = HWP_END },
    };

/*
 * PHY related information
 * .chip        = PHY Chip model (refer to phy_type_t).
 * .mac_id      = The first port id of this PHY. For example, the 8218D is connected to
 *                port 0 ~ 7, then the .mac_id  = 0.
 * .phy_max     = The MAX port number of this PHY. For examplem the 8218D is an octet PHY,
 *                so this number is 8.
 */

phy_hwp_phyDescp_t     my_phy_Descp[] = {
        [0] = { .chip = RTK_PHYTYPE_RTL8264, .mac_id = 0, .phy_max = 4 },
        [1] = { .chip = RTK_PHYTYPE_RTL8264, .mac_id = 4, .phy_max = 4 },
        [2] = { .chip = RTK_PHYTYPE_RTL8261, .mac_id = 8, .phy_max = 1 },
        [3] = { .chip = RTK_PHYTYPE_RTL8261, .mac_id = 9, .phy_max = 1 },
        [4] = { .chip = HWP_END },
    };




void all_rtk_phy_api(void)
{
    rtk_port_phyTestMode_t      testMode;
    rtk_port_phyPolarCtrl_t     polarCtrl;
    osal_memset(&testMode, 0, sizeof(testMode));
    osal_memset(&polarCtrl, 0, sizeof(polarCtrl));

    rtk_port_linkMedia_get(0, 8, NULL, NULL);
    rtk_port_speedDuplex_get( 0, 8, NULL, NULL);
    rtk_port_adminEnable_set(0, 8, ENABLED);
    rtk_port_phyAutoNegoEnable_get(0, 8, NULL);
    rtk_port_phyAutoNegoEnable_set(0, 8, ENABLED);
    rtk_port_phyAutoNegoAbilityLocal_get(0, 8, NULL);
    rtk_port_phyAutoNegoAbility_get( 0, 8, NULL);
    rtk_port_phyAutoNegoAbility_set( 0, 8, NULL);
    rtk_port_phyForceModeAbility_get( 0, 8, NULL, NULL, NULL);
    rtk_port_phyForceModeAbility_set( 0, 8, PORT_SPEED_10G, PORT_FULL_DUPLEX, ENABLED);
    rtk_port_phyForceFlowctrlMode_get( 0, 8, NULL);
    rtk_port_phyForceFlowctrlMode_set( 0, 8, NULL);
    rtk_port_phyMasterSlave_get( 0, 8, NULL, NULL);
    rtk_port_phyMasterSlave_set( 0, 8, PORT_AUTO_MODE);
    rtk_port_phyReg_get( 0, 8, 0x1234, 31, NULL);
    rtk_port_phyReg_set( 0, 8, 0x1234, 31, 0x3260);
    rtk_port_phyExtParkPageReg_get( 0, 8, 0x4321, 0x3820, 0, 31, NULL);
    rtk_port_phyExtParkPageReg_set( 0, 8, 0x4321, 0x3820, 0, 31, 0x3260);
    rtk_port_phymaskExtParkPageReg_set( 0, NULL, 0x4321, 0x3820, 0, 31, 0x3260);
    rtk_port_phyMmdReg_get( 0, 8, 7, 3311, NULL);
    rtk_port_phyMmdReg_set( 0, 8, 7, 3311, 0x3260);
    rtk_port_phymaskMmdReg_set( 0, NULL, 7, 3311, 0x3260);
    rtk_port_phyComboPortMedia_get(0, 8, NULL);
    rtk_port_phyComboPortMedia_set(0, 8, PORT_MEDIA_COPPER);
    rtk_port_greenEnable_get(0, 8, NULL);
    rtk_port_greenEnable_set(0, 8, ENABLED);
    rtk_port_phyCrossOverMode_get(0, 8, NULL);
    rtk_port_phyCrossOverMode_set(0, 8, PORT_CROSSOVER_MODE_AUTO);
    rtk_port_phyCrossOverStatus_get(0, 8, NULL);
    rtk_port_phyComboPortFiberMedia_get(0, 8, NULL);
    rtk_port_phyComboPortFiberMedia_set(0, 8, PORT_FIBER_MEDIA_1000);
    rtk_port_linkDownPowerSavingEnable_get(0, 8, NULL);
    rtk_port_linkDownPowerSavingEnable_set(0, 8, ENABLED);
    rtk_port_gigaLiteEnable_get(0, 8, NULL);
    rtk_port_gigaLiteEnable_set(0, 8, ENABLED);
    rtk_port_2pt5gLiteEnable_get(0, 8, NULL);
    rtk_port_2pt5gLiteEnable_set(0, 8, ENABLED);
    rtk_port_phyReconfig_register(0, NULL);
    rtk_port_phyReconfig_unregister(0);
    rtk_port_downSpeedEnable_get(0, 8, NULL);
    rtk_port_downSpeedEnable_set(0, 8, ENABLED);
    rtk_port_downSpeedStatus_get(0, 8, NULL);
    rtk_port_fiberNwayForceLinkEnable_get(0, 8, NULL);
    rtk_port_fiberNwayForceLinkEnable_set(0, 8, ENABLED);
    rtk_port_phyLoopBackEnable_get(0, 8, NULL);
    rtk_port_phyLoopBackEnable_set(0, 8, ENABLED);
    rtk_port_10gMedia_set(0, 8, PORT_10GMEDIA_NONE);
    rtk_port_10gMedia_get(0, 8, NULL);
    rtk_port_phyFiberTxDis_set(0, 8, ENABLED);
    rtk_port_phyFiberTxDisPin_set(0, 8, 0x3260);
    rtk_port_fiberRxEnable_get(0, 8, NULL);
    rtk_port_fiberRxEnable_set(0, 8, ENABLED);
    rtk_port_phyIeeeTestMode_set(0, 8, &testMode);
    rtk_port_phyPolar_get(0, 8, NULL);
    rtk_port_phyPolar_set(0, 8, &polarCtrl);
    rtk_port_phyEyeMonitor_start(0, 8, 12, 6);
    rtk_port_phyEyeMonitorInfo_get(0, 8, 7, 6, NULL);
    rtk_port_imageFlash_load(0, 8, 1024, NULL);
    rtk_port_phySdsRxCaliStatus_get(0, 8, 12, NULL);
    rtk_port_phyReset_set(0, 8);
    rtk_port_phyLinkStatus_get(0, 8, NULL);
    rtk_port_phyPeerAutoNegoAbility_get(0, 8, NULL);
    rtk_port_phyMacIntfSerdesMode_get(0, 8, NULL);
    rtk_port_phyLedMode_set(0, 8, NULL);
    rtk_port_phyLedCtrl_get(0, 8, NULL);
    rtk_port_phyLedCtrl_set(0, 8, NULL);
    rtk_port_phyMacIntfSerdesLinkStatus_get(0, 8, NULL);
    rtk_port_phySdsEyeParam_get(0, 8, 12, NULL);
    rtk_port_phySdsEyeParam_set(0, 8, 12, NULL);
    rtk_port_phyMdiLoopbackEnable_get(0, 8, NULL);
    rtk_port_phyMdiLoopbackEnable_set(0, 8, ENABLED);
    rtk_port_phyIntr_init(0, 8, RTK_PHY_INTR_COMMON);
    rtk_port_phyIntrEnable_get(0, 8, RTK_PHY_INTR_STATUS_RLFD, NULL);
    rtk_port_phyIntrEnable_set(0, 8, RTK_PHY_INTR_STATUS_RLFD, ENABLED);
    rtk_port_phyIntrStatus_get(0, 8, RTK_PHY_INTR_COMMON, NULL);
    rtk_port_phyIntrMask_get(0, 8, RTK_PHY_INTR_COMMON, NULL);
    rtk_port_phyIntrMask_set(0, 8, RTK_PHY_INTR_COMMON, 0);
    rtk_port_phySdsTestMode_set(0, 8, 2, RTK_SDS_TESTMODE_PRBS11);
    rtk_port_phySdsTestModeCnt_get(0, 8, 2, NULL);
    rtk_port_phySdsLeq_get(0, 8, 12, NULL, NULL);
    rtk_port_phySdsLeq_set(0, 8, 12, ENABLED, 5678);
    rtk_port_phySds_get(0, 8, NULL);
    rtk_port_phySds_set(0, 8, NULL);
    rtk_port_phyCtrl_get(0, 8, RTK_PHY_CTRL_AN_COMPLETE, NULL);
    rtk_port_phyCtrl_set(0, 8, RTK_PHY_CTRL_AN_COMPLETE, 10);
    rtk_port_phyLiteEnable_get(0, 8, PORT_LITE_2P5G, NULL);
    rtk_port_phyLiteEnable_set(0, 8, PORT_LITE_2P5G, ENABLED);
    rtk_port_phyDbgCounter_get(0, 8, PHY_DBG_CNT_RX, NULL);
    rtk_eee_portEnable_get(0, 8, NULL);
    rtk_eee_portEnable_set(0, 8, ENABLED);
    rtk_diag_rtctEnable_set(0, NULL);
}

int call_all_phy_api(void)
{
    rtk_phy_initInfo_t initInfo;

    initInfo.port_desc = my_port_descp;
    initInfo.phy_desc = my_phy_Descp;
    rtk_init(&initInfo);
    all_rtk_phy_api();
    return 0;
}
