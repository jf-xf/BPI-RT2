#ifndef _RTL8367_MAPPER_RATE_H_
#define _RTL8367_MAPPER_RATE_H_
#include <dal/dal_ext_switch_mapper.h>


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
 *      The link status of the port is as following:
 *      - LINKDOWN
 *      - LINKUP
 */

int32
rtl8367_mapper_rate_portEgrBandwidthCtrlRate_get(rtk_port_t port, uint32 *pRate);



/* Function Name:
 *      rtl8367_mapper_rate_portEgrBandwidthCtrlRate_set
 * Description:
 *      Set the egress bandwidth control rate.
 * Input:
 *      port    - port id
 *      rate - egress bandwidth control rate
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
rtl8367_mapper_rate_portEgrBandwidthCtrlRate_set(rtk_port_t  port, uint32 rate);


#endif /* _RTL8367_MAPPER_RATE_H_ */

