#include <stdio.h>
#include <string.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <diag_str.h>
#include <parser/cparser_priv.h>
#include <rtk/rt/rt_time.h>

/*
 * rt_time init
 */
cparser_result_t
cparser_cmd_rt_time_init(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(rt_time_init(), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_time_init */

/*
 * rt_time get cur-time
 */
cparser_result_t
cparser_cmd_rt_time_get_cur_time(
    cparser_context_t *context)
{
    int32 ret;
    rt_time_timeStamp_t timeStamp;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rt_time_curTime_get(&timeStamp), ret);

    diag_util_mprintf("Current Time: %lld sec   %u nanosec\n", timeStamp.sec, timeStamp.nsec);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_time_get_cur_time */

/*
 * rt_time set offset-time ( increase | decrease ) <UINT64:second> <UINT:nanosecond>
 */
cparser_result_t
cparser_cmd_rt_time_set_offset_time_increase_decrease_second_nanosecond(
    cparser_context_t *context,
    uint64_t  *second_ptr,
    uint32_t  *nanosecond_ptr)
{
    DIAG_UTIL_PARAM_CHK();

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_time_set_offset_time_increase_decrease_second_nanosecond */

/*
 * rt_time get frequency
 */
cparser_result_t
cparser_cmd_rt_time_get_frequency(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("");

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_time_get_frequency */

/*
 * rt_time set frequency <UINT:frequency>
 */
cparser_result_t
cparser_cmd_rt_time_set_frequency_frequency(
    cparser_context_t *context,
    uint32_t  *frequency_ptr)
{
    DIAG_UTIL_PARAM_CHK();

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_time_set_frequency_frequency */

/*
 * rt_time set ptp port ( <PORT_LIST:ports> | all ) state ( enable | disable )
 */
cparser_result_t
cparser_cmd_rt_time_set_ptp_port_ports_all_state_enable_disable(
    cparser_context_t *context,
    char * *ports_ptr)
{
    DIAG_UTIL_PARAM_CHK();

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_time_set_ptp_port_ports_all_state_enable_disable */

/*
 * rt_time get ptp port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t
cparser_cmd_rt_time_get_ptp_port_ports_all_state(
    cparser_context_t *context,
    char * *ports_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("");

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_time_get_ptp_port_ports_all_state */

/*
 * rt_time set ptp ingress ( l2 | udp ) action ( nop | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_rt_time_set_ptp_ingress_l2_udp_action_nop_trap_to_cpu(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_time_set_ptp_ingress_l2_udp_action_nop_trap_to_cpu */

/*
 * rt_time get ptp ingress ( l2 | udp ) action
 */
cparser_result_t
cparser_cmd_rt_time_get_ptp_ingress_l2_udp_action(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("");

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_time_get_ptp_ingress_l2_udp_action */

/*
 * rt_time set ptp egress action ( nop | one-step | two-step )
 */
cparser_result_t
cparser_cmd_rt_time_set_ptp_egress_action_nop_one_step_two_step(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_time_set_ptp_egress_action_nop_one_step_two_step */

/*
 * rt_time get ptp egress action
 */
cparser_result_t
cparser_cmd_rt_time_get_ptp_egress_action(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("");

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_time_get_ptp_egress_action */

/*
 * rt_time set pontod ( xgpon | gpon | epon ) startpoint <UINT64:cnt> time <UINT64:second> <UINT:nanosecond>
 */
cparser_result_t
cparser_cmd_rt_time_set_pontod_xgpon_gpon_epon_startpoint_cnt_time_second_nanosecond(
    cparser_context_t *context,
    uint64_t  *cnt_ptr,
    uint64_t  *second_ptr,
    uint32_t  *nanosecond_ptr)
{
    int32     ret = RT_ERR_FAILED;
    rt_pon_tod_t ponTod;

    DIAG_UTIL_PARAM_CHK();

    if ('x' == TOKEN_CHAR(3,0))
    {
        ponTod.ponMode = 2;//xgpon
        ponTod.startPoint.xSuperFrame = *cnt_ptr;
    }
    else if ('g' == TOKEN_CHAR(3,0))
    {
        ponTod.ponMode = 1;//gpon
        ponTod.startPoint.superFrame = *cnt_ptr;
    }
    else //if ('e' == TOKEN_CHAR(3,0))
    {
        ponTod.ponMode = 0;//epon
        ponTod.startPoint.localTime = *cnt_ptr;
    }

    ponTod.timeStamp.sec = *second_ptr;
    ponTod.timeStamp.nsec = *nanosecond_ptr;
    DIAG_UTIL_ERR_CHK(rt_time_ponTodTime_set(ponTod), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_time_set_pontod_gpon_epon_startpoint_cnt_time_second_nanosecond */

/*
 * rt_time set pontod state ( enable | disable )
 */
cparser_result_t
cparser_cmd_rt_time_set_pontod_state_enable_disable(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rt_enable_t enable;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(4,0))
    {
        enable = DISABLED;
    }
    else //if ('e' == TOKEN_CHAR(10,0))
    {
        enable = ENABLED;
    }

    DIAG_UTIL_ERR_CHK(rt_time_ponTodTimeEnable_set(enable), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_time_set_pontod_state_enable_disable */

/*
 * rt_time get pontod
 */
cparser_result_t
cparser_cmd_rt_time_get_pontod(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rt_pon_tod_t ponTod;
    rt_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rt_time_ponTodTimeEnable_get(&enable), ret);
    DIAG_UTIL_ERR_CHK(rt_time_ponTodTime_get(&ponTod), ret);

    diag_util_mprintf("PON ToD state: %s\n",diagStr_enable[enable]);

    if(ponTod.ponMode == 2)
        diag_util_mprintf("XGPON ToD superframe: %llu  timestamp: %llu sec   %u nanosec\n",ponTod.startPoint.xSuperFrame,ponTod.timeStamp.sec,ponTod.timeStamp.nsec);
    else if(ponTod.ponMode == 1)
        diag_util_mprintf("GPON ToD superframe: %u  timestamp: %llu sec   %u nanosec\n",ponTod.startPoint.superFrame,ponTod.timeStamp.sec,ponTod.timeStamp.nsec);
    else
        diag_util_mprintf("EPON ToD localtime: %u  timestamp: %llu sec   %u nanosec\n",ponTod.startPoint.localTime,ponTod.timeStamp.sec,ponTod.timeStamp.nsec);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_time_get_pontod */
