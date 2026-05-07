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
 * Purpose : PHY miim wrapped module.
 *
 * Feature : This Module is used to convert RTK PHY driver MIIM usage to customer's
 *           MIIM driver.
 *
 */

#ifndef __PHY_WRAPPED_MIIM_H__
#define __PHY_WRAPPED_MIIM_H__


/* Function Name:
 *      wrapped_miim_read
 * Description:
 *      Get PHY registers.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
wrapped_miim_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      *pData);

/* Function Name:
 *      wrapped_miim_write
 * Description:
 *      Set PHY registers.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 *      data    - write data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
wrapped_miim_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      data);

/* Function Name:
 *      wrapped_miim_park_read
 * Description:
 *      Get PHY registers.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      parkPage    - PHY park page
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
wrapped_miim_park_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      *pData);


/* Function Name:
 *      wrapped_miim_park_write
 * Description:
 *      Set PHY registers.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      parkPage    - PHY park page
 *      phy_reg - PHY register
 *      data    - write data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
wrapped_miim_park_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      data);

/* Function Name:
 *      wrapped_mmd_read
 * Description:
 *      Get PHY registers.
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
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
wrapped_mmd_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      *pData);

/* Function Name:
 *      wrapped_mmd_write
 * Description:
 *      Set PHY registers.
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
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
wrapped_mmd_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      data);

/* Function Name:
 *      wrapped_miim_pollingEnable_get
 * Description:
 *      Get the mac polling PHY status of the specified port.
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
wrapped_miim_pollingEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    *pEnabled);

/* Function Name:
 *      wrapped_miim_pollingEnable_set
 * Description:
 *      Set the mac polling PHY status of the specified port.
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
wrapped_miim_pollingEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    enabled);

#endif /* __PHY_WRAPPED_MIIM_H__ */
