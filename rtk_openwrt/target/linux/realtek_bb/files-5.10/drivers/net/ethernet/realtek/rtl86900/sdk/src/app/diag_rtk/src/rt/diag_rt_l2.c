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
 * $Revision: $
 * $Date: $
 *
 * Purpose : Definition those XXX command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *
 */

/*
 * Include Files
 */
#include <stdio.h>
#include <string.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <diag_str.h>
#include <parser/cparser_priv.h>
#include <rtk/rt/rt_l2.h>


/*
 * rt_l2-table init
 */
cparser_result_t
cparser_cmd_rt_l2_table_init(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rt_l2_init(), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_l2_table_init */

/*
 * rt_l2-table get limit-learning port ( <PORT_LIST:ports> | all ) count
 */
cparser_result_t
cparser_cmd_rt_l2_table_get_limit_learning_port_ports_all_count(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret;
    rt_port_t port;
    diag_portlist_t portlist;
    uint32 cnt;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_l2_portLimitLearningCnt_get(port, &cnt), ret);
        diag_util_printf("\n Port %d learning limit: %d", port, cnt);
    }

    diag_util_printf("\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_l2_table_get_limit_learning_port_ports_all_count */

/*
 * rt_l2-table set limit-learning port ( <PORT_LIST:ports> | all ) count <INT:count>
 */
cparser_result_t
cparser_cmd_rt_l2_table_set_limit_learning_port_ports_all_count_count(
    cparser_context_t *context,
    char * *ports_ptr,
    int32_t  *count_ptr)
{
    int32 ret;
    rt_port_t port;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_l2_portLimitLearningCnt_set(port, *count_ptr), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_l2_table_set_limit_learning_port_ports_all_count_count */

/*
 * rt_l2-table get unknown-sa port ( <PORT_LIST:ports> | all ) action
 */
cparser_result_t
cparser_cmd_rt_l2_table_get_unknown_sa_port_ports_all_action(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret;
    rtk_port_t port;
    diag_portlist_t portlist;
    rt_action_t action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_l2_newMacOp_get(port, &action), ret);
        diag_util_printf("\n Port %d unknown SA Action: %s", port, diagStr_actionStr[action]);
    }
    diag_util_printf("\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_l2_table_get_unknown_sa_port_ports_all_action */

/*
 * rt_l2-table set unknown-sa port ( <PORT_LIST:ports> | all ) action ( drop | forward )
 */
cparser_result_t
cparser_cmd_rt_l2_table_set_unknown_sa_port_ports_all_action_drop_forward(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret;
    rtk_port_t port;
    diag_portlist_t portlist;
    rt_action_t action;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('d' == TOKEN_CHAR(6, 0))
        action = ACTION_DROP;
    else if('f' == TOKEN_CHAR(6, 0))
        action = ACTION_FORWARD;
    else
        return CPARSER_NOT_OK;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_l2_newMacOp_set(port, action), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_l2_table_set_unknown_sa_port_ports_all_action_drop_forward */

static int32
_display_l2Ucast_entry(rt_l2_ucastAddr_t *pL2Addr)
{

        diag_util_mprintf("MACAddress         Spa    Age    vid Static Filter\n");
        diag_util_mprintf("----------------- ---- ------ ------ ------ ------\n");
        diag_util_mprintf("%-17s %4d %6d %6d %6s %6s\n"
                                    ,diag_util_inet_mactoa(&pL2Addr->mac.octet[0])
                                    ,pL2Addr->port
                                    ,pL2Addr->age
                                    ,pL2Addr->vid
                                    ,(pL2Addr->staticFlag == ENABLED)?"En":"Dis"
                                    ,(pL2Addr->filterFlag == ENABLED)?"En":"Dis");

    return RT_ERR_OK;
}

/*
 * rt_l2-table add mac-ucast vid  <UINT:vid> mac-address <MACADDR:mac> spn <UINT:port>
 */
cparser_result_t
cparser_cmd_rt_l2_table_add_mac_ucast_vid_vid_mac_address_mac_spn_port(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *port_ptr)
{
    int32 ret;
    rt_l2_ucastAddr_t l2Addr;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&l2Addr, 0x00, sizeof(rt_l2_ucastAddr_t));
    osal_memcpy(&l2Addr.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    l2Addr.vid = *vid_ptr;

    ret = rt_l2_addr_get(&l2Addr);
    if(RT_ERR_OK != ret)
    {
        osal_memset(&l2Addr, 0x00, sizeof(rt_l2_ucastAddr_t));
        osal_memcpy(&l2Addr.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
        l2Addr.port = HAL_GET_MIN_PORT();
        l2Addr.vid = *vid_ptr;
    }

    l2Addr.port = *port_ptr;

    DIAG_UTIL_ERR_CHK(rt_l2_addr_add(&l2Addr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_l2_table_add_mac_ucast_vid_vid_mac_address_mac_spn_port */

/*
 * rt_l2-table add mac-ucast vid <UINT:vid> mac-address <MACADDR:mac> static state ( disable | enable )
 */
cparser_result_t
cparser_cmd_rt_l2_table_add_mac_ucast_vid_vid_mac_address_mac_static_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    cparser_macaddr_t  *mac_ptr)
{
    int32 ret;
    rt_l2_ucastAddr_t l2Addr;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&l2Addr, 0x00, sizeof(rt_l2_ucastAddr_t));
    osal_memcpy(&l2Addr.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    l2Addr.vid = *vid_ptr;

    ret = rt_l2_addr_get(&l2Addr);
    if(RT_ERR_OK != ret)
    {
        osal_memset(&l2Addr, 0x00, sizeof(rt_l2_ucastAddr_t));
        osal_memcpy(&l2Addr.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
        l2Addr.port = HAL_GET_MIN_PORT();
        l2Addr.vid = *vid_ptr;
    }

    if('e' == TOKEN_CHAR(9, 0))
        l2Addr.staticFlag = 1;
    else if('d' == TOKEN_CHAR(9, 0))
        l2Addr.staticFlag = 0;
    else
        return CPARSER_NOT_OK;

    DIAG_UTIL_ERR_CHK(rt_l2_addr_add(&l2Addr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_l2_table_add_mac_ucast_vid_vid_mac_address_mac_static_state_disable_enable */

/*
 * rt_l2-table add mac-ucast vid <UINT:vid> mac-address <MACADDR:mac> filter flag ( disable | enable )
 */
cparser_result_t
cparser_cmd_rt_l2_table_add_mac_ucast_vid_vid_mac_address_mac_filter_flag_disable_enable(
//cparser_cmd_rt_l2_table_add_mac_ucast_mac_address_mac_spn_port(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    cparser_macaddr_t  *mac_ptr)
{
    int32 ret;
    rt_l2_ucastAddr_t l2Addr;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&l2Addr, 0x00, sizeof(rt_l2_ucastAddr_t));
    osal_memcpy(&l2Addr.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    l2Addr.vid = *vid_ptr;

    ret = rt_l2_addr_get(&l2Addr);
    if(RT_ERR_OK != ret)
    {
        osal_memset(&l2Addr, 0x00, sizeof(rt_l2_ucastAddr_t));
        osal_memcpy(&l2Addr.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
        l2Addr.port = HAL_GET_MIN_PORT();
        l2Addr.vid = *vid_ptr;
    }

    if('e' == TOKEN_CHAR(9, 0))
        l2Addr.filterFlag = 1;
    else if('d' == TOKEN_CHAR(9, 0))
        l2Addr.filterFlag = 0;
    else
        return CPARSER_NOT_OK;

    
    DIAG_UTIL_ERR_CHK(rt_l2_addr_add(&l2Addr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_l2_table_add_mac_ucast_vid_vid_mac_address_mac_filter_flag_disable_enable */

/*
 * rt_l2-table del mac-ucast vid <UINT:vid> mac-address <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_rt_l2_table_del_mac_ucast_vid_vid_mac_address_mac(
    cparser_context_t *context,
    uint32_t  *vid_ptr,    
    cparser_macaddr_t  *mac_ptr)
{
    int32 ret;
    rt_l2_ucastAddr_t l2Addr;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&l2Addr, 0x00, sizeof(rt_l2_ucastAddr_t));
    osal_memcpy(&l2Addr.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    l2Addr.vid = *vid_ptr;

    DIAG_UTIL_ERR_CHK(rt_l2_addr_del(&l2Addr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_l2_table_del_mac_ucast_vid_vid_mac_address_mac */

/*
 * rt_l2-table get mac-ucast vid <UINT:vid> mac-address <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_rt_l2_table_get_mac_ucast_vid_vid_mac_address_mac(
    cparser_context_t *context,
    uint32_t  *vid_ptr,    
    cparser_macaddr_t  *mac_ptr)
{
    int32 ret;
    rt_l2_ucastAddr_t l2Addr;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&l2Addr, 0x00, sizeof(rt_l2_ucastAddr_t));
    osal_memcpy(&l2Addr.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    l2Addr.vid = *vid_ptr;

    DIAG_UTIL_ERR_CHK(rt_l2_addr_get(&l2Addr), ret);
    _display_l2Ucast_entry(&l2Addr);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_l2_table_get_mac_ucast_vid_vid_mac_address_mac */

/*
 * rt_l2-table get next-entry mac-ucast address <UINT:address>
 */
cparser_result_t
cparser_cmd_rt_l2_table_get_next_entry_mac_ucast_address_address(
    cparser_context_t *context,
    uint32_t  *address_ptr)
{
    int32 ret;
    int32 address;
    rt_l2_ucastAddr_t l2Addr;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    address = *address_ptr;
    osal_memset(&l2Addr, 0x00, sizeof(rt_l2_ucastAddr_t));

    DIAG_UTIL_ERR_CHK(rt_l2_nextValidAddr_get(&address, &l2Addr), ret);
    _display_l2Ucast_entry(&l2Addr);
    diag_util_printf("\n Address = %d", address);
    diag_util_printf("\n");


    return CPARSER_OK;
}    /* end of cparser_cmd_rt_l2_table_get_next_entry_mac_ucast_address_address */

/*
 * rt_l2-table get next-entry mac-ucast address <UINT:address> spn <UINT:port>
 */
cparser_result_t
cparser_cmd_rt_l2_table_get_next_entry_mac_ucast_address_address_spn_port(
    cparser_context_t *context,
    uint32_t  *address_ptr,
    uint32_t  *port_ptr)
{
    int32 ret;
    int32 address;
    rt_l2_ucastAddr_t l2Addr;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    address = *address_ptr;
    osal_memset(&l2Addr, 0x00, sizeof(rt_l2_ucastAddr_t));

    DIAG_UTIL_ERR_CHK(rt_l2_nextValidAddrOnPort_get(*port_ptr, &address, &l2Addr), ret);
    _display_l2Ucast_entry(&l2Addr);
    diag_util_printf("\n Address = %d", address);
    diag_util_printf("\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_l2_table_get_next_entry_mac_ucast_address_address_spn_port */

/*
 * rt_l2-table get port-move port ( <PORT_LIST:ports> | all ) action
 */
cparser_result_t
cparser_cmd_rt_l2_table_get_port_move_port_ports_all_action(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret;
    rt_port_t port;
    diag_portlist_t portlist;
    rt_action_t action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_l2_illegalPortMoveAction_get(port, &action), ret);
        diag_util_printf("\n Port %d Port move Action: %s", port, diagStr_actionStr[action]);
    }
    diag_util_printf("\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_l2_table_get_port_move_port_ports_all_action */

/*
 * rt_l2-table set port-move port ( <PORT_LIST:ports> | all ) action ( copy-to-cpu | drop | forward | trap-to-cpu ) */
cparser_result_t
cparser_cmd_rt_l2_table_set_port_move_port_ports_all_action_copy_to_cpu_drop_forward_trap_to_cpu(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret;
    rtk_port_t port;
    diag_portlist_t portlist;
    rtk_action_t action;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('c' == TOKEN_CHAR(6, 0))
        action = ACTION_COPY2CPU;
    else if('d' == TOKEN_CHAR(6, 0))
        action = ACTION_DROP;
    else if('f' == TOKEN_CHAR(6, 0))
        action = ACTION_FORWARD;
    else if('t' == TOKEN_CHAR(6, 0))
        action = ACTION_TRAP2CPU;
    else
        return CPARSER_NOT_OK;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_l2_illegalPortMoveAction_set(port, action), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_l2_table_set_port_move_port_ports_all_action_copy_to_cpu_drop_forward_trap_to_cpu */

 /*
  *rt_l2-table get age-time 
  */
cparser_result_t
cparser_cmd_rt_l2_table_get_age_time(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 ageTime;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rt_l2_ageTime_get(&ageTime), ret);
    diag_util_printf("\n Age Time: %d", ageTime);

    diag_util_printf("\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_l2_table_get_age_time */

/*
 *rt_l2-table set age-time <UINT:time> 
 */
cparser_result_t
cparser_cmd_rt_l2_table_set_age_time_time(
    cparser_context_t *context,
    uint32_t  *time_ptr)
{
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rt_l2_ageTime_set(*time_ptr), ret);
    
    return CPARSER_OK;
}       /* end of cparser_cmd_rt_l2_table_set_age_time_time */

 /*
  *rt_l2-table get ivl-svl 
  */
cparser_result_t
cparser_cmd_rt_l2_table_get_ivl_svl(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rt_enable_t ivlEn;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rt_l2_ivlSvl_get(&ivlEn), ret);
    diag_util_mprintf("\n IVL state: %s", diagStr_enable[ivlEn]);

    diag_util_printf("\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_l2_table_get_ivl_svl */    

/*
 *rt_l2-table set ivl-svl ( enable | disable ) 
 */
cparser_result_t
cparser_cmd_rt_l2_table_set_ivl_svl_enable_disable(
    cparser_context_t *context)
{
    rt_enable_t enable;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if('e'==TOKEN_CHAR(3,0))
        enable = ENABLED;
    else
        enable = DISABLED;

    DIAG_UTIL_ERR_CHK((rt_l2_ivlSvl_set(enable)), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_l2_table_set_ivl_svl_enable_disable */       