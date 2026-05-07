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
 * Purpose : Definition of IGMP extension API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of multicast learning mode
 *           (2) Configuration of multicast group
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
 *      rt_igmp_multicastMode_set
 * Description:
 *      Set multicast learning Mode 
 * Input:
 *      mcMode        - Multicast mode
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 * 	 RT_ERR_DRIVER_NOT_FOUND    - Driver not found
 * 	 RT_ERR_NOT_ALLOWED 	    - Driver return fail
 * Note:
 *      The API can set multicast learning mode
 */
int32
rt_igmp_multicastMode_set (
    rt_igmp_multicast_learning_mode_t mcMode )
{
    int32   ret=RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->igmp_multicastMode_set == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->igmp_multicastMode_set(mcMode))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;
}   /* end of rt_igmp_multicastMode_set */

/* Function Name:
 *      rt_igmp_multicastGroup_set
 * Description:
 *      Set multicast group.
 * Input:
 *      mcConfig        - Multicast group
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 * 	 RT_ERR_DRIVER_NOT_FOUND    - Driver not found
 * 	 RT_ERR_NOT_ALLOWED 	    - Driver return fail
 * Note:
 *      The API can set multicast group
 */
int32
rt_igmp_multicastGroup_set (
    rt_igmp_multicast_group_cfg_t mcConfig )
{
    int32   ret=RT_ERR_OK;
    rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->igmp_multicastGroup_set == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->igmp_multicastGroup_set(mcConfig))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;
}   /* end of rt_igmp_multicastGroup_set */


/* Function Name:
 *		rt_igmp_multicastGroupDev_set
 * Description:
 *		Set multicast groupDev.
 * Input:
 *		mcDevConfig		- Multicast groupDev
 * Output:
 *		None.
 * Return:
 *		RT_ERR_OK				- OK
 *	 RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *	 RT_ERR_NOT_ALLOWED 		- Driver return fail
 * Note:
 *		The API can set multicast groupDev
 */
int32
rt_igmp_multicastGroupDev_set (
	rt_igmp_group_devPort_cfg_t mcDevConfig )
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->igmp_multicastGroupDev_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->igmp_multicastGroupDev_set(&mcDevConfig))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}	/* end of rt_igmp_multicastGroupDev_set */

/* Function Name:
 *		rt_igmp_multicastGroupDev_get
 * Description:
 *		Get multicast groupDev Config.
 * Input:
 *		mcDevConfig 	- Multicast groupDev
 * Output:
 *		mcDevConfig 	- Multicast groupDev
 * Return:
 *		RT_ERR_OK				- OK
 *	 RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *	 RT_ERR_NOT_ALLOWED 		- Driver return fail
 * Note:
 *		The API can set multicast groupDev
 */
int32
rt_igmp_multicastGroupDev_get (
	rt_igmp_group_devPort_cfg_t *pMcDevConfig )
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->igmp_multicastGroupDev_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->igmp_multicastGroupDev_get(pMcDevConfig))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}	/* end of rt_igmp_multicastGroupDev_get */



int32 rt_igmp_multicastFlow_check(rt_igmp_multicast_group_cfg_t *pMcConfig )
{
	int32   ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->igmp_multicastFlow_check == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->igmp_multicastFlow_check(pMcConfig))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
} 


/* Function Name:
 *		rt_igmp_unknownMcAction_set
 * Description:
 *		Set unknown Multicsaat default action
 * Input:
 *		rt_igmp_unknownMc_cfg_t 	- unknown 
multicast config
 * Output:
 *		None.
 * Return:
 *		RT_ERR_OK				- OK
 * Note:
 *		The API can set unknown multicast drop/trap
 */
int32
rt_igmp_unknownMcAction_set (
	rt_igmp_unknownMc_cfg_t mcUnknCfg )
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->igmp_unknownMulticastAction_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->igmp_unknownMulticastAction_set(mcUnknCfg))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}	/* end of rt_igmp_unknownMcAction_set */


/* Function Name:
 *		rt_igmp_userGroup_check
 * Description:
 *		check if user group address is vaild
 * Input:
 *		rt_igmp_unknownMc_cfg_t 	- user group infomation
 * Output:
 *		None.
 * Return:
 *		RT_ERR_OK				- OK
 * Note:
 *		The API can check if user group address is vaild
 */
int32
rt_igmp_userGroup_check (
	rtk_igmp_userGroup_cfg_t *userGroup )
{
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->igmp_userGroup_check == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->igmp_userGroup_check(userGroup))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}	/* end of rt_igmp_unknownMcAction_set */


EXPORT_SYMBOL(rt_igmp_multicastMode_set);
EXPORT_SYMBOL(rt_igmp_multicastGroup_set);
EXPORT_SYMBOL(rt_igmp_multicastGroupDev_set);
EXPORT_SYMBOL(rt_igmp_multicastGroupDev_get);
EXPORT_SYMBOL(rt_igmp_unknownMcAction_set);
EXPORT_SYMBOL(rt_igmp_multicastFlow_check);
EXPORT_SYMBOL(rt_igmp_userGroup_check);


