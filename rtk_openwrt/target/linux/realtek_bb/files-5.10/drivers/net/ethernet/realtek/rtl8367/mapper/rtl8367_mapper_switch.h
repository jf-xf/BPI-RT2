#ifndef _RTL8367_MAPPER_SWITCH_H
#define _RTL8367_MAPPER_SWITCH_H




#include <dal/dal_ext_switch_mapper.h>


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

extern int32
rtl8367_mapper_switch_deviceInfo_get(rtk_switch_devInfo_t *pDevInfo);


#endif /* _RTL8367_MAPPER_SWITCH_H */

