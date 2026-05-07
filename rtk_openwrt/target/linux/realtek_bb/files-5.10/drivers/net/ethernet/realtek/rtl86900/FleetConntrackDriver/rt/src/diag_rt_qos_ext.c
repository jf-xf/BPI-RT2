#include <stdio.h>
#include <string.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <diag_str.h>
#include <parser/cparser_priv.h>

#include <rt_qos_ext.h>


/*
 * rt_qos set aqm disable
 */
cparser_result_t
cparser_cmd_rt_qos_set_aqm_disable(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_cfg_t aqm_cfg;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&aqm_cfg, 0x0, sizeof(rt_qos_aqm_cfg_t));
	aqm_cfg.enable = 0;

	ret = rt_qos_aqm_set(aqm_cfg);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to set aqm ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to set aqm. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set aqm successfully: %s\n", "Disabled");
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_qos_set_aqm_disable */

/*
 * rt_qos set aqm enable action_type <UINT:action_type>
 */
cparser_result_t
cparser_cmd_rt_qos_set_aqm_enable_action_type_action_type(
	cparser_context_t *context,
	uint32_t  *action_type_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_cfg_t aqm_cfg;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	if(*action_type_ptr >= RT_QOS_AQM_ACTION_TYPE_MAX)
	{
		diag_util_mprintf("Fail to set aqm ... action_type is out of range !! \n");
		return CPARSER_NOT_OK;
	}
	
	osal_memset(&aqm_cfg, 0x0, sizeof(rt_qos_aqm_cfg_t));
	aqm_cfg.enable = 1;
	aqm_cfg.action_type = *action_type_ptr;

	ret = rt_qos_aqm_set(aqm_cfg);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to set aqm ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to set aqm. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set aqm successfully: %s\n", "Enabled");
	}	

    return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_set_aqm_enable_action_type_action_type */

/*
 * rt_qos get aqm
 */
cparser_result_t
cparser_cmd_rt_qos_get_aqm(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_cfg_t aqm_cfg;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&aqm_cfg, 0x0, sizeof(rt_qos_aqm_cfg_t));

	ret = rt_qos_aqm_get(&aqm_cfg);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get aqm ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get aqm. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Aqm: %s \n", (aqm_cfg.enable)?"Enabled":"Disabled");
		diag_util_mprintf("action_type: %s \n", (aqm_cfg.action_type==RT_QOS_AQM_ACTION_TYPE_MODIFY_COS)?"Modify COS":"PIE drop");
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_qos_get_aqm */

/*
 * rt_qos set aqm cos flow_type_mask <UINT:flow_type_mask> big_cos <UINT:big_cos> middle_cos <UINT:middle_cos> small_cos <UINT:small_cos>
 */
cparser_result_t
cparser_cmd_rt_qos_set_aqm_cos_flow_type_mask_flow_type_mask_big_cos_big_cos_middle_cos_middle_cos_small_cos_small_cos(
    cparser_context_t *context,
    uint32_t  *flow_type_mask_ptr,
    uint32_t  *big_cos_ptr,
    uint32_t  *middle_cos_ptr,
    uint32_t  *small_cos_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_cos_cfg_t aqm_cos_cfg;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	if(*flow_type_mask_ptr >= RT_QOS_AQM_FLOW_TYPE_BITMASK_MAX)
	{
		diag_util_mprintf("Fail to set aqm cos ... flow_type_mask is out of range !! \n");
		return CPARSER_NOT_OK;
	}
	
	osal_memset(&aqm_cos_cfg, 0x0, sizeof(rt_qos_aqm_cos_cfg_t));
	aqm_cos_cfg.flow_type_mask 	= *flow_type_mask_ptr;
	aqm_cos_cfg.big_cos 		= *big_cos_ptr;
	aqm_cos_cfg.middle_cos 		= *middle_cos_ptr;
	aqm_cos_cfg.small_cos 		= *small_cos_ptr;

	ret = rt_qos_aqm_cos_set(aqm_cos_cfg);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to set aqm cos... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to set aqm cos. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set aqm cos successfully.\n");
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_qos_set_aqm_cos_flow_type_mask_flow_type_mask_big_cos_big_cos_middle_cos_middle_cos_small_cos_small_cos */

/*
 * rt_qos get aqm cos
 */
cparser_result_t
cparser_cmd_rt_qos_get_aqm_cos(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_cos_cfg_t aqm_cos_cfg;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&aqm_cos_cfg, 0x0, sizeof(rt_qos_aqm_cos_cfg_t));

	ret = rt_qos_aqm_cos_get(&aqm_cos_cfg);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get aqm cos ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get aqm cos. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Aqm cos enabled flow type: %s%s%s \n", 
							(aqm_cos_cfg.flow_type_mask&RT_QOS_AQM_FLOW_TYPE_BITMASK_BIG)?"BIG ":"",
							(aqm_cos_cfg.flow_type_mask&RT_QOS_AQM_FLOW_TYPE_BITMASK_MIDDLE)?"MIDDLE ":"",
							(aqm_cos_cfg.flow_type_mask&RT_QOS_AQM_FLOW_TYPE_BITMASK_SMALL)?"SMALL ":"");
		diag_util_mprintf("[Big] Aqm cos: %d \n", aqm_cos_cfg.big_cos);
		diag_util_mprintf("[Middle] Aqm cos: %d \n", aqm_cos_cfg.middle_cos);
		diag_util_mprintf("[Small] Aqm cos: %d \n", aqm_cos_cfg.small_cos);
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_qos_get_aqm_cos */

/*
 * rt_qos set aqm big protocol_mask <UINT:protocol_mask> pktSize_threshold <UINT:pktSize_threshold> byteCnt_threshold_on <UINT:byteCnt_threshold_on> byteCnt_threshold_off <UINT:byteCnt_threshold_off>
 */
cparser_result_t
cparser_cmd_rt_qos_set_aqm_big_protocol_mask_protocol_mask_pktSize_threshold_pktSize_threshold_byteCnt_threshold_on_byteCnt_threshold_on_byteCnt_threshold_off_byteCnt_threshold_off(
    cparser_context_t *context,
    uint32_t  *protocol_mask_ptr,
    uint32_t  *pktSize_threshold_ptr,
    uint32_t  *byteCnt_threshold_on_ptr,
    uint32_t  *byteCnt_threshold_off_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_threshold_cfg_t aqm_threshold_cfg;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&aqm_threshold_cfg, 0x0, sizeof(rt_qos_aqm_threshold_cfg_t));
	aqm_threshold_cfg.flow_type 				= RT_QOS_AQM_FLOW_TYPE_BIG;
	aqm_threshold_cfg.protocol_mask 			= *protocol_mask_ptr;
	aqm_threshold_cfg.pktSize_threshold 		= *pktSize_threshold_ptr;
	aqm_threshold_cfg.byteCnt_threshold_on 		= *byteCnt_threshold_on_ptr;
	aqm_threshold_cfg.byteCnt_threshold_off 	= *byteCnt_threshold_off_ptr;

	ret = rt_qos_aqm_threshold_set(aqm_threshold_cfg);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to set aqm big threshold... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to set aqm big threshold. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set aqm big threshold successfully.\n");
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_qos_set_aqm_big_protocol_mask_protocol_mask_pktsize_threshold_pktsize_threshold_bytecnt_threshold_on_bytecnt_threshold_on_bytecnt_threshold_off_bytecnt_threshold_off */

/*
 * rt_qos get aqm big
 */
cparser_result_t
cparser_cmd_rt_qos_get_aqm_big(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_threshold_cfg_t aqm_threshold_cfg;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&aqm_threshold_cfg, 0x0, sizeof(rt_qos_aqm_threshold_cfg_t));
	aqm_threshold_cfg.flow_type = RT_QOS_AQM_FLOW_TYPE_BIG;
	
	ret = rt_qos_aqm_threshold_get(&aqm_threshold_cfg);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get aqm big threshold... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get aqm big threshold. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Aqm big enabled protocol: %s%s%s \n", 
							(aqm_threshold_cfg.protocol_mask&RT_QOS_AQM_FLOW_PROTO_BITMASK_TCP)?"TCP ":"",
							(aqm_threshold_cfg.protocol_mask&RT_QOS_AQM_FLOW_PROTO_BITMASK_UDP)?"UDP ":"",
							(aqm_threshold_cfg.protocol_mask&RT_QOS_AQM_FLOW_PROTO_BITMASK_OTHER)?"OTHER ":"");
		diag_util_mprintf("===== unit: 128 bytes =====\n");
		diag_util_mprintf("\tpktSize_threshold: %d \n", aqm_threshold_cfg.pktSize_threshold);
		diag_util_mprintf("\tbyteCnt_threshold_on: %d \n", aqm_threshold_cfg.byteCnt_threshold_on);
		diag_util_mprintf("\tbyteCnt_threshold_off: %d \n", aqm_threshold_cfg.byteCnt_threshold_off);
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_qos_get_aqm_big */

/*
 * rt_qos set aqm middle protocol_mask <UINT:protocol_mask> pktSize_threshold <UINT:pktSize_threshold> byteCnt_threshold_on <UINT:byteCnt_threshold_on> byteCnt_threshold_off <UINT:byteCnt_threshold_off>
 */
cparser_result_t
cparser_cmd_rt_qos_set_aqm_middle_protocol_mask_protocol_mask_pktSize_threshold_pktSize_threshold_byteCnt_threshold_on_byteCnt_threshold_on_byteCnt_threshold_off_byteCnt_threshold_off(
    cparser_context_t *context,
    uint32_t  *protocol_mask_ptr,
    uint32_t  *pktSize_threshold_ptr,
    uint32_t  *byteCnt_threshold_on_ptr,
    uint32_t  *byteCnt_threshold_off_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_threshold_cfg_t aqm_threshold_cfg;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&aqm_threshold_cfg, 0x0, sizeof(rt_qos_aqm_threshold_cfg_t));
	aqm_threshold_cfg.flow_type 				= RT_QOS_AQM_FLOW_TYPE_MIDDLE;
	aqm_threshold_cfg.protocol_mask 			= *protocol_mask_ptr;
	aqm_threshold_cfg.pktSize_threshold 		= *pktSize_threshold_ptr;
	aqm_threshold_cfg.byteCnt_threshold_on 		= *byteCnt_threshold_on_ptr;
	aqm_threshold_cfg.byteCnt_threshold_off 	= *byteCnt_threshold_off_ptr;

	ret = rt_qos_aqm_threshold_set(aqm_threshold_cfg);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to set aqm middle threshold... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to set aqm middle threshold. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set aqm middle threshold successfully.\n");
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_qos_set_aqm_middle_protocol_mask_protocol_mask_pktsize_threshold_pktsize_threshold_bytecnt_threshold_on_bytecnt_threshold_on_bytecnt_threshold_off_bytecnt_threshold_off */

/*
 * rt_qos get aqm middle
 */
cparser_result_t
cparser_cmd_rt_qos_get_aqm_middle(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_threshold_cfg_t aqm_threshold_cfg;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&aqm_threshold_cfg, 0x0, sizeof(rt_qos_aqm_threshold_cfg_t));
	aqm_threshold_cfg.flow_type = RT_QOS_AQM_FLOW_TYPE_MIDDLE;
	
	ret = rt_qos_aqm_threshold_get(&aqm_threshold_cfg);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get aqm middle threshold... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get aqm middle threshold. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Aqm middle enabled protocol: %s%s%s \n", 
							(aqm_threshold_cfg.protocol_mask&RT_QOS_AQM_FLOW_PROTO_BITMASK_TCP)?"TCP ":"",
							(aqm_threshold_cfg.protocol_mask&RT_QOS_AQM_FLOW_PROTO_BITMASK_UDP)?"UDP ":"",
							(aqm_threshold_cfg.protocol_mask&RT_QOS_AQM_FLOW_PROTO_BITMASK_OTHER)?"OTHER ":"");
		diag_util_mprintf("===== unit: 128 bytes =====\n");
		diag_util_mprintf("\tpktSize_threshold: %d \n", aqm_threshold_cfg.pktSize_threshold);
		diag_util_mprintf("\tbyteCnt_threshold_on: %d \n", aqm_threshold_cfg.byteCnt_threshold_on);
		diag_util_mprintf("\tbyteCnt_threshold_off: %d \n", aqm_threshold_cfg.byteCnt_threshold_off);
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_qos_get_aqm_middle */

/*
 * rt_qos set aqm timer timer_interval <UINT:timer_interval> pktSize_window_shift <UINT:pktSize_window_shift> byteCnt_weight_shift <UINT:byteCnt_weight_shift>
 */
cparser_result_t
cparser_cmd_rt_qos_set_aqm_timer_timer_interval_timer_interval_pktSize_window_shift_pktSize_window_shift_byteCnt_weight_shift_byteCnt_weight_shift(
    cparser_context_t *context,
    uint32_t  *timer_interval_ptr,
    uint32_t  *pktSize_window_shift_ptr,
    uint32_t  *byteCnt_weight_shift_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_timer_cfg_t aqm_timer_cfg;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	if(*timer_interval_ptr<RT_QOS_AQM_TIMER_INTERVAL_MS_MIN || *timer_interval_ptr>RT_QOS_AQM_TIMER_INTERVAL_MS_MAX)
	{
		diag_util_mprintf("Fail to set aqm timer ... timer_interval is out of range !! \n");
		return CPARSER_NOT_OK;
	}
	if(*pktSize_window_shift_ptr > RT_QOS_AQM_TIMER_PKTSIZE_WINDOW_SHIFT_MAX)
	{
		diag_util_mprintf("Fail to set aqm timer ... pktSize_window_shift is out of range !! \n");
		return CPARSER_NOT_OK;
	}
	if(*byteCnt_weight_shift_ptr > RT_QOS_AQM_TIMER_BYTECNT_WEIGHT_SHIFT_MAX)
	{
		diag_util_mprintf("Fail to set aqm timer ... byteCnt_weight_shift is out of range !! \n");
		return CPARSER_NOT_OK;
	}
	
	osal_memset(&aqm_timer_cfg, 0x0, sizeof(rt_qos_aqm_timer_cfg_t));
	aqm_timer_cfg.timer_interval 			= *timer_interval_ptr;
	aqm_timer_cfg.pktSize_window_shift 		= *pktSize_window_shift_ptr;
	aqm_timer_cfg.byteCnt_weight_shift 		= *byteCnt_weight_shift_ptr;

	ret = rt_qos_aqm_timer_set(aqm_timer_cfg);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to set aqm timer... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to set aqm timer. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set aqm timer successfully.\n");
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_qos_set_aqm_timer_timer_interval_timer_interval_pktsize_window_shift_pktsize_window_shift_bytecnt_weight_shift_bytecnt_weight_shift */

/*
 * rt_qos get aqm timer
 */
cparser_result_t
cparser_cmd_rt_qos_get_aqm_timer(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_timer_cfg_t aqm_timer_cfg;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&aqm_timer_cfg, 0x0, sizeof(rt_qos_aqm_timer_cfg_t));

	ret = rt_qos_aqm_timer_get(&aqm_timer_cfg);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get aqm timer... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get aqm timer. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("timer_interval: %d ms\n", aqm_timer_cfg.timer_interval);
		diag_util_mprintf("pktSize_window_shift: %d \n", aqm_timer_cfg.pktSize_window_shift);
		diag_util_mprintf("byteCnt_weight_shift: %d \n", aqm_timer_cfg.byteCnt_weight_shift);
	}	

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_qos_get_aqm_timer */

/*
 * rt_qos get aqm flow_statistic <UINT:flow_idx>
 */
cparser_result_t
cparser_cmd_rt_qos_get_aqm_flow_statistic_flow_idx(
	cparser_context_t *context,
    uint32_t  *flow_idx_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_timer_cfg_t aqm_timer_cfg;
	rt_qos_aqm_flow_stat_t aqm_flow_stat;
	static char* aqm_flow_proto_Name[4] = { "Invalid", "Tcp", "Udp", "Other"};
	static char* aqm_flow_type_Name[4] = { "Small", "Middle", "Big", "Error type"};
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	if(*flow_idx_ptr >= RT_QOS_AQM_FLOW_TABLE_SIZE)
	{
		diag_util_mprintf("Fail to get aqm flow_statistic ... flow_idx[%d] is out of range(0~%d) !! \n", *flow_idx_ptr, RT_QOS_AQM_FLOW_TABLE_SIZE-1);
		return CPARSER_NOT_OK;
	}

	osal_memset(&aqm_timer_cfg, 0x0, sizeof(rt_qos_aqm_timer_cfg_t));
	osal_memset(&aqm_flow_stat, 0x0, sizeof(rt_qos_aqm_flow_stat_t));
	
	ret = rt_qos_aqm_timer_get(&aqm_timer_cfg);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get aqm timer... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get aqm timer. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}

	aqm_flow_stat.flow_idx = *flow_idx_ptr;
	ret = rt_qos_aqm_flow_statistic_get(&aqm_flow_stat);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get aqm flow_statistic ... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get aqm flow_statistic. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("flow_proto: %s \n", aqm_flow_proto_Name[aqm_flow_stat.flow_proto]);
		diag_util_mprintf("flow_type : %s \n", aqm_flow_type_Name[aqm_flow_stat.flow_type]);
		diag_util_mprintf("===== timer_interval: %d ms, unit: 128 bytes =====\n", aqm_timer_cfg.timer_interval);
		diag_util_mprintf("cur_byteCnt: %u(0x%x)\n", aqm_flow_stat.cur_byteCnt, aqm_flow_stat.cur_byteCnt);
		diag_util_mprintf("avg_byteCnt: %u(0x%x)\n", aqm_flow_stat.avg_byteCnt, aqm_flow_stat.avg_byteCnt);
		diag_util_mprintf("window_pktSize: %u(0x%x)\n", aqm_flow_stat.window_pktSize, aqm_flow_stat.window_pktSize);
		diag_util_mprintf("avg_pktSize: %u(0x%x)\n", aqm_flow_stat.window_pktSize>>16, aqm_flow_stat.window_pktSize>>16);
	}	

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_get_aqm_flow_statistic_flow_idx */


//PIE
/*
 * rt_qos set aqm pie enable <UINT:enable> deepq <UINT:deepq> ldpid <UINT:ldpid>
 */
cparser_result_t
cparser_cmd_rt_qos_set_aqm_pie_enable_enable_deepq_deepq_ldpid_ldpid(
	cparser_context_t *context,
	uint32_t  *enable_ptr,
	uint32_t  *deepq_ptr,
	uint32_t  *ldpid_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_enable_t aqm_pie_enable;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	aqm_pie_enable.enable=*enable_ptr;
	aqm_pie_enable.deepq=*deepq_ptr;
	aqm_pie_enable.ldpid=*ldpid_ptr;

	ret = rt_qos_aqm_pie_enable_set(aqm_pie_enable);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to set aqm pie enable... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to set aqm pie enable. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set aqm pie enable successfully.\n");
	}

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_set_aqm_pie_enable_enable_deepq_deepq_ldpid_ldpid */

/*
 * rt_qos get aqm pie enable deepq <UINT:deepq> ldpid <UINT:ldpid>
 */
cparser_result_t
cparser_cmd_rt_qos_get_aqm_pie_enable_deepq_deepq_ldpid_ldpid(
	cparser_context_t *context,
	uint32_t  *deepq_ptr,
	uint32_t  *ldpid_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_enable_t aqm_pie_enable;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&aqm_pie_enable, 0x0, sizeof(rt_qos_aqm_pie_enable_t));
	aqm_pie_enable.deepq=*deepq_ptr;
	aqm_pie_enable.ldpid=*ldpid_ptr;

	ret = rt_qos_aqm_pie_enable_get(&aqm_pie_enable);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get aqm pie enable... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get aqm pie enable. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("deepq[%u] ldpid[%u]: %s\n", *deepq_ptr, *ldpid_ptr, aqm_pie_enable.enable?"enabled":"disabled");
	}
	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_get_aqm_pie_enable_deepq_deepq_ldpid_ldpid */

/*
 * rt_qos set aqm pie match <UINT:tch> profile <UINT:profile> ldpid <UINT:ldpid> ldpid_msk <UINT:ldpid_msk> cos <UINT:cos> cos_msk <UINT:cos_msk>
 */
cparser_result_t
cparser_cmd_rt_qos_set_aqm_pie_match_match_profile_profile_ldpid_ldpid_ldpid_msk_ldpid_msk_cos_cos_cos_msk_cos_msk(
	cparser_context_t *context,
	uint32_t  *match_ptr,
	uint32_t  *profile_ptr,
	uint32_t  *ldpid_ptr,
	uint32_t  *ldpid_msk_ptr,
	uint32_t  *cos_ptr,
	uint32_t  *cos_msk_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_match_t aqm_pie_match;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	aqm_pie_match.match=*match_ptr;
	aqm_pie_match.profile=*profile_ptr;
	aqm_pie_match.ldpid=*ldpid_ptr;
	aqm_pie_match.ldpid_msk=*ldpid_msk_ptr;
	aqm_pie_match.cos=*cos_ptr;
	aqm_pie_match.cos_msk=*cos_msk_ptr;

	ret = rt_qos_aqm_pie_match_set(aqm_pie_match);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to set aqm pie match... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to set aqm pie match. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set aqm pie match successfully.\n");
	}

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_set_aqm_pie_match_match_profile_profile_ldpid_ldpid_ldpid_msk_ldpid_msk_cos_cos_cos_msk_cos_msk */

/*
 * rt_qos get aqm pie match profile <UINT:profile>
 */
cparser_result_t
cparser_cmd_rt_qos_get_aqm_pie_match_profile_profile(
	cparser_context_t *context,
	uint32_t  *profile_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_match_t aqm_pie_match;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&aqm_pie_match, 0x0, sizeof(rt_qos_aqm_pie_match_t));
	aqm_pie_match.profile=*profile_ptr;

	ret = rt_qos_aqm_pie_match_get(&aqm_pie_match);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get aqm pie match... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get aqm pie match. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("profile[%u]: %s with ldpid %u mask 0x%x cos %u mask 0x%x \n", *profile_ptr, aqm_pie_match.match?"match":"unmatch", aqm_pie_match.ldpid, aqm_pie_match.ldpid_msk, aqm_pie_match.cos, aqm_pie_match.cos_msk);
	}
	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_get_aqm_pie_match_profile_profile */

/*
 * rt_qos set aqm pie mapping <UINT:mapping> ldpid <UINT:ldpid> cos <UINT:cos> pie_voq <UINT:pie_voq>
 */
cparser_result_t
cparser_cmd_rt_qos_set_aqm_pie_mapping_mapping_ldpid_ldpid_cos_cos_pie_voq_pie_voq(
	cparser_context_t *context,
	uint32_t  *mapping_ptr,
	uint32_t  *ldpid_ptr,
	uint32_t  *cos_ptr,
	uint32_t  *pie_voq_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_mapping_t aqm_pie_mapping;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	aqm_pie_mapping.mapping=*mapping_ptr;
	aqm_pie_mapping.pie_voq=*pie_voq_ptr;
	aqm_pie_mapping.ldpid=*ldpid_ptr;
	aqm_pie_mapping.cos=*cos_ptr;

	ret = rt_qos_aqm_pie_mapping_set(aqm_pie_mapping);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to set aqm pie mapping... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to set aqm pie mapping. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set aqm pie mapping successfully.\n");
	}

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_set_aqm_pie_mapping_mapping_ldpid_ldpid_cos_cos_pie_voq_pie_voq */

/*
 * rt_qos get aqm pie mapping ldpid <UINT:ldpid> cos <UINT:cos>
 */
cparser_result_t
cparser_cmd_rt_qos_get_aqm_pie_mapping_ldpid_ldpid_cos_cos(
	cparser_context_t *context,
	uint32_t  *ldpid_ptr,
	uint32_t  *cos_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_mapping_t aqm_pie_mapping;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&aqm_pie_mapping, 0x0, sizeof(rt_qos_aqm_pie_mapping_t));
	aqm_pie_mapping.ldpid=*ldpid_ptr;
	aqm_pie_mapping.cos=*cos_ptr;

	ret = rt_qos_aqm_pie_mapping_get(&aqm_pie_mapping);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get aqm pie mapping... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get aqm pie mapping. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		if(aqm_pie_mapping.mapping)
			diag_util_mprintf("ldpid %u cos %u mapping to pie_voq[%u].\n", aqm_pie_mapping.ldpid, aqm_pie_mapping.cos, aqm_pie_mapping.pie_voq);
		else
			diag_util_mprintf("ldpid %u cos %u unmapping.\n", aqm_pie_mapping.ldpid, aqm_pie_mapping.cos);
	}
	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_get_aqm_pie_mapping_ldpid_ldpid_cos_cos */

/*
 * rt_qos set aqm pie threshold flow_type <UINT:flow_type> trigger <UINT:trigger> full <UINT:full> mean <UINT:mean>
 */
cparser_result_t
cparser_cmd_rt_qos_set_aqm_pie_threshold_flow_type_flow_type_trigger_trigger_full_full_mean_mean(
	cparser_context_t *context,
	uint32_t  *flow_type_ptr,
	uint32_t  *trigger_ptr,
	uint32_t  *full_ptr,
	uint32_t  *mean_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_threshold_t aqm_pie_threshold;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	aqm_pie_threshold.flow_type=*flow_type_ptr;
	aqm_pie_threshold.trigger=*trigger_ptr;
	aqm_pie_threshold.full=*full_ptr;
	aqm_pie_threshold.mean=*mean_ptr;

	ret = rt_qos_aqm_pie_threshold_set(aqm_pie_threshold);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to set aqm pie threshold... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to set aqm pie threshold. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set aqm pie threshold successfully.\n");
	}

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_set_aqm_pie_threshold_flow_type_flow_type_trigger_trigger_full_full_mean_mean */

/*
 * rt_qos get aqm pie threshold flow_type <UINT:flow_type>
 */
cparser_result_t
cparser_cmd_rt_qos_get_aqm_pie_threshold_flow_type_flow_type(
	cparser_context_t *context,
	uint32_t  *flow_type_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_threshold_t aqm_pie_threshold;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&aqm_pie_threshold, 0x0, sizeof(rt_qos_aqm_pie_threshold_t));
	aqm_pie_threshold.flow_type=*flow_type_ptr;

	ret = rt_qos_aqm_pie_threshold_get(&aqm_pie_threshold);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get aqm pie threshold... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get aqm pie threshold. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("flow_type[%u] trigger=%u full=%u mean=%u\n", *flow_type_ptr, aqm_pie_threshold.trigger, aqm_pie_threshold.full, aqm_pie_threshold.mean);
	}
	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_get_aqm_pie_threshold_flow_type_flow_type */

/*
 * rt_qos set aqm pie dp flow_type <UINT:flow_type> scaling <UINT:alpha> beta <UINT:beta>
 */
cparser_result_t
cparser_cmd_rt_qos_set_aqm_pie_dp_flow_type_flow_type_scaling_alpha_beta_beta(
	cparser_context_t *context,
	uint32_t  *flow_type_ptr,
	uint32_t  *alpha_ptr,
	uint32_t  *beta_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_dpScale_t aqm_pie_dpScale;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	aqm_pie_dpScale.flow_type=*flow_type_ptr;
	aqm_pie_dpScale.alpha=*alpha_ptr;
	aqm_pie_dpScale.beta=*beta_ptr;

	ret = rt_qos_aqm_pie_dpScale_set(aqm_pie_dpScale);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to set aqm pie dpScale... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to set aqm pie dpScale. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set aqm pie dpScale successfully.\n");
	}

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_set_aqm_pie_dp_flow_type_flow_type_scaling_alpha_beta_beta */

/*
 * rt_qos get aqm pie dp flow_type <UINT:flow_type> scaling
 */
cparser_result_t
cparser_cmd_rt_qos_get_aqm_pie_dp_flow_type_flow_type_scaling(
	cparser_context_t *context,
	uint32_t  *flow_type_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_dpScale_t aqm_pie_dpScale;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&aqm_pie_dpScale, 0x0, sizeof(rt_qos_aqm_pie_dpScale_t));
	aqm_pie_dpScale.flow_type=*flow_type_ptr;

	ret = rt_qos_aqm_pie_dpScale_get(&aqm_pie_dpScale);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get aqm pie dpScale... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get aqm pie dpScale. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("flow_type[%u] alpha=%u beta=%u\n", *flow_type_ptr, aqm_pie_dpScale.alpha, aqm_pie_dpScale.beta);
	}
	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_get_aqm_pie_dp_flow_type_flow_type_scaling */

/*
 * rt_qos set aqm pie dp flow_type <UINT:flow_type> threshold <UINT:curr_ceiling> accu_ceiling <UINT:accu_ceiling> accu_floor <UINT:accu_floor>
 */
cparser_result_t
cparser_cmd_rt_qos_set_aqm_pie_dp_flow_type_flow_type_threshold_curr_ceiling_accu_ceiling_accu_ceiling_accu_floor_accu_floor(
	cparser_context_t *context,
	uint32_t  *flow_type_ptr,
	uint32_t  *curr_ceiling_ptr,
	uint32_t  *accu_ceiling_ptr,
	uint32_t  *accu_floor_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_dpThreshold_t aqm_pie_dpThreshold;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	aqm_pie_dpThreshold.flow_type=*flow_type_ptr;
	aqm_pie_dpThreshold.ceiling=*curr_ceiling_ptr;
	aqm_pie_dpThreshold.accu_ceiling=*accu_ceiling_ptr;
	aqm_pie_dpThreshold.accu_floor=*accu_floor_ptr;

	ret = rt_qos_aqm_pie_dpThreshold_set(aqm_pie_dpThreshold);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to set aqm pie dpThreshold... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to set aqm pie dpThreshold. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set aqm pie dpThreshold successfully.\n");
	}

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_set_aqm_pie_dp_flow_type_flow_type_threshold_curr_ceiling_accu_ceiling_accu_ceiling_accu_floor_accu_floor */

/*
 * rt_qos get aqm pie dp flow_type <UINT:flow_type> threshold
 */
cparser_result_t
cparser_cmd_rt_qos_get_aqm_pie_dp_flow_type_flow_type_threshold(
	cparser_context_t *context,
	uint32_t  *flow_type_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_dpThreshold_t aqm_pie_dpThreshold;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&aqm_pie_dpThreshold, 0x0, sizeof(rt_qos_aqm_pie_dpThreshold_t));
	aqm_pie_dpThreshold.flow_type=*flow_type_ptr;

	ret = rt_qos_aqm_pie_dpThreshold_get(&aqm_pie_dpThreshold);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get aqm pie dpThreshold... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get aqm pie dpThreshold. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("flow_type[%u] curr_ceiling=%u accu_ceiling=%u accu_floor=%u\n", *flow_type_ptr, aqm_pie_dpThreshold.ceiling, aqm_pie_dpThreshold.accu_ceiling, aqm_pie_dpThreshold.accu_floor);
	}
	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_get_aqm_pie_dp_flow_type_flow_type_threshold */

/*
 * rt_qos set aqm pie dp flow_type <UINT:flow_type> burst <UINT:allowance> decrement <UINT:decrement>
 */
cparser_result_t
cparser_cmd_rt_qos_set_aqm_pie_dp_flow_type_flow_type_burst_allowance_decrement_decrement(
	cparser_context_t *context,
	uint32_t  *flow_type_ptr,
	uint32_t  *allowance_ptr,
	uint32_t  *decrement_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_dpBurst_t aqm_pie_dpBurst;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	aqm_pie_dpBurst.flow_type=*flow_type_ptr;
	aqm_pie_dpBurst.allowance=*allowance_ptr;
	aqm_pie_dpBurst.decrement=*decrement_ptr;

	ret = rt_qos_aqm_pie_dpBurst_set(aqm_pie_dpBurst);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to set aqm pie dpBurst... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to set aqm pie dpBurst. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set aqm pie dpBurst successfully.\n");
	}

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_set_aqm_pie_dp_flow_type_flow_type_burst_allowance_decrement_decrement */

/*
 * rt_qos get aqm pie dp flow_type <UINT:flow_type> burst
 */
cparser_result_t
cparser_cmd_rt_qos_get_aqm_pie_dp_flow_type_flow_type_burst(
	cparser_context_t *context,
	uint32_t  *flow_type_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_dpBurst_t aqm_pie_dpBurst;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&aqm_pie_dpBurst, 0x0, sizeof(rt_qos_aqm_pie_dpBurst_t));
	aqm_pie_dpBurst.flow_type=*flow_type_ptr;

	ret = rt_qos_aqm_pie_dpBurst_get(&aqm_pie_dpBurst);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get aqm pie dpBurst... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get aqm pie dpBurst. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("flow_type[%u] allowance=%u decrement=%u\n", *flow_type_ptr, aqm_pie_dpBurst.allowance, aqm_pie_dpBurst.decrement);
	}
	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_get_aqm_pie_dp_flow_type_flow_type_burst */

/*
 * rt_qos set aqm pie qdelay flow_type <UINT:flow_type> pie_voq <UINT:pie_voq> burst_reset <UINT:burst_reset> high_latency <UINT:high_latency> low_latency <UINT:low_latency> minimum_target <UINT:minimum_target>
 */
cparser_result_t
cparser_cmd_rt_qos_set_aqm_pie_qdelay_flow_type_flow_type_pie_voq_pie_voq_burst_reset_burst_reset_high_latency_high_latency_low_latency_low_latency_minimum_target_minimum_target(
	cparser_context_t *context,
	uint32_t  *flow_type_ptr,
	uint32_t  *pie_voq_ptr,
	uint32_t  *burst_reset_ptr,
	uint32_t  *high_latency_ptr,
	uint32_t  *low_latency_ptr,
	uint32_t  *minimum_target_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_qdelay_t aqm_pie_qdelay;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	aqm_pie_qdelay.flow_type=*flow_type_ptr;
	aqm_pie_qdelay.pie_voq=*pie_voq_ptr;
	aqm_pie_qdelay.burst_reset=*burst_reset_ptr;
	aqm_pie_qdelay.high_latency=*high_latency_ptr;
	aqm_pie_qdelay.low_latency=*low_latency_ptr;
	aqm_pie_qdelay.minimum_target=*minimum_target_ptr;

	ret = rt_qos_aqm_pie_qdelay_set(aqm_pie_qdelay);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to set aqm pie qdelay... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to set aqm pie qdelay. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set aqm pie qdelay successfully.\n");
	}

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_set_aqm_pie_qdelay_flow_type_flow_type_pie_voq_pie_voq_burst_reset_burst_reset_high_latency_high_latency_low_latency_low_latency_minimum_target_minimum_target */

/*
 * rt_qos get aqm pie qdelay flow_type <UINT:flow_type> pie_voq <UINT:pie_voq>
 */
cparser_result_t
cparser_cmd_rt_qos_get_aqm_pie_qdelay_flow_type_flow_type_pie_voq_pie_voq(
	cparser_context_t *context,
	uint32_t  *flow_type_ptr,
	uint32_t  *pie_voq_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_qdelay_t aqm_pie_qdelay;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&aqm_pie_qdelay, 0x0, sizeof(rt_qos_aqm_pie_qdelay_t));
	aqm_pie_qdelay.flow_type=*flow_type_ptr;
	aqm_pie_qdelay.pie_voq=*pie_voq_ptr;

	ret = rt_qos_aqm_pie_qdelay_get(&aqm_pie_qdelay);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get aqm pie qdelay... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get aqm pie qdelay. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("flow_type[%u] pie_voq[%u] burst_reset=%u high=%u low=%u target=%u\n", *flow_type_ptr, *pie_voq_ptr, aqm_pie_qdelay.burst_reset, aqm_pie_qdelay.high_latency, aqm_pie_qdelay.low_latency, aqm_pie_qdelay.minimum_target);
	}
	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_get_aqm_pie_qdelay_flow_type_flow_type_pie_voq_pie_voq */

/*
 * rt_qos set aqm pie rate pie_voq <UINT:pie_voq> msr <UINT:msr> peak <UINT:peak>
 */
cparser_result_t
cparser_cmd_rt_qos_set_aqm_pie_rate_pie_voq_pie_voq_msr_msr_peak_peak(
	cparser_context_t *context,
	uint32_t  *pie_voq_ptr,
	uint32_t  *msr_ptr,
	uint32_t  *peak_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_rate_t aqm_pie_rate;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	aqm_pie_rate.pie_voq=*pie_voq_ptr;
	aqm_pie_rate.msr=*msr_ptr;
	aqm_pie_rate.peak=*peak_ptr;

	ret = rt_qos_aqm_pie_rate_set(aqm_pie_rate);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to set aqm pie rate... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to set aqm pie rate. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set aqm pie rate successfully.\n");
	}

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_set_aqm_pie_rate_pie_voq_pie_voq_msr_msr_peak_peak */

/*
 * rt_qos get aqm pie rate pie_voq <UINT:pie_voq>
 */
cparser_result_t
cparser_cmd_rt_qos_get_aqm_pie_rate_pie_voq_pie_voq(
	cparser_context_t *context,
	uint32_t  *pie_voq_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_rate_t aqm_pie_rate;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&aqm_pie_rate, 0x0, sizeof(rt_qos_aqm_pie_rate_t));
	aqm_pie_rate.pie_voq=*pie_voq_ptr;

	ret = rt_qos_aqm_pie_rate_get(&aqm_pie_rate);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get aqm pie rate... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get aqm pie rate. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("pie_voq[%u] msr=%u peak=%u\n", *pie_voq_ptr, aqm_pie_rate.msr, aqm_pie_rate.peak);
	}
	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_get_aqm_pie_rate_pie_voq_pie_voq */

/*
 * rt_qos set aqm pie cfg mode <UINT:mode> timer_interval <UINT:timer_interval> timer_prescalar <UINT:timer_prescalar> dp_scale <UINT:dp_scale> random <UINT:random> fix_LFSR <UINT:fix_LFSR>
 */
cparser_result_t
cparser_cmd_rt_qos_set_aqm_pie_cfg_mode_mode_timer_interval_timer_interval_timer_prescalar_timer_prescalar_dp_scale_dp_scale_random_random_fix_LFSR_fix_LFSR(
	cparser_context_t *context,
	uint32_t  *mode_ptr,
	uint32_t  *timer_interval_ptr,
	uint32_t  *timer_prescalar_ptr,
	uint32_t  *dp_scale_ptr,
	uint32_t  *random_ptr,
	uint32_t  *fix_LFSR_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_cfg_t aqm_pie_cfg;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	aqm_pie_cfg.mode=*mode_ptr;
	aqm_pie_cfg.timer_interval=*timer_interval_ptr;
	aqm_pie_cfg.timer_prescalar=*timer_prescalar_ptr;
	aqm_pie_cfg.dp_scale=*dp_scale_ptr;
	aqm_pie_cfg.random=*random_ptr;
	aqm_pie_cfg.fix_LFSR=*fix_LFSR_ptr;

	ret = rt_qos_aqm_pie_cfg_set(aqm_pie_cfg);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to set aqm pie cfg... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to set aqm pie cfg. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set aqm pie cfg successfully.\n");
	}

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_set_aqm_pie_cfg_mode_mode_timer_interval_timer_interval_timer_prescalar_timer_prescalar_dp_scale_dp_scale_random_random_fix_lfsr_fix_lfsr */

/*
 * rt_qos get aqm pie cfg
 */
cparser_result_t
cparser_cmd_rt_qos_get_aqm_pie_cfg(
	cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_qos_aqm_pie_cfg_t aqm_pie_cfg;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rt_qos_aqm_pie_cfg_get(&aqm_pie_cfg);
	if(ret != RT_ERR_OK){
		if(ret==RT_ERR_DRIVER_NOT_FOUND)
			diag_util_mprintf("Fail to get aqm pie cfg... Chip does not support !!! \n");
		else
			diag_util_mprintf("Fail to get aqm pie cfg. RT API return 0x%x \n", ret);
		
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("mode=%u timer interval=%u precalar=%u dp_scale=%u random=%u fix_LFSR=%u\n", aqm_pie_cfg.mode, aqm_pie_cfg.timer_interval, aqm_pie_cfg.timer_prescalar, aqm_pie_cfg.dp_scale, aqm_pie_cfg.random, aqm_pie_cfg.fix_LFSR);
	}
	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_qos_get_aqm_pie_cfg */


