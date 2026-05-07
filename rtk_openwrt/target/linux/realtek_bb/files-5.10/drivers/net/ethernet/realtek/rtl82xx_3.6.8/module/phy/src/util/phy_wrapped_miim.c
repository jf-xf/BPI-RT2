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


#include <common/rt_type.h>
#include <common/rt_error.h>
#include <phy/inc/phy_hal.h>
#include <hal/common/halctrl.h>


#define PAGE_REG            0x1F
#define MAX_PAGE_NUM        HAL_MIIM_FIX_PAGE
#define NO_PARK_PAGE        0x1F

extern hal_control_t   phy_hal_ctrl[RTK_MAX_NUM_OF_UNIT];


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
int32
wrapped_miim_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      *pData)
{

    /* Set PHY to assign page */
    if(page != MAX_PAGE_NUM)
        phy_hal_mii_write(unit, port, PAGE_REG, page);

    /* Read register data back */
    phy_hal_mii_read(unit, port, phy_reg, pData);

    /* Set PHY to page 0 */
    phy_hal_mii_write(unit, port, PAGE_REG, 0);

    return RT_ERR_OK;
} /* end of wrapped_miim_read */


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
int32
wrapped_miim_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      data)
{
    /* Set PHY to assign page */
    if(page != MAX_PAGE_NUM)
        phy_hal_mii_write(unit, port, PAGE_REG, page);

    /* Read register data back */
    phy_hal_mii_write(unit, port, phy_reg, data);

    /* Set PHY to page 0 */
    phy_hal_mii_write(unit, port, PAGE_REG, 0);

    return RT_ERR_OK;
} /* end of wrapped_miim_write */

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
int32
wrapped_miim_park_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      *pData)
{
    /* Set PHY to assign page */
    if(page != MAX_PAGE_NUM)
        phy_hal_mii_write(unit, port, PAGE_REG, page);

    /* Read register data back */
    phy_hal_mii_read(unit, port, phy_reg, pData);

    /* Set PHY to page 0 */
    if(parkPage != NO_PARK_PAGE)
        phy_hal_mii_write(unit, port, PAGE_REG, parkPage);

    return RT_ERR_OK;
} /* end of wrapped_miim_park_read */


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
int32
wrapped_miim_park_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      data)
{
    /* Set PHY to assign page */
    if(page != MAX_PAGE_NUM)
        phy_hal_mii_write(unit, port, PAGE_REG, page);

    /* Read register data back */
    phy_hal_mii_write(unit, port, phy_reg, data);

    /* Set PHY to page 0 */
    if(parkPage != NO_PARK_PAGE)
        phy_hal_mii_write(unit, port, PAGE_REG, parkPage);

    return RT_ERR_OK;

} /* end of wrapped_miim_park_write */



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
int32
wrapped_mmd_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      *pData)
{
    phy_hal_mmd_read(unit, port, mmdAddr, mmdReg, pData);

    return RT_ERR_OK;

} /* end of wrapped_mmd_read */


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
int32
wrapped_mmd_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      data)
{
    phy_hal_mmd_write(unit, port, mmdAddr, mmdReg, data);
    return RT_ERR_OK;

} /* end of wrapped_mmd_write */

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
int32
wrapped_miim_pollingEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    *pEnabled)
{
    int32 ret;

    ret = phy_hal_miim_pollingEnable_get(unit, port, pEnabled);

    return ret;
} /* end of wrapped_miim_pollingEnable_get */

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
int32
wrapped_miim_pollingEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    enabled)
{
    int32 ret;

    ret = phy_hal_miim_pollingEnable_set(unit, port, enabled);

    return ret;
} /* end of wrapped_miim_pollingEnable_set */


hal_control_t *
hal_ctrlInfo_get(uint32 unit)
{
    if (unit >= RTK_MAX_NUM_OF_UNIT)
    {
        return NULL;
    }

    if (phy_hal_ctrl[unit].chip_flags & HAL_CHIP_ATTACHED)
    {
        return &phy_hal_ctrl[unit];
    }
    else
    {
        return NULL;
    }
}

/* The Fake API*/
/* ===================================================================== */
/* The following APIs are used for build PHY driver release package only */
/* ===================================================================== */
uint32
drv_swcore_chipFamilyId_get(uint32 chip_id)
{
    return 0x93100000;
} /* end of drv_swcore_cid_get */

/* Function Name:
 *      hal_mac_serdes_rst
 * Description:
 *      Reset MAC serdes.
 * Input:
 *      unit   - unit id
 *      sdsId   - serdes id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_OUT_OF_RANGE  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
hal_mac_serdes_rst(uint32 unit, uint32 sdsId)
{
    return RT_ERR_OK;
} /* end of hal_mac_serdes_rst */


