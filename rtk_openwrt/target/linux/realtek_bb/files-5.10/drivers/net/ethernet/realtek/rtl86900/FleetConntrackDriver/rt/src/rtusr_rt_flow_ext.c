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
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <osal/lib.h>
#include <rtk/rtusr/include/rtusr_util.h>
#include <common/rt_type.h>
	 
#include <rtdrv_ext_netfilter.h>


#include "rt_stat_ext.h" 				//for rt_stat_flow_mib_t

//#include <net/netfilter/nf_conntrack.h>	//for struct nf_conn


/*
 * Symbol Definition
 */
 
/*
 * Macro Declaration
 */


/*
 * Data Declaration
 */


/*
 * Function Declaration
 */
/* Function Name:
 *		rt_flow_operation_add
 * Description:
 *		make a flow operation
 * Input:
 *		flowOp	- flow oparation
  *		pOpData	- data for oparation
 * Output:
 *		None.
 * Return:
 *		RT_ERR_OK				- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *		The API can make an operation to flows.
 *		Please note that data type of pOpData is varied according to the operation.
 */
int32
rt_flow_operation_add(
	rt_flow_ops_t flowOp,
	rt_flow_ops_data_t* pOpData)
{
    rtdrv_rt_flowCfg_ext_t rt_flow_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pOpData), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_flow_cfg.flowOp, &flowOp, sizeof(rt_flow_ops_t));
    osal_memcpy(&rt_flow_cfg.opData, pOpData, sizeof(rt_flow_ops_data_t));
    GETSOCKOPT(RTDRV_RT_FLOW_OPERATION_ADD, &rt_flow_cfg, rtdrv_rt_flowCfg_ext_t, 1);
	osal_memcpy(pOpData, &rt_flow_cfg.opData, sizeof(rt_flow_ops_data_t));

    return RT_ERR_OK;
}   /* end of rt_flow_operation_add */
	
/* Function Name:
 *		rt_flow_hwnatMode_set
 * Description:
 *		set configuration of hwnat mode
 * Input:
 *		hwnatMode	- Configuration of hwnat mode
 * Output:
 *		None.
 * Return:
 *		RT_ERR_OK				- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *		The API can set configuration of hwnat mode
 */
int32
rt_flow_hwnatMode_set (
	rt_flow_hwnat_mode_t hwnatMode)
{
    rtdrv_rt_flowCfg_ext_t rt_flow_cfg={0};

    /* function body */
    osal_memcpy(&rt_flow_cfg.hwnatMode, &hwnatMode, sizeof(rt_flow_hwnat_mode_t));
    SETSOCKOPT(RTDRV_RT_FLOW_HWNATMODE_SET, &rt_flow_cfg, rtdrv_rt_flowCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_flow_hwnatMode_set */

/* Function Name:
 *		rt_flow_hwnatMode_get
 * Description:
 *		get configuration of hwnat mode
 * Input:
 *		None.
 * Output:
 *		pHwnatMode	- Configuration of hwnat mode
 * Return:
 *		RT_ERR_OK				- OK
 *      RT_ERR_DRIVER_NOT_FOUND	- Driver not found
 *      RT_ERR_NOT_ALLOWED		- Driver return fail
 * Note:
 *		The API can get configuration of hwnat mode
 */
int32
rt_flow_hwnatMode_get (
	rt_flow_hwnat_mode_t *pHwnatMode)
{
    rtdrv_rt_flowCfg_ext_t rt_flow_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pHwnatMode), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_flow_cfg.hwnatMode, pHwnatMode, sizeof(rt_flow_hwnat_mode_t));
    GETSOCKOPT(RTDRV_RT_FLOW_HWNATMODE_GET, &rt_flow_cfg, rtdrv_rt_flowCfg_ext_t, 1);
    osal_memcpy(pHwnatMode, &rt_flow_cfg.hwnatMode, sizeof(rt_flow_hwnat_mode_t));

    return RT_ERR_OK;
}   /* end of rt_flow_hwnatMode_get */

