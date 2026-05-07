#include <rtk_types.h>
#include <rtl8367_mapper_rate.h>
#include <rate.h>


#include <osal/memory.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_bitop.h>
#include <common/util/rt_util.h>
#include <common/debug/rt_log.h>
#include <hal/common/halctrl.h>


/* Function Name:
 *      rtl8367_mapper_rate_portEgrBandwidthCtrlRate_get
 * Description:
 *     Get the egress bandwidth control rate.
 * Input:
 *      port    - port id
 * Output:
 *      pRate - egress bandwidth control rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 */


int32
rtl8367_mapper_rate_portEgrBandwidthCtrlRate_get(rtk_port_t port, uint32 *pRate)
{

    rtksw_port_t port_id;
    rtksw_api_ret_t retVal;
    rtksw_rate_t port_rate;
    rtksw_enable_t pIfg_include;

 //   printk("%s port:%d \n", __FUNCTION__, port);
#if 1
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE),"%s",__FUNCTION__);

    /* check Init status */
    //RT_INIT_CHK(port_init);

    /* parameter check */
    //RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
#endif
    port_id = (rtksw_port_t)RT_PORT_TO_EXT_SWITCH_PORT(port);
    memset(&port_rate, 0x00, sizeof(rtksw_rate_t));
    if ((retVal = rtksw_rate_egrBandwidthCtrlRate_get(port_id, &port_rate, &pIfg_include)) != RT_ERR_OK)
        return (int32)retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtl8367_mapper_rate_portEgrBandwidthCtrlRate_set
 * Description:
 *      Set the egress bandwidth control rate.
 * Input:
 *      port    - port id
 *      rate    - egress bandwidth control rate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN - link down port status
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity in Apollo is 8Kbps.
 */


int32
rtl8367_mapper_rate_portEgrBandwidthCtrlRate_set(rtk_port_t port, uint32 rate)
{

    rtksw_port_t port_id;
    rtksw_api_ret_t retVal;
    rtksw_rate_t port_rate;
    rtksw_enable_t pIfg_include;

    //printk("%s port:%d \n", __FUNCTION__, port);
#if 1
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE),"%s",__FUNCTION__);

    /* check Init status */
    //RT_INIT_CHK(port_init);

    /* parameter check */
    //RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == rate), RT_ERR_NULL_POINTER);
#endif
    port_id = (rtksw_port_t)RT_PORT_TO_EXT_SWITCH_PORT(port);
    port_rate = rate;
    pIfg_include = RTKSW_ENABLED;
    if ((retVal = rtksw_rate_egrBandwidthCtrlRate_set(port_id, port_rate, pIfg_include)) != RT_ERR_OK)
	   return (int32)retVal;
    //printk("%s port_rate:%d pIfg_include:%d retVal=%d\n", __FUNCTION__, port_rate, pIfg_include, retVal);
    return RT_ERR_OK;
}


