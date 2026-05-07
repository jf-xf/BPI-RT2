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
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <osal/lib.h>
#include <rtk/rtusr/include/rtusr_util.h>
#include <common/rt_type.h>

#include <rtdrv_ext_netfilter.h>

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
    rtdrv_rt_aclCfg_ext_t rt_acl_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == acl_filter_idx), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_acl_cfg.acl_filter, &acl_filter, sizeof(rt_acl_filterAndQos_t));
    GETSOCKOPT(RTDRV_RT_ACL_FILTERANDQOS_ADD, &rt_acl_cfg, rtdrv_rt_aclCfg_ext_t, 1);
    osal_memcpy(acl_filter_idx, &rt_acl_cfg.acl_filter_idx, sizeof(uint32));

    return RT_ERR_OK;
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
    rtdrv_rt_aclCfg_ext_t rt_acl_cfg={0};

    /* function body */
    osal_memcpy(&rt_acl_cfg.acl_filter_idx, &acl_filter_idx, sizeof(uint32));
    SETSOCKOPT(RTDRV_RT_ACL_FILTERANDQOS_DEL, &rt_acl_cfg, rtdrv_rt_aclCfg_ext_t, 1);

    return RT_ERR_OK;
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
 *      RT_ERR_NOT_ALLOWED 	    - Driver return fail
 * Note:
 *      The API can get acl filter rule by rule index
 */
int32
rt_acl_filterAndQos_get (
    uint32 acl_filter_idx, rt_acl_filterAndQos_t *acl_filter )
{
    rtdrv_rt_aclCfg_ext_t rt_acl_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == acl_filter), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_acl_cfg.acl_filter_idx, &acl_filter_idx, sizeof(uint32));
    GETSOCKOPT(RTDRV_RT_ACL_FILTERANDQOS_GET, &rt_acl_cfg, rtdrv_rt_aclCfg_ext_t, 1);
    osal_memcpy(acl_filter, &rt_acl_cfg.acl_filter, sizeof(rt_acl_filterAndQos_t));

    return RT_ERR_OK;
}   /* end of rt_acl_filterAndQos_get */

/* Function Name:
 *		rt_acl_reserved_status_get
 * Description:
 *		Get reserved acl status by reserved type 
 * Input:
 *		reserved type		  	- the type of the reserve acl which need to get
 * Output:
 *		status		  			- the reserved acl status which be found
 * Return:
 *		RT_ERR_OK				- OK
 *		RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *		RT_ERR_NOT_ALLOWED 	    - Driver return fail
 * Note:
 *		The API can get reserved acl status by rule type
 */
int32
rt_acl_reserved_status_get (
	rt_acl_reserved_type_t reserved_type, uint32 *status )
{
	rtdrv_rt_aclCfg_ext_t rt_acl_cfg;

	/* parameter check */
	RT_PARAM_CHK((NULL == status), RT_ERR_NULL_POINTER);

	/* function body */
	osal_memcpy(&rt_acl_cfg.reserved_type, &reserved_type, sizeof(uint32));
	GETSOCKOPT(RTDRV_RT_ACL_RESERVED_STATUS_GET, &rt_acl_cfg, rtdrv_rt_aclCfg_ext_t, 1);
	osal_memcpy(status, &rt_acl_cfg.reserved_status, sizeof(uint32));

	return RT_ERR_OK;
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
 *		RT_ERR_DRIVER_NOT_FOUND - Driver not found
 *		RT_ERR_NOT_ALLOWED 	    - Driver return fail
 * Note:
 *		The API can update reserved acl status by rule type
 */
int32
rt_acl_reserved_status_set (
	rt_acl_reserved_type_t reserved_type, uint32 status )
{
	rtdrv_rt_aclCfg_ext_t rt_acl_cfg = {0};

	/* function body */
	osal_memcpy(&rt_acl_cfg.reserved_type, &reserved_type, sizeof(uint32));
	osal_memcpy(&rt_acl_cfg.reserved_status, &status, sizeof(uint32));
	SETSOCKOPT(RTDRV_RT_ACL_RESERVED_STATUS_SET, &rt_acl_cfg, rtdrv_rt_aclCfg_ext_t, 1);

	return RT_ERR_OK;
}	/* end of rt_acl_reserved_status_set */

