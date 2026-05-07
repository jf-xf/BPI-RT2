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
 * Purpose : PHY HAL module.
 *
 * Feature : HAL is Hareware Abstraction Layer.
 *           The module is used to implement Hareware related APIs.
 *
 */

#include <common/rt_type.h>
#include <common/rt_error.h>
#include <phy/inc/phy_hal.h>
#include <hal/phy/phydef.h>

uint32                      hwp_useDefHwp = FALSE;



/* Function Name:
 *      phy_hal_mii_read
 * Description:
 *      Get PHY registers data by Clause 22.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. phy_reg valid range is 0 ~ 31
 */
int32
phy_hal_mii_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      phy_reg,
    uint32      *pData)
{

    return RT_ERR_OK;
} /* end of phy_hal_mii_read */

/* Function Name:
 *      phy_hal_mii_write
 * Description:
 *      Set PHY registers by Clause 22.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      phy_reg - PHY register
 *      data    - write data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. phy_reg valid range is 0 ~ 31
 */
int32
phy_hal_mii_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      phy_reg,
    uint32      data)
{

    return RT_ERR_OK;
} /* end of phy_hal_mii_write */

/* Function Name:
 *      phy_hal_mmd_read
 * Description:
 *      Get PHY registers by Clause 45.
 *      If the MDC/MDIO controller is Clause 22, the API shall implement Clause 22's register 13 and 14 to access MMD register of the PHY.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mmdAddr - mmd device address
 *      mmdReg  - mmd reg id
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 */
int32
phy_hal_mmd_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      *pData)
{
    return RT_ERR_OK;

} /* end of phy_hal_mmd_read */


/* Function Name:
 *      phy_hal_mmd_write
 * Description:
 *      Set PHY registers by Clause 45.
 *      If the MDC/MDIO controller is Clause 22, the API shall implement Clause 22's register 13 and 14 to access MMD register of the PHY.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mmdAddr - mmd device address
 *      mmdReg  - mmd reg id
 *      data    - write data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 */
int32
phy_hal_mmd_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      data)
{
    return RT_ERR_OK;

} /* end of phy_hal_mmd_write */


/* Function Name:
 *      phy_hal_miim_pollingEnable_get
 * Description:
 *      Get the mac polling PHY status of the specified port.
 *
 *      Realtek MAC chip will automatically access MDC/MDIO controller to get (polling) PHY's link, speed, ... status.
 *      This API is called by RTK PHY driver to disable the ability of hardware auto polling
 *      because for some PHY function control, it contains a series of PHY accesses and the accesses shall not be interrupted.
 *      After the series of PHY accesses is done, this API will be called again to recover the hardware auto polling.
 *
 *      Implement this API If your hardware MDC/MDIO controller also has ability of auto access PHY.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pEnabled - pointer buffer of mac polling PHY status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
phy_hal_miim_pollingEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    *pEnabled)
{
    return RT_ERR_OK;
} /* end of phy_hal_miim_pollingEnable_get */

/* Function Name:
 *      phy_hal_miim_pollingEnable_set
 * Description:
 *      Set the mac polling PHY status of the specified port.
 *
 *      Realtek MAC chip will automatically access MDC/MDIO controller to get (polling) PHY's link, speed, ... status.
 *      This API is called by RTK PHY driver to disable the ability of hardware auto polling
 *      because for some PHY function control, it contains a series of PHY accesses and the accesses shall not be interrupted.
 *      After the series of PHY accesses is done, this API will be called again to recover the hardware auto polling.
 *
 *      Implement this API If your hardware MDC/MDIO controller also has ability of auto access PHY.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enabled - mac polling PHY status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
phy_hal_miim_pollingEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    enabled)
{

    return RT_ERR_OK;
} /* end of phy_hal_miim_pollingEnable_set */




/* Function Name:
 *      phy_hal_miim_portSmiMdxProto_set
 * Description:
 *      Configure SMI MDC/MDIO protocol for the specified port's SMI interface
 * Input:
 *      unit - unit id
 *      port - port id
 *      proto  - protocol as Clause 22 or Clause 45
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED - chip not support the change of SMI protocol of the port
 * Note:
 *      None
 */
int32
phy_hal_miim_portSmiMdxProto_set(uint32 unit, rtk_port_t port, drv_smi_mdxProtoSel_t proto)
{
    return RT_ERR_CHIP_NOT_SUPPORTED;
}

/* Function Name:
 *      phy_hal_miim_portSmiMdxProto_get
 * Description:
 *      Get SMI MDC/MDIO protocol for the specified port's SMI interface
 * Input:
 *      unit - unit id
 *      port - port id
 *      proto  - protocol as Clause 22 or Clause 45
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_hal_miim_portSmiMdxProto_get(uint32 unit, rtk_port_t port, drv_smi_mdxProtoSel_t *pProto)
{
    *pProto = DRV_SMI_MDX_PROTO_C22;
    return RT_ERR_OK;
}



/* Function Name:
 *      ioal_phy_reset
 * Description:
 *      Reset PHY chips through hardware pin
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   Reset success
 *      RT_ERR_CHIP_NOT_SUPPORTED   Not support hardware reset PHY
 * Note:
 *    None
 */
int32
ioal_phy_reset(uint32 unit)
{
    return RT_ERR_CHIP_NOT_SUPPORTED;
}


/* Function Name:
 *      ioal_param_phyXfmrRj45Impd_get
 * Description:
 *      impedance between transformer and RJ45 a.k.a copper MDI differential pair impedance for CS patch.
 *      This API will be called by RTL8218D chip initial flow.
 *      The initial value of some registers will be different according to the different impedance level on the board.
 *
 *      Consult your HW what enum in rtk_phy_xfmrRj45Impd_t shall be used for your board.
 *      Different board may have to return differnt value in this API.
 * Input:
 *      unit        - unit id
 * Output:
 *      pImpd - enum value of impedance level.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *    Apply for PHY RTL8218D
 */
int32
ioal_param_phyXfmrRj45Impd_get(uint32 unit, rtk_phy_xfmrRj45Impd_t *pImpd)
{

    *pImpd = RTK_PHY_XFMRRJ45IMPD_95_OHM;

    return RT_ERR_OK;
}


/* Function Name:
 *      ioal_param_8218dUseCsPatch_get
 * Description:
 *      For RTL8218D PHY chip to use CS patch or not.
 * Input:
 *      unit        - unit id
 * Output:
 *      pUse_cs_patch - 1: use CS patch; 0: use non-CS patch
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *    Apply for PHY RTL8218D
 */
int32
ioal_param_8218dUseCsPatch_get(uint32 *pUse_cs_patch)
{

    *pUse_cs_patch = 0;
    return RT_ERR_OK;
}


