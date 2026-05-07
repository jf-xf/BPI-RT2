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
 * Purpose : Definition of ACL extension API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Reset/display/configure acl rule structure and then add this rule.
 *           (2) Get/delete acl rule by rule index.
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
 *      rt_acl_filterAndQos_add
 * Description:
 *      Add acl rule 
 * Input:
 *      acl_filter        - a acl rule include the patterns which need to be filtered, and related action
 * Output:
 *      acl_filter_idx        - the index of the added acl rule
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *      RT_ERR_NOT_ALLOWED      - Driver return fail
 * Note:
 *      The API can add acl filter and qos remarking rule
 */
int32
rt_acl_filterAndQos_add (
    rt_acl_filterAndQos_t acl_filter, uint32 *acl_filter_idx )
{
	int32   ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->acl_filterAndQos_add == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->acl_filterAndQos_add(&acl_filter, acl_filter_idx))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}   /* end of rt_acl_filterAndQos_add */

/* Function Name:
 *      rt_acl_filterAndQos_del
 * Description:
 *      Delete acl filter rule 
 * Input:
 *      acl_filter_idx        - the index of the acl rule which need to be delete
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *      RT_ERR_NOT_ALLOWED      - Driver return fail
 * Note:
 *      The API can delete acl rule
 */
int32
rt_acl_filterAndQos_del (
    uint32 acl_filter_idx )
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();
	
	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;
	
	if(p_rt_ext->acl_filterAndQos_del == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;
	
	if((p_rt_ext->acl_filterAndQos_del(acl_filter_idx))!=0)
		return RT_ERR_NOT_ALLOWED;
	
	return ret;
}   /* end of rt_acl_filterAndQos_del */

/* Function Name:
 *      rt_acl_filterAndQos_get
 * Description:
 *      Get acl filter rule 
 * Input:
 *      acl_filter_idx        - the index of the acl rule which need to get
 * Output:
 *      acl_filter        - the acl rule which be found
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *      RT_ERR_NOT_ALLOWED      - Driver return fail
 * Note:
 *      The API can get acl filter rule by rule index
 */
int32
rt_acl_filterAndQos_get (
    uint32 acl_filter_idx, rt_acl_filterAndQos_t *acl_filter )
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();
	
	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;
	
	if(p_rt_ext->acl_filterAndQos_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;
	
	if((p_rt_ext->acl_filterAndQos_get(acl_filter_idx, acl_filter))!=0)
		return RT_ERR_NOT_ALLOWED;
	
	return ret;
}   /* end of rt_acl_filterAndQos_get */

/* Function Name:
 *		rt_acl_reserved_status_get
 * Description:
 *		Get reserved acl status by reserved type 
 * Input:
 *		reserved type			- the type of the reserve acl which need to get
 * Output:
 *		status					- the reserved acl status which be found
 * Return:
 *		RT_ERR_OK				- OK
 *      RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *      RT_ERR_NOT_ALLOWED      - Driver return fail
 * Note:
 *		The API can get reserved acl status by rule type
 */
int32
rt_acl_reserved_status_get (
	rt_acl_reserved_type_t reserved_type, uint32 *status )
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();
	
	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;
	
	if(p_rt_ext->acl_reserved_status_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;
	
	if((p_rt_ext->acl_reserved_status_get(reserved_type, status))!=0)
		return RT_ERR_NOT_ALLOWED;
	
	return ret;
}	/* end of rt_acl_reserved_status_get */

/* Function Name:
 *		rt_acl_reserved_status_set
 * Description:
 *		Set reserved acl status by reserved type 
 * Input:
 *		reserved type			- the type of the reserve acl which need to set
 * Output:
 *		status					- the reserved acl status should be update
 * Return:
 *		RT_ERR_OK				- OK
 *      RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *      RT_ERR_NOT_ALLOWED      - Driver return fail
 * Note:
 *		The API can update reserved acl status by rule type
 */
int32
rt_acl_reserved_status_set (
	rt_acl_reserved_type_t reserved_type, uint32 status )
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();
	
	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;
	
	if(p_rt_ext->acl_reserved_status_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;
	
	if((p_rt_ext->acl_reserved_status_set(reserved_type, status))!=0)
		return RT_ERR_NOT_ALLOWED;
	
	return ret;

}	/* end of rt_acl_reserved_status_set */

EXPORT_SYMBOL(rt_acl_filterAndQos_add);
EXPORT_SYMBOL(rt_acl_filterAndQos_del);
EXPORT_SYMBOL(rt_acl_filterAndQos_get);
EXPORT_SYMBOL(rt_acl_reserved_status_get);
EXPORT_SYMBOL(rt_acl_reserved_status_set);


