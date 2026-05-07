/*
 * Copyright (C) 2023 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision:  $
 * $Date:  $
 *
 * Purpose : Definition of TIME API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) IEEE 1588
 *           (2) PON ToD
 *           (3) 1PPS/ToD
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
#include <rtdrv/rtdrv_netfilter.h>
#include <common/rt_type.h>
#include <rtk/rt/rt_time.h>

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
 *      rt_time_init
 * Description:
 *      Initialize Time module.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize Time module before calling any Time APIs.
 */
int32
rt_time_init(void)
{
    rtdrv_rt_timeCfg_t rt_time_cfg;

    /* function body */
    osal_memset(&rt_time_cfg,0x0,sizeof(rtdrv_rt_timeCfg_t));
    SETSOCKOPT(RTDRV_RT_TIME_INIT, &rt_time_cfg, rtdrv_rt_timeCfg_t, 1);

    return RT_ERR_OK;
} /* end of rt_time_init */

/* Function Name:
 *      rt_time_curTime_get
 * Description:
 *      Get the current time.
 * Input:
 *      None
 * Output:
 *      pTimeStamp - pointer buffer of the current time
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
rt_time_curTime_get(rt_time_timeStamp_t *pTimeStamp)
{
    rtdrv_rt_timeCfg_t rt_time_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pTimeStamp), RT_ERR_NULL_POINTER);

    /* function body */
    GETSOCKOPT(RTDRV_RT_TIME_CURTIME_GET, &rt_time_cfg, rtdrv_rt_timeCfg_t, 1);
    osal_memcpy(pTimeStamp, &rt_time_cfg.timeStamp, sizeof(rt_time_timeStamp_t));

    return RT_ERR_OK;
} /* end of rt_time_curTime_get */

/* Function Name:
 *      rt_time_ponTodTime_set
 * Description:
 *      Set the PON TOD time.
 * Input:
 *      ponTod   - pon tod configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 * Note:
 *      Set corresponding superfram/localtime to update timer
 */
int32
rt_time_ponTodTime_set(rt_pon_tod_t ponTod)
{
    rtdrv_rt_timeCfg_t rt_time_cfg;

    osal_memcpy(&rt_time_cfg.ponTod, &ponTod, sizeof(rt_pon_tod_t));
    SETSOCKOPT(RTDRV_RT_TIME_PONTODTIME_SET, &rt_time_cfg, rtdrv_rt_timeCfg_t, 1);

    return RT_ERR_OK;
} /* end of rt_time_ponTodTime_set */

/* Function Name:
 *      rt_time_ponTodTime_get
 * Description:
 *      Get the PON TOD time.
 * Input:
 *      None
 * Output:
 *      pPonTod   - pon tod configuration
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 * Note:
 *      Get corresponding superfram/localtime to update timer
 */
int32
rt_time_ponTodTime_get(rt_pon_tod_t *pPonTod)
{
    rtdrv_rt_timeCfg_t rt_time_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pPonTod), RT_ERR_NULL_POINTER);

    /* function body */
    GETSOCKOPT(RTDRV_RT_TIME_PONTODTIME_GET, &rt_time_cfg, rtdrv_rt_timeCfg_t, 1);
    osal_memcpy(pPonTod, &rt_time_cfg.ponTod, sizeof(rt_pon_tod_t));

    return RT_ERR_OK;
} /* end of rt_time_ponTodTime_get */

/* Function Name:
 *      rt_time_ponTodTimeEnable_set
 * Description:
 *      Set the PON TOD time.
 * Input:
 *      enable   - enable pon tod configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 * Note:
 *      Enable/Disable PON ToD to update time
 */
int32
rt_time_ponTodTimeEnable_set(rt_enable_t enable)
{
    rtdrv_rt_timeCfg_t rt_time_cfg;

    osal_memcpy(&rt_time_cfg.enable, &enable, sizeof(rt_enable_t));
    SETSOCKOPT(RTDRV_RT_TIME_PONTODTIMEENABLE_SET, &rt_time_cfg, rtdrv_rt_timeCfg_t, 1);

    return RT_ERR_OK;
} /* end of rt_time_ponTodTimeEnable_set */

/* Function Name:
 *      rt_time_ponTodTimeEnable_get
 * Description:
 *      Get the PON TOD time.
 * Input:
 *      None
 * Output:
 *      pEnable   - enable pon tod configuration
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 * Note:
 *      Getting status enable/eisable PON ToD to update time
 */
int32
rt_time_ponTodTimeEnable_get(rt_enable_t *pEnable)
{
    rtdrv_rt_timeCfg_t rt_time_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    /* function body */
    GETSOCKOPT(RTDRV_RT_TIME_PONTODTIMEENABLE_GET, &rt_time_cfg, rtdrv_rt_timeCfg_t, 1);
    osal_memcpy(pEnable, &rt_time_cfg.enable, sizeof(rt_enable_t));

    return RT_ERR_OK;
} /* end of rt_time_ponTodTimeEnable_get */
