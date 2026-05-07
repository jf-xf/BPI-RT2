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

#ifndef __PHY_HAL_H__
#define __PHY_HAL_H__

#include <common/rt_type.h>
#include <common/rt_error.h>

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
extern int32
phy_hal_mii_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      phy_reg,
    uint32      *pData);

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
extern int32
phy_hal_mii_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      phy_reg,
    uint32      data);

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
extern int32
phy_hal_mmd_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      *pData);

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
extern int32
phy_hal_mmd_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      data);

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
extern int32
phy_hal_miim_pollingEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    *pEnabled);

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
extern int32
phy_hal_miim_pollingEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    enabled);


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
 *      RT_ERR_INPUT  - error smi id or proto
 * Note:
 *      None
 */
extern int32
phy_hal_miim_portSmiMdxProto_set(uint32 unit, rtk_port_t port, drv_smi_mdxProtoSel_t proto);

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
 *      RT_ERR_INPUT  - error smi id or proto
 * Note:
 *      None
 */
extern int32
phy_hal_miim_portSmiMdxProto_get(uint32 unit, rtk_port_t port, drv_smi_mdxProtoSel_t *pProto);


#endif /* __PHY_HAL_H__ */

