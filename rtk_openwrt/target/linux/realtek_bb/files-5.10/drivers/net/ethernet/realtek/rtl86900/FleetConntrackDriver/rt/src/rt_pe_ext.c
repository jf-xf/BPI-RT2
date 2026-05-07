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
 * Purpose : Definition of PE API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of http test
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
 *      rt_pe_http_test
 * Description:
 *      Set configuration of pe http test
 * Input:
 *      http_test_req	- Configuration of http test request 
 * Output:
 *		pHttp_test_result
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of pe http test
 */
int rt_pe_http_test(rt_pe_http_test_request_t http_test_req, rt_pe_http_test_result_t *pHttp_test_result)
{
    int32 ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->pe_http_test == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;
	
	if((p_rt_ext->pe_http_test(http_test_req, pHttp_test_result))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}   /* end of rt_pe_http_test */

EXPORT_SYMBOL(rt_pe_http_test);

