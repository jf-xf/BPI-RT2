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
    rtdrv_rt_igmpCfg_ext_t rt_igmp_cfg;
	osal_memset(&rt_igmp_cfg, 0x0, sizeof(rt_igmp_cfg));	

    /* function body */
    osal_memcpy(&rt_igmp_cfg.mcMode, &mcMode, sizeof(rt_igmp_multicast_learning_mode_t));
    SETSOCKOPT(RTDRV_RT_IGMP_MULTICASTMODE_SET, &rt_igmp_cfg, rtdrv_rt_igmpCfg_ext_t, 1);

    return RT_ERR_OK;
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
    rtdrv_rt_igmpCfg_ext_t rt_igmp_cfg;
	osal_memset(&rt_igmp_cfg, 0x0, sizeof(rt_igmp_cfg));	

    /* function body */
    osal_memcpy(&rt_igmp_cfg.mcConfig, &mcConfig, sizeof(rt_igmp_multicast_group_cfg_t));
    SETSOCKOPT(RTDRV_RT_IGMP_MULTICASTGROUP_SET, &rt_igmp_cfg, rtdrv_rt_igmpCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_igmp_multicastGroup_set */


/* Function Name:
 *      rt_igmp_multicastGroupDev_set
 * Description:
 *      Set multicast group Device config.
 * Input:
 *      mcDevConfig        - Multicast group
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 * 		RT_ERR_DRIVER_NOT_FOUND    - Driver not found
 * 	 	RT_ERR_NOT_ALLOWED 	    - Driver return fail
 * Note:
 *      The API can set multicast group
 */
int32
rt_igmp_multicastGroupDev_set (
    rt_igmp_group_devPort_cfg_t mcDevConfig )
{
    rtdrv_rt_igmpCfg_ext_t rt_igmp_cfg;
	osal_memset(&rt_igmp_cfg, 0x0, sizeof(rt_igmp_cfg));	

    /* function body */
    osal_memcpy(&rt_igmp_cfg.mcDevConfig, &mcDevConfig, sizeof(rt_igmp_group_devPort_cfg_t));
    SETSOCKOPT(RTDRV_RT_IGMP_MULTICASTGROUP_DEV_SET, &rt_igmp_cfg, rtdrv_rt_igmpCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_igmp_multicastGroup_set */


/* Function Name:
 *		rt_igmp_multicastGroupDev_get
 * Description:
 *		Get multicast group Device config.
 * Input:
 *		mcDevConfig        - Multicast group ingress patten
 * Output:
 *		mcDevConfig        - Multicast group egress device/port
 * Return:
 *      RT_ERR_OK               - OK
 * 		RT_ERR_DRIVER_NOT_FOUND    - Driver not found
 * 	 	RT_ERR_NOT_ALLOWED 	    - Driver return fail

 * Note:
 *		 The API can get multicast group device/port info
 */
int32
rt_igmp_multicastGroupDev_get (
	rt_igmp_group_devPort_cfg_t *pMcDevConfig)
{
	rtdrv_rt_igmpCfg_ext_t rt_igmp_cfg;
	osal_memset(&rt_igmp_cfg, 0x0, sizeof(rt_igmp_cfg));	

    /* parameter check */
    RT_PARAM_CHK((NULL == pMcDevConfig), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_igmp_cfg.mcDevConfig, pMcDevConfig, sizeof(rt_igmp_group_devPort_cfg_t));
    GETSOCKOPT(RTDRV_RT_IGMP_MULTICASTGROUP_DEV_GET, &rt_igmp_cfg, rtdrv_rt_igmpCfg_ext_t, 1);
    osal_memcpy(pMcDevConfig, &rt_igmp_cfg.mcDevConfig, sizeof(rt_igmp_group_devPort_cfg_t));

    return RT_ERR_OK;
}   /* end of rt_flow_hwnatMode_get */



/* Function Name:
 *		rt_igmp_unknownMcAction_set
 * Description:
 *		Set unknown Multicsaat default action
 * Input:
 *		rt_igmp_unknownMc_cfg_t		- unknown multicast config
 * Output:
 *		None.
 * Return:
 *		RT_ERR_OK				- OK
 * Note:
 *		The API can set unknown multicast drop/trap
 */
int32
rt_igmp_unknownMcAction_set (
	rt_igmp_unknownMc_cfg_t mcUnknCfg)
{
	rtdrv_rt_igmpCfg_ext_t rt_igmp_cfg;
	osal_memset(&rt_igmp_cfg, 0x0, sizeof(rt_igmp_cfg));	

	/* function body */
	osal_memcpy(&rt_igmp_cfg.mcUnknCfg, &mcUnknCfg, sizeof(rt_igmp_unknownMc_cfg_t));
	SETSOCKOPT(RTDRV_RT_IGMP_UNKNOWN_MC_SET, &rt_igmp_cfg, rtdrv_rt_igmpCfg_ext_t, 1);

	return RT_ERR_OK;
}	/* end of rt_igmp_unknownMcAction_set */


