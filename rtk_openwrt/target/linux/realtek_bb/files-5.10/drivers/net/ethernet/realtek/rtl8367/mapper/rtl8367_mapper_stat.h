#ifndef _RTL8367_MAPPER_STAT_H
#define _RTL8367_MAPPER_STAT_H
/*
 * Include Files
 */
#include <dal/dal_ext_switch_mapper.h> 
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <rtk_types.h>
#include <rtk/stat.h>


//#include <rtk_types.h>


/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*
 * Macro Declaration
 */


/*
 * Function Declaration
 */

/* Function Name:
 *      rtl8367_mapper_stat_port_reset
 * Description:
 *      Reset the specified port counters in the specified device.
 * Input:
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Note:
 *      None
 */

extern int32 rtl8367_mapper_stat_port_reset(rtk_port_t port);

/* Function Name:
 *      rtl8367_mapper_stat_port_getAll
 * Description:
 *      Get all counters of one specified port in the specified device.
 * Input:
 *      port        - port id
 * Output:
 *      pPortCntrs - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_NULL_POINTER        - input parameter may be null pointer
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Note:
 *      None
 */
extern int32 rtl8367_mapper_stat_port_getAll(rtk_port_t port, rtk_stat_port_cntr_t *pPortCntrs);




#endif /* _RTL8367_MAPPER_STAT_H */

