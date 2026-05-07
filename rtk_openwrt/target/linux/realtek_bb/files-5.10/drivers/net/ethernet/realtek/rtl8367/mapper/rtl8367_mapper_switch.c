

#include <rtl8367_mapper_switch.h>

#include <osal/memory.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_bitop.h>
#include <common/util/rt_util.h>
#include <common/debug/rt_log.h>
#include <hal/common/halctrl.h>



#define RTL8367_GE_PORT_MASK    (0xF) /*Port 0~Port 3*/



/* Function Name:
 *      rtl8367_mapper_switch_deviceInfo_get
 * Description:
 *      Get device information of the specific unit
 * Input:
 *      none
 * Output:
 *      pDevInfo - pointer to the device information
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */

int32
rtl8367_mapper_switch_deviceInfo_get(rtk_switch_devInfo_t *pDevInfo)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH),"%s",__FUNCTION__);

    /* check Init status */
    //RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pDevInfo), RT_ERR_NULL_POINTER);

    //memset(pDevInfo, 0x00, sizeof(rtk_switch_devInfo_t));
    pDevInfo->ge.portmask.bits[0]  =  (RTL8367_GE_PORT_MASK << CONFIG_EXTERNAL_SWITCH_PORT_OFFSET);
    //printk("%s portmask=%x \n", __FUNCTION__, pDevInfo->ge.portmask.bits[0]);

    return RT_ERR_OK;

}


