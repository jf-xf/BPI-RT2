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
#include <common/rt_type.h>
#include <rtk/rt/rt_port.h>
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
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;
	
	if((p_rt_ext->qos_aqm_get(pAqm_cfg))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_set(aqm_cfg))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_cos_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;
	
	if((p_rt_ext->qos_aqm_cos_get(pAqm_cos_cfg))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_cos_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_cos_set(aqm_cos_cfg))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_threshold_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;
	
	if((p_rt_ext->qos_aqm_threshold_get(pAqm_threshold_cfg))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_threshold_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_threshold_set(aqm_threshold_cfg))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_timer_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;
	
	if((p_rt_ext->qos_aqm_timer_get(pAqm_timer_cfg))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_timer_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_timer_set(aqm_timer_cfg))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_flow_statistic_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;
	
	if((p_rt_ext->qos_aqm_flow_statistic_get(pAqm_flow_stat))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_enable_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_enable_get(pAqmPie_enable))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
	int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_enable_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_enable_set(aqmPie_enable))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
    int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_match_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_match_get(pAqmPie_match))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
    int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_match_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_match_set(aqmPie_match))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
    int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_mapping_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_mapping_get(pAqmPie_mapping))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
    int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_mapping_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_mapping_set(aqmPie_mapping))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
    int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_threshold_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_threshold_get(pAqmPie_threshold))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
    int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_threshold_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_threshold_set(aqmPie_threshold))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
    int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_dpScale_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_dpScale_get(pAqmPie_dpScale))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
    int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_dpScale_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_dpScale_set(aqmPie_dpScale))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
   int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_dpThreshold_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_dpThreshold_get(pAqmPie_dpThreshold))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
    int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_dpThreshold_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_dpThreshold_set(aqmPie_dpThreshold))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
   int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_dpBurst_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_dpBurst_get(pAqmPie_dpBurst))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
    int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_dpBurst_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_dpBurst_set(aqmPie_dpBurst))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
    int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_qdelay_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_qdelay_get(pAqmPie_qdelay))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
    int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_qdelay_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_qdelay_set(aqmPie_qdelay))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
    int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_rate_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_rate_get(pAqmPie_rate))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
    int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_rate_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_rate_set(aqmPie_rate))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
    int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_cfg_get == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_cfg_get(pAqmPie_cfg))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
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
    int32	ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();

	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->qos_aqm_pie_cfg_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->qos_aqm_pie_cfg_set(aqmPie_cfg))!=0)
		return RT_ERR_NOT_ALLOWED;

	return ret;
}   /* end of rt_qos_aqm_pie_cfg_set */

EXPORT_SYMBOL(rt_qos_aqm_get);
EXPORT_SYMBOL(rt_qos_aqm_set);
EXPORT_SYMBOL(rt_qos_aqm_cos_get);
EXPORT_SYMBOL(rt_qos_aqm_cos_set);
EXPORT_SYMBOL(rt_qos_aqm_threshold_get);
EXPORT_SYMBOL(rt_qos_aqm_threshold_set);
EXPORT_SYMBOL(rt_qos_aqm_timer_get);
EXPORT_SYMBOL(rt_qos_aqm_timer_set);
EXPORT_SYMBOL(rt_qos_aqm_flow_statistic_get);
EXPORT_SYMBOL(rt_qos_aqm_pie_enable_get);
EXPORT_SYMBOL(rt_qos_aqm_pie_enable_set);
EXPORT_SYMBOL(rt_qos_aqm_pie_match_get);
EXPORT_SYMBOL(rt_qos_aqm_pie_match_set);
EXPORT_SYMBOL(rt_qos_aqm_pie_mapping_get);
EXPORT_SYMBOL(rt_qos_aqm_pie_mapping_set);
EXPORT_SYMBOL(rt_qos_aqm_pie_threshold_get);
EXPORT_SYMBOL(rt_qos_aqm_pie_threshold_set);
EXPORT_SYMBOL(rt_qos_aqm_pie_dpScale_get);
EXPORT_SYMBOL(rt_qos_aqm_pie_dpScale_set);
EXPORT_SYMBOL(rt_qos_aqm_pie_dpThreshold_get);
EXPORT_SYMBOL(rt_qos_aqm_pie_dpThreshold_set);
EXPORT_SYMBOL(rt_qos_aqm_pie_dpBurst_get);
EXPORT_SYMBOL(rt_qos_aqm_pie_dpBurst_set);
EXPORT_SYMBOL(rt_qos_aqm_pie_qdelay_get);
EXPORT_SYMBOL(rt_qos_aqm_pie_qdelay_set);
EXPORT_SYMBOL(rt_qos_aqm_pie_rate_get);
EXPORT_SYMBOL(rt_qos_aqm_pie_rate_set);
EXPORT_SYMBOL(rt_qos_aqm_pie_cfg_get);
EXPORT_SYMBOL(rt_qos_aqm_pie_cfg_set);

