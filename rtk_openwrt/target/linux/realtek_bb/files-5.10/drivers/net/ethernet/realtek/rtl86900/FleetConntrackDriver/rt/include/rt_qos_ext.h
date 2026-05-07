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


#ifndef __RT_QOS_EXT_H__
#define __RT_QOS_EXT_H__


/*
 * Include Files
 */
#include <common/rt_type.h>

/*
 * Symbol Definition
 */
#define RT_QOS_AQM_FLOW_TABLE_SIZE 65536
#define RT_QOS_AQM_TIMER_INTERVAL_MS_MIN 1 
#define RT_QOS_AQM_TIMER_INTERVAL_MS_MAX 51000
#define RT_QOS_AQM_TIMER_PKTSIZE_WINDOW_SHIFT_MAX 16
#define RT_QOS_AQM_TIMER_BYTECNT_WEIGHT_SHIFT_MAX 15
/*
 * Data Declaration
 */

typedef enum {
	RT_QOS_AQM_ACTION_TYPE_PIE_DROP		= 0,
	RT_QOS_AQM_ACTION_TYPE_MODIFY_COS 	= 1,
	RT_QOS_AQM_ACTION_TYPE_MAX,
} rt_qos_aqm_action_type_e;

typedef enum {
	 RT_QOS_AQM_FLOW_TYPE_SMALL  	= 0,
	 RT_QOS_AQM_FLOW_TYPE_MIDDLE	= 1,
	 RT_QOS_AQM_FLOW_TYPE_BIG 	 	= 2,
	 RT_QOS_AQM_FLOW_TYPE_MAX,
} rt_qos_aqm_flow_type_e;

typedef enum {
	RT_QOS_AQM_FLOW_TYPE_BITMASK_SMALL    	= (0x1<<RT_QOS_AQM_FLOW_TYPE_SMALL),
	RT_QOS_AQM_FLOW_TYPE_BITMASK_MIDDLE   	= (0x1<<RT_QOS_AQM_FLOW_TYPE_MIDDLE),
	RT_QOS_AQM_FLOW_TYPE_BITMASK_BIG	   	= (0x1<<RT_QOS_AQM_FLOW_TYPE_BIG),
	RT_QOS_AQM_FLOW_TYPE_BITMASK_MAX	   	= (0x1<<RT_QOS_AQM_FLOW_TYPE_MAX),
} rt_qos_aqm_flow_type_bitmask_e;

typedef enum {
	RT_QOS_AQM_FLOW_PROTO_TCP		   	= 0,
	RT_QOS_AQM_FLOW_PROTO_UDP		   	= 1,
	RT_QOS_AQM_FLOW_PROTO_OTHER	   		= 2,
	RT_QOS_AQM_FLOW_PROTO_MAX	   		= 3,
} rt_qos_aqm_flow_protocol_e;
	
typedef enum {
	RT_QOS_AQM_FLOW_PROTO_BITMASK_TCP		   	= (0x1<<RT_QOS_AQM_FLOW_PROTO_TCP),
	RT_QOS_AQM_FLOW_PROTO_BITMASK_UDP		   	= (0x1<<RT_QOS_AQM_FLOW_PROTO_UDP),
	RT_QOS_AQM_FLOW_PROTO_BITMASK_OTHER	   		= (0x1<<RT_QOS_AQM_FLOW_PROTO_OTHER),
	RT_QOS_AQM_FLOW_PROTO_BITMASK_MAX	   		= (0x1<<RT_QOS_AQM_FLOW_PROTO_MAX),
} rt_qos_aqm_flow_protocol_bitmask_e;

typedef struct rt_qos_aqm_cfg_s{
	unsigned int enable;
	rt_qos_aqm_action_type_e action_type;
}rt_qos_aqm_cfg_t;

typedef struct rt_qos_aqm_cos_cfg_s{
	rt_qos_aqm_flow_type_bitmask_e flow_type_mask;
	unsigned int big_cos;
	unsigned int middle_cos;
	unsigned int small_cos;
}rt_qos_aqm_cos_cfg_t;

typedef struct rt_qos_aqm_threshold_cfg_s{
	rt_qos_aqm_flow_type_e flow_type;
	rt_qos_aqm_flow_protocol_bitmask_e protocol_mask;
	unsigned int pktSize_threshold;
	unsigned int byteCnt_threshold_on;
	unsigned int byteCnt_threshold_off;
}rt_qos_aqm_threshold_cfg_t;

typedef struct rt_qos_aqm_timer_cfg_s{
	unsigned int timer_interval; //ms
	unsigned int pktSize_window_shift;
	unsigned int byteCnt_weight_shift;
}rt_qos_aqm_timer_cfg_t;

typedef struct rt_qos_aqm_flow_stat_s{
	unsigned int flow_idx;
	unsigned int flow_proto;
	rt_qos_aqm_flow_type_e flow_type;
	unsigned int avg_byteCnt;
	unsigned int cur_byteCnt;
	unsigned int window_pktSize;
}rt_qos_aqm_flow_stat_t;

//PIE
typedef struct rt_qos_aqm_pie_enable_s{
	unsigned int enable;
	unsigned int deepq;
	unsigned int ldpid;
}rt_qos_aqm_pie_enable_t;

typedef struct rt_qos_aqm_pie_match_s{
	unsigned int match;
	unsigned int profile;
	unsigned int ldpid;
	unsigned int ldpid_msk;
	unsigned int cos;
	unsigned int cos_msk;
}rt_qos_aqm_pie_match_t;

typedef struct rt_qos_aqm_pie_mapping_s{
	unsigned int mapping;
	unsigned int pie_voq;
	unsigned int ldpid;
	unsigned int cos;
}rt_qos_aqm_pie_mapping_t;

typedef struct rt_qos_aqm_pie_threshold_s{
	rt_qos_aqm_flow_type_e flow_type;
	unsigned int trigger;
	unsigned int full;
	unsigned int mean;
}rt_qos_aqm_pie_threshold_t;

typedef struct rt_qos_aqm_pie_dpScale_s{
	rt_qos_aqm_flow_type_e flow_type;
	unsigned int alpha;
	unsigned int beta;
}rt_qos_aqm_pie_dpScale_t;

typedef struct rt_qos_aqm_pie_dpThreshold_s{
	rt_qos_aqm_flow_type_e flow_type;
	unsigned int ceiling;
	unsigned int accu_ceiling;
	unsigned int accu_floor;
}rt_qos_aqm_pie_dpThreshold_t;

typedef struct rt_qos_aqm_pie_dpBurst_s{
	rt_qos_aqm_flow_type_e flow_type;
	unsigned int allowance;
	unsigned int decrement;
}rt_qos_aqm_pie_dpBurst_t;

typedef struct rt_qos_aqm_pie_qdelay_s{
	rt_qos_aqm_flow_type_e flow_type;
	unsigned int pie_voq;
	unsigned int burst_reset;
	unsigned int high_latency;
	unsigned int low_latency;
	unsigned int minimum_target;
}rt_qos_aqm_pie_qdelay_t;

typedef struct rt_qos_aqm_pie_rate_s{
	unsigned int pie_voq;
	unsigned int msr;
	unsigned int peak;
}rt_qos_aqm_pie_rate_t;

typedef struct rt_qos_aqm_pie_cfg_s{
	unsigned int mode;
	unsigned int timer_interval;
	unsigned int timer_prescalar;
	unsigned int dp_scale;
	unsigned int random;
	unsigned int fix_LFSR;
}rt_qos_aqm_pie_cfg_t;

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
extern int32 rt_qos_aqm_get(rt_qos_aqm_cfg_t* pAqm_cfg);


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
extern int32 rt_qos_aqm_set(rt_qos_aqm_cfg_t aqm_cfg);

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
extern int32 rt_qos_aqm_cos_get(rt_qos_aqm_cos_cfg_t* pAqm_cos_cfg);


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
extern int32 rt_qos_aqm_cos_set(rt_qos_aqm_cos_cfg_t aqm_cos_cfg);

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
extern int32 rt_qos_aqm_threshold_get(rt_qos_aqm_threshold_cfg_t* pAqm_threshold_cfg);


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
extern int32 rt_qos_aqm_threshold_set(rt_qos_aqm_threshold_cfg_t aqm_threshold_cfg);

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
extern int32 rt_qos_aqm_timer_get(rt_qos_aqm_timer_cfg_t* pAqm_timer_cfg);


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
extern int32 rt_qos_aqm_timer_set(rt_qos_aqm_timer_cfg_t aqm_timer_cfg);

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
extern int32 rt_qos_aqm_flow_statistic_get(rt_qos_aqm_flow_stat_t* pAqm_flow_stat);

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
extern int32 rt_qos_aqm_pie_enable_get(rt_qos_aqm_pie_enable_t* pAqmPie_enable);

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
extern int32 rt_qos_aqm_pie_enable_set(rt_qos_aqm_pie_enable_t aqmPie_enable);

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
extern int32 rt_qos_aqm_pie_match_get(rt_qos_aqm_pie_match_t* pAqmPie_match);

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
extern int32 rt_qos_aqm_pie_match_set(rt_qos_aqm_pie_match_t aqmPie_match);

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
extern int32 rt_qos_aqm_pie_mapping_get(rt_qos_aqm_pie_mapping_t* pAqmPie_mapping);

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
extern int32 rt_qos_aqm_pie_mapping_set(rt_qos_aqm_pie_mapping_t aqmPie_mapping);

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
extern int32 rt_qos_aqm_pie_threshold_get(rt_qos_aqm_pie_threshold_t* pAqmPie_threshold);

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
extern int32 rt_qos_aqm_pie_threshold_set(rt_qos_aqm_pie_threshold_t aqmPie_threshold);

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
extern int32 rt_qos_aqm_pie_dpScale_get(rt_qos_aqm_pie_dpScale_t* pAqmPie_dpScale);

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
extern int32 rt_qos_aqm_pie_dpScale_set(rt_qos_aqm_pie_dpScale_t aqmPie_dpScale);

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
extern int32 rt_qos_aqm_pie_dpThreshold_get(rt_qos_aqm_pie_dpThreshold_t* pAqmPie_dpThreshold);

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
extern int32 rt_qos_aqm_pie_dpThreshold_set(rt_qos_aqm_pie_dpThreshold_t aqmPie_dpThreshold);

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
extern int32 rt_qos_aqm_pie_dpBurst_get(rt_qos_aqm_pie_dpBurst_t* pAqmPie_dpBurst);

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
extern int32 rt_qos_aqm_pie_dpBurst_set(rt_qos_aqm_pie_dpBurst_t aqmPie_dpBurst);

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
extern int32 rt_qos_aqm_pie_qdelay_get(rt_qos_aqm_pie_qdelay_t* pAqmPie_qdelay);

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
extern int32 rt_qos_aqm_pie_qdelay_set(rt_qos_aqm_pie_qdelay_t aqmPie_qdelay);

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
extern int32 rt_qos_aqm_pie_rate_get(rt_qos_aqm_pie_rate_t* pAqmPie_rate);

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
extern int32 rt_qos_aqm_pie_rate_set(rt_qos_aqm_pie_rate_t aqmPie_rate);

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
extern int32 rt_qos_aqm_pie_cfg_get(rt_qos_aqm_pie_cfg_t* pAqmPie_cfg);

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
extern int32 rt_qos_aqm_pie_cfg_set(rt_qos_aqm_pie_cfg_t aqmPie_cfg);

#endif /* __RT_QOS_EXT_H__ */
