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
 * $Revision: 61228 $
 * $Date: 2016-03-16 14:49:14 +0800 (Thu, 16 Mar 2016) $
 *
 * Purpose : Definition of MAIO API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) MDIO
 *
 */
#include <dal/rtl9602c/dal_rtl9602c.h>
#include <rtk/mdio.h>
#include <rtk/gpio.h>
#include <osal/time.h>
#include <dal/rtl9602c/dal_rtl9602c_mdio.h>
#include <dal/rtl9602c/dal_rtl9602c_gpio.h>

static uint32 mdio_init = { INIT_NOT_COMPLETED } ;

static uint8 SET = 0;
static uint8 PORT = 0;
static uint8 PHYID = 0;
static rtk_mdio_format_t FMT = MDIO_FMT_C22;



#ifdef CONFIG_EXTERNAL_PHY_POLLING_USING_GPIO


#define RTL9602C_MII_CLOCK_GPIO_PIN (CONFIG_EXTERNAL_PHY_MII_CLOCK_GPIO_PIN)
#define RTL9602C_MII_DATA_GPIO_PIN  (CONFIG_EXTERNAL_PHY_MII_DATA_GPIO_PIN)



static void _rtl9602c_extMdcMdio_wait_half_CLK(void)
{
    int32 i;

    for(i = 0; i<= 10000; i++)
    {
        /* Dummy loop */
    }
}


static int32 _rtl9602c_extMdcMdio_setHigh(void)
{
    uint32 data;
    int32 ret;

    /* DIR=write, data=1, clock=0 */
    data = 0;
    if((ret = dal_rtl9602c_gpio_databit_set(RTL9602C_MII_CLOCK_GPIO_PIN, data)) != RT_ERR_OK)
        return ret;

    data = 1;
    if((ret = dal_rtl9602c_gpio_databit_set(RTL9602C_MII_DATA_GPIO_PIN, data)) != RT_ERR_OK)
        return ret;

    _rtl9602c_extMdcMdio_wait_half_CLK();

    /* DIR=write, data=1, clock=1 */
    data = 1;
    if((ret = dal_rtl9602c_gpio_databit_set(RTL9602C_MII_CLOCK_GPIO_PIN, data)) != RT_ERR_OK)
        return ret;

    data = 1;
    if((ret = dal_rtl9602c_gpio_databit_set(RTL9602C_MII_DATA_GPIO_PIN, data)) != RT_ERR_OK)
        return ret;

    _rtl9602c_extMdcMdio_wait_half_CLK();
    return RT_ERR_OK;
}

static int32 _rtl9602c_extMdcMdio_setLow(void)
{
    uint32 data;
    int32 ret;

    /* DIR=write, data=0, clock=0 */
    data = 0;
    if((ret = dal_rtl9602c_gpio_databit_set(RTL9602C_MII_CLOCK_GPIO_PIN, data)) != RT_ERR_OK)
        return ret;

    data = 0;
    if((ret = dal_rtl9602c_gpio_databit_set(RTL9602C_MII_DATA_GPIO_PIN, data)) != RT_ERR_OK)
        return ret;

    _rtl9602c_extMdcMdio_wait_half_CLK();

    /* DIR=write, data=0, clock=1 */
    data = 1;
    if((ret = dal_rtl9602c_gpio_databit_set(RTL9602C_MII_CLOCK_GPIO_PIN, data)) != RT_ERR_OK)
        return ret;

    data = 0;
    if((ret = dal_rtl9602c_gpio_databit_set(RTL9602C_MII_DATA_GPIO_PIN, data)) != RT_ERR_OK)
        return ret;

    _rtl9602c_extMdcMdio_wait_half_CLK();
    return RT_ERR_OK;
}

static int32 _rtl9602c_extMdcMdio_writeBit(uint8 data)
{
    int32 ret;

    if(data == 0)
    {
        if((ret = _rtl9602c_extMdcMdio_setLow()) != RT_ERR_OK)
            return ret;
    }
    else
    {
        if((ret = _rtl9602c_extMdcMdio_setHigh()) != RT_ERR_OK)
            return ret;
    }

    return RT_ERR_OK;
}

static int32 _rtl9602c_extMdcMdio_readBit(uint8 *pData)
{
    uint32 bit;
    uint32 data;
    int32 ret;

    /* Clock low & wait half clock */
    data = 0;
    if((ret = dal_rtl9602c_gpio_databit_set(RTL9602C_MII_CLOCK_GPIO_PIN, data)) != RT_ERR_OK)
        return ret;
    _rtl9602c_extMdcMdio_wait_half_CLK();

    /* Read data bit */
    if((ret = dal_rtl9602c_gpio_databit_get(RTL9602C_MII_DATA_GPIO_PIN, &bit)) != RT_ERR_OK)
        return ret;

    /* clock high & wait half clock */
    data = 1;
    if((ret = dal_rtl9602c_gpio_databit_set(RTL9602C_MII_CLOCK_GPIO_PIN, data)) != RT_ERR_OK)
       return ret;
    _rtl9602c_extMdcMdio_wait_half_CLK();

    *pData = bit;
    return RT_ERR_OK;
}



/* Function Name:
 *      rtl9602c_extPhy_read
 * Description:
 *      Get external PHY registers .
 * Input:
 *      phyID      - PHY id
 *      page       - page number
 *      phyRegAddr - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
*/
int32
rtl9602c_extPhy_read(
    uint32      phyID,
    uint32      phyRegAddr,
    uint16      *pData)
{
    int i;
    uint16 data;
    uint8 bit;
    uint32 reg_value;
    int32 ret;

    /* 32 bits preamble */
    for (i = 0; i < 32; i++)
    {
        if((ret = _rtl9602c_extMdcMdio_writeBit(1)) != RT_ERR_OK)
            return ret;
    }

    /* Start of command 0->1 */
    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit(1)) != RT_ERR_OK)
        return ret;

    /* Writing OP code 1->0 */
    if((ret = _rtl9602c_extMdcMdio_writeBit(1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    /* output PHY address */
    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyID & 0x10) >> 4)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyID & 0x08) >> 3)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyID & 0x04) >> 2)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyID & 0x02) >> 1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyID & 0x01))) != RT_ERR_OK)
        return ret;

    /* output register address */
    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x10) >> 4)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x08) >> 3)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x04) >> 2)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x02) >> 1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x01))) != RT_ERR_OK)
        return ret;

    /* Turn around */
    if((ret = _rtl9602c_extMdcMdio_writeBit(1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    /* Change Data pin to INPUT mode */
    reg_value = 0;
    if((ret = dal_rtl9602c_gpio_mode_set(RTL9602C_MII_DATA_GPIO_PIN, GPIO_INPUT)) != RT_ERR_OK)
        return ret;

    /* Read 16 bit data */
    data = 0;
    for(i = 0; i <= 15; i++)
    {
        data = data << 1;
        if((ret = _rtl9602c_extMdcMdio_readBit(&bit)) != RT_ERR_OK)
            return ret;
        data = data | bit;
    }

    /* Change Data pin to OUTPUT mode */
    reg_value = 1;
    if((ret = dal_rtl9602c_gpio_mode_set(RTL9602C_MII_DATA_GPIO_PIN, GPIO_OUTPUT)) != RT_ERR_OK)
        return ret;
    *pData = data;

    return RT_ERR_OK;


}

/* Function Name:
 *      rtl9602c_extPhy_write
 * Description:
 *      Set external PHY registers.
 * Input:
 *      phyID      - PHY id
 *      page       - page number
 *      phyRegAddr - PHY register
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
*/

int32
rtl9602c_extPhy_write(
    uint32      phyID,
    uint32      phyRegAddr,
    uint16      data)
{
    int i;
    uint32 bit;
    int32 ret;

    /* 32 bits preamble */
    for (i = 0; i < 32; i++)
    {
        if((ret = _rtl9602c_extMdcMdio_writeBit(1)) != RT_ERR_OK)
            return ret;
    }

    /* Start of command 0->1 */
    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit(1)) != RT_ERR_OK)
        return ret;

    /* Writing OP code 0->1 */
    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit(1)) != RT_ERR_OK)
        return ret;

    /* output PHY address */
    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyID & 0x10) >> 4)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyID & 0x08) >> 3)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyID & 0x04) >> 2)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyID & 0x02) >> 1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyID & 0x01))) != RT_ERR_OK)
        return ret;

    /* output register address */
    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x10) >> 4)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x08) >> 3)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x04) >> 2)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x02) >> 1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x01))) != RT_ERR_OK)
        return ret;

    /* Turn around */
    if((ret = _rtl9602c_extMdcMdio_writeBit(1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    /* output data-16bit */
    for(i = 15; i >= 0; i--)
    {
        bit = ((data>>i) & 0x1) > 0 ? 1 : 0;
        if((ret = _rtl9602c_extMdcMdio_writeBit((uint8)bit)) != RT_ERR_OK)
            return ret;
    }

    return RT_ERR_OK;

}





/* Function Name:
 *      rtl9602c_c45Phy_read
 * Description:
 *      Get c45 PHY registers .
 * Input:
 *      phyID      - PHY id
 *      page       - page number
 *      phyRegAddr - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
*/
int32
rtl9602c_c45Phy_read(
    uint8       dev,
    uint32      phyRegAddr,
    uint16      *pData)
{
    int i;
    uint16 data;
    uint8 bit;
    int32 ret;

    /*step write address*/
    /* 32 bits preamble */
    for (i = 0; i < 32; i++)
    {
        if((ret = _rtl9602c_extMdcMdio_writeBit(1)) != RT_ERR_OK)
            return ret;
    }

    /* Start of command 0->0 for C45*/
    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    /* Writing OP code read command 0->0 */
    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    /* output PRTAD[4:0] phy address*/
    /* output PHY address[4:0] */
    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x10) >> 4)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x08) >> 3)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x04) >> 2)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x02) >> 1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x01))) != RT_ERR_OK)
        return ret;


    /* output register addresds DEVAD[4:0] */
    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x10) >> 4)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x08) >> 3)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x04) >> 2)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x02) >> 1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x01))) != RT_ERR_OK)
        return ret;


    /* Turn around */
    if((ret = _rtl9602c_extMdcMdio_writeBit(1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    /* output register address[15:0] */
    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x8000) >> 15)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x4000) >> 14)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x2000) >> 13)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x1000) >> 12)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x800) >> 11)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x400) >> 10)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x200) >> 9)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x100) >> 8)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x80) >> 7)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x40) >> 6)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x20) >> 5)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x10) >> 4)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x08) >> 3)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x04) >> 2)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x02) >> 1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x01))) != RT_ERR_OK)
        return ret;






    /*step 2 read*/    
    /* 32 bits preamble */
    for (i = 0; i < 32; i++)
    {
        if((ret = _rtl9602c_extMdcMdio_writeBit(1)) != RT_ERR_OK)
            return ret;
    }

    /* Start of command 0->0 for C45*/
    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    /* Writing OP code read command 1->0 */
    if((ret = _rtl9602c_extMdcMdio_writeBit(1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    /* output PRTAD[4:0] phy address*/
    /* output PHY address[4:0] */
    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x10) >> 4)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x08) >> 3)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x04) >> 2)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x02) >> 1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x01))) != RT_ERR_OK)
        return ret;

    /* output register addresds DEVAD[4:0] */
    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x10) >> 4)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x08) >> 3)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x04) >> 2)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x02) >> 1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x01))) != RT_ERR_OK)
        return ret;


    /* Turn around */
    if((ret = _rtl9602c_extMdcMdio_writeBit(1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    /* Change Data pin to INPUT mode */
    if((ret = dal_rtl9602c_gpio_mode_set(RTL9602C_MII_DATA_GPIO_PIN,GPIO_INPUT)) != RT_ERR_OK)
        return ret;

    /* Read 16 bit data */
    data = 0;
    for(i = 0; i <= 15; i++)
    {
        data = data << 1;
        if((ret = _rtl9602c_extMdcMdio_readBit(&bit)) != RT_ERR_OK)
            return ret;
        data = data | bit;
    }

    /* Change Data pin to OUTPUT mode */
    if((ret = dal_rtl9602c_gpio_mode_set(RTL9602C_MII_DATA_GPIO_PIN, GPIO_OUTPUT)) != RT_ERR_OK)
        return ret;

    *pData = data;

    return RT_ERR_OK;


}


/* Function Name:
 *      rtl9602c_c45Phy_write
 * Description:
 *      Set c45 PHY registers .
 * Input:
 *      dev        - PHY id
 *      phyRegAddr - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
*/
int32
rtl9602c_c45Phy_write(
    uint8       dev,
    uint32      phyRegAddr,
    uint16      data)
{
    int i;
    int32 ret;

    /*step 1 write address*/
    /* 32 bits preamble */
    for (i = 0; i < 32; i++)
    {
        if((ret = _rtl9602c_extMdcMdio_writeBit(1)) != RT_ERR_OK)
            return ret;
    }

    /* Start of command 0->0 for C45*/
    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    /* Writing OP code read command 0->0 */
    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    /* output PRTAD[4:0] phy address*/
    /* output PHY address[4:0] */
    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x10) >> 4)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x08) >> 3)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x04) >> 2)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x02) >> 1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x01))) != RT_ERR_OK)
        return ret;





    /* output register addresds DEVAD[4:0] */
    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x10) >> 4)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x08) >> 3)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x04) >> 2)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x02) >> 1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x01))) != RT_ERR_OK)
        return ret;


    /* Turn around */
    if((ret = _rtl9602c_extMdcMdio_writeBit(1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    /* output register address[15:0] */
    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x8000) >> 15)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x4000) >> 14)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x2000) >> 13)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x1000) >> 12)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x800) >> 11)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x400) >> 10)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x200) >> 9)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x100) >> 8)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x80) >> 7)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x40) >> 6)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x20) >> 5)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x10) >> 4)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x08) >> 3)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x04) >> 2)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x02) >> 1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (phyRegAddr & 0x01))) != RT_ERR_OK)
        return ret;






    /*step 2 wrtie*/    
    /* 32 bits preamble */
    for (i = 0; i < 32; i++)
    {
        if((ret = _rtl9602c_extMdcMdio_writeBit(1)) != RT_ERR_OK)
            return ret;
    }

    /* Start of command 0->0 for C45*/
    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    /* Writing OP code wrtie command 0->1 */
    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit(1)) != RT_ERR_OK)
        return ret;

    /* output PRTAD[4:0] phy address*/
    /* output PHY address[4:0] */
    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x10) >> 4)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x08) >> 3)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x04) >> 2)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x02) >> 1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (PHYID & 0x01))) != RT_ERR_OK)
        return ret;





    /* output register addresds DEVAD[4:0] */
    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x10) >> 4)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x08) >> 3)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x04) >> 2)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x02) >> 1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (dev & 0x01))) != RT_ERR_OK)
        return ret;


    /* Turn around */
    if((ret = _rtl9602c_extMdcMdio_writeBit(1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit(0)) != RT_ERR_OK)
        return ret;


    /* output write date[15:0] */
    if((ret = _rtl9602c_extMdcMdio_writeBit( (data & 0x8000) >> 15)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (data & 0x4000) >> 14)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (data & 0x2000) >> 13)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (data & 0x1000) >> 12)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (data & 0x800) >> 11)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (data & 0x400) >> 10)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (data & 0x200) >> 9)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (data & 0x100) >> 8)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (data & 0x80) >> 7)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (data & 0x40) >> 6)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (data & 0x20) >> 5)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (data & 0x10) >> 4)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (data & 0x08) >> 3)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (data & 0x04) >> 2)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (data & 0x02) >> 1)) != RT_ERR_OK)
        return ret;

    if((ret = _rtl9602c_extMdcMdio_writeBit( (data & 0x01))) != RT_ERR_OK)
        return ret;




    return RT_ERR_OK;


}


#endif



/* Function Name:
 *      dal_rtl9602c_mdio_init
 * Description:
 *      Init MDIO configuration
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Init MDIO configuration
 */
int32
dal_rtl9602c_mdio_init(void)
{
    int32 ret;
#ifdef CONFIG_EXTERNAL_PHY_POLLING_USING_GPIO
    uint32 data;
#endif    
    ret=RT_ERR_OK;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MDIO), "");

    /* function body */
    mdio_init=INIT_COMPLETED;

#ifdef CONFIG_EXTERNAL_PHY_POLLING_USING_GPIO
    /* Enable GPIO */
    data = 1;
    if((ret = reg_array_field_write(RTL9602C_IO_GPIO_ENr, REG_ARRAY_INDEX_NONE, RTL9602C_MII_CLOCK_GPIO_PIN, RTL9602C_IO_GPIO_ENf, &data)) != RT_ERR_OK)
        return ret;

    data = 1;
    if((ret = reg_array_field_write(RTL9602C_IO_GPIO_ENr, REG_ARRAY_INDEX_NONE, RTL9602C_MII_DATA_GPIO_PIN, RTL9602C_IO_GPIO_ENf, &data)) != RT_ERR_OK)
        return ret;

    /* configure GPIO to OUTPUT mode */
    data = 1;
    if((ret = dal_rtl9602c_gpio_mode_set(RTL9602C_MII_CLOCK_GPIO_PIN, GPIO_OUTPUT)) != RT_ERR_OK)
        return ret;

    data = 1;
    if((ret = dal_rtl9602c_gpio_mode_set(RTL9602C_MII_DATA_GPIO_PIN, GPIO_OUTPUT)) != RT_ERR_OK)
        return ret;
#else
    ret=RT_ERR_OK;
#endif

    RT_INIT_CHK(mdio_init);

    return ret;
}   /* end of dal_rtl9602c_mdio_init */

/* Function Name:
 *      dal_rtl9602c_mdio_cfg_set
 * Description:
 *      Set MDIO configuration
 * Input:
 *      set   - Select which set of MDIO
 *      port  - Select shich virtual port to access MDIO
 *      phyid - IEEE 802.3
 *      fmt   - MDIO format to access
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must set MDIO configuration before MDIO read/write
 */
int32
dal_rtl9602c_mdio_cfg_set(uint8 set,uint8 port,uint8 phyid,rtk_mdio_format_t fmt)
{

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MDIO), "set=%d,port=%d,phyid=%d,fmt=%d",set, port, phyid, fmt);

    /* parameter check */
    RT_PARAM_CHK((1 <=set), RT_ERR_INPUT);
    RT_PARAM_CHK((32 <=port), RT_ERR_INPUT);
    RT_PARAM_CHK((32 <=phyid), RT_ERR_INPUT);
    RT_PARAM_CHK((MDIO_FMT_END <=fmt), RT_ERR_INPUT);

    RT_INIT_CHK(mdio_init);

    /* function body */
    if(fmt==MDIO_FMT_C22) //clause 22
    {

    }
    else //clause 45
    {
    }

   

    SET = set;
    PORT = port;
    PHYID = phyid;
    FMT = fmt;

    return RT_ERR_OK;
}   /* end of dal_rtl9602c_mdio_cfg_set */

/* Function Name:
 *      dal_rtl9602c_mdio_cfg_get
 * Description:
 *      Get MDIO configuration
 * Input:
 *      None
 * Output:
 *      set   - Select which set of MDIO
 *      port  - Select shich virtual port to access MDIO
 *      phyid - clause 22 of IEEE 802.3 define
 *      fmt   - MDIO format to access
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must check MDIO configuration before MDIO read/write
 */
int32
dal_rtl9602c_mdio_cfg_get(uint8 *set,uint8 *port,uint8 *phyid,rtk_mdio_format_t *fmt)
{
    /* parameter check */
    RT_PARAM_CHK((NULL == set), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == port), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == phyid), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == fmt), RT_ERR_NULL_POINTER);

    /* function body */
    RT_INIT_CHK(mdio_init);

    *set = SET;
    *port = PORT;
    *phyid = PHYID; 
    *fmt = FMT;

    return RT_ERR_OK;
}   /* end of dal_rtl9602c_mdio_cfg_get */

/* Function Name:
 *      dal_rtl9602c_mdio_c22_write
 * Description:
 *      Write MDIO by clause 22
 * Input:
 *      reg   - clause 22 of IEEE 802.3 define
 *      data  - clause 22 of IEEE 802.3 define
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      
 */
int32
dal_rtl9602c_mdio_c22_write(uint8 c22_reg,uint16 data)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MDIO), "c22_reg=%d,data=%d",c22_reg, data);

    /* parameter check */
    RT_PARAM_CHK((32 <=c22_reg), RT_ERR_INPUT);

    RT_INIT_CHK(mdio_init);


#ifdef CONFIG_EXTERNAL_PHY_POLLING_USING_GPIO
    /* function body */
    if ((ret = rtl9602c_extPhy_write(PHYID, c22_reg, data)) != RT_ERR_OK)
        return ret;
#else
    ret=RT_ERR_OK;
    return ret;
#endif

    return RT_ERR_OK;
}   /* end of dal_rtl9602c_mdio_c22_write */

/* Function Name:
 *      dal_rtl9602c_mdio_c22_read
 * Description:
 *      Read MDIO by clause 22
 * Input:
 *      reg   - clause 22 of IEEE 802.3 define
 * Output:
 *      data  - clause 22 of IEEE 802.3 define
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      
 */
int32
dal_rtl9602c_mdio_c22_read(uint8 c22_reg,uint16 *data)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MDIO), "c22_reg=%d",c22_reg);

    /* parameter check */
    RT_PARAM_CHK((32 <=c22_reg), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == data), RT_ERR_NULL_POINTER);

    RT_INIT_CHK(mdio_init);


#ifdef CONFIG_EXTERNAL_PHY_POLLING_USING_GPIO
    /* function body */
    if ((ret = rtl9602c_extPhy_read(PHYID, c22_reg, data)) != RT_ERR_OK)
        return ret;
#else
    ret=RT_ERR_OK;
    return ret;
#endif

    return RT_ERR_OK;
}   /* end of dal_rtl9602c_mdio_c22_read */

/* Function Name:
 *      dal_rtl9602c_mdio_c45_write
 * Description:
 *      Write MDIO by clause 45 
 * Input:
 *      dev   - clause 45 of IEEE 802.3 define
 *      reg   - clause 45 of IEEE 802.3 define
 *      data  - clause 45 of IEEE 802.3 define
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      
 */
int32
dal_rtl9602c_mdio_c45_write(uint8 dev,uint16 c45_reg,uint16 data)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MDIO), "dev=%d,c45_reg=%d,data=%d",dev, c45_reg, data);

    /* parameter check */
    RT_PARAM_CHK((32 <=dev), RT_ERR_INPUT);

    RT_INIT_CHK(mdio_init);

    /* function body */
#ifdef CONFIG_EXTERNAL_PHY_POLLING_USING_GPIO
    /* function body */
    if ((ret = rtl9602c_c45Phy_write(dev,c45_reg, data)) != RT_ERR_OK)
        return ret;
#else
    ret=RT_ERR_OK;
    return ret;
#endif
    return RT_ERR_OK;
}   /* end of dal_rtl9602c_mdio_c45_write */

/* Function Name:
 *      dal_rtl9602c_mdio_c45_write
 * Description:
 *      Read MDIO by clause 45 
 * Input:
 *      dev   - clause 45 of IEEE 802.3 define
 *      reg   - clause 45 of IEEE 802.3 define
 * Output:
 *      data  - clause 45 of IEEE 802.3 define
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      
 */
int32
dal_rtl9602c_mdio_c45_read(uint8 dev,uint16 c45_reg,uint16 *data)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MDIO), "dev=%d,c45_reg=%d",dev, c45_reg);

    /* parameter check */
    RT_PARAM_CHK((32 <=dev), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == data), RT_ERR_NULL_POINTER);

    RT_INIT_CHK(mdio_init);

    /* function body */
 
#ifdef CONFIG_EXTERNAL_PHY_POLLING_USING_GPIO
    /* function body */
    if ((ret = rtl9602c_c45Phy_read(dev, c45_reg, data)) != RT_ERR_OK)
        return ret;
#else
    ret=RT_ERR_OK;
    return ret;
#endif



    return RT_ERR_OK;
}   /* end of dal_rtl9602c_mdio_c45_read */
 
