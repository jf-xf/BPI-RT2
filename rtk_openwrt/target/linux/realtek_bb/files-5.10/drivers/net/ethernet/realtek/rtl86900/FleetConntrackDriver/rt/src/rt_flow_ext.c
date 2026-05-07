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
 * $Revision: 68395 $
 * $Date: 2016-05-27 16:38:35 +0800 (Fri, 27 May 2016) $
 *
 * Purpose : Definition of flow extension API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) flow callback functions register
 *           (2) flow operations
 *           (3) flow acceleration mode configuration
 *
 */

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/init.h>
#include <rtk/default.h>
#include <dal/dal_mgmt.h>
#include <common/util/rt_util.h>
#include <hal/common/halctrl.h>

#include <rt_ext_mapper.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */

/*
 * Function Declaration
 */
 /* Function Name:
 *      rt_flow_callbackFunc_register
 * Description:
 *      register callback function
 * Input:
 *      cbFuncType	- callback function type
 *      pCbFunc		- callback function pointer
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can register callback function.
 *      Please note that data type of pCbFunc is varied according to the callback function type.
 */
extern int32
rt_flow_callbackFunc_register (
	rt_flow_callback_func_t cbFuncType,
	void* pCbFunc)
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->flow_callbackFunc_register == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->flow_callbackFunc_register(cbFuncType, pCbFunc))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}	/* end of rt_flow_callbackFunc_register */

 
/* Function Name:
 *      rt_flow_operation_add
 * Description:
 *      make a flow operation
 * Input:
 *      flowOp	- flow oparation
 *      pOpData	- data for oparation
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can make an operation to flows.
 *      Please note that data type of pOpData is varied according to the operation.
 */
extern int32
rt_flow_operation_add(
	rt_flow_ops_t flowOp,
	rt_flow_ops_data_t* pOpData)
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->flow_operation_add == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->flow_operation_add(flowOp, pOpData))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}	/* end of rt_flow_operation_add */

	
/* Function Name:
 *      rt_flow_hwnatMode_set
 * Description:
 *      set configuration of hwnat mode
 * Input:
 *      hwnatMode	- Configuration of hwnat mode
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can set configuration of hwnat mode
 */
extern int32
rt_flow_hwnatMode_set (
	rt_flow_hwnat_mode_t hwnatMode)
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->flow_hwnatMode_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->flow_hwnatMode_set(hwnatMode))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}	/* end of rt_flow_hwnatMode_set */


/* Function Name:
 *      rt_flow_hwnatMode_get
 * Description:
 *      get configuration of hwnat mode
 * Input:
 *      None.
 * Output:
 *      pHwnatMode	- Configuration of hwnat mode
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *      The API can get configuration of hwnat mode
 */
extern int32
rt_flow_hwnatMode_get (
	rt_flow_hwnat_mode_t *pHwnatMode)
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->flow_hwnatMode_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->flow_hwnatMode_get(pHwnatMode))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}	/* end of rt_flow_hwnatMode_set */

EXPORT_SYMBOL(rt_flow_callbackFunc_register);
EXPORT_SYMBOL(rt_flow_operation_add);
EXPORT_SYMBOL(rt_flow_hwnatMode_set);
EXPORT_SYMBOL(rt_flow_hwnatMode_get);


