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
 * Purpose : Definition of qos extension API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of aqm
 *           (2) Configuration of aqm pie
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
 *      rt_qos_aqm_get
 * Description:
 *      Get configuration of aqm
 * Input:
 *      
 * Output:
 *      pAqm_cfg 	- Configuration of aqm
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm
 */
int32 rt_qos_aqm_get(rt_qos_aqm_cfg_t* pAqm_cfg)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pAqm_cfg), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqm_cfg, pAqm_cfg, sizeof(rt_qos_aqm_cfg_t));
    GETSOCKOPT(RTDRV_RT_QOS_AQM_GET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);
    osal_memcpy(pAqm_cfg, &rt_qos_cfg.aqm_cfg, sizeof(rt_qos_aqm_cfg_t));

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_get */


/* Function Name:
 *      rt_qos_aqm_set
 * Description:
 *      Set configuration of aqm
 * Input:
 *      aqm_cfg 	- Configuration of aqm
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm
 */
int32 rt_qos_aqm_set(rt_qos_aqm_cfg_t aqm_cfg)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg={0};

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqm_cfg, &aqm_cfg, sizeof(rt_qos_aqm_cfg_t));
    SETSOCKOPT(RTDRV_RT_QOS_AQM_SET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_set */

/* Function Name:
 *      rt_qos_aqm_cos_get
 * Description:
 *      Get configuration of aqm cos
 * Input:
 *      
 * Output:
 *      pAqm_cos_cfg 	- Configuration of aqm cos
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm cos
 */
int32 rt_qos_aqm_cos_get(rt_qos_aqm_cos_cfg_t* pAqm_cos_cfg)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pAqm_cos_cfg), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqm_cos_cfg, pAqm_cos_cfg, sizeof(rt_qos_aqm_cos_cfg_t));
    GETSOCKOPT(RTDRV_RT_QOS_AQM_COS_GET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);
    osal_memcpy(pAqm_cos_cfg, &rt_qos_cfg.aqm_cos_cfg, sizeof(rt_qos_aqm_cos_cfg_t));

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_cos_get */


/* Function Name:
 *      rt_qos_aqm_cos_set
 * Description:
 *      Set configuration of aqm cos
 * Input:
 *      aqm_cos_cfg 	- Configuration of aqm cos
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm cos
 */
int32 rt_qos_aqm_cos_set(rt_qos_aqm_cos_cfg_t aqm_cos_cfg)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg={0};

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqm_cos_cfg, &aqm_cos_cfg, sizeof(rt_qos_aqm_cos_cfg_t));
    SETSOCKOPT(RTDRV_RT_QOS_AQM_COS_SET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_cos_set */

/* Function Name:
 *      rt_qos_aqm_threshold_get
 * Description:
 *      Get configuration of aqm threshold
 * Input:
 *      
 * Output:
 *      pAqm_threshold_cfg 	- Configuration of aqm threshold
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm threshold
 */
int32 rt_qos_aqm_threshold_get(rt_qos_aqm_threshold_cfg_t* pAqm_threshold_cfg)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pAqm_threshold_cfg), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqm_threshold_cfg, pAqm_threshold_cfg, sizeof(rt_qos_aqm_threshold_cfg_t));
    GETSOCKOPT(RTDRV_RT_QOS_AQM_THRESHOLD_GET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);
    osal_memcpy(pAqm_threshold_cfg, &rt_qos_cfg.aqm_threshold_cfg, sizeof(rt_qos_aqm_threshold_cfg_t));

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_threshold_get */


/* Function Name:
 *      rt_qos_aqm_threshold_set
 * Description:
 *      Set configuration of aqm threshold
 * Input:
 *      aqm_threshold_cfg 	- Configuration of aqm threshold
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm threshold
 */
int32 rt_qos_aqm_threshold_set(rt_qos_aqm_threshold_cfg_t aqm_threshold_cfg)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg={0};

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqm_threshold_cfg, &aqm_threshold_cfg, sizeof(rt_qos_aqm_threshold_cfg_t));
    SETSOCKOPT(RTDRV_RT_QOS_AQM_THRESHOLD_SET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_threshold_set */

/* Function Name:
 *      rt_qos_aqm_timer_get
 * Description:
 *      Get configuration of aqm timer
 * Input:
 *      
 * Output:
 *      pAqm_timer_cfg 	- Configuration of aqm timer
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm timer
 */
int32 rt_qos_aqm_timer_get(rt_qos_aqm_timer_cfg_t* pAqm_timer_cfg)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pAqm_timer_cfg), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqm_timer_cfg, pAqm_timer_cfg, sizeof(rt_qos_aqm_timer_cfg_t));
    GETSOCKOPT(RTDRV_RT_QOS_AQM_TIMER_GET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);
    osal_memcpy(pAqm_timer_cfg, &rt_qos_cfg.aqm_timer_cfg, sizeof(rt_qos_aqm_timer_cfg_t));

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_timer_get */


/* Function Name:
 *      rt_qos_aqm_timer_set
 * Description:
 *      Set configuration of aqm timer
 * Input:
 *      aqm_timer_cfg 	- Configuration of aqm timer
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm timer
 */
int32 rt_qos_aqm_timer_set(rt_qos_aqm_timer_cfg_t aqm_timer_cfg)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg={0};

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqm_timer_cfg, &aqm_timer_cfg, sizeof(rt_qos_aqm_timer_cfg_t));
    SETSOCKOPT(RTDRV_RT_QOS_AQM_TIMER_SET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_timer_set */

/* Function Name:
 *      rt_qos_aqm_flow_statistic_get
 * Description:
 *      Get flow statistic
 * Input:
 *      flow_idx
 * Output:
 *      pAqm_flow_stat 	- flow statistic
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get flow statistic
 */
int32 rt_qos_aqm_flow_statistic_get(rt_qos_aqm_flow_stat_t* pAqm_flow_stat)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pAqm_flow_stat), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqm_flow_statistic, pAqm_flow_stat, sizeof(rt_qos_aqm_flow_stat_t));
    GETSOCKOPT(RTDRV_RT_QOS_AQM_FLOW_STATISTIC_GET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);
    osal_memcpy(pAqm_flow_stat, &rt_qos_cfg.aqm_flow_statistic, sizeof(rt_qos_aqm_flow_stat_t));

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_flow_statistic_get */

//PIE
/* Function Name:
 *      rt_qos_aqm_pie_enable_get
 * Description:
 *      Get configuration of aqm pie enable
 * Input:
 *      
 * Output:
 *      pAqmPie_enable 	- Configuration of aqm pie enable
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie enable
 */
int32 rt_qos_aqm_pie_enable_get(rt_qos_aqm_pie_enable_t* pAqmPie_enable)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pAqmPie_enable), RT_ERR_NULL_POINTER);
    /* function body */

    osal_memcpy(&rt_qos_cfg.aqmPie_enable, pAqmPie_enable, sizeof(rt_qos_aqm_pie_enable_t));
    GETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_ENABLE_GET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);
    osal_memcpy(pAqmPie_enable, &rt_qos_cfg.aqmPie_enable, sizeof(rt_qos_aqm_pie_enable_t));

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_enable_get */

/* Function Name:
 *      rt_qos_aqm_pie_enable_set
 * Description:
 *      Set configuration of aqm pie enable
 * Input:
 *      aqmPie_enable 	- Configuration of aqm pie enable
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie enable
 */
int32 rt_qos_aqm_pie_enable_set(rt_qos_aqm_pie_enable_t aqmPie_enable)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    osal_memset(&rt_qos_cfg,0,sizeof(rtdrv_rt_qosCfg_ext_t));

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_enable, &aqmPie_enable, sizeof(rt_qos_aqm_pie_enable_t));
    SETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_ENABLE_SET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_enable_set */

/* Function Name:
 *      rt_qos_aqm_pie_match_get
 * Description:
 *      Get configuration of aqm pie match
 * Input:
 *      
 * Output:
 *      pAqmPie_match 	- Configuration of aqm pie match
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie match
 */
int32 rt_qos_aqm_pie_match_get(rt_qos_aqm_pie_match_t* pAqmPie_match)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pAqmPie_match), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_match, pAqmPie_match, sizeof(rt_qos_aqm_pie_match_t));
    GETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_MATCH_GET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);
    osal_memcpy(pAqmPie_match, &rt_qos_cfg.aqmPie_match, sizeof(rt_qos_aqm_pie_match_t));

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_match_get */

/* Function Name:
 *      rt_qos_aqm_pie_match_set
 * Description:
 *      Set configuration of aqm pie match
 * Input:
 *      aqmPie_match 	- Configuration of aqm pie match
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie match
 */
int32 rt_qos_aqm_pie_match_set(rt_qos_aqm_pie_match_t aqmPie_match)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    osal_memset(&rt_qos_cfg,0,sizeof(rtdrv_rt_qosCfg_ext_t));

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_match, &aqmPie_match, sizeof(rt_qos_aqm_pie_match_t));
    SETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_MATCH_SET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_match_set */

/* Function Name:
 *      rt_qos_aqm_pie_mapping_get
 * Description:
 *      Get configuration of aqm pie mapping
 * Input:
 *      
 * Output:
 *      pAqmPie_mapping 	- Configuration of aqm pie mapping
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie mapping
 */
int32 rt_qos_aqm_pie_mapping_get(rt_qos_aqm_pie_mapping_t* pAqmPie_mapping)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pAqmPie_mapping), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_mapping, pAqmPie_mapping, sizeof(rt_qos_aqm_pie_mapping_t));
    GETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_MAPPING_GET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);
    osal_memcpy(pAqmPie_mapping, &rt_qos_cfg.aqmPie_mapping, sizeof(rt_qos_aqm_pie_mapping_t));

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_mapping_get */

/* Function Name:
 *      rt_qos_aqm_pie_mapping_set
 * Description:
 *      Set configuration of aqm pie mapping
 * Input:
 *      aqmPie_mapping 	- Configuration of aqm pie mapping
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie mapping
 */
int32 rt_qos_aqm_pie_mapping_set(rt_qos_aqm_pie_mapping_t aqmPie_mapping)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    osal_memset(&rt_qos_cfg,0,sizeof(rtdrv_rt_qosCfg_ext_t));

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_mapping, &aqmPie_mapping, sizeof(rt_qos_aqm_pie_mapping_t));
    SETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_MAPPING_SET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_mapping_set */

/* Function Name:
 *      rt_qos_aqm_pie_threshold_get
 * Description:
 *      Get configuration of aqm pie threshold
 * Input:
 *      
 * Output:
 *      pAqmPie_threshold 	- Configuration of aqm pie threshold
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie threshold
 */
int32 rt_qos_aqm_pie_threshold_get(rt_qos_aqm_pie_threshold_t* pAqmPie_threshold)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pAqmPie_threshold), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_threshold, pAqmPie_threshold, sizeof(rt_qos_aqm_pie_threshold_t));
    GETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_THRESHOLD_GET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);
    osal_memcpy(pAqmPie_threshold, &rt_qos_cfg.aqmPie_threshold, sizeof(rt_qos_aqm_pie_threshold_t));

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_threshold_get */

/* Function Name:
 *      rt_qos_aqm_pie_threshold_set
 * Description:
 *      Set configuration of aqm pie threshold
 * Input:
 *      aqmPie_threshold 	- Configuration of aqm pie threshold
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie threshold
 */
int32 rt_qos_aqm_pie_threshold_set(rt_qos_aqm_pie_threshold_t aqmPie_threshold)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    osal_memset(&rt_qos_cfg,0,sizeof(rtdrv_rt_qosCfg_ext_t));

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_threshold, &aqmPie_threshold, sizeof(rt_qos_aqm_pie_threshold_t));
    SETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_THRESHOLD_SET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_threshold_set */

/* Function Name:
 *      rt_qos_aqm_pie_dpScale_get
 * Description:
 *      Get configuration of aqm pie dpScale
 * Input:
 *      
 * Output:
 *      pAqmPie_dpScale 	- Configuration of aqm pie dpScale
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie dpScale
 */
int32 rt_qos_aqm_pie_dpScale_get(rt_qos_aqm_pie_dpScale_t* pAqmPie_dpScale)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pAqmPie_dpScale), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_dpScale, pAqmPie_dpScale, sizeof(rt_qos_aqm_pie_dpScale_t));
    GETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_DPSCALE_GET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);
    osal_memcpy(pAqmPie_dpScale, &rt_qos_cfg.aqmPie_dpScale, sizeof(rt_qos_aqm_pie_dpScale_t));

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_dpScale_get */

/* Function Name:
 *      rt_qos_aqm_pie_dpScale_set
 * Description:
 *      Set configuration of aqm pie dpScale
 * Input:
 *      aqmPie_dpScale 	- Configuration of aqm pie dpScale
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie dpScale
 */
int32 rt_qos_aqm_pie_dpScale_set(rt_qos_aqm_pie_dpScale_t aqmPie_dpScale)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    osal_memset(&rt_qos_cfg,0,sizeof(rtdrv_rt_qosCfg_ext_t));

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_dpScale, &aqmPie_dpScale, sizeof(rt_qos_aqm_pie_dpScale_t));
    SETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_DPSCALE_SET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_dpScale_set */

/* Function Name:
 *      rt_qos_aqm_pie_dpThreshold_get
 * Description:
 *      Get configuration of aqm pie dpThreshold
 * Input:
 *      
 * Output:
 *      pAqmPie_dpThreshold 	- Configuration of aqm pie dpThreshold
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie dpThreshold
 */
int32 rt_qos_aqm_pie_dpThreshold_get(rt_qos_aqm_pie_dpThreshold_t* pAqmPie_dpThreshold)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pAqmPie_dpThreshold), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_dpThreshold, pAqmPie_dpThreshold, sizeof(rt_qos_aqm_pie_dpThreshold_t));
    GETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_DPTHRESHOLD_GET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);
    osal_memcpy(pAqmPie_dpThreshold, &rt_qos_cfg.aqmPie_dpThreshold, sizeof(rt_qos_aqm_pie_dpThreshold_t));

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_dpThreshold_get */

/* Function Name:
 *      rt_qos_aqm_pie_dpThreshold_set
 * Description:
 *      Set configuration of aqm pie dpThreshold
 * Input:
 *      aqmPie_dpThreshold 	- Configuration of aqm pie dpThreshold
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie dpThreshold
 */
int32 rt_qos_aqm_pie_dpThreshold_set(rt_qos_aqm_pie_dpThreshold_t aqmPie_dpThreshold)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    osal_memset(&rt_qos_cfg,0,sizeof(rtdrv_rt_qosCfg_ext_t));

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_dpThreshold, &aqmPie_dpThreshold, sizeof(rt_qos_aqm_pie_dpThreshold_t));
    SETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_DPTHRESHOLD_SET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_dpThreshold_set */

/* Function Name:
 *      rt_qos_aqm_pie_dpBurst_get
 * Description:
 *      Get configuration of aqm pie dpBurst
 * Input:
 *      
 * Output:
 *      pAqmPie_dpBurst 	- Configuration of aqm pie dpBurst
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie dpBurst
 */
int32 rt_qos_aqm_pie_dpBurst_get(rt_qos_aqm_pie_dpBurst_t* pAqmPie_dpBurst)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pAqmPie_dpBurst), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_dpBurst, pAqmPie_dpBurst, sizeof(rt_qos_aqm_pie_dpBurst_t));
    GETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_DPBURST_GET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);
    osal_memcpy(pAqmPie_dpBurst, &rt_qos_cfg.aqmPie_dpBurst, sizeof(rt_qos_aqm_pie_dpBurst_t));

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_dpBurst_get */

/* Function Name:
 *      rt_qos_aqm_pie_dpBurst_set
 * Description:
 *      Set configuration of aqm pie dpBurst
 * Input:
 *      aqmPie_dpBurst 	- Configuration of aqm pie dpBurst
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie dpBurst
 */
int32 rt_qos_aqm_pie_dpBurst_set(rt_qos_aqm_pie_dpBurst_t aqmPie_dpBurst)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    osal_memset(&rt_qos_cfg,0,sizeof(rtdrv_rt_qosCfg_ext_t));

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_dpBurst, &aqmPie_dpBurst, sizeof(rt_qos_aqm_pie_dpBurst_t));
    SETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_DPBURST_SET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_dpBurst_set */

/* Function Name:
 *      rt_qos_aqm_pie_qdelay_get
 * Description:
 *      Get configuration of aqm pie qdelay
 * Input:
 *      
 * Output:
 *      pAqmPie_qdelay 	- Configuration of aqm pie qdelay
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie qdelay
 */
int32 rt_qos_aqm_pie_qdelay_get(rt_qos_aqm_pie_qdelay_t* pAqmPie_qdelay)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pAqmPie_qdelay), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_qdelay, pAqmPie_qdelay, sizeof(rt_qos_aqm_pie_qdelay_t));
    GETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_QDELAY_GET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);
    osal_memcpy(pAqmPie_qdelay, &rt_qos_cfg.aqmPie_qdelay, sizeof(rt_qos_aqm_pie_qdelay_t));

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_qdelay_get */

/* Function Name:
 *      rt_qos_aqm_pie_qdelay_set
 * Description:
 *      Set configuration of aqm pie qdelay
 * Input:
 *      aqmPie_qdelay 	- Configuration of aqm pie qdelay
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie qdelay
 */
int32 rt_qos_aqm_pie_qdelay_set(rt_qos_aqm_pie_qdelay_t aqmPie_qdelay)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    osal_memset(&rt_qos_cfg,0,sizeof(rtdrv_rt_qosCfg_ext_t));

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_qdelay, &aqmPie_qdelay, sizeof(rt_qos_aqm_pie_qdelay_t));
    SETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_QDELAY_SET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_qdelay_set */

/* Function Name:
 *      rt_qos_aqm_pie_rate_get
 * Description:
 *      Get configuration of aqm pie rate
 * Input:
 *      
 * Output:
 *      pAqmPie_rate 	- Configuration of aqm pie rate
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie rate
 */
int32 rt_qos_aqm_pie_rate_get(rt_qos_aqm_pie_rate_t* pAqmPie_rate)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pAqmPie_rate), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_rate, pAqmPie_rate, sizeof(rt_qos_aqm_pie_rate_t));
    GETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_RATE_GET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);
    osal_memcpy(pAqmPie_rate, &rt_qos_cfg.aqmPie_rate, sizeof(rt_qos_aqm_pie_rate_t));

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_rate_get */

/* Function Name:
 *      rt_qos_aqm_pie_rate_set
 * Description:
 *      Set configuration of aqm pie rate
 * Input:
 *      aqmPie_rate 	- Configuration of aqm pie rate
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie rate
 */
int32 rt_qos_aqm_pie_rate_set(rt_qos_aqm_pie_rate_t aqmPie_rate)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    osal_memset(&rt_qos_cfg,0,sizeof(rtdrv_rt_qosCfg_ext_t));

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_rate, &aqmPie_rate, sizeof(rt_qos_aqm_pie_rate_t));
    SETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_RATE_SET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_rate_set */

/* Function Name:
 *      rt_qos_aqm_pie_cfg_get
 * Description:
 *      Get configuration of aqm pie cfg
 * Input:
 *      
 * Output:
 *      pAqmPie_cfg 	- Configuration of aqm pie cfg
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can get configuration of aqm pie cfg
 */
int32 rt_qos_aqm_pie_cfg_get(rt_qos_aqm_pie_cfg_t* pAqmPie_cfg)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pAqmPie_cfg), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_cfg, pAqmPie_cfg, sizeof(rt_qos_aqm_pie_cfg_t));
    GETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_CFG_GET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);
    osal_memcpy(pAqmPie_cfg, &rt_qos_cfg.aqmPie_cfg, sizeof(rt_qos_aqm_pie_cfg_t));

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_cfg_get */

/* Function Name:
 *      rt_qos_aqm_pie_cfg_set
 * Description:
 *      Set configuration of aqm pie cfg
 * Input:
 *      aqmPie_cfg 	- Configuration of aqm pie cfg
 * Output:
 *
 * Return:
 *      RT_ERR_OK					- OK
 *      RT_ERR_DRIVER_NOT_FOUND		- Driver not found
 *      RT_ERR_NOT_ALLOWED			- Driver return fail
 * Note:
 *      The API can set configuration of aqm pie cfg
 */
int32 rt_qos_aqm_pie_cfg_set(rt_qos_aqm_pie_cfg_t aqmPie_cfg)
{
    rtdrv_rt_qosCfg_ext_t rt_qos_cfg;

    osal_memset(&rt_qos_cfg,0,sizeof(rtdrv_rt_qosCfg_ext_t));

    /* function body */
    osal_memcpy(&rt_qos_cfg.aqmPie_cfg, &aqmPie_cfg, sizeof(rt_qos_aqm_pie_cfg_t));
    SETSOCKOPT(RTDRV_RT_QOS_AQM_PIE_CFG_SET, &rt_qos_cfg, rtdrv_rt_qosCfg_ext_t, 1);

    return RT_ERR_OK;
}   /* end of rt_qos_aqm_pie_cfg_set */

